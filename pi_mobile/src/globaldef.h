#ifndef GLOBALDEF_H
#define GLOBALDEF_H

#include "axclib_os_include.h"
#include "axclib.h"
#include "pluginmanager.h"  //hamlet
#include <mutex>
#include <vector>

#include "System/safepointer.h"
#include "heatfinderlogmanager.h"
// define list
#define		SYS_EXECUTE_NICKNAME	"ADE_Camera_IF"
#define MAX_UDP_SESSION_NUMBER	(64)

using namespace std;

#define		SYS_EXECUTE_VERSION		"3.0.0.0"
#ifdef MACHINE_TARGET_PI
    #define 	DEVICE_TYPE			"AD-HF048-P"
#elif defined(MACHINE_TARGET_PI_WP)
    #define 	DEVICE_TYPE			"AD-HF048-WP"
#elif defined(MACHINE_TARGET_PI_WPSR)
    #define 	DEVICE_TYPE			"AD-HF048-WPSR"
#elif defined(MACHINE_TARGET_PI_ML)
    #define 	DEVICE_TYPE			"AD-HF048-ML"
#elif defined(MACHINE_TARGET_PI_ER)
    #define 	DEVICE_TYPE			"AD-HF048-ER"
#elif defined(MACHINE_TARGET_PI_SR)
    #define 	DEVICE_TYPE			"AD-HF048-SR"
#elif defined(MACHINE_TARGET_TI)
    #define 	DEVICE_TYPE			"AD-HF048-T"
#elif defined(MACHINE_TARGET_INTEL)
    #define 	DEVICE_TYPE			"AD-HF048-I"
#else
    #define 	DEVICE_TYPE			"AD-HF048-U"
    #error 		"Invalidate Machine-Target !"
#endif
#define		DEVICE_LABEL			"AD-HF048"

#define SPI_TX_PACKET_LEN		(164)
#define SPI_TX_DATA_LEN			(SPI_TX_PACKET_LEN - 4)
#define PAKCET_NUMBER_PER_IPP	(5)  // 每个UDP/TCP包最多包括几个ADE2CAM小包
#define PORT_TCP_LISTEN			(5555)
#define PORT_TCP_REMOTECTL		(5558)
#define PORT_UDP_COMMANDSRV		(5556)
#define PORT_UDP_VIEWSEND		(5557)
#define serverIP				"127.0.0.1"
#define serverH264DataPort		(2190)
#define serverThermalDataPort	(2191)

#define TCP_SESSION_CONTEXT_OPEN_THERMAL (0x1) // tcp session: open stream for thermal
#define TCP_SESSION_CONTEXT_OPEN_VISION  (0x2) // tcp session: open stream for vision
#define UDP_SESSION_CONTEXT_OPEN_THERMAL TCP_SESSION_CONTEXT_OPEN_THERMAL
#define UDP_SESSION_CONTEXT_OPEN_VISION  TCP_SESSION_CONTEXT_OPEN_VISION

#define MEMVisionCapture_Valid	(0x0001)
#define MEMVisionFrame_Valid	(0x0001)

#define ADE2CAM_CHANNEL_LEPTONRAW	(0)
#define ADE2CAM_CHANNEL_VISUALMAIN	(1)
#define ADE2CAM_CHANNEL_VISUALSUB	(2)

#define LEPTON_PKGSEND2_UDPMEDIACLIENT	(0)
#define LEPTON_PKGSEND2_PLUGINTARGET	(1)

#define VISION_H264PKGSEND2_TCPPORT5555 (0)
#define VISION_H264PKGSEND2_UDPPORT5557 (1)
#define VISION_H264PKGSEND2_ENCODER (2)
#define VISION_H264PKGSEND2_MEDIASRV (3)
#define VISION_H264PKGSEND2_PLUGIN (4)
#define VISION_RAWPKGSEND2_PLUGIN (5)


#define TCPSENDER_OUTPUT_VISION (0x01)
#define TCPSENDER_OUTPUT_THERMAL (0x02)
#define UDPSENDER_OUTPUT_VISION TCPSENDER_OUTPUT_VISION
#define UDPSENDER_OUTPUT_THERMAL TCPSENDER_OUTPUT_THERMAL

#define UPGRADE_FILE	"/boot/Heat-Finder-update.tar.bz2"
#define UPGRADE_BIG_FILE	"/home/pi/Downloads/Heat-Finder-update.tar.bz2"
#define CONFIGURATION_FILE	"/home/pi/ade_camera_udp_out/adehf.ini"

