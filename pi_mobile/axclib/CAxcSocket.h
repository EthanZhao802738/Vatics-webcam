#ifndef _C_AXC_SOCKET_H_
#define _C_AXC_SOCKET_H_

#include "axclib_data_type.h"
#include "CAxcBaseResource.h"

//
// CAxcSocket的所有 IP, PORT 参数都是主机字节序（host ip/port）
// 比如： 
// IP地址 "127.0.0.1" 在内存中为 "0x01 0x00 0x00 0x7F"
// PORT "80" 在内存中为 "0x50 0x00"
//

class AXC_API CAxcSocket : public CAxcBaseResource
{
public:
	// 全局初始化。用于在Windows系统中对 WinSock 初始化。
	static axc_bool SocketInit();
	// 全局反初始化
	static void SocketUninit();

	// 生成IP地址
	static AXC_INLINE axc_dword MakeIp(axc_byte byIp1, axc_byte byIp2, axc_byte byIp3, axc_byte byIp4)
	{
		axc_dword dwResult = 0;
		axc_byte* pbyResult = (axc_byte*)&dwResult;
		pbyResult[3] = byIp1;
		pbyResult[2] = byIp2;
		pbyResult[1] = byIp3;
		pbyResult[0] = byIp4;
		return dwResult;
	}

	// 判断一个IP是否LAN地址
	static AXC_INLINE axc_bool IsLanIp(axc_dword dwHostIp)
	{
		// 在IP地址3种主要类型里，各保留了3个区域作为私有地址，其地址范围如下：
		// A类地址：10.0.0.0 ～ 10.255.255.255 
		// B类地址：172.16.0.0 ～ 172.31.255.255 
		// C类地址：192.168.0.0 ～ 192.168.255.255 
		axc_byte* pbyIp = (axc_byte*)&dwHostIp;
		return (
			(pbyIp[3] == 10) ||
			(pbyIp[3] == 172 && pbyIp[2] >= 16 && pbyIp[2] <= 31) ||
			(pbyIp[3] == 192 && pbyIp[2] == 168) ||
			(dwHostIp == 0x7F000001));
	}
	// 判断一个IP地址是否组播地址
	static AXC_INLINE axc_bool IsMulticastIp(axc_dword dwHostIp)
	{
		// 224.0.0.0 ~ 239.255.255.255
		axc_byte* pbyIp = (axc_byte*)&dwHostIp;
		return (pbyIp[3] >= 224 && pbyIp[3] <= 239);
	}

	// IP地址转换为字符串
	static axc_bool IpToString(const axc_dword dwIp, char* szOut);
	// 从字符串解析出IP地址
	// szIp： 可以是 ip-address 比如 "192.168.0.1"，也可以是域名 比如 "www.myhost.com"
	// bIsHostName： 如果需要解析的是域名，设为axc_true；如果仅仅解析IP地址，设为axc_false
	static axc_dword StringToIp(const char* szIp, axc_bool bIsHostName);

	// SOCKET选项的设置和读取
	static axc_bool SocketSetOpt(SOCKET s, axc_i32 level, axc_i32 optname, void* optval, axc_i32 optlen);
	static axc_bool SocketGetOpt(SOCKET s, axc_i32 level, axc_i32 optname, void* optval, axc_i32 optlen);
	// 设置SOCKET属性：发送buffer的大小
	static axc_bool SocketSetSendBufferSize(SOCKET s, axc_i32 nSendBufSize);
	static axc_i32  SocketGetSendBufferSize(SOCKET s);
	// 设置SOCKET属性：接收buffer的大小
	static axc_bool SocketSetRecvBufferSize(SOCKET s, axc_i32 nRecvBufSize);
	static axc_i32  SocketGetRecvBufferSize(SOCKET s);
	// 设置SOCKET属性：接收、发送的超时时间
	static axc_bool SocketSetSendTimeout(SOCKET s, axc_dword dwTimeoutMilliSeconds);
	static axc_bool SocketSetRecvTimeout(SOCKET s, axc_dword dwTimeoutMilliSeconds);
	// 设置SOCKET属性：广播
	static axc_bool SocketSetBroadcast(SOCKET s, axc_bool bBroadcast);
	// 设置SOCKET属性：组播
	static axc_bool SocketSetMulticast(SOCKET s, axc_dword dwMcastIp, axc_bool bAdd);
	// 设置SOCKET属性：TCP no-delay (for windows)
	static axc_bool SocketSetTcpNodelay(SOCKET s, axc_bool bNoDelay);
	// 获取SOCKET绑定的本机端口和IP
	static axc_bool SocketGetBindIpPort(SOCKET s, axc_dword* pdwIp, axc_word* pwPort);

