#include "heatfindervisioncapture.h"
#include "ade_camera_sdk.h"
#include "XStreamParse.h"
#include "heatfinderconfigmanager.h"

CHeatFinderVisionCapture::CHeatFinderVisionCapture():
    m_hThread("vision/keeplive"),
    m_bStopThread(true),
    m_fLastFrameTimeMs(0.0),
    m_bRestartVision(false),
    m_bPrintVisionStatus(true),
    m_bDataSend2Udp_enable(true),
    m_bChkFirstFrameisI_afterDataSend2UdpOffOn(false),
    m_bChkFirstFrameisIEnable(true),
    m_bh264send2encoder(false),
    m_brawsend2plugin(false),
    //m_bReservationDataSend2UDP(false),
    m_memRPICamFrameRework("rpi/rework"),
    //m_hThreadReadVectorList("vision/readframe"),
    m_hThreadReadMem("vision/readmem"),
    m_memoryCache(4*1024*1024)
{
    //m_fifoNetSend("net/vision_data_queue"),
    //m_hfifoThread("vision/pop_queue"),
    memset(m_abyH264Sps, 0, sizeof(m_abyH264Sps));
    memset(m_abyH264Pps, 0, sizeof(m_abyH264Pps));
    m_pvs_sizeVisionMainImage= {0,0};

    m_iRawDataWidth =0;
    m_iRawDataHeight =0;
    m_fLastRawControlTimeMs =0.0;

    m_iH264SPSLen =0;
    m_iH264PPSLen =0;
    m_bRawEncodeRestart = false;
    m_bFrameNotCollectDone =false;
    m_IPFrameCollectedLength =0;
}

CHeatFinderVisionCapture::~CHeatFinderVisionCapture()
{
    GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [%s] Try to Destroy 1\n", "Vision Capture");
    m_bStopThread = true;
    if (m_hThread.IsValid() == axc_true)
        m_hThread.Destroy(2000);
    GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [%s] Try to Destroy 2\n", "Vision Capture");
    //if (m_hThreadReadVectorList.IsValid() == axc_true)
    //	m_hThreadReadVectorList.Destroy(2000);

	if (m_hThreadReadMem.IsValid() == axc_true)
		m_hThreadReadMem.Destroy(2000);

    m_memRPICamFrameRework.Free();

    m_RPICameraFW.CloseCamera();

    GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [%s] Destroy\n", "Vision Capture");
}

bool CHeatFinderVisionCapture::Run()
{
	fpsH264Frame.Reset(); //gavin ++

	bool bRst = false;
    Stop();

    GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [%s] %s\n", "Vision Capture", __func__);
    m_RPICameraFW.AddH264ReceivedEvent(OnAnalysisH264PKG, this);
    m_RPICameraFW.AddRawReceivedEvent(OnReceivedRawData, this);

    //setDefaultRPICamera();
    if(!m_memRPICamFrameRework.Create(256*1024)){		// Gavin: Refer to raspberryPIcamera::s_frameBuff definition
    	//not even used...
    	//return bRst;
    	fprintf(stderr, "E: [%s] %s  m_memRPICamFrameRework Fail\n", "Vision Capture", __func__);
    }else{
    	printf("D: [%s] %s  m_memRPICamFrameRework ok\n", "Vision Capture", __func__);
    }

    if (!setVisioParameter2RPICamera(true, true)){
    	//CHeatFinderUtility::PrintNowData(1);
    	GLog(tCaptureVisionTrace, tERRORTrace_MSK, "E: [%s] Set booting Parameter to RPI Camera FW Fail ---- when Start\n", "Vision Capture");
    }

    if (0 != m_RPICameraFW.OpenCamera()){
		Cheatfinderconfigmanager *pConfigContextManager = (Cheatfinderconfigmanager *)CHeatFinderUtility::GetConfigObj();
		T_CONFIG_FILE_OPTIONS *pConfigContext = ((pConfigContextManager == NULL)?NULL:pConfigContextManager->GetConfigContext());
		if((pConfigContext->iOpenStream & OPEN_CAMERA_STREAM) == 0)
		{
			GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [%s] raspberryPIcamera suspended by adehf.ini open_stream configuration.\n", "Vision Capture");
		}
		else
		{
			GLog(tCaptureVisionTrace, tERRORTrace_MSK, "E: [%s] %s m_RPICameraFW Fail\n", "Vision Capture", __func__);
		}
		return bRst;
    }
    if (!setVisioParameter2RPICamera(false, true)){
    	//CHeatFinderUtility::PrintNowData(1);
    	GLog(tCaptureVisionTrace, tERRORTrace_MSK, "E: [%s] Set run time Parameter to RPI Camera FW Fail ---- when Start\n", "Vision Capture");
    }
    //printf("D: [%s] %s m_RPICameraFW ok\n", "Vision Capture", __func__);

    // RPI SDK Monitor: make sure I/P frame exist, which can check health.
    m_bStopThread = false;
    if (!m_hThread.Create(thread_process, this))
        return bRst;
    //printf("D: [%s] %s thread_process pass\n", "Vision Capture", __func__);

    if (!m_hThreadReadMem.Create(thread_process_ReadDataFromMEM, this))
    	return bRst;
    //printf("D: [%s] %s thread_process_ReadDataFromMEM pass\n", "Vision Capture", __func__);

    GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [%s] Start\n", "Vision Capture");
    bRst = true;
    return bRst;
}

void CHeatFinderVisionCapture::Stop()
{
    GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [%s] Try to stop 1\n", "Vision Capture");
    m_bStopThread = true;


    if (m_hThread.IsValid() == axc_true)
        m_hThread.Destroy(2000);

    //if (m_hThreadReadVectorList.IsValid() == axc_true)
    //	m_hThreadReadVectorList.Destroy(2000);

    if (m_hThreadReadMem.IsValid() == axc_true)
    	m_hThreadReadMem.Destroy(2000);

    m_memRPICamFrameRework.Free();

    if (m_RPICameraFW.IsRunning())
    	m_RPICameraFW.CloseCamera();

    GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [%s] Stop\n", "Vision Capture");
}

void CHeatFinderVisionCapture::SetRestartVision(const bool value)
{
	m_locker.lock();
    m_bRestartVision = value;
    m_locker.unlock();
}

void CHeatFinderVisionCapture::SetIsDataSend2UDP(const bool value)
{
	m_data2udp_locker.lock();
	//because vision capture will pass to 'raspivid',
	//pause start can not just 'control' by here.

	if (value == false){
		m_chkFirstFrameType_locker.lock();
		m_bChkFirstFrameisI_afterDataSend2UdpOffOn = true;
		m_bChkFirstFrameisIEnable = true;
		m_bDataSend2Udp_enable = false;

		//FIXME
		GLog(tCaptureVisionTrace, tWARNTrace_MSK, "W: [Vision Capture] Send UDP message stop signal to any plug-in\n");
		xLeptonOverlay xLoi;
		memset(&xLoi, 0, sizeof(xLoi));
		fireSetThermalOverlayNotify(xLoi);

		m_chkFirstFrameType_locker.unlock();
	}else{
		//m_bReservationDataSend2UDP = true;
		m_bRawEncodeRestart = true;

	}


	m_data2udp_locker.unlock();
}

