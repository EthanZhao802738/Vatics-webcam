/*
 * heatfindersystemsignalmanager.cpp
 *
 *  Created on: Jan 9, 2018
 *      Author: markhsieh
 */
#include "heatfindersystemsignalmanager.h"
#include "ade_camera_sdk.h"

static void OnRecieveSystemSingalHandler(int s);
void *CHeatFinderSystemSignalManager::m_pActiveSignalManager = NULL;

// Object manager---------------------------
CHeatFinderSystemSignalManager::CHeatFinderSystemSignalManager():
    m_bPrintSystemSignalManagerStatus(false)
{
	SetActiveSignalManager(this);

	//catch a ctrl-c event
	//ref.:https://stackoverflow.com/questions/28247501/why-is-my-signal-handler-not-working
	memset(&m_ADESigalInterruptHandler.sigalInterruptHandler, 0, sizeof(m_ADESigalInterruptHandler.sigalInterruptHandler));

}

CHeatFinderSystemSignalManager::~CHeatFinderSystemSignalManager()
{
    Stop();
}

bool CHeatFinderSystemSignalManager::Run()
{
	GLog(tAll, tDEBUGTrace_MSK, "D: [%s] Start ...  \n","SystemSignalManager");
    bool bRst = false;

    // init.
    CHeatFinderSystemSignalManager * self = (CHeatFinderSystemSignalManager *) GetActiveSignalManager();
    self->AddSystemSingalReceiveNotifyEvent();

    bRst = GetSystemSignalStatus();
    return bRst;
}

void CHeatFinderSystemSignalManager::Stop()
{
	GLog(tAll, tDEBUGTrace_MSK, "D: [%s] Stop \n","SystemSignalManager");
	m_bPrintSystemSignalManagerStatus = false;
	m_ADESigalInterruptHandler.sigalInterruptHandler.sa_flags = 0;
	sigemptyset(&m_ADESigalInterruptHandler.sigalInterruptHandler.sa_mask);
	memset(&m_ADESigalInterruptHandler.sigalInterruptHandler, 0, sizeof(m_ADESigalInterruptHandler.sigalInterruptHandler));

}

bool CHeatFinderSystemSignalManager::GetSystemSignalStatus()
{
	return m_bPrintSystemSignalManagerStatus;
}

void CHeatFinderSystemSignalManager::SetActiveSignalManager(void *pActiveSignalManager)
{
	m_pActiveSignalManager = pActiveSignalManager;
}

void* CHeatFinderSystemSignalManager::GetActiveSignalManager()
{
    return m_pActiveSignalManager;
}

// setting/init.----------------------------
// set event function pointer (callback action)
void CHeatFinderSystemSignalManager::AddCaptureStatusChangeNotifyEvent(OnCaptureStatusChangeNotify fnEvent, void *pContext)
{
	xCaptureStatusChangeHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;

    m_CaptureStatusChange_List.push_back(handler);
}

void CHeatFinderSystemSignalManager::AddAPPStatusChangeNotifyEvent(OnAPPStatusChangeNotify fnEvent, void *pContext){
	xAPPStatusChangeHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;

    m_APPStatusChange_List.push_back(handler);
}

