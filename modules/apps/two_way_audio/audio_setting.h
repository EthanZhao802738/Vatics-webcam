#ifndef AUDIO_SETTING
#define AUDIO_SETTING

#include <getopt.h>
#include <pthread.h>

#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <audiotk/audio_capture_mmap.h>
#include <audiotk/audio_playback_mmap.h>

#include <audiotk/audio_vol_ctrl.h>
#include <audiotk/audio_common.h>

#include <audiotk/audio_encoder.h>
#include <audiotk/fdk_aac_processor.h> // For AAC4 decoder.
#include <g726/G726SDec.h> // For G711 decoder.
#include <g711/G711SDec.h> // For G711 decoder.

#include <MsgBroker/msg_broker.h>


#define PCM 3

#define TWOWAY_CMD_FIFO "/tmp/twoway/c0/command.fifo"


typedef struct
{
	// audio capture/playback setup params
	int model_type; //clinet or server
	char *pcm_name; //"hw:0,0" or "plug:vatics", default: "hw:0,0"
	int input_type;	//0: MicIn, 1: LineIn, default: 0
	
	// audio capture
	ATK_AUDIOCAP_CONFIG_T atk_capture_config;
	ATK_AUDIOCAP_HANDLE_T *atk_capture_handle;
//	unsigned char *atk_capture_bufs[ATK_AUDIO_MAX_CHANNELS];
	size_t atk_capture_buf_bytes;

	// audio playback
	ATK_AUDIOPLAY_HANDLE_T *atk_playback_handle;
	ATK_AUDIOPLAY_CONFIG_T atk_playback_config;
	unsigned char *atk_playback_bufs[ATK_AUDIO_MAX_CHANNELS];
	size_t atk_playback_buf_bytes;
	
	size_t period_frame_size;
	unsigned int playback_volume;

	// Audio codec
	unsigned int codec_type;	// 0: AAC4, 1: G711, 2: G726, 3: PCM
	unsigned int bitrate;
	size_t audiodec_in_buf_size;
	unsigned int aac4_stream_type;

	// audio encoder
	ATK_AUDIOENC_HANDLE_T *audioenc_handle[ATK_AUDIO_MAX_CHANNELS];
	ATK_AUDIOENC_ONEFRAME_CONF_T audioenc_out_frame_config[ATK_AUDIO_MAX_CHANNELS];	
	
	// network
	char* server_ip;
	int server_port; // default: 999
	int enable_noDelay;
	unsigned int transfer_type; // 0: UDP, 1: TCP
	
	// network sockets
	int server_sockfd; // initialize to -1 first before calling socket()
	int client_sockfd; // initialize to -1 first before calling socket()

	// network structures
	struct sockaddr_in server_sockaddr_in;
	struct sockaddr_in client_sockaddr_in;
	socklen_t sizeof_client_sockaddr_in; // shall be sizeof(client_sockaddr_in)

	// program control
	unsigned int terminate;
	
	// cellphone mode (udp remote server ip)
	char* udp_server_ip;	

	// for suspend and resume
	pthread_mutex_t sr_mutex;
	pthread_cond_t capture_cond;
	pthread_cond_t playback_cond;
	STATUS sr_status;
} twa_ctx;

//void print_ifaddrs();

typedef struct {
	unsigned char *buffer;
	int length;
	int start;
	int end;
} RingBuffer;


typedef enum {
	CLIENT_SITE = 1,
	SERVER_SITE = 2,	
} MODEL;


int twa_ctx_init(twa_ctx* ctx, MODEL type);
int twa_ctx_load_prog_args(int argc, char* argv[], twa_ctx* ctx);
int twa_ctx_audio_init(twa_ctx* ctx);
int twa_ctx_audio_release(twa_ctx* ctx);
int twa_ctx_network_init(twa_ctx* ctx);
int twa_ctx_network_release(twa_ctx* ctx);

int twa_ctx_start(twa_ctx* ctx);
int twa_ctx_stop(twa_ctx* ctx);



#endif
