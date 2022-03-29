
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
#ifndef CONFIG_IFP_H
#define CONFIG_IFP_H

#include <comm/video_buf.h>

#define VMF_IFP_MAX_CH_NUM 2
#define VMF_IFP_FUSION_MAX_CHANNEL 4

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Ifp option
 */
typedef enum
{
	//! Ifp option: min
	IFP_OPTION_RESERVED_MIN,

	//! Ifp option: top control
	IFP_OPTION_TOP_CONTROL,

	//! Ifp option: output control
	IFP_OPTION_OUTPUT_CONTROL,

	//! Ifp option: clamp 0 control
	IFP_OPTION_BLACK_CLAMP_0_CONTROL,

	//! Ifp option: clamp 1 control
	IFP_OPTION_BLACK_CLAMP_1_CONTROL,

	//! Ifp option: clamp 2 control
	IFP_OPTION_BLACK_CLAMP_2_CONTROL,

	//! Ifp option: clamp 3 control
	IFP_OPTION_BLACK_CLAMP_3_CONTROL,

	//! Ifp option: rgb ir control
	IFP_OPTION_RGBIR_CONTROL,

	//! Ifp option: defect pixel correct 0 control
	IFP_OPTION_DEFECT_PIXEL_CORRECT_0_CONTROL,

	//! Ifp option: fixed pattern noise correct control
	IFP_OPTION_FIXED_PATTERN_NOISE_CORRECT_CONTROL,

	//! Ifp option: color aberration correct control
	IFP_OPTION_COLOR_ABERRATION_CORRECT_CONTROL,

	//! Ifp option: lens shading correct control
	IFP_OPTION_LENS_SHADING_CORRECT_CONTROL,

	//! Ifp option: lens shading correct control
	IFP_OPTION_DECOMPANDING_CONTROL,

	//! Ifp option: black clamp on dc control
	IFP_OPTION_BLACK_CLAMP_ON_DC_CONTROL,

	//! Ifp option: bayer gamma control
	IFP_OPTION_BAYER_GAMMA_CONTROL,

	//! Ifp option: digital gain 0 control
	IFP_OPTION_DIGITAL_GAIN_0_CONTROL,

	//! Ifp option: digital gain 1 control
	IFP_OPTION_DIGITAL_GAIN_1_CONTROL,

	//! Ifp option: digital gain 2 control
	IFP_OPTION_DIGITAL_GAIN_2_CONTROL,

	//! Ifp option: digital gain 3 control
	IFP_OPTION_DIGITAL_GAIN_3_CONTROL,

	//! Ifp option:  white balance gain 0 control
	IFP_OPTION_WB_GAIN_0_CONTROL,

	//! Ifp option:  white balance gain 1 control
	IFP_OPTION_WB_GAIN_1_CONTROL,

	//! Ifp option:  white balance gain 2 control
	IFP_OPTION_WB_GAIN_2_CONTROL,

	//! Ifp option:  white balance gain 3 control
	IFP_OPTION_WB_GAIN_3_CONTROL,

	//! Ifp option: fusion control
	IFP_OPTION_FUSION_CONTROL,

	//! Ifp option: white balance gain on fusion control
	IFP_OPTION_WB_GAIN_ON_FUNSION_CONTROL,

	//! Ifp option: local tone mapping control
	IFP_OPTION_LOCAL_TONE_MAPPING_CONTROL,

	//! Ifp option: local tone mapping gp control
	IFP_OPTION_LOCAL_TONE_MAPPING_GP_CONTROL,

	//! Ifp option: defect pixel correcl 1 control
	IFP_OPTION_DEFECT_PIXEL_CORRECT_1_CONTROL,

	//! Ifp option: noise reduction 2d control
	IFP_OPTION_NOISE_REDUCTION_2D_CONTROL,

	//! Ifp option: noise reduction 3d control
	IFP_OPTION_NOISE_REDUCTION_3D_CONTROL,

	//! Ifp option: white balance gain on bayer noise reduction control
	IFP_OPTION_WB_GAIN_ON_BNR_CONTROL,

	//! Ifp option: color filter array control
	IFP_OPTION_COLOR_FILTER_ARRAY_CONTROL,

	//! Ifp option: extgamma control
	IFP_OPTION_EXTGAMMA_CONTROL,

	//! Ifp option: color correction 0 control
	IFP_OPTION_COLOR_CORRECTION_0_CONTROL,

	//! Ifp option: color correction 1 control
	IFP_OPTION_COLOR_CORRECTION_1_CONTROL,

	//! Ifp option: purple fringe correction control
	IFP_OPTION_PURPLE_FRINGE_CORRECTION_CONTROL,

	//! Ifp option: color lut 3d control
	IFP_OPTION_COLOR_LUT_3D_CONTROL,

	//! Ifp option: color transform control
	IFP_OPTION_COLOR_TRANSFORM_CONTROL,

	//! Ifp option: saturation & bright & contrast y control
	IFP_OPTION_SBC_Y_CONTROL,

	//! Ifp option: saturation & bright & contrast cbcr control
	IFP_OPTION_SBC_CBCR_CONTROL,

	//! Ifp option: edge enhancement control
	IFP_OPTION_EDGE_ENHANCEMENT_CONTROL,

	//! Ifp option: privacy mask control
	IFP_OPTION_PRIVACY_MASK_CONTROL,

	//! Ifp option: motion detection control
	IFP_OPTION_MOTION_DETECTION_CONTROL,

	//! Ifp option: imap control
	IFP_OPTION_IMAP_CONTROL,

	//! Ifp option: statistic focus 0 control
	IFP_OPTION_STATISTIC_FOCUS_0_CONTROL,

	//! Ifp option: statistic focus 1 control
	IFP_OPTION_STATISTIC_FOCUS_1_CONTROL,

	//! Ifp option: statistic histogram 0 control
	IFP_OPTION_STATISTIC_HISTOGRAM_0_CONTROL,

	//! Ifp option: statistic histogram 1 control
	IFP_OPTION_STATISTIC_HISTOGRAM_1_CONTROL,

	//! Ifp option: statistic grid 0 control
	IFP_OPTION_STATISTIC_GRID_0_CONTROL,

	//! Ifp option: statistic grid 1 control
	IFP_OPTION_STATISTIC_GRID_1_CONTROL,

	//! Ifp option: yuv to bayer control
	IFP_OPTION_YUV_2_BAYER_CONTROL,

	//! Ifp option: complex info control
	IFP_OPTION_COMPLEXINFO_CONTROL,

	//! Ifp option: autoscene control
	IFP_OPTION_AUTOSCENE_CONTROL,

	//! Ifp option: hdr control
	IFP_OPTION_HDR_RATIO_CONTROL,

	//! Ifp option: statistical position control
	IFP_OPTION_STATISTIC_POSITION_CONTROL,

	//! Ifp option: reserved
	IFP_OPTION_RESERVED_VMTK,

	//! Ifp option: out buffer info
	IFP_OPTION_OUT_BUFF_INFO,

	//! Ifp option: subsmaple control
	IFP_OPTION_SUBSAMPLE_CONTROL,

	//! Ifp option: sub ir control
	IFP_OPTION_SUB_IR_CONTROL,

	//! Ifp option: dynamic I-frame control
	IFP_OPTION_DYNAMIC_I_CONTROL,

	//! Ifp option: resolution control
	IFP_OPTION_SET_RES,

	//! Ifp option: reserved max
	IFP_OPTION_RESERVED_MAX
} VMF_IFP_OPTION_FLAG_T;

/*
 * A data structure for ifp option
 */
typedef struct
{
	//! A data for ifp option index
	unsigned int dwIndex;

	//! A data for ifp option flag
	VMF_IFP_OPTION_FLAG_T eOptionFlag;

	//! A pointer data for ifp option config
	unsigned int* pdwData;
} VMF_IFP_OPTION_T;

/*
 * Video subsample mode
 */
