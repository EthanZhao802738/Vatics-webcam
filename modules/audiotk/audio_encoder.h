
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
#ifndef AUDIO_ENCODER_H
#define AUDIO_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <audiotk/audio_common.h>

typedef struct ATK_AUDIOENC_HANDLE_T ATK_AUDIOENC_HANDLE_T;

typedef struct
{
	int iCompressionMode; /**< It can be 0: ulaw or 1: alaw. */ /*G711ENC_MODE_U_LAW, G711ENC_MODE_A_LAW*/
} ATK_G711ENC_CONFIG_T;

typedef struct
{
	unsigned int dwBitRate; /**< Bit rate. */
	unsigned int dwSampleRate; /**< Sample rate. */
	unsigned int dwAdts; /**< It can be 0: Raw, 1: ADTS or 2: ADIF */
	unsigned int dwStereoMode; /**< It can be 0: stereo, 1: joint stereo or 3: mono to control the desired output channel of AAC */
} ATK_AAC4ENC_CONFIG_T;

typedef struct
{
	unsigned int dwBitRate; /**< Bit rate. */
} ATK_G726ENC_CONFIG_T;

typedef struct
{
	unsigned int dwBitRate; /**< Bit rate. */
	unsigned int dwSampleRate; /**< Sample rate. */
	unsigned int dwMode; /**< Sample rate mode. */
} ATK_GAMRENC_CONFIG_T;

typedef struct
{
	ATK_AUDIO_ENCODER_TYPE eType; /**< The type of audio encoder. */
	unsigned int dwChannels; /**< The number of channels. */
	unsigned int bIsInterleaved; /**< The input data is interleaved or not. */
	unsigned int dwPeriodSizeInFrames; /**< Distance between interrupts is # frames. */
} ATK_AUDIOENC_INITOPT_T;

typedef struct
{
	unsigned char *pbyInBuf;  /**< The input data buffer. */
	unsigned char *pbyOutBuf;  /**< The output encoded data buffer. */
	size_t dwOutBufSize;  /**< The size of output data buffer.*/
} ATK_AUDIOENC_ONEFRAME_CONF_T;

/**
 * @brief Function to initialize audio encoder.
 *
 * @param[in] init_opt Initial options for audio encoder.
 * @param[in] config Initial configuration for audio encoder.
 * @return The handle of audio encoder (tk). If it is NULL, it fails.
 */
ATK_AUDIOENC_HANDLE_T* ATK_AudioEnc_Init(const ATK_AUDIOENC_INITOPT_T *ptInitOpt, const void *pConfig);

/**
 * @brief Function to release audio encoder.
 *
 * @param[in] handle The handle of audio encoder (tk).
 * @return Success: 0  Fail: negative integer.
 */
int ATK_AudioEnc_Release(ATK_AUDIOENC_HANDLE_T *ptHandle);

/**
 * @brief Function to encode one audio frame.
 *
 * @param[in] handle The handle of audio encoder (tk).
 * @param[in] config The Configuration for encode one frame.
 * @return negative: failed, otherwise: The size of encoded data (bytes).
 */
int ATK_AudioEnc_EncodeOneFrame(ATK_AUDIOENC_HANDLE_T *ptHandle, ATK_AUDIOENC_ONEFRAME_CONF_T* ptConf);

/**
 * @brief Function to get information about the encoder.
 *
 * @param[in] handle The handle of audio encoder (tk).
 * @param[out] conf_buf The buffer which to store the information.
 * @return negative: failed, otherwise: success.
 */
int ATK_AudioEnc_GetConf(ATK_AUDIOENC_HANDLE_T *ptHandle, void *pConfBuf);

#ifdef __cplusplus
}
#endif

#endif
