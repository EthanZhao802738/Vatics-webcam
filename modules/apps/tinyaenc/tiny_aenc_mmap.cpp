
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
#include <string>

#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include <audiotk/audio_capture_mmap.h>
#include <audiotk/audio_vol_ctrl.h>
#include <audiotk/audio_encoder.h>
#include <MsgBroker/msg_broker.h>
#include <SyncRingBuffer/sync_ring_buffer.h>

#define MAX_CONNECT_NUM (5)
#define DEFAULT_PID (0xffff)

#ifdef GAMR_SUPPORT
#include <opencore/interf_enc.h> //! For GAMR encoder.
#endif

#ifdef SPKR
#include <vmf/spkr.h>
#include "aenc_msg_format.h"
#include <MsgBroker/msg_video.h>

#define VENC_FIFO 	"/tmp/venc/c0/command.fifo"

/*======Copy from streamer_msg_format.h======*/

#define MSG_SRC_SET_OSD 			"setOSD"


/*======Copy from streamer_msg_format.h======*/
#endif	




// Flag for message broker (main loop).
static int is_terminate_ = 0;

typedef enum 
{
	SPKR_STOP = 0,
	SPKR_ENROLL = 1,
	SPKR_VERIFY = 2,
	SPKR_RESET = 3,	
}SPKR_STATE;

typedef struct
{
	//! connect pid
	pid_t connect_pid;
	//! Flag to indicate whether we need to encode data or not.
	unsigned int do_encoding;
} connect_info_t;

connect_info_t g_atconnect_info[MAX_CONNECT_NUM];

typedef struct
{
	// Flag to indicate whether we need to send configuration about each encoder or not.
	bool send_conf;

	// The handles for SynRingBuf.
	srb_handle_t* srb_handle;
	// The buffers for SynRingBuf.
	srb_buffer_t srb_buf;

	unsigned int enc_type; // FourCC of encoder.
	ATK_AUDIOENC_HANDLE_T* enc_handle; // The handle of encoder.

	unsigned int seq_num;

	ATK_AUDIOENC_ONEFRAME_CONF_T oneframe_conf;
#ifdef SPKR
	VMF_SPKR_HANDLE_T* spkr_handle;	
	SPKR_STATE spkr_state;
	short spkr_threshold;
	int spkr_index;
	unsigned int target_samples;
	unsigned int current_samples;
	unsigned int period_size_in_frames;
	short* spkr_buffer;
	bool start_capture;
	unsigned int target_loop;
	unsigned int current_loop;
	
#endif	
	pthread_mutex_t data_mutex;
	pthread_cond_t data_cond;
	STATUS process_status;

	ATK_AUDIOCAP_CONFIG_T	*p_audiocap_config;
	ATK_AUDIOCAP_HANDLE_T **p_cap_handle;
} user_data_t;

//#define DUMP_TIMESTAMP
//#define DUMP_PCM_DATA

#ifdef DUMP_PCM_DATA
static int audio_fd_ = -1;

static int open_pcm_file(/* int is_interleaved, unsigned int channels */)
{
	const char *filename = "/tmp/vrecord/videoclips/debug_audio.pcm";
	audio_fd_ = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	if(audio_fd_ == -1)
	{
		fprintf(stderr, "[%s, %s]: Open file error: %s\n", __FILE__, __func__, strerror(errno));
		return -1;
	}

	return 0;
}

static void close_pcm_file()
{
	if(audio_fd_ >= 0)
		close(audio_fd_);
}

static int write_pcm_data_to_file(int /*is_interleaved*/, unsigned int /*channels*/, unsigned char* const* audio_bufs, size_t data_bytes)
{
	ssize_t ret = write(audio_fd_, audio_bufs[0], data_bytes);
	if(ret < 0)
	{
		fprintf(stderr, "[%s, %s]: Write error. %s\n", __FILE__, __func__, strerror(errno));
		return -1;
	}
	else if((size_t) ret != data_bytes)
	{
		fprintf(stderr, "[%s, %s]: Data loss ..............\n", __FILE__, __func__);
		return -1;
	}
	return 0;
}

#endif // DUMP_PCM_DATA

#ifdef SPKR
static MsgContext msg_context;
static MsgContext msg_face;
static msg_osd_t osd_msg;

//#define RECOED_SPKR
#ifdef RECOED_SPKR
static char reg[50] = {0};
static char detect[50] = {0};
static int detect_id = 0;
static int audio_fd_ = -1;
static int audio_detect = -1;
#endif

