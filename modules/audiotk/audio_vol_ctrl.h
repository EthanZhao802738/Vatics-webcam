
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
#ifndef AUDIO_VOL_CTRL_H
#define AUDIO_VOL_CTRL_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <alsa/asoundlib.h>
#pragma GCC diagnostic pop

// ================================ Simple Controls ================================

typedef struct
{
	snd_pcm_stream_t eStream; /**< It can be SND_PCM_STREAM_PLAYBACK or SND_PCM_STREAM_CAPTURE. */
	const char *szCard; /**< Select the card number to control. The format of it is "hw:N" where N is specified card number. If it is NULL, it will use "default" */
	const char *szSelemName; /**< Name part of a mixer simple element identifier. If it is NULL, it will use "Master" (We can get it from amixer). */
	unsigned int dwSelemIndex; /**< Index part of a mixer simple element identifier (We can get it from amixer).*/
} ATK_AUDIO_SCTRL_CONFIG_T;

/**
 * @brief Function to set volume of all channels.
 *
 * @param[in] ptConfig The configuration for mixer.
 * @param[in] lVol Volume (0~100).
 * @return 0: Successful, Negative value: Failed, -2: No control.
 */
int ATK_Audio_SCtrl_SetVolume(const ATK_AUDIO_SCTRL_CONFIG_T *ptConfig, long lVol);

/**
 * @brief Function to set volume of the specific channel.
 *
 * @param[in] ptConfig The configuration for mixer.
 * @param[in] eChannelId ID of the channel.
 * @param[in] lVol Volume (0~100).
 * @return 0: Successful, Negative value: Failed, -2: No control.
 */
int ATK_Audio_SCtrl_SetChannelVolume(const ATK_AUDIO_SCTRL_CONFIG_T *ptConfig, snd_mixer_selem_channel_id_t eChannelId, long lVol);

/**
 * @brief Function to get volume of the specific channel.
 *
 * @param[in] ptConfig The configuration for mixer.
 * @param[in] eChannelId ID of the channel.
 * @param[out] lVol Volume (0~100).
 * @return 0: Successful, Negative value: Failed, -2: No control.
 */
int ATK_Audio_SCtrl_GetChannelVolume(const ATK_AUDIO_SCTRL_CONFIG_T *ptConfig, snd_mixer_selem_channel_id_t eChannelId, long *plVol);

/**
 * @brief Function to set volume in dB of all channels.
 *
 * @param[in] ptConfig The configuration for mixer.
 * @param[in] lVol The value in dB of volume control.
 * @return 0: Successful, Negative value: Failed, -2: No control.
 */
int ATK_Audio_SCtrl_SetVolume_dB(const ATK_AUDIO_SCTRL_CONFIG_T *ptConfig, long lVol);

/**
 * @brief Function to set volume in dB of the specific channel.
 *
 * @param[in] ptConfig The configuration for mixer.
 * @param[in] eChannelId ID of the channel.
 * @param[in] lVol The value in dB of volume control.
 * @return 0: Successful, Negative value: Failed, -2: No control.
 */
int ATK_Audio_SCtrl_SetChannelVolume_dB(const ATK_AUDIO_SCTRL_CONFIG_T *ptConfig, snd_mixer_selem_channel_id_t eChannelId, long lVol);

/**
 * @brief Function to get volume in dB of the specific channel.
 *
 * @param[in] ptConfig The configuration for mixer.
 * @param[in] eChannelId ID of the channel.
 * @param[out] plVol The value in dB of volume control.
 * @return 0: Successful, Negative value: Failed, -2: No control.
 */
int ATK_Audio_SCtrl_GetChannelVolume_dB(const ATK_AUDIO_SCTRL_CONFIG_T *ptConfig, snd_mixer_selem_channel_id_t eChannelId, long *plVol);

/**
 * @brief Function to get the range of volume in dB.
 *
 * @param[in] ptConfig The configuration for mixer.
 * @param[out] plMin The min value in dB of volume control.
 * @param[out] plMax The max alue in dB of volume control.
 * @return 0: Successful, Negative value: Failed, -2: No control.
 */
int ATK_Audio_SCtrl_GetVolume_Range_dB(const ATK_AUDIO_SCTRL_CONFIG_T *ptConfig, long *plMin, long *plMax);

/**
 * @brief Function to turn on or off all channels.
 *
 * @param[in] ptConfig The configuration for mixer.
 * @param[in] bIsOn non-zero: on, zero: off.
 * @return 0: Successful, Negative value: Failed, -2: No control.
 */
int ATK_Audio_SCtrl_SetSwitch(const ATK_AUDIO_SCTRL_CONFIG_T *ptConfig, int bIsOn);

