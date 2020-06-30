#pragma once

#include "commonInclude.h"
#include "COptionDlg.h"
#include <queue>

class CCollectDlg
{
public:
	CCollectDlg(int _objectIndex);
	~CCollectDlg();

private:
	const int m_objectIndex = 0;
	HWND m_parentHwnd = NULL;

	HWND m_ownHwnd = NULL;
	UINT m_countLine = 0;
	BOOL m_showMsgData = FALSE; // 시작(TRUE)/일시정지(FALSE)
	HWND m_startAndSuspend;

	std::queue<MsgData> m_inputMsg;
	HANDLE m_threadHandle = INVALID_HANDLE_VALUE;
	CRITICAL_SECTION m_readWriteCS;
	BOOL m_threadQuit = FALSE;

	HWND m_optionHwnd = NULL;

	COptionDlg *m_childOption = NULL;
	SettingData m_curSettingData;


public:
	BOOL Start(HWND _parentHwnd, std::vector<std::wstring> _inputProcessName);
	BOOL End();

	static INT_PTR CALLBACK CCollectDlg::RunProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// 다이얼로그 초기화
	BOOL InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Command(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

	// 입력 받은 데이터를 queue에 추가
	void InsertData(MsgData MsgData);
	// queue의 데이터를 기반으로 출력
	static UINT WINAPI DisplayDataThread(void *arg);
	void DisplayData();
	void MakeStyleString(std::wstring *_inputString, UINT _inputStyle);

	// 현재 ListBox 데이터 파일로 저장
	BOOL SaveLog(HWND hwnd);
};
