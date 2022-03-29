/*
 *******************************************************************************
 *  Copyright (c) 2010-2017 VATICS Inc. All rights reserved.
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
#ifndef VIDEO_ENCODER_H
#define VIDEO_ENCODER_H

#include <vmf/vector_dma.h>
#include <vmf/source_connect.h>
#include <vmf/config_osd.h>
#include <TextRender/text_render.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * An enumeration for Codecs type in VMF video encoder
 */
typedef enum
{
	VMF_VENC_CODEC_TYPE_H264, //! H.264 Codec
	VMF_VENC_CODEC_TYPE_H265, //! H.265 Codec
	VMF_VENC_CODEC_TYPE_MJPG, //! MJPG Codec
} VMF_VENC_CODEC_TYPE;

/*
 * An enumeration for H.264 Profiles
 */
typedef enum
{
	VMF_H4E_PROFILE_BASE, //! H.264 base profile
	VMF_H4E_PROFILE_MAIN, //! H.264 main profile
	VMF_H4E_PROFILE_HIGH, //! H.264 high profile
} VMF_H4E_PROFILE;

/*
 * An enumeration for H.264 advance mode
 */
typedef enum
{
	VMF_ADMODE_MEET_FPS,     //! fps priority
	VMF_ADMODE_MEET_QUALITY, //! quality priority
	VMF_ADMODE_CUSTOMIZED,   //! customized
} VMF_ADMODE;

/*
 * A structure for H.264 advance mode
 */
typedef struct
{
	//! Quantization parameter
	unsigned int dwQp;

	//! Bitrate control.  0 -> no bitrate constraint. 0 -> VBR, others -> CBR
	unsigned int dwBitrate;

	//! Input frame rate. It is used for bitrate control
	double dbFps;

	//! Group of pictures
	unsigned int dwGop;

	//! H.264 Profile setting: base / main / high
	VMF_H4E_PROFILE eProfile;

	//! Bitrate control policy
	//! 0: disable , != 0 : put delta_qp to I frame
	//! Larger number means low quality and low size to I frame
	int iSliceQualityStrategy;

	//! Advanced mode for rate control
	VMF_ADMODE eAdMode;

	//! VMF_ADMODE_CUSTOMIZED, minimum qp
	unsigned int dwMinQp;

	//! VMF_ADMODE_CUSTOMIZED, maximum qp
	unsigned int dwMaxQp;

	//! VMF_ADMODE_CUSTOMIZED, minimum fps
	unsigned int dwMinFps;

	//! Virtual I-frame interval
	//! (0 ~ GOPSize-1). Default: 0 (disable)
	unsigned int dwVirtIFrameInterval;

	//! PIQ setting. Default: 0 (disable)
	unsigned int dwPIQ;
} VMF_H4E_CONFIG_T;

/*
 * A structure for H.265 config
 */
typedef struct
{
	//! Quantization parameter
	unsigned int dwQp;

	//! Bitrate control. 0 -> no bitrate constraint. 0 -> VBR, others -> CBR
	unsigned int dwBitrate;

	//! Frame rate
	unsigned int dwFps;

	//! Group of pictures
	unsigned int dwGop;

	//! Bitrate control policy
	//! 0: disable , != 0 : put delta_qp to I frame
	//! Larger number means low quality and low size to I frame
	int iSliceQualityStrategy;	

	//! Minimum quantization parameter
	unsigned int dwMinQp;

	//! Maximum quantization parameter
	unsigned int dwMaxQp;
	
	//! Virtual I-frame interval
	//! (0 ~ GOPSize-1). Default: 0 (disable)
	unsigned int dwVirtIFrameInterval;

	//! PIQ setting. Default: 0 (disable)
	unsigned int dwPIQ;

	//! Advanced mode for rate control
	VMF_ADMODE eAdMode;

	//! Complex map control in vbr mode: 0: diable, 1: enable
	unsigned int bEnComplexMapInVBRmode;
} VMF_H5E_CONFIG_T;

/*
 * A structure for MJPG config
 */
typedef struct
{
	//! Quantization parameter. value: 0~200, 0~100 for better quality
	unsigned int dwQp;

	//! Enable thumbnail or not. value: 0 - disable, 1 - enable
	unsigned int bEnableThumbnail;

	//! Quantization parameter for thumbnail. value: 0~200, 0~100 for better quality. Note: The size of thumbnail can't exceed 65535 bytes.
	unsigned int dwThumbnailQp;

	//! Enable JFIF header or not. value: 0 - disable, 1 - enable
	unsigned int bJfifHdr;

	//! Bit rate. 0 -> disable rate control. Currently, it is useless in the initialized function.
	unsigned int dwBitrate;

	//! Frame rate. It is used for rate control. 0 -> disable rate control. Currently, it is useless in the initialized function.
	unsigned int dwFps;
} VMF_JE_CONFIG_T;

