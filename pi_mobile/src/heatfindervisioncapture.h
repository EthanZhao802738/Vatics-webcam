#ifndef HEATFINDERVISIONCAPTURE_H
#define HEATFINDERVISIONCAPTURE_H

#include "globaldef.h"
#include <mutex>
#include <vector>

#include "raspberryPIcamera.h"
#include "FpsHelper.h"
#include "RingBuffer.h"

//#define VISION_STREAM_RESTART       1
//#define VISION_STREAM_RELOAD        2
#define VISION_FRAME_REWORK	true
#define VISION_FRAME_REGROUP true
#define RPI_RESTART_CONF_RELOAD (1)
#define RPI_RESTART_CONF_RENEW	(2)
#define RPI_RESTART_CONF_NOCHANGE	(3)

#define VISION_TIMEOUT_LIMITATION	(1*60)

// H.264 NAL type
enum H264NALTYPE{
H264NT_NAL = 0,
H264NT_SLICE,
H264NT_SLICE_DPA,
H264NT_SLICE_DPB,
H264NT_SLICE_DPC,
H264NT_SLICE_IDR,
H264NT_SEI,
H264NT_SPS,
H264NT_PPS,
};


typedef void (*OnVisionSizeChangedNotify)(void *pContext, const unsigned int Width, const unsigned int Height);
typedef struct tagVisionSizeChangedNotifyHandler
{
    OnVisionSizeChangedNotify fnEvent;
    void *pContext;
}xVisionSizeChangedNotifyHandler;

/**
 * use :ade_camera_sdk *ac_obj = (ade_camera_sdk *) CHeatFinderUtility::GetADECameraSDK();
 * to satisfy this demand
typedef void (*OnVisionStreamChangedNotify)(void *pContext, const int StreamChangedType);
typedef struct tagVisionStreamChangedNotifyHandler
{
    OnVisionStreamChangedNotify fnEvent;
    void *pContext;
}xVisionStreamChangedNotifyHandler;
**/

//--------------------------------------------
typedef void (*OnLedStatusTwinkleNotify)(void *pContext, const bool value);
typedef struct tagVLedStatusTwinkleNotifyHandler
{
    OnLedStatusTwinkleNotify fnEvent;
    void *pContext;
}xVLedStatusTwinkleNotifyHandler;

/**
 * channelIndex:
 * globaldef.h ...
 * define VISION_xxxPKGSEND2_ooo
 */
typedef void (*OnVDataReceivedEvent)(void *pContext, axc_byte *pBuf, const axc_i32 size, const unsigned short channelIndex);
typedef struct tagVDataReceivedEventHandler
{
    OnVDataReceivedEvent fnEvent;
    void *pContext;

}xVDataReceivedEventHandler;

typedef int (*OnRawDataReceivedEvent)(void *pContext, const axc_i32 Width, const axc_i32 Height, const axc_i32 RawFMT, const axc_i32 size, axc_byte *pBuf);
typedef struct tagRawDataReceivedEventHandler
{
	OnRawDataReceivedEvent fnEvent;
    void *pContext;
}xRawDataReceivedEventHandler;

typedef int (*OnSetThermalOverlay) (void *pOwner , xLeptonOverlay &xLoi);
typedef struct tagSetOnTimeThermalOverlayNotifyHandler
{
	OnSetThermalOverlay fnEvent;
    void *pContext;
}xSetOnTimeThermalOverlayNotifyHandler;

typedef void (*OnVisionRestartFinishNotifyEvent)(void* pContext);
typedef struct tagVisionRestartFinishNotifyEventHandler
{
	OnVisionRestartFinishNotifyEvent fnEvent;
    void *pContext;
}xVisionRestartFinishNotifyEventHandler;

//--------------------------------------------

class CHeatFinderVisionCapture
{
public:
    CHeatFinderVisionCapture();
    ~CHeatFinderVisionCapture();

    bool Run();
    void Stop();
    void SetRestartVision(const bool value);
    void SetIsDataSend2UDP(const bool value);
    void PrintVisionStatus(const bool value);

    void SetH264Send2encoderStatus(const bool bValue);
    bool GetH264Send2encoderStatus();

    void SetRawSend2PluginStatus(const bool bValue);
    bool GetRawSend2PluginStatus();

    void AddVisionSizeChangedNotify(OnVisionSizeChangedNotify fnEvent, void *pContext);
    void AddLedStatusTwinkleNotify(OnLedStatusTwinkleNotify fnEvent, void *pContext);
    void AddDataReceivedEvent(OnVDataReceivedEvent fnEvent, void *pContext);

    axc_dword thread_process();

    void AddRawDataReceivedEvent(OnRawDataReceivedEvent fnEvent, void *pContext);
    void AddSetThermalOverlayNotify(OnSetThermalOverlay fnEvent, void *pContext);

    axc_dword thread_process_ReadDataFromMEM();

    void AddVisionRestartFinishReceivedEvent(OnVisionRestartFinishNotifyEvent fnEvent, void* pContext);
protected:

