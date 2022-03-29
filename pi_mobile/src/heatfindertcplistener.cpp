#include "heatfindertcplistener.h"
#include "ade_camera_sdk.h"
#include "heatfindertimer.h"
#include "adeSDKAPIleptonthermal.h"
#include "heatfinderconfigmanager.h"

CHeatFinderTcpListener::CHeatFinderTcpListener():
    m_hThread("net/tcp_listen"),
    m_bStopThread(true),
    m_sockTcpListen("net/tcp_listen"),
    m_listTcpSession("net/tcp_session"),
    m_hSenderThread("net/tcp_sendcb")
{
	m_bPrintHideDebugMessage = false;
}

CHeatFinderTcpListener::~CHeatFinderTcpListener()
{
    m_bStopThread = true;
    if (m_hThread.IsValid())
        m_hThread.Destroy(2000);
    if (m_hSenderThread.IsValid())
    	m_hSenderThread.Destroy(2000);

    m_sockTcpListen.Destroy();

	GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] TCP-session: kill all session\n", "TCP Listener");
	const axc_i32 iTcpSessionCount = m_listTcpSession.GetCount();
	for(axc_i32 i=iTcpSessionCount-1; i>=0; i--)
	{
		CAxcSocketTcpSession* pSession = (CAxcSocketTcpSession*) m_listTcpSession.GetID(i);
		m_listTcpSession.Remove(i);
		if(pSession)
		{
			pSession->Destroy();
			delete pSession;
			pSession = NULL;
		}
	}
	m_listTcpSession.Destroy();

	GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] Destroy \n", "TCP Listener");
}

unsigned int CHeatFinderTcpListener::Run()
{
    Stop();

    m_bStopThread = false;

    if(axc_false == m_sockTcpListen.Create(PORT_TCP_LISTEN, 0))
    {
    	GLog(tAll, tDEBUGTrace_MSK, "D: [Gavin]  ---CHeatFinderTcpListener::Run ...OPENPORT\n");
        return OPENPORT;
    }

    if(axc_false == m_sockTcpListen.Listen())
    {
    	GLog(tAll, tDEBUGTrace_MSK, "D: [Gavin]  ---CHeatFinderTcpListener::Run ...OPENPORT\n");
        return STARTLISTEN;
    }

    if(axc_false == m_listTcpSession.Create(32))  //MAX_TCP_CONNECT_NUMBER =32, 64
    {
    	GLog(tAll, tDEBUGTrace_MSK, "D: [Gavin]  ---CHeatFinderTcpListener::Run ...SESSIONLISTCREATE\n");
    	return SESSIONLISTCREATE;
    }

    if (!m_hThread.Create(thread_process, this))
    {
    	GLog(tAll, tDEBUGTrace_MSK, "D: [Gavin]  ---CHeatFinderTcpListener::Run ...PTHREAD1\n");
       return PTHREAD1;
    }

    if (!m_hSenderThread.Create(thread_sendpkg_process, this))
    {
    	GLog(tAll, tDEBUGTrace_MSK, "D: [Gavin]  ---CHeatFinderTcpListener::Run ...PTHREAD2\n");
    	return PTHREAD2;
    }

    if (!m_tcpeventnotifyMonitor.Run()){
    	GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] Monitor start fail\n", "TCP Listener");
    }
    CHeatFinderTimer *pTimer = (CHeatFinderTimer *) CHeatFinderUtility::GetTimerService();
    pTimer->AddCheckFunctionOnTimeNotifyEvent(OnFuncChkTCPTimeEventNotify, 1.5, "TCP Lis.Time Event per 1 sec.", this);

    return NOERROR;
}

void CHeatFinderTcpListener::Stop()
{
    m_bStopThread = true;
    if (m_hThread.IsValid())
        m_hThread.Destroy(2000);

    if (m_hSenderThread.IsValid())
    	m_hSenderThread.Destroy(2000);
	GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] Stop\n", "TCP Listener");

}

void CHeatFinderTcpListener::SetPrintNewTcpSessionAccepted(const bool value)
{
	m_bPrintHideDebugMessage = value;
}

void CHeatFinderTcpListener::AddTcpAcceptSessionEvent(OnTcpAcceptSessionEvent fnEvent, void *pContext)
{
    m_locker.lock();
    /**if (false == m_locker.try_lock()){
    	return;
    }**/

    xTcpAcceptSessionEventHandler xHanlder;
    xHanlder.fnEvent = fnEvent;
    xHanlder.pContext = pContext;
    m_TcpAcceptSessionEventList.push_back(xHanlder);
    m_locker.unlock();
}

void CHeatFinderTcpListener::AddTcpNeedtoRemoveSessionEvent(OnTcpNeedtoRemoveSessionEvent fnEvent, void *pContext){
    m_locker.lock();
    xTcpNeedtoRemoveSessionEventHandler xHanlder;
    xHanlder.fnEvent = fnEvent;
    xHanlder.pContext = pContext;
    m_TcpNeedtoRemoveSessionEventList.push_back(xHanlder);
    m_locker.unlock();
}

void CHeatFinderTcpListener::AddBroadcastRenewNotifyEvent(OnTcpBroadcastRenewNotifyEvent fnEvent, void *pContext){

	xTcpBroadcastRenewNotifyEventHandler xHanlder;
	xHanlder.fnEvent = fnEvent;
	xHanlder.pContext = pContext;
	m_TcpBroadcastRenewNotifyEventList.push_back(xHanlder);
}

bool CHeatFinderTcpListener::SetBroadcastInformationBackoff(CAxcString* caxcStrObj){

	if (caxcStrObj->GetLength() < 1){
		GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] new broadcast string small than 1 double word (%d)\n", "TCP Listener", caxcStrObj->GetLength());
		return false;
	}
	m_locker_broadcast_renew_notify.lock();
	m_caxcStrBroadcastRecord.Set(caxcStrObj->GetBuffer(), caxcStrObj->GetBufferSize()+1);
	m_doubleBroadcastRenewTimestamp = CAxcTime::GetCurrentUTCTimeMs();
	m_locker_broadcast_renew_notify.unlock();

	return true;
}


void CHeatFinderTcpListener::AddSessionSendtoEvent(OnSessionSendtofunc fnEvent, void *pContext){
	xOnSessionSendtoEventHandler xHandler;
	xHandler.fnEvent = fnEvent;
	xHandler.pContext = pContext;
	m_TcpSessionSendtoList.push_back(xHandler);
}

void CHeatFinderTcpListener::AddTcpReceivedUpgreadeEventNotify(OnReceivedUpgradeEvent fnEvent, void *pContext){
	xOnReceivedUpgradeEventHandler xHandler;
	xHandler.fnEvent = fnEvent;
	xHandler.pContext = pContext;
	m_TcpReceivedUpgradeEventList.push_back(xHandler);
}

int CHeatFinderTcpListener::OnSessionSendto(CAxcList *listSessionSend, CAxcList *listSessionDrop){
	if (m_TcpSessionSendtoList.size() <=0)
		return -1;

	int iFRst =0;
    for(unsigned int i = 0; i < m_TcpSessionSendtoList.size(); ++i)
    {
    	int iRst =0;
    	xOnSessionSendtoEventHandler handler = m_TcpSessionSendtoList[i];
    	iRst =handler.fnEvent(handler.pContext, listSessionSend, listSessionDrop);
    	if ((iRst == -1)&&(iFRst != -2)){
    		iFRst = -1;
    	}else if (iRst == -2){
    		iFRst = -2;
    	}
    }
    return iFRst;
}

void CHeatFinderTcpListener::fireTcpAcceptSessionEvent(CAxcSocketTcpSession *pValue, const bool bValue)
{
    if (m_TcpAcceptSessionEventList.size() <=0)
        return;

    unsigned int luValue = 0;
    if (bValue == ThisCommandIsVideoOut){
		void* pContext = m_listTcpSession.GetItem((axc_ddword)pValue);
		axc_ddword ddwContext = (axc_ddword) pContext;

		luValue += (ddwContext & TCP_SESSION_CONTEXT_OPEN_THERMAL)? 0x01: 0;
		luValue += (ddwContext & TCP_SESSION_CONTEXT_OPEN_VISION)? 0x02: 0;
    }else{
    	luValue =0;
    }

    for(unsigned int i = 0; i < m_TcpAcceptSessionEventList.size(); ++i)
    {
        xTcpAcceptSessionEventHandler handler = m_TcpAcceptSessionEventList[i];
        handler.fnEvent(handler.pContext, pValue, luValue);
    }
}

