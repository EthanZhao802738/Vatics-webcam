#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

//
//  Bit stream
//


typedef struct
{
    uint8_t *pBuffer;
    int iTotalBits;
    int iReadBits;
    int iWriteBits;
} T_BITSTREAM;

inline void Bitstream_Init(T_BITSTREAM *bs, const void *pBuffer, const int iBufferSize)
{
    memset(bs, 0, sizeof(T_BITSTREAM));
    bs->pBuffer = (uint8_t*) pBuffer;
    bs->iTotalBits = iBufferSize * 8;
    bs->iReadBits = 0;
    bs->iWriteBits = 0;
} 

inline unsigned int Bitstream_ReadBits(T_BITSTREAM *bs, const int iBits)
{
    unsigned int dwValue = 0;
    int iByteOffset = 0;
    int iBitOffset = 0;
    int i = 0;

    if(iBits <= 0 || iBits > 32 || (bs->iReadBits + iBits) > bs->iTotalBits)
        return 0;

    for(i = 0; i < iBits; i++)
    {
        iByteOffset = bs->iReadBits / 8;
        iBitOffset = bs->iReadBits % 8;

        dwValue <<= 1;
        dwValue |= (bs->pBuffer[iByteOffset] >> (7 - iBitOffset)) & 1;

        bs->iReadBits ++;
    }

    return dwValue;
}

inline int Bitstream_WriteBits(T_BITSTREAM *bs, const int dwValue, const int iBits)
{
    int iByteOffset = 0;
    int iBitOffset = 0;
    int i = 0;

    if(iBits <= 0 || iBits > 32 || (bs->iWriteBits + iBits) > bs->iTotalBits)
        return 0;


    for(i = 0; i < iBits; i++)
    {
        iByteOffset = bs->iWriteBits / 8;
        iBitOffset = bs->iWriteBits % 8;

        bs->pBuffer[ iByteOffset ] |= ((dwValue >> (iBits - i -1)) & 1) << (7 - iBitOffset);
        bs->iWriteBits ++;
    }
    return i;
}


#define MAX_MB_WIDTH        16
#define MAX_MB_HEIGHT      16

int UlisRawdataCompress(uint8_t *src_buf, int src_len, uint8_t *dst_buf, int dst_len, uint16_t srcimg_width, uint16_t srcimg_height, uint16_t mb_width, uint16_t mb_height, void * metadata_buf, int metadata_len)
{
    //printf("[%s] Start to Compress data\n",__func__);

    T_BITSTREAM bs;
    short *src16 = (short*) src_buf;
    int mb_xnum = 0, mb_ynum = 0, x = 0, y = 0;
    short mb_buffer[MAX_MB_HEIGHT * MAX_MB_WIDTH];
    int samples = 0;

    if (NULL == src_buf || src_len < 2 || dst_buf == NULL || dst_len < 2) {
        printf("[%s]  Compress data failed : 1\n",__func__);
        return 0;
    }

    if (srcimg_width <= 0 || srcimg_height <= 0 || mb_width <= 0 || mb_height <= 0) {
        printf("[%s]  Compress data failed : 2\n",__func__);
        return 0;
    }

    if (srcimg_width < mb_width || (srcimg_width % mb_width) != 0 || srcimg_height < mb_height || (srcimg_height % mb_height) != 0) {
            printf("[%s]  Compress data failed : 3\n",__func__);
            return 0;
    }
    
    if (mb_width > MAX_MB_WIDTH || mb_height > MAX_MB_HEIGHT)
    {
        printf("[%s]  Compress data failed : 4\n",__func__);
        return 0;
    }


    memset(dst_buf, 0, dst_len);
    Bitstream_Init(&bs, dst_buf, dst_len);

    Bitstream_WriteBits(&bs, 2, 4);
    Bitstream_WriteBits(&bs, srcimg_width, 16);
    Bitstream_WriteBits(&bs, srcimg_height, 16);
    Bitstream_WriteBits(&bs, mb_width, 8);
    Bitstream_WriteBits(&bs, mb_height, 8);

    mb_xnum = srcimg_width / mb_width;
    mb_ynum = srcimg_height / mb_height;
    samples = mb_width * mb_height;

    for(y = 0; y < mb_ynum; y++)
    {
        for(x =0; x < mb_xnum; x++)
        {
            short *psrc = src16 + (y * mb_height * srcimg_width) + (x * mb_width);
            short *pdst = mb_buffer;
            short val_min = 0x7FFF, val_max = 0;
            uint32_t val_range = 0;
            int max_bits = 1;
            int i = 0;
            for(i = 0; i < mb_height; i++, psrc += srcimg_width, pdst += mb_width)
            {
                memcpy(pdst, psrc, mb_width * sizeof(short));
            }

            for(i = 0; i < (mb_width * mb_height); i++)
            {
                if (mb_buffer[i] < val_min)
                    val_min = mb_buffer[i];
                
                if (mb_buffer[i] > val_max)
                    val_max = mb_buffer[i];
            }
            val_range = (unsigned int)(val_max - val_min);
            for(max_bits = 14; max_bits > 1; max_bits--)
            {
                if (val_range & (1 << (max_bits -1)))
                    break;
            }

            if (val_min >= 0)
            {
                Bitstream_WriteBits(&bs, val_min, 14);
            }
            else {
                unsigned short tmp = (unsigned short) ( -val_min);
                tmp &= 0x1FFF;
                tmp |= 0x2000;
                Bitstream_WriteBits(&bs, tmp, 14);
            }
            Bitstream_WriteBits(&bs, max_bits, 4);
            for(i = 0; i < samples; i++)
            {
                Bitstream_WriteBits(&bs, mb_buffer[i] - val_min, max_bits);
            }
        }
    }

    if (metadata_buf != NULL && (metadata_len > 0 && metadata_len < 256)) {
        uint8_t *mbuf = (uint8_t*) metadata_buf;
        int i = 0;
        Bitstream_WriteBits(&bs, metadata_len, 8);
        for( i = 0; i < metadata_len; i++, mbuf++)
        {
            Bitstream_WriteBits(&bs, *mbuf, 8);
        }
    }
    //printf("[%s] bs.iWriteBits return value = %d\n",__func__, (bs.iWriteBits + 7) / 8); 
    return (bs.iWriteBits + 7) / 8;
}