//gpio send breath 3 time per min., and 'This app' will check 2 time per min..
#define WATCHDOG_GPIO_MAX_LOSE_COUNT (2)
#define WATCHDOG_GPIO_CHK_INTERVAL (30.0)

#ifndef THERMAL_OVERLAY_COL
#define THERMAL_OVERLAY_COL (12)
#endif

// Thermal 遮蔽区域的基准坐标
#define THERMAL_MASK_BASE_CX	40 // 必须可以被8整除
#define THERMAL_MASK_BASE_CY	40

typedef struct tagSendBuf
{
    axc_byte *pBuf;
    axc_i32 iSize;
    axc_ddword ddwChannelIndex;
    axc_dword dwRemoteIP;
    axc_word wRemotePort;
    axc_dword dwTimestamp;
    bool bIsIFrame;

    tagSendBuf()
    {
        pBuf = NULL;
        iSize = 0;
        ddwChannelIndex = 0;
        dwTimestamp =0;
        bIsIFrame =false;
    }
    ~tagSendBuf()
    {
    	//Avoid Error in `./adehf_mobile': double free or corruption (out): 0x7172b898
        //--if ((pBuf)&&(iSize>0))
        //--    delete[] pBuf;
        //iSize = 0;
        //ddwChannelIndex = 0;
        Release();
    }
    //hamlet
    //markhsieh
    void Release()
    {
        if ((pBuf!=NULL)&&(iSize>0)){
    		SAFEDELETE_ARRAY(pBuf);
    	}
        iSize = 0;
        ddwChannelIndex = 0;
        dwTimestamp =0;
        bIsIFrame =false;

        //pBuf = NULL;
    }
    void CopyData(axc_byte *pBuffer, const axc_i32 size, const axc_ddword channelIndex)//, axc_dword timestamp, bool isIFrame)
    {
        if ((pBuf!=NULL)&&(iSize>0)){
        	SAFEDELETE_ARRAY(pBuf);
        }
        pBuf = new axc_byte[size];
        memcpy(pBuf, pBuffer, size);
        iSize = size;
        ddwChannelIndex = channelIndex;
        dwTimestamp = 0.0; //timestamp;
        bIsIFrame = false; //isIFrame;
    }
    /*
    tagSendBuf& operator=(const tagSendBuf &obj)
    {
        this->CopyData(obj.pBuf, obj.iSize, obj.ddwChannelIndex);
        return *this;
    }
    */
}xSendBuf;

typedef struct
{
    unsigned int   CodecFourcc;	// codec name, 'H264' / 'LRV1' / 'MJPG'
    unsigned int   FrameLen;    // frame data bytes, not include this HEADER
    unsigned int   FrameSeq;	// frame seq number
    unsigned int   TimeHigh;	// UTC time
    unsigned int   TimeLow;		// milli-seconds
    unsigned short Width;		// video width
    unsigned short Height;		// video height
    unsigned short Crc16;		// CCITT CRC 16bit
    unsigned char  FrameType;	// 0 - keyfame, 1 - non keyframe
    unsigned char  HeaderLen;	// length of this HEADER
} T_ADE2CAM_FRAME_HEADER;

typedef struct _UDP_SESSION
{
    DWORD	dwRemoteIp;
    WORD	wRemotePort;
    WORD	wStreamMode; // TCP_SESSION_CONTEXT_OPEN_THERMAL or TCP_SESSION_CONTEXT_OPEN_VISION
    time_t	tLastPollingTime;
} T_UDP_SESSION;
//static T_UDP_SESSION		g_UdpSessions[MAX_UDP_SESSION_NUMBER] = {{0,0,0},};

//
// rect, point, size
//
typedef struct _RECT
{
    int left;
    int top;
    int right;
    int bottom;
} RECT;
typedef struct _POINT
{
    int x;
    int y;
} POINT;

#define MAX_THERMAL_HEATOBJ_NUM	(16)
typedef struct
{
    BOOL	bLeptonIsOnline; // lepton module is working ?
    WORD	wTempeMin; // temperature min value (0.01 Kelvin)
    WORD	wTempeMax; // temperature max value (0.01 Kelvin)
    WORD	wTempeAvg; // temperature avg value (0.01 Kelvin)
    WORD	wHeatObj;  // heat-object number
    struct	T_HEATOBJ
    {
        WORD	wTempe;
        POINT	ptPos;
        RECT	rcObj;
    } HeatObjList[MAX_THERMAL_HEATOBJ_NUM];
} T_THERMAL_IVS_OUTPUT;


// image size (thermal, vision-main, vision-sub)
typedef struct
{
    axc_dword cx;
    axc_dword cy;
} SIZE;

