/*******************************************************************************
FILE : control_curve_editor.c

LAST MODIFIED : 16 June 2000

DESCRIPTION :
Provides the widgets to modify Control_curve structures.
==============================================================================*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <Xm/ToggleBG.h>
#include "general/debug.h"
#include "choose/choose_enumerator.h"
#include "choose/choose_field_component.h"
#include "curve/control_curve.h"
#include "curve/control_curve_editor.h"
#include "curve/control_curve_editor.uidh"
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module types
------------
*/
typedef struct Control_curve_drawing_information
	Control_curve_drawing_information_settings;

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int curve_editor_hierarchy_open=0;
static MrmHierarchy curve_editor_hierarchy;
#endif /* defined (MOTIF) */

struct Control_curve_editor
/*******************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
Contains all the information needed for the control_curve_editor widget.
==============================================================================*/
{
	int current_component_no,current_element_no,current_node_no,
		components_in_view,max_components_in_view,first_component_in_view,
		range_component_no,comp_box_width,y_axis_width,comp_box_height,
		x_axis_height,cursor_displayed;
	int snap_parameter,snap_value;
	/* following are ranges of component(s) and parameter visible in window: */
	int max_number_of_components; /* size of view_comp_min,view_comp_max arrays */
	FE_value cursor, *view_comp_min,*view_comp_max,view_parameter_min,view_parameter_max;
	struct Control_curve *edit_curve;
	struct User_interface *user_interface;
	struct Callback_data update_callback;
	struct Control_curve_drawing_information *curve_drawing_information;
	Widget basis_type_form,basis_type_widget,num_components_text,drawing_area,
		component_form,component_widget,min_comp_text,max_comp_text,
		extend_mode_form,extend_mode_widget, comp_grid_text,
		parameter_grid_text,full_range_button,min_parameter_text,max_parameter_text,
		comps_shown_text,snap_value_button,snap_parameter_button,
		up_button,down_button;
	Widget *widget_address,widget,widget_parent;
}; /* control_curve_editor_struct */

struct Control_curve_drawing_information
/*******************************************************************************
LAST MODIFIED : 26 September 1997

DESCRIPTION :
Information needed for drawing a curve.  Windowing system dependent
==============================================================================*/
{
	int curve_segments,horizontal_axis_height,vertical_axis_width,tick_length,
		control_point_size,pick_distance,min_grid_spacing,
		min_horizontal_label_spacing,min_vertical_label_spacing;
	/* extra zones added to components & parameter ranges to make borders: */
	FE_value extra_component_range,extra_parameter_range;
	Pixel axis_colour,background_colour,control_point_colour,drag_colour,
		function_colour,major_grid_colour,minor_grid_colour,separator_colour,
		cursor_colour;
	GC axis_gc,control_point_gc,drag_gc,function_gc,major_grid_gc,minor_grid_gc,
		separator_gc, cursor_gc;
	struct User_interface *user_interface;
	XFontStruct *font;
}; /* struct Control_curve_drawing_information */

/*
Module functions
----------------
*/
struct Control_curve_drawing_information *create_Control_curve_drawing_information(
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 2 October 1997

DESCRIPTION :
Creates a struct Control_curve_drawing_information, filling it with default colour,
font and graphics_context information read in from the Cmgui defaults file.
==============================================================================*/
{
	Display *display;
	int depth;
	Pixmap depth_screen_drawable;
	struct Control_curve_drawing_information *curve_drawing_information;
#define XmNcurveEditorAxisColour "curveEditorAxisColour"
#define XmCControl_curveEditorAxisColour "Control_curveEditorAxisColour"
#define XmNcurveEditorBackgroundColour "curveEditorBackgroundColour"
#define XmCControl_curveEditorBackgroundColour "Control_curveEditorBackgroundColour"
#define XmNcurveEditorControlPointColour "curveEditorControlPointColour"
#define XmCControl_curveEditorControlPointColour "Control_curveEditorControlPointColour"
#define XmNcurveEditorDragColour "curveEditorDragColour"
#define XmCControl_curveEditorDragColour "Control_curveEditorDragColour"
#define XmNcurveEditorFunctionColour "curveEditorFunctionColour"
#define XmCControl_curveEditorFunctionColour "Control_curveEditorFunctionColour"
#define XmNcurveEditorMajorGridColour "curveEditorMajorGridColour"
#define XmCControl_curveEditorMajorGridColour "Control_curveEditorMajorGridColour"
#define XmNcurveEditorMinorGridColour "curveEditorMinorGridColour"
#define XmCControl_curveEditorMinorGridColour "Control_curveEditorMinorGridColour"
#define XmNcurveEditorSeparatorColour "curveEditorSeparatorColour"
#define XmCControl_curveEditorSeparatorColour "Control_curveEditorSeparatorColour"
#define XmNcurveEditorCursorColour "curveEditorCursorColour"
#define XmCControl_curveEditorCursorColour "Control_curveEditorCursorColour"
#define XmNcurveEditorCurveSegments "curveEditorCurveSegments"
#define XmCControl_curveEditorCurveSegments "Control_curveEditorCurveSegments"
#define XmNcurveEditorHorizontalAxisHeight \
	"curveEditorHorizontalAxisHeight"
#define XmCControl_curveEditorHorizontalAxisHeight \
	"Control_curveEditorHorizontalAxisHeight"
#define XmNcurveEditorVerticalAxisWidth "curveEditorVerticalAxisWidth"
#define XmCControl_curveEditorVerticalAxisWidth "Control_curveEditorVerticalAxisWidth"
#define XmNcurveEditorTickLength "curveEditorTickLength"
#define XmCControl_curveEditorTickLength "Control_curveEditorTickLength"
#define XmNcurveEditorExtraComponentRange "curveEditorExtraComponentRange"
#define XmCControl_curveEditorExtraComponentRange "Control_curveEditorExtraComponentRange"
#define XmNcurveEditorExtraParameterRange "curveEditorExtraParameterRange"
#define XmCControl_curveEditorExtraParameterRange "Control_curveEditorExtraParameterRange"
#define XmNcurveEditorControlPointSize "curveEditorControlPointSize"
#define XmCControl_curveEditorControlPointSize "Control_curveEditorControlPointSize"
#define XmNcurveEditorPickDistance "curveEditorPickDistance"
#define XmCControl_curveEditorPickDistance "Control_curveEditorPickDistance"
#define	XmNcurveEditorMinGridSpacing "curveEditorMinGridSpacing"
#define	XmCControl_curveEditorMinGridSpacing "Control_curveEditorMinGridSpacing"
#define	XmNcurveEditorMinHorizontalLabelSpacing \
	"curveEditorMinHorizontalLabelSpacing"
#define	XmCControl_curveEditorMinHorizontalLabelSpacing \
	"Control_curveEditorMinHorizontalLabelSpacing"
#define	XmNcurveEditorMinVerticalLabelSpacing \
	"curveEditorMinVerticalLabelSpacing"
#define	XmCControl_curveEditorMinVerticalLabelSpacing \
	"Control_curveEditorMinVerticalLabelSpacing"
	static XtResource resources[]=
	{
		{
			XmNcurveEditorAxisColour,
			XmCControl_curveEditorAxisColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Control_curve_drawing_information_settings,axis_colour),
			XmRString,
			"white"
		},
		{
			XmNcurveEditorBackgroundColour,
			XmCControl_curveEditorBackgroundColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Control_curve_drawing_information_settings,background_colour),
			XmRString,
			"midnight blue"
		},
		{
			XmNcurveEditorControlPointColour,
			XmCControl_curveEditorControlPointColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Control_curve_drawing_information_settings,control_point_colour),
			XmRString,
			"white"
		},
		{
			XmNcurveEditorDragColour,
			XmCControl_curveEditorDragColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Control_curve_drawing_information_settings,drag_colour),
			XmRString,
			"green"
		},
		{
			XmNcurveEditorFunctionColour,
			XmCControl_curveEditorFunctionColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Control_curve_drawing_information_settings,function_colour),
			XmRString,
			"yellow"
		},
		{
			XmNcurveEditorMajorGridColour,
			XmCControl_curveEditorMajorGridColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Control_curve_drawing_information_settings,major_grid_colour),
			XmRString,
			"grey60"
		},
		{
			XmNcurveEditorMinorGridColour,
			XmCControl_curveEditorMinorGridColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Control_curve_drawing_information_settings,minor_grid_colour),
			XmRString,
			"grey40"
		},
		{
			XmNcurveEditorSeparatorColour,
			XmCControl_curveEditorSeparatorColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Control_curve_drawing_information_settings,separator_colour),
			XmRString,
			"orange red"
		},
		{
			XmNcurveEditorCursorColour,
			XmCControl_curveEditorCursorColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Control_curve_drawing_information_settings,cursor_colour),
			XmRString,
			"yellow"
		},
		{
			XmNcurveEditorCurveSegments,
			XmCControl_curveEditorCurveSegments,
			XmRInt,
			sizeof(int),
			XtOffsetOf(Control_curve_drawing_information_settings,curve_segments),
			XmRString,
			"24"
		},
		{
			XmNcurveEditorHorizontalAxisHeight,
			XmCControl_curveEditorHorizontalAxisHeight,
			XmRInt,
			sizeof(int),
			XtOffsetOf(Control_curve_drawing_information_settings,horizontal_axis_height),
			XmRString,
			"32"
		},
		{
			XmNcurveEditorVerticalAxisWidth,
			XmCControl_curveEditorVerticalAxisWidth,
			XmRInt,
			sizeof(int),
			XtOffsetOf(Control_curve_drawing_information_settings,vertical_axis_width),
			XmRString,
			"40"
		},
		{
			XmNcurveEditorTickLength,
			XmCControl_curveEditorTickLength,
			XmRInt,
			sizeof(int),
			XtOffsetOf(Control_curve_drawing_information_settings,tick_length),
			XmRString,
			"8"
		},
		{
			XmNcurveEditorExtraComponentRange,
			XmCControl_curveEditorExtraComponentRange,
			XmRFloat,
			sizeof(float),
			XtOffsetOf(Control_curve_drawing_information_settings,extra_component_range),
			XmRString,
			"0.1"
		},
		{
			XmNcurveEditorExtraParameterRange,
			XmCControl_curveEditorExtraParameterRange,
			XmRFloat,
			sizeof(float),
			XtOffsetOf(Control_curve_drawing_information_settings,extra_parameter_range),
			XmRString,
			"0.05"
		},
		{
			XmNcurveEditorControlPointSize,
			XmCControl_curveEditorControlPointSize,
			XmRInt,
			sizeof(int),
			XtOffsetOf(Control_curve_drawing_information_settings,control_point_size),
			XmRString,
			"5"
		},
		{
			XmNcurveEditorPickDistance,
			XmCControl_curveEditorPickDistance,
			XmRInt,
			sizeof(int),
			XtOffsetOf(Control_curve_drawing_information_settings,pick_distance),
			XmRString,
			"7"
		},
		{
			XmNcurveEditorMinGridSpacing,
			XmCControl_curveEditorMinGridSpacing,
			XmRInt,
			sizeof(int),
			XtOffsetOf(Control_curve_drawing_information_settings,min_grid_spacing),
			XmRString,
			"10"
		},
		{
			XmNcurveEditorMinHorizontalLabelSpacing,
			XmCControl_curveEditorMinHorizontalLabelSpacing,
			XmRInt,
			sizeof(int),
			XtOffsetOf(Control_curve_drawing_information_settings,
				min_horizontal_label_spacing),
			XmRString,
			"60"
		},
		{
			XmNcurveEditorMinVerticalLabelSpacing,
			XmCControl_curveEditorMinVerticalLabelSpacing,
			XmRInt,
			sizeof(int),
			XtOffsetOf(Control_curve_drawing_information_settings,
				min_vertical_label_spacing),
			XmRString,
			"30"
		}
	};
	unsigned long mask;
	XGCValues values;

	ENTER(create_Control_curve_drawing_information);
	/* check arguments */
	if (user_interface)
	{
		if (ALLOCATE(curve_drawing_information,
			struct Control_curve_drawing_information,1))
		{
			curve_drawing_information->user_interface=user_interface;
			curve_drawing_information->font=user_interface->normal_font;
			/* retrieve_settings */
			XtVaGetApplicationResources(user_interface->application_shell,
				curve_drawing_information,resources,XtNumber(resources),NULL);
			/* create the graphics contexts */
			display=user_interface->display;
			/* the drawable has to have the correct depth and screen */
			XtVaGetValues(user_interface->application_shell,XmNdepth,&depth,NULL);
			depth_screen_drawable=XCreatePixmap(user_interface->display,
				XRootWindow(user_interface->display,
				XDefaultScreen(user_interface->display)),1,1,depth);
			mask=GCLineStyle|GCBackground|GCFont|GCForeground|GCFunction;
			values.font=user_interface->normal_font->fid;
			values.line_style=LineSolid;
			values.background=curve_drawing_information->background_colour;
			values.foreground=curve_drawing_information->axis_colour;
			values.function=GXcopy;
			curve_drawing_information->axis_gc=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=curve_drawing_information->control_point_colour;
			values.function=GXcopy;
			curve_drawing_information->control_point_gc=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=curve_drawing_information->drag_colour^
				curve_drawing_information->background_colour;
			values.function=GXxor;
			curve_drawing_information->drag_gc=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=curve_drawing_information->function_colour;
			values.function=GXcopy;
			curve_drawing_information->function_gc=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=curve_drawing_information->major_grid_colour;
			values.function=GXcopy;
			curve_drawing_information->major_grid_gc=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=curve_drawing_information->minor_grid_colour;
			values.function=GXcopy;
			curve_drawing_information->minor_grid_gc=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=curve_drawing_information->separator_colour;
			values.function=GXcopy;
			curve_drawing_information->separator_gc=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=curve_drawing_information->cursor_colour;
			values.function=GXcopy;
			curve_drawing_information->cursor_gc=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			XFreePixmap(user_interface->display,depth_screen_drawable);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_curve_drawing_information.  Could not allocate memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_curve_drawing_information.  Missing user_interface");
		curve_drawing_information=(struct Control_curve_drawing_information *)NULL;
	}
	LEAVE;

	return (curve_drawing_information);
} /* create_Control_curve_drawing_information */

int destroy_Control_curve_drawing_information(
	struct Control_curve_drawing_information **curve_drawing_information_address)
