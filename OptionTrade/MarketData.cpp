#include "stdafx.h"
#include "MarketData.h"  
#include "CTSMD.h"

MarketData* MarketData::m_Instance = NULL;
MarketData::CGarbo MarketData::Garbo;

MarketData* MarketData::GetInstance()
{
	if( m_Instance == NULL)
	{
		//pthread_mutex_lock(&mutex);
		if( m_Instance == NULL)
		{ 
			m_Instance = new MarketData();
		}
		//pthread_mutex_unlock(&mutex);
	}
	return m_Instance;
}

int MarketData::subscribe(IQuoteSink*pObserver, std::string& error_msg)
//int MarketData::subscribe(Callback SymbolCallback_, std::string& error_msg)
{
	int ret = frontConnect(&ctsHandle,"t2sdk.ini");
	if (0 == ret)
	{
		ret = SubscribeMd_Opt(ctsHandle, OnDataCallback);
		if ( ret >= 0)
		{
			m_pQuote.reset(new Quote(pObserver));
		}
		else if (ret == -1)
		{
			error_msg = GetMCLastError(ctsHandle);
		}
		else
		{
			error_msg = GetErrorMsg(ctsHandle,ret);
		}
	}
	else
	{
		error_msg = GetErrorMsg(ctsHandle,ret);
	}
	return ret;
}

bool MarketData::GetMarketData(const std::string & symbol, CITICS_MD_OPTPRICE& market_data)
{
	bool ret = false;
	CMyLock lock(m_Lock);
	MarketDataType::iterator iter = m_DataMap.find(symbol);
	if (iter != m_DataMap.end())
	{
		market_data = iter->second;
		ret = true;
	}
	return ret;
}

void MarketData::AddMarketData(pCITICS_MD_OPTPRICE ctsOptCode)
{
	//CMyLock lock(m_Lock);
	//m_DataMap.insert(make_pair(std::string(ctsOptCode->option_code) , *ctsOptCode));
	m_DataMap[std::string(ctsOptCode->option_code)] = *ctsOptCode;
	
	m_pQuote->PublishMarketData(ctsOptCode);
}

void MarketData::OnDataCallback(pCITICS_MD_OPTPRICE ctsOptCode)
{
	if (ctsOptCode != NULL) 
	{
		if (ctsOptCode->exchange_type == "2" || ctsOptCode->last_price == 0 || ctsOptCode->opt_buy_price1 == 0 || ctsOptCode->opt_sale_price1 == 0) return;
		m_Instance->AddMarketData(ctsOptCode);
	}
}