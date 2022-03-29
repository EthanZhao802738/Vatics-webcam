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
#ifndef VIDEO_SOURCE_H
#define VIDEO_SOURCE_H

#include <config_isp.h>
#include <config_vsrc.h>
#include <config_fec.h>
#include <config_osd.h>
#include <config_asc.h>
#include <config_ivs.h>
#include <config_ae.h>
#include <TextRender/text_render.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_VSRC_OUTPUT_COUNT 4   //! Maximum multi-dewarp stream count

typedef struct VMF_VSRC_HANDLE_T VMF_VSRC_HANDLE_T;
/**
 * An enumeration for VSRC application mode
 */
typedef enum
{
	VMF_VSRC_APP_MODE_NORMAL = 0,  //! VSRC application normal mode
	VMF_VSRC_APP_MODE_FUSION,      //! VSRC application fusion mode
	VMF_VSRC_APP_MODE_360,         //! VSRC application dual lens mode
	VMF_VSRC_APP_MODE_BT1120,      //! VSRC application BT1120(Dual BT656 Input) mode
	VMF_VSRC_APP_MODE_14BIT, 	   //! VSRC application 14BIT mode
	VMF_VSRC_APP_MODE_DMA422TO420	//! Using alpha blending (YUV422 to YUV420) to replace IFP processing. (For IT6604)
} VMF_VSRC_APP_MODE;

/**
 * An enumeration for ISP mode
 */
typedef enum
{
	VMF_ISP_MODE_DISABLE = 0,//! ISP mode disable
	VMF_ISP_MODE_FEC,      	 //! ISP FEC mode
	VMF_ISP_MODE_FEC_C,      //! ISP FEC_C mode with region mode or 360 mode
	VMF_ISP_MODE_GC,         //! ISP GC mode
	VMF_ISP_MODE_DI,         //! ISP DI mode
	VMF_ISP_MODE_RT          //! ISP RT mode
} VMF_ISP_MODE;

/**
 * An enumeration for DI mode
 */
typedef enum
{
	VMF_DI_DISABLE = 0,			//! DI mode disable
	VMF_DI_BLEND_MODE,      	//! DI blend mode
	VMF_DI_SPATIAL_MODE,      	//! DI spatial mode
	VMF_DI_MA_MODE,         	//! DI Motion Adaptive mode
} VMF_DI_MODE;

/**
 * A structure for VSRC fisheye initial config
 */
typedef struct
{
	VMF_FEC_COEF_MODE eCoeffMode;  //! FEC transformation mode
	VMF_FEC_METHOD eFecMethod;     //! FEC transformation method
	void *ptFecConfig;             //! FEC transformation config according to eCoeffMode
	VMF_FEC_GRID_SIZE_TYPE eGridSize;  //! FEC transformation grid size
	unsigned int dwClearBackColor;    //! Stream background color, Byte 0: on-off flag, 1: y color, 2:u color, 3:v color

	int iFecCenterOffsetX;  //! FEC center horizontal offset
	int iFecCenterOffsetY;  //! FEC center vertical offset
	int iFecRadiusOffset;   //! FEC radius offset
	//! A data for user-defined lens curve node number, should not exceed VMF_FEC_CRV_FIT_NODE_MAX_NUM
	unsigned int dwLensCurveNodeNum;
	//! A data ayyay for curve fit nodes x
	float afLensCurveNodeX[VMF_FEC_CRV_FIT_NODE_MAX_NUM];
	//! A data array for curve fit nodes y
	float afLensCurveNodeY[VMF_FEC_CRV_FIT_NODE_MAX_NUM];
} VMF_VSRC_FEC_INIT_CONFIG_T;

/**
 * A structure for VSRC frontend initial config
 */
typedef struct
{
	unsigned int dwSensorConfigCount; //! VIC sensor config count, range: 1 ~ VMF_IFP_FUSION_MAX_CHANNEL
	const char* apszSensorConfig[VMF_IFP_FUSION_MAX_CHANNEL]; //! VIC sensor config file path.
	const char* pszIvsConfig;	 //! IVS config file path

	unsigned int dwSubSampleMode; //! IFP sub sample mode, 0: disable, 1: 2 to 1, 2: 4 to 1, 3: 8 to 1, 4: 16 to 1
	unsigned int dwSubIrMode;  //! IFP sub ir mode, 0: disable, 1: 1 to 1, 2: 2 to 1, 3: 4 to 1, 4: 8 to 1
	unsigned int dwMdGridSize; //! IFP motion detection grid size, 0:16x16, 1:32x32
	unsigned int dwLscHorGridSize; //! IFP lens shading correction horizontal grid size, 0:8x8, 1:16x16
	unsigned int dwLscVerGridSize; //! IFP lens shading correction vertical grid size, 0:8x8, 1:16x16
	unsigned int dwStatGridHorNum; //! IFP statistic window horzontal grid number
	unsigned int dwStatGridVerNum; //! IFP statistic window vertical grid number

	VMF_VSRC_FEC_INIT_CONFIG_T tFecInitConfig; //! fec initial config
} VMF_VSRC_FRONTEND_CONFIG_T;

