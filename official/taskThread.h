#ifndef TASKTHREAD_H
#define TASKTHREAD_H

std::mutex mtx;  //probingThread mutex
int INTERVAL;

/*
Purpose: thread for spwaning a probing thread every millisecond
         only launches a thread if previously spawned thread has Finished
         Otherwise, program breaks because of control discrepancy
*/
void *taskThread(void *ptr);

endif
