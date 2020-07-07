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
	m_queueNotEmpty = CreateEventW(NULL, TRUE, TRUE, NULL);
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
	m_threadQuit = TRUE;

	SetEvent(m_queueNotEmpty);
	CloseHandle(m_queueNotEmpty);
	LeaveCriticalSection(&m_readWriteCS);

	DestroyWindow(m_collectListHwnd);
	DestroyWindow(m_ownHwnd);


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
		if (TRUE == succFunc)
		{
			if (FALSE == pointerThis->m_showMsgData)
			{
				PostMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDC_STARTSUSPEND, IDC_STARTSUSPEND), NULL);
			}

			// wParam, lParam Show/noShow
			if (TRUE == pointerThis->m_curSettingData.wParamShow)
			{
				pointerThis->m_lvCol.cx = 70;
				pointerThis->m_lvCol.pszText = L"wParam";
				ListView_SetColumn(pointerThis->m_collectListHwnd, colIndex::wParam, &(pointerThis->m_lvCol));
			}
			else
			{
				pointerThis->m_lvCol.cx = 0;
				pointerThis->m_lvCol.pszText = L"wParam";
				ListView_SetColumn(pointerThis->m_collectListHwnd, colIndex::wParam, &(pointerThis->m_lvCol));
			}

			if (TRUE == pointerThis->m_curSettingData.lParamShow)
			{
				pointerThis->m_lvCol.cx = 70;
				pointerThis->m_lvCol.pszText = L"lParam";
				ListView_SetColumn(pointerThis->m_collectListHwnd, colIndex::lParam, &(pointerThis->m_lvCol));
			}
			else
			{
				pointerThis->m_lvCol.cx = 0;
				pointerThis->m_lvCol.pszText = L"lParam";
				ListView_SetColumn(pointerThis->m_collectListHwnd, colIndex::lParam, &(pointerThis->m_lvCol));
			}
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
	InitCommonControls(); // Force the common controls DLL to be loaded.

	// window is a handle to my window that is already created.
	m_collectListHwnd = CreateWindowExW(
		0, (LPWSTR)WC_LISTVIEWW, NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_SHOWSELALWAYS | LVS_REPORT,
		11, 38, 776, 255,
		hwnd, NULL, NULL, NULL);

	memset(&m_lvCol, 0, sizeof(LVCOLUMNW));
	m_lvCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	m_lvCol.iSubItem = 0;
	m_lvCol.fmt = LVCFMT_LEFT;

	m_lvCol.cx = 70;
	m_lvCol.pszText = L"Index";
	ListView_InsertColumn(m_collectListHwnd, (int)colIndex::index, &m_lvCol);

	m_lvCol.cx = 80;
	m_lvCol.pszText = L"Process Name";
	ListView_InsertColumn(m_collectListHwnd, (int)colIndex::pName, &m_lvCol);

	m_lvCol.cx = 60;
	m_lvCol.pszText = L"PID";
	ListView_InsertColumn(m_collectListHwnd, (int)colIndex::pID, &m_lvCol);

	m_lvCol.cx = 60;
	m_lvCol.pszText = L"TID";
	ListView_InsertColumn(m_collectListHwnd, (int)colIndex::tID, &m_lvCol);

	m_lvCol.cx = 60;
	m_lvCol.pszText = L"Msg Content";
	ListView_InsertColumn(m_collectListHwnd, (int)colIndex::msgContent, &m_lvCol);

	m_lvCol.cx = 60;
	m_lvCol.pszText = L"Msg Code";
	ListView_InsertColumn(m_collectListHwnd, (int)colIndex::msgCode, &m_lvCol);

	m_lvCol.cx = 20;
	m_lvCol.pszText = L"Msg Type";
	ListView_InsertColumn(m_collectListHwnd, (int)colIndex::msgType, &m_lvCol);

	m_lvCol.cx = 70;
	m_lvCol.pszText = L"wParam";
	ListView_InsertColumn(m_collectListHwnd, (int)colIndex::wParam, &m_lvCol);

	m_lvCol.cx = 70;
	m_lvCol.pszText = L"lParam";
	ListView_InsertColumn(m_collectListHwnd, (int)colIndex::lParam, &m_lvCol);

	m_lvCol.cx = 60;
	m_lvCol.pszText = L"Caption";
	ListView_InsertColumn(m_collectListHwnd, (int)colIndex::caption, &m_lvCol);

	m_lvCol.cx = 60;
	m_lvCol.pszText = L"Class Name";
	ListView_InsertColumn(m_collectListHwnd, (int)colIndex::className, &m_lvCol);

	m_lvCol.cx = 60;
	m_lvCol.pszText = L"Style";
	ListView_InsertColumn(m_collectListHwnd, (int)colIndex::style, &m_lvCol);

	m_lvCol.cx = 100;
	m_lvCol.pszText = L"Detail";
	ListView_InsertColumn(m_collectListHwnd, (int)colIndex::detail, &m_lvCol);

	memset(&m_lvItem, 0, sizeof(LVITEMW));
	m_lvItem.mask = LVIF_TEXT;   // Text Style
	m_lvItem.cchTextMax = 256; // Max size of test
	m_lvItem.iSubItem = 0;       // col
	m_lvItem.iItem = 0;     // row

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
			SendMessageW(GetDlgItem(hwnd, IDC_STARTSUSPEND), WM_SETTEXT, 0, (LPARAM)L"일시정지");
		}
		else
		{
			SendMessageW(GetDlgItem(hwnd, IDC_STARTSUSPEND), WM_SETTEXT, 0, (LPARAM)L"시작");
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
		if (TRUE == m_showMsgData)
		{
			m_inputMsg.push(_inputMsgData);
		}
		LeaveCriticalSection(&m_readWriteCS);
		SetEvent(m_queueNotEmpty);
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
	m_saveDataList.reserve(1024);
	while (true)
	{
		HWND listHwnd = GetDlgItem(m_ownHwnd, IDC_COLLECTLIST);
		if (TRUE == m_threadQuit)
		{
			break;
		}

		EnterCriticalSection(&m_readWriteCS);
		std::swap(inputMsgQeuue, m_inputMsg);
		if (inputMsgQeuue.empty() && m_inputMsg.empty())
		{
			ResetEvent(m_queueNotEmpty);
		}
		LeaveCriticalSection(&m_readWriteCS);

		if (TRUE == m_threadQuit)
		{
			break;
		}

		WaitForSingleObject(m_queueNotEmpty, INFINITE);

		while (!inputMsgQeuue.empty())
		{
			SaveData curSaveData;
			MsgData inputMsgData = inputMsgQeuue.front();
			inputMsgQeuue.pop();

			if (65536 <= inputMsgData.msgCode)
			{
				continue;
			}

			if (false == m_curSettingData.msgOptions[inputMsgData.msgCode]
				&& false == m_curSettingData.otherMsgShow)
			{
				continue;
			}

			if (TRUE == m_threadQuit)
			{
				break;
			}

			WCHAR buf[32] = { 0, };

			// Line number(8자리수) 추가
			wsprintf(buf, L"<%08d>", m_listRowIndex);
			curSaveData.index = std::wstring(buf);

			// Process Name 추가
			curSaveData.pName = std::wstring(inputMsgData.processName);

			// Process ID 추가
			curSaveData.pID = std::to_wstring(inputMsgData.processID);

			// Thread ID 추가
			curSaveData.tID = std::to_wstring(inputMsgData.threadID);

			// Message Content 추가
			if (NULL == wmToString[inputMsgData.msgCode])
			{
				curSaveData.msgContent = L"unknown";
			}
			else
			{
				curSaveData.msgContent = std::wstring(wmToString[inputMsgData.msgCode]);
			}

			// Message Code 추가
			memset(buf, 0, 32);
			wsprintf(buf, L"%d", inputMsgData.msgCode);
			curSaveData.msgCode = std::wstring(buf);

			switch (inputMsgData.hookType)
			{
			case MSG_CALLWND:
				// Msg type 추가
				curSaveData.msgType = L"S";
				break;

			case MSG_CALLWNDRET:
				// Msg type 추가
				curSaveData.msgType = L"R";
				break;

			case MSG_GETMSG:
				// Msg type 추가
				curSaveData.msgType = L"P";
				break;
			}

			// wParam 추가
			memset(buf, 0, 32);
			wsprintf(buf, L"0x%016X", inputMsgData.wParam);
			curSaveData.wParam = std::wstring(buf);

			// lParam 추가
			memset(buf, 0, 32);
			wsprintf(buf, L"0x%016X", inputMsgData.lParam);
			curSaveData.lParam = std::wstring(buf);

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
				WCHAR longBuf[MAX_PATH] = { 0, };
				GetWindowTextW((HWND)inputMsgData.hwnd, longBuf, MAX_PATH);
				if (longBuf[0] != '\0')
				{
					curSaveData.caption += std::wstring(longBuf);
				}

				memset(longBuf, 0, MAX_PATH);
				GetClassNameW((HWND)inputMsgData.hwnd, longBuf, MAX_PATH);
				if (longBuf[0] != '\0')
				{
					curSaveData.className += std::wstring(longBuf);
				}

				WNDCLASSW wndClass;
				GetClassInfoW((HINSTANCE)inputMsgData.hInstance, longBuf, &wndClass);

				AddStyleString(&curSaveData.style, wndClass.style);
				break;
			}

			std::wstring optionData;
			size_t front = 0;
			switch (inputMsgData.msgCode)
			{
			case WM_CREATE:
			case WM_WINDOWPOSCHANGED:
				curSaveData.detail += std::wstring(inputMsgData.otherData);
				break;

			case WM_SETTEXT:
				curSaveData.detail += L"Text:" + std::wstring(inputMsgData.otherData);
				break;

			case WM_STYLECHANGED:
				optionData = std::wstring(inputMsgData.otherData);
				front = optionData.find(L':');
				curSaveData.detail += L"Old";
				AddStyleString(&curSaveData.detail, std::stoi(optionData.substr(front + 1)), 2);

				curSaveData.detail += L"  New";
				front = optionData.find_last_of(L':');
				AddStyleString(&curSaveData.detail, std::stoi(optionData.substr(front + 1)), 2);
				break;

			case WM_SHOWWINDOW:
				curSaveData.detail += L"WindowShow:";
				curSaveData.detail += inputMsgData.wParam ? L"TRUE" : L"FALSE";

				switch (inputMsgData.lParam)
				{
				case SW_OTHERUNZOOM:
					curSaveData.detail += L" Status:SW_OTHERUNZOOM";
					break;

				case SW_OTHERZOOM:
					curSaveData.detail += L" Status:SW_OTHERZOOM";
					break;

				case SW_PARENTCLOSING:
					curSaveData.detail += L" Status:SW_PARENTCLOSING";
					break;

				case SW_PARENTOPENING:
					curSaveData.detail += L" Status:SW_PARENTOPENING";
					break;
				}
				break;

			case WM_NCACTIVATE:
				curSaveData.detail += L" IconDraw:";
				curSaveData.detail += inputMsgData.wParam ? L"TRUE" : L"FALSE";
				break;

			case WM_SIZE:
				switch (inputMsgData.wParam)
				{
				case SIZE_MAXHIDE:
					curSaveData.detail += L" ResizingType:SIZE_MAXHIDE";
					break;

				case SIZE_MAXIMIZED:
					curSaveData.detail += L" ResizingType:SIZE_MAXIMIZED";
					break;

				case SIZE_MAXSHOW:
					curSaveData.detail += L" ResizingType:SIZE_MAXSHOW";
					break;

				case SIZE_MINIMIZED:
					curSaveData.detail += L" ResizingType:SIZE_MINIMIZED";
					break;

				case SIZE_RESTORED:
					curSaveData.detail += L" ResizingType:SIZE_RESTORED";
					break;
				}

				curSaveData.detail += L" Width:" + std::to_wstring(LOWORD(inputMsgData.lParam));
				curSaveData.detail += L" Height:" + std::to_wstring(HIWORD(inputMsgData.lParam));
				break;

			case WM_MOVE:
				curSaveData.detail += L" CX:" + std::to_wstring(LOWORD(inputMsgData.lParam));
				curSaveData.detail += L" CY:" + std::to_wstring(HIWORD(inputMsgData.lParam));
				break;
			}


			if (TRUE == m_threadQuit)
			{
				break;
			}

			m_lvItem.iItem = m_listRowIndex++;     // choose item

			m_lvItem.iSubItem = (int)colIndex::index;
			m_lvItem.pszText = &curSaveData.index[0];
			ListView_InsertItem(m_collectListHwnd, (LPARAM)&m_lvItem);

			m_lvItem.iSubItem = (int)colIndex::pName;
			m_lvItem.pszText = &curSaveData.pName[0];
			ListView_SetItem(m_collectListHwnd, (LPARAM)&m_lvItem);

			m_lvItem.iSubItem = (int)colIndex::pID;
			m_lvItem.pszText = &curSaveData.pID[0];
			ListView_SetItem(m_collectListHwnd, (LPARAM)&m_lvItem);

			m_lvItem.iSubItem = (int)colIndex::tID;
			m_lvItem.pszText = &curSaveData.tID[0];
			ListView_SetItem(m_collectListHwnd, (LPARAM)&m_lvItem);

			m_lvItem.iSubItem = (int)colIndex::msgContent;
			m_lvItem.pszText = &curSaveData.msgContent[0];
			ListView_SetItem(m_collectListHwnd, (LPARAM)&m_lvItem);

			m_lvItem.iSubItem = (int)colIndex::msgCode;
			m_lvItem.pszText = &curSaveData.msgCode[0];
			ListView_SetItem(m_collectListHwnd, (LPARAM)&m_lvItem);

			m_lvItem.iSubItem = (int)colIndex::msgType;
			m_lvItem.pszText = &curSaveData.msgType[0];
			ListView_SetItem(m_collectListHwnd, (LPARAM)&m_lvItem);

			m_lvItem.iSubItem = (int)colIndex::wParam;
			m_lvItem.pszText = &curSaveData.wParam[0];
			ListView_SetItem(m_collectListHwnd, (LPARAM)&m_lvItem);

			m_lvItem.iSubItem = (int)colIndex::lParam;
			m_lvItem.pszText = &curSaveData.lParam[0];
			ListView_SetItem(m_collectListHwnd, (LPARAM)&m_lvItem);

			m_lvItem.iSubItem = (int)colIndex::caption;
			m_lvItem.pszText = &curSaveData.caption[0];
			ListView_SetItem(m_collectListHwnd, (LPARAM)&m_lvItem);

			m_lvItem.iSubItem = (int)colIndex::className;
			m_lvItem.pszText = &curSaveData.className[0];
			ListView_SetItem(m_collectListHwnd, (LPARAM)&m_lvItem);

			m_lvItem.iSubItem = (int)colIndex::style;
			m_lvItem.pszText = &curSaveData.style[0];
			ListView_SetItem(m_collectListHwnd, (LPARAM)&m_lvItem);

			m_lvItem.iSubItem = (int)colIndex::detail;
			m_lvItem.pszText = &curSaveData.detail[0];
			ListView_SetItem(m_collectListHwnd, (LPARAM)&m_lvItem);

			ListView_Scroll(m_collectListHwnd, 0, m_listRowIndex - 1);

			m_saveDataList.push_back(curSaveData);
		}
	}
}

