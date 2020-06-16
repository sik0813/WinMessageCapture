#pragma once
#include <Windows.h>
#include <string>
#define OPTIONLEN 8192

typedef struct sendData
{
	WCHAR processName[MAX_PATH];
	DWORD processID;
	DWORD MessageCode;
	WCHAR MessageContent[64];
	WPARAM wParam;
	LPARAM lParam;
	sendData()
	{
		memset(&processName, 0, MAX_PATH);
		processID = 0;
		MessageCode = 0;
		memset(&MessageContent, 0, 64);
		wParam = NULL;
		lParam = NULL;
	}
}sendData, *LPSendData;

class CClient 
{
public:
	CClient(LPWSTR _processName, DWORD _processID);
	~CClient();

private:
	HANDLE writeEvent = NULL;
	HANDLE readerEvent = NULL;
	HANDLE writeMutex = NULL;
	HANDLE readerMutex = NULL;
	HANDLE sharedMemory = NULL;

	LPWSTR sharedMemoryName = L"Local\\SPYmemory";
	LPSendData sendDataBuf = NULL;
	sendData curSendData;

public:
	int Connect();
	void MakeMsg(DWORD _MessageCode, LPWSTR _MessageContent, WPARAM _wParam, LPARAM _lParam);
	BOOL SendMsg();
	LPWSTR GetProcessName();
};