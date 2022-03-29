
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
#ifndef VIDEO_BIND_H
#define VIDEO_BIND_H
#include <vmf/source_connect.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define VBIND_STRIDE_ALIGN 15
#define VBIND_STRIDE_ALIGN 31 //! stride of h265 have to do with 32 alignment

typedef struct vmf_bind_context_t VMF_BIND_CONTEXT_T;

typedef int (*VMF_BIND_QUERY_FUNC)(void* ptSrcHandle, void* ptQueryInfo);

typedef int (*VMF_BIND_CONFIG_ISP_FUNC)(void* ptSrcHandle, unsigned int dwIndex, unsigned int dwLayer, int dwIspIndex, void* ptOption);

/*
 * A data structure for bind initial option
 */
typedef struct
{
	//! A data for source output index
	unsigned int dwSrcOutputIndex;

	//! A pointer data for source handle
	void* ptSrcHandle;

	//! A data for bind query function
	VMF_BIND_QUERY_FUNC pfnQueryFunc;

	//! A data for bind isp function
	VMF_BIND_CONFIG_ISP_FUNC pfnIspFunc;
} VMF_BIND_INITOPT_T;

/**
 * @brief Function to initial bind device
 *
 * @param[in] ptInitOpt The initial options about bind.
 * @return Success: The pointer of VMF_BIND_CONTEXT_T  Fail: NULL.
 */
VMF_BIND_CONTEXT_T* VMF_BIND_Init(const VMF_BIND_INITOPT_T* ptInitOpt);

/**
 * @brief Function to release bind device
 *
 * @param[in] ptContext The bind context.
 * @return Success: 1  Fail: negative integer.
 */
int VMF_BIND_Release(VMF_BIND_CONTEXT_T* ptContext);

/**
 * @brief Function to request source output stream.
 *
 * @param[in] ptContext The bind context.
 * @param[in] dwWidth The video source output width.
 * @param[in] dwHeight The video source output height.
 * @param[in] dwStride The video source output Stride.
 * @param[in] ptConnectInfo The bind device info.
 * @return Success: 1  Fail: negative integer.
 */
int VMF_BIND_Request(VMF_BIND_CONTEXT_T* ptContext, unsigned int dwWidth, unsigned int dwHeight,
	unsigned int dwStride, unsigned int dwFps, VMF_SRC_CONNECT_INFO_T* ptConnectInfo);

#ifdef __cplusplus
}
#endif

#endif
