/**
 *
 * Copyright (c) 2015 VATICS INC.
 * All Rights Reserved.
 *
 * @brief		Atk two way audio Integration api
 *				
 * @author		Rossi Chung
 * @date		2017/2/16
 *
 */

 
#include "audio_setting.h"
#include <MemBroker/mem_broker.h>
 
  #if 0
 static ATK_AAC4ENC_CONFIG_T default_aac4_config = {
	 32000, 		 // bit_rate - 8000, 16000, 24000, 32000
	 8000,			 // dwSampleRate - 8000, 16000
	 1, 			 // dwAdts - 0: Raw, 1: ADTS or 2: ADIF
	 0				 // stereo_mode - 0: stereo, 1: joint stereo, 3:mono
 };
 static ATK_G711ENC_CONFIG_T default_g711_config = {
	 0				 // compression_mode - 0: ulaw, 1: alaw
 };

 static ATK_G726ENC_CONFIG_T default_g726_config = {
	 32000			 // bit_rate - 16000, 24000, 32000, 40000
 };
 static atk_gamrenc_config_t default_gamr_config = {
	 12200, 		 // bit_rate - 4750 ,5150 , 5900, 6700, 7400, 7950, 10200, 12200
	 8000			 // dwSampleRate
 };
#endif

typedef struct 
{
 	void* dec_handle;
 	unsigned int codec_type;	// 0: AAC4, 1: G711, 2: G726, 3: PCM
 	unsigned char *recv_buff;	
 	int recv_bytes;
	unsigned char *decode_out_buf;
	unsigned int decode_step;
	unsigned int decode_data_bytes_factor; // This is only for G.711
	size_t total_play_size; 
	size_t playback_buf_bytes;
}decode_state;


#define RingBuffer_available_data(B) (((B)->end >= (B)->start) ? ((B)->end - (B)->start) : ((B)->length - ((B)->start - (B)->end + 1)))	//(((B)->end + 1) % (B)->length - (B)->start - 1)
#define RingBuffer_available_space(B) 	(((B)->end >= (B)->start) ? ((B)->length - ((B)->end - (B)->start + 1)) : ((B)->start - (B)->end + 1))
#define RingBuffer_commit_read(B, A) ((B)->start = ((B)->start + (A)) % ((B)->length-1))	//((B)->start = ((B)->start + (A)) % (B)->length)
#define RingBuffer_commit_write(B, A) ((B)->end = ((B)->end + (A)) % ((B)->length-1))//((B)->end = ((B)->end + (A)) % (B)->length)
#define RingBuffer_starts_at(B) ((B)->buffer + (B)->start)
#define RingBuffer_ends_at(B) ((B)->buffer + (B)->end)


//#define SET_TCP	    //tcp udp  are exclusively!
#define SET_UDP
#define MAX_CLIENT_CONNECTIONS		1

RingBuffer *RingBuffer_create(int length)
{
	RingBuffer *buffer = (RingBuffer *)calloc(1, sizeof(RingBuffer));
	buffer->length  = length + 1;
	buffer->start = 0;
	buffer->end = 0;
	buffer->buffer = (unsigned char *)calloc(buffer->length, sizeof(unsigned char));
	return buffer;
}
 
void RingBuffer_destroy(RingBuffer *buffer)
{
	if(buffer) {
		free(buffer->buffer);
		free(buffer);
	}
}
 
int RingBuffer_write(RingBuffer *buffer, unsigned char *data, int length)
{
	// check available data
	if(RingBuffer_available_data(buffer) == 0) {
		buffer->start = buffer->end = 0;
	}
 
	// check available space
	if(length > RingBuffer_available_space(buffer)) {
		//printf("Not enough space: %d request, %d available \n", length, RingBuffer_available_space(buffer));
		return -1;
	}
 
	if((buffer->length - buffer->end -1) >= length)
	{
		memcpy(RingBuffer_ends_at(buffer), data, length);
		//printf("\t RingBuffer_write : length = %d \n", length);
	}
	else
	{
		int frontLen = -1, backLen = -1;
		frontLen = buffer->length - buffer->end -1;
		backLen = length - frontLen;
		memcpy(RingBuffer_ends_at(buffer), data, frontLen);
		memcpy(buffer->buffer, data+frontLen, backLen);
	}
 
	RingBuffer_commit_write(buffer, length);
 
	return length;
}
 
int RingBuffer_read(RingBuffer *buffer, unsigned char *target, int amount)
{
	// check available data
	if(amount > RingBuffer_available_data(buffer)) {
		//printf("Not enough in the buffer: has %d, needs %d, start = %d, end = %d \n", RingBuffer_available_data(buffer), amount, buffer->start, buffer->end);
		return -1;
	}
 
	if(buffer->end >= buffer->start)
	{
		memcpy(target, RingBuffer_starts_at(buffer), amount);
		//printf("\t RingBuffer_read : amount = %d \n", amount);
	}
	else
	{
		int frontLen = -1, backLen = -1;
		if(amount > (buffer->length - buffer->start - 1))
		{
			frontLen = buffer->length - buffer->start -1;
			backLen = amount - frontLen;
			memcpy(target, RingBuffer_starts_at(buffer), frontLen);
			memcpy(target, buffer->buffer, backLen);
		}
		else
		{
			memcpy(target, RingBuffer_starts_at(buffer), amount);
		}
	}
 
	RingBuffer_commit_read(buffer, amount);
 
	if(buffer->end == buffer->start) {
		buffer->start = buffer->end = 0;
	}
 
	return amount;
}

static void msg_callback(MsgContext* msg, void* user_data)
{
	if (!user_data) return;
	twa_ctx* ctx = (twa_ctx*) user_data;

	if( !strcasecmp(msg->pszHost, SR_MODULE_NAME) ){
		if( !strcasecmp(msg->pszCmd, SUSPEND_CMD) ) {
					
			if (ctx->atk_capture_handle != NULL) {
				pthread_mutex_lock(&(ctx->sr_mutex));
				ctx->sr_status = STOP;
				pthread_cond_wait(&(ctx->capture_cond), &(ctx->sr_mutex));
				pthread_mutex_unlock(&(ctx->sr_mutex));

				struct timespec to;
				struct timeval now;

				gettimeofday(&now,NULL);
				to.tv_sec = now.tv_sec;
				to.tv_nsec = (now.tv_usec + 100) * 1000; // for wait receive timeout
				to.tv_sec += to.tv_nsec / 1000000000L;
				to.tv_nsec = to.tv_nsec % 1000000000L;

				pthread_mutex_lock(&(ctx->sr_mutex));
				pthread_cond_timedwait(&(ctx->playback_cond), &(ctx->sr_mutex), &to);
				pthread_mutex_unlock(&(ctx->sr_mutex));
			}

			if(ctx->atk_capture_handle) 
				ATK_AudioCap_Release(ctx->atk_capture_handle);
			if(ctx->atk_playback_handle) 
				ATK_AudioPlay_Release(ctx->atk_playback_handle);

			MsgBroker_SuspendAckMsg();
		}else if( !strcasecmp(msg->pszCmd, RESUME_CMD) ) {
			if (ctx->atk_capture_handle)
				ctx->atk_capture_handle = ATK_AudioCap_Init(&ctx->atk_capture_config);
			if (ctx->atk_playback_handle)
				ctx->atk_playback_handle = ATK_AudioPlay_Init(&ctx->atk_playback_config);
			
			pthread_mutex_lock(&(ctx->sr_mutex));
			ctx->sr_status = START;
			pthread_cond_signal(&(ctx->playback_cond));
			pthread_mutex_unlock(&(ctx->sr_mutex));
		}
	}

	if (msg->bHasResponse)
		msg->dwDataSize = 0;
}