void CHeatFinderSystemSignalManager::AddSystemSingalReceiveNotifyEvent(){

	xReceiveSystemSignalHandler handler;

	handler.pContext = (CHeatFinderSystemSignalManager *) GetActiveSignalManager();
	handler.sigalInterruptHandler.sa_handler = OnRecieveSystemSingalHandler;
	sigemptyset(&handler.sigalInterruptHandler.sa_mask);
	handler.sigalInterruptHandler.sa_flags = 0;

	sigaddset(&handler.sigalInterruptHandler.sa_mask, SIGHUP);
	sigaddset(&handler.sigalInterruptHandler.sa_mask, SIGUSR1);
	sigaddset(&handler.sigalInterruptHandler.sa_mask, SIGUSR2);
	sigaddset(&handler.sigalInterruptHandler.sa_mask, SIGTERM);

	//Ctrl+C
	sigaddset(&handler.sigalInterruptHandler.sa_mask, SIGINT);
	sigaddset(&handler.sigalInterruptHandler.sa_mask, SIGALRM);
	sigaddset(&handler.sigalInterruptHandler.sa_mask, SIGCONT);

	m_bPrintSystemSignalManagerStatus = true;
	if (sigaction(SIGHUP, &handler.sigalInterruptHandler, NULL) == -1) {
		GLog(tAll, tERRORTrace_MSK, "E: [%s]  cannot handle SIGHUP\n", "SystemSignalManager");
		m_bPrintSystemSignalManagerStatus = false;
	}

	if (sigaction(SIGUSR1, &handler.sigalInterruptHandler, NULL) == -1) {
		GLog(tAll, tERRORTrace_MSK, "E: [%s]  cannot handle SIGUSR1\n", "SystemSignalManager");
		m_bPrintSystemSignalManagerStatus = false;
	}

	if (sigaction(SIGUSR2, &handler.sigalInterruptHandler, NULL) == -1) {
		GLog(tAll, tERRORTrace_MSK, "E: [%s]  cannot handle SIGUSR2\n", "SystemSignalManager");
		m_bPrintSystemSignalManagerStatus = false;
	}

	if (sigaction(SIGTERM, &handler.sigalInterruptHandler, NULL) == -1) {
		GLog(tAll, tERRORTrace_MSK, "E: [%s]  cannot handle SIGTERM\n", "SystemSignalManager");
		m_bPrintSystemSignalManagerStatus = false;
	}

	if (sigaction(SIGINT, &handler.sigalInterruptHandler, NULL) == -1) {
		GLog(tAll, tERRORTrace_MSK, "E: [%s]  cannot handle SIGINT\n", "SystemSignalManager");
		m_bPrintSystemSignalManagerStatus = false;
	}

	if (sigaction(SIGALRM, &handler.sigalInterruptHandler, NULL) == -1) {
		GLog(tAll, tERRORTrace_MSK, "E: [%s]  cannot handle SIGALRM\n", "SystemSignalManager");
		m_bPrintSystemSignalManagerStatus = false;
	}

	if (sigaction(SIGCONT, &handler.sigalInterruptHandler, NULL) == -1) {
		GLog(tAll, tERRORTrace_MSK, "E: [%s]  cannot handle SIGCONT\n", "SystemSignalManager");
		m_bPrintSystemSignalManagerStatus = false;
	}

	m_ADESigalInterruptHandler = handler;
}

// action-----------------------------------
// use event function pointer
/**
 * fireCaptureStatusChangeNotify
 * parameter:
 * 		flag: VISION=0, THERMAL=1
 * 		action:
 * 			vision:
 * 				action:
 * 				1. restart vision: stop, then start raspivid
 * 				2.3. pause/play: pause or play all data capture
 * 			lepton thermal:
 * 				action:
 * 				1.2. pause/play: pause or play all data capture
 * 		time_interval: event time interval ... for debug/workaround
 */
void CHeatFinderSystemSignalManager::fireCaptureStatusChangeNotify(const unsigned int flag, const unsigned int action, const unsigned int time_interval){

    for(unsigned int i = 0; i < m_CaptureStatusChange_List.size(); ++i)
    {
    	xCaptureStatusChangeHandler handler = m_CaptureStatusChange_List[i];
        handler.fnEvent(handler.pContext, flag, action, time_interval);
    }
}

/**
 * OnTriggerCaptureStatusChangeNotifyEvent
 * parameter:
 * 		flag: VISION=0, THERMAL=1
 * 		action: (VISION) {1: restart raspivid, 2: pause send view, 3: start send view}
 * 				(THERMAL) {1: pause send view, 2: start send view}
 * 		time_interval: event time interval ... for debug/workaround
 */
void CHeatFinderSystemSignalManager::OnTriggerCaptureStatusChangeNotifyEvent(const unsigned int flag, const unsigned int action, const unsigned int time_interval){
	GLog(tAll, tDEBUGTrace_MSK, "D: [%s] TriggerAPPStatusChangeNotifyEvent flag:%s\n","SystemSignalManager", (flag==0)?"VISION":((flag==1)?"THERMAL":"Unknown"));

	if (flag ==0){
		if((action!=1)
			&&(action!=2)
			&&(action!=3)){
			GLog(tAll, tWARNTrace_MSK, "W: [%s] action:%d not support\n","SystemSignalManager", action);
			return;
		}else{
			GLog(tAll, tDEBUGTrace_MSK, "D: [%s] action:%s\n","SystemSignalManager", (action==1)?"restart raspivid":((action==2)?"pause send view":"start send view"));
		}
	}else if (flag ==1){
		if((action!=1)
			&&(action!=2)){
			GLog(tAll, tWARNTrace_MSK, "W: [%s] action:%d not support\n","SystemSignalManager", action);
			return;
		}else{
			GLog(tAll, tDEBUGTrace_MSK, "D: [%s] action:%s\n","SystemSignalManager", (action==1)?"pause send view":((action==2)?"start send view":"Unknown"));
		}
	}else{
		return;
	}
	fireCaptureStatusChangeNotify(flag, action, time_interval);
}

