#include "heatfinderapp.h"

CHeatFinderApp::CHeatFinderApp():
    m_bStopService(true),
    m_bIsReady2Reboot(false)
{
    CHeatFinderUtility::SetActiveApp(this);
    //CHeatFinderUtility::SetADECameraSDK(&m_adeCameraSDK);
    CHeatFinderUtility::SetGlobalsupport(&m_global);
    CHeatFinderUtility::SetTimerService(&m_timer);
    CHeatFinderUtility::SetConfigObj(&m_ConfigManager);
    //CHeatFinderUtility::SetLeptonSDK(&mLeptonSDK);

    m_sizeVisionMainImage.cx = 0;
    m_sizeVisionMainImage.cy = 0;

    m_sizeVisionSubImage.cx = 0;
    m_sizeVisionSubImage.cy = 0;

    m_sizeThermalImage.cx = 0;
    m_sizeThermalImage.cy = 0;

    m_bIsGpioSetReboot = false;
    m_ireSend = 0;

    m_iIsStatusFineCount = -1;
    m_fLastBadStatusTime =0;

    m_iDoubleCheck =0;
}

CHeatFinderApp::~CHeatFinderApp()
{
	m_bStopService = true;
}

/**
 * @brief CHeatFinderApp::exec
 * 最主要執行的程序
 * @return
 */
