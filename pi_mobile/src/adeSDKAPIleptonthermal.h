/*
 * adeSDKAPIleptonthermal.h
 *
 *  Created on: Feb 1, 2018
 *      Author: markhsieh
 */

#ifndef ADESDKAPILEPTONTHERMAL_H_
#define ADESDKAPILEPTONTHERMAL_H_

#include "globaldef.h"
#include "ADEThermal_SDK.h"
#include <mutex>
#include <vector>

#define THERMAL_AVG_LIST_SIZE	(1*60*9) // 1 minute @ 9FPS
#define THERMAL_SPI_RW_THREAD_PRIORITY		(13) /* spi-thread priority: 0 ~ 15, default 0, 15 highest */
#define LEPTON_SDK_CPUBIND_ZERO		(0)
#define LEPTON_SDK_CPUBIND_ONE		(1)
#define LEPTON_SDK_CPUBIND_TWO		(2)
#define LEPTON_SDK_CPUBIND_THREE	(3)
#define LEPTON_SDK_CPUBIND_AUTO		(-1)


#define LEPTON_SDK_CPUBIND_DEFAULT	LEPTON_SDK_CPUBIND_THREE
#define LEPTON_SDK_DEFAULT_FAIL_INTERVAL	(10.0)
#define THERMAL_FRAME_LOSE_RELOAD_TIME	(2*60)  // 2 min.
#define THERMAL_TIMEOUT_LIMITATION	(3 * THERMAL_FRAME_LOSE_RELOAD_TIME)

typedef enum _E_THERMAL_COMMAND_MODE
{
	CMD_RAD_ENABLE=0,
	CMD_RAD_DISABLE,
	CMD_FFC,
	CMD_REBOOT,
	CMD_EMISSIVITY,
	CMD_SETCPUBIND,
	CMD_SDK_CONTENT,
	CMD_GETEMI,
	CMD_RESTART,
	CMD_T_LAST,
} E_THERMAL_COMMAND_MODE;

typedef struct _Emissity
{
	float			fEmissivity; //max. value range:(0~1)
	double			dLastUpdateTime;
}T_Emissity;

typedef void (*OnDataSendNotifyEvent)(void *pContext, axc_byte *pBuf, const axc_i32 size, T_ADETHERMAL_RAWDATA_FRAME *raw);
typedef struct tagDataSendNotifyEventHandler
{
	OnDataSendNotifyEvent fnEvent;
    void *pContext;
}xDataSendNotifyEventHandler;

typedef void (*OnHeatObjectSendNotifyEvent)(void *pContext, xPluginHeatObject *pPluginHeatObj);
typedef struct tagHeatObjectSendNotifyEventHandler
{
	OnHeatObjectSendNotifyEvent fnEvent;
    void *pContext;
}xHeatObjectSendNotifyEventHandler;

typedef void (*OnReceivedThermalRestartFinishNotifyEvent)(void *pContext);
typedef struct tagReceivedThermalRestartFinishNotifyEventHandler
{
	OnReceivedThermalRestartFinishNotifyEvent fnEvent;
    void *pContext;
}xReceivedThermalRestartFinishNotifyEventHandler;

class CadeSDKAPIleptonthermal
{
public:
	CadeSDKAPIleptonthermal();
	~CadeSDKAPIleptonthermal();

	bool run();
	void stop();
	bool command_reader();
	int	 getThermalIvsHeatObjCount();
	float getThermalEmissivity();
	bool setThermalEmissivity(float fValue);
	bool clickedThermalRADEnable();
	bool clickedThermalRADDisable();
	bool clickedThermalFFC();
	bool clickedThermalCPUBind(int iCPUIndex);
	/**
	 * only reopen by SDK
	 */
	bool clickedThermalREBOOT();
	/**
	 * try to destroy running sdk process, then restart ...
	 */
	bool clickedThermalRESTART();
	void AddDataSendNotifyEvent(OnDataSendNotifyEvent fnEvent, void *pContext);
	int getThermalCurrentParameterValue(const char* ptype_name, unsigned short (*parameter)[12], uint16_t row_num);

	bool ProcessUpdateThermalIvsOnce();
	bool ProcessLeptonReadRawDataOnce();
	double getLastFrameMs();

	bool setIntervalOfIdleStatusAnalyzeHeatObject(double fInterval);
	void AddHeatObjectSendNotifyEvent(OnHeatObjectSendNotifyEvent fnEvent, void *pContext);

	bool setIntervalValue(double fInterval);
	void AddReceivedThermalRestartFinishNotifyEvent(OnReceivedThermalRestartFinishNotifyEvent fnEvent, void *pContext);

private:
	CAxcMemory		m_caxcmemThermalCompress;
	CAxcMemory		m_caxcmemThermalIvsRawDataAnalyze;
	CAxcFifoBufferPtr	m_caxcfifobufferThermalIvs;

	bool			m_bStopAllThread;
	//CAxcThread		m_threadUpdateThermalIvs;
	//CAxcThread		m_threadReadLeptonRawData;

	CAxcFifo*		m_pClientfifoNetTcpSend;
	CAxcFifo*		m_pClientfifoNetUdpSend;

	uint8_t 		m_byLogLevel; // show verbose-log
	int				m_iCountNoData;

	T_Emissity		m_fEmissivity;
	uint16_t		m_iSceneEmissivity;
	double			m_fLastFrameSecondTime;
	bool			m_bPauseReading;
	bool			m_bInitReady;

	//unsigned int 	m_iWidth, m_iHeight;

	std::vector<xDataSendNotifyEventHandler> m_DataSendNotifyEventList;
	std::mutex		m_locker;

	// manual to analysis how many heat objects, now.
	bool			m_bStartAnalyzeHeatObjectOnce;
	double			m_fLastTimeAnalyzeHeatObject;
	double			m_fIntervalOfAnalyzeHeatObject;

	std::vector<xHeatObjectSendNotifyEventHandler> m_HeatObjectSendNotifyEventList;
	std::mutex		m_locker_heatobj;
	xPluginHeatObject	m_PlugInHeatObjects;

	int				m_iCPUBindNumber;
	
	double			m_interval;

	int             m_iOpenIniNumber;

	std::vector<xReceivedThermalRestartFinishNotifyEventHandler> m_ReceivedThermalRestartFinishNotifyEventList;
	std::mutex		m_locker_restart_notify;

protected:
	bool get_thermal_value_range(short* pwRawData, short* pwMin, short* pwMax, short* pwAvg, POINT* pptMin, POINT* pptMax);
	static void sdk_log_callback(uint8_t level, const char* log);
	int sdk_error_reader(E_ADETHERMAL_RESULT enumErrno, T_ADETHERMAL_CONFIG *pConfig);

	float getLeptonEmissivity();
	bool OnParserThermalCommand(E_THERMAL_COMMAND_MODE CMD_MODE, int iValue, bool bValue);
	E_ADETHERMAL_RESULT OnRestartLepton();

	void fireDataSendNotify(axc_byte* pData, axc_dword dwDataSize, T_ADETHERMAL_RAWDATA_FRAME *raw);
	int32_t GetOverlayParameter(uint16_t ptype, unsigned short (*parameter)[12], uint16_t row_num);

	double getCurrentTime();

	// manager
	void onReceivedLeptonTempObjInfo(unsigned short iIndex, short iTemp, POINT ptObj, RECT rcObj);
	void onReceivedLeptonTempObjCount(unsigned short iCount);
	void fireHeatObjectSendNotify();

	void fireReceivedThermalRestartFinishNotify();
};


#endif /* ADESDKAPILEPTONTHERMAL_H_ */
