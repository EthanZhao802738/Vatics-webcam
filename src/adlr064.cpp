#include <axclib.h>
#include <constants.h>
#include <spi.h>
#include <algorithm>
#include <vector>

#define HEIGHT 80
#define WIDTH   80

// var
static bool g_deviceOpen = false;
static bool g_bStopCapture = false;
static int g_thml_UID1 = 0, g_thml_UID2 = 0, g_thml_UID3 = 0;
std::vector<uint16_t> g_frameCache;


// function
static CAxcThread g_threadADLR064Capture("M5s064_capture");
static axc_dword thread_adlr064Capture(CAxcThread *pThread, void *pContext);
extern CAxcFifoBufferPtr g_fifoRawFrames;


bool isCompleteFrame(int &idxStart, int &idxEnd);
bool g_bfifo_rawdata_frame;

// uart utility
double getTempOffset();
double m_fTempOffset;

bool ADLR064_Init()
{

    // init ADLR064 device

    if(!SpiOpenPort(SPI_SPEED_10M))
    {
        printf("Failed to open spi-dev\n");
        return false;
    }
    printf("Open spi port Successfully\n");
    g_deviceOpen = true;

    // create video-capture thread
    const uint32_t iFifoPacketSize = 13000;
    const uint32_t iFifoTotalBytes = 2 * 1024 *1024; 

    g_bfifo_rawdata_frame = true;

    

    // create vido-capture thread
    g_bStopCapture = false;
    printf("Creating thread_adlr064Capture.....\n");
    if(!g_threadADLR064Capture.Create(thread_adlr064Capture, NULL, 0, 12))
    {
        printf("[%s] Failed to create ADLR064 capture-thread\n");
        return false;
    }
    printf("thread_adlr064Capture Create Successfully\n");


    return true;
}

void ADLR064_Release()
{
    // close capture-thread
    printf("ADLR064_UnInit\n");
    g_bStopCapture = true;
    g_threadADLR064Capture.Destroy(10000);
    g_fifoRawFrames.Destroy();
}


// video-capture thread
axc_dword thread_adlr064Capture(CAxcThread *pThread, void *pContext)
{
    uint8_t iFrameBuffer[FRAME_SIZE_UINT8];
    uint32_t io_err_num = 0;
    uint32_t framecount = 0;
    //int iCount = 0; // TempOffset Frame count

    int iPacketSize;
    while(g_deviceOpen)
    {

        if( !spi_rx(iFrameBuffer, FRAME_SIZE_UINT8) )
        {
                printf("Spi reading Error\n");
                break;
        }

        // Byte order => MSB or LSB
        uint16_t *pFrameBuffer  = (uint16_t*)iFrameBuffer;
        
        for( int i = 0; i < FRAME_SIZE_UINT16; i++ )
        {
            pFrameBuffer[i] = swap_uint16t(pFrameBuffer[i]);
        }

        /*printf("pFrameBuffer[0] = %x\n",pFrameBuffer[2]);
        printf("pFrameBuffer[1] = %x\n",pFrameBuffer[3]);
        printf("pFrameBuffer[6430] = %x\n",pFrameBuffer[6429]);*/

     
        uint16_t  *pSrc = (uint16_t*)iFrameBuffer;
           
        for( int i = 0; i < ADLR064_IMAGE_WIDTH * ADLR064_IMAGE_HIGHT; i++)
        {
           pSrc[i] = (uint16_t)  (((( float )( pFrameBuffer[i + 2]) / 100.0 ) -273.15 ) * 10);
        }


        if(g_bfifo_rawdata_frame)
        {
            if (!g_fifoRawFrames.Push(iFrameBuffer, sizeof(iFrameBuffer), NULL, (( WIDTH & 0xFFFF) << 16) | ( HEIGHT & 0xFFFF)))
            {
                printf("[%s] Push to RawFifo failed\n",__func__);
            }
        }

        CAxcTime::SleepMilliSeconds(40);
    }
}



bool isCompleteFrame(int &idxStart, int &idxEnd)
{
    bool bRst  = false;
    if(g_thml_UID1 == 0  ||  g_thml_UID2 == 0 || g_thml_UID3 == 0)
        return bRst;

    std::vector<uint16_t>::iterator iterUID1 = g_frameCache.begin();
    std::vector<uint16_t>::iterator iterUID2 = g_frameCache.begin();
    std::vector<uint16_t>::iterator iterUID3 = g_frameCache.begin();

do
{
        iterUID1 = std::find(iterUID1, g_frameCache.end(), g_thml_UID1);
        if (iterUID1 == g_frameCache.end()) {
            bRst = false;
            break;
        }

        iterUID2 = std::find(iterUID1, g_frameCache.end(), g_thml_UID2);
        if (iterUID2 == g_frameCache.end()) {
            bRst = false;
            break;
        }

        iterUID3 = std::find(iterUID2, g_frameCache.end(), g_thml_UID3);
        if(iterUID3 == g_frameCache.end()) {
            bRst = false;
            break;
        }

        //check postion
        int idx = std::distance(g_frameCache.begin(), iterUID3);
        if ((iterUID3 - iterUID2) == 1 && (iterUID2 - iterUID1) == 1 && (idx - ADLR064_FRAME_USHORT + 1) >= 0)
            bRst = true;
        else
            ++iterUID1;

    } while ( !bRst );
    
    if (bRst) {
        idxStart = std::distance(g_frameCache.begin(), iterUID3) - ADLR064_FRAME_USHORT +1;
        idxEnd = std::distance(g_frameCache.begin(), iterUID3);
    }
    else {
        idxStart = -1;
        idxEnd = -1;
    }

    return bRst;
}




