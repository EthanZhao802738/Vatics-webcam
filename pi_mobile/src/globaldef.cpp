#include "globaldef.h"
#include <unistd.h>
#include "ade_camera_sdk.h"

#include "heatfinderconfigmanager.h"
#include "adeSDKAPIleptonthermal.h"

#include<stdio.h>
#include<time.h>

GlobalDef::GlobalDef():
	m_queueNetDataForSend("queue/net/send"),
	m_queueNetUDPDataForSend("queue/udpnet/send")
{
    m_sizeVisionMainImage.cx = 0;
    m_sizeVisionMainImage.cy = 0;

    m_sizeVisionSubImage.cx = 0;
    m_sizeVisionSubImage.cy = 0;

    m_sizeThermalImage.cx = 0;
    m_sizeThermalImage.cy = 0;

    m_aptm_Emissivity = 1.0;

    m_bQueueIsValid = false;

    m_iFifoTotalBytes = (4*1024*1024);
    m_iFifoPacketSize = 1600;

	if(!m_queueNetDataForSend.Create(m_iFifoPacketSize, (m_iFifoTotalBytes/m_iFifoPacketSize))){
		//CHeatFinderUtility::PrintNowData(1);
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] create virtual queue fail\n", "CHeatFinderApp");
	}else{
		m_bQueueIsValid = true;
	}

	if(!m_queueNetUDPDataForSend.Create(m_iFifoPacketSize, (m_iFifoTotalBytes/m_iFifoPacketSize))){
		//CHeatFinderUtility::PrintNowData(1);
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] create virtual udp queue fail\n", "CHeatFinderApp");
	}else{
		m_bQueueIsValid = true;
	}

	m_ThermalIvsOut.bLeptonIsOnline = false;
	m_ThermalIvsOut.wHeatObj=0;
	m_ThermalIvsOut.wTempeAvg=0;
	m_ThermalIvsOut.wTempeMax =0;
	m_ThermalIvsOut.wTempeMin =0;
	memset(m_ThermalIvsOut.HeatObjList, 0, sizeof(m_ThermalIvsOut.HeatObjList));

	m_isVisioLostRestart = false;

	m_iTcppollingCount = 0;

	// valgrind say fix: Conditional jump or move depends on uninitialised value(s)
	m_bChkGpioBreathe = 0;

	GLog(tAll, tDEBUGTrace_MSK, "|");
	GLog(tAll, tDEBUGTrace_MSK, "| ========[adehf_mobile program start]========");
	GLog(tAll, tDEBUGTrace_MSK, "|");
	GLog(tAll, tDEBUGTrace_MSK, "W: [CHeatFinderApp] global support create success");
}
GlobalDef::~GlobalDef()
{
	if (m_pthreadRecordList.size() > 0){
		m_pthreadRecordList.erase(m_pthreadRecordList.begin(), m_pthreadRecordList.end());
		m_pthreadRecordList.clear();
	}
	m_queueNetDataForSend.Destroy();
	m_queueNetUDPDataForSend.Destroy();

	GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [CHeatFinderApp] global support stop & erase all record\n");
}

CAxcFifo* GlobalDef::GetDataQueueAddress(){
	if (m_queueNetDataForSend.IsValid() != axc_true){
		return NULL;
	}

	return &m_queueNetDataForSend;
}

bool GlobalDef::ChkIsDataQueueEmpty(){
	if (m_queueNetDataForSend.IsValid() != axc_true){
		return false;
	}

	return (m_queueNetDataForSend.GetCount()<=0)?true:false;
}

CAxcFifo* GlobalDef::GetUDPDataQueueAddress(){
	if (m_queueNetUDPDataForSend.IsValid() != axc_true){
		return NULL;
	}

	return &m_queueNetUDPDataForSend;
}

void GlobalDef::SetTcpPollingCount(unsigned int iValue){
	if (m_iTcppollingCount != iValue)
		m_iTcppollingCount = iValue;
}

bool GlobalDef::ChkIsTcpPollingCountEmpty(){
	return (m_iTcppollingCount>0)?false:true;
}

void GlobalDef::OnCollectThermalMsgToString(CAxcString& cszSend){
	Cheatfinderconfigmanager *pConfigContextManager = (Cheatfinderconfigmanager *)CHeatFinderUtility::GetConfigObj();
	T_CONFIG_FILE_OPTIONS *pConfigContext = ((pConfigContextManager == NULL)?NULL:pConfigContextManager->GetConfigContext());

    cszSend.Append("command=reply\r\n");
    cszSend.Append("reply_src=get_thermal_ivs_result\r\n");

    m_locker.lock();
    m_thermalmsg_locker.lock();
    /**if (false == m_locker.try_lock()){
		cszSend.Append("result_code=1\r\n");
		cszSend.Append("result_text=FAIL\r\n");
    }else**/
    {
		cszSend.Append("result_code=0\r\n");
		cszSend.Append("result_text=OK\r\n");

		const axc_bool bVisionOnline = (m_sizeVisionMainImage.cx != 0 && m_sizeVisionMainImage.cy != 0);
		cszSend.AppendFormat("vision_camera_online=%d\r\n", bVisionOnline);
		cszSend.AppendFormat("vision_camera_width=%d\r\n", m_sizeVisionMainImage.cx);
		cszSend.AppendFormat("vision_camera_height=%d\r\n", m_sizeVisionMainImage.cy);

		//m_locker.lock();

		cszSend.AppendFormat("thermal_camera_online=%d\r\n", (unsigned short)m_ThermalIvsOut.bLeptonIsOnline);
		cszSend.AppendFormat("thermal_camera_width=%d\r\n", m_sizeThermalImage.cx);
		cszSend.AppendFormat("thermal_camera_height=%d\r\n", m_sizeThermalImage.cy);
		cszSend.AppendFormat("min_temperature_k=%u\r\n", m_ThermalIvsOut.wTempeMin);
		cszSend.AppendFormat("max_temperature_k=%u\r\n", m_ThermalIvsOut.wTempeMax);
		cszSend.AppendFormat("avg_temperature_k=%u\r\n", m_ThermalIvsOut.wTempeAvg);
		cszSend.AppendFormat("heat_object_number=%u\r\n", m_ThermalIvsOut.wHeatObj);
		if (pConfigContext->bThermalHeatObj_Enable == axc_true){
			for(axc_word i=0; i<m_ThermalIvsOut.wHeatObj && i < MAX_THERMAL_HEATOBJ_NUM; i++)
			{
				cszSend.AppendFormat("heat_object%d_temperature_k=%u\r\n", i+1, m_ThermalIvsOut.HeatObjList[i].wTempe);
				cszSend.AppendFormat("heat_object%d_pos=%d,%d\r\n", i+1, m_ThermalIvsOut.HeatObjList[i].ptPos.x, m_ThermalIvsOut.HeatObjList[i].ptPos.y);
			}
		}
    }
    m_thermalmsg_locker.unlock();
    m_locker.unlock();

    cszSend.Append("\r\n"); // END
}

bool GlobalDef::OnRenewThermalResolution(const unsigned int Width, const unsigned int Height, const unsigned int DataSize){
	if ((Width == 0) || (Height == 0)){
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [CHeatFinderApp] can't renew global thermal resolution record by (w, h)= {%u, %u}\n", Width, Height);
		return false;
	}
    m_thermalmsg_locker.lock();
    m_sizeThermalImage.cx = Width;
    m_sizeThermalImage.cy = Height;
    m_iThermalDataSize = DataSize;
    m_thermalmsg_locker.unlock();
    return true;
}

bool GlobalDef::OnRenewVisionResolution(const unsigned int Width, const unsigned int Height){
	if ((Width == 0) || (Height == 0)){
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [CHeatFinderApp] can't renew global Vision resolution record by (w, h)= {%u, %u}\n", Width, Height);
		return false;
	}
    m_locker.lock();
    m_sizeVisionMainImage.cx = Width;
    m_sizeVisionMainImage.cy = Height;
    m_locker.unlock();
    return true;
}

