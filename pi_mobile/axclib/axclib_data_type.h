#ifndef _AXCLIB_DATA_TYPE_H_
#define _AXCLIB_DATA_TYPE_H_

//
// 先确定OS
//
#include "axclib_os_check.h"

//
// 默认参数
//
#include "axclib_default.h"

//
// 常用数据类型的定义
//
typedef signed char   axc_i8;
typedef unsigned char axc_u8;

typedef signed short   axc_i16;
typedef unsigned short axc_u16;

typedef signed int   axc_i32;
typedef unsigned int axc_u32;

#if defined(AXC_OS_WINDOWS)
	typedef signed __int64     axc_i64;
	typedef unsigned __int64   axc_u64;
#elif defined(AXC_OS_LINUX)
	typedef signed long long   axc_i64;
	typedef unsigned long long axc_u64;
#else
	#error "Not supported this OS !"
#endif

typedef axc_i32 axc_bool;
#define axc_true  (1)
#define axc_false (0)

typedef axc_u8   axc_byte;
typedef axc_u16  axc_word;
typedef axc_u32  axc_dword;
typedef axc_u64  axc_ddword;

typedef void* axc_handle;

#if !defined(AXC_OS_WINDOWS)
	typedef axc_i32 BOOL;
	typedef axc_u8  BYTE;
	typedef axc_u16 WORD;
	typedef axc_u32 DWORD;
	typedef axc_u64 ULONGLONG;

	typedef void* HANDLE;

	#ifndef TRUE
	#define TRUE 1
	#endif

	#ifndef FALSE
	#define FALSE 0
	#endif

	#ifndef MAX_PATH
	#define MAX_PATH 260
	#endif

	typedef int SOCKET;
	#define SOCKET_ERROR (-1)
	#define INVALID_SOCKET (-1)
	#define closesocket close
#endif


//
// 库的API定义前缀
//
#if defined(AXC_OS_WINDOWS)
	#ifdef AXCLIB_EXPORTS
	#define AXC_API __declspec(dllexport)
	#else
	#define AXC_API __declspec(dllimport)
	#endif
#else
	#define AXC_API
#endif

#define AXC_STDCALL __stdcall

#if defined(AXC_OS_WINDOWS)
	#define AXC_INLINE __inline
#else
	#define AXC_INLINE inline
#endif


//
// 版本号结构
//
#pragma pack(8)
typedef union _AXC_U_VERSION
{
	axc_dword dwValue;
	struct _VERSION
	{
		axc_word wMajor;
		axc_word wMinor;
	} Version;
} AXC_U_VERSION;
#pragma pack()


//
// 最大、最小取值
//
#define AXC_MIN(a,b)	((a)<(b) ? (a) : (b)) // 最小取值
#define AXC_MAX(a,b)	((a)>(b) ? (a) : (b)) // 最大取值

#define AXC_MIN3(a,b,c)	((a)<(b) ? AXC_MIN(a,c) : AXC_MIN(b,c))
#define AXC_MAX3(a,b,c)	((a)>(b) ? AXC_MAX(a,c) : AXC_MAX(b,c))


//
// 目录分隔符、断行符等
//
#if defined(AXC_OS_WINDOWS)
	#define AXC_CHAR_FOLDER_SEP		'\\'
	#define AXC_STRING_FOLDER_SEP	"\\"
	#define AXC_STRING_LINE_END		"\r\n"
#elif defined(AXC_OS_LINUX)
	#define AXC_CHAR_FOLDER_SEP		'/'
	#define AXC_STRING_FOLDER_SEP	"/"
	#define AXC_STRING_LINE_END		"\n"
#elif defined (AXC_OS_APPLE)
	#define AXC_CHAR_FOLDER_SEP		'/'
	#define AXC_STRING_FOLDER_SEP	"/"
	#define AXC_STRING_LINE_END		"\r"
#endif


//
// 预置宏
//
#if defined(AXC_OS_WINDOWS)
#ifndef __func__
#define __func__ __FUNCTION__
#endif
#endif

#if defined(AXC_OS_WINDOWS)
#define AXC_THREAD_LOCAL __declspec(thread)
#else
// gcc doesn't know _Thread_local from C11 yet
#ifdef __GNUC__
#define AXC_THREAD_LOCAL __thread
#elif __STDC_VERSION__ >= 201112L
#define AXC_THREAD_LOCAL _Thread_local
#else
#error "Cannot define AXC_THREAD_LOCAL"
#endif
#endif

//
// 用 errno 保存最后错误代码
//
extern AXC_API void AxcSetLastError(axc_i32 iCode);
#define SET_LAST_ERROR(iCode)	{ AxcSetLastError((axc_i32)iCode); }
//
#if defined(AXC_OS_WINDOWS)
#define SAVE_LAST_ERROR()	{ const int iError = (int) GetLastError(); if(iError != 0) SET_LAST_ERROR(-iError); }
#else
#define SAVE_LAST_ERROR()
#endif


