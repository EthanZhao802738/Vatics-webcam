/**
 * This class is working for updating live view package to docker:media-server
 */

#include "heatfinderudpmediaclient.h"
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "heatfinderconfigmanager.h"

CHeatFinderUdpMediaClient::CHeatFinderUdpMediaClient():
    m_hThread("net/udp_MediaSrv"),
    m_bStopThread(true),
    //m_sockUdpSend("net/udp_MediaSrv"),
    m_iUDPSocketHandle(-1),
    //m_queueNetDataForSend("net/udp_queue/MediaSrv")
    m_hThread_thermal("net/udp_MediaSrv")
{

}

CHeatFinderUdpMediaClient::~CHeatFinderUdpMediaClient()
{
    m_bStopThread = true;
    if (m_hThread.IsValid())
        m_hThread.Destroy(2000);

    if (m_hThread_thermal.IsValid())
    	m_hThread_thermal.Destroy(2000);

    //m_sockUdpSend.Destroy();
    //m_queueNetDataForSend.Destroy();
    //hamlet
    if (m_SendBufList.empty() == false)
    {
        for(unsigned int i = 0; i < m_SendBufList.size(); ++i)
        {
            xSendBuf *pSendBuf = m_SendBufList[i];
            delete pSendBuf;
        }
        m_SendBufList.clear();
    }
    if (m_SendBufListThermal.empty() == false)
    {
        for(unsigned int i = 0; i < m_SendBufListThermal.size(); ++i)
        {
            xSendBuf *pSendBuf = m_SendBufListThermal[i];
            delete pSendBuf;
        }
        m_SendBufListThermal.clear();
    }
    GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] Destroy\n", "Udp Media Client");
}

bool CHeatFinderUdpMediaClient::Run()
{
    bool bRst = false;
    Stop();

    m_iUDPSocketHandle = socket(AF_INET, SOCK_DGRAM, 0);
	memset((char *) &m_sockaddr_in_H264DataAddress, 0, sizeof(m_sockaddr_in_H264DataAddress));
	memset((char *) &m_sockaddr_in_ThermalDataAddress, 0, sizeof(m_sockaddr_in_ThermalDataAddress));

	m_sockaddr_in_H264DataAddress.sin_family = AF_INET;
	m_sockaddr_in_H264DataAddress.sin_addr.s_addr = inet_addr(serverIP); // this is address of host which I want to send the socket
	m_sockaddr_in_H264DataAddress.sin_port = htons(serverH264DataPort);

	GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] h264 port %u  %u\n", "Udp Media Client", serverH264DataPort, m_sockaddr_in_H264DataAddress.sin_port);

	m_sockaddr_in_ThermalDataAddress.sin_family = AF_INET;
	m_sockaddr_in_ThermalDataAddress.sin_addr.s_addr = inet_addr(serverIP); // this is address of host which I want to send the socket
	m_sockaddr_in_ThermalDataAddress.sin_port = htons(serverThermalDataPort);

	GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] thermal port %u  %u\n", "Udp Media Client", serverThermalDataPort, m_sockaddr_in_ThermalDataAddress.sin_port);

    //if(!m_sockUdpSend.Create(0, 0, 64*1024, 64*1024))
    //    return bRst;

    m_bStopThread = false;
    if (!m_hThread.Create(thread_process, this))
        return bRst;
    if (!m_hThread_thermal.Create(thread_process_thermal, this))
        return bRst;

    GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] Start\n", "Udp Media Client");
    bRst = true;
    return bRst;
}

void CHeatFinderUdpMediaClient::Stop()
{
    m_bStopThread = true;
    if (m_hThread.IsValid())
        m_hThread.Destroy(2000);

    //m_sockUdpSend.Destroy();
    GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] Stop\n", "Udp Media Client");
}