typedef enum
{
	//! Video subsample mode: disable
	IFP_VIDEO_SUBSAMPLE_MODE_DISABLE = 0,

	//! Video subsample mode: 2 to 1
	IFP_VIDEO_SUBSAMPLE_MODE_2_TO_1 = 1,

	//! Video subsample mode: 4 to 1
	IFP_VIDEO_SUBSAMPLE_MODE_4_TO_1 = 2,

	//! Video subsample mode: 8 to 1
	IFP_VIDEO_SUBSAMPLE_MODE_8_TO_1 = 3,

	//! Video subsample mode: 16 to 1
	IFP_VIDEO_SUBSAMPLE_MODE_16_TO_1 = 4,

	//! Video subsample mode: num
	IFP_VIDEO_SUBSAMPLE_MODE_NUM = 5
} VMF_IFP_VIDEO_SUB_SAMPLE_MODE;

/*
 * IR subsample mode
 */
typedef enum
{
	//! IR subsample mode: 1 to 1
	IFP_IR_SUBSAMPLE_MODE_1_TO_1 = 0,

	//! IR subsample mode: 2 to 1
	IFP_IR_SUBSAMPLE_MODE_2_TO_1 = 1,

	//! IR subsample mode: 4 to 1
	IFP_IR_SUBSAMPLE_MODE_4_TO_1 = 2,

	//! IR subsample mode: 8 to 1
	IFP_IR_SUBSAMPLE_MODE_8_TO_1 = 3,

	//! IR subsample mode: num
	IFP_IR_SUBSAMPLE_MODE_NUM = 4
} VMF_IFP_IR_SUB_SAMPLE_MODE;

/*
 * GRID block size
 */
typedef enum
{
	//! GRID block size: 16X16
	IFP_GRID_BLOCK_SIZE_16X16 = 0,

	//! GRID block size: 32X32
	IFP_GRID_BLOCK_SIZE_32X32 = 1,

	//! GRID block size: num
	IFP_GRID_BLOCK_SIZE_NUM = 2
} VMF_IFP_GRID_BLOCK_SIZE;

/*
 * Lens shading correction (LSC) block size
 */
typedef enum
{
	//! Lens shading correction: 8
	IFP_LSC_BLOCK_SIZE_8 = 2,

	//! Lens shading correction: 16
	IFP_LSC_BLOCK_SIZE_16 = 3
} VMF_IFP_LSC_BLOCK_SIZE;

/*
 * Video Compression ratio
 */
typedef enum
{
	//! Video Compression ratio: none
	IFP_VIDEO_COMPRESSION_RATIO_NONE = 0,

	//! Video Compression ratio: 8 to 6
	IFP_VIDEO_COMPRESSION_RATIO_8_TO_6 = 1,

	//! Video Compression ratio: 12 to 8
	IFP_VIDEO_COMPRESSION_RATIO_12_TO_8 = 2,

	//! Video Compression ratio: 12 to 6
	IFP_VIDEO_COMPRESSION_RATIO_12_TO_6 = 3
} VMF_IFP_VIDEO_COMPRESSION_RATIO;

/*
 * A data structure for manual white balance gain (MWBG)
 */
typedef struct
{
	//! A data for MWBG enable : (1: enable, 0: disable)
	unsigned int bEnable;

	//! A data for MWBG's gain in gr channel : (range 1~8191,  1024 = 1x)
	unsigned int dwGainGr;

	//! A data for MWBG's gain in r channel : (range 1~8191,  1024 = 1x)
	unsigned int dwGainR;

	//! A data for MWBG's gain in b channel : (range 1~8191,  1024 = 1x)
	unsigned int dwGainB;

	//! A data for MWBG's gain in gb channel : (range 1~8191,  1024 = 1x)
	unsigned int dwGainGb;
} VMF_IFP_MANUAL_WB_GAIN_PARAM_T;

/*
 * A data structure for color correction matrix (CCM)
 */
typedef struct
{
	//! A data array for CCM: Array[9] = [RR] [GR] [BR] [RG] [GG] [BG] [RB] [GB] [BB] range:-511~511, 128 = 1x, [R_in_offset] [G_in_offset] [B_in_offset] [R_out_offset] [G_out_offset] [B_out_offset] range: -255~255
	int aiRGB2RGBMatrix[15];
} VMF_IFP_COLOR_CORRECT_MATRIX_PARAM_T;

/*
 * A data structure for rgb to yuv
 */
typedef struct
{
	//! A data array for rgb to yuv : (range : -511~511,  128 = 1x)
	int aiRgb2YuvMatrix[15];
} VMF_IFP_RGB2YUV_PARAM_T;

/*
 * A data structure for RGB IR parameter
 */
typedef struct
{
	//! A data for RGB IR enable : (1: enable, 0: disable)
	unsigned int bEnable;
} VMF_IFP_RGBIR_PARAM_T;

/*
 * A data structure for optical black (OB)
 */
typedef struct
{
	//! A data for Gr channel's black clamp : (range 0~4095)
	unsigned int dwCompGr;

	//! A data for R channel's black clamp : (range 0~4095)
	unsigned int dwCompR;

	//! A data for Gb channel's black clamp : (range 0~4095)
	unsigned int dwCompGb;

	//! A data for B channel's black clamp : (range 0~4095)
	unsigned int dwCompB;
} VMF_IFP_OPTICAL_BLACK_PARAM_T;

/*
 * A data structure for decompand optical black (DCOB)
 */
typedef struct
{
	//! A data for enable : (1: enable, 0: disable)
	unsigned int bEnable;

	//! A data for Gr channel's black clamp : (range 0~4095)
	unsigned int dwCompGr;

	//! A data for R channel's black clamp : (range 0~4095)
	unsigned int dwCompR;

	//! A data for Gb channel's black clamp : (range 0~4095)
	unsigned int dwCompGb;

	//! A data for B channel's black clamp : (range 0~4095)
	unsigned int dwCompB;
} VMF_IFP_DECOMPAND_OPTICAL_BLACK_PARAM_T;

/*
 * A data structure for defect pixel correction (DFC)
 */
typedef struct
{
	//! A data for DPC enable : (1: enable, 0: disable)
	unsigned int bEnable;

	//! A data for bit map mode enable : (1: enable, 0: disable)
	unsigned int bBitMapEn;

	//! A data for neighbor variance weight ThdH : (range 0~4095)
	unsigned int dwNbrVarWgtThdH;

	//! A data for neighbor variance weight ThdL : (range 0~4095)
	unsigned int dwNbrVarWgtThdL;

	//! A data for central variance weight ThdH : (range 0~4095)
	unsigned int dwCenVarWgtThdH;

	//! A data for central variance weight ThdL : (range 0~4095)
	unsigned int dwCenVarWgtThdL;
} VMF_IFP_DPC_PARAM_T;

/*
 * A data structure for green balance (GB)
 */
typedef struct
{
	//! A data for green balance enable : (1: enable, 0: disable)
	unsigned int bEnable;
} VMF_IFP_GREEN_BALANCE_PARAM_T;

/*
 * A data structure for fixed pattern noise correct (FPNC)
 */
typedef struct
{
	//! A data for fix pattern noise enable : (1: enable, 0: disable)
	unsigned int bEnable;
} VMF_IFP_FPN_CORRECT_PARAM_T;

/*
 * A data structure for lens shading correction (LSC)
 */
typedef struct
{
	//! A data for LSC enable : (0: disable, 1: enable)
	unsigned int bEnable;

	//! A data for red channel gain : (range 0~255)
	unsigned int dwRedGain;

	//! A data for red channel offset : (range -32767~32767)
	short nRedOffset;

	//! A data for green channel gain : (range 0~255)
	unsigned int dwGreenGain;

	//! A data for green channel offset : (range -32767~32767)
	short nGreenOffset;

	//! A data for blue channel gain : (range 0~255)
	unsigned int dwBlueGain;

	//! A data for blue channel offset : (range -32767~32767)
	short nBlueOffset;
} VMF_IFP_LSC_CORRECT_PARAM_T;

/*
 * A data structure for local tone mapping (LTM)
 */
