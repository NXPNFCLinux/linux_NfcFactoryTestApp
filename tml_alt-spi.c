/*
*         Copyright (c), NXP Semiconductors Caen / France
*
*                     (C)NXP Semiconductors
*       All rights are reserved. Reproduction in whole or in part is
*      prohibited without the written consent of the copyright owner.
*  NXP reserves the right to make changes without notice at any time.
* NXP makes no warranty, expressed, implied or statutory, including but
* not limited to any implied warranty of merchantability or fitness for any
*particular purpose, or that the use will not infringe any third party patent,
* copyright or trademark. NXP must not be liable for any loss or damage
*                          arising from its use.
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <tml.h>
#include <linux/spi/spidev.h>

#define SPI_BUS         "/dev/spidev0.0"
#define SPI_MODE	SPI_MODE_0;
#define SPI_BITS	8;
#define SPI_SPEED       5000000

#define PIN_INT         23
#define PIN_ENABLE      24

#define EDGE_NONE    0
#define EDGE_RISING  1
#define EDGE_FALLING 2
#define EDGE_BOTH    3

static int iEnableFd    = 0;
static int iInterruptFd = 0;

static int verifyPin( int pin, int isoutput, int edge ) {
    char buf[40];
    int hasGpio = 0;

    sprintf( buf, "/sys/class/gpio/gpio%d", pin );
    int fd = open( buf, O_RDONLY );
    if ( fd <= 0 ) {
        // Pin not exported yet
        if ( ( fd = open( "/sys/class/gpio/export", O_WRONLY ) ) > 0 ) {
            sprintf(buf, "%d", pin);
            if ( write( fd, buf, strlen(buf)) == strlen(buf)) {
                hasGpio = 1;
            }
            close( fd );
        }
    } else {
        hasGpio = 1;
        close( fd );
    }
    usleep(100000);
    if ( hasGpio ) {
        // Make sure it is an output
        sprintf( buf, "/sys/class/gpio/gpio%d/direction", pin );
        fd = open( buf, O_WRONLY );
        if ( fd > 0 ) {
            if ( isoutput ) {
                write(fd,"out",3);
                close(fd);

                // Open pin and make sure it is off
                sprintf( buf, "/sys/class/gpio/gpio%d/value", pin );
                fd = open( buf, O_RDWR );
                if ( fd > 0 ) {
                    write( fd, "0", 1 );
                    return( fd );  // Success
                }
            } else {
                write(fd,"in",2);
                close(fd);

                if(edge != EDGE_NONE) {
                    // Open pin edge control
                    sprintf( buf, "/sys/class/gpio/gpio%d/edge", pin );
                    fd = open( buf, O_RDWR );
                    if ( fd > 0 ) {
                        char * edge_str = "none";
                        switch ( edge ) {
                          case EDGE_RISING:  edge_str = "rising"; break;
                          case EDGE_FALLING: edge_str = "falling"; break;
                          case EDGE_BOTH:    edge_str = "both"; break;
                          default: break;
                        }
                        write( fd, edge_str, strlen(edge_str));
                        close(fd);
                    }
                }
                // Open pin
                sprintf( buf, "/sys/class/gpio/gpio%d/value", pin );
                fd = open( buf, O_RDONLY );
                if ( fd > 0 ) {
                    return( fd ); // Success
                }
            }
        }
    }
    return( 0 );
}

static int pnGetint( void ) {
    char buf[2];
    int len;
    if (iInterruptFd <= 0) return -1;
    lseek(iInterruptFd, SEEK_SET, 0);
    len = read(iInterruptFd, buf, 2);
    if (len != 2) return 0;
    return (buf[0] != '0');
}

static int SpiRead(int pDevHandle, char* pBuffer, int nBytesToRead) {
    int numRead = 0;
    struct spi_ioc_transfer spi[2];
    char buf = 0xFF;
    memset(spi, 0x0, sizeof(spi));
    spi[0].tx_buf = (unsigned long)&buf;
    spi[0].rx_buf = (unsigned long)NULL;
    spi[0].len = 1;
    spi[0].delay_usecs = 0;
    spi[0].speed_hz = SPI_SPEED;
    spi[0].bits_per_word = SPI_BITS;
    spi[0].cs_change = 0;
    spi[0].tx_nbits = 0;
    spi[0].rx_nbits = 0;
    spi[1].tx_buf = (unsigned long)NULL;
    spi[1].rx_buf = (unsigned long)pBuffer;
    spi[1].len = nBytesToRead;
    spi[1].delay_usecs = 0;
    spi[1].speed_hz = SPI_SPEED;
    spi[1].bits_per_word = SPI_BITS;
    spi[1].cs_change = 0;
    spi[1].tx_nbits = 0;
    spi[1].rx_nbits = 0;
    numRead = ioctl(pDevHandle, SPI_IOC_MESSAGE(2), &spi);
    if (numRead > 0) numRead -= 1;
    return numRead;
}

int tml_open(int * handle)
{
    unsigned char spi_mode = SPI_MODE;
    unsigned char spi_bitsPerWord = SPI_BITS;
    static unsigned int speed = SPI_SPEED;
    iInterruptFd = verifyPin(PIN_INT, 0, EDGE_RISING);
    iEnableFd = verifyPin(PIN_ENABLE, 1, EDGE_NONE);
    *handle = open(SPI_BUS, O_RDWR | O_NOCTTY);
    if((*handle <= 0) || (iInterruptFd <= 0) || (iEnableFd <= 0)) goto error;
    if(ioctl(*handle, SPI_IOC_WR_MODE, &spi_mode) < 0) goto error;
    if(ioctl(*handle, SPI_IOC_RD_MODE, &spi_mode) < 0) goto error;
    if(ioctl(*handle, SPI_IOC_WR_BITS_PER_WORD, &spi_bitsPerWord) < 0) goto error;
    if(ioctl(*handle, SPI_IOC_RD_BITS_PER_WORD, &spi_bitsPerWord) < 0) goto error;
    if(ioctl(*handle, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) goto error;

    return 0;

error:
    if (iEnableFd) close(iEnableFd);
    if (iInterruptFd) close(iInterruptFd);
    if (*handle) close(*handle);
    return -1;
}

void tml_close(int handle)
{
    if(iEnableFd) close(iEnableFd);
    if(iInterruptFd) close(iInterruptFd);
    if(handle) close(handle);
}

void tml_reset(int handle)
{
    if(iEnableFd) write(iEnableFd, "0", 1 );
    usleep(10 * 1000);
    if(iEnableFd) write(iEnableFd, "1", 1 );
    usleep(10 * 1000);
}

int tml_send(int handle, char *pBuff, int buffLen)
{
    struct spi_ioc_transfer spi;
    char tx_buf[257];
    char rx_buf[257] = {0};
    int ret;
    memset(&spi, 0x0, sizeof(spi));
    tx_buf[0] = 0x7F;
    memcpy(&tx_buf[1], pBuff, buffLen);
    spi.tx_buf = (unsigned long)tx_buf;
    spi.rx_buf = (unsigned long)rx_buf;
    spi.len = buffLen+1;
    spi.delay_usecs = 0;
    spi.speed_hz = SPI_SPEED;
    spi.bits_per_word = SPI_BITS;
    spi.tx_nbits = 0;
    spi.rx_nbits = 0;
    spi.cs_change = 0;
    ret = ioctl(handle, SPI_IOC_MESSAGE(1), &spi);
    if (rx_buf[0] != 0xFF) ret =0;

    PRINT_BUF(">> ", pBuff, ret);
    usleep(10 * 1000);
    return ret;
}

int tml_receive(int handle, char *pBuff, int buffLen)
{
    int numRead = 0;
    struct timeval tv;
    fd_set rfds;
    int ret;

    if(pnGetint())
    {
        FD_ZERO(&rfds);
        FD_SET(handle, &rfds);
        tv.tv_sec = 2;
        tv.tv_usec = 1;

        ret = select(handle+1, &rfds, NULL, NULL, &tv);
        if(ret <= 0) return 0;

        ret = SpiRead(handle, pBuff, 3);
        if (ret <= 0) return 0;
        numRead = 3;
        if(pBuff[2] + 3 > buffLen) return 0;

        ret = SpiRead(handle, &pBuff[3], pBuff[2]);
        if (ret <= 0) return 0;
        numRead += ret;

        PRINT_BUF("<< ", pBuff, numRead);
    }

    return numRead;
}

int tml_transceive(int handle, char *pTx, int TxLen, char *pRx, int RxLen)
{
    int NbBytes = 0;
    if(tml_send(handle, pTx, TxLen) == 0) return 0;
    while(NbBytes==0) NbBytes = tml_receive(handle, pRx, RxLen);
    return NbBytes;
}












