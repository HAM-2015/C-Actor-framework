
// mfc_coro.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// Cmfc_coroApp:
// �йش����ʵ�֣������ mfc_coro.cpp
//

class Cmfc_coroApp : public CWinApp
{
public:
	Cmfc_coroApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern Cmfc_coroApp theApp;