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
#ifndef VIDEO_OUTPUT_H
#define VIDEO_OUTPUT_H

#include <comm/video_buf.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VMF_VO_REQBUFS_MIN 3
#define VMF_VO_REQBUFS_MAX 32

typedef struct VMF_VO_HANDLE_T VMF_VO_HANDLE_T;

/*
 * A data structure for video display init
 */
typedef struct
{
	//!VO Device Name, ex: "/dev/video0"
	const char*  pszVoDevName; 
	//! A data for Video format
	unsigned int dwInPixFormat;
	//!The number of buffers requested. value: VMF_VO_REQBUFS_MIN ~ VMF_VO_REQBUFS_MAX
	unsigned int dwReqBuffCount;  
	//! A data for Video Width
	unsigned int dwVideoWidth;
	//! A data for Video Height
	unsigned int dwVideoHeight;
	//! A data for StrideLuma
	unsigned int dwStrideLuma;  
	//! A data for StrideChroma
	unsigned int dwStrideChroma;
	//! A data for Luma framesize
	unsigned int dwFrameSizeLuma;
	//! A data for Chroma framesize
	unsigned int dwFrameSizeChroma;
	//! A data for PIP offset X
	unsigned int dwPIPOffsetX;
	//! A data for PIP offset Y
	unsigned int dwPIPOffsetY;
} VMF_VO_INITOPT_T;

/**
 * @brief Function to initialize VMF Video Output
 *
 * @param[in] ptInitOpt A pointer to the VMF_VO_INITOPT_T structure.
 * @return The handle of VMF Video Output.
 * @note This is NOT thread-safe.
 */
VMF_VO_HANDLE_T *VMF_VO_Init(const VMF_VO_INITOPT_T* ptInitOpt);

/**
 * @brief Function to release VMF Video Output
 *
 * @param[in] ptHandle A pointer to the VMF_VO_HANDLE_T structure.
 * @r@return Success: 0  Fail: negative integer.
 * @note This is NOT thread-safe.
 */
int VMF_VO_Release(VMF_VO_HANDLE_T* ptHandle);

/**
 * @brief Function to start VMF Video Output
 *
 * @param[in] ptHandle A pointer to the VMF_VO_HANDLE_T structure.
 * @r@return Success: 0  Fail: negative integer.
 * @note This is NOT thread-safe.
 */
int VMF_VO_Start(VMF_VO_HANDLE_T* ptHandle);

/**
 * @brief Function to stop VMF Video Output
 *
 * @param[in] ptHandle A pointer to the VMF_VO_HANDLE_T structure.
 * @r@return Success: 0  Fail: negative integer.
 * @note This is NOT thread-safe.
 */
int VMF_VO_Stop(VMF_VO_HANDLE_T* ptHandle);

/**
 * @brief Function to enqueue VMF Video Output buffer
 *
 * @param[in] ptHandle A pointer to the VMF_VO_HANDLE_T structure.
 * @param[in] ptVBuf A pointer to the VMF_VIDEO_BUF_T structure.
 * @param[in] dwID A data for queue index.
 * @r@return Success: 0  Fail: negative integer.
 * @note This is NOT thread-safe.
 */
int VMF_VO_QueueBuff(VMF_VO_HANDLE_T* ptHandle, VMF_VIDEO_BUF_T* ptVBuf, unsigned int dwID);

/**
 * @brief Function to dequeue VMF Video Output
 *
 * @param[in] ptHandle A pointer to the VMF_VO_HANDLE_T structure.
 * @param[in] pdwID A pointer to dequeue index.
 * @r@return Success: 0  Fail: negative integer.
 * @note This is NOT thread-safe.
 */
int VMF_VO_DequeueBuff(VMF_VO_HANDLE_T* ptHandle, unsigned int *pdwID);

#ifdef __cplusplus
}
#endif

#endif //! guard VIDEO_OUTPUT_H
