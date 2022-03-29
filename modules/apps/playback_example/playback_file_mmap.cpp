#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <string>

#include <audiotk/audio_playback_mmap.h>
#include <audiotk/audio_common.h>
#include <g711/G711SDec.h>

#define PERIOD_SIZE_IN_FRAMES 1024

static bool is_running_ = true;
static unsigned int g_codec = 0; //0: PCM, 1: G711ULaw

static void exit_process()
{
	is_running_ = false;
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

static void dump_trace(int /*signo*/)
{
	printf(" ===== Segmentation fault. ===== \n");
	exit(EXIT_FAILURE);
}

static void print_usage(const char *ap_name)
{
	fprintf(stderr, "Usage:\n"
			"    %s -d PCM_device_name -r sample_rate -c channels -f filename\n"
			"Options:\n"
			"    -d                 PCM device name for ALSA (ex: hw:0,0).\n"
			"    -r                 Sample rate for audio.\n"
			"    -c                 The number of audio channels.\n"
			"    -f                 The filename of the input raw audio file.\n"
			"    -C                 Codec of the input audio file. 0: PCM(default), 1:G711ULaw\n"
			"    -h                 This help\n", ap_name);
	fprintf(stderr, "ex:\n"
			"%s -d \"hw:0,0\" -r 8000 -c 2 -f audio.raw\n", ap_name);
}

static void play_interleaved_data(ATK_AUDIOPLAY_HANDLE_T *playback_handle, 
	size_t period_frame_size, const char* filename)
{
	size_t read_size = 0;
	size_t total_read_size = 0;
	FILE *audio_fd = fopen(filename, "rb");
	unsigned char* encoded_buf = NULL;

	// Audio output buffer.
	unsigned char *bufs[ATK_AUDIO_MAX_CHANNELS] = {NULL};

	if(audio_fd == NULL)
	{
		fprintf(stderr, "[%s,%s] Can't open input audio file.\n", __FILE__, __func__);
		return;
	}

	printf("period_frame_size = %u\n", period_frame_size);

	if(g_codec) {
		encoded_buf = (unsigned char*)malloc(PERIOD_SIZE_IN_FRAMES);
		if(!encoded_buf) {
			fprintf(stderr, "[%s,%s] Can't malloc audio buffer.\n", __FILE__, __func__);
			fclose(audio_fd);
			return;
		}
	}

	// Get the first output buffer.
	if(ATK_AudioPlay_PlayPeriodFrames(playback_handle, bufs) < 0)
	{
		fprintf(stderr, "[%s,%s] Can't get the audio output buffer..\n", __FILE__, __func__);
		fclose(audio_fd);
		if(encoded_buf)
			free(encoded_buf);
		return;
	}

	while(!feof(audio_fd) && is_running_)
	{
		total_read_size = 0;
		
		if(g_codec) {
			while((total_read_size < PERIOD_SIZE_IN_FRAMES) && (!feof(audio_fd))) {
				read_size = fread(encoded_buf + total_read_size, 1, PERIOD_SIZE_IN_FRAMES - total_read_size, audio_fd);
				if(ferror(audio_fd))
				{
					fprintf(stderr, "[%s,%s] Read file failed.\n", __FILE__, __func__);
					fclose(audio_fd);
					if(encoded_buf)
						free(encoded_buf);
					return;
				}

				total_read_size += read_size;
			}
			G711_ULaw_Decode( encoded_buf, (short*)bufs[0], PERIOD_SIZE_IN_FRAMES, 2 );
		}
		else {
			while((total_read_size < period_frame_size) && (!feof(audio_fd)))
			{
				read_size = fread(bufs[0] + total_read_size, 1, period_frame_size - total_read_size, audio_fd);
				
				if(ferror(audio_fd))
				{
					fprintf(stderr, "[%s,%s] Read file failed.\n", __FILE__, __func__);
					fclose(audio_fd);
					return;
				}

				total_read_size += read_size;
			}

			if(total_read_size != period_frame_size)
			{
				printf("Data length is different from the period size. total_read_size = %u, period_frame_size = %u\n", total_read_size, period_frame_size);
			}
		}

		// Output the current buffer.
		if(ATK_AudioPlay_PlayPeriodFrames(playback_handle, bufs) < 0)
		{
			fprintf(stderr, "[%s,%s] Can't get the audio output buffer..\n", __FILE__, __func__);
			break;
		}
	}

	fclose(audio_fd);
	if(encoded_buf)
		free(encoded_buf);
}

int main(int argc, char **argv)
{
	int opt;
	int ret_val = 0;
	size_t period_frame_size = 0;
	// Audio output buffer.
	ATK_AUDIOPLAY_HANDLE_T *playback_handle = NULL;
	ATK_AUDIOPLAY_CONFIG_T config;
	std::string pcm_name = "hw:0,0";
	std::string audio_filename;

	// Default value of configuration.
	memset(&config, 0, sizeof(ATK_AUDIOPLAY_CONFIG_T));
	config.szPcmName = pcm_name.c_str();
	config.bIsInterleaved = 1;
	config.eFormat = SND_PCM_FORMAT_S16_LE;
	config.dwChannelsCount = 2;
	config.dwSampleRate = 8000;
	config.bUseSimpleConfig = 0;
	config.dwPeriodsPerBuffer = 4;
	config.dwPeriodSizeInFrames = PERIOD_SIZE_IN_FRAMES;

	while ((opt = getopt(argc, argv, "hd:r:c:f:C:")) != -1)
	{
		switch(opt)
		{
			case 'd':
				pcm_name = optarg;
				config.szPcmName = pcm_name.c_str();
				break;
			case 'r':
				config.dwSampleRate = atoi(optarg);
				break;
			case 'c':
				config.dwChannelsCount = atoi(optarg);
				break;
			case 'f':
				audio_filename = optarg;
				break;
			case 'C':
				g_codec = atoi(optarg);
				break;
			case 'h':
			default:
				print_usage(argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	if(audio_filename.empty())
	{
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	signal(SIGTERM, sig_kill);
	signal(SIGINT, sig_kill);
	signal(SIGSEGV, dump_trace);

	// Initialize audio playback.
	playback_handle = ATK_AudioPlay_Init(&config);
	if(playback_handle == NULL)
	{
		fprintf(stderr, "[%s,%s] Can't initialize the audio playback.\n", __FILE__, __func__);
		ret_val = -1;
		goto main_end;
	}

	// Get the total bytes of data in one period.
	period_frame_size = ATK_AudioPlay_GetPeriodFramesSize(playback_handle);

	if(config.bIsInterleaved)
	{
		// Start the loop to play audio.
		play_interleaved_data(playback_handle, period_frame_size, audio_filename.c_str());
	}

main_end:

	// Release audio playback.
	ATK_AudioPlay_Release(playback_handle);

	return ret_val;
}

