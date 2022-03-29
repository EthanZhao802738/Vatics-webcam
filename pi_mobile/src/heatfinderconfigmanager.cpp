/*
 * heatfinderconfigmanager.cpp
 *
 *  Created on: Feb 2, 2018
 *      Author: markhsieh
 */

#define DEBUG_CONFIGMANAGER false

#include "heatfinderconfigmanager.h"

/// general
#include <vector>
#include <string>

//dragon ++ >>
#include <vector>
#include <string>
//dragon ++ <<

//dragon ++ >>
inline std::vector<std::string> split(const std::string& str,
                                      const std::string& seperator)
{
    std::vector<std::string> result;
    typedef std::string::size_type string_size;
    string_size i = 0;

    while(i != str.size())
    {
        //找到字符串中首个不等于分隔符的字母；
        int flag = 0;
        while(i != str.size() && flag == 0)
        {
            flag = 1;
            for(string_size x = 0; x < seperator.size(); ++x)
            if(str[i] == seperator[x])
            {
                ++i;
                flag = 0;
                break;
            }
        }

        //找到又一个分隔符，将两个分隔符之间的字符串取出；
        flag = 0;
        string_size j = i;
        while(j != str.size() && flag == 0)
        {
            for(string_size x = 0; x < seperator.size(); ++x)
            if(str[j] == seperator[x]) {
                flag = 1;
                break;
            }
            if(flag == 0) ++j;
        }
        if(i != j) {
            result.push_back(str.substr(i, j-i));
            i = j;
        }
    }
    return result;
}
//dragon ++ <<

/// public
Cheatfinderconfigmanager::Cheatfinderconfigmanager(){
	//SetConfigContextAsDefault();
	bInitalComplete =false;
}
Cheatfinderconfigmanager::~Cheatfinderconfigmanager(){

}

bool Cheatfinderconfigmanager::run(){
	GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] %s: fill with default setting\n", "Config. Manager", __func__);
	SetConfigContextAsDefault();
	GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] %s: dynamic change setting by configuration file\n", "Config. Manager", __func__);
	bInitalComplete = ImportConfigContextFromFile(CONFIGURATION_FILE);
	return bInitalComplete;
}

void Cheatfinderconfigmanager::SetConfigContextAsDefault(){
	ConfigFile_MakeDefault(GetConfigContext());
}

bool Cheatfinderconfigmanager::ImportConfigContextFromFile(const char* cstrFilePath){
	return ((ConfigFile_Read(cstrFilePath, GetConfigContext()) == axc_true)?true:false);
}

bool Cheatfinderconfigmanager::ExportConfigContextToFile(const char* cstrFilePath){
	return ((ConfigFile_Write(GetConfigContext(), cstrFilePath) == axc_true)?true:false);
}

T_CONFIG_FILE_OPTIONS* Cheatfinderconfigmanager::GetConfigContext(){
	return &m_ConfigContext;
}

void Cheatfinderconfigmanager::DebugPrintf(int iValue, const char* pStr1, const char* pStr2){
#if DEBUG_CONFIGMANAGER
	printf("D: [Config. Manager] /%d '%s': '%s'\n", iValue, pStr1, pStr2);
#else
	//pass
#endif
}

bool Cheatfinderconfigmanager::GetIsVisionNeedRestartCapture(){
	if (m_ConfigContext.bNeedRestart == axc_true) {
		return true;
	} else {
		return false;
	}
}

bool Cheatfinderconfigmanager::GetIsVisionParameterChanged(){
	if (m_ConfigContext.bVisionParameterChanged == axc_true){
		return true;
	}else{
		return false;
	}
}

unsigned int Cheatfinderconfigmanager::GetChangedRuntimeVisionCommand(){
	return m_ConfigContext.iChangedCommand_Runtime;
}
unsigned int Cheatfinderconfigmanager::GetChangedBootingVisionCommand(){
	return m_ConfigContext.iChangedCommand_Boottime;
}

/**
 * @parameter eParaID: check 'E_RPICAMERA_PARAMETER' at globaldef.h
 *            *pcstrValue: string copy if this value is string or return NULL
 *            iStrSize: length of *pcstrValue, which shell bigger than 15
 *            &iValue: number copy if this value is integer or return 0
 * @return -1: error happen.
 *         0: which is c string return
 *         1: which is number reture
 */
int Cheatfinderconfigmanager::GetImportVisionCommandValue(E_RPICAMERA_PARAMETER eParaID, char *pcstrValue, unsigned short iStrSize, int &iValue){
	if ((eParaID >= RPICAMERA_TOTAL_PARAMETERS)||(eParaID <= RPICAMERA_PARAMETER_UNKNOWN)){
    	//CHeatFinderUtility::PrintNowData(1);
		GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] %s: parameter error: unknown id %d\n", "Config. Manager", __func__, eParaID);
		pcstrValue = NULL;
		iValue = 0;
		return -1;
	}else if (iStrSize < 16){
    	//CHeatFinderUtility::PrintNowData(1);
		GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] %s: parameter error: C String Buffer too small: %d\n", "Config. Manager", __func__, iStrSize);
		pcstrValue = NULL;
		iValue = 0;
		return -1;
	}else if (((m_TRpiCommandRecord[eParaID].id == RPICAMERA_COMMAND_RUNKNOWN)&& (m_TRpiCommandRecord[eParaID].isBooting == false))||
			((m_TRpiCommandRecord[eParaID].isBooting == true)&& (m_TRpiCommandRecord[eParaID].id == RPICAMERA_COMMAND_BUNKNOWN))){
		GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] %s: no value of this command_record[%d].id=%d .argv=%s\n",
				"Config. Manager", __func__, eParaID, m_TRpiCommandRecord[eParaID].id, m_TRpiCommandRecord[eParaID].argv);
		pcstrValue = NULL;
		iValue = 0;
		return -1;
	}

	if (m_TRpiCommandRecord[eParaID].isNum == true){
		iValue = atoi(m_TRpiCommandRecord[eParaID].argv);
		strncpy(pcstrValue, "-999", iStrSize);
		return 1;
	}else{
		//iValue = atoi(m_TRpiCommandRecord[eParaID].argv);
		iValue = -999;
		strncpy(pcstrValue, m_TRpiCommandRecord[eParaID].argv, iStrSize);
		return 0;
	}
}

int Cheatfinderconfigmanager::GetIDfromCommandRecordbyIndex(E_RPICAMERA_PARAMETER eParaID){
	return m_TRpiCommandRecord[eParaID].id;
}

bool Cheatfinderconfigmanager::IsBootCommandfromCommandRecordbyIndex(E_RPICAMERA_PARAMETER eParaID){
	return m_TRpiCommandRecord[eParaID].isBooting;
}

bool Cheatfinderconfigmanager::IsInitialComplete(){
	return bInitalComplete;
}

