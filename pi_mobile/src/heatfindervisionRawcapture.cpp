/*
 * heatfindervisionRawcapture.cpp
 *
 *  Created on: Jan 26, 2018
 *      Author: markhsieh
 */

#include "heatfindervisionRawcapture.h"

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

CHeatfindervisionRawcapture::CHeatfindervisionRawcapture():
	m_threadVisionRawCatch("vision/rawcap"),
	m_bStopThread(false)
{
	// TODO Auto-generated constructor stub
	m_fdVisionRawFifo = -1;
	m_sizeVisionMainImage = {0,0};

}

CHeatfindervisionRawcapture::~CHeatfindervisionRawcapture() {
	// TODO Auto-generated destructor stub
    printf("D: [%s] Try to Destroy\n", "Vision Raw Capture");
    m_bStopThread = true;
    if (m_threadVisionRawCatch.IsValid() == axc_true)
    	m_threadVisionRawCatch.Destroy(2000);

    printf("D: [%s] Destroy\n", "Vision Raw Capture");
}

void CHeatfindervisionRawcapture::setOwner(void *pOwner)
{
    m_pOwner = pOwner;
}

bool CHeatfindervisionRawcapture::run()
{
    bool bRst = false;
    //stop();
	//Raw_VisionCapture();
    m_bStopThread = false;
    if (!m_threadVisionRawCatch.Create(thread_process, this))
        return bRst;
    printf("D: [%s] Start\n", "Vision Raw Capture");

    bRst = true;
    return bRst;
}

void CHeatfindervisionRawcapture::stop()
{
    printf("D: [%s] Try to Stop\n", "Vision Raw Capture");
    m_bStopThread = true;
    if (m_threadVisionRawCatch.IsValid() == axc_true)
    	m_threadVisionRawCatch.Destroy(2000);

    printf("D: [%s] Stop\n", "Vision Raw Capture");

}


void CHeatfindervisionRawcapture::AddReceivedCameraRawNotify(OnReceivedCameraRawFrame fnEvent, void *pContext)
{
	xOnReceivedCameraRawFrameNotifyHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;

    m_ReceivedCameraRawnotifyhandlerList.push_back(handler);
}

void CHeatfindervisionRawcapture::AddSetThermalOverlayNotify(OnSetThermalOverlay fnEvent, void *pContext)
{
	xSetOnTimeThermalOverlayNotifyHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;

    m_SetThermalOverlaynotifyhandlerList.push_back(handler);
}

void CHeatfindervisionRawcapture::fireReceivedCameraRawFrameNotify(const int Width, const int Height, unsigned char *pRGB){
	if (m_ReceivedCameraRawnotifyhandlerList.size() <=0){
			return;
	}

	for(unsigned int i = 0; i < m_ReceivedCameraRawnotifyhandlerList.size(); ++i)
	{

		m_ReceivedCameraRawnotifyhandlerList[i].fnEvent(m_ReceivedCameraRawnotifyhandlerList[i].pContext, Width, Height, pRGB);

	}
}

void CHeatfindervisionRawcapture::fireSetThermalOverlayNotify(xLeptonOverlay &xLoi){
	if (m_SetThermalOverlaynotifyhandlerList.size() <=0){
		return;
	}

	for(unsigned int i = 0; i < m_SetThermalOverlaynotifyhandlerList.size(); ++i)
	{

		m_SetThermalOverlaynotifyhandlerList[i].fnEvent(m_SetThermalOverlaynotifyhandlerList[i].pContext, xLoi);

	}
}

void CHeatfindervisionRawcapture::OnRenewOverlay(){
	GlobalDef *pGlobal = (GlobalDef *) CHeatFinderUtility::GetGlobalsupport();
	xLeptonOverlay xLoi;

	pGlobal->GetVisioResolution(m_sizeVisionMainImage.cx, m_sizeVisionMainImage.cy);
	pGlobal->GetOverlaybyResolution(m_sizeVisionMainImage.cx, m_sizeVisionMainImage.cy, xLoi);

	//m_funcSetThermalOverlay(m_pOwner, xLoi);
	fireSetThermalOverlayNotify(xLoi);
}

