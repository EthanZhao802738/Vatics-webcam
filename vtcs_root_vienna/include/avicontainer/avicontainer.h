
/*
 *******************************************************************************
 * $Header: $
 *
 *  Copyright (c) 2007-2014 Vatics Inc. All rights reserved.
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
 * $History: $
 *
 *******************************************************************************
 */

#ifndef _AVICONTAINER_H_
#define _AVICONTAINER_H_

/*
 The data format of config info
 
	Video codecs (H264, JPEG)
		FOURCC_type (FOURCC_H264 or FOURCC_JPEG, 4 bytes)
		width (4 bytes)
		height (4 bytes)

	G711:
		FOURCC_G711 (4 bytes)
		compression_format (FOURCC_ULAW or FOURCC_ALAW, 4bytes)

	G726:
		FOURCC_G726 (4 bytes)
		dwCodewordBits (bitrate/sample rate, 4 bytes)

	GAMR:
		FOURCC_GAMR (4 bytes)

	AAC4
		FOURCC_AAC4 (4 bytes)
		sameple rate (4 bytes)
		channel num (4 bytes)
*/

#define AVIC_PADDING_SIZE 2
#define AVIC_KEYFRAME 0x00000010
#define AVIC_NONE 0x00000000

typedef struct
{
	char            *szAVIFile; /**< file name */
	unsigned int dwVideoTrackNum; /**< The total number of track */
	unsigned char* ptVideoTrackBufInfo[2]; /**< The video config info array */
	unsigned int dwAudioTrackNum; /**< total audio number of track */
	unsigned char* ptAudioTrackBufInfo[2]; /**< The audio config info array */
} AVICCreateOptions;

typedef struct AVICHandle AVICHandle;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Function to initialize AVI container. 
 *
 * @return The handle of AVI container
 */
AVICHandle* AVIC_Init();

/**
 * @brief Function to release AVI container.
 *
 * @param[in] handle The handle of AVI container.
 */
void AVIC_Release(AVICHandle* handle);

/**
 * @brief Function to create an AVI file.
 *
 * @param[in] handle The handle of AVI container.
 * @param[in] option The AVI container's configuration.
 * @return Success: 0  Fail: negative integer.
 */
int AVIC_CreateFile(AVICHandle* handle, const AVICCreateOptions* option);

typedef struct
{
	unsigned int dwFlags;
	unsigned char* data;
} avic_sample_info_t;
/**
 * @brief Add a sample into an AVI file.
 *
 * @param[in] handle The handle of AVI container.
 * @param[in] dwTrackID The AVI track id for written data.
 * @param[in] pbyRawData The pointer to a sample.
 * @param[in] dwSampleSz the size of a sample.
 * @return Success: 0  Fail: negative integer.
 */
int AVIC_AddSample(AVICHandle* handle, unsigned int dwTrackID, const avic_sample_info_t* sample);

/**
 * @brief Function to cloase an AVI file.
 *
 * @param[in] handle The handle of AVI container.
 * @param[in] video_duration_ms The video data's duration (in msec).
 * @return Success: 0  Fail: negative integer.
 */
int AVIC_CloseFile(AVICHandle* handle, unsigned int video_duration_ms);

/**
 * @brief Start/Notify to sync data into disk (Non-Blocking).
 *
 * @param[in] handle The handle of AVI container.
 * @return Success: 0  Fail: negative integer.
 */
int AVIC_CommitData(AVICHandle* handle);

/**
 * @brief Flush I/O cache and sync data into disk.
 *
 * @param[in] handle The handle of AVI container.
 * @return Success: 0  Fail: negative integer.
 */
int AVIC_FlushCache(AVICHandle* handle);

#ifdef AVIv2
/**
 * @brief Avi 2.0, Update SuperIndx and standard index
 *
 * @param[in] handle The handle of AVI container.
 * @return Success: 0  Fail: negative integer.
 */
int AVIC_UpdateFile(AVICHandle* handle, int duration_ms);
#endif

#ifdef __cplusplus
}
#endif

#endif // _AVICONTAINER_H_
