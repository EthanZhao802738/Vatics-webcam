#ifndef _ADEXCHANGE_CPP_LIB_H_
#define _ADEXCHANGE_CPP_LIB_H_

#include "axclib_os_include.h"
#include "axclib_data_type.h"

#include "CAxcMemory.h"
#include "CAxcMutex.h"
#include "CAxcEvent.h"
#include "CAxcThread.h"
#include "CAxcSocket.h"
#include "CAxcFile.h"
#include "CAxcTime.h"
#include "CAxcSystem.h"
#include "CAxcFileSystem.h"
#include "CAxcUtility.h"
#include "CAxcString.h"

#include "CAxcMD5.h"
#include "CAxcCRC.h"
#include "CAxcBase64.h"

#include "CAxcAutoLock.h"
#include "CAxcLog.h"
#include "CAxcList.h"
#include "CAxcListStruct.h"
#include "CAxcFifo.h"
#include "CAxcFifoBufferPtr.h"
#include "CAxcChain.h"
#include "CAxcTree.h"


//
// 获取axclib库使用的API的版本
// 如果AxcGetApiVersion()返回的版本号和AXC_API_VERSION不一致，就不能调用后续的函数
// 
#define AXC_API_VERSION	(6) // API 6
#define AXC_IS_VERSION_MATCH() (AXC_API_VERSION == AxcGetApiVersion())
AXC_API 
axc_word AxcGetApiVersion();

//
// 获取axclib库文件的版本号
//
AXC_API 
AXC_U_VERSION AxcGetLibVersion();

//
// 设置、获取本线程的最后错误代码
//
AXC_API void AxcSetLastError(axc_i32 iCode);
AXC_API axc_i32 AxcGetLastError();

//
// 获取错误代码的描述文字
//
AXC_API
axc_bool AxcGetErrorText(const axc_i32 iError, char* szTextBuffer, axc_dword dwTextBufferSize);

//
// 分配一个线程相关的局部变量，共 256 bytes
//
AXC_API
char* AxcGetThreadLocalBuffer256B();

//
// 读取本线程的最后错误代码的描述文字，保存到 AxcGetThreadLocalBuffer256B() 的buffer中
//
AXC_API
char* AxcGetLastErrorText();

//
// 做一个全局唯一ID
//
AXC_API axc_dword  AxcMakeId32();
AXC_API axc_ddword AxcMakeId64();

//
// 4 char & 8 char tag
//
AXC_API axc_dword  AxcMakeTag32(const char* szTag);
AXC_API axc_ddword AxcMakeTag64(const char* szTag);


#endif // _ADEXCHANGE_CPP_LIB_H_
