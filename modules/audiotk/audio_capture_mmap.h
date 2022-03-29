
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
#ifndef AUDIO_CAPTURE_MMAP_H
#define AUDIO_CAPTURE_MMAP_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <alsa/asoundlib.h>
#pragma GCC diagnostic pop
#include <sys/time.h>
#include <audiotk/audio_common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ATK_AUDIOCAP_HANDLE_T ATK_AUDIOCAP_HANDLE_T;

typedef struct
{
	int bIsInterleaved; /**< Interleaved mode. */
	unsigned int dwChannels; /**< Channels. */

	unsigned char *ppbyAudioBufs[ATK_AUDIO_MAX_CHANNELS]; /**< Buffer pointers to point audio data. */
	size_t dwDataBytes; /**< The number of bytes of data. */
	size_t dwOneFrameBytes; /**< The number of bytes of one frame. */
	struct timeval tDataTimestamp; /**< The timestamp of data frame. */
} ATK_AUDIO_NOTIFY_DATA_INFO_T;

typedef void (*ATK_AUDIO_CAP_DATA_FUNC)(const ATK_AUDIO_NOTIFY_DATA_INFO_T*, void*);

typedef struct
{
	const char *szPcmName; /**< The PCM name for audio device. */
	int bUseSimpleConfig; /**< If it is non-zero, we just use pcm_name, is_interleaved, format, channels_count, sample_rate and first callback to configure. */
	int bIsInterleaved; /**< Interleaved mode. */
	snd_pcm_format_t eFormat; /**< Sample format. */
	unsigned int dwChannelsCount; /**< Channels. */
	unsigned int dwSampleRate; /**< Sample rate. */
	unsigned int dwPeriodsPerBuffer; /**< Periods per buffer. */
	unsigned long dwPeriodSizeInFrames; /**< Distance between interrupts is # frames. */

	ATK_AUDIO_CAP_DATA_FUNC pfnCallback; /**< Callback function for receiving audio data. */
	void *pUserData; /**< Use private data. This data will be passed to callback function. */
} ATK_AUDIOCAP_CONFIG_T;


/**
 * @brief Function to initialize audio capture device.
 *
 * @param[in] ptConfig The audio capture device's configuration.
 * @return The handle of audio capture device (tk). If it is NULL, it fails.
 */
ATK_AUDIOCAP_HANDLE_T* ATK_AudioCap_Init(ATK_AUDIOCAP_CONFIG_T* ptConfig);

/**
 * @brief Function to release audio capture device
 *
 * @param[in] ptHandle The handle of audio capture device (tk).
 */
void ATK_AudioCap_Release(ATK_AUDIOCAP_HANDLE_T *ptHandle);

#ifdef __cplusplus
}
#endif

#endif
