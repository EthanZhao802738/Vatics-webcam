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
#ifndef CONFIG_AE_H
#define CONFIG_AE_H

#include <comm/video_buf.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * AE option flag enumeration 
 */ 
typedef enum
{
	//! AE option flag: reset
	AE_OPTION_RESET,

	//! AE option flag: converge speed
	AE_OPTION_CONVERGE_SPEED,

	//! AE option flag: control
	AE_OPTION_CONTROL,

	//! AE option flag: autoscene
	AE_OPTION_AUTOSCENE,

	//! AE option flag: sensor info
	AE_OPTION_SENSOR_INFO,

	//! AE option flag: statistic range
	AE_OPTION_STATISTIC_RANGE,

	//! AE option flag: target exchange
	AE_OPTION_TARGET_EXCHANGE,

	//! AE option flag: ai control
	AE_OPTION_AI_CONTROL,
	
	//! ASC task option: reserved
	VMF_AE_RESERVED,
	
	//! AE option flag: iris control info
	AE_OPTION_IRIS_CONTROL,

	//! AE option flag: current shutter info
	AE_OPTION_CURR_SHUTTER,

	//! Ifp option: reserved max
	VMF_AE_RESERVED_MAX
} VMF_AE_OPTION_FLAG_T;

/*
 * A data structure for AE general option
 */
typedef struct
{
	//! A data for option flags
    VMF_AE_OPTION_FLAG_T eOptionFlags;

    //! A data for AE handle index (Available in dual lens 360 mode)
	unsigned int dwIndex;

    //!  A pointer data for user data
    void *pData; 
} VMF_AE_OPTION_T;

/*
 * A data structure for AE in autoscene option, AE_OPTION_AUTOSCENE
 */
typedef struct
{
	//! A data for power frequence of video signal : 1:50HZ, 2:60HZ, 3:24HZ, 4:30HZ 
	VMF_VIDEO_SIGNAL_FREQUENCY eVideoSignalFreq;

	//! A data for AE control mode : 0: Auto, 1: Black light, 2: Customized 
	unsigned int dwMode;

	//! A data for AE lock : 1: AE lock, 0: AE unlock 
	unsigned int bLock;

	//! A data for AE's target luma : range: 0~255 
	unsigned int dwTargetLuma;

	//! A data for AE's target offset : range 0~255 
	unsigned int dwTargetOffset;

	//! A data for AE's action range with min shutter speed : range 1~1000000,  1000000 = 1 second 
	unsigned int dwMinShutter;

	//! A data for AE's action range with max shutter speed : range 1~1000000,  1000000 = 1 second 
	unsigned int dwMaxShutter;

	//! A data for AE's action range with min gain : range 1~128,  1 = 1x gain 
	unsigned int dwMinGain;

	//! A data for AE's action range with max gain : range 1~128,  1 = 1x gain
	unsigned int dwMaxGain;

	//! A data for iris status : 0: fixed to largest, 1: auto iris, 2: manual iris
	unsigned int dwIrisStatus;

	//! A data for active shutter speed for auto iris conrol when dwIrisStatus = 1 : range 1~1000000,  1000000 = 1 second
	unsigned int dwIrisActiveTime;

	//! A data for WDR ratio : range 1,2,4,8,16,32,64,128,256
	unsigned int dwWdrRatio;
} VMF_AE_ASC_PARAM_T;

/*
 * A data structure for AE ctrl option, AE_OPTION_CONTROL
 */  
