// serialCommunicationDlg.h : 头文件
//

#pragma once
#include "PCOMM.H"
#include "cpptooltip_src/PPTooltip.h"
#include "tinyxml2.h"
#include <afxbutton.h>
#include "afxwin.h"
/////////////////////////////////
#define TIMER_ORDER	0	
#define TIMER_GET_DATA	1
//#define TIMER_MAC	2
#define TIMER_TIME_RUN	3

#define SN_DATA		0
#define MAC_DATA	1
#define SNMAC_DATA	2

#define BLACK		0
#define RED			1
#define BLUE		2
#define GREEN		3
/////////////////////////////
using namespace tinyxml2;

typedef struct TAG_CONFIG_LABEL {
	int index; 
	CString source; 
	int btime;
	bool bhaveSignal;
} CONFIG_LABEL;

typedef struct TAG_COLORTMP_LABEL {
	int RDrv;
	int GDrv;
	int BDrv;
} COLORTMP_LABEL;
// CserialCommunicationDlg 对话框
class CserialCommunicationDlg : public CDialog
{
// 构造
public:
	CserialCommunicationDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SERIALCOMMUNICATION_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

private:
	UINT8 baud;
	UINT8 stop_bit;
	UINT8 check_mode;
	UINT8 data_length;
	BOOL serial_isopen;
	UINT8 com_port;
	BOOL sendlrcr;
	CArray<CString> strPath;			//文件名
	CPPToolTip m_toolstip;

	bool sendStringConfig;

