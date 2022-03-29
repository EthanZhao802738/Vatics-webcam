#include <string.h>
#include <axclib.h>
#include <dbm.h>
#include <constants.h>
#include <net.h>
#include <ulis_encode.h>

#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>


//#include "adlr064.h"

typedef struct main
{
    DWORD   dwRemoteIp;
    WORD    wRemotePort;
    WORD    wStreamMode;
    time_t  tLastPollingTime;
} T_UDP_SESSION;

enum __networkCard
{
    none = 0,
    eth0,
    wlan0,
} Network_Card;


//extern CAxcFifo g_fifoRawFrames;



static bool g_bStopNetThreads = false;

/* get value of "key = value\r\n" frome string */
bool get_header_value(const char *szHeader, const char *szKey, char *szValue);

/* local network - card mac -address (eth0) */
static CAxcString g_cszMacAddress;

/* video width & height */
static axc_word g_wVideoWidth = 0;
static axc_word g_wVideoHeight = 0;

/* get mac address */
axc_bool GetMacAddress(const char *szInterface, axc_byte abyMac[6]);

// for local recording
bool g_bLocalRecording = true;
void Record(axc_byte *abyPak, axc_i32 iPakSize);


// thread : read raw-frames from fifo, encode,send to udp/tcp (fifo)
static CAxcFifo g_fifoUdpFrames("M5s064_udpframes");
static CAxcFifo g_fifoTcpFrames("M5s064_tcpframes");
static CAxcThread g_threadEncode("M5s064_encoder");
static axc_dword thread_encoder(CAxcThread *pThread, void *pContext); 


/* thread : network broadcast (udp command) */
static CAxcSocketUdp g_sockUdpCommand("M5s064_updcommand");
static CAxcThread g_threadUdpCommand("M5s064_updcommand");
static axc_dword thread_udp_command(CAxcThread *pThread, void *pContext);

//thread : udp video sender
static CAxcSocketUdp g_sockUdpVideo("M5s064_udpvideo");
static T_UDP_SESSION g_UdpSessions[MAX_UDP_SESSION_NUMBER] = {{0,0,0,0},};
static CAxcThread g_threadUdpVideo("M5s064_udpvideo");
static axc_dword thread_udp_video(CAxcThread *pThread, void *pContext);

//thread : tcp listen
static CAxcSocketTcpListen g_sockTcpListen("M5s064_tcp_listen");
static CAxcThread g_threadTcpListen("M5s064_tcp_listen");
static CAxcList g_listTcpSession("M5s064_tcp_listen");
static axc_dword thread_tcp_listen(CAxcThread *pThread, void *pContext);

//thread : tcp sender
static CAxcThread g_threadTcpSend("M5s064_tcp_listen");
static axc_dword thread_tcp_video(CAxcThread *pThread, void *pContext);

void PrepareBroadcastData(char *szData, int len);
void TCPCommandHandler(char *szCommand, char *szRecvBuffer, axc_i32 iPacketSize);
int GetIpAddress( char *ip_str);
axc_bool PushFrameToEncodeFifo(CAxcFifo *fifo, const void *data, int len, axc_byte channel_index, int width, int height, axc_ddword ddwPts, axc_dword dwFrameSeq, axc_bool bIsKey);
#define ADE2CAM_CHANNEL_ULISRAW     (0)
#define ADE2CAM_CHANNEL_VISUALMAIN	(1)

//extern CAxcFifoBufferPtr g_fifoRawFrames;
CAxcFifoBufferPtr g_fifoRawFrames("M5s_rawfifo");

