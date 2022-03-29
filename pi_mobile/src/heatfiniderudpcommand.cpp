#include "heatfiniderudpcommand.h"
//#include "heatfinderapp.h"

CHeatFiniderUdpCommand::CHeatFiniderUdpCommand():
    m_hThread("net/udp_thread_command"),
    m_hThread_Response("net/udp_thread_response"),
    m_sockUdpCommand("net_socket/udp_command"),
    m_bStopThread(true),
	m_bForceStop_thread_process(false),
	m_dbUdpCmdRecved(0.0),
	m_dbUdpCmdRecved_Last(0.0),
	m_dbTheSameStart(0.0),
	m_bneedCheckUDPRecv(false),
	m_nReOpenUdpCount(0)
{
	//ref.: http://www.voidcn.com/article/p-yatpbttw-rn.html
	m_bIsprintDebugMessage = false;
}

CHeatFiniderUdpCommand::~CHeatFiniderUdpCommand()
{
    m_bStopThread = true;
    //printf("D: [%s] finish step 1\n", "UDP Command Receiver");
    if (m_hThread.IsValid())
        m_hThread.Destroy(2000, axc_true);

    //printf("D: [%s] finish step 2\n", "UDP Command Receiver");
    if (m_hThread_Response.IsValid())
    	m_hThread_Response.Destroy(2000);

    //printf("D: [%s] finish step 3\n", "UDP Command Receiver");
    m_sockUdpCommand.Destroy();
    GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] Destroy \n", "UDP Command Receiver");
}

bool CHeatFiniderUdpCommand::Run()
{
    bool bRst = false;
    Stop();

    if(!m_sockUdpCommand.Create(PORT_UDP_COMMANDSRV, 0, 64*1024, 64*1024)){
    	//CHeatFinderUtility::PrintNowData(1);
    	GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] UDP port %d, socket receiver create fail\n", "UDP Command Receiver", PORT_UDP_COMMANDSRV );
        return bRst;
    } else {
        GLog( tAll, tDEBUGTrace_MSK, "D: [UDP Command Receiver] ================> UDP port %d listen created", PORT_UDP_COMMANDSRV );
    }

    m_bStopThread = false;
    if (!m_hThread.Create(thread_process, this)){
    	//CHeatFinderUtility::PrintNowData(1);
    	GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] thread receive create fail \n", "UDP Command Receiver");
        return bRst;
    }


    if (!m_hThread_Response.Create(thread_process_response, this)){
    	GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] thread response create fail \n", "UDP Command Receiver");
        return bRst;
    }

    bRst = true;
    return bRst;
}

void CHeatFiniderUdpCommand::Stop()
{
    m_bStopThread = true;
    //printf("D: [%s] finish step 1\n", "UDP Command Receiver");
    if (m_hThread.IsValid())
        m_hThread.Destroy(2000, axc_true);

    //printf("D: [%s] finish step 2\n", "UDP Command Receiver");
    if (m_hThread_Response.IsValid())
    	m_hThread_Response.Destroy(2000);

    GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] Stop\n", "UDP Command Receiver");
}

bool CHeatFiniderUdpCommand::SetBroadcastInformationBackoff(CAxcString* caxcStrObj){

	if (caxcStrObj->GetLength() < 1){
		GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] new broadcast string small than 1 double word (%d)\n", "UDP Command Receiver", caxcStrObj->GetLength());
		return false;
	}
	m_locker.lock();
	m_caxcStrBroadcastRecord.Set(NULL);
	m_caxcStrBroadcastRecord.Set(caxcStrObj->GetBuffer(), caxcStrObj->GetLength());
	m_caxcStrBroadcastRecord.Append("\r\n\0");
	m_doubleBroadcastRenewTimestamp = CAxcTime::GetCurrentUTCTimeMs();
	m_locker.unlock();

	return true;
}

