#include "CCollectDlg.h"
#include <process.h>


CCollectDlg::CCollectDlg(int _objectIndex) :
	m_objectIndex(_objectIndex)
{
	m_curSettingData.msgOptions[WM_CREATE] = true;
	m_curSettingData.msgOptions[WM_SHOWWINDOW] = true;
	m_curSettingData.msgOptions[WM_WINDOWPOSCHANGED] = true;
	m_curSettingData.msgOptions[WM_STYLECHANGED] = true;
	m_curSettingData.msgOptions[WM_NCACTIVATE] = true;
	m_curSettingData.msgOptions[WM_SETTEXT] = true;
	m_curSettingData.msgOptions[WM_SIZE] = true;
	m_curSettingData.msgOptions[WM_MOVE] = true;
	m_curSettingData.msgOptions[WM_DESTROY] = true;
}

CCollectDlg::~CCollectDlg()
{
}

BOOL CCollectDlg::Start(HWND _parentHwnd, std::vector<std::wstring> _inputProcessName)
{
	m_parentHwnd = _parentHwnd;
	InitializeCriticalSection(&m_readWriteCS);

	m_threadHandle = (HANDLE)_beginthreadex(NULL, 0, DisplayDataThread, (LPVOID)this, NULL, NULL);
	if (INVALID_HANDLE_VALUE == m_threadHandle)
	{
		return FALSE;
	}

	m_ownHwnd = CreateDialogParamW(NULL, MAKEINTRESOURCEW(IDD_COLLECTPAGE), NULL, CCollectDlg::RunProc, (LPARAM)this);
	
	// 캡션 추가
	std::wstring showCaption = L"";
	for (auto x : _inputProcessName)
	{
		showCaption += x + L"|";
	}
	showCaption.pop_back();
	SetWindowTextW(m_ownHwnd, showCaption.data());

	// 창띄우기
	ShowWindow(m_ownHwnd, SW_SHOW);

	return TRUE;
}

BOOL CCollectDlg::End()
{
	if (NULL != m_childOption)
	{
		m_childOption->End(0);
		delete m_childOption;
		m_childOption = NULL;
	}

	LeaveCriticalSection(&m_readWriteCS);

	DestroyWindow(m_ownHwnd);

	m_threadQuit = TRUE;
	WaitForSingleObject(m_threadHandle, INFINITE);
	CloseHandle(m_threadHandle);
	m_threadHandle = INVALID_HANDLE_VALUE;

	DeleteCriticalSection(&m_readWriteCS);
	

	PostMessage(m_parentHwnd, WM_CHILDEND, (WPARAM)m_objectIndex, NULL);
	return 0;
}

INT_PTR CALLBACK CCollectDlg::RunProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CCollectDlg* pointerThis = (CCollectDlg*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lParam);
		pointerThis = (CCollectDlg*)lParam;
		pointerThis->m_ownHwnd = hwnd;
		pointerThis->InitDialog(hwnd, (HWND)(wParam), lParam);
		break;

	case WM_COMMAND:
		pointerThis->Command(hwnd, (int)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam));
		break;

	case WM_CLOSE:
		pointerThis->End();
		break;

	case WM_SETOPTION:
		BOOL succFunc = pointerThis->m_childOption->GetOption(&(pointerThis->m_curSettingData));
		if (TRUE == succFunc && FALSE == pointerThis->m_showMsgData)
		{
			pointerThis->m_showMsgData = TRUE;
		}
		delete pointerThis->m_childOption;
		pointerThis->m_childOption = NULL;
		break;
	}

	return FALSE;
}

// 다이얼로그 초기화
BOOL CCollectDlg::InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	m_startAndSuspend = GetDlgItem(hwnd, IDC_STARTSUSPEND);
	SendMessageW(m_startAndSuspend, WM_SETTEXT, 0, (LPARAM)L"시작");

	//Option 설정
	PostMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDC_OPTION, IDC_OPTION), NULL);
	return TRUE;
}

void CCollectDlg::Command(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	BOOL SuccessFuc = FALSE;
	switch (id)
	{
	case IDCANCEL:
	case IDOK:
		//End();
		break;

	case IDC_STARTSUSPEND:
		if (FALSE == m_showMsgData)
		{
			SendMessageW(m_startAndSuspend, WM_SETTEXT, 0, (LPARAM)L"일시정지");
		}
		else
		{
			SendMessageW(m_startAndSuspend, WM_SETTEXT, 0, (LPARAM)L"시작");
		}
		m_showMsgData = !m_showMsgData;
		break;

	case IDC_FILEOUT:
		SaveLog(hwnd);
		break;

	case IDC_OPTION:
		if (NULL != m_childOption)
		{
			break;
		}
		m_childOption = new COptionDlg();
		m_childOption->Start(m_ownHwnd, &m_curSettingData);
		break;
	}
}