void CHeatFinderUdpMediaClient::OnRecivedSendMsg(axc_byte *pBuf, const axc_i32 size, const axc_ddword channelIndex)
{
	//printf("N: [%s] Got a new Data. size:%d\n", "Udp Media Client", size);

	bool bIsNeedErase = false;
    m_locker.lock();
	unsigned int iQueueSize = m_SendBufList.size();

	if (iQueueSize >= m_SendBufList.max_size() -1){
		GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] udp media client remove oldest data, because queue is full (%u/ %u)\n", "Udp Media Client", iQueueSize, (m_SendBufList.max_size()-1));

		//clear
		bIsNeedErase = true;
		//m_SendBufList.erase(m_SendBufList.begin());
	}

    xSendBuf *xBuf = new xSendBuf();
    xBuf->CopyData(pBuf, size, channelIndex);

    if (bIsNeedErase){
        xSendBuf *xT= m_SendBufList.front();
    	m_SendBufList.erase(m_SendBufList.begin());
        delete xT;
    	iQueueSize = 0;
    }

    m_SendBufList.push_back(xBuf);
    unsigned int iQueueNewSize = m_SendBufList.size();
    //printf("N: [%s] Got a new Data. push finish\n", "Udp Media Client", size);

    if (iQueueNewSize <= iQueueSize){
    	//CHeatFinderUtility::PrintNowData(1);
    	GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] udp media client add data %p@%d to queue fail:%s\n", "Udp Media Client",pBuf, size, (iQueueNewSize <= iQueueSize)?"data not push into queue":"queue valid");
    }

    m_locker.unlock();
}

void CHeatFinderUdpMediaClient::OnRecivedSendMsgThermal(axc_byte *pBuf, const axc_i32 size, const axc_ddword channelIndex)
{
	//printf("N: [%s] Got a new Data. size:%d\n", "Udp Media Client", size);

	bool bIsNeedErase = false;

	m_thermal_locker.lock();
	unsigned int iQueueSize = m_SendBufListThermal.size();

	if (iQueueSize >= m_SendBufListThermal.max_size() -1){
		GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] udp media client remove oldest data, because thermal queue is full (%u/ %u)\n", "Udp Media Client", iQueueSize, (m_SendBufListThermal.max_size()-1));

		//clear
		bIsNeedErase = true;
		//m_SendBufListThermal.erase(m_SendBufListThermal.begin());
	}

    xSendBuf *xBuf = new xSendBuf();
    xBuf->CopyData(pBuf, size, channelIndex);

    if (bIsNeedErase){
        xSendBuf *xT = m_SendBufListThermal.front();
    	m_SendBufListThermal.erase(m_SendBufListThermal.begin());
        delete xT;
    	iQueueSize = 0;
    }

    m_SendBufListThermal.push_back(xBuf);
    unsigned int iQueueNewSize = m_SendBufListThermal.size();

    if (iQueueNewSize <= iQueueSize){
    	//CHeatFinderUtility::PrintNowData(1);
		GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] udp media client add data %p@%d to thermal queue fail:%s\n", "Udp Media Client",pBuf, size, (iQueueNewSize <= iQueueSize)?"data not push into queue":"queue valid");
	}

    m_thermal_locker.unlock();
}

void CHeatFinderUdpMediaClient::OnReceivedDirectSend(axc_byte *pBuf, const axc_i32 size, const axc_ddword channelIndex){

    if (channelIndex == ADE2CAM_CHANNEL_LEPTONRAW)
    {

    	axc_i32 iSend = sendto(m_iUDPSocketHandle, (char*) pBuf, size, 0,
    			(const struct sockaddr*) &m_sockaddr_in_ThermalDataAddress, sizeof(struct sockaddr_in));
        if (iSend <= 0){
            CHeatFinderUtility::MsgSendtoStrerror(errno);
        }else{

        }

    }
    //else if (xBuf.ddwChannelIndex & ADE2CAM_CHANNEL_VISUALMAIN)
    else if (channelIndex == ADE2CAM_CHANNEL_VISUALMAIN)
    {
    	//printf("N: [%s: thread_process] ADE2CAM CHANNEL: VISUALMAIN \n", "Udp Media Client");
		if(0)
		{
			static FILE* fp = fopen("/home/pi/output_MSrv.h264", "wb");
			if(fp)
			{
				fwrite(pBuf, 1, size, fp);
			}
			//return;
		}
    	axc_i32 iSend = sendto(m_iUDPSocketHandle, (char*) pBuf, size, 0,
    			(const struct sockaddr*) &m_sockaddr_in_H264DataAddress, sizeof(struct sockaddr_in));
        if (iSend < 0){
        	GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tNORMALTrace_MSK, "N: [%s: thread_process] send fail - len:%d\n", "Udp Media Client", iSend);
            CHeatFinderUtility::MsgSendtoStrerror(errno);
        }else if (iSend == 0){
        	//pass
        }

    }
    else
    {
        //something wrong?
    	GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s: thread_process] unknown channel index: %llu\n", "Udp Media Client", channelIndex);
    }
}


