

#include "movement.h"

/*
//Moves forward the robot at a certain distance until it hits an object, and returns the distance it has travelled
double move_forward(oi_t *sensor, double millimeters){

    //The distance the robot has traveled during the turn
    double sum = 0;

    //
    double error = 0;

    //Takes 1/7 of the millimeters to travel for P loop purposes
    int oneSeventhMillimeters = (millimeters / 7);

    //Takes 1/10 of the millimeters to travel for P loop purposes
    int oneTenthMillimeters = (millimeters / 10);

    //The proportional constant the error value is multiplied by
    const double startEndPropConst = 1;

    //While the robot has not yet traveled the specified distance
    while (sum < millimeters){

        //Update the sensor, and increment sum with how much distance the robot has traveled according to the sensor
        oi_update(sensor);
        sum += sensor->distance;

        //If either the left or right sensors are hit, then return out of the function by giving the specified distance - how much was actually travelled
        if(sensor->bumpLeft || sensor->bumpRight){
            return millimeters - sum;
        }

        //if the distance traveled currently is less than 1/7th of the the total distance specified
        if(sum <= oneSeventhMillimeters){

            //if sum is even less than 1/10 of the total distance specified
            if(sum <= oneTenthMillimeters){
                oi_setWheels(oneTenthMillimeters, oneTenthMillimeters); //Set wheels to a slow speed of 1/10 of the distance value (to avoid sudden jolts)
            } else {
                sum *= startEndPropConst; //Multiply sum with the p loop proportional constant
                oi_setWheels(sum, sum); //Set wheels to the same speed as sum
            }
        }

        //If the robot has less than or equal 1/7th of total distance to still cover, will assign a slower speed for the robot to avoid sudden stops.
        else if(((millimeters - sum) <= oneSeventhMillimeters)) {

            //If the robot is has even less than or equal to 1/10th of the total distance to cover, will assign an even slower speed
            if((millimeters - sum) <= oneTenthMillimeters){
                oi_setWheels(oneTenthMillimeters, oneTenthMillimeters);

            } else {
                error = (millimeters - sum) * startEndPropConst; //Otherwise, set error with the distance to be covered still times the constant
                oi_setWheels(error, error); //set speed to the value of the distance
            }
        }
        else {
            oi_setWheels(oneSeventhMillimeters, oneSeventhMillimeters); //Maintain a constant speed of 1/7th of the speed until it reaches either one of the p loop bounds
        }
    }
    oi_setWheels(0, 0); //Stops the wheels
    return 0; //Returns 0, used by collisionProgram to know it has made it all the way to the end of specified distance

}
*/

//Moves the robot backwards a certain millimeter amount
void move_backward(oi_t *sensor, double millimeters){

    //Sum defines the distance covered by the robot
    double sum = 0;

    //As the sum is greater than the millimeters specified (millimeters should be a negative number)
    while (sum > millimeters){

        //Updates the sensor, increments sum with the read sensor value, and sets wheels to 100 backwards until sum is = to millimeters, meaning it has went that distance
        oi_update(sensor);
        sum += sensor->distance;
        oi_setWheels(-100, -100);
    }
    //Stops wheels
    oi_setWheels(0, 0);
}

//Moves the robot forward a certain millimeter amount
int simple_move_forward(oi_t *sensor, double millimeters){

    //Clears sensor before moving to ensure no stored values
    oi_update(sensor);


    //Sum defines the distance covered by the robot
    double sum = 0;

    //As the sum is less than the millimeters specified (millimeters should be a positive number)
    while (sum < millimeters){

        //oi_update(sensor);

        //If either the left or right sensors are hit, then return out of the function by giving the specified distance - how much was actually travelled
        if(sensor->bumpLeft || sensor->bumpRight){
            //If the left bumper has been hit
            if (sensor->bumpLeft){

                //Moves the robot back 15 cm
                move_backward(sensor, -100);

                //Turns the robot clockwise 90 degrees
                turn_clockwise(sensor, 45);

                //Moves the robot forward 25 cm
                move_forward(sensor, 200);

                //Turns the robot counterclockwise 90 degrees to reposition back
                turn_counterclockwise(sensor, 90);

                return 1;

            //If the right bumper was hit
            } else if(sensor->bumpRight){

                //Moves the robot back 15 cm
                move_backward(sensor, -100);

                //Turns the robot counterclockwise 90 degrees
                turn_counterclockwise(sensor, 45);

                //Moves the robot forward 25 cm
                move_forward(sensor, 200);

                //Turns the robot clockwise 90 degrees to reposition back
                turn_clockwise(sensor, 90);

                return 2;
            }
        }

        //Updates the sensor, increments sum with the read sensor value, and sets wheels to 100 forwards until sum is = to millimeters, meaning it has went that distance
        oi_update(sensor);
        sum += sensor->distance;
        oi_setWheels(25, 25);

    }
    //Stops wheels
    oi_setWheels(0, 0);

    return 0;
}

