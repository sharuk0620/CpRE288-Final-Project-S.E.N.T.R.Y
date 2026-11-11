/*
 * servo.h
 *
 *  Created on: Nov 4, 2026
 *      Author: ssharuk
 */

#ifndef SERVO_H_
#define SERVO_H_

//Initializes the servo peripheral
void servo_init(void);

//Moves the servo to the commanded degree value
int servo_move(float degrees);

void servo_calibrate();

void servo_position_control(void);



#endif /* SERVO_H_ */