void CHeatFinderTcpListener::fireNeedtoRemoveSessionEvent(CAxcSocketTcpSession *pValue){
    if (m_TcpNeedtoRemoveSessionEventList.size() <=0)
        return;

    for(unsigned int i = 0; i < m_TcpNeedtoRemoveSessionEventList.size(); ++i)
    {
    	xTcpNeedtoRemoveSessionEventHandler handler = m_TcpNeedtoRemoveSessionEventList[i];
        handler.fnEvent(handler.pContext, pValue);
    }
}

void CHeatFinderTcpListener::fireNeedtoRenewBroadcastRecordEvent(){

    if (m_TcpBroadcastRenewNotifyEventList.size() <=0)
        return;

    for(unsigned int i = 0; i < m_TcpBroadcastRenewNotifyEventList.size(); ++i)
    {
    	xTcpBroadcastRenewNotifyEventHandler handler = m_TcpBroadcastRenewNotifyEventList[i];
        handler.fnEvent(handler.pContext);
    }
}

void CHeatFinderTcpListener::fireTcpReceivedUpgradeEventNotify(double dDelaytimestampMS){
    if (m_TcpReceivedUpgradeEventList.size() <=0)
        return;

    for(unsigned int i = 0; i < m_TcpReceivedUpgradeEventList.size(); ++i)
    {
    	xOnReceivedUpgradeEventHandler handler = m_TcpReceivedUpgradeEventList[i];
        handler.fnEvent(handler.pContext, dDelaytimestampMS);
    }
}

bool CHeatFinderTcpListener::OnRemoveSession(CAxcSocketTcpSession* pSession){
	bool iRst = false;

	if(m_listTcpSession.Remove((axc_ddword)pSession) >= 0)
	{
		char szIp[64] = {""};
		CAxcSocket::IpToString(pSession->GetRemoteIp(), szIp);
		if (m_bPrintHideDebugMessage == true){
			GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tNORMALTrace_MSK, "N: [%s] delete TCP-session: remote %s:%u closed, socket = %d, session_cnt = %d\n",
					"TCP Listener", szIp,
					pSession->GetRemotePort(), pSession->GetSocket(),
					m_listTcpSession.GetCount());
		}
		delete pSession;
		pSession = NULL;

		iRst = true;
	}

	return iRst;
}

bool CHeatFinderTcpListener::onGetBroadcastLocalRecord(CAxcString* pCAcxcStrObj, double* pdoubleTimestamp){
	if (m_caxcStrBroadcastRecord.GetLength()<1){
		return false;
	}

	m_locker_broadcast_renew_notify.lock();
	if (m_doubleBroadcastRenewTimestamp < *pdoubleTimestamp){
		// timestamp too old. we need new data.
		m_locker_broadcast_renew_notify.unlock();
		return false;
	}

	pCAcxcStrObj->AppendFormat("%s", m_caxcStrBroadcastRecord.Get());

	m_locker_broadcast_renew_notify.unlock();
	return true;
}

void CHeatFinderTcpListener::OnFuncChkTCPTimeEventNotify(const double dCurrentTimeMS, const unsigned int uiAction, const signed int siValue){
	//printf("D: [%s] %s\n","TCP Listener", __func__);

	m_tcpeventnotifyMonitor.OnChkAllEventAndTriggerNotify();
}

// thread process
axc_dword CHeatFinderTcpListener::thread_process()
{
	// use enum 'E_TCP_LISTERN_CREATE_ERROR' to return value
    void *pContext = CHeatFinderUtility::GetGlobalsupport();
    GlobalDef *pGlobal = reinterpret_cast<GlobalDef*> (pContext);
    pGlobal->AddpThreadRecord("Tcp Port Listener: TXRX");

    // do 分析，create 清單，send 丟包, end & wait next time
    while(!m_bStopThread && m_sockTcpListen.IsValid())
    {
        CAxcSocketTcpSession* pSession = m_sockTcpListen.Accept();
        if (NULL == pSession)
        {
            m_hThread.SleepMilliSeconds(100);
            continue;
        }

        char szIp[64] = {""};
        CAxcSocket::IpToString(pSession->GetRemoteIp(), szIp);

        bool bSucceed = false;
		if(m_listTcpSession.GetCount() >= m_listTcpSession.GetMaxItemCount())
		{
			GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] new TCP-session: failed, list full (number=%d) !\n", "TCP Listener", m_listTcpSession.GetCount());
			//drop this session.
			CAxcString cszSend;
			// resource is busy to do new command, to close session
			{
				cszSend.Append("command=reply\r\n");
				cszSend.AppendFormat("reply_from=%s\r\n", szIp);
				cszSend.Append("result_code=-1\r\n");
				cszSend.Append("result_text=resource is busy, dropping current command!\r\n");
				cszSend.Append("\r\n"); // END
			}
			// send to client
			pSession->Send(cszSend.GetBuffer(), cszSend.GetLength()+1);
		}
		else
		{
			const axc_ddword ddwId = (axc_ddword) pSession;
			const int iIndex = m_listTcpSession.Add(ddwId, 0);
			if(iIndex < 0)
			{
				const axc_i32 iCount = m_listTcpSession.GetCount();
				CAxcString cszLog;
				cszLog.Format("[TCP Listener] new TCP-session: failed add to list, num=%d, new=0x%llX(%p)", iCount, ddwId, pSession);
				for(axc_i32 i = 0; i<iCount; i++)
				{
					const axc_ddword ddwId2 = m_listTcpSession.GetID(i);
					const CAxcSocketTcpSession* pSession2 = (CAxcSocketTcpSession*) ddwId2;
					cszLog.AppendFormat(", [%d]=0x%llX(%p)", i, ddwId2, pSession2);
				}
				cszLog.Append(" !");
				GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] %s\n", "TCP Listener", cszLog.Get());
			}
			else
			{
				//
				// create thread to receive tcp-session data
				CAxcThread* pThread = NULL;
				try
				{
					pThread = new CAxcThread("net/tcp_session_recv");
				}
				catch(...)
				{
					pThread = NULL;
				}

				if(NULL == pThread)
				{
					GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] new TCP-session: failed to new tcp-session-thread !\n", "TCP Listener");
					m_listTcpSession.Remove((axc_ddword)pSession);
				}
				else{
					struct TcpListenNewSessionEventHandler objEventHandler;
					objEventHandler.pSender = this;
					objEventHandler.pSession = pSession;

					//printf("D: [%s] ready to create new thread for session \n", "TCP Listener");
					if(!pThread->Create(thread_tcpsession_delivery, &objEventHandler))
					{
						GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] new TCP-session: failed to create tcp-session-thread !\n", "TCP Listener");
						m_listTcpSession.Remove((axc_ddword)pSession);

						delete pThread;
						pThread = NULL;
					}
					else
					{
						bSucceed = true; // ok !
						if(m_bPrintHideDebugMessage == true){
							GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tNORMALTrace_MSK, "N: [%s] new TCP-session: remote %s:%u connected, socket = %d, session_cnt = %d\n",
									"TCP Listener",
									szIp,
									pSession->GetRemotePort(), pSession->GetSocket(),
									(int)m_listTcpSession.GetCount());
						}
						// timeout for send: 3 seconds
						CAxcSocket::SocketSetSendTimeout(pSession->GetSocket(), 3000);
					}
				}
			}
		}
		if(!bSucceed && pSession != NULL)
		{
			delete pSession;
			pSession = NULL;
		}
    }

    return NOERROR;
}

