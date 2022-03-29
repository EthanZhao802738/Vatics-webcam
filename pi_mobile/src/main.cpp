#include "heatfinderapp.h"
#include "singletonprocess.h"

int main(int argc, char *argv[])
{
    SingletonProcess singleton(5544);
    if(!singleton())
    {
    	GLog( 1, tDEBUGTrace_MSK, "D: [SingletonProcess] process running already!\n" );
    	return -1;
    }

    CHeatFinderApp theApp;
    theApp.exec();

    return 0;
}
