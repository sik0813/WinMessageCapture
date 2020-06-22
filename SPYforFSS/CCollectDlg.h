#pragma once

#include "commonInclude.h"
#include "COptionDlg.h"
#include <queue>

class CCollectDlg
{
public:
	CCollectDlg(int _objectIndex);
	~CCollectDlg();

private:
	const int objectIndex = 0;
	HWND parentHwnd = NULL;

	HWND ownHwnd = NULL;
	UINT countLine = 0;
	BOOL showMsgData = FALSE;

	std::queue<MsgData> inputMsg;
	HANDLE threadHandle = INVALID_HANDLE_VALUE;
	HANDLE readDataEvent = INVALID_HANDLE_VALUE;
	HANDLE writeDataEvent = INVALID_HANDLE_VALUE;
	BOOL threadQuit = FALSE;

public:
	BOOL Start(HWND _parentHwnd);
	BOOL End();

	static INT_PTR CALLBACK CCollectDlg::RunProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// 다이얼로그 초기화
	BOOL InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Command(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

	// 입력 받은 데이터를 ListBox에 추가
	void InsertData(MsgData *MsgData);

	static UINT WINAPI DisplayDataThread(void *arg);
	void DisplayData();

	// 현재 ListBox 데이터 출력
	BOOL SaveLog(HWND hwnd);
};
