#ifndef _ULIS_ENCODE_H
#define _ULIS_ENCODE_H

int UlisRawdataCompress(uint8_t *src_buf, int src_len, uint8_t *dst_buf, int dst_len, uint16_t srcimg_width, uint16_t srcimg_height, uint16_t mb_width, uint16_t mb_height, void * metadata_buf, int metadata_len);
int UlisRawdataDecompress(uint8_t *src_buf, int src_len,uint8_t *dst_buf, int dst_len, void *dst_metadata_buf, int dst_metadata_buflen, int *dst_metadata_len, uint16_t *get_width, uint16_t *get_height);

#endif