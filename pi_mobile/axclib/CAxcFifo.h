#ifndef _C_AXC_FIFO_H_
#define _C_AXC_FIFO_H_

#include "axclib_data_type.h"
#include "CAxcBaseResource.h"
#include "CAxcMutex.h"
#include "CAxcMemory.h"

class AXC_API CAxcFifo : public CAxcBaseResource
{
protected:
	#pragma pack(8)
	typedef struct _ITEM
	{
		axc_ddword ddwContext1;
		axc_ddword ddwContext2;
		CAxcMemory* pData;
		axc_i32 iDataLen;
	} ITEM;
	#pragma pack()

public:
	CAxcFifo(const char* szResourceName);
	virtual ~CAxcFifo();

	axc_bool Create(axc_i32 nMaxItemSize = 4096, axc_i32 nMaxItemCount = 200);
	void Destroy();
	axc_bool IsValid() { return m_bufItems.IsValid(); }

	axc_bool Push(void* pData, axc_i32 nDataLen, axc_ddword ddwContext1 = 0, axc_ddword ddwContext2 = 0);
	axc_bool Pop(void* pData, axc_i32 nBufLen, axc_i32* pActualDataLen, axc_ddword* pddwContext1 = NULL, axc_ddword* pddwContext2 = NULL);
	axc_bool Peek(void* pData, axc_i32 nBufLen, axc_i32* pActualDataLen, axc_ddword* pddwContext1 = NULL, axc_ddword* pddwContext2 = NULL);

	void Reset();
	axc_i32 GetCount();

	AXC_INLINE axc_i32 GetMaxItemCount() { return m_iMaxItemCount; }
	AXC_INLINE axc_i32 GetMaxItemSize() { return m_iMaxItemSize; }

	// 同步锁
	CAxcMutex* GetMutex() { return &m_mutex; }

protected:
	CAxcMutex m_mutex;
	CAxcMemory m_bufItems;
	axc_i32 m_iMaxItemSize;
	axc_i32 m_iMaxItemCount;
	axc_i32 m_iReadPos;
	axc_i32 m_iWritePos;
	axc_i32 m_iItemCount;

public:
	// 向子类请求目前的状态信息
	// 子类需要继承这个函数，向axclib库报告自己的状态
	// 返回值： 0-正常， 1-有轻微错误， 2-有严重错误
	virtual axc_dword ReportResourceStatus(char* szStatusText, const axc_dword dwTextBufferSize) { szStatusText[0] = 0; return 0; }
};

#endif // _C_AXC_FIFO_H_