axc_dword CHeatFinderTcpListener::thread_tcpsession_delivery(CAxcThread* pThread, CAxcSocketTcpSession* pSession){
	// important:
	/**
	 * pSession of 'video-out' need to remove by tcp-client,
	 * because all send action of video-out obeyed by tcp-client.
	 * here (tcplisten) only service new tcp socket arrive and response command.
	 */
	bool bIsVideoCommand = false;

	CAxcMemory memRead("net/tcp_session_recv");
	if(memRead.Create(512*1024))
	{
		axc_ddword ddwLastPacketTimeSec = CAxcTime::GetCurrentUTCTime64();
		char* pszReadBuffer = (char*)memRead.GetAddress();
		axc_dword dwCacheLen = 0;

		while(!m_bStopThread && m_sockTcpListen.IsValid()){
			//xy, 2017-10-31, BEGIN
			if((dwCacheLen + 10*1024) >= memRead.GetBufferSize())
			{
				const axc_dword dwNewSize = memRead.GetBufferSize() + 10*1024;
				if(dwNewSize >= (100*1024*1024)){
					printf("D :[TCP Listener] %s: cannot re-alloc buffer any more, size = %u\n", __func__, dwNewSize);
					break;
				}
				else if(!memRead.Resize(dwNewSize, dwCacheLen+1))
				{
					printf("D :[TCP Listener] %s: failed to re-alloc buffer, size = %u, error = '%s'\n", __func__, dwNewSize, AxcGetLastErrorText());
					break;
				}
				//printf("D: [TCP Listener] %s: re-alloc buffer size = %u", __func__, dwNewSize);
				pszReadBuffer = (char*)memRead.GetAddress();
			}
			//xy, 2017-10-31, END

			//const axc_i32 iRecvLen = pSession->Recv(pszReadBuffer + dwCacheLen, memRead.GetBufferSize() - dwCacheLen - 1, 30);
			const axc_i32 iRecvLen = pSession->Recv(pszReadBuffer + dwCacheLen, memRead.GetBufferSize() - dwCacheLen - 1, 3); //idle session must died, ANY COST!
			if(iRecvLen == 0){
				if(!pSession->IsValid()){
					//fireTcpAcceptSessionEvent(pSession, false);
					pSession->Destroy();
					// talk to client this Session of pointer must be killed!!
					// for tcp-client double checking
					//fireNeedtoRemoveSessionEvent(pSession);
					break;
				}
				const axc_ddword ddwCurrTimeSec = CAxcTime::GetCurrentUTCTime64();
				if((ddwCurrTimeSec - ddwLastPacketTimeSec) >= 10){
					// is video-channel ?
					void* pContext = m_listTcpSession.GetItem((axc_ddword)pSession);
					axc_ddword ddwContext = (axc_ddword) pContext;
					if(ddwContext & (TCP_SESSION_CONTEXT_OPEN_THERMAL | TCP_SESSION_CONTEXT_OPEN_VISION)){
						ddwLastPacketTimeSec = ddwCurrTimeSec;

					}else{
						break;
					}
				}
			}else if(iRecvLen < 0){ //-1 == iRecvLen
				//please die A.S.A.P, ANY COST!
				const axc_ddword ddwCurrTimeSec = CAxcTime::GetCurrentUTCTime64();
				char szIp[64] = {""};
				CAxcSocket::IpToString(pSession->GetRemoteIp(), szIp);
				printf("W :[TCP Listener] %s: (%s:%d) request live view but already die. It idel %llu, Kill it.\n", __func__, szIp, pSession->GetRemotePort(), (ddwCurrTimeSec - ddwCurrTimeSec));
				if(!pSession->IsValid()){
					//pSession->Destroy(); //add remove action
					bIsVideoCommand = false;
				}
				//fireTcpAcceptSessionEvent(pSession, false);
				break;
			}else{
				ddwLastPacketTimeSec = CAxcTime::GetCurrentUTCTime64(); // time
				dwCacheLen += iRecvLen;
				pszReadBuffer[dwCacheLen] = 0;
				char* pszEnd = strstr(pszReadBuffer, "\r\n\r\n");


				if(NULL != pszEnd){
					pszEnd += 4;
					*pszEnd = 0; // END
					char szCommand[128] = {""};
					bool bCloseSession = axc_false;

					if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(pszReadBuffer, dwCacheLen, "command", szCommand, (axc_dword)sizeof(szCommand)) > 0)
					{
						if (true == m_locker_session_delivery.try_lock()){
							if(0 == CAxcString::strcmp(szCommand,"polling",axc_false)) //More rigorous
							{
								bCloseSession = ProcessTcpCommand_Polling(pszReadBuffer, dwCacheLen, pThread, pSession);
								bIsVideoCommand = (bCloseSession == axc_true)? false: true;

								//fireTcpAcceptSessionEvent(pSession, bIsVideoCommand);
							}
							else if(0 == CAxcString::strcmp(szCommand,"get_netconfig",axc_true))
							{
								bCloseSession = ProcessTcpCommand_GetNetConfig(pszReadBuffer, dwCacheLen, pThread, pSession);
							}
							//else if(0 == CAxcString::strncmp(szCommand,"probe",(axc_dword)sizeof(szCommand) ,axc_true))
							else if(0 == CAxcString::strcmp(szCommand,"probe",axc_true))
							{
								GLog( 0, tDEBUGTrace_MSK, "D: [TCP Listener] tcp command received 'probe'\n");
								bCloseSession = ProcessTcpCommand_Probe(pszReadBuffer, dwCacheLen, pThread, pSession);
							}
							else if(0 == CAxcString::strcmp(szCommand,"get_config",axc_true))
							{
								bCloseSession = ProcessTcpCommand_GetConfig(pszReadBuffer, dwCacheLen, pThread, pSession);
							}
							else if(0 == CAxcString::strncmp(szCommand,"get_thermal_ivs_config",(axc_dword)sizeof(szCommand) ,axc_true))
							{
								bCloseSession = ProcessTcpCommand_GetThermalIvsConfig(pszReadBuffer, dwCacheLen, pThread, pSession);
							}
							else if(0 == CAxcString::strncmp(szCommand,"get_thermal_ivs_result",(axc_dword)sizeof(szCommand) ,axc_true))
							{
								bCloseSession = ProcessTcpCommand_GetThermalIvsResult(pszReadBuffer, dwCacheLen, pThread, pSession);
							}
							else if(0 == CAxcString::strncmp(szCommand,"get_thermal_overlay_locate",(axc_dword)sizeof(szCommand) ,axc_true))
							{
								bCloseSession = ProcessTcpCommand_GetThermalOverlayLocate(pszReadBuffer, dwCacheLen, pThread, pSession);
							}
							else if(0 == CAxcString::strncmp(szCommand,"get_emissivity",(axc_dword)sizeof(szCommand) ,axc_true))
							{
								bCloseSession = ProcessTcpCommand_GetEmissivity(pszReadBuffer, dwCacheLen, pThread, pSession);
							}
							else if(0 == CAxcString::strncmp(szCommand,"set_emissivity",(axc_dword)sizeof(szCommand) ,axc_true))
							{
								bCloseSession = ProcessTcpCommand_SetEmissivity(pszReadBuffer, dwCacheLen, pThread, pSession);
							}
							else if(0 == CAxcString::strncmp(szCommand,"execute_ffc",(axc_dword)sizeof(szCommand) ,axc_true))
							{
								bCloseSession = ProcessTcpCommand_SetThermalLeptonFFC(pszReadBuffer, dwCacheLen, pThread, pSession);
							}
							else if(0 == CAxcString::strcmp(szCommand,"upgrade",axc_true))
							{
								bCloseSession = ProcessTcpCommand_Upgrade(pszReadBuffer, dwCacheLen, pThread, pSession);
							}
							else if(0 == CAxcString::strncmp(szCommand,"say_goodmorning",(axc_dword)sizeof(szCommand) ,axc_true))
							{
								bCloseSession = ProcessTcpCommand_ReceivedGPIOServiceNotify(pszReadBuffer, dwCacheLen, pThread, pSession);
							}
							else if(0 == CAxcString::strncmp(szCommand,"say_hi",(axc_dword)sizeof(szCommand) ,axc_true))
							{
								bCloseSession = ProcessTcpCommand_ReceivedGPIOServiceTrigger(pszReadBuffer, dwCacheLen, pThread, pSession);
							}
							else if(0 == CAxcString::strncmp(szCommand,"set_event_notify_config",(axc_dword)sizeof(szCommand) ,axc_true))
							{
								bCloseSession = ProcessTcpCommand_SetEventNotifyConfigAboutTemperature(pszReadBuffer, dwCacheLen, pThread, pSession);
							}
							else if(0 == CAxcString::strncmp(szCommand,"get_event_notify_config",(axc_dword)sizeof(szCommand) ,axc_true))
							{
								bCloseSession = ProcessTcpCommand_GetEventNotifyConfigAboutTemperature(pszReadBuffer, dwCacheLen, pThread, pSession);
							}
							/**
							else if(0 == CAxcString::strncmp(szCommand,"make_a_snapshot",(axc_dword)sizeof(szCommand) ,axc_true))
							{
								bCloseSession = ProcessTcpCommand_Snapshot(pszReadBuffer, dwCacheLen, pThread, pSession);
							}
							else if(0 == CAxcString::strncmp(szCommand,"make_a_videoclip",(axc_dword)sizeof(szCommand) ,axc_true))
							{
								bCloseSession = ProcessTcpCommand_VideoClip(pszReadBuffer, dwCacheLen, pThread, pSession);
							}
							else if(0 == CAxcString::strncmp(szCommand,"send_alert_event",(axc_dword)sizeof(szCommand) ,axc_true))
							{
								bCloseSession = ProcessTcpCommand_AlertEvent(pszReadBuffer, dwCacheLen, pThread, pSession);
							}**/
							else
							{
								// unknow command, to close session
								CAxcString cszSend;
								cszSend.Append("command=reply\r\n");
								cszSend.AppendFormat("reply_src=%s\r\n", szCommand);
								cszSend.Append("result_code=1\r\n");
								cszSend.AppendFormat("result_text=unknown command, '%s'\r\n",szCommand);
								cszSend.Append("\r\n"); // END
								// send to client
								pSession->Send(cszSend.GetBuffer(), cszSend.GetLength()+1);
								bCloseSession = axc_true;
							}
							m_locker_session_delivery.unlock();
							// to close session
							if(bCloseSession)
							{
								break;
							}

						}//axc_true == m_locker_session_delivery.Lock()
						else
						{
							CAxcString cszSend;
							// resource is busy to do new command, to close session
							{
								cszSend.Append("command=reply\r\n");
								cszSend.AppendFormat("reply_src=%s\r\n", szCommand);
								cszSend.Append("result_code=-1\r\n");
								cszSend.Append("result_text=resource is busy, dropping current command!\r\n");
								cszSend.Append("\r\n"); // END
							}
							// send to client
							pSession->Send(cszSend.GetBuffer(), cszSend.GetLength()+1);
							break;
						}

					} // rev command



					const axc_dword dwCommandLen = pszEnd + 1 - pszReadBuffer;
					if(dwCommandLen < dwCacheLen)
					{
						const axc_dword dwRemainLen = dwCacheLen - dwCommandLen;
						CAxcMemory::SafeCopy(pszReadBuffer, pszEnd + 1, dwRemainLen);
						dwCacheLen = dwRemainLen;
					}
					else
					{
						dwCacheLen = 0;
					}
				}//NULL != pszEnd
			}//(iRecvLen <= 0)
			usleep(20000);
		}//while(g_bAppRunning && g_sockTcpListen.IsValid())
	}

	/**
	 * add session remove event: remove a specific pSession
	 * 		by callback
	 */
	if(m_listTcpSession.Remove((axc_ddword)pSession) >= 0)
	{
		char szIp[64] = {""};
		CAxcSocket::IpToString(pSession->GetRemoteIp(), szIp);
		if ((m_bPrintHideDebugMessage == true)
				&&(bIsVideoCommand == false)){
			GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tNORMALTrace_MSK, "N: [%s] del TCP-session: remote %s:%u closed, socket = %d, session_cnt = %d\n",
					"TCP Listener", szIp,
					pSession->GetRemotePort(), pSession->GetSocket(),
					m_listTcpSession.GetCount());
		}

		// video out is controlled by tcp client
		if (bIsVideoCommand == false){
			pSession->Destroy();
			if (pSession){
				delete pSession;
				pSession = NULL;
			}
		}
	}

	delete pThread;
	pThread = NULL;
	if (m_bPrintHideDebugMessage == true){
		GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tNORMALTrace_MSK, "N: [%s] %s stop thread end \n", "TCP Listener", __func__);
	}

	return 1;
}


