// dllmain.cpp : DLL 응용 프로그램의 진입점을 정의합니다.
#include "stdafx.h"
#include "MessageSYP.h"
#include <Psapi.h>
#include <strsafe.h>
#include "CClient.h"

HINSTANCE kdllInstanceHandle = NULL;
static HHOOK kconnnectHook = NULL;

BOOL APIENTRY DllMain(HMODULE hModule,	DWORD  ul_reason_for_call,	LPVOID lpReserved)
{
	WCHAR szProcessName[MAX_PATH] = { 0, };
	DWORD successFunc = GetModuleFileName(NULL, szProcessName, sizeof(szProcessName) / sizeof(WCHAR));
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		kdllInstanceHandle = (HINSTANCE)hModule;
		if (NULL != wcsrchr(szProcessName, L'\\'))
		{
			if (0 == wcscmp(L"notepad.exe", wcsrchr(szProcessName, L'\\') + 1))
			{
				//Client = new ClientPipe();
			}
		}

		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		if (NULL != wcsrchr(szProcessName, L'\\'))
		{
			if (0 == wcscmp(L"notepad.exe", wcsrchr(szProcessName, L'\\') + 1))
			{
				//delete Client;
			}
		}
		break;
	}
	return TRUE;
}

EXPORT void StartHook(DWORD threadID)
{
	kconnnectHook = SetWindowsHookExW(WH_CALLWNDPROC, (HOOKPROC)GetMsgProc, kdllInstanceHandle, threadID);
	return;
}


EXPORT void StopHook(void)
{
	UnhookWindowsHookEx(kconnnectHook);
	return;
}

LRESULT WINAPI GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	WCHAR szProcessName[MAX_PATH] = { 0, };
	GetModuleFileNameEx(GetCurrentProcess(), 0, szProcessName, sizeof(szProcessName) / sizeof(WCHAR));

	if (nCode < 0)
		return CallNextHookEx(kconnnectHook, nCode, wParam, lParam);

	LPWSTR compProcessName = wcsrchr(szProcessName, L'\\');
	if (NULL != compProcessName)
	{
		switch (nCode)
		{
		case HC_ACTION:
			switch (wParam)
			{
			case PM_REMOVE:
			case PM_NOREMOVE:
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
	}

	return CallNextHookEx(kconnnectHook, nCode, wParam, lParam);
}

BOOL WINAPI CClient::CheckMemory()
{
	while (1)
	{

		Sleep(300);
	}
}

