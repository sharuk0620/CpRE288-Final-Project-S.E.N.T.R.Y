/*
 * ping.c
 *
 *  Created on: Oct 28, 2026
 *      Author: ssharuk
 */

#include "timer.h"
#include <math.h>
#include "lcd.h"
//#include ""


volatile unsigned int risingTime = 0;
volatile unsigned int fallingTime = 0;
volatile unsigned char risingFlag = 0;
volatile unsigned char doneFlag = 0;
volatile unsigned char numOverflows = 0;

void TIMER3B_Handler(void);
void sendPulse(void);


//Initializes the timer module to be used with ping sensor
void ping_init(void){
    //GPIO INITIALIZATION
    SYSCTL_RCGCGPIO_R |= 0x00000002; //Turns on clock for port B
    SYSCTL_RCGCTIMER_R |= 0x00000008; //Turns on clock for TIMER 3 Module

    NVIC_EN1_R |= 0x00000010; //enable uart1 interrupts - page 104

    timer_waitMillis(0.5);

    /*
    //PB3 CONFIG (GPIO OUTPUT)
    GPIO_PORTB_DEN_R |= 0x00000008;
    GPIO_PORTB_DIR_R |= 0x00000008;
    GPIO_PORTB_AFSEL_R &= ~(0x00000008);
    */


    //TIMER CONFIG (USING TIMER A)
    TIMER3_CTL_R &= ~(0x00000100); // Disables TIMER3B
    timer_waitMillis(0.5);

    //Clears bits and sets configuration to separate timers
    TIMER3_CFG_R &= ~(0x00000007);
    TIMER3_CFG_R |= 0x00000004;

    //TIMER MODE Reg
    TIMER3_TBMR_R |= 0x00000007; //Sets Capture Mode (Bits 0,1) and Edge-Time (bit 2)
    TIMER3_TBMR_R &= ~(0x00000010); //Clears bit 4 for counting down

    //TIMER INTERVAL LOAD
    TIMER3_TBILR_R |= 0x0000FFFF; //Sets upper bound
    TIMER3_TBPR_R |= 0x000000FF; //Sets prescalar (last 8 bits) to all ones


    //INTERRUPT INITIALIZATION
//    TIMER3_IMR_R |= 0x00000400; //Enables timer interrupts
    TIMER3_CTL_R |= 0x00000C00; // Enables interrupts for both rising and falling edges

    TIMER3_CTL_R |= 0x00000100; // Re-Enables TIMER3B

    IntRegister(INT_TIMER3B, TIMER3B_Handler); //give the microcontroller the address of our interrupt handler - page 104 22 is the vector number
}

//Logic for pulsing the ping sensor.
int ping_read(void){
    risingTime = 0;
    fallingTime = 0;
    int pingDist = 0;
    int pulseWidth = 0;

    //PB3 CONFIG (GPIO OUTPUT)
     GPIO_PORTB_DEN_R |= 0x00000008;
     GPIO_PORTB_DIR_R |= 0x00000008;
     GPIO_PORTB_AFSEL_R &= ~(0x00000008);

    //sendPulse();

     //Mask Interrupt
     TIMER3_IMR_R &= ~(0x00000400); //Disables timer interrupts


     //Send 1st low pulse for 2 seconds
     GPIO_PORTB_DATA_R &= ~(0x00000008);
     //timer_waitMicros(10);

     //Sends the High Pulse
     GPIO_PORTB_DATA_R |= 0x00000008;
     timer_waitMicros(5);

     //Sends the 2nd Low Pulse
     GPIO_PORTB_DATA_R &= ~(0x00000008);

     //Set GPIO back to input
     //End sendPulse function

    //PB3 CONFIG (GPIO INPUT CAPTURE)
    GPIO_PORTB_DIR_R &= ~(0x00000008);
    GPIO_PORTB_AFSEL_R |= 0x00000008;
    GPIO_PORTB_PCTL_R |= 0x00007000;

    TIMER3_ICR_R |= 0x00000400; //Clears all timer interrupts
    TIMER3_IMR_R |= 0x00000400; //Enables timer interrupts

    risingFlag = 0;
    while(doneFlag == 0){}//Wait until the rising edge

    pulseWidth = (risingTime - fallingTime);

    //        return pulseWidth; //Exclusively Part 2

    //Begin Part 3 version
    if(pulseWidth != (pulseWidth & 0x00FFFFFF)){
        numOverflows++;
    }


    //Mathematics Behind Ping distance calculation
        //pingDist = (signal width (clock cycles))*(Clock period)*(Speed of sound)/2
        //            num clock cycles * seconds per clock cycle * cm per second
        //            (variables)      / 16,000,000Hz            * 34,000         /2 = x*.0010625
    pingDist = pulseWidth*0.0010625; //Part 3 Version

    doneFlag = 0;

    risingTime = 0;
    fallingTime = 0;

    //lcd_printf("Object dist: %dcm\nPulse Width: %d\nOverflow Count: %d",
               //pingDist,               pulseWidth,      numOverflows);

    return pingDist;

}

void TIMER3B_Handler(void){

    if(TIMER3_MIS_R & 0x00000400){
        TIMER3_ICR_R |= 0x00000400; //Clears all timer interrupts
        if((risingFlag == 0)){
            risingTime = ((TIMER3_TBR_R) | (TIMER3_TBPS_R<<16)) & 0x00FFFFFF;
            risingFlag = 1;
        }
        else if(risingFlag == 1){
            fallingTime = ((TIMER3_TBR_R) | (TIMER3_TBPS_R<<16)) & 0x00FFFFFF;//(TIMER3_TBR_R & 0x0000FFFF);// | (TIMER3_TBPS_R<<16)) & 0x00FFFFFF;

            doneFlag = 1;
            risingFlag = 0;
        }
    }

}

void sendPulse(void){

    //Mask Interrupt
    TIMER3_IMR_R &= ~(0x00000400); //Disables timer interrupts


    //Send 1st low pulse for 2 seconds
    GPIO_PORTB_DATA_R &= ~(0x00000008);
    //timer_waitMicros(10);

    //Sends the High Pulse
    GPIO_PORTB_DATA_R |= 0x00000008;
    timer_waitMicros(5);

    //Sends the 2nd Low Pulse
    GPIO_PORTB_DATA_R &= ~(0x00000008);

    //Set GPIO back to input

}
