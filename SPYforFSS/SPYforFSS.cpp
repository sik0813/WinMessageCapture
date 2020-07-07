#include "commonInclude.h"
#include "CMainDlg.h"
#include "../MessageSYP/MessageSPY.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	HANDLE otherBitQuitEvent = CreateEventW(NULL, TRUE, TRUE, L"otherBitDllQuit");
	ResetEvent(otherBitQuitEvent);
	CMainDlg mainDlg;
	BOOL succFunc = mainDlg.Start(hInstance);
	StartHook();

	// 64 bit dll Injection & Hooking
	WCHAR rundllPath[] = L"C:\\Windows\\System32\\";
	WCHAR programPath[_MAX_PATH];
	GetModuleFileName(NULL, programPath, _MAX_PATH);
	std::wstring paramString = L"\"" + std::wstring(programPath);
	paramString = paramString.substr(0, paramString.find_last_of(L'\\') + 1);
	paramString += L"MessageSYP_amd64.dll\" SpyStart";

	SHELLEXECUTEINFO seInfo;
	memset(&seInfo, 0, sizeof(SHELLEXECUTEINFO));
	seInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	seInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	seInfo.lpVerb = L"open";
	seInfo.lpDirectory = rundllPath;
	seInfo.lpFile = L"rundll32.exe";
	seInfo.lpParameters = paramString.data();
	seInfo.nShow = SW_HIDE;
	ShellExecuteEx(&seInfo);

	mainDlg.Show();

	mainDlg.End();

	StopHook();	
	SetEvent(otherBitQuitEvent);
	CloseHandle(otherBitQuitEvent);
	WaitForSingleObject(seInfo.hProcess, INFINITE);

	return 0;
}