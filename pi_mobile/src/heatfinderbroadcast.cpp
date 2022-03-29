#include "heatfinderbroadcast.h"

CHeatFinderBroadcast::CHeatFinderBroadcast():
	m_caxcThread("broadcast/renew"),
    m_bStopThread(true)
{
    memset(m_szFW_Version, 0, FW_Version_Length);
    memset(m_szDev_Default_Name, 0, Dev_Default_Name_Length);
    memset(m_szDev_Type, 0, Dev_Type_Length);
    memset(m_szAppBuildTime, 0, 128);
    memset(m_abyMacAddress_eth0, 0, 6);
    memset(m_abyMacAddress_wlan0, 0, 6);
    m_bSocketUDPEnalbeStatus = false;
    m_sizeThermalImage.cx = 0;
    m_sizeThermalImage.cy = 0;
    m_sizeVisionMainImage.cx = 0;
    m_sizeVisionMainImage.cy = 0;

    const char szTime[] = { __TIME__ };
    char szDate[64] = { __DATE__ };
    struct tm tmDate;
    memset(&tmDate, 0, sizeof(tmDate));
    if(NULL != strptime(szDate, "%b %d %Y", &tmDate))
    {
        sprintf(szDate, "%04u%02u%02u", tmDate.tm_year+1900, tmDate.tm_mon+1, tmDate.tm_mday);
    }
    sprintf(m_szAppBuildTime, "%s %s", szDate, szTime);

    SetfwVersion(SYS_EXECUTE_VERSION);
    Setdevdefaultname(DEVICE_LABEL);
    SetdevType(DEVICE_TYPE);

    sem_init( &requestOutput, 0, 0 );
}

CHeatFinderBroadcast::~CHeatFinderBroadcast()
{
	Stop();

    sem_destroy(&requestOutput);
    GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] Destroy \n", "Broadcast msg.");
}

void CHeatFinderBroadcast::SetfwVersion(const char *fw_version)
{
    CAxcString::strncpy(m_szFW_Version, fw_version, FW_Version_Length);
}

void CHeatFinderBroadcast::Setdevdefaultname(const char *default_name)
{
    CAxcString::strncpy(m_szDev_Default_Name, default_name, Dev_Default_Name_Length);
}

void CHeatFinderBroadcast::SetdevType(const char *devtype)
{
    CAxcString::strncpy(m_szDev_Type, devtype, Dev_Type_Length);
}

void CHeatFinderBroadcast::SetSocketUDPEnalbeStatus(const bool value)
{
    m_bSocketUDPEnalbeStatus = value;
}

bool CHeatFinderBroadcast::Run()
{
    bool bRst = false;
    Stop();

    getStableSystemInformation();

    m_bStopThread = false;
    if (!m_caxcThread.Create(thread_process, this))
        return bRst;

    bRst = true;
    return bRst;
}

void CHeatFinderBroadcast::Stop()
{
    m_bStopThread = true;
    if (m_caxcThread.IsValid()){
    	sem_post(&requestOutput); //avoid blocking at sem_wait()
    	m_caxcThread.Destroy(2000);
    }
    GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] Stop\n", "Broadcast msg.");
}

void CHeatFinderBroadcast::AddBroadcastInfoChangedNotifyEvent(OnBroadcastInfoChangedNotify fnEvent, void *pContext)
{
    xBroadcastInfoChangedNotifyHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;
    m_BroadcastInfoNotifyList.push_back(handler);
}

void CHeatFinderBroadcast::AddBroadcastInfoRenewNotifyEvent(BroadcastValueRenewHandler objEvent, void *pContext)
{
	xReceiveBroadcastRenewNotifyHandler handler;
    handler.objEvent = objEvent;
    handler.pContext = pContext;
    m_ReceivedBroadcastRenewNotifyList.push_back(handler);
    sem_post(&requestOutput);
}

void CHeatFinderBroadcast::updatePluginInfomations(char *szString, unsigned int iLength){
	m_PluginInformation_locker.lock();
	m_cszPluginsVersion.Set(szString, iLength);

	m_PluginInformation_locker.unlock();
}