typedef struct
{
	//! A data for LTM enable : (0: disable, 1: enable)
	unsigned int bEnable;

	//! A data for LTM dark tone curve index : (range 0~10)
	unsigned int dwDarkToneCurveIdx;

	//! A data for LTM gamma curve index : (range 0~10)
	unsigned int dwGammaCurveIdx;

	//! A data for dark area : (range 0~2047)
	unsigned int dwDarkAreaLimit;

	//! A data for LTM dark tone gain : (range 0~7)
	unsigned int dwDtoneGain;

	//! A data for LTM bright tone enable : (0: disable, 1: enable)
	unsigned int dwBtoneEnable;

	//! A data for LTM bright tone gain : (range 0~7)
	unsigned int dwBtoneGain;

	//! A data for LTM edge enhance enable : (0: disable, 1: enable)
	unsigned int bEeEnable;

	//! A data for LTM edge enhance coring : (range 0~4095)
	unsigned int dwEeCoring;

	//! A data for LTM edge enhance gain : (range 0~1023)
	unsigned int dwEeGain;

	//! A data for LTM edge enhance shift bits : (range 0~15)
	unsigned int dwEeShiftBits;

	//! A data array for CCM: Array[9] = [RR] [GR] [BR]  [RG] [GG] [BG]  [RB] [GB] [BB] range: -511~511 128 = 1x, [R_in_offset] [G_in_offset] [B_in_offset] [R_out_offset] [G_out_offset] [B_out_offset] range:-255~255
	int aiRGB2RGBMatrix[15];
} VMF_IFP_LTM_PARAM_T;

/*
 * A data structure for gamma (GMA)
 */
typedef struct
{
	//! A data for GMA curve index : (range 0~10)
	unsigned int dwCurveIdx;
} VMF_IFP_GAMMA_PARAM_T;

/*
 * A data structure for purple fringe correctiong (PFC)
 */
typedef struct
{
	//! A data for PFC enable : (0: disable, 1: enable)
	unsigned int bEnable;

	//! A data for PFC hue angle : (range 0~360)
	unsigned int dwHueAngle;

	//! A data for PFC hue roi : (range 0~360)
	unsigned int dwHueRoi;

	//! A data for PFC coring : (range 256)
	unsigned int dwCoring;

	//! A data for PFC gain : (range 0~256)
	unsigned int dwGain;
} VMF_IFP_PFC_PARAM_T;

/*
 * A data structure for color white (CW)
 */
typedef struct
{
	//! A data for CW enable : (0: disable, 1: enable)
	unsigned int bEnable;

	//! A data for CW coring : (range 63)
	unsigned int dwCoring;

	//! A data for CW gain : (range 0~63)
	unsigned int dwGain;

	//! A data for CW threshold max: (range 0~63)
	unsigned int dwThresholdMax;
} VMF_IFP_COLOR_WHITE_PARAM_T;

/*
 * A data structure for saturation & bright & contrast (SBC)
 */
typedef struct
{
	//! A data for brightness : (range -127~127, 0 = off)
	int iBright;

	//! A data for saturation : (range 0~255, 128 = off)
	unsigned int dwSaturation;

	//! A data for contrast : (range -127~127, 0 = off)
	int iContrast;
} VMF_IFP_SBC_PARAM_T;

/*
 * A data structure for contrast enhance (CE)
 */
typedef struct
{
	//! A data for CE enable : (0: disable, 1: enable)
	unsigned int bEnable;

	//! A data for CE used LUT table or curve index : (0: use curve index, 1: use LUT)
	unsigned int bUseLut;

	//! A data for CE curve index : (range 0~10)
	unsigned int dwCurveIdx;

	//! A data pointer for contrast enhancement table
	unsigned char *pbyLut;
} VMF_IFP_CONTRAST_ENHANCE_PARAM_T;

/*
 * A data structure for imap
 */
typedef struct
{
	//! A data for window size : DWORD (range 0~3)
	unsigned int dwWinSize;

	//! A data for down shift bits : DWORD (range 0~7)
	unsigned int dwWinShiftBit;

	//! A data for corring : DWORD (range 0~255)
	unsigned int dwCoring;

	//! A data for gain : DWORD (range 0~255)
	unsigned int dwGain;

	//! A data for threshold high : DWORD (range 0~255)
	unsigned int dwThdH;

	//! A data for threshold low : DWORD (range 0~255)
	unsigned int dwThdL;
} VMF_IFP_IMAP_PARAM_T;

/*
 * A data structure for bayer noise reduction 3d (BNR3D)
 */
typedef struct
{
	//! A data for BNR3D enable : (0: disable, 1: enable)
	unsigned int bEnable;

	//! A data for BNR3D current frame corring : (range 0~63)
	unsigned int dwCurrFrameCoring;

	//! A data for BNR3D current frame gain : (range 0~63)
	unsigned int dwCurrFrameGain;

	//! A data for BNR3D current frame threshold high : (range 0~255)
	unsigned int dwCurrFrameThdHigh;

	//! A data for BNR3D current frame threshold low : (range 0~255)
	unsigned int dwCurrFrameThdLow;

	//! A data for BNR3D reference frame corring : (range 0~63)
	unsigned int dwRefFrameCoring;

	//! A data for BNR3D reference frame gain : (range 0~63)
	unsigned int dwRefFrameGain;

	//! A data for BNR3D reference frame threshold high : (range 0~255)
	unsigned int dwRefFrameThdHigh;

	//! A data for BNR3D reference frame threshold low : (range 0~255)
	unsigned int dwRefFrameThdLow;
}VMF_IFP_BNR3D_PARAM_T;

/*
 * A data structure for bayer noise reduction 2D (BNR2D)
 */
typedef struct
{
	//! A data for BNR2D enable : (0: disable, 1: enable)
	unsigned int bEnable;

	//! A data for BNR2D global search range : (range 0~3)
	unsigned int dwGlobalSr;

	//! A data for BNR2D local search range : (range 0~2)
	unsigned int dwLocalSr;

	//! A data for BNR2D global strength : DWORD (range 0~15)
	unsigned int dwGlobalStrength;

	//! A data for BNR2D motion enable : (0: disable, 1: enable)
	unsigned int bMotionEnable;

	//! A data for BNR2D motion coring : (range 0~63)
	unsigned int dwMotionCoring;

	//! A data for BNR2D motion gain : (range 0~63)
	unsigned int dwMotionGain;

	//! A data for BNR2D motion threshold high : (range 0~255)
	unsigned int dwMotionThdHigh;

	//! A data for BNR2D motion threshold low : (range 0~255)
	unsigned int dwMotionThdLow;

	//! A data for BNR2D motion noise level threshold high : DWORD (range 0~255)
	unsigned int dwMotionNrLevelThdHigh;

	//! A data for BNR2D motion noise level threshold low : DWORD (range 0~255)
	unsigned int dwMotionNrLevelThdLow;

	//! A data for BNR2D LTM enable : (0: disable, 1: enable)
	unsigned int bLTMEnable;

	//! A data for BNR2D LTM coring : (range 0~63)
	unsigned int dwLTMCoring;

	//! A data for BNR2D LTM gain : (range 0~63)
	unsigned int dwLTMGain;

	//! A data for BNR2D LTM threshold high : (range 0~255)
	unsigned int dwLTMThdHigh;

	//! A data for BNR2D LTM threshold low : (range 0~255)
	unsigned int dwLTMThdLow;

	//! A data for BNR2D LTM noise level threshold high : DWORD (range 0~255)
	unsigned int dwLTMNrLevelThdHigh;

	//! A data for BNR2D LTM noise level threshold low : DWORD (range 0~255)
	unsigned int dwLTMNrLevelThdLow;

	//! A data for BNR2D LSC enable : (0: disable, 1: enable)
	unsigned int bLSCEnable;

	//! A data for BNR2D LSC coring : (range 0~63)
	unsigned int dwLSCCoring;

	//! A data for BNR2D LSC gain : (range 0~63)
	unsigned int dwLSCGain;

	//! A data for BNR2D LSC threshold high : (range 0~255)
	unsigned int dwLSCThdHigh;

	//! A data for BNR2D LSC threshold low : (range 0~255)
	unsigned int dwLSCThdLow;

	//! A data for BNR2D LSC noise level threshold high : DWORD (range 0~255)
	unsigned int dwLSCNrLevelThdHigh;

	//! A data for BNR2D LSC noise level threshold low : DWORD (range 0~255)
	unsigned int dwLSCNrLevelThdLow;

	//! A data for BNR2D Luma enable : (0: disable, 1: enable)
	unsigned int bLumaEnable;

	//! A data for BNR2D reference frame enable : (0: disable, 1: enable)
	unsigned int bRefFrameEnable;

	//! A data for BNR2D reference frame coring : (range 0~63)
	unsigned int dwRefFrameCoring;

	//! A data for BNR2D reference frame gain : (range 0~63)
	unsigned int dwRefFrameGain;

	//! A data for BNR2D reference frame threshold high : (range 0~255)
	unsigned int dwRefFrameThdHigh;

	//! A data for BNR2D reference frame threshold low : (range 0~255)
	unsigned int dwRefFrameThdLow;

	//! A data for BNR2D reference frame peanlty weight threshold high : (range 0~255)
	unsigned int dwRefFramePenaltyWgtThdHigh;

	//! A data for BNR2D reference frame peanlty weight threshold low : (range 0~255)
	unsigned int dwRefFramePenaltyWgtThdLow;
}VMF_IFP_BNR2D_PARAM_T;

