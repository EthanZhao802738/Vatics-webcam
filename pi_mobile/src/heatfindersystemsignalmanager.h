/*
 * heatfindersystemsignalmanager.h
 *
 *  Created on: Jan 9, 2018
 *      Author: markhsieh
 */

#ifndef HEATFINDERSYSTEMSIGNALMANAGER_H_
#define HEATFINDERSYSTEMSIGNALMANAGER_H_

#include "globaldef.h"
#include <mutex>
#include <vector>

// define list
#define SYSTEMSIGNALEVENT_FIRE_ACTIONDOCARE (0)
#define SYSTEMSIGNALEVENT_FIRE_TIMEDOCARE (0)

// event
/**
 * system signal: event --- receive
 */
typedef struct tagReceiveSystemSignalHandler
{
    void *pContext;
    struct sigaction sigalInterruptHandler;
}xReceiveSystemSignalHandler;

/**
 * notify --- fire
 * capture device:
 * 		flag:
 * 		1. vision
 * 		2. lepton thermal
 * vision:
 * 		action:
 * 		1. restart vision: stop, then start raspivid
 * 		2.3. pause/play: pause or play all data capture
 * lepton thermal:
 * 		action:
 * 		1.2. pause/play: pause or play all data capture
 */
typedef void (*OnCaptureStatusChangeNotify)(void *pContext, const unsigned int flag, const unsigned int action, const unsigned int time_interval);
typedef struct tagCaptureStatusChangeHandler
{
	OnCaptureStatusChangeNotify fnEvent;
    void *pContext;
}xCaptureStatusChangeHandler;

/**
 * APP: notify --- fire
 * 		report status: {exit, }
 */
typedef void (*OnAPPStatusChangeNotify)(void *pContext, const unsigned int flag, const unsigned int action, const unsigned int time_interval);
typedef struct tagAPPStatusChangeHandler
{
	OnAPPStatusChangeNotify fnEvent;
    void *pContext;
}xAPPStatusChangeHandler;

// class
class CHeatFinderSystemSignalManager
{
public:
	CHeatFinderSystemSignalManager();
	~CHeatFinderSystemSignalManager();

    bool Run();
    void Stop();
    bool GetSystemSignalStatus();

	//Register the events that need to be viewed
	//setting action of event receive
    void AddCaptureStatusChangeNotifyEvent(OnCaptureStatusChangeNotify fnEvent, void *pContext);
    void AddSystemSingalReceiveNotifyEvent();
    void AddAPPStatusChangeNotifyEvent(OnAPPStatusChangeNotify fnEvent, void *pContext);

    // ... make a shell
    void OnTriggerCaptureStatusChangeNotifyEvent(const unsigned int flag, const unsigned int action, const unsigned int time_interval);
    void OnTriggerAPPStatusChangeNotifyEvent(const unsigned int flag, const unsigned int action, const unsigned int time_interval);

	void SetActiveSignalManager(void *pActiveSignalManager);
	static void* GetActiveSignalManager();
protected:

    bool m_bPrintSystemSignalManagerStatus;
    static void *m_pActiveSignalManager;

    //simple event
    xReceiveSystemSignalHandler m_ADESigalInterruptHandler;

	//restore request event
	std::vector<xCaptureStatusChangeHandler> m_CaptureStatusChange_List;
	std::vector<xAPPStatusChangeHandler> m_APPStatusChange_List;

	//trigger target active object --- fire
	void fireCaptureStatusChangeNotify(const unsigned int flag, const unsigned int action, const unsigned int time_interval);
	void fireAPPStatusChangeNotify(const unsigned int flag, const unsigned int action, const unsigned int time_interval);

	//listener --- receive ... because below I cannot success... I try to be another way
	/**static void OnRecieveSystemSingalHandler(void *pContext, int s){
		CHeatFinderSystemSignalManager *pSender = reinterpret_cast<CHeatFinderSystemSignalManager*> (pContext);
		return pSender->OnRecieveSystemSingalHandler(s);
	}
	void OnRecieveSystemSingalHandler(int s);
	**/


};


#endif /* HEATFINDERSYSTEMSIGNALMANAGER_H_ */
