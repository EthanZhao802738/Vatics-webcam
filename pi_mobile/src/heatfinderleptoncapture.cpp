#include "heatfinderleptoncapture.h"
#include "heatfinderconfigmanager.h"
#include "ade_camera_sdk.h"
#include <iostream>
#include <list>

CHeatFinderLeptonCapture::CHeatFinderLeptonCapture():
    m_hThread("lepton/capture"),
    m_bStopThread(true),
    m_bPrintLeptonStatus(false),
    m_send2plugin_pauseplay(false),
    m_send2MediaRTSPSer_pauseplay(true),
    m_hThread_IVSOUT("lepton/ivsout"),
    m_memThermalOutput("thermal/frame"),
    m_hThread_READ("lepton/read"),
    m_caxcmemThermalCompress("thermal/compress")
{
    m_sizeThermalImage.cx = 0;
    m_sizeThermalImage.cy = 0;

    m_iQueueCacheLen = 0;
    sem_init( &requestOutput, 0, 0 );

}

CHeatFinderLeptonCapture::~CHeatFinderLeptonCapture()
{
    m_bStopThread = true;
    if (m_hThread.IsValid())
        m_hThread.Destroy(2000);

    if (m_hThread_IVSOUT.IsValid())
    	m_hThread_IVSOUT.Destroy(2000);

    if (m_hThread_READ.IsValid()){
    	sem_post(&requestOutput); //avoid blocking at sem_wait()
    	m_hThread_READ.Destroy(2000);
    }

    //mLeptonSDK.stop(); //!

    if (m_memThermalOutput.IsValid()){
    	m_memThermalOutput.Free();
    }
    if (m_caxcmemThermalCompress.IsValid()){
    	m_caxcmemThermalCompress.Free();
    }
    sem_destroy(&requestOutput);

    GLog(tCaptureThermalTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] Destroy\n", "Lepton Thermal Capture");
}

bool CHeatFinderLeptonCapture::Run()
{
    bool bRst = false;
    Stop();

    m_bStopThread = false;

    GLog(tCaptureThermalTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] Start Lepton SDK\n", "Lepton Thermal Capture");

    if(!m_memThermalOutput.Create(1*1024*1024)){
    	return bRst;
    }

    mLeptonSDK.AddDataSendNotifyEvent(OnReceivedThermalData, this);
    mLeptonSDK.AddHeatObjectSendNotifyEvent(OnReceivedHeatObjects, this);
    CHeatFinderUtility::SetLeptonSDK(&mLeptonSDK);

    CadeSDKAPIleptonthermal *pLeptonSDK = (CadeSDKAPIleptonthermal*) CHeatFinderUtility::GetLeptonSDK();

    GLog(tCaptureThermalTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] Make Lepton SDK Run\n", "Lepton Thermal Capture");
    if (pLeptonSDK !=NULL){
		if (false == pLeptonSDK->run()){
			GLog(tCaptureThermalTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] Start Lepton SDK Fail\n", "Lepton Thermal Capture");
			return bRst;
		}
    }else{
    	GLog(tCaptureThermalTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] Start Lepton SDK Fail, no lepton SDK\n", "Lepton Thermal Capture");
    	return bRst;
    }

    if (!m_hThread.Create(thread_read_thermal_buffer_process, this, 0, 0/*12*/))
        return bRst;

    if (!m_hThread_IVSOUT.Create(thread_ivsout_process, this))
        return bRst;

    if (!m_hThread_READ.Create(thread_read_process ,this))
    	return bRst;

    GLog(tCaptureThermalTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] Start Lepton SDK Success^^\n", "Lepton Thermal Capture");
    bRst = true;
    return bRst;
}

