/*******************************************************************************
FILE : error_handler.c

LAST MODIFIED : 17 September 1999

DESCRIPTION :
Error handling routines.
???DB.  Not using POSIX because I can do everything I need with ANSI.
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
