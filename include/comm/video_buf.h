
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
#ifndef VMF_VIDEO_BUF_H
#define VMF_VIDEO_BUF_H

#define VMF_MAX_SSM_HEADER_SIZE 256
#define VMF_MAX_SSM_NAME_SIZE 32
#define VMF_MAX_SSM_NAME_PREFIX 10
#define VMF_MAX_PATH_LENGTH 256
#define VMF_MAX_INPUT_LINE_LENGTH 256
#define VMF_MAX_RESIZE_NUM 4

#define VMF_RESOURCE_IFP_SUBDIR "IFPE/"
#define VMF_RESOURCE_ISP_SUBDIR "ISP/"
#define VMF_RESOURCE_AE_SUBDIR  "AE/"
#define VMF_RESOURCE_AWB_SUBDIR "AWB/"
#define VMF_RESOURCE_ASC_SUBDIR "AutoScene/"

#define VMF_8_ALIGN(a)  (((a) + 7) & (~7))
#define VMF_16_ALIGN(a)  (((a) + 15) & (~15))
#define VMF_32_ALIGN(a)  (((a) + 31) & (~31))
#define VMF_64_ALIGN(a)  (((a) + 63) & (~63))
#define VMF_128_ALIGN(a)  (((a) + 127) & (~127))
#define VMF_256_ALIGN(a)  (((a) + 255) & (~255))


#define ENC_DEFAULT_FPS	30
#define ENC_MAX_FPS		120
#define ENC_DEFAULT_GOP	30
#define ENC_MAX_GOP		1200

#define DEFAULT_QP		25
#define MIN_QP 			10
#define MAX_QP 			45

#define H265_MIN_QP 0
#define H265_MAX_QP 51

/*
 * Video signal format flag enumeration
 */
typedef enum
{
    VMF_VIDEO_SIGNAL_FREQUENCY_50HZ = 1,
    VMF_VIDEO_SIGNAL_FREQUENCY_60HZ = 2,
    VMF_VIDEO_SIGNAL_FREQUENCY_24HZ = 3,
    VMF_VIDEO_SIGNAL_FREQUENCY_30HZ = 4
} VMF_VIDEO_SIGNAL_FREQUENCY;

/*
  * Video format flag enumeration
*/
typedef enum
{
	VMF_FRAME_FORMAT_MONO = 1,
	VMF_FRAME_FORMAT_NORMAL_BAY = 11,
	VMF_FRAME_FORMAT_NORMAL_YUV422 = 12,
	VMF_FRAME_FORMAT_NORMAL_RGBIr = 13,
	VMF_FRAME_FORMAT_NORMAL_YUV420 = 14,
	VMF_FRAME_FORMAT_NORMAL_YUV444 = 15,
	VMF_FRAME_FORMAT_DECOMPANDING_BAY = 21,
	VMF_FRAME_FORMAT_DECOMPANDING_YUV422 = 22,
	VMF_FRAME_FORMAT_DECOMPANDING_RGBIr = 23,
	VMF_FRAME_FORMAT_DECOMPANDING_YUV420 = 24,
	VMF_FRAME_FORMAT_FUSION_BAY = 31,
	VMF_FRAME_FORMAT_FUSION_YUV422 = 32,
	VMF_FRAME_FORMAT_FUSION_RGBIr = 33,
	VMF_FRAME_FORMAT_FUSION_YUV420 = 34,
	VMF_FRAME_FORMAT_NORMAL_14BIT = 999 //! Specical mode 
} VMF_VIDEO_FORMAT;

/*
 * A data structure for vmf video buffer
 */
typedef struct
{
	//! A data for Video format
	VMF_VIDEO_FORMAT eVideoFormat;
	//! A data for input and output Width
	unsigned int dwWidth;
	//! A data for input and output Height
	unsigned int dwHeight;
	//! A data for input and output stride
	unsigned int adwStride[4];
	//! A data for exchanging information between hardware engines.
	unsigned int adwHWInfo[12];
	//! A data for input virtual address
	unsigned char  *apbyVirtAddr[4];
	//! A data for input physical address
	unsigned char  *apbyPhysAddr[4];
} VMF_VIDEO_BUF_T;

/*
 * A data structure for vmf frame buffer
 */
typedef struct
{
	//! A data for frame buffer
	unsigned char* apdwData[4];
} VMF_FRAME_BUF_T;

/*
 * A data structure for vmf canvas
 */
typedef struct
{
	//! A data for Width
	unsigned int dwWidth;
	//! A data for Height
	unsigned int dwHeight;
	//! A data for Start X
	unsigned int dwAlignedStartX;
	//! A data for Start X, Y offset should match the hardware restriction
	unsigned int dwStartX;
	//! A data for Start Y
	unsigned int dwStartY;
} VMF_CANVAS_T;

/*
 * A data structure for vmf layout
 */
typedef struct
{
	//! A data for Canvas Width
	unsigned int dwCanvasWidth;
	//! A data for Canvas Height
	unsigned int dwCanvasHeight;
	//! A data for Video Positon X
	unsigned int dwVideoPosX;
	//! A data for Video Positon Y
	unsigned int dwVideoPosY;
	//! A data for Video Width
	unsigned int dwVideoWidth;
	//! A data for Video Height
	unsigned int dwVideoHeight;
} VMF_LAYOUT_T;

/*
 * A data structure for venc input info
 */
typedef struct
{
	//! A data for checkin physical address
	unsigned int bIsPhyAddress;
	//! A data for seconds
	unsigned int dwSec;
	//! A data for microseconds
	unsigned int dwUsec;
	//! A data for Frame Buffer
	VMF_FRAME_BUF_T tFrameBuf;
	//! A data for Frame Buffer Physical adddress
	VMF_FRAME_BUF_T tFrameBufPhys;
	//! A data for Map Info
	void* pMapInfo;
	//! A data for VSRC Output Info
	void* pVsrcOutputInfo;
} VMF_VENC_INPUT_INFO_T;

/*
 * A data structure for venc output info
 */
typedef struct
{
	//! A data for destination buffer virtual address
	unsigned char* pbyDstVirtBuf;
	//! A data for destination buffer phisical address
	unsigned char* pbyDstPhysBuf;
	//! A data for destination buffer size limit
	unsigned int dwBufSize;
	//! A data for watermark string NULL: no watermark, Others: watermark string
	const char* pszWatermarkStr;
} VMF_VENC_OUTPUT_INFO_T;

#endif //! VMTK_FRAME_H
