#pragma once
#include "commonInclude.h"
#include "CCollectDlg.h"
#include <process.h>
#include <Psapi.h>

static INT_PTR CALLBACK RunProcMain(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class CMainDlg
{
public:
	CMainDlg();
	~CMainDlg();

private:
	HWND ownHwnd = NULL;
	DWORD counter = 0;
	HANDLE recvDataThread = NULL;

	HANDLE sharedMemory = NULL;
	HANDLE writeEvent = NULL;
	HANDLE readerEvent = NULL;
	HANDLE writeMutex = NULL;
	HANDLE readerMutex = NULL;
	HANDLE otherProcessMutex = NULL;

	LPWSTR sharedMemoryName = L"SPYmemory"; // �����޸� �̸�
	LPSendData recvDataBuf = NULL; // �����޸� ���� ������

public:
	int InitTrasmission();
	static UINT WINAPI RecvDataThread(void *arg);
	BOOL RecvData();
	BOOL DisPlay();

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