void CHeatFinderBroadcast::getStableSystemInformation(){

    FILE *l_fp = NULL;  //hamlet
    char L_token[128] = {""};
    char L_token2[128] = {""};

    if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/home/pi/heat-finder-runScripts/rest-server/config/camera_config.json"))) {
        l_fp = popen("jq -r .pid /home/pi/heat-finder-runScripts/rest-server/config/camera_config.json | sed 's/-//g'", "r");
    }else if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-merge/config/camera_config.json"))) {
        l_fp = popen("jq -r .pid /home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-merge/config/camera_config.json | sed 's/-//g'", "r");
    }else if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-release/config/camera_config.json"))) {
        l_fp = popen("jq -r .pid /home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-release/config/camera_config.json | sed 's/-//g'", "r");
    }

    if (l_fp == NULL) {
    	//CHeatFinderUtility::PrintNowData(1);
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] %s: failed to get manual dev. name (short), error:[%d] %s\n", "Broadcast msg.", __func__, errno, "can not read string by popen()");
        m_cszLessName.AppendFormat("Unknown");
    }
    else
    {
          while (fgets(L_token2, sizeof(L_token2)-1, l_fp) != NULL) {
              char *pos;
              if ((pos=strchr(L_token2, '\n')) != NULL)
              {    *pos = '\0';}
              if (strlen(L_token2))
              {
                  //cszLessName =L_token2;
            	  m_cszLessName.Append(L_token2, strlen(L_token2));
              }
          }

          pclose(l_fp);//hamlet
    }

    L_token[0] = '\0';
    if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/home/pi/heat-finder-runScripts/rest-server/config/camera_config.json"))) {
            l_fp = popen("jq -r .pid /home/pi/heat-finder-runScripts/rest-server/config/camera_config.json", "r");
    }else if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-merge/config/camera_config.json"))) {
        l_fp = popen("jq -r .pid /home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-merge/config/camera_config.json", "r");
    }else if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-release/config/camera_config.json"))) {
        l_fp = popen("jq -r .pid /home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-release/config/camera_config.json", "r");
    }

    if (l_fp == NULL) {
    	//CHeatFinderUtility::PrintNowData(1);
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] %s: failed to get manual dev. pid, error:[%d] %s\n", "Broadcast msg.", __func__, errno, "can not read string by popen()");
        m_cszManualPID.AppendFormat("Unknown");
    }
    else
    {
          while (fgets(L_token, sizeof(L_token)-1, l_fp) != NULL) {
              char *pos;
              if ((pos=strchr(L_token, '\n')) != NULL)
              {    *pos = '\0';}
              if (strlen(L_token))
              {
                  //cszManualPID =L_token;
            	  m_cszManualPID.Append(L_token, strlen(L_token));
              }
          }
          pclose(l_fp); //hamlet
    }


    L_token[0] = '\0';
    if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/home/pi/heat-finder-runScripts/rest-server/package.json"))) {
            l_fp = popen("jq .version /home/pi/heat-finder-runScripts/rest-server/package.json | sed s'/\"//g'", "r");
    }else if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-merge/package.json"))) {
        l_fp = popen("jq .version /home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-merge/package.json | sed s'/\"//g'", "r");
    }else if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-release/package.json"))) {
        l_fp = popen("jq .version /home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-release/package.json | sed s'/\"//g'", "r");
    }

    if (l_fp == NULL) {
    	//CHeatFinderUtility::PrintNowData(1);
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] %s: failed to get manual remote control service version, error:[%d] %s\n", "Broadcast msg.", __func__, errno, "can not read string by popen()");
        m_cszRemoteCtlSerVersion.AppendFormat("Unknown");
    }
    else
    {
          while (fgets(L_token, sizeof(L_token)-1, l_fp) != NULL) {
              char *pos;
              if ((pos=strchr(L_token, '\n')) != NULL)
              {    *pos = '\0';}
              if (strlen(L_token))
              {
                  //cszRemoteCtlSerVersion =L_token;
            	  m_cszRemoteCtlSerVersion.Append(L_token, strlen(L_token));
              }
          }

          pclose(l_fp); //hamlet
    }

    l_fp = NULL;    //hamlet
    L_token[0] = '\0';
    if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-merge/config/versionCurrent.json"))) {
            l_fp = popen("jq .overall /home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-merge/config/versionCurrent.json | sed s'/\"//g'", "r");
    }else if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-merge/config/versionRecord.json"))) {
        l_fp = popen("jq .overall /home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-merge/config/versionRecord.json | sed s'/\"//g'", "r");
    }

    if (l_fp == NULL) {
    	//CHeatFinderUtility::PrintNowData(1);
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] %s: failed to get total update version, error:[%d] %s\n", "Broadcast msg.", __func__, errno, "can not read string by popen()");
        m_cszTotalVersion.AppendFormat("Unknown");
    }
    else
    {
          while (fgets(L_token, sizeof(L_token)-1, l_fp) != NULL) {
              char *pos;
              if ((pos=strchr(L_token, '\n')) != NULL)
              {    *pos = '\0';}
              if (strlen(L_token))
              {
                  //cszTotalVersion =L_token;
            	  m_cszTotalVersion.Append(L_token, strlen(L_token));
              }
          }

          pclose(l_fp); //hamlet
    }

}