/**
 * @brief Function to turn on or off the specific channel.
 *
 * @param[in] ptConfig The configuration for mixer.
 * @param[in] eChannelId ID of the channel.
 * @param[in] bIsOn non-zero: on, zero: off.
 * @return 0: Successful, Negative value: Failed, -2: No control.
 */
int ATK_Audio_SCtrl_SetChannelSwitch(const ATK_AUDIO_SCTRL_CONFIG_T *ptConfig, snd_mixer_selem_channel_id_t eChannelId, int bIsOn);

/**
 * @brief Function to get the switch status of the specific channel.
 *
 * @param[in] ptConfig The configuration for mixer.
 * @param[in] eChannelId ID of the channel.
 * @param[out] pbIsOn non-zero: on, zero: off.
 * @return 0: Successful, Negative value: Failed, -2: No control.
 */
int ATK_Audio_SCtrl_GetChannelSwitch(const ATK_AUDIO_SCTRL_CONFIG_T *ptConfig, snd_mixer_selem_channel_id_t eChannelId, int *pbIsOn);

/**
 * @brief Function to set one enumerated element.
 *
 * @param[in] ptConfig The configuration for mixer.
 * @param[in] eChannelId ID of the channel.
 * @param[in] szEnumStr The name of the enumerated item.
 * @return 0: Successful, Negative value: Failed.
 */
int ATK_Audio_SCtrl_Set_Enumerated(const ATK_AUDIO_SCTRL_CONFIG_T *ptConfig, snd_mixer_selem_channel_id_t eChannelId, const char *szEnumStr);

// ================================ Controls ================================

typedef struct
{
	const char *szCard; /**< Select the card number to control. The format of it is "hw:N" where N is specified card number. If it is NULL, it will use "default" */
	const char *szElemName; /**< Name part of a mixer simple element identifier. If it is NULL, it will use "Master" (We can get it from amixer). */
} ATK_AUDIO_CTRL_CONFIG_T;

/**
 * @brief Function to set one interger or boolean element.
 *
 * @param[in] ptConfig The configuration for mixer.
 * @param[in] dwIdx Entry index of the element (You can think it is channel index, when you set volume).
 * @param[in] lVal The value for the entry.
 * @return 0: Successful, Negative value: Failed.
 */
int ATK_Audio_Ctrl_Set_Int_Or_Bool(const ATK_AUDIO_CTRL_CONFIG_T *ptConfig, unsigned int dwIdx, long lVal);

// ================================ High level control ================================
typedef enum { kTKAudioMicIn = 0, kTKAudioLineIn, kTKAudioByPass } TK_AUDIO_INPUT_TYPE;

/**
 * @brief Function to set the volume of capture.
 *
 * @param[in] lVol The volume of capture (0~100).
 * @return 0: Successful, Negative value: Failed.
 */
int ATK_Audio_SetCaptureVolume(long lVol);

/**
 * @brief Function to set the volume in dB of capture.
 *
 * @param[in] lVol The dB value of capture.
 * @return 0: Successful, Negative value: Failed.
 */
int ATK_Audio_SetCaptureVolume_dB(long lVol);

/**
 * @brief Function to set the pga control value.
 *
 * @param[in] lPga value for capture (0~100).
 * @return 0: Successful, Negative value: Failed.
 */
int ATK_Audio_SetCapturePga(long lPga);

/**
 * @brief Function to set the pga control value in dB.
 *
 * @param[in] lPga value in dB for capture.
 * @return 0: Successful, Negative value: Failed.
 */
int ATK_Audio_SetCapturePga_dB(long lPga);

/**
 * @brief Function to mute or unmute the input.
 *
 * @param[in] bIsMute non-zero: mute, zero: unmute.
 * @param[in] eType The input which you want to control.
 * @return 0: Successful, Negative value: Failed.
 */
int ATK_Audio_SetCaptureMute(int bIsMute, TK_AUDIO_INPUT_TYPE eType);

/**
 * @brief Function to select one input.
 *
 * @param[in] eType The input which you want to control.
 * @return 0: Successful, Negative value: Failed.
 */
int ATK_Audio_InputSelection(TK_AUDIO_INPUT_TYPE eType);

/**
 * @brief Function to set the volume of playback.
 *
 * @param[in] lVol The volume of playback (0~100).
 * @return 0: Successful, Negative value: Failed.
 */
int ATK_Audio_SetPlaybackVolume(long lVol);

/**
 * @brief Function to set the volume in dB of playback.
 *
 * @param[in] lVol The dB value of playback.
 * @return 0: Successful, Negative value: Failed.
 */
int ATK_Audio_SetPlaybackVolume_dB(long lVol);

/**
 * @brief Function to mute or unmute the playback.
 *
 * @param[in] bIsMute non-zero: mute, zero: unmute.
 * @return 0: Successful, Negative value: Failed.
 */
int ATK_Audio_SetPlaybackMute(int bIsMute);

#ifdef __cplusplus
}
#endif

#endif
