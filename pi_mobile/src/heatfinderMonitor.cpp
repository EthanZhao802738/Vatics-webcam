/*
 * heatfinderWatchdog.cpp
 *
 *  Created on: Jan 24, 2018
 *      Author: markhsieh
 */

#include "heatfinderMonitor.h"
#include "ade_camera_sdk.h"


CHeatfinderMonitor::CHeatfinderMonitor():
	m_caxclistThermalTemperatureMonitor("monitor/temperature"),
	m_hThread("monitor/checker")
{
	m_caxclistThermalTemperatureMonitor.Create(32);
	m_bStopThread = false;
    sem_init( &requestOutput, 0, 0 );
}

CHeatfinderMonitor::~CHeatfinderMonitor(){

	Stop();

	GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] Tepperature Alert Notify list: Clear\n", "Monitor Manager");
	const axc_i32 iTcpSessionCount = m_caxclistThermalTemperatureMonitor.GetCount();
	for(axc_i32 i=iTcpSessionCount-1; i>=0; i--)
	{
		HeatFinderMonitor4Temperature* pObj = (HeatFinderMonitor4Temperature*) m_caxclistThermalTemperatureMonitor.GetID(i);
		m_caxclistThermalTemperatureMonitor.Remove(i);
		if((pObj->Session)
			|| (pObj->IsUsingLibcurl() == axc_true)){
			pObj->Session->Destroy();
			delete pObj;
			pObj = NULL;
		}
	}
	m_caxclistThermalTemperatureMonitor.Destroy();

    sem_destroy(&requestOutput);
}

bool CHeatfinderMonitor::Run(){
	bool _bRst = true;

    m_bStopThread = false;

    if (!m_hThread.Create(thread_process, this))
    	_bRst = false;

    return _bRst;

}
void CHeatfinderMonitor::Stop(){
    m_bStopThread = true;
    if (m_hThread.IsValid()){
    	sem_post(&requestOutput); //avoid blocking at sem_wait()
        m_hThread.Destroy(2000);
    }
}

//public
int CHeatfinderMonitor::SetTemperatureEventToMonitor(const char* pccReceivedTCPStr, const axc_dword ui32ReceivedTCPStrLength, CAxcThread* pThread, char* pcRetrunMsg){
	if ((pccReceivedTCPStr == NULL) || (ui32ReceivedTCPStrLength <= 0)){
		GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Monitor Manager] input data not correct! pReceivedStr:(%s) ReceivedStrLength:(%s:%d)\n", ((pccReceivedTCPStr == NULL)?"Wrong":"OK"), ((ui32ReceivedTCPStrLength <= 0)?"Wrong":"OK"), ui32ReceivedTCPStrLength);
		return EINVAL;
	}

	return OnAddTemperatureEvent(pccReceivedTCPStr, ui32ReceivedTCPStrLength, pThread, pcRetrunMsg);
}

int CHeatfinderMonitor::GetTemperatureEventFromMonitor(const char* pccReceivedTCPStr, const axc_dword ui32ReceivedTCPStrLength, CAxcThread* pThread, char* pcRetrunMsg, CAxcString &refcaxcstrNotifyStatus, CAxcString &refcaxcstrNotifyAddress){
	if ((pccReceivedTCPStr == NULL) || (ui32ReceivedTCPStrLength <= 0)){
		GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Monitor Manager] input data not correct! pReceivedStr:(%s) ReceivedStrLength:(%s:%d)\n", ((pccReceivedTCPStr == NULL)?"Wrong":"OK"), ((ui32ReceivedTCPStrLength <= 0)?"Wrong":"OK"), ui32ReceivedTCPStrLength);
		return EINVAL;
	}

	return OnChkTemperatureEvnetSetting(pccReceivedTCPStr, ui32ReceivedTCPStrLength, pThread, pcRetrunMsg, refcaxcstrNotifyStatus, refcaxcstrNotifyAddress);
}

