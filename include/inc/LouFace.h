
// LouFace.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CLouFaceApp: 
// �йش����ʵ�֣������ LouFace.cpp
//

class CLouFaceApp : public CWinApp
{
public:
	CLouFaceApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��
	ULONG_PTR m_gdiplusToken; //GDI������Ա����

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CLouFaceApp theApp;