void CHeatfindervisionRawcapture::Raw_VisionCapture(){
	//creating height normalization and total size limit
	unsigned int l_iHeight = 0, l_iWidth = 0;
	bool l_bIsReady = false, l_bIsError = false, l_bIsRestarted = true;
	int l_iReadLen = 0;
	unsigned long long int l_lliMaxSize =0;
	//ade_camera_sdk *pADECamSDK = (ade_camera_sdk *) CHeatFinderUtility::GetADECameraSDK();
	GlobalDef *pGlobal = (GlobalDef *) CHeatFinderUtility::GetGlobalsupport();
	int m_fdVisionRawFifo = 0; // pADECamSDK->get_raw_color_fd();
	bool	l_bShowDebugMsgOnce = false;
	double	l_fLastResolutionChkMSTime = 0.0;

	printf("D: [Camera RawRGB] %s init.... fd:%d \n", __func__, m_fdVisionRawFifo);

	//read(m_fdVisionRawFifo, abyReadBuffer, sizeof(abyReadBuffer));
	//raspivid -r ... -rf rgb  >>> RGB888
	/**
	 * Height = 16*Y
	 * 1080 = 16*67+8 -> 16*68 = 1088
	 * 720 = 16*45
	 * 360 = 16*22+8 -> 16*23 = 368
	 * Width = 32*X
	 * 1920 = 32*60
	 * 1280 = 32*40
	 * 640 = 32*20
	 */
	unsigned int Large_w=1920, Large_h=1088; //6220800 ~= 5M   (workable)
	unsigned int Mid_w=1280, Mid_h=720; //2764800 ~= 2M
	unsigned int Small_w=640, Small_h=368; //691200 ~= 675K   (workable)

	/**
	 * collect data to std::vector<BYTE> first, then copty to buffer
	 */
	// see dev. detail: adehf-detect/src/module_vision.cpp
	int BUF_SIZE = 4096;  //only 4kb
	axc_byte RecvBuffer[BUF_SIZE];
	std::vector<BYTE> bufList;
	printf("D: [Camera RawRGB] %s init. ... memory finish \n", __func__);

	l_fLastResolutionChkMSTime = CAxcTime::GetCurrentUTCTimeMs();
	while (!m_bStopThread ){
/*
		if (false == m_funcChkisReqRGBStream(m_pOwner)){
			//waiting the call
			isRestart = true;
			usleep(500 * 1000);
			continue;
		}
        else
*/ //hamlet
		double l_fCurrentTimeRecord = CAxcTime::GetCurrentUTCTimeMs();

		if (m_fdVisionRawFifo < 0){
			m_threadVisionRawCatch.SleepMilliSeconds(1*1000);
			m_fdVisionRawFifo = 0; //pADECamSDK->get_raw_color_fd();
			continue;

		}else{
			//init. let's get a new photo.
			//Data_total_size = 0;
			l_bIsError = false;
			if (l_bIsRestarted){
				l_bIsRestarted = false;
				m_fdVisionRawFifo = 0;// pADECamSDK->get_raw_color_fd();
				printf("D: [Camera RawRGB] %s change fd:%d \n", __func__, m_fdVisionRawFifo);

				//overlay renew & fire to this object owner
				//m_funcSetThermalOverlay(m_pOwner);
				OnRenewOverlay();
				l_fLastResolutionChkMSTime = CAxcTime::GetCurrentUTCTimeMs();
			}else{
				// Per 1 sec.
				if ((l_fCurrentTimeRecord - l_fLastResolutionChkMSTime) > 1.0){
					// Sync Vision Resolution
					pGlobal->GetVisioResolution(m_sizeVisionMainImage.cx, m_sizeVisionMainImage.cy);
					l_fLastResolutionChkMSTime = CAxcTime::GetCurrentUTCTimeMs();
				}
			}
		}



		if (m_sizeVisionMainImage.cx == 1920){
			if (m_sizeVisionMainImage.cx != l_iWidth){
				l_iHeight = Large_h;
				l_iWidth = Large_w;
				//Data_size_per_row = 3*Large_w;
			}
			l_bShowDebugMsgOnce = false;
			//abyReadBuffer_point = &abyReadBuffer_L[0];
		}else if (m_sizeVisionMainImage.cx == 1280){
			if (m_sizeVisionMainImage.cx != l_iWidth){
				l_iHeight = Mid_h;
				l_iWidth = Mid_w;
				//Data_size_per_row = 3*Mid_w;
			}
			l_bShowDebugMsgOnce = false;
			//abyReadBuffer_point = &abyReadBuffer_M[0];
		}else if (m_sizeVisionMainImage.cx == 640){
			if (m_sizeVisionMainImage.cx != l_iWidth){
				l_iHeight = Small_h;
				l_iWidth = Small_w;
				//Data_size_per_row = 3*Small_w;
			}
			l_bShowDebugMsgOnce = false;
			//abyReadBuffer_point = &abyReadBuffer_S[0];
		}else if (l_bShowDebugMsgOnce != true){
			l_bShowDebugMsgOnce = true;
			printf("W: [Camera RawRGB] sorry, we do not support this size:(%d, %d) Raw(RGB888) data\n", m_sizeVisionMainImage.cx, m_sizeVisionMainImage.cy);
			m_threadVisionRawCatch.SleepMilliSeconds(100);
			continue;
		}else{
			//usleep(20 * 1000);
			m_threadVisionRawCatch.SleepMilliSeconds(20);
			continue;
		}

		//init.
		l_bIsReady = false;
		l_lliMaxSize = (l_iHeight *l_iWidth *3);
		l_iReadLen =0;
		//isRestart =false;
		l_bIsError =false;
		//printf("V: [Camera RawRGB] %s photo captured size W:%d H:%d total:%llu\n", __func__, Width, Height, Max_size);

		//unsigned int catch_length =0;
		while (true){
			if(m_bStopThread){
				l_bIsReady = false;
				printf("D: [Camera RawRGB] %s stop capturing view because system shutdown -- drop len:%u\n", __func__, bufList.size());
				//Data_total_size = 0;
				bufList.erase(bufList.begin(), bufList.end());
				break;
			}

			if (m_fdVisionRawFifo <0){
		    	CHeatFinderUtility::PrintNowData(1);
				printf("E: [Camera RawRGB] read raw fail, fd: %d\n", m_fdVisionRawFifo);
				l_iReadLen =0;
				l_bIsError = true;

			}

			/*l_isRASPIVIDwork*/
/*
			if (false == m_funcChkisReqRGBStream(m_pOwner)){
				//shutdown capture raw data
				isReady = false;
				printf("N: [Camera RawRGB] %s stop capturing view because option remove -- drop len:%u\n", __func__, bufList.size());
				//Data_total_size = 0;
				//drop before data
				//memset(abyReadBuffer_point, 0, (Height *Data_size_per_row));
				bufList.erase(bufList.begin(), bufList.begin() + bufList.size());
				break;
			}else */if (l_bIsError == true){
				l_bIsReady = false;
				l_bIsRestarted = true;
				printf("W: [Camera RawRGB] %s stop capturing view because error happen -- drop len:%u\n", __func__, bufList.size());
				//Data_total_size = 0;
				//drop before data
				//memset(abyReadBuffer_point, 0, (Height *Data_size_per_row));
				bufList.erase(bufList.begin(), bufList.begin() + bufList.size());
				//m_threadVisionRawCatch.SleepMilliSeconds(20);
				break;
			}
 //hamlet
			if (bufList.size() >= l_lliMaxSize ){
				l_bIsReady = true;
				break;
			}

			l_iReadLen = 0;
			try{

				l_iReadLen = read(m_fdVisionRawFifo, RecvBuffer, BUF_SIZE);
			}
			catch(...)
			{
		    	CHeatFinderUtility::PrintNowData(1);
				printf("E: [Camera RawRGB] read raw fail, %d: %s\n", errno, strerror(errno));
				l_iReadLen =0;
				l_bIsError = true;
			}

			//iReadLen = read(m_fdVisionRawFifo, abyReadBuffer_L, Data_size_per_row);
			if (l_iReadLen < 0){
				//CHeatFinderUtility::MsgSendtoStrerror(errno);
				l_bIsError = true;
				continue;
			}else if (l_iReadLen == 0){
				//usleep(20 * 1000);
				m_threadVisionRawCatch.SleepMilliSeconds(10);
				//printf("V: [Camera RawRGB] %s wait new data to fill with. %llu/%llu ---> 1\n", __func__, bufList.size(), Max_size);
				continue;
			}else{
				//AxcLogV(g_log, "[Camera RawRGB] %s get len: %llu + %d\n", __func__, Data_total_size, iReadLen);
				//if (file_input_demo == false){
				bufList.insert(bufList.end(), RecvBuffer, RecvBuffer + l_iReadLen);

			}
		}

		if (l_bIsReady == false){
			m_threadVisionRawCatch.SleepMilliSeconds(20);
			continue;
		}else{
			printf("N: [Camera RawRGB] %s pass size %u \n", __func__, bufList.size());
		}

		if (l_iWidth == Large_w){
			axc_byte abyReadBuffer_L[(3*Large_w*Large_h)];
			std::fill_n(abyReadBuffer_L, l_lliMaxSize, 0);

			std::copy(bufList.begin(), bufList.begin() + l_lliMaxSize, abyReadBuffer_L);
			//m_funcReceivedCameraRaw(m_pOwner, (int) Width, (int) Height, (unsigned char *)abyReadBuffer_L);
			fireReceivedCameraRawFrameNotify((int) l_iWidth, (int) l_iHeight, (unsigned char *)abyReadBuffer_L);

		}else if (l_iWidth == Mid_w){
			axc_byte abyReadBuffer_M[(3*Mid_w*Mid_h)];
			std::copy(bufList.begin(), bufList.begin() + l_lliMaxSize, abyReadBuffer_M);
			//m_funcReceivedCameraRaw(m_pOwner, (int) Width, (int) Height, (unsigned char *)abyReadBuffer_M);
			fireReceivedCameraRawFrameNotify((int) l_iWidth, (int) l_iHeight, (unsigned char *)abyReadBuffer_M);

		}else if (l_iWidth == Small_w){
			axc_byte abyReadBuffer_S[(3*Small_w*Small_h)];
			std::fill_n(abyReadBuffer_S, l_lliMaxSize, 0);

			std::copy(bufList.begin(), bufList.begin() + l_lliMaxSize, abyReadBuffer_S);
			//m_funcReceivedCameraRaw(m_pOwner, (int) Width, (int) Height, (unsigned char *)abyReadBuffer_S);
			fireReceivedCameraRawFrameNotify((int) l_iWidth, (int) l_iHeight, (unsigned char *)abyReadBuffer_S);
		}
		bufList.erase(bufList.begin(), bufList.begin() + l_lliMaxSize);

		//debug: control the cpu loading
		//base on "Experimental record", (1920*1088*3 * 1Mbps) 10~8 pictures / per 1 second
		//we sleep 100ms to down the frequence to 5~4 pictures / per 1 second  <---- cpu loading 47% +-5
		//m_threadVisionRawCatch.SleepMilliSeconds(100);
		//we sleep 200ms to down the frequence to 4~3 pictures / per 1 second  <---- cpu loading 36.5% +-2
		//m_threadVisionRawCatch.SleepMilliSeconds(200);
		//we sleep 300ms to down the frequence to 3~2 pictures / per 1 second  <---- cpu loading 30.5% +-2 ?
		//m_threadVisionRawCatch.SleepMilliSeconds(300);
		//we sleep 400ms to down the frequence to 2~2 pictures / per 1 second  <---- cpu loading 25.5% +-2 ?
		m_threadVisionRawCatch.SleepMilliSeconds(400);
	}
	printf("D: [Camera RawRGB] %s close & leave \n", __func__);
}


axc_dword CHeatfindervisionRawcapture::thread_process()
{

	Raw_VisionCapture();
	return 1;
}