/**
typedef struct tagLeptonOverlay
{
    short LeptonW;
    short LeptonH;
    short LeptonL;
    short LeptonT;
    short LeptonR;
    short LeptonB;

    short CameraW;
    short CameraH;
    short CameraL;
    short CameraT;
    short CameraR;
    short CameraB;
}xLeptonOverlay;
**/

typedef struct _X_RPICAMERA_COMMAND{
	int id;
	//char* argv;
	char argv[32];
	bool isNum;
	bool isBooting;
}T_RPICAMERA_COMMAND;

typedef enum _E_RPICAMERA_PARAMETER
{
	RPICAMERA_PARAMETER_UNKNOWN = -1,
	RPICAMERA_PARAMETER_SHARPNESS = 0,
	RPICAMERA_PARAMETER_BRIGHTNESS = 1,
	RPICAMERA_PARAMETER_CONTRAST = 2,
	RPICAMERA_PARAMETER_SATURATION = 3,
	RPICAMERA_PARAMETER_EXPOSURE = 4,
	RPICAMERA_PARAMETER_AWB = 5,
	RPICAMERA_PARAMETER_AWBGAIN = 6,
	RPICAMERA_PARAMETER_IMAGEEFFECT = 7,
	RPICAMERA_PARAMETER_VERBOSE = 8,
	RPICAMERA_FIRST_NEED_RESTART_PARAMETER = 9,
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
	RPICAMERA_PARAMETER_VFLIP = 20,
	RPICAMERA_PARAMETER_HFLIP = 21,
	RPICAMERA_TOTAL_PARAMETERS =22,
} E_RPICAMERA_PARAMETER;

typedef enum _E_RPICAMERA_RUN_COMMAND_ID
{
	RPICAMERA_COMMAND_RUNKNOWN = 0x0000,
	RPICAMERA_COMMAND_SHARPNESS = 0x0001,
	RPICAMERA_COMMAND_BRIGHTNESS = 0x0002,
	RPICAMERA_COMMAND_CONTRAST = 0x0004,
	RPICAMERA_COMMAND_SATURATION = 0x0008,
	RPICAMERA_COMMAND_EXPOSURE = 0x0010,
	RPICAMERA_COMMAND_AWB = 0x0020,
	RPICAMERA_COMMAND_AWBGAIN = 0x0040,
	RPICAMERA_COMMAND_IMAGEEFFECT = 0x0080,
	RPICAMERA_COMMAND_VERBOSE = 0x0100,
} E_RPICAMERA_RUN_COMMAND_ID;
typedef enum _E_RPICAMERA_BOOT_COMMAND_ID
{
	RPICAMERA_COMMAND_BUNKNOWN = 0x0000,
	RPICAMERA_COMMAND_BITRATE = 0x0001,
	RPICAMERA_COMMAND_TIMEOUT = 0x0002,
	RPICAMERA_COMMAND_PREVIEWENC = 0x0004,
	RPICAMERA_COMMAND_SHOWSPSPPS = 0x0008,
	RPICAMERA_COMMAND_RESOLUTION_W = 0x0010,
	RPICAMERA_COMMAND_RESOLUTION_H = 0x0020,
	RPICAMERA_COMMAND_FPS = 0x0040,
	RPICAMERA_COMMAND_IPFRAMERATE = 0x0080,
	RPICAMERA_COMMAND_RAWCAPTURE = 0x0100,
	RPICAMERA_COMMAND_QP = 0x0200,
	RPICAMERA_COMMAND_H264PROFILE = 0x0400,
	RPICAMERA_COMMAND_VFLIP = 0x0800,
	RPICAMERA_COMMAND_HFLIP = 0x1000,
} E_RPICAMERA_BOOT_COMMAND_ID;

typedef enum _E_DEBUG_LEVEL
{
	LOG_DISABLE=0,
	LOG_VERBOSE=1,
	LOG_NORMAL=2,
	LOG_DEBUG=3,
	LOG_WARN=4,
	LOG_ERROR=5,
	LOG_FATAL=6,
	LOG_AUTO
} E_DEBUG_LEVEL;

class CHeatFinderUtility
{
  public:
    CHeatFinderUtility()
    {

    }
    static axc_bool string_replace(const char* szSrc, char* szDest, size_t nDestBufferSize, const char* szExisted, const char* szNew);
    static void MsgSendtoStrerror(int Strerr_id);

