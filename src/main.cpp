#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <string.h>

#include <dbm.h>
#include <constants.h>
#include <main.h>
#include <version.h>
#include <adlr064.h>
#include <IMX307.h>
#include <net.h>

int g_bTerminate = 0;

static void sig_kill(int signo)
{
    g_bTerminate = 1;
    ADLR064_Release();
    IMX307_Release();
    NET_Release();
}



int main(int argc,char *argv[])
{

    bool b_ADLR064 = true;
    bool b_IMX307 = true;
    bool b_IMX307Nonblocking = false; 


    signal(SIGTERM, sig_kill);
	signal(SIGINT, sig_kill);

    // show version
    /*const char cszBuildTime[] = {__TIME__};
    const char cszBuildDate[] = {__DATE__};
    char szAppVersion[128] = {0};

    sprintf(szAppVersion,"%s,%s,%s",APP_VERSION,cszBuildDate,cszBuildTime);   
    bool bUpdateFWVersion = dbmSetValue("version","fw",szAppVersion);*/


    //b_IMX307 = IMX307_Init(b_IMX307Nonblocking);

    b_ADLR064 = ADLR064_Init();
    if (!b_ADLR064) {
        ADLR064_Release();
    }

    Net_Init();

    while(1) {
        CAxcTime::SleepMilliSeconds(100);
    }


   
    return 0;
}