/*
 * A data structure for edge enhance (EE)
 */
typedef struct
{
	//! A data for EE enable : (0: disable, 1: enable)
	unsigned int bEnable;

	//! A data for EE HPF shift bits : (range 0~15)
	unsigned int dwHPFShiftBits;

	//! A data for EE HPF coring : (range 0~2047)
	unsigned int dwHPFCoring;

	//! A data for EE HPF gain : (range 0~255)
	unsigned int dwHPFGain;

	//! A data for EE CPLX enable : (0: disable, 1: enable)
	unsigned int bCplxEnable;

	//! A data for EE CPLX filter shift bits : (range 0~15)
	unsigned int dwCplxShiftBits;

	//! A data for EE CPLX filter coring : (range 0~2047)
	unsigned int dwCplxCoring;

	//! A data for EE CPLX filter gain : (range 0~255)
	unsigned int dwCplxGain;

	//! A data for EE distance enable : (0: disable, 1: enable)
	unsigned int bDistEnable;

	//! A data for BNR2D distance threshold high : (range 0~255)
	unsigned int dwDistThdHigh;

	//! A data for BNR2D distance threshold low : (range 0~255)
	unsigned int dwDistThdLow;

	//! A data for BNR2D post gain : (range 0~255)
	unsigned int dwPostGain;

	//! A data for BNR2D luma enable : (0: disable, 1: enable)
	unsigned int bLumaEnable;

	//! A data for BNR2D overshoot enable : (0: disable, 1: enable)
	unsigned int bOverShootEnable;

	//! A data for BNR2D OverShoot threshold high : (range 0~255)
	unsigned int dwOverShootThdHigh;

	//! A data for BNR2D OverShoot threshold low : (range 0~255)
	unsigned int dwOverShootThdLow;
} VMF_IFP_EDGE_ENHANCE_PARAM_T;

/*
 * A data structure for output ctrl
 */
typedef struct
{
	//! A data for bayer noise reduction 3D enable
	unsigned int bBayerNoiseReduction3DEn;

	//! A data for bayer noise reduction 2D enable
	unsigned int bBayerNoiseReduction2DEn;

	//! A data for local tone mapping enable
	unsigned int bLocalToneMappingEn;

	//! A data for white balance gain on fusion enable
	unsigned int bWhiteBalanceGainOnFusionEn;

	//! A data for fusion enable
	unsigned int bFusionEn;

	//! A data for white balance gain on exposure frame 3 enable
	unsigned int bWhiteBalanceGainOnExpoFrame3En;

	//! A data for white balance gain on exposure frame 2 enable
	unsigned int bWhiteBalanceGainOnExpoFrame2En;

	//! A data for white balance gain on exposure frame 1 enable
	unsigned int bWhiteBalanceGainOnExpoFrame1En;

	//! A data for white balance gain on exposure frame 0 enable
	unsigned int bWhiteBalanceGainOnExpoFrame0En;

	//! A data for digital gain on exposure frame 3 enable
	unsigned int bDigitalGainOnExpoFrame3En;

	//! A data for digital gain on exposure frame 2 enable
	unsigned int bDigitalGainOnExpoFrame2En;

	//! A data for digital gain on exposure frame 1 enable
	unsigned int bDigitalGainOnExpoFrame1En;

	//! A data for digital gain on exposure frame 0 enable
	unsigned int bDigitalGainOnExpoFrame0En;

	//! A data for bayer gamma enable
	unsigned int bBayerGammaEn;

	//! A data for optical black on decompanding enale
	unsigned int bOpticalBlackOnDeCompandingEn;

	//! A data for decompanding enale
	unsigned int bDeCompandingEn;

	//! A data for lens shading correction enale
	unsigned int bLensShadingCorrectionEn;

	//! A data for chromatic aberration correction enale
	unsigned int bChromaticAberrationCorrectionEn;

	//! A data for fixed pattern noise reduction enale
	unsigned int bFixedPatternNoiseReductionEn;

	//! A data for green balance on exposure frame 1 enale
	unsigned int bGreenBalanceOnExpoFrame1En;

	//! A data for green balance on exposure frame 0 enale
	unsigned int bGreenBalanceOnExpoFrame0En;

	//! A data for defect pixel correction exposure frame 1 enale
	unsigned int bDefectPixelCorrectionOnExpoFrame1En;

	//! A data for defect pixel correction exposure frame 0 enale
	unsigned int bDefectPixelCorrectionOnExpoFrame0En;

	//! A data for rgb ir enale
	unsigned int bRGBIrEn;

	//! A data for optical black on exposure frame 3 enable
	unsigned int bOpticalBlackOnExpoFrame3En;

	//! A data for optical black on exposure frame 2 enable
	unsigned int bOpticalBlackOnExpoFrame2En;

	//! A data for optical black on exposure frame 1 enable
	unsigned int bOpticalBlackOnExpoFrame1En;

	//! A data for optical black on exposure frame 0 enable
	unsigned int bOpticalBlackOnExpoFrame0En;

	//! A data for post luma histogram enable
	unsigned int bPostLumaHistogramEn;

	//! A data for clear white enable
	unsigned int bClearWhiteEn;

	//! A data for motion detection enable
	unsigned int bMotionDetectionEn;

	//! A data for complex info enable
	unsigned int bComplexInfoEn;

	//! A data for statistics on exposure frame 1 enable
	unsigned int bStatisticsOnExpoFrame1En;

	//! A data for statistics on exposure frame 0 enable
	unsigned int bStatisticsOnExpoFrame0En;

	//! A data for post saturation histogram enable
	unsigned int bPostSaturationHistogramEn;

	//! A data for histogram on exposure frame 1 enable
	unsigned int bHistogramOnExpoFrame1En;

	//! A data for histogram on exposure frame 0 enable
	unsigned int bHistogramOnExpoFrame0En;

	//! A data for focus value on exposure frame 1 enable
	unsigned int bFocusValueOnExpoFrame1En;

	//! A data for focus value on exposure frame 0 enable
	unsigned int bFocusValueOnExpoFrame0En;

	//! A data for privacy mask enable
	unsigned int bPrivacyMaskEn;

	//! A data for edge enhancement enable
	unsigned int bEdgeEnhancementEn;

	//! A data for contrast enhance Curve enable
	unsigned int bContrastEnhanceCurveEn;

	//! A data for lut 3d enable
	unsigned int bLUT3DEn;

	//! A data for purple fringe correction enable
	unsigned int bPurpleFringeCorrectionEn;

	//! A data for white balance gain on bayer noise reduction enable
	unsigned int bWhiteBalanceGainOnBayerNREn;
} VMF_IFP_IFPE_OPTIONS_T;

/*
 * A data structure for output control
 */
typedef struct
{
	//! A data for motion detection mirror enable
	unsigned int bMDMirrorEn;

	//! A data for motion detection flip enable
	unsigned int bMDFlipEn;

	//! A data for complex info mirror enable
	unsigned int bCplxInfoMirrorEn;

	//! A data for complex info flip enable
	unsigned int bCplxInfoFlipEn;

	//! A data for MR mirror enable
	unsigned int bMRMirrorEn;

	//! A data for MR flip enable
	unsigned int bMRFlipEn;

	//! A data for ir sub mirror enable
	unsigned int bIrSubMirrorEn;

	//! A data for ir sub flip enable
	unsigned int bIrSubFlipEn;

	//! A data for sub mirror enable
	unsigned int bSubMirrorEn;

	//! A data for sub flip enable
	unsigned int bSubFlipEn;

	//! A data for mirror enable
	unsigned int bMirrorEn;

	//! A data for flip enable
	unsigned int bFlipEn;
} VMF_IFP_OUTPUT_CTRL_OPTION_T;

/*
 * A data structure for optical black
 */