bool Net_Init()
{
    bool bResult = false;

    printf("Creating g_fifoRawFrames.....\n");
    if (!g_fifoRawFrames.Create(50)) {
        printf("[%s] Failed to create raw-frames-fifo\n");
        return false;
    } 
    else
        printf("g_fifoRawFrames success\n");


    do {
        g_bStopNetThreads = false;

        const axc_i32 iFifoPacketSize = 1600;                   // max network-packet size
        const axc_i32 iFifoTotalBytes = 2*1024*1024;   // 2 MB
        if (!g_fifoTcpFrames.Create(iFifoPacketSize, iFifoTotalBytes/iFifoPacketSize)) {
            printf("[%s] failed to create g_fifoTcpFrames, error: %s",__func__,AxcGetLastErrorText());
            break;
        }

        if (!g_threadEncode.Create(thread_encoder, NULL)) {
            printf("[%s] failed to create encoder-thread, error: %s",__func__,AxcGetLastErrorText());
            break;
        }
        
        // thread : network broadcast (udp command)  
        if (!g_sockUdpCommand.Create(UDP_COMMAND_PORT)) {
            printf("Sock Upd Command create failed\n");
            break;
        }

        printf("Socket Upd Command create successfully\n");

        if (!g_threadUdpCommand.Create(thread_udp_command, NULL)) {
            printf("Thread Udp Command created failed\n");
            break;
        }
    

        // creat tcp-listen socket
        if (!g_sockTcpListen.Create(TCP_LISTEN_PORT)) {
            printf("Failed to create tcp-listen socket\n");
            bResult = false;
            break;
        }
        if (!g_sockTcpListen.Listen()) {
            printf("Failed to start tcp-listen socket\n");
            bResult = false;
            break;
        }

        // create tcp-session-list
        if (!g_listTcpSession.Create(MAX_TCP_CONNECT_NUMBER)) {
            printf("Failed to create tcp-session-list\n");
            bResult = false;
            break;
        }

        // create tcp-listen thread
        if (!g_threadTcpListen.Create(thread_tcp_listen,NULL)) {
            printf("Failed to create tcp-listen thread\n");
            bResult = false;
            break;
        }

        // create tcp sender thread
        if (!g_threadTcpSend.Create(thread_tcp_video,NULL)) {
            printf("Failed to create sender thread\n");
            bResult = false;
            break;
        }

    bResult = true;

    } while(0);

    if (!bResult) {
        printf("Net Initial Failed\n");
        NET_Release();
    }

}


bool NET_Release()
{
    g_bStopNetThreads = true;
    g_sockUdpCommand.Destroy();
    g_threadUdpCommand.Destroy(10000);
    g_threadEncode.Destroy(10000);
    //
    g_sockTcpListen.Destroy();
    g_threadTcpListen.Destroy(10000);
    //
    g_threadTcpSend.Destroy(10000);

    CAxcTime::SleepMilliSeconds(1000);
    const axc_i32 iTcpSessionCount = g_listTcpSession.GetCount();
    for (axc_i32 i = iTcpSessionCount - 1; 0 <= i; i--)
    {
        CAxcSocketTcpSession *pSession = (CAxcSocketTcpSession*) g_listTcpSession.GetID(i);
        g_listTcpSession.Remove(i);
        if (pSession) {
            pSession->Destroy();
            delete pSession;
            pSession = NULL;
        }
    }
    g_listTcpSession.Destroy();
    g_fifoRawFrames.Destroy();
}



