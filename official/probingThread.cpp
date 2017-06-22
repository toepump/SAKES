#include "common.h"
#include "probingThread.h"
/*
Purpose: every millisecond, forcibly take mutex protecting encoder data
         and do complete processing of that data within 1 millisecond.
*/
void *probingThread(void *ptr){
    //do stuff on mutex protected data
    dataMtx.lock();
    probeAngleDeg[indexProbe] = (double)RealNetAngleIncrement/ PULSE_PER_DEGREE; //store current netAngleDegree
    dataMtx.unlock();
    indexProbe++;
    //release mutex if finished
    mtx.unlock();
    return (void*) NULL;
}
