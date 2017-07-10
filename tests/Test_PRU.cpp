/*
 * TestRT_3.cpp
 *
 * Created on: May 15, 2017
 *   Author: Vincent
 */

#include <iostream>
#include <unistd.h>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sched.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/stat.h>

using namespace std;


#define NSEC_PER_SEC  (1000000000) /* The number of nsecs per sec. */
#define MAX_BUFFER_SIZE 512
#define DEVICE_PATH "/dev/rpmsg_pru31"

void timespec_diff(struct timespec *start, struct timespec *stop,struct timespec *result);

int setParamThreadFIFO(pthread_attr_t attr, struct sched_param param, int priority);
void polyEval(double coeffs[], struct encoder *encoder, struct timespec *monotonicTime);
void polyAngToIncAng(double *polyAng, struct encoder *encoder);
int copyCurrToPrevEnc(struct encoder *previous, struct encoder *current);
int fetchAngInc(struct encoder *sourceEnc, struct encoder *current);
int angleIncToDeg (struct encoder *current);
int calcVelAndAcc(struct encoder *current, struct encoder *previous);
int controller(struct encoder *encKnee, struct encoder *encMotor, struct motor *cmdMotor);
int cmdMotor(struct motor *cmdMotor);
int storeIntoOutput(struct encoder *encKnee, struct encoder *encMotor, struct motor *cmdMotor, struct output *output, int increment);
int storeEncoderStruct(struct encoder *encoder, struct outputEnc *output, int increment);
int fileTestMotor(struct output *output);
int fileOutputEncoder(struct outputEnc *output);


int setTimeOrigin(struct timeStruct *time);
int getTimeSinceOrigin(struct timeStruct *time);

void *testThread1(void *ptr);
void *testThread2(void *ptr);


const int TIME_MAX = 100; // time max for the loop in ms
const int TIME_MAX_ENC = 5110; // time max for the loop in ms

const int INTERVALMS =1000000; // in nanosecond

const int INTERVAL_T1 = 10000000; //in nanosecond, interval for the thread 2
const int INTERVAL_T2 = 900000; //in nanosecond, interval for the thread 2

const int ONESECINNANO = 1000000000; //one second in nanosecond unit
double INTERVAL_S=double(INTERVALMS)/1000000000.0;

int ticks_t1=0; //Incremental value for the thread 1
int ticks_t2=0; //Incremental value for the thread 2
int sizeArrayOuput=5000;


struct encoder{
	int angInc; //value of the angle in increment
	double angDeg; //value of the angle in degree
	double velDegSec; //the velocity in deg/sec
	double accDegSec; //the acceleration in deg/secˆ2
	int pulsePerTurn; //number of increment to make one turn for one channel
	int numOfChannel; //number of channel used for this encoder
	int numOfEdge; //number of edge detected for this encoder for one channel (either 1 or 2)
	double timeFetchMilli; //the time when the angle has been fect in millisecond

	int monotonicTimeEvalSec;
	int monotonicTimeEvalNano;

	double angToInc; //to convert from ang to inc
};

struct motor{
	const double dutyMin; //Value of duty min set in the maxon board
	const double dutyMax; //Value of duty max set in the maxon board
	const double velMotorMin; //Value of the velocity min set on the maxon board in rpm
	const double velMotorMax; //Value of the velocity max set on the maxon board in rpm
	const double degSecToRPM; //to convert a speed from deg/sec to rpm
	double gearRatio; //from knee to motor, when the motor does one turn, how many turn does the knee
	double currentVelocity; //Value of the current velocity in rpm
	double currentDuty; //Value of the current duty
	double desiredVelocity; //Value of the desired velocity in rpm
	double desiredDuty; //Value of the desired duty

};

struct outputEnc{

	int AngInc[5000]; //value of the angle in increment
	double AngDeg[5000]; //value of the angle in degree

	int monotonicTimeEvalSec[5000];
	int monotonicTimeEvalNano[5000];
	};

struct output{
	double motorCurrVelocity[5000]; //Value of the current velocity in rpm
	double motorCurrDuty[5000]; //Value of the current duty
	double motorDesVelocity[5000]; //Value of the desired velocity in rpm
	double motorDesDuty[5000]; //Value of the desired duty