typedef struct
{
	//! A data for Video Signal Frequency
    unsigned int dwVideoSignalFreq;

    //! A data for Metering type
    unsigned int dwMeteringType;

    //! A data for AE luma target
    unsigned int dwTarget;	

    //! A data for AE luma convergence range
    unsigned int dwOffset;	

    //! A data for max system gain  		
    unsigned int dwMaxGain;	

    //! A data for min system gain  	
    unsigned int dwMinGain;	

    //! A data for max sensor shutter  		
    unsigned int dwMaxShutter;	

    //! A data for min sensor shutter  	
    unsigned int dwMinShutter;	

    //! A data for WDR enable. (0: Linear mode(WDR off), 1: WDR mode(WDR on))
    unsigned int bWdrEn;

    //! A data for WDR ratio
    unsigned int dwWdrRatio;

    //! A data for exposure mode
    unsigned int dwExposureMode;

    //! A data for the ROI percentage for short exposure histogram which will be taken into consideration (from histogram brightness part)
    unsigned int dwDescentRoiInShortExpHist;

    //! A data for minimum WDR output ratio
    unsigned int dwMinOutputWdrRatio;

    //! A data for maximum WDR output ratio
    unsigned int dwMaxOutputWdrRatio;

    //! A data for luma target of WDR short exposure
    unsigned int dwWdrShortExpTarget;

    //! A data for luma convergence range of WDR short exposure
    unsigned int dwWdrShortExpConvergeRange;

    //! A data for stepping size of WDR convergence
    unsigned int dwWdrConvergeStepSize;	

    //! A data for Update Win Priority	 
    unsigned int bUpdateWinPriority;

    //! A pointer data for Win Priority	 
    unsigned int* pdwWinPriority;
} VMF_AE_CTRL_OPTION_T;

/*
 * A data structure for AE sensor info option, AE_OPTION_SENSOR_INFO
 */ 
typedef struct
{
	//! A data for AE's sensor info range with min shutter  
    unsigned int dwMinShutter;

    //! A data for AE's sensor info range with max shutter  
    unsigned int dwMaxShutter;

    //! A data for AE's sensor info range with min gain 
    unsigned int dwMinGain;

    //! A data for AE's sensor info range with max gain 
    unsigned int dwMaxGain;

    //! A data for min expect step 
    unsigned int dwMinExpStep;

    //! A data for min gain step 
    unsigned int dwMinGainStep;

    //! A data for frame rate
    unsigned int dwFrameRate;
} VMF_AE_SENSOR_INFO_OPTION_T;

/*
 * A data structure for AE converge speed option, AE_OPTION_CONVERGE_SPEED
 */ 
typedef struct
{
	//! A data for AE speed
    unsigned int dwSpeed;
} VMF_AE_SPEED_OPTION_T;

/*
 * A data structure for AE target exchange option, AE_OPTION_TARGET_EXCHANGE
 */
typedef struct
{
	//! A data for AE's target offset
    int iAETargetOffset;

    //! A data for AE's target high gain
    unsigned int dwHighGain;

    //! A data for AE's target low gain
    unsigned int dwLowGain;
} VMF_AE_TARGET_EXCHANGE_OPTION_T;

/*
 * A data structure for AE statistic range option, AE_OPTION_STATISTIC_RANGE
 */
typedef struct
{
	//! A data for auto start ratio 
    unsigned int dwAutoStartRatio;

    //! A data for auto end ratio 
    unsigned int dwAutoEndRatio;

    //! A data for BLight  start ratio
    unsigned int dwBLightStartRatio;

    //! A data for BLight end ratio
    unsigned int dwBLightEndRatio;
} VMF_AE_STATS_RANGE_OPTION_T;

/*
 * A data structure for AE auto iris option, AE_OPTION_AI_CONTROL
 */
typedef struct
{
	//! A data for auto iris enable
    unsigned int bAutoIrisEn;

    //! A data for AI active time
    unsigned int dwAIActiveTime;
} VMF_AE_AUTO_IRIS_OPTION_T;

/*
 * A data structure for AE iris control info
 */
typedef struct
{
    //! A data for iris speed
    unsigned int dwSpeed;
} VMF_AE_IRIS_CONTROL_INFO_T;

/*
 * A data structure for AE current shutter info
 */
typedef struct
{
    //! A data for current shutter
    unsigned int dwCurrShutter;

	 //! A data for current sensor gain
    unsigned int dwCurrSensorGain;

	 //! A data for current ISP gain
    unsigned int dwCurrISPGain;
} VMF_AE_CURR_SHUTTER_INFO_T;

#ifdef __cplusplus
}
#endif

#endif