void CHeatFinderLeptonCapture::Stop()
{
    m_bStopThread = true;
    if (m_hThread.IsValid())
        m_hThread.Destroy(2000);

    if (m_hThread_IVSOUT.IsValid())
    	m_hThread_IVSOUT.Destroy(2000);

    if (m_hThread_READ.IsValid()){
    	sem_post(&requestOutput); //avoid blocking at sem_wait()
    	m_hThread_READ.Destroy(2000);
    }

    CadeSDKAPIleptonthermal *pLeptonSDK = (CadeSDKAPIleptonthermal*) CHeatFinderUtility::GetLeptonSDK();
    if (pLeptonSDK !=NULL){
    	pLeptonSDK->stop();
    }

    if (m_memThermalOutput.IsValid()){
    	m_memThermalOutput.Free();
    }
    sem_destroy(&requestOutput);

    GLog(tCaptureThermalTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] Stop\n", "Lepton Thermal Capture");
}

void CHeatFinderLeptonCapture::PrintLeptonStatus(const bool value)
{
   m_bPrintLeptonStatus = value;
}

void CHeatFinderLeptonCapture::AddDataReceivedEvent(OnDataReceivedEvent fnEvent, unsigned short type, void *pContext)
{
    xDataReceivedEventHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;
    handler.type = type;
    m_DataReceivedEventList.push_back(handler);
}

void CHeatFinderLeptonCapture::AddLedStatusTwinkleNotifyEvent(OnLedStatusTwinkleNotify fnEvent, void *pContext)
{
    xLedStatusTwinkleNotifyHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;

    m_LedStatusTwinkleNotifyList.push_back(handler);
}

void CHeatFinderLeptonCapture::AddThermalSizeChangedNotifyEvent(OnThermalSizeChangedNotify fnEvent, void *pContext)
{
    xThermalSizeChangedNotifyHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;

    m_ThermalSizeChangedNotifyList.push_back(handler);
}

/**
 * mark.hsieh ++
 */
void CHeatFinderLeptonCapture::AddThermalStatusChangedNotifyEvent(ThermalStatusChangedEvent objEvent, void *pContext){
	xThermalStatusChangedNotifyHandler handler;
    handler.pContext = pContext;
    memset(&handler.objEvent, 0, sizeof(handler.objEvent));
    handler.objEvent = objEvent;

    m_ThermalStatusChangedList.push_back(handler);
}
void CHeatFinderLeptonCapture::AddDataPopedEvent(OnDataPopedEvent fnEvent, void *pContext){
	xDataPopedEventHandler handler;
	handler.pContext = pContext;
	handler.fnEvent = fnEvent;

	m_DataPopedEventList.push_back(handler);
}
void CHeatFinderLeptonCapture::AddHeatObjectReceivedNotifyEvent(OnHeatObjectReceivedEvent fnEvent , void *pContext){
	xHeatObjectReceivedEventHandler handler;
	handler.fnEvent = fnEvent;
	handler.pContext = pContext;

	m_HeatObjectReceivedNotifyList.push_back(handler);
}

void CHeatFinderLeptonCapture::AddReceivedThermalRestartEventNotify(OnReceivedThermalRestartFinishNotifyEvent fnEvent , void *pContext){
	mLeptonSDK.AddReceivedThermalRestartFinishNotifyEvent(fnEvent, pContext);
}

void CHeatFinderLeptonCapture::SetSend2PluginPausePlay(bool bValue){
	if (m_send2plugin_pauseplay != bValue)
		m_send2plugin_pauseplay = bValue;
}
bool CHeatFinderLeptonCapture::GetSend2PluginPausePlay(){
	return m_send2plugin_pauseplay;
}

/**
 * @return:
 * 		true: 	enable Analyze Heat Object When Idle (still do analysis job when 'bThermalHeatObj_Enable' disable)
 * 		false:	disable ... Analysis job only trigger by 'bThermalHeatObj_Enable' enable
 */
bool CHeatFinderLeptonCapture::setIntervalOfAnalyzeHeatObjectWhenIdle(double fInterval){
	bool _bRst = false;

	CadeSDKAPIleptonthermal *pLeptonSDK = (CadeSDKAPIleptonthermal*) CHeatFinderUtility::GetLeptonSDK();
	_bRst = pLeptonSDK->setIntervalOfIdleStatusAnalyzeHeatObject(fInterval);

	return _bRst;
}

