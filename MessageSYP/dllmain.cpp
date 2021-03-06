// dllmain.cpp : DLL 응용 프로그램의 진입점을 정의합니다.
#include "stdafx.h"
#include "MessageSPY.h"
#include <Psapi.h>
#include <strsafe.h>
#include <process.h>
#include "CClient.h"


#define NUMHOOK 3


HINSTANCE kdllInstance = NULL;
static HHOOK hookList[NUMHOOK];
CClient *nowClient = NULL;


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	WCHAR m_processName[MAX_PATH] = { 0, };
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		
		GetModuleFileName(NULL, m_processName, sizeof(m_processName) / sizeof(WCHAR));
		if (NULL == m_processName)
		{
			return TRUE;
		}
		StringCchCopyW(m_processName, MAX_PATH, wcsrchr(m_processName, L'\\') + 1);
		if (NULL == m_processName)
		{
			return TRUE;
		}
		kdllInstance = (HINSTANCE)hModule;
		nowClient = new CClient();
		nowClient->Start(m_processName);
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		nowClient->End();
		delete nowClient;
		break;
	}
	return TRUE;
}

EXPORT void CALLBACK SpyStart(HWND hwnd, HINSTANCE hInstance, LPTSTR lpszCmdLine, int nCmdShow)
{
	HANDLE otherBitQuitEvent = CreateEventW(NULL, TRUE, TRUE, L"otherBitDllQuit");
	ResetEvent(otherBitQuitEvent);
	StartHook();

	WaitForSingleObject(otherBitQuitEvent, INFINITE);
	CloseHandle(otherBitQuitEvent);

	StopHook();
}

// 메시지 후킹 시작
EXPORT void StartHook(DWORD threadID)
{
	hookList[MSG_CALLWND] = SetWindowsHookExW(WH_CALLWNDPROC, (HOOKPROC)CallWndProc, kdllInstance, threadID); // send
	hookList[MSG_CALLWNDRET] = SetWindowsHookExW(WH_CALLWNDPROCRET, (HOOKPROC)CallWndRetProc, kdllInstance, threadID); // return
	hookList[MSG_GETMSG] = SetWindowsHookExW(WH_GETMESSAGE, (HOOKPROC)GetMsgProc, kdllInstance, threadID); // post
	return;
}


EXPORT void StopHook(void)
{
	UnhookWindowsHookEx(hookList[MSG_CALLWND]);
	UnhookWindowsHookEx(hookList[MSG_CALLWNDRET]);
	UnhookWindowsHookEx(hookList[MSG_GETMSG]);
	return;
}

