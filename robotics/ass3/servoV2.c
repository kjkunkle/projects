// PART 2
#pragma config(Sensor, in1,    L_sensor,       sensorReflection)
#pragma config(Sensor, in2,    Pot,            sensorPotentiometer)
#pragma config(Motor,  port6,           Servo,         tmotorNormal, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

float pot_angle();

int searchLight(){
	int MaxLight = 1123; // max darkness
	int templight; // temporary value used for coping
	int MaxAngle;  // angle value of sensor where maximum brightness has been found
	int servo_pos = -127; // initial servo position

	motor[Servo] = -127; // set initial servo position - all the way clockwise
	wait1Msec(400);

	while(servo_pos<=127){ // 128 iterations in steps of 2
		//writeDebugStreamLine("%d\n", servo_pos);
		motor[Servo] = servo_pos; // set servo to the next position 
		wait1Msec(10); // give it time to move
		templight = SensorValue[L_sensor]; // store value of light

		if (templight<MaxLight){ // if brighter light found
			MaxLight = templight; // update max brightness found
			MaxAngle = servo_pos; // update angle of max brightness
		}
		servo_pos=servo_pos+2; // +2 rotation values in counter clockwise direction
	}

	motor[Servo] = MaxAngle; // point back to brightest direction
	wait1Msec(500); // wait for movement
	return pot_angle(); // returns the physical angle of the brightest light
}

void pot_test(){ //moves the servo a specified amount and then prints the potentiometer’s value at the position
	motor[Servo] = -127;
	wait1Msec(10000);
	writeDebugStreamLine("Pot reading at negative limit = %d", SensorValue[Pot]);
	wait1Msec(200);
	motor[Servo] = 127;
	wait1Msec(10000);
	writeDebugStreamLine("Pot reading at positive limit = %d", SensorValue[Pot]);
	wait1Msec(200);
}

float pot_angle(){ //calculates the angle based on current pot reading
	float reading;
	float angle;

	reading = SensorValue[Pot]; // current pot reading 
	angle = -0.0577*(reading) + 163.33; // calibration function
	return angle; // return actual physical angle
}


task main()
{
	wait1Msec(2000);
	while(true){	// loop forever
		wait1Msec(5000);
		//motor[Servo] = 0; set servo to desired position 
		//pot_test();		used for potentiometer calibration
		writeDebugStreamLine("%d\n",searchLight());
	}
}