	int kneeAngInc[5000]; //value of the angle in increment
	double kneeAngDeg[5000]; //value of the angle in degree
	double kneeVelDegSec[5000]; //the velocity in deg/sec
	double kneeAccDegSec[5000]; //the acceleration in deg/secˆ2

	int motorAngInc[5000]; //value of the angle in increment
	double motorAngDeg[5000]; //value of the angle in degree
	double motorVelDegSec[5000]; //the velocity in deg/sec
	double motorAccDegSec[5000]; //the acceleration in deg/secˆ2

	double timeInMilliKnee[5000]; //the time in millisecond since hte beginning of the fetching
	double timeInMilliMotor[5000]; //the time in millisecond since hte beginning of the fetching
	};

struct timeStruct{

	double originSec; //the origin of time in second
	double originNano; //the origin of time, the part in nanosecond
	double originGlobalMilli; //The origin time, with oth part, nano and second, expressed in ms

	double tSec; //the time in sec
	double tNano; //the part in nano sec
	double tMilli; //the global time in ms

};

const int MAXDEGREEPOLY=57; //57 for coeffs1,  112 for coeffs2 or 3

double coeffsKnee1[MAXDEGREEPOLY]={ 2161704178.57744, -7678966834.50137, 7321336263.45535, 0.0, 0.0, 0.0, -4367145721.75599, 0.0, 0.0, 3978221771.45533, 0.0, 0.0, -985186083.37859, 0.0, 0.0, 0.0, 0.0, -2539164036.74563, 0.0, 0.0, 0.0, 5490325943.05484, 0.0, 0.0, 0.0, -7322471014.88356, 0.0, 0.0, 0.0, 8222278890.26401, 0.0, 0.0, 0.0, -10920714548.72234, 0.0, 0.0,16644524938.81657, 0.0, -16364372664.70290, 0.0,8080579968.97197, 0.0, 0.0, -2834740327.51965, 0.0,  1331213566.88150,  1010961461.30280, -2387418495.97765,1659337298.89251, -622192768.04629, 138921217.362358, -18305060.8984875,1320820.91241054, -50395.6593240169, 1594.07054186194, 38.7663699452653,11.8011809149536};
double coeffsMotor1[MAXDEGREEPOLY]={ 60*2161704178.57744, -60*7678966834.50137, 60*7321336263.45535, 0.0, 0.0, 0.0, -60*4367145721.75599, 0.0, 0.0, 60*3978221771.45533, 0.0, 0.0, -60*985186083.37859, 0.0, 0.0, 0.0, 0.0, -60*2539164036.74563, 0.0, 0.0, 0.0, 60*5490325943.05484, 0.0, 0.0, 0.0, -60*7322471014.88356, 0.0, 0.0, 0.0, 60*8222278890.26401, 0.0, 0.0, 0.0, -60*10920714548.72234, 0.0, 0.0,60*16644524938.81657, 0.0, -60*16364372664.70290, 0.0,60*8080579968.97197, 0.0, 0.0, -60*2834740327.51965, 0.0,  60*1331213566.88150,  60*1010961461.30280, -60*2387418495.97765,60*1659337298.89251, -60*622192768.04629, 60*138921217.362358, -60*18305060.8984875,60*1320820.91241054, -60*50395.6593240169, 60*1594.07054186194, 60*38.7663699452653,60*11.8011809149536};

