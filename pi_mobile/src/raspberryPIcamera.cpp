/*
 * raspberryPIcamera.cpp
 *
 *  Created on: Feb 1, 2018
 *      Author: markhsieh
 */

#include "raspberryPIcamera.h"
#include "heatfinderconfigmanager.h"
#include "heatfinderlogmanager.h"

/// general
#include <string.h>
#include <memory.h>
#include <sysexits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;

/// base on raspiXXX version
#define VERSION_STRING "v1.3.12"

// Standard port setting for the camera component
#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2

// Port configuration for the splitter component
#define SPLITTER_OUTPUT_PORT 0
#define SPLITTER_PREVIEW_PORT 1

// Video format information
// 0 implies variable
#define VIDEO_FRAME_RATE_NUM 30
#define VIDEO_FRAME_RATE_DEN 1

/// Video render needs at least 2 buffers.
#define VIDEO_OUTPUT_BUFFERS_NUM 3

// Max bitrate we allow for recording
const int MAX_BITRATE_MJPEG = 25000000; // 25Mbits/s
const int MAX_BITRATE_LEVEL4 = 25000000; // 25Mbits/s
const int MAX_BITRATE_LEVEL42 = 62500000; // 62.5Mbits/s

/// Interval at which we check for an failure abort during capture
const int ABORT_INTERVAL = 100; // ms

/// Command ID's and Structure defining our command line options
#define CommandHelp         0
#define CommandWidth        1
#define CommandHeight       2
#define CommandBitrate      3
#define CommandOutput       4
#define CommandVerbose      5
#define CommandTimeout      6
#define CommandDemoMode     7
#define CommandFramerate    8
#define CommandPreviewEnc   9
#define CommandIntraPeriod  10
#define CommandProfile      11
#define CommandTimed        12
#define CommandSignal       13
#define CommandKeypress     14
#define CommandInitialState 15
#define CommandQP           16
#define CommandInlineHeaders 17
#define CommandSegmentFile  18
#define CommandSegmentWrap  19
#define CommandSegmentStart 20
#define CommandSplitWait    21
#define CommandCircular     22
#define CommandIMV          23
#define CommandCamSelect    24
#define CommandSettings     25
#define CommandSensorMode   26
#define CommandIntraRefreshType 27
#define CommandFlush        28
#define CommandSavePTS      29
#define CommandCodec        30
#define CommandLevel        31
#define CommandRaw          32
#define CommandRawFormat    33
#define CommandNetListen    34

static COMMAND_LIST cmdline_commands[] =
{
   { CommandHelp,          (char *)"-help",       (char *)"?",  (char *)"This help information", 0 },
   { CommandWidth,         (char *)"-width",      (char *)"w",  (char *)"Set image width <size>. Default 1920", 1 },
   { CommandHeight,        (char *)"-height",     (char *)"h",  (char *)"Set image height <size>. Default 1080", 1 },
   { CommandBitrate,       (char *)"-bitrate",    (char *)"b",  (char *)"Set bitrate. Use bits per second (e.g. 10MBits/s would be -b 10000000)", 1 },
   { CommandOutput,        (char *)"-output",     (char *)"o",  (char *)"Output filename <filename> (to write to stdout, use '-o -').\n"
         "\t\t  Connect to a remote IPv4 host (e.g. tcp://192.168.1.2:1234, udp://192.168.1.2:1234)\n"
         "\t\t  To listen on a TCP port (IPv4) and wait for an incoming connection use -l\n"
         "\t\t  (e.g. raspivid -l -o tcp://0.0.0.0:3333 -> bind to all network interfaces, raspivid -l -o tcp://192.168.1.1:3333 -> bind to a certain local IPv4)", 1 },
   { CommandVerbose,       (char *)"-verbose",    (char *)"v",  (char *)"Output verbose information during run", 0 },
   { CommandTimeout,       (char *)"-timeout",    (char *)"t",  (char *)"Time (in ms) to capture for. If not specified, set to 5s. Zero to disable", 1 },
   { CommandDemoMode,      (char *)"-demo",       (char *)"d",  (char *)"Run a demo mode (cycle through range of camera options, no capture)", 1},
   { CommandFramerate,     (char *)"-framerate",  (char *)"fps",(char *)"Specify the frames per second to record", 1},
   { CommandPreviewEnc,    (char *)"-penc",       (char *)"e",  (char *)"Display preview image *after* encoding (shows compression artifacts)", 0},
   { CommandIntraPeriod,   (char *)"-intra",      (char *)"g",  (char *)"Specify the intra refresh period (key frame rate/GoP size). Zero to produce an initial I-frame and then just P-frames.", 1},
   { CommandProfile,       (char *)"-profile",    (char *)"pf", (char *)"Specify H264 profile to use for encoding", 1},
   { CommandTimed,         (char *)"-timed",      (char *)"td", (char *)"Cycle between capture and pause. -cycle on,off where on is record time and off is pause time in ms", 0},
   { CommandSignal,        (char *)"-signal",     (char *)"s",  (char *)"Cycle between capture and pause on Signal", 0},
   { CommandKeypress,      (char *)"-keypress",   (char *)"k",  (char *)"Cycle between capture and pause on ENTER", 0},
   { CommandInitialState,  (char *)"-initial",    (char *)"i",  (char *)"Initial state. Use 'record' or 'pause'. Default 'record'", 1},
   { CommandQP,            (char *)"-qp",         (char *)"qp", (char *)"Quantisation parameter. Use approximately 10-40. Default 0 (off)", 1},
   { CommandInlineHeaders, (char *)"-inline",     (char *)"ih", (char *)"Insert inline headers (SPS, PPS) to stream", 0},
   { CommandSegmentFile,   (char *)"-segment",    (char *)"sg", (char *)"Segment output file in to multiple files at specified interval <ms>", 1},
   { CommandSegmentWrap,   (char *)"-wrap",       (char *)"wr", (char *)"In segment mode, wrap any numbered filename back to 1 when reach number", 1},
   { CommandSegmentStart,  (char *)"-start",      (char *)"sn", (char *)"In segment mode, start with specified segment number", 1},
   { CommandSplitWait,     (char *)"-split",      (char *)"sp", (char *)"In wait mode, create new output file for each start event", 0},
   { CommandCircular,      (char *)"-circular",   (char *)"c",  (char *)"Run encoded data through circular buffer until triggered then save", 0},
   { CommandIMV,           (char *)"-vectors",    (char *)"x",  (char *)"Output filename <filename> for inline motion vectors", 1 },
   { CommandCamSelect,     (char *)"-camselect",  (char *)"cs", (char *)"Select camera <number>. Default 0", 1 },
   { CommandSettings,      (char *)"-settings",   (char *)"set",(char *)"Retrieve camera settings and write to stdout", 0},
   { CommandSensorMode,    (char *)"-mode",       (char *)"md", (char *)"Force sensor mode. 0=auto. See docs for other modes available", 1},
   { CommandIntraRefreshType, (char *)"-irefresh", (char *)"if", (char *)"Set intra refresh type", 1},
   { CommandFlush,         (char *)"-flush",      (char *)"fl",  (char *)"Flush buffers in order to decrease latency", 0 },
   { CommandSavePTS,       (char *)"-save-pts",   (char *)"pts",(char *)"Save Timestamps to file for mkvmerge", 1 },
   { CommandCodec,         (char *)"-codec",      (char *)"cd", (char *)"Specify the codec to use - H264 (default) or MJPEG", 1 },
   { CommandLevel,         (char *)"-level",      (char *)"lev",(char *)"Specify H264 level to use for encoding", 1},
   { CommandRaw,           (char *)"-raw",        (char *)"r",  (char *)"Output filename <filename> for raw video", 1 },
   { CommandRawFormat,     (char *)"-raw-format", (char *)"rf", (char *)"Specify output format for raw video. Default is yuv", 1},
   { CommandNetListen,     (char *)"-listen",     (char *)"l", (char *)"Listen on a TCP socket", 0},
};
static int cmdline_commands_size = sizeof(cmdline_commands) / sizeof(cmdline_commands[0]);

/**
 * Fast group keyword search :
 * 1. Create & Destroy
 * 2. Default, Initial, Checking methods
 * 3. Error Dump or Display
 * 4. Call Back Function
 * 5. Tools
 * 6. Operations
 * 7. Open/ Destroy Camera
 *
 */

//gavin ++ >>
unsigned char raspberryPIcamera::s_frameBuff[DEF_FRAME_BUFFER_SIZE] = {0};
int raspberryPIcamera::s_frameLen = 0;;
CFpsHelper raspberryPIcamera::s_fpsH264EncodedFrame;
//gavin ++ <<

/// ------------------ Create & Destroy ------------------------------
raspberryPIcamera* raspberryPIcamera::m_pSelf = NULL;
raspberryPIcamera::raspberryPIcamera()
{
    camera_preview_port = NULL;
    camera_video_port = NULL;
    camera_still_port = NULL;
    preview_input_port = NULL;
    encoder_input_port = NULL;
    encoder_output_port = NULL;
    splitter_input_port = NULL;
    splitter_output_port = NULL;
    splitter_preview_port = NULL;

    bIsNeedRestart = false;
    m_pSelf = this;
    bcm_host_init();

    default_status(&state);
    bIsOpen = false;

    bIsPreview = true;
    m_hflip = 0;
    m_vflip = 0;
}

raspberryPIcamera::~raspberryPIcamera()
{
    bcm_host_deinit();
}

/// ----------------- Default, Initial, Checking methods -------------------
bool raspberryPIcamera::IsRunning(){
	return bIsOpen;
}

/**
 * Assign a default set of parameters to the state passed in
 *
 * @param state Pointer to state structure to assign defaults to
 */
void raspberryPIcamera::default_status(RASPIVID_STATE *state)
{
   if (!state)
   {
      vcos_assert(0);
      return;
   }

   // Default everything to zero
   memset(state, 0, sizeof(RASPIVID_STATE));

   // Now set anything non-zero
   state->timeout = 5000;     // 5s delay before take image
   state->width = 1920;       // Default to 1080p
   state->height = 1080;
   state->encoding = MMAL_ENCODING_H264;
   state->bitrate = 17000000; // This is a decent default bitrate for 1080p
   state->framerate = VIDEO_FRAME_RATE_NUM;
   state->intraperiod = -1;    // Not set
   state->quantisationParameter = 0;
   state->demoMode = 0;
   state->demoInterval = 250; // ms
   state->immutableInput = 1;
   state->profile = MMAL_VIDEO_PROFILE_H264_HIGH;
   state->level = MMAL_VIDEO_LEVEL_H264_4;
   state->waitMethod = WAIT_METHOD_NONE;
   state->onTime = 5000;
   state->offTime = 5000;

   state->bCapturing = 0;
   state->bInlineHeaders = 0;

   state->segmentSize = 0;  // 0 = not segmenting the file.
   state->segmentNumber = 1;
   state->segmentWrap = 0; // Point at which to wrap segment number back to 1. 0 = no wrap
   state->splitNow = 0;
   state->splitWait = 0;

   state->inlineMotionVectors = 0;
   state->cameraNum = 0;
   state->settings = 0;
   state->sensor_mode = 0;

   state->intra_refresh_type = -1;

   state->frame = 0;
   state->save_pts = 0;

   state->netListen = false;

   // Setup preview window defaults
   raspipreview_set_defaults(&state->preview_parameters);

   // Set up the camera_parameters to default
   raspicamcontrol_set_defaults(&state->camera_parameters);
}

int raspberryPIcamera::mmal_status_to_int(MMAL_STATUS_T status)
{
    if (status == MMAL_SUCCESS)
       return 0;
    else
    {
       GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [RPI Camera] ");
       switch (status)
       {
       case MMAL_ENOMEM :   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "Out of memory"); break;
       case MMAL_ENOSPC :   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "Out of resources (other than memory)"); break;
       case MMAL_EINVAL:    GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "Argument is invalid"); break;
       case MMAL_ENOSYS :   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "Function not implemented"); break;
       case MMAL_ENOENT :   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "No such file or directory"); break;
       case MMAL_ENXIO :    GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "No such device or address"); break;
       case MMAL_EIO :      GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "I/O error"); break;
       case MMAL_ESPIPE :   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "Illegal seek"); break;
       case MMAL_ECORRUPT : GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "Data is corrupt \attention FIXME: not POSIX"); break;
       case MMAL_ENOTREADY :GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "Component is not ready \attention FIXME: not POSIX"); break;
       case MMAL_ECONFIG :  GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "Component is not configured \attention FIXME: not POSIX"); break;
       case MMAL_EISCONN :  GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "Port is already connected "); break;
       case MMAL_ENOTCONN : GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "Port is disconnected"); break;
       case MMAL_EAGAIN :   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "Resource temporarily unavailable. Try again later"); break;
       case MMAL_EFAULT :   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "Bad address"); break;
       default :            GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "Unknown status error"); break;
       }
       GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "\n");
       return 1;
    }
}

void raspberryPIcamera::check_camera_model(int cam_num)
{
   //MMAL_COMPONENT_T *camera_info;

   // Valgrind say fix: Conditional jump or move depends on uninitialised value(s)
   MMAL_COMPONENT_T *camera_info = 0;

   MMAL_STATUS_T status;

   // Valgrind say fix: Conditional jump or move depends on uninitialised value(s)
   memset(&status, 0, sizeof(status));

   // Try to get the camera name
   status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA_INFO, &camera_info);
   if (status == MMAL_SUCCESS)
   {
      MMAL_PARAMETER_CAMERA_INFO_T param;

      // Valgrind say fix: Conditional jump or move depends on uninitialised value(s)
      memset(&param, 0, sizeof(param));

      param.hdr.id = MMAL_PARAMETER_CAMERA_INFO;
      param.hdr.size = sizeof(param)-4;  // Deliberately undersize to check firmware version
      status = mmal_port_parameter_get(camera_info->control, &param.hdr);

      if (status != MMAL_SUCCESS)
      {
         // Running on newer firmware
         param.hdr.size = sizeof(param);
         status = mmal_port_parameter_get(camera_info->control, &param.hdr);
         if (status == MMAL_SUCCESS && ((int)param.num_cameras > cam_num))
         {
            if (!strncmp(param.cameras[cam_num].camera_name, "toshh2c", 7))
            {
               GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E:The driver for the TC358743 HDMI to CSI2 chip you are using is NOT supported.\n");
               GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E:They were written for a demo purposes only, and are in the firmware on an as-is\n");
               GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E:basis and therefore requests for support or changes will not be acted on.\n\n");
            }
         }
      }

      mmal_component_destroy(camera_info);
   }
}

/**
 * Parse the incoming command line and put resulting parameters in to the state
 *
 * @param argc Number of arguments in command line
 * @param argv Array of pointers to strings from command line
 * @param state Pointer to state structure to assign any discovered parameters to
 * @return Non-0 if failed for some reason, 0 otherwise
 */
