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

#ifndef VIDEO_ENCODER_ADJUST_H
#define VIDEO_ENCODER_ADJUST_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vmf/video_encoder.h>
#include <comm/vmf_log.h>
#include <SyncRingBuffer/sync_ring_buffer.h>
#include <comm/frame_info.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * A structure for real drop frame configuration depend
 */
typedef struct
{
	//! encoder handle
	VMF_VENC_HANDLE_T *ptVencHandle;

	//! real drop frame rate levels, ex: 30, 15, 10
	unsigned int *pdwFpsLevels;
	
	//! element count of FpsLevels, ex: for {30, 15, 10}, dwFpsLevelsSize is 3
	unsigned int dwFpsLevelsSize;

	//! Target bitrate. It will not change the original bitrate if this value equals to zero.
	unsigned int dwBitrate;
	
	//! Overflow percentage of target bitrate that triggers frame dropping and change to a lower frame rate. Range: 0~100.
	unsigned int dwBitrateOverflowPercentage;
	
	//! Underflow percentage of target bitrate to trigger frame dropping and change to a higher frame rate. Range: 0~100.
	unsigned int dwBitrateUnderflowPercentage;
	
	//! benable/disable this feature, 1: benable, 0: disable
	unsigned int benable;							
} VMF_VENC_ADJ_RDF_CONFIG_T;

typedef struct venc_adj_handle_t VENC_ADJ_HANDLE_T;

//int VMF_VENC_adjust_FPS(VMF_VENC_ENCODE_INFO_T* ptEncodeInfo, void* pUserdata);

/**
 * @brief Function to initialize VMF video encoder adjust
 *
 * @param[in] ptConfig Video encoder adjust configuration.
 * @return The handle of VMF video encoder adjust.
 */
VENC_ADJ_HANDLE_T* VMF_VENC_ADJ_Init(VMF_VENC_ADJ_RDF_CONFIG_T *ptConfig);

/**
 * @brief Function to release VMF video encoder adjust
 *
 * @param[in] ptAdjHandle The handle of VMF video encoder adjust.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VENC_ADJ_Release(VENC_ADJ_HANDLE_T *ptAdjHandle);

#ifdef __cplusplus
}
#endif

#endif //! guard
