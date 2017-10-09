#include "stdafx.h"  
#include "TradeInterface.h"
#include <Winsock2.h>
#include <iphlpapi.h>
#include "constant.h"
#include "EasyLog.h"

#define EASYLOG

TradeInterface::TradeInterface(ITradeSink* pSink)  
	:m_pSink(pSink),
	m_EntrustWay('3'),
	m_opStation(""),
	lpConfig(NULL),
	lpConnection(NULL),
	lpSub(NULL)
{  
	memset(m_opUserToken, 0 , 512);
	m_BranchNo = -1;
	memset(m_client_id, 0 , 18);
	iSystemNo = -1;

	callback.SetRequestMode(this);
	subCallback.SetRequestMode(this);
}  

TradeInterface::~TradeInterface()
{
	callback.SetRequestMode(NULL);
	subCallback.SetRequestMode(NULL);

	if (lpConnection)
	{
		lpConnection->Release();
	}

	if (lpConfig)
	{
		lpConfig->Release();
	}

	if (lpSub)
	{
		//释放订阅端
		lpSub->Release();
	}
}

bool TradeInterface::GetMacByGetAdaptersAddresses(std::string& macOUT)
{
	bool ret = false;
	ULONG outBufLen = sizeof(IP_ADAPTER_ADDRESSES);
	PIP_ADAPTER_ADDRESSES pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);

	if (pAddresses == NULL) 
		return false;

	// Make an initial call to GetAdaptersAddresses to get the necessary size into the ulOutBufLen variable
	if(GetAdaptersAddresses(AF_UNSPEC, 0, NULL, pAddresses, &outBufLen) == ERROR_BUFFER_OVERFLOW)
	{
		free(pAddresses);

		pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
		if (pAddresses == NULL) 
			return false;
	}

	if(GetAdaptersAddresses(AF_UNSPEC, 0, NULL, pAddresses, &outBufLen) == NO_ERROR)
	{
		// If successful, output some information from the data we received
		for(PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses; pCurrAddresses != NULL; pCurrAddresses = pCurrAddresses->Next)
		{
			// 确保MAC地址的长度为 00-00-00-00-00-00
			if(pCurrAddresses->PhysicalAddressLength != 6)
				continue;

			char acMAC[32];
			sprintf(acMAC, "%02X-%02X-%02X-%02X-%02X-%02X",
				int (pCurrAddresses->PhysicalAddress[0]),
				int (pCurrAddresses->PhysicalAddress[1]),
				int (pCurrAddresses->PhysicalAddress[2]),
				int (pCurrAddresses->PhysicalAddress[3]),
				int (pCurrAddresses->PhysicalAddress[4]),
				int (pCurrAddresses->PhysicalAddress[5]));

			macOUT = acMAC;
			ret = true;
			break;
		}
	} 

	free(pAddresses);
	return ret;
}

int TradeInterface::InitConn()
{
	std::string sMac = "";
	if(!GetMacByGetAdaptersAddresses(sMac))
	{
		m_pSink->OnLoginProcess(std::string("InitConn fail!get Mac Address error!"), false);
		return -1;
	}
	//m_opStation.append("TYJR-YTTZ- IIP.");
	//m_opStation.append("59.174.29.158");
	m_opStation.append("TYJR-YTTZ");
	//m_opStation.append("-LIP.");
	//m_opStation.append(ipAddresses[0]);
	m_opStation.append("-MAC.");
	m_opStation.append(sMac);
	//m_opStation.append(hostName);

	lpConfig = NewConfig();
	lpConfig->AddRef();
	lpConfig->Load("t2sdk.ini");

	int iRet = 0;

	if(lpConnection != NULL)
	{
		lpConnection->Release();
		lpConnection = NULL;
	}

	lpConnection = NewConnection(lpConfig);
	//cout<<"Connected successfully"<<endl;
	lpConnection->AddRef();
	if (0 != (iRet = lpConnection->Create2BizMsg(&callback)))
	{
		m_pSink->OnLoginProcess(std::string(lpConnection->GetErrorMsg(iRet)), false); 
		return -1;
	}
	if (0 != (iRet = lpConnection->Connect(5000)))
	{
		m_pSink->OnLoginProcess(std::string(lpConnection->GetErrorMsg(iRet)),false); 
		return -1;
	}

	return 0;
}