void CHeatFinderVisionCapture::PrintVisionStatus(const bool value)
{
   m_bPrintVisionStatus = value;
}

void CHeatFinderVisionCapture::SetH264Send2encoderStatus(const bool bValue){
	if (m_bh264send2encoder != bValue)
		m_bh264send2encoder = bValue;
}
bool CHeatFinderVisionCapture::GetH264Send2encoderStatus(){
	return m_bh264send2encoder;
}

void CHeatFinderVisionCapture::SetRawSend2PluginStatus(const bool bValue){
	if (m_brawsend2plugin != bValue)
		m_brawsend2plugin = bValue;
}
bool CHeatFinderVisionCapture::GetRawSend2PluginStatus(){
	return m_brawsend2plugin;
}

void CHeatFinderVisionCapture::sendDefaultConfigEventNotify(){
	Cheatfinderconfigmanager *pConfigManager = (Cheatfinderconfigmanager *)CHeatFinderUtility::GetConfigObj();
	pConfigManager->SetConfigContextAsDefault();
}
void CHeatFinderVisionCapture::sendImportConfigEventNotify(){
	Cheatfinderconfigmanager *pConfigManager = (Cheatfinderconfigmanager *)CHeatFinderUtility::GetConfigObj();
	pConfigManager->ImportConfigContextFromFile(CONFIGURATION_FILE);
}

void CHeatFinderVisionCapture::AddVisionSizeChangedNotify(OnVisionSizeChangedNotify fnEvent, void *pContext)
{
	xVisionSizeChangedNotifyHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;

    m_visionsizechangenotifyhandlerList.push_back(handler);
}

void CHeatFinderVisionCapture::AddLedStatusTwinkleNotify(OnLedStatusTwinkleNotify fnEvent, void *pContext)
{
	xVLedStatusTwinkleNotifyHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;

    m_ledstatustwinklenotifyhandlerList.push_back(handler);
}

void CHeatFinderVisionCapture::AddDataReceivedEvent(OnVDataReceivedEvent fnEvent, void *pContext)
{
	xVDataReceivedEventHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;

    m_datareceivedeventList.push_back(handler);
}

void CHeatFinderVisionCapture::AddRawDataReceivedEvent(OnRawDataReceivedEvent fnEvent, void *pContext){

	xRawDataReceivedEventHandler handler;
	handler.fnEvent = fnEvent;
	handler.pContext = pContext;
	m_rawdatareceivedhandleList.push_back(handler);
}

void CHeatFinderVisionCapture::AddSetThermalOverlayNotify(OnSetThermalOverlay fnEvent, void *pContext)
{
	struct tagSetOnTimeThermalOverlayNotifyHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;

    m_SetThermalOverlaynotifyhandlerList.push_back(handler);
}

void CHeatFinderVisionCapture::AddVisionRestartFinishReceivedEvent(OnVisionRestartFinishNotifyEvent fnEvent, void* pContext){
	xVisionRestartFinishNotifyEventHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;

    m_VisionRestartFinishNotifyEventListener.push_back(handler);
}

bool CHeatFinderVisionCapture::restartRPIEncoder(unsigned short iType){
	bool ok = false;
	unsigned short _itype = iType;

	/** 2.1.1.12 start: update selection of open stream */
	Cheatfinderconfigmanager *pConfigManager = (Cheatfinderconfigmanager *)CHeatFinderUtility::GetConfigObj();
	T_CONFIG_FILE_OPTIONS* currentConfig = pConfigManager->GetConfigContext();
	if ((currentConfig->iOpenStream & OPEN_CAMERA_STREAM) == 0) {
		return true;
	}
	/** 2.1.1.12 end */

	switch (_itype) {
	case RPI_RESTART_CONF_RELOAD:
		sendDefaultConfigEventNotify();
		sendImportConfigEventNotify();
		break;
	case RPI_RESTART_CONF_RENEW:
		sendImportConfigEventNotify();
		break;
	case RPI_RESTART_CONF_NOCHANGE:
	default:
		break;
	}

    SetIsDataSend2UDP(false);
    usleep(200000);
    if (true) {
    	m_RPICameraFW.CloseCamera();
    }

    if (!setVisioParameter2RPICamera(true, true)) {
    	ok = false;
    	GLog(tCaptureVisionTrace, tERRORTrace_MSK, "E: [%s] Set Parameter to RPI Camera FW Fail ---- at %s\n", "Vision Capture", "auto reload action");
    } else {
        ok = ((m_RPICameraFW.OpenCamera() == 0)? true: false);
    	setVisioParameter2RPICamera(false, true);

    	/**
    	 * static XREF_T  initial_map[] =
			{
			   {(char *)"record",     0},
			   {(char *)"pause",      1},
			};
    	 */
	  	Cheatfinderconfigmanager *pConfigContextManager = (Cheatfinderconfigmanager *)CHeatFinderUtility::GetConfigObj();
	  	T_CONFIG_FILE_OPTIONS *pConfigContext = ((pConfigContextManager == NULL)?NULL:pConfigContextManager->GetConfigContext());
	  	if ((pConfigContext->iOpenStream & OPEN_CAMERA_STREAM) == 0) {
	  		GLog(1, tDEBUGTrace_MSK, "D: [%s][Gavin] ... vision capture suppressed\n", "Vision Capture");
	  		m_RPICameraFW.SetCapturingState(0);
	  	} else {
	  		GLog(1, tDEBUGTrace_MSK, "D: [%s][Gavin] ... vision capture start\n", "Vision Capture");
	  		m_RPICameraFW.SetCapturingState(1);
	  	}
    }
    return ok;
}