/// protected
void Cheatfinderconfigmanager::ConfigFile_MakeDefault(T_CONFIG_FILE_OPTIONS* pConfig)
{
	if(NULL != pConfig)
	{
		memset(pConfig, 0, sizeof(T_CONFIG_FILE_OPTIONS));

		//
		// Log
		//
		pConfig->dwLogLevel = AXC_LOG_LEVEL_DEBUG;

		//
		// Lepton Thermal Module
		//
#if defined(MACHINE_TARGET_PI) || defined(MACHINE_TARGET_ML) || defined(MACHINE_TARGET_WP) || defined(MACHINE_TARGET_PI_ER)
		FILE *l_fp = NULL;
	    char l_cstrPid[128] = {""};
	    if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/home/pi/heat-finder-runScripts/rest-server/config/camera_config.json"))) {
	        l_fp = popen("jq -r .pid /home/pi/heat-finder-runScripts/rest-server/config/camera_config.json | sed 's/-//g'", "r");
	    }else if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-merge/config/camera_config.json"))) {
	        l_fp = popen("jq -r .pid /home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-merge/config/camera_config.json | sed 's/-//g'", "r");
	    }else if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-release/config/camera_config.json"))) {
	        l_fp = popen("jq -r .pid /home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-release/config/camera_config.json | sed 's/-//g'", "r");
	    }
	    if (l_fp == NULL) {
	    	//CHeatFinderUtility::PrintNowData(1);
	        GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] %s: failed to get manual dev. name (short), error:[%d] %s\n", "Config. Manager", __func__, errno, "can not read string by popen()");
	    }else{
	          while (fgets(l_cstrPid, sizeof(l_cstrPid)-1, l_fp) != NULL) {
	              char *pos;
	              if ((pos=strchr(l_cstrPid, '\n')) != NULL)
	              {    *pos = '\0';}
	          }
	          pclose(l_fp);
	    }

		pConfig->bThermalEnable = axc_true;
		CAxcString::strcpy(pConfig->szThermalI2cDevice, "");
		CAxcString::strcpy(pConfig->szThermalSpiDevice, "/dev/spidev0.0");
		CAxcString::strcpy(pConfig->szThermalUartDevice, "/dev/serial0");  //Hank
		pConfig->dwThermalSpiSpeed = 10000000;
		pConfig->dwThermalSpiMode = 3;
		pConfig->dwThermalSpiBits = 8;
		pConfig->bThermalSpiSwapWord = axc_true;
		pConfig->bThermalSpiSwapByte = axc_false;
		pConfig->ucThermalOptional = 0x00;

		// if ((strlen(l_cstrPid) > 0)
		// 	&&(strstr(l_cstrPid, "AD-HF048-P") != NULL)){
		// 	// Only stander product no Ocher glass, so Thermal option keep 0x0.
		// 	pConfig->ucThermalOptional = 0x00;
		// }else{
		// 	pConfig->ucThermalOptional = 0x01;
		// }
		pConfig->dwEmissivity = 1000;
#elif defined(MACHINE_TARGET_TI)
		pConfig->bThermalEnable = axc_true;
		CAxcString::strcpy(pConfig->szThermalI2cDevice, "/dev/i2c/0");
		CAxcString::strcpy(pConfig->szThermalSpiDevice, "/dev/spidev1.0");
		pConfig->dwThermalSpiSpeed = 18000000;
		pConfig->dwThermalSpiMode = 3;
		pConfig->dwThermalSpiBits = 16;
		pConfig->bThermalSpiSwapWord = axc_false;
		pConfig->bThermalSpiSwapByte = axc_false;
		pConfig->ucThermalOptional = 0x00;
#else
		pConfig->bThermalEnable = axc_false;
		CAxcString::strcpy(pConfig->szThermalI2cDevice, "/dev/i2c-1");
		CAxcString::strcpy(pConfig->szThermalSpiDevice, "/dev/spidev0.0");
		pConfig->dwThermalSpiSpeed = 4000000;
		pConfig->dwThermalSpiMode = 3;
		pConfig->dwThermalSpiBits = 8;
		pConfig->bThermalSpiSwapWord = axc_true;
		pConfig->bThermalSpiSwapByte = axc_false;
		pConfig->ucThermalOptional = 0x00;
#endif

		//
		// Vision Camera
		//
