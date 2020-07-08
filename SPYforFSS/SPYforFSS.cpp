#include "commonInclude.h"
#include "CMainDlg.h"
#include "../MessageSYP/MessageSPY.h"


BOOL Is64BitWindows()
{
#if defined(_WIN64)
	return TRUE;  // 64-bit programs run only on Win64
#elif defined(_WIN32)
	// 32-bit programs run on both 32-bit and 64-bit Windows
	// so must sniff
	BOOL f64 = FALSE;
	return IsWow64Process(GetCurrentProcess(), &f64) && f64;
#else
	return FALSE; // Win64 does not support Win16
#endif
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	BOOL f64 = FALSE;
	BOOL is64BitOS = IsWow64Process(GetCurrentProcess(), &f64) && f64;
	HANDLE otherBitQuitEvent = NULL;
	SHELLEXECUTEINFO seInfo;
	memset(&seInfo, 0, sizeof(SHELLEXECUTEINFO));
	
	CMainDlg mainDlg;
	BOOL succFunc = mainDlg.Start(hInstance);
	StartHook();

	// 64 bit dll Injection & Hooking
	if (is64BitOS)
	{
		otherBitQuitEvent = CreateEventW(NULL, TRUE, TRUE, L"otherBitDllQuit");
		ResetEvent(otherBitQuitEvent);

		WCHAR rundllPath[] = L"C:\\Windows\\System32\\";
		WCHAR programPath[_MAX_PATH];
		GetModuleFileName(NULL, programPath, _MAX_PATH);
		std::wstring paramString = L"\"" + std::wstring(programPath);
		paramString = paramString.substr(0, paramString.find_last_of(L'\\') + 1);
		paramString += L"MessageSPY_amd64.dll\" SpyStart";

		
		seInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		seInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		seInfo.lpVerb = L"open";
		seInfo.lpDirectory = rundllPath;
		seInfo.lpFile = L"rundll32.exe";
		seInfo.lpParameters = paramString.data();
		seInfo.nShow = SW_HIDE;
		ShellExecuteEx(&seInfo);
	}
	
	mainDlg.Show();

	mainDlg.End();

	StopHook();	

	if (is64BitOS)
	{
		SetEvent(otherBitQuitEvent);
		CloseHandle(otherBitQuitEvent);
		WaitForSingleObject(seInfo.hProcess, INFINITE);
	}

	return 0;
}