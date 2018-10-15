// serialCommunicationDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "serialCommunication.h"
#include "serialCommunicationDlg.h"
#include "WebService.h"
#include "Markup.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIME_SHOW_MESSAGE_SERIAL	5000	//串口数据显示定时器ID
#define TIME_SHOW_MESSAGE_TEST_DELAY	3000	//自动测试通道延时
#define TIME_SHOW_MESSAGE_TEST_DELAY_OTHER	7000	//自动测试通道延时其他用
#define TIME_SHOW_MESSAGE_KEYPAY_TEST_OTHER  8000  //按键测试及其他附加测试
#define TIME_SHOW_MESSAGE_TEST_MAC  9000   //MAC上传MES 专用
#define TIME_SHOW_MESSAGE_KEYPAD_TIME   5678
#define CONFIGMEG			_T("ConfigMessage")
WebService  m_WebService;
//下面是MES 相关变量
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
//////////////////////////////////////////////////////////////////////
CString Done;//用来判断执行的动作，主要是判断读MAC					//
char FieldName[10]={0};												//
char OrderID[16]={0};//LotSN 对应的订单号							//
char LotSN[32]={0};//为电视上的SN值	
char MacEth[32];//电视MAC地址	//
//char* LotSN="FUN484000084416230102996";//为电视上的SN值			//
//
char* TestDevice="写号" ;//测试站点  该工具就用在写号站点			//
char UserName[32]  ;//用户名										//
char* ResourceName;//资源名称（电脑名称） --电脑名称				//
char WorkcenterName[8];//线体名										//
//
std::map<CString, vector<CString> > mesSettingMap; //mes表格设置	//
std::map<CString, CString> mesDataMap;//mes订单数据					//
std::map<CString, CString> mesGetDataMap;//mes订单配置数据			//
std::map<CString, CString> mesGetWordMap;//mes订单预置数据			//
std::map<CString, CString> mesGetTimeMap;//获取服务器时间			//
//
std::map<CString, CString> mesCheckorSubmitMap;//mes检查数据		//
//
//
multimap<string, vector<pair<string, string> > >multiMapTestData;	//
vector<pair<string, string> > vectorTestData;						//
char* charMesTestResult = "TestResult";								//
char* charMesTestItem = "TestItem";									//
char* charMesTestTime = "TestTime";									//
char* charMesTestValue = "TestValue";								//
char* charMesData = "Data";											//
char* charMesWriteData = "WriteData";								//
char* charMesWrite = "Write";										//
CString cstrMesMacEth = _T("MAC");									//
CString cstrTestResult = _T("Option1");								//
CString cstrTestResultOK = _T("Pass");								//
CString cstrTestResultNG = _T("NG");								//
//////////////////////////////////////////////////////////////////////
BOOL boolScanok = false;
BOOL boolScanDataOK = false;
BOOL boolCheckFail = false;
BOOL boolScanSNDataOK = false;
BOOL boolScanMACDataOK = false;
BOOL boolCheckRev = false;
CString strSWVersion("");
CString strBuildTime("");
CString strCheckMeg("");
CString MacCheck("");
CString strWriteMeg("");
CString strReadMeg("");
CString strMainKey("");
CString strMACMeg("");
CString strCheckFail("");
int  intcount=0;
////////////////////////////////////////////////////////////////////////
//add for MES
CString CserialCommunicationDlg::ADCGetResourceName()
{
	char charBuffer[MAX_PATH] = "DefaultResourceName";

	CString cstrResourceName;
	DWORD dwNameLen;
	dwNameLen = MAX_PATH;
	TCHAR ComputerName[MAX_PATH];
	if (GetComputerName(ComputerName, &dwNameLen))
	{
		cstrResourceName = ComputerName;
	}
	else
	{
		cstrResourceName = charBuffer;
	}

	return cstrResourceName;
	//CString strTest;
	//strTest.Format(_T("A4008956N"));
	//return strTest;
}
BOOL CserialCommunicationDlg::ADCCheckLotSN(char* PostCode, char* LotSN, char* FieldName)//LotSN就是机身SN，FieldName为SN在MES中对应的字段名，PostCode只能为“TEST”
{

	char * codefile = NULL;
	char computername[100];
	memset(computername,0,100);
	sprintf(computername,"%s",ADCGetResourceName());
	codefile = m_WebService.ADCCheckLotSN(PostCode, LotSN, FieldName,OrderID, TestDevice,WorkcenterName,computername ,UserName);
	//MessageBox(codefile);
	if (strcmp(codefile, "error") != 0)
	{
		//AfxMessageBox(codefile);
		parserADCCheckorSubmitXML(codefile);
		delete[] codefile;//webservice new,使用完毕后要清理内存
		return true;
	}
	return false;
}
BOOL CserialCommunicationDlg::ADCGetInterfaceParameter(char* PostCode, char* LotSN, char* FieldName)
{
	//CDApp  *papp = (CDApp*)AfxGetApp();                   // ***** 生成指向运用程序类的指针*****
	char * codefile;
	char computername[100];
	memset(computername,0,100);
	sprintf(computername,"%s",ADCGetResourceName());
	codefile = m_WebService.ADCGetInterfaceParameter(PostCode, LotSN, FieldName, OrderID, TestDevice, WorkcenterName,computername, UserName);
	if (strcmp(codefile, "error") != 0)
	{
		//AfxMessageBox(codefile);
		parserADCGetInterfaceParameterXML(PostCode, codefile);
		delete[] codefile;//webservice new,使用完毕后要清理内存
		return true;
	}
	return false;
}
BOOL CserialCommunicationDlg::ADCSubmitTestData(char* PostCode, char* LotSN, char* FieldName,int MacInput)
{
	//CDApp  *papp = (CDApp*)AfxGetApp();                   // ***** 生成指向运用程序类的指针*****
	char * codefile;
	GenTestDate(MacInput);//生成需要测试数据和需要插入数据库中的数据
	CMarkup xml;
	xml.Load(_T("./mestestdata.xml"));
	MCD_STR strXML = xml.GetDoc();
	char strTestData[1024];
	memset(strTestData,0,1024);
	sprintf(strTestData,"%s",strXML);
	char computername[100];
	memset(computername,0,100);
	sprintf(computername,"%s",ADCGetResourceName());
	codefile = m_WebService.ADCSubmitTestData(PostCode, LotSN, FieldName, OrderID, TestDevice, WorkcenterName, computername, UserName,strTestData);
	//delete[] strTestData;//使用完毕后要清理内存

	if (strcmp(codefile, "error") != 0)
	{
		parserADCCheckorSubmitXML(codefile);
		delete[] codefile;//webservice new,使用完毕后要清理内存
		return true;
	}
	return false;
}
void CserialCommunicationDlg::parserADCCheckorSubmitXML(char* parserxml)
{
	CMarkup xml;
	mesCheckorSubmitMap.clear();////先清理再使用map，以免重复使用时上一次的数据没清掉
	if (!xml.SetDoc((CString)parserxml))//setdoc返回0说明xml格式无效
	{
		CString cstrMesError;
		cstrMesError.Format(_T("[xml格式无效]调用parserADCCheckorSubmitXML时\n[MES服务器]%s"), parserxml);
		AfxMessageBox(cstrMesError);
		return;
	}
	xml.Save(_T("./mesADCCheckLotSNorADCSubmitTestData.xml"));
	xml.FindElem(); // root DBSET element
	xml.IntoElem(); // inside DBSET
	while (xml.FindElem(_T("RESULT")))
	{
		xml.IntoElem();//inside RESULT
		CString strName("") ;
		CString strData ("");
		while (xml.FindElem())
		{
			strName = xml.GetTagName();
			strData = xml.GetData();
			mesCheckorSubmitMap.insert(map<CString, CString>::value_type(strName, strData));
		}
	}
	while (xml.FindElem(_T("RECORD")))
	{
		xml.IntoElem();//inside RECORD
		CString strName("") ;
		CString strData ("");
		while (xml.FindElem())
		{
			strName = xml.GetTagName();
			strData = xml.GetData();
			mesCheckorSubmitMap.insert(map<CString, CString>::value_type(strName, strData));
		}
	}
	xml.OutOfElem();
}
void CserialCommunicationDlg::parserADCGetInterfaceParameterXML(char* PostCode, char* parserxml)
{
	CMarkup xml;
	if (!xml.SetDoc((CString)parserxml))//setdoc返回0说明xml格式无效
	{
		CString cstrMesError;
		cstrMesError.Format(_T("[xml格式无效]调用parserADCGetInterfaceParameterXML时\n[MES服务器]%s"), parserxml);
		AfxMessageBox(cstrMesError);
		return;
	}
	xml.Save(_T("./mesADCGetInterfaceParameter.xml"));
	mesDataMap.clear();//先清理再使用map，以免重复使用时上一次的数据没清掉
	mesSettingMap.clear();//先清理再使用map，以免重复使用时上一次的数据没清掉
	mesGetDataMap.clear();//先清理再使用map，以免重复使用时上一次的数据没清掉
	mesGetWordMap.clear();//先清理再使用map，以免重复使用时上一次的数据没清掉
	xml.FindElem(); // root DBSET element
	xml.IntoElem(); // inside DBSET
	while (xml.FindElem(_T("RESULT")))
	{
		xml.IntoElem();
		CString strName("");
		CString strData ("");
		if (strcmp("FieldRule", PostCode) == 0)
		{
			std::vector<CString> resultvec;
			while (xml.FindElem())
			{
				strName = xml.GetTagName();
				strData = xml.GetData();
				resultvec.clear();
				resultvec.push_back(strData);
				mesSettingMap.insert(map<CString, vector<CString> >::value_type(strName, resultvec));
			}
		}
		else if (strcmp("FieldData", PostCode) == 0)
		{
			while (xml.FindElem())
			{
				strName = xml.GetTagName();
				strData = xml.GetData();
				mesDataMap.insert(map<CString, CString>::value_type(strName, strData));
			}
		}
		else if (strcmp("GETDATE", PostCode) == 0)
		{
			while (xml.FindElem())
			{
				strName = xml.GetTagName();
				strData = xml.GetData();
				mesGetTimeMap.insert(map<CString, CString>::value_type(strName, strData));
			}
		}
		else if (strcmp("FieldSet", PostCode) == 0)
		{
			while (xml.FindElem())
			{
				strName = xml.GetTagName();
				strData = xml.GetData();
				mesGetDataMap.insert(map<CString, CString>::value_type(strName, strData));
			}
		}
		else if (strcmp("FieldWord", PostCode) == 0)
		{
			while (xml.FindElem())
			{
				strName = xml.GetTagName();
				strData = xml.GetData();
				mesGetWordMap.insert(map<CString, CString>::value_type(strName, strData));
			}
		}

		xml.OutOfElem();
	}
	while (xml.FindElem(_T("RECORD")))
	{
		xml.IntoElem();
		CString strName("");
		CString strData ("");

		if (strcmp("FieldRule", PostCode) == 0)
		{
			std::vector<CString> recordvec;
			while (xml.FindElem())
			{
				if (xml.GetTagName().Compare(_T("FieldName")) == 0)
				{
					strName = xml.GetData();
				}
				else
				{
					strData = xml.GetData();
					recordvec.push_back(strData);
				}
			}
			mesSettingMap.insert(map<CString, vector<CString> >::value_type(strName, recordvec));
			recordvec.clear();
		}
		else if (strcmp("FieldData", PostCode) == 0)
		{
			while (xml.FindElem())
			{
				strName = xml.GetTagName();
				strData = xml.GetData();
				mesDataMap.insert(map<CString, CString>::value_type(strName, strData));
			}
		}
		else if (strcmp("GETDATE", PostCode) == 0)
		{
			while (xml.FindElem())
			{
				strName = xml.GetTagName();
				strData = xml.GetData();
				mesGetTimeMap.insert(map<CString, CString>::value_type(strName, strData));
			}
		}
		else if (strcmp("FieldSet", PostCode) == 0)
		{
			while (xml.FindElem())
			{
				strName = xml.GetTagName();
				strData = xml.GetData();
				mesGetDataMap.insert(map<CString, CString>::value_type(strName, strData));
			}
		}
		else if (strcmp("FieldWord", PostCode) == 0)
		{
			while (xml.FindElem())
			{
				strName = xml.GetTagName();
				strData = xml.GetData();
				mesGetWordMap.insert(map<CString, CString>::value_type(strName, strData));
			}
		}

		xml.OutOfElem();
	}
}


BOOL CserialCommunicationDlg::GenTestDate(int MacInput)
{
	CMarkup xml;
	xml.AddElem(_T("TestData"));
	xml.IntoElem();
	multiMapTestData.clear();//先清理再使用map，以免重复使用时上一次的数据没清掉
	//CString timetmp = getServerTime();
	CString timetmp;
	ADCGetInterfaceParameter("GETDATE",LotSN,FieldName);
	timetmp=mesGetTimeMap[_T("DATE")];

	char test[100];
	memset(test,0,100);
	sprintf(test,"%s",timetmp);

	vectorTestData.clear();
	vectorTestData.push_back(make_pair<string, string>(charMesTestResult, "PASS"));
	vectorTestData.push_back(make_pair<string, string>(charMesTestItem, "BurnSN"));
	vectorTestData.push_back(make_pair<string, string>(charMesTestTime,test));
	vectorTestData.push_back(make_pair<string, string>(charMesTestValue, "default value"));
	multiMapTestData.insert(pair<string, vector<pair<string, string> > >(charMesData, vectorTestData));
	vectorTestData.clear();
	if(MacInput==1)
	{
		memset(test,0,100);
		sprintf(test,"%s=%s,%s=%s",cstrMesMacEth,GetMacData, cstrTestResult, cstrTestResultOK);//cstrMesMacEth为要写进MES的MAC字段名，MacEth为MAC地址
		vectorTestData.push_back(make_pair<string, string>(charMesWriteData,test));
		multiMapTestData.insert(pair<string, vector<pair<string, string> > >(charMesWrite, vectorTestData));
	}else if (MacInput==2)
	{
		memset(test, 0, 100);
		sprintf(test, "%s=%s", cstrTestResult, MacEth);//cstrTestResult为要写进MES的测试结果字段名，MacEth为测试结果
		vectorTestData.push_back(make_pair<string, string>(charMesWriteData, test));
		multiMapTestData.insert(pair<string, vector<pair<string, string> > >(charMesWrite, vectorTestData));
	}
	multimap<string, vector<pair<string, string> > >::iterator multitr;
	vector<pair<string, string> >::iterator intertr;
	for (multitr = multiMapTestData.begin(); multitr != multiMapTestData.end(); multitr++)
	{
		xml.AddElem((CString)(multitr->first).c_str());
		xml.IntoElem();
		for (intertr = multitr->second.begin(); intertr != multitr->second.end(); intertr++)
		{
			xml.AddElem((CString)(intertr->first).c_str(), (CString)(intertr->second).c_str());
		}
		xml.OutOfElem();
	}
	xml.Save(_T("./mestestdata.xml"));
	return TRUE;
}
////////////////////////////////////////////////////////////////////////
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
	int m_iSetTime_Dtv1;
	int m_iSetTime_Dtv2;
	BOOL m_bSetup_Dtv1;
	BOOL m_bSetup_Dtv2;
	BOOL m_bSetup_Dtv3;
	BOOL m_bSetup_Atv1;
	BOOL m_bSetup_Atv2;
	BOOL m_bSetup_Dvbs;
	BOOL m_bSetup_Av1;
	BOOL m_bSetup_Av2;
	BOOL m_bSetup_Ypbpr;
	BOOL m_bSetup_Hdmi1;
	BOOL m_bSetup_Hdmi2;
	BOOL m_bSetup_Hdmi3;
	BOOL m_bSetup_Hdmi4;
	BOOL m_bSetup_Scart;
	BOOL m_bSetup_Pc;
	BOOL m_bSetup_Usb;
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
, m_iSetTime_Dtv1(1)
, m_iSetTime_Dtv2(1)
, m_bSetup_Dtv1(FALSE)
, m_bSetup_Dtv2(FALSE)
, m_bSetup_Dtv3(FALSE)
, m_bSetup_Atv1(FALSE)
, m_bSetup_Atv2(FALSE)
, m_bSetup_Dvbs(FALSE)
, m_bSetup_Av1(FALSE)
, m_bSetup_Av2(FALSE)
, m_bSetup_Ypbpr(FALSE)
, m_bSetup_Hdmi1(FALSE)
, m_bSetup_Hdmi2(FALSE)
, m_bSetup_Hdmi3(FALSE)
, m_bSetup_Hdmi4(FALSE)
, m_bSetup_Scart(FALSE)
, m_bSetup_Pc(FALSE)
, m_bSetup_Usb(FALSE)
{
	UpdateData(true);
	CserialCommunicationApp *pApp = (CserialCommunicationApp *)AfxGetApp();
	//pApp->app_strSource = m_Iflow;   // m_Iflow  是a编辑框关联的变量
	//parent->GetDlgItem()->SetWindowText()
	for (int j = 0; j < 20; j++)
	{
		if (pApp->app_strSource.Find("DTV1") != -1)
			m_bSetup_Dtv1 = TRUE;
		if (pApp->app_strSource.Find("DTV2") != -1)
			m_bSetup_Dtv2 = TRUE;
		if (pApp->app_strSource.Find("DTV3") != -1)
			m_bSetup_Dtv3 = TRUE;
		if (pApp->app_strSource.Find("ATV1") != -1)
			m_bSetup_Atv1 = TRUE;
		if (pApp->app_strSource.Find("ATV2") != -1)
			m_bSetup_Atv2 = TRUE;
		if (pApp->app_strSource.Find("DVBS") != -1)
			m_bSetup_Dvbs = TRUE;
		if (pApp->app_strSource.Find("AV1") != -1)
			m_bSetup_Av1 = TRUE;
		if (pApp->app_strSource.Find("AV2") != -1)
			m_bSetup_Av2 = TRUE;
		if (pApp->app_strSource.Find("YPBPR") != -1)
			m_bSetup_Ypbpr = TRUE;
		if (pApp->app_strSource.Find("HDMI1") != -1)
			m_bSetup_Hdmi1 = TRUE;
		if (pApp->app_strSource.Find("HDMI2") != -1)
			m_bSetup_Hdmi2 = TRUE;
		if (pApp->app_strSource.Find("HDMI3") != -1)
			m_bSetup_Hdmi3 = TRUE;
		if (pApp->app_strSource.Find("HDMI4") != -1)
			m_bSetup_Hdmi4 = TRUE;
		if (pApp->app_strSource.Find("SCART") != -1)
			m_bSetup_Scart = TRUE;
		if (pApp->app_strSource.Find("PC") != -1)
			m_bSetup_Pc = TRUE;
		if (pApp->app_strSource.Find("USB") != -1)
			m_bSetup_Usb = TRUE;
	}
	//UpdateData(FALSE);

}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_SETTIME_DTV1, m_iSetTime_Dtv1);
	DDV_MinMaxInt(pDX, m_iSetTime_Dtv1, 1, 50);
	DDX_Text(pDX, IDC_EDIT_SETTIME_DTV2, m_iSetTime_Dtv2);
	DDV_MinMaxInt(pDX, m_iSetTime_Dtv2, 1, 50);
	DDX_Check(pDX, IDC_CHECK_DTV1, m_bSetup_Dtv1);
	DDX_Check(pDX, IDC_CHECK_DTV2, m_bSetup_Dtv2);
	DDX_Check(pDX, IDC_CHECK_DTV3, m_bSetup_Dtv3);
	DDX_Check(pDX, IDC_CHECK_ATV1, m_bSetup_Atv1);
	DDX_Check(pDX, IDC_CHECK_ATV2, m_bSetup_Atv2);
	DDX_Check(pDX, IDC_CHECK_DVBS, m_bSetup_Dvbs);
	DDX_Check(pDX, IDC_CHECK_AV1, m_bSetup_Av1);
	DDX_Check(pDX, IDC_CHECK_AV2, m_bSetup_Av2);
	DDX_Check(pDX, IDC_CHECK_YPBPR, m_bSetup_Ypbpr);
	DDX_Check(pDX, IDC_CHECK_HDMI1, m_bSetup_Hdmi1);
	DDX_Check(pDX, IDC_CHECK_HDMI2, m_bSetup_Hdmi2);
	DDX_Check(pDX, IDC_CHECK_HDMI3, m_bSetup_Hdmi3);
	DDX_Check(pDX, IDC_CHECK1_HDMI4, m_bSetup_Hdmi4);
	DDX_Check(pDX, IDC_CHECK_SCART, m_bSetup_Scart);
	DDX_Check(pDX, IDC_CHECK_PC, m_bSetup_Pc);
	DDX_Check(pDX, IDC_CHECK_USB, m_bSetup_Usb);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CserialCommunicationDlg 对话框




CserialCommunicationDlg::CserialCommunicationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CserialCommunicationDlg::IDD, pParent)
	, m_cEdit_value_use(_T(""))
	, m_strEdit_colorTmp(_T(""))
	, m_strOtherTestDelay(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hIcon2 = AfxGetApp()->LoadIcon(IDI_ICON1);
	m_port = _T("");
	m_baud = _T("");
	m_cEdit_value_use_OK = _T("");
	m_cEdit_value_use_NG = _T("");
	m_strOtherTestDelay = _T("");
	m_brush.CreateSolidBrush(RGB(255,255,255));
}

void CserialCommunicationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//DDX_Text(pDX, IDC_EDIT1, m_cEdit_value_use);
	DDX_Text(pDX, IDC_EDIT_COLORTMP, m_strEdit_colorTmp);
	DDX_Text(pDX, IDC_EDIT_OK, m_cEdit_value_use_OK);
	DDX_Text(pDX, IDC_EDIT_FAILED, m_cEdit_value_use_NG);

	DDX_CBString(pDX, IDC_COMBO_PORT_SELECT, m_port);
	DDX_CBString(pDX, IDC_COMBO_BAUD_SELECT, m_baud);
	DDX_Check(pDX, IDC_CHECK_WB, m_checkwb);
	DDX_Control(pDX, IDC_MFCBUTTON_SOURCE, m_cMfcButton_SOURCE);
	DDX_Control(pDX, IDC_MFCBUTTON_CHP, m_cMfcButton_CHP);
	DDX_Control(pDX, IDC_MFCBUTTON_CHM, m_cMfcButton_CHM);
	DDX_Control(pDX, IDC_MFCBUTTON_VOLP, m_cMfcButton_VOLP);
	DDX_Control(pDX, IDC_MFCBUTTON_VOLM, m_cMfcButton_VOLM);
	DDX_Control(pDX, IDC_MFCBUTTON_MENU, m_cMfcButton_MENU);
	DDX_Control(pDX, IDC_MFCBUTTON_POWER, m_cMfcButton_POWER);
	DDX_Control(pDX, IDC_COMBO_DELAY, m_CComboBox_delay);
	DDX_CBString(pDX, IDC_COMBO_DELAY, m_strOtherTestDelay);
	//DDX_Control(pDX, IDC_MFCBUTTON_TIME, m_CMFButton_Time);
	//add for MES
	DDX_Control(pDX, IDC_COMBO_LINE, m_CCombox_Line);
	DDX_Control(pDX, IDC_COMBO_DATA_SECCLE, m_CCombox_model);
	DDX_Control(pDX, IDC_COMBO_TestProject, m_CCombox_testproject);
	DDX_Control(pDX, IDC_LIST_DATA, m_list);
	DDX_Control(pDX, IDC_EDIT_JNUMBER, m_userid);
	DDX_Control(pDX, IDC_EDIT_ORDER, m_orderid);
	DDX_Control(pDX, IDC_STATIC_SHOW_MESSAGE, m_psdinfo);
	DDX_Control(pDX, IDC_EDIT_INTO_MESSAGE, m_IntoEdit);
	//DDX_Control(pDX, IDC_BUTTON_INTO, m_Button_in);
	DDX_Control(pDX, IDC_EDIT_SN_DATA, m_Check_MAC1);
	DDX_Control(pDX, IDC_EDIT_MAC_DATA, m_Check_MAC2);
	//DDX_Control(pDX, IDC_EDIT_BUILD_MESSAGE, m_SN_length);
	//DDX_Control(pDX, IDC_EDIT_SW_VERSION, m_public_data);
	DDX_Control(pDX, IDC_STATIC_SHOW_TIMER, m_time);
	DDX_Control(pDX, IDC_COMBO_DATA_READ, m_Combox_Read);
	DDX_Control(pDX, IDC_COMBO_MAINKEY, m_CComBoMainKey);
	//////////////////
}