void CHeatFinderBroadcast::getCurrentSystemInformation(){

	 m_locker.lock();

	// mac-address string
	m_cszEthZeroDividedMac.Format("%02X:%02X:%02X:%02X:%02X:%02X",
			m_abyMacAddress_eth0[0], m_abyMacAddress_eth0[1], m_abyMacAddress_eth0[2],
			m_abyMacAddress_eth0[3], m_abyMacAddress_eth0[4], m_abyMacAddress_eth0[5]);
	//printf("D: [%s] %s: mac string is: %s \n", "Broadcast msg.", __func__, m_cszEthZeroDividedMac.Get());

	m_cszEthZeroSequentialMac.Format("%02X%02X%02X%02X%02X%02X",
			m_abyMacAddress_eth0[0], m_abyMacAddress_eth0[1], m_abyMacAddress_eth0[2],
			m_abyMacAddress_eth0[3], m_abyMacAddress_eth0[4], m_abyMacAddress_eth0[5]);
	m_cszWlanZeroSequentialMac.Format("%02X%02X%02X%02X%02X%02X",
			m_abyMacAddress_wlan0[0], m_abyMacAddress_wlan0[1], m_abyMacAddress_wlan0[2],
			m_abyMacAddress_wlan0[3], m_abyMacAddress_wlan0[4], m_abyMacAddress_wlan0[5]);

    // get new name
    // jq -r .name /home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-merge/config/camera_config.json
    // cat /home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-merge/config/camera_config.json | grep name | head -1 | sed 's/"//g' | sed 's/,//g' | sed 's/^.*name://g' | sed 's/ //g'
    FILE *l_fp = NULL;  //hamlet
    char L_token[128] = {""};
	if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/home/pi/heat-finder-runScripts/rest-server/config/camera_config.json"))) {
		l_fp = popen("jq -r .name /home/pi/heat-finder-runScripts/rest-server/config/camera_config.json | sed 's/HF048-P/HF048/g'", "r");
	}else if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-merge/config/camera_config.json"))) {
	    l_fp = popen("jq -r .name /home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-merge/config/camera_config.json | sed 's/HF048-P/HF048/g'", "r");
	}else if (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-release/config/camera_config.json"))) {
	    l_fp = popen("jq -r .name /home/pi/heat-finder-runScripts/rest-server/Heat-finder-server-release/config/camera_config.json | sed 's/HF048-P/HF048/g'", "r");
	}

	bool _bReadCameraConfigurationFail = false;
    if (l_fp == NULL) {
    	//CHeatFinderUtility::PrintNowData(1);
        GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] %s: failed to get manual dev. name, error:[%d] %s\n", "Broadcast msg.", __func__, errno, strerror(errno));//"can not read string by popen()");
        //m_cszManualName.AppendFormat("Unknown");
        _bReadCameraConfigurationFail = true;
    }
    else
    {
          while (fgets(L_token, sizeof(L_token)-1, l_fp) != NULL) {
              char *pos;
              if ((pos=strchr(L_token, '\n')) != NULL)
              {    *pos = '\0';}
              if (strlen(L_token))
              {
            	  m_cszManualName.Set(L_token, strlen(L_token));
              }
          }
          pclose(l_fp);     //hamlet
    }

    bool _bCheckTemporaryNameFail = false;
    if (_bReadCameraConfigurationFail){
    	_bCheckTemporaryNameFail = (axc_true ==(CAxcFileSystem::AccessCheck_IsExisted("/tmp/device_label")))?false:true;
    }else{

    	m_locker.unlock();
    	//pass and leave
    	return;
    }

	if (_bCheckTemporaryNameFail){
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] %s: no data before, using default context=%s\n", "Broadcast msg.", "device_label", m_szDev_Default_Name);
		m_cszManualName.Format("%s", m_szDev_Default_Name);

		system("sudo touch /tmp/device_label");
		system("sudo chmod +rw /tmp/device_label");
		system("echo 'AD-HF048' > /tmp/device_label");
	}else{
		FILE *fp = NULL;    //hamlet
		char name_label[512] = {""};
		char name_label_remove_newline[512] = {""};
		char token[128] = {""};
		fp = popen("cat /tmp/device_label", "r");

		if (fp == NULL)
		{
			//CHeatFinderUtility::PrintNowData(1);
			GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] %s: failed to get device_label, error:[%d] %s\n", "Broadcast msg.", __func__, errno, strerror(errno));//"can not read string by popen()");
			m_cszManualName.Format("%s", m_szDev_Default_Name);
		}
		else
		{
			  while (fgets(token, sizeof(token)-1, fp) != NULL) {
				  CAxcString::strncat(name_label, token, sizeof(name_label));
			  }
			  name_label[strlen(name_label)] = '\0';
			  if(CHeatFinderUtility::string_replace(name_label, name_label_remove_newline, (char)sizeof(name_label_remove_newline), "\n", ""))
			  {
				  m_cszManualName.Format("%s", name_label_remove_newline);
				  //printf("D: [Net] %s: take %s success, using length=%d context=%s", "Broadcast msg.", __func__, "device_label", strlen(name_label_remove_newline), name_label_remove_newline);
			  }
			  else
			  {
				  m_cszManualName.Format("%s", name_label);
				  //printf("D: [Net] %s: take %s success, using length=%d context=%s", "Broadcast msg.", __func__, "device_label", strlen(name_label), name_label);
			  }

			  pclose(fp);   //hamlet
		}

	}

	 m_locker.unlock();
}

