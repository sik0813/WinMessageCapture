#pragma once
#include "sameInclude.h"
#include "COptionDlg.h"

typedef struct settingData {
	WCHAR processName[MAX_PATH]; // 260 * 2 = 520
	HWND wndHwnd; // 8
	CHAR activeFlag; // 1
	options option;
}settingData;

class CCollectDlg
{
public:
	CCollectDlg();
	CCollectDlg(DWORD inputNum);
	~CCollectDlg();

private:
	HINSTANCE parentInstance = NULL;
	HWND ownHwnd = NULL;
	DWORD windowNum = 0;

public:
	BOOL Show(HINSTANCE _parentInstance);

	static INT_PTR CALLBACK CCollectDlg::RunProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// 다이얼로그 초기화
	BOOL InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);

	void Command(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
};