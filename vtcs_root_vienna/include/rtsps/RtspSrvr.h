/*
 *******************************************************************************
 *  Copyright (c) 2010-2015 VATICS Inc. All rights reserved.
 *
 *  +-----------------------------------------------------------------+
 *  | THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY ONLY BE USED |
 *  | AND COPIED IN ACCORDANCE WITH THE TERMS AND CONDITIONS OF SUCH  |
 *  | A LICENSE AND WITH THE INCLUSION OF THE THIS COPY RIGHT NOTICE. |
 *  | THIS SOFTWARE OR ANY OTHER COPIES OF THIS SOFTWARE MAY NOT BE   |
 *  | PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY OTHER PERSON. THE   |
 *  | OWNERSHIP AND TITLE OF THIS SOFTWARE IS NOT TRANSFERRED.        |
 *  |                                                                 |
 *  | THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT   |
 *  | ANY PRIOR NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY |
 *  | VATICS INC.                                                     |
 *  +-----------------------------------------------------------------+
 *
 *******************************************************************************
 */
#ifndef __RTSP_SERVER_H__
#define __RTSP_SERVER_H__

#include "LiveMediaSrc.h"
#include "RtpMapInfo.h"

/*! 
 *********************************************************************
 * The access file related information.
 * e.g. If you access the streaming by url rtsp://172.17.255.19/live1.sdp,
 * then the access name is live1.sdp.
 *********************************************************************
*/
typedef struct access_file_information
{
	//! The access file name.
	char* szAccessName;
	//! Which video track is related to this access file,
	//! set to -1 to disable video track.
	int		iVTrackNo;
	//! Which audio track is related to this access file,
	//! set to -1 to disable audio track.
	int		iATrackNo;
	//! Which mdata track is related to this aceess file,	
	//! set to -1 to disable mdata track.
	int		iDTrackNo;
	//! Which audio back channel track is related to this access file,
	//! set to -1 to disable audio back channel track.
	int		iAudioBackChannelTrackNo;

	//! The multicast enable/disable
	unsigned int bMltcstEnable;
	//! The multicast ip address of this access file,
	char* szMltcstIP;
	//! The Time To Live value of the multicast,
	//! this value is meanless when multicast is diabled.
	unsigned char byMltcstTTL;
	//! The multicast rtp port number of video track,
	//! this value is meanless when multicast is diabled or video is disabled.
	unsigned short usMltcstVideoPort;
	//! The multicast rtp port number of audio track,
	//! this value is meanless when multicast is diabled or audio is disabled.
	unsigned short usMltcstAudioPort;
	//! The multicast rtp port number of mdata track,
	//! this value is meanless when multicast is diabled or mdata is disabled.
	unsigned short usMltcstMDataPort;
	//! The multicast rtp port number of audio back channel track,
	//! this value is meanless when multicast is diabled or audio back channel is disabled.
	unsigned short usMltcstAudioBackChannelPort;
} TAcsFileInfo;
typedef TAcsFileInfo*	PTAcsFileInfo;

typedef struct live_msrc_init_opt
{
	char *szSharedBuffer;
	char *szCmdFiFoPath;
} TLiveMSrcInitOpt;

