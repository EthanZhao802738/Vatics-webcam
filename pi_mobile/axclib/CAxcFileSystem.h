#ifndef _C_AXC_FILE_SYSTEM_H_
#define _C_AXC_FILE_SYSTEM_H_

#include "axclib_data_type.h"

class AXC_API CAxcFileSystem
{
public:
	CAxcFileSystem();
	~CAxcFileSystem();

	//==========
	// 文件操作
	//==========

	// 删除文件
	static axc_bool RemoveFile(const char* szFileName);
	// 批量删除文件
	// szPathFiles 应为全路径名; 支持通配符 (比如 "/tmp/*.tmp")
	// 返回值：被删除的文件的数量，小于0表示有错误发生
	static axc_i32 RemoveMatchedFiles(const char* szPathFilesPattern);

	// 复制一个文件
	// bOverwrite: 如果目标文件szDestFile已经存在的话，是否覆盖目标文件. axc_true - 覆盖，axc_false - 不覆盖
	static axc_bool CopyFileToFile(const char* szSourceFile, const char* szDestFile, axc_bool bOverwrite);
	// 批量复制文件到指定目录下，不包括子目录、和子目录下的文件
	// szSourceFilesPattern支持通配符
	static axc_i32 CopyMatchedFilesToFolder(const char* szSourceFilesPattern, const char* szDestFolder, axc_bool bOverwrite);

	// 移动文件
	static axc_bool MoveFileToFile(const char* szSourceFile, const char* szNewFile);
	// 批量移动文件
	// szSourceFilesPattern 支持通配符
	static axc_i32 MoveMatchedFilesToFolder(const char* szSourceFilesPattern, const char* szDestFolder);

	// 更名，可以用于文件、也可以用于目录; szSource必须存在、szDest必须不存在
	static axc_bool RenameFileOrFolder(const char* szSource, const char* szDest);


	//==========
	// 目录操作
	//==========

	// 创建目录
	// bCreateAllPath: 是否创建整个目录树
	static axc_bool CreateFolder(const char* szNewFolder, axc_bool bCreateAllPath);

	// 删除空目录 (目录不为空时直接返回 axc_false)
	static axc_bool RemoveEmptyFolder(const char* szFolder);
	// 删除整个目录，包括目录下的所有文件和子目录
	// piRemoveFolderNumber: 被删除的目录数量
	// piRemoveFileNumber： 被删除的文件数量
	// pddwRemoveBytes: 总共删除文件的字节数
	static axc_bool RemoveEntireFolder(const char* szFolder, axc_i32* piRemoveFolderNumber = NULL, axc_i32* piRemoveFileNumber = NULL, axc_ddword* pddwRemoveBytes = NULL);

	// 复制目录
	// 比如 CopyFolder("/a/b","/c") 会生成新目录 "/c/b"，并把 "/a/b" 目录下的所有文件和子目录复制到 "/c/b"
	static AXC_INLINE axc_bool CopyFolder(const char* szSourceFolder, const char* szDestParentFolder, axc_bool bOverwrite)
	{
		return CopyOrMoveFolder(axc_false, szSourceFolder, szDestParentFolder, bOverwrite);
	}

	// 移动目录
	// 比如 MoveFolder("/a/b","/c") 会生成新目录 "/c/b"，并把 "/a/b" 目录下的所有文件和子目录移动到 "/c/b"，并删除 "/a/b"
	static AXC_INLINE axc_bool MoveFolder(const char* szSourceFolder, const char* szDestParentFolder, axc_bool bOverwrite)
	{
		return CopyOrMoveFolder(axc_true, szSourceFolder, szDestParentFolder, bOverwrite);
	}

	// 搜索目录下的所有文件和子目录
	// szPattern 指定搜寻的文件（目录）名称，可以包含通配符(*?)。等于NULL表示搜索所有的文件（目录）
	static axc_handle SearchOpen(const char* szPathName, const char* szPattern = NULL);
	// 逐个获取文件或者子目录的名字. 如果返回NULL，表示没有新的结果了
	// 之后可以调用 GetStatInfo() 去判断是文件还是目录、以及获取文件的大小、时间等信息
	static const char* SearchGetNextName(axc_handle hSearch, axc_bool* pbIsDir = NULL, axc_ddword* pddwFileSize = NULL);
	// 释放搜索资源
	static void SearchClose(axc_handle hSearch);


	//==========
	// 辅助函数
	//==========

	// 获取本进程的执行文件的全路径名
	static axc_bool GetCurrentProcessPathFileName(char* szPathFileName, axc_dword dwBufferSize);

	// 从一个文件全路径名中，分离出目录名、文件名、目录分隔符
	static axc_bool ParsePathFileName(const char* szPathFileName, char* szPath, axc_dword dwPathBufferSize, char* szFile, axc_dword dwFileBufferSize, char* pchFolderSeparator);

	// 用目录名、文件名（或者子目录名）合成一个全路径名
	static axc_bool MakePathFileName(const char* szPathName, const char* szFileName, char* szPathFileName, axc_dword dwBufferSize);

	// 获取和设置本进程的工作目录
	static axc_bool GetCurrentWorkPath(char* szPathName, axc_dword dwBufferSize);
	static axc_bool SetCurrentWorkPath(const char* szPathName);

	// 检查文件或者目录的属性（是否存在、是否有读/写/执行的权限）
	// iMode可以为以下组合条件: 0-是否存在(F_OK)，1-是否可执行(X_OK)，2-是否可写(W_OK)，4-是否可读(R_OK)
	static axc_bool AccessCheck(const char* szFileName, axc_i32 iMode);
	static AXC_INLINE axc_bool AccessCheck_IsExisted(const char* szFileName) { return AccessCheck(szFileName, AXC_FILEACCESS_F); }
	static AXC_INLINE axc_bool AccessCheck_CanExecute(const char* szFileName) { return AccessCheck(szFileName, AXC_FILEACCESS_X); }
	static AXC_INLINE axc_bool AccessCheck_CanWrite(const char* szFileName) { return AccessCheck(szFileName, AXC_FILEACCESS_W); }
	static AXC_INLINE axc_bool AccessCheck_CanRead(const char* szFileName) { return AccessCheck(szFileName, AXC_FILEACCESS_R); }
	static AXC_INLINE axc_bool AccessCheck_CanReadWrite(const char* szFileName) { return AccessCheck(szFileName, AXC_FILEACCESS_W | AXC_FILEACCESS_R); }

	// 获取文件或者目录的信息：类型（文件还是目录）、大小、时间等
	// 可以借助 AXC_STATINFO_xxx 宏解析 AXC_T_STAT_INFO 结构
	static axc_bool GetStatInfo(const char* szFileOrDirectoryName, AXC_T_STAT_INFO* pInfo);

private:
	// 复制或者移动目录
	static axc_bool CopyOrMoveFolder(axc_bool bMove, const char* szSourceFolder, const char* szDestParentFolder, axc_bool bOverwrite);
};

#endif // _C_AXC_FILE_SYSTEM_H_
