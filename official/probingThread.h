#ifndef PROBINGTHREAD_H
#define PROBINGTHREAD_H

/*
Purpose: every millisecond, forcibly take mutex protecting encoder data
         and do complete processing of that data within 1 millisecond.
*/
void *probingThread(void *ptr);

void printProbe(void);


#endif