bool GlobalDef::OnRenewVisionSubResolution(const unsigned int Width, const unsigned int Height){
	if ((Width == 0) || (Height == 0)){
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [CHeatFinderApp] can't renew global Vision-Sub resolution record by (w, h)= {%u, %u}\n", Width, Height);
		return false;
	}
    m_locker.lock();
    m_sizeVisionSubImage.cx = Width;
    m_sizeVisionSubImage.cy = Height;
    m_locker.unlock();
    return true;
}

void GlobalDef::AddpThreadRecord(const char* pName){
	m_addpThreadRecord_locker.lock();
	int l_iCoordinate =-1;
	if (m_pthreadRecordList.size() > 0){
		//check the same name
		for(unsigned int i=0; i<m_pthreadRecordList.size(); i++ ){
			if (0 ==strcmp(m_pthreadRecordList[i].name, pName)){
				l_iCoordinate = (signed int) i;
			}
		}
	}

	if (l_iCoordinate > -1){
		m_pthreadRecordList[l_iCoordinate].count += 1;
	}else{
		pThreadRecord l_pTRobj;
		memset(&l_pTRobj, 0, sizeof(l_pTRobj));
		strncpy(l_pTRobj.name, pName, sizeof(l_pTRobj.name));
		l_pTRobj.count =1;

		m_pthreadRecordList.push_back(l_pTRobj);
	}

	m_addpThreadRecord_locker.unlock();
}

void GlobalDef::OnPrintfpThreadRecord(){
	CountpThreadRecordNumber(true);
}

unsigned int GlobalDef::GetpThreadRecordNumber(){
	return CountpThreadRecordNumber(false);
}

unsigned int GlobalDef::CountpThreadRecordNumber(bool bisPrint){
	unsigned int l_counter =0;
	bool l_isPrintenable = bisPrint;
	CAxcString l_caxcStr;

	m_addpThreadRecord_locker.lock();
	if (m_pthreadRecordList.size() > 0){
		//check the same name
		for(unsigned int i=0; i<m_pthreadRecordList.size(); i++ ){
			// printf
			if (l_isPrintenable)
				l_caxcStr.AppendFormat("W: [CHeatFinderApp] pThread:%s Count:%u\n", m_pthreadRecordList[i].name, m_pthreadRecordList[i].count);

			// count
			l_counter += (unsigned int)(m_pthreadRecordList[i].count);
		}
	}

	if (l_isPrintenable)
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [CHeatFinderApp] Total registered pThreads:%u\n%s\n", l_counter, l_caxcStr.Get());
	m_addpThreadRecord_locker.unlock();

	return l_counter;
}

bool GlobalDef::PushFrameToNetDataQueue(const void* pData, int iData_length, unsigned short uChannel_index, int iWidth, int iHeight, double dfLastFrameTimeStamp_MS, unsigned int uiFrameOrderofSequence, bool bIsKeyframe){
	if (m_bQueueIsValid == false){
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [CHeatFinderApp] Net Data Queue not Ready\n");
		return false;
	}

	if (iData_length <= 0){
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [CHeatFinderApp] intput data length too short: %d\n", iData_length);
		return false;
	}

	if ((uChannel_index != ADE2CAM_CHANNEL_LEPTONRAW) &&(uChannel_index != ADE2CAM_CHANNEL_VISUALMAIN) && (uChannel_index != ADE2CAM_CHANNEL_VISUALSUB)){
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [CHeatFinderApp] unknown channel require: (0~2: lepton, vision, vision-sub) %u\n", uChannel_index);
		return false;
	}

	if ((iWidth == 0)||(iHeight == 0)){
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [CHeatFinderApp] resolution parameter error (w, h)= {%u, %u}\n", iWidth, iHeight);
		return false;
	}

	//change type
	const axc_ddword ddwPts = (axc_ddword) dfLastFrameTimeStamp_MS;
	const axc_dword dwPts = (axc_dword)(ddwPts & 0xFFFFFFFFULL);

	//axc_true:(1),  axc_false:(0)
	int iIsKeyFrame = ((bIsKeyframe == true)?1:0);

	AxcSetLastError(0);
	axc_bool axcbRst = CHeatFinderUtility::PushFrameToTxFifo(&m_queueNetDataForSend, pData, iData_length, (axc_byte)uChannel_index, iWidth, iHeight, dwPts, (axc_dword)uiFrameOrderofSequence, iIsKeyFrame);
	if (axcbRst == axc_false){
		//CHeatFinderUtility::PrintNowData(1);
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [CHeatFinderApp] fail to add data to Net Data Queue\n");
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [CHeatFinderApp] Detail: %s\n", AxcGetLastErrorText());
	}

	AxcSetLastError(0);
	axc_bool axcbRst2 = CHeatFinderUtility::PushFrameToTxFifo(&m_queueNetUDPDataForSend, pData, iData_length, (axc_byte)uChannel_index, iWidth, iHeight, dwPts, (axc_dword)uiFrameOrderofSequence, iIsKeyFrame);
	if (axcbRst2 == axc_false){
		//CHeatFinderUtility::PrintNowData(1);
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [CHeatFinderApp] fail to add data to Net UDP Data Queue\n");
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [CHeatFinderApp] Detail: %s\n", AxcGetLastErrorText());
		axcbRst = axc_false;
	}



	return ((axcbRst == axc_true)?true:false);
}

CAxcFifo* GlobalDef::GetNetDataQueue(){
	return &m_queueNetDataForSend;
}

CAxcFifo* GlobalDef::GetNetUDPDataQueue(){
	return &m_queueNetUDPDataForSend;
}

bool GlobalDef::SetEmissivity(float fValue){
	bool bRst = false;
	if ((fValue >= 0.00) &&
			(fValue <= 1.00)){
		m_aptm_Emissivity = fValue;
		bRst = true;
	}else{
		//CHeatFinderUtility::PrintNowData(1);
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [CHeatFinderApp] renew emissivity value fail! (0.0 ~ 1.0) %f\n", fValue);
		bRst = false;
	}
	return bRst;
}

float GlobalDef::GetEmissivity(){
	return m_aptm_Emissivity;
}

T_THERMAL_IVS_OUTPUT* GlobalDef::GetThermalIvsOutSavingTable(){
	//if()
	return &m_ThermalIvsOut;
}

bool GlobalDef::GetLeptonIsOnline(){

	return m_ThermalIvsOut.bLeptonIsOnline;
}

void GlobalDef::SetLeptonOnlineState(bool bValue){
	m_ThermalIvsOut.bLeptonIsOnline = bValue;
}

void GlobalDef::SetLeptonIvsOutRecordReInitinal(){
	m_ThermalIvsOut.bLeptonIsOnline = false;
	m_ThermalIvsOut.wHeatObj=0;
	m_ThermalIvsOut.wTempeAvg=0;
	m_ThermalIvsOut.wTempeMax =0;
	m_ThermalIvsOut.wTempeMin =0;
	memset(m_ThermalIvsOut.HeatObjList, 0, sizeof(m_ThermalIvsOut.HeatObjList));
}

void GlobalDef::SetLeptonTempValue(short iMax, short iMin, short iAvg){
	m_ThermalIvsOut.wTempeMax = (WORD)(((iMax/10.0)+273.15)*100);
	m_ThermalIvsOut.wTempeMin = (WORD)(((iMin/10.0)+273.15)*100);
	m_ThermalIvsOut.wTempeAvg = (WORD)(((iAvg/10.0)+273.15)*100);
}

