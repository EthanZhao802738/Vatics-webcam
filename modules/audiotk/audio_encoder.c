
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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <audio_encoder.h>
#ifdef HAS_HW_AAC_ENC
#include <vmf/audio_aac4enc.h>
#else
#include <fdk-aac/aacenc_lib.h>
#endif


#ifdef GAMR_SUPPORT
#include <opencore/interf_enc.h> //! For GAMR encoder.
#endif

#include <g711/G711SEnc.h>
#include <g726/G726SEnc.h>


typedef int (*Enc_Release) (void** handle);
typedef int (*Enc_EncodeOneFrame) (void* handle, ATK_AUDIOENC_ONEFRAME_CONF_T* conf);
typedef int (*Enc_GetConf) (void* handle, void *conf_buf);

struct ATK_AUDIOENC_HANDLE_T
{

#ifdef HAS_HW_AAC_ENC
	VMF_AAC4ENC_HANDLE_T* handle_; 
#else
	HANDLE_AACENCODER handle_; /**< The handle of audio encoders (FDK). */
#endif

	unsigned int aac_output_channels_;

	int g711_mode_;
	unsigned int gamr_mode_;
	unsigned int step_offset_;

	Enc_Release release_func; /**< The callback function of one encoder for release resource. */
	Enc_EncodeOneFrame enc_func; /**< The callback function of one encoder for encode data. */
	Enc_GetConf getconf_func; /**< The callback function of one encoder for get configuration. */

	unsigned int sample_rate_; /**< Sample rate. */
	unsigned int channels_; /**< The number of channels. */
	unsigned int bit_rate_; /**< The number of channels. */
	unsigned int is_interleaved_; /**< the input data is interleaved or not. */

	unsigned int period_size_in_frames; /**< Distance between interrupts is # frames. */
	void* pcmbuffer;
};



#ifdef HAS_HW_AAC_ENC
/**
 * @brief Function to initialize AAC4 audio encoder.
 *
 * @param[in] aac4enc_handle The handle of AAC4 audio encoder.
 * @param[in] config Initial configuration for audio encoder.
 * @return negative: failed, otherwise: success.
 */
static int ATK_AAC4Enc_Init(ATK_AUDIOENC_HANDLE_T *aac4enc_handle, const ATK_AAC4ENC_CONFIG_T *config)
{
	VMF_AAC4ENC_INITOPT_T tVmfAac4encInit;
	memset(&tVmfAac4encInit, 0, sizeof(VMF_AAC4ENC_INITOPT_T));
	tVmfAac4encInit.dwBitRate = config->dwBitRate;
	tVmfAac4encInit.dwSampleRate = aac4enc_handle->sample_rate_ = config->dwSampleRate;
	tVmfAac4encInit.dwADTS = config->dwAdts; // 0: Raw, 1: ADTS or 2: ADIF
	if( (aac4enc_handle->channels_ == 2) && (aac4enc_handle->is_interleaved_) )
	{
		// stereo
		tVmfAac4encInit.dwChannel = 2;
	}
	else if( (aac4enc_handle->channels_ == 1) || (aac4enc_handle->is_interleaved_ == 0) )
	{
		// mono
		tVmfAac4encInit.dwChannel = 1;
	}
	else
	{
		fprintf(stderr, "[%s, %s]: Unsupport channel number for AAC4 encoder.\n", __FILE__, __func__);
		return -1;
	}

	switch(config->dwStereoMode)
	{
		case 0: // stereo
		case 1: // joint stereo
			if (tVmfAac4encInit.dwChannel == 1)
			{
				fprintf(stderr, "[%s, %s]: The output channels of AAC is large than the input channels.\n", __FILE__, __func__);
				return -1;
			}

			aac4enc_handle->aac_output_channels_ = 2;
			tVmfAac4encInit.dwStereoMode = config->dwStereoMode;
			break;
		case 3: // mono
			aac4enc_handle->aac_output_channels_ = 1;
			tVmfAac4encInit.dwStereoMode = config->dwStereoMode;
			break;
		default:
			fprintf(stderr, "[%s, %s]: Unsupport stereo mode for AAC4 encoder.\n", __FILE__, __func__);
			return -1;
	}

	aac4enc_handle->handle_ = VMF_AAC4ENC_Init(&tVmfAac4encInit);
	if (!aac4enc_handle->handle_) {
		fprintf(stderr, "[%s, %s]: AAC4 init failed.\n", __FILE__, __func__);
		return -1;
	}

	return 0;
}

