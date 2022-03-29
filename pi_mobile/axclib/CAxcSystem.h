#ifndef _C_AXC_SYSTEM_H_
#define _C_AXC_SYSTEM_H_

#include "axclib_data_type.h"

class AXC_API CAxcSystem
{
public:
	CAxcSystem();
	virtual ~CAxcSystem();

	// 检查是否有键盘输入动作。如果返回true，可以调用getch()获取输入的字符
	static axc_bool kbhit();
	static axc_i32 getch();

	// 加载动态库
	static axc_handle LibraryLoad(const char* szLibPathName, const axc_dword dwFlag = 0);
	// 释放动态库
	static axc_bool LibraryFree(axc_handle hLib);
	// 获取动态库中的函数的指针
	static void* LibraryGetProc(axc_handle hLib, const char* szProcName);

	// 获取本进程的ID
	static axc_ddword GetCurrentProcessId();
	// 获取本线程的ID
	// 在windows下，GetCurrentThreadIdOnProcess()和GetCurrentThreadIdOnSystem()是相同的，都调用 GetCurrentThreadId()
	// 在linux下，GetCurrentThreadIdOnProcess()调用 pthread_self()， GetCurrentThreadIdOnSystem()调用 gettid()
	static axc_ddword GetCurrentThreadIdOnProcess(); // 获取本线程的ID（进程内的唯一ID）
	static axc_ddword GetCurrentThreadIdOnSystem(); // 获取本线程的ID（OS内的唯一ID）
};

#endif // _C_AXC_SYSTEM_H_
