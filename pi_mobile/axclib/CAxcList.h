#ifndef _C_AXC_LIST_H_
#define _C_AXC_LIST_H_

#include "axclib_data_type.h"
#include "CAxcBaseResource.h"
#include "CAxcMemory.h"
#include "CAxcMutex.h"

class AXC_API CAxcList : public CAxcBaseResource
{
protected:
	#pragma pack(8)
	typedef struct _ITEM
	{
		axc_ddword ddwID;
		void* pContext;
	} ITEM;
	#pragma pack()

public:
	CAxcList(const char* szResourceName);
	virtual ~CAxcList();

	// 创建、释放
	axc_bool Create(axc_i32 iMaxItemCount);
	void Destroy();
	axc_bool IsValid() { return m_bufItems.IsValid(); }
	// 最大条目数
	AXC_INLINE axc_i32 GetMaxItemCount() { return m_iMaxItemCount; }

	// 查找
	axc_i32 Found(const axc_ddword ddwID);

	// 添加
	axc_i32 Add(const axc_ddword ddwID, void* pContext);

	// 删除
	void Remove(axc_i32 iIndex);
	axc_i32 Remove(axc_ddword ddwID);
	void RemoveAll();

	// 获取条目
	axc_i32 GetCount() { return m_iItemCount; }
	void* GetItem(axc_i32 iIndex);
	void* GetItem(axc_ddword ddwID);
	axc_ddword GetID(axc_i32 iIndex);

	// 更新
	void SetItem(axc_i32 iIndex, void* pContext);
	void SetItem(axc_ddword ddwID, void* pContext);

	// 同步锁
	CAxcMutex* GetMutex() { return &m_mutex; }

protected:
	CAxcMutex m_mutex;
	CAxcMemory m_bufItems;
	axc_i32 m_iMaxItemCount;
	axc_i32 m_iItemCount;

public:
	// 向子类请求目前的状态信息
	// 子类需要继承这个函数，向axclib库报告自己的状态
	// 返回值： 0-正常， 1-有轻微错误， 2-有严重错误
	virtual axc_dword ReportResourceStatus(char* szStatusText, const axc_dword dwTextBufferSize) { szStatusText[0] = 0; return 0; }
};

#endif // _C_AXC_LIST_H_