	// 获取本机网卡信息
#pragma pack(8)
	typedef struct _T_NETCARD_INFO
	{
		char		szName[122];
		axc_byte	abyMac[6];
		axc_dword	dwIpAddr;
		axc_dword	dwMask;
		axc_dword	dwBroadcastAddr;
		axc_dword	dwReserve;
	} T_NETCARD_INFO;
#pragma pack()
	static axc_i32 GetLocalIpList(T_NETCARD_INFO* pInfoList, axc_i32 iListSize);

public:
	CAxcSocket(const char* szResourceName);
	virtual ~CAxcSocket();

	axc_bool CreateFrom(
		SOCKET s, // existed socket
		axc_bool bIsTcp); // axc_true - TCP socket, axc_false - UDP socket
	void Destroy();
	axc_bool IsValid() { return (INVALID_SOCKET != m_sock); }

	AXC_INLINE SOCKET GetSocket() { return m_sock; }
	AXC_INLINE axc_bool IsTcp() { return m_bIsTcp; }
	AXC_INLINE axc_dword GetBindIp() { return m_dwBindIp; }
	AXC_INLINE axc_word GetBindPort() { return m_wBindPort; }

	AXC_INLINE axc_ddword GetCreateTime() { return m_ddwCreateTime; }
	AXC_INLINE axc_ddword GetLastRecvTime() { return m_ddwLastRecvTime; }
	AXC_INLINE axc_ddword GetLastSendTime() { return m_ddwLastSendTime; }
	AXC_INLINE axc_ddword GetLastAliveTime() { return AXC_MAX3(m_ddwCreateTime, m_ddwLastRecvTime, m_ddwLastSendTime); }

protected:
	axc_bool Create(
		axc_bool bIsTcp, // axc_true - TCP, axc_false - UDP
		axc_word wBindLocalPort = 0,
		axc_dword dwBindLocalIp = 0,
		axc_i32 iSendBufferSize = 0,
		axc_i32 iRecvBufferSize = 0);

protected:
	SOCKET		m_sock;
	axc_bool	m_bIsTcp;
	axc_dword	m_dwBindIp;
	axc_word	m_wBindPort;
	axc_ddword	m_ddwCreateTime;
	axc_ddword	m_ddwLastRecvTime;
	axc_ddword	m_ddwLastSendTime;

public:
	// 向子类请求目前的状态信息
	// 子类需要继承这个函数，向axclib库报告自己的状态
	// 返回值： 0-正常， 1-有轻微错误， 2-有严重错误
	virtual axc_dword ReportResourceStatus(char* szStatusText, const axc_dword dwTextBufferSize) { szStatusText[0] = 0; return 0; }
};


class AXC_API CAxcSocketUdp : public CAxcSocket
{
public:
	CAxcSocketUdp(const char* szResourceName);
	virtual ~CAxcSocketUdp();

	// 创建 UDP socket
	axc_bool Create(
		axc_word wBindLocalPort = 0,
		axc_dword dwBindLocalIp = 0,
		axc_i32 iSendBufferSize = 0,
		axc_i32 iRecvBufferSize = 0)
	{
		return CAxcSocket::Create(axc_false, wBindLocalPort, dwBindLocalIp, iSendBufferSize, iRecvBufferSize);
	}