int CHeatFinderApp::exec()
{
	//FIXME: add pid and checking, witch will avoid multi-execution of the same process at the same time
	//CHeatFinderUtility::SetTwinkleLedShowStatusOK();
	OnLedStatusTwinkleNotify(true);

	//start log engine
    LogManager glog;

    GLog(tAll, tDEBUGTrace_MSK, "D: [%s] Hello ~~ Version: %s\n", SYS_EXECUTE_NICKNAME, SYS_EXECUTE_VERSION);
    AXC_U_VERSION  axcUVxValue = AxcGetLibVersion();
    GLog(tAll, tDEBUGTrace_MSK, "D: [%s] utility library (axclib) version: (%d) %d.%d\n", SYS_EXECUTE_NICKNAME, axcUVxValue.dwValue, axcUVxValue.Version.wMajor, axcUVxValue.Version.wMinor);
    if(!AXC_IS_VERSION_MATCH())
    {
		//CHeatFinderUtility::PrintNowData(1);
    	GLog(tAll, tERRORTrace_MSK, "E: [%s] axclib version not match!\n", SYS_EXECUTE_NICKNAME);
    	return -1;
    }

    bool bCamRst = m_ConfigManager.run();
    if(bCamRst == false) //1: true
	{
		//CHeatFinderUtility::PrintNowData(1);
		GLog(tAll, tERRORTrace_MSK, "E: [%s] %s\n", SYS_EXECUTE_NICKNAME, "open config manager fail.");
		CAxcTime::SleepMilliSeconds(20);
	}
	else
	{
		GLog(tAll, tDEBUGTrace_MSK, "D: [%s]  %s\n", SYS_EXECUTE_NICKNAME, "open config manager success.");
	}
    glog.Run(m_broadcastInformationCollector.getAppBuildTime());
    while (!m_ConfigManager.IsInitialComplete())
	{
    	usleep(500 * 1000);
    }

    // chk manual selection
    T_CONFIG_FILE_OPTIONS* currentConfig = m_ConfigManager.GetConfigContext();

    // create System Singal lister/manager
    bool SSM_Rst = m_systemSignalManager.Run();
    if (SSM_Rst == true){
    	GLog(tAll, tDEBUGTrace_MSK, "D: [%s]  %s\n", SYS_EXECUTE_NICKNAME, "open system signal listener success.");
    }else{
    	GLog(tAll, tWARNTrace_MSK, "W: [%s]  %s\n", SYS_EXECUTE_NICKNAME, "open system signal listener fail.");
    }
    m_systemSignalManager.AddAPPStatusChangeNotifyEvent(OnAPPStatusChangeNotify_FUNCTION, this);
    m_systemSignalManager.AddCaptureStatusChangeNotifyEvent(OnCaptureStatusChangeNotify_FUNCTION, this);

    // process watchdog
    m_iHeartBeastLoseTime = 0;
    m_watchdog.setHealthCheckInterval(25);
    m_watchdog.setLongestHeartBeatStopTime(75);
    m_watchdog.addProcessDeadNotifyEventHandler(OnProcessDeadNotifyEvent_func, this);
    m_watchdog.addWatchDogStopNotiyEventHandler(OnWatchDogStopNotiyEvent_func, this);
    bool bWatchDogRst = m_watchdog.Run();
    if (bWatchDogRst == false){
    	AxcSetLastError(0);
    	printf("N: [%s] Watch Dog Error: %s  \n", SYS_EXECUTE_NICKNAME, AxcGetLastErrorText());
    }

    // create global support: funsion ... value ...
    //m_global

    // create Lepton Capture/Analysis/Assign Event
    m_leptonCapture.AddDataReceivedEvent(OnThermalDataReceivedEvent_func, LEPTON_PKGSEND2_UDPMEDIACLIENT, this);
    m_leptonCapture.AddDataReceivedEvent(OnThermalDataReceived2PluginEvent, LEPTON_PKGSEND2_PLUGINTARGET, this);
    m_leptonCapture.AddLedStatusTwinkleNotifyEvent(OnLedStatusTwinkleNotify, this);
    m_leptonCapture.AddThermalSizeChangedNotifyEvent(OnThermalSizeChangedNotify, this);
    m_leptonCapture.AddDataPopedEvent(OnThermalDataPopedEvent_func, this);
    m_leptonCapture.AddReceivedThermalRestartEventNotify(OnReceivedThermalRestartFinishNotify, this);
    if ((m_watchdog.IsAcceptNewHeartBeatRecord()) &&
    		((currentConfig->iOpenStream & OPEN_THERMAL_STREAM)!=0))
    	m_watchdog.addHeartBeatRecord("Thermal", 16);

    GLog(tAll, tDEBUGTrace_MSK, "D: [%s] currentConfig->iOpenStream & OPEN_THERMAL_STREAM ==> %d\n", SYS_EXECUTE_NICKNAME, currentConfig->iOpenStream & OPEN_THERMAL_STREAM);

    // create Vision Capture
    //m_visionCapture.AddDataPopedEvent(OnVisionDataPopedEvent_func, this);
    m_visionCapture.AddDataReceivedEvent(OnVisionDataReceived2WhoEvent, this);
    m_visionCapture.AddLedStatusTwinkleNotify(OnLedStatusTwinkleNotify, this);
    m_visionCapture.AddVisionSizeChangedNotify(OnVisionSizeChangedNotify, this);
    m_visionCapture.AddRawDataReceivedEvent(OnReceivedCameraRawFrame_func, this);
    m_visionCapture.AddSetThermalOverlayNotify(OnSetThermalOverlay_func, this);
    m_visionCapture.AddVisionRestartFinishReceivedEvent(OnReceivedVisionRestartFinishNotify, this);
    if ((m_watchdog.IsAcceptNewHeartBeatRecord()) &&
    		((currentConfig->iOpenStream & OPEN_CAMERA_STREAM)!=0))
    	m_watchdog.addHeartBeatRecord("Vision", 16);

    GLog(tAll, tDEBUGTrace_MSK, "D: [%s] currentConfig->iOpenStream & OPEN_CAMERA_STREAM ==> %d\n", SYS_EXECUTE_NICKNAME, currentConfig->iOpenStream & OPEN_CAMERA_STREAM);

    // create Vision RawCapture
    //m_visionRawCapture.setOwner(this);
    //m_visionRawCapture.m_funcReceivedCameraRaw = OnReceivedCameraRawFrame_func;
    //m_visionRawCapture.m_funcSetThermalOverlay = OnSetThermalOverlay_func;
    //m_visionRawCapture.AddReceivedCameraRawNotify(OnReceivedCameraRawFrame_func, this);
    //m_visionRawCapture.AddSetThermalOverlayNotify(OnSetThermalOverlay_func, this);

    // create Broadcast Information Collector/Delivery
    /**
     * AddBroadcastInfoChangedNotifyEvent():
     * 			add to udp, tcp, plugIN(might be)
     */
    //tcpclient:
    //m_broadcastInformationCollector.AddBroadcastInfoChangedNotifyEvent();
    //udpsend:
    m_broadcastInformationCollector.SetSocketUDPEnalbeStatus(true);
    m_broadcastInformationCollector.SetdevType("AD-HF048-P");

    // create Tcp Lister
    m_tcpListener.AddBroadcastRenewNotifyEvent(OnTcpBroadcastRenewNotifyEvent_func, this);
    m_tcpListener.AddTcpAcceptSessionEvent(OnTcpAcceptSessionEvent_func, this);
    m_tcpListener.AddTcpNeedtoRemoveSessionEvent(OnTcpNeedtoRemoveSessionEvent_func, this);
    //m_tcpListener.AddSessionSendtoEvent(OnSessionSendtofunc_func, this);
    m_tcpListener.AddTcpReceivedUpgreadeEventNotify(OnReceivedUpgradeEvent_func, this);

    // create Tcp live view sender
    //m_tcpLiveviewSender.AddTcpSendedEventHandler(OnTcpSendedEvent_func, this);

    // create Udp command Receiver
    m_udpCommandReceiver.AddBroadcastRenewNotifyEvent(OnUdpBroadcastRenewNotifyEvent_func, this);

    // create Udp media-client Sender
    //m_udpMediaClientSender

    // create Udp view sender
    //m_udpViewSender {HF048-ML product need}
    GLog(tAll, tDEBUGTrace_MSK, "D: [%s] Udp view sender can memory & server %d session on list\n", SYS_EXECUTE_NICKNAME, m_udpViewSender.GetMaxUdpSession());

    // create timer
    bool l_timerRst = m_timer.SetPrintDebugMsg(false);
    GLog(tAll, tDEBUGTrace_MSK, "D: [%s] Timer print hide debug message: %s\n", SYS_EXECUTE_NICKNAME, ((l_timerRst == true)?"enable":"disable"));
    m_timer.SetInterval(10); //ms.
    m_timer.AddCheckFunctionOnTimeNotifyEvent(OnFuncChkModbusDNotify, 0.01, "Modbus check per 0.01 sec.", this);
    m_timer.AddCheckFunctionOnTimeNotifyEvent(OnFuncChkThermalIvsInfoNotify, 1.0, "Thermal IVSInfo. renew per 1 sec.", this);
    //m_timer.AddCheckFunctionOnTimeNotifyEvent(OnFuncChkENotify, 1.0, "Lepton can use re-chk per 1 sec.", this);
    m_timer.AddCheckFunctionOnTimeNotifyEvent(OnFuncChkLedStatusNotify, 3.0, "No Led zombie per 3 sec.", this);
    m_timer.AddCheckFunctionOnTimeNotifyEvent(OnFuncChkADEGPIOAliveNotify, 30.0, "ADE GPIO Alive? per 3 sec.", this);
    //m_timer.AddCheckFunctionOnTimeNotifyEvent(OnFuncChkRaspividNotify, 10.0, "Raspivid fine? per 10 sec.", this);

    // Integration
    {
    	GLog(tAll, tDEBUGTrace_MSK, "D: [%s]  connect broadcast information collector ---> tcp lister, udp command receiver broadcast information backup\n", SYS_EXECUTE_NICKNAME);
    	// contain (bool) CHeatFinderTcpListener::SetBroadcastInformationBackoff(CAxcString* caxcStrObj)
    	m_broadcastInformationCollector.AddBroadcastInfoChangedNotifyEvent(OnBroadcastInfoChangedNotify_func, this);
    }

    // run thread function:
    bool bleptoncatureRst = false;
    CAxcString leptoncatureErrstr;
    bool bvisioncatureRst = false;
    CAxcString visioncatureErrstr;
    //bool bvisionRawcapture = false;
    //CAxcString visionRawcaptureErrstr;
    bool bbroadcastInfocollectorRst = false;
    CAxcString broadcastInfoCollectorErrstr;
    int btcplistenerRst = 0;
    CAxcString tcplistenerErrstr;
    /**
    bool btcpsender = false;
    CAxcString tcpsenderErrstr;
    **/
    bool budpcommandreceiver = false;
    CAxcString udpcommandreceiverErrstr;
    bool budpmediaclientsender = false;
    CAxcString udpmediaclientsenderErrstr;
    bool budpviewsender = false;
    CAxcString udpviewsenderErrstr;
    bool bTimer = false;
    CAxcString TimerErrstr;

    //if(1 == iCamRst) //1: true
    if (bCamRst)
    {
    	AxcSetLastError(0);
    	bbroadcastInfocollectorRst = m_broadcastInformationCollector.Run();
    	if (bbroadcastInfocollectorRst == false){
    		broadcastInfoCollectorErrstr.Set(AxcGetLastErrorText());
    	}

#if !defined(DEF_SuppressThermal)
    	AxcSetLastError(0);
    	bleptoncatureRst = m_leptonCapture.Run();
    	if (bleptoncatureRst == false){
    		leptoncatureErrstr.Set(AxcGetLastErrorText());
    	}
#endif
#if !defined(DEF_SuppressVision)
    	AxcSetLastError(0);
        bvisioncatureRst = m_visionCapture.Run();
        //bvisioncatureRst = true;
        if (bvisioncatureRst ==false){
        	visioncatureErrstr.Set(AxcGetLastErrorText());
        }
#endif
    	//AxcSetLastError(0);
        //bvisionRawcapture =m_visionRawCapture.run();
        //if (bvisionRawcapture ==false){
        //	visionRawcaptureErrstr.Set(AxcGetLastErrorText());
		//}

    	AxcSetLastError(0);
    	btcplistenerRst = m_tcpListener.Run();
    	if (btcplistenerRst != NOERROR){
    		tcplistenerErrstr.Set(AxcGetLastErrorText());
    	}

    	/**
    	AxcSetLastError(0);
    	btcpsender = m_tcpLiveviewSender.Run();
    	if (btcpsender == false){
    		tcpsenderErrstr.Set(AxcGetLastErrorText());
    	}**/

    	AxcSetLastError(0);
    	budpcommandreceiver = m_udpCommandReceiver.Run();
    	//budpcommandreceiver = true;
    	if (budpcommandreceiver == false){
    		udpcommandreceiverErrstr.Set(AxcGetLastErrorText());
    	}

#if !defined(DEF_SuppressVision) || !defined(DEF_SuppressThermal)
    	AxcSetLastError(0);
    	budpmediaclientsender = m_udpMediaClientSender.Run();
    	//budpmediaclientsender = true;
    	if (budpmediaclientsender == false){
    		udpmediaclientsenderErrstr.Set(AxcGetLastErrorText());
		}
#endif

    	AxcSetLastError(0);
    	budpviewsender = m_udpViewSender.Run();
    	if (budpviewsender == false){
    		udpviewsenderErrstr.Set(AxcGetLastErrorText());
		}

    	AxcSetLastError(0);
    	//FIXME: shell not call below function single.
    	CHeatFinderUtility::SetGPIOSrvChkRecordInit();
    	bTimer = m_timer.Run();
    	if (bTimer == false){
    		TimerErrstr.Set(AxcGetLastErrorText());
    	}

    	m_bStopService = false;
    }else{
    	m_bStopService = true;
    }

    // print error
    if ((currentConfig->iOpenStream & OPEN_THERMAL_STREAM) == 0){
    	GLog(tAll, tDEBUGTrace_MSK, "D: [%s: thread]  lepton cature not selected\n", SYS_EXECUTE_NICKNAME);
	}else if (bleptoncatureRst){
    	GLog(tAll, tDEBUGTrace_MSK, "D: [%s: thread]  lepton cature success\n", SYS_EXECUTE_NICKNAME);
    }else{
		//CHeatFinderUtility::PrintNowData(1);
    	GLog(tAll, tERRORTrace_MSK, "E: [%s: thread]  lepton cature fail\n", SYS_EXECUTE_NICKNAME);
    	GLog(tAll, tERRORTrace_MSK, "E: [Error Detail] %s \n",leptoncatureErrstr.Get());
    }

    if ((currentConfig->iOpenStream & OPEN_CAMERA_STREAM) == 0){
        GLog(tAll, tDEBUGTrace_MSK, "D: [%s: thread]  vision cature not selected\n", SYS_EXECUTE_NICKNAME);
    }else if (bvisioncatureRst){
    	GLog(tAll, tDEBUGTrace_MSK, "D: [%s: thread]  vision cature success\n", SYS_EXECUTE_NICKNAME);
    }else{
		//CHeatFinderUtility::PrintNowData(1);
    	GLog(tAll, tERRORTrace_MSK, "E: [%s: thread]  vision cature fail\n", SYS_EXECUTE_NICKNAME);
    	GLog(tAll, tERRORTrace_MSK, "E: [Error Detail] %s \n",visioncatureErrstr.Get());
    }

    //if (bvisionRawcapture){
    //	printf("D: [%s: thread]  vision raw cature success\n", SYS_EXECUTE_NICKNAME);
    //}else{
    //	printf("E: [%s: thread]  vision raw cature fail\n", SYS_EXECUTE_NICKNAME);
    //	printf("E: [Error Detail] %s \n",visionRawcaptureErrstr.Get());
	//}

    if (bbroadcastInfocollectorRst){
    	GLog(tAll, tDEBUGTrace_MSK, "D: [%s: thread]  broadcast Information collector success\n", SYS_EXECUTE_NICKNAME);
    }else{
    	m_bStopService = true;
		//CHeatFinderUtility::PrintNowData(1);
    	GLog(tAll, tERRORTrace_MSK, "E: [%s: thread]  broadcast Information collector fail\n", SYS_EXECUTE_NICKNAME);
    	GLog(tAll, tERRORTrace_MSK, "E: [Error Detail] %s \n",broadcastInfoCollectorErrstr.Get());
    }

    if ((btcplistenerRst >=OPENPORT) || (btcplistenerRst<=PTHREAD2)){
		//CHeatFinderUtility::PrintNowData(1);
    }
    switch (btcplistenerRst){
    case OPENPORT:
    	m_bStopService = true;
    	GLog(tAll, tERRORTrace_MSK, "E: [%s: thread]  Tcp listener open port fail: %u\n", SYS_EXECUTE_NICKNAME, PORT_TCP_LISTEN);
    	GLog(tAll, tERRORTrace_MSK, "E: [Error Detail] %s \n",tcplistenerErrstr.Get());
    	break;
    case STARTLISTEN:
    	m_bStopService = true;
    	GLog(tAll, tERRORTrace_MSK, "E: [%s: thread]  Tcp listener start listen port fail\n", SYS_EXECUTE_NICKNAME);
    	GLog(tAll, tERRORTrace_MSK, "E: [Error Detail] %s \n",tcplistenerErrstr.Get());
    	break;
    case SESSIONLISTCREATE:
    	m_bStopService = true;
    	GLog(tAll, tERRORTrace_MSK, "E: [%s: thread]  Tcp listener create new element to session list fail\n", SYS_EXECUTE_NICKNAME);
    	GLog(tAll, tERRORTrace_MSK, "E: [Error Detail] %s \n",tcplistenerErrstr.Get());
    	break;
    case PTHREAD1:
    	m_bStopService = true;
    	GLog(tAll, tERRORTrace_MSK, "E: [%s: thread]  Tcp listener create thread 1 fail\n", SYS_EXECUTE_NICKNAME);
    	GLog(tAll, tERRORTrace_MSK, "E: [Error Detail] %s \n",tcplistenerErrstr.Get());
    	break;
    case PTHREAD2:
    	m_bStopService = true;
    	GLog(tAll, tERRORTrace_MSK, "E: [%s: thread]  Tcp listener create thread 2 fail\n", SYS_EXECUTE_NICKNAME);
    	GLog(tAll, tERRORTrace_MSK, "E: [Error Detail] %s \n",tcplistenerErrstr.Get());
    	break;
    default:
    	GLog(tAll, tDEBUGTrace_MSK, "D: [%s: thread]  Tcp listener start listen success\n", SYS_EXECUTE_NICKNAME);
    	break;
    }

    /**
    if (btcpsender){
    	printf("D: [%s: thread]  Tcp Live view Sender create success\n", SYS_EXECUTE_NICKNAME);
    }else{
    	m_bStopService= true;
    	printf("E: [%s: thread]  Tcp Live view Sender create fail\n", SYS_EXECUTE_NICKNAME);
    	printf("E: [Error Detail] %s \n",tcpsenderErrstr.Get());
	}**/

    if (budpcommandreceiver){
    	GLog(tAll, tDEBUGTrace_MSK, "D: [%s: thread]  udp command receiver create success\n", SYS_EXECUTE_NICKNAME);
    }else{
    	m_bStopService = true;
    	//CHeatFinderUtility::PrintNowData(1);
    	GLog(tAll, tERRORTrace_MSK, "E: [%s: thread]  udp command receiver create fail: %u\n", SYS_EXECUTE_NICKNAME, PORT_UDP_COMMANDSRV);
    	GLog(tAll, tERRORTrace_MSK, "E: [Error Detail] %s \n",udpcommandreceiverErrstr.Get());
    }

	if (budpmediaclientsender){
    	GLog(tAll, tDEBUGTrace_MSK, "D: [%s: thread]  udp media-client sender create success\n", SYS_EXECUTE_NICKNAME);
	}else{
#if !defined(DEF_SuppressVision) || !defined(DEF_SuppressThermal)
		m_bStopService = true;
#endif
    	//CHeatFinderUtility::PrintNowData(1);
    	GLog(tAll, tERRORTrace_MSK, "E: [%s: thread]  udp media-client sender create fail\n", SYS_EXECUTE_NICKNAME);
    	GLog(tAll, tERRORTrace_MSK, "E: [Error Detail] %s \n",udpmediaclientsenderErrstr.Get());
	}

	if (budpviewsender){
    	GLog(tAll, tDEBUGTrace_MSK, "D: [%s: thread]  udp view sender create success\n", SYS_EXECUTE_NICKNAME);
	}else{
    	m_bStopService = true;
    	//CHeatFinderUtility::PrintNowData(1);
    	GLog(tAll, tERRORTrace_MSK, "E: [%s: thread]  udp view sender create fail: %u\n", SYS_EXECUTE_NICKNAME, PORT_UDP_VIEWSEND);
    	GLog(tAll, tERRORTrace_MSK, "E: [Error Detail] %s \n",udpviewsenderErrstr.Get());
	}

	if (bTimer){
		GLog(tAll, tDEBUGTrace_MSK, "D: [%s: thread] timer create success\n", SYS_EXECUTE_NICKNAME);
	}else{
    	m_bStopService = true;
    	//CHeatFinderUtility::PrintNowData(1);
    	GLog(tAll, tERRORTrace_MSK, "E: [%s: thread]  timer create fail\n", SYS_EXECUTE_NICKNAME);
    	GLog(tAll, tERRORTrace_MSK, "E: [Error Detail] %s \n",TimerErrstr.Get());
	}

    //hamlet
	char cPluginPath[64] = "/home/pi/heat_finder/ex-support/plugin";
    int iPluginCount = m_pluginManager.LoadPluginFromPath(cPluginPath);
    if (iPluginCount > 0){
        GLog(tAll, tDEBUGTrace_MSK, "D: [%s] Plug in manager load success: collect %d plug-in elements \n", SYS_EXECUTE_NICKNAME, iPluginCount);
        m_pluginManager.Run();
        m_leptonCapture.SetSend2PluginPausePlay(true);
        m_visionCapture.SetRawSend2PluginStatus(true);

        m_leptonCapture.AddHeatObjectReceivedNotifyEvent(OnThermalHeatObjectReceivedEvent_func, this);
        if(!m_leptonCapture.setIntervalOfAnalyzeHeatObjectWhenIdle(1.0)){
        	GLog(tAll, tWARNTrace_MSK, "W: [%s] Plug in manager: lepton Capture support method 'setIntervalOfAnalyzeHeatObjectWhenIdle' fail\n", SYS_EXECUTE_NICKNAME);
        }
    }else{
        GLog(tAll, tDEBUGTrace_MSK, "D: [%s] Plug in manager load finish: not any plug-in elements \n", SYS_EXECUTE_NICKNAME);
        m_leptonCapture.SetSend2PluginPausePlay(false);
        m_visionCapture.SetRawSend2PluginStatus(false);
    }
    m_pluginManager.GetPlugInInformation();
    char cVersionInfomations[1024];
    m_pluginManager.CopyPlugInInformation(cVersionInfomations, 1024);
    m_broadcastInformationCollector.updatePluginInfomations(cVersionInfomations, 1024);

    //xiongyi@ade.tw Modbus listen service  //Hank--
	// if(!ModbusUart_Init())  
	// {
    // 	//CHeatFinderUtility::PrintNowData(1);
	// 	GLog(tAll, tERRORTrace_MSK, "E: [Main] failed to start modbus-uart server !");
	// }
	if(!ModbusTcp_Init())
	{
    	//CHeatFinderUtility::PrintNowData(1);
		GLog(tAll, tERRORTrace_MSK, "E: [Main] failed to start modbus-tcp server !");
	}

	//FIXME: print pthread record by timer pthread changed checker
	m_global.OnPrintfpThreadRecord();
    while(!m_bStopService)
    {
        //do something
        usleep(144 * 1000);
    }
    printf("................... start finish steps\n");

    //kill collector
    m_broadcastInformationCollector.Stop();
    m_watchdog.Stop();

    //kill Modbus service
	// ModbusUart_Uninit();  //Hank--
	ModbusTcp_Uninit();

    //kill communication interface
    m_tcpListener.Stop();
    //m_tcpLiveviewSender.Stop();
    m_udpCommandReceiver.Stop();
    m_udpMediaClientSender.Stop();
    m_udpViewSender.Stop();
    printf("network close finish\n");

    //kill view
    m_leptonCapture.Stop();
    m_visionCapture.Stop();
    //m_visionRawCapture.stop();

    //kill signal reader
    m_systemSignalManager.Stop();

    //CHeatFinderUtility::SetTwinkleLedShowStatusWrong();
    OnLedStatusTwinkleNotify(false);
    GLog(tAll, tWARNTrace_MSK,  "W: [%s] %s\n", SYS_EXECUTE_NICKNAME, "close camera device and leave.");

    //glog.Stop();
    return 0;
}