bool CHeatFinderBroadcast::GetLangChar(char *lang, uint32_t len)
{
    if (len < 3)
        return false;

    int32_t iReadSize = adethermal_device_get_parameters("Lang", lang, sizeof(lang));
    if (iReadSize <= 0)
    {
        // GLog(tThermalSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [Thermal_SDK] getLangParameter: error occur, adethermal_device_get_parameters return %d\n", iReadSize);
        return false;
    }
    else 
        return true;  
}

void CHeatFinderBroadcast::UpdateBroadcastInfo()
{
    m_locker.lock();
    /**if (false == m_locker.try_lock()){
    	return;
    }**/

    axc_bool rst_of_IpToString = axc_true;

    CAxcString cszIP;
    if (rst_of_IpToString == axc_true)
    {
        cszIP.Format("%s",m_cszETH0IP.Get());
    }
    else
    {
        cszIP.Append("0.0.0.0");
    }

    //char t_IP2STR2[32];
    axc_bool rst_of_IpToString2 = axc_true;
    //rst_of_IpToString2 = CAxcSocket::IpToString(l_b_WLAN0IP, t_IP2STR2);
    CAxcString cszIP2;
    if (rst_of_IpToString2 == axc_true)
    {
        cszIP2.Format("%s",m_cszWLAN0IP.Get());
    }
    else
    {
        cszIP2.Append("0.0.0.0");
    }

    CAxcString cszBcInfo;
    cszBcInfo.Append("command=node_v2\r\n");
    cszBcInfo.Append("factory=ADE\r\n");
    if (m_cszManualPID.GetLength() > 0){
        cszBcInfo.AppendFormat("type=%s\r\n", m_cszManualPID.Get());
    }else{
        cszBcInfo.AppendFormat("type=%s\r\n", m_szDev_Type);
    }
    cszBcInfo.AppendFormat("fw_version=%s build %s\r\n", m_szFW_Version, m_szAppBuildTime);

    AXC_U_VERSION  axcUVxValue = AxcGetLibVersion();
    cszBcInfo.AppendFormat("thermal_sdk_version=%d.%d_%d\r\n", axcUVxValue.Version.wMajor, axcUVxValue.Version.wMinor, axcUVxValue.dwValue);

    char LangValue[3] = {0};
    bool bRst = GetLangChar(LangValue, 3);  //ken
    cszBcInfo.AppendFormat("lang=ww\r\n"/*, LangValue*/);

    if (m_cszRemoteCtlSerVersion.GetLength() > 0){
        cszBcInfo.AppendFormat("remote_ctl_server_version=%s\r\n", m_cszRemoteCtlSerVersion.Get());
    }else{
        cszBcInfo.Append("remote_ctl_server_version=Unknown\r\n");
    }

    if (m_cszTotalVersion.GetLength() > 0){
        cszBcInfo.AppendFormat("total_version=%s\r\n", m_cszTotalVersion.Get());
    }else{
        cszBcInfo.Append("total_version=Unknown\r\n");
    }

    if ((m_cszPluginsVersion.GetLength() > 0) &&
    		(m_PluginInformation_locker.try_lock())){
    	cszBcInfo.AppendFormat("plugin_version=%s\r\n", m_cszPluginsVersion.Get());
		m_PluginInformation_locker.unlock();
    }

    if (m_cszLessName.GetLength() > 0){
        cszBcInfo.AppendFormat("hw_id=%s-%s\r\n", m_cszLessName.Get(), m_cszEthZeroSequentialMac.Get());
    }else{
        cszBcInfo.AppendFormat("hw_id=%s-%s\r\n", m_szDev_Type, m_cszEthZeroSequentialMac.Get());
    }
    cszBcInfo.AppendFormat("eth0_mac=%s\r\n", m_cszEthZeroDividedMac.Get());
    cszBcInfo.AppendFormat("mac=%s\r\n", m_cszEthZeroDividedMac.Get());
    cszBcInfo.AppendFormat("eth0_ip=%s\r\n", cszIP.Get());
    cszBcInfo.AppendFormat("wlan0_mac=%s\r\n", m_cszWlanZeroSequentialMac.Get());
    cszBcInfo.AppendFormat("wlan0_ip=%s\r\n", cszIP2.Get());

    if (m_cszManualName.GetLength() > 0){
    	cszBcInfo.AppendFormat("device_label=%s\r\n", m_cszManualName.Get());
    }else{
    	cszBcInfo.Append("device_label=Unknown\r\n");
    }

    int iStreamNumber = 1;


    cszBcInfo.AppendFormat("ch1_stream1_res_list=lepton/%dx%d;\r\n", m_sizeThermalImage.cx, m_sizeThermalImage.cy);
    cszBcInfo.AppendFormat("ch1_stream1_res=lepton/%dx%d\r\n", m_sizeThermalImage.cx, m_sizeThermalImage.cy);
    if(m_sizeVisionMainImage.cx != 0 && m_sizeVisionMainImage.cy != 0)
    {
        cszBcInfo.Append("ch1_type=video/lepton;video/h264;\r\n");
        iStreamNumber++;
        cszBcInfo.AppendFormat("ch1_stream2_res_list=h264/%dx%d;\r\n", m_sizeVisionMainImage.cx, m_sizeVisionMainImage.cy);
        cszBcInfo.AppendFormat("ch1_stream2_res=h264/%dx%d\r\n", m_sizeVisionMainImage.cx, m_sizeVisionMainImage.cy);
    }
    else
    {
        cszBcInfo.Append("ch1_type=video/lepton;\r\n");
    }
    cszBcInfo.Append("ch_num=1\r\n");
    cszBcInfo.AppendFormat("ch1_stream_num=%d\r\n",iStreamNumber);

    //printf("D: [%s] 3.\n", __func__);
    //if(g_CbProc_CBSU() == true)
    if(m_bSocketUDPEnalbeStatus == true)
    {
        cszBcInfo.AppendFormat("ch1_stream1_get=udp/%d;tcp/%d;\r\n", 5557, 5555); //UDP_VIDEO_PORT, TCP_LISTEN_PORT
    }
    else
    {
        cszBcInfo.AppendFormat("ch1_stream1_get=tcp/%d;\r\n", 5555); //TCP_LISTEN_PORT
    }

    cszBcInfo.Append("\r\n\0"); // End
    //printf("D: [%s] 4.\n", __func__);
    // update
    m_cszBroadcastInfo.Set(NULL);
    m_cszBroadcastInfo.Set(cszBcInfo.GetBuffer(), cszBcInfo.GetLength());
    //printf("D: [%s] 5.\n", __func__);
    //printf("D: [%s] %s list broadcast info. :\n %s", "Broadcast msg.", __func__, m_cszBroadcastInfo.Get());
    //printf("\n");

    m_locker.unlock();

}

