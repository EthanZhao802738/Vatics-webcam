#ifndef _RTSPCLNT_H_
#define _RTSPCLNT_H_

#include "rtspc_common.h"

#define RTSPCLNT_VERSION MAKEFOURCC( 2, 0, 0, 6 )

typedef enum rtspclnt_status
{
    rcsConnecting,
    rcsConnected,
    rcsDisconnecting,
    rcsDisconnected
} ERtspClntStatus;

typedef struct rtspclnt_ubuffer_information
{
    BYTE    *pbyUBufHdr;
    DWORD   dwUBufHdrLen;
    BYTE    *pbyUBufPayload;
    DWORD   dwUBufPayloadLen;
} TRtspClntUBufInfo;

typedef SCODE (*PFRtspClntStatusCB)(HANDLE hUserContext, ERtspClntStatus eStatus);
typedef SCODE (*PFRtspClntBitStrmCB)(HANDLE hUserContext, TRtspClntUBufInfo *ptUBufInfo);
typedef SCODE (*PFRtspClntGetBufCB)(HANDLE hUserContext, TRtspClntUBufInfo *ptUBufInfo);
typedef SCODE (*PFRtspClntReleaseBufCB)(HANDLE hUserContext, TRtspClntUBufInfo *ptUBufInfo);

typedef struct rtspclnt_initial_options
{
    HANDLE                  hStatusCBContext;
    HANDLE                  hVideoCBContext;
    HANDLE                  hAudioCBContext;

    PFRtspClntStatusCB      pfStatusCB;

    // For video bitstream
    PFRtspClntBitStrmCB     pfVideoCB;
    PFRtspClntGetBufCB      pfVideoGetBufCB;
    PFRtspClntReleaseBufCB  pfVideoReleaseBufCB;

    // For audio bitstream
    PFRtspClntBitStrmCB     pfAudioCB;
    PFRtspClntGetBufCB      pfAudioGetBufCB;
    PFRtspClntReleaseBufCB  pfAudioReleaseBufCB;
} TRtspClntInitOpts;

typedef enum rtspclnt_transport_mode
{
    rctmUdp,
    rctmTcp,
    rctmHttp,
    rctmHttps,
    rctmMulticast,
    rctmAuto,
    rctmNum
} ERtspClntTransportMode;

typedef enum rtspclnt_media_mode
{
    rcmmVideoOnly,
    rcmmAudioOnly,
    rcmmVideoAudio
} ERtspClntMediaMode;

typedef struct rtspclnt_connection_options
{
    char                    *szServerURL;	// e.g. rtsp://[IP:Port]/[AggregatedName]
    char					*szUsername;
    char					*szPassword;
    ERtspClntTransportMode  eTransportMode;	// Udp/Tcp/Http/Multicast
    ERtspClntMediaMode      eMediaMode;		// Video/Audio/Video+Audio
} TRtspClntConnOpts;

typedef enum rtspclnt_media_type
{
    rcmtVideo,
    rcmtAudio
} ERtspClntMType;

SCODE RtspClnt_Initial(HANDLE *phRtspClnt, TRtspClntInitOpts *ptInitOpts);
SCODE RtspClnt_Release(HANDLE *phRtspClnt);

SCODE RtspClnt_Start(HANDLE hRtspClnt);
SCODE RtspClnt_Stop(HANDLE hRtspClnt);

SCODE RtspClnt_SetConnOptions(HANDLE hRtspClnt, TRtspClntConnOpts *ptConnOpts);
SCODE RtspClnt_GetUBufConf(HANDLE hRtspClnt, BYTE *pbyConfBuf, DWORD *pdwConfBufLen, ERtspClntMType eMType);

#endif //_RTSPCLNT_H_