//double coeffs2[MAXDEGREEPOLY]={-7804126.68756267, 0, 25506481.892212, 0, 0, -31995497.6264176, 0, 0, 0, 0, 0, 0, 0, 60180709.9392192, 0, 0, 0, 0, -91290146.7205068, 0, 0, 0, 0, 0, 62966686.2936572, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -41489120.4273827, 0, 0, 0, 0, 0, 0, 0, 0, 0, 51569710.2406328, 0, 0, 0, 0, 0, 0, 0, 0, 0 , -61482650.4011263, 0, 0, 0, 0, 0, 0, 0, 0, 0, 119151566.929812, 0, 0, 0, 0, -158836801.811755, 0, 0, 0, 0, 107845792.501427, 0, 0, 0, 0, -40438704.5594945, 0, 0, 0, 0, -2017204.55659056, 0, 0, 0, 22368787.3291233, 0, 0, 0, -38587201.2961324,0, 0, 68338463.6321268, 0, -91431570.8652916, 0, 136730933.950438, -146381938.709179, 76449935.5389264, -22951812.7425929, 3901914.86045682, -302425.558782687, -2995.35800355861, 1143.64231358147, 70.3494501822929, 11.3679676751559};
//double coeffs3[MAXDEGREEPOLY]={-5568914.69050683, 0, 18550269.9593775, 0, 0, -23986691.8154377, 0, 0, 0, 0, 0, 0, 0, 49531094.6617748, 0, 0, 0, 0, -80515111.3284084, 0, 0, 0, 0, 0, 61150107.658354, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -52944834.5893432, 0, 0, 0, 0, 0, 0, 0, 0, 0, 88145386.7882361, 0, 0, 0, 0, 0, 0, 0, 0, 0, -156446891.058224, 0, 0, 0, 0, 0, 0, 0, 0, 0, 533593350.532314, 0, 0, 0, 0, -1048894258.17445, 0, 0, 0, 0, 1199016711.57069, 0, 0, 0, 0, -1033184989.92329, 0, 0, 0, 0, 862028565.726653, 0, 0, 0, -705213545.729899, 0, 0, 0, 614832548.422423, 0, 0, -746208588.092968, 0, 791392111.350242, 0, -947590096.72696, 914359301.492975, -436255303.466556, 123792199.437433, -21816443.1312183, 2387498.2633569, -158624.061403544, 4988.62315434288, 158.259318872659, 11.4720337508105};

//Specification of the knee encoder E30S4-3000-6-L-5 from Autonics, with channel A and B activated
struct encoder kneePoly={.angInc=0, .angDeg=0.0, .velDegSec=0.0, .accDegSec=0.0, .pulsePerTurn=3000, .numOfChannel=2, .numOfEdge=2, .timeFetchMilli=0.0, .monotonicTimeEvalSec=0, .monotonicTimeEvalNano=0, .angToInc=12000.0/360.0}; //for the polynomial function
struct encoder motorPoly={.angInc=0, .angDeg=0.0, .velDegSec=0.0, .accDegSec=0.0, .pulsePerTurn=1024, .numOfChannel=4, .numOfEdge=1, .timeFetchMilli=0.0, .monotonicTimeEvalSec=0, .monotonicTimeEvalNano=0, .angToInc=4096.0/360.0};

struct encoder kneeCurrent={.angInc=0, .angDeg=0.0, .velDegSec=0.0, .accDegSec=0.0, .pulsePerTurn=3000, .numOfChannel=2, .numOfEdge=2, .timeFetchMilli=0.0, .monotonicTimeEvalSec=0, .monotonicTimeEvalNano=0, .angToInc=12000.0/360.0}; //to be use for real
struct encoder kneePrevious={.angInc=0, .angDeg=0.0, .velDegSec=0.0, .accDegSec=0.0, .pulsePerTurn=3000, .numOfChannel=2, .numOfEdge=2, .timeFetchMilli=0.0, .monotonicTimeEvalSec=0, .monotonicTimeEvalNano=0, .angToInc=12000.0/360.0}; //to be use for real
struct encoder kneeProbing={.angInc=0, .angDeg=0.0, .velDegSec=0.0, .accDegSec=0.0, .pulsePerTurn=3000, .numOfChannel=2, .numOfEdge=2, .timeFetchMilli=0.0, .monotonicTimeEvalSec=0, .monotonicTimeEvalNano=0, .angToInc=12000.0/360.0}; //to fetch the value of kneePoly directly

