#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <cstdio>
#include <cstring>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <SyncRingBuffer/sync_ring_buffer.h>
#include <MsgBroker/msg_broker.h>
#include <vmf/video_encoder.h>

#include <axclib.h>
#include <algorithm>
#include <vector>
#include <net.h>



using namespace std;


#define MAKEFOURCC(ch0, ch1, ch2, ch3)  ((unsigned int)(unsigned char)(ch0) | ((unsigned int)(unsigned char)(ch1) << 8) | ((unsigned int)(unsigned char)(ch2) << 16) | ((unsigned int)(unsigned char)(ch3) << 24 ))

#define FOURCC_CONF (MAKEFOURCC('C','O','N','F'))	
#define FOURCC_H264 (MAKEFOURCC('H','2','6','4'))
#define FOURCC_H265 (MAKEFOURCC('H','2','6','5'))
#define FOURCC_JPEG (MAKEFOURCC('J','P','E','G'))

#define FOURCC_AAC4 (MAKEFOURCC('A','A','C','4'))
#define FOURCC_G711 (MAKEFOURCC('G','7','1','1'))
#define FOURCC_G726 (MAKEFOURCC('G','7','2','6'))
#define FOURCC_GAMR (MAKEFOURCC('G','A','M','R'))
#define FOURCC_GPCM (MAKEFOURCC('G','P','C','M'))

#define OUTPUT_BUFFER_HEADER 256

#define SRB_RCV_VENC_PIN1     "venc_srb_1"                //! VMF_VideoEnc Output pin
#define SRB_RCV_VENC_PIN2     "venc_srb_2"                //! VMF_VideoEnc Output pin
#define SRB_RCV_VENC_CMD_FIFO "/tmp/venc/c0/command.fifo" //! communicate with rtsps, vrec, etc.

#define SRB_RCV_AENC_PIN      "aenc_srb_1"                
#define SRB_RCV_AENC_CMD_FIFO "/tmp/aenc/c0/command.fifo"
#define SRB_RCV_CMD_FIFO "/tmp/srb_receiver_command.fifo"

#define WIDTH    1920
#define HEIGHT  1080



bool g_bFifoPush = 1;
bool g_bBlocking = 0;
bool g_bRunning = 1;
extern int g_bTerminate;
int g_bSuspend = 0;
char* g_pszOutputPath = NULL;
SRB_HANDLE_T* g_aptSrbHandle[2] = {NULL, NULL};
unsigned int g_bPause[3] = {0, 0, 0};


static bool g_deviceOpen = false;
static bool g_bStopCapture = false;

static CAxcThread g_thread_IMX307Capture("M5s307_capture");
static CAxcThread g_thread_IMX307Capture_nonblocking("M5s307_capture_nonblocking");
static axc_dword thread_IMX307Capture(CAxcThread *pThread, void *pContext);
static axc_dword thread_IMX307Capture_nonblocking(CAxcThread *pThread, void *pContext);
static axc_dword srb_reader(CAxcThread *pThread, void *pContext);
static axc_dword srb_reader_nonblocking(CAxcThread *pThread, void *pContext);
extern CAxcFifoBufferPtr g_fifoRawFrames;




/*void srb_receiver_suspend()
{
	g_bSuspend = 1;
	
	if (g_bBlocking) {
		while(1)
		{
			if(g_bPause[0]) {
				break;
			}
			usleep(10000);
		}
	} else {
		if (g_aptSrbHandle[0]) {
			SRB_WakeupReader(g_aptSrbHandle[0]);
		}
		if (g_aptSrbHandle[1]) {
			SRB_WakeupReader(g_aptSrbHandle[1]);
		}
		while(1)
		{
			if(g_bPause[0]) {
				break;
			}
			usleep(10000);
		}
		while(1)
		{
			if(g_bPause[1]) {
				break;
			}
			usleep(10000);
		}
	}
}*/

/*void srb_receiver_resume()
{
	g_bSuspend = 0;

	if (g_bBlocking) {
		pthread_cond_signal(&g_VideoCond[0]);
	} else {
		pthread_cond_signal(&g_VideoCond[0]);
		pthread_cond_signal(&g_VideoCond[1]);
	}
}*/



