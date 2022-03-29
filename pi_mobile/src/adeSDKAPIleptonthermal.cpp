/*
 * adeSDKAPIleptonthermal.cpp
 *
 *  Created on: Feb 1, 2018
 *      Author: markhsieh
 */

#include "adeSDKAPIleptonthermal.h"
#include "heatfinderconfigmanager.h"
#include <linux/spi/spidev.h>

#ifndef IS_BORDER
#define IS_BORDER(x,y)	(RawDataAnalyze[y*l_iWidth+x] <= wGapValue)
#endif

/// public
	//m_threadUpdateThermalIvs("thermal/update/ivs"),
	//m_threadReadLeptonRawData("thermal/read/raw"),
CadeSDKAPIleptonthermal::CadeSDKAPIleptonthermal():
	m_caxcmemThermalCompress("thermal/compress"),
	m_caxcmemThermalIvsRawDataAnalyze("thermal/ivs_analyze"),
	m_caxcfifobufferThermalIvs("thermal/fifobuffer"),
	m_bStopAllThread(false),
	m_bPauseReading(false)
{
	m_byLogLevel = 3;
	m_iCountNoData = 0;
	m_fEmissivity.fEmissivity = 1.0;
	m_fEmissivity.dLastUpdateTime = 0.0; //shell sync to real time
	m_iSceneEmissivity = 0;
	m_fLastFrameSecondTime = 0.0;

	m_bInitReady = false;

	//m_iWidth = m_iHeight = 0;
	m_bStartAnalyzeHeatObjectOnce = false;
	m_fLastTimeAnalyzeHeatObject = 0.0;
	m_fIntervalOfAnalyzeHeatObject = 0.0; //default 0 sec., which mean 'no' analysis action.

	m_iCPUBindNumber = LEPTON_SDK_CPUBIND_AUTO;

	m_interval = LEPTON_SDK_DEFAULT_FAIL_INTERVAL;

	m_iOpenIniNumber = 0;
}

CadeSDKAPIleptonthermal::~CadeSDKAPIleptonthermal()
{
	adethermal_device_close();
	adethermal_sdk_uninit();
	m_bInitReady = false;
	m_bPauseReading = false;
    m_bStopAllThread = true;

    //if (m_threadUpdateThermalIvs.IsValid())
    //	m_threadUpdateThermalIvs.Destroy(2000);
    //if (m_threadReadLeptonRawData.IsValid())
    //	m_threadReadLeptonRawData.Destroy(2000);

	while(m_caxcfifobufferThermalIvs.Pop(axc_true))
	{
		//pop every element and throw them to universe
	}
	m_caxcfifobufferThermalIvs.Destroy();

	m_caxcmemThermalCompress.Free();
	m_caxcmemThermalIvsRawDataAnalyze.Free();

	GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Thermal API] Destroy \n");
}

bool CadeSDKAPIleptonthermal::run()
{
	bool bRst = false;
    m_bStopAllThread = false;

    // init.
    m_PlugInHeatObjects.wHeatObj = 0;
    int i = 0; //MAXHEATOBJECTNUMBER;
    for (i = 0; i < MAXHEATOBJECTNUMBER; i++){
    	m_PlugInHeatObjects.HeatObjList[i].wTempe = 0;
    	m_PlugInHeatObjects.HeatObjList[i].posX = 0;
    	m_PlugInHeatObjects.HeatObjList[i].posY = 0;
    }

	//GlobalDef *pGlobal = (GlobalDef *) CHeatFinderUtility::GetGlobalsupport();
	/// @brief 'T_CONFIG_FILE_OPTIONS' all display setting
	Cheatfinderconfigmanager *pConfigContextManager = (Cheatfinderconfigmanager *)CHeatFinderUtility::GetConfigObj();
	T_CONFIG_FILE_OPTIONS *pConfigContext = ((pConfigContextManager == NULL)?NULL:pConfigContextManager->GetConfigContext());

	///FIXME: log level shell follow the config file!
	///m_byLogLevel = cfg.dwLogLevel
	unsigned int l_iLogLevel = 0;
	if (pConfigContext != NULL) {
		l_iLogLevel = pConfigContext->dwLogLevel;
	} else {
		GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Thermal API] global config not ready\n");
	}
	switch(l_iLogLevel)
	{
	case AXC_LOG_LEVEL_VERBOSE:		m_byLogLevel = 3; break;
	case AXC_LOG_LEVEL_NORMAL: 		m_byLogLevel = 3; break;
	case AXC_LOG_LEVEL_WARN:		m_byLogLevel = 2; break;
	case AXC_LOG_LEVEL_DEBUG:
	case AXC_LOG_LEVEL_ERROR:
	case AXC_LOG_LEVEL_FATAL:		m_byLogLevel = 1; break;
	case AXC_LOG_LEVEL_DISABLE:
	default:						m_byLogLevel = 0; break;
	}
	adethermal_sdk_init(m_byLogLevel, sdk_log_callback, NULL);

	// open device
	// T_ADETHERMAL_CONFIG config;
	// memset(&config, 0, sizeof(config));
	// config.config_size = sizeof(config);
	// if (pConfigContext !=NULL){
	// 	CAxcString::strncpy(config.i2c_device, pConfigContext->szThermalI2cDevice, (axc_dword)sizeof(config.i2c_device));
	// 	CAxcString::strncpy(config.spi_device, pConfigContext->szThermalSpiDevice, (axc_dword)sizeof(config.spi_device));
	// 	CAxcString::strncpy(config.uart_device, pConfigContext->szThermalUartDevice, (axc_dword)sizeof(config.uart_device));  // Hank
	// 	config.spi_speed = pConfigContext->dwThermalSpiSpeed;
	// 	config.spi_rx_mode = (uint8_t) pConfigContext->dwThermalSpiMode;
	// 	config.spi_rx_bits = (uint8_t) pConfigContext->dwThermalSpiBits;
	// 	config.spi_word_swap = (uint8_t) pConfigContext->bThermalSpiSwapWord;
	// 	config.spi_byte_swap = (uint8_t) pConfigContext->bThermalSpiSwapByte;
	// 	config.optional = (uint8_t) pConfigContext->ucThermalOptional;
	// 	GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Thermal API] open device from pConfigContext\n");
	// }else{
	// 	//use default of stander product base on Raspberry PI3 OS: '2017-04-10-raspbian-jessie'
	// 	CAxcString::strncpy(config.i2c_device, "", (axc_dword)sizeof(config.i2c_device));
	// 	CAxcString::strncpy(config.spi_device, "/dev/spidev0.0", (axc_dword)sizeof(config.spi_device));
	// 	CAxcString::strncpy(config.uart_device, "/dev/serial0", (axc_dword)sizeof(config.uart_device));  // Hank
	// 	config.spi_speed = 10000000;
	// 	config.spi_rx_mode = 3;
	// 	config.spi_rx_bits = 8;
	// 	config.spi_word_swap = axc_true;
	// 	config.spi_byte_swap = axc_false;
	// 	config.optional = 0x00;
	// 	GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Thermal API] open device ready\n");
	// }
	// config.thread_priority = 0 /*THERMAL_SPI_RW_THREAD_PRIORITY*/;
	// config.mirror_downup = 0;
	// config.mirror_leftright = 0;

	T_ADETHERMAL_CONFIG config = { sizeof(T_ADETHERMAL_CONFIG),
                                   "",
                                   "/dev/spidev0.0",
                                   "/dev/serial0",
                                   10000000,
                                   SPI_MODE_0,
                                   8,
                                   1,
                                   0,
                                   0,
                                   0,
                                   0,
                                   0
                                 };

	/** 2.1.1.12 start: update selection of open stream */
	if((pConfigContext->iOpenStream & OPEN_THERMAL_STREAM)==0){
		GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Thermal API] LeptonSDK suspended by adehf.ini open_stream configuration. (did not invoke adethermal_device_open)\n");

		m_fLastFrameSecondTime = getCurrentTime();

		if(!m_caxcfifobufferThermalIvs.Create(3))
		{
			//CHeatFinderUtility::PrintNowData(1);
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [Thermal API] failed to create thermal-ivs internal memory buffer, error: %s\n", AxcGetLastErrorText());
			return bRst;
		}

		//printf("D: [Thermal API] sdk open 4\n");
		const uint32_t dwSdkVer = adethermal_sdk_get_version();
		GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Thermal API] sdk v%u.%u, succeed open on '%s' and '%s' and '%s'\n", dwSdkVer>>16, dwSdkVer&0xFFFF, config.i2c_device, config.spi_device, config.uart_device);

		bRst = true;
		return bRst;
	}
	/** 2.1.1.12 end */

	const int nTotalRetry = 10;
	const int nRetryInterval = 200000;	// 200ms
	for (int nRetry = 0; nRetry < nTotalRetry; nRetry++) {
		GLog(tAll, tDEBUGTrace_MSK, "D: [Thermal API] %s [Gavin]invoke adethermal_device_open -%d/%d- with 'config.optional=%d'\n", __func__, nRetry+1, nTotalRetry, config.optional);
		const E_ADETHERMAL_RESULT eResult = adethermal_device_open(&config);
		//const E_ADETHERMAL_RESULT eResult = ADETHERMAL_RESULT_SUCCEED;
		int iErrRst = sdk_error_reader(eResult, &config);
		//int iErrRst = 0;
		if (iErrRst > 0){
			//CHeatFinderUtility::PrintNowData(1);
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [Thermal API] %s [Gavin]sdk run fail. -%d/%d-  thermal error code:%d\n", __func__, nRetry+1, nTotalRetry, eResult);
			if( nRetry + 1 == nTotalRetry ) {
				GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [Thermal API] =====================================\n");
				GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [Thermal API] From now on thermal was STOP.\n\n");
				bRst = false;
				return bRst;
			}
			else {
				usleep(nRetryInterval);
			}
		}else{
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Thermal API] %s [Gavin]sdk open success -%d/%d-\n", __func__, nRetry+1, nTotalRetry);
			m_bInitReady = true;

			if (config.thread_priority > 5){
				m_iCPUBindNumber = LEPTON_SDK_CPUBIND_DEFAULT;
			}else{
				m_iCPUBindNumber = LEPTON_SDK_CPUBIND_AUTO;
			}
			break;
		}
	}

	bool bRet = clickedThermalCPUBind(m_iCPUBindNumber);
	if (!bRet){
		GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Thermal API] set cpu bind fail\n");
	}
	bRet = setThermalEmissivity((float)pConfigContext->dwEmissivity/1000.0);
	if (!bRet){
		GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Thermal API] set emissivity fail\n");
	} else {
		m_fEmissivity.fEmissivity = (float)pConfigContext->dwEmissivity/1000.0;
		m_fEmissivity.dLastUpdateTime = getCurrentTime();
	}

	// check direct throw out device does exist?
	if (m_DataSendNotifyEventList.size() <= 0){
		GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Thermal API] miss thermal data direct output setting (MediaSer/Plugin)\n");
		//bRst = false;
		//return bRst;
	}
	//printf("D: [Thermal API] sdk open 1\n");
	m_fLastFrameSecondTime = getCurrentTime();

	// thermal ivs fifo & thread
	//AxcSetLastError(0);
	if(!m_caxcfifobufferThermalIvs.Create(3))
	{
		//CHeatFinderUtility::PrintNowData(1);
		GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [Thermal API] failed to create thermal-ivs internal memory buffer, error: %s\n", AxcGetLastErrorText());
		return bRst;
	}

	//printf("D: [Thermal API] sdk open 4\n");
	const uint32_t dwSdkVer = adethermal_sdk_get_version();
	GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Thermal API] sdk v%u.%u, succeed open on '%s' and '%s' and '%s'\n", dwSdkVer>>16, dwSdkVer&0xFFFF, config.i2c_device, config.spi_device, config.uart_device);

	bRst = true;
	return bRst;
}

