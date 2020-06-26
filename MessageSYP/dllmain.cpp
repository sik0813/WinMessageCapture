// dllmain.cpp : DLL 응용 프로그램의 진입점을 정의합니다.
#include "stdafx.h"
#include "MessageSYP.h"
#include <Psapi.h>
#include <strsafe.h>
#include <process.h>
#include "CClient.h"


#define NUMHOOK 7


HINSTANCE kdllInstance = NULL;
static HHOOK hookList[NUMHOOK];

CClient *nowClient = NULL;

LPCWSTR deniedProcessName = L"SPYforFSS.exe";

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	WCHAR processName[MAX_PATH] = { 0, };
	GetModuleFileName(NULL, processName, sizeof(processName) / sizeof(WCHAR));
	if (NULL == processName)
	{
		return TRUE;
	}
	StringCchCopyW(processName, MAX_PATH, wcsrchr(processName, L'\\') + 1);	
	if (NULL == processName)
	{
		return TRUE;
	}

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		kdllInstance = (HINSTANCE)hModule;		
		nowClient = new CClient();
		nowClient->Start(processName);
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		//nowClient->End();
		delete nowClient;
		break;
	}
	return TRUE;
}

// 메시지 후킹 시작
EXPORT void StartHook(DWORD threadID)
{
	hookList[MSG_CALLWND] = SetWindowsHookExW(WH_CALLWNDPROC, (HOOKPROC)CallWndProc, kdllInstance, threadID); // send
	hookList[MSG_CALLWNDRET] = SetWindowsHookExW(WH_CALLWNDPROCRET, (HOOKPROC)CallWndRetProc, kdllInstance, threadID); // return
	hookList[MSG_GETMSG] = SetWindowsHookExW(WH_GETMESSAGE, (HOOKPROC)GetMsgProc, kdllInstance, threadID); // post
	//hookList[MSG_KEYBOARD] = SetWindowsHookExW(WH_KEYBOARD, (HOOKPROC)KeyboardProc, kdllInstance, threadID); // 
	//hookList[MSG_MOUSE] = SetWindowsHookExW(WH_MOUSE, (HOOKPROC)MouseProc, kdllInstance, threadID); // 
	//hookList[MSG_MSGFILTER] = SetWindowsHookExW(WH_MSGFILTER, (HOOKPROC)MsgFilterProc, kdllInstance, threadID); // 
	return;
}


EXPORT void StopHook(void)
{
	UnhookWindowsHookEx(hookList[MSG_CALLWND]);
	UnhookWindowsHookEx(hookList[MSG_CALLWNDRET]);
	UnhookWindowsHookEx(hookList[MSG_GETMSG]);
	//UnhookWindowsHookEx(hookList[MSG_KEYBOARD]);
	//UnhookWindowsHookEx(hookList[MSG_MOUSE]);
	//UnhookWindowsHookEx(hookList[MSG_MSGFILTER]);	
	return;
}

/****************************************************************
WH_CALLWND hook procedure
****************************************************************/
LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	const int hookIndex = MSG_CALLWND;

	if (deniedProcessList[nowClient->processName] ||
		NULL == nowClient ||
		nCode < 0)
	{
		return CallNextHookEx(hookList[hookIndex], nCode, wParam, lParam);
	}
		
	if (false == nowClient->processName.empty())
	{
		switch (nCode)
		{
		case HC_ACTION:
			if (0 != wParam)
			{
				LPCWPSTRUCT nowMsg = (LPCWPSTRUCT)lParam;
				nowClient->MakeMsg(nCode, wParam, lParam, hookIndex);
				nowClient->SendMsg();
			}
			break;

		default:
			break;
		}
	}

	return CallNextHookEx(hookList[hookIndex], nCode, wParam, lParam);
}

/****************************************************************
WH_CALLWNDRET hook procedure
****************************************************************/
LRESULT CallWndRetProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	const int hookIndex = MSG_CALLWNDRET;

	if (deniedProcessList[nowClient->processName] ||
		NULL == nowClient ||
		nCode < 0)
	{
		return CallNextHookEx(hookList[hookIndex], nCode, wParam, lParam);
	}

	if (false == nowClient->processName.empty())
	{
		switch (nCode)
		{
		case HC_ACTION:
			if (0 != wParam)
			{
				CWPRETSTRUCT *nowMsg = (CWPRETSTRUCT*)lParam;
				nowClient->MakeMsg(nCode, wParam, lParam, hookIndex);
				nowClient->SendMsg();
			}
			break;

		default:
			break;
		}
	}

	return CallNextHookEx(hookList[hookIndex], nCode, wParam, lParam);
}

