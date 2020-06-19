#include "COptionDlg.h"

COptionDlg::COptionDlg()
{
}

COptionDlg::~COptionDlg()
{
}

BOOL COptionDlg::Start()
{
	DialogBoxParamW(NULL, MAKEINTRESOURCEW(IDD_OPTIONPAGE), NULL, COptionDlg::RunProc, (LPARAM)this);
	return TRUE;
}

INT_PTR CALLBACK COptionDlg::RunProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	COptionDlg* pointerThis = (COptionDlg*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lParam);
		pointerThis = (COptionDlg*)lParam;
		pointerThis->ownHwnd = hwnd;
		pointerThis->InitDialog(hwnd, (HWND)(wParam), lParam);
		break;

	case WM_COMMAND:
		pointerThis->Command(hwnd, (int)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam));
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