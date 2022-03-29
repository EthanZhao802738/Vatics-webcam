#ifndef _MP4CONTAINER_H_
#define _MP4CONTAINER_H_

typedef struct {
	char		*szMP4File;
	unsigned int	dwVideoTrackNum;
	unsigned char	*ptVideoTrackBufInfo[2];
	unsigned int	dwAudioTrackNum;
	unsigned char	*ptAudioTrackBufInfo[2];
} MP4CreateOptions;

typedef struct MP4CHandle	MP4CHandle;

#ifdef __cplusplus
extern "C" {
#endif

MP4CHandle *MP4C_Init(void);
void MP4C_Release(MP4CHandle *ptHandle);
int MP4C_CreateFile(MP4CHandle *ptHandle, const MP4CreateOptions *ptOption);
int MP4C_AddSample(MP4CHandle *ptHandle, unsigned int dwTrackID, unsigned char *pucData, unsigned int uiDataLem, unsigned int uiSecs, unsigned int uiUSecs, unsigned int uiIsKeyFrame);
int MP4C_CloseFile(MP4CHandle *ptHandle);
int MP4C_CommitData(MP4CHandle *ptHandle);
int MP4C_FlushCache(MP4CHandle *ptHandle);

#ifdef __cplusplus
}
#endif

#endif