/**
 * @brief Function to encode one audio frame.
 *
 * @param[in] handle The handle of audio encoder (tk).
 * @param[in] pbyInBuf The input data buffer,
 * @param[out] pbyOutBuf The output encoded data buffer.
 * @param[in] dwOutBufSize The size of output data buffer.
 * @return negative: failed, otherwise: The size of encoded data (bytes).
 */
static int ATK_AAC4Enc_EncodeOneFrame(ATK_AUDIOENC_HANDLE_T *handle, ATK_AUDIOENC_ONEFRAME_CONF_T* conf)
{	
	VMF_AAC4ENC_ONEFRAME_CONF_T tVmfAac4OneFrameConf;
	VMF_AAC4ENC_HANDLE_T *ptAac4encHandle = handle->handle_;
	memset(&tVmfAac4OneFrameConf, 0, sizeof(VMF_AAC4ENC_ONEFRAME_CONF_T));
	tVmfAac4OneFrameConf.pbyInBuf = conf->pbyInBuf;
	tVmfAac4OneFrameConf.pbyOutBuf = conf->pbyOutBuf;
	tVmfAac4OneFrameConf.dwOutBufSize = conf->dwOutBufSize;
	if (0 != VMF_AAC4ENC_SetOptions(ptAac4encHandle, &tVmfAac4OneFrameConf))
	{
		fprintf(stderr, "[%s, %s]: Unable to set options for encoder.\n", __FILE__, __func__);
		return -1;
	}

	if (0 != VMF_AAC4ENC_PorcessOneFrame(ptAac4encHandle))
	{
		fprintf(stderr, "[%s, %s]: AAC4Enc_OneFrame failed.\n", __FILE__, __func__);
		return -1;
	}

	return VMF_AAC4ENC_GetBitStreamSize(ptAac4encHandle);
}

/**
 * @brief Function to get information about the encoder.
 *
 * @param[in] handle The handle of audio encoder (tk).
 * @param[out] conf_buf The buffer which to store the information.
 * @return negative: failed, otherwise: success.
 */
static int ATK_AAC4Enc_GetConf(ATK_AUDIOENC_HANDLE_T *handle, void *conf_buf)
{
	//---header
	//dwDataType (4 bytes)
	//payload len (4 bytes)

	//--paylaod
	//codec_type (4 bytes)
	//sameple rate (4 bytes)
	//channel num (4 bytes)
	//dwProfileLevel (4 bytes)
	//aac header size (4 bytes)
	//aac header data

	unsigned int *values = (unsigned int*) conf_buf;
	uint32_t dwSpecConfSize = 0;
	if (0 != VMF_AAC4ENC_GetConf(handle->handle_, conf_buf, &dwSpecConfSize, values))
	{
		return -1;
	}

	values[0] = FOURCC_CONF;
	values[1] = 20 + dwSpecConfSize;
	values[2] = FOURCC_AAC4;
	values[3] = handle->sample_rate_;
	values[4] = handle->aac_output_channels_;
	values[6] = dwSpecConfSize;

	return 0;
}


#else

/**
 * @brief Function to initialize AAC4 audio encoder.
 *
 * @param[in] aac4enc_handle The handle of AAC4 audio encoder.
 * @param[in] config Initial configuration for audio encoder.
 * @return negative: failed, otherwise: success.
 */
