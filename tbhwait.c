#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, dgtl11, slowDown,       sensorTouch)
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

// Update inteval (in mS) for the flywheel control loop
#define FW_LOOP_SPEED              25

// Maximum power we want to send to the flywheel motors
#define FW_MAX_POWER              127

// encoder counts per revolution depending on motor
#define MOTOR_TPR_269           240.448
#define MOTOR_TPR_393R          261.333
#define MOTOR_TPR_393S          392
#define MOTOR_TPR_393T          627.2
#define MOTOR_TPR_QUAD          360.0

// encoder tick per revolution
float           ticks_per_rev;          ///< encoder ticks per revolution

// Encoder
long            encoder_counts;         ///< current encoder count
long            encoder_counts_last;    ///< current encoder count

// velocity measurement
float           motor_velocity;         ///< current velocity in rpm
long            nSysTime_last;          ///< Time of last velocity calculation

// TBH control algorithm variables
long            target_velocity;        ///< target_velocity velocity
float           current_error;          ///< error between actual and target_velocity velocities
float           last_error;             ///< error last time update called
float           gain;                   ///< gain
float           drive;                  ///< final drive out of TBH (0.0 to 1.0)
float           drive_at_zero;          ///< drive at last zero crossing
long            first_cross;            ///< flag indicating first zero crossing
float           drive_approx;           ///< estimated open loop drive

// final motor drive
long            motor_drive;            ///< final motor control value