void CHeatFinderBroadcast::UpdateNetInfo()
{
    m_locker.lock();
    /**if (false == m_locker.try_lock()){
    	return;
    }**/

    memset(m_abyMacAddress_eth0, 0, sizeof(m_abyMacAddress_eth0));
    memset(m_abyMacAddress_wlan0, 0, sizeof(m_abyMacAddress_wlan0));

    // get current info
    int skfd = -1;
    if((skfd=socket(AF_INET,SOCK_DGRAM,0)) >= 0)
    {
        int ret = 0;
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strcpy(ifr.ifr_name, "eth0");

        ret = ioctl(skfd,SIOCGIFHWADDR, &ifr);
        shutdown(skfd, 2);
        close(skfd);
        if(ret >= 0)
        {
            memcpy(m_abyMacAddress_eth0, (char*)&ifr.ifr_addr.sa_data, 6);
            //printf("D: [%s] we got eth0 mac %s\n", __func__, ifr.ifr_addr.sa_data);
        }else{
        	//printf("D: [%s] we lose eth0 mac\n", __func__);
        }
    }

    skfd = -1;
    if((skfd=socket(AF_INET,SOCK_DGRAM,0)) >= 0)
    {
        //int ret = 0;
        struct ifreq ifr;
        //ref.: https://stackoverflow.com/questions/2283494/get-ip-address-of-an-interface-on-linux?answertab=votes#tab-top
        /* I want to get an IPv4 IP address */
        memset(&ifr, 0, sizeof(ifr));
        ifr.ifr_addr.sa_family = AF_INET;

        /* I want IP address attached to "eth0" */
        strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
        //ret =
        ioctl(skfd, SIOCGIFADDR, &ifr);
        close(skfd);
        //if(ret >= 0)
        //{
            const char* t_StrIP = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
            m_cszETH0IP.Format("%s",t_StrIP);
            //printf("D >> [%s] eth0 ip: %s\n", "Broadcast msg.", __func__,l_b_ETH0IP.Get());
        //}
    }

    skfd = -1;
    if((skfd=socket(AF_INET,SOCK_DGRAM,0)) >= 0)
    {
        int ret = 0;
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strcpy(ifr.ifr_name, "wlan0");
        ret = ioctl(skfd,SIOCGIFHWADDR, &ifr);
        shutdown(skfd, 2);
        close(skfd);
        if(ret >= 0)
        {
            memcpy(m_abyMacAddress_wlan0, (char*)&ifr.ifr_addr.sa_data, 6);
            //printf("D: [%s] we got wlan0 mac %s\n", __func__, ifr.ifr_addr.sa_data);
        }else{
        	//printf("D: [%s] we lose wlan0 mac\n", __func__);
        }
    }

    skfd = -1;
    if((skfd=socket(AF_INET,SOCK_DGRAM,0)) >= 0)
    {
        //int ret = 0;
        struct ifreq ifr;
        /* I want to get an IPv4 IP address */
        memset(&ifr, 0, sizeof(ifr));
        ifr.ifr_addr.sa_family = AF_INET;

        /* I want IP address attached to "eth0" */
        strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ-1);
        //ret =
        ioctl(skfd, SIOCGIFADDR, &ifr);
        close(skfd);
        //if(ret >= 0)
        //{
            const char* t_StrIP = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
            m_cszWLAN0IP.Format("%s",t_StrIP);
            //printf("D >> [%s] wlan0 ip: %s\n", "Broadcast msg.", __func__,l_b_WLAN0IP.Get());
        //}
    }
    m_locker.unlock();

}