axc_dword CHeatFinderVisionCapture::thread_process()
{
    GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [%s: thread] %s enter\n", "Vision Capture", __func__);
    void *pContext = CHeatFinderUtility::GetGlobalsupport();
    GlobalDef *pGlobal = reinterpret_cast<GlobalDef*> (pContext);
    pGlobal->AddpThreadRecord("Vision Capture");

    GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [%s: thread] %s auth\n", "Vision Capture", __func__);
    //init.

    axc_bool bIsVideoLost = axc_false;
    double fVideoLostTimeMs = 0;

    double fVideoLastRestartTimeMs = 0.0;
    axc_bool reload_adehf_config = axc_false;


    m_fLastFrameTimeMs = CAxcTime::GetCurrentUTCTimeMs()*1000;
    fVideoLastRestartTimeMs = CAxcTime::GetCurrentUTCTimeMs()*1000;

    double fConfigLastChkTimeMs = 0.0;
    axc_ddword ddwConfigLastModifiedTimeMs = 0;
    axc_ddword ddwConfigNowModifiedTimeMs = 0;

    fConfigLastChkTimeMs = CAxcTime::GetCurrentUTCTimeMs()*1000;

    struct stat attrib;
    stat("/home/pi/ade_camera_udp_out/adehf.ini", &attrib);
    ddwConfigLastModifiedTimeMs = attrib.st_ctime;

    //double fReservationDataSend2UDPChkTimeMs = fConfigLastChkTimeMs;  //obeyed by fConfigLastChkTimeMs system

    GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [%s: thread] %s init.\n", "Vision Capture", __func__);

    while(!m_bStopThread) // && fdVisionFifo >= 0)
    {
    	if (m_RPICameraFW.IsRunning() != true){
    		m_hThread.SleepMilliSeconds(100);
    		continue;
    	}

        if(((CAxcTime::GetCurrentUTCTimeMs()*1000) - fConfigLastChkTimeMs) > 3000)
        {
            struct stat attrib_now;
            stat("/home/pi/ade_camera_udp_out/adehf.ini", &attrib_now);
            ddwConfigNowModifiedTimeMs = attrib_now.st_ctime;

                if (ddwConfigLastModifiedTimeMs != ddwConfigNowModifiedTimeMs)
                {
                    ddwConfigLastModifiedTimeMs = ddwConfigNowModifiedTimeMs;
                    //m_fLastFrameTimeMs -= 10000;   // force to restart rspivid
                    reload_adehf_config = axc_true;
                    GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [%s] video need restart for new configuration\n", "Vision Capture");
                }
            fConfigLastChkTimeMs = CAxcTime::GetCurrentUTCTimeMs()*1000;

        }

        if (((m_bRestartVision == true)||(reload_adehf_config == axc_true)) &&
                (((CAxcTime::GetCurrentUTCTimeMs()*1000) - fVideoLastRestartTimeMs) > 5000))
        {
            if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/tmp/raspi2"))) {
                CAxcFileSystem::RemoveFile("/tmp/raspi2");
            }

            axc_bool ok = axc_false;
            GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [%s] video manual restart \n", "Vision Capture");
            //renew all default ... this action will also help thermal process get new configuration data.
            if (reload_adehf_config == axc_true)
            {
            	ok = (restartRPIEncoder(RPI_RESTART_CONF_RELOAD) == true)?axc_true:axc_false;
                reload_adehf_config = (reload_adehf_config == axc_true)? axc_false: reload_adehf_config;

            }else if (m_bRestartVision == true){

            	ok = (restartRPIEncoder(RPI_RESTART_CONF_RENEW) == true)?axc_true:axc_false;
            }

            if(!bIsVideoLost)
            {
                bIsVideoLost = axc_true;
                fVideoLostTimeMs = m_fLastFrameTimeMs;
                GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [%s] video manual restart %s\n", "Vision Capture", ok ? "ok" : "failed");
            }
            m_fLastFrameTimeMs = CAxcTime::GetCurrentUTCTimeMs()*1000;
            pGlobal->SetVisioLostRestartStatus(true);
        }
        // no data still 10 seconds, restart capture-app & warning after data lost 7 seconds
        else if(((CAxcTime::GetCurrentUTCTimeMs()*1000) - m_fLastFrameTimeMs) > 7000)
        {
        	Cheatfinderconfigmanager *pConfigContextManager = (Cheatfinderconfigmanager *)CHeatFinderUtility::GetConfigObj();
        	T_CONFIG_FILE_OPTIONS *pConfigContext = ((pConfigContextManager == NULL)?NULL:pConfigContextManager->GetConfigContext());
        	if((pConfigContext->iOpenStream & OPEN_CAMERA_STREAM)!=0)
        	{
        		fireLedStatusTwinkleNotify(false);
				//CHeatFinderUtility::PrintNowData(1);
				float _fdiff = ((CAxcTime::GetCurrentUTCTimeMs()*1000) - m_fLastFrameTimeMs)/1000;
				if (((_fdiff >= 7.0) && (_fdiff <7.2)) ||
					((_fdiff >= 8.0) && (_fdiff <8.2)) ||
					((_fdiff >= 9.0) && (_fdiff <9.2)))
                    GLog(tCaptureVisionTrace, tWARNTrace_MSK, "W: [%s] lose vision stream above %.02f seconds\n", "Vision Capture", _fdiff);

				if (_fdiff < 10.0){
					m_hThread.SleepMilliSeconds(100);
					continue;

				}else if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/tmp/raspi2"))) {
					CAxcFileSystem::RemoveFile("/tmp/raspi2");
				}

				GLog(tCaptureVisionTrace, tWARNTrace_MSK, "W: [%s] video lost need restart \n", "Vision Capture");

				axc_bool ok = axc_false;
				ok = (restartRPIEncoder(RPI_RESTART_CONF_NOCHANGE) == true)?axc_true:axc_false;

				if(!bIsVideoLost)
				{
					bIsVideoLost = axc_true;
					fVideoLostTimeMs = m_fLastFrameTimeMs;
					GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [%s] video lost %.1fs, restart %s\n", "Vision Capture", CAxcTime::GetCurrentUTCTimeMs() - (m_fLastFrameTimeMs/1000.0), ok ? "ok" : "failed");
				}

				m_fLastFrameTimeMs = CAxcTime::GetCurrentUTCTimeMs()*1000;

				//avoid raspivid hung (camera capturing was frozen)
				// todo callback to restart pi
				pGlobal->SetVisioLostRestartStatus(true);
        	}
        	else
        	{
            	fireLedStatusTwinkleNotify(true);

        		m_hThread.SleepMilliSeconds(1000);
				m_fLastFrameTimeMs = CAxcTime::GetCurrentUTCTimeMs()*1000;
        	}
        }

        // video restore
        if(bIsVideoLost)
        {
            bIsVideoLost = axc_false;
            GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [%s] video restore (lost %.1fs)\n", "Vision Capture", CAxcTime::GetCurrentUTCTimeMs() - (fVideoLostTimeMs/1000.0));
            fVideoLastRestartTimeMs = CAxcTime::GetCurrentUTCTimeMs()*1000;

            // Program will make sure udp send is active when visio capturing restart
            if (m_bDataSend2Udp_enable == false){
            	m_data2udp_locker.lock();
            	m_bDataSend2Udp_enable = true;
            	//m_bReservationDataSend2UDP = false;
            	m_data2udp_locker.unlock();
            }

            //CHeatFinderUtility::SetTwinkleLedShowStatusON();
        	fireLedStatusTwinkleNotify(true);
            SetIsDataSend2UDP(true);
            fireVisionRestartFinishNotify();

            /**avoid raspivid hung (camera capturing was frozen)
             *
             * Because read null will do 'continue', flag 'g_isVisioLostRestart' will change to false when got value.
             */
            m_bRestartVision = (m_bRestartVision == true)? false:m_bRestartVision;
            pGlobal->SetVisioLostRestartStatus(false);
        }else{

        }
        m_hThread.SleepMilliSeconds(100);
		continue;
    }
    GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [%s: thread] %s finish\n", "Vision Capture", __func__);
    return 1;
}