static void do_spkr(user_data_t* user_data, short* audio_buf)
{
	if(!user_data->start_capture) {
		user_data->current_samples = 0;
		user_data->current_loop = 0;
		user_data->start_capture = true;		
	}
	
	if(user_data->current_samples < user_data->target_samples) {
		for (unsigned int i=0; i < user_data->period_size_in_frames; i++) {				
			user_data->spkr_buffer[user_data->current_samples] = audio_buf[i << 1];				
			user_data->current_samples++;
			if(user_data->current_samples == user_data->target_samples) {				
				if(SPKR_ENROLL == user_data->spkr_state) {
					VMF_SPKR_ENR_T enr;
					enr.dwIndex = user_data->spkr_index;
					enr.ptInputBuffer = (unsigned char*)user_data->spkr_buffer;					
					if(0 != VMF_SPKR_Enroll(user_data->spkr_handle, &enr))
						printf("[%s] VMF_SPKR_Enroll fail\n", __func__);					
#ifdef RECOED_SPKR
					else {						
						size_t data_bytes = user_data->current_samples*2;
					
						if(audio_fd_ == -1)
							printf("open spkr_audio.pcm fail\n");
						else {
							ssize_t ret = write(audio_fd_, user_data->spkr_buffer, data_bytes);
							if(ret < 0)
							{
								fprintf(stderr, "[%s, %s]: Write error. %s\n", __FILE__, __func__, strerror(errno));
							}
							else if((size_t) ret != data_bytes)
							{
								fprintf(stderr, "[%s, %s]: Data loss ..............\n", __FILE__, __func__);
							}
						}
					}
#endif					
				} 
				else if(SPKR_VERIFY == user_data->spkr_state) {
					VMF_SPKR_REC_T rec;
					rec.bDetected = 0;
					rec.ptInputBuffer = (unsigned char*)user_data->spkr_buffer;
					if(0 != VMF_SPKR_Recognize(user_data->spkr_handle, &rec)) {
						printf("[%s] VMF_SPKR_Recognize fail\n", __func__);
					}
#ifdef RECOED_SPKR					
					else {
						size_t data_bytes = user_data->current_samples*2;
						
						if(audio_detect == -1)
							printf("open spkr_detect_audio.pcm fail\n");
						else {
							ssize_t ret = write(audio_detect, user_data->spkr_buffer, data_bytes);
							if(ret < 0)
							{
								fprintf(stderr, "[%s, %s]: Write error. %s\n", __FILE__, __func__, strerror(errno));
							}
							else if((size_t) ret != data_bytes)
							{
								fprintf(stderr, "[%s, %s]: Data loss ..............\n", __FILE__, __func__);
							}
						}

					}
#endif					
					if(rec.bDetected) {
						user_data->spkr_state = SPKR_STOP;
						user_data->start_capture = false;						
						unsigned int face_action = 1;
						char cmd[10] = {0};

						snprintf(cmd, 10, "face%d", rec.dwDetectedIndex);
						msg_face.pszCmd = cmd;
						msg_face.dwCmdLen = strlen(msg_face.pszCmd) + 1;						

						msg_face.dwDataSize = sizeof(unsigned int);
						msg_face.pbyData    = (unsigned char*)&face_action;
						MsgBroker_SendMsg(VENC_FIFO, &msg_face);
#ifdef RECOED_SPKR				
						if(audio_detect >= 0) {
							close(audio_detect);
							audio_detect = -1;
							detect_id++;
						}
#endif
					}
				}
				user_data->current_samples = 0;
				if(SPKR_STOP != user_data->spkr_state)
					continue;
				break;
			}
		}
	}	
}
#endif //SPKR
static int pid_default(int dwStart)
{	
	unsigned int i;
	for (i = 0; i < MAX_CONNECT_NUM; ++i)
	{
		if (dwStart) {
			//! set default pid and start encoding 
			if (!g_atconnect_info[i].connect_pid) {
				g_atconnect_info[i].connect_pid = DEFAULT_PID;
				++g_atconnect_info[i].do_encoding;
				break;
			} else if (g_atconnect_info[i].connect_pid == DEFAULT_PID){
				++g_atconnect_info[i].do_encoding;
				break;
			} else {
				printf("[%s] Error: Something wrong \n", __func__);
				break;
			}
		} else {
			//! set default pid and stop encoding
			if (g_atconnect_info[i].connect_pid == DEFAULT_PID) {
				--g_atconnect_info[i].do_encoding;
				if (!g_atconnect_info[i].do_encoding) {
					g_atconnect_info[i].connect_pid = 0;
				}
				break;
			}
		}
	}

	if(i == MAX_CONNECT_NUM){
		printf("[%s] Error: AENC Connect number to MAX\n",__func__);
		return 1;
	}
	return 0;
}	