void CHeatFiniderUdpCommand::AddUdpReceivedEvent(tagUdpReceivedCommandEvent inputevent, void *pContext){

	xUdpReceivedCommandHandler newRecEvent;
	newRecEvent.objEvent.flag = inputevent.flag;
	newRecEvent.objEvent.doubleRecTime = inputevent.doubleRecTime;
	newRecEvent.objEvent.dwFromIp = inputevent.dwFromIp;
	newRecEvent.objEvent.wFromPort = inputevent.wFromPort;
	newRecEvent.pContext = pContext;

	//char szIp[64] = {""};
	//CAxcSocket::IpToString(newRecEvent.objEvent.dwFromIp, szIp);
	//printf("D: [%s] Add udp received command evnet: ip:%s port:%u\n", "UDP Command Receiver", szIp, newRecEvent.objEvent.wFromPort);
	//m_rec_locker.lock();
	while (m_rec_locker.try_lock() == false){
		m_hThread.SleepMilliSeconds(20);
	}
	m_UdpReceiveEventList.push_back(newRecEvent);
	m_rec_locker.unlock();
}

void CHeatFiniderUdpCommand::AddBroadcastRenewNotifyEvent(OnUdpBroadcastRenewNotifyEvent fnEvent, void *pContext){

	xUdpBroadcastRenewNotifyEventHandler xHanlder;
	xHanlder.fnEvent = fnEvent;
	xHanlder.pContext = pContext;
	m_UdpBroadcastRenewNotifyEventList.push_back(xHanlder);
}

void CHeatFiniderUdpCommand::OnUdpResponseRecievedEvent(){
	m_rec_locker.lock();


	//printf("D: [UDP Command Receiver] response/send package...\n");

	unsigned int last_ops = 0;
	unsigned int l_flag = 99;
	unsigned int l_event_size = m_UdpReceiveEventList.size();
    for(unsigned int i = 0; i < l_event_size; ++i){
    	last_ops++;
    	l_flag = m_UdpReceiveEventList[i].objEvent.flag;

    	if (l_flag == FLAG_UDPRECEIVE_PROBE){
    		char szIp[64] = {""};
    		CAxcSocket::IpToString(m_UdpReceiveEventList[i].objEvent.dwFromIp, szIp);
    		//printf("D: [%s] udp response notify [probe]: ip:%s port:%u\n", "UDP Command Receiver", szIp, m_UdpReceiveEventList[i].objEvent.wFromPort);

    		//while (m_UdpReceiveEventList[i].objEvent.doubleRecTime >= m_doubleBroadcastRenewTimestamp){
    			// message is not newer than requisition been created.
    		//	m_hThread.SleepMilliSeconds(50);
    			// do it again
    			//AddUdpReceivedEvent(m_UdpReceiveEventList[i].objEvent, this);

    			//use the sample rule
    			//m_UdpReceiveEventList.insert(m_UdpReceiveEventList.end(), m_UdpReceiveEventList[i]);
    			//m_rec_locker.unlock();
    		//	continue;
    		//}
			//if (m_locker.try_lock() == true){
			m_locker.lock();
			//printf("D: [UDP Command Response] we got this messgae len: %d\n", m_caxcStrBroadcastRecord.GetLength());
			//printf("send broadcast info.: %s\n", m_caxcStrBroadcastRecord.Get());

			//int iLeft = m_caxcStrBroadcastRecord.GetLength() - 500;
			//m_sockUdpCommand.Send(m_caxcStrBroadcastRecord.GetBuffer(), 500, m_UdpReceiveEventList[i].objEvent.dwFromIp, m_UdpReceiveEventList[i].objEvent.wFromPort);
			//m_sockUdpCommand.Send(m_caxcStrBroadcastRecord.GetBuffer()+500, iLeft, m_UdpReceiveEventList[i].objEvent.dwFromIp, m_UdpReceiveEventList[i].objEvent.wFromPort);
			int sentCount = m_sockUdpCommand.Send(m_caxcStrBroadcastRecord.GetBuffer(), m_caxcStrBroadcastRecord.GetLength(), m_UdpReceiveEventList[i].objEvent.dwFromIp, m_UdpReceiveEventList[i].objEvent.wFromPort);
			GLog( 0, tDEBUGTrace_MSK, "D: [UDP Command Receiver] send broadcast data ...(%d bytes)\n", sentCount );
			m_locker.unlock();

    	}
    	else if (l_flag == FLAG_UDPRECEIVE_THERMAL_IVS){
    		//char szIp[64] = {""};
    		//CAxcSocket::IpToString(m_UdpReceiveEventList[i].objEvent.dwFromIp, szIp);
    		//printf("D: [%s] udp response notify [thermal ivs]: ip:%s port:%u\n", "UDP Command Receiver", szIp, m_UdpReceiveEventList[i].objEvent.wFromPort);

    		CAxcString cszSend;
			void *pContext = CHeatFinderUtility::GetGlobalsupport();
			GlobalDef *pApp = reinterpret_cast<GlobalDef*> (pContext);
			pApp->OnCollectThermalMsgToString(cszSend);
			m_sockUdpCommand.Send(cszSend.GetBuffer(), cszSend.GetLength()+1, m_UdpReceiveEventList[i].objEvent.dwFromIp, m_UdpReceiveEventList[i].objEvent.wFromPort);
    	}else{
    		GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] unknown udp received event (%d)\n", "UDP Command Receiver", l_flag);
    	}

    }
    m_UdpReceiveEventList.erase(m_UdpReceiveEventList.begin()+0, m_UdpReceiveEventList.begin()+last_ops);


    //printf("D: [UDP Command Receiver] response end\n");
	m_rec_locker.unlock();
}

