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
 *
 * This source code is a product of Sun Microsystems, Inc. and is provided
 * for unrestricted use.  Users may copy or modify this source code without
 * charge.
 *
 * SUN SOURCE CODE IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING
 * THE WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 *
 * Sun source code is provided with no support and without any obligation on
 * the part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY THIS SOFTWARE
 * OR ANY PART THEREOF.
 *
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

/* ============================================================================================== */
#ifndef __G711SENC_H__
#define __G711SENC_H__

/*
 * Functions have been updated to correctly
 * convert unquantized 16 bit values.
 * Tables for direct u- to A-law and A- to u-law conversions have been
 * corrected.
 * Borge Lindberg, Center for PersonKommunikation, Aalborg University.
 * bli@cpk.auc.dk
 */

/* ============================================================================================== */
#include <stdio.h>
#include <stdlib.h>

/* ============================================================================================== */
#ifndef MAKETHREECC
    #define MAKETHREECC(ch0, ch1, ch2)  ((unsigned int)(unsigned char)(ch0) | ((unsigned int)(unsigned char)(ch1) << 8) | ((unsigned int)(unsigned char)(ch2) << 16) )
#endif //defined(MAKETHREECC)

#define G711SENC_VERSION MAKETHREECC(1, 0, 0)

#ifdef __cplusplus
extern "C" {
#endif

/* ===============================================================================================*/
void G711_ALaw_Encode(short* in_buf, unsigned char* out_buf, const unsigned int frame_size, const unsigned int step);
void G711_ULaw_Encode(short* in_buf, unsigned char* out_buf, const unsigned int frame_size, const unsigned int step);

int G711SEnc_GetVersionInfo(unsigned char* pbyMajor, unsigned char* pbyMinor, unsigned char* pbyRevision);

#ifdef __cplusplus
}
#endif

/* ===============================================================================================*/
#endif //__G711ENC_H__