static int pid_start (pid_t new_pid)
{
	unsigned int i = 0;
	for (i = 0; i < MAX_CONNECT_NUM; i++) 
	{
		if (g_atconnect_info[i].connect_pid == 0 && !g_atconnect_info[i].do_encoding) {	
			g_atconnect_info[i].connect_pid = new_pid;
			g_atconnect_info[i].do_encoding++;
			break;
		} else if (g_atconnect_info[i].connect_pid == new_pid) {
			g_atconnect_info[i].do_encoding++;
			break;
		}
	}

	if(i == MAX_CONNECT_NUM){
		printf("[%s] Error: AENC Connect number to MAX\n",__func__);
		return 1;
	}
		
	return 0;
}

static int pid_stop(pid_t new_pid)
{
	for (size_t i = 0; i < MAX_CONNECT_NUM; i++) 
	{
		if (g_atconnect_info[i].connect_pid != 0
			&& g_atconnect_info[i].connect_pid == new_pid
			&& g_atconnect_info[i].do_encoding) {	
			g_atconnect_info[i].do_encoding--;
			if (!g_atconnect_info[i].do_encoding)
				g_atconnect_info[i].connect_pid = 0;
			break;
		} 
	}

	return 0;
}

static int pid_do_encoding(void)
{
	for (size_t i = 0; i < MAX_CONNECT_NUM; i++) 
	{
		if (g_atconnect_info[i].connect_pid != 0 && g_atconnect_info[i].do_encoding) 
			return 1;
	}
	return 0;
}


static void audiocap_callback(const ATK_AUDIO_NOTIFY_DATA_INFO_T *audio_info, void* user_data)
{
	//Input pcm data should be interleaved
	user_data_t *temp_data = (user_data_t*) user_data;

#ifdef DUMP_TIMESTAMP
	printf("time = %lu.%lu\n", audio_info->tDataTimestamp.tv_sec, audio_info->tDataTimestamp.tv_usec);
#endif // DUMP_TIMESTAMP

#ifdef DUMP_PCM_DATA
	write_pcm_data_to_file(audio_info->bIsInterleaved, audio_info->dwChannels, audio_info->ppbyAudioBufs, audio_info->dwDataBytes);
#endif
	pthread_mutex_lock(&(temp_data->data_mutex));	
#ifdef SPKR	
	
	if(SPKR_RESET == temp_data->spkr_state) {
		VMF_SPKR_Reset(temp_data->spkr_handle);
		temp_data->spkr_state = SPKR_STOP;
	}	
	else if(SPKR_STOP != temp_data->spkr_state) {
		do_spkr(temp_data, (short*)(audio_info->ppbyAudioBufs[0]));
	}
	
#endif	

	if (temp_data->send_conf)
	{
		temp_data->send_conf = false;
		// Send configuration of encoder.
		printf("send conf .............\n");
		if(FOURCC_GPCM != temp_data->enc_type) {
			ATK_AudioEnc_GetConf(temp_data->enc_handle, temp_data->srb_buf.buffer);
		}
		else {
			unsigned int *values = (unsigned int*) (temp_data->srb_buf.buffer);
			values[0] = FOURCC_CONF;
			values[1] = 4;
			values[2] = FOURCC_GPCM;	
		}
		SRB_SendGetWriterBuff(temp_data->srb_handle, &temp_data->srb_buf);
	}

	
	if (temp_data->process_status == STOP) {
		pthread_cond_signal(&(temp_data->data_cond));	
		pthread_mutex_unlock(&(temp_data->data_mutex));	
		return;
	}
	pthread_mutex_unlock(&(temp_data->data_mutex));	
	
	if (pid_do_encoding())
	{
		if(FOURCC_GPCM == temp_data->enc_type)
		{
			if(audio_info->dwDataBytes > 0)
			{
				unsigned int* buf_ptr = (unsigned int*) temp_data->srb_buf.buffer;
				buf_ptr[0] = temp_data->enc_type;
				buf_ptr[1] = audio_info->tDataTimestamp.tv_sec;
				buf_ptr[2] = audio_info->tDataTimestamp.tv_usec;
				buf_ptr[3] = audio_info->dwDataBytes;
				buf_ptr[4] = audio_info->bIsInterleaved;
				buf_ptr[5] = audio_info->dwChannels;

				memcpy(temp_data->srb_buf.buffer + MAX_AUDIO_DATA_HEADER_SIZE, audio_info->ppbyAudioBufs[0], audio_info->dwDataBytes);
				SRB_SendGetWriterBuff(temp_data->srb_handle, &temp_data->srb_buf);
			}
			else
			{
				fprintf(stderr, "[%s, %s]: PCM error !!!\n", __FILE__, __func__);
			}
		}
		else
		{
			// Encode audio frames.
			temp_data->oneframe_conf.pbyInBuf = audio_info->ppbyAudioBufs[0];
			temp_data->oneframe_conf.pbyOutBuf = temp_data->srb_buf.buffer + MAX_AUDIO_DATA_HEADER_SIZE;

			//time_t nowtime = time(NULL);
			//printf("[%s] (%d) audio_info->data_bytes = %d !!!!!!!!!!!!!!!!!!!!!!!! \n", __func__, nowtime, audio_info->data_bytes);

			int encode_data_size = ATK_AudioEnc_EncodeOneFrame(temp_data->enc_handle, &temp_data->oneframe_conf);
			if(encode_data_size > 0)
			{
				unsigned int* buf_ptr = (unsigned int*) temp_data->srb_buf.buffer;
				buf_ptr[0] = temp_data->enc_type;
				buf_ptr[1] = audio_info->tDataTimestamp.tv_sec;
				buf_ptr[2] = audio_info->tDataTimestamp.tv_usec;
				buf_ptr[3] = encode_data_size;
				buf_ptr[4] = temp_data->seq_num;

				SRB_SendGetWriterBuff(temp_data->srb_handle, &temp_data->srb_buf);
			}
			else if(encode_data_size < 0)
			{
				fprintf(stderr, "[%s, %s]: Encode error !!!\n", __FILE__, __func__);
			}
			++(temp_data->seq_num);
		}
	}
}

