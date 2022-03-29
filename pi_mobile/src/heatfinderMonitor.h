/*
 * heatfinderWatchdog.h
 *
 *  Created on: Jan 24, 2018
 *      Author: markhsieh
 */

#ifndef HEATFINDERWATCHDOG_H_
#define HEATFINDERWATCHDOG_H_

#include "globaldef.h"
#include <mutex>
#include <vector>
#include <semaphore.h>

class HeatFinderMonitor4Temperature
{
public:
	HeatFinderMonitor4Temperature()
	{
		m_wBindLocalPort =PORT_TCP_REMOTECTL;
		m_dwBindLocalIp =0;
		m_iSendBufferSize =64*1024; //the biggest size is 64k
		m_iRecvBufferSize =64*1024;
		m_temp_gate = 0.0;
		m_gpio_pin = -1;
		m_dwRemoteIp = 0;
		m_wRemotePort = 0;
		m_sizeThermalImage_LT.cx = m_sizeThermalImage_LT.cy = 0;
		m_sizeThermalImage_RB.cx = m_sizeThermalImage_RB.cy = 0;
		m_TriggerDelay = 1.0;
		Session = NULL;
		//Session->CAxcSocketTcpSession("temperature/watch_dog");

		m_axcbIsSendbyHttp = axc_false;
		m_axcbIsWorking = axc_false;
	}
	virtual ~HeatFinderMonitor4Temperature()
	{

		if(Session) { Session->Destroy(); delete Session; Session = NULL;}
	}

	//method
	axc_dword GetRemoteIp() { return m_dwRemoteIp; }
	axc_dword GetRemotePort() { return m_wRemotePort; }
	const char* GetRemoteMsg() { return m_Msg.Get(); }

	const char* GetCurlHttpAddrFile() { return m_MsgFilePath.Get(); }

	double GetTriggerDelay() { return m_TriggerDelay; }
	axc_dword GetMsgLength() { return m_Msg.GetLength(); }

	axc_bool IsUsingLibcurl() { return m_axcbIsSendbyHttp; }
	axc_bool IsWorking() { return m_axcbIsWorking; }
	void SetWorkingStatus(axc_bool axcbValue) { m_axcbIsWorking = axcbValue; }
	void SetWorkingStatus(bool bValue) { m_axcbIsWorking = ((bValue==true)?axc_true:axc_false); }

	axc_bool set_remote_target_for_sending_msg(
			double					ddwTriggerDelay,
			axc_dword 				dwRemoteIp,
			axc_word 				wRemotePort,
			const char*				charRemoteMsg
			)
	{
		if((dwRemoteIp == 0) || (wRemotePort == 0) || (ddwTriggerDelay < 1.0))
		{
			return axc_false;
		}
		m_TriggerDelay = ddwTriggerDelay;
		m_dwRemoteIp = dwRemoteIp;
		m_wRemotePort = wRemotePort;
		m_Msg.Format("%s", charRemoteMsg);

		return axc_true;
	}

	axc_bool set_remote_webaddr_for_sending_msg(
			const char*				charRemoteMsg = NULL
			)
	{
		if (charRemoteMsg == NULL)
		{
			m_axcbIsSendbyHttp = axc_false;
			return axc_false;
		}

		m_Msg.Format("%s", charRemoteMsg);
		m_axcbIsSendbyHttp = axc_true;
		return axc_true;
	}

	axc_bool set_remote_webaddr_save_path(
			const char*				charRemoteMsg = NULL
			)
	{
		if (charRemoteMsg == NULL)
		{
			m_axcbIsSendbyHttp = axc_false;
			return axc_false;
		}

		m_MsgFilePath.Format("%s", charRemoteMsg);
		m_axcbIsSendbyHttp = axc_true;
		return axc_true;
	}

	int o_strncmp(
			const char*				charCmp = NULL
			)
	{
		if (charCmp == NULL)
		{

			return -1;
		}

		return CAxcString::strncmp(m_Msg.GetBuffer(), charCmp, m_Msg.GetBufferSize());

	}

	CAxcSocketTcpSession 	*Session;
protected:
	//Tcp Session for send message to remote IP, which have static Port.
	//CreateClient()
	axc_word 				m_wBindLocalPort;
	axc_dword 				m_dwBindLocalIp;
	axc_i32 				m_iSendBufferSize;
	axc_i32 				m_iRecvBufferSize;
	axc_dword 				m_dwRemoteIp;
	axc_word 				m_wRemotePort;