	tinyxml2::XMLDocument configXmlFile;
	bool configFileLoadOk;
	bool colorTmpconfigFileLoadOk;
	CStringArray* buttonSendString;//存放按钮的发送字符串
	CStringArray* listSendString;	//存放list中的发送字符串
	bool bWaitFeedback;
	bool bKeypadStart;
	char cCheckCout;
	int iCurretIndex;
	int waitWBtimeout;
	bool bFacWriteWBcheck;
	bool bFacWriteWBOK;
	bool bFacWriteWBAllfinished;
	bool bTestKeyPad_Source;
	bool bTestKeyPad_Menu;
	bool bTestKeyPad_Power;
	bool bTestKeyPad_CHPlus;
	bool bTestKeyPad_CHmins;
	bool bTestKeyPad_VolPlus;
	bool bTestKeyPad_VolMins;
	bool TestVERBegin;
	bool TestWifiBegin;      //<--
	bool TestAddVoice;
	bool TestCHVoiceBegin;   //<--声音80开关
	bool TestBlueBegin;
	bool TestUsbBegin;
	bool TestNetBegin;
	bool TestMACBegin;
	bool TestMACSend;
	bool TestVoiceBegin;
	bool TestWifiSend;
	bool TestBlueSend;
	bool TestUsbSend;
	bool TestNetSend;		
	bool TestDTVbegin;
	bool TestAVbegin;
	bool TestATVbegin;	
	bool TestYPBPRbegin;
	bool TestHDMI1begin;
	bool TestHDMI2begin;
	bool TestHDMI3begin;
	bool bGetTvMacBegin;//<--
	BOOL WriteMesOK;
	//<--白平衡
	bool bFacWriteWB_Normal_Red;
	bool bFacWriteWB_Normal_Green;
	bool bFacWriteWB_Normal_Blue;
	bool bFacWriteWB_Cool_Red;
	bool bFacWriteWB_Cool_Green;
	bool bFacWriteWB_Cool_Blue;
	bool bFacWriteWB_Warm_Red;
	bool bFacWriteWB_Warm_Green;
	bool bFacWriteWB_Warm_Blue;
	//<--
	bool boolSnScanDataOK;
	bool bisSevenkey;
	bool bisOnekey;
	char cCheckCout_tmp;
	bool bStartCheckData;
	char cCheckOtherCount_tmp;
	char cCheckKEYPADCount_tmp;
	char cCheckMacCount_tmp;
	int iTestSourceTimerCount;
	CString m_strConfigFile;//串口配置	
	CString	m_port;
	CString	m_baud;
	CString strTime;
	CString stMac;
	CString GetMacData;
	CString CheckData;
	int m_checkwb;
	CBrush m_brush;
	int iOtherTestDelay;
	CString strLastSource;
public:
	CString sMacAddr_config;
	CString sMacAddr;
	CString curretBufString_test;
	CString serialReadTemp;
	CString strHex;
	CString showString;
	CString sendString;
	CString sVersion;
	CString sVersion_config;
	CString sWifi_config;
	bool bFactoryTestOk;
	CONFIG_LABEL SourceConfig[20];
	//bool bFactoryTestAuto;
	bool bFactoryTestAutoStart;
	bool bDoubleCheckMac;
	COLORTMP_LABEL m_stColorTmp[3];
	//CBrush m_brush;
	CFont m_Font_Small;
	CFont m_Font;
	CFont m_Font_middle;
	CFont m_Font_Big;
	bool bTestKeypadOK;
	int mKeyPadPower;
	int mKeypadSource;
	int mKeyPadLeft;
	int mKeyPadRight;
	int mKeyPadDown;
	int mKeyPadUp;
	int mKeyPadMenu;
private:
	void SerialsProcessBuffer( void );
	CRect m_rect;
// 实现
protected:
	HICON m_hIcon;
	HICON  m_hIcon2;
	//BYTE bTestKeypad;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	void Readconfig();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSendSetColorTmp(INT_PTR index, int value);
	afx_msg void OnSendSaveColorTmp(INT_PTR index);
	afx_msg void SerialsCheckBuffer(CString curretBufString);
	afx_msg void OnCbnSetfocusComboPortSelect();
	afx_msg void OnCbnCloseupComboBaudSelect();
	afx_msg void OnCbnCloseupComboDataBitSelect();
	afx_msg void OnCbnCloseupComboStopSelect();
	afx_msg void OnCbnCloseupComboCheckSelect();
	afx_msg void OnBnClickedButtonSerialControl();
	afx_msg void OCbnSetfocusComboDelay();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonReceiveClear();
	afx_msg void OnBnClickedButtonReceiveSaveFile();
	afx_msg void OnBnClickedButtonSend();
	afx_msg void OnBnClickedButtonSendClear();
	afx_msg void OnBnClickedButtonSendFile();
	afx_msg void OnCbnSetfocusComboConfigFileSelect();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonLoadConfigFile();
	afx_msg void OnBnClickedButtonSendShortFuction();
	afx_msg void OnBnClickedButtonSendSelectShort();
	afx_msg void OnCbnCloseupComboSendStringConfig();
	int String2Hex(char * str, char * hexdata);
	void OnBnClickedButtonShort_fun(INT_PTR index);
	void ReceiveSaveMacSuccessFile(CString cMessage);
	void ReceiveSaveMacErrorFile(CString cMessage);
	CString m_cEdit_value_use;
	CString m_cEdit_value_use_OK;
	CString m_cEdit_value_use_NG;
	afx_msg void LoadConfigXMLFile();
	afx_msg void OnBnClickedButtonControl();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButtonTestStop();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnStnClickedStaticPicture();
	afx_msg void OnBnClickedButtonLoadConfigFile2();
	CString m_strEdit_colorTmp;
	afx_msg void OnBnClickedWB();
	CMFCButton m_cMfcButton_SOURCE;
	CMFCButton m_cMfcButton_CHP;
	CMFCButton m_cMfcButton_CHM;
	CMFCButton m_cMfcButton_VOLP;
	CMFCButton m_cMfcButton_VOLM;
	CMFCButton m_cMfcButton_MENU;
	CMFCButton m_cMfcButton_POWER;
	CComboBox m_CComboBox_delay;
	afx_msg void OnCbnSelchangeComboDelay();
	CString m_strOtherTestDelay;
	afx_msg void SerialReConnect();
	CMFCButton m_MFCButton_keypadResuilt;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//CMFCButton m_CMFButton_Time;
	afx_msg void OnBnClickedButtonSetup();
	afx_msg void OnBnClickedMfcbuttonKeypadResuit();
	//add for MES
	void parserADCCheckorSubmitXML(char* parserxml);
	void parserADCGetInterfaceParameterXML(char* PostCode, char* parserxml);
	CString ADCGetResourceName();
	BOOL GenTestDate(int MacInput);//MacInput参数预值为0或1.1代表是需要将MAC写入MES，0代表否
	afx_msg BOOL ADCCheckLotSN(char* PostCode, char* LotSN, char* FieldName);
	afx_msg BOOL ADCGetInterfaceParameter(char* PostCode, char* LotSN, char* FieldName);
	afx_msg BOOL ADCSubmitTestData(char* PostCode, char* LotSN, char* FieldName,int MacInput);//MacInput表示是否需要将MAC写入MES
public: //<--add for MES
	CComboBox m_CCombox_Line;// 选择线体

