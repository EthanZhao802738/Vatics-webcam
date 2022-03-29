
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
#ifndef VMF_FRAME_INFO_H
#define VMF_FRAME_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

/*! Make from four character codes to one 32-bits DWORD */
#ifndef VMF_MAKEFOURCC
#define VMF_MAKEFOURCC(ch0, ch1, ch2, ch3)  ((unsigned int)(unsigned char)(ch0) | ((unsigned int)(unsigned char)(ch1) << 8) | ((unsigned int)(unsigned char)(ch2) << 16) | ((unsigned int)(unsigned char)(ch3) << 24 ))
#endif //defined(VMF_MAKEFOURCC)

/*! FOURCC for video conf */
#ifndef VMF_FOURCC_CONF
#define VMF_FOURCC_CONF (VMF_MAKEFOURCC('C','O','N','F'))
#endif

/*! FOURCC for H264 video codec */
#ifndef VMF_FOURCC_H264
#define VMF_FOURCC_H264 (VMF_MAKEFOURCC('H','2','6','4'))
#endif

/*! FOURCC for H265 video codec */
#ifndef VMF_FOURCC_H265
#define VMF_FOURCC_H265 (VMF_MAKEFOURCC('H','2','6','5'))
#endif

/*! FOURCC for JPEG image codec */
#ifndef VMF_FOURCC_JPEG
#define VMF_FOURCC_JPEG (VMF_MAKEFOURCC('J','P','E','G'))
#endif

/*
 * A data Structure for HW Device ID
 */
typedef enum
{
	VMF_HW_VIC = 0,
	VMF_HW_IFP,
	VMF_HW_AE,
	VMF_HW_AWB,
	VMF_HW_ASC,
	VMF_HW_ISP
} VMF_HW_DEVICE_ID;

/*
 * A data Structure for Frame Info 
 */
typedef struct
{
	//! A data for seconds
	unsigned int dwSec;

	//! A data for microseconds
	unsigned int dwUSec;
} vmf_frame_info_t;

typedef vmf_frame_info_t VMF_FRAME_INFO_T;

/*
 * A data Structure for VMF Buffer 
 */
typedef struct
{
	//! A data for position X
	unsigned int dwPosX;

	//! A data for position Y
	unsigned int dwPosY;

	//! A data for stride
	unsigned int dwStride;

	//! A data for height
	unsigned int dwHeight;
} vmf_buf_alloc_t;

typedef vmf_buf_alloc_t VMF_BUF_ALLOC_T;

#ifdef __cplusplus
}
#endif

#endif


