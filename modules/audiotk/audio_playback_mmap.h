
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
#ifndef AUDIO_PLAYBACK_MMAP_H
#define AUDIO_PLAYBACK_MMAP_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <alsa/asoundlib.h>
#pragma GCC diagnostic pop
#include <audiotk/audio_common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ATK_AUDIOPLAY_HANDLE_T ATK_AUDIOPLAY_HANDLE_T;

typedef struct
{
	const char *szPcmName; /**< The PCM name for audio device. */
	int bUseSimpleConfig; /**< If it is non-zero, we just use pcm_name, is_interleaved, format, channels_count, sample_rate to configure. */
	int bIsInterleaved; /**< Interleaved mode. */
	snd_pcm_format_t eFormat; /**< Sample format. */
	unsigned int dwChannelsCount; /**< Channels. */
	unsigned int dwSampleRate; /**< Sample rate. */
	unsigned int dwPeriodsPerBuffer; /**< Periods per buffer. */
	unsigned long dwPeriodSizeInFrames; /**< Distance between interrupts is # frames. */
	unsigned int bDropFramesBeforeStop; /**0: Stop a PCM preserving pending frames. non-zero: Stop a PCM dropping pending frames.*/
} ATK_AUDIOPLAY_CONFIG_T;

/**
 * @brief Function to initialize audio playback device.
 *
 * @param[in] ptConfig The audio playback device's configuration.
 * @return The handle of audio playback device (tk). If it is NULL, it fails.
 */
ATK_AUDIOPLAY_HANDLE_T* ATK_AudioPlay_Init(ATK_AUDIOPLAY_CONFIG_T* ptConfig);

/**
 * @brief Function to release audio playback device.
 *
 * @param[in] ptHandle The handle of audio playback device (tk).
 */
void ATK_AudioPlay_Release(ATK_AUDIOPLAY_HANDLE_T *ptHandle);

/**
 * @brief Function to get the total bytes of data in one period.
 *
 * @param[in] ptHandle The handle of audio playback device (tk).
 * @return The total bytes of data in one period if it is interleaved mode.
 * Otherwise, the total bytes of data of one channel in one period if it is non-interleaved mode.
 */
size_t ATK_AudioPlay_GetPeriodFramesSize(ATK_AUDIOPLAY_HANDLE_T *ptHandle);

/**
 * @brief Function to get the total bytes of data in one frame (sample).
 *
 * @param[in] ptHandle The handle of audio playback device (tk).
 * @return The total bytes of data in one frame if it is interleaved mode.
 * Otherwise, the total bytes of data of one channel in one sample if it is non-interleaved mode.
 */
size_t ATK_AudioPlay_GetOneFrameSize(ATK_AUDIOPLAY_HANDLE_T *ptHandle);

/**
 * @brief Function to play audio frames (one period) in the current mmap buffer and get next mmap buffer.
 * Don't use this function with @see ATK_AudioPlay_PlayFrames.
 *
 * @param[in] ptHandle The handle of audio playback device (tk).
 * @param[in, out] bufs The data those need to be played. If it is interleaved mode,
 * all data are in the buf[0]. If it is non-interleaved mode, all data of each
 * channel are in the buf[channel_index]. If we call this function first time, the bufs[*] should be zero.
 *
 * @return -1: Failed. 0: Success.
 */
int ATK_AudioPlay_PlayPeriodFrames(ATK_AUDIOPLAY_HANDLE_T *ptHandle, unsigned char **ppbyBufs);

/**
 * @brief Function to play audio frames in the current mmap buffer and get next mmap buffer.
 * Don't use this function with @see ATK_AudioPlay_PlayPeriodFrames.
 *
 * @param[in] ptHandle The handle of audio playback device (tk).
 * @param[in, out] ppbyBufs The data those need to be played. If it is interleaved mode,
 * all data are in the buf[0]. If it is non-interleaved mode, all data of each
 * channel are in the buf[channel_index]. If we call this function first time, the bufs[*] should be zero.
 * @param[out] pdwFrames mmap area portion size in frames. This is the number of frames you must write to the mmap buffer.
 *
 * @return -1: Failed. 0: Success.
 */
int ATK_AudioPlay_PlayFrames(ATK_AUDIOPLAY_HANDLE_T *ptHandle, unsigned char **ppbyBufs, unsigned int *pdwFrames);

/**
 * @brief Function to set silence data in the buffer within one period size.
 *
 * @param[in] ptHandle The handle of audio playback device (tk).
 * @param[in] ppbyBufs The buffers those need to be set. If it is interleaved mode,
 * all data are in the buf[0]. If it is non-interleaved mode, all data of each
 * channel are in the buf[channel_index].
 * @param[in] dwOffset The offset for the start point which need to be set silence (bytes).
 * @param[in] dwBytes The size for the portion of the buffer which need to be set silence.
 * @param[in] iChannel This parameter is used for non-interleaved mode to specify which
 * channel we want to set. If it is invalid, the data of all channels will be set.
 *
 * @return -1: Failed. 0: Success.
 */
int ATK_AudioPlay_SetSilenceWithinPeriod(ATK_AUDIOPLAY_HANDLE_T *ptHandle, unsigned char **ppbyBufs, size_t dwOffset, size_t dwBytes, int iChannel);

#ifdef __cplusplus
}
#endif

#endif
