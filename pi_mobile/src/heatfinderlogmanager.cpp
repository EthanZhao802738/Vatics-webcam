/*
 * heatfinderlogmanager.cpp
 *
 *  Created on: Mar 31, 2018
 *      Author: markhsieh
 *
 *  Ref.: Gavin Chang on 2018/4/9.
 *  		GLogService
 */

#include "globaldef.h"
#include "heatfinderlogmanager.h"
#include "heatfinderconfigmanager.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void GLogImp( unsigned int dwDebugZone, unsigned int dwDebugLevel, const char* format, ... ) {
	//printf("D: [%s] %s start %d \n", "Log Manager", __func__, __LINE__);
	if( dwDebugZone == 0 )
		return;

	char buffer[10240] = {0};
	std::string debugMsg;
	va_list args;
	va_start (args, format);
	vsprintf (buffer,format, args);
	debugMsg = std::string(buffer);
	va_end (args);
	//printf("D: [%s] %s start %d \n", "Log Manager", __func__, __LINE__);
	LogManager::AddDebugMsg(debugMsg, dwDebugLevel);
	//printf("D: [%s] %s start %d \n", "Log Manager", __func__, __LINE__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int LogManager::sm_glogDebugZoneSeed = tAll & ~(tRingBuffer_MSK|tH264NAL_MSK); //tNone;
sem_t LogManager::requestOutput;
pthread_mutex_t LogManager::lock;
list<T_LOG_DEFINE> LogManager::listDebugMsg;
//CAxcLog		m_log("process/log");
void		*LogManager::m_pLog=NULL;
void		*LogManager::m_pThreadStop = NULL;

LogManager::LogManager():
		m_log("process/log")
{
	//m_bThreadStop = false;
	SetMyLog(&m_log);
	SetMyThreadStopBoolean(&m_bThreadStop);
	memset(m_szLogPath, '0', sizeof(m_szLogPath));
	sem_init( &requestOutput, 0, 0 );
	pthread_mutex_init( &lock, NULL );

}

LogManager::~LogManager(){
	Stop();
}


bool LogManager::Run(char* szAppBuildTime){
	//printf("D: [%s] %s start \n", "Log Manager", __func__);
	m_bThreadStop = false;

	Cheatfinderconfigmanager *pConfigManager = (Cheatfinderconfigmanager *) CHeatFinderUtility::GetConfigObj();
	T_CONFIG_FILE_OPTIONS *pConfig = pConfigManager->GetConfigContext();

	//
	// setting: log level & file
	//
	AxcSetLastError(0);
	if (axc_false == m_log.SetOutputMode(CAxcLog::GetSupportOutputMode())){
		fprintf(stderr, "E: [SetOutputMode] fail to use AxcLog object: %s\n", AxcGetLastErrorText());
	}
	if (axc_false == m_log.SetOutputLevel(pConfig->dwLogLevel)){
		fprintf(stderr, "E: [SetOutputLevel] fail to use AxcLog object: %s\n", AxcGetLastErrorText());
	}

	// def: static axc_i32 snprintf(char* szDest, axc_dword dwDestBufferSize, const char* szFormat, ...);
	CAxcString::snprintf(m_szLogPath, (axc_dword)sizeof(m_szLogPath), "%s%clog", "/home/pi/ade_camera_udp_out", AXC_CHAR_FOLDER_SEP);

	if(!CAxcFileSystem::AccessCheck_IsExisted(m_szLogPath))
	{
		CAxcFileSystem::CreateFolder(m_szLogPath, axc_false);
	}

	// 设置日志文件的规则
	// szFolder: 文件所在的目录
	// szFilePrefix: 文件名的前缀。实际文件名称为 “目录 + 前缀 + 日期 + 索引号”，比如 “d:\LogFolder\Prefix-20160629-1.log”
	// dwSizeLimit: 每个文件的最大尺寸（Bytes），超过这个尺寸会关闭老文件、创建新文件。设置为0时，使用默认值 5*1024*1024
	// def: axc_bool SetFileName(const char* szFolder, const char* szFilePrefix, const axc_dword dwSizeLimit);
	if (axc_false == m_log.SetFileName(m_szLogPath, "adehf_mobile", (3*1024*1024))){
		fprintf(stderr, "E: [SetFileName] fail to use AxcLog object: %s\n", AxcGetLastErrorText());
	}

	// start thread: log queue
	pthread_create( &thread, NULL, LogManager::run, this );

	// startup log
    FILE *l_fp = NULL;  //hamlet
    char L_token[128] = {""};
    CAxcString cszManualName;
	if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/home/pi/heat-finder-runScripts/rest-server/config/camera_config.json"))) {
		l_fp = popen("jq -r .name /home/pi/heat-finder-runScripts/rest-server/config/camera_config.json | sed 's/HF048-P/HF048/g'", "r");
	}else if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-merge/config/camera_config.json"))) {
	    l_fp = popen("jq -r .name /home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-merge/config/camera_config.json | sed 's/HF048-P/HF048/g'", "r");
	}else if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-release/config/camera_config.json"))) {
	    l_fp = popen("jq -r .name /home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-release/config/camera_config.json | sed 's/HF048-P/HF048/g'", "r");
	}

    if (l_fp == NULL) {
    	//CHeatFinderUtility::PrintNowData(1);
    	GLog( tAll, tERRORTrace_MSK, "E: [%s] %s: failed to get manual dev. name, error:[%d] %s\n", "Broadcast msg.", __func__, errno, "can not read string by popen()");
    }
    else
    {
          while (fgets(L_token, sizeof(L_token)-1, l_fp) != NULL) {
              char *pos;
              if ((pos=strchr(L_token, '\n')) != NULL)
              {    *pos = '\0';}
              if (strlen(L_token))
              {
                  //cszManualName =L_token;
                  cszManualName.Append(L_token, strlen(L_token));
              }
          }
          pclose(l_fp);     //hamlet
    }

	//printf("D: [%s] %s setting finish \n", "Log Manager", __func__);
    GLog( tAll, tDEBUGTrace_MSK, "\n==================  Copyright (c) ADE Technology inc.  ==================\n");
    GLog( tAll, tDEBUGTrace_MSK, "[Main] app startup, hw-type %s, fw-version %s build %s", cszManualName.Get(), SYS_EXECUTE_VERSION, szAppBuildTime);

	return true;
}

