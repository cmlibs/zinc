/*******************************************************************************
FILE : map_dialog.c

LAST MODIFIED : 27 November 2001

DESCRIPTION :
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#if defined (MOTIF)
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
#include <Xm/Protocols.h>
#include <Xm/ToggleBG.h>
#include <Mrm/MrmPublic.h>
#include <Mrm/MrmDecls.h>
#endif /* defined (MOTIF) */
#include "general/debug.h"
#include "unemap/map_dialog.h"
#include "unemap/map_dialog.uidh"
#include "unemap/mapping.h"
#include "unemap/rig.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int map_dialog_hierarchy_open=0;
static MrmHierarchy map_dialog_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
static void destroy_map_dialog(Widget widget,XtPointer map_dialog_structure,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Tidys up when the user destroys the map dialog box.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(destroy_map_dialog);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		busy_cursor_off(map_dialog->shell,map_dialog->user_interface);
		if (map_dialog->address)
		{
			*(map_dialog->address)=(struct Map_dialog *)NULL;
		}
		destroy_Shell_list_item(&(map_dialog->shell_list_item));
		/*???unghost the mapping_map_button ? */
		DEALLOCATE(map_dialog);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_map_dialog.  Missing map_dialog_structure");
	}
	LEAVE;
} /* destroy_map_dialog */

static void identify_map_dialog_range_type(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the range type option menu in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_range_type);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		(map_dialog->range).type_option_menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_range_type.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_range_type */

static void identify_map_dialog_range_autom(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Saves the id of the automatic range option in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_range_autom);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		(map_dialog->range).type_option.automatic= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_range_autom.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_range_autom */

static void identify_map_dialog_range_fixed(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Saves the id of the fixed range option in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_range_fixed);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		(map_dialog->range).type_option.fixed= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_range_fixed.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_range_fixed */

static void identify_map_dialog_range_minim(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Saves the id of the minimum value text field in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_range_minim);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->range.minimum_value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_range_minim.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_range_minim */

static void identify_map_dialog_range_maxim(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Saves the id of the maximum value text field in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_range_maxim);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->range.maximum_value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_range_maxim.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_range_maxim */

static void identify_map_dialog_spectrum(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the spectrum type option menu in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_spectrum);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		(map_dialog->spectrum).type_option_menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_spectrum.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_spectrum */

static void identify_map_dialog_spectrum_no(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Saves the id of the no spectrum option in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_spectrum_no);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		(map_dialog->spectrum).type_option.none= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_spectrum_no.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_spectrum_no */

static void identify_map_dialog_blue_red(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Saves the id of the blue->red spectrum option in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_blue_red);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		(map_dialog->spectrum).type_option.blue_red= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_blue_red.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_blue_red */

static void identify_map_dialog_bl_wh_rd(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 May 2000

DESCRIPTION :
Saves the id of the blue_white_red spectrum option in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_bl_wh_rd);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		(map_dialog->spectrum).type_option.blue_white_red= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_bl_wh_rd.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_bl_wh_rd */

static void identify_map_dialog_red_blue(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Saves the id of the red->blue spectrum option in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_red_blue);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		(map_dialog->spectrum).type_option.red_blue= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_red_blue.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_red_blue */

static void identify_map_dialog_log_blue_re(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Saves the id of the log(blue->red) spectrum option in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_log_blue_re);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		(map_dialog->spectrum).type_option.log_blue_red= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_log_blue_re.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_log_blue_re */

static void identify_map_dialog_log_red_blu(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Saves the id of the log(red->blue) spectrum option in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_log_red_blu);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		(map_dialog->spectrum).type_option.log_red_blue= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_log_red_blu.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_log_red_blu */

static void identify_map_dialog_interpolati(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the interpolation option menu in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_interpolati);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->interpolation.option_menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_interpolati.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_interpolati */

static void identify_map_dialog_inter_bicub(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the bicubic interpolation option in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_inter_bicub);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->interpolation.option.bicubic= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_inter_bicub.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_inter_bicub */

static void set_interpolation_bicubic(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Sets the sensitivity of widgets in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(set_interpolation_bicubic);
	USE_PARAMETER(widget_id);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		XtSetSensitive(map_dialog->contours.row_column,True);
		XtSetSensitive(map_dialog->interpolation.mesh_rows,True);
		XtSetSensitive(map_dialog->interpolation.mesh_columns,True);
		if ((map_dialog->map)&&(*(map_dialog->map))&&((*(map_dialog->map))->type)&&
			(POTENTIAL== *((*(map_dialog->map))->type)))
		{
			XtSetSensitive(map_dialog->animation.row_column,True);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_interpolation_bicubic.  Missing map_dialog_structure");
	}
	LEAVE;
} /* set_interpolation_bicubic */

static void identify_map_dialog_inter_direc(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 12 December 2000

DESCRIPTION :
Finds the id of the direct interpolation option in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_inter_direc);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->interpolation.option.direct= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_inter_direc.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_inter_direc */

