#ifndef HEATFINIDERUDPCOMMAND_H
#define HEATFINIDERUDPCOMMAND_H

#include "globaldef.h"
#include <mutex>
#include <vector>

#define FLAG_UDPRECEIVE_PROBE (0)
#define FLAG_UDPRECEIVE_THERMAL_IVS (1)

struct tagUdpReceivedCommandEvent{
	unsigned int flag;
	double doubleRecTime;
	axc_dword dwFromIp;
	axc_word wFromPort;
};
typedef struct tagUdpReceivedCommandHandler
{
    void *pContext;
    struct tagUdpReceivedCommandEvent objEvent;
}xUdpReceivedCommandHandler;

typedef void (*OnUdpBroadcastRenewNotifyEvent)(void *pContext);
typedef struct tagUdpBroadcastRenewNotifyEventHandler
{
	OnUdpBroadcastRenewNotifyEvent fnEvent;
    void *pContext;
}xUdpBroadcastRenewNotifyEventHandler;

class CHeatFiniderUdpCommand
{
public:
    CHeatFiniderUdpCommand();
    ~CHeatFiniderUdpCommand();

    bool Run();
    void Stop();

    bool SetBroadcastInformationBackoff(CAxcString* caxcStrObj);
    void AddUdpReceivedEvent(tagUdpReceivedCommandEvent inputevent, void *pContext);
    void AddBroadcastRenewNotifyEvent(OnUdpBroadcastRenewNotifyEvent fnEvent, void *pContext);

protected:
    CAxcThread m_hThread;       //g_threadUdpCommand
    CAxcThread m_hThread_Response;
    CAxcSocketUdp m_sockUdpCommand;
    bool m_bStopThread;         //stop thread flag
    std::mutex m_locker;

    double m_dbUdpCmdRecved, m_dbUdpCmdRecved_Last, m_dbTheSameStart;

    bool m_bIsprintDebugMessage;
    bool m_bForceStop_thread_process, m_bneedCheckUDPRecv;
    int m_nReOpenUdpCount;
    CAxcString m_caxcStrBroadcastRecord;
    double m_doubleBroadcastRenewTimestamp;

    std::vector<xUdpReceivedCommandHandler> m_UdpReceiveEventList;
    std::mutex m_rec_locker;

    std::vector<xUdpBroadcastRenewNotifyEventHandler> m_UdpBroadcastRenewNotifyEventList;

    static axc_dword thread_process(CAxcThread* pThread, void* pContext)
    {
        CHeatFiniderUdpCommand *pSender = reinterpret_cast<CHeatFiniderUdpCommand*> (pContext);
        return pSender->thread_process();
    }
    axc_dword thread_process();

    static axc_dword thread_process_response(CAxcThread* pThread, void* pContext)
	{
		CHeatFiniderUdpCommand *pSender = reinterpret_cast<CHeatFiniderUdpCommand*> (pContext);
		return pSender->thread_process_response();
	}
	axc_dword thread_process_response();

    void OnUdpResponseRecievedEvent();
    void fireNeedtoRenewBroadcastRecordEvent();
    bool CheckUDPRecv_Available();
    bool ReOpenUDPPort();
};

#endif // HEATFINIDERUDPCOMMAND_H