void CHeatFinderApp::make_thermal_ivs_result_string(CAxcString& cszSend)
{
	m_global.OnCollectThermalMsgToString(cszSend);
}

void CHeatFinderApp::OnLedStatusTwinkleNotify(const bool value)
{
	//FIXME: send event to timer, avoid event too much. (timer will service the different frequency with event appending and done)
	if (value == false){
		if(m_iIsStatusFineCount > -1){
			m_iIsStatusFineCount -=1;
		}
		if (m_iIsStatusFineCount < 0){
			float _fCurrentTime = CAxcTime::GetCurrentUTCTimeMs();
			if ((_fCurrentTime - m_fLastBadStatusTime) > 1.0){
				m_fLastBadStatusTime = CAxcTime::GetCurrentUTCTimeMs();
			}else{
				return;  //avoid too busy for led configuration change.
			}
		}

	}else{
		if (m_iIsStatusFineCount < 0){
			m_iIsStatusFineCount +=1;
		}else{
			return;
		}
	}

	//if (value == true){
	if (m_iIsStatusFineCount >= 0){
		CHeatFinderUtility::SetTwinkleLedShowStatusOK();
	}else{
		CHeatFinderUtility::SetTwinkleLedShowStatusWrong();
	}
}

void CHeatFinderApp::OnThermalSizeChangedNotify(const unsigned int Width, const unsigned int Height)
{
    GLog(tAll, tDEBUGTrace_MSK, "D: [%s] Thermal Size Changed: %d x %d\n", SYS_EXECUTE_NICKNAME, Width, Height);
    //renew resolution of thermal view
    m_sizeThermalImage.cx = Width;
    m_sizeThermalImage.cy = Height;
    //FIXME: global might cause race condition delay
    //m_global.OnRenewThermalResolution(m_sizeThermalImage.cx, m_sizeThermalImage.cy);

    //broadcast add event
    //thermal
    struct BroadcastValueRenewHandler l_broadcastValueRenew_handler;
    memset(&l_broadcastValueRenew_handler, 0, sizeof(l_broadcastValueRenew_handler));
    l_broadcastValueRenew_handler.type = BroadcastValueRenew_Type_Thermal;
    l_broadcastValueRenew_handler.action = BroadcastValueRenew_Action_Resize;
    l_broadcastValueRenew_handler.w =Width;
    l_broadcastValueRenew_handler.h =Height;
    m_broadcastInformationCollector.AddBroadcastInfoRenewNotifyEvent(l_broadcastValueRenew_handler, this);

}