struct encoder motorCurrent={.angInc=0, .angDeg=0.0, .velDegSec=0.0, .accDegSec=0.0, .pulsePerTurn=1024, .numOfChannel=4, .numOfEdge=1, .timeFetchMilli=0.0, .monotonicTimeEvalSec=0, .monotonicTimeEvalNano=0, .angToInc=4096.0/360.0}; //to be use for real
struct encoder motorPrevious={.angInc=0, .angDeg=0.0, .velDegSec=0.0, .accDegSec=0.0, .pulsePerTurn=1024, .numOfChannel=4, .numOfEdge=1, .timeFetchMilli=0.0, .monotonicTimeEvalSec=0, .monotonicTimeEvalNano=0, .angToInc=4096.0/360.0}; //to be use for real

//Specification of the motor
struct motor maxon1={.dutyMin=0.10, .dutyMax=0.90, .velMotorMin=-8000.0, .velMotorMax=8000.0, .degSecToRPM=(1.0/6.0), .gearRatio=(1.0/60.0),.currentVelocity=0.0, .currentDuty=0.0, .desiredVelocity=0.0, .desiredDuty=0.0};

struct timespec timePoly;
struct output outputArray;
struct outputEnc outputKneePoly;

void timespec_diff(struct timespec *start, struct timespec *stop, struct timespec *result){
    if ((stop->tv_nsec - start->tv_nsec) < 0) {
        result->tv_sec = stop->tv_sec - start->tv_sec - 1;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
    } else {
        result->tv_sec = stop->tv_sec - start->tv_sec;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec;
    }

    return;
}

void polyEval(double coeffs[], struct encoder *encoder, struct timespec *monotonicTime){

	//from the coefficient in coeffs[] and the time in @time, give the angle in @angle

	int i=0;
	double result=0.0;
	double timeT=0.0;
	timeT=double(monotonicTime->tv_nsec)/double(ONESECINNANO);

	//the time used must be expressed in second, between 0 and 1
	for(i=0;i<MAXDEGREEPOLY-1;i++){
		result+=coeffs[i]*pow(timeT,MAXDEGREEPOLY-1-i);
	}
	result+=coeffs[MAXDEGREEPOLY-1];
	encoder->angDeg=result;

	encoder->monotonicTimeEvalNano=monotonicTime->tv_nsec;
	encoder->monotonicTimeEvalSec=monotonicTime->tv_sec;

}

void polyAngToIncAng(double *polyAng, struct encoder *polyEnc){
	//put the value of the angle in degree to the value of angInc of the encoder


	polyEnc->angInc=int(*polyAng*polyEnc->angToInc);
}

int copyCurrToPrevEnc(struct encoder *previous, struct encoder *current){
	//Copy the value of the variable from @current to @previous
	previous->angInc=current->angInc;
	previous->angDeg=current->angDeg;
	previous->velDegSec=current->velDegSec;
	previous->accDegSec=current->accDegSec;
	previous->timeFetchMilli=current->timeFetchMilli;
	previous->monotonicTimeEvalSec=current->monotonicTimeEvalSec;
	previous->monotonicTimeEvalNano=current->monotonicTimeEvalNano;

	return 0;
}

int fetchAngInc(struct encoder *sourceEnc, struct encoder *current){
	//Copy the value of @sourceAngInc into @current.angInc,
	current->angInc=sourceEnc->angInc;
	current->timeFetchMilli=double(sourceEnc->monotonicTimeEvalNano)/1000000.0+double(sourceEnc->monotonicTimeEvalSec)*1000.0;
	current->monotonicTimeEvalSec=sourceEnc->monotonicTimeEvalSec;
	current->monotonicTimeEvalNano=sourceEnc->monotonicTimeEvalNano;

	return 0;
}

int angleIncToDeg (struct encoder *current){
	/*Take the value of an angle in increment and turn it into an angle in deg,
	 * using the number of pulse per turn, the number of channel used and the number of edge that create an interrupt.
	 */
	current->angDeg=double(current->angInc)/current->angToInc;
	return 0;
}

int calcVelAndAcc(struct encoder *current, struct encoder *previous){
	//Calculate the current value of the velocity and the acceleration
	current->velDegSec=(previous->angDeg-current->angDeg)/((previous->timeFetchMilli-current->timeFetchMilli)/1000.0);
	current->accDegSec=(previous->velDegSec-current->velDegSec)/((previous->timeFetchMilli-current->timeFetchMilli)/1000.0);
	return 0;
}

