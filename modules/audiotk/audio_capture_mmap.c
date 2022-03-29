
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
#include <audio_capture_mmap.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#ifndef USE_WALL_CLOCK
#include <time.h>
#endif

#define DEBUG_AUDIO_LOG

struct ATK_AUDIOCAP_HANDLE_T
{
	snd_pcm_t *cap_handle_; /**< ALSA PCM handle. */
	int is_interleaved_; /**< Interleaved mode. */
	unsigned int channels_; /**< Channels. */
	unsigned int bytes_per_sample_; /**< The total bytes of one sample. */
	unsigned int bytes_per_frame_; /**< The total bytes of one audio frame. */
	unsigned int one_channel_period_bytes_; /**< The total size of one channel of one period. */
	unsigned int chunk_bytes_; /**< The total size of one period. */
	unsigned long period_size_; /**< Distance between interrupts is # frames. */
	pthread_t cap_thread_; /**< Thread for capturing audio data. */
	int is_running_; /**< Flag to indicate the state machine is running. */
#ifdef DEBUG_AUDIO_LOG
	snd_output_t *log_out_; /**< Output object for ALSA. */
#endif

	ATK_AUDIO_CAP_DATA_FUNC callback_; /**< Callback funcion for receiving audio data. */
	void* user_data_; /**< User's private data can be passed into the callback funcion. */
};

/**
 * @brief Function to set the parameters for ALSA.
 *
 * @param[in] handle The handle of audio capture device (tk).
 * @param[in] config The audio capture device's configuration.
 * @return 0: Successful, otherwise: Failed.
 */
static int set_params(ATK_AUDIOCAP_HANDLE_T *handle, ATK_AUDIOCAP_CONFIG_T *config)
{
	int err = 0;
	int ret = 0;
	unsigned int rate = 0;
	snd_pcm_hw_params_t *params = NULL;
	snd_pcm_sw_params_t *swparams = NULL;
	snd_pcm_uframes_t buffer_size = 0;
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
	err = snd_pcm_hw_params_any(handle->cap_handle_, params);
	if(err < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't get hardware configurations. %s\n", __FILE__, __func__, snd_strerror(err));
		return -1;
	}

#ifdef DEBUG_AUDIO_LOG
	fprintf(stdout, "\n======== Current hardware configurations start ========\n");
	snd_pcm_dump_hw_setup(handle->cap_handle_, handle->log_out_);
	fprintf(stdout, "======== Current hardware configurations end ========\n\n");
#endif

	// Set access type.
	if(config->bIsInterleaved)
	{
		err = snd_pcm_hw_params_set_access(handle->cap_handle_, params, SND_PCM_ACCESS_MMAP_INTERLEAVED);
	}
	else
	{
		err = snd_pcm_hw_params_set_access(handle->cap_handle_, params, SND_PCM_ACCESS_MMAP_NONINTERLEAVED);
	}

	if(err < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't set access type. %s\n", __FILE__, __func__, snd_strerror(err));
		ret = -1;
		goto set_params_end;
	}

	// Set format.
	err = snd_pcm_hw_params_set_format(handle->cap_handle_, params, config->eFormat);
	if(err < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't set format. %s\n", __FILE__, __func__, snd_strerror(err));
		ret = -1;
		goto set_params_end;
	}

	// Set channels count.
	err = snd_pcm_hw_params_set_channels(handle->cap_handle_, params, config->dwChannelsCount);
	if(err < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't set channels count. %s\n", __FILE__, __func__, snd_strerror(err));
		ret = -1;
		goto set_params_end;
	}

	// Set sample rate.
	rate = config->dwSampleRate;
	err = snd_pcm_hw_params_set_rate_near(handle->cap_handle_, params, &(config->dwSampleRate), 0);
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
		err = snd_pcm_hw_params_set_periods(handle->cap_handle_, params, config->dwPeriodsPerBuffer, 0);
		if(err < 0)
		{
			fprintf(stderr, "[%s, %s]: Can't set periods per buffer. %s\n", __FILE__, __func__, snd_strerror(err));
			ret = -1;
			goto set_params_end;
		}

		// Distance between interrupts is # frames.
		err = snd_pcm_hw_params_set_period_size_near(handle->cap_handle_, params, &(config->dwPeriodSizeInFrames), 0);
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

		err = snd_pcm_hw_params_set_period_time_near(handle->cap_handle_, params, &period_time, 0);
		if(err < 0)
		{
			fprintf(stderr, "[%s, %s]: Can't set period time. %s\n", __FILE__, __func__, snd_strerror(err));
			ret = -1;
			goto set_params_end;
		}

		err = snd_pcm_hw_params_set_buffer_time_near(handle->cap_handle_, params, &buffer_time, 0);
		if(err < 0)
		{
			fprintf(stderr, "[%s, %s]: Can't set period time. %s\n", __FILE__, __func__, snd_strerror(err));
			ret = -1;
			goto set_params_end;
		}
	}

	// Install hardware parameters.
	err = snd_pcm_hw_params(handle->cap_handle_, params);
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
	snd_pcm_sw_params_current(handle->cap_handle_, swparams);

