/*
 * part4.c
 *
 *  Created on: Oct 26, 2026
 *      Author: ssharuk
 */

#include "uart.h"
#include "servo.h"
#include "ping.h"
#include "Timer.h"
#include "lcd.h"
#include <string.h>
#include "movement.h"
#include "open_interface.h"
#include <math.h>
#include "adc.h"
//#include "sensor-data.h"

typedef struct
{ //I made our object data a struct
    float Width;
    float Dist;
    char Loc;
    char Rad;
    char Ang;
} object;

char buffer[21];

extern volatile char uart_data; // Your UART interupt code can place read data here
extern volatile char flag;

char message[50];
float recordedVals[91];

void sendMessage(char messageSent[]);
void Outlier(float data_array[], char i);
void EdgeCleanup(float data_array[], char i);
float calcLinWidth(char angle, float dist);
void Careful_move_forward(oi_t *sensor, float distToTravel, object smallest,
                          float recordedVals[]);
void Clean_Data(float recordedVals[], char message[], float maxDist);

#define PI_F 3.14159265358979323846f

int main(void)
{
    //Initializes UART serial communication with cybot
    uart_init();

    //Initializes scan ping feature

    timer_init(); //initiallize timer
    lcd_init(); //initiallize LCD
    servo_init();
    //sensor_calibrate(); //Calibrates the scanner to be at the right position


    //Initializes ADC module and readies conversion
    adc_init();



    //Loop incremental variables
    int i = 0;
    int j = 0;
    //int k = 0;

    float maxDist = 60.00f;
    //object current;
    object smallestObj;
    //smallest.Width = 180.0f;
    //char objectCount = 0;
    //char objFirstFlag = 0;

    //float linWidth = 0.0f;
    //int curIRDist = 0;
    //float curPingDist = 0.00;
    //int sum = 0;
    //float calculatedIR = 0.00;
    //char objAngToMove = 0;


    //Loop incremental variables
    int k = 0;

    char radialDist = 0;
    char objectCount = 0;
    char objFirstFlag = 0;
    char firstAng = 0;
    float linWidth = 0.0f;
    float smallestWidth = 180.0f;
    char smallestObjLoc = 0;
    float smallestDist = 0;
    char objLoc = 0;
    int curIRDist = 0;
    float pingDist = 0.00;
    int sum = 0;
    int average = 0;
    float calculatedIR = 0.00;
    int dummy;
    int servoDummy;
    char keyGiven;
    //float average;

    char buffer[5]; //Buffer to store keyGiven and inputted new line char
    char movementKey; //Accesses the first element in buffer, or the actual key press
    int index; //Index variable to access buffer

    //Control
    char toggle = 0; //0 is manual, 1 is auto

#define PI_F 3.14159265358979323846f

    //edited code starting from lab 5
    //Prints out label for sensor data analysis.
    char sensorLabels[] = "Angle\t\tDistance\n";

    //float average; //Was used for a no longer used function, now used during scanning

    //edited code starting from lab 5
    //Prints out label for sensor data analysis.

    //SECTION -1:

    //Initializes sensor_data struct to be used in movement functions
    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);

    while (1)
    {
        /*
         //keyGiven stores the key that was pressed on the keyboard
         index = 0;

         keyGiven = uart_receive();

         //While keyGiven is a new line
         while (keyGiven != '\n')
         {
         //Adds the key given to the buffer
         buffer[index] = keyGiven;

         //Increments index and grabs the next value
         index++;
         keyGiven = uart_receive();
         }

         //Adds the newline at where the while loop started
         buffer[index] = '\n';

         //After, adds the null byte to tell wher to stop printing if needed
         buffer[index + 1] = 0;

         //Accesses the raw key press
         movementKey = buffer[0];
         */

        if (toggle)
        {
            //================================================AUTO MODE===========================================================

            /*
             sprintf(message, "AUTO-MODE ON\n");
             sendMessage(message);
             memset(message, 0, sizeof(message));
             */

            //keyGiven stores the key that was pressed on the keyboard
            index = 0;

            //Grabs the next sent byte
            keyGiven = uart_receive();

            //While keyGiven is a new line
            while (keyGiven != '\n')
            {
                //Adds the key given to the buffer
                buffer[index] = keyGiven;

                //Increments index and grabs the next value
                index++;
                keyGiven = uart_receive();
            }

            //Adds the newline at where the while loop started
            buffer[index] = '\n';

            //After, adds the null byte to tell wher to stop printing if needed
            buffer[index + 1] = 0;

            //Accesses the raw key press
            movementKey = buffer[0];

            //If allowed, start scanning
            if (movementKey == 'm')
            {
                //Perform a scan
                // sendMessage(sensorLabels);
                for (i = 0; i < 91; i++)
                {
                    //Starting a 0, wait a little bit so the sensor can take a proper scan
                    if (i == 0)
                    {
                        servoDummy = servo_move(i * 2);
                        timer_waitMillis(1000);
                    }

                    //Scans at angle i*2, grabs value from pointer, prints to putty, and clears message.

                    servoDummy = servo_move(i * 2);
                    timer_waitMillis(100);

                    //REPLACED ADC LOGIC
                    curIRDist = adc_read();
                    calculatedIR = adc_convert(curIRDist);

                    recordedVals[i] = calculatedIR;

                    /*
                     sprintf(message, "%d\t\t\t%.2f\t\t\t\n", i * 2,
                     calculatedIR);
                     sendMessage(message);
                     memset(message, 0, sizeof(message));
                     */

                    sum = 0;
                }

                //SECTION 1: DATA CLEAN UP:

                for (j = 0; j < 91; j++)
                {

                    //Any Data Past maximum distance is set to the maximum distance
                    if (recordedVals[j] > maxDist)
                    {
                        recordedVals[j] = maxDist;
                    }

                    //Runs outlier function on every data point except 0 and 90
                    if (j != 0 && j != 90)
                    {
                        Outlier(recordedVals, j); //runs the outlier function
                    }
                }

                //Print out Clean Data
                sendMessage(sensorLabels);
                for (i = 0; i < 91; i++)
                {

                    //Print out data from cleaning algorithm
                    sprintf(message, "%d\t\t\t%.2f\n", i * 2, recordedVals[i]);
                    sendMessage(message);
                    memset(message, 0, sizeof(message));
                }

                //END label for sending data to client via socket
                sprintf(message, "END\n");
                sendMessage(message);
                memset(message, 0, sizeof(message));

                /*
                 char Labels[] =
                 "Object#\tAngle\t\tIR-Distance\tRadial Width\t Linear Width\tAngle To Point\n";
                 sendMessage(Labels);
                 */

                //Handles the case of i being 0
                if (recordedVals[0] < maxDist)
                {
                    objFirstFlag = 1;
                    firstAng = 0;
                }

                //SECTION 2: DATA ANALYSIS
                for (i = 1; i < 91; i++)
                {

                    if ((recordedVals[i] > (recordedVals[i - 1] + 10))
                            || (recordedVals[i] < (recordedVals[i - 1] - 10)))
                    {

                        //Edit 2: redefined a detected object as when there is a significant change in distance //If less than max, meaning the dist is for an detected object

                        //If first flag is 0, and if that recorded val at i is less than max, then set first flag is 1, and initialize first angle of object (Right edge)
                        if (objFirstFlag == 0)
                        {
                            if (recordedVals[i] < maxDist)
                            {
                                objFirstFlag = 1;
                                firstAng = i * 2;
                            }

                            //If first flag is 1, meaning object has already been detected, must mean this is the left edge
                        }
                        else
                        {
                            //Increments object count, resets flags and measurements, and checks if it's the smallest lin width it has recorded
                            if (radialDist <= 10)
                            {
                                firstAng = 0;
                                radialDist = 0;
                                objLoc = 0;
                                objFirstFlag = 0;
                                continue;
                            }
                            objectCount++;
                            objFirstFlag = 0;
                            linWidth = calcLinWidth(radialDist,
                                                    recordedVals[i - 1]);
                            objLoc = firstAng + (radialDist / 2);
                            servoDummy = servo_move(objLoc);
                            timer_waitMillis(500);
                            pingDist = ping_read(); //NEW Variable, Scan PING DIST. used in place of (IR dist from left edge)
                            timer_waitMillis(1000);
                            if (linWidth < smallestWidth)
                            {
                                smallestWidth = linWidth;
                                smallestObjLoc = objLoc;
                                smallestDist = pingDist;
                            }

                            //Prints object info to putty
                            /*
                             sprintf(message,
                             "%d\t%d\t\t%.2f\t\t%d\t\t%.2f\t\t%d\n",
                             objectCount, firstAng, pingDist, radialDist,
                             linWidth, smallestObjLoc);
                             sendMessage(message);
                             memset(message, 0, sizeof(message)); //clears message to be reused
                             */
                            firstAng = 0;
                            radialDist = 0;
                            objLoc = 0;
                        }
                    }
                    else
                    {
                        //If first flag is 1, meaning object is detected already
                        if (objFirstFlag == 1)
                        {

                            //Increments object's radial dist by 2
                            radialDist += 2;

                            //Special case, if object is still detected at 90, increments AND accounts for object
                            if (i == 90)
                            {
                                if (radialDist <= 10)
                                {
                                    firstAng = 0;
                                    radialDist = 0;
                                    objLoc = 0;
                                    objFirstFlag = 0;
                                    continue;
                                }
                                objectCount++; //adds object
                                objFirstFlag = 0;
                                //USING FIRST ANG, RADIAL DIST, MAKE PING SENSOR SCAN AT MIDDLE POINT OF OBJECT FOR DISTANCE
                                linWidth = calcLinWidth(radialDist,
                                                        recordedVals[i]);
                                objLoc = firstAng + (radialDist / 2);
                                servoDummy = servo_move(objLoc);
                                timer_waitMillis(500);
                                pingDist = ping_read(); //NEW Variable, Scan PING DIST. used in place of (IR dist from left edge)
                                timer_waitMillis(1000);
                                if (linWidth < smallestWidth)
                                {
                                    smallestWidth = linWidth;
                                    smallestObjLoc = objLoc;
                                    smallestDist = pingDist;
                                }
                                //Prints object info to putty
                                /*
                                 sprintf(message,
                                 "%d\t%d\t\t%.2f\t\t%d\t\t%.2f\t\t%d\n",
                                 objectCount, firstAng, pingDist,
                                 radialDist, linWidth, smallestObjLoc);
                                 sendMessage(message);
                                 memset(message, 0, sizeof(message)); //clears message to be reused
                                 */
                                firstAng = 0;
                                radialDist = 0;
                                objLoc = 0;
                            }
                        }
                    }
                }
            }

            //Bot evaluates what to do here and asks
            if (objectCount == 0)
            {

                /*
                 sprintf(message, "No object detected, move forward?\n");
                 sendMessage(message);
                 memset(message, 0, sizeof(message)); //clears message to be reused
                 */

                //keyGiven stores the key that was pressed on the keyboard
                index = 0;

                //Grabs the next sent byte
                keyGiven = uart_receive();

                //While keyGiven is a new line
                while (keyGiven != '\n')
                {
                    //Adds the key given to the buffer
                    buffer[index] = keyGiven;

                    //Increments index and grabs the next value
                    index++;
                    keyGiven = uart_receive();
                }

                //Adds the newline at where the while loop started
                buffer[index] = '\n';

                //After, adds the null byte to tell wher to stop printing if needed
                buffer[index + 1] = 0;

                //Accesses the raw key press
                movementKey = buffer[0];

                //If allowed, start scanning
                if (movementKey == 'h')
                {

                    //Move forward
                    dummy = simple_move_forward(sensor_data, 100);

                    sprintf(message, "Moved forward 10 cm\n");
                    sendMessage(message);
                    memset(message, 0, sizeof(message)); //clears message to be reused
                }
                else if (movementKey == 't')
                {

                    //if asked to go to manual, set toggle to 0
                    toggle = 0;

                    sprintf(message, "TOGGLED TO MANUAL MODE\n");
                    sendMessage(message);
                    memset(message, 0, sizeof(message)); //clears message to be reused
                }

                //If the given key was not m, echo it back to the client
                /*
                 if(movementKey != 'm'){
                 uart_sendChar(movementKey);
                 if(movementKey != '\n'){
                 uart_sendChar('\n');
                 }
                 }
                 */
            }
            else
            {

                /*
                 sprintf(message,
                 "I want to point to %d and move forward %.2fcm\n",
                 smallestObjLoc, (smallestDist / 2));
                 sendMessage(message);
                 memset(message, 0, sizeof(message)); //clears message to be reused
                 uart_sendChar('\n');
                 */

                //keyGiven stores the key that was pressed on the keyboard
                index = 0;

                //Grabs the next sent byte
                keyGiven = uart_receive();

                //While keyGiven is a new line
                while (keyGiven != '\n')
                {
                    //Adds the key given to the buffer
                    buffer[index] = keyGiven;

                    //Increments index and grabs the next value
                    index++;
                    keyGiven = uart_receive();
                }

                //Adds the newline at where the while loop started
                buffer[index] = '\n';

                //After, adds the null byte to tell wher to stop printing if needed
                buffer[index + 1] = 0;

                //Accesses the raw key press
                movementKey = buffer[0];

                //If allowed
                if (movementKey == 'h')
                {
                    //Move forward
                    if (smallestDist > 5.00)
                    {
                        if (smallestObjLoc < 90)
                        {
                            turn_clockwise(sensor_data, (90 - smallestObjLoc));
                        }
                        else if (smallestObjLoc > 90)
                        {
                            turn_counterclockwise(sensor_data,
                                                  (smallestObjLoc - 90));
                        }
                        dummy = simple_move_forward(sensor_data,
                                                    (smallestDist / 2) * 10);
                        if (dummy == 1)
                        {
                            sprintf(message, "LEFT BUMPER HIT, REDIRECTING\n");
                            sendMessage(message);
                            memset(message, 0, sizeof(message)); //clears message to be reused
                        }
                        else if (dummy == 2)
                        {
                            sprintf(message, "RIGHT BUMPER HIT, REDIRECTING\n");
                            sendMessage(message);
                            memset(message, 0, sizeof(message)); //clears message to be reused
                        }
                        else
                        {
                            sprintf(message, "NO COLLISION\n");
                            sendMessage(message);
                            memset(message, 0, sizeof(message)); //clears message to be reused
                        }
                    }
                    else
                    {
                        sprintf(message, "AT OBJECT\n");
                        sendMessage(message);
                        memset(message, 0, sizeof(message)); //clears message to be reused
                        break;
                    }
                    smallestWidth = 180.00f;
                    smallestObjLoc = 0;
                    smallestDist = 0;
                    objectCount = 0;

                    /*
                     //If the given key was not m, echo it back to the client
                     if(movementKey != 'm'){
                     uart_sendChar(movementKey);
                     if(movementKey != '\n'){
                     uart_sendChar('\n');
                     }
                     }
                     */
                }
                else if (movementKey == 't')
                {
                    //if asked to go to manual, set toggle to 0
                    toggle = 0;

                    sprintf(message, "TOGGLED TO MANUAL MODE\n");
                    sendMessage(message);
                    memset(message, 0, sizeof(message)); //clears message to be reused
                }
            }
        }
        else
        {
            //================================================MANUAL MODE===========================================================

            /*
             sprintf(message, "MANUAL-MODE ON\n");
             sendMessage(message);
             memset(message, 0, sizeof(message));

             */
            //Gets message
            //keyGiven stores the key that was pressed on the keyboard
            index = 0;

            keyGiven = uart_receive();

            //While keyGiven is a new line
            while (keyGiven != '\n')
            {
                //Adds the key given to the buffer
                buffer[index] = keyGiven;

                //Increments index and grabs the next value
                index++;
                keyGiven = uart_receive();
            }

            //Adds the newline at where the while loop started
            buffer[index] = '\n';

            //After, adds the null byte to tell wher to stop printing if needed
            buffer[index + 1] = 0;

            //Accesses the raw key press
            movementKey = buffer[0];

            //If such key is not m, or the message to PuTTY conditional
            if (movementKey != 't')
            {
                //If such key pressed was w, commands the bot to move forward 200 millimeters
                if (movementKey == 'w')
                {
                    dummy = simple_move_forward(sensor_data, 25);

                    if (dummy == 1)
                    {
                        sprintf(message, "LEFT BUMPER HIT, REDIRECTING\n");
                        sendMessage(message);
                        memset(message, 0, sizeof(message)); //clears message to be reused
                        //uart_sendChar('\n');
                    }
                    else if (dummy == 2)
                    {
                        sprintf(message, "RIGHT BUMPER HIT, REDIRECTING\n");
                        sendMessage(message);
                        memset(message, 0, sizeof(message)); //clears message to be reused
                        //uart_sendChar('\n');
                    }
                    else
                    {
                        sprintf(message, "NO COLLISION Moved Forward 2.5 cm\n");
                        sendMessage(message);
                        memset(message, 0, sizeof(message)); //clears message to be reused
                        //uart_sendChar('\n');
                    }

                    //If such key pressed was s, commands the bot to move backward 200 millimeters
                }
                else if (movementKey == 's')
                {
                    move_backward(sensor_data, -25);

                    sprintf(message, "Moved Back 2.5 cm\n");
                    sendMessage(message);
                    memset(message, 0, sizeof(message));
                    //uart_sendChar('\n');

                    //If such key pressed was a, commands the bot to turn left 90 degrees and move forward 200 millimeters
                }
                else if (movementKey == 'a')
                {
                    turn_counterclockwise(sensor_data, 15);

                    sprintf(message, "Turned Left 15 degrees\n");
                    sendMessage(message);
                    memset(message, 0, sizeof(message));
                    //uart_sendChar('\n');

                    //If such key pressed was d, commands the bot to turn right 90 degrees and move forward 200 millimeters
                }
                else if (movementKey == 'd')
                {
                    turn_clockwise(sensor_data, 15);

                    sprintf(message, "Turned Right 15 degrees\n");
                    sendMessage(message);
                    memset(message, 0, sizeof(message));
                    //uart_sendChar('\n');
                    //Otherwise, print the character to the LCD screen on the bot.
                }
                else if (movementKey == 'm')
                {
                    //SECTION 0: Collect Data
                    /*
                     sprintf(message, "SCANNING!\n");
                     sendMessage(message);
                     memset(message, 0, sizeof(message));
                     */

                    // sendMessage(sensorLabels);
                    for (i = 0; i < 91; i++)
                    {
                        //Starting a 0, wait a little bit so the sensor can take a proper scan
                        if (i == 0)
                        {
                            servoDummy = servo_move(i * 2);
                            timer_waitMillis(1000);
                        }

                        //Scans at angle i*2, grabs value from pointer, prints to putty, and clears message.

                        servoDummy = servo_move(i * 2);
                        timer_waitMillis(100);

                        //REPLACED ADC LOGIC
                        curIRDist = adc_read();
                        calculatedIR = adc_convert(curIRDist);

                        recordedVals[i] = calculatedIR;

                        /*
                         sprintf(message, "%d\t\t\t%.2f\t\t\t\n", i * 2,
                         calculatedIR);
                         sendMessage(message);
                         memset(message, 0, sizeof(message));
                         */

                        sum = 0;
                    }

                    //SECTION 1: DATA CLEAN UP:

                    for (j = 0; j < 91; j++)
                    {

                        //Any Data Past maximum distance is set to the maximum distance
                        if (recordedVals[j] > maxDist)
                        {
                            recordedVals[j] = maxDist;
                        }

                        //Runs outlier function on every data point except 0 and 90
                        if (j != 0 && j != 90)
                        {
                            Outlier(recordedVals, j); //runs the outlier function
                        }
                    }

                    //Print out Clean Data
                    sendMessage(sensorLabels);
                    for (i = 0; i < 91; i++)
                    {

                        //Print out data from cleaning algorithm
                        sprintf(message, "%d\t\t\t%.2f\n", i * 2,
                                recordedVals[i]);
                        sendMessage(message);
                        memset(message, 0, sizeof(message));
                    }

                    //END label for sending data to client via socket
                    sprintf(message, "END\n");
                    sendMessage(message);
                    memset(message, 0, sizeof(message));

                    /*
                     char Labels[] =
                     "Object#\tAngle\t\tIR-Distance\tRadial Width\t Linear Width\tAngle To Point\n";
                     sendMessage(Labels);
                     */

                    //Handles the case of i being 0
                    if (recordedVals[0] < maxDist)
                    {
                        objFirstFlag = 1;
                        firstAng = 0;
                    }

                    //SECTION 2: DATA ANALYSIS
                    for (i = 1; i < 91; i++)
                    {

                        if ((recordedVals[i] > (recordedVals[i - 1] + 10))
                                || (recordedVals[i] < (recordedVals[i - 1] - 10)))
                        {

                            //Edit 2: redefined a detected object as when there is a significant change in distance //If less than max, meaning the dist is for an detected object

                            //If first flag is 0, and if that recorded val at i is less than max, then set first flag is 1, and initialize first angle of object (Right edge)
                            if (objFirstFlag == 0)
                            {
                                if (recordedVals[i] < maxDist)
                                {
                                    objFirstFlag = 1;
                                    firstAng = i * 2;
                                }

                                //If first flag is 1, meaning object has already been detected, must mean this is the left edge
                            }
                            else
                            {
                                /*
                                 if(radialDist == 0){
                                 firstAng = 0;
                                 radialDist = 0;
                                 continue;
                                 }
                                 */
                                //Increments object count, resets flags and measurements, and checks if it's the smallest lin width it has recorded
                                if (radialDist <= 10)
                                {
                                    firstAng = 0;
                                    radialDist = 0;
                                    objLoc = 0;
                                    objFirstFlag = 0;
                                    continue;
                                }
                                objectCount++;
                                objFirstFlag = 0;
                                linWidth = calcLinWidth(radialDist,
                                                        recordedVals[i - 1]);
                                objLoc = firstAng + (radialDist / 2);
                                servoDummy = servo_move(objLoc);
                                timer_waitMillis(500);
                                pingDist = ping_read(); //NEW Variable, Scan PING DIST. used in place of (IR dist from left edge)
                                timer_waitMillis(1000);
                                if (linWidth < smallestWidth)
                                {
                                    smallestWidth = linWidth;
                                    smallestObjLoc = objLoc;
                                    smallestDist = pingDist;
                                }

                                //Prints object info to putty
                                /*
                                 sprintf(message,
                                 "%d\t%d\t\t%.2f\t\t%d\t\t%.2f\t\t%d\n",
                                 objectCount, firstAng, pingDist,
                                 radialDist, linWidth, smallestObjLoc);
                                 sendMessage(message);
                                 memset(message, 0, sizeof(message)); //clears message to be reused
                                 */
                                firstAng = 0;
                                radialDist = 0;
                                objLoc = 0;
                            }
                        }
                        else
                        {
                            //If first flag is 1, meaning object is detected already
                            if (objFirstFlag == 1)
                            {

                                //Increments object's radial dist by 2
                                radialDist += 2;

                                //Special case, if object is still detected at 90, increments AND accounts for object
                                if (i == 90)
                                {
                                    /*
                                     //Increments object count, resets flags and measurements, and checks if it's the smallest lin width it has recorded
                                     if(radialDist == 0){
                                     firstAng = 0;
                                     radialDist = 0;
                                     continue;
                                     }
                                     */
                                    if (radialDist <= 10)
                                    {
                                        firstAng = 0;
                                        radialDist = 0;
                                        objLoc = 0;
                                        objFirstFlag = 0;
                                        continue;
                                    }
                                    objectCount++; //adds object
                                    objFirstFlag = 0;
                                    //USING FIRST ANG, RADIAL DIST, MAKE PING SENSOR SCAN AT MIDDLE POINT OF OBJECT FOR DISTANCE
                                    linWidth = calcLinWidth(radialDist,
                                                            recordedVals[i]);
                                    objLoc = firstAng + (radialDist / 2);
                                    servoDummy = servo_move(objLoc);
                                    timer_waitMillis(500);
                                    pingDist = ping_read(); //NEW Variable, Scan PING DIST. used in place of (IR dist from left edge)
                                    timer_waitMillis(1000);
                                    if (linWidth < smallestWidth)
                                    {
                                        smallestWidth = linWidth;
                                        smallestObjLoc = objLoc;
                                        smallestDist = pingDist;
                                    }
                                    //Prints object info to putty
                                    /*
                                     sprintf(message,
                                     "%d\t%d\t\t%.2f\t\t%d\t\t%.2f\t\t%d\n",
                                     objectCount, firstAng, pingDist,
                                     radialDist, linWidth,
                                     smallestObjLoc);
                                     sendMessage(message);
                                     memset(message, 0, sizeof(message)); //clears message to be reused
                                     */
                                    firstAng = 0;
                                    radialDist = 0;
                                    objLoc = 0;
                                }
                            }
                        }
                    }
                }

            }
            else
            {
                toggle = 1;
                sprintf(message, "TOGGLED TO AUTO MODE\n");
                sendMessage(message);
                memset(message, 0, sizeof(message)); //clears message to be reused
            }

        }

    }

    //Frees the sensor_data, and de-initializes it.
    oi_free(sensor_data);

    return 0;
}

