// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"

#include "synch.h"
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
	int	type = kernel->machine->ReadRegister(2);
	int	val;
    Lock* pageLock=NULL;

    switch (which) {
	case SyscallException:
	    switch(type) {
		case SC_Halt:
		    DEBUG(dbgAddr, "Shutdown, initiated by user program.\n");
   		    kernel->interrupt->Halt();
		    break;
		case SC_PrintInt:
			val=kernel->machine->ReadRegister(4);
			cout << "Print integer:" <<val << endl;
			return;
/*		case SC_Exec:
			DEBUG(dbgAddr, "Exec\n");
			val = kernel->machine->ReadRegister(4);
			kernel->StringCopy(tmpStr, retVal, 1024);
			cout << "Exec: " << val << endl;
			val = kernel->Exec(val);
			kernel->machine->WriteRegister(2, val);
			return;
*/		case SC_Exit:
			DEBUG(dbgAddr, "Program exit\n");
			val=kernel->machine->ReadRegister(4);
			cout << "return value:" << val << endl;
			kernel->currentThread->Finish();
			break;
		// Practice
		case SC_Example:
			val = kernel->machine->ReadRegister(4);
			cout << "Example Value:" << val << endl;
			return;
		case SC_Sleep:
			val = kernel->machine->ReadRegister(4);
			cout << "Sleep Value:" << val << endl;
			kernel->alarm->WaitUntil(val);
			return;
		default:
		    cerr << "Unexpected system call " << type << "\n";
 		    break;
	    }
	    break;
    // @shungfu: Edit ad Hw3
    case PageFaultException:    // Page Fault happens

        if (pageLock == NULL){  // make a semaphore lock if there is not
            pageLock = new Lock("PageLock");    // only one process can do page replacement at on time
        }
        pageLock->Acquire();

        kernel->stats->numPageFaults++; // Add statistic Number of Page Faults
        int Error_VAddr;
        unsigned int vpn;
        
        Error_VAddr = kernel->machine->ReadRegister(BadVAddrReg);   // the virtual addr makes page fault.
        vpn = (unsigned)Error_VAddr / PageSize;   // virtual page number
  
        cout << "\n-------------------- Page Fault ---------------------" << endl; 
        kernel->memManageUnit->pageFault(vpn);
        cout << "------------------------------------------------------\n" << endl;
        pageLock->Release();
        
        return;
	default:
        cerr << "Unexpected user mode exception" << which << "\n";
	    break;
    }
    ASSERTNOTREACHED();
}