int UlisRawdataDecompress(uint8_t *src_buf, int src_len,uint8_t *dst_buf, int dst_len, void *dst_metadata_buf, int dst_metadata_buflen, int *dst_metadata_len, uint16_t *get_width, uint16_t *get_height)
{
    T_BITSTREAM bs;
    short *dst16 = (short*) dst_buf;
    short mb_buffer[MAX_MB_HEIGHT * MAX_MB_WIDTH];
    int version = 2;
    uint16_t img_width = 0, img_height = 0, mb_width = 0, mb_height = 0;
    int mb_xnum = 0, mb_ynum = 0;
    int samples =  0, total_samples = 0, x = 0, y = 0; 

    if (src_buf == NULL || src_len < 2 || dst_buf == NULL || dst_len < 2) {
        printf("[%s] failed at 1\n",__func__);
        return 0;
    }

    Bitstream_Init(&bs, src_buf, src_len);
    version = Bitstream_ReadBits(&bs, 4);
    if (version != 2) {
        printf("[%s] failed at 2, version = %d\n",__func__,version);
        return 0;
    } 

    img_width = Bitstream_ReadBits(&bs, 16);
    img_height = Bitstream_ReadBits(&bs, 16);
    mb_width = Bitstream_ReadBits(&bs, 8);
    mb_height = Bitstream_ReadBits(&bs, 8);

    if (img_width < mb_width || (img_width % mb_width) != 0 || img_height < mb_height || (img_height % mb_height) != 0 ) {
        printf("[%s] failed at 3\n",__func__);
        return 0;
    }
    
    if (mb_width > MAX_MB_WIDTH || mb_height > MAX_MB_HEIGHT) {
        printf("[%s] failed at 4\n",__func__);
        return 0;
    }

    if (get_width) {
        *get_width = (unsigned short) img_width;
    }
    if (get_height) {
        *get_height = (unsigned short) img_height;
    }

    mb_xnum = img_width / mb_width;
    mb_ynum = img_height / mb_height;
    samples = mb_width * mb_height;

    for(y = 0; y < mb_ynum; y++)
    {
        for(x = 0; x < mb_xnum; x++)
        {
            int i = 0;
            unsigned short tmp_min = 0;
            short val_min = 0;
            int max_bits = 1;
            short *psrc = mb_buffer;
            short *pdst = dst16 + ( y * mb_height * img_width) + ( x * mb_width);

            tmp_min = Bitstream_ReadBits(&bs, 14);
            if (tmp_min & 0x2000) {
                val_min = (short)(tmp_min & 0x1FFF);
                val_min = -val_min;
            }
            else {
                val_min = (short) tmp_min;
            }
            max_bits = Bitstream_ReadBits(&bs, 4);
            if (samples > (MAX_MB_HEIGHT * MAX_MB_WIDTH))
                return 0;

            for(i = 0; i < samples; i++)
            {
                mb_buffer[i] =  val_min + Bitstream_ReadBits(&bs, max_bits);
            }
            total_samples += samples;

            for( i = 0; i < mb_height; i++, psrc+=mb_width, pdst+=img_width)
            {
                memcpy(pdst, psrc, mb_width * sizeof(uint16_t));
            }
        }
    }  

    if (dst_metadata_buf != NULL && dst_metadata_buflen > 0 && dst_metadata_len != NULL) {
        const int mlen = (int) Bitstream_ReadBits(&bs, 8);
        if (mlen <= 0 || mlen > dst_metadata_buflen) {
            *dst_metadata_len = 0;
        }
        else {
            unsigned char *mbuf = (unsigned char*) dst_metadata_buf;
            int i = 0;

            for(i = 0; i < mlen; i++, mbuf++)
            {
                *mbuf = (unsigned char) Bitstream_ReadBits(&bs, 8);
            }
            *dst_metadata_len = mlen;
        }
    }
    return (total_samples * sizeof(short));
}