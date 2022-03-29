#ifndef _C_AXC_MEMORY_H_
#define _C_AXC_MEMORY_H_

#include "axclib_data_type.h"
#include "CAxcBaseResource.h"

class AXC_API CAxcMemory : public CAxcBaseResource
{
public:
	CAxcMemory(const char* szResourceName);
	virtual ~CAxcMemory();

	axc_bool IsValid() { return (m_pAlignAddress != NULL && m_dwBufferValidSize > 0); }

	axc_bool Create(axc_dword dwSize, axc_dword dwAlign = AXC_DEFAULT_MEM_ALIGN);
	void Free();

	axc_bool Resize(axc_dword dwNewSize, axc_dword dwReserveDataOnBuffer);

	void* GetAddress() { return m_pAlignAddress; }
	axc_dword GetBufferSize() { return m_dwBufferValidSize; }
	axc_dword GetDataSize() { return m_dwDataSize; }
	axc_dword SetDataSize(axc_dword dwSize) { if(dwSize > m_dwBufferValidSize) {dwSize = m_dwBufferValidSize;} m_dwDataSize = dwSize; return m_dwDataSize; }
	
	void* CopyTo(void* pDest, axc_dword dwSize);
	void* CopyFrom(const void* pSrc, axc_dword dwSize);
	axc_i32 Memcmp(const void* pBuffer, axc_dword dwSize);

	// 在现有buffer的后面添加新的数据
	//     bAutoResize: 如果现有buffer的尺寸不够，会自动扩大buffer
	void* Append(const void* pSrc, axc_dword dwSize, axc_bool bAutoResize = axc_true);

	static void* SafeCopy(void* pDest, const void* pSrc, const axc_dword dwSize);

protected:
	void* m_pAllocAddress;
	void* m_pAlignAddress;
	axc_dword m_dwAllocSize;
	axc_dword m_dwBufferValidSize;
	axc_dword m_dwDataSize;
	axc_dword m_dwAlign;

public:
	// 向子类请求目前的状态信息
	// 子类需要继承这个函数，向axclib库报告自己的状态
	// 返回值： 0-正常， 1-有轻微错误， 2-有严重错误
	virtual axc_dword ReportResourceStatus(char* szStatusText, const axc_dword dwTextBufferSize) { szStatusText[0] = 0; return 0; }
};

#endif // _C_AXC_MEMORY_H_
