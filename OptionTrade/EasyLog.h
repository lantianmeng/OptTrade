/* 
简单的日志记录类. (日志而已，何必那么复杂！！！） 
W.J.Chang 2013.12.13 
 
说明:(EasyLog.h) 
1, 简单的单件实现（自动垃圾回收） 
2, 使用方法：EasyLog::Inst()->Log("Run..."); 
3, 日志记录结果：Run...    [2013.12.13 16:38:42 Friday] 
*/  
#pragma once  
#ifndef EASY_LOG_H_8080  
#define EASY_LOG_H_8080  
#include <memory>  
#include <ctime>  
#include <iostream>  
#include <fstream>  
class EasyLog  
{  
public:  
    static EasyLog * Inst(){  
        if (0 == _instance){  
            _instance = new EasyLog;  
        }  
        return _instance;  
    }  
  
    void Log(std::string msg); // 写日志的方法  
private:  
    EasyLog(void){}  
    //virtual ~EasyLog(void){delete _instance;}  
    friend class std::auto_ptr<EasyLog>;  
    static EasyLog* _instance;  

	class CGarbo // 它的唯一工作就是在析构函数中删除CSingleton的实例  
	{ 
	public: 
		~CGarbo() 
		{  
			if (EasyLog::_instance) 
				delete EasyLog::_instance; 
		} 
	}; 
	static CGarbo Garbo; 
};  
  
EasyLog* EasyLog::_instance = NULL;  
EasyLog::CGarbo EasyLog::Garbo;
  
void EasyLog::Log(std::string loginfo) {  
    std::ofstream ofs;  
    time_t t = time(0);  
    char tmp[64];  
    strftime(tmp, sizeof(tmp), "[%Y.%m.%d %X %A]", localtime(&t));  
    ofs.open("EasyLog.log", std::ofstream::app);    
    ofs << tmp ;  
	ofs.write(loginfo.c_str(), loginfo.size());
	ofs << '\n';   
    ofs.close();  
}  
#endif