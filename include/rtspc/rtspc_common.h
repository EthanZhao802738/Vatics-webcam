/* =========================================================================================== */
#ifndef _RTSPC_COMMON_H_
#define _RTSPC_COMMON_H_



#ifdef _DEBUG
#include <stdio.h>
#include <stdarg.h>
static inline void debug_printf(const char *fmt,...)
{
  va_list args;
  va_start (args, fmt);
  vfprintf(stderr, fmt, args);
  va_end (args);
}
#define DBPRINT0(S)	printf(S);
#define DBPRINT1(S, S1)	printf(S, S1);
#define DBPRINT2(S, S1, S2)	printf(S, S1, S2);
#define DBPRINT3(S, S1, S2, S3)	printf(S, S1, S2, S3);
#define DBPRINT4(S, S1, S2, S3, S4)	printf(S, S1, S2, S3, S4);
#else
#define debug_printf(...)
#define DBPRINT0(S)
#define DBPRINT1(S, S1)
#define DBPRINT2(S, S1, S2)
#define DBPRINT3(S, S1, S2, S3)
#define DBPRINT4(S, S1, S2, S3, S4)
#endif





//
// return type
//
// to avoid conflicting with windows type
#if !defined(__wtypes_h__) && !defined(_SCODE_)
typedef unsigned int SCODE;
#define	_SCODE_
#endif

//
// general return codes
//
#ifndef S_OK
#define S_OK                           ((SCODE)  0)
#endif
#ifndef S_FAIL
#define S_FAIL                         ((SCODE) -1)
#endif











/*! Make from four character codes to one 32-bits DWORD */
#ifndef MAKEFOURCC
    #define MAKEFOURCC(ch0, ch1, ch2, ch3)	((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) | ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif //defined(MAKEFOURCC)


/*! FOURCC codec for H263 codec codec */
#define FOURCC_H263 (MAKEFOURCC('H','2','6','3'))
/*! FOURCC codec for MPEG-4 video codec */
#define FOURCC_MP4V (MAKEFOURCC('M','P','4','V'))
/*! FOURCC for MPEG-2 video codec */
#define FOURCC_MP2V (MAKEFOURCC('M','P','2','V'))
/*! FOURCC for JPEG image codec */
#define FOURCC_JPEG (MAKEFOURCC('J','P','E','G'))
/*! FOURCC codec for H264 codec */
#define FOURCC_H264 (MAKEFOURCC('H','2','6','4'))
/*! FOURCC codec for H265 codec */
#define FOURCC_H265 (MAKEFOURCC('H','2','6','5'))


/*! FOURCC for MPEG-1 layer 3 audio codec */
#define FOURCC_MP3A (MAKEFOURCC('M','P','3','A'))
/*! FOURCC for MPEG-2 AAC audio codec */
#define FOURCC_AAC2 (MAKEFOURCC('A','A','C','2'))
/*! FOURCC for MPEG-4 AAC audio codec */
#define FOURCC_AAC4 (MAKEFOURCC('A','A','C','4'))

/*! FOURCC for GSM-AMR speech codec */
#define FOURCC_GAMR (MAKEFOURCC('G','A','M','R'))
/*! FOURCC for G.711 codec */
#define FOURCC_G711 (MAKEFOURCC('G','7','1','1'))
/*! FOURCC for a-LAW codec */
#define FOURCC_ALAW (MAKEFOURCC('A','L','A','W'))
/*! FOURCC for u-LAW codec */
#define FOURCC_ULAW (MAKEFOURCC('U','L','A','W'))
/*! FOURCC for G.722 speech codec */
#define FOURCC_G722 (MAKEFOURCC('G','7','2','2'))
/*! FOURCC for G.722.1 speech codec */
#define FOURCC_G721 (MAKEFOURCC('G','7','2','1'))
/*! FOURCC for G.723.1 speech codec */
#define FOURCC_G723 (MAKEFOURCC('G','7','2','3'))
/*! FOURCC for G.728 speech codec */
#define FOURCC_G728 (MAKEFOURCC('G','7','2','8'))
/*! FOURCC for G.729 speech codec */
#define FOURCC_G729 (MAKEFOURCC('G','7','2','9'))
/*! FOURCC for G.729AB speech codec */
#define FOURCC_729A (MAKEFOURCC('7','2','9','A'))
/*! FOURCC for G.729B speech codec */
#define FOURCC_729B (MAKEFOURCC('7','2','9','B'))

/*! FOURCC for VIVO codec */
#define FOURCC_VIVO (MAKEFOURCC('V','I','V','O'))
/*! FOURCC for TEXT codec */
#define FOURCC_TEXT (MAKEFOURCC('T','E','X','T'))
/*! FOURCC for HINT codec */
#define FOURCC_HINT (MAKEFOURCC('H','I','N','T'))
/*! FOURCC for EVNT */
#define FOURCC_EVNT (MAKEFOURCC('E','V','N','T'))
/*! FOURCC for Motion Event */
#define FOURCC_MOTN (MAKEFOURCC('M','O','T','N'))
/*! FOURCC for Digital Input Event */
#define FOURCC_DIGI (MAKEFOURCC('D','I','G','I'))
/*! FOURCC codec for Meta Data */
#define FOURCC_MDAT (MAKEFOURCC('M','D','A','T'))

/*! FOURCC for configuration */
#define FOURCC_CONF	(MAKEFOURCC('C','O','N','F'))

/*! FOURCC for proprietary codec */
#define FOURCC_PROP	(MAKEFOURCC('P','R','O','P'))

/*! FOURCC for display format */
#define FOURCC_YM12	(MAKEFOURCC('Y','M','1','2'))
























#if defined(_WIN32) || defined(_CYGWIN)
	#include <limits.h>
