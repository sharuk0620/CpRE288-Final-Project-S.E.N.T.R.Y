/*
 * ping.h
 *
 *  Created on: Oct 28, 2026
 *      Author: ssharuk
 */

#ifndef PING_H_
#define PING_H_

//Initializes the timer module to be used with ping sensor
void ping_init(void);

//Logic for pulsing the ping sensor.
int ping_read(void);


#endif /* PING_H_ */
