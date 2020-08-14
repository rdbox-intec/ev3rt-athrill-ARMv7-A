/**
 * This sample program balances a two-wheeled Segway type robot such as Gyroboy in EV3 core set.
 *
 * References:
 * http://www.hitechnic.com/blog/gyro-sensor/htway/
 * http://www.cs.bgu.ac.il/~ami/teaching/Lejos-2013/classes/src/lejos/robotics/navigation/Segoway.java
 */

#include "ev3api.h"
#include "app.h"

#include "ros.h"
#include "std_msgs/String.h"

#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include <string.h>
#include <stdlib.h>

using namespace std;


#ifdef __cplusplus
extern "C" {
#endif
//dark
//#define white 78
//#define black 20
//bright
#define white 100
#define black 50

/**
 * Define the connection ports of the gyro sensor and motors.
 * By default, the Gyro Boy robot uses the following ports:
 * Gyro sensor: Port 2
 * Left motor:  Port A
 * Right motor: Port D
 */
const static int gyro_sensor = EV3_PORT_2;
const static motor_port_t left_motor = EV3_PORT_A;
const static motor_port_t right_motor = EV3_PORT_B;

typedef enum {
	ROBO_CONTROL_STOP = 0,
	ROBO_CONTROL_START,
} RoboControlType;

static RoboControlType robo_control = ROBO_CONTROL_STOP;
static void ctrl_callback(std_msgs::String *msg)
{
	int ctrl;
	syslog(LOG_NOTICE,"ctrl_callback():%s", msg->data.c_str());
	sscanf(msg->data.c_str(), "v:%d", &ctrl);
	if (ctrl > 0) {
		robo_control = ROBO_CONTROL_START;
	}
	else {
		robo_control = ROBO_CONTROL_STOP;
	}
	return;
}

void usr_task1(intptr_t unused)
{
	int argc = 0;
	char *argv = NULL;
	ros::init(argc,argv,"robo_controller");
	ros::NodeHandle n;
	ros::Subscriber sub;

	sub = n.subscribe("robo_ctrl",1, ctrl_callback);

	syslog(LOG_NOTICE,"========Activate user task1========");

    ev3_sensor_config(EV3_PORT_1, COLOR_SENSOR);

    // Configure motors
    ev3_motor_config(left_motor, LARGE_MOTOR);
    ev3_motor_config(right_motor, LARGE_MOTOR);

    syslog(LOG_NOTICE, "#### motor control start");
    while(1) {
        tslp_tsk(20); /* 20msec */
    	if (robo_control == ROBO_CONTROL_STOP) {
    		ev3_motor_steer(left_motor, right_motor, 0, 0);
    		continue;
    	}
		/**
		 * PID controller
		 */
        static float lasterror = 0, integral = 0;
        static float midpoint = (white - black) / 2 + black;
        {
            float error = midpoint - ev3_color_sensor_get_reflect(EV3_PORT_1);
            integral = error + integral * 0.3;
            float steer = 0.6 * error + 0.3 * integral + 1 * (error - lasterror);
            ev3_motor_steer(left_motor, right_motor, 10, steer);
            lasterror = error;
        }
    }
    return;
}
static char str_buf[1024];

static void topic_publish(ros::Publisher &pub, int value)
{
	std_msgs::String str;
	sprintf(str_buf, "v:%d", value);
	str.data = string(str_buf);
	pub.publish(str);
	return;
}

void usr_task2(intptr_t unused)
{
	syslog(LOG_NOTICE,"========Activate user task2========");
	int argc = 0;
	char *argv = NULL;

	ros::init(argc,argv,"robo_sensor");
	ros::NodeHandle n;
	ros::Rate loop_rate(10); //100msec

	ros::Publisher pub = n.advertise<std_msgs::String>("robo_sensor", 1);

	while (1) {
        float sensor_data = ev3_color_sensor_get_reflect(EV3_PORT_1);
        int sensor_data_100 = (int) (sensor_data * 100.0);
		topic_publish(pub, sensor_data_100);
		loop_rate.sleep();
	}
	return;
}

#ifdef __cplusplus
}
#endif
