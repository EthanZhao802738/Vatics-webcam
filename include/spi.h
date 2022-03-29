#ifndef SPI_H
#define SPI_H
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>


#define spi_rx(rx, rx_len)  spi_run(rx, NULL, rx_len)
#define spi_tx(tx, tx_len)   spi_run(NULL, tx, tx_len)
#define swap_uint16t(reg) (((reg >> 8) & 0x00FF) | ((reg << 8) & 0xFF00))



int SpiOpenPort(unsigned long spi_speed);
int SpiClosePort();
int spi_run(void *rx, void *tx, int len);



#endif