void CserialCommunicationDlg::Readconfig()
{
	CFileFind f;  
	BOOL bFind = f.FindFile(".\\config.ini");
	if (!bFind)
	{
		return;
	}

	::GetPrivateProfileString("COMM", "port", "", m_port.GetBuffer(20), 20, m_strConfigFile);			//串口号
	::GetPrivateProfileString("COMM", "baudRate", "", m_baud.GetBuffer(20), 20, m_strConfigFile);    //波特率e);    //波特率
	::GetPrivateProfileString("COMM", "otherTestDelay", "", m_strOtherTestDelay.GetBuffer(20), 20, m_strConfigFile);
	m_port.ReleaseBuffer();
	m_baud.ReleaseBuffer();
	m_strOtherTestDelay.ReleaseBuffer();
	UpdateData(false);

	CString strOpenBtn;
	::GetPrivateProfileString("COMM", "openBtn", "0", strOpenBtn.GetBuffer(2), 2, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	if (strOpenBtn == "1")
	{
		OnBnClickedButtonSerialControl();
	}
	CString strOpendelay;
	::GetPrivateProfileString("COMM", "otherTestDelay", "200ms", strOpendelay.GetBuffer(6), 6, m_strConfigFile);
	if (strOpendelay == "100ms")
		iOtherTestDelay = 100;
	else if (strOpendelay == "200ms")
		iOtherTestDelay = 200;
	else if (strOpendelay == "300ms")
		iOtherTestDelay = 300;
	else if (strOpendelay == "400ms")
		iOtherTestDelay = 400;
	else
		iOtherTestDelay = 500;
}

BEGIN_MESSAGE_MAP(CserialCommunicationDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_CBN_SETFOCUS(IDC_COMBO_PORT_SELECT, &CserialCommunicationDlg::OnCbnSetfocusComboPortSelect)
	ON_CBN_CLOSEUP(IDC_COMBO_BAUD_SELECT, &CserialCommunicationDlg::OnCbnCloseupComboBaudSelect)
	ON_CBN_CLOSEUP(IDC_COMBO_DATA_BIT_SELECT, &CserialCommunicationDlg::OnCbnCloseupComboDataBitSelect)
	ON_CBN_CLOSEUP(IDC_COMBO_STOP_SELECT, &CserialCommunicationDlg::OnCbnCloseupComboStopSelect)
	ON_CBN_CLOSEUP(IDC_COMBO_CHECK_SELECT, &CserialCommunicationDlg::OnCbnCloseupComboCheckSelect)
	ON_BN_CLICKED(IDC_BUTTON_SERIAL_CONTROL, &CserialCommunicationDlg::OnBnClickedButtonSerialControl)
	ON_CBN_SETFOCUS(IDC_COMBO_DELAY, &CserialCommunicationDlg::OCbnSetfocusComboDelay)
	ON_CBN_CLOSEUP(IDC_COMBO_DELAY, &CserialCommunicationDlg::OnCbnSelchangeComboDelay)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_RECEIVE_CLEAR, &CserialCommunicationDlg::OnBnClickedButtonReceiveClear)
	ON_BN_CLICKED(IDC_BUTTON_RECEIVE_SAVE_FILE, &CserialCommunicationDlg::OnBnClickedButtonReceiveSaveFile)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CserialCommunicationDlg::OnBnClickedButtonSend)
	ON_BN_CLICKED(IDC_BUTTON_SEND_CLEAR, &CserialCommunicationDlg::OnBnClickedButtonSendClear)
	ON_BN_CLICKED(IDC_BUTTON_SEND_FILE, &CserialCommunicationDlg::OnBnClickedButtonSendFile)
	ON_CBN_SETFOCUS(IDC_COMBO_CONFIG_FILE_SELECT, &CserialCommunicationDlg::OnCbnSetfocusComboConfigFileSelect)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_CONFIG_FILE, &CserialCommunicationDlg::OnBnClickedButtonLoadConfigFile)
	ON_CBN_CLOSEUP(IDC_COMBO_SEND_STRING_CONFIG, &CserialCommunicationDlg::OnCbnCloseupComboSendStringConfig)
	ON_BN_CLICKED(IDC_BUTTON3, &CserialCommunicationDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON_TEST_STOP, &CserialCommunicationDlg::OnBnClickedButtonTestStop)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_LOAD_CONFIG_FILE2, &CserialCommunicationDlg::OnBnClickedButtonLoadConfigFile2)
	ON_BN_CLICKED(IDC_CHECK_WB, &CserialCommunicationDlg::OnBnClickedWB)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//ON_BN_CLICKED(IDC_BUTTON_SETUP, &CserialCommunicationDlg::OnBnClickedButtonSetup)
	//add for MES
	ON_BN_CLICKED(IDC_CHECK_SN, &CserialCommunicationDlg::OnBnClickedCheckSn)
	ON_BN_CLICKED(IDC_CHECK_MAC, &CserialCommunicationDlg::OnBnClickedCheckMac)
	//<--
END_MESSAGE_MAP()

void CserialCommunicationDlg::OnBnClickedCheckSn()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strConfigData("");
	if (BST_UNCHECKED == IsDlgButtonChecked(IDC_CHECK_SN))
	{
		SetDlgItemText(IDC_EDIT_SN_DATA,_T(""));
		GetDlgItem(IDC_EDIT_SN_DATA)->EnableWindow(false);
	}
	else
	{
		::GetPrivateProfileString(CONFIGMEG, _T("SNDataLen"), _T(""), strConfigData.GetBuffer(5), 5, m_strConfigFile);
		SetDlgItemText(IDC_EDIT_SN_DATA,strConfigData);
		intSNLen = _ttoi(strConfigData);
		GetDlgItem(IDC_EDIT_SN_DATA)->EnableWindow(true);
	}
}

void CserialCommunicationDlg::OnBnClickedCheckMac()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strConfigData("");
	if (BST_UNCHECKED == IsDlgButtonChecked(IDC_CHECK_MAC))
	{
		SetDlgItemText(IDC_EDIT_MAC_DATA,_T(""));
		//GetDlgItem(IDC_EDIT_MAINKEY)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_MAC_DATA)->EnableWindow(false);
	}
	else
	{
		::GetPrivateProfileString(CONFIGMEG, _T("MACCheck"), _T(""), strConfigData.GetBuffer(10), 10, m_strConfigFile);
		SetDlgItemText(IDC_EDIT_MAC_DATA,strConfigData);
		intMACLen = _ttoi(strConfigData);
		//MacCheck = strConfigData.MakeLower();
		//GetDlgItem(IDC_EDIT_MAINKEY)->EnableWindow(true);
		GetDlgItem(IDC_EDIT_MAC_DATA)->EnableWindow(true);
	}
}

//实现控件随窗口大小而变话  
void CserialCommunicationDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	if (nType != SIZE_MINIMIZED) {
#if 0
		int tmpRight = 0;
		CRect rect;
		//CWnd *pWnd;
		//pWnd[0] = GetDlgItem(IDC_STATIC_PICTURE);
		if (GetDlgItem(IDC_STATIC_PICTURE))
		{
			GetDlgItem(IDC_STATIC_PICTURE)->GetWindowRect(&rect);
			ScreenToClient(&rect);
			rect.left = rect.left;
			rect.right = rect.right*cx / m_rect.Width();
			tmpRight = rect.right;
			rect.top = rect.top;
			rect.bottom = rect.bottom*cy / m_rect.Height();
			GetDlgItem(IDC_STATIC_PICTURE)->MoveWindow(rect);//设置控件大小  伸缩控件  
		}
		if (GetDlgItem(IDC_EDIT1))
		{
			GetDlgItem(IDC_EDIT1)->GetWindowRect(&rect);
			ScreenToClient(&rect);
			rect.left = tmpRight + 2;
			rect.right = rect.right*cx / m_rect.Width();
			tmpRight = rect.right;
			rect.top = rect.top;
			rect.bottom = rect.bottom*cy / m_rect.Height();
			GetDlgItem(IDC_EDIT1)->MoveWindow(rect);//设置控件大小  伸缩控件
			GetDlgItem(IDC_EDIT_OK)->MoveWindow(rect);
			GetDlgItem(IDC_EDIT_FAILED)->MoveWindow(rect);
		}
		if (GetDlgItem(IDC_EDIT_TEST_ITEM))
		{
			GetDlgItem(IDC_EDIT_TEST_ITEM)->GetWindowRect(&rect);
			ScreenToClient(&rect);
			rect.left = tmpRight + 2;
			rect.right = rect.right*cx / m_rect.Width();
			tmpRight = rect.right;
			rect.top = rect.top;
			rect.bottom = rect.bottom*cy / m_rect.Height();
			GetDlgItem(IDC_EDIT_TEST_ITEM)->MoveWindow(rect);//设置控件大小  伸缩控件
		}
		if (GetDlgItem(IDC_EDIT_RECEIVE))
		{
			GetDlgItem(IDC_EDIT_RECEIVE)->GetWindowRect(&rect);
			ScreenToClient(&rect);
			rect.left = tmpRight + 2;
			rect.right = rect.right*cx / m_rect.Width();
			tmpRight = rect.right;
			rect.top = rect.top;
			rect.bottom = rect.bottom*cy / m_rect.Height();
			GetDlgItem(IDC_EDIT_RECEIVE)->MoveWindow(rect);//设置控件大小  伸缩控件
			GetDlgItem(IDC_STATIC)->MoveWindow(rect);//设置控件大小  伸缩控件

		}


		GetClientRect(&m_rect);//最后要更新对话框的大小，当做下一次变化的旧坐标；   
#endif
	 // TODO: Add your message handler code here  
#if 1
		CWnd *pWnd[72];
		pWnd[0] = GetDlgItem(IDC_STATIC_PICTURE);
		//pWnd[1] = GetDlgItem(IDC_EDIT1); //<--IDC_EDIT_ORDER
		pWnd[1] = GetDlgItem(IDC_EDIT_ORDER);
		pWnd[2] = GetDlgItem(IDC_EDIT_OK);
		pWnd[3] = GetDlgItem(IDC_EDIT_FAILED);
		pWnd[4] = GetDlgItem(IDC_EDIT_TEST_ITEM);
		pWnd[5] = GetDlgItem(IDC_EDIT_RECEIVE);
		pWnd[6] = GetDlgItem(IDC_STATIC);
		pWnd[7] = GetDlgItem(IDC_BUTTON3);
		pWnd[8] = GetDlgItem(IDC_EDIT_COLORTMP);
		pWnd[9] = GetDlgItem(IDC_COMBO_MAINKEY);
		//pWnd[10] = GetDlgItem(IDC_EDIT_TESTING);
		pWnd[10] = GetDlgItem(IDC_STATIC_SHOW_MESSAGE);//<--
		pWnd[11] = GetDlgItem(IDC_EDIT_VERSION);
		pWnd[12] = GetDlgItem(IDC_STATIC_KEYPAD);
		pWnd[13] = GetDlgItem(IDC_MFCBUTTON_SOURCE);
		pWnd[14] = GetDlgItem(IDC_MFCBUTTON_MENU);
		pWnd[15] = GetDlgItem(IDC_MFCBUTTON_POWER);
		pWnd[16] = GetDlgItem(IDC_MFCBUTTON_CHP);
		pWnd[17] = GetDlgItem(IDC_MFCBUTTON_CHM);
		pWnd[18] = GetDlgItem(IDC_MFCBUTTON_VOLP);
		pWnd[19] = GetDlgItem(IDC_MFCBUTTON_VOLM);
		pWnd[20] = GetDlgItem(IDC_STATIC_VERSION);
		pWnd[21] = GetDlgItem(IDC_BUTTON_RECEIVE_SAVE_FILE);
		pWnd[22] = GetDlgItem(IDC_BUTTON_RECEIVE_CLEAR);
		pWnd[23] = GetDlgItem(IDC_STATIC_DATA);
		pWnd[24] = GetDlgItem(IDC_EDIT_SEND);
		pWnd[25] = GetDlgItem(IDC_BUTTON_SEND_FILE);
		pWnd[26] = GetDlgItem(IDC_BUTTON_SEND_CLEAR);
		pWnd[27] = GetDlgItem(IDC_BUTTON_SEND);
		pWnd[28] = GetDlgItem(IDC_COMBO_SEND_STRING_CONFIG);
		pWnd[29] = GetDlgItem(IDC_STATIC_MAIN_VER);
		pWnd[30] = GetDlgItem(IDC_BUTTON_LOAD_CONFIG_FILE2);
		pWnd[31] = GetDlgItem(IDC_BUTTON_TEST_STOP);
		pWnd[32] = GetDlgItem(IDC_STATIC_DELAY);
		pWnd[33] = GetDlgItem(IDC_COMBO_DELAY);
		pWnd[34] = GetDlgItem(IDC_BUTTON_LOAD_CONFIG_FILE);
		pWnd[35] = GetDlgItem(IDC_EDIT_JNUMBER);
		pWnd[36] = GetDlgItem(IDC_EDIT_INTO_MESSAGE);
		pWnd[37] = GetDlgItem(IDC_LIST_DATA);
		pWnd[38] = GetDlgItem(IDC_STATIC_SHOW_TIMER);
		pWnd[39] = GetDlgItem(IDC_COMBO_PORT_SELECT);
		pWnd[40] = GetDlgItem(IDC_COMBO_BAUD_SELECT);
		pWnd[41] = GetDlgItem(IDC_COMBO_STOP_SELECT);
		pWnd[42] = GetDlgItem(IDC_COMBO_CHECK_SELECT);
		pWnd[43] = GetDlgItem(IDC_BUTTON_SERIAL_CONTROL);
		pWnd[44] = GetDlgItem(IDC_STATIC_USBscan);
		pWnd[45] = GetDlgItem(IDC_CHECK_WB);
		pWnd[46] = GetDlgItem(IDC_STATIC_chuankou);
		pWnd[47] = GetDlgItem(IDC_STATIC_botelv);
		pWnd[48] = GetDlgItem(IDC_STATIC_shujuwei);
		pWnd[49] = GetDlgItem(IDC_STATIC_tingzhiwei);
		pWnd[50] = GetDlgItem(IDC_STATIC_xiaoyanwei);
		pWnd[51] = GetDlgItem(IDC_STATIC_ckkongzhi);
		pWnd[52] = GetDlgItem(IDC_COMBO_DATA_BIT_SELECT);
		pWnd[53] = GetDlgItem(IDC_STATIC_tongxunpeizhi);
		pWnd[54] = GetDlgItem(IDC_STATIC_shujupeizhi);
		pWnd[55] = GetDlgItem(IDC_STATIC_gonghao);
		pWnd[56] = GetDlgItem(IDC_EDIT_MAINKEY);
		pWnd[57] = GetDlgItem(IDC_STATIC_dingdanhao);
		pWnd[58] = GetDlgItem(IDC_CHECK_SN);
		pWnd[59] = GetDlgItem(IDC_CHECK_MAC);
		pWnd[60] = GetDlgItem(IDC_STATIC_gonggong);
		pWnd[61] = GetDlgItem(IDC_STATIC_zhujianming);
		pWnd[62] = GetDlgItem(IDC_STATIC_xianti);
		pWnd[63] = GetDlgItem(IDC_STATIC_huoqushuju);
		pWnd[64] = GetDlgItem(IDC_STATIC_jianyanshuju);
		pWnd[65] = GetDlgItem(IDC_STATIC_ceshifangan);
		pWnd[66] = GetDlgItem(IDC_COMBO_LINE);
		pWnd[67] = GetDlgItem(IDC_COMBO_DATA_READ);
		pWnd[68] = GetDlgItem(IDC_COMBO_DATA_SECCLE);
		pWnd[69] = GetDlgItem(IDC_COMBO_TestProject);
		pWnd[70] = GetDlgItem(IDC_EDIT_SN_DATA);
		pWnd[71] = GetDlgItem(IDC_EDIT_MAC_DATA);


		for (int i = 0; i < 72; i++)
		{
			//获取控件句柄   
			if (pWnd[i])//判断是否为空，因为对话框创建时会调用此函数，而当时控件还未创建   
			{
				CRect rect; //获取控件变化前大小   
				pWnd[i]->GetWindowRect(&rect);
				ScreenToClient(&rect);//将控件大小转换为在对话框中的区域坐标   
				rect.left = rect.left*cx / m_rect.Width();/**//////调整控件大小   
				rect.right = rect.right*cx / m_rect.Width();
				rect.top = rect.top*cy / m_rect.Height();
				rect.bottom = rect.bottom*cy / m_rect.Height();
				pWnd[i]->MoveWindow(rect);//设置控件大小  伸缩控件  
			}
		}
		GetClientRect(&m_rect);//最后要更新对话框的大小，当做下一次变化的旧坐标；   
#endif
	}
}

// CserialCommunicationDlg 消息处理程序
BOOL CserialCommunicationDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	//ShowWindow(SW_MAXIMIZE);
	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon2, TRUE);		// 设置小图标
	//add for MES
	FontSetting();
	InitData();
	GetConfigData();
	///////////////////////////////
	//获取对话框创建时的大小  
	GetClientRect(&m_rect);

	// TODO: 在此添加额外的初始化代码
	((CComboBox*)GetDlgItem(IDC_COMBO_BAUD_SELECT))->SetCurSel(5);
	((CComboBox*)GetDlgItem(IDC_COMBO_DATA_BIT_SELECT))->SetCurSel(2);
	((CComboBox*)GetDlgItem(IDC_COMBO_STOP_SELECT))->SetCurSel(0);
	((CComboBox*)GetDlgItem(IDC_COMBO_CHECK_SELECT))->SetCurSel(0);

	((CComboBox*)GetDlgItem(IDC_COMBO_SEND_STRING_CONFIG))->SetCurSel(0);
	((CComboBox*)GetDlgItem(IDC_COMBO_DELAY))->SetCurSel(1);

	baud = B115200;
	data_length = BIT_8;
	check_mode = P_NONE;
	stop_bit = STOP_1;
	serial_isopen = FALSE;
	com_port = 0;
	countMacPass = 0;
	sendlrcr = FALSE;

	showString.Empty();
	sendString.Empty();
	serialReadTemp.Empty();

	GetCurrentDirectory(100,m_strConfigFile.GetBuffer(100));
	m_strConfigFile.ReleaseBuffer();
	m_strConfigFile += "\\config.ini";

	configFileLoadOk = false;
	colorTmpconfigFileLoadOk = false;
	buttonSendString = new CStringArray;
	listSendString = new CStringArray;

	sendStringConfig = false;
	bFactoryTestOk = false;
	bWaitFeedback = false;
	bStartCheckData = false;
	cCheckCout = 0;
	iCurretIndex = 0;
	bTestKeyPad_Source = FALSE;
	bTestKeyPad_Menu = FALSE;
	bTestKeyPad_Power = FALSE;
	bTestKeyPad_CHPlus = FALSE;
	bTestKeyPad_CHmins = FALSE;
	bTestKeyPad_VolPlus = FALSE;
	bTestKeyPad_VolMins = FALSE;
	cCheckCout_tmp = 0;
	sVersion = "";
	sMacAddr = "";
	curretBufString_test = "";
	GetMacData = "";
	strPath.RemoveAll();
	
	m_toolstip.Create(this);
	m_toolstip.AddTool(GetDlgItem(IDC_BUTTON_SEND_FILE),"通过串口发送文件,无通讯协议的发送");

	//SetTimer(TIME_SHOW_MESSAGE_SERIAL,10,NULL);	//用于刷新edit界面 10MS一次
	SetTimer(TIME_SHOW_MESSAGE_SERIAL,200,NULL);	//用于刷新edit界面 200MS一次 测试阶段试试效果
	//GetDlgItem(IDC_BUTTON_CONTROL)->EnableWindow(true);
	GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_TEST_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_COMBO_DATA_BIT_SELECT)->EnableWindow(FALSE);
	GetDlgItem(IDC_COMBO_STOP_SELECT)->EnableWindow(FALSE);
	GetDlgItem(IDC_COMBO_CHECK_SELECT)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_PICTURE)->ShowWindow(SW_HIDE);//<--
	GetDlgItem(IDC_EDIT_INTO_MESSAGE)->EnableWindow(FALSE);
	m_cMfcButton_SOURCE.EnableWindow(FALSE);
	m_cMfcButton_MENU.EnableWindow(FALSE);
	m_cMfcButton_CHP.EnableWindow(FALSE);
	m_cMfcButton_CHM.EnableWindow(FALSE);
	m_cMfcButton_VOLP.EnableWindow(FALSE);
	m_cMfcButton_VOLM.EnableWindow(FALSE);
	m_cMfcButton_POWER.EnableWindow(FALSE);

	LoadConfigXMLFile();
	OnCbnSetfocusComboPortSelect();
	
	//CFont m_Font;
	//m_Font.CreateFont(10, 0, 0, 0, 100, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_SWISS, "Arial");
	//m_Font_Small.CreatePointFont(150, _T("Arial"), NULL);
	m_Font.CreatePointFont(200, _T("Arial"), NULL);
	m_Font_middle.CreatePointFont(250, _T("Arial"), NULL);
	m_Font_Big.CreatePointFont(350, _T("Arial"), NULL);
	//GetDlgItem(IDC_EDIT1)->SetFont(&m_Font, FALSE); //<--
	GetDlgItem(IDC_EDIT_OK)->SetFont(&m_Font, FALSE);//<--
	GetDlgItem(IDC_EDIT_FAILED)->SetFont(&m_Font, FALSE);
	GetDlgItem(IDC_EDIT_INTO_MESSAGE)->SetFont(&m_Font_middle, FALSE);
	//GetDlgItem(IDC_EDIT_TEST_ITEM)->SetFont(&m_Font, FALSE);
	//GetDlgItem(IDC_EDIT_TESTING)->SetFont(&m_Font_Big, FALSE);//<--
	//GetDlgItem(IDC_EDIT1)->SetWindowText(_T(""));
	GetDlgItem(IDC_EDIT_OK)->SetWindowText(_T(""));
	GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(_T(""));
	GetDlgItem(IDC_EDIT_TEST_ITEM)->SetWindowText(_T("测试项目"));

	//GetDlgItem(IDC_EDIT1)->ShowWindow(SW_SHOW);
	//GetDlgItem(IDC_EDIT_OK)->ShowWindow(SW_HIDE);
	//GetDlgItem(IDC_EDIT_FAILED)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_OK)->ShowWindow(SW_SHOW);  //<--
	GetDlgItem(IDC_EDIT_FAILED)->ShowWindow(SW_SHOW);

	GetDlgItem(IDC_EDIT_VERSION)->SetFont(&m_Font_middle, FALSE);
	GetDlgItem(IDC_BUTTON3)->SetFont(&m_Font, FALSE);
	
	//m_MFCButton_keypadResuilt.SetFont(&m_Font_Big, FALSE);
	//m_MFCButton_keypadResuilt.SetFaceColor(RGB(255, 255, 255));
	//GetDlgItem(IDC_TEST_RESUILT)->SetWindowText("NG");
	
	Readconfig();

	GetDlgItem(IDC_BUTTON_LOAD_CONFIG_FILE2)->EnableWindow(FALSE);
	return FALSE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CserialCommunicationDlg::FontSetting()//初始化字体设置
{
	m_ftCtrl.CreateFont( 32,
		0,
		0,
		0,
		FW_BOLD,
		FALSE,
		FALSE,
		0,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS,
		_T("黑体"));
	m_psdinfo.SetFont(&m_ftCtrl);
	m_time.SetFont(&m_ftCtrl);
}

void CserialCommunicationDlg::InitData()
{
	//创建初始化配置文件，用于保存一些操作信息。
	GetCurrentDirectory(100,m_strConfigFile.GetBuffer(100));  //取得当前运行路径
	m_strConfigFile.ReleaseBuffer();
	m_strConfigFile += "\\config.ini";

	ControlID_Flag = BLUE;
	number = 1;
	mytimer = 0;

	GetDlgItem(IDC_EDIT_JNUMBER)->SetFocus();
	//3个组合框赋初值
	m_CCombox_Line.AddString(_T("1A"));
	m_CCombox_Line.AddString(_T("2A"));
	m_CCombox_Line.AddString(_T("3A"));
	m_CCombox_Line.SetCurSel(0);//设置列表框默认值
	m_CCombox_Line.EnableWindow(FALSE);
	m_CCombox_testproject.AddString(_T("T962"));
	m_CCombox_testproject.AddString(_T("V510"));
	m_CCombox_testproject.AddString(_T("T966"));
	m_CCombox_testproject.AddString(_T("T968"));
	m_CCombox_testproject.SetCurSel(0);//设置列表框默认值
	m_CCombox_testproject.EnableWindow(TRUE);
	m_CComBoMainKey.AddString(_T("SN"));
	m_CComBoMainKey.AddString(_T("BarCode"));
	m_CComBoMainKey.SetCurSel(0);//设置列表框默认值
	m_CCombox_model.AddString(_T("蓝牙"));
	m_CCombox_model.SetCurSel(0);//设置列表框默认值
	m_CCombox_model.EnableWindow(FALSE);
	m_Combox_Read.AddString(_T("SN"));
	m_Combox_Read.SetCurSel(0);//设置读取列表框默认信息
	m_Combox_Read.EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK_MAC)->EnableWindow(TRUE);
	//list control的初始化
	//m_list.SetImageList(&m_Image,LVSIL_SMALL); 
	m_list.SetImageList(&m_Image, LVSIL_NORMAL);
	m_list.ModifyStyle(LVS_TYPEMASK,LVS_REPORT);
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_FLATSB|LVS_EX_TWOCLICKACTIVATE |LVS_EX_GRIDLINES);
	m_list.InsertColumn(0,_T("数量"),LVCFMT_LEFT,40);
	m_list.InsertColumn(1,_T("时间"),LVCFMT_LEFT,160);
	m_list.InsertColumn(2,_T("测试结果"),LVCFMT_LEFT,80);
	m_list.InsertColumn(3,_T("MAC"),LVCFMT_LEFT,140);
	m_list.InsertColumn(4,_T("SN"),LVCFMT_LEFT,180);
	m_IntoEdit.EnableWindow(FALSE);
	//m_Button_in.EnableWindow(FALSE);
	m_uTimerPsd=SetTimer(TIMER_ORDER,200,NULL);//设置一个定时器检查订单号的输入
	ShowMessage(BLUE,_T("请输入有效员工号！"),0);
	Order_Flag=0;//用来判断第次输入订单号
	
}

void CserialCommunicationDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CserialCommunicationDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CserialCommunicationDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CserialCommunicationDlg::OnCbnSetfocusComboPortSelect()
{
	//搜索系统当前串口，并实时显示在选择框中
	BYTE i = 0;
	CString str;
	str.Empty();
	((CComboBox*)GetDlgItem(IDC_COMBO_PORT_SELECT))->ResetContent();//清除显示项
	for(i = 0; i < 255; i++)
	{
		if(SIO_OK == sio_open(i))
		{
			 sio_close(i);
			 str.AppendFormat("COM%d",i);
			 ((CComboBox *)GetDlgItem(IDC_COMBO_PORT_SELECT))->AddString(str);
			 str.Empty();
		}
	}
	i = ((CComboBox *)GetDlgItem(IDC_COMBO_PORT_SELECT))->GetCount();
	if(i ==0)
	{
		GetDlgItem(IDC_BUTTON_SERIAL_CONTROL)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_BUTTON_SERIAL_CONTROL)->EnableWindow(TRUE);
	}
}

void CserialCommunicationDlg::OnCbnCloseupComboBaudSelect()
{
	// TODO: 在此添加控件通知处理程序代码
	int i = 0;
	i = ((CComboBox*)GetDlgItem(IDC_COMBO_BAUD_SELECT))->GetCurSel();
	if(-1== i)
	{
		baud = 0x80;
		MessageBox("请选择一个波特率","错误",MB_OK);
	}
	else
	{
		switch (i)
		{
		case 0:
			baud = B4800;
			break;
		case 1:
			baud = B9600;
			break;
		case 2:
			baud = B19200;
			break;
		case 3:
			baud = B38400;
			break;
		case 4:
			baud = B57600;
			break;
		case 5:
			baud = B115200;
			break;
		case 6:
			baud = B230400;
			break;
		case 7:
			baud = B460800;
			break;
		}
	}
}

void CserialCommunicationDlg::OnCbnCloseupComboDataBitSelect()
{
	int i = 0;
	i = ((CComboBox*)GetDlgItem(IDC_COMBO_DATA_BIT_SELECT))->GetCurSel();
	if(-1== i)
	{
		data_length = 0x80;
		MessageBox("请选择一个数据长度","错误",MB_OK);
	}
	else
	{
		switch (i)
		{
		case 0:
			data_length = BIT_6;
			break;
		case 1:
			data_length = BIT_7;
			break;
		case 2:
			data_length = BIT_8;
			break;
		}
	}
}

void CserialCommunicationDlg::OnCbnCloseupComboStopSelect()
{
	// TODO: 在此添加控件通知处理程序代码
	int i = 0;
	i = ((CComboBox*)GetDlgItem(IDC_COMBO_STOP_SELECT))->GetCurSel();
	if(-1== i)
	{
		stop_bit = 0x80;
		MessageBox("请选择一个停止位","错误",MB_OK);
	}
	else
	{
		switch (i)
		{
		case 0:
			stop_bit = STOP_1;
			break;
		case 1:
			stop_bit = STOP_2;
			break;
		}
	}
}

void CserialCommunicationDlg::OnCbnCloseupComboCheckSelect()
{
	// TODO: 在此添加控件通知处理程序代码
	int i = 0;
	i = ((CComboBox*)GetDlgItem(IDC_COMBO_CHECK_SELECT))->GetCurSel();
	if(-1== i)
	{
		check_mode = 0x80;
		MessageBox("请选择一个停止位","错误",MB_OK);
	}
	else
	{
		switch (i)
		{
		case 0:
			check_mode = P_EVEN;
			break;
		case 1:
			check_mode = P_ODD;
			break;
		case 2:
			check_mode = P_NONE;
			break;
		}
	}
}

void CserialCommunicationDlg::SerialReConnect()
{
	int i = 0;
	while (i++ <= 20)
	{
		Sleep(10);
		if (sio_open(com_port) == SIO_OK)
		{
			int config = data_length | stop_bit | check_mode;
			serial_isopen = TRUE;
			sio_flowctrl(com_port, 0x00);//关闭硬件流控制
			sio_lctrl(com_port, 0x00);//关闭RTS DTR
			sio_ioctl(com_port, baud, config);
			sio_flush(com_port, 2);
			break;
		}
	}
	//if (!serial_isopen)
	//{
	//	MessageBox("串口重连20次", "失败", MB_OK);
	//}
}