/**
 * fireAPPStatusChangeNotify
 * parameter:
 * 		flag: EXIT=0
 * 		action: null
 * 		time_interval: event time interval ... for debug/workaround
 */
void CHeatFinderSystemSignalManager::fireAPPStatusChangeNotify(const unsigned int flag, const unsigned int action, const unsigned int time_interval){
    for(unsigned int i = 0; i < m_APPStatusChange_List.size(); ++i)
    {
    	xAPPStatusChangeHandler handler = m_APPStatusChange_List[i];
        handler.fnEvent(handler.pContext, flag, action, time_interval);
    }
}

/**
 * OnTriggerAPPStatusChangeNotifyEvent
 * parameter:
 * 		flag: EXIT=0
 * 		action: null
 * 		time_interval: event time interval ... for debug/workaround
 */
void CHeatFinderSystemSignalManager::OnTriggerAPPStatusChangeNotifyEvent(const unsigned int flag, const unsigned int action, const unsigned int time_interval){
	GLog(tAll, tDEBUGTrace_MSK, "D: [%s] TriggerAPPStatusChangeNotifyEvent flag:%d\n","SystemSignalManager",flag);
	fireAPPStatusChangeNotify(flag, action, time_interval);
}

// callback---------------------------------
static void OnRecieveSystemSingalHandler(int s){
	GLog(tAll, tDEBUGTrace_MSK, "D: [%s] Caught signal %d\n","SystemSignalManager", s);
	const char *signal_name;
	int action_case =0;

	switch (s) {
	  case SIGHUP:
		  signal_name = "SIGHUP:Reserved";
		  break;
	  case SIGUSR1:
		  signal_name = "SIGUSR1:Restart Visio";
		  action_case =2;
		  break;
	  case SIGUSR2:
		  signal_name = "SIGUSR2:Reserved";
		  break;
	  case SIGINT:
		  signal_name = "SIGINT:Ctrl+C Interrupt & Stop APP";
		  action_case =1;
		  break;
	  case SIGTERM:
		  signal_name = "SIGTERM:Exit APP";
		  action_case =1;
		  break;
	  case SIGALRM:
		  signal_name = "SIGALRM:Stop send visio and thermal view by network";
		  action_case =3;
		  break;
	  case SIGCONT:
		  signal_name = "SIGCONT:Start|Re-Start send visio and thermal view by udp";
		  action_case =4;
		  break;
	  default:
		  signal_name = "Non-support";
		  break;
	}
	GLog(tAll, tDEBUGTrace_MSK, "D: [%s] signal mean %s\n","SystemSignalManager", signal_name);

	if (action_case > 0){
		CHeatFinderSystemSignalManager *systemSignalManager = (CHeatFinderSystemSignalManager *) CHeatFinderSystemSignalManager::GetActiveSignalManager();
		if (action_case ==1){
			systemSignalManager->OnTriggerAPPStatusChangeNotifyEvent(0, SYSTEMSIGNALEVENT_FIRE_ACTIONDOCARE, SYSTEMSIGNALEVENT_FIRE_TIMEDOCARE);
		}else if (action_case ==2){
			systemSignalManager->OnTriggerCaptureStatusChangeNotifyEvent(0, 1, SYSTEMSIGNALEVENT_FIRE_TIMEDOCARE);
		}else if (action_case ==3){
			systemSignalManager->OnTriggerCaptureStatusChangeNotifyEvent(0, 2, SYSTEMSIGNALEVENT_FIRE_TIMEDOCARE);
			systemSignalManager->OnTriggerCaptureStatusChangeNotifyEvent(1, 1, SYSTEMSIGNALEVENT_FIRE_TIMEDOCARE);
		}else if (action_case ==4){
			systemSignalManager->OnTriggerCaptureStatusChangeNotifyEvent(0, 3, SYSTEMSIGNALEVENT_FIRE_TIMEDOCARE);
			systemSignalManager->OnTriggerCaptureStatusChangeNotifyEvent(1, 2, SYSTEMSIGNALEVENT_FIRE_TIMEDOCARE);

		}
	}
}
