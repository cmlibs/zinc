/*******************************************************************************
FILE : projection_dialog.c

LAST MODIFIED : 15 May 2000

DESCRIPTION :
???DB.  Started as map_dialog.c in emap
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xlib.h>
#include <X11/Composite.h>
#include <X11/Shell.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/LabelG.h>
#include <Xm/DialogS.h>
#include <Xm/DrawingA.h>
#include <Xm/RowColumn.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleBG.h>
#include <Mrm/MrmPublic.h>
#include <Mrm/MrmDecls.h>
#include "general/debug.h"
#include "graphics/spectrum.h"
#include "projection/projection.h"
#include "projection/projection_dialog.h"
#include "projection/projection_dialog.uidh"
#include "projection/projection_window.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int projection_dialog_hierarchy_open=0;
static MrmHierarchy projection_dialog_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
static void destroy_Projection_dialog(Widget widget,XtPointer projection_dialog,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Tidys up when the user destroys the projection dialog box.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(destroy_Projection_dialog);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		if (dialog->address)
		{
			*(dialog->address)=(struct Projection_dialog *)NULL;
		}
		DEALLOCATE(dialog);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_Projection_dialog.  Missing projection_dialog");
	}
	LEAVE;
} /* destroy_Projection_dialog */

static void identify_prj_dialog_range_type(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 10 May 1997

DESCRIPTION :
Finds the id of the range type option menu in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_range_type);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		(dialog->range).type_option_menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_range_type.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_range_type */

static void identify_prj_dialog_range_autom(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 10 May 1997

DESCRIPTION :
Saves the id of the automatic range option in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_range_autom);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		(dialog->range).type_option.automatic= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_range_autom.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_range_autom */

static void identify_prj_dialog_range_fixed(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 10 May 1997

DESCRIPTION :
Saves the id of the fixed range option in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_range_fixed);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		(dialog->range).type_option.fixed= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_range_fixed.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_range_fixed */

static void identify_prj_dialog_range_minim(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 10 May 1997

DESCRIPTION :
Saves the id of the minimum value text field in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_range_minim);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->range.minimum_value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_range_minim.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_range_minim */

static void identify_prj_dialog_range_maxim(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 10 May 1997

DESCRIPTION :
Saves the id of the maximum value text field in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_range_maxim);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->range.maximum_value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_range_maxim.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_range_maxim */

static void identify_prj_dialog_spectrum(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 10 May 1997

DESCRIPTION :
Finds the id of the spectrum type option menu in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_spectrum);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		(dialog->spectrum).type_option_menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_spectrum.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_spectrum */

static void identify_prj_dialog_spectrum_no(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 10 May 1997

DESCRIPTION :
Saves the id of the no spectrum option in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_spectrum_no);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		(dialog->spectrum).type_option.none= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_spectrum_no.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_spectrum_no */

static void identify_prj_dialog_blue_red(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 10 May 1997

DESCRIPTION :
Saves the id of the blue->red spectrum option in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_blue_red);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		(dialog->spectrum).type_option.blue_red= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_blue_red.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_blue_red */

static void identify_prj_dialog_bl_wh_rd(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
Saves the id of the blue_white_red spectrum option in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_bl_wh_rd);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		(dialog->spectrum).type_option.blue_white_red= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_bl_wh_rd.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_bl_wh_rd */

static void identify_prj_dialog_red_blue(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 10 May 1997

DESCRIPTION :
Saves the id of the red->blue spectrum option in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_red_blue);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		(dialog->spectrum).type_option.red_blue= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_red_blue.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_red_blue */

static void identify_prj_dialog_log_blue_re(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 10 May 1997

DESCRIPTION :
Saves the id of the log(blue->red) spectrum option in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_log_blue_re);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		(dialog->spectrum).type_option.log_blue_red= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_log_blue_re.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_log_blue_re */

static void identify_prj_dialog_log_red_blu(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 10 May 1997

DESCRIPTION :
Saves the id of the log(red->blue) spectrum option in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_log_red_blu);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		(dialog->spectrum).type_option.log_red_blue= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_log_red_blu.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_log_red_blu */

static void identify_prj_dialog_contours(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 10 May 1997

DESCRIPTION :
Finds the id of the contours type option menu in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_contours);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		(dialog->contours).type_option_menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_contours_bu.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_contours_bu */

static void identify_prj_dialog_contours_no(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 10 May 1997

DESCRIPTION :
Saves the id of the no contours option in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_contours_no);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		(dialog->contours).type_option.none= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_contours_no.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_contours_no */

static void identify_prj_dialog_contours_co(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 10 May 1997

DESCRIPTION :
Saves the id of the constant thickness contours option in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_contours_co);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		(dialog->contours).type_option.constant_thickness= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_contours_co.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_contours_co */

static void identify_prj_dialog_contours_va(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 10 May 1997

DESCRIPTION :
Saves the id of the variable thickness contours option in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_contours_va);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		(dialog->contours).type_option.variable_thickness= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_contours_va.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_contours_va */

static void identify_prj_dialog_contours_do(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Finds the id of the increment number of contours arrow in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_contours_do);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->contours.down_arrow= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_contours_do.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_contours_do */

static void decrement_number_of_contours(Widget widget,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Decrement the number of contours.
==============================================================================*/
{
	struct Projection_dialog *dialog;
	char number_string[3];

	ENTER(decrement_number_of_contours);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		if (99==dialog->number_of_contours)
		{
			XtManageChild(dialog->contours.up_arrow);
		}
		(dialog->number_of_contours)--;
		sprintf(number_string,"%2d",dialog->number_of_contours);
		XtVaSetValues(dialog->contours.number,
			XmNlabelString,XmStringCreate(number_string,XmSTRING_DEFAULT_CHARSET),
			NULL);
		if (2==dialog->number_of_contours)
		{
			XtUnmanageChild(dialog->contours.down_arrow);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"decrement_number_of_contours.  Missing projection_dialog");
	}
	LEAVE;
} /* decrement_number_of_contours */

static void identify_prj_dialog_contours_nu(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Finds the id of the label showing the number of contours in the projection
dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_contours_nu);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->contours.number= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_contours_nu.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_contours_nu */