bool LogManager::Stop(){
	m_bThreadStop = true;
	sem_post(&requestOutput);
	pthread_cancel(thread);
	pthread_join(thread, NULL);
	sem_destroy(&requestOutput);
	return true;
}

void LogManager::SetMyLog(void *Log){
	m_pLog = Log;
}

void* LogManager::GetMyLog(){
	return m_pLog;
}

void LogManager::SetMyThreadStopBoolean(void *ThreadStop){
	m_pThreadStop = ThreadStop;
}

void* LogManager::GetMyThreadStopBoolean(){
	return m_pThreadStop;
}

void* LogManager::run(void* param) {
    void *pContext = CHeatFinderUtility::GetGlobalsupport();
    GlobalDef *pGlobal = reinterpret_cast<GlobalDef*> (pContext);
    pGlobal->AddpThreadRecord("Log Manager");

	bool *pThreadStop = (bool *) GetMyThreadStopBoolean();

	while(*pThreadStop == false){
		sem_wait(&requestOutput);
		pthread_mutex_lock(&lock);
		//string debugMsg = listDebugMsg.front();
		T_LOG_DEFINE node = listDebugMsg.front();
		string debugMsg = node.szContext;
		listDebugMsg.pop_front();
		pthread_mutex_unlock(&lock);

		if (LogManager::CheckIsAcceptOutputMessagebyThisLevel(node.iLevel) == false){
			//pass this time
		}else{
			LogManager::OutputDebugMessage(debugMsg, node.iLevel);
		}
		//usleep(5000); //avoid too busy: I/O
	}
	return NULL;
}

void LogManager::AddDebugMsg(string dbgMsg, unsigned int iLevel) {
	//printf("D: [%s] %s start %d \n", "Log Manager", __func__, __LINE__);
	pthread_mutex_lock(&lock);
	T_LOG_DEFINE node;
	node.szContext = dbgMsg;
	node.iLevel = iLevel;
	//listDebugMsg.push_back(dbgMsg);

	if (listDebugMsg.size() < (listDebugMsg.max_size() -1))
		listDebugMsg.push_back(node);
	pthread_mutex_unlock(&lock);
	sem_post(&requestOutput);
	//printf("D: [%s] %s start %d \n", "Log Manager", __func__, __LINE__);
}

void LogManager::SetOutputLogLevel(unsigned int iValue){
	pthread_mutex_lock(&lock);
	CAxcLog *pLog = (CAxcLog *) GetMyLog();
	pLog->SetOutputLevel((axc_dword) iValue);

	pthread_mutex_unlock(&lock);
}

