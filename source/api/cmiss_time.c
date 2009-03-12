/*******************************************************************************
FILE : cmiss_time.c

DESCRIPTION :
The public interface to the Cmiss_time_object which supplies a concept of time 
to Cmgui.
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

#include "time/time.h"
#include "api/cmiss_time.h"
#include "general/debug.h"
#include "user_interface/message.h"

Cmiss_time_object_id Cmiss_time_object_create(const char *name)
{
	Cmiss_time_object_id time_object; 

	ENTER(miss_time_object_create);
	if (name)
	{
		time_object = ACCESS(Time_object)(CREATE(Time_object)(name));
	}
	LEAVE;

	return time_object;
}

int Cmiss_time_object_destroy(Cmiss_time_object_id *time_object_address)
{
	return(DEACCESS(Time_object)(time_object_address));
}