int TradeInterface::ReqFunction331100()
{
	IBizMessage* lpBizMessage=NewBizMessage();
	lpBizMessage->AddRef();
	lpBizMessage->SetFunction(331100);

	lpBizMessage->SetPacketType(REQUEST_PACKET);

	IF2Packer *pPacker=NewPacker(2);

	if (!pPacker)
	{
		m_pSink->OnError("331100 create packer error");
		return -1;
	}

	pPacker->AddRef();
	pPacker->BeginPack();
	///加入字段名
	pPacker->AddField("op_branch_no", 'I', 5);//操作分支机构
	pPacker->AddField("op_entrust_way", 'C', 1);//委托方式 
	pPacker->AddField("op_station", 'S', 255);//站点地址
	pPacker->AddField("branch_no", 'I', 5); 
	pPacker->AddField("password",'S', 15);
	pPacker->AddField("password_type", 'C'); 
	pPacker->AddField("input_content", 'C'); 
	pPacker->AddField("account_content", 'S', 30);
	pPacker->AddField("content_type",'S', 6);  
	pPacker->AddField("asset_prop", 'C', 1);


	///加入对应的字段值
	pPacker->AddInt(0);						
	pPacker->AddChar(m_EntrustWay);				
	pPacker->AddStr(m_opStation.c_str());				
	pPacker->AddInt(1);
	pPacker->AddStr(m_Password.c_str());
	pPacker->AddChar('2');		
	pPacker->AddChar('1');
	pPacker->AddStr(m_AccountName.c_str());			
	pPacker->AddStr("0");	
	pPacker->AddChar('B');	//0-普通账户，7-信用账户，B-衍生品账户

	///结束打包
	pPacker->EndPack();

	lpBizMessage->SetContent(pPacker->GetPackBuf(),pPacker->GetPackLen());
	lpConnection->SendBizMsg(lpBizMessage, 1);
	pPacker->FreeMem(pPacker->GetPackBuf());
	pPacker->Release();
	lpBizMessage->Release();
	return 0;
}

int TradeInterface::ReqFunction338011(char* exchange_type,char* option_code, int entrust_amount,double entrust_price,char entrust_bs, char entrust_oc, char covered_flag /*= ' '*/, const std::string &entrust_prop /*= "0"*/)
{
	int iRet = 0, hSend = 0;	
	///获取版本为2类型的pack打包器
	IF2Packer *pPacker = NewPacker(2);
	if(!pPacker)
	{
		m_pSink->OnError("338011 create packer error");
		return -1;
	}
	pPacker->AddRef();

	///定义解包器
	//IF2UnPacker *pUnPacker = NULL;

	IBizMessage* lpBizMessage = NewBizMessage();

	lpBizMessage->AddRef();

	///应答业务消息
	IBizMessage* lpBizMessageRecv = NULL;
	//功能号
	lpBizMessage->SetFunction(338011);
	//请求类型
	lpBizMessage->SetPacketType(REQUEST_PACKET);
	lpBizMessage->SetSystemNo(iSystemNo);
	///其他的应答信息
	LPRET_DATA pRetData = NULL;
	///开始打包
	pPacker->BeginPack();

	///加入字段名
	pPacker->AddField("op_branch_no", 'I', 5);//名字 类型 长度
	pPacker->AddField("op_entrust_way", 'C', 1);
	pPacker->AddField("op_station", 'S', 255);
	pPacker->AddField("branch_no", 'I', 5); 
	pPacker->AddField("client_id", 'S', 18);//客户ID
	pPacker->AddField("fund_account", 'S', 18);//资金账号
	pPacker->AddField("password", 'S', 15);
	pPacker->AddField("password_type", 'C', 1);	
	pPacker->AddField("user_token", 'S', 512);
	pPacker->AddField("asset_prop", 'C', 1);
	pPacker->AddField("exchange_type", 'S', 4);
	pPacker->AddField("option_account", 'S', 13);
	pPacker->AddField("option_code", 'S', 8);
	pPacker->AddField("entrust_amount", 'I', 10);
	pPacker->AddField("opt_entrust_price", 'F', 255, 4);
	pPacker->AddField("entrust_bs", 'C', 1);
	pPacker->AddField("entrust_oc", 'C', 1);
	pPacker->AddField("covered_flag", 'C', 1);
	pPacker->AddField("entrust_prop", 'S', 3);	
	pPacker->AddField("batch_no", 'I', 8);

	///加入对应的字段值
	pPacker->AddInt(m_BranchNo);						
	pPacker->AddChar(m_EntrustWay);					
	pPacker->AddStr(m_opStation.c_str());					
	pPacker->AddInt(m_BranchNo);				
	pPacker->AddStr(m_client_id);			 
	pPacker->AddStr(m_AccountName.c_str());			
	pPacker->AddStr(m_Password.c_str());				
	pPacker->AddChar('2');					
	pPacker->AddStr(m_opUserToken);
	pPacker->AddChar('B');	//0-普通账户，7-信用账户，B-衍生品账户
	pPacker->AddStr(exchange_type);					
	pPacker->AddStr("0");
	pPacker->AddStr(option_code);				
	pPacker->AddDouble(entrust_amount);
	pPacker->AddDouble(entrust_price);		
	pPacker->AddChar(entrust_bs);	
	pPacker->AddChar(entrust_oc);
	pPacker->AddChar(covered_flag);
	pPacker->AddStr(entrust_prop.c_str());					
	pPacker->AddInt(0);						

	pPacker->EndPack();
	lpBizMessage->SetContent(pPacker->GetPackBuf(),pPacker->GetPackLen());
	lpConnection->SendBizMsg(lpBizMessage, 1);
	pPacker->FreeMem(pPacker->GetPackBuf());
	pPacker->Release();
	lpBizMessage->Release();
	return 0;
}

