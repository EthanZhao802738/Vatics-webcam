/*
 * heatfinderlogmanager.h
 *
 *  Created on: Mar 31, 2018
 *      Author: markhsieh
 *
 *  Naming Policy:
 *  https://dotblogs.com.tw/regionbbs/2009/09/06/codingstandards
 */

#ifndef HEATFINDERLOGMANAGER_H_
#define HEATFINDERLOGMANAGER_H_

#include "axclib.h"

//#ifdef __cplusplus
//extern "C" {
//#endif
#include "CAxcLog.h"
//#ifdef __cplusplus
//}
//#endif


#include <pthread.h>
#include <semaphore.h>
#include <string>
#include <list>
#include "glogdbgzone.h"

using namespace std;

typedef struct _T_LOG_DEFINE
{
	string szContext;
	unsigned int iLevel;

}T_LOG_DEFINE;


class LogManager
{
public:
	LogManager();
    ~LogManager();

    static unsigned int sm_glogDebugZoneSeed;

    bool Run(char* szAppBuildTime);
    bool Stop();

    static void AddDebugMsg(string dbgMsg, unsigned int iLevel);
    static void SetOutputLogLevel(unsigned int iValue);

private:
    CAxcLog		m_log;
	char 		m_szLogPath[128];
	bool		m_bThreadStop;

	static sem_t requestOutput;
	static pthread_mutex_t lock;
	//static list<string> listDebugMsg;
	static list<T_LOG_DEFINE> listDebugMsg;
	pthread_t thread;
	static void		*m_pLog;
	static void		*m_pThreadStop;

	static void* run(void* param);
	static void OutputDebugMessage(string dbgMsg, unsigned int iLevel);

	//ref.: https://stackoverflow.com/questions/472697/how-do-i-get-the-size-of-a-directory-in-c?answertab=votes#tab-top
	//int sum(const char *fpath, const struct stat *sb, int typeflag);

	static void SetMyLog(void *Log);
	static void* GetMyLog();
	static void SetMyThreadStopBoolean(void *ThreadStop);
	static void* GetMyThreadStopBoolean();

	static bool CheckIsAcceptOutputMessagebyThisLevel(unsigned int iLevel);
	static bool OutputData2Log(const axc_byte byLevel, const char* szFormat, ...);
	static bool RemoveTheOldLogToKeepStorageSafety(unsigned int iStorageSize); //MB

    //ref.:https://blog.csdn.net/wy5761/article/details/9320331
    int sem_timedwait_millsecs(sem_t *sem, long msecs);

    static void PrintNowDate(short iStdout);
};

void GLogImp( unsigned int dwDebugZone, unsigned int dwDebugLevel, const char* format, ... );

#ifdef ENABLE_GLOG
	#define GLog(cond, level, fmt, ...) ((void)((cond)?(GLogImp(cond, level, fmt, ##__VA_ARGS__)), 1:0))
#else
	#define GLog(...)
#endif


#endif /* HEATFINDERLOGMANAGER_H_ */
