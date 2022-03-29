#ifndef FDK_AAC_PROCESSOR_H
#define FDK_AAC_PROCESSOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fdk-aac/aacdecoder_lib.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef struct AAC4_DECODER_INITOPT_T
{
	unsigned int dwChannels;
	unsigned int dwSampleRate;
	unsigned int dwAdts;
	size_t dwSpecConfSize;
	unsigned char *pbySpecConf;

} AAC4_DECODER_INITOPT_T;

typedef struct AAC4_DECODER_DECODED_STREAM_INFO_T
{
	unsigned int dwSampleRate;
	unsigned int dwFrameSize;
	unsigned int dwChannels;
} AAC4_DECODER_DECODED_STREAM_INFO_T;


typedef struct AAC4_DECODER_HANDLE_T
{
	HANDLE_AACDECODER ptAacDecHandle;

	unsigned int dwChannels;
	unsigned int dwSampleRate;
	unsigned int dwAdts;
	size_t dwSpecConfSize;
	unsigned char *pbySpecConf;

} AAC4_DECODER_HANDLE_T;

typedef enum AAC4_DECODER_CONCEAL_METHOD {
    CONCEAL_METHOD_SPECTRAL_MUTING      =  0,
    CONCEAL_METHOD_NOISE_SUBSTITUTION   =  1,
    CONCEAL_METHOD_ENERGY_INTERPOLATION =  2,
    CONCEAL_METHOD_NB,
} AAC4_DECODER_CONCEAL_METHOD;


/**
 * @brief Initializes handle for AAC4 decoder
 *
 * @param[in] channels number of output channels.
 */
static inline AAC4_DECODER_HANDLE_T *Fdk_AAC4_Decoder_Init(AAC4_DECODER_INITOPT_T *init_opt)
{
	AAC4_DECODER_HANDLE_T *handle = NULL;
	AAC_DECODER_ERROR err;
	TRANSPORT_TYPE type = TT_MP4_RAW;

	if (NULL == init_opt)
	{
		printf("(%s, %d) invalid param: init_opt is NULL!\n", __func__, __LINE__);
	}
	handle = (AAC4_DECODER_HANDLE_T *) calloc(1, sizeof(AAC4_DECODER_HANDLE_T));
	if (!handle)
	{
		goto INIT_FAILED;
	}
	switch(init_opt->dwAdts) {
		case 0:
			type = TT_MP4_RAW;
			break;
		case 1:
			type = TT_MP4_ADTS;
			break;
		case 2:
			type = TT_MP4_ADIF;
			break;
	}
	handle->ptAacDecHandle = aacDecoder_Open(type, 1);
	if (!handle->ptAacDecHandle)
	{
		goto INIT_FAILED;
	}
	handle->dwChannels = init_opt->dwChannels;
	handle->dwSampleRate = init_opt->dwSampleRate;
	handle->dwAdts = init_opt->dwAdts;
	err = aacDecoder_SetParam(handle->ptAacDecHandle, AAC_PCM_MAX_OUTPUT_CHANNELS, init_opt->dwChannels);
	if (err != AAC_DEC_OK)
	{
		printf("(%s, %d) aacDecoder_SetParam(AAC_PCM_MAX_OUTPUT_CHANNELS(%u)) failed with err(%x)\n", __func__, __LINE__, init_opt->dwChannels, err);
		goto INIT_FAILED;
	}
	err = aacDecoder_SetParam(handle->ptAacDecHandle, AAC_CONCEAL_METHOD, CONCEAL_METHOD_NOISE_SUBSTITUTION);
	if (err != AAC_DEC_OK)
	{
		printf("(%s, %d) aacDecoder_SetParam(AAC_CONCEAL_METHOD(CONCEAL_METHOD_NOISE_SUBSTITUTION)) failed with err(%x)\n", __func__, __LINE__, err);
		goto INIT_FAILED;
	}
	if (!init_opt->dwAdts)
	{
		handle->pbySpecConf = (uint8_t *) malloc(init_opt->dwSpecConfSize * sizeof(uint8_t));
		if(handle->pbySpecConf == NULL)
		{
			goto INIT_FAILED;
		}
		handle->dwSpecConfSize = init_opt->dwSpecConfSize;
		memcpy(handle->pbySpecConf, init_opt->pbySpecConf, handle->dwSpecConfSize);

		err = aacDecoder_ConfigRaw(handle->ptAacDecHandle, &init_opt->pbySpecConf, (const UINT *)&init_opt->dwSpecConfSize);
		if (err != AAC_DEC_OK)
		{
			printf("(%s, %d) aacDecoder_ConfigRaw(handle->dwSpecConfSize(%d)) failed with err(%x)\n", __func__, __LINE__, handle->dwSpecConfSize, err);
			goto INIT_FAILED;
		}
	}

	return handle;

INIT_FAILED:
	if (handle)
	{
		if (handle->pbySpecConf)
		{
			free(handle->pbySpecConf);
			handle->pbySpecConf = NULL;
		}
		if (handle->ptAacDecHandle)
		{
			aacDecoder_Close(handle->ptAacDecHandle);
			handle->ptAacDecHandle = NULL;
		}
		free(handle);
		handle = NULL;
	}

	return NULL;
}



