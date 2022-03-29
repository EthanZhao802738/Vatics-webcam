/*
 * pluginDataTransmissionSupport.cpp
 *
 *  Created on: May 16, 2018
 *      Author: markhsieh
 */

#include "pluginDataTransmissionSupport.h"
#include "heatfinderconfigmanager.h"

CPluginDataTransmissionSupport::CPluginDataTransmissionSupport(){

}
CPluginDataTransmissionSupport::~CPluginDataTransmissionSupport(){

}

bool CPluginDataTransmissionSupport::AddThermalDataReceivedNotifyEvent(OnThermalDataReceived fnEvent, void *pContext){
	bool _bRst = false;
	xThermalDataReceivedEventHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;

    m_pluginThermalDataReceivePortListenrList.push_back(handler);
	return _bRst;
}
bool CPluginDataTransmissionSupport::AddVisionH264DataReceivedNotifyEvent(OnVisionH264DataReceived fnEvent, void *pContext){
	bool _bRst = false;
	xVisionH264DataReceivedEventHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;

    m_pluginVisionH264DataReceivePortListenrList.push_back(handler);
	return _bRst;
}
bool CPluginDataTransmissionSupport::AddVisionRawDataReceivedNotifyEvent(OnVisionRawDataReceived fnEvent, void *pContext){
	bool _bRst = false;
	xVisionRawDataReceivedEventHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;

    m_pluginVisionRawDataReceivePortListenrList.push_back(handler);
	return _bRst;
}

int CPluginDataTransmissionSupport::AddReceivedThermalFrame(const int FrameWidth, const int FrameHeight, bool IsCompress, const int BufSize, short *pBuf){
	int _iRst = SUPPORT;

	m_thermal_locker.lock();
	unsigned int iQueueSize = m_ThermalUnitList.size();
	bool bIsNeedErase = false;

	if (iQueueSize >= m_ThermalUnitList.max_size() -1){
		GLog(tPLUGINTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] remove oldest data, because thermal queue is full (%u/ %u)\n", "Plugin Tx Support", iQueueSize, (m_ThermalUnitList.max_size()-1));
		//clear
		bIsNeedErase = true;
	}

    //Error in `./adehf_mobile': double free or corruption (out): 0x7172b898
	xPluginThermalUnit *xBuf = new xPluginThermalUnit();
    xBuf->CopyData(pBuf, BufSize, IsCompress, (uint16_t)FrameWidth, (uint16_t)FrameHeight);

    if (bIsNeedErase){
        xPluginThermalUnit *xT = m_ThermalUnitList.front();
        delete xT;
        xT = NULL;
    	m_ThermalUnitList.erase(m_ThermalUnitList.begin());
    	iQueueSize = 0;
    }

    m_ThermalUnitList.push_back(xBuf);
    unsigned int iQueueNewSize = m_ThermalUnitList.size();

    if (iQueueNewSize <= iQueueSize){
		GLog(tPLUGINTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] add data %p@%d to thermal queue fail:%s\n", "Plugin Tx Support",pBuf, BufSize, (iQueueNewSize <= iQueueSize)?"data not push into queue":"queue valid");
		_iRst = IOERROR;
	}
	m_thermal_locker.unlock();
	return _iRst;
}
int CPluginDataTransmissionSupport::AddReceivedH264Frame(const int BufSize, unsigned char *pBuf){

	m_visionh264_locker.lock();


	m_visionh264_locker.unlock();
	return NOTSUPPORT;
}
int CPluginDataTransmissionSupport::AddReceivedRGBFrame(const int Width, const int Height, const int RawFMT, const int size, unsigned char *pBuf){

	m_visionraw_locker.lock();

	m_visionraw_locker.unlock();
	return NOTSUPPORT;
}

short CPluginDataTransmissionSupport::getOneOfAlreadyStoredFramesPerQueue(){
	short _iDelayCount =0;

	if (!getThermalDataFromQueue()){
		_iDelayCount +=1;
	}
	if (!getVisionH264DataFromQueue()){
		//_iDelayCount +=1;
	}
	if (!getVisionRawDataFraomQueue()){
		//_iDelayCount +=1;
	}

	/**
	switch(_iDelayCount){
	case 3:
		usleep(5*1000);
	case 2:
		usleep(5*1000);
	case 1:
		usleep(10*1000);
		break;
	default:
		break;
	}
	**/

	return _iDelayCount;
}

bool CPluginDataTransmissionSupport::getThermalDataFromQueue(){
    if ((m_ThermalUnitList.size() <= 0) || (m_ThermalUnitList.empty())){
    	return false;
    }

    if (m_thermal_locker.try_lock()){

    	xPluginThermalUnit lxBuf;
        xPluginThermalUnit *pSendBuf = m_ThermalUnitList.front();
        lxBuf.CopyData(pSendBuf->mThermalData, pSendBuf->mDataSize, pSendBuf->mIsCompress, pSendBuf->mDataWidth, pSendBuf->mDataHeight);
        m_ThermalUnitList.erase(m_ThermalUnitList.begin());
        delete pSendBuf;
    	m_thermal_locker.unlock();
    	fireThermalDataReceivedNotify(lxBuf.mDataWidth, lxBuf.mDataHeight, lxBuf.mIsCompress, lxBuf.mDataSize, lxBuf.mThermalData);

    }else{
    	return false;
    }
	return true;
}
bool CPluginDataTransmissionSupport::getVisionH264DataFromQueue(){
	return false; //not support yet
}
bool CPluginDataTransmissionSupport::getVisionRawDataFraomQueue(){
	return false; //not support yet

}

void CPluginDataTransmissionSupport::fireThermalDataReceivedNotify(const int FrameWidth, const int FrameHeight, bool IsCompress, const int BufSize, short *pBuf){

	if (m_pluginThermalDataReceivePortListenrList.size() <=0){
		return;
	}

	for(unsigned int i = 0; i < m_pluginThermalDataReceivePortListenrList.size(); ++i){
		m_pluginThermalDataReceivePortListenrList[i].fnEvent(m_pluginThermalDataReceivePortListenrList[i].pContext, FrameWidth, FrameHeight, IsCompress, BufSize, pBuf);
	}
}
void CPluginDataTransmissionSupport::fireVisionH264DataReceivedNotify(const int BufSize, unsigned char *pBuf){
	//not support yet
}
void CPluginDataTransmissionSupport::fireVisionRawDataReceivedNotifyEvent(const int Width, const int Height, const int RawFMT, const int size, unsigned char *pBuf){
	//not support yet
}