static void audio_server_audiocapture_callback(const ATK_AUDIO_NOTIFY_DATA_INFO_T *audio_info, void* user_data)
{
 	
	if (!user_data) return;
	twa_ctx* ctx = (twa_ctx*) user_data;

	int send_bytes = -1;
	int send_data_size = -1;
	int length = sizeof(ctx->client_sockaddr_in); //for UDP
	if (ctx->terminate)
	{
		/* Because there is no disconnection in UDP, it is need to do something to close server. */
		//printf("bTerminate = 1 \n");
		return;
	}

	if (1 == ctx->transfer_type && ctx->client_sockfd < 0)
	{
		printf("[%s] TCP client_sockfd < 0 \n", __func__);
		return;
	}

	if (ctx->sr_status == STOP) {
		pthread_mutex_lock(&(ctx->sr_mutex));	
		pthread_cond_signal(&(ctx->capture_cond));	
		pthread_mutex_unlock(&(ctx->sr_mutex));	
		return;
	}

	ctx->atk_capture_buf_bytes = audio_info->dwDataBytes;
	if  (audio_info->bIsInterleaved)
	{
		if(ctx->codec_type == PCM){
			ctx->audioenc_out_frame_config[0].pbyOutBuf = audio_info->ppbyAudioBufs[0];
			send_data_size = audio_info->dwDataBytes;
		}
		else
		{	
			// Encode the audio data.
			ctx->audioenc_out_frame_config[0].pbyInBuf = audio_info->ppbyAudioBufs[0];
			send_data_size = ATK_AudioEnc_EncodeOneFrame(ctx->audioenc_handle[0], &(ctx->audioenc_out_frame_config[0]));
		}	
		//assert((size_t)send_data_size == audioenc_out_frame_config[0].dwOutBufSize);
		if(send_data_size > 0)
		{
			if (1 == ctx->transfer_type) //TCP
				send_bytes = send(ctx->client_sockfd, ctx->audioenc_out_frame_config[0].pbyOutBuf, send_data_size, 0);
			else {
				//! cellphone mode
				if(ctx->udp_server_ip) {
					struct sockaddr_in my_sockaddr_in;
					memset(&my_sockaddr_in, 0, sizeof(struct sockaddr_in));
					my_sockaddr_in.sin_family = AF_INET;
					my_sockaddr_in.sin_port = htons(ctx->server_port);
					my_sockaddr_in.sin_addr.s_addr = inet_addr(ctx->udp_server_ip);					
					int offset = 0;
					//bool is_leftover = 0;							
					
					//! due to MTU
					while(send_data_size > 0) {		
						if(send_data_size/320 > 0)
							send_bytes = sendto(ctx->server_sockfd, ctx->audioenc_out_frame_config[0].pbyOutBuf + offset, 320, 0, (struct sockaddr*)&my_sockaddr_in, sizeof(struct sockaddr_in));							
						else
							send_bytes = sendto(ctx->server_sockfd, ctx->audioenc_out_frame_config[0].pbyOutBuf + offset, send_data_size%320, 0, (struct sockaddr*)&my_sockaddr_in, sizeof(struct sockaddr_in));

						if(-1 == send_bytes)
							break;
						
						offset += send_bytes;
						send_data_size -= send_bytes;
					}				
				}
				else {
					send_bytes = sendto(ctx->server_sockfd, ctx->audioenc_out_frame_config[0].pbyOutBuf, send_data_size, 0, (struct sockaddr*)&ctx->client_sockaddr_in, length);
				}
			}
		}
		else
		{
			fprintf(stderr, "[%s, %u] Unable to encode audio data.\n", __func__, __LINE__);
			return;
		}

		if (send_bytes < 0)
		{
			printf("[%s] sending data to client failed, send bytes: %d, terminating ...\n", "audio_server", send_bytes);
			ctx->terminate = true;
		}

	}
	else
	{
		for(unsigned int i = 0; i < audio_info->dwChannels; ++i)
		{
			if(ctx->codec_type == PCM){
				ctx->audioenc_out_frame_config[0].pbyOutBuf = audio_info->ppbyAudioBufs[0];
				send_data_size = audio_info->dwDataBytes;
			}
			else
			{	
				// Encode the audio data.
				ctx->audioenc_out_frame_config[i].pbyInBuf = audio_info->ppbyAudioBufs[i];
				send_data_size = ATK_AudioEnc_EncodeOneFrame(ctx->audioenc_handle[i], ctx->audioenc_out_frame_config + i);
			}
			//assert((size_t)send_data_size == ctx->audioenc_out_frame_config[i].dwOutBufSize);
			
			if(send_data_size > 0)
			{
				if (1 == ctx->transfer_type) //TCP
					send_bytes = send(ctx->client_sockfd, ctx->audioenc_out_frame_config[i].pbyOutBuf, send_data_size, 0);
				else
					send_bytes = sendto(ctx->server_sockfd, ctx->audioenc_out_frame_config[i].pbyOutBuf, send_data_size, 0, (struct sockaddr*)&ctx->client_sockaddr_in, length);
			}
			else
			{
				return;
				fprintf(stderr, "[%s, %u] Unable to encode audio data.\n", __func__, __LINE__);
			}

			if (send_bytes < 0)
			{
				printf("[%s] sending data to client failed, send bytes: %d, terminating ...\n", "audio_server", send_bytes);
				ctx->terminate = true;
				break;
			}
		}
	}
}

 static void audio_client_audiocapture_callback(const ATK_AUDIO_NOTIFY_DATA_INFO_T *audio_info, void* user_data)
 {

	if (!user_data) return;
	twa_ctx* ctx = (twa_ctx*) user_data;

	int send_data_size = -1;
	int send_bytes = -1;
	
	if (ctx->terminate)
	{
		//printf("audio_client_audiocapture_callback: bTerminate \n");
		return;
	}
	
	if (ctx->client_sockfd < 0)
	{
		printf("audio_client_audiocapture_callback: client_sockfd<0 \n");
		return;
	}
	

	if (ctx->sr_status == STOP) {
		pthread_mutex_lock(&(ctx->sr_mutex));	
		pthread_cond_signal(&(ctx->capture_cond));	
		pthread_mutex_unlock(&(ctx->sr_mutex));	
		return;
	}

	ctx->atk_capture_buf_bytes = audio_info->dwDataBytes;
	
	if	(audio_info->bIsInterleaved)
	{
		if(ctx->codec_type == PCM){
			ctx->audioenc_out_frame_config[0].pbyOutBuf = audio_info->ppbyAudioBufs[0];
			send_data_size = audio_info->dwDataBytes;
		}
		else
		{	
			// Encode the audio data.
			ctx->audioenc_out_frame_config[0].pbyInBuf = audio_info->ppbyAudioBufs[0];
			send_data_size = ATK_AudioEnc_EncodeOneFrame(ctx->audioenc_handle[0], ctx->audioenc_out_frame_config);
		}
		//assert((size_t)send_data_size == ctx->audioenc_out_frame_config[0].dwOutBufSize);
		
		if(send_data_size > 0)
		{
			if (1 == ctx->transfer_type) //TCP
				send_bytes = send(ctx->client_sockfd, ctx->audioenc_out_frame_config[0].pbyOutBuf, send_data_size, 0);
			else
				send_bytes = sendto(ctx->client_sockfd, ctx->audioenc_out_frame_config[0].pbyOutBuf, send_data_size, 0, (struct sockaddr*)&ctx->server_sockaddr_in, sizeof(ctx->server_sockaddr_in));
		}
		else
		{			
			fprintf(stderr, "[%s, %u] Unable to encode audio data.\n", __func__, __LINE__);
			return;
		}

		if (send_bytes < 0)
		{
			printf("[%s] sending data to server failed, send bytes: %d, terminating ...\n", "audio_client", send_bytes);
			ctx->terminate = true;
		}
	}
	else
	{
		for(unsigned int i = 0; i < audio_info->dwChannels; ++i)
		{
			if(ctx->codec_type == PCM){
				ctx->audioenc_out_frame_config[0].pbyOutBuf = audio_info->ppbyAudioBufs[0];
				send_data_size = audio_info->dwDataBytes;
			}
			else
			{
				// Encode the audio data.
				ctx->audioenc_out_frame_config[i].pbyInBuf = audio_info->ppbyAudioBufs[i];
				send_data_size = ATK_AudioEnc_EncodeOneFrame(ctx->audioenc_handle[i], ctx->audioenc_out_frame_config + i);
			}
			//assert((size_t)send_data_size == ctx->audioenc_out_frame_config[i].dwOutBufSize);
			
			if(send_data_size > 0)
			{
				if (1 == ctx->transfer_type) //TCP
					send_bytes = send(ctx->client_sockfd, ctx->audioenc_out_frame_config[i].pbyOutBuf, send_data_size, 0);
				else
					send_bytes = sendto(ctx->client_sockfd, ctx->audioenc_out_frame_config[i].pbyOutBuf, send_data_size, 0, (struct sockaddr*)&ctx->server_sockaddr_in, sizeof(ctx->server_sockaddr_in));
			}
			else
			{
				return;
				fprintf(stderr, "[%s, %u] Unable to encode audio data.\n", __func__, __LINE__);
			}

			if (send_bytes < 0)
			{
				printf("[%s] sending data to server failed, send bytes: %d, terminating ...\n", "audio_client", send_bytes);
				ctx->terminate = true;
				break;
			}
		}
	}

 }

