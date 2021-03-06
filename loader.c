#pragma config(Sensor, S1, touchOrigin, sensorTouch)
#pragma config(Sensor, S2, touchOnTop, sensorTouch)
#pragma config(Sensor, S3, colorSensor, sensorCOLORFULL)
//#pragma config(Sensor, S4, sonarSensor,  sensorSONAR)

#include "constants.h"
#include "motor.c"


#define PushMotor motorA
#define LiftMotor motorB
const float LiftGear = 24;  // 8/3.0;
const float PushGear = 24;

void loadPlate();
int * readBarcode();

void clearInbox(){
	// Clear the message inbox
	for(int i=0; i<100; i++)
		ClearMessage();
}

task listenToBluetooth(){
	int receiver, method, payload;
	while(true)
	{
		receiver = messageParm[0];
		method = messageParm[1];
		payload = messageParm[2];
		if(receiver != 0 || method	!= 0 || payload != 0){
			PlaySound(soundBlip);
			switch(method){
			case LOADER_LOAD_PLATE:
				sendMessageWithParm(WEBSERVER, STT_LOADING, 0);
				loadPlate();
				break;
			default:
				PlaySound(soundException);
				// method not supported
			}
			ClearMessage();
		}
		wait1Msec(500);
	}
}

int barCode[4];
int* readBarcode(){
	barCode[0]=1;
	int k=0;
	int i=1;

	while (i < 4) {
		while ( SensorValue[colorSensor]!=REDCOLOR &&
			SensorValue[colorSensor]!=BLUECOLOR &&
		SensorValue[colorSensor]!=WHITECOLOR &&
		SensorValue[colorSensor]!=YELLOWCOLOR &&
		SensorValue[colorSensor]!=GREENCOLOR ) {
			// do nothing -- just wait for above colors
		}

		if (SensorValue[colorSensor]!=barCode[i-1] &&	SensorValue[colorSensor]!=BLACKCOLOR){
				barCode[i]= SensorValue [ colorSensor ];
				k = 10 * k + barCode[i];
				i=i+1;
				wait1Msec(50);
		}
	}
	PlaySound(soundBeepBeep);
	sendMessageWithParm(WEBSERVER, STT_BARCODE , k);
	return	barCode;
}


void moveToOrigin(){
	// make sure touch sensor is untriggered
	while(true)
	{
		if(SensorValue[touchOrigin] == 0)
			motor[PushMotor] = -20;
		else
			break;
	}
	motor[PushMotor] = 0;


	driveGear(0.5,10,PushMotor, PushGear);
	PlaySound(soundShortBlip);
}

void moveLiftToOrigin(){
	// make sure touch sensor is untriggered
	while(true)
	{
		if(SensorValue[touchOnTop] == 0)
			motor[LiftMotor] = -20;
		else
			break;
	}
	motor[LiftMotor] = 0;


	driveGear(0.5,10,LiftMotor, LiftGear);
	PlaySound(soundShortBlip);
}



void pushForward(){
	driveGear(36,20,PushMotor, PushGear);
	driveGear(7,5,PushMotor, PushGear);
}

void pushDown(){
	//load the plate on conveyor
	driveGear(15,20,LiftMotor,LiftGear)	;
	driveGear(18,10,LiftMotor,LiftGear)	;

	//repeat punching the plate
	int i = 0;
	while(i<4)
	{
		driveGear(3,-40,LiftMotor,LiftGear)	;
		driveGear(4.5,10,LiftMotor,LiftGear)	;
		wait1Msec(500);
		i++;
	}
}

// Check whether there are enough plates
//bool havePlate(){
//	const int distance_empty_plate=14;												//16; //distance for empty warehouse status
//	for(int c=0; c<5; c++){
//		if(SensorValue[sonarSensor] < distance_empty_plate)
//			return true;
//		wait1Msec(100);
//	}
//	return false;
//}

void loadPlate(){
	// Sonar sensor returns incorrect values due to specular effects
	//if(!havePlate()){
	//	sendMessageWithParm(WEBSERVER, ERR_NO_PLATES, SensorValue[sonarSensor]);
	//	PlaySound(soundException);
	//	return;
	//}

	// Push a plate to the conveyor
	driveGear(2,30,PushMotor, PushGear);
	moveToOrigin();
	moveLiftToOrigin();
	pushForward();
	moveToOrigin();

	// Plug into convetyor
	pushDown();
	moveLiftToOrigin();

	sendMessageWithParm(CONVEYOR, CONVEYOR_PLATE_LOADED, 0);
	readBarcode();
}

//Main Program
task main()
{
	clearInbox();
	StartTask(listenToBluetooth);
	while(true){wait10Msec(100);}
	return;
}