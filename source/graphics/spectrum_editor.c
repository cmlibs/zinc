/*******************************************************************************
FILE : spectrum_editor.c

LAST MODIFIED : 4 September 2000

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
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Contains all the information carried by the graphical element editor widget.
==============================================================================*/
{
	/* This editor_material is used when displaying the spectrum in the 
		3d widget */
	struct Graphical_material *editor_material;
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
	Widget *widget_address,widget,widget_parent;
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
		compile_Graphical_material(spectrum_editor->editor_material, NULL);
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

static void spectrum_editor_destroy_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Callback for the spectrum_editor dialog - tidies up all details - mem etc
==============================================================================*/
{
	struct Spectrum_editor *spectrum_editor;
	struct GT_pointset *tick_labels;

	ENTER(spectrum_editor_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* Get the pointer to the data for the widget */
		XtVaGetValues(widget,XmNuserData,&spectrum_editor,NULL);
		if (spectrum_editor)
		{
			DEACCESS(Graphical_material)(&spectrum_editor->editor_material);
			/* The strings in the labels graphics object are stored in two 
				ALLOCATED blocks instead of ALLOCATING each string individually.
				So I will manually DEALLOCATE the strings and set them to NULL */
			tick_labels = GT_OBJECT_GET(GT_pointset)(spectrum_editor->tick_labels_graphics_object, 0);
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
			/* destroy edit_spectrum */
			if (spectrum_editor->edit_spectrum)
			{
				DEACCESS(Spectrum)(
					&(spectrum_editor->edit_spectrum));
			}
			*(spectrum_editor->widget_address)=(Widget)NULL;
			DEALLOCATE(spectrum_editor);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_destroy_CB.  "
				"Missing spectrum_editor struct");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_destroy_CB.  Missing widget");
	}
	LEAVE;
} /* spectrum_editor_destroy_CB */

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

static void spectrum_editor_viewer_input_CB(
	Widget scene_viewer_widget,
	void *spectrum_editor_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Callback for when input is received by the scene_viewer.
==============================================================================*/
{
	struct Spectrum_editor *spectrum_editor;
	X3dThreeDDrawCallbackStruct *input_callback_data;
	XEvent *event;

	ENTER(spectrum_editor_viewer_input_CB);
	USE_PARAMETER(scene_viewer_widget);
	if ((spectrum_editor=(struct Spectrum_editor *)spectrum_editor_void)&&
		(input_callback_data=(X3dThreeDDrawCallbackStruct *)call_data)&&
		(X3dCR_INPUT==input_callback_data->reason)&&
		(event=(XEvent *)(input_callback_data->event)))
	{
		if (ButtonPress==event->type)
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

int create_Spectrum_colour_bar(struct GT_object **graphics_object_address,
	char *name,struct Spectrum *spectrum,Triple bar_centre,Triple bar_axis,
	Triple side_axis,float bar_length,float bar_radius,float extend_length,
	int tick_divisions,float tick_length,char *number_format,char *number_string,
	struct Graphical_material *bar_material,
	struct Graphical_material *tick_label_material)
/*******************************************************************************
LAST MODIFIED : 4 September 2000

DESCRIPTION :
Creates a coloured bar with annotation for displaying the scale of <spectrum>.
First makes a graphics object named <name> consisting of a cylinder of size
<bar_length>, <bar_radius> centred at <bar_centre> and aligned with the
<bar_axis>, combining the <bar_material> with the <spectrum>. The bar is
coloured by the current range of the spectrum. An extra <extend_length> is added
on to each end of the bar and given the colour the spectrum returns outside its
range at that end.
The <side_axis> must not be collinear with <bar_axis> and is used to calculate
points around the bar and the direction of the <tick_divisions>+1 ticks at which
spectrum values are written.
Attached to the bar graphics_object are two graphics objects using the
<tick_label_material>, one containing the ticks, the other the labels. The
labels are written using the <number_format>, printed into the <number_string>
which should be allocated large enough to hold it.
On successful return a pointer to the bar_graphics_object is put at
<*graphics_object_address>. If there is already a colour_bar at this address it
is cleared and redefined.

The side_axis is made to be orthogonal to the bar_axis, and both are made unit
length by this function.

???RC Function probably belongs elsewhere. We have module
finite_element_to_graphics_object, but how about a more generic module for
graphics_objects that don't come from finite_elements?
==============================================================================*/
{
	char **labels;
	float cos_theta,extend_fraction,half_final_length,length_factor,magnitude,
		sin_theta,spectrum_factor,spectrum_minimum,spectrum_maximum,theta,time,
		unit_factor;
	int allocated_labels,i,j,number_of_ticks,points_along_bar,points_around_bar,
		return_code;
	GTDATA spectrum_value,*data,*datum;
	struct GT_object *bar_graphics_object,*label_graphics_object,
		*tick_graphics_object;
	struct GT_pointset *pointset;
	struct GT_polyline *polyline;
	struct GT_surface *surface;
	Triple base,centre,front_axis,*point,*points,*normal,*normalpoints,
		scaled_axis,scaled_front,scaled_side;

	ENTER(create_Spectrum_colour_bar);
	if (graphics_object_address&&name&&spectrum&&centre&&bar_axis&&side_axis&&
		(0.0<bar_length)&&(0.0<bar_radius)&&(0.0<=extend_length)&&
		(0<tick_divisions)&&(100>=tick_divisions)&&(0.0<=tick_length)&&
		number_format&&number_string&&bar_material&&tick_label_material)
	{
		return_code=1;
		/* add all graphics objects at time 0.0 */
		time = 0.0;
		/* calculate and get range of spectrum */
		Spectrum_calculate_range(spectrum);
		spectrum_minimum=get_Spectrum_minimum(spectrum);
		spectrum_maximum=get_Spectrum_maximum(spectrum);
		/* get orthogonal unit vectors along bar_axis, side and front */
		if (0.0<(magnitude=sqrt(bar_axis[0]*bar_axis[0]+bar_axis[1]*bar_axis[1]+
			bar_axis[2]*bar_axis[2])))
		{
			bar_axis[0] /= magnitude;
			bar_axis[1] /= magnitude;
			bar_axis[2] /= magnitude;
			/* get front = unit cross product of bar_axis and side_axis */
			front_axis[0]=bar_axis[1]*side_axis[2] - bar_axis[2]*side_axis[1];
			front_axis[1]=bar_axis[2]*side_axis[0] - bar_axis[0]*side_axis[2];
			front_axis[2]=bar_axis[0]*side_axis[1] - bar_axis[1]*side_axis[0];
			if (0.0<(magnitude=sqrt(front_axis[0]*front_axis[0]+
				front_axis[1]*front_axis[1]+front_axis[2]*front_axis[2])))
			{
				front_axis[0] /= magnitude;
				front_axis[1] /= magnitude;
				front_axis[2] /= magnitude;
				/* now get side_axis = front_axis (x) bar_axis */
				side_axis[0]=front_axis[1]*bar_axis[2] - front_axis[2]*bar_axis[1];
				side_axis[1]=front_axis[2]*bar_axis[0] - front_axis[0]*bar_axis[2];
				side_axis[2]=front_axis[0]*bar_axis[1] - front_axis[1]*bar_axis[0];
			}
			else
			{
				display_message(ERROR_MESSAGE,"create_Spectrum_colour_bar.  "
					"side axis (tick direction) is in-line with bar axis");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Spectrum_colour_bar.  Invalid bar axis");
			return_code=0;
		}
		if (return_code)
		{
			if (*graphics_object_address)
			{
				/* check whether the existing graphics object was really a colour_bar */
				bar_graphics_object = *graphics_object_address;
				if ((g_SURFACE == bar_graphics_object->object_type) &&
					(0 == strcmp(bar_graphics_object->name,name)) &&
					(tick_graphics_object = bar_graphics_object->nextobject) &&
					(g_POLYLINE == tick_graphics_object->object_type) &&
					(label_graphics_object = tick_graphics_object->nextobject) &&
					(g_POINTSET == label_graphics_object->object_type))
				{
					GT_object_delete_time(bar_graphics_object,time);
					GT_object_delete_time(tick_graphics_object,time);
					GT_object_delete_time(label_graphics_object,time);
					/* valid existing colour_bar */
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Spectrum_colour_bar.  Invalid existing colour_bar");
					return_code=0;
				}
			}
			else
			{
				/* new colour_bar: create and link the bar, ticks and labels */
				if ((bar_graphics_object=
					CREATE(GT_object)(name,g_SURFACE,bar_material))&&
					(tick_graphics_object=
						CREATE(GT_object)("ticks",g_POLYLINE,tick_label_material))&&
					(label_graphics_object=
						CREATE(GT_object)("labels",g_POINTSET,tick_label_material)))
				{
					bar_graphics_object->nextobject =
						ACCESS(GT_object)(tick_graphics_object);
					tick_graphics_object->nextobject =
						ACCESS(GT_object)(label_graphics_object);
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Spectrum_colour_bar.  Could not create graphics objects");
					if (bar_graphics_object)
					{
						DESTROY(GT_object)(&bar_graphics_object);
						if (tick_graphics_object)
						{
							DESTROY(GT_object)(&tick_graphics_object);
						}
					}
					return_code=0;
				}
			}
		}
		if (return_code)
		{
			/* create the colour bar */
			points=(Triple *)NULL;
			data=(GTDATA *)NULL;
			surface=(struct GT_surface *)NULL;
			points_along_bar = 109;
			points_around_bar = 25;
			if (ALLOCATE(points,Triple,points_along_bar*points_around_bar)&&
				ALLOCATE(normalpoints,Triple,points_along_bar*points_around_bar)&&
				ALLOCATE(data,GTDATA,points_along_bar*points_around_bar))
			{
				extend_fraction=extend_length/bar_length;
				half_final_length=0.5*bar_length+extend_length;
				scaled_axis[0]=half_final_length*bar_axis[0];
				scaled_axis[1]=half_final_length*bar_axis[1];
				scaled_axis[2]=half_final_length*bar_axis[2];
				point=points;
				normal=normalpoints;
				datum=data;
				for (i=0;i<points_along_bar;i++)
				{
#if defined (OLD_CODE)
					point=points+i;
					normal=point+points_along_bar*points_around_bar;
					datum=data+i;
#endif /* defined (OLD_CODE) */
					/* get factor ranging from 0.0 to 1.0 over extended length */
					unit_factor=(float)i/(float)(points_along_bar-1);
					/* get factor ranging from -1 to +1 along bar */
					length_factor=2.0*unit_factor-1.0;
					/* get centre of bar for ith point */
					centre[0]=bar_centre[0] + length_factor*scaled_axis[0];
					centre[1]=bar_centre[1] + length_factor*scaled_axis[1];
					centre[2]=bar_centre[2] + length_factor*scaled_axis[2];
					/* get factor ranging from 0 to 1 over the bar_length and extending
						 outside that range in the extend_length */
					spectrum_factor=unit_factor*(1.0+2.0*extend_fraction)-extend_fraction;
					/* interpolate to get spectrum value */
					spectrum_value = (GTDATA)((1.0-spectrum_factor)*spectrum_minimum +
						spectrum_factor*spectrum_maximum);
					for (j=0;j<points_around_bar;j++)
					{
						theta=(float) j * 2.0 * PI / (float)(points_around_bar-1);
						cos_theta=cos(theta);
						sin_theta=sin(theta);
						scaled_side[0]=cos_theta*side_axis[0];
						scaled_side[1]=cos_theta*side_axis[1];
						scaled_side[2]=cos_theta*side_axis[2];
						scaled_front[0]=sin_theta*front_axis[0];
						scaled_front[1]=sin_theta*front_axis[1];
						scaled_front[2]=sin_theta*front_axis[2];
						/* set point */
						(*point)[0]=centre[0]+bar_radius*(scaled_side[0]+scaled_front[0]);
						(*point)[1]=centre[1]+bar_radius*(scaled_side[1]+scaled_front[1]);
						(*point)[2]=centre[2]+bar_radius*(scaled_side[2]+scaled_front[2]);
						point++;
						/* set normal */
						(*normal)[0]=scaled_side[0]+scaled_front[0];
						(*normal)[1]=scaled_side[1]+scaled_front[1];
						(*normal)[2]=scaled_side[2]+scaled_front[2];
						normal++;
						/* set data */
						*datum=spectrum_value;
						datum++;
					}
				}
				return_code=(
					(surface=CREATE(GT_surface)(g_SHADED,g_QUADRILATERAL,
						points_around_bar,points_along_bar,points,normalpoints,
						/*texturepoints*/(Triple *)NULL,/*n_data_components*/1,data)) &&
					set_GT_object_default_material(bar_graphics_object,bar_material) &&
					set_GT_object_Spectrum(bar_graphics_object,spectrum)&&
					GT_OBJECT_ADD(GT_surface)(bar_graphics_object,time,surface));
			}
			else
			{
				return_code=0;
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"create_Spectrum_colour_bar.  Could not build spectrum bar");
				if (surface)
				{
					DESTROY(GT_surface)(&surface);
				}
				else
				{
					DEALLOCATE(points);
					DEALLOCATE(data);
				}
			}
		}
		/* get values for calculating tick and label positions */
		number_of_ticks=tick_divisions+1;
		scaled_axis[0]=bar_length*bar_axis[0];
		scaled_axis[1]=bar_length*bar_axis[1];
		scaled_axis[2]=bar_length*bar_axis[2];
		base[0]=bar_centre[0]-0.5*scaled_axis[0]+bar_radius*side_axis[0];
		base[1]=bar_centre[1]-0.5*scaled_axis[1]+bar_radius*side_axis[1];
		base[2]=bar_centre[2]-0.5*scaled_axis[2]+bar_radius*side_axis[2];
		scaled_side[0]=tick_length*side_axis[0];
		scaled_side[1]=tick_length*side_axis[1];
		scaled_side[2]=tick_length*side_axis[2];
		if (return_code)
		{
			/* create the scale ticks */
			points=(Triple *)NULL;
			polyline=(struct GT_polyline *)NULL;
			if (ALLOCATE(points,Triple,2*number_of_ticks))
			{
				point=points;
				for (i=0;i<number_of_ticks;i++)
				{
					/* get factor ranging from 0.0 to 1.0 over bar_length */
					unit_factor=(float)i/(float)(number_of_ticks-1);
					(*point)[0]=base[0]+unit_factor*scaled_axis[0];
					(*point)[1]=base[1]+unit_factor*scaled_axis[1];
					(*point)[2]=base[2]+unit_factor*scaled_axis[2];
					(*(point+1))[0]=(*point)[0]+scaled_side[0];
					(*(point+1))[1]=(*point)[1]+scaled_side[1];
					(*(point+1))[2]=(*point)[2]+scaled_side[2];
					point += 2;
				}
				return_code=(
					(polyline=CREATE(GT_polyline)(g_PLAIN_DISCONTINUOUS,number_of_ticks,
						points,/*normalpoints*/(Triple *)NULL,/*n_data_components*/0,
						(GTDATA *)NULL))&&
					set_GT_object_default_material(tick_graphics_object,
						tick_label_material) &&
					GT_OBJECT_ADD(GT_polyline)(tick_graphics_object,time,polyline));
			}
			else
			{
				return_code=0;
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"create_Spectrum_colour_bar.  Could not build scale ticks");
				if (polyline)
				{
					DESTROY(GT_polyline)(&polyline);
				}
				else
				{
					DEALLOCATE(points);
				}
			}
		}
		/* labels are drawn at the ends of the ticks */
		base[0] += scaled_side[0];
		base[1] += scaled_side[1];
		base[2] += scaled_side[2];
		if (return_code)
		{
			/* create the scale labels */
			points=(Triple *)NULL;
			labels=(char **)NULL;
			pointset=(struct GT_pointset *)NULL;
			allocated_labels=0;
			if (ALLOCATE(points,Triple,number_of_ticks)&&
				ALLOCATE(labels,char *,number_of_ticks))
			{
				point=points;
				for (i=0;return_code&&(i<number_of_ticks);i++)
				{
					/* get factor ranging from 0.0 to 1.0 over bar_length */
					unit_factor=(float)i/(float)(number_of_ticks-1);
					(*point)[0]=base[0]+unit_factor*scaled_axis[0];
					(*point)[1]=base[1]+unit_factor*scaled_axis[1];
					(*point)[2]=base[2]+unit_factor*scaled_axis[2];
					point++;
					/* interpolate to get spectrum value */
					spectrum_value=(GTDATA)((1.0-unit_factor)*spectrum_minimum +
						unit_factor*spectrum_maximum);
					sprintf(number_string,number_format,spectrum_value);
					if (ALLOCATE(labels[i],char,strlen(number_string)+1))
					{
						allocated_labels++;
						strcpy(labels[i],number_string);
					}
					else
					{
						return_code=0;
					}
				}
				return_code=((pointset=
					CREATE(GT_pointset)(number_of_ticks,points,labels,g_NO_MARKER,0.0,
						/*n_data_components*/0,(GTDATA *)NULL,(int *)NULL)) &&
					set_GT_object_default_material(label_graphics_object,
						tick_label_material) &&
					GT_OBJECT_ADD(GT_pointset)(label_graphics_object,time,pointset));
			}
			else
			{
				return_code=0;
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"create_Spectrum_colour_bar.  Could not build scale labels");
				if (pointset)
				{
					DESTROY(GT_pointset)(&pointset);
				}
				else
				{
					DEALLOCATE(points);
					for (i=0;i<allocated_labels;i++)
					{
						DEALLOCATE(labels[i]);
					}
					DEALLOCATE(labels);
				}
			}
			if (!(*graphics_object_address))
			{
				if (return_code)
				{
					*graphics_object_address = bar_graphics_object;
				}
				else
				{
					DESTROY(GT_object)(&bar_graphics_object);
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Spectrum_colour_bar.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* create_Spectrum_colour_bar */

Widget create_spectrum_editor_widget(Widget *spectrum_editor_widget,
	Widget parent,struct Spectrum *spectrum,
	struct User_interface *user_interface,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(Light) *light_manager,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct MANAGER(Texture) *texture_manager)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Creates a spectrum_editor widget.
???RC Currently not using the scene_manager; hence left warning on.
==============================================================================*/
{
	int i,j,return_code,surface_discretise_xi1=24,surface_discretise_xi2=108;
	GTDATA *data;
	MrmType spectrum_editor_dialog_class;
	struct Spectrum_editor *spectrum_editor=NULL;
	struct Callback_data callback;
	static MrmRegisterArg callback_list[]=
	{
		{"spec_ed_destroy_CB",(XtPointer)spectrum_editor_destroy_CB},
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
	struct Light *viewer_light;
	struct Light_model *viewer_light_model;
	Widget return_widget;
	Triple *points, *normalpoints;
	float value_xi1, value_xi2, light_direction[3] = {0, -0.2, -1.0};
	struct GT_surface *cylinder_surface;
	struct GT_polyline *tick_lines;
	struct GT_pointset *tick_labels;
	struct Graphical_material *tick_material;
	char **strings, *string_data;
	struct Spectrum *default_scene_spectrum;

	ENTER(create_spectrum_editor_widget);
	return_widget=(Widget)NULL;
	/* check arguments */
	if (spectrum_editor_widget&&parent&&user_interface)
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
				spectrum_editor->widget_address=spectrum_editor_widget;
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
									"create_spectrum_editor_widget.  "
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
									compile_Graphical_material(spectrum_editor->editor_material, NULL);
									tick_material = CREATE(Graphical_material)("editor_material");
									Graphical_material_set_ambient(tick_material, &off_white );
									Graphical_material_set_diffuse(tick_material, &off_white );
									Graphical_material_set_shininess(tick_material, 0.8 );
									compile_Graphical_material(tick_material, NULL);
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
												points, normalpoints, /*texturepoints*/(Triple *)NULL,
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
													"create_spectrum_editor_widget. Unable to create surface");
											}
										}
										else
										{
											DEALLOCATE( points );
											DEALLOCATE( data );
											return_code = 0;
											display_message(ERROR_MESSAGE,
												"create_spectrum_editor_widget. Unable to create graphics_object");
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
													"create_spectrum_editor_widget. Unable to create lines");
											}
										}
										else
										{
											return_code = 0;
											display_message(ERROR_MESSAGE,
												"create_spectrum_editor_widget. Unable to create tick line graphics_object");
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
													"create_spectrum_editor_widget. Unable to create tick label pointset");
											}
										}
										else
										{
											return_code = 0;
											display_message(ERROR_MESSAGE,
												"create_spectrum_editor_widget. Unable to create tick label graphics_object");
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
										spectrum_editor->spectrum_editor_scene_viewer = CREATE(Scene_viewer)(
											spectrum_editor->viewer_form,
											&background_colour,SCENE_VIEWER_DOUBLE_BUFFER,
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
										callback.procedure = spectrum_editor_viewer_input_CB;
										callback.data = (void *)spectrum_editor;
										Scene_viewer_set_input_callback(
											spectrum_editor->spectrum_editor_scene_viewer,&callback);
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
								if (spectrum)
								{
									spectrum_editor_set_Spectrum(
										spectrum_editor->widget,spectrum);
								}
								return_widget=spectrum_editor->widget;
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
								"create_spectrum_editor_widget.  "
								"Could not fetch spectrum_editor widget");
							DEALLOCATE(spectrum_editor);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_spectrum_editor_widget.  "
							"Could not register identifiers");
						DEALLOCATE(spectrum_editor);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_spectrum_editor_widget.  "
						"Could not register callbacks");
					DEALLOCATE(spectrum_editor);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_spectrum_editor_widget.  "
					"Could not allocate spectrum_editor widget structure");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_spectrum_editor_widget.  Could not open hierarchy");
		}
		*spectrum_editor_widget=return_widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_spectrum_editor_widget.  Invalid argument(s)");
	}
	LEAVE;

	return (return_widget);
} /* create_spectrum_editor_widget */