axc_dword CHeatFinderLeptonCapture::thread_read_thermal_buffer_process(){
	GLog(tCaptureThermalTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] %s Start: call sdk to capture/copy the data from register\n", "Lepton Thermal Capture", __func__);
    //init.
    //double fLastFrameTimeMs = 0;

    void *pContext = CHeatFinderUtility::GetGlobalsupport();
    GlobalDef *pGlobal = reinterpret_cast<GlobalDef*> (pContext);
    pGlobal->AddpThreadRecord("Thermal Capture");

    CadeSDKAPIleptonthermal *pLeptonSDK = (CadeSDKAPIleptonthermal*) CHeatFinderUtility::GetLeptonSDK();

    /**
     * mark.hsieh ++
     * below design work for helping frame lose FSM.
     */
    double fInterval = LEPTON_SDK_DEFAULT_FAIL_INTERVAL;
    if(false == pLeptonSDK->setIntervalValue(fInterval)){
    	GLog(tCaptureThermalTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] %s new interval time of fault bee deny. Your input: %.03lf ms\n", "Lepton Thermal Capture", __func__, fInterval);
    }

    while (!m_bStopThread) {
        if (pLeptonSDK == NULL) {
        	m_hThread.SleepMilliSeconds(100);
        	continue;
        }
        //if (fdThermalFifo < 0){
        //update and read last data from queue
        if (pLeptonSDK->ProcessLeptonReadRawDataOnce() == false) {
        	// object will call fail occur interval
        	continue;
        }

        /*
         * lepton sdk will fill buffer after few time.
         * suggestion is 20 ms
         */
        m_hThread.SleepMilliSeconds(20);    
    }

    GLog(tCaptureThermalTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] %s Stop\n", "Lepton Thermal Capture", __func__);
    return 1;
}