void init_decoder(decode_state* dec_state, twa_ctx* ctx) 
{
	switch (dec_state->codec_type) {// 0: AAC4, 1: G711, 2: G726, 3: PCM 
		case kAAC4: {
			unsigned int* buffer = (unsigned int*)calloc(1, 256);
			ATK_AUDIOPLAY_CONFIG_T* playback_config = &(ctx->atk_playback_config);
			AAC4_DECODER_INITOPT_T aac4dec_init_opt;
			memset(&aac4dec_init_opt, 0, sizeof(aac4dec_init_opt));
			ATK_AudioEnc_GetConf(ctx->audioenc_handle[0], buffer);
			//printf("[%s] AAC4 conf size: %d\n", __func__, buffer[6]);
			aac4dec_init_opt.dwChannels = playback_config->dwChannelsCount;
			aac4dec_init_opt.dwSampleRate = playback_config->dwSampleRate;
			aac4dec_init_opt.dwAdts = ctx->aac4_stream_type;
			if(0 != ctx->aac4_stream_type) {
				aac4dec_init_opt.dwSpecConfSize = buffer[6];
				aac4dec_init_opt.pbySpecConf = (unsigned char*)(buffer[7]);
			}			
			dec_state->dec_handle = (void*)Fdk_AAC4_Decoder_Init(&aac4dec_init_opt);	
			free(buffer);
			break;
		}
		case kG726:
			dec_state->dec_handle = (void*)g726_dec_init(ctx->bitrate);
			break;
	}
}

void release_decoder(decode_state* dec_state) 
{
	switch (dec_state->codec_type) {// 0: AAC4, 1: G711, 2: G726, 3: PCM 
		case kAAC4: {
			Fdk_AAC4_Decoder_Release((AAC4_DECODER_HANDLE_T* )dec_state->dec_handle);
			break;
		}
		case kG726:
			g726_dec_release((g726_dec_handle_t* )dec_state->dec_handle);
			break;
	}
}


void decode_audio(decode_state* dec_state) 
{
	switch (dec_state->codec_type) {// 0: AAC4, 1: G711, 2: G726, 3: PCM
		case kAAC4: {
			AAC4_DECODER_DECODED_STREAM_INFO_T stream_info;
			memset(&stream_info, 0, sizeof(AAC4_DECODER_DECODED_STREAM_INFO_T));
			AAC4_DECODER_HANDLE_T *aac4dec_handle = (AAC4_DECODER_HANDLE_T *)dec_state->dec_handle;
			Fdk_AAC4_Decoder_OneFrame(aac4dec_handle, dec_state->recv_buff, dec_state->recv_bytes, dec_state->decode_out_buf, dec_state->playback_buf_bytes);
			Fdk_AAC4_Decoder_Get_Decoded_Stream_Info(aac4dec_handle, &stream_info);
			dec_state->total_play_size = stream_info.dwChannels * stream_info.dwFrameSize * 2;	
			break;
		}
		case kG711:
			G711_ULaw_Decode(dec_state->recv_buff, (short*)(dec_state->decode_out_buf), dec_state->recv_bytes, dec_state->decode_step);
			dec_state->total_play_size = dec_state->recv_bytes * dec_state->decode_data_bytes_factor; // recv_bytes * decode_data_bytes_factor is for G.711
			break;
		case kG726:
			g726_dec_handle_t* g726_handle = (g726_dec_handle_t*)dec_state->dec_handle;
			g726_decode_frame(g726_handle, (int16_t*)dec_state->decode_out_buf, dec_state->recv_buff, dec_state->recv_bytes, dec_state->decode_step);	
			dec_state->total_play_size = dec_state->playback_buf_bytes;
			break;
	}
}

