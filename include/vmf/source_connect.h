
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
#ifndef SOURCE_CONNECT_H
#define SOURCE_CONNECT_H

#include <comm/video_buf.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * A data structure for source connect info
 */
typedef struct
{
	//! A data array for source pin
	char szSrcPin[VMF_MAX_SSM_NAME_SIZE];

	//! A data for source width
	unsigned int dwSrcWidth;

	//! A data for source height
	unsigned int dwSrcHeight;

	//! A data for source Y stride
	unsigned int dwSrcYStride;

	//! A data for source uv stride
	unsigned int dwSrcUVStride;

	//! A data for type of frame data
	unsigned int dwDataType;

	//! A data for resized source
	unsigned int bUseResizedSrc;

	//! A data for ssm shared
	unsigned int bIsSsmShared;

	//! A data for connect ifp
	unsigned int bConnectIfp;

	//! A data for codec type
	unsigned int dwCodecType;

	//! A data for disable shared osd flag
	unsigned int bDisableSharedOsd;

	//! A data for encoder handle
	void* ptVencHandle;

	//! A data for unregister. 0: register, 1: unregister
	unsigned int bUnregister;
} VMF_SRC_CONNECT_INFO_T;

typedef int (*VMF_SRC_CONNECT_FUNC)(void* pBind,
	unsigned int dwReqWidth, unsigned int dwReqHeight, unsigned int dwReqStride, unsigned int dwFps, VMF_SRC_CONNECT_INFO_T* ptConnectInfo);

#ifdef __cplusplus
}
#endif

#endif
