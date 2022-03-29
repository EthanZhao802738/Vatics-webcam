
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
#include <audio_playback_mmap.h>

#define DEBUG_AUDIO_LOG

typedef int (*SILENCE_FUNC)(ATK_AUDIOPLAY_HANDLE_T *handle, unsigned char **bufs, size_t offset, size_t bytes, int channel);

struct ATK_AUDIOPLAY_HANDLE_T
{
	snd_pcm_t *play_handle_; /**< ALSA PCM handle. */
	int is_interleaved_; /**< Interleaved mode. */
	unsigned int channels_; /**< Channels. */
	unsigned int bytes_per_sample_; /**< The total bytes of one sample. */
	unsigned int bytes_per_frame_; /**< The total bytes of one audio frame. */
	unsigned int one_channel_period_bytes_; /**< The total size of one channel of one period. */
	unsigned int chunk_bytes_; /**< The total size of one period. */
	unsigned long period_size_; /**< Distance between interrupts is # frames. */
#ifdef DEBUG_AUDIO_LOG
	snd_output_t *log_out_; /**< Output object for ALSA. */
#endif
	snd_pcm_format_t format_; /**< Sample format. */

	int need_start_pcm_; /**< Flag to control whether we need to start PCM. */
	int is_initialized_; /**< Flag to indicate this object is initialized or not. */
	SILENCE_FUNC silence_func_; /**< Function pointer for setting silence. */
	snd_pcm_uframes_t mmap_offset_; /**< mmap area offset in area steps (==frames). */
	snd_pcm_uframes_t mmap_frames_; /**< mmap area portion size in frames. */
	const snd_pcm_channel_area_t *mmap_areas; /**< mmap areas. */

	unsigned int is_drop_pending_frames_before_stop_; /**0: Stop a PCM preserving pending frames. non-zero: Stop a PCM dropping pending frames.*/
};

/**
 * @brief Function to handle I/O error.
 *
 * @param[in] handle The handle of audio playback device (tk).
 * @return 0: Successful, otherwise: Failed.
 */
