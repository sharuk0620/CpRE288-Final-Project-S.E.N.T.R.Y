
#ifndef MOVEMENT

#define MOVEMENT





#include "open_interface.h"
#include <stdio.h>

//Moves the cybot forward a specified number of millimeters (1000 millimeters = 1 meter)
//double move_forward(oi_t *sensor, double millimeters);

void move_forward(oi_t *sensor, double millimeters);

void move_backward(oi_t *sensor, double millimeters);

int simple_move_forward(oi_t *sensor, double millimeters);

//Turns the Cybot clockwise a specified number of degrees
void turn_clockwise(oi_t *sensor, int degrees);

void turn_counterclockwise(oi_t *sensor, int degrees);

double collisionDetection(oi_t *sensor, double distToTravel);

void collisionProgram(oi_t *sensor, double distToTravel);



#endif
