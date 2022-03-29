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

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>

#include <audiotk/audio_playback_mmap.h>
#include <MsgBroker/msg_broker.h>
#include <SyncRingBuffer/sync_ring_buffer.h>
#include <audiotk/audio_common.h>
#include <g711/G711SDec.h> // For G711 decoder.
#include <g726/G726SDec.h> // For G726 decoder.

#ifdef AAC_SW_DEC
#include <audiotk/fdk_aac_processor.h> // For AAC4 decoder.
#endif
#ifdef GAMR_SUPPORT
#include <opencore/interf_dec.h> //! For GAMR decoder.
#endif
#define S16_BYTES_PER_SAMPLE					2

//#define DUMP_PCM_DATA

static int g_adts = 0; // 0: raw, 1: ADTS, 2: ADIF

#define CMD_PLAY_FIFO_PATH "/tmp/playback/c0/command.fifo"

typedef struct {
	pthread_mutex_t data_mutex;
	pthread_cond_t data_cond;

	//pthread_mutex_t play_mutex;
	pthread_cond_t play_cond;
	STATUS process_status;

	ATK_AUDIOPLAY_HANDLE_T **p_playback_handle;
	ATK_AUDIOPLAY_CONFIG_T *p_config;
} user_data_t;


#ifdef DUMP_PCM_DATA
static int audio_fd_ = -1;

static int open_pcm_file()
{
	const char *filename = "./debug_audio.pcm";
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

static int write_pcm_data_to_file(unsigned char* const* audio_bufs, size_t data_bytes)
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

static int is_terminate_ = 0;


// The handle of ring buffer to receive audio data.
static srb_handle_t* audio_srb_handle_ = NULL;

static std::list<unsigned char*> free_buf_list_;
static std::list<unsigned char*> audio_buf_list_;
static std::mutex audio_buf_mutex_;
static std::mutex free_buf_mutex_;
static std::condition_variable audio_buf_cv_;

static void exit_process()
{
	is_terminate_ = 1;
	if(audio_srb_handle_)
	{
		// Notify the reader of ring buffer to exit.
		SRB_WakeupReader(audio_srb_handle_);
	}

	audio_buf_cv_.notify_all();
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
			"    %s -d PCM_device_name -r sample_rate -c channels -C codec_type [-B] [-R name] [-f]\n"
			"Options:\n"
			"    -d                 PCM device name for ALSA (ex: hw:0,0).\n"
			"    -r                 Sample rate for audio.\n"
			"    -c                 The number of audio channels.\n"
			"    -B                 ONVIF back channel mode.\n"
			"    -R                 The name of ring buffer for receiving data.\n"
			"    -D                 Run as Daemon.\n"
			"    -C                 Codec type (0: AAC4, 1: G711, 2: G726 3: GAMR 4: GPCM).\n"
			"    -t                 Codec type need AAC4 0: raw, 1: ADTS default: 0\n"
			"    -h                 This help\n", ap_name);
	fprintf(stderr, "ex:\n"
			"%s -d \"hw:0,0\" -r 8000 -c 2\n"
			"%s -d \"hw:0,0\" -r 8000 -c 2 -B -R audio_backchannel_srb_1\n", ap_name, ap_name);
}

static void send_cmd(const char *cmd)
{
	// Send the command to the audio encoder.
	MsgContext msg_context;
	pid_t connect_pid = getpid();
	msg_context.bHasResponse = 0;
	msg_context.pszHost = "encoder";
	msg_context.dwHostLen = strlen(msg_context.pszHost) + 1;
	msg_context.pszCmd = cmd;
	msg_context.dwCmdLen = strlen(msg_context.pszCmd) + 1;
	msg_context.dwDataSize = sizeof(pid_t);
	msg_context.pbyData = (unsigned char *) &connect_pid;
	MsgBroker_SendMsg(CMD_FIFO_PATH, &msg_context);
}