axc_dword CHeatFinderTcpListener::thread_sendpkg_process()
{
	GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] %s start loop tcp send service ...\n", "TCP Listener", __func__);

	GlobalDef *pGlobal = (GlobalDef *) CHeatFinderUtility::GetGlobalsupport();
	//int l_iLiveViewDataQueueCount = 0;
    pGlobal->AddpThreadRecord("Tcp Port View Sender");

	AxcSetLastError(0);
	axc_byte abyPacket[1600];
	axc_i32 iPacketSize = 0;
	axc_ddword ddwChannelIndex = 0;
	axc_ddword ddwImageRes = 0;

	bool l_bDebugStopflag = false;

	while((!m_bStopThread) && (m_listTcpSession.IsValid()) && (!l_bDebugStopflag))
	{
		//axc_dword current_T = CAxcTime::GetCurrentUTCTime32();
		//printf("D: [%s] PULL time:%u size:%d\n", SYS_EXECUTE_NICKNAME, current_T, (int)(iPacketSize));
		//little bear: polling all in-coming tcp connection, and send one picture to them.
		//FixME: shell using a thread to 'Read' thermal, then using multi-thread to 'Send'.

		//l_iLiveViewDataQueueCount = pGlobal->GetDataQueueAddress()->GetCount();
		if (pGlobal->ChkIsDataQueueEmpty() == true){
			/**
	    	printf("W: [%s] '%s' '%s' not ready \n", "TCP View Sender", ((pGlobal->GetDataQueueAddress()->IsValid() == axc_false)?"Data Queue":""),
	    			((l_iLiveViewDataQueueCount <=0)?"Live view Data":""));
	    	**/
	    	m_hSenderThread.SleepMilliSeconds(100);
			continue;
		}

		iPacketSize = 0;
		if(!pGlobal->GetDataQueueAddress()->Pop(abyPacket, (axc_i32)sizeof(abyPacket), &iPacketSize, &ddwChannelIndex, &ddwImageRes))
		{
			//led_network_twink = false;
			m_hSenderThread.SleepMilliSeconds(10);
			continue;
		}else if (iPacketSize <= 0){
			//printf("V: [TCP View Sender] Pop read too short %d ... %d\n", iPacketSize, __LINE__);
			m_hSenderThread.SleepMilliSeconds(10);
			continue;
		}

		if (m_listTcpSession.GetCount()<=0){
			//m_hSenderThread.SleepMilliSeconds(20);
			continue;
		}


		for(axc_i32 i=m_listTcpSession.GetCount()-1; i>=0; i--)
		{
			CAxcSocketTcpSession* pSession = (CAxcSocketTcpSession*) m_listTcpSession.GetID(i);
			if(pSession && pSession->IsValid())
			{
				void* pContext = (void*) m_listTcpSession.GetItem(i);
				axc_ddword ddwContext = (axc_ddword) pContext;

				/** prototype
				if(((ddwContext & TCP_SESSION_CONTEXT_OPEN_THERMAL)>0)
				   || ((ddwContext & TCP_SESSION_CONTEXT_OPEN_VISION)>0))
				{

					if ((ADE2CAM_CHANNEL_LEPTONRAW == ddwChannelIndex) || (ADE2CAM_CHANNEL_VISUALMAIN == ddwChannelIndex || ADE2CAM_CHANNEL_VISUALSUB == ddwChannelIndex)){
						const axc_i32 iSendResult = pSession->Send(abyPacket, iPacketSize);
						if(iSendResult != iPacketSize)
						{
							pSession->Destroy();
						}
						else
						{
							//cannot add led control, which will cause VERY LAG ....
							//CHeatFinderUtility::SetTwinkleLedShowNetworkUsed();
						}
					}
				}**/

				if((ddwContext & TCP_SESSION_CONTEXT_OPEN_THERMAL)>0)
				{
					if (ADE2CAM_CHANNEL_LEPTONRAW == ddwChannelIndex){
						const axc_i32 iSendResult = pSession->Send(abyPacket, iPacketSize);
						if(iSendResult != iPacketSize)
						{
							pSession->Destroy();
						}
					}
				}

				if((ddwContext & TCP_SESSION_CONTEXT_OPEN_VISION)>0)
				{
					if ((ADE2CAM_CHANNEL_VISUALMAIN == ddwChannelIndex || ADE2CAM_CHANNEL_VISUALSUB == ddwChannelIndex)){
						const axc_i32 iSendResult = pSession->Send(abyPacket, iPacketSize);
						if(iSendResult != iPacketSize)
						{
							pSession->Destroy();
						}
					}
				}
			}//if(pSession && pSession->IsValid())
			else
			{
				//do nothing

			}
		} //for loop

		//send pkg and get the drop list
		//const axc_i32 iSendResult = OnSessionSendto(&l_listTcpSession2SendPKG, &l_listTcpSession2Drop);

		/**
		if(iSendResult == -1){
			//do data
			//printf("Relax ...\n");
			m_hSenderThread.SleepMilliSeconds(20);
			continue;
		}else if (iSendResult == -2)
		{
			//some session can not use anymore.
			axc_i32 l_ilistsize = l_listTcpSession2Drop.GetCount();
			for (axc_i32 i=0; i<l_ilistsize; i++){
				CAxcSocketTcpSession* pSession = (CAxcSocketTcpSession*) l_listTcpSession2Drop.GetID(i);
				const axc_ddword ddwId = (axc_ddword) pSession;

				fireTcpAcceptSessionEvent(pSession, false);
				m_listTcpSession.Remove(ddwId);

				if(pSession){
					pSession->Destroy();
					delete pSession;
					pSession = NULL;
				}
			}
			m_hSenderThread.SleepMilliSeconds(20);
		}else{

			//printf("correct? ...Session: %u ,Rst: %d\n", m_listTcpSession.GetCount(), iSendResult);
		}
		**/
	}

	return 1;
}

