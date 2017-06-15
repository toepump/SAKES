#ifndef COUNTER_H
#define COUNTER_H

int indexOutput;
int netAngleIncrement = 0;                  //storage for temporary netAngleIncrement, to copy in to netAngleIncrement
int RealNetAngleIncrement = 0;              //storage for actual netAngleIncrement, used while being probed
double outputNetAngle[MAX_PULSE];           // the storage space for the netAngle debug
int outputState[MAX_PULSE];                 // the storage space for the state debug
int outputNetIncrement[MAX_PULSE];          //Store the value at each interrupt
int failInt;
double netAngleDegree=0; //the net angle in degree

void counter(int channelSig);

#endif
