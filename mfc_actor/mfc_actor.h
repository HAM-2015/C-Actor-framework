
// mfc_actor.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// Cmfc_actorApp:
// �йش����ʵ�֣������ mfc_actor.cpp
//

class Cmfc_actorApp : public CWinApp
{
public:
	Cmfc_actorApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern Cmfc_actorApp theApp;