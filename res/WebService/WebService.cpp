// WebService.cpp : ʵ���ļ�
//

#include "stdafx.h"
//#include "d.h"
#include "WebService.h"
#import "msxml4.dll" named_guids
//using namespace MSXML2;
#import "C:\Program Files (x86)\Common Files\MSSOAP\Binaries\MSSOAP30.dll" \
        exclude("IStream", "IErrorInfo", "ISequentialStream", "_LARGE_INTEGER", \
        "_ULARGE_INTEGER", "tagSTATSTG", "_FILETIME")
using namespace MSSOAPLib30;
//char* mesServer = "http://172.16.9.53/ATSWebservice/MyService.asmx";

//char* mesServer = "http://172.16.9.15/ATSWebservice_Test/MyService.asmx";//�з���

//char* mesServer = "http://172.16.82.127/ATSWebservice/MyService.asmx";//bisonpc

char* mesServer = "http://172.16.9.118/ATSWebservice/MyService.asmx";//��ʽ��

//char* mesServer = "http://172.16.82.142/ATSWebservice/MyService.asmx";
char* mesNamespace = "http://tempuri.org/";
// WebService

IMPLEMENT_DYNAMIC(WebService, CWnd)

WebService::WebService()
{

}

WebService::~WebService()
{
}


BEGIN_MESSAGE_MAP(WebService, CWnd)
END_MESSAGE_MAP()



// WebService ��Ϣ�������
void WebService::WriteErrLog(char* errMessage)
{
	CTime tm = CTime::GetCurrentTime();
	CString strtm = tm.Format("%Y-%m-%d");
	strtm += _T(".txt");

	FILE *fp;
	fp = fopen((LPSTR)(LPCTSTR)strtm, "a+");
	if (fp != NULL)
	{
		fprintf(fp, "%s\n", errMessage);
		fclose(fp);
	}
}

