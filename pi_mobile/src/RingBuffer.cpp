/*
 * RingBuffer.cpp
 *
 *  Created on: Sep 4, 2018
 *      Author: markhsieh
 */

#include "globaldef.h"
#include "RingBuffer.h"

CRingBuffer::CRingBuffer(const int buffSize) :
	m_totalSize(buffSize)
{
	m_pBuffer = new axc_byte[m_totalSize];
	m_readIdx = -1;
	m_writeIdx = 0;

	m_lastPackLen4Pop = -1;

	pthread_mutex_init( &m_lock, NULL );
}

CRingBuffer::~CRingBuffer() {
	delete[] m_pBuffer;
}

void CRingBuffer::Reset()
{
	pthread_mutex_lock(&m_lock);
	m_writeIdx = 0;
	m_readIdx = -1;
	m_lastPackLen4Pop = -1;
	GLog(1, tDEBUGTrace_MSK, "Gavin: RESET RingBuffer\n" );
	pthread_mutex_unlock(&m_lock);
}

axc_bool CRingBuffer::push_back_len(const axc_dword dwLength, const axc_byte* pData)
{
	pthread_mutex_lock(&m_lock);
	SValidBuffSize sValidBuffSize;
	GetValidBuffSize(sValidBuffSize);
	if( dwLength + sizeof(dwLength) > sValidBuffSize.dwSeg1Len + sValidBuffSize.dwSeg2Len )
	{
		GLog(tAll, tDEBUGTrace_MSK, "Gavin: !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\nGavin: !!!!!!!      RingBuffer full      !!!!!!!\nGavin: !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n" );
		pthread_mutex_unlock(&m_lock);
		return false;
	}
	else
	{
		Append( sizeof(dwLength), (const axc_byte*)&dwLength, sValidBuffSize);
		Append( dwLength, pData, sValidBuffSize);
		pthread_mutex_unlock(&m_lock);
		return true;
	}
}

axc_bool CRingBuffer::pop_front(LPSRingBuffElement pElement)
{
	pthread_mutex_lock(&m_lock);
	int nReadLength = 0;
	if( Pop( (axc_byte*)&nReadLength, true ) )
	{
		if((int)pElement->dwLength < nReadLength)
		{
			GLog(tAll, tDEBUGTrace_MSK, "Gavin: RingBuffer pop_front failed [NOT enough buffer size] %d/%d !!!!!\n", nReadLength, pElement->dwLength );
			UndoPop();
			pthread_mutex_unlock(&m_lock);
			pElement->dwLength = nReadLength;
			return false;
		}
		else
		{
			pElement->dwLength = nReadLength;
			Pop( pElement->data );

			if(m_readIdx == m_writeIdx)
			{
				m_writeIdx = 0;
				m_readIdx = -1;
				m_lastPackLen4Pop = -1;
				GLog(0, tDEBUGTrace_MSK, "Gavin: RESET RingBuffer (inside)\n" );
			}
		}
	}
	else
	{
		// There is no data can be popped
		//
		pElement->dwLength = 0;
	}
	pthread_mutex_unlock(&m_lock);
	return true;
}

void CRingBuffer::GetValidBuffSize(SValidBuffSize& validBuffSize)
{
	if( m_writeIdx > m_readIdx )
	{
		validBuffSize.dwSeg1Len = m_totalSize - m_writeIdx;
		validBuffSize.dwSeg2Len = m_readIdx < 0 ? 0 : m_readIdx;
	}
	else if( m_writeIdx < m_readIdx )
	{
		validBuffSize.dwSeg1Len = m_readIdx - m_writeIdx - 1;
		validBuffSize.dwSeg2Len = 0;
	}
	else
	{
		validBuffSize.dwSeg1Len = 0;
		validBuffSize.dwSeg2Len = 0;
	}
	GLog(tRingBuffer, tDEBUGTrace_MSK, "Gavin:      RingBuffer GetValidBuffSize Total:%d W:%d R:%d, seg1(%d), seg2(%d)\n", m_totalSize, m_writeIdx, m_readIdx, validBuffSize.dwSeg1Len, validBuffSize.dwSeg2Len );
}

