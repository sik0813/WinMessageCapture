#pragma once
#include "commonInclude.h"
#include "CCollectDlg.h"
#include <process.h>
#include <Psapi.h>

#define THREAD_SIZE 2
#define BUF_SIZE 4096
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
	HINSTANCE m_parentInstance = NULL;
	HWND m_ownHwnd = NULL;
	HWND m_curDlgHwnd = NULL;
	DWORD m_counter = 0;

	LPCWSTR m_pipeName = L"\\\\.\\pipe\\SPYFSS";
	
	/* Shared Memory Start */
	LPCWSTR m_sharedMemName = L"Local\\SPYSendList";
	LPCWSTR m_writeDoneEvent = L"listWriteDone";
	HANDLE m_listWriteDone = NULL;
	HANDLE m_hMapFile = NULL;
	/* Shared Memory End */

	IoKey m_ioKeys;
	HANDLE m_hPort = NULL;
	OVERLAPPED m_overLap;
	HANDLE m_eventHandle = NULL;
	HANDLE m_threadHandles[THREAD_SIZE];
	CRITICAL_SECTION m_deleteDlg; // CCollectDlg 객체가 사라질 때 값 동기화


	// Object 가지고 있을 map
	std::map<std::wstring, std::vector<std::pair<int, CCollectDlg*>>> m_curCollectDlg;
	int m_curChildIndex = 0;
	LPWSTR m_pBuf = NULL;

public:
	static CMainDlg* procAccess;
	static BOOL quitThread;

public:
	// Pipe, Thread Pool 생성
	BOOL InitTrasmission();
	static UINT WINAPI RecvDataThread(void *arg);
	BOOL RecvData(HANDLE portHandle);
	void Display(MsgData &msgData);

	BOOL Start(HINSTANCE _parentInstance);
	BOOL Show();
	BOOL End();
	
	INT_PTR CALLBACK RunProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	
	// 다이얼로그 초기화
	BOOL InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Command(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	
	// 현재 프로세스 리스트 조회 및 표시
	BOOL RefreshList(HWND hwnd);
	
	// PIDLIST 에서 더블 클릭한 항목 삽입
	void InsertClickProcess(HWND hwndCtl, HWND hwnd);
	
	// EditControl(IDC_SELELTS)의 내용을 읽어서 파싱하고 CCollect 생성
	BOOL StartCollect(HWND hwnd);
	BOOL EndCollect(int eraseIndex);
	void UpdateSendList();
};

// ThreadPool 생성시 argment list
typedef struct _PipeThread {
	CMainDlg* curMainDlg;
	HANDLE portHandle;
	_PipeThread()
	{
		curMainDlg = NULL;
		portHandle = NULL;
	}
}PipeThread, *LPPipeThread;