static int handle_io_err(ATK_AUDIOPLAY_HANDLE_T *handle)
{
	snd_pcm_status_t *status = NULL;
	int ret = 0;

	snd_pcm_status_alloca(&status);
	if ((ret = snd_pcm_status(handle->play_handle_, status)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't get status information. %s\n", __FILE__, __func__, snd_strerror(ret));
		return -1;
	}

	if(snd_pcm_status_get_state(status) == SND_PCM_STATE_XRUN)
	{
		fprintf(stderr, "[%s, %s]: Underrun!!!!!!!!!.\n", __FILE__, __func__);
#ifdef DEBUG_AUDIO_LOG
		fprintf(stderr, "============ Status start ============\n");
		snd_pcm_status_dump(status, handle->log_out_);
		fprintf(stderr, "============ Status end ============\n");
#endif
		if((ret = snd_pcm_prepare(handle->play_handle_)) < 0)
		{
			fprintf(stderr, "[%s, %s]: Prepare error. %s\n", __FILE__, __func__, snd_strerror(ret));
			return -1;
		}

		// ok, data should be accepted again.
		return 0;
	}
	if(snd_pcm_status_get_state(status) == SND_PCM_STATE_DRAINING)
	{
#ifdef DEBUG_AUDIO_LOG
		fprintf(stderr, "============ Status(DRAINING) start ============\n");
		snd_pcm_status_dump(status, handle->log_out_);
		fprintf(stderr, "============ Status(DRAINING) end ============\n");
#endif

		fprintf(stderr, "[%s, %s]: capture stream format change? attempting recover...\n", __FILE__, __func__);
		if((ret = snd_pcm_prepare(handle->play_handle_)) < 0)
		{
			fprintf(stderr, "[%s, %s]: (DRAINING) Prepare error. %s\n", __FILE__, __func__, snd_strerror(ret));
			return -1;
		}

		return 0;
	}
#ifdef DEBUG_AUDIO_LOG
	fprintf(stderr, "============ Status(R/W) start ============\n");
	snd_pcm_status_dump(status, handle->log_out_);
	fprintf(stderr, "============ Status(R/W) end ============\n");
#endif
	fprintf(stderr, "[%s, %s]: read/write error, state = %s\n", __FILE__, __func__, snd_pcm_state_name(snd_pcm_status_get_state(status)));

	return -1;
}

/**
 * @brief Function to handle I/O suspend.
 *
 * @param[in] handle The handle of audio playback device (tk).
 * @return 0: Successful, -1: Failed, -2: State machine is not running.
 */
static int handle_io_suspend(ATK_AUDIOPLAY_HANDLE_T *handle)
{
	int ret;

#ifdef DEBUG_AUDIO_LOG
	fprintf(stderr, "[%s, %s]: Suspended. Trying resume.\n", __FILE__, __func__);
#endif

	while( (ret = snd_pcm_resume(handle->play_handle_)) == -EAGAIN )
	{
		// wait until suspend flag is released
		sleep(1);
		if(!handle->is_initialized_)
		{
			return -2;
		}
	}

	if(ret < 0)
	{
		fprintf(stderr, "[%s, %s]: Failed. Restarting stream.\n", __FILE__, __func__);
		if((ret = snd_pcm_prepare(handle->play_handle_)) < 0)
		{
			fprintf(stderr, "[%s, %s]: Suspend: prepare error: %s.\n", __FILE__, __func__, snd_strerror(ret));
			return -1;
		}
	}

	fprintf(stderr, "[%s, %s]: Resume from suspend.\n", __FILE__, __func__);
	return 0;
}

/**
 * @brief Function to handle some error code of ALSA.
 *
 * @param[in] handle The handle of audio playback device (tk).
 * @param[in] err_code The error code from ALSA API.
 * @return 0: Successful, negative: Failed.
 */
static int handle_err(ATK_AUDIOPLAY_HANDLE_T *handle, int err_code)
{
	if(err_code == -EPIPE)
	{
		return handle_io_err(handle);
	}
	else if(err_code == -ESTRPIPE)
	{
		return handle_io_suspend(handle);
	}
	else if(err_code < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't handle error. %s\n", __FILE__, __func__, snd_strerror(err_code));
		return -1;
	}

	return 0;
}

/**
 * @brief Function to set the parameters for ALSA.
 *
 * @param[in] handle The handle of audio playback device (tk).
 * @param[in] config The audio playback device's configuration.
 * @return 0: Successful, otherwise: Failed.
 */
static int set_params(ATK_AUDIOPLAY_HANDLE_T *handle, ATK_AUDIOPLAY_CONFIG_T *config)
{
	int err = 0;
	int ret = 0;
	unsigned int rate = 0;
	snd_pcm_hw_params_t *params = NULL;
	snd_pcm_sw_params_t *swparams = NULL;
	snd_pcm_uframes_t buffer_size = 0;
	snd_pcm_uframes_t boundary = 0;
	int bits_per_sample = 0;
	unsigned int bits_per_frame = 0;
	unsigned int period_time = 0;
	unsigned int buffer_time = 0;

	if((handle == NULL) || (config == NULL))
	{
		return -1;
	}

	// Allocate memory for hardware parameters.
	// This temporary space is automatically freed when the
	// function that called alloca() returns to its caller.
	snd_pcm_hw_params_alloca(&params);

	// Get current hardware configurations.
	err = snd_pcm_hw_params_any(handle->play_handle_, params);
	if(err < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't get hardware configurations. %s\n", __FILE__, __func__, snd_strerror(err));
		return -1;
	}

#ifdef DEBUG_AUDIO_LOG
	fprintf(stdout, "\n======== Current hardware configurations start ========\n");
	snd_pcm_dump_hw_setup(handle->play_handle_, handle->log_out_);
	fprintf(stdout, "======== Current hardware configurations end ========\n\n");
#endif

	// Set access type.
	if(config->bIsInterleaved)
	{
		err = snd_pcm_hw_params_set_access(handle->play_handle_, params, SND_PCM_ACCESS_MMAP_INTERLEAVED);
	}
	else
	{
		err = snd_pcm_hw_params_set_access(handle->play_handle_, params, SND_PCM_ACCESS_MMAP_NONINTERLEAVED);
	}

	if(err < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't set access type. %s\n", __FILE__, __func__, snd_strerror(err));
		ret = -1;
		goto set_params_end;
	}

	// Set format.
	err = snd_pcm_hw_params_set_format(handle->play_handle_, params, config->eFormat);
	if(err < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't set format. %s\n", __FILE__, __func__, snd_strerror(err));
		ret = -1;
		goto set_params_end;
	}
	handle->format_ = config->eFormat;

	// Set channels count.
	err = snd_pcm_hw_params_set_channels(handle->play_handle_, params, config->dwChannelsCount);
	if(err < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't set channels count. %s\n", __FILE__, __func__, snd_strerror(err));
		ret = -1;
		goto set_params_end;
	}

	// Set sample rate.
	rate = config->dwSampleRate;
	err = snd_pcm_hw_params_set_rate_near(handle->play_handle_, params, &(config->dwSampleRate), 0);
	if(err < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't set sample rate. %s\n", __FILE__, __func__, snd_strerror(err));
		ret = -1;
		goto set_params_end;
	}

	if ((float)(rate) * 1.05 < config->dwSampleRate || (float)(rate) * 0.95 > config->dwSampleRate)
	{
		fprintf(stderr, "[%s, %s]: Warning: rate is not accurate (requested = %uHz, got = %uHz)\n", __FILE__, __func__, rate, config->dwSampleRate);
	}

	if(!(config->bUseSimpleConfig))
	{
		// Set periods per buffer.
		err = snd_pcm_hw_params_set_periods(handle->play_handle_, params, config->dwPeriodsPerBuffer, 0);
		if(err < 0)
		{
			fprintf(stderr, "[%s, %s]: Can't set periods per buffer. %s\n", __FILE__, __func__, snd_strerror(err));
			ret = -1;
			goto set_params_end;
		}

		// Distance between interrupts is # frames.
		err = snd_pcm_hw_params_set_period_size_near(handle->play_handle_, params, &(config->dwPeriodSizeInFrames), 0);
		if(err < 0)
		{
			fprintf(stderr, "[%s, %s]: Can't set period size. %s\n", __FILE__, __func__, snd_strerror(err));
			ret = -1;
			goto set_params_end;
		}
	}
	else
	{
		err = snd_pcm_hw_params_get_buffer_time_max(params, &buffer_time, 0);
		if(err < 0)
		{
			fprintf(stderr, "[%s, %s]: Can't get max buffer time. %s\n", __FILE__, __func__, snd_strerror(err));
			ret = -1;
			goto set_params_end;
		}
		if(buffer_time > 500000)
			buffer_time = 500000;

		period_time = buffer_time / 4;

		err = snd_pcm_hw_params_set_period_time_near(handle->play_handle_, params, &period_time, 0);
		if(err < 0)
		{
			fprintf(stderr, "[%s, %s]: Can't set period time. %s\n", __FILE__, __func__, snd_strerror(err));
			ret = -1;
			goto set_params_end;
		}

		err = snd_pcm_hw_params_set_buffer_time_near(handle->play_handle_, params, &buffer_time, 0);
		if(err < 0)
		{
			fprintf(stderr, "[%s, %s]: Can't set period time. %s\n", __FILE__, __func__, snd_strerror(err));
			ret = -1;
			goto set_params_end;
		}
	}

	// Install hardware parameters.
	err = snd_pcm_hw_params(handle->play_handle_, params);
	if(err < 0)
	{
		fprintf(stderr, "[%s, %s]: Unable to install hardware parameters. %s\n", __FILE__, __func__, snd_strerror(err));
		ret = -1;
		goto set_params_end;
	}

	snd_pcm_hw_params_get_period_size(params, &(handle->period_size_), 0);
	snd_pcm_hw_params_get_buffer_size(params, &buffer_size);
	if(handle->period_size_ == buffer_size)
	{
		fprintf(stderr, "[%s, %s]: Can't use period equal to buffer size (%lu == %lu).\n", __FILE__, __func__, handle->period_size_, buffer_size);
		ret = -1;
		goto set_params_end;
	}


	// Allocate memory for software parameters.
	// This temporary space is automatically freed when the
	// function that called alloca() returns to its caller.
	snd_pcm_sw_params_alloca(&swparams);

	// Get current software configurations.
	snd_pcm_sw_params_current(handle->play_handle_, swparams);

#ifdef DEBUG_AUDIO_LOG
	fprintf(stdout, "\n======== Current software configurations start ========\n");
	snd_pcm_dump_sw_setup(handle->play_handle_, handle->log_out_);
	fprintf(stdout, "======== Current software configurations end ========\n\n");
#endif

	// Minimum available frames to consider PCM ready.
	err = snd_pcm_sw_params_set_avail_min(handle->play_handle_, swparams, handle->period_size_);
	if(err < 0)
	{
		fprintf(stderr, "[%s, %s]: Unable to set minimum available frames. %s\n", __FILE__, __func__, snd_strerror(err));
		ret = -1;
		goto set_params_end;
	}

	// Set start threshold in frames. PCM is automatically started when playback frames available to PCM are >= threshold.
	err = snd_pcm_sw_params_set_start_threshold(handle->play_handle_, swparams, buffer_size);
	if(err < 0)
	{
		fprintf(stderr, "[%s, %s]: Unable to set start threshold. %s\n", __FILE__, __func__, snd_strerror(err));
		ret = -1;
		goto set_params_end;
	}

	// Set stop threshold in frames. PCM is automatically stopped in SND_PCM_STATE_XRUN state
	// when available frames is >= threshold. If the stop threshold is equal to boundary
	// (also software parameter - sw_param) then automatic stop will be disabled (thus device
	// will do the endless loop in the ring buffer).
	err = snd_pcm_sw_params_set_stop_threshold(handle->play_handle_, swparams, buffer_size);
	if(err < 0)
	{
		fprintf(stderr, "[%s, %s]: Unable to set stop threshold. %s\n", __FILE__, __func__, snd_strerror(err));
		ret = -1;
		goto set_params_end;
	}

	// The special case is when silence size value is equal or greater than boundary.
	// The unused portion of the ring buffer (initial written samples are untouched)
	// is filled with silence at start. Later, only just processed sample area is
	// filled with silence. Note: silence_threshold must be set to zero.
	err = snd_pcm_sw_params_get_boundary(swparams, &boundary);
	if(err < 0)
	{
		fprintf(stderr, "[%s, %s]: Unable to get boundary. %s\n", __FILE__, __func__, snd_strerror(err));
		ret = -1;
		goto set_params_end;
	}

	err = snd_pcm_sw_params_set_silence_size(handle->play_handle_, swparams, boundary);
	if(err < 0)
	{
		fprintf(stderr, "[%s, %s]: Unable to set silence size. %s\n", __FILE__, __func__, snd_strerror(err));
		ret = -1;
		goto set_params_end;
	}

	err = snd_pcm_sw_params_set_silence_threshold(handle->play_handle_, swparams, 0);
	if(err < 0)
	{
		fprintf(stderr, "[%s, %s]: Unable to set silence size. %s\n", __FILE__, __func__, snd_strerror(err));
		ret = -1;
		goto set_params_end;
	}

	// Install software parameters.
	err = snd_pcm_sw_params(handle->play_handle_, swparams);
	if(err < 0)
	{
		fprintf(stderr, "[%s, %s]: Unable to set software parameters. %s\n", __FILE__, __func__, snd_strerror(err));
		ret = -1;
		goto set_params_end;
	}

	// Allocate audio buffer.
	bits_per_sample = snd_pcm_format_physical_width(config->eFormat);
	if(bits_per_sample < 0)
	{
		fprintf(stderr, "[%s, %s]: Unable to get bits per sample. %s\n", __FILE__, __func__, snd_strerror(err));
		ret = -1;
		goto set_params_end;
	}
	handle->bytes_per_sample_ = bits_per_sample >> 3;
	bits_per_frame = bits_per_sample * config->dwChannelsCount;
	handle->bytes_per_frame_ = bits_per_frame >> 3;
	handle->chunk_bytes_ = (handle->period_size_ * bits_per_frame) >> 3;
	handle->one_channel_period_bytes_ = (handle->period_size_ * bits_per_sample) >> 3;

set_params_end:
#ifdef DEBUG_AUDIO_LOG
	fprintf(stdout, "\n======== New configurations start ========\n");
	fprintf(stdout, "======== Hardware ========\n");
	snd_pcm_dump_hw_setup(handle->play_handle_, handle->log_out_);
	fprintf(stdout, "======== Software ========\n");
	snd_pcm_dump_sw_setup(handle->play_handle_, handle->log_out_);
	fprintf(stdout, "======== New configurations end ========\n\n");
#endif

	return ret;
}

/**
 * @brief Function to set silence data in the buffer within one period size.
 *
 * @param[in] handle The handle of audio playback device (tk).
 * @param[in] buf The buffers those need to be set. If it is interleaved mode,
 * all data are in the buf[0]. If it is non-interleaved mode, all data of each
 * channel are in the buf[channel_index].
 * @param[in] offset The offset for the start point which need to be set silence (bytes).
 * @param[in] bytes The size for the portion of the buffer which need to be set silence.
 * @param[in] channel This parameter is used for non-interleaved mode to specify which
 * channel we want to set. If it is invalid, the data of all channel will be set.
 *
 * @return -1: Failed. 0: Success.
 */
static int set_silence_within_period_interleaved(ATK_AUDIOPLAY_HANDLE_T *handle, unsigned char **bufs, size_t offset, size_t bytes, int channel __attribute__((__unused__)))
{
	size_t n_frames = offset / handle->bytes_per_frame_;
	size_t n_silence_frames = bytes / handle->bytes_per_frame_;

	if(n_frames < handle->period_size_)
	{
		if((n_frames + n_silence_frames) > handle->period_size_)
		{
			n_silence_frames = handle->period_size_ - n_frames;
		}

		int err = snd_pcm_format_set_silence(handle->format_, bufs[0] + n_frames * handle->bytes_per_frame_, n_silence_frames * handle->channels_);
		if(err < 0)
		{
			fprintf(stderr, "[%s, %s]: Set silence error: %s\n", __FILE__, __func__, snd_strerror(err));
			return -1;
		}
	}

	return 0;
}

/**
 * @brief Function to set silence data in the buffer within one period size.
 *
 * @param[in] handle The handle of audio playback device (tk).
 * @param[in] buf The buffers those need to be set. If it is interleaved mode,
 * all data are in the buf[0]. If it is non-interleaved mode, all data of each
 * channel are in the buf[channel_index].
 * @param[in] offset The offset for the start point which need to be set silence (bytes).
 * @param[in] bytes The size for the portion of the buffer which need to be set silence.
 * @param[in] channel This parameter is used for non-interleaved mode to specify which
 * channel we want to set. If it is invalid, the data of all channel will be set.
 *
 * @return -1: Failed. 0: Success.
 */
static int set_silence_within_period_non_interleaved(ATK_AUDIOPLAY_HANDLE_T *handle, unsigned char **bufs, size_t offset, size_t bytes, int channel)
{
	size_t n_frames = offset / handle->bytes_per_sample_;
	size_t n_silence_frames = bytes / handle->bytes_per_frame_;

	if(n_frames < handle->period_size_)
	{
		size_t temp_offset = n_frames * handle->bytes_per_sample_;

		if((n_frames + n_silence_frames) > handle->period_size_)
		{
			n_silence_frames = handle->period_size_ - n_frames;
		}

		int err = 0;
		if((channel >= 0) && ((unsigned int)channel < handle->channels_))
		{
			err = snd_pcm_format_set_silence(handle->format_, bufs[channel] + temp_offset, n_silence_frames);
			if(err < 0)
			{
				fprintf(stderr, "[%s, %s]: Set silence error: %s\n", __FILE__, __func__, snd_strerror(err));
				return -1;
			}

		}
		else
		{
			for(unsigned int i = 0; i < handle->channels_; ++i)
			{
				err = snd_pcm_format_set_silence(handle->format_, bufs[i] + temp_offset, n_silence_frames);
				if(err < 0)
				{
					fprintf(stderr, "[%s, %s]: Set silence error: %s\n", __FILE__, __func__, snd_strerror(err));
					return -1;
				}
			}
		}
	}

	return 0;
}

ATK_AUDIOPLAY_HANDLE_T* ATK_AudioPlay_Init(ATK_AUDIOPLAY_CONFIG_T* ptConfig)
{
	int err = 0;
	ATK_AUDIOPLAY_HANDLE_T *handle = NULL;

	if(ptConfig == NULL)
	{
		goto init_err;
	}

	// Allocate memory for handler.
	handle = (ATK_AUDIOPLAY_HANDLE_T*) calloc(1, sizeof(ATK_AUDIOPLAY_HANDLE_T));
	if(handle == NULL)
	{
		fprintf(stderr, "[%s, %s]: Can't allocate memory.\n", __FILE__, __func__);
		goto init_err;
	}

	// Store some information in the handle.
	handle->is_interleaved_ = ptConfig->bIsInterleaved;
	handle->channels_ = ptConfig->dwChannelsCount;
	handle->need_start_pcm_ = 1;
	handle->is_initialized_ = 1;
	handle->silence_func_ = (ptConfig->bIsInterleaved) ? set_silence_within_period_interleaved : set_silence_within_period_non_interleaved;

	// Open device. We use block mode.
	err = snd_pcm_open(&(handle->play_handle_), ptConfig->szPcmName, SND_PCM_STREAM_PLAYBACK, 0);
	if(err < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't open device. %s\n", __FILE__, __func__, snd_strerror(err));
		goto init_err;
	}

#ifdef DEBUG_AUDIO_LOG
	snd_output_stdio_attach(&(handle->log_out_), stdout, 0);
#endif

	if(set_params(handle, ptConfig) < 0)
	{
		goto init_err;
	}

	// Prepare PCM for use.
	err = snd_pcm_prepare(handle->play_handle_);
	if(err < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't prepare PCM for use. %s\n", __FILE__, __func__, snd_strerror(err));
		goto init_err;
	}

	handle->is_drop_pending_frames_before_stop_ = ptConfig->bDropFramesBeforeStop;

	return handle;

init_err:
	ATK_AudioPlay_Release(handle);
	return NULL;
}

size_t ATK_AudioPlay_GetPeriodFramesSize(ATK_AUDIOPLAY_HANDLE_T *ptHandle)
{
	if(ptHandle)
	{
		return (ptHandle->is_interleaved_) ? (ptHandle->chunk_bytes_) : (ptHandle->one_channel_period_bytes_);
	}

	return 0;
}

size_t ATK_AudioPlay_GetOneFrameSize(ATK_AUDIOPLAY_HANDLE_T *ptHandle)
{
	if(ptHandle)
	{
		return (ptHandle->is_interleaved_) ? (ptHandle->bytes_per_frame_) : (ptHandle->bytes_per_sample_);
	}

	return 0;
}

/**
 * @brief Function to play audo frames in the current mmap buffer and get next mmap buffer.
 *
 * @param[in] handle The handle of audio playback device (tk).
 * @param[in, out] bufs The data those need to be played. If it is interleaved mode,
 * all data are in the buf[0]. If it is non-interleaved mode, all data of each
 * channel are in the buf[channel_index]. If we call this function first time, the bufs[0] should be zero.
 * @param[out] frames mmap area portion size in frames. This is the number of frames you must write to the mmap buffer.
 * @param[in] is_write_period_mode Flag to indicate whether we force to write the complete period data.
 *
 * @return -1: Failed. 0: Success.
 */
static int atk_audioplay_play_frames(ATK_AUDIOPLAY_HANDLE_T *handle, unsigned char **bufs, unsigned int *frames, unsigned int is_write_period_mode)
{
	snd_pcm_sframes_t avail = 0, commitres = 0;
	snd_pcm_state_t state;
	int err = 0;
	unsigned int i = 0;

	//if((bufs[0] != NULL) && (handle->mmap_frames_ != 0))
	if(handle->mmap_frames_ != 0)
	{
		// Has completed the access to mmap area.
		commitres = snd_pcm_mmap_commit(handle->play_handle_, handle->mmap_offset_, handle->mmap_frames_);
		if (commitres < 0 || (snd_pcm_uframes_t)commitres != handle->mmap_frames_)
		{
			if ((err = handle_err(handle, commitres >= 0 ? -EPIPE : commitres)) < 0)
			{
				fprintf(stderr, "[%s, %s]: MMAP commit error: %s\n", __FILE__, __func__, snd_strerror(err));
				return -1;
			}
			handle->need_start_pcm_ = 1;
		}
	}

	*frames = 0;

	while(handle->is_initialized_)
	{
		state = snd_pcm_state(handle->play_handle_);
		if(state == SND_PCM_STATE_XRUN)
		{
			if((err = handle_io_err(handle)) < 0)
			{
				fprintf(stderr, "[%s, %s]: XRUN recovery failed: %s\n", __FILE__, __func__, snd_strerror(err));
				return -1;
			}
			handle->need_start_pcm_ = 1;
		}
		else if(state == SND_PCM_STATE_SUSPENDED)
		{
			if((err = handle_io_suspend(handle)) < 0)
			{
				fprintf(stderr, "[%s, %s]: SUSPEND recovery failed: %s\n", __FILE__, __func__, snd_strerror(err));
				return -1;
			}
		}

		avail = snd_pcm_avail_update(handle->play_handle_);
		if(avail < 0)
		{
			if((err = handle_err(handle, avail)) < 0)
			{
				fprintf(stderr, "[%s, %s]: Avail update failed: %s\n", __FILE__, __func__, snd_strerror(err));
				return -1;
			}
			handle->need_start_pcm_ = 1;
			continue;
		}
		if((unsigned int)avail < handle->period_size_)
		{
			if(handle->need_start_pcm_)
			{
				handle->need_start_pcm_ = 0;
				if((err = snd_pcm_start(handle->play_handle_)) < 0)
				{
					fprintf(stderr, "[%s, %s]: Start error: %s\n", __FILE__, __func__, snd_strerror(err));
					return -1;
				}
			}
			else
			{
				// snd_pcm_wait() function contains embedded poll waiting implementation.
				// Wait for the data is ready, timeout is four seconds.
				if((err = snd_pcm_wait(handle->play_handle_, 4000)) < 0)
				{
					if((err = handle_err(handle, err)) < 0)
					{
						fprintf(stderr, "[%s, %s]: snd_pcm_wait error: %s\n", __FILE__, __func__, snd_strerror(err));
						return -1;
					}
					handle->need_start_pcm_ = 1;
				}
			}
			continue;
		}

		handle->mmap_frames_ = handle->period_size_;
		// Request to access a portion of mmap area.
		if((err = snd_pcm_mmap_begin(handle->play_handle_, &(handle->mmap_areas), &(handle->mmap_offset_), &(handle->mmap_frames_))) < 0)
		{
			if ((err = handle_err(handle, err)) < 0)
			{
				fprintf(stderr, "[%s, %s]: MMAP begin error: %s\n", __FILE__, __func__, snd_strerror(err));
				return -1;
			}
			handle->need_start_pcm_ = 1;
		}

		if(is_write_period_mode && (handle->mmap_frames_ != handle->period_size_))
		{
			snd_pcm_mmap_commit(handle->play_handle_, handle->mmap_offset_, handle->mmap_frames_);
			fprintf(stderr, "[%s, %s]: MMAP error.\n", __FILE__, __func__);
			return -1;
		}

		*frames = handle->mmap_frames_;

		// Get the start address in the mmap area for each channel.
		for(i = 0; i < handle->channels_; ++i)
		{
			bufs[i] = ((unsigned char*)(handle->mmap_areas[i].addr)) + ((handle->mmap_areas[i].first + handle->mmap_offset_ * handle->mmap_areas[i].step) >> 3);
		}

		break;
	}

	return 0;
}

int ATK_AudioPlay_PlayPeriodFrames(ATK_AUDIOPLAY_HANDLE_T *ptHandle, unsigned char **ppbyBufs)
{
	unsigned int frames;
	return atk_audioplay_play_frames(ptHandle, ppbyBufs, &frames, 1);
}

int ATK_AudioPlay_PlayFrames(ATK_AUDIOPLAY_HANDLE_T *ptHandle, unsigned char **ppbyBufs, unsigned int *pdwFrames)
{
	return atk_audioplay_play_frames(ptHandle, ppbyBufs, pdwFrames, 0);
}

int ATK_AudioPlay_SetSilenceWithinPeriod(ATK_AUDIOPLAY_HANDLE_T *ptHandle, unsigned char **ppbyBufs, size_t dwOffset, size_t dwBytes, int iChannel)
{
	return ((ptHandle) ? (ptHandle->silence_func_(ptHandle, ppbyBufs, dwOffset, dwBytes, iChannel)) : -1);
}

void ATK_AudioPlay_Release(ATK_AUDIOPLAY_HANDLE_T *ptHandle)
{
	if(ptHandle != NULL)
	{
		ptHandle->is_initialized_ = 0;

		if(ptHandle->mmap_frames_ != 0)
		{
			// Because we fill the silence automatically in the ring buffer, we don't silence the last buffer here.

			// Complete the access to the last buffer
			snd_pcm_mmap_commit(ptHandle->play_handle_, ptHandle->mmap_offset_, ptHandle->mmap_frames_);
			ptHandle->mmap_frames_ = 0;
		}

		// Close the sound device.
		if(ptHandle->play_handle_ != NULL)
		{
			if(ptHandle->is_drop_pending_frames_before_stop_)
			{
				snd_pcm_drop(ptHandle->play_handle_);
			}
			else
			{
				snd_pcm_drain(ptHandle->play_handle_);
			}

			snd_pcm_close(ptHandle->play_handle_);
		}

#ifdef DEBUG_AUDIO_LOG
		snd_output_close(ptHandle->log_out_);
#endif
		snd_config_update_free_global();

		free(ptHandle);
	}
}