axc_dword CHeatFinderLeptonCapture::thread_ivsout_process(){
	GLog(tCaptureThermalTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] %s Start: renew SDK running information detail and checking health\n", "Lepton Thermal Capture", __func__);

	GlobalDef *pGlobal = (GlobalDef *) CHeatFinderUtility::GetGlobalsupport();
    CadeSDKAPIleptonthermal *pLeptonSDK = (CadeSDKAPIleptonthermal*) CHeatFinderUtility::GetLeptonSDK();

    pGlobal->AddpThreadRecord("Thermal IVS Reflash");

    //init. dynamic var
    unsigned int x,y,datasize;

    // valgrind say fix: Conditional jump or move depends on uninitialised value(s)
    x = 0;
    y = 0;
    datasize = 0;

    double l_fCurrentTimeMs = 0.0;
    bool bRestart = false;
    double l_fLoseTimeMs = 0.0;
    while (!m_bStopThread) {
    	l_fCurrentTimeMs = CAxcTime::GetCurrentUTCTimeMs();
    	if ((l_fCurrentTimeMs - (pLeptonSDK->getLastFrameMs())) > 5.0) {
    		//CHeatFinderUtility::SetTwinkleLedShowStatusWrong();
    		l_fLoseTimeMs = l_fCurrentTimeMs - (pLeptonSDK->getLastFrameMs());

    		if (bRestart == false) {
		    	Cheatfinderconfigmanager *pConfigContextManager = (Cheatfinderconfigmanager *)CHeatFinderUtility::GetConfigObj();
		    	T_CONFIG_FILE_OPTIONS *pConfigContext = ((pConfigContextManager == NULL)?NULL:pConfigContextManager->GetConfigContext());
		    	if((pConfigContext->iOpenStream & OPEN_THERMAL_STREAM) != 0)
		    	{
		    		fireLedStatusTwinkleNotify(false);
					//CHeatFinderUtility::PrintNowData(1);
                    GLog(tCaptureThermalTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] %s : lose thermal stream above 5 seconds\n", "Lepton Thermal Capture", __func__);
					bRestart = true;
		    	}
		    	else
		    	{
		    		fireLedStatusTwinkleNotify(true);
		    		m_hThread.SleepMilliSeconds(1000);
		    	}
    		}
    	} else {
    		//CHeatFinderUtility::SetTwinkleLedShowStatusOK();
    		if (bRestart) {
    			GLog(tCaptureThermalTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] frame resume success, lose %f seconds\n", "Lepton Thermal Capture", l_fLoseTimeMs);
    			fireLedStatusTwinkleNotify(true);
    			l_fLoseTimeMs = 0;
    			bRestart = false;
    		} else {
    			//FIXME: don't why SDK auto-reopen will cause twink trigger
    			//fireLedStatusTwinkleNotify(true);
    		}
    	}

        if (pLeptonSDK == NULL) {
        	m_hThread.SleepMilliSeconds(20);
        	continue;
        }

        //ac_obj->get_thermal_resolution(&x, &y, &datasize);
        pGlobal->GetThermalResolution(x, y, datasize);
        if (m_sizeThermalImage.cx != x || m_sizeThermalImage.cy != y) {
            //also mean the queue of view need to restore.
        	m_queuelocker.lock();

            fireThermalSizeChangedNotify(x, y);
            m_iQueueCacheLen = 0;

            m_queuelocker.unlock();
        }

        if (pLeptonSDK->ProcessUpdateThermalIvsOnce() == false) {
        	m_hThread_IVSOUT.SleepMilliSeconds(20);
        	continue;
        } else {
        	//m_hThread.SleepMilliSeconds(5);
        	//CHeatFinderUtility::SetTwinkleLedShowNetworkUsed();
        }
    }
    GLog(tCaptureThermalTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] %s Stop\n", "Lepton Thermal Capture", __func__);
    return 1;
}

axc_dword CHeatFinderLeptonCapture::thread_read_process(){
	GLog(tCaptureThermalTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] %s Start: read the view from queue and send notify to registered observer\n", "Lepton Thermal Capture", __func__);
    //init.
    //double fLastFrameTimeMs = 0;

    void *pContext = CHeatFinderUtility::GetGlobalsupport();
    GlobalDef *pGlobal = reinterpret_cast<GlobalDef*> (pContext);
    pGlobal->AddpThreadRecord("Thermal Distributor");

    /**
     * mark.hsieh ++
     */
    axc_byte abyReadBuffer[80*80*2];
    int _dataLen = 80*80*2;

    while (!m_bStopThread) {
    	sem_wait(&requestOutput);
    	if (m_bStopThread){
    		break;
    	}

		//read thermal data queue
		int iReadLen = -1;
		iReadLen = readMem(abyReadBuffer, sizeof(abyReadBuffer));
		if (iReadLen <= 0) {
			if (iReadLen < 0) {
                GLog(tCaptureThermalTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] read view wrong!\n", "Lepton Thermal Capture");
            }
			m_hThread_READ.SleepMilliSeconds(10);
			continue;
		} else if (iReadLen < _dataLen) {
			GLog(tCaptureThermalTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] read view warning! data length not enough. (%d/%d)\n", "Lepton Thermal Capture", iReadLen, _dataLen);
		}

		ProcessSendThermalView(abyReadBuffer, (axc_dword)iReadLen, 0);
	}
    GLog(tCaptureThermalTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] %s Stop\n", "Lepton Thermal Capture", __func__);
	return 1;
}