void CserialCommunicationDlg::OnBnClickedButtonSerialControl()
{
	// TODO: 在此添加控件通知处理程序代码
	if(serial_isopen == TRUE)//此时需要关闭串口
	{
		if(com_port > 0)
		{
			if(sio_lstatus(com_port) >= 0)
				sio_close(com_port);
			com_port = 0;
		}
		serial_isopen = FALSE;
		((CComboBox*)GetDlgItem(IDC_COMBO_PORT_SELECT))->EnableWindow(TRUE);
		((CComboBox*)GetDlgItem(IDC_COMBO_BAUD_SELECT))->EnableWindow(TRUE);
		((CComboBox*)GetDlgItem(IDC_COMBO_STOP_SELECT))->EnableWindow(TRUE);
		((CComboBox*)GetDlgItem(IDC_COMBO_CHECK_SELECT))->EnableWindow(TRUE);
		((CComboBox*)GetDlgItem(IDC_COMBO_DATA_BIT_SELECT))->EnableWindow(TRUE);
		((CButton*)GetDlgItem(IDC_BUTTON_SERIAL_CONTROL))->EnableWindow(TRUE);
		((CButton*)GetDlgItem(IDC_BUTTON_SERIAL_CONTROL))->SetWindowText("打开串口");
	}
	else
	{
		//获取当前选择的串口号码
		int i = 0;
		CString str;
		int config;
#if 0
		for (i = 0; i < 255; i++)
		{
			if (SIO_OK == sio_open(i))
			{
				sio_close(i);
				//str.AppendFormat("COM%d", i);
				//((CComboBox *)GetDlgItem(IDC_COMBO_PORT_SELECT))->AddString(str);
				//str.Empty();
				break;
			}
		}
#endif
		 if(-1==i)
		 {
			 MessageBox("请选择一个串口","失败",MB_OK);
			 return;
		 }
		 else
		 {
			 ((CComboBox*)GetDlgItem(IDC_COMBO_PORT_SELECT))->GetWindowText(str);
			 str = str.Mid(3,str.GetLength());
			 i = atoi(str);
			 //检测配置信息
			 if(baud == 0x80)
			 {
				 MessageBox("波特率未选择","错误",MB_OK);
				 return;
			 }
			 if(data_length == 0x80)
			 {
				 MessageBox("数据长度未选择","错误",MB_OK);
				 return;
			 }
			 if(stop_bit == 0x80)
			 {
				 MessageBox("停止位未选择","错误",MB_OK);
				 return;
			 }
			 if(check_mode == 0x80)
			 {
				 MessageBox("校验模式未选择","错误",MB_OK);
				 return;
			 }
			config = data_length|stop_bit|check_mode;
			 //开始串口配置
			 if( sio_open(i) != SIO_OK)
			 {
				 MessageBox("串口打开失败","提示",MB_OK);
				 this->serial_isopen = FALSE;
				 com_port = 0;
				 return;
			 }
			//运行到这里代表打开成功
			com_port = i;
			serial_isopen = TRUE;
			sio_flowctrl(com_port,0x00);//关闭硬件流控制
			sio_lctrl(com_port,0x00);//关闭RTS DTR
			sio_ioctl(com_port,baud,config);
			sio_flush(com_port,2);

			((CComboBox*)GetDlgItem(IDC_COMBO_PORT_SELECT))->EnableWindow(FALSE);
			((CComboBox*)GetDlgItem(IDC_COMBO_BAUD_SELECT))->EnableWindow(FALSE);
			((CComboBox*)GetDlgItem(IDC_COMBO_STOP_SELECT))->EnableWindow(FALSE);
			((CComboBox*)GetDlgItem(IDC_COMBO_CHECK_SELECT))->EnableWindow(FALSE);
			((CComboBox*)GetDlgItem(IDC_COMBO_DATA_BIT_SELECT))->EnableWindow(FALSE);
			((CButton*)GetDlgItem(IDC_BUTTON_SERIAL_CONTROL))->SetWindowText("关闭串口");
			//GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
			UpdateData();
			::WritePrivateProfileString("COMM","port",m_port,m_strConfigFile);
			::WritePrivateProfileString("COMM","baudRate",m_baud,m_strConfigFile);
			::WritePrivateProfileString("COMM","openBtn","1",m_strConfigFile);
			//::WritePrivateProfileString("COMM", "otherTestDelay", m_strOtherTestDelay, m_strConfigFile);
		 }
	}
}
void CserialCommunicationDlg::SerialsProcessBuffer( void )
{
	serialReadTemp.Empty();
	if(com_port > 0 && this->serial_isopen == TRUE)
	{
		char readBuffer[2048] = {0};
		//		sio_flush(m_comPort,1);
		int length = sio_read(com_port,readBuffer,2048);
		if(length > 0)
		{
			for(int i = 0; i < length; i++)
			{
				strHex.Format(_T("%02X"),(unsigned char)readBuffer[i]); 
				serialReadTemp += strHex;
				//serialReadTemp.AppendChar(readBuffer[i]);
			}
		}
	}
}
const char WBresbond[] = {0x03,0x0c,0xf1};// "MTC_WB_OK";
void CserialCommunicationDlg::SerialsCheckBuffer(CString curretBufString)
{
	if (bFacWriteWB_Normal_Red)
	{
		if (curretBufString.GetLength() < 3)
		{
			m_cEdit_value_use_NG +=  "normal_red  写失败";
			GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
			curretBufString_test = "";
		}
		bFacWriteWB_Normal_Red = false;
	}
	if (bFacWriteWB_Normal_Green)
	{
		if (curretBufString.GetLength() < 3)
		{
			m_cEdit_value_use_NG +=  "normal_green  写失败";
			GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
			curretBufString_test = "";
		}
		bFacWriteWB_Normal_Green = false;
	}
	if (bFacWriteWB_Normal_Blue)
	{
		if (curretBufString.GetLength() < 3)
		{
			m_cEdit_value_use_NG +=  "normal_blue  写失败";
			GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
			curretBufString_test = "";
		}
		//GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText("写色温Normal完成");
		bFacWriteWB_Normal_Blue = false;
	}
	if (bFacWriteWB_Cool_Red)
	{
		if (curretBufString.GetLength() < 3)
		{
			m_cEdit_value_use_NG +=  "cool_red  写失败";
			GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
			curretBufString_test = "";
		}
		bFacWriteWB_Cool_Red = false;
	}
	if (bFacWriteWB_Cool_Green)
	{
		if (curretBufString.GetLength() < 3)
		{
			m_cEdit_value_use_NG +=  "cool_green  写失败";
			GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
			curretBufString_test = "";
		}
		bFacWriteWB_Cool_Green = false;
	}
	if (bFacWriteWB_Cool_Blue)
	{
		if (curretBufString.GetLength() < 3)
		{
			m_cEdit_value_use_NG +=  "cool_blue  写失败";
			GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
			curretBufString_test = "";
		}
		//GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText("写色温Cool完成");
		bFacWriteWB_Cool_Blue = false;
	}
	if (bFacWriteWB_Warm_Red)
	{
		if (curretBufString.GetLength() < 3)
		{
			m_cEdit_value_use_NG +=  "warm_red  写失败";
			GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
			curretBufString_test = "";
		}
		bFacWriteWB_Warm_Red = false;
	}
	if (bFacWriteWB_Warm_Green)
	{
		if (curretBufString.GetLength() < 3)
		{
			m_cEdit_value_use_NG +=  "warm_green  写失败";
			GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
			curretBufString_test = "";
		}
		bFacWriteWB_Warm_Green = false;
	}
	if (bFacWriteWB_Warm_Blue)
	{
		if (curretBufString.GetLength() < 3)
		{
			m_cEdit_value_use_NG +=  "warm_blue  写失败";
			GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
			curretBufString_test = "";
		}
		GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText("写色温完成");
		bFacWriteWB_Warm_Blue = false;
	}
	if (curretBufString.Find("55B207") != -1)			//<--VER
	{
		if (TestVERBegin)
		//if (1)  //测试用
		{
			CheckData += curretBufString;
			if (CheckData.GetLength() == 24)
			{
				curretBufString = CheckData;
				int iStart = curretBufString.Find("55B207");  //07是返回数据位数
				int iEnd = 0;
				CString tmp1 = curretBufString.Mid(iStart + 6, 50);
				iEnd = tmp1.Find("FE");
				if (iEnd == -1)
					iEnd = 30;
				sVersion.Empty();
				sVersion = tmp1.Left(14);
				//MessageBox(sVersion);
				CString VERSIONMonth = sVersion.Left(6);
				CString VMonth1 = VERSIONMonth.Left(2);
				CString VMonth2 = VERSIONMonth.Mid(2, 2);
				CString VMonth3 = VERSIONMonth.Right(2);
				int xMonth1 = strtol(VMonth1, NULL, 16);//此时，xMonth1=77；
				int xMonth2 = strtol(VMonth2, NULL, 16);
				int xMonth3 = strtol(VMonth3, NULL, 16);
				CString VERSIONTime = sVersion.Right(8);
				CString VTime1 = VERSIONTime.Left(2);
				CString VTime2 = VERSIONTime.Mid(2, 2);
				CString VTime3 = VERSIONTime.Mid(4, 2);
				CString VTime4 = VERSIONTime.Right(2);
				int xTime1 = strtol(VTime1, NULL, 16);
				int xTime2 = strtol(VTime2, NULL, 16);
				int xTime3 = strtol(VTime3, NULL, 16);
				int xTime4 = strtol(VTime4, NULL, 16);
				sVersion.Format("%c", xMonth1);
				sVersion.AppendFormat("%c", xMonth2);
				sVersion.AppendFormat("%c ", xMonth3);
				if (xTime1 < 10)
				{
					sVersion.AppendFormat("%d ", xTime1);
				}
				else
				{
					sVersion.AppendFormat("%02d ", xTime1);
				}
				sVersion.AppendFormat("%02d:", xTime2);
				sVersion.AppendFormat("%02d:", xTime3);
				sVersion.AppendFormat("%02d", xTime4);
				//GetDlgItem(IDC_EDIT_VERSION)->SetWindowText(sVersion);
				CheckData = "";
				if (sVersion_config.Find(sVersion) != -1)
				{
					m_cEdit_value_use_OK += "VER	      PASS\r\n";
					GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
					TestVERBegin = FALSE;
					curretBufString_test = "";
				}
				else
				{
					if (strTestProject.Find("T962") != -1)
					{
						if (countVer < 2)
						{
							countVer++;
							TestVERBegin = TRUE;
							OnBnClickedButtonShort_fun(23);   //<--VER	
						}
						else
						{
							m_cEdit_value_use_NG += "VER	      NG\r\n";
							GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
							TestVERBegin = FALSE;
							OnBnClickedButtonReceiveSaveFile();
							curretBufString_test = "";
						}
					}
					else if (strTestProject.Find("T968") != -1)
					{
						if (countVer < 2)
						{
							countVer++;
							TestVERBegin = TRUE;
							OnBnClickedButtonShort_fun(23);   //<--VER	
						}
						else
						{
							m_cEdit_value_use_NG += "VER	      NG\r\n";
							GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
							TestVERBegin = FALSE;
							OnBnClickedButtonReceiveSaveFile();
							curretBufString_test = "";
						}
					}
					else if (strTestProject.Find("V510") != -1)
					{
						/*m_cEdit_value_use_NG += "VER	      NG\r\n";
						GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
						TestVERBegin = FALSE;
						OnBnClickedButtonReceiveSaveFile();
						curretBufString_test = "";*/
						if (countVer < 2)
						{
							countVer++;
							TestVERBegin = TRUE;
							OnBnClickedButtonShort_fun(23);   //<--VER	
						}
						else
						{
							m_cEdit_value_use_NG += "VER	      NG\r\n";
							GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
							TestVERBegin = FALSE;
							OnBnClickedButtonReceiveSaveFile();
							curretBufString_test = "";
						}
					}
					else if (strTestProject.Find("T966") != -1)
					{
						/*m_cEdit_value_use_NG += "VER	      NG\r\n";
						GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
						TestVERBegin = FALSE;
						OnBnClickedButtonReceiveSaveFile();
						curretBufString_test = "";*/
						if (countVer < 2)
						{
							countVer++;
							TestVERBegin = TRUE;
							OnBnClickedButtonShort_fun(23);   //<--VER	
						}
						else
						{
							m_cEdit_value_use_NG += "VER	      NG\r\n";
							GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
							TestVERBegin = FALSE;
							OnBnClickedButtonReceiveSaveFile();
							curretBufString_test = "";
						}
					}
				}
			}
			else if (CheckData.GetLength() > 24)
			{
				CheckData = "";
			}
			curretBufString_test = "";
			sVersion.Empty();
		}
	}
	if((curretBufString.Find("55B207") != -1)&&(curretBufString.Find("46FE") != -1))  //55 B2 07 01 01 01 01 01 01 01 40 FE
	{   //55 B2 07 01 00 00 00 00 00 00 46 FE
		CString m_KeyPadMenu = curretBufString.Mid(6,2);
		mKeyPadMenu = strtol(m_KeyPadMenu,NULL,16);
		//55 B2 07 00 01 00 00 00 00 00 46 FE
		CString m_KeyPadSource = curretBufString.Mid(8,2);
		 mKeypadSource = strtol(m_KeyPadSource,NULL,16);
		//55 B2 07 00 00 01 00 00 00 00 46 FE
		CString m_KeyPadLeft = curretBufString.Mid(10,2);
		 mKeyPadLeft = strtol(m_KeyPadLeft,NULL,16);
		//55 B2 07 00 00 00 01 00 00 00 46 FE
		CString m_KeyPadRight = curretBufString.Mid(12,2);
		 mKeyPadRight = strtol(m_KeyPadRight,NULL,16);
		//55 B2 07 00 00 00 00 01 00 00 46 FE 	//ch+
		CString m_KeyPadDown = curretBufString.Mid(14,2);
		mKeyPadDown = strtol(m_KeyPadDown,NULL,16);
		//55 B2 07 00 00 00 00 00 01 00 46 FE  //ch-
		CString m_KeyPadUp = curretBufString.Mid(16,2);
		mKeyPadUp = strtol(m_KeyPadUp,NULL,16);
		//55 B2 07 00 00 00 00 00 00 01 46 FE 	//power
		CString m_KeyPadPower = curretBufString.Mid(18,2);
		mKeyPadPower = strtol(m_KeyPadPower,NULL,16);
		if (bisOnekey)
		{
			if (mKeyPadMenu > 0)
			{
				bTestKeyPad_Menu = TRUE;
				m_cMfcButton_POWER.EnableWindow(TRUE);
				m_cMfcButton_POWER.SetFaceColor(RGB(0, 255, 0));
			}
		}
		else {
			if (bisSevenkey)
			{
				if (mKeyPadPower > 0)
				{
					bTestKeyPad_Power = TRUE;
					m_cMfcButton_POWER.EnableWindow(TRUE);
					m_cMfcButton_POWER.SetFaceColor(RGB(0, 255, 0));
				}
				if (mKeypadSource > 0)
				{
					bTestKeyPad_Source = TRUE;
					m_cMfcButton_SOURCE.EnableWindow(TRUE);
					m_cMfcButton_SOURCE.SetFaceColor(RGB(0, 255, 0));
				}
				if (mKeyPadLeft > 0)
				{

					if (strTestProject.Find("T962") != -1)
					{
						bTestKeyPad_VolPlus = TRUE;
						m_cMfcButton_VOLP.EnableWindow(TRUE);
						m_cMfcButton_VOLP.SetFaceColor(RGB(0, 255, 0));
						/*bTestKeyPad_VolMins = TRUE;
						GetDlgItem(IDC_MFCBUTTON_VOLM)->EnableWindow(TRUE);
						m_cMfcButton_VOLM.SetFaceColor(RGB(0, 255, 0));*/
					}
					else if (strTestProject.Find("T968") != -1)
					{
						bTestKeyPad_VolPlus = TRUE;
						m_cMfcButton_VOLP.EnableWindow(TRUE);
						m_cMfcButton_VOLP.SetFaceColor(RGB(0, 255, 0));
						/*bTestKeyPad_VolMins = TRUE;
						GetDlgItem(IDC_MFCBUTTON_VOLM)->EnableWindow(TRUE);
						m_cMfcButton_VOLM.SetFaceColor(RGB(0, 255, 0));*/
					}
					else if (strTestProject.Find("V510") != -1)
					{
						bTestKeyPad_VolMins = TRUE;
						GetDlgItem(IDC_MFCBUTTON_VOLM)->EnableWindow(TRUE);
						m_cMfcButton_VOLM.SetFaceColor(RGB(0, 255, 0));
					}
					else if (strTestProject.Find("T966") != -1)
					{
						bTestKeyPad_VolPlus = TRUE;
						m_cMfcButton_VOLP.EnableWindow(TRUE);
						m_cMfcButton_VOLP.SetFaceColor(RGB(0, 255, 0));
					}
				}
				if (mKeyPadRight > 0)
				{
					if (strTestProject.Find("T962") != -1)
					{
						bTestKeyPad_VolMins = TRUE;
						GetDlgItem(IDC_MFCBUTTON_VOLM)->EnableWindow(TRUE);
						m_cMfcButton_VOLM.SetFaceColor(RGB(0, 255, 0));
						/*bTestKeyPad_VolPlus = TRUE;
						m_cMfcButton_VOLP.EnableWindow(TRUE);
						m_cMfcButton_VOLP.SetFaceColor(RGB(0, 255, 0));*/
					}
					else if (strTestProject.Find("T968") != -1)
					{
						bTestKeyPad_VolMins = TRUE;
						GetDlgItem(IDC_MFCBUTTON_VOLM)->EnableWindow(TRUE);
						m_cMfcButton_VOLM.SetFaceColor(RGB(0, 255, 0));
						/*bTestKeyPad_VolPlus = TRUE;
						m_cMfcButton_VOLP.EnableWindow(TRUE);
						m_cMfcButton_VOLP.SetFaceColor(RGB(0, 255, 0));*/
					}
					else if (strTestProject.Find("V510") != -1)
					{

						bTestKeyPad_VolPlus = TRUE;
						m_cMfcButton_VOLP.EnableWindow(TRUE);
						m_cMfcButton_VOLP.SetFaceColor(RGB(0, 255, 0));
					}
					else if (strTestProject.Find("T966") != -1)
					{
						bTestKeyPad_VolMins = TRUE;
						GetDlgItem(IDC_MFCBUTTON_VOLM)->EnableWindow(TRUE);
						m_cMfcButton_VOLM.SetFaceColor(RGB(0, 255, 0));
					}
				}
				if (mKeyPadUp > 0)
				{
					bTestKeyPad_CHmins = TRUE;
					m_cMfcButton_CHM.EnableWindow(TRUE);
					m_cMfcButton_CHM.SetFaceColor(RGB(0, 255, 0));
				}
				if (mKeyPadDown > 0)
				{
					bTestKeyPad_CHPlus = TRUE;
					m_cMfcButton_CHP.EnableWindow(TRUE);
					m_cMfcButton_CHP.SetFaceColor(RGB(0, 255, 0));
				}
				if (mKeyPadMenu > 0)
				{
					bTestKeyPad_Menu = TRUE;
					m_cMfcButton_MENU.EnableWindow(TRUE);
					m_cMfcButton_MENU.SetFaceColor(RGB(0, 255, 0));
				}
			}
			else
			{
				if (mKeyPadLeft > 0)
				{

					if (strTestProject.Find("T962") != -1)
					{
						bTestKeyPad_VolPlus = TRUE;
						m_cMfcButton_VOLP.EnableWindow(TRUE);
						m_cMfcButton_VOLP.SetFaceColor(RGB(0, 255, 0));
						/*bTestKeyPad_VolMins = TRUE;
						GetDlgItem(IDC_MFCBUTTON_VOLM)->EnableWindow(TRUE);
						m_cMfcButton_VOLM.SetFaceColor(RGB(0, 255, 0));*/
					}
					else if (strTestProject.Find("T968") != -1)
					{
						bTestKeyPad_VolPlus = TRUE;
						m_cMfcButton_VOLP.EnableWindow(TRUE);
						m_cMfcButton_VOLP.SetFaceColor(RGB(0, 255, 0));
						/*bTestKeyPad_VolMins = TRUE;
						GetDlgItem(IDC_MFCBUTTON_VOLM)->EnableWindow(TRUE);
						m_cMfcButton_VOLM.SetFaceColor(RGB(0, 255, 0));*/
					}
					else if (strTestProject.Find("V510") != -1)
					{
						bTestKeyPad_VolMins = TRUE;
						GetDlgItem(IDC_MFCBUTTON_VOLM)->EnableWindow(TRUE);
						m_cMfcButton_VOLM.SetFaceColor(RGB(0, 255, 0));
					}
					else if (strTestProject.Find("T966") != -1)
					{
						bTestKeyPad_VolPlus = TRUE;
						m_cMfcButton_VOLP.EnableWindow(TRUE);
						m_cMfcButton_VOLP.SetFaceColor(RGB(0, 255, 0));
					}
				}
				if (mKeyPadRight > 0)
				{
					if (strTestProject.Find("T962") != -1)
					{
						bTestKeyPad_VolMins = TRUE;
						GetDlgItem(IDC_MFCBUTTON_VOLM)->EnableWindow(TRUE);
						m_cMfcButton_VOLM.SetFaceColor(RGB(0, 255, 0));
						/*bTestKeyPad_VolPlus = TRUE;
						m_cMfcButton_VOLP.EnableWindow(TRUE);
						m_cMfcButton_VOLP.SetFaceColor(RGB(0, 255, 0));*/
					}
					else if (strTestProject.Find("T968") != -1)
					{
						bTestKeyPad_VolMins = TRUE;
						GetDlgItem(IDC_MFCBUTTON_VOLM)->EnableWindow(TRUE);
						m_cMfcButton_VOLM.SetFaceColor(RGB(0, 255, 0));
						/*bTestKeyPad_VolPlus = TRUE;
						m_cMfcButton_VOLP.EnableWindow(TRUE);
						m_cMfcButton_VOLP.SetFaceColor(RGB(0, 255, 0));*/
					}
					else if (strTestProject.Find("V510") != -1)
					{

						bTestKeyPad_VolPlus = TRUE;
						m_cMfcButton_VOLP.EnableWindow(TRUE);
						m_cMfcButton_VOLP.SetFaceColor(RGB(0, 255, 0));
					}
					else if (strTestProject.Find("T966") != -1)
					{
						bTestKeyPad_VolMins = TRUE;
						GetDlgItem(IDC_MFCBUTTON_VOLM)->EnableWindow(TRUE);
						m_cMfcButton_VOLM.SetFaceColor(RGB(0, 255, 0));
					}
				}
				if (mKeyPadUp > 0)
				{
					bTestKeyPad_CHmins = TRUE;
					m_cMfcButton_CHM.EnableWindow(TRUE);
					m_cMfcButton_CHM.SetFaceColor(RGB(0, 255, 0));
				}
				if (mKeyPadDown > 0)
				{
					bTestKeyPad_CHPlus = TRUE;
					m_cMfcButton_CHP.EnableWindow(TRUE);
					m_cMfcButton_CHP.SetFaceColor(RGB(0, 255, 0));
				}
				if (mKeyPadMenu > 0)
				{
					bTestKeyPad_Power = TRUE;
					m_cMfcButton_POWER.EnableWindow(TRUE);
					m_cMfcButton_POWER.SetFaceColor(RGB(0, 255, 0));
				}
			}
		}
		curretBufString_test = "";
	} //<--
	if ((curretBufString.Find("55B206")) != -1 )  //MAC
	{
		if (TestMACSend)
		//if(1) //测试用
		{	
			CheckData += curretBufString_test;
			if (CheckData.GetLength() == 24)
			{
				curretBufString_test = CheckData;
				ReceiveSaveMacSuccessFile(CheckData);
				CString m_Mac = curretBufString.Mid(6, 12);
				m_Mac = m_Mac.MakeLower();
				if (m_Mac.GetLength() == 12) //Mac长度检测,保证获取的Mac长度为12位
				{
					CString stemp;
					int k;
					for (int i = 0; i < 7; i++)
					{
						k = i * 2;
						stemp = m_Mac.Mid(k, 2);
						if (stemp.IsEmpty())
						{
							break;
						}
						if (i != 5)
						{
							stemp.Append(":");
						}
						stMac += stemp;
						stemp.Empty();
					}
					GetMacData = stMac;
					if (GetMacData.Find(MacCheck) != -1)
					{
						TestMACSend = FALSE;
						Sleep(100);
						WriteResultMES(GetMacData,1);
						stMac.Empty();
					}
					else
					{
						ShowMessage(RED, _T("MAC校验错误,请检测."), 0);//Mac校验错误;
					}
				}
				else
				{
					if (countMac < 1)
					{
						//OnBnClickedButtonReceiveSaveFile();
						countMac++;
						//TestMACSend = TRUE;
						OnBnClickedButtonShort_fun(37);    //<--MAC
						curretBufString_test = "";
					}
					else
					{
						m_cEdit_value_use_NG += "MAC	      NG\r\n";
						GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
						//TestMACSend = FALSE;
						OnBnClickedButtonReceiveSaveFile();
						curretBufString_test = "";
					}
				}
				GetMacData.Empty();
				CheckData = "";
			}
			else if(CheckData.GetLength() < 24)
			{
				ReceiveSaveMacErrorFile(CheckData);
				CheckData = "";
				curretBufString_test = "";
				if (countMac < 1)
				{
					//OnBnClickedButtonReceiveSaveFile();
					countMac++;
					//TestMACSend = TRUE;
					OnBnClickedButtonShort_fun(37);    //<--MAC
					curretBufString_test = "";
				}
				else
				{
					m_cEdit_value_use_NG += "MAC	      NG\r\n";
					GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
					//TestMACSend = FALSE;
					OnBnClickedButtonReceiveSaveFile();
					curretBufString_test = "";
				}
			}
			else
			{
				//Mac Error.Mac > 24
				ReceiveSaveMacErrorFile(CheckData);
				CheckData = "";
				curretBufString_test = "";
				 if (countMac < 2)
				 {
				 	countMac++;
				 	//TestMACSend = TRUE;
				 	OnBnClickedButtonShort_fun(37);    //<--MAC
				 	curretBufString_test = "";
				 }
				 else
				 {
				 	m_cEdit_value_use_NG += "MAC	      NG\r\n";
				 	GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
				 	//TestMACSend = FALSE;
				 	OnBnClickedButtonReceiveSaveFile();
				 	curretBufString_test = "";
				 }
			}
		}
		curretBufString_test = "";
	}
	if (curretBufString.Find("MTC_UART_TEST") == -1)
	{
		bFactoryTestOk = TRUE;

		//serialReadTemp
		 
	  	 if (curretBufString.Find("55B202010100000000004AFE") != -1)
		{
			if (TestATVbegin)
			{
				SourceConfig[iCurretIndex].bhaveSignal = TRUE;
				m_cEdit_value_use_OK += "ATV1	      PASS\r\n";
				GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
				TestATVbegin = FALSE;
			}
		}
		 if (curretBufString.Find("55B202010000000000004BFE") != -1)
		{
			if(TestATVbegin)
			{
				SourceConfig[iCurretIndex].bhaveSignal = FALSE;
				m_cEdit_value_use_NG += "ATV1	      NG\r\n";
				GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
				TestATVbegin = FALSE;
			}		
		}	
		 if (curretBufString.Find("55B2020201000000000049FE") != -1) 
		 {
			 if (TestDTVbegin)
			 {
				 SourceConfig[iCurretIndex].bhaveSignal = TRUE;
				 m_cEdit_value_use_OK += "DTV1	      PASS\r\n";
				 GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
				 TestDTVbegin = FALSE;
			 }
		 }
		 if (curretBufString.Find("55B202020000000000004AFE") != -1)
		 {
			 if (TestDTVbegin)
			 {
				 SourceConfig[iCurretIndex].bhaveSignal = FALSE;
				 m_cEdit_value_use_NG += "DTV1	      NG\r\n";
				 GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
				 TestDTVbegin = FALSE;
			 }
		 }
		 if (curretBufString.Find("55B2020301000000000048FE") != -1)
		{
			if(TestAVbegin)
			{
				SourceConfig[iCurretIndex].bhaveSignal = TRUE;
				m_cEdit_value_use_OK += "AV1	      PASS\r\n";
				GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
				TestAVbegin = FALSE;
			}
		}
		 if (curretBufString.Find("55B2020300000000000049FE") != -1)
		{
			if(TestAVbegin)
			{
				SourceConfig[iCurretIndex].bhaveSignal = FALSE;
				m_cEdit_value_use_NG += "AV1	        NG\r\n";
				GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
				TestAVbegin = FALSE;
			}
		}
		 if (curretBufString.Find("55B2020701000000000044FE") != -1)
		{
			if(TestHDMI1begin)
			{
				SourceConfig[iCurretIndex].bhaveSignal = TRUE;
				m_cEdit_value_use_OK += "HDMI1	      PASS\r\n";
				GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
				TestHDMI1begin = FALSE;
			}
		}
	 	 if (curretBufString.Find("55B2020700000000000045FE") != -1)
		{
			if(TestHDMI1begin)
			{
				SourceConfig[iCurretIndex].bhaveSignal = FALSE;
				m_cEdit_value_use_NG += "HDMI1	      NG\r\n";
				GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
				TestHDMI1begin = FALSE;
			}
		}
		 if (curretBufString.Find("55B2020801000000000043FE") != -1)
		{
			if(TestHDMI2begin)
			{
				SourceConfig[iCurretIndex].bhaveSignal = TRUE;
				m_cEdit_value_use_OK += "HDMI2	      PASS\r\n";
				GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
				TestHDMI2begin = FALSE;
			}
		}
		 if (curretBufString.Find("55B2020800000000000044FE") != -1)
		{
			if(TestHDMI2begin)
			{
				SourceConfig[iCurretIndex].bhaveSignal = FALSE;
				m_cEdit_value_use_NG += "HDMI2	      NG\r\n";
				GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
				TestHDMI2begin = FALSE;
			}
		}
		 //<--
		if (curretBufString.Find("55B201") != -1)  //WIFI_BLUE_NET
		 {
			 if (TestWifiSend)
			 {
				 CString m_Wifi = curretBufString.Mid(6,2);
				 int mWifi = strtol(m_Wifi,NULL,16);  //<--WiFi返回个数
				 if (mWifi > 0)
				 {
					 m_cEdit_value_use_OK += "WIFI	      PASS\r\n";
					 GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
					 TestWifiSend = FALSE;
					 curretBufString_test = "";
				 }
				 else
				 {
					 if (countWifi < 1)
					 {
						 countWifi++;
						 TestWifiSend = TRUE;
						 curretBufString_test = "";
						 OnBnClickedButtonShort_fun(38);    //<--WIFI
					 }
					 else
					 {
						 m_cEdit_value_use_NG += "WIFI	      NG\r\n";
						 GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
						 TestWifiSend = FALSE;
						 OnBnClickedButtonReceiveSaveFile();
						 curretBufString_test = "";
					 }	 
				 }
			 }
			 if (TestBlueSend)
			 {
				 CString m_Blue = curretBufString.Mid(6,2);
				 int mBlue = strtol(m_Blue,NULL,16);  //<--蓝色返回个数
				 if (mBlue > 0)
				 {
					 m_cEdit_value_use_OK += "BLUE	      PASS\r\n";
					 GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
					 TestBlueSend = FALSE;
					 curretBufString_test = "";
				 }
				 else
				 {
					 if (countBlue < 1)
					 {
						 countBlue++;
						 TestBlueSend = TRUE;
						 curretBufString_test = "";
						 OnBnClickedButtonShort_fun(39);		//<--蓝牙
					 }
					 else
					 {
						 m_cEdit_value_use_NG += "BLUE	      NG\r\n";
						 GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
						 TestBlueSend = FALSE;
						 OnBnClickedButtonReceiveSaveFile();
						 curretBufString_test = "";
					 }
				 }
			 }
		 }
		if (curretBufString.Find("55B201") != -1)
		 {
			 if (TestNetSend)   //检测网口--ping IP地址
			 {
				 CString m_Wifi = curretBufString.Mid(6,2);
				 int mWifi = strtol(m_Wifi,NULL,16);  //<--
				 if (mWifi == 1)
				 {
					 m_cEdit_value_use_OK += "网口	      PASS\r\n";
					 GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
					 TestNetSend = FALSE;
					 curretBufString_test = "";
				 }
				 else
				 {
					 if (countNet < 1)
					 {
						 countNet++;
						 TestNetSend = TRUE;
						 Sleep(300);
						 curretBufString_test = "";
						 OnBnClickedButtonShort_fun(36);			//<--NET
					 }
					 else
					 {
						 m_cEdit_value_use_NG += "网口	        NG\r\n";
						 GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
						 TestNetSend = FALSE;
						 OnBnClickedButtonReceiveSaveFile();
						 curretBufString_test = "";
					 }
				 }
			 }
		 }
		if (curretBufString.Find("55B206") != -1)   // USB+SD卡               T968方案USB端口测试码值回复错误
		 {
				if (TestUsbSend)
				{
					if (strTestProject.Find("T962") != -1)
					
					{
						CString m_Usb1 = curretBufString.Mid(8,2);							//55 B2 06 03  00  02 00 01 00 00 42FE
						CString m_Usb2 = curretBufString.Mid(10,2);
						CString m_Sd =  curretBufString.Mid(16,2);
						int mUsb1 = strtol(m_Usb1,NULL,16);   //上方USB
						int mUsb2 = strtol(m_Usb2,NULL,16);   //下方USB
						int mSd = strtol(m_Sd,NULL,16); //SD卡
						if((mUsb1 > 0)&&(mUsb2 > 0))
						{
							m_cEdit_value_use_OK+= "USB	      PASS\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
						}
						else if (mUsb1 == 0 && mUsb2 == 1)
							{
								m_cEdit_value_use_NG += "USB1	        NG\r\n";
								Sleep(500);
								GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
								m_cEdit_value_use_OK += "USB2	      PASS\r\n";
								Sleep(500);
								GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
							}
						else if (mUsb1 == 1 && mUsb2 == 0 )
								{
									m_cEdit_value_use_NG += "USB2	        NG\r\n";
									Sleep(500);
									GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
									m_cEdit_value_use_OK += "USB1	      PASS\r\n";
									Sleep(500);
									GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
								}
						else if (mUsb1 == 0 && mUsb2 == 0)
								{
									m_cEdit_value_use_NG += "USB	        NG\r\n";
									Sleep(500);
									GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
								}
						if (mSd == 1)
						{
							m_cEdit_value_use_OK += "SD卡	      PASS\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
						}
						else
						{
							m_cEdit_value_use_NG += "SD卡	        NG\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
						}
					}
					else if (strTestProject.Find("T968") != -1)

					{
						CString m_Usb1 = curretBufString.Mid(8, 2);							//55 B2 06 03  00  02 00 01 00 00 42FE
						CString m_Usb2 = curretBufString.Mid(10, 2);
						CString m_Sd = curretBufString.Mid(16, 2);
						int mUsb1 = strtol(m_Usb1, NULL, 16);   //上方USB
						int mUsb2 = strtol(m_Usb2, NULL, 16);   //下方USB
						int mSd = strtol(m_Sd, NULL, 16); //SD卡
						if ((mUsb1 > 0) && (mUsb2 > 0))
						{
							m_cEdit_value_use_OK += "USB	      PASS\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
						}
						else if (mUsb1 == 0 && mUsb2 == 1)
						{
							m_cEdit_value_use_NG += "USB1	        NG\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
							m_cEdit_value_use_OK += "USB2	      PASS\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
						}
						else if (mUsb1 == 1 && mUsb2 == 0)
						{
							m_cEdit_value_use_NG += "USB2	        NG\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
							m_cEdit_value_use_OK += "USB1	      PASS\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
						}
						else if (mUsb1 == 0 && mUsb2 == 0)
						{
							m_cEdit_value_use_NG += "USB	        NG\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
						}
						if (mSd == 1)
						{
							m_cEdit_value_use_OK += "SD卡	      PASS\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
						}
						else
						{
							m_cEdit_value_use_NG += "SD卡	        NG\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
						}
					}
					else if (strTestProject.Find("T966") != -1)
					{
						CString m_Usb3 = curretBufString.Mid(8, 2);							//55 B2 06 03  00  02 00 01 00 00 42FE
						CString m_Usb2 = curretBufString.Mid(10, 2);
						CString m_Usb1 = curretBufString.Mid(12, 2);
						int mUsb3 = strtol(m_Usb3, NULL, 16);   //USB3
						int mUsb2 = strtol(m_Usb2, NULL, 16);   //USB2
						int mUsb1 = strtol(m_Usb1, NULL, 16);   //USB1
						     if ((mUsb1 == 1) && (mUsb2 == 1) && (mUsb3 == 1))
						{
							m_cEdit_value_use_OK += "USB	      PASS\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
						}  //123OK
						else if (mUsb1 == 0 && mUsb2 == 0 && mUsb3 == 0)
						{
							m_cEdit_value_use_NG += "USB	        NG\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
						} //123 NG
						else if (mUsb1 == 1 && mUsb2 == 1 && mUsb3 ==0)
						{
							m_cEdit_value_use_NG += "USB3	        NG\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
							m_cEdit_value_use_OK += "USB1,2	      PASS\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
						} //12OK 3NG
						else if (mUsb1 == 1 && mUsb2 == 0 && mUsb3 == 1)
						{
							m_cEdit_value_use_NG += "USB2	        NG\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
							m_cEdit_value_use_OK += "USB1,3	      PASS\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
						}//13OK,2NG
						else if (mUsb1 == 0 && mUsb2 == 1 && mUsb3 ==1)
						{
							m_cEdit_value_use_NG += "USB1	        NG\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
							m_cEdit_value_use_OK += "USB2,3	      PASS\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
						}//23OK,1NG
						else if (mUsb1 == 0 && mUsb2 == 0 && mUsb3 == 1)
						{
							m_cEdit_value_use_NG += "USB1,2	        NG\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
							m_cEdit_value_use_OK += "USB3	      PASS\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
						}//3OK,12NG
						else if (mUsb1 == 1 && mUsb2 == 0 && mUsb3 == 0)
						{
							m_cEdit_value_use_NG += "USB2,3        NG\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
							m_cEdit_value_use_OK += "USB1	      PASS\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
						}//1OK,23NG
						else if (mUsb1 == 0 && mUsb2 == 1 && mUsb3 == 0)
						{
							m_cEdit_value_use_NG += "USB1,3        NG\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
							m_cEdit_value_use_OK += "USB2	      PASS\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
						}//2OK,13NG
						/*else if (mUsb1 == 0 && mUsb2 == 0 && mUsb3 == 1)
						{
							m_cEdit_value_use_NG += "USB1,2	        NG\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
							m_cEdit_value_use_OK += "USB3	      PASS\r\n";
							Sleep(500);
							GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
						}*/
						/*else if*/
					}
					else if (strTestProject.Find("V510") != -1)
					{
						MessageBox("请注意,V510方案无法检测USB!");
						//CString m_Usb1 = curretBufString.Mid(8, 2);							//55 B2 06 03  00  02 00 01 00 00 42FE
						//CString m_Usb2 = curretBufString.Mid(10, 2);
						//CString m_Sd = curretBufString.Mid(16, 2);
						//int mUsb1 = strtol(m_Usb1, NULL, 16);   //上方USB
						//int mUsb2 = strtol(m_Usb2, NULL, 16);   //下方USB
						//int mSd = strtol(m_Sd, NULL, 16); //SD卡
						//if ((mUsb1 > 0) && (mUsb2 > 0))
						//{
						//	m_cEdit_value_use_OK += "USB/SD	      PASS\r\n";
						//	GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
						//}
						//else if (mUsb1 == 0 && mUsb2 == 1)
						//{
						//	m_cEdit_value_use_NG += "USB/SD	        NG\r\n";
						//	GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
						//}
					}
					TestUsbSend = FALSE;	
				}
		}
		 
		//<--V510
		if (curretBufString.Find("55B201010100000000004BFE") != -1)
		{
			if (TestATVbegin)
			{
				SourceConfig[iCurretIndex].bhaveSignal = TRUE;
				m_cEdit_value_use_OK += "ATV1	      PASS\r\n";
				GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
				TestATVbegin = FALSE;
			}
		}
		else if (curretBufString.Find("55B201010000000000004CFE") != -1)
		{
			if(TestATVbegin)
			{
				SourceConfig[iCurretIndex].bhaveSignal = FALSE;
				m_cEdit_value_use_NG += "ATV1	      NG\r\n";
				GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
				TestATVbegin = FALSE;
			}		
		}	
		else if (curretBufString.Find("55B201020100000000004AFE") != -1)
		 {
			 if (TestDTVbegin)
			 {
				 SourceConfig[iCurretIndex].bhaveSignal = TRUE;
				 m_cEdit_value_use_OK += "DTV1	      PASS\r\n";
				 GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
				 TestDTVbegin = FALSE;
			 }
		 }
		else if (curretBufString.Find("55B201020000000000004BFE") != -1)
		 {
			 if (TestDTVbegin)
			 {
				 SourceConfig[iCurretIndex].bhaveSignal = FALSE;
				 m_cEdit_value_use_NG += "DTV1	      NG\r\n";
				 GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
				 TestDTVbegin = FALSE;
			 }
		 }
		else if (curretBufString.Find("55B2010301000000000049FE") != -1)
		 {
			 if(TestAVbegin)
			 {
				 SourceConfig[iCurretIndex].bhaveSignal = TRUE;
				 m_cEdit_value_use_OK += "AV1	      PASS\r\n";
				 GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
				 TestAVbegin = FALSE;
			 }
		 }
		else if (curretBufString.Find("55B201030000000000004AFE") != -1)
		 {
			 if(TestAVbegin)
			 {
				 SourceConfig[iCurretIndex].bhaveSignal = FALSE;
				 m_cEdit_value_use_NG += "AV1	      NG\r\n";
				 GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
				 TestAVbegin = FALSE;
			 }
		 }
		else if (curretBufString.Find("55B2010501000000000047FE") != -1)
		{
			if (TestYPBPRbegin)
			{
				SourceConfig[iCurretIndex].bhaveSignal = TRUE;
				m_cEdit_value_use_OK += "YPBPR	      PASS\r\n";
				GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
				TestYPBPRbegin = FALSE;
			}
		}
		else if (curretBufString.Find("55B2010500000000000048FE") != -1)
		{
			if (TestYPBPRbegin)
			{
				SourceConfig[iCurretIndex].bhaveSignal = FALSE;
				m_cEdit_value_use_NG += "YPBPR	      NG\r\n";
				GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
				TestYPBPRbegin = FALSE;
			}
		}
		else if (curretBufString.Find("55B2010701000000000045FE") != -1)
		 {
			 if(TestHDMI1begin)
			 {
				 SourceConfig[iCurretIndex].bhaveSignal = TRUE;
				 m_cEdit_value_use_OK += "HDMI1	      PASS\r\n";
				 GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
				 TestHDMI1begin = FALSE;
			 }
		 }
		else if (curretBufString.Find("55B2010700000000000046FE") != -1)
		 {
			 if(TestHDMI1begin)
			 {
				 SourceConfig[iCurretIndex].bhaveSignal = FALSE;
				 m_cEdit_value_use_NG += "HDMI1	      NG\r\n";
				 GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
				 TestHDMI1begin = FALSE;
			 }
		 }
		else if (curretBufString.Find("55B2010801000000000044FE") != -1)
		 {
			 if(TestHDMI2begin)
			 {
				 SourceConfig[iCurretIndex].bhaveSignal = TRUE;
				 m_cEdit_value_use_OK += "HDMI2	      PASS\r\n";
				 GetDlgItem(IDC_EDIT_OK)->SetWindowText(m_cEdit_value_use_OK);
				 TestHDMI2begin = FALSE;
			 }
		 }
		else if (curretBufString.Find("55B2010800000000000045FE") != -1)
		 {
			 if(TestHDMI2begin)
			 {
				 SourceConfig[iCurretIndex].bhaveSignal = FALSE;
				 m_cEdit_value_use_NG += "HDMI2	      NG\r\n";
				 GetDlgItem(IDC_EDIT_FAILED)->SetWindowText(m_cEdit_value_use_NG);
				 TestHDMI2begin = FALSE;
			 }
		 }
		else
		{
			//
		}
		curretBufString_test = "";
	}
	curretBufString_test = "";
}
void CserialCommunicationDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CString strTemp;
	CString strShowMessage("");
	if(TIME_SHOW_MESSAGE_SERIAL == nIDEvent)
	{
		if(serial_isopen == TRUE)
		{
			//串口打开,检查串口状态
			if(sio_lstatus(com_port) >= 0)
			{
				//串口OK,读取串口数据
				SerialsProcessBuffer();
				if(serialReadTemp.GetLength() > 0)
				{
					//if(bStartCheckData)
					curretBufString_test.Append(serialReadTemp);
					showString.Append(serialReadTemp);
					showString.Append("\r\n");
					GetDlgItem(IDC_EDIT_RECEIVE)->SetWindowText(showString);
					int line = ((CEdit*)(GetDlgItem(IDC_EDIT_RECEIVE)))->GetLineCount();//选中最后一行
					((CEdit*)(GetDlgItem(IDC_EDIT_RECEIVE)))->LineScroll(line);
					int line_OK = ((CEdit*)(GetDlgItem(IDC_EDIT_OK)))->GetLineCount();//选中最后一行
					((CEdit*)(GetDlgItem(IDC_EDIT_OK)))->LineScroll(line_OK);
					int line_NG = ((CEdit*)(GetDlgItem(IDC_EDIT_FAILED)))->GetLineCount();//选中最后一行
					((CEdit*)(GetDlgItem(IDC_EDIT_FAILED)))->LineScroll(line_NG);
					SerialsCheckBuffer(curretBufString_test);
				}
			}
			else
			{
				//串口丢失,暂时不管			
			}
		}
	}
	else if (TIME_SHOW_MESSAGE_TEST_DELAY_OTHER == nIDEvent)  
	{
		if (cCheckOtherCount_tmp == 0)
		{
			cCheckOtherCount_tmp = 1;
			OnBnClickedButtonShort_fun(23);   //<--VER
			Sleep(100);
			TestVERBegin = TRUE;
		}
		else if (cCheckOtherCount_tmp == 1)				
		{
			if(colorTmpconfigFileLoadOk)   //配置色温
			{	
				cCheckOtherCount_tmp = 2;
				OnBnClickedButtonShort_fun(33);  //<--写色温normal
				Sleep(200);
				GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText("写色温Noraml");
			}
			else
				cCheckOtherCount_tmp = 14;
		}
		else if (cCheckOtherCount_tmp == 2)
		{
			cCheckOtherCount_tmp = 3;
			bFacWriteWB_Normal_Red = TRUE;
			OnSendSetColorTmp(1, m_stColorTmp[0].RDrv);  //<--normal_red
			Sleep(200);
		}
		else if (cCheckOtherCount_tmp == 3)
		{
			cCheckOtherCount_tmp = 4;
			bFacWriteWB_Normal_Green = TRUE;
			OnSendSetColorTmp(2, m_stColorTmp[0].GDrv);  //<--normal_green
			Sleep(200);
		}
		else if (cCheckOtherCount_tmp == 4)
		{
			cCheckOtherCount_tmp = 5;
			bFacWriteWB_Normal_Blue = TRUE;
			OnSendSetColorTmp(3, m_stColorTmp[0].BDrv);  //<--normal_blue	
			//Sleep(100);	
		}
		else if (cCheckOtherCount_tmp == 5)
		{
			cCheckOtherCount_tmp = 6;
			OnBnClickedButtonShort_fun(34);  //<--写色温cool
			GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText("写色温Cool");
		}
		else if (cCheckOtherCount_tmp == 6)
		{
			cCheckOtherCount_tmp = 7;
			bFacWriteWB_Cool_Red = TRUE;
			OnSendSetColorTmp(1, m_stColorTmp[1].RDrv);  //<--cool_red
		}
		else if (cCheckOtherCount_tmp == 7)
		{
			cCheckOtherCount_tmp = 8;
			bFacWriteWB_Cool_Green = TRUE;
			OnSendSetColorTmp(2, m_stColorTmp[1].GDrv);  //<--cool_green
		}
		else if (cCheckOtherCount_tmp == 8)
		{
			cCheckOtherCount_tmp = 9;
			bFacWriteWB_Cool_Blue = TRUE;
			OnSendSetColorTmp(3, m_stColorTmp[1].BDrv);  //<--cool_blue
		}
		else if (cCheckOtherCount_tmp == 9)
		{
			cCheckOtherCount_tmp = 10;
			OnBnClickedButtonShort_fun(35);  //<--写色温Warm
			GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText("写色温Warm");
		}
		else if (cCheckOtherCount_tmp == 10)
		{
			cCheckOtherCount_tmp = 11;
			bFacWriteWB_Warm_Red = TRUE;
			OnSendSetColorTmp(1, m_stColorTmp[2].RDrv);  //<--warm_red
		}
		else if (cCheckOtherCount_tmp == 11)
		{
			cCheckOtherCount_tmp = 12;
			bFacWriteWB_Warm_Green = TRUE;
			OnSendSetColorTmp(2, m_stColorTmp[2].GDrv);  //<--warm_green
		}
		else if (cCheckOtherCount_tmp == 12)
		{
			cCheckOtherCount_tmp = 13;
			bFacWriteWB_Warm_Blue = TRUE;
			OnSendSetColorTmp(3, m_stColorTmp[2].BDrv);  //<--warm_blue
		}
		else if (cCheckOtherCount_tmp == 13)			 
		{
			cCheckOtherCount_tmp = 14;
			Sleep(100);	
			OnBnClickedButtonShort_fun(40);				//<--Save_WhiteBlance(V510方案需要保存白平衡值)
		}
		else if (cCheckOtherCount_tmp == 14)
		{
			cCheckOtherCount_tmp = 15;
			if (TestWifiBegin)
			{
				TestWifiSend = TRUE;
				OnBnClickedButtonShort_fun(38);    //<--WIFI
			}
		}
		else if (cCheckOtherCount_tmp == 15)
		{
			cCheckOtherCount_tmp = 20;
			if (TestAddVoice)
			{
				OnBnClickedButtonShort_fun(31);		//<--声音加至80
				TestAddVoice = FALSE;
			}
		}
		else if (cCheckOtherCount_tmp == 20)	
		{
			cCheckOtherCount_tmp = 21;
			if (TestBlueBegin) 
			{
				TestBlueSend = TRUE;
				OnBnClickedButtonShort_fun(39);		//<--蓝牙
			}
		}
		else if (cCheckOtherCount_tmp == 21)
		{
			cCheckOtherCount_tmp = 22;
			if (TestUsbBegin)
			{
				TestUsbSend = TRUE;
				OnBnClickedButtonShort_fun(9);   //<--USB
			}
		}
		else if (cCheckOtherCount_tmp == 22)
		{
			cCheckOtherCount_tmp = 16;
			if (TestNetBegin)
			{
				TestNetSend = TRUE;
				OnBnClickedButtonShort_fun(36);			//<--NET
			}
		}
		//else if (cCheckOtherCount_tmp == 16)
		//{
		//	cCheckOtherCount_tmp = 17;			//<--预留
		//	//TestMACSend = TRUE;
		//	OnBnClickedButtonShort_fun(37);    //<--MAC
		//}
		else
		{
			KillTimer(TIME_SHOW_MESSAGE_TEST_DELAY_OTHER);
			SetTimer(TIME_SHOW_MESSAGE_TEST_DELAY, 500, NULL); 
		}
	}
	else if (TIME_SHOW_MESSAGE_TEST_DELAY == nIDEvent)
	{
		if ((serial_isopen == TRUE) && bFactoryTestAutoStart)
		{
			//串口打开,检查串口状态
			if (sio_lstatus(com_port) >= 0)
			{
				int i = cCheckCout;
				iTestSourceTimerCount++;
				if(bWaitFeedback)
				{
					if (iTestSourceTimerCount >= 2*SourceConfig[i].btime)
					{
						GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText("取状态...");
						OnBnClickedButtonShort_fun(32);
						//OnBnClickedButtonShort_fun(31);
						//SetTimer(TIME_SHOW_MESSAGE_TEST_DELAY, 500, NULL);
						if (cCheckCout_tmp++ >= 1)
						{
							cCheckCout_tmp = 0;
							bWaitFeedback = FALSE;
							cCheckCout++;
						}
					}
				}
				else
				{
					if (SourceConfig[i].index == 0)//结束测试
					{
						bTestKeyPad_Source = FALSE;
						bTestKeyPad_Menu = FALSE;
						bTestKeyPad_Power = FALSE;
						bTestKeyPad_CHPlus = FALSE;
						bTestKeyPad_CHmins = FALSE;
						bTestKeyPad_VolPlus = FALSE;
						bTestKeyPad_VolMins = FALSE;
						bFactoryTestAutoStart = FALSE;
						//Sleep(100);
						if (strLastSource.Find("ATV1") != -1)
						{

							OnBnClickedButtonShort_fun(6);
						}
						/*else if (strLastSource.Find("ATV2") != -1)
						{
						OnBnClickedButtonShort_fun(7);
						}*/
						else if (strLastSource.Find("DTV1") != -1)
						{
							OnBnClickedButtonShort_fun(2);
						}
						/*else if (strLastSource.Find("DTV2") != -1)
						{
						OnBnClickedButtonShort_fun(3);
						}*/
						/*else if (strLastSource.Find("DTV3") != -1)
						{
						OnBnClickedButtonShort_fun(4);
						}*/
						else if (strLastSource.Find("AV1") != -1)
						{
							OnBnClickedButtonShort_fun(10);
							Sleep(1000);
						}
						/*else if (strLastSource.Find("AV2") != -1)
						{
						OnBnClickedButtonShort_fun(11);
						}*/
						else if (strLastSource.Find("HDMI1") != -1)
						{
							OnBnClickedButtonShort_fun(15);
						}
						else if (strLastSource.Find("HDMI2") != -1)
						{
							OnBnClickedButtonShort_fun(16);
							Sleep(1000);
						}
						/*else if (strLastSource.Find("HDMI3") != -1)
						{
						OnBnClickedButtonShort_fun(17);
						}
						else if (strLastSource.Find("HDMI4") != -1)
						{
						OnBnClickedButtonShort_fun(27);
						}
						else if (strLastSource.Find("SCART") != -1)
						{
						OnBnClickedButtonShort_fun(26);
						}*/
						else if (strLastSource.Find("USB") != -1)
						{
							OnBnClickedButtonShort_fun(9);
						}
						else if (strLastSource.Find("YPBPR") != -1)
						{
							OnBnClickedButtonShort_fun(14);
						}
						/*else if (strLastSource.Find("DVBS") != -1)
						{
						OnBnClickedButtonShort_fun(29);
						}*/
						else if (strLastSource.Find("PC") != -1)
						{
							OnBnClickedButtonShort_fun(13);
						}
						else if (strLastSource.Find("NET") != -1)
						{
							OnBnClickedButtonShort_fun(37);
						}
						//else if (strLastSource.Find("KEYPAD") != -1)
						//{
						//	//OnBnClickedButtonShort_fun(29);
						//}
						else if (TestVoiceBegin)
						{
							m_psdinfo.SetWindowText(("话筒测试开始"));
							OnBnClickedButtonShort_fun(20);   //<--话筒测试
															  //if (TestVoiceBegin)
															  //	{
															  //		m_psdinfo.SetWindowText(("话筒测试开始"));
															  //		OnBnClickedButtonShort_fun(20);   //<--话筒测试
															  //	}
						}
						KillTimer(TIME_SHOW_MESSAGE_TEST_DELAY);
						bKeypadStart = TRUE;
						if (bKeypadStart)  //<--
						{
							//ShowMessage(RED, _T("按键测试开始，请在5秒内操作完"), 0);
							cCheckKEYPADCount_tmp = 0;
							SetTimer(TIME_SHOW_MESSAGE_KEYPAY_TEST_OTHER, 300, NULL);
						}
					}
					if (SourceConfig[i].source.Find("ATV1") != -1)
					{
							TestATVbegin = TRUE;
							GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText("测试ATV1");
							OnBnClickedButtonShort_fun(6);
							bWaitFeedback = TRUE;
							iCurretIndex = i;
						iTestSourceTimerCount = 0;
					}
					//else if (SourceConfig[i].source.Find("ATV2") != -1)
					//{
					//	GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText("测试ATV2");
					//	OnBnClickedButtonShort_fun(7);
					//	bWaitFeedback = TRUE;
					//	iCurretIndex = i;
					//	//KillTimer(TIME_SHOW_MESSAGE_TEST_DELAY);
					//	//SetTimer(TIME_SHOW_MESSAGE_TEST_DELAY, 1000 * SourceConfig[i].btime, NULL);
					//	//Sleep(1000*SourceConfig[i].btime);
					//	//OnBnClickedButtonShort_fun(25);
					//	iTestSourceTimerCount = 0;
					//}
					else if (SourceConfig[i].source.Find("DTV1") != -1)
					{
						TestDTVbegin = TRUE;
						GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText("测试DTV1");
						OnBnClickedButtonShort_fun(2);
						bWaitFeedback = TRUE;
						iCurretIndex = i;
						iTestSourceTimerCount = 0;
					}
					//else if (SourceConfig[i].source.Find("DTV2") != -1)
					//{
					//	GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText("测试DTV2");
					//	OnBnClickedButtonShort_fun(3);
					//	bWaitFeedback = TRUE;
					//	iCurretIndex = i;
					//	//SetTimer(TIME_SHOW_MESSAGE_TEST_DELAY, 1000 * SourceConfig[i].btime, NULL);
					//	//Sleep(1000*SourceConfig[i].btime);
					//	//OnBnClickedButtonShort_fun(25);
					//	iTestSourceTimerCount = 0;
					//}
					//else if (SourceConfig[i].source.Find("DTV3") != -1)
					//{
					//	GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText("测试DTV3");
					//	OnBnClickedButtonShort_fun(4);
					//	bWaitFeedback = TRUE;
					//	iCurretIndex = i;
					//	//SetTimer(TIME_SHOW_MESSAGE_TEST_DELAY, 1000 * SourceConfig[i].btime, NULL);
					//	//Sleep(1000*SourceConfig[i].btime);
					//	//OnBnClickedButtonShort_fun(25);
					//	iTestSourceTimerCount = 0;
					//}
					else if (SourceConfig[i].source.Find("AV1") != -1)
					{
						TestAVbegin = TRUE;
						GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText("测试AV1");
						OnBnClickedButtonShort_fun(10);
						bWaitFeedback = TRUE;
						iCurretIndex = i;
						iTestSourceTimerCount = 0;
					}
					/*else if (SourceConfig[i].source.Find("AV2") != -1)
					{
						GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText("测试AV2");
						OnBnClickedButtonShort_fun(11);
						bWaitFeedback = TRUE;
						iCurretIndex = i;
						iTestSourceTimerCount = 0;
					}*/
					else if (SourceConfig[i].source.Find("YPBPR") != -1)
					{
						TestYPBPRbegin = TRUE;
						GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText("测试YPBPR");
						OnBnClickedButtonShort_fun(14);
						bWaitFeedback = TRUE;
						iCurretIndex = i;
						//SetTimer(TIME_SHOW_MESSAGE_TEST_DELAY, 1000 * SourceConfig[i].btime, NULL);
						//Sleep(1000*SourceConfig[i].btime);
						//OnBnClickedButtonShort_fun(25);
						iTestSourceTimerCount = 0;
					}
					else if (SourceConfig[i].source.Find("HDMI1") != -1)
					{
						TestHDMI1begin = TRUE;
						GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText("测试HDMI1");
						OnBnClickedButtonShort_fun(15);
						bWaitFeedback = TRUE;
						iCurretIndex = i;
						iTestSourceTimerCount = 0;
					}
					else if (SourceConfig[i].source.Find("HDMI2") != -1)
					{
						TestHDMI2begin = TRUE;
						GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText("测试HDMI2");
						OnBnClickedButtonShort_fun(16);
						bWaitFeedback = TRUE;
						iCurretIndex = i;
						iTestSourceTimerCount = 0;
					}
					//else if (SourceConfig[i].source.Find("HDMI3") != -1)
					//{
					//	TestHDMI3begin = TRUE;
					//	GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText("测试HDMI3");
					//	OnBnClickedButtonShort_fun(17);
					//	bWaitFeedback = TRUE;
					//	iCurretIndex = i;
					//	iTestSourceTimerCount = 0;
					//}
					//else if (SourceConfig[i].source.Find("HDMI4") != -1)
					//{
					//	GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText("测试HDMI4");
					//	OnBnClickedButtonShort_fun(27);
					//	bWaitFeedback = TRUE;
					//	iCurretIndex = i;
					//	//SetTimer(TIME_SHOW_MESSAGE_TEST_DELAY, 1000 * SourceConfig[i].btime, NULL);
					//	//Sleep(1000*SourceConfig[i].btime);
					//	//OnBnClickedButtonShort_fun(25);
					//	iTestSourceTimerCount = 0;
					//}
					//else if (SourceConfig[i].source.Find("SCART") != -1)
					//{
					//	GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText("测试SCART");
					//	OnBnClickedButtonShort_fun(26);
					//	bWaitFeedback = TRUE;
					//	iCurretIndex = i;
					//	//SetTimer(TIME_SHOW_MESSAGE_TEST_DELAY, 1000 * SourceConfig[i].btime, NULL);
					//	//Sleep(1000*SourceConfig[i].btime);
					//	//OnBnClickedButtonShort_fun(25);
					//	iTestSourceTimerCount = 0;
					//}
					//else if (SourceConfig[i].source.Find("USB") != -1)
					//{
					//	GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText("测试USB");
					//	OnBnClickedButtonShort_fun(9);
					//	bWaitFeedback = TRUE;
					//	iCurretIndex = i;
					//	//SetTimer(TIME_SHOW_MESSAGE_TEST_DELAY, 1000 * SourceConfig[i].btime, NULL);
					//	//Sleep(1000*SourceConfig[i].btime);
					//	//OnBnClickedButtonShort_fun(25);
					//	iTestSourceTimerCount = 0;
					//}
					//else if (SourceConfig[i].source.Find("DVBS") != -1)
					//{
					//	GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText("测试DVBS");
					//	OnBnClickedButtonShort_fun(29);
					//	bWaitFeedback = TRUE;
					//	iCurretIndex = i;
					//	//SetTimer(TIME_SHOW_MESSAGE_TEST_DELAY, 1000 * SourceConfig[i].btime, NULL);
					//	//Sleep(1000*SourceConfig[i].btime);
					//	//OnBnClickedButtonShort_fun(25);
					//	iTestSourceTimerCount = 0;
					//}
					//else if (SourceConfig[i].source.Find("PC") != -1)
					//{
					//	GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText("测试PC");
					//	OnBnClickedButtonShort_fun(13);
					//	bWaitFeedback = TRUE;
					//	iCurretIndex = i;
					//	//SetTimer(TIME_SHOW_MESSAGE_TEST_DELAY, 1000 * SourceConfig[i].btime, NULL);
					//	//Sleep(1000*SourceConfig[i].btime);
					//	//OnBnClickedButtonShort_fun(25);
					//	iTestSourceTimerCount = 0;
					//}
					//else if (SourceConfig[i].source.Find("NET") != -1)
					//{
					//	GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText("测试网口");
					//	OnBnClickedButtonShort_fun(37);
					//	bWaitFeedback = TRUE;
					//	iCurretIndex = i;
					//	//SetTimer(TIME_SHOW_MESSAGE_TEST_DELAY, 1000 * SourceConfig[i].btime, NULL);
					//	//Sleep(1000*SourceConfig[i].btime);
					//	//OnBnClickedButtonShort_fun(25);
					//	iTestSourceTimerCount = 0;
					//}
					else
					{
						bWaitFeedback = TRUE;
						//if (SourceConfig[i].index == 0)
						KillTimer(TIME_SHOW_MESSAGE_TEST_DELAY);
						//GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
						bFactoryTestAutoStart = FALSE;
						iTestSourceTimerCount = 0;
						if (SourceConfig[i].index != 0)
							MessageBox("无效的配置端子");
					}
				}
			}
			else
			{
				SerialReConnect();
			}
		}
	}
	else if (TIME_SHOW_MESSAGE_KEYPAY_TEST_OTHER == nIDEvent)
	{
		if (cCheckKEYPADCount_tmp == 0)
		{
			cCheckKEYPADCount_tmp = 1;
			OnBnClickedButtonShort_fun(28);   //<--加锁按键
			Sleep(100);
			OnStartKeypadTimer();
		}
		else if (cCheckKEYPADCount_tmp == 1)
		{
			if (myKeypadtimer == 0)
			{
				cCheckKEYPADCount_tmp = 2;
				OnStopKeypadTimer();
				Sleep(100);
				//OnBnClickedButtonShort_fun(29);   //<--检测按键
			}
			else if (bisOnekey)
			{
				if (bTestKeyPad_Menu)
				{
					cCheckKEYPADCount_tmp = 2;
					OnStopKeypadTimer();
					Sleep(100);
				}
			}
			else if (bisSevenkey)
			{
				if (bTestKeyPad_Power&&bTestKeyPad_VolMins&&bTestKeyPad_VolPlus&&bTestKeyPad_CHPlus&&bTestKeyPad_CHmins&&bTestKeyPad_Menu&&bTestKeyPad_Source)	
				{
					cCheckKEYPADCount_tmp = 2;
					OnStopKeypadTimer();
					Sleep(100);
				}
			}
			else 
			{
				if (bTestKeyPad_Power&&bTestKeyPad_VolMins&&bTestKeyPad_VolPlus&&bTestKeyPad_CHPlus&&bTestKeyPad_CHmins)
				{
					cCheckKEYPADCount_tmp = 2;
					OnStopKeypadTimer();
					Sleep(100);
				}
			}
		}
		else if (cCheckKEYPADCount_tmp == 2)
		{
			cCheckKEYPADCount_tmp = 3;
			Sleep(100);
			if (bisOnekey)
			{
				if (bTestKeyPad_Menu)
				{
					ShowMessage(BLUE, _T("按键测试	OK"), 0);
					if (m_cEdit_value_use_NG.IsEmpty())
					{
						WriteResultMES(cstrTestResultOK, 2);
						m_list.SetItemText(0, 2, (CString)_T("PASS"));
					}
				}
				else
				{
					ShowMessage(RED, _T("按键测试	NG"), 0);
					WriteResultMES(cstrTestResultNG, 2);
					m_list.SetItemText(0, 2, (CString)_T("NG"));
				}
			}
			else
			{
				if (bisSevenkey)
				{
					if (bTestKeyPad_Power&&bTestKeyPad_VolMins&&bTestKeyPad_VolPlus&&bTestKeyPad_CHPlus&&bTestKeyPad_CHmins&&bTestKeyPad_Menu&&bTestKeyPad_Source)
					{
						ShowMessage(BLUE, _T("按键测试	OK"), 0);
						if (m_cEdit_value_use_NG.IsEmpty())
						{
							WriteResultMES(cstrTestResultOK, 2);
							m_list.SetItemText(0, 2, (CString)_T("PASS"));
						}
					}
					else
					{
						ShowMessage(RED, _T("按键测试	NG"), 0);
						WriteResultMES(cstrTestResultNG, 2);
						m_list.SetItemText(0, 2, (CString)_T("NG"));
					}
				}
				else
				{
					if (bTestKeyPad_Power&&bTestKeyPad_VolMins&&bTestKeyPad_VolPlus&&bTestKeyPad_CHPlus&&bTestKeyPad_CHmins)
					{
						ShowMessage(BLUE, _T("按键测试	OK"), 0);
						if (m_cEdit_value_use_NG.IsEmpty())
						{
							WriteResultMES(cstrTestResultOK, 2);
							m_list.SetItemText(0, 2, (CString)_T("PASS"));
						}
					}
					else
					{
						ShowMessage(RED, _T("按键测试	NG"), 0);
						WriteResultMES(cstrTestResultNG, 2);
						m_list.SetItemText(0, 2, (CString)_T("NG"));
					}
				}
			}
			OnBnClickedButtonShort_fun(30);	  //<--解锁按键
		}
		else if (cCheckKEYPADCount_tmp == 3)
		{
			cCheckKEYPADCount_tmp = 4;
			OnBnClickedButtonShort_fun(19);   //<--关灯
			OnStopTimer();
		}
		else if (cCheckKEYPADCount_tmp == 4)
		{
			cCheckKEYPADCount_tmp = 5;
			KillTimer(TIMER_GET_DATA);
			boolSnScanDataOK = false;
		}
		else{
			cCheckKEYPADCount_tmp = 6;
			KillTimer(TIME_SHOW_MESSAGE_KEYPAY_TEST_OTHER);
			OnBnClickedButton3();
		}
	}
	else if (TIME_SHOW_MESSAGE_TEST_MAC == nIDEvent)
	{
		 if (cCheckMacCount_tmp == 0)    
		{
			cCheckMacCount_tmp = 1;
			OnBnClickedButtonShort_fun(18);//<--开指示灯
			m_cMfcButton_MENU.SetFaceColor(RGB(255, 255, 255));
			m_cMfcButton_SOURCE.SetFaceColor(RGB(255, 255, 255));
			m_cMfcButton_POWER.SetFaceColor(RGB(255, 255, 255));
			m_cMfcButton_CHP.SetFaceColor(RGB(255, 255, 255));
			m_cMfcButton_CHM.SetFaceColor(RGB(255, 255, 255));
			m_cMfcButton_VOLP.SetFaceColor(RGB(255, 255, 255));
			m_cMfcButton_VOLM.SetFaceColor(RGB(255, 255, 255));
		 }
		 else if (cCheckMacCount_tmp == 1)
		 {
			 if (TestMACBegin)
			 {
				 TestMACSend = TRUE;
				 OnBnClickedButtonShort_fun(37);    //<--MAC
				 TestMACBegin = FALSE;
				 cCheckMacCount_tmp = 2;
			 }
		 }
		 else
		 {
			 KillTimer(TIME_SHOW_MESSAGE_TEST_MAC);
			 SetTimer(TIME_SHOW_MESSAGE_TEST_DELAY_OTHER, 500, NULL);  //<-- iOtherTestDelay==500 为了满足V510方案写色温 
		 }
	}
	else if (TIMER_ORDER == nIDEvent)
	{
		if(Order_Flag==0)
		{
			m_userid.GetWindowText(strTemp);
			if(strTemp.GetLength()==7)
			{
				//SetDlgItemText(IDC_STATIC1, _T("请第一次输入12位有效订单号！"));
				strShowMessage.Format(_T("请第一次输入12位有效订单号！"));
				ShowMessage(BLUE,strShowMessage,0);
				m_userid.SetReadOnly(TRUE);
				m_userid.GetWindowText(strTemp);
				memset(UserName,0,32);
				sprintf(UserName,"%S",strTemp);//获取用户名
				GetDlgItem(IDC_EDIT_ORDER)->SetFocus();
				Order_Flag=1;
			}
		}else if(Order_Flag==1)
		{
			m_orderid.GetWindowText(strTemp);
			if(strTemp.GetLength()==12)
			{
				cstrHelp="";
				cstrHelp=strTemp;
				//SetDlgItemText(IDC_STATIC1, _T("请第二次输入12位有效订单号！"));
				strShowMessage.Format(_T("请第二次输入12位有效订单号！"));
				ShowMessage(BLUE,strShowMessage,0);
				m_orderid.SetWindowText(_T(""));
				Order_Flag=2;
			}
		}else if(Order_Flag==2)
		{
			m_orderid.GetWindowText(strTemp);
			if(strTemp.GetLength()==12)
			{
				if(strTemp!=cstrHelp)
				{	
					strShowMessage.Format(_T("两次输入订单号不一致！\n请重新第一次输入12位订单号！"));
					ShowMessage(RED,strShowMessage,0);
					m_orderid.SetWindowText(_T(""));
					Order_Flag=3;
				}
				else
				{
					memset(OrderID,0,16);
					sprintf(OrderID,"%S",strTemp);//获取订单号
					strShowMessage.Format(_T("订单号已确认！\n请确认其他配置信息！"));
					ShowMessage(BLUE,strShowMessage,0);
					m_orderid.SetReadOnly(TRUE);
					//m_Button_in.EnableWindow(TRUE);
					Order_Flag=4;
					KillTimer(TIMER_ORDER);
					//break;
				}
			}
		}
		else if(Order_Flag==3)
		{
			m_orderid.GetWindowText(strTemp);
			if(strTemp.GetLength()==12)
			{
				cstrHelp="";
				cstrHelp=strTemp;
				m_orderid.SetWindowText(_T(""));
				SetDlgItemText(IDC_STATIC_SHOW_MESSAGE, _T("请第二次输入12位有效订单号！"));
				Order_Flag=2;
			}
		}
		}
	else if (TIMER_TIME_RUN == nIDEvent)
	{
		time_run();  
	}
	else if (TIME_SHOW_MESSAGE_KEYPAD_TIME == nIDEvent)
	{
		Keypad_time_run();
	}
	else if (TIMER_GET_DATA == nIDEvent)
	{
		if(1)
			ScanData();
		else
			SetDlgItemText(IDC_EDIT_INTO_MESSAGE,_T(""));
	}
	CDialog::OnTimer(nIDEvent);
}