static void* play_interleaved_data(void* args) {
	unsigned int compression_format = 0;
	unsigned int bitrate = 0;
	unsigned int* values = NULL;
	unsigned char* temp_buf = NULL;
	g726_dec_handle_t *g726_handle = NULL;

	user_data_t *temp_data = (user_data_t*) args;
	unsigned int channels = temp_data->p_config->dwChannelsCount;
	int is_interleaved = temp_data->p_config->bIsInterleaved;
	ATK_AUDIOPLAY_HANDLE_T *playback_handle = *(temp_data->p_playback_handle);

#ifdef AAC_SW_DEC
	AAC4_DECODER_HANDLE_T *aac4dec_handle = NULL;
	AAC4_DECODER_DECODED_STREAM_INFO_T aac4_decoded_stream_info;
#endif
#ifdef GAMR_SUPPORT
	void *gamr_handle = NULL;
#endif

	// Get total bytes of data in one period.
	size_t period_frame_size = ATK_AudioPlay_GetPeriodFramesSize(playback_handle);

	// Audio output buffer.
	unsigned char *bufs[ATK_AUDIO_MAX_CHANNELS] = {NULL};

	// Calculate the buffer offset for the decoder.
	unsigned int decode_step = ((is_interleaved) && (channels > 1)) ? channels : 1;

	// Prepare the buffer pointer for the ring buffer.
	srb_buffer_t srb_buf;
	memset(&srb_buf, 0, sizeof(srb_buffer_t));

	// Tell the audio encoder to send the configuration data.
	send_cmd("forceCI");

	while(!is_terminate_)
	{
		
		if (temp_data->process_status == STOP) {
			pthread_mutex_lock(&(temp_data->data_mutex));	
			pthread_cond_signal(&(temp_data->data_cond));	
			pthread_mutex_unlock(&(temp_data->data_mutex));	
			
			pthread_mutex_lock(&(temp_data->data_mutex));	
			pthread_cond_wait(&(temp_data->play_cond), &(temp_data->data_mutex));	
			pthread_mutex_unlock(&(temp_data->data_mutex));	
		}

#ifdef DUMP_PCM_DATA
		if(bufs[0] != NULL)
		{
			write_pcm_data_to_file(bufs, period_frame_size);
		}
#endif

		// Output the current buffer and get the next output buffer.
		if(ATK_AudioPlay_PlayPeriodFrames(playback_handle, bufs) < 0)
		{
			fprintf(stderr, "[%s,%s] Can't get the audio output buffer..\n", __FILE__, __func__);
			break;
		}

		// Get the buffer with audio data from the audio encoder.
		if(SRB_ReturnReceiveReaderBuff(audio_srb_handle_, &srb_buf) == 0)
		{
			// We check whether the data is audio data or not.
			values = (unsigned int*)srb_buf.buffer;

			if(values[0] != FOURCC_CONF)
			{
				// This is audio data.

				// Check whether the data is G.711 data or not.
				if(values[0] == FOURCC_G711)
				{
					//printf("sec = %u, usec = %u\n", values[1], values[2]);

					// This is the payload (G.711 audio data).
					temp_buf = srb_buf.buffer + MAX_AUDIO_DATA_HEADER_SIZE;

					// Use G.711 decoder to decode the G.711 audio data and output to the output buffer.
					switch(compression_format)
					{
						case FOURCC_ULAW:
							G711_ULaw_Decode(temp_buf, (short*) bufs[0], PERIOD_SIZE_IN_FRAMES, decode_step);
							break;
						case FOURCC_ALAW:
							G711_ALaw_Decode(temp_buf, (short*) bufs[0], PERIOD_SIZE_IN_FRAMES, decode_step);
							break;
						default:
							fprintf(stderr, "[%s,%s] Unknown format.\n", __FILE__, __func__);
							break;
					}
				}
				else if(values[0] == FOURCC_G726)
				{
					if(g726_handle)
					{
						//printf("sec = %u, usec = %u\n", values[1], values[2]);

						// This is the payload (G.726 audio data).
						temp_buf = srb_buf.buffer + MAX_AUDIO_DATA_HEADER_SIZE;

						// Use G.726 decoder to decode the G.726 audio data and output to the output buffer.
						g726_decode_frame(g726_handle, (int16_t*) bufs[0], temp_buf, values[3], decode_step);
					}
				}
				else if(values[0] == FOURCC_GAMR)
				{
#ifdef GAMR_SUPPORT
					if(gamr_handle)
					{
						static short decbuffer[PERIOD_SIZE_IN_FRAMES_GAMR];

						//printf("[%s] len(%u)\n", __func__, values[3]);
						temp_buf = srb_buf.buffer + MAX_AUDIO_DATA_HEADER_SIZE;

						Decoder_Interface_Decode(gamr_handle, temp_buf, decbuffer, 0);

						short *ptr = (short*) bufs[0];
						for (int i = 0; i < PERIOD_SIZE_IN_FRAMES_GAMR; i++) {
							*ptr++ = decbuffer[i];
							*ptr++ = decbuffer[i];
						}
					}
#endif
				}
				else if(values[0] == FOURCC_GPCM)
				{
					temp_buf = srb_buf.buffer + MAX_AUDIO_DATA_HEADER_SIZE;
					memcpy(bufs[0], temp_buf, values[3]);
				}
#ifdef AAC_SW_DEC
				else if(values[0] == FOURCC_AAC4)
				{
					if (aac4dec_handle)
					{
						// This is the payload (AAC4 audio data).
						temp_buf = srb_buf.buffer + MAX_AUDIO_DATA_HEADER_SIZE;
						int ret = Fdk_AAC4_Decoder_OneFrame(aac4dec_handle, temp_buf, values[3], bufs[0], period_frame_size);
						if (!ret)
						{
							if (!aac4_decoded_stream_info.dwChannels)
							{
								ret = Fdk_AAC4_Decoder_Get_Decoded_Stream_Info(aac4dec_handle, &aac4_decoded_stream_info);
								if(ret)
								{
									fprintf(stderr, "[%s,%s] Fdk_AAC4_Decoder_Get_Decoded_Stream_Info() failed, ret = %d.\n", __FILE__, __func__, ret);
								}
							}
						}
						else
						{
							fprintf(stderr, "[%s,%s] Fdk_AAC4_Decoder_OneFrame(%u) failed, ret = %d.\n", __FILE__, __func__, values[3], ret);
						}
					}
				}
#endif
			}
			else
			{
				// This is configuration data.
				if(values[2] == FOURCC_G711)
				{
					// FOURCC_ULAW or FOURCC_ALAW
					compression_format = values[3];
					if(compression_format == FOURCC_ULAW)
					{
						printf("FOURCC_ULAW\n");
					}
					else if(compression_format == FOURCC_ALAW)
					{
						printf("FOURCC_ALAW\n");
					}
					else
					{
						printf("Unknow format\n");
					}
				}
				else if(values[2] == FOURCC_G726)
				{
					bitrate = values[3] * 8000;
					printf("G.726 bitrate = %u\n", bitrate);

					if(g726_handle)
					{
						g726_dec_release(g726_handle);
						g726_handle = NULL;
					}

					g726_handle = g726_dec_init(bitrate);
					if(g726_handle == NULL)
					{
						fprintf(stderr, "[%s,%s] Can't initialize the G.726 decoder.\n", __FILE__, __func__);
					}
				}
				else if(values[2] == FOURCC_GAMR)
				{
#ifdef GAMR_SUPPORT
					if(gamr_handle)
					{
						Decoder_Interface_exit(gamr_handle);
						gamr_handle = NULL;
					}

					gamr_handle = Decoder_Interface_init();
					if(gamr_handle == NULL)
					{
						fprintf(stderr, "[%s,%s] Can't initialize the G.AMR decoder.\n", __FILE__, __func__);
					}
#endif
				}
#ifdef AAC_SW_DEC
				else if(values[2] == FOURCC_AAC4)
				{
					AAC4_DECODER_INITOPT_T aac4dec_init_opt;
					memset(&aac4dec_init_opt, 0, sizeof(aac4dec_init_opt));
					memset(&aac4_decoded_stream_info, 0, sizeof(aac4_decoded_stream_info));
					aac4dec_init_opt.dwChannels = channels;
					aac4dec_init_opt.dwSampleRate = values[3];
					aac4dec_init_opt.dwSpecConfSize = values[6];
					aac4dec_init_opt.pbySpecConf = (uint8_t *) ((unsigned int *) (values + 7));
					aac4dec_init_opt.dwAdts = g_adts;
					aac4dec_handle = Fdk_AAC4_Decoder_Init(&aac4dec_init_opt);
					if(aac4dec_handle == NULL)
					{
						fprintf(stderr, "[%s,%s] Can't initialize the AAC4 decoder.\n", __FILE__, __func__);
					}
				}
#endif
				// Tell the audio encoder to start to encode data.
				send_cmd("start");
			}
		}
	}

	// Return the last buffer to the ring buffer.
	SRB_ReturnReaderBuff(audio_srb_handle_, &srb_buf);

	// Tell the audio encoder to stop encoding data.
	send_cmd("stop");

	if(g726_handle)
	{
		g726_dec_release(g726_handle);
		g726_handle = NULL;
	}
#ifdef GAMR_SUPPORT
	if(gamr_handle)
	{
		Decoder_Interface_exit(gamr_handle);
		gamr_handle = NULL;
	}
#endif
#ifdef AAC_SW_DEC
	if(aac4dec_handle)
	{
		Fdk_AAC4_Decoder_Release(aac4dec_handle);
		aac4dec_handle = NULL;
	}
#endif
	return NULL;
}