/*
 * net work checking
 *
 * baseon mark.hsieh 20170601
 *
 * int CheckLink(char *ifname);
 * bool IsUpLink(int if_flags);
 * bool IsRunningLink(int if_flags);
 * */

int CHeatFinderBroadcast::CheckLink(const char *ifname) {
    int state = 0;

    int skfd = -1;
	if((skfd=socket(AF_INET,SOCK_DGRAM,0)) >= 0)
	{
		int ret = 0;
		struct ifreq ifr;
		memset(&ifr, 0, sizeof(ifr));
		strcpy(ifr.ifr_name, ifname);
		ret = ioctl(skfd,SIOCGIFFLAGS, &ifr);

		close(skfd);
		if(ret >= 0)
		{
			state += ifr.ifr_flags;
		}
		else
		{
	    	//CHeatFinderUtility::PrintNowData(1);
			GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] %s cannot get '%s' device info.", "Broadcast msg.", __func__, ifname);
			state = -1;
		}
	}
	else
	{
    	//CHeatFinderUtility::PrintNowData(1);
		GLog(tUtilityTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] %s no any network service ... ", "Broadcast msg.", __func__);
		state = -1;
	}

	return state;
}
bool CHeatFinderBroadcast::IsUpLink(int if_flags) {
	bool state = false;
	if (if_flags & IFF_UP){
		state = true;
	}
	return state;
}
bool CHeatFinderBroadcast::IsRunningLink(int if_flags) {
	bool state = false;
	if (if_flags & IFF_RUNNING){
		state = true;
	}
	return state;
}