void GlobalDef::SetLeptonTempObjInfo(unsigned short iIndex, short iTemp, POINT ptObj, RECT rcObj){
	if (iIndex >= MAX_THERMAL_HEATOBJ_NUM){
		//CHeatFinderUtility::PrintNowData(1);
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [CHeatFinderApp] %s: New Heat Object is our of index: %d > %d\n", __func__, iIndex, MAX_THERMAL_HEATOBJ_NUM);
		return;
	}

	m_ThermalIvsOut.HeatObjList[iIndex].wTempe = (WORD)(((iTemp/10.0)+273.15)*100);
	m_ThermalIvsOut.HeatObjList[iIndex].ptPos.x = ptObj.x;
	m_ThermalIvsOut.HeatObjList[iIndex].ptPos.y = ptObj.y;
	m_ThermalIvsOut.HeatObjList[iIndex].rcObj.bottom = rcObj.bottom;
	m_ThermalIvsOut.HeatObjList[iIndex].rcObj.top = rcObj.top;
	m_ThermalIvsOut.HeatObjList[iIndex].rcObj.right = rcObj.right;
	m_ThermalIvsOut.HeatObjList[iIndex].rcObj.left = rcObj.left;
}

void GlobalDef::SetLeptonHeatObjCount(unsigned short iCount) {
	m_ThermalIvsOut.wHeatObj = (WORD)iCount;
}

unsigned short GlobalDef::GetLeptonHeatObjCount() {
	return m_ThermalIvsOut.wHeatObj;
}

/**
 * return:
 * -1: configuration not exist, 0: read fail, 1: read/update record success
 */
int	GlobalDef::RenewThermalCurrentConfiguration() {
    if (access(CONFIGURATION_FILE, F_OK) != -1) {
        // file exists
    	bool l_bRst = OnReadADEHeatFinderConfigurationFile();
    	return (l_bRst == true)?1:0;
    } else {
        // file doesn't exist
    	return -1;
    }
}

/**
 * return:
 * -1: configuration not exist, 0: close/not collect heat object, 1: turn on/collecting heat objects
 */
int GlobalDef::GetThermalCurrentHeatObjSeekStatus() {
    if (access(CONFIGURATION_FILE, F_OK) != -1) {
        // file exists
    	//bool l_bRst = OnReadADEHeatFinderConfigurationFile();
    	//return ((l_bRst == true)?((m_bThermalHeatObj_Enable == true)?1:0):-1);
    	return (m_bThermalHeatObj_Enable == true)?1:0;
    } else {
        // file doesn't exist
    	return -1;
    }
}

bool GlobalDef::OnReadADEHeatFinderConfigurationFile() {
	//
	// open file
	//
	CAxcFile file("config_file/read");
	if(!file.Open(CONFIGURATION_FILE,"rb"))
	{
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] failed to open file, error: %s\n", "Read Config", AxcGetLastErrorText());
		return false;
	}
	const axc_i64 iFileSize = file.FileSize();
	if(iFileSize <= 0 || iFileSize > (10*1024*1024))
	{
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] failed to read: file size error: %lld\n", "Read Config", iFileSize);
		return false;
	}
	const axc_dword dwFileSize = (axc_dword) iFileSize;

	//
	// read file to buffer
	//
	CAxcMemory buffer("config_file/read_buffer");
	if(!buffer.Create(dwFileSize+2))
	{
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] failed to create buffer, size = %u, error: %s\n", "Read Config", dwFileSize, AxcGetLastErrorText());
		return false;
	}
	char* pszReadBuffer = (char*) buffer.GetAddress();
	const axc_dword dwReadSize = file.Read(pszReadBuffer,dwFileSize);
	if(dwReadSize == 0)
	{
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] failed to read file to buffer, read size %u, error: %s\n", "Read Config", dwReadSize, AxcGetLastErrorText());
		return false;
	}
	// close file
	file.Close();
	// ensure '\0' on string tail
	pszReadBuffer[dwReadSize] = '\0';

	//
	// get some config-options from string buffer
	//
	char szValue[4096] = {""};
	memset(szValue, 0, sizeof(szValue));
	if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(pszReadBuffer, dwReadSize, "thermal_heatobj_enable", szValue, (axc_dword)sizeof(szValue)) > 0)
	{
		int l_iRst = (axc_bool)atoi(szValue);
		m_bThermalHeatObj_Enable = ((l_iRst == axc_true)?true:false);
	}

	/**
	 * get all value from Cheatfinderconfigmanager::object
	 * FIXME:
	 */
	/**
	if (CHeatFinderUtility::GetConfigContext() != NULL){
		Cheatfinderconfigmanager *pConfigContextManager = (Cheatfinderconfigmanager *)CHeatFinderUtility::GetConfigContext();
		T_CONFIG_FILE_OPTIONS *pConfigContext = pConfigContextManager->GetConfigContext();

		//printf("D: [%s] get Thermal configuration manager return value: bThermalHeatObj_Enable(%d)\n", "Read Config", pConfigContext->bThermalHeatObj_Enable);
		m_bThermalHeatObj_Enable = ((pConfigContext->bThermalHeatObj_Enable == axc_true)?true:false);
	}**/

	return true;
}

int	GlobalDef::GetThermalCurrentFoundHeatObj(){
	return (int)m_ThermalIvsOut.wHeatObj;
}

void GlobalDef::HealGPIOBody(){
	if (m_bChkGpioBreathe < (WATCHDOG_GPIO_MAX_LOSE_COUNT)){

		if (m_bChkGpioBreathe < (WATCHDOG_GPIO_MAX_LOSE_COUNT-5)){
			m_bChkGpioBreathe +=5; //shell be alive a least 5 sec.
		}else{
			m_bChkGpioBreathe = WATCHDOG_GPIO_MAX_LOSE_COUNT;
		}
	}
	//printf("N: [%s] GPIO Breathe:%d\n", __func__, m_bChkGpioBreathe);
}

void GlobalDef::AttackGPIOBody(){
	if (m_bChkGpioBreathe >= -1*(WATCHDOG_GPIO_MAX_LOSE_COUNT)){
		m_bChkGpioBreathe -=1;
	}
	//printf("N: [%s] GPIO Breathe:%d\n", __func__, m_bChkGpioBreathe);
}

bool GlobalDef::ChkIsGPIOSrvAlive(){
	if (m_bChkGpioBreathe <= -1*(WATCHDOG_GPIO_MAX_LOSE_COUNT)){
		//already dead
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tNORMALTrace_MSK, "N: [%s] GPIO Breathe:%d\n", __func__, m_bChkGpioBreathe);
		return false;
	}else{
		//has enough blood or rescue time to survive
		return true;
	}
}
void GlobalDef::HealGPIO2Breathe(){
	if (m_bChkGpioBreathe < (WATCHDOG_GPIO_MAX_LOSE_COUNT)){
		//Arise，my champion！
		m_bChkGpioBreathe = WATCHDOG_GPIO_MAX_LOSE_COUNT;
	}
}

void GlobalDef::SetVisioLostRestartStatus(bool bIsRetartNow){
	return OnChangeFlagVisioLostAfterChange(bIsRetartNow);
}
bool GlobalDef::GetVisioLostRestartStatus(){
	return m_isVisioLostRestart;
}
void GlobalDef::OnChangeFlagVisioLostAfterChange(bool bIsRetartNow){
	if (bIsRetartNow){
		m_isVisioLostRestart = (m_isVisioLostRestart == false)? true:m_isVisioLostRestart;
	}else{
		m_isVisioLostRestart = (m_isVisioLostRestart == true)? false:m_isVisioLostRestart;
	}

}

bool GlobalDef::GetVisioResolution(unsigned int &w, unsigned int &h){
	w = m_sizeVisionMainImage.cx;
	h = m_sizeVisionMainImage.cy;
	return true;
}

bool GlobalDef::GetThermalResolution(unsigned int &w, unsigned int &h, unsigned int &size){
	if ((m_sizeThermalImage.cx == 0) || (m_sizeThermalImage.cy == 0)){
		return false;
	}

	w = m_sizeThermalImage.cx;
	h = m_sizeThermalImage.cy;
	size = m_iThermalDataSize;
	return true;
}