char* WebService::QueryWebService(char* EndPointURL, char* Namespace, char* method, vector<pair<string, string> >& mParam, bool rettext)
{
	//char* Namespace = "http://tempuri.org/";
	char* strReturnValues;
	//char strReturnValues[1024]
	//string strReturnValues;
	ISoapSerializerPtr Serializer;
	ISoapReaderPtr Reader;
	ISoapConnectorPtr Connector;
	
	HRESULT hr = CoInitialize(NULL);//��ʼ��com����
	
	if (FAILED(hr))
	{
		WriteErrLog("initialized com failed");//��ʼ��COMʧ��
		//AfxMessageBox(_T("[ERROR]��ʼ��COMʧ��!"));
		return "error";
	}
	// Connect to the service
	hr = Connector.CreateInstance(__uuidof(HttpConnector30));
	if (FAILED(hr))
	{
		//����com�������һ������Ϊû�а�װcom
		WriteErrLog("create HttpConnector failed.");//����HttpConnector FAILED
		//AfxMessageBox(_T("[ERROR]����HttpConnectorʧ��!"));
		return "error";
	}
	Connector->Property["EndPointURL"] = EndPointURL;        // �ӿ�λ��
	hr = Connector->Connect();                               // �ͷ���������
	if (FAILED(hr))
	{
		WriteErrLog("connecting webservice failed.");//����WEBSERVICE FAILED
		//AfxMessageBox(_T("[ERROR]����WEBSERVICEʧ��!"));
		return "error";
	}

	// Begin message
	Connector->Property["SoapAction"] = _bstr_t(Namespace) + _bstr_t(method);
	Connector->BeginMessage();


	// Create the SoapSerializer object.
	hr = Serializer.CreateInstance(__uuidof(SoapSerializer30));
	if (FAILED(hr))
	{
		WriteErrLog("CreateInstance Serializer FAILED");//CreateInstance Serializer FAILED
		//AfxMessageBox(_T("[ERROR]����Serializerʵ��ʧ��!"));
		return "error";
	}

	// ��serializer���ӵ�connector�������ַ���
	// Connect the serializer object to the input stream of the connector object.
	Serializer->Init(_variant_t((IUnknown*)Connector->InputStream));

	// Build the SOAP Message.
	// ����SOAP��Ϣ
	Serializer->StartEnvelope("soap", "", "");
	Serializer->StartBody("body");
	Serializer->StartElement(method, Namespace, "", ""); // �����ռ������
	for (vector<pair<string, string> >::iterator it = mParam.begin(); it != mParam.end(); it++)
	{
		string sParamNameTmp = it->first;
		string sParamValueTmp = it->second;
		Serializer->StartElement(sParamNameTmp.c_str(), Namespace, "", "");
		Serializer->WriteString(sParamValueTmp.c_str());
		Serializer->EndElement();
	}
	Serializer->EndElement();
	Serializer->EndBody();
	Serializer->EndEnvelope();
	// Send the message to the web service
	try
	{
		Connector->EndMessage();
	}
	catch (...)
	{
		WriteErrLog("�ύSOAP��Ϣ����");//�ύSOAP��Ϣ����
		//AfxMessageBox(_T("[ERROR]���ݿ�����ʧ�ܣ��ύSOAP��Ϣ����!"));
		return "error";
	}

	// Read the response.
	hr = Reader.CreateInstance(__uuidof(SoapReader30));    //��ȡ��Ӧ
	if (FAILED(hr))
	{
		WriteErrLog("CreateInstance Serializer FAILED"); //CreateInstance READER FAILED
		//AfxMessageBox(_T("[ERROR]����Serializerʵ��ʧ��,��ȡ����Ӧ!"));
		return "error";
	}

	// Connect the reader to the output stream of the connector object.
	try
	{
		Reader->Load(_variant_t((IUnknown*)Connector->OutputStream), "");  //��reader���ӵ�connector������ַ���  
	}
	catch (...)
	{
		WriteErrLog("Reader Load ReturnResult FAILED"); //CreateInstance READER FAILED
		//AfxMessageBox(_T("[ERROR]��������Ӧ��Ϣʧ��(ReaderLoad Failed)�����������״̬!"));
		return "error";
	}
	if (rettext)
	{	
		//AfxMessageBox(_T("-------1----------------"));
		strReturnValues = new char[strlen(Reader->GetRpcResult()->text) + 1];
		//CString test;
		//test = Reader->RpcResult->text;
		strcpy(strReturnValues, (const char*)Reader->RpcResult->text); //Display the text result.
	}
	else
	{
		strReturnValues = new char[strlen(Reader->RpcResult->xml) + 1];
		strcpy(strReturnValues, (const char*)Reader->RpcResult->xml); //Display the xml result.	 
	}
	WriteErrLog("mtcxml:");
	WriteErrLog(Reader->RpcResult->xml);
	WriteErrLog("mtctext:");
	WriteErrLog(Reader->RpcResult->text);
	WriteErrLog("mtcend:");
	CoUninitialize();
	return strReturnValues;
}
char* WebService::GetTpName(char* ipAddr)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//map<string,string> MapTestParam; 
	vector<pair<string, string> > VectortempParam;
	//MapTestParam.insert(map<string, string>::value_type("IPAddress",ipAddr));  
	VectortempParam.push_back(make_pair<string, string>("IPAddress", ipAddr));
	char* strReruenValues;
	strReruenValues = QueryWebService("http://www.webservicex.net/geoipservice.asmx", "http://www.webservicex.net/", "GetGeoIP", VectortempParam, false);
	WriteErrLog("web service Reruen Values:");
	WriteErrLog(strReruenValues);
	return strReruenValues;
}