int raspberryPIcamera::parse_cmdline(int argc, const char **argv, RASPIVID_STATE *state)
{
   // Parse the command line arguments.
   // We are looking for --<something> or -<abbreviation of something>

   int valid = 1;
   int i =0;

   for (i = 1; i < argc && valid; i++)
   {
      int command_id, num_parameters;

      if (!argv[i])
         continue;

      if (argv[i][0] != '-')
      {
         valid = 0;
         continue;
      }

      // Assume parameter is valid until proven otherwise
      valid = 1;

      command_id = raspicli_get_command_id(cmdline_commands, cmdline_commands_size, &argv[i][1], &num_parameters);

      // If we found a command but are missing a parameter, continue (and we will drop out of the loop)
      if (command_id != -1 && num_parameters > 0 && (i + 1 >= argc) )
         continue;

      //  We are now dealing with a command line option
      switch (command_id)
      {
      case CommandHelp:
         display_valid_parameters((char*)basename(argv[0]));
         return -1;

      case CommandWidth: // Width > 0
         if (sscanf(argv[i + 1], "%u", &state->width) != 1)
            valid = 0;
         else
            i++;
         break;

      case CommandHeight: // Height > 0
         if (sscanf(argv[i + 1], "%u", &state->height) != 1)
            valid = 0;
         else
            i++;
         break;

      case CommandBitrate: // 1-100
         if (sscanf(argv[i + 1], "%u", &state->bitrate) == 1)
         {
            i++;
         }
         else
            valid = 0;

         break;

      case CommandOutput:  // output filename
      {
         int len = strlen(argv[i + 1]);
         if (len)
         {
            state->filename = (char*)malloc(len + 1);
            vcos_assert(state->filename);
            if (state->filename)
               strncpy(state->filename, argv[i + 1], len+1);
            i++;
         }
         else
            valid = 0;
         break;
      }

      case CommandVerbose: // display lots of data during run
         state->verbose = 1;
         break;

      case CommandTimeout: // Time to run viewfinder/capture
      {
         if (sscanf(argv[i + 1], "%u", &state->timeout) == 1)
         {
            // Ensure that if previously selected a waitMethod we don't overwrite it
            if (state->timeout == 0 && state->waitMethod == WAIT_METHOD_NONE)
               state->waitMethod = WAIT_METHOD_FOREVER;

            i++;
         }
         else
            valid = 0;
         break;
      }

      case CommandDemoMode: // Run in demo mode - no capture
      {
         // Demo mode might have a timing parameter
         // so check if a) we have another parameter, b) its not the start of the next option
         if (i + 1 < argc  && argv[i+1][0] != '-')
         {
            if (sscanf(argv[i + 1], "%u", &state->demoInterval) == 1)
            {
               // TODO : What limits do we need for timeout?
               if (state->demoInterval == 0)
                  state->demoInterval = 250; // ms

               state->demoMode = 1;
               i++;
            }
            else
               valid = 0;
         }
         else
         {
            state->demoMode = 1;
         }

         break;
      }

      case CommandFramerate: // fps to record
      {
         if (sscanf(argv[i + 1], "%u", &state->framerate) == 1)
         {
            // TODO : What limits do we need for fps 1 - 30 - 120??
            i++;
         }
         else
            valid = 0;
         break;
      }

      case CommandPreviewEnc:
         state->immutableInput = 0;
         break;

      case CommandIntraPeriod: // key frame rate
      {
         if (sscanf(argv[i + 1], "%u", &state->intraperiod) == 1)
            i++;
         else
            valid = 0;
         break;
      }

      case CommandQP: // quantisation parameter
      {
         if (sscanf(argv[i + 1], "%u", &state->quantisationParameter) == 1)
            i++;
         else
            valid = 0;
         break;
      }

      case CommandProfile: // H264 profile
      {
         state->profile = raspicli_map_xref(argv[i + 1], profile_map, profile_map_size);

         if( state->profile == -1)
            state->profile = MMAL_VIDEO_PROFILE_H264_HIGH;

         i++;
         break;
      }

      case CommandInlineHeaders: // H264 inline headers
      {
         state->bInlineHeaders = 1;
         break;
      }

      case CommandTimed:
      {
         if (sscanf(argv[i + 1], "%u,%u", &state->onTime, &state->offTime) == 2)
         {
            i++;

            if (state->onTime < 1000)
               state->onTime = 1000;

            if (state->offTime < 1000)
               state->offTime = 1000;

            state->waitMethod = WAIT_METHOD_TIMED;
         }
         else
            valid = 0;
         break;
      }

      case CommandKeypress:
         state->waitMethod = WAIT_METHOD_KEYPRESS;
         break;

      case CommandSignal:
         state->waitMethod = WAIT_METHOD_SIGNAL;
         // Reenable the signal
         //signal(SIGUSR1, signal_handler);
         break;

      case CommandInitialState:
      {
         state->bCapturing = raspicli_map_xref(argv[i + 1], initial_map, initial_map_size);

         if( state->bCapturing == -1)
            state->bCapturing = 0;

         i++;
         break;
      }

      case CommandSegmentFile: // Segment file in to chunks of specified time
      {
         if (sscanf(argv[i + 1], "%u", &state->segmentSize) == 1)
         {
            // Must enable inline headers for this to work
            state->bInlineHeaders = 1;
            i++;
         }
         else
            valid = 0;
         break;
      }

      case CommandSegmentWrap: // segment wrap value
      {
         if (sscanf(argv[i + 1], "%u", &state->segmentWrap) == 1)
            i++;
         else
            valid = 0;
         break;
      }

      case CommandSegmentStart: // initial segment number
      {
         if((sscanf(argv[i + 1], "%u", &state->segmentNumber) == 1) && (!state->segmentWrap || (state->segmentNumber <= state->segmentWrap)))
            i++;
         else
            valid = 0;
         break;
      }

      case CommandSplitWait: // split files on restart
      {
         // Must enable inline headers for this to work
         state->bInlineHeaders = 1;
         state->splitWait = 1;
         break;
      }

      case CommandCircular:
      {
         state->bCircularBuffer = 1;
         break;
      }

      case CommandIMV:  // output filename
      {
         state->inlineMotionVectors = 1;
         int len = strlen(argv[i + 1]);
         if (len)
         {
            state->imv_filename = (char*)malloc(len + 1);
            vcos_assert(state->imv_filename);
            if (state->imv_filename)
               strncpy(state->imv_filename, argv[i + 1], len+1);
            i++;
         }
         else
            valid = 0;
         break;
      }
      case CommandCamSelect:  //Select camera input port
      {
         if (sscanf(argv[i + 1], "%u", &state->cameraNum) == 1)
         {
            i++;
         }
         else
            valid = 0;
         break;
      }

      case CommandSettings:
         state->settings = 1;
         break;

      case CommandSensorMode:
      {
         if (sscanf(argv[i + 1], "%u", &state->sensor_mode) == 1)
         {
            i++;
         }
         else
            valid = 0;
         break;
      }

      case CommandIntraRefreshType:
      {
         state->intra_refresh_type = raspicli_map_xref(argv[i + 1], intra_refresh_map, intra_refresh_map_size);
         i++;
         break;
      }

      case CommandFlush:
      {
         state->callback_data.flush_buffers = 1;
         break;
      }
      case CommandSavePTS:  // output filename
      {
         state->save_pts = 1;
         int len = strlen(argv[i + 1]);
         if (len)
         {
            state->pts_filename = (char*)malloc(len + 1);
            vcos_assert(state->pts_filename);
            if (state->pts_filename)
               strncpy(state->pts_filename, argv[i + 1], len+1);
            i++;
         }
         else
            valid = 0;
         break;
      }
      case CommandCodec:  // codec type
      {
         int len = strlen(argv[i + 1]);
         if (len)
         {
            if (len==4 && !strncmp("H264", argv[i+1], 4))
               state->encoding = MMAL_ENCODING_H264;
            else  if (len==5 && !strncmp("MJPEG", argv[i+1], 5))
               state->encoding = MMAL_ENCODING_MJPEG;
            else
               valid = 0;
            i++;
         }
         else
            valid = 0;
         break;
      }

      case CommandLevel: // H264 level
      {
         state->level = raspicli_map_xref(argv[i + 1], level_map, level_map_size);

         if( state->level == -1)
            state->level = MMAL_VIDEO_LEVEL_H264_4;

         i++;
         break;
      }

      case CommandRaw:  // output filename
      {
         state->raw_output = 1;
         state->raw_output_fmt = RAW_OUTPUT_FMT_YUV;
         int len = strlen(argv[i + 1]);
         if (len)
         {
            state->raw_filename = (char*)malloc(len + 1);
            vcos_assert(state->raw_filename);
            if (state->raw_filename)
               strncpy(state->raw_filename, argv[i + 1], len+1);
            i++;
         }
         else
            valid = 0;
         break;
      }

      case CommandRawFormat:
      {
         state->raw_output_fmt = (RAW_OUTPUT_FMT)raspicli_map_xref(argv[i + 1], raw_output_fmt_map, raw_output_fmt_map_size);

         if (state->raw_output_fmt == -1)
            valid = 0;

         i++;
         break;
      }

      case CommandNetListen:
      {
         state->netListen = true;

         break;
      }

      default:
      {
         // Try parsing for any image specific parameters
         // result indicates how many parameters were used up, 0,1,2
         // but we adjust by -1 as we have used one already
         const char *second_arg = (i + 1 < argc) ? argv[i + 1] : NULL;
         int parms_used = (raspicamcontrol_parse_cmdline(&state->camera_parameters, &argv[i][1], second_arg));

         // Still unused, try preview options
         if (!parms_used)
            parms_used = raspipreview_parse_cmdline(&state->preview_parameters, &argv[i][1], second_arg);


         // If no parms were used, this must be a bad parameters
         if (!parms_used)
            valid = 0;
         else
            i += parms_used - 1;

         break;
      }
      }
   }

   if (!valid)
   {
      GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E:Invalid command line option (%s)\n", argv[i-1]);
      return 1;
   }

   // Always disable verbose if output going to stdout
   if (state->filename && state->filename[0] == '-')
   {
      state->verbose = 0;
   }

   return 0;
}

/// ------------------ Error Dump or Display ------------------------
/**
 * Dump image state parameters to stderr.
 *
 * @param state Pointer to state structure to assign defaults to
 */
void raspberryPIcamera::dump_status(RASPIVID_STATE *state)
{
   int i =0;

   if (!state)
   {
      vcos_assert(0);
      return;
   }

   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W:Width %d, Height %d, filename %s\n", state->width, state->height, state->filename);
   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W:bitrate %d, framerate %d, time delay %d\n", state->bitrate, state->framerate, state->timeout);
   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W:H264 Profile %s\n", raspicli_unmap_xref(state->profile, profile_map, profile_map_size));
   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W:H264 Level %s\n", raspicli_unmap_xref(state->level, level_map, level_map_size));
   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W:H264 Quantisation level %d, Inline headers %s\n", state->quantisationParameter, state->bInlineHeaders ? "Yes" : "No");
   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W:H264 Intra refresh type %s, period %d\n", raspicli_unmap_xref(state->intra_refresh_type, intra_refresh_map, intra_refresh_map_size), state->intraperiod);

   // Not going to display segment data unless asked for it.
   if (state->segmentSize)
      GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W:Segment size %d, segment wrap value %d, initial segment number %d\n", state->segmentSize, state->segmentWrap, state->segmentNumber);

   if (state->raw_output)
      GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W:Raw output enabled, format %s\n", raspicli_unmap_xref(state->raw_output_fmt, raw_output_fmt_map, raw_output_fmt_map_size));

   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W:Wait method : ");
   for (i=0;i<wait_method_description_size;i++)
   {
      if (state->waitMethod == wait_method_description[i].nextWaitMethod)
         GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W:%s", wait_method_description[i].description);
   }
   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: Initial state '%s'\n\n\n", raspicli_unmap_xref(state->bCapturing, initial_map, initial_map_size));

   raspipreview_dump_parameters(&state->preview_parameters);
   raspicamcontrol_dump_parameters(&state->camera_parameters);
}

/// ---------------------- Call Back Function -----------------------
/**
 *  buffer header callback function for camera control
 *
 *  Callback will dump buffer data to the specific file
 *
 * @param port Pointer to port from which callback originated
 * @param buffer mmal buffer header pointer
 */
void raspberryPIcamera::camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
   if (buffer->cmd == MMAL_EVENT_PARAMETER_CHANGED)
   {
      MMAL_EVENT_PARAMETER_CHANGED_T *param = (MMAL_EVENT_PARAMETER_CHANGED_T *)buffer->data;
      switch (param->hdr.id) {
         case MMAL_PARAMETER_CAMERA_SETTINGS:
         {
            MMAL_PARAMETER_CAMERA_SETTINGS_T *settings = (MMAL_PARAMETER_CAMERA_SETTINGS_T*)param;
            vcos_log_error("Exposure now %u, analog gain %u/%u, digital gain %u/%u",
            settings->exposure,
                        settings->analog_gain.num, settings->analog_gain.den,
                        settings->digital_gain.num, settings->digital_gain.den);
            vcos_log_error("AWB R=%u/%u, B=%u/%u",
                        settings->awb_red_gain.num, settings->awb_red_gain.den,
                        settings->awb_blue_gain.num, settings->awb_blue_gain.den
                        );
         }
         break;
      }
   }
   else if (buffer->cmd == MMAL_EVENT_ERROR)
   {
      vcos_log_error("No data received from sensor. Check all connections, including the Sunny one on the camera board");
   }
   else
   {
      vcos_log_error("Received unexpected camera control callback event, 0x%08x", buffer->cmd);
   }

   mmal_buffer_header_release(buffer);
}

//gavin ++ >>
void raspberryPIcamera::PushFrameBuffToFifo(unsigned char* pBuff, int nLength)
{
	axc_dword fps;
	if(s_fpsH264EncodedFrame.Progress(fps))
	{
		GLog(0, tDEBUGTrace_MSK, "Gavin: h264Frame FPS = %d\n", fps );
	}

	GLog( tRingBuffer, tDEBUGTrace_MSK, "Gavin: callback len:%.1fK\n", (float)(nLength)/1024.0 );
	for(unsigned int i = 0; i < m_pSelf->m_H264ReceivedEventList.size(); ++i)
	{
		xH264ReceivedEventHandler handler = m_pSelf->m_H264ReceivedEventList[i];
		handler.fnEvent(handler.pContext, nLength, pBuff);
	}
}
//gavin ++ <<

/**
 * @brief Already rework
 *  buffer header callback function for encoder
 *
 *  Callback will dump buffer data to the specific file
 *
 * @param port Pointer to port from which callback originated
 * @param buffer mmal buffer header pointer
 */
void raspberryPIcamera::encoder_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
   MMAL_BUFFER_HEADER_T *new_buffer;
   static int64_t base_time =  -1;
   static int64_t last_second = -1;

   //printf("%s: 1\n", __func__);
#if false
   uint32_t iBufferLength = 16*1024;
   uint8_t iReadBuffer[iBufferLength];
   uint32_t iReadedLength = 0;
   memset(iReadBuffer, 0, iBufferLength* sizeof(uint8_t));
#endif

   // All our segment times based on the receipt of the first encoder callback
   if (base_time == -1)
      base_time = vcos_getmicrosecs64()/1000;

   // We pass our file handle and other stuff in via the userdata field.

   PORT_USERDATA *pData = (PORT_USERDATA *)port->userdata;

   if (pData)
   {
    //gavin modify  >>
	   int64_t current_time = vcos_getmicrosecs64()/1000;

	 if (buffer->length)
	 {
         if (pData->cb_buff)
         {
            if(buffer->flags & MMAL_BUFFER_HEADER_FLAG_CONFIG)
			{
				if(buffer->length > sizeof(pData->header_bytes))
				{
					GLog( tAll, tERRORTrace_MSK, "E: Error in header bytes\n" );
				}
				else
				{
					// These are the header bytes, save them for final output
					mmal_buffer_header_mem_lock(buffer);
                    memcpy(pData->header_bytes, buffer->data, buffer->length);
					mmal_buffer_header_mem_unlock(buffer);
					pData->header_wptr = buffer->length;
					GLog( 0, tDEBUGTrace_MSK, "D: header %d bytes\n", pData->header_wptr );

                    PushFrameBuffToFifo((unsigned char*)pData->header_bytes, pData->header_wptr);
                    //dump_binary(pData->cb_buff, 5);
				}
			}
            else if((buffer->flags & MMAL_BUFFER_HEADER_FLAG_CODECSIDEINFO))
			{
				// Do something with the inline motion vectors...
			}
			else
			{
				static int s_seq_no = 0;
				static int s_indx = 0;
				static int frame_start = -1;
				int i;

				if(frame_start == -1) {
					frame_start = pData->cb_wptr;
					s_indx = 0;
				}

				if(buffer->flags & MMAL_BUFFER_HEADER_FLAG_FRAME_END) {
					frame_start = -1;
				} else {
					s_indx ++;
				}

                if(pData->cb_wptr + buffer->length >= pData->cb_len) {
                    GLog( tAll, tERRORTrace_MSK, "E: NOT enough buffer size!!!\n");
                } else {
                    mmal_buffer_header_mem_lock(buffer);
                    // We are pushing data into a circular buffer
                    memcpy(pData->cb_buff + pData->cb_wptr, buffer->data, buffer->length);
                    mmal_buffer_header_mem_unlock(buffer);

                    //dump_binary(pData->cb_buff + pData->cb_wptr, 5);

                    if((pData->cb_wptr + buffer->length) > pData->cb_len)
                       pData->cb_wrap = 1;
                    pData->cb_wptr = (pData->cb_wptr + buffer->length) % pData->cb_len;

                    if(frame_start == -1) {
                        PushFrameBuffToFifo((unsigned char*)pData->cb_buff, pData->cb_wptr);
                        //dump_binary(pData->cb_buff, 5);
                        if(buffer->flags & MMAL_BUFFER_HEADER_FLAG_KEYFRAME) {
                            GLog( 0, tDEBUGTrace_MSK, "D: I seq:%d part:%d %.1fk bytes\n", s_seq_no, s_indx, (float)pData->cb_wptr/1024.0 );
                            s_seq_no = 0;
                        } else {
                            GLog( 0, tDEBUGTrace_MSK, "D: P seq:%d part:%d %.1fk bytes\n", s_seq_no, s_indx, (float)pData->cb_wptr/1024.0 );
                            s_seq_no++;
                        }
                        pData->cb_wptr = 0;
                    }
                }
			}
         }
	 }
    //gavin modify <<


      // See if the second count has changed and we need to update any annotation
      if (current_time/1000 != last_second)
      {
         m_pSelf->update_annotation_data(pData->pstate);
         last_second = current_time/1000;
      }
   }
   else
   {
      vcos_log_error("Received a encoder buffer callback with no state");
   }

   // release buffer back to the pool
   mmal_buffer_header_release(buffer);

   // and send one back to the port (if still open)
   if (port->is_enabled)
   {
      MMAL_STATUS_T status;

      new_buffer = mmal_queue_get(pData->pstate->encoder_pool->queue);

      if (new_buffer)
         status = mmal_port_send_buffer(port, new_buffer);

      if (!new_buffer || status != MMAL_SUCCESS)
         vcos_log_error("Unable to return a buffer to the encoder port");
   }
}