void CHeatfinderMonitor::OnChkAllEventAndTriggerNotify(){
	if (m_caxclistThermalTemperatureMonitor.GetCount() <= 0){
		return;
	}

	sem_post(&requestOutput);

}

axc_dword CHeatfinderMonitor::thread_process(){
	GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Monitor Manager:thread] start\n");

    void *pContext = CHeatFinderUtility::GetGlobalsupport();
    GlobalDef *pGlobal = reinterpret_cast<GlobalDef*> (pContext);
    pGlobal->AddpThreadRecord("Monitor");

	while (!m_bStopThread){
		sem_wait(&requestOutput);
		if (m_bStopThread){
			continue;
		}

		int l_iTemperatureRst = OnChkTemperatureEventAndTriggerNotify();
		if (l_iTemperatureRst < 1){
			//error happen
		}
	}
	GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Monitor Manager:thread] stop\n");
	return 1;
}

//protected
int CHeatfinderMonitor::OnChkTemperatureEventAndTriggerNotify(){
	unsigned int  heat_alarm_count =0;
	GlobalDef *pGlobal = (GlobalDef *) CHeatFinderUtility::GetGlobalsupport();

	//axc_bool	isResend	= axc_false;  //<--- need global
	//axc_bool	m_bNotify_was_sended	= axc_false;  //<--- need global
	const char p_Curl_Command[128] = {"curl -H 'Content-Type: text/html; charset=utf-8' --get "}; //take care file name

	if (pGlobal->GetThermalCurrentHeatObjSeekStatus() > 0){
		heat_alarm_count = pGlobal->GetThermalCurrentFoundHeatObj(); // g_ThermalIvsOut.wHeatObj;
	}else{
		heat_alarm_count = 0;
	}


	if (heat_alarm_count <= 0){
		if(m_bNotify_was_sended == axc_true)
		{
			m_bNotify_was_sended = axc_false;
			//printf("D: [Net] %s turn off notify warning\n", __func__);
		}

		return 1;
	}

	if ((heat_alarm_count >= 1)
			&& (m_bNotify_was_sended == axc_false))
	{
		for(axc_i32 i=m_caxclistThermalTemperatureMonitor.GetCount()-1; i>=0; i--)
		{
			HeatFinderMonitor4Temperature* pTWD = (HeatFinderMonitor4Temperature*) m_caxclistThermalTemperatureMonitor.GetID(i);

			mNotifyLock.lock();
			{

				if (pTWD->IsUsingLibcurl() == axc_true)
				{
					GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Net] %s using curl ... \n", __func__);
					// using libcurl get()!
					// using raspberry pi os command 'curl --get'
					// command
					CAxcString pszCommandAll;
					// remote input string encode by UTF-8, so that do not need re-encode.
					if (pTWD->IsWorking() == axc_true)
					{
						pszCommandAll.AppendFormat("%s", p_Curl_Command);
						pszCommandAll.AppendFormat(" \"%s\"", pTWD->GetRemoteMsg());

						GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Net] %s report send: %s\n", __func__, pszCommandAll.Get());

						// system call
						system(pszCommandAll.Get());

					}

				}
				else
				{

					CAxcString cszTempAlarmSend;

					cszTempAlarmSend.Append("command=reply\r\n");
					cszTempAlarmSend.Append("reply_src=event_notify_alarm\r\n");
					cszTempAlarmSend.Append("result_code=0\r\n");
					cszTempAlarmSend.Append("result_text=OK\r\n");

					cszTempAlarmSend.AppendFormat("event_notify=%s\r\n", pTWD->GetRemoteMsg());
					cszTempAlarmSend.Append("\r\n");

					// if no socket, create one
					if (pTWD->Session == NULL)
					{
						CAxcSocketTcpSession tcp("watchdog/msgsend");
						if(tcp.CreateClient() &&
								tcp.ClientConnectToServer(pTWD->GetRemoteIp(), pTWD->GetRemotePort()))
						{
							pTWD->Session = &tcp;
						}
					}

					// send
					axc_i32 send_buffer_len = (cszTempAlarmSend.GetLength()+1);
					axc_i32 send_length = pTWD->Session->Send(cszTempAlarmSend.GetBuffer(), send_buffer_len);

					if (send_length != send_buffer_len)
					{
						GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Net] %s checking report: %s, send %d len pkg\n", __func__, cszTempAlarmSend.Get(), send_length);
					}
				}
				mNotifyLock.unlock();
			}
		} // for loop

		m_bNotify_was_sended = axc_true;
	}
	return 1; //success
}