//Moves the robot forward a certain millimeter amount
void move_forward(oi_t *sensor, double millimeters){

    //Clears sensor before moving to ensure no stored values
    oi_update(sensor);


    //Sum defines the distance covered by the robot
    double sum = 0;

    //As the sum is less than the millimeters specified (millimeters should be a positive number)
    while (sum < millimeters){


        //Updates the sensor, increments sum with the read sensor value, and sets wheels to 100 forwards until sum is = to millimeters, meaning it has went that distance
        oi_update(sensor);
        sum += sensor->distance;
        oi_setWheels(25, 25);

    }
    //Stops wheels
    oi_setWheels(0, 0);
}

//Turns the robot clockwise a specified number of degrees
void turn_clockwise(oi_t *sensor, int degrees){

    //The current angle the robot has turned
    double curAngle = 0;

    //While the current angle is greater than the amount specified (clockwise angles are read by CyBot as negative degrees)
    while(curAngle > (-1 * degrees)){

        //Updates sensor, and increments curAngle with the angles currently turned
        oi_update(sensor);
        curAngle += sensor->angle;

        //If the CyBot has turned more than 80% of the degrees specified, slows down the wheels to avoid overshoot, else keeps it a constant 75.
        if(curAngle > (-0.80 * degrees)){
            oi_setWheels(-75, 75);
        } else {
            oi_setWheels(-20, 20);
        }

    }

    //Stops the wheels
    oi_setWheels(0, 0);

}

//Turns the robot counterclockwise a specified number of degrees
void turn_counterclockwise(oi_t *sensor, int degrees){

    //The current angle the robot has turned
    double curAngle = 0;

    //While the current angle is less than the amount specified  (clockwise angles are read by CyBot as negative degrees)
    while(curAngle < degrees){

        //Updates sensor, and increments curAngle with the angles currently turned
        oi_update(sensor);
        curAngle += sensor->angle;

        //If the CyBot has turned more than 80% of the degrees specified, slows down the wheels to avoid overshoot, else keeps it a constant 75.
        if(curAngle < (0.80 * degrees)){
            oi_setWheels(75, -75);
        } else {
            oi_setWheels(20, -20);
        }

    }

    //Stops the wheels
    oi_setWheels(0, 0);

}


//Determines whether the left or right bumper has been pressed, and applies moving-around logic accordingly
double collisionDetection(oi_t *sensor, double distToTravel){

    //A dummy variable to hold the value from the call to move_forward
    double dummy = 0;

    //If distToTravel is not 0, meaning move_forward hasn't fully went the full distance, checks if the bumpers have been hit
    if(distToTravel != 0){

        //If the left bumper has been hit
        if (sensor->bumpLeft){

            //Moves the robot back 15 cm
            move_backward(sensor, -150);

            //Turns the robot clockwise 90 degrees
            turn_clockwise(sensor, 90);

            //Moves the robot forward 25 cm
            move_forward(sensor, 250);

            //Turns the robot counterclockwise 90 degrees to reposition back
            turn_counterclockwise(sensor, 90);

            //Adds 150mm to distToTravel to account for the moving back of 15 cm
            return distToTravel += 150;

        //If the right bumper was hit
        } else if(sensor->bumpRight){

            //Moves the robot back 15 cm
            move_backward(sensor, -150);

            //Turns the robot counterclockwise 90 degrees
            turn_counterclockwise(sensor, 90);

            //Moves the robot forward 25 cm
            move_forward(sensor, 250);

            //Turns the robot clockwise 90 degrees to reposition back
            turn_clockwise(sensor, 90);

            //Adds 150mm to distToTravel to account for the moving back of 15 cm
            return distToTravel += 150;
        }

    }
    //Returns distToTravel back if it is 0, meaning it has completed the full distance
    return distToTravel;

}

//Program to run the robot to move a specified number of millimeters and navigate possible objects
void collisionProgram(oi_t *sensor, double distToTravel){

    //While distToTravel is not 0, meaning it hasn't fully completed the specified distance
    while(distToTravel != 0){

        //Sets distToTravel to the distance that is left to cover once moving forward (0 if it completes, some value if it hits an object)
        distToTravel = simple_move_forward(sensor, distToTravel);

        //Checks to see whether left or right bumper was hit, and repositions robot around object
        distToTravel = collisionDetection(sensor, distToTravel);
    }

    //Stops wheels
    oi_setWheels(0, 0);
}








