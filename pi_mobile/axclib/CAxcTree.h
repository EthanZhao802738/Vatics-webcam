#ifndef _C_AXC_TREE_H_
#define _C_AXC_TREE_H_

#include "axclib_data_type.h"
#include "CAxcBaseResource.h"
#include "CAxcMutex.h"

class AXC_API CAxcTree : public CAxcBaseResource
{
public:
	#pragma pack(8)
	typedef struct _CONTEXT
	{
		axc_ddword	ddwUserData[16];
	} CONTEXT;
	#pragma pack()

protected:
	#pragma pack(8)
	typedef struct _ITEM
	{
		_ITEM*	pParent;
		_ITEM*	pChild;
		_ITEM*	pPrev;
		_ITEM*	pNext;
		CONTEXT	Context;
	} ITEM;
	#pragma pack()

public:
	CAxcTree(const char* szResourceName);
	virtual ~CAxcTree();

	// 分配一个新条目
	virtual axc_handle AllocItem();

	// 释放条目（包括这个条目下的所有子条目）
	virtual void FreeItem(axc_handle hItem);

	// 判断一个条目是否为根条目
	AXC_INLINE axc_bool IsRoot(axc_handle hItem) { ITEM* pItem = (ITEM*)hItem; return (pItem == &m_root || pItem == &m_rootTemp); }
	// 判断一个条目是否处于本层的顶部
	AXC_INLINE axc_bool IsTopItem(axc_handle hItem) { ITEM* pItem = (ITEM*)hItem; return (pItem && NULL == pItem->pPrev); }
	// 判断一个条目是否处于本层的底部
	AXC_INLINE axc_bool IsBottomItem(axc_handle hItem) { ITEM* pItem = (ITEM*)hItem; return (pItem && NULL == pItem->pNext); }

	// 获取根条目
	AXC_INLINE axc_handle GetRoot() { return &m_root; }
	AXC_INLINE axc_handle GetTempRoot() { return &m_rootTemp; }
	// 获取本条目的上级条目
	AXC_INLINE axc_handle GetParent(axc_handle hItem) { ITEM* pItem = (ITEM*)hItem; return (pItem ? pItem->pParent : NULL); }
	// 获取本条目的平级条目
	AXC_INLINE axc_handle GetPrev(axc_handle hItem) { ITEM* pItem = (ITEM*)hItem; return (pItem ? pItem->pPrev : NULL); }
	AXC_INLINE axc_handle GetNext(axc_handle hItem) { ITEM* pItem = (ITEM*)hItem; return (pItem ? pItem->pNext : NULL); }
	
	// 获取本条目的Context
	AXC_INLINE CONTEXT* GetConext(axc_handle hItem) { ITEM* pItem = (ITEM*)hItem; return (pItem ? &pItem->Context : NULL); }

	// 获取本条目的第一个子条目
	axc_handle GetFirstChild(axc_handle hItem);
	// 获取本条目的最后一个子条目
	axc_handle GetLastChild(axc_handle hItem);
	// 获取本条目下的索引号为iIndex的子条目
	axc_handle GetChild(axc_handle hItem, axc_i32 iChildIndex);
	// 获取本条目下的子条目数量
	// bSearchSub: 是否嵌套搜索子条目下的子条目
	axc_ddword GetChildItemCount(axc_handle hItem, axc_bool bSearchSub);
	// 获取本条目处于树中的第几层（root为0层），如果失败返回-1
	axc_i32 GetItemLevel(axc_handle hItem);
	// 获取本条目处于本层中的索引位置
	axc_i32 GetItemIndexInLevel(axc_handle hItem);

	// 移动 hSrc 到 hAfter 的后面
	axc_bool MovetoAfter(axc_handle hSrc, axc_handle hAfter);
	// 移动 hSrc 到 hBefore 的前面
	axc_bool MovetoBefore(axc_handle hSrc, axc_handle hBefore);
	// 移动 hSrc 到 hParent 的顶部
	axc_bool MovetoParentTop(axc_handle hSrc, axc_handle hParent);
	// 移动 hSrc 到 hParent 的底部
	axc_bool MovetoParentBottom(axc_handle hSrc, axc_handle hParent);

	// 同步锁
	CAxcMutex* GetMutex() { return &m_mutex; }

protected:
	// 断开本条目与上级(parent)和平级(prev,next)之间的链接
	void Offline(axc_handle hItem);

protected:
	CAxcMutex	m_mutex;

private:
	ITEM		m_root; // 树的根条目
	ITEM		m_rootTemp; // 临时树的跟条目: 保存已分配、但尚未被使用的条目
	axc_ddword	m_ddwAllocItemCount; // 已经分配的条目数
	axc_ddword	m_ddwFreeItemCount; // 已经释放的条目数

public:
	// 向子类请求目前的状态信息
	// 子类需要继承这个函数，向axclib库报告自己的状态
	// 返回值： 0-正常， 1-有轻微错误， 2-有严重错误
	virtual axc_dword ReportResourceStatus(char* szStatusText, const axc_dword dwTextBufferSize);
};


#endif // _C_AXC_TREE_H_