static void* play_interleaved_data_back_channel(void* args)
{
	unsigned int *values = NULL;
	unsigned char *temp_buf = NULL;
	unsigned char *temp_payload_buf = NULL;
	unsigned char *temp_input_buf = NULL;

	user_data_t *temp_data = (user_data_t*) args;
	unsigned int channels = temp_data->p_config->dwChannelsCount;
	int is_interleaved = temp_data->p_config->bIsInterleaved;
	ATK_AUDIOPLAY_HANDLE_T *playback_handle = *(temp_data->p_playback_handle);

	// Get total bytes of data in one period.
	size_t period_frame_size = ATK_AudioPlay_GetPeriodFramesSize(playback_handle);

	// Audio output buffer.
	unsigned char *bufs[ATK_AUDIO_MAX_CHANNELS] = {NULL};

	// Calculate the buffer offset for the decoder.
	unsigned int decode_step = ((is_interleaved) && (channels > 1)) ? channels : 1;

	// Flag to indicate whether the data is ready to write out.
	bool is_ready_to_output = false;
	// The number of encoded frames those are received from the back channel of RTSP server.
	size_t current_encoded_frames = 0;
	// The number of encoded frames those are processed.
	size_t processed_encoded_frames = 0;
	// The number of decoded frames.
	size_t current_decoded_frames = 0;
	// The number of frames those are prepare to be decoded.
	size_t prepare_decode_frames = 0;

	// The number of decoded samples.
	size_t current_decoded_samples = 0;
	// The number of decoded bytes.
	size_t current_decoded_bytes = 0;
	// The number of decoded bytes those are processed.
	size_t processed_decoded_bytes = 0;
	// The bytes of data those are output.
	size_t current_output_bytes = 0;
	// The bytes of data those are prepare to be output.
	size_t prepare_output_bytes = 0;

	std::list<unsigned char*> temp_list;
	std::list<unsigned char*> temp_free_list;

	unsigned int bitrate = 0;
	G726DecContext *g726_handle = NULL;
	unsigned char *internal_buf[ATK_AUDIO_MAX_CHANNELS] = {NULL};
	size_t internal_buf_size = 0;
	size_t max_decode_size = 0;
	size_t byte_per_frame = period_frame_size / PERIOD_SIZE_IN_FRAMES;
	//size_t byte_per_frame = ATK_AudioPlay_GetOneFrameSize(playback_handle);

	// Get the first output buffer.
	if(ATK_AudioPlay_PlayPeriodFrames(playback_handle, bufs) < 0)
	{
		fprintf(stderr, "[%s,%s] Can't get the first audio output buffer..\n", __FILE__, __func__);
		return NULL;
	}

	while(!is_terminate_)
	{
		{
			// Get the audio data from queue.
			std::unique_lock<std::mutex> lk(audio_buf_mutex_);
			if(audio_buf_list_.empty())
			{
				audio_buf_cv_.wait(lk);
				if(audio_buf_list_.empty()) continue;
			}

			temp_list.splice(temp_list.end(), audio_buf_list_);
		}

		while(!is_terminate_ && !temp_list.empty())
		{
			if (temp_data->process_status == STOP) {
				pthread_mutex_lock(&(temp_data->data_mutex));	
				pthread_cond_signal(&(temp_data->data_cond));	
				pthread_mutex_unlock(&(temp_data->data_mutex));	
				
				pthread_mutex_lock(&(temp_data->data_mutex));	
				pthread_cond_wait(&(temp_data->play_cond), &(temp_data->data_mutex));	
				pthread_mutex_unlock(&(temp_data->data_mutex));	
			}
			
			temp_input_buf = temp_list.front();
			temp_list.pop_front();
			temp_free_list.push_back(temp_input_buf); // We place the buffer into free list first.

			// We check whether the data is audio data or not.
			values = (unsigned int*)temp_input_buf;
			temp_payload_buf = temp_input_buf + MAX_AUDIO_DATA_HEADER_SIZE;

			if(values[5] == FOURCC_G711)
			{
				// Check whether the data is G.711 data or not.
				current_encoded_frames = values[1];
				processed_encoded_frames = 0;

				while(processed_encoded_frames < current_encoded_frames)
				{
					// This is the payload (G.711 audio data).
					temp_buf = temp_payload_buf + processed_encoded_frames;

					if((current_encoded_frames - processed_encoded_frames + current_decoded_frames) >= PERIOD_SIZE_IN_FRAMES)
					{
						prepare_decode_frames = PERIOD_SIZE_IN_FRAMES - current_decoded_frames;
						is_ready_to_output = true;
					}
					else
					{
						prepare_decode_frames = current_encoded_frames - processed_encoded_frames;
						is_ready_to_output = false;
					}

					// Use G.711 decoder to decode the G.711 audio data and output to the output buffer.
					switch(values[6])
					{
						case FOURCC_ULAW:
							G711_ULaw_Decode(temp_buf, ((short*)bufs[0]) + current_decoded_frames*decode_step, prepare_decode_frames, decode_step);
							break;
						case FOURCC_ALAW:
							G711_ALaw_Decode(temp_buf, ((short*)bufs[0]) + current_decoded_frames*decode_step, prepare_decode_frames, decode_step);
							break;
						default:
							fprintf(stderr, "[%s,%s] Unknown format.\n", __FILE__, __func__);
							break;
					}

					if(is_ready_to_output)
					{
						// Output the current buffer and get the next output buffer.
						if(ATK_AudioPlay_PlayPeriodFrames(playback_handle, bufs) < 0)
						{
							fprintf(stderr, "[%s,%s] Can't get the audio output buffer..\n", __FILE__, __func__);
						}

						current_decoded_frames = 0;
					}
					else
					{
						current_decoded_frames += prepare_decode_frames;
					}

					processed_encoded_frames += prepare_decode_frames;
				}
			} // FOURCC_G711
			else if(values[5] == FOURCC_G726)
			{
				// Check whether the data is G.726 data or not.

				if(bitrate != values[6])
				{
					if(g726_handle)
					{
						g726_dec_release(g726_handle);
					}
					g726_handle = g726_dec_init(values[6] * 1000);
					if(g726_handle == NULL)
					{
						fprintf(stderr, "[%s,%s] Can't create G.726 decoder\n", __FILE__, __func__);
						continue;
					}
					bitrate = values[6];
				}

				if(g726_handle == NULL)
				{
					continue;
				}

				max_decode_size = ( (values[1] << 3) / (values[6]>>3) ) * byte_per_frame;

				if((internal_buf[0] == NULL) || (internal_buf_size < max_decode_size))
				{
					if(internal_buf[0])
					{
						free(internal_buf[0]);
					}

					// This buffer size should be larger than the size of total decoded frames.
					internal_buf[0] = (unsigned char*)malloc(max_decode_size);
					if(internal_buf[0] == NULL)
					{
						fprintf(stderr, "[%s,%s] Unable to allocate memory for internal buffer.\n", __FILE__, __func__);
						return NULL;
					}
					internal_buf_size = max_decode_size;
				}

				// This is the payload (G.726 audio data).
				processed_decoded_bytes = 0;
				current_decoded_samples = g726_decode_frame(g726_handle, (int16_t*)internal_buf[0], temp_payload_buf, values[1], decode_step);
				current_decoded_bytes = current_decoded_samples << 1;

				while(processed_decoded_bytes < current_decoded_bytes)
				{
					if((current_decoded_bytes - processed_decoded_bytes + current_output_bytes) >= period_frame_size)
					{
						prepare_output_bytes = period_frame_size - current_output_bytes;
						is_ready_to_output = true;
					}
					else
					{
						prepare_output_bytes = current_decoded_bytes - processed_decoded_bytes;
						is_ready_to_output = false;
					}

					memcpy(bufs[0] + current_output_bytes, internal_buf[0] + processed_decoded_bytes, prepare_output_bytes);

					if(is_ready_to_output)
					{
						// Output the current buffer and get the next output buffer.
						if(ATK_AudioPlay_PlayPeriodFrames(playback_handle, bufs) < 0)
						{
							fprintf(stderr, "[%s,%s] Can't get the audio output buffer..\n", __FILE__, __func__);
						}

						current_output_bytes = 0;
					}
					else
					{
						current_output_bytes += prepare_output_bytes;
					}

					processed_decoded_bytes += prepare_output_bytes;
				}
			} // FOURCC_G726
		};

		{
			// Reclaim the buffer.
			std::unique_lock<std::mutex> lk(free_buf_mutex_);
			free_buf_list_.splice(free_buf_list_.end(), temp_list);
			free_buf_list_.splice(free_buf_list_.end(), temp_free_list);
		}
	} // while(is_running_)

	if(g726_handle)
	{
		g726_dec_release(g726_handle);
	}

	if(internal_buf[0])
	{
		free(internal_buf[0]);
	}
	
	return NULL;
}