/**
 * A structure for Video 360 mode initial config
 */
typedef struct
{
	unsigned int dwBlendingWidth; //! The blending width for V360 mode
	unsigned int dwSkipBlendingWidth; //! The skip blending width for V360 mode on CPU blending
	unsigned int dwBlendingMode;  //! Weighted blending mode, 0: no blending, 1: blend by DMA, 2: blend by CPU.
	unsigned int bMultiSsm;       //! The flag to control using multiSsm on pipeline flow or single SSM on straight flow
	unsigned int bDuplexMode;     //! 0: ISP duplex mode off, 1:ISP duplex mode on for V360 resize
 	unsigned int bDualCamSync;    //! 0: autoscene sync off, 1: autoscene sync on
} VMF_VSRC_V360_INIT_CONFIG_T;

/**
 * A structure for determine further usage of VSRC stream
 */
typedef struct
{
	unsigned int bEnableSpec;  //! Determine if spec config enable, 0: disable, 1: enable
	VMF_ISP_MODE dwIspMode;    //! Determine ISP mode

	VMF_ENC_SPEC_T tIfpEncSpec;//! Determine encoding spec of IFP output stream
	VMF_ENC_SPEC_T tIspEncSpec;//! Determine encoding spec of ISP output stream
} VMF_VSRC_SPEC_CONFIG_T;

/**
 * A structure for VMF video source initial config
 */
typedef struct
{
	VMF_VSRC_APP_MODE eAppMode; //! The application mode
	unsigned int dwFrontConfigCount;           //! The number of front config
	VMF_VSRC_FRONTEND_CONFIG_T* ptFrontConfig; //! The pointer to front config array
	VMF_LAYOUT_T tMainLayout;   //! Initial layout config of primary output
	VMF_VSRC_INIT_FUNC fnInitCallback; //! The callback funcion after initialize sucessfully or failed
	VMF_VSRC_VI_SIGNAL_FUNC fnViSignalCallback; //! The callback funcion to feedback VI signal
	VMF_VSRC_VI_RAWDATA_FUNC fnViRawDataCallback; //! The callback funcion to dump VI raw data buffer
	VMF_VSRC_V360_INIT_CONFIG_T tV360InitConfig; //! V360 initial extra config
	VMF_VSRC_SPEC_CONFIG_T tSpecConfig; //! VSRC spec config

	unsigned int bShared;			//! Specify VSRC output buffer can be shared between processes or not
	const char* pszOutPinPrefix;	//! SyncShareMemory writer prefix which length should not exceed VMF_MAX_SSM_NAME_PREFIX
	const char* pszAutoSceneConfig;	//! Autoscene config file path
	const char* pszResourceDir;		//! The path to resource directory
	unsigned int dwReducingSSMUsage;	//! 0: Auto allocate SSM buffers
										//! 1: Restrict IFPE SSM buffer count to 1 and ISPE SSM buffer count to 2
										//! 2: Customized IFPE SSM buffer count and ISPE SSM buffer count refer to dwIfpeBufCount and dwIspeBufCount
	unsigned int bOsdInIFP;      //! Show text overlay on IFPE output
	unsigned int dwIfpeBufCount; //! IFPE output buffer count. This value is valid while dwReducingSSMUsage is 2
								 //! 0: Auto allocate SSM buffers, 1~: IFPE buffer count
	unsigned int dwIspeBufCount; //! ISPE output buffer count. This value is valid while dwReducingSSMUsage is 2
								 //! 0: Auto allocate SSM buffers, 2~: ISPE buffer count
	VMF_FEC_DUAL_OUTPUT_TYPE eDualOutputType;
} VMF_VSRC_INITOPT_T;

/**
 *  An enumeration for ISP config layer
 */
