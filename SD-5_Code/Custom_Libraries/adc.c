/*
 * adc.c
 *
 *  Created on: Oct 21, 2026
 *      Author: ssharuk
 */

#include "timer.h"
#include "lcd.h"
#include <math.h>

//Initializes GPIO and ADC registers
void adc_init(void){

    timer_init();


    //GPIO REGISTER INITIALIZATION: WANT TO USE PB4
    SYSCTL_RCGCGPIO_R |= 0x00000002; //Turns on clock for port b
    SYSCTL_RCGCADC_R |= 0x00000001; //Enables ADC0 module

    GPIO_PORTB_DIR_R &= ~(0x00000010);
    GPIO_PORTB_DEN_R &= ~(0x00000010);
    GPIO_PORTB_AFSEL_R |= 0x00000010; //PB4 Using hardware
    GPIO_PORTB_AMSEL_R |= 0x00000010; //Turns on analog mode for pb4


    //ADC REGISTER INITIALIZATION
    ADC0_ACTSS_R &=  ~(0x00000008); //Turns off ADC0 SS3 sequencer
    ADC0_EMUX_R &= ~(0x0000F000); //Assign SS3 to trigger from writing to software (via PSSI)
    ADC0_SAC_R |= 0x00000004; //Turns on 16x hardware sampling mode.
    ADC0_SSMUX3_R |= 0x0000000A; //Set where we are sampling from (Ain 10)
    ADC0_SSCTL3_R |= 0x00000006; //Make sure 1st sample is one and only sample
    ADC0_ACTSS_R |=  0x00000008; //Turns on ADC0 SS3 sequencer


}

//Read a converted value
int adc_read(void){

    int value;

    //SAMPLING STARTS HERE!
    ADC0_PSSI_R |= 0x00000008;

    while ( (ADC0_RIS_R & 0x00000008)==0){
        //Polling-loop to keep waiting until sample converts
    }

    //Clears the interrupt flag to allow next sample to convert and throw raw int signal
    ADC0_ISC_R |= 0x00000008;

    value = ADC0_SSFIFO3_R & 0x00000FFF;


    return value;
}

//Convert an adc dig. value into a distance (cm)
float adc_convert(int adc_dig_val){

    //BOT 4: 205108.331 * (pow(readSample, -1.28376));


    return 205108.331 * (pow(adc_dig_val, -1.28376));
}

