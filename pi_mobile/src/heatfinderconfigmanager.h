/*
 * heatfinderconfigmanager.h
 *
 *  Created on: Feb 2, 2018
 *      Author: markhsieh
 */

#ifndef HEATFINDERCONFIGMANAGER_H_
#define HEATFINDERCONFIGMANAGER_H_

#include "globaldef.h"
#include "axclib.h"
using namespace std;

// Thermal 遮蔽区域的基准坐标
#ifndef THERMAL_MASK_BASE_CX
#define THERMAL_MASK_BASE_CX	40 // 必须可以被8整除
#endif
#ifndef THERMAL_MASK_BASE_CY
#define THERMAL_MASK_BASE_CY	40
#endif

typedef enum _E_OPEN_STREAM_ID
{
	OPEN_NONE_STREAM = 0x0000,
	OPEN_CAMERA_STREAM = 0x0001,
	OPEN_THERMAL_STREAM = 0x0002
} E_OPEN_STREAM_ID;

typedef struct _T_CONFIG_FILE_OPTIONS
{
	//
	// Log
	//
	axc_dword	dwLogLevel; // 0~6 (AXC_LOG_LEVEL_DISABLE ~ AXC_LOG_LEVEL_FATAL)

	//
	// Lepton Thermal Module
	//
	axc_bool	bThermalEnable;
	char		szThermalI2cDevice[MAX_PATH];
	char		szThermalSpiDevice[MAX_PATH];
	char		szThermalUartDevice[MAX_PATH];  //Hank
	axc_dword	dwThermalSpiSpeed;
	axc_dword	dwThermalSpiMode;
	axc_dword	dwThermalSpiBits;
	axc_dword	dwEmissivity;		//emissivity (0.0*1000 <--> 1.0*1000)
	axc_bool	bThermalSpiSwapWord;
	axc_bool	bThermalSpiSwapByte;
	uint8_t 	ucThermalOptional;  //mark.hsieh 2017-10-25 ++ (thermal sdk >= v2.55_PI)
	//
	// Vision Camera
	//
	axc_bool	bVisionEnable;
	char		szVisionCaptureApp[MAX_PATH];
	axc_bool	bNeedRestart;
	axc_bool	bVisionParameterChanged;
	unsigned int 	iChangedCommand_Runtime;
	unsigned int 	iChangedCommand_Boottime;
	//
	// GPIO pin index : <0 disable, >=0 gpio pin index
	//
	axc_i32		iGpioPin_LED_Status;
	axc_i32		iGpioPin_LED_Network;
	axc_i32		iGpioPin_LED_Alarm;
	axc_i32		iGpioPin_LED_Light;
	axc_i32		iGpioPin_Button_Reset;
	//
	// Modbus Uart
	//
	axc_bool	bModbusUartEnable;
	axc_dword	dwModbusUartDeviceId;
	axc_dword	dwModbusUartBaud;
	char		szModbusUartDeviceName[MAX_PATH];
	//
	// Modbus TCP
	//
	axc_bool	bModbusTcpEnable;
	axc_dword	dwModbusTcpPort;
	//
	// Thermal IVS: heat-object
	//
	axc_bool	bThermalHeatObj_Enable;
	axc_i32		iThermalHeatObj_BackTempeMode; // 0: C, 1: +-C, 2: +-%
	double		dbThermalHeatObj_BackTempe;
	axc_i32		iThermalHeatObj_HeatTempeMode;
	double		dbThermalHeatObj_HeatTempe;

	axc_byte	ThermalMask_Bits[THERMAL_MASK_BASE_CY][THERMAL_MASK_BASE_CX/8];
	axc_byte	ThermalMask_Reserve[110];

	//
	// live view
	//
	unsigned short iOpenStream;
} T_CONFIG_FILE_OPTIONS;

class Cheatfinderconfigmanager{
public:
	Cheatfinderconfigmanager();
	~Cheatfinderconfigmanager();
	bool run();

	void SetConfigContextAsDefault();
	bool ImportConfigContextFromFile(const char* cstrFilePath);
	bool ExportConfigContextToFile(const char* cstrFilePath);

	T_CONFIG_FILE_OPTIONS* GetConfigContext();
	bool GetIsVisionNeedRestartCapture();
	bool GetIsVisionParameterChanged();
	unsigned int GetChangedRuntimeVisionCommand();
	unsigned int GetChangedBootingVisionCommand();
	int GetImportVisionCommandValue(E_RPICAMERA_PARAMETER eParaID, char *pcstrValue, unsigned short iStrSize, int &iValue);
	int GetIDfromCommandRecordbyIndex(E_RPICAMERA_PARAMETER eParaID);
	bool IsBootCommandfromCommandRecordbyIndex(E_RPICAMERA_PARAMETER eParaID);
	bool IsInitialComplete();
protected:
	T_CONFIG_FILE_OPTIONS m_ConfigContext;
	//const char* VISION_CAPTURE_FIFO;
	//std::string VISION_CAPTURE_FIFO_;   //dragon ++
	//const char* VISION_RAW_CAPTURE_FIFO;
	//std::string VISION_RAW_CAPTURE_FIFO_;   //dragon ++

	T_RPICAMERA_COMMAND m_TRpiCommandRecord[RPICAMERA_TOTAL_PARAMETERS];

	void ConfigFile_MakeDefault(T_CONFIG_FILE_OPTIONS* pConfig);
	axc_bool ConfigFile_Read(const char* szFile, T_CONFIG_FILE_OPTIONS* pConfig);
	axc_bool ConfigFile_Write(const T_CONFIG_FILE_OPTIONS* pConfig, const char* szFile);
	void DebugPrintf(int iValue, const char* pStr1, const char* pStr2);
	char *getOpenStreamConfig();
    void ResetFlipFlag();

private:
	//
	// Status
	//
	std::mutex	m_joblocker;  // no mater R/W,
	bool	bInitalComplete;
};


#endif /* HEATFINDERCONFIGMANAGER_H_ */