static void identify_prj_dialog_contours_up(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Finds the id of the decrement number of contours arrow in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_contours_up);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->contours.up_arrow= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_contours_up.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_contours_up */

static void increment_number_of_contours(Widget widget,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Increment the number of contours.
==============================================================================*/
{
	struct Projection_dialog *dialog;
	char number_string[3];

	ENTER(increment_number_of_contours);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		if (2==dialog->number_of_contours)
		{
			XtManageChild(dialog->contours.down_arrow);
		}
		(dialog->number_of_contours)++;
		sprintf(number_string,"%2d",dialog->number_of_contours);
		XtVaSetValues(dialog->contours.number,
			XmNlabelString,XmStringCreate(number_string,XmSTRING_DEFAULT_CHARSET),
			NULL);
		if (99==dialog->number_of_contours)
		{
			XtUnmanageChild(dialog->contours.up_arrow);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"increment_number_of_contours.  Missing projection_dialog");
	}
	LEAVE;
} /* increment_number_of_contours */

static void identify_prj_dialog_xi_3_value(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Saves the id of the xi_3 value text field in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_xi_3_value);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->xi_3_value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_xi_3_value.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_xi_3_value */

static void identify_prj_dialog_type_menu(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Finds the id of the type option menu in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_type_menu);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->type_option_menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_type_menu.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_type_menu */

static void identify_prj_dialog_type_hammer(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Finds the id of the hammer button in type option menu in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_type_hammer);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->type_option.hammer= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_type_hammer.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_type_hammer */

static void identify_prj_dialog_type_polar(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Finds the id of the polar button in type option menu in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_type_polar);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->type_option.polar= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_type_polar.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_type_polar */

static void identify_prj_dialog_type_cylind(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Finds the id of the cylindrical button in type option menu in the projection
dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_type_cylind);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->type_option.cylindrical= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_type_cylind.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_type_cylind */

static void id_prj_dialog_elements_menu(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 11 October 1996

DESCRIPTION :
Finds the id of the elements option menu in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(id_prj_dialog_elements_menu);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->elements_option_menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_prj_dialog_elements_menu.  Missing projection_dialog");
	}
	LEAVE;
} /* id_prj_dialog_elements_menu */

static void id_prj_dialog_elements_name(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 11 October 1996

DESCRIPTION :
Finds the id of the name_and_boundary button in elements option menu in the
projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(id_prj_dialog_elements_name);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->elements_option.name_and_boundary= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_prj_dialog_elements_name.  Missing projection_dialog");
	}
	LEAVE;
} /* id_prj_dialog_elements_name */

static void id_prj_dialog_elements_bound(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 11 October 1996

DESCRIPTION :
Finds the id of the boundary_only button in elements option menu in the
projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(id_prj_dialog_elements_bound);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->elements_option.boundary_only= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_prj_dialog_elements_bound.  Missing projection_dialog");
	}
	LEAVE;
} /* id_prj_dialog_elements_bound */

static void id_prj_dialog_elements_hide(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 11 October 1996

DESCRIPTION :
Finds the id of the hide button in elements option menu in the projection
dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(id_prj_dialog_elements_hide);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->elements_option.hide= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_prj_dialog_elements_hide.  Missing projection_dialog");
	}
	LEAVE;
} /* id_prj_dialog_elements_hide */

static void identify_prj_dialog_nodes_menu(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Finds the id of the nodes option menu in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_nodes_menu);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->nodes_option_menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_nodes_menu.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_nodes_menu */

static void identify_prj_dialog_nodes_name(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Finds the id of the name button in nodes option menu in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_nodes_name);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->nodes_option.name= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_nodes_name.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_nodes_name */

static void identify_prj_dialog_nodes_value(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Finds the id of the value button in nodes option menu in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_nodes_value);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->nodes_option.value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_nodes_value.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_nodes_value */

static void identify_prj_dialog_nodes_hide(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Finds the id of the hide button in nodes option menu in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_nodes_hide);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->nodes_option.hide= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_nodes_hide.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_nodes_hide */

static void identify_prj_dialog_fibre_menu(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Finds the id of the fibres option menu in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_fibre_menu);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->fibres_option_menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_fibre_menu.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_fibre_menu */

static void identify_prj_dialog_fibre_hide(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Finds the id of the fibre button in the fibres option menu in the projection
dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_fibre_hide);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->fibres_option.hide= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_fibre_hide.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_fibre_hide */

static void identify_prj_dialog_fibre_fine(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Finds the id of the fine button in the fibres option menu in the projection
dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_fibre_fine);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->fibres_option.fine= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_fibre_fine.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_fibre_fine */

static void identify_prj_dialog_fibre_mediu(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Finds the id of the medium button in the fibres option menu in the projection
dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_fibre_mediu);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->fibres_option.medium= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_fibre_mediu.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_fibre_mediu */

static void identify_prj_dialog_fibre_coars(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Finds the id of the coarse button in the fibres option menu in the projection
dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_fibre_coars);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->fibres_option.coarse= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_fibre_coars.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_fibre_coars */

static void identify_prj_dialog_landmarks_b(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Finds the id of the landmarks button in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_landmarks_b);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->show_landmarks_toggle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_landmarks_b.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_landmarks_b */

static void identify_prj_dialog_extrema_but(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 May 1997

DESCRIPTION :
Finds the id of the extrema button in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_extrema_but);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->show_extrema_toggle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_extrema_but.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_extrema_but */

static void identify_prj_dialog_aspect_rati(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 May 1997

DESCRIPTION :
Finds the id of the maintain aspect ratio button in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_aspect_rati);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->maintain_aspect_ratio_toggle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_aspect_rati.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_aspect_rati */

static void identify_prj_dialog_print_spect(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 May 1997

DESCRIPTION :
Finds the id of the print spectrum button in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_print_spect);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->print_spectrum_toggle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_print_spect.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_print_spect */

static void identify_prj_dialog_cancel_butt(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Finds the id of the cancel button in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_cancel_butt);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->cancel_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_cancel_butt.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_cancel_butt */

static void identify_prj_dialog_apply_butto(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 December 1995

DESCRIPTION :
Finds the id of the apply button in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_apply_butto);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->apply_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_apply_butto.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_apply_butto */

