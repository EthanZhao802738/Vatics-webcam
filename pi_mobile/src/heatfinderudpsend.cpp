#include "heatfinderudpsend.h"

CHeatFinderUdpSend::CHeatFinderUdpSend():
    m_hThread("net/udp_send"),
    m_bStopThread(true),
    m_iMaxUdpSeesion(64),
    m_sockUdpSend("net/udp_send"),
    m_queueNetDataForSend("net/udp_queue")
{
	m_iQueuePacketSize = (1600);
}

CHeatFinderUdpSend::~CHeatFinderUdpSend()
{
    m_bStopThread = true;
    if (m_hThread.IsValid())
        m_hThread.Destroy(2000);

    if (m_sockUdpSend.IsValid()){

        m_sockUdpSend.Destroy();
    }
    m_queueNetDataForSend.Destroy();
    GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] Destroy\n", "Udp View Sender");

}

bool CHeatFinderUdpSend::Run()
{
    bool bRst = false;
    Stop();

    if(!m_sockUdpSend.Create(PORT_UDP_VIEWSEND, 0, 64*1024, 64*1024))
        return bRst;

    m_bStopThread = false;
    if (!m_hThread.Create(thread_process, this))
        return bRst;

    GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] Start\n", "Udp View Sender");
    bRst = true;
    return bRst;
}

void CHeatFinderUdpSend::Stop()
{
    m_bStopThread = true;
    if (m_hThread.IsValid())
        m_hThread.Destroy(2000);

    //if (m_sockUdpSend.IsValid()){

    //    m_sockUdpSend.Destroy();
    //}
    //m_queueNetDataForSend.Destroy();
    GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] Stop\n", "Udp View Sender");
}

void CHeatFinderUdpSend::SetMaxUdpSession(const int value)
{
    m_iMaxUdpSeesion = value;
}

int CHeatFinderUdpSend::GetMaxUdpSession()
{
    return m_iMaxUdpSeesion;
}

void CHeatFinderUdpSend::OnRecivedSendMsg(axc_byte *pBuf, const axc_i32 size, const axc_ddword channelIndex)
{
	/**
	 * not use any more
	 */

	if ((m_bStopThread) || (m_queueNetDataForSend.IsValid() == axc_false)){
		//printf("W: [%s] udp media client drop this data, because queue is not ready\n", "Udp View Sender");
		return;
	}

	if (m_UdpSessionList.size() <= 0){
		//clear
		m_queueNetDataForSend.Reset();

		//pass
		//return;
	}
    m_locker.lock();
	if (m_queueNetDataForSend.GetCount() > (m_queueNetDataForSend.GetMaxItemCount()-1)){
		GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] udp media client remove oldest data, because queue is full (%u/ %u)\n", "Udp View Sender", m_queueNetDataForSend.GetCount(), m_queueNetDataForSend.GetMaxItemCount());

		//clear
		m_queueNetDataForSend.Reset();
	}

	axc_bool axcbQueuePushRst = axc_false;
	axc_i32 ilastDataIndex = m_queueNetDataForSend.GetCount();
	axc_byte abyBuf[m_iQueuePacketSize];

	memcpy(abyBuf, pBuf, size);
	AxcSetLastError(0);

	if (channelIndex == UDPSENDER_OUTPUT_THERMAL){
		axcbQueuePushRst = m_queueNetDataForSend.Push(abyBuf, size, ADE2CAM_CHANNEL_LEPTONRAW);
	}else if (channelIndex == UDPSENDER_OUTPUT_VISION){
		axcbQueuePushRst = m_queueNetDataForSend.Push(abyBuf, size, ADE2CAM_CHANNEL_VISUALMAIN);
	}
	else
	{
    	//CHeatFinderUtility::PrintNowData(1);
		GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] unknown %llu \n", "Udp View Sender", channelIndex);
	}
	if (axcbQueuePushRst == axc_false ){
    	//CHeatFinderUtility::PrintNowData(1);
		GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] tcp client add data %p@%d to queue (init=%d) fail:%d %s\n", "Udp View Sender",pBuf, size, m_queueNetDataForSend.IsValid(), axcbQueuePushRst, (ilastDataIndex == m_queueNetDataForSend.GetCount())?"data not push into queue":"queue valid");
		GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: Detail: %s\n", AxcGetLastErrorText());
	}
    m_locker.unlock();
}