/*
 * A structure for VMF video encoder information
 */
typedef struct
{
	//! Encoded codec type
	VMF_VENC_CODEC_TYPE eCodecType;

	//! Encoded image width
	unsigned int dwEncWidth;

	//! Encoded image height
	unsigned int dwEncHeight;

	//! Encoded image stride
	unsigned int dwEncStride;

	//! Frame rate
	unsigned int dwFps;

	//! Group of picture
	unsigned int dwGop;

	//! Bitrate
	unsigned int dwBitrate;

	//! Frame sec
	unsigned int dwSec;

	//! Frame usec
	unsigned int dwUSec;

	//! Sequence number
	unsigned int dwSeqNum;

	//! Key frame or not
	//! MJPG: always 1
	//! H.264/H.265: I-frame is 1, P-frame is 0
	unsigned int dwIsKeyFrame;

	//! Encoded video size in bytes
	unsigned int dwEncodedBytes;

	//! H.265 ROI 64 windows status on 64bits
	unsigned long long bH265RoiBits;

	//! H.265 CBR ROI sum of threshold
	unsigned int dH265Roithreshold;

	//! Encoded Data offset
	unsigned int dwBufOffset;
} VMF_VENC_ENCODE_INFO_T;

/*
 * A structure for VMF video encoder config
 */
typedef struct
{
	//! Encoded image width
	unsigned int dwEncWidth;

	//! Encoded image height
	unsigned int dwEncHeight;

	//! On/Off to enable cropping
	unsigned int bEnableCropping;

	//! Request image width from VSRC
	unsigned int dwRequestWidth;

	//! Request image height from VSRC
	unsigned int dwRequestHeight;

	//! X-axis offset of cropping
	unsigned int dwCropStartX;

	//! Y-axis offset of cropping
	unsigned int dwCropStartY;

	//! Force reinit
	unsigned int dwReInit;

	//! Frame rate
	unsigned int dwFps;

	//! Connect encoder with ifp frame
	unsigned int bConnectIfp;
	//! A data for disable shared osd flag
	unsigned int bDisableSharedOsd;

	//! Codec configuration
	//! Codec type: H5E / H4E / MJPG
	VMF_VENC_CODEC_TYPE eCodecType;

	//! Required argument, shall be filled in one of the following structures:
	//! VMF_H4E_CONFIG_T
	//! VMF_H5E_CONFIG_T
	//! VMF_JE_CONFIG_T
	void* pCodecConfig;

	//! If no signal, the YUV value of backscreen.
	unsigned int dwNoSignalBackGroundColorY;
	unsigned int dwNoSignalBackGroundColorU;
	unsigned int dwNoSignalBackGroundColorV;

	//! Callback function before an encoded frame is produced
	//! Please DO NOT call VMF VENC functions inside this callback or will lead to deadlocks
	int (*fnOnPreProcessCallback) (void* pUserData); 
	//! Customized user data for PreProcessCallback function
	void* pOnPreProcessCallbackUserData;

	//! Callback function to set output buffer for process one frame
	//! Please DO NOT call VMF VENC functions inside this callback or will lead to deadlocks
	int (*fnOnSetOutputCallback) (unsigned char** ppbyOutBuff, unsigned char** ppbyOutPhysBuff, unsigned int* pdwOutBuffSize, void* pUserData);
	//! Customized user data for set output buffer callback function
	void* pOnSetOutputCallbackUserData;

	//! Callback function when streaming header is produced
	//! Please DO NOT call VMF VENC functions inside this callback or will lead to deadlocks
	int (*fnOnStreamHeaderCallback) (VMF_VENC_ENCODE_INFO_T* ptEncodeInfo, void* pStreamHeader, void* pUserData);
	//! Customized user data for producing streaming header callback function
	void* pOnStreamHeaderCallbackUserData;

	//! Callback function when an encoded frame data is produced
	//! Please DO NOT call VMF VENC functions inside this callback or will lead to deadlocks
	int (*fnOnDataCallback) (VMF_VENC_ENCODE_INFO_T* ptEncodeInfo, unsigned char* pbyData, unsigned int dwDataBytes, void* pUserData);
	//! Customized user data for producing encoded frame data callback function
	void* pOnDataCallbackUserData;

	//! Callback function after an encoded frame is produced
	//! You can call VMF VENC functions in this callback to control video encoder during runtime
	int (*fnOnPostProcessCallback) (VMF_VENC_ENCODE_INFO_T* ptEncodeInfo, void* pUserData);
	//! Customized user data for producing encoded frame data callback function
	void* pOnPostProcessCallbackUserData;

	//! Callback function that requests raw video streams from video sources, mostly this is set to VMF_BIND_Request()
	VMF_SRC_CONNECT_FUNC fnSrcConnectFunc;

	//! VMF_BIND instance
	void* pBind;

	//! Determines whether function VMF_VENC_Config() blocks until feature configuration is done
	//! 0: non-block mode
	//! 1: block mode
	unsigned int bWaitComplete;
} VMF_VENC_CONFIG_T;

