
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
#ifndef RESIZE_H
#define RESIZE_H

#include <comm/video_buf.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VMF_RS_HANDLE_T VMF_RS_HANDLE_T;

/**
 * A structure for resize object initialization.
 */
typedef struct {
	unsigned int dwSrcWidth;		//! The width of input frame.
	unsigned int dwSrcHeight;		//! The height of input frame.
	unsigned int dwSrcStride;		//! The stride of input frame.
	unsigned int dwIsMono;			//! Use mono frames or not, 0 or 1.
	unsigned int dwFormatFlag;		//! Enable/disable special format, 0 or 1.
	unsigned int dwUseDuplex;		//! Use duplex mode for resizing or not, 0 or 1.
	const char*  pszParamsDir;      //! Ths location of isp config
} VMF_RS_INITOPT_T;

/**
 * A structure for output resize buffer setup.
 */
typedef struct {
	unsigned int dwDstWidth;		//! The width of output frame.
	unsigned int dwDstHeight;		//! The height of output frame.
	unsigned int dwDstStride;		//! The stride of output frame.
	unsigned int dwIsMono;			//! Use mono frames or not, 0 or 1.
	unsigned int dwSharpness;		//! Sharpness level, from 0 to 5.
	unsigned int dwAntiAliasing;	//! Enable/disable anti-aliasing, 0 or 1.
} VMF_RS_CONFIG_T;

/**
 * @brief Function to initialize a resize object.
 *
 * @param[in] ptOpt The Resize object's initializing option.
 * @return The handle of resize object
 */
VMF_RS_HANDLE_T* VMF_RS_Init(const VMF_RS_INITOPT_T* ptOpt,  const VMF_RS_CONFIG_T* ptConfigOpt);

/**
 * @brief Function to setup output resize buffer.
 *
 * @param[in] ptHandle The handle of resize object.
 * @param[in] ptOpt The Resize object's initializing option.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_RS_Config(VMF_RS_HANDLE_T* ptHandle, const VMF_RS_CONFIG_T* ptConfigOpt);

/**
 * @brief Function to release a resize object.
 *
 * @param[in] ptHandle The handle of resize object.
 * @return Success: 0  Fail: negative integer.
 */
 
int VMF_RS_Release(VMF_RS_HANDLE_T* ptHandle);

/**
 * @brief Process resizing (blocking).
 *
 * @param[in] ptHandle The handle of resize object.
 * @param[in] ptDstBuf The pointer of VMF_VIDEO_BUF_T structure.
 * @param[in] ptSrcBuf The pointer of VMF_VIDEO_BUF_T structure.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_RS_ProcessOneFrame(VMF_RS_HANDLE_T* ptHandle, VMF_VIDEO_BUF_T* ptDstBuf, const VMF_VIDEO_BUF_T* ptSrcBuf);

/**
 * @brief Start processing resize (non-blocking).
 *
 * @param[in] ptHandle The handle of resize object.
 * @param[in] ptDstBuf The output pointer of VMF_VIDEO_BUF_T structure.
 * @param[in] ptSrcBuf The input pointer of VMF_VIDEO_BUF_T structure.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_RS_StartOneFrame(VMF_RS_HANDLE_T* ptHandle, VMF_VIDEO_BUF_T* ptDstBuf, const VMF_VIDEO_BUF_T* ptSrcBuf);

/**
 * @brief Wait for the StartOneFrame function to complete.
 * @param[in] ptHandle The handle of resize object.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_RS_WaitOneFrameComplete(VMF_RS_HANDLE_T* ptHandle);

#ifdef __cplusplus
}
#endif

#endif
