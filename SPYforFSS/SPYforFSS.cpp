#include "commonInclude.h"
#include "CMainDlg.h"
#include "../MessageSYP/MessageSYP.h"


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	CMainDlg mainDlg;
	BOOL succFunc = mainDlg.Start(hInstance);
	StartHook();
	mainDlg.Show();
	StopHook();
	mainDlg.End();

	return 0;
}