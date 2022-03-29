
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

#ifndef MSG_FORMAT_H
#define MSG_FORMAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define MSG_SPKR_ENROLL 	"enroll"
#define MSG_SPKR_VERIFY 	"verify"
#define MSG_SPKR_STOP 		"stop"
#define MSG_SPKR_RESET 		"reset"
#define MSG_SPKR_THRESHOLD 	"threshold"

typedef struct  {
	int speaker_index;	
	pid_t speak_pid;
} Speak_info_t;

#ifdef __cplusplus
}
#endif

#endif