void CHeatFinderLeptonCapture::OnReceivedThermalData(axc_byte* pData, axc_i32 ldDataSize, T_ADETHERMAL_RAWDATA_FRAME *raw){
    unsigned int last_ops = 0;
    if (m_ThermalStatusChangedList.size() > 0){
    	GLog(tCaptureThermalTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] thermal got %d status change event\n", "Lepton Thermal Capture", m_ThermalStatusChangedList.size());
        for(unsigned int i = 0; i < m_ThermalStatusChangedList.size(); ++i){
        	if (m_send2MediaRTSPSer_pauseplay != m_ThermalStatusChangedList[i].objEvent.pauseplay){
        		m_send2MediaRTSPSer_pauseplay = m_ThermalStatusChangedList[i].objEvent.pauseplay;
        	}
        	last_ops++;
        }
        m_ThermalStatusChangedList.erase(m_ThermalStatusChangedList.begin()+0, m_ThermalStatusChangedList.begin()+last_ops);
    }

    /**
     * Write the received data/frame into the queue {rtsp: memory, tcp/udp ade style: fifo type list}
     */
	if (m_send2MediaRTSPSer_pauseplay == true){
		//ProcessSendThermalView(pData, (axc_dword)ldDataSize, 0);
		//write thermal data queue

		writeMem(pData, ldDataSize);
		sem_post(&requestOutput);
	}

#if true
	GlobalDef *pGlobal = (GlobalDef *) CHeatFinderUtility::GetGlobalsupport();
	// send data with no-notify. Push data to fifo (the queue of TCP ade format)
	if (pGlobal->GetNetDataQueue()->IsValid()){
		// push compressed-frame to network-fifo (tcp & udp)
		const axc_dword dwNeedCompressBufferSize = ldDataSize + 128;
		if(m_caxcmemThermalCompress.GetBufferSize() < dwNeedCompressBufferSize){
			m_caxcmemThermalCompress.Resize(dwNeedCompressBufferSize, 0);
		}
		const axc_dword dwCompressDataSize = adethermal_rawdata_compress(raw, m_caxcmemThermalCompress.GetAddress(), m_caxcmemThermalCompress.GetBufferSize());
		if(0 < dwCompressDataSize)
		{
			if(!CHeatFinderUtility::PushFrameToTxFifo(pGlobal->GetNetDataQueue(), m_caxcmemThermalCompress.GetAddress(), (int)dwCompressDataSize, ADE2CAM_CHANNEL_LEPTONRAW, (int)raw->width, (int)raw->height, raw->frame_pts, raw->frame_seq, axc_true))
			{
				GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Thermal API] failed push thermal-frame to tcp-fifo\n");
			}
			if(!CHeatFinderUtility::PushFrameToTxFifo(pGlobal->GetNetUDPDataQueue(), m_caxcmemThermalCompress.GetAddress(), (int)dwCompressDataSize, ADE2CAM_CHANNEL_LEPTONRAW, (int)raw->width, (int)raw->height, raw->frame_pts, raw->frame_seq, axc_true))
			{
				GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: Thermal API] failed push thermal-frame to udp-fifo\n");
			}
		}
	}
#endif
}

