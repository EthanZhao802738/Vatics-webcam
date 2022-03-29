#ifndef HEATFINDERUDPMEDIASRV_H
#define HEATFINDERUDPMEDIASRV_H

#include "globaldef.h"
#include <mutex>
#include <vector>

class CHeatFinderUdpMediaClient
{
public:
    CHeatFinderUdpMediaClient();
    ~CHeatFinderUdpMediaClient();

    bool Run();
    void Stop();

    /**
     * save in queue, and send by thread
     */
    void OnRecivedSendMsg(axc_byte *pBuf, const axc_i32 size, const axc_ddword channelIndex);
    void OnRecivedSendMsgThermal(axc_byte *pBuf, const axc_i32 size, const axc_ddword channelIndex);
    void OnReceivedDirectSend(axc_byte *pBuf, const axc_i32 size, const axc_ddword channelIndex);
protected:
    CAxcThread m_hThread;       //g_threadUdpSend
    bool m_bStopThread;         //stop thread flag
    std::mutex m_locker;
    std::vector<xSendBuf*> m_SendBufList;   //hamlet
    //CAxcSocketUdp m_sockUdpSend;
    int m_iUDPSocketHandle;
    struct sockaddr_in m_sockaddr_in_H264DataAddress, m_sockaddr_in_ThermalDataAddress;
    std::vector<xSendBuf*> m_SendBufListThermal;    //hamlet
    CAxcThread m_hThread_thermal;       //g_threadUdpSend
    std::mutex m_thermal_locker;

    //CAxcFifo m_queueNetDataForSend;
    //axc_i32 m_iQueueTotalBytes;
	//axc_i32 m_iQueuePacketSize;
	//unsigned int m_max_SendBuffer_size;

    static axc_dword thread_process(CAxcThread* pThread, void* pContext)
    {
        CHeatFinderUdpMediaClient *pSender = reinterpret_cast<CHeatFinderUdpMediaClient*> (pContext);
        return pSender->thread_process();
    }
    axc_dword thread_process();

    static axc_dword thread_process_thermal(CAxcThread* pThread, void* pContext)
    {
        CHeatFinderUdpMediaClient *pSender = reinterpret_cast<CHeatFinderUdpMediaClient*> (pContext);
        return pSender->thread_process_thermal();
    }
    axc_dword thread_process_thermal();

};

#endif // HEATFINDERUDPMEDIASRV_H
