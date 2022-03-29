
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
#ifndef CONFIG_ISPE_H
#define CONFIG_ISPE_H

#include <comm/video_buf.h>
#include <vmf/config_fec.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ISP option flag
 */
typedef enum
{
	//! isp option flag: column width setting
	ISP_OPTION_COLUMN_WIDTH_SETTINGS,

	//! followings are the VMTK options
	ISP_OPTION_RESERVED_VMTK,
	
	//! isp option flag: block setting
	ISP_OPTION_BLOCK_SETTING,

	//! isp option flag: comp setting
	ISP_OPTION_COMP_SETTING,

	//! isp option flag: fish eye correction setting
	ISP_OPTION_FISH_EYE_CORRECTION_SETTINGS,

	//! isp option flag: noise reduction setting
	ISP_OPTION_NOISE_REDUCTION_SETTINGS,

	//! isp option flag: resize setting
	ISP_OPTION_RESIZE_SETTINGS,

	//! isp option flag: privacy mask setting
	ISP_OPTION_PRIVACY_MASK_SETTINGS,

	//! isp option flag: edge enhacement setting
	ISP_OPTION_EDGE_ENHANCEMENT_SETTINGS,

	//! isp option flag: rotation setting, flip then rotate 90 degree clockwise, or rotate 90 degree clockwise then mirror.
	ISP_OPTION_ROTATION_SETTINGS,

	//! isp option flag: deinterlace setting
	ISP_OPTION_DEINTERLACE_SETTINGS,

	//! isp option flag: geo lens distortion correction setting
	ISP_OPTION_GEO_LENS_DISTORTION_CORRECTION_SETTINGS,

	//! isp option flag: geometric transform reder setting
	ISP_OPTION_GEOMETRIC_TRANSFORM_RENDER_SETTINGS,

	//! isp option flag: geometric transform reder for dvs setting
	ISP_OPTION_GEOMETRIC_TRANSFORM_RENDER_DVS_SETTINGS,

	//! followings are the VMF options
	ISP_OPTION_RESERVED_VMF,
	
	//! isp option: init rs isp handle
	ISP_OPTION_NO_MAIN_OUT_INIT,

	//! isp option: update duplex resize fps
	ISP_OPTION_DUPLEX_RESIZE_FPS_SETTINGS,

	//! isp option: update isp motion dection
	ISP_OPTION_ISP_MOTION_DECTION_SETTINGS,

	//! isp option flag: geometric transform reder rotation setting
	ISP_OPTION_GEOMETRIC_TRANSFORM_RENDER_ROTATION_SETTINGS,
	
	//! isp option flag: reserved max
	ISP_OPTION_RESERVED_MAX
} VMF_ISP_OPTION_FLAG_T;

/*
 * ISP config flag
 */
typedef enum
{
	//! isp config flag: di
	ISP_CONFIG_DI = (1<<0),

	//! isp config flag: nr
	ISP_CONFIG_NR = (1<<1),

	//! isp config flag:
	ISP_CONFIG_RS = (1<<2),

	//! isp config flag: ee
	ISP_CONFIG_EE = (1<<3),

	//! isp config flag: pm
	ISP_CONFIG_PM = (1<<4),

	//! isp config flag: md
	ISP_CONFIG_MD = (1<<5),

	//! isp config flag: rt
	ISP_CONFIG_RT = (1<<6),

	//! isp config flag: gc
	ISP_CONFIG_GC = (1<<7),

	//! isp config flag: fec
	ISP_CONFIG_FEC = (1<<8),

	//! isp config flag: gtr
	ISP_CONFIG_GTR = (1<<9)
} VMF_ISP_CONFIG_FLAG_T;

/*
 * A data structure for isp option
 */
typedef struct
{
	//! A data for isp option flag
	VMF_ISP_OPTION_FLAG_T option_flag;

	//! A data array for isp option
	unsigned int adwData[3];
} VMF_ISP_OPTION_T;

/*
 * A data structure for isp resize config
 */
