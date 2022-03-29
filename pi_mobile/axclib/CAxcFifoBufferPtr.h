#ifndef _C_AXC_FIFO_BUFFER_PTR_H_
#define _C_AXC_FIFO_BUFFER_PTR_H_

#include "axclib_data_type.h"
#include "CAxcBaseResource.h"
#include "CAxcMutex.h"
#include "CAxcMemory.h"
#include "CAxcEvent.h"

class AXC_API CAxcFifoBufferPtr : public CAxcBaseResource
{
protected:
	#pragma pack(8)
	typedef struct _ITEM
	{
		axc_ddword	ddwContext;
		void*		pContext;
		CAxcMemory*	pAllocBuffer;
		axc_i32		iValidDataLen;
	} ITEM;
	#pragma pack()

public:
	CAxcFifoBufferPtr(const char* szResourceName);
	virtual ~CAxcFifoBufferPtr();

	axc_bool Create(axc_i32 iMaxItemCount, axc_i32 iAlignBytes = 16, axc_bool bSyncEvent = axc_false);
	void Destroy();
	axc_bool IsValid() { return m_bufItems.IsValid(); }

	// max item count
	AXC_INLINE axc_i32 GetMaxItemCount() { return m_iMaxItemCount; }

	// copy input data to my-buffer
	axc_bool Push(void* pData, axc_i32 iDataLen, void* pContext1 = NULL, axc_ddword ddwContext2 = 0);

	// output my-buffer ptr, no copy
	axc_bool Peek(void** ppData, axc_i32* pActualDataLen, void** ppContext1 = NULL, axc_ddword* pddwContext2 = NULL);
	axc_bool Pop(axc_bool bFreeBuffer);

	axc_bool Wait(axc_dword dwTimeout = 0xFFFFFFFFUL);

	void Reset();
	axc_i32  GetCount();

	// 同步锁
	CAxcMutex* GetMutex() { return &m_mutex; }

protected:
	CAxcMutex m_mutex;
	CAxcMemory m_bufItems;
	CAxcEvent* m_pEvent;
	axc_i32 m_iMaxItemCount;
	axc_i32 m_iReadPos;
	axc_i32 m_iWritePos;
	axc_i32 m_iItemCount;
	axc_i32 m_iAlignBytes;

public:
	// 向子类请求目前的状态信息
	// 子类需要继承这个函数，向axclib库报告自己的状态
	// 返回值： 0-正常， 1-有轻微错误， 2-有严重错误
	virtual axc_dword ReportResourceStatus(char* szStatusText, const axc_dword dwTextBufferSize) { szStatusText[0] = 0; return 0; }
};


#endif // _C_AXC_FIFO_BUFFER_PTR_H_
