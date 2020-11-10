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

/**
 * Define the connection ports of the gyro sensor and motors.
 * By default, the Gyro Boy robot uses the following ports:
 * Gyro sensor: Port 2
 * Left motor:  Port A
 * Right motor: Port D
 */
const static motor_port_t left_motor = EV3_PORT_A;
const static motor_port_t right_motor = EV3_PORT_B;

static void motor_steer_callback(std_msgs::String *msg)
{
	int power, turn_ratio;
	sscanf(msg->data.c_str(), "v:%d,v:%d", &power, &turn_ratio);
  ev3_motor_steer(left_motor, right_motor, power, turn_ratio);
	return;
}

void usr_task1(intptr_t unused)
{
	syslog(LOG_NOTICE,"========Activate user task1========");

  // Configure color sensor
  ev3_sensor_config(EV3_PORT_1, COLOR_SENSOR);
  // Configure motors
  ev3_motor_config(left_motor, LARGE_MOTOR);
  ev3_motor_config(right_motor, LARGE_MOTOR);
  syslog(LOG_NOTICE, "#### motor control start");

	int argc = 0;
	char *argv = NULL;
	ros::init(argc, argv, "robo_controller");
	ros::NodeHandle n;
  ros::Rate loop_rate(100); // 10msec

	ros::Subscriber sub = n.subscribe("motor_steer",1, motor_steer_callback);

  while(1) {
    loop_rate.sleep();
  }
  return;
}

static char str_buf[1024];

static void topic_publish_one(ros::Publisher &pub, int value)
{
	std_msgs::String str;
	sprintf(str_buf, "v:%d", value);
	str.data = string(str_buf);
	pub.publish(str);
	return;
}

static void topic_publish_two(ros::Publisher &pub, int32_t value1, int32_t value2)
{
	std_msgs::String str;
	sprintf(str_buf, "v:%d,v:%d", value1, value2);
	str.data = string(str_buf);
	pub.publish(str);
	return;
}


void usr_task2(intptr_t unused)
{
	syslog(LOG_NOTICE,"========Activate user task2========");
	int argc = 0;
	char *argv = NULL;

	ros::init(argc, argv, "robo_sensor");
	ros::NodeHandle n;
	ros::Rate loop_rate(100);
  
  ros::Publisher pub1 = n.advertise<std_msgs::String>("color_sensor_reflect", 1);
  ros::Publisher pub2 = n.advertise<std_msgs::String>("color_sensor_color", 1);
  ros::Publisher pub3 = n.advertise<std_msgs::String>("motor_counts", 1);

	while (1) {
    colorid_t color_type = ev3_color_sensor_get_color(EV3_PORT_1);
    int sensor_data =  (int) ev3_color_sensor_get_reflect(EV3_PORT_1);
    int32_t left_motor_count = ev3_motor_get_counts(EV3_PORT_A);
    int32_t right_motor_count = ev3_motor_get_counts(EV3_PORT_B);
		topic_publish_one(pub1, sensor_data);
		topic_publish_one(pub2, color_type);
    topic_publish_two(pub3, left_motor_count, right_motor_count);
		loop_rate.sleep();
	}
	return;
}

#ifdef __cplusplus
}
#endif