int cmdfifo_video_sender(const char* cmd, int ch)
{
	MsgContext msg_context;
	char host[64] = {0};		
	sprintf(host, "encoder%d", ch);
	
	msg_context.pszHost = host;
	msg_context.dwHostLen = strlen(host) + 1;
	msg_context.pszCmd = cmd;
	msg_context.dwCmdLen = strlen(msg_context.pszCmd) + 1;
	msg_context.dwDataSize = 0;
	msg_context.pbyData = NULL;					
	msg_context.bHasResponse = 0;		
	if (MsgBroker_SendMsg(SRB_RCV_VENC_CMD_FIFO, &msg_context)) {
		return 0;
	}

	return -1;
}


#define BUFFER_SIZE 100

static void double_buf(unsigned int channel, unsigned char *pbyEncPtr, unsigned int dataSize) 
{
	static unsigned char tmpbuf[2][2][BUFFER_SIZE] = {0};
	static unsigned int buf_idx[2] = {0};
	static unsigned char bswitch[2] = {0};
	unsigned char* buf;
		
	if (buf_idx[channel] + dataSize < BUFFER_SIZE) {
		buf = tmpbuf[channel][bswitch[channel]];
		memcpy(buf+buf_idx[channel], pbyEncPtr, dataSize);
		buf_idx[channel] = buf_idx[channel] + dataSize; 		
	}
	else {
		g_fifoRawFrames.Push(tmpbuf[channel][bswitch[channel]], buf_idx[channel], NULL, (( WIDTH & 0xFFFF) << 16) | ( HEIGHT & 0xFFFF));
            
		if (dataSize > BUFFER_SIZE ) {
			g_fifoRawFrames.Push(pbyEncPtr,dataSize,NULL,(( WIDTH & 0xFFFF) << 16) | ( HEIGHT & 0xFFFF));
			return;
		}
			
		bswitch[channel] = !bswitch[channel];
		buf = tmpbuf[channel][bswitch[channel]];
		buf_idx[channel] = 0;
		memcpy(buf+buf_idx[channel], pbyEncPtr, dataSize);
		buf_idx[channel] = buf_idx[channel] + dataSize; 
	}
}



void h264_output_data(unsigned int dwIndex, bool* pbIsKeyFrame, unsigned char *pbyEncPtr)
{
	VMF_VENC_STREAM_DATA_HDR* data_hdr = (VMF_VENC_STREAM_DATA_HDR*) pbyEncPtr;
	unsigned char* pdwPayload = pbyEncPtr + OUTPUT_BUFFER_HEADER;
				
	if (data_hdr->bIsKeyFrame) {
		*pbIsKeyFrame = 1;
	}

	if (!(*pbIsKeyFrame)) {
		return;
	}
	
	/*printf("[srb_receiver] %s() ch[%d] nal type(%d), data size(%d) \n", 
		__func__, dwIndex, (pdwPayload[4] & 0x1F), data_hdr->dwDataBytes);*/
				
	if (g_bFifoPush) {		
		double_buf(dwIndex - 1, pdwPayload, data_hdr->dwDataBytes);
	}
}

void h265_output_data(unsigned int dwIndex, bool* pbIsKeyFrame, unsigned char *pbyEncPtr)
{
	VMF_VENC_STREAM_DATA_HDR* data_hdr = (VMF_VENC_STREAM_DATA_HDR*) pbyEncPtr;
	unsigned char* pdwPayload = pbyEncPtr + OUTPUT_BUFFER_HEADER + data_hdr->dwBufOffset;
	
	if (data_hdr->bIsKeyFrame) {
		*pbIsKeyFrame = 1;				
	}
								
	if (!(*pbIsKeyFrame)) {
		return;
	}
	
	printf("[srb_receiver] %s() ch[%d] nal type(%d), data size(%d) \n",
		__func__, dwIndex, (pdwPayload[4]>>1) & 0x3F, data_hdr->dwDataBytes);

	if (g_bFifoPush) { 
		double_buf(dwIndex - 1, pdwPayload, data_hdr->dwDataBytes);
	}
}



