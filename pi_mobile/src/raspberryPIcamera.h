/*
 * raspberryPIcamera.h
 *
 *  Created on: Feb 1, 2018
 *      Author: markhsieh
 *      Ref: hamlet
 *
 */

#ifndef RASPBERRYPICAMERA_H_
#define RASPBERRYPICAMERA_H_

/// \brief general
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <stdbool.h>
#include <mutex>
#include <vector>

/// \brief raspberry pi firmware support (Hardware: OpenMX-IL structure):
#include <bcm_host.h>
#include <interface/vcos/vcos.h>
#include <interface/mmal/mmal.h>
#include <interface/mmal/mmal_logging.h>
#include <interface/mmal/mmal_buffer.h>
#include <interface/mmal/util/mmal_util.h>
#include <interface/mmal/util/mmal_util_params.h>
#include <interface/mmal/util/mmal_default_components.h>
#include <interface/mmal/util/mmal_connection.h>
#include <interface/mmal/mmal_parameters_camera.h>

/// \brief raspberry pi camera capture solution: raspiXXX
#ifdef __cplusplus
extern "C" {
#endif
#include "RaspiCamControl.h"
#include "RaspiPreview.h"
#include "RaspiCLI.h"
#ifdef __cplusplus
}
#endif

/// \brief ADE utility tools
#include "axclib.h"
#include "FpsHelper.h" //gavin ++

/// \brief Defines
/// Capture/Pause switch method
/// Simply capture for time specified
#define WAIT_METHOD_NONE           0
/// Cycle between capture and pause for times specified
#define WAIT_METHOD_TIMED          1
/// Switch between capture and pause on keypress
#define WAIT_METHOD_KEYPRESS       2
/// Switch between capture and pause on signal
#define WAIT_METHOD_SIGNAL         3
/// Run/record forever
#define WAIT_METHOD_FOREVER        4

/// \brief Data Type & Value range Defines
/** Forward
 */
typedef struct RASPIVID_STATE_S RASPIVID_STATE;

/** Possible raw output formats
 */
typedef enum {
   RAW_OUTPUT_FMT_YUV = 1,
   RAW_OUTPUT_FMT_RGB = 2,
   RAW_OUTPUT_FMT_GRAY = 3
} RAW_OUTPUT_FMT;

/** Structure to cross reference H264 profile strings against the MMAL parameter equivalent
 *
 */
static XREF_T  profile_map[] =
{
   {(char *)"baseline",     MMAL_VIDEO_PROFILE_H264_BASELINE},
   {(char *)"main",         MMAL_VIDEO_PROFILE_H264_MAIN},
   {(char *)"high",         MMAL_VIDEO_PROFILE_H264_HIGH},
//FIXME:   {"constrained",  MMAL_VIDEO_PROFILE_H264_CONSTRAINED_BASELINE} // Does anyone need this?
};
static int profile_map_size = sizeof(profile_map) / sizeof(profile_map[0]);

/** Structure to cross reference H264 level strings against the MMAL parameter equivalent
 *
 */
static XREF_T  level_map[] =
{
   {(char *)"4",           MMAL_VIDEO_LEVEL_H264_4},
   {(char *)"4.1",         MMAL_VIDEO_LEVEL_H264_41},
   {(char *)"4.2",         MMAL_VIDEO_LEVEL_H264_42},
};

static int level_map_size = sizeof(level_map) / sizeof(level_map[0]);

//mark.hsieh fix order
static XREF_T  initial_map[] =
{
   {(char *)"record",     1},
   {(char *)"pause",      0},
};

static int initial_map_size = sizeof(initial_map) / sizeof(initial_map[0]);

static XREF_T  intra_refresh_map[] =
{
   {(char *)"cyclic",       MMAL_VIDEO_INTRA_REFRESH_CYCLIC},
   {(char *)"adaptive",     MMAL_VIDEO_INTRA_REFRESH_ADAPTIVE},
   {(char *)"both",         MMAL_VIDEO_INTRA_REFRESH_BOTH},
   {(char *)"cyclicrows",   MMAL_VIDEO_INTRA_REFRESH_CYCLIC_MROWS},
//FIXME:   {"random",       MMAL_VIDEO_INTRA_REFRESH_PSEUDO_RAND} Cannot use random, crashes the encoder. No idea why.
};

static int intra_refresh_map_size = sizeof(intra_refresh_map) / sizeof(intra_refresh_map[0]);

static XREF_T  raw_output_fmt_map[] =
{
   {(char *)"yuv",  RAW_OUTPUT_FMT_YUV},
   {(char *)"rgb",  RAW_OUTPUT_FMT_RGB},
   {(char *)"gray", RAW_OUTPUT_FMT_GRAY},
};

