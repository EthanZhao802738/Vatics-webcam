/**
 *
 * Copyright (c) 2015 VATICS INC.
 * All Rights Reserved.
 *
 * @brief		Audio PCM raw data playback example.
 *			This demo listens to GPIO keypad and console keyboard events to perform audio data playback using ATK.
 * @author	Max Chang
 * @date		2015/10/30
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <pthread.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/time.h>

#include <audiotk/audio_playback_mmap.h>
#include <audiotk/audio_vol_ctrl.h>
#include "gpio_sysfs_ctrl.h"


#define SND_PLAYBACK_APP_NAME		"snd_playback"

// The period size for audio libraries.
#define PERIOD_SIZE_IN_FRAMES 	1024

// Default GPIO pin number.
#define DEFAULT_GPIO_PIN_NUM	85

// Default size of fd set.
#define FD_SET_NUM				2

// Poll timeout for fds.
#define POLL_TIMEOUT 			(1 * 1000) 				// 3 seconds

// The flag to indicate whether the loop is running or not.
static int bTerminate = 0;

static ATK_AUDIOPLAY_HANDLE_T *atk_playback_handle = NULL;
static ATK_AUDIOPLAY_CONFIG_T atk_playback_config;
static size_t period_frame_size = 0;
static unsigned int playback_volume = 100;

// Audio file buffer.
static char *snd_file_buffer = NULL;
static long int snd_file_size = 0L;

// Audio output buffer.
static unsigned char *atk_playback_bufs[ATK_AUDIO_MAX_CHANNELS] = {NULL};

// GPIO pin number.
static unsigned int gpio_pin_num = DEFAULT_GPIO_PIN_NUM;

// GPIO fdset.
static struct pollfd gpio_fdset[FD_SET_NUM];


typedef struct runtime_context
{
	pthread_mutex_t playback_mutex;
	pthread_cond_t playback_cond;
} runtime_context;

static	runtime_context _runtime_context;

void *playback_thread(void *arg)
{
	runtime_context *context = (runtime_context *) arg;
	char *snd_file_buffer_iterator = NULL;
	long int snd_file_processed_size = 0L;
	ssize_t playback_processed_size = 0;
	(void)playback_processed_size;
	size_t playback_size = 0;

	struct timeval prev_tv;
	struct timeval tv;

	float fltotal_time = 0.0;

	if (!snd_file_buffer)
	{
		fprintf(stderr, "[%s,%d] Failed: snd_file_buffer is NULL\n", __func__, __LINE__);
		return NULL;
	}
	if (!atk_playback_handle)
	{
		fprintf(stderr, "[%s,%d] Failed: atk_playback_handle is NULL\n", __func__, __LINE__);
		return NULL;
	}

	// Get the first output buffer.
	if(ATK_AudioPlay_PlayPeriodFrames(atk_playback_handle, atk_playback_bufs) < 0)
	{
		fprintf(stderr, "[%s,%s] Can't get the audio output buffer..\n", __FILE__, __func__);
		return NULL;
	}

	if (!atk_playback_bufs[0])
	{
		fprintf(stderr, "[%s,%d] Failed: atk_playback_bufs[0] is NULL\n", __func__, __LINE__);
		return NULL;
	}

	pthread_mutex_lock(&context->playback_mutex);
	while (!bTerminate)
	{
		pthread_cond_wait(&context->playback_cond, &context->playback_mutex);
		if (bTerminate)
		{
			printf("[%s,%d] Program exiting ...\n", __func__, __LINE__);
			break;
		}

		printf("[%s,%d] +++++ Start playback +++++ \n", __func__, __LINE__);
		snd_file_buffer_iterator = snd_file_buffer;
		snd_file_processed_size = 0L;
		playback_processed_size = 0;
		playback_size = 0;
		gettimeofday(&prev_tv, NULL);
		fltotal_time = 0.0;

		while (!bTerminate && atk_playback_bufs[0] && (snd_file_processed_size < snd_file_size))
		{
			playback_size = ((snd_file_processed_size + period_frame_size) > (unsigned long int) snd_file_size) ? (snd_file_size - snd_file_processed_size) : period_frame_size;
			memcpy(atk_playback_bufs[0], snd_file_buffer_iterator, playback_size);

			//playback_processed_size = ATK_AudioPlay_PlayFrames(atk_playback_handle, atk_playback_bufs, (unsigned int)playback_size);
			// Output the current buffer.
			if(ATK_AudioPlay_PlayPeriodFrames(atk_playback_handle, atk_playback_bufs) < 0)
			{
				fprintf(stderr, "[%s,%s] Can't get the audio output buffer..\n", __FILE__, __func__);
				break;
			}
			snd_file_processed_size += playback_size;
			snd_file_buffer_iterator += playback_size;
		}

		gettimeofday(&tv, NULL);
		fltotal_time = ((tv.tv_sec - prev_tv.tv_sec)*1000000+tv.tv_usec)-prev_tv.tv_usec;
		printf("[%s,%d] Playback lasted for %f seconds... \n", __func__, __LINE__, fltotal_time/1000000.0f);
		printf("[%s,%d] ----- End playback ----- \n", __func__, __LINE__);
	}
	pthread_mutex_unlock(&context->playback_mutex);

	return NULL;
}

void *gpio_keypad_event_thread(void *arg)
{
	runtime_context *context = (runtime_context *) arg;

	int n_fds = 1;
	int gpio_fd = -1;
	char buf[MAX_BUF];
	unsigned int gpio = gpio_pin_num;

	memset(gpio_fdset, 0, sizeof(gpio_fdset));
	gpio_fdset[0].fd = fileno(stdin);
	gpio_fdset[0].events = POLLRDNORM;

	gpio_export(gpio);
	gpio_set_dir(gpio, 0);
	gpio_set_edge(gpio, GPIO_EDGE_RISING);

	gpio_fd = gpio_fd_open(gpio);
	if (gpio_fd > 0)
	{
		memset(buf, 0, sizeof(buf));
		gpio_fdset[1].fd = gpio_fd;
		gpio_fdset[1].events = POLLPRI;
		n_fds++;
	}
	while (!bTerminate)
	{
		poll(gpio_fdset, n_fds, POLL_TIMEOUT);

		if (gpio_fdset[0].revents & POLLRDNORM)
		{
			printf("Reading data from stdin\n");
			read(gpio_fdset[0].fd, buf, MAX_BUF);
			pthread_mutex_lock(&context->playback_mutex);
			pthread_cond_signal(&context->playback_cond);
			pthread_mutex_unlock(&context->playback_mutex);
		}
		if (gpio_fdset[1].revents & POLLPRI)
		{
			printf("Reading data from GPIO pin %u\n", gpio);
			read(gpio_fdset[1].fd, buf, MAX_BUF);
			pthread_mutex_lock(&context->playback_mutex);
			pthread_cond_signal(&context->playback_cond);
			pthread_mutex_unlock(&context->playback_mutex);
		}
	}
	if (gpio_fd > 0)
	{
		gpio_fd_close(gpio_fd);
	}
	return NULL;
}

static void print_usage(const char *ap_name)
{
        fprintf(stderr, "Usage:\n"
                        "    %s -f filepath [-c channel_num] [-g gpio_pin] [-i interleaved] [-p periods_per_buffer]\n"
                        "                   [-r sample_rate] [-s simple_config] [-v volume]\n"
                        "Options:\n"
                        "    -f                 [Required] Raw data file path (Required) \n"
                        "    -c                 [Optional] Number of channels (Default: 2) \n"
                        "    -g                 [Optional] GPIO pin number (Default: 85) \n"
                        "    -i                 [Optional] Interleaved playback (0 or 1, default: 1) \n"
                        "    -p                 [Optional] Periods per buffer (Default: 4)\n"
                        "    -r                 [Optional] Sample rate (8000, 16000, etc..., Default: 8000) \n"
                        "    -s                 [Optional] Simple configuration (0 or 1, Default: 0) \n"
                        "    -v                 [Optional] Volume (0 to 100, Default: 100) \n"
                        "    -h                 This help. \n", ap_name);
}


static void exit_process()
{
	bTerminate = 1;

	if (gpio_fdset[0].fd)
	{
		gpio_fd_close(gpio_fdset[0].fd);
	}

	pthread_mutex_lock(&_runtime_context.playback_mutex);
	pthread_cond_signal(&_runtime_context.playback_cond);
	pthread_mutex_unlock(&_runtime_context.playback_mutex);
}

static void sig_kill(int signo)
{
	printf("[%s,%s] Receive SIGNAL %d!!!\n", __FILE__, __func__, signo);
	switch(signo)
	{
		case SIGTERM:
		case SIGINT:
			exit_process();
			break;
		default:
			break;
	}
}

int main(int argc, char* argv[])
{
	int ret_val = 0;
	int opt = -1;

	// SND audio file info.
	char *snd_filename = NULL;
	FILE *snd_file_fp = NULL;
	size_t snd_readfile_size = 0;

	// threads and runtime context.
	pthread_t playback_tid = 0;
	pthread_t gpio_tid = 0;

	// Default value of configuration.
	memset(&atk_playback_config, 0, sizeof(ATK_AUDIOPLAY_CONFIG_T));
	atk_playback_config.szPcmName = "hw:0,0";
	atk_playback_config.bIsInterleaved = 1;
	atk_playback_config.eFormat = SND_PCM_FORMAT_S16_LE;
	atk_playback_config.dwChannelsCount = 2;
	atk_playback_config.dwSampleRate = 8000;
	atk_playback_config.bUseSimpleConfig = 0;
	atk_playback_config.dwPeriodsPerBuffer = 4;
	atk_playback_config.dwPeriodSizeInFrames = PERIOD_SIZE_IN_FRAMES;

	while ((opt = getopt(argc, argv, "c:f:g:h:i:p:r:s:v:")) != -1)
	{
		switch(opt)
		{
			case 'c':
				atk_playback_config.dwChannelsCount = atoi(optarg);
				break;
			case 'f':
				snd_filename = strdup(optarg);
				break;
			case 'g':
				gpio_pin_num = atoi(optarg);
				break;
			case 'h':
				print_usage(SND_PLAYBACK_APP_NAME);
				goto main_end;
				break;
			case 'i':
				atk_playback_config.bIsInterleaved = atoi(optarg);
				break;
			case 'p':
				atk_playback_config.dwPeriodsPerBuffer = atoi(optarg);
				break;
			case 'r':
				atk_playback_config.dwSampleRate = atoi(optarg);
				break;
			case 's':
				atk_playback_config.bUseSimpleConfig = atoi(optarg);
				break;
			case 'v':
				playback_volume = atoi(optarg);
				break;
			default:
				print_usage(SND_PLAYBACK_APP_NAME);
				goto main_end;
				break;
		}
	}

	if (!snd_filename)
	{
		fprintf(stderr, "[%s,%d] Please specify SND file to load by -f option\n", __func__, __LINE__);
		ret_val = -1;
		goto main_end;
	}

	// Open the SND file
	printf("[%s,%d] Loading raw data from file %s...\n", __func__, __LINE__, snd_filename);
	snd_file_fp = fopen(snd_filename, "rb");
	if (!snd_file_fp)
	{
		fprintf(stderr, "[%s,%d] Failed to load file - %s\n", __func__, __LINE__, snd_filename);
		ret_val = -1;
		goto main_end;
	}

	// Check file size
	fseek(snd_file_fp, 0L, SEEK_END);
	snd_file_size = ftell(snd_file_fp);
	fseek(snd_file_fp, 0L, SEEK_SET);

	if (snd_file_size > 0)
	{
		snd_file_buffer = (char *)malloc(sizeof(char) * snd_file_size);
	}

	if (!snd_file_buffer)
	{
		fprintf(stderr, "[%s,%d] Failed to allocate memory for snd_file_buffer\n", __func__, __LINE__);
		ret_val = -1;
		goto main_end;
	}
	snd_readfile_size = fread(snd_file_buffer, 1, snd_file_size, snd_file_fp);

	printf("[%s,%d] snd_file_size %ld...\n", __func__, __LINE__, snd_file_size);
	printf("[%s,%d] snd_readfile_size %zu...\n", __func__, __LINE__, snd_readfile_size);

	signal(SIGTERM, sig_kill);
	signal(SIGINT, sig_kill);

	// Initialize ATK audio playback.
	atk_playback_handle = ATK_AudioPlay_Init(&atk_playback_config);
	if(atk_playback_handle == NULL)
	{
		fprintf(stderr, "[%s,%d] Can't initialize the audio playback.\n", __func__, __LINE__);
		ret_val = -1;
		goto main_end;
	}

	ret_val = ATK_Audio_SetPlaybackVolume(playback_volume);
	if (ret_val)
	{
		fprintf(stderr, "[%s,%d] Can't set audio playback volume: %u.\n", __func__, __LINE__, playback_volume);
	}

	// Get the total bytes of data in one period.
	period_frame_size = ATK_AudioPlay_GetPeriodFramesSize(atk_playback_handle);
	//atk_playback_bufs[0] = (unsigned char*) malloc(period_frame_size);

	pthread_mutex_init(&_runtime_context.playback_mutex, NULL);
	pthread_cond_init(&_runtime_context.playback_cond, NULL);

	pthread_create(&playback_tid, NULL, playback_thread, &_runtime_context);
	pthread_create(&gpio_tid, NULL, gpio_keypad_event_thread, &_runtime_context);

	pthread_join(gpio_tid, NULL);
	pthread_join(playback_tid, NULL);

	pthread_cond_destroy(&_runtime_context.playback_cond);
	pthread_mutex_destroy(&_runtime_context.playback_mutex);

main_end:

	/*if (atk_playback_bufs[0])
	{
		free(atk_playback_bufs[0]);
		atk_playback_bufs[0] = NULL;
	}*/
	if (atk_playback_handle)
	{
		ATK_AudioPlay_Release(atk_playback_handle);
		atk_playback_handle = NULL;
	}
	if (snd_file_buffer)
	{
		free(snd_file_buffer);
		snd_file_buffer = NULL;
	}
	if (snd_file_fp)
	{
		fclose(snd_file_fp);
		snd_file_fp = NULL;
	}
	if (snd_filename)
	{
		free(snd_filename);
		snd_filename = NULL;
	}

	return ret_val;
}