/****************************************************************
WH_CALLWND hook procedure
****************************************************************/
LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	const int hookIndex = MSG_CALLWND;

	if (deniedProcessList[nowClient->m_processName] ||
		NULL == nowClient ||
		nCode < 0)
	{
		return CallNextHookEx(hookList[hookIndex], nCode, wParam, lParam);
	}

	if (false == nowClient->m_processName.empty())
	{
		switch (nCode)
		{
		case HC_ACTION:
			if (0 == wParam)
			{
				// other thread
				LPCWPSTRUCT nowMsg = (LPCWPSTRUCT)lParam;
				nowClient->MakeMsg(nCode, wParam, lParam, hookIndex);
				nowClient->SendMsg();
			}
			else
			{
				// current thread
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

	if (deniedProcessList[nowClient->m_processName] ||
		NULL == nowClient ||
		nCode < 0)
	{
		return CallNextHookEx(hookList[hookIndex], nCode, wParam, lParam);
	}

	if (false == nowClient->m_processName.empty())
	{
		switch (nCode)
		{
		case HC_ACTION:
			if (NULL == wParam)
			{
				// other process
			}
			else
			{
				// current process
				CWPRETSTRUCT *nowMsg = (CWPRETSTRUCT*)lParam;
				nowClient->MakeMsg(nCode, wParam, lParam, hookIndex);
				nowClient->SendMsg();
			}
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

	if (deniedProcessList[nowClient->m_processName] ||
		NULL == nowClient ||
		nCode < 0)
	{
		return CallNextHookEx(hookList[hookIndex], nCode, wParam, lParam);
	}

	if (false == nowClient->m_processName.empty())
	{
		switch (nCode)
		{
		case HC_ACTION:
			switch (wParam)
			{
			case PM_NOREMOVE:
				break;

			case PM_REMOVE:
				MSG *nowMsg = (MSG*)lParam;
				nowClient->MakeMsg(nCode, wParam, lParam, hookIndex);
				nowClient->SendMsg();
				break;
			}
			break;
		}
	}

	return CallNextHookEx(hookList[hookIndex], nCode, wParam, lParam);
}

BOOL CClient::m_readListQuit = FALSE;
BOOL CClient::m_sendFlag = FALSE;

CClient::CClient()
{}

CClient::~CClient()
{}

void CClient::Start(LPWSTR _processName)
{
	m_processName = std::wstring(_processName);
	for (int i = 0; i < m_processName.size(); i++)
	{
		m_processName[i] = towlower(m_processName[i]);
	}

	if (TRUE == deniedProcessList[m_processName])
	{
		return;
	}

	StringCchCopyW(m_curSendData.processName, wcslen(_processName) + 1, m_processName.data());
	m_curSendData.processID = GetCurrentProcessId();
	m_curSendData.hInstance = (INT64)GetModuleHandleW(NULL);


	m_listWriteDone = CreateEventW(NULL, TRUE, TRUE, m_writeDoneEvent);

	m_readListHandle = (HANDLE)_beginthreadex(NULL, 0, ReadListThread, (LPVOID)this, 0, NULL);
}

void CClient::End()
{
	if (TRUE == deniedProcessList[m_processName])
	{
		return;
	}

	m_readListQuit = TRUE;
	UnmapViewOfFile(m_pBuf);
	CloseHandle(m_listWriteDone);
	CloseHandle(m_hMapFile);
	
	DWORD retFunc = WaitForSingleObject(m_readListHandle, 10000);
	if (WAIT_TIMEOUT == retFunc)
	{
		TerminateThread(m_readListHandle, 0);
	}

	m_readListHandle = INVALID_HANDLE_VALUE;
	CloseHandle(m_readListHandle);
}

BOOL CClient::Connect()
{
	m_pipeHandle = CreateFileW(
		m_pipeName,   // pipe name 
		GENERIC_READ | GENERIC_WRITE, // read and write access 
		0,              // no sharing 
		NULL,           // default security attributes
		OPEN_EXISTING,  // opens existing pipe 
		0,              // default attributes 
		NULL);          // no template file 

	if (INVALID_HANDLE_VALUE == m_pipeHandle)
	{
		if (ERROR_PIPE_BUSY != GetLastError())
		{
			wprintf(L"Could not open pipe. GLE=%d\n", GetLastError());
			return FALSE;
		}

		// 5초간 파이프 핸들 대기
		if (!WaitNamedPipe(m_pipeName, PIPE_TIMEOUT))
		{
			printf("Could not open pipe: 5 second wait timed out.");
			return FALSE;
		}
	}

	DWORD pipeMode = PIPE_READMODE_MESSAGE;
	BOOL fSuccess = SetNamedPipeHandleState(
		m_pipeHandle,    // pipe handle 
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
	FlushFileBuffers(m_pipeHandle);
	CloseHandle(m_pipeHandle);
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
		m_hMapFile = OpenFileMapping(
			FILE_MAP_ALL_ACCESS,   // read/write access
			FALSE,                 // do not inherit the name
			m_sharedMemName);               // name of mapping object
		if (m_hMapFile == NULL)
		{
			wprintf(L"Could not create file mapping object (%d).\n", GetLastError());
			break;
		}

		m_pBuf = (LPWSTR)MapViewOfFile(m_hMapFile, // handle to map object
			FILE_MAP_ALL_ACCESS,  // read/write permission
			0,
			0,
			BUF_SIZE);
		if (m_pBuf == NULL)
		{
			wprintf(L"Could not map view of file (%d).\n", GetLastError());
			CloseHandle(m_hMapFile);
			m_hMapFile = INVALID_HANDLE_VALUE;
			break;
		}

		if (TRUE == m_readListQuit
			|| (m_pBuf[0] == 0xFFFF && m_pBuf[1] == 0xFFFF))
		{
			break;
		}

		std::wstring nowData(m_pBuf);
		size_t subLocation = nowData.find(m_processName);
		if (std::wstring::npos == subLocation)
		{
			m_sendFlag = FALSE;
		}
		else
		{
			m_sendFlag = TRUE;
		}

		UnmapViewOfFile(m_pBuf);
		m_pBuf = NULL;
		CloseHandle(m_hMapFile);
		m_hMapFile = INVALID_HANDLE_VALUE;

		WaitForSingleObject(m_listWriteDone, INFINITE); // 60sec
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

	default:
		break;
	}

	LPCREATESTRUCTW tmpCSpt = NULL;
	LPWINDOWPOS tmpWPpt = NULL;
	LPSTYLESTRUCT tmpSSpt = NULL;

	switch (sendMsgCode)
	{
	case WM_CREATE:
		tmpCSpt = (LPCREATESTRUCTW)sendlParam;
		memset(m_curSendData.otherData, 0, MAX_PATH);
		wsprintf(m_curSendData.otherData, L"x:%d | y:%d | cx:%d | cy:%d",
			tmpCSpt->x, tmpCSpt->y, tmpCSpt->cx, tmpCSpt->cy);
		break;

	case WM_WINDOWPOSCHANGED:
		tmpWPpt = (LPWINDOWPOS)sendlParam;
		memset(m_curSendData.otherData, 0, MAX_PATH);
		wsprintf(m_curSendData.otherData, L"x:%d | y:%d | cx:%d | cy:%d | flags:%d",
			tmpWPpt->x, tmpWPpt->y, tmpWPpt->cx, tmpWPpt->cy, tmpWPpt->flags);
		break;

	case WM_STYLECHANGED:
		tmpSSpt = (LPSTYLESTRUCT)sendlParam;
		memset(m_curSendData.otherData, 0, MAX_PATH);
		wsprintf(m_curSendData.otherData, L"old:%d,new:%d",
			tmpSSpt->styleOld, tmpSSpt->styleNew);
		break;

	case WM_SETTEXT:
		memset(m_curSendData.otherData, 0, MAX_PATH);
		wsprintf(m_curSendData.otherData, L"%s",
			(LPWSTR)sendlParam);
		break;
		
	default:
		memset(m_curSendData.otherData, 0, MAX_PATH);
		break;
	}

	m_curSendData.hookType = _hookType;
	m_curSendData.msgCode = sendMsgCode;
	m_curSendData.hwnd = (INT64)sendHwnd;
	m_curSendData.wParam = sendwParam;
	m_curSendData.lParam = sendlParam;

	m_curSendData.threadID = GetThreadId(GetCurrentThread());
}

BOOL CClient::SendMsg()
{
	if (FALSE == m_sendFlag)
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
	DWORD sendMsgLen = sizeof(MsgData);
	succFunc = WriteFile(
		m_pipeHandle,                  // pipe handle 
		&m_curSendData,             // message 
		sendMsgLen,              // message length 
		&cbWritten,             // bytes written 
		&ov);                  // overlapped 
	if (FALSE == succFunc)
	{
		DWORD lastError = GetLastError();
		if (ERROR_IO_PENDING == lastError)
		{
			lastError = WaitForSingleObject(ov.hEvent, 10000);
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

