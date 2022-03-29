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

#define G726SENC_VERSION MAKETHREECC(1, 0, 0)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct G726EncContext g726_enc_handle_t;

/**
 * @brief Initialise a G.726 encode context.
 *
 * @param[in] bitrate The required bit rate for the ADPCM data. The valid rates are 16000, 24000, 32000 and 40000.
 * @return A pointer to the G.726 context, or NULL for error.
 */
g726_enc_handle_t* g726_enc_init(int bitrate);


/**
 * @brief Free a G.726 encode context.
 *
 * @param[in] handle The G.726 context.
 */
void g726_enc_release(g726_enc_handle_t *handle);

/**
 * @brief Encode a buffer of linear PCM data to G.726 ADPCM.
 *
 * @param[in] handle The G.726 context.
 * @param[in, out] g726_data The G.726 data produced.
 * @param[in] samples The audio sample buffer.
 * @param[in] nb_samples The number of samples in the buffer.
 * @param[in] step The offset of frames in the input buffer because the output G.726 audio data is one channel so we should use it to get the input data from "multiple channels" buffers.
 * @return The number of bytes of G.726 data produced.
*/
int g726_encode_frame(g726_enc_handle_t *handle, uint8_t g726_data[], const int16_t samples[], int nb_samples, unsigned int step);

int G726SEnc_GetVersionInfo(unsigned char* pbyMajor, unsigned char* pbyMinor, unsigned char* pbyRevision);

#ifdef __cplusplus
}
#endif

/* ===============================================================================================*/
#endif //__G711ENC_H__