void CCollectDlg::AddStyleString(std::wstring *_inputString, UINT _inputStyle, int _type)
{
	std::wstring styleString = L"";

	if (WS_BORDER & _inputStyle)
	{
		styleString += L"WS_BORDER|";
	}

	if (WS_CAPTION & _inputStyle)
	{
		styleString += L"WS_CAPTION|";
	}

	if (WS_CHILD & _inputStyle)
	{
		styleString += L"WS_CHILD|";
	}

	if (WS_CHILDWINDOW & _inputStyle)
	{
		styleString += L"WS_CHILDWINDOW|";
	}

	if (WS_CLIPCHILDREN & _inputStyle)
	{
		styleString += L"WS_CLIPCHILDREN|";
	}

	if (WS_CLIPSIBLINGS & _inputStyle)
	{
		styleString += L"WS_CLIPSIBLINGS|";
	}

	if (WS_DISABLED & _inputStyle)
	{
		styleString += L"WS_DISABLED|";
	}

	if (WS_DLGFRAME & _inputStyle)
	{
		styleString += L"WS_DLGFRAME|";
	}

	if (WS_GROUP & _inputStyle)
	{
		styleString += L"WS_GROUP|";
	}

	if (WS_HSCROLL & _inputStyle)
	{
		styleString += L"WS_HSCROLL|";
	}

	if (WS_ICONIC & _inputStyle)
	{
		styleString += L"WS_ICONIC|";
	}

	if (WS_MAXIMIZE & _inputStyle)
	{
		styleString += L"WS_MAXIMIZE|";
	}

	if (WS_MAXIMIZEBOX & _inputStyle)
	{
		styleString += L"WS_MAXIMIZEBOX|";
	}

	if (WS_MINIMIZE & _inputStyle)
	{
		styleString += L"WS_MINIMIZE|";
	}

	if (WS_MINIMIZEBOX & _inputStyle)
	{
		styleString += L"WS_MINIMIZEBOX|";
	}

	if (WS_OVERLAPPED & _inputStyle)
	{
		styleString += L"WS_OVERLAPPED|";
	}

	if (WS_OVERLAPPEDWINDOW & _inputStyle)
	{
		styleString += L"WS_OVERLAPPEDWINDOW|";
	}

	if (WS_POPUP & _inputStyle)
	{
		styleString += L"WS_POPUP|";
	}

	if (WS_POPUPWINDOW & _inputStyle)
	{
		styleString += L"WS_POPUPWINDOW|";
	}

	if (WS_SIZEBOX & _inputStyle)
	{
		styleString += L"WS_SIZEBOX|";
	}

	if (WS_SYSMENU & _inputStyle)
	{
		styleString += L"WS_SYSMENU|";
	}

	if (WS_TABSTOP & _inputStyle)
	{
		styleString += L"WS_TABSTOP|";
	}

	if (WS_THICKFRAME & _inputStyle)
	{
		styleString += L"WS_THICKFRAME|";
	}

	if (WS_TILED & _inputStyle)
	{
		styleString += L"WS_TILED|";
	}

	if (WS_TILEDWINDOW & _inputStyle)
	{
		styleString += L"WS_TILEDWINDOW|";
	}

	if (WS_VISIBLE & _inputStyle)
	{
		styleString += L"WS_VISIBLE|";
	}

	if (WS_VSCROLL & _inputStyle)
	{
		styleString += L"WS_VSCROLL";
	}

	if (!styleString.empty())
	{
		if (L'|' == styleString.back())
		{
			styleString.pop_back();
		}

		if (1 == _type)
		{
			*_inputString += styleString;
		}
		else if (2 == _type)
		{
			*_inputString += L"Style: " + styleString;
		}
	}
}