void CCollectDlg::InsertData(MsgData _inputMsgData)
{
	if (FALSE == m_threadQuit)
	{
		EnterCriticalSection(&m_readWriteCS);
		m_inputMsg.push(_inputMsgData);
		LeaveCriticalSection(&m_readWriteCS);
	}
}


UINT CCollectDlg::DisplayDataThread(void * arg)
{
	((CCollectDlg*)arg)->DisplayData();
	return 0;
}

void CCollectDlg::DisplayData()
{
	std::queue<MsgData> inputMsgQeuue;
	while (true)
	{
		HWND listHwnd = GetDlgItem(m_ownHwnd, IDC_COLLECTLIST);
		if (TRUE == m_threadQuit)
		{
			break;
		}		

		EnterCriticalSection(&m_readWriteCS);
		std::swap(inputMsgQeuue, m_inputMsg);
		LeaveCriticalSection(&m_readWriteCS);

		if (TRUE == m_threadQuit)
		{
			break;
		}

		while (!inputMsgQeuue.empty())
		{
			MsgData inputMsgData = inputMsgQeuue.front();
			inputMsgQeuue.pop();
			
			if (false == m_curSettingData.msgOptions[inputMsgData.msgCode]
				&& false == m_curSettingData.otherMsgShow)
			{
				continue;
			}

			if (FALSE == m_showMsgData || TRUE == m_threadQuit)
			{
				std::queue<MsgData> clearQueue;
				std::swap(clearQueue, inputMsgQeuue);
				continue;
			}

			std::wstring viewMsg = L"";
			std::wstring lineNum = std::to_wstring(m_countLine++);

			// Line number(8자리수) 추가
			viewMsg += L"<";
			for (int i = 0; i < 8 - lineNum.size(); i++)
				viewMsg += L"0";
			viewMsg += lineNum + L"> ";

			// Process Name 추가
			viewMsg += L" [" + std::wstring(inputMsgData.processName);

			// Process ID 추가
			viewMsg += L" (" + std::to_wstring(inputMsgData.processID) + L")";

			// Thread ID 추가
			viewMsg += L"[" + std::to_wstring(inputMsgData.threadID) + L"]]";

			// Message Content 추가
			if (NULL == wmToString[inputMsgData.msgCode])
			{
				viewMsg += L" [unknown]";
			}
			else
			{
				viewMsg += L" [" + std::wstring(wmToString[inputMsgData.msgCode]) + L"]";
			}

			// Message Code 추가
			WCHAR buf[32] = { 0, };
			wsprintf(buf, L"0x%08X", inputMsgData.msgCode);
			viewMsg += L"(" + std::wstring(buf) + L")";

			switch (inputMsgData.hookType)
			{
			case MSG_CALLWND:
				// Msg type 추가
				viewMsg += L" S";

				// wParam 추가
				memset(buf, 0, 32);
				wsprintf(buf, L"0x%016X", inputMsgData.wParam);
				viewMsg += L"   wParam : " + std::wstring(buf);

				// lParam 추가
				memset(buf, 0, 32);
				wsprintf(buf, L"0x%016X", inputMsgData.lParam);
				viewMsg += L"   lParam : " + std::wstring(buf);
				break;

			case MSG_CALLWNDRET:
				// Msg type 추가
				viewMsg += L" R";

				// wParam 추가
				memset(buf, 0, 32);
				wsprintf(buf, L"0x%016X", inputMsgData.wParam);
				viewMsg += L"   wParam : " + std::wstring(buf);

				// lParam 추가
				memset(buf, 0, 32);
				wsprintf(buf, L"0x%016X", inputMsgData.lParam);
				viewMsg += L"   lParam : " + std::wstring(buf);
				break;

			case MSG_GETMSG:
				// Msg type 추가
				viewMsg += L" P";

				// wParam 추가
				memset(buf, 0, 32);
				wsprintf(buf, L"0x%016X", inputMsgData.wParam);
				viewMsg += L"   wParam : " + std::wstring(buf);

				// lParam 추가
				memset(buf, 0, 32);
				wsprintf(buf, L"0x%016X", inputMsgData.lParam);
				viewMsg += L"   lParam : " + std::wstring(buf);
				break;

			default:
				break;
			}

			switch (inputMsgData.msgCode)
			{
			case WM_CREATE:
			case WM_SHOWWINDOW:
			case WM_WINDOWPOSCHANGED:
			case WM_STYLECHANGED:
			case WM_NCACTIVATE:
			case WM_SETTEXT:
			case WM_SIZE:
			case WM_MOVE:
			case WM_DESTROY:
				if (NULL == inputMsgData.hwnd)
				{
					break;
				}

				WCHAR longBuf[MAX_PATH] = { 0, };
				GetWindowTextW(inputMsgData.hwnd, longBuf, MAX_PATH);
				viewMsg += L"  Caption: " + std::wstring(longBuf);

				memset(longBuf, 0, MAX_PATH);
				GetClassNameW(inputMsgData.hwnd, longBuf, MAX_PATH);
				viewMsg += L"  ClassName: " + std::wstring(longBuf);

				WNDCLASSW wndClass;
				GetClassInfoW(inputMsgData.hInstance, longBuf, &wndClass);
				
				MakeStyleString(&viewMsg, wndClass.style);

				memset(buf, 0, 32);
				wsprintf(buf, L"0x%08X", wndClass.style);
				viewMsg += L"   style : " + std::wstring(buf);
				break;
			}

			if (TRUE == m_threadQuit)
			{
				break;
			}

			SendMessageW(listHwnd, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)viewMsg.data());
			LRESULT count = SendMessageW(listHwnd, LB_GETCOUNT, 0, 0);
			SendMessageW(listHwnd, LB_SETTOPINDEX, (WPARAM)count - 1, (LPARAM)0);
		}
		
	}
}

