#include "CCollectDlg.h"

CCollectDlg::CCollectDlg()
{
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