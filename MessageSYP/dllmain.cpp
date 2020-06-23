// dllmain.cpp : DLL 응용 프로그램의 진입점을 정의합니다.
#include "stdafx.h"
#include "MessageSYP.h"
#include <Psapi.h>
#include <strsafe.h>
#include <process.h>
#include "CClient.h"

HINSTANCE kdllInstance = NULL;
static HHOOK kCallWnd = NULL;
static HHOOK kCallWndRet = NULL;
static HHOOK kGetMsg = NULL;
CClient *nowClient = NULL;

LPCWSTR deniedProcessName = L"SPYforFSS.exe";

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	WCHAR processName[MAX_PATH] = { 0, };
	GetModuleFileName(NULL, processName, sizeof(processName) / sizeof(WCHAR));
	if (NULL == processName)
	{
		return FALSE;
	}
	StringCchCopyW(processName, MAX_PATH, wcsrchr(processName, L'\\') + 1);	

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		kdllInstance = (HINSTANCE)hModule;		
		nowClient = new CClient();
		nowClient->Start(processName, GetCurrentProcessId());
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		delete nowClient;
		break;
	}
	return TRUE;
}

// 메시지 후킹 시작
EXPORT void StartHook(DWORD threadID)
{
	kCallWnd = SetWindowsHookExW(WH_CALLWNDPROC, (HOOKPROC)CallWndProc, kdllInstance, threadID); // send
	kCallWndRet = SetWindowsHookExW(WH_CALLWNDPROCRET, (HOOKPROC)CallWndRetProc, kdllInstance, threadID); // return
	kGetMsg = SetWindowsHookExW(WH_GETMESSAGE, (HOOKPROC)GetMsgProc, kdllInstance, threadID); // post
	return;
}


EXPORT void StopHook(void)
{
	UnhookWindowsHookEx(kCallWnd);
	UnhookWindowsHookEx(kCallWndRet);
	UnhookWindowsHookEx(kGetMsg);
	return;
}

// SendMSG
LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (0 == wcscmp(deniedProcessName, nowClient->processName) ||
		nCode < 0)
	{
		return CallNextHookEx(kCallWnd, nCode, wParam, lParam);
	}
		
	if (NULL != nowClient->processName)
	{
		switch (nCode)
		{
		case HC_ACTION:
			if (0 != wParam)
			{
				LPCWPSTRUCT nowMsg = (LPCWPSTRUCT)lParam;
				nowClient->MakeMsg(L'S', nowMsg->message, nowMsg->wParam, nowMsg->lParam);
				nowClient->SendMsg();
			}
			break;

		default:
			break;
		}
	}

	return CallNextHookEx(kCallWnd, nCode, wParam, lParam);
}

// SendMSG return
LRESULT CallWndRetProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (0 == wcscmp(deniedProcessName, nowClient->processName) ||
		nCode < 0)
	{
		return CallNextHookEx(kCallWnd, nCode, wParam, lParam);
	}

	if (NULL != nowClient->processName)
	{
		switch (nCode)
		{
		case HC_ACTION:
			if (0 != wParam)
			{
				CWPRETSTRUCT *nowMsg = (CWPRETSTRUCT*)lParam;
				nowClient->MakeMsg(L'R', nowMsg->message, nowMsg->wParam, nowMsg->lParam);
				nowClient->SendMsg();
			}
			break;

		default:
			break;
		}
	}

	return CallNextHookEx(kCallWndRet, nCode, wParam, lParam);
}

// PostMSG
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (0 == wcscmp(deniedProcessName, nowClient->processName) ||
		nCode < 0)
	{
		return CallNextHookEx(kCallWnd, nCode, wParam, lParam);
	}

	if (NULL != nowClient->processName)
	{
		switch (nCode)
		{
		case HC_ACTION:
			if (0 != wParam)
			{
				MSG *nowMsg = (MSG*)lParam;
				nowClient->MakeMsg(L'P', nowMsg->message, nowMsg->wParam, nowMsg->lParam);
				nowClient->SendMsg();
			}
			break;

		default:
			break;
		}
	}

	return CallNextHookEx(kGetMsg, nCode, wParam, lParam);
}

CClient::CClient()
{
}

CClient::~CClient() {}

void CClient::Start(LPWSTR _processName, DWORD _processId)
{
	memset(processName, 0, MAX_PATH);
	StringCchCopyW(processName, wcslen(_processName) + 1, _processName);
	StringCchCopyW(curSendData.processName, wcslen(_processName) + 1, _processName);
	curSendData.processID = _processId;
}

BOOL CClient::Connect()
{
	pipeHandle = CreateFileW(
		pipeName,   // pipe name 
		GENERIC_READ | GENERIC_WRITE, // read and write access 
		0,              // no sharing 
		NULL,           // default security attributes
		OPEN_EXISTING,  // opens existing pipe 
		0,              // default attributes 
		NULL);          // no template file 

	if (INVALID_HANDLE_VALUE == pipeHandle)
	{
		if (ERROR_PIPE_BUSY != GetLastError())
		{
			wprintf(L"Could not open pipe. GLE=%d\n", GetLastError());
			return FALSE;
		}

		// 5초간 파이프 핸들 대기
		if (!WaitNamedPipe(pipeName, PIPE_TIMEOUT))
		{
			printf("Could not open pipe: 5 second wait timed out.");
			return FALSE;
		}
	}

	DWORD pipeMode = PIPE_READMODE_MESSAGE;
	BOOL fSuccess = SetNamedPipeHandleState(
		pipeHandle,    // pipe handle 
		&pipeMode,	  // pipe mode 
		NULL,		 // maximum bytes 
		NULL);		// maximum time 
	if (FALSE == fSuccess)
	{
		wprintf(L"SetNamedPipeHandleState failed. GLE=%d\n", GetLastError());
		return FALSE;
	}

	return TRUE;
}

void CClient::Disconnect()
{
	FlushFileBuffers(pipeHandle);
	CloseHandle(pipeHandle);
}

void CClient::MakeMsg(WCHAR _msgType, DWORD _msgCode, WPARAM _wParam, LPARAM _lParam)
{
	curSendData.msgType = _msgType;
	curSendData.msgCode = _msgCode;
	
	curSendData.wParam = _wParam;
	curSendData.lParam = _lParam;
}

BOOL CClient::SendMsg()
{
	BOOL succFunc = Connect();
	if (FALSE == succFunc)
	{
		Disconnect();
		return FALSE;
	}

	OVERLAPPED ov;
	memset(&ov, 0, sizeof(ov));

	DWORD cbWritten = 0;
	DWORD sendMsgLen = sizeof(curSendData);
	succFunc = WriteFile(
		pipeHandle,                  // pipe handle 
		&curSendData,             // message 
		sendMsgLen,              // message length 
		&cbWritten,             // bytes written 
		&ov);                  // overlapped 
	if (FALSE == succFunc)
	{
		wprintf(L"WriteFile to pipe failed. GLE=%d\n", GetLastError());
		return FALSE;
	}

	Disconnect();

	return TRUE;
}

