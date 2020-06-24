#pragma once
#include <Windows.h>
#include <string>

#define MSG_LEN 64
#define PIPE_TIMEOUT 5000
typedef struct _MsgData
{
	WCHAR processName[MAX_PATH];
	DWORD processID;
	DWORD msgCode;
	WPARAM wParam;
	LPARAM lParam;
	WCHAR msgType;
	_MsgData()
	{
		memset(&processName, 0, MAX_PATH);
		processID = 0;
		msgCode = 0;
		wParam = NULL;
		lParam = NULL;
		msgType = NULL;
	}
}MsgData, *LPMsgData;

class CClient 
{
public:
	CClient();
	~CClient();
	void Start(LPWSTR _processName, DWORD _processId);

private:
	LPCWSTR pipeName = L"\\\\.\\pipe\\SPYFSS";
	HANDLE pipeHandle = INVALID_HANDLE_VALUE;

	MsgData curSendData;

public:
	std::wstring processName = L"";
	
public:
	void MakeMsg(WCHAR msgType, DWORD _msgCode, WPARAM _wParam, LPARAM _lParam);
	BOOL SendMsg();

private:
	BOOL Connect();
	void Disconnect();
};