bool CHeatFiniderUdpCommand::CheckUDPRecv_Available() {
	if(!m_bneedCheckUDPRecv) {
		return true;
	}
	if(m_dbUdpCmdRecved > 0.0) {
		if(m_dbUdpCmdRecved_Last != m_dbUdpCmdRecved) {
			m_dbUdpCmdRecved_Last = m_dbUdpCmdRecved;
			m_dbTheSameStart = 0;
		}
		else {
			double now = CAxcTime::GetCurrentUTCTimeMs()*1000;
			if(m_dbTheSameStart == 0.0) {
				m_dbTheSameStart = now;
			}
			else {
				if(now - m_dbTheSameStart > 60000.0) {
					m_dbTheSameStart = 0.0;
					return false;
				} else {
					GLog( 0, tDEBUGTrace_MSK, "UDP: now - m_dbTheSameStart => %.0f\n", (now - m_dbTheSameStart));
				}
			}
		}
	}
	return true;
}

bool CHeatFiniderUdpCommand::ReOpenUDPPort() {
	m_dbUdpCmdRecved = 0.0;
	m_dbUdpCmdRecved_Last = 0.0;
	m_dbTheSameStart = 0.0;
	m_bneedCheckUDPRecv = false;

	m_sockUdpCommand.Destroy();
	m_bForceStop_thread_process = true;
	bool retVal = false;
    if (m_hThread.IsValid())
        m_hThread.Destroy(2000, axc_true);

    if(!m_sockUdpCommand.Create(PORT_UDP_COMMANDSRV, 0, 64*1024, 64*1024)){
    	//CHeatFinderUtility::PrintNowData(1);
    	GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] UDP port %d, socket receiver create fail\n", "UDP Command Receiver", PORT_UDP_COMMANDSRV );

    	retVal = false;
    } else {
        //GLog( tAll, tDEBUGTrace_MSK, "D: [UDP Command Receiver] ================> UDP port %d listen created (Recover)", PORT_UDP_COMMANDSRV );
        m_hThread_Response.SleepMilliSeconds(1000);
        retVal = true;
    }
	m_bForceStop_thread_process = false;
    if (!m_hThread.Create(thread_process, this)){
    	//CHeatFinderUtility::PrintNowData(1);
    	GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] thread receive create fail \n", "UDP Command Receiver");

    	retVal = false;
    }

    return retVal;
}

