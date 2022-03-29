#include "heatfindertimer.h"

#include <functional>

CHeatFinderTimer::CHeatFinderTimer():
    m_hThread("sys/anything"),
    m_iInterval(100),
    m_bStopThread(true),
    m_bLedNetTwinkle(false),
    m_bLedAlarmTwinkle(false),
    m_bLedStatus(false),
    m_bLedStatusTurnOn(false),
    m_tmTcpProbe(0),
    m_bIsPrintDebugMsg(false),
    m_dBigestTimeInterval(0.0)
{

}

CHeatFinderTimer::~CHeatFinderTimer()
{
    Stop();
}

void CHeatFinderTimer::SetInterval(const int interval)
{
    m_iInterval = interval;
}

int CHeatFinderTimer::GetInterval()
{
    return m_iInterval;
}

void CHeatFinderTimer::Stop()
{
    m_bStopThread = true;
    if (m_hThread.IsValid())
        m_hThread.Destroy(2000);
}

/**
 * @brief CHeatFinderTimer::AddLedNetTwinkleNotifyEvent
 * Oh my god. notify other classes Led net Twinkle change
 * @param fnEvent
 * @param pContext
 */
void CHeatFinderTimer::AddLedNetTwinkleNotifyEvent(OnLedNetTwinkleNotify fnEvent, void *pContext)
{
    xLedNetTwinkleNotifyHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;
    m_LedNetTwinkleNotifyList.push_back(handler);
}

void CHeatFinderTimer::AddLedAlarmTwinkleNotifyEvent(OnLedAlarmTwinkleNotify fnEvent, void *pContext)
{
    xLedAlarmTwinkleNotifyHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;
    m_LedAlarmTwinkleNotifyList.push_back(handler);
}

void CHeatFinderTimer::AddLedStatusTwinkleNotifyEvent(OnLedStatusTwinkleNotify fnEvent, void *pContext)
{
    xLedStatusTwinkleTimerNotifyHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;
    m_LedStatusTwinkleNotifyList.push_back(handler);
}

void CHeatFinderTimer::AddLedStatusTurnOnNotifyEvent(OnLedStatusTurnOnNotify fnEvent, void *pContext)
{
    xLedStatusTurnOnTimerNotifyHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;
    m_LedStatusTurnOnNotifyList.push_back(handler);
}

void CHeatFinderTimer::AddCheckFunctionOnTimeNotifyEvent(OnFunctionCheckOnTimeNotify fnEvent, double dTimeInterval, const char* pcStr, void *pContext){
	xOnFunctionCheckOnTimeNotifyHandler xHandler;
	memset(xHandler.cstr32Name, 0, sizeof(xHandler.cstr32Name)-1);
	strncpy(xHandler.cstr32Name, pcStr, sizeof(xHandler.cstr32Name)-1);

	if (dTimeInterval <= 0.0){
		GLog(tTIMERTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] Add '%s' to Timer Checking list fail. Time Interval wrong (< 0.0): %.03lf\n", "Timer", xHandler.cstr32Name, dTimeInterval);
		return;
	}else{
		xHandler.dTimeInterval = dTimeInterval;
	}

	/**
	 * eg. int m_iInterval = 100 ms
	 * 15.278 s -> 15278 ms -> 15000 + [278] ms => 200 + 78(less than m_iInterval)
	 * remove the number m_iInterval can not check.
	 */
	int iMod = ((int)(dTimeInterval*1000))%1000;
	int l_dChkIntervalGap = iMod% m_iInterval;
	if (l_dChkIntervalGap > 0){
		xHandler.dTimeInterval = (double)((int)(dTimeInterval*1000)-l_dChkIntervalGap)/1000;
	}

	xHandler.dLastTriggerTime=0.0;
	xHandler.fnEvent = fnEvent;
	xHandler.pContext = pContext;

	m_locker.lock();

	m_FuncChkOnTimeNotifyList.push_back(xHandler);
	m_locker.unlock();

	if(m_bIsPrintDebugMsg){
		GLog(tTIMERTrace(LogManager::sm_glogDebugZoneSeed), tVERBOSETrace_MSK, "V: [%s] '%s' be added. time interval:%lf\n", "Timer", xHandler.cstr32Name, xHandler.dTimeInterval);
	}
	pcStr=NULL;

}