typedef struct
{
	//! A data for enable
	unsigned int bEnable;

	//! A data for disable main output
	unsigned int bDisableMainOutput;

	//! A data for anti aliasing
	unsigned int bAntiAliasing;

	//! A data for sharpness(0~5), small value is more sharper
	unsigned int dwSharpness;

	//! A data for channel num: 1 ~ 4
	unsigned int dwChannelNum;

	//! A data array for start x
	unsigned int dwStartX[VMF_MAX_RESIZE_NUM];

	//! A data array for width
	unsigned int dwWidth[VMF_MAX_RESIZE_NUM];

	//! A data array for height
	unsigned int dwHeight[VMF_MAX_RESIZE_NUM];

	//! A data array for stride
	unsigned int dwStride[VMF_MAX_RESIZE_NUM];

	//! A data array for fps
	unsigned int dwFps[VMF_MAX_RESIZE_NUM];
	
	//! A data array for index 
	unsigned int dwRsIndex[VMF_MAX_RESIZE_NUM];
} VMF_ISP_RS_CONFIG_T;

/*
 * A data structure for isp privacy mask config
 */
typedef struct
{
	//! A data for enable
	unsigned int bEnable;

	//! A data array for color yuv. Byte 0:y, 1:u, 2:v (range: 0 ~ 255)
	unsigned char abyColorYUV[3];

	//! A data for mask stride
	unsigned int dwMaskStride;

	//! A pointer data for mask buffer
	const unsigned char* pMaskBuffer;
} VMF_ISP_PM_CONFIG_T;

/*
 * A data structure for isp edge enhancement config
 */
typedef struct
{
	//! A data for enable
	unsigned int bEnable;

	//! A data for complex adapt enable
	unsigned int bCplxAdptEn;

	//! A data for distortion adapt enable
    unsigned int bDistAdptEn;

    //! A data for overshoot clamp enable
    unsigned int bOvershootClampEn;

    //! A data for overshoot max
    unsigned int dwOvershootMax;

    //! A data for overshoot min
    unsigned int dwOvershootMin;

    //! A data for luma adapt enable
    unsigned int bLumaAdptEn;

    //! A data for hpf shift enable
    unsigned int dwHPFShiftBit;

    //! A data for hpf coring
    unsigned int dwHPFCoring;

    //! A data for hpf gain
    unsigned int dwHPFGain;

    //! A data for ee post gain
    unsigned int dwEEPostGain;

    //! A data for complex gain
    unsigned int dwCplxGain;

    //! A data for complex shift bit
    unsigned int dwCplxShiftBit;

    //! A data for complex coring
    unsigned int dwCplxCoring;

    //! A data for da horizontal thdH
    unsigned int dwDAHorzThdH;

    //! A data for da horizontal thdL
    unsigned int dwDAHorzThdL;

    //! A data for da vertical thdH
    unsigned int dwDAVertThdH;

    //! A data for da vertical thdL
    unsigned int dwDAVertThdL;

    //! A data array for complex coefficient
    int aiCplxCoeff[9];

    //! A data array for hpf coefficient
    int aiHPFCoeff[9];

  	//! A data array for la curve x
    unsigned int adwLACurveX[9];

    //! A data array for la curve y
    unsigned int adwLACurveY[9];
} VMF_ISP_EE_CONFIG_T;

/*
 * A data structure for isp noise reduction 2D config
 */
typedef struct
{
	//! A data for enable
	unsigned int bEnable;

	//! A data for local search range: (range: 0 ~ 2, 0: 1x1, 1: 3x3, 2: 5x5)
	unsigned int dwLocalSearchRange;

	//! A data for global search range: (range: 0 ~ 1, 0: 3x3, 1: 5x5)
	unsigned int dwGlobalSearchRange;

	//! A data for weighted coefficient: (range: 0 ~ 1023)
	unsigned int dwWeightedCoeff;

	//! A data for min weight: (range: 0 ~ 1)
	unsigned int dwMinWeight;
} VMF_ISP_NR2D_CONFIG_T;