int CHeatfinderMonitor::OnAddTemperatureEvent(const char* pccReceivedTCPStr, const axc_dword ui32ReceivedTCPStrLength, CAxcThread* pThread, char* pcRetrunMsg){

	bool l_bOnOffExternalThread = false;
	bool l_bOnOffFeedBackMsg = false;
	int l_iRst = 0; //success

	//---------------init.----------------
	l_bOnOffExternalThread = ((pThread == NULL)?false:true);
	l_bOnOffFeedBackMsg = ((pcRetrunMsg == NULL)?false:true);
	//------------------------------------

	//check gate
	axc_bool rst=axc_false;
	//create or destroy
	axc_bool live_or_dead = axc_false;
	//using ip+port or libcurl send message
	axc_bool use_libcurl = axc_false;

	//tmp record
	//axc_dword 				t_RemoteIp = 0;  //xiaogyi suggest [20170511]
	//axc_word 				t_RemotePort = 0;  //xiaogyi suggest [20170511]
	CAxcString				t_Msg;

	if(m_caxclistThermalTemperatureMonitor.GetCount() >= m_caxclistThermalTemperatureMonitor.GetMaxItemCount())
	{
		GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Monitor Manager] set_event_notify_config: new Temperature Watcher failed, list full (num=%d: limite=%d) !\n", m_caxclistThermalTemperatureMonitor.GetCount(), m_caxclistThermalTemperatureMonitor.GetMaxItemCount());

		l_iRst = 11;
		if (l_bOnOffFeedBackMsg){
			CAxcString::strncpy(pcRetrunMsg, "Setting Fail, Event List Full", (axc_dword)sizeof(pcRetrunMsg));
		}
	}
	else
	{
		char szValue[256] = {""};
		if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(pccReceivedTCPStr, ui32ReceivedTCPStrLength, "event_notify_enable", szValue, (axc_dword)sizeof(szValue)) > 0)
		{
			const axc_bool bTempWatchCreate = ((NULL != strstr(szValue,"1"))
										|| (NULL != strstr(szValue,"on"))
										|| (NULL != strstr(szValue,"yes"))
										|| (NULL != strstr(szValue,"true"))
										);
			const axc_bool bTempWatchDestroy = ((NULL != strstr(szValue,"0"))
										|| (NULL != strstr(szValue,"off"))
										|| (NULL != strstr(szValue,"no"))
										|| (NULL != strstr(szValue,"false"))
										);

			if ((bTempWatchCreate == axc_false) && (bTempWatchDestroy == axc_false))
			{
		    	//CHeatFinderUtility::PrintNowData(1);
				GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [Monitor Manager] set_event_notify_config: event_notify_enable value illegal (value=%s), please checking your command context\n",szValue);
				rst=axc_false;

				l_iRst = EINVAL; //Invalid argument
				if (l_bOnOffFeedBackMsg){
					CAxcString::strncpy(pcRetrunMsg, "event_notify_enable value illegal:", (axc_dword)sizeof(pcRetrunMsg));
					CAxcString::strncat(pcRetrunMsg, szValue, (axc_dword)sizeof(pcRetrunMsg));
				}
			}
			else
			{
				if (bTempWatchCreate == axc_true)
				{
					live_or_dead = axc_true;
				}
				else if (bTempWatchDestroy == axc_true)
				{
					live_or_dead = axc_false;
				}
				rst = axc_true;
			}
		}
		else
		{
	    	//CHeatFinderUtility::PrintNowData(1);
			GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [Monitor Manager] set_event_notify_config: miss event_notify_enable, please checking your command context\n");
			rst=axc_false;

			l_iRst = EINVAL; //Invalid argument
			if (l_bOnOffFeedBackMsg){
				CAxcString::strncpy(pcRetrunMsg, "miss event_notify_enable", (axc_dword)sizeof(pcRetrunMsg));
			}
		}

		//when check event_notify_enable legal, process will be accepted to check others command value.
		if (rst == axc_true)
		{
			if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(pccReceivedTCPStr, ui32ReceivedTCPStrLength, "notify_http_address", szValue, (axc_dword)sizeof(szValue)) > 0)
			{
				t_Msg.AppendFormat("%s", szValue);
				use_libcurl = axc_true;
				GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Monitor Manager] set_event_notify_config: get %s\n", t_Msg.Get());
			}
			//create
			if (live_or_dead == axc_true)
			{
				//while(twdMutex.Lock() == axc_false)
				while(mQueueLock.try_lock() == false)
				{
					if (l_bOnOffExternalThread){
						CAxcTime::SleepMilliSeconds(20); //wait object unlock
					}else{
						usleep(20*1000);
					}
				}

				//create new watcher for new event
				HeatFinderMonitor4Temperature* pTWD_new = new HeatFinderMonitor4Temperature();
				pTWD_new->SetWorkingStatus(true);
				/*
				 * functions
				 * set_remote_target_for_sending_msg(
				 *		double					ddwTriggerDelay=1.0,
				 *		axc_dword 				dwRemoteIp=0,
				 *		axc_word 				wRemotePort=0,
				 *		count char*				charRemoteMsg
				 *		}
				 * */

				if (axc_true == use_libcurl)
				{
					if (pTWD_new->set_remote_webaddr_for_sending_msg(t_Msg.Get()) != axc_true)
					{
						GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Monitor Manager] set_event_notify_config: fail to new notify event [%s]\n", t_Msg.Get());
					}

					//debug
					if (true){
						if(pTWD_new->IsUsingLibcurl() == axc_true){
							GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Monitor Manager] set_event_notify_config: restore new web addr. [%s]\n", pTWD_new->GetRemoteMsg());
						}else{
					    	//CHeatFinderUtility::PrintNowData(1);
							GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [Monitor Manager] set_event_notify_config: fail to restore web addr. [%s]\n", t_Msg.Get());
						}
					}
				}

				//add to global list
				if (true){
					// only support one address
					for(axc_i32 i=m_caxclistThermalTemperatureMonitor.GetCount()-1; i>=0; i--){
						HeatFinderMonitor4Temperature* pTWD = (HeatFinderMonitor4Temperature*) m_caxclistThermalTemperatureMonitor.GetID(i);
						if (pTWD->o_strncmp(t_Msg.Get()) != 0){
							GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Monitor Manager] set_event_notify_config: we only support one unique set. Remove the same configuration of list\n");

							m_caxclistThermalTemperatureMonitor.Remove(i);
							if((pTWD->Session)
									|| (pTWD->IsUsingLibcurl() == axc_true))
							{
								delete pTWD;
								pTWD = NULL;
							}

							break;
						}
					}

					const axc_ddword ddwId = (axc_ddword) pTWD_new;
					const int iIndex = m_caxclistThermalTemperatureMonitor.Add(ddwId, 0);
					if(iIndex < 0){
						l_iRst = 11; //EAGAIN: Resource temporarily unavailable
						if (l_bOnOffFeedBackMsg){
							CAxcString::snprintf(pcRetrunMsg, (axc_dword)sizeof(pcRetrunMsg), "restore watch list fail: %s", AxcGetLastErrorText());
						}
						if (axc_true == use_libcurl){
							GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Monitor Manager] set_event_notify_config: save this event into global list fail!\n");
						}
					}else{
						GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Monitor Manager] set_event_notify_config: save this event into global list success\n");
						l_iRst=0; //success
					}
				}// progame debug


				//twdMutex.Unlock();
				mQueueLock.unlock();
			}
			else
			{  //(live_or_dead != axc_true)
				//while(twdMutex.Lock() == axc_false)
				while(mQueueLock.try_lock() == false)
				{
					if (l_bOnOffExternalThread){
						CAxcTime::SleepMilliSeconds(20); //wait object unlock
					}else{
						usleep(20*1000);
					}
				}

				GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Monitor Manager] set_event_notify_config: search target event for closing ... \n");
				l_iRst=3; //ESRCH:No such process
				//search match event to destroy
				for(axc_i32 i=m_caxclistThermalTemperatureMonitor.GetCount()-1; i>=0; i--)
				{
					axc_bool  chk_command_legal = axc_true;
					HeatFinderMonitor4Temperature* pTWD = (HeatFinderMonitor4Temperature*) m_caxclistThermalTemperatureMonitor.GetID(i);

					if ((pTWD->IsUsingLibcurl() == axc_true)
							&& (pTWD->GetMsgLength() > 10))
					{
						// http://xxx 10 char.
						if (pTWD->o_strncmp(t_Msg.Get()) != 0)
						{
							chk_command_legal = axc_false;
						}
					}
					if(true) //match with the same ip and port
					{
						if(chk_command_legal == axc_true)
						{

							pTWD->SetWorkingStatus(false);

							GLog(tMonitorTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Monitor Manager] set_event_notify_config: closing success\n");
							l_iRst = 0; //success
						}else{
							m_caxclistThermalTemperatureMonitor.Remove(i);
						}
					}
				}// for loop
				if(l_iRst == 3)
				{
					l_iRst =0;
					if (l_bOnOffFeedBackMsg){
						CAxcString::strncpy(pcRetrunMsg, "OK", (axc_dword)sizeof(pcRetrunMsg));
					}
				}


				//twdMutex.Unlock();
				mQueueLock.unlock();
			}//(live_or_dead != axc_true)

		}// (rst == axc_true), command of create or destroy is legal
	}// (m_caxclistThermalTemperatureMonitor.GetCount() < m_caxclistThermalTemperatureMonitor.GetMaxItemCount())
	return l_iRst;
}