void CHeatFinderApp::OnThermalDataReceivedEvent_func(axc_byte *pBuf, const axc_i32 size, const axc_ddword channelIndex){
    //printf("N: [%s] Thermal Data Revceived: %d\n", SYS_EXECUTE_NICKNAME, size);
    if (size > 0){
    	// don't add data event to tcp client {Foremark, T-Guard, Vms.}

    	// add data event to udp send {media-server}
    	if (channelIndex != ADE2CAM_CHANNEL_LEPTONRAW){
    		GLog(tAll, tWARNTrace_MSK, "W: [%s] Thermal Data Revceived: len:%d data not belong this re-direction\n", SYS_EXECUTE_NICKNAME, size);
    	}
    	/**
    	 * FIXME: memory leak??
    	 */
    	m_udpMediaClientSender.OnRecivedSendMsgThermal(pBuf, size, channelIndex);
    	//m_udpMediaClientSender.OnReceivedDirectSend(pBuf, size, channelIndex);

    	T_CONFIG_FILE_OPTIONS* currentConfig= m_ConfigManager.GetConfigContext();
        if ((m_watchdog.IsAcceptNewHeartBeatRecord()) &&
        		((currentConfig->iOpenStream & OPEN_THERMAL_STREAM)!=0))
        	m_watchdog.addHeartBeatRecord("Thermal", 16);
    }
}