void *receive_and_playback_client_thread(void *data)
{
	RingBuffer *rb = NULL;
	int ret = -1;	
	twa_ctx* ctx = (twa_ctx*) data;
	ATK_AUDIOPLAY_CONFIG_T *atk_playback_config = &ctx->atk_playback_config;
	decode_state dec_state;
	memset(&dec_state, 0, sizeof(decode_state));
	dec_state.codec_type = ctx->codec_type;
	dec_state.decode_step = ((atk_playback_config->bIsInterleaved) && (atk_playback_config->dwChannelsCount > 1)) ? atk_playback_config->dwChannelsCount : 1;
	dec_state.decode_data_bytes_factor = (dec_state.decode_step << 1); // This is only for G.711

	int length = sizeof(ctx->server_sockaddr_in);	

	if (!ctx->atk_playback_handle)
	{
		fprintf(stderr, "[%s,%d] Failed: atk_playback_handle is NULL\n", __func__, __LINE__);
		return NULL;
	}	
	if(ctx->codec_type == PCM){
		ctx->atk_playback_buf_bytes = (ctx->atk_capture_buf_bytes > 0)?ctx->atk_capture_buf_bytes:4096;
		dec_state.playback_buf_bytes = ctx->audiodec_in_buf_size = ctx->atk_playback_buf_bytes;
	}
	else
	{	
		// Get total bytes of data in one period.
		ctx->period_frame_size = ATK_AudioPlay_GetPeriodFramesSize(ctx->atk_playback_handle);
		dec_state.playback_buf_bytes = ctx->atk_playback_buf_bytes = ctx->period_frame_size;
		printf("Audio period_frame_size = %d \n", ctx->period_frame_size);	
	}

	if (NULL == ctx->atk_playback_bufs[0])
	{		
		/* Get memory map buffer */
		if(ATK_AudioPlay_PlayPeriodFrames(ctx->atk_playback_handle, ctx->atk_playback_bufs) < 0)
		{
			fprintf(stderr, "[%s,%s] Can't get the audio output buffer..\n", __FILE__, __func__);
			return NULL;
		}		
	}
	
	dec_state.recv_buff = (unsigned char*) malloc(ctx->audiodec_in_buf_size);
	
	rb = RingBuffer_create(ctx->atk_playback_buf_bytes*2);		
	dec_state.decode_out_buf = (unsigned char*) malloc(ctx->atk_playback_buf_bytes);

	init_decoder(&dec_state, ctx);

	while (!ctx->terminate)
	{
		if (ctx->sr_status == STOP) {
			pthread_mutex_lock(&(ctx->sr_mutex));	
			pthread_cond_signal(&(ctx->playback_cond));	
			pthread_mutex_unlock(&(ctx->sr_mutex));	
			
			pthread_mutex_lock(&(ctx->sr_mutex));	
			pthread_cond_wait(&(ctx->playback_cond), &(ctx->sr_mutex));	
			pthread_mutex_unlock(&(ctx->sr_mutex));	
		}
		
		if (1 == ctx->transfer_type) //TCP
			dec_state.recv_bytes = recv(ctx->client_sockfd, dec_state.recv_buff, ctx->audiodec_in_buf_size, 0);
		else
			dec_state.recv_bytes = recvfrom(ctx->client_sockfd, dec_state.recv_buff, ctx->audiodec_in_buf_size, 0, (struct sockaddr*)&ctx->server_sockaddr_in, (socklen_t *)&length);
	
		if (dec_state.recv_bytes > 0)
		{
			if(ctx->codec_type == PCM)
			{
				ret = RingBuffer_write(rb, dec_state.recv_buff, dec_state.recv_bytes);
			}
			else
			{
	 			decode_audio(&dec_state);
				ret = RingBuffer_write(rb, dec_state.decode_out_buf, dec_state.total_play_size); // recv_bytes * decode_data_bytes_factor is for G.711
			}
			if(-1 == ret) printf("RingBuffer overflow \n");
			while(!ctx->terminate && ((int) ctx->period_frame_size == RingBuffer_read(rb, ctx->atk_playback_bufs[0], ctx->period_frame_size)))
			{
				if(ATK_AudioPlay_PlayPeriodFrames(ctx->atk_playback_handle, ctx->atk_playback_bufs) < 0)
				{
					fprintf(stderr, "[%s,%s] Can't output the audio data.\n", __FILE__, __func__);
					ctx->terminate = 1;
					break;
				}
			}
		}
		else
		{
			printf("[%s] receiving audio data from client failed, received bytes: %d, terminating ...\n", "audio_client", dec_state.recv_bytes);
			ctx->terminate = 1;
			break;
		}
	}

	if (dec_state.recv_buff)
	{
		free(dec_state.recv_buff);
		dec_state.recv_buff = NULL;
	}
	RingBuffer_destroy(rb);
	if(dec_state.decode_out_buf)
	{
		free(dec_state.decode_out_buf);
		dec_state.decode_out_buf = NULL;
	}

	release_decoder(&dec_state);

	return NULL;

}

