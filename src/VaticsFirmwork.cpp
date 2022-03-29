#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>
#include <unistd.h>
#include <stdarg.h>

#include <MemBroker/mem_broker.h>
#include <MsgBroker/msg_broker.h>
#include <sync_shared_memory.h>
#include <vmf/video_encoder_output_srb.h>
#include <vmf/video_source.h>
#include <vmf/video_encoder.h>
#include <vmf/video_bind.h>
#include <comm/frame_info.h>



#define MODULE_NAME	"Vatics_venc1"
#define VENC_OUTPUT_BUF_NUM  3
#define VENC_VSRC_PIN     "vsrc_ssm"					//! VMF_VSRC Output pin
#define VENC_OUTPUT_PIN   "venc_srb_1"					//! VMF_VideoEnc Output pin
#define VENC_OUTPUT_PIN_RS   "venc_srb_2"					//! VMF_VideoEnc Output pin
#define VENC_CMD_FIFO     "/tmp/venc/c0/command.fifo" 	//! communicate with rtsps, vrec, etc.
#define VENC_RESOURCE_DIR  "./Resource/"                //! directory contains ISP, AE, AWB, AutoScene sub directory
#define VENC_ENCODE_BUF_SIZE (4*1024*1024)	            //! 4*1024*1024
#define VENC_ENCODE_BUF_SIZE_RS (1*1024*1024)	            //! 4*1024*1024

VMF_LAYOUT_T g_tLayout;
static int g_iTerminate;
static int g_iProcIfpOnly;
unsigned int g_dwStreamingNum = 0;
unsigned int g_dwStreamingNumRs = 0;
unsigned int g_eFecMethod = 0;
char *g_ptszAutoSceneConfig = NULL;
char *g_ptszSensorConfig = NULL;
VMF_VSRC_HANDLE_T *g_ptVsrcHandle = NULL;
VMF_BIND_CONTEXT_T *g_ptBind = NULL;
VMF_VENC_OUT_SRB_T *g_ptVencOutputSRB = NULL;
VMF_VENC_OUT_SRB_T *g_ptVencOutputRsSRB = NULL;
VMF_VENC_HANDLE_T *g_ptVencHandle = NULL;
VMF_VENC_HANDLE_T *g_ptVencRsHandle = NULL;


void print_msg(const char *fmt, ...)
{
	va_list ap;		
	va_start(ap, fmt);
	fprintf(stderr, "[%s] ", MODULE_NAME);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}


static void vsrc_init_callback(unsigned int width, unsigned int height)
{
    memset(&g_tLayout, 0, sizeof(VMF_LAYOUT_T));
    g_tLayout.dwCanvasHeight = height;
    g_tLayout.dwCanvasWidth = width;
    g_tLayout.dwVideoPosX = 0;
    g_tLayout.dwVideoPosY = 0;
    g_tLayout.dwVideoWidth = width;
    g_tLayout.dwVideoHeight = height;
    print_msg("[%s]: width:%d, height:%d \n",__func__, width, height);
}


static void release_video_source(void)
{
    VMF_VSRC_Stop(g_ptVsrcHandle);
    VMF_VSRC_Release(g_ptVsrcHandle);
}

static void setup_spec(VMF_VSRC_SPEC_CONFIG_T *ptSpec, VMF_VENC_CODEC_TYPE eCodecType)
{
    ptSpec->bEnableSpec = 1;
    ptSpec->dwIspMode =VMF_ISP_MODE_DISABLE;

    if (eCodecType == VMF_VENC_CODEC_TYPE_H265)
        ptSpec->tIfpEncSpec.bEncH265 = 1;

    else if (eCodecType == VMF_VENC_CODEC_TYPE_H264)
        ptSpec->tIfpEncSpec.bEncH264 = 1;

    else if (eCodecType == VMF_VENC_CODEC_TYPE_MJPG) 
        ptSpec->tIfpEncSpec.bEncJPEG = 1;


    if (eCodecType == VMF_VENC_CODEC_TYPE_H265)
        ptSpec->tIspEncSpec.bEncH265 = 1;

    else if (eCodecType == VMF_VENC_CODEC_TYPE_H264)
        ptSpec->tIspEncSpec.bEncH264 = 1;

    else if (eCodecType == VMF_VENC_CODEC_TYPE_MJPG) 
        ptSpec->tIspEncSpec.bEncJPEG = 1;
}

