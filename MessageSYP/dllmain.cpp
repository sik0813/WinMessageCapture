// dllmain.cpp : DLL 응용 프로그램의 진입점을 정의합니다.
#include "stdafx.h"
#include "MessageSYP.h"
#include <Psapi.h>
#include <strsafe.h>
#include <process.h>
#include "CClient.h"

#define SETTINGDATALEN 20

HINSTANCE kdllInstanceHandle = NULL;
static HHOOK kconnnectHook = NULL;
CClient* curClient = NULL;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	WCHAR processName[MAX_PATH] = { 0, };
	DWORD successFunc = GetModuleFileName(NULL, processName, sizeof(processName) / sizeof(WCHAR));
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		kdllInstanceHandle = (HINSTANCE)hModule;
		if (NULL != wcsrchr(processName, L'\\'))
		{
			if (0 == wcscmp(L"SPYforFSS.exe", wcsrchr(processName, L'\\') + 1))
			{
				return TRUE;
			}

			curClient = new CClient(wcsrchr(processName, L'\\') + 1);
		}

		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		if (NULL != wcsrchr(processName, L'\\'))
		{
			if (0 == wcscmp(L"SPYforFSS.exe", wcsrchr(processName, L'\\') + 1))
			{
				return TRUE;
			}

			delete curClient;
			curClient = NULL;
		}
		break;
	}
	return TRUE;
}

// 메시지 후킹 시작
EXPORT void StartHook(DWORD threadID)
{
	kconnnectHook = SetWindowsHookExW(WH_CALLWNDPROC, (HOOKPROC)CallWndProc, kdllInstanceHandle, threadID);
	return;
}


EXPORT void StopHook(void)
{
	UnhookWindowsHookEx(kconnnectHook);
	return;
}

LRESULT WINAPI CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	WCHAR processName[MAX_PATH] = { 0, };
	GetModuleFileNameEx(GetCurrentProcess(), 0, processName, sizeof(processName) / sizeof(WCHAR));
	if (0 == wcscmp(L"SPYforFSS.exe", wcsrchr(processName, L'\\') + 1))
	{
		return CallNextHookEx(kconnnectHook, nCode, wParam, lParam);
	}

	if (nCode < 0)
		return CallNextHookEx(kconnnectHook, nCode, wParam, lParam);

	LPWSTR compProcessName = wcsrchr(processName, L'\\');
	if (NULL != compProcessName)
	{
		switch (nCode)
		{
		case HC_ACTION:
			if (0 != wParam)
			{
				if (1 == curClient->GetActiveFlage())
				{
					curClient->CheckMsg((LPCWPSTRUCT)lParam);
				}
				
			}
			break;

		default:
			break;
		}
	}

	return CallNextHookEx(kconnnectHook, nCode, wParam, lParam);
}

CClient::CClient(LPWSTR _processName)
{
	memset(&currentProcessName, 0, sizeof(currentProcessName));
	StringCchCopyW(currentProcessName, MAX_PATH + 1, _processName);
	memset(&currentData, 0, sizeof(currentData));
	checkThreadHandle = (HANDLE)_beginthreadex(NULL, 0, CheckMemoryThread, this, NULL, NULL);
}

CClient::~CClient() 
{
	quitFlag = TRUE;
	WaitForSingleObject(checkThreadHandle, INFINITE);
	CloseHandle(checkThreadHandle);
	UnmapViewOfFile(connectMemory);
	CloseHandle(connectMemory);
}

// 공유 메모리 영역 확인 함수
BOOL CClient::CheckMemory()
{
	while (1) 
	{
		if (NULL == connectMemory)
		{
			while (1) 
			{
				// 공유메모리 연결
				connectMemory = OpenFileMapping(
					FILE_MAP_READ | FILE_MAP_WRITE,
					FALSE,
					L"Local\\SPYFORFSS");
				if (NULL != connectMemory) 
				{
					// 공유 메모리 버퍼 연결
					recvSettingDataBuf = (settingData*)MapViewOfFile(
						connectMemory,
						PAGE_READONLY,
						0,
						0,
						0);
					break;
				}

				if (quitFlag)
				{
					break;
				}
			}
		}

		for (int i = 0; i < SETTINGDATALEN; i++) 
		{
			if (NULL == recvSettingDataBuf[i].wndHwnd)
			{
				break;
			}

			if (0 == wcscmp(recvSettingDataBuf[i].processName, currentProcessName))
			{
				currentData = recvSettingDataBuf[i];
			}
		}

		if (quitFlag)
		{
			break;
		}

	}
	return TRUE;
}

// 공유 메모리 영역 확인 Thread
unsigned int WINAPI CClient::CheckMemoryThread(void* arg)
{
	((CClient*)arg)->CheckMemory();
	return 0;
}

CHAR CClient::GetActiveFlage()
{
	return (currentData.activeFlag);
}

// 지정된 옵션에 맞는 Msg여부 확인후 문자열 생성(FALSE 반환시 )
BOOL CClient::CheckMsg(LPCWPSTRUCT targetMsg)
{
	DWORD fstSelect = targetMsg->message / 8;
	DWORD sndSelect = targetMsg->message % 8;
	DWORD andCalcResult = currentData.option.msgOption[fstSelect] & sndSelect;
	sendMsg = L"";
	if (0 == andCalcResult)
	{
		return FALSE;
	}

	sendMsg += L"<";
	std::wstring counterString = std::to_wstring(counter);
	size_t counterStringLen = counterString.size();
	for (int i = 0; i < 8 - counterStringLen; i++)
	{
		sendMsg += L"0";
	}
	sendMsg += counterString + L">";
	

	if (NULL != wmTranslation[targetMsg->message])
	{
		sendMsg += L"unknown(";
	}
	else
	{
		sendMsg += std::wstring(wmTranslation[targetMsg->message]) + L"(";
	}
	sendMsg += std::to_wstring(targetMsg->message);


	
	return TRUE;
}

// 출력 메시지를 Decoing 하는 부분
BOOL CClient::DecodeMsg()
{
	return TRUE;
}

// 만들어진 Msg 문자열 출력
BOOL CClient::PrintMsg()
{
	return TRUE;
}

