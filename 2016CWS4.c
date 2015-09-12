#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, I2C_1,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Motor,  port1,           intake2,       tmotorVex393TurboSpeed_HBridge, openLoop)
#pragma config(Motor,  port2,           lrFlywheel,    tmotorVex393TurboSpeed_MC29, openLoop, encoderPort, I2C_1)
#pragma config(Motor,  port3,           urFlywheel,    tmotorVex393TurboSpeed_MC29, openLoop)
#pragma config(Motor,  port4,           llFlywheel,    tmotorVex393TurboSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port5,           ulFlywheel,    tmotorVex393TurboSpeed_MC29, openLoop, reversed)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX)

//Competition Control and Duration Settings
#pragma competitionControl(Competition)
#pragma autonomousDuration(20)
#pragma userControlDuration(120)

#include "Vex_Competition_Includes.c"   //Main competition background code...do not modify!
int motorSpeed = 0;
int velocity;
int position1;
int position2;

task flywheelVelocity(){

	while(true){

		position1 = nMotorEncoder(lrFlywheel);
		wait1Msec(200);
		position2 = nMotorEncoder(lrFlywheel);
		wait1Msec(200);
		velocity = (position2 - position1)/4;

	}
}




void speedUpFlywheel(){
	while(motorSpeed < 90)
	if(motorSpeed < 90)
		motorSpeed += 2;
	else
		motorSpeed = 90;

	motor[urFlywheel] = motorSpeed;
	motor[lrFlywheel] = motorSpeed;
	motor[ulFlywheel] = motorSpeed;
	motor[llFlywheel] = motorSpeed;

	wait1Msec(150);
}
}

void slowDownFlywheel(){
	if(motorSpeed > 0)
		motorSpeed -= 2;
	else
		motorSpeed = 0;

	motor[urFlywheel] = motorSpeed;
	motor[lrFlywheel] = motorSpeed;
	motor[ulFlywheel] = motorSpeed;
	motor[llFlywheel] = motorSpeed;

	wait1Msec(150);
}

task shooter(){
	while(true){
		if(vexRT(Btn6U)){
			while(!vexRT(Btn6D))
				speedUpFlywheel();
		}
		else if(vexRT(Btn6D)){
			while(!vexRT(Btn6U))
				slowDownFlywheel();
		}
	}
}


void pre_auton()
{
	bStopTasksBetweenModes = true;
}


task autonomous()
{
	AutonomousCodePlaceholderForTesting();  // Remove this function call once you have "real" code.
}

task usercontrol()
{
	startTask(shooter);
	startTask(flywheelVelocity);
	while (true)
	{

		if(vexRT(Btn5U))
			motor[intake2] = 127;
		else if(vexRT(Btn5D))
			motor[intake2] = -127;
		else
			motor[intake2] = 0;
	}

}
