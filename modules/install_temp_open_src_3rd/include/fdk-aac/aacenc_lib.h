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
/* -----------------------------------------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

?Copyright  1995 - 2015 Fraunhofer-Gesellschaft zur Förderung der angewandten Forschung e.V.
  All rights reserved.

 1.    INTRODUCTION
The Fraunhofer FDK AAC Codec Library for Android ("FDK AAC Codec") is software that implements
the MPEG Advanced Audio Coding ("AAC") encoding and decoding scheme for digital audio.
This FDK AAC Codec software is intended to be used on a wide variety of Android devices.

AAC's HE-AAC and HE-AAC v2 versions are regarded as today's most efficient general perceptual
audio codecs. AAC-ELD is considered the best-performing full-bandwidth communications codec by
independent studies and is widely deployed. AAC has been standardized by ISO and IEC as part
of the MPEG specifications.

Patent licenses for necessary patent claims for the FDK AAC Codec (including those of Fraunhofer)
may be obtained through Via Licensing (www.vialicensing.com) or through the respective patent owners
individually for the purpose of encoding or decoding bit streams in products that are compliant with
the ISO/IEC MPEG audio standards. Please note that most manufacturers of Android devices already license
these patent claims through Via Licensing or directly from the patent owners, and therefore FDK AAC Codec
software may already be covered under those patent licenses when it is used for those licensed purposes only.

Commercially-licensed AAC software libraries, including floating-point versions with enhanced sound quality,
are also available from Fraunhofer. Users are encouraged to check the Fraunhofer website for additional
applications information and documentation.

2.    COPYRIGHT LICENSE

Redistribution and use in source and binary forms, with or without modification, are permitted without
payment of copyright license fees provided that you satisfy the following conditions:

You must retain the complete text of this software license in redistributions of the FDK AAC Codec or
your modifications thereto in source code form.

You must retain the complete text of this software license in the documentation and/or other materials
provided with redistributions of the FDK AAC Codec or your modifications thereto in binary form.
You must make available free of charge copies of the complete source code of the FDK AAC Codec and your
modifications thereto to recipients of copies in binary form.

The name of Fraunhofer may not be used to endorse or promote products derived from this library without
prior written permission.

You may not charge copyright license fees for anyone to use, copy or distribute the FDK AAC Codec
software or your modifications thereto.

Your modified versions of the FDK AAC Codec must carry prominent notices stating that you changed the software
and the date of any change. For modified versions of the FDK AAC Codec, the term
"Fraunhofer FDK AAC Codec Library for Android" must be replaced by the term
"Third-Party Modified Version of the Fraunhofer FDK AAC Codec Library for Android."

3.    NO PATENT LICENSE

NO EXPRESS OR IMPLIED LICENSES TO ANY PATENT CLAIMS, including without limitation the patents of Fraunhofer,
ARE GRANTED BY THIS SOFTWARE LICENSE. Fraunhofer provides no warranty of patent non-infringement with
respect to this software.

You may use this FDK AAC Codec software or modifications thereto only for purposes that are authorized
by appropriate patent licenses.

4.    DISCLAIMER

This FDK AAC Codec software is provided by Fraunhofer on behalf of the copyright holders and contributors
"AS IS" and WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, including but not limited to the implied warranties
of merchantability and fitness for a particular purpose. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE for any direct, indirect, incidental, special, exemplary, or consequential damages,
including but not limited to procurement of substitute goods or services; loss of use, data, or profits,
or business interruption, however caused and on any theory of liability, whether in contract, strict
liability, or tort (including negligence), arising in any way out of the use of this software, even if
advised of the possibility of such damage.

5.    CONTACT INFORMATION

Fraunhofer Institute for Integrated Circuits IIS
Attention: Audio and Multimedia Departments - FDK AAC LL
Am Wolfsmantel 33
91058 Erlangen, Germany

www.iis.fraunhofer.de/amm
amm-info@iis.fraunhofer.de
----------------------------------------------------------------------------------------------------------- */

#ifndef _AAC_ENC_LIB_H_
#define _AAC_ENC_LIB_H_

#include "machine_type.h"
#include "FDK_audio.h"

#ifndef MAKETHREECC
    #define MAKETHREECC(ch0, ch1, ch2)  ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) | ((DWORD)(BYTE)(ch2) << 16) )
#endif //defined(MAKETHREECC)

#define AACENC_VERSION MAKETHREECC(0, 1, 5)

#define AACENCODER_LIB_VL0 3
#define AACENCODER_LIB_VL1 4
#define AACENCODER_LIB_VL2 22