static void set_interpolation_direct(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 26 November 2001

DESCRIPTION :
Sets the sensitivity of widgets in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(set_interpolation_direct);
	USE_PARAMETER(widget_id);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		XtSetSensitive(map_dialog->contours.row_column,True);
		XtSetSensitive(map_dialog->interpolation.mesh_rows,False);
		XtSetSensitive(map_dialog->interpolation.mesh_columns,False);
		if ((map_dialog->map)&&(*(map_dialog->map))&&((*(map_dialog->map))->type)&&
			(POTENTIAL== *((*(map_dialog->map))->type)))
		{
			if((*(map_dialog->map))->projection_type==THREED_PROJECTION)
			{
				/*not sure what we're going to do with 3D movies yet*/
				/*for now make them behave like bicubic 2D movies */
				XtSetSensitive(map_dialog->animation.row_column,True);
			}
			else
			{
				XtSetSensitive(map_dialog->animation.row_column,False);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_interpolation_direct.  Missing map_dialog_structure");
	}
	LEAVE;
} /* set_interpolation_direct */

static void identify_map_dialog_inter_none(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the none interpolation option in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_inter_none);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->interpolation.option.none= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_inter_none.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_inter_none */

static void set_interpolation_none(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 26 November 2001

DESCRIPTION :
Sets the sensitivity of widgets in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(set_interpolation_none);
	USE_PARAMETER(widget_id);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		XtSetSensitive(map_dialog->contours.row_column,False);
		XtSetSensitive(map_dialog->interpolation.mesh_rows,False);
		XtSetSensitive(map_dialog->interpolation.mesh_columns,False);
		if ((map_dialog->map)&&(*(map_dialog->map))&&((*(map_dialog->map))->type)&&
			(POTENTIAL== *((*(map_dialog->map))->type)))
		{	
			if((*(map_dialog->map))->projection_type==THREED_PROJECTION)
			{
				/*not sure what we're going to do with 3D movies yet*/
				/*for now make them behave like bicubic 2D movies */
				XtSetSensitive(map_dialog->animation.row_column,True);
			}
			else
			{
				XtSetSensitive(map_dialog->animation.row_column,False);
			}		
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_interpolation_none.  Missing map_dialog_structure");
	}
	LEAVE;
} /* set_interpolation_none */

static void id_map_dialog_inter_mrows(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the mesh rows row-column in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(id_map_dialog_inter_mrows);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->interpolation.mesh_rows= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_map_dialog_inter_mrows.  Missing map_dialog_structure");
	}
	LEAVE;
} /* id_map_dialog_inter_mrows */

static void id_map_dialog_inter_mrows_text(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the mesh rows text in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(id_map_dialog_inter_mrows_text);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->interpolation.mesh_rows_text= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_map_dialog_inter_mrows_text.  Missing map_dialog_structure");
	}
	LEAVE;
} /* id_map_dialog_inter_mrows_text */

static void id_map_dialog_inter_mcols(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the mesh columns row-column in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(id_map_dialog_inter_mcols);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->interpolation.mesh_columns= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_map_dialog_inter_mcols.  Missing map_dialog_structure");
	}
	LEAVE;
} /* id_map_dialog_inter_mcols */

static void id_map_dialog_inter_mcols_text(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the mesh columns text in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(id_map_dialog_inter_mcols_text);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->interpolation.mesh_columns_text= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_map_dialog_inter_mcols_text.  Missing map_dialog_structure");
	}
	LEAVE;
} /* id_map_dialog_inter_mcols_text */

static void identify_map_dialog_contours(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the contours row-column in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_contours);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		(map_dialog->contours).row_column= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_contours.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_contours*/

static void identify_map_dialog_contours_op(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the contours type option menu in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_contours_op);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		(map_dialog->contours).type_option_menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_contours_op.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_contours_op */

static void identify_map_dialog_contours_no(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Saves the id of the no contours option in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_contours_no);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		(map_dialog->contours).type_option.none= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_contours_no.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_contours_no */

static void identify_map_dialog_contours_co(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Saves the id of the constant thickness contours option in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_contours_co);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		(map_dialog->contours).type_option.constant_thickness= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_contours_co.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_contours_co */

static void identify_map_dialog_contours_va(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Saves the id of the variable thickness contours option in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_contours_va);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		(map_dialog->contours).type_option.variable_thickness= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_contours_va.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_contours_va */

static void identify_map_dialog_contours_do(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the increment number of contours arrow in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_contours_do);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->contours.down_arrow= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_contours_do.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_contours_do */

static void decrement_number_of_contours(Widget widget,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Decrement the number of contours.
==============================================================================*/
{
	struct Map_dialog *map_dialog;
	char number_string[3];

	ENTER(decrement_number_of_contours);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		if (99==map_dialog->number_of_contours)
		{
			XtManageChild(map_dialog->contours.up_arrow);
		}
		(map_dialog->number_of_contours)--;
		sprintf(number_string,"%2d",map_dialog->number_of_contours);
		XtVaSetValues(map_dialog->contours.number,
			XmNlabelString,XmStringCreate(number_string,XmSTRING_DEFAULT_CHARSET),
			NULL);
		if (2==map_dialog->number_of_contours)
		{
			XtUnmanageChild(map_dialog->contours.down_arrow);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"decrement_number_of_contours.  Missing map_dialog_structure");
	}
	LEAVE;
} /* decrement_number_of_contours */

static void identify_map_dialog_contours_nu(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the label showing the number of contours in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_contours_nu);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->contours.number= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_contours_nu.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_contours_nu */

static void identify_map_dialog_contours_up(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the decrement number of contours arrow in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_contours_up);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->contours.up_arrow= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_contours_up.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_contours_up */

static void increment_number_of_contours(Widget widget,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Increment the number of contours.
==============================================================================*/
{
	struct Map_dialog *map_dialog;
	char number_string[3];

	ENTER(increment_number_of_contours);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		if (2==map_dialog->number_of_contours)
		{
			XtManageChild(map_dialog->contours.down_arrow);
		}
		(map_dialog->number_of_contours)++;
		sprintf(number_string,"%2d",map_dialog->number_of_contours);
		XtVaSetValues(map_dialog->contours.number,
			XmNlabelString,XmStringCreate(number_string,XmSTRING_DEFAULT_CHARSET),
			NULL);
		if (99==map_dialog->number_of_contours)
		{
			XtUnmanageChild(map_dialog->contours.up_arrow);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"increment_number_of_contours.  Missing map_dialog_structure");
	}
	LEAVE;
} /* increment_number_of_contours */

static void identify_map_dialog_elect_menu(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the electrodes option menu in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_elect_menu);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->electrodes.label_menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_elect_menu.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_elect_menu */

static void identify_map_dialog_elect_name(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the name button in electrodes option menu in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_elect_name);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->electrodes.label.name= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_elect_name.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_elect_name */

static void identify_map_dialog_elect_value(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the value button in electrodes option menu in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_elect_value);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->electrodes.label.value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_elect_value.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_elect_value */

static void identify_map_dialog_elect_chann(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the channel button in electrodes option menu in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_elect_chann);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->electrodes.label.channel= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_elect_chann.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_elect_chann */

static void identify_map_dialog_elect_hide(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the hide button in electrodes option menu in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_elect_hide);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->electrodes.label.hide= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_elect_hide.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_elect_hide */