/*
 * A data structure for isp rotation config
 */
typedef struct
{
	//! A data for enable. CAUTION: flip then rotate 90 degree clockwise, or rotate 90 degree clockwise then mirror
	unsigned int bEnable;
} VMF_ISP_RT_CONFIG_T;

/*
 * A data structure for isp geo lens distrotion correction config
 */
typedef struct
{
	//! A data for enable
	unsigned int bEnable;

	//! A data for filtering mode: (range: 0 ~ 1, 0: 1x1, 1: 2x2)
	unsigned int dwFilteringMode;

	//! A data for center x
	unsigned int dwCenterX;

	//! A data for cebter y
	unsigned int dwCenterY;

	//! A data for offset x
	int iOffsetX;

	//! A data for offset y
	int iOffsetY;

	//! A data for strength: (range: -32 ~ 32)
	int iStrength;

	//! A data for vertical adjust: (range:0 ~ 1023)
	unsigned int dwVerticalAdjust;

	//! A data for source width
	unsigned int dwSrcWidth;

	//! A data for source height
	unsigned int dwSrcHeight;

	//! A data for destination width
	unsigned int dwDstWidth;

	//! A data for destination height
	unsigned int dwDstHeight;

	//! A data for enable background color
	unsigned int enableBGColor;

	//! A data array for background color yuv: Byte 0:y, 1:u, 2:v
	unsigned char abyBGColorYUV[3];
} VMF_ISP_GC_CONFIG_T;

/*
 * A data structure for isp deinterlace config
 */
typedef struct
{
	//! A data for enable
	unsigned int bEnable;

	//! A data for di mode: 0: Blending mode, 1: Spatial mode, 2: Motion adaptive mode
	unsigned int dwDIMode;

	//! A data for static map num: Reference static map number (range: 0 ~ 7, recommand value: 4)
	unsigned int dwStaticMapNum;

	//! A data for spatial search range: (range: 0 ~ 31, recommand value: 15)
	unsigned int dwSpatialSearchRange;
} VMF_ISP_DI_CONFIG_T;

/*
 * A data structure for isp fisheye config
 */
typedef struct
{
	//! A data for enable
	unsigned int bEnable;

	//! A data for destination width
	unsigned int dwDstWidth;

	//! A data for destination height
	unsigned int dwDstHeight;

	//! A data for destination stride
	unsigned int dwDstStride;

	//! A data for filtering mode (range: 0 ~ 2, recommand value: 2)
	unsigned int dwFilteringMode;

	//! A data array for fixd coefficient
	unsigned int adwFixedCoeff[8];

	//! A data for back ground enable
	unsigned int bBGColorEnable;

	//! A data array for back ground color yuv: Byte 0:y, 1:u, 2:v
	unsigned char abyBGColorYUV[3];

	//! A data for grid coefficient data size
	unsigned int dwCoeffDataDize;

	//! A data for grid coefficient data (virtual address)
	unsigned int dwCoeffData;

	//! A data for grid coefficient data (physical address)
	unsigned int dwCoeffPhysAddr;

	//! A data for coefficient buffer mode (0: VMTK_ISP will take care coeff buffer, buff_phys_addr: programer has to keep data consistency)
	unsigned int dwCoeffBuffMode;

	//! A data for isp line pixels (0: Default isp line pixel number, (range: 128 ~ 1280) value should align 8)
	unsigned int dwIspLinePixels;
} VMF_ISP_FEC_CONFIG_T;

/*
 * A data structure for isp Geometric Transform Render config
 */