// process
axc_bool CHeatFinderTcpListener::ProcessTcpCommand_Polling(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession)
{
	char szValue[256] = "";
	if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(szRecvBuffer, dwRecvSize, "open_stream", szValue, (axc_dword)sizeof(szValue)) > 0)
	{
		// Read the current configuration
		void* pContext = m_listTcpSession.GetItem((axc_ddword)pSession);
		axc_ddword ddwContext = (axc_ddword) pContext;

		// Merge the new Setting to configuration
		const axc_bool bThermal = (NULL != strstr(szValue,"thermal"));
		const axc_bool bVision = (NULL != strstr(szValue,"vision"));
		if(bThermal && bVision){
			ddwContext = (axc_ddword)(TCP_SESSION_CONTEXT_OPEN_THERMAL | TCP_SESSION_CONTEXT_OPEN_VISION);
		}else if(bThermal){
			ddwContext = (axc_ddword)(TCP_SESSION_CONTEXT_OPEN_THERMAL);
		}else if(bVision){
			ddwContext = (axc_ddword)(TCP_SESSION_CONTEXT_OPEN_VISION);
		}else{
			ddwContext = (axc_ddword)0;
		}

		// Write the new Configuration to 'm_listTcpSession'
		void* pContextSet = m_listTcpSession.GetItem((axc_ddword)pSession);
		pContextSet = (void *) ddwContext;
		m_listTcpSession.SetItem((axc_ddword)pSession, pContextSet);

		// Fire to TCP-Client

	}
	return axc_false; // NOT close session
}

axc_bool CHeatFinderTcpListener::ProcessTcpCommand_GetNetConfig(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession)
{
	CAxcString cszSend;
	int iResultCode = 0;
	cszSend.Append("command=reply\r\n");
	cszSend.Append("reply_src=get_netconfig\r\n");

	// read ade project network setting record
	CAxcString cszResultFile;
	char szResultText[256] = {"no error"};
	char szResultTemp[128] = {"\0"};
	CAxcFile file("net/tcp_session/get_net_config");
	CAxcString cszFilename;
	cszFilename.Append("/home/pi/heat_finder/record/record_adepri_hf048_netconfig");

	/*mark.hsieh 20170327
	* iResultCode :
	* 	0: RST_OK
	*	1: RST_REJECT
	*	2: RST_FAIL
	**/
	// create ade project network setting record
	// * give a flag for race condition protect first!

	const int sys_res1 = system("touch /tmp/adepri_hf048_rc ");
	if(0 != sys_res1)
	{
		GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] creat race condition failed, result = %d  by os, %s\n", "TCP Listener", sys_res1, strerror(errno));
		iResultCode = 1;
		strcpy(szResultTemp, "system call reject, so get network detail fail.");
		CAxcString::strncpy(szResultText, szResultTemp, (axc_dword)sizeof(szResultTemp));
	}
	const int sys_res2 = system("sudo rm -f /home/pi/heat_finder/record/record_adepri_hf048_netconfig ");
	if((0 != sys_res2)&&(1 != iResultCode))
	{
		GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] delect network old record failed, result = %d  by os, %s\n", "TCP Listener", sys_res2, strerror(errno));
		iResultCode = 1;
		strcpy(szResultTemp, "system call remove old file reject, so get network detail fail.");
		CAxcString::strncpy(szResultText, szResultTemp, (axc_dword)sizeof(szResultTemp));
	}

	if (1 == iResultCode)
	{
		cszSend.AppendFormat("result_code=%d\r\n", -1);
		cszSend.AppendFormat("result_text=[sys. call error] %s\r\n", szResultText);
		cszSend.Append("\r\n"); // END
		// send to client
		pSession->Send(cszSend.GetBuffer(), cszSend.GetLength()+1);
		return axc_true; // close session
	}

	bool g_bNetSysCallRunning =axc_true;
	unsigned int SysCallDelay_limitation =6;
	//ADE_SystemCall(NET_ONLY_LISTEN);
	const int result = system("sudo bash /home/pi/heat_finder/function_bash/output_adepri_hf048_netconfig.sh");
	if(0 != result)
	{
		g_bNetSysCallRunning =axc_false;
		GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] creat network status record failed, result = %d  by os\n", "TCP Listener", result);
	}
	// delay 5 time.
	while((g_bNetSysCallRunning == axc_true) && (SysCallDelay_limitation > 0)){
		// waiting 1 sec. when:
		// 1. race condition flag not cancer.
		// 2. new network info. not been created.
		if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/tmp/adepri_hf048_rc"))||axc_true !=(CAxcFileSystem::AccessCheck_IsExisted("/home/pi/heat_finder/record/record_adepri_hf048_netconfig")))
		{
			GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] wait rc done... 1 sec.\n", "TCP Listener");
			sleep(1);//usleep(600);
			SysCallDelay_limitation = (SysCallDelay_limitation- 1);
		}else{
			g_bNetSysCallRunning =axc_false;
			if (SysCallDelay_limitation <= 0){
				GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] race condition process timeout.\n", "TCP Listener");
			}
		}
	}
	//printf("D: [TCP Listener] read new network status record");

	int file_open_rst = 0;
	unsigned int loop_count = 5;
	file_open_rst = file.Open(cszFilename.Get(), "rb");
	while ((0 == file_open_rst) && (loop_count > 0))
	{
		sleep(1);
		loop_count = loop_count-1;
		file_open_rst = file.Open(cszFilename.Get(), "rb");
		GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] read %s fail, try again.\n", "TCP Listener", cszFilename.Get());
	}

	if (0 == file_open_rst)
	{
		iResultCode = errno;
		CAxcString::strncpy(szResultText, cszFilename.Get(), (axc_dword)sizeof(szResultText));
		CAxcString::strncat(szResultText, AxcGetLastErrorText(), (axc_dword)sizeof(szResultText));
	}
	else
	{
		axc_i64 iFileSize = file.FileSize();
		if(iFileSize <= 0 || iFileSize > (10*1024*1024))
		{
			if (iFileSize <= 0)
			{
				iResultCode = ENOENT;
			}
			else
			{
				iResultCode = EFBIG;
			}
			CAxcString::strncpy(szResultText, AxcGetLastErrorText(), (axc_dword)sizeof(szResultText));
		}
		else // iFileSize > 0 && iFileSize < (10*1024*1024)
		{
			// get $ADEPRI_NETWORK_CONFIG_FILE content text
			cszResultFile.ResizeBuffer((axc_dword)iFileSize+1);
			const axc_dword dwReadLen = file.Read(cszResultFile.GetBuffer(), cszResultFile.GetBufferSize()-1);
			char* psz = (char*) cszResultFile.GetBuffer();
			psz[dwReadLen] = 0;
			cszResultFile.UpdateTextLength();
		}
		file.Close();
	}

	if (0 == iResultCode)
	{
		cszSend.AppendFormat("result_code=%d\r\n", 0);
		cszSend.AppendFormat("result_text=%s\r\n", szResultText);
		//send message
		cszSend.AppendFormat("%s", cszResultFile.Get());
		cszSend.Append("\r\n");
	}
	else if (1 == iResultCode)
	{
		cszSend.AppendFormat("result_code=%d\r\n", -1);
		cszSend.AppendFormat("result_text=%s\r\n", szResultText);
	}
	else // if (2 == iResultCode) || (2 <= iResultCode)
	{
		cszSend.AppendFormat("result_code=%d\r\n", iResultCode);
		cszSend.AppendFormat("result_text=[file read/open error] %s\r\n", szResultText);
	}
	cszSend.Append("\r\n"); // END
	// send to client
	pSession->Send(cszSend.GetBuffer(), cszSend.GetLength()+1);
	return axc_true; // close session
}