static void identify_map_dialog_elect_marke(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the electrodes marker menu in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_elect_marke);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->electrodes.marker_type_menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_elect_marke.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_elect_marke */

static void identify_map_dialog_elect_circl(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the circle button in electrodes marker menu in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_elect_circl);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->electrodes.marker_type.circle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_elect_circl.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_elect_circl */

static void identify_map_dialog_elect_plus(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the plus button in electrodes marker menu in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_elect_plus);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->electrodes.marker_type.plus= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_elect_plus.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_elect_plus */

static void identify_map_dialog_elect_squar(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the square button in electrodes marker menu in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_elect_squar);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->electrodes.marker_type.square= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_elect_squar.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_elect_squar */

static void identify_map_dialog_elect_none(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 May 2000

DESCRIPTION :
Finds the id of the none button in electrodes marker menu in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_elect_none);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->electrodes.marker_type.none= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_elect_none.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_elect_none */

static void identify_map_dialog_elect_colou(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 May 2000

DESCRIPTION :
Finds the id of the electrodes colour toggle in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_elect_colou);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->electrodes.marker_colour_toggle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_elect_colou.  Missing map_dialog_structure");
	}
	LEAVE;
} /*identify_map_dialog_elect_colou  */

static void identify_map_dialog_elect_size(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Saves the id of the electrode marker size text field in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_elect_size);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->electrodes.marker_size_text= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_elect_size.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_elect_size */

static void identify_map_dialog_fibre_menu(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the fibres option menu in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_fibre_menu);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->fibres_option_menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_fibre_menu.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_fibre_menu */

static void identify_map_dialog_fibre_hide(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the fibre button in the fibres option menu in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_fibre_hide);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->fibres_option.hide= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_fibre_hide.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_fibre_hide */

static void identify_map_dialog_fibre_fine(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the fine button in the fibres option menu in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_fibre_fine);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->fibres_option.fine= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_fibre_fine.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_fibre_fine */

static void identify_map_dialog_fibre_mediu(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the medium button in the fibres option menu in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_fibre_mediu);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->fibres_option.medium= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_fibre_mediu.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_fibre_mediu */

static void identify_map_dialog_fibre_coars(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the coarse button in the fibres option menu in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_fibre_coars);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->fibres_option.coarse= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_fibre_coars.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_fibre_coars */

static void identify_map_dialog_landmarks_b(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the landmarks button in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_landmarks_b);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->show_landmarks_toggle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_landmarks_b.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_landmarks_b */

static void identify_map_dialog_extrema_but(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the extrema button in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_extrema_but);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->show_extrema_toggle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_extrema_but.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_extrema_but */

static void identify_map_dialog_aspect_rati(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the maintain aspect ratio button in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_aspect_rati);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->maintain_aspect_ratio_toggle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_aspect_rati.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_aspect_rati */

static void identify_map_dialog_print_spect(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the print spectrum button in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_print_spect);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->print_spectrum_toggle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_print_spect.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_print_spect */

static void identify_map_dialog_animation(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the animation row/column in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_animation);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->animation.row_column= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_animation.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_animation */

static void id_map_dialog_animation_start_t(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the animation start time text in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(id_map_dialog_animation_start_t);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->animation.start_time_text= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_map_dialog_animation_start_t.  Missing map_dialog_structure");
	}
	LEAVE;
} /* id_dialog_map_animation_start_t */

static void id_map_dialog_animation_end_tim(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the animation end time text in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(id_map_dialog_animation_end_tim);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->animation.end_time_text= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_map_dialog_animation_end_tim.  Missing map_dialog_structure");
	}
	LEAVE;
} /* id_dialog_map_animation_end_tim */

static void id_map_dialog_animation_frame_n(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the animation frame number text in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(id_map_dialog_animation_frame_n);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->animation.frame_number_text= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_map_dialog_animation_frame_n.  Missing map_dialog_structure");
	}
	LEAVE;
} /* id_dialog_map_animation_frame_n */

static void id_map_dialog_animation_num_fra(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the animation number of frames text in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(id_map_dialog_animation_num_fra);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->animation.number_of_frames_text= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_map_dialog_animation_num_fra.  Missing map_dialog_structure");
	}
	LEAVE;
} /* id_dialog_map_animation_num_fra */

static void identify_map_dialog_cancel_butt(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the cancel button in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_cancel_butt);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->cancel_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_cancel_butt.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_cancel_butt */

static void identify_map_dialog_apply_butto(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the apply button in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_apply_butto);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->apply_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_apply_butto.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_apply_butto */

static void identify_map_dialog_ok_button(Widget *widget_id,
	XtPointer map_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the ok button in the map dialog.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(identify_map_dialog_ok_button);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		map_dialog->ok_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_map_dialog_ok_button.  Missing map_dialog_structure");
	}
	LEAVE;
} /* identify_map_dialog_ok_button */

