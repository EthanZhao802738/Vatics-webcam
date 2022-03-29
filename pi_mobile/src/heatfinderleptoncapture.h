#ifndef HEATFINDERLEPTONCAPTURE_H
#define HEATFINDERLEPTONCAPTURE_H

#include "globaldef.h"
#include <mutex>
#include <vector>
#include <pthread.h>
#include <semaphore.h>

#include "adeSDKAPIleptonthermal.h"

#define THERMAL_SOURCE_DEV_FLIR_LWIR	(1 << 31)
#define THERMAL_SUPPORT_GROUP			(THERMAL_SOURCE_DEV_FLIR_LWIR)

#define THERMAL_DATA_SIZE (12800)  //Hank
//#define THERMAL_DATA_SIZE 10240
char const thermalDataStartCode[4] = {0x01, 0x03, 0x05, 0x07};
char const thermalDataLengthSCode[4] = {0x0a, 0x0d, 0x0e, 0x01};
char const thermalDataTimeStampSCode[4] = {0x0a, 0x0d, 0x0e, 0x02};
char const thermalDataRawSCode[4] = {0x0a, 0x0d, 0x0e, 0x03};

struct VCEThermalHeader {
    uint8_t mStartCode[4];
    uint32_t mDataSize;
    uint64_t mTimeStamp;
    uint16_t mDataWidth;
    uint16_t mDataHeight;
    VCEThermalHeader(): mDataSize(0), mTimeStamp(0), mDataWidth(80), mDataHeight(80) {
        // Add start code for thermal data: 0x01, 0x03, 0x05, 0x07
        memcpy(mStartCode, thermalDataStartCode, sizeof(mStartCode));
        // Add time stamp
        struct timeval now;
        gettimeofday(&now, NULL);
        mTimeStamp = (int64_t) now.tv_sec * 1000000 + (int64_t) now.tv_usec;
        mDataSize = mDataWidth * mDataHeight * 2;
    }
    int GetHeaderSize() {
        return (sizeof(mStartCode) + sizeof(mDataSize) + sizeof(mTimeStamp)
                + sizeof(mDataWidth) + sizeof(mDataHeight));
    }
};

struct VCEThermalData {
/*	char mStartCode[4];
    int mDataSize;
    int64_t mTimeStamp;
    char mThermalData[THERMAL_DATA_SIZE];
*/

    VCEThermalHeader mHeader;
    int8_t mThermalData[THERMAL_DATA_SIZE];
    VCEThermalData(): mHeader()/*, mThermalData(NULL)*/ {}
    ~VCEThermalData() {
        if(mThermalData) {
            //delete [] mThermalData;
            //mThermalData = NULL;
        }
    }
};

typedef void (*OnDataReceivedEvent)(void *pContext, axc_byte *pBuf, const axc_i32 size, const axc_ddword channelIndex);
typedef struct tagDataReceivedEventHandler
{
    OnDataReceivedEvent fnEvent;
    void *pContext;
    unsigned short type;
}xDataReceivedEventHandler;
//
typedef void (*OnLedStatusTwinkleNotify)(void *pContext, const bool value);
typedef struct tagLedStatusTwinkleNotifyHandler
{
    OnLedStatusTwinkleNotify fnEvent;
    void *pContext;
}xLedStatusTwinkleNotifyHandler;

//
typedef void (*OnLedStatusTurnOnNotify)(void *pContext, const bool value);
typedef struct tagLedStatusTurnOnNotifyHandler
{
    OnLedStatusTurnOnNotify fnEvent;
    void *pContext;
}xLedStatusTurnOnNotifyHandler;

typedef void (*OnThermalSizeChangedNotify)(void *pContext, const unsigned int Width, const unsigned int Height);
typedef struct tagThermalSizeChangedNotifyHandler
{
    OnThermalSizeChangedNotify fnEvent;
    void *pContext;
}xThermalSizeChangedNotifyHandler;

/**
 * mark.hsieh ++
 */
struct ThermalStatusChangedEvent{
	bool	pauseplay;	//false:pause, true:play
};
typedef struct tagThermalStatusChangedNotifyHandler
{
	ThermalStatusChangedEvent objEvent;
    void *pContext;
}xThermalStatusChangedNotifyHandler;
typedef void (*OnDataPopedEvent)(void *pContext, axc_byte *pBuf, const axc_i32 size);
typedef struct tagDataPopedEventHandler
{
	OnDataPopedEvent fnEvent;
    void *pContext;
}xDataPopedEventHandler;

typedef void (*OnHeatObjectReceivedEvent)(void *pContext, xPluginHeatObject *pPluginHeatObj);
typedef struct tagHeatObjectReceivedEventHandler
{
	OnHeatObjectReceivedEvent fnEvent;
    void *pContext;
}xHeatObjectReceivedEventHandler;

class CHeatFinderLeptonCapture
{
public:
    CHeatFinderLeptonCapture();
    ~CHeatFinderLeptonCapture();

    bool Run();
    void Stop();
    void PrintLeptonStatus(const bool value);

    void AddDataReceivedEvent(OnDataReceivedEvent fnEvent, unsigned short type, void *pContext);
    void AddLedStatusTwinkleNotifyEvent(OnLedStatusTwinkleNotify fnEvent, void *pContext);
    void AddThermalSizeChangedNotifyEvent(OnThermalSizeChangedNotify objEvent, void *pContext);