int TradeInterface::ReqFunction338012(const std::string &entrust_no)
{
	int iRet = 0, hSend = 0;	
	///获取版本为2类型的pack打包器
	IF2Packer *pPacker = NewPacker(2);
	if(!pPacker)
	{
		m_pSink->OnError("338012 create packer error");
		return -1;
	}
	pPacker->AddRef();

	///定义解包器
	//IF2UnPacker *pUnPacker = NULL;

	IBizMessage* lpBizMessage = NewBizMessage();

	lpBizMessage->AddRef();

	///应答业务消息
	IBizMessage* lpBizMessageRecv = NULL;
	//功能号
	lpBizMessage->SetFunction(338012);
	//请求类型
	lpBizMessage->SetPacketType(REQUEST_PACKET);
	lpBizMessage->SetSystemNo(iSystemNo);
	///其他的应答信息
	LPRET_DATA pRetData = NULL;
	///开始打包
	pPacker->BeginPack();

	///加入字段名
	pPacker->AddField("op_branch_no", 'I', 5);//名字 类型 长度
	pPacker->AddField("op_entrust_way", 'C', 1);
	pPacker->AddField("op_station", 'S', 255);
	pPacker->AddField("branch_no", 'I', 5); 
	pPacker->AddField("client_id", 'S', 18);//客户ID
	pPacker->AddField("fund_account", 'S', 18);//资金账号
	pPacker->AddField("password", 'S', 15);
	pPacker->AddField("password_type", 'C', 1);	
	pPacker->AddField("user_token", 'S', 512);
	pPacker->AddField("asset_prop",'C', 1);
	pPacker->AddField("exchange_type", 'S', 4);
	pPacker->AddField("entrust_no", 'S', 6);

	///加入对应的字段值
	pPacker->AddInt(m_BranchNo);						
	pPacker->AddChar(m_EntrustWay);					
	pPacker->AddStr(m_opStation.c_str());					
	pPacker->AddInt(m_BranchNo);				
	pPacker->AddStr(m_client_id);			 
	pPacker->AddStr(m_AccountName.c_str());			
	pPacker->AddStr(m_Password.c_str());				
	pPacker->AddChar('2');					
	pPacker->AddStr(m_opUserToken);			
	pPacker->AddChar('B');					
	pPacker->AddStr("");
	pPacker->AddStr(entrust_no.c_str());				

	pPacker->EndPack();
	lpBizMessage->SetContent(pPacker->GetPackBuf(),pPacker->GetPackLen());
	lpConnection->SendBizMsg(lpBizMessage, 1);
	pPacker->FreeMem(pPacker->GetPackBuf());
	pPacker->Release();
	lpBizMessage->Release();
	return 0;
}

int TradeInterface::Subscribe(char* topicName)
{
	lpSub = lpConnection->NewSubscriber(&subCallback, topicName, 5000);
	if (!lpSub)
	{
		m_pSink->OnError(lpConnection->GetMCLastError());
		return -1;
	}
	lpSub->AddRef();

	//订阅参数获取
	CSubscribeParamInterface* lpSubscribeParam = NewSubscribeParam();
	lpSubscribeParam->AddRef();

	lpSubscribeParam->SetTopicName(topicName);
	lpSubscribeParam->SetFilter("branch_no",  const_cast<char*>(basic_type_to_String<int>(m_BranchNo).c_str()));
	lpSubscribeParam->SetFilter("fund_account", const_cast<char*>(m_AccountName.c_str()));

	int  iRet = lpSub->SubscribeTopic(lpSubscribeParam, 5000, NULL);
	if(iRet>0)
	{
		//m_subParamMap[iRet] = lpSubscribeParam;
	}
	else
	{
		m_pSink->OnError(lpConnection->GetErrorMsg(iRet));
		return-1;
	}

	lpSubscribeParam->Release();

	return 0;
}