/*
Global functions
----------------
*/
struct Map_dialog *create_Map_dialog(struct Map_dialog **map_dialog_address,
	struct Map **map,Widget activation,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 25 May 2000

DESCRIPTION :
Allocates the memory for a map dialog.  Retrieves the necessary widgets and
initializes the appropriate fields.
==============================================================================*/
{
	Atom WM_DELETE_WINDOW;
	MrmType map_dialog_class;
	static MrmRegisterArg callback_list[]={
		{"identify_map_dialog_range_type",
			(XtPointer)identify_map_dialog_range_type},
		{"identify_map_dialog_range_autom",
			(XtPointer)identify_map_dialog_range_autom},
		{"identify_map_dialog_range_fixed",
			(XtPointer)identify_map_dialog_range_fixed},
		{"identify_map_dialog_range_minim",
			(XtPointer)identify_map_dialog_range_minim},
		{"identify_map_dialog_range_maxim",
			(XtPointer)identify_map_dialog_range_maxim},
		{"identify_map_dialog_spectrum",
			(XtPointer)identify_map_dialog_spectrum},
		{"identify_map_dialog_spectrum_no",
			(XtPointer)identify_map_dialog_spectrum_no},
		{"identify_map_dialog_blue_red",
			(XtPointer)identify_map_dialog_blue_red},
		{"identify_map_dialog_bl_wh_rd",
			(XtPointer)identify_map_dialog_bl_wh_rd},
		{"identify_map_dialog_red_blue",
			(XtPointer)identify_map_dialog_red_blue},
		{"identify_map_dialog_log_blue_re",
			(XtPointer)identify_map_dialog_log_blue_re},
		{"identify_map_dialog_log_red_blu",
			(XtPointer)identify_map_dialog_log_red_blu},
		{"identify_map_dialog_interpolati",
			(XtPointer)identify_map_dialog_interpolati},
		{"identify_map_dialog_inter_bicub",
			(XtPointer)identify_map_dialog_inter_bicub},
		{"set_interpolation_bicubic",(XtPointer)set_interpolation_bicubic},
		{"identify_map_dialog_inter_direc",
			(XtPointer)identify_map_dialog_inter_direc},
		{"set_interpolation_direct",(XtPointer)set_interpolation_direct},
		{"identify_map_dialog_inter_none",
			(XtPointer)identify_map_dialog_inter_none},
		{"set_interpolation_none",(XtPointer)set_interpolation_none},
		{"id_map_dialog_inter_mrows",(XtPointer)id_map_dialog_inter_mrows},
		{"id_map_dialog_inter_mrows_text",
			(XtPointer)id_map_dialog_inter_mrows_text},
		{"id_map_dialog_inter_mcols",(XtPointer)id_map_dialog_inter_mcols},
		{"id_map_dialog_inter_mcols_text",
			(XtPointer)id_map_dialog_inter_mcols_text},
		{"identify_map_dialog_contours",
			(XtPointer)identify_map_dialog_contours},
		{"identify_map_dialog_contours_op",
			(XtPointer)identify_map_dialog_contours_op},
		{"identify_map_dialog_contours_no",
			(XtPointer)identify_map_dialog_contours_no},
		{"identify_map_dialog_contours_co",
			(XtPointer)identify_map_dialog_contours_co},
		{"identify_map_dialog_contours_va",
			(XtPointer)identify_map_dialog_contours_va},
		{"identify_map_dialog_contours_do",
			(XtPointer)identify_map_dialog_contours_do},
		{"decrement_number_of_contours",(XtPointer)decrement_number_of_contours},
		{"identify_map_dialog_contours_nu",
			(XtPointer)identify_map_dialog_contours_nu},
		{"identify_map_dialog_contours_up",
			(XtPointer)identify_map_dialog_contours_up},
		{"increment_number_of_contours",(XtPointer)increment_number_of_contours},
		{"identify_map_dialog_elect_name",
			(XtPointer)identify_map_dialog_elect_name},
		{"identify_map_dialog_elect_value",
			(XtPointer)identify_map_dialog_elect_value},
		{"identify_map_dialog_elect_chann",
			(XtPointer)identify_map_dialog_elect_chann},
		{"identify_map_dialog_elect_hide",
			(XtPointer)identify_map_dialog_elect_hide},
		{"identify_map_dialog_elect_menu",
			(XtPointer)identify_map_dialog_elect_menu},
		{"identify_map_dialog_elect_circl",
			(XtPointer)identify_map_dialog_elect_circl},
		{"identify_map_dialog_elect_plus",
			(XtPointer)identify_map_dialog_elect_plus},
		{"identify_map_dialog_elect_squar",
			(XtPointer)identify_map_dialog_elect_squar},
		{"identify_map_dialog_elect_none",
			(XtPointer)identify_map_dialog_elect_none},
		{"identify_map_dialog_elect_marke",
			(XtPointer)identify_map_dialog_elect_marke},
		{"identify_map_dialog_elect_colou",
			(XtPointer)identify_map_dialog_elect_colou},	
		{"identify_map_dialog_elect_size",
			(XtPointer)identify_map_dialog_elect_size},
		{"identify_map_dialog_landmarks_b",
			(XtPointer)identify_map_dialog_landmarks_b},
		{"identify_map_dialog_extrema_but",
			(XtPointer)identify_map_dialog_extrema_but},
		{"identify_map_dialog_aspect_rati",
			(XtPointer)identify_map_dialog_aspect_rati},
		{"identify_map_dialog_print_spect",
			(XtPointer)identify_map_dialog_print_spect},
		{"identify_map_dialog_fibre_hide",
			(XtPointer)identify_map_dialog_fibre_hide},
		{"identify_map_dialog_fibre_fine",
			(XtPointer)identify_map_dialog_fibre_fine},
		{"identify_map_dialog_fibre_mediu",
			(XtPointer)identify_map_dialog_fibre_mediu},
		{"identify_map_dialog_fibre_coars",
			(XtPointer)identify_map_dialog_fibre_coars},
		{"identify_map_dialog_fibre_menu",
			(XtPointer)identify_map_dialog_fibre_menu},
		{"identify_map_dialog_animation",(XtPointer)identify_map_dialog_animation},
		{"id_map_dialog_animation_start_t",
			(XtPointer)id_map_dialog_animation_start_t},
		{"id_map_dialog_animation_end_tim",
			(XtPointer)id_map_dialog_animation_end_tim},
		{"id_map_dialog_animation_num_fra",
			(XtPointer)id_map_dialog_animation_num_fra},
		{"id_map_dialog_animation_frame_n",
			(XtPointer)id_map_dialog_animation_frame_n},
		{"identify_map_dialog_ok_button",(XtPointer)identify_map_dialog_ok_button},
		{"identify_map_dialog_apply_butto",
			(XtPointer)identify_map_dialog_apply_butto},
		{"identify_map_dialog_cancel_butt",
			(XtPointer)identify_map_dialog_cancel_butt},
		{"close_map_dialog",(XtPointer)close_map_dialog}};
	static MrmRegisterArg identifier_list[]=
	{
		{"map_dialog_structure",(XtPointer)NULL}
	};
	struct Map_dialog *map_dialog;
	Widget parent;
	XmFontList font_list;

	ENTER(create_Map_dialog);
	/* check arguments */
	if (user_interface)
	{
		if (MrmOpenHierarchy_base64_string(map_dialog_uidh,
			&map_dialog_hierarchy,&map_dialog_hierarchy_open))
		{
			if (ALLOCATE(map_dialog,struct Map_dialog,1))
			{
				map_dialog->user_interface=user_interface;
				map_dialog->activation=activation;
				map_dialog->dialog=(Widget)NULL;
				map_dialog->shell=(Widget)NULL;
				map_dialog->shell_list_item=(struct Shell_list_item *)NULL;
				map_dialog->range.type_option_menu=(Widget)NULL;
				map_dialog->range.type_option.automatic=(Widget)NULL;
				map_dialog->range.type_option.fixed=(Widget)NULL;
				map_dialog->range.minimum_value=(Widget)NULL;
				map_dialog->range.maximum_value=(Widget)NULL;
				map_dialog->spectrum.type_option_menu=(Widget)NULL;
				map_dialog->spectrum.type_option.none=(Widget)NULL;
				map_dialog->spectrum.type_option.blue_red=(Widget)NULL;	
				map_dialog->spectrum.type_option.blue_white_red=(Widget)NULL;
				map_dialog->spectrum.type_option.red_blue=(Widget)NULL;
				map_dialog->spectrum.type_option.log_blue_red=(Widget)NULL;
				map_dialog->spectrum.type_option.log_red_blue=(Widget)NULL;
				map_dialog->interpolation.option_menu=(Widget)NULL;
				map_dialog->interpolation.option.bicubic=(Widget)NULL;
				map_dialog->interpolation.option.direct=(Widget)NULL;
				map_dialog->interpolation.option.none=(Widget)NULL;
				map_dialog->interpolation.mesh_rows=(Widget)NULL;
				map_dialog->interpolation.mesh_rows_text=(Widget)NULL;
				map_dialog->interpolation.mesh_columns=(Widget)NULL;
				map_dialog->interpolation.mesh_columns_text=(Widget)NULL;
				map_dialog->contours.row_column=(Widget)NULL;
				map_dialog->contours.type_option_menu=(Widget)NULL;
				map_dialog->contours.type_option.none=(Widget)NULL;
				map_dialog->contours.type_option.constant_thickness=(Widget)NULL;
				map_dialog->contours.type_option.variable_thickness=(Widget)NULL;
				map_dialog->contours.down_arrow=(Widget)NULL;
				map_dialog->contours.number=(Widget)NULL;
				map_dialog->contours.up_arrow=(Widget)NULL;
				map_dialog->electrodes.label_menu=(Widget)NULL;
				map_dialog->electrodes.label.name=(Widget)NULL;
				map_dialog->electrodes.label.value=(Widget)NULL;
				map_dialog->electrodes.label.channel=(Widget)NULL;
				map_dialog->electrodes.label.hide=(Widget)NULL;
				map_dialog->electrodes.marker_type_menu=(Widget)NULL;
				map_dialog->electrodes.marker_type.circle=(Widget)NULL;
				map_dialog->electrodes.marker_type.plus=(Widget)NULL;
				map_dialog->electrodes.marker_type.square=(Widget)NULL;	
				map_dialog->electrodes.marker_type.none=(Widget)NULL;
				map_dialog->electrodes.marker_colour_toggle=(Widget)NULL;	
				map_dialog->electrodes.marker_size_text=(Widget)NULL;
				map_dialog->fibres_option_menu=(Widget)NULL;
				map_dialog->fibres_option.hide=(Widget)NULL;
				map_dialog->fibres_option.fine=(Widget)NULL;
				map_dialog->fibres_option.medium=(Widget)NULL;
				map_dialog->fibres_option.coarse=(Widget)NULL;
				map_dialog->show_landmarks_toggle=(Widget)NULL;
				map_dialog->show_extrema_toggle=(Widget)NULL;
				map_dialog->maintain_aspect_ratio_toggle=(Widget)NULL;
				map_dialog->print_spectrum_toggle=(Widget)NULL;
				map_dialog->animation.row_column=(Widget)NULL;
				map_dialog->animation.start_time_text=(Widget)NULL;
				map_dialog->animation.end_time_text=(Widget)NULL;
				map_dialog->animation.number_of_frames_text=(Widget)NULL;
				map_dialog->animation.frame_number_text=(Widget)NULL;
				map_dialog->ok_button=(Widget)NULL;
				map_dialog->cancel_button=(Widget)NULL;
				map_dialog->address=map_dialog_address;
				map_dialog->map=map;
				/* create the dialog shell */
				if (!(parent=activation)||(True!=XtIsWidget(parent)))
				{
					parent=user_interface->application_shell;
				}
				if (map_dialog->shell=XtVaCreatePopupShell(
					"mapping_map_dialog_shell",
					xmDialogShellWidgetClass,parent,
					XmNdeleteResponse,XmDO_NOTHING,
/*					XmNmwmDecorations,MWM_DECOR_ALL&(~MWM_DECOR_RESIZEH),
					XmNmwmFunctions,MWM_FUNC_MOVE&MWM_FUNC_CLOSE,*/
					XmNmwmDecorations,MWM_DECOR_ALL|MWM_DECOR_RESIZEH,
					XmNmwmFunctions,MWM_FUNC_MOVE|MWM_FUNC_CLOSE,
					XmNtitle,"Map settings",
					NULL))
				{
					map_dialog->shell_list_item=
						create_Shell_list_item(&(map_dialog->shell),user_interface);
					/* Set up window manager callback for close window message */
					WM_DELETE_WINDOW=XmInternAtom(
						XtDisplay(map_dialog->shell),"WM_DELETE_WINDOW",False);
					XmAddWMProtocolCallback(map_dialog->shell,
						WM_DELETE_WINDOW,close_map_dialog,map_dialog);
					/* add the destroy callback */
					XtAddCallback(map_dialog->shell,XmNdestroyCallback,
						destroy_map_dialog,(XtPointer)map_dialog);
					/* register the other callbacks */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(map_dialog_hierarchy,
						callback_list,XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value=(XtPointer)map_dialog;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(map_dialog_hierarchy,
							identifier_list,XtNumber(identifier_list)))
						{
							/* fetch the dialog widget */
							if (MrmSUCCESS==MrmFetchWidget(map_dialog_hierarchy,"map_dialog",
								map_dialog->shell,&(map_dialog->dialog),&map_dialog_class))
							{
								/* set fonts for option menus */
								if (((map_dialog->range).type_option_menu)&&
									((map_dialog->range).type_option.automatic))
								{
									XtVaGetValues((map_dialog->range).type_option.automatic,
										XmNfontList,&font_list,NULL);
									XtVaSetValues(XmOptionLabelGadget((map_dialog->range).
										type_option_menu),XmNfontList,font_list,NULL);
								}
								if (((map_dialog->spectrum).type_option_menu)&&
									((map_dialog->spectrum).type_option.none))
								{
									XtVaGetValues((map_dialog->spectrum).type_option.none,
										XmNfontList,&font_list,NULL);
									XtVaSetValues(XmOptionLabelGadget((map_dialog->spectrum).
										type_option_menu),XmNfontList,font_list,NULL);
								}
								if (((map_dialog->interpolation).option_menu)&&
									((map_dialog->interpolation).option.none))
								{
									XtVaGetValues((map_dialog->interpolation).option.none,
										XmNfontList,&font_list,NULL);
									XtVaSetValues(XmOptionLabelGadget((map_dialog->interpolation).
										option_menu),XmNfontList,font_list,NULL);
								}
								if (((map_dialog->contours).type_option_menu)&&
									((map_dialog->contours).type_option.none))
								{
									XtVaGetValues((map_dialog->contours).type_option.none,
										XmNfontList,&font_list,NULL);
									XtVaSetValues(XmOptionLabelGadget((map_dialog->contours).
										type_option_menu),XmNfontList,font_list,NULL);
								}
								if ((map_dialog->electrodes.label_menu)&&
									(map_dialog->electrodes.label.hide))
								{
									XtVaGetValues(map_dialog->electrodes.label.hide,
										XmNfontList,&font_list,NULL);
									XtVaSetValues(XmOptionLabelGadget(map_dialog->
										electrodes.label_menu),XmNfontList,font_list,NULL);
								}
								if ((map_dialog->electrodes.marker_type_menu)&&
									(map_dialog->electrodes.marker_type.plus))
								{
									XtVaGetValues(map_dialog->electrodes.marker_type.plus,
										XmNfontList,&font_list,NULL);
									XtVaSetValues(XmOptionLabelGadget(map_dialog->
										electrodes.marker_type_menu),XmNfontList,font_list,NULL);
								}
								if ((map_dialog->fibres_option_menu)&&
									(map_dialog->fibres_option.hide))
								{
									XtVaGetValues(map_dialog->fibres_option.hide,
										XmNfontList,&font_list,NULL);
									XtVaSetValues(XmOptionLabelGadget(map_dialog->
										fibres_option_menu),XmNfontList,font_list,NULL);
								}
								XtManageChild(map_dialog->dialog);
								XtRealizeWidget(map_dialog->shell);
								if (map_dialog_address)
								{
									*map_dialog_address=map_dialog;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"create_Map_dialog.  Could not fetch the dialog widget");
								XtDestroyWidget(map_dialog->shell);
								map_dialog=(struct Map_dialog *)NULL;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_Map_dialog.  Could not register identifiers");
							XtDestroyWidget(map_dialog->shell);
							map_dialog=(struct Map_dialog *)NULL;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_Map_dialog.  Could not register callbacks");
						XtDestroyWidget(map_dialog->shell);
						map_dialog=(struct Map_dialog *)NULL;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Map_dialog.  Could not create dialog shell");
					DEALLOCATE(map_dialog);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_Map_dialog.  Could not allocate memory for map dialog");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Map_dialog.  Could not open hierarchy");
			map_dialog=(struct Map_dialog *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Map_dialog.  Missing user_interface");
		map_dialog=(struct Map_dialog *)NULL;
	}
	LEAVE;

	return (map_dialog);
} /* create_Map_dialog */

int open_map_dialog(struct Map_dialog *map_dialog)
/*******************************************************************************
LAST MODIFIED : 27 November 2001

DESCRIPTION :
Opens the <map_dialog>.
==============================================================================*/
{
	char value_string[20];
	enum Spectrum_simple_type spectrum_type;
	int return_code;
	struct Map *map;	
	struct Rig *rig;
	Widget option_widget;

	ENTER(open_map_dialog);
	rig =(struct Rig *)NULL;
	map=(struct Map *)NULL;
	if (map_dialog)
	{
		if ((map_dialog->map)&&(map= *(map_dialog->map)))
		{
			/* configure the dialog box to be consistent with the map */
			if(map->colour_electrodes_with_signal)
			/* colour_electrodes_with_signal can be changed in the code (draw_map_2d), as well  */
			/* from the GUI, so ensure in synce here. cf update_map_from_dialog*/
			{
				XmToggleButtonGadgetSetState(map_dialog->electrodes.marker_colour_toggle
					,True,False);
			}
			else
			{	
				XmToggleButtonGadgetSetState(map_dialog->electrodes.marker_colour_toggle
					,False,False);
			}
			if(*(map->type)==NO_MAP_FIELD)
			/* if we have no map field, can't colour electrodes with signal, so disable this option*/
			{
				XtSetSensitive(map_dialog->electrodes.marker_colour_toggle,False);
			}
			else
			{
				XtSetSensitive(map_dialog->electrodes.marker_colour_toggle,True);
			}
			if (map->colour_option==SHOW_COLOUR)
			{ 
				spectrum_type=Spectrum_get_contoured_simple_type(
					map->drawing_information->spectrum);
				switch (spectrum_type)
				{
					case BLUE_TO_RED_SPECTRUM:
					{
						option_widget=map_dialog->spectrum.type_option.blue_red;
					} break;
					case LOG_BLUE_TO_RED_SPECTRUM:
					{
						option_widget=map_dialog->spectrum.type_option.log_blue_red;
					} break;
					case BLUE_WHITE_RED_SPECTRUM:
					{
						option_widget=map_dialog->spectrum.type_option.blue_white_red;
					} break;
					case LOG_RED_TO_BLUE_SPECTRUM:
					{
						option_widget=map_dialog->spectrum.type_option.log_red_blue;
					} break;
					case RED_TO_BLUE_SPECTRUM:
					{
						option_widget=map_dialog->spectrum.type_option.red_blue;
					} break;
					default:
					{
						option_widget=map_dialog->spectrum.type_option.none;
						display_message(ERROR_MESSAGE,
							"open_map_dialog.  Unknown spectrum");
					} break;
				}
			}
			else
			{
				option_widget=map_dialog->spectrum.type_option.none;
			}
			XtVaSetValues(map_dialog->spectrum.type_option_menu,
				XmNmenuHistory,option_widget,
				NULL);
			if((map->rig_pointer)&&(rig=*(map->rig_pointer))&&(rig->current_region)&&
				(rig->current_region->type==TORSO)&&
				((*(map->type)==POTENTIAL)||(*(map->type)==NO_MAP_FIELD)) )
			/* can only have DIRECT_INTERPOLATION for TORSO,POTENTIAL */	
			{
				XtSetSensitive(map_dialog->interpolation.option.direct,True);
			}
			else
			{
				XtSetSensitive(map_dialog->interpolation.option.direct,False);
				if(map->interpolation_type==DIRECT_INTERPOLATION)
				{
					map->interpolation_type=BICUBIC_INTERPOLATION;
				}
			}			
			switch (map->interpolation_type)
			{
				case NO_INTERPOLATION:
				{
					option_widget=map_dialog->interpolation.option.none;
					XtSetSensitive(map_dialog->contours.row_column,False);
					XtSetSensitive(map_dialog->interpolation.mesh_rows,False);
					XtSetSensitive(map_dialog->interpolation.mesh_columns,False);
				} break;
				case BICUBIC_INTERPOLATION:
				{
					option_widget=map_dialog->interpolation.option.bicubic;
					XtSetSensitive(map_dialog->contours.row_column,True);
					XtSetSensitive(map_dialog->interpolation.mesh_rows,True);
					XtSetSensitive(map_dialog->interpolation.mesh_columns,True);
				} break;
				case DIRECT_INTERPOLATION:
				{
					option_widget=map_dialog->interpolation.option.direct;
					XtSetSensitive(map_dialog->contours.row_column,True);
					XtSetSensitive(map_dialog->interpolation.mesh_rows,False);
					XtSetSensitive(map_dialog->interpolation.mesh_columns,False);
				} break;
			}				

			XtVaSetValues(map_dialog->interpolation.option_menu,
				XmNmenuHistory,option_widget,
				NULL);
			map_dialog->number_of_mesh_rows=map->finite_element_mesh_rows;
			if (map_dialog->number_of_mesh_rows<1)
			{
				map_dialog->number_of_mesh_rows=1;
			}
			sprintf(value_string,"%d",map_dialog->number_of_mesh_rows);
			XtVaSetValues(map_dialog->interpolation.mesh_rows_text,
				XmNvalue,value_string,
				NULL);
			map_dialog->number_of_mesh_columns=map->finite_element_mesh_columns;
			if (map_dialog->number_of_mesh_columns<1)
			{
				map_dialog->number_of_mesh_columns=1;
			}
			sprintf(value_string,"%d",map_dialog->number_of_mesh_columns);
			XtVaSetValues(map_dialog->interpolation.mesh_columns_text,
				XmNvalue,value_string,
				NULL);
			if (map->contours_option==SHOW_CONTOURS)
			{
				switch (map->contour_thickness)
				{
					case CONSTANT_THICKNESS:
					{
						option_widget=map_dialog->contours.type_option.constant_thickness;
					} break;
					case VARIABLE_THICKNESS:
					{
						option_widget=map_dialog->contours.type_option.variable_thickness;
					} break;
				}
			}
			else
			{
				option_widget=map_dialog->contours.type_option.none;
			}
			XtVaSetValues(map_dialog->contours.type_option_menu,
				XmNmenuHistory,option_widget,
				NULL);
			if (map->fixed_range)
			{
				option_widget=map_dialog->range.type_option.fixed;
			}
			else
			{
				option_widget=map_dialog->range.type_option.automatic;
			}
			XtVaSetValues(map_dialog->range.type_option_menu,
				XmNmenuHistory,option_widget,
				NULL);
			sprintf(value_string,"%g",map->maximum_value);
			sscanf(value_string,"%f",&(map_dialog->range_maximum));
			XtVaSetValues(map_dialog->range.maximum_value,
				XmNvalue,value_string,
				NULL);
			sprintf(value_string,"%g",map->minimum_value);
			sscanf(value_string,"%f",&(map_dialog->range_minimum));
			XtVaSetValues(map_dialog->range.minimum_value,
				XmNvalue,value_string,
				NULL);
			map_dialog->number_of_contours=map->number_of_contours;
			if (map_dialog->number_of_contours<=2)
			{
				map_dialog->number_of_contours=2;
				XtUnmanageChild(map_dialog->contours.down_arrow);
				XtManageChild(map_dialog->contours.up_arrow);
			}
			else
			{
				XtManageChild(map_dialog->contours.down_arrow);
				if (map_dialog->number_of_contours<99)
				{
					XtManageChild(map_dialog->contours.up_arrow);
				}
				else
				{
					map_dialog->number_of_contours=99;
					XtUnmanageChild(map_dialog->contours.up_arrow);
				}
			}
			sprintf(value_string,"%2d",map_dialog->number_of_contours);
			XtVaSetValues(map_dialog->contours.number,
				XmNlabelString,XmStringCreate(value_string,XmSTRING_DEFAULT_CHARSET),
				NULL);
			switch (map->electrodes_label_type)
			{
				case SHOW_ELECTRODE_NAMES:
				{
					option_widget=map_dialog->electrodes.label.name;
				} break;
				case SHOW_ELECTRODE_VALUES:
				{
					option_widget=map_dialog->electrodes.label.value;
				} break;
				case SHOW_CHANNEL_NUMBERS:
				{
					option_widget=map_dialog->electrodes.label.channel;
				} break;
				case HIDE_ELECTRODE_LABELS:
				{
					option_widget=map_dialog->electrodes.label.hide;
				} break;
			}
			XtVaSetValues(map_dialog->electrodes.label_menu,
				XmNmenuHistory,option_widget,
				NULL);
			map_dialog->electrodes_marker_size=map->electrodes_marker_size;
			if (map_dialog->electrodes_marker_size<1)
			{
				map_dialog->electrodes_marker_size=1;
			}
			sprintf(value_string,"%d",map_dialog->electrodes_marker_size);
			XtVaSetValues(map_dialog->electrodes.marker_size_text,
				XmNvalue,value_string,
				NULL);
			switch (map->electrodes_marker_type)
			{
				case CIRCLE_ELECTRODE_MARKER:
				{
					option_widget=map_dialog->electrodes.marker_type.circle;
				} break;
				case PLUS_ELECTRODE_MARKER:
				{
					option_widget=map_dialog->electrodes.marker_type.plus;
				} break;
				case SQUARE_ELECTRODE_MARKER:
				{
					option_widget=map_dialog->electrodes.marker_type.square;
				} break;	
				case HIDE_ELECTRODE_MARKER:
				{
					option_widget=map_dialog->electrodes.marker_type.none;
				} break;
			}
			XtVaSetValues(map_dialog->electrodes.marker_type_menu,
				XmNmenuHistory,option_widget,
				NULL);
			switch (map->fibres_option)
			{
				case HIDE_FIBRES:
				{
					option_widget=map_dialog->fibres_option.hide;
				} break;
				case SHOW_FIBRES_FINE:
				{
					option_widget=map_dialog->fibres_option.fine;
				} break;
				case SHOW_FIBRES_MEDIUM:
				{
					option_widget=map_dialog->fibres_option.medium;
				} break;
				case SHOW_FIBRES_COARSE:
				{
					option_widget=map_dialog->fibres_option.coarse;
				} break;
			}
			XtVaSetValues(map_dialog->fibres_option_menu,
				XmNmenuHistory,option_widget,
				NULL);
			if (map->landmarks_option==SHOW_LANDMARKS)
			{
				XmToggleButtonGadgetSetState(map_dialog->show_landmarks_toggle,True,
					False);
			}
			else
			{
				XmToggleButtonGadgetSetState(map_dialog->show_landmarks_toggle,False,
					False);
			}
			if (map->maintain_aspect_ratio)
			{
				XmToggleButtonGadgetSetState(map_dialog->maintain_aspect_ratio_toggle,
					True,False);
			}
			else
			{
				XmToggleButtonGadgetSetState(map_dialog->maintain_aspect_ratio_toggle,
					False,False);
			}
			if (map->print_spectrum)
			{
				XmToggleButtonGadgetSetState(map_dialog->print_spectrum_toggle,True,
					False);
			}
			else
			{
				XmToggleButtonGadgetSetState(map_dialog->print_spectrum_toggle,False,
					False);
			}
			/* set animation options */
			/*??JW this will incorretly report the number of frames for*/
			/*a 3D animations. Need to sort out the sub_maps for 3D */
			/* 3D animation still works */
			map_dialog->number_of_frames=map->number_of_sub_maps;	
			sprintf(value_string,"%d",map->number_of_sub_maps);

			XtVaSetValues(map_dialog->animation.number_of_frames_text,
				XmNvalue,value_string,
				NULL);
			map_dialog->frame_number=(map->sub_map_number)+1;
			sprintf(value_string,"%d",map_dialog->frame_number);
			XtVaSetValues(map_dialog->animation.frame_number_text,
				XmNvalue,value_string,
				NULL);
			sprintf(value_string,"%g",map->start_time);
			sscanf(value_string,"%f",&(map_dialog->start_time));
			XtVaSetValues(map_dialog->animation.start_time_text,
				XmNvalue,value_string,NULL);
			sprintf(value_string,"%g",map->end_time);
			sscanf(value_string,"%f",&(map_dialog->end_time));
			XtVaSetValues(map_dialog->animation.end_time_text,
				XmNvalue,value_string,
				NULL);					  
			if ((map->type)&&(POTENTIAL==*(map->type)))
			{
				switch(map->interpolation_type)
				{
					case BICUBIC_INTERPOLATION:
					{
						if((ELECTRICAL_IMAGING==*map->analysis_mode)&&
							(*map->first_eimaging_event))
						{	
							/*no movies if showing (lots of little) maps of electrical imaging events*/
							XtSetSensitive(map_dialog->animation.row_column,False);
						}
						else
						{
							XtSetSensitive(map_dialog->animation.row_column,True);
						}
					}break;
					case NO_INTERPOLATION:
					case DIRECT_INTERPOLATION:
					default:
					{
						if(map->projection_type==THREED_PROJECTION)
						{
							/*not sure what we're going to do with 3D movies yet*/
							/*for now make them behave like bicubic 2D movies */
							XtSetSensitive(map_dialog->animation.row_column,True);
						}
						else
						{
							XtSetSensitive(map_dialog->animation.row_column,False);
						}
					}break;				
				}/* switch(map->interpolation_type)	*/
			}
			else
			{
				XtSetSensitive(map_dialog->animation.row_column,False);
			}	
			/*???Set menu history for interpolation choice */
			/* ghost the activation button */
			XtSetSensitive(map_dialog->activation,False);
			/* pop up the map dialog */
			busy_cursor_on(map_dialog->shell,map_dialog->user_interface);
			XtManageChild(map_dialog->dialog);
			return_code=1;
		}
		else
		{
			return_code=0;
			display_message(ERROR_MESSAGE,"open_map_dialog.  Missing map");
		}
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,"open_map_dialog.  Missing map_dialog");
	}
	LEAVE;

	return (return_code);
} /* open_map_dialog */

void close_map_dialog(Widget widget,XtPointer map_dialog_structure,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Closes the windows associated with the map dialog box.
==============================================================================*/
{
	struct Map_dialog *map_dialog;

	ENTER(close_map_dialog);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (map_dialog=(struct Map_dialog *)map_dialog_structure)
	{
		busy_cursor_off(map_dialog->shell,map_dialog->user_interface);
		/* close the map dialog box */
		XtUnmanageChild(map_dialog->dialog);
		/* unghost the activation button */
		XtSetSensitive(map_dialog->activation,True);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"close_map_dialog.  Missing map_dialog_structure");
	}
	LEAVE;
} /* close_map_dialog */
