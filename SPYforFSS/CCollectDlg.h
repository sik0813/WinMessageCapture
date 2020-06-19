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

	// ���̾�α� �ʱ�ȭ
	BOOL InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Command(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

	// �Է� ���� �����͸� ListBox�� �߰�
	void InsertData(MsgData *MsgData);

	// ���� ListBox ������ ���
	BOOL SaveLog(HWND hwnd);
};