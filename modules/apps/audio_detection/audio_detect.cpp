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
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <string>
#include <vmf/audio_tu.h>

#include <MsgBroker/msg_broker.h>

#define ATU_CMD_FIFO "/tmp/atu_cmd.fifo"

static int g_terminate = 0;
static VMF_ATU_HANDLE_T* g_handle = NULL;

static void exit_process()
{
	g_terminate = 1;
}

static void msg_callback(MsgContext* msg_context, void* user_data)
{
	(void) user_data;
	printf("msg_context->pszHost=%s, msg_context->pszCmd=%s \n", 
			msg_context->pszHost, msg_context->pszCmd);
	if(NULL == g_handle)
		return;

	if (!strcasecmp(msg_context->pszCmd, "enable"))
		VMF_ATU_SetOption(g_handle, VMF_ATU_CMD_ENABLE);
	else if (!strcasecmp(msg_context->pszCmd, "disable")) 
		VMF_ATU_SetOption(g_handle, VMF_ATU_CMD_DISABLE);
	else if (!strcasecmp(msg_context->pszCmd, "reset")) 
		VMF_ATU_SetOption(g_handle, VMF_ATU_CMD_RESET_FLAG);

	if (msg_context->bHasResponse) {
		msg_context->dwDataSize = 0;
	}
}

static void sig_kill(int signo)
{
	printf("[%s,%s] Receive SIGNAL %d!!!\n", __FILE__, __func__, signo);
	switch(signo)
	{
		case SIGTERM:
		case SIGINT:
			exit_process();
			break;
		default:
			break;
	}
}

static void dump_trace(int /*signo*/)
{
	printf(" ===== Segmentation fault. ===== \n");
	exit(EXIT_FAILURE);
}

static void print_usage(const char *ap_name)
{
	fprintf(stderr, "Usage:\n"
			"    %s [-D]\n"
			"Options:\n"
			"    -D                 Run as Daemon.\n"
			"    -h                 This help\n", ap_name);
}

void* alarm_checker(void*)
{
	VMF_ATU_RESULT_T result;

	g_handle = VMF_ATU_Init();
	if(NULL == g_handle) {
		printf("[%s] VMF_ATU_Init fail!!\n", __func__);
		return NULL;
	}

	while(!g_terminate) {
		if(0 == VMF_ATU_GetResult(g_handle, &result)) {
			if(result.bT3Detected)
				printf("[Alarm] T3 alarm detected!!\n");
			if(result.bT4Detected)
				printf("[Alarm] T4 alarm detected!!\n");
			printf("Azimuth estimation is %d\n", result.iDOAResult);
		}
		else
			printf("VMF_ATU_GetResult error\n");
		sleep(2);
	}

	VMF_ATU_Release(g_handle);
	return NULL;
}

int main(int argc, char **argv)
{
	int opt;
	bool is_daemon = false;

	pthread_t checker_thread;

	while ((opt = getopt(argc, argv, "Dh")) != -1)
	{
		switch(opt)
		{
			case 'D':
				is_daemon = true;
				break;
			case 'h':
			default:
				print_usage(argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	signal(SIGTERM, sig_kill);
	signal(SIGINT, sig_kill);
	signal(SIGSEGV, dump_trace);

	if(is_daemon)
		daemon(1,1);

	if(0 != pthread_create(&checker_thread, NULL, alarm_checker, NULL)) {
		printf("[ATU] pthread create fail!!\n");
		exit(EXIT_FAILURE);
	}

	MsgBroker_Run(ATU_CMD_FIFO, msg_callback, NULL, &g_terminate);

	pthread_join(checker_thread, NULL);
	return 0;
}

