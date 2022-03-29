
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
#include <stdarg.h>


static void print_msg(const char *fmt, ...)
{
	va_list ap;		
	va_start(ap, fmt);
	fprintf(stderr, "[%s] ", "atu");
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

int main(int argc, char* argv[])
{
	int ch;
	MsgContext tMsgCtx;

	tMsgCtx.bHasResponse = 0;
	tMsgCtx.pszHost = "atu_cmd";
	tMsgCtx.dwHostLen = strlen("atu_cmd") + 1;
	tMsgCtx.dwDataSize = 0;
	tMsgCtx.pbyData = NULL;
	
	while ((ch = getopt(argc, argv, "edr")) != -1)
	{
		switch(ch)
		{
			case 'e': 
			{
				tMsgCtx.pszCmd = "enable";
				break;	
			}
			case 'd': 
			{
				tMsgCtx.pszCmd = "disable";
				break;	
			}
			case 'r': 
			{
				tMsgCtx.pszCmd = "reset";
				break;	
			}
			default:
				print_msg("Usage: %s [-e][-d][-r]\r\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}
	tMsgCtx.dwCmdLen = strlen(tMsgCtx.pszCmd) + 1;
	MsgBroker_SendMsg("/tmp/atu_cmd.fifo", &tMsgCtx);
	
	return 0;
}

