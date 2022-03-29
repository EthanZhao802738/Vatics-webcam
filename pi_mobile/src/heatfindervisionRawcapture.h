/*
 * heatfindervisionRawcapture.h
 *
 *  Created on: Jan 26, 2018
 *      Author: markhsieh
 */

#ifndef HEATFINDERVISIONRAWCAPTURE_H_
#define HEATFINDERVISIONRAWCAPTURE_H_

#include "axclib.h"
#include "ade_camera_sdk.h"
#include "globaldef.h"
#include <vector>

	using namespace std;



//callback
typedef int (*OnReceivedCameraRawFrame)(void *pOwner, const int Width, const int Height, unsigned char *pRGB);
typedef int (*OnSetThermalOverlay) (void *pOwner , xLeptonOverlay &xLoi);
typedef void (*onSyncVisionResolution) (void *pOwner,unsigned int &x, unsigned int &y);

//callback check
typedef bool (*OnChkisReqRGBStream) (void *pOwner);
typedef bool (*OnChkisSystemRun)(void *pOwner);
typedef int (*Onrenew_camera_rawfifo_fd) (void *pOwner);

typedef struct
{
	unsigned int cx;
	unsigned int cy;
} L_SIZE;


//typedef void (*OnVisionSizeChangedNotify)(void *pContext, const unsigned int Width, const unsigned int Height);
typedef struct tagOnReceivedCameraRawFrameNotifyHandler
{
	OnReceivedCameraRawFrame fnEvent;
    void *pContext;
}xOnReceivedCameraRawFrameNotifyHandler;

typedef struct tagSetThermalOverlayNotifyHandler
{
	OnSetThermalOverlay fnEvent;
    void *pContext;
}xSetOnTimeThermalOverlayNotifyHandler;

class CHeatfindervisionRawcapture {
public:
	CHeatfindervisionRawcapture();
	virtual ~CHeatfindervisionRawcapture();

	bool run();
	void stop();
	void setOwner(void *pOwner);

	//virtual
	//virtual int renew_camera_rawfifo_fd(ade_camera_sdk 	*l_obj) { };

	OnReceivedCameraRawFrame m_funcReceivedCameraRaw;
	OnSetThermalOverlay m_funcSetThermalOverlay;
	//OnChkisReqRGBStream m_funcChkisReqRGBStream;
	//OnChkisSystemRun m_funcChkisSystemRun;
	//onSyncVisionResolution m_funcSyncVisionResolution;
	//Onrenew_camera_rawfifo_fd m_funSyncRenewCamRawFD;

    void AddReceivedCameraRawNotify(OnReceivedCameraRawFrame fnEvent, void *pContext);
    void AddSetThermalOverlayNotify(OnSetThermalOverlay fnEvent, void *pContext);
    axc_dword thread_process();

protected:
	int		m_fdVisionRawFifo;
	L_SIZE	m_sizeVisionMainImage;
	void 	*m_pOwner;
	CAxcThread m_threadVisionRawCatch;
	bool m_bStopThread;

	std::vector<xOnReceivedCameraRawFrameNotifyHandler> m_ReceivedCameraRawnotifyhandlerList;
	std::vector<xSetOnTimeThermalOverlayNotifyHandler> m_SetThermalOverlaynotifyhandlerList;

    static axc_dword thread_process(CAxcThread* pThread, void* pContext)
    {
    	CHeatfindervisionRawcapture *pSender = reinterpret_cast<CHeatfindervisionRawcapture*> (pContext);
        return pSender->thread_process();
    }

    void OnRenewOverlay();

    void fireReceivedCameraRawFrameNotify(const int Width, const int Height, unsigned char *pRGB);
    void fireSetThermalOverlayNotify(xLeptonOverlay &xLoi);
private:
	void 	Raw_VisionCapture();
};


#endif /* HEATFINDERVISIONRAWCAPTURE_H_ */
