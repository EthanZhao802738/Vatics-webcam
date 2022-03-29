#ifndef _C_AXC_AUTO_LOCK_H_
#define _C_AXC_AUTO_LOCK_H_

#include "CAxcMutex.h"

class AXC_API CAxcAutoLock
{
public:
	CAxcAutoLock(CAxcMutex* pMutex)
	{
		m_pMutex = pMutex;
		if(m_pMutex)
		{
			m_pMutex->Lock();
		}
	}
	~CAxcAutoLock()
	{
		if(m_pMutex)
		{
			m_pMutex->Unlock();
		}
	}
protected:
	CAxcMutex* m_pMutex;
};

#endif // _C_AXC_AUTO_LOCK_H_
