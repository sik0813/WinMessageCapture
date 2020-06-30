#pragma once
#include "commonInclude.h"

#define WMNUMBER(X) \X

class COptionDlg
{
public:
	COptionDlg();
	~COptionDlg();

private:
	HWND m_parentHwnd = NULL;
	HWND m_ownHwnd = NULL;

	int m_optionListIndex = 0;
	BOOL m_changeOption = FALSE;

	HWND m_msgListHwnd = NULL;
	HWND m_wParamCheck = NULL;
	HWND m_lParamCheck = NULL;
	
	SettingData m_changeSettingData;

public:
	BOOL Start(HWND _parentHwnd, LPSettingData _inputSetting);
	BOOL End(int quitCase = 1); // 1: ����, 2: ���

	static INT_PTR CALLBACK RunProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// ���̾�α� �ʱ�ȭ
	BOOL InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);

	void Command(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

	// ����� �� ��ȯ
	BOOL GetOption(LPSettingData _inputSetting);
};