void CRingBuffer::Append(const axc_dword dwLength, const axc_byte* pData, SValidBuffSize& validBuffSize)
{
	axc_byte* pDest = m_pBuffer + m_writeIdx;
	if( validBuffSize.dwSeg1Len >= dwLength )
	{
		memcpy( pDest, pData, dwLength );
		m_writeIdx += dwLength;
		if(m_writeIdx >= m_totalSize)
			m_writeIdx = m_writeIdx - m_totalSize;
		validBuffSize.dwSeg1Len -= dwLength;

		GLog(tRingBuffer, tDEBUGTrace_MSK, "Gavin:      RingBuffer Append len:%d W:%d/%d R:%d/%d\n", dwLength, m_writeIdx, m_totalSize, m_readIdx, m_totalSize );
	}
	else
	{
		int len1 = dwLength - validBuffSize.dwSeg1Len;
		memcpy( pDest, pData, len1 );
		m_writeIdx = dwLength - len1;
		memcpy( m_pBuffer, pData + len1, m_writeIdx );

		validBuffSize.dwSeg1Len = m_readIdx - m_writeIdx - 1;
		validBuffSize.dwSeg2Len = 0;

		GLog(tRingBuffer, tDEBUGTrace_MSK, "Gavin: >>>> RingBuffer Append len:%d W:%d/%d R:%d/%d\n", dwLength, m_writeIdx, m_totalSize, m_readIdx, m_totalSize );
	}
}

void CRingBuffer::GetValidDataSize(SValidBuffSize& validDataSize)
{
	if( m_writeIdx > m_readIdx )
	{
		validDataSize.dwSeg1Len = m_writeIdx - (m_readIdx >= 0 ? m_readIdx : 0);
		validDataSize.dwSeg2Len = 0;
	}
	else if( m_writeIdx < m_readIdx )
	{
		validDataSize.dwSeg1Len = m_totalSize - m_readIdx;
		validDataSize.dwSeg2Len = m_writeIdx-1 >= 0 ? m_writeIdx-1 : 0;
	}
	else
	{
		validDataSize.dwSeg1Len = 0;
		validDataSize.dwSeg2Len = 0;
	}
}

axc_bool CRingBuffer::Pop(axc_byte* readBuff, axc_bool isPackLen)
{
	m_lastReadIdx = m_readIdx;
	SValidBuffSize sValidDataSize;
	GetValidDataSize(sValidDataSize);
	if(isPackLen)
	{
		if( sizeof(m_lastPackLen4Pop) > sValidDataSize.dwSeg1Len + sValidDataSize.dwSeg2Len )
		{
			GLog(0, tDEBUGTrace_MSK, "Gavin: RingBuffer empty !!!!!!!!!!!!!!\n" );
			m_lastPackLen4Pop = 0;
			return false;
		}
		else
		{
			PopCore(sizeof(m_lastPackLen4Pop), (axc_byte*)&m_lastPackLen4Pop, sValidDataSize);
			memcpy(readBuff, (const axc_byte*)&m_lastPackLen4Pop, sizeof(m_lastPackLen4Pop));
			return true;
		}
	}
	else
	{
		if(m_lastPackLen4Pop <= 0)
		{
			GLog(tAll, tDEBUGTrace_MSK, "Gavin: RingBuffer Pop -- No data --\n" );
		}
		else
		{
			PopCore(m_lastPackLen4Pop, readBuff, sValidDataSize);
		}
	}
	return true;
}

void CRingBuffer::UndoPop()
{
	m_readIdx = m_lastReadIdx;
}

void CRingBuffer::PopCore(axc_dword dwLength, axc_byte* pData, SValidBuffSize& validDataSize)
{
	if(m_readIdx < 0)
		m_readIdx = 0;
	axc_byte* pSrc = m_pBuffer + m_readIdx;
	if(validDataSize.dwSeg1Len >= dwLength)
	{
		memcpy( pData, pSrc, dwLength );
		m_readIdx += dwLength;
		if(m_readIdx >= m_totalSize)
			m_readIdx = m_readIdx - m_totalSize;
		validDataSize.dwSeg1Len -= dwLength;

		GLog(tRingBuffer, tDEBUGTrace_MSK, "Gavin:      RingBuffer PopCore len:%d W:%d/%d R:%d/%d\n", dwLength, m_writeIdx, m_totalSize, m_readIdx, m_totalSize );
	}
	else
	{
		int len1 = dwLength - validDataSize.dwSeg1Len;
		memcpy( pData, pSrc, len1 );
		m_readIdx = dwLength - len1;
		memcpy( pData + len1, m_pBuffer, m_readIdx );

		validDataSize.dwSeg1Len = m_writeIdx - m_readIdx;
		validDataSize.dwSeg2Len = 0;

		GLog(tRingBuffer, tDEBUGTrace_MSK, "Gavin: <<<< RingBuffer PopCore len:%d W:%d/%d R:%d/%d\n", dwLength, m_writeIdx, m_totalSize, m_readIdx, m_totalSize );
	}
}