#if defined(MACHINE_TARGET_PI) || defined(MACHINE_TARGET_ML) || defined(MACHINE_TARGET_WP) || defined(MACHINE_TARGET_PI_ER)
		pConfig->bVisionEnable = axc_true;
		pConfig->iChangedCommand_Boottime = 0;
		pConfig->iChangedCommand_Runtime = 0;
		CAxcString::snprintf(pConfig->szVisionCaptureApp, (axc_dword)sizeof(pConfig->szVisionCaptureApp), "raspivid -n -t 0 -w 1920 -h 1080 -b 1000000 -ih -g 15 -fps 15 -qp 0 -profile high -sh 0 -co 0 -br 50 -sa 0 -ex auto -awb auto -ifx none -o /tmp/adehf_vision.fifo 3>&2 2>>/tmp/raspi2");

		//memset(m_TRpiCommandRecord, 0, sizeof(m_TRpiCommandRecord)*RPICAMERA_TOTAL_PARAMETERS);
		for(/*E_RPICAMERA_PARAMETER*/int k=0; k<RPICAMERA_TOTAL_PARAMETERS; k++){
    		m_TRpiCommandRecord[k].id = 0;
    		//m_TRpiCommandRecord[k].argv = NULL;
    		memset(m_TRpiCommandRecord[k].argv, '\0', sizeof(m_TRpiCommandRecord[k].argv));
    		m_TRpiCommandRecord[k].isNum = false;
    		m_TRpiCommandRecord[k].isBooting = false;
		}
		ResetFlipFlag();

		//m_TRpiCommandRecord[RPICAMERA_TOTAL_PARAMETERS]
		std::string 	tmp = "0";
		const char* 	l_width =tmp.c_str();
		const char* 	l_height =tmp.c_str();
		const char* 	l_bitrate =tmp.c_str();
		const char* 	l_g =tmp.c_str();
		const char* 	l_fps =tmp.c_str();
		const char* 	l_sh =tmp.c_str();
		const char* 	l_co =tmp.c_str();
		const char* 	l_br =tmp.c_str();
		const char* 	l_sa =tmp.c_str();
		const char* 	l_ex =tmp.c_str();
		const char* 	l_awb =tmp.c_str();
		const char* 	l_ifx =tmp.c_str();
		const char* 	l_rf =tmp.c_str();
		const char* 	l_timeout =tmp.c_str();
		const char* 	l_qp =tmp.c_str();
		const char* 	l_h264 =tmp.c_str();
		//bool diffornot = false;
	    std::vector<std::string> l_strList = split(std::string(pConfig->szVisionCaptureApp), " ");
	    for (size_t i = 0; i < l_strList.size(); ++i)
	    {
	    	if (l_strList[i] == "-w"){
	    		l_width= l_strList[i+1].c_str();
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_RESOLUTION_W].id = RPICAMERA_COMMAND_RESOLUTION_W;
	    		//m_TRpiCommandRecord[RPICAMERA_PARAMETER_RESOLUTION_W].argv = (char *)l_width;
	    		strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_RESOLUTION_W].argv, l_width, strlen(l_width));
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_RESOLUTION_W].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_RESOLUTION_W].isBooting = true;
	    		DebugPrintf(RPICAMERA_PARAMETER_RESOLUTION_W, l_strList[i].c_str(), l_strList[i+1].c_str());
	    	}else if (l_strList[i] == "-h"){
	    		l_height= l_strList[i+1].c_str();
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_RESOLUTION_H].id = RPICAMERA_COMMAND_RESOLUTION_H;
	    		//m_TRpiCommandRecord[RPICAMERA_PARAMETER_RESOLUTION_H].argv = (char *)l_height;
	    		strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_RESOLUTION_H].argv, l_height, strlen(l_height));
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_RESOLUTION_H].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_RESOLUTION_H].isBooting = true;
	    		DebugPrintf(RPICAMERA_PARAMETER_RESOLUTION_H, l_strList[i].c_str(), l_strList[i+1].c_str());
	    	}else if (l_strList[i] == "-b"){
	    		l_bitrate= l_strList[i+1].c_str();
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_BITRATE].id = RPICAMERA_COMMAND_BITRATE;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_BITRATE].argv = (char *)l_bitrate;
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_BITRATE].argv, l_bitrate, strlen(l_bitrate));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_BITRATE].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_BITRATE].isBooting = true;
	    		DebugPrintf(RPICAMERA_PARAMETER_BITRATE, l_strList[i].c_str(), l_strList[i+1].c_str());
	    	}else if (l_strList[i] == "-g"){
	    		l_g= l_strList[i+1].c_str();
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_IPFRAMERATE].id = RPICAMERA_COMMAND_IPFRAMERATE;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_IPFRAMERATE].argv = (char *)l_g;
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_IPFRAMERATE].argv, l_g, strlen(l_g));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_IPFRAMERATE].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_IPFRAMERATE].isBooting = true;
	    		DebugPrintf(RPICAMERA_PARAMETER_IPFRAMERATE, l_strList[i].c_str(), l_strList[i+1].c_str());
	    	}else if (l_strList[i] == "-fps"){
	    		l_fps= l_strList[i+1].c_str();
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_FPS].id = RPICAMERA_COMMAND_FPS;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_FPS].argv = (char *)l_fps;
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_FPS].argv, l_fps, strlen(l_fps));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_FPS].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_FPS].isBooting = true;
	    		DebugPrintf(RPICAMERA_PARAMETER_FPS, l_strList[i].c_str(), l_strList[i+1].c_str());
	    	}else if (l_strList[i] == "-sh"){
	    		l_sh= l_strList[i+1].c_str();
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHARPNESS].id = RPICAMERA_COMMAND_SHARPNESS;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHARPNESS].argv = (char *)l_sh;
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHARPNESS].argv, l_sh, strlen(l_sh));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHARPNESS].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHARPNESS].isBooting = false;
	    		DebugPrintf(RPICAMERA_PARAMETER_SHARPNESS, l_strList[i].c_str(), l_strList[i+1].c_str());
	    	}else if (l_strList[i] == "-co"){
	    		l_co= l_strList[i+1].c_str();
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_CONTRAST].id = RPICAMERA_COMMAND_CONTRAST;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_CONTRAST].argv = (char *)l_co;
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_CONTRAST].argv, l_co, strlen(l_co));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_CONTRAST].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_CONTRAST].isBooting = false;
	    		DebugPrintf(RPICAMERA_PARAMETER_CONTRAST, l_strList[i].c_str(), l_strList[i+1].c_str());
	    	}else if (l_strList[i] == "-br"){
	    		l_br= l_strList[i+1].c_str();
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_BRIGHTNESS].id = RPICAMERA_COMMAND_BRIGHTNESS;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_BRIGHTNESS].argv = (char *)l_br;
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_BRIGHTNESS].argv, l_br, strlen(l_br));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_BRIGHTNESS].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_BRIGHTNESS].isBooting = false;
	    		DebugPrintf(RPICAMERA_PARAMETER_BRIGHTNESS, l_strList[i].c_str(), l_strList[i+1].c_str());
	    	}else if (l_strList[i] == "-sa"){
	    		l_sa= l_strList[i+1].c_str();
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_SATURATION].id = RPICAMERA_COMMAND_SATURATION;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_SATURATION].argv = (char *)l_sa;
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_SATURATION].argv, l_sa, strlen(l_sa));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_SATURATION].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_SATURATION].isBooting = false;
	    		DebugPrintf(RPICAMERA_PARAMETER_SATURATION, l_strList[i].c_str(), l_strList[i+1].c_str());
	    	}else if (l_strList[i] == "-ex"){
	    		l_ex= l_strList[i+1].c_str();
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_EXPOSURE].id = RPICAMERA_COMMAND_EXPOSURE;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_EXPOSURE].argv = (char *)l_ex;
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_EXPOSURE].argv, l_ex, strlen(l_ex));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_EXPOSURE].isNum = false;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_EXPOSURE].isBooting = false;
	    		DebugPrintf(RPICAMERA_PARAMETER_EXPOSURE, l_strList[i].c_str(), l_strList[i+1].c_str());
	    	}else if (l_strList[i] == "-awb"){
	    		l_awb= l_strList[i+1].c_str();
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_AWB].id = RPICAMERA_COMMAND_AWB;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_AWB].argv = (char *)l_awb;
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_AWB].argv, l_awb, strlen(l_awb));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_AWB].isNum = false;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_AWB].isBooting = false;
	    		DebugPrintf(RPICAMERA_PARAMETER_AWB, l_strList[i].c_str(), l_strList[i+1].c_str());
	    	}else if (l_strList[i] == "-ifx"){
	    		l_ifx= l_strList[i+1].c_str();
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_IMAGEEFFECT].id = RPICAMERA_COMMAND_IMAGEEFFECT;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_IMAGEEFFECT].argv = (char *)l_ifx;
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_IMAGEEFFECT].argv, l_ifx, strlen(l_ifx));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_IMAGEEFFECT].isNum = false;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_IMAGEEFFECT].isBooting = false;
	    		DebugPrintf(RPICAMERA_PARAMETER_IMAGEEFFECT, l_strList[i].c_str(), l_strList[i+1].c_str());
	    	}else if (l_strList[i] == "-rf"){
	    		l_rf= l_strList[i+1].c_str();
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_RAWCAPTURE].id = RPICAMERA_COMMAND_RAWCAPTURE;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_RAWCAPTURE].argv = (char *)l_rf;
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_RAWCAPTURE].argv, l_rf, strlen(l_rf));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_RAWCAPTURE].isNum = false;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_RAWCAPTURE].isBooting = true;
	    		DebugPrintf(RPICAMERA_PARAMETER_RAWCAPTURE ,l_strList[i].c_str(), l_strList[i+1].c_str());
	    	}else if (l_strList[i] == "-t"){
	    		l_timeout= l_strList[i+1].c_str();
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_TIMEOUT].id = RPICAMERA_COMMAND_TIMEOUT;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_TIMEOUT].argv = (char *)l_timeout;
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_TIMEOUT].argv, l_timeout, strlen(l_timeout));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_TIMEOUT].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_TIMEOUT].isBooting = true;
	    		DebugPrintf(RPICAMERA_PARAMETER_TIMEOUT, l_strList[i].c_str(), l_strList[i+1].c_str());
	    	}else if (l_strList[i] == "-v"){
	    		//l_timeout= l_strList[i+1].c_str();
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_VERBOSE].id = RPICAMERA_COMMAND_VERBOSE;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_VERBOSE].argv = (char *)"1";
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_VERBOSE].argv, "1", sizeof(m_TRpiCommandRecord[RPICAMERA_PARAMETER_VERBOSE].argv));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_VERBOSE].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_VERBOSE].isBooting = false;
	    		DebugPrintf(RPICAMERA_PARAMETER_VERBOSE, l_strList[i].c_str(), "(DONOT Care) I got it.");
	    	}else if (l_strList[i] == "-n"){
	    		//l_timeout= l_strList[i+1].c_str();
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_PREVIEWENC].id = RPICAMERA_COMMAND_PREVIEWENC;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_PREVIEWENC].argv = (char *)"1";
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_PREVIEWENC].argv, "1", sizeof(m_TRpiCommandRecord[RPICAMERA_PARAMETER_PREVIEWENC].argv));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_PREVIEWENC].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_PREVIEWENC].isBooting = true;
	    		DebugPrintf(RPICAMERA_PARAMETER_PREVIEWENC, l_strList[i].c_str(), "(DONOT Care) I got it.");
	    	}else if (l_strList[i] == "-ih"){
	    		//l_timeout= l_strList[i+1].c_str();
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHOWSPSPPS].id = RPICAMERA_COMMAND_SHOWSPSPPS;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHOWSPSPPS].argv = (char *)"1";
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHOWSPSPPS].argv, "1", sizeof(m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHOWSPSPPS].argv));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHOWSPSPPS].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHOWSPSPPS].isBooting = true;
	    		DebugPrintf(RPICAMERA_PARAMETER_SHOWSPSPPS, l_strList[i].c_str(), "(DONOT Care) I got it.");
	    	}else if (l_strList[i] == "-qp"){
	    		l_qp = l_strList[i+1].c_str();
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_QP].id = RPICAMERA_COMMAND_QP;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHOWSPSPPS].argv = (char *)"1";
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_QP].argv, l_qp, sizeof(m_TRpiCommandRecord[RPICAMERA_PARAMETER_QP].argv));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_QP].isNum = true;
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_QP].isBooting = true;
				DebugPrintf(RPICAMERA_PARAMETER_QP, l_strList[i].c_str(), l_qp);
	    	}else if (l_strList[i] == "-profile"){
	    		l_h264 = l_strList[i+1].c_str();
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_H264PROFILE].id = RPICAMERA_COMMAND_H264PROFILE;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHOWSPSPPS].argv = (char *)"1";
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_H264PROFILE].argv, l_h264, sizeof(m_TRpiCommandRecord[RPICAMERA_PARAMETER_H264PROFILE].argv));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_H264PROFILE].isNum = false;
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_H264PROFILE].isBooting = true;
				DebugPrintf(RPICAMERA_PARAMETER_H264PROFILE, l_strList[i].c_str(), l_h264);
	    	}else if (l_strList[i] == "-hf"){
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_HFLIP].id = RPICAMERA_COMMAND_HFLIP;
				strcpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_HFLIP].argv, "1");
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_HFLIP].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_HFLIP].isBooting = true;
	    		DebugPrintf(RPICAMERA_PARAMETER_HFLIP, l_strList[i].c_str(), "Need --hflip");
	    	}else if (l_strList[i] == "-vf"){
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_VFLIP].id = RPICAMERA_COMMAND_VFLIP;
				strcpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_VFLIP].argv, "1");
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_VFLIP].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_VFLIP].isBooting = true;
	    		DebugPrintf(RPICAMERA_PARAMETER_VFLIP, l_strList[i].c_str(), "Need --vflip");
	    	}
	    }
