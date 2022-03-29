/*
 * adeWatchdog.cpp
 *
 *  Created on: Apr 30, 2018
 *      Author: markhsieh
 */

#include "adeWatchdog.h"
#include "heatfinderlogmanager.h"
#include "globaldef.h"

CADEWatchDog::CADEWatchDog():
	m_hThread_ChkAlive("watchdog/check/notify")
{
    m_bStopThread = false;

    m_iNumberofHearts = 0;
    m_iHealthCheckInterval = 20; //sec.
    m_iHeartBeatMaxStopTime = 60; //3*m_iHealthCheckInterval
    //sem_init( &requestOutput, 0, 0 );

    m_lfStartTime = 0.0;
}
CADEWatchDog::~CADEWatchDog()
{
	Stop();
}

bool CADEWatchDog::Run()
{
	bool _bRst = true;

    if (!m_hThread_ChkAlive.Create(thread_chkalive_process, this))
    	_bRst = false;

    if (_bRst)
    	m_lfStartTime = CAxcTime::GetCurrentUTCTimeMs();

	return _bRst;

}
void CADEWatchDog::Stop()
{
	m_bStopThread = true;

    //sem_destroy(&requestOutput);

    if (m_hThread_ChkAlive.IsValid()){
    	m_hThread_ChkAlive.Destroy(2000);
    }

    Reset();
}

void CADEWatchDog::Reset()
{
    if (m_listAliveRecord.empty() == false)
    {   //avoid memory leak++
        for(unsigned int i = 0; i < m_listAliveRecord.size(); ++i)
        {
        	T_HeartBeatInfo *pElement = m_listAliveRecord[i];
            delete pElement;
            pElement = NULL;
        }
        m_listAliveRecord.clear();
    }
}

void CADEWatchDog::printAllInformation()
{
	GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Watch Dog] the current heart beat records of list:\n");
    if (m_listAliveRecord.empty() == false)
    {   //avoid memory leak++
        for(unsigned int i = 0; i < m_listAliveRecord.size(); ++i)
        {
        	T_HeartBeatInfo *pElement = m_listAliveRecord[i];
        	GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Watch Dog] [%u] '%s' last timestamp: %.03lf\n", (i+1), pElement->name, pElement->lastHeartBeat);
        }
    }
}

void CADEWatchDog::setHealthCheckInterval()
{
	m_iHealthCheckInterval = 25;
	m_iHeartBeatMaxStopTime = (m_iHealthCheckInterval*3);

	GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Watch Dog] %s renew interval: %u, Longest heartbeat stop time: %u\n", __func__, m_iHealthCheckInterval, m_iHeartBeatMaxStopTime);
}

void CADEWatchDog::setHealthCheckInterval(unsigned int iValue)
{
	if (iValue <= 0){
		return;
	}else if (m_iHealthCheckInterval != 0){
		/**
		 * "one job done, than another job can be accept." ....
		 *
		 * When an error occurs, the video capture program is automatically restarted,
		 * and the monitoring time is not allowed to be reset.
		 * Since "image non-user operation restart" indicates that an error has occurred,
		 * it is necessary to confirm that the encoder has been set correctly and obtain a new image.
		 */

		return;
	}
	m_iHealthCheckInterval = iValue;
	//m_iHeartBeatMaxStopTime = m_iHealthCheckInterval*3;
	m_lfStartTime = CAxcTime::GetCurrentUTCTimeMs();

	GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Watch Dog] %s renew interval: %u\n", __func__, m_iHealthCheckInterval);
}

void CADEWatchDog::setLongestHeartBeatStopTime(unsigned int iValue)
{
	if (iValue <= 0){
		return;
	}else if (m_iHeartBeatMaxStopTime != 0){
		/**
		 * "one job done, than another job can be accept." ....
		 *
		 * When an error occurs, the video capture program is automatically restarted,
		 * and the monitoring time is not allowed to be reset.
		 * Since "image non-user operation restart" indicates that an error has occurred,
		 * it is necessary to confirm that the encoder has been set correctly and obtain a new image.
		 */

		return;
	}

	if (iValue < (m_iHealthCheckInterval*2)){
		m_iHeartBeatMaxStopTime = (m_iHealthCheckInterval*2);
	}else{
		m_iHeartBeatMaxStopTime = iValue;
	}

	GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Watch Dog] %s renew Longest heartbeat stop time: %u\n", __func__,  m_iHeartBeatMaxStopTime);
}

void CADEWatchDog::addProcessDeadNotifyEventHandler(OnProcessDeadNotifyEvent fnEvent, void *pContext)
{
	xProcessDeadNotifyEventHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;
    m_listProcessDeadNotifyListener.push_back(handler);
}

void CADEWatchDog::addWatchDogStopNotiyEventHandler(OnWatchDogStopNotifyEvent fnEvent, void *pContext){
	xWatchDogStopNotifyEventHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;
    m_listWatchDogStopNotifyListener.push_back(handler);
}

/**
 * param:
 * 		szData = 'who's Heart beat?'
 * 		iSize = the size of 'who'
 * warn:
 *      will auto free() char* szData
 */