void CHeatFinderVisionCapture::OnAnalysisH264PKG(axc_byte* pData, axc_dword dwDataSize)
{
	//try to make the data more smaller than pure capturing view.
	//GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [%s: thread] %s ... len: %d\n", "Vision Capture", __func__, dwDataSize);

	/**
	 * below restore to MEM. as Data
	 */
	if ( !m_memoryCache.push_back_len(dwDataSize, pData) ){
		GLog(tCaptureVisionTrace, tWARNTrace_MSK, "W: [%s] Queue already full\n", __func__);
		m_memoryCache.Reset();
		m_memoryCache.push_back_len(dwDataSize, pData);
	}
}

axc_dword CHeatFinderVisionCapture::thread_process_ReadDataFromMEM(){
    GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [%s: thread] %s init.\n", "Vision Capture", __func__);

    void *pContext = CHeatFinderUtility::GetGlobalsupport();
    GlobalDef *pGlobal = reinterpret_cast<GlobalDef*> (pContext);
    pGlobal->AddpThreadRecord("Vision Distributor");

    // gavin ++ >>
    const int cFrameBuffSize = DEF_FRAME_BUFFER_SIZE;
    axc_byte receiveBuff[4+cFrameBuffSize];
    LPSRingBuffElement pElement = (LPSRingBuffElement)receiveBuff;
    while(!m_bStopThread)
    {
    	pElement->dwLength = cFrameBuffSize;
    	if( m_memoryCache.pop_front(pElement) )
    	{
			if(pElement->dwLength > 0)
			{
				OnReceivedH264Frame(pElement->data, pElement->dwLength);
			}
			else
			{
				m_hThreadReadMem.SleepMilliSeconds(20);
			}
    	}
    	else
    	{
    		GLog(tAll, tDEBUGTrace_MSK, "D: Vision Capture: +=====================================================+\nVision Capture: |!!!!  thread_process_ReadDataFromMEM loss frame  !!!!|\nVision Capture: +=====================================================+\n\n\n");
    	}
    }
    // gavin ++ <<
    GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [%s: thread] %s leave\n", "Vision Capture", __func__);
    return 1;
}

void CHeatFinderVisionCapture::OnRegroupH264Data(axc_byte* pData, axc_dword dwDataSize){
	fireDataReceivedEventHandler((unsigned char*)pData, (int)dwDataSize, VISION_H264PKGSEND2_MEDIASRV);
}

