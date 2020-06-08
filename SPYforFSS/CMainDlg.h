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

	// ���̾�α� �ʱ�ȭ
	BOOL InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);

	void Command(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	
	// ���� ���μ��� ����Ʈ ��ȸ �� ǥ��
	BOOL RefreshList(HWND hwnd);

	// PIDLIST ���� ���� Ŭ���� �׸� ����
	void InsertClickProcess(HWND hwndCtl, HWND hwnd);

	// EditControl(IDC_SELELTS)�� ������ �о �Ľ��ϰ� �ش� ���� ��ŭ CCollect ����
	BOOL StartCollect(HWND hwnd);

};