axc_dword CHeatFiniderUdpCommand::thread_process()
{
	GLog( 0, tDEBUGTrace_MSK, "++++++++++++++++++++++ UDP listen start ++++++++++++++++++++++\n");

    char RecvBuffer[1600];
    //char SendBuffer[1600];
    int iRecvLen = 0;
    axc_dword dwFromIp = 0;
    axc_word wFromPort = 0;
    char szValue[256];
    bool	isShowBroadcast = false;

    void *pContext = CHeatFinderUtility::GetGlobalsupport();
    GlobalDef *pGlobal = reinterpret_cast<GlobalDef*> (pContext);
    pGlobal->AddpThreadRecord("udp command rx");
    m_bneedCheckUDPRecv = true;

    while(!m_bStopThread)
    {
        // block waitting
        //iRecvLen = m_sockUdpCommand.Recv(RecvBuffer, sizeof(RecvBuffer)-1, &dwFromIp, &wFromPort, TRUE);
        iRecvLen = m_sockUdpCommand.Recv(RecvBuffer, sizeof(RecvBuffer)-1, &dwFromIp, &wFromPort, FALSE);
        //if(iRecvLen <= 0){
        //    break;
        //}

        if(m_bForceStop_thread_process){
        	break;
        }
        if(iRecvLen <= 0){
        	m_hThread.SleepMilliSeconds(500);
        	continue;
        }
        else if (m_bIsprintDebugMessage == true){
        	GLog(tAll, tDEBUGTrace_MSK, "D: [UDP Command Receiver] got udp pkg len:%d\nUDP: %s\n\n", iRecvLen, RecvBuffer);
        }
        m_dbUdpCmdRecved = CAxcTime::GetCurrentUTCTimeMs()*1000;

        if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/tmp/show_broadcast.lock")))
            isShowBroadcast = true;
        else
            isShowBroadcast = false;

        // process command packet
        RecvBuffer[iRecvLen] = 0;

        if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(RecvBuffer, (axc_dword)iRecvLen, "command", szValue, (axc_dword)sizeof(szValue)) > 0)
        {
            if(0 == CAxcString::strcmp(szValue,"get_thermal_ivs_result",axc_true))
            {
            	//printf("D: [UDP Command Receiver] get_thermal_ivs_result\n");
            	/**
                CAxcString cszSend;
                void *pContext = CHeatFinderUtility::GetGlobalsupport();
                GlobalDef *pApp = reinterpret_cast<GlobalDef*> (pContext);
                pApp->OnCollectThermalMsgToString(cszSend);
                m_sockUdpCommand.Send(cszSend.GetBuffer(), cszSend.GetLength()+1, dwFromIp, wFromPort);
                **/

            	struct tagUdpReceivedCommandEvent newRecObjEvent;
            	memset(&newRecObjEvent, 0, sizeof(newRecObjEvent));
            	newRecObjEvent.flag = FLAG_UDPRECEIVE_THERMAL_IVS;
            	newRecObjEvent.doubleRecTime =CAxcTime::GetCurrentUTCTimeMs();
            	newRecObjEvent.dwFromIp = dwFromIp;
            	newRecObjEvent.wFromPort = wFromPort;

            	AddUdpReceivedEvent(newRecObjEvent, this);
            }
            else if(0 == CAxcString::strcmp(szValue,"probe",axc_true))
            {
            	GLog( 0, tDEBUGTrace_MSK, "D: [UDP Command Receiver] udp command received 'probe'\n");
                if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/tmp/broadcast.lock")))
                {
                    if (isShowBroadcast == true)
                    {
                        GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tNORMALTrace_MSK, "N: [%s] %s blocking send broadcast message\n", "UDP Command Receiver", __func__);
                    }
                }
                else
                {
                	fireNeedtoRenewBroadcastRecordEvent();

                	/**
                	m_locker.lock();
                    if (isShowBroadcast == true)
                    {
                        printf("N: [%s] %s send broadcast message to %lu: %u\n", "UDP Command Receiver", __func__, dwFromIp, wFromPort);
                        if (m_bIsprintDebugMessage == true){
                        	printf("N: %s \n", GetBroadcastInfo()->Get());
                        }
                    }

                    m_sockUdpCommand.Send(GetBroadcastInfo()->GetBuffer(), GetBroadcastInfo()->GetLength()+1, dwFromIp, wFromPort);
                    m_locker.unlock();
                    **/
                	struct tagUdpReceivedCommandEvent newRecObjEvent;
                	memset(&newRecObjEvent, 0, sizeof(newRecObjEvent));
                	newRecObjEvent.flag = FLAG_UDPRECEIVE_PROBE;
                	newRecObjEvent.doubleRecTime =CAxcTime::GetCurrentUTCTimeMs();
                	newRecObjEvent.dwFromIp = dwFromIp;
                	newRecObjEvent.wFromPort = wFromPort;

                	AddUdpReceivedEvent(newRecObjEvent, this);
                }

            }
        }else if(0 == CAxcString::strcmp(szValue,"terminate",axc_true)){
        	GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [UDP Command Receiver] terminate\n");

        	m_bStopThread = true;
        }else if (m_bIsprintDebugMessage == true){
        	GLog(tNetworkUDPTrace(LogManager::sm_glogDebugZoneSeed), tNORMALTrace_MSK, "N: [UDP Command Receiver] got udp pkg :%s\n", RecvBuffer);
        }
        m_hThread.SleepMilliSeconds(200);
    }
    m_bneedCheckUDPRecv = false;
    return 1;
}