    /**
     * mark.hsieh ++
     */
    void AddThermalStatusChangedNotifyEvent(ThermalStatusChangedEvent objEvent, void *pContext);
    void AddDataPopedEvent(OnDataPopedEvent fnEvent, void *pContext);
    void SetSend2PluginPausePlay(bool bValue);
    bool GetSend2PluginPausePlay();

    void AddHeatObjectReceivedNotifyEvent(OnHeatObjectReceivedEvent fnEvent , void *pContext);
    bool setIntervalOfAnalyzeHeatObjectWhenIdle(double fInterval);

    void AddReceivedThermalRestartEventNotify(OnReceivedThermalRestartFinishNotifyEvent fnEvent , void *pContext);
private:

    CAxcThread m_hThread;       //g_threadUdpSend
    bool m_bStopThread;         //stop thread flag
    std::mutex m_locker;
    bool m_bPrintLeptonStatus;

    SIZE m_sizeThermalImage;

    std::vector<xDataReceivedEventHandler> m_DataReceivedEventList;
    std::vector<xLedStatusTwinkleNotifyHandler> m_LedStatusTwinkleNotifyList;
    std::vector<xLedStatusTurnOnNotifyHandler> m_LedStatusTurnOnNotifyList;
    std::vector<xThermalSizeChangedNotifyHandler> m_ThermalSizeChangedNotifyList;

    /**
     * mark.hsieh ++
     */
    std::vector<xThermalStatusChangedNotifyHandler> m_ThermalStatusChangedList;
    //CAxcFifo m_fifoNetSend;
    //CAxcThread m_hfifoThread;
    std::vector<xDataPopedEventHandler> m_DataPopedEventList;
    bool m_send2plugin_pauseplay;

    CadeSDKAPIleptonthermal mLeptonSDK;
    bool	m_send2MediaRTSPSer_pauseplay;
    CAxcThread m_hThread_IVSOUT;

    std::mutex m_queuelocker;
    axc_u32	m_iQueueCacheLen;
    CAxcMemory m_memThermalOutput;
    CAxcThread m_hThread_READ;
    sem_t requestOutput;

    CAxcMemory		m_caxcmemThermalCompress;

    std::vector<xHeatObjectReceivedEventHandler> m_HeatObjectReceivedNotifyList;

protected:
    static axc_dword thread_read_thermal_buffer_process(CAxcThread* pThread, void* pContext)
    {
        CHeatFinderLeptonCapture *pSender = reinterpret_cast<CHeatFinderLeptonCapture*> (pContext);
        return pSender->thread_read_thermal_buffer_process();
    }
    axc_dword thread_read_thermal_buffer_process();

    void ProcessSendThermalView(axc_byte* pData, axc_dword dwDataSize, axc_dword flags);
    void fireLedStatusTwinkleNotify(const bool value);
    void fireLedStatusTurnOnNotify(const bool value);
    void fireThermalSizeChangedNotify(const unsigned int Width, const unsigned int Height);
    void fireHeatObjectSendNotify(xPluginHeatObject *pPluginHeatObj);

    /**
     * mark.hsieh ++
     *
     */
    static axc_dword thread_virtual_fifo_process(CAxcThread* pThread, void* pContext)
    {
        CHeatFinderLeptonCapture *pSender = reinterpret_cast<CHeatFinderLeptonCapture*> (pContext);
        return pSender->thread_virtual_fifo_process();
    }
    axc_dword thread_virtual_fifo_process();

    static axc_dword thread_ivsout_process(CAxcThread* pThread, void* pContext)
    {
        CHeatFinderLeptonCapture *pSender = reinterpret_cast<CHeatFinderLeptonCapture*> (pContext);
        return pSender->thread_ivsout_process();
    }
    axc_dword thread_ivsout_process();

    static axc_dword thread_read_process(CAxcThread* pThread, void* pContext)
    {
        CHeatFinderLeptonCapture *pSender = reinterpret_cast<CHeatFinderLeptonCapture*> (pContext);
        return pSender->thread_read_process();
    }
    axc_dword thread_read_process();

    int readMem(axc_byte* pData, axc_dword dwDataSize);
    int writeMem(axc_byte* pData, axc_dword dwDataSize);

    // call back function
    static void OnReceivedThermalData(void *pContext, axc_byte* pData, axc_i32 ldDataSize, T_ADETHERMAL_RAWDATA_FRAME *raw)
	{
    	CHeatFinderLeptonCapture *pSender = reinterpret_cast<CHeatFinderLeptonCapture*> (pContext);
		return pSender->OnReceivedThermalData(pData, ldDataSize, raw);
	}
    void OnReceivedThermalData(axc_byte* pData, axc_i32 ldDataSize, T_ADETHERMAL_RAWDATA_FRAME *raw);

    static void OnReceivedHeatObjects(void *pContext, xPluginHeatObject *pPluginHeatObj)
	{
    	CHeatFinderLeptonCapture *pSender = reinterpret_cast<CHeatFinderLeptonCapture*> (pContext);
		return pSender->OnReceivedHeatObjects(pPluginHeatObj);
	}
    void OnReceivedHeatObjects(xPluginHeatObject *pPluginHeatObj);
};

#endif // HEATFINDERLEPTONCAPTURE_H