int controller(struct encoder *encKnee, struct encoder *encMotor, struct motor *cmdMotor){

	//Simple controller
	//Evaluate the velocity of the motor in RPM
	cmdMotor->desiredVelocity=encKnee->velDegSec*cmdMotor->degSecToRPM/cmdMotor->gearRatio;

	//Calculate the value of the desired duty
	cmdMotor->desiredDuty=(cmdMotor->desiredVelocity-cmdMotor->velMotorMin)*(cmdMotor->dutyMax-cmdMotor->dutyMin)/(cmdMotor->velMotorMax-cmdMotor->velMotorMin)+cmdMotor->dutyMin;

	//Put the former value int current
	cmdMotor->currentVelocity=cmdMotor->desiredVelocity;
	cmdMotor->currentDuty=cmdMotor->desiredDuty;

	return 0;
}

int cmdMotor(struct motor *cmdMotor){

	//TODO

	return 0;
}

int storeIntoOutput(struct encoder *encKnee, struct encoder *encMotor, struct motor *cmdMotor, struct output *output, int increment){

	//knee Encoder
	output->kneeAngInc[increment]=encKnee->angInc; //value of the angle in increment
	output->kneeAngDeg[increment]=encKnee->angDeg; //value of the angle in degree
	output->kneeVelDegSec[increment]=encKnee->velDegSec; //the velocity in deg/sec
	output->kneeAccDegSec[increment]=encKnee->accDegSec; //the acceleration in deg/secˆ2
	output->timeInMilliKnee[increment]=encKnee->timeFetchMilli; //the time in milli since the beginning of the experiment

	//Motor Encoder
	output->motorAngInc[increment]=encMotor->angInc; //value of the angle in increment
	output->motorAngDeg[increment]=encMotor->angDeg; //value of the angle in degree
	output->motorVelDegSec[increment]=encMotor->velDegSec; //the velocity in deg/sec
	output->motorAccDegSec[increment]=encMotor->accDegSec; //the acceleration in deg/secˆ2
	output->timeInMilliMotor[increment]=encMotor->timeFetchMilli; //the time in milli since the beginning of the experiment

	//Motor command
	output->motorCurrVelocity[increment]=cmdMotor->currentVelocity; //Value of the current velocity in rpm
	output->motorCurrDuty[increment]=cmdMotor->currentDuty; //Value of the current duty
	output->motorDesVelocity[increment]=cmdMotor->desiredVelocity; //Value of the desired velocity in rpm
	output->motorDesDuty[increment]=cmdMotor->desiredDuty; //Value of the desired duty

	return 0;
}

int storeEncoderStruct(struct encoder *encoder, struct outputEnc *output, int increment){

	//knee Encoder
	output->AngInc[increment]=encoder->angInc; //value of the angle in increment
	output->AngDeg[increment]=encoder->angDeg; //value of the angle in degree
	output->monotonicTimeEvalNano[increment]=encoder->monotonicTimeEvalNano; //the velocity in deg/sec
	output->monotonicTimeEvalSec[increment]=encoder->monotonicTimeEvalSec; //the acceleration in deg/secˆ2

	return 0;
}

