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
        pTelemetry->voltage = result[1];
    }
    ADCSequenceDisable(ADC0_BASE, 2);
}