static axc_dword thread_udp_command(CAxcThread *pThread, void *pContext)
{
    char RecvBuffer[1600];
    char SendBuffer[1600];
    int iRecvLen = 0;
    uint32_t dwFromIp = 0;
    uint16_t wFromPort = 0;
    char szValue[256];
    char ipc[64];

     // waitting for get - local - mac - address
    char szMacAddress[64] = {""};
    axc_byte abyMac[6] = {0,0,0,0,0,0};

    while(!g_bStopNetThreads)
    {
        CAxcTime::SleepSeconds(1);
        if (GetMacAddress("eth0", abyMac) || GetMacAddress("enp1s0", abyMac)) {
            sprintf(szMacAddress, "%02X:%02X:%02X:%02X:%02X:%02X:", abyMac[0], abyMac[1], abyMac[2], abyMac[3], abyMac[4], abyMac[5]);
            printf("[%s] get local MAC:  %s\n", __func__, szMacAddress);
            g_cszMacAddress.Set(NULL);
            g_cszMacAddress.Append(szMacAddress);
            break;
        }
    }

    bool firstTime = true;

    while (!g_bStopNetThreads && g_sockUdpCommand.IsValid())
    {
            iRecvLen = g_sockUdpCommand.Recv(RecvBuffer, sizeof(RecvBuffer) -1, &dwFromIp, &wFromPort, axc_true);
            if (iRecvLen <= 0) {
                break;
            }
    

        //process command packet
        RecvBuffer[iRecvLen] = 0;
        SendBuffer[0] = 0;
        //if (get_header_value(RecvBuffer, "command=", szValue)) {
            PrepareBroadcastData(SendBuffer, sizeof(SendBuffer));

            /*if(0 == strcmp(szValue,  "polling")) {
                strcpy(SendBuffer, "command=replay_polling\r\n\r\n");
            }
            else if(0 == strcmp(szValue, "probe")) {
                PrepareBroadcastData(SendBuffer, sizeof(SendBuffer));
            }*/

            if (0 != SendBuffer[0]) {
                g_sockUdpCommand.Send(SendBuffer, strlen(SendBuffer) +1, dwFromIp, wFromPort);
            } 
        //}
    }
    printf("Out of thread_udp_command\n");
    return 1;
}


static axc_bool IsH264StartCode(axc_byte *pData)
{
    return (NULL != pData && 0x00 == pData[0] && 0x00 == pData[1] && 0x00 == pData[2] && 0x01 == pData[3]);
}


static  int FindH264Code(axc_byte *pData, axc_i32 iDataSize)
{
    if (iDataSize > 5) {
        for(axc_i32 i = 0; i < iDataSize - 4; i++)
        {
            if (IsH264StartCode(pData)) {
                return i;
            }
        }
    }
    return -1;
}



static axc_dword thread_encoder(CAxcThread *pThread, void *pContext)
{
    printf("[%s] begin\n",__func__);

    void *pRawData = NULL;
    axc_i32 iRawDataSize = 0;
    axc_ddword ddwRawDataContext = 0;
    CAxcMemory bufferEncode("M5s064_encoder");


    if (!bufferEncode.Create(MAX_ENCODE_FRAME_SIZE)) {
        printf("[%s] failed to create encoder-buffer",__func__);
        return 0;
    }

    axc_byte *pEncoderBuffer = (axc_byte*)bufferEncode.GetAddress();
    axc_dword dwFrameSeq = 0;
    

    while(!g_bStopNetThreads)
    {
        // lock buffer
        if (!g_fifoRawFrames.Peek(&pRawData, &iRawDataSize, NULL, &ddwRawDataContext)) {
            CAxcTime::SleepMilliSeconds(10);
            continue;
        }

        // encode and push to sender-fifo
        if (pRawData != NULL && iRawDataSize > 0 && ddwRawDataContext != 0) {

            axc_byte *ptr = (axc_byte*)pRawData;
            const axc_word w = (axc_word)((ddwRawDataContext >> 16) & 0xFFFF);
            const axc_word h = (axc_word)(ddwRawDataContext & 0xFFFF);

            //if(w != 0 && w <= MAX_WIDTH && h != 0 && h <= MAX_HEIGHT && iRawDataSize >= (axc_i32)(w*h*sizeof(short)))
            if (w != 0 && w <= MAX_WIDTH && h != 0 && h <= MAX_HEIGHT) {
                const double dbUseTime1 = CAxcTime::GetCurrentUTCTimeMs();

                int iEncodeFrameLen = UlisRawdataCompress((axc_byte*)pRawData, iRawDataSize, pEncoderBuffer, bufferEncode.GetBufferSize(), w , h, 8, 8, NULL,0); 

                if (iEncodeFrameLen > 0) {
                    // Decompress check : useless
                    /*if (0) {
                        static size_t s_nDecBufSize = w * h * sizeof(short);
                        static axc_byte *s_pDecBuf = (axc_byte*)malloc(s_nDecBufSize);
                        int iDecLen = UlisRawdataDecompress(pEncoderBuffer, iEncodeFrameLen, s_pDecBuf,(int)s_nDecBufSize,NULL, 0, NULL, NULL, NULL);
                        if (iDecLen != iRawDataSize) {
                            printf("[%s] failed verify codec : src - len %d, dec - le %d\n",__func__, iRawDataSize, iDecLen);
                        }
                        else if (memcmp(s_pDecBuf, pRawData, iDecLen) != 0) {
                            printf("[%s] failed verify codec : data not match\n",__func__);
                        }
                        else {
                            printf("[%s] verify codec OK\n",__func__);
                        }
                    }*/

                    // save video width & height
                    g_wVideoHeight = h;
                    g_wVideoWidth = w;

                    //  make header 
                    const axc_ddword ddwPTS = (axc_ddword)(CAxcTime::GetCurrentUTCTimeMs() * 1000);
                    dwFrameSeq++;

                    
                    // push to tcp fifo
                    if (!PushFrameToEncodeFifo(&g_fifoTcpFrames, pEncoderBuffer, iEncodeFrameLen, ADE2CAM_CHANNEL_ULISRAW, (int)w, (int)h, ddwPTS, dwFrameSeq, axc_true)) {
                        printf("[%s] failed to push thermal - frame to tcp fifo\n",__func__);
                    }
                }
            }
            else {
                // vision part
                const axc_ddword ddwPTS = (axc_ddword)(CAxcTime::GetCurrentUTCTimeMs() * 1000);
                /*time_t timep;
                time (&timep);
                printf("%s",asctime(gmtime(&timep)));*/
                dwFrameSeq++;
                /*int header_position;
                header_position = FindH264Code((axc_byte*)pRawData,iRawDataSize);
                printf("Header Position = %d\n",header_position);*/
                //PushFrameToEncodeFifo(&g_fifoTcpFrames, pRawData, iRawDataSize, ADE2CAM_CHANNEL_VISUALMAIN, (int)w, (int)h, ddwPTS, dwFrameSeq, 0);
            }
        }

    g_fifoRawFrames.Pop(axc_false);
    }

    bufferEncode.Free();
    return 1;
}



