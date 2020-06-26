#include "CCollectDlg.h"
#include <process.h>

CCollectDlg::CCollectDlg(int _objectIndex):
	objectIndex(_objectIndex)
{
}

CCollectDlg::~CCollectDlg()
{
}

BOOL CCollectDlg::Start(HWND _parentHwnd)
{
	parentHwnd = _parentHwnd;
	readDataEvent = CreateEventW(NULL, TRUE, TRUE, NULL);
	writeDataEvent = CreateEventW(NULL, TRUE, TRUE, NULL);
	SetEvent(readDataEvent);
	ResetEvent(writeDataEvent);

	threadHandle = (HANDLE)_beginthreadex(NULL, 0, DisplayDataThread, (LPVOID)this, NULL, NULL);
	if (INVALID_HANDLE_VALUE == threadHandle)
	{
		return FALSE;
	}
	
	ownHwnd = CreateDialogParamW(NULL, MAKEINTRESOURCEW(IDD_COLLECTPAGE), NULL, CCollectDlg::RunProc, (LPARAM)this);
	ShowWindow(ownHwnd, SW_SHOW);
	return TRUE;
}

BOOL CCollectDlg::End()
{
	if (NULL != childOption)
	{
		childOption->End(0);
		delete childOption;
		childOption = NULL;
	}

	DestroyWindow(ownHwnd);

	threadQuit = TRUE;
	SetEvent(writeDataEvent);
	SetEvent(readDataEvent);

	CloseHandle(readDataEvent);
	readDataEvent = INVALID_HANDLE_VALUE;
	CloseHandle(writeDataEvent);
	writeDataEvent = INVALID_HANDLE_VALUE;

	WaitForSingleObject(threadHandle, INFINITE);
	CloseHandle(threadHandle);
	threadHandle = INVALID_HANDLE_VALUE;	

	PostMessage(parentHwnd, WM_CHILDEND, (WPARAM)objectIndex, NULL);
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
		pointerThis->ownHwnd = hwnd;
		pointerThis->InitDialog(hwnd, (HWND)(wParam), lParam);
		break;

	case WM_COMMAND:
		pointerThis->Command(hwnd, (int)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam));
		break;

	case WM_CLOSE:
		pointerThis->End();
		break;

	case WM_SETOPTION:
		BOOL succFunc = pointerThis->childOption->GetOption(&(pointerThis->curSettingData));
		if (TRUE == succFunc && FALSE == pointerThis->showMsgData)
		{
			pointerThis->showMsgData = TRUE;
		}
		delete pointerThis->childOption;
		pointerThis->childOption = NULL;
		break;
	}

	return FALSE;
}

// ���̾�α� �ʱ�ȭ
BOOL CCollectDlg::InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	startAndSuspend = GetDlgItem(hwnd, IDC_STARTSUSPEND);
	SendMessageW(startAndSuspend, WM_SETTEXT, 0, (LPARAM)L"���");

	//Option ����
	//PostMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDC_OPTION, IDC_OPTION), NULL);
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
		if (FALSE == showMsgData)
		{
			SendMessageW(startAndSuspend, WM_SETTEXT, 0, (LPARAM)L"�Ͻ�����");
		}
		else
		{
			SendMessageW(startAndSuspend, WM_SETTEXT, 0, (LPARAM)L"����");
		}
		showMsgData = !showMsgData;
		break;

	case IDC_FILEOUT:
		SaveLog(hwnd);
		break;

	case IDC_OPTION:
		if (NULL != childOption)
		{
			break;
		}
		childOption = new COptionDlg();
		childOption->Start(ownHwnd, &curSettingData);
		break;
	}
}

void CCollectDlg::InsertData(MsgData _inputMsgData)
{
	WaitForSingleObject(readDataEvent, INFINITE);
	ResetEvent(writeDataEvent);
	inputMsg.push(_inputMsgData);
	SetEvent(writeDataEvent);
}

UINT CCollectDlg::DisplayDataThread(void * arg)
{
	((CCollectDlg*)arg)->DisplayData();
	return 0;
}