void CadeSDKAPIleptonthermal::stop(){
	adethermal_device_close();
	adethermal_sdk_uninit();
	m_bInitReady = false;
	m_bPauseReading = false;
    m_bStopAllThread = true;

	while(m_caxcfifobufferThermalIvs.Pop(axc_true))
	{
		//pop every element and throw them to universe
	}
	m_caxcfifobufferThermalIvs.Destroy();
    //if (m_threadUpdateThermalIvs.IsValid())
    //	m_threadUpdateThermalIvs.Destroy(2000);
    //if (m_threadReadLeptonRawData.IsValid())
    //	m_threadReadLeptonRawData.Destroy(2000);

	GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Thermal API] Stop device\n");
}

void CadeSDKAPIleptonthermal::AddDataSendNotifyEvent(OnDataSendNotifyEvent fnEvent, void *pContext){
	xDataSendNotifyEventHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;
    m_locker.lock();
    m_DataSendNotifyEventList.push_back(handler);
    m_locker.unlock();
}

void CadeSDKAPIleptonthermal::fireDataSendNotify(axc_byte* pData, axc_dword dwDataSize, T_ADETHERMAL_RAWDATA_FRAME *raw)
{
	unsigned int l_iElementCount = 0;
    m_locker.lock();
	if (m_DataSendNotifyEventList.size() <= 0){
		return;
	}else{
		l_iElementCount = m_DataSendNotifyEventList.size();
	}
	m_locker.unlock();

    for(unsigned int i = 0; i < l_iElementCount; i++)
    {
    	m_DataSendNotifyEventList[i].fnEvent(m_DataSendNotifyEventList[i].pContext, pData, dwDataSize, raw);

    }
}

void CadeSDKAPIleptonthermal::AddHeatObjectSendNotifyEvent(OnHeatObjectSendNotifyEvent fnEvent, void *pContext)
{
	xHeatObjectSendNotifyEventHandler handler;
	handler.fnEvent = fnEvent;
	handler.pContext = pContext;
	m_locker_heatobj.lock();
	m_HeatObjectSendNotifyEventList.push_back(handler);

	m_locker_heatobj.unlock();
}

void CadeSDKAPIleptonthermal::fireHeatObjectSendNotify()
{
	unsigned int l_iElementCount = 0;
	m_locker_heatobj.lock();
	if (m_HeatObjectSendNotifyEventList.size() <= 0) {
		return;
	} else {
		l_iElementCount = m_HeatObjectSendNotifyEventList.size();
	}
	m_locker_heatobj.unlock();

	xPluginHeatObject _PluginHeatObjs;
	_PluginHeatObjs.wHeatObj = m_PlugInHeatObjects.wHeatObj;
	for (unsigned short iIndex = 0; iIndex < MAXHEATOBJECTNUMBER; iIndex++){
		if (iIndex > (_PluginHeatObjs.wHeatObj - 1)) {
			_PluginHeatObjs.HeatObjList[iIndex].wTempe = 0;
			_PluginHeatObjs.HeatObjList[iIndex].posX = 0;
			_PluginHeatObjs.HeatObjList[iIndex].posY = 0;
			continue;
		}
		_PluginHeatObjs.HeatObjList[iIndex].wTempe = m_PlugInHeatObjects.HeatObjList[iIndex].wTempe;
		_PluginHeatObjs.HeatObjList[iIndex].posX = m_PlugInHeatObjects.HeatObjList[iIndex].posX;
		_PluginHeatObjs.HeatObjList[iIndex].posY = m_PlugInHeatObjects.HeatObjList[iIndex].posY;
	}

    for(unsigned int i = 0; i < l_iElementCount; i++)
    {
    	m_HeatObjectSendNotifyEventList[i].fnEvent(m_HeatObjectSendNotifyEventList[i].pContext, &_PluginHeatObjs);
    }
}

void CadeSDKAPIleptonthermal::AddReceivedThermalRestartFinishNotifyEvent(OnReceivedThermalRestartFinishNotifyEvent fnEvent, void *pContext)
{
	xReceivedThermalRestartFinishNotifyEventHandler handler;
	handler.fnEvent = fnEvent;
	handler.pContext = pContext;
	m_locker_restart_notify.lock();
	m_ReceivedThermalRestartFinishNotifyEventList.push_back(handler);

	m_locker_restart_notify.unlock();
}