void CHeatFinderApp::OnThermalDataReceived2PluginEvent(axc_byte *pBuf, const axc_i32 size, const axc_ddword channelIndex){
    //printf("N: [%s] Thermal Data Revceived and Send to Plug-in interface: %d\n", SYS_EXECUTE_NICKNAME, size);
	if (m_pluginManager.GetPluginCount() <= 0){
		//printf("D: %s: %s no element\n", SYS_EXECUTE_NICKNAME, __func__);
		return;
	}

	int iErrorNum = 0;
    if (size > 0) {
    	// add data event to plug-in client
    	//printf("D: %s: %s has %u data length and plugin count: %u\n", SYS_EXECUTE_NICKNAME, __func__, size, m_pluginManager.GetPluginCount());

        //hamlet
    	if ((m_sizeThermalImage.cx == 80) && (m_sizeThermalImage.cy == 80)) {
    		iErrorNum = m_pluginManager.SetReceivedThermalFrame(80, 80, false, (int)size, (short*)pBuf);
    		//iErrorNum = m_pluginManager.AddReceivedThermalFrame(80, 60, false, (int)size, (short*)pBuf);
        } else if ((m_sizeThermalImage.cx != 0) && (m_sizeThermalImage.cy != 0) &&
        		  ((THERMAL_SUPPORT_GROUP & THERMAL_SOURCE_DEV_FLIR_LWIR) == 0)) {
        	GLog(tAll, tWARNTrace_MSK, "W: [%s] size issue happen: w:%d h:%d, please check the support list of thermal source.\n", SYS_EXECUTE_NICKNAME, m_sizeThermalImage.cx, m_sizeThermalImage.cy);

        	iErrorNum = m_pluginManager.SetReceivedThermalFrame(m_sizeThermalImage.cx, m_sizeThermalImage.cy, false, (int)size, (short*)pBuf);
        	//iErrorNum = m_pluginManager.AddReceivedThermalFrame(m_sizeThermalImage.cx, m_sizeThermalImage.cy, false, (int)size, (short*)pBuf);
        }

    	if (iErrorNum != 0) {
    		GLog(tAll, tERRORTrace_MSK, "E: [%s] some issue happen: %s\n", SYS_EXECUTE_NICKNAME, strerror(iErrorNum));
    	}
    }
}

void CHeatFinderApp::OnThermalDataPopedEvent_func(axc_byte *pBuf, const axc_i32 size){
	//printf("N: [%s] Thermal Data from virtual fifo Poped: %d\n", SYS_EXECUTE_NICKNAME, size);
	if (size > 0){

		//add data event to tcp client {Foremark, T-Guard, Vms.}
		//m_tcpLiveviewSender.OnRecivedSendMsg(pBuf, size, TCPSENDER_OUTPUT_THERMAL);

		m_udpViewSender.OnRecivedSendMsg(pBuf, size, UDPSENDER_OUTPUT_THERMAL);
	}
}

void CHeatFinderApp::OnReceivedThermalRestartFinishNotify(){
	GLog(tAll, tDEBUGTrace_MSK, "D: [%s] %s ---> start streaming watch dog\n",SYS_EXECUTE_NICKNAME, __func__);

	T_CONFIG_FILE_OPTIONS* currentConfig= m_ConfigManager.GetConfigContext();

	m_watchdog.setHealthCheckInterval(25);
	if ((m_watchdog.IsAcceptNewHeartBeatRecord()) &&
	    ((currentConfig->iOpenStream & OPEN_THERMAL_STREAM)!=0))
		m_watchdog.addHeartBeatRecord("Thermal", 16);
}

/**
 * #define VISION_H264PKGSEND2_TCPPORT5555 (0)
 * #define VISION_H264PKGSEND2_UDPPORT5557 (1)
 * #define VISION_H264PKGSEND2_ENCODER (2)
 * #define VISION_H264PKGSEND2_MEDIASRV (3)
 * #define VISION_H264PKGSEND2_PLUGIN (4)
 * #define VISION_RAWPKGSEND2_PLUGIN (5)
 */
void CHeatFinderApp::OnVisionDataReceived2WhoEvent(axc_byte *pBuf, const axc_i32 size, const unsigned short channelIndex){
    //printf("N: [%s] Vision Data Revceived len: %d\n", SYS_EXECUTE_NICKNAME, size);
    if (size > 0){
    	if (channelIndex == VISION_H264PKGSEND2_MEDIASRV){
    		//printf("N: [%s] Vision Data Revceived len: %d to mediasrv\n", SYS_EXECUTE_NICKNAME, size);
    		if(0)
    		{
    			static FILE* fp = fopen("/home/pi/Downloads/output_app.h264", "wb");
    			if(fp)
    			{
    				fwrite(pBuf, 1, size, fp);
    			}
    			//return;
    		}

    		//m_udpMediaClientSender.OnReceivedDirectSend(pBuf, size, ADE2CAM_CHANNEL_VISUALMAIN);
    		m_udpMediaClientSender.OnRecivedSendMsg(pBuf, size, ADE2CAM_CHANNEL_VISUALMAIN);

    		T_CONFIG_FILE_OPTIONS* currentConfig= m_ConfigManager.GetConfigContext();
    	    if ((m_watchdog.IsAcceptNewHeartBeatRecord()) &&
    	    		((currentConfig->iOpenStream & OPEN_CAMERA_STREAM)!=0))
    	    	m_watchdog.addHeartBeatRecord("Vision", 16);

    	}else if (channelIndex == VISION_H264PKGSEND2_PLUGIN){
    		//FIXME
    	}else if (channelIndex == VISION_H264PKGSEND2_ENCODER){

    	}else{
    		GLog(tAll, tNORMALTrace_MSK, "N: [%s] Vision Data Revceived to ?: %u\n", SYS_EXECUTE_NICKNAME, channelIndex);
    	}
    }
}

void CHeatFinderApp::OnVisionDataPopedEvent_func(axc_byte *pBuf, const axc_i32 size){
	//printf("N: [%s] Vision Data from virtual fifo Poped: %d\n", SYS_EXECUTE_NICKNAME, size);
	if (size > 0){
		//add data event to tcp client {Foremark, T-Guard, Vms.}
		//printf("N: [%s] Vision Data from virtual fifo Poped: %d to tcp liveview sender & udp view sender\n", SYS_EXECUTE_NICKNAME, size);
		if(0)
		{
			static FILE* fp = fopen("/home/pi/input-appvision.h264", "wb");
			if(fp)
			{
				fwrite(pBuf, 1, size, fp);
			}
			//return;
		}

		//m_tcpLiveviewSender.OnRecivedSendMsg(pBuf, size, TCPSENDER_OUTPUT_VISION);
		//m_tcpLiveviewSender.OnRecivedVisionSendMsg(pBuf, size, TCPSENDER_OUTPUT_VISION);

		m_udpViewSender.OnRecivedSendMsg(pBuf, size, UDPSENDER_OUTPUT_VISION);
	}
}

void CHeatFinderApp::OnVisionSizeChangedNotify(const unsigned int Width, const unsigned int Height){
    GLog(tAll, tDEBUGTrace_MSK, "D: [%s] Vision Size Changed: %d x %d\n", SYS_EXECUTE_NICKNAME, Width, Height);
    //renew resolution of thermal view
    m_sizeVisionMainImage.cx = Width;
    m_sizeVisionMainImage.cy = Height;
    //FIXME: global might cause race condition delay
    m_global.OnRenewVisionResolution(m_sizeVisionMainImage.cx, m_sizeVisionMainImage.cy);

    //broadcast add event
    //thermal
    struct BroadcastValueRenewHandler l_broadcastValueRenew_handler;
    memset(&l_broadcastValueRenew_handler, 0, sizeof(l_broadcastValueRenew_handler));
    l_broadcastValueRenew_handler.type = BroadcastValueRenew_Type_Vision;
    l_broadcastValueRenew_handler.action = BroadcastValueRenew_Action_Resize;
    l_broadcastValueRenew_handler.w =Width;
    l_broadcastValueRenew_handler.h =Height;
    m_broadcastInformationCollector.AddBroadcastInfoRenewNotifyEvent(l_broadcastValueRenew_handler, this);
}

