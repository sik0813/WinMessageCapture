#include "COptionDlg.h"

COptionDlg::COptionDlg()
{
}

COptionDlg::~COptionDlg()
{
}

BOOL COptionDlg::Start(HWND _parentHwnd, LPSettingData _inputSetting)
{
	m_parentHwnd = _parentHwnd;
	m_changeSettingData = *_inputSetting;
	m_ownHwnd = CreateDialogParamW(NULL, MAKEINTRESOURCEW(IDD_OPTIONPAGE), NULL, COptionDlg::RunProc, (LPARAM)this);
	ShowWindow(m_ownHwnd, SW_SHOW);
	return TRUE;
}

BOOL COptionDlg::End(int quitCase)
{
	PostMessageW(m_parentHwnd, WM_SETOPTION, NULL, NULL);
	if (2 == quitCase)
	{
		DestroyWindow(m_ownHwnd);
	}
	
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
		pointerThis->m_ownHwnd = hwnd;
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
		m_changeOption = FALSE;
		End(2);
		break;

	case IDC_START:
		m_changeOption = TRUE;
		End();
		break;

	case IDC_ALLSELECT:
		SendMessageW(GetDlgItem(hwnd, IDC_MSGLIST), LB_SETSEL, TRUE, (LPARAM)-1);
		break;

	case IDC_ALLDESELECT:
		SendMessageW(GetDlgItem(hwnd, IDC_MSGLIST), LB_SETSEL, FALSE, (LPARAM)-1);
		break;
	}
}

BOOL COptionDlg::GetOption(LPSettingData _inputSetting)
{
	m_msgListHwnd = GetDlgItem(m_ownHwnd, IDC_MSGLIST);
	m_wParamCheck = GetDlgItem(m_ownHwnd, IDC_SHOWWPARAM);
	m_lParamCheck = GetDlgItem(m_ownHwnd, IDC_SHOWLPARAM);

	if (FALSE == m_changeOption)
	{
		return FALSE;
	}

	// 상위 9개 메세지에 대하여 check
	WCHAR wmBuf[64] = { 0, };
	for (int i = 0; i < m_optionListIndex; i++)
	{
		LRESULT isSelected = SendMessageW(m_msgListHwnd, LB_GETSEL, i, NULL);
		if (0 < isSelected)
		{
			memset(wmBuf, 0, sizeof(wmBuf));
			SendMessageW(m_msgListHwnd, LB_GETTEXT, i, (LPARAM)wmBuf);
			if (NULL != wmBuf)
			{
				DWORD wmIndex = stringToWm[std::wstring(wmBuf)];
				m_changeSettingData.msgOptions[wmIndex] = true;
			}
		}
		else
		{
			SendMessageW(m_msgListHwnd, LB_GETTEXT, i, (LPARAM)wmBuf);
			if (NULL != wmBuf)
			{
				m_changeSettingData.msgOptions[stringToWm[std::wstring(wmBuf)]] = false;
			}
		}
	}

	// 전체 메시지 여부 선택 여부 확인
	LRESULT isSelected = SendMessageW(m_msgListHwnd, LB_GETSEL, m_optionListIndex, NULL);
	if (0 < isSelected)
	{
		memset(wmBuf, 0, sizeof(wmBuf));
		SendMessageW(m_msgListHwnd, LB_GETTEXT, m_optionListIndex, (LPARAM)wmBuf);
		m_changeSettingData.otherMsgShow = true;
	}
	else
	{
		SendMessageW(m_msgListHwnd, LB_GETTEXT, m_optionListIndex, (LPARAM)wmBuf);
		m_changeSettingData.otherMsgShow = false;
	}

	LRESULT checkBoxRet = SendMessageW(m_wParamCheck, BM_GETCHECK, NULL, NULL);
	if (BST_CHECKED == checkBoxRet)
	{
		m_changeSettingData.wParamShow = true;
	}
	else
	{
		m_changeSettingData.wParamShow = false;
	}

	// check box 2개 확인
	checkBoxRet = SendMessageW(m_lParamCheck, BM_GETCHECK, NULL, NULL);
	if (BST_CHECKED == checkBoxRet)
	{
		m_changeSettingData.lParamShow = true;
	}
	else
	{
		m_changeSettingData.lParamShow = false;
	}

	memcpy(_inputSetting, &m_changeSettingData, sizeof(SettingData));

	DestroyWindow(m_ownHwnd);
	return TRUE;
}