void CCollectDlg::DisplayData()
{
	while (true)
	{
		
		if (TRUE == threadQuit)
		{
			break;
		}

		if (inputMsg.size() == 0)
		{
			ResetEvent(writeDataEvent);
		}

		WaitForSingleObject(writeDataEvent, INFINITE);
		if (TRUE == threadQuit)
		{
			break;
		}

		ResetEvent(readDataEvent);
		MsgData inputMsgData;
		if (inputMsg.size() == 0)
		{
			SetEvent(readDataEvent);
			continue;
		}

		if (NULL == inputMsg.front().processName)
		{
			inputMsg.pop();
			SetEvent(readDataEvent);
			continue;
		}
		inputMsgData = inputMsg.front();
		inputMsg.pop();
		SetEvent(readDataEvent);

		if (FALSE == showMsgData || TRUE == threadQuit)
		{
			continue;
		}

		HWND listHwnd = GetDlgItem(ownHwnd, IDC_COLLECTLIST);
		std::wstring viewMsg = L"";
		std::wstring lineNum = std::to_wstring(countLine++);

		// Line number(8�ڸ���) �߰�
		viewMsg += L"<";
		for (int i = 0; i < 8 - lineNum.size(); i++)
			viewMsg += L"0";
		viewMsg += lineNum + L"> ";

		// Process Name �߰�
		viewMsg += L" [" + std::wstring(inputMsgData.processName);

		// Process ID �߰�
		viewMsg += L" (" + std::to_wstring(inputMsgData.processID) + L")";

		// Thread ID �߰�
		viewMsg += L"[" + std::to_wstring(inputMsgData.threadID) + L"]]";

		// Message Content �߰�
		if (NULL == wmTranslation[inputMsgData.msgCode])
		{
			viewMsg += L" [unknown]";
		}
		else
		{
			viewMsg += L" [" + std::wstring(wmTranslation[inputMsgData.msgCode]) + L"]";
		}

		// Message Code �߰�
		WCHAR buf[32] = { 0, };
		wsprintf(buf, L"0x%08X", inputMsgData.msgCode);
		viewMsg += L"(" + std::wstring(buf) + L")";

		switch (inputMsgData.hookType)
		{
		case MSG_CALLWND:
			// Msg type �߰�
			viewMsg += L" S";

			// wParam �߰�
			memset(buf, 0, 32);
			wsprintf(buf, L"0x%08X", inputMsgData.wParam);
			viewMsg += L"   wParam : " + std::wstring(buf);

			// lParam �߰�
			memset(buf, 0, 32);
			wsprintf(buf, L"0x%016X", inputMsgData.lParam);
			viewMsg += L"   lParam : " + std::wstring(buf);
			break;

		case MSG_CALLWNDRET:
			// Msg type �߰�
			viewMsg += L" R";

			// wParam �߰�
			memset(buf, 0, 32);
			wsprintf(buf, L"0x%08X", inputMsgData.wParam);
			viewMsg += L"   wParam : " + std::wstring(buf);

			// lParam �߰�
			memset(buf, 0, 32);
			wsprintf(buf, L"0x%016X", inputMsgData.lParam);
			viewMsg += L"   lParam : " + std::wstring(buf);
			break;

		case MSG_GETMSG:
			// Msg type �߰�
			viewMsg += L" P";

			// wParam �߰�
			memset(buf, 0, 32);
			wsprintf(buf, L"0x%08X", inputMsgData.wParam);
			viewMsg += L"   wParam : " + std::wstring(buf);

			// lParam �߰�
			memset(buf, 0, 32);
			wsprintf(buf, L"0x%016X", inputMsgData.lParam);
			viewMsg += L"   lParam : " + std::wstring(buf);
			break;

		case MSG_KEYBOARD:
			// wParam �߰� | virtual key code
			memset(buf, 0, 32);
			wsprintf(buf, L"0x%02X", inputMsgData.wParam);
			viewMsg += L"   vKeyCode : " + std::wstring(buf);

			// lParam �߰�
			memset(buf, 0, 32);
			wsprintf(buf, L"0x%016X", inputMsgData.lParam);
			viewMsg += L"   lParam: " + std::wstring(buf);
			break;

		case MSG_MOUSE:
			// wParam, mouse message�� ����(����)

			// lParam �߰�
			memset(buf, 0, 32);
			wsprintf(buf, L"x: %d, y: %d", LOWORD(inputMsgData.lParam), HIWORD(inputMsgData.lParam));
			viewMsg += L"   x: " + std::wstring(buf);
			break;

		case MSG_MSGFILTER:
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
			memset(buf, 0, 32);
			wsprintf(buf, L"0x%08X", inputMsgData.hwnd);
			//viewMsg += L"   hwnd : " + std::wstring(buf);
			break;

		default:
			break;
		}

		

		
		SendMessageW(listHwnd, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)viewMsg.data());
		LRESULT count = SendMessageW(listHwnd, LB_GETCOUNT, 0, 0);
		SendMessageW(listHwnd, LB_SETTOPINDEX, (WPARAM)count-1, (LPARAM)0);
	}
}

BOOL CCollectDlg::SaveLog(HWND hwnd)
{
	HWND ListBoxHwnd = GetDlgItem(hwnd, IDC_COLLECTLIST);
	LRESULT count = SendMessageW(ListBoxHwnd, LB_GETCOUNT, 0, 0);

	// ���� ��� ����
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
		MessageBoxW(NULL, L"���� ���", NULL, MB_OK);
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

	// UTF-8 ����(WCHAR ���)
	unsigned short mark = 0xFEFF;
	WriteFile(fileHandle, &mark, sizeof(mark), &returnLen, NULL);
	
	for (UINT i = 0; i < count; i++)
	{
		textLen = SendMessageW(ListBoxHwnd, LB_GETTEXTLEN, (WPARAM)(i), (LPARAM)NULL);
		curText = new WCHAR[textLen + 1];
		SendMessageW(ListBoxHwnd, LB_GETTEXT, (WPARAM)(i), (LPARAM)curText);
		curText[textLen] = L'\n';

		// ���� ����		
		WriteFile(fileHandle, curText, (textLen + 1) * sizeof(WCHAR), &returnLen, NULL);

		delete[] curText;
		returnLen = 0;
		textLen = 0;
		curText = NULL;
	}

	CloseHandle(fileHandle);

	return TRUE;
}