axc_bool CHeatFinderTcpListener::ProcessTcpCommand_Probe(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession)
{

	CAxcString cszSend;
	CAxcString cszErrorSend;
	bool ret = false;
	double l_doubleCurrentTimestamp = CAxcTime::GetCurrentUTCTimeMs();

	GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tVERBOSETrace_MSK, "V: [%s] %s.\n", "TCP Listener", __func__);

	//fire 'Probe' broadcast renew request event
	fireNeedtoRenewBroadcastRecordEvent();

	//only wait 3 second. for avoid old data
	double l_dwAvoidDeadendTimestamp =0;
	while(ret == false){
		l_dwAvoidDeadendTimestamp = CAxcTime::GetCurrentUTCTimeMs();
		if (3.0 < (l_dwAvoidDeadendTimestamp - l_doubleCurrentTimestamp)){
			GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] broadcast information flash and update timeout.\n", "TCP Listener");
			break;
		}
		usleep(100*1000);
		ret = onGetBroadcastLocalRecord(&cszSend, &l_doubleCurrentTimestamp);
	}

	if (ret == true){

		pSession->Send(cszSend.GetBuffer(), cszSend.GetLength()+1);
	}else{
		cszErrorSend.Append("command=reply\r\n");
		cszErrorSend.Append("reply_src=probe\r\n");
		cszErrorSend.Append("result_code=-1\r\n");
		cszErrorSend.Append("result_text=resource is busy, dropping current command!\r\n");
		cszErrorSend.Append("\r\n"); // END
		pSession->Send(cszErrorSend.GetBuffer(), cszErrorSend.GetLength()+1);
	}

    return axc_true; // close session
}

axc_bool CHeatFinderTcpListener::ProcessTcpCommand_GetConfig(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession)
{
	// read config, encode to base64, send to client
	CAxcFile file("net/tcp_session/get_config");
	if(file.Open("/home/pi/ade_camera_udp_out/adehf.ini", "rb"))
	{
		const axc_i64 iFileSize = file.FileSize();
		if(iFileSize > 0 && iFileSize < (100*1024*1024)) // 1B ~ 100MB
		{
			const axc_dword dwFileSize = (axc_dword) iFileSize;
			CAxcMemory memRead("net/tcp_session/get_config/read");
			CAxcMemory memBase64("net/tcp_session/get_config/read");
			if(memRead.Create(dwFileSize) &&
			   memBase64.Create(CAxcBase64::GetEncodeOutputLength(dwFileSize+4)))
			{
				if(dwFileSize == file.Read(memRead.GetAddress(), dwFileSize))
				{
					const axc_i32 iBase64Len = CAxcBase64::Encode((axc_byte*)memRead.GetAddress(), dwFileSize, (char*)memBase64.GetAddress(), (axc_i32)memBase64.GetBufferSize());
					if(iBase64Len > 0)
					{
						char* pszBase64 = (char*) memBase64.GetAddress();
						pszBase64[iBase64Len] = 0; // resure end of '\0'
						CAxcString cszSend;
						cszSend.Append("command=reply\r\n");
						cszSend.Append("reply_src=get_config\r\n");
						cszSend.Append("result_code=0\r\n");
						cszSend.Append("result_text=OK\r\n");
						cszSend.Append("config_file_base64=");
						cszSend.Append(pszBase64);
						cszSend.Append("\r\n\r\n"); // END
						// send to client
						pSession->Send(cszSend.GetBuffer(), cszSend.GetLength()+1);
					}
				}
			}
		}
	}
	return axc_true; // close session
}

axc_bool CHeatFinderTcpListener::ProcessTcpCommand_GetThermalIvsConfig(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession)
{
	CAxcString cszSend;
	cszSend.Append("command=reply\r\n");
	cszSend.Append("reply_src=get_thermal_ivs_config\r\n");
	cszSend.Append("result_code=0\r\n");
	cszSend.Append("result_text=OK\r\n");

	//char szValue[512] = {""};

	CAxcFile file("net/tcp_session/get_config");
	if(file.Open("/home/pi/ade_camera_udp_out/adehf.ini", "rb"))
	{
		const axc_i64 iFileSize = file.FileSize();
		if(iFileSize > 0 && iFileSize < (10*1024*1024)) // 1B ~ 10MB
		{
			const axc_dword dwFileSize = (axc_dword) iFileSize;
			CAxcString cszResultFile;
			cszResultFile.ResizeBuffer((axc_dword)iFileSize+1);
			if(dwFileSize == file.Read(cszResultFile.GetBuffer(), dwFileSize))
			{
				cszSend.Append(cszResultFile.Get());
			}
		}
	}

	cszSend.Append("\r\n"); // END
	// send to client
	pSession->Send(cszSend.GetBuffer(), cszSend.GetLength()+1);
	return axc_true; // close session
}

axc_bool CHeatFinderTcpListener::ProcessTcpCommand_GetThermalIvsResult(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession)
{
	CAxcString cszSend;
    void *pContext = CHeatFinderUtility::GetGlobalsupport();
    GlobalDef *pApp = reinterpret_cast<GlobalDef*> (pContext);
    pApp->OnCollectThermalMsgToString(cszSend);

	// send to client
	pSession->Send(cszSend.GetBuffer(), cszSend.GetLength()+1);

	// client query keep-tcp-connection ?
	axc_bool bKeepConnection = axc_false;
	char szValue[128] = {""};
	if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(szRecvBuffer, dwRecvSize, "keep_connection", szValue, (axc_dword)sizeof(szValue)) > 0)
	{
		bKeepConnection = (axc_bool) atoi(szValue);
	}
	return !bKeepConnection;
}

axc_bool CHeatFinderTcpListener::ProcessTcpCommand_GetThermalOverlayLocate(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession){

	CAxcString cszSend;
	//ade_camera_sdk *ac_obj = (ade_camera_sdk *)CHeatFinderUtility::GetADECameraSDK();
	CadeSDKAPIleptonthermal *pLepSDK = (CadeSDKAPIleptonthermal *)CHeatFinderUtility::GetLeptonSDK();

	cszSend.Append("\r\n");
	pSession->Send(cszSend.GetBuffer(), cszSend.GetLength()+1);

	cszSend.Append("command=reply\r\n");
	cszSend.Append("reply_src=get_thermal_overlay_locate\r\n");

	unsigned short overlay_p[3][THERMAL_OVERLAY_COL] = {{0,},};
	int rst = 0;
	//rst = ac_obj->thermal_get_overlay_parameter("overlay", overlay_p, 3);
	rst = pLepSDK->getThermalCurrentParameterValue("overlay", overlay_p, 3);

	if (rst >0){
		for(int i=0; i<3; i++){
			GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] debug... overlay #%d: (%ux%u) rect (%u,%u,%u,%u) / (%ux%u) rect (%u,%u,%u,%u)\n", "TCP Listener", i+1,
			overlay_p[i][0],overlay_p[i][1],overlay_p[i][2],overlay_p[i][3],overlay_p[i][4],overlay_p[i][5],
			overlay_p[i][6],overlay_p[i][7],overlay_p[i][8],overlay_p[i][9],overlay_p[i][10],overlay_p[i][11]);
		}
	}

	char szValue[128] = {""};
	if (rst <= 0) {
		//GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] %s: get overlay parameter fail\n", "TCP Listener", __func__);
	} else if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(szRecvBuffer, dwRecvSize, "resolution", szValue, (axc_dword)sizeof(szValue)) > 0) {
		if (0 == CAxcString::strncmp(szValue,"1920x1080",(axc_dword)sizeof(szValue) ,axc_true)) {
			GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] 1920x1080 \n", "TCP Listener");
			cszSend.Append("result_code=0\r\n");
			cszSend.Append("result_text=OK\r\n");
			cszSend.Append("resolution=1920x1080\r\n");
			cszSend.AppendFormat("lepton_w=%u\r\n",overlay_p[0][0]);
			cszSend.AppendFormat("lepton_h=%u\r\n",overlay_p[0][1]);
            cszSend.AppendFormat("lepton_l=%u\r\n",overlay_p[0][2]);
            cszSend.AppendFormat("lepton_t=%u\r\n",overlay_p[0][3]);
			cszSend.AppendFormat("lepton_r=%u\r\n",overlay_p[0][4]);
			cszSend.AppendFormat("lepton_b=%u\r\n",overlay_p[0][5]);
			cszSend.AppendFormat("camera_w=%u\r\n",overlay_p[0][6]);
			cszSend.AppendFormat("camera_h=%u\r\n",overlay_p[0][7]);
            cszSend.AppendFormat("camera_l=%u\r\n",overlay_p[0][8]);
            cszSend.AppendFormat("camera_t=%u\r\n",overlay_p[0][9]);
			cszSend.AppendFormat("camera_r=%u\r\n",overlay_p[0][10]);
			cszSend.AppendFormat("camera_b=%u\r\n",overlay_p[0][11]);
		} else if (0 == CAxcString::strncmp(szValue,"1280x720",(axc_dword)sizeof(szValue) ,axc_true)) {
			GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] 1280x720 \n", "TCP Listener");
			cszSend.Append("result_code=0\r\n");
			cszSend.Append("result_text=OK\r\n");
			cszSend.Append("resolution=1280x720\r\n");
			cszSend.AppendFormat("lepton_w=%u\r\n",overlay_p[1][0]);
			cszSend.AppendFormat("lepton_h=%u\r\n",overlay_p[1][1]);
            cszSend.AppendFormat("lepton_l=%u\r\n",overlay_p[1][2]);
            cszSend.AppendFormat("lepton_t=%u\r\n",overlay_p[1][3]);
			cszSend.AppendFormat("lepton_r=%u\r\n",overlay_p[1][4]);
			cszSend.AppendFormat("lepton_b=%u\r\n",overlay_p[1][5]);
			cszSend.AppendFormat("camera_w=%u\r\n",overlay_p[1][6]);
			cszSend.AppendFormat("camera_h=%u\r\n",overlay_p[1][7]);
            cszSend.AppendFormat("camera_l=%u\r\n",overlay_p[1][8]);
            cszSend.AppendFormat("camera_t=%u\r\n",overlay_p[1][9]);
			cszSend.AppendFormat("camera_r=%u\r\n",overlay_p[1][10]);
			cszSend.AppendFormat("camera_b=%u\r\n",overlay_p[1][11]);
		} else if (0 == CAxcString::strncmp(szValue,"640x360",(axc_dword)sizeof(szValue) ,axc_true)) {
			GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] 640x360 \n", "TCP Listener");
			cszSend.Append("result_code=0\r\n");
			cszSend.Append("result_text=OK\r\n");
			cszSend.Append("resolution=640x360\r\n");
			cszSend.AppendFormat("lepton_w=%u\r\n",overlay_p[2][0]);
			cszSend.AppendFormat("lepton_h=%u\r\n",overlay_p[2][1]);
            cszSend.AppendFormat("lepton_l=%u\r\n",overlay_p[2][2]);
            cszSend.AppendFormat("lepton_t=%u\r\n",overlay_p[2][3]);
			cszSend.AppendFormat("lepton_r=%u\r\n",overlay_p[2][4]);
			cszSend.AppendFormat("lepton_b=%u\r\n",overlay_p[2][5]);
			cszSend.AppendFormat("camera_w=%u\r\n",overlay_p[2][6]);
			cszSend.AppendFormat("camera_h=%u\r\n",overlay_p[2][7]);
            cszSend.AppendFormat("camera_l=%u\r\n",overlay_p[2][8]);
            cszSend.AppendFormat("camera_t=%u\r\n",overlay_p[2][9]);
			cszSend.AppendFormat("camera_r=%u\r\n",overlay_p[2][10]);
			cszSend.AppendFormat("camera_b=%u\r\n",overlay_p[2][11]);
		} else {
			GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] unknow resolution (return zero value) \n", "TCP Listener");
			cszSend.Append("result_code=-1\r\n");
			cszSend.Append("result_text=FAIL\r\n");
		}
	}

	cszSend.Append("\r\n"); // END
	pSession->Send(cszSend.GetBuffer(), cszSend.GetLength()+1);
	return axc_true;
}