static void audio_receiver()
{
	unsigned char *temp_buf = NULL;
	unsigned int *values = NULL;
	const size_t buf_size = 4 * 4 * 1024;

	// Prepare the buffer pointer for the ring buffer.
	srb_buffer_t srb_buf;
	memset(&srb_buf, 0, sizeof(srb_buffer_t));
	//static size_t counter = 0; // Currently, we don't limit the size of queue.

	while(!is_terminate_)
	{
		// Get the buffer with audio data from the back channel of RTSP server.
		if(SRB_ReturnReceiveReaderBuff(audio_srb_handle_, &srb_buf) < 0)
		{
			fprintf(stderr, "[%s,%s] Can't get the data from the back channel of RTSP server ...\n", __FILE__, __func__);
			continue;
		}

		// We check whether the data is from back channel of RTSP server.
		values = (unsigned int*)srb_buf.buffer;
		if(values[0] != FOURCC_EXTEND)
		{
			continue;
		}

		if(temp_buf == NULL)
		{
			std::unique_lock<std::mutex> lk(free_buf_mutex_);
			// Get one buffer
			if(!free_buf_list_.empty())
			{
				temp_buf = free_buf_list_.front();
				free_buf_list_.pop_front();
				//printf("get from free list .......\n");
			}
			else
			{
				temp_buf = (unsigned char*) malloc(buf_size);
				//++counter;
				//printf("counter = %u\n", counter);
			}
		}

		memcpy(temp_buf, srb_buf.buffer, values[1] + MAX_AUDIO_DATA_HEADER_SIZE);

		{
			std::unique_lock<std::mutex> lk(audio_buf_mutex_);
			audio_buf_list_.push_back(temp_buf);
		}
		audio_buf_cv_.notify_all();
		temp_buf = NULL;
	}

	// Return the last buffer to the ring buffer.
	SRB_ReturnReaderBuff(audio_srb_handle_, &srb_buf);
}