static void identify_prj_dialog_ok_button(Widget *widget_id,
	XtPointer projection_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Finds the id of the ok button in the projection dialog.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(identify_prj_dialog_ok_button);
	USE_PARAMETER(call_data);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		dialog->ok_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_prj_dialog_ok_button.  Missing projection_dialog");
	}
	LEAVE;
} /* identify_prj_dialog_ok_button */

static void update_projection_from_dialog(Widget widget,
	XtPointer projection_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
Updates the projection settings based on the configure dialog and redraws the
projection if necessary.
==============================================================================*/
{
	char projection_settings_changed,temp_string[20],*value_string;
	enum Contour_thickness contour_thickness;
	enum Elements_option elements_option;
	enum Fibres_option fibres_option;
	enum Nodes_option nodes_option;
	enum Projection_type projection_type;
	float maximum_range,minimum_range,value,xi_3;
	int recalculate;
	struct Projection *projection;
	struct Projection_dialog *configure_dialog;
	struct Projection_window *window;
	Widget option_widget;

	ENTER(update_projection_from_dialog);
	USE_PARAMETER(call_data);
	if ((window=(struct Projection_window *)projection_window)&&
		(projection=window->projection)&&
		(configure_dialog=window->configure_dialog))
	{
		projection_settings_changed=0;
		recalculate=0;
		XtVaGetValues(configure_dialog->type_option_menu,
			XmNmenuHistory,&option_widget,
			NULL);
		if (option_widget==configure_dialog->type_option.hammer)
		{
			projection_type=HAMMER_PROJECTION;
		}
		else
		{
			if (option_widget==configure_dialog->type_option.polar)
			{
				projection_type=POLAR_PROJECTION;
			}
			else
			{
				projection_type=CYLINDRICAL_PROJECTION;
			}
		}
		if (projection->type!=projection_type)
		{
			projection->type=projection_type;
			projection_settings_changed=1;
			recalculate=2;
		}
		XtVaGetValues(configure_dialog->spectrum.type_option_menu,
			XmNmenuHistory,&option_widget,
			NULL);
		if (option_widget==configure_dialog->spectrum.type_option.none)
		{
			if (projection->colour_option!=HIDE_COLOUR)
			{
				projection_settings_changed=1;
				projection->colour_option=HIDE_COLOUR;
			}
		}
		else
		{
			if (projection->colour_option!=SHOW_COLOUR)
			{
				projection_settings_changed=1;
				projection->colour_option=SHOW_COLOUR;
			}
			/*???DB.  Need a spectrum widget */
			{
				if (option_widget==configure_dialog->spectrum.type_option.blue_red)
				{
					if (BLUE_TO_RED_SPECTRUM != Spectrum_get_simple_type(projection->spectrum))
					{
						Spectrum_set_simple_type(projection->spectrum,
							BLUE_TO_RED_SPECTRUM);
						projection_settings_changed = 1;
					}
				}
				else if (option_widget==configure_dialog->spectrum.type_option.blue_white_red)
				{
					if (BLUE_WHITE_RED_SPECTRUM != Spectrum_get_simple_type(projection->spectrum))
					{
						Spectrum_set_simple_type(projection->spectrum,
							BLUE_WHITE_RED_SPECTRUM);
						projection_settings_changed = 1;
					}
				}
				else if (option_widget==configure_dialog->spectrum.type_option.
						log_blue_red)
				{
					if (LOG_BLUE_TO_RED_SPECTRUM != Spectrum_get_simple_type(projection->spectrum))
					{
						Spectrum_set_simple_type(projection->spectrum,
							LOG_BLUE_TO_RED_SPECTRUM);
						projection_settings_changed = 1;
					}
				}
				else if (option_widget==
							configure_dialog->spectrum.type_option.log_red_blue)
				{
					if (BLUE_TO_RED_SPECTRUM != Spectrum_get_simple_type(projection->spectrum))
					{
						Spectrum_set_simple_type(projection->spectrum,
							BLUE_TO_RED_SPECTRUM);
						projection_settings_changed = 1;
					}
				}
				else
				{
					if (BLUE_TO_RED_SPECTRUM != Spectrum_get_simple_type(projection->spectrum))
					{
						Spectrum_set_simple_type(projection->spectrum,
							BLUE_TO_RED_SPECTRUM);
						projection_settings_changed = 1;
					}
				}
			}
		}
#if defined (OLD_CODE)
		if (XmToggleButtonGadgetGetState(configure_dialog->show_colour_toggle))
		{
			if (projection->colour_option!=SHOW_COLOUR)
			{
				projection_settings_changed=1;
				projection->colour_option=SHOW_COLOUR;
			}
		}
		else
		{
			if (projection->colour_option!=HIDE_COLOUR)
			{
				projection_settings_changed=1;
				projection->colour_option=HIDE_COLOUR;
			}
		}
		if (XmToggleButtonGadgetGetState(configure_dialog->contours.show_toggle))
		{
			if (projection->contours_option!=SHOW_CONTOURS)
			{
				projection_settings_changed=1;
				projection->contours_option=SHOW_CONTOURS;
			}
		}
		else
		{
			if (projection->contours_option!=HIDE_CONTOURS)
			{
				projection_settings_changed=1;
				projection->contours_option=HIDE_CONTOURS;
			}
		}
		XtVaGetValues(configure_dialog->contours.thickness_option_menu,
			XmNmenuHistory,&option_widget,
			NULL);
		if (option_widget==configure_dialog->contours.thickness_option.constant)
		{
			contour_thickness=CONSTANT_THICKNESS;
		}
		else
		{
			if (option_widget==configure_dialog->contours.thickness_option.variable)
			{
				contour_thickness=VARIABLE_THICKNESS;
			}
		}
		if (projection->contour_thickness!=contour_thickness)
		{
			projection->contour_thickness=contour_thickness;
			projection_settings_changed=1;
		}
#endif /* defined (OLD_CODE) */
		XtVaGetValues(configure_dialog->contours.type_option_menu,
			XmNmenuHistory,&option_widget,
			NULL);
		if (option_widget==configure_dialog->contours.type_option.none)
		{
			if (projection->contours_option!=HIDE_CONTOURS)
			{
				projection_settings_changed=1;
				projection->contours_option=HIDE_CONTOURS;
			}
		}
		else
		{
			if (projection->contours_option!=SHOW_CONTOURS)
			{
				projection_settings_changed=1;
				projection->contours_option=SHOW_CONTOURS;
			}
			if (option_widget==
				configure_dialog->contours.type_option.constant_thickness)
			{
				contour_thickness=CONSTANT_THICKNESS;
			}
			else
			{
				if (option_widget==
					configure_dialog->contours.type_option.variable_thickness)
				{
					contour_thickness=VARIABLE_THICKNESS;
				}
			}
			if (projection->contour_thickness!=contour_thickness)
			{
				projection->contour_thickness=contour_thickness;
				projection_settings_changed=1;
			}
		}
		value_string=(char *)NULL;
		XtVaGetValues((configure_dialog->xi_3_value),
			XmNvalue,&value_string,
			NULL);
		if (1==sscanf(value_string,"%f",&xi_3))
		{
			if (xi_3<0)
			{
				xi_3=0;
			}
			else
			{
				if (xi_3>1)
				{
					xi_3=1;
				}
			}
		}
		else
		{
			xi_3=configure_dialog->xi_3;
		}
		XtFree(value_string);
		if ((FE_value)xi_3!=projection->xi_3)
		{
			projection->xi_3=(FE_value)xi_3;
			projection_settings_changed=1;
			recalculate=2;
			XtVaGetValues(configure_dialog->range.type_option_menu,
				XmNmenuHistory,&option_widget,
				NULL);
			if (option_widget==configure_dialog->range.type_option.fixed)
			{
				projection->expand_spectrum=0;
			}
			else
			{
				projection->expand_spectrum=1;
			}
		}
		value_string=(char *)NULL;
		XtVaGetValues((configure_dialog->range.maximum_value),
			XmNvalue,&value_string,
			NULL);
		if (1!=sscanf(value_string,"%f",&maximum_range))
		{
			maximum_range=configure_dialog->range_maximum;
		}
		XtFree(value_string);
		value_string=(char *)NULL;
		XtVaGetValues((configure_dialog->range.minimum_value),
			XmNvalue,&value_string,
			NULL);
		if (1!=sscanf(value_string,"%f",&minimum_range))
		{
			minimum_range=configure_dialog->range_minimum;
		}
		XtFree(value_string);
		if (minimum_range>maximum_range)
		{
			value=minimum_range;
			minimum_range=maximum_range;
			maximum_range=value;
		}
		if (minimum_range!=configure_dialog->range_minimum)
		{
			configure_dialog->range_minimum=minimum_range;
			set_Spectrum_minimum(projection->spectrum,minimum_range);
#if defined (DRAG_CONTOUR_RANGE)
			if (minimum_range>projection->contour_minimum)
			{
				projection->contour_minimum=minimum_range;
				if (minimum_range>projection->contour_maximum)
				{
					projection->contour_maximum=maximum_range;
				}
			}
#else
			projection->contour_minimum=minimum_range;
#endif /* defined (DRAG_CONTOUR_RANGE) */
			projection_settings_changed=1;
			recalculate=1;
		}
		if (maximum_range!=configure_dialog->range_maximum)
		{
			configure_dialog->range_maximum=maximum_range;
			set_Spectrum_maximum(projection->spectrum,maximum_range);
#if defined (DRAG_CONTOUR_RANGE)
			if (maximum_range<projection->contour_maximum)
			{
				projection->contour_maximum=maximum_range;
				if (maximum_range<projection->contour_minimum)
				{
					projection->contour_minimum=minimum_range;
				}
			}
#else
			projection->contour_maximum=maximum_range;
#endif /* defined (DRAG_CONTOUR_RANGE) */
			projection_settings_changed=1;
			recalculate=1;
		}
		if (projection->number_of_contours!=configure_dialog->number_of_contours)
		{
			projection->number_of_contours=configure_dialog->number_of_contours;
			if (SHOW_CONTOURS==projection->contours_option)
			{
				projection_settings_changed=1;
			}
		}
		XtVaGetValues(configure_dialog->elements_option_menu,
			XmNmenuHistory,&option_widget,
			NULL);
		if (option_widget==configure_dialog->elements_option.name_and_boundary)
		{
			elements_option=SHOW_ELEMENT_NAMES_AND_BOUNDARIES;
		}
		else
		{
			if (option_widget==configure_dialog->elements_option.boundary_only)
			{
				elements_option=SHOW_ELEMENT_BOUNDARIES_ONLY;
			}
			else
			{
				if (option_widget==configure_dialog->elements_option.hide)
				{
					elements_option=HIDE_ELEMENTS;
				}
			}
		}
		if (projection->elements_option!=elements_option)
		{
			projection->elements_option=elements_option;
			projection_settings_changed=1;
		}
		XtVaGetValues(configure_dialog->nodes_option_menu,
			XmNmenuHistory,&option_widget,
			NULL);
		if (option_widget==configure_dialog->nodes_option.name)
		{
			nodes_option=SHOW_NODE_NAMES;
		}
		else
		{
			if (option_widget==configure_dialog->nodes_option.value)
			{
				nodes_option=SHOW_NODE_VALUES;
			}
			else
			{
				if (option_widget==configure_dialog->nodes_option.hide)
				{
					nodes_option=HIDE_NODES;
				}
			}
		}
		if (projection->nodes_option!=nodes_option)
		{
			projection->nodes_option=nodes_option;
			projection_settings_changed=1;
		}
		XtVaGetValues(configure_dialog->fibres_option_menu,
			XmNmenuHistory,&option_widget,
			NULL);
		if (option_widget==configure_dialog->fibres_option.hide)
		{
			fibres_option=HIDE_FIBRES;
		}
		else
		{
			if (option_widget==configure_dialog->fibres_option.fine)
			{
				fibres_option=SHOW_FIBRES_FINE;
			}
			else
			{
				if (option_widget==configure_dialog->fibres_option.medium)
				{
					fibres_option=SHOW_FIBRES_MEDIUM;
				}
				else
				{
					if (option_widget==configure_dialog->fibres_option.coarse)
					{
						fibres_option=SHOW_FIBRES_COARSE;
					}
				}
			}
		}
		if (projection->fibres_option!=fibres_option)
		{
			projection->fibres_option=fibres_option;
			projection_settings_changed=1;
		}
#if defined (OLD_CODE)
/*???DB.  Landmarks need updating */
		if (XmToggleButtonGadgetGetState(configure_dialog->show_landmarks_toggle))
		{
			if (SHOW_LANDMARKS!=projection->landmarks_option)
			{
				projection_settings_changed=1;
				projection->landmarks_option=SHOW_LANDMARKS;
			}
		}
		else
		{
			if (HIDE_LANDMARKS!=projection->landmarks_option)
			{
				projection_settings_changed=1;
				projection->landmarks_option=HIDE_LANDMARKS;
			}
		}
#endif
		if (XmToggleButtonGadgetGetState(configure_dialog->show_extrema_toggle))
		{
			if (SHOW_EXTREMA!=projection->extrema_option)
			{
				projection_settings_changed=1;
				projection->extrema_option=SHOW_EXTREMA;
			}
		}
		else
		{
			if (HIDE_EXTREMA!=projection->extrema_option)
			{
				projection_settings_changed=1;
				projection->extrema_option=HIDE_EXTREMA;
			}
		}
		if (XmToggleButtonGadgetGetState(configure_dialog->
			maintain_aspect_ratio_toggle))
		{
			if (!(projection->maintain_aspect_ratio))
			{
				projection_settings_changed=1;
				recalculate=2;
				projection->maintain_aspect_ratio=1;
			}
		}
		else
		{
			if (projection->maintain_aspect_ratio)
			{
				projection_settings_changed=1;
				recalculate=2;
				projection->maintain_aspect_ratio=0;
			}
		}
		if (XmToggleButtonGadgetGetState(configure_dialog->print_spectrum_toggle))
		{
			if (!(projection->print_spectrum))
			{
				projection->print_spectrum=1;
			}
		}
		else
		{
			if (projection->print_spectrum)
			{
				projection->print_spectrum=0;
			}
		}
		if (projection_settings_changed)
		{
			update_projection_drawing_area(window,recalculate);
			update_projection_spectrum(window);
		}
		if (widget==configure_dialog->ok_button)
		{
			/* close the projection dialog */
			close_projection_dialog((Widget)NULL,(XtPointer)configure_dialog,
				(XtPointer)NULL);
		}
		else
		{
			if (get_Spectrum_maximum(projection->spectrum)!=
				configure_dialog->range_maximum)
			{
				sprintf(temp_string,"%g",get_Spectrum_maximum(projection->spectrum));
				sscanf(temp_string,"%f",&(configure_dialog->range_maximum));
				XtVaSetValues(configure_dialog->range.maximum_value,
					XmNvalue,temp_string,
					NULL);
			}
			if (get_Spectrum_minimum(projection->spectrum)!=
				configure_dialog->range_minimum)
			{
				sprintf(temp_string,"%g",get_Spectrum_minimum(projection->spectrum));
				sscanf(temp_string,"%f",&(configure_dialog->range_minimum));
				XtVaSetValues(configure_dialog->range.minimum_value,
					XmNvalue,temp_string,
					NULL);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"update_projection_from_dialog.  Invalid or missing projection_window");
	}
	LEAVE;
} /* update_projection_from_dialog */

/*
Global functions
----------------
*/
struct Projection_dialog *create_Projection_dialog(
	struct Projection_dialog **projection_dialog_address,
	struct Projection_window *projection_window,Widget creator,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
Allocates the memory for a projection dialog.  Retrieves the necessary widgets
and initializes the appropriate fields.
==============================================================================*/
{
	MrmType projection_dialog_class;
	static MrmRegisterArg callback_list[]={
		{"identify_prj_dialog_type_hammer",
			(XtPointer)identify_prj_dialog_type_hammer},
		{"identify_prj_dialog_type_polar",
			(XtPointer)identify_prj_dialog_type_polar},
		{"identify_prj_dialog_type_cylind",
			(XtPointer)identify_prj_dialog_type_cylind},
		{"identify_prj_dialog_type_menu",
			(XtPointer)identify_prj_dialog_type_menu},
		{"identify_prj_dialog_range_type",
			(XtPointer)identify_prj_dialog_range_type},
		{"identify_prj_dialog_range_autom",
			(XtPointer)identify_prj_dialog_range_autom},
		{"identify_prj_dialog_range_fixed",
			(XtPointer)identify_prj_dialog_range_fixed},
		{"identify_prj_dialog_range_minim",
			(XtPointer)identify_prj_dialog_range_minim},
		{"identify_prj_dialog_range_maxim",
			(XtPointer)identify_prj_dialog_range_maxim},
		{"identify_prj_dialog_spectrum",
			(XtPointer)identify_prj_dialog_spectrum},
		{"identify_prj_dialog_spectrum_no",
			(XtPointer)identify_prj_dialog_spectrum_no},
		{"identify_prj_dialog_blue_red",
			(XtPointer)identify_prj_dialog_blue_red},
		{"identify_prj_dialog_bl_wh_rd",
			(XtPointer)identify_prj_dialog_bl_wh_rd},
		{"identify_prj_dialog_red_blue",
			(XtPointer)identify_prj_dialog_red_blue},
		{"identify_prj_dialog_log_blue_re",
			(XtPointer)identify_prj_dialog_log_blue_re},
		{"identify_prj_dialog_log_red_blu",
			(XtPointer)identify_prj_dialog_log_red_blu},
		{"identify_prj_dialog_contours",
			(XtPointer)identify_prj_dialog_contours},
		{"identify_prj_dialog_contours_no",
			(XtPointer)identify_prj_dialog_contours_no},
		{"identify_prj_dialog_contours_co",
			(XtPointer)identify_prj_dialog_contours_co},
		{"identify_prj_dialog_contours_va",
			(XtPointer)identify_prj_dialog_contours_va},
		{"identify_prj_dialog_contours_do",
			(XtPointer)identify_prj_dialog_contours_do},
		{"decrement_number_of_contours",(XtPointer)decrement_number_of_contours},
		{"identify_prj_dialog_contours_nu",
			(XtPointer)identify_prj_dialog_contours_nu},
		{"identify_prj_dialog_contours_up",
			(XtPointer)identify_prj_dialog_contours_up},
		{"increment_number_of_contours",(XtPointer)increment_number_of_contours},
		{"identify_prj_dialog_xi_3_value",
			(XtPointer)identify_prj_dialog_xi_3_value},
		{"id_prj_dialog_elements_name",(XtPointer)id_prj_dialog_elements_name},
		{"id_prj_dialog_elements_bound",(XtPointer)id_prj_dialog_elements_bound},
		{"id_prj_dialog_elements_hide",(XtPointer)id_prj_dialog_elements_hide},
		{"id_prj_dialog_elements_menu",(XtPointer)id_prj_dialog_elements_menu},
		{"identify_prj_dialog_nodes_name",
			(XtPointer)identify_prj_dialog_nodes_name},
		{"identify_prj_dialog_nodes_value",
			(XtPointer)identify_prj_dialog_nodes_value},
		{"identify_prj_dialog_nodes_hide",
			(XtPointer)identify_prj_dialog_nodes_hide},
		{"identify_prj_dialog_nodes_menu",
			(XtPointer)identify_prj_dialog_nodes_menu},
		{"identify_prj_dialog_fibre_hide",
			(XtPointer)identify_prj_dialog_fibre_hide},
		{"identify_prj_dialog_fibre_fine",
			(XtPointer)identify_prj_dialog_fibre_fine},
		{"identify_prj_dialog_fibre_mediu",
			(XtPointer)identify_prj_dialog_fibre_mediu},
		{"identify_prj_dialog_fibre_coars",
			(XtPointer)identify_prj_dialog_fibre_coars},
		{"identify_prj_dialog_fibre_menu",
			(XtPointer)identify_prj_dialog_fibre_menu},
		{"identify_prj_dialog_landmarks_b",
			(XtPointer)identify_prj_dialog_landmarks_b},
		{"identify_prj_dialog_extrema_but",
			(XtPointer)identify_prj_dialog_extrema_but},
		{"identify_prj_dialog_aspect_rati",
			(XtPointer)identify_prj_dialog_aspect_rati},
		{"identify_prj_dialog_print_spect",
			(XtPointer)identify_prj_dialog_print_spect},
		{"identify_prj_dialog_ok_button",(XtPointer)identify_prj_dialog_ok_button},
		{"update_projection_from_dialog",(XtPointer)update_projection_from_dialog},
		{"identify_prj_dialog_apply_butto",
			(XtPointer)identify_prj_dialog_apply_butto},
		{"identify_prj_dialog_cancel_butt",
			(XtPointer)identify_prj_dialog_cancel_butt},
		{"close_projection_dialog",(XtPointer)close_projection_dialog}};
	static MrmRegisterArg identifier_list[]={
		{"projection_dialog_structure",NULL},
		{"projection_window_structure",NULL},
		{"widget_spacing",(XtPointer)5}};
	struct Projection_dialog *projection_dialog;
	Widget parent;

	ENTER(create_Projection_dialog);
	/* check arguments */
	if (user_interface)
	{
		if (MrmOpenHierarchy_base64_string(projection_dialog_uidh,
			&projection_dialog_hierarchy,&projection_dialog_hierarchy_open))
		{
			if (ALLOCATE(projection_dialog,struct Projection_dialog,1))
			{
				projection_dialog->creator=creator;
				projection_dialog->dialog=(Widget)NULL;
				projection_dialog->shell=(Widget)NULL;
				projection_dialog->type_option_menu=(Widget)NULL;
				projection_dialog->type_option.hammer=(Widget)NULL;
				projection_dialog->type_option.polar=(Widget)NULL;
				projection_dialog->type_option.cylindrical=(Widget)NULL;
				projection_dialog->range.type_option_menu=(Widget)NULL;
				projection_dialog->range.type_option.automatic=(Widget)NULL;
				projection_dialog->range.type_option.fixed=(Widget)NULL;
				projection_dialog->range.minimum_value=(Widget)NULL;
				projection_dialog->range.maximum_value=(Widget)NULL;
				projection_dialog->spectrum.type_option_menu=(Widget)NULL;
				projection_dialog->spectrum.type_option.none=(Widget)NULL;
				projection_dialog->spectrum.type_option.blue_red=(Widget)NULL;	
				projection_dialog->spectrum.type_option.blue_white_red=(Widget)NULL;
				projection_dialog->spectrum.type_option.red_blue=(Widget)NULL;
				projection_dialog->spectrum.type_option.log_blue_red=(Widget)NULL;
				projection_dialog->spectrum.type_option.log_red_blue=(Widget)NULL;
				projection_dialog->contours.type_option_menu=(Widget)NULL;
				projection_dialog->contours.type_option.none=(Widget)NULL;
				projection_dialog->contours.type_option.constant_thickness=(Widget)NULL;
				projection_dialog->contours.type_option.variable_thickness=(Widget)NULL;
				projection_dialog->contours.down_arrow=(Widget)NULL;
				projection_dialog->contours.number=(Widget)NULL;
				projection_dialog->contours.up_arrow=(Widget)NULL;
				projection_dialog->xi_3_value=(Widget)NULL;
				projection_dialog->elements_option_menu=(Widget)NULL;
				projection_dialog->elements_option.boundary_only=(Widget)NULL;
				projection_dialog->elements_option.name_and_boundary=(Widget)NULL;
				projection_dialog->elements_option.hide=(Widget)NULL;
				projection_dialog->nodes_option_menu=(Widget)NULL;
				projection_dialog->nodes_option.name=(Widget)NULL;
				projection_dialog->nodes_option.value=(Widget)NULL;
				projection_dialog->nodes_option.hide=(Widget)NULL;
				projection_dialog->fibres_option_menu=(Widget)NULL;
				projection_dialog->fibres_option.hide=(Widget)NULL;
				projection_dialog->fibres_option.fine=(Widget)NULL;
				projection_dialog->fibres_option.medium=(Widget)NULL;
				projection_dialog->fibres_option.coarse=(Widget)NULL;
				projection_dialog->show_landmarks_toggle=(Widget)NULL;
				projection_dialog->show_extrema_toggle=(Widget)NULL;
				projection_dialog->maintain_aspect_ratio_toggle=(Widget)NULL;
				projection_dialog->print_spectrum_toggle=(Widget)NULL;
				projection_dialog->ok_button=(Widget)NULL;
				projection_dialog->apply_button=(Widget)NULL;
				projection_dialog->cancel_button=(Widget)NULL;
				projection_dialog->address=projection_dialog_address;
				projection_dialog->projection= &(projection_window->projection);
				/* create the dialog shell */
/*        if (!(parent=creator)||(True!=XtIsWidget(parent)))
				{*/
					parent=User_interface_get_application_shell(user_interface);
/*        }*/
				if (projection_dialog->shell=XtVaCreatePopupShell(
					"projection_dialog_shell",
					xmDialogShellWidgetClass,parent,
/*          XmNmwmDecorations,MWM_DECOR_ALL&(~MWM_DECOR_RESIZEH),
					XmNmwmFunctions,MWM_FUNC_MOVE&MWM_FUNC_CLOSE,*/
					XmNmwmDecorations,MWM_DECOR_ALL|MWM_DECOR_RESIZEH,
					XmNmwmFunctions,MWM_FUNC_MOVE|MWM_FUNC_CLOSE,
					XmNtitle,"Projection settings",
					NULL))
				{
					/* add the destroy callback */
					XtAddCallback(projection_dialog->shell,XmNdestroyCallback,
						destroy_Projection_dialog,(XtPointer)projection_dialog);
					/* register the other callbacks */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						projection_dialog_hierarchy,callback_list,XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value=(XtPointer)projection_dialog;
						identifier_list[1].value=(XtPointer)projection_window;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
							projection_dialog_hierarchy,identifier_list,
							XtNumber(identifier_list)))
						{
							/* fetch the dialog widget */
							if (MrmSUCCESS==MrmFetchWidget(projection_dialog_hierarchy,
								"projection_dialog",projection_dialog->shell,
								&(projection_dialog->dialog),&projection_dialog_class))
							{
								XtManageChild(projection_dialog->dialog);
								XtRealizeWidget(projection_dialog->shell);
								if (projection_dialog_address)
								{
									*projection_dialog_address=projection_dialog;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
								"create_Projection_dialog.  Could not fetch the dialog widget");
								XtDestroyWidget(projection_dialog->shell);
								projection_dialog=(struct Projection_dialog *)NULL;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_Projection_dialog.  Could not register identifiers");
							XtDestroyWidget(projection_dialog->shell);
							projection_dialog=(struct Projection_dialog *)NULL;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_Projection_dialog.  Could not register callbacks");
						XtDestroyWidget(projection_dialog->shell);
						projection_dialog=(struct Projection_dialog *)NULL;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Projection_dialog.  Could not create dialog shell");
					DEALLOCATE(projection_dialog);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
	"create_Projection_dialog.  Could not allocate memory for projection dialog");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Projection_dialog.  Could not open hierarchy");
			projection_dialog=(struct Projection_dialog *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Projection_dialog.  Missing user_interface");
		projection_dialog=(struct Projection_dialog *)NULL;
	}
	LEAVE;

	return (projection_dialog);
} /* create_Projection_dialog */

int open_projection_dialog(struct Projection_dialog *projection_dialog)
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
Opens the <projection_dialog>.
==============================================================================*/
{
	char value_string[20];
	enum Spectrum_simple_type spectrum_type;
	int return_code;
	struct Projection *projection;
	Widget option_widget;

	ENTER(open_projection_dialog);
	if (projection_dialog)
	{
		if ((projection_dialog->projection)&&
			(projection= *(projection_dialog->projection)))
		{
			/* configure the dialog box to be consistent with the projection */
			switch (projection->type)
			{
				case HAMMER_PROJECTION:
				{
					option_widget=projection_dialog->type_option.hammer;
				} break;
				case POLAR_PROJECTION:
				{
					option_widget=projection_dialog->type_option.polar;
				} break;
				case CYLINDRICAL_PROJECTION:
				{
					option_widget=projection_dialog->type_option.cylindrical;
				} break;
			}
			XtVaSetValues(projection_dialog->type_option_menu,
				XmNmenuHistory,option_widget,
				NULL);
#if defined (OLD_CODE)
			if (SHOW_COLOUR==projection->colour_option)
			{
				XmToggleButtonGadgetSetState(projection_dialog->show_colour_toggle,True,
					False);
			}
			else
			{
				XmToggleButtonGadgetSetState(projection_dialog->show_colour_toggle,
					False,False);
			}
#endif /* defined (OLD_CODE) */
			if (SHOW_COLOUR==projection->colour_option)
			{
				spectrum_type = Spectrum_get_simple_type(projection->spectrum);
				switch (spectrum_type)
				{
					case BLUE_TO_RED_SPECTRUM:
					{
						option_widget=projection_dialog->spectrum.type_option.blue_red;
					} break;	
					case BLUE_WHITE_RED_SPECTRUM:
					{
						option_widget=projection_dialog->spectrum.type_option.blue_white_red;
					} break;
					case LOG_BLUE_TO_RED_SPECTRUM:
					{
						option_widget=projection_dialog->spectrum.type_option.log_blue_red;
					} break;
					case LOG_RED_TO_BLUE_SPECTRUM:
					{
						option_widget=projection_dialog->spectrum.type_option.log_red_blue;
					} break;
					case RED_TO_BLUE_SPECTRUM:
					{
						option_widget=projection_dialog->spectrum.type_option.red_blue;
					} break;
					default:
					{
						option_widget=projection_dialog->spectrum.type_option.none;
						display_message(ERROR_MESSAGE,
							"open_projection_dialog.  Unknown spectrum");
					} break;
				}
			}
			else
			{
				option_widget=projection_dialog->spectrum.type_option.none;
			}
			XtVaSetValues(projection_dialog->spectrum.type_option_menu,
				XmNmenuHistory,option_widget,
				NULL);
#if defined (OLD_CODE)
			if (SHOW_CONTOURS==projection->contours_option)
			{
				XmToggleButtonGadgetSetState(projection_dialog->contours.show_toggle,
					True,False);
			}
			else
			{
				XmToggleButtonGadgetSetState(projection_dialog->contours.show_toggle,
					False,False);
			}
			switch (projection->contour_thickness)
			{
				case CONSTANT_THICKNESS:
				{
					option_widget=projection_dialog->contours.thickness_option.constant;
				} break;
				case VARIABLE_THICKNESS:
				{
					option_widget=projection_dialog->contours.thickness_option.variable;
				} break;
			}
			XtVaSetValues(projection_dialog->contours.thickness_option_menu,
				XmNmenuHistory,option_widget,
				NULL);
#endif /* defined (OLD_CODE) */
			if (projection->contours_option==SHOW_CONTOURS)
			{
				switch (projection->contour_thickness)
				{
					case CONSTANT_THICKNESS:
					{
						option_widget=
							projection_dialog->contours.type_option.constant_thickness;
					} break;
					case VARIABLE_THICKNESS:
					{
						option_widget=
							projection_dialog->contours.type_option.variable_thickness;
					} break;
				}
			}
			else
			{
				option_widget=projection_dialog->contours.type_option.none;
			}
			XtVaSetValues(projection_dialog->contours.type_option_menu,
				XmNmenuHistory,option_widget,
				NULL);
			sprintf(value_string,"%g",get_Spectrum_maximum(projection->spectrum));
			sscanf(value_string,"%f",&(projection_dialog->range_maximum));
			XtVaSetValues(projection_dialog->range.maximum_value,
				XmNvalue,value_string,
				NULL);
			sprintf(value_string,"%g",get_Spectrum_minimum(projection->spectrum));
			sscanf(value_string,"%f",&(projection_dialog->range_minimum));
			XtVaSetValues(projection_dialog->range.minimum_value,
				XmNvalue,value_string,
				NULL);
			projection_dialog->number_of_contours=projection->number_of_contours;
			if (projection_dialog->number_of_contours<=2)
			{
				projection_dialog->number_of_contours=2;
				XtUnmanageChild(projection_dialog->contours.down_arrow);
				XtManageChild(projection_dialog->contours.up_arrow);
			}
			else
			{
				XtManageChild(projection_dialog->contours.down_arrow);
				if (projection_dialog->number_of_contours<99)
				{
					XtManageChild(projection_dialog->contours.up_arrow);
				}
				else
				{
					projection_dialog->number_of_contours=99;
					XtUnmanageChild(projection_dialog->contours.up_arrow);
				}
			}
			sprintf(value_string,"%2d",projection_dialog->number_of_contours);
			XtVaSetValues(projection_dialog->contours.number,
				XmNlabelString,XmStringCreate(value_string,XmSTRING_DEFAULT_CHARSET),
				NULL);
			sprintf(value_string,"%g",projection->xi_3);
			sscanf(value_string,"%f",&(projection_dialog->xi_3));
			XtVaSetValues(projection_dialog->xi_3_value,
				XmNvalue,value_string,
				NULL);
			switch (projection->elements_option)
			{
				case SHOW_ELEMENT_NAMES_AND_BOUNDARIES:
				{
					option_widget=projection_dialog->elements_option.name_and_boundary;
				} break;
				case SHOW_ELEMENT_BOUNDARIES_ONLY:
				{
					option_widget=projection_dialog->elements_option.boundary_only;
				} break;
				case HIDE_ELEMENTS:
				{
					option_widget=projection_dialog->elements_option.hide;
				} break;
			}
			XtVaSetValues(projection_dialog->elements_option_menu,
				XmNmenuHistory,option_widget,
				NULL);
			switch (projection->nodes_option)
			{
				case SHOW_NODE_NAMES:
				{
					option_widget=projection_dialog->nodes_option.name;
				} break;
				case SHOW_NODE_VALUES:
				{
					option_widget=projection_dialog->nodes_option.value;
				} break;
				case HIDE_NODES:
				{
					option_widget=projection_dialog->nodes_option.hide;
				} break;
			}
			XtVaSetValues(projection_dialog->nodes_option_menu,
				XmNmenuHistory,option_widget,
				NULL);
			switch (projection->fibres_option)
			{
				case HIDE_FIBRES:
				{
					option_widget=projection_dialog->fibres_option.hide;
				} break;
				case SHOW_FIBRES_FINE:
				{
					option_widget=projection_dialog->fibres_option.fine;
				} break;
				case SHOW_FIBRES_MEDIUM:
				{
					option_widget=projection_dialog->fibres_option.medium;
				} break;
				case SHOW_FIBRES_COARSE:
				{
					option_widget=projection_dialog->fibres_option.coarse;
				} break;
			}
			XtVaSetValues(projection_dialog->fibres_option_menu,
				XmNmenuHistory,option_widget,
				NULL);
			if (SHOW_LANDMARKS==projection->landmarks_option)
			{
				XmToggleButtonGadgetSetState(projection_dialog->show_landmarks_toggle,
					True,False);
			}
			else
			{
				XmToggleButtonGadgetSetState(projection_dialog->show_landmarks_toggle,
					False,False);
			}
			if (projection->maintain_aspect_ratio)
			{
				XmToggleButtonGadgetSetState(
					projection_dialog->maintain_aspect_ratio_toggle,True,False);
			}
			else
			{
				XmToggleButtonGadgetSetState(
					projection_dialog->maintain_aspect_ratio_toggle,False,False);
			}
			/* ghost the creator button */
			XtSetSensitive(projection_dialog->creator,False);
			/* pop up the projection dialog */
			XtManageChild(projection_dialog->dialog);
			return_code=1;
		}
		else
		{
			return_code=0;
			display_message(ERROR_MESSAGE,
				"open_projection_dialog.  Missing projection");
		}
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"open_projection_dialog.  Missing projection_dialog");
	}
	LEAVE;

	return (return_code);
} /* open_projection_dialog */

void close_projection_dialog(Widget widget,XtPointer projection_dialog,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Closes the windows associated with the projection dialog box.
==============================================================================*/
{
	struct Projection_dialog *dialog;

	ENTER(close_projection_dialog);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (dialog=(struct Projection_dialog *)projection_dialog)
	{
		/* close the projection dialog box */
		XtUnmanageChild(dialog->dialog);
		/* unghost the activation button */
		XtSetSensitive(dialog->creator,True);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"close_projection_dialog.  Missing projection_dialog");
	}
	LEAVE;
} /* close_projection_dialog */