void CHeatFinderLeptonCapture::ProcessSendThermalView(axc_byte* pData, axc_dword dwDataSize, axc_dword flags)
{
    VCEThermalData vceData;

    if (dwDataSize > 0)
    {
        memset(vceData.mThermalData, 0, THERMAL_DATA_SIZE);

        // Append raw data into send package
        //int sentSize = 0;
        // Added by VCE

    	//printf("D: [%s] %s!\n", "Lepton Thermal Capture", __func__);
        /*
         * In VCE's data structure, you only need to update mStartCode, mDataSize, mTimeStamp, mThermalData, height/width. That's it. (update at +-09/08)
         * Below is the sample codes from VCE.
         * We found the timestamp value in your codes are incorrect.
         * The overall size is 4 + 4 + 8 + 4 + 9600 = 9620.
         */
        // Copy it by VCE's data structure.
        //memcpy(data.mStartCode, thermalDataStartCode, sizeof(data.mStartCode));
        vceData.mHeader.mDataSize = ((int)dwDataSize >= THERMAL_DATA_SIZE)? (int)dwDataSize: THERMAL_DATA_SIZE;
        //data.mTimeStamp = (int64_t) now.tv_sec * 1000000 + (int64_t) now.tv_usec;

        // Add width and height
        vceData.mHeader.mDataWidth = m_sizeThermalImage.cx;
        vceData.mHeader.mDataHeight = m_sizeThermalImage.cy;

        memcpy(vceData.mThermalData, (char*)pData, vceData.mHeader.mDataSize);

        // Total size is 4 + 4 + 8 + 4 + 9600 = 9620.
        // Send Header first, due to we can not send pointers both.
        //sendto(UDPSocketHandle, (char*) &vceData.mHeader, vceData.mHeader.GetHeaderSize(), 0,
        //                    (const struct sockaddr*) &ThermalDataAddress, sizeof(struct sockaddr_in));
        // Send Data
        //sentSize = sendto(UDPSocketHandle, (char*) vceData.mThermalData, vceData.mHeader.mDataSize, 0,
        //                    (const struct sockaddr*) &ThermalDataAddress, sizeof(struct sockaddr_in));

        for(unsigned int i = 0; i < m_DataReceivedEventList.size(); ++i)
        {
        	if (m_DataReceivedEventList[i].type == LEPTON_PKGSEND2_UDPMEDIACLIENT){
        		//printf("UDPMEDIACLIENT\n");

				m_DataReceivedEventList[i].fnEvent(m_DataReceivedEventList[i].pContext,
					(axc_byte*)&(vceData.mHeader), vceData.mHeader.GetHeaderSize(), ADE2CAM_CHANNEL_LEPTONRAW);

				m_DataReceivedEventList[i].fnEvent(m_DataReceivedEventList[i].pContext,
					(axc_byte*)(vceData.mThermalData), vceData.mHeader.mDataSize, ADE2CAM_CHANNEL_LEPTONRAW);

        	}else if (m_DataReceivedEventList[i].type == LEPTON_PKGSEND2_PLUGINTARGET){
        		//printf("PLUGINTARGET: send %s \n", (m_send2plugin_pauseplay == true)?"enable":"disable");

				if (m_send2plugin_pauseplay == true){
					m_DataReceivedEventList[i].fnEvent(m_DataReceivedEventList[i].pContext,
						(axc_byte*)(vceData.mThermalData), vceData.mHeader.mDataSize, ADE2CAM_CHANNEL_LEPTONRAW);
				}
        	}else{
            	//CHeatFinderUtility::PrintNowData(1);
        		GLog(tCaptureThermalTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] unknown send to call back type: %u\n", "Lepton Thermal Capture", m_DataReceivedEventList[i].type);
        	}
        }

        //ref.: https://linux.die.net/man/3/send
        //if (sentSize <= 0){
        //    CHeatFinderUtility::MsgSendtoStrerror(errno);
        //}
        //else if (sentSize != (int)vceData.mHeader.mDataSize)
        //{
        //    //printf("W: [%s] UDP %d send thermal size= [%d] to ip= %ud port= %u ack received len= [%d]\n", __func__, UDPSocketHandle, sizeof(data), ThermalDataAddress.sin_addr.s_addr, ThermalDataAddress.sin_port, sentSize);
        //}
    }
}

void CHeatFinderLeptonCapture::OnReceivedHeatObjects(xPluginHeatObject *pPluginHeatObj)
{
	fireHeatObjectSendNotify(pPluginHeatObj);
}

void CHeatFinderLeptonCapture::fireLedStatusTwinkleNotify(const bool value)
{
    for(unsigned int i = 0; i < m_LedStatusTwinkleNotifyList.size(); ++i)
    {
        xLedStatusTwinkleNotifyHandler handler = m_LedStatusTwinkleNotifyList[i];
        handler.fnEvent(handler.pContext, value);
    }
}

