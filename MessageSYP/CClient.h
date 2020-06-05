#pragma once
#include <Windows.h>

class CClient 
{
public:
	CClient() {}
	~CClient() {}


	BOOL WINAPI CheckMemory();

	BOOL CheckMsg()
	{
		return TRUE;
	}

	BOOL DecodeMsg()
	{

	}

	BOOL PrintMsg()
	{

	}
};