axc_dword CHeatFiniderUdpCommand::thread_process_response()
{

    void *pContext = CHeatFinderUtility::GetGlobalsupport();
    GlobalDef *pGlobal = reinterpret_cast<GlobalDef*> (pContext);
    pGlobal->AddpThreadRecord("udp command tx");

    while(!m_bStopThread)
    {
    	if(!CheckUDPRecv_Available()){
    		if(!ReOpenUDPPort()) {
    			GLog( tAll, tERRORTrace_MSK, "+-----------------------------------------------------------" );
    			GLog( tAll, tERRORTrace_MSK, "| E: [UDP Command Receiver] ReOpenUDPPort Failed!!!" );
    			GLog( tAll, tERRORTrace_MSK, "+-----------------------------------------------------------" );
    			system("sudo killall -s SIGUSR2 ade_gpio_set");
    		} else {
    			m_nReOpenUdpCount++;
    			if( m_nReOpenUdpCount > 100 ) {
    				m_nReOpenUdpCount = 0;
    				GLog( tAll, tERRORTrace_MSK, "+-----------------------------------------------------------------" );
    				GLog( tAll, tERRORTrace_MSK, "| E: [UDP Command Receiver] ReOpenUDPPort more than 100 times!!!" );
    				GLog( tAll, tERRORTrace_MSK, "+-----------------------------------------------------------------" );
    				//system("sudo killall -s SIGUSR2 ade_gpio_set");
    			}
    		}
    	}
    	if(m_UdpReceiveEventList.size()<=0){
    		// empty
    		m_hThread_Response.SleepMilliSeconds(200);
    		continue;
    	}

    	if(axc_false == m_sockUdpCommand.IsValid()){
    		// not ready
    		m_hThread_Response.SleepMilliSeconds(200);
    		continue;
    	}

		// send the message obeyed by udp command we receive
		OnUdpResponseRecievedEvent();


    }
	return 1;
}

void CHeatFiniderUdpCommand::fireNeedtoRenewBroadcastRecordEvent(){

    if (m_UdpBroadcastRenewNotifyEventList.size() <=0)
        return;

    for(unsigned int i = 0; i < m_UdpBroadcastRenewNotifyEventList.size(); ++i)
    {
    	xUdpBroadcastRenewNotifyEventHandler handler = m_UdpBroadcastRenewNotifyEventList[i];
        handler.fnEvent(handler.pContext);
    }
}