typedef struct
{
	//! A data for clamp gr
	unsigned int dwClampGr;

	//! A data for clamp r
	unsigned int dwClampR;

	//! A data for clamp b
	unsigned int dwClampB;

	//! A data for clamp gb
	unsigned int dwClampGb;
} VMF_IFP_OPTICAL_BLACK_OPTION_T;

/*
 * A data structure for rgbir option
 */
typedef struct
{
	//! A data for EIR enable
	unsigned int bEIREn;

	//! A data for peaking enable
	unsigned int bPeakingEn;

	//! A data array for peaking LPF coefficient data
	unsigned int adwPeakingLPFCoeff[9];

	//! A data for EIR max ratio in r channel
	unsigned int dwEIRMaxRatioR;

	//! A data for EIR max ratio in g channel
	unsigned int dwEIRMaxRatioG;

	//! A data for EIR max ratio in b channel
	unsigned int dwEIRMaxRatioB;

	//! A data for peaking gain
	unsigned int dwPeakingGain;

	//! A data for peaking bound
	unsigned int dwPeakingBound;

	//! A data for enable channel gain
	unsigned int bChannelGainEn;

	//! A data for channel gain strength
	unsigned int dwChannelGainStrength;

	//! A data for EIR out gain in r channel
	unsigned int dwEIROutGainR;

	//! A data for EIR out gain in g channel
	unsigned int dwEIROutGainG;

	//! A data for EIR out gain in b channel
	unsigned int dwEIROutGainB;
} VMF_IFP_RGBIR_OPTION_T;

/*
 * A data structure for defect pixel correction
 */
typedef struct
{
	//! A data for bit map enable
	unsigned int bBitMapEn;

	//! A data for neighbor variance weight enable
	unsigned int bNbrVarWgtEn;

	//! A data for central variance weight enable
	unsigned int bCenVarWgtEn;

	//! A data for local max corr enable
	unsigned int bLocalMaxCorrEn;

	//! A data for local min corr enable
	unsigned int bLocalMinCorrEn;

	//! A data for neighbor variance weight thdH
	unsigned int dwNbrVarWgtThdH;

	//! A data for neighbor variance weight thdL
	unsigned int dwNbrVarWgtThdL;

	//! A data for central variance weight thdH
	unsigned int dwCenVarWgtThdH;

	//! A data for central variance weight thdL
	unsigned int dwCenVarWgtThdL;

	//! A data for gb mode
	unsigned int dwGBMode;

	//! A data array for gb luma level
	unsigned int adwGBLumaLevel[6];

	//! A data array for gb luma gain
	unsigned int adwGBLumaGain[6];

	//! A data array for gb flat level
	unsigned int adwGBFlatLevel[6];

	//! A data array for gb flat gain
	unsigned int adwGBFlatGain[6];

	//! A data for map update
	unsigned int bMapUpdate;

	//! A pointer data for map buffer
	unsigned char *pbyMapBuff;

	//! A data for map buffer physical address
	unsigned int dwMapBufPhyAddr;
} VMF_IFP_DPC_OPTION_T;

/*
 * A data structure for fixed pattern noise
 */
typedef struct
{
	//! A data for mode
	unsigned int bMode;

	//! A data for map 0 gain
	unsigned int dwMap0Gain;

	//! A data for map 1 gain
	unsigned int dwMap1Gain;

	//! A data for map 0 coring
	int iMap0Coring;

	//! A data for map 1 coring
	int iMap1Coring;

	//! A data for map 0 post coring
	int iMap0PostCoring;

	//! A data for map 1 post coring
	int iMap1PostCoring;

	//! A data for two map bleng weight
	unsigned int dwTwoMapBlendWgt;

	//! A data for map 0 update
	unsigned int bMap0Update;

	//! A pointer data for map 0 buffer
	unsigned char* pbyMap0Buff;

	//! A data for map 0 buffer's physical address
	unsigned int dwMap0BufPhyAddr;

	//! A pointer data for map 0 average buffer
	unsigned char* pbyMap0AvgBuff;

	//! A data for map 0 average buffer's physical address
	unsigned int dwMap0AvgBufPhyAddr;

	//! A data for map 1 update
	unsigned int bMap1Update;

	//! A pointer data for map 1 buffer
	unsigned char* pbyMap1Buff;

	//! A data for map 1 buffer's physical address
	unsigned int dwMap1BufPhyAddr;

	//! A pointer data for map 1 average buffer
	unsigned char* pbyMap1AvgBuff;

	//! A data for map 1 average buffer's physical address
	unsigned int dwMap1AvgBufPhyAddr;
} VMF_IFP_FPN_OPTION_T;

/*
 * A data structure for chromatic_aberration_correction
 */
typedef struct
{
	//! A data for b scale mode
	unsigned int bBScaleMode;

	//! A data for b scale enable
	unsigned int bBScaleEn;

	//! A data for r scale mode
	unsigned int bRScaleMode;

	//! A data for r scale enable
	unsigned int bRScaleEn;

	//! A data for y ratio offset
	int iYRatioOffset;

	//! A data for x ratio offset
	int iXRatioOffset;
} VMF_IFP_CAC_OPTION_T;

/*
 * A data structure for lens shading correction
 */
typedef struct
{
	//! A data for lsc 3d enable
	unsigned int bLSC3DEn;

	//! A data for decompanding enable
	unsigned int bDeCompandEn;

	//! A data for companding enable
	unsigned int bCompandEn;

	//! A data for linear mode
	unsigned int bLinearMode;

	//! A data for lsc 3d start point
	unsigned int dwLSC3DStartPoint;

	//! A data for lsc 3d end gain
	unsigned int dwLSC3DEndGain;

	//! A data for horizontal block size
	unsigned int dwHorzBlockSize;

	//! A data for vertical block size
	unsigned int dwVertBlockSize;

	//! A data for lsc table gain in r channel
	unsigned int dwLSCTableGainR;

	//! A data for lsc table offset in r channel
	int iLSCTableOffsetR;

	//! A data for lsc table gain in g channel
	unsigned int dwLSCTableGainG;

	//! A data for lsc table offset in g channel
	int iLSCTableOffsetG;

	//! A data for lsc table gain in b channel
	unsigned int dwLSCTableGainB;

	//! A data for lsc table offset in b channel
	int iLSCTableOffsetB;

	//! A data for lsc 3d end point
	unsigned int dwLSC3DEndPoint;

	//! A data for map update
	unsigned int bMapUpdate;

	//! A pointer data for map buffer
	unsigned char* pbyMapBuff;

	//! A pointer data for map buffer's physical address
	unsigned int dwMapBufPhyAddr;
} VMF_IFP_LSC_OPTION_T;

/*
 * A data structure for de companding
 */
typedef struct
{
	//! A data for output bit width
	unsigned int dwOutputBitWidth;

	//! A data array for curve y
	unsigned int adwCurveY[9];

	//! A data array for curve x
	unsigned int adwCurveX[9];
} VMF_IFP_DE_COMPANDING_OPTION_T;

/*
 * A data structure for bayer gamma
 */
typedef struct
{
	//! A data for bayer gammar option index
	unsigned int dwTblIndx;
} VMF_IFP_BAYER_GAMMA_OPTION_T;

/*
 * A data structure for digital gain
 */
typedef struct
{
	//! A data for gain in gr channel
	unsigned int dwGainGr;

	//! A data for gain in r channel
	unsigned int dwGainR;

	//! A data for gain in b channel
	unsigned int dwGainB;

	//! A data for gain in gb channel
	unsigned int dwGainGb;
} VMF_IFP_DIGITAL_GAIN_OPTION_T;

/*
 * A data structure for white balance gain
 */
typedef struct
{
	//! A data for gain in gr channel
	unsigned int dwGainGr;

	//! A data for gain in r channel
	unsigned int dwGainR;

	//! A data for gain in b channel
	unsigned int dwGainB;

	//! A data for gain in gb channel
	unsigned int dwGainGb;
} VMF_IFP_WHITE_BALANCE_GAIN_OPTION_T;

/*
 * A data structure for fusion
 */