void CadeSDKAPIleptonthermal::fireReceivedThermalRestartFinishNotify()
{
	m_locker_restart_notify.lock();
	if (m_ReceivedThermalRestartFinishNotifyEventList.size() <= 0)
	{
		return;
	}

    for(unsigned int i = 0; i < m_ReceivedThermalRestartFinishNotifyEventList.size(); i++)
    {
    	m_ReceivedThermalRestartFinishNotifyEventList[i].fnEvent(m_ReceivedThermalRestartFinishNotifyEventList[i].pContext);
    }

	m_locker_restart_notify.unlock();
}

int	CadeSDKAPIleptonthermal::getThermalIvsHeatObjCount()
{
	GlobalDef *pGlobal = (GlobalDef *) CHeatFinderUtility::GetGlobalsupport();
	T_THERMAL_IVS_OUTPUT *l_pThermalIvsOut = pGlobal->GetThermalIvsOutSavingTable();
	return (int)l_pThermalIvsOut->wHeatObj;
}

float CadeSDKAPIleptonthermal::getThermalEmissivity()
{
	Cheatfinderconfigmanager *pConfigContextManager = (Cheatfinderconfigmanager *)CHeatFinderUtility::GetConfigObj();
	T_CONFIG_FILE_OPTIONS *pConfigContext = ((pConfigContextManager == NULL)?NULL:pConfigContextManager->GetConfigContext());
	float fRst = 0.0;
	fRst = getLeptonEmissivity();
	if (fRst == 0.0) {
		if(pConfigContext) {
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Thermal API] return emissivity value %.1f by camera_config.json, because lepton sdk return value:%f (shell be more than 0.0 and less than 1.0)\n", (float)pConfigContext->dwEmissivity/1000.0, fRst);
			fRst = (float)pConfigContext->dwEmissivity/1000.0;
		} else {
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Thermal API] return default emissivity value 1.0, because lepton sdk return value:%f (shell be more than 0.0 and less than 1.0)\n", fRst);
			fRst = 1.0;
			pConfigContext->dwEmissivity = 1000;
		}
	} else {
		if(pConfigContext) {
			axc_dword readValue = (axc_dword)(fRst*1000);
			if( pConfigContext->dwEmissivity != readValue ) {
				GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Thermal API] return emissivity value %d (%.1f ---> %.1f)\n", (float)pConfigContext->dwEmissivity/1000.0, fRst);
				pConfigContext->dwEmissivity = readValue;
			}
		}
	}
	return fRst;
}

double CadeSDKAPIleptonthermal::getLastFrameMs(){
	return m_fLastFrameSecondTime;
}

/// protected
float CadeSDKAPIleptonthermal::getLeptonEmissivity()
{
	GLog(tAll, tDEBUGTrace_MSK, "D: [Thermal API] +++CadeSDKAPIleptonthermal::getLeptonEmissivity\n");
	float fRst = 0.0;
	double dCurrentTime = getCurrentTime();
	if((m_fEmissivity.dLastUpdateTime <= 0.0)
		|| (m_fEmissivity.fEmissivity <= 0.0)
		|| ((dCurrentTime - m_fEmissivity.dLastUpdateTime)>10.0)){
		///need update value first
		///@parameter CMD_MODE=CMD_GETEMI, iValue&bValue don't care
		if (OnParserThermalCommand(CMD_GETEMI, 0, false) == false){
			fRst = 0.0;
		}else{
			fRst = m_fEmissivity.fEmissivity;
		}
	}
	GLog(tAll, tDEBUGTrace_MSK, "D: [Thermal API] +++CadeSDKAPIleptonthermal::getLeptonEmissivity ...%.1f\n", fRst);
	return fRst;
}

/**
 * return Current UTC time (second)
 * 带毫秒数的系统UTC时间，整数位为秒数，小数位为毫秒数
 */
double CadeSDKAPIleptonthermal::getCurrentTime(){
	return CAxcTime::GetCurrentUTCTimeMs();
}

bool CadeSDKAPIleptonthermal::setThermalEmissivity(float fValue){
	bool bRst = false;
	if(OnParserThermalCommand(CMD_EMISSIVITY, (int)(fValue*1000), false) == true){
		bRst = true;
	}
	return bRst;
}

bool CadeSDKAPIleptonthermal::setIntervalValue(double fInterval){
	// has limitation
	if (fInterval < 10.0){
		GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [Thermal API] Interval time can not less than 10 ms pre reading action. You input %.03lf\n", fInterval);
		return false;
	}
	m_interval = fInterval;
	return true;
}

bool CadeSDKAPIleptonthermal::setIntervalOfIdleStatusAnalyzeHeatObject(double fInterval){
	bool _bRst = false;
	if (fInterval < 0){
		_bRst = false;
	}else if (fInterval == 0){
		m_fIntervalOfAnalyzeHeatObject = (m_fIntervalOfAnalyzeHeatObject == 0)?m_fIntervalOfAnalyzeHeatObject:0.0;
		_bRst = false;
	}else if (m_fIntervalOfAnalyzeHeatObject != fInterval){
		m_fIntervalOfAnalyzeHeatObject = fInterval;
		_bRst = true;
	}else{
		//fInterval: 1. >=0, 2.!=0, 3. the same. ------> already enable Analyze Heat Object when idle.
		_bRst = true;
	}

	if (_bRst == false){
		GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Thermal API] new interval: '%s' so return false\n",
				((fInterval <= 0)?"small than/equal zero":"OK"));
	}
	return _bRst;
}

bool CadeSDKAPIleptonthermal::clickedThermalRADEnable(){
	bool bRst = false;
	if(OnParserThermalCommand(CMD_RAD_ENABLE, 0, false) == true){
		bRst = true;
	}
	return bRst;
}

bool CadeSDKAPIleptonthermal::clickedThermalRADDisable(){
	bool bRst = false;
	if(OnParserThermalCommand(CMD_RAD_DISABLE, 0, false) == true){
		bRst = true;
	}
	return bRst;
}
bool CadeSDKAPIleptonthermal::clickedThermalFFC(){
	bool bRst = false;
	if(OnParserThermalCommand(CMD_FFC, 0, false) == true){
		bRst = true;
	}
	return bRst;
}

/**
 * only reopen by SDK
 */
bool CadeSDKAPIleptonthermal::clickedThermalREBOOT(){
	bool bRst = false;
	if(OnParserThermalCommand(CMD_REBOOT, 0, false) == true){
		bRst = true;
	}
	return bRst;
}

/**
 * try to destroy running sdk process, then recreate a new sdk process ...
 * critical work around!!
 */
bool CadeSDKAPIleptonthermal::clickedThermalRESTART(){
	bool bRst = false;
	if(OnParserThermalCommand(CMD_RESTART, 0, false) == true){
		bRst = true;
	}
	return bRst;
}

bool CadeSDKAPIleptonthermal::clickedThermalCPUBind(int iCPUIndex){
	bool bRst = false;

	if (iCPUIndex == LEPTON_SDK_CPUBIND_AUTO){
		GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Thermal API] auto cpu bind\n");
		bRst = true;
	}else if(OnParserThermalCommand(CMD_SETCPUBIND, iCPUIndex, false) == true){
		bRst = true;
	}
	return bRst;
}

