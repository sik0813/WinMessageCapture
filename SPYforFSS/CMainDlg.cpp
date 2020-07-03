#include "CMainDlg.h"
#include <ShlObj.h>

CMainDlg* CMainDlg::procAccess = 0;
BOOL CMainDlg::quitThread = FALSE;

CMainDlg::CMainDlg()
{
	procAccess = this;
	for (int i = 0; i < THREAD_SIZE; i++)
	{
		m_threadHandles[i] = INVALID_HANDLE_VALUE;
	}
}

CMainDlg::~CMainDlg()
{
}

static INT_PTR CALLBACK RunProcMain(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return CMainDlg::procAccess->RunProc(hwnd, uMsg, wParam, lParam);
}


BOOL CMainDlg::InitTrasmission()
{
	m_hPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, THREAD_SIZE);
		
	SECURITY_DESCRIPTOR sd;
	memset(&sd, 0, sizeof(SECURITY_DESCRIPTOR));
	BOOL successFunc = InitializeSecurityDescriptor(
		&sd,
		SECURITY_DESCRIPTOR_REVISION);
	if (!successFunc)
	{
		wprintf(L"InitializeSecurityDescriptor Fail \n");
		return FALSE;
	}

	successFunc = SetSecurityDescriptorDacl(
		&sd,
		TRUE, // TRUE 세번째 인자 사용
		NULL, // NULL allows all access to the objeclt
		FALSE);
	if (!successFunc)
	{
		wprintf(L"SetSecurityDescriptorDacl Fail \n");
		return FALSE;
	}

	SECURITY_ATTRIBUTES sa;
	memset(&sa, 0, sizeof(SECURITY_ATTRIBUTES));
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = &sd;

	typedef BOOL(WINAPI *LPIsUserAnAdmin) (void);
	LPIsUserAnAdmin IsUserAnAdmin = NULL;
	CHAR szPath[MAX_PATH] = { 0, };
	BOOL isAdmin = FALSE;

	IsUserAnAdmin = (LPIsUserAnAdmin)GetProcAddress(GetModuleHandleW(L"shell32"), "IsUserAnAdmin");
	if (IsUserAnAdmin != NULL)
	{
		if (TRUE == IsUserAnAdmin())
		{
			// 관리자 권한
			isAdmin = TRUE;
		}
	}

	m_ioKeys.ioPort = m_hPort;
	m_ioKeys.pipeHandle = CreateNamedPipeW(m_pipeName,
		PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
		PIPE_WAIT | PIPE_READMODE_MESSAGE | PIPE_TYPE_MESSAGE,
		PIPE_UNLIMITED_INSTANCES,
		0,
		0,
		INFINITE,
		TRUE == isAdmin ? &sa : NULL
	);

	// overlaped read Event Handle
	m_eventHandle = CreateEventW(NULL, FALSE, TRUE, NULL);
	m_overLap.hEvent = m_eventHandle;

	m_ioKeys.ov = &m_overLap;
	ConnectNamedPipe(m_ioKeys.pipeHandle, m_ioKeys.ov);

	if (INVALID_HANDLE_VALUE == m_ioKeys.pipeHandle)
	{
		return FALSE;
	}
	if (NULL == CreateIoCompletionPort(m_ioKeys.pipeHandle, m_hPort, (ULONG_PTR)&m_ioKeys, THREAD_SIZE))
	{
		wprintf(L"IOCP connect Error");
		return FALSE;
	}

	for (int i = 0; i < THREAD_SIZE; i++)
	{
		m_threadHandles[i] = (HANDLE)_beginthreadex(NULL, 0, RecvDataThread, (void*)m_hPort, NULL, NULL);
		if (INVALID_HANDLE_VALUE == m_threadHandles[i])
		{
			return FALSE;
		}
	}
	
	return TRUE;
}


UINT WINAPI CMainDlg::RecvDataThread(void *arg)
{
	procAccess->RecvData((HANDLE)arg);
	return 0;
}