//tests if there is an outlier, removes it, returns 1 if yes, 0 if no
void Outlier(float data_array[], char i)
{

    float a = data_array[i - 1];
    float b = data_array[i];
    float c = data_array[i + 1];
    float avg = (a + c) / 2;
    if (((b > (a + 7)) && (b > (c + 7))) || (b < (a - 7) && (b < (c - 7))))
    {
        data_array[i] = avg;
    }
}

void EdgeCleanup(float data_array[], char i)
{
//See what value is at point i
//compare it to point i+1.
//If the magnitude of the difference of the 2 points is or more than 2-3 cm, check the next point i+2
//Now, if the next point is also 2-3 cm difference, check the next point i+3
//NOW, if the next point I+4 is within 1-2 cm of i+3, we need to clear out points between i+3 and i
//OR, if it isnt, keep checking the next point x until the next point y is within 1-2 cm, and then clear out all values betwen i and point x
    float finalVal;
    int j = 1;
    char risingFlag = 0;
    char k;
    while (((data_array[i + j] - data_array[i + j - 1]) > 5
            || (data_array[i + j] - data_array[i + j - 1]) < -5) && (i + j) < 90)
    {
        j++;
        if ((data_array[i + j] - data_array[i + j - 1]) > 5)
        {
            risingFlag = 1;
        }
    }

    switch (risingFlag)
    {
    case 0:
        finalVal = data_array[i + j];
        break;
    case 1:
        finalVal = data_array[i];
        j--;
        break;
    }

    for (k = i + 1; k < (i + j); k++)
    {
        data_array[k] = finalVal;
    }

}

//Sends a message to putty byte by byte
void sendMessage(char messageSent[])
{
//While not equal to null
    while (*messageSent != '\0')
    {
        uart_sendChar(*messageSent);
        messageSent++;
    }
}

//Calculates the lin width of an object using math
float calcLinWidth(char angle, float dist)
{
    float rad = angle * (PI_F / 180.0f);
    float answer = 2.0f * dist * sinf(rad / 2.0f);
    return answer;
}