void CHeatFinderLeptonCapture::fireThermalSizeChangedNotify(const unsigned int Width, const unsigned int Height)
{
    m_sizeThermalImage.cx = Width;
    m_sizeThermalImage.cy = Height;
    for(unsigned int i = 0; i < m_ThermalSizeChangedNotifyList.size(); ++i)
    {
        xThermalSizeChangedNotifyHandler handler = m_ThermalSizeChangedNotifyList[i];
        handler.fnEvent(handler.pContext, Width, Height);
    }
}

void CHeatFinderLeptonCapture::fireHeatObjectSendNotify(xPluginHeatObject *pPluginHeatObj)
{
    for(unsigned int i = 0; i < m_HeatObjectReceivedNotifyList.size(); ++i)
    {
    	xHeatObjectReceivedEventHandler handler = m_HeatObjectReceivedNotifyList[i];
        handler.fnEvent(handler.pContext, pPluginHeatObj);
    }
}

int CHeatFinderLeptonCapture::readMem(axc_byte* pData, axc_dword dwDataSize){
	m_queuelocker.lock();
	axc_byte* pCacheBegin = (axc_byte*) m_memThermalOutput.GetAddress();
    axc_dword dwMaxCacheLen = m_memThermalOutput.GetBufferSize();
    axc_dword dwRealReadLen = dwDataSize;

    if ((pData == NULL) || (dwRealReadLen < 5)){
    	//usleep(10*1000);
    	m_queuelocker.unlock();
    	return -1;
    }

    if(NULL == pCacheBegin || dwMaxCacheLen <= 0){
    	//usleep(10*1000);
    	m_queuelocker.unlock();
        return 0;
    }

    if(dwRealReadLen >= dwMaxCacheLen) {
    	printf("E [%s] read size (%u) >= max size (%u) \n", __func__, dwRealReadLen, dwMaxCacheLen);
        //usleep(10*1000);
    	m_queuelocker.unlock();
        return -1;
    } else if (dwRealReadLen > m_iQueueCacheLen) {
    	dwRealReadLen = m_iQueueCacheLen;
    }

    // read data by length
    memcpy(pData, pCacheBegin, dwRealReadLen);

	// move remain data to cahce-head
	CAxcMemory::SafeCopy(pCacheBegin, pCacheBegin + dwRealReadLen, m_iQueueCacheLen - dwRealReadLen);
	if (dwRealReadLen > m_iQueueCacheLen) {
		m_iQueueCacheLen = 0;
	} else {
		m_iQueueCacheLen -= dwRealReadLen;
	}

	m_queuelocker.unlock();
	return dwRealReadLen;
}

int CHeatFinderLeptonCapture::writeMem(axc_byte* pData, axc_dword dwDataSize){
	//save data to queue first ... path 1
	m_queuelocker.lock();
    axc_byte* pCacheBegin = (axc_byte*) m_memThermalOutput.GetAddress();
    axc_dword dwMaxCacheLen = m_memThermalOutput.GetBufferSize();

    if(NULL == pCacheBegin || dwMaxCacheLen <= 0){
    	m_queuelocker.unlock();
    	//usleep(10*1000);
        return -1;
    }

    if(dwDataSize > dwMaxCacheLen){
    	printf("E [%s] input size (%u) > max size (%u) \n", __func__, dwDataSize, dwMaxCacheLen);
    	m_iQueueCacheLen = 0;

    	m_queuelocker.unlock();
        //usleep(10*1000);
        return 0;
    }

    if((m_iQueueCacheLen + dwDataSize) > dwMaxCacheLen){
    	printf("E [%s] cache size (%u) + input size (%u) > max size (%u) \n", __func__, m_iQueueCacheLen, dwDataSize, dwMaxCacheLen);
    	m_iQueueCacheLen = 0;     //lose old data?
    }

    memcpy(pCacheBegin + m_iQueueCacheLen, pData, dwDataSize);
    m_iQueueCacheLen += dwDataSize;
	m_queuelocker.unlock();

	return dwDataSize;
}
