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
	UINT countLine = 0;
	BOOL showMsgData = FALSE;

public:
	BOOL Start();
	BOOL End();

	static INT_PTR CALLBACK CCollectDlg::RunProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// 다이얼로그 초기화
	BOOL InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Command(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

	// 입력 받은 데이터를 ListBox에 추가
	void InsertData(MsgData *MsgData);

	// 현재 ListBox 데이터 출력
	BOOL SaveLog(HWND hwnd);
};