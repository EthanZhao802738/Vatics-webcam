#ifndef _C_AXC_FILE_H_
#define _C_AXC_FILE_H_

#include "axclib_data_type.h"
#include "CAxcBaseResource.h"

class AXC_API CAxcFile : public CAxcBaseResource
{
public:
	CAxcFile(const char* szResourceName);
	virtual ~CAxcFile();

	// 打开文件
	// szMode 参考 fopen() 函数，目前支持的参数有 "wrba+"
	axc_bool Open(const char* szFileName, const char* szMode);

	// 关闭文件
	void Close();

	// 是否已经打开了文件
	axc_bool IsValid() { return (NULL != m_fp); }
	// 文件名
	AXC_INLINE const char* FileName() { return m_szFileName; }

	// 读
	axc_dword Read(void* pBuffer, axc_dword dwReadBytes);

	// 写
	axc_dword Write(const void* pBuffer, axc_dword dwWriteBytes);
	// 刷新缓存
	void Flush();

	// 定位
	// iOrigin: SEEK_SET, SEEK_CUR, SEEK_END
	axc_bool Seek(axc_i64 iOffset, int iOrigin);

	// 目前位置
	axc_i64 Tell();

	// 获取已经打开的文件的大小
	// 备注：如果希望不打开文件、直接获取文件大小，可以调用 CAxcFileSystem::GetStatInfo()
	AXC_INLINE axc_i64 FileSize()
	{
		if (m_fp == NULL)
		{
			SET_LAST_ERROR(ENOENT);
			return -1;
		}
		axc_i64 iCurrPos = Tell();
		Seek(0, SEEK_END);
		const axc_i64 iFileSize = Tell();
		Seek(iCurrPos, SEEK_SET);
		return iFileSize;
	}

protected:
	FILE*	m_fp;
	char	m_szFileName[MAX_PATH];

public:
	// 向子类请求目前的状态信息
	// 子类需要继承这个函数，向axclib库报告自己的状态
	// 返回值： 0-正常， 1-有轻微错误， 2-有严重错误
	virtual axc_dword ReportResourceStatus(char* szStatusText, const axc_dword dwTextBufferSize) { szStatusText[0] = 0; return 0; }
};

#endif // _C_AXC_FILE_H_