axc_dword CHeatFinderUdpSend::thread_process()
{

	GlobalDef *pGlobal = (GlobalDef *) CHeatFinderUtility::GetGlobalsupport();
	CAxcFifo *pQueue = pGlobal->GetNetUDPDataQueue();

	// change 'm_queueNetDataForSend.' to 'pQueue->'


	while(!pQueue->IsValid()){
		m_hThread.SleepMilliSeconds(20);
	}

	while(!m_sockUdpSend.IsValid()){
		m_hThread.SleepMilliSeconds(2000);
	    GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s:thread_process] socket not ready\n", "Udp View Sender");
	}

    char szRecvBuffer[1600];
    //axc_byte txdata[1600];
    axc_byte abyPacket[m_iQueuePacketSize];


    //unsigned int count =30;
    int iRecvLen = 0;
    DWORD dwRemoteIp = 0;
    WORD wRemotePort = 0;
    char szValue[256] = {""};
    axc_i32 i = 0;
    time_t tCurrTime = 0;

    memset(szRecvBuffer, 0, sizeof(szRecvBuffer));
    memset(abyPacket, 0, sizeof(abyPacket));

    GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s:thread_process] enter\n", "Udp View Sender");
    while(!m_bStopThread)
    {
        // Receive udp polling command
        while((iRecvLen = m_sockUdpSend.Recv(szRecvBuffer,sizeof(szRecvBuffer)-1,&dwRemoteIp,&wRemotePort,axc_false)) > 0)
        {
            szRecvBuffer[iRecvLen] = 0;
            //if (iRecvLen > 0){
            	//printf("D: [%s:thread_process] get message: %s\n", "Udp View Sender", szRecvBuffer);
            //}

            if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(szRecvBuffer, (axc_dword)iRecvLen, "command", szValue, (axc_dword)sizeof(szValue)) > 0)
            {
            	//printf("D: [%s:thread_process] get command: %s\n", "Udp View Sender", szValue);
                if(0 == strcmp(szValue,"polling"))
                {
                	//CHeatFinderUtility::PrintNowData(1);
                	//printf("D: [%s:thread_process] get polling\n", "Udp View Sender");

                    WORD wStreamMode = UDP_SESSION_CONTEXT_OPEN_THERMAL | UDP_SESSION_CONTEXT_OPEN_VISION;
                    if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(szRecvBuffer, (axc_dword)iRecvLen, "open_stream", szValue, (axc_dword)sizeof(szValue)) > 0)
                    {
                        wStreamMode = 0;
                        if(NULL != strstr(szValue,"thermal")) wStreamMode |= UDP_SESSION_CONTEXT_OPEN_THERMAL;
                        if(NULL != strstr(szValue,"vision")) wStreamMode |= UDP_SESSION_CONTEXT_OPEN_VISION;
                    }
                    //T_UDP_SESSION udpSession;
                    //udpSession.tLastPollingTime = time(NULL);
                    //udpSession.wStreamMode = wStreamMode;
                    //bool bFound = false;
                    int iIndex = -1;
                    for(unsigned int i = 0; i < m_UdpSessionList.size(); ++i)
                    {
                    	T_UDP_SESSION UdpSessionPart;
                    	memcpy(&UdpSessionPart, &m_UdpSessionList[i], sizeof(UdpSessionPart));
                        if (dwRemoteIp == UdpSessionPart.dwRemoteIp &&
                            wRemotePort == UdpSessionPart.wRemotePort)
                        {
                            iIndex = i;
                            break;
                        }
                    }

                    if (iIndex < 0)
                    {
                        //new
                        T_UDP_SESSION udpSession;
                        udpSession.dwRemoteIp = dwRemoteIp;
                        udpSession.wRemotePort = wRemotePort;
                        udpSession.tLastPollingTime = time(NULL);
                        udpSession.wStreamMode = wStreamMode;
                        m_UdpSessionList.push_back(udpSession);

                        //unsigned char* pbyte = (unsigned char*) &dwRemoteIp;
                        //printf("D: [%s:thread_process] update new [%u.%u.%u.%u:%u] [%d]\n", "Udp View Sender", pbyte[3], pbyte[2], pbyte[1], pbyte[0], wRemotePort, wStreamMode);
                    }
                    else
                    {
                        //old update info
                        m_UdpSessionList[i].tLastPollingTime = time(NULL);
                        m_UdpSessionList[i].wStreamMode = wStreamMode;
                        //unsigned char* pbyte = (unsigned char*) &dwRemoteIp;
                        //printf("D: Network: new UDP-session [%u.%u.%u.%u:%u]\n", pbyte[3], pbyte[2], pbyte[1], pbyte[0], wRemotePort);

                        //unsigned char* pbyte = (unsigned char*) &dwRemoteIp;
                        //printf("D: [%s:thread_process] renew old [%u.%u.%u.%u:%u] [%d]\n", "Udp View Sender", pbyte[3], pbyte[2], pbyte[1], pbyte[0], wStreamMode, wStreamMode);
                    }

                }
            }
        }

    	if (pQueue->GetCount() <= 0){
    		//printf("N: [%s:thread_process] no data\n", "Udp View Sender");
    		m_hThread.SleepMilliSeconds(20);
    		continue;
    	}

		axc_i32 iPacketSize = 0;
		axc_ddword ddwChannelIndex = 0;
		axc_ddword ddwImageRes = 0;

		if (m_bStopThread){
    		GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tNORMALTrace_MSK, "N: [%s:thread_process] stop... \n", "Udp View Sender");
			break;
		}

        m_locker.lock();
		if(!pQueue->Pop(abyPacket, (axc_i32)sizeof(abyPacket), &iPacketSize, &ddwChannelIndex, &ddwImageRes)){
	        m_locker.unlock();
			m_hThread.SleepMilliSeconds(15);
			continue;
		}
	    m_locker.unlock();

        tCurrTime = time(NULL);
        //printf("N: [%s:thread_process] has data to send \n", "Udp View Sender");
        //for(int i = m_UdpSessionList.size() - 1; i >= 0 ; i--)
        unsigned int udpMaxSessionNumber = m_UdpSessionList.size();
        for(unsigned int i =0; i<udpMaxSessionNumber; i++)
        {
        	if (m_bStopThread){
				break;
			}

            if (0 == m_UdpSessionList[i].tLastPollingTime)
                m_UdpSessionList[i].tLastPollingTime = tCurrTime;

            if ((tCurrTime - m_UdpSessionList[i].tLastPollingTime) > 15){
            	unsigned char* pbyte = (unsigned char*) &m_UdpSessionList[i].dwRemoteIp;
            	GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s:thread_process] del [%u.%u.%u.%u:%u]\n", "Udp View Sender", pbyte[3], pbyte[2], pbyte[1], pbyte[0], m_UdpSessionList[i].wStreamMode);

                m_UdpSessionList.erase(m_UdpSessionList.begin() + i);

                if (m_UdpSessionList.size() <=0 ){
                	//clear
                	pQueue->Reset();
                }
            }

            else
            {
                // send to client
                if((ADE2CAM_CHANNEL_LEPTONRAW == ddwChannelIndex && (m_UdpSessionList[i].wStreamMode & UDP_SESSION_CONTEXT_OPEN_THERMAL)) ||
                   ((ADE2CAM_CHANNEL_VISUALMAIN == ddwChannelIndex || ADE2CAM_CHANNEL_VISUALSUB == ddwChannelIndex) && (m_UdpSessionList[i].wStreamMode & UDP_SESSION_CONTEXT_OPEN_VISION)) )
                {
                    if(m_sockUdpSend.Send(abyPacket, iPacketSize, m_UdpSessionList[i].dwRemoteIp, m_UdpSessionList[i].wRemotePort) > 0)
                    {
                    	CHeatFinderUtility::SetTwinkleLedShowNetworkUsed();
                    }else{
                    	//m_hThread.SleepMilliSeconds(15);
                    }
                }
            }
        }
    }

	GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tNORMALTrace_MSK, "N: [%s:thread_process] Stop \n", "Udp View Sender");

    return 1;
}
