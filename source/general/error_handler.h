/*******************************************************************************
FILE : error_handler.h

LAST MODIFIED : 8 August 2002

DESCRIPTION :
Error handling routines.
???DB.   Not using POSIX because I can do everything I need with ANSI.
???DB.  Hard to abstract because of the special requirements for setjmp and
longjmp .
==============================================================================*/
/* to get access to signal codes */
#include <signal.h>
#include <setjmp.h>

/*
Global variables
----------------
*/
extern int signal_code;
extern jmp_buf jump_buffer;
#if !defined (WIN32_SYSTEM)
/*???DB.  POSIX */
/*extern sigjmp_buf jump_buffer;*/
/*???DB. SIGBUS is not POSIX */
extern void (*old_SIGBUS_handler)(int);
#endif /* !defined (WIN32_SYSTEM) */
extern void (*old_SIGFPE_handler)(int);
extern void (*old_SIGILL_handler)(int);
#if defined (UNIX)
extern void (*old_SIGPIPE_handler)(int);
#endif /* defined (UNIX) */
extern void (*old_SIGSEGV_handler)(int);

/*
Global functions
----------------
*/
void new_error_handler(int sig);

/*
Global macros
-------------
*/
#if !defined (WIN32_SYSTEM)
#define START_ERROR_HANDLING \
	/* determine old handlers */ \
	old_SIGBUS_handler=signal(SIGBUS,new_error_handler); \
	old_SIGFPE_handler=signal(SIGFPE,new_error_handler); \
	old_SIGILL_handler=signal(SIGILL,new_error_handler); \
	old_SIGPIPE_handler=signal(SIGPIPE,new_error_handler); \
	old_SIGSEGV_handler=signal(SIGSEGV,new_error_handler); \
	/* set the point where jumps should return to */ \
	if (0==setjmp(jump_buffer)) \
/*???DB.  POSIX */ \
/*  if (0==sigsetjmp(jump_buffer,1))*/ \
	{ \
		signal_code=0; \
	} \
	/* install new handler (new to make sure that its installed after every \
		signal*/ \
	signal(SIGBUS,new_error_handler); \
	signal(SIGFPE,new_error_handler); \
	signal(SIGILL,new_error_handler); \
	signal(SIGPIPE,new_error_handler); \
	signal(SIGSEGV,new_error_handler)
#else
#define START_ERROR_HANDLING \
	/* determine old handlers */ \
	old_SIGFPE_handler=signal(SIGFPE,new_error_handler); \
	old_SIGILL_handler=signal(SIGILL,new_error_handler); \
	old_SIGSEGV_handler=signal(SIGSEGV,new_error_handler); \
	/* set the point where jumps should return to */ \
	if (0==setjmp(jump_buffer)) \
/*???DB.  POSIX */ \
/*  if (0==sigsetjmp(jump_buffer,1))*/ \
	{ \
		signal_code=0; \
	} \
	/* install new handler (new to make sure that its installed after every \
		signal*/ \
	signal(SIGFPE,new_error_handler); \
	signal(SIGILL,new_error_handler); \
	signal(SIGSEGV,new_error_handler)
#endif /* !defined (WIN32_SYSTEM) */

#if !defined (WIN32_SYSTEM)
#define END_ERROR_HANDLING \
	/* restore old handlers */ \
	if (SIG_ERR!=old_SIGBUS_handler) \
	{ \
		signal(SIGBUS,old_SIGBUS_handler); \
	} \
	if (SIG_ERR!=old_SIGFPE_handler) \
	{ \
		signal(SIGFPE,old_SIGFPE_handler); \
	} \
	if (SIG_ERR!=old_SIGILL_handler) \
	{ \
		signal(SIGILL,old_SIGILL_handler); \
	} \
	if (SIG_ERR!=old_SIGPIPE_handler) \
	{ \
		signal(SIGPIPE,old_SIGPIPE_handler); \
	} \
	if (SIG_ERR!=old_SIGSEGV_handler) \
	{ \
		signal(SIGSEGV,old_SIGSEGV_handler); \
	}
#else
#define END_ERROR_HANDLING \
	/* restore old handlers */ \
	if (SIG_ERR!=old_SIGFPE_handler) \
	{ \
		signal(SIGFPE,old_SIGFPE_handler); \
	} \
	if (SIG_ERR!=old_SIGILL_handler) \
	{ \
		signal(SIGILL,old_SIGILL_handler); \
	} \
	if (SIG_ERR!=old_SIGPIPE_handler) \
	{ \
		signal(SIGPIPE,old_SIGPIPE_handler); \
	} \
	if (SIG_ERR!=old_SIGSEGV_handler) \
	{ \
		signal(SIGSEGV,old_SIGSEGV_handler); \
	}
#endif /* !defined (WIN32_SYSTEM) */
