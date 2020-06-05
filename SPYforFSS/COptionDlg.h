#pragma once
#include "sameInclude.h"

#define OPTIONLEN 8192

typedef struct options {
	CHAR msgOption[OPTIONLEN]; // 8192(윈도우 메시지 크기 1byte)
	CHAR printOption; // 1
	DWORD maxLine; // 4
}options;


class COptionDlg
{
public:
	COptionDlg();
	~COptionDlg();

private:
	HINSTANCE parentInstance = NULL;
	HWND ownHwnd = NULL;
public:
	BOOL Show(HINSTANCE _parentInstance);

	static INT_PTR CALLBACK RunProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// 다이얼로그 초기화
	BOOL InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);

	void Command(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
};