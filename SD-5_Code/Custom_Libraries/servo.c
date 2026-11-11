/*
 * servo.c
 *
 *  Created on: Nov 4, 2026
 *      Author: ssharuk
 */

#include "timer.h"
#include <math.h>
#include "button.h"
#include "lcd.h"

//Placeholder values for servo left and right counter value threshold!
//CHANGE THESE AFTER CALIBRATING
int servo_right_val = 7280; //0 Degrees
int servo_left_val = 35140; //180 Degrees

/* LIST OF BOT VALUES FOR REFERENCE:
 * Bot 2: Right: 6720, Left: 34960
 * Bot 24: Right 7280, Left: 35140
 *
 */


//Initializes the servo peripheral
void servo_init(void){
    //CLOCK Registers
    SYSCTL_RCGCGPIO_R |= 0x00000002; //Turns on clock for port B
    SYSCTL_RCGCTIMER_R |= 0x00000002; //Turns on clock for TIMER 1 Module

    timer_waitMicros(5);

    //GPIO Registers
    GPIO_PORTB_DEN_R |= 0x00000020; //Digitally Enable PB5
    GPIO_PORTB_DIR_R |= 0x00000020; //Configure PB5 as an OUTPUT
    GPIO_PORTB_AFSEL_R |= 0x00000020; //Configure PB5 to use Alt. Hardware
    GPIO_PORTB_PCTL_R |= 0x00700000; //Configure PB5 to use T1CCP1 (Timer 1B)

    //TIMER1B Setup
    TIMER1_CTL_R &= ~(0x00000100); // Disables TIMER1B
    TIMER1_CTL_R |= 0x00004000; // Inverts PWM (Low before match, high after)
    TIMER1_CFG_R &= ~(0x00000007); //Clears bits 0-2
    TIMER1_CFG_R |= 0x00000004; //Sets bits 0-2 to 4 to enable seperate timer mode (T1B is now a 16 bit timer)
    TIMER1_TBMR_R &= ~(0x00000015); //Clears bit 4 for counting down
    TIMER1_TBMR_R |= 0x0000000A; //Sets Timer Mode to Periodic Timer, sets PWM Mode w/ Alt. Mode Select

    TIMER1_TBILR_R |= 0x00003280; //Sets 0x7100 into ILR (last 16 bits) of 160,000
    TIMER1_TBPR_R |= 0x00000002; //Sets 0x02 into Prescaler (first 8 bits) of 160,000

    TIMER1_TBMATCHR_R = 0x00005171; //Sets 0x3280 into Match Register (last 16 bits) of 144,000
    TIMER1_TBPMR_R |= 0x00000000; //Sets 0x02 into  prescaler match register (first 8 bits) of 144,000

    TIMER1_CTL_R |= 0x00000100; //Enables Timer1B
}

//Moves the servo to the commanded degree value
int servo_move(float degrees){

    int counterMatch;

    counterMatch = round( ((servo_left_val - servo_right_val) / 180.0)  * degrees + servo_right_val);

    TIMER1_TBMATCHR_R = counterMatch;

    return counterMatch;

}


void servo_calibrate(void){

    char buttonPressed;
    char runFlag = 1;
    button_init();
    lcd_init();

    //First initialize servo to point at theoretical 0 degrees (not really)
    TIMER1_TBMATCHR_R = 0x00003E30;
    int curVal = TIMER1_TBMATCHR_R;

    while(runFlag){
        buttonPressed = button_getButton();

       switch(buttonPressed){
           case 4:
               curVal -= 20;
               TIMER1_TBMATCHR_R = curVal;
               lcd_clear();
               lcd_printf("%d", curVal);
               break;
           case 3:
               curVal += 20;
               TIMER1_TBMATCHR_R = curVal;
               lcd_clear();
               lcd_printf("%d", curVal);
               break;
           case 2:
               lcd_clear();
               lcd_printf("Exited Calibration");
               runFlag = 0;
               break;
       }
    }

}


void servo_position_control(void){

    button_init();
    lcd_init();

    char buttonPressed; //Int variable to store button being pressed
    char rotationFlag = 0; //0 is counterclockwise, 1 is clockwise

    //Variable to hold counter value for the degree the servo is point at
    //ASSUMING THAT SERVO IS ALREADY CALIBRATED
    int command;

    //Variable to hold the degree value to point
    float degreePoint = 90.0;
    command = servo_move(degreePoint);
    lcd_clear();


    while(1){

        //Gets button pressed
        buttonPressed = button_getButton();

        //Prints info to LCD
        lcd_printf("%.1f\n%d\n%d", degreePoint, command, rotationFlag);


        switch(buttonPressed){
            case 1: //Calls to move servo by 1 degree

                //If clockwise, decrement angle to point by 1
                if(rotationFlag){
                    degreePoint -= 1.0;

                } else {
                    degreePoint += 1.0; //Otherwise, increment by 1
                }

                //Max bounds to make sure servo doesn't point past these values
                if(degreePoint > 180.0){
                    degreePoint = 180.0;
                }
                if(degreePoint < 0.0){
                    degreePoint = 0.0;
                }

                //Commands servo to point at degreePoint
                command = servo_move(degreePoint);
                break;
            case 2: //Calls to move servo by 5 degrees

                //If clockwise, decrement angle by 5 degrees
                if(rotationFlag){
                    degreePoint -= 5.0;
                } else {
                    degreePoint += 5.0; //else increment it by 5
                }

                //Max bounds to make sure servo doesn't point past these values
                if(degreePoint > 180.0){
                    degreePoint = 180.0;
                }
                if(degreePoint < 0.0){
                    degreePoint = 0.0;
                }

                //Commands servo to point at degreePoint
                command = servo_move(degreePoint);
                break;


            case 3: //Toggles rotation flag from clockwise to counterclockwise
                rotationFlag ^= 1;
                break;
            case 4: //If clockwise, command all the way to 0 degrees
                if(rotationFlag){
                    degreePoint = 0.0;
                    command = servo_move(0.0);
                } else { //Otherwise, command all the way to 180 degrees
                    degreePoint = 180.0;
                    command = servo_move(180.0);
                }
                break;

        }
        timer_waitMillis(250);
    }
}