int GlobalDef::GetOverlaybyResolution(unsigned int Width, unsigned int Height, xLeptonOverlay &xLol){
	unsigned int l_SyncsizeVisionMainImageX =0;
	unsigned int l_SyncsizeVisionMainImageY =0;
	if ((Width ==0) || (Height ==0)){
		//CHeatFinderUtility::PrintNowData(1);
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [Camera Overlay] %s resolution input value error (w %u, h %u) \n", __func__, Width, Height);
		return -1;
	}
	l_SyncsizeVisionMainImageX = Width;
	l_SyncsizeVisionMainImageY = Height;

	//ade_camera_sdk *pADECamSDK = (ade_camera_sdk *) CHeatFinderUtility::GetADECameraSDK();
	CadeSDKAPIleptonthermal *pLepSDK = (CadeSDKAPIleptonthermal *)CHeatFinderUtility::GetLeptonSDK();
	//overlay
	unsigned short overlay_p[3][THERMAL_OVERLAY_COL] = {{0,},};
	int rst = 0;
	//rst = pADECamSDK->thermal_get_overlay_parameter("overlay", overlay_p, 3);
	rst = pLepSDK->getThermalCurrentParameterValue("overlay", overlay_p, 3);
	if (rst >0){
		//xLeptonOverlay xLol;
		if (l_SyncsizeVisionMainImageX == 1920){
			xLol.CameraW = overlay_p[0][0];
			xLol.CameraH = overlay_p[0][1];
			xLol.CameraL = overlay_p[0][3];
			xLol.CameraT = overlay_p[0][2];
			xLol.CameraR = overlay_p[0][4];
			xLol.CameraB = overlay_p[0][5];

			xLol.LeptonW = overlay_p[0][6];
			xLol.LeptonH = overlay_p[0][7];
			xLol.LeptonL = overlay_p[0][9];
			xLol.LeptonT = overlay_p[0][8];
			xLol.LeptonR = overlay_p[0][10];
			xLol.LeptonB = overlay_p[0][11];
		}else if (l_SyncsizeVisionMainImageX == 1280){
			xLol.CameraW = overlay_p[1][0];
			xLol.CameraH = overlay_p[1][1];
			xLol.CameraL = overlay_p[1][3];
			xLol.CameraT = overlay_p[1][2];
			xLol.CameraR = overlay_p[1][4];
			xLol.CameraB = overlay_p[1][5];

			xLol.LeptonW = overlay_p[1][6];
			xLol.LeptonH = overlay_p[1][7];
			xLol.LeptonL = overlay_p[1][9];
			xLol.LeptonT = overlay_p[1][8];
			xLol.LeptonR = overlay_p[1][10];
			xLol.LeptonB = overlay_p[1][11];
		}else if (l_SyncsizeVisionMainImageX == 640){
			xLol.CameraW = overlay_p[2][0];
			xLol.CameraH = overlay_p[2][1];
			xLol.CameraL = overlay_p[2][3];
			xLol.CameraT = overlay_p[2][2];
			xLol.CameraR = overlay_p[2][4];
			xLol.CameraB = overlay_p[2][5];

			xLol.LeptonW = overlay_p[2][6];
			xLol.LeptonH = overlay_p[2][7];
			xLol.LeptonL = overlay_p[2][9];
			xLol.LeptonT = overlay_p[2][8];
			xLol.LeptonR = overlay_p[2][10];
			xLol.LeptonB = overlay_p[2][11];
		}else{
			//CHeatFinderUtility::PrintNowData(1);
			GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [Camera Overlay] %s resolution input value not support (w %u, h %u) \n", __func__, l_SyncsizeVisionMainImageX, l_SyncsizeVisionMainImageY);
		}

	}else{
		//CHeatFinderUtility::PrintNowData(1);
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [Camera Overlay] %s get overlay fail... \n", __func__);
	}
	return rst;
}


////////////////////////////////////
//===================protect=====================
void *CHeatFinderUtility::m_pActiveApp = NULL;
void *CHeatFinderUtility::m_pADECameraSDK = NULL;

/**
 * mark.hsieh ++
 */
void *CHeatFinderUtility::m_pSystemSignalListener = NULL;
void *CHeatFinderUtility::m_pGlobalsupport = NULL;
void *CHeatFinderUtility::m_pTimerSerivce = NULL;
void *CHeatFinderUtility::m_pConfigContext = NULL;
void *CHeatFinderUtility::m_pLeptonSDK = NULL;

//=====================public=====================
void CHeatFinderUtility::PrintNowData(short iStdout){
    time_t unix_timestamp=1429641418;
    unix_timestamp = time(NULL);

    struct tm *tmdate=localtime(&unix_timestamp);

    //printf("unix_timestamp =%d\n", unix_timestamp);
    //printf("Local Time is :asctime=%s", asctime(tmdate));
    //printf("GMT Time is   :gmtime=%s", asctime(gmtime(&unix_timestamp)));

    char weekday[][8]={"Sun.","Mon.","Tue.","Wed.","Thu.","Fri.","Sat."};
    if (iStdout ==1){
    	fprintf(stdout, "Now: %04d-%02d-%02d %02d:%02d:%02d %s\n",tmdate->tm_year+1900,tmdate->tm_mon+1,tmdate->tm_mday,tmdate->tm_hour,tmdate->tm_min,tmdate->tm_sec,weekday[tmdate->tm_wday]);
    	fflush(stdout);
    }else{
    	fprintf(stderr, "Now: %04d-%02d-%02d %02d:%02d:%02d %s\n",tmdate->tm_year+1900,tmdate->tm_mon+1,tmdate->tm_mday,tmdate->tm_hour,tmdate->tm_min,tmdate->tm_sec,weekday[tmdate->tm_wday]);
    }
}

axc_bool CHeatFinderUtility::string_replace(const char* szSrc, char* szDest, size_t nDestBufferSize, const char* szExisted, const char* szNew)
{
    axc_bool bResult = axc_false;
    if(NULL != szSrc && NULL != szDest && NULL != szExisted && 0 != szExisted[0] && NULL != szNew)
    {
        const size_t nSrcLen = strlen(szSrc);
        if(nSrcLen == 0)
        {
            szDest[0] = 0;
            bResult = axc_true;
        }
        else
        {
            const size_t nExistedLen = strlen(szExisted);
            const size_t nNewLen = strlen(szNew);
            const char* pszSrcBegin = szSrc;
            const char* pszFind = NULL;
            char* pszDestEnd = szDest + nDestBufferSize;
            char* pszDest = szDest;
            //
            *pszDest = 0;
            while(NULL != (pszFind = strstr(pszSrcBegin,szExisted)))
            {
                const size_t nCopyLen = (pszFind - pszSrcBegin);
                if((pszDest + nCopyLen + nNewLen) >= pszDestEnd)
                {
                    return axc_false;
                }
                if(nCopyLen > 0)
                {
                    memcpy(pszDest, pszSrcBegin, nCopyLen);
                    pszDest += nCopyLen;
                    *pszDest = 0;
                }
                if(nNewLen > 0)
                {
                    memcpy(pszDest, szNew, nNewLen);
                    pszDest += nNewLen;
                    *pszDest = 0;
                }
                pszSrcBegin = pszFind + nExistedLen;
            }
            if(NULL != pszSrcBegin)
            {
                const size_t nCopyLen = (szSrc + nSrcLen - pszSrcBegin);
                if((pszDest + nCopyLen) >= pszDestEnd)
                {
                    return axc_false;
                }
                memcpy(pszDest, pszSrcBegin, nCopyLen);
                pszDest += nCopyLen;
                *pszDest = 0;
            }
            bResult = axc_true;
        }
    }
    return bResult;
}