bool LogManager::CheckIsAcceptOutputMessagebyThisLevel(unsigned int iLevel){
	bool _bRst = true;
	int _LogLevel = 0;

	/** FIXME
	 *  here shell change to Observer mode
	 *  wait config manager trigger a notify into this class
	 *  and use SetOutputLogLevel.
	 *
	 *  than we just use pLog->GetOutputLevel() to check the log engine output level.
	 */
    Cheatfinderconfigmanager *pConfigContextManager = (Cheatfinderconfigmanager *) CHeatFinderUtility::GetConfigObj();
	T_CONFIG_FILE_OPTIONS *pConfigContext = ((pConfigContextManager == NULL)?NULL:pConfigContextManager->GetConfigContext());
	if (pConfigContext != NULL){
		_LogLevel = pConfigContext->dwLogLevel;
	}else{
		_LogLevel = AXC_LOG_LEVEL_DEBUG;
	}
	//printf("D: [%s] %s start %d \n", "Log Manager", __func__, __LINE__);
    if (_LogLevel == AXC_LOG_LEVEL_DISABLE){
    	_bRst = false;
    	return _bRst;
    }
    //printf("D: [%s] %s start %d \n", "Log Manager", __func__, __LINE__);
    // check context output level
    // _LogLevel: {0~6: disable, verbose, normal, debug, warn, error, fatal} mean the limitation of log recording.
    // _OutputLevel: {0~6 disable, verbose, normal, debug, warn, error, fatal} mean the type of current message.
    if ((unsigned int)_LogLevel > iLevel){
    	_bRst = false;
    	return _bRst;
    }

	return _bRst;
}

void LogManager::OutputDebugMessage(string dbgMsg, unsigned int iLevel) {

	axc_byte eumValue = LOG_DISABLE;
	if (tFATALTrace(iLevel)){
		eumValue =LOG_FATAL;
	}else if (tERRORTrace(iLevel)){
		eumValue =LOG_ERROR;
	}else if (tWARNTrace(iLevel)){
		eumValue =LOG_WARN;
	}else if (tDEBUGTrace(iLevel)){
		eumValue =LOG_DEBUG;
	}else if (tNORMALTrace(iLevel)){
		eumValue =LOG_NORMAL;
	}else if (tVERBOSETrace(iLevel)){
		eumValue =LOG_VERBOSE;
	}else{
		eumValue =LOG_DISABLE;
	}
	//only for debug
	//printf( dbgMsg.c_str() );
	//printf("[%u %u] %s", iLevel, eumValue, dbgMsg.c_str() );

	//const axc_byte u8Level = eumValue;
	std::string _debugMsg;
	_debugMsg = dbgMsg;
	if (false == OutputData2Log(eumValue, "%s", _debugMsg.c_str())){
		PrintNowDate(2);
		printf("E: LogManager: fail to save log\n");

		// remove the log if outoff size limitation
		RemoveTheOldLogToKeepStorageSafety(50);
	}
}

//ref.:https://stackoverflow.com/questions/1149620/how-to-write-to-the-output-window-in-visual-studio?answertab=votes#tab-top
//ref.:http://c.biancheng.net/cpp/html/299.html
bool LogManager::OutputData2Log(const axc_byte byLevel, const char* szFormat, ...){
    va_list _ap;
    int _retval;
    axc_bool _iRst = axc_true;
    unsigned int _OutputLevel = (unsigned int) byLevel;

    va_start(_ap, szFormat);
    char _szBuff[1024];
	_retval = vsnprintf(_szBuff, sizeof(_szBuff), szFormat, _ap);

    if (_retval <0){
    	printf("Non-storage] E: (%d):%s", errno, strerror(errno));
		va_end(_ap);
		return false;
    }

    AxcSetLastError(0);
    CAxcLog *pLog = (CAxcLog *) GetMyLog();
    switch(_OutputLevel){
	case LOG_VERBOSE:
		_iRst = pLog->Output(AXC_LOG_LEVEL_VERBOSE, "%s", _szBuff);
		break;
	case LOG_NORMAL:
		_iRst =pLog->Output(AXC_LOG_LEVEL_NORMAL, "%s", _szBuff);
		break;
	case LOG_DEBUG:
		_iRst =pLog->Output(AXC_LOG_LEVEL_DEBUG, "%s", _szBuff);
		break;
	case LOG_WARN:
		_iRst =pLog->Output(AXC_LOG_LEVEL_WARN, "%s", _szBuff);
		break;
	case LOG_ERROR:
		_iRst =pLog->Output(AXC_LOG_LEVEL_ERROR, "%s", _szBuff);
		break;
	case LOG_FATAL:
		_iRst =pLog->Output(AXC_LOG_LEVEL_FATAL, "%s", _szBuff);
		break;
	case LOG_AUTO:
		_iRst =pLog->Output(AXC_LOG_LEVEL_DEBUG, "%s", _szBuff);
		break;
	default:
		//pass LOG_DISABLE
		break;
	}

    va_end(_ap);
    if (_iRst == axc_false){
    	printf("Non-storage] E: ade tools lib.(axclib): %s \n", AxcGetLastErrorText());
    }

    return (_iRst == axc_true)?true:false;
}

