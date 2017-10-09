#ifndef _Lock_H  
#define _Lock_H  
  
#include <windows.h>  
  
//锁接口类  
class ILock  
{  
public:  
    virtual ~ILock() {}  
  
    virtual void Lock() const = 0;  
    virtual void Unlock() const = 0;  
};  
  
//互斥对象锁类  
class Mutex : public ILock  
{  
public:  
    Mutex();  
    ~Mutex();  
  
    virtual void Lock() const;  
    virtual void Unlock() const;  
  
private:  
    HANDLE m_mutex;  
};  
  
//临界区锁类  
class CriSection : public ILock  
{  
public:  
    CriSection();  
    ~CriSection();  
  
    virtual void Lock() const;  
    virtual void Unlock() const;  
  
private:  
    CRITICAL_SECTION m_critclSection;  
};  
  
  
//锁  
class CMyLock  
{  
public:  
    CMyLock(const ILock&);  
    ~CMyLock();  
  
private:  
    const ILock& m_lock;  
};  
  
  
#endif 