static void* play_interleaved_data_frame_mode(void* args)
{
	unsigned int compression_format = 0;
	unsigned int bitrate = 0;
	unsigned int bits_per_encode_sample = 0;
	unsigned int* values = NULL;
	unsigned char* temp_buf = NULL;
	unsigned char *temp_payload_buf = NULL;
	g726_dec_handle_t *g726_handle = NULL;
	unsigned char *internal_buf[ATK_AUDIO_MAX_CHANNELS] = {NULL};
	size_t internal_buf_size = 0;
	size_t max_decode_size = 0;

	user_data_t *temp_data = (user_data_t*) args;
	unsigned int channels = temp_data->p_config->dwChannelsCount;
	int is_interleaved = temp_data->p_config->bIsInterleaved;
	ATK_AUDIOPLAY_HANDLE_T *playback_handle = *(temp_data->p_playback_handle);
	
	size_t byte_per_frame = ATK_AudioPlay_GetOneFrameSize(playback_handle);
	unsigned int mmap_frames = 0;
	unsigned int mmap_frames_bytes = 0;

	// Audio output buffer.
	unsigned char *bufs[ATK_AUDIO_MAX_CHANNELS] = {NULL};

	// Calculate the buffer offset for the decoder.
	unsigned int decode_step = ((is_interleaved) && (channels > 1)) ? channels : 1;

	// Flag to indicate whether the data is ready to write out.
	bool is_ready_to_output = false;
	// The number of encoded frames those are received from the back channel of RTSP server.
	size_t current_encoded_frames = 0;
	// The number of encoded frames those are processed.
	size_t processed_encoded_frames = 0;
	// The number of decoded frames.
	size_t current_decoded_frames = 0;
	// The number of frames those are prepare to be decoded.
	size_t prepare_decode_frames = 0;

	// The number of decoded samples.
	size_t current_decoded_samples = 0;
	// The number of decoded bytes.
	size_t current_decoded_bytes = 0;
	// The number of decoded bytes those are processed.
	size_t processed_decoded_bytes = 0;
	// The bytes of data those are output.
	size_t current_output_bytes = 0;
	// The bytes of data those are prepare to be output.
	size_t prepare_output_bytes = 0;


	// Prepare the buffer pointer for the ring buffer.
	srb_buffer_t srb_buf;
	memset(&srb_buf, 0, sizeof(srb_buffer_t));

	// Tell the audio encoder to send the configuration data.
	send_cmd("forceCI");

	// Tell the audio encoder to start to encode data.
	send_cmd("start");

	printf("frame mode .......................\n");

	while(!is_terminate_)
	{
		// Get first output buffer.
		if(ATK_AudioPlay_PlayPeriodFrames(playback_handle, bufs) < 0)
		{
			fprintf(stderr, "[%s,%s] Can't get the audio output buffer..\n", __FILE__, __func__);
			is_terminate_ = 1;
			break;
		}

		if(mmap_frames > 0)
		{
			mmap_frames_bytes = mmap_frames * byte_per_frame;
			break;
		}
		else
		{
			continue;
		}
	}

	while(!is_terminate_)
	{
		if (temp_data->process_status == STOP) {
			pthread_mutex_lock(&(temp_data->data_mutex));	
			pthread_cond_signal(&(temp_data->data_cond));	
			pthread_mutex_unlock(&(temp_data->data_mutex));	
			
			pthread_mutex_lock(&(temp_data->data_mutex));	
			pthread_cond_wait(&(temp_data->play_cond), &(temp_data->data_mutex));	
			pthread_mutex_unlock(&(temp_data->data_mutex));	
		}
		
		// Get the buffer with audio data from the audio encoder.
		if(SRB_ReturnReceiveReaderBuff(audio_srb_handle_, &srb_buf) == 0)
		{
			// We check whether the data is audio data or not.
			values = (unsigned int*)srb_buf.buffer;

			if(values[0] != FOURCC_CONF)
			{
				// This is the payload (audio data).
				temp_payload_buf = srb_buf.buffer + MAX_AUDIO_DATA_HEADER_SIZE;

				if(values[0] == FOURCC_G711)
				{
					// Check whether the data is G.711 data or not.
					current_encoded_frames = values[3];
					processed_encoded_frames = 0;

					while(processed_encoded_frames < current_encoded_frames)
					{
						// This is the payload (G.711 audio data).
						temp_buf = temp_payload_buf + processed_encoded_frames;

						if((current_encoded_frames - processed_encoded_frames + current_decoded_frames) >= mmap_frames)
						{
							prepare_decode_frames = mmap_frames - current_decoded_frames;
							is_ready_to_output = true;
						}
						else
						{
							prepare_decode_frames = current_encoded_frames - processed_encoded_frames;
							is_ready_to_output = false;
						}

						// Use G.711 decoder to decode the G.711 audio data and output to the output buffer.
						switch(compression_format)
						{
							case FOURCC_ULAW:
								G711_ULaw_Decode(temp_buf, ((short*)bufs[0]) + current_decoded_frames*decode_step, prepare_decode_frames, decode_step);
								break;
							case FOURCC_ALAW:
								G711_ALaw_Decode(temp_buf, ((short*)bufs[0]) + current_decoded_frames*decode_step, prepare_decode_frames, decode_step);
								break;
							default:
								fprintf(stderr, "[%s,%s] Unknown format.\n", __FILE__, __func__);
								break;
						}

						if(is_ready_to_output)
						{
							while(!is_terminate_)
							{
								// Output the current buffer and get the next output buffer.
								if(ATK_AudioPlay_PlayPeriodFrames(playback_handle, bufs) < 0)
								{
									fprintf(stderr, "[%s,%s] Can't get the audio output buffer..\n", __FILE__, __func__);
								}

								if(mmap_frames > 0)
								{
									break;
								}
								else
								{
									continue;
								}
							}
							mmap_frames_bytes = mmap_frames * byte_per_frame;

							current_decoded_frames = 0;
						}
						else
						{
							current_decoded_frames += prepare_decode_frames;
						}

						processed_encoded_frames += prepare_decode_frames;
					}
				} // FOURCC_G711
				else if(values[0] == FOURCC_G726)
				{
					// Check whether the data is G.726 data or not.
					if(bits_per_encode_sample == 0) continue;

					// Note: The size of the payload should be multiple of 2, 3, 4 and 5. Otherwise, the below can't work correctly.
					assert((values[3] % bits_per_encode_sample) == 0);

					max_decode_size = ( (values[3] << 3) / bits_per_encode_sample ) * byte_per_frame;

					if((internal_buf[0] == NULL) || (internal_buf_size < max_decode_size))
					{
						if(internal_buf[0])
						{
							free(internal_buf[0]);
						}

						// This buffer size should be larger than the size of total decoded frames.
						internal_buf[0] = (unsigned char*)malloc(max_decode_size);
						if(internal_buf[0] == NULL)
						{
							fprintf(stderr, "[%s,%s] Unable to allocate memory for internal buffer.\n", __FILE__, __func__);
							return NULL;
						}
						internal_buf_size = max_decode_size;
					}

					// This is the payload (G.726 audio data).
					processed_decoded_bytes = 0;
					current_decoded_samples = g726_decode_frame(g726_handle, (int16_t*)internal_buf[0], temp_payload_buf, values[3], decode_step);
					current_decoded_bytes = current_decoded_samples << 1;

					while(processed_decoded_bytes < current_decoded_bytes)
					{
						if((current_decoded_bytes - processed_decoded_bytes + current_output_bytes) >= mmap_frames_bytes)
						{
							prepare_output_bytes = mmap_frames_bytes - current_output_bytes;
							is_ready_to_output = true;
						}
						else
						{
							prepare_output_bytes = current_decoded_bytes - processed_decoded_bytes;
							is_ready_to_output = false;
						}

						memcpy(bufs[0] + current_output_bytes, internal_buf[0] + processed_decoded_bytes, prepare_output_bytes);

						if(is_ready_to_output)
						{
							while(!is_terminate_)
							{
								// Output the current buffer and get the next output buffer.
								if(ATK_AudioPlay_PlayPeriodFrames(playback_handle, bufs) < 0)
								{
									fprintf(stderr, "[%s,%s] Can't get the audio output buffer..\n", __FILE__, __func__);
								}

								if(mmap_frames > 0)
								{
									break;
								}
								else
								{
									continue;
								}
							}

							mmap_frames_bytes = mmap_frames * byte_per_frame;

							current_output_bytes = 0;
						}
						else
						{
							current_output_bytes += prepare_output_bytes;
						}

						processed_decoded_bytes += prepare_output_bytes;
					}
				} // FOURCC_G726
			}
			else
			{
				// This is configuration data.
				if(values[2] == FOURCC_G711)
				{
					// FOURCC_ULAW or FOURCC_ALAW
					compression_format = values[3];
					if(compression_format == FOURCC_ULAW)
					{
						printf("FOURCC_ULAW\n");
					}
					else if(compression_format == FOURCC_ALAW)
					{
						printf("FOURCC_ALAW\n");
					}
					else
					{
						printf("Unknow format\n");
					}
				}
				else if(values[2] == FOURCC_G726)
				{
					bits_per_encode_sample = values[3];
					bitrate = bits_per_encode_sample * 8000;
					printf("G.726 bitrate = %u\n", bitrate);

					if(g726_handle)
					{
						g726_dec_release(g726_handle);
						g726_handle = NULL;
					}

					g726_handle = g726_dec_init(bitrate);
					if(g726_handle == NULL)
					{
						fprintf(stderr, "[%s,%s] Can't initialize the G.726 decoder.\n", __FILE__, __func__);
					}
				}
			}
		}
	}

	// Return the last buffer to the ring buffer.
	SRB_ReturnReaderBuff(audio_srb_handle_, &srb_buf);

	// Tell the audio encoder to stop encoding data.
	send_cmd("stop");

	if(g726_handle)
	{
		g726_dec_release(g726_handle);
		g726_handle = NULL;
	}

	if(internal_buf[0])
	{
		free(internal_buf[0]);
	}

	return NULL;
}

