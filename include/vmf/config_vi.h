
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
#ifndef CONFIG_VI_H
#define CONFIG_VI_H

#include <comm/video_buf.h>

#define VMF_VI_IDX_ALL 0xFFFFFFFF
#define VMF_VI_MAX_CH_NUM 5

#ifdef __cplusplus
extern "C" {
#endif

/*
 * video option flag
 */
typedef enum
{
	//! video option: reserved min
	VI_OPTION_RESERVED_MIN,

	//! video option: reset
	VI_OPTION_RESET,

	//! video option: set brightness
	VI_OPTION_SET_BRIGHTNESS,

	//! video option: set contrast
	VI_OPTION_SET_CONTRAST,

	//! video option: set hue
	VI_OPTION_SET_HUE,

	//! video option: set saturation
	VI_OPTION_SET_SATURATION,

	//! video option: set color temperature
	VI_OPTION_SET_COLOR_TEMPERATURE,

	//! video option: set auto exposure control
	VI_OPTION_SET_AUTO_EXPOSURE_CTRL,

	//! video option: set auto gain control
	VI_OPTION_SET_AUTO_GAIN_CTRL,

	//! video option: set auto white balance control
	VI_OPTION_SET_AUTO_WHITE_BALANCE_CTRL,

	//! video option: set auto brightness control
	VI_OPTION_SET_AUTO_BRIGHTNESS_CTRL,

	//! video option: set frequency
	VI_OPTION_SET_FREQUENCY,

	//! video option: set flip
	VI_OPTION_SET_FLIP,

	//! video option: set mirror
	VI_OPTION_SET_MIRROR,

	//! video option: set mono
	VI_OPTION_SET_MONO,

	//! video option: set low pass filter
	VI_OPTION_SET_LOW_PASS_FILTER,

	//! video option: set capure area
	VI_OPTION_SET_CAPTURE_AREA,

	//! video option: set start pixel
	VI_OPTION_SET_START_PIXEL,

	//! video option: set night mode
	VI_OPTION_SET_NIGHT_MODE,

	//! video option: set frame rate
	VI_OPTION_SET_FRAME_RATE,

	//! video option: set output type
	VI_OPTION_SET_OUTPUT_TYPE,

	//! video option: set zoom
	VI_OPTION_SET_ZOOM,

	//! video option: set privacy mask
	VI_OPTION_SET_PRIVACY_MASK,

	//! video option: set auto track white
	VI_OPTION_SET_AUTO_TRACK_WHITE,

	//! video option: set exposure time
	VI_OPTION_SET_EXPOSURE_TIME,

	//! video option: set field inverse
	VI_OPTION_SET_FIELD_INVERSE,

	//! video option: set frame rate control
	VI_OPTION_RESET_FRAME_RATE_CTRL,

	//! video option: set shutter control
	VI_OPTION_SET_SHUTTER_CTRL,

	//! video option: set global gain
	VI_OPTION_SET_GLOBALGAIN_CTRL,

	//! video option: set config
	VI_OPTION_SET_CONFIG,

	//! video option: set backlight compensation
	VI_OPTION_SET_BACKLIGHT_COMPENSATION,

	//! video option: set auto iris control
	VI_OPTION_SET_AUTO_IRIS_CTRL,

	//! video option: set exposure level
	VI_OPTION_SET_EXPOSURE_LEVEL,

	//! video option: set sharpness
	VI_OPTION_SET_SHARPNESS,

	//! video option: set half sized output
	VI_OPTION_SET_HALF_SIZED_OUTPUT,

	//! video option: set color correction
	VI_OPTION_SET_COLOR_CORRECTION,

	//! video option: set gamma table
	VI_OPTION_SET_GAMMA_TABLE,

	//! video option: set tone mapping
	VI_OPTION_SET_TONE_MAPPING,

	//! video option: set contrast enhancement
	VI_OPTION_SET_CONTRAST_ENHANCEMENT,

	//! video option: photo ldc calibrate
	VI_OPTION_PHOTO_LDC_CALIBRATE,

	//! video option: set photo ldc table
	VI_OPTION_SET_PHOTO_LDC_TABLE,

	//! video option: set auto color suppression
	VI_OPTION_SET_AUTO_COLOR_SUPPRESSION,

	//! video option: set photo ldc enable
	VI_OPTION_SET_PHOTO_LDC_EN,

	//! video option: set pbject mask
	VI_OPTION_SET_OBJECT_MASK,

	//! video option: auto detect std
	VI_OPTION_AUTO_DETECT_STD,

	//! video option: set wdr
	VI_OPTION_SET_WDR,

	//! video option: set auto exposure windows
	VI_OPTION_SET_AUTO_EXPOSURE_WINDOWS,

	//! video option: set auto exposure windows priority
	VI_OPTION_SET_AUTO_EXPOSURE_WINDOW_PRIORITY,

	//! video option: set auto max shutter
	VI_OPTION_SET_AUTO_EXPOSURE_MAX_SHUTTER,

	//! video option: set auto exposure max gain
	VI_OPTION_SET_AUTO_EXPOSURE_MAX_GAIN,

	//! video option: set auto exposure target luminance
	VI_OPTION_SET_AUTO_EXPOSURE_TARGET_LUMINANCE,

	//! video option: set auto exposure min shutter
	VI_OPTION_SET_AUTO_EXPOSURE_MIN_SHUTTER,

	//! video option: set auto exposure min gain
	VI_OPTION_SET_AUTO_EXPOSURE_MIN_GAIN,

	//! video option: set auto exposure mode
	VI_OPTION_SET_AUTO_EXPOSURE_MODE,

	//! video option: set auto iris enable
	VI_OPTION_SET_AUTO_IRIS_EN,

	//! video option: set auto focus window
	VI_OPTION_SET_AUTO_FOCUS_WINDOW,

	//! video option: set focus position
	VI_OPTION_SET_FOCUS_POSITION,

	//! video option: set focus speed
	VI_OPTION_SET_FOCUS_SPEED,

	//! video option: set zoom position
	VI_OPTION_SET_ZOOM_POSITION,

	//! video option: set zoom speed
	VI_OPTION_SET_ZOOM_SPEED,

	//! video option: set focus noise thres
	VI_OPTION_SET_FOCUS_NOISE_THRES,

	//! video option: set zoomtracking focus enable
	VI_OPTION_SET_ZOOMTRACKING_FOCUS_EN,

	//! video option: set auto focus table size
	VI_OPTION_GET_AUTO_FOCUS_TABLE_SIZE,

	//! video option: set auto focus calibrate
	VI_OPTION_SET_AUTO_FOCUS_CALIBRATE,

	//! video option: set auto focus table
	VI_OPTION_SET_AUTO_FOCUS_TABLE,

	//! video option: set anti aliasing
	VI_OPTION_SET_ANTI_ALIASING,

	//! video option: set exposure speed
	VI_OPTION_SET_AUTO_EXPOSURE_SPEED,

	//! video option: set auto iris active time
	VI_OPTION_SET_AUTO_IRIS_ACTIVE_TIME,

	//! video option: set auto scene
	VI_OPTION_SET_AUTO_SCENE,

	//! video option: set black clamp
	VI_OPTION_SET_BLACK_CLAMP,

	//! video option: set impluse noise removal
	VI_OPTION_SET_IMPULSE_NOISE_REMOVAL,

	//! video option: set auto white balance window priority
	VI_OPTION_SET_AUTO_WHITE_BALANCE_WINDOW_PRIORITY,

	//! video option: set cache coherence
	VI_OPTION_SET_CACHE_COHERENCE,

	//! video option: get color temerature
	VI_OPTION_GET_COLOR_TEMPERATURE,

	//! video option: set color transform
	VI_OPTION_SET_COLOR_TRANSFORM,

	//! video option: sensor direct access
	VI_OPTION_SENSOR_DIRECT_ACCESS,

	//! video option: set compress format
	VI_OPTION_SET_COMPRESS_FORMAT,

	//! video option: set fata width flag
	VI_OPTION_SET_DATA_WIDTH_FLAG,

	//! video option: set video frame format
	VI_OPTION_SET_VIDEO_FRAME_FORMAT,

	//! video option: set auto expoure control
	VI_OPTION_AUTO_EXPOSURE_CONTROL,

	//! video option: auto white balance control
	VI_OPTION_AUTO_WHITE_BALANCE_CONTROL,

	//! video option: fixed pattern noise correct control
	VI_OPTION_FIXED_PATTERN_NOISE_CORRECT_CONTROL,

	//! video option: color aberation correct control
	VI_OPTION_COLOR_ABERRATION_CORRECT_CONTROL,

	//! video option: defect pixel correct control
	VI_OPTION_DEFECT_PIXEL_CORRECT_CONTROL,

	//! video option: decompading control
	VI_OPTION_DECOMPANDING_CONTROL,

	//! video option: black clamp control
	VI_OPTION_BLACK_CLAMP_CONTROL,

	//! video option: lens shading correct control
	VI_OPTION_LENS_SHADING_CORRECT_CONTROL,

	//! video option: local tone mapping control
	VI_OPTION_LOCAL_TONE_MAPPING_CONTROL,

	//! video option: color correction control
	VI_OPTION_COLOR_CORRECTION_CONTROL,

	//! video option: color transform control
	VI_OPTION_COLOR_TRANSFORM_CONTROL,

	//! video option: gamma control
	VI_OPTION_GAMMA_CONTROL,

	//! video option: saturation brightness contrast control
	VI_OPTION_SATURATION_BRIGHTNESS_CONTRAST_CONTROL,

	//! video option: contrast enhance control
	VI_OPTION_CONTRAST_ENHANCE_CONTROL,

	//! video option: clip control
	VI_OPTION_CLIP_CONTROL,

	//! video option: mirror flip control
	VI_OPTION_MIRROR_FLIP_CONTROL,

	//! video option: noise reduct statistic control
	VI_OPTION_NOISE_REDUCT_STATISTIC_CONTROL,

	//! video option: auto focus control
	VI_OPTION_AUTO_FOCUS_CONTROL,

	//! video option: ir cut control
	VI_OPTION_IR_CUT_CONTROL,

	//! video option: local tone mapping curve control
	VI_OPTION_LOCAL_TONE_MAPPING_CURVE_CONTROL,


	//! video option: reserved vml restart (followings are the VML options need to stop & start vi.)
	VI_OPTION_RESERVED_VML_RESTART,

	//! video option: reserved vmtk (followings are the VMTK options)
	VI_OPTION_RESERVED_VMTK,

	//! video option: get video frame format (only get)
	VI_OPTION_GET_VIDEO_FRAME_FORMAT,

	//! video option: reserved vmtk restart
	VI_OPTION_RESERVED_VMTK_RESTART,

	//! video option: size (VMF_VI_SIZE_CONFIG_T)
	VI_OPTION_SIZE,

	//! video option: interface (VMF_VI_INTERFACE_INFO_T)(only get option)
	VI_OPTION_INTERFACE,
	//VI_OPTION_RESOLUTION,
	//VI_OPTION_XY_OFFSET            //! only get

	//! video option: reserved max
	VI_OPTION_RESERVED_MAX
} vmf_vi_option_flag_t;
typedef vmf_vi_option_flag_t VMF_VI_OPTION_FLAG_T;

/*
 * A data structure for video size option ( sensor -> (cap_in) -> vic -> (cap_out))
 */
typedef struct
{
	//! A data for max width (max width of the output buffer, also be the stride, CANNOT be changed by SetOption)
	unsigned int dwMaxWidth;

	//! A data for max height (max height of the output buffer, CANNOT be changed by SetOption)
	unsigned int dwMaxHeight;

	//! A data for capture width (input frame width)
	unsigned int dwCapInWidth;

	//! A data for capture height (input frame width, CANNOT be changed by SetOption)
	unsigned int dwCapInHeight;

	//! A data for capture out width (capture width, must be multiple of 16)
	unsigned int dwCapOutWidth;

	//! A data for capture out height (capture height)
	unsigned int dwCapOutHeight;

	//! A data for start x (apture x-offset on cap_in frame, must be even number)
	unsigned int dwStartX;

	//! A data for start y (capture y-offset on cap_in frame)
	unsigned int dwStartY;
} vmf_vi_size_config_t;
typedef vmf_vi_size_config_t VMF_VI_SIZE_CONFIG_T;

/*
 * A data structure for video option
 */
typedef struct
{
	//! A data for the index of target vi channel. VMF_VI_IDX_ALL means applying to all channels.
	unsigned int dwIndex;

	//! A data for option flag, which decides the type of data
	VMF_VI_OPTION_FLAG_T eOptionFlag;

	//! A pointer data for config data
	void* pData;
} vmf_vi_option_t;
typedef vmf_vi_option_t VMF_VI_OPTION_T;

/*
 * A data structure for ae set info
 */
typedef struct
{
	//! A data for shutter
	unsigned int dwShutter;

	//! A data for gain
	unsigned int dwGain;

	//! A data for HDRratio
	unsigned int dwHDRratio;
} VMF_VI_AE_SET_INFO_T;

/*
 * A data structure for video gamma control
 */
typedef struct
{
	//! A data for type
	unsigned int dwType;
} VMF_VI_GAMMA_CTRL_T;

/*
 * A data structure for video mirror flip control
 */
typedef struct
{
	//! A data for mirror enable
	unsigned int bMirrorEn;

	//! A data for flip enable
	unsigned int bFlipEn;
} VMF_VI_MIRROR_FLIP_CTRL_T;

/*
 * A data structure for VIC interface information
 */
typedef struct
{
	//! A data for VIC interface information
	unsigned int adwInterfaces[VMF_VI_MAX_CH_NUM];
} VMF_VI_INTERFACE_INFO_T;

#ifdef __cplusplus
}
#endif

#endif

