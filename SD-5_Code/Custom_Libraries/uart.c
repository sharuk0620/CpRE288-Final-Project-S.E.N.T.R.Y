/*
*
*   uart.c
*
*
*
*
*
*   @author
*   @date
*/

#include "uart.h"
#include "timer.h"
#include "lcd.h"

volatile  char uart_data;  // Your UART interupt code can place read data here
volatile  char flag;       // Your UART interupt can update this flag
                                  // to indicate that it has placed new data
                                  // in uart_data

void uart_init(void){

    //UART GPIO INIT

    SYSCTL_RCGCGPIO_R |= 0x00000002; //Turns on clock for port B

    SYSCTL_RCGCUART_R |= 0x00000002; //Turns on clock for UART1

    timer_waitMillis(1); // Small delay before accessing device after turning on clock

    GPIO_PORTB_AFSEL_R |= 0x00000003; //Chooses PB0 and PB1 to use alternative hardware

    GPIO_PORTB_PCTL_R &= ~(0x000000FF);  //Selects PB0 and PB1 to use U1RX and U1TX respectively
    GPIO_PORTB_PCTL_R |= 0x00000011;

    GPIO_PORTB_DEN_R |= 0x00000003; //Enables PB0 and PB1

    GPIO_PORTB_DIR_R &= 0xFFFFFFFE; //Sets PB0 to input, PB1 to output
    GPIO_PORTB_DIR_R |= 0x00000002;


    //Baud Rate Calculations: We will use a baudrate of 115200, data bits = 8, stop bits = 1, and no parity bits

    /*
    int ibrd; //integer portion is 8
    double fbrd; //fractional portion is 0.680555555556 (Repeating) (44 post equation)

    ibrd = 8;

    //0.680555556 * 64 + 0.5 (rounded)
    fbrd = 44;
    */


    //Disable UART to begin configuration
    UART1_CTL_R &= ~(0x00000001);

    //Assign UARTIBRD register the integer portion of calculated BRD
    //UART1_IBRD_R &= ~(0x0000FFFF);
    //UART1_IBRD_R |= 0x00000008;
    UART1_IBRD_R = 8;

    //Assign UARTFBRD register the fractional portion (post equation) of calculated BRD
    //UART1_FBRD_R &= ~(0x0000003F);
    //UART1_FBRD_R |= 0x0000002C;
    UART1_FBRD_R = 44;

    //Set frame of data being received/transmitted (8 data bits, 1 stop bit, no parity bit (10 bit total))
    UART1_LCRH_R |= 0x00000060;

    //Set system clock for UART1 Device (Using system clock)
    UART1_CC_R &= ~(0x0000000F);

    //Set bits 9, 8, and 0 to 1 to enable uart and RX/TX
    UART1_CTL_R |= 0x00000301;





}

void uart_sendChar(char data){
	//TODO

    while((UART1_FR_R & 0x00000020) != 0){
        //Blocking Loop; Waits until UART1_FR_R is empty (i.e all 0s)
    }

    //Writes data to register
    UART1_DR_R = data;
}

char uart_receive(void){
	char data = 0;

	while((UART1_FR_R & 0x00000010) != 0){
	    //Blocking Loop; Waits until UART1_FR_R is full
	}

	data = (char)(UART1_DR_R & 0x000000FF);

	return data;

}

void uart_sendStr(const char *data){
	//TODO for reference see lcd_puts from lcd.c file

    while(*data != '\0' && *data != '\r')
    {
        lcd_putc(*data);
        data++;
    }
}


void uart_interrupt_init()
{
    // Enable interrupts for receiving bytes through UART1
    UART1_IM_R |= 0x00000010; //enable interrupt on receive - page 924

    // Find the NVIC enable register and bit responsible for UART1 in table 2-9
    // Note: NVIC register descriptions are found in chapter 3.4
    //BIT 6 IN EN0 needs to be 1
    NVIC_EN0_R |= 0x00000040; //enable uart1 interrupts - page 104

    // Find the vector number of UART1 in table 2-9 ! UART1 is 22 from vector number page 104
    IntRegister(INT_UART1, uart_interrupt_handler); //give the microcontroller the address of our interrupt handler - page 104 22 is the vector number

}

void uart_interrupt_handler()
{
// STEP1: Check the Masked Interrupt Status
    //Checks bit 4 (RXMIS)
    if((UART1_MIS_R & 0x00000010) != 0){//interrupt mask register
        flag = 1; //Sets flag to 1, meaning receive interrupt was sent to NVIC and ready to be serviced
    }

//STEP2:  Copy the data

    //Reads data into uart_data from UART1_DR_R
    uart_data = uart_receive();
    //char data = uart_receive();
    //uart_sendChar(data);
//STEP3:  Clear the interrupt
    UART1_ICR_R |= 0x00000010; //Register 13 UARTICR clear bits 4 (RX) Page 934
}