void CadeSDKAPIleptonthermal::sdk_log_callback(uint8_t log_level, const char* log_string)
{
	if(log_string != NULL && log_string[0] != 0)
	{
		char cLogLevel[8] = { '\0' };
		axc_byte byLogLevel = AXC_LOG_LEVEL_NORMAL;  // 2
		switch(log_level)
		{
		case  1: byLogLevel = AXC_LOG_LEVEL_ERROR;
			strncpy(cLogLevel, "E", sizeof(cLogLevel));
		break;
		case  2: byLogLevel = AXC_LOG_LEVEL_DEBUG;
			strncpy(cLogLevel, "D", sizeof(cLogLevel));
		break;
		case  3: byLogLevel = AXC_LOG_LEVEL_VERBOSE;
			strncpy(cLogLevel, "V", sizeof(cLogLevel));
		break;
		default: byLogLevel = AXC_LOG_LEVEL_NORMAL;
			strncpy(cLogLevel, "N", sizeof(cLogLevel));
		break;
		}

    	//CHeatFinderUtility::PrintNowData(1);
		if (byLogLevel == AXC_LOG_LEVEL_ERROR){
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "%s(%d): [Thermal_SDK] %s", cLogLevel, byLogLevel, log_string);
		}else if (byLogLevel == AXC_LOG_LEVEL_DEBUG){
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "%s(%d): [Thermal_SDK] %s", cLogLevel, byLogLevel, log_string);
		}else if (byLogLevel == AXC_LOG_LEVEL_VERBOSE){
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tVERBOSETrace_MSK, "%s(%d): [Thermal_SDK] %s", cLogLevel, byLogLevel, log_string);
		}else if (byLogLevel == AXC_LOG_LEVEL_NORMAL){
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tNORMALTrace_MSK, "%s(%d): [Thermal_SDK] %s", cLogLevel, byLogLevel, log_string);
		}else{
			printf("%s(%d): [Thermal_SDK] %s", cLogLevel, byLogLevel, log_string);
		}

		/** FIXME
		 * need reboot
		 * "Failed to spi_rx_tx, retry #N"
		 */
	}
}

int CadeSDKAPIleptonthermal::sdk_error_reader(E_ADETHERMAL_RESULT enumErrno, T_ADETHERMAL_CONFIG *pConfig){
	T_ADETHERMAL_CONFIG config;
	memset(&config, 0, sizeof(config));
	config.config_size = sizeof(config);

	/**
	 * Might be called with few information.
	 */
	if ((pConfig == NULL) &&
		((enumErrno == ADETHERMAL_RESULT_FAILED_OPEN_SPI) ||
			(enumErrno == ADETHERMAL_RESULT_FAILED_OPEN_I2C))){
		/// @brief 'T_CONFIG_FILE_OPTIONS' all display setting
		Cheatfinderconfigmanager *pConfigContextManager = (Cheatfinderconfigmanager *)CHeatFinderUtility::GetConfigObj();
		T_CONFIG_FILE_OPTIONS *pConfigContext = ((pConfigContextManager == NULL)?NULL:pConfigContextManager->GetConfigContext());
		if (pConfigContext !=NULL){
			CAxcString::strncpy(config.i2c_device, pConfigContext->szThermalI2cDevice, (axc_dword)sizeof(config.i2c_device));
			CAxcString::strncpy(config.spi_device, pConfigContext->szThermalSpiDevice, (axc_dword)sizeof(config.spi_device));
			CAxcString::strncpy(config.uart_device, pConfigContext->szThermalUartDevice, (axc_dword)sizeof(config.uart_device));  //Hank
		}else{
			//use default of stander product base on Raspberry PI3 OS: '2017-04-10-raspbian-jessie'
			CAxcString::strncpy(config.i2c_device, "", (axc_dword)sizeof(config.i2c_device));
			CAxcString::strncpy(config.spi_device, "/dev/spidev0.0", (axc_dword)sizeof(config.spi_device));
			CAxcString::strncpy(config.uart_device, "/dev/serial0", (axc_dword)sizeof(config.uart_device));  //Hank
		}
		pConfig = &config;
	}

	if(ADETHERMAL_RESULT_SUCCEED != enumErrno)
	{
		//CHeatFinderUtility::PrintNowData(1);
		GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [Thermal API] failed to open device, ade camera sdk error (I/O Error) = %d, \n", enumErrno);
		switch(enumErrno)
		{
		case ADETHERMAL_RESULT_SDK_NOT_READY:
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "SDK not ready, need adethermal_sdk_init(log_level, log_proc=NULL) first! \n");
			break;
		case ADETHERMAL_RESULT_INVALID_PARAMETER:
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "parameter not correct! \n");
			break;
		case ADETHERMAL_RESULT_NO_ENOUGH_MEMORY:
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "memory not enough! \n");
			break;
		case ADETHERMAL_RESULT_NOT_SUPPORT:
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "function not support \n");
			break;
		case ADETHERMAL_RESULT_FAILED_OPEN_SPI:
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "SPI device cannot open:%s ! \n", pConfig->spi_device);
			break;
		case ADETHERMAL_RESULT_FAILED_CONFIG_SPI:
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "SPI can not accept this configuration! \n");
			break;
		case ADETHERMAL_RESULT_FAILED_OPEN_I2C:
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "I2C device cannot open:%s ! \n", pConfig->i2c_device);
			break;
		case ADETHERMAL_RESULT_FAILED_CONFIG_I2C:
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "I2C can not accept this configuration! \n");
			break;
		case ADETHERMAL_RESULT_FAILED_IOCTL:
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "fail to connect with device, this is I/O error! \n");
			break;
		case ADETHERMAL_RESULT_FAILED_CREATE_THREAD:
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "create process thread fail! \n");
			break;
		case ADETHERMAL_RESULT_DEVICE_NOT_OPENED:
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "need open device first, adethermal_device_open(T_ADETHERMAL_CONFIG* config)! \n");
			break;
		default:
			//ADETHERMAL_RESULT_NO_MORE_DATA
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "no any device can be connected, and no any result feed back! \n");
			break;
		}
		
		return 5; //EIO IO error
	}
	return 0;
}

//
// get min/max/avg temperature on thermal-raw-data buffer
//
bool CadeSDKAPIleptonthermal::get_thermal_value_range(short* pwRawData, short* pwMin, short* pwMax, short* pwAvg, POINT* pptMin, POINT* pptMax)
{
	GlobalDef *pGlobal = (GlobalDef *) CHeatFinderUtility::GetGlobalsupport();
	unsigned int l_iWidth = 0;
	unsigned int l_iHeight = 0;
	unsigned int l_iDataSize = 0;
	pGlobal->GetThermalResolution(l_iWidth, l_iHeight, l_iDataSize);

	if (NULL != pwRawData)
	{
		short wMinVal = 0x7FFF, wMaxVal = 0;
		POINT ptMin = {0,0};
		POINT ptMax = {0,0};
		const int iPixels = l_iWidth * l_iHeight;
		int iValidPixels = 0;
		long dwSum = 0;
		int i = 0;
		for (i = 0; i < iPixels; i++)
		{
			if (0xFFFF != (WORD)pwRawData[i])
			{
				iValidPixels ++;
				dwSum += pwRawData[i];
				if (pwRawData[i] < wMinVal)
				{
					wMinVal = pwRawData[i];
					ptMin.x = i%l_iWidth;
					ptMin.y = i/l_iWidth;  //2.1.1.3 fix
				}
				if (pwRawData[i] > wMaxVal)
				{
					wMaxVal = pwRawData[i];
					ptMax.x = i%l_iWidth;
					ptMax.y = i/l_iWidth;  //2.1.1.3 fix
				}
			}
		}
		if (iValidPixels > 0)
		{
			if (pwMin)  *pwMin = wMinVal;
			if (pwMax)  *pwMax = wMaxVal;
			if (pwAvg)  *pwAvg = (short)(dwSum / iValidPixels);
			if (pptMin) *pptMin = ptMin;
			if (pptMax) *pptMax = ptMax;
			return true;
		}
	}
	if (pwMin)  *pwMin = 0;
	if (pwMax)  *pwMax = 0;
	if (pwAvg)  *pwAvg = 0;
	if (pptMin) pptMin->x = pptMin->y = 0;
	if (pptMax) pptMax->x = pptMax->y = 0;
	return false;
}

// need Thermal SDK ^(v2.53)
int CadeSDKAPIleptonthermal::getThermalCurrentParameterValue(const char* ptype_name, unsigned short (*parameter)[12], uint16_t row_num){
	unsigned short ptype_num =0;
	int rst_Readbyte =0;
	if(0 == (CAxcString::strncmp(ptype_name, "overlay", (sizeof(char)*10), 1))){
		ptype_num =1;

		rst_Readbyte = (unsigned int)GetOverlayParameter(ptype_num, parameter, row_num);
	}
	return rst_Readbyte;
}

