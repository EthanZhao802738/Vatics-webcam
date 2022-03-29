#ifndef _C_AXC_EVENT_H_
#define _C_AXC_EVENT_H_

#include "axclib_data_type.h"
#include "CAxcBaseResource.h"

class AXC_API CAxcEvent : public CAxcBaseResource
{
public:
	CAxcEvent(const char* szResourceName, axc_bool bManualReset = axc_true);
	virtual ~CAxcEvent();
	
	axc_bool IsValid() { return (m_hEvent != NULL); }

	// 将当前事件对象设置为有信号状态
	// 若自动重置，则等待该事件对象的所有线程只有一个可被调度
	// 若人工重置，则等待该事件对象的所有线程变为可被调度
	axc_bool Set();

	// 将当前事件对象设置为无信号状态
	axc_bool Reset();

	// 以当前事件对象，阻塞线程，直到事件对象被设置为有信号状态、或者挂起指定时间间隔
	// 之后线程自动恢复可调度
	axc_bool Wait(const axc_dword dwMilliSeconds = 0x7FFFFFFFL);

protected:
	axc_handle m_hEvent;

public:
	// 向子类请求目前的状态信息
	// 子类需要继承这个函数，向axclib库报告自己的状态
	// 返回值： 0-正常， 1-有轻微错误， 2-有严重错误
	virtual axc_dword ReportResourceStatus(char* szStatusText, const axc_dword dwTextBufferSize) { szStatusText[0] = 0; return 0; }
};

#endif // _C_AXC_EVENT_H_