/****************************************************************
WH_GETMSG hook procedure
****************************************************************/
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	const int hookIndex = MSG_GETMSG;

	if (deniedProcessList[nowClient->processName] ||
		NULL == nowClient ||
		nCode < 0)
	{
		return CallNextHookEx(hookList[hookIndex], nCode, wParam, lParam);
	}

	if (false == nowClient->processName.empty())
	{
		switch (nCode)
		{
		case HC_ACTION:
			if (0 != wParam)
			{
				MSG *nowMsg = (MSG*)lParam;
				nowClient->MakeMsg(nCode, wParam, lParam, hookIndex);
				nowClient->SendMsg();
			}
			break;

		default:
			break;
		}
	}

	return CallNextHookEx(hookList[hookIndex], nCode, wParam, lParam);
}

/****************************************************************
WH_MOUSE hook procedure
****************************************************************/
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	const int hookIndex = MSG_MOUSE;

	if (deniedProcessList[nowClient->processName] ||
		NULL == nowClient ||
		nCode < 0)
	{
		return CallNextHookEx(hookList[hookIndex], nCode, wParam, lParam);
	}

	if (false == nowClient->processName.empty())
	{
		nowClient->MakeMsg(nCode, wParam, lParam, hookIndex); // wParam: Mouse Message, 
		nowClient->SendMsg();
	}

	return CallNextHookEx(hookList[hookIndex], nCode, wParam, lParam);
}

/****************************************************************
WH_KEYBOARD hook procedure
****************************************************************/
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	const int hookIndex = MSG_KEYBOARD;
	
	if (deniedProcessList[nowClient->processName] ||
		NULL == nowClient ||
		nCode < 0)
	{
		return CallNextHookEx(hookList[hookIndex], nCode, wParam, lParam);
	}

	if (false == nowClient->processName.empty())
	{
		nowClient->MakeMsg(nCode, wParam, lParam, hookIndex); // wParam: Mouse Message, 
		nowClient->SendMsg();
	}

	return CallNextHookEx(hookList[hookIndex], nCode, wParam, lParam);
}

/****************************************************************
WH_MSGFILTER hook procedure
****************************************************************/
LRESULT CALLBACK MsgFilterProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	const int hookIndex = MSG_MSGFILTER;
	WCHAR szBuf[128];
	WCHAR szMsg[16];
	WCHAR szCode[32];
	static int c = 0;
	size_t cch;
	HRESULT hResult;

	if (nCode < 0)  // do not process message 
		return CallNextHookEx(hookList[hookIndex], nCode, wParam, lParam);

	switch (nCode)
	{
	case MSGF_DIALOGBOX:
		hResult = StringCchCopy(szCode, 32 / sizeof(TCHAR), L"MSGF_DIALOGBOX");
		if (FAILED(hResult))
		{
			// TODO: write error handler
		}
		break;

	case MSGF_MENU:
		hResult = StringCchCopy(szCode, 32 / sizeof(TCHAR), L"MSGF_MENU");
		if (FAILED(hResult))
		{
			// TODO: write error handler
		}
		break;

	case MSGF_SCROLLBAR:
		hResult = StringCchCopy(szCode, 32 / sizeof(TCHAR), L"MSGF_SCROLLBAR");
		if (FAILED(hResult))
		{
			// TODO: write error handler
		}
		break;

	default:
		hResult = StringCchPrintf(szCode, 128 / sizeof(TCHAR), L"Unknown: %d", nCode);
		if (FAILED(hResult))
		{
			// TODO: write error handler
		}
		break;
	}


	hResult = StringCchPrintf(szBuf, 128 / sizeof(TCHAR),
		L"MSGFILTER  nCode: %s, msg: %s, %d times    ",
		szCode, szMsg, c++);
	if (FAILED(hResult))
	{
		// TODO: write error handler
	}
	hResult = StringCchLength(szBuf, 128 / sizeof(TCHAR), &cch);
	if (FAILED(hResult))
	{
		// TODO: write error handler
	}

	return CallNextHookEx(hookList[hookIndex], nCode, wParam, lParam);
}



BOOL CClient::readListQuit = FALSE;
BOOL CClient::sendFlag = FALSE;

CClient::CClient()
{}

CClient::~CClient() 
{}

void CClient::Start(LPWSTR _processName)
{
	processName = std::wstring(_processName);
	/*for (int i = 0; i < processName.size(); i++)
	{
		processName[i] = towlower(processName[i]);
	}*/
	StringCchCopyW(curSendData.processName, wcslen(_processName) + 1, _processName);
	curSendData.processID = GetCurrentProcessId();

	hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,   // read/write access
		FALSE,                 // do not inherit the name
		sharedMemName);               // name of mapping object
	if (hMapFile == NULL)
	{
		wprintf(L"Could not create file mapping object (%d).\n", GetLastError());
		return;
	}

	pBuf = (LPWSTR)MapViewOfFile(hMapFile, // handle to map object
		FILE_MAP_ALL_ACCESS,  // read/write permission
		0,
		0,
		BUF_SIZE);
	if (pBuf == NULL)
	{
		wprintf(L"Could not map view of file (%d).\n", GetLastError());
		CloseHandle(hMapFile);
		return;
	}

	listWriteDone = CreateEventW(NULL, TRUE, TRUE, wrDoneEvent);

	readListHandle = (HANDLE)_beginthreadex(NULL, 0, ReadListThread, (LPVOID)this, 0, NULL);
}

