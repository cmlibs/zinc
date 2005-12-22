/*******************************************************************************
FILE : spectrum_editor_settings.c

LAST MODIFIED : 14 July 1998

DESCRIPTION :
Provides the widgets to manipulate element group settings.
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
#include <math.h>
#include <stdio.h>
#if defined (MOTIF)
#include <Xm/Xm.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include <Xm/RowColumn.h>
#endif /* defined (MOTIF) */
#include "general/callback_motif.h"
#include "general/debug.h"
#include "graphics/spectrum.h"
#include "graphics/spectrum_settings.h"
#include "graphics/spectrum_editor_settings.h"
static char spectrum_editor_settings_uidh[] =
#include "graphics/spectrum_editor_settings.uidh"
	;
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int spectrum_editor_settings_hierarchy_open=0;
static MrmHierarchy spectrum_editor_settings_hierarchy;
#endif /* defined (MOTIF) */

struct Spectrum_editor_settings
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
Contains all the information carried by the graphical element editor widget.
==============================================================================*/
{
	struct Spectrum_settings *current_settings;
	struct Callback_data update_callback;
	Widget type_menu_widget, type_option_widget;
	Widget reverse_toggle_widget;
	Widget exag_widget, exag_entry_form, exag_left_toggle, exag_right_toggle;
	Widget component_widget, component_entry_form;
	Widget band_number_widget, band_ratio_widget, band_entry_form;
	Widget step_value_widget, step_value_entry_form;
	Widget range_min_widget, range_max_widget;
	Widget extend_above_toggle, extend_below_toggle;
	Widget fix_maximum_toggle,fix_minimum_toggle;
	Widget value_entry_form, value_min_widget, value_max_widget;
	Widget colour_menu_widget, colour_option_widget;
	Widget render_menu_widget, render_option_widget;
	Widget *widget_address,widget,widget_parent;
}; /* Spectrum_editor_settings */

/*
Module functions
----------------
*/
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, type_menu_widget)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, type_option_widget)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, reverse_toggle_widget)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, exag_widget)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, exag_entry_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, exag_left_toggle)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, exag_right_toggle)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, component_widget)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, component_entry_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, band_number_widget)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, band_ratio_widget)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, band_entry_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, step_value_widget)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, step_value_entry_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, range_min_widget)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, range_max_widget)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, extend_above_toggle)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, extend_below_toggle)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, fix_maximum_toggle)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, fix_minimum_toggle)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, value_min_widget)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, value_max_widget)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, value_entry_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, colour_menu_widget)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, colour_option_widget)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, render_menu_widget)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor_settings, \
	Spectrum_editor_settings, render_option_widget)