void CHeatFinderApp::OnAPPStatusChangeNotify_FUNCTION(const unsigned int flag, const unsigned int action, const unsigned int time_interval){
	GLog(tAll, tDEBUGTrace_MSK, "D: [%s] App Status Change Comply \n",SYS_EXECUTE_NICKNAME);
	if (flag == 0){
		//action: don't care
		GLog(tAll, tDEBUGTrace_MSK, "D: [%s] EXIT \n",SYS_EXECUTE_NICKNAME);
		m_bStopService = true;
	}else{
		GLog(tAll, tDEBUGTrace_MSK, "D: [%s] Unknown: {%u, %u, time interval:%u} \n",SYS_EXECUTE_NICKNAME, flag, action, time_interval);
	}
}

void CHeatFinderApp::OnCaptureStatusChangeNotify_FUNCTION(const unsigned int flag, const unsigned int action, const unsigned int time_interval){
	GLog(tAll, tDEBUGTrace_MSK, "D: [%s] Capture Status Change Comply \n",SYS_EXECUTE_NICKNAME);
	bool chk_capturestatuschange = false;
	if (flag ==0){
		if(action==1){
			chk_capturestatuschange = true;

		}else if(action==2){
			chk_capturestatuschange = true;

		}else if(action==3){
			chk_capturestatuschange = true;

		}else{
	    	//CHeatFinderUtility::PrintNowData(1);
			GLog(tAll, tERRORTrace_MSK, "E: [%s] vision action:%d not support \n",SYS_EXECUTE_NICKNAME, action);
		}
	}else if (flag ==1){
		struct ThermalStatusChangedEvent obj_statuschange_event;
		memset(&obj_statuschange_event, 0, sizeof(obj_statuschange_event));
		if(action==1){
			chk_capturestatuschange = true;
			obj_statuschange_event.pauseplay = false;
			m_leptonCapture.AddThermalStatusChangedNotifyEvent(obj_statuschange_event, this);
		}else if(action==2){
			chk_capturestatuschange = true;
			obj_statuschange_event.pauseplay = true;
			m_leptonCapture.AddThermalStatusChangedNotifyEvent(obj_statuschange_event, this);
		}else{
	    	//CHeatFinderUtility::PrintNowData(1);
			GLog(tAll, tERRORTrace_MSK, "E: [%s] vision action:%d not support \n",SYS_EXECUTE_NICKNAME, action);
		}
	}else{
		GLog(tAll, tDEBUGTrace_MSK, "D: [%s] Unknown: {%u, %u, time interval:%u} \n",SYS_EXECUTE_NICKNAME, flag, action, time_interval);
	}

	if (chk_capturestatuschange == true){
		GLog(tAll, tDEBUGTrace_MSK, "D: [%s] Capture Status Change Send Event Success\n",SYS_EXECUTE_NICKNAME);
	}
}

void CHeatFinderApp::OnUdpBroadcastRenewNotifyEvent_func(){
	//printf("D: [%s] Udp Command Receiver: need newest device message for responsing broadcast\n",SYS_EXECUTE_NICKNAME);
	// fire broadcast renew notify event
	BroadcastValueRenewHandler broadcastRenewObj;
	memset(&broadcastRenewObj, 0, sizeof(broadcastRenewObj));

	broadcastRenewObj.type = BroadcastValueRenew_Type_Renew;
	m_broadcastInformationCollector.AddBroadcastInfoRenewNotifyEvent(broadcastRenewObj, this);
}

void CHeatFinderApp::OnTcpBroadcastRenewNotifyEvent_func(){
	//printf("D: [%s] Tcp Listenr: need newest device message for responsing broadcast\n",SYS_EXECUTE_NICKNAME);
	// fire broadcast renew notify event
	BroadcastValueRenewHandler broadcastRenewObj;
	memset(&broadcastRenewObj, 0, sizeof(broadcastRenewObj));

	broadcastRenewObj.type = BroadcastValueRenew_Type_Renew;
	m_broadcastInformationCollector.AddBroadcastInfoRenewNotifyEvent(broadcastRenewObj, this);
}

void CHeatFinderApp::OnTcpAcceptSessionEvent_func(CAxcSocketTcpSession *pValue, const unsigned int luValue){
	//twinkle network led: mean App using network now.
	CHeatFinderUtility::SetTwinkleLedShowNetworkUsed();

	//if (bValue == ThisCommandIsVideoOut){
	if (luValue >= 0){
		char szIp[64] = {""};
		CAxcSocket::IpToString(pValue->GetRemoteIp(), szIp);
		char szType[64] = {""};
		if ((luValue & 0x01) > 0){
			CAxcString::strcat(szType, "vision ");
		}
		if ((luValue & 0x02) > 0){
			CAxcString::strcat(szType, "thermal ");
		}
		if (luValue == 0x00){
			CAxcString::strcat(szType, "remove ");
		}

		GLog(tAll, tDEBUGTrace_MSK, "D: [%s] Tcp Listenr: get a video out [%s] session [IP: %s, Port: %u], pass to tcp client\n",SYS_EXECUTE_NICKNAME, szType, szIp, pValue->GetRemotePort());
		//tcp client add pSession

		//xTcpSessionConfiguration newObj;
		//newObj.pValue = pValue;
		//newObj.output_type = luValue;
		//m_tcpLiveviewSender.AddTcpSessionConfiguration(newObj);

		//pValue->Destroy();
	}
}

void CHeatFinderApp::OnTcpNeedtoRemoveSessionEvent_func(CAxcSocketTcpSession *pValue){
	char szIp[64] = {""};
	CAxcSocket::IpToString(pValue->GetRemoteIp(), szIp);
	GLog(tAll, tNORMALTrace_MSK, "N: [%s] Tcp Listenr: this video out session [IP: %s, Port: %u] shell be destroyed\n",SYS_EXECUTE_NICKNAME, szIp, pValue->GetRemotePort());

	//debug call back
}

int CHeatFinderApp::OnSessionSendtofunc_func (CAxcList *listSessionSend, CAxcList *listSessionDrop){
	//char szIp[64] = {""};
	//CAxcSocket::IpToString(pSession->GetRemoteIp(), szIp);
	//printf("N: [%s] Tcp Listenr: send data by session [IP: %s, Port: %u]\n",SYS_EXECUTE_NICKNAME, szIp, pSession->GetRemotePort());

	axc_i32 iSendPkgRst = 0;
	//iSendPkgRst = m_tcpLiveviewSender.OnSessionSendto(listSessionSend, listSessionDrop);

	return iSendPkgRst;
}

// tcp client/view sender
void CHeatFinderApp::OnTcpSendedEvent_func(){
	CHeatFinderUtility::SetTwinkleLedShowNetworkUsed();
}

