#ifndef _C_AXC_STRING_H_
#define _C_AXC_STRING_H_

#include "axclib_data_type.h"
#include "CAxcMemory.h"


class AXC_API CAxcString
{
public:
	//==========
	// 全局函数
	//==========

	// 字符串加密和解密（源字符串不包括末尾的\0，最大可以为 124 char；加密后的字符串固定为 256 char）
	static axc_bool Encrypt256(const char* szSrc, char szDest[256]);
	// 字符串解密，返回解密后的字符串的长度（不包括末尾的\0）；返回 -1 表示解密失败
	static axc_i32 Decrypt256(const char szSrc[256], char* szDest);

	// 字符串复制
	static char* strcpy(char* szDest, const char* szSrc);
	static char* strncpy(char* szDest, const char* szSrc, axc_dword dwDestBufferSize);
	// 字符串格式化
	static axc_i32 sprintf(char* szDest, const char* szFormat, ...);
	static axc_i32 snprintf(char* szDest, axc_dword dwDestBufferSize, const char* szFormat, ...);
	// 字符串比较
	static axc_i32 strcmp(const char* szString1, const char* szString2, axc_bool bIgnoreCase = axc_false);
	static axc_i32 strncmp(const char* szString1, const char* szString2, const axc_dword dwSize, axc_bool bIgnoreCase = axc_false);
	// 字符串添加
	static char* strcat(char* szDest, const char* szSrc);
	static char* strncat(char* szDest, const char* szSrc, axc_dword dwDestBufferSize);

	// 字符串转为数字
	static axc_i32 StringToInt32(const char* szString);
	static axc_u32 StringToUint32(const char* szString);
	static axc_i64 StringToInt64(const char* szString);
	static axc_u64 StringToUint64(const char* szString);

	// 计算字符串长度，不包括最后的 \0 结束符
	static axc_dword GetTextLength(char* szBuffer, axc_dword dwBufferSize);

public:
	CAxcString();
	CAxcString(const char* szString);
	CAxcString(CAxcString& strCopy);
	virtual ~CAxcString();

	// 获取字符串长度，不包括最后的 \0 结束符
	axc_dword GetLength();
	// 获取字符串
	const char* Get();
	// 字符串赋值. szNew等于NULL会释放内部buffer
	const char* Set(const char* szNew, const axc_i32 iNewTextLen = -1);
	// 格式化字符串
	const char* Format(const char* szFormat, ...);
	// 在字符串末尾添加新的字符串
	const char* Append(const char* szAdd, const axc_i32 iAddTextLen = -1);
	// 格式化新的字符串，并添加到老字符的末尾
	const char* AppendFormat(const char* szFormat, ...);

	// 直接获取buffer指针
	char* GetBuffer();
	axc_dword GetBufferSize();
	// !!!
	// !!! 更新buffer内容后，需要调用UpdateTextLength()去从新计算字符串的长度
	// !!!
	void UpdateTextLength() { m_dwTextLen = GetTextLength((char*)m_buffer.GetAddress(), m_buffer.GetBufferSize()); }
	// 扩大buffer尺寸
	axc_bool ResizeBuffer(axc_dword dwNewSize);

	// 重载操作符
	operator const char* () { return Get(); }
	const char* operator = (const char* szNew) { return Set(szNew); }
	const char* operator = (CAxcString& strCopy) { return Set(strCopy.Get()); }
	const char* operator += (const char* szNew) { return Append(szNew); }
	const char* operator += (CAxcString& strCopy) { return Append(strCopy.Get()); }
	axc_bool operator == (const char* szCmp) { return (0 == CAxcString::strcmp(Get(), szCmp)) ? axc_true : axc_false; }
	axc_bool operator == (CAxcString& strCmp) { return (0 == CAxcString::strcmp(Get(), strCmp.Get())) ? axc_true : axc_false; }
	axc_bool operator != (const char* szCmp) { return (0 != CAxcString::strcmp(Get(), szCmp)) ? axc_true : axc_false; }
	axc_bool operator != (CAxcString& strCmp) { return (0 != CAxcString::strcmp(Get(), strCmp.Get())) ? axc_true : axc_false; }

protected:
	CAxcMemory	m_buffer;
	axc_dword	m_dwTextLen;
	char		m_szEmptyBuffer[4];
};

#endif // _C_AXC_STRING_H_