static int ATK_AAC4Enc_Init(ATK_AUDIOENC_HANDLE_T *aac4enc_handle, const ATK_AAC4ENC_CONFIG_T *config)
{

	int format = 0;

	if (config->dwAdts == 1)
		format = 2;
	else if(config->dwAdts == 2)
		format = 1;
	
	switch(config->dwStereoMode)
	{
		case 0: // stereo
		case 1: // joint stereo
			aac4enc_handle->aac_output_channels_ = 2;
			break;
		case 3: // mono
			aac4enc_handle->aac_output_channels_ = 1;
			break;
		default:
			fprintf(stderr, "[%s, %s]: Unsupport stereo mode for AAC4 encoder.\n", __FILE__, __func__);
			return -1;
	}

	if (aacEncOpen(&(aac4enc_handle->handle_), 0, aac4enc_handle->channels_) != AACENC_OK) {
		fprintf(stderr, "[%s, %s]: Unable to open encoder.\n", __FILE__, __func__);
		return -1;
	}
	if (aacEncoder_SetParam(aac4enc_handle->handle_, AACENC_SAMPLERATE, config->dwSampleRate) != AACENC_OK) {
		fprintf(stderr, "[%s, %s]: Unable to set the AOT.\n", __FILE__, __func__);
		return -1;
	}
	if (aacEncoder_SetParam(aac4enc_handle->handle_, AACENC_CHANNELMODE, aac4enc_handle->channels_) != AACENC_OK) {
		fprintf(stderr, "[%s, %s]: Unable to set the channel mode.\n", __FILE__, __func__);
		return -1;
	}
	if (aacEncoder_SetParam(aac4enc_handle->handle_, AACENC_CHANNELORDER, 1) != AACENC_OK) {
		fprintf(stderr, "[%s, %s]: Unable to set the wav channel order.\n", __FILE__, __func__);
		return -1;
	}
	if (aacEncoder_SetParam(aac4enc_handle->handle_, AACENC_BITRATE, config->dwBitRate) != AACENC_OK) {
		fprintf(stderr, "[%s, %s]: Unable to set the bitrate.\n", __FILE__, __func__); //now bitrate is VBR in low level, so this not work. 
		return -1;
	}
	if (aacEncoder_SetParam(aac4enc_handle->handle_, AACENC_TRANSMUX, format) != AACENC_OK) {
		fprintf(stderr, "[%s, %s]: Unable to set the ADTS transmux.\n", __FILE__, __func__); //now bitrate is VBR in low level, so this not work. 		
		return -1;
	}
	if (aacEncEncode(aac4enc_handle->handle_, NULL, NULL, NULL, NULL) != AACENC_OK) {
		fprintf(stderr, "[%s, %s]: Unable to initialize the encoder.\n", __FILE__, __func__); 
		return -1;
	}
	aac4enc_handle->sample_rate_ = config->dwSampleRate;
	
	return 0;
}

/**
 * @brief Function to release encoder.
 *
 * @param[in] ptHandle The handle of AAC encoder.
 * @return Success: 0  Fail: negative integer.
 */
static int ATK_AACEnc_Release(void** handle)
{
	if (aacEncClose((HANDLE_AACENCODER *)&handle) != 0) {
		fprintf(stderr, "[%s, %s]: Release the encoder fail.\n", __FILE__, __func__);	
		return -1;
	}
	*handle = NULL;
	return 0;

}

/**
 * @brief Function to get information about the encoder.
 *
 * @param[in] handle The handle of audio encoder (tk).
 * @param[out] conf_buf The buffer which to store the information.
 * @return negative: failed, otherwise: success.
 */
static int ATK_AAC4Enc_GetConf(ATK_AUDIOENC_HANDLE_T *handle, void *conf_buf)
{
	//---header
	//dwDataType (4 bytes)
	//payload len (4 bytes)

	//--paylaod
	//codec_type (4 bytes)
	//sameple rate (4 bytes)
	//channel num (4 bytes)
	//dwProfileLevel (4 bytes)
	//aac header size (4 bytes)
	//aac header data

	AACENC_InfoStruct info;
	memset(&info, 0, sizeof(AACENC_InfoStruct
));
	
	unsigned int *values = (unsigned int*) conf_buf;

	if (aacEncInfo(handle->handle_, &info) != AACENC_OK) {
		fprintf(stderr, "[%s, %s]: Unable to get the encoder info.\n", __FILE__, __func__);
		return -1;
	}
	handle->period_size_in_frames = info.frameLength;
	
	values[0] = FOURCC_CONF;
	values[1] = 20 + info.confSize;
	values[2] = FOURCC_AAC4;
	values[3] = handle->sample_rate_;
	values[4] = handle->aac_output_channels_;
	values[6] = info.confSize;
	memcpy(&values[7], info.confBuf, info.confSize);

	
	return 0;
}

/**
 * @brief Function to encode one audio frame.
 *
 * @param[in] handle The handle of audio encoder (tk).
 * @param[in] pbyInBuf The input data buffer,
 * @param[out] pbyOutBuf The output encoded data buffer.
 * @param[in] dwOutBufSize The size of output data buffer.
 * @return negative: failed, otherwise: The size of encoded data (bytes).
 */