void CHeatFinderApp::OnBroadcastInfoChangedNotify_func(CAxcString value){
	if (value.GetLength() < 1){
    	//CHeatFinderUtility::PrintNowData(1);
		GLog(tAll, tERRORTrace_MSK, "E: [%s] broadcast renew string length is small than 1 double word (%d) \n",SYS_EXECUTE_NICKNAME, value.GetLength());
		return;
	}else{
		//printf("D: [%s] broadcast renew string notify event forward to tcp-listener\n",SYS_EXECUTE_NICKNAME);
	}

	//printf("send broadcast message to tcp listener\n");
	//tcp listener:
	m_tcpListener.SetBroadcastInformationBackoff(&value);

	//printf("send broadcast message to udp command receiver\n");
	//udp command receiver:
	m_udpCommandReceiver.SetBroadcastInformationBackoff(&value);
}

int CHeatFinderApp::OnReceivedUpgradeEvent_func (double dDelaytimestampMS){
	int l_iInterval = (int)(dDelaytimestampMS*1000);

	CHeatFinderUtility::SetGPIORebootStep();
	CAxcTime::SleepMilliSeconds(l_iInterval);
	return 1;
}

void CHeatFinderApp::OnFuncChkADEGPIOAliveNotify(const double dCurrentTimeMS, const unsigned int uiAction, const signed int siValue){
	//printf("D: [%s] %s\n",SYS_EXECUTE_NICKNAME, __func__);
	if (m_bIsGpioSetReboot == true){
		GLog(tAll, tDEBUGTrace_MSK, "D: [%s] %s GPIO Srv. reboot and need to know Capturing alive\n",SYS_EXECUTE_NICKNAME, __func__);

		m_ireSend +=1;
	}else if (m_bStopService){
		//pass this time
		return;
	}

	if (CHeatFinderUtility::IsGPIOSrvAlive() == false){
		//CHeatFinderUtility::PrintNowData(1);
		GLog(tAll, tNORMALTrace_MSK, "N: [%s] %s GPIO Serivce not alive\n",SYS_EXECUTE_NICKNAME, __func__);
		CHeatFinderUtility::SetGPIOSrvRestart();
		m_bIsGpioSetReboot = true;
		//CHeatFinderUtility::SetTwinkleLedShowStatusON();
	}else{
		if (m_ireSend >= 2){
			m_bIsGpioSetReboot = false; //mean it was alive or reboot success.
			m_ireSend =0;
			OnLedStatusTwinkleNotify(true);
		}
		//printf("N: [%s] %s GPIO Serivce chk once :%lf\n",SYS_EXECUTE_NICKNAME, __func__, dCurrentTimeMS);
		CHeatFinderUtility::SendGPIOSrvChkNotify();
	}
}
void CHeatFinderApp::OnFuncChkThermalIvsInfoNotify(const double dCurrentTimeMS, const unsigned int uiAction, const signed int siValue){
	//printf("D: [%s] %s\n",SYS_EXECUTE_NICKNAME, __func__);

	GlobalDef *pGlobal = (GlobalDef *) CHeatFinderUtility::GetGlobalsupport();
	//ade_camera_sdk *pADECamSDK = (ade_camera_sdk *) CHeatFinderUtility::GetADECameraSDK();
	T_THERMAL_IVS_OUTPUT *pThermalIVSOut = pGlobal->GetThermalIvsOutSavingTable();

	int l_iHeatObjCount = 0;
	int l_iRenewConfig = 0;

    try {
    	// ONLY Update at here!!!!
        //get thermal heat points locatation and temperature
    	//pADECamSDK->get_thermal_ivs_out(pGlobal->GetThermalIvsOutSavingTable());
    	l_iRenewConfig = pGlobal->RenewThermalCurrentConfiguration();

        if (pGlobal->GetThermalCurrentHeatObjSeekStatus() > 0) {
        	//l_iHeatObjCount = pGlobal->GetThermalCurrentFoundHeatObj();
        	l_iHeatObjCount = (int)pThermalIVSOut->wHeatObj;
        } else {
        	l_iHeatObjCount = 0;
        }
    } catch(...) {
        l_iHeatObjCount = 0;
        GLog(tAll, tWARNTrace_MSK, "W: [%s] cannot get thermal heat object information\n", __func__);
    }

    if (l_iHeatObjCount > 0) {
    	CHeatFinderUtility::SetTwinkleLedShowHeatAlarm();

    	// chat-bot: alert event: checking per 'OnFuncChkThermalIvsInfoNotify' sec.
    	// control the interval between two alerm event create for chat-bot cloud, default is '6' seconds.
    	// if (pGlobal->GetThermalCurrentHeatObjSeekStatus() > 0) than only send '1' time. <--- this rule must write into the 'mADEChatBot.ReceivingAlermEvent'
    	// mADEChatBot.ReceivingAlermEvent(double [current ms time]);s
    }

    if (l_iRenewConfig < 1) {
    	//CHeatFinderUtility::PrintNowData(1);
    	GLog(tAll, tERRORTrace_MSK, "E: [%s] Read Thermal Config. Fail:%s\n", __func__, ((l_iRenewConfig < 0)?"Config. 'adehf.ini' not exist or wrong location":"read fail (contain: file empty or too large)"));
    }
}

/**
 * chk: /tmp/raspi2
 * 1. init. fail or dual request deny
 * [init. fail]
 * 		raspivid: no process found
 * 		mmalipc: mmal_vc_init_fd: could not open vchiq service
 * 		mmal: mmal_vc_component_create: failed to initialise mmal ipc for 'vc.ril.camera' (7:EIO)
 * 		mmal: mmal_component_create_core: could not create component 'vc.ril.camera' (7)
 * [dual request deny] ... busy
 * 		mmal: mmal_vc_component_enable: failed to enable component: ENOSPC
 * 		mmal: camera component couldn't be enabled
 *
 * act: raspivid
 * [init. fail]
 * 		check again, count two will reboot machine
 * [... busy]
 * 		try to killall raspivid
void CHeatFinderApp::OnFuncChkRaspividNotify(const double dCurrentTimeMS, const unsigned int uiAction, const signed int siValue){
	printf("D: [%s] %s\n",SYS_EXECUTE_NICKNAME, __func__);
	int l_iChkRpvdErrtype = -1;
	GlobalDef *pGlobal = (GlobalDef *) CHeatFinderUtility::GetGlobalsupport();
	int RpvdErrcount[3] = { 0, };

	//if (diff_T_chkvision >= 10)
	{

		if (pGlobal->GetVisioLostRestartStatus() == true){
			l_iChkRpvdErrtype = CHeatFinderUtility::Utility_ChkRaspivid_strerr(RpvdErrcount);
		}else{
			l_iChkRpvdErrtype = -1;
		}

		if ((l_iChkRpvdErrtype == 1) && (RpvdErrcount[l_iChkRpvdErrtype] >=1)){
			printf("W: [%s]: !!!!!!Raspivid had Critical issue: get %d typical warnings!!!!!!! \n", __func__, RpvdErrcount[l_iChkRpvdErrtype]);

			if (m_bIsReady2Reboot == false){
				m_bIsReady2Reboot = true;
			}else{
				printf("W: [%s]: !!!!!!%s will start reboot step !!!!!!!!  \n", __func__, DEVICE_TYPE);
				//system("sudo killall -s SIGUSR2 ade_gpio_set &");
				CHeatFinderUtility::SetGPIORebootStep();
			}
		}else if (l_iChkRpvdErrtype != -1){
			printf("W: [%s]: open raspivid err: %s :detect %d time\n", __func__, (l_iChkRpvdErrtype ==2)?"system busy":"please check /tmp/raspi2", RpvdErrcount[l_iChkRpvdErrtype]);
		}else{
			m_bIsReady2Reboot = false;
		}
		//last_T_chkvision = CAxcTime::GetCurrentUTCTime32();
	}

}
**/