    // 从文本命令中找到 key，复制 value，并返回 value 的字符长度，不包括末尾的\0；
    // 如果 value 的实际长度大于dwValueOutBufferSize，value会被截断复制到szValueOut，但返回值会是value的实际长度，而不是 szValueOut 的字符长度
    // 返回-1表示失败
    static axc_i32 CvmsHelper_Cmd_GetValue(const char* szCmdText, const axc_dword dwCmdTextLen, const char* szCmdKey, char* szValueOut, axc_dword dwValueOutBufferSize);

    // 在网络发来的命令包中找到"文本"的结尾，把命令包拆分成 "文本" 和 "附加数据" 两段
    static axc_bool CvmsHelper_Cmd_UnpackToTextAndExt(void* pInData, const int iInSize, const int iInTextSize, char** ppszText, int* piTextLen, void** ppExtData, int* piExtDataLen);

    // 设置文本命令中某一个key的值
    // 如果key已经存在，更新它的取值为szNewValue；如果key不存在，把它添加在文本命令串的末尾
    // 失败返回-1；buffer大小不够，返回-2；成功返回key的偏移位置
    static axc_i32 CvmsHelper_Cmd_SetValue(char* szCmdText, axc_dword dwCmdTextLen, const axc_dword dwCmdTextBufferSize, const char* szCmdKey, const char* szNewValue);

    // 从一行文字中，找到指定的分隔符号（比如;），用于解析行内多选项文字，比如从文字 "value1;value2;value3" 中解析出 "value1","value2","value3"
    // 返回值为szSubValueOut的字符串大小，不包括末尾的\0结束符；返回-1表示失败
    // ppszNext用于返回下一个SubValue的开始位置
    static axc_i32 CvmsHelpder_Cmd_GetValueSub(const char* szValueListText, const char chSep, char* szSubValueOut, const axc_dword dwSubValueOutSize, const char** ppszNext);

    static axc_bool PushFrameToTxFifo(CAxcFifo* fifo, const void* data, int len, axc_byte channel_index, int width, int height, axc_dword dwPts, axc_dword dwFrameSeq, axc_bool bIsKey);
    static int Utility_ChkRaspivid_strerr(int *count);
    static void SetActiveApp(void *pActiveApp);
    static void* GetActiveApp();
    static void SetADECameraSDK(void *pADECameraSDK);
    static void *GetADECameraSDK();
    /**
     * mark.hsieh ++
     */
    static void SetSystemSignalListener(void *pSystemSignalListener);
    static void *GetSystemSignalListener();
    static void SetGlobalsupport(void *pGlobalsupport);
    static void *GetGlobalsupport();

    static void SetTwinkleLedShowNetworkUsed();
    static void SetTwinkleLedShowStatusOK();
    static void SetTwinkleLedShowStatusWrong();
    static void SetTwinkleLedShowHeatAlarm();

    static void SetGPIORebootStep();
    static void SetGPIOSrvRestart();

    static void RecvGPIOSrvResponse();
    static void SendGPIOSrvChkNotify();
    static bool IsGPIOSrvAlive();
    static void SetGPIOSrvChkRecordInit();

    static void SetTimerService(void *m_pTimerSerivce);
    static void *GetTimerService();

    static void CoordinateConvertRect(const RECT rcSrc, const int iSrcWidth, const int iSrcHeight, RECT* prcDest, const int iDestWidth, const int iDestHeight, const int iMode);

    static double AdjustTemperatureValue(double dbAvgTempe, int iMode, double dbTempeAdjust);
    static BOOL TemperatureToText(double dbValue, int iMode, char* szOutText);
    static BOOL TextToTemperature(const char* szText, double* pdbValue, int* piMode);

    static void SetConfigObj(void *pConfigContext);
    static void *GetConfigObj();

    static void SetLeptonSDK(void *pLeptonSDK);
    static void *GetLeptonSDK();

    static void PrintNowData(short iStdout);

    static axc_dword GetTickCount();  //gavin ++
    static int GetCameraConfigParameter(const char* lpcszKey, char* szResult, size_t dwLen);
protected:
    static void *m_pActiveApp;
    static void *m_pADECameraSDK;
    /**
     * mark.hsieh ++
     */
    static void *m_pSystemSignalListener;
    static void *m_pGlobalsupport;
    static void *m_pTimerSerivce;
    static void *m_pConfigContext;
    static void *m_pLeptonSDK;
};

typedef struct
{
    char name[64];
    unsigned short count;
} pThreadRecord;

class GlobalDef
{
public:
    GlobalDef();
    ~GlobalDef();