#else
		pConfig->bVisionEnable = axc_false;
		CAxcString::strcpy(pConfig->szVisionCaptureApp, "");
#endif

		//
		// GPIO pin index : <0 disable, >=0 gpio pin index
		//
#if defined(MACHINE_TARGET_PI) || defined(MACHINE_TARGET_ML) || defined(MACHINE_TARGET_WP) || defined(MACHINE_TARGET_PI_ER)
		pConfig->iGpioPin_LED_Status = 17;
		pConfig->iGpioPin_LED_Network = 27;
		pConfig->iGpioPin_LED_Alarm = 22;
		pConfig->iGpioPin_LED_Light = 26;
		pConfig->iGpioPin_Button_Reset = 23;
#else
		pConfig->iGpioPin_LED_Status = -1;
		pConfig->iGpioPin_LED_Network = -1;
		pConfig->iGpioPin_LED_Alarm = -1;
		pConfig->iGpioPin_LED_Light = -1;
		pConfig->iGpioPin_Button_Reset = -1;
#endif

		//
		// Modbus Uart
		//
#if defined(MACHINE_TARGET_PI) || defined(MACHINE_TARGET_ML) || defined(MACHINE_TARGET_WP) || defined(MACHINE_TARGET_PI_ER)
		pConfig->bModbusUartEnable = axc_true;
		pConfig->dwModbusUartDeviceId = 51;
		pConfig->dwModbusUartBaud = 0;
		CAxcString::strcpy(pConfig->szModbusUartDeviceName, "/dev/serial0");
#else
		pConfig->bModbusUartEnable = axc_false;
		pConfig->dwModbusUartDeviceId = 51;
		pConfig->dwModbusUartBaud = 9600;
		CAxcString::strcpy(pConfig->szModbusUartDeviceName, "");
#endif

		//
		// Modbus TCP
		//
		pConfig->bModbusTcpEnable = axc_true;
		pConfig->dwModbusTcpPort = 5502;

		//
		// Thermal IVS: heat-object
		//
		pConfig->bThermalHeatObj_Enable = axc_true;
		pConfig->iThermalHeatObj_BackTempeMode = 0;
		pConfig->dbThermalHeatObj_BackTempe = 28.0;
		pConfig->iThermalHeatObj_HeatTempeMode = 0;
		pConfig->dbThermalHeatObj_HeatTempe = 33.0;
		memset(pConfig->ThermalMask_Bits, 0, sizeof(pConfig->ThermalMask_Bits));

		pConfig->bNeedRestart = axc_false;
		pConfig->bVisionParameterChanged = axc_false;

		//
		// live view
		//
		pConfig->iOpenStream = (unsigned short)(OPEN_CAMERA_STREAM | OPEN_THERMAL_STREAM);
	}
}