typedef struct
{
	//! A data for exposure ratio ep0
	unsigned int dwExpoRatioEP0;

	//! A data for exposure ratio ep1
	unsigned int dwExpoRatioEP1;

	//! A data for exposure ratio ep2
	unsigned int dwExpoRatioEP2;

	//! A data for exposure ratio ep3
	unsigned int dwExpoRatioEP3;

	//! A data for MCFS curve enable
	unsigned int bMCFSCurveEn;

	//! A data for MCFS enable
	unsigned int bMCFSEn;

	//! A data for MR coring
	unsigned int dwMRCoring;

	//! A data for MR gain
	unsigned int dwMRGain;

	//! A data for MR ThdH
	unsigned int dwMRThdH;

	//! A data for MR ThdL
	unsigned int dwMRThdL;

	//! A data array for MCFR curve in
	unsigned int adwMCFRCurveIn[9];

	//! A data array for MCFR curve out
	unsigned int adwMCFRCurveOut[9];

	//! A data for Fsw0 tbl index
	unsigned int dwFsw0TblIndx;

	//! A data for Fsw1 tbl index
	unsigned int dwFsw1TblIndx;

	//! A data for Fsw2 tbl index
	unsigned int dwFsw2TblIndx;
} VMF_IFP_FUSION_OPTION_T;

/*
 * A data structure for local tone mapping
 */
typedef struct
{
	//! A data for bright tone ebale
	unsigned int bBToneEn;

	//! A data for tone ratio ebale
	unsigned int bToneRatioEn;

	//! A data for EE ebale
	unsigned int bEEEn;

	//! A data for dark tone gain
	unsigned int dwDToneGain;

	//! A data for bright tone gain
	unsigned int dwBToneGain;

	//! A data for ltm gain' limitH
	unsigned int dwLTMGainLimitH;

	//! A data for ltm gain' limitL
	unsigned int dwLTMGainLimitL;

	//! A data for EE coring
	unsigned int dwEECoring;

	//! A data for EE gain
	unsigned int dwEEGain;

	//! A data for EE shift bit
	unsigned int dwEEShiftBit;

	//! A data array for EE coefficient 2
	int aiEECoeff2[3];

	//! A data array for EE coefficient 1
	int aiEECoeff1[3];

	//! A data array for EE coefficient 0
	int aiEECoeff0[3];

	//! A data for color correction coefficient RR
	unsigned int dwCcCoeffRR;

	//! A data for color correction coefficient GR
	int iCcCoeffGR;

	//! A data for color correction coefficient BR
	int iCcCoeffBR;

	//! A data for color correction coefficient RG
	int iCcCoeffRG;

	//! A data for color correction coefficient GG
	unsigned int dwCcCoeffGG;

	//! A data for color correction coefficient BG
	int iCcCoeffBG;

	//! A data for color correction coefficient RB
	int iCcCoeffRB;

	//! A data for color correction coefficient GB
	int iCcCoeffGB;

	//! A data for color correction coefficient BB
	unsigned int dwCcCoeffBB;

	//! A data for color correction inr offset
	int iCcInROffset;

	//! A data for color correction ing offset
	int iCcInGOffset;

	//! A data for color correction inb offset
	int iCcInBOffset;

	//! A data for color correction outr offset
	int iCcOutROffset;

	//! A data for color correction outg offset
	int iCcOutGOffset;

	//! A data for color correction outb offset
	int iCcOutBOffset;

	//! A data for force update all table
	unsigned int bForceUpdateAllTbl;

	//! A data for gma index
	unsigned int dwGmaIndx;

	//! A data for dark tone index
	unsigned int dwDToneIndx;

	//! A data for bright tone index
	unsigned int dwBToneIndx;

	//! A data for log index
	unsigned int dwLogIndx;
} VMF_IFP_LTM_OPTION_T;

/*
 * A data structure for local tone mapping grid processor
 */
typedef struct
{
	//! A data for dark area limit
	unsigned int dwDarkAreaLimit;
} VMF_IFP_LTM_GRID_PROC_OPTION_T;

/*
 * A data structure for bayer noise reduction 2D
 */
typedef struct
{
	//! A data for noise reduction global search range
	unsigned int dwNRGlobalSR;

	//! A data for noise reduction local search range
	unsigned int dwNRLocalSR;

	//! A data for luma adapt enable
	unsigned int bLumaAdptEn;

	//! A data for shade adapt enable
	unsigned int bShadeAdptEn;

	//! A data for tone adapt enable
	unsigned int bToneAdptEn;

	//! A data for motion adapt enable
	unsigned int bMotionAdptEn;

	//! A data for 2d purple fringe enable
	unsigned int b2DPFEn;

	//! A data for 2d purple fringe 0 enable
	unsigned int b2DPF0En;

	//! A data for 2d purple fringe 1 enable
	unsigned int b2DPF1En;

	//! A data for 2d RF enable
	unsigned int b2DRFEn;

	//! A data for noise reduction global strength
	unsigned int dwNRGlobalStrength;

	//! A data for Mamr coring
	unsigned int dwMaMrCoring;

	//! A data for Mamr gain
	unsigned int dwMaMrGain;

	//! A data for Mamr thdH
	unsigned int dwMaMrThdH;

	//! A data for Mamr thdL
	unsigned int dwMaMrThdL;

	//! A data for Mamr level thdH0
	unsigned int dwMaNrLevelThdH0;

	//! A data for Mamr level thdH1
	unsigned int dwMaNrLevelThdH1;

	//! A data for Mamr level thdH2
	unsigned int dwMaNrLevelThdH2;

	//! A data for Mamr level thdH3
	unsigned int dwMaNrLevelThdH3;

	//! A data for Mamr level thdL0
	unsigned int dwMaNrLevelThdL0;

	//! A data for Mamr level thdL1
	unsigned int dwMaNrLevelThdL1;

	//! A data for Mamr level thdL2
	unsigned int dwMaNrLevelThdL2;

	//! A data for Mamr level thdL3
	unsigned int dwMaNrLevelThdL3;

	//! A data for tatr coring
	unsigned int dwTaTrCoring;

	//! A data for tatr gain
	unsigned int dwTaTrGain;

	//! A data for tatr thdH
	unsigned int dwTaTrThdH;

	//! A data for tatr thdL
	unsigned int dwTaTrThdL;

	//! A data for 2d rfmr coring
	unsigned int dw2DRFMrCoring;

	//! A data for 2d rfmr gain
	unsigned int dw2DRFMrGain;

	//! A data for 2d rfmr thdH
	unsigned int dw2DRFMrThdH;

	//! A data for 2d rfmr thdL
	unsigned int dw2DRFMrThdL;

	//! A data for 2d rf penalty weight h
	unsigned int dw2DRFPenaltyWgtH;

	//! A data for 2d rf penalty weight l
	unsigned int dw2DRFPenaltyWgtL;

	//! A data for 2d pf penalty weight
	unsigned int dw2DPFPenaltyWgt;

	//! A data for tanr level thdH0
	unsigned int dwTaNrLevelThdH0;

	//! A data for tanr level thdH1
	unsigned int dwTaNrLevelThdH1;

	//! A data for tanr level thdH2
	unsigned int dwTaNrLevelThdH2;

	//! A data for tanr level thdH3
	unsigned int dwTaNrLevelThdH3;

	//! A data for tanr level thdL0
	unsigned int dwTaNrLevelThdL0;

	//! A data for tanr level thdL1
	unsigned int dwTaNrLevelThdL1;

	//! A data for tanr level thdL2
	unsigned int dwTaNrLevelThdL2;

	//! A data for tanr level thdL3
	unsigned int dwTaNrLevelThdL3;

	//! A data for sasr mode
	unsigned int dwSaSrMode;

	//! A data for sasr coring
	unsigned int dwSaSrCoring;

	//! A data for sasr gain
	unsigned int dwSaSrGain;

	//! A data for sasr thdH
	unsigned int dwSaSrThdH;

	//! A data for sasr thdL
	unsigned int dwSaSrThdL;

	//! A data for sanr level thdH0
	unsigned int dwSaNrLevelThdH0;

	//! A data for sanr level thdH1
	unsigned int dwSaNrLevelThdH1;

	//! A data for sanr level thdH2
	unsigned int dwSaNrLevelThdH2;

	//! A data for sanr level thdH3
	unsigned int dwSaNrLevelThdH3;

	//! A data for sanr level thdL0
	unsigned int dwSaNrLevelThdL0;

	//! A data for sanr level thdL1
	unsigned int dwSaNrLevelThdL1;

	//! A data for sanr level thdL2
	unsigned int dwSaNrLevelThdL2;

	//! A data for sanr level thdH3
	unsigned int dwSaNrLevelThdL3;

	//! A data array for la curvex
	unsigned int adwLACurveX[9];

	//! A data array for la curvey
	unsigned int adwLACurveY[9];
} VMF_IFP_BAYER_NR2D_OPTION_T;