int fileTestMotor(struct output *output){

	cout << "Printing of the output starts" << endl;

		int i=0;
		FILE *fj1=fopen("fileTestMotor.dat","w");

		fprintf(fj1,"indexOutput;"
				"MotorCurrentVelocity; MotorCurrentDuty; MotorDesiredVelocity; MotorDesiredDuty;"
				"TimeInMilliKneeEnc; KneeEncAngInc; KneeEncAngDeg; KneeEncVelDegsec; KneeAccDegsecsec;"
				"TimeInMilliMotorEnc; MotorEncAngInc; MotorEncAngDeg; MotorEncVelDegsec; MotorAccDegsecsec; \r\n");

		while(i<TIME_MAX){
		    fprintf(fj1,"%d;"
		    		"%f;%f;%f;%f;"
		    		"%f;%d;%f;%f;%f;"
		    		"%f;%d;%f;%f;%f;"
		    		"\r\n",
		    	i+1,
				output->motorCurrVelocity[i], output->motorCurrDuty[i],output->motorDesVelocity[i], output->motorDesDuty[i],
				output->timeInMilliKnee[i],output->kneeAngInc[i], output->kneeAngDeg[i], output->kneeVelDegSec[i], output->kneeAccDegSec[i],
				output->timeInMilliMotor[i],output->motorAngInc[i], output->motorAngDeg[i], output->motorVelDegSec[i],output->motorAccDegSec[i]);

		    if(i==TIME_MAX-1){
		    	fclose(fj1);
		    }
		    i++ ;
		}

	cout << "Printing of the output is done" << endl;

	return 0;
}

int fileOutputEncoder(struct outputEnc *output){

	cout << "Printing of the encoder output starts" << endl;

		int i=0;
		FILE *fj2=fopen("fileOutputEnc.dat","w");

		fprintf(fj2,"indexOutputPoly;"
				"PolyAngInc; PolyAngDeg; PolyMonotonicTimeSec; PolyMonotonicTimeNano; \r\n");

		while(i<TIME_MAX_ENC){
		    fprintf(fj2,"%d;"
		    		"%d;%f;%d;%d;"
		    		"\r\n",
		    	i+1,
				output->AngInc[i], output->AngDeg[i], output->monotonicTimeEvalSec[i], output->monotonicTimeEvalNano[i]);

		    if(i==TIME_MAX_ENC-1){
		    	fclose(fj2);
		    }
		    i++ ;
		}

	cout << "Printing of the encoder output is done" << endl;

	return 0;
}

int setTimeOrigin(struct timeStruct *time){

	struct timespec timeFetcher;
	clock_gettime(CLOCK_MONOTONIC ,&timeFetcher);

	time->originSec=double(timeFetcher.tv_sec);
	time->originNano=double(timeFetcher.tv_nsec);
	time->originGlobalMilli=time->originSec*1000.0+time->originNano/1000000.0;

	time->tSec=0.0;
	time->tNano=0.0;
	time->tMilli=0.0;

	return 0;
}

int getTimeSinceOrigin(struct timeStruct *time){

	struct timespec timeFetcher;
	clock_gettime(CLOCK_MONOTONIC ,&timeFetcher);

	time->tSec=double(timeFetcher.tv_sec)-time->originSec;
	time->tNano=double(timeFetcher.tv_nsec)-time->originNano;
	time->tMilli=time->tSec*1000.0+time->tNano/1000000.0;

	return 0;
}

int setParamThreadFIFO(pthread_attr_t attr, struct sched_param param, int priority){
	//Function: Set the attr and parm as a FIFO function with priority


	int checkParam; //Variable to check if the setting of the thread is okay

	//Create independent threads each of which will execute function
	pthread_attr_getschedparam(&attr, &param); // put the scheduling param of att to parm
	checkParam=param.sched_priority = priority; //return the minimum priority
	checkParam=pthread_attr_setschedpolicy(&attr, SCHED_FIFO); //set the scheduling policy of attr1 as FIFIO
	checkParam=pthread_attr_setschedparam(&attr, &param); //set the scheduling parameter of attr1 as parm1

	if(checkParam!=0){
		cout << "Problem in the initialization of a thread "<< endl;
		checkParam=0;
	}

	return checkParam;
}