void CserialCommunicationDlg::ScanData()
{
	//MessageBox(_T("ScanData"));
	int color =RED; 
	CString strGetData(""),strShowMessage("");
	if(intSNLen > intMACLen)
		SendDlgItemMessage(IDC_EDIT_INTO_MESSAGE,EM_LIMITTEXT,intSNLen,0);
	else
		SendDlgItemMessage(IDC_EDIT_INTO_MESSAGE,EM_LIMITTEXT,intMACLen,0);
	GetDlgItemText(IDC_EDIT_INTO_MESSAGE,strGetData);
	if(strGetData.IsEmpty())
		return;
	ShowMessage(RED,"",0);
	m_CutData += strGetData;
	if((intSNLen == m_CutData.GetLength())||(intMACLen == m_CutData.GetLength()))
	{
		strGetData = m_CutData;
		m_CutData = "";
	}
	else if((intSNLen > m_CutData.GetLength())||(intMACLen > m_CutData.GetLength()))
	{
		strShowMessage.Format(_T("NG\r\n请扫描正确数据\r\n"));
		ShowMessage(RED,strShowMessage,0);
		SetDlgItemText(IDC_EDIT_INTO_MESSAGE,_T(""));
		return;
	}
	else
	{
		m_CutData = "";
		strShowMessage.Format(_T("NG\r\n请扫描正确数据\r\n"));
		ShowMessage(RED,strShowMessage,0);
		SetDlgItemText(IDC_EDIT_INTO_MESSAGE,_T(""));
		return;
	}
	ShowMessage(BLUE,strGetData,0);
	if((intSNLen == strGetData.GetLength())&&(BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_SN)))
	{
		BOOL boolSNRev = DataVerification(strGetData,SN_DATA);
		if (boolSnScanDataOK)
		{
			KillTimer(TIMER_GET_DATA);
			SetTimer(TIME_SHOW_MESSAGE_TEST_MAC,300,NULL);  //500->300 优化时间
		}
	}
	else if(intMACLen ==strGetData.GetLength()&&(BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_MAC)))
	{
		BOOL boolMACRev = DataVerification(strGetData,MAC_DATA);
	}
}

