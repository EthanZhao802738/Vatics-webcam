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
#ifndef CONFIG_ASC_H
#define CONFIG_ASC_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Autoscene task option flag enumeration
 */
typedef enum
{
	//! ASC task option flag: asc
	VMF_ASC_TASK_OPTION_ASC,

	//! ASC task option flag: ae
	VMF_ASC_TASK_OPTION_AE,

	//! ASC task option flag: awb
	VMF_ASC_TASK_OPTION_AWB,
} VMF_ASC_TASK_OPTION_FLAG_T;

/*
 * A data structure for autoscene task general option
 */
typedef struct
{
	//! A data for ASC Task option flag
    VMF_ASC_TASK_OPTION_FLAG_T eOptionFlags;
   	
   	//! A pointer to user data
    void* pData; 
} VMF_ASC_TSK_OPTION_T;

/*
 * Auto scene option flag enumeration
 */
typedef enum
{
	//! ASC option flag: set frequency
	ASC_OPTION_SET_FREQUENCY,

	//! ASC option flag: auto exposure mode
	ASC_OPTION_SET_AUTO_EXPOSURE_MODE,

	//! ASC option flag: set exposure level
	ASC_OPTION_SET_EXPOSURE_LEVEL,

	//! ASC option flag: set exposure min shutter
	ASC_OPTION_SET_AUTO_EXPOSURE_MIN_SHUTTER,

	//! ASC option flag: set exposure max shutter
	ASC_OPTION_SET_AUTO_EXPOSURE_MAX_SHUTTER,

	//! ASC option flag: set exposure min gain
	ASC_OPTION_SET_AUTO_EXPOSURE_MIN_GAIN,

	//! ASC option flag: set exposure max gain
	ASC_OPTION_SET_AUTO_EXPOSURE_MAX_GAIN,

	//! ASC option flag: set slow frame rate
	ASC_OPTION_SET_SLOW_FRAME_RATE,

	//! ASC option flag: set wdr ratio
	ASC_OPTION_SET_WDR_RATIO,

	//! ASC option flag: set iris mode
	ASC_OPTION_SET_IRIS_MODE,

	//! ASC option flag: set auto iris active time
	ASC_OPTION_SET_AUTO_IRIS_ACTIVE_TIME,

	//! ASC option flag: set auto exposure lock
	ASC_OPTION_SET_AUTO_EXPOSURE_LOCK,

	//! ASC option flag: set auto white balance mode 
	ASC_OPTION_SET_AUTO_WHITE_BALANCE_MODE,

	//! ASC option flag: set auto white balance lock
	ASC_OPTION_SET_AUTO_WHITE_BALANCE_LOCK,

	//! ASC option flag: set blight level
	ASC_OPTION_SET_BRIGHT_LEVEL,

	//! ASC option flag: set contrast level
	ASC_OPTION_SET_CONTRAST_LEVEL,

	//! ASC option flag: set saturation level
	ASC_OPTION_SET_SATURATION_LEVEL,

	//! ASC option flag: set noise reduction mode
	ASC_OPTION_SET_NOISE_REDUCTION_MODE,

	//! ASC option flag: set 2d noise reduction level
	ASC_OPTION_SET_2D_NOISE_REDUCTION_LEVEL,

	//! ASC option flag: set 3d noise reduction level
	ASC_OPTION_SET_3D_NOISE_REDUCTION_LEVEL,

	//! ASC option flag: set sharpness level
	ASC_OPTION_SET_SHARPNESS_LEVEL,

	//! ASC option flag: set tm level
	ASC_OPTION_SET_TM_LEVEL,

	//! ASC option flag: set hue level
	ASC_OPTION_SET_HUE_LEVEL,

	//! ASC option flag: set mono
	ASC_OPTION_SET_MONO,

	//! ASC option flag: set hight light compress
	ASC_OPTION_SET_HIGH_LIGHT_COMPRESS,

	//! ASC option flag: set defog
	ASC_OPTION_SET_DEFOG,

	//! ASC option flag: set ltm ccm disconnect
	ASC_OPTION_SET_LTM_CCM_DISCONNECT,

	//! ASC option flag: set sgc ration
	ASC_OPTION_SET_SGC_RATIO,

	//! ASC option flag: set dgc
	ASC_OPTION_SET_DGC,

	//! ASC option
	NUM_OF_ASC_OPTION,

	//! ASC option flag: set debug
	ASC_OPTION_SET_DEBUG,

	//! ASC option flag: set reload reference file
	ASC_OPTION_SET_RELOAD_REFERENCE_FILE,

	RESERVED,
	
	//! ASC option flag: set dualcam sync
	ASC_OPTION_SET_DUALCAM_SYNC,
	
	//! ASC option flag: set autoscene mode
	ASC_OPTION_SET_MODE,		
} VMF_ASC_OPTION_FLAG_T;