void *receive_and_playback_server_thread(void *data)
{
	RingBuffer *rb = NULL;
	int ret = -1; 	
	twa_ctx* ctx = (twa_ctx*) data;
	ATK_AUDIOPLAY_CONFIG_T *atk_playback_config = &ctx->atk_playback_config; 	
	decode_state dec_state;
	memset(&dec_state, 0, sizeof(decode_state));
	dec_state.codec_type = ctx->codec_type;
	dec_state.decode_step = ((atk_playback_config->bIsInterleaved) && (atk_playback_config->dwChannelsCount > 1)) ? atk_playback_config->dwChannelsCount : 1;
	dec_state.decode_data_bytes_factor = (dec_state.decode_step << 1); // This is only for G.711

	int length = sizeof(ctx->server_sockaddr_in);	
	int udp_conn = 0;  //for UDP

	if (!ctx->atk_playback_handle)
	{
		fprintf(stderr, "[%s,%d] Failed: atk_playback_handle is NULL\n", __func__, __LINE__);
		return NULL;
	}	
	if(ctx->codec_type == PCM){
		ctx->atk_playback_buf_bytes = (ctx->atk_capture_buf_bytes > 0)?ctx->atk_capture_buf_bytes:4096;
		dec_state.playback_buf_bytes = ctx->audiodec_in_buf_size = ctx->atk_playback_buf_bytes;
	}
	else
	{		
		// Get total bytes of data in one period.
		ctx->period_frame_size = ATK_AudioPlay_GetPeriodFramesSize(ctx->atk_playback_handle);
		dec_state.playback_buf_bytes = ctx->atk_playback_buf_bytes = ctx->period_frame_size;
		printf("Audio period_frame_size = %d \n", ctx->period_frame_size);
	}

	if (NULL == ctx->atk_playback_bufs[0])
	{		
		/* Get memory map buffer */
		if(ATK_AudioPlay_PlayPeriodFrames(ctx->atk_playback_handle, ctx->atk_playback_bufs) < 0)
		{
			fprintf(stderr, "[%s,%s] Can't get the audio output buffer..\n", __FILE__, __func__);
			return NULL;
		}	
	}
	
	dec_state.recv_buff = (unsigned char*) malloc(ctx->audiodec_in_buf_size);

	rb = RingBuffer_create(ctx->atk_playback_buf_bytes*2);
	dec_state.decode_out_buf = (unsigned char*) malloc(ctx->atk_playback_buf_bytes);

	init_decoder(&dec_state, ctx);

	//! cellphone mode, udp waiting for ipc sendto (move to location before of recvfrom blocking due to two side wait for each other)
	if(ctx->udp_server_ip) {			
		if(0 == udp_conn)
		{
			// Initialize ATK audio capture.
			
			ctx->atk_capture_handle = ATK_AudioCap_Init(&ctx->atk_capture_config);
			if (NULL == ctx->atk_capture_handle)
			{
				fprintf(stderr, "[%s,%d] Can't initialize the audio capture.\n", __func__, __LINE__);
				ctx->terminate = true;
				udp_conn = 0;
			}
			else udp_conn = 1;
		}		
	}

	while (!ctx->terminate)
	{
		if (ctx->sr_status == STOP) {
			pthread_mutex_lock(&(ctx->sr_mutex));	
			pthread_cond_signal(&(ctx->playback_cond));	
			pthread_mutex_unlock(&(ctx->sr_mutex));	
			
			pthread_mutex_lock(&(ctx->sr_mutex));	
			pthread_cond_wait(&(ctx->playback_cond), &(ctx->sr_mutex));	
			pthread_mutex_unlock(&(ctx->sr_mutex));	
		}
		
		if (1 == ctx->transfer_type) //TCP
			dec_state.recv_bytes = recv(ctx->client_sockfd, dec_state.recv_buff, ctx->audiodec_in_buf_size, 0);
		else {
			dec_state.recv_bytes = recvfrom(ctx->server_sockfd, dec_state.recv_buff, ctx->audiodec_in_buf_size, 0, (struct sockaddr*)&ctx->client_sockaddr_in, (socklen_t *)&length);	
			if(!ctx->udp_server_ip) {			
				if(0 == udp_conn)
				{
					printf("client_sockaddr_in addr = %s", inet_ntoa(ctx->client_sockaddr_in.sin_addr));
					// Initialize ATK audio capture.
					
					ctx->atk_capture_handle = ATK_AudioCap_Init(&ctx->atk_capture_config);
					if (NULL == ctx->atk_capture_handle)
					{
						fprintf(stderr, "[%s,%d] Can't initialize the audio capture.\n", __func__, __LINE__);
						ctx->terminate = true;
						udp_conn = 0;
					}
					else udp_conn = 1;
				}		
			}			
		}

		if (dec_state.recv_bytes > 0)
		{
			//memset(ctx->atk_playback_bufs[0], 0, ctx->atk_playback_buf_bytes);
			if(ctx->codec_type == PCM)
			{
				ret = RingBuffer_write(rb, dec_state.recv_buff, dec_state.recv_bytes);
			}
			else
			{
				decode_audio(&dec_state);
				
				ret = RingBuffer_write(rb, dec_state.decode_out_buf, dec_state.total_play_size); 
			}
			if(-1 == ret) printf("RingBuffer overflow \n");
			while(!ctx->terminate && ((int) ctx->period_frame_size == RingBuffer_read(rb, ctx->atk_playback_bufs[0], ctx->period_frame_size)))
			{
				if(ATK_AudioPlay_PlayPeriodFrames(ctx->atk_playback_handle, ctx->atk_playback_bufs) < 0)
				{
					fprintf(stderr, "[%s,%s] Can't output the audio data.\n", __FILE__, __func__);
					ctx->terminate = 1;
					break;
				}
			}
		}
		else
		{
			printf("[%s] receiving audio data from client failed, received bytes: %d, terminating ...\n", "audio_server", dec_state.recv_bytes);
			ctx->terminate = 1;
			break;
		}
	}

	if (dec_state.recv_buff)
	{
		free(dec_state.recv_buff);
		dec_state.recv_buff = NULL;
	}
	RingBuffer_destroy(rb);
	if(dec_state.decode_out_buf)
	{
		free(dec_state.decode_out_buf);
		dec_state.decode_out_buf = NULL;
	}

	release_decoder(&dec_state);

	return NULL;
}

void print_ifaddrs()
{
	struct ifaddrs * ifaddr_list = NULL;
	struct ifaddrs * ifa = NULL;
	void * tmp_addr = NULL;
 
	getifaddrs(&ifaddr_list);
 
	printf("===== Server network interfaces =====\n");
	for (ifa = ifaddr_list; ifa != NULL; ifa = ifa->ifa_next)
	{
		if (!ifa->ifa_addr)
		{
			continue;
		}
		if (ifa->ifa_addr->sa_family == AF_INET) // IPv4
		{
			tmp_addr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
			char address_buf_ipv4[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, tmp_addr, address_buf_ipv4, INET_ADDRSTRLEN);
			printf("%s IPv4 Address %s\n", ifa->ifa_name, address_buf_ipv4);
		}
		else if (ifa->ifa_addr->sa_family == AF_INET6) // IPv6
		{
			tmp_addr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
			char address_buf_ipv6[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, tmp_addr, address_buf_ipv6, INET6_ADDRSTRLEN);
			printf("%s IPv6 Address %s\n", ifa->ifa_name, address_buf_ipv6);
		}
	}
	printf("===== Server network interfaces =====\n");
	if (ifaddr_list != NULL)
	{
		freeifaddrs(ifaddr_list);
	}
 
}