/**
 *  buffer header callback function for splitter
 *
 *  Callback will dump buffer data to the specific file
 *
 * @param port Pointer to port from which callback originated
 * @param buffer mmal buffer header pointer
 */
void raspberryPIcamera::splitter_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
   MMAL_BUFFER_HEADER_T *new_buffer;
   PORT_USERDATA *pData = (PORT_USERDATA *)port->userdata;

   if (pData)
   {
      //int bytes_written = 0;
      int bytes_to_write = buffer->length;

      /* Write only luma component to get grayscale image: */
      if (buffer->length && pData->pstate->raw_output_fmt == RAW_OUTPUT_FMT_GRAY)
         bytes_to_write = port->format->es->video.width * port->format->es->video.height;

      //vcos_assert(pData->raw_file_handle);    //hamlet no file handle

      //must be RGB
      //if (pData->pstate->raw_output_fmt != RAW_OUTPUT_FMT_RGB){
    //	  //SetRawCapturingEable(true, (RAW_OUTPUT_FMT)2);
    //	  printf("E: [RPI Camera] %s camera not rgb format!!!!\n", __func__);
      //}

      if (bytes_to_write)
      {
         mmal_buffer_header_mem_lock(buffer);
         for(unsigned int i = 0; i < m_pSelf->m_RawReceivedEventList.size(); ++i)
         {
             xRawReceivedEventHandler handler = m_pSelf->m_RawReceivedEventList[i];
             handler.fnEvent(handler.pContext,
                             pData->pstate->width, pData->pstate->height,
                             pData->pstate->raw_output_fmt,
                             buffer->length, buffer->data);
         }
         //bytes_written  = bytes_to_write;
         //bytes_written = fwrite(buffer->data, 1, bytes_to_write, pData->raw_file_handle);
         mmal_buffer_header_mem_unlock(buffer);

         //if (bytes_written != bytes_to_write)
         //{
         //   vcos_log_error("Failed to write raw buffer data (%d from %d)- aborting", bytes_written, bytes_to_write);
         //   pData->abort = 1;
         //}
      }
   }
   else
   {
      vcos_log_error("Received a camera buffer callback with no state");
   }

   // release buffer back to the pool
   mmal_buffer_header_release(buffer);

   // and send one back to the port (if still open)
   if (port->is_enabled)
   {
      MMAL_STATUS_T status;

      new_buffer = mmal_queue_get(pData->pstate->splitter_pool->queue);

      if (new_buffer)
         status = mmal_port_send_buffer(port, new_buffer);

      if (!new_buffer || status != MMAL_SUCCESS)
         vcos_log_error("Unable to return a buffer to the splitter port");
   }
}

void raspberryPIcamera::AddH264ReceivedEvent(OnH264ReceivedEvent fnEvent, void *pContext)
{
    xH264ReceivedEventHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;

    m_H264ReceivedEventList.push_back(handler);
}

void raspberryPIcamera::AddRawReceivedEvent(OnRawReceivedEvent fnEvent, void *pContext)
{
    xRawReceivedEventHandler handler;
    handler.fnEvent = fnEvent;
    handler.pContext = pContext;

    m_RawReceivedEventList.push_back(handler);
}

/// ----------------------- Tools -----------------------------------
/**
 * Display usage information for the application to stdout
 *
 * @param app_name String to display as the application name
 */
void raspberryPIcamera::display_valid_parameters(char *app_name)
{
   int i=0;

   fprintf(stdout, "Display camera output to display, and optionally saves an H264 capture at requested bitrate\n\n");
   fprintf(stdout, "\nusage: %s [options]\n\n", app_name);

   fprintf(stdout, "Image parameter commands\n\n");

   raspicli_display_help(cmdline_commands, cmdline_commands_size);

   // Profile options
   fprintf(stdout, "\n\nH264 Profile options :\n%s", profile_map[0].mode );

   for (i=1;i<profile_map_size;i++)
   {
      fprintf(stdout, ",%s", profile_map[i].mode);
   }

   // Level options
   fprintf(stdout, "\n\nH264 Level options :\n%s", level_map[0].mode );

   for (i=1;i<level_map_size;i++)
   {
      fprintf(stdout, ",%s", level_map[i].mode);
   }

   // Intra refresh options
   fprintf(stdout, "\n\nH264 Intra refresh options :\n%s", intra_refresh_map[0].mode );

   for (i=1;i<intra_refresh_map_size;i++)
   {
      fprintf(stdout, ",%s", intra_refresh_map[i].mode);
   }

   // Raw output format options
   fprintf(stdout, "\n\nRaw output format options :\n%s", raw_output_fmt_map[0].mode );

   for (i=1;i<raw_output_fmt_map_size;i++)
   {
      fprintf(stdout, ",%s", raw_output_fmt_map[i].mode);
   }

   fprintf(stdout, "\n");

   // Help for preview options
   raspipreview_display_help();

   // Now display any help information from the camcontrol code
   raspicamcontrol_display_help();

   fprintf(stdout, "\n");

   return;
}

/**
 * Update any annotation data specific to the video.
 * This simply passes on the setting from cli, or
 * if application defined annotate requested, updates
 * with the H264 parameters
 *
 * @param state Pointer to state control struct
 *
 */
void raspberryPIcamera::update_annotation_data(RASPIVID_STATE *state)
{
   // So, if we have asked for a application supplied string, set it to the H264 parameters
   if (state->camera_parameters.enable_annotate & ANNOTATE_APP_TEXT)
   {
      char *text;
      const char *refresh = raspicli_unmap_xref(state->intra_refresh_type, intra_refresh_map, intra_refresh_map_size);

      asprintf(&text,  "%dk,%df,%s,%d,%s,%s",
            state->bitrate / 1000,  state->framerate,
            refresh ? refresh : "(none)",
            state->intraperiod,
            raspicli_unmap_xref(state->profile, profile_map, profile_map_size),
            raspicli_unmap_xref(state->level, level_map, level_map_size));

      raspicamcontrol_set_annotate(state->camera_component, state->camera_parameters.enable_annotate, text,
                       state->camera_parameters.annotate_text_size,
                       state->camera_parameters.annotate_text_colour,
                       state->camera_parameters.annotate_bg_colour);

      free(text);
   }
   else
   {
      raspicamcontrol_set_annotate(state->camera_component, state->camera_parameters.enable_annotate, state->camera_parameters.annotate_string,
                       state->camera_parameters.annotate_text_size,
                       state->camera_parameters.annotate_text_colour,
                       state->camera_parameters.annotate_bg_colour);
   }
}

/**
 * Connect two specific ports together
 *
 * @param output_port Pointer the output port
 * @param input_port Pointer the input port
 * @param Pointer to a mmal connection pointer, reassigned if function successful
 * @return Returns a MMAL_STATUS_T giving result of operation
 *
 */
MMAL_STATUS_T raspberryPIcamera::connect_ports(MMAL_PORT_T *output_port, MMAL_PORT_T *input_port, MMAL_CONNECTION_T **connection)
{
   MMAL_STATUS_T status;

   status =  mmal_connection_create(connection, output_port, input_port, MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT);

   if (status == MMAL_SUCCESS)
   {
      status =  mmal_connection_enable(*connection);
      if (status != MMAL_SUCCESS)
         mmal_connection_destroy(*connection);
   }

   return status;
}

/**
 * Checks if specified port is valid and enabled, then disables it
 *
 * @param port  Pointer the port
 *
 */
void raspberryPIcamera::check_disable_port(MMAL_PORT_T *port)
{
   if (port && port->is_enabled)
      mmal_port_disable(port);
}

/**
 * Pause for specified time, but return early if detect an abort request
 *
 * @param state Pointer to state control struct
 * @param pause Time in ms to pause
 * @param callback Struct contain an abort flag tested for early termination
 *
 */
int raspberryPIcamera::pause_and_test_abort(RASPIVID_STATE *state, int pause)
{
   int wait=0;

   if (!pause)
      return 0;

   // Going to check every ABORT_INTERVAL milliseconds
   for (wait = 0; wait < pause; wait+= ABORT_INTERVAL)
   {
      vcos_sleep(ABORT_INTERVAL);
      if (state->callback_data.abort)
         return 1;
   }

   return 0;
}

/**
 * Function to wait in various ways (depending on settings)
 *
 * @param state Pointer to the state data
 *
 * @return !0 if to continue, 0 if reached end of run
 */
int raspberryPIcamera::wait_for_next_change(RASPIVID_STATE *state)
{
   int keep_running = 1;
   static int64_t complete_time = -1;

   // Have we actually exceeded our timeout?
   int64_t current_time =  vcos_getmicrosecs64()/1000;

   if (complete_time == -1)
      complete_time =  current_time + state->timeout;

   // if we have run out of time, flag we need to exit
   if (current_time >= complete_time && state->timeout != 0)
      keep_running = 0;

   switch (state->waitMethod)
   {
   case WAIT_METHOD_NONE:
      (void)pause_and_test_abort(state, state->timeout);
      return 0;

   case WAIT_METHOD_FOREVER:
   {
      // We never return from this. Expect a ctrl-c to exit.
      while (1)
         // Have a sleep so we don't hog the CPU.
         vcos_sleep(10000);

      return 0;
   }

   case WAIT_METHOD_TIMED:
   {
      int abort;

      if (state->bCapturing)
         abort = pause_and_test_abort(state, state->onTime);
      else
         abort = pause_and_test_abort(state, state->offTime);

      if (abort)
         return 0;
      else
         return keep_running;
   }

   case WAIT_METHOD_KEYPRESS:
   {
      char ch;

      if (state->verbose)
         GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E:Press Enter to %s, X then ENTER to exit, [i,o,r] then ENTER to change zoom\n", state->bCapturing ? "pause" : "capture");

      ch = getchar();
      if (ch == 'x' || ch == 'X')
         return 0;
      else if (ch == 'i' || ch == 'I')
      {
         if (state->verbose)
            GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E:Starting zoom in\n");

         raspicamcontrol_zoom_in_zoom_out(state->camera_component, ZOOM_IN, &(state->camera_parameters).roi);

         if (state->verbose)
            dump_status(state);
      }
      else if (ch == 'o' || ch == 'O')
      {
         if (state->verbose)
            GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E:Starting zoom out\n");

         raspicamcontrol_zoom_in_zoom_out(state->camera_component, ZOOM_OUT, &(state->camera_parameters).roi);

         if (state->verbose)
            dump_status(state);
      }
      else if (ch == 'r' || ch == 'R')
      {
         if (state->verbose)
            GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E:starting reset zoom\n");

         raspicamcontrol_zoom_in_zoom_out(state->camera_component, ZOOM_RESET, &(state->camera_parameters).roi);

         if (state->verbose)
            dump_status(state);
      }

      return keep_running;
   }


   case WAIT_METHOD_SIGNAL:
   {
      // Need to wait for a SIGUSR1 signal
      sigset_t waitset;
      int sig;
      int result = 0;

      sigemptyset( &waitset );
      sigaddset( &waitset, SIGUSR1 );

      // We are multi threaded because we use mmal, so need to use the pthread
      // variant of procmask to block SIGUSR1 so we can wait on it.
      pthread_sigmask( SIG_BLOCK, &waitset, NULL );

      if (state->verbose)
      {
         GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E:Waiting for SIGUSR1 to %s\n", state->bCapturing ? "pause" : "capture");
      }

      result = sigwait( &waitset, &sig );

      if (state->verbose && result != 0)
         GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E:Bad signal received - error %d\n", errno);

      return keep_running;
   }

   } // switch

   return keep_running;
}

/// ----------------------- Operations ------------------------------
/**
 * Create the camera component, set up its ports
 *
 * @param state Pointer to state control struct
 *
 * @return MMAL_SUCCESS if all OK, something else otherwise
 *
 */