#ifdef DEBUG_AUDIO_LOG
	fprintf(stdout, "\n======== Current software configurations start ========\n");
	snd_pcm_dump_sw_setup(handle->cap_handle_, handle->log_out_);
	fprintf(stdout, "======== Current software configurations end ========\n\n");
#endif

	// Minimum available frames to consider PCM ready.
	err = snd_pcm_sw_params_set_avail_min(handle->cap_handle_, swparams, handle->period_size_);
	if(err < 0)
	{
		fprintf(stderr, "[%s, %s]: Unable to set minimum available frames. %s\n", __FILE__, __func__, snd_strerror(err));
		ret = -1;
		goto set_params_end;
	}

	// Set start threshold in frames. PCM is automatically started when requested capture frames are >= threshold.
	err = snd_pcm_sw_params_set_start_threshold(handle->cap_handle_, swparams, buffer_size);
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
	err = snd_pcm_sw_params_set_stop_threshold(handle->cap_handle_, swparams, buffer_size);
	if(err < 0)
	{
		fprintf(stderr, "[%s, %s]: Unable to set stop threshold. %s\n", __FILE__, __func__, snd_strerror(err));
		ret = -1;
		goto set_params_end;
	}

	// Install software parameters.
	err = snd_pcm_sw_params(handle->cap_handle_, swparams);
	if(err < 0)
	{
		fprintf(stderr, "[%s, %s]: Unable to set software parameters. %s\n", __FILE__, __func__, snd_strerror(err));
		ret = -1;
		goto set_params_end;
	}

	// Calculate some information.
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
	snd_pcm_dump_hw_setup(handle->cap_handle_, handle->log_out_);
	fprintf(stdout, "======== Software ========\n");
	snd_pcm_dump_sw_setup(handle->cap_handle_, handle->log_out_);
	fprintf(stdout, "======== New configurations end ========\n\n");
#endif

	return ret;
}

/**
 * @brief Function to handle I/O error.
 *
 * @param[in] handle The handle of audio capture device (tk).
 * @return 0: Successful, negative: Failed.
 */