int twa_ctx_init(twa_ctx* ctx, MODEL type)
{
 	memset(ctx, 0, sizeof(twa_ctx));
 	ctx->model_type = type;
	 // setup default values for atk_capture_config, atk_playback_config, and some other defaults
	ATK_AUDIOCAP_CONFIG_T *atk_capture_config = &ctx ->atk_capture_config;

	atk_capture_config->szPcmName = "hw:0,0";
	atk_capture_config->bIsInterleaved = 1;
	atk_capture_config->eFormat = SND_PCM_FORMAT_S16_LE;
	atk_capture_config->dwChannelsCount = 2;
	atk_capture_config->dwSampleRate = 8000;
	atk_capture_config->bUseSimpleConfig = 0;
	atk_capture_config->dwPeriodsPerBuffer = 8;
	atk_capture_config->dwPeriodSizeInFrames = PERIOD_SIZE_IN_FRAMES;
	ATK_AUDIOPLAY_CONFIG_T *atk_playback_config = &ctx->atk_playback_config;

	atk_playback_config->szPcmName = "hw:0,0";
	atk_playback_config->bIsInterleaved = 1;
	atk_playback_config->eFormat = SND_PCM_FORMAT_S16_LE;
	atk_playback_config->dwChannelsCount = 2;
	atk_playback_config->dwSampleRate = 8000;
	atk_playback_config->bUseSimpleConfig = 0;
	atk_playback_config->dwPeriodsPerBuffer = 8;
	atk_playback_config->dwPeriodSizeInFrames = PERIOD_SIZE_IN_FRAMES;

	ctx->sizeof_client_sockaddr_in = sizeof(ctx->client_sockaddr_in);
	ctx->period_frame_size = 0;
	ctx->playback_volume = 100;
	ctx->server_port = 999;
	ctx->server_sockfd = -1;
	ctx->client_sockfd = -1;
	ctx->codec_type = kG711;
	ctx->bitrate = 32000;
	ctx->aac4_stream_type = 1;
	
	
	for (int i = 0; i < ATK_AUDIO_MAX_CHANNELS; i++)
		ctx->atk_playback_bufs[i] = NULL;

	ctx->atk_playback_handle = NULL;

	pthread_mutex_init(&(ctx->sr_mutex), NULL); 
	pthread_cond_init(&(ctx->capture_cond), NULL);
	pthread_cond_init(&(ctx->playback_cond), NULL);
	ctx->sr_status = START;
	
	return 0;
}
 
int twa_ctx_load_prog_args(int argc, char* argv[], twa_ctx* ctx)
{
	// getopt and setup atk_capture_config, atk_playback_config
	int opt = -1;
	ATK_AUDIOCAP_CONFIG_T *capture_config = &ctx->atk_capture_config;	
	ATK_AUDIOPLAY_CONFIG_T *playback_config = &ctx->atk_playback_config;

	while ((opt = getopt(argc, argv, "a:c:d:i:l:P:p:r:s:v:h:C:b:t:A:u:")) != -1)
	{
		switch(opt)
		{
			case 'a':
				ctx->server_ip = strdup(optarg);
				break;
			 case 'u':
				 ctx->udp_server_ip = strdup(optarg);
				 break;	
			case 'P':
				ctx->server_port = atoi(optarg);
				break;
			case 'c':
				capture_config->dwChannelsCount = playback_config->dwChannelsCount = atoi(optarg);
				break;
			case 'd':
			 	ctx->pcm_name = strdup(optarg);
				playback_config->szPcmName = capture_config->szPcmName =  ctx->pcm_name;
				break;
			case 'l':
			 	ctx->input_type = (atoi(optarg) != 0)? 1:0; 
				break;
			case 'i':
				capture_config->bIsInterleaved = playback_config->bIsInterleaved = atoi(optarg);
				break;
			case 'p':
				capture_config->dwPeriodsPerBuffer = playback_config->dwPeriodsPerBuffer = atoi(optarg);
				break;
			case 'r':
				capture_config->dwSampleRate = playback_config->dwSampleRate = atoi(optarg);
				break;
			case 's':
				capture_config->bUseSimpleConfig = playback_config->bUseSimpleConfig = atoi(optarg);
				break;
			case 't':
			 	ctx->transfer_type = atoi(optarg);
			case 'v':
			 	ctx->playback_volume = atoi(optarg);
				break;
			case 'A':
			 	ctx->aac4_stream_type = atoi(optarg);
				break;
			case 'C':				
				ctx->codec_type = atoi(optarg);				
				break;		 
			case 'b':				
				ctx->bitrate = atoi(optarg);				
				break;				
			case 'h':
				return -1;
				break;
			default:
				return -1;
				break;
		}
	}
	return 0;
}

 
 