void CHeatFinderUtility::MsgSendtoStrerror(int Strerr_id)
{
	//CHeatFinderUtility::PrintNowData(1);
    int SendtoErrno = Strerr_id;
    switch (SendtoErrno)
    {
    //EAGAIN:
    case EWOULDBLOCK:
    	GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] (%d: EAGAIN) When no input is immediately available, read waits for some input. Or, reading a large amount of data from a character special file.\n", __func__, SendtoErrno);
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] (%d: EWOULDBLOCK) The socket's file descriptor is marked O_NONBLOCK and the requested operation would block.\n", __func__, SendtoErrno);
        break;
    case EBADF:
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] (%d) The socket argument is not a valid file descriptor.\n", __func__, SendtoErrno);
        break;
    case ECONNRESET:
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] (%d) A connection was forcibly closed by a peer.\n", __func__, SendtoErrno);
        break;
    case EDESTADDRREQ:
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] (%d) The socket is not connection-mode and no peer address is set.\n", __func__, SendtoErrno);
        break;
    case EINTR:
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] (%d) A signal occurred before any data was transmitted.\n", __func__, SendtoErrno);
        break;
    case EMSGSIZE:
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] (%d) The message is too large to be sent all at once, as the socket requires.\n", __func__, SendtoErrno);
        //The socket type requires that message be sent atomically, and the size of the message to be sent made this impossible.
        break;
    case ENOTCONN:
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] (%d) The socket is not connected or otherwise has not had the peer pre-specified.\n\t (no target has been given)\n", __func__, SendtoErrno);
        //The socket is not connected, and no target has been given.
        break;
    case ENOTSOCK:
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] (%d) The socket argument does not refer to a socket. (The argument sockfd is not a socket.)\n", __func__, SendtoErrno);
        break;
    case EOPNOTSUPP:
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] (%d) The socket argument is associated with a socket that does not support one\n\t or more of the values set in flags.\n\t (Some bit in the flags argument is inappropriate for the socket type.)\n", __func__, SendtoErrno);
        break;
    case EPIPE:
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] (%d) The socket is shut down for writing, or the socket is connection-mode and is no longer connected.\n\t In the latter case, and if the socket is of type SOCK_STREAM,\n\t the SIGPIPE signal is generated to the calling thread.\n\t (The local end has been shut down on a connection oriented socket.\n\t In this case the process will also receive a SIGPIPE unless MSG_NOSIGNAL is set.)\n", __func__, SendtoErrno);
        break;
    case EFAULT:
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] (%d) An invalid user space address was specified for an argument.\n", __func__, SendtoErrno);
        break;
    case EINVAL:
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] (%d) Invalid argument passed.\n", __func__, SendtoErrno);
        break;
    case EISCONN:
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] (%d) The connection-mode socket was connected already but a recipient was specified.\n\t (Now either this error is returned, or the recipient specification is ignored.\n", __func__, SendtoErrno);
        break;
    case ENOMEM:
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] (%d) No memory available.\n", __func__, SendtoErrno);
        break;

    // below is happened on lower change
    case EACCES:
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] (%d) The calling process does not have the appropriate privileges.\n\t Or,  Write permission is denied on the destination socket file,\n\t or search permission is denied for one of the directories the path prefix.\n", __func__, SendtoErrno);
        break;
    case EIO:
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] (%d) An I/O error occurred while reading from or writing to the file system.\n", __func__, SendtoErrno);
        break;
    case ENETDOWN:
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] (%d) The local network interface used to reach the destination is down.\n", __func__, SendtoErrno);
        break;
    case ENETUNREACH:
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] (%d) No route to the network is present.\n", __func__, SendtoErrno);
        break;
    case ENOBUFS:
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] (%d) The output queue for a network interface was full.\n\t This generally indicates that the interface has stopped sending,\n\t but may be caused by transient congestion. (Normally, this does not occur in Linux.\n\t Packets are just silently dropped when a device queue overflows.)\n", __func__, SendtoErrno);
        break;

    default:
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] (%d) unknow error ???? \n", __func__, SendtoErrno);
        break;
    }
}

axc_i32 CHeatFinderUtility::CvmsHelper_Cmd_GetValue(const char* szCmdText, const axc_dword dwCmdTextLen, const char* szCmdKey, char* szValueOut, axc_dword dwValueOutBufferSize)
{
    if (NULL == szCmdText || 0 == szCmdText[0] || dwCmdTextLen < 5 || NULL == szCmdKey || 0 == szCmdKey[0] || NULL == szValueOut || dwValueOutBufferSize < 2)
    {
        return -1;
    }
    const axc_dword dwKeyNameLen = (axc_dword)strlen(szCmdKey);
    const char* pszFind = szCmdText;
    const char* pszCmdTextEnd = szCmdText + dwCmdTextLen;
    while ((pszFind = strstr(pszFind, szCmdKey)) != NULL)
    {
        if ((pszFind == szCmdText || *(pszFind - 1) == '\n') &&
            (pszFind + dwKeyNameLen) < pszCmdTextEnd &&
            *(pszFind + dwKeyNameLen) == '=')
        {
            // 找到了key，开始找value的结尾
            const char* pszValue = pszFind + dwKeyNameLen + 1;
            const char* pszValueEnd = strstr(pszValue, "\r\n");
            if (NULL == pszValueEnd)
            {
                pszValueEnd = pszCmdTextEnd; // 允许末尾没有 \r\n 结束符
                //return -1;
            }
            // 复制value
            const axc_dword dwValueLen = (axc_dword)(pszValueEnd - pszValue);
            const axc_dword dwCopyBytes = AXC_MIN(dwValueLen, (dwValueOutBufferSize - 1));
            memcpy(szValueOut, pszValue, dwCopyBytes);
            *(szValueOut + dwCopyBytes) = 0; // 结束符
            // 返回pszValue的长度，而不是 szValueOut 的长度
            return (axc_i32)dwValueLen;
        }
        if ((pszFind = strstr(pszFind, "\r\n")) == NULL)
        {
            return -1;
        }
        pszFind += 2;
    }
    return -1;
}

// 在网络发来的命令包中找到"文本"的结尾，把命令包拆分成 "文本" 和 "附加数据" 两段
axc_bool CHeatFinderUtility::CvmsHelper_Cmd_UnpackToTextAndExt(void* pInData, const int iInSize, const int iInTextSize, char** ppszText, int* piTextLen, void** ppExtData, int* piExtDataLen)
{
	static char TEXT_END_TAG[5] = { "\r\n\r\n" };
	static int TEXT_END_TAG_LEN = sizeof(TEXT_END_TAG);
	if (NULL != pInData && iInSize >= TEXT_END_TAG_LEN)
	{
		char* pszText = (char*)pInData;
		if (iInTextSize >= 5 && iInTextSize <= iInSize && 0 == memcmp(pszText + iInTextSize - TEXT_END_TAG_LEN, TEXT_END_TAG, TEXT_END_TAG_LEN))
		{
			const int iExtLen = iInSize - iInTextSize;
			if (ppszText)		*ppszText = pszText;
			if (piTextLen)		*piTextLen = iInTextSize;
			if (ppExtData)		*ppExtData = (iExtLen <= 0) ? NULL : (pszText + iInTextSize);
			if (piExtDataLen)	*piExtDataLen = iExtLen;
			return axc_true;
		}
		else
		{
			for (int i = 0; i <= (iInSize - TEXT_END_TAG_LEN); i++, pszText++)
			{
				if (0 == memcmp(pszText, TEXT_END_TAG, TEXT_END_TAG_LEN))
				{
					const int iTextLen = i + TEXT_END_TAG_LEN;
					const int iExtLen = iInSize - iTextLen;
					if (ppszText)		*ppszText = (char*)pInData;
					if (piTextLen)		*piTextLen = iTextLen;
					if (ppExtData)		*ppExtData = (iExtLen <= 0) ? NULL : (pszText + TEXT_END_TAG_LEN);
					if (piExtDataLen)	*piExtDataLen = iExtLen;
					return axc_true;
				}
			}
		}
	}
	return axc_false;
}