int main(int argc, char* argv[]){

	//Creation of the thread
	pthread_t thread1, thread2;
	const char *message1 = "Thread 1";
	const char *message2 = "Thread 2";
	int iret1, iret2;

	pthread_attr_t attr1, attr2; //Creation of the variable for the attribute
	struct sched_param param1, param2; //Creation of new sched_param

	int checkAttrInit;
	int setInherited;
	int setPolicy;
	int checkschedParam;


	checkAttrInit=pthread_attr_init(&attr1); //Initialize the thread attributes with default attribute
	if(checkAttrInit!=0){
		printf("Problem attribute 1: %d \n", checkAttrInit);
	}

	setInherited=pthread_attr_setinheritsched(&attr1, PTHREAD_EXPLICIT_SCHED);
	if(setInherited!=0){
		printf("Problem set inherited 1: %d \n", setInherited);
	}

	setPolicy=pthread_attr_setschedpolicy(&attr1, SCHED_FIFO);
	if(setPolicy!=0){
		printf("Problem set policy 1: %d \n", setPolicy);
	}

	param1.sched_priority = 70;

	checkschedParam=pthread_attr_setschedparam(&attr1, &param1);
	if(checkschedParam!=0){
		printf("Problem set param 1: %d \n", checkschedParam);
	}

	iret1 = pthread_create(&thread1, &attr1, testThread1, (void*) message1);

	printf("pthread_create() 1 for returns: %d\n", iret1); // if it fails, return not 0

	/*

	checkAttrInit=pthread_attr_init(&attr2); //Initialize the thread attributes with default attribute
	if(checkAttrInit!=0){
		printf("Problem attribute 2: %d \n", checkAttrInit);
	}

	setInherited=pthread_attr_setinheritsched(&attr2, PTHREAD_EXPLICIT_SCHED);
	if(setInherited!=0){
		printf("Problem set inherited 2: %d \n", setInherited);
	}

	setPolicy=pthread_attr_setschedpolicy(&attr2, SCHED_FIFO);
	if(setPolicy!=0){
		printf("Problem set policy 2: %d \n", setPolicy);
	}


	param2.sched_priority = 70;

	checkschedParam=pthread_attr_setschedparam(&attr2, &param2);
	if(checkschedParam!=0){
		printf("Problem set param 1: %d \n", checkschedParam);
	}

	iret2 = pthread_create(&thread2, &attr2, testThread2, (void*) message2);
		printf("pthread_create() 2 for returns: %d\n", iret2);

	*/

	/* Wait till threads are complete before main continues. Unless we */
	/* wait we run the risk of executing an exit which will terminate  */
	/* the process and all threads before the threads have completed.  */

	pthread_join( thread1, NULL);
	pthread_join( thread2, NULL);

	exit(EXIT_SUCCESS);
}

