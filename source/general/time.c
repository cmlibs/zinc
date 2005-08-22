/*******************************************************************************
FILE : time.c

LAST MODIFIED : 26 June 2002

DESCRIPTION :
Defines the gettimeofday and relevant structure for UNIX and WIN32_SYSTEM
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
#if defined (WIN32_SYSTEM)
#include <windows.h>
#include "general/time.h"
#include "general/debug.h"

int gettimeofday(struct timeval *time, void *timezone)
{
	USE_PARAMETER(timezone);

	unsigned int gettime;
  
	gettime = timeGetTime();
	time->tv_sec = gettime / 1000;
	time->tv_usec = (gettime - (time->tv_sec*1000)) * 1000;
	return 0;
}

clock_t times(struct tms *buffer)
{
	USE_PARAMETER(buffer);
	clock_t gettime;
  
	gettime = timeGetTime();
	return gettime;
}
#else /* defined (WIN32_SYSTEM) */
/* Declare something so that this file will compile */
int dummy;
#endif /* defined (WIN32_SYSTEM) */