/*
 * A data structure for autoscene general option
 */
typedef struct
{
	//! A data for  ASC option flag
	VMF_ASC_OPTION_FLAG_T eOptionFlags;

	//! A data for ASC handle index (Available in dual lens 360 mode)
	unsigned int dwIndex;

	//! A pointer data to user data
	void *pData; 
} VMF_ASC_OPTION_T;
/*
 * A data structure for auto white balance mode option, ASC_OPTION_SET_AUTO_WHITE_BALANCE_MODE
 */
typedef struct
{
	//! A data for AWB mode : (0:auto, 1:full, 2:customized, 3:push_hold)
	unsigned int dwAwbMode;

	//! A data for AWB red gain in customized mode (range:1~8191, 1024=x1)
	unsigned int dwCustomRGain;

	//! A data for AWB blue gain in customized mode (range:1~8191, 1024=x1)
	unsigned int dwCustomBGain;
} VMF_ASC_AWB_MODE_PARAM_T;

/*
 * A data structure for high light compression option, ASC_OPTION_SET_HIGH_LIGHT_COMPRESS
 */
typedef struct
{
	//! A data for HLC enable : (1: enable, 0: disable)
	unsigned int bEnable;

	//! A data for HLC manual enable : (1: manual, 0: auto)
	unsigned int bManual;

	//! A data for HLC mask : (1: enable, 0: disable)
	unsigned int bMask;

	//! A data for HLC level : (range: 0~100)
	unsigned int dwLevel;
} VMF_ASC_HLC_PARAM_T;

/*
 * A data structure for defog option, ASC_OPTION_SET_DEFOG
 */
typedef struct
{
	//! A data for defog enable : (1: enable, 0: disable)
	unsigned int bEnable;

	//! A data for defog level : (range: 0~100)
	unsigned int dwLevel;

	//! A data for defog sensitivity : (range: 0~31)
	unsigned int dwSensitivity;
} VMF_ASC_DEFOG_PARAM_T;

/*
 * A data structure for static gain control (SGC), ASC_OPTION_SET_SGC_RATIO
 */
typedef struct
{
	//! A data for SGC enable : (1: enable, 0: disable)
	unsigned int bEnable;

	//! A data array for SGC red color ratio : (range: 0~4096,  1024 = 1x) 
	unsigned int dwRedRatio[2];

	//! A data array for SGC green color ratio : (range: 0~4096,  1024 = 1x)
	unsigned int dwGreenRatio[2];

	//! A data array for SGC blue color ratio : (range: 0~4096,  1024 = 1x)
	unsigned int dwBlueRatio[2];
} VMF_ASC_SGC_PARAM_T;

/*
 * A data structure for dynamic gain control (DGC), ASC_OPTION_SET_DGC
 */
typedef struct
{
	//! A data for DGC enable : (1: enable, 0: disable)
	unsigned int bEnable;

	//! A data array for DGC ROI horizontal window numbers
	unsigned int dwRoiHorNum[2];

	//! A data array for DGC ROI vertical window numbers
	unsigned int dwRoiVerNum[2];

	//! A pointer data array for DGC ROI Mask buffer
	unsigned char *pbyRoiMaskBuf[2];

	//! A data for DGC operation per frame
	unsigned int dwOpFrames;
} VMF_ASC_DGC_PARAM_T;

#ifdef __cplusplus
}
#endif

#endif