/**
 * mark.hsieh++
 * 		thread
 */
axc_dword CHeatFinderBroadcast::thread_process(){

    void *pContext = CHeatFinderUtility::GetGlobalsupport();
    GlobalDef *pGlobal = reinterpret_cast<GlobalDef*> (pContext);
    pGlobal->AddpThreadRecord("System Information Collector: broadcast");

	//axc_dword l_current_Time = CAxcTime::GetCurrentUTCTime32();
	axc_dword l_last_onSyncBroadcast_Time = CAxcTime::GetCurrentUTCTime32();
	axc_dword l_diff_lastSync_current_Time = 0;

	bool	l_chk_ExternalTaskExist = false;

	//Vision
	bool 	l_chk_TaskTypeVision = false;
	unsigned int V_w, V_h;
	V_w =0;
	V_h =0;
	//Thermal
	bool 	l_chk_TaskTypeThermal = false;
	unsigned int T_w, T_h;
	T_w =0;
	T_h =0;
	//Need Current Broadcast msg.
	bool 	l_chk_TaskTypeRenew = false;

    while(!m_bStopThread)
    {
		// check if has any req-event of list
    	// choice the last request
        unsigned int last_ops = 0;
        sem_wait(&requestOutput);

        if (m_ReceivedBroadcastRenewNotifyList.size() > 0){
        	//printf("D: [%s] broadcast got %d renew event\n", "Broadcast msg.", m_ReceivedBroadcastRenewNotifyList.size());
        }else{
        	m_caxcThread.SleepMilliSeconds(300);
        	continue;
        }

        for(unsigned int i = 0; i < m_ReceivedBroadcastRenewNotifyList.size(); ++i){
        	if (m_ReceivedBroadcastRenewNotifyList[i].objEvent.type == BroadcastValueRenew_Type_Vision
        			&& m_ReceivedBroadcastRenewNotifyList[i].objEvent.action == BroadcastValueRenew_Action_Resize){
        		l_chk_TaskTypeVision = (l_chk_TaskTypeVision == false)? true:l_chk_TaskTypeVision;
        		V_w =m_ReceivedBroadcastRenewNotifyList[i].objEvent.w;
        		V_h =m_ReceivedBroadcastRenewNotifyList[i].objEvent.h;

        	}else if (m_ReceivedBroadcastRenewNotifyList[i].objEvent.type == BroadcastValueRenew_Type_Thermal
        			&& m_ReceivedBroadcastRenewNotifyList[i].objEvent.action == BroadcastValueRenew_Action_Resize){
        		l_chk_TaskTypeThermal = (l_chk_TaskTypeThermal == false)? true:l_chk_TaskTypeThermal;
        		T_w =m_ReceivedBroadcastRenewNotifyList[i].objEvent.w;
        		T_h =m_ReceivedBroadcastRenewNotifyList[i].objEvent.h;

        	}else if (m_ReceivedBroadcastRenewNotifyList[i].objEvent.type == BroadcastValueRenew_Type_Renew){
        		l_chk_TaskTypeRenew = (l_chk_TaskTypeRenew == false)? true:l_chk_TaskTypeRenew;
            	//printf("N: [%s] broadcast got renew event: Task type\n", "Broadcast msg.");
        	}
        	last_ops++;
        }
        // erase the event "broadcast" receive 'now'
        m_ReceivedBroadcastRenewNotifyList.erase(m_ReceivedBroadcastRenewNotifyList.begin()+0, m_ReceivedBroadcastRenewNotifyList.begin()+last_ops);
        // change action if it be needed.
        l_chk_ExternalTaskExist = (l_chk_TaskTypeVision | l_chk_TaskTypeThermal | l_chk_TaskTypeRenew);

    	// renew/monitor control
    	//l_current_Time = CAxcTime::GetCurrentUTCTime32();
    	l_diff_lastSync_current_Time = CAxcTime::GetCurrentUTCTime32() - l_last_onSyncBroadcast_Time;
    	if (l_chk_ExternalTaskExist){
            //printf("D: [%s] %s manual renew device message: %s \n", "Broadcast msg.", __func__, ((l_chk_ExternalTaskExist == true)?"true":"false"));
    		l_chk_ExternalTaskExist = false; //job done

    		// task: renew
    		if (l_diff_lastSync_current_Time >= 2){
				UpdateNetInfo();
				getCurrentSystemInformation();
				l_last_onSyncBroadcast_Time = CAxcTime::GetCurrentUTCTime32();
    		}
			UpdateBroadcastInfo();

			// fire
    		if (l_chk_TaskTypeVision){
    			OnVisionSizeChangedNotify(V_w, V_h);
    			l_chk_TaskTypeVision = false;
    		}

    		if (l_chk_TaskTypeThermal){
    			OnThermalSizeChangedNotify(T_w, T_h);
    			l_chk_TaskTypeThermal = false;
    		}

    		if (l_chk_TaskTypeRenew){
    			//printf("D: [%s] %s we do l_chk_TaskTypeRenew at %lf\n", "Broadcast msg.", __func__, CAxcTime::GetCurrentUTCTimeMs());
    			OnGeneralInformationUpdateNotify();
    			l_chk_TaskTypeRenew = false;
    		}
    		l_last_onSyncBroadcast_Time = CAxcTime::GetCurrentUTCTime32();
#if false
    	}else if (l_diff_lastSync_current_Time >= 3){
			UpdateNetInfo();
			UpdateBroadcastInfo();
			l_last_onSyncBroadcast_Time = CAxcTime::GetCurrentUTCTime32();
			OnGeneralInformationUpdateNotify();
			//usleep(1000 * 1000);
			m_caxcThread.SleepMilliSeconds(100);
		}else{
			//usleep(20 * 1000);
			m_caxcThread.SleepMilliSeconds(20);

#endif
		}

    }
	return 1;
}