typedef struct VMF_VENC_HANDLE_T VMF_VENC_HANDLE_T;

/*
 * A structure for H.264 streaming header
 */
typedef struct
{
	//! FourCC of config
	unsigned int dwConfFourCC;

	//! streaming header total size
	unsigned int dwTotalSize;

	//! FourCC of H264
	unsigned int dwH264FourCC;

	//! Encode width
	unsigned int dwEncWidth;

	//! Encode height
	unsigned int dwEncHeight;

	//! SVC flag
	unsigned int bIsSvc;

	//! SPS data size
	unsigned int dwSpsSize;

	//! PPS data size
	unsigned int dwPpsSize;

	//! SPS data
	unsigned char abySpsData[128];

	//! PPS data
	unsigned char abyPpsData[128];
} VMF_VENC_H264_STREAM_HDR;

/*
 * A structure for H.265 streaming header
 */
typedef struct
{
	//! FourCC of config
	unsigned int dwConfFourCC;

	//! Streaming header total size
	unsigned int dwTotalSize;

	//! FourCC of H265
	unsigned int dwH265FourCC;

	//! Encode width
	unsigned int dwEncWidth;

	//! Encode height
	unsigned int dwEncHeight;

	//! SVC flag
	unsigned int bIsSvc;

	//! VPS data size
	unsigned int dwVpsSize;

	//! SPS data size
	unsigned int dwSpsSize;

	//! PPS data size
	unsigned int dwPpsSize;

	//! VPS data
	unsigned char abyVpsData[128];

	//! SPS data
	unsigned char abySpsData[128];

	//! PPS data
	unsigned char abyPpsData[128];
} VMF_VENC_H265_STREAM_HDR;

/*
 * A structure for MJPG streaming header
 */
typedef struct
{
	//! FourCC of CONF
	unsigned int dwConfFourCC;

	//! Encoded data size
	unsigned int dwDataBytes;

	//! FourCC of MJPG
	unsigned int dwMjpgFourCC;

	//! Encode width
	unsigned int dwEncWidth;

	//! Encode height
	unsigned int dwEncHeight;
} VMF_VENC_MJPG_STREAM_HDR;

/*
 * A structure for video streaming buffer header
 */
typedef struct
{
	//! FourCC of CONF
	unsigned int dwFourCC;

	//! Frame timestamp in seconds
	unsigned int dwFrameSec;

	//! Frame timestamp in usecond
	unsigned int dwFrameUSec;

	//! Encoded data size in bytes
	unsigned int dwDataBytes;

	//! Sequence number of encoded frames
	unsigned int dwSeqNum;

	//! Is key frame or not
	unsigned int bIsKeyFrame;

	//! Encoded Data offset
	unsigned int dwBufOffset;
} VMF_VENC_STREAM_DATA_HDR;

/*
 * An enurmation for advanced features, H.264 SDF - Smooth Drop Frame Modes
 */
typedef enum
{
	//! Disabled
	VMF_H4E_SDF_MODE_DISABLED,

	//! Enabled right away if current condition matches, smooth drop frame starts at next GOP
	VMF_H4E_SDF_MODE_IMMEDIATE,

	//! Enabled at next GOP if condition matches, smooth drop frame starts at the GOP after next GOP
	VMF_H4E_SDF_MODE_NEXT_GOP,
} VMF_H4E_SDF_MODE;

/*
 * An enurmation for leave H.264 SDF conditions
 */
typedef enum
{
	//! Leave smooth drop frame condition, judged from bit rate
	VMF_H4E_SDF_OUT_CONDITION_BITRATE,

	//! Leave smooth drop frame condition, judged from QP
	VMF_H4E_SDF_OUT_CONDITION_QP,
} VMF_H4E_SDF_OUT_CONDITION;

/*
 * A structure for H.264 SDF (smooth drop frame) config
 */
