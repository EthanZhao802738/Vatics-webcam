#ifndef HEATFINDERTCPLISTENER_H
#define HEATFINDERTCPLISTENER_H

#include "globaldef.h"
#include <mutex>
#include <vector>
#include "heatfinderMonitor.h"

#define ThisCommandIsVideoOut (true)

typedef void (*OnTcpAcceptSessionEvent)(void *pContext, CAxcSocketTcpSession *pValue, const unsigned int luValue);
typedef struct tagTcpAcceptSessionEventHandler
{
    OnTcpAcceptSessionEvent fnEvent;
    void *pContext;
}xTcpAcceptSessionEventHandler;

struct TcpListenNewSessionEventHandler{
	void* pSender;
	CAxcSocketTcpSession* pSession;
};

typedef void (*OnTcpNeedtoRemoveSessionEvent)(void *pContext, CAxcSocketTcpSession *pValue);
typedef struct tagTcpNeedtoRemoveSessionEventHandler
{
	OnTcpNeedtoRemoveSessionEvent fnEvent;
    void *pContext;
}xTcpNeedtoRemoveSessionEventHandler;

typedef void (*OnTcpBroadcastRenewNotifyEvent)(void *pContext);
typedef struct tagTcpBroadcastRenewNotifyEventHandler
{
	OnTcpBroadcastRenewNotifyEvent fnEvent;
    void *pContext;
}xTcpBroadcastRenewNotifyEventHandler;

typedef int (*OnSessionSendtofunc) (void *pContext, CAxcList *listSessionSend, CAxcList *listSessionDrop);
typedef struct tagOnSessionSendtoEventHandler
{
	OnSessionSendtofunc fnEvent;
    void *pContext;
}xOnSessionSendtoEventHandler;

typedef int (*OnReceivedUpgradeEvent) (void *pContext, double dDelaytimestampMS);
typedef struct tagReceivedUpgradeEventHandler
{
	OnReceivedUpgradeEvent fnEvent;
    void *pContext;
}xOnReceivedUpgradeEventHandler;

/**
 * Error message
 */
typedef enum _E_TCP_LISTERN_CREATE_ERROR
{
	NOERROR =0x0000,
	OPENPORT =0x0001,
	STARTLISTEN =0x0002,
	SESSIONLISTCREATE =0x0003,
	PTHREAD1 =0x0004,
	PTHREAD2 =0x0005
} E_TCP_LISTERN_CREATE_ERROR;

/**
 * @brief The CHeatFinderTcpListener class
 *
 */
class CHeatFinderTcpListener
{
public:
    CHeatFinderTcpListener();
    ~CHeatFinderTcpListener();
    unsigned int Run();
    void Stop();

    void SetPrintNewTcpSessionAccepted(const bool value);

    void AddTcpAcceptSessionEvent(OnTcpAcceptSessionEvent fnEvent, void *pContext);
    void AddTcpNeedtoRemoveSessionEvent(OnTcpNeedtoRemoveSessionEvent fnEvent, void *pContext);
    void AddBroadcastRenewNotifyEvent(OnTcpBroadcastRenewNotifyEvent fnEvent, void *pContext);

    bool SetBroadcastInformationBackoff(CAxcString* caxcStrObj);

    bool OnRemoveSession(CAxcSocketTcpSession* pSession);
    void AddSessionSendtoEvent(OnSessionSendtofunc fnEvent, void *pContext);

    void AddTcpReceivedUpgreadeEventNotify(OnReceivedUpgradeEvent fnEvent, void *pContext);

    static void OnFuncChkTCPTimeEventNotify(void *pContext, const double dCurrentTimeMS, const unsigned int uiAction, const signed int siValue){
        	CHeatFinderTcpListener *pSender = reinterpret_cast<CHeatFinderTcpListener*> (pContext);
        	return pSender->OnFuncChkTCPTimeEventNotify(dCurrentTimeMS, uiAction, siValue);
	}
	void OnFuncChkTCPTimeEventNotify(const double dCurrentTimeMS, const unsigned int uiAction, const signed int siValue);
protected:
    CAxcThread m_hThread;       //g_threadTcpListen

