#pragma once
#include "sameInclude.h"
#include "CCollectDlg.h"
#include <Psapi.h>

static INT_PTR CALLBACK RunProcMain(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class CMainDlg
{
public:
	CMainDlg();
	~CMainDlg();

private:
	HINSTANCE parentInstance = NULL;
	HWND ownHwnd = NULL;
	DWORD counter = 0;
public:
	BOOL Show(HINSTANCE _parentInstance);
	
	static CMainDlg* procAccess;
	INT_PTR CALLBACK RunProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// 다이얼로그 초기화
	BOOL InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);

	void Command(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	
	// 현재 프로세스 리스트 조회 및 표시
	void PrintProcessNameAndID(DWORD processID, HWND hwnd);
	BOOL ShowProcessList(HWND hwnd);

};