int TradeInterface::Login(const std::string& account, const std::string& password)
{
	m_AccountName = account;
	m_Password = password;

	int iRet = InitConn();
	if(iRet >= 0)
	{
		iRet = ReqFunction331100();
	}

	return iRet;
}

int TradeInterface::SendOrder(const stOrderInfo& order_info)
{
#ifdef EASYLOG
	std::ostringstream ostr; 
	ostr << "SendOrder:" << order_info.option_code << "\t" << order_info.entrust_bs << "\t" << order_info.entrust_oc
		 << "\t" << order_info.opt_entrust_price  << "\t" << order_info.entrust_amount;
	EasyLog::Inst()->Log(ostr.str()); 
#endif
	return ReqFunction338011(const_cast<char*>(order_info.exchange_type.c_str()), const_cast<char*>(order_info.option_code.c_str()), 
		order_info.entrust_amount, order_info.opt_entrust_price, order_info.entrust_bs,order_info.entrust_oc);
}

int TradeInterface::CancelOrder(const std::string &entrust_no)
{
#ifdef EASYLOG
	std::ostringstream ostr; 
	ostr << "CancelOrder:" << entrust_no ;
	EasyLog::Inst()->Log(ostr.str()); 
#endif
	return ReqFunction338012(entrust_no);
}

unsigned long FUNCTION_CALL_MODE CCallback::QueryInterface(const char *iid, IKnown **ppv)
{
	return 0;
}

unsigned long FUNCTION_CALL_MODE CCallback::AddRef()
{
	return 0;
}

unsigned long FUNCTION_CALL_MODE CCallback::Release()
{
	return 0;
}

void FUNCTION_CALL_MODE CCallback::OnConnect(CConnectionInterface *lpConnection)
{

}

void FUNCTION_CALL_MODE CCallback::OnSafeConnect(CConnectionInterface *lpConnection)
{

}

void FUNCTION_CALL_MODE CCallback::OnRegister(CConnectionInterface *lpConnection)
{

}

void FUNCTION_CALL_MODE CCallback::OnClose(CConnectionInterface *lpConnection)
{

}

void FUNCTION_CALL_MODE CCallback::OnSent(CConnectionInterface *lpConnection, int hSend, void *reserved1, void *reserved2, int nQueuingData)
{

}

void FUNCTION_CALL_MODE CCallback::Reserved1(void *a, void *b, void *c, void *d)
{

}

void FUNCTION_CALL_MODE CCallback::Reserved2(void *a, void *b, void *c, void *d)
{

}

int FUNCTION_CALL_MODE CCallback::Reserved3()
{
	return 0;
}

void FUNCTION_CALL_MODE CCallback::Reserved4()
{

}

void FUNCTION_CALL_MODE CCallback::Reserved5()
{

}

void FUNCTION_CALL_MODE CCallback::Reserved6()
{

}

void FUNCTION_CALL_MODE CCallback::Reserved7()
{

}

void FUNCTION_CALL_MODE CCallback::OnReceivedBiz(CConnectionInterface *lpConnection, int hSend, const void *lpUnPackerOrStr, int nResult)
{

}

void FUNCTION_CALL_MODE CCallback::OnReceivedBizEx(CConnectionInterface *lpConnection, int hSend, LPRET_DATA lpRetData, const void *lpUnpackerOrStr, int nResult)
{

}

void FUNCTION_CALL_MODE CCallback::OnReceivedBizMsg(CConnectionInterface *lpConnection, int hSend, IBizMessage* lpMsg)
{
	if (lpMsg!=NULL)
	{
		//成功,应用程序不能释放lpBizMessageRecv消息
		if (lpMsg->GetErrorNo() ==0)
		{
			int iLen = 0;
			const void * lpBuffer = lpMsg->GetContent(iLen);
			IF2UnPacker * lpUnPacker = NewUnPacker((void *)lpBuffer,iLen);
			lpUnPacker->AddRef();
			//cout<<"Function ID:"<<lpMsg->GetFunction()<<endl;
			lpMsg->GetFunction();
			switch(lpMsg->GetFunction())
			{
			case 331100:
				OnResponse_331100(lpUnPacker);
				lpUnPacker->Release();
				break;
			}
		}
		else
		{
#ifdef EASYLOG
			EasyLog::Inst()->Log(lpMsg->GetErrorInfo()); 
#endif
			//error_info
			lpReqMode->m_pSink->OnError(lpMsg->GetErrorInfo());
		}
	}
}

