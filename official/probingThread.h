#ifndef PROBINGTHREAD_H
#define PROBINGTHREAD_H

double probeAngleDeg[PROBE_STORAGE_SIZE];   // the strorage space for probed data
int probeIncrement[PROBE_STORAGE_SIZE];     // the storage space for the probed incremental data
int index;
int indexOutput;

/*
Purpose: every millisecond, forcibly take mutex protecting encoder data
         and do complete processing of that data within 1 millisecond.
*/
void *probingThread(void *ptr);

void printProbe(void);


#endif
