// alarm.cc
//	Routines to use a hardware timer device to provide a
//	software alarm clock.  For now, we just provide time-slicing.
//
//	Not completely implemented.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "alarm.h"
#include "main.h"

//----------------------------------------------------------------------
// Alarm::Alarm
//      Initialize a software alarm clock.  Start up a timer device
//
//      "doRandom" -- if true, arrange for the hardware interrupts to 
//		occur at random, instead of fixed, intervals.
//----------------------------------------------------------------------

Alarm::Alarm(bool doRandom)
{
    timer = new Timer(doRandom, this);
}

//----------------------------------------------------------------------
// Alarm::CallBack
//	Software interrupt handler for the timer device. The timer device is
//	set up to interrupt the CPU periodically (once every TimerTicks).
//	This routine is called each time there is a timer interrupt,
//	with interrupts disabled.
//
//	Note that instead of calling Yield() directly (which would
//	suspend the interrupt handler, not the interrupted thread
//	which is what we wanted to context switch), we set a flag
//	so that once the interrupt handler is done, it will appear as 
//	if the interrupted thread called Yield at the point it is 
//	was interrupted.
//
//	For now, just provide time-slicing.  Only need to time slice 
//      if we're currently running something (in other words, not idle).
//	Also, to keep from looping forever, we check if there's
//	nothing on the ready list, and there are no other pending
//	interrupts.  In this case, we can safely halt.
//----------------------------------------------------------------------

void 
Alarm::CallBack() 
{
    Interrupt *interrupt = kernel->interrupt;
    MachineStatus status = interrupt->getStatus();	
	
	bool WakeUp = false;

    // if someone to wakeup, then put it back to ready list,and pop
	if(kernel->scheduler->WakeUp()){
		WakeUp = true;
	}

    /* Added at HW2 */
//    kernel->currentThread->setPriority(kernel->currentThread->getPriority() - 1);

	// Check also Block status, and if someone wakeup do context switch
    if (status == IdleMode && kernel->scheduler->IsBlockEmpty() && !WakeUp) {	// is it time to quit?
        if (!interrupt->AnyFutureInterrupts()) {
	    timer->Disable();	// turn off the timer
	}
    } else {			// there's someone to preempt
        /* Added at HW2 */
        if(kernel->scheduler->getSchedulerType() == RR ||
            kernel->scheduler->getSchedulerType() == Priority_pt ||
            kernel->scheduler->getSchedulerType() == SRTF) { // RR, Priority-preempt,SRTF than preempt
            interrupt->YieldOnReturn();
        }
    }
}


//-------------------------------------
// Alarm::WaitUntil()
// Suspend execution until time > now + x
// **Writen by @shungfu**
//-------------------------------------

void
Alarm::WaitUntil(int x)
{
	// turn off the interrupt
	IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
	
	Thread *thread = kernel->currentThread;
	thread->SetSleepTime(x);
	
	// Make the thread Sleep
    kernel->scheduler->Blocked(thread);
	thread->Sleep(false);

	// turn on the interrupt
	kernel->interrupt->SetLevel(oldLevel);
}



