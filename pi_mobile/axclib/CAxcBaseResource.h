#ifndef _C_AXC_BASE_RESOURCE_H_
#define _C_AXC_BASE_RESOURCE_H_

#include "axclib_data_type.h"

//
// 资源的基类，用于统计各种资源的使用情况
// 比如：memory, thread, socket, file, mutex, event, list, fifo, chain ...
// 可以记录资源的状态（创建、释放、活动情况等）
//

class AXC_API CAxcBaseResource
{
public:
	CAxcBaseResource(axc_dword dwType, const char* szCounterList, const char* szName = NULL);
	virtual ~CAxcBaseResource();

	// 资源的类型: AXC_RESOURCE_xxx
	AXC_INLINE axc_dword GetResourceType() { return m_dwResourceType; }
	AXC_INLINE const char* GetResourceTypeText() { return ResourceTypeTextFromIndex(m_dwResourceType); }
	// 资源类型的索引值和描述文字的转换
	static axc_dword ResourceTypeIndexFromText(const char* szType);
	static const char* ResourceTypeTextFromIndex(axc_dword dwType);

	// 资源的名称，一般为 "模块名称/资源名称"，比如 "demo/main-ui-thread"
	void SetResourceName(const char* szName);
	AXC_INLINE const char* GetResourceName() { return m_szResourceName; }

	// 向子类请求目前的状态信息
	// 子类需要继承这个函数，向axclib库报告自己的状态
	// 返回值： 0-正常， 1-有轻微错误， 2-有严重错误
	virtual axc_dword ReportResourceStatus(char* szStatusText, const axc_dword dwTextBufferSize) = 0;

	// 用户私有数据
	axc_bool AllocUserData(axc_dword dwSize);
	void FreeUserData();
	axc_dword GetUserDataSize() { return m_dwUserDataSize; }
	void* GetUserData() { return m_pUserData; }

protected:
	// 资源的类型和名称
	axc_dword m_dwResourceType;
	char m_szResourceName[64];
	// 用户私有数据
	void* m_pUserData;
	axc_dword m_dwUserDataSize;
};

#endif // _C_AXC_BASE_RESOURCE_H_
