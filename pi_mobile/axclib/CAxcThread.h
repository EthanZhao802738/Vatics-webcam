#ifndef _C_AXC_THREAD_H_
#define _C_AXC_THREAD_H_

#include "axclib_data_type.h"
#include "CAxcBaseResource.h"

//#define AXC_SUPPORT_SUSPEND_THREAD

class CAxcThread;
typedef axc_dword(*axc_thread_proc)(CAxcThread* pThread, void* pContext);

class AXC_API CAxcThread : public CAxcBaseResource
{
public:
	CAxcThread(const char* szResourceName);
	virtual ~CAxcThread();

	axc_bool IsValid() { return (m_hThread != NULL); }

	// 设置新的 Context 值。返回值为前次的 m_pContext 值
	void* SetContext(void* pContext) { void* pResult = m_pContext; m_pContext = pContext; return pResult; }
	// 获取目前的 Context 值
	void* GetContext() { return m_pContext; }

	// 创建线程
	axc_bool Create(
		axc_thread_proc pfnThreadProc, //线程函数
		void* pContext, // Context 值
		axc_dword dwStackSize = 0, // 栈尺寸，Bytes；如果为 0 使用操作系统默认值 (也跟随应用程序的工程设定) // Windows中，如果设定的值小于系统默认值，会使用系统默认值，不会使用dwStackSize值
		axc_i32 iPriority = 0 // 优先级, 范围 -15(最低优先级,idle) ~ 15(最高优先级,realtime)，普通优先级为 0
#ifdef AXC_SUPPORT_SUSPEND_THREAD
		, axc_bool bSuspended = axc_false // axc_true - 线程创建后处于挂起状态，需要调用 Resume() 去激活； axc_false - 线程创建后直接开始运行
#endif
		);

	// 释放线程
	axc_bool Destroy(
		axc_dword dwWaitMilliSeconds, // 等待线程自己退出，毫秒
		axc_bool bTerminateIfTimeout = axc_true); // axc_true - 超时后强制关闭线程, axc_false - 不强制关闭

	// 检查线程是否存活
	axc_bool CheckAlive(); 

	// 获取线程ID
	AXC_INLINE axc_ddword GetThreadId() { return m_ddwThreadId; }

	// 调整优先级
	axc_bool SetPriority(axc_i32 iPriority);

#ifdef AXC_SUPPORT_SUSPEND_THREAD
	// 挂起线程
	// 返回值：线程前一次的挂起计数，-1 表示失败, >=0 表示成功
	axc_i32 Suspend();
	// 恢复线程
	// 返回值：线程前一次的挂起计数，-1 表示失败，0 表示线程之前就已经是活动的，1 表示线程被重新激活了, >1 意味着线程仍然被挂起着
	axc_i32 Resume();
#endif

	// 毫秒级延时
	void SleepMilliSeconds(axc_dword dwMilliSeconds);

public:
	// 内部调用
	void OnGlobalCallback();

protected:
	axc_handle m_hThread;
	axc_ddword m_ddwThreadId;
	axc_thread_proc m_pfnThreadProc;
	void* m_pContext;

public:
	// 向子类请求目前的状态信息
	// 子类需要继承这个函数，向axclib库报告自己的状态
	// 返回值： 0-正常， 1-有轻微错误， 2-有严重错误
	virtual axc_dword ReportResourceStatus(char* szStatusText, const axc_dword dwTextBufferSize) { szStatusText[0] = 0; return 0; }
};


#endif // _C_AXC_THREAD_H_
