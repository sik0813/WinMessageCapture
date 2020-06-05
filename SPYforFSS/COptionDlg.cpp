#include "COptionDlg.h"

COptionDlg::COptionDlg()
{
}

COptionDlg::~COptionDlg()
{
}

BOOL COptionDlg::Show(HINSTANCE _parentInstance)
{
	parentInstance = _parentInstance;
	DialogBoxParamW(parentInstance, MAKEINTRESOURCEW(IDD_OPTIONPAGE), NULL, COptionDlg::RunProc, (LPARAM)this);
	return TRUE;
}

INT_PTR CALLBACK COptionDlg::RunProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	COptionDlg* pThis = (COptionDlg*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lParam);
		pThis = (COptionDlg*)lParam;
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

void COptionDlg::Command(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	BOOL SuccessFuc = FALSE;
	switch (id)
	{
	case IDCANCEL:
	case IDOK:
		EndDialog(hwnd, id);
		break;
	}
}

// 다이얼로그 초기화

BOOL COptionDlg::InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	return TRUE;
}