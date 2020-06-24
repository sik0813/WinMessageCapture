#pragma once
#include "commonInclude.h"

class COptionDlg
{
public:
	COptionDlg();
	~COptionDlg();

private:
	HWND parentHwnd = NULL;
	HWND ownHwnd = NULL;

	BOOL changeOption = FALSE;
	
	SettingData changeSettingData;

public:
	BOOL Start(HWND _parentHwnd, LPSettingData _inputSetting);
	BOOL End(int quitCase = 1);

	static INT_PTR CALLBACK RunProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// ���̾�α� �ʱ�ȭ
	BOOL InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);

	void Command(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

	// ����� �� ��ȯ
	BOOL GetOption(LPSettingData _inputSetting);
};