axc_bool Cheatfinderconfigmanager::ConfigFile_Read(const char* szFile, T_CONFIG_FILE_OPTIONS* pConfig)
{
	//GLog(tAll, tDEBUGTrace_MSK, "D: [Config] [Gavin] +++Cheatfinderconfigmanager::ConfigFile_Read\n");
	//GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] %s prepare to read file: '%s'\n", "Config. Manager", __func__, szFile);
	if(NULL == szFile || 0 == szFile[0] || NULL == pConfig)
	{
		GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] %s failed to read: invalid parameters\n", "Config. Manager", __func__);
		GLog(tAll, tDEBUGTrace_MSK, "D: [Config] [Gavin] ---Cheatfinderconfigmanager::ConfigFile_Read (7)\n");
		return axc_false;
	}

	//
	// open file
	//
	CAxcFile file("config_file/read");
	if(!file.Open(szFile,"rb"))
	{
		GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] %s failed to open file '%s', error: %s\n", "Config. Manager", __func__, szFile, AxcGetLastErrorText());
		GLog(tAll, tDEBUGTrace_MSK, "D: [Config] [Gavin] ---Cheatfinderconfigmanager::ConfigFile_Read (6)\n");
		return axc_false;
	}
	const axc_i64 iFileSize = file.FileSize();
	if(iFileSize <= 0 || iFileSize > (10*1024*1024))
	{
		GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] %s failed to read: file size error: %lld\n", "Config. Manager", __func__, iFileSize);
		GLog(tAll, tDEBUGTrace_MSK, "D: [Config] [Gavin] ---Cheatfinderconfigmanager::ConfigFile_Read (5)\n");
		return axc_false;
	}
	const axc_dword dwFileSize = (axc_dword)iFileSize;

	//
	// read file to buffer
	//
	CAxcMemory buffer("config_file/read_buffer");
	if(!buffer.Create(dwFileSize+2))
	{
		GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] %s failed to create buffer, size = %u, error: %s\n", "Config. Manager", __func__, dwFileSize, AxcGetLastErrorText());
		GLog(tAll, tDEBUGTrace_MSK, "D: [Config] [Gavin] ---Cheatfinderconfigmanager::ConfigFile_Read (4)\n");
		return axc_false;
	}
	char* pszReadBuffer = (char*) buffer.GetAddress();
	const axc_dword dwReadSize = file.Read(pszReadBuffer, dwFileSize);
	if(dwReadSize == 0)
	{
		GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] %s failed to read file to buffer, read size %u, error: %s\n", "Config. Manager", __func__, dwReadSize, AxcGetLastErrorText());
		GLog(tAll, tDEBUGTrace_MSK, "D: [Config] [Gavin] ---Cheatfinderconfigmanager::ConfigFile_Read (3)\n");
		return axc_false;
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
	if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(pszReadBuffer, dwReadSize, "log_level", szValue, (axc_dword)sizeof(szValue)) > 0)
	{
		const axc_dword dwLevel = (axc_dword) atoi(szValue);
		if(dwLevel <= 6)
		{
			pConfig->dwLogLevel = dwLevel;
		}
	}
	if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(pszReadBuffer, dwReadSize, "thermal_heatobj_enable", szValue, (axc_dword)sizeof(szValue)) > 0)
	{
		pConfig->bThermalHeatObj_Enable = (axc_bool) atoi(szValue);
	}
	if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(pszReadBuffer, dwReadSize, "thermal_heatobj_back_tempe", szValue, (axc_dword)sizeof(szValue)) > 0)
	{
		if(!CHeatFinderUtility::TextToTemperature(szValue, &pConfig->dbThermalHeatObj_BackTempe, &pConfig->iThermalHeatObj_BackTempeMode))
		{
			GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] %s Invalidate option 'thermal_heatobj_back_tempe' value '%s' !\n", "Config. Manager", __func__, szValue);
			GLog(tAll, tDEBUGTrace_MSK, "D: [Config] [Gavin] ---Cheatfinderconfigmanager::ConfigFile_Read (2)\n");
			return axc_false;
		}
	}
	if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(pszReadBuffer, dwReadSize, "thermal_heatobj_heat_tempe",szValue, sizeof(szValue)) > 0)
	{
		if(!CHeatFinderUtility::TextToTemperature(szValue, &pConfig->dbThermalHeatObj_HeatTempe, &pConfig->iThermalHeatObj_HeatTempeMode))
		{
			GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [%s] %s Invalidate option 'thermal_heatobj_heat_tempe' value '%s' !\n", "Config. Manager", __func__, szValue);
			GLog(tAll, tDEBUGTrace_MSK, "D: [Config] [Gavin] ---Cheatfinderconfigmanager::ConfigFile_Read (1)\n");
			return axc_false;
		}
	}
	if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(pszReadBuffer, dwReadSize, "thermal_mask_bits",szValue, sizeof(szValue)) >= (axc_i32)(sizeof(pConfig->ThermalMask_Bits)*2))
	{
		const int iBytes = sizeof(pConfig->ThermalMask_Bits);
		axc_byte* pbyBits = (axc_byte*) pConfig->ThermalMask_Bits;
		const char* pszValue = szValue;
		axc_dword dwTemp = 0;
		char szTemp[3] = {0,};
		for(int i = 0; i<iBytes; i++, pbyBits++, pszValue += 2)
		{
			memcpy(szTemp, pszValue, 2);
			szTemp[2] = 0;
			dwTemp = 0;
			sscanf(szTemp, "%02X", &dwTemp);
			*pbyBits = (axc_byte)(dwTemp & 0xFF);
		}
		//GLog(tAll, tDEBUGTrace_MSK, "D: [%s] %s [Gavin]read 'thermal_mask_bits' form adehf.ini\n", "Config. Manager", __func__);
	}

	if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(pszReadBuffer, dwReadSize, "thermal_extend_option", szValue, (axc_dword)sizeof(szValue)) > 0)
	{
		pConfig->ucThermalOptional = (uint8_t) atoi(szValue);
		//GLog(tAll, tDEBUGTrace_MSK, "D: [%s] %s [Gavin]read 'thermal_extend_option=%d' form adehf.ini\n", "Config. Manager", __func__, pConfig->ucThermalOptional);
	}

	if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(pszReadBuffer, dwReadSize, "modbus_uart_baud",szValue,sizeof(szValue)) > 0)
	{
		pConfig->dwModbusUartBaud = (axc_dword) atoi(szValue);
	}
	if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(pszReadBuffer, dwReadSize, "modbus_uart_device_id",szValue,sizeof(szValue)) > 0)
	{
		pConfig->dwModbusUartDeviceId = (axc_dword) atoi(szValue);
	}

	if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(pszReadBuffer, dwReadSize, "open_stream", szValue, (axc_dword)sizeof(szValue)) > 0){
		// Merge the new Setting to configuration
		axc_bool bThermal = (NULL != strstr(szValue,"thermal"));
		axc_bool bVision = (NULL != strstr(szValue,"vision"));
		if(bThermal && bVision){
			pConfig->iOpenStream = (unsigned short)(OPEN_CAMERA_STREAM | OPEN_THERMAL_STREAM);
		}else if(bThermal){
			pConfig->iOpenStream = (unsigned short)(OPEN_THERMAL_STREAM);
		}else if(bVision){
			pConfig->iOpenStream = (unsigned short)(OPEN_CAMERA_STREAM);
		}else{
			pConfig->iOpenStream = (unsigned short)OPEN_NONE_STREAM;
		}
	}

	int nReadLen = CHeatFinderUtility::GetCameraConfigParameter("emissivity", szValue, sizeof(szValue));
	if (0 <= nReadLen || nReadLen < sizeof(szValue))
	{
		double dbValue = atof(szValue);
		pConfig->dwEmissivity = (axc_dword)(dbValue * 1000);
		//GLog(tAll, tDEBUGTrace_MSK, "D: [Config. Manager] %s [Gavin]read 'emissivity=%d (%.1f)' form camera_config.json\n", __func__, pConfig->dwEmissivity, dbValue);
	}

	/*need check input diff. or not*/
	pConfig->iChangedCommand_Boottime = 0;
	pConfig->iChangedCommand_Runtime = 0;
	//const char* 	cc_vc_fifo = VISION_CAPTURE_FIFO_.c_str();
	//const char* 	cc_vc_raw_fifo = VISION_RAW_CAPTURE_FIFO_.c_str();
	std::string 	tmp = "000000";
	const char* 	l_width =tmp.c_str();
	const char* 	l_height =tmp.c_str();
	const char* 	l_bitrate =tmp.c_str();
	const char* 	l_g =tmp.c_str();
	const char* 	l_fps =tmp.c_str();
	const char* 	l_sh =tmp.c_str();
	const char* 	l_co =tmp.c_str();
	const char* 	l_br =tmp.c_str();
	const char* 	l_sa =tmp.c_str();
	const char* 	l_ex =tmp.c_str();
	const char* 	l_awb =tmp.c_str();
	const char* 	l_ifx =tmp.c_str();
	const char* 	l_rf =tmp.c_str();
	const char* 	l_timeout =tmp.c_str();
	bool diffornot = false;
    std::vector<std::string> l_strList = split(std::string(pConfig->szVisionCaptureApp), " ");
    for (size_t i = 0; i < l_strList.size(); ++i)
    {
    	if (l_strList[i] == "-w"){
    		l_width= l_strList[i+1].c_str();
    	}else if (l_strList[i] == "-h"){
    		l_height= l_strList[i+1].c_str();
    	}else if (l_strList[i] == "-b"){
    		l_bitrate= l_strList[i+1].c_str();
    	}else if (l_strList[i] == "-g"){
    		l_g= l_strList[i+1].c_str();
    	}else if (l_strList[i] == "-fps"){
    		l_fps= l_strList[i+1].c_str();
    	}else if (l_strList[i] == "-sh"){
    		l_sh= l_strList[i+1].c_str();
    	}else if (l_strList[i] == "-co"){
    		l_co= l_strList[i+1].c_str();
    	}else if (l_strList[i] == "-br"){
    		l_br= l_strList[i+1].c_str();
    	}else if (l_strList[i] == "-sa"){
    		l_sa= l_strList[i+1].c_str();
    	}else if (l_strList[i] == "-ex"){
    		l_ex= l_strList[i+1].c_str();
    	}else if (l_strList[i] == "-awb"){
    		l_awb= l_strList[i+1].c_str();
    	}else if (l_strList[i] == "-ifx"){
    		l_ifx= l_strList[i+1].c_str();
    	}else if (l_strList[i] == "-rf"){
    		l_rf= l_strList[i+1].c_str();
    	}else if (l_strList[i] == "-t"){
    		l_timeout= l_strList[i+1].c_str();
    	}
    	/**
    	else if (l_strList[i] == "-profile"){
    		l_profile= l_strList[i+1].c_str();
    	}else if (l_strList[i] == "-qp"){
    		l_qp= l_strList[i+1].c_str();
    	}else if (l_strList[i] == "-t"){
    		l_timeout= l_strList[i+1].c_str();
    	}**/
    }
    //GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] Original vision_capture_app string: '%s'\n", "Config. Manager", pConfig->szVisionCaptureApp);

	if(CHeatFinderUtility::CvmsHelper_Cmd_GetValue(pszReadBuffer, dwReadSize, "vision_capture_app",szValue,sizeof(szValue)) > 0)
	{
		if(strlen(szValue) < sizeof(pConfig->szVisionCaptureApp))
		{
			CAxcString::strcpy(pConfig->szVisionCaptureApp, szValue);
		}
	}

    //GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] --New--  vision_capture_app string: '%s'\n", "Config. Manager", pConfig->szVisionCaptureApp);
    ResetFlipFlag();
    //dragon ++ >>
    // parse fifo path.
	// check different or not. Reload action will not do restart function when no different
    int l_iDiffCount = 0;
    std::vector<std::string> strList = split(std::string(pConfig->szVisionCaptureApp), " ");
    for (size_t i = 0; i < strList.size(); ++i)
    {
    	/**
        if (strList[i] == "-o")
        {
            VISION_CAPTURE_FIFO_ = strList[i+1];
            VISION_CAPTURE_FIFO = VISION_CAPTURE_FIFO_.c_str();
            diffornot |= (0 == CAxcString::strcmp(cc_vc_fifo, VISION_CAPTURE_FIFO, axc_false))? false: true;
            if (diffornot == true){ printf("D: [Vision] '%s' != '%s' \n",cc_vc_fifo,VISION_CAPTURE_FIFO); }
        }
        else if (strList[i] == "-r")
        {
            VISION_RAW_CAPTURE_FIFO_ = strList[i+1];
            VISION_RAW_CAPTURE_FIFO = VISION_RAW_CAPTURE_FIFO_.c_str();
            diffornot |= (0 == CAxcString::strcmp(cc_vc_raw_fifo, VISION_RAW_CAPTURE_FIFO, axc_false))? false: true;
            if (diffornot == true){ printf("D: [Vision] '%s' != '%s' \n",cc_vc_raw_fifo,VISION_RAW_CAPTURE_FIFO); }
        }
        else
        **/
        if (strList[i] == "-w"){
        	diffornot = (0 == CAxcString::strcmp(l_width, strList[i+1].c_str()))? false: true;
            if (diffornot == true){
            	//GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Config. Manager] -w '%s' != '%s' \n",l_width,strList[i+1].c_str()); l_iDiffCount++;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_RESOLUTION_W].id = RPICAMERA_COMMAND_RESOLUTION_W;
	    		//m_TRpiCommandRecord[RPICAMERA_PARAMETER_RESOLUTION_W].argv = (char *)strList[i+1].c_str();
	    		strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_RESOLUTION_W].argv, (char *)strList[i+1].c_str(), sizeof(m_TRpiCommandRecord[RPICAMERA_PARAMETER_RESOLUTION_W].argv));
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_RESOLUTION_W].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_RESOLUTION_W].isBooting = true;
	    		pConfig->iChangedCommand_Boottime |= RPICAMERA_COMMAND_RESOLUTION_W;
            }
    	}else if (strList[i] == "-h"){
    		diffornot = (0 == CAxcString::strcmp(l_height, strList[i+1].c_str()))? false: true;
            if (diffornot == true){
            	//GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Config. Manager] -h '%s' != '%s' \n",l_height,strList[i+1].c_str()); l_iDiffCount++;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_RESOLUTION_H].id = RPICAMERA_COMMAND_RESOLUTION_H;
	    		//m_TRpiCommandRecord[RPICAMERA_PARAMETER_RESOLUTION_H].argv = (char *)strList[i+1].c_str();
	    		strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_RESOLUTION_H].argv, (char *)strList[i+1].c_str(), sizeof(m_TRpiCommandRecord[RPICAMERA_PARAMETER_RESOLUTION_H].argv));
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_RESOLUTION_H].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_RESOLUTION_H].isBooting = true;
	    		pConfig->iChangedCommand_Boottime |= RPICAMERA_COMMAND_RESOLUTION_H;
            }
    	}else if (strList[i] == "-b"){
    		diffornot = (0 == CAxcString::strcmp(l_bitrate, strList[i+1].c_str()))? false: true;
            if (diffornot == true){
            	//GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Config. Manager] -b '%s' != '%s' \n",l_bitrate,strList[i+1].c_str()); l_iDiffCount++;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_BITRATE].id = RPICAMERA_COMMAND_BITRATE;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_BITRATE].argv = (char *)strList[i+1].c_str();
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_BITRATE].argv, (char *)strList[i+1].c_str(), sizeof(m_TRpiCommandRecord[RPICAMERA_PARAMETER_BITRATE].argv));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_BITRATE].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_BITRATE].isBooting = true;
	    		pConfig->iChangedCommand_Boottime |= RPICAMERA_COMMAND_BITRATE;
            }
    	}else if (strList[i] == "-g"){
    		diffornot = (0 == CAxcString::strcmp(l_g, strList[i+1].c_str()))? false: true;
            if (diffornot == true){
            	//GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Config. Manager] -g '%s' != '%s' \n",l_g,strList[i+1].c_str()); l_iDiffCount++;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_IPFRAMERATE].id = RPICAMERA_COMMAND_IPFRAMERATE;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_IPFRAMERATE].argv = (char *)strList[i+1].c_str();
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_IPFRAMERATE].argv, (char *)strList[i+1].c_str(), sizeof(m_TRpiCommandRecord[RPICAMERA_PARAMETER_IPFRAMERATE].argv));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_IPFRAMERATE].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_IPFRAMERATE].isBooting = true;
	    		pConfig->iChangedCommand_Boottime |= RPICAMERA_COMMAND_IPFRAMERATE;
            }
    	}else if (strList[i] == "-fps"){
    		diffornot = (0 == CAxcString::strcmp(l_fps, strList[i+1].c_str()))? false: true;
            if (diffornot == true){
            	//GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Config. Manager] -fps '%s' != '%s' \n",l_fps,strList[i+1].c_str()); l_iDiffCount++;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_FPS].id = RPICAMERA_COMMAND_FPS;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_FPS].argv = (char *)strList[i+1].c_str();
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_FPS].argv, (char *)strList[i+1].c_str(), sizeof(m_TRpiCommandRecord[RPICAMERA_PARAMETER_FPS].argv));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_FPS].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_FPS].isBooting = true;
	    		pConfig->iChangedCommand_Boottime |= RPICAMERA_COMMAND_FPS;
            }
    	}else if (strList[i] == "-sh"){
    		diffornot = (0 == CAxcString::strcmp(l_sh, strList[i+1].c_str()))? false: true;
            if (diffornot == true){
            	//GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Config. Manager] -sh '%s' != '%s' \n",l_sh,strList[i+1].c_str()); l_iDiffCount++;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHARPNESS].id = RPICAMERA_COMMAND_SHARPNESS;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHARPNESS].argv = (char *)strList[i+1].c_str();
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHARPNESS].argv, (char *)strList[i+1].c_str(), sizeof(m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHARPNESS].argv));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHARPNESS].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHARPNESS].isBooting = false;
	    		pConfig->iChangedCommand_Runtime |= RPICAMERA_COMMAND_SHARPNESS;
            }
    	}else if (strList[i] == "-co"){
    		diffornot = (0 == CAxcString::strcmp(l_co, strList[i+1].c_str()))? false: true;
            if (diffornot == true){
            	//GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Config. Manager] -co '%s' != '%s' \n",l_co,strList[i+1].c_str()); l_iDiffCount++;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_CONTRAST].id = RPICAMERA_COMMAND_CONTRAST;
				//TRpiCommandRecord[RPICAMERA_PARAMETER_CONTRAST].argv = (char *)strList[i+1].c_str();;
	    		strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_CONTRAST].argv, (char *)strList[i+1].c_str(), sizeof(m_TRpiCommandRecord[RPICAMERA_PARAMETER_CONTRAST].argv));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_CONTRAST].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_CONTRAST].isBooting = false;
	    		pConfig->iChangedCommand_Runtime |= RPICAMERA_COMMAND_CONTRAST;
            }
    	}else if (strList[i] == "-br"){
    		diffornot = (0 == CAxcString::strcmp(l_br, strList[i+1].c_str()))? false: true;
            if (diffornot == true){
            	//GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Config. Manager] -br '%s' != '%s' \n",l_br,strList[i+1].c_str()); l_iDiffCount++;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_BRIGHTNESS].id = RPICAMERA_COMMAND_BRIGHTNESS;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_BRIGHTNESS].argv = (char *)strList[i+1].c_str();
	    		strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_BRIGHTNESS].argv, (char *)strList[i+1].c_str(), sizeof(m_TRpiCommandRecord[RPICAMERA_PARAMETER_BRIGHTNESS].argv));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_BRIGHTNESS].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_BRIGHTNESS].isBooting = false;
	    		pConfig->iChangedCommand_Runtime |= RPICAMERA_COMMAND_BRIGHTNESS;

            }
    	}else if (strList[i] == "-sa"){
    		diffornot = (0 == CAxcString::strcmp(l_sa, strList[i+1].c_str()))? false: true;
            if (diffornot == true){
            	//GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Config. Manager] -sa '%s' != '%s' \n",l_sa,strList[i+1].c_str()); l_iDiffCount++;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_SATURATION].id = RPICAMERA_COMMAND_SATURATION;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_SATURATION].argv = (char *)strList[i+1].c_str();
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_SATURATION].argv, (char *)strList[i+1].c_str(), sizeof(m_TRpiCommandRecord[RPICAMERA_PARAMETER_SATURATION].argv));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_SATURATION].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_SATURATION].isBooting = false;
	    		pConfig->iChangedCommand_Runtime |= RPICAMERA_COMMAND_SATURATION;
            }
    	}else if (strList[i] == "-ex"){
    		diffornot = (0 == CAxcString::strcmp(l_ex, strList[i+1].c_str()))? false: true;
            if (diffornot == true){
            	//GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Config. Manager] -ex '%s' != '%s' \n",l_ex,strList[i+1].c_str()); l_iDiffCount++;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_EXPOSURE].id = RPICAMERA_COMMAND_EXPOSURE;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_EXPOSURE].argv = (char *)strList[i+1].c_str();
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_EXPOSURE].argv, (char *)strList[i+1].c_str(), sizeof(m_TRpiCommandRecord[RPICAMERA_PARAMETER_EXPOSURE].argv));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_EXPOSURE].isNum = false;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_EXPOSURE].isBooting = false;
	    		pConfig->iChangedCommand_Runtime |= RPICAMERA_COMMAND_EXPOSURE;
            }
    	}else if (strList[i] == "-awb"){
    		diffornot = (0 == CAxcString::strcmp(l_awb, strList[i+1].c_str()))? false: true;
            if (diffornot == true){
            	//GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Config. Manager] -awb '%s' != '%s' \n",l_awb,strList[i+1].c_str()); l_iDiffCount++;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_AWB].id = RPICAMERA_COMMAND_AWB;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_AWB].argv = (char *)strList[i+1].c_str();
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_AWB].argv, (char *)strList[i+1].c_str(), sizeof(m_TRpiCommandRecord[RPICAMERA_PARAMETER_AWB].argv));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_AWB].isNum = false;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_AWB].isBooting = false;
	    		pConfig->iChangedCommand_Runtime |= RPICAMERA_COMMAND_AWB;
            }
    	}else if (strList[i] == "-ifx"){
    		diffornot = (0 == CAxcString::strcmp(l_ifx, strList[i+1].c_str()))? false: true;
            if (diffornot == true){
            	//GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Config. Manager] -ifx '%s' != '%s' \n",l_ifx,strList[i+1].c_str()); l_iDiffCount++;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_IMAGEEFFECT].id = RPICAMERA_COMMAND_IMAGEEFFECT;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_IMAGEEFFECT].argv = (char *)strList[i+1].c_str();
	    		strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_IMAGEEFFECT].argv, (char *)strList[i+1].c_str(), sizeof(m_TRpiCommandRecord[RPICAMERA_PARAMETER_IMAGEEFFECT].argv));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_IMAGEEFFECT].isNum = false;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_IMAGEEFFECT].isBooting = false;
	    		pConfig->iChangedCommand_Runtime |= RPICAMERA_COMMAND_IMAGEEFFECT;
            }
    	}else if (strList[i] == "-rf"){
    		diffornot = (0 == CAxcString::strcmp(l_rf, strList[i+1].c_str()))? false: true;
            if (diffornot == true){
            	//GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Config. Manager] -rf '%s' != '%s' \n",l_rf,strList[i+1].c_str()); l_iDiffCount++;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_RAWCAPTURE].id = RPICAMERA_COMMAND_RAWCAPTURE;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_RAWCAPTURE].argv = (char *)l_rf;
	    		strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_RAWCAPTURE].argv, (char *)strList[i+1].c_str(), sizeof(m_TRpiCommandRecord[RPICAMERA_PARAMETER_RAWCAPTURE].argv));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_RAWCAPTURE].isNum = false;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_RAWCAPTURE].isBooting = true;
	    		pConfig->iChangedCommand_Boottime |= RPICAMERA_COMMAND_RAWCAPTURE;
            }
    	}else if (strList[i] == "-t"){
    		diffornot = (0 == CAxcString::strcmp(l_timeout, strList[i+1].c_str()))? false: true;
    		if (diffornot == true){
    			//GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Config. Manager] -t '%s' != '%s' \n",l_timeout,strList[i+1].c_str()); l_iDiffCount++;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_TIMEOUT].id = RPICAMERA_COMMAND_TIMEOUT;
				//m_TRpiCommandRecord[RPICAMERA_PARAMETER_TIMEOUT].argv = (char *)l_timeout;
				strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_TIMEOUT].argv, (char *)strList[i+1].c_str(), sizeof(m_TRpiCommandRecord[RPICAMERA_PARAMETER_TIMEOUT].argv));
				m_TRpiCommandRecord[RPICAMERA_PARAMETER_TIMEOUT].isNum = true;
	    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_TIMEOUT].isBooting = true;
	    		pConfig->iChangedCommand_Boottime |= RPICAMERA_COMMAND_TIMEOUT;
    		}
    	}else if (strList[i] == "-v"){
    		//l_timeout= l_strList[i+1].c_str();
    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_VERBOSE].id = RPICAMERA_COMMAND_VERBOSE;
			//m_TRpiCommandRecord[RPICAMERA_PARAMETER_VERBOSE].argv = (char *)"1";
			strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_VERBOSE].argv, (char *)"1", sizeof(m_TRpiCommandRecord[RPICAMERA_PARAMETER_VERBOSE].argv));
			m_TRpiCommandRecord[RPICAMERA_PARAMETER_VERBOSE].isNum = true;
    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_VERBOSE].isBooting = false;
    		pConfig->iChangedCommand_Runtime |= RPICAMERA_COMMAND_VERBOSE;
    	}else if (strList[i] == "-n"){
    		//l_timeout= l_strList[i+1].c_str();
    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_PREVIEWENC].id = RPICAMERA_COMMAND_PREVIEWENC;
			//m_TRpiCommandRecord[RPICAMERA_PARAMETER_PREVIEWENC].argv = (char *)"1";
    		strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_PREVIEWENC].argv, (char *)"1", sizeof(m_TRpiCommandRecord[RPICAMERA_PARAMETER_PREVIEWENC].argv));
			m_TRpiCommandRecord[RPICAMERA_PARAMETER_PREVIEWENC].isNum = true;
    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_PREVIEWENC].isBooting = true;
    		pConfig->iChangedCommand_Boottime |= RPICAMERA_COMMAND_PREVIEWENC;
    	}else if (strList[i] == "-ih"){
    		//l_timeout= l_strList[i+1].c_str();
    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHOWSPSPPS].id = RPICAMERA_COMMAND_SHOWSPSPPS;
			//m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHOWSPSPPS].argv = (char *)"1";
			strncpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHOWSPSPPS].argv, (char *)"1", sizeof(m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHOWSPSPPS].argv));
			m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHOWSPSPPS].isNum = true;
    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_SHOWSPSPPS].isBooting = true;
    		pConfig->iChangedCommand_Boottime |= RPICAMERA_COMMAND_SHOWSPSPPS;
    	}else if (strList[i] == "-hf"){
    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_HFLIP].id = RPICAMERA_COMMAND_HFLIP;
			strcpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_HFLIP].argv, "1");
			m_TRpiCommandRecord[RPICAMERA_PARAMETER_HFLIP].isNum = true;
    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_HFLIP].isBooting = true;
    		pConfig->iChangedCommand_Boottime |= RPICAMERA_COMMAND_HFLIP;
    	}else if (strList[i] == "-vf"){
    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_VFLIP].id = RPICAMERA_COMMAND_VFLIP;
			strcpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_VFLIP].argv, "1");
			m_TRpiCommandRecord[RPICAMERA_PARAMETER_VFLIP].isNum = true;
    		m_TRpiCommandRecord[RPICAMERA_PARAMETER_VFLIP].isBooting = true;
    		pConfig->iChangedCommand_Boottime |= RPICAMERA_COMMAND_VFLIP;
    	}
    }
    //printf("D: [Config. Manager] raspivid start \n");
    //printf("D: [Vision] VISION_CAPTURE_FIFO = %s\n", VISION_CAPTURE_FIFO);
    //printf("D: [Vision] VISION_RAW_CAPTURE_FIFO = %s\n", VISION_RAW_CAPTURE_FIFO);
    //GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Config. Manager] diff. chk: %s\n", (l_iDiffCount >= 1)?"different with last record":"the same with before");
    if (l_iDiffCount >= 1) {
    	pConfig->bVisionParameterChanged = axc_true;
        if (pConfig->iChangedCommand_Boottime != RPICAMERA_COMMAND_BUNKNOWN) {
        	pConfig->bNeedRestart = axc_true;
        } else {
        	pConfig->bNeedRestart = axc_false;
        }
    } else {
    	pConfig->bVisionParameterChanged = axc_false;
    	pConfig->bNeedRestart = axc_false;
    }


	//dragon ++ <<

    //GLog(tAll, tDEBUGTrace_MSK, "D: [Config] [Gavin] Cheatfinderconfigmanager::ConfigFile_Read ^^\n");
	//
	// print options
	//
	if ((3 >= pConfig->dwLogLevel) && (0 < pConfig->dwLogLevel))
	{
		//GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Config] log_level = %u\n", pConfig->dwLogLevel);
		//GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Config] vision_capture_app = %s\n", pConfig->szVisionCaptureApp);
		//GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Config] open_stream = %s\r\n", getOpenStreamConfig());
		//GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Config] modbus_uart_baud = %u\n", pConfig->dwModbusUartBaud);
		//GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Config] modbus_uart_device_id = %u\n", pConfig->dwModbusUartDeviceId);
		//GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Config] thermal_extend_option = 0x%02x\n", pConfig->ucThermalOptional);
		//GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Config] thermal_heatobj_enable = %d\n", pConfig->bThermalHeatObj_Enable);
		if(CHeatFinderUtility::TemperatureToText(pConfig->dbThermalHeatObj_BackTempe, pConfig->iThermalHeatObj_BackTempeMode, szValue))
		{
			//GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Config] thermal_heatobj_back_tempe = %s\n", szValue);
		}
		if(CHeatFinderUtility::TemperatureToText(pConfig->dbThermalHeatObj_HeatTempe, pConfig->iThermalHeatObj_HeatTempeMode, szValue))
		{
			//GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [Config] thermal_heatobj_heat_tempe = %s\n", szValue);
		}
	}

	//GLog(tAll, tDEBUGTrace_MSK, "D: [Config] [Gavin] ---Cheatfinderconfigmanager::ConfigFile_Read (success)\n");
	return axc_true;
}