typedef struct
{
	//! Smooth drop frame mode
	VMF_H4E_SDF_MODE eMode;

	//! Condition to start smooth drop frame behavior, judged from frames
	unsigned int dwInCondFrames;

	//! Condition to start smooth drop frame behavior, judged from bit rate
	unsigned int dwInCondBits;

	//! Condition to leave smooth drop frame behavior, judged from bit rate or QP
	VMF_H4E_SDF_OUT_CONDITION eOutCond;
} VMF_H4E_SDF_CONFIG_T;

/*
 * A structure for H.264 PDS (prediction search) config
 */
typedef struct
{
	//! H.264 PDS on/off flag, 0: Disabled, 1: Enabled
	unsigned int dwEnable;
} VMF_H4E_PDS_CONFIG_T;

/*
 * A structure for config to decrease I/P-frame quality gap to solve static scene respiratory effects
 */
typedef struct
{
	//! PIQ on/off flag, 0: Disabled, 1: Enabled
	unsigned int dwEnable;
} VMF_VENC_PIQ_CONFIG_T;


/*
 * A structure H.264 watermark string config
 */
typedef struct
{
	//! 1: Enabled, 0: Disabled
	unsigned int dwEnable;

	//! The pointer to watermark string
	const char* pszWatermarkStr;
} VMF_H4E_WATERMARK_STR_CONFIG_T;

#define MAX_H264_ROI_WINDOW_INDEX		7
#define MAX_H265_ROI_WINDOW_INDEX		63

/*
 * A structure for ROI window config
 */
typedef struct
{
	//! 1: Enabled, 0: Disabled
	unsigned int bEnable;

	//! H.264: 0~7 (MAX_H264_ROI_WINDOW_INDEX), H.265: 0~63 (MAX_H265_ROI_WINDOW_INDEX)
	//! Note: H.264 grid unit 16x16, H.265 64x64
	unsigned int dwWindowIdx;

	//! X-axis of top-left pixel
	unsigned int dwStartX;

	//! Y-axis of top-left pixel
	unsigned int dwStartY;

	//! X-axis of bottom-right pixel
	unsigned int dwEndX;

	//! Y-axis of bottom-right pixel
	unsigned int dwEndY;

	//! Delta QP value
	int sdwDeltaQp;

	//! Used by H4E only
	unsigned int dwEncodingInterval;
} VMF_VENC_ROI_WINDOW_T;

/*
 * A structure for watermark config(vatics demo application watermark) config
 */
typedef struct
{
	//! Watermark on off flag, 0: Disabled, 1: Enabled
	unsigned int dwEnable;
} VMF_VENC_WATERMARK_CONFIG_T;

/*
 * A structure for resolution config (vatics demo application resolution change) 
 */
typedef struct
{
	//! Resolution on off flag, 0: Disabled, 1: Enabled
	unsigned int dwEnable;

	unsigned int dwWidth;

	unsigned int dwHeight;
} VMF_VENC_RES_CONFIG_T;

/*
 * A structure for delta QP in complex info config
 */
typedef struct
{
    unsigned int dwCplxTableType; //! 0: I-frame table, 1: P-frame table
    char chDeltaQp[7]; 
} VMF_VENC_COMPLEX_INFO_T;

/*
 * An enurmation for codec feature
 */
typedef enum
{
	//! H.264 feature: smooth drop frame
	VMF_CODEC_FEATURE_H4E_SDF,

	//! H.264 feature: prediction search
	VMF_CODEC_FEATURE_H4E_PDS,

	//! H.264 / H.265 feature: decrease I/P-frame quality gap to solve static scene respiratory effects
	VMF_CODEC_FEATURE_PIQ,

	//! H.264 feature: encapsulated watermark string
	VMF_CODEC_FEATURE_H4E_WATERMARK_STR,

	//! H.264 feature: all mode 
	VMF_CODEC_FEATURE_H4E_ALL_MODE,

	//! H.264 / H.265 feature: ROI window
	VMF_CODEC_FEATURE_ROI_WINDOW,

	//! For VATICS DEMO applications
	VMF_CODEC_FEATURE_WATERMARK,

	//! For VATICS DEMO resolution applications
	VMF_CODEC_FEATURE_RES,

	//! H.264 / H.265 feature: delta_qp to I frame (larger number means low quality and low size to I frame)
	VMF_CODEC_FEATURE_ISLICE_QP,

	//! H.264 / H.265 feature: Change delta QP setting of complex info
	VMF_CODEC_FEATURE_COMPLEX_QP,

	//! H.264 / H.265 feature: Change Gop
	VMF_CODEC_FEATURE_GOP,

	//! H.264 / H.265 feature: Output black screen when VIC no signal
	VMF_CODEC_FEATURE_BLACK_SCREEN,

	//! Encoder: skip frame 
	VMF_CODEC_FEATURE_SKIP_FRAME
} VMF_CODEC_FEATURE_FLAG_T;