int32_t CadeSDKAPIleptonthermal::GetOverlayParameter(uint16_t ptype, unsigned short (*parameter)[12], uint16_t row_num){
	// ref: http://cs-fundamentals.com/c-programming/arrays-in-c.php
	int32_t iReadBytes =0;
	uint16_t col_num =12; // this is stable
	uint16_t parameter_size =row_num*col_num*(sizeof(uint16_t));

	if(ptype != 1){
		iReadBytes = 0;
		GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Thermal_SDK] getOverlayParameter: unfit parameter name, you choice wrong function.\n");
	} else {
		uint16_t oy[3][12] = {{0,},};
		if (parameter_size < (sizeof(oy))) {
			iReadBytes = -1;
			//CHeatFinderUtility::PrintNowData(1);
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [Thermal_SDK] getOverlayParameter: get 'overlay' parameter fail: have wrong size %d != %d\n", parameter_size, (sizeof(oy)));
			return iReadBytes;
		} else if (row_num < 3) {
			iReadBytes = -1;
			//CHeatFinderUtility::PrintNowData(1);
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [Thermal_SDK] getOverlayParameter: get 'overlay' parameter fail: row %d <3\n", row_num);
			return iReadBytes;
		}

		iReadBytes = adethermal_device_get_parameters("overlay", oy, sizeof(oy));
		//GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Thermal_SDK] getOverlayParameter: read overlay parameter len %d\n", iReadBytes);
		if (iReadBytes <= 0) {
			// GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Thermal_SDK] getOverlayParameter: error occur, adethermal_device_get_parameters return %d\n", iReadBytes);
		} else if(((uint32_t)iReadBytes) >= sizeof(oy)) {
			for (int i=0; i<3; i++) {
				/*
				printf("D: [Thermal_SDK] test... overlay #%d: (%ux%u) rect (%u,%u,%u,%u) / (%ux%u) rect (%u,%u,%u,%u)\n",
					i+1,
					oy[i][0],oy[i][1],oy[i][2],oy[i][3],oy[i][4],oy[i][5],
					oy[i][6],oy[i][7],oy[i][8],oy[i][9],oy[i][10],oy[i][11]);
					*/
				for (int j=0; j<12; j++) {
					(*parameter)[j] = (unsigned short) oy[i][j];
				}
				parameter +=1;
			}

		}
	}
	return iReadBytes;
}

bool CadeSDKAPIleptonthermal::OnParserThermalCommand(E_THERMAL_COMMAND_MODE CMD_MODE, int iValue, bool bValue)
{
	axc_bool rst = axc_true;

	//char error_string[128];
	char szCommand[64];

	E_ADETHERMAL_RESULT ADETHERMAL_RESULT = ADETHERMAL_RESULT_NO_MORE_DATA;
	if (CMD_MODE >= CMD_T_LAST)
	{
		//CHeatFinderUtility::PrintNowData(1);
		GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [Thermal API] failed to set lepton command, input command number out of range:%d\n", CMD_MODE);
		rst = axc_false;
		return ((rst == axc_true)?true:false);
	}
	else
	{
		if(CMD_MODE == CMD_RAD_ENABLE)
		{
			CAxcString::strncpy(szCommand, "rad_enable", (axc_dword)sizeof(szCommand));

		}else
		if(CMD_MODE == CMD_RAD_DISABLE)
		{
			CAxcString::strncpy(szCommand, "rad_disable", (axc_dword)sizeof(szCommand));

		}else
		if(CMD_MODE == CMD_FFC)
		{
			CAxcString::strncpy(szCommand, "ffc", (axc_dword)sizeof(szCommand));

		}else
		if(CMD_MODE == CMD_REBOOT)
		{
			CAxcString::strncpy(szCommand, "reboot", (axc_dword)sizeof(szCommand));

		}else
		if(CMD_MODE == CMD_EMISSIVITY){
			double l_emissivity = ((float)iValue / 1000.0) * 8192;
			// https://msdn.microsoft.com/zh-tw/library/hh279667.aspx
			unsigned int lc_emissivity = static_cast<unsigned int>(l_emissivity);
			CAxcString::snprintf(szCommand, (axc_dword)sizeof(szCommand), "sceneEmissivity %u", lc_emissivity);
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Thermal API] lepton command '%s'\n", szCommand);

		}else
		if(CMD_MODE == CMD_SETCPUBIND){
				char command[64];
				sprintf(command, "cpubind %d", iValue);
				CAxcString::strncpy(szCommand, command, (axc_dword)sizeof(szCommand));
				GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Thermal API] lepton command '%s'\n", szCommand);
		}else
		if(CMD_MODE == CMD_GETEMI)
		{
			if(adethermal_device_get_parameters("sceneEmissivity",&m_iSceneEmissivity,sizeof(m_iSceneEmissivity)) > 0)
			{
				float l_emissivity = m_iSceneEmissivity / (float)8192;
				m_fEmissivity.fEmissivity = l_emissivity;
				m_fEmissivity.dLastUpdateTime = getCurrentTime();
				GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Thermal API] read sceneEmissivity parameter: %u = %.02f\n", m_iSceneEmissivity, m_fEmissivity.fEmissivity);
			}else{
				GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Thermal API] read sceneEmissivity parameter: Fail!\n");
				m_fEmissivity.fEmissivity = 1.0; //default
			}

			ADETHERMAL_RESULT = ADETHERMAL_RESULT_SUCCEED;
		}else
		if(CMD_MODE == CMD_RESTART){
			ADETHERMAL_RESULT = OnRestartLepton();

		}else{
			ADETHERMAL_RESULT =ADETHERMAL_RESULT_INVALID_PARAMETER;
		}

		//check is setting command or not!
		if ((ADETHERMAL_RESULT !=ADETHERMAL_RESULT_INVALID_PARAMETER) &&
			(CMD_MODE < CMD_SDK_CONTENT))
		{
			// custom function content tool sdk method and other method
			ADETHERMAL_RESULT =adethermal_device_command(szCommand);
		}
	}

	if (ADETHERMAL_RESULT != ADETHERMAL_RESULT_SUCCEED){
		if (CMD_MODE < CMD_SDK_CONTENT){
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Thermal API] deny command '%s'\n", szCommand);
		}
		sdk_error_reader(ADETHERMAL_RESULT, NULL);
		rst = axc_false;
	}else{
		rst = axc_true;
	}

	return ((rst == axc_true)?true:false);
}