void CCollectDlg::MakeStyleString(std::wstring *_inputString, UINT _inputStyle)
{
	*_inputString += L"   style : ";

	if (WS_BORDER & _inputStyle)
	{
		*_inputString += L"WS_BORDER | ";
	}

	if (WS_CAPTION & _inputStyle)
	{
		*_inputString += L"WS_CAPTION | ";
	}

	if (WS_CHILD & _inputStyle)
	{
		*_inputString += L"WS_CHILD | ";
	}
}

BOOL CCollectDlg::SaveLog(HWND hwnd)
{
	HWND ListBoxHwnd = GetDlgItem(hwnd, IDC_COLLECTLIST);
	LRESULT count = SendMessageW(ListBoxHwnd, LB_GETCOUNT, 0, 0);

	// 저장 경로 지정
	OPENFILENAMEW oFile;

	WCHAR saveFileName[MAX_PATH] = L"log";

	memset(&oFile, 0, sizeof(oFile));

	oFile.lStructSize = sizeof(oFile);
	oFile.hwndOwner = NULL;
	oFile.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
	oFile.lpstrFile = saveFileName;
	oFile.nMaxFile = MAX_PATH;
	oFile.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	oFile.lpstrDefExt = (LPCWSTR)L"txt";

	BOOL succFunc = GetSaveFileNameW(&oFile);
	if (FALSE == succFunc)
	{
		MessageBoxW(NULL, L"저장 취소", NULL, MB_OK);
		return FALSE;
	}

	HANDLE fileHandle = CreateFileW(
		saveFileName,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (INVALID_HANDLE_VALUE == fileHandle)
	{
		return FALSE;
	}

	DWORD returnLen = 0;
	LRESULT textLen = 0;
	LPWSTR curText = NULL;

	// UTF-8 설정(WCHAR 출력)
	unsigned short mark = 0xFEFF;
	WriteFile(fileHandle, &mark, sizeof(mark), &returnLen, NULL);

	for (UINT i = 0; i < count; i++)
	{
		textLen = SendMessageW(ListBoxHwnd, LB_GETTEXTLEN, (WPARAM)(i), (LPARAM)NULL);
		curText = new WCHAR[textLen + 1];
		SendMessageW(ListBoxHwnd, LB_GETTEXT, (WPARAM)(i), (LPARAM)curText);
		curText[textLen] = L'\n';

		// 파일 쓰기		
		WriteFile(fileHandle, curText, (textLen + 1) * sizeof(WCHAR), &returnLen, NULL);

		delete[] curText;
		returnLen = 0;
		textLen = 0;
		curText = NULL;
	}

	CloseHandle(fileHandle);

	return TRUE;
}