/**
 *  AAC encoder error codes.
 */
typedef enum {
    AACENC_OK                     = 0x0000,  /*!< No error happened. All fine. */

    AACENC_INVALID_HANDLE         = 0x0020,  /*!< Handle passed to function call was invalid. */
    AACENC_MEMORY_ERROR           = 0x0021,  /*!< Memory allocation failed. */
    AACENC_UNSUPPORTED_PARAMETER  = 0x0022,  /*!< Parameter not available. */
    AACENC_INVALID_CONFIG         = 0x0023,  /*!< Configuration not provided. */

    AACENC_INIT_ERROR             = 0x0040,  /*!< General initialization error. */
    AACENC_INIT_AAC_ERROR         = 0x0041,  /*!< AAC library initialization error. */
    AACENC_INIT_SBR_ERROR         = 0x0042,  /*!< SBR library initialization error. */
    AACENC_INIT_TP_ERROR          = 0x0043,  /*!< Transport library initialization error. */
    AACENC_INIT_META_ERROR        = 0x0044,  /*!< Meta data library initialization error. */

    AACENC_ENCODE_ERROR           = 0x0060,  /*!< The encoding process was interrupted by an unexpected error. */

    AACENC_ENCODE_EOF             = 0x0080   /*!< End of file reached. */

} AACENC_ERROR;


/**
 *  AAC encoder buffer descriptors identifier.
 *  This identifier are used within buffer descriptors AACENC_BufDesc::bufferIdentifiers.
 */
typedef enum {
    /* Input buffer identifier. */
    IN_AUDIO_DATA      = 0,                  /*!< Audio input buffer, interleaved INT_PCM samples. */
    IN_ANCILLRY_DATA   = 1,                  /*!< Ancillary data to be embedded into bitstream. */
    IN_METADATA_SETUP  = 2,                  /*!< Setup structure for embedding meta data. */

    /* Output buffer identifier. */
    OUT_BITSTREAM_DATA = 3,                  /*!< Buffer holds bitstream output data. */
    OUT_AU_SIZES       = 4                   /*!< Buffer contains sizes of each access unit. This information
                                                  is necessary for superframing. */

} AACENC_BufferIdentifier;


/**
 *  AAC encoder handle.
 */
typedef struct AACENCODER *HANDLE_AACENCODER;


/**
 *  Provides some info about the encoder configuration.
 */
typedef struct {

    UINT                maxOutBufBytes;      /*!< Maximum number of encoder bitstream bytes within one frame.
                                                  Size depends on maximum number of supported channels in encoder instance.
                                                  For superframing (as used for example in DAB+), size has to be a multiple accordingly. */

    UINT                maxAncBytes;         /*!< Maximum number of ancillary data bytes which can be inserted into
                                                  bitstream within one frame. */

    UINT                inBufFillLevel;      /*!< Internal input buffer fill level in samples per channel. This parameter
                                                  will automatically be cleared if samplingrate or channel(Mode/Order) changes. */

    UINT                inputChannels;       /*!< Number of input channels expected in encoding process. */

    UINT                frameLength;         /*!< Amount of input audio samples consumed each frame per channel, depending
                                                  on audio object type configuration. */

    UINT                encoderDelay;        /*!< Codec delay in PCM samples/channel. Depends on framelength and AOT. Does not
                                                  include framing delay for filling up encoder PCM input buffer. */

    UCHAR               confBuf[64];         /*!< Configuration buffer in binary format as an AudioSpecificConfig
                                                  or StreamMuxConfig according to the selected transport type. */

    UINT                confSize;            /*!< Number of valid bytes in confBuf. */

} AACENC_InfoStruct;


/**
 *  Describes the input and output buffers for an aacEncEncode() call.
 */
typedef struct {
    INT                 numBufs;             /*!< Number of buffers. */
    void              **bufs;                /*!< Pointer to vector containing buffer addresses. */
    INT                *bufferIdentifiers;   /*!< Identifier of each buffer element. See ::AACENC_BufferIdentifier. */
    INT                *bufSizes;            /*!< Size of each buffer in 8-bit bytes. */
    INT                *bufElSizes;          /*!< Size of each buffer element in bytes. */

} AACENC_BufDesc;


/**
 *  Defines the input arguments for an aacEncEncode() call.
 */
typedef struct {
    INT                 numInSamples;        /*!< Number of valid input audio samples (multiple of input channels). */
    INT                 numAncBytes;         /*!< Number of ancillary data bytes to be encoded. */

} AACENC_InArgs;