bool CHeatFinderTimer::SetPrintDebugMsg(bool bValue){
	m_bIsPrintDebugMsg = bValue;
	return m_bIsPrintDebugMsg;
}

/**
 * @brief CHeatFinderTimer::OnLedNetTwinkleNotify
 * get Led network Twinkleing status from...
 * @param value
 */
void CHeatFinderTimer::OnLedNetTwinkleNotify_func(const bool value)
{
    m_bLedNetTwinkle = value;
}

void CHeatFinderTimer::OnLedAlarmTwinkleNotify_func(const bool value)
{
    m_bLedAlarmTwinkle = value;
}

void CHeatFinderTimer::OnLedStatusTwinkleNotify_func(const bool value)
{
    m_bLedStatus = value;
}

void CHeatFinderTimer::OnLedStatusTurnOnNotify_func(const bool value)
{
    m_bLedStatusTurnOn = value;
}

/**
 * @brief CHeatFinderTimer::OnTCPProbeNotify
 * get TCP Probe timestamp from...
 * @param value
 */
void CHeatFinderTimer::OnTCPProbeNotify_func(axc_dword value)
{
    m_tmTcpProbe = value;
}

/**
 * @brief CHeatFinderTimer::fireLedNetTwinkleNotify
 * fire Led Net Twinkle notify event
 * @param value
 */
void CHeatFinderTimer::fireLedNetTwinkleNotify(const bool value)
{
    if (m_LedNetTwinkleNotifyList.empty())
        return;

    for(unsigned int i = 0; i < m_LedNetTwinkleNotifyList.size(); ++i)
    {
        xLedNetTwinkleNotifyHandler handler = m_LedNetTwinkleNotifyList[i];
        handler.fnEvent(handler.pContext, value);
    }
}

void CHeatFinderTimer::fireLedAlarmTwinkleNotify(const bool value)
{
    if (m_LedAlarmTwinkleNotifyList.empty())
        return;

    for(unsigned int i = 0; i < m_LedAlarmTwinkleNotifyList.size(); ++i)
    {
        xLedAlarmTwinkleNotifyHandler handler = m_LedAlarmTwinkleNotifyList[i];
        handler.fnEvent(handler.pContext, value);
    }
}

void CHeatFinderTimer::fireLedStatusNotify(const bool value)
{
    if (m_LedStatusTwinkleNotifyList.empty())
        return;

    for(unsigned int i = 0; i < m_LedStatusTwinkleNotifyList.size(); ++i)
    {
        xLedStatusTwinkleTimerNotifyHandler handler = m_LedStatusTwinkleNotifyList[i];
        handler.fnEvent(handler.pContext, value);
    }
}

void CHeatFinderTimer::fireLedStatusTurnOnNotify(const bool value)
{
    if (m_LedStatusTurnOnNotifyList.empty())
        return;

    for(unsigned int i = 0; i < m_LedStatusTurnOnNotifyList.size(); ++i)
    {
        xLedStatusTurnOnTimerNotifyHandler handler = m_LedStatusTurnOnNotifyList[i];
        handler.fnEvent(handler.pContext, value);
    }
}

void CHeatFinderTimer::fireCheckFunctionOnTimeNotify(unsigned int uiIndex, double dCurrentTimeMS, unsigned int uiAction, signed int siValue){
	if (m_FuncChkOnTimeNotifyList.size()<=0)
		return ;

	//only trigger one function.
	xOnFunctionCheckOnTimeNotifyHandler handler = m_FuncChkOnTimeNotifyList[uiIndex];
	const double l_dCurrentTimeMS = dCurrentTimeMS;
	const unsigned int l_uiAction = uiAction;
	const signed int l_siValue = siValue;

	if(m_bIsPrintDebugMsg){
		GLog(tTIMERTrace(LogManager::sm_glogDebugZoneSeed), tVERBOSETrace_MSK, "V: [%s] '%s' be triggered. time interval:%lf\n", "Timer", handler.cstr32Name, handler.dTimeInterval);
	}

	handler.fnEvent(handler.pContext, l_dCurrentTimeMS, l_uiAction, l_siValue);
}

