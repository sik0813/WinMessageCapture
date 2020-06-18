#pragma once
#include <Windows.h>
#include <windowsx.h>
#include <string>
#include <vector>
#include <map>
#include "resource.h"

#define MSG_LEN 64

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
