/*******************************************************************************
FILE : error_handler.c

LAST MODIFIED : 19 May 1998

DESCRIPTION :
Error handling routines.
???DB.  Not using POSIX because I can do everything I need with ANSI.
???DB.  Hard to abstract because of the special requirements for setjmp and
longjmp .
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "general/error_handler.h"

/*
Global variables
----------------
*/
int signal_code;
jmp_buf jump_buffer;
/*???DB.  POSIX */ \
/*sigjmp_buf jump_buffer;*/
/*???DB.  SIGBUS is not POSIX */ \
void (*old_SIGBUS_handler)(int);
void (*old_SIGFPE_handler)(int);
void (*old_SIGILL_handler)(int);
#if defined (UNIX)
void (*old_SIGPIPE_handler)(int);
#endif /* defined (UNIX) */
void (*old_SIGSEGV_handler)(int);

/*
Global functions
----------------
*/
void new_error_handler(int sig)
/*******************************************************************************
LAST MODIFIED : 18 May 1998

DESCRIPTION :
Sets <signal_code> to <sig> and longjmp's to the place specified by
START_ERROR_HANDLING .
==============================================================================*/
{
#if defined (UNIX)
	if (SIGPIPE==sig)
	{
		printf("Error: SIGPIPE\n");
	}
	else
	{
#endif /* defined (UNIX) */
		signal_code=sig;
		longjmp(jump_buffer,1);
/*???DB.  POSIX */ \
/*	  siglongjmp(jump_buffer,1);*/
#if defined (UNIX)
	}
#endif /* defined (UNIX) */
} /* new_error_handler */
