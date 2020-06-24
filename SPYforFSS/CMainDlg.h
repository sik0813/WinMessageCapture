#pragma once
#include "commonInclude.h"
#include "CCollectDlg.h"
#include <process.h>
#include <Psapi.h>


#define PIPE_TIMEOUT 5000
#define GETIOCP_TIMEOUT 5000

//IOCP completion key
typedef struct _Iokey {
	LPOVERLAPPED ov;
	HANDLE pipeHandle;
	HANDLE ioPort;
	MsgData msgDataBuf;
	_Iokey()
	{
		ov = NULL;
		pipeHandle = INVALID_HANDLE_VALUE;
		ioPort = NULL;
	}
}IoKey;

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

	LPCWSTR pipeName = L"\\\\.\\pipe\\SPYFSS";
	
	IoKey *ioKeys = NULL;
	HANDLE hPort = NULL;
	OVERLAPPED *overLaps = NULL;
	HANDLE *eventHandles = NULL;
	HANDLE *threadHandles = NULL;
	HANDLE deleteDlg = NULL; // CCollectDlg ��ü�� ����� �� �� ����ȭ

	int pipeSize = 0;
	int threadSize = 0;

	// Object ������ ���� map
	std::map<std::wstring, std::vector<std::pair<int, CCollectDlg*>>> curCollectDlg;
	int curChildIndex = 0;

public:
	static CMainDlg* procAccess;
	static BOOL quitThread;

public:
	// Pipe, Thread Pool ����
	BOOL InitTrasmission();
	static UINT WINAPI RecvDataThread(void *arg);
	BOOL RecvData(HANDLE portHandle);
	void DIsplay(MsgData msgData);

	BOOL Start(HINSTANCE _parentInstance);	
	BOOL End();
	
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

	BOOL EndCollect(int eraseIndex);

};

// ThreadPool ������ argment list
typedef struct _PipeThread {
	CMainDlg* curMainDlg;
	HANDLE portHandle;
	_PipeThread()
	{
		curMainDlg = NULL;
		portHandle = NULL;
	}
}PipeThread, *LPPipeThread;