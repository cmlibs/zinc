/*******************************************************************************
FILE : gui_dialog_macros.h

LAST MODIFIED : 10 July 1998

DESCRIPTION :
Macros for automating some of the tasks involved in creating motif dialog
controls:
1. DIALOG_IDENTIFY macro responds to MrmNcreateCallback.
==============================================================================*/
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
#if defined (FULL_NAMES)
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