void CClient::End()
{
	readListQuit = TRUE;
	UnmapViewOfFile(pBuf);
	CloseHandle(listWriteDone);
	CloseHandle(hMapFile);
	WaitForSingleObject(readListHandle, 10000);
	CloseHandle(readListHandle);
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

UINT CClient::ReadListThread(void * arg)
{
	((CClient*)arg)->ReadList();
	return 0;
}

void CClient::ReadList()
{
	while (true)
	{
		if (TRUE == readListQuit)
		{
			break;
		}

		std::wstring nowData(pBuf);
		size_t subLocation = nowData.find(processName);
		if (std::wstring::npos == subLocation)
		{
			sendFlag = FALSE;
		}
		else
		{
			sendFlag = TRUE;
		}

		WaitForSingleObject(listWriteDone, 60000); // 60sec
	}
}

void CClient::MakeMsg(int _nCode, WPARAM _wParam, LPARAM _lParam, int _hookType)
{
	LPCWPSTRUCT callWndMsg = NULL;
	LPCWPRETSTRUCT callWndRetMsg = NULL;
	LPMSG getMsgMsg = NULL;
	LPMOUSEHOOKSTRUCT mouseMsg = NULL;

	DWORD sendMsgCode = 0;
	HWND sendHwnd = NULL;
	WPARAM sendwParam = 0;
	LPARAM sendlParam = NULL;

	switch (_hookType)
	{
	case MSG_CALLWND:
		callWndMsg = (LPCWPSTRUCT)_lParam;
		sendMsgCode = callWndMsg->message;
		sendHwnd = callWndMsg->hwnd;
		sendwParam = callWndMsg->wParam;
		sendlParam = callWndMsg->lParam;
		break;

	case MSG_CALLWNDRET:
		callWndRetMsg = (LPCWPRETSTRUCT)_lParam;
		sendMsgCode = callWndRetMsg->message;
		sendHwnd = callWndRetMsg->hwnd;
		sendwParam = callWndRetMsg->wParam;
		sendlParam = callWndRetMsg->lParam;
		break;

	case MSG_GETMSG:
		getMsgMsg = (LPMSG)_lParam;
		sendMsgCode = getMsgMsg->message;
		sendHwnd = getMsgMsg->hwnd;
		sendwParam = getMsgMsg->wParam;
		sendlParam = getMsgMsg->lParam;
		break;

	case MSG_KEYBOARD:
		sendMsgCode = (DWORD)_wParam;
		sendHwnd = NULL;
		sendwParam = _wParam;
		sendlParam = _lParam;
		break;

	case MSG_MOUSE:
		mouseMsg = (LPMOUSEHOOKSTRUCT)_lParam;
		sendMsgCode = (DWORD)_wParam;
		sendHwnd = mouseMsg->hwnd;
		sendwParam = _wParam;
		sendlParam = _lParam;
		break;

	case MSG_MSGFILTER:
		break;

	default:
		break;
	}
	curSendData.hookType = _hookType;
	curSendData.msgCode = sendMsgCode;
	curSendData.hwnd = sendHwnd;
	curSendData.wParam = sendwParam;
	curSendData.lParam = sendlParam;

	curSendData.threadID = GetThreadId(GetCurrentThread());
}

BOOL CClient::SendMsg()
{
	if (FALSE == sendFlag)
	{
		return FALSE;
	}

	BOOL succFunc = Connect();
	if (FALSE == succFunc)
	{
		Disconnect();
		return FALSE;
	}

	HANDLE writeEventHandle = CreateEventW(NULL, TRUE, TRUE, NULL);
	OVERLAPPED ov;
	memset(&ov, 0, sizeof(ov));
	ov.hEvent = writeEventHandle;

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
		DWORD lastError = GetLastError();
		if (ERROR_IO_PENDING == lastError)
		{
			lastError = WaitForSingleObject(ov.hEvent, 2000);
			if (WAIT_OBJECT_0 != lastError)
			{
				wprintf(L"WriteFile to pipe failed. GLE=%d\n", GetLastError());
				CloseHandle(writeEventHandle);
				Disconnect();
				return FALSE;
			}
		}
		else
		{
			wprintf(L"WriteFile to pipe failed. GLE=%d\n", GetLastError());
			CloseHandle(writeEventHandle);
			Disconnect();
			return FALSE;
		}
	}

	CloseHandle(writeEventHandle);
	Disconnect();

	return TRUE;
}

