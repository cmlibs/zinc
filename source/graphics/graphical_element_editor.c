/*******************************************************************************
FILE : graphical_element_editor.c

LAST MODIFIED : 22 December 1999

DESCRIPTION :
Provides the widgets to manipulate graphical element group settings.
==============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include "choose/choose_computed_field.h"
#include "choose/choose_enumerator.h"
#include "choose/choose_fe_field.h"
#include "command/parser.h"
#include "general/debug.h"
#include "graphics/graphical_element.h"
#include "graphics/graphical_element_editor.h"
#include "graphics/graphical_element_editor.uidh"
#include "graphics/graphics_object.h"
#include "graphics/settings_editor.h"
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int gelem_editor_hierarchy_open=0;
static MrmHierarchy gelem_editor_hierarchy;
#endif /* defined (MOTIF) */

struct Graphical_element_editor_struct
/*******************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Contains all the information carried by the graphical element editor widget.
==============================================================================*/
{
	struct Computed_field_package *computed_field_package;
	struct GT_element_group *edit_gt_element_group;
	struct Graphical_material *default_material;
	struct LIST(GT_object) *glyph_list;
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(Graphical_material) *graphical_material_manager;
	void *material_manager_callback_id;
	struct Spectrum *default_spectrum;
	struct MANAGER(Spectrum) *spectrum_manager;
	void *spectrum_manager_callback_id;
	struct MANAGER(VT_volume_texture) *volume_texture_manager;
	/*???RC implement later */
	void *volume_texture_manager_callback_id;
	struct User_interface *user_interface;
	enum GT_element_settings_type current_settings_type;
	int current_dimension;
	struct GT_element_settings *current_settings;
	struct Callback_data update_callback;
	Widget general_button,general_rowcol,default_coordinate_field_form,
		default_coordinate_field_widget,element_disc_text,
		circle_disc_text,native_discretization_button,
		native_discretization_field_form,native_discretization_field_widget,
		dimension_0_button,dimension_1_button,dimension_2_button,
		dimension_3_button,dimension_all_button,settings_type_form,
		settings_type_widget,settings_scroll,settings_rowcol,add_button,
		delete_button,up_button,down_button,settings_form,settings_widget,
		cylinders_button;
	Widget *widget_address,widget,widget_parent;
}; /* Graphical_element_editor_struct */

/*
Module functions
----------------
*/
static int make_edit_gt_element_group(
	struct Graphical_element_editor_struct *gelem_editor,
	struct GT_element_group *gt_element_group)