bool CHeatFinderTimer::Run()
{
    bool bRst = false;
    Stop();

    m_bStopThread = false;
    if (!m_hThread.Create(thread_process, this))
        return bRst;

    bRst = true;
    return bRst;
}

void CHeatFinderTimer::OnChangeFunctionIntervalTimeList(std::vector<xFunctionCheckListHandler> &dList){

	dList.clear();

	std::vector<xFunctionCheckListHandler> l_dList;
	//Do self check. Does any new function auth.?
	xFunctionCheckListHandler xHandler;
	xHandler.dTimeInterval = 1.0;
	xHandler.uiIndex = 0;
	l_dList.push_back(xHandler);

	//sorting and initial
	double l_dBigestTimeInterval =0.0;
	for(int i =((int)m_FuncChkOnTimeNotifyList.size())-1; i>=0; i--){
		//for sorting
		xFunctionCheckListHandler xHandler;
		xHandler.dTimeInterval = (m_FuncChkOnTimeNotifyList[i].dTimeInterval);
		xHandler.uiIndex = i;
		l_dList.push_back(xHandler);

		//init.
		m_FuncChkOnTimeNotifyList[i].dLastTriggerTime = CAxcTime::GetCurrentUTCTimeMs();
	}

	m_dBigestTimeInterval = l_dBigestTimeInterval;

	/**
	 * lower interval will trigger more often, so need to check more frequent than bigger interval
	 * Sorting ... list:{lower, , --->, ,bigger}
	 */
	for(int j =((int)l_dList.size())-1; j>=0; j--){
		if (dList.size()<=0){
			dList.push_back(l_dList[j]);
		}else{
			//Ascending powers for dList, but checking by Power down
			int iTemporarilyIndex =((int)dList.size())-1;
			int k;
			for(k =((int)dList.size())-1; k>=0; k--){
				if (dList[k].dTimeInterval >= l_dList[j].dTimeInterval){

					 // 4 way:
					 // {11} -> {2, 3, 7, 9}: stop at start.
					 // {4}  -> {2, 3, 7, 9}: to index: [2]
					 // {7}  -> {2, 3, 7, 9}: to index: [2]
					 // {1}  -> {2, 3, 7, 9}: to index: [0] and leave for loop

					iTemporarilyIndex =k;
				}else{
					if (iTemporarilyIndex == (((int)dList.size())-1)){
						dList.push_back(l_dList[j]);
						break;
					}
					// insert
					dList.insert(dList.begin()+iTemporarilyIndex+1, l_dList[j]);
					break;
				}
			}
			if ((iTemporarilyIndex > k)&&(k <0)){
				dList.insert(dList.begin(), l_dList[j]);
			}
		}
	}


}

