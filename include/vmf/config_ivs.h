
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
#ifndef CONFIG_IVS_H
#define CONFIG_IVS_H

#include <comm/video_buf.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VMF_MAX_MD_WIDOW_SIZE 25
#define VMF_MAX_OT_WIN_NUM 4
#define VMF_MAX_OM_WIN_NUM 3

#define IVS_CONFIG_MAX 5


/*
 * IVS option flag
 */
typedef enum
{
	IVS_TD_CONFIG, //! ivs config flag: tamper detection
	IVS_MD_CONFIG, //! ivs config flag: motion detection
	IVS_OT_CONFIG, //! ivs config flag: object tracking
	IVS_OM_CONFIG, //! ivs config flag: object missing
} VMF_IVS_CONFIG_FLAG;

typedef enum
{
	VMF_IVS_OM_MODE_NORMAL = 0,
	VMF_IVS_OM_MODE_TAMPER
} VMF_IVS_OM_MODE_TYPE;

/*
 * A structure for IVS configuration
 */
typedef struct
{
	VMF_IVS_CONFIG_FLAG eIvsFlag;  //! IVS config flag
	unsigned int dwIvsIndex;  //! IVS index
	void* pConfig; //! Pointer to IVS config
} VMF_IVS_CONFIG_T;

/**
 * A structure for IVS tamper detection information
 */
typedef struct
{
	unsigned int dwSec;  //! Tamper detected second
	unsigned int dwUsec; //! Tamper detected usec
	unsigned int bTamperDetected; //! Is tamper Detected
} VMF_IVS_TD_INFO_T;

/**
 * A structure for IVS motion detection information
 */
typedef struct
{
	unsigned int dwSec;  //! Motion detected second
	unsigned int dwUsec; //! Motion detected usecond
	unsigned char abMotionTrigger[VMF_MAX_MD_WIDOW_SIZE]; //! Motion trigger flag
} VMF_IVS_MD_INFO_T;

/**
 * A structure for IVS object tracking output information
 */
typedef struct
{
	unsigned int dwCentX;	//! OT detected center X
	unsigned int dwCentY;	//! OT detected center Y
	unsigned int dwMaxDist;	//! OT detected max distance
} VMF_IVS_OT_OUTPUT_T;

/**
 * A structure for IVS object tracking information
 */
typedef struct
{
	unsigned int sec;	//! OT detected second
	unsigned int usec;	//! OT detected usecond
	VMF_IVS_OT_OUTPUT_T atOtOutput[VMF_MAX_OT_WIN_NUM]; //! Object Tracking
} VMF_IVS_OT_INFO_T;

typedef struct
{
	unsigned int abEventMotion[VMF_MAX_OM_WIN_NUM];	//! Object Missing detected windows
	unsigned int abEventMiss[VMF_MAX_OM_WIN_NUM];	//! Object Missing miss windows	
	unsigned int abEventAlert[VMF_MAX_OM_WIN_NUM];	//! Object Missing alert windows
	unsigned int abEventResume[VMF_MAX_OM_WIN_NUM];	//! Object Missing resume windows
} VMF_IVS_OM_INFO_T;

typedef void (*VMF_IVS_TD_INFO_FUNC)(VMF_IVS_TD_INFO_T* ptTdInfo);
typedef void (*VMF_IVS_MD_INFO_FUNC)(VMF_IVS_MD_INFO_T* ptMdInfo);
typedef void (*VMF_IVS_OT_INFO_FUNC)(VMF_IVS_OT_INFO_T* ptOtInfo);
typedef void (*VMF_IVS_OM_INFO_FUNC)(VMF_IVS_OM_INFO_T* ptOmInfo);

/**
 * A structure for IVS tamper detection config
 */
typedef struct
{
	VMF_IVS_TD_INFO_FUNC pfnTdCallback; //! Tamper detection callback function
	unsigned int bOutFocusEn;           //! Out of focus detection enable flag
	unsigned int dwOutFocusSensitivity; //! Out of focus sensitivity
	unsigned int dwOutFocusStrength;    //! Out of focus strength
	unsigned int bMaskDetEn;            //! Mask detection enable flag
	unsigned int dwMaskDetSensitivity;  //! Mask detection sensitivity
} VMF_IVS_TD_CONFIG_T;