E_ADETHERMAL_RESULT CadeSDKAPIleptonthermal::OnRestartLepton(){
	E_ADETHERMAL_RESULT bRst = ADETHERMAL_RESULT_SDK_NOT_READY;
	GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Thermal API] lepton command '%s'\n", "restart thermal sdk");
	m_bPauseReading = true;
	//usleep(20*1000);

	// close sdk
	adethermal_device_close();
	adethermal_sdk_uninit();
	m_bInitReady = false;
	usleep(100*1000);
	// restart sdk
	//GlobalDef *pGlobal = (GlobalDef *) CHeatFinderUtility::GetGlobalsupport();
	/// @brief 'T_CONFIG_FILE_OPTIONS' all display setting
	Cheatfinderconfigmanager *pConfigContextManager = (Cheatfinderconfigmanager *)CHeatFinderUtility::GetConfigObj();
	T_CONFIG_FILE_OPTIONS *pConfigContext = ((pConfigContextManager == NULL)?NULL:pConfigContextManager->GetConfigContext());

	unsigned int l_iLogLevel = 1; // only show error-log
	if (pConfigContext !=NULL){
		l_iLogLevel = pConfigContext->dwLogLevel;
	}
	switch(l_iLogLevel)
	{
	case AXC_LOG_LEVEL_VERBOSE:		m_byLogLevel = 3; break;
	case AXC_LOG_LEVEL_NORMAL: 		m_byLogLevel = 3; break;
	case AXC_LOG_LEVEL_WARN:		m_byLogLevel = 2; break;
	case AXC_LOG_LEVEL_DEBUG:
	case AXC_LOG_LEVEL_ERROR:
	case AXC_LOG_LEVEL_FATAL:		m_byLogLevel = 1; break;
	case AXC_LOG_LEVEL_DISABLE:
	default:						m_byLogLevel = 0; break;
	}
	adethermal_sdk_init(m_byLogLevel, sdk_log_callback, NULL);

	// open device
	// T_ADETHERMAL_CONFIG config;
	// memset(&config, 0, sizeof(config));
	// config.config_size = sizeof(config);
	// if (pConfigContext !=NULL){
	// 	CAxcString::strncpy(config.i2c_device, pConfigContext->szThermalI2cDevice, (axc_dword)sizeof(config.i2c_device));
	// 	CAxcString::strncpy(config.spi_device, pConfigContext->szThermalSpiDevice, (axc_dword)sizeof(config.spi_device));
	// 	CAxcString::strncpy(config.uart_device, pConfigContext->szThermalUartDevice, (axc_dword)sizeof(config.uart_device));  // Hank
	// 	config.spi_speed = pConfigContext->dwThermalSpiSpeed;
	// 	config.spi_rx_mode = (uint8_t) pConfigContext->dwThermalSpiMode;
	// 	config.spi_rx_bits = (uint8_t) pConfigContext->dwThermalSpiBits;
	// 	config.spi_word_swap = (uint8_t) pConfigContext->bThermalSpiSwapWord;
	// 	config.spi_byte_swap = (uint8_t) pConfigContext->bThermalSpiSwapByte;
	// 	config.optional = (uint8_t) pConfigContext->ucThermalOptional;
	// 	GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Thermal API] open device from pConfigContext\n");
	// }else{
	// 	//use default of stander product base on Raspberry PI3 OS: '2017-04-10-raspbian-jessie'
	// 	CAxcString::strncpy(config.i2c_device, "", (axc_dword)sizeof(config.i2c_device));
	// 	CAxcString::strncpy(config.spi_device, "/dev/spidev0.0", (axc_dword)sizeof(config.spi_device));
	// 	CAxcString::strncpy(config.uart_device, "/dev/serial0", (axc_dword)sizeof(config.uart_device));  // Hank
	// 	config.spi_speed = 10000000;
	// 	config.spi_rx_mode = 3;
	// 	config.spi_rx_bits = 8;
	// 	config.spi_word_swap = axc_true;
	// 	config.spi_byte_swap = axc_false;
	// 	config.optional = 0x00;
	// 	GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Thermal API] open device ready\n");
	// }
	// config.thread_priority = 0 /*THERMAL_SPI_RW_THREAD_PRIORITY*/;
	// config.mirror_downup = 0;
	// config.mirror_leftright = 0;

	T_ADETHERMAL_CONFIG config = { sizeof(T_ADETHERMAL_CONFIG),
                                   "",
                                   "/dev/spidev0.0",
                                   "/dev/serial0",
                                   10000000,
                                   SPI_MODE_0,
                                   8,
                                   1,
                                   0,
                                   0,
                                   0,
                                   0,
                                   0
                                 };

	const E_ADETHERMAL_RESULT ADETHERMAL_RESULT = adethermal_device_open(&config);

	if (ADETHERMAL_RESULT == ADETHERMAL_RESULT_SUCCEED){
		const uint32_t dwSdkVer = adethermal_sdk_get_version();
		GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Thermal API] sdk v%u.%u, succeed re-open on '%s' and '%s' and '%s'", dwSdkVer>>16, dwSdkVer&0xFFFF, config.i2c_device, config.spi_device, config.uart_device);

		m_bInitReady = true;
	}else{
		//CHeatFinderUtility::PrintNowData(1);
		GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [Thermal API] sdk reboot by manual fail. thermal error code:%d", ADETHERMAL_RESULT);
	}

	if(m_bInitReady == true){
		if (config.thread_priority > 5){
			m_iCPUBindNumber = LEPTON_SDK_CPUBIND_DEFAULT;
		}else{
			m_iCPUBindNumber = LEPTON_SDK_CPUBIND_AUTO;
		}
	}

	bool bRet = clickedThermalCPUBind(m_iCPUBindNumber);
	if (!bRet){
		GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Thermal API] set cpu bind fail\n");
	}
	bRet = setThermalEmissivity((float)pConfigContext->dwEmissivity/1000.0);
	if (!bRet){
		GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Thermal API] set emissivity fail\n");
	} else {
		m_fEmissivity.fEmissivity = (float)pConfigContext->dwEmissivity/1000.0;
		m_fEmissivity.dLastUpdateTime = getCurrentTime();
	}
	//send notify to alarm thermal restart message
	fireReceivedThermalRestartFinishNotify();

	bRst = ADETHERMAL_RESULT;
	m_bPauseReading = false;
	return bRst;
}

