// ���� ifdef ����� DLL���� ���������ϴ� �۾��� ���� �� �ִ� ��ũ�θ� ����� 
// ǥ�� ����Դϴ�. �� DLL�� ��� �ִ� ������ ��� ����ٿ� ���ǵ� _EXPORTS ��ȣ��
// �����ϵǸ�, �ٸ� ������Ʈ������ �� ��ȣ�� ������ �� �����ϴ�.
// �̷��� �ϸ� �ҽ� ���Ͽ� �� ������ ��� �ִ� �ٸ� ��� ������Ʈ������ 
// MESSAGESYP_API �Լ��� DLL���� �������� ������ ����, �� DLL��
// �� DLL�� �ش� ��ũ�η� ���ǵ� ��ȣ�� ���������� ������ ���ϴ�.
#ifdef MESSAGESYP_EXPORTS
#define MESSAGESYP_API __declspec(dllexport)
#else
#define MESSAGESYP_API __declspec(dllimport)
#endif

#ifdef __cplusplus
#define EXPORT extern "C" MESSAGESYP_API
#else
#define EXPORT MESSAGESYP_API
#endif

#pragma once

#include <Windows.h>
#include <map>


EXPORT void StartHook(DWORD threadID = 0);
EXPORT void StopHook(void);
LRESULT CALLBACK CallWndProc(int code, WPARAM wParam, LPARAM lParam);
LRESULT CallWndRetProc(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK GetMsgProc(int code, WPARAM wParam, LPARAM lParam);

std::map<std::wstring, BOOL> deniedProcessList = {
	std::pair<std::wstring, BOOL>(L"SPYforFSS.exe", TRUE)
};
