#include "COptionDlg.h"

COptionDlg::COptionDlg()
{
}

COptionDlg::~COptionDlg()
{
}

BOOL COptionDlg::Start(HWND _parentHwnd, LPSettingData _inputSetting)
{
	parentHwnd = _parentHwnd;
	changeSettingData = *_inputSetting;
	ownHwnd = CreateDialogParamW(NULL, MAKEINTRESOURCEW(IDD_OPTIONPAGE), NULL, COptionDlg::RunProc, (LPARAM)this);
	ShowWindow(ownHwnd, SW_SHOW);
	return TRUE;
}

BOOL COptionDlg::End(int quitCase)
{
	DestroyWindow(ownHwnd);
	if(1 == quitCase)
	PostMessageW(parentHwnd, WM_SETOPTION, NULL, NULL);
	
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
	case IDOK:
		break;

	case IDCANCEL:
	case IDC_CANCLE:
		changeOption = FALSE;
		End();
		break;

	case IDC_START:
		changeOption = TRUE;
		End();
		break;
	}
}

BOOL COptionDlg::GetOption(LPSettingData _inputSetting)
{
	if (FALSE == changeOption)
	{
		return FALSE;
	}
	*_inputSetting = changeSettingData;
	return TRUE;
}

// 다이얼로그 초기화
BOOL COptionDlg::InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	return TRUE;
}