axc_bool Cheatfinderconfigmanager::ConfigFile_Write(const T_CONFIG_FILE_OPTIONS* pConfig, const char* szFile)
{
	if(NULL == szFile || 0 == szFile[0] || NULL == pConfig)
	{
    	//CHeatFinderUtility::PrintNowData(1);
		GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [Config] failed to write config-file: invalid parameters\n");
		errno = EINVAL;
		return axc_false;
	}

	// string
	CAxcString cszConfig;
	char szValue[1024];
	cszConfig.AppendFormat("log_level=%u\r\n", pConfig->dwLogLevel);
	cszConfig.AppendFormat("vision_capture_app=%s\r\n", pConfig->szVisionCaptureApp);
	cszConfig.AppendFormat("open_stream=%s\r\n", getOpenStreamConfig());
	cszConfig.AppendFormat("modbus_uart_baud=%u\r\n", pConfig->dwModbusUartBaud);
	cszConfig.AppendFormat("modbus_uart_device_id=%u\r\n", pConfig->dwModbusUartDeviceId);
	cszConfig.AppendFormat("thermal_extend_option=%u\r\n", pConfig->ucThermalOptional);
	GLog(tAll, tDEBUGTrace_MSK, "D: [%s] %s [Gavin]write 'thermal_extend_option=%d' to adehf.ini\n", "Config. Manager", __func__, pConfig->ucThermalOptional);
	cszConfig.AppendFormat("thermal_heatobj_enable=%d\r\n", pConfig->bThermalHeatObj_Enable);
	if(CHeatFinderUtility::TemperatureToText(pConfig->dbThermalHeatObj_BackTempe, pConfig->iThermalHeatObj_BackTempeMode, szValue))
	{
		cszConfig.AppendFormat("thermal_heatobj_back_tempe=%s\r\n", szValue);
	}
	if(CHeatFinderUtility::TemperatureToText(pConfig->dbThermalHeatObj_HeatTempe, pConfig->iThermalHeatObj_HeatTempeMode, szValue))
	{
		cszConfig.AppendFormat("thermal_heatobj_heat_tempe=%s\r\n", szValue);
	}

	if(1)
	{
		GLog(tAll, tDEBUGTrace_MSK, "D: [%s] %s [Gavin]write 'thermal_mask_bits' to adehf.ini\n", "Config. Manager", __func__);
		cszConfig.Append("thermal_mask_bits=");
		const int iBytes = sizeof(pConfig->ThermalMask_Bits);
		axc_byte* pbyBits = (axc_byte*) pConfig->ThermalMask_Bits;
		const char* pszValue = szValue;
		char szTemp[64] = {0,};
		for(int i=0; i<iBytes; i++, pbyBits++, pszValue+=2)
		{
			sprintf(szTemp, "%02X", *pbyBits);
			cszConfig.Append(szTemp);
		}
		cszConfig.Append("\r\n");
	}

	// open file, write string to file
	CAxcFile file("config_file/read");
	if(!file.Open(szFile,"wb"))
	{
		GLog(tCONFIGMANAGERTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Config] failed to write config-file: %s\n", AxcGetLastErrorText());
		return axc_false;
	}
	file.Write(cszConfig.Get(), cszConfig.GetLength());
	file.Close();

	return axc_true;
}

