#include "CMainDlg.h"

CMainDlg* CMainDlg::procAccess = 0;
BOOL CMainDlg::quitThread = FALSE;

CMainDlg::CMainDlg()
{
	procAccess = this;
	deleteDlg = CreateEventW(NULL, TRUE, TRUE, NULL);
	SetEvent(deleteDlg);

	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	pipeSize = sysinfo.dwNumberOfProcessors;
	threadSize = pipeSize;
}

CMainDlg::~CMainDlg()
{
	CloseHandle(deleteDlg);
}

static INT_PTR CALLBACK RunProcMain(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return CMainDlg::procAccess->RunProc(hwnd, uMsg, wParam, lParam);
}


BOOL CMainDlg::InitTrasmission()
{
	hPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, threadSize);

	ioKeys = new IoKey[pipeSize];
	overLaps = new OVERLAPPED[pipeSize];
	memset(overLaps, 0, sizeof(overLaps));
	eventHandles = new HANDLE[pipeSize];
	memset(eventHandles, 0, sizeof(eventHandles));

	for (int i = 0; i < pipeSize; i++)
	{
		ioKeys[i].ioPort = hPort;
		ioKeys[i].pipeHandle = CreateNamedPipeW(pipeName,
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_WAIT | PIPE_READMODE_MESSAGE | PIPE_TYPE_MESSAGE,
			PIPE_UNLIMITED_INSTANCES,
			0,
			0,
			INFINITE,
			NULL
		);

		eventHandles[i] = CreateEventW(NULL, FALSE, TRUE, NULL);
		overLaps[i].hEvent = eventHandles[i];

		ioKeys[i].ov = &overLaps[i];
		ConnectNamedPipe(ioKeys[i].pipeHandle, ioKeys[i].ov);
		
		if (INVALID_HANDLE_VALUE == ioKeys[i].pipeHandle)
		{
			return FALSE;
		}
		if (NULL == CreateIoCompletionPort(ioKeys[i].pipeHandle, hPort, (ULONG_PTR)&ioKeys[i], threadSize))
		{
			wprintf(L"IOCP connect Error");
			return FALSE;
		}
	}

	threadHandles = new HANDLE[threadSize];
	for (int i = 0; i < threadSize; i++)
	{
		threadHandles[i] = (HANDLE)_beginthreadex(NULL, 0, RecvDataThread, (void*)hPort, NULL, NULL);
		if (INVALID_HANDLE_VALUE == threadHandles[i])
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
		succFunc = GetQueuedCompletionStatus(portHandle, &readLen, (PULONG_PTR)&nowKey, &ov, INFINITE);
		if (!succFunc) {
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
					DIsplay(&(nowKey->msgDataBuf));
				}
			}
			else 
			{
				wprintf(L"Fail Pending Error GLE=%d", GetLastError());
			}
		}
		else
		{
			DIsplay(&(nowKey->msgDataBuf));
		}		

		FlushFileBuffers(nowKey->pipeHandle);
		DisconnectNamedPipe(nowKey->pipeHandle);
		ConnectNamedPipe(nowKey->pipeHandle, nowKey->ov);
	}

	return TRUE;
}


void CMainDlg::DIsplay(MsgData *inputMsgData)
{
	std::wstring recvProcessName = std::wstring(inputMsgData->processName);
	
	// 7초 대기 후 진행
	WaitForSingleObject(deleteDlg, 7000);
	for (int i = 0; i < curCollectDlg[recvProcessName].size(); i++)
	{
		(curCollectDlg[recvProcessName][i].second)->InsertData(inputMsgData);
	}
}


BOOL CMainDlg::Start(HINSTANCE _parentInstance)
{
	BOOL succFunc = InitTrasmission();
	if (FALSE == succFunc)
	{
		MessageBoxW((HWND)_parentInstance, L"PIPE make Error", L"ERROR", MB_OK);
	}
	DialogBoxParamW(_parentInstance, MAKEINTRESOURCEW(IDD_MAINPAGE), NULL, ::RunProcMain, NULL);//(LPARAM)this);
	return TRUE;
}

BOOL CMainDlg::End()
{
	quitThread = TRUE;
	for (int i = 0; i < pipeSize; i++)
	{
		CloseHandle(ioKeys[i].pipeHandle);
		CloseHandle(ioKeys[i].ov->hEvent);
	}
	WaitForMultipleObjects(threadSize, threadHandles, TRUE, INFINITE);

	for (int i = 0; i < threadSize; i++)
	{
		CloseHandle(threadHandles[i]);
	}

	delete[] overLaps;
	delete[] eventHandles;
	delete[] ioKeys;
	delete[] threadHandles;
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
	RefreshList(hwnd);
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
			WCHAR szProcessName[MAX_PATH] = { 0, };
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
					szProcessName,
					sizeof(szProcessName) / sizeof(WCHAR));
				if (NULL != successFunc)
				{
					std::wstring addItem(wcsrchr(szProcessName, L'\\') + 1);
					addItem += L"(PID: ";
					addItem += std::to_wstring(processlist[i]);
					addItem += L")";
					ListBox_AddString(listHwnd, addItem.c_str());
				}
			}
			CloseHandle(processHandle);
		}
	}
	return TRUE;
}

void CMainDlg::InsertClickProcess(HWND hwndCtl, HWND hwnd)
{
	int count = SendMessageW(hwndCtl, LB_GETCURSEL, 0, 0);
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
	//LPWSTR currentContent = NULL;
	std::map<std::wstring, BOOL> runList;
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

		runList[subString] = TRUE;

		if (std::string::npos == subLocation)
		{
			break;
		}
	}

	if (0 == runList.size()) return 0;

	CCollectDlg* newCCollectDlg = new CCollectDlg(curChildIndex);

	for (auto i = runList.begin(); i != runList.end(); i++)
	{
		curCollectDlg[i->first].push_back(std::pair<int, CCollectDlg*>(curChildIndex, newCCollectDlg));
	}

	newCCollectDlg->Start(hwnd);
	curChildIndex++;
	
	return 0;
}

BOOL CMainDlg::EndCollect(int eraseIndex)
{
	ResetEvent(deleteDlg);
	for (auto i = curCollectDlg.begin(); i != curCollectDlg.end(); i++)
	{
		int nextLoopSize = i->second.size();
		for (int j = 0; j < nextLoopSize; j++)
		{
			if ((i->second)[j].first == eraseIndex)
			{
				//(i->second)[j].second->End();
				delete (i->second)[j].second;
				(i->second).erase((i->second).begin() + j);
				break;
			}
		}
	}
	SetEvent(deleteDlg);

	return TRUE;
}