static int ATK_AAC4Enc_EncodeOneFrame(ATK_AUDIOENC_HANDLE_T *handle, ATK_AUDIOENC_ONEFRAME_CONF_T* conf)
{
	
	int input_size = 2*handle->channels_*handle->period_size_in_frames; //16bits * channel * frames (bytes)

	AACENC_BufDesc in_buf = { 0 }, out_buf = { 0 };
	AACENC_InArgs in_args = { 0 };
	AACENC_OutArgs out_args = { 0 };
	int in_identifier = IN_AUDIO_DATA;
	int in_elem_size;
	int out_identifier = OUT_BITSTREAM_DATA;
	int out_size;//, out_elem_size;
	void *in_ptr, *out_ptr;
	AACENC_ERROR err;
	in_ptr = conf->pbyInBuf;
	in_elem_size = 16/8; //16 bits = 2bytes fixed

	in_args.numInSamples = input_size/2;
	in_buf.numBufs = 1;
	in_buf.bufs = &in_ptr;
	in_buf.bufferIdentifiers = &in_identifier;
	in_buf.bufSizes = &input_size;
	in_buf.bufElSizes = &in_elem_size;

	out_ptr = conf->pbyOutBuf;
	out_size = conf->dwOutBufSize;
	out_buf.numBufs = 1;
	out_buf.bufs = &out_ptr;
	out_buf.bufferIdentifiers = &out_identifier;
	out_buf.bufSizes = &out_size;
	out_buf.bufElSizes = (signed int *)&handle->aac_output_channels_;

	if ((err = aacEncEncode(handle->handle_, &in_buf, &out_buf, &in_args, &out_args)) != AACENC_OK) {
		fprintf(stderr, "[%s, %s]: Encoding failed err code 0x%x.\n", __FILE__, __func__,err);
		return -1;
	}

	return out_args.numOutBytes;

}
#endif


#ifdef GAMR_SUPPORT
// ==========================================================================
/**
 * @brief Function to initialize GAMR audio encoder.
 *
 * @param[in] aac4enc_handle The handle of GAMR audio encoder.
 * @param[in] config Initial configuration for audio encoder.
 * @return negative: failed, otherwise: success.
 */
static int ATK_GAMREnc_Init(ATK_AUDIOENC_HANDLE_T *gamrenc_handle, const ATK_GAMRENC_CONFIG_T *config)
{
	gamrenc_handle->sample_rate_ = config->dwSampleRate;
	gamrenc_handle->bit_rate_ = config->dwBitRate;
	gamrenc_handle->gamr_mode_ = config->dwMode;
	gamrenc_handle->handle_ = Encoder_Interface_init(0);//dtx
	return (gamrenc_handle->handle_ != NULL) ? 0 : -1;
}

/**
 * @brief Function to encode one audio frame.
 *
 * @param[in] handle The handle of audio encoder (tk).
 * @param[in] in_buf The input data buffer,
 * @param[out] out_buf The output encoded data buffer.
 * @param[in] out_buf_size The size of output data buffer.
 * @return negative: failed, otherwise: The size of encoded data (bytes).
 */
static int ATK_GAMREnc_EncodeOneFrame(ATK_AUDIOENC_HANDLE_T *handle, ATK_AUDIOENC_ONEFRAME_CONF_T* conf)
{
	size_t outputSize = 0;
	short* pcmbuffer = (short*)handle->pcmbuffer;
	short* tmpIn = (short*)conf->pbyInBuf;
	for (unsigned int i=0; i < handle->period_size_in_frames; i++)
	{
		pcmbuffer[i] = tmpIn[i << 1];
	}

	outputSize = Encoder_Interface_Encode(handle->handle_, handle->gamr_mode_, pcmbuffer, conf->pbyOutBuf, 0);
	return outputSize;
}

/**
 * @brief Function to get information about the encoder.
 *
 * @param[in] handle The handle of audio encoder (tk).
 * @param[out] conf_buf The buffer which to store the information.
 * @return negative: failed, otherwise: success.
 */
static int ATK_GAMREnc_GetConf(ATK_AUDIOENC_HANDLE_T *handle, void *conf_buf)
{
	//---header
	//dwDataType (4 bytes)
	//payload len (4 bytes)

	//--paylaod
	//codec_type (4 bytes)
	//sample rate (4 bytes)

	unsigned int *values = (unsigned int*) conf_buf;
	values[0] = FOURCC_CONF;
	values[1] = 8;
	values[2] = FOURCC_GAMR;
	values[3] = handle->sample_rate_;

	return 0;
}

static int ATK_GAMREnc_Release(void** handle)
{
	Encoder_Interface_exit(*handle);
	*handle = NULL;

	return 0;
}
#endif


