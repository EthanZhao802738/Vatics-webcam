#ifndef HEATFINDERUDPSEND_H
#define HEATFINDERUDPSEND_H

#include "globaldef.h"
#include <mutex>
#include <vector>

class CHeatFinderUdpSend
{
public:
    CHeatFinderUdpSend();
    ~CHeatFinderUdpSend();

    bool Run();
    void Stop();
    void SetMaxUdpSession(const int value);
    int GetMaxUdpSession();
    /**
    static void OnRecivedSendMsg(void *pContext, axc_byte *pBuf, const axc_i32 size, const axc_ddword channelIndex)
    {
        CHeatFinderUdpSend *pSender = reinterpret_cast<CHeatFinderUdpSend*> (pContext);
        pSender->OnRecivedSendMsg(pBuf, size, channelIndex);
    }**/

    void OnRecivedSendMsg(axc_byte *pBuf, const axc_i32 size, const axc_ddword channelIndex);
    
protected:
    CAxcThread m_hThread;       //g_threadUdpSend
    bool m_bStopThread;         //stop thread flag
    std::mutex m_locker;
    int m_iMaxUdpSeesion;
    CAxcSocketUdp m_sockUdpSend;
    std::vector<T_UDP_SESSION> m_UdpSessionList;

    CAxcFifo m_queueNetDataForSend;
	axc_i32 m_iQueuePacketSize;

    static axc_dword thread_process(CAxcThread* pThread, void* pContext)
    {
        CHeatFinderUdpSend *pSender = reinterpret_cast<CHeatFinderUdpSend*> (pContext);
        return pSender->thread_process();
    }
    axc_dword thread_process();
};

#endif // HEATFINDERUDPSEND_H
