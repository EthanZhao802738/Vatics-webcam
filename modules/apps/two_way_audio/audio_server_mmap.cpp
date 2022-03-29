/**
 *
 * Copyright (c) 2015 VATICS INC.
 * All Rights Reserved.
 *
 * @brief		Atk two way audio example.
 *				This demo demostrates server-side two way audio using TCP socket and Atk.
 * @author		Max Chang
 * @date		2016/2/25
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <pthread.h>

#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <audio_setting.h>


#define APP_NAME		"audio_server"

twa_ctx audio_server_mmap_config;

static void print_usage(const char *ap_name)
{
	fprintf(stderr, "Usage:\n"
			"    %s [-P server_port] [-c channel_num] [-d pcm_device] [-i interleaved] [-l input_type] [-p periods_per_buffer]\n"
			"                   [-r sample_rate] [-s simple_config] [-v volume] [-h]\n"
			"Options:\n"
			"    -P                 [Optional] Server port (Default: 999) \n"
			"    -c                 [Optional] Number of channels (Default: 2) \n"
			"    -d                 [Optional] PCM device (hw:0,0 or plug:vatics(ALSA plugin device for AEC). Default: hw:0,0) \n"
			"    -i                 [Optional] Interleaved playback (0 or 1, default: 1) \n"
			"    -l                 [Optional] Input type of audio (0: MicIn, 1: LineIn. Default: MicIn).\n"
			"    -p                 [Optional] Periods per buffer (Default: 4)\n"
			"    -r                 [Optional] Sample rate (8000, 16000, etc..., Default: 8000) \n"
			"    -s                 [Optional] Simple configuration (0 or 1, Default: 0) \n"
			"    -t                 [Optional] Transfer type (0: UDP 1: TCP, Default: 0) \n"
			"    -v                 [Optional] Playback volume (0 to 100, Default: 100) \n"
			"    -C                 [Optional] Codec type for encoder (0: AAC4, 1: G711, 2: G726, 3:PCM, Default:G711).\n"	
			"    -b                 [Optional] Bit rate for audio encoder (AAC4: 8000 ~ 320000, G726: 16000, 24000, 32000, 40000).\n"
			"    -A                 [Optional] AAC4 stream type (1: ADTS, 2: ADIF, Default: 1) \n"
			"    -h                 [Optional] This help. \n", ap_name);
}

static void sig_kill(int signo)
{
	printf("[%s,%s] Receive SIGNAL %d!!!\n", __FILE__, __func__, signo);
	switch(signo)
	{
		case SIGTERM:
		case SIGINT:
			twa_ctx_stop(&audio_server_mmap_config);
			break;
		default:
			break;
	}
}

int main(int argc, char* argv[])
{
	int ret_val = 0;

    twa_ctx_init(&audio_server_mmap_config, SERVER_SITE);

	ret_val = twa_ctx_load_prog_args(argc,argv,&audio_server_mmap_config);

	if(ret_val == -1){
		print_usage(APP_NAME);
		goto main_end;
	}

	ret_val = twa_ctx_network_init(&audio_server_mmap_config);
	if(ret_val == -1)
		goto main_end;

	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, sig_kill);
	signal(SIGINT, sig_kill);

	twa_ctx_audio_init(&audio_server_mmap_config);
	if(ret_val == -1)
		goto main_end;

	twa_ctx_start(&audio_server_mmap_config);
	

main_end:
	twa_ctx_audio_release(&audio_server_mmap_config);
	twa_ctx_network_release(&audio_server_mmap_config);
	return ret_val;
}


