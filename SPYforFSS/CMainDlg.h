#pragma once
#include "commonInclude.h"
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
	BOOL RefreshList(HWND hwnd);

	// PIDLIST 에서 더블 클릭한 항목 삽입
	void InsertClickProcess(HWND hwndCtl, HWND hwnd);

	// EditControl(IDC_SELELTS)의 내용을 읽어서 파싱하고 해당 개수 만큼 CCollect 생성
	BOOL StartCollect(HWND hwnd);

};