static void msg_callback(MsgContext* msg, void* args)
{
	user_data_t *temp_data = (user_data_t*) args;
	if( !strcasecmp(msg->pszHost, SR_MODULE_NAME) ){
		if( !strcasecmp(msg->pszCmd, SUSPEND_CMD) ) {
						
			pthread_mutex_lock(&(temp_data->data_mutex));
			SRB_WakeupReader(audio_srb_handle_);
			temp_data->process_status = STOP;
			
			pthread_cond_wait(&(temp_data->data_cond), &(temp_data->data_mutex));
			pthread_mutex_unlock(&(temp_data->data_mutex));

			if(*(temp_data->p_playback_handle))
				ATK_AudioPlay_Release(*(temp_data->p_playback_handle));
				
			MsgBroker_SuspendAckMsg();
		}else if( !strcasecmp(msg->pszCmd, RESUME_CMD) ) {
			*(temp_data->p_playback_handle) = ATK_AudioPlay_Init(temp_data->p_config);
			pthread_mutex_lock(&(temp_data->data_mutex));
			temp_data->process_status = START;
			pthread_cond_signal(&(temp_data->play_cond));
			pthread_mutex_unlock(&(temp_data->data_mutex));
		}
	}

	if (msg->bHasResponse)
		msg->dwDataSize = 0;
}


