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
#ifndef CONFIG_FEC_H
#define CONFIG_FEC_H

#ifdef __cplusplus
extern "C" {
#endif

/*
app      |       mode                 |    x       |     y       | zoom     | focal_length |  pan     |  tilt
===============================================================================================================
Ceiling  |      Area                  | 0~in_width | 0~in_height | 0.1~10.0 | 0.3~1.0      |          |
Table    |      PTZ                   |            |             | 0.1~10.0 | 0.3~1.0      | -180~180 | 0~90
         | Panorama 360 full          |            |             | 0.1~10.0 |              | -180~180 | 0~90
         | Panorama 360 separated     |            |             | 0.1~10.0 |              | -180~180 | 0~90
         | Panorama 360 half          |            |             | 0.1~10.0 |              | -180~180 | 0~90
         | VR Sphere                  |            |             |          | 0.3~1.0      |          |
==============================================================================================================
Wall     |      Area                  | 0~in_width | 0~in_height | 0.1~10.0 | 0.3~1.0      |          |
         |      PTZ                   |            |             | 0.1~10.0 | 0.3~1.0      | -90~90   | -90~90
         | Panorama 180 all direction |            |             | 0.1~10.0 | 0.3~1.0      | -90~90   | -90~90
         | Panorama 180 one direction |            |             | 0.1~10.0 |              | -90~90   | -90~90
         | Panorama 180 two direction |            |             | 0.1~10.0 |              | -90~90   | -90~90
         | VR Sphere                  |            |             |          | 0.3~1.0      |          |
==============================================================================================================
*/

/*
 * homography matrix size
 */
#define SIZE_OF_HOMOGRAPHY_MATRIX 16

/*
 * vmf fec crv max num fit node
 */
#define VMF_FEC_CRV_FIT_NODE_MAX_NUM 64

/*
 * fix coef size
 */
#define VMF_FIX_COEF_MAX_NUM 6

/*
 * option_flags
 */
#define VMF_FEC_OPTION_P180_AUTO_ZOOM (1<<0)

/*
 * FEC processing method.
 */
typedef enum
{
	//! FEC method: geometry transform render
	VMF_FEC_METHOD_GTR = 0, 

	//! FEC method: coefficient generate
	VMF_FEC_METHOD_CGE      
} VMF_FEC_METHOD;


/*
 * FEC rotate method 
 */
typedef enum
{
	//! Fec Rotate: 0 degree
	VMF_FEC_ROTATE_DEFAULT = 0, 
	
	//! Fec Rotate: 90 degree clockwisse
	VMF_FEC_ROTATE_CLOCKWISE_90,
	
	//! Fec Rotate: 180 degree clockwisse
	VMF_FEC_ROTATE_CLOCKWISE_180,
	
	//! Fec Rotate: 90 degree counterclockwisse
	VMF_FEC_ROTATE_COUNTER_CLOCKWISE_90
} VMF_FEC_ROTATE_TYPE;

/*
 * FEC processing unit.
 */
typedef enum
{
	//! FEC grid size: 4X4
	VMF_FEC_GRID_4X4     = 4,

	//! FEC grid size: 8X8
	VMF_FEC_GRID_8X8     = 8,

	//! FEC grid size: 16X16
	VMF_FEC_GRID_16X16   = 16,

	//! FEC grid size: 32X32
	VMF_FEC_GRID_32X32   = 32,

	//! FEC grid size: 64X64
	VMF_FEC_GRID_64X64   = 64,

	//! FEC grid size: 128X128
	VMF_FEC_GRID_128X128 = 128,
} VMF_FEC_GRID_SIZE_TYPE;

/*
 * FEC Coefficient mode.
 */
typedef enum
{
	//! FEC Coefficient mode: orginal
	VMF_FEC_COEF_MODE_ORIG = 0,

	//! FEC Coefficient mode: area
	VMF_FEC_COEF_MODE_AREA,

	//! FEC Coefficient mode: ptz
	VMF_FEC_COEF_MODE_PTZ,

	//! FEC Coefficient mode: p360
	VMF_FEC_COEF_MODE_P360,

	//! FEC Coefficient mode: p180
	VMF_FEC_COEF_MODE_P180,

	//! FEC Coefficient mode: virtual reality - sphere
	VMF_FEC_COEF_MODE_VR,    

	//! FEC Coefficient mode: object 
	VMF_FEC_COEF_MODE_OBJ,

	//! FEC Coefficient mode: dull
	VMF_FEC_COEF_MODE_DUAL,

	//! FEC Coefficient mode: projection transform
	VMF_FEC_COEF_MODE_PT,  

	//! FEC Coefficient mode: do nothing
	VMF_FEC_COEF_MODE_NULL,  

	//! FEC Coefficient mode: max num
	VMF_FEC_COEF_MODE_NUM
} VMF_FEC_COEF_MODE;

typedef enum
{
	VMF_FEC_DUAL_VIDEO_360 = 0,

	VMF_FEC_DUAL_VIDEO_RS_ORIGINAL,

	VMF_FEC_DUAL_VIDEO_P180_UP_DOWN,

	VMF_FEC_DUAL_VIDEO_1O_UP_DOWN
	
}VMF_FEC_DUAL_OUTPUT_TYPE;

/*
 * FEC mode.
 */
typedef enum
{
	//! FEC type: area
	VMF_FEC_MODE_AREA,

	//! FEC type: ptz
	VMF_FEC_MODE_PTZ,

	//! FEC panorama 180 mode type: all
	VMF_FEC_MODE_PANO_180_ALL_DIRECTION,

	//! FEC panorama 180 mode type: one direction
	VMF_FEC_MODE_PANO_180_ONE_DIRECTION,

	//! FEC panorama 180 mode type: two direction
	VMF_FEC_MODE_PANO_180_TWO_DIRECTION,

	//! FEC panorama 360 mode type: full
	VMF_FEC_MODE_PANO_360_FULL,

	//! FEC panorama 360 mode type: seperate
	VMF_FEC_MODE_PANO_360_SEPE,

	//! FEC panorama 360 mode type: half
	VMF_FEC_MODE_PANO_360_HALF,

	//! FEC mode: lens distortion correction 
	VMF_FEC_MODE_LDC,

	//! FEC mode: original
	VMF_FEC_MODE_ORI,

	//! FEC virtual reality mode: sphere
	VMF_FEC_MODE_VR_SPHERE,

	//! FEC virtual reality mode: cylinder
	VMF_FEC_MODE_VR_CYLINDER,

	//! FEC virtual reality mode: object
	VMF_FEC_MODE_OBJECT,

	//! FEC Panorama 360  mode: full
	VMF_FEC_MODE_PANO_360_FULL_EQUIRECT,

	//! FEC Panorama 360  mode: sepeate
	VMF_FEC_MODE_PANO_360_SEPE_EQUIRECT,

	//! FEC Panorama 360  mode: half
	VMF_FEC_MODE_PANO_360_HALF_EQUIRECT,

	//! FEC mode: dual lens 0
	VMF_FEC_MODE_DUAL_LENS_0,

	//! FEC mode: dual lens 1
	VMF_FEC_MODE_DUAL_LENS_1,

	//! FEC mode: dual 720 clbr
	VMF_FEC_MODE_DUAL_P720_CLBR,

	//! FEC mode: dual 720 disp
	VMF_FEC_MODE_DUAL_P720_DISP,

	//! FEC mode: projection transform
	VMF_FEC_MODE_PT	
} VMF_FEC_MODE_TYPE;

/*
 * FEC App type.
 */
typedef enum
{
	//! FEC app type: ceiling. Except panorama 180 mode
	VMF_FEC_APP_CEIL,

	//! FEC app type: table. Except panorama 180 mode
	VMF_FEC_APP_TABL,

	//! FEC app type: wall. Except panorama 360 mode
	VMF_FEC_APP_WALL,

	//! FEC app type: ldc
	VMF_FEC_APP_LDC,

	//! FEC app type: p720
	VMF_FEC_APP_P720,

	//! FEC app type: dvs
	VMF_FEC_APP_DVS,

	//! FEC app type: cge
	VMF_FEC_APP_CGE
} VMF_FEC_APP_TYPE;

/*
 * FEC Lens type.
 */
typedef enum
{
	//! FEC lens type: stereographic
	VMF_FEC_LENS_STEREOGRAPHIC,

	//! FEC lens type: equisolidangle
	VMF_FEC_LENS_EQUISOLIDANGLE,

	//! FEC lens type: equidistant
	VMF_FEC_LENS_EQUIDISTANT,

	//! FEC lens type: orthographic
	VMF_FEC_LENS_ORTHOGRAPHIC,

	//! FEC lens type: ldc
	VMF_FEC_LENS_LDC,

	//! FEC lens type: no distort
	VMF_FEC_LENS_NODISTORT,

	//! FEC lens type: max
	VMF_FEC_LENS_USER_DEF,
} VMF_FEC_LENS_TYPE;

/*
 * A data structure for the configuration of fisheye correction
 */
typedef struct
{
	//! A data for fec method: GTR or CoeffGen
	VMF_FEC_METHOD eFecMethod;

	//! A data for video width. It must be even. (Range: 2-65534.)
	unsigned int dwInWidth;

	//! A data for video height. It must be even. (Range: 2-65534.)
	unsigned int dwInHeight;

	//! A data for horizontal offset between the calculated center point of image and the actual center point of image. Range: -(input height)/2 ~ (input height)/2.
	float dwInOffsetX;

	//! A data for vertical offset between the calculated center point of image and the actual center point of image. Range: -(input height)/2 ~ (input height)/2.
	float dwInOffsetY;

	//! A data for vertical offset between the calculated center point of image and the actual center point of image. Range: -(input height)/2 ~ (input height)/2. Range: 2-65534.
	float dwInRadiusOffset;

	//! A data for output video width. The value must be even. Range: grid size ~ (grid size)x4096.
	unsigned int dwOutWidth;

	//! A data for output video height. The value must be even. Range: (grid size) ~ 65534.
	unsigned int dwOutHeight;

	//! A data for output buffer width. Range: (grid size) ~ (grid size)x4096.
	unsigned int dwOutStride;

	//! A data for library processing unit. Recommend: 8.
	VMF_FEC_GRID_SIZE_TYPE eGridSize;

	//! A data for mask function: 0:disable, 1: source image mask, 2: destination image mask
	unsigned int bMaskEnable;

	//! A data for mask function: Mask width. The value should equal to the input frame width.
	unsigned int dwMaskWidth;

	//! A data for mask function: Mask height. The value should equal to the input frame height.
	unsigned int dwMaskHeight;

	//! A data for mask function: Mask height. The value should equal to the input frame height.
	unsigned int dwMaskStride;

	//! A data for mask function: Pointer to the mask buffer. The size of mask buffer should equal to the (input frame stride/8)*(input frame height).
	unsigned char* pbyMaskBuf;

	//! A data for coefficient data size.
	unsigned int dwCoeffDataSize;

	//! A pointer data for the coefficient data.
	unsigned char* pbyCoeffData;

	//! A data for the coefficient data stride.
	unsigned int dwCoeffDataStride;

	//! A pointer data for GTR data
	unsigned char* pbyGtrData;

	//! A pointer data for out-of-boundary information: Check if the center point of de-warped frame inside the original image.
	unsigned int *pbIsOutBndCenter;

	//! A pointer data for out-of-boundary information: if the left up corner of de-warped frame inside the original image.
	unsigned int *pbIsOutBndLeftUp;

	//!  A pointer data for out-of-boundary information: if the left down corner of de-warped frame inside the original image.
	unsigned int *pbIsOutBndLeftDown;

	//!  A pointer data for out-of-boundary information: if the right up corner of de-warped frame inside the original image.
	unsigned int *pbIsOutBndRightUp;

	//!  A pointer data for out-of-boundary information: if the right down corner of de-warped frame inside the original image.
	unsigned int *pbIsOutBndRightDown;

	//! A data for out-of-boundary information: 0: default isp line pixel number, max value:1280, other value should consult with technical support
	unsigned int  dwIspLinePixels;

	//! A data for destination rotation angle. (Range -180.0~180.0)
	float fDstRotateAngle;

	//! A data for user-defined lens curve node number, should not exceed VMF_FEC_CRV_FIT_NODE_MAX_NUM
	unsigned int dwLensCurveNodeNum;

	//! A data for curve fit nodes x
	float afLensCurveNodeX[VMF_FEC_CRV_FIT_NODE_MAX_NUM];

	//! A data for curve fit nodes y
	float afLensCurveNodeY[VMF_FEC_CRV_FIT_NODE_MAX_NUM];
} VMF_FEC_INFO_T;

/*
 * A data structure for the layout information of fisheye correction.
 */
typedef struct
{
	//! A data for output video width. The value must be even. Range: (grid size) ~ (output width).
	unsigned int dwWidth;

	//! A data for output video height. The value must be even. Range: (grid size) ~ (output height).
	unsigned int dwHeight;

	//! A data for output buffer width. The value must be even. Range: (grid size) ~ (output height).
	unsigned int dwStride;

	//! A data for horizontal offset. The value must be even. Range: -(output width)/2 ~ (output width)/2.
	unsigned int dwOffsetX;

	//! A data for vertical offset. The value must be even. Range: -(output height)/2 ~ (output height)/2.
	unsigned int dwOffsetY;
} VMF_FEC_ROI_T;

/*
 * A data structure for the de-warped frame information of fisheye correction.
 */
typedef struct
{
	//! A data for the center point of de-warped frame is /0: inside the original image. / 1:outside the original image.
	unsigned int bIsOutBndCenter;

	//! A data for the left-top corner point of de-warped frame is / 0: inside the original image, / 1:outside the original image.
	unsigned int bIsOutBndLeftUp;

	//! A data for the left-bottom corner point of de-warped frame is / 0: inside the original image, / 1:outside the original image.
	unsigned int bIsOutBndLeftDown;

	//! A data for the right-top corner point of de-warped frame is / 0: inside the original image, / 1:outside the original image.
	unsigned int bIsOutBndRightUp;

	//! A data for the right-bottom point of de-warped frame is / 0: inside the original image, / 1:outside the original image.
	unsigned int bIsOutBndRightDown;
} VMF_FEC_STATE_T;

/*
 * A data structure for the configuration of FEC area mode.
 */
typedef struct
{
	//! A data for the x-axis of center point to the mapped area.
	unsigned int dwCenterX;

	//! A data for the y-axis of center point to the mapped area.
	unsigned int dwCenterY;

	//! A data for the Zoom. (To magnify or shrink the image size. Range: 0.01 ~ 100.)
	float fZoom;

	//! A data for the focallength. (Strength to correct the fisheye lens distortion. Range: 0.1 ~ 1.5.)
	float fFocalLength;

	VMF_FEC_ROTATE_TYPE eFECRotateType;

	//!A data for the fec application type.
	VMF_FEC_APP_TYPE eAppType;

	//! A data for the fec lens type.
	VMF_FEC_LENS_TYPE eLensType;

	//! A pointer data for the output location setting.
	VMF_FEC_ROI_T* ptRoi;

	//! A data for the radius of output area. (default 0: the half of output roi width)
	unsigned int dwOutRadius;
} VMF_FEC_AREA_CONFIG_T;

/*
 * A data structure for the configuration of FEC object mode.
 */
typedef struct
{
	//! A data for the x-axis of center point to the mapped area.
	unsigned int dwCenterX;
	//! A data for the y-axis of center point to the mapped area.
	unsigned int dwCenterY;

	//! A data for the input fisheye image radius. Its value must be even. It should be between 2~65534.
	unsigned int dwRadius;

	//! A data for the zoom (To magnify or shrink the image size. Range: 0.01 ~ 100.)
	float fZoom;

	//! A data for focal length (Strength to correct the fisheye lens distortion. Range: 0.1 ~ 1.5.)
	float fFocalLength;

	//! A data for fec application type.
	VMF_FEC_APP_TYPE eAppType;

	//! A data for fec lens type.
	VMF_FEC_LENS_TYPE eLensType;

	//! A pointer data for the output location setting.
	VMF_FEC_ROI_T* ptRoi;

	//! A data for the radius of output area. It is suggested to be the half of minimal of output width and height. Default 0: the half of output ROI width
	unsigned int dwOutRadius;
} VMF_FEC_OBJ_CONFIG_T;

/*
 * A data structure for the configuration of FEC PTZ.
 */
typedef struct
{
	//! A data for pan angle. Range: -180 ~ 180.
	float fPan;

	//! A data for tilt angle. Range: -135 ~ 135.
	float fTilt;

	//! A data for zoom (To magnify or shrink the image size. Range: 0.01 ~ 100.)
	float fZoom;

	//! A data for the focal strength (Strength to correct the fisheye lens distortion. Range: 0.1 ~ 1.5.)
	float fFocalLength;

	//! A data for the PTZ rotate(0: default, 1: 90 degree clockwisse, 2: 180 degree clockwisse 3: 90 degree counterclockwisse)
	VMF_FEC_ROTATE_TYPE eFECRotateType;

	//! A data for fec application type.
	VMF_FEC_APP_TYPE eAppType;

	//! A data for fec lens type.
	VMF_FEC_LENS_TYPE eLensType;

	//! A pointer data for the output location setting.
	VMF_FEC_ROI_T* ptRoi;

	//! A data for the radius of output area. It is suggested to be the half of minimal of output width and height. Default 0: the half of output ROI width
	unsigned int dwOutRadius;

	//! A data for the horizontal offset of output area.
	float fDstOffsetX;

	//! A data for the vertical offset of output area.
	float fDstOffsetY;
} VMF_FEC_PTZ_CONFIG_T;

/*
 * A data structure for the configuration of FEC Panorama 360 mode.
 */
typedef struct
{
	//! A data for pan angle. Range: -180 ~ 180.
	float fPan;

	//! A data for tilt angle. Range: 0 ~ 90.
	float fTilt;

	//! A data for zoom (To magnify or shrink the image size. Range: 0.1 ~ 10.)
	float fZoom;

	//! A data for focal length (Strength to correct the fisheye lens distortion. Range: 0.3 ~ 1.5.)
	float fFocalLength;

	//! A data for fec application type. (table / ceiling mount)
	VMF_FEC_APP_TYPE eAppType;

	//! A data for fec mode type.
	VMF_FEC_MODE_TYPE eModeType;

	//! A data for fec lens type.
	VMF_FEC_LENS_TYPE eLensType;

	//! A pointer data for the output location setting.
	VMF_FEC_ROI_T* ptRoi;

	//! A data for the radius of output area. It is suggested to be the half of minimal of output width and height. Default 0: the half of output ROI width
	unsigned int dwOutRadius;

	//! A data for version (0: default p360 mode, 1: new P360 mode)
	unsigned int dwVersion;
} VMF_FEC_P360_CONFIG_T;

/*
 * A data structure for the configuration of FEC Panorama 360 mode.
 */
typedef struct
{
	//! A data for pan angle. Range: -90 ~ 90.
	float fPan;

	//! A data for tilt angle. Range: -90 ~ 90.
	float fTilt;

	//! A data for zoom (To magnify or shrink the image size. Range: 0.01 ~ 100.)
	float fZoom;

	//! A data for focal length (Strength to correct the fisheye lens distortion. Range: 0.3 ~ 1.5.)
	float fFocalLength;

	//! A data for the curvature of the rectangle. Range: 0.0 ~ 1.0.
	float fRectCurvature;

	//! A data for the corner slope of the rectangle. Range: 0.0 ~ 1.0.
	float fRectSlope;

	//! A data for the horizontal offset of output area.
	float fDstOffsetX;

	//! A data for the vertical offset of output area.
	float fDstOffsetY;

	//! A data for the width/height ratio of output area.
	float fDstXYRatio;

	//! A data for the PTZ rotate(0: default, 1: 90 degree clockwisse, 2: 180 degree clockwisse 3: 90 degree counterclockwisse)
	VMF_FEC_ROTATE_TYPE eFECRotateType;

	//! A data for fec mode type.
	VMF_FEC_MODE_TYPE eModeType;

	//! A data for fec lens type. All-direction needs to set lens_type, others don't need.
	VMF_FEC_LENS_TYPE eLensType;

	//! A pointer data for the output location setting.
	VMF_FEC_ROI_T* ptRoi;

	//! A data for the radius of output area. (default 0: non-orig the half of output roi width, orig the half of output roi height)
	unsigned int dwOutRadius; 

	//! A data for the optional functions flags. Currently it only supports VMF_FEC_OPTION_P180_AUTO_ZOOM
	unsigned int dwOptionFlags;
} VMF_FEC_P180_CONFIG_T;

/*
 * A data structure for the configuration of FEC original mode.
 */
typedef struct
{
	//! A data for zoom (To magnify or shrink the image size. Range: 0.1 ~ 10.0.)
	float fZoom;

	//! A data for fec application type.
	VMF_FEC_APP_TYPE eAppType;

	//! A pointer data for the output location setting.
	VMF_FEC_ROI_T* ptRoi;

	//! A data for the radius of output area. It is suggested to be the half of minimal of output width and height. Default 0: the half of output ROI HEIGHT.
	unsigned int dwOutRadius;

	//! A data for the PTZ rotate(0: default, 1: 90 degree clockwisse, 2: 180 degree clockwisse 3: 90 degree counterclockwisse)
	VMF_FEC_ROTATE_TYPE eFECRotateType;
} VMF_FEC_ORIG_CONFIG_T;

/*
 * A data structure for the configuration of FEC original mode.
 */
typedef struct
{
	//! A pointer data for the coefficient data.
	void* pCoeffData;

	//! A data for the coefficient data size.
	unsigned int dwCoeffDataSize;

} VMF_FEC_PT_CONFIG_T;

/*
 * A data structure for the configuration of FEC Virtual Reality mode.
 */
typedef struct
{
	//! A data for pan angle. Range: -180 ~ 180.
	float fPan;

	//! A data for tilt angle. Range: 0 ~ 90.
	float fTilt;

	//! A data for focal length (Strength to correct the fisheye lens distortion. Range: 0.3 ~ 1.5.)
	float fFocalLength;

	//! A data for  fec application type. (Table / Ceiling)
	VMF_FEC_APP_TYPE  eAppType;

	//! A data for fec mode type.
	VMF_FEC_MODE_TYPE eModeType; //! VMF_FEC_MODE_VR_SPHERE

	//! A data for fec lens type.
	VMF_FEC_LENS_TYPE eLensType; //! COEFGEN_LENS_EQUISOLIDANGLE

	//! A pointer data for the output location setting.
	VMF_FEC_ROI_T* ptRoi;

	//! A data for the radius of output area. It is suggested to be the half of minimal of output width and height. Default 0: the half of output ROI width
	unsigned int dwOutRadius;
} VMF_FEC_VR_CONFIG_T;

/*
 * A data structure for the configuration of FEC dual lens mode.
 */
typedef struct
{
	//! A data for pan angle. Range: -90 ~ 90.
	float fPan;

	//! A data for tilt angle. Range: -90 ~ 90.
	float fTilt;

	//! A data for zoom (To magnify or shrink the image size. Range: 0.01 ~ 100.)
	float fZoom;

	//! A data for focal length (Strength to correct the fisheye lens distortion. Range: 0.3 ~ 1.5.)
	float fFocalLength;

	//! A data for the horizontal offset of output area.
	float fDstOffsetX;

	//! A data for the vertical offset of output area.
	float fDstOffsetY;

	//! A data for the width/height ratio of output area.
	float fDstXYRatio;

	//! A data for fec mode type.
	VMF_FEC_MODE_TYPE eModeType;

	//! A data for fec lens type. If VMF_FEC_P180_AUTO_ZOOM is set to option_flags, don't care lens_type
	VMF_FEC_LENS_TYPE eLensType;

	//! A pointer data for  the output location setting.
	VMF_FEC_ROI_T* ptRoi;

	//! A data for the radius of output area. (default 0: non-orig the half of output roi width, orig the half of output roi height)
	unsigned int dwOutRadius; 

	//! A data for blending width for stitching two image. It only used in VMF_FEC_MODE_DUAL_LENS_1 mode.
	unsigned int dwBlendingWidth;

	//! A data for homography matrix for dual sensor mode (The size must be SIZE_OF_HOMOGRAPHY_MATRIX).
	const float *pfHomographyMatrix;

	//! A data for the optional functions flags. Currently it only supports VMF_FEC_OPTION_P180_AUTO_ZOOM
	unsigned int dwOptionFlags;
} VMF_FEC_DUAL_LENS_CONFIG_T;

/*
 * A data structure for the configuration of looking-up FEC pixel position.
 */
typedef struct
{
	//! A data for input x-position
	float fInputX;

	//! A data for input y-position
	float fInputY;

	//! A data for output x-position
	float fOutputX;

	//! A data for output y-position
	float fOutputY;

	//! A data for pan angle. Range: -180 ~ 180.
	float fPan;

	//! A data for tilt angle. Range: 0 ~ 90.
	float fTilt;

	//! A data for zoom (To magnify or shrink the image size. Range: 0.1 ~ 10.0.)
	float fZoom;

	//! A data for focal length (Strength to correct the fisheye lens distortion. Range: 0.3 ~ 1.5.)
	float fFocalLength;

	//! A data for the curvature of the rectangle. Range: 0.0 ~ 1.0.
	float fRectCurvature;

	//! A data for the corner slope of the rectangle. Range: 0.0 ~ 1.0.
	float fRectSlope;
 
	//! A data for isforward, 1: look up de-warped pixel position from original position 0: look up original pixel from de-warped position
	int bIsforward;

	//! A data for fec application type.
	VMF_FEC_APP_TYPE eAppType;

	//! A data for fec mode type.
	VMF_FEC_MODE_TYPE eModeType;

	//! A data for fec lens type.
	VMF_FEC_LENS_TYPE eLensType;

	//! A pointer data for the output location setting.
	VMF_FEC_ROI_T* ptRoi;

	//! A data for the radius of output area. It is suggested to be the half of minimal of output width and height. Default 0: the half of output ROI width
	unsigned int dwOutRadius;
} VMF_FEC_LOOKUP_CONFIG_T;

/*
 * A data structure for the configuration of FEC calibration.
 */
typedef struct
{
	//! A data for input frame width.
	unsigned int dwInWidth;

	//! A data for input frame height.
	unsigned int dwInHeight;

	//! A data for input frame stride. i.e. buffer width
	unsigned int dwInStride;

	//! A pointer data for input Y frame buffer
	unsigned char *pbyInFrameY;
} VMF_FEC_CALIBRATE_CONFIG_T;

typedef struct 
{
	//! Array for offset center X
	int aiOffsetX[2];

	//! Array for offset center Y
	int aiOffsetY[2];

	//! Array for offset radius
	int aiOffsetRadius[2];	
}VMF_FEC_CALIBRATE_OUTPUT_T;

/*
 * A data structure for the configuration of FEC Coefficient gen.
 */
typedef struct
{
	//! A data for fec coefficient mode
	VMF_FEC_COEF_MODE eMode;

	//! A pointer data for mode configuration
	void* ptModeCfg;
} VMF_FEC_COEFGEN_CONFIG_T;

/**
 * @brief This function is to setup the data structure of the Fisheye-Correction Coefficient Generator software engine. The coefficient data is used to transform image by ImageProc hardware engine.
 *
 * @param[in/out] ptInfo The pointer of VMF_FEC_INFO_T.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_FEC_Info_Init(VMF_FEC_INFO_T* ptInfo);

/**
 * @brief This function is to release the data structure of the Fisheye-Correction Coefficient Generator software engine.
 *
 * @param[in/out] ptInfo The pointer of VMF_FEC_INFO_T.
 * @return Success: 0  Fail: negative integer.
 * @remark: If use_outer_buff of vmtk_imgproc_fec_config_t is true, do not call VMF_ImgProc_ProcessOneFrame() after VMF_FEC_Info_Release() because the coeff_data of VMF_FEC_INFO_T will be released .
 */
int VMF_FEC_Info_Release(VMF_FEC_INFO_T* ptInfo);

/**
 * @brief Get the information of generation.
 *
 * @param[in] ptInfo The pointer of VMF_FEC_INFO_T
 * @param[in/out] ptGenState The pointer of VMF_FEC_STATE_T
 * @return Success: 0  Fail: negative integer.
 * @remark: Use VMF_FEC_Info_Init() to setup VMF_FEC_INFO_T* info first.
 */
int VMF_FEC_GetGenState(const VMF_FEC_INFO_T* ptInfo, VMF_FEC_STATE_T* ptGenState);

/**
 * @brief This function is to get center and radius offset of the input image to calibrate coefficient data.
 *
 * @param[in] ptConfig The pointer of VMF_FEC_CALIBRATE_CONFIG_T
 * @param[out] out_center_x The pointer of x-coordinate of the center
 * @param[out] out_center_y The pointer of y-coordinate of the center
 * @param[out] out_radius   radius
 * @return Success: 0  Fail: negative integer.
 */
int VMF_FEC_Calibrate(const VMF_FEC_CALIBRATE_CONFIG_T *ptConfig,
		unsigned int *pdwOutCenterX,
		unsigned int *pdwOutCenterY,
		unsigned int *pdwOutRadius);

/**
 * @brief This function is to lookup pixel location between the original image and de-warped image.
 *
 * @param[in] ptInfo The pointer of VMF_FEC_INFO_T
 * @param[in/out] ptConfig The pointer of VMF_FEC_LOOKUP_CONFIG_T
 * @return Success: 0  Fail: negative integer.
 * @remark: Use VMF_FEC_Info_Init() to setup VMF_FEC_INFO_T* info first.
 */
int VMF_FEC_PixLookup(const VMF_FEC_INFO_T* ptInfo, VMF_FEC_LOOKUP_CONFIG_T* ptConfig);

/**
 * @brief Generate Coef data of different fec mode by VMF_FEC_COEFGEN_CONFIG_T input.
 *
 * @param[in] ptInfo The pointer of VMF_FEC_INFO_T
 * @param[in] ptConfig The pointer of VMF_FEC_COEFGEN_CONFIG_T
 * @return Success: 0  Fail: negative integer.
 * @remark: Use VMF_FEC_Info_Init() to setup VMF_FEC_INFO_T* info first.
 */
int VMF_FEC_Coeff_Gen(const VMF_FEC_INFO_T* ptInfo, const VMF_FEC_COEFGEN_CONFIG_T* ptConfig);

#ifdef __cplusplus
}
#endif

#endif