MMAL_STATUS_T raspberryPIcamera::create_camera_component(RASPIVID_STATE *state)
{
   MMAL_COMPONENT_T *camera = 0;
   //MMAL_ES_FORMAT_T *format;

   // Valgrind say fix: Conditional jump or move depends on uninitialised value(s)
   MMAL_ES_FORMAT_T *format = 0;

   MMAL_PORT_T *preview_port = NULL, *video_port = NULL, *still_port = NULL;
   MMAL_STATUS_T status;

   // Valgrind say fix: Conditional jump or move depends on uninitialised value(s)
   memset(&status, 0, sizeof(status));

   /* Create the component */
   status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("Failed to create camera component");
      goto error;
   }

   int istatus;
   istatus = 0;
   istatus = raspicamcontrol_set_stereo_mode(camera->output[0], &state->camera_parameters.stereo_mode);
   istatus += raspicamcontrol_set_stereo_mode(camera->output[1], &state->camera_parameters.stereo_mode);
   istatus += raspicamcontrol_set_stereo_mode(camera->output[2], &state->camera_parameters.stereo_mode);

   if (istatus != MMAL_SUCCESS)
   {
      vcos_log_error("Could not set stereo mode : error %d", status);
      goto error;
   }

   //hamlet
   //MMAL_PARAMETER_INT32_T camera_num =
   //   {{MMAL_PARAMETER_CAMERA_NUM, sizeof(camera_num)}, state->cameraNum};
   MMAL_PARAMETER_INT32_T camera_num;

   // Valgrind say fix: Conditional jump or move depends on uninitialised value(s)
   memset(&camera_num, 0, sizeof(camera_num));

   camera_num.hdr.id = MMAL_PARAMETER_CAMERA_NUM;
   camera_num.hdr.size = sizeof(camera_num);
   camera_num.value = state->cameraNum;

   status = mmal_port_parameter_set(camera->control, &camera_num.hdr);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("Could not select camera : error %d", status);
      goto error;
   }

   if (!camera->output_num)
   {
      status = MMAL_ENOSYS;
      vcos_log_error("Camera doesn't have output ports");
      goto error;
   }

   status = mmal_port_parameter_set_uint32(camera->control, MMAL_PARAMETER_CAMERA_CUSTOM_SENSOR_CONFIG, state->sensor_mode);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("Could not set sensor mode : error %d", status);
      goto error;
   }

   preview_port = camera->output[MMAL_CAMERA_PREVIEW_PORT];
   video_port = camera->output[MMAL_CAMERA_VIDEO_PORT];
   still_port = camera->output[MMAL_CAMERA_CAPTURE_PORT];

   if (state->settings)
   {
      MMAL_PARAMETER_CHANGE_EVENT_REQUEST_T change_event_request =
         {{MMAL_PARAMETER_CHANGE_EVENT_REQUEST, sizeof(MMAL_PARAMETER_CHANGE_EVENT_REQUEST_T)},
          MMAL_PARAMETER_CAMERA_SETTINGS, 1};

      status = mmal_port_parameter_set(camera->control, &change_event_request.hdr);
      if ( status != MMAL_SUCCESS )
      {
         vcos_log_error("No camera settings events");
      }
   }

   // Enable the camera, and tell it its control callback function
   status = mmal_port_enable(camera->control, camera_control_callback);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("Unable to enable control port : error %d", status);
      goto error;
   }

   //  set up the camera configuration
   {
	  MMAL_PARAMETER_CAMERA_CONFIG_T cam_config;
	  memset(&cam_config, 0, sizeof(cam_config));

      cam_config.hdr.id = MMAL_PARAMETER_CAMERA_CONFIG;
      cam_config.hdr.size = (uint32_t)sizeof(cam_config);
      cam_config.max_stills_w = (uint32_t) state->width;
      cam_config.max_stills_h = (uint32_t) state->height;
      cam_config.stills_yuv422 = 0;
      cam_config.one_shot_stills = 0;
      cam_config.max_preview_video_w = (uint32_t) state->width;
      cam_config.max_preview_video_h = (uint32_t) state->height;
      cam_config.num_preview_video_frames = (uint32_t)(3 + vcos_max(0, (state->framerate-30)/10));
      cam_config.stills_capture_circular_buffer_height = 0;
      cam_config.fast_preview_resume = 0;
      cam_config.use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RAW_STC;

      mmal_port_parameter_set(camera->control, &cam_config.hdr);
   }

   // Now set up the port formats

   // Set the encode format on the Preview port
   // HW limitations mean we need the preview to be the same size as the required recorded output

   format = preview_port->format;

   format->encoding = MMAL_ENCODING_OPAQUE;
   format->encoding_variant = MMAL_ENCODING_I420;

   if(state->camera_parameters.shutter_speed > 6000000)
   {
        MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)},
                                                     { 50, 1000 }, {166, 1000}};
        mmal_port_parameter_set(preview_port, &fps_range.hdr);
   }
   else if(state->camera_parameters.shutter_speed > 1000000)
   {
        MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)},
                                                     { 166, 1000 }, {999, 1000}};
        mmal_port_parameter_set(preview_port, &fps_range.hdr);
   }

   //enable dynamic framerate if necessary
   if (state->camera_parameters.shutter_speed)
   {
      if (state->framerate > 1000000./state->camera_parameters.shutter_speed)
      {
         state->framerate=0;
         if (state->verbose)
            GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E:Enable dynamic frame rate to fulfil shutter speed requirement\n");
      }
   }

   // for 'preview port'
   format->encoding = MMAL_ENCODING_OPAQUE;
   format->es->video.width = VCOS_ALIGN_UP(state->width, 32);
   format->es->video.height = VCOS_ALIGN_UP(state->height, 16);
   format->es->video.crop.x = 0;
   format->es->video.crop.y = 0;
   format->es->video.crop.width = state->width;
   format->es->video.crop.height = state->height;
   format->es->video.frame_rate.num = PREVIEW_FRAME_RATE_NUM;
   format->es->video.frame_rate.den = PREVIEW_FRAME_RATE_DEN;

   status = mmal_port_format_commit(preview_port);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("camera viewfinder format couldn't be set");
      goto error;
   }

   // Set the encode format on the video  port

   format = video_port->format;
   format->encoding_variant = MMAL_ENCODING_I420;

   if(state->camera_parameters.shutter_speed > 6000000)
   {
        MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)},
                                                     { 50, 1000 }, {166, 1000}};
        mmal_port_parameter_set(video_port, &fps_range.hdr);
   }
   else if(state->camera_parameters.shutter_speed > 1000000)
   {
        MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)},
                                                     { 167, 1000 }, {999, 1000}};
        mmal_port_parameter_set(video_port, &fps_range.hdr);
   }

   // for 'video port'
   format->encoding = MMAL_ENCODING_OPAQUE;
   format->es->video.width = VCOS_ALIGN_UP(state->width, 32);
   format->es->video.height = VCOS_ALIGN_UP(state->height, 16);
   format->es->video.crop.x = 0;
   format->es->video.crop.y = 0;
   format->es->video.crop.width = state->width;
   format->es->video.crop.height = state->height;
   format->es->video.frame_rate.num = state->framerate;
   format->es->video.frame_rate.den = VIDEO_FRAME_RATE_DEN;

   status = mmal_port_format_commit(video_port);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("camera video format couldn't be set");
      goto error;
   }

   // Ensure there are enough buffers to avoid dropping frames
   if (video_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
      video_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;


   // Set the encode format on the still  port

   format = still_port->format;

   format->encoding = MMAL_ENCODING_OPAQUE;
   format->encoding_variant = MMAL_ENCODING_I420;

   format->es->video.width = VCOS_ALIGN_UP(state->width, 32);
   format->es->video.height = VCOS_ALIGN_UP(state->height, 16);
   format->es->video.crop.x = 0;
   format->es->video.crop.y = 0;
   format->es->video.crop.width = state->width;
   format->es->video.crop.height = state->height;
   format->es->video.frame_rate.num = 0;
   format->es->video.frame_rate.den = 1;

   status = mmal_port_format_commit(still_port);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("camera still format couldn't be set");
      goto error;
   }

   /* Ensure there are enough buffers to avoid dropping frames */
   if (still_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
      still_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;

   /* Enable component */
   status = mmal_component_enable(camera);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("camera component couldn't be enabled");
      goto error;
   }

   // Note: this sets lots of parameters that were not individually addressed before.
   raspicamcontrol_set_all_parameters(camera, &state->camera_parameters);

   state->camera_component = camera;

   update_annotation_data(state);

   if (state->verbose)
      GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "W:Camera component done\n");

   return status;

error:

   if (camera)
      mmal_component_destroy(camera);

   return status;
}

/**
 * Destroy the camera component
 *
 * @param state Pointer to state control struct
 *
 */
void raspberryPIcamera::destroy_camera_component(RASPIVID_STATE *state)
{
   if (state->camera_component)
   {
      mmal_component_destroy(state->camera_component);
      state->camera_component = NULL;
   }
}

/**
 * Create the splitter component, set up its ports
 *
 * @param state Pointer to state control struct
 *
 * @return MMAL_SUCCESS if all OK, something else otherwise
 *
 */
MMAL_STATUS_T raspberryPIcamera::create_splitter_component(RASPIVID_STATE *state)
{
   MMAL_COMPONENT_T *splitter = 0;
   MMAL_PORT_T *splitter_output = NULL;
   MMAL_ES_FORMAT_T *format;
   MMAL_STATUS_T status;
   MMAL_POOL_T *pool;
   unsigned int i=0;

   if (state->camera_component == NULL)
   {
      status = MMAL_ENOSYS;
      vcos_log_error("Camera component must be created before splitter");
      goto error;
   }

   /* Create the component */
   status = mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_SPLITTER, &splitter);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("Failed to create splitter component");
      goto error;
   }

   if (!splitter->input_num)
   {
      status = MMAL_ENOSYS;
      vcos_log_error("Splitter doesn't have any input port");
      goto error;
   }

   if (splitter->output_num < 2)
   {
      status = MMAL_ENOSYS;
      vcos_log_error("Splitter doesn't have enough output ports");
      goto error;
   }

   /* Ensure there are enough buffers to avoid dropping frames: */
   mmal_format_copy(splitter->input[0]->format, state->camera_component->output[MMAL_CAMERA_PREVIEW_PORT]->format);

   if (splitter->input[0]->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
      splitter->input[0]->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;

   status = mmal_port_format_commit(splitter->input[0]);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("Unable to set format on splitter input port");
      goto error;
   }

   /* Splitter can do format conversions, configure format for its output port: */
   for (i = 0; i < splitter->output_num; i++)
   {
      mmal_format_copy(splitter->output[i]->format, splitter->input[0]->format);

      if (i == SPLITTER_OUTPUT_PORT)
      {
         format = splitter->output[i]->format;

         switch (state->raw_output_fmt)
         {
         case RAW_OUTPUT_FMT_YUV:
         case RAW_OUTPUT_FMT_GRAY: /* Grayscale image contains only luma (Y) component */
            format->encoding = MMAL_ENCODING_I420;
            format->encoding_variant = MMAL_ENCODING_I420;
            GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [RPI Camera] %s YUV or GRAY raw output (%u)\n", __func__, (unsigned)state->raw_output_fmt);
            break;
         case RAW_OUTPUT_FMT_RGB:
            if (mmal_util_rgb_order_fixed(state->camera_component->output[MMAL_CAMERA_CAPTURE_PORT]))
               format->encoding = MMAL_ENCODING_RGB24;
            else
               format->encoding = MMAL_ENCODING_BGR24;

            GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [RPI Camera] %s RGB raw output (%u)\n", __func__, (unsigned)state->raw_output_fmt);
            format->encoding_variant = 0;  /* Irrelevant when not in opaque mode */
            break;
         default:
            status = MMAL_EINVAL;
            vcos_log_error("unknown raw output format");
            goto error;
         }
      }

      status = mmal_port_format_commit(splitter->output[i]);

      if (status != MMAL_SUCCESS)
      {
         vcos_log_error("Unable to set format on splitter output port %d", i);
         goto error;
      }
   }

   /* Enable component */
   status = mmal_component_enable(splitter);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("splitter component couldn't be enabled");
      goto error;
   }

   /* Create pool of buffer headers for the output port to consume */
   splitter_output = splitter->output[SPLITTER_OUTPUT_PORT];
   pool = mmal_port_pool_create(splitter_output, splitter_output->buffer_num, splitter_output->buffer_size);

   if (!pool)
   {
      vcos_log_error("Failed to create buffer header pool for splitter output port %s", splitter_output->name);
   }

   state->splitter_pool = pool;
   state->splitter_component = splitter;

   if (state->verbose)
      GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "W:Splitter component done\n");

   return status;

error:

   if (splitter)
      mmal_component_destroy(splitter);

   return status;
}

/**
 * Destroy the splitter component
 *
 * @param state Pointer to state control struct
 *
 */
void raspberryPIcamera::destroy_splitter_component(RASPIVID_STATE *state)
{
   // Get rid of any port buffers first
   if (state->splitter_pool)
   {
      mmal_port_pool_destroy(state->splitter_component->output[SPLITTER_OUTPUT_PORT], state->splitter_pool);
   }

   if (state->splitter_component)
   {
      mmal_component_destroy(state->splitter_component);
      state->splitter_component = NULL;
   }
}

/**
 * Create the encoder component, set up its ports
 *
 * @param state Pointer to state control struct
 *
 * @return MMAL_SUCCESS if all OK, something else otherwise
 *
 */
MMAL_STATUS_T raspberryPIcamera::create_encoder_component(RASPIVID_STATE *state)
{
   MMAL_COMPONENT_T *encoder = 0;
   MMAL_PORT_T *encoder_input = NULL, *encoder_output = NULL;
   MMAL_STATUS_T status;
   MMAL_POOL_T *pool;

   status = mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER, &encoder);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("Unable to create video encoder component");
      goto error;
   }

   if (!encoder->input_num || !encoder->output_num)
   {
      status = MMAL_ENOSYS;
      vcos_log_error("Video encoder doesn't have input/output ports");
      goto error;
   }

   encoder_input = encoder->input[0];
   encoder_output = encoder->output[0];

   // We want same format on input and output
   mmal_format_copy(encoder_output->format, encoder_input->format);

   // Only supporting H264 at the moment
   encoder_output->format->encoding = state->encoding;

   if(state->encoding == MMAL_ENCODING_H264)
   {
      if(state->level == MMAL_VIDEO_LEVEL_H264_4)
      {
         if(state->bitrate > MAX_BITRATE_LEVEL4)
         {
            GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E:Bitrate too high: Reducing to 25MBit/s\n");
            state->bitrate = MAX_BITRATE_LEVEL4;
         }
      }
      else
      {
         if(state->bitrate > MAX_BITRATE_LEVEL42)
         {
            GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E:Bitrate too high: Reducing to 62.5MBit/s\n");
            state->bitrate = MAX_BITRATE_LEVEL42;
         }
      }
   }
   else if(state->encoding == MMAL_ENCODING_MJPEG)
   {
      if(state->bitrate > MAX_BITRATE_MJPEG)
      {
         GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E:Bitrate too high: Reducing to 25MBit/s\n");
         state->bitrate = MAX_BITRATE_MJPEG;
      }
   }

   encoder_output->format->bitrate = state->bitrate;

   if (state->encoding == MMAL_ENCODING_H264)
      encoder_output->buffer_size = encoder_output->buffer_size_recommended;
   else
      encoder_output->buffer_size = 256<<10;


   if (encoder_output->buffer_size < encoder_output->buffer_size_min)
      encoder_output->buffer_size = encoder_output->buffer_size_min;

   encoder_output->buffer_num = encoder_output->buffer_num_recommended;

   if (encoder_output->buffer_num < encoder_output->buffer_num_min)
      encoder_output->buffer_num = encoder_output->buffer_num_min;

   // We need to set the frame rate on output to 0, to ensure it gets
   // updated correctly from the input framerate when port connected
   encoder_output->format->es->video.frame_rate.num = 0;
   encoder_output->format->es->video.frame_rate.den = 1;

   // Commit the port changes to the output port
   status = mmal_port_format_commit(encoder_output);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("Unable to set format on video encoder output port");
      goto error;
   }

   // Set the rate control parameter
   if (0)
   {
      MMAL_PARAMETER_VIDEO_RATECONTROL_T param = {{ MMAL_PARAMETER_RATECONTROL, sizeof(param)}, MMAL_VIDEO_RATECONTROL_DEFAULT};
      status = mmal_port_parameter_set(encoder_output, &param.hdr);
      if (status != MMAL_SUCCESS)
      {
         vcos_log_error("Unable to set ratecontrol");
         goto error;
      }

   }

   if (state->encoding == MMAL_ENCODING_H264 &&
       state->intraperiod != -1)
   {
      MMAL_PARAMETER_UINT32_T param = {{ MMAL_PARAMETER_INTRAPERIOD, sizeof(param)}, (uint32_t) state->intraperiod};
      status = mmal_port_parameter_set(encoder_output, &param.hdr);
      if (status != MMAL_SUCCESS)
      {
         vcos_log_error("Unable to set intraperiod");
         goto error;
      }
   }

   if (state->encoding == MMAL_ENCODING_H264 &&
       state->quantisationParameter)
   {
      MMAL_PARAMETER_UINT32_T param = {{ MMAL_PARAMETER_VIDEO_ENCODE_INITIAL_QUANT, sizeof(param)}, (uint32_t) state->quantisationParameter};
      status = mmal_port_parameter_set(encoder_output, &param.hdr);
      if (status != MMAL_SUCCESS)
      {
         vcos_log_error("Unable to set initial QP");
         goto error;
      }

      MMAL_PARAMETER_UINT32_T param2 = {{ MMAL_PARAMETER_VIDEO_ENCODE_MIN_QUANT, sizeof(param)}, (uint32_t) state->quantisationParameter};
      status = mmal_port_parameter_set(encoder_output, &param2.hdr);
      if (status != MMAL_SUCCESS)
      {
         vcos_log_error("Unable to set min QP");
         goto error;
      }

      MMAL_PARAMETER_UINT32_T param3 = {{ MMAL_PARAMETER_VIDEO_ENCODE_MAX_QUANT, sizeof(param)}, (uint32_t) state->quantisationParameter};
      status = mmal_port_parameter_set(encoder_output, &param3.hdr);
      if (status != MMAL_SUCCESS)
      {
         vcos_log_error("Unable to set max QP");
         goto error;
      }

   }

   if (state->encoding == MMAL_ENCODING_H264)
   {
      MMAL_PARAMETER_VIDEO_PROFILE_T  param;
      param.hdr.id = MMAL_PARAMETER_PROFILE;
      param.hdr.size = sizeof(param);

      param.profile[0].profile = (MMAL_VIDEO_PROFILE_T)state->profile;

      if((VCOS_ALIGN_UP(state->width,16) >> 4) * (VCOS_ALIGN_UP(state->height,16) >> 4) * state->framerate > 245760)
      {
         if((VCOS_ALIGN_UP(state->width,16) >> 4) * (VCOS_ALIGN_UP(state->height,16) >> 4) * state->framerate <= 522240)
         {
            GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E:Too many macroblocks/s: Increasing H264 Level to 4.2\n");
            state->level=MMAL_VIDEO_LEVEL_H264_42;
         }
         else
         {
            vcos_log_error("Too many macroblocks/s requested");
            goto error;
         }
      }

      param.profile[0].level = (MMAL_VIDEO_LEVEL_T)state->level;

      status = mmal_port_parameter_set(encoder_output, &param.hdr);
      if (status != MMAL_SUCCESS)
      {
         vcos_log_error("Unable to set H264 profile");
         goto error;
      }
   }

   if (mmal_port_parameter_set_boolean(encoder_input, MMAL_PARAMETER_VIDEO_IMMUTABLE_INPUT, state->immutableInput) != MMAL_SUCCESS)
   {
      vcos_log_error("Unable to set immutable input flag");
      // Continue rather than abort..
   }

   //set INLINE HEADER flag to generate SPS and PPS for every IDR if requested
   if (mmal_port_parameter_set_boolean(encoder_output, MMAL_PARAMETER_VIDEO_ENCODE_INLINE_HEADER, state->bInlineHeaders) != MMAL_SUCCESS)
   {
      vcos_log_error("failed to set INLINE HEADER FLAG parameters");
      // Continue rather than abort..
   }

   //set INLINE VECTORS flag to request motion vector estimates
   if (state->encoding == MMAL_ENCODING_H264 &&
       mmal_port_parameter_set_boolean(encoder_output, MMAL_PARAMETER_VIDEO_ENCODE_INLINE_VECTORS, state->inlineMotionVectors) != MMAL_SUCCESS)
   {
      vcos_log_error("failed to set INLINE VECTORS parameters");
      // Continue rather than abort..
   }

   // Adaptive intra refresh settings
   if (state->encoding == MMAL_ENCODING_H264 &&
       state->intra_refresh_type != -1)
   {
      MMAL_PARAMETER_VIDEO_INTRA_REFRESH_T  param;
      param.hdr.id = MMAL_PARAMETER_VIDEO_INTRA_REFRESH;
      param.hdr.size = sizeof(param);

      // Get first so we don't overwrite anything unexpectedly
      status = mmal_port_parameter_get(encoder_output, &param.hdr);
      if (status != MMAL_SUCCESS)
      {
         vcos_log_warn("Unable to get existing H264 intra-refresh values. Please update your firmware");
         // Set some defaults, don't just pass random stack data
         param.air_mbs = param.air_ref = param.cir_mbs = param.pir_mbs = 0;
      }

      param.refresh_mode = (MMAL_VIDEO_INTRA_REFRESH_T)state->intra_refresh_type;

      //if (state->intra_refresh_type == MMAL_VIDEO_INTRA_REFRESH_CYCLIC_MROWS)
      //   param.cir_mbs = 10;

      status = mmal_port_parameter_set(encoder_output, &param.hdr);
      if (status != MMAL_SUCCESS)
      {
         vcos_log_error("Unable to set H264 intra-refresh values");
         goto error;
      }
   }

   //  Enable component
   status = mmal_component_enable(encoder);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("Unable to enable video encoder component");
      goto error;
   }

   /* Create pool of buffer headers for the output port to consume */
   pool = mmal_port_pool_create(encoder_output, encoder_output->buffer_num, encoder_output->buffer_size);

   if (!pool)
   {
      vcos_log_error("Failed to create buffer header pool for encoder output port %s", encoder_output->name);
   }

   state->encoder_pool = pool;
   state->encoder_component = encoder;

   if (state->verbose)
      GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "W:Encoder component done\n");

   return status;

   error:
   if (encoder)
      mmal_component_destroy(encoder);

   state->encoder_component = NULL;

   return status;
}