	CComboBox m_CCombox_model;// 机型

	CComboBox m_CCombox_testproject;// 选择操作动作

	CListCtrl m_list;// list control的控件变量
	CImageList     m_Image;


	CEdit m_userid;// 工号

	CEdit m_orderid;// 订单号
	CStatic m_psdinfo;//打印提示信息的静态文本框



	CEdit m_IntoEdit;// USB扫描输入的SN

	int Order_Flag;//用来标识订单号是第几次输入
	CString cstrHelp;//用来帮助判断两次订单号是否一致

	CButton m_Button_in;// 点击进入按钮

	CEdit m_Check_MAC1;// 检查MAC地址中的前几位

	CEdit m_Check_MAC2;// 检查MAC地址前几位的第二个对话框

	CEdit m_SN_length;// SN的长度-数据长度-检查扫描的SN合法性

	CEdit m_public_data;// 公共数据--检查SN中都包含的一部分

	////////////////////////////////////////////
	UINT	m_uTimerPsd;           // 监控员工工号和订单号输入的定时器
	UINT	m_uTimerGetData;	   //一个定时器用来检测USB扫描枪扫描到的数据
	INT SN_Handle();  //定义一个函数对扫描到的SN进行处理判断，该函数用于监控SN的定时器中
	CString Old_SN;//用来判断存储扫描到的SN  判断是否是新扫描到的SN
	UINT	m_uTimerGetMAC;			//一个定时器用来检测获取电视机MAC
	INT MAC_Handle();//用来在定时器中对MAC处理
	INT MAC_SN_flag;//一个标记标志着MAC和SN只能一对一处理，防止一个MAC对应了两个SN的情况出现
	CString MAC1;
	CString MAC2;
	CString MacCheck;
	CString strTestProject;
	int number;//统计有多少成功写入MES，并显示
	UINT intSNLen,intMACLen;
	CString m_strSNData,m_strMACData;
	CWinThread *myThread;
	UINT ControlID_Flag;
	CFont m_ftCtrl;
	//CString m_strConfigFile; //配置文件
	CString m_CutData;
	///////////////////////////////////////////////
	UINT	m_uTimer_RUN;
	void CserialCommunicationDlg::time_run();
	void CserialCommunicationDlg::WriteConfigData();
	void CserialCommunicationDlg::UpdateControls();
	void CserialCommunicationDlg::GetIntoData();
	void CserialCommunicationDlg::ScanData();
	BOOL CserialCommunicationDlg::DataVerification(CString strVerData, UINT intFlag);
	void CserialCommunicationDlg::GetConfigData();
	void CserialCommunicationDlg::ShowMessage(UINT iColor, CString cstrMessage, UINT idelay);
	void CserialCommunicationDlg::InitData();
	void CserialCommunicationDlg::OnStartTimer();
	void CserialCommunicationDlg::OnStopTimer();
	void CserialCommunicationDlg::WriteResultMES(CString strVerData, int MacInput);
	bool CserialCommunicationDlg::InforCheck(CString strTVMeg);
	void CserialCommunicationDlg::FontSetting();
	void CserialCommunicationDlg::ReSize(void);
	int mytimer;
	CStatic m_time;// 显示跑秒，表示软件在运行中
	//<--KEYPAD定时器
	void CserialCommunicationDlg::OnStartKeypadTimer();
	void CserialCommunicationDlg::OnStopKeypadTimer();
	void CserialCommunicationDlg::Keypad_time_run();
	int myKeypadtimer;
	int countVer;
	int countNet;
	int countMac;
	int countWifi;
	int countBlue;
	int countMacPass;
	//CStatic m_Keypadtime;// 显示跑秒，表示软件在运行中
	//<--
	CComboBox m_CComBoMainKey;
	//afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonInto();
	CComboBox m_Combox_Read;
	afx_msg void OnBnClickedCheckSn();
	afx_msg void OnBnClickedCheckMac();
	//afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCbnSelchangeComboWr();
};