static int init_video_source(VMF_VENC_CODEC_TYPE eCodeType)
{
    VMF_VSRC_INITOPT_T tVsrcInitOpt;
    VMF_VSRC_FRONTEND_CONFIG_T tVsrcFrontendConfig;

    memset(&tVsrcInitOpt, 0, sizeof(VMF_VSRC_INITOPT_T));
    memset(&tVsrcFrontendConfig, 0, sizeof(VMF_VSRC_FRONTEND_CONFIG_T));

    // one sensor conf. for the IFPE
    tVsrcFrontendConfig.dwSensorConfigCount= 1;
    tVsrcFrontendConfig.apszSensorConfig[0] = g_ptszSensorConfig;

    // IFPE
    tVsrcInitOpt.dwFrontConfigCount = 1;
    tVsrcInitOpt.ptFrontConfig = &tVsrcFrontendConfig;
	tVsrcInitOpt.pszAutoSceneConfig = g_ptszAutoSceneConfig;
	tVsrcInitOpt.pszOutPinPrefix = VENC_VSRC_PIN;                                   //     SSM name
	tVsrcInitOpt.bShared = 1;                                                                                  //     allow SSM buffer IPC usage
	tVsrcInitOpt.fnInitCallback = vsrc_init_callback;                                   //      callback after VSRC inited  
	tVsrcInitOpt.pszResourceDir = VENC_RESOURCE_DIR;                       //      Resource folder name
    setup_spec(&tVsrcInitOpt.tSpecConfig, eCodeType);

    printf("VMFS_VSRC_Init\n");

    g_ptVsrcHandle = VMF_VSRC_Init(&tVsrcInitOpt);
    if ( !g_ptVsrcHandle )
    {
        print_msg("[%s] VMF_VSRC_Init failed!\n", __func__);
        return -1;
    }

    if ( VMF_VSRC_Start(g_ptVsrcHandle, NULL) != 0 )
    {
        print_msg( "[%s] VMF_VSRC_Start failed!\n", __func__);
        release_video_source();
        return -1;
    }

    return 0;
}


static void release_bind(void)
{
    VMF_BIND_Release(g_ptBind);
}


static int init_bind(void)
{
    VMF_BIND_INITOPT_T tBindOpt;
    memset(&tBindOpt, 0, sizeof(VMF_BIND_INITOPT_T));

    tBindOpt.dwSrcOutputIndex = 0;
    tBindOpt.ptSrcHandle = g_ptVsrcHandle;
    tBindOpt.pfnQueryFunc = (VMF_BIND_QUERY_FUNC) VMF_VSRC_GetInfo;
    tBindOpt.pfnIspFunc = (VMF_BIND_CONFIG_ISP_FUNC) VMF_VSRC_ConfigISP;

    g_ptBind = VMF_BIND_Init(&tBindOpt);
    if ( !g_ptBind )
    {
        print_msg("[%s] VMF_BIND_Init failed!!\n", __func__);
		release_video_source();
		return -1;
    }

    return 0;
}


static int init_output_srb(const char *name, unsigned int buf_number,
                                                          unsigned int buf_size, VMF_VENC_OUT_SRB_T **pptVencOutputSRB )
{
    VMF_VENC_OUT_SRB_INITOPT_T tSrbInitOpt;
    memset(&tSrbInitOpt, 0, sizeof(VMF_VENC_OUT_SRB_INITOPT_T));

    tSrbInitOpt.pszSrbName = name;
    tSrbInitOpt.dwSrbNum  = buf_number;
	tSrbInitOpt.dwSrbSize = buf_size;

    if ( VMF_VENC_OUT_SRB_Init(pptVencOutputSRB, &tSrbInitOpt) != 0 )
    {
        print_msg("[%s] VMF_VENC_OUT_SRB_Init failed!!\n", __func__);
		release_video_source();
		release_bind();
		return -1;
    }
    

    return 0;
}

static void release_video_encoder( VMF_VENC_HANDLE_T *ptVencHandle, VMF_VENC_OUT_SRB_T **pptVencOutputSRB)
{
    VMF_VENC_Release(ptVencHandle);
    VMF_VENC_OUT_SRB_Release(pptVencOutputSRB);
}

