#ifndef HEATFINDERBROADCAST_H
#define HEATFINDERBROADCAST_H

#include "globaldef.h"
#include "ADEThermal_SDK.h"
#include <mutex>
#include <vector>
#include <semaphore.h>

#define FW_Version_Length           20
#define Dev_Default_Name_Length     30
#define Dev_Type_Length             20
#define Lang_Type_Length            3

typedef void (*OnBroadcastInfoChangedNotify)(void *pContext, CAxcString value);
typedef struct tagBroadcastInfoChangedNotifyHandler
{
    OnBroadcastInfoChangedNotify fnEvent;
    void *pContext;
}xBroadcastInfoChangedNotifyHandler;

// event
#define BroadcastValueRenew_Type_Vision (0)
#define BroadcastValueRenew_Type_Thermal (1)
#define BroadcastValueRenew_Type_Renew (2)
#define BroadcastValueRenew_Action_Resize (0)
/**
 * broadcast: event --- receive
 * 		type: 0:Vision, 1:Thermal, 2:Renew
 * 		action: 0:Renew Size ...
 * 		x,y: ... x,y
 */
struct BroadcastValueRenewHandler{
	unsigned short type;
	unsigned short action;
	unsigned int w;
	unsigned int h;
};
typedef struct tagReceiveBroadcastRenewNotifyHandler
{
    void *pContext;
    struct BroadcastValueRenewHandler objEvent;
}xReceiveBroadcastRenewNotifyHandler;

class CHeatFinderBroadcast
{
public:
    CHeatFinderBroadcast();
    ~CHeatFinderBroadcast();

    bool Run();
    void Stop();

    void SetfwVersion(const char *fw_version);
    void Setdevdefaultname(const char *default_name);
    void SetdevType(const char *devtype);
    void SetSocketUDPEnalbeStatus(const bool value);

    void AddBroadcastInfoChangedNotifyEvent(OnBroadcastInfoChangedNotify fnEvent, void *pContext);
    void AddBroadcastInfoRenewNotifyEvent(BroadcastValueRenewHandler objEvent, void *pContext);
    void updatePluginInfomations(char *szString, unsigned int iLength);

    static void OnThermalSizeChangedNotify(void *pContext, const unsigned int cx, const unsigned cy)
    {
        CHeatFinderBroadcast *pSender = reinterpret_cast<CHeatFinderBroadcast*> (pContext);
        pSender->OnThermalSizeChangedNotify(cx, cy);
    }

    static void OnVisionSizeChangedNotify(void *pContext, const unsigned int cx, const unsigned cy)
    {
        CHeatFinderBroadcast *pSender = reinterpret_cast<CHeatFinderBroadcast*> (pContext);
        pSender->OnVisionSizeChangedNotify(cx, cy);
    }
    char* getAppBuildTime() { return m_szAppBuildTime; }
protected:
    std::mutex m_locker;
    std::mutex m_PluginInformation_locker;

    CAxcThread m_caxcThread;       //m_threadTimerChecker
    bool m_bStopThread;         //stop thread flag

    char m_szFW_Version[FW_Version_Length];
    char m_szDev_Default_Name[Dev_Default_Name_Length];
    char m_szDev_Type[Dev_Type_Length];
    bool m_bSocketUDPEnalbeStatus;
    //----building time
    char m_szAppBuildTime[128];

    std::vector<xBroadcastInfoChangedNotifyHandler> m_BroadcastInfoNotifyList;

    std::vector<xReceiveBroadcastRenewNotifyHandler> m_ReceivedBroadcastRenewNotifyList;

    //----total info.
    CAxcString m_cszBroadcastInfo;
    //----mac
    axc_byte m_abyMacAddress_eth0[6];
    axc_byte m_abyMacAddress_wlan0[6];

    //----ip
    CAxcString m_cszETH0IP;
    CAxcString m_cszWLAN0IP;

    //----stable information
    CAxcString m_cszManualName;
    CAxcString m_cszLessName;
    CAxcString m_cszManualPID;
    CAxcString m_cszRemoteCtlSerVersion;
    CAxcString m_cszTotalVersion;
    CAxcString m_cszPluginsVersion;

    // mac-address string
    CAxcString m_cszEthZeroDividedMac;
    CAxcString m_cszEthZeroSequentialMac;
    CAxcString m_cszWlanZeroSequentialMac;

    // monitor control
    sem_t requestOutput;

    SIZE m_sizeThermalImage;
    SIZE m_sizeVisionMainImage;
    void UpdateBroadcastInfo();
    void UpdateNetInfo();
    int CheckLink(const char *ifname);
    bool GetLangChar(char *lang, uint32_t len);
    bool IsUpLink(int if_flags);
    bool IsRunningLink(int if_flags);

    //----Unchangeable information
    void getStableSystemInformation();

    //----Need Use I/O RW
    void getCurrentSystemInformation();

    void OnThermalSizeChangedNotify(const unsigned int cx, const unsigned cy);
    void OnVisionSizeChangedNotify(const unsigned int cx, const unsigned cy);
    void OnGeneralInformationUpdateNotify(/**const bool isLednotifyevent**/);
    void fireBroadcastNotify(CAxcString &BroadcastInfo);

    /**
     * mark.hsieh ++
     * 		thread >> Callback 多載
     *
     */
    static axc_dword thread_process(CAxcThread* pThread, void* pContext)
    {
    	CHeatFinderBroadcast *pSender = reinterpret_cast<CHeatFinderBroadcast*> (pContext);
        return pSender->thread_process();
    }
    axc_dword thread_process();
};

#endif // HEATFINDERBROADCAST_H
