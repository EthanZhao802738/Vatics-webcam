
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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string>
#include <poll.h>

#include <SyncRingBuffer/sync_ring_buffer.h>
#include <audiotk/audio_capture_mmap.h>
#include <audiotk/audio_playback_mmap.h>
#include <audiotk/audio_vol_ctrl.h>

#define SRB_HEADER_SIZE 4
#define ATK_AUDIO_MAX_CHANNELS 32
#define PERIOD_SIZE_IN_FRAMES 1024
#define APP_NAME "UAC Service"
#define CAP_DIR "./"

#define UNUSED(x) (void)(x)

enum {
	FALSE = 0,
	TRUE,
};

//Playback Handle
ATK_AUDIOPLAY_HANDLE_T* playback_handle;

//Capture Handle
ATK_AUDIOCAP_HANDLE_T* capture_handle;

static int isRunning = 0;
static int reset = 0;
static int savefile = 0;
char const *file_name = NULL;
char filename[128];
FILE* fp;
srb_handle_t* reader_handle = NULL;
srb_handle_t* writer_handle = NULL;
srb_buffer_t srb_writer_buf;
srb_buffer_t srb_reader_buf;

//======================================
//HELP
static void print_usage(const char *ap_name)
{
	fprintf(stderr, "Usage:\n"
			"    %s [-i input_type]  [-m][-h]\n"
			"Options:\n"
			"	 -r					Sample rate, it is applied to both capture and playback\n"
			"	 -i 				Input Selection 0:Mic, 1:LineIn (0: default)\n"
			"	 -s 				save recording file with filename 0:Don't save 1:Save (0:default)\n"
			"	 -f 				filename  Required if using -s option\n "
			"	 -t 				Select if want to play interlevead audio 1: interleaved 0: no interleaved default :0\n "
			"	 -c 				number of channels: 1: 1 channel  2: 2 channels  \n "
			"	 -o 				simple configuration 1: simple conf on 0: simple config off, defualt value 0\n "
			"	 -p 				periods contained in one buffer. default value 8 \n "
			"	 -d 				Sound device 0:Normal, 1:USB (0: default)\n "
			"    -h                 This help.\n", ap_name);
}

//======================================
//write to fp
static void write_to_file(unsigned char *data_buf, size_t buf_size)
{
	fwrite(data_buf, buf_size, 1, fp);
	fflush(fp);
}

//======================================
//capture thread / SRB writer
static void audiocap_callback(const ATK_AUDIO_NOTIFY_DATA_INFO_T *audio_info, void* user_data)
{
	UNUSED(user_data);
	if(!isRunning)
		return ;

	if(audio_info->dwDataBytes > 0) {
		
		if (savefile){
			//append data to a file
			write_to_file(audio_info->ppbyAudioBufs[0], audio_info->dwDataBytes);
		}
		
		unsigned int* buf_ptr = (unsigned int*)srb_writer_buf.buffer;
		buf_ptr[0] = audio_info->dwDataBytes;
		memcpy(srb_writer_buf.buffer + SRB_HEADER_SIZE, audio_info->ppbyAudioBufs[0], audio_info->dwDataBytes);
		SRB_SendGetWriterBuff(writer_handle, &srb_writer_buf);
		
	}
	else {
		printf("[%s,%d] sending data to client failed, send bytes: %d, terminating ...\n", __func__,__LINE__, audio_info->dwDataBytes);
		isRunning = false;
	}
}

//playback buff
void* srb_reader(void *ptr __attribute__((unused)))
{
	if (!playback_handle){
		printf(" [%s,%d] playback handle init error \n",__func__,__LINE__);
		return NULL;
	}

	// Audio output buffer.
	unsigned char *pbyBufs[ATK_AUDIO_MAX_CHANNELS] = {NULL};
	unsigned int dwDataBytes;
	
	while(isRunning && !reset){	
		if(ATK_AudioPlay_PlayPeriodFrames(playback_handle, pbyBufs) < 0){
			fprintf(stderr, "[%s,%s] Can't get the audio output buffer..\n", __FILE__, __func__);
			isRunning = false;
			break;
		}
		
		if(SRB_ReturnReceiveReaderBuff(reader_handle, &srb_reader_buf)==0){
			unsigned int* buf_ptr =  (unsigned int*)(srb_reader_buf.buffer);
			dwDataBytes = buf_ptr[0];
			memcpy(pbyBufs[0],srb_reader_buf.buffer + SRB_HEADER_SIZE,dwDataBytes);
		}
	}
	return NULL;
}

