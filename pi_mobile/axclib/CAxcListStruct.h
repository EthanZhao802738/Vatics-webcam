#ifndef _C_AXC_LIST_STRUCT_H_
#define _C_AXC_LIST_STRUCT_H_

#include "axclib_data_type.h"
#include "CAxcBaseResource.h"
#include "CAxcMemory.h"
#include "CAxcMutex.h"

//
// 类 CAxcListStruct，管理一个结构数据：
// 1、结构长度固定
// 2、结构内必须有一个成员是具有唯一ID的、可作为索引值用。该成员位置和长度不限
//

// 计算一个结构体内的某一个成员的偏移量
#define AXC_MEMBER_OFFSET(struct_name, member_name) (axc_i32)(&(((##struct_name*)0)->##member_name))

// 计算结构体内某一个成员的长度
#define AXC_MEMBER_SIZE(struct_name, member_name) sizeof(((##struct_name*)0)->##member_name)


class AXC_API CAxcListStruct : public CAxcBaseResource
{
public:
	CAxcListStruct(const char* szResourceName);
	virtual ~CAxcListStruct();

	// 创建
	axc_bool Create(axc_i32 iItemSize, axc_i32 iMaxItemCount, axc_i32 iIdOffsetWithinItem, axc_i32 iIdSize);
	// 释放
	void Destroy();
	// 是否已经被创建
	axc_bool IsValid() { return m_bufItems.IsValid(); }

	// 添加
	axc_i32 Add(void* pNewItem);
	// 删除
	// bSort: axc_true -- 维持原来的排列顺序, axc_false -- 不维持原来的排列顺序，用最后一个Item取代被删除的这个
	axc_i32 Remove(axc_i32 iIndex, axc_bool bSort);
	void RemoveAll();
	// 查找
	axc_i32 Find(void* IdPtr);
	// 获取条目
	void* GetItem(axc_i32 iIndex);
	axc_bool GetItem(axc_i32 iIndex, void* pOutBuffer, axc_dword dwOutBufferSize);
	// 获取条目总数
	axc_i32 GetItemCount();
	// 最大条目数
	AXC_INLINE axc_i32 GetMaxItemCount() { return m_iMaxItemCount; }

	// 同步锁
	CAxcMutex* GetMutex() { return &m_mutex; }

protected:
	CAxcMutex m_mutex;
	CAxcMemory m_bufItems;
	axc_i32 m_iItemCount;
	axc_i32 m_iItemSize;
	axc_i32 m_iMaxItemCount;
	axc_i32 m_iIdOffsetWithinItem;
	axc_i32 m_iIdSize;

public:
	// 向子类请求目前的状态信息
	// 子类需要继承这个函数，向axclib库报告自己的状态
	// 返回值： 0-正常， 1-有轻微错误， 2-有严重错误
	virtual axc_dword ReportResourceStatus(char* szStatusText, const axc_dword dwTextBufferSize) { szStatusText[0] = 0; return 0; }
};

#endif //_C_AXC_LIST_STRUCT_H_