/**
 * A structure for IVS motion detection window
 */
typedef struct
{
	unsigned int bEnable;   //! Motion window enable flag
	unsigned int dwStartX;  //! Horizontal start position of window
	unsigned int dwStartY;  //! Vertical start position of window
	unsigned int dwWidth;   //! Window width
	unsigned int dwHeight;  //! Window height
	unsigned int dwObjSize; //! object blocks threshold: target 8x8 block count
} VMF_IVS_MD_WINDOW_T;

/**
 * A structure for IVS motion detection configuration
 */
typedef struct
{
	unsigned int bMdEnable;  //! Motion detection enable flag
	VMF_IVS_MD_INFO_FUNC pfnMdCallback; //! Motion detection callback function
	VMF_IVS_MD_WINDOW_T atMdWindows[VMF_MAX_MD_WIDOW_SIZE]; //! Motion window configuration array
} VMF_IVS_MD_CONFIG_T;

/**
 * A structure for IVS object tracking window
 */
typedef struct
{
	VMF_IVS_OT_INFO_FUNC pfnOt1ObjCallback;
	VMF_IVS_OT_INFO_FUNC pfnOt4ObjCallback;

	unsigned int  dwStartX; 		//!  A data for object track Start X : (range 0 ~ 8191)
	unsigned int  dwStartY; 		//!  A data for object track Start Y : (range 0 ~ 8191)
	unsigned int  dwMinMotion;      //!  A data for object track MinMotion : (range 0 ~ 15)
	unsigned char bObjectTrackEn;   //!  A data for object track function enable : (range 0 ~ 1)
	unsigned char bMotionValidMapEn;//!  A data for object track valid motion map enable : (range 0 ~ 1)
	unsigned char bSingleOutEn;     //!  A data for object track single view enable : (range 0 ~ 1)
} VMF_IVS_OT_CONFIG_T;

/**
 * A structure for IVS object missing window
 */
typedef struct
{
	unsigned int bEnable;			 //! This is used to enable/disable OM function
	unsigned int dwStartX;			 //! Detection window of start position x
	unsigned int dwStartY;			 //! Detection window of start position y
	unsigned int dwWidth;			 //! This specifies the width of detection window
	unsigned int dwHeight;			 //! This specifies the height of detection window
	unsigned int dwLocalThreshold;   //! Pixel mismatch threshold, Range: 0-255 (more sensitive as number becomes smaller)
	unsigned int dwGlobalSensitivity;//! ROI mismatch threshold, Range: 0-63 (more sensitive as number becomes greater)
	unsigned int dwMissTimer;		 //! Missing time to trigger missed state (frame based)
	unsigned int dwAlertTimer;		 //! Numbers of frame intervals to trigger alert
	unsigned int dwResumeTimer;		 //! Resume object miss detect time
	VMF_IVS_OM_MODE_TYPE eMode; 	 //! A data for object miss detect mode
} VMF_IVS_OM_WINDOW_T;

/**
 * A structure for IVS object missing resize data
 */
typedef struct
{
	unsigned char *pbyRsFrameY;   //! A data pointer for current re-size smaller Y frame
	unsigned char *pbyRsFrameU;   //! Reserved value
	unsigned char *pbyRsFrameV;   //! Reserved value
	unsigned int dwRsFrameWidth;  //! Width of current re-size Y frame
	unsigned int dwRsRrameHeight; //! Height of current re-size Y frame
} VMF_IVS_OM_RESIZE_DATA_T;

/**
 * A structure for IVS object missing configuration
 */
typedef struct
{
	unsigned int bOmEnable;  			//! Object missing enable flag
	VMF_IVS_OM_INFO_FUNC pfnOmCallback;	//! Function pointer of callback function for missing detection
	VMF_IVS_OM_WINDOW_T atOmWindows[VMF_MAX_OM_WIN_NUM]; //! This is used to define object missing windows
	VMF_IVS_OM_RESIZE_DATA_T tRsData;	//! This is used to set smaller y frame information
	unsigned int bUpdateRsData;   		//! Update the Rs data
} VMF_IVS_OM_CONFIG_T;

#ifdef __cplusplus
}
#endif

#endif