BOOL CMainDlg::RecvData(HANDLE _portHandle)
{
	HANDLE portHandle = _portHandle;
	IoKey *nowKey = NULL;
	WCHAR cBuf[BUFSIZ] = { 0, };
	LPOVERLAPPED ov = NULL;

	HANDLE readEvent = CreateEventW(NULL, FALSE, TRUE, NULL);
	OVERLAPPED readFileOverlapped;
	memset(&readFileOverlapped, 0, sizeof(OVERLAPPED));
	readFileOverlapped.hEvent = readEvent;

	while (true)
	{
		if (TRUE == quitThread)
		{
			break;
		}

		BOOL succFunc = FALSE;
		DWORD readLen = 0, pipeRead = 0;

		//GETIOCP_TIMEOUT: 5sec
		succFunc = GetQueuedCompletionStatus(portHandle, &readLen, (PULONG_PTR)&nowKey, &ov, GETIOCP_TIMEOUT);
		if (FALSE == succFunc)
		{
			continue;
		}

		succFunc = ReadFile(nowKey->pipeHandle, &(nowKey->msgDataBuf), sizeof(MsgData), &pipeRead, &readFileOverlapped);
		if (!succFunc)
		{
			DWORD readErr = GetLastError();
			if (readErr == ERROR_IO_PENDING)
			{
				DWORD waitReturn = WaitForSingleObject(readEvent, 3000);
				if (WAIT_FAILED == waitReturn)
				{
					wprintf(L"Fail ReadFile GLE=%d", GetLastError());
				}
				else if (WAIT_OBJECT_0 == waitReturn)
				{
					Display(nowKey->msgDataBuf);
				}
			}
			else
			{
				wprintf(L"Fail Pending Error GLE=%d", GetLastError());
			}
		}
		else
		{
			Display(nowKey->msgDataBuf);
		}

		FlushFileBuffers(nowKey->pipeHandle);
		DisconnectNamedPipe(nowKey->pipeHandle);
		ConnectNamedPipe(nowKey->pipeHandle, nowKey->ov);
	}

	return TRUE;
}


void CMainDlg::Display(MsgData &_inputMsgData)
{
	MsgData inputMsgData = _inputMsgData;
	std::wstring recvProcessName = std::wstring(inputMsgData.processName);

	EnterCriticalSection(&m_deleteDlg);
	for (int i = 0; i < m_curCollectDlg[recvProcessName].size(); i++)
	{
		(m_curCollectDlg[recvProcessName][i].second)->InsertData(inputMsgData);
	}
	LeaveCriticalSection(&m_deleteDlg);
	//SetEvent(readEndEvent);
}


BOOL CMainDlg::Start(HINSTANCE _parentInstance)
{
	m_parentInstance = _parentInstance;
	InitializeCriticalSection(&m_deleteDlg);

	BOOL succFunc = InitTrasmission();
	if (FALSE == succFunc)
	{
		MessageBoxW((HWND)_parentInstance, L"PIPE make Error", L"ERROR", MB_OK);
	}

	m_listWriteDone = CreateEventW(NULL, TRUE, TRUE, m_writeDoneEvent);
	ResetEvent(m_listWriteDone);

	m_hMapFile = CreateFileMappingW(
		INVALID_HANDLE_VALUE,    // use paging file
		NULL,                    // default security
		PAGE_READWRITE,          // read/write access
		0,                       // maximum object size (high-order DWORD)
		BUF_SIZE,                // maximum object size (low-order DWORD)
		m_sharedMemName);                 // name of mapping object
	if (m_hMapFile == NULL)
	{
		wprintf(L"Could not create file mapping object (%d).\n", GetLastError());
		return FALSE;
	}

	m_pBuf = (LPWSTR)MapViewOfFile(m_hMapFile,   // handle to map object
		FILE_MAP_ALL_ACCESS, // read/write permission
		0,
		0,
		BUF_SIZE);
	if (m_pBuf == NULL)
	{
		wprintf(L"Could not map view of file (%d).\n", GetLastError());
		CloseHandle(m_hMapFile);
		return FALSE;
	}
	memset(m_pBuf, 0, BUF_SIZE);

	return TRUE;
}

