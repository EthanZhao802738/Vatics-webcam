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

#ifndef SSM_INFO_H
#define SSM_INFO_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A structure for VSRC SSM output information.
 */
typedef struct
{
	//! Y stride of video frame
	unsigned int dwYStride;

	//! Y size of video frame
	unsigned int dwYSize;

	//! UV size of video frame
	unsigned int dwUVSize;

	//! raw data offsets for Y / U / V
	unsigned int dwOffset[3];

	//! YUV width
	unsigned int dwWidth;

	//! YUV height
	unsigned int dwHeight;

	//! YUV type
	unsigned int dwType;

} VMF_VSRC_SSM_OUTPUT_INFO_T;


typedef struct
{
	//! RGB statistics map
	int iRgbSumMapOffset0;
	int iRgbAnsMapOffset0;
	int iRgbSumMapOffset1;
	int iRgbAnsMapOffset1;

	//! Histogram statistics map
	int iHistMapOffset0;
	int iHistMapOffset1;
	int iSatHisMapOffset;
	int iYHistMapOffset;

	//! Focus value map
	int iFocusMapOffset0;
	int iFocusMapOffset1;

	//! luma channel using luma high pass filter 0
	unsigned int dwGlobalFv0Luma0;
	//! luma channel using luma high pass filter 1
	unsigned int dwGlobalFv0Luma1;

	//! RGB & Focus Map Grid
	unsigned int dwStatGridHorNum;
	unsigned int dwStatGridVerNum;

} VMF_STATS_MAP_INFO_T;

/*
 * Only available while using GTR.
 */
typedef struct
{
	unsigned int dwCplxMapSize;
	unsigned int dwCplxMapStride;
	//! Complex map offset in ssm
	unsigned int dwCplxOffset;
	unsigned int dwCplxVirAddr;

	unsigned int dwMrfMapSize;
	unsigned int dwMrfMapStride;
	//! MRF map offset in ssm
	unsigned int dwMrfOffset; 
	unsigned int dwMrfVirAddr;
} VMF_ISP_MAP_INFO_T;

/**
 * @brief Function to set raw YUV video frame information from SSM header.
 *
 * @param[in] pbySsmBuff The SSM buffer.
 * @param[in] ptVsrcSsmInfo A VMF_VSRC_SSM_OUTPUT_INFO_T structure to set SSM header.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_SSM_SetInfo(unsigned char* pbySsmBuff, VMF_VSRC_SSM_OUTPUT_INFO_T* ptVsrcSsmInfo);


/**
 * @brief Function to fetch raw YUV video frame information from SSM header.
 *
 * @param[in] pbySsmBuff The SSM buffer.
 * @param[out] ptVsrcSsmInfo A VMF_VSRC_SSM_OUTPUT_INFO_T structure to get SSM header.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_SSM_GetInfo(unsigned char* pbySsmBuff, VMF_VSRC_SSM_OUTPUT_INFO_T* ptVsrcSsmInfo);

/**
 * @brief Function to fetch raw YUV video frame statistic information from SSM header.
 *
 * @param[in] pbySsmBuff The SSM buffer.
 * @param[out] ptStatInfo A VMF_STATS_MAP_INFO_T structure to get statistic information.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_SSM_GetStatInfo(unsigned char* pbySsmBuff, VMF_STATS_MAP_INFO_T* ptStatInfo);
/**
 * @brief Function to fetch raw YUV video frame map information from SSM header.
 *
 * @param[in] pbySsmBuff The SSM buffer.
 * @param[out] ptIspMapInfo A VMF_ISP_MAP_INFO_T structure to get map information.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VSRC_SSM_GetCplxMrfMapInfo(unsigned char* pbySsmBuff, VMF_ISP_MAP_INFO_T* ptIspMapInfo);

#ifdef __cplusplus
}
#endif

#endif //! guard
