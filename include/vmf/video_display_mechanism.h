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
#ifndef VIDEO_DISPLAY_MECHANISM_H
#define VIDEO_DISPLAY_MECHANISM_H

#include <comm/video_buf.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VMF_VIDEO_DISPLAY_MIN_QUEUE_SIZE 3

typedef struct VMF_VDISP_HANDLE_T VMF_VDISP_HANDLE_T;

/*
 * An enumeration for video display type
 */
typedef enum
{
	//! normal type: video display on the second frame 
	VMF_VMF_VDISP_TYPE_NORMAL,    
	//! special type: video display on the first frame
	VMF_VMF_VDISP_TYPE_SPECIAL,
} VMF_VDISP_TYPE;

/*
 * A data structure for video display init option
 */
typedef struct
{
	//! A data for output display format
	unsigned int dwInPixFormat;
	//! A data for max video display buffer width
	unsigned int dwMaxInWidth;
	//! A data for max video display buffer heigth
	unsigned int dwMaxInHeight;
	//! A data for video display type, 0: display in second frame 1: display in first frame
	VMF_VDISP_TYPE eVdispType;
} VMF_VDISP_INITOPT_T;

/*
 * A data structure for pip video display config
 */
typedef struct
{
	//! A data for The output height of PIP video display
	unsigned int dwInPixFormat;
	//! A data for the output width of PIP video display
	unsigned int dwWidth;
	//! A data for the output height of PIP video display
	unsigned int dwHeight;
	//! A data for the output stride of PIP video display
	unsigned int dwStride;
	//! A data for PIP starting X position
	unsigned int dwStartX;
	//! A data for PIP starting Y position
	unsigned int dwStartY;
} VMF_VDISP_PIP_CONFIG_T;

/**
 * @brief Function to initialize the video display.
 *
 * @param[in] ptVideoOpt The point of VMF_VDISP_INITOPT_T structure.
 * @return The handle of video display.
 */
VMF_VDISP_HANDLE_T* VMF_VDISP_Init(const VMF_VDISP_INITOPT_T* ptVideoOpt);

/**
 * @brief Function to release the video display.
 *
 * @param[in] ptHandle The handle of video display.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VDISP_Release(VMF_VDISP_HANDLE_T* ptHandle);

/**
 * @brief Function to push one frame into the queue of video display.
 *
 * @param[in] ptHandle The handle of video display.
 * @param[in] ptSrcBuf The point of VMF_FRAME_BUF_T structure.
 * @param[in] pdwIndex The point of process index of the image buffer.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VDISP_ProcessOneFrame(VMF_VDISP_HANDLE_T* ptHandle, const VMF_FRAME_BUF_T* ptSrcBuf, unsigned int* pdwIndex);

/**
 * @brief Function to set the PIP video display data.
 *
 * @param[in] ptHandle The handle of video display.
 * @param[in] ptConfig The point of VMF_VDISP_PIP_CONFIG_T structure.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VDISP_PIP_SetInfo(VMF_VDISP_HANDLE_T* ptHandle, const VMF_VDISP_PIP_CONFIG_T* ptConfig);

/**
 * @brief Function to push one frame into the queue of PIP video display.
 *
 * @param[in] ptHandle The handle of video display.
 * @param[in] ptSrcBuf The point of VMF_FRAME_BUF_T structure.
 * @param[in] pdwIndex The point of process index of the image buffer.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VDISP_PIP_ProcessOneFrame(VMF_VDISP_HANDLE_T* ptHandle, const VMF_FRAME_BUF_T* ptSrcBuf, unsigned int* pdwIndex);

/**
 * @brief Function to stop the PIP video display.
 *
 * @note Please remember to use mutex to protect outside,
 *		 especially when you use multi-thread to handle display.
 * @param[in] ptHandle The handle of video display.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VDISP_PIP_Stop(VMF_VDISP_HANDLE_T *ptHandle);

#ifdef __cplusplus
}
#endif

#endif
