#include "CCollectDlg.h"

CCollectDlg::CCollectDlg()
{
}

CCollectDlg::CCollectDlg(DWORD inputNum)
{
	windowNum = inputNum;
}

CCollectDlg::~CCollectDlg()
{
}

BOOL CCollectDlg::Show(HINSTANCE _parentInstance)
{
	parentInstance = _parentInstance;
	DialogBoxParamW(parentInstance, MAKEINTRESOURCEW(IDD_COLLECTPAGE), NULL, CCollectDlg::RunProc, (LPARAM)this);
	return TRUE;
}

INT_PTR CALLBACK CCollectDlg::RunProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CCollectDlg* pThis = (CCollectDlg*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lParam);
		pThis = (CCollectDlg*)lParam;
		pThis->ownHwnd = hwnd;
		HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, pThis->InitDialog);
		//pThis->InitDialog(hwnd, (HWND)(wParam), lParam);
		break;

	case WM_COMMAND:
		HANDLE_WM_COMMAND(hwnd, wParam, lParam, pThis->Command);
		//pThis->Command(hwnd, (int)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam));
		break;
	}

	return FALSE;
}

// 다이얼로그 초기화
BOOL CCollectDlg::InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	return TRUE;
}

void CCollectDlg::Command(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	BOOL SuccessFuc = FALSE;
	switch (id)
	{
	case IDCANCEL:
	case IDOK:
		EndDialog(hwnd, id);
		break;

	case IDC_STARTSUSPEND:
		ListBox_AddString(GetDlgItem(hwnd, IDC_COLLECTLIST), std::to_wstring(windowNum).c_str());
		break;

	case IDC_FILEOUT:
		break;

	case IDC_OPTION:
		COptionDlg option;
		option.Show(parentInstance);
		//option.~COptionDlg();
		break;
	}
}