/*-----------------------------------------------------------------------------*/
/** @brief      Set the flywheen motors                                        */
/** @param[in]  value motor control value                                      */
/*-----------------------------------------------------------------------------*/
void
FwMotorSet( int value )
{
    motor[ LDflywheel ] = value;
    motor[ LUflywheel ] = value;
    motor[ RDflywheel ] = value;
    motor[ RUflywheel ] = value;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get the flywheen motor encoder count                           */
/*-----------------------------------------------------------------------------*/
long
FwMotorEncoderGet()
{
    return( nMotorEncoder[ RUflywheel ] );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Set the controller position                                    */
/** @param[in]  desired velocity                                               */
/** @param[in]  predicted_drive estimated open loop motor drive                */
/*-----------------------------------------------------------------------------*/
void
FwVelocitySet( int velocity, float predicted_drive )
{
    // set target_velocity velocity (motor rpm)
    target_velocity = velocity;

    // Set error so zero crossing is correctly detected
    current_error = target_velocity - motor_velocity;
    last_error    = current_error;

    // Set predicted open loop drive value
    drive_approx  = predicted_drive;
    // Set flag to detect first zero crossing
    first_cross   = 1;
    // clear tbh variable
    drive_at_zero = 0;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Calculate the current flywheel motor velocity                  */
/*-----------------------------------------------------------------------------*/
void
FwCalculateSpeed()
{
    int     delta_ms;
    int     delta_enc;

    // Get current encoder value
    encoder_counts = FwMotorEncoderGet();

    // This is just used so we don't need to know how often we are called
    // how many mS since we were last here
    delta_ms = nSysTime - nSysTime_last;
    nSysTime_last = nSysTime;

    // Change in encoder count
    delta_enc = (encoder_counts - encoder_counts_last);

    // save last position
    encoder_counts_last = encoder_counts;

    // Calculate velocity in rpm
    motor_velocity = (1000.0 / delta_ms) * delta_enc * 60.0 / ticks_per_rev;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Update the velocity tbh controller variables                   */
/*-----------------------------------------------------------------------------*/
void
FwControlUpdateVelocityTbh()
{
    // calculate error in velocity
    // target_velocity is desired velocity
    // current is measured velocity
    current_error = target_velocity - motor_velocity;

    // Calculate new control value
    drive =  drive + (current_error * gain);

    // Clip to the range 0 - 1.
    // We are only going forwards
    if( drive > 1 )
          drive = 1;
    if( drive < 0 )
          drive = 0;

    // Check for zero crossing
    if( sgn(current_error) != sgn(last_error) ) {
        // First zero crossing after a new set velocity command
        if( first_cross ) {
            // Set drive to the open loop approximation
            drive = drive_approx;
            first_cross = 0;
        }
        else
            drive = 0.5 * ( drive + drive_at_zero );

        // Save this drive value in the "tbh" variable
        drive_at_zero = drive;
    }

    // Save last error
    last_error = current_error;
}

/*-----------------------------------------------------------------------------*/
/** @brief     Task to control the velocity of the flywheel                    */
/*-----------------------------------------------------------------------------*/
task
FwControlTask()
{
    // Set the gain
    gain = 0.00006;

    // We are using Speed geared motors
    // Set the encoder ticks per revolution
    ticks_per_rev = MOTOR_TPR_393T;

    while(1)
        {
        // Calculate velocity
        FwCalculateSpeed();

        // Do the velocity TBH calculations
        FwControlUpdateVelocityTbh() ;

        // Scale drive into the range the motors need
        motor_drive  = (drive * FW_MAX_POWER) + 0.5;

        // Final Limit of motor values - don't really need this
        if( motor_drive >  127 ) motor_drive =  127;
        if( motor_drive < -127 ) motor_drive = -127;

        // and finally set the motor control value
        FwMotorSet( motor_drive );

        // Run at somewhere between 20 and 50mS
        wait1Msec( FW_LOOP_SPEED );
        }
}

// Main user task
task fwmain()
{
    char  str[32];

    bLCDBacklight = true;

    // Start the flywheel control task
    startTask( FwControlTask );

    FwVelocitySet( 65, 0.7 );
    // Main user control loop
    while(1)
        {
        // Different speeds set by buttons
        if( vexRT[ Btn8L ] == 1 )
            FwVelocitySet( 144, 0.55 );
        if( vexRT[ Btn8U ] == 1 )
            FwVelocitySet( 120, 0.38 );
        if( vexRT[ Btn8R ] == 1 )
            FwVelocitySet( 50, 0.2 );
        if( vexRT[ Btn8D ] == 1 )
            FwVelocitySet( 00, 0 );

        // Display useful things on the LCD
        sprintf( str, "%4d %4d  %5.2f", target_velocity,  motor_velocity, nImmediateBatteryLevel/1000.0 );
        displayLCDString(0, 0, str );
        sprintf( str, "%4.2f %4.2f ", drive, drive_at_zero );
        displayLCDString(1, 0, str );

        // Don't hog the cpu :)
        wait1Msec(10);
        }
}

task feederWait () {
	clearTimer(T4);
	while(true) {
		if(!SensorValue[ballHigh]) {
			motor[feeder] = 127;
		} else if(time1[T4]>=1500) {
			motor[feeder] = 127;
			clearTimer(T4);
			wait1Msec(200);
		} else {
			motor[feeder] = 0;
		}
		wait1Msec(25);
	}
}

void pre_auton() {
  // Set bStopTasksBetweenModes to false if you want to keep user created tasks running between
  // Autonomous and Tele-Op modes. You will need to manage all user created tasks if set to false.
  bStopTasksBetweenModes = true;

	// All activities that occur before the competition starts
	// Example: clearing encoders, setting servo positions, ...
}

task autonomous() {

	AutonomousCodePlaceholderForTesting();  // Remove this function call once you have "real" code.
}


task usercontrol() {
	nMotorEncoder[RUflywheel]=0;
	startTask(fwmain);
	startTask(feederWait);
	while(true){
		if(SensorValue[slowDown]){
			motor[feeder]=0;
			stopTask(fwmain);
			stopTask(FwControlTask);
			stopTask(feederWait);
			int motorSpeed = motor[LUflywheel];
			while(motorSpeed>0){
				motorSpeed-=2;
				FwMotorSet(motorSpeed);
				wait1Msec(50);
			}
			stopAllTasks();
		}
	}

}