static void gen_output_path(char* pszOutputPath, char* pszFolder, unsigned int dwChannelIdx, const char* pszFileName)
{
	if (pszFolder) {
		if (pszFolder[strlen(pszFolder) - 1] == '/') {
			sprintf(pszOutputPath, "%svtcs_srb_ch%d.%s", pszFolder, dwChannelIdx, pszFileName);
		} else {
			sprintf(pszOutputPath, "%s/vtcs_srb_ch%d.%s", pszFolder, dwChannelIdx, pszFileName);
		}
	} else {
		sprintf(pszOutputPath, "vtcs_srb_ch%d.%s", dwChannelIdx, pszFileName);
	}
}


static axc_dword srb_reader(CAxcThread *pThread, void *pContext)
{
	int ret = 0;
	//unsigned int dwChannelIdx = (unsigned int) pContext;
	unsigned int dwChannelIdx = (unsigned int) 1;
	SRB_HANDLE_T* ptSrbHandle = NULL;	
	SRB_BUFFER_T srb_buf;	
	unsigned int* values;
	unsigned char *enc_ptr;
	bool bIsKeyframe = 0;
	unsigned char vps[64] = {0};
	unsigned char sps[64] = {0};
	unsigned char pps[64] = {0};  
	unsigned short vps_size = 0;
	unsigned short sps_size = 0;
	unsigned short pps_size = 0; 	


	if (dwChannelIdx == 1) {
		ptSrbHandle = g_aptSrbHandle[0] = SRB_InitReader(SRB_RCV_VENC_PIN1);
	} else {
		ptSrbHandle = g_aptSrbHandle[1] = SRB_InitReader(SRB_RCV_VENC_PIN2);
	}
	memset(&srb_buf, 0, sizeof(SRB_BUFFER_T));	
	cmdfifo_video_sender("start", dwChannelIdx-1);
	cmdfifo_video_sender("forceCI", dwChannelIdx-1); // conf	

	while (!g_bTerminate) {

		//! Receive video data 
		ret = SRB_ReturnReceiveReaderBuff(ptSrbHandle, &srb_buf);
		if (ret < 0)  { 
			usleep(1000); 
			printf("[srb_receiver] %s() SRB_ReturnReceiveReaderBuff Fail !\n", __func__); 
			continue; 
		}
		else if(g_bSuspend) {
			continue;
		}
		enc_ptr = srb_buf.buffer;		
		values = (unsigned int*) enc_ptr;

		if (values[0] == FOURCC_CONF) {
			if (values[2] == FOURCC_H264) {		
				//! SPS/PPS 
				unsigned char* dst_buf = enc_ptr + 32;  
				sps_size = values[6];
				pps_size = values[7];
				if (sps_size < sizeof(sps)) {
					memcpy(sps, dst_buf, sps_size);
				}
				if (pps_size < sizeof(pps)) {
					memcpy(pps, dst_buf + sps_size, pps_size);	
				}
				//printf("[srb_receiver] %s() h264 - sps_size(%d) pps_size(%d) \n", __func__, sps_size, pps_size);
			} else if (values[2] == FOURCC_H265) {
				unsigned char* dst_buf = enc_ptr + 36;  
				vps_size = values[6];
				sps_size = values[7];
				pps_size = values[8];
				if (vps_size < sizeof(vps)) {
					memcpy(vps, dst_buf, vps_size);
				}
				
				if (sps_size < sizeof(sps)) {
					memcpy(sps, dst_buf + vps_size, sps_size);	
				}
				
				if (pps_size < sizeof(pps)) {
					memcpy(pps, dst_buf + vps_size + sps_size, pps_size);	
				}
				if (g_bFifoPush) {
					/*if (!fp) {
						char szPath[128] = {0};
						gen_output_path(szPath, g_pszOutputPath, dwChannelIdx, "h265");						
						fp = fopen(szPath, "wb+");
						fwrite(vps, vps_size, 1, fp);
						fwrite(sps, sps_size, 1, fp);
						fwrite(pps, pps_size, 1, fp);
					}*/
				}
				
				printf("[srb_receiver] %s() h265 - vps_size(%d) sps_size(%d) pps_size(%d) \n", __func__, vps_size, sps_size, pps_size);
			}
		} else {	
			switch (values[0]) {
				case FOURCC_H264: {
					h264_output_data(dwChannelIdx, &bIsKeyframe, enc_ptr);
				} break;
				
				case FOURCC_H265: {
					h265_output_data(dwChannelIdx, &bIsKeyframe, enc_ptr);
				} break;
				
			}				
		}
	}

	SRB_ReturnReaderBuff(ptSrbHandle, &srb_buf);
	SRB_Release(ptSrbHandle);
	cmdfifo_video_sender("stop", dwChannelIdx - 1);
	
	return NULL;
}


