#ifndef MARKETDATA_H
#define MARKETDATA_H

#include "CTSMdApiStruct.h"
#include "Lock.h"
#include <map>
#include <string>
#include <memory>

//sink方式的回调函数
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

//经典懒汉单例模式  无内存泄漏
class MarketData
{
public:
	MarketData():m_pQuote(NULL) {}
	static MarketData* GetInstance();

	//error_msg: 输出参数 错误信息
	//返回值: >=0 正确  < 0 错误
	//int subscribe(Callback market_data, void*p, std::string& error_msg);
	int subscribe(IQuoteSink*pObserver, std::string& error_msg);

	//market_data: 输出参数 行情数据
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

	class CGarbo // 它的唯一工作就是在析构函数中删除CSingleton的实例  
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