typedef enum
{
	VMF_VSRC_CONFIG_ISP_DEFAULT = 0,  //! config second layer, if master object not exist, config 1-st layer
	VMF_VSRC_CONFIG_ISP_ALL_LAYER,    //! config all the exising ISP
	VMF_VSRC_CONFIG_ISP_FIRST_LAYER,  //! always config 1-st layer, if master object not exist, do nothing
	VMF_VSRC_CONFIG_ISP_SECOND_LAYER, //! always config second layer, if master object not exist, do nothing
	VMF_VSRC_GET_ISP_OPTION			  //! get the isp option value
} VMF_VSRC_CONFIG_ISP_LAYER;

/**
 * A structure for FEC detail output config within VMF_VSRC_OUTPUT_CONFIG_T
 */
typedef struct
{
	VMF_FEC_INFO_T  *ptFecInfo; //! The pointer to VMF_FEC_INFO_T
	VMF_FEC_COEF_MODE eFecMode; //! Fec transformation mode

	unsigned int dwOffsetX;  //! Output horizontal offset in output stream
	unsigned int dwOffsetY;  //! Output vertical offset in output stream
} VMF_VSRC_OUTPUT_T;

/**
 * An struct
 * VMF video source FEC output config for multi-dewarp effect
 */
typedef struct
{
	VMF_LAYOUT_T tLayout;        //! layout of VSRC stream output
	unsigned int dwBackColor;    //! background color for VSRCbuffer output
	VMF_FEC_METHOD eFecMethod;   //! 0: Fec Gtr mode, 1: Fec coeffgen mode
	unsigned int dwOutputLength; //! the number of ptOutput
	VMF_VSRC_OUTPUT_T* ptOutput; //! the array of VMF_VSRC_OUTPUT_T
	VMF_ENC_SPEC_T tIspEncSpec;  //! Determine encoding spec of ISP output stream
	unsigned int bDuplexMode;    //! 0: ISP duplex mode off, 1:ISP duplex mode on
} VMF_VSRC_OUTPUT_CONFIG_T;

/**
 * An struct
 * VMF video source resolution config
 */
typedef struct
{
	unsigned int dwInWidth;   //! videocap input width
	unsigned int dwInHeight;  //! videocap input height
	unsigned int dwCapWidth;  //! videocap output width
	unsigned int dwCapHeight; //! videocap output height
} VMF_VSRC_RES_CONFIG_T;

/**
 * @brief Function to initialize VMF video source.
 *
 * @param[in] ptInitOpt A pointer to the VMF_VSRC_INITOPT_T structure.
 * @return The handle of VMF video source.
 */
VMF_VSRC_HANDLE_T* VMF_VSRC_Init(const VMF_VSRC_INITOPT_T* ptInitOpt);