bool CADEWatchDog::addHeartBeatRecord(char* szData, unsigned int iSize){
	bool _bRst = true;

	if (m_bStopThread){
		// valgrind: [Memory leak] 4,016 (2,016 direct, 2,000 indirect) bytes in 126 blocks are definitely lost in loss record 290 of 320
		//free(szData);

		return _bRst;
	}

	if (m_iHealthCheckInterval <= 0){
		// valgrind: [Memory leak] 4,016 (2,016 direct, 2,000 indirect) bytes in 126 blocks are definitely lost in loss record 290 of 320
		//free(szData);

		GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Watch Dog] %s deny! Plz. setHealthCheckInterval first\n", __func__);
		return _bRst;
	}

	T_HeartBeatInfo *info = new T_HeartBeatInfo();
	info->CopyData(szData, iSize);

	// check the new one or exist one, append the new and renew the exist.
	m_chklocker.lock();
	bool _bFoundNewOne = true;
	for(unsigned int j = 0; j < m_listAliveRecord.size(); ++j){

		T_HeartBeatInfo *pExist = m_listAliveRecord[j];
		if (0 == strncmp(info->name, pExist->name, info->iSize)){
			_bFoundNewOne = false;
			pExist->lastHeartBeat = info->lastHeartBeat;
			break;
		}
	}

	if ((_bFoundNewOne == true) &&
		(m_listAliveRecord.size() < m_listAliveRecord.max_size()-1)){
		m_listAliveRecord.push_back(info);

		m_iNumberofHearts +=1;
		GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Watch Dog] %s get new one: %s \n", __func__, info->name);
	}
	else
	{
		delete info;
	}
	m_chklocker.unlock();

	return _bRst;
}

bool CADEWatchDog::IsAcceptNewHeartBeatRecord(){
	bool _bRst = false;

	if (m_iHealthCheckInterval > 0){
		_bRst = true;
	}

	return _bRst;
}

axc_dword CADEWatchDog::thread_chkalive_process(){

	double _lfLastHeartBeat = 0.0;
	double _lfTimeDiff = 0.0;
	unsigned int _iCurrentHeartCount = 0;

	GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Watch Dog] %s start \n", __func__);

    void *pContext = CHeatFinderUtility::GetGlobalsupport();
    GlobalDef *pGlobal = reinterpret_cast<GlobalDef*> (pContext);
    pGlobal->AddpThreadRecord("Watch Dog");

	while(!m_bStopThread){
		/**
		 * base on the setting step after Run():
		 * 1. set the interval (>0)
		 * 2. give the Heart Beat Record
		 */
		if ((m_iHealthCheckInterval == 0) ||
			(m_listAliveRecord.size() == 0)){
			m_hThread_ChkAlive.SleepMilliSeconds(1000);
			continue;
		}

		//printf("N: [Watch Dog] %s checking \n", __func__);
		_iCurrentHeartCount = 0;
		_lfLastHeartBeat = CAxcTime::GetCurrentUTCTimeMs();
    	m_chklocker.lock();
    	for(unsigned int j = 0; j < m_listAliveRecord.size(); ++j){

    		T_HeartBeatInfo *pExist = m_listAliveRecord[j];
    		_lfTimeDiff = _lfLastHeartBeat - pExist->lastHeartBeat;
    		if (_lfTimeDiff < m_iHealthCheckInterval){
    			_iCurrentHeartCount +=1;
    		}else if (_lfTimeDiff > m_iHealthCheckInterval){
    			// some process dead/idle ?
    			fireProcessDeadNotifyEvent(pExist->name, pExist->iSize, _lfTimeDiff);

    			//error happen: checking process renew start time.
    			m_lfStartTime = _lfLastHeartBeat;
    		}
    	}
    	m_chklocker.unlock();

    	if (_iCurrentHeartCount > m_iNumberofHearts ){
    		GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tVERBOSETrace_MSK, "V: [Watch Dog] Heart Alive ++: %u -> %u\n", m_iNumberofHearts, _iCurrentHeartCount);
    		m_iNumberofHearts = _iCurrentHeartCount;
    	}else if (_iCurrentHeartCount < m_iNumberofHearts ){
    		GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tVERBOSETrace_MSK, "V: [Watch Dog] Heart Alive --: %u -> %u\n", m_iNumberofHearts, _iCurrentHeartCount);
    		m_iNumberofHearts = _iCurrentHeartCount;
    	}else{
    		GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tVERBOSETrace_MSK, "V: [Watch Dog] Heart Alive : %u\n", m_iNumberofHearts);
    	}

		if (m_iHeartBeatMaxStopTime < (_lfLastHeartBeat - m_lfStartTime)){
			GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Watch Dog] monitor job done from %.03lf ~ %.03lf\n", m_lfStartTime, _lfLastHeartBeat);
			//m_bStopThread = true;

			printAllInformation();
			fireProcessDeadNotifyEvent();
			m_iHealthCheckInterval = 0;
			Reset();
		}

		//printf("N: [Watch Dog] %s job done \n", __func__);
		m_hThread_ChkAlive.SleepMilliSeconds(m_iHealthCheckInterval*1000);
	}
	return 1;
}

void CADEWatchDog::fireProcessDeadNotifyEvent(char *szStr, int isize, double losttime){
	if(m_listProcessDeadNotifyListener.size() <= 0){
		return;
	}

	for(unsigned int i = 0; i < m_listProcessDeadNotifyListener.size(); ++i)
	{
		m_listProcessDeadNotifyListener[i].fnEvent(m_listProcessDeadNotifyListener[i].pContext, szStr, isize, losttime);
	}
}

void CADEWatchDog::fireProcessDeadNotifyEvent(){
	if(m_listWatchDogStopNotifyListener.size() <= 0){
		return;
	}

	for(unsigned int i = 0; i < m_listWatchDogStopNotifyListener.size(); ++i)
	{
		m_listWatchDogStopNotifyListener[i].fnEvent(m_listWatchDogStopNotifyListener[i].pContext);
	}

}