BOOL CserialCommunicationDlg::DataVerification(CString strVerData, UINT intFlag)
{
	//MessageBox(_T("DataVerification"));
	BOOL boolDVer = false;
	CString strShowMessage("");
	OnStartTimer();
	int color =BLUE;
	if(SN_DATA == intFlag)
	{
		sprintf(LotSN,"%s",strVerData);
		sprintf(FieldName,"%s",strMainKey);
		BOOL boolRev = ADCCheckLotSN("TEST",  LotSN,  FieldName);
		//for test
		GetMacData = "b0:b1:b2:b3:b4:b5";
		WriteResultMES(GetMacData, 1);
		//MessageBox(_T("ADCCheckLotSN"));
		if(!boolRev) //<--ADCCheckLotSN 失败
		{
			KillTimer(TIMER_GET_DATA);
			OnStopTimer();
			GetDlgItem(IDC_BUTTON3)->EnableWindow(true);
			return false;
		}
		else
		{
			//MessageBox("ADCCheckLotSN is True");
		}
		CString strRev = mesCheckorSubmitMap[_T("ReturnResult")];//检查返回结果--CString 值为“1”或“0”
		int intRev = _ttoi(strRev);
		if(0 == intRev)   //0
		{
			CString strRevMeg = mesCheckorSubmitMap[_T("ReturnMessage")];//检查返回结果如失败，这是一个CString的失败原因
			OnStopTimer();
			if(0)
			{

			}
			else
			{
				//MessageBox(_T("检查返回结果失败"));
				boolCheckFail = true;
				boolCheckRev = false;
				strShowMessage.Format(_T("NG\r\n请扫描正确数据\r\n%s"),strRevMeg); //MES检测错误,继续检测
				color = RED;
				boolScanDataOK = false;
				SetTimer(TIME_SHOW_MESSAGE_TEST_DELAY_OTHER, 500, NULL);  //<--zb
				m_cMfcButton_MENU.SetFaceColor(RGB(255, 255, 255));
				m_cMfcButton_SOURCE.SetFaceColor(RGB(255, 255, 255));
				m_cMfcButton_POWER.SetFaceColor(RGB(255, 255, 255));
				m_cMfcButton_CHP.SetFaceColor(RGB(255, 255, 255));
				m_cMfcButton_CHM.SetFaceColor(RGB(255, 255, 255));
				m_cMfcButton_VOLP.SetFaceColor(RGB(255, 255, 255));
				m_cMfcButton_VOLM.SetFaceColor(RGB(255, 255, 255));
			}
		}
		else 
		{
			ADCGetInterfaceParameter("GETDATE",  LotSN,  FieldName);//获取当前时间
			boolCheckFail = true;
			CString strSNaMAC = (_T("SN/MAC"));
			CString strGetMeg("");
			BOOL boolDoulData = false;
			boolRev =ADCGetInterfaceParameter("FieldData",  LotSN,  FieldName);//获取对应主键的数据
			if(!boolRev)
			{
				KillTimer(TIMER_GET_DATA);
				OnStopTimer();
				GetDlgItem(IDC_BUTTON3)->EnableWindow(true);
				return boolDVer;
			}else
			{
				CString cstritm0;
				cstritm0.Format(_T("%d"),number);
				m_list.InsertItem(0,cstritm0, 0);
				m_list.SetItemText(0,1, mesGetTimeMap[_T("DATE")]);
				m_list.SetItemText(0,4,(CString)strVerData);
				boolSnScanDataOK = true;
				strShowMessage.Format(_T("%s"), strVerData);
			}
			//else
			//{
			//	MessageBox("通过SN值获取得到SN值,失败");
			//}
			number++;
		}
	}
	ShowMessage(color,strShowMessage,0);
	SetDlgItemText(IDC_EDIT_INTO_MESSAGE,_T(""));
	return boolDVer;
}
void CserialCommunicationDlg::WriteResultMES(CString strVerData,int MacInput)
{
	CString strShowMessage("");
	CString strRev("");
	//MessageBox(strVerData);
	int color=BLACK;
	if((BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_SN)))
	{
		sprintf(MacEth,"%s",strVerData);
		ADCSubmitTestData("TEST",  LotSN,  FieldName, MacInput);
	}
	else
	{
		ADCSubmitTestData("TEST",  LotSN,  FieldName,0);//1代表要讲MAC地址写入MES 0代表否
	}
	strRev = mesCheckorSubmitMap[_T("ReturnResult")];
	int intRev = _ttoi(strRev);
	if(0 == intRev)
	{
		if (!WriteMesOK)
		{
			strShowMessage.Format(_T("NG\r\n%s"),mesCheckorSubmitMap[_T("ReturnMessage")]);
			color = RED;
			ShowMessage(RED,strShowMessage,0);
			//m_list.SetItemText(0,2,(CString) _T("NG"));
		}	
		//m_list.SetItemText(0,2,(CString) _T("NG"));
		//GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText(strShowMessage);
		//return;
	}
	else
	{
		GetMacData = "";
		//m_list.SetItemText(0,2,(CString) _T("PASS"));
		//m_list.SetTextColor();
		m_list.SetItemText(0,3, (CString) strVerData);
		WriteMesOK = true;
		//m_strSNData.Format(_T(""));
		//m_strMACData.Format(_T(""));
		//strShowMessage.Format(_T("OK"));
		/*if("WF/BT" == strCheckMeg)
			strShowMessage.Format(_T("wifi验证OK\r\n蓝牙验证OK\r\n编译信息验证成功\r\n烧录成功\r\nPASS\r\n"));
		else if("WIFI" == strCheckMeg)
			strShowMessage.Format(_T("wifi验证OK\r\n编译信息验证成功\r\n烧录成功\r\nPASS\r\n"));
		else if("蓝牙" == strCheckMeg)
			strShowMessage.Format(_T("蓝牙验证OK\r\n编译信息验证成功\r\n烧录成功\r\nPASS\r\n"));
		else
			strShowMessage.Format(_T("编译信息验证成功\r\n烧录成功\r\nPASS\r\n"));*/
		//color = BLUE;
	}
	boolScanDataOK = false;
	//boolScanSNDataOK = false;
	//boolScanMACDataOK = false;
	//OnStopTimer();
	//ShowMessage(color,strShowMessage,6000);
}
void CserialCommunicationDlg::OnBnClickedButtonReceiveClear()
{
	// TODO: 在此添加控件通知处理程序代码
	showString.Empty();
	((CEdit*)GetDlgItem(IDC_EDIT_RECEIVE))->SetWindowText("");
	((CEdit*)GetDlgItem(IDC_EDIT_RECEIVE))->LineScroll(0);
}

void CserialCommunicationDlg::ReceiveSaveMacSuccessFile(CString cMessage)
{
	countMacPass += 1;
	CTime tm = CTime::GetCurrentTime();
	CString strtm = tm.Format("ReceiveMacMessage_%Y_%m_%d");
	strtm += _T(".txt");
	char* cFileName = (LPSTR)(LPCTSTR)strtm;
	//m_cEdit_value_use_NG = "BLUE	      NG\r\n";
	CTime t = CTime::GetCurrentTime();
	CString s = t.Format("\r\n***************Receive Mac Message At Time %Y_%m_%d-%H:%M:%S***************\r\n");
	FILE *fp;
	fp = fopen(cFileName, "a+");
	if (fp != NULL)
	{
		fprintf(fp, "%s\n", s);
		fprintf(fp, "MacPass %d\n", countMacPass);
		fprintf(fp, "%s\n", CheckData);
		fclose(fp);
	}
}
void CserialCommunicationDlg::ReceiveSaveMacErrorFile(CString cMessage)
{	
	countMacPass += 1;
	CTime tm = CTime::GetCurrentTime();
	CString strtm = tm.Format("ReceiveMacMessage_%Y_%m_%d");
	strtm += _T(".txt");
	char* cFileName = (LPSTR)(LPCTSTR)strtm;
	//m_cEdit_value_use_NG = "BLUE	      NG\r\n";
	CTime t = CTime::GetCurrentTime();
	CString s = t.Format("\r\n***************Receive Mac Message At Time %Y_%m_%d-%H:%M:%S***************\r\n");
	FILE *fp;
	fp = fopen(cFileName, "a+");
	if (fp != NULL)
	{
		fprintf(fp, "%s\n", s);
		fprintf(fp, "MacNG %d\n", countMacPass);
		fprintf(fp, "%s\n", CheckData);
		fclose(fp);
	}
}
void CserialCommunicationDlg::OnBnClickedButtonReceiveSaveFile()
{
	// TODO: 在此添加控件通知处理程序代码
	//showString.Append("552627136281361283612372618");
	//showString.Append("21312321321321321313aadsdad");;
	if (1)
	{
		CTime tm = CTime::GetCurrentTime();
		CString strtm = tm.Format("%Y_%m_%d");
		strtm += _T(".txt");
		char* cFileName = (LPSTR)(LPCTSTR)strtm;
		//m_cEdit_value_use_NG = "BLUE	      NG\r\n";
		CTime t = CTime::GetCurrentTime();
		CString s = t.Format("\r\n***************Receive Error Message At Time %Y_%m_%d-%H:%M:%S***************\r\n");
		FILE *fp;
		fp = fopen(cFileName, "a+");
		if (fp != NULL)
		{
			fprintf(fp, "%s\n", s);
			fprintf(fp, "%s\n", m_cEdit_value_use_NG);
			fprintf(fp, "%s\n", curretBufString_test);
			fclose(fp);
		}
	}
}
void CserialCommunicationDlg::OnBnClickedButtonSend()
{
	// TODO: 在此添加控件通知处理程序代码
	if(serial_isopen == TRUE)
	{
		//变量打开了,要去测试串口状态
		if(sio_lstatus(com_port)>= 0)
		{
			CString str1;
			sendString.Empty();
			//没有对\r\n特殊处理
			((CEdit*)GetDlgItem(IDC_EDIT_SEND))->GetWindowText(str1);

			if(sendStringConfig == false)sendString.AppendFormat("%s",str1);
			else sendString.AppendFormat("%s\r\n",str1);
			
			char ttt[30] = { 0x69, 0x73, 0x23, 0,0,0,'\0' };
			int strLength = str1.GetLength();// + 1;
			char *pValue = new char[strLength];
			strncpy(pValue, str1, strLength);
			String2Hex(pValue, ttt);
			sio_write(com_port, ttt, strLength/2);//strLength/2
			//sio_write(com_port,sendString.GetBuffer(),sendString.GetLength());
			//curretBufString_test.AppendFormat("55B20536570050D0E0EF0FE");
			//curretBufString_test.AppendFormat("55B2075365701D110C09DCFE");
			//curretBufString_test.AppendFormat("55B206B0A37EB0A3EE0036FE");
			//SerialsCheckBuffer(curretBufString_test);
		}
		else
		{
			//说明串口已经丢失
			MessageBox("串口丢失,请关闭后重新打开","错误",MB_OK);
		}
	}
	else
	{
		//说明串口已经丢失
		MessageBox("请先打开串口","错误",MB_OK);
	}
}

void CserialCommunicationDlg::OnBnClickedButtonSendClear()
{
	// TODO: 在此添加控件通知处理程序代码
	sendString.Empty();
	((CEdit*)GetDlgItem(IDC_EDIT_SEND))->SetWindowText("");
	((CEdit*)GetDlgItem(IDC_EDIT_SEND))->LineScroll(0);
}

void CserialCommunicationDlg::OnBnClickedButtonSendFile()
{
	// TODO: 在此添加控件通知处理程序代码
	if(serial_isopen == TRUE)
	{
		if(sio_lstatus(com_port) >= 0)
		{
			CFileDialog filedig(TRUE);
			filedig.m_ofn.lpstrTitle = "串口发送文件选择";
			filedig.m_ofn.lpstrFilter = "Text Files(*.txt)\0*.txt\0ALL Files(*.*)\0*.*\0\0";
			filedig.m_ofn.lpstrDefExt = "txt";
			if(IDOK == filedig.DoModal())
			{
				CString filePath;
				filePath.AppendFormat(filedig.GetFolderPath());
				filePath.Append("\\");
				filePath.Append(filedig.GetFileName());
				CFile file(filePath,CFile::modeRead|CFile::modeNoTruncate);	
				file.SeekToBegin();
				UINT32 length = (UINT32)(file).GetLength();
				char* str = (char*)malloc(sizeof(char)*length);
				file.Read(str,length);
				sio_write(com_port,str,length);
			}
		}
		else
		{
			MessageBox("串口丢失,请关闭后重新打开","错误",MB_OK);
		}
	}
	else
	{
		MessageBox("请先打开串口","错误",MB_OK);
	}
}



void CserialCommunicationDlg::OnCbnSetfocusComboConfigFileSelect()
{
	// TODO: 在此添加控件通知处理程序代码
	byte i =0,j = 0;
	CFileFind ff;
	BOOL ret = ff.FindFile("*.xml");
	strPath.RemoveAll();
	while(ret)
	{
		ret = ff.FindNextFile();
		strPath.Add(ff.GetFileName()); 
		i++;
	}
	ff.Close();
	((CComboBox*)(GetDlgItem(IDC_COMBO_CONFIG_FILE_SELECT)))->ResetContent();
	for(j = 0; j < i; j++)
	{
		((CComboBox*)(GetDlgItem(IDC_COMBO_CONFIG_FILE_SELECT)))->AddString(strPath.GetAt(j));
	}
}

BOOL CserialCommunicationDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	m_toolstip.RelayEvent(pMsg);
	return CDialog::PreTranslateMessage(pMsg);
}

