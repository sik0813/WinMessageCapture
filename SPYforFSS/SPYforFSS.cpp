#include "commonInclude.h"
#include "CMainDlg.h"
#include "../MessageSYP/MessageSYP.h"


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	CMainDlg mainDlg;
	StartHook();
	mainDlg.Start(hInstance);
	mainDlg.End();
	StopHook();

	return 0;
}