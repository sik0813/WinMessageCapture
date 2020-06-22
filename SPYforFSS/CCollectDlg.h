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

	// ���̾�α� �ʱ�ȭ
	BOOL InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Command(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

	// �Է� ���� �����͸� ListBox�� �߰�
	void InsertData(MsgData *MsgData);

	static UINT WINAPI DisplayDataThread(void *arg);
	void DisplayData();

	// ���� ListBox ������ ���
	BOOL SaveLog(HWND hwnd);
};