axc_dword CHeatFinderUdpMediaClient::thread_process()
{
	GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s: thread_process] enter \n", "Udp Media Client");

    void *pContext = CHeatFinderUtility::GetGlobalsupport();
    GlobalDef *pGlobal = reinterpret_cast<GlobalDef*> (pContext);
    pGlobal->AddpThreadRecord("UDP Sender: Vision");

	Cheatfinderconfigmanager *pConfigManager = (Cheatfinderconfigmanager *)CHeatFinderUtility::GetConfigObj();
	char cValue[32];
	memset(cValue, 0, sizeof(cValue));
	int iValue =0;
	int iRst = -1;

    while(!m_bStopThread)
    {
    	//Error in `./adehf_mobile': double free or corruption (out): 0x7172b898
        if ((m_SendBufList.size() <= 0) || (m_SendBufList.empty())){

        	m_hThread.SleepMilliSeconds(10);
        	continue;
        }else{
			//printf("N: [%s: thread_process] has data to send\n", "Udp Media Client");


			m_locker.lock();
			//ref:http://www.cplusplus.com/reference/vector/vector/pop_back/
			//ref:https://stackoverflow.com/questions/40656871/remove-first-item-of-vector
			//Data Struct FIFO Queue: push_back() -> take front() & .erase(queue.begin())

			//printf("N: [%s: thread_process] %lf has data to send\n", "Udp Media Client", CAxcTime::GetCurrentUTCTimeMs());

			xSendBuf lxBuf;
			xSendBuf *pSendBuf = m_SendBufList.front();
			lxBuf.CopyData(pSendBuf->pBuf, pSendBuf->iSize, pSendBuf->ddwChannelIndex);
			m_SendBufList.erase(m_SendBufList.begin());
			delete pSendBuf;
			m_locker.unlock();

			if (lxBuf.ddwChannelIndex == ADE2CAM_CHANNEL_VISUALMAIN)
			{
#if true
                const uint32_t iBufferLength = 16000;
                unsigned char iReadBuffer[iBufferLength] = {0};
				uint32_t iReadedLength = 0;

				while(iReadedLength < (unsigned int)lxBuf.iSize){
					uint32_t iCopyLen = 0;
					uint32_t iUnReadedLen = lxBuf.iSize - iReadedLength;
					if (iUnReadedLen < iBufferLength){
						iCopyLen = iUnReadedLen;
					}else{
                        iCopyLen = iBufferLength;
					}

					memcpy(iReadBuffer, lxBuf.pBuf + iReadedLength, iCopyLen);
					//printf("N: [%s: thread_process] ready to send rpi-camera view %u/%u\n", "Udp Media Client", iCopyLen, lxBuf.iSize);

					iReadedLength += iCopyLen;

                    axc_i32 iSend = sendto(m_iUDPSocketHandle, iReadBuffer, iCopyLen, 0,
                            (const struct sockaddr*) &m_sockaddr_in_H264DataAddress, sizeof(struct sockaddr_in));
                    if (iSend <= 0){
                        GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s: thread_process] Queue(1/%u) length:%u vision \n", "Udp Media Client", m_SendBufList.size(), iCopyLen);
                        CHeatFinderUtility::MsgSendtoStrerror(errno);
                    }
				}
#else
				axc_i32 iSend = sendto(m_iUDPSocketHandle, lxBuf.pBuf, lxBuf.iSize, 0,
						(const struct sockaddr*) &m_sockaddr_in_H264DataAddress, sizeof(struct sockaddr_in));
				if (iSend <= 0){
                    GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s: thread_process] Queue(1/%u) length:%u vision \n", "Udp Media Client", m_SendBufList.size(), lxBuf.iSize);
					CHeatFinderUtility::MsgSendtoStrerror(errno);
				}
#endif
				//CHeatFinderUtility::SetTwinkleLedShowNetworkUsed();
			}
			else
			{
				//something wrong?
				GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s: thread_process] unknown channel index: %llu\n", "Udp Media Client", lxBuf.ddwChannelIndex);
			}
			//printf("N: [%s: thread_process] %lf lease data been sent.\n", "Udp Media Client", CAxcTime::GetCurrentUTCTimeMs());

        }

    }
	GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s: thread_process] leave \n", "Udp Media Client");
    return 1;
}


