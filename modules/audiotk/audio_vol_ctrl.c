
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
#include <audio_vol_ctrl.h>
#include <string.h>

//#define VTCS_AUDIO_CTRL

typedef struct
{
	int (*has_volume)(snd_mixer_elem_t *elem);
	int (*get_volume_range)(snd_mixer_elem_t *elem, long *min, long *max);
	int (*set_volume_all)(snd_mixer_elem_t *elem, long value);
	int (*set_volume)(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, long value);
	int (*get_volume)(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, long *value);

	int (*get_dB_range)(snd_mixer_elem_t *elem, long *min, long *max);
	int (*set_dB_all)(snd_mixer_elem_t *elem, long value, int dir);
	int (*set_dB)(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, long value, int dir);
	int (*get_dB)(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, long *value);

	int (*has_switch)(snd_mixer_elem_t *elem);
	int (*set_switch_all)(snd_mixer_elem_t *elem, int value);
	int (*set_switch)(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, int value);
	int (*get_switch)(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, int *value);
} mixer_ops_t;

static mixer_ops_t mixer_ops_funcs[2] = {
	{
		.has_volume = snd_mixer_selem_has_playback_volume,
		.get_volume_range = snd_mixer_selem_get_playback_volume_range,
		.set_volume_all = snd_mixer_selem_set_playback_volume_all,
		.set_volume = snd_mixer_selem_set_playback_volume,
		.get_volume = snd_mixer_selem_get_playback_volume,
		.get_dB_range = snd_mixer_selem_get_playback_dB_range,
		.set_dB_all = snd_mixer_selem_set_playback_dB_all,
		.set_dB = snd_mixer_selem_set_playback_dB,
		.get_dB = snd_mixer_selem_get_playback_dB,
		.has_switch = snd_mixer_selem_has_playback_switch,
		.set_switch_all = snd_mixer_selem_set_playback_switch_all,
		.set_switch = snd_mixer_selem_set_playback_switch,
		.get_switch = snd_mixer_selem_get_playback_switch
	},
	{
		.has_volume = snd_mixer_selem_has_capture_volume,
		.get_volume_range = snd_mixer_selem_get_capture_volume_range,
		.set_volume_all = snd_mixer_selem_set_capture_volume_all,
		.set_volume = snd_mixer_selem_set_capture_volume,
		.get_volume = snd_mixer_selem_get_capture_volume,
		.get_dB_range = snd_mixer_selem_get_capture_dB_range,
		.set_dB_all = snd_mixer_selem_set_capture_dB_all,
		.set_dB = snd_mixer_selem_set_capture_dB,
		.get_dB = snd_mixer_selem_get_capture_dB,
		.has_switch = snd_mixer_selem_has_capture_switch,
		.set_switch_all = snd_mixer_selem_set_capture_switch_all,
		.set_switch = snd_mixer_selem_set_capture_switch,
		.get_switch = snd_mixer_selem_get_capture_switch
	}
};

/**
 * @brief Function to help to find a element.
 *
 * @param[in] handle The handle for ALSA mixer.
 * @param[in] config The configuration for mixer.
 * @return NULL: Failed, otherwise: element.
 */
