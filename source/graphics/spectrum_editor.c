/*******************************************************************************
FILE : spectrum_editor.c

LAST MODIFIED : 12 August 2002

DESCRIPTION :
Initially pillaged from graphics/graphical_element_editor.c
Provides the widgets to manipulate spectrum settings.
==============================================================================*/
#include <stdio.h>
#include <math.h>
#include <Xm/Xm.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include "command/parser.h"
#include "general/debug.h"
#include "graphics/scene.h"
#include "graphics/scene_viewer.h"
#include "graphics/spectrum.h"
#include "graphics/spectrum_editor.h"
#include "graphics/spectrum_editor.uidh"
#include "graphics/spectrum_editor_settings.h"
#include "graphics/spectrum_settings.h"
#include "three_d_drawing/ThreeDDraw.h"
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int spectrum_editor_hierarchy_open=0;
static MrmHierarchy spectrum_editor_hierarchy;
#endif /* defined (MOTIF) */

struct Spectrum_editor
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Contains all the information carried by the graphical element editor widget.
==============================================================================*/
{
	/* This editor_material is used when displaying the spectrum in the 
		3d widget */
	struct Graphical_material *editor_material, *tick_material;
	struct Spectrum_settings *current_settings;
	struct Spectrum *edit_spectrum;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct User_interface *user_interface;
	void *material_manager_callback_id;
	void *spectrum_manager_callback_id;
	struct Callback_data update_callback;
	Widget settings_scroll,settings_rowcol,add_button,
		delete_button,up_button,down_button,settings_form,settings_widget,
		viewer_form, opaque_button;
	Widget widget,widget_parent;
	struct Scene *spectrum_editor_scene;
	struct Scene_viewer *spectrum_editor_scene_viewer;
	struct GT_object *graphics_object, *tick_lines_graphics_object,
		*tick_labels_graphics_object;
	int viewer_type;
}; /* spectrum_editor */