void CserialCommunicationDlg::OnBnClickedButtonLoadConfigFile()  //加载config
{
	CFileDialog dlg(TRUE, NULL, NULL, NULL, NULL);
	CserialCommunicationApp *pApp = (CserialCommunicationApp *)AfxGetApp();
	//GetDlgItem(IDC_EDIT_TEST_ITEM)->SetWindowText(_T(""));
	TestWifiBegin = TRUE;
	TestBlueBegin = TRUE;
	TestUsbBegin  = TRUE;
	TestNetBegin  = TRUE;
	TestVoiceBegin = TRUE;
	TestUsbSend = FALSE;
	TestWifiSend = FALSE;
	TestBlueSend = FALSE;
	TestNetSend = FALSE;
	TestAddVoice = TRUE;
	bFacWriteWB_Normal_Red = FALSE;
	bFacWriteWB_Normal_Green = FALSE;
	bFacWriteWB_Normal_Blue = FALSE;
	bFacWriteWB_Cool_Red = FALSE;
	bFacWriteWB_Cool_Green = FALSE;
	bFacWriteWB_Cool_Blue = FALSE;
	bFacWriteWB_Warm_Red = FALSE;
	bFacWriteWB_Warm_Green = FALSE;
	bFacWriteWB_Warm_Blue = FALSE;
	m_cMfcButton_MENU.ShowWindow(SW_SHOW);
	m_cMfcButton_SOURCE.ShowWindow(SW_SHOW);
	m_cMfcButton_CHM.ShowWindow(SW_SHOW);
	m_cMfcButton_CHP.ShowWindow(SW_SHOW);
	m_cMfcButton_VOLM.ShowWindow(SW_SHOW);
	m_cMfcButton_VOLP.ShowWindow(SW_SHOW);
	m_cMfcButton_POWER.ShowWindow(SW_SHOW);
	bisOnekey = FALSE;
	bisSevenkey = FALSE;
	boolSnScanDataOK = false ;
	//pApp->app_strSource
	if (dlg.DoModal() == IDOK)//
	{	
		CString strTestItem,strTemp;
		CString str, str1;
		str = dlg.GetPathName();
		str1 = dlg.GetFileName();
		CEdit* cfolder;
		cfolder = (CEdit*)GetDlgItem(IDC_EDIT_OK);
		cfolder->SetWindowText(str + "  name:" + str1);
		str1.MakeLower();
		if (str1.Find(".txt") == -1)
		{
			MessageBox("无效文件");
			return;
		}
		if (str1.Find("色温") != -1)
		{
			MessageBox("无效文件");
			return;
		}
		if (str1.Find("rgb_value") != -1)
		{
			MessageBox("无效文件");
			return;
		}
		long Filelen;
		UCHAR* a_pData;
		FILE* fp;
		errno_t err;
		char *pBuff = str.GetBuffer(0);
		if ((err = fopen_s(&fp, pBuff, "rb")) == 0)//(fp=fopen_s(pBuff,"rb"))       //str是我的.bin 文件路径
		{
			fseek(fp, 0, SEEK_END);
			Filelen = ftell(fp);
			//m_lFilelen = Filelen;
			//分配内存
			a_pData = (UCHAR*)malloc(Filelen);
			//u_newData = (UCHAR*)malloc(Filelen);
			//读取文件到分配的内存
			fseek(fp, 0, SEEK_SET);
			fread(a_pData, 1, Filelen, fp);
			fclose(fp);

			CString s, sResult;
			int n=0;
			sMacAddr_config = "";
			strLastSource = "";
			sVersion_config = "";
			for (int j = 0; j < 20; j++)
			{
				SourceConfig[j].index = 0;
				SourceConfig[j].source = "";
				SourceConfig[j].btime = 0;
				SourceConfig[j].bhaveSignal = FALSE;
			}
			//<--

			for (int i = 0; i <(Filelen-8); i++)//m_lFilelen
			{
				if((a_pData[i] == '/') && (a_pData[i + 1] == '/') && (a_pData[i + 2] == 'W') && (a_pData[i + 3] == 'I') && (a_pData[i + 4] == 'F') && (a_pData[i + 5] == 'I'))
				{
					TestWifiBegin = FALSE;
				}
				if((a_pData[i] == '/') && (a_pData[i + 1] == '/') && (a_pData[i + 2] == 'B') && (a_pData[i + 3] == 'L') && (a_pData[i + 4] == 'U') && (a_pData[i + 5] == 'E'))
				{
					TestBlueBegin = FALSE;
				}
				if((a_pData[i] == '/') && (a_pData[i + 1] == '/') && (a_pData[i + 2] == 'U') && (a_pData[i + 3] == 'S') && (a_pData[i + 4] == 'B'))
				{
					TestUsbBegin = FALSE;
				}
				if((a_pData[i] == '/') && (a_pData[i + 1] == '/') && (a_pData[i + 2] == 'N') && (a_pData[i + 3] == 'E') && (a_pData[i + 4] == 'T'))
				{
					TestNetBegin = FALSE;
				}
				if((a_pData[i] == '/') && (a_pData[i + 1] == '/') && (a_pData[i + 2] == 'V') && (a_pData[i + 3] == 'O') && (a_pData[i + 4] == 'I') && (a_pData[i + 5] == 'C') && (a_pData[i + 6] == 'E'))
				{
					TestVoiceBegin = FALSE;
				}
				//u_newData[i] = a_pData[i];
				if (a_pData[i] == '.')
				{
					char tmp[2];
					tmp[0] = a_pData[i - 2];
					tmp[1] = a_pData[i - 1];
					int index;
					index = atoi(tmp);
					SourceConfig[n].index = index;
					if (a_pData[i + 3] == '=')
					{
						tmp[0] = a_pData[i + 1];
						tmp[1] = a_pData[i + 2];
						s.Format("%c", tmp[0]);
						AfxMessageBox(s);
						SourceConfig[n].source=s;
						s.Format("%c", tmp[1]);
						SourceConfig[n].source += s;
					}
					else if (a_pData[i + 4] == '=')
					{
						char tmp3[3];
						tmp3[0] = a_pData[i + 1];
						tmp3[1] = a_pData[i + 2];
						tmp3[2] = a_pData[i + 3];
						s.Format("%c", tmp3[0]);
						SourceConfig[n].source = s;
						s.Format("%c", tmp3[1]);
						SourceConfig[n].source += s;
						s.Format("%c", tmp3[2]);
						SourceConfig[n].source += s;
					}
					else if (a_pData[i + 5] == '=')
					{
						char tmp4[4];
						tmp4[0] = a_pData[i + 1];
						tmp4[1] = a_pData[i + 2];
						tmp4[2] = a_pData[i + 3];
						tmp4[3] = a_pData[i + 4];
						s.Format("%c", tmp4[0]);
						SourceConfig[n].source = s;
						s.Format("%c", tmp4[1]);
						SourceConfig[n].source += s;
						s.Format("%c", tmp4[2]);
						SourceConfig[n].source += s;
						s.Format("%c", tmp4[3]);
						SourceConfig[n].source += s;
					}
					else if (a_pData[i + 6] == '=')
					{
						char tmp5[5];
						tmp5[0] = a_pData[i + 1];
						tmp5[1] = a_pData[i + 2];
						tmp5[2] = a_pData[i + 3];
						tmp5[3] = a_pData[i + 4];
						tmp5[4] = a_pData[i + 5];
						s.Format("%c", tmp5[0]);
						SourceConfig[n].source = s;
						s.Format("%c", tmp5[1]);
						SourceConfig[n].source += s;
						s.Format("%c", tmp5[2]);
						SourceConfig[n].source += s;
						s.Format("%c", tmp5[3]);
						SourceConfig[n].source += s;
						s.Format("%c", tmp5[4]);
						SourceConfig[n].source += s;
					}

				}
				if (a_pData[i] == '=')
				{
					if (a_pData[i + 2] == 'S')
					{
						char tmp[2];
						tmp[0] = a_pData[i +1];
						tmp[1] = a_pData[i + 2];
						SourceConfig[n].btime = atoi(tmp);
					}
					else if (a_pData[i + 3] == 'S')
					{
						char tmp[2];
						tmp[0] = a_pData[i + 1];
						tmp[1] = a_pData[i + 2];
						SourceConfig[n].btime = atoi(tmp);
					}
					else if (a_pData[i + 4] == 'S')
					{
						char tmp[3];
						tmp[0] = a_pData[i + 1];
						tmp[1] = a_pData[i + 2];
						tmp[2] = a_pData[i + 3];
						SourceConfig[n].btime = atoi(tmp);
					}
					else if (a_pData[i + 5] == 'S')
					{
						char tmp[4];
						tmp[0] = a_pData[i + 1];
						tmp[1] = a_pData[i + 2];
						tmp[2] = a_pData[i + 3];
						tmp[3] = a_pData[i + 4];
						SourceConfig[n].btime = atoi(tmp);
					}
					n++;
				}
				if ((a_pData[i] == 'E') && (a_pData[i + 1] == 'N') && (a_pData[i + 2] == 'D') && (a_pData[i + 3] == ':'))
				{
					break;
				}
				if ((a_pData[i] == 'L') && (a_pData[i + 1] == 'A') && (a_pData[i + 2] == 'S') && (a_pData[i + 3] == 'T'))
				{
					int j = i + 5;
					int k = 0;
					char tmp[10];
					strLastSource = "";
					for (j ; j <= (i + 10); j++)
					{
						if (a_pData[j] == ')')
							break;
						tmp[k] = a_pData[j];
						s.Format("%c", tmp[k]);
						strLastSource += s;
						k++;
					}
				}
				if ((a_pData[i] == 'K') && (a_pData[i + 1] == 'e') && (a_pData[i + 2] == 'y') && (a_pData[i + 3] == 'p') && (a_pData[i + 4] == 'a') && (a_pData[i + 5] == 'd'))
				{
					if (a_pData[i + 7] == '1')
					{
						bisOnekey = TRUE;
					}
					else {
						if (a_pData[i + 7] == '5')
							bisSevenkey = FALSE;
						else
							bisSevenkey = TRUE;
					}
				}
				if ((a_pData[i] == 'V') && (a_pData[i + 1] == 'E') && (a_pData[i + 2] == 'R'))
				{
					int j = 0;
					char tmp[50];
					sVersion_config = "";
					while (j<48)
					{
						tmp[j] = a_pData[i + j + 4];
						if (tmp[j] == ')')
						{
							//tmp[j+1] = '\r';
							//tmp[j+2] = '\n';
							break;
						}
						s.Format("%c", tmp[j]);
						sVersion_config += s;
						j++;
					}
					//s.Format("%c", tmp);
					//sVersion_config = s;
					GetDlgItem(IDC_EDIT_VERSION)->SetWindowText(sVersion_config);
				}
				if ((a_pData[i] == 'M') && (a_pData[i + 1] == 'A') && (a_pData[i + 2] == 'C'))
				{
					int j = 0;
					char tmp[13];
					sMacAddr_config = "";
					while (j<13)
					{
						if (a_pData[i+j] == ')')
						{
							break;
						}
						tmp[j] = a_pData[i + j ];
						s.Format("%c", tmp[j]);
						sMacAddr_config += s;
						j++;
					}
					//s.Format("%c", tmp);
					//sVersion_config = s;
				}
				//s.Format("%c", a_pData[i]);//后面加个空格，把每个字节分开，可以看得清楚些 
				//sResult += s; //sResult就是要的16进制字符串了
				//m_file += a_pData[i];
				//if (((i>16) && ((i + 1) % 16 == 0)) || (i == 15))
					//sResult += "\r\n";
				//n++;
				if (n >= 20)
					break;
			}
			configFileLoadOk = true;
			GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
			strTestItem = "测试项目如下:\r\n";
			strTestItem += "===========================\r\n";
			strTestItem += "配置版本号：\r\n";
			if (TestWifiBegin)
			{
				strTestItem += "测试WIFI";
				strTestItem += "\r\n";
			}
			if (TestBlueBegin)
			{
				strTestItem += "测试BLUE";
				strTestItem += "\r\n";
			}
			if (TestUsbBegin)
			{
				strTestItem += "测试USB";
				strTestItem += "\r\n";
			}
			if (TestNetBegin)
			{
				strTestItem += "测试网口";
				strTestItem += "\r\n";
			}
			if (TestVoiceBegin)
			{
				strTestItem += "测试话筒";
				strTestItem += "\r\n";
			}
			strTestItem += "-----------------------------------\r\n";
			if (sMacAddr_config.Find("MAC")!=-1)
			{
				strTestItem += "配置MAC：\r\n";
				strTestItem += sMacAddr_config + ')';
				strTestItem += "\r\n";
				strTestItem += "-----------------------------------\r\n";
			}
			if (bisOnekey)
			{
				strTestItem += "测试1键";
			}
			else {
				if (!bisSevenkey)
					strTestItem += "测试5键";
				else
					strTestItem += "测试7键";
			}
			strTestItem += "\r\n";
			strTestItem += "-----------------------------------\r\n";
			
			for (int j = 0; j < 20; j++)
			{
				if (SourceConfig[j].index == 0)
					break;
				strTestItem += SourceConfig[j].source;
				strTemp.Format("等待%d S", SourceConfig[j].btime);
				strTestItem += strTemp;
				strTestItem += "\r\n";
				pApp->app_strSource = strTestItem;
			}
			strTestItem += "-----------------------------------\r\n";
			strTestItem += "Source完成后跳转：";
			strTestItem += "\r\n";
			strTestItem += strLastSource;
			strTestItem += "\r\n";
			GetDlgItem(IDC_EDIT_TEST_ITEM)->SetWindowText(strTestItem);

			if (bisOnekey)
			{
				m_cMfcButton_MENU.ShowWindow(SW_HIDE);
				m_cMfcButton_SOURCE.ShowWindow(SW_HIDE);
				m_cMfcButton_CHM.ShowWindow(SW_HIDE);
				m_cMfcButton_CHP.ShowWindow(SW_HIDE);
				m_cMfcButton_VOLM.ShowWindow(SW_HIDE);
				m_cMfcButton_VOLP.ShowWindow(SW_HIDE);
			}
			else {
				if (bisSevenkey)
				{
					m_cMfcButton_MENU.ShowWindow(SW_SHOW);
					m_cMfcButton_SOURCE.ShowWindow(SW_SHOW);
				}
				else
				{
					m_cMfcButton_MENU.ShowWindow(SW_HIDE);
					m_cMfcButton_SOURCE.ShowWindow(SW_HIDE);
				}
			}
			//UpdateData(FALSE);
			//m_lFilelen = 256;
			//EDID_data_Fresh_txt(a_pData);
			//int size = sResult.GetLength();
			//file.Write(sResult,size);
			//file.Flush();
			//file.Close();
		}
	}
}

void CserialCommunicationDlg::LoadConfigXMLFile()
{
	// 装载配置文件
	//if(configFileLoadOk == false)//载入文件,载入后按钮变成卸载,下拉列表框变成禁用
	{
		//int i = ((CComboBox*)(GetDlgItem(IDC_COMBO_CONFIG_FILE_SELECT)))->GetCurSel();
		CString fileName;
		/*if(i == -1)
		{
			MessageBox("请选择有效文件","提示",MB_OK);
			return;
		}
		else
		{
			fileName = strPath.GetAt(i);
		}*/
		fileName = "config1.xml";
		//加载文件
		XMLError load = configXmlFile.LoadFile(fileName);
		if(load != XML_SUCCESS)
		{
			MessageBox("文件加载失败","提示",MB_OK);
			//configFileLoadOk = false;
			return;
		}
		//文件加载成功
		XMLElement* pElement;
		XMLElement* hRoot;
		//加载根节点
		hRoot = configXmlFile.RootElement();
		if(!hRoot)
		{
			MessageBox("根节点错误","提示",MB_OK);
			return;
		} 
		//根节点加载成功
		buttonSendString->RemoveAll();
		//加载快捷按键信息
		pElement = hRoot->FirstChildElement("button_messages")->FirstChildElement("button");//寻找一个子元素
		CString index ;
		CString nameString ;
		CString promptMessage;
		CString serialSendString;
		CString enable;
		bool enableFlag;
		int indexValue;
		//获取全部button数据
		do 
		{
			index = pElement->FirstChildElement("index")->GetText();
			nameString = pElement->FirstChildElement("name")->GetText();
			promptMessage = pElement->FirstChildElement("promptmessage")->GetText();
			serialSendString = pElement->FirstChildElement("serialSendString")->GetText();
			enable = pElement->FirstChildElement("enable")->GetText();
			XMLUtil util;
			util.ToInt(index,&indexValue);
			util.ToBool(enable,&enableFlag);
			buttonSendString->Add(serialSendString);
			
			pElement = pElement->NextSiblingElement();
		} while (pElement);

		//加载列表框讯息
#if 0
		listSendString->RemoveAll();
		pElement = hRoot->FirstChildElement("list_messages")->FirstChildElement("list");//寻找一个子元素
		UINT32 num = 0;
		do 
		{
			index = pElement->FirstChildElement("index")->GetText();
			nameString = pElement->FirstChildElement("name")->GetText();
			promptMessage = pElement->FirstChildElement("promptmessage")->GetText();
			serialSendString = pElement->FirstChildElement("serialSendString")->GetText();
			enable = pElement->FirstChildElement("enable")->GetText();
			XMLUtil util;
			util.ToInt(index,&indexValue);
			util.ToBool(enable,&enableFlag);
			if(enableFlag)
			{
				listSendString->Add(serialSendString);
				CString show;
				show.Empty();
				show.AppendFormat("%d  命令名: %s  功能: %s",num,nameString,promptMessage);
				((CListBox*)GetDlgItem(IDC_LIST1))->AddString(show);
				num++;
			}
			pElement = pElement->NextSiblingElement();
		} while (pElement);
		
		int count = ((CListBox*)GetDlgItem(IDC_LIST1))->GetCount();
		if(count >  0)
		{
			GetDlgItem(IDC_BUTTON_SEND_SHORT_FUCTION)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_SEND_SELECT_SHORT)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_FUNC_SEL)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_ADD_FUNC)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_DELFUNC)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_DEFAULT)->EnableWindow(TRUE);
		}
#endif
		//GetDlgItem(IDC_COMBO_CONFIG_FILE_SELECT)->EnableWindow(FALSE);
		//GetDlgItem(IDC_BUTTON_LOAD_CONFIG_FILE)->SetWindowText("卸载");
		//configFileLoadOk = true;
	}
	/*else
	{
		((CListBox*)GetDlgItem(IDC_LIST1))->ResetContent();

		GetDlgItem(IDC_BUTTON_SHORT1)->SetWindowText("none");
		GetDlgItem(IDC_BUTTON_SHORT2)->SetWindowText("none");
		GetDlgItem(IDC_BUTTON_SHORT3)->SetWindowText("none");
		GetDlgItem(IDC_BUTTON_SHORT4)->SetWindowText("none");
		GetDlgItem(IDC_BUTTON_SHORT5)->SetWindowText("none");
		GetDlgItem(IDC_BUTTON_SHORT6)->SetWindowText("none");
		GetDlgItem(IDC_BUTTON_SHORT7)->SetWindowText("none");
		GetDlgItem(IDC_BUTTON_SHORT8)->SetWindowText("none");
		GetDlgItem(IDC_BUTTON_SHORT1)->EnableWindow(false);
		m_toolstip.DelTool(GetDlgItem(IDC_BUTTON_SHORT1));
		GetDlgItem(IDC_BUTTON_SHORT2)->EnableWindow(false);
		m_toolstip.DelTool(GetDlgItem(IDC_BUTTON_SHORT2));
		GetDlgItem(IDC_BUTTON_SHORT3)->EnableWindow(false);
		m_toolstip.DelTool(GetDlgItem(IDC_BUTTON_SHORT3));
		GetDlgItem(IDC_BUTTON_SHORT4)->EnableWindow(false);
		m_toolstip.DelTool(GetDlgItem(IDC_BUTTON_SHORT4));
		GetDlgItem(IDC_BUTTON_SHORT5)->EnableWindow(false);
		m_toolstip.DelTool(GetDlgItem(IDC_BUTTON_SHORT5));
		GetDlgItem(IDC_BUTTON_SHORT6)->EnableWindow(false);
		m_toolstip.DelTool(GetDlgItem(IDC_BUTTON_SHORT6));
		GetDlgItem(IDC_BUTTON_SHORT7)->EnableWindow(false);
		m_toolstip.DelTool(GetDlgItem(IDC_BUTTON_SHORT7));
		GetDlgItem(IDC_BUTTON_SHORT8)->EnableWindow(false);
		m_toolstip.DelTool(GetDlgItem(IDC_BUTTON_SHORT8));
		GetDlgItem(IDC_COMBO_CONFIG_FILE_SELECT)->EnableWindow(true);
		GetDlgItem(IDC_BUTTON_LOAD_CONFIG_FILE)->SetWindowText("载入");

		GetDlgItem(IDC_BUTTON_SEND_SHORT_FUCTION)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_SEND_SELECT_SHORT)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_FUNC_SEL)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_ADD_FUNC)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_DELFUNC)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_DEFAULT)->EnableWindow(FALSE);
		configFileLoadOk = false;
	}*/
}
#if 0
void CserialCommunicationDlg::OnBnClickedButtonSendShortFuction()
{
	//获取当前选中
	UINT32 i = ((CListBox*)GetDlgItem(IDC_LIST1))->GetCurSel();
	if(i == -1)
	{
		MessageBox("请先选中一行","错误",MB_OK);
		return ;
	}
	else
	{
		if(serial_isopen == TRUE)
		{
			//变量打开了,要去测试串口状态
			if(sio_lstatus(com_port)>= 0)
			{
				//没有对\r\n特殊处理
				sendString.Empty();
				if(sendStringConfig == false)sendString.AppendFormat("%s",listSendString->GetAt(i));
				else sendString.AppendFormat("%s\r\n",listSendString->GetAt(i));
				char ttt[7] = { 0x69, 0x73, 0x23, 0,0,0,'\0' };
				int strLength = sendString.GetLength() + 1;
				char *pValue = new char[strLength];
				strncpy(pValue, sendString, strLength);
				String2Hex(pValue, ttt);
				sio_write(com_port, ttt, strLength/2);
				//sio_write(com_port,sendString.GetBuffer(),sendString.GetLength());
				((CEdit*)GetDlgItem(IDC_EDIT_SEND))->SetWindowText(sendString.GetBuffer());
			}
			else
			{
				//说明串口已经丢失
				MessageBox("串口丢失,请关闭后重新打开","错误",MB_OK);
			}
		}
		else
		{
			//说明串口已经丢失
			MessageBox("请先打开串口","错误",MB_OK);
		}
	}
}

void CserialCommunicationDlg::OnBnClickedButtonSendSelectShort()
{
	// TODO: 在此添加控件通知处理程序代码
	CString num;
	num.Empty();
	((CEdit*)GetDlgItem(IDC_EDIT_FUNC_SEL))->GetWindowText(num);
	int funcNum;
	if(num.GetLength() <= 0)
	{
		MessageBox("请写入一个指令序号","提示",MB_OK);
		return;
	}
	funcNum = atoi(num);
	if(funcNum >= listSendString->GetCount())
	{
		MessageBox("指令序号超范围","提示",MB_OK);
		return ;
	}
	if(serial_isopen == TRUE)
	{
		//变量打开了,要去测试串口状态
		if(sio_lstatus(com_port)>= 0)
		{
			//没有对\r\n特殊处理
			sendString.Empty();
			if(sendStringConfig == false)sendString.AppendFormat("%s",listSendString->GetAt(funcNum));
			else sendString.AppendFormat("%s\r\n",listSendString->GetAt(funcNum));
			sio_write(com_port,sendString.GetBuffer(),sendString.GetLength());
		}
		else
		{
			//说明串口已经丢失
			MessageBox("串口丢失,请关闭后重新打开","错误",MB_OK);
		}
	}
	else
	{
		//说明串口已经丢失
		MessageBox("请先打开串口","错误",MB_OK);
	}
}
#endif
void CserialCommunicationDlg::OnCbnCloseupComboSendStringConfig()
{
	// TODO: 在此添加控件通知处理程序代码
	int i = 0;
	i = ((CComboBox*)GetDlgItem(IDC_COMBO_SEND_STRING_CONFIG))->GetCurSel();//获取当前配置
	if(i == 0)
	{
		sendStringConfig = false;
	}
	else
	{
		sendStringConfig = true;
	}
}


//String2Hex的源代码
int CserialCommunicationDlg::String2Hex(char * str, char * hexdata)
{
#if 1
	//char singlechar = 0;
	int singleResult = 0;
	//int totalResult = 0;
	int tmp = 0;
	while (*str != '\0')
	{

		singleResult = *str;
		//对每个字符进行转换
		if ((singleResult >= '0') && (singleResult <= '9'))
		{
			singleResult = (*str - '0');
		}
		else
		{
			if ((singleResult >= 'a') && (singleResult <= 'f'))
			{
				singleResult = (*str - 'a') + 10;
			}
			else
			{
				if ((singleResult >= 'A') && (singleResult <= 'F'))
				{
					singleResult = (*str - 'A') + 10;
				}
				else
				{
					//如果字符不为16进制则放弃处理此字符串
					return 0;
				}
			}
		}

		//保存16进制的每一位
		if (tmp == 1)
		{
			*hexdata = (*hexdata << 4) | singleResult;
			hexdata++;
		}
		else
			*hexdata = singleResult;

		if (tmp++ >= 1)
			tmp = 0;
		//计算总的数值
		//totalResult += singleResult;
		str++;

	}
#else
	char singlechar = 0;
	int singleResult = 0;
	int totalResult = 0;
	int nlength = str.GetLength();
	if (nlength<1)
	{
		return 0;
	}

	for (int i = 0; i<nlength; i++)
	{
		singleResult = str[i];
		//对每个字符进行转换
		if ((singleResult >= '0') && (singleResult <= '9'))
		{
			singleResult = (str[i] - '0');
		}
		else
		{
			if ((singleResult >= 'a') && (singleResult <= 'f'))
			{
				singleResult = (str[i] - 'a') + 10;
			}
			else
			{
				if ((singleResult >= 'A') && (singleResult <= 'F'))
				{
					singleResult = (str[i] - 'A') + 10;
				}
				else
				{
					//如果字符不为16进制则放弃处理此字符串
					return 0;
				}
			}
		}

		//保存16进制的每一位
		hexdata.Add(singleResult);

		//计算每个字符位置的数值
		for (int j = 0; j<(nlength - 1 - i); j++)
		{
			singleResult = singleResult * 16;
		}
		//计算总的数值
		totalResult += singleResult;
	}

	return totalResult;
#endif
}

void CserialCommunicationDlg::OnSendSetColorTmp(INT_PTR index, int value)
{
	// TODO: 在此添加控件通知处理程序代码
	if (serial_isopen == TRUE)
	{
		//变量打开了,要去测试串口状态
		if (sio_lstatus(com_port) >= 0)
		{
			CString tmpstring("");
			sendString.Empty();
			 int ttt[13] = {85, 11, 2, 0, 128, 00, 00, 00, 00, 00, 243, 254};
			 char ttt1[13] = {0x75, 0x67, 0x01};
			//MessageBox(ttt);
			switch (index)
			{
				case 0x01://Red
				{
					ttt[1] = 10;
					ttt[4] = value;
					ttt[10] = ttt[11] - (ttt[1] + ttt[2] + ttt[3] + ttt[4] + ttt[5] + ttt[6] + ttt[7] + ttt[8] + ttt[9]) + 2;
				}
				break;
				case 0x02://Green
				{
					ttt[1] = 11;
					ttt[4] = value;
					ttt[10] = ttt[11] - (ttt[1] + ttt[2] + ttt[3] + ttt[4] + ttt[5] + ttt[6] + ttt[7] + ttt[8] + ttt[9]) + 2;
				}
				break;
				case 0x03://Blue
				{
					ttt[1] = 12;
					ttt[4] = value;
					ttt[10] = ttt[11] - (ttt[1] + ttt[2] + ttt[3] + ttt[4] + ttt[5] + ttt[6] + ttt[7] + ttt[8] + ttt[9]) + 2;
				}
				break;
				default:
					break;
			}
			for (int i = 0; i < 12; i++)
			{
				tmpstring.Format(_T("%02X"),ttt[i]);
				sendString += tmpstring;
			}
			//AfxMessageBox(sendString);
			int strLength = sendString.GetLength() + 1;
			char *pValue = new char[strLength];
			strncpy(pValue, sendString, strLength);
			String2Hex(pValue, ttt1);
			sio_write(com_port, ttt1, strLength/2);
			((CEdit*)GetDlgItem(IDC_EDIT_SEND))->SetWindowText(sendString.GetBuffer());
		}
		else
		{
			//说明串口已经丢失
			MessageBox("串口丢失,请关闭后重新打开", "错误", MB_OK);
		}
	}
	else
	{
		//说明串口已经丢失
		MessageBox("请先打开串口", "错误", MB_OK);
	}
}

void CserialCommunicationDlg::OnSendSaveColorTmp(INT_PTR index)
{
	// TODO: 在此添加控件通知处理程序代码
	if (serial_isopen == TRUE)
	{
		//变量打开了,要去测试串口状态
		if (sio_lstatus(com_port) >= 0)
		{
			sendString.Empty();
			char ttt[7] = { 0x07, 0x51, 0x00, 0x00, 0x00, 0x00, 0x00 };
			switch (index)
			{
			case 0x01://save namorl value
			{
				ttt[5] = 0x02;
			}
			break;
			case 0x02://save cool value
			{
				ttt[5] = 0x01;
			}
			break;
			case 0x03://save warm value
			{
				ttt[5] = 0x03;
			}
			break;
			default:
				break;
			}
			sio_write(com_port, ttt, 7);
			sendString.Format("0X%02X", ttt);
			((CEdit*)GetDlgItem(IDC_EDIT_SEND))->SetWindowText(sendString.GetBuffer());
		}
		else
		{
			//说明串口已经丢失
			MessageBox("串口丢失,请关闭后重新打开", "错误", MB_OK);
		}
	}
	else
	{
		//说明串口已经丢失
		MessageBox("请先打开串口", "错误", MB_OK);
	}
}

void CserialCommunicationDlg::OnBnClickedButtonShort_fun(INT_PTR index)
{
	// TODO: 在此添加控件通知处理程序代码
	if (serial_isopen == TRUE)
	{
		//变量打开了,要去测试串口状态
		if (sio_lstatus(com_port) >= 0)
		{
			//没有对\r\n特殊处理
			sendString.Empty();
			if (sendStringConfig == false)sendString.AppendFormat("%s", buttonSendString->GetAt(index));
			else sendString.AppendFormat("%s\r\n", buttonSendString->GetAt(index));
			char ttt[60] = { 0x69, 0x73, 0x23, 0,0,0,'\0' };
			int strLength = sendString.GetLength() + 1;
			char *pValue = new char[strLength];
			strncpy(pValue, sendString, strLength);
			String2Hex(pValue, ttt);
			sio_write(com_port, ttt, strLength/2);
			//sio_write(com_port,sendString.GetBuffer(),sendString.GetLength());
			((CEdit*)GetDlgItem(IDC_EDIT_SEND))->SetWindowText(sendString.GetBuffer());
		}
		else
		{
			//说明串口已经丢失
			MessageBox("串口丢失,请关闭后重新打开", "错误", MB_OK);
		}
	}
	else
	{
		//说明串口已经丢失
		MessageBox("请先打开串口", "错误", MB_OK);
	}
}



