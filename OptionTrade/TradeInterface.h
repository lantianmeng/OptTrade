#ifndef TRADE_INTERFACE_H
#define TRADE_INTERFACE_H

#include <string>
#include "t2sdk_interface.h"

struct stTradeInfo
{
	int entrust_no;              //委托编号
	std::string business_id;     //成交编号
	int	business_amount;         //成交数量
	double opt_business_price;   //成交价格
	int business_time;           //成交时间
	int	report_no;               //委托库记录号
	char entrust_status;         //委托状态
};

struct stOrderInfo
{
	int entrust_no;             //委托编号
	std::string exchange_type;  //交易类别        
	std::string option_code;    //期权合约编码
	char entrust_bs;            //买卖方向
	char entrust_oc;            //开平仓方向
	char covered_flag;          //备兑标志
	char entrust_status;        //委托状态
	int	entrust_amount;         //委托数量
	double	opt_entrust_price;  //委托价格
	int	report_no;              //申请编号
	int	batch_no;               //委托批号
};

class ITradeSink  
{  
public:  
	//错误
	virtual void OnError(const std::string& error_msg) = 0;
	//登陆过程
	virtual void OnLoginProcess(std::string& tips, bool status = true) = 0;
	//委托回报
	virtual void OnOrder(const stOrderInfo& order_info) = 0;
	//成交回报  包含废单/撤单等  对应恒生 33011主推
	virtual void OnTrade(const stTradeInfo& trade_info) = 0;
}; 

class TradeInterface;

class CSubCallback : public CSubCallbackInterface
{
public:
	// 因为CSubCallbackInterface的最终纯虚基类是IKnown，所以需要实现一下这3个方法
	unsigned long  FUNCTION_CALL_MODE QueryInterface(const char *iid, IKnown **ppv);
	unsigned long  FUNCTION_CALL_MODE AddRef();
	unsigned long  FUNCTION_CALL_MODE Release();

	void FUNCTION_CALL_MODE OnReceived(CSubscribeInterface *lpSub,int subscribeIndex, const void *lpData, int nLength,LPSUBSCRIBE_RECVDATA lpRecvData);
	void FUNCTION_CALL_MODE OnRecvTickMsg(CSubscribeInterface *lpSub,int subscribeIndex,const char* TickMsgInfo);

	void SetRequestMode(TradeInterface* lpMode) {lpReqMode = lpMode;}
private:
	TradeInterface* lpReqMode;
};

// 自定义类CCallback，通过继承（实现）CCallbackInterface，来自定义各种事件（包括连接成功、
// 连接断开、发送完成、收到数据等）发生时的回调方法
class CCallback : public CCallbackInterface 
{
public:
	// 因为CCallbackInterface的最终纯虚基类是IKnown，所以需要实现一下这3个方法
	unsigned long  FUNCTION_CALL_MODE QueryInterface(const char *iid, IKnown **ppv);
	unsigned long  FUNCTION_CALL_MODE AddRef();
	unsigned long  FUNCTION_CALL_MODE Release();

	// 各种事件发生时的回调方法，实际使用时可以根据需要来选择实现，对于不需要的事件回调方法，可直接return
	// Reserved?为保留方法，为以后扩展做准备，实现时可直接return或return 0。
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
	//331100 登入
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

	//主推-个股期权成交回报 33011 33012
	int Subscribe(char* topicName);

	int SendOrder(const stOrderInfo& order_info);
	int CancelOrder(const std::string &entrust_no);

private:
	bool GetMacByGetAdaptersAddresses(std::string& macOUT);
	int InitConn();
	//331100 账户登录
	int ReqFunction331100();
	//338011 期权委托 
	int ReqFunction338011(char* exchange_type,char* option_code, int entrust_amount,double entrust_price,char entrust_bs, char entrust_oc, char covered_flag = ' ', const std::string &entrust_prop = "0");
	//338012期权撤单
	int ReqFunction338012(const std::string &entrust_no);

	void SetUserToken(const char * pUserToken){strncpy_s(m_opUserToken , pUserToken, sizeof(m_opUserToken));};
	void SetBranchNo(int BranchNo) { m_BranchNo = BranchNo;}
	void SetClientId(const char * pClientId){strncpy_s(m_client_id, pClientId, sizeof(m_client_id));}
	void SetSystemNo(int SystemNo) {iSystemNo = SystemNo;}

private:
	ITradeSink* m_pSink;

	// <中信 个性化交易系统技术规范>
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