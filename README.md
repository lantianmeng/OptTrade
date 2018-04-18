# OptTrade
1、sink方式实现回调机制
   如果对sink模式进行深度探索，就涉及到观察者模式 与 订阅发布模式， 生产者消费者模式  等 设计模式的内容。
   
3、线程安全的STL（锁lock的实现 无异议，C++11已经可以直接调用）

4、单利模式（线程安全，无内存泄漏）

5、关于“friend”的使用
lpReqMode->m_pSink->XXXX    "m_pSink是private，TradeInterface对象lpReqMode在类的外部直接使用"

6、搞清楚 lock_guard 与unique_lock的区别  (threadpool 在github上有个很类似的版本，搜索即可)

7、MemoryPool （在github上搜索，star最多的那个，写demo弄明白  （百度搜索 c++内存池） http://blog.csdn.net/xjtuse2014/article/details/52302083）

8、无锁队列   （对比 threadsafe_queue种的队列）  的应用场景，实现
http://blog.csdn.net/cws1214/article/details/47680773

10、learn c++ network programming  （目前文件中主要是熟悉api， 迭代并发服务器的逻辑都没有，需进一步学习）


11、unsigned char * 十六进制 转换为string字符串    【openssl rc4加密算法中会用到】
    如 “0x510x090x500xbd0x7d0xc8”  ->  "510950bd7dc8"
	//unsigned char * 十六进制 转换为string字符串
	stringstream ss;
	for (int i = 0; i != nMsgBodyLen; ++i)
	{
		//ss << "0x" << setw(2) << setfill('0') << hex << (int)szCryptData[i] << ", ";
		ss << setw(2) << setfill('0') << hex << (int)szCryptData[i];
	}

	strCrypt = ss.str();

12、编译gRpc
（1）windows平台编译  vs2015及以上
https://github.com/grpc/grpc/blob/master/INSTALL.md   
https://blog.csdn.net/xie1xiao1jun/article/details/52514206
（按照官网cmake的方式编译， 跑demo时会发现debug版本存在与release调用方面的问题，这是因为默认的cmake生成的vs工程中 属性/ c/c++ /代码生成/运行库 在debug时为/MD而不是/MDd，在vs中将依赖工程全部设置正确后即可）
（2）依赖库可参考gRpcTest工程