    static axc_dword thread_process(CAxcThread* pThread, void* pContext)
    {
        CHeatFinderVisionCapture *pSender = reinterpret_cast<CHeatFinderVisionCapture*> (pContext);
        return pSender->thread_process();
    }

    bool restartRPIEncoder(unsigned short iType);

    void fireVisionSizeChangedNotify(const unsigned int Width, const unsigned int Height);
    void fireLedStatusTwinkleNotify(const bool bValue);
    void fireDataReceivedEventHandler(unsigned char *pBuf, const int BufSize, const unsigned short channelIndex);

    void fireRawDataReceivedEvnetHandler(const axc_i32 Width, const axc_i32 Height, const axc_i32 RawFMT, const axc_i32 size, axc_byte *pBuf);
    void fireSetThermalOverlayNotify(xLeptonOverlay &xLoi);
    void fireVisionRestartFinishNotify();

    static void OnAnalysisH264PKG(void* pContext, const axc_i32 size, axc_byte *pBuf){
    	CHeatFinderVisionCapture *pSender = reinterpret_cast<CHeatFinderVisionCapture*> (pContext);
    	pSender->OnAnalysisH264PKG(pBuf, (axc_dword)size);
    }
    void OnAnalysisH264PKG(axc_byte* pData, axc_dword dwDataSize);

    void OnRefactoringH264Data(axc_byte* pData, axc_dword dwDataSize);
    void OnReceivedH264Frame(axc_byte* pData, axc_dword dwDataSize);

    static void OnReceivedRawData(void *pContext, const axc_i32 Width, const axc_i32 Height, const axc_i32 RawFMT, const axc_i32 size, axc_byte *pBuf){
    	CHeatFinderVisionCapture *pSender = reinterpret_cast<CHeatFinderVisionCapture*> (pContext);
    	pSender->OnReceivedRawData(Width, Height, RawFMT, size, pBuf);
    }
    void OnReceivedRawData(const axc_i32 Width, const axc_i32 Height, const axc_i32 RawFMT, const axc_i32 size, axc_byte *pBuf);

    int FindH264StartCode(axc_byte* pData, axc_i32 iDataSize);

    axc_bool IsH264StartCode(axc_byte* pData);

    void sendDefaultConfigEventNotify();
    void sendImportConfigEventNotify();
    bool setVisioParameter2RPICamera(bool bType, bool bInit);

    static axc_dword thread_process_ReadDataFromMEM(CAxcThread* pThread, void* pContext){
    	CHeatFinderVisionCapture *pSender = reinterpret_cast<CHeatFinderVisionCapture*> (pContext);
    	return pSender->thread_process_ReadDataFromMEM();
    }
    void OnRegroupH264Data(axc_byte* pData, axc_dword dwDataSize);

    int readMem(axc_byte* pData, axc_dword dwDataSize);
private:
    char* getBootingParameterIdentify(int bootingId);
    //CMemoryCache m_memoryCache; //gavin --

    CAxcThread m_hThread;       //g_threadUdpSend
    bool m_bStopThread;         //stop thread flag
    std::mutex m_locker;
    double m_fLastFrameTimeMs;
    bool m_bRestartVision;

    axc_byte m_abyH264Sps[1024];
    axc_byte m_abyH264Pps[1024];
    bool m_bPrintVisionStatus;
    SIZE m_pvs_sizeVisionMainImage;
    std::mutex m_data2udp_locker;
    bool m_bDataSend2Udp_enable;
    std::mutex m_chkFirstFrameType_locker;
    bool m_bChkFirstFrameisI_afterDataSend2UdpOffOn;
    bool m_bChkFirstFrameisIEnable;

    bool m_bh264send2encoder;
    bool m_brawsend2plugin;

    bool m_bReservationDataSend2UDP;

    raspberryPIcamera m_RPICameraFW;
    std::mutex m_memRPI_locker;

    unsigned int m_iH264SPSLen;
    unsigned int m_iH264PPSLen;
    CAxcMemory m_memRPICamFrameRework;
    bool m_bRawEncodeRestart;

    //CAxcThread m_hThreadReadVectorList;

    int m_iRawDataWidth;
    int m_iRawDataHeight;

    double m_fLastRawControlTimeMs;

    bool	m_bFrameNotCollectDone;
    unsigned int	m_IPFrameCollectedLength;

    CAxcThread m_hThreadReadMem;

    //Arsenal
    std::vector<xVisionSizeChangedNotifyHandler> m_visionsizechangenotifyhandlerList;
    std::vector<xVLedStatusTwinkleNotifyHandler> m_ledstatustwinklenotifyhandlerList;
    std::vector<xVDataReceivedEventHandler> m_datareceivedeventList;

    std::vector<xRawDataReceivedEventHandler> m_rawdatareceivedhandleList;
    std::vector<xSetOnTimeThermalOverlayNotifyHandler> m_SetThermalOverlaynotifyhandlerList;
    std::vector<xVisionRestartFinishNotifyEventHandler> m_VisionRestartFinishNotifyEventListener;

private:
    CRingBuffer m_memoryCache;

    CFpsHelper fpsH264Frame;
};

#endif // HEATFINDERVISIONCAPTURE_H
