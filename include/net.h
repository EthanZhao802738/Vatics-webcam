#ifndef _NET_H
#define _NET_H

typedef struct
{
    unsigned int                CodecFourcc;             // codec name, 'H264' / 'LRV1' / 'MJPG' / 'MLZO'
    unsigned int                FrameLen;                   // frame data bytes, not include this HEADER
    unsigned int                FrameSeq;                  //  frame seq number
    unsigned int                TimeHigh;                  //  UTC time
    unsigned int                TimeLow;                   //  milli-seconds
    unsigned short          Width;                          //  video width
    unsigned short          Height;                        //  video height
    unsigned short          Crc16;                          //  CCITT CRC 16bit
    unsigned char            FrameType;              //  0 - keyfame, 1 - non keyframe
    unsigned char            HeaderLen;              //  length of this HEADER
 } T_ADE2CAM_FRAME_HEADER;



bool Net_Init(void);
bool NET_Release(void);
axc_bool PushFrameToEncodeFifo(CAxcFifo *fifo, const void *data, int len, axc_byte channel_index, int width, int height, axc_ddword ddwPts, axc_dword dwFrameSeq, axc_bool bIsKey);

#endif