struct Callback_data *spectrum_editor_get_callback(
	Widget spectrum_editor_widget)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Returns a pointer to the update_callback item of the
spectrum_editor_widget.
==============================================================================*/
{
	struct Callback_data *return_address;
	struct Spectrum_editor *spectrum_editor;

	ENTER(spectrum_editor_get_callback);
	/* check arguments */
	if (spectrum_editor_widget)
	{
		/* Get the pointer to the data for the dialog */
		XtVaGetValues(spectrum_editor_widget,XmNuserData,
			&spectrum_editor,NULL);
		if (spectrum_editor)
		{
			return_address=&(spectrum_editor->update_callback);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_get_callback.  Missing widget data");
			return_address=(struct Callback_data *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_get_callback.  Missing widget");
		return_address=(struct Callback_data *)NULL;
	}
	LEAVE;

	return (return_address);
} /* spectrum_editor_get_callback */

int spectrum_editor_set_callback(
	Widget spectrum_editor_widget,struct Callback_data *new_callback)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Changes the callback function for the spectrum_editor_widget, which
will be called when the spectrum changes in any way.
==============================================================================*/
{
	int return_code;
	struct Spectrum_editor *spectrum_editor;

	ENTER(spectrum_editor_set_callback);
	/* check arguments */
	if (spectrum_editor_widget&&new_callback)
	{
		/* Get the pointer to the data for the choose_settings dialog */
		XtVaGetValues(spectrum_editor_widget,XmNuserData,
			&spectrum_editor,NULL);
		if (spectrum_editor)
		{
			spectrum_editor->update_callback.procedure=new_callback->procedure;
			spectrum_editor->update_callback.data=new_callback->data;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_set_callback.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_set_callback.  Missing widget");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_set_callback */

int spectrum_editor_set_Spectrum(
	Widget spectrum_editor_widget,
	struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Sets the spectrum to be edited by the spectrum_editor widget.
==============================================================================*/
{
	int return_code;
	struct Callback_data callback;
	struct Spectrum_editor *spectrum_editor;

	ENTER(spectrum_editor_set_Spectrum);
	/* check arguments */
	if (spectrum_editor_widget)
	{
		/* Get the pointer to the data for the spectrum_editor_widget */
		XtVaGetValues(spectrum_editor_widget,XmNuserData,&spectrum_editor,
			NULL);
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
						spectrum_editor->settings_widget,&callback);
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
				"spectrum_editor_set_Spectrum.  "
				"Missing spectrum_editor struct");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_set_Spectrum.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_set_Spectrum */

struct Spectrum *spectrum_editor_get_Spectrum(
	Widget spectrum_editor_widget)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Returns the spectrum currently being edited.
==============================================================================*/
{
	struct Spectrum *return_address;
	struct Spectrum_editor *spectrum_editor;

	ENTER(spectrum_editor_get_spectrum);
	/* check arguments */
	if (spectrum_editor_widget)
	{
		/* Get the pointer to the data for the dialog */
		XtVaGetValues(spectrum_editor_widget,XmNuserData,
			&spectrum_editor,NULL);
		if (spectrum_editor)
		{
			return_address=spectrum_editor->edit_spectrum;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_get_spectrum.  Missing widget data");
			return_address=(struct Spectrum *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_get_spectrum.  Missing widget");
		return_address=(struct Spectrum *)NULL;
	}
	LEAVE;

	return (return_address);
} /* spectrum_editor_get_spectrums */

int spectrum_editor_refresh(Widget spectrum_editor_widget)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Clears all the settings_changed flags globally (later) and in the list of
settings.
==============================================================================*/
{
	int return_code;
	struct Spectrum_editor *spectrum_editor;

	ENTER(spectrum_editor_refresh);
	/* check arguments */
	if (spectrum_editor_widget)
	{
		/* Get the pointer to the data for the choose_settings dialog */
		XtVaGetValues(spectrum_editor_widget,XmNuserData,
			&spectrum_editor,NULL);
		if (spectrum_editor&&spectrum_editor->edit_spectrum)
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
				"spectrum_editor_refresh.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_refresh.  Missing widget");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_refresh */

int spectrum_editor_update_changes(Widget spectrum_editor_widget)
/*******************************************************************************
LAST MODIFIED : 23 July 1998
DESCRIPTION :
This function is called to update the editor when other
things (such as the autorange button) have changed the
edit spectrum.
==============================================================================*/
{
	int return_code;
	struct Spectrum_editor *spectrum_editor;

	ENTER(spectrum_editor_update_changes);
	/* check arguments */
	if (spectrum_editor_widget)
	{
		XtVaGetValues(spectrum_editor_widget,XmNuserData,
			&spectrum_editor,NULL);
		if (spectrum_editor&&spectrum_editor->edit_spectrum)
		{
			spectrum_editor_settings_set_settings(
				spectrum_editor->settings_widget,
				spectrum_editor->current_settings );
			spectrum_editor_make_settings_list(spectrum_editor);
			spectrum_editor_update_scene_viewer(
				spectrum_editor );
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_update_changes.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_update_changes.  Missing widget");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_update_changes */


