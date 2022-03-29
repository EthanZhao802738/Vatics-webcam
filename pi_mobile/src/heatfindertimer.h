#ifndef HEATFINDERTIMER_H
#define HEATFINDERTIMER_H

//#include <thread> std::thread is c++11
#include "globaldef.h"
#include <mutex>
#include <vector>

typedef void (*OnLedNetTwinkleNotify)(void *pContext, const bool value);
typedef struct tagLedNetTwinkleNotifyHandler
{
    OnLedNetTwinkleNotify fnEvent;
    void *pContext;
}xLedNetTwinkleNotifyHandler;

//
typedef void (*OnLedAlarmTwinkleNotify)(void *pContext, const bool value);
typedef struct tagLedAlarmTwinkleNotifyHandler
{
    OnLedAlarmTwinkleNotify fnEvent;
    void *pContext;
}xLedAlarmTwinkleNotifyHandler;

//
typedef void (*OnLedStatusTwinkleNotify)(void *pContext, const bool value);
typedef struct tagLedStatusTwinkleTimerNotifyHandler
{
    OnLedStatusTwinkleNotify fnEvent;
    void *pContext;
}xLedStatusTwinkleTimerNotifyHandler;

//
typedef void (*OnLedStatusTurnOnNotify)(void *pContext, const bool value);
typedef struct tagLedStatusTurnOnTimerNotifyHandler
{
    OnLedStatusTurnOnNotify fnEvent;
    void *pContext;
}xLedStatusTurnOnTimerNotifyHandler;

//Function auto keep-checking structure
//
typedef void (*OnFunctionCheckOnTimeNotify)(void *pContext, const double dCurrentTimeMS, const unsigned int uiAction, const signed int siValue);
typedef struct tagOnFunctionCheckOnTimeNotifyHandler{
	OnFunctionCheckOnTimeNotify fnEvent;
	double dTimeInterval;
	double dLastTriggerTime;
	char cstr32Name[32];
	void *pContext;
}xOnFunctionCheckOnTimeNotifyHandler;

typedef struct tagFunctionCheckListHandler{
	double dTimeInterval;
	unsigned int uiIndex;
}xFunctionCheckListHandler;
/**
 * @brief The CHeatFinderTimer class
 * 使用Thread做Timer，主要是檢核狀態
 */
class CHeatFinderTimer
{
public:
    CHeatFinderTimer();
    ~CHeatFinderTimer();
    void SetInterval(const int interval);
    int GetInterval();
    bool Run();
    void Stop();

    void AddLedNetTwinkleNotifyEvent(OnLedNetTwinkleNotify fnEvent, void *pContext);
    void AddLedAlarmTwinkleNotifyEvent(OnLedAlarmTwinkleNotify fnEvent, void *pContext);
    void AddLedStatusTwinkleNotifyEvent(OnLedStatusTwinkleNotify fnEvent, void *pContext);
    void AddLedStatusTurnOnNotifyEvent(OnLedStatusTurnOnNotify fnEvent, void *pContext);

    static void OnLedNetTwinkleNotify_func(void *pContext, const bool value)
    {
        CHeatFinderTimer *pSender = reinterpret_cast<CHeatFinderTimer*> (pContext);
        pSender->OnLedNetTwinkleNotify_func(value);
    }

    static void OnLedAlarmTwinkleNotify_func(void *pContext, const bool value)
    {
        CHeatFinderTimer *pSender = reinterpret_cast<CHeatFinderTimer*> (pContext);
        pSender->OnLedAlarmTwinkleNotify_func(value);
    }

    static void OnLedStatusTwinkleNotify_func(void *pContext, const bool value)
    {
        CHeatFinderTimer *pSender = reinterpret_cast<CHeatFinderTimer*> (pContext);
        pSender->OnLedStatusTwinkleNotify_func(value);
    }

    static void OnLedStatusTurnOnNotify_func(void *pContext, const bool value)
    {
        CHeatFinderTimer *pSender = reinterpret_cast<CHeatFinderTimer*> (pContext);
        pSender->OnLedStatusTurnOnNotify_func(value);
    }

    static void OnTCPProbeNotify_func(void *pContext, axc_dword value)
    {
        CHeatFinderTimer *pSender = reinterpret_cast<CHeatFinderTimer*> (pContext);
        pSender->OnTCPProbeNotify_func(value);
    }

    void AddCheckFunctionOnTimeNotifyEvent(OnFunctionCheckOnTimeNotify fnEvent, double dTimeInterval, const char* pcStr, void *pContext);
    bool SetPrintDebugMsg(bool bValue);
protected:
    CAxcThread m_hThread;       //g_threadTimer
    int m_iInterval;            //sleep milliseconds
    bool m_bStopThread;         //stop thread flag
    bool m_bLedNetTwinkle;        //Led Net Twinkle status
    bool m_bLedAlarmTwinkle;      //Led Alarm Twinkle status
    bool m_bLedStatus;          //Led Status
    bool m_bLedStatusTurnOn;    //Led status turn on light
    //bool m_bBroadcastInfoRefresh;//need update

    axc_dword m_tmTcpProbe; //timestamp TCP Probe
    bool m_bIsPrintDebugMsg;
    double m_dBigestTimeInterval;

    std::mutex m_locker;
    std::vector<xLedNetTwinkleNotifyHandler> m_LedNetTwinkleNotifyList;
    std::vector<xLedAlarmTwinkleNotifyHandler> m_LedAlarmTwinkleNotifyList;
    std::vector<xLedStatusTwinkleTimerNotifyHandler> m_LedStatusTwinkleNotifyList;
    std::vector<xLedStatusTurnOnTimerNotifyHandler> m_LedStatusTurnOnNotifyList;

    std::vector<xOnFunctionCheckOnTimeNotifyHandler> m_FuncChkOnTimeNotifyList;

    static axc_dword thread_process(CAxcThread* pThread, void* pContext)
    {
        CHeatFinderTimer *pSender = reinterpret_cast<CHeatFinderTimer*> (pContext);
        return pSender->thread_process();
    }
    axc_dword thread_process();

    void OnLedNetTwinkleNotify_func(const bool value);
    void OnLedAlarmTwinkleNotify_func(const bool value);
    void OnLedStatusTwinkleNotify_func(const bool value);
    void OnLedStatusTurnOnNotify_func(const bool value);
    void OnTCPProbeNotify_func(axc_dword value);

    void fireLedNetTwinkleNotify(const bool value);
    void fireLedAlarmTwinkleNotify(const bool value);
    void fireLedStatusNotify(const bool value);
    void fireLedStatusTurnOnNotify(const bool value);

    void fireCheckFunctionOnTimeNotify(unsigned int uiIndex, double dCurrentTimeMS, unsigned int uiAction, signed int siValue);
    void OnChangeFunctionIntervalTimeList(std::vector<xFunctionCheckListHandler> &dList);
};

#endif // HEATFINDERTIMER_H