	//Life delay time
	double					m_TriggerDelay;

	//temperature
	double					m_temp_gate;

	//using libcurl or not
	axc_bool 				m_axcbIsSendbyHttp;

	//is working
	axc_bool				m_axcbIsWorking;
	//send to global var, then MAIN.timer() will check module_gpio.GPIO_Timer
	axc_i32					m_gpio_pin;

	SIZE					m_sizeThermalImage_LT;
	SIZE					m_sizeThermalImage_RB;

	//message
	CAxcString 				m_Msg;
	CAxcString 				m_MsgFilePath;

};

class CHeatfinderMonitor
{
public:
	CHeatfinderMonitor();
	~CHeatfinderMonitor();

	bool Run();
	void Stop();

	/** SetTemperatureEvent4Monitor **
	 * only for tcp receive string.
	 *
	 * parameter:
	 * 		pccReceivedTCPStr, ui32ReceivedTCPStrLength: received TCP string
	 * 		pThread:	if has, default is NULL
	 * 		pcRetrunMsg: feed back the detail 'Operational situation'
	 *
	 * return:
	 * 		error code by integer
	 * 		0:	success
	 * 		22: EINVAL -- Invalid argument
	 * 		11: EAGAIN -- Resource temporarily unavailable (queue is full or other issue)
	 */
	int SetTemperatureEventToMonitor(const char* pccReceivedTCPStr=NULL, const axc_dword ui32ReceivedTCPStrLength=0, CAxcThread* pThread=NULL, char* pcRetrunMsg=NULL);

	/** GetTemperatureEventFromMonitor **
	 * only for tcp receive string.
	 *
	 * parameter:
	 * 		pccReceivedTCPStr, ui32ReceivedTCPStrLength: received TCP string
	 * 		pThread:	if has, default is NULL
	 * 		pcRetrunMsg: feed back the detail 'Operational situation'
	 *		refcaxcstrNotifyAddress: collect all notify address  -- "http://xxx.xxx:2x4/demo;http://ooo.ooo:77/gogo;NotLegal"  (if address no 'http' will return 'NotLegal')
	 *		refcaxcstrNotifyStatus: collect all notify status -- "1;0;0"
	 * return:
	 * 		error code by integer
	 * 		0:	success
	 * 		22: EINVAL -- Invalid argument
	 * 		-1: no enable notify
	 */
	int GetTemperatureEventFromMonitor(const char* pccReceivedTCPStr, const axc_dword ui32ReceivedTCPStrLength, CAxcThread* pThread, char* pcRetrunMsg, CAxcString &refcaxcstrNotifyStatus, CAxcString &refcaxcstrNotifyAddress);

	void OnChkAllEventAndTriggerNotify();
protected:
	std::mutex 	mQueueLock;
	CAxcList	m_caxclistThermalTemperatureMonitor;	// Gavin reviewed:
														// We may use HeatFinderMonitor4Temperature* m_psubscriber to replace entire manipulation.
	bool 	m_bNotify_was_sended;
	std::mutex mNotifyLock;

    CAxcThread m_hThread;       //g_threadTcpListen
    bool m_bStopThread;         //stop thread flag
    sem_t requestOutput;

    static axc_dword thread_process(CAxcThread* pThread, void* pContext)
    {
    	CHeatfinderMonitor *pSender = reinterpret_cast<CHeatfinderMonitor*> (pContext);
        return pSender->thread_process();
    }
    axc_dword thread_process();

	int OnAddTemperatureEvent(const char* pccReceivedTCPStr, const axc_dword ui32ReceivedTCPStrLength, CAxcThread* pThread, char* pcRetrunMsg);
	int OnChkTemperatureEvnetSetting(const char* pccReceivedTCPStr, const axc_dword ui32ReceivedTCPStrLength, CAxcThread* pThread, char* pcRetrunMsg, CAxcString &refcaxcstrNotifyStatus, CAxcString &refcaxcstrNotifyAddress);

	int OnChkTemperatureEventAndTriggerNotify();
};

#endif /* HEATFINDERWATCHDOG_H_ */
