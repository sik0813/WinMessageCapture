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
	WCHAR msgContent[MSG_LEN];
	WPARAM wParam;
	LPARAM lParam;
	WCHAR msgType;
	_MsgData()
	{
		memset(&processName, 0, MAX_PATH);
		processID = 0;
		msgCode = 0;
		memset(&msgContent, 0, MSG_LEN);
		wParam = NULL;
		lParam = NULL;
		msgType = NULL;
	}
}MsgData, *LPMsgData;

class CClient 
{
public:
	CClient(LPWSTR _processName, DWORD _processID);
	~CClient();

private:
	LPCWSTR pipeName = L"\\\\.\\pipe\\SPYFSS";
	HANDLE pipeHandle = INVALID_HANDLE_VALUE;

	MsgData curSendData;

public:
	void MakeMsg(WCHAR msgType, DWORD _msgCode, LPWSTR _msgContent, WPARAM _wParam, LPARAM _lParam);
	BOOL SendMsg();
	LPWSTR GetProcessName();

private:
	BOOL Connect();
	void Disconnect();
};