	// 发送到指定地址
	axc_i32 Send(const void* pData, axc_i32 iDataSize, axc_dword dwRemoteIP, axc_word wRemotePort);

	// 接收
	axc_i32 Recv(void* pRecvBuffer, axc_i32 iRecvBufferSize, axc_dword* pdwRemoteIp = NULL, axc_word* pwRemotePort = NULL, axc_bool bIsBlock = axc_true);
};


class AXC_API CAxcSocketTcpSession : public CAxcSocket
{
public:
	CAxcSocketTcpSession(const char* szResourceName);
	virtual ~CAxcSocketTcpSession();

	// 创建 TCP session socket; 通常被 CAxcSocketListen::Accept() 调用
	axc_bool CreateSession(SOCKET s, axc_dword dwRemoteIp, axc_word wRemotePort)
	{
		if (!CAxcSocket::CreateFrom(s, axc_true))
		{
			return axc_false;
		}
		m_dwRemoteIp = dwRemoteIp;
		m_wRemotePort = wRemotePort;
		return axc_true;
	}

	// 创建 TCP client socket
	axc_bool CreateClient(
		axc_word wBindLocalPort = 0,
		axc_dword dwBindLocalIp = 0,
		axc_i32 iSendBufferSize = 0,
		axc_i32 iRecvBufferSize = 0)
	{
		return CAxcSocket::Create(axc_true, wBindLocalPort, dwBindLocalIp, iSendBufferSize, iRecvBufferSize);
	}
	// TCP client 连接到 TCP server
	axc_bool ClientConnectToServer(axc_dword dwServerIp, axc_word wServerPort);
	// TCP client 通过代理服务器连接到 TCP server
	axc_bool ClientConnectToServerViaProxy(axc_dword dwServerIp, axc_word wServerPort, axc_dword dwProxyIp, axc_word wProxyPort, axc_dword dwTimeoutSeconds, char* szErrorText);

	AXC_INLINE axc_dword GetRemoteIp() { return m_dwRemoteIp; }
	AXC_INLINE axc_word GetRemotePort() { return m_wRemotePort; }

	// 发送数据
	axc_i32 Send(const void* pData, axc_i32 iDataSize);

	// 接收数据
	// iTimeoutMilliSeconds : <0 阻塞等待, 0 不等待, >0 等待指定的毫秒数
	axc_i32 Recv(void* pRecvBuffer, axc_i32 iRecvBufferSize, axc_i32 iTimeoutMilliSeconds = -1);
	
	// 循环接收，直到读取到指定长度的数据、或者发生错误、或者超时
	axc_i32 RecvLoop(void* pRecvBuffer, axc_i32 iReadSize, axc_i32 iTimeoutMilliSeconds = -1)
	{
		axc_i32 iRecvLen = 0;
		axc_i32 iRemainLen = iReadSize;
		char* pchReadPtr = (char*)pRecvBuffer;
		while (iRemainLen > 0)
		{
			iRecvLen = Recv(pchReadPtr, iRemainLen, iTimeoutMilliSeconds);
			if (iRecvLen <= 0)
			{
				return iRecvLen;
			}
			else if (iRecvLen > iRemainLen)
			{
				return -1;
			}
			pchReadPtr += iRecvLen;
			iRemainLen -= iRecvLen;
		}
		return iReadSize;
	}

protected:
	axc_dword m_dwRemoteIp;
	axc_word m_wRemotePort;
};


class AXC_API CAxcSocketTcpListen : public CAxcSocket
{
public:
	CAxcSocketTcpListen(const char* szResourceName);
	virtual ~CAxcSocketTcpListen();

	// 创建
	axc_bool Create(
		axc_word wBindLocalPort = 0,
		axc_dword dwBindLocalIp = 0)
	{
		return CAxcSocket::Create(axc_true, wBindLocalPort, dwBindLocalIp);
	}

	// 开始监听
	axc_bool Listen();

	// 接收一个accept请求
	CAxcSocketTcpSession* Accept();
};


#endif // _C_AXC_SOCKET_H_
