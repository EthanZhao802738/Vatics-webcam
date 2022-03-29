#ifndef HEATFINDERAPP_H
#define HEATFINDERAPP_H

#include "globaldef.h"
#include "ade_camera_sdk.h"
#include <mutex>
using namespace std;

#include "heatfinderleptoncapture.h"

/**
 * mark.hsieh ++
 */
#include "heatfindersystemsignalmanager.h"
#include "heatfinderbroadcast.h"
#include "heatfindertcplistener.h"
#include "heatfiniderudpcommand.h"
#include "heatfinderudpmediaclient.h"
#include "heatfinderudpsend.h"
#include "heatfindervisioncapture.h"
#include "heatfindertimer.h"

//#include "heatfindervisionRawcapture.h"
#include "heatfinderconfigmanager.h"
#include "adeSDKAPIleptonthermal.h"

#include "Service/module_modbus.h"

#include "adeWatchdog.h"
#include "FpsHelper.h" //gavin ++

/**
 * @brief main Heatfinder App class
 * 最主要的Class
 */
class CHeatFinderApp
{
public:
    CHeatFinderApp();
    ~CHeatFinderApp();

    void make_thermal_ivs_result_string(CAxcString& cszSend);
    int exec();

private:
    //global setting
    SIZE m_sizeVisionMainImage;
    SIZE m_sizeVisionSubImage;
    SIZE m_sizeThermalImage;

    bool m_bStopService;
    std::mutex m_locker;

    //T_THERMAL_IVS_OUTPUT m_ThermalIvsOut;

    //ade_camera_sdk m_adeCameraSDK;

    CHeatFinderLeptonCapture m_leptonCapture;

    /**
     * mark.hsieh ++
     */
    CHeatFinderSystemSignalManager m_systemSignalManager;
    CHeatFinderBroadcast m_broadcastInformationCollector;
    CHeatFinderTcpListener m_tcpListener;
    //CHeatFinderTcpClient m_tcpLiveviewSender;
    CHeatFiniderUdpCommand m_udpCommandReceiver;
    GlobalDef m_global;
    CHeatFinderUdpMediaClient m_udpMediaClientSender;
    CHeatFinderUdpSend m_udpViewSender;
    CHeatFinderVisionCapture m_visionCapture;
    CHeatFinderTimer m_timer;

    Cheatfinderconfigmanager m_ConfigManager;
    //CadeSDKAPIleptonthermal mLeptonSDK;
    //CHeatfindervisionRawcapture m_visionRawCapture;

    CPluginManager m_pluginManager;     //hamlet

    CADEWatchDog m_watchdog;
    unsigned int	m_iHeartBeastLoseTime;

    bool m_bIsReady2Reboot;
    bool m_bIsGpioSetReboot;
    int m_ireSend;

    int m_iIsStatusFineCount;
    float m_fLastBadStatusTime;

    unsigned int m_iDoubleCheck;

protected:
    //thermal
    static void OnLedStatusTwinkleNotify(void *pContext, const bool value)
    {
        //reinterpret_cast: 將一種型態的指標轉換為另一種型態的指標
        CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
        return pSender->OnLedStatusTwinkleNotify(value);
    }
    void OnLedStatusTwinkleNotify(const bool value);

    static void OnThermalSizeChangedNotify(void *pContext, const unsigned int Width, const unsigned int Height)
    {
        CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
        return pSender->OnThermalSizeChangedNotify(Width, Height);
    }
    void OnThermalSizeChangedNotify(const unsigned int Width, const unsigned int Height);

    static void OnThermalDataReceivedEvent_func(void *pContext, axc_byte *pBuf, const axc_i32 size, const axc_ddword channelIndex)
    {
        CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
        return pSender->OnThermalDataReceivedEvent_func(pBuf, size, channelIndex);
    }
    void OnThermalDataReceivedEvent_func(axc_byte *pBuf, const axc_i32 size, const axc_ddword channelIndex);

    static void OnThermalDataReceived2PluginEvent(void *pContext, axc_byte *pBuf, const axc_i32 size, const axc_ddword channelIndex)
    {
        CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
        return pSender->OnThermalDataReceived2PluginEvent(pBuf, size, channelIndex);
    }
    void OnThermalDataReceived2PluginEvent(axc_byte *pBuf, const axc_i32 size, const axc_ddword channelIndex);

    static void OnThermalDataPopedEvent_func(void *pContext, axc_byte *pBuf, const axc_i32 size){
    	CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
    	return pSender->OnThermalDataPopedEvent_func(pBuf, size);
    }
    void OnThermalDataPopedEvent_func(axc_byte *pBuf, const axc_i32 size);