/*******************************************************************************
LAST MODIFIED : 26 September 1997

DESCRIPTION :
Frees memory used by the struct Control_curve_drawing_information.
==============================================================================*/
{
	Display *display;
	int return_code;
	struct Control_curve_drawing_information *curve_drawing_information;

	ENTER(destroy_Control_curve_drawing_information);
	if (curve_drawing_information_address&&
		(curve_drawing_information= *curve_drawing_information_address)&&
		(curve_drawing_information->user_interface))
	{
		display=curve_drawing_information->user_interface->display;
		XFreeGC(display,curve_drawing_information->axis_gc);
		XFreeGC(display,curve_drawing_information->control_point_gc);
		XFreeGC(display,curve_drawing_information->drag_gc);
		XFreeGC(display,curve_drawing_information->function_gc);
		XFreeGC(display,curve_drawing_information->major_grid_gc);
		XFreeGC(display,curve_drawing_information->minor_grid_gc);
		XFreeGC(display,curve_drawing_information->separator_gc);
		XFreeGC(display,curve_drawing_information->cursor_gc);
		DEALLOCATE(*curve_drawing_information_address);
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* destroy_Control_curve_drawing_information */

static int control_curve_editor_update(
	struct Control_curve_editor *curve_editor)
/*******************************************************************************
LAST MODIFIED : 25 September 1997

DESCRIPTION :
Tells CMGUI about the current values, when they are changed.
==============================================================================*/
{
	int return_code;

	ENTER(control_curve_editor_update);
	/* checking arguments */
	if (curve_editor)
	{
		/* Now send an update to the client if requested */
		if ((curve_editor->update_callback).procedure)
		{
			(curve_editor->update_callback.procedure)(
				curve_editor->widget,curve_editor->update_callback.data,
				curve_editor->edit_curve);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_update.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* control_curve_editor_update */

DECLARE_DIALOG_IDENTIFY_FUNCTION(control_curve_editor, \
	Control_curve_editor,drawing_area)
DECLARE_DIALOG_IDENTIFY_FUNCTION(control_curve_editor, \
	Control_curve_editor,basis_type_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(control_curve_editor, \
	Control_curve_editor,num_components_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(control_curve_editor, \
	Control_curve_editor,component_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(control_curve_editor, \
	Control_curve_editor,min_comp_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(control_curve_editor, \
	Control_curve_editor,max_comp_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(control_curve_editor, \
	Control_curve_editor,extend_mode_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(control_curve_editor, \
	Control_curve_editor,comp_grid_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(control_curve_editor, \
	Control_curve_editor,parameter_grid_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(control_curve_editor, \
	Control_curve_editor,full_range_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(control_curve_editor, \
	Control_curve_editor,min_parameter_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(control_curve_editor, \
	Control_curve_editor,max_parameter_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(control_curve_editor, \
	Control_curve_editor,comps_shown_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(control_curve_editor, \
	Control_curve_editor,snap_value_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(control_curve_editor, \
	Control_curve_editor,snap_parameter_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(control_curve_editor, \
	Control_curve_editor,up_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(control_curve_editor, \
	Control_curve_editor,down_button)

static void control_curve_editor_destroy_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 21 October 1997

DESCRIPTION :
Callback for the curve_editor dialog - tidies up all details - mem etc
==============================================================================*/
{
	struct Control_curve_editor *curve_editor;

	ENTER(control_curve_editor_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* Get the pointer to the data for the widget */
		XtVaGetValues(widget,XmNuserData,&curve_editor,NULL);
		if (curve_editor)
		{
			/* destroy edit_curve */
			if (curve_editor->edit_curve)
			{
				DESTROY(Control_curve)(&(curve_editor->edit_curve));
			}
			if (curve_editor->view_comp_min)
			{
				DEALLOCATE(curve_editor->view_comp_min);
			}
			if (curve_editor->view_comp_max)
			{
				DEALLOCATE(curve_editor->view_comp_max);
			}
			/* free curve drawing information */
			if (curve_editor->curve_drawing_information)
			{
				destroy_Control_curve_drawing_information(
					&(curve_editor->curve_drawing_information));
			}
			*(curve_editor->widget_address)=(Widget)NULL;
			DEALLOCATE(curve_editor);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_destroy_CB.  Missing widget data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_destroy_CB.  Missing widget");
	}
	LEAVE;
} /* control_curve_editor_destroy_CB */

static int check_range(FE_value *high_value,FE_value *low_value)
/*******************************************************************************
LAST MODIFIED : 30 September 1997

DESCRIPTION :
Adjusts low_value and/or high_value so that they specify a non-zero range.
???RC keep this here?
==============================================================================*/
{
	FE_value temp;
	int return_code;

	ENTER(check_range);
	if (high_value&&low_value)
	{
		if (*high_value < *low_value)
		{
			/* swap high and low values */
			temp=*high_value;
			*high_value=*low_value;
			*low_value=temp;
		}
		else
		{
			if (*high_value == *low_value)
			{
				if (0.0 < *high_value)
				{
					*low_value=0.0;
					*high_value *= 2.0;
				}
				else
				{
					if (0.0 > *high_value)
					{
						*high_value=0.0;
						*low_value *= 2.0;
					}
					else
					{
						*high_value=1.0;
					}
				}
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"check_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* check_range */

static int control_curve_editor_view_full_range(
	struct Control_curve_editor *curve_editor)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Sets the visible range of parameter and components to the range of parameters and
components in the curve being edited.
???RC Currently does not force any redraws.
==============================================================================*/
{
	char temp_string[30];
	FE_value *view_comp_min,*view_comp_max,min,max,extra_range;
	int return_code,comp_no,number_of_components;
	struct Control_curve *curve;

	ENTER(control_curve_editor_view_full_range);
	if (curve_editor&&(curve=curve_editor->edit_curve))
	{
		number_of_components=Control_curve_get_number_of_components(curve);
		if (!(return_code=(number_of_components <
			curve_editor->max_number_of_components)))
		{
			if ((REALLOCATE(view_comp_min,curve_editor->view_comp_min,FE_value,
				number_of_components))&&(REALLOCATE(view_comp_max,
				curve_editor->view_comp_max,FE_value,number_of_components)))
			{
				curve_editor->max_number_of_components=number_of_components;
				curve_editor->view_comp_min=view_comp_min;
				curve_editor->view_comp_max=view_comp_max;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"control_curve_editor_view_full_range.  Could not allocate arrays");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* get range of component values and parameters in curve, add extra range */
			for (comp_no=0;comp_no<number_of_components;comp_no++)
			{
				Control_curve_get_edit_component_range(curve,comp_no,&min,&max);
				check_range(&max,&min);
				extra_range=(max-min)*
					curve_editor->curve_drawing_information->extra_component_range;
				curve_editor->view_comp_max[comp_no]=max+extra_range;
				curve_editor->view_comp_min[comp_no]=min-extra_range;
			}
			if (0<Control_curve_get_number_of_elements(curve))
			{
				Control_curve_get_parameter_range(curve,&min,&max);
			}
			else
			{
				max=min=0.0;
			}
			check_range(&max,&min);
			extra_range=(max-min)*
				curve_editor->curve_drawing_information->extra_parameter_range;
			curve_editor->view_parameter_max=max+extra_range;
			curve_editor->view_parameter_min=min-extra_range;
			/* redisplay the parameter range in the widgets */
			sprintf(temp_string,"%g",curve_editor->view_parameter_min);
			XtVaSetValues(curve_editor->min_parameter_text,XmNvalue,temp_string,NULL);
			sprintf(temp_string,"%g",curve_editor->view_parameter_max);
			XtVaSetValues(curve_editor->max_parameter_text,XmNvalue,temp_string,NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_view_full_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* control_curve_editor_view_full_range */

static int get_drawing_scale_factors_x(
	struct Control_curve_editor *curve_editor,
	int *xmin,FE_value *xscale,FE_value *xstartvalue)
/*******************************************************************************
LAST MODIFIED : 7 October 1997

DESCRIPTION :
Calculates scale factors for converting parameter values to pixels in the
x-direction. The conversion formula to be used is:
	screen x = xmin + xscale * (parameter - xstartvalue)
???RC later support having components along the x-axis as well, eg. x-y graphs.
==============================================================================*/
{
	int return_code;

	ENTER(get_drawing_scale_factors_x);
	if (curve_editor&&curve_editor->edit_curve&&xmin&&xscale&&xstartvalue)
	{
		*xscale=(FE_value)curve_editor->comp_box_width/
			(curve_editor->view_parameter_max-curve_editor->view_parameter_min);
		*xmin=curve_editor->y_axis_width-1;
		/* note: -0.5/xscale term added to produce rounding */
		*xstartvalue=curve_editor->view_parameter_min-0.5/(*xscale);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_drawing_scale_factors_x.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_drawing_scale_factors_x */

static int get_drawing_scale_factors_y(
	struct Control_curve_editor *curve_editor,int component_no,
	int *ymin,FE_value *yscale,FE_value *ystartvalue)
/*******************************************************************************
LAST MODIFIED : 7 October 1997

DESCRIPTION :
Calculates scale factors for converting component values to pixels in the
y-direction. The conversion formula to be used is:
	screen y = ymin + yscale * (value - ystartvalue)
Note that yscale will be negative since y coordinates increase downwards.
==============================================================================*/
{
	int return_code;

	ENTER(get_drawing_scale_factors_y);
	if (curve_editor&&curve_editor->edit_curve&&ymin&&yscale&&ystartvalue&&
		(0 <= component_no)&&(component_no<
		Control_curve_get_number_of_components(curve_editor->edit_curve)))
	{
		*yscale=(FE_value)(-curve_editor->comp_box_height)/
			(curve_editor->view_comp_max[component_no]-
			curve_editor->view_comp_min[component_no]);
		*ymin=curve_editor->comp_box_height*
			(component_no+1-curve_editor->first_component_in_view);
		/* note: -0.5/yscale term added to produce rounding */
		*ystartvalue=curve_editor->view_comp_min[component_no]-0.5/(*yscale);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_drawing_scale_factors_y.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_drawing_scale_factors_y */

static int control_curve_editor_drawing_layout_change(
	struct Control_curve_editor *curve_editor)
/*******************************************************************************
LAST MODIFIED : 16 June 2000

DESCRIPTION :
Calculates the size of the various parts of the drawing_area based on the
window dimensions, number of components in view and default drawing information.
Call this routine after the drawing area is opened or resized or the number of
components being displayed is changed.
==============================================================================*/
{
	Dimension winwidth,winheight;
	int number_of_components,return_code;

	ENTER(control_curve_editor_drawing_layout_change);
	if (curve_editor&&curve_editor->drawing_area&&
		curve_editor->curve_drawing_information)
	{
		if (curve_editor->edit_curve)
		{
			number_of_components=Control_curve_get_number_of_components(
				curve_editor->edit_curve);
			curve_editor->components_in_view=number_of_components;
			if (curve_editor->components_in_view > curve_editor->max_components_in_view)
			{
				curve_editor->components_in_view=curve_editor->max_components_in_view;
			}
			/* make sure first component shown is within allowable range */
			if (number_of_components-curve_editor->components_in_view <
				curve_editor->first_component_in_view)
			{
				curve_editor->first_component_in_view=
					number_of_components-curve_editor->components_in_view;
			}
			/* get size of window */
			XtVaGetValues(curve_editor->drawing_area,XmNwidth,&winwidth,
				XmNheight,&winheight,NULL);
			curve_editor->y_axis_width=
				curve_editor->curve_drawing_information->vertical_axis_width;
			curve_editor->comp_box_width=(winwidth-curve_editor->y_axis_width);
			curve_editor->comp_box_height=(winheight-
				curve_editor->curve_drawing_information->horizontal_axis_height)/
				curve_editor->components_in_view;
			curve_editor->x_axis_height=winheight-
				curve_editor->comp_box_height*curve_editor->components_in_view;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_drawing_layout_change.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* control_curve_editor_drawing_layout_change */

static int control_curve_editor_get_grid_spacing(FE_value *minor_grid_size,
	int *minor_grids_per_major,FE_value scale,int min_minor_grid_pixels,
	int min_major_grid_pixels)
/*******************************************************************************
LAST MODIFIED : 17 October 1997

DESCRIPTION :
Returns the spacing of minor and major (major include labels in this case) grids
that cross the drawing area. From an initial, desired <minor_grid_spacing>, this
is enlarged by multiplying by 2,2.5,2 (to give 2x,5x,10x,20x the original value)
until the grid is spaced apart by at least <min_minor_grid_pixels> on the
screen. FE_Value values are multiplied by <scale> to give pixel values.
A suitable number of minor_grids_per_major is then calculated, ensuring that
major grid lines are spaced apart by at least <min_major_grid_pixels>.
==============================================================================*/
{
	int return_code,j;

	ENTER(control_curve_editor_get_grid_spacing);
	if (*minor_grid_size&&(0< *minor_grid_size)&&minor_grids_per_major&&
		(0.0!=scale))
	{
		j=1;
		while (fabs(scale*(*minor_grid_size)) < min_minor_grid_pixels)
		{
			if (j=(j+1)%3)
			{
				*minor_grid_size *= 2.0;
			}
			else
			{
				*minor_grid_size *= 2.5;
			}
		}
		*minor_grids_per_major=1;
		if (0==j)
		{
			j=1;
			if (fabs(scale*(*minor_grids_per_major)*(*minor_grid_size))<
				min_major_grid_pixels)
			{
				*minor_grids_per_major *= 2;
			}
		}
		while (fabs(scale*(*minor_grids_per_major)*(*minor_grid_size))<
			min_major_grid_pixels)
		{
			if (j=(j+1)%2)
			{
				*minor_grids_per_major *= 5;
			}
			else
			{
				*minor_grids_per_major *= 2;
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_get_grid_spacing.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* control_curve_editor_get_grid_spacing */

static int control_curve_editor_draw_component(
	struct Control_curve_editor *curve_editor,int component_no,
	int rubber_band)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Draws a component of the curve, including axis and grid.
If <rubber_band> is true, draws only those bits that are shifted during editing
of a node.
==============================================================================*/
{
	char any_string[50],*comp_name;
	Display *display;
	FE_value xstartvalue,ystartvalue,xscale,yscale,xfinishvalue,yfinishvalue,
		dS_dxi,parameter,parameter_change,component_grid_size,
		parameter_grid_size,xn,yn,value;
	FE_value *values,*derivatives,*comp_values,real_num_segments,xi;
	int return_code,number_of_elements,elem_no,node_no,i,first,last,xmin,ymin,
		x,y,number_of_nodes,node_size,pick_distance,first_node_no,last_node_no,
		boxleft,boxright,boxtop,boxbottom,num_segments,grid_lines_per_label,
		tickleft,length,ascent,descent,direction,number_of_components;
	Window window;
	GC gc,control_gc;
	struct FE_field *field;
	struct FE_field_component field_component;
	struct Control_curve *curve;
	XCharStruct bounds;
	XFontStruct *font;
	XPoint *points;
	XRectangle clip_rectangle;

	ENTER(control_curve_editor_draw_component);
	if (curve_editor&&curve_editor->user_interface&&curve_editor->drawing_area&&
		curve_editor->curve_drawing_information&&
		(curve=curve_editor->edit_curve)&&
		(display=curve_editor->user_interface->display)&&
		(window=XtWindow(curve_editor->drawing_area))&&
		(0 <= component_no)&&
		(Control_curve_get_number_of_components(curve)>component_no))
	{
		font=curve_editor->curve_drawing_information->font;
		/* allocate space for points array where polyline vertices stored */
		/* ...and space for num_segments+1 component values */
		num_segments=curve_editor->curve_drawing_information->curve_segments;
		/* Override number of points when the function is linear */
		if (LINEAR_LAGRANGE==Control_curve_get_fe_basis_type(curve))
		{
			num_segments = 1;
		}
		real_num_segments=(FE_value)num_segments;
		number_of_components=Control_curve_get_number_of_components(curve);
		points=(XPoint *)NULL;
		comp_values=(FE_value *)NULL;
		values=(FE_value *)NULL;
		derivatives=(FE_value *)NULL;
		if ((0<num_segments)&&ALLOCATE(points,XPoint,num_segments+1)&&
			ALLOCATE(comp_values,FE_value,num_segments+1)&&
			ALLOCATE(values,FE_value,number_of_components)&&
			ALLOCATE(derivatives,FE_value,number_of_components))
		{
			return_code=1;
			if ((0<curve_editor->comp_box_width)&&(0<curve_editor->comp_box_height)&&
				get_drawing_scale_factors_x(curve_editor,&xmin,&xscale,&xstartvalue)&&
				get_drawing_scale_factors_y(curve_editor,component_no,&ymin,&yscale,
					&ystartvalue))
			{
				tickleft=xmin-curve_editor->curve_drawing_information->tick_length+1;
				xfinishvalue=xstartvalue+curve_editor->comp_box_width/xscale;
				yfinishvalue=ystartvalue-curve_editor->comp_box_height/yscale;
				/* get size of component box - excluding border */
				boxleft=curve_editor->y_axis_width;
				boxright=curve_editor->y_axis_width+curve_editor->comp_box_width-2;
				boxbottom=ymin-1;
				boxtop=ymin-curve_editor->comp_box_height+1;
				if (!rubber_band)
				{
					/* clear the area to be taken up by the component display */
					XClearArea(display,window,1,boxtop,boxright,
						curve_editor->comp_box_height-1,False);
					/* draw grid lines to extent of area for component */
					if (Control_curve_get_parameter_grid(curve,&parameter_grid_size)&&
						Control_curve_get_value_grid(curve,&component_grid_size))
					{
						/* parameter grid lines */
						if ((0<parameter_grid_size)&&control_curve_editor_get_grid_spacing(
							&parameter_grid_size,&grid_lines_per_label,xscale,
							curve_editor->curve_drawing_information->min_grid_spacing,
							curve_editor->curve_drawing_information->
							min_horizontal_label_spacing))
						{
							/* grid lines only drawn for visible range */
							first=ceil(xstartvalue/parameter_grid_size);
							last=floor(xfinishvalue/parameter_grid_size);
							for (i=first;i<=last;i++)
							{
								if (0==(i % grid_lines_per_label))
								{
									gc=curve_editor->curve_drawing_information->major_grid_gc;
								}
								else
								{
									gc=curve_editor->curve_drawing_information->minor_grid_gc;
								}
								x=xmin+xscale*(i*parameter_grid_size-xstartvalue);
								XDrawLine(display,window,gc,x,boxtop,x,boxbottom);
							}
						}
						/* component grid lines */
						if ((0<component_grid_size)&&control_curve_editor_get_grid_spacing(
							&component_grid_size,&grid_lines_per_label,yscale,
							curve_editor->curve_drawing_information->min_grid_spacing,
							curve_editor->curve_drawing_information->
							min_vertical_label_spacing))
						{
							/* grid lines only drawn for visible range */
							first=ceil(ystartvalue/component_grid_size);
							last=floor(yfinishvalue/component_grid_size);
							for (i=first;i<=last;i++)
							{
								value=(FE_value)i*component_grid_size;
								y=ymin+yscale*(value-ystartvalue);
								if (0==(i % grid_lines_per_label))
								{
									gc=curve_editor->curve_drawing_information->axis_gc;
									XDrawLine(display,window,gc,tickleft,y,xmin-1,y);
									sprintf(any_string,"%g",value);
									length=strlen(any_string);
									XTextExtents(font,any_string,length,&direction,&ascent,
										&descent,&bounds);
									XDrawString(display,window,gc,tickleft-bounds.width-1,
										y,any_string,length);
									gc=curve_editor->curve_drawing_information->major_grid_gc;
								}
								else
								{
									gc=curve_editor->curve_drawing_information->minor_grid_gc;
									XDrawLine(display,window,gc,tickleft,y,xmin-1,y);
								}
								XDrawLine(display,window,gc,boxleft,y,boxright,y);
							}
						}
					}
					/* draw border/component separator lines */
					gc=curve_editor->curve_drawing_information->separator_gc;
					XDrawLine(display,window,gc,0,boxbottom,0,boxtop);
					XDrawLine(display,window,gc,0,boxtop-1,boxright+1,boxtop-1);
					XDrawLine(display,window,gc,boxright+1,boxtop,boxright+1,boxbottom);
					XDrawLine(display,window,gc,boxleft-1,boxbottom,boxleft-1,boxtop);
				}
				number_of_elements=Control_curve_get_number_of_elements(curve);
				number_of_nodes=Control_curve_get_nodes_per_element(curve);
				last_node_no=number_of_nodes-1;
				node_size=curve_editor->curve_drawing_information->control_point_size;
				/* set clip boundary rectangle */
				clip_rectangle.x=boxleft;
				clip_rectangle.y=boxtop;
				clip_rectangle.width=curve_editor->comp_box_width-1;
				clip_rectangle.height=curve_editor->comp_box_height-1;
				if (!rubber_band)
				{
					gc=curve_editor->curve_drawing_information->function_gc;
					control_gc=curve_editor->curve_drawing_information->control_point_gc;
					/* clip to bounding box */
					XSetClipRectangles(display,gc,0,0,&clip_rectangle,1,Unsorted);
					XSetClipRectangles(display,control_gc,0,0,&clip_rectangle,1,Unsorted);
					/* write component name in drawing area */
					if (field=Control_curve_get_value_field(curve))
					{
						comp_name=(char *)NULL;
						field_component.field=field;
						field_component.number=component_no;
						if (GET_NAME(FE_field_component)(&field_component,&comp_name))
						{
							length=strlen(comp_name);
							XTextExtents(font,comp_name,length,&direction,&ascent,
								&descent,&bounds);
							XDrawString(display,window,gc,boxleft+4,boxtop+2+ascent,comp_name,
								length);
							DEALLOCATE(comp_name);
						}
					}
					for (elem_no=1;return_code&&(elem_no <= number_of_elements);elem_no++)
					{
						/* draw element curves */
						if (Control_curve_calculate_component_over_element(curve,
							elem_no,component_no,num_segments,comp_values))
						{
							for (i=0;(i<=num_segments)&&return_code;i++)
							{
								xi=(FE_value)i/real_num_segments;
								if (Control_curve_get_parameter_in_element(curve,elem_no,xi,&parameter))
								{
									points[i].x=xmin+xscale*(parameter-xstartvalue);
									points[i].y=ymin+yscale*(comp_values[i]-ystartvalue);
								}
								else
								{
									return_code=0;
								}
							}
						}
						if (return_code)
						{
							XDrawLines(display,window,gc,points,num_segments+1,
								CoordModeOrigin);
						}
						/* draw nodes */
						if (1<elem_no)
						{
							first_node_no=1;
						}
						else
						{
							first_node_no=0;
						}
						for (node_no=first_node_no;node_no<number_of_nodes;node_no++)
						{
							if (Control_curve_get_parameter(curve,elem_no,node_no,&parameter)&&
								Control_curve_get_node_values(curve,elem_no,node_no,values))
							{
								x=xmin+(parameter-xstartvalue)*xscale-node_size/2;
								y=ymin+(values[component_no]-ystartvalue)*yscale-node_size/2;
								if ((0==node_no)||(last_node_no==node_no))
								{
									XFillRectangle(display,window,control_gc,x,y,
										node_size,node_size);
								}
								else
								{
									XDrawRectangle(display,window,control_gc,x,y,
										node_size-1,node_size-1);
								}
							}
							else
							{
								return_code=0;
							}
						}
					}
				}
				/* rubber band effect: */
				if ((0 < curve_editor->current_element_no)&&
					(0 <= curve_editor->current_node_no))
				{
					gc=curve_editor->curve_drawing_information->drag_gc;
					/* clip to bounding box */
					XSetClipRectangles(display,gc,0,0,&clip_rectangle,1,Unsorted);
					first=curve_editor->current_element_no;
					last=first;
					if ((0 == curve_editor->current_node_no) && (1 < first))
					{
						first--;
					}
					if (((number_of_nodes-1) == curve_editor->current_node_no)&&
						(last < number_of_elements))
					{
						last++;
					}
					/* draw element curves */
					for (elem_no=first;return_code&&(elem_no <= last);elem_no++)
					{
						/* draw element curves */
						if (Control_curve_calculate_component_over_element(curve,
							elem_no,component_no,num_segments,comp_values))
						{
							for (i=0;(i<=num_segments)&&return_code;i++)
							{
								xi=(FE_value)i/real_num_segments;
								if (Control_curve_get_parameter_in_element(curve,elem_no,xi,&parameter))
								{
									points[i].x=xmin+xscale*(parameter-xstartvalue);
									points[i].y=ymin+yscale*(comp_values[i]-ystartvalue);
								}
								else
								{
									return_code=0;
								}
							}
						}
						if (return_code)
						{
							XDrawLines(display,window,gc,points,num_segments+1,
								CoordModeOrigin);
						}
						/* draw nodes */
						if (first<elem_no)
						{
							first_node_no=1;
						}
						else
						{
							first_node_no=0;
						}
						for (node_no=first_node_no;node_no<number_of_nodes;node_no++)
						{
							if (Control_curve_get_parameter(curve,elem_no,node_no,&parameter)&&
								Control_curve_get_node_values(curve,elem_no,node_no,values))
							{
								x=xmin+(parameter-xstartvalue)*xscale-node_size/2;
								y=ymin+(values[component_no]-ystartvalue)*yscale-node_size/2;
								if ((0==node_no)||(last_node_no==node_no))
								{
									XFillRectangle(display,window,gc,x,y,node_size,node_size);
								}
								else
								{
									XDrawRectangle(display,window,gc,x,y,node_size-1,
										node_size-1);
								}
							}
							else
							{
								return_code=0;
							}
						}
					}
					/* draw selected node */
					elem_no=curve_editor->current_element_no;
					node_no=curve_editor->current_node_no;
					if (Control_curve_get_parameter(curve,elem_no,node_no,&parameter)&&
						Control_curve_get_node_values(curve,elem_no,node_no,values))
					{
						pick_distance=
							curve_editor->curve_drawing_information->pick_distance;
						x=xmin+(parameter-xstartvalue)*xscale-pick_distance;
						y=ymin+(values[component_no]-ystartvalue)*yscale-pick_distance;
						XDrawArc(display,window,gc,x,y,pick_distance*2,pick_distance*2,
							0,23040);
						if (0<Control_curve_get_derivatives_per_node(curve))
						{
							if (Control_curve_get_node_derivatives(curve,
								curve_editor->current_element_no,curve_editor->current_node_no,
								derivatives))
							{
								/* (xn,yn) = location of selected node */
								xn=xmin+(parameter-xstartvalue)*xscale;
								yn=ymin+(values[component_no]-ystartvalue)*yscale;
								/* draw lines and control points for setting slopes */
								if ((first<last)||
									((number_of_nodes-1)==curve_editor->current_node_no))
								{
									/* draw "left" arm of control lines and points */
									if (Control_curve_get_element_parameter_change(curve,first,
										&parameter_change)&&Control_curve_get_scale_factor(curve,
											first,number_of_nodes-1,&dS_dxi))
									{
										x=xmin+(parameter-parameter_change/3.0-xstartvalue)*xscale;
										y=ymin+(values[component_no]-
											derivatives[component_no]*dS_dxi/3.0-ystartvalue)*yscale;
										XDrawLine(display,window,gc,xn,yn,x,y);
										x-=node_size/2;
										y-=node_size/2;
										XDrawRectangle(display,window,gc,x,y,
											node_size-1,node_size-1);
									}
								}
								if ((first<last)||(0==curve_editor->current_node_no))
								{
									/* draw "right" arm of control lines and points */
									if (Control_curve_get_element_parameter_change(curve,last,
										&parameter_change)&&Control_curve_get_scale_factor(curve,
											last,0,&dS_dxi))
									{
										x=xmin+(parameter+parameter_change/3.0-xstartvalue)*xscale;
										y=ymin+(values[component_no]+
											derivatives[component_no]*dS_dxi/3.0-ystartvalue)*yscale;
										XDrawLine(display,window,gc,xn,yn,x,y);
										x-=node_size/2;
										y-=node_size/2;
										XDrawRectangle(display,window,gc,x,y,
											node_size-1,node_size-1);
									}
								}
							}
							else
							{
								return_code=0;
							}
						}
					}
					else
					{
						return_code=0;
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_draw_component.  Error creating points array");
			return_code=0;
		}
		if (points)
		{
			DEALLOCATE(points);
		}
		if (comp_values)
		{
			DEALLOCATE(comp_values);
		}
		if (values)
		{
			DEALLOCATE(values);
		}
		if (derivatives)
		{
			DEALLOCATE(derivatives);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_draw_component.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* control_curve_editor_draw_component */

static int control_curve_editor_draw_x_axis(
	struct Control_curve_editor *curve_editor)
/*******************************************************************************
LAST MODIFIED : 17 October 1997

DESCRIPTION :
Draws the x-axis used by all components.
==============================================================================*/
{
	char any_string[50];
	Dimension winwidth,winheight;
	Display *display;
	FE_value xstartvalue,xscale,xfinishvalue,component_grid_size,parameter_grid_size,
		parameter;
	int return_code,i,x,first,last,xmin,ytop,ybottom,tickbottom,length,ascent,
		descent,direction,grid_lines_per_label;
	Window window;
	GC gc;
	struct Control_curve *curve;
	XCharStruct bounds;
	XFontStruct *font;

	ENTER(control_curve_editor_draw_x_axis);
	if (curve_editor&&curve_editor->user_interface&&curve_editor->drawing_area&&
		curve_editor->curve_drawing_information&&
		(curve=curve_editor->edit_curve)&&
		(display=curve_editor->user_interface->display)&&
		(window=XtWindow(curve_editor->drawing_area)))
	{
		font=curve_editor->curve_drawing_information->font;
		/* get size of window */
		XtVaGetValues(curve_editor->drawing_area,XmNwidth,&winwidth,
			XmNheight,&winheight,NULL);
		get_drawing_scale_factors_x(curve_editor,&xmin,&xscale,&xstartvalue);
		xfinishvalue=xstartvalue+curve_editor->comp_box_width/xscale;
		/* clear area used by axis */
		ytop=winheight-curve_editor->x_axis_height;
		ybottom=winheight-1;
		tickbottom=ytop+curve_editor->curve_drawing_information->tick_length-1;
		XClearArea(display,window,0,ytop,winwidth,winheight,False);
		/* draw border/component separator lines */
		gc=curve_editor->curve_drawing_information->separator_gc;
		XDrawLine(display,window,gc,0,ybottom,0,ytop);
		XDrawLine(display,window,gc,1,ytop,xmin,ytop);
		XDrawLine(display,window,gc,winwidth-1,ytop,winwidth-1,ybottom);
		/*XDrawLine(display,window,gc,xmin,ybottom-1,xmin,ytop+1);*/
		XDrawLine(display,window,gc,winwidth-2,ybottom,1,ybottom);
		/* draw grid lines to extent of area for component */
		if (Control_curve_get_parameter_grid(curve,&parameter_grid_size)&&
			Control_curve_get_value_grid(curve,&component_grid_size))
		{
			gc=curve_editor->curve_drawing_information->separator_gc;
			XDrawLine(display,window,gc,xmin+1,ytop,winwidth-2,ytop);
			/* parameter grid lines */
			if ((0<parameter_grid_size)&&control_curve_editor_get_grid_spacing(
				&parameter_grid_size,&grid_lines_per_label,xscale,
				curve_editor->curve_drawing_information->min_grid_spacing,
				curve_editor->curve_drawing_information->min_horizontal_label_spacing))
			{
				/* grid lines only drawn for visible range */
				first=ceil(xstartvalue/parameter_grid_size);
				last=floor(xfinishvalue/parameter_grid_size);
				for (i=first;i<=last;i++)
				{
					parameter=i*parameter_grid_size;
					x=xmin+xscale*(parameter-xstartvalue);
					if (0==(i % grid_lines_per_label))
					{
						gc=curve_editor->curve_drawing_information->axis_gc;
						sprintf(any_string,"%g",parameter);
						length=strlen(any_string);
						XTextExtents(font,any_string,length,&direction,&ascent,&descent,
							&bounds);
						XDrawString(display,window,gc,x-bounds.width/2,tickbottom+ascent,
							any_string,length);
					}
					else
					{
						gc=curve_editor->curve_drawing_information->minor_grid_gc;
					}
					XDrawLine(display,window,gc,x,ytop+1,x,tickbottom);
				}
			}
		}
		gc=curve_editor->curve_drawing_information->axis_gc;
		sprintf(any_string,"Parameter:");
		length=strlen(any_string);
		XTextExtents(font,any_string,length,&direction,&ascent,&descent,&bounds);
		XDrawString(display,window,gc,4,tickbottom+ascent,any_string,length);
		return_code=1;
	}
	else
	{
		printf("control_curve_editor_draw_x_axis\n");
		printf("curve_editor = %p\n",curve_editor);
		printf("curve_editor->user_interface = %p\n",curve_editor->user_interface);
		printf("curve_editor->drawing_area = %p\n",curve_editor->drawing_area);
		printf("curve_editor->curve_drawing_information = %p\n",
			curve_editor->curve_drawing_information);
		printf("curve_editor->edit_curve = %p\n",curve_editor->edit_curve);
		printf("display = %p\n",display);
		printf("window = %p\n",(void *)window); /* The string conversion is just to suppress a compiler warning */
		display_message(ERROR_MESSAGE,
			"control_curve_editor_draw_x_axis.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* control_curve_editor_draw_x_axis */

static int control_curve_editor_draw_cursor(
	struct Control_curve_editor *curve_editor)
/*******************************************************************************
LAST MODIFIED : 9 November 1999

DESCRIPTION :
Draws the x-axis used by all components.
==============================================================================*/
{
	Dimension winwidth,winheight;
	Display *display;
	FE_value xstartvalue,xscale;
	int return_code,x,xmin,ytop,ybottom;
	Window window;
	GC gc;
#if defined (OLD_CODE)
	XCharStruct bounds;
	XFontStruct *font;
#endif /* defined (OLD_CODE) */

	ENTER(control_curve_editor_draw_x_axis);
	if (curve_editor&&curve_editor->user_interface&&curve_editor->drawing_area&&
		curve_editor->curve_drawing_information&&
		(curve_editor->edit_curve)&&
		(display=curve_editor->user_interface->display)&&
		(window=XtWindow(curve_editor->drawing_area)))
	{
		gc=curve_editor->curve_drawing_information->cursor_gc;
#if defined (OLD_CODE)
		font=curve_editor->curve_drawing_information->font;
#endif /* defined (OLD_CODE) */
		/* get size of window */
		XtVaGetValues(curve_editor->drawing_area,XmNwidth,&winwidth,
			XmNheight,&winheight,NULL);
		ytop=1;
		ybottom=winheight-1;
		get_drawing_scale_factors_x(curve_editor,&xmin,&xscale,&xstartvalue);

		x=xmin+xscale*(curve_editor->cursor-xstartvalue);
		XDrawLine(display,window,gc,x,ybottom,x,ytop);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_draw_cursor.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* control_curve_editor_draw_cursor */

static int control_curve_editor_drawing_area_redraw(
	struct Control_curve_editor *curve_editor,int rubber_band)
/*******************************************************************************
LAST MODIFIED : 2 October 1997

DESCRIPTION :
Redraws the entire drawing area.
==============================================================================*/
{
	int return_code,comp_no;

	ENTER(control_curve_editor_drawing_area_redraw);
	if (curve_editor)
	{
		/* only redraw if curve being edited and window set up */
		if ((curve_editor->edit_curve)&&(XtWindow(curve_editor->drawing_area)))
		{
			for (comp_no=curve_editor->first_component_in_view;comp_no<
				(curve_editor->first_component_in_view+curve_editor->components_in_view);
				comp_no++)
			{
				control_curve_editor_draw_component(curve_editor,comp_no,rubber_band);
			}
			if (!rubber_band)
			{
				control_curve_editor_draw_x_axis(curve_editor);
				if ( curve_editor->cursor_displayed )
				{
					control_curve_editor_draw_cursor(curve_editor);
				}
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_drawing_area_redraw.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* control_curve_editor_drawing_area_redraw */

static void control_curve_editor_expose_drawing_area_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 October 1997

DESCRIPTION :
Called when the drawing area is exposed.
==============================================================================*/
{
	struct Control_curve_editor *curve_editor;
#if defined (OLD_CODE)
	GC gc;
	Display *display;
	Window window;
#endif /* defined (OLD_CODE) */

	ENTER(control_curve_editor_expose_drawing_area_CB);
	USE_PARAMETER(call_data);
	if (widget&&
		(curve_editor=(struct Control_curve_editor *)client_data))
	{
		if (curve_editor->edit_curve)
		{
			control_curve_editor_drawing_area_redraw(curve_editor,0);
		}
#if defined (OLD_CODE)
		/* get the widget from the call data */
		if (window=((XmDrawingAreaCallbackStruct *)call_data)->window)
		{
			display=curve_editor->user_interface->display;
			gc=curve_editor->curve_drawing_information->axis_gc;
			XDrawLine(display,window,gc,0,0,100,50);
			gc=curve_editor->curve_drawing_information->control_point_gc;
			XDrawLine(display,window,gc,0,200,200,50);
			gc=curve_editor->curve_drawing_information->drag_gc;
			XDrawLine(display,window,gc,0,100,100,90);
			gc=curve_editor->curve_drawing_information->function_gc;
			XDrawLine(display,window,gc,10,30,249,50);
			gc=curve_editor->curve_drawing_information->grid_gc;
			XDrawLine(display,window,gc,250,0,150,200);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_expose_drawing_area_CB.  Invalid callback data");
		}
#endif /*defined (OLD_CODE)*/
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_expose_drawing_area_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* control_curve_editor_expose_drawing_area_CB */

static void control_curve_editor_resize_drawing_area_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 2 October 1997

DESCRIPTION :
Called when the drawing area is resized.
==============================================================================*/
{
	struct Control_curve_editor *curve_editor;

	ENTER(control_curve_editor_resize_drawing_area_CB);
	USE_PARAMETER(call_data);
	if (widget&&
		(curve_editor=(struct Control_curve_editor *)client_data))
	{
		control_curve_editor_drawing_layout_change(curve_editor);
		control_curve_editor_drawing_area_redraw(curve_editor,0);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_resize_drawing_area_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* control_curve_editor_resize_drawing_area_CB */

static int control_curve_editor_get_picked_control_point(
	struct Control_curve_editor *curve_editor,int pointer_x,int pointer_y)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
If there is a currently selected node, and derivatives are defined there,
tries to detect whether the pointer is selecting a control point for the slope.
Returned values are 0=no control point selected or error, 1=left control point
selected, 1=right control point selected. (Left and right refer to the two
"arms" coming out from the selected node, the left one in the direction of the
next lower element, the right one in the opposite direction.)
New version registers a pick if the slope line is clicked on - not just the
control point at the end.
==============================================================================*/
{
	FE_value *derivatives,*values;
	FE_value pick_distance,parameter,parameter_change,dS_dxi,xstartvalue,
		ystartvalue,xscale,yscale,xn,yn,x,y,mag,c1x,c1y,c2x,c2y,dist1;
	int return_control_point,comp_no,elem_no,node_no,boxleft,boxright,boxtop,
		boxbottom,xmin,ymin,number_of_elements,number_of_nodes,first,last,
		number_of_components;
	struct Control_curve *curve;

	ENTER(control_curve_editor_get_picked_control_point);
	return_control_point=0;
	if (curve_editor&&curve_editor->curve_drawing_information&&
		(curve=curve_editor->edit_curve))
	{
		if ((0<curve_editor->current_element_no)&&
			(0<Control_curve_get_derivatives_per_node(curve)))
		{
			number_of_components=Control_curve_get_number_of_components(curve);
			values=(FE_value *)NULL;
			derivatives=(FE_value *)NULL;
			if (ALLOCATE(values,FE_value,number_of_components)&&
				ALLOCATE(derivatives,FE_value,number_of_components))
			{
				get_drawing_scale_factors_x(curve_editor,&xmin,&xscale,&xstartvalue);
				boxleft=xmin+1;
				boxright=xmin+curve_editor->comp_box_width-1;
				/* make sure pointer lies between sides of component boxes */
				if ((pointer_x >= boxleft)&&(pointer_x <= boxright))
				{
					/* now find which component box the pointer is in, if any */
					for (comp_no=curve_editor->first_component_in_view;comp_no<
						(curve_editor->first_component_in_view+
							curve_editor->components_in_view);comp_no++)
					{
						get_drawing_scale_factors_y(curve_editor,comp_no,&ymin,&yscale,
							&ystartvalue);
						boxbottom=ymin-1;
						boxtop=ymin-curve_editor->comp_box_height+1;
						if ((pointer_y >= boxtop)&&(pointer_y <= boxbottom))
						{
							curve_editor->current_component_no=comp_no;
							number_of_nodes=Control_curve_get_nodes_per_element(curve);
							number_of_elements=Control_curve_get_number_of_elements(curve);
							pick_distance=
								0.5*curve_editor->curve_drawing_information->pick_distance;
							elem_no=curve_editor->current_element_no;
							node_no=curve_editor->current_node_no;
							/* get first and last element containing node */
							first=curve_editor->current_element_no;
							last=first;
							if ((0==curve_editor->current_node_no) && (1<first))
							{
								first--;
							}
							if (((number_of_nodes-1)==curve_editor->current_node_no)&&
								(last<number_of_elements))
							{
								last++;
							}
							if (Control_curve_get_parameter(curve,elem_no,node_no,&parameter)&&
								Control_curve_get_node_values(curve,elem_no,node_no,values)&&
								Control_curve_get_node_derivatives(curve,elem_no,node_no,
									derivatives))
							{
								/* get location of node */
								xn=xmin+(parameter-xstartvalue)*xscale;
								yn=ymin+(values[comp_no]-ystartvalue)*yscale;
								if ((first<last)||
									((number_of_nodes-1)==curve_editor->current_node_no))
								{
									/* check "left" control point */
									if (pointer_x < xn)
									{
										if (Control_curve_get_element_parameter_change(curve,first,
											&parameter_change)&&Control_curve_get_scale_factor(curve,
												first,number_of_nodes-1,&dS_dxi))
										{
											/* get location of control point */
											x=xmin+(parameter-parameter_change/3.0-xstartvalue)*xscale;
											y=ymin+(values[comp_no]-derivatives[comp_no]*
												dS_dxi/3.0-ystartvalue)*yscale;
											/* get axis along control line and its normal */
											if (0<(mag=sqrt((x-xn)*(x-xn)+(y-yn)*(y-yn))))
											{
												c1x=(x-xn)/mag;
												c1y=(y-yn)/mag;
											}
											else
											{
												c1x=-1;
												c1y=0;
											}
											c2x=c1y;
											c2y=-c1x;
											/* check pointer is close enough to line */
											if (fabs((pointer_x-xn)*c2x+(pointer_y-yn)*c2y)<
												pick_distance)
											{
												/* check pointer not too far past end of line */
												if ((dist1=fabs((pointer_x-xn)*c1x+(pointer_y-yn)*c1y))<
													(mag+pick_distance))
												{
													/* avoid selecting if pointer close to node */
													if ((mag<=pick_distance)||(dist1>=pick_distance))
													{
														return_control_point=1;
														/* following makes logic simpler for later: */
														curve_editor->current_element_no=first;
														curve_editor->current_node_no=number_of_nodes-1;
													}
												}
											}
										}
									}
								}
								if ((first<last)||(0==curve_editor->current_node_no))
								{
									if (pointer_x > xn)
									{
										/* check "right" control point */
										if (Control_curve_get_element_parameter_change(curve,last,
											&parameter_change)&&Control_curve_get_scale_factor(curve,
												last,0,&dS_dxi))
										{
											/* get location of control point */
											x=xmin+(parameter+parameter_change/3.0-xstartvalue)*xscale;
											y=ymin+(values[comp_no]+derivatives[comp_no]*
												dS_dxi/3.0-ystartvalue)*yscale;
											/* get axis along control line and its normal */
											if (0<(mag=sqrt((x-xn)*(x-xn)+(y-yn)*(y-yn))))
											{
												c1x=(x-xn)/mag;
												c1y=(y-yn)/mag;
											}
											else
											{
												c1x=1;
												c1y=0;
											}
											c2x=c1y;
											c2y=-c1x;
											/* check pointer is close enough to line */
											if (fabs((pointer_x-xn)*c2x+(pointer_y-yn)*c2y)<
												pick_distance)
											{
												/* check pointer not too far past end of line */
												if ((dist1=fabs((pointer_x-xn)*c1x+(pointer_y-yn)*c1y))<
													(mag+pick_distance))
												{
													/* avoid selecting if pointer close to node */
													if ((mag<=pick_distance)||(dist1>=pick_distance))
													{
														return_control_point=2;
														/* following makes logic simpler for later: */
														curve_editor->current_element_no=last;
														curve_editor->current_node_no=0;
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
			if (values)
			{
				DEALLOCATE(values);
			}
			if (derivatives)
			{
				DEALLOCATE(derivatives);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_get_picked_control_point.  Invalid argument(s)");
	}
	LEAVE;

	return (return_control_point);
} /* control_curve_editor_get_picked_control_point */

static FE_value snap_value(FE_value value,FE_value snap_spacing)
/*******************************************************************************
LAST MODIFIED : 22 October 1997

DESCRIPTION :
If snap_spacing is positive, value is rounded to the nearest multiple of it.
Otherwise, value is returned unchanged.
==============================================================================*/
{
	FE_value return_value;

	ENTER(snap_value);
	if (0.0 < snap_spacing)
	{
		return_value=floor(value/snap_spacing+0.5)*snap_spacing;
	}
	else
	{
		return_value=value;
	}
	LEAVE;

	return (return_value);
} /* snap_value */

static int control_curve_editor_snap_parameter(
	struct Control_curve_editor *curve_editor,FE_value *parameter)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
If snap is on for the parameter in <curve>, snaps <parameter> to the nearest
parameter grid division - if not zero.
==============================================================================*/
{
	FE_value parameter_grid;
	int return_code;

	ENTER(control_curve_editor_snap_parameter);
	if (curve_editor&&parameter)
	{
		if (curve_editor->snap_parameter&&
			Control_curve_get_parameter_grid(curve_editor->edit_curve,
				&parameter_grid))
		{
			*parameter=snap_value(*parameter,parameter_grid);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_snap_parameter.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* control_curve_editor_snap_parameter */

static int control_curve_editor_snap_value(
	struct Control_curve_editor *curve_editor,int component_no,FE_value *value)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
If snap is on for the value in <curve>, snaps <value> to the nearest
value grid division - if not zero. Then limits value to allowed range
for the given <component_no>.
==============================================================================*/
{
	FE_value max_value,min_value,value_grid;
	int return_code;

	ENTER(control_curve_editor_snap_value);
	if (curve_editor&&value)
	{
		if (curve_editor->snap_value&&
			Control_curve_get_value_grid(curve_editor->edit_curve,&value_grid))
		{
			*value=snap_value(*value,value_grid);
		}
		if (Control_curve_get_edit_component_range(curve_editor->edit_curve,
			component_no,&min_value,&max_value))
		{
			if (*value > max_value)
			{
				*value = max_value;
			}
			else if (*value < min_value)
			{
				*value = min_value;
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_snap_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* control_curve_editor_snap_value */

static int control_curve_editor_get_picked_node(
	struct Control_curve_editor *curve_editor,int pointer_x,int pointer_y)
/*******************************************************************************
LAST MODIFIED : 29 November 1999

DESCRIPTION :
Finds the nearest node - ie. element_no/node_no/component_no - out of the
components visible on screen. Node must be within pick_distance of the mouse
pointer location.
Return node as curve_editor members current_element_no,current_node_no,
current_component_no.
==============================================================================*/
{
	int return_code,comp_no,elem_no,node_no,last_node_no,subdivided,
		boxleft,boxright,boxtop,boxbottom,xmin,ymin,number_of_elements,
		number_of_nodes,i,number_of_components,first_node_no;
	FE_value this_distance,pick_distance,xi,dx,dy,parameter,*values,
		xstartvalue,ystartvalue,xscale,yscale,elem_parameter1,elem_parameter2,
		max_range,min_range;
	struct Control_curve *curve;

	ENTER(control_curve_editor_get_picked_node);
	if (curve_editor&&curve_editor->curve_drawing_information&&
		(curve=curve_editor->edit_curve))
	{
		return_code=1;
		curve_editor->current_element_no=0;
		curve_editor->current_node_no=-1;
		curve_editor->current_component_no=-1;
		get_drawing_scale_factors_x(curve_editor,&xmin,&xscale,&xstartvalue);
		boxleft=xmin+1;
		boxright=xmin+curve_editor->comp_box_width-1;
		/* make sure pointer lies between sides of component boxes */
		if ((pointer_x >= boxleft)&&(pointer_x <= boxright))
		{
			number_of_components=Control_curve_get_number_of_components(curve);
			if (ALLOCATE(values,FE_value,number_of_components))
			{
				/* now find which component box the pointer is in, if any */
				for (comp_no=curve_editor->first_component_in_view;comp_no<
					(curve_editor->first_component_in_view+
						curve_editor->components_in_view);comp_no++)
				{
					get_drawing_scale_factors_y(curve_editor,comp_no,&ymin,&yscale,
						&ystartvalue);
					boxbottom=ymin-1;
					boxtop=ymin-curve_editor->comp_box_height+1;
					if ((pointer_y >= boxtop)&&(pointer_y <= boxbottom))
					{
						number_of_nodes=Control_curve_get_nodes_per_element(curve);
						if (0<(number_of_elements=
							Control_curve_get_number_of_elements(curve)))
						{
							pick_distance=
								curve_editor->curve_drawing_information->pick_distance;
							for (elem_no=1;return_code&&(elem_no <= number_of_elements);
									 elem_no++)
							{
								if (1==elem_no)
								{
									first_node_no=0;
								}
								else
								{
									first_node_no=1;
								}
								for (node_no=first_node_no;(node_no<number_of_nodes)&&
									return_code;node_no++)
								{
									if (Control_curve_get_parameter(curve,elem_no,node_no,&parameter)&&
										Control_curve_get_node_values(curve,elem_no,node_no,values))
									{
										dx=xmin+(parameter-xstartvalue)*xscale-pointer_x;
										dy=ymin+(values[comp_no]-ystartvalue)*yscale-pointer_y;
										if ((this_distance=sqrt(dx*dx+dy*dy)) < pick_distance)
										{
											curve_editor->current_element_no=elem_no;
											curve_editor->current_node_no=node_no;
											curve_editor->current_component_no=comp_no;
											pick_distance=this_distance;
										}
									}
									else
									{
										return_code=0;
									}
								}
							}
						}
						else
						{
							/* add first element at mouse pointer; select last node in it */
							parameter=xstartvalue+(pointer_x-xmin)/xscale;
							control_curve_editor_snap_parameter(curve_editor,&parameter);
							if (Control_curve_add_element(curve,1))
							{
								/* set all components of each node in element to min range */
								for (i=0;i<number_of_components;i++)
								{
									if (i==comp_no)
									{
										values[i]=(pointer_y-ymin)/yscale+ystartvalue;
										control_curve_editor_snap_value(curve_editor,i,
											&(values[i]));
									}
									else
									{
										Control_curve_get_edit_component_range(curve,i,
											&min_range,&max_range);
										values[i]=min_range;
									}
								}
								for (node_no=0;node_no<number_of_nodes;node_no++)
								{
									if (Control_curve_is_node_parameter_modifiable(curve,node_no))
									{
										Control_curve_set_parameter(curve,1,node_no,parameter);
									}
									Control_curve_set_node_values(curve,1,node_no,values);
								}
								/* select last node for dragging */
								curve_editor->current_element_no=1;
								curve_editor->current_node_no=number_of_nodes-1;
								curve_editor->current_component_no=comp_no;
							}
						}
						/* if no node picked, may be trying to subdivide element */
						if (0==curve_editor->current_element_no)
						{
							last_node_no=Control_curve_get_nodes_per_element(curve)-1;
							pick_distance *= 0.5; /* must be close to pick it */
							/* get parameter corresponding to pointer_x */
							parameter=xstartvalue+(pointer_x-xmin)/xscale;
							subdivided=0;
							for (elem_no=1;return_code&&(!subdivided)&&
										 (elem_no <= number_of_elements);elem_no++)
							{
								if (Control_curve_get_parameter(curve,elem_no,0,&elem_parameter1)
									&&Control_curve_get_parameter(curve,elem_no,last_node_no,
										&elem_parameter2))
								{
									if ((elem_parameter1 < parameter)&&(elem_parameter2 > parameter))
									{
										xi=(parameter-elem_parameter1)/(elem_parameter2-elem_parameter1);
										if (Control_curve_get_values_in_element(curve,
											elem_no,xi,values,(FE_value *)NULL))
										{
											dy=ymin+(values[comp_no]-ystartvalue)*yscale-pointer_y;
											if (fabs(dy) < pick_distance)
											{
												subdivided=1;
												if (Control_curve_subdivide_element(curve,elem_no,xi))
												{
													control_curve_editor_drawing_area_redraw(
														curve_editor,0);
													curve_editor->current_element_no=elem_no;
													curve_editor->current_node_no=last_node_no;
													curve_editor->current_component_no=comp_no;
												}
												else
												{
													return_code=0;
												}
											}
										}
										else
										{
											return_code=0;
										}
									}
								}
								else
								{
									return_code=0;
								}
							}
						}
					}
				}
				DEALLOCATE(values);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_get_picked_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* control_curve_editor_get_picked_node */

enum Moving_status
{
	MOVING_NONE,
	MOVING_CONTROL_POINT,
	MOVING_NODE
};

static void control_curve_editor_drawing_area_input_CB(Widget widget,
	XtPointer curve_editor_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
The callback for input to the drawing area where nodes and control points can
be moved.
==============================================================================*/
{
	char buffer[20];
	int bufsize=20;
	XComposeStatus compose;
	Boolean owner_events;
	KeySym keysym;
	Cursor cursor;
	Display *display;
	enum Moving_status moving;
	FE_value left_parameter,parameter,right_parameter;
	FE_value xstartvalue,ystartvalue,xscale,yscale,*values,pointer_parameter,
		pointer_value,velocity,comp_value,*derivatives,
		dS_dxi_dt[2][2],parameter_change,dS_dxi;
	int xmin,ymin,x,y,limit,first,last,elem_no,picked_control_point,pointer_x,
		pointer_y,keyboard_mode,pointer_mode,number_of_nodes,i,left_element_no,
		right_element_no,
		number_of_components,number_of_elements,deleted_element_no;
	struct Control_curve *curve;
	struct Control_curve_editor *curve_editor;
	unsigned int working_button;
	Window confine_to,working_window;
	XButtonEvent *button_event;
	XKeyEvent *key_event;
	XmDrawingAreaCallbackStruct *callback;
	XEvent xevent;
#if defined (OLD_CODE)
	XCharStruct bounds;
	XFontStruct *font;
#endif /* defined (OLD_CODE) */

	ENTER(control_curve_editor_drawing_area_input_CB);
	USE_PARAMETER(widget);
	if ((curve_editor=(struct Control_curve_editor *)curve_editor_void)&&
		(curve=curve_editor->edit_curve)&&(curve_editor->user_interface)&&
		curve_editor->curve_drawing_information&&
		(callback=(XmDrawingAreaCallbackStruct *)call_data)&&
		(callback->event)&&
		(XmCR_INPUT==callback->reason))
	{
		switch (callback->event->type)
		{
			case KeyPress:
			{
				key_event=&(callback->event->xkey);
#if defined (OLD_CODE)
				{
					int charcount;

					charcount=XLookupString(key_event,buffer,bufsize,&keysym,&compose);
				}
#endif /* defined (OLD_CODE) */
				XLookupString(key_event,buffer,bufsize,&keysym,&compose);
				switch (keysym)
				{
					case XK_BackSpace:
					case XK_Delete:
					{
						/*printf("* Delete pressed!\n");*/
						if (0<curve_editor->current_element_no)
						{
							number_of_nodes=Control_curve_get_nodes_per_element(curve);
							number_of_elements=Control_curve_get_number_of_elements(curve);
							/* change last node of element to first node of next element */
							if (((number_of_nodes-1)==curve_editor->current_node_no)&&
								(curve_editor->current_element_no<number_of_elements))
							{
								curve_editor->current_element_no++;
								curve_editor->current_node_no=0;
							}
							else
							{
								if ((number_of_nodes-1)>curve_editor->current_node_no)
								{
									curve_editor->current_node_no=0;
								}
							}
							deleted_element_no=curve_editor->current_element_no;
							if (Control_curve_delete_element(curve,
								curve_editor->current_element_no,curve_editor->current_node_no))
							{
								number_of_elements--;
								if (1<curve_editor->current_element_no)
								{
									curve_editor->current_element_no--;
								}
								if (0==number_of_elements)
								{
									curve_editor->current_element_no=0;
									curve_editor->current_node_no=-1;
									curve_editor->current_component_no=-1;
								}
								if ((0<Control_curve_get_derivatives_per_node(curve))&&
									(curve_editor->current_element_no<deleted_element_no))
								{
									/* enforce continuity over affected nodes */
									if (1<curve_editor->current_element_no)
									{
										Control_curve_enforce_continuity(curve,
											curve_editor->current_element_no-1,number_of_nodes-1,0,
											CONTROL_CURVE_CONTINUITY_SLOPE);
									}
									if (curve_editor->current_element_no<number_of_elements)
									{
										Control_curve_enforce_continuity(curve,
											curve_editor->current_element_no+1,0,0,
											CONTROL_CURVE_CONTINUITY_SLOPE);
									}
								}
								control_curve_editor_drawing_area_redraw(curve_editor,0);
								/* inform the client of the change */
								control_curve_editor_update(curve_editor);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"control_curve_editor_drawing_area_input_CB.  "
									"Could not delete");
							}
						}
					} break;
					default:
					{
						printf("* Unused key pressed!\n");
					} break;
				}
			} break;
			case ButtonPress:
			{
				/* remove currently drawn rubber_band, if any */
				control_curve_editor_drawing_area_redraw(curve_editor,1);
				display=curve_editor->user_interface->display;
				button_event= &(callback->event->xbutton);
				pointer_x=button_event->x;
				pointer_y=button_event->y;
				working_button=button_event->button;
				if ((picked_control_point=control_curve_editor_get_picked_control_point(
					curve_editor,pointer_x,pointer_y))||
					control_curve_editor_get_picked_node(curve_editor,pointer_x,pointer_y))
				{
					if (0<curve_editor->current_element_no)
					{
						if (picked_control_point)
						{
							/*printf(">> picked control point: %i\n",picked_control_point);*/
							moving=MOVING_CONTROL_POINT;
						}
						else
						{
							moving=MOVING_NODE;
						}
						/*printf("Picked node: Elem %i LNNo %i Component %i\n",
							curve_editor->current_element_no,curve_editor->current_node_no,
							curve_editor->current_component_no);*/
						/* draw rubber_band for first parameter */
						/*					control_curve_editor_drawing_area_redraw(curve_editor,1);*/
						/* node picked - now move it */
						/* grab the pointer until button released */
						owner_events=True;
						pointer_mode=GrabModeAsync;
						keyboard_mode=GrabModeAsync;
						confine_to=None;
						cursor=XCreateFontCursor(display,XC_crosshair);
						if (GrabSuccess==XtGrabPointer(curve_editor->drawing_area,
							owner_events,
							ButtonMotionMask|ButtonPressMask|ButtonReleaseMask,
							pointer_mode,keyboard_mode,confine_to,cursor,CurrentTime))
						{
							working_window=XtWindow(curve_editor->drawing_area);
							while (moving != MOVING_NONE)
							{
								XNextEvent(display,&xevent);
								switch (xevent.type)
								{
									case MotionNotify:
									{
										/* reduce the number of motion events displayed */
										while (XCheckMaskEvent(display,ButtonMotionMask,&xevent));
										pointer_x=xevent.xmotion.x;
										pointer_y=xevent.xmotion.y;
										switch (moving)
										{
											case MOVING_CONTROL_POINT:
											{
												control_curve_editor_draw_component(curve_editor,
													curve_editor->current_component_no,1);
												number_of_nodes=
													Control_curve_get_nodes_per_element(curve);
												number_of_components=
													Control_curve_get_number_of_components(curve);
												/* get node parameter and values */
												values=(FE_value *)NULL;
												derivatives=(FE_value *)NULL;
												if (ALLOCATE(values,FE_value,number_of_components)&&
													ALLOCATE(derivatives,FE_value,number_of_components))
												{
													if (Control_curve_get_parameter(curve,
														curve_editor->current_element_no,
														curve_editor->current_node_no,&parameter)&&
														Control_curve_get_node_values(curve,
															curve_editor->current_element_no,
															curve_editor->current_node_no,values))
													{
														/* get pointer parameter and value */
														get_drawing_scale_factors_x(curve_editor,
															&xmin,&xscale,&xstartvalue);
														pointer_parameter=(pointer_x-xmin)/xscale+xstartvalue;
														get_drawing_scale_factors_y(curve_editor,
															curve_editor->current_component_no,&ymin,&yscale,
															&ystartvalue);
														pointer_value=(pointer_y-ymin)/yscale+ystartvalue;
														control_curve_editor_snap_parameter(curve_editor,
															&pointer_parameter);
														control_curve_editor_snap_value(curve_editor,
															curve_editor->current_component_no,&pointer_value);
														if (((1==picked_control_point)&&(pointer_parameter<parameter))||
															(2==picked_control_point)&&(pointer_parameter>parameter))
														{
															velocity=(values[curve_editor->current_component_no]-
																pointer_value)/(parameter-pointer_parameter);
															/* enforce velocity at node while keeping */
															/* unit vector derivative and scaling factor and */
															/* not changing velocities of other components */
															if (Control_curve_get_node_derivatives(
																curve,curve_editor->current_element_no,
																curve_editor->current_node_no,derivatives)&&
																Control_curve_get_scale_factor(curve,
																	curve_editor->current_element_no,
																	curve_editor->current_node_no,&dS_dxi)&&
																Control_curve_get_element_parameter_change(
																	curve,curve_editor->current_element_no,
																	&parameter_change))
															{
																/* multiply by dS_dxi to get dx_dxi vector */
																for (i=0;i<number_of_components;i++)
																{
																	derivatives[i] *= dS_dxi;
																}
																/* plug in new dx_dxi for shifted control point */
																derivatives[curve_editor->current_component_no]=
																	velocity*parameter_change;
																Control_curve_unitize_vector(derivatives,
																	number_of_components,&dS_dxi);
																Control_curve_set_node_derivatives(curve,
																	curve_editor->current_element_no,
																	curve_editor->current_node_no,derivatives);
																Control_curve_set_scale_factor(curve,
																	curve_editor->current_element_no,
																	curve_editor->current_node_no,dS_dxi);
																/* enforce continuity at node */
																Control_curve_enforce_continuity(curve,
																	curve_editor->current_element_no,
																	curve_editor->current_node_no,0,
																	CONTROL_CURVE_CONTINUITY_SLOPE);
															}
														}
													}
													control_curve_editor_draw_component(curve_editor,
														curve_editor->current_component_no,1);
													if (values)
													{
														DEALLOCATE(values);
													}
													if (derivatives)
													{
														DEALLOCATE(derivatives);
													}
												}
											} break;
											case MOVING_NODE:
											{
												control_curve_editor_drawing_area_redraw(curve_editor,1);
												if (ALLOCATE(values,FE_value,
													Control_curve_get_number_of_components(curve)))
												{
													if (Control_curve_get_node_values(curve,
														curve_editor->current_element_no,
														curve_editor->current_node_no,values))
													{
														get_drawing_scale_factors_y(curve_editor,
															curve_editor->current_component_no,&ymin,&yscale,
															&ystartvalue);
														y=pointer_y;
														/* make sure y is inside component box */
														if ((y < (limit=ymin-curve_editor->comp_box_height))||
															(y > (limit=ymin+1)))
														{
															y=limit;
														}
														comp_value=(y-ymin)/yscale+ystartvalue;
														control_curve_editor_snap_value(curve_editor,
															curve_editor->current_component_no,&comp_value);
														values[curve_editor->current_component_no]=comp_value;
														Control_curve_set_node_values(curve,
															curve_editor->current_element_no,
															curve_editor->current_node_no,values);
													}
													if (Control_curve_is_node_parameter_modifiable(curve,
														curve_editor->current_node_no))
													{
														/* cubic Hermite: code to keep velocity continuous */
														if (0<Control_curve_get_derivatives_per_node(curve))
														{
															first=curve_editor->current_element_no;
															last=first;
															if ((0==curve_editor->current_node_no)&&(1<first))
															{
																first--;
															}
															if (((Control_curve_get_nodes_per_element(curve)-1)==
																curve_editor->current_node_no)&&
																(last<Control_curve_get_number_of_elements(curve)))
															{
																last++;
															}
															for (elem_no=first;elem_no<=last;elem_no++)
															{
																Control_curve_get_node_scale_factor_dparameter(
																	curve,elem_no,0,&(dS_dxi_dt[elem_no-first][0]));
																Control_curve_get_node_scale_factor_dparameter(
																	curve,elem_no,1,&(dS_dxi_dt[elem_no-first][1]));
															}
														}
														get_drawing_scale_factors_x(curve_editor,
															&xmin,&xscale,&xstartvalue);
														x=pointer_x;
														/* make sure x is inside component box */
														if ((x > (limit=xmin+curve_editor->comp_box_width-1))||
															(x < (limit=xmin)))
														{
															x=limit;
														}
														parameter=(x-xmin)/xscale+xstartvalue;
														control_curve_editor_snap_parameter(curve_editor,
															&parameter);
														/* limit parameter to range of times of neighbour elements */
														if (0==curve_editor->current_node_no)
														{
															left_element_no=curve_editor->current_element_no-1;
														}
														else
														{
															left_element_no=curve_editor->current_element_no;
														}
														right_element_no = left_element_no+1;
														if (0<left_element_no)
														{
															if (Control_curve_get_parameter(curve,left_element_no,
																0,&left_parameter))
															{
																if (parameter<left_parameter)
																{
																	parameter=left_parameter;
																}
															}
														}
														if (right_element_no <=
															Control_curve_get_number_of_elements(curve))
														{
															if (Control_curve_get_parameter(curve,right_element_no,
																Control_curve_get_nodes_per_element(curve)-1,
																&right_parameter))
															{
																if (parameter>right_parameter)
																{
																	parameter=right_parameter;
																}
															}
														}
														Control_curve_set_parameter(curve,
															curve_editor->current_element_no,
															curve_editor->current_node_no,parameter);
														/* cubic Hermite:code to keep velocity continuous */
														if (0<Control_curve_get_derivatives_per_node(curve))
														{
															for (elem_no=first;elem_no<=last;elem_no++)
															{
																if (Control_curve_get_element_parameter_change(
																	curve,elem_no,&parameter_change))
																{
																	Control_curve_set_scale_factor(curve,elem_no,
																		0,parameter_change*dS_dxi_dt[elem_no-first][0]);
																	Control_curve_set_scale_factor(curve,elem_no,
																		1,parameter_change*dS_dxi_dt[elem_no-first][1]);
																}
															}
														}
													}
													DEALLOCATE(values);
												}
												control_curve_editor_drawing_area_redraw(curve_editor,1);
											} break;
										}
									} break;
									case ButtonPress:
									{
										if (xevent.xbutton.button==working_button)
										{
											display_message(ERROR_MESSAGE,
												"control_curve_editor_drawing_area_input_CB.  "
												"Unexpected button press");
											moving=MOVING_NONE;
										}
									} break;
									case ButtonRelease:
									{
										if (xevent.xbutton.button==working_button)
										{
											if (xevent.xbutton.window==working_window)
											{
												control_curve_editor_drawing_area_redraw(curve_editor,0);
												/* inform the client of the change */
												control_curve_editor_update(curve_editor);
											}
											moving=MOVING_NONE;
										}
									} break;
									default:
									{
										XtDispatchEvent(&xevent);
									}
								}
							}
							/* release the pointer */
							XtUngrabPointer(curve_editor->drawing_area,CurrentTime);
						}
						XFreeCursor(display,cursor);
					}
				}
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_drawing_area_input_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* control_curve_editor_drawing_area_input_CB */

static void control_curve_editor_update_basis_type(Widget widget,
	void *control_curve_editor_void,void *fe_basis_type_string_void)
/*******************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
Callback for change of basis_type.
==============================================================================*/
{
	enum FE_basis_type fe_basis_type;
	struct Control_curve *curve;
	struct Control_curve_editor *curve_editor;

	ENTER(control_curve_editor_update_basis_type);
	USE_PARAMETER(widget);
	if ((curve_editor=(struct Control_curve_editor *)control_curve_editor_void)&&
		(curve=curve_editor->edit_curve))
	{
		if ((FE_BASIS_TYPE_INVALID != (fe_basis_type=
			Control_curve_FE_basis_type_from_string(
				(char *)fe_basis_type_string_void)))&&
			(Control_curve_get_fe_basis_type(curve) != fe_basis_type))
		{
			/* Ensure no node selected since may not exist under new basis */
			curve_editor->current_element_no=0;
			curve_editor->current_node_no=-1;
			curve_editor->current_component_no=-1;
			if (Control_curve_set_fe_basis_type(curve,fe_basis_type))
			{
				control_curve_editor_drawing_area_redraw(curve_editor,0);
				/* inform the client of the change */
				control_curve_editor_update(curve_editor);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"control_curve_editor_update_basis_type.  Could not set basis type");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_update_basis_type.  Invalid argument(s)");
	}
	LEAVE;
} /* control_curve_editor_update_basis_type */

static int control_curve_editor_set_range_component_no(
	struct Control_curve_editor *curve_editor,int component_no)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Selects the current component, and displays the ranges for that component.
==============================================================================*/
{
	char temp_string[30];
	FE_value max_range,min_range;
	int return_code;
	struct Control_curve *curve;

	ENTER(control_curve_editor_set_range_component_no);
	if (curve_editor&&(curve=curve_editor->edit_curve))
	{
		if ((0<=component_no)&&
			(Control_curve_get_number_of_components(curve)>component_no))
		{
			return_code=1;
			if (component_no != curve_editor->range_component_no)
			{
				if (return_code=choose_field_component_set_field_component(
					curve_editor->component_widget,Control_curve_get_value_field(curve),
					component_no))
				{
					curve_editor->range_component_no=component_no;
				}
			}
			/* display the component ranges */
			if (Control_curve_get_edit_component_range(curve,
				curve_editor->range_component_no,&min_range,&max_range))
			{
				sprintf(temp_string,"%g",min_range);
				XtVaSetValues(curve_editor->min_comp_text,XmNvalue,temp_string,NULL);
				sprintf(temp_string,"%g",max_range);
				XtVaSetValues(curve_editor->max_comp_text,XmNvalue,temp_string,NULL);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"curve_set_range_component_no.  Invalid component number");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"curve_set_range_component_no.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* curve_set_range_component_no */

static void control_curve_editor_num_components_text_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 13 October 1997

DESCRIPTION :
Callback for changing the number of components in the curve.
==============================================================================*/
{
	struct Control_curve_editor *curve_editor;
	char *num_comp_string,temp_string[30];
	int new_number_of_components;

	ENTER(control_curve_editor_num_components_text_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* Get the pointer to the text field user data and text */
		XtVaGetValues(widget,XmNuserData,&curve_editor,
			XmNvalue,&num_comp_string,NULL);
		if (curve_editor&&curve_editor->edit_curve&&num_comp_string)
		{
			new_number_of_components=atoi(num_comp_string);
			/*???RC do not like maximum being hardcoded like this */
			if ((0<new_number_of_components) && (25 >= new_number_of_components))
			{
				if (new_number_of_components !=
					Control_curve_get_number_of_components(curve_editor->edit_curve))
				{
					if (Control_curve_set_number_of_components(curve_editor->edit_curve,
						new_number_of_components))
					{
						curve_editor->range_component_no=-1;
						control_curve_editor_set_range_component_no(curve_editor,0);
						control_curve_editor_drawing_layout_change(curve_editor);
						control_curve_editor_view_full_range(curve_editor);
						control_curve_editor_drawing_area_redraw(curve_editor,0);
						/* inform the client of the change */
						control_curve_editor_update(curve_editor);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Number of components must be from 1 to 25");
			}
			/* redisplay the parameter range in the widgets */
			sprintf(temp_string,"%i",
				Control_curve_get_number_of_components(curve_editor->edit_curve));
			XtVaSetValues(curve_editor->num_components_text,XmNvalue,temp_string,NULL);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_num_components_text_CB.  Missing widget data");
		}
		if (num_comp_string)
		{
			XtFree(num_comp_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_num_components_text_CB.  Missing widget");
	}
	LEAVE;
} /* control_curve_editor_num_components_text_CB */

static void control_curve_editor_change_component(
	Widget control_curve_editor_widget,
	void *curve_editor_void,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 9 November 1999

DESCRIPTION :
Callback for change of field component for selecting ranges.
???RC access/deaccess not used here
==============================================================================*/
{
	struct FE_field *field;
	int component_no;
	struct Control_curve_editor *curve_editor;

	ENTER(control_curve_editor_change_component);
	USE_PARAMETER(control_curve_editor_widget);
	USE_PARAMETER(dummy_void);
	if (curve_editor=(struct Control_curve_editor *)curve_editor_void)
	{
		if (choose_field_component_get_field_component(
			curve_editor->component_widget,&field,&component_no))
		{
			if (field)
			{
				if ((0<=component_no)&&
					(component_no<get_FE_field_number_of_components(field)))
				{
					if (component_no != curve_editor->range_component_no)
					{
						curve_editor->range_component_no=component_no;
						control_curve_editor_set_range_component_no(curve_editor,
							component_no);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"control_curve_editor_change_component.  Invalid component number");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"control_curve_editor_change_component.  Missing field");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_change_component.  Invalid argument(s)");
	}
	LEAVE;
} /* control_curve_editor_change_component */

static void control_curve_editor_min_component_text_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Callback for changing the minimum range of the current component.
==============================================================================*/
{
	struct Control_curve_editor *curve_editor;
	char *comp_string;
	FE_value min_range,current_max_range,current_min_range, extra_range;
	struct Control_curve *curve;

	ENTER(control_curve_editor_min_component_text_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* Get the pointer to the text field user data and text */
		XtVaGetValues(widget,XmNuserData,&curve_editor,XmNvalue,&comp_string,NULL);
		if (curve_editor&&(curve=curve_editor->edit_curve)&&comp_string)
		{
			if (Control_curve_get_edit_component_range(curve,
				curve_editor->range_component_no,&current_min_range,&current_max_range))
			{
				if ((min_range=atof(comp_string)) != current_min_range)
				{
					if (current_max_range < min_range)
					{
						current_max_range=min_range;
					}
					if (Control_curve_set_edit_component_range(curve,
						curve_editor->range_component_no,min_range,current_max_range))
					{
						check_range(&current_max_range,&min_range);
						extra_range=(current_max_range-min_range)*
							curve_editor->curve_drawing_information->extra_component_range;
						curve_editor->view_comp_min[curve_editor->range_component_no]=min_range-extra_range;
						curve_editor->view_comp_max[curve_editor->range_component_no]=current_max_range+extra_range;
						control_curve_editor_drawing_area_redraw(curve_editor,0);
						/* inform the client of the change */
						control_curve_editor_update(curve_editor);
					}
				}
			}
			control_curve_editor_set_range_component_no(curve_editor,
				curve_editor->range_component_no);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_min_component_text_CB.  Missing widget data");
		}
		if (comp_string)
		{
			XtFree(comp_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_min_component_text_CB.  Missing widget");
	}
	LEAVE;
} /* control_curve_editor_min_component_text_CB */

static void control_curve_editor_max_component_text_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Callback for changing the maximum range of the current component.
==============================================================================*/
{
	struct Control_curve_editor *curve_editor;
	char *comp_string;
	FE_value max_range,current_max_range,current_min_range, extra_range;
	struct Control_curve *curve;

	ENTER(control_curve_editor_min_component_text_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* Get the pointer to the text field user data and text */
		XtVaGetValues(widget,XmNuserData,&curve_editor,XmNvalue,&comp_string,NULL);
		if (curve_editor&&(curve=curve_editor->edit_curve)&&comp_string)
		{
			if (Control_curve_get_edit_component_range(curve,
				curve_editor->range_component_no,&current_min_range,&current_max_range))
			{
				if ((max_range=atof(comp_string)) != current_max_range)
				{
					if (current_min_range > max_range)
					{
						current_min_range=max_range;
					}
					if (Control_curve_set_edit_component_range(curve,
						curve_editor->range_component_no,current_min_range,max_range))
					{
						check_range(&max_range,&current_min_range);
						extra_range=(max_range-current_min_range)*
							curve_editor->curve_drawing_information->extra_component_range;
						curve_editor->view_comp_min[curve_editor->range_component_no]=current_min_range-extra_range;
						curve_editor->view_comp_max[curve_editor->range_component_no]=max_range+extra_range;
						control_curve_editor_drawing_area_redraw(curve_editor,0);
						/* inform the client of the change */
						control_curve_editor_update(curve_editor);
					}
				}
			}
			control_curve_editor_set_range_component_no(curve_editor,
				curve_editor->range_component_no);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_min_component_text_CB.  Missing widget data");
		}
		if (comp_string)
		{
			XtFree(comp_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_min_component_text_CB.  Missing widget");
	}
	LEAVE;
} /* control_curve_editor_max_component_text_CB */

static void control_curve_editor_update_extend_mode(Widget widget,
	void *control_curve_editor_void,void *extend_mode_string_void)
/*******************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
Callback for change of extend_mode.
==============================================================================*/
{
	enum Control_curve_extend_mode extend_mode;
	struct Control_curve *curve;
	struct Control_curve_editor *curve_editor;

	ENTER(control_curve_editor_update_extend_mode);
	USE_PARAMETER(widget);
	if ((curve_editor=(struct Control_curve_editor *)control_curve_editor_void)&&
		(curve=curve_editor->edit_curve))
	{
		if ((CONTROL_CURVE_EXTEND_MODE_INVALID != (extend_mode=
			Control_curve_extend_mode_from_string((char *)extend_mode_string_void)))&&
			(Control_curve_get_extend_mode(curve) != extend_mode))
		{
			if (Control_curve_set_extend_mode(curve,extend_mode))
			{
				control_curve_editor_update(curve_editor);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"control_curve_editor_update_extend_mode.  "
					"Could not set new extend mode");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_update_extend_mode.  Invalid argument(s)");
	}
	LEAVE;
} /* control_curve_editor_update_extend_mode */

FE_value control_curve_editor_get_cursor_parameter(
	Widget control_curve_editor_widget)
/*******************************************************************************
LAST MODIFIED : 16 April 1998

DESCRIPTION :
Gets the current position of the parameter cursor.
==============================================================================*/
{
	FE_value return_code;
	struct Control_curve_editor *curve_editor;

	ENTER(control_curve_editor_get_cursor_parameter);

	XtVaGetValues( control_curve_editor_widget,
		XmNuserData, &curve_editor,
		NULL );

	if (curve_editor)
	{
		return_code = curve_editor->cursor;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_get_cursor_parameter.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* control_curve_editor_get_cursor_parameter */

int control_curve_editor_set_cursor_parameter(
	Widget control_curve_editor_widget, FE_value parameter)
/*******************************************************************************
LAST MODIFIED : 16 April 1998

DESCRIPTION :
Sets the current position of the parameter cursor and displays it if it isn't
already shown.
==============================================================================*/
{
	int return_code;
	struct Control_curve_editor *curve_editor;

	ENTER(control_curve_editor_set_cursor_parameter);

	XtVaGetValues( control_curve_editor_widget,
		XmNuserData, &curve_editor,
		NULL );

	if (curve_editor)
	{
		curve_editor->cursor = parameter;
		curve_editor->cursor_displayed = 1;
		control_curve_editor_drawing_area_redraw(curve_editor,0);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_set_cursor_parameter.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* control_curve_editor_set_cursor_parameter */

static void control_curve_editor_component_grid_text_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Callback for changing the component grid size stored with the curve.
==============================================================================*/
{
	struct Control_curve_editor *curve_editor;
	char *size_string,temp_string[30];
	FE_value component_grid_size,current_component_grid_size;
	struct Control_curve *curve;

	ENTER(control_curve_editor_component_grid_text_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* Get the pointer to the text field user data and text */
		XtVaGetValues(widget,XmNuserData,&curve_editor,XmNvalue,&size_string,NULL);
		if (curve_editor&&(curve=curve_editor->edit_curve)&&size_string)
		{
			if (Control_curve_get_value_grid(curve,&current_component_grid_size))
			{
				if ((component_grid_size=atof(size_string)) !=
					current_component_grid_size)
				{
					if (0 <= component_grid_size)
					{
						if (Control_curve_set_value_grid(curve,component_grid_size))
						{
							current_component_grid_size=component_grid_size;
						}
						control_curve_editor_drawing_area_redraw(curve_editor,0);
						/* inform the client of the change */
						control_curve_editor_update(curve_editor);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Grid size must be >= 0");
					}
				}
				/* redisplay the component range in the widgets */
				sprintf(temp_string,"%g",current_component_grid_size);
				XtVaSetValues(curve_editor->comp_grid_text,
					XmNvalue,temp_string,NULL);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_component_grid_text_CB.  Missing widget data");
		}
		if (size_string)
		{
			XtFree(size_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_component_grid_text_CB.  Missing widget");
	}
	LEAVE;
} /* control_curve_editor_component_grid_text_CB */

static void control_curve_editor_parameter_grid_text_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Callback for changing the parameter grid size stored with the curve.
==============================================================================*/
{
	struct Control_curve_editor *curve_editor;
	char *size_string,temp_string[30];
	FE_value parameter_grid_size,current_parameter_grid_size;
	struct Control_curve *curve;

	ENTER(control_curve_editor_parameter_grid_text_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* Get the pointer to the text field user data and text */
		XtVaGetValues(widget,XmNuserData,&curve_editor,XmNvalue,&size_string,NULL);
		if (curve_editor&&(curve=curve_editor->edit_curve)&&size_string)
		{
			if (Control_curve_get_parameter_grid(curve,&current_parameter_grid_size))
			{
				if ((parameter_grid_size=atof(size_string)) != current_parameter_grid_size)
				{
					if (0 <= parameter_grid_size)
					{
						if (Control_curve_set_parameter_grid(curve,parameter_grid_size))
						{
							current_parameter_grid_size=parameter_grid_size;
						}
						control_curve_editor_drawing_area_redraw(curve_editor,0);
						/* inform the client of the change */
						control_curve_editor_update(curve_editor);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Grid size must be >= 0");
					}
				}
				/* redisplay the parameter range in the widgets */
				sprintf(temp_string,"%g",current_parameter_grid_size);
				XtVaSetValues(curve_editor->parameter_grid_text,XmNvalue,temp_string,NULL);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_parameter_grid_text_CB.  Missing widget data");
		}
		if (size_string)
		{
			XtFree(size_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_parameter_grid_text_CB.  Missing widget");
	}
	LEAVE;
} /* control_curve_editor_parameter_grid_text_CB */

static void control_curve_editor_full_range_button_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 7 October 1997

DESCRIPTION :
Callback for zooming to show full range of components and parameters in curve.
==============================================================================*/
{
	struct Control_curve_editor *curve_editor;

	ENTER(control_curve_editor_full_range_button_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* Get the pointer to the data for the widget */
		XtVaGetValues(widget,XmNuserData,&curve_editor,NULL);
		if (curve_editor)
		{
			control_curve_editor_view_full_range(curve_editor);
			control_curve_editor_drawing_area_redraw(curve_editor,0);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_full_range_button_CB.  Missing widget data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_full_range_button_CB.  Missing widget");
	}
	LEAVE;
} /* control_curve_editor_full_range_button_CB */

static void control_curve_editor_min_parameter_text_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 7 October 1997

DESCRIPTION :
Callback for changing the minimum parameter shown in the drawing area.
==============================================================================*/
{
	struct Control_curve_editor *curve_editor;
	char *parameter_string,temp_string[30];
	FE_value min_parameter,max_parameter;

	ENTER(control_curve_editor_min_parameter_text_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* Get the pointer to the text field user data and text */
		XtVaGetValues(widget,XmNuserData,&curve_editor,XmNvalue,&parameter_string,NULL);
		if (curve_editor&&parameter_string)
		{
			if ((min_parameter=atof(parameter_string)) != curve_editor->view_parameter_min)
			{
				max_parameter=curve_editor->view_parameter_max;
				if (max_parameter <= min_parameter)
				{
					/* just shift the current range */
					max_parameter=min_parameter+curve_editor->view_parameter_max-curve_editor->view_parameter_min;
				}
				/* set the new parameter range and redraw */
				curve_editor->view_parameter_min=min_parameter;
				curve_editor->view_parameter_max=max_parameter;
				control_curve_editor_drawing_area_redraw(curve_editor,0);
			}
			/* redisplay the parameter range in the widgets */
			sprintf(temp_string,"%g",curve_editor->view_parameter_min);
			XtVaSetValues(curve_editor->min_parameter_text,XmNvalue,temp_string,NULL);
			sprintf(temp_string,"%g",curve_editor->view_parameter_max);
			XtVaSetValues(curve_editor->max_parameter_text,XmNvalue,temp_string,NULL);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_min_parameter_text_CB.  Missing widget data");
		}
		if (parameter_string)
		{
			XtFree(parameter_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_min_parameter_text_CB.  Missing widget");
	}
	LEAVE;
} /* control_curve_editor_min_parameter_text_CB */

static void control_curve_editor_max_parameter_text_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 7 October 1997

DESCRIPTION :
Callback for changing the maximum parameter shown in the drawing area.
==============================================================================*/
{
	struct Control_curve_editor *curve_editor;
	char *parameter_string,temp_string[30];
	FE_value min_parameter,max_parameter;

	ENTER(control_curve_editor_max_parameter_text_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* Get the pointer to the text field user data and text */
		XtVaGetValues(widget,XmNuserData,&curve_editor,XmNvalue,&parameter_string,NULL);
		if (curve_editor&&parameter_string)
		{
			if ((max_parameter=atof(parameter_string)) != curve_editor->view_parameter_max)
			{
				min_parameter=curve_editor->view_parameter_min;
				if (min_parameter >= max_parameter)
				{
					min_parameter=max_parameter-curve_editor->view_parameter_max+curve_editor->view_parameter_min;
				}
				/* set the new parameter range and redraw */
				curve_editor->view_parameter_min=min_parameter;
				curve_editor->view_parameter_max=max_parameter;
				control_curve_editor_drawing_area_redraw(curve_editor,0);
			}
			/* redisplay the parameter range in the widgets */
			sprintf(temp_string,"%g",curve_editor->view_parameter_min);
			XtVaSetValues(curve_editor->min_parameter_text,XmNvalue,temp_string,NULL);
			sprintf(temp_string,"%g",curve_editor->view_parameter_max);
			XtVaSetValues(curve_editor->max_parameter_text,XmNvalue,temp_string,NULL);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_max_parameter_text_CB.  Missing widget data");
		}
		if (parameter_string)
		{
			XtFree(parameter_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_max_parameter_text_CB.  Missing widget");
	}
	LEAVE;
} /* control_curve_editor_max_parameter_text_CB */

static void control_curve_editor_snap_value_button_CB(Widget widget,
	int *tag,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Callback for toggling snap on/off for component axes.
==============================================================================*/
{
	struct Control_curve_editor *curve_editor;

	ENTER(control_curve_editor_snap_value_button_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* Get the pointer to the widget data */
		XtVaGetValues(widget,XmNuserData,&curve_editor,NULL);
		if (curve_editor)
		{
			curve_editor->snap_value=
				XmToggleButtonGadgetGetState(curve_editor->snap_value_button);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_snap_value_button_CB.  Missing widget data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_snap_value_button_CB.  Missing widget");
	}
	LEAVE;
} /* control_curve_editor_snap_value_button_CB */

static void control_curve_editor_snap_parameter_button_CB(Widget widget,
	int *tag,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Callback for toggling snap on/off for the parameter axis.
==============================================================================*/
{
	struct Control_curve_editor *curve_editor;

	ENTER(control_curve_editor_snap_parameter_button_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* Get the pointer to the widget data */
		XtVaGetValues(widget,XmNuserData,&curve_editor,NULL);
		if (curve_editor)
		{
			curve_editor->snap_parameter=
				XmToggleButtonGadgetGetState(curve_editor->snap_parameter_button);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_snap_parameter_button_CB.  Missing widget data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_snap_parameter_button_CB.  Missing widget");
	}
	LEAVE;
} /* control_curve_editor_snap_parameter_button_CB */

static void control_curve_editor_max_components_shown_text_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 16 October 1997

DESCRIPTION :
Callback for changing the maximum parameter shown in the drawing area.
==============================================================================*/
{
	char *value_string,temp_string[20];
	int max_components_shown;
	struct Control_curve_editor *curve_editor;

	ENTER(control_curve_editor_max_components_shown_text_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* Get the pointer to the text field user data and text */
		XtVaGetValues(widget,XmNuserData,&curve_editor,XmNvalue,&value_string,NULL);
		if (curve_editor&&value_string)
		{
			max_components_shown=atoi(value_string);
			if (1>max_components_shown)
			{
				max_components_shown=1;
			}
			if (max_components_shown != curve_editor->max_components_in_view)
			{
				curve_editor->max_components_in_view=max_components_shown;
				control_curve_editor_drawing_layout_change(curve_editor);
				control_curve_editor_drawing_area_redraw(curve_editor,0);
			}
			/* redisplay the max number of components in view */
			sprintf(temp_string,"%i",curve_editor->max_components_in_view);
			XtVaSetValues(curve_editor->comps_shown_text,XmNvalue,temp_string,NULL);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_max_components_shown_text_CB.  Missing widget data");
		}
		if (value_string)
		{
			XtFree(value_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_max_components_shown_text_CB.  Missing widget");
	}
	LEAVE;
} /* control_curve_editor_max_components_shown_text_CB */

static void control_curve_editor_up_button_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 15 October 1997

DESCRIPTION :
Callback for scrolling up to see earlier component numbers.
==============================================================================*/
{
	struct Control_curve_editor *curve_editor;

	ENTER(control_curve_editor_up_button_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* Get the pointer to the widget data */
		XtVaGetValues(widget,XmNuserData,&curve_editor,NULL);
		if (curve_editor)
		{
			if (0<curve_editor->first_component_in_view)
			{
				curve_editor->first_component_in_view--;
				control_curve_editor_drawing_layout_change(curve_editor);
				control_curve_editor_drawing_area_redraw(curve_editor,0);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_up_button_CB.  Missing widget data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_up_button_CB.  Missing widget");
	}
	LEAVE;
} /* control_curve_editor_up_button_CB */

static void control_curve_editor_down_button_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 15 October 1997

DESCRIPTION :
Callback for scrolling down to see later component numbers.
==============================================================================*/
{
	struct Control_curve_editor *curve_editor;
	struct Control_curve *curve;

	ENTER(control_curve_editor_down_button_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* Get the pointer to the widget data */
		XtVaGetValues(widget,XmNuserData,&curve_editor,NULL);
		if (curve_editor&&(curve=curve_editor->edit_curve))
		{
			if (curve_editor->first_component_in_view<
				(Control_curve_get_number_of_components(curve)-
				curve_editor->max_components_in_view))
			{
				curve_editor->first_component_in_view++;
				control_curve_editor_drawing_layout_change(curve_editor);
				control_curve_editor_drawing_area_redraw(curve_editor,0);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_down_button_CB.  Missing widget data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_down_button_CB.  Missing widget");
	}
	LEAVE;
} /* control_curve_editor_down_button_CB */

/*
Global functions
----------------
*/
Widget create_control_curve_editor_widget(Widget *curve_editor_widget,
	Widget parent,struct Control_curve *curve,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
Creates a control_curve_editor widget.
==============================================================================*/
{
	char temp_string[30],**valid_strings;
	int number_of_valid_strings,init_widgets;
	MrmType control_curve_editor_dialog_class;
	struct Control_curve_editor *curve_editor=NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"curve_ed_destroy_CB",(XtPointer)control_curve_editor_destroy_CB},
		{"curve_ed_id_drawing_area",(XtPointer)
			DIALOG_IDENTIFY(control_curve_editor,drawing_area)},
		{"curve_ed_id_basis_type_form",(XtPointer)
			DIALOG_IDENTIFY(control_curve_editor,basis_type_form)},
		{"curve_ed_id_num_components_text",(XtPointer)
			DIALOG_IDENTIFY(control_curve_editor,num_components_text)},
		{"curve_ed_id_component_form",(XtPointer)
			DIALOG_IDENTIFY(control_curve_editor,component_form)},
		{"curve_ed_id_min_comp_text",(XtPointer)
			DIALOG_IDENTIFY(control_curve_editor,min_comp_text)},
		{"curve_ed_id_max_comp_text",(XtPointer)
			DIALOG_IDENTIFY(control_curve_editor,max_comp_text)},
		{"curve_ed_id_extend_mode_form",(XtPointer)
			DIALOG_IDENTIFY(control_curve_editor,extend_mode_form)},
		{"curve_ed_id_comp_grid_text",(XtPointer)
			DIALOG_IDENTIFY(control_curve_editor,comp_grid_text)},
		{"curve_ed_id_parameter_grid_text",(XtPointer)
			DIALOG_IDENTIFY(control_curve_editor,parameter_grid_text)},
		{"curve_ed_id_full_range_btn",(XtPointer)
			DIALOG_IDENTIFY(control_curve_editor,full_range_button)},
		{"curve_ed_id_min_parameter_text",(XtPointer)
			DIALOG_IDENTIFY(control_curve_editor,min_parameter_text)},
		{"curve_ed_id_max_parameter_text",(XtPointer)
			DIALOG_IDENTIFY(control_curve_editor,max_parameter_text)},
		{"curve_ed_id_snap_value_btn",(XtPointer)
			DIALOG_IDENTIFY(control_curve_editor,snap_value_button)},
		{"curve_ed_id_snap_parameter_btn",(XtPointer)
			DIALOG_IDENTIFY(control_curve_editor,snap_parameter_button)},
		{"curve_ed_id_comps_shown_text",(XtPointer)
			DIALOG_IDENTIFY(control_curve_editor,comps_shown_text)},
		{"curve_ed_id_up_btn",(XtPointer)
			DIALOG_IDENTIFY(control_curve_editor,up_button)},
		{"curve_ed_id_down_btn",(XtPointer)
			DIALOG_IDENTIFY(control_curve_editor,down_button)},
		{"curve_ed_expose_drawing_area_CB",(XtPointer)
			control_curve_editor_expose_drawing_area_CB},
		{"curve_ed_resize_drawing_area_CB",(XtPointer)
			control_curve_editor_resize_drawing_area_CB},
		{"curve_ed_drawing_area_input_CB",(XtPointer)
			control_curve_editor_drawing_area_input_CB},
		{"curve_ed_num_components_text_CB",(XtPointer)
			control_curve_editor_num_components_text_CB},
		{"curve_ed_min_comp_text_CB",(XtPointer)
			control_curve_editor_min_component_text_CB},
		{"curve_ed_max_comp_text_CB",(XtPointer)
			control_curve_editor_max_component_text_CB},
		{"curve_ed_comp_grid_text_CB",(XtPointer)
			control_curve_editor_component_grid_text_CB},
		{"curve_ed_parameter_grid_text_CB",(XtPointer)
			control_curve_editor_parameter_grid_text_CB},
		{"curve_ed_full_range_btn_CB",(XtPointer)
			control_curve_editor_full_range_button_CB},
		{"curve_ed_min_parameter_text_CB",(XtPointer)
			control_curve_editor_min_parameter_text_CB},
		{"curve_ed_max_parameter_text_CB",(XtPointer)
			control_curve_editor_max_parameter_text_CB},
		{"curve_ed_snap_value_btn_CB",(XtPointer)
			control_curve_editor_snap_value_button_CB},
		{"curve_ed_snap_parameter_btn_CB",(XtPointer)
			control_curve_editor_snap_parameter_button_CB},
		{"curve_ed_comps_shown_text_CB",(XtPointer)
			control_curve_editor_max_components_shown_text_CB},
		{"curve_ed_up_btn_CB",(XtPointer)
			control_curve_editor_up_button_CB},
		{"curve_ed_down_btn_CB",(XtPointer)
			control_curve_editor_down_button_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"curve_ed_structure",(XtPointer)NULL}
	};
	Widget return_widget;
#if defined (OLD_CODE)
/*???RC temporary*/
	XColor screen_def_return,exact_def_return;
#endif /* defined (OLD_CODE) */

	ENTER(create_control_curve_editor_widget);
	return_widget=(Widget)NULL;
	if (curve_editor_widget&&parent&&user_interface)
	{
		if (MrmOpenHierarchy_base64_string(control_curve_editor_uidh,
			&curve_editor_hierarchy,&curve_editor_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(curve_editor,struct Control_curve_editor,1))
			{
				/* initialise the structure */
				curve_editor->edit_curve=(struct Control_curve *)NULL;
				curve_editor->user_interface=user_interface;
				curve_editor->widget_parent=parent;
				curve_editor->widget_address=curve_editor_widget;
				curve_editor->widget=(Widget)NULL;
				curve_editor->drawing_area=(Widget)NULL;
				curve_editor->basis_type_form=(Widget)NULL;
				curve_editor->basis_type_widget=(Widget)NULL;
				curve_editor->extend_mode_form=(Widget)NULL;
				curve_editor->extend_mode_widget=(Widget)NULL;
				curve_editor->num_components_text=(Widget)NULL;
				curve_editor->component_form=(Widget)NULL;
				curve_editor->component_widget=(Widget)NULL;
				curve_editor->full_range_button=(Widget)NULL;
				curve_editor->min_parameter_text=(Widget)NULL;
				curve_editor->max_parameter_text=(Widget)NULL;
				curve_editor->min_comp_text=(Widget)NULL;
				curve_editor->max_comp_text=(Widget)NULL;
				curve_editor->comp_grid_text=(Widget)NULL;
				curve_editor->parameter_grid_text=(Widget)NULL;
				curve_editor->comps_shown_text=(Widget)NULL;
				curve_editor->snap_value_button=(Widget)NULL;
				curve_editor->snap_parameter_button=(Widget)NULL;
				curve_editor->up_button=(Widget)NULL;
				curve_editor->down_button=(Widget)NULL;
				curve_editor->update_callback.procedure=(Callback_procedure *)NULL;
				curve_editor->update_callback.data=(void *)NULL;
				curve_editor->curve_drawing_information=
					(struct Control_curve_drawing_information *)NULL;
				curve_editor->current_element_no=0;
				curve_editor->current_node_no=-1;
				curve_editor->current_component_no=-1;
				curve_editor->range_component_no=-1;
				curve_editor->components_in_view=1;
				curve_editor->max_components_in_view=3;
				curve_editor->first_component_in_view=0;
				curve_editor->max_number_of_components=0;
				curve_editor->view_comp_min=(FE_value *)NULL;
				curve_editor->view_comp_max=(FE_value *)NULL;
				curve_editor->view_parameter_min=0.0;
				curve_editor->view_parameter_max=0.0;
				curve_editor->cursor = 0;
				curve_editor->cursor_displayed = 0;
				curve_editor->snap_parameter=0;
				curve_editor->snap_value=0;
				/* register the callbacks */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
					curve_editor_hierarchy,callback_list,XtNumber(callback_list)))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)curve_editor;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						curve_editor_hierarchy,identifier_list,XtNumber(identifier_list)))
					{
						/* fetch graphical element editor widget */
						if (MrmSUCCESS==MrmFetchWidget(curve_editor_hierarchy,
							"curve_ed_widget",curve_editor->widget_parent,
							&(curve_editor->widget),&control_curve_editor_dialog_class))
						{
							init_widgets=1;
							if (!(curve_editor->curve_drawing_information=
								create_Control_curve_drawing_information(user_interface)))
							{
								init_widgets=0;
							}
							else
							{
								/* set dialog widget values (ie. not part of curve) */
								XtVaSetValues(curve_editor->drawing_area,XmNbackground,
									curve_editor->curve_drawing_information->background_colour,
									NULL);
								sprintf(temp_string,"%i",curve_editor->max_components_in_view);
								XtVaSetValues(curve_editor->comps_shown_text,
									XmNvalue,temp_string,NULL);
								XtVaSetValues(curve_editor->snap_value_button,
									XmNset,0,NULL);
								XtVaSetValues(curve_editor->snap_value_button,
									XmNset,curve_editor->snap_value,NULL);
								XtVaSetValues(curve_editor->snap_parameter_button,
									XmNset,0,NULL);
								XtVaSetValues(curve_editor->snap_parameter_button,
									XmNset,curve_editor->snap_parameter,NULL);
							}
							valid_strings=Control_curve_FE_basis_type_get_valid_strings(
								&number_of_valid_strings);
							if (!(curve_editor->basis_type_widget=
								create_choose_enumerator_widget(curve_editor->basis_type_form,
									number_of_valid_strings,valid_strings,
									FE_basis_type_string(LINEAR_LAGRANGE))))
							{
								init_widgets=0;
							}
							DEALLOCATE(valid_strings);
							if (!(curve_editor->component_widget=
								create_choose_field_component_widget(
									curve_editor->component_form,(struct FE_field *)NULL,0)))
							{
								init_widgets=0;
							}
							valid_strings=Control_curve_extend_mode_get_valid_strings(
								&number_of_valid_strings);
							if (!(curve_editor->extend_mode_widget=
								create_choose_enumerator_widget(curve_editor->extend_mode_form,
									number_of_valid_strings,valid_strings,
									Control_curve_extend_mode_string(
										CONTROL_CURVE_EXTEND_CLAMP))))
							{
								init_widgets=0;
							}
							DEALLOCATE(valid_strings);
							if (init_widgets)
							{
								control_curve_editor_set_curve(curve_editor->widget,curve);
								XtManageChild(curve_editor->widget);
								return_widget=curve_editor->widget;
							}
							else
							{
								XtDestroyWidget(curve_editor->widget);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_control_curve_editor_widget.  "
								"Could not fetch control_curve_editor widget");
							DEALLOCATE(curve_editor);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_control_curve_editor_widget.  "
							"Could not register identifiers");
						DEALLOCATE(curve_editor);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_control_curve_editor_widget.  "
						"Could not register callbacks");
					DEALLOCATE(curve_editor);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_control_curve_editor_widget.  "
					"Could not allocate control_curve_editor widget structure");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_control_curve_editor_widget.  Could not open hierarchy");
		}
		*curve_editor_widget=return_widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_control_curve_editor_widget.  Invalid argument(s)");
	}
	LEAVE;

	return (return_widget);
} /* create_control_curve_editor_widget */

struct Callback_data *control_curve_editor_get_callback(
	Widget control_curve_editor_widget)
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Returns a pointer to the update_callback item of the control_curve_editor
widget.
==============================================================================*/
{
	struct Callback_data *return_callback;
	struct Control_curve_editor *curve_editor;

	ENTER(control_curve_editor_get_callback);
	/* check arguments */
	if (control_curve_editor_widget)
	{
		/* Get the pointer to the data for the dialog */
		XtVaGetValues(control_curve_editor_widget,XmNuserData,&curve_editor,NULL);
		if (curve_editor)
		{
			return_callback=&(curve_editor->update_callback);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_get_callback.  Missing widget data");
			return_callback=(struct Callback_data *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_get_callback.  Missing widget");
		return_callback=(struct Callback_data *)NULL;
	}
	LEAVE;

	return (return_callback);
} /* control_curve_editor_get_callback */

int control_curve_editor_set_callback(Widget control_curve_editor_widget,
	struct Callback_data *new_callback)
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Changes the callback function for the control_curve_editor_widget, which will be
called when the curve changes in any way.
==============================================================================*/
{
	int return_code;
	struct Control_curve_editor *curve_editor;

	ENTER(control_curve_editor_set_callback);
	/* check arguments */
	if (control_curve_editor_widget&&new_callback)
	{
		/* Get the pointer to the data for the choose_settings dialog */
		XtVaGetValues(control_curve_editor_widget,XmNuserData,
			&curve_editor,NULL);
		if (curve_editor)
		{
			curve_editor->update_callback.procedure=new_callback->procedure;
			curve_editor->update_callback.data=new_callback->data;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_set_callback.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_set_callback.  Missing widget");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* control_curve_editor_set_callback */

struct Control_curve *control_curve_editor_get_curve(
	Widget control_curve_editor_widget)
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Returns the Control_curve currently being edited.
==============================================================================*/
{
	struct Control_curve *return_curve;
	struct Control_curve_editor *curve_editor;

	ENTER(control_curve_editor_get_curve);
	/* check arguments */
	if (control_curve_editor_widget)
	{
		/* Get the pointer to the data for the dialog */
		XtVaGetValues(control_curve_editor_widget,XmNuserData,&curve_editor,NULL);
		if (curve_editor)
		{
			return_curve=curve_editor->edit_curve;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_get_curve.  Missing widget data");
			return_curve=(struct Control_curve *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_get_curve.  Missing widget");
		return_curve=(struct Control_curve *)NULL;
	}
	LEAVE;

	return (return_curve);
} /* control_curve_editor_get_curve */

int control_curve_editor_set_curve(Widget control_curve_editor_widget,
	struct Control_curve *curve)
/*******************************************************************************
LAST MODIFIED : 16 June 2000

DESCRIPTION :
Sets the Control_curve to be edited by the control_curve_editor widget.
==============================================================================*/
{
	char temp_string[30];
	FE_value component_grid_size,parameter_grid_size;
	int return_code;
	struct Callback_data callback;
	struct Control_curve *temp_curve;
	struct Control_curve_editor *curve_editor;

	ENTER(control_curve_editor_set_curve);
	if (control_curve_editor_widget)
	{
		/* Get the pointer to the data for the control_curve_editor_widget */
		XtVaGetValues(control_curve_editor_widget,XmNuserData,&curve_editor,NULL);
		if (curve_editor)
		{
			return_code=1;
			if (curve_editor->edit_curve)
			{
				DESTROY(Control_curve)(&(curve_editor->edit_curve));
			}
			if (curve)
			{
				if ((curve_editor->edit_curve=CREATE(Control_curve)("copy",
					LINEAR_LAGRANGE,1))&&(MANAGER_COPY_WITHOUT_IDENTIFIER(
					Control_curve,name)(curve_editor->edit_curve,curve))&&
					control_curve_editor_view_full_range(curve_editor))
				{
					/* make sure no node selected initially: */
					curve_editor->current_element_no=0;
					curve_editor->current_node_no=-1;
					curve_editor->current_component_no=-1;
					curve_editor->first_component_in_view=0;
					/* set widget values */
					choose_enumerator_set_string(curve_editor->basis_type_widget,
						FE_basis_type_string(
							Control_curve_get_fe_basis_type(curve_editor->edit_curve)));
					choose_enumerator_set_string(curve_editor->extend_mode_widget,
						Control_curve_extend_mode_string(
							Control_curve_get_extend_mode(curve_editor->edit_curve)));
					sprintf(temp_string,"%i",
						Control_curve_get_number_of_components(curve_editor->edit_curve));
					XtVaSetValues(curve_editor->num_components_text,
						XmNvalue,temp_string,NULL);
					curve_editor->range_component_no=-1;
					control_curve_editor_set_range_component_no(curve_editor,0);

					Control_curve_get_parameter_grid(curve,&parameter_grid_size);
					sprintf(temp_string,"%g",parameter_grid_size);
					XtVaSetValues(curve_editor->parameter_grid_text,
						XmNvalue,temp_string,NULL);
					Control_curve_get_value_grid(curve,&component_grid_size);
					sprintf(temp_string,"%g",component_grid_size);
					XtVaSetValues(curve_editor->comp_grid_text,
						XmNvalue,temp_string,NULL);

					/* draw new curve */
					control_curve_editor_drawing_layout_change(curve_editor);
					control_curve_editor_drawing_area_redraw(curve_editor,0);
					/* turn on callbacks */
					callback.data=(void *)curve_editor;
					callback.procedure=control_curve_editor_update_basis_type;
					choose_enumerator_set_callback(
						curve_editor->basis_type_widget,&callback);
					callback.procedure=control_curve_editor_change_component;
					choose_field_component_set_callback(
						curve_editor->component_widget,&callback);
					callback.procedure=control_curve_editor_update_extend_mode;
					choose_enumerator_set_callback(
						curve_editor->extend_mode_widget,&callback);
					XtSetSensitive(curve_editor->widget,True);
				}
				else
				{
					if (curve_editor->edit_curve)
					{
						DESTROY(Control_curve)(&(curve_editor->edit_curve));
					}
					display_message(ERROR_MESSAGE,
						"control_curve_editor_set_curve.  Could not make copy of curve");
					curve=(struct Control_curve *)NULL;
					return_code=0;
				}
			}
			if (!curve)
			{
				/* make simple curve to show parameters for one that will be created */
				if (temp_curve=CREATE(Control_curve)("temp",LINEAR_LAGRANGE,1))
				{
					control_curve_editor_set_curve(control_curve_editor_widget,
						temp_curve);
					DESTROY(Control_curve)(&temp_curve);
				}
				else
				{
					/* switch off subwidget callbacks */
					callback.procedure=(Callback_procedure *)NULL;
					callback.data=(void *)NULL;
					choose_enumerator_set_callback(
						curve_editor->basis_type_widget,&callback);
					choose_field_component_set_callback(
						curve_editor->component_widget,&callback);
					choose_enumerator_set_callback(
						curve_editor->extend_mode_widget,&callback);
					/* set subwidget curves to NULL */
					choose_field_component_set_field_component(
						curve_editor->component_widget,(struct FE_field *)NULL,0);
				}
				/* gray out widgets */
				XtSetSensitive(curve_editor->widget,False);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_set_curve.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_set_curve.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* control_curve_editor_set_curve */
