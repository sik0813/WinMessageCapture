#include "CMainDlg.h"

CMainDlg* CMainDlg::procAccess = 0;

CMainDlg::CMainDlg()
{
	procAccess = this;
}

CMainDlg::~CMainDlg()
{
}

static INT_PTR CALLBACK RunProcMain(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return CMainDlg::procAccess->RunProc(hwnd, uMsg, wParam, lParam);
}


BOOL CMainDlg::Show(HINSTANCE _parentInstance)
{
	parentInstance = _parentInstance;
	DialogBoxParamW(parentInstance, MAKEINTRESOURCEW(IDD_MAINPAGE), NULL, ::RunProcMain, NULL);//(LPARAM)this);
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
		collectDlg.Show(parentInstance);
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
	// Get the list of process identifiers.
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

	WCHAR currentContent[1000] = { 0, };
	HWND editHwnd = GetDlgItem(hwnd, IDC_SELECTS);
	GetWindowTextW(editHwnd, currentContent, 1000);

	std::wstring output = std::wstring(inputPID) + L"|" + std::wstring(currentContent);
	SetWindowTextW(editHwnd, output.c_str());
}

BOOL CMainDlg::StartCollect(HWND hwnd)
{
	WCHAR currentContent[1000] = { 0, };
	HWND editHwnd = GetDlgItem(hwnd, IDC_SELECTS);
	GetWindowTextW(editHwnd, currentContent, 1000);



	return 0;
}
