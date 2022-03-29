#include "pluginmanager.h"
#include <string>
#include <dirent.h>
#include "adhfpluginloader.h"
#include "heatfinderlogmanager.h"
#include "globaldef.h"


CPluginManager::CPluginManager():
	m_hThread_TX("timer/tx")
{
	m_bThreadStop = false;
}

CPluginManager::~CPluginManager()
{
    Stop();
}

int CPluginManager::Init(){
	//m_TxSupport.AddVisionH264DataReceivedNotifyEvent();
	//m_TxSupport.AddVisionRawDataReceivedNotifyEvent();
	m_TxSupport.AddThermalDataReceivedNotifyEvent(SetReceivedThermalFrameCB, this);

	return 0;
}

int CPluginManager::Run(){
	// add thread for .... m_TxSupport.getOneOfAlreadyStoredFramesPerQueue();
	Init();
	bool _bRst = true;

    if (!m_hThread_TX.Create(thread_process, this))
    	_bRst = false;

	return (_bRst == true)?0:1; //true: 0
}

void CPluginManager::Stop(){
	m_bThreadStop = true;

    if (m_hThread_TX.IsValid()){
    	m_hThread_TX.Destroy(2000);
    }

    for(unsigned int i = 0; i < m_pluginList.size(); ++i)
    {
        adhfplugin_loader *plugin = m_pluginList[i];
        delete plugin;
    }
    m_pluginList.clear();

}

int CPluginManager::LoadPluginFromPath(char *pPath)
{

    std::string szPath(pPath);

    DIR *pD = NULL;
    struct dirent *pDir = NULL;
    pD = opendir(pPath);
    if (pD)
    {
        while((pDir = readdir(pD)) != NULL)
        {
            std::string szFile(pDir->d_name);
            if (szFile.compare(".") != 0 && szFile.compare("..") != 0)
            {
                //find plugin
                std::string szFilename = szPath + "/" + szFile;
                if (szFilename.find(".so") == std::string::npos)
                    continue;
                adhfplugin_loader *pluginLoader = new adhfplugin_loader();
                CHeatFinderPlugin *pPlugin = pluginLoader->DynamicLoading((char*)szFilename.c_str());
                if (pPlugin)
                    m_pluginList.push_back(pluginLoader);
                else
                    delete pluginLoader;

            }

        }
        closedir(pD);
    }

    return m_pluginList.size();
}

int CPluginManager::GetPluginsRequestSteam()
{
    int iRst = 0;
    for(unsigned int i = 0; i < m_pluginList.size(); ++i)
    {
        adhfplugin_loader *plugin = m_pluginList[i];
        iRst = iRst | plugin->m_pHeatFinder->GetRequestSteam();
    }

    return iRst;
}

int CPluginManager::GetPluginCount()
{
    return m_pluginList.size();
}

// mark.hsieh ++
int CPluginManager::GetPlugInInformation(){
	int _iRst = 0;
	unsigned int PluginNumber = m_pluginList.size();
	xPluginInfo InfoList[PluginNumber];

	GLog(tPLUGINTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: %u Plugins information list:\n", PluginNumber);
    for(unsigned int i = 0; i < m_pluginList.size(); ++i)
    {
        adhfplugin_loader *plugin = m_pluginList[i];
        _iRst = _iRst | plugin->m_pHeatFinder->GetPlugInInformation(&InfoList[i]);

        GLog(tPLUGINTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%u] Short Name: %s, Version: %s, Description: %s\n",
        		i, InfoList[i].ShortName, InfoList[i].Version, InfoList[i].Description);
    }

    return _iRst;
}