/*
 * A data structure for bayer noise reduction 3D
 */
typedef struct
{
	//! A data for current mr coring
	unsigned int dwCurMRCoring;

	//! A data for current mr gain
	unsigned int dwCurMRGain;

	//! A data for current mr thdH
	unsigned int dwCurMRThdH;

	//! A data for current mr thdL
	unsigned int dwCurMRThdL;

	//! A data for reference mr coring
	unsigned int dwRefMRCoring;

	//! A data for reference mr gain
	unsigned int dwRefMRGain;

	//! A data for reference mr thdH
	unsigned int dwRefMRThdH;

	//! A data for reference mr thdL
	unsigned int dwRefMRThdL;
}  VMF_IFP_BAYER_NR3D_OPTION_T;

/*
 * A data structure for color filter array
 */
typedef struct
{
	//! A data for fcs enable
	unsigned int bFCSEn;

	//! A data for fci enable
	unsigned int bFCIEn;

	//! A data for hvb enable
	unsigned int bHVBEn;

	//! A data for favor cfab land enable
	unsigned int bFavorCFABlendEn;

	//! A data for favor cfab land weight
	unsigned int dwFavorCFABlendWeight;
} VMF_IFP_COLOR_FILTER_OPTION_T;

/*
 * A data structure for color correction
 */
typedef struct
{
	//! A data for coefficient RR
	int iCoeffRR;

	//! A data for coefficient GR
	int iCoeffGR;

	//! A data for coefficient BR
	int iCoeffBR;

	//! A data for coefficient RG
	int iCoeffRG;

	//! A data for coefficient GG
	int iCoeffGG;

	//! A data for coefficient BG
	int iCoeffBG;

	//! A data for coefficient RB
	int iCoeffRB;

	//! A data for coefficient GB
	int iCoeffGB;

	//! A data for coefficient BB
	int iCoeffBB;

	//! A data for offset in r channel
	int iInROffset;

	//! A data for offset in g channel
	int iInGOffset;

	//! A data for offset in g channel
	int iInBOffset;

	//! A data for outr offset
	int iOutROffset;

	//! A data for outg offset
	int iOutGOffset;

	//! A data for outb offset
	int iOutBOffset;
} VMF_IFP_COLOR_CORRECTION_OPTION_T;

/*
 * A data structure for purple fringe correction
 */
typedef struct
{
	//! A data for hue angle
	unsigned int dwHueAngle;

	//! A data for hue roi
	unsigned int dwHueROI;

	//! A data for purple fringe correction weight coring
	unsigned int dwPFCWeightCoring;

	//! A data for purple fringe correction weight gain
	unsigned int dwPFCWeightGain;
} VMF_IFP_PFC_OPTION_T;

/*
 * A data structure for lut 3D
 */
typedef struct
{
	//! A data for lut table index
	unsigned int dwLutTblIndx;
} VMF_IFP_LUT_3D_OPTION_T;

/*
 * A data structure for rgb to yuv
 */
typedef struct
{
	//! A data for g to y coefficient 2
	int iR2YCoeff2;

	//! A data for g to y coefficient 1
	int iR2YCoeff1;

	//! A data for g to y coefficient 0
	int iR2YCoeff0;

	//! A data for g to y coefficient 5
	int iR2YCoeff5;

	//! A data for g to y coefficient 4
	int iR2YCoeff4;

	//! A data for g to y coefficient 3
	int iR2YCoeff3;

	//! A data for g to y coefficient 8
	int iR2YCoeff8;

	//! A data for g to y coefficient 7
	int iR2YCoeff7;

	//! A data for g to y coefficient 6
	int iR2YCoeff6;

	//! A data for cwe cb center
	unsigned int dwCWECbCenter;

	//! A data for cwe cr center
	unsigned int dwCWECrCenter;

	//! A data for cwe cb roi
	unsigned int dwCWECbROI;

	//! A data for cwe cr roi
	unsigned int dwCWECrROI;

	//! A data for cwe weight coring
	unsigned int dwCWEWeightCoring;

	//! A data for cwe weight gain
	unsigned int dwCWEWeightGain;

	//! A data for cwe weight max thd
	unsigned int dwCWEWeightMaxThd;

	//! A data for r to y offset in r channel
	int iR2YInOffsetR;

	//! A data for r to y offset in g channel
	int iR2YInOffsetG;

	//! A data for r to y offset in b channel
	int iR2YInOffsetB;

	//! A data for r to y out_offset in y channel
	int iR2YOutOffsetY;

	//! A data for r to y out_offset in u channel
	int iR2YOutOffsetU;

	//! A data for r to y out_offset in v channel
	int iR2YOutOffsetV;
} VMF_IFP_RGb2YUV_OPTION_T;

/*
 * A data structure for s_b_c_y
 */
typedef struct
{
	//! A data for Y clip thdH
	unsigned int dwYClipThdH;

	//! A data for Y clip thdL
	unsigned int dwYClipThdL;

	//! A data for brightness
	int iBrightness;

	//! A data for contrast
	int iContrast;

	//! A data for luma curve enable
	unsigned int bLumaCurveEn;

	//! A data array for luma curve x
	unsigned int adwLumaCurveX[17];

	//! A data array for luma curve y
	unsigned int adwLumaCurveY[17];

	//! A data for ce tbl index
	unsigned int dwCETblIndx;
} VMF_IFP_SBCY_OPTION_T;

/*
 * A data structure for s_b_c_c_b_c_r
 */
typedef struct
{
	//! A data for cbcr 422 mode
	unsigned int bCbCr422Mode;

	//! A data for cbcr clip thdH
	unsigned int dwCbCrClipThdH;

	//! A data for cbcr clip thdL
	unsigned int dwCbCrClipThdL;

	//! A data for saturation
	unsigned int dwSaturation;
} VMF_IFP_SBCCBCR_OPTION_T;

/*
 * A data structure for edge enhancement
 */
typedef struct
{
	//! A data for enable
	unsigned int bEnable;

	//! A data for luma adapt enable
	unsigned int bLumaAdptEn;

	//! A data for overshoopt min
	unsigned int dwOvershootMin;

	//! A data for overshoopt max
	unsigned int dwOvershootMax;

	//! A data for overshoopt clamp enable
	unsigned int bOvershootClampEn;

	//! A data for distance adapt enable
	unsigned int bDistAdptEn;

	//! A data for complex adapt enable
	unsigned int bCplxAdptEn;

	//! A data for edge enhancement post gain
	unsigned int dwEEPostGain;

	//! A data for hpf gain
	unsigned int dwHPFGain;

	//! A data for hpf coring
	unsigned int dwHPFCoring;

	//! A data for hpf shift bit
	unsigned int dwHPFShiftBit;

	//! A data for complex coring
	unsigned int dwCplxCoring;

	//! A data for complex shift bit
	unsigned int dwCplxShiftBit;

	//! A data for ca thdH
	unsigned int dwCAThdH;

	//! A data for ca thdL
	unsigned int dwCAThdL;

	//! A data for complex gain
	unsigned int dwCplxGain;

	//! A data for da vertical thdL
	unsigned int dwDAVertThdL;

	//! A data for da horizontal thdL
	unsigned int dwDAHorzThdL;

	//! A data array for complex coefficient
	int aiCplxCoeff[9];

	//! A data array for hpf coefficient
	int aiHPFCoeff[9];

	//! A data array for la curve x
	unsigned int adwLACurveX[9];

	//! A data array for la curve y
	unsigned int adwLACurveY[9];

	//! A data for da vertical thdL
	unsigned int dwDAVertThdH;

	//! A data for da horizontal thdL
	unsigned int dwDAHorzThdH;

	//! A data for nlg table index
	unsigned int dwNLGTblIndx;
} VMF_IFP_EE_OPTION_T;

/*
 * A data structure for privacy mask
 */
typedef struct
{
	//! A data for enable
	unsigned int bEnable;

	//! A data array for color
	unsigned int adwColor[3];

	//! A pointer data for y map buffer
	unsigned char* pbyYMapBuff;

	//! A pointer data for cb cr map buffer
	unsigned char* pbyCbCrMapBuff;
} VMF_IFP_PM_OPTION_T;