static int ATK_G711Enc_ALaw_Encode(ATK_AUDIOENC_HANDLE_T* handle, ATK_AUDIOENC_ONEFRAME_CONF_T* conf)
{
	G711_ALaw_Encode((short*)conf->pbyInBuf, conf->pbyOutBuf, handle->period_size_in_frames, handle->step_offset_);

	return handle->period_size_in_frames;
}

static int ATK_G711Enc_ULaw_Encode(ATK_AUDIOENC_HANDLE_T* handle, ATK_AUDIOENC_ONEFRAME_CONF_T* conf)
{
	G711_ULaw_Encode((short*)conf->pbyInBuf, conf->pbyOutBuf, handle->period_size_in_frames, handle->step_offset_);

	return handle->period_size_in_frames;
}

/**
 * @brief Function to get information about the encoder.
 *
 * @param[in] handle The handle of audio encoder (tk).
 * @param[out] conf_buf The buffer which to store the information.
 * @return negative: failed, otherwise: success.
 */
static int ATK_G711Enc_GetConf(ATK_AUDIOENC_HANDLE_T *handle, void *conf_buf)
{
	//---header
	//dwDataType (4 bytes)
	//payload len (4 bytes)

	//--paylaod
	//codec_type (4 bytes)
	//compression_format [ulaw or alaw] (4 bytes)

	unsigned int *values = (unsigned int*) conf_buf;
	values[0] = FOURCC_CONF;
	values[1] = 8;
	values[2] = FOURCC_G711;
	values[3] = (handle->g711_mode_ == 0) ? FOURCC_ULAW : FOURCC_ALAW;

	return 0;
}

// ==========================================================================
/**
 * @brief Function to initialize G726 audio encoder.
 *
 * @param[in] g726enc_handle The handle of G726 audio encoder.
 * @param[in] config Initial configuration for audio encoder.
 * @return negative: failed, otherwise: success.
 */
static int ATK_G726Enc_Init(ATK_AUDIOENC_HANDLE_T *g726enc_handle, const ATK_G726ENC_CONFIG_T *config)
{
	g726enc_handle->bit_rate_ = config->dwBitRate;
	g726enc_handle->handle_ = (void *)g726_enc_init(g726enc_handle->bit_rate_);

	return (g726enc_handle->handle_ != NULL)? 0:-1;
}

static int ATK_G726Enc_Release(void** handle)
{
	g726_enc_release((g726_enc_handle_t*)(*handle));
	*handle = NULL;

	return 0;
}

/**
 * @brief Function to encode one audio frame.
 *
 * @param[in] handle The handle of audio encoder (tk).
 * @param[in] in_buf The input data buffer,
 * @param[out] out_buf The output encoded data buffer.
 * @param[in] out_buf_size The size of output data buffer.
 * @return negative: failed, otherwise: The size of encoded data (bytes).
 */
static int ATK_G726Enc_EncodeOneFrame(ATK_AUDIOENC_HANDLE_T *handle, ATK_AUDIOENC_ONEFRAME_CONF_T* conf)
{
	return g726_encode_frame((g726_enc_handle_t*)handle->handle_, conf->pbyOutBuf, (const int16_t*) conf->pbyInBuf, handle->period_size_in_frames, handle->step_offset_);
}

/**
 * @brief Function to get information about the encoder.
 *
 * @param[in] handle The handle of audio encoder (tk).
 * @param[out] conf_buf The buffer which to store the information.
 * @return negative: failed, otherwise: success.
 */
static int ATK_G726Enc_GetConf(ATK_AUDIOENC_HANDLE_T *handle, void *conf_buf)
{
	//---header
	//dwDataType (4 bytes)
	//payload len (4 bytes)

	//--paylaod
	//codec_type (4 bytes)
	//dwCodewordBits (4 bytes)

	unsigned int *values = (unsigned int*) conf_buf;

	values[0] = FOURCC_CONF;
	values[1] = 8;
	values[2] = FOURCC_G726;
	values[3] = handle->bit_rate_ / 8000;

	return 0;
}

