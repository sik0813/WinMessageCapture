#include "commonInclude.h"
#include "CMainDlg.h"
#include "../MessageSYP/MessageSYP.h"


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	CMainDlg tmp;
	//StartHook();
	tmp.Show(hInstance);
	//StopHook();

	return 0;
}