static int spectrum_editor_settings_update(
	struct Spectrum_editor_settings *spectrum_editor_settings)
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
Tells CMGUI about the current values. Returns a pointer to the current point
settings.
==============================================================================*/
{
	int return_code;

	ENTER(spectrum_editor_settings_update);
	/* checking arguments */
	if (spectrum_editor_settings)
	{
		/* Now send an update to the client if requested */
		if ((spectrum_editor_settings->update_callback).procedure)
		{
			(spectrum_editor_settings->update_callback.procedure)(
				spectrum_editor_settings->widget,spectrum_editor_settings->update_callback.data,
				spectrum_editor_settings->current_settings);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_settings_update.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_settings_update */

static int spectrum_editor_settings_set_type(
	struct Spectrum_editor_settings *spectrum_editor_settings)
/*******************************************************************************
LAST MODIFIED : 13 March 1998

DESCRIPTION :
Sets the current settings type on the choose menu according to the
current settings in spectrum_editor_settings.
==============================================================================*/
{
	enum Spectrum_settings_type type, current_type;
	int return_code,num_children,i;
	Widget *child_list;

	ENTER(spectrum_editor_settings_set_type);
	if (spectrum_editor_settings &&
		spectrum_editor_settings->current_settings &&
		spectrum_editor_settings->type_menu_widget)
	{
		current_type = Spectrum_settings_get_type(
			spectrum_editor_settings->current_settings);
		/* get children of the menu so that one may be selected */
		XtVaGetValues(spectrum_editor_settings->type_menu_widget,XmNnumChildren,
			&num_children,XmNchildren,&child_list,NULL);
		return_code=0;
		for (i=0;(!return_code)&&(i<num_children);i++)
		{
			XtVaGetValues(child_list[i],XmNuserData,&type,NULL);
			if (type==current_type)
			{
				XtVaSetValues(spectrum_editor_settings->type_option_widget,
					XmNmenuHistory,child_list[i],NULL);
				return_code=1;
			}
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_settings_set_type.  Invalid type");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_settings_set_type.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_settings_set_type */

static void spectrum_editor_settings_type_menu_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 March 1998

DESCRIPTION :
Callback for the settings type.
==============================================================================*/
{
	enum Spectrum_settings_type new_spectrum_type;
	struct Spectrum_editor_settings *spectrum_editor_settings;
	struct Spectrum_settings *settings;
	Widget menu_item_widget;

	ENTER(spectrum_editor_settings_type_menu_CB);
	if (widget &&
		(spectrum_editor_settings = (struct Spectrum_editor_settings *)client_data)&&
		(settings = spectrum_editor_settings->current_settings))
	{
		/* get the widget from the call data */
		if (menu_item_widget=((XmRowColumnCallbackStruct *)call_data)->widget)
		{
			/* Get the type this menu item represents and make it current */
			XtVaGetValues(menu_item_widget, XmNuserData, &new_spectrum_type, NULL);
			if (Spectrum_settings_get_type(settings) != new_spectrum_type)
			{
				if (Spectrum_settings_set_type(settings, new_spectrum_type))
				{
					spectrum_editor_settings_update(spectrum_editor_settings);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"spectrum_editor_settings_type_menu_CB.  "
				"Could not find the activated menu item");
		}
		/* make sure the correct type type is shown in case of error */
		spectrum_editor_settings_set_settings(widget, settings);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_settings_type_menu_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* spectrum_editor_settings_type_menu_CB */

static void spectrum_editor_settings_reverse_toggle_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 July 1998

DESCRIPTION :
Callback for the settings reverse toggle button.
==============================================================================*/
{
	int reverse;
	struct Spectrum_editor_settings *spectrum_editor_settings;
	struct Spectrum_settings *settings;
	
	ENTER(spectrum_editor_settings_reverse_toggle_CB);
	USE_PARAMETER(call_data);
	if (widget &&
		(spectrum_editor_settings = (struct Spectrum_editor_settings *)client_data)&&
		(settings = spectrum_editor_settings->current_settings))
	{
		reverse = XmToggleButtonGetState(widget);
		Spectrum_settings_set_reverse_flag(settings, reverse);
		spectrum_editor_settings_update(spectrum_editor_settings);

		/* make sure the correct reverse flag is shown in case of error */
		spectrum_editor_settings_set_settings(widget, settings);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_settings_reverse_toggle_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* spectrum_editor_settings_reverse_toggle_CB */

static void spectrum_editor_settings_exag_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 July 1998

DESCRIPTION :
Callback for the exaggeration widgets.
==============================================================================*/
{
	char *text;
	float new_parameter;
	struct Spectrum_editor_settings *spectrum_editor_settings;
	struct Spectrum_settings *settings;

	ENTER(spectrum_editor_settings_exag_CB);
	USE_PARAMETER(call_data);
	/* check arguments */
	if ((spectrum_editor_settings = (struct Spectrum_editor_settings *)client_data)&&
		(settings = spectrum_editor_settings->current_settings))
	{
		new_parameter = 1.0;
		if ( spectrum_editor_settings->exag_widget )
		{
			XtVaGetValues(spectrum_editor_settings->exag_widget,
				XmNvalue,&text,NULL);
			if (text)
			{
				sscanf(text,"%f",&new_parameter);
				XtFree(text);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"spectrum_editor_settings_exag_CB.  Missing widget text");
			}
		}
		if ( widget == spectrum_editor_settings->exag_right_toggle )
		{
			new_parameter *= -1.0;
		}
		else if ( widget == spectrum_editor_settings->exag_left_toggle )
		{
			/* Do nothing */
		}
		else
		{
			/* Get the current state */
			if (XmToggleButtonGadgetGetState(spectrum_editor_settings->exag_right_toggle))
			{
				new_parameter *= -1.0;
			}

		}
		if(new_parameter !=
			Spectrum_settings_get_exaggeration(settings))
		{
			Spectrum_settings_set_exaggeration(settings,new_parameter);
			spectrum_editor_settings_update(spectrum_editor_settings);
		}
		spectrum_editor_settings_set_settings(widget, settings);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_settings_exag_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* spectrum_editor_settings_exag_CB */

static void spectrum_editor_settings_component_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 June 1999

DESCRIPTION :
Callback for the exaggeration widgets.
==============================================================================*/
{
	char *text;
	int new_component;
	struct Spectrum_editor_settings *spectrum_editor_settings;
	struct Spectrum_settings *settings;

	ENTER(spectrum_editor_settings_component_CB);
	USE_PARAMETER(call_data);
	/* check arguments */
	if ((spectrum_editor_settings = (struct Spectrum_editor_settings *)client_data)&&
		(settings = spectrum_editor_settings->current_settings))
	{
		new_component = 1;
		if ( spectrum_editor_settings->component_widget )
		{
			XtVaGetValues(spectrum_editor_settings->component_widget,
				XmNvalue,&text,NULL);
			if (text)
			{
				sscanf(text,"%d",&new_component);
				XtFree(text);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"spectrum_editor_settings_component_CB.  Missing widget text");
			}
		}
		if(new_component !=
			Spectrum_settings_get_component_number(settings))
		{
			Spectrum_settings_set_component_number(settings,new_component);
			spectrum_editor_settings_update(spectrum_editor_settings);
		}
		spectrum_editor_settings_set_settings(widget, settings);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_settings_component_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* spectrum_editor_settings_component_CB */

static void spectrum_editor_settings_bands_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 31 July 1998

DESCRIPTION :
Callback for the bands widgets.
==============================================================================*/
{
	char *text;
	int new_parameter;
	float new_ratio;
	struct Spectrum_editor_settings *spectrum_editor_settings;
	struct Spectrum_settings *settings;

	ENTER(spectrum_editor_settings_bands_CB);
	USE_PARAMETER(call_data);
	/* check arguments */
	if ((spectrum_editor_settings = (struct Spectrum_editor_settings *)client_data)&&
		(settings = spectrum_editor_settings->current_settings))
	{
		if ( widget == spectrum_editor_settings->band_number_widget )
		{
			XtVaGetValues(widget,
				XmNvalue,&text,NULL);
			if (text)
			{
				sscanf(text,"%d",&new_parameter);
				XtFree(text);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"spectrum_editor_settings_bands_CB.  Missing widget text");
			}
			if(new_parameter !=
				Spectrum_settings_get_number_of_bands(settings))
			{
				Spectrum_settings_set_number_of_bands(settings,new_parameter);
				spectrum_editor_settings_update(spectrum_editor_settings);
			}
			spectrum_editor_settings_set_settings(widget, settings);
		}
		else if ( widget == spectrum_editor_settings->band_ratio_widget )
		{
			XtVaGetValues(widget,
				XmNvalue,&text,NULL);
			if (text)
			{
				sscanf(text,"%f",&new_ratio);
				XtFree(text);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"spectrum_editor_settings_bands_CB.  Missing widget text");
			}
			new_parameter = (new_ratio * 1000.0 + 0.5);
			if(new_parameter !=
				Spectrum_settings_get_black_band_proportion(settings))
			{
				Spectrum_settings_set_black_band_proportion(settings,new_parameter);
				spectrum_editor_settings_update(spectrum_editor_settings);
			}
			spectrum_editor_settings_set_settings(widget, settings);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_settings_bands_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* spectrum_editor_settings_bands_CB */

static void spectrum_editor_settings_step_value_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 July 1998

DESCRIPTION :
Callback for the step_value widgets.
==============================================================================*/
{
	char *text;
	float new_parameter;
	struct Spectrum_editor_settings *spectrum_editor_settings;
	struct Spectrum_settings *settings;

	ENTER(spectrum_editor_settings_step_value_CB);
	USE_PARAMETER(call_data);
	/* check arguments */
	if ((spectrum_editor_settings = (struct Spectrum_editor_settings *)client_data)&&
		(settings = spectrum_editor_settings->current_settings))
	{
		if ( spectrum_editor_settings->step_value_widget )
		{
			XtVaGetValues(spectrum_editor_settings->step_value_widget,
				XmNvalue,&text,NULL);
			if (text)
			{
				sscanf(text,"%f",&new_parameter);
				XtFree(text);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"spectrum_editor_settings_step_value_CB.  Missing widget text");
			}
			if(new_parameter !=
				Spectrum_settings_get_step_value(settings))
			{
				Spectrum_settings_set_step_value(settings,new_parameter);
				spectrum_editor_settings_update(spectrum_editor_settings);
			}
			spectrum_editor_settings_set_settings(widget, settings);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_settings_step_value_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* spectrum_editor_settings_step_value_CB */

static void spectrum_editor_settings_range_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 July 1998

DESCRIPTION :
Callback for the range text widgets.
==============================================================================*/
{
	char *text;
	float new_parameter;
	int extend,fix;
	struct Spectrum_editor_settings *spectrum_editor_settings;
	struct Spectrum_settings *settings;

	ENTER(spectrum_editor_settings_range_CB);
	USE_PARAMETER(call_data);
	/* check arguments */
	if ((spectrum_editor_settings = (struct Spectrum_editor_settings *)client_data)&&
		(settings = spectrum_editor_settings->current_settings))
	{
		if (widget == spectrum_editor_settings->range_min_widget)
		{
			XtVaGetValues(widget,
				XmNvalue,&text,NULL);
			if (text)
			{
				sscanf(text,"%f",&new_parameter);
				if(new_parameter !=
					Spectrum_settings_get_range_minimum(settings))
				{
					Spectrum_settings_set_range_minimum(settings,new_parameter);
					spectrum_editor_settings_update(spectrum_editor_settings);
				}
				XtFree(text);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"spectrum_editor_settings_range_CB.  Missing widget text");
			}
		}
		else if (widget == spectrum_editor_settings->range_max_widget)
		{
			XtVaGetValues(widget,
				XmNvalue,&text,NULL);
			if (text)
			{
				sscanf(text,"%f",&new_parameter);
				if(new_parameter !=
					Spectrum_settings_get_range_maximum(settings))
				{
					Spectrum_settings_set_range_maximum(settings,new_parameter);
					spectrum_editor_settings_update(spectrum_editor_settings);
				}
				XtFree(text);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"spectrum_editor_settings_range_CB.  Missing widget text");
			}
		}
		else if (widget == spectrum_editor_settings->extend_above_toggle)
		{
			extend = XmToggleButtonGadgetGetState(widget);
			Spectrum_settings_set_extend_above_flag(settings, extend);
			spectrum_editor_settings_update(spectrum_editor_settings);
		}
		else if (widget == spectrum_editor_settings->extend_below_toggle)
		{
			extend = XmToggleButtonGadgetGetState(widget);
			Spectrum_settings_set_extend_below_flag(settings, extend);
			spectrum_editor_settings_update(spectrum_editor_settings);
		}	
		else if (widget == spectrum_editor_settings->fix_maximum_toggle)
		{
			fix = XmToggleButtonGadgetGetState(widget);
			Spectrum_settings_set_fix_maximum_flag(settings, fix);
			spectrum_editor_settings_update(spectrum_editor_settings);
		}
		else if (widget == spectrum_editor_settings->fix_minimum_toggle)
		{
			fix = XmToggleButtonGadgetGetState(widget);
			Spectrum_settings_set_fix_minimum_flag(settings, fix);
			spectrum_editor_settings_update(spectrum_editor_settings);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_settings_range_CB.  Unknown widget for event");
		}
		spectrum_editor_settings_set_settings(widget, settings);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_settings_range_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* spectrum_editor_settings_range_CB */

static void spectrum_editor_settings_colour_value_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 July 1998

DESCRIPTION :
Callback for the colour_value text widgets.
==============================================================================*/
{
	char *text;
	float new_parameter;
	struct Spectrum_editor_settings *spectrum_editor_settings;
	struct Spectrum_settings *settings;

	ENTER(spectrum_editor_settings_colour_value_CB);
	USE_PARAMETER(call_data);
	/* check arguments */
	if ((spectrum_editor_settings = (struct Spectrum_editor_settings *)client_data)&&
		(settings = spectrum_editor_settings->current_settings))
	{
		XtVaGetValues(widget,
			XmNvalue,&text,NULL);
		if (text)
		{
			sscanf(text,"%f",&new_parameter);
			if (widget == spectrum_editor_settings->value_min_widget)
			{
				if(new_parameter !=
					Spectrum_settings_get_colour_value_minimum(settings))
				{
					Spectrum_settings_set_colour_value_minimum(settings,new_parameter);
					spectrum_editor_settings_update(spectrum_editor_settings);
				}
			}
			else if (widget == spectrum_editor_settings->value_max_widget)
			{
				if(new_parameter !=
					Spectrum_settings_get_colour_value_maximum(settings))
				{
					Spectrum_settings_set_colour_value_maximum(settings,new_parameter);
					spectrum_editor_settings_update(spectrum_editor_settings);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"spectrum_editor_settings_colour_value_CB.  Unknown widget for event");
			}
			XtFree(text);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_settings_colour_value_CB.  Missing widget text");
		}
		spectrum_editor_settings_set_settings(widget, settings);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_settings_colour_value_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* spectrum_editor_settings_colour_value_CB */

static void spectrum_editor_settings_colour_menu_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 July 1998

DESCRIPTION :
Callback for the settings colour mapping type.
==============================================================================*/
{
	enum Spectrum_settings_colour_mapping new_colour_mapping;
	struct Spectrum_editor_settings *spectrum_editor_settings;
	struct Spectrum_settings *settings;
	Widget menu_item_widget;
	
	ENTER(spectrum_editor_settings_colour_menu_CB);
	if (widget &&
		(spectrum_editor_settings = (struct Spectrum_editor_settings *)client_data)&&
		(settings = spectrum_editor_settings->current_settings))
	{
		/* get the widget from the call data */
		if (menu_item_widget=((XmRowColumnCallbackStruct *)call_data)->widget)
		{
			/* Get the render this menu item represents and make it current */
			XtVaGetValues(menu_item_widget, XmNuserData, &new_colour_mapping, NULL);
			if (Spectrum_settings_get_colour_mapping(settings) != new_colour_mapping)
			{
				if (Spectrum_settings_set_colour_mapping(settings, new_colour_mapping))
				{
					spectrum_editor_settings_update(spectrum_editor_settings);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"spectrum_editor_settings_colour_menu_CB.  "
				"Could not find the activated menu item");
		}
		/* make sure the correct render render is shown in case of error */
		spectrum_editor_settings_set_settings(widget, settings);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_settings_colour_menu_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* spectrum_editor_settings_colour_menu_CB */

static void spectrum_editor_settings_render_menu_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 July 1998

DESCRIPTION :
Callback for the settings render type.
==============================================================================*/
{
	enum Spectrum_settings_render_type new_spectrum_render;
	struct Spectrum_editor_settings *spectrum_editor_settings;
	struct Spectrum_settings *settings;
	Widget menu_item_widget;
	
	ENTER(spectrum_editor_settings_render_menu_CB);
	if (widget &&
		(spectrum_editor_settings = (struct Spectrum_editor_settings *)client_data)&&
		(settings = spectrum_editor_settings->current_settings))
	{
		/* get the widget from the call data */
		if (menu_item_widget=((XmRowColumnCallbackStruct *)call_data)->widget)
		{
			/* Get the render this menu item represents and make it current */
			XtVaGetValues(menu_item_widget, XmNuserData, &new_spectrum_render, NULL);
			if (Spectrum_settings_get_render_type(settings) != new_spectrum_render)
			{
				if (Spectrum_settings_set_render_type(settings, new_spectrum_render))
				{
					spectrum_editor_settings_update(spectrum_editor_settings);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"spectrum_editor_settings_render_menu_CB.  "
				"Could not find the activated menu item");
		}
		/* make sure the correct render render is shown in case of error */
		spectrum_editor_settings_set_settings(widget, settings);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_settings_render_menu_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* spectrum_editor_settings_render_menu_CB */

static void spectrum_editor_settings_destroy_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
Callback for the spectrum_editor_settings dialog - tidies up all details - mem etc
==============================================================================*/
{
	struct Spectrum_editor_settings *spectrum_editor_settings;

	ENTER(spectrum_editor_settings_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* Get the pointer to the data for the widget */
		XtVaGetValues(widget,XmNuserData,&spectrum_editor_settings,NULL);
		if (spectrum_editor_settings)
		{
			/* destroy current settings if any */
			if (spectrum_editor_settings->current_settings)
			{
				DESTROY(Spectrum_settings)(&(spectrum_editor_settings->current_settings));
			}
			*(spectrum_editor_settings->widget_address)=(Widget)NULL;
			DEALLOCATE(spectrum_editor_settings);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_settings_destroy_CB.  "
				"Missing spectrum_editor_settings struct");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_settings_destroy_CB.  Missing widget");
	}
	LEAVE;
} /* spectrum_editor_settings_destroy_CB */

/*
Global functions
----------------
*/
Widget create_spectrum_editor_settings_widget(Widget *spectrum_editor_settings_widget,
	Widget parent,struct Spectrum_settings *settings)
/*******************************************************************************
LAST MODIFIED : 12 March 1998
				settings->settings_changed=1;

DESCRIPTION :
Creates a spectrum_editor_settings widget.
==============================================================================*/
{
	MrmType spectrum_editor_settings_dialog_class;
	struct Spectrum_editor_settings *spectrum_editor_settings=NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"spec_ed_set_destroy_CB",(XtPointer)
			spectrum_editor_settings_destroy_CB},
		{"spec_ed_set_id_type_menu",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, type_menu_widget)},
		{"spec_ed_set_id_type_option",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, type_option_widget)},
		{"spec_ed_set_type_menu_CB",(XtPointer)
			spectrum_editor_settings_type_menu_CB},
		{"spec_ed_set_linear",(XtPointer)
			SPECTRUM_LINEAR},
		{"spec_ed_set_log",(XtPointer)
			SPECTRUM_LOG},
		{"spec_ed_set_bands",(XtPointer)
			SPECTRUM_BANDED},
		{"spec_ed_set_step",(XtPointer)
			SPECTRUM_STEP},
		{"spec_ed_set_id_rev_toggle",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, reverse_toggle_widget)},
		{"spec_ed_set_rev_toggle_CB",(XtPointer)
			spectrum_editor_settings_reverse_toggle_CB},
		{"spec_ed_set_id_exag_enty",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, exag_entry_form)},
		{"spec_ed_set_id_exag_text",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, exag_widget)},
		{"spec_ed_set_id_exag_left_toggle",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, exag_left_toggle)},
		{"spec_ed_set_id_exag_rght_toggle",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, exag_right_toggle)},
		{"spec_ed_set_exag_CB",(XtPointer)spectrum_editor_settings_exag_CB},
		{"spec_ed_set_id_component_enty",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, component_entry_form)},
		{"spec_ed_set_id_component_text",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, component_widget)},
		{"spec_ed_set_component_CB",(XtPointer)
		   spectrum_editor_settings_component_CB},
		{"spec_ed_set_id_band_enty",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, band_entry_form)},
		{"spec_ed_set_id_band_number_text",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, band_number_widget)},
		{"spec_ed_set_id_band_ratio_text",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, band_ratio_widget)},
		{"spec_ed_set_bands_CB",(XtPointer)
			spectrum_editor_settings_bands_CB},
		{"spec_ed_set_id_step_enty",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, step_value_entry_form)},
		{"spec_ed_set_id_step_text",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, step_value_widget)},
		{"spec_ed_set_step_CB",(XtPointer)
			spectrum_editor_settings_step_value_CB},
		{"spec_ed_set_id_range_min_text",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, range_min_widget)},
		{"spec_ed_set_id_range_max_text",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, range_max_widget)},
		{"spec_ed_set_id_extend_above_btn",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, extend_above_toggle)},
		{"spec_ed_set_id_extend_below_btn",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, extend_below_toggle)},
		{"spec_ed_set_id_fix_max_btn",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, fix_maximum_toggle)},
		{"spec_ed_set_id_fix_min_btn",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, fix_minimum_toggle)},
		{"spec_ed_set_range_CB",(XtPointer)
			spectrum_editor_settings_range_CB},
		{"spec_ed_set_id_value_min_text",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, value_min_widget)},
		{"spec_ed_set_id_value_max_text",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, value_max_widget)},
		{"spec_ed_set_id_value_enty",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, value_entry_form)},
		{"spec_ed_set_colour_value_CB",(XtPointer)
			spectrum_editor_settings_colour_value_CB},
		{"spec_ed_set_id_colour_menu",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, colour_menu_widget)},
		{"spec_ed_set_id_colour_opt",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, colour_option_widget)},
		{"spec_ed_set_colour_menu_CB",(XtPointer)
			spectrum_editor_settings_colour_menu_CB},
		{"spec_ed_set_rainbow",(XtPointer)
			SPECTRUM_RAINBOW},
		{"spec_ed_set_red",(XtPointer)
			SPECTRUM_RED},
		{"spec_ed_set_green",(XtPointer)
			SPECTRUM_GREEN},
		{"spec_ed_set_blue",(XtPointer)
			SPECTRUM_BLUE},
		{"spec_ed_set_white_to_blue",(XtPointer)
			SPECTRUM_WHITE_TO_BLUE},
		{"spec_ed_set_white_to_red",(XtPointer)
			SPECTRUM_WHITE_TO_RED},
		{"spec_ed_set_alpha",(XtPointer)
			SPECTRUM_ALPHA},
		{"spec_ed_set_id_render_menu",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, render_menu_widget)},
		{"spec_ed_set_id_render_opt",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor_settings, render_option_widget)},
		{"spec_ed_set_render_menu_CB",(XtPointer)
			spectrum_editor_settings_render_menu_CB},
		{"spec_ed_set_amb_diff",(XtPointer)
			SPECTRUM_AMBIENT_AND_DIFFUSE},
		{"spec_ed_set_diffuse",(XtPointer)
			SPECTRUM_DIFFUSE},
		{"spec_ed_set_amb_diff",(XtPointer)
			SPECTRUM_AMBIENT_AND_DIFFUSE},
		{"spec_ed_set_ambient",(XtPointer)
			SPECTRUM_AMBIENT},
		{"spec_ed_set_diffuse",(XtPointer)
			SPECTRUM_DIFFUSE},
		{"spec_ed_set_emission",(XtPointer)
			SPECTRUM_EMISSION},
		{"spec_ed_set_specular",(XtPointer)
			SPECTRUM_SPECULAR}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"spec_ed_set_structure",(XtPointer)NULL}
	};
	Widget return_widget;

	ENTER(create_spectrum_editor_settings_widget);
	return_widget=(Widget)NULL;
	/* check arguments */
	if (spectrum_editor_settings_widget&&parent)
	{
		if (MrmOpenHierarchy_binary_string(spectrum_editor_settings_uidh,sizeof(spectrum_editor_settings_uidh),
			&spectrum_editor_settings_hierarchy,&spectrum_editor_settings_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(spectrum_editor_settings,struct Spectrum_editor_settings,1))
			{
				/* initialise the structure */
				spectrum_editor_settings->current_settings=
					(struct Spectrum_settings *)NULL;
				spectrum_editor_settings->type_menu_widget = (Widget)NULL;
				spectrum_editor_settings->type_option_widget = (Widget)NULL;
				spectrum_editor_settings->exag_widget = (Widget)NULL;
				spectrum_editor_settings->exag_entry_form = (Widget)NULL;
				spectrum_editor_settings->exag_left_toggle = (Widget)NULL;
				spectrum_editor_settings->exag_right_toggle = (Widget)NULL;
				spectrum_editor_settings->band_number_widget = (Widget)NULL;
				spectrum_editor_settings->band_ratio_widget = (Widget)NULL;
				spectrum_editor_settings->band_entry_form = (Widget)NULL;
				spectrum_editor_settings->step_value_widget = (Widget)NULL;
				spectrum_editor_settings->step_value_entry_form = (Widget)NULL;
				spectrum_editor_settings->range_min_widget = (Widget)NULL;
				spectrum_editor_settings->range_max_widget = (Widget)NULL;
				spectrum_editor_settings->extend_above_toggle = (Widget)NULL;
				spectrum_editor_settings->extend_below_toggle = (Widget)NULL;
				spectrum_editor_settings->fix_maximum_toggle = (Widget)NULL;
				spectrum_editor_settings->fix_minimum_toggle = (Widget)NULL;
				spectrum_editor_settings->value_min_widget = (Widget)NULL;
				spectrum_editor_settings->value_max_widget = (Widget)NULL;
				spectrum_editor_settings->value_entry_form = (Widget)NULL;
				spectrum_editor_settings->colour_menu_widget = (Widget)NULL;
				spectrum_editor_settings->colour_option_widget = (Widget)NULL;
				spectrum_editor_settings->render_menu_widget = (Widget)NULL;
				spectrum_editor_settings->render_option_widget = (Widget)NULL;
				spectrum_editor_settings->widget_parent=parent;
				spectrum_editor_settings->widget_address=spectrum_editor_settings_widget;
				spectrum_editor_settings->widget=(Widget)NULL;
				spectrum_editor_settings->update_callback.procedure=
					(Callback_procedure *)NULL;
				spectrum_editor_settings->update_callback.data=(void *)NULL;
				/* register the callbacks */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
					spectrum_editor_settings_hierarchy,callback_list,XtNumber(callback_list)))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)spectrum_editor_settings;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(spectrum_editor_settings_hierarchy,
						identifier_list,XtNumber(identifier_list)))
					{
						/* fetch settings editor widget */
						if (MrmSUCCESS==MrmFetchWidget(spectrum_editor_settings_hierarchy,
							"spec_ed_set_widget",
							spectrum_editor_settings->widget_parent,
							&(spectrum_editor_settings->widget),
							&spectrum_editor_settings_dialog_class))
						{
							spectrum_editor_settings_set_settings(
								spectrum_editor_settings->widget, settings);
							return_widget=spectrum_editor_settings->widget;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_spectrum_editor_settings_widget.  "
								"Could not fetch spectrum_editor_settings widget");
							DEALLOCATE(spectrum_editor_settings);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_spectrum_editor_settings_widget.  "
							"Could not register identifiers");
						DEALLOCATE(spectrum_editor_settings);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_spectrum_editor_settings_widget.  "
						"Could not register callbacks");
					DEALLOCATE(spectrum_editor_settings);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_spectrum_editor_settings_widget.  "
					"Could not allocate spectrum_editor_settings widget structure");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_spectrum_editor_settings_widget.  Could not open hierarchy");
		}
		*spectrum_editor_settings_widget=return_widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_spectrum_editor_settings_widget.  Invalid argument(s)");
	}
	LEAVE;

	return (return_widget);
} /* create_spectrum_editor_settings_widget */