#else //_WIN32
	#include <linux/version.h>

	#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 0))
	#include <limits.h>
	#endif
#endif //_WIN32

// to avoid confliction if including "windows.h"
#ifndef _WINDOWS_
// the following types are already define in windows.h

#ifndef __TYPES_H__		// for psos
// the following types are already define in psos.h

#ifndef UCHAR	// uc
typedef unsigned char   UCHAR;
#endif

#ifndef ULONG	// ul
typedef unsigned long   ULONG;
#endif

#ifndef USHORT	// us
typedef unsigned short  USHORT;
#endif

#ifndef UINT	// ui
typedef unsigned int    UINT;
#endif

#endif	// end of __TYPES_H__	// for psos

//  1 byte
#ifndef char	// c
typedef char            CHAR;
#endif

#ifndef PCHAR	// pc
typedef char            *PCHAR;
#endif

#ifndef PUCHAR	// puc
typedef unsigned char   *PUCHAR;
#endif

#ifndef BYTE	// by
typedef unsigned char   BYTE;
#endif

#ifndef PBYTE	// pby
typedef BYTE*           PBYTE;
#endif

//  2 bytes
#ifndef short	// s
typedef short           SHORT;
#endif

#ifndef PSHORT	// ps
typedef short           *PSHORT;
#endif

#ifndef PUSHORT	// pus
typedef unsigned short  *PUSHORT;
#endif

#ifndef WORD	// w
typedef unsigned short  WORD;
#endif

#ifndef PWORD	// pw
typedef WORD*           PWORD;
#endif

//  4 bytes
#ifndef DWORD	// dw
	#if defined(_WIN32) || defined(_CYGWIN)
		#if defined(__arm)	// for ADS compiler -- armcc
			typedef unsigned long   DWORD;
		#elif UINT_MAX == 4294967295
			typedef unsigned int	DWORD;
		#elif ULONG_MAX == 4294967295
			typedef unsigned long   DWORD;
		#endif // ULONG_MAX == 4294967295
	#else
		#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 0))
			typedef unsigned long   DWORD;
		#elif defined(__arm)	// for ADS compiler -- armcc
			typedef unsigned long   DWORD;
		#elif UINT_MAX == 4294967295
			typedef unsigned int	DWORD;
		#elif ULONG_MAX == 4294967295
			typedef unsigned long   DWORD;
		#endif // ULONG_MAX == 4294967295
	#endif //_WIN32
#endif

#ifndef PDWORD	// pdw
typedef DWORD*          PDWORD;
#endif

#ifndef PUINT	// pui
typedef UINT*           PUINT;
#endif

#ifndef long	// l
typedef long            LONG;
#endif

#ifndef PLONG	// pl
typedef long            *PLONG;
#endif

#ifndef PULONG	// plu
typedef unsigned long   *PULONG;
#endif


#ifndef BOOLEAN	// b
typedef unsigned int    BOOLEAN;
#endif

#ifndef BOOL	// b
typedef BOOLEAN         BOOL;
#endif

// bool is keyword in C++
#if defined(_WIN32) || defined(_CYGWIN)
	#if !defined(bool) && !defined(__cplusplus)
	typedef BOOLEAN         bool;
	#endif
#else
	#if !defined(bool) && !defined(__cplusplus) && (LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 0))
	typedef BOOLEAN         bool;
	#endif
#endif //_WIN32

#ifndef PVOID	// pv
typedef void *          PVOID;
#endif

#ifndef HANDLE	// h
typedef void *          HANDLE;
#endif

#ifndef SOCKET  //sck
typedef int             SOCKET;
#endif

/* the new SCODE is defined in errordef.h
#ifndef SCODE	// sc
typedef signed int	SCODE;
#endif
*/
// --------- for floating point -------------
#ifndef FLOAT	// fl
#ifdef _DOUBLE_PRECISION
	typedef double FLOAT;
#else
	typedef float  FLOAT;
#endif

#endif

#endif

// non-window conflict types

#ifndef SCHAR   //c
typedef signed char     SCHAR;
#endif

#ifndef SWORD	// sw
typedef signed short    SWORD;
#endif

#ifndef SDWORD	// sdw
	#if defined(_WIN32) || defined(_CYGWIN)
		#if INT_MAX == 2147483647
			typedef signed int		SDWORD;
		#elif LONG_MAX == 2147483647
			typedef signed long     SDWORD;
		#endif // LONG_MAX == 2147483647
	#else
		#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 0))
			typedef signed int		SDWORD;
		#elif INT_MAX == 2147483647
			typedef signed int		SDWORD;
		#elif LONG_MAX == 2147483647
			typedef signed long     SDWORD;
		#endif // LONG_MAX == 2147483647
	#endif
#endif

#ifndef TASK
typedef void            TASK;
#endif

#if defined(_WIN32)
	typedef unsigned __int64 	QWORD;	// qw
	typedef __int64				SQWORD;	// sqw
#elif defined(_EQUATOR_X_) || defined(__arm)
	typedef unsigned long long	QWORD;	// qw
    typedef long long           SQWORD;	// sqw
#elif defined(__GNUC__) || defined(_CYGWIN)
	typedef unsigned long long int	QWORD;	// qw
    typedef long long int           SQWORD;	// sqw
#endif


// --------- for fix point -------------
typedef signed short     FIX16;
typedef unsigned short   UFIX16;
typedef signed long      FIX;
typedef unsigned long    UFIX;


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

#define ON  1
#define OFF 0

#ifndef S_OK
#define S_OK 0
#endif

#ifndef S_FAIL
#define S_FAIL (SCODE)(-1)
#endif

#ifndef S_INVALID_VERSION
#define S_INVALID_VERSION (SCODE)(-2)
#endif










/* =========================================================================================== */
#endif //_RTSPC_COMMON_H_

