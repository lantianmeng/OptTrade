#ifndef MARKETDATA_H
#define MARKETDATA_H

#include "CTSMdApiStruct.h"
#include "Lock.h"
#include <map>
#include <string>
#include <memory>

//sink��ʽ�Ļص�����
class IQuoteSink  
{  
public:  
    virtual void OnMarketData(pCITICS_MD_OPTPRICE ctsOptCode) = 0;  
}; 

class Quote
{
public:
	Quote(IQuoteSink* pSink)  
        :m_pSink(pSink)  
    {  
    }  
	void PublishMarketData(pCITICS_MD_OPTPRICE pData) {m_pSink->OnMarketData(pData);}
private:  
	IQuoteSink* m_pSink;  
};

//������������ģʽ  ���ڴ�й©
class MarketData
{
public:
	MarketData():m_pQuote(NULL) {}
	static MarketData* GetInstance();

	//error_msg: ������� ������Ϣ
	//����ֵ: >=0 ��ȷ  < 0 ����
	//int subscribe(Callback market_data, void*p, std::string& error_msg);
	int subscribe(IQuoteSink*pObserver, std::string& error_msg);

	//market_data: ������� ��������
	bool GetMarketData(const std::string & symbol, CITICS_MD_OPTPRICE& market_data);

private:
//	MarketData(const MarketData&){};
//	MarketData& operator=(const MarketData&){};
	static void __stdcall OnDataCallback(pCITICS_MD_OPTPRICE ctsOptCode);
	void AddMarketData(pCITICS_MD_OPTPRICE ctsOptCode);

private:
	std::shared_ptr<Quote> m_pQuote;
	ctsMdHandle ctsHandle ;

	CriSection m_Lock;
	typedef std::map<std::string, CITICS_MD_OPTPRICE> MarketDataType;
	MarketDataType m_DataMap;

	static MarketData *m_Instance;
	//Callback SymbolCallback;

	class CGarbo // ����Ψһ��������������������ɾ��CSingleton��ʵ��  
	{ 
	public: 
		~CGarbo() 
		{  
			if (MarketData::m_Instance) 
				delete MarketData::m_Instance; 
			
		} 
	}; 
	static CGarbo Garbo; 
};

#endif