BOOL CCollectDlg::SaveLog(HWND hwnd)
{
	//HWND ListBoxHwnd = GetDlgItem(hwnd, IDC_COLLECTLIST);
	//LRESULT count = SendMessageW(ListBoxHwnd, LB_GETCOUNT, 0, 0);

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

	// UNICODE(Little Endian) 설정(WCHAR 출력)
	//unsigned char mark[2] = { 0xFF, 0xFE };
	// UTF-8
	//unsigned char mark[3] = { 0xEF, 0xBB, 0xBF };
	//WriteFile(fileHandle, &mark, 2, &returnLen, NULL);
	std::string header = "index,pName,pID,tID,msgContent,msgCode,msgType,wParam,lParam,caption,className,style,detail\n";
	WriteFile(fileHandle, header.data(), header.size(), &returnLen, NULL);

	ULONGLONG count = m_saveDataList.size();
	for (UINT i = 0; i < count; i++)
	{
		std::string ouputTxtA = "";
		std::wstring outputTxtW = L"";
		outputTxtW += m_saveDataList[i].index + L",";
		outputTxtW += m_saveDataList[i].pName + L",";
		outputTxtW += m_saveDataList[i].pID + L",";
		outputTxtW += m_saveDataList[i].tID + L",";
		outputTxtW += m_saveDataList[i].msgContent + L",";
		outputTxtW += m_saveDataList[i].msgCode + L",";
		outputTxtW += m_saveDataList[i].msgType + L",";
		outputTxtW += m_saveDataList[i].wParam + L",";
		outputTxtW += m_saveDataList[i].lParam + L",";
		outputTxtW += m_saveDataList[i].caption + L",";
		outputTxtW += m_saveDataList[i].className + L",";
		outputTxtW += m_saveDataList[i].style + L",";
		outputTxtW += m_saveDataList[i].detail + L"\n";

		ouputTxtA = wcs2mbs(outputTxtW, std::locale("kor"));
		// 파일 쓰기
		WriteFile(fileHandle, ouputTxtA.data(), ouputTxtA.size(), &returnLen, NULL);

		delete[] curText;
		returnLen = 0;
		textLen = 0;
		curText = NULL;
	}

	CloseHandle(fileHandle);
	MessageBoxW(NULL, L"저장 완료", L"Success", MB_OK);

	return TRUE;
}

std::string CCollectDlg::wcs2mbs(std::wstring const & str, std::locale const & loc)
{
	typedef std::codecvt <wchar_t, char, std::mbstate_t> codecvt_t;
	std::codecvt <wchar_t, char, std::mbstate_t> const& codecvt = std::use_facet<codecvt_t>(loc);
	std::mbstate_t state = std::mbstate_t();
	std::vector<char> buf((str.size() + 1) * codecvt.max_length());
	wchar_t const* in_next = str.c_str();
	char* out_next = &buf[0];
	codecvt_t::result r = codecvt.out(state, str.c_str(), str.c_str() + str.size(), in_next, &buf[0], &buf[0] + buf.size(), out_next);
	return std::string(&buf[0]);
}
