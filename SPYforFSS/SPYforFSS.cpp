#include "commonInclude.h"
#include "CMainDlg.h"
#include "../MessageSYP/MessageSYP.h"

settingData sendSettingData[20];
HANDLE mappingHandle = NULL;
settingData* sendBuf = NULL;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	memset(&sendSettingData[0], 0, sizeof(sendSettingData));
	mappingHandle = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		sizeof(sendSettingData),
		L"Local\\SPYFORFSS"
	);
	if (NULL == mappingHandle) {
		wprintf(L"mapping Fail\n");
		return 0;
	}

	sendBuf = (settingData*)MapViewOfFile(
		mappingHandle,
		PAGE_READONLY,
		0,
		0,
		0
	);

	//StartHook();
	CMainDlg tmp;
	tmp.Show(hInstance);
	//StopHook();

	UnmapViewOfFile(mappingHandle);
	CloseHandle(mappingHandle);

	return 0;
}