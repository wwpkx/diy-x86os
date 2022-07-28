#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define _UTHREAD_PRIVATE
#include "preempt.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100

void preempt_save(sigset_t *level)
{
	// save current preemption status and disable preemption
	if(sigprocmask(0, NULL, level) == -1){
		fprintf(stderr, "Failure to save signal mask.\n");
		return;
	}
	preempt_disable();
}

void preempt_restore(sigset_t *level)
{
	// Reset current mask to `level` mask
	sigprocmask(SIG_SETMASK,level,NULL);
}

void preempt_enable(void)
{
	sigset_t newSignal;
	sigemptyset(&newSignal);		
	// Add Alarm Signal 
	sigaddset(&newSignal,SIGVTALRM);
	// block Alarm Signal
	sigprocmask(SIG_UNBLOCK,&newSignal,NULL);
}

void preempt_disable(void)
{
	// create a new signal.
	sigset_t newSignal;
	// Set signal to empty 
	sigemptyset(&newSignal);		
	// Add Alarm Signal
	sigaddset(&newSignal,SIGVTALRM); 
	// block Alarm Signal
	sigprocmask(SIG_BLOCK,&newSignal,NULL);
}

bool preempt_disabled(void)
{
	sigset_t currentMask;
	sigprocmask(0, NULL, & currentMask);
	if(sigismember(&currentMask,SIGVTALRM ))
	{
		return true;
	}
	return false;
}

/*
 * timer_handler - Timer signal handler (aka interrupt handler)
 * @signo - Received signal number (can be ignored)
 */
static void timer_handler(int signo)
{
	// Force currently running thread to yield.
	uthread_yield();
}

void preempt_start(void)
{
	struct sigaction sa;
	struct itimerval it;

	/*
	 * Install signal handler @timer_handler for dealing with alarm signals
	 */
	sa.sa_handler = timer_handler;
	sigemptyset(&sa.sa_mask);
	/* Make functions such as read() or write() to restart instead of
	 * failing when interrupted */
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGVTALRM, &sa, NULL)) {
		perror("sigaction");
		exit(1);
	}

	/*
	 * Configure timer to fire alarm signals at a certain frequency
	 */
	it.it_value.tv_sec = 0;
	it.it_value.tv_usec = 1000000 / HZ;
	it.it_interval.tv_sec = 0;
	it.it_interval.tv_usec = 1000000 / HZ;
	if (setitimer(ITIMER_VIRTUAL, &it, NULL)) {
		perror("setitimer");
		exit(1);
	}
}

