#ifndef SPI_H
#define SPI_H

#define monInit spiInit
#define monRead spiRead
#define monReadWait spiReadWait
#define monWrite spiWrite
#define monInterrupt 0
#define monInterruptHandler

#define XFER_OK    0
#define XFER_RETRY 1

void spiInit();
char spiReadWait();
int spiRead(char *pSpiData);
void spiWrite(char spiData);

#endif