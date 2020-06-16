#pragma once

#include "commonInclude.h"
#include "COptionDlg.h"

class CCollectDlg
{
public:
	CCollectDlg();
	~CCollectDlg();

private:
	HINSTANCE parentInstance = NULL;
	HWND ownHwnd = NULL;

public:
	BOOL Show();

	static INT_PTR CALLBACK CCollectDlg::RunProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// 다이얼로그 초기화
	BOOL InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);

	void Command(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
};