int spectrum_editor_settings_set_callback(Widget spectrum_editor_settings_widget,
	struct Callback_data *new_callback)
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
Changes the callback function for the spectrum_editor_settings_widget, which will
be called when the chosen settings changes in any way.
==============================================================================*/
{
	int return_code;
	struct Spectrum_editor_settings *spectrum_editor_settings;

	ENTER(spectrum_editor_settings_set_callback);
	/* check arguments */
	if (spectrum_editor_settings_widget&&new_callback)
	{
		/* Get the pointer to the data for the choose_settings dialog */
		XtVaGetValues(spectrum_editor_settings_widget,XmNuserData,
			&spectrum_editor_settings,NULL);
		if (spectrum_editor_settings)
		{
			spectrum_editor_settings->update_callback.procedure=new_callback->procedure;
			spectrum_editor_settings->update_callback.data=new_callback->data;
			XtManageChild(spectrum_editor_settings->widget);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_settings_set_callback.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_settings_set_callback.  Missing widget");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_settings_set_callback */

int spectrum_editor_settings_set_settings(Widget spectrum_editor_settings_widget,
	struct Spectrum_settings *new_settings)
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
Changes the currently chosen settings.
==============================================================================*/
{
	char temp_string[50];
	enum Spectrum_settings_type type;
	enum Spectrum_settings_render_type button_type, render_type;
	enum Spectrum_settings_colour_mapping button_colour_mapping, colour_mapping;
	float exaggeration, step_value, band_ratio;
	int component,extend,fix,i, num_children, number_of_bands, black_band_proportion,
		return_code;
	struct Spectrum_settings *settings;
	struct Spectrum_editor_settings *spectrum_editor_settings;
	Widget *child_list;
	XtPointer pointer_var;

	ENTER(spectrum_editor_settings_set_settings);
	/* check arguments */
	if (spectrum_editor_settings_widget)
	{
		/* Get the pointer to the data for the choose_settings dialog */
		XtVaGetValues(spectrum_editor_settings_widget,XmNuserData,
			&spectrum_editor_settings,NULL);
		if (spectrum_editor_settings)
		{
			return_code=1;
			if (new_settings != spectrum_editor_settings->current_settings)
			{
				/* destroy current settings if any */
				if (spectrum_editor_settings->current_settings)
				{
					DESTROY(Spectrum_settings)
						(&(spectrum_editor_settings->current_settings));
				}
				if (new_settings)
				{
					/* make current settings a copy of new settings */
					if (spectrum_editor_settings->current_settings =
						CREATE(Spectrum_settings)())
					{
						if (COPY(Spectrum_settings)(
							spectrum_editor_settings->current_settings,new_settings))
						{
							spectrum_editor_settings_set_type(spectrum_editor_settings);

							XmToggleButtonSetState(spectrum_editor_settings->reverse_toggle_widget,
								Spectrum_settings_get_reverse_flag(new_settings),
								False );

							/* Set the colour mapping widget */
							if (spectrum_editor_settings->colour_menu_widget)
							{
								colour_mapping = Spectrum_settings_get_colour_mapping(
									spectrum_editor_settings->current_settings);
								/* get children of the menu so that one may be selected */
								XtVaGetValues(spectrum_editor_settings->colour_menu_widget,XmNnumChildren,
									&num_children,XmNchildren,&child_list,NULL);
								return_code=0;
								for (i=0;(!return_code)&&(i<num_children);i++)
								{
									XtVaGetValues(child_list[i],XmNuserData,&pointer_var,NULL);
									button_colour_mapping = (enum Spectrum_settings_colour_mapping)pointer_var;
									if (button_colour_mapping==colour_mapping)
									{
										XtVaSetValues(spectrum_editor_settings->colour_option_widget,
											XmNmenuHistory,child_list[i],NULL);
										return_code=1;
									}
								}
								if (!return_code)
								{
									display_message(ERROR_MESSAGE,
										"spectrum_editor_settings_set_settings.  Invalid colour mapping");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"spectrum_editor_settings_set_settings.  Colour menu widget not set");
								return_code=0;
							}
							
							
							/* Set the render type widget */
							if (spectrum_editor_settings->render_menu_widget)
							{
								render_type = Spectrum_settings_get_render_type(
									spectrum_editor_settings->current_settings);
								/* get children of the menu so that one may be selected */
								XtVaGetValues(spectrum_editor_settings->render_menu_widget,XmNnumChildren,
									&num_children,XmNchildren,&child_list,NULL);
								return_code=0;
								for (i=0;(!return_code)&&(i<num_children);i++)
								{
									XtVaGetValues(child_list[i],XmNuserData,&pointer_var,NULL);
									button_type = (enum Spectrum_settings_render_type)pointer_var;
									if (button_type==render_type)
									{
										XtVaSetValues(spectrum_editor_settings->render_option_widget,
											XmNmenuHistory,child_list[i],NULL);
										return_code=1;
									}
								}
								if (!return_code)
								{
									display_message(ERROR_MESSAGE,
										"spectrum_editor_settings_set_settings.  Invalid render type");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"spectrum_editor_settings_set_settings.  Render menu widget not set");
								return_code=0;
							}
							
						}
						else
						{
							DESTROY(Spectrum_settings)
								(&(spectrum_editor_settings->current_settings));
							new_settings=(struct Spectrum_settings *)NULL;
							return_code=0;
						}
					}
					else
					{
						new_settings=(struct Spectrum_settings *)NULL;
						return_code=0;
					}
				}
				if ((struct Spectrum_settings *)NULL==new_settings)
				{
					XtUnmanageChild(spectrum_editor_settings->widget);
				}
				else
				{
					XtManageChild(spectrum_editor_settings->widget);
				}
			}
			/* Ensure the correct widgets are visible */
			if (settings = spectrum_editor_settings->current_settings )
			{
				type = Spectrum_settings_get_type(settings);
				colour_mapping = Spectrum_settings_get_colour_mapping(
					settings);
				if (spectrum_editor_settings->value_entry_form)
				{
					if (SPECTRUM_BANDED == colour_mapping
						 || SPECTRUM_STEP == colour_mapping)
					{
						XtSetSensitive(spectrum_editor_settings->value_entry_form,
							False );
					}
					else
					{
						XtSetSensitive(spectrum_editor_settings->value_entry_form,
							True );
					}
				}
				if (spectrum_editor_settings->render_option_widget)
				{
					if (SPECTRUM_ALPHA == colour_mapping || SPECTRUM_BANDED == colour_mapping
						 || SPECTRUM_STEP == colour_mapping)
					{
						XtSetSensitive(spectrum_editor_settings->render_option_widget,
							False );
					}
					else
					{
						XtSetSensitive(spectrum_editor_settings->render_option_widget,
							True );
					}
				}
				if (spectrum_editor_settings->exag_widget)
				{
					exaggeration = Spectrum_settings_get_exaggeration(settings);
					sprintf(temp_string,"%10g",fabs(exaggeration));
					XtVaSetValues(spectrum_editor_settings->exag_widget,
						XmNvalue,temp_string,
						NULL);
					if (exaggeration >= 0.0)
					{
						XmToggleButtonGadgetSetState(spectrum_editor_settings->exag_left_toggle, True, False);
						XmToggleButtonGadgetSetState(spectrum_editor_settings->exag_right_toggle, False, False);
					}
					else
					{
						XmToggleButtonGadgetSetState(spectrum_editor_settings->exag_left_toggle, False, False);
						XmToggleButtonGadgetSetState(spectrum_editor_settings->exag_right_toggle, True, False);
					}
				}
				if (spectrum_editor_settings->exag_entry_form)
				{
					if (SPECTRUM_LOG == type)
					{
						XtSetSensitive(spectrum_editor_settings->exag_entry_form,
							True );
					}
					else
					{
						XtSetSensitive(spectrum_editor_settings->exag_entry_form,
							False );
					}
				}
				if (spectrum_editor_settings->component_widget)
				{
					component = Spectrum_settings_get_component_number(settings);
					sprintf(temp_string,"%d",component);
					XtVaSetValues(spectrum_editor_settings->component_widget,
						XmNvalue,temp_string,
						NULL);
				}
				if (spectrum_editor_settings->band_number_widget)
				{
					number_of_bands = Spectrum_settings_get_number_of_bands(settings);
					sprintf(temp_string,"%10d",number_of_bands);
					XtVaSetValues(spectrum_editor_settings->band_number_widget,
						XmNvalue,temp_string,
						NULL);
				}
				if (spectrum_editor_settings->band_ratio_widget)
				{
					black_band_proportion = Spectrum_settings_get_black_band_proportion(settings);
					band_ratio = (float)(black_band_proportion) / 1000.0;
					sprintf(temp_string,"%10g",band_ratio);
					XtVaSetValues(spectrum_editor_settings->band_ratio_widget,
						XmNvalue,temp_string,
						NULL);
				}
				if (spectrum_editor_settings->band_entry_form)
				{
					if (SPECTRUM_BANDED == colour_mapping)
					{
						XtSetSensitive(spectrum_editor_settings->band_entry_form,
							True );
					}
					else
					{
						XtSetSensitive(spectrum_editor_settings->band_entry_form,
							False );
					}
				}
				if (spectrum_editor_settings->step_value_widget)
				{
					step_value = Spectrum_settings_get_step_value(settings);
					sprintf(temp_string,"%10g",step_value);
					XtVaSetValues(spectrum_editor_settings->step_value_widget,
						XmNvalue,temp_string,
						NULL);
				}
				if (spectrum_editor_settings->step_value_entry_form)
				{
					if (SPECTRUM_STEP == colour_mapping)
					{
						XtSetSensitive(spectrum_editor_settings->step_value_entry_form,
							True );
					}
					else
					{
						XtSetSensitive(spectrum_editor_settings->step_value_entry_form,
							False );
					}
				}
				if (spectrum_editor_settings->range_min_widget)
				{
					sprintf(temp_string,"%10g",
						Spectrum_settings_get_range_minimum(settings));
					XtVaSetValues(spectrum_editor_settings->range_min_widget,
						XmNvalue,temp_string,
						NULL);
				}
				if (spectrum_editor_settings->range_max_widget)
				{
					sprintf(temp_string,"%10g",
						Spectrum_settings_get_range_maximum(settings));
					XtVaSetValues(spectrum_editor_settings->range_max_widget,
						XmNvalue,temp_string,
						NULL);
				}
				if (spectrum_editor_settings->extend_above_toggle)
				{
					extend = Spectrum_settings_get_extend_above_flag(settings);
					XmToggleButtonGadgetSetState(spectrum_editor_settings->extend_above_toggle,
						extend, False);
				}
				if (spectrum_editor_settings->extend_below_toggle)
				{
					extend = Spectrum_settings_get_extend_below_flag(settings);
					XmToggleButtonGadgetSetState(spectrum_editor_settings->extend_below_toggle,
						extend, False);
				}
				if (spectrum_editor_settings->fix_maximum_toggle)
				{
					fix = Spectrum_settings_get_fix_maximum_flag(settings);
					XmToggleButtonGadgetSetState(spectrum_editor_settings->fix_maximum_toggle,
						fix, False);
					/* can't change the range when it's fixed*/
					if(fix)
					{
						XtSetSensitive(spectrum_editor_settings->range_max_widget,
							False );
					}
					else
					{
						XtSetSensitive(spectrum_editor_settings->range_max_widget,
							True );
					}
				}
				if (spectrum_editor_settings->fix_minimum_toggle)
				{
					fix = Spectrum_settings_get_fix_minimum_flag(settings);
					XmToggleButtonGadgetSetState(spectrum_editor_settings->fix_minimum_toggle,
						fix, False);
					/* can't change the range when it's fixed*/
					if(fix)
					{
						XtSetSensitive(spectrum_editor_settings->range_min_widget,
							False );
					}
					else
					{
						XtSetSensitive(spectrum_editor_settings->range_min_widget,
							True );
					}
				}
				if (spectrum_editor_settings->value_min_widget)
				{
					sprintf(temp_string,"%10g",
						Spectrum_settings_get_colour_value_minimum(settings));
					XtVaSetValues(spectrum_editor_settings->value_min_widget,
						XmNvalue,temp_string,
						NULL);
				}
				if (spectrum_editor_settings->value_max_widget)
				{
					sprintf(temp_string,"%10g",
						Spectrum_settings_get_colour_value_maximum(settings));
					XtVaSetValues(spectrum_editor_settings->value_max_widget,
						XmNvalue,temp_string,
						NULL);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_settings_set_settings.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_settings_set_settings.  Missing widget");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_settings_set_settings */

struct Callback_data *spectrum_editor_settings_get_callback(
	Widget spectrum_editor_settings_widget)
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
Returns a pointer to the update_callback item of the
spectrum_editor_settings_widget.
==============================================================================*/
{
	struct Callback_data *return_address;
	struct Spectrum_editor_settings *spectrum_editor_settings;

	ENTER(spectrum_editor_settings_get_callback);
	/* check arguments */
	if (spectrum_editor_settings_widget)
	{
		/* Get the pointer to the data for the choose_settings dialog */
		XtVaGetValues(spectrum_editor_settings_widget,XmNuserData,
			&spectrum_editor_settings,NULL);
		if (spectrum_editor_settings)
		{
			return_address=&(spectrum_editor_settings->update_callback);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_settings_get_callback.  Missing widget data");
			return_address=(struct Callback_data *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_settings_get_callback.  Missing widget");
		return_address=(struct Callback_data *)NULL;
	}
	LEAVE;

	return (return_address);
} /* spectrum_editor_settings_get_callback */

struct Spectrum_settings *spectrum_editor_settings_get_settings(
	Widget spectrum_editor_settings_widget)
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
Returns the currently chosen settings.
==============================================================================*/
{
	struct Spectrum_settings *return_address;
	struct Spectrum_editor_settings *spectrum_editor_settings;

	ENTER(spectrum_editor_settings_get_settings);
	/* check arguments */
	if (spectrum_editor_settings_widget)
	{
		/* Get the pointer to the data for the choose_settings dialog */
		XtVaGetValues(spectrum_editor_settings_widget,XmNuserData,
			&spectrum_editor_settings,NULL);
		if (spectrum_editor_settings)
		{
			return_address=spectrum_editor_settings->current_settings;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_settings_get_settings.  Missing widget data");
			return_address=(struct Spectrum_settings *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_settings_get_settings.  Missing widget");
		return_address=(struct Spectrum_settings *)NULL;
	}
	LEAVE;

	return (return_address);
} /* spectrum_editor_settings_get_settings */