//
// 时间
//
#pragma pack(8)
typedef struct _AXC_T_SYSTEM_TIME_
{
	axc_word wYear;
	axc_word wMonth; // 1 - 一月 ... 12 - 十二月
	axc_word wDayOfWeek; // 0 - 星期天，1 - 星期一 ... 6 - 星期六
	axc_word wDay;
	axc_word wHour;
	axc_word wMinute;
	axc_word wSecond;
	axc_word wMilliseconds;
} AXC_T_SYSTEM_TIME;
#pragma pack()


//
// 文件或者目录的访问权限
//
#define AXC_FILEACCESS_F	(0) // 检查文件是否存在
#define AXC_FILEACCESS_X	(1) // 可执行文件
#define AXC_FILEACCESS_W	(2) // 可写
#define AXC_FILEACCESS_R	(4) // 可读

//
// 文件或者目录的信息 (struct stat)
//
#pragma pack(8)
typedef struct _AXC_T_STAT_INFO_
{
	axc_dword	dwDevIndex;		//device 文件的设备编号
	axc_dword	dwINode;		//inode 文件的i-node
	axc_dword	dwMode;			//protection 文件的类型和存取的权限
	axc_dword	dwNlink;		//number of hard links 连到该文件的硬连接数目, 刚建立的文件值为1.
	axc_dword	dwUid;			//user ID of owner 文件所有者的用户识别码
	axc_dword	dwGid;			//group ID of owner 文件所有者的组识别码
	axc_dword	dwRDev;			//device type 若此文件为装置设备文件, 则为其设备编号
	axc_dword	dwReserve;		//reserve 为8字节对齐的保留字段
	axc_ddword	ddwSize;		//total size, in bytes 文件大小, 以字节计算
	axc_ddword	ddwAccessTime;	//time of lastaccess 文件最近一次被存取或被执行的时间, 一般只有在用mknod、utime、read、write 与tructate 时改变.
	axc_ddword	ddwModifyTime;	//time of last modification 文件最后一次被修改的时间, 一般只有在用mknod、utime 和write 时才会改变
	axc_ddword	ddwCreateTime;	//time of last change i-node 最近一次被更改的时间, 此参数会在文件所有者、组、权限被更改时更新
} AXC_T_STAT_INFO;
#pragma pack()
// 解析stat信息
#define AXC_STATINFO_IS_FILE(st)	((st).dwMode & S_IFREG) // 是文件吗?
#define AXC_STATINFO_IS_DIR(st)		((st).dwMode & S_IFDIR) // 是目录吗?
#define AXC_STATINFO_CAN_READ(st)	((st).dwMode & S_IREAD) // 可读
#define AXC_STATINFO_CAN_WRITE(st)	((st).dwMode & S_IWRITE) // 可写
#define AXC_STATINFO_CAN_EXEC(st)	((st).dwMode & S_IEXEC) // 可执行
#define AXC_STATINFO_SIZE(st)		((st).ddwSize) // 文件大小
#define AXC_STATINFO_ATIME(st)		((st).ddwAccessTime) // 最后访问时间
#define AXC_STATINFO_MTIME(st)		((st).ddwModifyTime) // 最后修改时间
#define AXC_STATINFO_CTIME(st)		((st).ddwCreateTime) // 创建时间


//
// 日志级别
//
#define AXC_LOG_LEVEL_DISABLE	(0) // 关闭日志
#define AXC_LOG_LEVEL_VERBOSE	(1) // 详细日志
#define AXC_LOG_LEVEL_NORMAL	(2) // 普通日志
#define AXC_LOG_LEVEL_DEBUG		(3) // 调试
#define AXC_LOG_LEVEL_WARN		(4) // 警告
#define AXC_LOG_LEVEL_ERROR		(5) // 一般错误
#define AXC_LOG_LEVEL_FATAL		(6) // 致命错误

//
// 日志输出方式，可以多种组合
//
#define AXC_LOG_OUTPUT_FILE			(0x00000001UL) // 写到文件
#define AXC_LOG_OUTPUT_CONSOLE		(0x00000002UL) // 写到控制台窗口
#define AXC_LOG_OUTPUT_OS_DEBUGER	(0x00000004UL) // 写到操作系统的调试池（windows: OutputDebugString, linux: 无输出）


//
// 资源类型
//
#define AXC_RESOURCE_UNKNOW				(0)
#define AXC_RESOURCE_MEMORY				(1)  // CAxcMemory
#define AXC_RESOURCE_THREAD				(2)  // CAxcThread
#define AXC_RESOURCE_SOCKET				(3)  // CAxcSocket
#define AXC_RESOURCE_FILE				(4)  // CAxcFile
#define AXC_RESOURCE_MUTEX				(5)  // CAxcMutex
#define AXC_RESOURCE_EVENT				(6)  // CAxcEvent
#define AXC_RESOURCE_LIST				(7)  // CAxcList
#define AXC_RESOURCE_LIST_STRUCT		(8)  // CAxcListStruct
#define AXC_RESOURCE_FIFO				(9)  // CAxcFifo
#define AXC_RESOURCE_FIFO_BUFFER_PTR	(10) // CAxcFifoBufferPtr
#define AXC_RESOURCE_CHAIN				(11) // CAxcChain
#define AXC_RESOURCE_TREE				(12) // CAxcTree


#endif // _AXCLIB_DATA_TYPE_H_
