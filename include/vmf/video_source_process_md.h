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
#ifndef VIDEO_SOURCE_PROCESS_MD_H
#define VIDEO_SOURCE_PROCESS_MD_H

#ifdef __cplusplus
extern "C" {
#endif

#define VMF_MAX_ISP_MD_WIDOW_SIZE (32)
#define VMF_MAX_ISP_MD_POINT_SIZE (6)

typedef struct vmf_isp_md_handle_t VMF_ISP_MD_HANDLE_T;

typedef struct vmf_isp_md_context_t VMF_ISP_MD_CONTEXT_T;

/**
 * A structure for Isp Map point
 */	
typedef struct  
{ 
	//! A data for point x
    unsigned int x; 
    //! A data for point y
    unsigned int y; 
}POINT_T;

/**
 * A structure for isp motion detection information
 */
typedef struct
{
	//! Motion detected second
	unsigned int dwSec;
	//! Motion detected usecond  
	unsigned int dwUsec;
	//! Motion trigger flag
	unsigned char abMotionTrigger[VMF_MAX_ISP_MD_WIDOW_SIZE]; 
} VMF_ISP_MD_INFO_T;

/**
 * A enumeration for isp motion detection type
 */
typedef enum
{	
	//! ISP motion detection type: window
	VMF_ISP_MD_CONFIG_WINDOW = 0,
	//! ISP motion detection type: map
	VMF_ISP_MD_CONFIG_MAP,
	//! ISP motion detection type: unsupport mode
	VMF_ISP_MD_UNSUPPORT_CHANGE_MODE,
} VMF_ISP_MD_TYPE;

typedef int (*VMF_ISP_MD_INFO_FUNC)(VMF_ISP_MD_INFO_T* ptIspMdInfo);

typedef int (*VMF_ISP_MD_PROC_FUNC)(VMF_ISP_MD_CONTEXT_T* ptIspMdContext);

typedef int (*VMF_ISP_MD_CORE_PROC_FUNC)(void* ptHandle);

/**
 * A structure for Isp motion detection window
 */
typedef struct
{
	//! Motion window enable flag, 0: diable, 1: enable
	unsigned int bEnable;   
	 //! Horizontal start position of window
	unsigned int dwStartX; 
	//! Vertical start position of window
	unsigned int dwStartY; 
	//! Window width 
	unsigned int dwWidth;
	//! Window height   
	unsigned int dwHeight;  
	//! Window Threshold: 0~100%
	unsigned int dwWindowThr; 
	//! Window Object size: 0~100%
	unsigned int dwObjectSize;
} VMF_ISP_MD_WINDOW_T;

/**
 * A structure for Isp motion detection map
 */
typedef struct
{ 
	//! A data for map points
	POINT_T atPoints[VMF_MAX_ISP_MD_POINT_SIZE];
	//! A data for map threshold: 0~100%
	unsigned int dwMapThr;
	//! A data for map object size: 0~100%
	unsigned int dwObjectSize;
} VMF_ISP_MD_MAP_T;

/**
 * A structure for isp motion detection configuration
 */
typedef struct
{
	//! Motion detection type
	VMF_ISP_MD_TYPE eMdType;
	//! Motion detection enable flag
	unsigned int bMdEnable; 
	//! Motion dection frame count number
	unsigned int dwMotionDetectionCount; 
	//! Motion detection callback function
	VMF_ISP_MD_INFO_FUNC pfnMdCallback; 
	//! Motion detection configuration data: window data/map data
	void *pMDData;
	//! Motion detection data size 
	//! Map mode: only support one map. Window mode: support max 32 window
	unsigned int dwDataSize;
} VMF_ISP_MD_CONFIG_T;

#ifdef __cplusplus
}
#endif

#endif