/**
 * @brief Function to re-initialize VMF video source.
 *
 * @param[in] ptHandle The handle of VMF video source.
 * @param[in] ptInitOpt A pointer to the VMF_VSRC_INITOPT_T structure.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_ReInit(VMF_VSRC_HANDLE_T* ptHandle, const VMF_VSRC_INITOPT_T* ptInitOpt);

/**
 * @brief Function to release VMF video source.
 *
 * @param[in] ptHandle The handle of VMF video source.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_Release(VMF_VSRC_HANDLE_T* ptHandle);

/**
 * @brief Function to start VMF video source.
 *
 * @param[in] ptHandle The handle of VMF video source.
 * @param[in] ptConfig A pointer to the VMF_VSRC_CONFIG_T structure.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_Start(VMF_VSRC_HANDLE_T* ptHandle, const VMF_VSRC_CONFIG_T* ptConfig);

/**
 * @brief Function to stop VMF video source.
 *
 * @param[in] ptHandle The handle of VMF video source.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_Stop(VMF_VSRC_HANDLE_T* ptHandle);

/**
 * @brief Function to resume VMF video source.
 *
 * @param[in] ptHandle The handle of VMF video source.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_Resume(VMF_VSRC_HANDLE_T* ptHandle);

/**
 * @brief Function to suspend VMF video source.
 *
 * @param[in] ptHandle The handle of VMF video source.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_Suspend(VMF_VSRC_HANDLE_T* ptHandle);

/**
 * @brief Function to configure video capture device.
 *
 * @param[in] ptHandle The handle of VMF video source.
 * @param[in] ptOptions An array of configurations.
 * @param[in] dwOptionNum The size of 'ptOption' array.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_SetVIOptions(VMF_VSRC_HANDLE_T* ptHandle, VMF_VI_OPTION_T* ptOptions, unsigned int dwOptionNum);

/**
 * @brief Function to get the current configuration of video capture device.
 *
 * @param[in] ptHandle The handle of VMF video source.
 * @param[out] ptOptions A pointer of VMF_VI_OPTION_T structure.
 * @param[in] dwOptionNum The size of 'ptOptions' array.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_GetVIOptions(VMF_VSRC_HANDLE_T* ptHandle, VMF_VI_OPTION_T* ptOptions, unsigned int dwOptionNum);

/**
 * @brief Function to configure image frontend processing device.
 *
 * @param[in] ptHandle The handle of VMF video source.
 * @param[in] ptOptions A pointer of VMF_IFP_OPTION_T structure.
 * @param[in] dwOptionNum The size of 'ptOptions' array.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_SetIFPOptions(VMF_VSRC_HANDLE_T* ptHandle, VMF_IFP_OPTION_T* ptOptions, unsigned int dwOptionNum);

/**
 * @brief Function to get the current configuration of image frontend processing device.
 *
 * @param[in] ptHandle The handle of VMF video source.
 * @param[out] ptOptions A pointer of VMF_IFP_OPTION_T structure.
 * @param[in] dwOptionNum The size of 'ptOptions' array.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_GetIFPOptions(VMF_VSRC_HANDLE_T* ptHandle, VMF_IFP_OPTION_T* ptOptions, unsigned int dwOptionNum);

/**
 * @brief Function to get the VSRC information.
 *
 * @param[in] ptHandle The handle of VMF video source.
 * @param[in] ptInfo The pointer VMF_VSRC_QUERY_INFO_T.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_GetInfo(VMF_VSRC_HANDLE_T* ptHandle, VMF_VSRC_QUERY_INFO_T* ptInfo);

/**
 * @brief Function to set the FEC offset information.
 *
 * @param[in] ptHandle The handle of VMF video source.
 * @param[in] dwIndex  The index of VSRC front end stream.
 * @param[in] iOffsetX The value of fec center horizontal offset.
 * @param[in] iOffsetY The value of fec center vertical offset.
 * @param[in] iOffsetR The value of fec radius offset.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_SetFecOffset(VMF_VSRC_HANDLE_T* ptHandle, unsigned int dwIndex, int iOffsetX, int iOffsetY, int iOffsetR);

/**
 * @brief Function to config ISP engine in VSRC.
 *
 * @param[in] ptHandle The handle of VMF video source.
 * @param[in] dwIndex The index of VSRC stream.
 * @param[in] dwLayer The layer of ISP config.
 * @param[in] iIspIndex The index of ISP handle in stream, -1 for all ISP handle in stream.
 * @param[in] ptOption The pointer of VMF_ISP_OPTION_T.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_ConfigISP(VMF_VSRC_HANDLE_T *ptHandle, unsigned int dwIndex, unsigned int dwLayer,
	int iIspIndex, const VMF_ISP_OPTION_T *ptOption);

/**
 * @brief Function to config FEC multi-dewarping effect in VSRC.
 *
 * @param[in] ptHandle The handle of VMF video source.
 * @param[in] dwIndex The index of VSRC stream.
 * @param[in] ptOutput The pointer of VMF_VSRC_OUTPUT_CONFIG_T.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_SetOutput(VMF_VSRC_HANDLE_T *ptHandle, unsigned int dwIndex, const VMF_VSRC_OUTPUT_CONFIG_T *ptOutput);

/**
 * @brief Function to update FEC config in VSRC.
 *
 * @param[in] ptHandle The handle of VMF video source.
 * @param[in] dwIndex The index of VSRC stream.
 * @param[in] dwCount The fec update area count.
 * @param[in] apdwIspIndex A pointer to the specific index of ISP engine array.
 * @param[in] aptFecInfo A pointer to the fec info array.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_UpdateFec(VMF_VSRC_HANDLE_T *ptHandle, unsigned int dwIndex, unsigned int dwCount,
	unsigned int* apdwIspIndex, VMF_FEC_INFO_T* aptFecInfo);

/**
 * @brief Function to configure font of text overlay in VSRC ISP.
 *
 * @param[in] ptHandle The handle of VMF video source.
 * @param[in] dwStreamIdx The index of VSRC stream.
 * @param[in] dwSubIdx 0: main stream, 1~4: resize stream index.
 * @param[in] ptFontInfo The pointer of FONT_INFO_T.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_SetFont(VMF_VSRC_HANDLE_T* ptHandle, unsigned int dwStreamIdx, unsigned int dwSubIdx, const FONT_INFO_T* ptFontInfo);

/**
 * @brief Function to configure font of text overlay in VSRC IFP.
 *
 * @param[in] ptHandle The handle of VMF video source.
 * @param[in] ptFontInfo The pointer of FONT_INFO_T.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_SetFont_IFP(VMF_VSRC_HANDLE_T* ptHandle, const FONT_INFO_T* ptFontInfo);

/**
 * @brief Function to configure text overlay of VSRC ISP.
 *
 * @param[in] ptHandle The handle of VMF video source.
 * @param[in] dwStreamIdx The index of VSRC stream.
 * @param[in] dwSubIdx 0: main stream, 1~4: resize stream index.
 * @param[in] ptConfig The pointer of VMF_OVERLAY_CONFIG_T.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_ConfigOverlay(VMF_VSRC_HANDLE_T* ptHandle, unsigned int dwStreamIdx, unsigned int dwSubIdx, const VMF_OVERLAY_CONFIG_T* ptConfig);

/**
 * @brief Function to configure text overlay of VSRC IFP.
 *
 * @param[in] ptHandle The handle of VMF video source.
 * @param[in] ptConfig The pointer of VMF_OVERLAY_CONFIG_T.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_ConfigOverlay_IFP(VMF_VSRC_HANDLE_T* ptHandle, const VMF_OVERLAY_CONFIG_T* ptConfig);

/**
 * @brief Function to configure autoscene task options, includes AE and AWB options.
 *
 * @param[in] ptHandle The handle of VMF video source.
 * @param[in] ptOptions The pointer of VMF_ASC_TSK_OPTION_T.
 * @param[in] dwOptionNum The number of options.
 * @param[in] bSync 0: asynchronous mode, 1. synchronous mode.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_ASC_SetOptions(VMF_VSRC_HANDLE_T* ptHandle, VMF_ASC_TSK_OPTION_T* ptOptions, unsigned int dwOptionNum, unsigned int bSync);

/**
 * @brief Function to get autoscene task options, includes AE and AWB options.
 *
 * @param[in] ptHandle The handle of VMF video source.
 * @param[in] ptOptions The pointer of VMF_ASC_TSK_OPTION_T.
 * @param[in] dwOptionNum The number of options.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_ASC_GetOptions(VMF_VSRC_HANDLE_T* ptHandle, VMF_ASC_TSK_OPTION_T* ptOptions, unsigned int dwOptionNum);

/**
 * @brief Function to get autoexposure options.
 *
 * @param[in] ptHandle The handle of autotask handle.
 * @param[in] ptOptions The autoexposure option
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_AE_GetOptions(VMF_VSRC_HANDLE_T* ptHandle, VMF_AE_OPTION_T* ptOptions);

/**
 * @brief Function to configure IVS options.
 *
 * @param[in] ptHandle The handle of VMF video source.
 * @param[in] ptConfig The pointer of VMF_IVS_CONFIG_T.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_IVS_Config(VMF_VSRC_HANDLE_T* ptHandle, VMF_IVS_CONFIG_T* ptConfig);

/**
 * @brief Function to reinit video capture.
 *
 * @param[in] pptHandle The handle of VMF video source.
 * @param[in] ptInitOpt A pointer to the VMF_VSRC_INITOPT_T structure.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_ReInitVI(VMF_VSRC_HANDLE_T** pptHandle, const VMF_VSRC_INITOPT_T* ptInitOpt);


/**
 * @brief Function to set Resoultion (Binning mode).
 *
 * @param[in] ptSrcHandle The handle of VMF video source.
 * @param[in] ptResConfig A pointer to VMF_VSRC_RES_CONFIG_T structure 
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_RES_Config(VMF_VSRC_HANDLE_T* ptSrcHandle, VMF_VSRC_RES_CONFIG_T *ptResConfig);

/**
 * @brief Function to calibrate the center of circle.
 *
 * @param[in] ptHandle The handle of VMF video source.
 * @param[in] ptInitOpt A pointer to the VMF_FEC_CALIBRATE_OUTPUT_T structure.
 * @param[in] bApply 0: Do not apply to video source, 1: Apply to video source.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_Calibrate(VMF_VSRC_HANDLE_T* ptHandle, VMF_FEC_CALIBRATE_OUTPUT_T* ptOutput, unsigned int bApply);

#ifdef __cplusplus
}
#endif

#endif