/**
 *  Defines the output arguments for an aacEncEncode() call.
 */
typedef struct {
    INT                 numOutBytes;         /*!< Number of valid bitstream bytes generated during aacEncEncode(). */
    INT                 numInSamples;        /*!< Number of input audio samples consumed by the encoder. */
    INT                 numAncBytes;         /*!< Number of ancillary data bytes consumed by the encoder. */

} AACENC_OutArgs;


/**
 *  Meta Data Compression Profiles.
 */
typedef enum {
    AACENC_METADATA_DRC_NONE          = 0,   /*!< None. */
    AACENC_METADATA_DRC_FILMSTANDARD  = 1,   /*!< Film standard. */
    AACENC_METADATA_DRC_FILMLIGHT     = 2,   /*!< Film light. */
    AACENC_METADATA_DRC_MUSICSTANDARD = 3,   /*!< Music standard. */
    AACENC_METADATA_DRC_MUSICLIGHT    = 4,   /*!< Music light. */
    AACENC_METADATA_DRC_SPEECH        = 5    /*!< Speech. */

} AACENC_METADATA_DRC_PROFILE;


/**
 *  Meta Data setup structure.
 */
typedef struct {

  AACENC_METADATA_DRC_PROFILE drc_profile;             /*!< MPEG DRC compression profile. See ::AACENC_METADATA_DRC_PROFILE. */
  AACENC_METADATA_DRC_PROFILE comp_profile;            /*!< ETSI heavy compression profile. See ::AACENC_METADATA_DRC_PROFILE. */

  INT                         drc_TargetRefLevel;      /*!< Used to define expected level to:
                                                            Scaled with 16 bit. x*2^16. */
  INT                         comp_TargetRefLevel;     /*!< Adjust limiter to avoid overload.
                                                            Scaled with 16 bit. x*2^16. */

  INT                         prog_ref_level_present;  /*!< Flag, if prog_ref_level is present */
  INT                         prog_ref_level;          /*!< Programme Reference Level = Dialogue Level:
                                                            -31.75dB .. 0 dB ; stepsize: 0.25dB
                                                            Scaled with 16 bit. x*2^16.*/

  UCHAR                       PCE_mixdown_idx_present; /*!< Flag, if dmx-idx should be written in programme config element */
  UCHAR                       ETSI_DmxLvl_present;     /*!< Flag, if dmx-lvl should be written in ETSI-ancData */

  SCHAR                       centerMixLevel;          /*!< Center downmix level (0...7, according to table) */
  SCHAR                       surroundMixLevel;        /*!< Surround downmix level (0...7, according to table) */

  UCHAR                       dolbySurroundMode;       /*!< Indication for Dolby Surround Encoding Mode.
                                                            - 0: Dolby Surround mode not indicated
                                                            - 1: 2-ch audio part is not Dolby surround encoded
                                                            - 2: 2-ch audio part is Dolby surround encoded */
} AACENC_MetaData;


/**
 * AAC encoder control flags.
 *
 * In interaction with the ::AACENC_CONTROL_STATE parameter it is possible to get information about the internal
 * initialization process. It is also possible to overwrite the internal state from extern when necessary.
 */
typedef enum
{
    AACENC_INIT_NONE              = 0x0000,  /*!< Do not trigger initialization. */
    AACENC_INIT_CONFIG            = 0x0001,  /*!< Initialize all encoder modules configuration. */
    AACENC_INIT_STATES            = 0x0002,  /*!< Reset all encoder modules history buffer. */
    AACENC_INIT_TRANSPORT         = 0x1000,  /*!< Initialize transport lib with new parameters. */
    AACENC_RESET_INBUFFER         = 0x2000,  /*!< Reset fill level of internal input buffer. */
    AACENC_INIT_ALL               = 0xFFFF   /*!< Initialize all. */
}
AACENC_CTRLFLAGS;


/**
 * \brief  AAC encoder setting parameters.
 *
 * Use aacEncoder_SetParam() function to configure, or use aacEncoder_GetParam() function to read
 * the internal status of the following parameters.
 */
