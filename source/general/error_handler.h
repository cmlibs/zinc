/*******************************************************************************
FILE : error_handler.h

LAST MODIFIED : 8 August 2002

DESCRIPTION :
Error handling routines.
???DB.   Not using POSIX because I can do everything I need with ANSI.
???DB.  Hard to abstract because of the special requirements for setjmp and
longjmp .
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
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
