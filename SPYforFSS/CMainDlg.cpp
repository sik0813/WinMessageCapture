#include "CMainDlg.h"

CMainDlg* CMainDlg::procAccess = 0;

CMainDlg::CMainDlg()
{
	procAccess = this;
	writeEvent = CreateEventW(NULL, FALSE, TRUE, L"SPYFSSwirterE");
	readerEvent = CreateEventW(NULL, FALSE, TRUE, L"SPYFSSreaderE");
	writeMutex = CreateMutexW(NULL, FALSE, L"SPYFSSwirterM");
	readerMutex = CreateMutexW(NULL, FALSE, L"SPYFSSreaderM");
	otherProcessMutex = CreateMutexW(NULL, FALSE, L"SPYFSSotherProcessM");
}

CMainDlg::~CMainDlg()
{
	UnmapViewOfFile(recvDataBuf);
	CloseHandle(sharedMemory);
	CloseHandle(writeEvent);
	CloseHandle(readerEvent);
	CloseHandle(writeMutex);
	CloseHandle(readerMutex);
	CloseHandle(otherProcessMutex);
}

static INT_PTR CALLBACK RunProcMain(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return CMainDlg::procAccess->RunProc(hwnd, uMsg, wParam, lParam);
}


int CMainDlg::InitTrasmission()
{
	ResetEvent(writeEvent);
	ResetEvent(readerEvent);

	HANDLE sharedMemory = CreateFileMapping(
		INVALID_HANDLE_VALUE,    // use paging file
		NULL,                    // default security
		PAGE_READWRITE,          // read/write access
		0,                       // maximum object size (high-order DWORD)
		sizeof(sendData),                // maximum object size (low-order DWORD)
		sharedMemoryName);                // name of mapping object

	if (sharedMemory == NULL)
	{
		return 1;
	}
	recvDataBuf = (LPSendData)MapViewOfFile(sharedMemory,   // handle to map object
		FILE_MAP_ALL_ACCESS, // read/write permission
		0,
		0,
		sizeof(sendData));

	if (recvDataBuf == NULL)
	{
		CloseHandle(sharedMemory);
		return 1;
	}
	return 0;
}

UINT WINAPI CMainDlg::RecvDataThread(void *arg)
{
	((CMainDlg*)arg)->RecvData();
	return 0;
}



BOOL CMainDlg::RecvData()
{
	while (true)
	{
		WaitForSingleObject(readerEvent, INFINITE);
		DisPlay();
		SetEvent(writeEvent);
	}	
	return TRUE;
}

BOOL CMainDlg::DisPlay()
{

	return TRUE;
}

BOOL CMainDlg::Show(HINSTANCE _parentInstance)
{
	_beginthreadex(NULL, 0, RecvDataThread, (void*)this, NULL, NULL);
	DialogBoxParamW(_parentInstance, MAKEINTRESOURCEW(IDD_MAINPAGE), NULL, ::RunProcMain, NULL);//(LPARAM)this);
	return TRUE;
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
	}

	return FALSE;
}

void CMainDlg::Command(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	BOOL SuccessFuc = FALSE;
	switch (id)
	{
	case IDCANCEL:
	case IDOK:
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
		CCollectDlg collectDlg;
		collectDlg.Show();
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
	WCHAR currentContent[1000] = { 0, };
	HWND editHwnd = GetDlgItem(hwnd, IDC_SELECTS);
	GetWindowTextW(editHwnd, currentContent, 1000);

	return 0;
}