static int raw_output_fmt_map_size = sizeof(raw_output_fmt_map) / sizeof(raw_output_fmt_map[0]);

/// \brief Structure Defines
/** Capture/Pause switch method
 *  default composition
 */
//std::string strWAIT_METHOD_NONE("Simple capture");
//std::string strWAIT_METHOD_FOREVER("Capture forever");
//std::string strWAIT_METHOD_TIMED("Cycle on time");
//std::string strWAIT_METHOD_KEYPRESS("Cycle on keypress");
//std::string strWAIT_METHOD_SIGNAL("Cycle on signal");
static struct
{
   const char *description;
   int nextWaitMethod;
} wait_method_description[] =
{
      {(char *)"Simple capture",         WAIT_METHOD_NONE},
      {(char *)"Capture forever",        WAIT_METHOD_FOREVER},
      {(char *)"Cycle on time",          WAIT_METHOD_TIMED},
      {(char *)"Cycle on keypress",      WAIT_METHOD_KEYPRESS},
      {(char *)"Cycle on signal",        WAIT_METHOD_SIGNAL},
};
static int wait_method_description_size = sizeof(wait_method_description) / sizeof(wait_method_description[0]);

/** Struct used to pass information in encoder port userdata to callback
 */
typedef struct
{
   FILE *file_handle;                   /// File handle to write buffer data to.
   RASPIVID_STATE *pstate;              /// pointer to our state in case required in callback
   int abort;                           /// Set to 1 in callback if an error occurs to attempt to abort the capture
   char *cb_buff;                       /// Circular buffer
   int   cb_len;                        /// Length of buffer
   int   cb_wptr;                       /// Current write pointer
   int   cb_wrap;                       /// Has buffer wrapped at least once?
   int   cb_data;                       /// Valid bytes in buffer
#define IFRAME_BUFSIZE (60*1000)
   int   iframe_buff[IFRAME_BUFSIZE];          /// buffer of iframe pointers
   int   iframe_buff_wpos;
   int   iframe_buff_rpos;
   char  header_bytes[29];
   int  header_wptr;
   FILE *imv_file_handle;               /// File handle to write inline motion vectors to.
   FILE *raw_file_handle;               /// File handle to write raw data to.
   int  flush_buffers;
   FILE *pts_file_handle;               /// File timestamps
} PORT_USERDATA;


/** Structure containing all state information for the current run
 */
struct RASPIVID_STATE_S
{
   int timeout;                        /// Time taken before frame is grabbed and app then shuts down. Units are milliseconds
   int width;                          /// Requested width of image
   int height;                         /// requested height of image
   MMAL_FOURCC_T encoding;             /// Requested codec video encoding (MJPEG or H264)
   int bitrate;                        /// Requested bitrate
   int framerate;                      /// Requested frame rate (fps)
   int intraperiod;                    /// Intra-refresh period (key frame rate)
   int quantisationParameter;          /// Quantisation parameter - quality. Set bitrate 0 and set this for variable bitrate
   int bInlineHeaders;                  /// Insert inline headers to stream (SPS, PPS)


   char *filename;                     /// filename of output file

   int verbose;                        /// !0 if want detailed run information
   int demoMode;                       /// Run app in demo mode
   int demoInterval;                   /// Interval between camera settings changes
   int immutableInput;                 /// Flag to specify whether encoder works in place or creates a new buffer. Result is preview can display either
                                       /// the camera output or the encoder output (with compression artifacts)
   int profile;                        /// H264 profile to use for encoding
   int level;                          /// H264 level to use for encoding
   int waitMethod;                     /// Method for switching between pause and capture

   int onTime;                         /// In timed cycle mode, the amount of time the capture is on per cycle
   int offTime;                        /// In timed cycle mode, the amount of time the capture is off per cycle

   int segmentSize;                    /// Segment mode In timed cycle mode, the amount of time the capture is off per cycle
   int segmentWrap;                    /// Point at which to wrap segment counter
   int segmentNumber;                  /// Current segment counter
   int splitNow;                       /// Split at next possible i-frame if set to 1.
   int splitWait;                      /// Switch if user wants splited files

   RASPIPREVIEW_PARAMETERS preview_parameters;   /// Preview setup parameters
   RASPICAM_CAMERA_PARAMETERS camera_parameters; /// Camera setup parameters

   MMAL_COMPONENT_T *camera_component;    /// Pointer to the camera component
   MMAL_COMPONENT_T *splitter_component;  /// Pointer to the splitter component
   MMAL_COMPONENT_T *encoder_component;   /// Pointer to the encoder component
   MMAL_CONNECTION_T *preview_connection; /// Pointer to the connection from camera or splitter to preview
   MMAL_CONNECTION_T *splitter_connection;/// Pointer to the connection from camera to splitter
   MMAL_CONNECTION_T *encoder_connection; /// Pointer to the connection from camera to encoder