/**
 * Destroy the encoder component
 *
 * @param state Pointer to state control struct
 *
 */
void raspberryPIcamera::destroy_encoder_component(RASPIVID_STATE *state)
{
   // Get rid of any port buffers first
   if (state->encoder_pool)
   {
      mmal_port_pool_destroy(state->encoder_component->output[0], state->encoder_pool);
   }

   if (state->encoder_component)
   {
      mmal_component_destroy(state->encoder_component);
      state->encoder_component = NULL;
   }
}

/// ----------------------- Open/ Destroy/ Setting Camera  ----------------------------
bool raspberryPIcamera::SetVerbose(bool bValue){
	bool bRst = false;

    if (!state.camera_component){
 	   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [RPI Camera] %s camera component not ready\n", __func__);
    	return bRst;
    }

    if (bValue == true){
    	state.verbose = 1;
    }else{
    	state.verbose = 0;
    }
    bRst = true;

    return bRst;
}

bool raspberryPIcamera::SetResolution(unsigned int iWidth, unsigned int iHeight){
	bool bRst = false;

    if (state.camera_component){
 	   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [RPI Camera] %s camera component not close\n", __func__);
    	return bRst;
    }

    if(iWidth ==0 || iHeight ==0){
    	// error
    	GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [RPI Camera] Resolution parameter error: w:%u h:%u\n", iWidth, iHeight);
    }else{
    	state.width = iWidth;
    	state.height = iHeight;
    	bIsNeedRestart = true;
    	bRst = true;
    	GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [RPI Camera] Resolution parameter : w:%u h:%u\n", iWidth, iHeight);
    }

    return bRst;
}

bool raspberryPIcamera::SetTimeout(unsigned int iTimeout){
	bool bRst = false;

    if (state.camera_component){
 	   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [RPI Camera] %s camera component not close\n", __func__);
    	return bRst;
    }

    /*
     *   if (sscanf(argv[i + 1], "%u", &state->timeout) == 1)
         {
            // Ensure that if previously selected a waitMethod we don't overwrite it
            if (state->timeout == 0 && state->waitMethod == WAIT_METHOD_NONE)
               state->waitMethod = WAIT_METHOD_FOREVER;

            i++;
         }
     */

    state.timeout = iTimeout;
    if (state.timeout == 0 && state.waitMethod == WAIT_METHOD_NONE)
        state.waitMethod = WAIT_METHOD_FOREVER;

	bIsNeedRestart = true;
	bRst = true;
	GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [RPI Camera] %s set timeout:%d, waitMethod:%d\n", __func__, state.timeout, state.waitMethod);

    return bRst;
}

bool raspberryPIcamera::SetInlineHeadersEnable(bool bValue){
	bool bRst = false;

	if (state.camera_component){
		   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [RPI Camera] %s camera component not close\n", __func__);
		return bRst;
	}

	if ((bValue == true)&&(state.bInlineHeaders!=1)){
		state.bInlineHeaders = 1;
		bIsNeedRestart = true;
	}else if ((bValue == false)&&(state.bInlineHeaders!=0)){
		state.bInlineHeaders = 0;
		bIsNeedRestart = true;
	}else{
		// already setting.
	}

	bRst = true;

	return bRst;
}

bool raspberryPIcamera::SetPreviewAfterEncEnable(bool bValue){
	bool bRst = false;

	if (state.camera_component){
		   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [RPI Camera] %s camera component not close\n", __func__);
		return bRst;
	}

	/**
	if ((bValue == true)&&(state.immutableInput != 0)){
		state.immutableInput = 0;
	}else if ((bValue == false)&&(state.immutableInput != 1)){
		state.immutableInput = 1;
	}else{
		// already setting.
	}
	**/

	if ((bValue == true)&&(bIsPreview == false)){
		bIsPreview = true;
	}else if ((bValue == false)&&(bIsPreview == true)){
		bIsPreview = false;
	}else{
		// already setting.
	}

	bRst = true;

	return bRst;
}

bool raspberryPIcamera::SetBitrate(unsigned int iBitrate){
//      case CommandBitrate: // 1-100
//         if (sscanf(argv[i + 1], "%u", &state->bitrate) == 1)
	bool bRst = false;

    if (state.camera_component){
 	   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [RPI Camera] %s camera component not close\n", __func__);
    	return bRst;
    }

    //if (iBitrate >= 1 && iBitrate <= 100)
    {
        //MMAL_RATIONAL_T value = {iBitrate, 100};
        //if (0== mmal_status_to_int(mmal_port_parameter_set_rational(state.camera_component->control, MMAL_PARAMETER_SATURATION, value))){
        //	bRst = true;
        //}
    	state.bitrate = iBitrate;
    	GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [RPI Camera] %s state.bitrate = %d\n", __func__, state.bitrate);
    	bRst = true;
        bIsNeedRestart = true;
    }
    //else
    //{
        //ofLog(OF_LOG_ERROR, "Invalid saturation value");
    //	printf("E [%s] %s Invalid saturation value: %d\n", "RPI Camera", __func__, iBitrate);
    //}

    return bRst;
}

bool raspberryPIcamera::SetIntraAndPeriodFrameRate(int iIPFPS){
	bool bRst = false;

    if (state.camera_component){
 	   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [RPI Camera] %s camera component not close\n", __func__);
    	return bRst;
    }

    //if (state->encoding == MMAL_ENCODING_H264 &&
    //    state->intraperiod != -1)
    //       MMAL_PARAMETER_UINT32_T param = {{ MMAL_PARAMETER_INTRAPERIOD, sizeof(param)}, (uint32_t) state->intraperiod};
    // status = mmal_port_parameter_set(encoder_output, &param.hdr);

    if (iIPFPS < 1){
    	// close (default setting)
    	state.intraperiod = -1;
    }else if ((iIPFPS > 1) && (iIPFPS <=30 )){
    	state.intraperiod = iIPFPS;
    }else{
    	GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [RPI Camera] device could not support well, key frame rate:%d\n", iIPFPS);
    	state.intraperiod = iIPFPS;
    }

    bIsNeedRestart = true;
    bRst = true;
    return bRst;
}

bool raspberryPIcamera::SetFramePerSecond(unsigned int iFPS){
	//state->framerate
	bool bRst = false;

    if (state.camera_component){
 	   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [RPI Camera] %s camera component not close\n", __func__);
    	return bRst;
    }

    //fps 1 - 30 - 120
    if (iFPS == 0){
    	GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [RPI Camera] device not support, fps:%d\n", iFPS);
    }else if(iFPS > 1 && iFPS <= 30){
        bRst = true;
        state.framerate = iFPS;
        bIsNeedRestart = true;
    }else{
    	// iFPS > 30
    	GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [RPI Camera] device could not support well, fps:%d\n", iFPS);
        bRst = true;
        state.framerate = iFPS;
        bIsNeedRestart = true;
    }

    return bRst;
}


bool raspberryPIcamera::SetSaturation(int iSaturation)
{
	bool bRst = false;

    if (!state.camera_component){
 	   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [RPI Camera] %s camera component not ready\n", __func__);
    	return bRst;
    }

    if (iSaturation >= -100 && iSaturation <= 100)
    {
        MMAL_RATIONAL_T value = {iSaturation, 100};
        if (0== mmal_status_to_int(mmal_port_parameter_set_rational(state.camera_component->control, MMAL_PARAMETER_SATURATION, value))){
        	bRst = true;
        }
    }
    else
    {
        //ofLog(OF_LOG_ERROR, "Invalid saturation value");
    	printf("E [%s] %s Invalid saturation value: %d\n", "RPI Camera", __func__, iSaturation);
    }
	return bRst;
}

bool raspberryPIcamera::SetSharpness(int iSharpness)
{
	bool bRst = false;

    if (!state.camera_component){
 	   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [RPI Camera] %s camera component not ready\n", __func__);
    	return bRst;
    }

    if (iSharpness >= -100 && iSharpness <= 100)
    {
        MMAL_RATIONAL_T value = {iSharpness, 100};
        if (0== mmal_status_to_int(mmal_port_parameter_set_rational(state.camera_component->control, MMAL_PARAMETER_SHARPNESS, value))){
        	bRst = true;
        }
    }
    else
    {
        //ofLog(OF_LOG_ERROR, "Invalid saturation value");
    	printf("E [%s] %s Invalid saturation value: %d\n", "RPI Camera", __func__, iSharpness);

    }
	return bRst;
}

bool raspberryPIcamera::SetContrast(int iContrast)
{
	bool bRst = false;

    if (!state.camera_component){
 	   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [RPI Camera] %s camera component not ready\n", __func__);
    	return bRst;
    }

    if (iContrast >= -100 && iContrast <= 100)
    {
        MMAL_RATIONAL_T value = {iContrast, 100};
        if (0== mmal_status_to_int(mmal_port_parameter_set_rational(state.camera_component->control, MMAL_PARAMETER_CONTRAST, value))){
        	bRst = true;
        }
    }
    else
    {
        //ofLog(OF_LOG_ERROR, "Invalid saturation value");
    	printf("E [%s] %s Invalid saturation value: %d\n", "RPI Camera", __func__, iContrast);

    }
	return bRst;
}


//int raspicamcontrol_set_brightness(MMAL_COMPONENT_T *camera, int brightness)
bool raspberryPIcamera::SetBrightness(int iBrightness)
{
   //int ret = 0;

   //if (!camera)
      //return 1;
   bool bRst = false;

   if (!state.camera_component){
	   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [RPI Camera] %s camera component not ready\n", __func__);
	   return bRst;
   }

   if (iBrightness >= 0 && iBrightness <= 100)
   {
      MMAL_RATIONAL_T value = {iBrightness, 100};
      //ret = mmal_status_to_int(mmal_port_parameter_set_rational(camera->control, MMAL_PARAMETER_BRIGHTNESS, value));
      if (0== mmal_status_to_int(mmal_port_parameter_set_rational(state.camera_component->control, MMAL_PARAMETER_BRIGHTNESS, value))){
      	bRst = true;
      }
   }
   else
   {
      vcos_log_error("Invalid brightness value");
      //ret = 1;
   }

	return bRst;
}

/**
 * Set exposure mode for images
 * @param camera Pointer to camera component
 * @param mode Exposure mode to set from
 *   - MMAL_PARAM_EXPOSUREMODE_OFF,
 *   - MMAL_PARAM_EXPOSUREMODE_AUTO,
 *   - MMAL_PARAM_EXPOSUREMODE_NIGHT,
 *   - MMAL_PARAM_EXPOSUREMODE_NIGHTPREVIEW,
 *   - MMAL_PARAM_EXPOSUREMODE_BACKLIGHT,
 *   - MMAL_PARAM_EXPOSUREMODE_SPOTLIGHT,
 *   - MMAL_PARAM_EXPOSUREMODE_SPORTS,
 *   - MMAL_PARAM_EXPOSUREMODE_SNOW,
 *   - MMAL_PARAM_EXPOSUREMODE_BEACH,
 *   - MMAL_PARAM_EXPOSUREMODE_VERYLONG,
 *   - MMAL_PARAM_EXPOSUREMODE_FIXEDFPS,
 *   - MMAL_PARAM_EXPOSUREMODE_ANTISHAKE,
 *   - MMAL_PARAM_EXPOSUREMODE_FIREWORKS,
 *
 * @return true if successful, non-zero if any parameters out of range
 */