bool get_header_value(const char *szHeader, const char *szKey, char *szValue)
{
    const char *pszSrc = NULL;
    char *pszDest = szValue;
    const size_t nKeyLen = (NULL == szKey) ? (0) : (strlen(szKey));

    if (nKeyLen <= 0 || NULL == szHeader || NULL == szValue) {
        return false;
    }

    if (NULL == (pszSrc = strstr(szHeader, szKey))) {
        strcpy(szValue, "");
        return false;
    }

    pszSrc += nKeyLen;
    pszDest = szValue;

    while (*pszSrc != '\r' && *pszSrc != '\n' && *pszSrc != '\0') {
        *pszDest = *pszSrc;
        pszSrc++;
        pszDest++;
    }   
    *pszDest = '\0';
    return true; 
}




axc_bool GetMacAddress(const char *szInterface, axc_byte abyMac[6])
{
    int skfd = -1;
    memset(abyMac, 0, 6);
    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0) {
        int ret = 0;
        struct  ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strcpy(ifr.ifr_name , szInterface);
        ret = ioctl(skfd, SIOCGIFHWADDR, &ifr);                                                                // Ask Mac address in the user space
        shutdown(skfd, 2);
        close(skfd);
        if (ret >= 0) {
            memcpy(abyMac, (char*)&ifr.ifr_addr.sa_data, 6);
        }
    }
    if (0 == abyMac[0] && 0 == abyMac[1] && 0 == abyMac[2] && 0 == abyMac[3] && 0 == abyMac[4] && 0 == abyMac[5]) {
        return axc_false;
    }
    return axc_true;
    
}