   MMAL_POOL_T *splitter_pool; /// Pointer to the pool of buffers used by splitter output port 0
   MMAL_POOL_T *encoder_pool; /// Pointer to the pool of buffers used by encoder output port

   PORT_USERDATA callback_data;        /// Used to move data to the encoder callback

   int bCapturing;                     /// State of capture/pause
   int bCircularBuffer;                /// Whether we are writing to a circular buffer

   int inlineMotionVectors;             /// Encoder outputs inline Motion Vectors
   char *imv_filename;                  /// filename of inline Motion Vectors output
   int raw_output;                      /// Output raw video from camera as well
   RAW_OUTPUT_FMT raw_output_fmt;       /// The raw video format

   char *raw_filename;                  /// Filename for raw video output
   //hamlet
   //bool bRawOutput;                     // for raw callback
   int cameraNum;                       /// Camera number
   int settings;                        /// Request settings from the camera
   int sensor_mode;			            /// Sensor mode. 0=auto. Check docs/forum for modes selected by other values.
   int intra_refresh_type;              /// What intra refresh type to use. -1 to not set.
   int frame;
   char *pts_filename;
   int save_pts;
   int64_t starttime;
   int64_t lasttime;

   bool netListen;
};



/// \brief Call-back format & Setting interface
typedef void (*OnH264ReceivedEvent)(void *pContext, const axc_i32 size, axc_byte *pBuf);
typedef struct tagH264ReceivedEventHandler
{
    OnH264ReceivedEvent fnEvent;
    void *pContext;
}xH264ReceivedEventHandler;

typedef void (*OnRawReceivedEvent)(void *pContext, const axc_i32 Width, const axc_i32 Height, const axc_i32 RawFMT, const axc_i32 size, axc_byte *pBuf);
typedef struct tagRawReceivedEventHandler
{
    OnRawReceivedEvent fnEvent;
    void *pContext;
}xRawReceivedEventHandler;

#define DEF_FRAME_BUFFER_SIZE 256*1024
/// \brief Class
class raspberryPIcamera
{
public:
    raspberryPIcamera();
    ~raspberryPIcamera();
    int OpenCamera();
    void CloseCamera();

    // set enable detail information
    bool SetVerbose(bool bValue);
    // Time to run viewfinder/capture
    // Ensure that if previously selected a waitMethod we don't overwrite it
    bool SetTimeout(unsigned int iTimeout);
    // Run in demo mode - no capture <------ this should not be used in our program
    //bool SetDemoModeEnable();
    // Display preview image *after* encoding
    bool SetPreviewAfterEncEnable(bool bValue);
    // H264 inline headers
    // Insert inline headers (SPS, PPS) to stream  (-ih | --inline)
    bool SetInlineHeadersEnable(bool bValue);


    // set width & height > 0
    bool SetResolution(unsigned int iWidth, unsigned int iHeight);
    // set bitrate 1~100
    bool SetBitrate(unsigned int iBitrate);
    // set fps to record (Encoder)
    bool SetFramePerSecond(unsigned int iFPS);
    // set key frame (I-frame) rate (-g | --intra)
    bool SetIntraAndPeriodFrameRate(int iIPFPS);
    // set Raw data capturing enable
    bool SetRawCapturingbyCStr(bool bValue, const char* pcstrName);
    bool SetRawCapturingEable(bool bValue, RAW_OUTPUT_FMT eRawFmt);
    // set Quantisation parameter for H264/MPEG-4 Encoder
    // In particular, the quantization parameter QP regulates how much spatial detail is saved.
    // When QP is very small, almost all that detail is retained.
    // As QP is increased, some of that detail is aggregated so that the bit rate drops –
    // but at the price of some increase in distortion and some loss of quality.
    bool SetQuantisation(unsigned int iQP);
    bool SetQuantization(unsigned int iQP);
    // set H264 profile {"baseline", "main" or "high"}
    bool SetH264Profile(MMAL_VIDEO_PROFILE_T eVideoProfile); //(const char* pcsProfileName);
    // set sharpness (-sh | --sharpness :-100 ~100)
    bool SetSharpness(int iSharpness);
    // set brightness (-br | --brightness : 0 ~ 100)
    bool SetBrightness(int iBrightness);
    // set contrast (-co | --contrast: 0~-100)
    bool SetContrast(int iContrast);
    // set saturation (-sa | --saturation :-100 ~100)
    bool SetSaturation(int iSaturation);
    // set exposure (-ex | --exposure: off,auto,night,nightpreview,backlight,spotlight,sports,snow,beach,verylong,fixedfps,antishake,fireworks)
    bool SetExposurebyCStr(const char* pcstrName);
    bool SetExposure(MMAL_PARAM_EXPOSUREMODE_T tExposureMode);
    // set write balance (-awb | Set AWB mode)
    bool SetWriteBalancebyCStr(const char* pcstrName);
    bool SetWriteBalance(MMAL_PARAM_AWBMODE_T TAwbMode);
    // set awb color Red & Blue Gain value
    bool SetAutoWriteBalanceGain(float fR_gain, float fB_gain);
    // set image effect (-ifx | --imxfx: none,negative,solarise,sketch,denoise,emboss,oilpaint,hatch,gpen,pastel,watercolour,film,blur,saturation,colourswap,washedout,posterise,colourpoint,colourbalance,cartoon)
    bool SetImageViewEffectbyCStr(const char* pcstrName);
    bool SetImageViewEffect(MMAL_PARAM_IMAGEFX_T TimageFX);

