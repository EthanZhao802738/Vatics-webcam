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
#ifndef VIDEO_SOURCE_CONFIG_H
#define VIDEO_SOURCE_CONFIG_H

#include <config_vi.h>
#include <config_ifp.h>

/*
 * A data structure for video source config
 */
typedef struct
{
	//! A data for video option number
	unsigned int dwViOptionNum;

	//! A pointer data for video option
	VMF_VI_OPTION_T* ptViOptions;

	//! A data for ifp option number
	unsigned int dwIfpOptionNum;

	//! A pointer data for ifp option
	VMF_IFP_OPTION_T* ptIfpOptions;
} VMF_VSRC_CONFIG_T;

/**
 * A structure for determine encoding spec of VSRC stream
 */
typedef struct
{
	unsigned int bEncH264; //! Specify if yuv420 buffer connect with H264 encode
	unsigned int bEncH265; //! Specify if yuv420 buffer connect with H265 encode
	unsigned int bEncJPEG; //! Specify if yuv420 buffer connect with MJPG encode
	unsigned int bOthers;  //! Specify if yuv420 buffer will use directly
} VMF_ENC_SPEC_T;

typedef void (*VMF_VSRC_INIT_FUNC) (unsigned int dwWidth, unsigned int dwHeight);
typedef void (*VMF_VSRC_VI_SIGNAL_FUNC) (unsigned int dwNoSignal);

/*
 * video source info flag
 */
typedef enum
{
	//! video source info flag: fec offset
	VMF_VSRC_INFO_FEC_OFFSET,

	//! video source info flag: pin prefix
	VMF_VSRC_INFO_PIN_PREFIX,

	//! video source info flag: stream size
	VMF_VSRC_INFO_STREAM_SIZE,

	//! video source info flag: ifp size
	VMF_VSRC_INFO_IFP_SIZE,

	//! video source info flag: video capture
	VMF_VSRC_INFO_VI_CAPTURE,

	//! video source info flag: resize stream
	VMF_VSRC_INFO_RESIZE_STREAM,

	//! video source info flag: is ifp output
	VMF_VSRC_INFO_IS_IFP_OUTPUT,

	//! video source info flag: ifp output pin
	VMF_VSRC_INFO_IFP_OUTPUT_PIN,

	//! video source info flag: sub smaple pin
	VMF_VSRC_INFO_IFP_SUB_SAMPLE_PIN,

	//! video source info flag: ifp sub ir pin
	VMF_VSRC_INFO_IFP_SUB_IR_PIN,

	//! video source info flag: ifp statistic pin
	VMF_VSRC_INFO_IFP_STATIS_PIN,

	//! video source info flag: is ssm shared
	VMF_VSRC_INFO_IS_SSM_SHARED,

	//! video source info flag: resource path
	VMF_VSRC_INFO_RESOURCE_PATH,

	//! video source info flag: check dual 360 resize
	VMF_VSRC_INFO_CHK_DUAL_RESIZE,

	//! video source info flag: isp motion detection totalblock
	VMF_VSRC_INFO_ISP_MD_TOTALBLOCKS,

	//! video source info flag: isp MRF coordinate check.
	VMF_VSRC_INFO_ISP_MD_COORDINATECHECK,

} VMF_VSRC_INFO_FLAG;

/*
 * A data structure for video source query info
 */
typedef struct
{
	//! A data for video source info flag
	VMF_VSRC_INFO_FLAG eInfoFlag;

	//! A data for stream index
	unsigned int dwStreamIdx;

	//! A data array for video source query info data
	unsigned int adwData[8];
} VMF_VSRC_QUERY_INFO_T;

/*
 * A data structure for video source resize stream
 */
typedef struct
{
	//! A data for channel number (range: 0 ~ 4)
	unsigned int dwChannelNum;

	//! A data array for resize stream width
	unsigned int dwWidth[VMF_MAX_RESIZE_NUM];

	//! A data array for resize stream height
	unsigned int dwHeight[VMF_MAX_RESIZE_NUM];

	//! A data array for resize stream stride
	unsigned int dwStride[VMF_MAX_RESIZE_NUM];
} VMF_VSRC_RESIZE_STREAM_INFO_T;

/*
 * A data structure for VI callback function rawdata buffer
 */
typedef struct
{
	//! A data for width of VI buffer
	unsigned int dwWidth;

	//! A data for height of VI buffer
	unsigned int dwHeight;

	//! A data for stride of VI buffer
	unsigned int adwStride[4];

	//! A pointer for VI rawdata buffer
	unsigned char* apbyViBuffer[4];
} VMF_VSRC_VI_BUFFER_RAWDATA_INFO_T;

typedef void (*VMF_VSRC_VI_RAWDATA_FUNC) (VMF_VSRC_VI_BUFFER_RAWDATA_INFO_T* pViBufferStruct);

#endif