void PrepareBroadcastData(char *szData, int len)
{
    static char szTemp[256];
    memset(szData, 0, len);

    char ip[INET_ADDRSTRLEN];
    static char undefined[] = "undefined";

    int type = GetIpAddress(ip);


    static char name[32] = {"Vatics-ADLR064"};

    static char version[16];

    strcat(szData, "command = node_info\r\n");
    strcat(szData, "factory=ADR\r\n");
    strcat(szData, "type=Vatics-M5s\r\n");
    sprintf(szTemp, "fw_version=%s %s %s\r\n", "1.0.0", __DATE__, __TIME__);

    strcat(szData, szTemp);
    sprintf(szTemp, "total_version=%s\r\n", version);

    strcat(szData, szTemp);
    sprintf(szTemp, "hw_id=pi/%s\r\n", g_cszMacAddress.Get());

    strcat(szData, szTemp);
    sprintf(szTemp, "eth0_mac=%s\r\n", g_cszMacAddress.Get());

    strcat(szData, szTemp);
    sprintf(szTemp, "mac=%s\r\n", g_cszMacAddress.Get());

    strcat(szData, szTemp);
    sprintf(szTemp,"eth0_ip=%s\r\n", ip);

    strcat(szData, szTemp);
    sprintf(szTemp, "device_label=%s\r\n", name);

    strcat(szData, szTemp);
    strcat(szData, "ch_num=1\r\n");
    strcat(szData, "ch1_type=video/ulis;\r\n");
    strcat(szData, "ch1_stream_num=1\r\n");
    sprintf(szTemp, "ch1_stream1_res=ulis/%dx%d\r\n",g_wVideoWidth,g_wVideoHeight);
    strcat(szData, szTemp);
    sprintf(szTemp, "ch1_stream1_get=udp/%d;tcp/%d;\r\n",UDP_VIDEO_PORT, TCP_LISTEN_PORT);
    sprintf(szTemp, "a.4.0.2_20210520= Vatics-M5s\r\n");
    strcat(szData, szTemp);
    strcat(szData, "\r\n");
}


// thread : tcp listen
static axc_dword thread_tcp_listen(CAxcThread *pThread, void *pContext)
{
    printf("thread tcp listen start\n");

    while(!g_bStopNetThreads && g_sockTcpListen.IsValid())
    {
        printf("Enter thread_tcp_listen while loop......\n");
        printf("\n");
        CAxcSocketTcpSession *pSession = g_sockTcpListen.Accept();
        printf("After Accept......\n");
        if (pSession == NULL) {
            CAxcTime::SleepMilliSeconds(100);
            printf("g_sockTcpListen doesn't accept\n");
            continue;
        }
        printf("create a new session......\n");
        // create a new session
        char szIP[64] = {""};
        CAxcSocket::IpToString(pSession->GetRemoteIp(),szIP);
        printf("thread tcp get ip : %s\n",szIP);
        axc_bool bSucceed = axc_false;
        const axc_ddword ddwId = (axc_ddword)pSession;
        const int iIndex = g_listTcpSession.Add(ddwId, 0);

        if (iIndex < 0) {
            printf("[%s] New Tcp-session: failed add to list, num = %d, new = 0x%11X(%p)",__func__, g_listTcpSession.GetCount(), ddwId, pSession);
        }
        else
        {
            printf("[%s] New Tcp-session: remote %s:%u connected, socket = %d, session_cnt = %d\n",
            __func__, szIP, pSession->GetRemotePort(), pSession->GetSocket(), g_listTcpSession.GetCount());

            bSucceed = axc_true;
        }

        if (!bSucceed && pSession != NULL) {
            delete pSession;
            pSession = NULL;
        }

        //CAxcTime::SleepMilliSeconds(100);
    }
    printf("Out while loop.....\n");
    return 1;
}



