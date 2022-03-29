/*
 * adeWatchdog.h
 *
 *  Created on: Apr 30, 2018
 *      Author: markhsieh
 */

#ifndef ADEWATCHDOG_H_
#define ADEWATCHDOG_H_

#include "axclib.h"
#include "System/safepointer.h"
#include <mutex>
#include <vector>
#include <semaphore.h>

using namespace std;

typedef void (*OnProcessDeadNotifyEvent)(void *pContext, char *szStr, int isize, double losttime);
typedef struct tagProcessDeadNotifyEventHandler
{
	OnProcessDeadNotifyEvent fnEvent;
    void *pContext;
}xProcessDeadNotifyEventHandler;

typedef void (*OnWatchDogStopNotifyEvent)(void *pContext);
typedef struct tagWatchDogStopNotifyEventHandler
{
	OnWatchDogStopNotifyEvent fnEvent;
    void *pContext;
}xWatchDogStopNotifyEventHandler;

typedef struct _HeartBeatInfo
{
	char* name;
	unsigned int iSize;
	double lastHeartBeat;

	_HeartBeatInfo()
    {
		name = NULL;
		iSize = 0;
        lastHeartBeat = 0.0;
    }
    ~_HeartBeatInfo()
    {
    	Release();
    }
    //hamlet
    //markhsieh
    void Release()
    {
        if (name!=NULL){
    		SAFEDELETE_ARRAY(name);
    	}
        iSize = 0;
        lastHeartBeat = 0.0;
    }
    void CopyData(char *pBuffer, unsigned int size)
    {
        if (name!=NULL){
    		SAFEDELETE_ARRAY(name);
    	}
        name = new char[size];
        memcpy(name, pBuffer, size);
        iSize = size;
        lastHeartBeat = CAxcTime::GetCurrentUTCTimeMs();
    }
}T_HeartBeatInfo;

class CADEWatchDog{
public:
	CADEWatchDog();
	~CADEWatchDog();

    bool Run();
    void Stop();

    void Reset();
    void setHealthCheckInterval();
    void setHealthCheckInterval(unsigned int iValue);
    void setLongestHeartBeatStopTime(unsigned int iValue);

    void addProcessDeadNotifyEventHandler(OnProcessDeadNotifyEvent fnEvent, void *pContext);
    void addWatchDogStopNotiyEventHandler(OnWatchDogStopNotifyEvent fnEvent, void *pContext);

    /**
     * param:
     * 		szData = 'who's Heart beat?'
     * 		iSize = the size of 'who'
     */
    bool addHeartBeatRecord(char* szData, unsigned int iSize);
    bool IsAcceptNewHeartBeatRecord();

protected:


    static axc_dword thread_Normalization_information_process(CAxcThread* pThread, void* pContext)
    {
    	CADEWatchDog *pSender = reinterpret_cast<CADEWatchDog*> (pContext);
        return pSender->thread_Normalization_information_process();
    }
    axc_dword thread_Normalization_information_process();

    static axc_dword thread_chkalive_process(CAxcThread* pThread, void* pContext)
    {
    	CADEWatchDog *pSender = reinterpret_cast<CADEWatchDog*> (pContext);
        return pSender->thread_chkalive_process();
    }
    axc_dword thread_chkalive_process();

    void fireProcessDeadNotifyEvent(char *szStr, int isize, double losttime);
    void fireProcessDeadNotifyEvent();

    void printAllInformation();
private:
    std::vector<xProcessDeadNotifyEventHandler> m_listProcessDeadNotifyListener;
    std::vector<xWatchDogStopNotifyEventHandler> m_listWatchDogStopNotifyListener;
    std::vector<T_HeartBeatInfo*> m_listAliveRecord;

    //CAxcThread m_hThread;
    bool m_bStopThread;
    std::mutex m_locker;
    sem_t requestOutput;

    CAxcThread m_hThread_ChkAlive;
    std::mutex m_chklocker;

    unsigned int m_iNumberofHearts;
    unsigned int m_iHealthCheckInterval;

    unsigned int m_iHeartBeatMaxStopTime;
    double	m_lfStartTime;
};


#endif /* ADEWATCHDOG_H_ */
