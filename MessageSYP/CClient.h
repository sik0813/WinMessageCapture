#pragma once
#include <Windows.h>
#include <string>
#define OPTIONLEN 8192

/*
msgOption
msgOption[WM / 8] & (WM % 8)

printOption
1 : �� ��ȣ
2 : ���� �޽��� �Ű� ����
4 : ���ڵ��� �޽��� �Ű� ����
8 : ���� ��ȯ ��
16 : ���ڵ��� ��ȯ ��
32 : �޽��� �߻� �ð�
64 : �޽��� ���콺 ��ġ
*/
typedef struct options {
	CHAR msgOption[OPTIONLEN]; // 8192(������ �޽��� ũ�� 1byte)
	CHAR printOption; // 1
	DWORD maxLine; // 4
}options;

typedef struct settingData {
	WCHAR processName[MAX_PATH]; // 260 * 2 = 520
	HWND wndHwnd; // 8
	CHAR activeFlag; // 1
	options option; // 4
}settingData;

class CClient 
{
public:
	CClient(LPWSTR processName);
	~CClient();

private:
	HANDLE checkThreadHandle = INVALID_HANDLE_VALUE;
	HANDLE connectMemory = NULL;
	settingData* recvSettingDataBuf = NULL;
	std::wstring sendMsg = L"";
	settingData currentData;
	DWORD counter = 0;
	BOOL quitFlag = FALSE;

public:
	WCHAR currentProcessName[MAX_PATH];	

public:
	BOOL CheckMemory();
	static unsigned int WINAPI CheckMemoryThread(void* arg);
	CHAR GetActiveFlage();
	BOOL CheckMsg(LPCWPSTRUCT targetMsg);
	BOOL DecodeMsg();
	BOOL PrintMsg();
};