axc_bool CHeatFinderTcpListener::ProcessTcpCommand_SetEmissivity(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession)
{
	CAxcString cszSend;
	int 	error_code =0;
	double   l_dEmissivity = 0.0;
	//ade_camera_sdk *ac_obj = (ade_camera_sdk *)CHeatFinderUtility::GetADECameraSDK();
	CadeSDKAPIleptonthermal *pLepSDK = (CadeSDKAPIleptonthermal *)CHeatFinderUtility::GetLeptonSDK();
    void *pContext = CHeatFinderUtility::GetGlobalsupport();
    GlobalDef *pGlobal = reinterpret_cast<GlobalDef*> (pContext);

	char szValue1[256];
	char result_error[256];
	if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(szRecvBuffer, (axc_dword)dwRecvSize, "emissivity", szValue1, (axc_dword)sizeof(szValue1)) > 0)
	{
		l_dEmissivity = atof(szValue1);
	}

	if ((l_dEmissivity >= 0.00) &&
		(l_dEmissivity <= 1.00)){
		pGlobal->SetEmissivity(l_dEmissivity);
	}
	else{
		GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] %s fail, whose vaule out of range[0~1]: %.2lf(str:%s) ", "TCP Listener", __func__, l_dEmissivity, szValue1);
		CAxcString::strncpy(result_error, "input value out of range(0~1:floating)", 256);
		error_code= 5;   //EIO: Input/output error
	}

	// set to system, device, or process
	if (error_code ==0){
		bool rst = true;
		//rst = ac_obj->set_thermal_emissivity(pGlobal->GetEmissivity());
		rst = pLepSDK->setThermalEmissivity(pGlobal->GetEmissivity());
		error_code = (rst == true)? 0: 5;
		if( rst == 0 ) {
			Cheatfinderconfigmanager *pConfigContextManager = (Cheatfinderconfigmanager *)CHeatFinderUtility::GetConfigObj();
			T_CONFIG_FILE_OPTIONS *pConfigContext = ((pConfigContextManager == NULL)?NULL:pConfigContextManager->GetConfigContext());
			if( pConfigContext ) {
				pConfigContext->dwEmissivity = (axc_dword)(pGlobal->GetEmissivity()*1000);
			}
		}
	}
	//error_code =?

	cszSend.Append("command=reply\r\n");
	cszSend.Append("reply_src=set_emissivity\r\n");
	if (error_code ==0){
		cszSend.Append("result_code=0\r\n");
		cszSend.Append("result_text=OK\r\n");
		//cszSend.AppendFormat("emissivity=%.2f\r\n", l_aptm_Emissivity);
		GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] %s success, those value: %.2f\n", "TCP Listener", __func__, pGlobal->GetEmissivity());
	}else{
		cszSend.AppendFormat("result_code=%d\r\n", error_code);
		cszSend.AppendFormat("result_text=Fail, %s\r\n", result_error);
	}

	cszSend.Append("\r\n");
	pSession->Send(cszSend.GetBuffer(), cszSend.GetLength()+1);

	// close session
	return axc_true;
}
axc_bool CHeatFinderTcpListener::ProcessTcpCommand_GetEmissivity(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession)
{
	CAxcString cszSend;
	int 	error_code =0;
	//ade_camera_sdk *ac_obj = (ade_camera_sdk *)CHeatFinderUtility::GetADECameraSDK();
	CadeSDKAPIleptonthermal *pLepSDK = (CadeSDKAPIleptonthermal *)CHeatFinderUtility::GetLeptonSDK();
    void *pContext = CHeatFinderUtility::GetGlobalsupport();
    GlobalDef *pGlobal = reinterpret_cast<GlobalDef*> (pContext);

	// get from system, device, or process
	float l_fRst = 1.0;
	//l_fRst = ac_obj->get_thermal_emissivity();
	l_fRst = pLepSDK->getThermalEmissivity();
	error_code = (l_fRst >= 0 )? 0: 5;
	if (error_code ==0){
		pGlobal->SetEmissivity(l_fRst);
	}
	GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] %s success, those value: %.2f from %.2f\n", "TCP Listener", __func__, pGlobal->GetEmissivity(), l_fRst);

	cszSend.Append("command=reply\r\n");
	cszSend.Append("reply_src=get_emissivity\r\n");
	if (error_code ==0){
		cszSend.Append("result_code=0\r\n");
		cszSend.Append("result_text=OK\r\n");
		cszSend.AppendFormat("emissivity=%.2f\r\n", pGlobal->GetEmissivity());
	}else{
		cszSend.AppendFormat("result_code=%d\r\n", error_code);
		cszSend.Append("result_text=Fail\r\n");
	}

	cszSend.Append("\r\n");
	pSession->Send(cszSend.GetBuffer(), cszSend.GetLength()+1);

	// close session
	return axc_true;
}

axc_bool CHeatFinderTcpListener::ProcessTcpCommand_SetThermalLeptonFFC(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession)
{
	CAxcString cszSend;
	int iResultCode = 0;
	char szResultText[256] = {"OK"};
	axc_bool rst=axc_false;
	//ade_camera_sdk *ac_obj = (ade_camera_sdk *)CHeatFinderUtility::GetADECameraSDK();
	CadeSDKAPIleptonthermal *pLepSDK = (CadeSDKAPIleptonthermal *)CHeatFinderUtility::GetLeptonSDK();

	//rst = ac_obj->set_thermal_command(2);   //((unsigned int) CMD_FFC);
	rst = (pLepSDK->clickedThermalFFC()==true)?axc_true:axc_false;
	if (rst!=axc_true)
	{
		iResultCode = -1;
		CAxcString::strncpy(szResultText, "Setting Fail", (axc_dword)sizeof(szResultText));
	}

	cszSend.Append("command=reply\r\n");
	cszSend.Append("reply_src=execute_ffc\r\n");
	cszSend.AppendFormat("result_code=%d\r\n", iResultCode);
	cszSend.AppendFormat("result_text=%s\r\n", szResultText);
	cszSend.Append("\r\n"); // END

	// send to client
	pSession->Send(cszSend.GetBuffer(), cszSend.GetLength()+1);

	return axc_true;
}

