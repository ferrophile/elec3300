#ifndef ADC_H
#define ADC_H

void adcInit();
void adcRead();
void adcStart();
#define adcInterrupt pir1.ADIF
#define adcInterruptHandler adcRead

#endif