void *testThread1(void *ptr) {

	char *message;
	message = (char *) ptr;
	struct timespec waitTime;
	struct timespec start;
	struct timespec previous_start;
	struct timespec diff;

	struct timespec sendMessage;
	struct timespec receiveMessage;
	struct timespec durationCommuciation;
	int answerTime[100];

	char readBuf[MAX_BUFFER_SIZE];
	struct pollfd pollfds[1];
	int result = 0;
	int number1;
	int number2;
	int number3;
	int number4;

	int finalResult;
	char filename[18] = "/dev/rpmsg_pru31";
	int fd;
	int toPru;

	int sleepOK=0;
	int i=0;

	//We set the begining if the thread in 1 second
	clock_gettime(CLOCK_MONOTONIC, &waitTime);
	waitTime.tv_sec+=1;

	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &waitTime, NULL);

	cout << "Beginning of loop " << endl;

	/* Open the rpmsg_pru character device file */
	pollfds[0].fd = open(DEVICE_PATH, O_RDWR);
	if (pollfds[0].fd < 0){
		printf("Failed to open \n");
	}

	while(ticks_t1<TIME_MAX+1){

		toPru=30;

		/* wait until next shot */
		if(sleepOK == 0){
			toPru=toPru+1;

			//wait to continue until the next waiTime (next millisecond)
			sleepOK=clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &waitTime, NULL);

			//get the time of the beginning of this cycle and calculate the interval since the previous cycle
			clock_gettime(CLOCK_MONOTONIC, &start);
			timespec_diff(&previous_start, &start, &durationCommuciation);


			//cout << "Before send to the PRU " << endl;


			//Get the time before sending a message
			clock_gettime(CLOCK_MONOTONIC, &sendMessage);

			//Message to the PRU through the RPMsg channel
			result = write(pollfds[0].fd, &toPru, sizeof(int));
			/*
			if (result > 0){
				printf("Message sent to PRU\n");
			}
			*/

			//test if we are respecting the time interval limit
			/*
			if(diff.tv_nsec>130000000)
			{
				cout << " " << endl;
				cout << "Waiting superior to 130 msec" << endl;
				cout << " " << endl;
			}
			*/
			result = read(pollfds[0].fd, readBuf, MAX_BUFFER_SIZE);
			//cout << "Result "<< result  << endl;
			if(result > 0){
			        number1= (int)(readBuf[0]);
			        number2= (int)(readBuf[1]);
			        number3= (int)(readBuf[2]);
			        number4= (int)(readBuf[3]);
			        //cout << "The number send by the PRU is : " << finalResult << endl;
			}else{
					cout << "Result not superior to 0 :"<< endl;
			}
			finalResult=number1+number2*256+number3*256*256+number4*256*256*256;

			//Get time after receiving the message
			clock_gettime(CLOCK_MONOTONIC, &receiveMessage);
			//Calculation of the tine difference between send and receive
			timespec_diff(&sendMessage, &receiveMessage, &diff);
			answerTime[ticks_t1]=sendMessage.tv_nsec;



	  		//put the value of the variable of start to previous start
		  	previous_start.tv_sec=start.tv_sec;
		  	previous_start.tv_nsec=start.tv_nsec;

		  	//Do the calculation for the next time so start the loop
		  	waitTime.tv_nsec+=INTERVAL_T1;
		  	if(waitTime.tv_nsec>= NSEC_PER_SEC){
		  		waitTime.tv_sec+=1;
		  		waitTime.tv_nsec-=NSEC_PER_SEC;
		  	}

		}else{
			cout << "The clock nanosleep has encountered a problem." << endl;
			cout << " " << endl;
			return (void*) NULL;
		}
		//cout << "Loop number : " << ticks_t1 << endl;
		ticks_t1=ticks_t1+1;
		if(ticks_t1==100){
			for(i=0;i<100;i++){
				cout << "Taaaa" << endl;
				cout << answerTime[i] << "  ";
			}
		}
	}

	/* Close the rpmsg_pru character device file */
	close(pollfds[0].fd);

	//We wait 2 seconds to output the files
	waitTime.tv_sec+=1;
	sleepOK=clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &waitTime, NULL);


	return (void*) NULL;
}

void *testThread2(void *ptr) {

	char *message;
	message = (char *) ptr;
	struct timespec t_Thread2;
	struct timespec calcAngle;

	double timeTestPoly=0.0;
	double timeRatio=double(INTERVALMS)/double(INTERVAL_T2);
	double maxTicks = double(TIME_MAX)*timeRatio+1000.0;

	/*Stuff I want to do*/
	/*here should start the things used with the rt preempt patch*/

  clock_gettime(CLOCK_MONOTONIC, &t_Thread2);
  /* start after one second */
  t_Thread2.tv_sec++;

  while(ticks_t2<int(maxTicks)) {

  	/* wait until next shot */
  	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t_Thread2, NULL);

  	//We get the time to
  	clock_gettime(CLOCK_MONOTONIC, &timePoly);

	//simulation of the polynomial
	polyEval(coeffsKnee1, &kneePoly, &timePoly); //put the value in angTestPoly
	polyEval(coeffsMotor1, &motorPoly, &timePoly);

	polyAngToIncAng(&kneePoly.angDeg, &kneePoly); //convert the value to ang
	polyAngToIncAng(&motorPoly.angDeg, &motorPoly); //convert the value to ang

	storeEncoderStruct(&kneePoly, &outputKneePoly, ticks_t2);

  	ticks_t2++; // Increment the ticks value

	/* calculate next shot */
  	t_Thread2.tv_nsec += INTERVAL_T2;
  	while (t_Thread2.tv_nsec >= NSEC_PER_SEC) {
  		t_Thread2.tv_nsec -= NSEC_PER_SEC;
  		t_Thread2.tv_sec++;
  	}
  }

  //We wait 2 seconds to output the files
  t_Thread2.tv_sec++;
  t_Thread2.tv_sec++;
  clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t_Thread2, NULL);

}
