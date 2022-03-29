#ifndef _C_AXC_CHAIN_H_
#define _C_AXC_CHAIN_H_

#include "axclib_data_type.h"
#include "CAxcBaseResource.h"
#include "CAxcMutex.h"
#include "CAxcMemory.h"

class AXC_API CAxcChain : public CAxcBaseResource
{
public:
	#pragma pack(4)
	typedef struct _NODE
	{
		void* data;
		_NODE* prev;
		_NODE* next;
		axc_dword dwAllocSize;
		axc_dword dwActualSize;
	} NODE;
	#pragma pack()

public:
	CAxcChain(const char* szResourceName);
	virtual ~CAxcChain();

	axc_bool Create(axc_i32 nAllocNodeCount, axc_i32 nNodeDataSize);
	void Destroy();
	axc_i32 GetChainLength();

	void AddToHead(_NODE* p);
	void AddToTail(_NODE* p);
	void InsertBefore(_NODE* exist, _NODE* p);
	void InsertAfter(_NODE* exist, _NODE* p);

	_NODE* RemoveFromHead();
	_NODE* RemoveFromTail();
	_NODE* Remove(_NODE* p);

	_NODE* PeekHead();
	_NODE* PeekTail();

	static _NODE* PeekPrev(_NODE* p);
	static _NODE* PeekNext(_NODE* p);
	static _NODE* AllocNode(axc_dword dwAllocSize);
	static void   FreeNode(_NODE* p);

	// 同步锁
	CAxcMutex* GetMutex() { return &m_mutex; }

protected:
	CAxcMutex m_mutex;
	_NODE*  m_first;
	_NODE*  m_last;

public:
	// 向子类请求目前的状态信息
	// 子类需要继承这个函数，向axclib库报告自己的状态
	// 返回值： 0-正常， 1-有轻微错误， 2-有严重错误
	virtual axc_dword ReportResourceStatus(char* szStatusText, const axc_dword dwTextBufferSize) { szStatusText[0] = 0; return 0; }
};

#endif // _C_AXC_CHAIN_H_