bool LogManager::RemoveTheOldLogToKeepStorageSafety(unsigned int iStorageSize){
	bool _KEEPCHK = true;
	int	_KEEPDAY = 5;
	unsigned int _maxSizeofStorage = iStorageSize*1024*1024;
	fprintf(stderr, "Non-storage] E: %s: check and keep logs total size are below with '%u' byte \n", __func__, _maxSizeofStorage);

	while (_KEEPCHK)
	{
	  DIR *d;
	  struct dirent *de;
	  struct stat buf;
	  int exists;
	  int total_size;

	  d = opendir("/home/pi/ade_camera_udp_out/log");
	  if (d == NULL) {
		  fprintf(stderr, "E: prsize: fail to use\n");
	  }

	  total_size = 0;

	  for (de = readdir(d); de != NULL; de = readdir(d)) {
		//exists = stat(de->d_name, &buf);
		try
		{
			exists = stat(de->d_name, &buf);
		}catch (...)
		{
			fprintf(stderr, "stat error: (%d): %s\n", errno, strerror(errno));
		}

		if (exists < 0) {
		  fprintf(stderr, "Couldn't stat %s\n", de->d_name);
		  //closedir(d);
		  break;
		} else {
		  total_size += buf.st_size;  /* Size of file, in bytes.  */
		}
	  }
	  closedir(d);
	  if (total_size < 0){ total_size = 0; }
	  printf("Non-storage] D: log len:%d\n", total_size);

	  if ((unsigned int)total_size >= _maxSizeofStorage) //100MB = 100*1024*1024
	  {
		  if (_KEEPDAY == 5){
			  system("sudo find /home/pi/ade_camera_udp_out/log -maxdepth 1 -mtime +5 -type f -name 'adehf_mobile-*' -delete");
			  _KEEPDAY -= 1;
		  }else if (_KEEPDAY == 4){
			  system("sudo find /home/pi/ade_camera_udp_out/log -maxdepth 1 -mtime +4 -type f -name 'adehf_mobile-*' -delete");
			  _KEEPDAY -= 1;
		  }else if (_KEEPDAY == 3){
			  system("sudo find /home/pi/ade_camera_udp_out/log -maxdepth 1 -mtime +3 -type f -name 'adehf_mobile-*' -delete");
			  _KEEPDAY -= 1;
		  }else if (_KEEPDAY == 2){
			  system("sudo find /home/pi/ade_camera_udp_out/log -maxdepth 1 -mtime +2 -type f -name 'adehf_mobile-*' -delete");
			  _KEEPDAY -= 1;
		  }else if (_KEEPDAY == 1){
			  system("sudo find /home/pi/ade_camera_udp_out/log -maxdepth 1 -mtime +1 -type f -name 'adehf_mobile-*' -delete");
			  _KEEPDAY -= 1;
		  }else {
			  //pass, cannot kill the strange file which been used.
			  _KEEPCHK = false;
		  }
	  }else{
		  _KEEPCHK = false;
	  }
	}

	return (!_KEEPCHK);
}

//////////////////////////////////////////////////////
int LogManager::sem_timedwait_millsecs(sem_t *sem, long msecs)
{
	/***
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);  //---> _TIME_H not define
    long secs = msecs/1000;
    msecs = msecs%1000;

    long add = 0;
    msecs = msecs*1000*1000 + ts.tv_nsec;
    add = msecs / (1000*1000*1000);
    ts.tv_sec += (add + secs);
    ts.tv_nsec = msecs%(1000*1000*1000);

    return sem_timedwait(sem, &ts);
    ***/
	return -1; //not support
}

void LogManager::PrintNowDate(short iStdout){
    time_t unix_timestamp=1429641418;
    unix_timestamp = time(NULL);

    struct tm *tmdate=localtime(&unix_timestamp);

    char buffer[10240] = {0};
	std::string debugMsg;

    char weekday[][8]={"Sun.","Mon.","Tue.","Wed.","Thu.","Fri.","Sat."};
    if (iStdout ==1){
    	sprintf(buffer, "Now: %04d-%02d-%02d %02d:%02d:%02d %s\n",tmdate->tm_year+1900,tmdate->tm_mon+1,tmdate->tm_mday,tmdate->tm_hour,tmdate->tm_min,tmdate->tm_sec,weekday[tmdate->tm_wday]);
    	//fflush(stdout);
    }else{
    	fprintf(stderr, "Now: %04d-%02d-%02d %02d:%02d:%02d %s\n",tmdate->tm_year+1900,tmdate->tm_mon+1,tmdate->tm_mday,tmdate->tm_hour,tmdate->tm_min,tmdate->tm_sec,weekday[tmdate->tm_wday]);
    }
	debugMsg = std::string(buffer);
	if ((iStdout ==1) && (false == OutputData2Log(LOG_AUTO, "%s", debugMsg.c_str()))) {
		//don't care success or not.
		printf("%s", debugMsg.c_str());
	}
}