static void sig_kill(int signo)
{
	printf("[%s,%d] Received SIGNAL %d!!!\n", __func__,__LINE__, signo);
	switch(signo){
		case SIGTERM:
		case SIGINT:
			isRunning = false;
			SRB_WakeupReader(reader_handle);
			break;
		default:
			break;
	}
}
//======================================

int main(int argc, char* argv[])
{
	int ret;
	int opt = -1;
	int input = 0; //Mic
	unsigned int interleaved = 1;  // is playing interleaved audio
	unsigned int sample_rate = 8000; // audio sample rate
	unsigned int channels = 2 ;  //number of channels
	int simple_conf = 0;  //use_simple_config
	unsigned int periods = 8; //periods_per_buffer
	unsigned int device = 0; //sound device number 0 normal ; 1 usb
	const char card[] = "hw:0,0";
	char as_on[20] = {'0'};
	signed short fd_as;
	pthread_t tid = 0;
	while ((opt = getopt(argc, argv, "i:r:s:f:t:c:o:p:d:")) != -1){

		switch(opt){
			case 'r':
				sample_rate = atoi(optarg);
				break;
			case 'i': //Input selection 0 MicIn / 1 LineIn
				input = atoi(optarg);//strdup(optarg);
				break;
			case 's':
				savefile = atoi(optarg);
				break;
			case 'f':
				file_name = strdup(optarg);
				break;
			case 't':
				interleaved = atoi(optarg);
				break;
			case 'c':
				channels = atoi(optarg);
				break;
			case 'o':
				simple_conf = atoi(optarg);
				break;
			case 'p':
				periods = atoi(optarg);
				break;
			case 'd':
				device = atoi(optarg);
				break;
			default:
				print_usage(APP_NAME);
				goto end;
				break;
		}
	}

	if (savefile && NULL == file_name){
		file_name = "audio_lookback.wav";
	}
    if (input > 1 || input < 0){
		printf(" [%s,%d] input value is either 0=Mic or 1=LineIn \n",__func__,__LINE__);
		goto end;
	} else{
		ret = input? ATK_Audio_InputSelection(kTKAudioLineIn):ATK_Audio_InputSelection(kTKAudioMicIn);
		if (ret){
			printf(" [%s,%d] input selection error \n",__func__,__LINE__);
			goto end;
		}
	}

	if (channels > 2){
		printf(" [%s,%d] invalid channels number... Using default value! \n",__func__,__LINE__);
		channels = 2;
	}

	//open FD to get sample rate
	fd_as = open("/sys/class/webcam/uac/as_stream",O_RDONLY);
	if(fd_as < 0){
		printf("Cannot read sample rate form UAC \n");
		goto end;
	}

	ATK_AUDIOCAP_CONFIG_T capture_config; //Capture
	ATK_AUDIOPLAY_CONFIG_T playback_config;//Playback

	//initial configuration
	memset(&playback_config,0,sizeof(ATK_AUDIOPLAY_CONFIG_T));
	memset(&capture_config,0,sizeof(ATK_AUDIOCAP_CONFIG_T));
	//Playback
	sprintf((char*)card,"hw:%d,0",device);
    playback_config.szPcmName = card;
    playback_config.bIsInterleaved = interleaved;
    playback_config.eFormat = SND_PCM_FORMAT_S16_LE;
    playback_config.dwChannelsCount = channels;
    playback_config.dwSampleRate = sample_rate;
    playback_config.bUseSimpleConfig = simple_conf;
    playback_config.dwPeriodsPerBuffer = periods;
    playback_config.dwPeriodSizeInFrames = PERIOD_SIZE_IN_FRAMES;
    playback_config.bDropFramesBeforeStop = TRUE;
	//Capture
    capture_config.szPcmName = "hw:0,0";
    capture_config.bIsInterleaved = interleaved;
    capture_config.eFormat = SND_PCM_FORMAT_S16_LE;
    capture_config.dwChannelsCount = channels;
    capture_config.dwSampleRate = sample_rate;
    capture_config.bUseSimpleConfig = simple_conf;
    capture_config.dwPeriodsPerBuffer = periods;
    capture_config.dwPeriodSizeInFrames = PERIOD_SIZE_IN_FRAMES;
    capture_config.pfnCallback = audiocap_callback;
    capture_config.pUserData = NULL;
	//save audio to file...
	if(savefile){
		snprintf(filename, sizeof(filename), CAP_DIR "%s", file_name);
		fp = fopen(filename, "ab");
	}

    signal(SIGPIPE, SIG_IGN);
    signal(SIGTERM, sig_kill);
    signal(SIGINT, sig_kill);

    isRunning = true;
    while(isRunning){

		while (as_on[0] == '0' || reset == 0)
		{
			struct pollfd p;
			int ret;


			#if 1
			p.fd = fd_as;
			p.events = POLLERR | POLLPRI;
			p.revents = 0;
			ret = poll(&p,1,-1);
			if (ret<0)
				printf("Error poll failed\n");
			else if (ret == 0)
				printf("Error poll timeout \n");
			else if (p.revents & (POLLERR | POLLPRI)) {
				if((ret = pread(fd_as,&as_on,sizeof(as_on),0))<0) {
					printf("error poll read failed \n");
				}else{
					reset = 1;
					if (as_on[0] == '0') {
						continue;
					}
				}
			}
			#else
			fd_set efds;
			FD_ZERO(&efds);
		
			FD_SET(fd_as, &efds);
			
			ret = select(fd_as + 1, NULL, NULL, &efds, NULL);
			
			if (-1 == ret) {
				printf("select error %d, %s\n",
						errno, strerror (errno));
				if (EINTR == errno)
					continue;
			}
			
			
			if (FD_ISSET(fd_as, &efds)) {
				if((ret = pread(fd_as,&as_on,sizeof(as_on),0))<0) {
					printf("error read failed \n");
				}else{
					reset = 1;
					if (as_on[0] == '0') 
						continue;
				}
			}
			
			#endif
			

			if (tid != 0) {
					SRB_WakeupReader(reader_handle);
				    pthread_join(tid, NULL);
					tid = 0;
			}

			//release playabck and capture change state to 0
			if (capture_handle)
				ATK_AudioCap_Release(capture_handle);

			if (playback_handle)
				ATK_AudioPlay_Release(playback_handle);
			
			//release srb handles
			if(writer_handle)
				SRB_Release(writer_handle);
			if(reader_handle)
				SRB_Release(reader_handle);

		}
		
		
		if(reset == 1)
		{
			sample_rate = atoi(as_on);
			playback_config.dwSampleRate = sample_rate;
			capture_config.dwSampleRate = sample_rate;
			reset = 0;

			writer_handle = SRB_InitWriter("srb_uac", MAX_RING_BUF_SIZE, 4);
			if (!writer_handle){
				printf(" [%s,%d] srb writer handle init error \n",__func__,__LINE__);
				isRunning = false;
				break;
			}
			SRB_SendGetWriterBuff(writer_handle, &srb_writer_buf);
			
			reader_handle = SRB_InitReader("srb_uac");
			if (!reader_handle){
				printf(" [%s,%d] srb reader handle init error \n",__func__,__LINE__);
				isRunning = false;
				break;
			}


			capture_handle = ATK_AudioCap_Init(&capture_config);
			if(!capture_handle){
				printf(" [%s,%d] ATK_AudioCap_Init error \n",__func__,__LINE__);
				isRunning = false;
				break;
			}


			playback_handle = ATK_AudioPlay_Init(&playback_config);
			if(!playback_handle){
				printf(" [%s,%d] ATK_AudioPlay_Init error \n",__func__,__LINE__);
				isRunning = false;
				break;
			}
			pthread_create(&tid, NULL, srb_reader, NULL);
			printf(" [%s,%d] pthread_create tid = %lu sample_rate = %d\n",__func__,__LINE__,tid, sample_rate);
		}

	}

    isRunning = false;
    pthread_join(tid, NULL);
    printf(" [%s,%d] Leaving the program \n",__func__,__LINE__);
end:
	if(savefile)
		fclose(fp);
	if(NULL!=writer_handle)
		SRB_Release(writer_handle);
	if(NULL!=reader_handle)
		SRB_Release(reader_handle);
	if(NULL!=playback_handle)
		ATK_AudioPlay_Release(playback_handle);
	if(NULL!=capture_handle)
		ATK_AudioCap_Release(capture_handle);

}
