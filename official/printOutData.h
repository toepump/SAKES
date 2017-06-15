#ifndef PRINTOUTDATA_H
#define PRINTOUTDATA_H

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>

// double outputNetAngle[MAX_PULSE];           // the storage space for the netAngle debug
// int outputState[MAX_PULSE];                 // the storage space for the state debug
// int outputNetIncrement[MAX_PULSE];          //Store the value at each interrupt

/*
Purpose: print out data collected from the encoders including:
         index, interrupt number, calculated angle of encoder, and state
*/
void printOutData(void);

#endif