// ==========================================================================
ATK_AUDIOENC_HANDLE_T* ATK_AudioEnc_Init(const ATK_AUDIOENC_INITOPT_T *ptInitOpt, const void *pConfig)
{
	ATK_AUDIOENC_HANDLE_T *handle = NULL;
	if(pConfig == NULL || ptInitOpt == NULL)
	{
		return NULL;
	}

	handle = (ATK_AUDIOENC_HANDLE_T*) calloc(1, sizeof(ATK_AUDIOENC_HANDLE_T));
	if(handle == NULL)
	{
		return NULL;
	}

	handle->channels_ = ptInitOpt->dwChannels;
	handle->is_interleaved_ = (ptInitOpt->bIsInterleaved > 0)? 1:0;
	handle->period_size_in_frames = ptInitOpt->dwPeriodSizeInFrames;
	int ret = -1;
	switch(ptInitOpt->eType)
	{
		case kAAC4:
#ifdef HAS_HW_AAC_ENC
			handle->release_func = (Enc_Release) VMF_AAC4ENC_Release;
#else
			handle->release_func = (Enc_Release) ATK_AACEnc_Release;
#endif
			handle->enc_func = (Enc_EncodeOneFrame) ATK_AAC4Enc_EncodeOneFrame;
			handle->getconf_func = (Enc_GetConf) ATK_AAC4Enc_GetConf;
			ret = ATK_AAC4Enc_Init(handle, pConfig);
			break;
		case kG711:
			{
				ATK_G711ENC_CONFIG_T* g711_config = (ATK_G711ENC_CONFIG_T*) pConfig;
				if (g711_config->iCompressionMode == 0)
				{
					handle->enc_func = (Enc_EncodeOneFrame) ATK_G711Enc_ULaw_Encode;
					handle->g711_mode_ = 0;
				}
				else
				{
					handle->enc_func = (Enc_EncodeOneFrame) ATK_G711Enc_ALaw_Encode;
					handle->g711_mode_ = 1;
				}
				handle->getconf_func = (Enc_GetConf) ATK_G711Enc_GetConf;
				ret = 0;
			}
			break;
		case kG726:
			handle->release_func = (Enc_Release)ATK_G726Enc_Release;
			handle->enc_func = (Enc_EncodeOneFrame) ATK_G726Enc_EncodeOneFrame;
			handle->getconf_func = (Enc_GetConf) ATK_G726Enc_GetConf;
			ret = ATK_G726Enc_Init(handle, pConfig);
			break;
		case kGAMR:
#ifdef GAMR_SUPPORT
			handle->release_func = (Enc_Release) ATK_GAMREnc_Release;//GAMREnc_Release;
			handle->enc_func = (Enc_EncodeOneFrame) ATK_GAMREnc_EncodeOneFrame;
			handle->getconf_func = (Enc_GetConf) ATK_GAMREnc_GetConf;
			handle->pcmbuffer = malloc(handle->period_size_in_frames * sizeof(short));

			if(handle->pcmbuffer != NULL)
			{
				ret = ATK_GAMREnc_Init(handle, pConfig);
				if(ret < 0)
				{
					free(handle->pcmbuffer);
					handle->pcmbuffer = NULL;
				}
			}
			else
			{
				fprintf(stderr, "[%s, %s]: Unable to allocate buffer.\n", __FILE__, __func__);
			}
#endif
			break;
		default:
			break;
	}

	if(ret < 0)
	{
		free(handle);
		return NULL;
	}
	handle->step_offset_ = ((handle->is_interleaved_) && (handle->channels_ > 1)) ? (handle->channels_) : 1;

	return handle;
}

int ATK_AudioEnc_Release(ATK_AUDIOENC_HANDLE_T *ptHandle)
{
	if(!ptHandle)
		return -1;

	if(ptHandle->handle_)
	{
		void* phandle = (void *)ptHandle->handle_;
#ifdef HAS_HW_AAC_ENC
	if(ptHandle->release_func && (ptHandle->release_func(&(phandle)) != 0))
		{
#else
	if(ptHandle->release_func && (ptHandle->release_func(&(phandle)) != 0))
		{
#endif 		
		
			fprintf(stderr, "[%s, %s]: Release encoder failed.\n", __FILE__, __func__);
		}
		ptHandle->handle_ = NULL;
	}

	if(ptHandle->pcmbuffer)
	{
		free(ptHandle->pcmbuffer);
		ptHandle->pcmbuffer = NULL;
	}

	free(ptHandle);

	return 0;
}

int ATK_AudioEnc_EncodeOneFrame(ATK_AUDIOENC_HANDLE_T *ptHandle, ATK_AUDIOENC_ONEFRAME_CONF_T* ptConf)
{
	return ptHandle->enc_func(ptHandle, ptConf);
}

int ATK_AudioEnc_GetConf(ATK_AUDIOENC_HANDLE_T *ptHandle, void *pConfBuf)
{
	return ptHandle->getconf_func(ptHandle, pConfBuf);
}