// 设置文本命令中某一个key的值
// 如果key已经存在，更新它的取值为szNewValue；如果key不存在，把它添加在文本命令串的末尾
// 失败返回-1；buffer大小不够，返回-2；成功返回key的偏移位置
axc_i32 CHeatFinderUtility::CvmsHelper_Cmd_SetValue(char* szCmdText, axc_dword dwCmdTextLen, const axc_dword dwCmdTextBufferSize, const char* szCmdKey, const char* szNewValue)
{
	if (NULL == szCmdText || 0 == szCmdText[0] || dwCmdTextLen < 5 || NULL == szCmdKey || 0 == szCmdKey[0] || dwCmdTextBufferSize <= dwCmdTextLen || NULL == szNewValue)
	{
		return -1;
	}
	// 必须以 \r\n\0 结尾
	if (szCmdText[dwCmdTextLen] != '\0')
	{
		szCmdText[dwCmdTextLen] = '\0';
	}
	if (szCmdText[dwCmdTextLen - 1] != '\n' || szCmdText[dwCmdTextLen - 2] != '\r')
	{
		if ((dwCmdTextLen + 2 + 1) > dwCmdTextBufferSize)
		{
			return -2;
		}
		szCmdText[dwCmdTextLen] = '\r'; dwCmdTextLen++;
		szCmdText[dwCmdTextLen] = '\n'; dwCmdTextLen++;
		szCmdText[dwCmdTextLen] = '\0';
	}
	const axc_dword dwKeyNameLen = (axc_dword)strlen(szCmdKey);
	const axc_dword dwNewValueLen = (axc_dword)strlen(szNewValue);
	char* pszFind = szCmdText;
	const char* pszCmdTextEnd = szCmdText + dwCmdTextLen;
	const char* pszCmdBufferEnd = szCmdText + dwCmdTextBufferSize;
	while ((pszFind = strstr(pszFind, szCmdKey)) != NULL)
	{
		if ((pszFind == szCmdText || *(pszFind - 1) == '\n') &&
			(pszFind + dwKeyNameLen) < pszCmdTextEnd &&
			*(pszFind + dwKeyNameLen) == '=')
		{
			// 找到了key，开始找value的结尾
			char* pszValue = pszFind + dwKeyNameLen + 1;
			const char* pszValueEnd = strstr(pszValue, "\r\n");
			if (NULL == pszValueEnd)
			{
				return -1;
			}
			const char* pszRemainText = pszValueEnd + 2;
			const axc_dword dwRemainTextLen = dwCmdTextLen - (axc_dword)(pszRemainText - szCmdText);
			const axc_dword dwOldValueLen = (axc_dword)(pszValueEnd - pszValue);
			// 更新value
			if (dwNewValueLen == dwOldValueLen)
			{
				if (dwNewValueLen != 0)
				{
					memcpy(pszValue, szNewValue, dwNewValueLen);
				}
			}
			else if (dwNewValueLen < dwOldValueLen)
			{
				memcpy(pszValue, szNewValue, dwNewValueLen);
				memcpy(pszValue + dwNewValueLen, "\r\n", 2);
				CAxcMemory::SafeCopy(pszValue + dwNewValueLen + 2, pszRemainText, dwRemainTextLen+1);
			}
			else // if (dwNewValueLen > dwOldValueLen)
			{
				if ((pszValue + dwNewValueLen + 2 + dwRemainTextLen + 1) >= pszCmdBufferEnd)
				{
					return -2;
				}
				CAxcMemory::SafeCopy(pszValue + dwNewValueLen + 2, pszRemainText, dwRemainTextLen+1);
				memcpy(pszValue, szNewValue, dwNewValueLen);
				memcpy(pszValue + dwNewValueLen, "\r\n", 2);
			}
			// 返回key的偏移位置
			return (axc_i32)(pszFind - szCmdText);
		}
		if ((pszFind = strstr(pszFind, "\r\n")) == NULL)
		{
			return -1;
		}
		pszFind += 2;
	}
	// 没有找到key，添加新值到字符串末尾
	if ((dwCmdTextLen + dwKeyNameLen + 1 + dwNewValueLen + 3) > dwCmdTextBufferSize)
	{
		return -2;
	}
	memcpy(szCmdText + dwCmdTextLen, szCmdKey, dwKeyNameLen);
	memcpy(szCmdText + dwCmdTextLen + dwKeyNameLen, "=", 1);
	memcpy(szCmdText + dwCmdTextLen + dwKeyNameLen + 1, szNewValue, dwNewValueLen);
	memcpy(szCmdText + dwCmdTextLen + dwKeyNameLen + 1 + dwNewValueLen, "\r\n\0", 3);
	return (axc_i32)dwCmdTextLen;
}

// 从一行文字中，找到指定的分隔符号（比如;），用于解析行内多选项文字，比如从文字 "value1;value2;value3" 中解析出 "value1","value2","value3"
// 返回值为szSubValueOut的字符串大小，不包括末尾的\0结束符；返回-1表示失败
// ppszNext用于返回下一个SubValue的开始位置
axc_i32 CHeatFinderUtility::CvmsHelpder_Cmd_GetValueSub(const char* szValueListText, const char chSep, char* szSubValueOut, const axc_dword dwSubValueOutSize, const char** ppszNext)
{
	if (NULL == szValueListText || 0 == szValueListText[0] || 0 == chSep || NULL == szSubValueOut || dwSubValueOutSize < 2 || NULL == ppszNext)
	{
		if (NULL != ppszNext)
		{
			*ppszNext = NULL;
		}
		return -1;
	}
	const char* pszFind = szValueListText;
	while (*pszFind != chSep && *pszFind != 0)
	{
		pszFind++;
	}
	if (*pszFind == 0 || *(pszFind + 1) == 0)
	{
		*ppszNext = NULL;
	}
	else
	{
		*ppszNext = pszFind + 1;
	}
	const axc_dword dwSubValueLen = (axc_dword)(pszFind - szValueListText);
	const axc_dword dwCopyLen = AXC_MIN(dwSubValueLen, dwSubValueOutSize - 1);
	if (dwCopyLen > 0)
	{
		memcpy(szSubValueOut, szValueListText, dwCopyLen);
	}
	*(szSubValueOut + dwCopyLen) = 0;
	return (axc_i32)(dwCopyLen);
}

