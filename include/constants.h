#ifndef _CONSTANTS_H
#define _CONSTANTS_H


#define ADLR064_FRAME_USHORT                    6423
#define FRAME_SIZE_UINT16                               6431
#define FRAME_SIZE_UINT8                                 (FRAME_SIZE_UINT16 * 2)
#define ADLR064_IMAGE_WIDTH                        80
#define ADLR064_IMAGE_HIGHT                         80



#define TCP_LISTEN_PORT         (5555)
#define UDP_COMMAND_PORT        (5556)
#define UDP_VIDEO_PORT          (5557)
#define MAX_TCP_CONNECT_NUMBER  (64)
#define MAX_UDP_SESSION_NUMBER  (64)

#define MAX_WIDTH               (80)
#define MAX_HEIGHT              (82)
#define MAX_ENCODE_FRAME_SIZE   (MAX_WIDTH * MAX_HEIGHT * 2)

#define TCP_SESSION_CONTEXT_OPEN_THERMAL    (0x1)       // tcp session: open stream for thermal
#define TCP_SESSION_CONTEXT_OPEN_VISION     (0x2)       // tcp session: open stream for vision


#define SPI_SPEED_10M             10000000

#define ADLR064_RAW                 (0)


#endif