typedef struct
{
	//! A data for enable
	unsigned int bEnable;

	//! A data for subview count
	unsigned int dwSubViewCount;

	//! A data for gtr config index
	unsigned int dwIndex;

	//! A data for destination width
	unsigned int dwDstWidth;

	//! A data for destination height
	unsigned int dwDstHeight;

	//! A data for subview x offset
	unsigned int dwViewOffX;

	//! A data for subview y offset
	unsigned int dwViewOffY;

	//! A data for process width
	unsigned int dwProcWidth;

	//! A data for process height
	unsigned int dwProcHeight;

	//! A data for process stride
	unsigned int dwProcStride;

	//! A data for fec app type
	VMF_FEC_APP_TYPE eCGEApp;

	//! A data for fec mode
	VMF_FEC_MODE_TYPE eCGEMode;

	//! A data for fec lens
	VMF_FEC_LENS_TYPE eCGELens;

	//! A data for focal
	float fFocal;

	//! A data for tilt
	float fTilt;
	//float fSpin;

	//! A data for pan
	float fPan;

	//! A data for zoom
	float fZoom;

	//! A data for area x
	float fAreaX;

	//! A data for area y
	float fAreaY;

	//! A data for object x
	float fObjX;

	//! A data for object y
	float fObjY;

	//! A data for object r
	float fObjR;

	//! A data for source x offset
	float fSrcOffX;

	//! A data for source y offset
	float fSrcOffY;

	//! A data for source radius offset
	float fSrcRadiusOffset;

	//! A data for destination x offset
	float fDstOffX;

	//! A data for destination y offset
	float fDstOffY;
	
	//! A data for the PTZ rotate(0: default, 1: 90 degree clockwisse, 2: 180 degree clockwisse 3: 90 degree counterclockwisse)
	VMF_FEC_ROTATE_TYPE eFECRotateType;

	//! A data for destination xy ratio
	float fDstXYRatio;
	//float fDstXYSkew;

	//! A data array for ldc coefficient
	float afLDCCoef[3];

	//! A data for the curvature of the rectangle. Range: 0.0 ~ 1.0.
	float fRectD;

	//! A data for the corner slope of the rectangle. Range: 0.0 ~ 1.0.
	float fRectL;

	//! A data for blengind withd which used in dual lens mode
	unsigned int dwBlendingWidth;

	//! A data for subview column width
	unsigned int dwColWidth;

	//! A data for block width
	unsigned int dwBlkWidth;

	//! A data for block height
	unsigned int dwBlkHeight;

	//! A array for homography matrix
	float afHomoMatrix[SIZE_OF_HOMOGRAPHY_MATRIX];

	//! A data for User-defined lens curve node number, should not exceed VMF_FEC_CRV_FIT_NODE_MAX_NUM
	unsigned int dwLensCurveNodeNum;

	//! A data ayyay for curve fit nodes x
	float afLensCurveNodeX[VMF_FEC_CRV_FIT_NODE_MAX_NUM];

	//! A data array for curve fit nodes y
	float afLensCurveNodeY[VMF_FEC_CRV_FIT_NODE_MAX_NUM];

	//! A data array for fix coef
	float afFixCoef[VMF_FIX_COEF_MAX_NUM];
} VMF_ISP_GTR_CONFIG_T;

/*
 * A data structure for isp block config
 */
typedef struct
{
	//! A data for block width (Range:0-4, 0:32, 1:64, 2:128, 3:256, 4:512)
    unsigned int dwBlkWidth;
    //! A data for block height (Range:0-3, 0:1, 1:2, 2:4, 3:8)
    unsigned int dwBlkHeight;
} VMF_ISP_BLK_CONFIG_T;

/*
 * A data structure for isp component config
 */
typedef struct
{
    // CI
    unsigned int  bCompCIEn; // complex map

    // MD
    unsigned int  bCompMDEn; // motion detection

    // MRF
    unsigned int  bCompMRFEn; // motion ratio map

    // LM
    unsigned int  bCompLMEn; // 8x8

    // IR
    unsigned int  bCompIREn; // 2x2, 4x4, 8x8, 16x16

    // PM
    unsigned int  bCompPMEn; // 2x2

    // MO // map only all size supported
    unsigned int  bCompMOEn;
}VMF_ISP_COMP_CONFIG_T;


#ifdef __cplusplus
}
#endif

#endif

