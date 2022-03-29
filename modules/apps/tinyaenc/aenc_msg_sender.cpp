/*
 *******************************************************************************
 *  Copyright (c) 2010-2017 VATICS Inc. All rights reserved.
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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <MsgBroker/msg_broker.h>
#include <aenc_msg_format.h>


static void print_usage(const char *name)
{
	printf("Usage:\n"
		" %s -e enroll_speaker_index. The upper bound is 10 (0~9). \n"
		" %s -v Recognize speaker.\n"
		" %s -s Stop enrolling speaker or recognizing speaker.\n"
		" %s -r Reset speaker.\n"
		" %s -t SPKR_threshold  \n"
		"Note: After starting to enroll speaker, speaker needs to speak keywords or sentences."
		"Repeat keywords or sentences and separate each keyword or sentence can raise the accuracy.\n"
		, name, name, name, name, name);
}

static int send_msg(MsgContext *msg, uint8_t *data, size_t data_size)
{
	if (!msg)
		return -1;
	msg->dwHostLen  = strlen(msg->pszHost) + 1;
	msg->dwCmdLen   = strlen(msg->pszCmd) + 1;
	msg->dwDataSize = data_size;
	msg->pbyData    = data;
	msg->bHasResponse = 1;
	printf("[send_msg]\n");
	MsgBroker_SendMsg("/tmp/aenc/c0/command.fifo", msg);
	return 0;
}

int main(int argc, char* argv[])
{
	int ch = 0;
	MsgContext msg_context;	
	Speak_info_t speak_info;
	int threshold = 0;
	memset(&speak_info, 0, sizeof(Speak_info_t));
	pid_t aenc_msg_pid = getpid();
	
	msg_context.pszHost = "spkr";
	while ((ch = getopt(argc, argv, "e:vst:rh")) != -1)
	{
		switch(ch)
		{
			case 'e': {
				msg_context.pszCmd = MSG_SPKR_ENROLL;
				speak_info.speaker_index = atoi(optarg);
				speak_info.speak_pid = aenc_msg_pid;
				send_msg(&msg_context, (uint8_t*)&speak_info, sizeof(Speak_info_t));
				break;
			}
			case 'v': {
				msg_context.pszCmd = MSG_SPKR_VERIFY;
				send_msg(&msg_context, (uint8_t*)&aenc_msg_pid, sizeof(pid_t));
				break;
			}
			case 's': {
				msg_context.pszCmd = MSG_SPKR_STOP;
				send_msg(&msg_context, NULL, 0);
				break;
			}
			case 'r': {
				msg_context.pszCmd = MSG_SPKR_RESET;
				send_msg(&msg_context, NULL, 0);
				break;
			}
			case 't': {
				msg_context.pszCmd = MSG_SPKR_THRESHOLD;
				threshold = atoi(optarg);
				send_msg(&msg_context, (uint8_t*)&threshold, sizeof(int));
				break;
			}
			case 'h':
			default:
				print_usage(argv[0]);
				exit(EXIT_FAILURE);
		}
	}
	

	return 0;
}
