#ifndef TRADE_INTERFACE_H
#define TRADE_INTERFACE_H

#include <string>
#include "t2sdk_interface.h"

struct stTradeInfo
{
	int entrust_no;              //ί�б��
	std::string business_id;     //�ɽ����
	int	business_amount;         //�ɽ�����
	double opt_business_price;   //�ɽ��۸�
	int business_time;           //�ɽ�ʱ��
	int	report_no;               //ί�п��¼��
	char entrust_status;         //ί��״̬
};

struct stOrderInfo
{
	int entrust_no;             //ί�б��
	std::string exchange_type;  //�������        
	std::string option_code;    //��Ȩ��Լ����
	char entrust_bs;            //��������
	char entrust_oc;            //��ƽ�ַ���
	char covered_flag;          //���ұ�־
	char entrust_status;        //ί��״̬
	int	entrust_amount;         //ί������
	double	opt_entrust_price;  //ί�м۸�
	int	report_no;              //������
	int	batch_no;               //ί������
};

class ITradeSink  
{  
public:  
	//����
	virtual void OnError(const std::string& error_msg) = 0;
	//��½����
	virtual void OnLoginProcess(std::string& tips, bool status = true) = 0;
	//ί�лر�
	virtual void OnOrder(const stOrderInfo& order_info) = 0;
	//�ɽ��ر�  �����ϵ�/������  ��Ӧ���� 33011����
	virtual void OnTrade(const stTradeInfo& trade_info) = 0;
}; 

class TradeInterface;

class CSubCallback : public CSubCallbackInterface
{
public:
	// ��ΪCSubCallbackInterface�����մ��������IKnown��������Ҫʵ��һ����3������
	unsigned long  FUNCTION_CALL_MODE QueryInterface(const char *iid, IKnown **ppv);
	unsigned long  FUNCTION_CALL_MODE AddRef();
	unsigned long  FUNCTION_CALL_MODE Release();

	void FUNCTION_CALL_MODE OnReceived(CSubscribeInterface *lpSub,int subscribeIndex, const void *lpData, int nLength,LPSUBSCRIBE_RECVDATA lpRecvData);
	void FUNCTION_CALL_MODE OnRecvTickMsg(CSubscribeInterface *lpSub,int subscribeIndex,const char* TickMsgInfo);

	void SetRequestMode(TradeInterface* lpMode) {lpReqMode = lpMode;}
private:
	TradeInterface* lpReqMode;
};

// �Զ�����CCallback��ͨ���̳У�ʵ�֣�CCallbackInterface�����Զ�������¼����������ӳɹ���
// ���ӶϿ���������ɡ��յ����ݵȣ�����ʱ�Ļص�����
class CCallback : public CCallbackInterface 
{
public:
	// ��ΪCCallbackInterface�����մ��������IKnown��������Ҫʵ��һ����3������
	unsigned long  FUNCTION_CALL_MODE QueryInterface(const char *iid, IKnown **ppv);
	unsigned long  FUNCTION_CALL_MODE AddRef();
	unsigned long  FUNCTION_CALL_MODE Release();

	// �����¼�����ʱ�Ļص�������ʵ��ʹ��ʱ���Ը�����Ҫ��ѡ��ʵ�֣����ڲ���Ҫ���¼��ص���������ֱ��return
	// Reserved?Ϊ����������Ϊ�Ժ���չ��׼����ʵ��ʱ��ֱ��return��return 0��
	void FUNCTION_CALL_MODE OnConnect(CConnectionInterface *lpConnection);
	void FUNCTION_CALL_MODE OnSafeConnect(CConnectionInterface *lpConnection);
	void FUNCTION_CALL_MODE OnRegister(CConnectionInterface *lpConnection);
	void FUNCTION_CALL_MODE OnClose(CConnectionInterface *lpConnection);
	void FUNCTION_CALL_MODE OnSent(CConnectionInterface *lpConnection, int hSend, void *reserved1, void *reserved2, int nQueuingData);
	void FUNCTION_CALL_MODE Reserved1(void *a, void *b, void *c, void *d);
	void FUNCTION_CALL_MODE Reserved2(void *a, void *b, void *c, void *d);
	int  FUNCTION_CALL_MODE Reserved3();
	void FUNCTION_CALL_MODE Reserved4();
	void FUNCTION_CALL_MODE Reserved5();
	void FUNCTION_CALL_MODE Reserved6();
	void FUNCTION_CALL_MODE Reserved7();
	void FUNCTION_CALL_MODE OnReceivedBiz(CConnectionInterface *lpConnection, int hSend, const void *lpUnPackerOrStr, int nResult);
	void FUNCTION_CALL_MODE OnReceivedBizEx(CConnectionInterface *lpConnection, int hSend, LPRET_DATA lpRetData, const void *lpUnpackerOrStr, int nResult);
	void FUNCTION_CALL_MODE OnReceivedBizMsg(CConnectionInterface *lpConnection, int hSend, IBizMessage* lpMsg);

	void SetRequestMode(TradeInterface* lpMode) {lpReqMode = lpMode;}
private:
	//331100 ����
	void OnResponse_331100(IF2UnPacker *lpUnPacker);
private:
	TradeInterface* lpReqMode;
};

class TradeInterface
{
	friend CCallback;
	friend CSubCallback;
public:
	TradeInterface(ITradeSink* pSink);
	~TradeInterface();

	int Login(const std::string& account, const std::string& password);

	//����-������Ȩ�ɽ��ر� 33011 33012
	int Subscribe(char* topicName);

	int SendOrder(const stOrderInfo& order_info);
	int CancelOrder(const std::string &entrust_no);

private:
	bool GetMacByGetAdaptersAddresses(std::string& macOUT);
	int InitConn();
	//331100 �˻���¼
	int ReqFunction331100();
	//338011 ��Ȩί�� 
	int ReqFunction338011(char* exchange_type,char* option_code, int entrust_amount,double entrust_price,char entrust_bs, char entrust_oc, char covered_flag = ' ', const std::string &entrust_prop = "0");
	//338012��Ȩ����
	int ReqFunction338012(const std::string &entrust_no);

	void SetUserToken(const char * pUserToken){strncpy_s(m_opUserToken , pUserToken, sizeof(m_opUserToken));};
	void SetBranchNo(int BranchNo) { m_BranchNo = BranchNo;}
	void SetClientId(const char * pClientId){strncpy_s(m_client_id, pClientId, sizeof(m_client_id));}
	void SetSystemNo(int SystemNo) {iSystemNo = SystemNo;}

private:
	ITradeSink* m_pSink;

	// <���� ���Ի�����ϵͳ�����淶>
	char m_EntrustWay;
	std::string m_opStation; 

	std::string m_AccountName; //maxlength 30
	std::string m_Password;  //maxlength 15

	CConfigInterface* lpConfig;
	CConnectionInterface *lpConnection;
	CSubscribeInterface * lpSub;

	CCallback callback;
	CSubCallback subCallback;
private:
	char m_opUserToken[512];
	int m_BranchNo;
	char m_client_id[18];
	int iSystemNo;
};

#endif