static VMF_VENC_HANDLE_T* init_video_encoder(VMF_VENC_CODEC_TYPE eCodecType, unsigned int dwWidth,  unsigned int dwHeight,
	unsigned int dwBitrate, VMF_VENC_OUT_SRB_T* ptVencOutputSRB)
{
    VMF_VENC_HANDLE_T* ptVencHandle = NULL;
	VMF_VENC_CONFIG_T tVencConfig;
	VMF_H4E_CONFIG_T g_h4e_config = {
	    25,							// dwQp
	    dwBitrate,					// dwBitrate
	    30,							// dbFps
	    60,							// dwGop
	    VMF_H4E_PROFILE_HIGH,		// eProfile
	    0,							// iSliceQualityStrategy
	    VMF_ADMODE_MEET_FPS,		// eAdMode
	    0,							// dwMinQp
	    0,							// dwMaxQp
	    0,							// dwMinFps
	    0,                          // dwVirtIFrameInterval
	    0                           // dwPIQ
	};

    VMF_H5E_CONFIG_T g_h5e_config = {
		25,							// dwQp
		dwBitrate,					// dwBitrate
		30,							// dwFps
		30,							// dwGop
		0,                          // iSliceQualityStrategy
		0,							// dwMinQp
		0,							// dwMaxQp	
		0,                          // Virtual I-frame interval
		0,							// dwPIQ
		VMF_ADMODE_MEET_FPS,		// eAdMode
		0                           // Complex map control in VBR mode
	};

	VMF_JE_CONFIG_T g_je_config = {
		50,							// dwQp
		0,							// bEnableThumbnail
		0,							// dwThumbnailQp
		0,							// bJfifHdr
		1024*1024,					// dwBitrate	        
		25		
	};


    memset(&tVencConfig, 0, sizeof(VMF_VENC_CONFIG_T));
    tVencConfig.dwEncHeight = dwHeight;
    tVencConfig.dwEncWidth = dwWidth;
    tVencConfig.dwFps = 30;
    tVencConfig.bConnectIfp = g_iProcIfpOnly;
    print_msg("[%s] enc :%d, %d\n", __func__, dwWidth, dwHeight);

    switch(eCodecType){
		case VMF_VENC_CODEC_TYPE_H264:
			tVencConfig.eCodecType = VMF_VENC_CODEC_TYPE_H264;
			tVencConfig.pCodecConfig = &g_h4e_config;
		    break;

		case VMF_VENC_CODEC_TYPE_H265: 
			tVencConfig.eCodecType = VMF_VENC_CODEC_TYPE_H265;
			tVencConfig.pCodecConfig = &g_h5e_config;
			break;
		
		case VMF_VENC_CODEC_TYPE_MJPG: 
			tVencConfig.eCodecType = VMF_VENC_CODEC_TYPE_MJPG;
			tVencConfig.pCodecConfig = &g_je_config;
			break;

		default: 
			print_msg("[%s] Invalid Codec Type\n", __func__);
			return NULL;
	}
    

    VMF_VENC_OUT_SRB_Setup_Config(&tVencConfig, tVencConfig.eCodecType, tVencConfig.pCodecConfig, ptVencOutputSRB);
    tVencConfig.fnSrcConnectFunc = (VMF_SRC_CONNECT_FUNC) VMF_BIND_Request;
	tVencConfig.pBind = g_ptBind;

    ptVencHandle = VMF_VENC_Init(&tVencConfig);
    if (!ptVencHandle) {
		release_video_source();
		release_bind();
		print_msg("[%s] VMF_VENC_Init() failed\n", __func__);		
		return NULL;
	}

    VMF_VENC_ProduceStreamHdr(ptVencHandle);
	return ptVencHandle;
}


static void msg_callback(MsgContext *msg_context, void *user_data)
{
    (void) user_data; 
    print_msg("msg_context->pszHost=%s, msg_context->pszCmd=%s \n", 
			msg_context->pszHost, msg_context->pszCmd);

    if( !strcasecmp(msg_context->pszHost, "encoder0"))
    {
        if( !strcasecmp(msg_context->pszCmd, "start"))
        {
            if( ++g_dwStreamingNum == 1)
                VMF_VENC_Start(g_ptVencHandle);
        }
        else if ( !strcasecmp(msg_context->pszCmd, "stop"))
        {
            if(g_dwStreamingNum)
            {
                if(--g_dwStreamingNum == 0)
                    VMF_VENC_Stop(g_ptVencHandle);
            }
        }
        else if (!strcasecmp(msg_context->pszCmd, "forceCI"))
        {
            VMF_VENC_ProduceStreamHdr(g_ptVencHandle);
        }
        else if (!strcasecmp(msg_context->pszCmd, "forceIntra"))
        {
            VMF_VENC_SetIntra(g_ptVencHandle);        
        }
    }

    else if ( !strcasecmp(msg_context->pszHost, "encoder1"))
    {
         if( !strcasecmp(msg_context->pszCmd, "start"))
        {
            if( ++g_dwStreamingNum == 1)
                VMF_VENC_Start(g_ptVencHandle);
        }
        else if ( !strcasecmp(msg_context->pszCmd, "stop"))
        {
            if(g_dwStreamingNum)
            {
                if(--g_dwStreamingNum == 0)
                    VMF_VENC_Stop(g_ptVencHandle);
            }
        }
        else if ( !strcasecmp(msg_context->pszCmd, "forceCI"))
        {
            VMF_VENC_ProduceStreamHdr(g_ptVencHandle);
        }
        else if (!strcasecmp(msg_context->pszCmd, "forceIntra"))
        {
            VMF_VENC_SetIntra(g_ptVencHandle);        
        }
    }
    else if( !strcasecmp(msg_context->pszHost, SR_MODULE_NAME))
    {
        if( !strcasecmp(msg_context->pszCmd, SUSPEND_CMD))          // !Suspend
        {
            VMF_VSRC_Suspend(g_ptVsrcHandle);

            VMF_VENC_Suspend(g_ptVencHandle);
            VMF_VENC_Suspend(g_ptVencRsHandle);
            MsgBroker_SuspendAckMsg();
        }

        else if( !strcasecmp(msg_context->pszCmd, RESUME_CMD))      // !Resume
        {
            VMF_VSRC_Resume(g_ptVsrcHandle);

            VMF_VENC_Resume(g_ptVencHandle);
            VMF_VENC_Resume(g_ptVencRsHandle);
        }
    }

    if (msg_context->bHasResponse)
        msg_context->dwDataSize = 0;
}