    // set output encoder view (just for comparing with ... )
    bool SetOutputParameter(const char* pcstrPath, unsigned int iPathLength);

    void AddH264ReceivedEvent(OnH264ReceivedEvent fnEvent, void *pContext);
    void AddRawReceivedEvent(OnRawReceivedEvent fnEvent, void *pContext);

    bool IsRunning();

    int ChkCapturingState();
    bool SetCapturingState(int iValue);
    void SetVFlip(int iValue);
    void SetHFlip(int iValue);

protected:
    int m_hflip;
    int m_vflip;
    static raspberryPIcamera *m_pSelf;
    //CAxcThread m_hThread;
    //bool m_bStopThread;         //stop thread flag
    // std::mutex m_locker;

    std::vector<xH264ReceivedEventHandler> m_H264ReceivedEventList;
    std::vector<xRawReceivedEventHandler> m_RawReceivedEventList;

    int mmal_status_to_int(MMAL_STATUS_T status);
    //void signal_handler(int signal_number);  <--------------------------------------- Terminate this Operation by ~raspberryPIcamera()
    void display_valid_parameters(char *app_name);

    static void camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
    static void encoder_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
    static void splitter_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

    MMAL_STATUS_T create_camera_component(RASPIVID_STATE *state);
    void destroy_camera_component(RASPIVID_STATE *state);

    MMAL_STATUS_T create_splitter_component(RASPIVID_STATE *state);
    void destroy_splitter_component(RASPIVID_STATE *state);

    MMAL_STATUS_T create_encoder_component(RASPIVID_STATE *state);
    void destroy_encoder_component(RASPIVID_STATE *state);

    MMAL_STATUS_T connect_ports(MMAL_PORT_T *output_port, MMAL_PORT_T *input_port, MMAL_CONNECTION_T **connection);
    void check_disable_port(MMAL_PORT_T *port);

    void default_status(RASPIVID_STATE *state);
    void check_camera_model(int cam_num);
    void dump_status(RASPIVID_STATE *state);
    //int parse_cmdline(int argc, const char **argv, RASPIVID_STATE *state); <---------- Need rework from "Consumer type-key to Program interface" to "Function Event to Object Receive Interface"
    //FILE *open_filename(RASPIVID_STATE *pState, char *filename);  <------------------- We transmit the image data directly to the application form.
    void update_annotation_data(RASPIVID_STATE *state);
    int wait_for_next_change(RASPIVID_STATE *state);
    int pause_and_test_abort(RASPIVID_STATE *state, int pause);
    int parse_cmdline(int argc, const char **argv, RASPIVID_STATE *state);
    RASPIVID_STATE state;

    MMAL_PORT_T *camera_preview_port;
    MMAL_PORT_T *camera_video_port;
    MMAL_PORT_T *camera_still_port;
    MMAL_PORT_T *preview_input_port;
    MMAL_PORT_T *encoder_input_port;
    MMAL_PORT_T *encoder_output_port;
    MMAL_PORT_T *splitter_input_port;
    MMAL_PORT_T *splitter_output_port;
    MMAL_PORT_T *splitter_preview_port;

    bool bIsNeedRestart;
    bool bIsOpen;
    bool bIsPreview;
    //

    //static axc_dword thread_process(CAxcThread* pThread, void* pContext)
    //{
    //    raspberryPIcamera *pSender = reinterpret_cast<raspberryPIcamera*> (pContext);
    //    return pSender->thread_process();
    //}

    //axc_dword thread_process();

    char m_filename[MAX_PATH];

    //gavin ++ >>
    static void PushFrameBuffToFifo(unsigned char* pBuff, int nLength);
    static unsigned char s_frameBuff[];
    static int s_frameLen;
    static CFpsHelper s_fpsH264EncodedFrame;
    static void dump_binary(char* pBuff, int nLen);
    //gavin ++ <<
};

#endif /* RASPBERRYPICAMERA_H_ */