    void OnCollectThermalMsgToString(CAxcString& cszSend);
    bool OnRenewThermalResolution(const unsigned int Width, const unsigned int Height, const unsigned int DataSize);
    bool OnRenewVisionResolution(const unsigned int Width, const unsigned int Height);
    bool OnRenewVisionSubResolution(const unsigned int Width, const unsigned int Height);

    void AddpThreadRecord(const char* pName);
    void OnPrintfpThreadRecord();
    unsigned int GetpThreadRecordNumber();

    /**
     * push data to queue, and auto segment.
     *
     * const void* pData, int iData_length : data input
     * ----> (unsigned short *) data, or (axc_byte *) data
     *
     * unsigned short uChannel_index
     * ----> unsigned short uChannel_index: ADE2CAM_CHANNEL_LEPTONRAW, ADE2CAM_CHANNEL_VISUALMAIN, ADE2CAM_CHANNEL_VISUALSUB
     *
     * int iWidth, int iHeight : the newest size of picture
     *
     * double dfLastFrameTimeStamp_MS : this frame source utc time stamp (m-sceond)
     * ----> dfLastFrameTimeStamp_MS: CAxcTime::GetCurrentUTCTimeMs()*1000
     *
     * unsigned int uiFrameOrderofSequence : this frame source data index of one view
     *
     * bool bIsKeyframe: is I-frame or not?
     */
    bool PushFrameToNetDataQueue(const void* pData, int iData_length, unsigned short uChannel_index, int iWidth, int iHeight, double dfLastFrameTimeStamp_MS, unsigned int uiFrameOrderofSequence, bool bIsKeyframe);
    CAxcFifo* GetNetDataQueue();
    CAxcFifo* GetNetUDPDataQueue();

    bool SetEmissivity(float fValue);
    float GetEmissivity();

    T_THERMAL_IVS_OUTPUT* GetThermalIvsOutSavingTable();
    bool GetLeptonIsOnline();
    void SetLeptonOnlineState(bool bValue);
    void SetLeptonIvsOutRecordReInitinal();
    void SetLeptonTempValue(short iMax, short iMin, short iAvg);
    void SetLeptonTempObjInfo(unsigned short iIndex, short iTemp, POINT ptObj, RECT rcObj);
    void SetLeptonHeatObjCount(unsigned short iCount);
    unsigned short GetLeptonHeatObjCount();

    /**
     * return:
     * -1: configuration not exist, 0: close/not collect heat object, 1: turn on/collecting heat objects
     */
    int	RenewThermalCurrentConfiguration();
    int GetThermalCurrentHeatObjSeekStatus();
    int	GetThermalCurrentFoundHeatObj();

    void HealGPIOBody();
    void AttackGPIOBody();
    bool ChkIsGPIOSrvAlive();
    void HealGPIO2Breathe();

    void SetVisioLostRestartStatus(bool bIsRetartNow);
    bool GetVisioLostRestartStatus();

    bool GetVisioResolution(unsigned int &w, unsigned int &h);
    bool GetThermalResolution(unsigned int &w, unsigned int &h, unsigned int &size);
    int	GetOverlaybyResolution(unsigned int Width, unsigned int Height, xLeptonOverlay &xLol);

    CAxcFifo* GetDataQueueAddress();
    bool ChkIsDataQueueEmpty();
    CAxcFifo* GetUDPDataQueueAddress();

    void SetTcpPollingCount(unsigned int iValue);
    bool ChkIsTcpPollingCountEmpty();

protected:
    SIZE m_sizeVisionMainImage;
    SIZE m_sizeVisionSubImage;
    SIZE m_sizeThermalImage;
    std::mutex m_locker;

    T_THERMAL_IVS_OUTPUT m_ThermalIvsOut;
    std::mutex m_thermalmsg_locker;

    std::vector<pThreadRecord> m_pthreadRecordList;
    std::mutex m_addpThreadRecord_locker;

    unsigned int CountpThreadRecordNumber(bool bisPrint);

    CAxcFifo m_queueNetDataForSend;
    bool	m_bQueueIsValid;
    float	m_aptm_Emissivity;
    int 	m_bChkGpioBreathe;
    bool	m_bThermalHeatObj_Enable;

    bool OnReadADEHeatFinderConfigurationFile();

    bool m_isVisioLostRestart;
    void OnChangeFlagVisioLostAfterChange(bool bIsRetartNow);

    int m_iFifoTotalBytes;
    int m_iFifoPacketSize;

    unsigned int m_iThermalDataSize;
    CAxcFifo m_queueNetUDPDataForSend;

    unsigned int m_iTcppollingCount;
};

#endif // GLOBALDEF_H
