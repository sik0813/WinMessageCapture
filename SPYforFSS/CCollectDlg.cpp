#include "CCollectDlg.h"
#include <process.h>

CCollectDlg::CCollectDlg(int _objectIndex):
	objectIndex(_objectIndex)
{
}

CCollectDlg::~CCollectDlg()
{
}

BOOL CCollectDlg::Start(HWND _parentHwnd)
{
	parentHwnd = _parentHwnd;
	readDataEvent = CreateEventW(NULL, TRUE, TRUE, NULL);
	writeDataEvent = CreateEventW(NULL, TRUE, TRUE, NULL);
	SetEvent(readDataEvent);
	ResetEvent(writeDataEvent);

	threadHandle = (HANDLE)_beginthreadex(NULL, 0, DisplayDataThread, (LPVOID)this, NULL, NULL);
	if (INVALID_HANDLE_VALUE == threadHandle)
	{
		return FALSE;
	}
	
	ownHwnd = CreateDialogParamW(NULL, MAKEINTRESOURCEW(IDD_COLLECTPAGE), NULL, CCollectDlg::RunProc, (LPARAM)this);
	ShowWindow(ownHwnd, SW_SHOW);
	return TRUE;
}

BOOL CCollectDlg::End()
{
	DestroyWindow(ownHwnd);

	threadQuit = TRUE;
	SetEvent(writeDataEvent);
	SetEvent(readDataEvent);

	CloseHandle(readDataEvent);
	readDataEvent = INVALID_HANDLE_VALUE;
	CloseHandle(writeDataEvent);
	writeDataEvent = INVALID_HANDLE_VALUE;

	WaitForSingleObject(threadHandle, INFINITE);
	CloseHandle(threadHandle);
	threadHandle = INVALID_HANDLE_VALUE;

	if (NULL != childOption)
	{
		childOption->End();
		delete childOption;
		childOption = NULL;
	}

	PostMessage(parentHwnd, WM_CHILDEND, (WPARAM)objectIndex, NULL);
	return 0;
}

INT_PTR CALLBACK CCollectDlg::RunProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CCollectDlg* pointerThis = (CCollectDlg*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lParam);
		pointerThis = (CCollectDlg*)lParam;
		//pointerThis->ownHwnd = hwnd;
		pointerThis->InitDialog(hwnd, (HWND)(wParam), lParam);
		break;

	case WM_COMMAND:
		pointerThis->Command(hwnd, (int)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam));
		break;

	case WM_SETOPTION:
		pointerThis->childOption->GetOption(&(pointerThis->curSettingData));
		pointerThis->childOption->End();
		delete pointerThis->childOption;
		pointerThis->childOption = NULL;
		break;
	}

	return FALSE;
}

// ���̾�α� �ʱ�ȭ
BOOL CCollectDlg::InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	//Option ����
	return TRUE;
}

void CCollectDlg::Command(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	BOOL SuccessFuc = FALSE;
	switch (id)
	{
	case IDCANCEL:
	case IDOK:		
		End();
		//EndDialog(hwnd, id);		
		break;

	case IDC_STARTSUSPEND:
		showMsgData = !showMsgData;
		break;

	case IDC_FILEOUT:
		SaveLog(hwnd);
		break;

	case IDC_OPTION:
		if (NULL != childOption)
		{
			break;
		}
		childOption = new COptionDlg();
		childOption->Start(ownHwnd, &curSettingData);
		break;
	}
}

void CCollectDlg::InsertData(MsgData *_inputMsgData)
{
	WaitForSingleObject(readDataEvent, INFINITE);
	ResetEvent(writeDataEvent);
	inputMsg.push(*_inputMsgData);
	SetEvent(writeDataEvent);
}

UINT CCollectDlg::DisplayDataThread(void * arg)
{
	((CCollectDlg*)arg)->DisplayData();
	return 0;
}

void CCollectDlg::DisplayData()
{
	while (true)
	{
		WaitForSingleObject(writeDataEvent, INFINITE);
		if (TRUE == threadQuit)
		{
			break;
		}

		if (inputMsg.size() == 0)
		{
			continue;
		}

		ResetEvent(readDataEvent);		
		MsgData inputMsgData = inputMsg.front();
		inputMsg.pop();
		SetEvent(readDataEvent);

		if (FALSE == showMsgData)
		{
			continue;
		}

		HWND listHwnd = GetDlgItem(ownHwnd, IDC_COLLECTLIST);
		std::wstring viewMsg = L"";
		std::wstring lineNum = std::to_wstring(countLine++);

		// Line number(8�ڸ���) �߰�
		viewMsg += L"<";
		for (int i = 0; i < 8 - lineNum.size(); i++)
			viewMsg += L"0";
		viewMsg += lineNum + L"> ";

		// Process Name �߰�
		viewMsg += L" [" + std::wstring(inputMsgData.processName) + L"]";

		// Process ID �߰�
		viewMsg += L" [" + std::to_wstring(inputMsgData.processID) + L"]";

		// Msg type �߰�
		viewMsg += L" ";
		viewMsg += inputMsgData.msgType;

		// Message Content �߰�
		if (NULL == wmTranslation[inputMsgData.msgCode])
		{
			viewMsg += L" [unknown]";
		}
		else
		{
			viewMsg += L" [" + std::wstring(wmTranslation[inputMsgData.msgCode]) + L"]";
		}
		

		// Message Code �߰�
		viewMsg += L"(" + std::to_wstring(inputMsgData.msgCode) + L")";

		// wParam �߰�
		viewMsg += L"   wParam : " + std::to_wstring(inputMsgData.wParam);

		// lParam �߰�
		viewMsg += L"   lParam : " + std::to_wstring(inputMsgData.lParam);


		SendMessageW(listHwnd, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)viewMsg.data());
	}
}

BOOL CCollectDlg::SaveLog(HWND hwnd)
{
	HWND ListBoxHwnd = GetDlgItem(hwnd, LB_INSERTSTRING);
	UINT count = SendMessageW(ListBoxHwnd, LB_GETCOUNT, 0, 0);
	
	// ���� ��� ����

	for (UINT i = 0; i < count; i++)
	{
		UINT testLen = SendMessageW(ListBoxHwnd, LB_GETTEXTLEN, (WPARAM)(0), (LPARAM)NULL);
		LPWSTR curText = new WCHAR[testLen];
		SendMessageW(ListBoxHwnd, LB_GETTEXT, (WPARAM)(count), (LPARAM)curText);
		// ���� ����
		delete[] curText;
	}

	return TRUE;
}