static axc_dword srb_reader_nonblocking(CAxcThread *pThread, void *pContext)
{
	int ret = 0;
	unsigned int* values;	
	g_aptSrbHandle[0] = SRB_InitReader(SRB_RCV_VENC_PIN1);
	g_aptSrbHandle[1] = SRB_InitReader(SRB_RCV_VENC_PIN2);	
	SRB_BUFFER_T srb_buf[3];
	
	memset(&srb_buf, 0, sizeof(SRB_BUFFER_T)*3);
	cmdfifo_video_sender("start", 0);
	cmdfifo_video_sender("start", 1);	

	while (g_bRunning) {
		if(g_bSuspend){
			SRB_ReturnReaderBuff(g_aptSrbHandle[0], &srb_buf[0]);
			SRB_ReturnReaderBuff(g_aptSrbHandle[1], &srb_buf[1]);
		}
		ret = SRB_QueryReaderBuff(g_aptSrbHandle[0], &srb_buf[0]);
		if (ret == 0) {
			values = (unsigned int*) srb_buf[0].buffer;
			printf("[srb_receiver] %s() 1! data size: %d \n", __func__, values[3]);
		} else if (ret == 1) {
			//printf("1! query with no result! \n");
			usleep(1);
		}
		ret = SRB_QueryReaderBuff(g_aptSrbHandle[1], &srb_buf[1]);
		if (ret == 0) {
			values = (unsigned int*) srb_buf[1].buffer;
			printf("[srb_receiver] %s() 2! data size: %d \n", __func__, values[3]);			
		} else if (ret == 1) {
			//printf("2! query with no result! \n");
			usleep(1);
		}		
	}

	SRB_ReturnReaderBuff(g_aptSrbHandle[0], &srb_buf[0]);
	SRB_Release(g_aptSrbHandle[0]);
	
	SRB_ReturnReaderBuff(g_aptSrbHandle[1], &srb_buf[1]);	
	SRB_Release(g_aptSrbHandle[1]);
	

	cmdfifo_video_sender("stop", 0);
	cmdfifo_video_sender("stop", 1);	

	return NULL;
}


void srb_receiver_msg_callback(MsgContext* msg_context, void* user_data)
{	
	(void) user_data;
	//printf("Host: %s Cmd: %s\n", msg_context->pszHost, msg_context->pszCmd);

	if( !strcasecmp(msg_context->pszHost, SR_MODULE_NAME) ){
		if( !strcasecmp(msg_context->pszCmd, SUSPEND_CMD) ) {
			//srb_receiver_suspend(); //! suspend

			MsgBroker_SuspendAckMsg();
		}else if( !strcasecmp(msg_context->pszCmd, RESUME_CMD) ) {
			//srb_receiver_resume(); //! resume	

		}
	}
	msg_context->bHasResponse = 0;
}



bool IMX307_Init(bool nonblocking)
{
    // create vido-capture thread

	if (nonblocking) {
		g_thread_IMX307Capture_nonblocking.Create(srb_reader_nonblocking, NULL, 0, 11);
	} else {
		printf("srb_reader start\n");
		g_thread_IMX307Capture.Create(srb_reader,NULL,0,11);
	}
	
	//printf("MsgBroker_Run\n");

	//MsgBroker_Run(SRB_RCV_CMD_FIFO, srb_receiver_msg_callback, NULL, &g_bTerminate);

}


void IMX307_Release()
{
	g_thread_IMX307Capture.Destroy(1000);
}