static snd_mixer_elem_t* mixer_find_elem_helper(snd_mixer_t *handle, const ATK_AUDIO_SCTRL_CONFIG_T *config)
{
	if((handle == NULL) || (config == NULL))
	{
		return NULL;
	}

	snd_mixer_selem_id_t *sid = NULL;
	const char *card = (config->szCard) ? (config->szCard) : "default";
	int err = 0;

	// Attach an HCTL specified with the CTL device name to an opened mixer.
	if((err = snd_mixer_attach(handle, card)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't attach control. %s\n", __FILE__, __func__, snd_strerror(err));
		return NULL;
	}
	// Register mixer simple element class.
	if((err = snd_mixer_selem_register(handle, NULL, NULL)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't register mixer. %s\n", __FILE__, __func__, snd_strerror(err));
		return NULL;
	}
	// Load a mixer elements.
	if((err = snd_mixer_load(handle)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't load mixer elements. %s\n", __FILE__, __func__, snd_strerror(err));
		return NULL;
	}

	snd_mixer_selem_id_alloca(&sid);
	// Set index part of a mixer simple element identifier.
	snd_mixer_selem_id_set_index(sid, config->dwSelemIndex);
	// Set name part of a mixer simple element identifier.
	snd_mixer_selem_id_set_name(sid, ((config->szSelemName) ? (config->szSelemName) : "Master"));

	// Find a mixer simple element.
	return snd_mixer_find_selem(handle, sid);
}

/**
 * @brief Function to convert the volume from one range to another range.
 *
 * @param[in] from_vol The volume need to be converted.
 * @param[in] from_vol_max The max volume of original range.
 * @param[in] from_vol_min The min volume of original range.
 * @param[in] to_vol_max The max volume of new range.
 * @param[in] to_vol_min The min volume of new range.
 * @return The new volume in the new range ('to' range).
 */
static long convert_range_vol(long from_vol, long from_vol_max, long from_vol_min, long to_vol_max, long to_vol_min)
{
	if((from_vol >= from_vol_max) || (from_vol_max == from_vol_min))
	{
		return to_vol_max - 1;
	}

	if(from_vol <= from_vol_min)
	{
		return 0;
	}

	return (((float)((from_vol - from_vol_min)*(to_vol_max - to_vol_min - 2 )) / (float)(from_vol_max - from_vol_min) + to_vol_min) + 1);

}

int ATK_Audio_SCtrl_SetVolume(const ATK_AUDIO_SCTRL_CONFIG_T *ptConfig, long lVol)
{
	int err = 0;
	snd_mixer_t *handle = NULL;
	snd_mixer_elem_t *elem = NULL;
	long vol_min = 0;
	long vol_max = 0;

	if((ptConfig == NULL) || (lVol < 0) || (lVol > 100))
	{
		return -1;
	}

	if((err = snd_mixer_open(&handle, 0)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't open mixer. %s\n", __FILE__, __func__, snd_strerror(err));
		return -1;
	}

	elem = mixer_find_elem_helper(handle, ptConfig);
	if(elem == NULL)
	{
		snd_mixer_close(handle);
		return -1;
	}

	int stream_idx = ptConfig->eStream;

	if(mixer_ops_funcs[stream_idx].has_volume(elem) == 1)
	{
		mixer_ops_funcs[stream_idx].get_volume_range(elem, &vol_min, &vol_max);
		if((err = mixer_ops_funcs[stream_idx].set_volume_all(elem, convert_range_vol(lVol, 100, 0, vol_max, vol_min))) < 0)
		{
			fprintf(stderr, "[%s, %s]: Can't set volume. %s\n", __FILE__, __func__, snd_strerror(err));
			snd_mixer_close(handle);
			return -1;
		}
	}
	else
	{
		fprintf(stderr, "[%s, %s]: Warning! No volumn control.\n", __FILE__, __func__);
		snd_mixer_close(handle);
		return -2;
	}

	if((err = snd_mixer_close(handle)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't close mixer. %s\n", __FILE__, __func__, snd_strerror(err));
	}

	return 0;
}

int ATK_Audio_SCtrl_SetChannelVolume(const ATK_AUDIO_SCTRL_CONFIG_T *ptConfig, snd_mixer_selem_channel_id_t eChannelId, long lVol)
{
	int err = 0;
	snd_mixer_t *handle = NULL;
	snd_mixer_elem_t *elem = NULL;
	long vol_min = 0;
	long vol_max = 0;

	if((ptConfig == NULL) || (lVol < 0) || (lVol > 100))
	{
		return -1;
	}

	if((err = snd_mixer_open(&handle, 0)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't open mixer. %s\n", __FILE__, __func__, snd_strerror(err));
		return -1;
	}

	elem = mixer_find_elem_helper(handle, ptConfig);
	if(elem == NULL)
	{
		snd_mixer_close(handle);
		return -1;
	}

	int stream_idx = ptConfig->eStream;

	if(mixer_ops_funcs[stream_idx].has_volume(elem) == 1)
	{
		mixer_ops_funcs[stream_idx].get_volume_range(elem, &vol_min, &vol_max);
		if((err = mixer_ops_funcs[stream_idx].set_volume(elem, eChannelId, convert_range_vol(lVol, 100, 0, vol_max, vol_min))) < 0)
		{
			fprintf(stderr, "[%s, %s]: Can't set volume. %s\n", __FILE__, __func__, snd_strerror(err));
			snd_mixer_close(handle);
			return -1;
		}
	}
	else
	{
		fprintf(stderr, "[%s, %s]: Warning! No volumn control.\n", __FILE__, __func__);
		snd_mixer_close(handle);
		return -2;
	}

	if((err = snd_mixer_close(handle)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't close mixer. %s\n", __FILE__, __func__, snd_strerror(err));
	}

	return 0;
}

int ATK_Audio_SCtrl_GetChannelVolume(const ATK_AUDIO_SCTRL_CONFIG_T *ptConfig, snd_mixer_selem_channel_id_t eChannelId, long *plVol)
{
	int err = 0;
	snd_mixer_t *handle = NULL;
	snd_mixer_elem_t *elem = NULL;
	long vol_min = 0;
	long vol_max = 0;
	long hw_vol = 0;

	if((ptConfig == NULL) || (plVol == NULL))
	{
		return -1;
	}

	if((err = snd_mixer_open(&handle, 0)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't open mixer. %s\n", __FILE__, __func__, snd_strerror(err));
		return -1;
	}

	elem = mixer_find_elem_helper(handle, ptConfig);
	if(elem == NULL)
	{
		snd_mixer_close(handle);
		return -1;
	}

	int stream_idx = ptConfig->eStream;

	if(mixer_ops_funcs[stream_idx].has_volume(elem) == 1)
	{
		mixer_ops_funcs[stream_idx].get_volume_range(elem, &vol_min, &vol_max);
		if((err = mixer_ops_funcs[stream_idx].get_volume(elem, eChannelId, &hw_vol)) < 0)
		{
			fprintf(stderr, "[%s, %s]: Can't get volume.\n", __FILE__, __func__);
			snd_mixer_close(handle);
			return -1;
		}
		*plVol = convert_range_vol(hw_vol, vol_max, vol_min, 100, 0);
	}
	else
	{
		fprintf(stderr, "[%s, %s]: Warning! No volumn control.\n", __FILE__, __func__);
		snd_mixer_close(handle);
		return -2;
	}

	if((err = snd_mixer_close(handle)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't close mixer. %s\n", __FILE__, __func__, snd_strerror(err));
	}

	return 0;
}

int ATK_Audio_SCtrl_SetVolume_dB(const ATK_AUDIO_SCTRL_CONFIG_T *ptConfig, long lVol)
{
	int err = 0;
	snd_mixer_t *handle = NULL;
	snd_mixer_elem_t *elem = NULL;

	if(ptConfig == NULL)
	{
		return -1;
	}

	if((err = snd_mixer_open(&handle, 0)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't open mixer. %s\n", __FILE__, __func__, snd_strerror(err));
		return -1;
	}

	elem = mixer_find_elem_helper(handle, ptConfig);
	if(elem == NULL)
	{
		snd_mixer_close(handle);
		return -1;
	}

	int stream_idx = ptConfig->eStream;

	if(mixer_ops_funcs[stream_idx].has_volume(elem) == 1)
	{
		if((err = mixer_ops_funcs[stream_idx].set_dB_all(elem, lVol*100, 0)) < 0)
		{
			fprintf(stderr, "[%s, %s]: Can't set volume in dB. %s\n", __FILE__, __func__, snd_strerror(err));
			snd_mixer_close(handle);
			return -1;
		}
	}
	else
	{
		fprintf(stderr, "[%s, %s]: Warning! No volumn control.\n", __FILE__, __func__);
		snd_mixer_close(handle);
		return -2;
	}

	if((err = snd_mixer_close(handle)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't close mixer. %s\n", __FILE__, __func__, snd_strerror(err));
	}

	return 0;
}

int ATK_Audio_SCtrl_SetChannelVolume_dB(const ATK_AUDIO_SCTRL_CONFIG_T *ptConfig, snd_mixer_selem_channel_id_t eChannelId, long lVol)
{
	int err = 0;
	snd_mixer_t *handle = NULL;
	snd_mixer_elem_t *elem = NULL;

	if(ptConfig == NULL)
	{
		return -1;
	}

	if((err = snd_mixer_open(&handle, 0)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't open mixer. %s\n", __FILE__, __func__, snd_strerror(err));
		return -1;
	}

	elem = mixer_find_elem_helper(handle, ptConfig);
	if(elem == NULL)
	{
		snd_mixer_close(handle);
		return -1;
	}

	int stream_idx = ptConfig->eStream;

	if(mixer_ops_funcs[stream_idx].has_volume(elem) == 1)
	{
		if((err = mixer_ops_funcs[stream_idx].set_dB(elem, eChannelId, lVol*100, 0)) < 0)
		{
			fprintf(stderr, "[%s, %s]: Can't set volume. %s\n", __FILE__, __func__, snd_strerror(err));
			snd_mixer_close(handle);
			return -1;
		}
	}
	else
	{
		fprintf(stderr, "[%s, %s]: Warning! No volumn control.\n", __FILE__, __func__);
		snd_mixer_close(handle);
		return -2;
	}

	if((err = snd_mixer_close(handle)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't close mixer. %s\n", __FILE__, __func__, snd_strerror(err));
	}

	return 0;
}

int ATK_Audio_SCtrl_GetChannelVolume_dB(const ATK_AUDIO_SCTRL_CONFIG_T *ptConfig, snd_mixer_selem_channel_id_t eChannelId, long *plVol)
{
	int err = 0;
	snd_mixer_t *handle = NULL;
	snd_mixer_elem_t *elem = NULL;

	if((ptConfig == NULL) || (plVol == NULL))
	{
		return -1;
	}

	if((err = snd_mixer_open(&handle, 0)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't open mixer. %s\n", __FILE__, __func__, snd_strerror(err));
		return -1;
	}

	elem = mixer_find_elem_helper(handle, ptConfig);
	if(elem == NULL)
	{
		snd_mixer_close(handle);
		return -1;
	}

	int stream_idx = ptConfig->eStream;

	if(mixer_ops_funcs[stream_idx].has_volume(elem) == 1)
	{
		if((err = mixer_ops_funcs[stream_idx].get_dB(elem, eChannelId, plVol)) < 0)
		{
			fprintf(stderr, "[%s, %s]: Can't get volume.\n", __FILE__, __func__);
			snd_mixer_close(handle);
			return -1;
		}

		*plVol /= 100;
	}
	else
	{
		fprintf(stderr, "[%s, %s]: Warning! No volumn control.\n", __FILE__, __func__);
		snd_mixer_close(handle);
		return -2;
	}

	if((err = snd_mixer_close(handle)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't close mixer. %s\n", __FILE__, __func__, snd_strerror(err));
	}

	return 0;
}

int ATK_Audio_SCtrl_GetVolume_Range_dB(const ATK_AUDIO_SCTRL_CONFIG_T *ptConfig, long *plMin, long *plMax)
{
	int err = 0;
	snd_mixer_t *handle = NULL;
	snd_mixer_elem_t *elem = NULL;

	if((ptConfig == NULL) || (plMin == NULL) || (plMax == NULL))
	{
		return -1;
	}

	if((err = snd_mixer_open(&handle, 0)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't open mixer. %s\n", __FILE__, __func__, snd_strerror(err));
		return -1;
	}

	elem = mixer_find_elem_helper(handle, ptConfig);
	if(elem == NULL)
	{
		snd_mixer_close(handle);
		return -1;
	}

	int stream_idx = ptConfig->eStream;

	if(mixer_ops_funcs[stream_idx].has_volume(elem) == 1)
	{
		if((err = mixer_ops_funcs[stream_idx].get_dB_range(elem, plMin, plMax)) < 0)
		{
			fprintf(stderr, "[%s, %s]: Can't get volume.\n", __FILE__, __func__);
			snd_mixer_close(handle);
			return -1;
		}

		*plMin /= 100;
		*plMax /= 100;
	}
	else
	{
		fprintf(stderr, "[%s, %s]: Warning! No volumn control.\n", __FILE__, __func__);
		snd_mixer_close(handle);
		return -2;
	}

	if((err = snd_mixer_close(handle)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't close mixer. %s\n", __FILE__, __func__, snd_strerror(err));
	}

	return 0;
}

int ATK_Audio_SCtrl_SetSwitch(const ATK_AUDIO_SCTRL_CONFIG_T *ptConfig, int bIsOn)
{
	int err = 0;
	snd_mixer_t *handle = NULL;
	snd_mixer_elem_t *elem = NULL;

	if(ptConfig == NULL)
	{
		return -1;
	}

	if((err = snd_mixer_open(&handle, 0)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't open mixer. %s\n", __FILE__, __func__, snd_strerror(err));
		return -1;
	}

	elem = mixer_find_elem_helper(handle, ptConfig);
	if(elem == NULL)
	{
		snd_mixer_close(handle);
		return -1;
	}

	int stream_idx = ptConfig->eStream;

	if(mixer_ops_funcs[stream_idx].has_switch(elem) == 1)
	{
		if((err = mixer_ops_funcs[stream_idx].set_switch_all(elem, bIsOn)) < 0)
		{
			fprintf(stderr, "[%s, %s]: Can't turn on or off. %s\n", __FILE__, __func__, snd_strerror(err));
			snd_mixer_close(handle);
			return -1;
		}
	}
	else
	{
		fprintf(stderr, "[%s, %s]: Warning! No volumn control.\n", __FILE__, __func__);
		snd_mixer_close(handle);
		return -2;
	}

	if((err = snd_mixer_close(handle)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't close mixer. %s\n", __FILE__, __func__, snd_strerror(err));
	}

	return 0;
}

int ATK_Audio_SCtrl_SetChannelSwitch(const ATK_AUDIO_SCTRL_CONFIG_T *ptConfig, snd_mixer_selem_channel_id_t eChannelId, int bIsOn)
{
	int err = 0;
	snd_mixer_t *handle = NULL;
	snd_mixer_elem_t *elem = NULL;

	if(ptConfig == NULL)
	{
		return -1;
	}

	if((err = snd_mixer_open(&handle, 0)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't open mixer. %s\n", __FILE__, __func__, snd_strerror(err));
		return -1;
	}

	elem = mixer_find_elem_helper(handle, ptConfig);
	if(elem == NULL)
	{
		snd_mixer_close(handle);
		return -1;
	}

	int stream_idx = ptConfig->eStream;

	if(mixer_ops_funcs[stream_idx].has_switch(elem) == 1)
	{
		if((err = mixer_ops_funcs[stream_idx].set_switch(elem, eChannelId, bIsOn)) < 0)
		{
			fprintf(stderr, "[%s, %s]: Can't mute. %s\n", __FILE__, __func__, snd_strerror(err));
			snd_mixer_close(handle);
			return -1;
		}
	}
	else
	{
		fprintf(stderr, "[%s, %s]: Warning! No volumn control.\n", __FILE__, __func__);
		snd_mixer_close(handle);
		return -2;
	}

	if((err = snd_mixer_close(handle)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't close mixer. %s\n", __FILE__, __func__, snd_strerror(err));
	}

	return 0;
}

int ATK_Audio_SCtrl_GetChannelSwitch(const ATK_AUDIO_SCTRL_CONFIG_T *ptConfig, snd_mixer_selem_channel_id_t eChannelId, int *pbIsOn)
{
	int err = 0;
	snd_mixer_t *handle = NULL;
	snd_mixer_elem_t *elem = NULL;

	if((ptConfig == NULL) || (pbIsOn == NULL))
	{
		return -1;
	}

	if((err = snd_mixer_open(&handle, 0)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't open mixer. %s\n", __FILE__, __func__, snd_strerror(err));
		return -1;
	}

	elem = mixer_find_elem_helper(handle, ptConfig);
	if(elem == NULL)
	{
		snd_mixer_close(handle);
		return -1;
	}

	int stream_idx = ptConfig->eStream;

	if(mixer_ops_funcs[stream_idx].has_switch(elem) == 1)
	{
		if((err = mixer_ops_funcs[stream_idx].get_switch(elem, eChannelId, pbIsOn)) < 0)
		{
			fprintf(stderr, "[%s, %s]: Can't mute. %s\n", __FILE__, __func__, snd_strerror(err));
			snd_mixer_close(handle);
			return -1;
		}
	}
	else
	{
		fprintf(stderr, "[%s, %s]: Warning! No volumn control.\n", __FILE__, __func__);
		snd_mixer_close(handle);
		return -2;
	}

	if((err = snd_mixer_close(handle)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't close mixer. %s\n", __FILE__, __func__, snd_strerror(err));
	}

	return 0;
}

int ATK_Audio_SCtrl_Set_Enumerated(const ATK_AUDIO_SCTRL_CONFIG_T *ptConfig, snd_mixer_selem_channel_id_t eChannelId, const char *szEnumStr)
{
	int err = 0;
	snd_mixer_t *handle = NULL;
	snd_mixer_elem_t *elem = NULL;

	if((ptConfig == NULL) || (szEnumStr == NULL))
	{
		return -1;
	}

	if((err = snd_mixer_open(&handle, 0)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't open mixer. %s\n", __FILE__, __func__, snd_strerror(err));
		return -1;
	}

	elem = mixer_find_elem_helper(handle, ptConfig);
	if(elem == NULL)
	{
		snd_mixer_close(handle);
		return -1;
	}

	if(snd_mixer_selem_is_enumerated(elem))
	{
		int items = snd_mixer_selem_get_enum_items(elem);
		if(items < 0)
		{
			snd_mixer_close(handle);
			return -1;
		}

		char name[128] = {'\0'};
		for(int i = 0; i < items; ++i)
		{
			if(snd_mixer_selem_get_enum_item_name(elem, i, sizeof(name)-1, name) < 0)
			{
				continue;
			}
			if(strcmp(name, szEnumStr) == 0)
			{
				if((err = snd_mixer_selem_set_enum_item(elem, eChannelId, i)) < 0)
				{
					fprintf(stderr, "[%s, %s]: Can't set enumerated value. %s\n", __FILE__, __func__, snd_strerror(err));
					snd_mixer_close(handle);
					return -1;
				}

				snd_mixer_close(handle);
				return 0;
			}
		}
	}

	snd_mixer_close(handle);
	return -1;
}

int ATK_Audio_Ctrl_Set_Int_Or_Bool(const ATK_AUDIO_CTRL_CONFIG_T *ptConfig, unsigned int dwIdx, long lVal)
{
	int err = 0;
	snd_hctl_t *hctl = NULL;
	snd_ctl_elem_id_t *id = NULL;
	snd_hctl_elem_t *elem = NULL;
	snd_ctl_elem_value_t *control = NULL;
	snd_ctl_elem_info_t *info = NULL;
	snd_ctl_elem_type_t type;

	if(ptConfig == NULL)
	{
		return -1;
	}

	if((err = snd_hctl_open(&hctl, (ptConfig->szCard) ? (ptConfig->szCard) : "hw:0", 0)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't open hight level control. %s\n", __FILE__, __func__, snd_strerror(err));
		return -1;
	}
	if((err = snd_hctl_load(hctl)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't load all elements for hight level control. %s\n", __FILE__, __func__, snd_strerror(err));
		snd_hctl_close(hctl);
		return -1;
	}

	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);
	snd_ctl_elem_id_set_name(id, ptConfig->szElemName);

	elem = snd_hctl_find_elem(hctl, id);
	if(elem == NULL)
	{
		fprintf(stderr, "[%s, %s]: Can't find element %s.\n", __FILE__, __func__, ptConfig->szElemName);
		snd_hctl_close(hctl);
		return -1;
	}

	snd_ctl_elem_info_alloca(&info);
	if((err = snd_hctl_elem_info(elem, info)) < 0)
	{
		fprintf(stderr, "[%s, %s]: snd_hctl_elem_info error: %s.\n", __FILE__, __func__, snd_strerror(err));
		snd_hctl_close(hctl);
		return -1;
	}
	type = snd_ctl_elem_info_get_type(info);

	snd_ctl_elem_value_alloca(&control);
	snd_ctl_elem_value_set_id(control, id);

	switch(type)
	{
		case SND_CTL_ELEM_TYPE_BOOLEAN:
			snd_ctl_elem_value_set_boolean(control, dwIdx, lVal);
			break;
		case SND_CTL_ELEM_TYPE_INTEGER:
			snd_ctl_elem_value_set_integer(control, dwIdx, lVal);
			break;
		default:
			// We don't use SND_CTL_ELEM_TYPE_INTEGER64
			fprintf(stderr, "[%s, %s]: Unsupport element type.\n", __FILE__, __func__);
			snd_hctl_close(hctl);
			return -1;
	}

	if((err = snd_hctl_elem_write(elem, control)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't write the value to element. %s\n", __FILE__, __func__, snd_strerror(err));
		snd_hctl_close(hctl);
		return -1;
	}

	if((err = snd_hctl_close(hctl)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't close hight level control. %s\n", __FILE__, __func__, snd_strerror(err));
	}

	return 0;
}

static int atk_audio_set_capture_volume(long vol, int mode)
{
	ATK_AUDIO_SCTRL_CONFIG_T audio_config;
	memset(&audio_config, 0, sizeof(ATK_AUDIO_SCTRL_CONFIG_T));

	typedef int (*set_vol_func_ptr)(const ATK_AUDIO_SCTRL_CONFIG_T *config, long vol);
	set_vol_func_ptr set_func = (mode) ? ATK_Audio_SCtrl_SetVolume_dB : ATK_Audio_SCtrl_SetVolume;

#ifdef VTCS_AUDIO_CTRL
	int ret = 0;
	int ret2 = 0;

	audio_config.eStream = SND_PCM_STREAM_CAPTURE;
	audio_config.szSelemName = "Capture";
	ret = set_func(&audio_config, vol);

	audio_config.eStream = SND_PCM_STREAM_CAPTURE;
	audio_config.szSelemName = "Input";
	ret2 = set_func(&audio_config, vol);

	return ((ret == 0) || (ret2 == 0)) ? 0 : -1;
#else
	audio_config.eStream = SND_PCM_STREAM_CAPTURE;
	audio_config.szSelemName = "Capture";
	return set_func(&audio_config, vol);
#endif
}

int ATK_Audio_SetCaptureVolume(long lVol)
{
	return atk_audio_set_capture_volume(lVol, 0);
}

int ATK_Audio_SetCaptureVolume_dB(long lVol)
{
	return atk_audio_set_capture_volume(lVol, 1);
}

/*
 * PGA vol  control
 * */
static int atk_audio_set_capture_pga(long vol, int mode)
{
	ATK_AUDIO_SCTRL_CONFIG_T audio_config;
	memset(&audio_config, 0, sizeof(ATK_AUDIO_SCTRL_CONFIG_T));

	typedef int (*set_vol_func_ptr)(const ATK_AUDIO_SCTRL_CONFIG_T *config, long vol);
	set_vol_func_ptr set_func = (mode) ? ATK_Audio_SCtrl_SetVolume_dB : ATK_Audio_SCtrl_SetVolume;

	audio_config.eStream = SND_PCM_STREAM_CAPTURE;
	audio_config.szSelemName = "PGA";

	return set_func(&audio_config, vol);
}

int ATK_Audio_SetCapturePga(long lPga)
{
	return atk_audio_set_capture_pga(lPga, 0);
}

int ATK_Audio_SetCapturePga_dB(long lPga)
{
	return atk_audio_set_capture_pga(lPga, 1);
}
/*
 * PGA ends
 * */
int ATK_Audio_SetCaptureMute(int bIsMute, TK_AUDIO_INPUT_TYPE eType)
{
#ifdef VTCS_AUDIO_CTRL
	int err = 0;
	int err2 = 0;
	int err3 = 0;
	ATK_AUDIO_CTRL_CONFIG_T audio_config;
	memset(&audio_config, 0, sizeof(ATK_AUDIO_CTRL_CONFIG_T));

	audio_config.szElemName = "Mic Mute";
	err = ATK_Audio_Ctrl_Set_Int_Or_Bool(&audio_config, 0, bIsMute);

	err2 = ATK_Audio_SetCaptureVolume((bIsMute) ? 0 : 100);

	audio_config.szElemName = "Left ADC mute";
	err3 = ATK_Audio_Ctrl_Set_Int_Or_Bool(&audio_config, 0, bIsMute);

	return ((err == 0) || (err2 == 0) || (err3 == 0)) ? 0 : -1;
#else
	int err = -1;
	int err2 = 0;

	ATK_AUDIO_SCTRL_CONFIG_T audio_config;
	memset(&audio_config, 0, sizeof(ATK_AUDIO_SCTRL_CONFIG_T));
	audio_config.eStream = SND_PCM_STREAM_CAPTURE;

	//audio_config.szSelemName = "Capture";
	//err = ATK_Audio_SCtrl_SetSwitch(&audio_config, !bIsMute);

	switch(eType)
	{
		case kTKAudioMicIn:
			audio_config.szSelemName = "Mic";
			err2 = ATK_Audio_SCtrl_SetSwitch(&audio_config, !bIsMute);
			break;
		case kTKAudioLineIn:
			audio_config.szSelemName = "Line";
			err2 = ATK_Audio_SCtrl_SetSwitch(&audio_config, !bIsMute);
			break;
		case kTKAudioByPass:
			break;
		default:
			err2 = -1;
			break;
	}

	return ((err == 0) || (err2 == 0)) ? 0 : -1;
#endif
}

int ATK_Audio_InputSelection(TK_AUDIO_INPUT_TYPE eType)
{
#if 1 //def VTCS_AUDIO_CTRL
	ATK_AUDIO_CTRL_CONFIG_T audio_config;
	memset(&audio_config, 0, sizeof(ATK_AUDIO_CTRL_CONFIG_T));

	if(eType != kTKAudioByPass)
	{
		//audio drivers use different "control anme".

		audio_config.szElemName = "Input Select"; //Pesaro built-in audio codec.
		// Line in: 0, Mic in (single ended): 2, Mic in (differential): 4
		if (ATK_Audio_Ctrl_Set_Int_Or_Bool(&audio_config, 0, ((eType == kTKAudioMicIn) ? 2 : 0)) == 0)
			return 0;

		//other audio codec for Mozart 3s
		audio_config.szElemName = "Input Type";
		if (ATK_Audio_Ctrl_Set_Int_Or_Bool(&audio_config, 0, ((eType == kTKAudioMicIn) ? 0 : 1)) == 0)
			return 0;

		audio_config.szElemName = "Input Selection";
		if(ATK_Audio_Ctrl_Set_Int_Or_Bool(&audio_config, 0, ((eType == kTKAudioMicIn) ? 2 : 1)) == 0)
			return 0;

		return -1;
	}

	audio_config.szElemName = "Input Selection";
	return ATK_Audio_Ctrl_Set_Int_Or_Bool(&audio_config, 0, 0);
#else
	int err = 0, err2 = 0, err3 = 0, err4 = 0;
	ATK_AUDIO_SCTRL_CONFIG_T audio_config;
	memset(&audio_config, 0, sizeof(ATK_AUDIO_SCTRL_CONFIG_T));

	audio_config.eStream = SND_PCM_STREAM_CAPTURE;

	switch(eType)
	{
		case kTKAudioMicIn:
			audio_config.szSelemName = "Bypass Capture";
			err = ATK_Audio_SCtrl_SetSwitch(&audio_config, 0);
			audio_config.szSelemName = "Line";
			err2 = ATK_Audio_SCtrl_SetSwitch(&audio_config, 0);
			audio_config.szSelemName = "Mic";
			err3 = ATK_Audio_SCtrl_SetSwitch(&audio_config, 1);
			audio_config.szSelemName = "Capture Source";
			err4 = ATK_Audio_SCtrl_Set_Enumerated(&audio_config, 0, "Mic");
			break;
		case kTKAudioLineIn:
			audio_config.szSelemName = "Bypass Capture";
			err = ATK_Audio_SCtrl_SetSwitch(&audio_config, 0);
			audio_config.szSelemName = "Mic";
			err2 = ATK_Audio_SCtrl_SetSwitch(&audio_config, 0);
			audio_config.szSelemName = "Line";
			err3 = ATK_Audio_SCtrl_SetSwitch(&audio_config, 1);
			audio_config.szSelemName = "Capture Source";
			err4 = ATK_Audio_SCtrl_Set_Enumerated(&audio_config, 0, "Line");
			break;
		case kTKAudioByPass:
			audio_config.szSelemName = "Line";
			err = ATK_Audio_SCtrl_SetSwitch(&audio_config, 0);
			audio_config.szSelemName = "Mic";
			err2 = ATK_Audio_SCtrl_SetSwitch(&audio_config, 0);
			audio_config.szSelemName = "Bypass Capture";
			err3 = ATK_Audio_SCtrl_SetSwitch(&audio_config, 1);
			break;
		default:
			return -1;
	}
	return ((err == 0) && (err2 == 0) && (err3 == 0) && (err4 == 0)) ? 0 : -1;
#endif
}

static int atk_audio_set_playback_volume(long vol, int mode)
{
	int err = 0, err2 = 0;

	typedef int (*set_vol_func_ptr)(const ATK_AUDIO_SCTRL_CONFIG_T *config, long vol);
	set_vol_func_ptr set_func = (mode) ? ATK_Audio_SCtrl_SetVolume_dB : ATK_Audio_SCtrl_SetVolume;

	ATK_AUDIO_SCTRL_CONFIG_T audio_config;
	memset(&audio_config, 0, sizeof(ATK_AUDIO_SCTRL_CONFIG_T));

	audio_config.eStream = SND_PCM_STREAM_PLAYBACK;

	audio_config.szSelemName = "Master";
	err = set_func(&audio_config, vol);

	audio_config.szSelemName = "Playback";
	err2 = set_func(&audio_config, vol);

	return ((err == 0) || (err2 == 0)) ? 0 : -1;
}

int ATK_Audio_SetPlaybackVolume(long lVol)
{
	return atk_audio_set_playback_volume(lVol, 0);
}

int ATK_Audio_SetPlaybackVolume_dB(long lVol)
{
	return atk_audio_set_playback_volume(lVol, 1);
}

int ATK_Audio_SetPlaybackMute(int bIsMute)
{
	int err = 0, err2 = 0;

	ATK_AUDIO_SCTRL_CONFIG_T audio_config;
	memset(&audio_config, 0, sizeof(ATK_AUDIO_SCTRL_CONFIG_T));

	audio_config.eStream = SND_PCM_STREAM_PLAYBACK;

	audio_config.szSelemName = "Master";
	err = ATK_Audio_SCtrl_SetSwitch(&audio_config, !bIsMute);

	audio_config.szSelemName = "Playback";
	err2 = ATK_Audio_SCtrl_SetSwitch(&audio_config, !bIsMute);

	return ((err == 0) || (err2 == 0)) ? 0 : -1;
}

