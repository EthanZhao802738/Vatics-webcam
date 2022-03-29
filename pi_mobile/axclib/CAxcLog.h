#ifndef _C_AXC_LOG_H_
#define _C_AXC_LOG_H_

#include "axclib_data_type.h"
#include "CAxcMemory.h"
#include "CAxcFile.h"
#include "CAxcMutex.h"

#if defined(AXC_OS_LINUX)
// 方便调用 CAxcLog::Output 的宏
#define AxcLogF(log, szFormat, arg...)	log.Output(AXC_LOG_LEVEL_FATAL, szFormat, ##arg)
#define AxcLogE(log, szFormat, arg...)	log.Output(AXC_LOG_LEVEL_ERROR, szFormat, ##arg)
#define AxcLogW(log, szFormat, arg...)	log.Output(AXC_LOG_LEVEL_WARN, szFormat, ##arg)
#define AxcLogD(log, szFormat, arg...)	log.Output(AXC_LOG_LEVEL_DEBUG, szFormat, ##arg)
#define AxcLogN(log, szFormat, arg...)	log.Output(AXC_LOG_LEVEL_NORMAL, szFormat, ##arg)
#define AxcLogV(log, szFormat, arg...)	log.Output(AXC_LOG_LEVEL_VERBOSE, szFormat, ##arg)
// 添加源文件名、函数名、行号的宏
#define AxcLogF2(log, szFormat, arg...)	log.Output(AXC_LOG_LEVEL_FATAL, "[%s] [%s] [Line#%d]" AXC_STRING_LINE_END szFormat, __FILE__, __func__, __LINE__, ##arg)
#define AxcLogE2(log, szFormat, arg...)	log.Output(AXC_LOG_LEVEL_ERROR, "[%s] [%s] [Line#%d]" AXC_STRING_LINE_END szFormat, __FILE__, __func__, __LINE__, ##arg)
#define AxcLogW2(log, szFormat, arg...)	log.Output(AXC_LOG_LEVEL_WARN, "[%s] [%s] [Line#%d]" AXC_STRING_LINE_END szFormat, __FILE__, __func__, __LINE__, ##arg)
#define AxcLogD2(log, szFormat, arg...)	log.Output(AXC_LOG_LEVEL_DEBUG, "[%s] [%s] [Line#%d]" AXC_STRING_LINE_END szFormat, __FILE__, __func__, __LINE__, ##arg)
#define AxcLogN2(log, szFormat, arg...)	log.Output(AXC_LOG_LEVEL_NORMAL, "[%s] [%s] [Line#%d]" AXC_STRING_LINE_END szFormat, __FILE__, __func__, __LINE__, ##arg)
#define AxcLogV2(log, szFormat, arg...)	log.Output(AXC_LOG_LEVEL_VERBOSE, "[%s] [%s] [Line#%d]" AXC_STRING_LINE_END szFormat, __FILE__, __func__, __LINE__, ##arg)
#else
// 方便调用 CAxcLog::Output 的宏
#define AxcLogF(log, szFormat, ...)		log.Output(AXC_LOG_LEVEL_FATAL, szFormat, ##__VA_ARGS__)
#define AxcLogE(log, szFormat, ...)		log.Output(AXC_LOG_LEVEL_ERROR, szFormat, ##__VA_ARGS__)
#define AxcLogW(log, szFormat, ...)		log.Output(AXC_LOG_LEVEL_WARN, szFormat, ##__VA_ARGS__)
#define AxcLogD(log, szFormat, ...)		log.Output(AXC_LOG_LEVEL_DEBUG, szFormat, ##__VA_ARGS__)
#define AxcLogN(log, szFormat, ...)		log.Output(AXC_LOG_LEVEL_NORMAL, szFormat, ##__VA_ARGS__)
#define AxcLogV(log, szFormat, ...)		log.Output(AXC_LOG_LEVEL_VERBOSE, szFormat, ##__VA_ARGS__)
// 添加源文件名、函数名、行号的宏
#define AxcLogF2(log, szFormat, ...)	log.Output(AXC_LOG_LEVEL_FATAL, "[%s] [%s] [Line#%d]" AXC_STRING_LINE_END szFormat, __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#define AxcLogE2(log, szFormat, ...)	log.Output(AXC_LOG_LEVEL_ERROR, "[%s] [%s] [Line#%d]" AXC_STRING_LINE_END szFormat, __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#define AxcLogW2(log, szFormat, ...)	log.Output(AXC_LOG_LEVEL_WARN, "[%s] [%s] [Line#%d]" AXC_STRING_LINE_END szFormat, __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#define AxcLogD2(log, szFormat, ...)	log.Output(AXC_LOG_LEVEL_DEBUG, "[%s] [%s] [Line#%d]" AXC_STRING_LINE_END szFormat, __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#define AxcLogN2(log, szFormat, ...)	log.Output(AXC_LOG_LEVEL_NORMAL, "[%s] [%s] [Line#%d]" AXC_STRING_LINE_END szFormat, __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#define AxcLogV2(log, szFormat, ...)	log.Output(AXC_LOG_LEVEL_VERBOSE, "[%s] [%s] [Line#%d]" AXC_STRING_LINE_END szFormat, __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#endif


class AXC_API CAxcLog
{
public:
	CAxcLog(const char* szResourceName);
	virtual ~CAxcLog();

	// 设定日志级别
	axc_bool SetOutputLevel(axc_byte byNewLevel);
	axc_byte GetOutputLevel() { return m_byOutputLevel; }

	// 获取系统支持的日志输出方式，为 AXC_LOG_OUTPUT_xxx 的组合
	static axc_dword GetSupportOutputMode();
	// 设定日志输出方式，可以多种组合
	axc_bool SetOutputMode(axc_dword dwNewMode);
	axc_dword GetOutputMode() { return m_dwOutputMode; }

	// 设置日志文件的规则
	// szFolder: 文件所在的目录
	// szFilePrefix: 文件名的前缀。实际文件名称为 “目录 + 前缀 + 日期 + 索引号”，比如 “d:\LogFolder\Prefix-20160629-1.log”
	// dwSizeLimit: 每个文件的最大尺寸（Bytes），超过这个尺寸会关闭老文件、创建新文件。设置为0时，使用默认值 5*1024*1024
	axc_bool SetFileName(const char* szFolder, const char* szFilePrefix, const axc_dword dwSizeLimit);

	// 写日志
	axc_bool Output(const axc_byte byLevel, const char* szFormat, ...);

private:
	axc_bool OutputToFile(const axc_dword dwTodayDate, const char* szLog, const axc_i32 iStringBytes);
	axc_i32 FindTodayFileLastIndex(const axc_dword dwTodayDate, axc_ddword* pddwFileSize);

	void OutputToConsole(const axc_byte byLevel, char* szLog, const axc_i32 iStringBytes);

protected:
	static CAxcMutex s_m_mutexGlobal;
	axc_byte m_byOutputLevel;
	axc_dword m_dwOutputMode;

	CAxcMemory m_text;
	CAxcFile m_file;
	CAxcMutex m_mutex;

	char m_szFolder[MAX_PATH];
	char m_szFilePrefix[MAX_PATH];
	axc_ddword m_ddwFileSizeLimit;

	axc_i32 m_iFileIndex;
	axc_dword m_dwFileDate;
	axc_ddword m_ddwPrevTryOpenFileTime;
};

#endif // _C_AXC_LOG_H_
