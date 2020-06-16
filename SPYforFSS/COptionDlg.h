#pragma once
#include "commonInclude.h"

class COptionDlg
{
public:
	COptionDlg();
	~COptionDlg();

private:
	HINSTANCE parentInstance = NULL;
	HWND ownHwnd = NULL;
public:
	BOOL Show();

	static INT_PTR CALLBACK RunProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// ���̾�α� �ʱ�ȭ
	BOOL InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);

	void Command(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
};