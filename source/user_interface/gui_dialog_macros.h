/*******************************************************************************
FILE : gui_dialog_macros.h

LAST MODIFIED : 10 July 1998

DESCRIPTION :
Macros for automating some of the tasks involved in creating motif dialog
controls:
1. DIALOG_IDENTIFY macro responds to MrmNcreateCallback.
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
#if !defined (GUI_DIALOG_MACROS_H)
#define GUI_DIALOG_MACROS_H

#include <stdio.h>
#if defined (MOTIF)
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */

/*
Global variables
----------------
*/

/*
Global functions
----------------
*/
#if ! defined (SHORT_NAMES)
#define DIALOG_IDENTIFY_( dialog_name , this_widget ) \
	dialog_name ## _identify_ ## this_widget
#else
#define DIALOG_IDENTIFY_( dialog_name , this_widget ) \
	dialog_name ## _id_ ## this_widget
#endif
#define DIALOG_IDENTIFY( dialog_name , this_widget) \
	DIALOG_IDENTIFY_(dialog_name,this_widget)

#define DECLARE_DIALOG_IDENTIFY_FUNCTION( dialog_name , Dialog_structure_type, \
	this_widget ) \
static void DIALOG_IDENTIFY(dialog_name,this_widget)(Widget *widget_id, \
	XtPointer client_data,XtPointer call_data) \
/***************************************************************************** \
LAST MODIFIED : 25 July 1997 \
\
DESCRIPTION : \
Standard function for responding to MrmNcreateCallback declared as: \
	procedure identify_widget_procedure_name(dialog_structure) \
Gives the widget_id that sent the create callback to the <this_widget> member \
of the dialog_structure. \
============================================================================*/ \
{ \
	struct Dialog_structure_type *dialog_structure; \
\
	ENTER(DIALOG_IDENTIFY(dialog_name,this_widget)); \
	USE_PARAMETER(call_data); \
	if (dialog_structure=(struct Dialog_structure_type *)client_data) \
	{ \
		dialog_structure->this_widget= *widget_id; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"DIALOG_IDENTIFY(" #dialog_name "," #this_widget \
			").  client_data missing"); \
	} \
	LEAVE; \
} /* DIALOG_IDENTIFY(dialog_name,this_widget) */

#define DECLARE_DIAGNOSTIC_DIALOG_IDENTIFY_FUNCTION( dialog_name , \
	Dialog_structure_type,this_widget ) \
static void DIALOG_IDENTIFY(dialog_name,this_widget)(Widget *widget_id, \
	XtPointer client_data,XtPointer call_data) \
/***************************************************************************** \
LAST MODIFIED : 10 July 1998 \
\
DESCRIPTION : \
Standard function for responding to MrmNcreateCallback declared as: \
	procedure identify_widget_procedure_name(dialog_structure) \
Gives the widget_id that sent the create callback to the <this_widget> member \
of the dialog_structure. \
Diagnostic version of macro writes out when widget is identified. \
============================================================================*/ \
{ \
	struct Dialog_structure_type *dialog_structure; \
\
	ENTER(DIALOG_IDENTIFY(dialog_name,this_widget)); \
	if (dialog_structure=(struct Dialog_structure_type *)client_data) \
	{ \
		display_message(INFORMATION_MESSAGE, \
			"DIALOG_IDENTIFY(" #dialog_name "," #this_widget \
			") as value %p\n",*widget_id); \
		dialog_structure->this_widget= *widget_id; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"DIALOG_IDENTIFY(" #dialog_name "," #this_widget \
			").  client_data missing"); \
	} \
	LEAVE; \
} /* DIALOG_IDENTIFY(dialog_name,this_widget) */

#endif /* !defined (GUI_DIALOG_MACROS_H) */