axc_dword CHeatFinderTimer::thread_process()
{
    void *pContext = CHeatFinderUtility::GetGlobalsupport();
    GlobalDef *pGlobal = reinterpret_cast<GlobalDef*> (pContext);
    pGlobal->AddpThreadRecord("Sample Timer");

	unsigned int uiListCount =0;
	std::vector<xFunctionCheckListHandler> l_TimeIntervalList;

	m_locker.lock();
	if (m_FuncChkOnTimeNotifyList.size() != uiListCount){
		OnChangeFunctionIntervalTimeList(l_TimeIntervalList);
		uiListCount = m_FuncChkOnTimeNotifyList.size();
	}
	m_locker.unlock();

    double l_dwTimeCurrent = CAxcTime::GetCurrentUTCTimeMs();
    double l_dwTimeDiff = 0.0;

    while(!m_bStopThread)
    {
    	l_dwTimeCurrent = CAxcTime::GetCurrentUTCTimeMs();

    	for(int i = ((int)l_TimeIntervalList.size())-1; i>=0; i--){
    		double l_dInterval = l_TimeIntervalList[i].dTimeInterval;
    		l_dwTimeDiff = l_dwTimeCurrent - m_FuncChkOnTimeNotifyList[l_TimeIntervalList[i].uiIndex].dLastTriggerTime;
    		if (l_dwTimeDiff >= l_dInterval){
    			fireCheckFunctionOnTimeNotify(l_TimeIntervalList[i].uiIndex, l_dwTimeCurrent, 1, 0);
    			m_FuncChkOnTimeNotifyList[l_TimeIntervalList[i].uiIndex].dLastTriggerTime = l_dwTimeCurrent;
    		}
    	}

    	/**
    	//reset trigger
    	if ((l_iIndexForAction == ((int)l_TimeIntervalList.size())-1) || (l_dwTimeDiff >= m_dBigestTimeInterval)){
    		//fire function notify event
    		fireCheckFunctionOnTimeNotify(l_TimeIntervalList[(l_iIndexForAction)].uiIndex, l_dwTimeCurrent, 1, 0);

    		l_dwTimeLastChk = CAxcTime::GetCurrentUTCTimeMs();
    		l_uiChkIndexMakeSureEveryFuncTrigger =0;

    		continue;
    	}

    	if (l_iIndexForAction >=0){

    		 // 	current time count: 8.247 seconds
    		 // 	time Interval list:
    		 // 	{2, 3, 3, 7, 7.020, 9, 9, 9, 11}
    		 // 	       ^
    		 // 	          ^
    		 // 	    last  current
    		 // 	               ^
    		 //A	             wrong because diff. 20 ms more than CHeatFinderTimer::m_iInterval (default is 100 milliseconds)
    		 // 	                    ^^
    		 //B	    if 'last' & 'current' correct. But here has 9 the same interval function need to be triggered.
    		 //
    		 // FIXME: only re-do 'one' shift.
    		 // But I try to add protect at AddCheckFunctionOnTimeNotifyEvent,
    		 // which will avoid the interval smaller than m_iInterval.

    		if (l_iIndexForAction > (l_uiChkIndexMakeSureEveryFuncTrigger+1)){

    			 // A:
    			 // because 2 different time stamp diff. less than CHeatFinderTimer::m_iInterval
    			 // need to fire the lost function triggering.


    			//fire function notify event
    			fireCheckFunctionOnTimeNotify(l_TimeIntervalList[(l_uiChkIndexMakeSureEveryFuncTrigger+1)].uiIndex, l_dwTimeCurrent, 1, 0);
    			fireCheckFunctionOnTimeNotify(l_TimeIntervalList[(l_iIndexForAction)].uiIndex, l_dwTimeCurrent, 1, 0);

    			l_uiChkIndexMakeSureEveryFuncTrigger = l_iIndexForAction;

    		}else if (l_iIndexForAction < l_uiChkIndexMakeSureEveryFuncTrigger){
    			//it can not be possible!!!
    			printf("E: [%s] Timer login error: index record wrong(current %u: last record %u)\n", "Timer", l_iIndexForAction, l_uiChkIndexMakeSureEveryFuncTrigger);
    			//work around: reset
    			l_dwTimeLastChk = CAxcTime::GetCurrentUTCTimeMs();
    			l_uiChkIndexMakeSureEveryFuncTrigger =0;
    		}else{
    			// l_iIndexForAction == l_uiChkIndexMakeSureEveryFuncTrigger
				//the same time interval will be triggered.

    			 // B:
    			bool l_bIsLoopChk = true;
				while(l_bIsLoopChk){
					//fire function notify event
					fireCheckFunctionOnTimeNotify(l_TimeIntervalList[(l_uiChkIndexMakeSureEveryFuncTrigger)].uiIndex, l_dwTimeCurrent, 1, 0);

					//true: need to fire next
					l_bIsLoopChk =
							(l_TimeIntervalList[l_uiChkIndexMakeSureEveryFuncTrigger].dTimeInterval == l_TimeIntervalList[(l_uiChkIndexMakeSureEveryFuncTrigger+1)].dTimeInterval)?
									true:false;

					l_uiChkIndexMakeSureEveryFuncTrigger +=1;
				}
    		}

    	}
    	**/


        CAxcTime::SleepMilliSeconds(m_iInterval);
    }


    return 1;
}