char* WebService::ADCCheckLotSN(char* PostCode, char* LotSN, char* fieldName, char* OrderID, char* TestDevice, char* WorkcenterName, char* ResourceName, char* UserName)
{
	
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	vector<pair<string, string> > VectortempParam;
	VectortempParam.push_back(make_pair<string, string>("postCode", PostCode));
	VectortempParam.push_back(make_pair<string, string>("lotSN", LotSN));
	VectortempParam.push_back(make_pair<string, string>("fieldName", fieldName));
	VectortempParam.push_back(make_pair<string, string>("orderID", OrderID));
	VectortempParam.push_back(make_pair<string, string>("testDevice", TestDevice));
	VectortempParam.push_back(make_pair<string, string>("workcenterName", WorkcenterName));
	VectortempParam.push_back(make_pair<string, string>("resourceName", ResourceName));
	VectortempParam.push_back(make_pair<string, string>("userName", UserName));
	VectortempParam.push_back(make_pair<string, string>("userParameterA", ""));
	VectortempParam.push_back(make_pair<string, string>("userParameterB", ""));
	VectortempParam.push_back(make_pair<string, string>("userParameterC", ""));
	VectortempParam.push_back(make_pair<string, string>("userParameterD", ""));
	VectortempParam.push_back(make_pair<string, string>("userParameterE", ""));
	char* strReruenValues;
	
	strReruenValues = QueryWebService(mesServer, mesNamespace, "ADCCheckLotSN", VectortempParam, true);
	
	WriteErrLog("[ADCCheckLotSN]web service Reruen Values:");
	WriteErrLog(strReruenValues);
	return strReruenValues;
}
char* WebService::ADCGetInterfaceParameter(char* PostCode, char* LotSN, char* fieldName, char* OrderID, char* TestDevice, char* WorkcenterName, char* ResourceName, char* UserName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	vector<pair<string, string> > VectortempParam;
	VectortempParam.push_back(make_pair<string, string>("postCode", PostCode));
	VectortempParam.push_back(make_pair<string, string>("lotSN", LotSN));
	VectortempParam.push_back(make_pair<string, string>("fieldName", fieldName));
	VectortempParam.push_back(make_pair<string, string>("orderID", OrderID));
	VectortempParam.push_back(make_pair<string, string>("testDevice", TestDevice));
	VectortempParam.push_back(make_pair<string, string>("workcenterName", WorkcenterName));
	VectortempParam.push_back(make_pair<string, string>("resourceName", ResourceName));
	VectortempParam.push_back(make_pair<string, string>("userName", UserName));
	VectortempParam.push_back(make_pair<string, string>("userParameterA", ""));
	VectortempParam.push_back(make_pair<string, string>("userParameterB", ""));
	VectortempParam.push_back(make_pair<string, string>("userParameterC", ""));
	VectortempParam.push_back(make_pair<string, string>("userParameterD", ""));
	VectortempParam.push_back(make_pair<string, string>("userParameterE", ""));
	char* strReruenValues;
	strReruenValues = QueryWebService(mesServer, mesNamespace, "ADCGetInterfaceParameter", VectortempParam, true);
	WriteErrLog("[ADCCheckLotSN]web service Reruen Values:");
	WriteErrLog(strReruenValues);
	return strReruenValues;
}
char* WebService::ADCSubmitTestData(char* PostCode, char* LotSN, char* fieldName, char* OrderID, char* TestDevice, char* WorkcenterName, char* ResourceName, char* UserName, char* TestData)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	vector<pair<string, string> > VectortempParam;
	VectortempParam.push_back(make_pair<string, string>("postCode", PostCode));
	VectortempParam.push_back(make_pair<string, string>("lotSN", LotSN));
	VectortempParam.push_back(make_pair<string, string>("fieldName", fieldName));
	VectortempParam.push_back(make_pair<string, string>("orderID", OrderID));
	VectortempParam.push_back(make_pair<string, string>("testDevice", TestDevice));
	VectortempParam.push_back(make_pair<string, string>("workcenterName", WorkcenterName));
	VectortempParam.push_back(make_pair<string, string>("resourceName", ResourceName));
	VectortempParam.push_back(make_pair<string, string>("userName", UserName));
	VectortempParam.push_back(make_pair<string, string>("testData", TestData));
	char* strReruenValues;
	strReruenValues = QueryWebService(mesServer, mesNamespace, "ADCSubmitTestData", VectortempParam, true);
	WriteErrLog("[ADCCheckLotSN]web service Reruen Values:");
	WriteErrLog(strReruenValues);
	return strReruenValues;
}