BOOL CMainDlg::Show()
{
	DialogBoxParamW(m_parentInstance, MAKEINTRESOURCEW(IDD_MAINPAGE), NULL, ::RunProcMain, NULL);//(LPARAM)this);
	return 0;
}

BOOL CMainDlg::End()
{
	WCHAR endMsg[4];
	memset(endMsg, 0xF, 8);
	memcpy(m_pBuf, endMsg, 8);
	SetEvent(m_listWriteDone);

	UnmapViewOfFile(m_pBuf);
	CloseHandle(m_listWriteDone);
	CloseHandle(m_hMapFile);

	quitThread = TRUE;	

	CloseHandle(m_ioKeys.pipeHandle);
	CloseHandle(m_ioKeys.ov->hEvent);

	CloseHandle(m_hPort);

	WaitForMultipleObjects(THREAD_SIZE, m_threadHandles, TRUE, INFINITE);

	for (int i = 0; i < THREAD_SIZE; i++)
	{
		CloseHandle(m_threadHandles[i]);
	}

	DeleteCriticalSection(&m_deleteDlg);

	return 0;
}


INT_PTR CALLBACK CMainDlg::RunProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		InitDialog(hwnd, (HWND)(wParam), lParam);
		break;

	case WM_COMMAND:
		Command(hwnd, (int)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam));
		break;

	case WM_CHILDEND:
		EndCollect((int)wParam);
		break;

	case WM_LISTEDITDONE:
		UpdateSendList();
		break;
	}

	return FALSE;
}

void CMainDlg::Command(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	BOOL SuccessFuc = FALSE;
	switch (id)
	{
	case IDOK: break;
	case IDCANCEL:
		EndDialog(hwnd, id);
		//DestroyWindow(curDlgHwnd);
		break;

	case IDC_PIDLIST:
		if (codeNotify == LBN_DBLCLK)
		{
			InsertClickProcess(hwndCtl, hwnd);
		}
		break;

	case IDC_REFRESH:
		RefreshList(hwnd);
		break;

	case IDC_WATCH:
		StartCollect(hwnd);
		break;
	}
}

// 다이얼로그 초기화
BOOL CMainDlg::InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	HBITMAP hBitmap = LoadBitmapW(m_parentInstance, MAKEINTRESOURCE(IDB_REFRESH));
	HWND hDlg = GetDlgItem(hwnd, IDC_REFRESH);
	SendMessageW(hDlg, BM_SETIMAGE, 0, (LPARAM)hBitmap);
	m_ownHwnd = hwnd;
	PostMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDC_REFRESH, IDC_REFRESH), NULL);
	return TRUE;
}

// 현재 프로세스 리스트 업데이트
BOOL CMainDlg::RefreshList(HWND hwnd)
{
	SetWindowTextW(GetDlgItem(hwnd, IDC_SELECTS), L"");

	DWORD processlist[1024] = { 0, }, listByte = 0;
	BOOL sucessFunc = EnumProcesses(processlist, sizeof(processlist), &listByte);
	if (NULL == sucessFunc)
	{
		return FALSE;
	}

	DWORD listLen = listByte / sizeof(DWORD);
	HWND listHwnd = GetDlgItem(hwnd, IDC_PIDLIST);
	SendMessage(listHwnd, LB_RESETCONTENT, 0, 0); // reset 
	for (unsigned int i = 0; i < listLen; i++)
	{
		if (processlist[i] != 0)
		{
			WCHAR m_processName[MAX_PATH] = { 0, };
			// Get a handle to the process.

			HANDLE processHandle = OpenProcess(
				PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
				FALSE,
				processlist[i]);

			// Get the process name.
			if (NULL != processHandle)
			{
				DWORD successFunc = GetModuleFileNameEx( // GetModuleBaseNameW : 실행 파일명
					processHandle,
					0,
					m_processName,
					sizeof(m_processName) / sizeof(WCHAR));
				if (NULL != successFunc)
				{
					WCHAR addItem[MAX_PATH] = { 0, };
					wsprintf(addItem, L"%s(%d)", wcsrchr(m_processName, L'\\') + 1, processlist[i]);
					ListBox_AddString(listHwnd, addItem);
				}
			}
			CloseHandle(processHandle);
		}
	}
	return TRUE;
}

