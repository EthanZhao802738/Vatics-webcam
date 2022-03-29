#ifndef _C_AXC_MUTEX_H_
#define _C_AXC_MUTEX_H_

#include "axclib_data_type.h"
#include "CAxcBaseResource.h"

//
// Mutex Object
//
class AXC_API CAxcMutex : public CAxcBaseResource
{
public:
	CAxcMutex(const char* szResourceName);
	virtual ~CAxcMutex();

	axc_bool IsValid() { return (NULL != m_hMutex); }
	
	axc_bool Lock();
	void Unlock();

protected:
	axc_handle m_hMutex;

public:
	// 向子类请求目前的状态信息
	// 子类需要继承这个函数，向axclib库报告自己的状态
	// 返回值： 0-正常， 1-有轻微错误， 2-有严重错误
	virtual axc_dword ReportResourceStatus(char* szStatusText, const axc_dword dwTextBufferSize) { szStatusText[0] = 0; return 0; }
};

#endif // _C_AXC_MUTEX_H_
