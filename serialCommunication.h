// serialCommunication.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CserialCommunicationApp:
// �йش����ʵ�֣������ serialCommunication.cpp
//

class CserialCommunicationApp : public CWinApp
{
public:
	CserialCommunicationApp();
	CString app_strSource;
// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CserialCommunicationApp theApp;