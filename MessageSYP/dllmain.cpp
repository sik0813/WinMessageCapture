// dllmain.cpp : DLL 응용 프로그램의 진입점을 정의합니다.
#include "stdafx.h"
#include "MessageSYP.h"
#include <Psapi.h>
#include <strsafe.h>
#include <process.h>
#include "CClient.h"

#define SETTINGDATALEN 20

HINSTANCE kdllInstance = NULL;
static HHOOK kCallWnd = NULL;
static HHOOK kHook = NULL;
static HHOOK kGetMsg = NULL;
CClient *nowClient = NULL;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	WCHAR processName[MAX_PATH] = { 0, };
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		kdllInstance = (HINSTANCE)hModule;
		GetModuleFileName(NULL, processName, sizeof(processName) / sizeof(WCHAR));
		nowClient = new CClient(processName, GetCurrentProcessId());
		//if (0 == wcscmp(nowClient->GetProcessName(), L"notepad.exe"))
		{
			nowClient->Connect();
		}
		
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		nowClient->~CClient();
		delete nowClient;
		break;
	}
	return TRUE;
}

// 메시지 후킹 시작
EXPORT void StartHook(DWORD threadID)
{
	kCallWnd = SetWindowsHookExW(WH_CALLWNDPROC, (HOOKPROC)CallWndProc, kdllInstance, threadID); // send
	//kHook = SetWindowsHookExW(WH_CALLWNDPROCRET, (HOOKPROC)Hookproc, kdllInstance, threadID); // return
	//kGetMsg = SetWindowsHookExW(WH_GETMESSAGE, (HOOKPROC)GetMsgProc, kdllInstance, threadID); // post
	return;
}


EXPORT void StopHook(void)
{
	UnhookWindowsHookEx(kCallWnd);
	//UnhookWindowsHookEx(kHook);
	//UnhookWindowsHookEx(kGetMsg);
	return;
}

// SendMSG
LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (0 == wcscmp(L"SPYforFSS.exe", nowClient->GetProcessName()) ||
		nCode < 0)
	{
		return CallNextHookEx(kCallWnd, nCode, wParam, lParam);
	}

	//if(0 != wcscmp(L"notepad.exe", nowClient->GetProcessName()))
	//	return CallNextHookEx(kCallWnd, nCode, wParam, lParam);
	
	if (NULL != nowClient->GetProcessName())
	{
		switch (nCode)
		{
		case HC_ACTION:
			if (0 != wParam)
			{
				LPCWPSTRUCT nowMsg = (LPCWPSTRUCT)lParam;
				nowClient->MakeMsg(nowMsg->message, wmTranslation[nowMsg->message], nowMsg->wParam, nowMsg->lParam);
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
LRESULT Hookproc(int nCode, WPARAM wParam, LPARAM lParam)
{
	return CallNextHookEx(kHook, nCode, wParam, lParam);
}

// PostMSG
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	return CallNextHookEx(kGetMsg, nCode, wParam, lParam);
}

CClient::CClient(LPWSTR _processName, DWORD _processID)
{	
	StringCchCopyW(curSendData.processName, MAX_PATH, wcsrchr(_processName, L'\\') + 1);
	curSendData.processID = _processID;
	
	writeEvent = CreateEventW(NULL, FALSE, TRUE, L"SPYFSSwirterE");
	readerEvent = CreateEventW(NULL, FALSE, TRUE, L"SPYFSSreaderE");
	writeMutex = OpenMutexW(SYNCHRONIZE, TRUE, L"SPYFSSwirterM");
	readerMutex = OpenMutexW(SYNCHRONIZE, TRUE, L"SPYFSSreaderM");
}

CClient::~CClient()
{
	UnmapViewOfFile(sendDataBuf);
	CloseHandle(sharedMemory);
	CloseHandle(writeEvent);
	CloseHandle(readerEvent);
	CloseHandle(writeMutex);
	CloseHandle(readerMutex);
}

int CClient::Connect()
{
	sharedMemory = OpenFileMappingW(
		FILE_MAP_ALL_ACCESS,
		TRUE,
		sharedMemoryName);
	if (sharedMemory == NULL)
	{
		return 1;
	}

	sendDataBuf = (LPSendData)MapViewOfFile(sharedMemory,   // handle to map object
		FILE_MAP_ALL_ACCESS, // read/write permission
		0,
		0,
		sizeof(sendData));
	if (sendDataBuf == NULL)
	{
		CloseHandle(sharedMemory);
		return 1;
	}

	return 0;
}

void CClient::MakeMsg(DWORD _MessageCode, LPWSTR _MessageContent, WPARAM _wParam, LPARAM _lParam)
{
	curSendData.MessageCode = _MessageCode;
	if (NULL == _MessageContent)
	{
		StringCchCopyW(curSendData.MessageContent, 64, L"unknown");
	}
	else
	{
		StringCchCopyW(curSendData.MessageContent, 64, _MessageContent);
	}
	
	curSendData.wParam = _wParam;
	curSendData.lParam = _lParam;
}

BOOL CClient::SendMsg()
{
	WaitForSingleObject(writeMutex, INFINITE);
	memcpy(sendDataBuf, &curSendData, sizeof(curSendData));
	SetEvent(readerEvent);
	WaitForSingleObject(writeEvent, INFINITE);
	ReleaseMutex(writeMutex);
	return TRUE;
}

LPWSTR CClient::GetProcessName()
{
	return curSendData.processName;
}