void CCallback::OnResponse_331100(IF2UnPacker *lpUnPacker)
{
	const char *pUserToken = lpUnPacker->GetStr("user_token");
	if( pUserToken != NULL)
	{
		lpReqMode->SetUserToken(pUserToken);
	}

	int iBranchNo = lpUnPacker->GetInt("branch_no");
	lpReqMode->SetBranchNo(iBranchNo);

	const char *pClientId = lpUnPacker->GetStr("client_id");
	if(pClientId)
	{
		lpReqMode->SetClientId(pClientId);
	}

	int iSystemNo_ = lpUnPacker->GetInt("sysnode_id");
	lpReqMode->SetSystemNo(iSystemNo_);

	lpReqMode->m_pSink->OnLoginProcess(std::string("success"), true);
}

unsigned long FUNCTION_CALL_MODE CSubCallback::QueryInterface(const char *iid, IKnown **ppv)
{
	return 0;
}

unsigned long FUNCTION_CALL_MODE CSubCallback::AddRef()
{
	return 0;
}

unsigned long FUNCTION_CALL_MODE CSubCallback::Release()
{
	return 0;
}

void FUNCTION_CALL_MODE CSubCallback::OnReceived(CSubscribeInterface *lpSub,int subscribeIndex, const void *lpData, int nLength,LPSUBSCRIBE_RECVDATA lpRecvData)
{
	////打印收到主推的二进制业务体信息
	IF2UnPacker* lpUnPacker = NewUnPacker((void*)lpData,nLength);
	if (lpUnPacker)
	{
		lpUnPacker->AddRef();
		//lpReqSub->PrintUnPack(lpUnPack);
		stOrderInfo order_info;
		stTradeInfo trade_info;
		while (!lpUnPacker->IsEOF())
		{
			if(strcmp(lpRecvData->szTopicName , SubTopicName[0]) == 0 )
			{
				order_info.entrust_no = lpUnPacker->GetInt("entrust_no");
				order_info.exchange_type = lpUnPacker->GetStr("exchange_type");
				order_info.option_code = lpUnPacker->GetStr("option_code");
				order_info.entrust_bs = lpUnPacker->GetChar("entrust_bs");     
				order_info.entrust_oc = lpUnPacker->GetChar("entrust_oc");    
				order_info.covered_flag = lpUnPacker->GetChar("covered_flag");   
				order_info.entrust_status = lpUnPacker->GetChar("entrust_status"); 
				order_info.entrust_amount = lpUnPacker->GetInt("entrust_amount");
				order_info.opt_entrust_price = lpUnPacker->GetDouble("opt_entrust_price");
				order_info.report_no = lpUnPacker->GetInt("report_no");
				order_info.batch_no = lpUnPacker->GetInt("batch_no");
				if(lpReqMode) lpReqMode->m_pSink->OnOrder(order_info);
#ifdef EASYLOG
				std::ostringstream ostr; 
				ostr << "OnOrder entrust_no:" << order_info.entrust_no << "\t" << order_info.report_no;
				EasyLog::Inst()->Log(ostr.str()); 
#endif
			}
            else if(strcmp(lpRecvData->szTopicName , SubTopicName[1]) == 0)
			{
				trade_info.entrust_no = lpUnPacker->GetInt("entrust_no");
				trade_info.business_id = lpUnPacker->GetStr("business_id");
				trade_info.business_amount = lpUnPacker->GetInt("business_amount");
				trade_info.opt_business_price = lpUnPacker->GetDouble("opt_business_price");
				trade_info.business_time = lpUnPacker->GetInt("business_time");
				trade_info.report_no = lpUnPacker->GetInt("report_no");
				trade_info.entrust_status = lpUnPacker->GetChar("entrust_status");
				if(lpReqMode) lpReqMode->m_pSink->OnTrade(trade_info);
#ifdef EASYLOG
				std::ostringstream ostr; 
				ostr << "OnTrade entrust_no:" << trade_info.entrust_no << "\tentrust_status:" << trade_info.entrust_status  << "\t"<< trade_info.report_no 
					<< "\tbusiness_amount:" << trade_info.business_amount;
				EasyLog::Inst()->Log(ostr.str()); 
#endif
			}
			lpUnPacker->Next();
		}
		lpUnPacker->Release();
	}
}

void FUNCTION_CALL_MODE CSubCallback::OnRecvTickMsg(CSubscribeInterface *lpSub,int subscribeIndex,const char* TickMsgInfo)
{

}
