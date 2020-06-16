#pragma once
#include <Windows.h>
#include <windowsx.h>
#include <string>
#include <vector>
#include <map>
#include "resource.h"

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