char* Cheatfinderconfigmanager::getOpenStreamConfig(){
	static char szRst[128] = {'\0'};  //gavin modify
	const T_CONFIG_FILE_OPTIONS* pConfig = GetConfigContext();

	switch (pConfig->iOpenStream){
	case OPEN_CAMERA_STREAM:
		strncpy(szRst, "vision", sizeof(szRst));
		break;
	case OPEN_THERMAL_STREAM:
		strncpy(szRst, "thermal", sizeof(szRst));
		break;
	case (OPEN_CAMERA_STREAM | OPEN_THERMAL_STREAM):
		strncpy(szRst, "vision,thermal", sizeof(szRst));
		break;
	default:
		break;
	}
	return szRst;
}

void Cheatfinderconfigmanager::ResetFlipFlag()
{
	// Reset hflip & vflip
	m_TRpiCommandRecord[RPICAMERA_PARAMETER_HFLIP].id = RPICAMERA_COMMAND_HFLIP;
	strcpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_HFLIP].argv, "0");
	m_TRpiCommandRecord[RPICAMERA_PARAMETER_HFLIP].isNum = true;
	m_TRpiCommandRecord[RPICAMERA_PARAMETER_HFLIP].isBooting = true;

	m_TRpiCommandRecord[RPICAMERA_PARAMETER_VFLIP].id = RPICAMERA_COMMAND_VFLIP;
	strcpy(m_TRpiCommandRecord[RPICAMERA_PARAMETER_VFLIP].argv, "0");
	m_TRpiCommandRecord[RPICAMERA_PARAMETER_VFLIP].isNum = true;
	m_TRpiCommandRecord[RPICAMERA_PARAMETER_VFLIP].isBooting = true;
}