/*
Module functions
----------------
*/
static int make_edit_spectrum(
	struct Spectrum_editor *spectrum_editor,
	struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Destroys the edit_spectrum member of <spectrum_editor> and rebuilds it as
a complete copy of <Spectrum>.
==============================================================================*/
{
	int return_code;

	ENTER(make_edit_spectrum);
	/* check arguments */
	if (spectrum_editor&&spectrum)
	{
		/* destroy current edit_spectrum */
		if (spectrum_editor->edit_spectrum)
		{
			DEACCESS(Spectrum)(&(spectrum_editor->edit_spectrum));
		}
		/* make an empty spectrum */
		if (spectrum_editor->edit_spectrum=ACCESS(Spectrum)(CREATE(Spectrum)("copy")))
		{
			/* copy general settings into new object */
			MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name)
				(spectrum_editor->edit_spectrum,spectrum);

			XmToggleButtonGadgetSetState(spectrum_editor->opaque_button,
				Spectrum_get_opaque_colour_flag(spectrum_editor->edit_spectrum),
				False);

			set_GT_object_Spectrum(spectrum_editor->graphics_object,
				(void *)spectrum_editor->edit_spectrum);

			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"make_edit_spectrum.  Could not make copy of spectrum");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"make_edit_spectrum.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* make_edit_spectrum */

static int create_settings_item_widget(struct Spectrum_settings *settings,
	void *spectrum_editor_void)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Clears then fills the settings list RowCol with descriptions of the settings
of the type spectrum_editor->settings_type.
==============================================================================*/
{
	int num_children,return_code;
	struct Spectrum_editor *spectrum_editor;
	char *settings_string;
	XmString new_string;
	Arg override_arg;
	MrmType settings_item_class;
	Widget temp_widget, *child_list;

	ENTER(create_settings_item_widget);
	/* check arguments */
	if (settings&&(spectrum_editor=
		(struct Spectrum_editor *)spectrum_editor_void))
	{
		if (spectrum_editor_hierarchy_open)
		{
			if (settings_string=Spectrum_settings_string(settings,
				SPECTRUM_SETTINGS_STRING_COMPLETE_PLUS))
			{
				XtSetArg(override_arg,XmNuserData,settings);
				temp_widget=(Widget)NULL;
				if (MrmSUCCESS==MrmFetchWidgetOverride(spectrum_editor_hierarchy,
					"spec_ed_settings_item",spectrum_editor->settings_rowcol,NULL,
					&override_arg,1,&temp_widget,&settings_item_class))
				{
					XtManageChild(temp_widget);
					/* get the children = visibility and select toggle buttons */
					XtVaGetValues(temp_widget,XmNnumChildren,&num_children,
						XmNchildren,&child_list,NULL);
					if (2==num_children)
					{
						/* set the visibility toggle button */
						if (Spectrum_settings_get_active(settings))
						{
							XtVaSetValues(child_list[0],XmNset,True,NULL);
						}
						else
						{
							XtVaSetValues(child_list[0],XmNset,False,NULL);
						}
						/* make the settings_string the name of this item */
						new_string=XmStringCreateSimple(settings_string);
						XtVaSetValues(child_list[1],XmNlabelString,new_string,NULL);
						XmStringFree(new_string);
						/* check current settings */
						if (settings==spectrum_editor->current_settings)
						{
							XtVaSetValues(child_list[1],XmNset,True,NULL);
						}
					}
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_settings_item_widget.  Could not fetch widget");
					return_code=0;
				}
				DEALLOCATE(settings_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_settings_item_widget.  Could not get settings string");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_settings_item_widget.  Hierarchy not open");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_settings_item_widget.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* create_settings_item_widget */

static int spectrum_editor_make_settings_list(
	struct Spectrum_editor *spectrum_editor)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Clears then fills the settings list RowColumn with descriptions of the settings
of the type spectrum_editor->settings_type.
==============================================================================*/
{
	int return_code;
	Widget temp_widget;
	Arg override_arg;
	MrmType settings_rowcol_class;

	ENTER(spectrum_editor_make_settings_list);
	/* check arguments */
	if (spectrum_editor)
	{
		if (spectrum_editor_hierarchy_open)
		{
			/* delete the rowcol and all the items it contains */
			if (spectrum_editor->settings_rowcol)
			{
				XtDestroyWidget(spectrum_editor->settings_rowcol);
			}
			/* recreate the rowcol */
			XtSetArg(override_arg,XmNuserData,spectrum_editor);
			temp_widget=(Widget)NULL;
			if (MrmSUCCESS==MrmFetchWidgetOverride(spectrum_editor_hierarchy,
				"spec_ed_settings_rowcol",spectrum_editor->settings_scroll,NULL,
				&override_arg,1,&temp_widget,&settings_rowcol_class))
			{
				/* now insert the new items/widgets */
				FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
					create_settings_item_widget,(void *)spectrum_editor,
					get_Spectrum_settings_list( spectrum_editor->edit_spectrum ));
				XtManageChild(temp_widget);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"spectrum_editor_make_settings_list.  Could not fetch rowcol");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_settings_item_widget.  Hierarchy not open");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_make_settings_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_make_settings_list */

static int spectrum_editor_update_scene_viewer(
	struct Spectrum_editor *spectrum_editor)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
==============================================================================*/
{
	char **strings, *string_data;
	const float extend_ends = 0.04;
	struct Colour black ={0, 0, 0}, white = {1.0, 1.0, 1.0};
	int i, j, npts1, npts2, number_of_points,
		tick_label_count, tick_line_count, return_code;
	float bar_min, bar_max, min, max, value_xi1;
	GTDATA *data;
	Triple *line_points, *label_points;
	struct GT_surface *surface;
	struct GT_polyline *tick_lines;
	struct GT_pointset *tick_labels;

	ENTER(spectrum_editor_update_scene_viewer);
	/* check arguments */
	if (spectrum_editor && spectrum_editor->edit_spectrum 
		&& spectrum_editor->graphics_object
		&& spectrum_editor->tick_lines_graphics_object
		&& spectrum_editor->tick_labels_graphics_object)
	{
		surface = GT_OBJECT_GET(GT_surface)(spectrum_editor->graphics_object, 0);
		tick_lines = GT_OBJECT_GET(GT_polyline)(spectrum_editor->tick_lines_graphics_object, 0);
		tick_labels = GT_OBJECT_GET(GT_pointset)(spectrum_editor->tick_labels_graphics_object, 0);
		data = surface->data;
		npts1 = surface->n_pts1;
		npts2 = surface->n_pts2;
		strings = tick_labels->text;
		label_points = tick_labels->pointlist;
		tick_label_count = tick_labels->n_pts;
		line_points = tick_lines->pointlist;
		tick_line_count = tick_lines->n_pts;

		switch (spectrum_editor->viewer_type % 6)
		{
			case 0:
			{
				Graphical_material_set_ambient(spectrum_editor->editor_material, &black );
				Graphical_material_set_diffuse(spectrum_editor->editor_material, &black );
				number_of_points = 5;
			} break;
			case 1:
			{
				number_of_points = 11;
			} break;
			case 2:
			{
				number_of_points = 2;
			} break;
			case 3:
			{
				Graphical_material_set_ambient(spectrum_editor->editor_material, &white );
				Graphical_material_set_diffuse(spectrum_editor->editor_material, &white );
				number_of_points = 5;
			} break;
			case 4:
			{
				number_of_points = 11;
			} break;
			case 5:
			{
				number_of_points = 2;
			} break;
		}
		if ( tick_label_count != number_of_points 
			|| tick_line_count != number_of_points )
		{
			if ( strings )
			{
				string_data = strings[0];
			}
			else
			{
				string_data = (char *)NULL;
			}
			if (REALLOCATE(line_points, line_points, Triple, 2 * number_of_points)
				&& REALLOCATE(label_points, label_points, Triple, number_of_points)
				&& REALLOCATE(string_data, string_data, char, number_of_points * 15)
				&& REALLOCATE(strings, strings, char *, number_of_points))
			{
				tick_line_count = number_of_points;
				tick_label_count = number_of_points;
				for ( i = 0 ; i < number_of_points ; i++ )
				{
					value_xi1 = (-5.0 + 10.0 * (float) i / 
						(float)(number_of_points - 1))
						/ (1. + 2.0 * extend_ends);
					label_points[i][0] = value_xi1;
					label_points[i][1] = 0;
					label_points[i][2] = -1.0;
					strings[i] = string_data + 15 * i;
					/* the strings will be set below */
				}
				i = 0;
				while( i < 2 * number_of_points )
				{
					value_xi1 = (-5.0 + 10.0 * (float) i / 
						(float)(2 * number_of_points - 2))
						/ (1. + 2.0 * extend_ends);
					line_points[i][0] = value_xi1;
					line_points[i][1] = 0;
					line_points[i][2] = -0.5;
					i++;
					line_points[i][0] = value_xi1;
					line_points[i][1] = 0;
					line_points[i][2] = -1.0;
					i++;
				}
				tick_labels->text = strings;
				tick_labels->pointlist = label_points;
				tick_labels->n_pts = tick_label_count;
				tick_lines->pointlist = line_points;
				tick_lines->n_pts = tick_line_count;
			}

			GT_object_changed(spectrum_editor->tick_lines_graphics_object);
		}
		min = get_Spectrum_minimum(spectrum_editor->edit_spectrum);
		max = get_Spectrum_maximum(spectrum_editor->edit_spectrum);

		for ( i = 0 ; i < tick_label_count ; i++ )
		{
			sprintf(strings[i], "%6.2g", min + (max - min)
				* (float)i / (float)(tick_label_count - 1));
		}

		bar_min = min - extend_ends * (max - min);
		bar_max = max + extend_ends * (max - min);
		for ( i = 0 ; i < npts2 ; i++ )
		{
			for ( j = 0 ; j < npts1 ; j++ )
			{
				*data = bar_min  + (bar_max - bar_min)
					* (float)j / (float)(npts1 - 1);
				data++;
			}
		}
		GT_object_changed(spectrum_editor->graphics_object);
		GT_object_changed(spectrum_editor->tick_labels_graphics_object);
		Scene_viewer_redraw(spectrum_editor->spectrum_editor_scene_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_update_scene_viewer.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_update_scene_viewer */

static int spectrum_editor_select_settings_item(
	struct Spectrum_editor *spectrum_editor)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Checks if current_settings is in the settings_rowcol; if not (or it was NULL)
the first item in this list becomes the current_settings, or NULL if empty.
The line/settings editors for the previous settings are then Unmanaged, and that
for the new settings Managed and filled with the new values.
If current_settings is NULL, no editing fields are displayed.
==============================================================================*/
{
	int return_code,i,num_children,num_toggles;
	struct Spectrum_settings *temp_settings;
	WidgetList child_list,toggle_list;

	ENTER(spectrum_editor_select_settings_item);
	/* check arguments */
	if (spectrum_editor)
	{
		/* get list of settings items */
		XtVaGetValues(spectrum_editor->settings_rowcol,XmNnumChildren,&num_children,
			XmNchildren,&child_list,NULL);
		if (0<num_children)
		{
			for (i=0;i<num_children;i++)
			{
				XtVaGetValues(child_list[i],XmNuserData,&temp_settings,
					XmNnumChildren,&num_toggles,XmNchildren,&toggle_list,NULL);
				if (2==num_toggles)
				{
					if (!spectrum_editor->current_settings)
					{
						spectrum_editor->current_settings=temp_settings;
					}
					if (temp_settings==spectrum_editor->current_settings)
					{
						XtVaSetValues(toggle_list[1],XmNset,True,NULL);
					}
					else
					{
						XtVaSetValues(toggle_list[1],XmNset,False,NULL);
					}
				}
			}
		}
		else
		{
			spectrum_editor->current_settings=(struct Spectrum_settings *)NULL;
		}
		/* Grey Delete and Priority buttons if no current_settings */
		XtSetSensitive(spectrum_editor->delete_button,
			(struct Spectrum_settings *)NULL != spectrum_editor->current_settings);
		XtSetSensitive(spectrum_editor->up_button,
			(struct Spectrum_settings *)NULL != spectrum_editor->current_settings);
		XtSetSensitive(spectrum_editor->down_button,
			(struct Spectrum_settings *)NULL != spectrum_editor->current_settings);
		/* send selected object to settings editor */
		return_code=spectrum_editor_settings_set_settings(
			spectrum_editor->settings_widget,spectrum_editor->current_settings);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_select_settings_item.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_select_settings_item */

static int spectrum_editor_update(
	struct Spectrum_editor *spectrum_editor)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Tells CMGUI about the current values. Returns a pointer to the
gt_element_object currently being edited.
==============================================================================*/
{
	int return_code;

	ENTER(spectrum_editor_update);
	/* checking arguments */
	if (spectrum_editor)
	{
		spectrum_editor_update_scene_viewer(spectrum_editor);
		/* Now send an update to the client if requested */
		if ((spectrum_editor->update_callback).procedure)
		{
			(spectrum_editor->update_callback.procedure)(
				spectrum_editor->widget,spectrum_editor->update_callback.data,
				spectrum_editor->edit_spectrum);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_update.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_update */

DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor, \
	Spectrum_editor,settings_scroll)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor, \
	Spectrum_editor,settings_rowcol)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor, \
	Spectrum_editor,add_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor, \
	Spectrum_editor,delete_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor, \
	Spectrum_editor,up_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor, \
	Spectrum_editor,down_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor, \
	Spectrum_editor,settings_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor, \
	Spectrum_editor,viewer_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(spectrum_editor, \
	Spectrum_editor,opaque_button)

static void spectrum_editor_settings_visibility_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Called when a settings visibility toggle button is selected.
==============================================================================*/
{
	struct Spectrum_editor *spectrum_editor;
	struct Spectrum_settings *settings;
	Widget settings_form;

	ENTER(spectrum_editor_settings_visibility_CB);
	USE_PARAMETER(call_data);
	if (widget&&
		(spectrum_editor=(struct Spectrum_editor *)client_data))
	{
		/* the settings is kept as XmNuserData with the parent settings form */
		if (settings_form=XtParent(widget))
		{
			/* Get the material this menu visibility represents and make it current */
			XtVaGetValues(settings_form,XmNuserData,&settings,NULL);
			if (settings)
			{
				Spectrum_settings_set_active(settings,
					XmToggleButtonGetState(widget));
				/* inform the client of the changes */
				spectrum_editor_update(spectrum_editor);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"spectrum_editor_settings_visibility_CB.  Missing settings");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_settings_visibility_CB.  "
				"Missing parent settings form");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_settings_visibility_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* spectrum_editor_settings_visibility_CB */

static void spectrum_editor_settings_select_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Called when a settings select toggle button is selected.
==============================================================================*/
{
	struct Spectrum_editor *spectrum_editor;
	struct Spectrum_settings *settings;
	Widget settings_form;

	ENTER(spectrum_editor_settings_select_CB);
	USE_PARAMETER(call_data);
	if (widget&&
		(spectrum_editor=(struct Spectrum_editor *)client_data))
	{
		/* the settings is kept as XmNuserData with the parent settings form */
		if (settings_form=XtParent(widget))
		{
			/* Get the material this menu select represents and make it current */
			XtVaGetValues(settings_form,XmNuserData,&settings,NULL);
			if (settings != spectrum_editor->current_settings)
			{
				spectrum_editor->current_settings=settings;
			}
			spectrum_editor_select_settings_item(spectrum_editor);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_settings_select_CB.  "
				"Missing parent settings form");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_settings_select_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* spectrum_editor_settings_select_CB */

static void spectrum_editor_modify_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 July 1998

DESCRIPTION :
Called when a modify button - add, delete, up, down - is activated.
==============================================================================*/
{
	int list_changed,position,return_code;
	struct Spectrum_editor *spectrum_editor;
	struct Spectrum_settings *settings;
	Widget modify_button;

	ENTER(spectrum_editor_modify_CB);
	if (widget&&(modify_button=((XmRowColumnCallbackStruct *)call_data)->widget)&&
		(spectrum_editor=(struct Spectrum_editor *)client_data)&&
		spectrum_editor->edit_spectrum)
	{
		list_changed=0;
		if (modify_button==spectrum_editor->add_button)
		{
			if (settings=CREATE(Spectrum_settings)())
			{
				return_code=1;
				if (spectrum_editor->current_settings)
				{
					/* copy current settings into new settings */
					return_code=COPY(Spectrum_settings)(settings,
						spectrum_editor->current_settings);
					/* make sure new settings is visible */
					Spectrum_settings_set_active(settings,1);
				}
				if (return_code&&Spectrum_add_settings(
					spectrum_editor->edit_spectrum,settings,0))
				{
					list_changed=1;
					spectrum_editor->current_settings=settings;
				}
				else
				{
					DESTROY(Spectrum_settings)(&settings);
				}
			}
		}
		else if (modify_button==spectrum_editor->delete_button)
		{
			list_changed=Spectrum_remove_settings(
				spectrum_editor->edit_spectrum,spectrum_editor->current_settings);
			spectrum_editor->current_settings=(struct Spectrum_settings *)NULL;
		}
		else if (modify_button==spectrum_editor->up_button)
		{
			/* increase position of current_settings by 1 */
			if (1<(position=Spectrum_get_settings_position(
				spectrum_editor->edit_spectrum,spectrum_editor->current_settings)))
			{
				list_changed=1;
				ACCESS(Spectrum_settings)(spectrum_editor->current_settings);
				Spectrum_remove_settings(spectrum_editor->edit_spectrum,
					spectrum_editor->current_settings);
				Spectrum_add_settings(spectrum_editor->edit_spectrum,
					spectrum_editor->current_settings,position-1);
				DEACCESS(Spectrum_settings)(&(spectrum_editor->current_settings));
			}
		}
		else if (modify_button==spectrum_editor->down_button)
		{
			/* decrease position of current_settings by 1 */
			if (position=Spectrum_get_settings_position(
				spectrum_editor->edit_spectrum,spectrum_editor->current_settings))
			{
				list_changed=1;
				ACCESS(Spectrum_settings)(spectrum_editor->current_settings);

				Spectrum_remove_settings(spectrum_editor->edit_spectrum,
					spectrum_editor->current_settings);
				Spectrum_add_settings(spectrum_editor->edit_spectrum,
					spectrum_editor->current_settings,position+1);
				DEACCESS(Spectrum_settings)(&(spectrum_editor->current_settings));
			}
		}
		if (list_changed)
		{
			spectrum_editor_make_settings_list(spectrum_editor);
			spectrum_editor_select_settings_item(spectrum_editor);
			/* inform the client of the changes */
			spectrum_editor_update(spectrum_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_modify_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* spectrum_editor_modify_CB */

static void spectrum_editor_update_settings(
	Widget spectrum_editor_settings_widget,
	void *spectrum_editor_void,void *new_settings_void)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Callback for when changes made in the settings editor.
==============================================================================*/
{
	int active;
	struct Spectrum_editor *spectrum_editor;
	struct Spectrum_settings *new_settings;

	ENTER(spectrum_editor_update_settings);
	USE_PARAMETER(spectrum_editor_settings_widget);
	if ((spectrum_editor=
			(struct Spectrum_editor *)spectrum_editor_void)
		&&(new_settings=(struct Spectrum_settings *)new_settings_void))
	{
		/* keep visibility of current_settings */
		active=
			Spectrum_settings_get_active(spectrum_editor->current_settings);
		Spectrum_settings_modify(spectrum_editor->current_settings,new_settings,
			get_Spectrum_settings_list( spectrum_editor->edit_spectrum ));
		Spectrum_settings_set_active(spectrum_editor->current_settings,
			active);
		spectrum_editor_make_settings_list(spectrum_editor);
		Spectrum_calculate_range(spectrum_editor->edit_spectrum);
		/* inform the client of the changes */
		spectrum_editor_update(spectrum_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_update_settings.  Invalid argument(s)");
	}
	LEAVE;
} /* spectrum_editor_update_settings */

static void spectrum_editor_viewer_input_callback(
	struct Scene_viewer *scene_viewer, struct Graphics_buffer_input *input,
	void *spectrum_editor_void)
/*******************************************************************************
LAST MODIFIED : 2 July 2002

DESCRIPTION :
Callback for when input is received by the scene_viewer.
==============================================================================*/
{
	struct Spectrum_editor *spectrum_editor;

	ENTER(spectrum_editor_viewer_input_CB);
	USE_PARAMETER(scene_viewer);
	if (spectrum_editor=(struct Spectrum_editor *)spectrum_editor_void)
	{
		if (GRAPHICS_BUFFER_BUTTON_PRESS==input->type)
		{
			/* Increment the type */
			spectrum_editor->viewer_type++;
			spectrum_editor_update_scene_viewer(spectrum_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_viewer_input_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* spectrum_editor_viewer_input_CB */

static void spectrum_editor_opaque_button_CB(Widget widget, int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Called when the opaque toggle button is changed.
==============================================================================*/
{
	Boolean state;
	struct Spectrum_editor *spectrum_editor;

	ENTER(spectrum_editor_opaque_button_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* Get the pointer to the data for the widget */
		XtVaGetValues(widget,
			XmNuserData,&spectrum_editor,
			XmNset, &state,
			NULL);
		if (spectrum_editor)
		{
			if (state)
			{
				if (!Spectrum_get_opaque_colour_flag(spectrum_editor->edit_spectrum))
				{
					Spectrum_set_opaque_colour_flag(spectrum_editor->edit_spectrum, 1);
					spectrum_editor_update_scene_viewer(spectrum_editor);
				}
			}
			else
			{
				if (Spectrum_get_opaque_colour_flag(spectrum_editor->edit_spectrum))
				{
					Spectrum_set_opaque_colour_flag(spectrum_editor->edit_spectrum, 0);
					spectrum_editor_update_scene_viewer(spectrum_editor);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_opaque_button_CB.  "
				"Missing spectrum_editor_dialog struct");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_opaque_button_CB.  Missing widget");
	}
	LEAVE;
} /* spectrum_editor_opaque_button_CB */

/*
Global functions
----------------
*/

struct Spectrum_editor *CREATE(Spectrum_editor)(
	Widget parent, struct Spectrum *spectrum,
	struct User_interface *user_interface,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(Light) *light_manager,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct MANAGER(Texture) *texture_manager)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Creates a spectrum_editor widget.
==============================================================================*/
{
	int i,j,return_code,surface_discretise_xi1=24,surface_discretise_xi2=108;
	GTDATA *data;
	MrmType spectrum_editor_dialog_class;
	struct Spectrum_editor *spectrum_editor;
	static MrmRegisterArg callback_list[]=
	{
		{"spec_ed_identify_settings_scr",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor,settings_scroll)},
		{"spec_ed_identify_settings_rc",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor,settings_rowcol)},
		{"spec_ed_id_add_btn",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor,add_button)},
		{"spec_ed_id_delete_btn",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor,delete_button)},
		{"spec_ed_id_up_btn",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor,up_button)},
		{"spec_ed_id_down_btn",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor,down_button)},
		{"spec_ed_identify_settings_form",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor,settings_form)},
		{"spec_ed_id_viewer_form",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor,viewer_form)},
		{"spec_ed_id_opaque_button",(XtPointer)
			DIALOG_IDENTIFY(spectrum_editor,opaque_button)},
		{"spec_ed_opaque_button_CB",(XtPointer)
			spectrum_editor_opaque_button_CB},
		{"spec_ed_modify_CB",(XtPointer)
			spectrum_editor_modify_CB},
		{"spec_ed_settings_visibility_CB",(XtPointer)
			spectrum_editor_settings_visibility_CB},
		{"spec_ed_settings_select_CB",(XtPointer)
			spectrum_editor_settings_select_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"spec_ed_structure",(XtPointer)NULL}
	};
	struct Colour background_colour = {0.1, 0.1, 0.1},
		ambient_colour = {0.2, 0.2, 0.2}, black ={0, 0, 0},
		off_white = {0.9, 0.8, 0.8};
	struct Graphics_buffer *graphics_buffer;
	struct Light *viewer_light;
	struct Light_model *viewer_light_model;
	Triple *points, *normalpoints;
	float value_xi1, value_xi2, light_direction[3] = {0, -0.2, -1.0};
	struct GT_surface *cylinder_surface;
	struct GT_polyline *tick_lines;
	struct GT_pointset *tick_labels;
	struct Graphical_material *tick_material;
	char **strings, *string_data;
	struct Spectrum *default_scene_spectrum;

	ENTER(CREATE(Spectrum_editor));
	spectrum_editor = (struct Spectrum_editor *)NULL;
	if (parent && user_interface)
	{
		if (MrmOpenHierarchy_base64_string(spectrum_editor_uidh,
			&spectrum_editor_hierarchy,&spectrum_editor_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(spectrum_editor,struct Spectrum_editor,1))
			{
				/* initialise the structure */
				spectrum_editor->current_settings = (struct Spectrum_settings *)NULL;
				spectrum_editor->edit_spectrum=(struct Spectrum *)NULL;
				spectrum_editor->user_interface=user_interface;
				spectrum_editor->material_manager_callback_id=(void *)NULL;
				spectrum_editor->spectrum_manager_callback_id=(void *)NULL;
				spectrum_editor->widget_parent=parent;
				spectrum_editor->widget=(Widget)NULL;
				spectrum_editor->settings_scroll=(Widget)NULL;
				spectrum_editor->settings_rowcol=(Widget)NULL;
				spectrum_editor->add_button=(Widget)NULL;
				spectrum_editor->delete_button=(Widget)NULL;
				spectrum_editor->up_button=(Widget)NULL;
				spectrum_editor->down_button=(Widget)NULL;
				spectrum_editor->settings_form=(Widget)NULL;
				spectrum_editor->settings_widget=(Widget)NULL;
				spectrum_editor->viewer_form=(Widget)NULL;
				spectrum_editor->opaque_button=(Widget)NULL;
				spectrum_editor->update_callback.procedure=(Callback_procedure *)NULL;
				spectrum_editor->update_callback.data=(void *)NULL;
				spectrum_editor->editor_material = (struct Graphical_material *)NULL;
				spectrum_editor->tick_material = (struct Graphical_material *)NULL;
				spectrum_editor->graphics_object = (struct GT_object *)NULL;
				spectrum_editor->tick_lines_graphics_object = (struct GT_object *)NULL;
				spectrum_editor->tick_labels_graphics_object = (struct GT_object *)NULL;
				spectrum_editor->spectrum_editor_scene = (struct Scene *)NULL;
				spectrum_editor->spectrum_editor_scene_viewer = (struct Scene_viewer *)NULL;
				spectrum_editor->viewer_type = 0;
				
				/* register the callbacks */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
					spectrum_editor_hierarchy,
					callback_list,XtNumber(callback_list)))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)spectrum_editor;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						spectrum_editor_hierarchy,
						identifier_list,XtNumber(identifier_list)))
					{
						/* fetch graphical element editor widget */
						if (MrmSUCCESS==MrmFetchWidget(spectrum_editor_hierarchy,
							"spec_ed_widget",spectrum_editor->widget_parent,
							&(spectrum_editor->widget),
							&spectrum_editor_dialog_class))
						{
							return_code=1;
							/* create settings editor with NULL settings */
							if (!(create_spectrum_editor_settings_widget(
								&(spectrum_editor->settings_widget),
								spectrum_editor->settings_form,(struct Spectrum_settings *)NULL)))
							{
								display_message(ERROR_MESSAGE,
									"CREATE(Spectrum_editor).  "
									"Could not create spectrum editor settings widget");
								return_code=0;
							}
							if (return_code)
							{
								if (spectrum_editor->viewer_form)
								{
									spectrum_editor->editor_material = ACCESS(Graphical_material)
										(CREATE(Graphical_material)("editor_material"));
									Graphical_material_set_ambient(spectrum_editor->editor_material, &black );
									Graphical_material_set_diffuse(spectrum_editor->editor_material, &black );
									Graphical_material_set_shininess(spectrum_editor->editor_material, 0.8 );
									tick_material = CREATE(Graphical_material)("editor_material");
									spectrum_editor->tick_material = ACCESS(Graphical_material)(
										tick_material);
									Graphical_material_set_ambient(tick_material, &off_white );
									Graphical_material_set_diffuse(tick_material, &off_white );
									Graphical_material_set_shininess(tick_material, 0.8 );
									if ( ALLOCATE( points, Triple, surface_discretise_xi1 *
										surface_discretise_xi2) &&
										ALLOCATE( normalpoints, Triple, surface_discretise_xi1 *
										surface_discretise_xi2) &&
										ALLOCATE( data, GTDATA, surface_discretise_xi1 *
										surface_discretise_xi2 ) )
									{
										for ( i = 0 ; i < surface_discretise_xi1 ; i++ )
										{
											value_xi1 = sin ( (float) i * 2.0 * PI / (float)(surface_discretise_xi1 - 1));
											value_xi2 = cos ( (float) i * 2.0 * PI / (float)(surface_discretise_xi1 - 1));
											for ( j = 0 ; j < surface_discretise_xi2 ; j++ )
											{
												points[i * surface_discretise_xi2 + j][0] = -5.0 + 10.0 * (float) j / 
													(float)(surface_discretise_xi2 - 1);
												points[i * surface_discretise_xi2 + j][1] = value_xi1;
												points[i * surface_discretise_xi2 + j][2] = 0.5 + value_xi2;
												/* Normals */
												normalpoints[i * surface_discretise_xi2 + j][0] = 0;
												normalpoints[i * surface_discretise_xi2 + j][1] = value_xi1;
												normalpoints[i * surface_discretise_xi2 + j][2] = value_xi2;
												/* Spectrum */
												data[i * surface_discretise_xi2 + j] = (float) j / 
													(float)(surface_discretise_xi2 - 1);
											}
										}
										if ((spectrum_editor->graphics_object =
											CREATE(GT_object)("spectrum_editor_surface",g_SURFACE,
											spectrum_editor->editor_material)))
										{
											ACCESS(GT_object)(spectrum_editor->graphics_object);
											if (cylinder_surface=CREATE(GT_surface)(
												g_SHADED_TEXMAP, g_QUADRILATERAL,
												surface_discretise_xi2, surface_discretise_xi1,
												points, normalpoints, /*tangentpoints*/(Triple *)NULL,
												/*texturepoints*/(Triple *)NULL,
												/* n_data_components */1, data))
											{
												GT_OBJECT_ADD(GT_surface)(
													spectrum_editor->graphics_object, 0,
													cylinder_surface);
											}
											else
											{
												DEALLOCATE( points );
												DEALLOCATE( data );
												return_code = 0;
												display_message(ERROR_MESSAGE,
													"CREATE(Spectrum_editor). Unable to create surface");
											}
										}
										else
										{
											DEALLOCATE( points );
											DEALLOCATE( data );
											return_code = 0;
											display_message(ERROR_MESSAGE,
												"CREATE(Spectrum_editor). Unable to create graphics_object");
										}
									}
									if ( return_code )
									{
										points = (Triple *)NULL;
										if ((spectrum_editor->tick_lines_graphics_object=
											CREATE(GT_object)("spectrum_editor_tick_lines",g_POLYLINE,
											tick_material)))
										{
											spectrum_editor->graphics_object->nextobject = 
												ACCESS(GT_object)(spectrum_editor->tick_lines_graphics_object);
											if (tick_lines = CREATE(GT_polyline)(
												g_PLAIN_DISCONTINUOUS, 0, points, /* normalpoints */(Triple *)NULL,
												g_NO_DATA, (GTDATA *)NULL))
											{
												GT_OBJECT_ADD(GT_polyline)(
													spectrum_editor->tick_lines_graphics_object, 0,
													tick_lines);
											}
											else
											{
												return_code = 0;
												display_message(ERROR_MESSAGE,
													"CREATE(Spectrum_editor). Unable to create lines");
											}
										}
										else
										{
											return_code = 0;
											display_message(ERROR_MESSAGE,
												"CREATE(Spectrum_editor). Unable to create tick line graphics_object");
										}
									}
									if ( return_code
										&& ALLOCATE( points, Triple, 1) &&
										ALLOCATE( strings, char *, 1) &&
										ALLOCATE( string_data, char, 1))
									{
										points[0][0] = 0;
										points[0][1] = 0;
										points[0][2] = 0;
										strings[0] = string_data;
										string_data[0] = 0;
										if ((spectrum_editor->tick_labels_graphics_object =
											CREATE(GT_object)("spectrum_editor_tick_labels",
											g_POINTSET,tick_material)))
										{
											spectrum_editor->tick_lines_graphics_object->nextobject = 
												ACCESS(GT_object)(spectrum_editor->tick_labels_graphics_object);
											if (tick_labels = CREATE(GT_pointset)(1,
												points, strings, g_NO_MARKER, 0.0,
												g_NO_DATA, (GTDATA *)NULL, (int *)NULL))
											{
												GT_OBJECT_ADD(GT_pointset)(
													spectrum_editor->tick_labels_graphics_object, 0,
													tick_labels);
											}
											else
											{
												return_code = 0;
												display_message(ERROR_MESSAGE,
													"CREATE(Spectrum_editor). Unable to create tick label pointset");
											}
										}
										else
										{
											return_code = 0;
											display_message(ERROR_MESSAGE,
												"CREATE(Spectrum_editor). Unable to create tick label graphics_object");
										}
									}
									if ( return_code )
									{
										default_scene_spectrum = CREATE(Spectrum)("default_scene_spectrum");
										spectrum_editor->spectrum_editor_scene = CREATE(Scene)("spectrum_editor_scene");
										Scene_enable_graphics( spectrum_editor->spectrum_editor_scene,
											glyph_list, graphical_material_manager, 
											spectrum_editor->editor_material, light_manager,
											spectrum_manager, default_scene_spectrum, 
											texture_manager);
										viewer_light = CREATE(Light)("spectrum_editor_light");
										set_Light_direction(viewer_light, light_direction);
										viewer_light_model = CREATE(Light_model)("spectrum_editor_light_model");
										Light_model_set_ambient(viewer_light_model, &ambient_colour);
										if (graphics_buffer = create_Graphics_buffer_X3d(
											spectrum_editor->viewer_form,
											GRAPHICS_BUFFER_DOUBLE_BUFFERING, GRAPHICS_BUFFER_MONO,
											/*minimum_colour_buffer_depth*/0,
											/*minimum_depth_buffer_depth*/0,
											/*minimum_accumulation_buffer_depth*/0,
											User_interface_get_specified_visual_id(
											user_interface)))
										{
											spectrum_editor->spectrum_editor_scene_viewer = 
												CREATE(Scene_viewer)(graphics_buffer,
												&background_colour,
												(struct MANAGER(Light) *)NULL,viewer_light,
												(struct MANAGER(Light_model) *)NULL,viewer_light_model,
												(struct MANAGER(Scene) *)NULL,
												spectrum_editor->spectrum_editor_scene,
												(struct MANAGER(Texture) *)NULL,
												user_interface );
											return_code=Scene_add_graphics_object(
												spectrum_editor->spectrum_editor_scene,
												spectrum_editor->graphics_object, 0,
												spectrum_editor->graphics_object->name,
												/*fast_changing*/0);
											Scene_viewer_set_input_mode(
												spectrum_editor->spectrum_editor_scene_viewer,
												SCENE_VIEWER_NO_INPUT );
											Scene_viewer_add_input_callback(
												spectrum_editor->spectrum_editor_scene_viewer,
												spectrum_editor_viewer_input_callback,
												(void *)spectrum_editor);
											Scene_viewer_set_viewport_size(
												spectrum_editor->spectrum_editor_scene_viewer,400,150);
											Scene_viewer_set_lookat_parameters(
												spectrum_editor->spectrum_editor_scene_viewer,0,-1,0,0,0,
												0,0,0,1);
											Scene_viewer_set_view_simple(
												spectrum_editor->spectrum_editor_scene_viewer,0,0,0,2.3,
												46,10);
											Scene_viewer_redraw(
												spectrum_editor->spectrum_editor_scene_viewer);
										}
									}
								}
								if (spectrum)
								{
									spectrum_editor_set_spectrum(spectrum_editor, spectrum);
								}
							}
							else
							{
								XtDestroyWidget(spectrum_editor->widget);
								DEALLOCATE(spectrum_editor);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"CREATE(Spectrum_editor).  "
								"Could not fetch spectrum_editor widget");
							DEALLOCATE(spectrum_editor);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"CREATE(Spectrum_editor).  "
							"Could not register identifiers");
						DEALLOCATE(spectrum_editor);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CREATE(Spectrum_editor).  "
						"Could not register callbacks");
					DEALLOCATE(spectrum_editor);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Spectrum_editor).  "
					"Could not allocate spectrum_editor widget structure");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Spectrum_editor).  Could not open hierarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Spectrum_editor).  Invalid argument(s)");
	}
	LEAVE;

	return (spectrum_editor);
} /* CREATE(Spectrum_editor) */

int DESTROY(Spectrum_editor)(struct Spectrum_editor **spectrum_editor_address)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Destroys the <*spectrum_editor_address> and sets
<*spectrum_editor_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct GT_pointset *tick_labels;
	struct Spectrum_editor *spectrum_editor;

	ENTER(DESTROY(Spectrum_editor));
	if (spectrum_editor_address &&
		(spectrum_editor = *spectrum_editor_address))
	{
		return_code = 1;
		DEACCESS(Graphical_material)(&spectrum_editor->editor_material);
		DEACCESS(Graphical_material)(&spectrum_editor->tick_material);
		/* The strings in the labels graphics object are stored in two 
			 ALLOCATED blocks instead of ALLOCATING each string individually.
			 So I will manually DEALLOCATE the strings and set them to NULL */
		tick_labels = GT_OBJECT_GET(GT_pointset)(
			spectrum_editor->tick_labels_graphics_object, 0);
		if ( tick_labels->text )
		{
			DEALLOCATE(tick_labels->text[0]);
			DEALLOCATE(tick_labels->text);
			tick_labels->text = (char **)NULL;
		}
		/* The DEACCESS for the first graphics object automatically works
			 down the linked list chain */
		DEACCESS(GT_object)(&spectrum_editor->graphics_object);
		DESTROY(Scene_viewer)(&spectrum_editor->spectrum_editor_scene_viewer);
		XtDestroyWidget(spectrum_editor->widget);
		/* destroy edit_spectrum */
		if (spectrum_editor->edit_spectrum)
		{
			DEACCESS(Spectrum)(
				&(spectrum_editor->edit_spectrum));
		}
		DEALLOCATE(*spectrum_editor_address);
		*spectrum_editor_address = (struct Spectrum_editor *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Spectrum_editor).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Spectrum_editor) */

int spectrum_editor_get_callback(
	struct Spectrum_editor *spectrum_editor,struct Callback_data *callback)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Get the update <callback> information for the <spectrum_editor>.
==============================================================================*/
{
	int return_code;

	ENTER(spectrum_editor_get_callback);
	return_code=0;
	/* check arguments */
	if (spectrum_editor&&callback)
	{
		callback->procedure=spectrum_editor->update_callback.procedure;
		callback->data=spectrum_editor->update_callback.data;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_get_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_get_callback */

int spectrum_editor_set_callback(
	struct Spectrum_editor *spectrum_editor,struct Callback_data *callback)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Set the update <callback> information for the <spectrum_editor>.
==============================================================================*/
{
	int return_code;

	ENTER(spectrum_editor_set_callback);
	return_code=0;
	/* check arguments */
	if (spectrum_editor&&callback)
	{
		spectrum_editor->update_callback.procedure=callback->procedure;
		spectrum_editor->update_callback.data=callback->data;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_set_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_set_callback */

struct Spectrum *spectrum_editor_get_spectrum(
	struct Spectrum_editor *spectrum_editor)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Returns the spectrum edited by the <spectrum_editor>.
==============================================================================*/
{
	struct Spectrum *spectrum;

	ENTER(spectrum_editor_get_spectrum);
	if (spectrum_editor)
	{
		spectrum = spectrum_editor->edit_spectrum;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_get_spectrum.  Invalid argument(s)");
		spectrum = (struct Spectrum *)NULL;
	}
	LEAVE;

	return (spectrum);
} /* spectrum_editor_get_spectrum */

int spectrum_editor_set_spectrum(
	struct Spectrum_editor *spectrum_editor, struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Set the <spectrum> to be edited by the <spectrum_editor>.
==============================================================================*/
{
	int return_code;
	struct Callback_data callback;

	ENTER(spectrum_editor_set_spectrum);
	if (spectrum_editor)
	{
		if (spectrum)
		{
			if (make_edit_spectrum(spectrum_editor,spectrum))
			{
				/* continue with the current_settings_type */
				spectrum_editor_make_settings_list(spectrum_editor);
				
				XtManageChild(spectrum_editor->widget);
				/* select the first settings item in the list (if any) */
				spectrum_editor->current_settings=(struct Spectrum_settings *)NULL;
				spectrum_editor_select_settings_item(spectrum_editor);
				/* turn on callbacks from settings editor */
				callback.procedure=spectrum_editor_update_settings;
				callback.data=(void *)spectrum_editor;
				spectrum_editor_settings_set_callback(
					spectrum_editor->settings_widget, &callback);
			}
			else
			{
				spectrum=(struct Spectrum *)NULL;
			}
		}
		if (!spectrum)
		{
			/* turn off settings editor by passing NULL settings */
			spectrum_editor->current_settings=(struct Spectrum_settings *)NULL;
			spectrum_editor_settings_set_settings(spectrum_editor->settings_widget,
				spectrum_editor->current_settings);
			XtUnmanageChild(spectrum_editor->widget);
			/* turn off callbacks from settings editors */
			callback.procedure=(Callback_procedure *)NULL;
			callback.data=(void *)NULL;
			spectrum_editor_settings_set_callback(spectrum_editor->settings_widget,&callback);
			if (spectrum_editor->edit_spectrum)
			{
				DESTROY(Spectrum)(&(spectrum_editor->edit_spectrum));
			}
		}
		spectrum_editor_update_scene_viewer(spectrum_editor);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_set_spectrum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_set_spectrum */

int spectrum_editor_refresh(struct Spectrum_editor *spectrum_editor)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Clears all the settings_changed flags globally (later) and in the list of
settings.
==============================================================================*/
{
	int return_code;

	ENTER(spectrum_editor_refresh);
	if (spectrum_editor)
	{
		if (spectrum_editor->edit_spectrum)
		{
			FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
				Spectrum_settings_clear_settings_changed,(void *)NULL,
				get_Spectrum_settings_list( spectrum_editor->edit_spectrum ));
			spectrum_editor_make_settings_list(spectrum_editor);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_refresh.  Missing edit_spectrum");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_refresh.  Missing spectrum_editor");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_refresh */

int spectrum_editor_update_changes(struct Spectrum_editor *spectrum_editor)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
This function is called to update the editor when other things, such as the
autorange button, have changed the edit spectrum.
==============================================================================*/
{
	int return_code;

	ENTER(spectrum_editor_update_changes);
	if (spectrum_editor)
	{
		if (spectrum_editor->edit_spectrum)
		{
			spectrum_editor_settings_set_settings(
				spectrum_editor->settings_widget,
				spectrum_editor->current_settings );
			spectrum_editor_make_settings_list(spectrum_editor);
			spectrum_editor_update_scene_viewer(spectrum_editor);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_update_changes.  Missing edit_spectrum");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_update_changes.  Missing spectrum_editor");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_update_changes */