// thread: tcp listen
static axc_dword thread_tcp_video(CAxcThread *pThread, void *pContext)
{
    uint16_t PopPacket[1600];
    axc_byte Packet[1600];
    axc_i32 iPacketSize = 0;
    char szValue[256] = {""};

    axc_ddword ddwChannelIndex = 0;
    axc_ddword ddwImageRes = 0;
    
    printf("Thread_tcp_video start\n");

    
    while(!g_bStopNetThreads && g_fifoRawFrames.IsValid())
    {
        // receive from tcp-client
        for(axc_i32 i = g_listTcpSession.GetCount() -1; i >= 0; i--)
        {
            CAxcSocketTcpSession *pSession = (CAxcSocketTcpSession*) g_listTcpSession.GetID(i);

            if (pSession) {
                iPacketSize = pSession->Recv(Packet, (axc_i32)sizeof(Packet) - 1, 0);
                if (iPacketSize < 0) {
                    pSession->Destroy();
                    if (g_listTcpSession.Remove((axc_ddword)pSession) >= 0) {
                        char szIp[64] = {""};
                        CAxcSocket::IpToString(pSession->GetRemoteIp(), szIp);
                        printf("[%s] del Tcp-session:recv failed: remote %s:%u closed, socket = %d, session_cnt = %d\n",
                        __func__, szIp, pSession->GetRemotePort(), pSession->GetSocket(), g_listTcpSession.GetCount());

                        delete pSession;
                        pSession = NULL;
                    }
                }
                else if (iPacketSize > 0) {
                    printf("iPacketSize > 0\n ");
                    Packet[iPacketSize] = 0;
                    char *szRecvBuffer = (char*)Packet;

                    if (get_header_value(szRecvBuffer,"command=",szValue)) {
                        if (strcmp(szValue,"polling") == 0) {
                            if (get_header_value(szRecvBuffer,"open_stream=",szValue)) {
                                printf("szRecvBuffer = %s\n",szRecvBuffer);

                                const axc_bool bThermal = (strstr(szValue,"threamal") != NULL);
                                const axc_bool bVision = (strstr(szValue,"vision") != NULL);
                                void *pContext = g_listTcpSession.GetItem((axc_ddword)pSession);
                                if (bThermal && bVision) {
                                    pContext = (void*)(TCP_SESSION_CONTEXT_OPEN_THERMAL | TCP_SESSION_CONTEXT_OPEN_VISION);
                                }
                                else if (bThermal) {
                                    pContext = (void*)(TCP_SESSION_CONTEXT_OPEN_THERMAL);
                                }
                                else if (bVision) {
                                    pContext = (void*)(TCP_SESSION_CONTEXT_OPEN_VISION);
                                }
                                else {
                                    pContext = (void*)0;
                                }
                                g_listTcpSession.SetItem((axc_ddword)pSession, pContext);
                            }
                        }
                        TCPCommandHandler(szValue, szRecvBuffer, iPacketSize);
                    }

                }
            }
        }

        // copy data from fifo


        iPacketSize = 0;
        if (!g_fifoTcpFrames.Pop(Packet, (axc_i32)sizeof(Packet), &iPacketSize, &ddwChannelIndex, &ddwImageRes)) {   
            //printf("[%s] fifo is empty\n",__func__);
            CAxcTime::SleepMilliSeconds(10);
            continue;
        }
    
        //printf("[%s] send to all client\n",__func__);

        // send to all client
        for(axc_i32 i = g_listTcpSession.GetCount() - 1; i >= 0; i--)
        {
            CAxcSocketTcpSession *pSession = (CAxcSocketTcpSession*) g_listTcpSession.GetID(i);
            if (pSession) {
                void *pContext = (void*)g_listTcpSession.GetItem(i);
                axc_ddword ddwContext = (axc_ddword) pContext;
                
            //if( ADLR064_RAW == ddwChannelIndex && ( ddwContext & TCP_SESSION_CONTEXT_OPEN_THERMAL))
                if(1) {
                    const axc_dword dwRemoteIp   = pSession->GetRemoteIp();
                    const axc_dword wRemotePort = pSession->GetRemotePort();
                    const double dbSendBeginTime = CAxcTime::GetCurrentUTCTimeMs(); 
                    //printf("[%s] Send to the client, iPacketSize = %d\n",__func__,iPacketSize);
                    const axc_i32 iSendResult = pSession->Send(Packet,iPacketSize);
                  
                    if (iSendResult != iPacketSize) {
                        pSession->Destroy();
                        if (g_listTcpSession.Remove((axc_ddword)pSession) >= 0) {
                            char szIp[64] = {""};
                            CAxcSocket::IpToString(pSession->GetRemoteIp(), szIp);
                            printf("[%s] delete Tcp-session:  send failed\n");
                            delete pSession;
                            pSession = NULL;
                        }
                    }

                    const double dbUseSeconds = CAxcTime::GetCurrentUTCTimeMs() - dbSendBeginTime;
                    if (iSendResult != iPacketSize || dbUseSeconds >= 4.0) {
                        char szIp[64] = {""};
                        CAxcSocket::IpToString(dwRemoteIp, szIp);
                        printf("[%s] TCP use %.1fs send to %s:%u len %d result %d", __func__, dbUseSeconds, szIp, wRemotePort, iPacketSize, iSendResult);
                    }
                }   
            }
        }
    }
    printf("End of thread_tcp_video\n");
    return 1;
}