    static void OnReceivedThermalRestartFinishNotify(void *pContext)
    {
        CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
        return pSender->OnReceivedThermalRestartFinishNotify();
    }
    void OnReceivedThermalRestartFinishNotify();

    static void OnVisionDataPopedEvent_func(void *pContext, axc_byte *pBuf, const axc_i32 size){
    	CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
    	return pSender->OnVisionDataPopedEvent_func(pBuf, size);
    }
    void OnVisionDataPopedEvent_func(axc_byte *pBuf, const axc_i32 size);

    static void OnVisionDataReceived2WhoEvent(void *pContext, axc_byte *pBuf, const axc_i32 size, const unsigned short channelIndex)
    {
        CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
        return pSender->OnVisionDataReceived2WhoEvent(pBuf, size, channelIndex);
    }
    void OnVisionDataReceived2WhoEvent(axc_byte *pBuf, const axc_i32 size, const unsigned short channelIndex);

    static void OnVisionSizeChangedNotify(void *pContext, const unsigned int Width, const unsigned int Height)
    {
        CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
        return pSender->OnVisionSizeChangedNotify(Width, Height);
    }
    void OnVisionSizeChangedNotify(const unsigned int Width, const unsigned int Height);

    static void OnReceivedVisionRestartFinishNotify(void *pContext)
    {
        CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
        return pSender->OnReceivedVisionRestartFinishNotify();
    }
    void OnReceivedVisionRestartFinishNotify();

    //system signal manager
    static void OnAPPStatusChangeNotify_FUNCTION(void *pContext, const unsigned int flag, const unsigned int action, const unsigned int time_interval)
    {
        //reinterpret_cast: 將一種型態的指標轉換為另一種型態的指標
    	CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
        return pSender->OnAPPStatusChangeNotify_FUNCTION(flag, action, time_interval);
    }
    void OnAPPStatusChangeNotify_FUNCTION(const unsigned int flag, const unsigned int action, const unsigned int time_interval);

    static void OnCaptureStatusChangeNotify_FUNCTION(void *pContext, const unsigned int flag, const unsigned int action, const unsigned int time_interval)
    {
    	CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
        return pSender->OnCaptureStatusChangeNotify_FUNCTION(flag, action, time_interval);
    }
    void OnCaptureStatusChangeNotify_FUNCTION(const unsigned int flag, const unsigned int action, const unsigned int time_interval);

    static void OnTcpBroadcastRenewNotifyEvent_func(void *pContext){
    	CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
    	return pSender->OnTcpBroadcastRenewNotifyEvent_func();
    }
    void OnTcpBroadcastRenewNotifyEvent_func();

    static void OnTcpAcceptSessionEvent_func(void *pContext, CAxcSocketTcpSession *pValue, const unsigned int luValue){
    	CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
    	return pSender->OnTcpAcceptSessionEvent_func(pValue, luValue);
    }
    void OnTcpAcceptSessionEvent_func(CAxcSocketTcpSession *pValue, const unsigned int luValue);

    static void OnBroadcastInfoChangedNotify_func(void *pContext, CAxcString value){
    	CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
    	return pSender->OnBroadcastInfoChangedNotify_func(value);
    }
    void OnBroadcastInfoChangedNotify_func(CAxcString value);

    static void OnUdpBroadcastRenewNotifyEvent_func(void *pContext){
    	CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
    	return pSender->OnUdpBroadcastRenewNotifyEvent_func();
    }
    void OnUdpBroadcastRenewNotifyEvent_func();

    static void OnTcpSendedEvent_func(void *pContext){
    	CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
    	return pSender->OnTcpSendedEvent_func();
    }
    void OnTcpSendedEvent_func();

    static void OnTcpSessionRemovingEvent_func(void *pContext, CAxcSocketTcpSession *pValue){
    	CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
    	return pSender->OnTcpSessionRemovingEvent_func(pValue);
    }
    void OnTcpSessionRemovingEvent_func(CAxcSocketTcpSession *pValue);

    static void OnTcpNeedtoRemoveSessionEvent_func(void *pContext, CAxcSocketTcpSession *pValue){
    	CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
    	return pSender->OnTcpNeedtoRemoveSessionEvent_func(pValue);
    }
    void OnTcpNeedtoRemoveSessionEvent_func(CAxcSocketTcpSession *pValue);

    static int OnSessionSendtofunc_func (void *pContext, CAxcList *listSessionSend, CAxcList *listSessionDrop){
    	CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
    	return pSender->OnSessionSendtofunc_func(listSessionSend, listSessionDrop);
    }
    int OnSessionSendtofunc_func (CAxcList *listSessionSend, CAxcList *listSessionDrop);