// 다이얼로그 초기화
BOOL COptionDlg::InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	// list box 구성
	m_msgListHwnd = GetDlgItem(hwnd, IDC_MSGLIST);
	m_wParamCheck = GetDlgItem(hwnd, IDC_SHOWWPARAM);
	m_lParamCheck = GetDlgItem(hwnd, IDC_SHOWLPARAM);

	SendMessageW(m_msgListHwnd, LB_INSERTSTRING, -1, (LPARAM)wmToString[WM_CREATE]);
	if(true == m_changeSettingData.msgOptions[WM_CREATE])
		SendMessageW(m_msgListHwnd, LB_SETSEL, TRUE, (LPARAM)m_optionListIndex);
	m_optionListIndex++;

	SendMessageW(m_msgListHwnd, LB_INSERTSTRING, -1, (LPARAM)wmToString[WM_SHOWWINDOW]);
	if (true == m_changeSettingData.msgOptions[WM_SHOWWINDOW])
		SendMessageW(m_msgListHwnd, LB_SETSEL, TRUE, (LPARAM)m_optionListIndex);
	m_optionListIndex++;

	SendMessageW(m_msgListHwnd, LB_INSERTSTRING, -1, (LPARAM)wmToString[WM_WINDOWPOSCHANGED]);
	if (true == m_changeSettingData.msgOptions[WM_WINDOWPOSCHANGED])
		SendMessageW(m_msgListHwnd, LB_SETSEL, TRUE, (LPARAM)m_optionListIndex);
	m_optionListIndex++;

	SendMessageW(m_msgListHwnd, LB_INSERTSTRING, -1, (LPARAM)wmToString[WM_STYLECHANGED]);
	if (true == m_changeSettingData.msgOptions[WM_STYLECHANGED])
		SendMessageW(m_msgListHwnd, LB_SETSEL, TRUE, (LPARAM)m_optionListIndex);
	m_optionListIndex++;

	SendMessageW(m_msgListHwnd, LB_INSERTSTRING, -1, (LPARAM)wmToString[WM_NCACTIVATE]);
	if (true == m_changeSettingData.msgOptions[WM_NCACTIVATE])
		SendMessageW(m_msgListHwnd, LB_SETSEL, TRUE, (LPARAM)m_optionListIndex);
	m_optionListIndex++;

	SendMessageW(m_msgListHwnd, LB_INSERTSTRING, -1, (LPARAM)wmToString[WM_SETTEXT]);
	if (true == m_changeSettingData.msgOptions[WM_SETTEXT])
		SendMessageW(m_msgListHwnd, LB_SETSEL, TRUE, (LPARAM)m_optionListIndex);
	m_optionListIndex++;

	SendMessageW(m_msgListHwnd, LB_INSERTSTRING, -1, (LPARAM)wmToString[WM_SIZE]);
	if (true == m_changeSettingData.msgOptions[WM_SIZE])
		SendMessageW(m_msgListHwnd, LB_SETSEL, TRUE, (LPARAM)m_optionListIndex);
	m_optionListIndex++;

	SendMessageW(m_msgListHwnd, LB_INSERTSTRING, -1, (LPARAM)wmToString[WM_MOVE]);
	if (true == m_changeSettingData.msgOptions[WM_MOVE])
		SendMessageW(m_msgListHwnd, LB_SETSEL, TRUE, (LPARAM)m_optionListIndex);
	m_optionListIndex++;

	SendMessageW(m_msgListHwnd, LB_INSERTSTRING, -1, (LPARAM)wmToString[WM_DESTROY]);
	if (true == m_changeSettingData.msgOptions[WM_DESTROY])
		SendMessageW(m_msgListHwnd, LB_SETSEL, TRUE, (LPARAM)m_optionListIndex);
	m_optionListIndex++;

	SendMessageW(m_msgListHwnd, LB_INSERTSTRING, -1, (LPARAM)L"Other Messages");
	if (true == m_changeSettingData.otherMsgShow)
		SendMessageW(m_msgListHwnd, LB_SETSEL, TRUE, (LPARAM)m_optionListIndex);

	if (true == m_changeSettingData.wParamShow)
		SendMessageW(m_wParamCheck, BM_SETCHECK, BST_CHECKED, (LPARAM)0);

	if (true == m_changeSettingData.lParamShow)
		SendMessageW(m_lParamCheck, BM_SETCHECK, BST_CHECKED, (LPARAM)0);

	return TRUE;
}