bool raspberryPIcamera::SetExposurebyCStr(const char* pcstrName){
	if(strstr(pcstrName, "off") != NULL){
		return SetExposure(MMAL_PARAM_EXPOSUREMODE_OFF);
	}else if (strstr(pcstrName, "auto") != NULL){
		return SetExposure(MMAL_PARAM_EXPOSUREMODE_AUTO);
	}else if (strstr(pcstrName, "night") != NULL){
		return SetExposure(MMAL_PARAM_EXPOSUREMODE_NIGHT);
	}else if (strstr(pcstrName, "nightpreview") != NULL){
		return SetExposure(MMAL_PARAM_EXPOSUREMODE_NIGHTPREVIEW);
	}else if (strstr(pcstrName, "backlight") != NULL){
		return SetExposure(MMAL_PARAM_EXPOSUREMODE_BACKLIGHT);
	}else if (strstr(pcstrName, "spotlight") != NULL){
		return SetExposure(MMAL_PARAM_EXPOSUREMODE_SPOTLIGHT);
	}else if (strstr(pcstrName, "snow") != NULL){
		return SetExposure(MMAL_PARAM_EXPOSUREMODE_SNOW);
	}else if (strstr(pcstrName, "beach") != NULL){
		return SetExposure(MMAL_PARAM_EXPOSUREMODE_BEACH);
	}else if (strstr(pcstrName, "verylong") != NULL){
		return SetExposure(MMAL_PARAM_EXPOSUREMODE_VERYLONG);
	}else if (strstr(pcstrName, "fixedcfps") != NULL){
		return SetExposure(MMAL_PARAM_EXPOSUREMODE_FIXEDFPS);
	}else if (strstr(pcstrName, "antshake") != NULL){
		return SetExposure(MMAL_PARAM_EXPOSUREMODE_ANTISHAKE);
	}else if (strstr(pcstrName, "fireworks") != NULL){
		return SetExposure(MMAL_PARAM_EXPOSUREMODE_FIREWORKS);
	}else if (strstr(pcstrName, "sports") != NULL){
		return SetExposure(MMAL_PARAM_EXPOSUREMODE_SPORTS);
	}else{

		GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [RPI Camera] %s unknown command '%s'\n", __func__, pcstrName);
		return false;
	}
}
//int raspicamcontrol_set_exposure_mode(MMAL_COMPONENT_T *camera, MMAL_PARAM_EXPOSUREMODE_T mode)
bool raspberryPIcamera::SetExposure(MMAL_PARAM_EXPOSUREMODE_T tExposureMode)
{
   MMAL_PARAMETER_EXPOSUREMODE_T exp_mode = {{MMAL_PARAMETER_EXPOSURE_MODE,sizeof(exp_mode)}, tExposureMode};

   //if (!camera)
   //return 1;
	bool bRst = false;

	if (!state.camera_component){
		   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [RPI Camera] %s camera component not ready\n", __func__);
		return bRst;
	}

    //return mmal_status_to_int(mmal_port_parameter_set(camera->control, &exp_mode.hdr));
	if (0== mmal_status_to_int(mmal_port_parameter_set(state.camera_component->control, &exp_mode.hdr))){
        bRst = true;
    }

	return bRst;
}

/**
 * Set the aWB (auto white balance) mode for images
 * @param camera Pointer to camera component
 * @param awb_mode Value to set from
 *   - MMAL_PARAM_AWBMODE_OFF,
 *   - MMAL_PARAM_AWBMODE_AUTO,
 *   - MMAL_PARAM_AWBMODE_SUNLIGHT,
 *   - MMAL_PARAM_AWBMODE_CLOUDY,
 *   - MMAL_PARAM_AWBMODE_SHADE,
 *   - MMAL_PARAM_AWBMODE_TUNGSTEN,
 *   - MMAL_PARAM_AWBMODE_FLUORESCENT,
 *   - MMAL_PARAM_AWBMODE_INCANDESCENT,
 *   - MMAL_PARAM_AWBMODE_FLASH,
 *   - MMAL_PARAM_AWBMODE_HORIZON,
 * @return true if successful, non-zero if any parameters out of range
 */
bool raspberryPIcamera::SetWriteBalancebyCStr(const char* pcstrName){
	if(strstr(pcstrName, "off") != NULL){
		return SetWriteBalance(MMAL_PARAM_AWBMODE_OFF);
	}else if (strstr(pcstrName, "auto") != NULL){
		return SetWriteBalance(MMAL_PARAM_AWBMODE_AUTO);
	}else if (strstr(pcstrName, "sunlight") != NULL){
		return SetWriteBalance(MMAL_PARAM_AWBMODE_SUNLIGHT);
	}else if (strstr(pcstrName, "cloudy") != NULL){
		return SetWriteBalance(MMAL_PARAM_AWBMODE_CLOUDY);
	}else if (strstr(pcstrName, "shade") != NULL){
		return SetWriteBalance(MMAL_PARAM_AWBMODE_SHADE);
	}else if (strstr(pcstrName, "tungsten") != NULL){
		return SetWriteBalance(MMAL_PARAM_AWBMODE_TUNGSTEN);
	}else if (strstr(pcstrName, "fluorescent") != NULL){
		return SetWriteBalance(MMAL_PARAM_AWBMODE_FLUORESCENT);
	}else if (strstr(pcstrName, "incandescent") != NULL){
		return SetWriteBalance(MMAL_PARAM_AWBMODE_INCANDESCENT);
	}else if (strstr(pcstrName, "flash") != NULL){
		return SetWriteBalance(MMAL_PARAM_AWBMODE_FLASH);
	}else if (strstr(pcstrName, "horizon") != NULL){
		return SetWriteBalance(MMAL_PARAM_AWBMODE_HORIZON);
	}else{

		GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [RPI Camera] %s unknown command '%s'\n", __func__, pcstrName);
		return false;
	}
}
bool raspberryPIcamera::SetWriteBalance(MMAL_PARAM_AWBMODE_T TAwbMode){
	bool bRst = false;

	MMAL_PARAMETER_AWBMODE_T param = {{MMAL_PARAMETER_AWB_MODE,sizeof(param)}, TAwbMode};

	if (!state.camera_component){
		   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [RPI Camera] %s camera component not ready\n", __func__);
		return bRst;
	}

	if (0== mmal_status_to_int(mmal_port_parameter_set(state.camera_component->control, &param.hdr))){
		 bRst = true;
	}

	return bRst;
}

// set awb color Red & Blue Gain value
bool raspberryPIcamera::SetAutoWriteBalanceGain(float fR_gain, float fB_gain)
{
   bool bRst = false;
   MMAL_PARAMETER_AWB_GAINS_T param = {{MMAL_PARAMETER_CUSTOM_AWB_GAINS,sizeof(param)}, {0,0}, {0,0}};

   //if (!camera)
   //   return 1;
	if (!state.camera_component){
		   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [RPI Camera] %s camera component not ready\n", __func__);
		return bRst;
	}

   if (!fR_gain || !fB_gain){
      GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [%s] input parameter error: r_gain:%.03f b_gain:%.03f\n", "RPI Camera", fR_gain, fB_gain);
      return bRst;
   }

   param.r_gain.num = (unsigned int)(fR_gain * 65536);
   param.b_gain.num = (unsigned int)(fB_gain * 65536);
   param.r_gain.den = 65536;
   param.b_gain.den = 65536;
   if (0== mmal_status_to_int(mmal_port_parameter_set(state.camera_component->control, &param.hdr))){
		 bRst = true;
	}
	return bRst;
}

/**
 * Set the image effect for the images
 * @param camera Pointer to camera component
 * @param imageFX Value from
 *   - MMAL_PARAM_IMAGEFX_NONE,
 *   - MMAL_PARAM_IMAGEFX_NEGATIVE,
 *   - MMAL_PARAM_IMAGEFX_SOLARIZE,
 *   - MMAL_PARAM_IMAGEFX_POSTERIZE,
 *   - MMAL_PARAM_IMAGEFX_WHITEBOARD,
 *   - MMAL_PARAM_IMAGEFX_BLACKBOARD,
 *   - MMAL_PARAM_IMAGEFX_SKETCH,
 *   - MMAL_PARAM_IMAGEFX_DENOISE,
 *   - MMAL_PARAM_IMAGEFX_EMBOSS,
 *   - MMAL_PARAM_IMAGEFX_OILPAINT,
 *   - MMAL_PARAM_IMAGEFX_HATCH,
 *   - MMAL_PARAM_IMAGEFX_GPEN,
 *   - MMAL_PARAM_IMAGEFX_PASTEL,
 *   - MMAL_PARAM_IMAGEFX_WATERCOLOUR,
 *   - MMAL_PARAM_IMAGEFX_FILM,
 *   - MMAL_PARAM_IMAGEFX_BLUR,
 *   - MMAL_PARAM_IMAGEFX_SATURATION,
 *   - MMAL_PARAM_IMAGEFX_COLOURSWAP,
 *   - MMAL_PARAM_IMAGEFX_WASHEDOUT,
 *   - MMAL_PARAM_IMAGEFX_POSTERISE,
 *   - MMAL_PARAM_IMAGEFX_COLOURPOINT,
 *   - MMAL_PARAM_IMAGEFX_COLOURBALANCE,
 *   - MMAL_PARAM_IMAGEFX_CARTOON,
 * @return true if successful, non-zero if any parameters out of range
 */
bool raspberryPIcamera::SetImageViewEffectbyCStr(const char* pcstrName){
	if(strstr(pcstrName, "none") != NULL){
		return SetImageViewEffect(MMAL_PARAM_IMAGEFX_NONE);
	}else if (strstr(pcstrName, "negative") != NULL){
		return SetImageViewEffect(MMAL_PARAM_IMAGEFX_NEGATIVE);
	}else if (strstr(pcstrName, "solarize") != NULL){
		return SetImageViewEffect(MMAL_PARAM_IMAGEFX_SOLARIZE);
	}else if (strstr(pcstrName, "posterize") != NULL){
		return SetImageViewEffect(MMAL_PARAM_IMAGEFX_POSTERIZE);
	}else if (strstr(pcstrName, "whiteboard") != NULL){
		return SetImageViewEffect(MMAL_PARAM_IMAGEFX_WHITEBOARD);
	}else if (strstr(pcstrName, "blackboard") != NULL){
		return SetImageViewEffect(MMAL_PARAM_IMAGEFX_BLACKBOARD);
	}else if (strstr(pcstrName, "sketch") != NULL){
		return SetImageViewEffect(MMAL_PARAM_IMAGEFX_SKETCH);
	}else if (strstr(pcstrName, "denoise") != NULL){
		return SetImageViewEffect(MMAL_PARAM_IMAGEFX_DENOISE);
	}else if (strstr(pcstrName, "emboss") != NULL){
		return SetImageViewEffect(MMAL_PARAM_IMAGEFX_EMBOSS);
	}else if (strstr(pcstrName, "oilpaint") != NULL){
		return SetImageViewEffect(MMAL_PARAM_IMAGEFX_OILPAINT);
	}else if (strstr(pcstrName, "hatch") != NULL){
		return SetImageViewEffect(MMAL_PARAM_IMAGEFX_HATCH);
	}else if (strstr(pcstrName, "gpen") != NULL){
		return SetImageViewEffect(MMAL_PARAM_IMAGEFX_GPEN);
	}else if (strstr(pcstrName, "pastel") != NULL){
		return SetImageViewEffect(MMAL_PARAM_IMAGEFX_PASTEL);
	}else if (strstr(pcstrName, "watercolour") != NULL){
		return SetImageViewEffect(MMAL_PARAM_IMAGEFX_WATERCOLOUR);
	}else if (strstr(pcstrName, "film") != NULL){
		return SetImageViewEffect(MMAL_PARAM_IMAGEFX_FILM);
	}else if (strstr(pcstrName, "blur") != NULL){
		return SetImageViewEffect(MMAL_PARAM_IMAGEFX_BLUR);
	}else if (strstr(pcstrName, "saturation") != NULL){
		return SetImageViewEffect(MMAL_PARAM_IMAGEFX_SATURATION);
	}else if (strstr(pcstrName, "colourswap") != NULL){
		return SetImageViewEffect(MMAL_PARAM_IMAGEFX_COLOURSWAP);
	}else if (strstr(pcstrName, "washedout") != NULL){
		return SetImageViewEffect(MMAL_PARAM_IMAGEFX_WASHEDOUT);
	}else if (strstr(pcstrName, "posterise") != NULL){
		return SetImageViewEffect(MMAL_PARAM_IMAGEFX_POSTERISE);
	}else if (strstr(pcstrName, "colourpoint") != NULL){
		return SetImageViewEffect(MMAL_PARAM_IMAGEFX_COLOURPOINT);
	}else if (strstr(pcstrName, "colourbalance") != NULL){
		return SetImageViewEffect(MMAL_PARAM_IMAGEFX_COLOURBALANCE);
	}else if (strstr(pcstrName, "cartoon") != NULL){
		return SetImageViewEffect(MMAL_PARAM_IMAGEFX_CARTOON);
	}else{

		GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [RPI Camera] %s unknown command '%s'\n", __func__, pcstrName);
		return false;
	}
}
//int raspicamcontrol_set_imageFX(MMAL_COMPONENT_T *camera, MMAL_PARAM_IMAGEFX_T imageFX)
bool raspberryPIcamera::SetImageViewEffect(MMAL_PARAM_IMAGEFX_T TimageFX)
{
   bool bRst = false;
   MMAL_PARAMETER_IMAGEFX_T imgFX = {{MMAL_PARAMETER_IMAGE_EFFECT,sizeof(imgFX)}, TimageFX};

   //if (!camera)
   //   return 1;
   if (!state.camera_component){
	   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [RPI Camera] %s camera component not ready\n", __func__);
	   return bRst;
   }

   if (0== mmal_status_to_int(mmal_port_parameter_set(state.camera_component->control, &imgFX.hdr))){
		 bRst = true;
	}
	return bRst;
}

/**
 * set raw capture is enable or not
 *
 * @param Raw capture Format Value from
 * RAW_OUTPUT_FMT_YUV = 1,
 * RAW_OUTPUT_FMT_RGB,
 * RAW_OUTPUT_FMT_GRAY
 *
 * @return true if successful, non-zero if any parameters out of range
 */
bool raspberryPIcamera::SetRawCapturingbyCStr(bool bValue, const char* pcstrName){
	if(strstr(pcstrName, "yuv") != NULL){
		return SetRawCapturingEable(bValue, (RAW_OUTPUT_FMT)1);
	}else if (strstr(pcstrName, "rgb") != NULL){
		return SetRawCapturingEable(bValue, (RAW_OUTPUT_FMT)2);
	}else if (strstr(pcstrName, "gray") != NULL){
		return SetRawCapturingEable(bValue, (RAW_OUTPUT_FMT)3);
	}else{
	    GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [RPI Camera] unknown raw output format '%s'\n", pcstrName);
		return false;
	}
}
bool raspberryPIcamera::SetRawCapturingEable(bool bValue, RAW_OUTPUT_FMT eRawFmt){
   bool bRst = false;

   if (state.camera_component) return bRst;

   if (bValue == true){
	   state.raw_output = 1;
   }else{
	   state.raw_output = 0;

       bIsNeedRestart = true;
	   // stop capturing, don't care the format about encoder
	   bRst = true;
	   return bRst;
   }

   //state.raw_output_fmt = RAW_OUTPUT_FMT_YUV;
   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [RPI Camera] set raw output format '%d'\n", eRawFmt);
   switch (eRawFmt)
   {
   //case RAW_OUTPUT_FMT_YUV:
   case 1:
      bRst = true;
      state.raw_output_fmt = (RAW_OUTPUT_FMT)1;
      break;
   //case RAW_OUTPUT_FMT_GRAY:
   case 3:
      bRst = true;
      state.raw_output_fmt = (RAW_OUTPUT_FMT)3;
      break;
   //case RAW_OUTPUT_FMT_RGB:
   case 2:
      bRst = true;
      state.raw_output_fmt = (RAW_OUTPUT_FMT)2;
      break;
   default:
      //state.raw_output_fmt = RAW_OUTPUT_FMT_RGB;
      state.raw_output_fmt = (RAW_OUTPUT_FMT)2;
      vcos_log_error("unknown raw output format");
      break;
   }

   if (bRst == true){
       bIsNeedRestart = true;
   }
   return bRst;
}

/**
 * @parameter 10~40 interger, 0 disable (default)
 */
bool raspberryPIcamera::SetQuantisation(unsigned int iQP){
	bool bRst = false;

	if (state.camera_component) return bRst;

	if (iQP < 10){
		if (iQP != 0){
			GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [RPI Camera] quantisation(quantization) parameter error: %d (0:disable, or 10~40)\n", iQP);

		}else{
			state.quantisationParameter = 0;
	        bIsNeedRestart = true;
			bRst = true;
		}
	}
	else if (iQP >= 10 && iQP <= 40)
	{
		state.quantisationParameter = iQP;
        bIsNeedRestart = true;
		bRst = true;
	}
	else
	{
		vcos_log_error("Invalid quantisation [QP] value");
		//ret = 1;
	}

	return bRst;
}
bool raspberryPIcamera::SetQuantization(unsigned int iQP){
	return SetQuantisation(iQP);
}

