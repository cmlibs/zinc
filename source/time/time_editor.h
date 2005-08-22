/*******************************************************************************
FILE : time_editor.h

LAST MODIFIED : 17 May 2002

DESCRIPTION :
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
#if !defined (TIME_EDITOR_H)
#define TIME_EDITOR_H

#include "general/callback_motif.h"

/*
Global Types
------------
*/
struct Time_editor;

/*
Global Functions
----------------
*/
struct Time_editor *CREATE(Time_editor)(
	struct Time_editor **time_editor_address,
	Widget parent,struct Time_keeper *time_keeper,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 17 May 2002

DESCRIPTION :
Creates a <*time_editor_address>.
==============================================================================*/

int DESTROY(Time_editor)(struct Time_editor **time_editor_address);
/*******************************************************************************
LAST MODIFIED : 17 May 2002

DESCRIPTION :
Destroy the <*time_editor_address> and sets <*time_editor_address> to NULL.
==============================================================================*/

int time_editor_get_close_callback(struct Time_editor *time_editor,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 8 June 2004

DESCRIPTION :
Get the close <callback> information for the <time_editor>.
==============================================================================*/

int time_editor_set_close_callback(struct Time_editor *time_editor,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 8 June 2004

DESCRIPTION :
Set the close <callback> information for the <time_editor>.
==============================================================================*/

int time_editor_get_time_keeper(struct Time_editor *time_editor,
	struct Time_keeper **time_keeper);
/*******************************************************************************
LAST MODIFIED : 17 May 2002

DESCRIPTION :
Get the <*time_keeper_address> for the <time_editor>.
Do not modify or DEALLOCATE the returned time_keeper.
==============================================================================*/

int time_editor_set_time_keeper(struct Time_editor *time_editor,
	struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 17 May 2002

DESCRIPTION :
Set the <time_keeper> for the <time_editor>.
==============================================================================*/

int time_editor_get_step(struct Time_editor *time_editor,float *step);
/*******************************************************************************
LAST MODIFIED : 17 May 2002

DESCRIPTION :
Get the <*step> for the <time_editor>.
==============================================================================*/

int time_editor_set_step(struct Time_editor *time_editor,float step);
/*******************************************************************************
LAST MODIFIED : 17 May 2002

DESCRIPTION :
Set the <step> for the <time_editor>.
==============================================================================*/
#endif /* !defined (TIME_EDITOR_H) */