typedef enum
{
  AACENC_BITRATE                  = 0x0101,  /*!< Total encoder bitrate. This parameter is mandatory and interacts with ::AACENC_BITRATEMODE.
                                                  - CBR: Bitrate in bits/second.
                                                    See \ref suppBitrates for details. */

  AACENC_SAMPLERATE               = 0x0103,  /*!< Audio input data sampling rate. Encoder supports following sampling rates:
                                                  8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000, 64000, 88200, 96000 */

  AACENC_CHANNELMODE              = 0x0106,  /*!< Set explicit channel mode. Channel mode must match with number of input channels.
                                                  - 1-7 and 33,34: MPEG channel modes supported, see ::CHANNEL_MODE in FDK_audio.h. */

  AACENC_CHANNELORDER             = 0x0107,  /*!< Input audio data channel ordering scheme:
                                                  - 0: MPEG channel ordering (e. g. 5.1: C, L, R, SL, SR, LFE). (default)
                                                  - 1: WAVE file format channel ordering (e. g. 5.1: L, R, C, LFE, SL, SR). */

  AACENC_TRANSMUX                 = 0x0300,  /*!< Transport type to be used. See ::TRANSPORT_TYPE in FDK_audio.h. Following
                                                  types can be configured in encoder library:
                                                  - 0: raw access units
                                                  - 1: ADIF bitstream format
                                                  - 2: ADTS bitstream format
                                                  - 6: Audio Mux Elements (LATM) with muxConfigPresent = 1
                                                  - 7: Audio Mux Elements (LATM) with muxConfigPresent = 0, out of band StreamMuxConfig
                                                  - 10: Audio Sync Stream (LOAS) */

  AACENC_NONE                     = 0xFFFF   /*!< ------ */

} AACENC_PARAM;


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief  Open an instance of the encoder.
 *
 * Allocate memory for an encoder instance with a functional range denoted by the function parameters.
 * Preinitialize encoder instance with default configuration.
 *
 * \param phAacEncoder  A pointer to an encoder handle. Initialized on return.
 * \param encModules    Specify encoder modules to be supported in this encoder instance:
 *                      - 0x0: Allocate memory for all available encoder modules.
 *                      - else: Select memory allocation regarding encoder modules. Following flags are possible and can be combined.
 *                              - 0x01: AAC module.
 *                              - 0x02: SBR module.
 *                              - 0x04: PS module.
 *                              - 0x10: Metadata module.
 *                              - example: (0x01|0x02|0x04|0x10) allocates all modules and is equivalent to default configuration denotet by 0x0.
 * \param maxChannels   Number of channels to be allocated. This parameter can be used in different ways:
 *                      - 0: Allocate maximum number of AAC and SBR channels as supported by the library.
 *                      - nChannels: Use same maximum number of channels for allocating memory in AAC and SBR module.
 *                      - nChannels | (nSbrCh<<8): Number of SBR channels can be different to AAC channels to save data memory.
 *
 * \return
 *          - AACENC_OK, on succes.
 *          - AACENC_INVALID_HANDLE, AACENC_MEMORY_ERROR, AACENC_INVALID_CONFIG, on failure.
 */
AACENC_ERROR aacEncOpen(
        HANDLE_AACENCODER        *phAacEncoder,
        const UINT                encModules,
        const UINT                maxChannels
        );


/**
 * \brief  Close the encoder instance.
 *
 * Deallocate encoder instance and free whole memory.
 *
 * \param phAacEncoder  Pointer to the encoder handle to be deallocated.
 *
 * \return
 *          - AACENC_OK, on success.
 *          - AACENC_INVALID_HANDLE, on failure.
 */
AACENC_ERROR aacEncClose(
        HANDLE_AACENCODER        *phAacEncoder
        );


/**
 * \brief Encode audio data.
 *
 * This function is mainly for encoding audio data. In addition the function can be used for an encoder (re)configuration
 * process.
 * - PCM input data will be retrieved from external input buffer until the fill level allows encoding a single frame.
 *   This functionality allows an external buffer with reduced size in comparison to the AAC or HE-AAC audio frame length.
 * - If the value of the input samples argument is zero, just internal reinitialization will be applied if it is
 *   requested.
 * - At the end of a file the flushing process can be triggerd via setting the value of the input samples argument to -1.
 *   The encoder delay lines are fully flushed when the encoder returns no valid bitstream data AACENC_OutArgs::numOutBytes.
 *   Furthermore the end of file is signaled by the return value AACENC_ENCODE_EOF.
 * - If an error occured in the previous frame or any of the encoder parameters changed, an internal reinitialization
 *   process will be applied before encoding the incoming audio samples.
 * - The function can also be used for an independent reconfiguration process without encoding. The first parameter has to be a
 *   valid encoder handle and all other parameters can be set to NULL.
 * - If the size of the external bitbuffer in outBufDesc is not sufficient for writing the whole bitstream, an internal
 *   error will be the return value and a reconfiguration will be triggered.
 *
 * \param hAacEncoder           A valid AAC encoder handle.
 * \param inBufDesc             Input buffer descriptor, see AACENC_BufDesc:
 *                              - At least one input buffer with audio data is expected.
 *                              - Optionally a second input buffer with ancillary data can be fed.
 * \param outBufDesc            Output buffer descriptor, see AACENC_BufDesc:
 *                              - Provide one output buffer for the encoded bitstream.
 * \param inargs                Input arguments, see AACENC_InArgs.
 * \param outargs               Output arguments, AACENC_OutArgs.
 *
 * \return
 *          - AACENC_OK, on success.
 *          - AACENC_INVALID_HANDLE, AACENC_ENCODE_ERROR, on failure in encoding process.
 *          - AACENC_INVALID_CONFIG, AACENC_INIT_ERROR, AACENC_INIT_AAC_ERROR, AACENC_INIT_SBR_ERROR, AACENC_INIT_TP_ERROR,
 *            AACENC_INIT_META_ERROR, on failure in encoder initialization.
 *          - AACENC_ENCODE_EOF, when flushing fully concluded.
 */