/** Video profiles.{ MMAL_VIDEO_PROFILE_T }
 * Only certain combinations of profile and level will be valid.
 * @ref MMAL_VIDEO_LEVEL_T
 * hander file: mmal_parameters_video.h
 * @parameter '+': h264 suggest choice (plz. choice one of profile_map)
 * @ref profile_map[]
 *
    MMAL_VIDEO_PROFILE_H263_BASELINE,
    MMAL_VIDEO_PROFILE_H263_H320CODING,
    MMAL_VIDEO_PROFILE_H263_BACKWARDCOMPATIBLE,
    MMAL_VIDEO_PROFILE_H263_ISWV2,
    MMAL_VIDEO_PROFILE_H263_ISWV3,
    MMAL_VIDEO_PROFILE_H263_HIGHCOMPRESSION,
    MMAL_VIDEO_PROFILE_H263_INTERNET,
    MMAL_VIDEO_PROFILE_H263_INTERLACE,
    MMAL_VIDEO_PROFILE_H263_HIGHLATENCY,
    MMAL_VIDEO_PROFILE_MP4V_SIMPLE,
    MMAL_VIDEO_PROFILE_MP4V_SIMPLESCALABLE,
    MMAL_VIDEO_PROFILE_MP4V_CORE,
    MMAL_VIDEO_PROFILE_MP4V_MAIN,
    MMAL_VIDEO_PROFILE_MP4V_NBIT,
    MMAL_VIDEO_PROFILE_MP4V_SCALABLETEXTURE,
    MMAL_VIDEO_PROFILE_MP4V_SIMPLEFACE,
    MMAL_VIDEO_PROFILE_MP4V_SIMPLEFBA,
    MMAL_VIDEO_PROFILE_MP4V_BASICANIMATED,
    MMAL_VIDEO_PROFILE_MP4V_HYBRID,
    MMAL_VIDEO_PROFILE_MP4V_ADVANCEDREALTIME,
    MMAL_VIDEO_PROFILE_MP4V_CORESCALABLE,
    MMAL_VIDEO_PROFILE_MP4V_ADVANCEDCODING,
    MMAL_VIDEO_PROFILE_MP4V_ADVANCEDCORE,
    MMAL_VIDEO_PROFILE_MP4V_ADVANCEDSCALABLE,
    MMAL_VIDEO_PROFILE_MP4V_ADVANCEDSIMPLE,
 +  MMAL_VIDEO_PROFILE_H264_BASELINE,
 +  MMAL_VIDEO_PROFILE_H264_MAIN,
    MMAL_VIDEO_PROFILE_H264_EXTENDED,
 +  MMAL_VIDEO_PROFILE_H264_HIGH,
    MMAL_VIDEO_PROFILE_H264_HIGH10,
    MMAL_VIDEO_PROFILE_H264_HIGH422,
    MMAL_VIDEO_PROFILE_H264_HIGH444,
    MMAL_VIDEO_PROFILE_H264_CONSTRAINED_BASELINE,
    MMAL_VIDEO_PROFILE_DUMMY = 0x7FFFFFFF
 *
 */
bool raspberryPIcamera::SetH264Profile(MMAL_VIDEO_PROFILE_T eVideoProfile){
	bool bRst = false;
	int	l_iMmalMode = -1;

	if (state.camera_component) return bRst;

	// check manual choosing be supported or not?
	int i=0;
	for (i=0; i<profile_map_size; i++){
		if (profile_map[i].mmal_mode == (int)eVideoProfile){
			l_iMmalMode = (int)eVideoProfile;
		}
	}

    if( l_iMmalMode == -1){
       GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [RPI Camera] Sorry, your choice [video profile:%d] is not support for ADE H264 encoder freature\n", eVideoProfile);
       state.profile = MMAL_VIDEO_PROFILE_H264_HIGH;
    }else{
    	state.profile = l_iMmalMode;
    }

    bIsNeedRestart = true;
    bRst = true;
	return bRst;
}

bool raspberryPIcamera::SetOutputParameter(const char* pcstrPath, unsigned int iPathLength){
	int valid = 1;

	if (state.camera_component) return false;

    int len = iPathLength;
    if (len >0)
    {
       strncpy(m_filename, pcstrPath, MAX_PATH-1);
       state.filename = m_filename; //(char*)malloc(len + 1);
       state.filename[len] = '\0';
       vcos_assert(state.filename);
       if (state.filename)
    	   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [RPI Camera] State is 'filename: %s'\n", state.filename);

       /**
       state.filename = (char*)malloc(len + 1);
       vcos_assert(state.filename);
       if (state.filename){
          strncpy(state.filename, pcstrPath, len+1);
          //GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [RPI Camera] State is 'filename: %s'\n", state.filename);
       }
       **/

    }
    else
    {
       valid = 0;
       vcos_log_error("Invalid quantisation [Output Path] value");
    }

    // Always disable verbose if output going to stdout
    if (state.filename && state.filename[0] == '-')
    {
       state.verbose = 0;
    }

    return (valid == 1)?true:false;
}

int raspberryPIcamera::ChkCapturingState(){
	if (state.verbose)
	{
		GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [RPI Camera] State is '%d: %s'\n", state.bCapturing, raspicli_unmap_xref(state.bCapturing, initial_map, initial_map_size));
	}
	return state.bCapturing;
}

bool raspberryPIcamera::SetCapturingState(int iValue){
	bool bRst = false;
	const char* pNewState = raspicli_unmap_xref(iValue, initial_map, initial_map_size);

	if (pNewState == NULL){
		GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "E: [RPI Camera] No State match insert value '%d'\n", iValue);
		return bRst;
	}else{
		GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [RPI Camera] Change state to '%s'\n", pNewState);
		state.bCapturing = iValue;
	}

	if (mmal_port_parameter_set_boolean(camera_video_port, MMAL_PARAMETER_CAPTURE, state.bCapturing) != MMAL_SUCCESS)
	{
		// How to handle?
		bRst = false;
		GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [RPI Camera] Change state fail\n");
	}else{

		if (state.verbose)
		{
			if (state.bCapturing)
			   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [RPI Camera] Starting video capture\n");
			else
			   GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: [RPI Camera] Pausing video capture\n");
		}
		bRst = true;
	}

	return bRst;
}

void raspberryPIcamera::SetVFlip(int iValue)
{
    const RASPICAM_CAMERA_PARAMETERS *params = &state.camera_parameters;
    m_vflip = iValue;
	if (!state.camera_component){
		GLog( 1, tERRORTrace_MSK, "E: [RPI Camera] %s camera component not ready\n", __func__);
	}
	else {
		raspicamcontrol_set_flips(state.camera_component, m_hflip, m_vflip);
		GLog( 1, tDEBUGTrace_MSK, "D: raspberryPIcamera::SetVFlip >>> Rotation %d, hflip %s, vflip %s\n", params->rotation, params->hflip ? "Yes":"No",params->vflip ? "Yes":"No");
		raspicamcontrol_dump_parameters(&state.camera_parameters);
	}
}

void raspberryPIcamera::SetHFlip(int iValue)
{
    const RASPICAM_CAMERA_PARAMETERS *params = &state.camera_parameters;
    m_hflip = iValue;
	if (!state.camera_component){
		GLog( 1, tERRORTrace_MSK, "E: [RPI Camera] %s camera component not ready\n", __func__);
	}
	else {
		raspicamcontrol_set_flips(state.camera_component, m_hflip, m_vflip);
		GLog( 1, tDEBUGTrace_MSK, "D: raspberryPIcamera::SetHFlip >>> Rotation %d, hflip %s, vflip %s\n", params->rotation, params->hflip ? "Yes":"No",params->vflip ? "Yes":"No");
		raspicamcontrol_dump_parameters(&state.camera_parameters);
	}
}

