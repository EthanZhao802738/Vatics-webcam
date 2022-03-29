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
#ifndef AUDIO_AAC4ENC_H
#define AUDIO_AAC4ENC_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vmf_aac4_handle_t VMF_AAC4ENC_HANDLE_T;

typedef struct VMF_AAC4ENC_INITOPT_T {
	//! A data for Bit rate
	unsigned int dwBitRate; 
	//! A data for Sample rate
	unsigned int dwSampleRate;
	//! A data for Audio Data Transport Stream. 0: Raw, 1: ADTS 2: ADIF
	unsigned int dwADTS;
	//! A data for Stereo Mode. 0: stereo 1: ADTS 3: mono 
	unsigned int dwStereoMode; 
	//! A data for number of channels. (shoud be 1~2)
	unsigned int dwChannel;
} VMF_AAC4ENC_INITOPT_T;

typedef struct
{
	//! A data for input data buffer
	unsigned char *pbyInBuf; 
	//! A data for output encoded data buffer
	unsigned char *pbyOutBuf;  
	//! A data for the size of output data buffer
	size_t dwOutBufSize;  
} VMF_AAC4ENC_ONEFRAME_CONF_T;

/**
 * @brief Function to initialize aac4encoder handle.
 *
 * @param[in] ptInitOpt The AAC4Encode's initializing option.
 * @return The handle of aac4encoder.
 */
VMF_AAC4ENC_HANDLE_T* VMF_AAC4ENC_Init(const VMF_AAC4ENC_INITOPT_T* ptInitOpt);

/**
 * @brief Function to release aac4encoder handle.
 *
 * @param[in] ptHandle The pointer to aac4enc handle.
 * @return Success: 0  Fail: not 0.
 */
int VMF_AAC4ENC_Release(void** pthandle);

/**
 * @brief Function to set aac4encoder option.
 *
 * @param[in] pthandle The handle of aac4encoder.
 * @param[in] ptOption The option of VMF_AAC4ENC_ONEFRAME_CONF_T structure.
 * @return Success: 0  Fail: negative number.
 */
int VMF_AAC4ENC_SetOptions(VMF_AAC4ENC_HANDLE_T* pthandle, VMF_AAC4ENC_ONEFRAME_CONF_T* ptOption);

/**
 * @brief Function to process aac4encoder one frame.
 *
 * @param[in] pthandle The handle of aac4encoder.
 * @return Success: 0  Fail: negative number.
 */
int VMF_AAC4ENC_PorcessOneFrame(VMF_AAC4ENC_HANDLE_T* pthandle);

/**
 * @brief Function to get aac4encoder config.
 *
 * @param[in] pthandle The handle of aac4encoder.
 * @param[in] pszbuf The config buffer of aac4encoder.
 * @param[in] pdwSpecConfSize The config size of aac4encoder.
 * @param[in] pdwProfileLevel The profile level of aac4encoder.
 * @return Success: 0  Fail: negative number.
 */
int VMF_AAC4ENC_GetConf(VMF_AAC4ENC_HANDLE_T* ptHandle, unsigned char* pszbuf, unsigned int* pdwSpecConfSize, unsigned int* pdwProfileLevel);

/**
 * @brief Function to get aac4encoder config
 *
 * @param[in] pthandle The handle of aac4encoder.
 * @return Success: 0  Fail: negative number.
 */
int VMF_AAC4ENC_GetBitStreamSize(VMF_AAC4ENC_HANDLE_T* ptHandle);

#ifdef __cplusplus
}
#endif

#endif 