/**
 * @brief Decodes AAC4-encoded bit streams to 16-bit linear PCM
 *
 * @param[in] handle AAC4 decoder handle.
 * @param[in] in_buf The input buffer of the AAC4 audio data.
 * @param[in] inbuf_size The input buffer size of the AAC4 audio data.
 * @param[in, out] out_buf The output buffer for the PCM data.
 * @param[in] outbuf_size The total bytes of the output buffer.
 * @return[int] decoded PCM data size
 */
static inline int Fdk_AAC4_Decoder_OneFrame(
	AAC4_DECODER_HANDLE_T *handle,
	uint8_t* in_buf, int inbuf_size,
	uint8_t* out_buf, int outbuf_size)
{
	AAC_DECODER_ERROR err;
	//int ret = 0;
	int size = inbuf_size;
	UINT valid = inbuf_size;

	if (NULL == handle || NULL == handle->ptAacDecHandle)
	{
		printf("(%s, %d) handle(%p)\n", __func__, __LINE__, (void*)handle);
		if (handle)
		{
			printf("(%s, %d) handle->ptAacDecHandle(%p)\n", __func__, __LINE__, (void*)handle->ptAacDecHandle);
		}
		return -1;
	}
	if (NULL == in_buf)
	{
		printf("(%s, %d) in_buf(%p)\n", __func__, __LINE__, (void*)in_buf);
		return -1;
	}
	if (inbuf_size <= 0)
	{
		printf("(%s, %d) invalid inbuf_size(%d)\n", __func__, __LINE__, inbuf_size);
		return -1;
	}
	if (NULL == out_buf)
	{
		printf("(%s, %d) out_buf(%p)\n", __func__, __LINE__, (void*)out_buf);
		return -1;
	}
	if (outbuf_size <= 0)
	{
		printf("(%s, %d) invalid outbuf_size(%d)\n", __func__, __LINE__, outbuf_size);
		return -1;
	}

	err = aacDecoder_Fill(handle->ptAacDecHandle, &in_buf, (const UINT *)&size, &valid);
	if (err != AAC_DEC_OK)
	{
		printf("(%s, %d) aacDecoder_Fill() failed with err(%x)\n", __func__, __LINE__, err);
		return -1;
	}
	err = aacDecoder_DecodeFrame(handle->ptAacDecHandle, (INT_PCM *) out_buf, outbuf_size, 0);
	if (err == AAC_DEC_NOT_ENOUGH_BITS)
	{
		return 0;
	}
	if (err != AAC_DEC_OK)
	{
		printf("(%s, %d) aacDecoder_DecodeFrame() failed with err(%x)\n", __func__, __LINE__, err);
		return -1;
	}

	return 0;
}

/**
 * @brief Releases handle for AAC4 decoder
 *
 * @param[in] handle AAC4 decoder handle.
 * @param[in, out] stream_info decoded PCM stream information.
 */
static inline int Fdk_AAC4_Decoder_Get_Decoded_Stream_Info(
	AAC4_DECODER_HANDLE_T *handle,
	AAC4_DECODER_DECODED_STREAM_INFO_T *stream_info)
{
	if (NULL == handle || NULL == handle->ptAacDecHandle)
	{
		printf("(%s, %d) invalid handle!\n",  __func__, __LINE__);
		return -1;
	}
	if (NULL == stream_info)
	{
		printf("(%s, %d) invalid param: stream_info is NULL!\n",  __func__, __LINE__);
		return -1;
	}

	CStreamInfo *info = aacDecoder_GetStreamInfo(handle->ptAacDecHandle);
	if (NULL == info)
	{
		printf("(%s, %d) aacDecoder_GetStreamInfo() failed\n",  __func__, __LINE__);
		return -1;
	}
	stream_info->dwChannels = info->numChannels;
	stream_info->dwSampleRate = info->sampleRate;
	stream_info->dwFrameSize = info->frameSize;

	return 0;
}


/**
 * @brief Releases handle for AAC4 decoder
 *
 * @param[in] handle AAC4 decoder handle.
 */
static inline void Fdk_AAC4_Decoder_Release(AAC4_DECODER_HANDLE_T *handle)
{
	if (NULL == handle) return;

	if (handle->pbySpecConf)
	{
		free(handle->pbySpecConf);
		handle->pbySpecConf = NULL;
	}
	if (handle->ptAacDecHandle)
	{
		aacDecoder_Close(handle->ptAacDecHandle);
		handle->ptAacDecHandle = NULL;
	}
	free(handle);
}


/**
 * @brief Releases handle for AAC4 decoder
 *
 * @param[in] handle AAC4 decoder handle.
 */
static inline int Fdk_AAC4_Decoder_Flush(AAC4_DECODER_HANDLE_T *handle)
{
	AAC_DECODER_ERROR err;
	if (NULL == handle || NULL == handle->ptAacDecHandle) return -1;

	err = aacDecoder_SetParam(handle->ptAacDecHandle, AAC_TPDEC_CLEAR_BUFFER, 1);
	if (err != AAC_DEC_OK)
	{
		printf("(%s, %d) aacDecoder_SetParam(AAC_TPDEC_CLEAR_BUFFER) failed with err(%x)\n", __func__, __LINE__, err);
		return -1;
	}

	return 0;
}



#ifdef __cplusplus
}
#endif

#endif