/// thread process
bool CadeSDKAPIleptonthermal::ProcessUpdateThermalIvsOnce(){
	//double dbLastFrameMs = 0;
	short* RawDataAnalyze = NULL;

	short AvgValueList[540];
	long dwAvgListNumber = 0;
	double dbAvgTemperatureOnLongTime = 0.0;

	memset(AvgValueList, 0, sizeof(short)*540);
	dwAvgListNumber = 0;
	dbAvgTemperatureOnLongTime = 0;

	GlobalDef *pGlobal = (GlobalDef *) CHeatFinderUtility::GetGlobalsupport();
	if (pGlobal == NULL){
		GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tNORMALTrace_MSK, "N: [Thermal API] %s global support not ready\n", __func__);
		return false;
	}

	/// @brief 'T_CONFIG_FILE_OPTIONS' all display setting
	Cheatfinderconfigmanager *pConfigContextManager = (Cheatfinderconfigmanager *)CHeatFinderUtility::GetConfigObj();
	
	if (m_iOpenIniNumber % 14 == 0)
	{
		pConfigContextManager->ImportConfigContextFromFile(CONFIGURATION_FILE);
		m_iOpenIniNumber = 0;
	}
	m_iOpenIniNumber++;
	
	T_CONFIG_FILE_OPTIONS *pConfigContext = ((pConfigContextManager == NULL)?NULL:pConfigContextManager->GetConfigContext());

	unsigned int l_iWidth = 0;
	unsigned int l_iHeight = 0;
	unsigned int l_iDataSize = 0;

	bool l_bIsNeedAnalyzeHeatObject = false;

	if (!m_bStopAllThread && (m_caxcfifobufferThermalIvs.IsValid() == axc_true))
	{
		void* pRawData = NULL;
		short* pwRawData = NULL;
		axc_i32 iRawDataBytes = 0;
		if ((m_caxcfifobufferThermalIvs.IsValid() != axc_true)
				|| (pGlobal == NULL)
				|| (pConfigContextManager == NULL)){
			//continue;
			return false;
		}

		// lock buffer
		if (!m_caxcfifobufferThermalIvs.Peek((void**)&pRawData, &iRawDataBytes, NULL, 0))
		{
			// check off-line
			if (pGlobal->GetLeptonIsOnline() &&
			    (getCurrentTime() - m_fLastFrameSecondTime) >= 10) // 10 seconds timeout
			{
				pGlobal->SetLeptonIvsOutRecordReInitinal();
			}
			//continue;
			return false;
		}

		pwRawData = (short*) pRawData;
		if(m_caxcmemThermalIvsRawDataAnalyze.GetBufferSize() < (axc_dword)iRawDataBytes)
		{
			if(!m_caxcmemThermalIvsRawDataAnalyze.Resize((axc_dword)iRawDataBytes, 0))
			{
				//CHeatFinderUtility::PrintNowData(1);
				GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [Thermal_API] failed to alloc. for ivs-analyze buffer, size = %d, error = %s\n", iRawDataBytes, AxcGetLastErrorText());
				return false;
			}
		}
		RawDataAnalyze = (short*)m_caxcmemThermalIvsRawDataAnalyze.GetAddress();
		if(NULL == RawDataAnalyze)
		{
			//CHeatFinderUtility::PrintNowData(1);
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [Thermal_API] Memory Address of Thermal Ivs Raw Data analyze is NULL: %s\n", AxcGetLastErrorText());
			return false;
		}

		// Sync. last thermal resolution
		pGlobal->GetThermalResolution(l_iWidth, l_iHeight, l_iDataSize);

		if(NULL != pwRawData && (axc_dword)iRawDataBytes >= (l_iWidth*l_iHeight*2))
		{
			// 处理遮蔽区域
			if(pConfigContext != NULL)
			{
				for(int y=0; y<THERMAL_MASK_BASE_CY; y++)
				{
					if(0 != pConfigContext->ThermalMask_Bits[y])
					{
						for(int x=0; x<THERMAL_MASK_BASE_CX; x++)
						{
							if(pConfigContext->ThermalMask_Bits[y][x/8] & (1 << (x%8)))
							{
								RECT rcMask = {x,y,x+1,y+1};
								CHeatFinderUtility::CoordinateConvertRect(rcMask, THERMAL_MASK_BASE_CX, THERMAL_MASK_BASE_CY, &rcMask, l_iWidth, l_iHeight, 1);
								for(int y2=rcMask.top; y2<rcMask.bottom; y2++)
								{
									for(int x2=rcMask.left; x2<rcMask.right; x2++)
									{
										pwRawData[y2 * l_iWidth + x2] = 0xFFFF; // clear, xiaogyi ++
									}
								}
							}
						}
					}
				}
			}

			// get max & min & avg temperature
			short wMinVal = 0x7FFF, wMaxVal = 0, wAvgVal = 0;
			if(get_thermal_value_range(pwRawData, &wMinVal, &wMaxVal, &wAvgVal, NULL, NULL))
			{
				pGlobal->SetLeptonTempValue(wMaxVal, wMinVal, wAvgVal);
			}

			// avg-temperature on long time (about 1 minute)
			if(0 != wAvgVal)
			{
				int i=0, iCount = 0;
				DWORD dwSum = 0;
				AvgValueList[dwAvgListNumber%THERMAL_AVG_LIST_SIZE] = wAvgVal;
				dwAvgListNumber++;
				iCount = (dwAvgListNumber > THERMAL_AVG_LIST_SIZE) ? THERMAL_AVG_LIST_SIZE : ((int)dwAvgListNumber);
				for(i=0; i<iCount; i++)
				{
					dwSum += AvgValueList[i];
				}
				dbAvgTemperatureOnLongTime = (dwSum/10.0) / iCount;
				if((0 == (dwAvgListNumber%10))
					&&((pConfigContext->dwLogLevel <= AXC_LOG_LEVEL_VERBOSE) && (pConfigContext->dwLogLevel > AXC_LOG_LEVEL_DISABLE)))
				{
					GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tVERBOSETrace_MSK, "V: [Thermal_API] Thermal: min=%d, max=%d, avg=%d (curr %.1f, avg %.1f)\n", wMinVal, wMaxVal, wAvgVal, wAvgVal/10.0, dbAvgTemperatureOnLongTime);
				}
			}

			// Make sure the action of heat object analysis work or not?
			if (pConfigContext == NULL) {	// pass
				l_bIsNeedAnalyzeHeatObject = false;
			} else if (pConfigContext->bThermalHeatObj_Enable == axc_true) {
				l_bIsNeedAnalyzeHeatObject = true;
			} else if (m_fIntervalOfAnalyzeHeatObject > 0.0) {
				double _fTimeDiff = getCurrentTime() - m_fLastTimeAnalyzeHeatObject;
				if ( _fTimeDiff >= m_fIntervalOfAnalyzeHeatObject){
					m_fLastTimeAnalyzeHeatObject = getCurrentTime();
					l_bIsNeedAnalyzeHeatObject = true;
				} else {
					l_bIsNeedAnalyzeHeatObject = false;
				}
			} else if (pConfigContext->bThermalHeatObj_Enable == axc_false) {
				l_bIsNeedAnalyzeHeatObject = false;
			}

			// 标注所有高温物件
			if (l_bIsNeedAnalyzeHeatObject == true) {
				///FIXME:
				//const double dbThermalHeatObj_BackTempe = CHeatFinderUtility::AdjustTemperatureValue(dbAvgTemperatureOnLongTime, g_cfg.iThermalHeatObj_BackTempeMode, g_cfg.dbThermalHeatObj_BackTempe);
				//const double dbThermalHeatObj_HeatTempe = CHeatFinderUtility::AdjustTemperatureValue(dbAvgTemperatureOnLongTime, g_cfg.iThermalHeatObj_HeatTempeMode, g_cfg.dbThermalHeatObj_HeatTempe);
				const double dbThermalHeatObj_BackTempe = CHeatFinderUtility::AdjustTemperatureValue(dbAvgTemperatureOnLongTime, pConfigContext->iThermalHeatObj_BackTempeMode, pConfigContext->dbThermalHeatObj_BackTempe);
				const double dbThermalHeatObj_HeatTempe = CHeatFinderUtility::AdjustTemperatureValue(dbAvgTemperatureOnLongTime, pConfigContext->iThermalHeatObj_HeatTempeMode, pConfigContext->dbThermalHeatObj_HeatTempe);

				// 背景温度阈值：大于等于这个值的都算前景（从背景中去掉）
				const short wBackThreshold = (short)(dbThermalHeatObj_BackTempe*10);
				WORD wHeatObjNumber = 0;
				// 复制 raw-data
				memcpy(RawDataAnalyze, pwRawData, (l_iWidth*l_iHeight*2));
				// 高温值，大于这个值的都会被标注
				const short wFireThreshold = (dbThermalHeatObj_HeatTempe <= dbThermalHeatObj_BackTempe) ? wBackThreshold : ((short)(dbThermalHeatObj_HeatTempe*10));

				const short wGapValue = wBackThreshold + ((wFireThreshold - wBackThreshold)/2);
				short wMax = 0x7FFF, wMin = 0;
				POINT ptObj = {0,0};
				while(wHeatObjNumber < MAX_THERMAL_HEATOBJ_NUM &&
					  get_thermal_value_range(RawDataAnalyze,&wMin,&wMax,NULL,NULL,&ptObj) &&
					  wMax > wMin &&
					  wMax >= wFireThreshold)
				{
					// 先找到物件的边界
					const int iMinObjSize = 8/2; // 假设最小物件为 8x8
					RECT rcObj = {0,0,0,0};
					int x=0,y=0;
					for(rcObj.left = AXC_MAX(ptObj.x-iMinObjSize, 1);
						rcObj.left > 0 && !IS_BORDER(rcObj.left, ptObj.y);
						rcObj.left --);
					for(rcObj.right = AXC_MIN(ptObj.x+iMinObjSize, (int)l_iWidth-1);
						rcObj.right < (int)l_iWidth && !IS_BORDER(rcObj.right, ptObj.y);
						rcObj.right ++);
					for(rcObj.top = AXC_MAX(ptObj.y-iMinObjSize, 1);
						rcObj.top > 0 && !IS_BORDER(ptObj.x, rcObj.top);
						rcObj.top --);
					for(rcObj.bottom = AXC_MIN(ptObj.y+iMinObjSize, (int)l_iHeight-1);
						rcObj.bottom < (int)l_iHeight && !IS_BORDER(ptObj.x, rcObj.bottom);
						rcObj.bottom ++);
					// 清除该物件，方便继续查找下一个高温点
					for(y=rcObj.top; y<rcObj.bottom; y++)
					{
						for(x=rcObj.left; x<rcObj.right; x++)
						{
							RawDataAnalyze[ y * l_iWidth + x ] = 0xFFFF;
						}
					}
					// 记录
					/*
					l_pThermalIvsOut->HeatObjList[wHeatObjNumber].wTempe = (WORD)(((wMax/10.0)+273.15)*100);
					l_pThermalIvsOut->HeatObjList[wHeatObjNumber].ptPos = ptObj;
					l_pThermalIvsOut->HeatObjList[wHeatObjNumber].rcObj = rcObj;
					*/
					//pGlobal->SetLeptonTempObjInfo(wHeatObjNumber, wMax, ptObj, rcObj);
					onReceivedLeptonTempObjInfo(wHeatObjNumber, wMax, ptObj, rcObj);

					wHeatObjNumber++;
				}
				if((0 == (dwAvgListNumber%10))
					&&((pConfigContext->dwLogLevel <= AXC_LOG_LEVEL_VERBOSE) && (pConfigContext->dwLogLevel > AXC_LOG_LEVEL_DISABLE)))
				{
					GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tVERBOSETrace_MSK, "V: [Thermal_API] heat_object=%u (back %.1fC, heat %.1fC, avg %.1fC)\n", wHeatObjNumber, dbThermalHeatObj_BackTempe, dbThermalHeatObj_HeatTempe, dbAvgTemperatureOnLongTime);
				}
				//pGlobal->SetLeptonHeatObjCount((unsigned short)wHeatObjNumber);
				onReceivedLeptonTempObjCount((unsigned short)wHeatObjNumber);

				// send notify, if anyone need this message.
				fireHeatObjectSendNotify();
			}else{
				//pGlobal->SetLeptonHeatObjCount(0);
				onReceivedLeptonTempObjCount(0);
			}
		}

		// lepton-module is online
		if(pGlobal->GetLeptonIsOnline() == false){
			pGlobal->SetLeptonOnlineState(true);
		}
		m_fLastFrameSecondTime = getCurrentTime();
		m_caxcfifobufferThermalIvs.Pop(axc_false);
		return true;
	}

	GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tNORMALTrace_MSK, "N: [Thermal API] %s End\n", __func__);
	return false;
}