void CserialCommunicationDlg::OnBnClickedButton3()   //自动测试
{
	// TODO: 在此添加控件通知处理程序代码
	//<--
	WriteConfigData();
	UpdateControls();
	GetIntoData();
	//<-- Add MES
	//MessageBox(strTestProject);
	bFactoryTestAutoStart = true;// !bFactoryTestAutoStart;
	WriteMesOK = false;
	bDoubleCheckMac = false;
	if (bFactoryTestAutoStart)
	{
		GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_TEST_STOP)->EnableWindow(TRUE);
		//MessageBox("开始自动测试");

		if (serial_isopen == FALSE)
		{
			MessageBox("串口未开");
			GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
			return;
		}
		if(configFileLoadOk == FALSE)
		{
			MessageBox("配置文件未加载");
			GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
			return;
		}
		if (m_checkwb)
		{
			if (colorTmpconfigFileLoadOk == FALSE)
			{
				MessageBox("白平衡文件未加载");
				GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
				return;
			}
		}
		bTestKeypadOK = TRUE;
		bFacWriteWBAllfinished = FALSE;
		bStartCheckData = TRUE;
		cCheckCout = 0;
		bTestKeyPad_Source = FALSE;
		bTestKeyPad_Menu = FALSE;
		bTestKeyPad_Power = FALSE;
		bTestKeyPad_CHPlus = FALSE;
		bTestKeyPad_CHmins = FALSE;
		bTestKeyPad_VolPlus = FALSE;
		bTestKeyPad_VolMins = FALSE;
		//<--
		countVer = 0;
		countNet = 0;
		countMac = 0;
		countWifi = 0;
		countBlue = 0;
		TestVERBegin = FALSE;
		TestMACBegin = TRUE;
		TestATVbegin = FALSE;
		TestDTVbegin = FALSE;
		TestAVbegin	= FALSE;
		TestYPBPRbegin = FALSE;
		TestHDMI1begin = FALSE;
		TestHDMI2begin = FALSE;
		//<--
		cCheckOtherCount_tmp = 0;
		cCheckMacCount_tmp = 0;
		//cCheckKEYPADCount_tmp = 0;
		iTestSourceTimerCount = 0;
		bFacWriteWBcheck = TRUE;
		bFacWriteWBOK = FALSE;
		waitWBtimeout = 0;
		SetTimer(TIMER_GET_DATA,300,NULL); //<--MES
		//OnBnClickedButtonSend();
		//SetTimer(TIME_SHOW_MESSAGE_TEST_MAC,500,NULL);
		//SetTimer(TIME_SHOW_MESSAGE_TEST_DELAY_OTHER, iOtherTestDelay, NULL);  //<--测试使用
		//KillTimer(TIME_SHOW_MESSAGE_TEST_DELAY);
		//SetTimer(TIME_SHOW_MESSAGE_TEST_DELAY, 50, NULL);
		showString = "";
		bWaitFeedback = false;
		m_cEdit_value_use = "";
		curretBufString_test = "";
		m_cEdit_value_use_NG = "";
		m_cEdit_value_use_OK = "";
		//从资源中加载图片
		CBitmap bitmap;
		//加载指定位图资源 Bmp图片ID
		bitmap.LoadBitmap(IDB_BITMAP_LOGO);
		//获取对话框上的句柄 图片控件ID
		CStatic *p = (CStatic *)GetDlgItem(IDC_STATIC_PICTURE);
		//设置静态控件窗口风格为位图居中显示
		p->ModifyStyle(0xf, SS_BITMAP | SS_CENTERIMAGE);
		//将图片设置到Picture控件上
		p->SetBitmap(bitmap);
	}
}



void CserialCommunicationDlg::UpdateControls()
{
	GetDlgItem(IDC_COMBO_LINE)->EnableWindow(false);
	GetDlgItem(IDC_COMBO_DATA_SECCLE)->EnableWindow(false);
	GetDlgItem(IDC_COMBO_TestProject)->EnableWindow(false);
	GetDlgItem(IDC_COMBO_DATA_READ)->EnableWindow(false);
	GetDlgItem(IDC_CHECK_SN)->EnableWindow(false);
	GetDlgItem(IDC_CHECK_MAC)->EnableWindow(false);
	GetDlgItem(IDC_EDIT_SN_DATA)->EnableWindow(false);
	GetDlgItem(IDC_EDIT_MAC_DATA)->EnableWindow(false);
	//GetDlgItem(IDC_EDIT_BUILD_MESSAGE)->EnableWindow(false);
	//GetDlgItem(IDC_EDIT_SW_VERSION)->EnableWindow(false);
	//GetDlgItem(IDC_BUTTON_INTO)->EnableWindow(false);
	GetDlgItem(IDC_COMBO_MAINKEY)->EnableWindow(false);
	//GetDlgItem(IDC_EDIT_MAINKEY)->EnableWindow(false);
	GetDlgItem(IDC_EDIT_INTO_MESSAGE)->EnableWindow(true);
	m_IntoEdit.SetFocus();
}


void CserialCommunicationDlg::GetIntoData()
{
	CString strDataLength;
	intSNLen =0;
	intMACLen = 0;
	if (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_SN))
	{
		GetDlgItemText(IDC_EDIT_SN_DATA,strDataLength);
		if(strDataLength !="")
			intSNLen = _ttoi(strDataLength);
	}
	if(BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_MAC))
	{
		GetDlgItemText(IDC_EDIT_MAC_DATA,strDataLength);
		if(strDataLength !="")
			intMACLen = _ttoi(strDataLength);
	}
}

void CserialCommunicationDlg::ShowMessage(UINT iColor, CString cstrMessage, UINT idelay)
{
	if(BLACK == iColor)
		ControlID_Flag = BLACK;
	else if(RED == iColor)
		ControlID_Flag = RED;
	else if(BLUE == iColor)
		ControlID_Flag = BLUE;
	else if(GREEN == iColor)
		ControlID_Flag = GREEN;
	m_psdinfo.SetWindowText(cstrMessage);;
	UpdateData(false); 
	Sleep(idelay);
}

void CserialCommunicationDlg::OnStartTimer()//启动定时器
{
	mytimer = 0;
	SetTimer(TIMER_TIME_RUN, 1000, 0);
	ControlID_Flag = BLUE;
	SetDlgItemText(IDC_STATIC_SHOW_TIMER,_T(" 0 秒"));
	ShowMessage(BLUE,_T("开始测试!"),0);
}

void CserialCommunicationDlg::OnStopTimer()//停止定时器
{
	KillTimer(TIMER_TIME_RUN);  
}


void CserialCommunicationDlg::time_run()
{
	CString str;
	mytimer++;
	str.Format(_T("%d秒"),mytimer);
	m_time.SetWindowText(str);
}

void CserialCommunicationDlg::OnStartKeypadTimer()//启动定时器
{
	CString str;
	myKeypadtimer = 10;//<--设计倒计时
	SetTimer(TIME_SHOW_MESSAGE_KEYPAD_TIME, 1000, 0);
	ControlID_Flag = BLUE;
	str.Format(_T("按键测试即将开始，请在%d秒内按键！！"),myKeypadtimer);
	m_psdinfo.SetWindowText(str);
}

void CserialCommunicationDlg::OnStopKeypadTimer()//停止定时器
{
	KillTimer(TIME_SHOW_MESSAGE_KEYPAD_TIME);  
}

void CserialCommunicationDlg::Keypad_time_run()
{
	CString str;
	myKeypadtimer--;
	str.Format(_T("按键测试还有%d秒将结束"),myKeypadtimer);
	m_psdinfo.SetWindowText(str);
}

void CserialCommunicationDlg::WriteConfigData()
{
	CString strConfigData;
	GetDlgItemText(IDC_EDIT_ORDER,strConfigData);
	sprintf(OrderID,"%s",strConfigData);
	GetDlgItemText(IDC_COMBO_LINE,strConfigData);
	::WritePrivateProfileString(CONFIGMEG,_T("WorkcenterName"),strConfigData,m_strConfigFile);
	sprintf(WorkcenterName,"%s",strConfigData);
	GetDlgItemText(IDC_EDIT_JNUMBER,strConfigData);
	sprintf(UserName,"%s",strConfigData);
	GetDlgItemText(IDC_COMBO_DATA_SECCLE,strConfigData);
	::WritePrivateProfileString(CONFIGMEG,_T("strCheckMeg"),strConfigData,m_strConfigFile);
	strCheckMeg = strConfigData;
	GetDlgItemText(IDC_COMBO_TestProject,strConfigData);
	::WritePrivateProfileString(CONFIGMEG,_T("strTestProject"),strConfigData,m_strConfigFile);
	strTestProject = strConfigData;
	GetDlgItemText(IDC_COMBO_DATA_READ,strConfigData);
	::WritePrivateProfileString(CONFIGMEG,_T("strReadMeg"),strConfigData,m_strConfigFile);
	strReadMeg = strConfigData;
	//GetDlgItemText(IDC_CHECK_SN,strConfigData);
	//GetDlgItemText(IDC_CHECK_MAC,strConfigData);
	GetDlgItemText(IDC_EDIT_SN_DATA,strConfigData);
	if (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_SN))
		::WritePrivateProfileString(CONFIGMEG,_T("SNDataLen"),strConfigData,m_strConfigFile);
	GetDlgItemText(IDC_EDIT_MAC_DATA,strConfigData);
	::WritePrivateProfileString(CONFIGMEG, _T("MACCheck"), strConfigData, m_strConfigFile);
	MacCheck = strConfigData.MakeLower();
	//if (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_MAC))
		//::WritePrivateProfileString(CONFIGMEG,_T("MACDataLen"),strConfigData,m_strConfigFile);
	//GetDlgItemText(IDC_EDIT_BUILD_MESSAGE,strConfigData);
	//::WritePrivateProfileString(CONFIGMEG,_T("strBuildTime"),strConfigData,m_strConfigFile);
	//strBuildTime = strConfigData;
	//GetDlgItemText(IDC_EDIT_SW_VERSION,strConfigData);
	//::WritePrivateProfileString(CONFIGMEG,_T("strSWVersion"),strConfigData,m_strConfigFile);
	//strSWVersion = strConfigData;
	GetDlgItemText(IDC_COMBO_MAINKEY,strConfigData);
	::WritePrivateProfileString(CONFIGMEG,_T("strMainKey"),strConfigData,m_strConfigFile);
	strMainKey = strConfigData;
	GetDlgItemText(IDC_EDIT_MAINKEY,strConfigData);
	::WritePrivateProfileString(CONFIGMEG,_T("strMACMeg"),strConfigData,m_strConfigFile);
	strMACMeg = strConfigData;
}

void CserialCommunicationDlg::GetConfigData()
{
	CString strConfigData("");
	::GetPrivateProfileString(CONFIGMEG, _T("WorkcenterName"), _T(""), strConfigData.GetBuffer(3), 3, m_strConfigFile);
	SetDlgItemText(IDC_COMBO_LINE,strConfigData);
	::GetPrivateProfileString(CONFIGMEG, _T("strCheckMeg"), _T(""), strConfigData.GetBuffer(7), 7, m_strConfigFile);
	SetDlgItemText(IDC_COMBO_DATA_SECCLE,strConfigData);
	::GetPrivateProfileString(CONFIGMEG, _T("strTestProject"), _T(""), strConfigData.GetBuffer(7), 7, m_strConfigFile);
	SetDlgItemText(IDC_COMBO_TestProject,strConfigData);
	::GetPrivateProfileString(CONFIGMEG, _T("strReadMeg"), _T(""), strConfigData.GetBuffer(7), 7, m_strConfigFile);
	SetDlgItemText(IDC_COMBO_DATA_READ,strConfigData);
	::GetPrivateProfileString(CONFIGMEG, _T("strBuildTime"), _T(""), strConfigData.GetBuffer(20), 20, m_strConfigFile);
	//SetDlgItemText(IDC_EDIT_BUILD_MESSAGE,strConfigData);
	//::GetPrivateProfileString(CONFIGMEG, _T("strSWVersion"), _T(""), strConfigData.GetBuffer(20), 20, m_strConfigFile);
	//SetDlgItemText(IDC_EDIT_SW_VERSION,strConfigData);
	//::GetPrivateProfileString(CONFIGMEG, _T("strMainKey"), _T(""), strConfigData.GetBuffer(10), 10, m_strConfigFile);
	SetDlgItemText(IDC_COMBO_MAINKEY,strConfigData);
	::GetPrivateProfileString(CONFIGMEG, _T("strMACMeg"), _T(""), strConfigData.GetBuffer(18), 18, m_strConfigFile);
	SetDlgItemText(IDC_EDIT_MAINKEY,strConfigData);
	GetDlgItem(IDC_EDIT_MAINKEY)->EnableWindow(false);
	GetDlgItem(IDC_EDIT_MAC_DATA)->EnableWindow(false);
	GetDlgItem(IDC_EDIT_SN_DATA)->EnableWindow(false);
	/*GetDlgItemText(IDC_EDIT_ORDER,strConfigData);
	sprintf(OrderID,"%S",strConfigData);
	GetDlgItemText(IDC_COMBO_LINE,strConfigData);
	sprintf(WorkcenterName,"%S",strConfigData);
	GetDlgItemText(IDC_EDIT_JNUMBER,strConfigData);
	sprintf(UserName,"%S",strConfigData);*/
}


void CserialCommunicationDlg::OnBnClickedButtonTestStop()
{
	// TODO: 在此添加控件通知处理程序代码
	cCheckCout = 0;
	GetDlgItem(IDC_COMBO_MAINKEY)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_JNUMBER)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_ORDER)->EnableWindow(TRUE);
	GetDlgItem(IDC_CHECK_SN)->EnableWindow(TRUE);
	GetDlgItem(IDC_CHECK_MAC)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_MAC_DATA)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_SN_DATA)->EnableWindow(TRUE);
	GetDlgItem(IDC_COMBO_TestProject)->EnableWindow(TRUE);
	//GetDlgItem(IDC_COMBO_MAINKEY)->EnableWindow(TRUE);
	bStartCheckData = FALSE;
	bTestKeyPad_Source = FALSE;
	bTestKeyPad_Menu = FALSE;
	bTestKeyPad_Power = FALSE;
	bTestKeyPad_CHPlus = FALSE;
	bTestKeyPad_CHmins = FALSE;
	bTestKeyPad_VolPlus = FALSE;
	bTestKeyPad_VolMins = FALSE;
	cCheckOtherCount_tmp = 0;
	iTestSourceTimerCount = 0;
	OnStopTimer();
	//bFactoryTestAutoStart = false;
	curretBufString_test = "";
	//bWaitFeedback = TRUE;
	KillTimer(TIME_SHOW_MESSAGE_TEST_DELAY_OTHER);
	KillTimer(TIME_SHOW_MESSAGE_TEST_DELAY);
	KillTimer(TIME_SHOW_MESSAGE_KEYPAY_TEST_OTHER);
	KillTimer(TIME_SHOW_MESSAGE_TEST_MAC);
	KillTimer(TIMER_TIME_RUN);
	GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
	bFactoryTestAutoStart = FALSE;
	MessageBox("自动测试结束");
	GetDlgItem(IDC_BUTTON_TEST_STOP)->EnableWindow(FALSE);
	// TODO: 在此添加控件通知处理程序代码
	//从资源中加载图片
	CBitmap bitmap;
	//加载指定位图资源 Bmp图片ID
	bitmap.LoadBitmap(IDB_BITMAP_LOGO);
	//获取对话框上的句柄 图片控件ID
	CStatic *p = (CStatic *)GetDlgItem(IDC_STATIC_PICTURE);
	//设置静态控件窗口风格为位图居中显示
	p->ModifyStyle(0xf, SS_BITMAP | SS_CENTERIMAGE);
	//将图片设置到Picture控件上
	p->SetBitmap(bitmap);
}



#if 1
HBRUSH CserialCommunicationDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO:  在此更改 DC 的任何特性

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔.

	/*if (pWnd->GetDlgCtrlID() == IDC_TEST_RESUILT)
	{
		pDC->SetTextColor(RGB(0, 0, 255));//修改字体的颜色
		pDC->SetBkMode(TRANSPARENT);//把字体的背景变成透明的
		return m_brush;//返回背景色
	}*/


	//对特定的控件做修改
	switch (pWnd->GetDlgCtrlID())
	{
	/*	case IDC_TEST_RESUILT: //对所有静态文本控件的设置
		{
			pDC->SetBkMode(TRANSPARENT);
			//设置背景为透明
			pDC->SetTextColor(RGB(0, 0, 255)); //设置字体颜色
			pWnd->SetFont(cFont); //设置字体
			HBRUSH B = CreateSolidBrush(RGB(255, 0, 255));
			//创建画刷
			return (HBRUSH)B; //返回画刷句柄
		}
		case IDC_BUTTON3:
		{
			pDC->SetBkMode(TRANSPARENT);
			pDC->SetTextColor(RGB(0, 255, 255));
			//pWnd->SetFont(cFont);
			HBRUSH B = CreateSolidBrush(RGB(255, 0, 255));
			return (HBRUSH)B;
		}*/
// 		case IDC_EDIT1: //对所有编辑框的设置
// 		{
// 		//	pDC->SetBkMode(TRANSPARENT);
// 			pDC->SetTextColor(RGB(0, 0, 0));
// 			pDC->SetBkMode(TRANSPARENT);
// 			pDC->SelectObject(&m_Font_middle);
// 		//	pWnd->SetFont(cFont);
// 		//	HBRUSH B = CreateSolidBrush(RGB(255, 0, 255));
// 			return m_brush;
// 		}
		case IDC_EDIT_OK: //对所有编辑框的设置
		{
		//	pDC->SetBkMode(TRANSPARENT);
			pDC->SetTextColor(RGB(0, 255, 0));
			pDC->SetBkMode(TRANSPARENT);
			pDC->SelectObject(&m_Font_middle);
		//	pWnd->SetFont(cFont);
		//	HBRUSH B = CreateSolidBrush(RGB(255, 0, 255));
			return m_brush;
		}
		case IDC_EDIT_FAILED: //对所有编辑框的设置
		{
		//	pDC->SetBkMode(TRANSPARENT);
			pDC->SetTextColor(RGB(255, 0, 0));
			pDC->SetBkMode(TRANSPARENT);
			pDC->SelectObject(&m_Font_middle);
		//	pWnd->SetFont(cFont);
		//	HBRUSH B = CreateSolidBrush(RGB(255, 0, 255));
			return m_brush;
		}
		case IDC_EDIT_TESTING: //对所有编辑框的设置
		{
			//	pDC->SetBkMode(TRANSPARENT);
			pDC->SetTextColor(RGB(0, 0, 255));
			pDC->SetBkMode(TRANSPARENT);
			pDC->SelectObject(&m_Font);
			//	pWnd->SetFont(cFont);
			//	HBRUSH B = CreateSolidBrush(RGB(255, 0, 255));
			return m_brush;
		case IDC_STATIC_SHOW_TIMER:
		{
			if(BLACK == ControlID_Flag)
				pDC->SetTextColor(RGB(0,0,0));
			else if(RED == ControlID_Flag)
				pDC->SetTextColor(RGB(255,0,0));
			else if(BLUE == ControlID_Flag)
				pDC->SetTextColor(RGB(0,0,255));
			else if(GREEN == ControlID_Flag)
				pDC->SetTextColor(RGB(0,0,255));
			else
				pDC->SetTextColor(RGB(0,0,255));
			return m_brush;
		}
		case IDC_STATIC_SHOW_MESSAGE:
			{
				if(BLACK == ControlID_Flag)
					pDC->SetTextColor(RGB(0,0,0));
				else if(RED == ControlID_Flag)
					pDC->SetTextColor(RGB(255,0,0));
				else if(BLUE == ControlID_Flag)
					pDC->SetTextColor(RGB(0,0,255));
				else if(GREEN == ControlID_Flag)
					pDC->SetTextColor(RGB(0,0,255));
				else
					pDC->SetTextColor(RGB(0,0,255));
				return m_brush;
			}
		}
	default:
		return hbr;// CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
}
#endif



void CserialCommunicationDlg::OnBnClickedButtonLoadConfigFile2()//load 色温配置
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog dlg(TRUE, NULL, NULL, NULL, NULL);
	if (dlg.DoModal() == IDOK)//
	{
		CString str, str1;
		str = dlg.GetPathName();
		str1 = dlg.GetFileName();
		CEdit* cfolder;
		cfolder = (CEdit*)GetDlgItem(IDC_EDIT_OK);
		cfolder->SetWindowText(str + "  name:" + str1);
		str1.MakeLower();
		if (str1.Find(".txt") == -1)
		{
			MessageBox("无效文件");
			return;
		}
		/*if (str1.Find("色温") == -1)//文件名要带色温两个字
		{
			MessageBox("无效文件");
			return;
		}*/
		if ((str1.Find("rgb_value") == -1) && (str1.Find("色温") == -1))//文件名要带色温两个字
		{
			MessageBox("无效文件");
			return;
		}
		long Filelen;
		UCHAR* a_pData;
		FILE* fp = NULL;
		errno_t err;
		char *pBuff = str.GetBuffer(0);
		if ((err = fopen_s(&fp, pBuff, "rb")) == 0)//(fp=fopen_s(pBuff,"rb"))       //str是我的.bin 文件路径
		{
			fseek(fp, 0, SEEK_END);
			Filelen = ftell(fp);
			//m_lFilelen = Filelen;
			//分配内存
			a_pData = (UCHAR*)malloc(Filelen);
			//u_newData = (UCHAR*)malloc(Filelen);
			//读取文件到分配的内存
			fseek(fp, 0, SEEK_SET);
			fread(a_pData, 1, Filelen, fp);
			fclose(fp);
			
			CString sResult;

			sResult.Format("%s", a_pData);
			m_strEdit_colorTmp = sResult;
			//m_strEdit_colorTmp = "";
			int n = 0;
			int colorIndex = 0;
			for (int j = 0; j < 3; j++)
			{
				m_stColorTmp[j].RDrv = 0;
				m_stColorTmp[j].GDrv = 0;
				m_stColorTmp[j].BDrv = 0;
			}

			//a_pData += n;
			char tmp[4] = {0,0,0,0};
			int trueValue =0;
			int p = 0;

			while ((n<=Filelen))//&&(*a_pData!='\0')
			{
				if ((*a_pData != '\r')&& (*a_pData != '\0'))
				{
					if ((*a_pData >= '0') && (*a_pData <= '9'))
					{
						tmp[p++] = *a_pData;

					}
				}
				else
				{
					p = 0;
					if (atoi(tmp) == 0)
					{
						tmp[0] = 0;
						tmp[1] = 0;
						tmp[2] = 0;
						tmp[3] = 0;
					}
					else
					{
						switch (colorIndex)
						{
							case 0:
								m_stColorTmp[trueValue].RDrv = atoi(tmp);
								//sResult.Format("%d", m_stColorTmp[trueValue].RDrv);
								//m_strEdit_colorTmp += "\r\nRed=" + sResult;
								break;
							case 1:
								m_stColorTmp[trueValue].GDrv = atoi(tmp);
								//sResult.Format("%d", m_stColorTmp[trueValue].GDrv);
								//m_strEdit_colorTmp += "\r\nGreen=" + sResult;
								break;
							case 2:
								m_stColorTmp[trueValue].BDrv = atoi(tmp);
								//sResult.Format("%d", m_stColorTmp[trueValue].BDrv);
								//m_strEdit_colorTmp += "\r\nBlue=" + sResult;
								trueValue++;
								//colorIndex = 0;
								break;
							default:
								break;
						}
						tmp[0] = 0;
						tmp[1] = 0;
						tmp[2] = 0;
						tmp[3] = 0;
						colorIndex++;
						if (colorIndex >= 3)
							colorIndex = 0;
						if (trueValue >= 3)
							break;
					}
				}
				a_pData++;
				n++;
			}
			UpdateData(false);
			colorTmpconfigFileLoadOk = true;
			//GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
			//m_lFilelen = 256;
			//EDID_data_Fresh_txt(a_pData);
			//int size = sResult.GetLength();
			//file.Write(sResult,size);
			//file.Flush();
			//file.Close();
		}
	}
}


void CserialCommunicationDlg::OnBnClickedWB()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if(m_checkwb)
	{
		GetDlgItem(IDC_BUTTON_LOAD_CONFIG_FILE2)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_BUTTON_LOAD_CONFIG_FILE2)->EnableWindow(FALSE);
	}
}

void CserialCommunicationDlg::OCbnSetfocusComboDelay()
{
	m_CComboBox_delay.ResetContent();
	m_CComboBox_delay.AddString("100ms");
	m_CComboBox_delay.AddString("200ms");
	m_CComboBox_delay.AddString("300ms");
	m_CComboBox_delay.AddString("400ms");
	m_CComboBox_delay.AddString("500ms");
}
void CserialCommunicationDlg::OnCbnSelchangeComboDelay()
{
	// TODO: 在此添加控件通知处理程序代码
	int i = 0;
	i = m_CComboBox_delay.GetCurSel();
	switch (i)
	{
	case 0:
		iOtherTestDelay = 100;
		m_strOtherTestDelay = "100ms";
		break;
	case 1:
		iOtherTestDelay = 200;
		m_strOtherTestDelay = "200ms";
		break;
	case 2:
		iOtherTestDelay = 300;
		m_strOtherTestDelay = "300ms";
		break;
	case 3:
		iOtherTestDelay = 400;
		m_strOtherTestDelay = "400ms";
		break;
	case 4:
		iOtherTestDelay = 500;
		m_strOtherTestDelay = "500ms";
		break;
	}
	::WritePrivateProfileString("COMM", "otherTestDelay", m_strOtherTestDelay, m_strConfigFile);
}


void CserialCommunicationDlg::OnBnClickedButtonSetup()
{
	// TODO: 在此添加控件通知处理程序代码
	//if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
}