/*
 * A data structure for motion detection
 */
typedef struct
{
	//! A data for window size
	unsigned int dwWinSize;

	//! A data for window shift bit
	unsigned int dwWinShiftBit;

	//! A data for coring
	unsigned int dwCoring;

	//! A data for gain
	unsigned int dwGain;

	//! A data for thdH
	unsigned int dwThdH;

	//! A data for thdL
	unsigned int dwThdL;
} VMF_IFP_MD_OPTION_T;

/*
 * A data structure for i_m_a_p
 */
typedef struct
{
	//! A data for window size
	unsigned int dwWinSize;

	//! A data for window shift bit
	unsigned int dwWinShiftBit;

	//! A data for coring
	unsigned int dwCoring;

	//! A data for gain
	unsigned int dwGain;

	//! A data for thdH
	unsigned int dwThdH;

	//! A data for thdL
	unsigned int dwThdL;
} VMF_IFP_IMAP_OPTION_T;

/*
 * A data structure for focus value
 */
typedef struct
{
	//! A data for Gcomp mode
	unsigned int dwGCompMode;

	//! A data for grid weight enable
	unsigned int bGridWgtEn;

	//! A data for grid info sel 0
	unsigned int dwGridInfoSel0;

	//! A data for grid info sel 1
	unsigned int dwGridInfoSel1;

	//! A data for shift bit of grid fv
	unsigned int dwShiftBitOfGridFV;

	//! A data for shift bit of global v
	unsigned int dwShiftBitOfGlobalV;

	//! A data array for coefficient gg0
	int aiCoeffGG0[25];

	//! A data array for coefficient gg1
	int aiCoeffGG1[25];

	//! A data array for coefficient rb0
	int aiCoeffRB0[25];

	//! A data array for coefficient gg1
	int aiCoeffRB1[25];

	//! A data array for coefficient yy0
	int aiCoeffYY0[25];

	//! A data array for coefficient yy1
	int aiCoeffYY1[25];

	//! A data for coefficient shift bit gg0
	unsigned int dwCoeffShiftBitGG0;

	//! A data for coefficient shift bit gg1
	unsigned int dwCoeffShiftBitGG1;

	//! A data for coefficient shift bit rb0
	unsigned int dwCoeffShiftBitRB0;

	//! A data for coefficient shift bit rb1
	unsigned int dwCoeffShiftBitRB1;

	//! A data for coefficient shift bit yy0
	unsigned int dwCoeffShiftBitYY0;

	//! A data for coefficient shift bit yy1
	unsigned int dwCoeffShiftBitYY1;

	//! A data for weoght update
	unsigned int bWgtUpdate;

	//! A pointer data for weight buffer
	unsigned char* pbyWgtBuff;

	//! A data for weight buffer's physical address
	unsigned int dwWgtBufPhyAddr;
} VMF_IFP_FOCUS_VALUE_OPTION_T;

/*
 * A data structure for histogram
 */
typedef struct
{
	//! A data for histogram mode
	unsigned int dwHistMode;

	//! A data for grid weight enable
	unsigned int bGridWgtEn;

	//! A data for weight update
	unsigned int bWgtUpdate;

	//! A pointer data for weight buffer
	unsigned char* pbyWgtBuff;

	//! A data for weight buffer's physical address
	unsigned int dwWgtBufPhyAddr;
} VMF_IFP_HISTOGRAM_OPTION_T;

/*
 * A data structure for statistics
 */
typedef struct
{
	//! A data for shift up bit of input
	unsigned int dwShiftUpBitOfInput;

	//! A data for non avg mode enable
	unsigned int bNonAvgModeEn;

	//! A data for non avg mode g postition
	unsigned int bNonAvgModeGPosition;

	//! A data array for ctc gain r
	unsigned int adwCTCGainR[8];

	//! A data array for ctc gain b
	unsigned int adwCTCGainB[8];

	//! A data for roi size ratio
	unsigned int dwROISizeRatio;
} VMF_IFP_STATISTICS_OPTION_T;

/*
 * A data structure for yuv 2 bayer
 */
typedef struct
{
	//! A data for cbcr format
	unsigned int dwCbCrFormat;

	//! A data array for y to r coefficient
	int aiY2RCoeff[9];

	//! A data for input offset y
	int iInputOffsetY;

	//! A data for input offset cb
	int iInputOffsetCb;

	//! A data for input offset cr
	int iInputOffsetCr;

	//! A data for input offset r
	int iOutputOffsetR;

	//! A data for input offset g
	int iOutputOffsetG;

	//! A data for input offset b
	int iOutputOffsetB;
} VMF_IFP_YUV2BAYER_OPTION_T;

/*
 * A data structure for complex info
 */
typedef struct
{
	//! A data for complex with motion enable
	unsigned int bCplxWithMotionEn;

	//! A data array for bit map array
	unsigned int adwBitMapArray[16];

	//! A data for mr mode
	unsigned int dwMRMode;

	//! A data for mr coring
	unsigned int dwMRCoring;

	//! A data for mr gain
	unsigned int dwMRGain;

	//! A data for mr thdL
	unsigned int dwMRThdL;

	//! A data for mr thdH
	unsigned int dwMRThdH;

	//! A data array for mr level
	unsigned int adwMRLevel[7];
} VMF_IFP_CPLXINFO_OPTION_T;

/*
 * A data structure for lmap option
 */
typedef struct
{
	//! A data for input format
	unsigned int dwInputFormat;

	//! A data array for frame max
	unsigned int adwFrameMax[4];

	//! A data for luma curent norm bit
	unsigned int dwLumaCurNormBit;

	//! A data for luma curent coring
	unsigned int dwLumaCurCoring;

	//! A data for luma curent gain
	unsigned int dwLumaCurGain;

	//! A data for luma curent thdH
	unsigned int dwLumaCurThdH;

	//! A data for luma curent thdL
	unsigned int dwLumaCurThdL;
} VMF_IFP_LMAP_OPTION_T;

/*
 * A data structure for ext gamma
 */
typedef struct
{
	//! A data for ext gamma option's index
	unsigned int dwTblIndx;
} VMF_IFP_EXT_GAMMA_OPTION_T;

/*
 * A data structure for sub sample mode
 */
typedef struct
{
	//! A data for subsmaple mode
	VMF_IFP_VIDEO_SUB_SAMPLE_MODE eSubSampleMode;
} VMF_IFP_SUBSAMPLE_OPTION_T;

/*
 * A data structure for sub ir control
 */
typedef struct
{
	//! A data for sub ir option enable
	unsigned int bEnable;

	//! A data for sub ir option mode
	VMF_IFP_IR_SUB_SAMPLE_MODE eSubIrMode;
} VMF_IFP_SUB_IR_OPTION_T;

/*
 * A data structure for hdr control
 */
typedef struct
{
	//! A data for hdr ratio
	unsigned int bHDRRatio;
} VMF_IFP_HDR_RATIO_OPTION_T;

/*
 * A data structure for statistics information position control
 */
typedef struct
{
	//! Input source select for STAT instance 1
	unsigned int dwStatInst1SrcSel;
	//! Input source select for STAT instance 0
	unsigned int dwStatInst0SrcSel;
	//! Input source select for HIST instance 1
	unsigned int dwHistInst1SrcSel;
	//! Input source select for HIST instance 0
	unsigned int dwHistInst0SrcSel;
	//! Input source select for FV instance 1
	unsigned int dwFvInst1SrcSel;
	//! Input source select for FV instance 0
	unsigned int dwFvInst0SrcSel;
} VMF_IFP_STATS_POSITION_OPTION_T;

/*
 * A data structure for dynamic I-frame control
 */
typedef struct
{
	//! A data for dynamic I-frame option enable
	unsigned int dwDynamicIEn;
	//! Interval of changing GOP
	unsigned int dwDetectInterval;
	//! GOP for motion detected
	unsigned int dwShortGop;
	//! GOP for motion un-detected
	unsigned int dwLongGop;
	//! Threshold of motion mount to switching GOP
	unsigned int dwThreshold;
} VMF_IFP_DYNAMIC_I_OPTION_T;


#ifdef __cplusplus
}
#endif

#endif

