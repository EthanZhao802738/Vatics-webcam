#ifndef _AXCLIB_OS_CHECK_H_
#define _AXCLIB_OS_CHECK_H_

//
// 判断操作系统的类型
//
#if defined(_WINDOWS) || defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
	#define AXC_OS_WINDOWS
#elif defined(__LINUX__) || defined(__linux__)
	#define AXC_OS_LINUX
#elif defined(__APPLE__)
	#define AXC_OS_APPLE
	#error "!!! NOT Support Apple OS !!!"
#else
	#define AXC_OS_UNKNOW
	#error "!!! NOT Support this OS !!!"
#endif


#endif // _AXCLIB_OS_CHECK_H_