axc_dword CHeatFinderUdpMediaClient::thread_process_thermal()
{
    //DWORD dwMediaSrv = inet_addr(serverIP);
	GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s: thread_process_thermal] enter \n", "Udp Media Client");

    void *pContext = CHeatFinderUtility::GetGlobalsupport();
    GlobalDef *pGlobal = reinterpret_cast<GlobalDef*> (pContext);
    pGlobal->AddpThreadRecord("UDP Sender: Thermal");

	//if(!m_queueNetDataForSend.Create(m_iQueuePacketSize, m_max_SendBuffer_size)){
	//	printf("E: [%s: thread_process_thermal] create virtual queue fail\n", "Udp Media Client");
	//	return 0;
	//}

    while(!m_bStopThread)
    {
    	//Error in `./adehf_mobile': double free or corruption (out): 0x7172b898
        if ((m_SendBufListThermal.size() <= 0) || (m_SendBufListThermal.empty())){
            //usleep(20 * 1000);
        	m_hThread_thermal.SleepMilliSeconds(20);
        	continue;
        }
    	//printf("N: [%s: thread_process_thermal] has data to send\n", "Udp Media Client");


        m_thermal_locker.lock();
        //ref:http://www.cplusplus.com/reference/vector/vector/pop_back/
        //ref:https://stackoverflow.com/questions/40656871/remove-first-item-of-vector
        //Data Struct FIFO Queue: push_back() -> take front() & .erase(queue.begin())

        //printf("N: [%s: thread_process_thermal] %lf has data to send\n", "Udp Media Client", CAxcTime::GetCurrentUTCTimeMs());

        xSendBuf lxBuf;
        xSendBuf *pSendBuf = m_SendBufListThermal.front();
        lxBuf.CopyData(pSendBuf->pBuf, pSendBuf->iSize, pSendBuf->ddwChannelIndex); //, 0, false);
        m_SendBufListThermal.erase(m_SendBufListThermal.begin());
        delete pSendBuf;

    	//printf("N: [%s: thread_process_thermal] drop finish\n", "Udp Media Client");
        m_thermal_locker.unlock();

        if (lxBuf.ddwChannelIndex == ADE2CAM_CHANNEL_LEPTONRAW)
        {
#if false
            const uint32_t iBufferLength = 16*1024;
            unsigned char iReadBuffer[iBufferLength] = {0};
        	uint32_t iReadedLength = 0;

        	while(iReadedLength < (unsigned int)lxBuf.iSize){
        		uint32_t iCopyLen = 0;
        		uint32_t iUnReadedLen = lxBuf.iSize - iReadedLength;
        		if (iUnReadedLen < iBufferLength){
        			iCopyLen = iUnReadedLen;
        		}else{
        			iCopyLen = iBufferLength* sizeof(unsigned char);
        		}

        		memcpy(iReadBuffer, lxBuf.pBuf + iReadedLength, iCopyLen);
        		printf("N: [%s: thread_process_thermal] ready to send lepton view %u/%u\n", "Udp Media Client", iCopyLen, lxBuf.iSize);

        		iReadedLength += iCopyLen;

        		axc_i32 iSend = sendto(m_iUDPSocketHandle, (axc_byte *) &iReadBuffer, iLen, 0,
						(const struct sockaddr*) &m_sockaddr_in_ThermalDataAddress, sizeof(struct sockaddr_in));
				if (iSend <= 0){
					printf("W: [%s: thread_process] Queue(1/%u) length:%u thermal \n", "Udp Media Client", m_SendBufList.size(), iCopyLen);
					CHeatFinderUtility::MsgSendtoStrerror(errno);
				}

        		memset(iReadBuffer, 0, iBufferLength* sizeof(unsigned char));
        	}
        	//printf("N: [%s: thread_process_thermal] send done\n", "Udp Media Client");
#else
        	axc_i32 iSend = sendto(m_iUDPSocketHandle, lxBuf.pBuf, lxBuf.iSize, 0,
        			(const struct sockaddr*) &m_sockaddr_in_ThermalDataAddress, sizeof(struct sockaddr_in));
            if (iSend <= 0){
            	GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s: thread_process_thermal] Queue(1/%u) length:%u thermal \n", "Udp Media Client", m_SendBufList.size(), lxBuf.iSize);
                CHeatFinderUtility::MsgSendtoStrerror(errno);
            }
#endif
            //CHeatFinderUtility::SetTwinkleLedShowNetworkUsed();
        }
        else
        {
            //something wrong?
        	GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s: thread_process_thermal] unknown channel index: %llu\n", "Udp Media Client", lxBuf.ddwChannelIndex);
        }

    }
	GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s: thread_process_thermal] leave \n", "Udp Media Client");
    return 1;
}