void CMainDlg::InsertClickProcess(HWND hwndCtl, HWND hwnd)
{
	LRESULT count = SendMessageW(hwndCtl, LB_GETCURSEL, 0, 0);
	WCHAR inputPID[MAX_PATH] = { 0, };
	SendMessageW(hwndCtl, LB_GETTEXT, (WPARAM)(count), (LPARAM)inputPID);

	std::wstring parsedInputPID(inputPID);
	parsedInputPID = parsedInputPID.substr(0, parsedInputPID.find(L"("));

	std::wstring currentContent = L"";
	HWND editHwnd = GetDlgItem(hwnd, IDC_SELECTS);
	int editSelectslength = GetWindowTextLengthW(editHwnd);
	if (0 != editSelectslength)
	{
		currentContent.resize(editSelectslength);
		GetWindowTextW(editHwnd, &currentContent[0], editSelectslength + 1);
	}
	std::wstring output = parsedInputPID + L'|' + currentContent;

	SetWindowTextW(editHwnd, output.c_str());
}

BOOL CMainDlg::StartCollect(HWND hwnd)
{
	std::vector<std::wstring> runList;
	std::wstring currentContent;
	HWND editHwnd = GetDlgItem(hwnd, IDC_SELECTS);
	DWORD editBoxLen = GetWindowTextLengthW(editHwnd);
	currentContent.resize(editBoxLen);

	GetWindowTextW(editHwnd, &currentContent[0], 1000);
	while (true)
	{
		size_t subLocation = currentContent.find(L"|");
		std::wstring subString = currentContent.substr(0, subLocation);
		currentContent = currentContent.substr(subLocation + 1);
		if (0 == subString.size())
		{
			break;
		}

		for (int i = 0; i < subString.size(); i++)
		{
			subString[i] = towlower(subString[i]);
		}
		
		runList.push_back(subString);
		//runList[subString] = TRUE;

		if (std::string::npos == subLocation)
		{
			break;
		}
	}

	if (0 == runList.size()) return 0;

	CCollectDlg* newCCollectDlg = new CCollectDlg(m_curChildIndex);

	for (auto x : runList)
	{
		m_curCollectDlg[x].push_back(std::pair<int, CCollectDlg*>(m_curChildIndex, newCCollectDlg));
	}

	newCCollectDlg->Start(hwnd, runList);
	m_curChildIndex++;

	PostMessageW(m_ownHwnd, WM_LISTEDITDONE, NULL, NULL);

	return 0;
}

BOOL CMainDlg::EndCollect(int eraseIndex)
{
	BOOL classDeleteDone = FALSE;

	EnterCriticalSection(&m_deleteDlg);
	for (auto i = m_curCollectDlg.begin(); i != m_curCollectDlg.end(); i++)
	{
		size_t nextLoopSize = i->second.size();
		for (int j = 0; j < nextLoopSize; j++)
		{
			if ((i->second)[j].first == eraseIndex)
			{
				if (FALSE == classDeleteDone)
				{
					delete (i->second)[j].second;
					classDeleteDone = TRUE;
				}
				(i->second).erase((i->second).begin() + j);
				break;
			}
		}
	}
	LeaveCriticalSection(&m_deleteDlg);

	PostMessageW(m_ownHwnd, WM_LISTEDITDONE, NULL, NULL);
	return TRUE;
}

void CMainDlg::UpdateSendList()
{
	std::wstring sendList = L"";
	for (auto i = m_curCollectDlg.begin(); i != m_curCollectDlg.end(); i++)
	{
		if (0 < i->second.size()) // 0 보다 큰 항목에 대하여 list 구성
		{
			sendList += i->first + L"|";
		}
	}
	memcpy(m_pBuf, sendList.data(), sendList.size() * sizeof(WCHAR));
	SetEvent(m_listWriteDone);
	ResetEvent(m_listWriteDone);
}
