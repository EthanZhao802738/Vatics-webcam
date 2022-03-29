#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <vector>
#include "adhfpluginloader.h"
#include "pluginDataTransmissionSupport.h"

using namespace std;

class CPluginManager
{
public:
    CPluginManager();
    ~CPluginManager();
    int Init();
    int Run();
    void Stop();

    int LoadPluginFromPath(char *pPath);
    int GetPluginsRequestSteam();
    int GetPluginCount();

    int GetPlugInInformation();
    int CopyPlugInInformation(char *szString, unsigned int iLength);

    int SetReceivedH264Frame(const int BufSize, unsigned char *pBuf);
    int SetReceivedThermalFrame(const int FrameWidth, const int FrameHeight, bool IsCompress, const int BufSize, short *pBuf);
    int SetReceivedRGBFrame(const int Width, const int Height, const int RawFMT, const int size, unsigned char *pBuf);
    int SetThermalOverlay(xLeptonOverlay overlay);

    int SetReceivedHeatObject(xPluginHeatObject *pHeatObjs);

    int AddReceivedThermalFrame(const int FrameWidth, const int FrameHeight, bool IsCompress, const int BufSize, short *pBuf);
    //int AddReceivedH264Frame(const int BufSize, unsigned char *pBuf);
    //int AddReceivedRGBFrame(const int Width, const int Height, const int RawFMT, const int size, unsigned char *pBuf);
protected:
    vector<adhfplugin_loader*> m_pluginList;
    CPluginDataTransmissionSupport m_TxSupport;

    static void SetReceivedThermalFrameCB(void *pContext, const int FrameWidth, const int FrameHeight, bool IsCompress, const int BufSize, short *pBuf)
    {
    	CPluginManager *pSender = reinterpret_cast<CPluginManager*> (pContext);
        pSender->SetReceivedThermalFrame(FrameWidth, FrameHeight, IsCompress, BufSize, pBuf);
    }

    static axc_dword thread_process(CAxcThread* pThread, void* pContext)
    {
    	CPluginManager *pSender = reinterpret_cast<CPluginManager*> (pContext);
        return pSender->thread_process();
    }
    axc_dword thread_process();

private:
    bool	m_bThreadStop;
    CAxcThread m_hThread_TX;
};

#endif // PLUGINMANAGER_H