bool CadeSDKAPIleptonthermal::ProcessLeptonReadRawDataOnce(){
	GlobalDef *pGlobal = (GlobalDef *) CHeatFinderUtility::GetGlobalsupport();
	/// @brief 'T_THERMAL_IVS_OUTPUT' is about heat objects detect information, which will be overlay with output view of thermal (lepton).
	//T_THERMAL_IVS_OUTPUT *l_pThermalIvsOut;
	//l_pThermalIvsOut = pGlobal->GetThermalIvsOutSavingTable();
	/// @brief 'T_CONFIG_FILE_OPTIONS' all display setting
	Cheatfinderconfigmanager *pConfigContextManager = (Cheatfinderconfigmanager *)CHeatFinderUtility::GetConfigObj();
	T_CONFIG_FILE_OPTIONS *pConfigContext = ((pConfigContextManager == NULL)?NULL:pConfigContextManager->GetConfigContext());

	//printf("D: [Thermal API] %s open 0\n", __func__);
	//memset(l_pThermalIvsOut, 0, sizeof(T_THERMAL_IVS_OUTPUT));
	//l_pThermalIvsOut->bLeptonIsOnline = FALSE;

	unsigned int l_iWidth =0;
	unsigned int l_iHeight =0;
	unsigned int l_iDataSize =0;

	bool bRst = false;
	if (!m_bStopAllThread &&
		(m_caxcfifobufferThermalIvs.IsValid() == axc_true) &&
		(!m_bPauseReading) &&
		(m_bInitReady))  // wait system re-init.
	{

		// get rawdata-frame
		T_ADETHERMAL_RAWDATA_FRAME raw;
		unsigned int rst = adethermal_device_lockbuffer(&raw);

		//printf("D: [Thermal API] %s open 3\n", __func__);
		if( rst == ADETHERMAL_RESULT_SUCCEED)
		{
			// save image-size information
			// Sync. last thermal resolution
			//pGlobal->GetThermalResolution(l_iWidth, l_iHeight, l_iDataSize);
			if(l_iWidth != raw.width || l_iHeight != raw.height)
			{
				l_iWidth = raw.width;
				l_iHeight = raw.height;
				l_iDataSize = raw.data_size;
				//UpdateBroadcastInfo();
				pGlobal->OnRenewThermalResolution(l_iWidth, l_iHeight, l_iDataSize);
			}

			if (raw.data_size > 0)
			{
				// for checking thermal data of lepton:
				// width, height, temp., reworking for range, area, and etc.
				m_caxcfifobufferThermalIvs.Push(raw.data_ptr, raw.data_size, NULL, 0);

				// send notify (has data) to other process. (need prepare queue for recv.)
				//if((axc_dword)raw.data_size >= (l_iWidth*l_iHeight*2))
				{
					fireDataSendNotify((axc_byte *)raw.data_ptr, raw.data_size, &raw);
				}

				if (m_iCountNoData > 0){
					//printf("D: [Thermal API] read success after lose %u time\n", m_iCountNoData);
					m_iCountNoData = 0;
				}
			}

			// unlock buffer
			adethermal_device_unlockbuffer();

			bRst = true;
		}else if( rst != ADETHERMAL_RESULT_NO_MORE_DATA){
			//printf("D: [Thermal API] %s open 3-2\n", __func__);
			//system("sudo killall -s SIGUSR1 ade_gpio_set 3>&1 2>&1 1>/dev/null");
			GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Thermal API] read fail, return %d \n", rst);
			sdk_error_reader((E_ADETHERMAL_RESULT)rst, NULL);
			//m_threadReadLeptonRawData.SleepMilliSeconds(10);
			//continue;
			bRst = false;
		}else{
			/**
			 * ADETHERMAL_RESULT_NO_MORE_DATA
			 */
			double diff = (getCurrentTime() - m_fLastFrameSecondTime);
			double loseMsCount = m_interval * m_iCountNoData;
			double loseCount = (loseMsCount / (float)1000.0 ); // seconds
			if ((loseCount < THERMAL_FRAME_LOSE_RELOAD_TIME) &&
				( diff < THERMAL_FRAME_LOSE_RELOAD_TIME))
			{
				/* happen:
				 * 1. thermal sdk: oem-reboot fail or video lost.
				 * 2. i2c lock by some where??
				 * 3. lepton thermal connect lost (h.w. I/O)
				 * 4. total lost time count less than 2 min.
				 */
				m_iCountNoData +=1;
				// total wait 110ms ~ 330ms, Thermal module will capture view 7 per-sec.
			}
			else if((pConfigContext->iOpenStream & OPEN_THERMAL_STREAM)==0){
				/** 2.1.1.12 start: update selection of open stream  */
				m_iCountNoData = 0;
				m_fLastFrameSecondTime = getCurrentTime();

				/** 2.1.1.12 end */
			}
			else
			{
				GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Thermal API] lose view too long time! (%.3lf sec. & count %.3lf sec. [%d time])\n", diff, loseCount, m_iCountNoData);

				axc_bool tc_ret = axc_false;
				tc_ret = (ADETHERMAL_RESULT_SUCCEED == OnRestartLepton())? axc_true: axc_false;
				//tc_ret = Thermal_Command(CMD_RESTART);
				GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Thermal API] try to reboot lepton thermal background tools, %s\n", ((tc_ret == axc_true)? "success":"fail"));

				m_iCountNoData = 0;
				m_fLastFrameSecondTime = getCurrentTime();
			}
			bRst = false;
		}
		//CAxcTime::SleepMilliSeconds(20);
	}

	//when error occur ... sleep by interval time
	if (bRst != true){
		usleep(m_interval*1000); // --> sleep ?? millisecond
	}

	return bRst;
}

void CadeSDKAPIleptonthermal::onReceivedLeptonTempObjInfo(unsigned short iIndex, short iTemp, POINT ptObj, RECT rcObj){

	GlobalDef *pGlobal = (GlobalDef *) CHeatFinderUtility::GetGlobalsupport();
	pGlobal->SetLeptonTempObjInfo(iIndex, iTemp, ptObj, rcObj);

	if (m_HeatObjectSendNotifyEventList.size() > 0){
	    m_PlugInHeatObjects.HeatObjList[iIndex].wTempe = (unsigned short)(((iTemp/10.0)+273.15)*100);
	    m_PlugInHeatObjects.HeatObjList[iIndex].posX = ptObj.x;
	    m_PlugInHeatObjects.HeatObjList[iIndex].posY = ptObj.y;
	}
}

void CadeSDKAPIleptonthermal::onReceivedLeptonTempObjCount(unsigned short iCount){

	GlobalDef *pGlobal = (GlobalDef *) CHeatFinderUtility::GetGlobalsupport();
	pGlobal->SetLeptonHeatObjCount(iCount);

	if (m_HeatObjectSendNotifyEventList.size() > 0){
		m_PlugInHeatObjects.wHeatObj = iCount;
	}
}