axc_bool CHeatFinderTcpListener::ProcessTcpCommand_Upgrade(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession)
{
	int iResultCode = -1;
	char szResultText[64] = {"failed"};

	GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] upgrade begin.\n", "Update");
	CAxcMemory memBase64("net/upgrade/base64");
	CAxcMemory memFile("net/upgrade/base64_dec");
	if(!memBase64.Create(dwRecvSize+1))
	{
		iResultCode = AxcGetLastError();
		CAxcString::strncpy(szResultText, AxcGetLastErrorText(), (axc_dword)sizeof(szResultText));
	}
	else if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(szRecvBuffer, dwRecvSize, "upgrade_file_base64", (char*)memBase64.GetAddress(), memBase64.GetBufferSize()) <= 0)
	{
		iResultCode = 1;
		strcpy(szResultText, "no data");
	}
	else
	{
		char* pszBase64 = (char*) memBase64.GetAddress();
		const axc_dword dwBase64Len = (axc_dword)strlen(pszBase64);
		axc_i32 iDecLen = 0;
		if(!memFile.Create(CAxcBase64::GetDecodeOutputLength(dwBase64Len)+4))
		{
			iResultCode = AxcGetLastError();
			CAxcString::strncpy(szResultText, AxcGetLastErrorText(), (axc_dword)sizeof(szResultText));
		}
		else if((iDecLen = CAxcBase64::Decode(pszBase64,(axc_i32)dwBase64Len,(axc_byte*)memFile.GetAddress(),(axc_i32)memFile.GetBufferSize())) <= 0)
		{
			iResultCode = AxcGetLastError();
			CAxcString::strncpy(szResultText, AxcGetLastErrorText(), (axc_dword)sizeof(szResultText));
		}
		else
		{
			CAxcFile file("net/upgrade/file");
			if(!file.Open(UPGRADE_FILE, "wb") ||
				file.Write(memFile.GetAddress(), iDecLen) <= 0)
			{
				iResultCode = AxcGetLastError();
				CAxcString::strncpy(szResultText, AxcGetLastErrorText(), (axc_dword)sizeof(szResultText));
			}
			else
			{
				iResultCode = 0;
				strcpy(szResultText,"OK");
				GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] write upgrade file '%s' size %lld bytes.", "Update", file.FileName(), file.Tell());
				file.Close();
			}
		}
	}

	CAxcString cszSend;
	cszSend.Append("command=reply\r\n");
	cszSend.Append("reply_src=upgrade\r\n");
	cszSend.AppendFormat("result_code=%d\r\n", iResultCode);
	cszSend.AppendFormat("result_text=%s\r\n", szResultText);
	cszSend.Append("\r\n"); // END
	// send to client
	pSession->Send(cszSend.GetBuffer(), cszSend.GetLength()+1);

	GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] upgrade finish.\n", "Update");
	// if succeed, reboot os
	if(CAxcFileSystem::AccessCheck_IsExisted(UPGRADE_FILE))
	{
		GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] ready to reboot.\n", "Update");

		//system("sudo killall -s SIGUSR2 ade_gpio_set &");
		fireTcpReceivedUpgradeEventNotify(0);
	}
	return axc_true; // close session
}

axc_bool CHeatFinderTcpListener::ProcessTcpCommand_ReceivedGPIOServiceTrigger(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession)
{
	CAxcString cszSend;
	//int iResultCode = 0;
	//char szResultText[256] = {"OK"};
	//printf("D: [%s] %s \n", "TCP Listener", __func__);


	cszSend.Append("command=reply\r\n");
	cszSend.Append("reply_src=say_hi\r\n");

	char szValue[256];
	if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(szRecvBuffer, (axc_dword)dwRecvSize, "who", szValue, (axc_dword)sizeof(szValue)) > 0)
	{
		if(0 == CAxcString::strcmp(szValue,"gpio_set",axc_true))
		{
			CHeatFinderUtility::RecvGPIOSrvResponse();
		}
		else
		{
			GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] %s unknown member: %s\n", "TCP Listener", __func__, szValue);
		}

	}
	cszSend.Append("\r\n"); // END
	// send to client
	pSession->Send(cszSend.GetBuffer(), cszSend.GetLength()+1);

	return axc_true; // close session
}

axc_bool CHeatFinderTcpListener::ProcessTcpCommand_ReceivedGPIOServiceNotify(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession)
{
	CAxcString cszSend;
	//int iResultCode = 0;
	//char szResultText[256] = {"OK"};
	printf("D: [%s] %s \n", "TCP Listener", __func__);


	cszSend.Append("command=reply\r\n");
	cszSend.Append("reply_src=say_hi\r\n");

	char szValue[256];
	if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(szRecvBuffer, (axc_dword)dwRecvSize, "who", szValue, (axc_dword)sizeof(szValue)) > 0)
	{
		if(0 == CAxcString::strcmp(szValue,"gpio_set",axc_true))
		{
			//need to send the ready signal to him
			CHeatFinderUtility::SetTwinkleLedShowStatusOK();
			CHeatFinderUtility::RecvGPIOSrvResponse();
		}
		else
		{
			GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] %s unknown member: %s\n", "TCP Listener", __func__, szValue);
		}

	}

	if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(szRecvBuffer, (axc_dword)dwRecvSize, "version", szValue, (axc_dword)sizeof(szValue)) > 0)
	{
		GLog(tNetworkTCPTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] %s GPIO Command Server Version: %s\n", "TCP Listener", __func__, szValue);
	}
	cszSend.Append("\r\n"); // END
	// send to client
	pSession->Send(cszSend.GetBuffer(), cszSend.GetLength()+1);

	return axc_true; // close session
}

axc_bool CHeatFinderTcpListener::ProcessTcpCommand_SetEventNotifyConfigAboutTemperature(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession)
{

	CAxcString cszSend;
	int iResultCode = 0;
	char szResultText[256] = {"OK"};

	//iResultCode = FUNC(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, szResultText)
	iResultCode = m_tcpeventnotifyMonitor.SetTemperatureEventToMonitor(szRecvBuffer, dwRecvSize, pThread, szResultText);

	cszSend.Append("command=reply\r\n");
	cszSend.Append("reply_src=set_event_notify_config\r\n");
	cszSend.AppendFormat("result_code=%d\r\n", iResultCode);
	cszSend.AppendFormat("result_text=%s\r\n", szResultText);
	cszSend.Append("\r\n"); // END

	// send to client
	pSession->Send(cszSend.GetBuffer(), cszSend.GetLength()+1);
	return axc_true;
}

axc_bool CHeatFinderTcpListener::ProcessTcpCommand_GetEventNotifyConfigAboutTemperature(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, CAxcSocketTcpSession* pSession)
{
	CAxcString cszSend;
	int iResultCode = 0;
	char szResultText[256] = {"OK"};

	axc_bool is_working = axc_false;
	CAxcString l_caxcstrAddress;
	l_caxcstrAddress.Append("");
	CAxcString l_caxcstrEnable;

	////iResultCode = FUNC(const char* szRecvBuffer, const axc_dword dwRecvSize, CAxcThread* pThread, szResultText, l_caxcstrEnable, l_caxcstrAddress)
	is_working = m_tcpeventnotifyMonitor.GetTemperatureEventFromMonitor(szRecvBuffer, dwRecvSize, pThread, szResultText, l_caxcstrEnable, l_caxcstrAddress);

	if (is_working <0){
		iResultCode = -1;
	}

	cszSend.Append("command=reply\r\n");
	cszSend.Append("reply_src=get_event_notify_config\r\n");
	cszSend.AppendFormat("result_code=%d\r\n", iResultCode);
	cszSend.AppendFormat("result_text=%s\r\n", szResultText);
	cszSend.AppendFormat("event_notify_enable=%d\r\n", is_working);
	cszSend.AppendFormat("event_notify_statusall=%s\r\n", l_caxcstrEnable.Get());
	cszSend.AppendFormat("notify_http_address=%s\r\n", l_caxcstrAddress.Get());
	cszSend.Append("\r\n"); // END

	// send to client
	pSession->Send(cszSend.GetBuffer(), cszSend.GetLength()+1);
	return axc_true;

}