static void msg_callback(MsgContext* msg, void* user_data)
{
	user_data_t *temp_data = (user_data_t*) user_data;

	if (!strncasecmp(msg->pszHost, "encoder", 7))
	{
		if (!strcasecmp(msg->pszCmd, "start"))
		{
			if (msg->dwDataSize) {
				pid_t *ptNew_pid =  (pid_t *)msg->pbyData;
				pid_start(*ptNew_pid);
			} else {
				pid_default(1);
			}
			
			printf("start .............. \n");
		}
		else if (!strcasecmp(msg->pszCmd, "stop"))
		{
			
			if (msg->dwDataSize) {
				pid_t *ptNew_pid =  (pid_t *)msg->pbyData;
				pid_stop(*ptNew_pid);
			} else {
				pid_default(0);
			}
			
			printf("stop .............. \n");
		}
		else if (!strcasecmp(msg->pszCmd, "forceCI"))
		{
			printf("forceCI ..............\n");
			temp_data->send_conf = true;
		}
	}
	else if( !strcasecmp(msg->pszHost, SR_MODULE_NAME) ){
		if( !strcasecmp(msg->pszCmd, SUSPEND_CMD) ) {
			pthread_mutex_lock(&(temp_data->data_mutex));
			temp_data->process_status = STOP;
			pthread_cond_wait(&(temp_data->data_cond), &(temp_data->data_mutex));
			pthread_mutex_unlock(&(temp_data->data_mutex));
			ATK_AudioCap_Release(*(temp_data->p_cap_handle));
			MsgBroker_SuspendAckMsg();
		}else if( !strcasecmp(msg->pszCmd, RESUME_CMD) ) {
			*(temp_data->p_cap_handle) = ATK_AudioCap_Init(temp_data->p_audiocap_config);
			pthread_mutex_lock(&(temp_data->data_mutex));
			temp_data->process_status = START;
			pthread_mutex_unlock(&(temp_data->data_mutex));
		}
	}
#ifdef SPKR	
	else if (!strncasecmp(msg->pszHost, "spkr", 4))
	{
		printf("speaker enabled!!! \n "); //PEDRO
		if (!strcasecmp(msg->pszCmd, MSG_SPKR_ENROLL))
		{
			Speak_info_t *ptSpeak_info = (Speak_info_t *)(msg->pbyData);
			int speaker_index = ptSpeak_info->speaker_index;
			pthread_mutex_lock(&(temp_data->data_mutex));
			pid_t new_pid = ptSpeak_info->speak_pid;
			pid_start(new_pid);
			temp_data->spkr_index = speaker_index;
			temp_data->spkr_state = SPKR_ENROLL;
			temp_data->start_capture = false;
#ifdef RECOED_SPKR				
			if(audio_detect >= 0) {
				close(audio_detect);
				audio_detect = -1;
				detect_id++;
			}
			snprintf( reg, 50, "%s%d%s", "spkr_audio_", temp_data->spkr_index, ".pcm");
			audio_fd_ = open(reg, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
#endif				
			pthread_mutex_unlock(&(temp_data->data_mutex));
			snprintf(osd_msg.str, OSD_STR_SIZE, "Please speak keyword to register speaker %d.", temp_data->spkr_index);
			MsgBroker_SendMsg(VENC_FIFO, &msg_context);		
			printf("[SPKR] Enroll speaker %d\n", speaker_index);
		}
		else if (!strcasecmp(msg->pszCmd, MSG_SPKR_VERIFY))
		{
			snprintf(osd_msg.str, OSD_STR_SIZE, "Recognizing...");
			MsgBroker_SendMsg(VENC_FIFO, &msg_context);
			pthread_mutex_lock(&(temp_data->data_mutex));
			pid_t *ptNew_pid =  (pid_t *)msg->pbyData;
			pid_start(*ptNew_pid);
			temp_data->spkr_state = SPKR_VERIFY;
#ifdef RECOED_SPKR				
			if(audio_fd_ >= 0) {
				close(audio_fd_);	
				audio_fd_ = -1;			
			}
			snprintf( detect, 50, "%s%d%s", "spkr_detect_audio_", detect_id, ".pcm");
			audio_detect = open(detect, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);	
#endif			
			pthread_mutex_unlock(&(temp_data->data_mutex));
		}
		else if (!strcasecmp(msg->pszCmd, MSG_SPKR_STOP))
		{
			pthread_mutex_lock(&(temp_data->data_mutex));
			temp_data->spkr_state = SPKR_STOP;
			temp_data->start_capture = false;
#ifdef RECOED_SPKR				
			if(audio_fd_ >= 0) {
				close(audio_fd_);	
				audio_fd_ = -1;			
			}
			if(audio_detect >= 0) {
				close(audio_detect);
				audio_detect = -1;
				detect_id++;
			}
#endif
			pthread_mutex_unlock(&(temp_data->data_mutex));
		}
		else if (!strcasecmp(msg->pszCmd, MSG_SPKR_THRESHOLD))
		{
			int* threshold = (int*)(msg->pbyData);
			pthread_mutex_lock(&(temp_data->data_mutex));			
			VMF_SPKR_SetThreshold(temp_data->spkr_handle,*threshold);
			pthread_mutex_unlock(&(temp_data->data_mutex));
		}
		else if (!strcasecmp(msg->pszCmd, MSG_SPKR_RESET))
		{	
			pthread_mutex_lock(&(temp_data->data_mutex));		
			temp_data->spkr_state = SPKR_RESET;
			temp_data->start_capture = false;
#ifdef RECOED_SPKR				
			if(audio_fd_ >= 0) {
				close(audio_fd_);	
				audio_fd_ = -1;			
			}
			if(audio_detect >= 0) {
				close(audio_detect);
				audio_detect = -1;
				detect_id++;
			}
#endif			
			pthread_mutex_unlock(&(temp_data->data_mutex));
		}
	}
#endif
	if (msg->bHasResponse)
		msg->dwDataSize = 0;
}

static void exit_process()
{
	is_terminate_ = 1;
}

static void sig_kill(int signo)
{
	fprintf(stderr, "[%s,%s] Receive SIGNAL %d!!!\n", __FILE__, __func__, signo);
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

static void print_usage(const char *ap_name)
{
	fprintf(stderr, "Usage:\n"
			"    %s -d PCM_device_name -r sample_rate -C codec_type [-b bit_rate] [-m compression_mode] [-S aac_stereo_mode] [-f command_FIFO_path] [-i input_type] [-s spkr_enable] [-T spkr_threshold] [-D][-h]\n"
			"Options:\n"
			"    -d                 PCM device name for ALSA (ex: hw:0,0).\n"
			"    -r                 Sample rate for audio.\n"
			"    -C                 Codec type for encoder (0: AAC4, 1: G711, 2: G726).\n"
			"    -b                 Bit rate for audio encoder (AAC4: 8000 ~ 320000, G726: 16000, 24000, 32000, 40000, G711: 64000).\n"
			"                       (GAMR:4750 ,5150 , 5900, 6700, 7400, 7950, 10200, 12200)\n"
			"    -m                 Compression mode for G711 (0: ulaw, 1: alaw).\n"
			"    -t                 Bitstream format for AAC (0: raw, 1: ADTS, 2: ADIF).\n"
			"    -S                 Stereo mode to control the desired output channel of AAC (0: stereo, 1: joint stereo, 3: mono).\n"
			"    -f                 The path of command FIFO.\n"
			"    -i                 Input type of audio (0: MicIn, 1: LineIn. Default: LineIn).\n"
			"    -D                 Run as Daemon.\n"
			"    -s                 Speaker recognition enable (0 disable, 1 enable).\n"
			"    -T                 Speaker recognition threshold(0-30000).\n"
			"    -h                 This help.\n", ap_name);

	fprintf(stderr, "ex:\n"
			"%s -d \"hw:0,0\" -r 8000 -C 0 -b 32000 -B \"aenc_srb_\" -s 1 -o 1 -f \"/tmp/aenc/c0/command.fifo\"\n"
			"%s -d \"hw:0,0\" -r 8000 -C 0 -b 32000 -t 0 -S 0\n"
			"%s -d \"hw:0,0\" -r 8000 -C 1 -b 64000 -m 0\n"
			"%s -d \"hw:0,0\" -r 8000 -C 2 -b 40000\n", ap_name, ap_name, ap_name, ap_name);
}

int main(int argc, char **argv)
{
	int opt;
	bool is_daemon = false;
	// Default setting.
	int codec_type = -1; // 0: AAC4, 1: G711, 2: G726 3: GAMR 4: GPCM
	int bit_rate = -1; // AAC4: 8000, 16000, 24000, 32000, G726: 16000, 24000, 32000, 40000, G711: 64000
	int compression_mode = -1; // 0: ulaw, 1: alaw
	int adts = 0; // 0: raw, 1: ADTS, 2: ADIF
	unsigned int aac_stereo_mode = 0; // 0: stereo, 1: joint stereo, 3: mono
	int input_type = 1; //0: MicIn, 1: LineIn
	std::string srb_name = "aenc_srb_1";
	std::string pcm_name = "hw:0,0";
	std::string cmd_fifo_path = CMD_FIFO_PATH;
	
#ifdef SPKR	
	short spkr_threshold = 7000;
	unsigned int spkr_enable = 0;
#endif

	ATK_AUDIOCAP_CONFIG_T audiocap_config;
	ATK_AUDIOCAP_HANDLE_T *cap_handle = NULL;
	memset(&audiocap_config, 0, sizeof(ATK_AUDIOCAP_CONFIG_T));

	audiocap_config.szPcmName = pcm_name.c_str();
	audiocap_config.bIsInterleaved = 1;
	audiocap_config.eFormat = SND_PCM_FORMAT_S16_LE;
	audiocap_config.dwChannelsCount = 2;
	audiocap_config.dwSampleRate = 8000;
	audiocap_config.dwPeriodsPerBuffer = 8;
	audiocap_config.dwPeriodSizeInFrames = PERIOD_SIZE_IN_FRAMES;
	audiocap_config.bUseSimpleConfig = 0;

	while ((opt = getopt(argc, argv, "Dhd:r:C:b:m:f:t:i:S:s:T:")) != -1)
	{
		switch(opt)
		{
			case 'd':
				pcm_name = optarg;
				audiocap_config.szPcmName = pcm_name.c_str();
				break;
			case 'r':
				audiocap_config.dwSampleRate = atoi(optarg);
				break;
			case 'C':
				codec_type = atoi(optarg);
				break;
			case 'b':
				bit_rate = atoi(optarg);
				break;
			case 'm':
				compression_mode = atoi(optarg);
				break;
			case 't':
				adts = atoi(optarg);
				break;
			case 'S':
				aac_stereo_mode = atoi(optarg);
				break;
			case 'f':
				cmd_fifo_path = optarg;
				break;
			case 'D':
				is_daemon = true;
				break;
			case 'i':
				input_type = (atoi(optarg) != 0)? 1:0;
				break;
#ifdef SPKR
			case 's':
				spkr_enable =  (atoi(optarg)!= 1)? 0:1;
				break;
			case 'T':
				spkr_threshold = atoi(optarg);
				break;
#endif					
			case 'h':
			default:
				print_usage(argv[0]);
				exit(EXIT_FAILURE);
		}
	}
	
	signal(SIGTERM, sig_kill);
	signal(SIGINT, sig_kill);

	if (is_daemon)
	{
		daemon(1,1);
	}

	if (input_type == 1)
	{
		ATK_Audio_InputSelection(kTKAudioLineIn);

		//set audio volume to 90
		ATK_Audio_SetCaptureVolume(90);

	}
	else
	{
		ATK_Audio_InputSelection(kTKAudioMicIn);

		//set audio volume to 90
		ATK_Audio_SetCaptureVolume(90);
	}

	// Reset the configurations for each encoder.
	ATK_AAC4ENC_CONFIG_T aac4_config;
	ATK_G711ENC_CONFIG_T g711_config;
	ATK_G726ENC_CONFIG_T g726_config;
	ATK_GAMRENC_CONFIG_T gamr_config;
	ATK_AUDIOENC_INITOPT_T audioenc_initopt;

	memset(&aac4_config, 0, sizeof(ATK_AAC4ENC_CONFIG_T));
	memset(&g711_config, 0, sizeof(ATK_G711ENC_CONFIG_T));
	memset(&g726_config, 0, sizeof(ATK_G726ENC_CONFIG_T));
	memset(&gamr_config, 0, sizeof(ATK_GAMRENC_CONFIG_T));
	memset(&audioenc_initopt, 0, sizeof(ATK_AUDIOENC_INITOPT_T));

	// Callbacks for audio capture and the private user data for callback.
	user_data_t user_data;
	memset(&user_data, 0, sizeof(user_data_t));
	user_data.send_conf = true;
	pthread_mutex_init(&(user_data.data_mutex), NULL); 
	pthread_cond_init(&(user_data.data_cond), NULL);
	user_data.process_status = START;
	user_data.p_audiocap_config = &audiocap_config;
	user_data.p_cap_handle = &cap_handle;

	audiocap_config.pfnCallback = audiocap_callback;
	audiocap_config.pUserData = (void*) (&user_data);


	audioenc_initopt.dwChannels = audiocap_config.dwChannelsCount;
	audioenc_initopt.bIsInterleaved = audiocap_config.bIsInterleaved;
	switch(codec_type)
	{
		case kAAC4: // AAC4
			if((bit_rate < 8000) || (bit_rate > 320000))
			{
				fprintf(stderr, "[%s,%s] AAC4 doesn't support bit rate = %d\n", __FILE__, __func__, bit_rate);
				return -1;
			}
			if( (audiocap_config.dwSampleRate < 8000) || (audiocap_config.dwSampleRate > 96000) )
			{
				fprintf(stderr, "[%s,%s] AAC doesn't support sample rate = %d\n", __FILE__, __func__, audiocap_config.dwSampleRate);
				return -1;
			}

			audioenc_initopt.eType = kAAC4;
			audioenc_initopt.dwPeriodSizeInFrames = PERIOD_SIZE_IN_FRAMES_AAC;
			aac4_config.dwBitRate = bit_rate; // 8000, 16000, 24000, 32000
			aac4_config.dwSampleRate = audiocap_config.dwSampleRate;
			aac4_config.dwAdts = adts;
			aac4_config.dwStereoMode = aac_stereo_mode;

			audiocap_config.dwPeriodSizeInFrames = PERIOD_SIZE_IN_FRAMES_AAC;
			user_data.oneframe_conf.dwOutBufSize = MAX_ENCODE_DATA_SIZE;

			user_data.enc_type = FOURCC_AAC4;
			user_data.enc_handle = ATK_AudioEnc_Init(&audioenc_initopt, &aac4_config);
			break;
		case kG711: // G711
			if((compression_mode != 0) && (compression_mode != 1))
			{
				fprintf(stderr, "[%s,%s] G711 doesn't support compression mode = %d\n", __FILE__, __func__, compression_mode);
				return -1;
			}
			if(bit_rate != 64000)
			{
				fprintf(stderr, "[%s,%s] G711 doesn't support bit rate = %d\n", __FILE__, __func__, bit_rate);
				return -1;
			}
			if(audiocap_config.dwSampleRate != 8000)
			{
				fprintf(stderr, "[%s,%s] G711 doesn't support sample rate = %d\n", __FILE__, __func__, audiocap_config.dwSampleRate);
				return -1;
			}
			audioenc_initopt.eType = kG711;
			audioenc_initopt.dwPeriodSizeInFrames = PERIOD_SIZE_IN_FRAMES;
			g711_config.iCompressionMode = compression_mode; /* It can be 0: ulaw or 1: alaw. */

			user_data.enc_type = FOURCC_G711;
			user_data.enc_handle = ATK_AudioEnc_Init(&audioenc_initopt, &g711_config);
			break;
		case kG726: // G726
			if((bit_rate != 16000) && (bit_rate != 24000) && (bit_rate != 32000) && (bit_rate != 40000))
			{
				fprintf(stderr, "[%s,%s] G726 doesn't support bit rate = %d\n", __FILE__, __func__, bit_rate);
				return -1;
			}
			if(audiocap_config.dwSampleRate != 8000)
			{
				fprintf(stderr, "[%s,%s] G726 doesn't support sample rate = %d\n", __FILE__, __func__, audiocap_config.dwSampleRate);
				return -1;
			}
			audioenc_initopt.eType = kG726;
			audioenc_initopt.dwPeriodSizeInFrames = PERIOD_SIZE_IN_FRAMES;
			g726_config.dwBitRate = bit_rate; // 16000, 24000, 32000, 40000

			user_data.enc_type = FOURCC_G726;
			user_data.enc_handle = ATK_AudioEnc_Init(&audioenc_initopt, &g726_config);
			break;
		case kGAMR:
#ifdef GAMR_SUPPORT
			if(audiocap_config.dwSampleRate != 8000)
			{
				fprintf(stderr, "[%s,%s] GAMR doesn't support sample rate = %d\n", __FILE__, __func__, audiocap_config.dwSampleRate);
				return -1;
			}
			audiocap_config.dwPeriodSizeInFrames = PERIOD_SIZE_IN_FRAMES_GAMR;
			{
				if(-1 == bit_rate) {
					bit_rate = 4750;
					gamr_config.dwMode = 0;
				}
				else {			
					int gamr_bitrate[] = {4750 ,5150 , 5900, 6700, 7400, 7950, 10200, 12200};
					int ret = -1;
					for(unsigned int i=0;i<8;i++) {
						if(bit_rate == gamr_bitrate[i]) {
							ret = 0;
							gamr_config.dwMode = i;
							break;
						}
					}				
					if(-1 == ret) {
						fprintf(stderr, "[%s,%s] GAMR doesn't support bitrate = %d\n", __FILE__, __func__, bit_rate);
						return -1;					
					}
				}
				//bit_rate = gamr_bitrate[MR122];
			}
			printf("[%s] GAMR bitrate: %d\n", __func__, bit_rate);
			audioenc_initopt.eType = kGAMR;
			audioenc_initopt.dwPeriodSizeInFrames = PERIOD_SIZE_IN_FRAMES_GAMR;
			gamr_config.dwBitRate = bit_rate;
			gamr_config.dwSampleRate = audiocap_config.dwSampleRate;

			user_data.enc_type = FOURCC_GAMR;
			user_data.enc_handle = ATK_AudioEnc_Init(&audioenc_initopt, &gamr_config);
#else
			fprintf(stderr, "[%s,%s] GAMR doesn't be supported, please rebuild sdk with GAMR_SUPPORT definition\n", __FILE__, __func__);
#endif
			break;
		case kGPCM:
			user_data.enc_type = FOURCC_GPCM;
			user_data.enc_handle = NULL;
			break;
		default:
			printf("Unknown codec type.\n");
			return -1;
	}

	// Check encoder is initialized
	if(codec_type != kGPCM && user_data.enc_handle == NULL)
	{
		goto main_end;
	}

	//SynRingBuffer
	user_data.srb_handle = SRB_InitWriter(srb_name.c_str(), MAX_RING_BUF_SIZE, 4);
	memset(&user_data.srb_buf, 0, sizeof(srb_buffer_t));
	// Get first buffer from SyncRingBuf.
	SRB_SendGetWriterBuff(user_data.srb_handle, &user_data.srb_buf);

	// Initialize the audio capture.
	cap_handle = ATK_AudioCap_Init(&audiocap_config);
	if(cap_handle == NULL)
	{
		fprintf(stderr, "[%s,%s] Can't initialize the audio capture.\n", __FILE__, __func__);
		goto main_end;
	}
#ifdef SPKR
	if(spkr_enable) {
		printf("[%s]Initialize SPKR handle!!.\n", __func__);
		VMF_SPKR_INITOPT_T spkr_init;
		spkr_init.dwSampleRate = audiocap_config.dwSampleRate;
		spkr_init.sThreshold = user_data.spkr_threshold = spkr_threshold;		
		spkr_init.dwSampleTimeInMs = 100;
		user_data.current_loop = 0;
	
		user_data.spkr_handle = VMF_SPKR_Init(&spkr_init);
		user_data.target_samples = audiocap_config.dwSampleRate/10;//(audiocap_config.dwSampleRate/1000)*100
		user_data.spkr_buffer = (short*)malloc(sizeof(short)*user_data.target_samples);
		user_data.spkr_state = SPKR_STOP;
		user_data.period_size_in_frames = audiocap_config.dwPeriodSizeInFrames;
		user_data.start_capture = false;
		
		
		memset(&osd_msg, 0, sizeof(osd_msg));
		osd_msg.output_index = 0;			
		msg_context.bHasResponse = 0;

		msg_context.pszHost = "source0";
		msg_context.pszCmd = MSG_SRC_SET_OSD;
		msg_context.dwHostLen = strlen(msg_context.pszHost) + 1;
		msg_context.dwCmdLen = strlen(msg_context.pszCmd) + 1;
		msg_context.dwDataSize = sizeof(msg_osd_t);
		msg_context.pbyData    = (unsigned char*)&osd_msg;

		msg_face.pszHost = "source0";
		msg_face.dwHostLen = strlen(msg_face.pszHost) + 1;
		msg_face.bHasResponse = 0;
	}
	
#endif
#ifdef DUMP_PCM_DATA
	if(open_pcm_file() < 0)
	{
		goto main_end;
	}
#endif
	MsgBroker_RegisterMsg(cmd_fifo_path.c_str());
	// Enter the main message loop.
	MsgBroker_Run(cmd_fifo_path.c_str(), msg_callback, &user_data, &is_terminate_);
	MsgBroker_UnRegisterMsg();

main_end:
	ATK_AudioCap_Release(cap_handle);
	ATK_AudioEnc_Release(user_data.enc_handle);
	SRB_Release(user_data.srb_handle);
	pthread_mutex_destroy(&(user_data.data_mutex));
	pthread_cond_destroy(&(user_data.data_cond));
#ifdef DUMP_PCM_DATA
	close_pcm_file();
#endif
#ifdef SPKR
	if(user_data.spkr_handle) VMF_SPKR_Release(user_data.spkr_handle);
	if(user_data.spkr_buffer) free(user_data.spkr_buffer);
	
#endif	

	return 0;
}