static int handle_io_err(ATK_AUDIOCAP_HANDLE_T *handle)
{
	snd_pcm_status_t *status = NULL;
	int ret = 0;

	snd_pcm_status_alloca(&status);
	if ((ret = snd_pcm_status(handle->cap_handle_, status)) < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't get status information. %s\n", __FILE__, __func__, snd_strerror(ret));
		return -1;
	}

	if(snd_pcm_status_get_state(status) == SND_PCM_STATE_XRUN)
	{
		fprintf(stderr, "[%s, %s]: Overrun!!!!!!!!!.\n", __FILE__, __func__);
#ifdef DEBUG_AUDIO_LOG
		fprintf(stderr, "============ Status start ============\n");
		snd_pcm_status_dump(status, handle->log_out_);
		fprintf(stderr, "============ Status end ============\n");
#endif
		if((ret = snd_pcm_prepare(handle->cap_handle_)) < 0)
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
		if((ret = snd_pcm_prepare(handle->cap_handle_)) < 0)
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
 * @param[in] handle The handle of audio capture device (tk).
 * @return 0: Successful, -1: Failed, -2: State machine is not running.
 */
static int handle_io_suspend(ATK_AUDIOCAP_HANDLE_T *handle)
{
	int ret;

#ifdef DEBUG_AUDIO_LOG
	fprintf(stderr, "[%s, %s]: Suspended. Trying resume.\n", __FILE__, __func__);
#endif

	while( (ret = snd_pcm_resume(handle->cap_handle_)) == -EAGAIN )
	{
		// wait until suspend flag is released
		sleep(1);
		if(!handle->is_running_)
		{
			return -2;
		}
	}

	if(ret < 0)
	{
		fprintf(stderr, "[%s, %s]: Failed. Restarting stream.\n", __FILE__, __func__);
		if((ret = snd_pcm_prepare(handle->cap_handle_)) < 0)
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
 * @param[in] handle The handle of audio capture device (tk).
 * @param[in] err_code The error code from ALSA API.
 * @return 0: Successful, negative: Failed.
 */
static int handle_err(ATK_AUDIOCAP_HANDLE_T *handle, int err_code)
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
 * @brief Main loop for capturing audio data.
 *
 * @param[in] data The handle of audio capture device (tk).
 * @return NULL.
 */
static void *audio_capture_loop(void *data)
{
	ATK_AUDIOCAP_HANDLE_T *handle = (ATK_AUDIOCAP_HANDLE_T*) data;
	const snd_pcm_channel_area_t *mmap_areas = NULL;
	snd_pcm_uframes_t offset = 0, frames = 0;
	snd_pcm_sframes_t avail = 0, commitres = 0;
	snd_pcm_state_t state;
	int err = 0, first = 1;
	unsigned int i = 0;
#ifndef USE_WALL_CLOCK
	struct timespec temp_time;
#endif

	// Initialize the notification data.
	ATK_AUDIO_NOTIFY_DATA_INFO_T audio_notify_data_info;
	memset(&audio_notify_data_info, 0, sizeof(ATK_AUDIO_NOTIFY_DATA_INFO_T));
	audio_notify_data_info.bIsInterleaved = handle->is_interleaved_;
	audio_notify_data_info.dwChannels = handle->channels_;
	if(handle->is_interleaved_)
	{
		audio_notify_data_info.dwDataBytes = handle->chunk_bytes_;
		audio_notify_data_info.dwOneFrameBytes = handle->bytes_per_frame_;
	}
	else
	{
		audio_notify_data_info.dwDataBytes = handle->one_channel_period_bytes_;
		audio_notify_data_info.dwOneFrameBytes = handle->bytes_per_sample_;
	}

	// Main loop.
	while(handle->is_running_)
	{
		state = snd_pcm_state(handle->cap_handle_);
		if(state == SND_PCM_STATE_XRUN)
		{
			if((err = handle_io_err(handle)) < 0)
			{
				fprintf(stderr, "[%s, %s]: XRUN recovery failed: %s\n", __FILE__, __func__, snd_strerror(err));
				return NULL;
			}

			first = 1;
		}
		else if(state == SND_PCM_STATE_SUSPENDED)
		{
			if((err = handle_io_suspend(handle)) < 0)
			{
				fprintf(stderr, "[%s, %s]: SUSPEND recovery failed: %s\n", __FILE__, __func__, snd_strerror(err));
				return NULL;
			}
		}

		// Get the number of frames ready to read.
		avail = snd_pcm_avail_update(handle->cap_handle_);
		if (avail < 0)
		{
			if((err = handle_err(handle, avail)) < 0)
			{
				fprintf(stderr, "[%s, %s]: Avail update failed: %s\n", __FILE__, __func__, snd_strerror(err));
				return NULL;
			}
			first = 1;
			continue;
		}

		if ((unsigned int)avail < handle->period_size_)
		{
			if (first)
			{
				first = 0;
				if((err = snd_pcm_start(handle->cap_handle_)) < 0)
				{
					fprintf(stderr, "[%s, %s]: Start error: %s\n", __FILE__, __func__, snd_strerror(err));
					return NULL;
				}
			}
			else
			{
				// snd_pcm_wait() function contains embedded poll waiting implementation.
				// Wait for the data is ready, timeout is four seconds.
				if((err = snd_pcm_wait(handle->cap_handle_, 4000)) < 0)
				{
					if ((err = handle_err(handle, err)) < 0)
					{
						fprintf(stderr, "[%s, %s]: snd_pcm_wait error: %s\n", __FILE__, __func__, snd_strerror(err));
						return NULL;
					}
					first = 1;
				}
			}
			continue;
		}

		frames = handle->period_size_;

		// Request to access a portion of mmap area.
		if((err = snd_pcm_mmap_begin(handle->cap_handle_, &mmap_areas, &offset, &frames)) < 0)
		{
			if ((err = handle_err(handle, err)) < 0)
			{
				fprintf(stderr, "[%s, %s]: MMAP begin error: %s\n", __FILE__, __func__, snd_strerror(err));
				return NULL;
			}
			first = 1;
		}

		if(frames != handle->period_size_)
		{
			snd_pcm_mmap_commit(handle->cap_handle_, offset, frames);
			fprintf(stderr, "[%s, %s]: MMAP error.\n", __FILE__, __func__);
			return NULL;
		}

		// Update timestamp.
#ifndef USE_WALL_CLOCK
		if(clock_gettime(CLOCK_MONOTONIC_RAW, &temp_time) == 0)
		{
			audio_notify_data_info.tDataTimestamp.tv_sec = temp_time.tv_sec;
			audio_notify_data_info.tDataTimestamp.tv_usec = temp_time.tv_nsec / 1000;
		}
		else
		{
			fprintf(stderr, "[%s, %s]: Unable to get monotonic clock : %s\n", __FILE__, __func__, strerror(errno));
		}
#else
		if(gettimeofday(&(audio_notify_data_info.tDataTimestamp), NULL) < 0)
		{
			fprintf(stderr, "[%s, %s]: Unable to get wall clock : %s\n", __FILE__, __func__, strerror(errno));
		}
#endif

		// Get the start address in the mmap area for each channel.
		for(i = 0; i < handle->channels_; ++i)
		{
			audio_notify_data_info.ppbyAudioBufs[i] = ((unsigned char*)mmap_areas[i].addr) + ((mmap_areas[i].first + offset * mmap_areas[i].step) >> 3);
		}

		// Call the callback function to handle audio data.
		handle->callback_(&audio_notify_data_info, handle->user_data_);

		// Has completed the access to mmap area.
		commitres = snd_pcm_mmap_commit(handle->cap_handle_, offset, frames);
		if (commitres < 0 || (snd_pcm_uframes_t)commitres != frames)
		{
			if ((err = handle_err(handle, commitres >= 0 ? -EPIPE : commitres)) < 0)
			{
				fprintf(stderr, "[%s, %s]: MMAP commit error: %s\n", __FILE__, __func__, snd_strerror(err));
				return NULL;
			}
			first = 1;
		}
	}

	return NULL;
}

/**
 * @brief Function to start capture.
 *
 * @param[in] handle The handle of audio capture device (tk).
 * @return 0: Successful, otherwise: Failed.
 */
static int start_capture(ATK_AUDIOCAP_HANDLE_T *handle)
{
	if((handle != NULL) && !(handle->is_running_))
	{
		handle->is_running_ = 1;

		int rc = pthread_create(&handle->cap_thread_, NULL, &audio_capture_loop, handle);
		if(rc)
		{
			fprintf(stderr, "[%s, %s]: ERROR: return code from pthread_create() is %d\n", __FILE__, __func__, rc);
			handle->is_running_ = 0;
			return -1;
		}
	}

	return 0;
}

/**
 * @brief Function to stop capture.
 *
 * @param[in] handle The handle of audio capture device (tk).
 * @return 0: Successful, otherwise: Failed.
 */
static int stop_capture(ATK_AUDIOCAP_HANDLE_T *handle)
{
	if((handle != NULL) && (handle->is_running_))
	{
		handle->is_running_ = 0;

		void* ret_val = NULL;
		int rc = pthread_join(handle->cap_thread_, &ret_val);
		if(rc)
		{
			fprintf(stderr, "[%s, %s]: ERROR: return code from pthread_join() is %d\n", __FILE__, __func__, rc);
			return -1;
		}
	}

	return 0;
}

ATK_AUDIOCAP_HANDLE_T* ATK_AudioCap_Init(ATK_AUDIOCAP_CONFIG_T* ptConfig)
{
	int err = 0;
	ATK_AUDIOCAP_HANDLE_T *handle = NULL;

	if(ptConfig == NULL)
	{
		fprintf(stderr, "[%s, %s]: Initial configuration is NULL.\n", __FILE__, __func__);
		goto init_err;
	}

	// Allocate memory for handler.
	handle = (ATK_AUDIOCAP_HANDLE_T*) calloc(1, sizeof(ATK_AUDIOCAP_HANDLE_T));
	if(handle == NULL)
	{
		fprintf(stderr, "[%s, %s]: Can't allocate memory.\n", __FILE__, __func__);
		goto init_err;
	}

	// Store some information in the handle.
	if(ptConfig->pfnCallback == NULL)
	{
		fprintf(stderr, "[%s, %s]: Callback function is NULL.\n", __FILE__, __func__);
		goto init_err;
	}
	handle->callback_ = ptConfig->pfnCallback;
	handle->user_data_ = ptConfig->pUserData;
	handle->is_interleaved_ = ptConfig->bIsInterleaved;
	handle->channels_ = ptConfig->dwChannelsCount;

	// Open device and we use block mode.
	err = snd_pcm_open(&(handle->cap_handle_), ptConfig->szPcmName, SND_PCM_STREAM_CAPTURE, 0);
	if(err < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't open device. %s\n", __FILE__, __func__, snd_strerror(err));
		goto init_err;
	}

#ifdef DEBUG_AUDIO_LOG
	// We set the third parameter to zero because we don't want to close the stdout when we call snd_output_close function.
	// Otherwise, we will be blocked in snd_output_close function.
	snd_output_stdio_attach(&(handle->log_out_), stdout, 0);
#endif

	if(set_params(handle, ptConfig) < 0)
	{
		goto init_err;
	}

	// Prepare PCM for use.
	err = snd_pcm_prepare(handle->cap_handle_);
	if(err < 0)
	{
		fprintf(stderr, "[%s, %s]: Can't prepare PCM for use. %s\n", __FILE__, __func__, snd_strerror(err));
		goto init_err;
	}

	if(start_capture(handle) < 0)
	{
		goto init_err;
	}

	return handle;

init_err:
	ATK_AudioCap_Release(handle);
	return NULL;
}

void ATK_AudioCap_Release(ATK_AUDIOCAP_HANDLE_T *ptHandle)
{
	if(ptHandle != NULL)
	{
		// Wait thread.
		stop_capture(ptHandle);

		// Close the sound device.
		if(ptHandle->cap_handle_ != NULL)
		{
			snd_pcm_close(ptHandle->cap_handle_);
			ptHandle->cap_handle_ = NULL;
		}

#ifdef DEBUG_AUDIO_LOG
		if(ptHandle->log_out_)
		{
			snd_output_close(ptHandle->log_out_);
		}
#endif
		snd_config_update_free_global();

		free(ptHandle);
	}
}