int raspberryPIcamera::OpenCamera()
{
	if (bIsOpen != false){
		GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [RPI Camera] %s Component Still running\n", __func__);
	}

    // Our main data storage vessel..
    //RASPIVID_STATE state;
    int exit_code = EX_OK;
    MMAL_STATUS_T status = MMAL_SUCCESS;  //printf("W: [RPI Camera] %s 2881\n", __func__);
    bcm_host_init(); //printf("W: [RPI Camera] %s 2882\n", __func__);
    // Register our application with the logging system
    vcos_log_register("RaspiVid", VCOS_LOG_CATEGORY); //printf("W: [RPI Camera] %s 2884\n", __func__);

    state.timeout = 5000;
    //default_status(&state);

    // test CommandOutput
    //state.filename = (char*)malloc(30);
    //memset(state.filename, 0, 30);
    //strncpy(state.filename, "/home/pi/Temp/raspiout.h264", 27);

    // test CommandRaw
    //state.raw_output = 1;
    //state.raw_output_fmt = RAW_OUTPUT_FMT_YUV;
    //state.raw_filename = (char*)malloc((30));
    //memset(state.raw_filename, 0, 30);
    //strncpy(state.raw_filename, "/home/pi/Temp/raspiout.raw", 26);

    //FIXME
    //if ((state.width == 0)||(state.height ==0))
    if(0)
    {
    	state.width = 1920;
    	state.height = 1080;
    	state.bitrate = 1000000;
    	state.framerate = 15;
    	state.intraperiod = 15;
    }

    if (state.verbose)
    {
    	GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tDEBUGTrace_MSK, "D: %s Camera App %s\n\n", "raspberryPIcamera"/*basename(argv[0])*/, VERSION_STRING);
       dump_status(&state);
    }

    //printf("W: [RPI Camera] %s 2917\n", __func__);
    check_camera_model(state.cameraNum);

    // OK, we have a nice set of parameters. Now set up our components
    // We have three components. Camera, Preview and encoder.

    s_fpsH264EncodedFrame.Reset();
    if ((status = create_camera_component(&state)) != MMAL_SUCCESS)
    {
       vcos_log_error("%s: Failed to create camera component", __func__);
       exit_code = EX_SOFTWARE;
    }
    else if ((status = raspipreview_create(&state.preview_parameters)) != MMAL_SUCCESS)
    {
       vcos_log_error("%s: Failed to create preview component", __func__);
       destroy_camera_component(&state);
       exit_code = EX_SOFTWARE;
    }
    else if ((status = create_encoder_component(&state)) != MMAL_SUCCESS)
    {
       vcos_log_error("%s: Failed to create encode component", __func__);
       raspipreview_destroy(&state.preview_parameters);
       destroy_camera_component(&state);
       exit_code = EX_SOFTWARE;
    }
    else if (state.raw_output && (status = create_splitter_component(&state)) != MMAL_SUCCESS)
    {
       vcos_log_error("%s: Failed to create splitter component", __func__);
       raspipreview_destroy(&state.preview_parameters);
       destroy_camera_component(&state);
       destroy_encoder_component(&state);
       exit_code = EX_SOFTWARE;
    }
    else
    {
    	if(state.camera_component) {
    		GLog( 1, tDEBUGTrace_MSK, "D: >>> hflip %s, vflip %s\n", m_hflip ? "Yes":"No", m_vflip ? "Yes":"No");
    		raspicamcontrol_set_flips(state.camera_component, m_hflip, m_vflip);
    	}

       if (state.verbose)
          fprintf(stderr, "Starting component connection stage\n");
       //printf("W: [RPI Camera] %s 2953\n", __func__);

       camera_preview_port = state.camera_component->output[MMAL_CAMERA_PREVIEW_PORT];
       camera_video_port   = state.camera_component->output[MMAL_CAMERA_VIDEO_PORT];
       camera_still_port   = state.camera_component->output[MMAL_CAMERA_CAPTURE_PORT];
       preview_input_port  = state.preview_parameters.preview_component->input[0];
       encoder_input_port  = state.encoder_component->input[0];
       encoder_output_port = state.encoder_component->output[0];

       if (state.raw_output)
       {
          splitter_input_port = state.splitter_component->input[0];
          splitter_output_port = state.splitter_component->output[SPLITTER_OUTPUT_PORT];
          splitter_preview_port = state.splitter_component->output[SPLITTER_PREVIEW_PORT];
       }

       if (state.preview_parameters.wantPreview )
       {
          if (state.raw_output)
          {
        	 //printf("W: [RPI Camera] %s 2973\n", __func__);
             if (state.verbose)
                fprintf(stderr, "Connecting camera preview port to splitter input port\n");

             // Connect camera to splitter
             status = connect_ports(camera_preview_port, splitter_input_port, &state.splitter_connection);

             if (status != MMAL_SUCCESS)
             {
                state.splitter_connection = NULL;
                vcos_log_error("%s: Failed to connect camera preview port to splitter input", __func__);
                goto error;
             }

             if (state.verbose)
             {
                fprintf(stderr, "Connecting splitter preview port to preview input port\n");
                fprintf(stderr, "Starting video preview\n");
             }

             // Connect splitter to preview
             if (bIsPreview)
            	 status = connect_ports(splitter_preview_port, preview_input_port, &state.preview_connection);
             //printf("W: [RPI Camera] %s 2996\n", __func__);
          }
          else
          {
        	    //printf("W: [RPI Camera] %s 3000\n", __func__);
             if (state.verbose)
             {
                fprintf(stderr, "Connecting camera preview port to preview input port\n");
                fprintf(stderr, "Starting video preview\n");
             }

             // Connect camera to preview
             if (bIsPreview)
            	 status = connect_ports(camera_preview_port, preview_input_port, &state.preview_connection);
          }

          if (status != MMAL_SUCCESS)
             state.preview_connection = NULL;
          //printf("W: [RPI Camera] %s 3014\n", __func__);
       }
       else
       {
          if (state.raw_output)
          {
        	    //printf("W: [RPI Camera] %s 3020\n", __func__);
             if (state.verbose)
                fprintf(stderr, "Connecting camera preview port to splitter input port\n");

             // Connect camera to splitter
             status = connect_ports(camera_preview_port, splitter_input_port, &state.splitter_connection);

             if (status != MMAL_SUCCESS)
             {
                state.splitter_connection = NULL;
                vcos_log_error("%s: Failed to connect camera preview port to splitter input", __func__);
                goto error;
             }
             //printf("W: [RPI Camera] %s 3033\n", __func__);
          }
          else
          {
             status = MMAL_SUCCESS;
          }
       }

       if (status == MMAL_SUCCESS)
       {
    	    //printf("W: [RPI Camera] %s 3043\n", __func__);
          if (state.verbose)
             fprintf(stderr, "Connecting camera video port to encoder input port\n");

          // Now connect the camera to the encoder
          status = connect_ports(camera_video_port, encoder_input_port, &state.encoder_connection);

          if (status != MMAL_SUCCESS)
          {
             state.encoder_connection = NULL;
             vcos_log_error("%s: Failed to connect camera video port to encoder input", __func__);
             goto error;
          }
          //printf("W: [RPI Camera] %s 3056\n", __func__);
       }

       if (status == MMAL_SUCCESS)
       {
			Cheatfinderconfigmanager *pConfigContextManager = (Cheatfinderconfigmanager *)CHeatFinderUtility::GetConfigObj();
			T_CONFIG_FILE_OPTIONS *pConfigContext = ((pConfigContextManager == NULL)?NULL:pConfigContextManager->GetConfigContext());

           // Set up our userdata - this is passed though to the callback where we need the information.
           state.callback_data.pstate = &state;
           state.callback_data.abort = 0;

           //printf("W: [RPI Camera] %s 3065\n", __func__);
           if (state.raw_output)
           {
              splitter_output_port->userdata = (struct MMAL_PORT_USERDATA_T *)&state.callback_data;

              if (state.verbose)
                 fprintf(stderr, "Enabling splitter output port\n");

    	  	  if ((pConfigContext->iOpenStream & OPEN_CAMERA_STREAM) == 0)
    	  	  {
    	  		  GLog(1, tDEBUGTrace_MSK, "D: [RPI Camera][Gavin] ... vision capture RAW stop\n");
    	  	  }
    	  	  else
    	  	  {
				  // Enable the splitter output port and tell it its callback function
				  status = mmal_port_enable(splitter_output_port, splitter_buffer_callback);
				  GLog(1, tDEBUGTrace_MSK, "D: [RPI Camera][Gavin] ... vision capture RAW ready\n");
    	  	  }
              if (status != MMAL_SUCCESS)
              {
                 vcos_log_error("%s: Failed to setup splitter output port", __func__);
                 goto error;
              }
           }
           //printf("W: [RPI Camera] %s 3082\n", __func__);

           state.callback_data.file_handle = NULL;

           //if (state.filename)
           //{
           //   if (state.filename[0] == '-')
           //   {
           //      state.callback_data.file_handle = stdout;

                 // Ensure we don't upset the output stream with diagnostics/info
           //      state.verbose = 0;
           //   }
           //   else
           //   {
           //      state.callback_data.file_handle = open_filename(&state, state.filename);
           //   }

           //   if (!state.callback_data.file_handle)
           //   {
                 // Notify user, carry on but discarding encoded output buffers
           //      vcos_log_error("%s: Error opening output file: %s\nNo output file will be generated\n", __func__, state.filename);
           //   }
           //}

           state.callback_data.imv_file_handle = NULL;

           //if (state.imv_filename)
           //{
           //   if (state.imv_filename[0] == '-')
           //   {
           //      state.callback_data.imv_file_handle = stdout;
           //   }
           //   else
           //   {
           //      state.callback_data.imv_file_handle = open_filename(&state, state.imv_filename);
           //   }

           //   if (!state.callback_data.imv_file_handle)
           //   {
                 // Notify user, carry on but discarding encoded output buffers
           //     fprintf(stderr, "Error opening output file: %s\nNo output file will be generated\n",state.imv_filename);
           //      state.inlineMotionVectors=0;
           //   }
           //}

           state.callback_data.pts_file_handle = NULL;

           //if (state.pts_filename)
           //{
           //   if (state.pts_filename[0] == '-')
           //   {
           //      state.callback_data.pts_file_handle = stdout;
           //   }
           //   else
           //   {
           //      state.callback_data.pts_file_handle = open_filename(&state, state.pts_filename);
           //      if (state.callback_data.pts_file_handle) /* save header for mkvmerge */
           //         fprintf(state.callback_data.pts_file_handle, "# timecode format v2\n");
           //   }

           //   if (!state.callback_data.pts_file_handle)
           //   {
                 // Notify user, carry on but discarding encoded output buffers
           //      fprintf(stderr, "Error opening output file: %s\nNo output file will be generated\n",state.pts_filename);
           //      state.save_pts=0;
           //   }
           //}

           state.callback_data.raw_file_handle = NULL;

           //if (state.raw_filename)
           //{
           //   if (state.raw_filename[0] == '-')
           //   {
           //      state.callback_data.raw_file_handle = stdout;
           //   }
           //   else
           //   {
           //      state.callback_data.raw_file_handle = open_filename(&state, state.raw_filename);
           //   }

           //   if (!state.callback_data.raw_file_handle)
           //   {
                 // Notify user, carry on but discarding encoded output buffers
           //      fprintf(stderr, "Error opening output file: %s\nNo output file will be generated\n", state.raw_filename);
           //      state.raw_output = 0;
           //   }
           //}

           //if(state.bCircularBuffer)
           //{
           //   if(state.bitrate == 0)
           //   {
           //      vcos_log_error("%s: Error circular buffer requires constant bitrate and small intra period\n", __func__);
           //      goto error;
           //   }
           //   else if(state.timeout == 0)
           //   {
           //      vcos_log_error("%s: Error, circular buffer size is based on timeout must be greater than zero\n", __func__);
           //      goto error;
           //   }
           //   else if(state.waitMethod != WAIT_METHOD_KEYPRESS && state.waitMethod != WAIT_METHOD_SIGNAL)
           //   {
           //      vcos_log_error("%s: Error, Circular buffer mode requires either keypress (-k) or signal (-s) triggering\n", __func__);
           //      goto error;
           //   }
           //   else if(!state.callback_data.file_handle)
           //   {
           //      vcos_log_error("%s: Error require output file (or stdout) for Circular buffer mode\n", __func__);
           //      goto error;
           //   }
           //   else
           //   {

                 int count = state.bitrate / 16;
                 GLog(1, tDEBUGTrace_MSK, "D: [RPI Camera][Gavin] state.bitrate = %d\n", state.bitrate);
                 GLog(1, tDEBUGTrace_MSK, "D:                     state.timeout = %d\n", state.timeout);
                 if( count <= sizeof(s_frameBuff) ) {
                	GLog(1, tDEBUGTrace_MSK, "D:                     count = %d <OK>\n", count);
                    state.callback_data.cb_buff = (char*)s_frameBuff;
					state.callback_data.cb_len = sizeof(s_frameBuff);
					state.callback_data.cb_wptr = 0;
					state.callback_data.cb_wrap = 0;
					state.callback_data.cb_data = 0;
					state.callback_data.iframe_buff_wpos = 0;
					state.callback_data.iframe_buff_rpos = 0;
					state.callback_data.header_wptr = 0;
                 } else {
                	 GLog(1, tERRORTrace_MSK, "E:                     count = %d is over frame buffer size:%d\n", count, sizeof(s_frameBuff));
                	 goto error;
                 }
           //   }
           //}

           // Set up our userdata - this is passed though to the callback where we need the information.
           encoder_output_port->userdata = (struct MMAL_PORT_USERDATA_T *)&state.callback_data;

           if (state.verbose)
              fprintf(stderr, "Enabling encoder output port\n");
           //printf("W: [RPI Camera] %s 3222\n", __func__);

 	  	   if((pConfigContext->iOpenStream & OPEN_CAMERA_STREAM)==0)
 	  	   {
 	  		   GLog(1, tDEBUGTrace_MSK, "D: [RPI Camera][Gavin] ... vision capture stop\n");
 	  	   }
 	  	   else
 	  	   {
 	  		   // Enable the encoder output port and tell it its callback function
 	  		   status = mmal_port_enable(encoder_output_port, encoder_buffer_callback);
 	  		   GLog(1, tDEBUGTrace_MSK, "D: [RPI Camera][Gavin] ... vision capture ready\n");
 	  	   }
           if (status != MMAL_SUCCESS)
           {
              vcos_log_error("Failed to setup encoder output");
              goto error;
           }


           //if (state.demoMode)
           //{
              // Run for the user specific time..
           //   int num_iterations = state.timeout / state.demoInterval;
           //   int i;

           //   if (state.verbose)
           //      fprintf(stderr, "Running in demo mode\n");

           //   for (i=0;state.timeout == 0 || i<num_iterations;i++)
           //   {
           //      raspicamcontrol_cycle_test(state.camera_component);
           //      vcos_sleep(state.demoInterval);
           //   }
           //}
           else
           {
               //printf("W: [RPI Camera] %s 3251\n", __func__);

               // Only encode stuff if we have a filename and it opened
               // Note we use the copy in the callback, as the call back MIGHT change the file handle
               //if (state.callback_data.file_handle)   //hamlet always running
               if (true)
               {
                  //int running = 1;

                  // Send all the buffers to the encoder output port
                  {
                     int num = mmal_queue_length(state.encoder_pool->queue);
                     int q =0;
                     for (q=0;q<num;q++)
                     {
                        MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(state.encoder_pool->queue);

                        if (!buffer)
                           vcos_log_error("Unable to get a required buffer %d from pool queue", q);

                        if (mmal_port_send_buffer(encoder_output_port, buffer)!= MMAL_SUCCESS)
                           vcos_log_error("Unable to send a buffer to encoder output port (%d)", q);
                     }
                  }
                  //printf("W: [RPI Camera] %s 3275\n", __func__);

                  // Send all the buffers to the splitter output port
                  if (state.raw_output) {
                      //printf("W: [RPI Camera] %s 3279\n", __func__);
                     int num = mmal_queue_length(state.splitter_pool->queue);
                     int q =0;
                     for (q = 0; q < num; q++)
                     {
                        MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(state.splitter_pool->queue);

                        if (!buffer)
                           vcos_log_error("Unable to get a required buffer %d from pool queue", q);

                        if (mmal_port_send_buffer(splitter_output_port, buffer)!= MMAL_SUCCESS)
                           vcos_log_error("Unable to send a buffer to splitter output port (%d)", q);
                     }
                     //printf("W: [RPI Camera] %s 3292\n", __func__);
                  }
                  //int initialCapturing=state.bCapturing;

                  //while (running)
                  {
                     // Change state

                      //printf("W: [RPI Camera] %s 3300\n", __func__);
                     state.bCapturing = !state.bCapturing;

                     if (mmal_port_parameter_set_boolean(camera_video_port, MMAL_PARAMETER_CAPTURE, state.bCapturing) != MMAL_SUCCESS)
                     {
                        // How to handle?
                     }

                     //printf("W: [RPI Camera] %s 3308\n", __func__);
                     // In circular buffer mode, exit and save the buffer (make sure we do this after having paused the capture
                     //if(state.bCircularBuffer && !state.bCapturing)
                     //{
                     //   break;
                     //}

                     if (state.verbose)
                     {
                        if (state.bCapturing)
                           fprintf(stderr, "Starting video capture\n");
                        else
                           fprintf(stderr, "Pausing video capture\n");
                     }

                     //if(state.splitWait)
                     //{
                     //   if(state.bCapturing)
                     //   {
                     //      if (mmal_port_parameter_set_boolean(encoder_output_port, MMAL_PARAMETER_VIDEO_REQUEST_I_FRAME, 1) != MMAL_SUCCESS)
                     //      {
                     //         vcos_log_error("failed to request I-FRAME");
                     //      }
                     //   }
                     //   else
                     //   {
                     //      if(!initialCapturing)
                     //         state.splitNow=1;
                     //   }
                     //   initialCapturing=0;
                     //}
                     //running = wait_for_next_change(&state);
                  }

                  if (state.verbose)
                     fprintf(stderr, "Finished capture\n");

               }
               //else
               //{
               //   if (state.timeout)
               //      vcos_sleep(state.timeout);
               //   else
               //   {
                     // timeout = 0 so run forever
               //      while(1)
               //         vcos_sleep(ABORT_INTERVAL);
               //   }
               //}
           }
       }
       else
       {
           //printf("W: [RPI Camera] %s 3361\n", __func__);
          mmal_status_to_int(status);
          vcos_log_error("%s: Failed to connect camera to preview", __func__);
       }

       //if(state.bCircularBuffer)
       //{
       //   int copy_from_end, copy_from_start;

       //   copy_from_end = state.callback_data.cb_len - state.callback_data.iframe_buff[state.callback_data.iframe_buff_rpos];
       //   copy_from_start = state.callback_data.cb_len - copy_from_end;
       //   copy_from_start = state.callback_data.cb_wptr < copy_from_start ? state.callback_data.cb_wptr : copy_from_start;
       //   if(!state.callback_data.cb_wrap)
       //   {
       //      copy_from_start = state.callback_data.cb_wptr;
       //      copy_from_end = 0;
       //   }

       //   fwrite(state.callback_data.header_bytes, 1, state.callback_data.header_wptr, state.callback_data.file_handle);
       //   Save circular buffer
       //   fwrite(state.callback_data.cb_buff + state.callback_data.iframe_buff[state.callback_data.iframe_buff_rpos], 1, copy_from_end, state.callback_data.file_handle);
       //   fwrite(state.callback_data.cb_buff, 1, copy_from_start, state.callback_data.file_handle);
       //   if(state.callback_data.flush_buffers) fflush(state.callback_data.file_handle);
       //}

       if (status == MMAL_SUCCESS){
    	   bIsOpen = true;
           GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [RPI Camera] %s Success done\n", __func__);
           return mmal_status_to_int(status);
       }
error:
       mmal_status_to_int(status);

       if (state.verbose){
          //fprintf(stderr, "Closing down\n");
          GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "Closing down\n");
       }

       CloseCamera();
/*
       // Disable all our ports that are not handled by connections
       check_disable_port(camera_still_port);
       check_disable_port(encoder_output_port);
       check_disable_port(splitter_output_port);

       if (state.preview_parameters.wantPreview && state.preview_connection)
          mmal_connection_destroy(state.preview_connection);

       if (state.encoder_connection)
          mmal_connection_destroy(state.encoder_connection);

       if (state.splitter_connection)
          mmal_connection_destroy(state.splitter_connection);

       // Can now close our file. Note disabling ports may flush buffers which causes
       // problems if we have already closed the file!
       if (state.callback_data.file_handle && state.callback_data.file_handle != stdout)
          fclose(state.callback_data.file_handle);
       if (state.callback_data.imv_file_handle && state.callback_data.imv_file_handle != stdout)
          fclose(state.callback_data.imv_file_handle);
       if (state.callback_data.pts_file_handle && state.callback_data.pts_file_handle != stdout)
          fclose(state.callback_data.pts_file_handle);
       if (state.callback_data.raw_file_handle && state.callback_data.raw_file_handle != stdout)
          fclose(state.callback_data.raw_file_handle);

       // Disable components
       if (state.encoder_component)
          mmal_component_disable(state.encoder_component);

       if (state.preview_parameters.preview_component)
          mmal_component_disable(state.preview_parameters.preview_component);

       if (state.splitter_component)
          mmal_component_disable(state.splitter_component);

       if (state.camera_component)
          mmal_component_disable(state.camera_component);

       destroy_encoder_component(&state);
       raspipreview_destroy(&state.preview_parameters);
       destroy_splitter_component(&state);
       destroy_camera_component(&state);

       if (state.verbose)
          fprintf(stderr, "Close down completed, all components disconnected, disabled and destroyed\n\n");

*/
    }

    if (status != MMAL_SUCCESS)
       raspicamcontrol_check_configuration(128);

    return exit_code;
}

void raspberryPIcamera::CloseCamera()
{
    state.callback_data.cb_buff = NULL;
   // m_bStopThread = true;
   // if (m_hThread.IsValid())
   //     m_hThread.Destroy(2000);
    GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [RPI Camera] %s Start\n", __func__);
    check_disable_port(camera_still_port);
    check_disable_port(encoder_output_port);
    check_disable_port(splitter_output_port);
    //printf("W: [RPI Camera] %s 3463\n", __func__);
    if (state.preview_parameters.wantPreview && state.preview_connection)
       mmal_connection_destroy(state.preview_connection);
    //printf("W: [RPI Camera] %s 3466\n", __func__);
    if (state.encoder_connection)
       mmal_connection_destroy(state.encoder_connection);
    //printf("W: [RPI Camera] %s 3469\n", __func__);
    if (state.splitter_connection)
       mmal_connection_destroy(state.splitter_connection);
    //printf("W: [RPI Camera] %s 3472\n", __func__);
    // Can now close our file. Note disabling ports may flush buffers which causes
    // problems if we have already closed the file!
    //if (state.callback_data.file_handle && state.callback_data.file_handle != stdout)
    //   fclose(state.callback_data.file_handle);
    //if (state.callback_data.imv_file_handle && state.callback_data.imv_file_handle != stdout)
    //   fclose(state.callback_data.imv_file_handle);
    //if (state.callback_data.pts_file_handle && state.callback_data.pts_file_handle != stdout)
    //   fclose(state.callback_data.pts_file_handle);
    //if (state.callback_data.raw_file_handle && state.callback_data.raw_file_handle != stdout)
    //   fclose(state.callback_data.raw_file_handle);

    // Disable components
    if (state.encoder_component)
       mmal_component_disable(state.encoder_component);
    //printf("W: [RPI Camera] %s 3487\n", __func__);
    if (state.preview_parameters.preview_component)
       mmal_component_disable(state.preview_parameters.preview_component);
    //printf("W: [RPI Camera] %s 3490\n", __func__);
    if (state.splitter_component)
       mmal_component_disable(state.splitter_component);
    //printf("W: [RPI Camera] %s 3493\n", __func__);
    if (state.camera_component)
       mmal_component_disable(state.camera_component);
    //printf("W: [RPI Camera] %s 3496\n", __func__);
    destroy_encoder_component(&state); //printf("W: [RPI Camera] %s 3497\n", __func__);
    raspipreview_destroy(&state.preview_parameters); //printf("W: [RPI Camera] %s 3498\n", __func__);
    destroy_splitter_component(&state); //printf("W: [RPI Camera] %s 3499\n", __func__);
    destroy_camera_component(&state); //printf("W: [RPI Camera] %s 3500\n", __func__);

    if (state.verbose){
       //fprintf(stderr, "Close down completed, all components disconnected, disabled and destroyed\n\n");
       GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tERRORTrace_MSK, "W:Close down completed, all components disconnected, disabled and destroyed\n\n");
    }
    GLog(tVisionSrcTrace(LogManager::sm_glogDebugZoneSeed), tWARNTrace_MSK, "W: [RPI Camera] %s Finish\n", __func__);
    bIsOpen = false;

}

void raspberryPIcamera::dump_binary(char* pBuff, int nLen) {

    char szBuff[1024]={0};
    char szElement[16]={0};
    for(int i=0 ; i<nLen ; i++) {
        sprintf( szElement," %02X", pBuff[i] );
        strcat( szBuff, szElement );
    }
    GLog( tAll, tDEBUGTrace_MSK, "%s\n", szBuff );
}
