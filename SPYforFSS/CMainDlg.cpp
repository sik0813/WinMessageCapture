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
	//auto pThis = (CMainDlg*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (uMsg)
	{
	case WM_INITDIALOG:
		HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, InitDialog);
		//SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lParam);
		//pThis = (CMainDlg*)lParam;
		//pThis->ownHwnd = hwnd;
		//HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, pThis->InitDialog);
		//pThis->InitDialog(hwnd, (HWND)(wParam), lParam);
		break;

	case WM_COMMAND:
		HANDLE_WM_COMMAND(hwnd, wParam, lParam, Command);
		//HANDLE_WM_COMMAND(hwnd, wParam, lParam, pThis->Command);
		//pThis->Command(hwnd, (int)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam));
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

	case IDC_WATCH:
		CCollectDlg tmp(counter);
		counter++;
		tmp.Show(parentInstance);
		break;
	}
}

// 다이얼로그 초기화

BOOL CMainDlg::InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{

	return TRUE;
}

BOOL CMainDlg::ShowProcessList(HWND hwnd)
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
	for (unsigned int i = 0; i < listLen; i++)
	{
		if (processlist[i] != 0)
		{
			PrintProcessNameAndID(processlist[i], listHwnd);
		}
	}
	return TRUE;
}

void CMainDlg::PrintProcessNameAndID(DWORD processID, HWND hwnd)
{
	WCHAR szProcessName[MAX_PATH] = { 0, };
	// Get a handle to the process.

	HANDLE processHandle = OpenProcess(
		PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
		FALSE,
		processID);

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
			addItem += std::to_wstring(processID);
			addItem += L")";
			ListBox_AddString(hwnd, addItem.c_str());
		}
	}
	CloseHandle(processHandle);
}