AACENC_ERROR aacEncEncode(
        const HANDLE_AACENCODER   hAacEncoder,
        const AACENC_BufDesc     *inBufDesc,
        const AACENC_BufDesc     *outBufDesc,
        const AACENC_InArgs      *inargs,
        AACENC_OutArgs           *outargs
        );


/**
 * \brief  Acquire info about present encoder instance.
 *
 * This function retrieves information of the encoder configuration. In addition to informative internal states,
 * a configuration data block of the current encoder settings will be returned. The format is either Audio Specific Config
 * in case of Raw Packets transport format or StreamMuxConfig in case of LOAS/LATM transport format. The configuration
 * data block is binary coded as specified in ISO/IEC 14496-3 (MPEG-4 audio), to be used directly for MPEG-4 File Format
 * or RFC3016 or RFC3640 applications.
 *
 * \param hAacEncoder           A valid AAC encoder handle.
 * \param pInfo                 Pointer to AACENC_InfoStruct. Filled on return.
 *
 * \return
 *          - AACENC_OK, on succes.
 *          - AACENC_INIT_ERROR, on failure.
 */
AACENC_ERROR aacEncInfo(
        const HANDLE_AACENCODER   hAacEncoder,
        AACENC_InfoStruct        *pInfo
        );


/**
 * \brief  Set one single AAC encoder parameter.
 *
 * This function allows configuration of all encoder parameters specified in ::AACENC_PARAM. Each parameter must be
 * set with a separate function call. An internal validation of the configuration value range will be done and an
 * internal reconfiguration will be signaled. The actual configuration adoption is part of the subsequent aacEncEncode() call.
 *
 * \param hAacEncoder           A valid AAC encoder handle.
 * \param param                 Parameter to be set. See ::AACENC_PARAM.
 * \param value                 Parameter value. See parameter description in ::AACENC_PARAM.
 *
 * \return
 *          - AACENC_OK, on success.
 *          - AACENC_INVALID_HANDLE, AACENC_UNSUPPORTED_PARAMETER, AACENC_INVALID_CONFIG, on failure.
 */
AACENC_ERROR aacEncoder_SetParam(
        const HANDLE_AACENCODER   hAacEncoder,
        const AACENC_PARAM        param,
        const UINT                value
        );


/**
 * \brief  Get one single AAC encoder parameter.
 *
 * This function is the complement to aacEncoder_SetParam(). After encoder reinitialization with user defined settings,
 * the internal status can be obtained of each parameter, specified with ::AACENC_PARAM.
 *
 * \param hAacEncoder           A valid AAC encoder handle.
 * \param param                 Parameter to be returned. See ::AACENC_PARAM.
 *
 * \return  Internal configuration value of specifed parameter ::AACENC_PARAM.
 */
UINT aacEncoder_GetParam(
        const HANDLE_AACENCODER   hAacEncoder,
        const AACENC_PARAM        param
        );


/**
 * \brief  Get information about encoder library build.
 *
 * Fill a given LIB_INFO structure with library version information.
 *
 * \param info  Pointer to an allocated LIB_INFO struct.
 *
 * \return
 *          - AACENC_OK, on success.
 *          - AACENC_INVALID_HANDLE, AACENC_INIT_ERROR, on failure.
 */
AACENC_ERROR aacEncGetLibInfo(
        LIB_INFO                 *info
        );


#ifdef __cplusplus
}
#endif

#endif   /* _AAC_ENC_LIB_H_ */