/*! 
 *********************************************************************
 * The RtspSrvr initial options.
 ********************************************************************* 
*/
typedef struct rtsp_server_initial_options
{
	//! The ip address of the machine,
	//! set to NULL to let RtspSrvr decide by itselt.
	const char		*szSrvIPAddr;
	//! The rtsp protocol listening port number.
	unsigned short usSrvListenPort;
	//! The starting port number of rtp over udp.
	unsigned short usRtpStartPort;
	unsigned short usBlockSize;
	unsigned short usTCPBlockSize;
	unsigned int max_conn_num;
	unsigned int httpserver_type;
	unsigned int bIPv6;
	//! The authentication mode, set to "none" or "basic", "digest" isn't supported now.
	//! \details See RFC 2617 HTTP Authentication: Basic and Digest Access Authentication.
	const char		*szAuthMode;
	//! The fdipc path with vatics's boa http server,
	//! set to NULL to disable rtsp over http or when you don't use vatics's modified boa http server.
	//! \note You must use vatics's modified boa http server to enable rtsp over http.
	//! \details About rtps over http: <a href="http://developer.apple.com/quicktime/icefloe/dispatch028.html">Tunnelling RTSP and RTP through HTTP.</a>
	const char		*szRtspOverHttpFdipcPath;
	//! Array of video LiveMediaSrc handles, see LiveMediaSrc.h.
	//! The posion in this array, started from 0, is the video track number in TAcsFileInfo.
	void** ahLiveVSrc;
	//! Numbers of video source handle in the array above.
	int				iLiveVSrcNum;
	//! Array of audio LiveMediaSrc handles, see LiveMediaSrc.h.
	//! The posion in this array, started from 0, is the audio track number in TAcsFileInfo.
	void** ahLiveASrc;
	//! Numbers of audio source handle in the array above.
	int				iLiveASrcNum;
	//! Array of mdata LiveMediaSrc handles, see LiveMediaSrc.h.
	//! The posion in this array, started from 0, is the mdata track number in TAcsFileInfo.
	void** ahLiveDSrc;
	//! Numbers of mdata source handle in the array above.
	int				iLiveDSrcNum;
	//! Array of audio LiveMediaSink handles, see LiveMediaSink.h.
	//! The posion in this array, started from 0, is the audio back channel track number in TAcsFileInfo.
	void** ahLiveAudioBackChannel;
	//! Array of audio back channel RtpMapInfo, see RtpMapInfo.h.
	//! The posion in this array, started from 0, is the audio back channel track number in TAcsFileInfo.
	const TRtpMapInfo* ahLiveAudioBackChannelRTPInfo;
	//! Numbers of audio back channel handle in the array above.
	int				iLiveAudioBackChannelNum;
	//! Array of pointers to TAcsFileInfo.
	//! \Note szAccessName in TAcsFileInfo should be unique among all TAcsFileInfos.		
	PTAcsFileInfo	*aptAcsFileInfo;
	//! Numbers of access file info pointer in the array above.
	int				iAcsFileInfoNum;

	// The callback functions for initialize or release the shared ring buffers of media source.
	PFLMSrcBufInitCB live_msrc_buf_init;
	PFLMSrcBufReleaseCB live_msrc_buf_release;

	// The initial options for creating the shared ring buffers of media source.
	TLiveMSrcInitOpt *LiveVideoMSrcInitOpts;
	TLiveMSrcInitOpt *LiveAudioMSrcInitOpts;
	TLiveMSrcInitOpt *LiveMDataMSrcInitOpts;

	int bDetectBufOverrun;
	int bSendSpsPpsInRtp;
} TRtspSrvrInitOpts;

int RtspSrvr_Initial(void** phRtspSrvrObj, TRtspSrvrInitOpts *ptOpts);

int RtspSrvr_Release(void** phRtspSrvrObj);

int RtspSrvr_Start(void* hRtspSrvrObj);

int RtspSrvr_Stop(void* hRtspSrvrObj);

int RtspSrvr_Suspend(void* hRtspSrvrObj);

int RtspSrvr_Resume(void* hRtspSrvrObj);

typedef struct rtspsrvr_acsfile_rtpport_map
{
	unsigned short usRtpPortStartAt;
	unsigned short usRtpPortLessThan;
} TRtspSrvrAcsFileRtpPortMap;

int RtspSrvr_SetAcsFileRtpPortMap(void* hRtspSrvrObj, TRtspSrvrAcsFileRtpPortMap *ptMap, unsigned int dwMapNum);

// advanced usage, don't use it if you don't know how to use it
// This function can be called only after the RtspSrvr_Initial and before RtspSrvr_Start
int RtspSrvr_SetupDestination(void* hRtspSrvrObj, unsigned int bSupport);
#endif // __RTSP_SERVER_H__
