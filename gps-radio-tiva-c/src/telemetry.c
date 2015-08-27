#include "telemetry.h"

#include <stdio.h>

#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>
#include <driverlib/adc.h>
#include <driverlib/rom.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>

void initializeTelemetry(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_1);
    ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 2, 1, ADC_CTL_CH6 | ADC_CTL_IE | ADC_CTL_END);
}

void getTelemetry(Telemetry* pTelemetry)
{
    uint32_t result[4]; // 2nd sequence has FIFO of size 4
    ADCSequenceEnable(ADC0_BASE, 2);
    ADCIntClear(ADC0_BASE, 2);
    ADCProcessorTrigger(ADC0_BASE, 2);
    while(!ADCIntStatus(ADC0_BASE, 2, false))
    {
    }
    ADCIntClear(ADC0_BASE, 2);
    ADCSequenceDataGet(ADC0_BASE, 2, result);
    if (pTelemetry)
    {
        pTelemetry->cpuTemperature = result[0];
        // 470K:180K voltage divider
        // Vadc = Vin * 18 / 65
        // result[1] = Vadc * 4096 / 3.3
        // Vin = (result[1] * 3.3 / 4096) * 65 / 18 = result[1] * 2145 / 737280
        // Vin [mV] = result[1] * 2145000 / 737280 = 17875 / 6144
        //  Cannot overflow, the maximum ADC reading yields numerator of ~17000000
        pTelemetry->voltage = (result[1] & 0xFFFU) * 17875U / 6144U;
    }
    ADCSequenceDisable(ADC0_BASE, 2);
}