void CHeatFinderVisionCapture::OnReceivedH264Frame(axc_byte* pData, axc_dword dwDataSize){

	static int s_pIndex = 0;

	//rework frame and send to other object queue directily ... path 2
	axc_byte* pCacheStart = (axc_byte*) m_memRPICamFrameRework.GetAddress();
    axc_dword dwMaxCacheSize = m_memRPICamFrameRework.GetBufferSize();

    axc_byte* pCopyEnd = pCacheStart;

	bool l_bisIFrame = false;
	bool l_bIsDrop = false;
	int  width = 0;
	int  height = 0;


	int iH264StartLocation = 0;
	if ((iH264StartLocation = FindH264StartCode(pData, dwDataSize-1)) >=0){

		GLog( (iH264StartLocation != 0), tDEBUGTrace_MSK, "D: Vision Capture: iH264StartLocation = %d\n", iH264StartLocation );
		/**
		printf("N H264StartCode at %d\n", iH264StartLocation);
		printf("%02x ", pData[iH264StartLocation + 0]);
		for (int i =1; i<4; i++){
			printf("%02x ", pData[iH264StartLocation + i]);
			if ((i % 8) == 0){
				printf("\n");
			}
		}
		printf("\n");
		**/

		const axc_byte byNalType = pData[4] & 0x1F;

		//printf("N H264 frame type is %d\n", byNalType);
		switch(byNalType)
		{
		case H264NT_SPS: // SPS (7)
			if(dwDataSize <= sizeof(m_abyH264Sps))
			{
				memcpy(m_abyH264Sps, pData, dwDataSize);

				if (m_iH264SPSLen != dwDataSize){
					m_iH264SPSLen = dwDataSize;
				}
				GLog( tH264NAL, tNORMALTrace_MSK, "N: Vision Capture: SPS found len: %d\n", m_iH264SPSLen );
			}
			if(H264SPS_GetImageSize(pData, (int)dwDataSize, &width, &height) && width > 0 && height > 0)
			{
				//todo
				//sync x,y
				if(m_pvs_sizeVisionMainImage.cx != (axc_dword)width || m_pvs_sizeVisionMainImage.cy != (axc_dword)height)
				{
					m_pvs_sizeVisionMainImage.cx = (axc_dword)width;
					m_pvs_sizeVisionMainImage.cy = (axc_dword)height;

					//fire vision size change notify event
					fireVisionSizeChangedNotify(m_pvs_sizeVisionMainImage.cx, m_pvs_sizeVisionMainImage.cy);
				}
			}
			l_bIsDrop = true;
		    break;
		case H264NT_PPS: // PPS (8)
		    if(dwDataSize <= sizeof(m_abyH264Pps))
		    {
		        memcpy(m_abyH264Pps, pData, dwDataSize);

		        if (m_iH264PPSLen != dwDataSize){
		        	m_iH264PPSLen = dwDataSize;
		        }
		        GLog( tH264NAL, tNORMALTrace_MSK, "N: Vision Capture: PPS found len: %d\n", m_iH264PPSLen );
		    }
		    l_bIsDrop = true;
		    break;
		case H264NT_SLICE_IDR: // I-Frame (5)
		    l_bisIFrame = true;
		case H264NT_SLICE: // P-Frame (1)
		//case H264NT_SLICE_DPA: //?
		//case H264NT_SLICE_DPB: //?
		//case H264NT_SLICE_DPC: //?
		    m_fLastFrameTimeMs = CAxcTime::GetCurrentUTCTimeMs()*1000;

		    if(l_bisIFrame)
		    {
		    	if((dwDataSize + m_iH264SPSLen + m_iH264PPSLen) > dwMaxCacheSize)
		    	{
		            GLog(tCaptureVisionTrace, tNORMALTrace_MSK, "N: [%s: %s] rework I Frame too large %d > %d\n", "Vision Capture", __func__, (int)(dwDataSize + m_iH264SPSLen + m_iH264PPSLen) , dwMaxCacheSize);
		            break;
		    	}
		        if(0 != m_iH264SPSLen)
		        {
		            memcpy(pCopyEnd, m_abyH264Sps, m_iH264SPSLen);
		            pCopyEnd += m_iH264SPSLen;
		        }
		        if(0 != m_iH264PPSLen)
		        {
		            memcpy(pCopyEnd, m_abyH264Pps, m_iH264PPSLen);
		            pCopyEnd += m_iH264PPSLen;
		        }

		    	//GLog( tH264NAL, tDEBUGTrace_MSK, "D: Vision Capture: I-frame len:%.1fK --------------------------\n", (float)dwDataSize/1024.0 );
		    	GLog( tH264NAL, tDEBUGTrace_MSK, "D: Vision Capture: I-frame len:%d --------------------------\n", dwDataSize );
		    	s_pIndex = 0;
		    }
		    else
		    {
		    	//GLog( tH264NAL, tDEBUGTrace_MSK, "D: Vision Capture: P[%d] len:%.1fK\n", ++s_pIndex, (float)dwDataSize/1024.0 );
		    	GLog( tH264NAL, tDEBUGTrace_MSK, "D: Vision Capture: P[%d] len:%d\n", ++s_pIndex, dwDataSize );
		    }
		    //printf("N: [%s] I/P Frame size %d\n", __func__, (int)(pCopyEnd-pCopyBegin));
		    memcpy(pCopyEnd, pData, dwDataSize);
		    pCopyEnd += dwDataSize;
		    break;
		default:
		    l_bIsDrop = true;
		    GLog( tAll, tWARNTrace_MSK, "W: Vision Capture: unknown NAL type(%02X) drop !\n", byNalType );
		    break;

		}
	} else {
		GLog( (iH264StartLocation!=0), tWARNTrace_MSK, "W: Vision Capture: Cannot find h264 start code!!!\n" );
		return;
	}

	//fireDataReceivedEventHandler((unsigned char*)pData, (int)dwDataSize, VISION_H264PKGSEND2_MEDIASRV);
	bool l_bisHasData2Send = false;

	if (m_bDataSend2Udp_enable == true)
	{
		// make sure data has the key frame 'I', checking by OnReceivedH264Frame()@thread_process()
		if (m_bChkFirstFrameisIEnable == true){
			if (l_bisIFrame == true){
				m_bChkFirstFrameisIEnable = false;
				l_bisHasData2Send = true;
			}else{
				//printf("N l_bisHasData2Send 2 == false\n");
				l_bisHasData2Send = false;
			}
		}else if ((m_bChkFirstFrameisIEnable == false) &&
				(l_bIsDrop == false)){
			l_bisHasData2Send = true;
		}
	}else{
		// drop image
		m_bChkFirstFrameisIEnable = true;
	}

	axc_dword fps;
	if(fpsH264Frame.Progress(fps))
	{
		GLog(tCaptureVisionTrace, tVERBOSETrace_MSK, "V: h264Frame FPS = %d\n", fps );
	}

    if (l_bisHasData2Send == true && l_bIsDrop == false){
    	m_bChkFirstFrameisI_afterDataSend2UdpOffOn = (m_bChkFirstFrameisI_afterDataSend2UdpOffOn==true)?false:m_bChkFirstFrameisI_afterDataSend2UdpOffOn;

    	GlobalDef *pGlobal = (GlobalDef *) CHeatFinderUtility::GetGlobalsupport();
		const axc_ddword ddwPts = (axc_ddword) m_fLastFrameTimeMs;
		const axc_dword dwPts = (axc_dword)(ddwPts & 0xFFFFFFFFULL);

		if ((pGlobal->GetDataQueueAddress()->IsValid() == axc_true) && (m_pvs_sizeVisionMainImage.cx != 0) && (m_pvs_sizeVisionMainImage.cy != 0)){

			if ((pCopyEnd-pCacheStart) > 0){
				if (axc_false == CHeatFinderUtility::PushFrameToTxFifo(pGlobal->GetDataQueueAddress(), pCacheStart, (int)(pCopyEnd-pCacheStart), ADE2CAM_CHANNEL_VISUALMAIN,
						m_pvs_sizeVisionMainImage.cx, m_pvs_sizeVisionMainImage.cy,
						dwPts, 0, (l_bisIFrame == true)?1:0)){
			    	//CHeatFinderUtility::PrintNowData(1);
					GLog(tCaptureVisionTrace, tERRORTrace_MSK, "E: [%s: %s] restore data to queue fail ----- \n", "Vision Capture", __func__);
				}

				if(axc_false == CHeatFinderUtility::PushFrameToTxFifo(pGlobal->GetNetUDPDataQueue(), pCacheStart, (int)(pCopyEnd-pCacheStart), ADE2CAM_CHANNEL_VISUALMAIN,
						m_pvs_sizeVisionMainImage.cx, m_pvs_sizeVisionMainImage.cy,
						dwPts, 0, (l_bisIFrame == true)?1:0)){
					//CHeatFinderUtility::PrintNowData(1);
					GLog(tCaptureVisionTrace, tERRORTrace_MSK, "E: [%s: %s] restore data to udp queue fail ----- \n", "Vision Capture", __func__);
				}
			}
		}else{
			//GLog(tCaptureVisionTrace, tWARNTrace_MSK, "W: [%s: %s] Queue(%s) size of Vision Main Image={%d, %d} \n", "Vision Capture", __func__, ((pGlobal->GetDataQueueAddress()->IsValid() == axc_true)?"OK":"NULL"), m_pvs_sizeVisionMainImage.cx, m_pvs_sizeVisionMainImage.cy);
		}
    }

    if (!VISION_FRAME_REWORK){
    	// pass
    }
    else if ((l_bisHasData2Send == true) && (l_bIsDrop == false))
	{
		//printf("N: [%s] send data len:%d, last input:%d to MediaSrv\n", __func__, (int)(pCopyEnd-pCacheStart), dwDataSize);
		// fire to plug-in & media-server

		OnRegroupH264Data((unsigned char*)pCacheStart, (int)(pCopyEnd-pCacheStart));
		if(m_brawsend2plugin == true){
			fireDataReceivedEventHandler((unsigned char*)pCacheStart, (int)(pCopyEnd-pCacheStart), VISION_H264PKGSEND2_PLUGIN);
		}
	}

}


axc_bool CHeatFinderVisionCapture::IsH264StartCode(axc_byte* pData)
{
    axc_bool rst = axc_false;
    if (NULL != pData && 0x00 == pData[0] && 0x00 == pData[1] && 0x00 == pData[2] && 0x01 == pData[3])
    {
        //GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [%s] video check %s Data[0~3]={0x%02x,0x%02x,0x%02x,0x%02x}\n", "Vision Capture", __func__, pData[0], pData[1], pData[2], pData[3]);
        rst = axc_true;
    }
    return rst;
}
int CHeatFinderVisionCapture::FindH264StartCode(axc_byte* pData, axc_i32 iDataSize)
{
	if(iDataSize >= 5)
	{
		for(axc_i32 i=0; i<iDataSize-4; i++, pData++)
		{
			if(IsH264StartCode(pData))
			{
				return i;
			}
		}
	}
	return -1;
}

