/*
 * pluginDataTransmissionSupport.h
 *
 *  -> insert the data to queue
 *  	outside action: just write {each type belong one method}
 *  <- read the data from queue
 *  	outside action: use timer + call-back function {sigal method}
 *
 *  Created on: May 16, 2018
 *      Author: markhsieh
 */

#ifndef PLUGINDATATRANSMISSIONSUPPORT_H_
#define PLUGINDATATRANSMISSIONSUPPORT_H_

#include "axclib.h"
#include <vector>
#include <mutex>
#include <vector>

#ifndef PLUGINDATATXSUPPORT
#define PLUGINDATATXSUPPORT
#define SUPPORT		(0)
#define NOTSUPPORT	(1)
#define IOERROR		(2)
#endif


/** call back define **/
typedef void (*OnThermalDataReceived)(void *pContext, const int FrameWidth, const int FrameHeight, bool IsCompress, const int BufSize, short *pBuf);
typedef struct tagThermalDataReceivedEventHandler
{
	OnThermalDataReceived fnEvent;
    void *pContext;
}xThermalDataReceivedEventHandler;

typedef void (*OnVisionH264DataReceived)(void *pContext, const int BufSize, unsigned char *pBuf);
typedef struct tagVisionH264DataReceivedEventHandler
{
	OnVisionH264DataReceived fnEvent;
    void *pContext;
}xVisionH264DataReceivedEventHandler;

typedef void (*OnVisionRawDataReceived)(void *pContext, const int Width, const int Height, const int RawFMT, const int size, unsigned char *pBuf);
typedef struct tagVisionRawDataReceivedEventHandler
{
	OnVisionRawDataReceived fnEvent;
    void *pContext;
}xVisionRawDataReceivedEventHandler;

/** struct **/
typedef struct PluginThermalUnit{
    uint16_t mDataWidth;
    uint16_t mDataHeight;
    bool	mIsCompress;
    uint32_t mDataSize;
    short *mThermalData;

    PluginThermalUnit()
    {
    	mThermalData = NULL;
    	mDataSize = 0;
    	mDataWidth = 0;
    	mDataHeight =0;
        mIsCompress =false;
    }
    ~PluginThermalUnit(){
    	Release();
    }
    void Release(){
        if ((mThermalData!=NULL)&&(mDataSize>0)){
        	if((mThermalData))
        	{ delete [] (mThermalData); mThermalData = 0; }
    	}
    	mDataSize = 0;
    	mDataWidth = 0;
    	mDataHeight =0;
        mIsCompress =false;
    }
    void CopyData(short *pBuffer, const axc_i32 size, bool isCompress, const uint16_t iWidth, const uint16_t iHeight)
    {
        if ((mThermalData!=NULL)&&(mDataSize>0)){
        	if((mThermalData))
        	{ delete [] (mThermalData); mThermalData = 0; }
    	}
        mThermalData = new short[size];
        memcpy(mThermalData, pBuffer, size);
        mDataSize = size;
    	mDataWidth = iWidth;
    	mDataHeight = iHeight;
        mIsCompress = isCompress;
    }

}xPluginThermalUnit;

class CPluginDataTransmissionSupport{
public:
	CPluginDataTransmissionSupport();
	~CPluginDataTransmissionSupport();

	bool AddThermalDataReceivedNotifyEvent(OnThermalDataReceived fnEvent, void *pContext);
	bool AddVisionH264DataReceivedNotifyEvent(OnVisionH264DataReceived fnEvent, void *pContext);
	bool AddVisionRawDataReceivedNotifyEvent(OnVisionRawDataReceived fnEvent, void *pContext);

	int AddReceivedThermalFrame(const int FrameWidth, const int FrameHeight, bool IsCompress, const int BufSize, short *pBuf);
	int AddReceivedH264Frame(const int BufSize, unsigned char *pBuf);
	int AddReceivedRGBFrame(const int Width, const int Height, const int RawFMT, const int size, unsigned char *pBuf);

	short getOneOfAlreadyStoredFramesPerQueue();
protected:
	bool getThermalDataFromQueue();
	bool getVisionH264DataFromQueue();
	bool getVisionRawDataFraomQueue();

	void fireThermalDataReceivedNotify(const int FrameWidth, const int FrameHeight, bool IsCompress, const int BufSize, short *pBuf);
	void fireVisionH264DataReceivedNotify(const int BufSize, unsigned char *pBuf);
	void fireVisionRawDataReceivedNotifyEvent(const int Width, const int Height, const int RawFMT, const int size, unsigned char *pBuf);

private:
	std::vector<xThermalDataReceivedEventHandler> m_pluginThermalDataReceivePortListenrList;
	std::vector<xVisionH264DataReceivedEventHandler> m_pluginVisionH264DataReceivePortListenrList;
	std::vector<xVisionRawDataReceivedEventHandler> m_pluginVisionRawDataReceivePortListenrList;

	std::mutex m_thermal_locker;
	std::mutex m_visionh264_locker;
	std::mutex m_visionraw_locker;

	std::vector<xPluginThermalUnit*> m_ThermalUnitList;
};


#endif /* PLUGINDATATRANSMISSIONSUPPORT_H_ */