int twa_ctx_audio_init(twa_ctx* ctx)
{
	// init atk_capture_handle, atk_capture_bufs, atk_capture_buf_bytes by atk_capture_config
	// init atk_playback_handle, atk_playback_bufs, atk_playback_buf_bytes by atk_playback_config

	int ret_val;
	unsigned int i; 
	ATK_AUDIOCAP_CONFIG_T *atk_capture_config = &ctx ->atk_capture_config;	
	
	//[5494] move AudioPlay_Init before than AudioCap_Init to work around playbacking underrun
	// Initialize ATK audio playback.	
	ctx->atk_playback_handle = ATK_AudioPlay_Init(&ctx->atk_playback_config);
	if (NULL == ctx->atk_playback_handle)
	{
		fprintf(stderr, "[%s,%d] Can't initialize the audio playback.\n", __func__, __LINE__);
		return -1;
	}

	ret_val = ATK_Audio_SetPlaybackVolume(ctx->playback_volume);
	if (ret_val)
	{
		fprintf(stderr, "[%s,%d] Can't set audio playback volume: %u.\n", __func__, __LINE__, ctx->playback_volume);
	}

	// Get the total bytes of data in one period.
	ctx->period_frame_size = ATK_AudioPlay_GetPeriodFramesSize(ctx->atk_playback_handle);
	
	ctx->audiodec_in_buf_size = ctx->period_frame_size;	 
	printf("[%s]period_frame_size: %d\n", __func__,  ctx->period_frame_size);
	//
	

	if(ctx ->model_type == SERVER_SITE)
		atk_capture_config->pfnCallback = audio_server_audiocapture_callback;
	else
		atk_capture_config->pfnCallback = audio_client_audiocapture_callback;
	atk_capture_config->pUserData = ctx;
	 
	if (ctx->input_type == 1)
	{
		ATK_Audio_InputSelection(kTKAudioLineIn);

		//set audio volume to 90
		ATK_Audio_SetCaptureVolume(90);
	}
	else
	{
		ATK_Audio_InputSelection(kTKAudioMicIn);

		//set audio volume to 90
		//ATK_Audio_SetCaptureVolume(90);
		ATK_Audio_SetCapturePga_dB(22);
	}
	if (1 == ctx->transfer_type) { //TCP
		// Initialize ATK audio capture.
		ctx->atk_capture_handle = ATK_AudioCap_Init(&ctx->atk_capture_config);
		if (NULL == ctx->atk_capture_handle)
		{
			fprintf(stderr, "[%s,%d] Can't initialize the audio capture.\n", __func__, __LINE__);
			return -1;
		}
	}
	else {

		if(ctx ->model_type == CLIENT_SITE){
			ctx->atk_capture_handle = ATK_AudioCap_Init(&ctx->atk_capture_config);
			if (NULL == ctx->atk_capture_handle)
			{
				fprintf(stderr, "[%s,%d] Can't initialize the audio capture.\n", __func__, __LINE__);
				return -1;
			}
		}
	}

	// Initialize audio encoder
	ATK_AUDIOENC_INITOPT_T audioenc_initopt;
	memset(&audioenc_initopt, 0, sizeof(ATK_AUDIOENC_INITOPT_T));
	audioenc_initopt.dwChannels = atk_capture_config->dwChannelsCount;
	audioenc_initopt.bIsInterleaved = (atk_capture_config->bIsInterleaved) ? 1 : 0;
	ATK_AAC4ENC_CONFIG_T aac4_config;
	ATK_G711ENC_CONFIG_T g711_config;
	ATK_G726ENC_CONFIG_T g726_config;

	if(ctx->codec_type != PCM)
	{
		switch(ctx->codec_type)
		{
			case kAAC4:
				memset(&aac4_config, 0, sizeof(ATK_AAC4ENC_CONFIG_T));
				audioenc_initopt.eType = kAAC4;
				audioenc_initopt.dwPeriodSizeInFrames = atk_capture_config->dwPeriodSizeInFrames = PERIOD_SIZE_IN_FRAMES_AAC;
				aac4_config.dwBitRate = ctx->bitrate; // 8000, 16000, 24000, 32000
				aac4_config.dwSampleRate = atk_capture_config->dwSampleRate;
				aac4_config.dwAdts = ctx->aac4_stream_type;
				aac4_config.dwStereoMode = 0;	
				break;
			case kG711: 	
				audioenc_initopt.eType = kG711;
				audioenc_initopt.dwPeriodSizeInFrames = atk_capture_config->dwPeriodSizeInFrames;
				memset(&g711_config, 0, sizeof(ATK_G711ENC_CONFIG_T));
				g711_config.iCompressionMode = 0; /* It can be 0: ulaw or 1: alaw. */
				break;
			case kG726: 
				memset(&g726_config, 0, sizeof(ATK_G726ENC_CONFIG_T));
				audioenc_initopt.eType = kG726;
				audioenc_initopt.dwPeriodSizeInFrames = atk_capture_config->dwPeriodSizeInFrames;
				g726_config.dwBitRate = ctx->bitrate; // 16000, 24000, 32000, 40000

				break;
		}

		for(i = 0; i < atk_capture_config->dwChannelsCount; ++i)
		{
			switch(ctx->codec_type)
			{
				case kAAC4:
					ctx->audioenc_handle[i] = ATK_AudioEnc_Init(&audioenc_initopt, &aac4_config);
					break;
				
				case kG711:
					ctx->audioenc_handle[i] = ATK_AudioEnc_Init(&audioenc_initopt, &g711_config);
					break;
					
				case kG726:
					ctx->audioenc_handle[i] = ATK_AudioEnc_Init(&audioenc_initopt, &g726_config);
					break;	
			}
			if (ctx->audioenc_handle[i] == NULL)
			{
				fprintf(stderr, "[%s,%d] Unable to initialize the audio encoder.\n", __func__, __LINE__);
				return -1;
			}

			// Setup the output buffer for audio encoder.
			// Because we use SND_PCM_FORMAT_S16_LE and period size is PERIOD_SIZE_IN_FRAMES in capture, the buffer size is PERIOD_SIZE_IN_FRAMES bytes for G.711.
			ctx->audioenc_out_frame_config[i].dwOutBufSize = (kAAC4 == ctx->codec_type)?MAX_ENCODE_DATA_SIZE:PERIOD_SIZE_IN_FRAMES;
			if(kGAMR != ctx->codec_type && kGPCM != ctx->codec_type)
				ctx->audioenc_out_frame_config[i].pbyOutBuf = (unsigned char*) MemBroker_GetMemory(ctx->audioenc_out_frame_config[i].dwOutBufSize, VMF_ALIGN_TYPE_128_BYTE);
			if(ctx->audioenc_out_frame_config[i].pbyOutBuf == NULL)
			{
				fprintf(stderr, "[%s,%d] Unable to allocate memory for the output of audio encoder.\n", __func__, __LINE__);
				return -1;
			}

			if(atk_capture_config->bIsInterleaved)
			{
				break;
			}
		}
	}	

	// Setup the input buffer for audio decoder.
	// Because we use SND_PCM_FORMAT_S16_LE and period size is PERIOD_SIZE_IN_FRAMES, the buffer size is PERIOD_SIZE_IN_FRAMES bytes for G.711.

	return 0;
}
 
int twa_ctx_audio_release(twa_ctx* ctx)
{
	// release atk_playback_handle, atk_playback_bufs
	// release atk_capture_handle, atk_capture_bufs
	int i;

	for (i = 0; i < ATK_AUDIO_MAX_CHANNELS; i++)
	{
		if (ctx->atk_playback_bufs[i])
		{
			ctx->atk_playback_bufs[i] = NULL;
		}
	}

	if (ctx->atk_playback_handle)
	{
		ATK_AudioPlay_Release(ctx->atk_playback_handle);
		ctx->atk_playback_handle = NULL;
	}

	if (ctx->atk_capture_handle)
	{
		ATK_AudioCap_Release(ctx->atk_capture_handle);
		ctx->atk_capture_handle = NULL;
	}

	if (ctx->pcm_name)
	{
		free(ctx->pcm_name);
		ctx->pcm_name = NULL;
	}

	for(i = 0; i < ATK_AUDIO_MAX_CHANNELS; ++i)
	{
		if (ctx->audioenc_handle[i])
		{
			ATK_AudioEnc_Release(ctx->audioenc_handle[i]);
			ctx->audioenc_handle[i] = NULL;
		}
		if(kGAMR != ctx->codec_type && kGPCM != ctx->codec_type && ctx->audioenc_out_frame_config[i].pbyOutBuf)
		{
			MemBroker_FreeMemory(ctx->audioenc_out_frame_config[i].pbyOutBuf);
			ctx->audioenc_out_frame_config[i].pbyOutBuf = NULL;
		}
	}

	pthread_mutex_destroy(&(ctx->sr_mutex));
	pthread_cond_destroy(&(ctx->capture_cond));
	pthread_cond_destroy(&(ctx->playback_cond));
	
	return -1;
}
 
