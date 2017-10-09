#include "stdafx.h"
#include "Lock.h"  
  
//---------------------------------------------------------------------------  
  
//����һ�������������  
Mutex::Mutex()  
{  
    m_mutex = ::CreateMutex(NULL, FALSE, NULL);  
}  
  
//���ٻ�������ͷ���Դ  
Mutex::~Mutex()  
{  
    ::CloseHandle(m_mutex);  
}  
  
//ȷ��ӵ�л��������̶߳Ա�������Դ�Ķ��Է���  
void Mutex::Lock() const  
{  
    DWORD d = WaitForSingleObject(m_mutex, INFINITE);  
}  
  
//�ͷŵ�ǰ�߳�ӵ�еĻ��������ʹ�����߳̿���ӵ�л�����󣬶Ա�������Դ���з���  
void Mutex::Unlock() const  
{  
    ::ReleaseMutex(m_mutex);  
}  
  
//---------------------------------------------------------------------------  
  
//��ʼ���ٽ���Դ����  
CriSection::CriSection()  
{  
    ::InitializeCriticalSection(&m_critclSection);  
}  
  
//�ͷ��ٽ���Դ����  
CriSection::~CriSection()  
{  
    ::DeleteCriticalSection(&m_critclSection);  
}  
  
//�����ٽ���������  
void CriSection::Lock() const  
{  
    ::EnterCriticalSection((LPCRITICAL_SECTION)&m_critclSection);  
}     
  
//�뿪�ٽ���������  
void CriSection::Unlock() const  
{  
    ::LeaveCriticalSection((LPCRITICAL_SECTION)&m_critclSection);  
}  
  
//---------------------------------------------------------------------------  
  
//����C++���ԣ������Զ�����  
CMyLock::CMyLock(const ILock& m) : m_lock(m)  
{  
    m_lock.Lock();  
}  
  
//����C++���ԣ������Զ�����  
CMyLock::~CMyLock()  
{  
    m_lock.Unlock();  
}  
