#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, dgtl12, ballHigh,       sensorTouch)
#pragma config(Sensor, I2C_1,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Motor,  port1,           feeder,        tmotorVex393TurboSpeed_HBridge, openLoop)
#pragma config(Motor,  port2,           LUflywheel,    tmotorVex393TurboSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port3,           LDflywheel,    tmotorVex393TurboSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port4,           LBMdrive,      tmotorVex393TurboSpeed_MC29, openLoop)
#pragma config(Motor,  port5,           LFdrive,       tmotorVex393TurboSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port6,           RBMdrive,      tmotorVex393TurboSpeed_MC29, openLoop)
#pragma config(Motor,  port7,           RFdrive,       tmotorVex393TurboSpeed_MC29, openLoop)
#pragma config(Motor,  port8,           RUflywheel,    tmotorVex393TurboSpeed_MC29, openLoop, encoderPort, I2C_1)
#pragma config(Motor,  port9,           RDflywheel,    tmotorVex393TurboSpeed_MC29, openLoop)
#pragma config(Motor,  port10,          intake1,       tmotorVex393TurboSpeed_HBridge, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX)

//Competition Control and Duration Settings
#pragma competitionControl(Competition)
#pragma autonomousDuration(20)
#pragma userControlDuration(120)

#include "Vex_Competition_Includes.c"   //Main competition background code...do not modify!
int motorSpeed = 0;
bool loadFireMode = false;
//float velocity;
int waitTime = 550;
int waitTimeP = 100;

void flywheelSpeed (int speed) {
	while(speed != motorSpeed) {
		if (speed > 127)									//If we give a speed that's too high, discard it
			speed = 127;
		else if (speed < 0)								//If we give a speed that would make the wheels spin negatively, set it to 0
			speed = 0;
		else if (speed < motorSpeed)      //If the wheels are too fast, lower the speed by 1 per x msec
			motorSpeed -= 1;
		else if (speed > motorSpeed)			//If the wheels are going too slow, raise the speed by 1 per x msec
			motorSpeed += 1;
		else															//catch any exceptions
			motorSpeed = speed;

		//set the speeds of the wheels
		motor[LUflywheel] = motorSpeed;
		motor[LDflywheel] = motorSpeed;
		motor[RUflywheel] = motorSpeed;
		motor[RDflywheel] = motorSpeed;

		wait1Msec(110);  //x msec delay
	}
}

task shooter(){
	//loop it
	while(true){

		if (vexRT(Btn7L))				//Slowly kill motors
			flywheelSpeed(0);

		else if (vexRT(Btn6U))	//Slow speed
			flywheelSpeed(75);

		else if (vexRT(Btn6D))	//Fast speed
			flywheelSpeed(60);

		else if(!loadFireMode && vexRT(Btn8L))
			flywheelSpeed(50);		//idle speed for flywheel

		wait1Msec(25); //25msec delay
	}
}



task drive(){
	//Drive loop
	while(true){
		motor[LFdrive] 	= vexRT(Ch2);
		motor[LBMdrive]	= vexRT(Ch2);
		motor[RFdrive] 	= vexRT(Ch3);
		motor[RBMdrive]	= vexRT(Ch3);
		wait1Msec(25);
	}
}

task loadFire() {
	int timesShot = 0;
		flywheelSpeed(72);
	while(true){
		clearTimer(T1); 								//Clear the timer
		while(!SensorValue(ballHigh))		//Get a ball into the top posision
			motor[feeder] = 127;
		while(SensorValue(ballHigh)) {	//When there is a ball in the top position...
			if(time1(T1) < waitTime )			//If we still need to wait more,
				motor[feeder] = 0;					//don't shoot.
			else													//If we've waited enough time,
				motor[feeder] = 127;				//shoot.
				if(timesShot != 0) {
					motor[LUflywheel] = 100;
					motor[LDflywheel] = 100;
					motor[RUflywheel] = 100;
					motor[RDflywheel] = 100;
				}
			timesShot++;
			wait1Msec(200);								//Wait for the ball to fully leave the intake
		}
		wait1Msec(25);									//25msec delay
	}
}

task loadFirePipe() {
	int timesShot = 0;
		flywheelSpeed(60);
	while(true){
		clearTimer(T3); 								//Clear the timer
		while(!SensorValue(ballHigh))		//Get a ball into the top posision
			motor[feeder] = 127;
		while(SensorValue(ballHigh)) {	//When there is a ball in the top position...
			if(time1(T3) < waitTimeP )			//If we still need to wait more,
				motor[feeder] = 0;					//don't shoot.
			else													//If we've waited enough time,
				motor[feeder] = 127;				//shoot.
				if(timesShot != 0) {
					motor[LUflywheel] = 80;
					motor[LDflywheel] = 80;
					motor[RUflywheel] = 80;
					motor[RDflywheel] = 80;
				}
			timesShot++;
			wait1Msec(200);								//Wait for the ball to fully leave the intake
		}
		wait1Msec(25);									//25msec delay
	}
}

task intake() {
	int swap = 0;
	while(true){

		//Begin shooting balls
		if(vexRT(Btn7D)){
			startTask(loadFirePipe);
			loadFireMode = true;
			swap = 1;
		}

		//Stop shooting balls
		else if(vexRT(Btn7U)){
			stopTask(loadFirePipe);
			loadFireMode = false;
			motor[feeder] = 0;
			swap = 0;
		}

		//Begin shooting balls
		else if(vexRT(Btn8D)){
			startTask(loadFire);
			loadFireMode = true;
			swap = 1;
		}

		//Stop shooting balls
		else if(vexRT(Btn8U)){
			stopTask(loadFire);
			loadFireMode = false;
			motor[feeder] = 0;
			swap = 0;
		}
		//Kills everything if we stop firing
		else if(swap == 0){
			motor[intake1] = 0;
			motor[feeder] = 0;
		}

		//Rake in balls
		if(vexRT(Btn5U)){
			motor[intake1] = 127;

		}

		//Manual feeder control
		if(vexRT(Btn5D)){
			motor[feeder] = 127;
		}

		wait1Msec(25); //25msec delay
	}
}

void pre_auton()
{
	bStopTasksBetweenModes = true;
}

task autonomous()
{
	clearTimer(T2);
	while(motorSpeed<90)
		flywheelSpeed(90); 		//gets it to 90 before we start loadFire
	startTask(loadFire);
	while(time1(T2)<10000)
		flywheelSpeed(90); 		//maintains 90
	while(time1(T2)>10000) 	//when there is 5 seconds left
		flywheelSpeed(0);			//begin to slow down
}

task usercontrol()
{
	startTask(shooter);
	startTask(drive);
	startTask(intake);
}