    bool m_bStopThread;         //stop thread flag
    CAxcSocketTcpListen		m_sockTcpListen;
    CAxcList				m_listTcpSession;

    std::mutex m_locker;
    std::vector<xTcpAcceptSessionEventHandler> m_TcpAcceptSessionEventList;
    std::vector<xTcpNeedtoRemoveSessionEventHandler> m_TcpNeedtoRemoveSessionEventList;
    std::mutex m_locker_session_delivery;
    std::vector<xTcpBroadcastRenewNotifyEventHandler> m_TcpBroadcastRenewNotifyEventList;
    std::mutex m_locker_broadcast_renew_notify;
    std::vector<xOnSessionSendtoEventHandler> m_TcpSessionSendtoList;

    bool m_bPrintHideDebugMessage;
    double m_doubleBroadcastRenewTimestamp;
    CAxcString m_caxcStrBroadcastRecord;
    CAxcThread m_hSenderThread;

    std::vector<xOnReceivedUpgradeEventHandler> m_TcpReceivedUpgradeEventList;

    CHeatfinderMonitor 	m_tcpeventnotifyMonitor;

    static axc_dword thread_process(CAxcThread* pThread, void* pContext)
    {
        CHeatFinderTcpListener *pSender = reinterpret_cast<CHeatFinderTcpListener*> (pContext);
        return pSender->thread_process();
    }
    axc_dword thread_process();

    static axc_dword thread_tcpsession_delivery(CAxcThread* pThread, void* pContext){
    	TcpListenNewSessionEventHandler *pHandler = reinterpret_cast<TcpListenNewSessionEventHandler*> (pContext);


        CHeatFinderTcpListener* pSender = reinterpret_cast<CHeatFinderTcpListener*> (pHandler->pSender);
        CAxcSocketTcpSession* pSession = pHandler->pSession;

        return pSender->thread_tcpsession_delivery(pThread, pSession);
    }
    axc_dword thread_tcpsession_delivery(CAxcThread* pThread, CAxcSocketTcpSession* pSession);

    static axc_dword thread_sendpkg_process(CAxcThread* pThread, void* pContext)
    {
        CHeatFinderTcpListener *pSender = reinterpret_cast<CHeatFinderTcpListener*> (pContext);
        return pSender->thread_sendpkg_process();
    }
    axc_dword thread_sendpkg_process();

    void fireTcpAcceptSessionEvent(CAxcSocketTcpSession *pValue, const bool bValue);
    void fireNeedtoRemoveSessionEvent(CAxcSocketTcpSession *pValue);
    void fireNeedtoRenewBroadcastRecordEvent();
    bool onGetBroadcastLocalRecord(CAxcString* pCAcxcStrObj, double* pdwTimestamp);
    void fireTcpReceivedUpgradeEventNotify(double dDelaytimestampMS);

    //process list
    axc_bool ProcessTcpCommand_Polling(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession);
    axc_bool ProcessTcpCommand_GetNetConfig(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession);
    axc_bool ProcessTcpCommand_Probe(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession);
    axc_bool ProcessTcpCommand_GetConfig(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession);
    axc_bool ProcessTcpCommand_GetThermalIvsConfig(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession);
    axc_bool ProcessTcpCommand_GetThermalIvsResult(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession);
    axc_bool ProcessTcpCommand_SetEmissivity(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession);
    axc_bool ProcessTcpCommand_GetEmissivity(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession);
    axc_bool ProcessTcpCommand_SetThermalLeptonFFC(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession);
    axc_bool ProcessTcpCommand_GetThermalOverlayLocate(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession);
    axc_bool ProcessTcpCommand_Upgrade(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession);
    axc_bool ProcessTcpCommand_ReceivedGPIOServiceTrigger(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession);
    axc_bool ProcessTcpCommand_ReceivedGPIOServiceNotify(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession);

    axc_bool ProcessTcpCommand_SetEventNotifyConfigAboutTemperature(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession);
    axc_bool ProcessTcpCommand_GetEventNotifyConfigAboutTemperature(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession);

    axc_i32 OnSessionSendto(CAxcList *listSessionSend, CAxcList *listSessionDrop);


};

#endif // HEATFINDERTCPLISTENER_H