/*
 * A structure for code feature config
 */
typedef struct
{
	VMF_CODEC_FEATURE_FLAG_T eFeatureFlag; //! Codec feature flag

	//! Codec feature config according to eFeatureFlag
	//! VMF_CODEC_FEATURE_H4E_SDF -> VMF_H4E_SDF_CONFIG_T
	//! VMF_CODEC_FEATURE_H4E_PDS -> VMF_H4E_PDS_CONFIG_T
	//! VMF_CODEC_FEATURE_PIQ -> VMF_VENC_PIQ_CONFIG_T
	//! VMF_CODEC_FEATURE_ROI_WINDOW -> VMF_VENC_ROI_WINDOW_T
	//! VMF_CODEC_FEATURE_COMPLEX_QP -> VMF_VENC_COMPLEX_INFO_T
	void *pData;

	//! Determines whether function VMF_VENC_ConfigFeature() blocks until feature configuration is done
	//! 0: non-block mode
	//! 1: block mode
	unsigned int bWaitComplete;
} VMF_CODEC_FEATURE_CONFIG_T;

/**
 * @brief Function to initialize VMF video encoder
 *
 * @param[in] ptConfig Video encoder configuration.
 * @return The handle of VMF video encoder.
 */
VMF_VENC_HANDLE_T* VMF_VENC_Init(const VMF_VENC_CONFIG_T* ptConfig);

/**
 * @brief Function to release VMF video encoder
 *
 * @param[in] ptHandle The handle of VMF video encoder.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VENC_Release(VMF_VENC_HANDLE_T* ptHandle);

/**
 * @brief Function to start encoding.
 *
 * @param[in] ptHandle The handle of VMF video encoder.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VENC_Start(VMF_VENC_HANDLE_T* ptHandle);

/**
 * @brief Function to stop encoding.
 *
 * @param[in] ptHandle The handle of VMF video encoder.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VENC_Stop(VMF_VENC_HANDLE_T* ptHandle);

/**
 * @brief Function to resume encoding.
 *
 * @param[in] ptHandle The handle of VMF video encoder.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VENC_Resume(VMF_VENC_HANDLE_T* ptHandle);

/**
 * @brief Function to suspend encoding.
 *
 * @param[in] ptHandle The handle of VMF video encoder.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VENC_Suspend(VMF_VENC_HANDLE_T* ptHandle);

/**
 * @brief Function to configure video encoder
 *
 * @param[in] ptHandle The handle of VMF video encoder.
 * @param[in] ptConfig Video encoder configuration.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VENC_Config(VMF_VENC_HANDLE_T* ptHandle, const VMF_VENC_CONFIG_T* ptConfig);

/**
 * @brief Force the next encoded frame to be IDR (H.264 and H.265).
 *
 * @param[in] ptHandle The handle of VMF video encoder.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VENC_SetIntra(VMF_VENC_HANDLE_T* ptHandle);

/**
 * @brief Function to produce streaming header.
 *
 * @param[in] ptHandle The handle of VMF video encoder.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VENC_ProduceStreamHdr(VMF_VENC_HANDLE_T* ptHandle);

/**
 * @brief Function to config codec feature.
 *
 * @param[in] ptHandle The handle of VMF video encoder.
 * @param[in] ptFeatureConfig Codec feature configuration.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VENC_ConfigFeature(VMF_VENC_HANDLE_T* ptHandle, VMF_CODEC_FEATURE_CONFIG_T* ptFeatureConfig);

/**
 * @brief Function to configure font of text overlay in video streaming.
 *
 * @param[in] ptHandle The handle of VMF video encoder.
 * @param[in] ptFontInfo The pointer of FONT_INFO_T.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VENC_SetFont(VMF_VENC_HANDLE_T* ptHandle, FONT_INFO_T* ptFontInfo);

/**
 * @brief Function to config overlay in video streaming.
 *
 * @param[in] ptHandle The handle of VMF video encoder.
 * @param[in] ptConfig The pointer of VMF_OVERLAY_CONFIG_T.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VENC_ConfigOverlay(VMF_VENC_HANDLE_T* ptHandle, VMF_OVERLAY_CONFIG_T* ptConfig);

#ifdef __cplusplus
}
#endif
#endif

