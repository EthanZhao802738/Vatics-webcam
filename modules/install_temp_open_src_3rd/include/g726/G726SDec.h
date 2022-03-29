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

/* ============================================================================================== */
#ifndef __G726SENC_H__
#define __G726SENC_H__

/* ============================================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* ============================================================================================== */
#ifndef MAKETHREECC
    #define MAKETHREECC(ch0, ch1, ch2)  ((unsigned int)(unsigned char)(ch0) | ((unsigned int)(unsigned char)(ch1) << 8) | ((unsigned int)(unsigned char)(ch2) << 16) )
#endif //defined(MAKETHREECC)

#define G726SDEC_VERSION MAKETHREECC(1, 0, 0)


#ifdef __cplusplus
extern "C" {
#endif

typedef struct G726DecContext g726_dec_handle_t;

/**
 * @brief Initialise a G.726 decode context.
 *
 * @param[in] bitrate The required bit rate for the ADPCM data. The valid rates are 16000, 24000, 32000 and 40000.
 * @return A pointer to the G.726 context, or NULL for error.
 */
g726_dec_handle_t* g726_dec_init(int bitrate);


/**
 * @brief Free a G.726 decode context.
 *
 * @param[in] handle The G.726 context.
 */
void g726_dec_release(g726_dec_handle_t *handle);

/**
 * @brief Decode a buffer of G.726 ADPCM data to linear PCM.
 *
 * @param[in] handle The G.726 context.
 * @param[in, out] samples The audio sample buffer.
 * @param[in] g726_data The input G.726 data.
 * @param[in] g726_bytes The total bytes of the input G.726 data.
 * @param[in] step The offset of frames in the output buffer because the input G.726 audio data is one channel so we should use it to output data to "multiple channels" buffers.
 * @return The number of samples returned.
 */
int g726_decode_frame(g726_dec_handle_t *handle, int16_t samples[], const uint8_t g726_data[], int g726_bytes, unsigned int step);

/**
 * @brief Reset some parameters for G.726.
 *
 * @param[in] handle The G.726 context.
 */
void g726_decode_flush(g726_dec_handle_t *handle);

int G726SDec_GetVersionInfo(unsigned char* pbyMajor, unsigned char* pbyMinor, unsigned char* pbyRevision);

#ifdef __cplusplus
}
#endif

/* ===============================================================================================*/
#endif //__G711ENC_H__