axc_bool CHeatFinderUtility::PushFrameToTxFifo(CAxcFifo* fifo, const void* data, int len, axc_byte channel_index, int width, int height, axc_dword dwPts, axc_dword dwFrameSeq, axc_bool bIsKey)
{
    if(NULL == data || len <= 0 || NULL == fifo || width <= 0 || height <= 0)
    {
    	GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] Parameter error: Data? Data Length =0? Queue? width, height <=0?\n", __func__);
    	GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] Detail: Data(%s) Len(%d) Queue(%s) Width,Height(%d, %d)\n", __func__, ((NULL == data)?"Null":"OK"), len, ((NULL == fifo)?"Null":"OK"), width, height);
        return 0;
    }
    if(channel_index != ADE2CAM_CHANNEL_LEPTONRAW &&
       channel_index != ADE2CAM_CHANNEL_VISUALMAIN &&
       channel_index != ADE2CAM_CHANNEL_VISUALSUB )
    {
    	GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] unknown channel index: %d\n", __func__, channel_index);
        return 0;
    }
    if(channel_index > 7)
    {
    	GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] unknown channel index and more than 7: %d\n", __func__, channel_index);
        return 0;
    }

    T_ADE2CAM_FRAME_HEADER Ade2camFrameHeader;
    memset(&Ade2camFrameHeader, 0, sizeof(Ade2camFrameHeader));
    if(ADE2CAM_CHANNEL_LEPTONRAW == channel_index)
    {
        memcpy(&Ade2camFrameHeader.CodecFourcc, "LRV1", 4);
    }
    else
    {
        memcpy(&Ade2camFrameHeader.CodecFourcc, "H264", 4);
    }
    Ade2camFrameHeader.Width = width;
    Ade2camFrameHeader.Height = height;
    Ade2camFrameHeader.HeaderLen = sizeof(T_ADE2CAM_FRAME_HEADER);
    Ade2camFrameHeader.FrameLen = (unsigned int) len;
    Ade2camFrameHeader.FrameSeq = dwFrameSeq;
    Ade2camFrameHeader.TimeHigh = time(NULL);
    Ade2camFrameHeader.TimeLow = dwPts;
    Ade2camFrameHeader.FrameType = bIsKey ? 0 : 1; // 0 - keyfame, 1 - non keyframe
    Ade2camFrameHeader.Crc16 = CAxcCRC::Make_CCITT(data, len);

    int packet_num = 0, i = 0;
    const unsigned char* src = (const unsigned char*) data;
    unsigned short crc = 0;

    unsigned char BigTxBuffer[SPI_TX_PACKET_LEN * PAKCET_NUMBER_PER_IPP]; // 必须是SPI_TX_PACKET_LEN的整数倍，并且小于1500
    int iPacketNumberOfBigTxBuffer = 0;

    packet_num = (sizeof(T_ADE2CAM_FRAME_HEADER) + len + SPI_TX_DATA_LEN-1) / SPI_TX_DATA_LEN;
    iPacketNumberOfBigTxBuffer = 0;
    unsigned char* TxBuffer = BigTxBuffer;
    for(i=0; i<packet_num && len>0; i++)
    {
        int txbuf_valid_len = SPI_TX_DATA_LEN;
        unsigned char* txdata = TxBuffer + 4;

        // first send frame-header
        if(0 == i)
        {
            memcpy(txdata, &Ade2camFrameHeader, sizeof(T_ADE2CAM_FRAME_HEADER));
            txdata += sizeof(T_ADE2CAM_FRAME_HEADER);
            txbuf_valid_len -= sizeof(T_ADE2CAM_FRAME_HEADER);
        }
        // send fram-data
        if(len < txbuf_valid_len)
        {
            memcpy(txdata, src, len);
            memset(txdata+len, 0xFF, txbuf_valid_len-len);
            src += len;
            len = 0;
        }
        else
        {
            memcpy(txdata, src, txbuf_valid_len);
            src += txbuf_valid_len;
            len -= txbuf_valid_len;
        }

        TxBuffer[0] = ((unsigned char)channel_index) & 0x0F; // channel index on bit[0:3]
        TxBuffer[1] = 0xFF; // reserve
        TxBuffer[2] = 0;
        TxBuffer[3] = 0;

        crc = CAxcCRC::Make_CCITT(TxBuffer, SPI_TX_PACKET_LEN); // make crc
        TxBuffer[2] = (unsigned char)(crc >> 8);
        TxBuffer[3] = (unsigned char)(crc & 0xFF);

        if(0 == i)
        {
            TxBuffer[0] |= 0x80; // first packet
        }
        if((packet_num-1) == i)
        {
            TxBuffer[0] |= 0x40; // last packet
        }

        TxBuffer += SPI_TX_PACKET_LEN;
        iPacketNumberOfBigTxBuffer ++;
        if(iPacketNumberOfBigTxBuffer >= PAKCET_NUMBER_PER_IPP || (packet_num-1) == i)
        {
            fifo->Push(BigTxBuffer, SPI_TX_PACKET_LEN * iPacketNumberOfBigTxBuffer, (axc_ddword)channel_index);
            TxBuffer = BigTxBuffer;
            iPacketNumberOfBigTxBuffer = 0;
        }
    }
    //printf("N: [%s] success\n", __func__);
    return 1;
}

/**
 * return {err type}
 * {err type}:
 * =1 :init. fail
 * =2 :request deny, busy
 * =0 :others
 */
int CHeatFinderUtility::Utility_ChkRaspivid_strerr(int *count)
{
    int ret = 0;
    int retcount[3] = { 0, };

    FILE *l_fp = NULL;
    char L_token[128] = {""};
    //char L_token2[128] = {""};
    if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/tmp/raspi2"))) {
            l_fp = popen("cat /tmp/raspi2", "r");
    }else{
            //do no-op
    }

    if (l_fp == NULL) {
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s]: failed to get raspivid err, error:[%d] %s\n", "ChkRaspivid_strerr", errno, "can not read string by popen()");
    }
    else
    {
        while (fgets(L_token, sizeof(L_token)-1, l_fp) != NULL) {
            if ((strstr(L_token, "mmal_vc_init_fd") != NULL)
                      && (strstr(L_token, "could not open vchiq service") != NULL)){
                if (ret !=1){ ret = 1; }
                retcount[ret] += 1;
                GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s]: [mmal_vc_init_fd ] Raspivid cannot open vchiq service \n", "ChkRaspivid_strerr");
                continue;
            }

            if ((strstr(L_token, "mmal_vc_component_create") != NULL)
                && (strstr(L_token, "failed to initialise mmal ipc for 'vc.ril.camera' (7:EIO)") != NULL)){
                if (ret !=1){ ret = 1; }
                retcount[ret] += 1;
                GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s]: [mmal_vc_component_create] Raspivid fail initialise mmal ipc 'vc.ril.camera' (7:EIO)\n", "ChkRaspivid_strerr");
                continue;
            }

            if ((strstr(L_token, "mmal_component_create_core") != NULL)
                && (strstr(L_token, "could not create component 'vc.ril.camera' (7)") != NULL)){
                if (ret !=1){ ret = 1; }
                retcount[ret] += 1;
                GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s]: [mmal_component_create_core] Raspivid cannot create component 'vc.ril.camera' (7)\n", "ChkRaspivid_strerr");
                continue;
            }

            if ((strstr(L_token, "mmal_vc_component_enable") != NULL)
                && (strstr(L_token, "failed to enable component: ENOSPC") != NULL)){
                if (ret ==0){ ret = 2; }
                retcount[ret] += 1;
                GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s]: [mmal_vc_component_enable] Raspivid is busy, witch failed to enable component: ENOSPC\n", "ChkRaspivid_strerr");
                continue;
            }

            if (strstr(L_token, "camera component couldn't be enabled") != NULL){
                if (ret ==0){ ret = 2; }
                retcount[ret] += 1;
                GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s]: [mmal_component_create_core] Raspivid say: camera component couldn't be enabled\n", "ChkRaspivid_strerr");
                continue;
            }
        }
        pclose(l_fp);
    }


    int i =0;
    for (i =0; i<=2; i++){
        count[i] = retcount[i];
    }

    return ret;
}

void CHeatFinderUtility::SetActiveApp(void *pActiveApp)
{
    m_pActiveApp = pActiveApp;
}

void* CHeatFinderUtility::GetActiveApp()
{
    return m_pActiveApp;
}

void CHeatFinderUtility::SetADECameraSDK(void *pADECameraSDK)
{
    m_pADECameraSDK = pADECameraSDK;
}

void *CHeatFinderUtility::GetADECameraSDK()
{
    return m_pADECameraSDK;
}

/**
 * mark.hsieh ++
 */
void CHeatFinderUtility::SetGlobalsupport(void *pGlobalsupport)
{
	m_pGlobalsupport = pGlobalsupport;
}
void *CHeatFinderUtility::GetGlobalsupport(){
	return m_pGlobalsupport;
}

void CHeatFinderUtility::SetTimerService(void *pTimerSerivce){
	m_pTimerSerivce = pTimerSerivce;
}
void *CHeatFinderUtility::GetTimerService(){
	return m_pTimerSerivce;
}

void CHeatFinderUtility::SetSystemSignalListener(void *pSystemSignalListener){
	m_pSystemSignalListener = pSystemSignalListener;
}
void *CHeatFinderUtility::GetSystemSignalListener(){
	return m_pSystemSignalListener;
}

void CHeatFinderUtility::SetConfigObj(void *pConfigContext){
	m_pConfigContext = pConfigContext;
}
void *CHeatFinderUtility::GetConfigObj(){
	return m_pConfigContext;
}

void CHeatFinderUtility::SetLeptonSDK(void *pLeptonSDK){
	m_pLeptonSDK = pLeptonSDK;
}
void *CHeatFinderUtility::GetLeptonSDK(){
	return m_pLeptonSDK;
}

void CHeatFinderUtility::SetTwinkleLedShowNetworkUsed(){
	system("sudo killall -s SIGCONT ade_gpio_set 2>&1 1>/dev/null");

}

void CHeatFinderUtility::SetTwinkleLedShowStatusOK(){
    system("sudo killall -s SIGPWR ade_gpio_set 2>&1 1>/dev/null &");
}

void CHeatFinderUtility::SetTwinkleLedShowStatusWrong(){
	system("sudo killall -s SIGUSR1 ade_gpio_set 2>&1 1>/dev/null &");
}