void CHeatFinderBroadcast::OnThermalSizeChangedNotify(const unsigned int cx, const unsigned cy)
{
    m_sizeThermalImage.cx = cx;
    m_sizeThermalImage.cy = cy;

    //UpdateBroadcastInfo();
    fireBroadcastNotify(m_cszBroadcastInfo);
}

void CHeatFinderBroadcast::OnVisionSizeChangedNotify(const unsigned int cx, const unsigned cy)
{
    m_sizeVisionMainImage.cx = cx;
    m_sizeVisionMainImage.cy = cy;

    //UpdateBroadcastInfo();
    fireBroadcastNotify(m_cszBroadcastInfo);
}

void CHeatFinderBroadcast::OnGeneralInformationUpdateNotify(/**const bool isLednotifyevent**/){
	//don't care isLednotifyevent now.

	fireBroadcastNotify(m_cszBroadcastInfo);
}

void CHeatFinderBroadcast::fireBroadcastNotify(CAxcString &BroadcastInfo)
{
	//printf("D: Size: %d\n", BroadcastInfo.GetLength());
	// because value change, need to send new value to Sender (help to update the record element of the member of Sender)
    for(unsigned int i = 0; i < m_BroadcastInfoNotifyList.size(); ++i)
    {
        xBroadcastInfoChangedNotifyHandler handler = m_BroadcastInfoNotifyList[i];
        handler.fnEvent(handler.pContext, BroadcastInfo);
    }
}
