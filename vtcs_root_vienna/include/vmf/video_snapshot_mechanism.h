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
#ifndef VIDEO_SNAPSHOT_MECHANISM_H
#define VIDEO_SNAPSHOT_MECHANISM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ssm_info.h>
#include <sync_shared_memory.h>


#define ISP_PARAMS_PATH_LENGTH_MAX 127

typedef struct VMF_SNAP_HANDLE_T VMF_SNAP_HANDLE_T;

typedef struct {
	unsigned int dwStartX;
	unsigned int dwStartY;
	unsigned int dwCropWidth;
	unsigned int dwCropHeight;
	void* pOutBuffer;
}VMF_SNAP_CROP_PARAMS_T;

/*
 * A data Structure for SNAP Init
 */
typedef struct {
	//! The Output pin of isp  
	const char *pszOutPinPrefix;    
	//! The index of stream.
	unsigned int dwStreamIdx;	
	//! The pointer to video source handle	
	void *pVsrcHandle;              
} VMF_SNAP_INITOPT_T;

/**
 * @brief Function to initial SNAP device
 *
 * @param[in] ptOpt The initial options about snap.
 * @return Success: VMF_SNAP_HANDLE_T  Fail: NULL.
 */
VMF_SNAP_HANDLE_T* VMF_SNAP_Init(const VMF_SNAP_INITOPT_T* ptOpt);

/**
 * @brief Function to release a snap object.
 *
 * @param[in] ptHandle The handle of snap object.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_SNAP_Release(VMF_SNAP_HANDLE_T* ptHandle);

/**
 * @brief Start processing snap
 *
 * @param[in] ptSnapHandle The handle of snap object.
 * @param[in] dwOutWidth The rs width of snap.
 * @param[in] dwOutHeight The rs height of snap.
 * @param[in] pOutBuffer The output buffer of snap.
 * @return Success: jpeg size  Fail: negative integer.
 */
int VMF_SNAP_ProcessOneFrame(VMF_SNAP_HANDLE_T* ptSnapHandle, unsigned int dwOutWidth, unsigned int dwOutHeight, void* pOutBuffer);

/**
 * @brief Start processing snap
 *
 * @param[in] ptSnapHandle The handle of snap object.
 * @param[in] ptCropInfo The crop infomation of snap.
 * @return Success: jpeg size  Fail: negative integer.
 */
int VMF_SNAP_ProcessOneFrame_AREA(VMF_SNAP_HANDLE_T* ptSnapHandle, VMF_SNAP_CROP_PARAMS_T* ptCropInfo);


int VMF_SNAP_ProcessOneFrame_YUV(SSM_BUFFER_T* ptSsmBuff, VMF_SNAP_CROP_PARAMS_T* ptCropInfo);


#ifdef __cplusplus
}
#endif

#endif

