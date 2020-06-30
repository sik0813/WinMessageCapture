#pragma once
#include <Windows.h>
#include <string>

#define BUF_SIZE 4096
#define PIPE_TIMEOUT 5000

typedef struct _MsgData
{
	WCHAR processName[MAX_PATH];
	DWORD processID;
	DWORD threadID;
	int hookType;
	HWND hwnd;
	DWORD msgCode;
	WPARAM wParam;
	LPARAM lParam;
	HINSTANCE hInstance;
	_MsgData()
	{
		memset(&processName, 0, MAX_PATH);
		processID = 0;
		threadID = 0;
		hookType = 0;
		hwnd = NULL;
		msgCode = 0;
		wParam = NULL;
		lParam = NULL;
		hInstance = NULL;
	}
}MsgData, *LPMsgData;

class CClient 
{
public:
	CClient();
	~CClient();
	void Start(LPWSTR _processName);
	void End();

private:
	LPCWSTR m_pipeName = L"\\\\.\\pipe\\SPYFSS";
	HANDLE pipeHandle = INVALID_HANDLE_VALUE;

	HANDLE dataRecvThread = INVALID_HANDLE_VALUE;

	MsgData curSendData;

	/* Shared Memory Start */
	HANDLE readListHandle = INVALID_HANDLE_VALUE;
	LPCWSTR m_sharedMemName = L"Local\\SPYSendList";
	LPCWSTR m_writeDoneEvent = L"listWriteDone";
	HANDLE m_listWriteDone = NULL;
	HANDLE m_hMapFile = NULL;
	LPWSTR m_pBuf = NULL;
	/* Shared Memory End */

public:
	std::wstring processName = L"";
	static BOOL readListQuit;
	static BOOL sendFlag;

public:
	void MakeMsg(int _nCode, WPARAM _wParam, LPARAM _lParam, int _hookType);
	BOOL SendMsg();
	static UINT WINAPI ReadListThread(void *arg);
	void ReadList();

private:
	BOOL Connect();
	void Disconnect();
};