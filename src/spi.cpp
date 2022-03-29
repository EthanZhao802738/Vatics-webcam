#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "spi.h"
#include "constants.h"


#define SPIDEV_0 "/dev/spidev0.0"
#define SPIDEV_1 "/dev/spidev1.0"

#define SPI_DELAY 0


int spi_cs0_fd = -1;
int spi_cs1_fd = -1;

unsigned char spi_mode = SPI_MODE_3;
unsigned char spi_bitPerWord = 8;
unsigned long spi_speed = 10000000;




#define MAX 12846
#define PORT 8080
#define SA struct sockaddr


static int spi_cs_fd = -1;


int SpiOpenPort(unsigned long useSpiSpeed)
{
    int status_value = -1;
    

    spi_mode = SPI_MODE_3;
    
    spi_bitPerWord = 8;

    spi_speed = useSpiSpeed;

    if(spi_cs_fd > 0)
    {
        return 1;
    }
    else
    {
      
        spi_cs_fd = open(SPIDEV_1, O_RDWR);

        if( spi_cs_fd < 0)
        {
            printf("Error - Couldn't open spi device\n");
            exit(1);
        }

        status_value = ioctl(spi_cs_fd, SPI_IOC_WR_MODE, &spi_mode);
    
        if(status_value < 0)
        {
            printf("Could not set SPIMode (WR).... iotcl fail\n");
            exit(1);
        }

        status_value = ioctl(spi_cs_fd, SPI_IOC_RD_MODE, &spi_mode);
    
        if(status_value < 0)
        {
            printf("Could not set SPIMode (RD).... iotcl fail\n");
            exit(1);
        }

        status_value = ioctl(spi_cs_fd, SPI_IOC_RD_BITS_PER_WORD, &spi_bitPerWord);
    
        if(status_value < 0)
        {
            printf("Could not set SPI bitPerWord (RD_BITS_PER_WORD).... iotcl fail\n");
            exit(1);
        }

        status_value = ioctl(spi_cs_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bitPerWord);
    
        if(status_value < 0)
        {
            printf("Could not set SPI bitPerWord (WR_BITS_PER_WORD).... iotcl fail\n");
            exit(1);
        }  

        status_value = ioctl(spi_cs_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed);
    
        if(status_value < 0)
        {
            printf("Could not set SPI speed (WR_MAX_SPEED_HZ).... iotcl fail\n");
            exit(1);
        }  

        status_value = ioctl(spi_cs_fd, SPI_IOC_RD_MAX_SPEED_HZ, &spi_speed);
    
        if(status_value < 0)
        {
            printf("Could not set SPI speed (RD_MAX_SPEED_HZ).... iotcl fail\n");
            exit(1);
        }  
    }

    printf("Successfully init Spi Device\n");
    return(1);
}

int SpiClosePort()
{
    int status_value = -1;

    status_value = close(spi_cs_fd);

    if(status_value < 0)
    {
        printf("Error - Could not close SPI Device\n");
        exit(1);
    }
    spi_cs_fd = -1;

    return(status_value);
}



int spi_run(void *rx, void *tx, int len)
{
    if(spi_cs_fd > 0 && (rx != NULL || tx != NULL) && len > 0)
    {
        struct spi_ioc_transfer connection;
        memset(&connection,0,sizeof(connection));
        connection.tx_buf = (unsigned long) tx;
        connection.rx_buf = (unsigned long) rx;
        connection.len = len;
        connection.delay_usecs = SPI_DELAY;
        
        if(ioctl(spi_cs_fd,SPI_IOC_MESSAGE(1), &connection) > 0)
        {
            return 1;
        }
    }
    return 0;
}