void CHeatFinderVisionCapture::OnReceivedRawData(const axc_i32 Width, const axc_i32 Height, const axc_i32 RawFMT, const axc_i32 size, axc_byte *pBuf){
	//GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [%s] %s fire w:%d h:%d format:%d size:%d\n", "Vision Capture", __func__, Width, Height, RawFMT, size);

	//FIXME: it shell different with yuv, gray, or rgb
	//int l_iMaxSize =0;
	//if (RawFMT !=RAW_OUTPUT_FMT_RGB){
	//	printf("E: [%s] %s only support 'rgb' raw data analysis, current raw output type:'%d:%s'\n", "Vision Capture", __func__, RawFMT,
	//			((RawFMT == RAW_OUTPUT_FMT_YUV)?"yuv":((RawFMT == RAW_OUTPUT_FMT_GRAY)?"gray":"unknown")));
	//	//return;
	//}
	// Please use the newest Respberry PI kernel and frameware: 'sudo rpi-update'

	if(m_brawsend2plugin == false){
		/**
		 * //wait for 2ms test:
		 * 1. 4mb -g 15 -fps 15
		 */
		//usleep(2000);

		return;
	}else if (m_bDataSend2Udp_enable == false){
		return;
	}else if (m_bChkFirstFrameisI_afterDataSend2UdpOffOn == true){
		return;
	}

	if ((Width != m_iRawDataWidth)||(Height != m_iRawDataHeight)){
	    m_iRawDataWidth = Width;
	    m_iRawDataHeight = Height;
		GlobalDef *pGlobal = (GlobalDef *) CHeatFinderUtility::GetGlobalsupport();
		xLeptonOverlay xLoi;
		pGlobal->GetOverlaybyResolution(Width, Height, xLoi);

		GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [%s] %s fire w:%d h:%d format:%d size:%d, and trigger Thermal Overlay Notify\n", "Vision Capture", __func__, Width, Height, RawFMT, size);
		fireSetThermalOverlayNotify(xLoi);
	}else if (m_bRawEncodeRestart == true){

		//FIXME
		GlobalDef *pGlobal = (GlobalDef *) CHeatFinderUtility::GetGlobalsupport();
		xLeptonOverlay xLoi;
		pGlobal->GetOverlaybyResolution(m_iRawDataWidth, m_iRawDataHeight, xLoi);

		GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [%s] %s use old resolution trigger Thermal Overlay Notify\n", "Vision Capture", __func__);
		fireSetThermalOverlayNotify(xLoi);
		m_bRawEncodeRestart = false;
	}

	fireRawDataReceivedEvnetHandler(Width, Height, RawFMT, size, pBuf);
}

// Arsenal ---------------------------------------------------------
void CHeatFinderVisionCapture::fireVisionSizeChangedNotify(const unsigned int Width, const unsigned int Height){

    for(unsigned int i = 0; i < m_visionsizechangenotifyhandlerList.size(); ++i)
    {
    	xVisionSizeChangedNotifyHandler handler = m_visionsizechangenotifyhandlerList[i];
        handler.fnEvent(handler.pContext, Width, Height);
    }
}

void CHeatFinderVisionCapture::fireLedStatusTwinkleNotify(const bool bValue){
	if (m_ledstatustwinklenotifyhandlerList.size() <=0){
		return;
	}

    for(unsigned int i = 0; i < m_ledstatustwinklenotifyhandlerList.size(); ++i)
    {

    	m_ledstatustwinklenotifyhandlerList[i].fnEvent(m_ledstatustwinklenotifyhandlerList[i].pContext, bValue);

    }
}

void CHeatFinderVisionCapture::fireDataReceivedEventHandler(unsigned char *pBuf, const int BufSize, const unsigned short channelIndex){
	if (m_datareceivedeventList.size() <=0){
		return;
	}
	if (BufSize <=0){
		return;
	}

	//printf("N: [%s] %s length: %d \n", "Vision Capture", __func__, BufSize);
	for(unsigned int i = 0; i < m_datareceivedeventList.size(); ++i)
	{
		m_datareceivedeventList[i].fnEvent(m_datareceivedeventList[i].pContext, pBuf, BufSize, channelIndex);
	}

}

void CHeatFinderVisionCapture::fireRawDataReceivedEvnetHandler(const axc_i32 Width, const axc_i32 Height, const axc_i32 RawFMT, const axc_i32 size, axc_byte *pBuf){
	if (m_rawdatareceivedhandleList.size() <=0){
		return;
	}

    for(unsigned int i = 0; i < m_rawdatareceivedhandleList.size(); ++i)
    {
    	m_rawdatareceivedhandleList[i].fnEvent(m_rawdatareceivedhandleList[i].pContext, Width, Height, RawFMT, size, pBuf);

    }
}

void CHeatFinderVisionCapture::fireSetThermalOverlayNotify(xLeptonOverlay &xLoi){
	if (m_SetThermalOverlaynotifyhandlerList.size() <=0){
		return;
	}

	for(unsigned int i = 0; i < m_SetThermalOverlaynotifyhandlerList.size(); ++i)
	{

		m_SetThermalOverlaynotifyhandlerList[i].fnEvent(m_SetThermalOverlaynotifyhandlerList[i].pContext, xLoi);

	}
}

void CHeatFinderVisionCapture::fireVisionRestartFinishNotify(){
	if (m_VisionRestartFinishNotifyEventListener.size() <=0){
		return;
	}

	for(unsigned int i = 0; i < m_VisionRestartFinishNotifyEventListener.size(); ++i)
	{
		m_VisionRestartFinishNotifyEventListener[i].fnEvent(m_VisionRestartFinishNotifyEventListener[i].pContext);
	}
}

char* CHeatFinderVisionCapture::getBootingParameterIdentify(int bootingId){

	static char s_chBuffer[64]={0};
	switch(bootingId) {
		case 0:
			return "RPICAMERA_PARAMETER_SHARPNESS";
		case 1:
			return "RPICAMERA_PARAMETER_BRIGHTNESS";
		case 2:
			return "RPICAMERA_PARAMETER_CONTRAST";
		case 3:
			return "RPICAMERA_PARAMETER_SATURATION";
		case 4:
			return "RPICAMERA_PARAMETER_EXPOSURE";
		case 5:
			return "RPICAMERA_PARAMETER_AWB";
		case 6:
			return "RPICAMERA_PARAMETER_AWBGAIN";
		case 7:
			return "RPICAMERA_PARAMETER_IMAGEEFFECT";
		case 8:
			return "RPICAMERA_PARAMETER_VERBOSE";
		case 9:
			return "RPICAMERA_PARAMETER_BITRATE";
		case 10:
			return "RPICAMERA_PARAMETER_TIMEOUT";
		case 11:
			return "RPICAMERA_PARAMETER_PREVIEWENC";
		case 12:
			return "RPICAMERA_PARAMETER_SHOWSPSPPS";
		case 13:
			return "RPICAMERA_PARAMETER_RESOLUTION_W";
		case 14:
			return "RPICAMERA_PARAMETER_RESOLUTION_H";
		case 15:
			return "RPICAMERA_PARAMETER_FPS";
		case 16:
			return "RPICAMERA_PARAMETER_IPFRAMERATE";
		case 17:
			return "RPICAMERA_PARAMETER_RAWCAPTURE";
		case 18:
			return "RPICAMERA_PARAMETER_QP";
		case 19:
			return "RPICAMERA_PARAMETER_H264PROFILE";
		case 20:
			return "RPICAMERA_PARAMETER_VFLIP";
		case 21:
			return "RPICAMERA_PARAMETER_HFLIP";
		default:
			CAxcString::snprintf(s_chBuffer, (axc_dword)sizeof(s_chBuffer), "Unknown_Parameter_ID:%d", bootingId);
			return s_chBuffer;
	}

}