/*******************************************************************************
LAST MODIFIED : 20 April 1999

DESCRIPTION :
Destroys the edit_gt_element_group member of <gelem_editor> and rebuilds it as
a complete copy of <gt_element_group>.
==============================================================================*/
{
	int return_code;

	ENTER(make_edit_gt_element_group);
	/* check arguments */
	if (gelem_editor&&gt_element_group)
	{
		/* destroy current edit_gt_element_group */
		if (gelem_editor->edit_gt_element_group)
		{
			DESTROY(GT_element_group)(&(gelem_editor->edit_gt_element_group));
		}
		/* make an empty GT_element_group */
		if (gelem_editor->edit_gt_element_group=CREATE(GT_element_group)(
			GT_element_group_get_element_group(gt_element_group),
			GT_element_group_get_node_group(gt_element_group),
			GT_element_group_get_data_group(gt_element_group)))
		{
			/* copy settings WITHOUT graphics objects */
			return_code=GT_element_group_copy(gelem_editor->edit_gt_element_group,
				gt_element_group);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"make_edit_gt_element_group.  Could not make copy of gt_element_group");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"make_edit_gt_element_group.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* make_edit_gt_element_group */

static int create_settings_item_widget(struct GT_element_settings *settings,
	void *gelem_editor_void)
/*******************************************************************************
LAST MODIFIED : 14 September 1998

DESCRIPTION :
Clears then fills the settings list RowCol with descriptions of the settings
of the type gelem_editor->settings_type.
==============================================================================*/
{
	int num_children,return_code;
	struct Graphical_element_editor_struct *gelem_editor;
	char *settings_string;
	XmString new_string;
	Arg override_arg;
	MrmType settings_item_class;
	Widget temp_widget;
	Widget *child_list;

	ENTER(create_settings_item_widget);
	/* check arguments */
	if (settings&&(gelem_editor=
		(struct Graphical_element_editor_struct *)gelem_editor_void))
	{
		if (gelem_editor_hierarchy_open)
		{
			if (GT_element_settings_uses_dimension(settings,
				(void *)&gelem_editor->current_dimension))
			{
				if (settings_string=GT_element_settings_string(settings,
					SETTINGS_STRING_COMPLETE_PLUS))
				{
					XtSetArg(override_arg,XmNuserData,settings);
					temp_widget=(Widget)NULL;
					if (MrmSUCCESS==MrmFetchWidgetOverride(gelem_editor_hierarchy,
						"gelem_ed_settings_item_form",gelem_editor->settings_rowcol,NULL,
						&override_arg,1,&temp_widget,&settings_item_class))
					{
						XtManageChild(temp_widget);
						/* get the children = visibility and select toggle buttons */
						XtVaGetValues(temp_widget,XmNnumChildren,&num_children,
							XmNchildren,&child_list,NULL);
						if (2==num_children)
						{
							/* set the visibility toggle button */
							if (GT_element_settings_get_visibility(settings))
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
							if (settings==gelem_editor->current_settings)
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
				return_code=1;
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

static int gelem_editor_make_settings_list(
	struct Graphical_element_editor_struct *gelem_editor)
/*******************************************************************************
LAST MODIFIED : 9 June 1998

DESCRIPTION :
Clears then fills the settings list RowColumn with descriptions of the settings
of the type gelem_editor->settings_type.
==============================================================================*/
{
	Arg override_arg;
	int return_code;
	MrmType settings_rowcol_class;
	Widget temp_widget;

	ENTER(gelem_editor_make_settings_list);
	/* check arguments */
	if (gelem_editor)
	{
		if (gelem_editor_hierarchy_open)
		{
			/* delete the rowcol and all the items it contains */
			if (gelem_editor->settings_rowcol)
			{
				XtDestroyWidget(gelem_editor->settings_rowcol);
			}
			/* recreate the rowcol */
			XtSetArg(override_arg,XmNuserData,gelem_editor);
			temp_widget=(Widget)NULL;
			if (MrmSUCCESS==MrmFetchWidgetOverride(gelem_editor_hierarchy,
				"gelem_ed_settings_rowcol",gelem_editor->settings_scroll,NULL,
				&override_arg,1,&temp_widget,&settings_rowcol_class))
			{
				/* now insert the new items/widgets */
				for_each_settings_in_GT_element_group(
					gelem_editor->edit_gt_element_group,
					create_settings_item_widget,(void *)gelem_editor);
				XtManageChild(temp_widget);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gelem_editor_make_settings_list.  Could not fetch rowcol");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gelem_editor_make_settings_list.  Hierarchy not open");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gelem_editor_make_settings_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gelem_editor_make_settings_list */

static int gelem_editor_select_settings_item(
	struct Graphical_element_editor_struct *gelem_editor)
/*******************************************************************************
LAST MODIFIED : 14 September 1998

DESCRIPTION :
Checks if current_settings is in the settings_rowcol; if not (or it was NULL)
the first item in this list becomes the current_settings, or NULL if empty.
The line/settings editors for the previous settings are then Unmanaged, and that
for the new settings Managed and filled with the new values.
If current_settings is NULL, no editing fields are displayed.
==============================================================================*/
{
	enum GT_element_settings_type settings_type;
	int i,num_children,num_toggles,return_code;
	struct GT_element_settings *temp_settings;
	WidgetList child_list,toggle_list;

	ENTER(gelem_editor_select_settings_item);
	if (gelem_editor)
	{
		settings_type=gelem_editor->current_settings_type;
		/* get list of settings items */
		XtVaGetValues(gelem_editor->settings_rowcol,XmNnumChildren,&num_children,
			XmNchildren,&child_list,NULL);
		if (0<num_children)
		{
			for (i=0;i<num_children;i++)
			{
				XtVaGetValues(child_list[i],XmNuserData,&temp_settings,
					XmNnumChildren,&num_toggles,XmNchildren,&toggle_list,NULL);
				if ((2==num_toggles)&&GT_element_settings_uses_dimension(
					temp_settings,(void *)&gelem_editor->current_dimension))
				{
					if ((!gelem_editor->current_settings)&&
						((GT_ELEMENT_SETTINGS_TYPE_INVALID==settings_type)||
						GT_element_settings_type_matches(temp_settings,
							(void *)settings_type)))
					{
						gelem_editor->current_settings=temp_settings;
					}
					if (temp_settings==gelem_editor->current_settings)
					{
						XtVaSetValues(toggle_list[1],XmNset,True,NULL);
						settings_type=GT_element_settings_get_settings_type(temp_settings);
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
			gelem_editor->current_settings=(struct GT_element_settings *)NULL;
		}
		i=((struct GT_element_settings *)NULL != gelem_editor->current_settings);
		/* Grey delete and move buttons if no current_settings */
		XtSetSensitive(gelem_editor->delete_button,i);
		XtSetSensitive(gelem_editor->up_button,i);
		XtSetSensitive(gelem_editor->down_button,i);
		/* make sure settings_type is valid */
		if (GT_ELEMENT_SETTINGS_TYPE_INVALID==settings_type)
		{
			settings_type=GT_ELEMENT_SETTINGS_TYPE_BEFORE_FIRST;
			settings_type++;
			if (-1 != gelem_editor->current_dimension)
			{
				while (!GT_element_settings_type_uses_dimension(settings_type,
					gelem_editor->current_dimension))
				{
					settings_type++;
				}
			}
		}
		/* if settings_type changed select it on the settings_type option menu */
		if (settings_type != gelem_editor->current_settings_type)
		{
			gelem_editor->current_settings_type=settings_type;
			choose_enumerator_set_string(gelem_editor->settings_type_widget,
				GT_element_settings_type_string(settings_type));
		}
		/* send selected object to settings editor */
		return_code=settings_editor_set_settings(gelem_editor->settings_widget,
			gelem_editor->current_settings);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gelem_editor_select_settings_item.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gelem_editor_select_settings_item */

static int gelem_editor_update_settings_item(
	struct Graphical_element_editor_struct *gelem_editor,
	struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 23 July 1998

DESCRIPTION :
Updates the label on item <settings> to match its contents.
==============================================================================*/
{
	char *settings_string;
	int i,num_children,num_toggles,return_code;
	struct GT_element_settings *temp_settings;
	WidgetList child_list,toggle_list;
	XmString new_string;

	ENTER(gelem_editor_update_settings_item);
	return_code=0;
	if (gelem_editor&&settings)
	{
		XtVaGetValues(gelem_editor->settings_rowcol,XmNnumChildren,&num_children,
			XmNchildren,&child_list,NULL);
		for (i=0;i<num_children;i++)
		{
			XtVaGetValues(child_list[i],XmNuserData,&temp_settings,
				XmNnumChildren,&num_toggles,XmNchildren,&toggle_list,NULL);
			if ((2==num_toggles)&&(temp_settings==settings))
			{
				if (settings_string=GT_element_settings_string(settings,
					SETTINGS_STRING_COMPLETE_PLUS))
				{
					/* make the settings_string the name of this item */
					new_string=XmStringCreateSimple(settings_string);
					XtVaSetValues(toggle_list[1],XmNlabelString,new_string,NULL);
					XmStringFree(new_string);
					return_code=1;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gelem_editor_update_settings_item.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* gelem_editor_update_settings_item */

static int graphical_element_editor_update(
	struct Graphical_element_editor_struct *gelem_editor)
/*******************************************************************************
LAST MODIFIED : 4 March 1999

DESCRIPTION :
Tells CMGUI about the current values. Returns a pointer to the
gt_element_group currently being edited.
==============================================================================*/
{
	int return_code;

	ENTER(graphical_element_editor_update);
	if (gelem_editor)
	{
		/* Now send an update to the client if requested */
		if ((gelem_editor->update_callback).procedure)
		{
			(gelem_editor->update_callback.procedure)(
				gelem_editor->widget,gelem_editor->update_callback.data,
				gelem_editor->edit_gt_element_group);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_update.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* graphical_element_editor_update */

DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor_struct,general_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor_struct,general_rowcol)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor_struct,default_coordinate_field_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor_struct,element_disc_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor_struct,circle_disc_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor_struct,native_discretization_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor_struct,native_discretization_field_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor_struct,dimension_0_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor_struct,dimension_1_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor_struct,dimension_2_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor_struct,dimension_3_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor_struct,dimension_all_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor_struct,settings_type_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor_struct,add_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor_struct,delete_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor_struct,up_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor_struct,down_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor_struct,settings_scroll)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor_struct,settings_rowcol)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor_struct,settings_form)

static void graphical_element_editor_destroy_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 August 1999

DESCRIPTION :
Callback for the gelem_editor dialog - tidies up all details - mem etc
==============================================================================*/
{
	struct Graphical_element_editor_struct *gelem_editor;

	ENTER(graphical_element_editor_destroy_CB);
	USE_PARAMETER(call_data);
	if (widget&&
		(gelem_editor=(struct Graphical_element_editor_struct *)client_data))
	{
		/* destroy edit_gt_element_group */
		if (gelem_editor->edit_gt_element_group)
		{
			DESTROY(GT_element_group)(
				&(gelem_editor->edit_gt_element_group));
		}
		/* deregister material manager callbacks */
		if (gelem_editor->material_manager_callback_id)
		{
			MANAGER_DEREGISTER(Graphical_material)(
				gelem_editor->material_manager_callback_id,
				gelem_editor->graphical_material_manager);
			gelem_editor->material_manager_callback_id=(void *)NULL;
		}
		/* deregister spectrum manager callbacks */
		if (gelem_editor->spectrum_manager_callback_id)
		{
			MANAGER_DEREGISTER(Spectrum)(
				gelem_editor->spectrum_manager_callback_id,
				gelem_editor->spectrum_manager);
			gelem_editor->spectrum_manager_callback_id=(void *)NULL;
		}
		*(gelem_editor->widget_address)=(Widget)NULL;
		DEALLOCATE(gelem_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_destroy_CB.  Missing widget");
	}
	LEAVE;
} /* graphical_element_editor_destroy_CB */

static int gelem_editor_set_general_settings(
	struct Graphical_element_editor_struct *gelem_editor)
/*******************************************************************************
LAST MODIFIED : 4 March 1999

DESCRIPTION :
Copies values of general settings into the general settings widgets. Also
controls graying out widgets not currently in use.
==============================================================================*/
{
	char temp_string[80];
	int circle_discretization,field_set,return_code;
	struct Computed_field *default_coordinate_field;
	struct Element_discretization element_discretization;
	struct FE_field *native_discretization_field;
	struct GT_element_group *gt_element_group;

	ENTER(gelem_editor_set_general_settings);
	if (gelem_editor&&(gt_element_group=gelem_editor->edit_gt_element_group))
	{
		/* default_coordinate_field */
		default_coordinate_field=
			GT_element_group_get_default_coordinate_field(gt_element_group);
		CHOOSE_OBJECT_SET_OBJECT(Computed_field)(
			gelem_editor->default_coordinate_field_widget,default_coordinate_field);
		/* circle_discretization */
		circle_discretization=GT_element_group_get_circle_discretization(
			gelem_editor->edit_gt_element_group);
		sprintf(temp_string,"%d",circle_discretization);
		XtVaSetValues(gelem_editor->circle_disc_text,XmNvalue,temp_string,NULL);
		/* element_discretization */
		GT_element_group_get_element_discretization(
			gelem_editor->edit_gt_element_group,&element_discretization);
		sprintf(temp_string,"%d*%d*%d",
			element_discretization.number_in_xi1,
			element_discretization.number_in_xi2,
			element_discretization.number_in_xi3);
		XtVaSetValues(gelem_editor->element_disc_text,XmNvalue,temp_string,NULL);
		/* native_discretization_field */
		if (field_set=((struct FE_field *)NULL != (native_discretization_field=
			GT_element_group_get_native_discretization_field(gt_element_group))))
		{
			CHOOSE_OBJECT_SET_OBJECT(FE_field)(
				gelem_editor->native_discretization_field_widget,
				native_discretization_field);
		}
		XtVaSetValues(gelem_editor->native_discretization_button,
			XmNset,(XtPointer)field_set,NULL);
		XtSetSensitive(gelem_editor->native_discretization_field_widget,field_set);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gelem_editor_set_general_settings.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gelem_editor_set_general_settings */

static void graphical_element_editor_general_button_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 2 June 1998

DESCRIPTION :
Toggle for switching the general settings forms on and off.
==============================================================================*/
{
	struct Graphical_element_editor_struct *gelem_editor;

	ENTER(graphical_element_editor_general_button_CB);
	USE_PARAMETER(call_data);
	if (widget&&
		(gelem_editor=(struct Graphical_element_editor_struct *)client_data))
	{
		if (XmToggleButtonGetState(widget))
		{
			XtManageChild(gelem_editor->general_rowcol);
		}
		else
		{
			XtUnmanageChild(gelem_editor->general_rowcol);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_general_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_general_button_CB */

static void graphical_element_editor_update_default_coordinate_field(
	Widget widget,void *gelem_editor_void,void *default_coordinate_field_void)
/*******************************************************************************
LAST MODIFIED : 19 August 1999

DESCRIPTION :
Callback for change of default coordinate field.
==============================================================================*/
{
	struct Graphical_element_editor_struct *gelem_editor;

	ENTER(graphical_element_editor_update_default_coordinate_field);
	USE_PARAMETER(widget);
	if (default_coordinate_field_void&&
		(gelem_editor=(struct Graphical_element_editor_struct *)gelem_editor_void))
	{
		GT_element_group_set_default_coordinate_field(
			gelem_editor->edit_gt_element_group,
			(struct Computed_field *)default_coordinate_field_void);
		/* inform the client of the change */
		graphical_element_editor_update(gelem_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_update_default_coordinate_field.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_update_default_coordinate_field */

static void graphical_element_editor_element_disc_text_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 August 1999

DESCRIPTION :
Called when entry is made into the element discretization text field.
==============================================================================*/
{
	char *disc_text;
	struct Graphical_element_editor_struct *gelem_editor;
	struct Parse_state *temp_state;
	struct Element_discretization element_discretization;

	ENTER(graphical_element_editor_element_disc_text_CB);
	USE_PARAMETER(call_data);
	if (widget&&
		(gelem_editor=(struct Graphical_element_editor_struct *)client_data))
	{
		/* Get the pointer to the text */
		XtVaGetValues(widget,XmNvalue,&disc_text,NULL);
		if (disc_text)
		{
			if (temp_state=create_Parse_state(disc_text))
			{
				if (GT_element_group_get_element_discretization(
					gelem_editor->edit_gt_element_group,&element_discretization)&&
					set_Element_discretization(temp_state,(void *)&element_discretization,
						(void *)gelem_editor->user_interface)&&
					GT_element_group_set_element_discretization(
						gelem_editor->edit_gt_element_group,
						&element_discretization,gelem_editor->user_interface))
				{
					/*gelem_editor_make_settings_list(gelem_editor);*/
					/* inform the client of the changes */
					graphical_element_editor_update(gelem_editor);
				}
				destroy_Parse_state(&temp_state);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"graphical_element_editor_element_disc_text_CB.  "
					"Could not create parse state");
			}
			XtFree(disc_text);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"graphical_element_editor_element_disc_text_CB.  Missing text");
		}
		/* always redisplay discretization to show assumed values */
		gelem_editor_set_general_settings(gelem_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_element_disc_text_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_element_disc_text_CB */

static void graphical_element_editor_circle_disc_text_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 3 March 1999

DESCRIPTION :
Called when entry is made into the circle discretization text field.
==============================================================================*/
{
	char *disc_text;
	struct Graphical_element_editor_struct *gelem_editor;

	ENTER(graphical_element_editor_circle_disc_text_CB);
	USE_PARAMETER(call_data);
	/* check arguments */
	if (widget&&
		(gelem_editor=(struct Graphical_element_editor_struct *)client_data))
	{
		/* Get the pointer to the text */
		XtVaGetValues(widget,XmNvalue,&disc_text,NULL);
		if (disc_text)
		{
			if (GT_element_group_set_circle_discretization(
				gelem_editor->edit_gt_element_group,atoi(disc_text),
				gelem_editor->user_interface))
			{
				/* gelem_editor_make_settings_list(gelem_editor); */
				/* inform the client of the changes */
				graphical_element_editor_update(gelem_editor);
			}
			XtFree(disc_text);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"graphical_element_editor_circle_disc_text_CB.  Missing text");
		}
		/* always redisplay discretization to show assumed values */
		gelem_editor_set_general_settings(gelem_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_circle_disc_text_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_circle_disc_text_CB */

static void graphical_element_editor_native_discretization_button_CB(
	Widget widget,void *gelem_editor_void,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 19 August 1999

DESCRIPTION :
Called when the native_discretization toggle button value changes.
==============================================================================*/
{
	struct FE_field *native_discretization_field;
	struct Graphical_element_editor_struct *gelem_editor;
	struct GT_element_group *gt_element_group;

	ENTER(graphical_element_editor_native_discretization_button_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(reason);
	if ((gelem_editor=(struct Graphical_element_editor_struct *)gelem_editor_void)
		&&(gt_element_group=gelem_editor->edit_gt_element_group))
	{
		if (GT_element_group_get_native_discretization_field(gt_element_group))
		{
			native_discretization_field=(struct FE_field *)NULL;
		}
		else
		{
			/* get native_discretization field from the widget */
			native_discretization_field=CHOOSE_OBJECT_GET_OBJECT(FE_field)(
				gelem_editor->native_discretization_field_widget);
		}
		GT_element_group_set_native_discretization_field(gt_element_group,
			native_discretization_field);
		native_discretization_field=
			GT_element_group_get_native_discretization_field(gt_element_group);
		/* (un)gray native_discretization field widget */
		XtSetSensitive(gelem_editor->native_discretization_field_widget,
			NULL != native_discretization_field);
		/* inform the client of the change */
		graphical_element_editor_update(gelem_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_native_discretization_button_CB.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_native_discretization_button_CB */

static void graphical_element_editor_update_native_discretization_field(
	Widget widget,void *gelem_editor_void,void *native_discretization_field_void)
/*******************************************************************************
LAST MODIFIED : 19 August 1999

DESCRIPTION :
Callback for change of default native_discretization field.
==============================================================================*/
{
	struct Graphical_element_editor_struct *gelem_editor;

	ENTER(graphical_element_editor_update_native_discretization_field);
	USE_PARAMETER(widget);
	if (gelem_editor=(struct Graphical_element_editor_struct *)gelem_editor_void)
	{
		/* only select it if we currently have a native_discretization field */
		if (GT_element_group_get_native_discretization_field(
			gelem_editor->edit_gt_element_group))
		{
			GT_element_group_set_native_discretization_field(
				gelem_editor->edit_gt_element_group,
				(struct FE_field *)native_discretization_field_void);
			/* inform the client of the change */
			graphical_element_editor_update(gelem_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_update_native_discretization_field.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_update_native_discretization_field */

static void graphical_element_editor_dimension_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 March 1999

DESCRIPTION :
Called when switching between ALL/0-D/1-D/2-D/3-D.
==============================================================================*/
{
	char **valid_strings;
	int dimension,number_of_valid_strings;
	struct Graphical_element_editor_struct *gelem_editor;
	Widget button_widget;

	ENTER(graphical_element_editor_dimension_CB);
	if (widget&&
		(gelem_editor=(struct Graphical_element_editor_struct *)client_data)&&
		(button_widget=((XmRowColumnCallbackStruct *)call_data)->widget))
	{
		if (XmToggleButtonGetState(button_widget))
		{
			dimension=gelem_editor->current_dimension;
			if (button_widget==gelem_editor->dimension_all_button)
			{
				dimension=-1;
			}
			else if (button_widget==gelem_editor->dimension_0_button)
			{
				dimension=0;
			}
			else if (button_widget==gelem_editor->dimension_1_button)
			{
				dimension=1;
			}
			else if (button_widget==gelem_editor->dimension_2_button)
			{
				dimension=2;
			}
			else if (button_widget==gelem_editor->dimension_3_button)
			{
				dimension=3;
			}
			/* check if not already looking at new settings type */
			if (dimension != gelem_editor->current_dimension)
			{
				gelem_editor->current_dimension=dimension;
				/* work out current_settings_type for changed dimension */
				if ((0<=dimension)&&!GT_element_settings_type_uses_dimension(
					gelem_editor->current_settings_type,dimension))
				{
					if (gelem_editor->current_settings=
						first_settings_in_GT_element_group_that(
							gelem_editor->edit_gt_element_group,
							GT_element_settings_uses_dimension,(void *)&dimension))
					{
						gelem_editor->current_settings_type=
							GT_element_settings_get_settings_type(
								gelem_editor->current_settings);
					}
					else
					{
						gelem_editor->current_settings_type=
							GT_ELEMENT_SETTINGS_TYPE_INVALID;
					}
				}
				/* remake the settings_type choose_enumerator */
				if (valid_strings=GT_element_settings_type_get_valid_strings(
					&number_of_valid_strings,dimension))
				{
					if (GT_ELEMENT_SETTINGS_TYPE_INVALID==
						gelem_editor->current_settings_type)
					{
						gelem_editor->current_settings_type=
							GT_element_settings_type_from_string(valid_strings[0]);
					}
					choose_enumerator_set_valid_strings(
						gelem_editor->settings_type_widget,valid_strings,
						number_of_valid_strings,GT_element_settings_type_string(
							gelem_editor->current_settings_type));
					DEALLOCATE(valid_strings);
				}
				gelem_editor_make_settings_list(gelem_editor);
				gelem_editor_select_settings_item(gelem_editor);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_dimension_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_dimension_CB */

static void graphical_element_editor_update_settings_type(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Called when switching between Points/Lines/Surfaces/Iso-surfaces etc.
==============================================================================*/
{
	struct Graphical_element_editor_struct *gelem_editor;
	enum GT_element_settings_type settings_type;

	ENTER(graphical_element_editor_update_settings_type);
	USE_PARAMETER(call_data);
	if (widget&&
		(gelem_editor=(struct Graphical_element_editor_struct *)client_data))
	{
		settings_type=GT_element_settings_type_from_string(
			choose_enumerator_get_string(gelem_editor->settings_type_widget));
		/* check if not already looking at new settings type */
		if (settings_type != gelem_editor->current_settings_type)
		{
			gelem_editor->current_settings_type=settings_type;
			gelem_editor->current_settings=(struct GT_element_settings *)NULL;
			gelem_editor_select_settings_item(gelem_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_update_settings_type.  Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_update_settings_type */

static void graphical_element_editor_settings_visibility_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 August 1999

DESCRIPTION :
Called when a settings visibility toggle button is selected.
==============================================================================*/
{
	struct Graphical_element_editor_struct *gelem_editor;
	struct GT_element_settings *settings;
	Widget settings_form;

	ENTER(graphical_element_editor_settings_visibility_CB);
	USE_PARAMETER(call_data);
	if (widget&&
		(gelem_editor=(struct Graphical_element_editor_struct *)client_data))
	{
		/* the settings is kept as XmNuserData with the parent settings form */
		if (settings_form=XtParent(widget))
		{
			/* Get the material this menu visibility represents and make it current */
			XtVaGetValues(settings_form,XmNuserData,&settings,NULL);
			if (settings)
			{
				GT_element_settings_set_visibility(settings,
					XmToggleButtonGetState(widget));
				gelem_editor_update_settings_item(gelem_editor,settings);
				/* inform the client of the changes */
				graphical_element_editor_update(gelem_editor);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"graphical_element_editor_settings_visibility_CB.  Missing settings");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"graphical_element_editor_settings_visibility_CB.  "
				"Missing parent settings form");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_settings_visibility_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_settings_visibility_CB */

static void graphical_element_editor_settings_select_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 August 1999

DESCRIPTION :
Called when a settings select toggle button is selected.
==============================================================================*/
{
	struct Graphical_element_editor_struct *gelem_editor;
	struct GT_element_settings *settings;
	Widget settings_form;

	ENTER(graphical_element_editor_settings_select_CB);
	USE_PARAMETER(call_data);
	if (widget&&
		(gelem_editor=(struct Graphical_element_editor_struct *)client_data))
	{
		/* the settings is kept as XmNuserData with the parent settings form */
		if (settings_form=XtParent(widget))
		{
			/* Get the material this menu select represents and make it current */
			XtVaGetValues(settings_form,XmNuserData,&settings,NULL);
			if (settings != gelem_editor->current_settings)
			{
				gelem_editor->current_settings=settings;
			}
			gelem_editor_select_settings_item(gelem_editor);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"graphical_element_editor_settings_select_CB.  "
				"Missing parent settings form");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_settings_select_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_settings_select_CB */

static void graphical_element_editor_modify_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1999

DESCRIPTION :
Called when a modify button - add, delete, up, down - is activated.
==============================================================================*/
{
	enum Streamline_type streamline_type;
	float streamline_length,streamline_width;
	int list_changed,position,return_code,reverse_track;
	struct Computed_field *iso_scalar_field,*orientation_scale_field,
		*stream_vector_field;
	struct Graphical_element_editor_struct *gelem_editor;
	struct GT_object *glyph,*old_glyph;
	struct GT_element_settings *settings;
	struct VT_volume_texture *volume_texture;
	Widget modify_button;
	Triple glyph_centre,glyph_scale_factors,glyph_size;

	ENTER(graphical_element_editor_modify_CB);
	if (widget&&(modify_button=((XmRowColumnCallbackStruct *)call_data)->widget)&&
		(gelem_editor=(struct Graphical_element_editor_struct *)client_data)&&
		gelem_editor->edit_gt_element_group)
	{
		list_changed=0;
		if (modify_button==gelem_editor->add_button)
		{
			if (settings=CREATE(GT_element_settings)(
				gelem_editor->current_settings_type))
			{
				return_code=1;
				if (gelem_editor->current_settings)
				{
					/* copy current settings into new settings */
					return_code=COPY(GT_element_settings)(settings,
						gelem_editor->current_settings);
					/* make sure new settings is visible */
					GT_element_settings_set_visibility(settings,1);
				}
				else
				{
					/* set material for all settings */
					GT_element_settings_set_material(settings,
						gelem_editor->default_material);
					/* set iso_scalar_field for iso_surfaces */
					if (GT_ELEMENT_SETTINGS_ISO_SURFACES==
						gelem_editor->current_settings_type)
					{
						if (iso_scalar_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
							Computed_field_has_1_component,(void *)NULL,
							Computed_field_package_get_computed_field_manager(
								gelem_editor->computed_field_package)))
						{
							if (!GT_element_settings_set_iso_surface_parameters(
								settings,iso_scalar_field,0.0))
							{
								return_code=0;
							}
						}
						else
						{
							display_message(WARNING_MESSAGE,"No scalar fields defined");
							return_code=0;
						}
					}
					/* set initial glyph for settings types that use them */
					if ((GT_ELEMENT_SETTINGS_NODE_POINTS==
						gelem_editor->current_settings_type)||
						(GT_ELEMENT_SETTINGS_DATA_POINTS==
							gelem_editor->current_settings_type)||
						(GT_ELEMENT_SETTINGS_ELEMENT_POINTS==
							gelem_editor->current_settings_type))
					{
						/* default to point glyph for fastest possible display */
						glyph=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)("point",
							gelem_editor->glyph_list);
						if (!(GT_element_settings_get_glyph_parameters(settings,
							&old_glyph,glyph_centre,glyph_size,&orientation_scale_field,
							glyph_scale_factors)&&
							GT_element_settings_set_glyph_parameters(settings,glyph,
								glyph_centre,glyph_size,orientation_scale_field,
								glyph_scale_factors)))
						{
							display_message(WARNING_MESSAGE,"No glyphs defined");
							return_code=0;
						}
					}
					if (GT_ELEMENT_SETTINGS_VOLUMES==gelem_editor->current_settings_type)
					{
						/* must have a volume texture */
						if (volume_texture=FIRST_OBJECT_IN_MANAGER_THAT(VT_volume_texture)(
							(MANAGER_CONDITIONAL_FUNCTION(VT_volume_texture) *)NULL,
							(void *)NULL,gelem_editor->volume_texture_manager))
						{
							if (!GT_element_settings_set_volume_texture(settings,
								volume_texture))
							{
								return_code=0;
							}
						}
						else
						{
							display_message(WARNING_MESSAGE,"No volume textures defined");
							return_code=0;
						}
					}
					/* set stream_vector_field for STREAMLINES */
					if (GT_ELEMENT_SETTINGS_STREAMLINES==
						gelem_editor->current_settings_type)
					{
						GT_element_settings_get_streamline_parameters(
							settings,&streamline_type,&stream_vector_field,&reverse_track,
							&streamline_length,&streamline_width);
						if (stream_vector_field=
							FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
							Computed_field_is_stream_vector_capable,(void *)NULL,
							Computed_field_package_get_computed_field_manager(
								gelem_editor->computed_field_package)))
						{
							if (!GT_element_settings_set_streamline_parameters(
								settings,streamline_type,stream_vector_field,reverse_track,
								streamline_length,streamline_width))
							{
								return_code=0;
							}
						}
						else
						{
							display_message(WARNING_MESSAGE,"No vector fields defined");
							return_code=0;
						}
					}
				}
				/* make sure new settings uses current dimension if not ALL */
				if (return_code&&(GT_ELEMENT_SETTINGS_ELEMENT_POINTS==
					gelem_editor->current_settings_type))
				{
					if (1==gelem_editor->current_dimension)
					{
						GT_element_settings_set_use_element_type(settings,USE_LINES);
					}
					else if (2==gelem_editor->current_dimension)
					{
						GT_element_settings_set_use_element_type(settings,USE_FACES);
					}
					else if (3==gelem_editor->current_dimension)
					{
						GT_element_settings_set_use_element_type(settings,USE_ELEMENTS);
					}
				}
				if (return_code&&GT_element_group_add_settings(
					gelem_editor->edit_gt_element_group,settings,0))
				{
					list_changed=1;
					gelem_editor->current_settings=settings;
				}
				else
				{
					DESTROY(GT_element_settings)(&settings);
				}
			}
		}
		else if (modify_button==gelem_editor->delete_button)
		{
			list_changed=GT_element_group_remove_settings(
				gelem_editor->edit_gt_element_group,gelem_editor->current_settings);
			gelem_editor->current_settings=(struct GT_element_settings *)NULL;
		}
		else if (modify_button==gelem_editor->up_button)
		{
			/* increase position of current_settings by 1 */
			if (1<(position=GT_element_group_get_settings_position(
				gelem_editor->edit_gt_element_group,gelem_editor->current_settings)))
			{
				list_changed=1;
				settings=gelem_editor->current_settings;
				ACCESS(GT_element_settings)(settings);
				GT_element_group_remove_settings(gelem_editor->edit_gt_element_group,
					gelem_editor->current_settings);
				GT_element_group_add_settings(gelem_editor->edit_gt_element_group,
					gelem_editor->current_settings,position-1);
				DEACCESS(GT_element_settings)(&settings);
			}
		}
		else if (modify_button==gelem_editor->down_button)
		{
			/* decrease position of current_settings by 1 */
			if (position=GT_element_group_get_settings_position(
				gelem_editor->edit_gt_element_group,gelem_editor->current_settings))
			{
				list_changed=1;
				settings=gelem_editor->current_settings;
				ACCESS(GT_element_settings)(settings);
				GT_element_group_remove_settings(gelem_editor->edit_gt_element_group,
					gelem_editor->current_settings);
				GT_element_group_add_settings(gelem_editor->edit_gt_element_group,
					gelem_editor->current_settings,position+1);
				DEACCESS(GT_element_settings)(&settings);
			}
		}
		if (list_changed)
		{
			gelem_editor_make_settings_list(gelem_editor);
			gelem_editor_select_settings_item(gelem_editor);
			/* inform the client of the changes */
			graphical_element_editor_update(gelem_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_modify_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_modify_CB */

static void graphical_element_editor_update_settings(
	Widget surface_settings_editor_widget,
	void *gelem_editor_void,void *settings_void)
/*******************************************************************************
LAST MODIFIED : 19 August 1999

DESCRIPTION :
Callback for when changes are made in the settings editor.
==============================================================================*/
{
	int visibility;
	struct Graphical_element_editor_struct *gelem_editor;
	struct GT_element_settings *settings;

	ENTER(graphical_element_editor_update_settings);
	USE_PARAMETER(surface_settings_editor_widget);
	if ((gelem_editor=
			(struct Graphical_element_editor_struct *)gelem_editor_void)
		&&(settings=(struct GT_element_settings *)settings_void))
	{
#if defined (OLD_CODE)
		/* only modify if the settings have changed */
		if (GT_element_settings_is_changed(settings))
		{
#endif /* defined (OLD_CODE) */
		/* only modify if the settings have changed */
			/* keep visibility of current_settings */
			visibility=
				GT_element_settings_get_visibility(gelem_editor->current_settings);
			GT_element_group_modify_settings(gelem_editor->edit_gt_element_group,
				gelem_editor->current_settings,settings);
			GT_element_settings_set_visibility(gelem_editor->current_settings,
				visibility);
			gelem_editor_update_settings_item(gelem_editor,
				gelem_editor->current_settings);
			/* inform the client of the changes */
			graphical_element_editor_update(gelem_editor);
#if defined (OLD_CODE)
		}
#endif /* defined (OLD_CODE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_update_settings.  Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_update_settings */

static void graphical_element_editor_material_manager_message(
	struct MANAGER_MESSAGE(Graphical_material) *message,void *data)
/*******************************************************************************
LAST MODIFIED : 16 February 1997

DESCRIPTION :
Something has changed globally in the material manager. If the event has
changed the name of a material, must remake the menu.
==============================================================================*/
{
	struct Graphical_element_editor_struct *gelem_editor;

	ENTER(graphical_element_editor_material_manager_message);
	/* checking arguments */
	if (message&&(gelem_editor=(struct Graphical_element_editor_struct *)data))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_ALL(Graphical_material):
			case MANAGER_CHANGE_IDENTIFIER(Graphical_material):
			case MANAGER_CHANGE_OBJECT(Graphical_material):
			{
				gelem_editor_make_settings_list(gelem_editor);
			} break;
			case MANAGER_CHANGE_DELETE(Graphical_material):
			case MANAGER_CHANGE_ADD(Graphical_material):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Graphical_material):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_material_manager_message.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_material_manager_message */

static void graphical_element_editor_spectrum_manager_message(
	struct MANAGER_MESSAGE(Spectrum) *message,void *data)
/*******************************************************************************
LAST MODIFIED : 16 February 1997

DESCRIPTION :
Something has changed globally in the spectrum manager. If the event has
changed the name of a spectrum, must remake the menu.
==============================================================================*/
{
	struct Graphical_element_editor_struct *gelem_editor;

	ENTER(graphical_element_editor_spectrum_manager_message);
	/* checking arguments */
	if (message&&(gelem_editor=(struct Graphical_element_editor_struct *)data))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_ALL(Spectrum):
			case MANAGER_CHANGE_IDENTIFIER(Spectrum):
			case MANAGER_CHANGE_OBJECT(Spectrum):
			{
				gelem_editor_make_settings_list(gelem_editor);
			} break;
			case MANAGER_CHANGE_DELETE(Spectrum):
			case MANAGER_CHANGE_ADD(Spectrum):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Spectrum):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_spectrum_manager_message.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_spectrum_manager_message */

/*
Global functions
----------------
*/
Widget create_graphical_element_editor_widget(Widget *gelem_editor_widget,
	Widget parent,struct GT_element_group *gt_element_group,
	struct Computed_field_package *computed_field_package,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_material,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *default_spectrum,
	struct MANAGER(VT_volume_texture) *volume_texture_manager,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Creates a graphical_element_editor widget.
==============================================================================*/
{
	char **valid_strings;
	int init_widgets,number_of_valid_strings;
	MrmType graphical_element_editor_dialog_class;
	struct Callback_data callback;
	struct Graphical_element_editor_struct *gelem_editor=NULL;
	struct MANAGER(Computed_field) *computed_field_manager;
	static MrmRegisterArg callback_list[]=
	{
		{"gelem_ed_destroy_CB",(XtPointer)graphical_element_editor_destroy_CB},
		{"gelem_ed_id_general_btn",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor,general_button)},
		{"gelem_ed_id_general_rc",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor,general_rowcol)},
		{"gelem_ed_id_def_coord_form",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor,default_coordinate_field_form)},
		{"gelem_ed_id_element_disc_text",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor,element_disc_text)},
		{"gelem_ed_id_circle_disc_text",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor,circle_disc_text)},
		{"gelem_ed_id_native_disc_btn",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor,native_discretization_button)},
		{"gelem_ed_id_native_disc_form",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor,
				native_discretization_field_form)},
		{"gelem_ed_id_dimension_0_btn",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor,dimension_0_button)},
		{"gelem_ed_id_dimension_1_btn",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor,dimension_1_button)},
		{"gelem_ed_id_dimension_2_btn",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor,dimension_2_button)},
		{"gelem_ed_id_dimension_3_btn",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor,dimension_3_button)},
		{"gelem_ed_id_dimension_all_btn",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor,dimension_all_button)},
		{"gelem_ed_id_settings_type_form",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor,settings_type_form)},
		{"gelem_ed_id_add_btn",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor,add_button)},
		{"gelem_ed_id_delete_btn",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor,delete_button)},
		{"gelem_ed_id_up_btn",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor,up_button)},
		{"gelem_ed_id_down_btn",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor,down_button)},
		{"gelem_ed_id_settings_scr",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor,settings_scroll)},
		{"gelem_ed_id_settings_rc",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor,settings_rowcol)},
		{"gelem_ed_id_settings_form",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor,settings_form)},
		{"gelem_ed_general_btn_CB",(XtPointer)
			graphical_element_editor_general_button_CB},
		{"gelem_ed_element_disc_text_CB",(XtPointer)
			graphical_element_editor_element_disc_text_CB},
		{"gelem_ed_circle_disc_text_CB",(XtPointer)
			graphical_element_editor_circle_disc_text_CB},
		{"gelem_ed_native_disc_btn_CB",(XtPointer)
			graphical_element_editor_native_discretization_button_CB},
		{"gelem_ed_dimension_CB",(XtPointer)
			graphical_element_editor_dimension_CB},
		{"gelem_ed_settings_visibility_CB",(XtPointer)
			graphical_element_editor_settings_visibility_CB},
		{"gelem_ed_settings_select_CB",(XtPointer)
			graphical_element_editor_settings_select_CB},
		{"gelem_ed_modify_CB",(XtPointer)
			graphical_element_editor_modify_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"gelem_ed_structure",(XtPointer)NULL}
	};
	Widget return_widget;

	ENTER(create_graphical_element_editor_widget);
	return_widget=(Widget)NULL;
	if (gelem_editor_widget&&parent&&computed_field_package&&
		(computed_field_manager=Computed_field_package_get_computed_field_manager(
			computed_field_package))&&element_manager&&fe_field_manager&&
		graphical_material_manager&&default_material&&glyph_list&&
		spectrum_manager&&default_spectrum&&volume_texture_manager&&user_interface)
	{
		if (MrmOpenHierarchy_base64_string(graphical_element_editor_uidh,
			&gelem_editor_hierarchy,&gelem_editor_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(gelem_editor,struct Graphical_element_editor_struct,1))
			{
				/* initialise the structure */
				gelem_editor->edit_gt_element_group=(struct GT_element_group *)NULL;
				gelem_editor->computed_field_package=computed_field_package;
				gelem_editor->element_manager=element_manager;
				gelem_editor->fe_field_manager=fe_field_manager;
				gelem_editor->glyph_list=glyph_list;
				gelem_editor->graphical_material_manager=graphical_material_manager;
				gelem_editor->default_material=default_material;
				gelem_editor->material_manager_callback_id=(void *)NULL;
				gelem_editor->default_spectrum=default_spectrum;
				gelem_editor->spectrum_manager=spectrum_manager;
				gelem_editor->spectrum_manager_callback_id=(void *)NULL;
				gelem_editor->volume_texture_manager=volume_texture_manager;
				gelem_editor->volume_texture_manager_callback_id=(void *)NULL;
				gelem_editor->user_interface=user_interface;
				gelem_editor->widget_parent=parent;
				gelem_editor->widget_address=gelem_editor_widget;
				gelem_editor->current_dimension=-1;
				gelem_editor->current_settings_type=GT_ELEMENT_SETTINGS_TYPE_INVALID;
				gelem_editor->current_settings=(struct GT_element_settings *)NULL;
				/* initialize widgets */
				gelem_editor->widget=(Widget)NULL;
				gelem_editor->general_button=(Widget)NULL;
				gelem_editor->general_rowcol=(Widget)NULL;
				gelem_editor->default_coordinate_field_form=(Widget)NULL;
				gelem_editor->default_coordinate_field_widget=(Widget)NULL;
				gelem_editor->element_disc_text=(Widget)NULL;
				gelem_editor->circle_disc_text=(Widget)NULL;
				gelem_editor->native_discretization_button=(Widget)NULL;
				gelem_editor->native_discretization_field_form=(Widget)NULL;
				gelem_editor->native_discretization_field_widget=(Widget)NULL;
				gelem_editor->dimension_0_button=(Widget)NULL;
				gelem_editor->dimension_1_button=(Widget)NULL;
				gelem_editor->dimension_2_button=(Widget)NULL;
				gelem_editor->dimension_3_button=(Widget)NULL;
				gelem_editor->dimension_all_button=(Widget)NULL;
				gelem_editor->settings_type_form=(Widget)NULL;
				gelem_editor->settings_type_widget=(Widget)NULL;
				gelem_editor->add_button=(Widget)NULL;
				gelem_editor->delete_button=(Widget)NULL;
				gelem_editor->up_button=(Widget)NULL;
				gelem_editor->down_button=(Widget)NULL;
				gelem_editor->settings_scroll=(Widget)NULL;
				gelem_editor->settings_rowcol=(Widget)NULL;
				gelem_editor->settings_form=(Widget)NULL;
				gelem_editor->settings_widget=(Widget)NULL;
				gelem_editor->update_callback.procedure=(Callback_procedure *)NULL;
				gelem_editor->update_callback.data=(void *)NULL;
				/* register the callbacks */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
					gelem_editor_hierarchy,
					callback_list,XtNumber(callback_list)))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)gelem_editor;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						gelem_editor_hierarchy,
						identifier_list,XtNumber(identifier_list)))
					{
						/* fetch graphical element editor widget */
						if (MrmSUCCESS==MrmFetchWidget(gelem_editor_hierarchy,
							"gelem_ed_widget",gelem_editor->widget_parent,
							&(gelem_editor->widget),
							&graphical_element_editor_dialog_class))
						{
							XtUnmanageChild(gelem_editor->general_rowcol);
							init_widgets=1;
							/* create chooser for default_coordinate_field */
							if (!(gelem_editor->default_coordinate_field_widget=
								CREATE_CHOOSE_OBJECT_WIDGET(Computed_field)(
								gelem_editor->default_coordinate_field_form,
								(struct Computed_field *)NULL,computed_field_manager,
								Computed_field_has_1_to_3_components)))
							{
								init_widgets=0;
							}
							/* create chooser for native_discretization_field */
							if (!(gelem_editor->native_discretization_field_widget=
								CREATE_CHOOSE_OBJECT_WIDGET(FE_field)(
								gelem_editor->native_discretization_field_form,
								(struct FE_field *)NULL,fe_field_manager,
								(MANAGER_CONDITIONAL_FUNCTION(FE_field) *)NULL)))
							{
								init_widgets=0;
							}
							/* create chooser for settings_type enumeration */
							valid_strings=GT_element_settings_type_get_valid_strings(
								&number_of_valid_strings,gelem_editor->current_dimension);
							if (!(gelem_editor->settings_type_widget=
								create_choose_enumerator_widget(
								gelem_editor->settings_type_form,valid_strings,
								number_of_valid_strings,
								GT_element_settings_type_string(GT_ELEMENT_SETTINGS_LINES))))
							{
								init_widgets=0;
							}
							DEALLOCATE(valid_strings);
							/* create settings editor with NULL settings */
							if (!(create_settings_editor_widget(
								&(gelem_editor->settings_widget),
								gelem_editor->settings_form,(struct GT_element_settings *)NULL,
								gelem_editor->computed_field_package,
								gelem_editor->element_manager,
								gelem_editor->fe_field_manager,
								gelem_editor->graphical_material_manager,
								gelem_editor->glyph_list,gelem_editor->spectrum_manager,
								gelem_editor->volume_texture_manager,
								gelem_editor->user_interface)))
							{
								init_widgets=0;
							}
							if (init_widgets)
							{
								/* turn on callbacks for choosers */
								callback.data=(void *)gelem_editor;
								callback.procedure=
									graphical_element_editor_update_default_coordinate_field;
								CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
									gelem_editor->default_coordinate_field_widget,&callback);
								callback.procedure=
									graphical_element_editor_update_native_discretization_field;
								CHOOSE_OBJECT_SET_CALLBACK(FE_field)(
									gelem_editor->native_discretization_field_widget,&callback);
								callback.procedure=
									graphical_element_editor_update_settings_type;
								choose_enumerator_set_callback(
									gelem_editor->settings_type_widget,&callback);
								/* select all dimensions to be displayed */
								XtVaSetValues(gelem_editor->dimension_all_button,
									XmNset,True,NULL);
								if (gt_element_group)
								{
									graphical_element_editor_set_gt_element_group(
										gelem_editor->widget,gt_element_group);
								}
								return_widget=gelem_editor->widget;
							}
							else
							{
								XtDestroyWidget(gelem_editor->widget);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_graphical_element_editor_widget.  "
								"Could not fetch graphical_element_editor widget");
							DEALLOCATE(gelem_editor);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_graphical_element_editor_widget.  "
							"Could not register identifiers");
						DEALLOCATE(gelem_editor);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_graphical_element_editor_widget.  "
						"Could not register callbacks");
					DEALLOCATE(gelem_editor);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_graphical_element_editor_widget.  "
					"Could not allocate graphical_element_editor widget structure");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_graphical_element_editor_widget.  Could not open hierarchy");
		}
		*gelem_editor_widget=return_widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_graphical_element_editor_widget.  Invalid argument(s)");
	}
	LEAVE;

	return (return_widget);
} /* create_graphical_element_editor_widget */

struct Callback_data *graphical_element_editor_get_callback(
	Widget graphical_element_editor_widget)
/*******************************************************************************
LAST MODIFIED : 22 July 1997

DESCRIPTION :
Returns a pointer to the update_callback item of the
graphical_element_editor_widget.
==============================================================================*/
{
	struct Callback_data *return_address;
	struct Graphical_element_editor_struct *gelem_editor;

	ENTER(graphical_element_editor_get_callback);
	/* check arguments */
	if (graphical_element_editor_widget)
	{
		/* Get the pointer to the data for the dialog */
		XtVaGetValues(graphical_element_editor_widget,XmNuserData,
			&gelem_editor,NULL);
		if (gelem_editor)
		{
			return_address=&(gelem_editor->update_callback);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"graphical_element_editor_get_callback.  Missing widget data");
			return_address=(struct Callback_data *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_get_callback.  Missing widget");
		return_address=(struct Callback_data *)NULL;
	}
	LEAVE;

	return (return_address);
} /* graphical_element_editor_get_callback */

int graphical_element_editor_set_callback(
	Widget graphical_element_editor_widget,struct Callback_data *new_callback)
/*******************************************************************************
LAST MODIFIED : 22 July 1997

DESCRIPTION :
Changes the callback function for the graphical_element_editor_widget, which
will be called when the gt_element_group changes in any way.
==============================================================================*/
{
	int return_code;
	struct Graphical_element_editor_struct *gelem_editor;

	ENTER(graphical_element_editor_set_callback);
	/* check arguments */
	if (graphical_element_editor_widget&&new_callback)
	{
		/* Get the pointer to the data for the choose_settings dialog */
		XtVaGetValues(graphical_element_editor_widget,XmNuserData,
			&gelem_editor,NULL);
		if (gelem_editor)
		{
			gelem_editor->update_callback.procedure=new_callback->procedure;
			gelem_editor->update_callback.data=new_callback->data;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"graphical_element_editor_set_callback.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_set_callback.  Missing widget");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* graphical_element_editor_set_callback */

struct GT_element_group *graphical_element_editor_get_gt_element_group(
	Widget graphical_element_editor_widget)
/*******************************************************************************
LAST MODIFIED : 22 July 1997

DESCRIPTION :
Returns the gt_element_group currently being edited.
==============================================================================*/
{
	struct GT_element_group *return_address;
	struct Graphical_element_editor_struct *gelem_editor;

	ENTER(graphical_element_editor_get_gt_element_group);
	/* check arguments */
	if (graphical_element_editor_widget)
	{
		/* Get the pointer to the data for the dialog */
		XtVaGetValues(graphical_element_editor_widget,XmNuserData,
			&gelem_editor,NULL);
		if (gelem_editor)
		{
			return_address=gelem_editor->edit_gt_element_group;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"graphical_element_editor_get_gt_element_group.  Missing widget data");
			return_address=(struct GT_element_group *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_get_gt_element_group.  Missing widget");
		return_address=(struct GT_element_group *)NULL;
	}
	LEAVE;

	return (return_address);
} /* graphical_element_editor_get_gt_element_groups */

int graphical_element_editor_set_gt_element_group(
	Widget graphical_element_editor_widget,
	struct GT_element_group *gt_element_group)
/*******************************************************************************
LAST MODIFIED : 4 March 1999

DESCRIPTION :
Sets the gt_element_group to be edited by the graphical_element_editor widget.
==============================================================================*/
{
	int return_code;
	struct Callback_data callback;
	struct Graphical_element_editor_struct *gelem_editor;

	ENTER(graphical_element_editor_set_gt_element_group);
	/* check arguments */
	if (graphical_element_editor_widget)
	{
		/* Get the pointer to the data for the graphical_element_editor_widget */
		XtVaGetValues(graphical_element_editor_widget,XmNuserData,&gelem_editor,
			NULL);
		if (gelem_editor)
		{
			if (gt_element_group)
			{
				if (make_edit_gt_element_group(gelem_editor,gt_element_group))
				{
					gelem_editor_set_general_settings(gelem_editor);
					/* continue with the current_settings_type */
					gelem_editor_make_settings_list(gelem_editor);
					XtManageChild(gelem_editor->widget);
					/* select the first settings item in the list (if any) */
					gelem_editor->current_settings=(struct GT_element_settings *)NULL;
					gelem_editor_select_settings_item(gelem_editor);
					/* turn on callbacks from settings editor */
					callback.procedure=graphical_element_editor_update_settings;
					callback.data=(void *)gelem_editor;
					settings_editor_set_callback(gelem_editor->settings_widget,&callback);
					/* register for any material changes */
					if (!gelem_editor->material_manager_callback_id)
					{
						gelem_editor->material_manager_callback_id=
							MANAGER_REGISTER(Graphical_material)(
							graphical_element_editor_material_manager_message,
							(void *)gelem_editor,gelem_editor->graphical_material_manager);
					}
					/* register for any material changes */
					if (!gelem_editor->spectrum_manager_callback_id)
					{
						gelem_editor->spectrum_manager_callback_id=
							MANAGER_REGISTER(Spectrum)(
							graphical_element_editor_spectrum_manager_message,
							(void *)gelem_editor,gelem_editor->spectrum_manager);
					}
				}
				else
				{
					gt_element_group=(struct GT_element_group *)NULL;
				}
			}
			if (!gt_element_group)
			{
				/* turn off settings editor by passing NULL settings */
				gelem_editor->current_settings=(struct GT_element_settings *)NULL;
				settings_editor_set_settings(gelem_editor->settings_widget,
					gelem_editor->current_settings);
				XtUnmanageChild(gelem_editor->widget);
				/* deregister material manager callbacks */
				if (gelem_editor->material_manager_callback_id)
				{
					MANAGER_DEREGISTER(Graphical_material)(
						gelem_editor->material_manager_callback_id,
						gelem_editor->graphical_material_manager);
					gelem_editor->material_manager_callback_id=(void *)NULL;
				}
				/* deregister spectrum manager callbacks */
				if (gelem_editor->spectrum_manager_callback_id)
				{
					MANAGER_DEREGISTER(Spectrum)(
						gelem_editor->spectrum_manager_callback_id,
						gelem_editor->spectrum_manager);
					gelem_editor->spectrum_manager_callback_id=(void *)NULL;
				}
				/* turn off callbacks from settings editors */
				callback.procedure=(Callback_procedure *)NULL;
				callback.data=(void *)NULL;
				settings_editor_set_callback(gelem_editor->settings_widget,&callback);
				if (gelem_editor->edit_gt_element_group)
				{
					DESTROY(GT_element_group)(&(gelem_editor->edit_gt_element_group));
				}
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"graphical_element_editor_set_gt_element_group.  "
				"Missing graphical_element_editor struct");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_set_gt_element_group.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* graphical_element_editor_set_gt_element_group */
