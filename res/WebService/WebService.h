#include <stdio.h>
#include <iostream>
#include <string> 
#include <map> 
#include <vector>  
using namespace std;

#pragma once


// WebService

class WebService : public CWnd
{
	DECLARE_DYNAMIC(WebService)

public:
	WebService();
	virtual ~WebService();
	char* QueryWebService(char*, char*, char*, vector<pair<string, string> >&, bool);
	void WriteErrLog(char*);
	char* GetTpName(char* ipAddr);
	char* ADCCheckLotSN(char* PostCode, char* LotSN, char* fieldName, char* OrderID, char* TestDevice, char* WorkcenterName, char* ResourceName, char* UserName);
	char* ADCGetInterfaceParameter(char* PostCode, char* LotSN, char* fieldName, char* OrderID, char* TestDevice, char* WorkcenterName, char* ResourceName, char* UserName);
	char* ADCSubmitTestData(char* PostCode, char* LotSN, char* fieldName, char* OrderID, char* TestDevice, char* WorkcenterName, char* ResourceName, char* UserName, char* TestData);
protected:
	DECLARE_MESSAGE_MAP()
};


