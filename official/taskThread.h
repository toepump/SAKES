#ifndef TASKTHREAD_H
#define TASKTHREAD_H

#include <time.h>
#include <pthread.h>
#include <mutex>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <sched.h>

/*
Purpose: thread for spwaning a probing thread every millisecond
         only launches a thread if previously spawned thread has Finished
         Otherwise, program breaks because of control discrepancy
*/
void *taskThread(void *ptr);
void printProbe(void);

#endif
