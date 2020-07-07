#pragma once

#include "commonInclude.h"
#include "COptionDlg.h"
#include <queue>
#include <CommCtrl.h>
#pragma comment( lib, "comctl32" )  

enum class colIndex {
	index, pName, pID, tID, msgContent, msgCode, msgType,
	wParam, lParam, caption, className, style, detail
};

typedef struct _SaveData
{
	std::wstring index;
	std::wstring pName;
	std::wstring pID;
	std::wstring tID;
	std::wstring msgCode;
	std::wstring msgContent;
	std::wstring msgType;
	std::wstring wParam;
	std::wstring lParam;
	std::wstring caption;
	std::wstring className;
	std::wstring style;
	std::wstring detail;
	_SaveData()
	{
		index = L"";
		pName = L"";
		pID = L"";
		tID = L"";
		msgCode = L"";
		msgContent = L"";
		msgType = L"";
		wParam = L"";
		lParam = L"";
		caption = L"";
		className = L"";
		style = L"";
		detail = L"";
	}
}SaveData, *LPSaveData;


class CCollectDlg
{
public:
	CCollectDlg(int _objectIndex);
	~CCollectDlg();

private:
	const int m_objectIndex = 0;
	HWND m_parentHwnd = NULL;

	HWND m_ownHwnd = NULL;
	BOOL m_showMsgData = FALSE; // 시작(TRUE)/일시정지(FALSE)

	HANDLE m_queueNotEmpty = NULL;
	std::queue<MsgData> m_inputMsg;
	HANDLE m_threadHandle = INVALID_HANDLE_VALUE;
	CRITICAL_SECTION m_readWriteCS;
	BOOL m_threadQuit = FALSE;

	HWND m_collectListHwnd = NULL;
	LVCOLUMNW m_lvCol;
	LVITEMW m_lvItem;
	ULONGLONG m_listRowIndex = 0;
	std::vector<SaveData> m_saveDataList;

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
	void AddStyleString(std::wstring *_inputString, UINT _inputStyle, int _type = 1);

	// 현재 ListBox 데이터 파일로 저장
	BOOL SaveLog(HWND hwnd);

	std::string wcs2mbs(std::wstring const& str, std::locale const& loc = std::locale());
};