bool CHeatFinderVisionCapture::setVisioParameter2RPICamera(bool bType, bool bInit){
	bool bRst = false;
	GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [Vision Capture] +++setVisioParameter2RPICamera\n");
	//bool bDoSetting = false;
	Cheatfinderconfigmanager *pConfigManager = (Cheatfinderconfigmanager *)CHeatFinderUtility::GetConfigObj();
	bool bConfigChangeedIsDiff = pConfigManager->GetIsVisionParameterChanged();
	unsigned int iBootingConfig = pConfigManager->GetChangedBootingVisionCommand();
	unsigned int iRunningConfig = pConfigManager->GetChangedRuntimeVisionCommand();
	bool bIsRPICameraRunning = m_RPICameraFW.IsRunning();
	if (bIsRPICameraRunning){
		if ((bConfigChangeedIsDiff) && (iBootingConfig != RPICAMERA_COMMAND_BUNKNOWN) && (bType == true)){
			GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [Vision Capture] config. changed & need modify booting setting, but RPI Camera shell close first.\n");
			bRst = false;
		}else if ((bConfigChangeedIsDiff) && (iBootingConfig == RPICAMERA_COMMAND_BUNKNOWN) && (iRunningConfig == RPICAMERA_COMMAND_RUNKNOWN)){
			GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [Vision Capture] config. changed, but no any api mapping.\n"); //shell not be here.
			bRst = false;
		}else{
			bRst = true;
		}
	}else{
		if ((bConfigChangeedIsDiff) && (iBootingConfig == RPICAMERA_COMMAND_BUNKNOWN) && (iRunningConfig == RPICAMERA_COMMAND_RUNKNOWN)){
			GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [Vision Capture] config. changed, but no any api mapping.\n"); //shell not be here.
			bRst = false;
		}else{
			bRst = true;
		}
	}

	int iRst = -1;
	char cValue[32];
	memset(cValue, 0, sizeof(cValue));
	int iValue =0;

	if (bRst){
		GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [Vision Capture] configuration to parameter start. ConfigChangeedIsDiff:%s BootingConfig:0x%02x RunningConfig:0x%02x IsRPICameraRunning:%s\n"
			,bConfigChangeedIsDiff?"true":"false",iBootingConfig,iRunningConfig,bIsRPICameraRunning?"true":"false");
	}

	if((bRst)&&(bType==false)){

		for(/*E_RPICAMERA_PARAMETER*/int i=0;i<RPICAMERA_FIRST_NEED_RESTART_PARAMETER;i++){
			//FIXME
			if (
				(i == RPICAMERA_PARAMETER_AWBGAIN)
			){
				//pase this element
				continue;
			}

			if (pConfigManager->IsBootCommandfromCommandRecordbyIndex((E_RPICAMERA_PARAMETER)i)){
				//if (((unsigned int)pConfigManager->GetIDfromCommandRecordbyIndex(i) & iBootingConfig) <= 0)
				GLog(tCaptureVisionTrace, tWARNTrace_MSK, "W: [Vision Capture] parameter: %u shell not belong with running\n", i);
				continue;
			}else{
				if ((((unsigned int)pConfigManager->GetIDfromCommandRecordbyIndex((E_RPICAMERA_PARAMETER)i) & iBootingConfig) <= 0) && !bInit)
					continue;
			}

			iRst = pConfigManager->GetImportVisionCommandValue((E_RPICAMERA_PARAMETER)i, cValue, 32, iValue);
			if (iRst <0){
				GLog(tCaptureVisionTrace, tWARNTrace_MSK, "W: [Vision Capture] parameter [%d] value is empty or not be used\n", i);
			}else{
				/**
				RPICAMERA_PARAMETER_SHARPNESS = 0,
				RPICAMERA_PARAMETER_BRIGHTNESS = 1,
				RPICAMERA_PARAMETER_CONTRAST = 2,
				RPICAMERA_PARAMETER_SATURATION = 3,
				RPICAMERA_PARAMETER_EXPOSURE = 4,
				RPICAMERA_PARAMETER_AWB = 5,
				RPICAMERA_PARAMETER_AWBGAIN = 6,
				RPICAMERA_PARAMETER_IMAGEEFFECT = 7,
				RPICAMERA_PARAMETER_VERBOSE = 8,
				**/


				if (iRst ==0){
					GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [Vision Capture] runtime parameter: %u set '%s'\n", i, cValue);
					//GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [Vision Capture] runtime parameter: %u set %d\n", i, iValue);
				}else if (iRst >=1){
					//GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [Vision Capture] runtime parameter: %u set '%s'\n", i, cValue);
					GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [Vision Capture] runtime parameter: %u set %d\n", i, iValue);
				}
			}

			bool bRPICamSettingRst = false;
			//FIXME
			switch(i){
			case RPICAMERA_PARAMETER_SHARPNESS:
				bRPICamSettingRst = m_RPICameraFW.SetSharpness(iValue);
				break;
			case RPICAMERA_PARAMETER_BRIGHTNESS:
				bRPICamSettingRst = m_RPICameraFW.SetBrightness(iValue); //FIXME: m_RPICameraFW.SetBrightness(iValue);
				break;
			case RPICAMERA_PARAMETER_CONTRAST:
				bRPICamSettingRst = m_RPICameraFW.SetContrast(iValue);
				break;
			case RPICAMERA_PARAMETER_SATURATION:
				bRPICamSettingRst = m_RPICameraFW.SetSaturation(iValue);
				break;
			case RPICAMERA_PARAMETER_EXPOSURE:
				bRPICamSettingRst = m_RPICameraFW.SetExposurebyCStr(cValue);
				break;
			case RPICAMERA_PARAMETER_AWB:
				bRPICamSettingRst = m_RPICameraFW.SetWriteBalancebyCStr(cValue);
				break;
			case RPICAMERA_PARAMETER_IMAGEEFFECT:
				bRPICamSettingRst = m_RPICameraFW.SetImageViewEffectbyCStr(cValue);
				break;
			case RPICAMERA_PARAMETER_VERBOSE:
				bRPICamSettingRst = m_RPICameraFW.SetVerbose(true);
				break;
			case RPICAMERA_PARAMETER_AWBGAIN:
				bRPICamSettingRst = true; //not support this time;
				break;
			default:
				GLog(tCaptureVisionTrace, tWARNTrace_MSK, "W: [Vision Capture] out of supporting list of running setting\n");
				break;
			}

			if (bRPICamSettingRst == false){
				GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [Vision Capture] runtime parameter: %u set (%d:%s) fault\n", i, iValue, cValue);
			}
		}

	}else if((bRst)&&(bType)){

		bool bIsResolutionChanged = false;
		int l_iW=0, l_iH=0;
		for(/*E_RPICAMERA_PARAMETER*/int j=RPICAMERA_FIRST_NEED_RESTART_PARAMETER;j<RPICAMERA_TOTAL_PARAMETERS;j++){
			if (pConfigManager->IsBootCommandfromCommandRecordbyIndex((E_RPICAMERA_PARAMETER)j)){
				if ((((unsigned int)pConfigManager->GetIDfromCommandRecordbyIndex((E_RPICAMERA_PARAMETER)j) & iRunningConfig) <= 0) && !bInit)
					continue;
			}else{
				GLog(tCaptureVisionTrace, tWARNTrace_MSK, "W: [Vision Capture] parameter: %u shell not belong with booting\n", j);
				continue;
			}

			iRst = pConfigManager->GetImportVisionCommandValue((E_RPICAMERA_PARAMETER)j, cValue, 32, iValue);
			if (iRst <0){
				GLog(tCaptureVisionTrace, tWARNTrace_MSK, "W: [Vision Capture] parameter [%d] value is empty or not be used\n", j);
			}else{
				/**
				RPICAMERA_PARAMETER_BITRATE = 9,
				RPICAMERA_PARAMETER_TIMEOUT = 10,
				RPICAMERA_PARAMETER_PREVIEWENC = 11,
				RPICAMERA_PARAMETER_SHOWSPSPPS = 12,
				RPICAMERA_PARAMETER_RESOLUTION_W = 13,
				RPICAMERA_PARAMETER_RESOLUTION_H = 14,
				RPICAMERA_PARAMETER_FPS = 15,
				RPICAMERA_PARAMETER_IPFRAMERATE = 16,
				RPICAMERA_PARAMETER_RAWCAPTURE = 17,
				RPICAMERA_PARAMETER_QP = 18,
				RPICAMERA_PARAMETER_H264PROFILE = 19,
				**/

				if (iRst ==0){
					GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [Vision Capture] booting parameter: %u[%s] set '%s'\n", j, getBootingParameterIdentify(j), cValue);
				}else if (iRst >=1){
					GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [Vision Capture] booting parameter: %u[%s] set %d\n", j, getBootingParameterIdentify(j), iValue);
				}
			}

			bool bRPICamSettingRst = false;
			switch(j){
			case RPICAMERA_PARAMETER_BITRATE:
				bRPICamSettingRst = m_RPICameraFW.SetBitrate((unsigned int)iValue);
				break;
			case RPICAMERA_PARAMETER_TIMEOUT:
				//GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [Vision Capture] /%d '-t': %d\n", RPICAMERA_PARAMETER_TIMEOUT, iValue);
				bRPICamSettingRst = m_RPICameraFW.SetTimeout((unsigned int)iValue);
				break;
			case RPICAMERA_PARAMETER_PREVIEWENC:
				bRPICamSettingRst = m_RPICameraFW.SetPreviewAfterEncEnable(false);
				break;
			case RPICAMERA_PARAMETER_SHOWSPSPPS:
				bRPICamSettingRst = m_RPICameraFW.SetInlineHeadersEnable(true);
				break;
			case RPICAMERA_PARAMETER_RESOLUTION_W:
				bIsResolutionChanged =true;
				l_iW = (iValue ==0)?1920:iValue;
				bRPICamSettingRst = true;
				break;
			case RPICAMERA_PARAMETER_RESOLUTION_H:
				bIsResolutionChanged =true;
				l_iH = (iValue ==0)?1080:iValue;
				bRPICamSettingRst = true;
				break;
			case RPICAMERA_PARAMETER_FPS:
				bRPICamSettingRst = m_RPICameraFW.SetFramePerSecond(iValue);
				break;
			case RPICAMERA_PARAMETER_IPFRAMERATE:
				bRPICamSettingRst = m_RPICameraFW.SetIntraAndPeriodFrameRate(iValue);
				break;
			case RPICAMERA_PARAMETER_RAWCAPTURE:
				bRPICamSettingRst = m_RPICameraFW.SetRawCapturingbyCStr(true, cValue);
				break;
			case RPICAMERA_PARAMETER_QP:
				bRPICamSettingRst = m_RPICameraFW.SetQuantization((unsigned int)iValue);
				break;
			case RPICAMERA_PARAMETER_H264PROFILE:
				//bRPICamSettingRst = m_RPICameraFW.SetH264Profile();
				bRPICamSettingRst = true; //use default
				break;
			case RPICAMERA_PARAMETER_VFLIP:
				m_RPICameraFW.SetVFlip(iValue);
				bRPICamSettingRst = true; //use default
				break;
			case RPICAMERA_PARAMETER_HFLIP:
				m_RPICameraFW.SetHFlip(iValue);
				bRPICamSettingRst = true; //use default
				break;
			default:
				GLog(tCaptureVisionTrace, tWARNTrace_MSK, "W: [Vision Capture] out of supporting list of booting setting\n");
				break;
			}


			if ((bRPICamSettingRst == false)
					&&((j!= RPICAMERA_PARAMETER_RESOLUTION_W)||(j!= RPICAMERA_PARAMETER_RESOLUTION_H))){
				GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [Vision Capture] booting parameter: %u[%s] set (%d:%s) fault\n", j, getBootingParameterIdentify(j), iValue, cValue);
			}
		}

		if (bIsResolutionChanged){
			if (!m_RPICameraFW.SetResolution((unsigned int)l_iW, (unsigned int)l_iH)){
				GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "D: [Vision Capture] booting parameter: set Resolution fault w:%d,h:%d\n", l_iW, l_iH);
			}
		}else if ((l_iW == 0) || (l_iH ==0)){
			//FIXME

		}

		//Demo status add output path will cause?????
		/**
		 * Raspberry PI Error:
		 *
		 * There is a strange problem here. When there is a notification to output to a file (whether sent to stdout or not only),
		 * the size of the I frame is reduced and the compression ratio of the image seems to change.
		 * Therefore, the fps is set to 30. There is still a certain degree of smoothness, and the picture quality is also good.
		 *
		 * This is the reason why we give a fake output file path.
		 **/
		if (m_RPICameraFW.SetOutputParameter("/tmp/FakeLocation", 16) == false){
			GLog(tCaptureVisionTrace, tERRORTrace_MSK, "E: [Vision Capture] booting parameter: set Output Path fault\n");
		}
	}else{
		/**
		 * 	bool bConfigChangeedIsDiff = pConfigManager->GetIsVisionParameterChanged();
		 * 	unsigned int iBootingConfig = pConfigManager->GetChangedBootingVisionCommand();
		 * 	unsigned int iRunningConfig = pConfigManager->GetChangedRuntimeVisionCommand();
		 * 	bool bIsRPICameraRunning = m_RPICameraFW.IsRunning();
		 */
		GLog(tCaptureVisionTrace, tWARNTrace_MSK, "W: [Vision Capture] configuration to parameter fail. ConfigChangeedIsDiff:%s BootingConfig:0'x%02x RunningConfig:0'x%02x IsRPICameraRunning:%s\n"
				,bConfigChangeedIsDiff?"true":"false",iBootingConfig,iRunningConfig,bIsRPICameraRunning?"true":"false");
	}

	GLog(tCaptureVisionTrace, tDEBUGTrace_MSK, "W: [Vision Capture] ---setVisioParameter2RPICamera\n");
	return bRst;
}

//////////////////////////////////////////////////////////////////////////
