# OptTrade
1、sink方式实现回调机制
   如果对sink模式进行深度探索，就涉及到观察者模式 与 订阅发布模式， 生产者消费者模式  等 设计模式的内容。
   
2、简易日志EasyLog

3、线程安全的STL（锁lock的实现 无异议，C++11已经可以直接调用）

4、单利模式（线程安全，无内存泄漏）

5、关于“friend”的使用
lpReqMode->m_pSink->XXXX    "m_pSink是private，TradeInterface对象lpReqMode在类的外部直接使用"

6、搞清楚 lock_guard 与unique_lock的区别  (threadpool 在github上有个很类似的版本，搜索即可)

7、MemoryPool （在github上搜索，star最多的那个，写demo弄明白  （百度搜索 c++内存池） http://blog.csdn.net/xjtuse2014/article/details/52302083）

8、无锁队列   （对比 threadsafe_queue种的队列）  的应用场景，实现
http://blog.csdn.net/cws1214/article/details/47680773

9、learn c++11   tips.h/tips.cpp  ConsoleApplication.cpp
