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
	HANDLE deleteDlg = NULL; // CCollectDlg 객체가 사라질 때 값 동기화

	int pipeSize = 0;
	int threadSize = 0;

	// Object 가지고 있을 map
	std::map<std::wstring, std::vector<std::pair<int, CCollectDlg*>>> curCollectDlg;
	int curChildIndex = 0;

public:
	static CMainDlg* procAccess;
	static BOOL quitThread;

public:
	// Pipe, Thread Pool 생성
	BOOL InitTrasmission();
	static UINT WINAPI RecvDataThread(void *arg);
	BOOL RecvData(HANDLE portHandle);
	void DIsplay(MsgData msgData);

	BOOL Start(HINSTANCE _parentInstance);	
	BOOL End();
	
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

	BOOL EndCollect(int eraseIndex);

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