int CHeatfinderMonitor::OnChkTemperatureEvnetSetting(const char* pccReceivedTCPStr, const axc_dword ui32ReceivedTCPStrLength, CAxcThread* pThread, char* pcRetrunMsg, CAxcString &refcaxcstrNotifyStatus, CAxcString &refcaxcstrNotifyAddress)
{
	int l_iRst = -1;

	if (m_caxclistThermalTemperatureMonitor.GetCount() >0)
		l_iRst = 0;

	for(axc_i32 i=m_caxclistThermalTemperatureMonitor.GetCount()-1; i>=0; i--)
	{
		HeatFinderMonitor4Temperature* pTWD = (HeatFinderMonitor4Temperature*) m_caxclistThermalTemperatureMonitor.GetID(i);
		if(i < m_caxclistThermalTemperatureMonitor.GetCount()-1){
			refcaxcstrNotifyStatus.Append(";");
			refcaxcstrNotifyAddress.Append(";");
		}
		if ((NULL != strstr(pTWD->GetRemoteMsg(),"http"))
			&&(pTWD->IsWorking() == axc_true))
		{
			refcaxcstrNotifyStatus.Append("1");
			refcaxcstrNotifyAddress.AppendFormat("%s", pTWD->GetRemoteMsg());
			l_iRst = 1;
		}else{
			refcaxcstrNotifyStatus.Append("0");
			//refcaxcstrNotifyAddress.AppendFormat("%s", pTWD->GetRemoteMsg());
			refcaxcstrNotifyAddress.Append("Not legal");
		}
	}

	return l_iRst;

}