    static int OnReceivedUpgradeEvent_func (void *pContext, double dDelaytimestampMS){
    	CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
    	return pSender->OnReceivedUpgradeEvent_func (dDelaytimestampMS);
    }
    int OnReceivedUpgradeEvent_func (double dDelaytimestampMS);

    static void OnFuncChkADEGPIOAliveNotify(void *pContext, const double dCurrentTimeMS, const unsigned int uiAction, const signed int siValue){
    	CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
    	return pSender->OnFuncChkADEGPIOAliveNotify(dCurrentTimeMS, uiAction, siValue);
    }
    void OnFuncChkADEGPIOAliveNotify(const double dCurrentTimeMS, const unsigned int uiAction, const signed int siValue);

    static void OnFuncChkThermalIvsInfoNotify(void *pContext, const double dCurrentTimeMS, const unsigned int uiAction, const signed int siValue){
    	CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
    	return pSender->OnFuncChkThermalIvsInfoNotify(dCurrentTimeMS, uiAction, siValue);
    }
    void OnFuncChkThermalIvsInfoNotify(const double dCurrentTimeMS, const unsigned int uiAction, const signed int siValue);

    /**
    static void OnFuncChkRaspividNotify(void *pContext, const double dCurrentTimeMS, const unsigned int uiAction, const signed int siValue){
    	CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
    	return pSender->OnFuncChkRaspividNotify(dCurrentTimeMS, uiAction, siValue);
    }
    void OnFuncChkRaspividNotify(const double dCurrentTimeMS, const unsigned int uiAction, const signed int siValue);
    **/

    static void OnFuncChkLedStatusNotify(void *pContext, const double dCurrentTimeMS, const unsigned int uiAction, const signed int siValue){
    	CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
    	return pSender->OnFuncChkLedStatusNotify(dCurrentTimeMS, uiAction, siValue);
    }
    void OnFuncChkLedStatusNotify(const double dCurrentTimeMS, const unsigned int uiAction, const signed int siValue);

    static void OnFuncChkModbusDNotify(void *pContext, const double dCurrentTimeMS, const unsigned int uiAction, const signed int siValue){
    	CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
    	return pSender->OnFuncChkModbusDNotify(dCurrentTimeMS, uiAction, siValue);
    }
    void OnFuncChkModbusDNotify(const double dCurrentTimeMS, const unsigned int uiAction, const signed int siValue);

    static void OnFuncChkENotify(void *pContext, const double dCurrentTimeMS, const unsigned int uiAction, const signed int siValue){
    	CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
    	return pSender->OnFuncChkENotify(dCurrentTimeMS, uiAction, siValue);
    }
    void OnFuncChkENotify(const double dCurrentTimeMS, const unsigned int uiAction, const signed int siValue);

    static int OnReceivedCameraRawFrame_func(void *pContext, const int Width, const int Height, const int RawFMT, const int size, unsigned char *pRGB){
    	CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
    	int iRst = pSender->OnReceivedCameraRawFrame_func(Width, Height, RawFMT, size, pRGB);
    	return iRst;
    }
    int OnReceivedCameraRawFrame_func(const int Width, const int Height, const int RawFMT, const int size, unsigned char *pRGB);

    static int OnSetThermalOverlay_func (void *pContext , xLeptonOverlay &xLoi){
    	CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
    	int iRst = pSender->OnSetThermalOverlay_func(xLoi);
    	return iRst;
    }
    int OnSetThermalOverlay_func(xLeptonOverlay &xLoi);

    static void OnProcessDeadNotifyEvent_func(void *pContext, char *szStr, int isize, double losttime){
    	CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
    	return pSender->OnProcessDeadNotifyEvent_func(szStr, isize, losttime);
    }
    void OnProcessDeadNotifyEvent_func(char *szStr, int isize, double losttime);

    static void OnWatchDogStopNotiyEvent_func(void *pContext){
    	CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
    	return pSender->OnWatchDogStopNotiyEvent_func();
    }
    void OnWatchDogStopNotiyEvent_func();

    static void OnThermalHeatObjectReceivedEvent_func(void *pContext, xPluginHeatObject *pHeatObjs){
    	CHeatFinderApp *pSender = reinterpret_cast<CHeatFinderApp*> (pContext);
    	return pSender->OnThermalHeatObjectReceivedEvent_func(pHeatObjs);
    }
    void OnThermalHeatObjectReceivedEvent_func(xPluginHeatObject *pHeatObjs);
};

#endif // HEATFINDERAPP_H