void CHeatFinderApp::OnFuncChkLedStatusNotify(const double dCurrentTimeMS, const unsigned int uiAction, const signed int siValue){
	//printf("D: [%s] %s\n",SYS_EXECUTE_NICKNAME, __func__);
	if (m_iIsStatusFineCount < 0){
		float _fCurrentTime = CAxcTime::GetCurrentUTCTimeMs();
		if ((_fCurrentTime - m_fLastBadStatusTime) > 10.0){
			m_iIsStatusFineCount = 0;
			m_fLastBadStatusTime = _fCurrentTime;
			CHeatFinderUtility::SetTwinkleLedShowStatusOK();

			//CHeatFinderUtility::PrintNowData(1);
			GLog(tAll, tWARNTrace_MSK, "W: [%s] %s: led bad status setting timeout 10 seconds\n",SYS_EXECUTE_NICKNAME, __func__);
		}
	}
}

void CHeatFinderApp::OnFuncChkModbusDNotify(const double dCurrentTimeMS, const unsigned int uiAction, const signed int siValue){
	//printf("D: [%s] %s\n",SYS_EXECUTE_NICKNAME, __func__);
	// ModbusUart_Timer();  //Hank--
	ModbusTcp_Timer();

}
void CHeatFinderApp::OnFuncChkENotify(const double dCurrentTimeMS, const unsigned int uiAction, const signed int siValue){
	//printf("D: [%s] %s\n",SYS_EXECUTE_NICKNAME, __func__);
	/*
	CadeSDKAPIleptonthermal *pLepSDK = (CadeSDKAPIleptonthermal *)CHeatFinderUtility::GetLeptonSDK();
	axc_bool rst=axc_false;
	rst = (pLepSDK->clickedThermalFFC()==true)?axc_true:axc_false;
	if (rst!=axc_true){
    	//CHeatFinderUtility::PrintNowData(1);
		GLog(tAll, tERRORTrace_MSK, "E: [%s] %s clickedThermalFFC fail\n",SYS_EXECUTE_NICKNAME, __func__);
	}
	*/
	GlobalDef *pGlobal = (GlobalDef *) CHeatFinderUtility::GetGlobalsupport();
	if (pGlobal->GetLeptonIsOnline()){
		CHeatFinderUtility::SetTwinkleLedShowNetworkUsed();
	}
}

int CHeatFinderApp::OnReceivedCameraRawFrame_func(const int Width, const int Height, const int RawFMT, const int size, unsigned char *pRGB){
	//printf("N: [%s] %s  %u---> send data to plug-in Raw Data receive Interface (w: %d, h: %d, size: %d, format:%d)\n"
	//		,SYS_EXECUTE_NICKNAME, __func__, (CAxcTime::GetCurrentUTCTime32()), Width, Height, size, RawFMT);
	if (m_pluginManager.GetPluginCount() <= 0){
		return 1;
	}
    //hamlet
	if(0)
	{
		axc_dword iTime = CAxcTime::GetCurrentUTCTime32();
		char name[128];
		sprintf(name, "/home/pi/input-appvision-%u.rgb", iTime);
		FILE* fp = fopen(name, "wb");
		if(fp)
		{
			fwrite(pRGB, 1, size, fp);
		}
		fclose(fp);
		//return;
	}
    m_pluginManager.SetReceivedRGBFrame(Width, Height, RawFMT, size, pRGB);
	return 1;
}

int CHeatFinderApp::OnSetThermalOverlay_func(xLeptonOverlay &xLoi){
	//printf("D: [%s] %s  ---> send overlay new information to plug-in Interface\n",SYS_EXECUTE_NICKNAME, __func__);
	if (m_pluginManager.GetPluginCount() <= 0){
		return 0;
	}
	GLog(tAll, tDEBUGTrace_MSK, "D: [%s] %s ---> send overlay new information to plug-in Interface\n",SYS_EXECUTE_NICKNAME, __func__);
	return m_pluginManager.SetThermalOverlay(xLoi);
	//return 1;
}

void CHeatFinderApp::OnReceivedVisionRestartFinishNotify(){
	GLog(tAll, tDEBUGTrace_MSK, "D: [%s] %s ---> start streaming watch dog\n",SYS_EXECUTE_NICKNAME, __func__);

	T_CONFIG_FILE_OPTIONS* currentConfig= m_ConfigManager.GetConfigContext();

	m_watchdog.setHealthCheckInterval(25);
	if ((m_watchdog.IsAcceptNewHeartBeatRecord()) &&
	    ((currentConfig->iOpenStream & OPEN_CAMERA_STREAM)!=0))
		m_watchdog.addHeartBeatRecord("Vision", 16);
}

void CHeatFinderApp::OnProcessDeadNotifyEvent_func(char *szStr, int isize, double losttime){
	char _szStr[64];
	memset(_szStr, '\0', sizeof(_szStr));
	bool bAlreadyTimeout = false;
	T_CONFIG_FILE_OPTIONS* currentConfig= m_ConfigManager.GetConfigContext();

	if (isize > 64){
		GLog(tAll, tERRORTrace_MSK, "E: [%s] %s --- notify name outof range.\n",SYS_EXECUTE_NICKNAME, __func__);
	}

	//valgrind say: Process terminating with default action of signal 11 (SIGSEGV), Access not within mapped region at address 0x0.
	if (isize > 64){
		strncpy(_szStr, szStr, 64);
	}else{
		strncpy(_szStr, szStr, isize);
	}
#if !defined(DEF_SuppressThermal)
	if( (currentConfig->iOpenStream & OPEN_THERMAL_STREAM)!=0 && 0 == strcmp(_szStr,"Thermal") ) {
		GLog(tAll, tWARNTrace_MSK, "W: [%s] %s --- '%s' lose heart beat too long time. iOpenStream=>%dL\n",SYS_EXECUTE_NICKNAME, __func__, _szStr, currentConfig->iOpenStream);
	}
#endif
#if !defined(DEF_SuppressVision)
	if( (currentConfig->iOpenStream & OPEN_CAMERA_STREAM)!=0 && 0 == strcmp(_szStr,"Vision") ) {
		GLog(tAll, tWARNTrace_MSK, "W: [%s] %s --- '%s' lose heart beat too long time. iOpenStream=>%dL\n",SYS_EXECUTE_NICKNAME, __func__, _szStr, currentConfig->iOpenStream);
	}
#endif
#if !defined(DEF_SuppressThermal)
	if( (currentConfig->iOpenStream & OPEN_THERMAL_STREAM)!=0 &&
		(0 == strncmp(_szStr, "Thermal", isize)) &&
		(losttime >= THERMAL_TIMEOUT_LIMITATION) ){
		bAlreadyTimeout = true;
	}
#endif
#if !defined(DEF_SuppressVision)
	if( (currentConfig->iOpenStream & OPEN_CAMERA_STREAM)!=0 &&
		(0 == strncmp(_szStr, "Vision", isize)) &&
		(losttime >= VISION_TIMEOUT_LIMITATION) ){
		bAlreadyTimeout = true;
	}
#endif

	if(bAlreadyTimeout){
		system("touch /tmp/adehf_watchdog_restart`date +%s`");
		GLog(tAll, tDEBUGTrace_MSK, "D: [%s] EXIT \n",SYS_EXECUTE_NICKNAME);
		m_bStopService = true;
	}
}

void CHeatFinderApp::OnWatchDogStopNotiyEvent_func(){
	GLog(tAll, tDEBUGTrace_MSK, "D: [%s] %s --- Suspend monitoring \n",SYS_EXECUTE_NICKNAME, __func__);
}

void CHeatFinderApp::OnThermalHeatObjectReceivedEvent_func(xPluginHeatObject *pHeatObjs){

	//printf("V: %s\n", __func__);
	m_pluginManager.SetReceivedHeatObject(pHeatObjs);

}
