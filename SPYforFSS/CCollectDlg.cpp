#include "CCollectDlg.h"

CCollectDlg::CCollectDlg()
{
}

CCollectDlg::~CCollectDlg()
{
}

BOOL CCollectDlg::Start()
{
	//CreateDialogParamW(NULL, MAKEINTRESOURCEW(IDD_COLLECTPAGE), NULL, CCollectDlg::RunProc, (LPARAM)this);
	DialogBoxParamW(NULL, MAKEINTRESOURCEW(IDD_COLLECTPAGE), NULL, CCollectDlg::RunProc, (LPARAM)this);
	return TRUE;
}

BOOL CCollectDlg::End()
{
	return 0;
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

// ���̾�α� �ʱ�ȭ
BOOL CCollectDlg::InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	//Option ����
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
		showMsgData = !showMsgData;
		break;

	case IDC_FILEOUT:
		SaveLog(hwnd);
		break;

	case IDC_OPTION:
		COptionDlg option;
		option.Start();
		//option.~COptionDlg();
		break;
	}
}

void CCollectDlg::InsertData(MsgData *inputMsgData)
{
	if (FALSE == showMsgData)
	{
		//return;
	}

	HWND listHwnd = GetDlgItem(ownHwnd, IDC_COLLECTLIST);
	std::wstring viewMsg = L"";
	std::wstring lineNum = std::to_wstring(countLine++);

	// Line number(8�ڸ���) �߰�
	viewMsg += L"<";
	for(int i = 0; i < 8 - lineNum.size(); i++)
		viewMsg += L"0";
	viewMsg += lineNum + L"> ";

	// Process Name �߰�
	viewMsg += L" [" + std::wstring(inputMsgData->processName) + L"]";

	// Process ID �߰�
	viewMsg += L" [" + std::to_wstring(inputMsgData->processID) + L"]";

	// Msg type �߰�
	viewMsg += L" ";
	viewMsg += inputMsgData->msgType;

	// Message Content �߰�
	viewMsg += L" [" + std::wstring(inputMsgData->msgContent) + L"]";

	// Message Code �߰�
	viewMsg += L"(" + std::to_wstring(inputMsgData->msgCode) + L")";

	// wParam �߰�
	viewMsg += L"   wParam : " + std::to_wstring(inputMsgData->wParam);

	// lParam �߰�
	viewMsg += L"   lParam : " + std::to_wstring(inputMsgData->lParam);


	SendMessageW(listHwnd, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)viewMsg.data());
}

BOOL CCollectDlg::SaveLog(HWND hwnd)
{
	HWND ListBoxHwnd = GetDlgItem(hwnd, LB_INSERTSTRING);
	UINT count = SendMessageW(ListBoxHwnd, LB_GETCOUNT, 0, 0);
	
	// ���� ��� ����

	for (UINT i = 0; i < count; i++)
	{
		UINT testLen = SendMessageW(ListBoxHwnd, LB_GETTEXTLEN, (WPARAM)(0), (LPARAM)NULL);
		LPWSTR curText = new WCHAR[testLen];
		SendMessageW(ListBoxHwnd, LB_GETTEXT, (WPARAM)(count), (LPARAM)curText);
		// ���� ����
		delete[] curText;
	}

	return TRUE;
}
