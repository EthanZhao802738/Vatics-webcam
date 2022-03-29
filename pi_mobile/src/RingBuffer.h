/*
 * RingBuffer.h
 *
 *  Created on: Sep 4, 2018
 *      Author: markhsieh
 */

#ifndef SRC_RINGBUFFER_H_
#define SRC_RINGBUFFER_H_

#include "axclib.h"

typedef struct tagRingBuffElement
{
	axc_dword dwLength;
	axc_byte data[1];

} SRingBuffElement, *LPSRingBuffElement;

typedef struct tagValidBuffSize
{
	axc_dword dwSeg1Len;
	axc_dword dwSeg2Len;

} SValidBuffSize, *LPSValidBuffSize;

class CRingBuffer {
private:
	axc_byte* m_pBuffer;
	int m_totalSize;
	int m_readIdx;
	int m_writeIdx;

	pthread_mutex_t m_lock;

	int m_lastReadIdx;
	int m_lastPackLen4Pop;

	void GetValidBuffSize(SValidBuffSize& validBuffSize);
	void Append(const axc_dword dwLength, const axc_byte* pData, SValidBuffSize& validBuffSize);

	void GetValidDataSize(SValidBuffSize& validDataSize);
	axc_bool Pop(axc_byte* readBuff, axc_bool isPackLen = false);
	void PopCore(axc_dword dwLength, axc_byte* pData, SValidBuffSize& validDataSize);
	void UndoPop();

public:
	CRingBuffer(const int buffSize);
	virtual ~CRingBuffer();

	/* To push data into RingBuffer
	 * return: true  -> success push_back
	 * 		   false -> buffer full
	 */
	axc_bool push_back_len(const axc_dword dwLength, const axc_byte* pData);

	/* To pop data from RingBuffer
	 *
	 * pElement->dwLength should be the bytes of valid data buffer
	 * return: true  -> success pop_front
	 * 		   false -> data buffer NOT enough
	 */
	axc_bool pop_front(LPSRingBuffElement pElement);

	void Reset();
};

#endif /* SRC_RINGBUFFER_H_ */