int CPluginManager::CopyPlugInInformation(char *szString, unsigned int iLength){
	int _iRst = 0;
	unsigned int PluginNumber = m_pluginList.size();
	xPluginInfo InfoList[PluginNumber];

	unsigned int _iWritedLength = 0;
	unsigned int _iInformationLength = 0;

	GLog(tPLUGINTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: %u Plugins information list:\n", PluginNumber);
    for(unsigned int i = 0; i < m_pluginList.size(); ++i)
    {
        adhfplugin_loader *plugin = m_pluginList[i];
        _iRst = _iRst | plugin->m_pHeatFinder->GetPlugInInformation(&InfoList[i]);

        _iInformationLength = strlen(InfoList[i].ShortName) + strlen(InfoList[i].Version) + 3; //append '.', ';' & '\0'
        if ((_iWritedLength + _iInformationLength) >= iLength){
        	_iRst = EIO;

        	GLog(tPLUGINTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: plugins version temporary string is too short. (%d)\n", iLength);
        	break;
        }else{
        	char Version[_iInformationLength + 3]; //append '.', ';' & '\0'
        	sprintf(Version, "%s.%s;", InfoList[i].ShortName, InfoList[i].Version);

        	strncat(szString+_iWritedLength, Version, strlen(Version)); // '\0' might be cut-off
        	_iWritedLength += (strlen(Version));
        }
    }

    return _iRst;
}

int CPluginManager::SetReceivedH264Frame(const int BufSize, unsigned char *pBuf)
{
    for(unsigned int i = 0; i < m_pluginList.size(); ++i)
    {
        adhfplugin_loader *plugin = m_pluginList[i];
        if (plugin->m_pHeatFinder->GetRequestSteam() & REQ_STREAM_H264)
        {
            plugin->m_pHeatFinder->OnReceivedH264Frame(BufSize, pBuf);
        }
    }

    return 0; //true
}

int CPluginManager::SetReceivedThermalFrame(const int FrameWidth, const int FrameHeight, bool IsCompress, const int BufSize, short *pBuf)
{
	//printf("D: PluginManager: %s has plugin count: %u\n", __func__, m_pluginList.size());
    for(unsigned int i = 0; i < m_pluginList.size(); ++i)
    {
        adhfplugin_loader *plugin = m_pluginList[i];
        if (plugin->m_pHeatFinder->GetRequestSteam() & REQ_STREAM_THERMAL)
        {
        	//printf("D: PluginManager: %s index:%u pass steam\n", __func__, i);
            plugin->m_pHeatFinder->OnReceivedThermalFrame(FrameWidth, FrameHeight, IsCompress, BufSize, pBuf);
        }
    }

    return 0; //true
}

int CPluginManager::SetReceivedRGBFrame(const int Width, const int Height, const int RawFMT, const int size, unsigned char *pBuf)
{
    for (unsigned int i = 0; i < m_pluginList.size(); ++i) {
        adhfplugin_loader *plugin = m_pluginList[i];
        if (plugin->m_pHeatFinder->GetRequestSteam() & REQ_STREAM_RGB) {
            plugin->m_pHeatFinder->OnReceivedRGBFrame(Width, Height, RawFMT, size, pBuf);
        }
    }

    return 0; //true
}

int CPluginManager::SetThermalOverlay(xLeptonOverlay overlay)
{
	//printf("D: PluginManager: %s has plugin count: %u\n", __func__, m_pluginList.size());
    for (unsigned int i = 0; i < m_pluginList.size(); ++i) {
        adhfplugin_loader *plugin = m_pluginList[i];
        plugin->m_pHeatFinder->SetThermalOverlay(overlay);
    }

    return 1; //true
}

// mark.hsieh ++
int CPluginManager::SetReceivedHeatObject(xPluginHeatObject *pHeatObjs)
{
    for (unsigned int i = 0; i < m_pluginList.size(); ++i) {
        adhfplugin_loader *plugin = m_pluginList[i];
        //if ((ThermalHeatObject & plugin->m_pHeatFinder->GetPlugInExtendSupport()) > 0)
        	plugin->m_pHeatFinder->OnReceivedHeatObject(pHeatObjs);
    }

    return 1; //true
}

int CPluginManager::AddReceivedThermalFrame(const int FrameWidth, const int FrameHeight, bool IsCompress, const int BufSize, short *pBuf)
{
	if (m_bThreadStop) {
		return EPERM;  /* Operation not permitted */
	}

	int _iRst = 0;
	_iRst = m_TxSupport.AddReceivedThermalFrame(FrameWidth, FrameHeight, IsCompress, BufSize, pBuf);

	if (_iRst == SUPPORT) {
		return 0; //no error
	} else if (_iRst == NOTSUPPORT) {
		return EPROTONOSUPPORT;  /* Protocol not supported */
	} else {
		// IOERROR
		return EIO;  /* I/O error */
	}
}

axc_dword CPluginManager::thread_process()
{
	//GLog(tPLUGINTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [%s] thread start\n", "Plugin Manager");

    void *pContext = CHeatFinderUtility::GetGlobalsupport();
    GlobalDef *pGlobal = reinterpret_cast<GlobalDef*> (pContext);
    pGlobal->AddpThreadRecord("Plugin Tx");

    short _iDelayedReference = 0;
	while(!m_bThreadStop){
		_iDelayedReference = m_TxSupport.getOneOfAlreadyStoredFramesPerQueue();

		switch(_iDelayedReference){
		case 3:
			m_hThread_TX.SleepMilliSeconds(5);
			//usleep(5*1000);
		case 2:
			m_hThread_TX.SleepMilliSeconds(5);
			//usleep(5*1000);
		case 1:
			m_hThread_TX.SleepMilliSeconds(5);
			//usleep(10*1000);
			break;
		default:
			break;
		}
	}
	return 1;
}