static void sig_kill(int signo)
{
	print_msg("[%s] receive SIGNAL: %d\n",__func__, signo);
	g_iTerminate = 1;
}


int main(int argc, char *argv[])
{
    int ch;
    VMF_VENC_CODEC_TYPE eCodecType = VMF_VENC_CODEC_TYPE_H264;
    unsigned int FrameCount = 0;

    while((ch = getopt(argc, argv, "c:C:a:f:m")) != -1) {
        switch (ch) {
            case 'c' :
                g_ptszSensorConfig = strdup(optarg);
                printf("Get Sensor Config\n");
            break;

            case 'C' :
                eCodecType = (VMF_VENC_CODEC_TYPE) atoi(optarg);
                break;
            
            case 'a' :
			    g_ptszAutoSceneConfig = strdup(optarg);
                printf("Get Auto Scene Config\n");
		        break;
				
		    case 'f' :
			    g_iProcIfpOnly = atoi(optarg);
		    break;

		    case 'm' :
			    g_eFecMethod = atoi(optarg);
		    break;
				
		    default:
			    print_msg("Usage: %s [-c<sensor_config_file>] [-C<codec_type>] [-a autosecne_config_file] [-f bIfpOnly] [-m FecMethod]\r\n", argv[0]);
			    exit(EXIT_FAILURE);
			    break;
        }
    }

    if (!g_ptszSensorConfig) {
        print_msg("[%s] Err: no sensor config\n", __func__);
		goto FAILURE;
    }

    signal(SIGTERM, sig_kill);
    signal(SIGINT, sig_kill);

	if (init_video_source(eCodecType)) {
		goto FAILURE;
	}

    if (init_bind()) {
		goto FAILURE;
	}

    if (init_output_srb(VENC_OUTPUT_PIN,  VENC_OUTPUT_BUF_NUM, VENC_ENCODE_BUF_SIZE, &g_ptVencOutputSRB)) {
		goto FAILURE;
	}
    
    if (!g_iProcIfpOnly) {
		/* initialize the SRB for resize encoder output buffer */
		if (init_output_srb(VENC_OUTPUT_PIN_RS,  VENC_OUTPUT_BUF_NUM, VENC_ENCODE_BUF_SIZE_RS, &g_ptVencOutputRsSRB)) {
			goto FAILURE;
		}
	}


    if ((g_ptVencHandle = init_video_encoder(eCodecType, g_tLayout.dwVideoWidth, 
		        g_tLayout.dwVideoHeight, 4000000,g_ptVencOutputSRB)) == NULL) {
		VMF_VENC_OUT_SRB_Release(&g_ptVencOutputSRB);
		goto FAILURE;
	}

    FrameCount = g_ptVencOutputSRB->dwFrameCount;
    printf("FrameCount = %d\n",FrameCount);

    MsgBroker_RegisterMsg(VENC_CMD_FIFO);
	MsgBroker_Run(VENC_CMD_FIFO, msg_callback, NULL, &g_iTerminate);
	MsgBroker_UnRegisterMsg();

	
    release_video_encoder(g_ptVencHandle, &g_ptVencOutputSRB);
    if (!g_iProcIfpOnly) {
		release_video_encoder(g_ptVencRsHandle, &g_ptVencOutputRsSRB);
	}

    release_bind();
	release_video_source();

    free(g_ptszAutoSceneConfig);
	free(g_ptszSensorConfig);
	print_msg("terminated successfully!\n");

    return 0;

    FAILURE:
	    free(g_ptszAutoSceneConfig);
	    free(g_ptszSensorConfig);
	    print_msg("terminated with error!\n");

    return -1;
}