void CHeatFinderUtility::SetTwinkleLedShowHeatAlarm(){
	system("sudo killall -s SIGALRM ade_gpio_set 3>&1 2>&1 1>/dev/null");
}

void CHeatFinderUtility::SetGPIORebootStep(){
	system("sudo killall -s SIGUSR2 ade_gpio_set &");
}

void CHeatFinderUtility::SetGPIOSrvRestart(){
	GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] restart ade_gpio_set because lose %d time\n", "Set GPIO Server Restart", WATCHDOG_GPIO_MAX_LOSE_COUNT);

	// can't use signal Terminate ... which will cause reboot.
	system("sudo kill -s SIGINT $(ps -A | grep ade_gpio_set | awk {'print $1'})");
	sleep(2);
	system("sudo /usr/bin/ade_gpio_set >/dev/null &");
	SetGPIOSrvChkRecordInit();
}

void CHeatFinderUtility::RecvGPIOSrvResponse(){
	GlobalDef *pGlobal = (GlobalDef *)m_pGlobalsupport;
	pGlobal->HealGPIOBody();
}
void CHeatFinderUtility::SendGPIOSrvChkNotify(){
	GlobalDef *pGlobal = (GlobalDef *)m_pGlobalsupport;
	pGlobal->AttackGPIOBody();
}
bool CHeatFinderUtility::IsGPIOSrvAlive(){
	GlobalDef *pGlobal = (GlobalDef *)m_pGlobalsupport;
	return pGlobal->ChkIsGPIOSrvAlive();
}
void CHeatFinderUtility::SetGPIOSrvChkRecordInit(){
	GlobalDef *pGlobal = (GlobalDef *)m_pGlobalsupport;
	pGlobal->HealGPIO2Breathe();
}

//
// 坐标转换
//   mode: 0 - 整除,  1 - 四舍五入,  2 - 中间值
//
void CHeatFinderUtility::CoordinateConvertRect(const RECT rcSrc, const int iSrcWidth, const int iSrcHeight, RECT* prcDest, const int iDestWidth, const int iDestHeight, const int iMode)
{
	if(prcDest)
	{
		if(2 == iMode)
		{
			prcDest->left   = (0 == (rcSrc.left   % iSrcWidth )) ? (rcSrc.left   * iDestWidth  / iSrcWidth ) : (((rcSrc.left   * iDestWidth ) / iSrcWidth ) + (iDestWidth  / (iSrcWidth *2)));
			prcDest->right  = (0 == (rcSrc.right  % iSrcWidth )) ? (rcSrc.right  * iDestWidth  / iSrcWidth ) : (((rcSrc.right  * iDestWidth ) / iSrcWidth ) + (iDestWidth  / (iSrcWidth *2)));
			prcDest->top    = (0 == (rcSrc.top    % iSrcHeight)) ? (rcSrc.top    * iDestHeight / iSrcHeight) : (((rcSrc.top    * iDestHeight) / iSrcHeight) + (iDestHeight / (iSrcHeight*2)));
			prcDest->bottom = (0 == (rcSrc.bottom % iSrcHeight)) ? (rcSrc.bottom * iDestHeight / iSrcHeight) : (((rcSrc.bottom * iDestHeight) / iSrcHeight) + (iDestHeight / (iSrcHeight*2)));
		}
		else if(1 == iMode)
		{
			prcDest->left   = ((rcSrc.left   * iDestWidth) + (iSrcWidth/2)) / iSrcWidth;
			prcDest->right  = ((rcSrc.right  * iDestWidth) + (iSrcWidth/2)) / iSrcWidth;
			prcDest->top    = ((rcSrc.top    * iDestHeight) + (iSrcHeight/2)) / iSrcHeight;
			prcDest->bottom = ((rcSrc.bottom * iDestHeight) + (iSrcHeight/2)) / iSrcHeight;
		}
		else
		{
			prcDest->left   = rcSrc.left   * iDestWidth / iSrcWidth;
			prcDest->right  = rcSrc.right  * iDestWidth / iSrcWidth;
			prcDest->top    = rcSrc.top    * iDestHeight / iSrcHeight;
			prcDest->bottom = rcSrc.bottom * iDestHeight / iSrcHeight;
		}
	}
}

double CHeatFinderUtility::AdjustTemperatureValue(double dbAvgTempe, int iMode, double dbTempeAdjust)
{
	if(1 == iMode)
	{
		return (dbAvgTempe + dbTempeAdjust);
	}
	else if(2 == iMode)
	{
		return (dbAvgTempe * (1.0 + (dbTempeAdjust/100.0)));
	}
	return dbTempeAdjust;
}

BOOL CHeatFinderUtility::TemperatureToText(double dbValue, int iMode, char* szOutText)
{
	if(szOutText)
	{
		switch(iMode)
		{
		case 0: sprintf(szOutText, "%.1fC", dbValue); break;
		case 1: if(dbValue>=0) sprintf(szOutText,"+%.1fC",dbValue); else sprintf(szOutText,"%.1fC",dbValue); break;
		case 2: if(dbValue>=0) sprintf(szOutText,"+%.1f%%",dbValue); else sprintf(szOutText,"%.1f%%",dbValue); break;
		default: return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CHeatFinderUtility::TextToTemperature(const char* szText, double* pdbValue, int* piMode)
{
	BOOL bResult = FALSE;
	if(szText != NULL && pdbValue != NULL && piMode != NULL)
	{
		char szValue[128];
		memset(szValue, 0, sizeof(szValue));
		memcpy(szValue, szText, sizeof(szValue)-1);
		size_t nLen = strlen(szValue);
		if(nLen > 0)
		{
			if('+' == szValue[0] || '-' == szValue[0])
			{
				if('%' == szValue[nLen-1])
				{
					szValue[nLen-1] = '\0';
					double dbValue = atof(szValue+1);
					if(dbValue >= 1 && dbValue <= 99)
					{
						if('-' == szValue[0])
						{
							dbValue = -dbValue;
						}
						bResult = TRUE;
						*pdbValue = dbValue;
						*piMode = 2;
					}
				}
				else
				{
					double dbValue = atof(szValue+1);
					if(dbValue != 0)
					{
						if('-' == szValue[0])
						{
							dbValue = -dbValue;
						}
						bResult = TRUE;
						*pdbValue = dbValue;
						*piMode = 1;
					}
				}
			}
			else
			{
				double dbValue = atof(szValue);
				if(0 != dbValue)
				{
					bResult = TRUE;
					*pdbValue = dbValue;
					*piMode = 0;
				}
			}
		}
	}
	return bResult;
}

//gavin ++ >>
axc_dword CHeatFinderUtility::GetTickCount()
{
	axc_dword dwTick = 0;
	struct timespec ts;
	if(clock_gettime(CLOCK_MONOTONIC,&ts) == 0)
	{
		dwTick = ts.tv_nsec / 1000000;
		dwTick += ts.tv_sec * 1000;
	}
	return dwTick;
}

int CHeatFinderUtility::GetCameraConfigParameter(const char* lpcszKey, char* szResult, size_t dwLen)
{
    FILE *l_fp = NULL;  //hamlet
    char command[128] = {0};
    char L_token[128] = {0};
    int nResult = -1;

    sprintf(command, "jq -r .%s /home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-merge/config/camera_config.json", lpcszKey);
    l_fp = popen(command, "r");

    if (l_fp == NULL) {
    	//CHeatFinderUtility::PrintNowData(1);
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [CHeatFinderUtility] %s: failed to get '%s' in camera_config.json\n", __func__, lpcszKey);
        szResult[0] = 0;
    }
    else
    {
		while (fgets(L_token, sizeof(L_token)-1, l_fp) != NULL) {
			char *pos;
			if ((pos=strchr(L_token, '\n')) != NULL) {
				*pos = '\0';
			}
			nResult = strlen(L_token);
			if (dwLen > nResult)
			{
				strcpy(szResult, L_token);
			}
		}
		pclose(l_fp);//hamlet
    }
    return nResult;
}
//gavin ++ <<