int twa_ctx_network_init(twa_ctx* ctx)
{
	 // init server_sockfd, client_sockfd
	 ctx->enable_noDelay = 1;
	 struct sockaddr_in* server_sockaddr_in = &ctx->server_sockaddr_in;
	 struct sockaddr_in* client_sockaddr_in = &ctx->client_sockaddr_in;
	 
	if(ctx->model_type == CLIENT_SITE){
		 
		printf("[%s] connecting to server: %s:%d ...\n", "audio_client", ctx->server_ip, ctx->server_port);
		if (1 == ctx->transfer_type) {//TCP
			ctx->client_sockfd = socket(AF_INET, SOCK_STREAM, 0);

			//Set TCP connection NODELAY to improve efficiency of send()
			if(0 > (setsockopt(ctx->client_sockfd, IPPROTO_TCP, TCP_NODELAY, (void*)&ctx->enable_noDelay, sizeof(ctx->enable_noDelay))))
			{
				printf("set TCP_NODELAY error \n");
				return -1;
			}
		}
		else {
			ctx->client_sockfd = socket(AF_INET, SOCK_DGRAM, 0);			
		}

		if (ctx->client_sockfd < 0) {
			fprintf(stderr, "[%s] failed to initialize socket", "audio_client");
			return -1;
		}
		memset(client_sockaddr_in, 0, sizeof(struct sockaddr_in));
		client_sockaddr_in->sin_family = AF_INET;
		client_sockaddr_in->sin_port = htons(ctx->server_port);			

		if (1 == ctx->transfer_type) {	//TCP
			client_sockaddr_in->sin_addr.s_addr = inet_addr(ctx->server_ip);
			if (connect(ctx->client_sockfd, (struct sockaddr*)client_sockaddr_in, sizeof(struct sockaddr_in)) < 0) {
				fprintf(stderr, "[%s] failed to connect to server", "audio_client");
				return -1;
			}
		}
		else {
			client_sockaddr_in->sin_addr.s_addr = htonl(INADDR_ANY);
			if (bind(ctx->client_sockfd, (struct sockaddr *)client_sockaddr_in, sizeof(struct sockaddr_in)) <0) {
				perror("bind failed!");
				return -1;
			}

			memset(server_sockaddr_in, 0, sizeof(struct sockaddr_in));
			server_sockaddr_in->sin_family = AF_INET;
			server_sockaddr_in->sin_port = htons(ctx->server_port);
			server_sockaddr_in->sin_addr.s_addr = inet_addr(ctx->server_ip);
		}
	}
	else {

		if (1 == ctx->transfer_type) //TCP
			ctx->server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
		else
			ctx->server_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	 		
		 if (ctx->server_sockfd < 0)
		 {
			 fprintf(stderr, "[%s] failed to initialize socket", "audio_server");
			 return -1;
		 }
		 memset(server_sockaddr_in, 0, sizeof(struct sockaddr_in));
		 server_sockaddr_in->sin_family = AF_INET;
		 server_sockaddr_in->sin_port = htons(ctx->server_port);
		 server_sockaddr_in->sin_addr.s_addr = htonl(INADDR_ANY);
		 if (bind(ctx->server_sockfd, (struct sockaddr *)server_sockaddr_in, sizeof(struct sockaddr_in)) <0) {
			 printf ("server bind failed \n");
			 return -1;
		 }
		 memset(client_sockaddr_in, 0, sizeof(struct sockaddr_in));
		if (1 == ctx->transfer_type) { //TCP
			listen(ctx->server_sockfd, MAX_CLIENT_CONNECTIONS);

			print_ifaddrs();
			printf("[%s] waiting for client connection on port %d ...\n", "audio_server", ctx->server_port);
			ctx->client_sockfd = accept(ctx->server_sockfd, (struct sockaddr*)client_sockaddr_in, &ctx->sizeof_client_sockaddr_in);

			if (ctx->client_sockfd < 0)
			{
			 fprintf(stderr, "[%s] (%s, %d) failed to initialize client socket", "audio_server", __func__, __LINE__);
			 return -1;
			}

			//Set TCP connection NODELAY to improve efficiency of send()
			if(0 > (setsockopt(ctx->client_sockfd, IPPROTO_TCP, TCP_NODELAY, (void*)&ctx->enable_noDelay, sizeof(ctx->enable_noDelay))))
			{
			 printf("set TCP_NODELAY error \n");
			 return -1;
			}
		}
		int enable = 1;
		if (setsockopt(ctx->server_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
			printf("setsockopt(SO_REUSEADDR) failed \n");
	}
	return 0;
}

int twa_ctx_network_release(twa_ctx* ctx)
{
	// release server_sockfd, client_sockfd
	if (ctx->client_sockfd >= 0)
	{
		close(ctx->client_sockfd);
		ctx->client_sockfd = -1;
	}
	if (ctx->server_ip)
	{
		free(ctx->server_ip);
		ctx->server_ip = NULL;
	}	
	if (ctx->server_sockfd >= 0)
	{
		close(ctx->server_sockfd);
		ctx->server_sockfd = -1;
	}
	return -1;
}
 
int twa_ctx_start(twa_ctx* ctx)
{
	pthread_t playback_tid = 0;
	// create recv_and_playback_thread by pthread_create
	if(ctx->model_type == CLIENT_SITE)
		pthread_create(&playback_tid, NULL, receive_and_playback_client_thread, (void*)ctx);
	else
		pthread_create(&playback_tid, NULL, receive_and_playback_server_thread, (void*)ctx);

	MsgBroker_RegisterMsg(TWOWAY_CMD_FIFO);
	MsgBroker_Run(TWOWAY_CMD_FIFO, msg_callback, (void*)ctx, (int *)&ctx->terminate);
	MsgBroker_UnRegisterMsg();
	
	pthread_join(playback_tid, NULL);
	 
	return 0;
}
 
int twa_ctx_stop(twa_ctx* ctx)
{
	// setup terminate flag and wait for thread completion by pthread_join
	printf("exit_process \n");
	ctx->terminate = 1;
	printf("exit_process bTerminate = 1;\n");
	if (ctx->client_sockfd >= 0)
	{
		close(ctx->client_sockfd);
		ctx->client_sockfd = -1;		
		printf("exit_process close(client_sockfd);\n");
	}
	if (ctx->server_sockfd >= 0)
	{
		shutdown(ctx->server_sockfd, SHUT_RDWR);
		close(ctx->server_sockfd);
		ctx->server_sockfd = -1;
		printf("exit_process close(server_sockfd);\n");
	} 
	return -1;
}
 
 

