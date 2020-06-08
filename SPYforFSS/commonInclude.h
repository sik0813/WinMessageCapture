#pragma once
#include <Windows.h>
#include <windowsx.h>
#include <string>
#include <vector>

#include "resource.h"

#define OPTIONLEN 8192

typedef struct options {
	CHAR msgOption[OPTIONLEN]; // 8192(윈도우 메시지 크기 1byte)
	CHAR printOption; // 1
	DWORD maxLine; // 4
}options;

typedef struct settingData {
	WCHAR processName[MAX_PATH]; // 260 * 2 = 520
	HWND wndHwnd; // 8
	CHAR activeFlag; // 1
	options option;
}settingData;

extern settingData sendSettingData[20];
extern HANDLE mappingHandle;
extern settingData* sendBuf;