int main(int argc, char **argv)
{
	int opt;
	bool is_daemon = false;
	bool is_onvif_back_channel = false;
	bool use_frame_mode = false;
	pthread_t playback_tid = 0;
	ATK_AUDIOPLAY_HANDLE_T *playback_handle = NULL;
	ATK_AUDIOPLAY_CONFIG_T config;

	std::string pcm_name = "hw:0,0";
	std::string ring_buf_name;
	std::shared_ptr<std::thread> audio_receiver_thread;
	int codec_type = -1; // 0: AAC4, 1: G711, 2: G726 3: GAMR 4: GPCM

	// Default value of configuration.
	memset(&config, 0, sizeof(ATK_AUDIOPLAY_CONFIG_T));
	config.szPcmName = pcm_name.c_str();
	config.bIsInterleaved = 1;
	config.eFormat = SND_PCM_FORMAT_S16_LE;
	config.dwChannelsCount = 2;
	config.dwSampleRate = 8000;
	config.bUseSimpleConfig = 0;
	config.dwPeriodsPerBuffer = 8;

	while ((opt = getopt(argc, argv, "BDhfd:r:c:R:C:t:")) != -1)
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
			case 't':
				g_adts = atoi(optarg);
				break;
			case 'B':
				is_onvif_back_channel = true;
				break;
			case 'R':
				ring_buf_name = optarg;
				break;
			case 'f':
				use_frame_mode = true;
				break;
			case 'D':
				is_daemon = true;
				break;
			case 'C':
				codec_type = atoi(optarg);
				break;
			case 'h':
			default:
				print_usage(argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	signal(SIGTERM, sig_kill);
	signal(SIGINT, sig_kill);
	signal(SIGSEGV, dump_trace);

	if(-1 == codec_type)
	{
		fprintf(stderr, "[%s,%s] codec_type(-C) not be assigned !\n", __FILE__, __func__);
		goto main_end;
	}

	if(kGAMR==codec_type)
	{
		config.dwPeriodSizeInFrames = PERIOD_SIZE_IN_FRAMES_GAMR;
	}
	else if(kAAC4==codec_type)
	{
		config.dwPeriodSizeInFrames = PERIOD_SIZE_IN_FRAMES_AAC;
	}
	else
	{
		config.dwPeriodSizeInFrames = PERIOD_SIZE_IN_FRAMES;
	}

	if (is_daemon)
	{
		daemon(1,1);
	}

	user_data_t user_data;
	memset(&user_data, 0, sizeof(user_data_t));
	pthread_mutex_init(&(user_data.data_mutex), NULL); 
	pthread_cond_init(&(user_data.data_cond), NULL);
	pthread_cond_init(&(user_data.play_cond), NULL);

	
	user_data.process_status = START;
	user_data.p_playback_handle = &playback_handle;
	user_data.p_config = &config;

	// Initialize audio playback.
	playback_handle = ATK_AudioPlay_Init(&config);
	if(playback_handle == NULL)
	{
		fprintf(stderr, "[%s,%s] Can't initialize the audio playback.\n", __FILE__, __func__);
		goto main_end;
	}

	// Initialize the reader of ring buffer.
	if(ring_buf_name.empty())
	{
		ring_buf_name = (!is_onvif_back_channel) ? "aenc_srb_1" : "audio_backchannel_srb_1";
	}
	audio_srb_handle_ = SRB_InitReader(ring_buf_name.c_str());
	if(audio_srb_handle_ == NULL)
	{
		fprintf(stderr, "[%s,%s] Can't initialize the reader of ring buffer.\n", __FILE__, __func__);
		goto main_end;
	}

#ifdef DUMP_PCM_DATA
	if(open_pcm_file() < 0)
	{
		goto main_end;
	}
#endif

	if(config.bIsInterleaved)
	{
		if(!is_onvif_back_channel)
		{
			// Start the loop to play audio.
			if(!use_frame_mode)
			{
				pthread_create(&playback_tid, NULL, play_interleaved_data, &user_data);
				//play_interleaved_data(playback_handle, config.dwChannelsCount, config.bIsInterleaved);

			}
			else
			{
				pthread_create(&playback_tid, NULL, play_interleaved_data_frame_mode, &user_data);
				//play_interleaved_data_frame_mode(playback_handle, config.dwChannelsCount, config.bIsInterleaved);
			}
		}
		else
		{
			audio_receiver_thread.reset(new std::thread(audio_receiver));
			pthread_create(&playback_tid, NULL, play_interleaved_data_back_channel, &user_data);
			//play_interleaved_data_back_channel(playback_handle, config.dwChannelsCount, config.bIsInterleaved);
		}
	}

	MsgBroker_RegisterMsg(CMD_PLAY_FIFO_PATH);
	MsgBroker_Run(CMD_PLAY_FIFO_PATH, msg_callback, (void *)&user_data, (int *)&is_terminate_);
	MsgBroker_UnRegisterMsg();

main_end:

	if(audio_receiver_thread)
	{
		audio_receiver_thread->join();
	}
	pthread_join(playback_tid, NULL);

	free_buf_list_.splice(free_buf_list_.end(), audio_buf_list_);
	while(!free_buf_list_.empty())
	{
		free(free_buf_list_.front());
		free_buf_list_.pop_front();
	}

	// Release audio playback.
	if(playback_handle)
		ATK_AudioPlay_Release(playback_handle);

	pthread_mutex_destroy(&(user_data.data_mutex));
	pthread_cond_destroy(&(user_data.data_cond));
	pthread_cond_destroy(&(user_data.play_cond));

	// Release the reader of ring buffer.
	if(audio_srb_handle_)
		SRB_Release(audio_srb_handle_);

#ifdef DUMP_PCM_DATA
	close_pcm_file();
#endif

	return 0;
}