void TCPCommandHandler(char *szCommand, char *szRecvBuffer, axc_i32 iPacketSize)
{
    char szValue[256] = {0};

    if (strcmp(szCommand,"set_local_rec") == 0) {
        if (get_header_value(szRecvBuffer,"recording=",szValue)) {
            if (strcmp(szValue,"true") == 0) {
                printf("[TCPCmd] Local record start == in this version doesn't have log record ==");
                g_bLocalRecording = true;
            }
            else if (strcmp(szValue, "false") == 0) {
                printf("[TCPCmd] Local record end == in this version doesn't have log record ==");
                g_bLocalRecording = false;
            }
        }
    }
}


// Push encode video-frame to fifo
#define SPI_TX_PACKET_LEN                   (164)
#define SPI_TX_DATA_LEN                         (SPI_TX_PACKET_LEN - 4)
#define PACKET_NUMBER_PER_IPP      (5)
axc_bool PushFrameToEncodeFifo(CAxcFifo *fifo, const void *data, int len, axc_byte channel_index, int width, int height, axc_ddword ddwPts, axc_dword dwFrameSeq, axc_bool bIsKey)
{
    int j;
    uint8_t *ptr = (uint8_t*)data;
    /*for(j = 0; j < 5; j++)
    {
        printf("[%s] data[%d] = %x\n",__func__, j , ptr[j]);
    }*/


    if (data == NULL || len <= 0 || NULL == fifo || width <= 0 || height <= 0 ) {
        return axc_false;
    }
        

    if (channel_index != ADE2CAM_CHANNEL_ULISRAW  && channel_index != ADE2CAM_CHANNEL_VISUALMAIN) {
        return axc_false;
    }
        

    if (channel_index > 7)
        return axc_false;



    T_ADE2CAM_FRAME_HEADER Ade2camFrameHeader;
    memset(&Ade2camFrameHeader, 0, sizeof(Ade2camFrameHeader));
    if (ADE2CAM_CHANNEL_ULISRAW == channel_index) {
        memcpy(&Ade2camFrameHeader.CodecFourcc, "LRV1", 4);
    }
    else if (ADE2CAM_CHANNEL_VISUALMAIN == channel_index) {
        memcpy(&Ade2camFrameHeader.CodecFourcc, "H264",4);      
    }
     
    
    //printf("[%s] CodecFourcc = %d\n",__func__,Ade2camFrameHeader.CodecFourcc);
    //printf("\n");
    Ade2camFrameHeader.Width = width;
    //printf("[%s] Width = %d\n",__func__,width);
    Ade2camFrameHeader.Height = height;
    //printf("[%s] Height = %d\n",__func__,height);
    Ade2camFrameHeader.HeaderLen = sizeof(T_ADE2CAM_FRAME_HEADER);
    //printf("[%s] HeaderLen = %d\n",__func__,sizeof(T_ADE2CAM_FRAME_HEADER));
    Ade2camFrameHeader.FrameLen = (unsigned int) len;
    //printf("[%s] FrameLen = %d\n",__func__,len);
    Ade2camFrameHeader.FrameSeq = dwFrameSeq;
    //printf("[%s] FrameSeq = %d\n",__func__,dwFrameSeq);
    Ade2camFrameHeader.TimeHigh = (axc_dword)((ddwPts >> 32) & 0xFFFFFFFFULL);
    //printf("[%s] TimeHigh = %d\n",__func__,(axc_dword)((ddwPts >> 32) & 0xFFFFFFFFULL));
    Ade2camFrameHeader.TimeLow = (axc_dword)(ddwPts  & 0xFFFFFFFFULL);
    //printf("[%s] TimeLow = %d\n",__func__,(axc_dword)(ddwPts & 0xFFFFFFFFULL));
    Ade2camFrameHeader.FrameType = bIsKey ? 0 : 1;        // 0 - keyframe,  1 - non keyframe
    //printf("[%s] FrameType = %d\n",__func__,bIsKey);
    Ade2camFrameHeader.Crc16 = CAxcCRC::Make_CCITT(data, len);
    //printf("[%s] Crc16 = %d\n",__func__,Ade2camFrameHeader.Crc16);

    int packet_num = 0, i = 0;
    const unsigned char *src = (const unsigned char*) data;
    unsigned short crc = 0;

    unsigned char BigTxBuffer[SPI_TX_PACKET_LEN * PACKET_NUMBER_PER_IPP];  
    int iPacketNumberOfBigTxBuffer = 0;

    packet_num = (sizeof(T_ADE2CAM_FRAME_HEADER) + len + SPI_TX_PACKET_LEN -1) / SPI_TX_DATA_LEN;

    iPacketNumberOfBigTxBuffer = 0;
    unsigned char *TxBuffer = BigTxBuffer;
    for(i = 0; i < packet_num && len > 0; i++)
    {
        int txbuf_valid_len = SPI_TX_DATA_LEN;
        unsigned char *txdata = TxBuffer + 4;


        // first send frame-header
        if (i == 0) {
            memcpy(txdata, &Ade2camFrameHeader, sizeof(T_ADE2CAM_FRAME_HEADER));
            txdata += sizeof(T_ADE2CAM_FRAME_HEADER);
            txbuf_valid_len -= sizeof(T_ADE2CAM_FRAME_HEADER);
        }
        // send frame-data
        if (len < txbuf_valid_len) {
            memcpy(txdata, src, len);
            memset(txdata + len, 0xFF, txbuf_valid_len - len);
            src += len;
            len = 0;
        }
        else {
            memcpy(txdata, src, txbuf_valid_len);
            src += txbuf_valid_len;
            len -= txbuf_valid_len;
        }

        TxBuffer[0] = ((unsigned char)channel_index & 0x0F);
        TxBuffer[1] = 0xFF;
        TxBuffer[2] = 0;
        TxBuffer[3] = 0;

        crc = CAxcCRC::Make_CCITT(TxBuffer, SPI_TX_DATA_LEN);
        TxBuffer[2] = (unsigned char)(crc >> 8);
        TxBuffer[3] = (unsigned char)(crc & 0xFF);

        if (i == 0) {
            TxBuffer[0] |= 0x80;                // first pakcet
        }
        if ((packet_num - 1) == i) {
            TxBuffer[0] |= 0x40;                // last packet
        }

        TxBuffer += SPI_TX_PACKET_LEN;
        iPacketNumberOfBigTxBuffer++; 
        if (iPacketNumberOfBigTxBuffer >= PACKET_NUMBER_PER_IPP || (packet_num - 1) == i ) {
            fifo->Push(BigTxBuffer, SPI_TX_PACKET_LEN * iPacketNumberOfBigTxBuffer, (axc_ddword)channel_index);
            TxBuffer = BigTxBuffer;
            iPacketNumberOfBigTxBuffer = 0;
        }
    }
    
    return axc_true;
}


int GetIpAddress( char *ip_str)
{
   int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_addr.sa_family = AF_INET;

    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);

    ioctl(fd, SIOCGIFADDR, &ifr);

    close(fd);

    strcpy(ip_str, inet_ntoa(((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr));

    return 0;
}


