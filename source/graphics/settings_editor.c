/*******************************************************************************
FILE : settings_editor.c

LAST MODIFIED : 2 April 2003

DESCRIPTION :
Provides the widgets to manipulate element group settings.
==============================================================================*/
#include <stdio.h>
#if defined (MOTIF)
#include <Xm/Xm.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#endif /* defined (MOTIF) */
#include "choose/choose_computed_field.h"
#include "choose/choose_graphical_material.h"
#include "choose/choose_enumerator.h"
#include "choose/choose_fe_field.h"
#include "choose/choose_gt_object.h"
#include "choose/choose_spectrum.h"
#include "choose/choose_volume_texture.h"
#include "choose/text_choose_fe_element.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "finite_element/finite_element.h"
#include "general/debug.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/element_group_settings.h"
#include "graphics/graphics_object.h"
#include "graphics/material.h"
#include "graphics/spectrum.h"
#include "graphics/settings_editor.h"
#include "graphics/settings_editor.uidh"
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int settings_editor_hierarchy_open=0;
static MrmHierarchy settings_editor_hierarchy;
#endif /* defined (MOTIF) */

struct Settings_editor
/*******************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Contains all the information carried by the graphical element editor widget.
==============================================================================*/
{
	struct Cmiss_region *root_region;
	struct GT_element_settings *current_settings;
	struct Computed_field_package *computed_field_package;
	struct MANAGER(Graphical_material) *graphical_material_manager;
	struct LIST(GT_object) *glyph_list;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct MANAGER(VT_volume_texture) *volume_texture_manager;
	struct User_interface *user_interface;
	struct Callback_data update_callback;
	/* main dialog widgets */
	Widget *widget_address,widget,widget_parent;
	/* geometry widgets */
	Widget main_scroll, main_form;
	Widget name_text,coordinate_button,coordinate_field_form,coordinate_field_widget,
		use_element_type_entry,use_element_type_form,use_element_type_widget,
		discretization_entry,discretization_text,
		exterior_face_entry,exterior_button,face_button,face_option,face_menu,
		radius_entry,constant_radius_text,radius_scalar_field_button,
		radius_scalar_field_form,radius_scalar_field_widget,
		radius_scale_factor_entry,radius_scale_factor_text,iso_surface_entry,
		iso_scalar_field_form,iso_scalar_field_widget,iso_value_text,
		label_field_entry,label_field_button,label_field_form,label_field_widget,
		native_discretization_entry, native_discretization_button,
		native_discretization_field_form, native_discretization_field_widget,
		xi_point_density_field_entry, xi_point_density_field_button,
		xi_point_density_field_form, xi_point_density_field_widget,
		xi_discretization_mode_entry,
		xi_discretization_mode_form,xi_discretization_mode_widget,
		glyph_group_entry,glyph_form,glyph_widget,
		glyph_scaling_mode_entry, glyph_scaling_mode_form,
		glyph_scaling_mode_widget,
		glyph_size_text,glyph_centre_text,
		glyph_orientation_scale_button,glyph_orientation_scale_field_form,
		glyph_orientation_scale_field_widget,
		glyph_scale_factors_entry,glyph_scale_factors_text,
		glyph_variable_scale_entry, glyph_variable_scale_button,
		glyph_variable_scale_field_form, glyph_variable_scale_field_widget,
		volume_texture_entry,volume_texture_form,
		volume_texture_widget,seed_element_entry,seed_element_button,
		seed_element_form,seed_element_widget,seed_xi_entry,seed_xi_text,
		streamline_entry,streamline_type_form,streamline_type_widget,
		streamline_length_text,streamline_width_text,stream_vector_field_form,
		stream_vector_field_widget,streamline_reverse_button,
		select_mode_entry,select_mode_form,select_mode_widget;
	/* appearance widgets */
	Widget material_form,material_widget,line_width_entry,line_width_text,
		texture_coord_field_entry,
		texture_coord_field_form,texture_coord_field_widget,
		texture_coord_field_button,data_field_button,data_field_form,
		data_field_widget,render_type_entry,render_type_form,render_type_widget,
		spectrum_entry,spectrum_form,spectrum_widget,
		streamline_data_type_entry,streamline_data_type_form,
		streamline_data_type_widget,selected_material_form,selected_material_widget;
}; /* struct Settings_editor */

/*
Module functions
----------------
*/

static int settings_editor_update(struct Settings_editor *settings_editor)
/*******************************************************************************
LAST MODIFIED : 3 March 1999

DESCRIPTION :
Tells CMGUI about the current values. Returns a pointer to the current point
settings.
==============================================================================*/
{
	int return_code;

	ENTER(settings_editor_update);
	if (settings_editor)
	{
		/* Now send an update to the client if requested */
		if ((settings_editor->update_callback).procedure)
		{
			(settings_editor->update_callback.procedure)(
				settings_editor->widget,settings_editor->update_callback.data,
				settings_editor->current_settings);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* settings_editor_update */

static int settings_editor_set_face_number(
	struct Settings_editor *settings_editor)
/*******************************************************************************
LAST MODIFIED : 8 June 1998

DESCRIPTION :
Sets the current face_number on the option menu and button.
==============================================================================*/
{
	int face,face_selected,num_children,return_code;
	Widget *child_list;

	ENTER(settings_editor_set_face_number);
	if (settings_editor&&settings_editor->face_menu)
	{
		face_selected=GT_element_settings_get_face(
			settings_editor->current_settings,&face);
		XtVaSetValues(settings_editor->face_button,
			XmNset,face_selected,NULL);
		if (!face_selected)
		{
			face=0;
		}
		/* get children of the menu so that one may be selected */
		XtVaGetValues(settings_editor->face_menu,XmNnumChildren,
			&num_children,XmNchildren,&child_list,NULL);
		if ((0 <= face)&&(face < num_children))
		{
			XtVaSetValues(settings_editor->face_option,
				XmNmenuHistory,child_list[face],NULL);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"settings_editor_set_face_number.  Invalid face or option menu");
			return_code=0;
		}
		XtSetSensitive(settings_editor->face_option,face_selected);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_set_face_number.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* settings_editor_set_face_number */

static int settings_editor_display_dimension_specific(
	struct Settings_editor *settings_editor)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Sets and manages the dimension buttons (0-D/1-D/2-D/3-D) and sets and manages
the exterior/face controls for 1-D and 2-D settings.
If there is no current_settings object or these widgets are not relevant to it,
both widget entries are unmanaged.
==============================================================================*/
{
	int dimension,dimension1,dimension2,return_code;
	struct GT_element_settings *settings;

	ENTER(settings_editor_display_dimension_specific);
	if (settings_editor)
	{
		if (settings=settings_editor->current_settings)
		{
			dimension1=1;
			dimension2=2;
			if (GT_element_settings_uses_dimension(settings,&dimension1)||
				GT_element_settings_uses_dimension(settings,&dimension2))
			{
				XtVaSetValues(settings_editor->exterior_button,
					XmNset,GT_element_settings_get_exterior(settings),NULL);
				settings_editor_set_face_number(settings_editor);
				dimension=GT_element_settings_get_dimension(settings);
				XtSetSensitive(settings_editor->exterior_face_entry,
					(1==dimension)||(2==dimension));
				XtManageChild(settings_editor->exterior_face_entry);
			}
			else
			{
				XtUnmanageChild(settings_editor->exterior_face_entry);
			}
		}
		else
		{
			XtUnmanageChild(settings_editor->exterior_face_entry);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_get_settings.  Missing widget");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* settings_editor_display_dimension_specific */

/* identify geometry settings widgets */
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,main_scroll)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,main_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,name_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,coordinate_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,coordinate_field_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,use_element_type_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,use_element_type_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,exterior_face_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,exterior_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,face_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,face_option)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,face_menu)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,radius_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,constant_radius_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,radius_scalar_field_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,radius_scalar_field_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,radius_scale_factor_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,radius_scale_factor_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,iso_surface_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,iso_scalar_field_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,iso_value_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,discretization_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,discretization_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,native_discretization_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,native_discretization_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,native_discretization_field_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor, xi_point_density_field_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor, xi_point_density_field_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor, xi_point_density_field_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,xi_discretization_mode_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,xi_discretization_mode_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,glyph_group_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,glyph_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,glyph_centre_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,glyph_scaling_mode_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,glyph_scaling_mode_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,glyph_size_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,glyph_orientation_scale_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,glyph_orientation_scale_field_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,glyph_scale_factors_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,glyph_scale_factors_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,glyph_variable_scale_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,glyph_variable_scale_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,glyph_variable_scale_field_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,label_field_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,label_field_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,label_field_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,volume_texture_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,volume_texture_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,seed_element_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,seed_element_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,seed_element_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,seed_xi_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,seed_xi_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,streamline_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,streamline_type_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,streamline_length_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,streamline_width_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,stream_vector_field_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,streamline_reverse_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,select_mode_form)
/* identify appearance settings widgets */
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,material_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,line_width_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,line_width_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,texture_coord_field_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,texture_coord_field_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,texture_coord_field_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,render_type_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,render_type_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,streamline_data_type_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,streamline_data_type_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,data_field_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,data_field_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,spectrum_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,spectrum_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(settings_editor, \
	Settings_editor,selected_material_form)

static void settings_editor_destroy_CB(Widget widget,XtPointer client_data,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 5 June 1998

DESCRIPTION :
Callback for the settings_editor dialog - tidies up all details - mem etc
==============================================================================*/
{
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_destroy_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(reason);
	if (settings_editor=(struct Settings_editor *)client_data)
	{
		/* destroy current settings if any */
		if (settings_editor->current_settings)
		{
			DESTROY(GT_element_settings)(&(settings_editor->current_settings));
		}
		*(settings_editor->widget_address)=(Widget)NULL;
		DEALLOCATE(settings_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_destroy_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_destroy_CB */

static void settings_editor_name_text_CB(
	Widget widget,XtPointer client_data,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 22 April 2004

DESCRIPTION :
Called when entry is made into the name text field.
==============================================================================*/
{
	char *name,new_name[200],*text_entry;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_constant_radius_text_CB);
	USE_PARAMETER(reason);
	if (widget&&(settings_editor=(struct Settings_editor *)client_data))
	{
		/* must check if have current settings since we can get losingFocus
			 callback after it is cleared */
		if (settings_editor->current_settings)
		{
			GET_NAME(GT_element_settings)(settings_editor->current_settings,
				&name);
			/* Get the text string */
			XtVaGetValues(widget,XmNvalue,&text_entry,NULL);
			if (text_entry)
			{
				sscanf(text_entry, "%s", new_name);
				if (strcmp(name, new_name))
				{
					GT_element_settings_set_name(
						settings_editor->current_settings, new_name);
					/* inform the client of the change */
					settings_editor_update(settings_editor);
				}
				XtFree(text_entry);
				
				XtVaSetValues(widget,XmNvalue,new_name,NULL);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"settings_editor_constant_radius_text_CB.  Missing text");
			}
			DEALLOCATE(name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_constant_radius_text_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_name_text_CB */

static void settings_editor_exterior_button_CB(Widget widget,
	XtPointer client_data,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 5 June 1998

DESCRIPTION :
Called when the exterior toggle button value changes.
==============================================================================*/
{
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_exterior_button_CB);
	USE_PARAMETER(reason);
	if (widget&&(settings_editor=(struct Settings_editor *)client_data))
	{
		GT_element_settings_set_exterior(settings_editor->current_settings,
			XmToggleButtonGetState(widget));
		/* inform the client of the change */
		settings_editor_update(settings_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_exterior_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_exterior_button_CB */

static void settings_editor_face_button_CB(Widget widget,XtPointer client_data,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 8 June 1998

DESCRIPTION :
Called when the face toggle button value changes.
==============================================================================*/
{
	int face,toggle_state;
	struct Settings_editor *settings_editor;
	Widget face_widget;

	ENTER(settings_editor_face_button_CB);
	USE_PARAMETER(reason);
	if (widget&&(settings_editor=(struct Settings_editor *)client_data))
	{
		if (toggle_state=XmToggleButtonGetState(widget))
		{
			XtVaGetValues(settings_editor->face_option,
				XmNmenuHistory,&face_widget,NULL);
			if (face_widget)
			{
				/* get the face from the face menu  */
				XtVaGetValues(face_widget,XmNuserData,&face,NULL);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"settings_editor_face_button_CB. Missing face widget");
				face=0;
			}
		}
		else
		{
			face=-1;
		}
		GT_element_settings_set_face(settings_editor->current_settings,face);
		/* inform the client of the change */
		settings_editor_update(settings_editor);
		/* set (un)grayed status of face_number option */
		XtSetSensitive(settings_editor->face_option,toggle_state);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_face_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_face_button_CB */

static void settings_editor_face_menu_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 June 1998

DESCRIPTION :
Callback for the option menu - change of face.
==============================================================================*/
{
	struct Settings_editor *settings_editor;
	int new_face_number;
	Widget menu_item_widget;

	ENTER(settings_editor_face_menu_CB);
	if (widget&&(settings_editor=(struct Settings_editor *)client_data)
		&&call_data)
	{
		/* get the widget from the call data */
		if (menu_item_widget=((XmRowColumnCallbackStruct *)call_data)->widget)
		{
			/* Get the face this menu item represents and make it current */
			XtVaGetValues(menu_item_widget,XmNuserData,&new_face_number,NULL);
			if ((0<=new_face_number)&&(6>new_face_number))
			{
				GT_element_settings_set_face(settings_editor->current_settings,
					new_face_number);
				/* inform the client of the change */
				settings_editor_update(settings_editor);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"settings_editor_face_menu_CB.  Invalid face number");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"settings_editor_face_menu_CB.  "
				"Could not find the activated menu item");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_face_menu_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_face_menu_CB */

static void settings_editor_update_coordinate_field(Widget widget,
	void *settings_editor_void,void *coordinate_field_void)
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Callback for change of coordinate field.
==============================================================================*/
{
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_update_coordinate_field);
	USE_PARAMETER(widget);
	if (settings_editor=(struct Settings_editor *)settings_editor_void)
	{
		/* skip messages from chooser if it is grayed out */
		if (XtIsSensitive(settings_editor->coordinate_field_widget))
		{
			GT_element_settings_set_coordinate_field(
				settings_editor->current_settings,
				(struct Computed_field *)coordinate_field_void);
			/* inform the client of the change */
			settings_editor_update(settings_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update_coordinate_field.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_update_coordinate_field */

static void settings_editor_coordinate_button_CB(Widget widget,
	XtPointer client_data,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 16 February 1999

DESCRIPTION :
Called when the coordinate field toggle button value changes.
==============================================================================*/
{
	struct Computed_field *coordinate_field;
	struct GT_element_settings *current_settings;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_coordinate_button_CB);
	USE_PARAMETER(reason);
	if (widget&&(settings_editor=(struct Settings_editor *)client_data)
		&&(current_settings=settings_editor->current_settings))
	{
		if (GT_element_settings_get_coordinate_field(current_settings))
		{
			coordinate_field=(struct Computed_field *)NULL;
		}
		else
		{
			/* get coordinate field from the widget */
			coordinate_field=CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
				settings_editor->coordinate_field_widget);
		}
		GT_element_settings_set_coordinate_field(current_settings,coordinate_field);
		/* (un)gray coordinate field widget */
		XtSetSensitive(settings_editor->coordinate_field_widget,
			NULL != coordinate_field);
		/* inform the client of the change */
		settings_editor_update(settings_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_coordinate_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_coordinate_button_CB */

static void settings_editor_update_use_element_type(Widget widget,
	void *settings_editor_void,void *use_element_type_string_void)
/*******************************************************************************
LAST MODIFIED : 20 March 2001

DESCRIPTION :
Callback for change of use_element_type.
==============================================================================*/
{
	enum Use_element_type use_element_type;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_update_use_element_type);
	USE_PARAMETER(widget);
	if (settings_editor=(struct Settings_editor *)settings_editor_void)
	{
		if (STRING_TO_ENUMERATOR(Use_element_type)(
			(char *)use_element_type_string_void, &use_element_type) &&
			GT_element_settings_set_use_element_type(
				settings_editor->current_settings, use_element_type))
		{
			/* make sure the correct widgets are shown for the new dimension */
			settings_editor_display_dimension_specific(settings_editor);
			/* inform the client of the change */
			settings_editor_update(settings_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update_use_element_type.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_update_use_element_type */

static void settings_editor_constant_radius_text_CB(
	Widget widget,XtPointer client_data,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 22 November 2001

DESCRIPTION :
Called when entry is made into the constant_radius text field.
==============================================================================*/
{
	char *text_entry,temp_string[50];
	float constant_radius,scale_factor;
	struct Computed_field *radius_scalar_field;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_constant_radius_text_CB);
	USE_PARAMETER(reason);
	if (widget&&(settings_editor=(struct Settings_editor *)client_data))
	{
		/* must check if have current settings since we can get losingFocus
			 callback after it is cleared */
		if (settings_editor->current_settings)
		{
			GT_element_settings_get_radius_parameters(
				settings_editor->current_settings,&constant_radius,
				&scale_factor,&radius_scalar_field);
			/* Get the text string */
			XtVaGetValues(widget,XmNvalue,&text_entry,NULL);
			if (text_entry)
			{
				sscanf(text_entry,"%g",&constant_radius);
				GT_element_settings_set_radius_parameters(
					settings_editor->current_settings,constant_radius,
					scale_factor,radius_scalar_field);
				/* inform the client of the change */
				settings_editor_update(settings_editor);
				XtFree(text_entry);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"settings_editor_constant_radius_text_CB.  Missing text");
			}
			/* always restore constant_radius to actual value in use */
			sprintf(temp_string,"%g",constant_radius);
			XtVaSetValues(widget,XmNvalue,temp_string,NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_constant_radius_text_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_constant_radius_text_CB */

static void settings_editor_radius_scalar_field_button_CB(Widget widget,
	XtPointer client_data,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Called when the variable radius toggle button value changes.
==============================================================================*/
{
	float constant_radius,scale_factor;
	int field_set;
	struct Computed_field *radius_scalar_field,*start_field;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_radius_scalar_field_button_CB);
	USE_PARAMETER(reason);
	if (widget&&(settings_editor=(struct Settings_editor *)client_data))
	{
		GT_element_settings_get_radius_parameters(
			settings_editor->current_settings,&constant_radius,
			&scale_factor,&radius_scalar_field);
		start_field=radius_scalar_field;
		if (radius_scalar_field)
		{
			radius_scalar_field=(struct Computed_field *)NULL;
		}
		else
		{
			/* get scalar from widget */
			radius_scalar_field=CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
				settings_editor->radius_scalar_field_widget);
		}
		GT_element_settings_set_radius_parameters(
			settings_editor->current_settings,constant_radius,
			scale_factor,radius_scalar_field);
		/* set status of radius widgets */
		GT_element_settings_get_radius_parameters(
			settings_editor->current_settings,&constant_radius,
			&scale_factor,&radius_scalar_field);
		field_set=((struct Computed_field *)NULL != radius_scalar_field);
		XtVaSetValues(settings_editor->radius_scalar_field_button,
			XmNset,field_set,NULL);
		XtSetSensitive(settings_editor->radius_scalar_field_widget,field_set);
		XtSetSensitive(settings_editor->radius_scale_factor_entry,field_set);
		if (radius_scalar_field != start_field)
		{
			/* inform the client of the change */
			settings_editor_update(settings_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_radius_scalar_field_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_radius_scalar_field_button_CB */

static void settings_editor_update_radius_scalar_field(Widget widget,
	void *settings_editor_void,void *radius_scalar_field_void)
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Callback for change of radius_scalar.
==============================================================================*/
{
	float constant_radius,scale_factor;
	struct Computed_field *radius_scalar_field;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_update_radius_scalar_field);
	USE_PARAMETER(widget);
	if ((settings_editor=(struct Settings_editor *)settings_editor_void)
		&&radius_scalar_field_void)
	{
		/* skip messages from chooser if grayed out */
		if (XtIsSensitive(settings_editor->radius_scalar_field_widget))
		{
			GT_element_settings_get_radius_parameters(
				settings_editor->current_settings,&constant_radius,
				&scale_factor,&radius_scalar_field);
			radius_scalar_field=(struct Computed_field *)radius_scalar_field_void;
			if (GT_element_settings_set_radius_parameters(
				settings_editor->current_settings,constant_radius,
				scale_factor,radius_scalar_field))
			{
				/* inform the client of the change */
				settings_editor_update(settings_editor);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update_radius_scalar_field.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_update_radius_scalar_field */

static void settings_editor_radius_scale_factor_text_CB(
	Widget widget,XtPointer client_data,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 22 November 2001

DESCRIPTION :
Called when entry is made into the radius_scale_factor_text field.
==============================================================================*/
{
	char *text_entry,temp_string[50];
	float constant_radius,scale_factor;
	struct Computed_field *radius_scalar_field;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_radius_scale_factor_text_CB);
	USE_PARAMETER(reason);
	if (widget&&(settings_editor=(struct Settings_editor *)client_data))
	{
		/* must check if have current settings since we can get losingFocus
			 callback after it is cleared */
		if (settings_editor->current_settings)
		{
			GT_element_settings_get_radius_parameters(
				settings_editor->current_settings,&constant_radius,
				&scale_factor,&radius_scalar_field);
			/* Get the text string */
			XtVaGetValues(widget,XmNvalue,&text_entry,NULL);
			if (text_entry)
			{
				sscanf(text_entry,"%g",&scale_factor);
				GT_element_settings_set_radius_parameters(
					settings_editor->current_settings,constant_radius,
					scale_factor,radius_scalar_field);
				/* inform the client of the change */
				settings_editor_update(settings_editor);
				XtFree(text_entry);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"settings_editor_radius_scale_factor_text_CB.  Missing text");
			}
			/* always restore scale_factor to actual value in use */
			sprintf(temp_string,"%g",scale_factor);
			XtVaSetValues(widget,XmNvalue,temp_string,NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_radius_scale_factor_text_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_radius_scale_factor_text_CB */

static void settings_editor_update_iso_scalar_field(Widget widget,
	void *settings_editor_void,void *scalar_field_void)
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Callback for change of iso_scalar field.
==============================================================================*/
{
	double iso_value;
	struct Computed_field *scalar_field;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_update_iso_scalar_field);
	USE_PARAMETER(widget);
	if ((settings_editor=(struct Settings_editor *)settings_editor_void)
		&&scalar_field_void)
	{
		GT_element_settings_get_iso_surface_parameters(
			settings_editor->current_settings,&scalar_field,&iso_value);
		scalar_field=(struct Computed_field *)scalar_field_void;
		if (GT_element_settings_set_iso_surface_parameters(
			settings_editor->current_settings,scalar_field,iso_value))
		{
			/* inform the client of the change */
			settings_editor_update(settings_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update_iso_scalar_field.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_update_iso_scalar_field */

static void settings_editor_iso_value_text_CB(Widget widget,
	XtPointer client_data,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 7 March 2002

DESCRIPTION :
Called when entry is made into the iso_value text field.
==============================================================================*/
{
	char *text_entry, temp_string[50];
	double current_iso_value, iso_value;
	struct Computed_field *scalar_field;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_constant_iso_value_text_CB);
	USE_PARAMETER(reason);	
	if (widget&&(settings_editor=(struct Settings_editor *)client_data))
	{
		/* must check if have current settings since we can get losingFocus
			 callback after it is cleared */
		if (settings_editor->current_settings)
		{
			GT_element_settings_get_iso_surface_parameters(
				settings_editor->current_settings,&scalar_field,&current_iso_value);
			/* Get the text string */
			XtVaGetValues(widget,XmNvalue,&text_entry,NULL);
			if (text_entry)
			{
				sscanf(text_entry,"%lg",&iso_value);
				XtFree(text_entry);
				if (iso_value != current_iso_value)
				{
					GT_element_settings_set_iso_surface_parameters(
						settings_editor->current_settings,scalar_field,iso_value);
					/* inform the client of the change */
					settings_editor_update(settings_editor);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"settings_editor_constant_iso_value_text_CB.  Missing text");
			}
			/* always restore constant_radius to actual value in use */
			sprintf(temp_string,"%g",iso_value);
			XtVaSetValues(widget,XmNvalue,temp_string,NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_constant_iso_value_text_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_iso_value_text_CB */

static void settings_editor_discretization_text_CB(Widget widget,
	XtPointer client_data,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 22 November 2001

DESCRIPTION :
Called when entry is made into the discretization text field.
==============================================================================*/
{
	char *text_entry,temp_string[50];
	struct Element_discretization discretization;
	struct Parse_state *temp_state;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_discretization_text_CB);
	USE_PARAMETER(reason);	
	if (widget&&(settings_editor=(struct Settings_editor *)client_data))
	{
		/* must check if have current settings since we can get losingFocus
			 callback after it is cleared */
		if (settings_editor->current_settings)
		{
			/* Get the text string */
			XtVaGetValues(widget,XmNvalue,&text_entry,NULL);
			if (text_entry)
			{
				if (temp_state=create_Parse_state(text_entry))
				{
					if (set_Element_discretization(temp_state,(void *)&discretization,
						(void *)settings_editor->user_interface)&&
						GT_element_settings_set_discretization(
							settings_editor->current_settings,&discretization,
							settings_editor->user_interface))
					{
						/* inform the client of the change */
						settings_editor_update(settings_editor);
					}
					destroy_Parse_state(&temp_state);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"settings_editor_discretization_text_CB.  "
						"Could not create parse state");
				}
				XtFree(text_entry);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"settings_editor_discretization_text_CB.  Missing text");
			}
			if (GT_element_settings_get_discretization(
				settings_editor->current_settings,&discretization))
			{
				/* always restore constant_radius to actual value in use */
				sprintf(temp_string,"%d*%d*%d",discretization.number_in_xi1,
					discretization.number_in_xi2,discretization.number_in_xi3);
				XtVaSetValues(widget,XmNvalue,temp_string,NULL);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_discretization_text_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_discretization_text_CB */

static void settings_editor_native_discretization_button_CB(Widget widget,
	XtPointer settings_editor_void,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Called when the native_discretization_field toggle button value changes.
==============================================================================*/
{
	int field_set;
	struct FE_field *native_discretization_field;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_native_discretization_button_CB);
	USE_PARAMETER(widget);	
	USE_PARAMETER(reason);	
	if (settings_editor=(struct Settings_editor *)settings_editor_void)
	{
		native_discretization_field=
			GT_element_settings_get_native_discretization_field(
				settings_editor->current_settings);
		if (native_discretization_field)
		{
			native_discretization_field=(struct FE_field *)NULL;
		}
		else
		{
			/* get data field from widget */
			native_discretization_field =
				FE_REGION_CHOOSE_OBJECT_GET_OBJECT(FE_field)(
					settings_editor->native_discretization_field_widget);
		}
		GT_element_settings_set_native_discretization_field(
			settings_editor->current_settings,native_discretization_field);
		/* inform the client of the change */
		settings_editor_update(settings_editor);
		/* set status of button and chooser */
		native_discretization_field=
			GT_element_settings_get_native_discretization_field(
				settings_editor->current_settings);
		field_set=((struct FE_field *)NULL != native_discretization_field);
		XtVaSetValues(settings_editor->native_discretization_button,
			XmNset,field_set,NULL);
		XtSetSensitive(settings_editor->native_discretization_field_widget,
			field_set);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_native_discretization_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_native_discretization_button_CB */

static void settings_editor_update_native_discretization_field(Widget widget,
	void *settings_editor_void,void *native_discretization_field_void)
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Callback for change of native_discretization_field.
==============================================================================*/
{
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_update_native_discretization_field);
	USE_PARAMETER(widget);
	if (settings_editor=(struct Settings_editor *)settings_editor_void)
	{
		/* skip messages from chooser if grayed out */
		if (XtIsSensitive(settings_editor->native_discretization_field_widget))
		{
			if (GT_element_settings_set_native_discretization_field(
				settings_editor->current_settings,
				(struct FE_field *)native_discretization_field_void))
			{
				/* inform the client of the change */
				settings_editor_update(settings_editor);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update_native_discretization_field.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_update_native_discretization_field */

static void settings_editor_update_xi_point_density_field(Widget widget,
	void *settings_editor_void, void *xi_point_density_field_void)
/*******************************************************************************
LAST MODIFIED : 1 May 2001

DESCRIPTION :
Callback for change of scalar data field.
==============================================================================*/
{
	enum Xi_discretization_mode xi_discretization_mode;
	struct Computed_field *xi_point_density_field;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_update_xi_point_density_field);
	USE_PARAMETER(widget);
	if (settings_editor = (struct Settings_editor *)settings_editor_void)
	{
		/* skip messages from chooser if it is grayed out */
		if (XtIsSensitive(settings_editor->xi_point_density_field_widget))
		{
			GT_element_settings_get_xi_discretization(
				settings_editor->current_settings, &xi_discretization_mode,
				&xi_point_density_field);
			xi_point_density_field =
				(struct Computed_field *)xi_point_density_field_void;
			GT_element_settings_set_xi_discretization(
				settings_editor->current_settings, xi_discretization_mode,
				xi_point_density_field);
			/* inform the client of the change */
			settings_editor_update(settings_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update_xi_point_density_field.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_update_xi_point_density_field */

static void settings_editor_update_xi_discretization_mode(Widget widget,
	void *settings_editor_void,void *xi_discretization_mode_string_void)
/*******************************************************************************
LAST MODIFIED : 3 May 2001

DESCRIPTION :
Callback for change of xi_discretization_mode.
==============================================================================*/
{
	enum Xi_discretization_mode old_xi_discretization_mode,
		xi_discretization_mode;
	struct Computed_field *old_xi_point_density_field, *xi_point_density_field;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_update_xi_discretization_mode);
	USE_PARAMETER(widget);
	if (settings_editor=(struct Settings_editor *)settings_editor_void)
	{
		if (GT_element_settings_get_xi_discretization(
			settings_editor->current_settings, &old_xi_discretization_mode,
			&old_xi_point_density_field) &&
			STRING_TO_ENUMERATOR(Xi_discretization_mode)(
				(char *)xi_discretization_mode_string_void, &xi_discretization_mode))
		{
			xi_point_density_field = old_xi_point_density_field;
			if ((XI_DISCRETIZATION_CELL_DENSITY == xi_discretization_mode) ||
				(XI_DISCRETIZATION_CELL_POISSON == xi_discretization_mode))
			{
				if (!xi_point_density_field)
				{
					/* get xi_point_density_field from widget */
					xi_point_density_field = CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
						settings_editor->xi_point_density_field_widget);
				}
			}
			else
			{
				xi_point_density_field = (struct Computed_field *)NULL;
			}
			if (GT_element_settings_set_xi_discretization(
				settings_editor->current_settings, xi_discretization_mode,
				xi_point_density_field))
			{
				/* inform the client of the change */
				settings_editor_update(settings_editor);
			}
			else
			{
				xi_discretization_mode = old_xi_discretization_mode;
				xi_point_density_field = old_xi_point_density_field;
				/* make sure the chooser is showing the previous mode */
				choose_enumerator_set_string(
					settings_editor->xi_discretization_mode_widget,
					ENUMERATOR_STRING(Xi_discretization_mode)(xi_discretization_mode));
			}
		}
		XtSetSensitive(settings_editor->discretization_entry,
			XI_DISCRETIZATION_EXACT_XI != xi_discretization_mode);
		XtSetSensitive(settings_editor->native_discretization_entry,
			XI_DISCRETIZATION_EXACT_XI != xi_discretization_mode);
		XtSetSensitive(settings_editor->xi_point_density_field_entry,
			(XI_DISCRETIZATION_CELL_DENSITY == xi_discretization_mode) ||
			(XI_DISCRETIZATION_CELL_POISSON == xi_discretization_mode));
		XtSetSensitive(settings_editor->seed_xi_entry,
			XI_DISCRETIZATION_EXACT_XI == xi_discretization_mode);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update_xi_discretization_mode.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_update_xi_discretization_mode */

static int settings_editor_display_glyph_specific(
	struct Settings_editor *settings_editor)
/*******************************************************************************
LAST MODIFIED : 24 November 2000

DESCRIPTION :
Ensures all the glyph widgets display the appropriate values for the current
settings. Does nothing if current settings type does not use glyphs.
==============================================================================*/
{
	char temp_string[50];
	enum Glyph_scaling_mode glyph_scaling_mode;
	enum GT_element_settings_type settings_type;
	int field_set1, field_set2, return_code;
	struct Computed_field *orientation_scale_field, *variable_scale_field;
	struct GT_element_settings *settings;
	struct GT_object *glyph;
	Triple glyph_centre, glyph_scale_factors, glyph_size;

	ENTER(settings_editor_display_glyph_specific);
	if (settings_editor && (settings = settings_editor->current_settings))
	{
		if (settings = settings_editor->current_settings)
		{
			settings_type = GT_element_settings_get_settings_type(settings);
			if (((GT_ELEMENT_SETTINGS_NODE_POINTS == settings_type) ||
				(GT_ELEMENT_SETTINGS_DATA_POINTS == settings_type) ||
				(GT_ELEMENT_SETTINGS_ELEMENT_POINTS == settings_type)) &&
				GT_element_settings_get_glyph_parameters(settings,
					&glyph, &glyph_scaling_mode, glyph_centre, glyph_size,
					&orientation_scale_field, glyph_scale_factors,
					&variable_scale_field))
			{
				CHOOSE_OBJECT_LIST_SET_OBJECT(GT_object)(
					settings_editor->glyph_widget,glyph);
				choose_enumerator_set_string(settings_editor->glyph_scaling_mode_widget,
					ENUMERATOR_STRING(Glyph_scaling_mode)(glyph_scaling_mode));
				sprintf(temp_string,"%g*%g*%g",
					glyph_size[0],glyph_size[1],glyph_size[2]);
				XtVaSetValues(settings_editor->glyph_size_text,XmNvalue,
					temp_string,NULL);
				sprintf(temp_string,"%g,%g,%g",
					glyph_centre[0],glyph_centre[1],glyph_centre[2]);
				XtVaSetValues(settings_editor->glyph_centre_text,XmNvalue,
					temp_string,NULL);

				if (field_set1 =
					((struct Computed_field *)NULL != orientation_scale_field))
				{
					CHOOSE_OBJECT_SET_OBJECT(Computed_field)(
						settings_editor->glyph_orientation_scale_field_widget,
						orientation_scale_field);
				}
				XtVaSetValues(settings_editor->glyph_orientation_scale_button,
					XmNset, field_set1, NULL);
				XtSetSensitive(settings_editor->glyph_orientation_scale_field_widget,
					field_set1);

				if (field_set2 =
					((struct Computed_field *)NULL != variable_scale_field))
				{
					CHOOSE_OBJECT_SET_OBJECT(Computed_field)(
						settings_editor->glyph_variable_scale_field_widget,
						variable_scale_field);
				}
				XtVaSetValues(settings_editor->glyph_variable_scale_button,
					XmNset, field_set2, NULL);
				XtSetSensitive(settings_editor->glyph_variable_scale_field_widget,
					field_set2);

				sprintf(temp_string,"%g*%g*%g",glyph_scale_factors[0],
					glyph_scale_factors[1],glyph_scale_factors[2]);
				XtVaSetValues(settings_editor->
					glyph_scale_factors_text,XmNvalue,temp_string,NULL);
				XtSetSensitive(settings_editor->glyph_scale_factors_entry,
					field_set1 || field_set2);
			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_display_glyph_specific.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* settings_editor_display_glyph_specific */

static void settings_editor_update_glyph(Widget widget,
	void *settings_editor_void,void *glyph_void)
/*******************************************************************************
LAST MODIFIED : 20 March 2001

DESCRIPTION :
Callback for change of glyph.
==============================================================================*/
{
	enum Glyph_scaling_mode glyph_scaling_mode;
	struct Computed_field *orientation_scale_field, *variable_scale_field;
	struct GT_object *glyph,*old_glyph;
	struct Settings_editor *settings_editor;
	Triple glyph_centre,glyph_scale_factors,glyph_size;

	ENTER(settings_editor_update_glyph);
	USE_PARAMETER(widget);
	if ((settings_editor=(struct Settings_editor *)settings_editor_void)
		&&(glyph=(struct GT_object *)glyph_void))
	{
		if (GT_element_settings_get_glyph_parameters(
			settings_editor->current_settings, &old_glyph, &glyph_scaling_mode,
			glyph_centre, glyph_size, &orientation_scale_field, glyph_scale_factors,
			&variable_scale_field)&&
			GT_element_settings_set_glyph_parameters(
				settings_editor->current_settings, glyph, glyph_scaling_mode,
				glyph_centre, glyph_size,	orientation_scale_field, glyph_scale_factors,
				variable_scale_field))
		{
			/* inform the client of the change */
			settings_editor_update(settings_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update_glyph.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_update_glyph */

static void settings_editor_update_glyph_scaling_mode(Widget widget,
	void *settings_editor_void,void *glyph_scaling_mode_string_void)
/*******************************************************************************
LAST MODIFIED : 20 March 2001

DESCRIPTION :
Callback for change of glyph.
==============================================================================*/
{
	enum Glyph_scaling_mode glyph_scaling_mode;
	struct Computed_field *orientation_scale_field, *variable_scale_field;
	struct GT_object *glyph;
	struct Settings_editor *settings_editor;
	Triple glyph_centre,glyph_scale_factors,glyph_size;

	ENTER(settings_editor_update_glyph_scaling_mode);
	USE_PARAMETER(widget);
	if (settings_editor=(struct Settings_editor *)settings_editor_void)
	{
		if (GT_element_settings_get_glyph_parameters(
			settings_editor->current_settings, &glyph, &glyph_scaling_mode,
			glyph_centre, glyph_size, &orientation_scale_field, glyph_scale_factors,
			&variable_scale_field))
		{
			if (STRING_TO_ENUMERATOR(Glyph_scaling_mode)(
				(char *)glyph_scaling_mode_string_void, &glyph_scaling_mode))
			{
				GT_element_settings_set_glyph_parameters(
					settings_editor->current_settings, glyph, glyph_scaling_mode,
					glyph_centre,	glyph_size,	orientation_scale_field,
					glyph_scale_factors, variable_scale_field);
			}
			settings_editor_display_glyph_specific(settings_editor);
			/* inform the client of the change */
			settings_editor_update(settings_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update_glyph_scaling_mode.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_update_glyph_scaling_mode */

static void settings_editor_glyph_size_text_CB(Widget widget,
	XtPointer client_data,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 22 November 2001

DESCRIPTION :
Called when entry is made into the glyph_size text field.
==============================================================================*/
{
	char *text_entry;
	enum Glyph_scaling_mode glyph_scaling_mode;
	struct Computed_field *orientation_scale_field, *variable_scale_field;
	struct GT_object *glyph;
	struct Parse_state *temp_state;
	struct Settings_editor *settings_editor;
	Triple glyph_centre,glyph_scale_factors,glyph_size;

	ENTER(settings_editor_glyph_size_text_CB);
	USE_PARAMETER(reason);
	if (widget&&(settings_editor=(struct Settings_editor *)client_data))
	{
		/* must check if have current settings since we can get losingFocus
			 callback after it is cleared */
		if (settings_editor->current_settings)
		{
			if (GT_element_settings_get_glyph_parameters(
				settings_editor->current_settings, &glyph, &glyph_scaling_mode, glyph_centre,
				glyph_size, &orientation_scale_field, glyph_scale_factors,
				&variable_scale_field))
			{
				/* Get the text string */
				XtVaGetValues(widget,XmNvalue,&text_entry,NULL);
				if (text_entry)
				{
					/* clean up spaces? */
					if (temp_state=create_Parse_state(text_entry))
					{
						set_special_float3(temp_state,glyph_size,"*");
						GT_element_settings_set_glyph_parameters(
							settings_editor->current_settings, glyph, glyph_scaling_mode,
							glyph_centre, glyph_size, orientation_scale_field,
							glyph_scale_factors, variable_scale_field);
						/* inform the client of the change */
						settings_editor_update(settings_editor);
						destroy_Parse_state(&temp_state);
					}
					XtFree(text_entry);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"settings_editor_glyph_size_text_CB.  Missing text");
				}
				settings_editor_display_glyph_specific(settings_editor);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_glyph_size_text_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_glyph_size_text_CB */

static void settings_editor_glyph_centre_text_CB(Widget widget,
	XtPointer client_data,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 22 November 2001

DESCRIPTION :
Called when entry is made into the glyph_centre text field.
==============================================================================*/
{
	char *text_entry;
	enum Glyph_scaling_mode glyph_scaling_mode;
	static int number_of_components=3;
	struct Computed_field *orientation_scale_field, *variable_scale_field;
	struct GT_object *glyph;
	struct Parse_state *temp_state;
	struct Settings_editor *settings_editor;
	Triple glyph_centre, glyph_scale_factors, glyph_size;

	ENTER(settings_editor_glyph_centre_text_CB);
	USE_PARAMETER(reason);
	if (widget&&(settings_editor=(struct Settings_editor *)client_data))
	{
		/* must check if have current settings since we can get losingFocus
			 callback after it is cleared */
		if (settings_editor->current_settings)
		{
			if (GT_element_settings_get_glyph_parameters(
				settings_editor->current_settings, &glyph, &glyph_scaling_mode, glyph_centre,
				glyph_size, &orientation_scale_field, glyph_scale_factors,
				&variable_scale_field))
			{
				/* Get the text string */
				XtVaGetValues(widget,XmNvalue,&text_entry,NULL);
				if (text_entry)
				{
					/* clean up spaces? */
					if (temp_state=create_Parse_state(text_entry))
					{
						set_float_vector(temp_state,glyph_centre,
							(void *)&number_of_components);
						GT_element_settings_set_glyph_parameters(
							settings_editor->current_settings, glyph, glyph_scaling_mode,
							glyph_centre, glyph_size, orientation_scale_field,
							glyph_scale_factors, variable_scale_field);
						/* inform the client of the change */
						settings_editor_update(settings_editor);
						destroy_Parse_state(&temp_state);
					}
					XtFree(text_entry);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"settings_editor_glyph_centre_text_CB.  Missing text");
				}
				settings_editor_display_glyph_specific(settings_editor);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_glyph_centre_text_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_glyph_centre_text_CB */

static void settings_editor_glyph_orientation_scale_button_CB(Widget widget,
	XtPointer client_data, unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Called when the glyph_orientation_scale toggle button value changes.
==============================================================================*/
{
	enum Glyph_scaling_mode glyph_scaling_mode;
	struct Computed_field *orientation_scale_field, *variable_scale_field;
	struct GT_object *glyph;
	struct Settings_editor *settings_editor;
	Triple glyph_centre,glyph_scale_factors,glyph_size;

	ENTER(settings_editor_glyph_orientation_scale_button_CB);
	USE_PARAMETER(reason);
	if (widget&&(settings_editor=(struct Settings_editor *)client_data))
	{
		GT_element_settings_get_glyph_parameters(
			settings_editor->current_settings, &glyph, &glyph_scaling_mode, glyph_centre,
			glyph_size, &orientation_scale_field, glyph_scale_factors,
			&variable_scale_field);
		if (orientation_scale_field)
		{
			orientation_scale_field=(struct Computed_field *)NULL;
		}
		else
		{
			/* get orientation_scale_field from widget */
			orientation_scale_field=CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
				settings_editor->glyph_orientation_scale_field_widget);
		}
		GT_element_settings_set_glyph_parameters(
			settings_editor->current_settings, glyph, glyph_scaling_mode,
			glyph_centre, glyph_size, orientation_scale_field,
			glyph_scale_factors, variable_scale_field);
		settings_editor_display_glyph_specific(settings_editor);
		/* inform the client of the change */
		settings_editor_update(settings_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_glyph_orientation_scale_button_CB.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_glyph_orientation_scale_button_CB */

static void settings_editor_update_glyph_orientation_scale_field(Widget widget,
	void *settings_editor_void,void *orientation_scale_field_void)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Callback for change of orientation_scale_field.
==============================================================================*/
{
	enum Glyph_scaling_mode glyph_scaling_mode;
	struct Computed_field *orientation_scale_field, *variable_scale_field;
	struct GT_object *glyph;
	struct Settings_editor *settings_editor;
	Triple glyph_centre, glyph_scale_factors, glyph_size;

	ENTER(settings_editor_update_glyph_orientation_scale_field);
	USE_PARAMETER(widget);
	if ((settings_editor=(struct Settings_editor *)settings_editor_void)
		&&orientation_scale_field_void)
	{
		/* skip messages from chooser if grayed out */
		if (XtIsSensitive(settings_editor->glyph_orientation_scale_field_widget))
		{
			GT_element_settings_get_glyph_parameters(
				settings_editor->current_settings, &glyph, &glyph_scaling_mode,
				glyph_centre, glyph_size, &orientation_scale_field, glyph_scale_factors,
				&variable_scale_field);
			orientation_scale_field=
				(struct Computed_field *)orientation_scale_field_void;
			if (GT_element_settings_set_glyph_parameters(
				settings_editor->current_settings, glyph, glyph_scaling_mode,
				glyph_centre, glyph_size, orientation_scale_field,
				glyph_scale_factors, variable_scale_field))
			{
				/* inform the client of the change */
				settings_editor_update(settings_editor);
			}
			settings_editor_display_glyph_specific(settings_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update_glyph_orientation_scale_field.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_update_glyph_orientation_scale_field */

static void settings_editor_glyph_scale_factors_text_CB(Widget widget,
	XtPointer client_data,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 22 November 2001

DESCRIPTION :
Called when entry is made into the glyph_scale_factors text field.
==============================================================================*/
{
	char *text_entry;
	enum Glyph_scaling_mode glyph_scaling_mode;
	struct Computed_field *orientation_scale_field, *variable_scale_field;
	struct GT_object *glyph;
	struct Parse_state *temp_state;
	struct Settings_editor *settings_editor;
	Triple glyph_centre,glyph_scale_factors,glyph_size;

	ENTER(settings_editor_glyph_scale_factors_text_CB);
	USE_PARAMETER(reason);
	if (widget&&(settings_editor=(struct Settings_editor *)client_data))
	{
		/* must check if have current settings since we can get losingFocus
			 callback after it is cleared */
		if (settings_editor->current_settings)
		{
			if (GT_element_settings_get_glyph_parameters(
				settings_editor->current_settings, &glyph, &glyph_scaling_mode, glyph_centre,
				glyph_size, &orientation_scale_field, glyph_scale_factors,
				&variable_scale_field))
			{
				/* Get the text string */
				XtVaGetValues(widget,XmNvalue,&text_entry,NULL);
				if (text_entry)
				{
					/* clean up spaces? */
					if (temp_state=create_Parse_state(text_entry))
					{
						set_special_float3(temp_state,glyph_scale_factors,"*");
						GT_element_settings_set_glyph_parameters(
							settings_editor->current_settings, glyph, glyph_scaling_mode,
							glyph_centre, glyph_size, orientation_scale_field,
							glyph_scale_factors, variable_scale_field);
						/* inform the client of the change */
						settings_editor_update(settings_editor);
						destroy_Parse_state(&temp_state);
					}
					XtFree(text_entry);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"settings_editor_glyph_scale_factors_text_CB.  Missing text");
				}
				settings_editor_display_glyph_specific(settings_editor);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_glyph_scale_factors_text_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_glyph_scale_factors_text_CB */

static void settings_editor_glyph_variable_scale_button_CB(Widget widget,
	XtPointer client_data, unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Called when the glyph_variable_scale toggle button value changes.
==============================================================================*/
{
	enum Glyph_scaling_mode glyph_scaling_mode;
	struct Computed_field *orientation_scale_field, *variable_scale_field;
	struct GT_object *glyph;
	struct Settings_editor *settings_editor;
	Triple glyph_centre,glyph_scale_factors,glyph_size;

	ENTER(settings_editor_glyph_variable_scale_button_CB);
	USE_PARAMETER(reason);
	if (widget&&(settings_editor=(struct Settings_editor *)client_data))
	{
		GT_element_settings_get_glyph_parameters(
			settings_editor->current_settings, &glyph, &glyph_scaling_mode, glyph_centre,
			glyph_size, &orientation_scale_field, glyph_scale_factors,
			&variable_scale_field);
		if (variable_scale_field)
		{
			variable_scale_field=(struct Computed_field *)NULL;
		}
		else
		{
			/* get variable_scale_field from widget */
			variable_scale_field=CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
				settings_editor->glyph_variable_scale_field_widget);
		}
		GT_element_settings_set_glyph_parameters(
			settings_editor->current_settings, glyph, glyph_scaling_mode,
			glyph_centre, glyph_size, orientation_scale_field,
			glyph_scale_factors, variable_scale_field);
		settings_editor_display_glyph_specific(settings_editor);
		/* inform the client of the change */
		settings_editor_update(settings_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_glyph_variable_scale_button_CB.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_glyph_variable_scale_button_CB */

static void settings_editor_update_glyph_variable_scale_field(Widget widget,
	void *settings_editor_void,void *variable_scale_field_void)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Callback for change of variable_scale_field.
==============================================================================*/
{
	enum Glyph_scaling_mode glyph_scaling_mode;
	struct Computed_field *orientation_scale_field, *variable_scale_field;
	struct GT_object *glyph;
	struct Settings_editor *settings_editor;
	Triple glyph_centre, glyph_scale_factors, glyph_size;

	ENTER(settings_editor_update_glyph_variable_scale_field);
	USE_PARAMETER(widget);
	if ((settings_editor = (struct Settings_editor *)settings_editor_void) &&
		variable_scale_field_void)
	{
		/* skip messages from chooser if grayed out */
		if (XtIsSensitive(settings_editor->glyph_variable_scale_field_widget))
		{
			GT_element_settings_get_glyph_parameters(
				settings_editor->current_settings, &glyph, &glyph_scaling_mode,
				glyph_centre, glyph_size, &orientation_scale_field, glyph_scale_factors,
				&variable_scale_field);
			variable_scale_field = (struct Computed_field *)variable_scale_field_void;
			if (GT_element_settings_set_glyph_parameters(
				settings_editor->current_settings, glyph, glyph_scaling_mode,
				glyph_centre, glyph_size, orientation_scale_field,
				glyph_scale_factors, variable_scale_field))
			{
				/* inform the client of the change */
				settings_editor_update(settings_editor);
			}
			settings_editor_display_glyph_specific(settings_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update_glyph_variable_scale_field.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_update_glyph_variable_scale_field */

static void settings_editor_label_button_CB(Widget widget,
	XtPointer settings_editor_void,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 28 June 1999

DESCRIPTION :
Called when the label field toggle button value changes.
==============================================================================*/
{
	int field_set;
	struct Computed_field *label_field,*start_field;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_label_button_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(reason);
	if (settings_editor=(struct Settings_editor *)settings_editor_void)
	{
		start_field=label_field=
			GT_element_settings_get_label_field(settings_editor->current_settings);
		if (label_field)
		{
			label_field=(struct Computed_field *)NULL;
		}
		else
		{
			/* get label field from widget */
			label_field=CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
				settings_editor->label_field_widget);
		}
		GT_element_settings_set_label_field(settings_editor->current_settings,
			label_field);
		/* set status of label field button and widgets */
		label_field=
			GT_element_settings_get_label_field(settings_editor->current_settings);
		field_set=((struct Computed_field *)NULL != label_field);
		XtVaSetValues(settings_editor->label_field_button,
			XmNset,field_set,NULL);
		XtSetSensitive(settings_editor->label_field_widget,field_set);
		if (label_field != start_field)
		{
			/* inform the client of the change */
			settings_editor_update(settings_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_label_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_label_button_CB */

static void settings_editor_update_label_field(Widget widget,
	void *settings_editor_void,void *label_field_void)
/*******************************************************************************
LAST MODIFIED : 28 June 1999

DESCRIPTION :
Callback for change of label field.
==============================================================================*/
{
	struct Computed_field *label_field;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_update_label_field);
	USE_PARAMETER(widget);
	if ((settings_editor=(struct Settings_editor *)settings_editor_void)
		&&label_field_void)
	{
		/* skip messages from chooser if it is grayed out */
		if (XtIsSensitive(settings_editor->label_field_widget))
		{
			label_field=(struct Computed_field *)label_field_void;
			GT_element_settings_set_label_field(
				settings_editor->current_settings,label_field);
			/* inform the client of the change */
			settings_editor_update(settings_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update_label_field.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_update_label_field */

static void settings_editor_update_volume_texture(Widget widget,
	void *settings_editor_void,void *volume_texture_void)
/*******************************************************************************
LAST MODIFIED : 27 July 1998

DESCRIPTION :
Callback for change of volume_texture.
==============================================================================*/
{
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_update_volume_texture);
	USE_PARAMETER(widget);
	if (settings_editor=(struct Settings_editor *)settings_editor_void)
	{
		GT_element_settings_set_volume_texture(
			settings_editor->current_settings,
			(struct VT_volume_texture *)volume_texture_void);
		/* inform the client of the change */
		settings_editor_update(settings_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update_volume_texture.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_update_volume_texture */

static void settings_editor_seed_element_button_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 August 1998

DESCRIPTION :
Called when the seed element toggle button value changes.
==============================================================================*/
{
	struct FE_element *seed_element;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_seed_element_button_CB);
	USE_PARAMETER(call_data);
	if (widget&&(settings_editor=(struct Settings_editor *)client_data))
	{
		seed_element=GT_element_settings_get_seed_element(
			settings_editor->current_settings);
		if (seed_element)
		{
			seed_element=(struct FE_element *)NULL;
		}
		else
		{
			seed_element=TEXT_CHOOSE_FROM_FE_REGION_GET_OBJECT(FE_element)(
				settings_editor->seed_element_widget);
		}
		GT_element_settings_set_seed_element(
			settings_editor->current_settings,seed_element);
		/* set state of toggle button and grayness of seed_element widget */
		XtVaSetValues(settings_editor->seed_element_button,
			XmNset,(NULL != seed_element),NULL);
		XtSetSensitive(settings_editor->seed_element_widget,
			NULL != seed_element);
		/* inform the client of the change */
		settings_editor_update(settings_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_seed_element_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_seed_element_button_CB */

static void settings_editor_update_seed_element(Widget widget,
	void *settings_editor_void,void *seed_element_void)
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Callback for change of seed element.
==============================================================================*/
{
	struct FE_element *seed_element;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_update_seed_element);
	USE_PARAMETER(widget);
	if (settings_editor=(struct Settings_editor *)settings_editor_void)
	{
		/* skip messages from chooser if it is grayed out */
		if (XtIsSensitive(settings_editor->seed_element_widget))
		{
			seed_element=(struct FE_element *)seed_element_void;
			GT_element_settings_set_seed_element(settings_editor->current_settings,
				seed_element);
			/* inform the client of the change */
			settings_editor_update(settings_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update_seed_element.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_update_seed_element */

static void settings_editor_seed_xi_text_CB(Widget widget,
	XtPointer client_data,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 22 November 2001

DESCRIPTION :
Called when entry is made into the seed_xi text field.
==============================================================================*/
{
	char *text_entry,temp_string[50];
	static int number_of_components=3;
	struct Parse_state *temp_state;
	struct Settings_editor *settings_editor;
	Triple seed_xi;

	ENTER(settings_editor_seed_xi_text_CB);
	USE_PARAMETER(reason);
	if (widget&&(settings_editor=(struct Settings_editor *)client_data))
	{
		/* must check if have current settings since we can get losingFocus
			 callback after it is cleared */
		if (settings_editor->current_settings)
		{
			if (GT_element_settings_get_seed_xi(
				settings_editor->current_settings,seed_xi))
			{
				/* Get the text string */
				XtVaGetValues(widget,XmNvalue,&text_entry,NULL);
				if (text_entry)
				{
					/* clean up spaces? */
					if (temp_state=create_Parse_state(text_entry))
					{
						set_float_vector(temp_state,seed_xi,
							(void *)&number_of_components);
						GT_element_settings_set_seed_xi(
							settings_editor->current_settings,seed_xi);
						/* inform the client of the change */
						settings_editor_update(settings_editor);
						destroy_Parse_state(&temp_state);
					}
					XtFree(text_entry);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"settings_editor_seed_xi_text_CB.  Missing text");
				}
				/* always re-display the values actually set */
				sprintf(temp_string,"%g,%g,%g",seed_xi[0],seed_xi[1],seed_xi[2]);
				XtVaSetValues(settings_editor->seed_xi_text,XmNvalue,
					temp_string,NULL);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_seed_xi_text_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_seed_xi_text_CB */

static void settings_editor_update_streamline_type(Widget widget,
	void *settings_editor_void,void *streamline_type_string_void)
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
Callback for change of streamline_type.
==============================================================================*/
{
	enum Streamline_type streamline_type;
	float streamline_length,streamline_width;
	int reverse_track;
	struct Computed_field *stream_vector_field;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_update_streamline_type);
	USE_PARAMETER(widget);
	if (settings_editor=(struct Settings_editor *)settings_editor_void)
	{
		if (GT_element_settings_get_streamline_parameters(
			settings_editor->current_settings, &streamline_type, &stream_vector_field,
			&reverse_track, &streamline_length, &streamline_width) &&
			STRING_TO_ENUMERATOR(Streamline_type)(
				(char *)streamline_type_string_void, &streamline_type) &&
			GT_element_settings_set_streamline_parameters(
				settings_editor->current_settings, streamline_type, stream_vector_field,
				reverse_track, streamline_length, streamline_width))
		{
			/* inform the client of the change */
			settings_editor_update(settings_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update_streamline_type.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_update_streamline_type */

static void settings_editor_streamline_length_text_CB(
	Widget widget,XtPointer client_data,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 22 November 2001

DESCRIPTION :
Called when entry is made into the radius_scale_factor_text field.
==============================================================================*/
{
	char *text_entry,temp_string[50];
	enum Streamline_type streamline_type;
	float streamline_length,streamline_width;
	int reverse_track;
	struct Computed_field *stream_vector_field;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_streamline_length_text_CB);
	USE_PARAMETER(reason);
	if (widget&&(settings_editor=(struct Settings_editor *)client_data))
	{
		/* must check if have current settings since we can get losingFocus
			 callback after it is cleared */
		if (settings_editor->current_settings)
		{
			GT_element_settings_get_streamline_parameters(
				settings_editor->current_settings,&streamline_type,
				&stream_vector_field,&reverse_track,&streamline_length,
				&streamline_width);
			/* Get the text string */
			XtVaGetValues(widget,XmNvalue,&text_entry,NULL);
			if (text_entry)
			{
				sscanf(text_entry,"%g",&streamline_length);
				GT_element_settings_set_streamline_parameters(
					settings_editor->current_settings,streamline_type,
					stream_vector_field,reverse_track,streamline_length,
					streamline_width);
				/* inform the client of the change */
				settings_editor_update(settings_editor);
				XtFree(text_entry);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"settings_editor_streamline_length_text_CB.  Missing text");
			}
			/* always restore streamline_length to actual value in use */
			sprintf(temp_string,"%g",streamline_length);
			XtVaSetValues(widget,XmNvalue,temp_string,NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_streamline_length_text_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_streamline_length_text_CB */

static void settings_editor_streamline_width_text_CB(
	Widget widget,XtPointer client_data,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 22 November 2001

DESCRIPTION :
Called when entry is made into the radius_scale_factor_text field.
==============================================================================*/
{
	char *text_entry,temp_string[50];
	enum Streamline_type streamline_type;
	float streamline_length,streamline_width;
	int reverse_track;
	struct Computed_field *stream_vector_field;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_streamline_width_text_CB);
	USE_PARAMETER(reason);
	if (widget&&(settings_editor=(struct Settings_editor *)client_data))
	{
		/* must check if have current settings since we can get losingFocus
			 callback after it is cleared */
		if (settings_editor->current_settings)
		{
			GT_element_settings_get_streamline_parameters(
				settings_editor->current_settings,&streamline_type,
				&stream_vector_field,&reverse_track,&streamline_length,
				&streamline_width);
			/* Get the text string */
			XtVaGetValues(widget,XmNvalue,&text_entry,NULL);
			if (text_entry)
			{
				sscanf(text_entry,"%g",&streamline_width);
				GT_element_settings_set_streamline_parameters(
					settings_editor->current_settings,streamline_type,
					stream_vector_field,reverse_track,streamline_length,streamline_width);
				/* inform the client of the change */
				settings_editor_update(settings_editor);
				XtFree(text_entry);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"settings_editor_streamline_width_text_CB.  Missing text");
			}
			/* always restore streamline_width to actual value in use */
			sprintf(temp_string,"%g",streamline_width);
			XtVaSetValues(widget,XmNvalue,temp_string,NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_streamline_width_text_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_streamline_width_text_CB */

static void settings_editor_update_render_type(Widget widget,
	void *settings_editor_void, void *render_type_string_void)
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
Callback for change of render_type.
==============================================================================*/
{
	enum Render_type render_type;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_update_render_type);
	USE_PARAMETER(widget);
	if (settings_editor = (struct Settings_editor *)settings_editor_void)
	{
		if (STRING_TO_ENUMERATOR(Render_type)(
			(char *)render_type_string_void, &render_type) &&
			GT_element_settings_set_render_type(
				settings_editor->current_settings, render_type))
		{
			/* inform the client of the change */
			settings_editor_update(settings_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update_render_type.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_update_render_type */

static void settings_editor_update_stream_vector_field(Widget widget,
	void *settings_editor_void,void *stream_vector_field_void)
/*******************************************************************************
LAST MODIFIED : 23 March 1999

DESCRIPTION :
Callback for change of stream_vector_field.
==============================================================================*/
{
	enum Streamline_type streamline_type;
	float streamline_length,streamline_width;
	int reverse_track;
	struct Computed_field *stream_vector_field;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_update_stream_vector_field);
	USE_PARAMETER(widget);
	if (settings_editor=(struct Settings_editor *)settings_editor_void)
	{
		if (GT_element_settings_get_streamline_parameters(
			settings_editor->current_settings,&streamline_type,
			&stream_vector_field,&reverse_track,&streamline_length,
			&streamline_width)&&
			GT_element_settings_set_streamline_parameters(
				settings_editor->current_settings,streamline_type,
				(struct Computed_field *)stream_vector_field_void,reverse_track,
				streamline_length,streamline_width))
		{
			/* inform the client of the change */
			settings_editor_update(settings_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update_stream_vector_field.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_update_stream_vector_field */

static void settings_editor_streamline_reverse_button_CB(
	Widget widget,XtPointer client_data,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 24 March 1999

DESCRIPTION :
Called when entry is made into the radius_scale_factor_text field.
==============================================================================*/
{
	enum Streamline_type streamline_type;
	float streamline_length,streamline_width;
	int reverse_track;
	struct Computed_field *stream_vector_field;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_streamline_reverse_button_CB);
	USE_PARAMETER(reason);
	if (widget&&(settings_editor=(struct Settings_editor *)client_data))
	{
		GT_element_settings_get_streamline_parameters(
			settings_editor->current_settings,&streamline_type,
			&stream_vector_field,&reverse_track,&streamline_length,
			&streamline_width);
		reverse_track = !reverse_track;
		GT_element_settings_set_streamline_parameters(
			settings_editor->current_settings,streamline_type,
			stream_vector_field,reverse_track,streamline_length,
			streamline_width);
		/* inform the client of the change */
		settings_editor_update(settings_editor);
		/* set state of toggle button and grayness of seed_element widget */
		XtVaSetValues(settings_editor->streamline_reverse_button,
			XmNset,reverse_track,NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_streamline_reverse_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_streamline_reverse_button_CB */

static void settings_editor_update_select_mode(Widget widget,
	void *settings_editor_void,void *select_mode_string_void)
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
Callback for change of select_mode.
==============================================================================*/
{
	enum Graphics_select_mode select_mode;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_update_select_mode);
	USE_PARAMETER(widget);
	if (settings_editor=(struct Settings_editor *)settings_editor_void)
	{
		if (STRING_TO_ENUMERATOR(Graphics_select_mode)(
			(char *)select_mode_string_void, &select_mode) &&
			GT_element_settings_set_select_mode(
				settings_editor->current_settings, select_mode))
		{
			/* inform the client of the change */
			settings_editor_update(settings_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update_select_mode.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_update_select_mode */

static void settings_editor_update_streamline_data_type(Widget widget,
	void *settings_editor_void,void *streamline_data_type_string_void)
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
Callback for change of streamline_data_type.
==============================================================================*/
{
	enum Streamline_data_type old_streamline_data_type,streamline_data_type;
	int field_set,spectrum_set;
	struct Computed_field *data_field;
	struct Settings_editor *settings_editor;
	struct Spectrum *spectrum;

	ENTER(settings_editor_update_streamline_type);
	USE_PARAMETER(widget);
	if (settings_editor=(struct Settings_editor *)settings_editor_void)
	{
		if (GT_element_settings_get_data_spectrum_parameters_streamlines(
			settings_editor->current_settings,&old_streamline_data_type,
			&data_field,&spectrum) &&
			STRING_TO_ENUMERATOR(Streamline_data_type)(
				(char *)streamline_data_type_string_void, &streamline_data_type))
		{
			if (streamline_data_type != old_streamline_data_type)
			{
				if (STREAM_FIELD_SCALAR==old_streamline_data_type)
				{
					data_field=(struct Computed_field *)NULL;
				}
				old_streamline_data_type=streamline_data_type;
				if (STREAM_FIELD_SCALAR==streamline_data_type)
				{
					/* get data_field from widget */
					data_field=CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
						settings_editor->data_field_widget);
					if (!data_field)
					{
						streamline_data_type=STREAM_NO_DATA;
					}
				}
				if ((STREAM_NO_DATA != streamline_data_type)&&!spectrum)
				{
					/* get spectrum from widget */
					spectrum=CHOOSE_OBJECT_GET_OBJECT(Spectrum)(
						settings_editor->spectrum_widget);
				}
				GT_element_settings_set_data_spectrum_parameters_streamlines(
					settings_editor->current_settings,streamline_data_type,
					data_field,spectrum);
				if (streamline_data_type != old_streamline_data_type)
				{
					/* update the choose_enumerator for streamline_data_type */
					choose_enumerator_set_string(
						settings_editor->streamline_data_type_widget,
						ENUMERATOR_STRING(Streamline_data_type)(streamline_data_type));
				}
				/* set grayed status of data_field/spectrum widgets */
				field_set=((struct Computed_field *)NULL != data_field);
				spectrum_set=(STREAM_NO_DATA != streamline_data_type);
				XtVaSetValues(settings_editor->data_field_button,
					XmNset,field_set,NULL);
				XtSetSensitive(settings_editor->data_field_widget,field_set);
				XtSetSensitive(settings_editor->spectrum_entry,spectrum_set);
				/* inform the client of the change */
				settings_editor_update(settings_editor);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update_streamline_data_type.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_update_streamline_data_type */

static void settings_editor_update_material(Widget widget,
	void *settings_editor_void,void *material_void)
/*******************************************************************************
LAST MODIFIED : 8 June 1998

DESCRIPTION :
Callback for change of material.
==============================================================================*/
{
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_update_material);
	USE_PARAMETER(widget);
	if (settings_editor=(struct Settings_editor *)settings_editor_void)
	{
		GT_element_settings_set_material(settings_editor->current_settings,
			(struct Graphical_material *)material_void);
		/* inform the client of the change */
		settings_editor_update(settings_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update_material.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_update_material */

static void settings_editor_update_selected_material(Widget widget,
	void *settings_editor_void,void *selected_material_void)
/*******************************************************************************
LAST MODIFIED : 22 February 2000

DESCRIPTION :
Callback for change of selected_material.
==============================================================================*/
{
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_update_selected_material);
	USE_PARAMETER(widget);
	if (settings_editor=(struct Settings_editor *)settings_editor_void)
	{
		GT_element_settings_set_selected_material(settings_editor->current_settings,
			(struct Graphical_material *)selected_material_void);
		/* inform the client of the change */
		settings_editor_update(settings_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update_selected_material.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_update_selected_material */

static void settings_editor_line_width_text_CB(
	Widget widget,XtPointer client_data,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 22 April 2004

DESCRIPTION :
Called when entry is made into the line_width_text field.
==============================================================================*/
{
	char *text_entry,temp_string[50];
	int line_width, new_line_width;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_line_width_text_CB);
	USE_PARAMETER(reason);
	if (widget&&(settings_editor=(struct Settings_editor *)client_data))
	{
		/* must check if have current settings since we can get losingFocus
			 callback after it is cleared */
		if (settings_editor->current_settings)
		{
			line_width = GT_element_settings_get_line_width(settings_editor->current_settings);
			new_line_width = line_width;
			/* Get the text string */
			XtVaGetValues(widget,XmNvalue,&text_entry,NULL);
			if (text_entry)
			{
				sscanf(text_entry,"%d",&new_line_width);
				if (new_line_width != line_width)
				{
					GT_element_settings_set_line_width(
						settings_editor->current_settings, new_line_width);
					/* inform the client of the change */
					settings_editor_update(settings_editor);
				}
				XtFree(text_entry);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"settings_editor_line_width_text_CB.  Missing text");
			}
			/* always restore streamline_width to actual value in use */
			sprintf(temp_string,"%d",new_line_width);
			XtVaSetValues(widget,XmNvalue,temp_string,NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_line_width_text_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_line_width_text_CB */

static void settings_editor_texture_coord_field_button_CB(Widget widget,
	XtPointer settings_editor_void,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 15 June 1999

DESCRIPTION :
Called when the texture coordinate field toggle button value changes.
==============================================================================*/
{
	int field_set;
	struct Computed_field *texture_coord_field,*start_field;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_texture_coord_field_button_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(reason);
	if (settings_editor=(struct Settings_editor *)settings_editor_void)
	{
		texture_coord_field = GT_element_settings_get_texture_coordinate_field
			(settings_editor->current_settings);
		start_field=texture_coord_field;
		if (texture_coord_field)
		{
			texture_coord_field=(struct Computed_field *)NULL;
		}
		else
		{
			/* get data field from widget */
			texture_coord_field=CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
				settings_editor->texture_coord_field_widget);
		}
		GT_element_settings_set_texture_coordinate_field(
			settings_editor->current_settings,texture_coord_field);
		/* set status of data field button and widgets */
		texture_coord_field=GT_element_settings_get_texture_coordinate_field
			(settings_editor->current_settings);
		field_set=((struct Computed_field *)NULL != texture_coord_field);
		XtVaSetValues(settings_editor->texture_coord_field_button,
			XmNset,field_set,NULL);
		XtSetSensitive(settings_editor->texture_coord_field_widget,field_set);
		if (texture_coord_field != start_field)
		{
			/* inform the client of the change */
			settings_editor_update(settings_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_texture_coord_field_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_texture_coord_field_button_CB */

static void settings_editor_update_texture_coord_field(Widget widget,
	void *settings_editor_void,void *texture_coord_field_void)
/*******************************************************************************
LAST MODIFIED : 15 June 1999

DESCRIPTION :
Callback for change of texture_coordinate field.
==============================================================================*/
{
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_update_texture_coord_field);
	USE_PARAMETER(widget);
	if ((settings_editor=(struct Settings_editor *)settings_editor_void)
		&&texture_coord_field_void)
	{
		/* skip messages from chooser if it is grayed out */
		if (XtIsSensitive(settings_editor->texture_coord_field_widget))
		{
			GT_element_settings_set_texture_coordinate_field(
				settings_editor->current_settings,
				(struct Computed_field *)texture_coord_field_void);
			/* inform the client of the change */
			settings_editor_update(settings_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update_texture_coord_field.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_update_texture_coord_field */

static void settings_editor_data_button_CB(Widget widget,
	XtPointer settings_editor_void,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Called when the scalar data field toggle button value changes.
==============================================================================*/
{
	int field_set;
	struct Computed_field *data_field,*start_field;
	struct Settings_editor *settings_editor;
	struct Spectrum *spectrum;

	ENTER(settings_editor_data_button_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(reason);
	if (settings_editor=(struct Settings_editor *)settings_editor_void)
	{
		GT_element_settings_get_data_spectrum_parameters(
			settings_editor->current_settings,&data_field,&spectrum);
		start_field=data_field;
		if (data_field)
		{
			data_field=(struct Computed_field *)NULL;
		}
		else
		{
			/* get data field from widget */
			data_field=CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
				settings_editor->data_field_widget);
			if ((struct Spectrum *)NULL==spectrum)
			{
				spectrum=CHOOSE_OBJECT_GET_OBJECT(Spectrum)(
					settings_editor->spectrum_widget);
			}
		}
		GT_element_settings_set_data_spectrum_parameters(
			settings_editor->current_settings,data_field,spectrum);
		/* set status of data field button and widgets */
		GT_element_settings_get_data_spectrum_parameters(
			settings_editor->current_settings,&data_field,&spectrum);
		field_set=((struct Computed_field *)NULL != data_field);
		XtVaSetValues(settings_editor->data_field_button,
			XmNset,field_set,NULL);
		XtSetSensitive(settings_editor->data_field_widget,field_set);
		XtSetSensitive(settings_editor->spectrum_entry,field_set);
		if (data_field != start_field)
		{
			/* inform the client of the change */
			settings_editor_update(settings_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_data_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_data_button_CB */

static void settings_editor_update_data_field(Widget widget,
	void *settings_editor_void,void *data_field_void)
/*******************************************************************************
LAST MODIFIED : 24 March 1999

DESCRIPTION :
Callback for change of scalar data field.
==============================================================================*/
{
	enum Streamline_data_type streamline_data_type;
	struct Computed_field *data_field;
	struct Settings_editor *settings_editor;
	struct Spectrum *spectrum;

	ENTER(settings_editor_update_data_field);
	USE_PARAMETER(widget);
	if ((settings_editor=(struct Settings_editor *)settings_editor_void)
		&&data_field_void)
	{
		/* skip messages from chooser if it is grayed out */
		if (XtIsSensitive(settings_editor->data_field_widget))
		{
			if (GT_ELEMENT_SETTINGS_STREAMLINES==GT_element_settings_get_settings_type
				(settings_editor->current_settings))
			{
				GT_element_settings_get_data_spectrum_parameters_streamlines(
					settings_editor->current_settings,&streamline_data_type,
					&data_field,&spectrum);
				data_field=(struct Computed_field *)data_field_void;
				GT_element_settings_set_data_spectrum_parameters_streamlines(
					settings_editor->current_settings,streamline_data_type,
					data_field,spectrum);
				/* inform the client of the change */
				settings_editor_update(settings_editor);
			}
			else
			{
				GT_element_settings_get_data_spectrum_parameters(
					settings_editor->current_settings,&data_field,&spectrum);
				data_field=(struct Computed_field *)data_field_void;
				GT_element_settings_set_data_spectrum_parameters(
					settings_editor->current_settings,data_field,spectrum);
				/* inform the client of the change */
				settings_editor_update(settings_editor);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update_data_field.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_update_data_field */

static void settings_editor_update_spectrum(Widget widget,
	void *settings_editor_void,void *spectrum_void)
/*******************************************************************************
LAST MODIFIED : 24 March 1999

DESCRIPTION :
Callback for change of spectrum.
==============================================================================*/
{
	enum Streamline_data_type streamline_data_type;
	struct Computed_field *data_field;
	struct Settings_editor *settings_editor;
	struct Spectrum *spectrum;

	ENTER(settings_editor_update_spectrum);
	USE_PARAMETER(widget);
	if (settings_editor=(struct Settings_editor *)settings_editor_void)
	{
		/* skip messages from chooser if it is grayed out */
		if (XtIsSensitive(settings_editor->spectrum_widget))
		{
			if (GT_ELEMENT_SETTINGS_STREAMLINES==GT_element_settings_get_settings_type
				(settings_editor->current_settings))
			{
				GT_element_settings_get_data_spectrum_parameters_streamlines(
					settings_editor->current_settings,&streamline_data_type,
					&data_field,&spectrum);
				spectrum=(struct Spectrum *)spectrum_void;
				GT_element_settings_set_data_spectrum_parameters_streamlines(
					settings_editor->current_settings,streamline_data_type,
					data_field,spectrum);
				/* inform the client of the change */
				settings_editor_update(settings_editor);
			}
			else
			{
				GT_element_settings_get_data_spectrum_parameters(
					settings_editor->current_settings,&data_field,&spectrum);
				spectrum=(struct Spectrum *)spectrum_void;
				GT_element_settings_set_data_spectrum_parameters(
					settings_editor->current_settings,data_field,spectrum);
				/* inform the client of the change */
				settings_editor_update(settings_editor);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_update_spectrum.  Invalid argument(s)");
	}
	LEAVE;
} /* settings_editor_update_spectrum */

/*
Global functions
----------------
*/
Widget create_settings_editor_widget(Widget *settings_editor_widget,
	Widget parent,struct GT_element_settings *settings,
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct LIST(GT_object) *glyph_list,struct MANAGER(Spectrum) *spectrum_manager,
	struct MANAGER(VT_volume_texture) *volume_texture_manager,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Creates a settings_editor widget.
==============================================================================*/
{
	char **valid_strings;
	int init_widgets,number_of_valid_strings;
	MrmType settings_editor_dialog_class;
	struct FE_region *fe_region;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Settings_editor *settings_editor=NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"seted_destroy_CB",(XtPointer)
			settings_editor_destroy_CB},
		{"seted_id_main_scroll",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,main_scroll)},
		{"seted_id_main_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,main_form)},
		{"seted_id_name_text",(XtPointer)
		   DIALOG_IDENTIFY(settings_editor,name_text)},
		{"seted_id_coordinate_btn",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,coordinate_button)},
		{"seted_id_coordinate_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,coordinate_field_form)},
		{"seted_id_use_elem_type_entry",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,use_element_type_entry)},
		{"seted_id_use_elem_type_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,use_element_type_form)},
		{"seted_id_exterior_face_entry",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,exterior_face_entry)},
		{"seted_id_exterior_btn",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,exterior_button)},
		{"seted_id_face_btn",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,face_button)},
		{"seted_id_face_opt",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,face_option)},
		{"seted_id_face_menu",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,face_menu)},
		{"seted_id_radius_entry",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,radius_entry)},
		{"seted_id_constant_radius_text",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,constant_radius_text)},
		{"seted_id_radius_scalar_btn",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,radius_scalar_field_button)},
		{"seted_id_radius_scalar_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,radius_scalar_field_form)},
		{"seted_id_radius_sfactor_entry",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,radius_scale_factor_entry)},
		{"seted_id_radius_sfactor_text",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,radius_scale_factor_text)},
		{"seted_id_iso_surface_entry",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,iso_surface_entry)},
		{"seted_id_iso_scalar_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,iso_scalar_field_form)},
		{"seted_id_iso_value_text",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,iso_value_text)},
		{"seted_id_discretization_entry",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,discretization_entry)},
		{"seted_id_discretization_text",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,discretization_text)},
		{"seted_id_native_disc_entry",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,native_discretization_entry)},
		{"seted_id_native_disc_btn",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,native_discretization_button)},
		{"seted_id_native_disc_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,native_discretization_field_form)},
		{"seted_id_density_field_entry",(XtPointer)
			DIALOG_IDENTIFY(settings_editor, xi_point_density_field_entry)},
		{"seted_id_density_field_btn",(XtPointer)
			DIALOG_IDENTIFY(settings_editor, xi_point_density_field_button)},
		{"seted_id_density_field_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor, xi_point_density_field_form)},
		{"seted_id_xi_disc_mode_entry",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,xi_discretization_mode_entry)},
		{"seted_id_xi_disc_mode_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,xi_discretization_mode_form)},
		{"seted_id_glyph_group_entry",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,glyph_group_entry)},
		{"seted_id_glyph_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,glyph_form)},
		{"seted_id_glyph_centre_text",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,glyph_centre_text)},
		{"seted_id_glyph_scaling_entry",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,glyph_scaling_mode_entry)},
		{"seted_id_glyph_scaling_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,glyph_scaling_mode_form)},
		{"seted_id_glyph_size_text",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,glyph_size_text)},
		{"seted_id_glyph_orient_btn",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,glyph_orientation_scale_button)},
		{"seted_id_glyph_orient_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,glyph_orientation_scale_field_form)},
		{"seted_id_glyph_sfactors_entry",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,glyph_scale_factors_entry)},
		{"seted_id_glyph_sfactors_text",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,glyph_scale_factors_text)},
		{"seted_id_glyph_variable_entry",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,glyph_variable_scale_entry)},
		{"seted_id_glyph_variable_btn",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,glyph_variable_scale_button)},
		{"seted_id_glyph_variable_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,glyph_variable_scale_field_form)},
		{"seted_id_label_field_entry",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,label_field_entry)},
		{"seted_id_label_field_btn",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,label_field_button)},
		{"seted_id_label_field_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,label_field_form)},
		{"seted_id_volume_texture_entry",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,volume_texture_entry)},
		{"seted_id_volume_texture_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,volume_texture_form)},
		{"seted_id_seed_element_entry",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,seed_element_entry)},
		{"seted_id_seed_element_btn",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,seed_element_button)},
		{"seted_id_seed_element_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,seed_element_form)},
		{"seted_id_seed_xi_entry",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,seed_xi_entry)},
		{"seted_id_seed_xi_text",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,seed_xi_text)},
		{"seted_id_streamline_entry",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,streamline_entry)},
		{"seted_id_strline_type_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,streamline_type_form)},
		{"seted_id_strline_length_text",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,streamline_length_text)},
		{"seted_id_strline_width_text",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,streamline_width_text)},
		{"seted_id_strline_vector_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,stream_vector_field_form)},
		{"seted_id_strline_reverse_btn",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,streamline_reverse_button)},
		{"seted_id_select_mode_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,select_mode_form)},
		{"seted_id_material_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,material_form)},
		{"seted_id_texture_coord_entry",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,texture_coord_field_entry)},
		{"seted_id_texture_coord_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,texture_coord_field_form)},
		{"seted_id_texture_coord_btn",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,texture_coord_field_button)},
		{"seted_id_render_type_entry",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,render_type_entry)},
		{"seted_id_render_type_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,render_type_form)},
		{"seted_id_strline_datatype_entry",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,streamline_data_type_entry)},
		{"seted_id_strline_datatype_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,streamline_data_type_form)},
		{"seted_id_data_field_btn",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,data_field_button)},
		{"seted_id_data_field_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,data_field_form)},
		{"seted_id_spectrum_entry",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,spectrum_entry)},
		{"seted_id_spectrum_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,spectrum_form)},
		{"seted_id_selected_mat_form",(XtPointer)
			DIALOG_IDENTIFY(settings_editor,selected_material_form)},
		{"seted_id_line_width_entry",(XtPointer)
		   DIALOG_IDENTIFY(settings_editor,line_width_entry)},
		{"seted_id_line_width_text",(XtPointer)
		   DIALOG_IDENTIFY(settings_editor,line_width_text)},
		{"seted_coordinate_btn_CB",(XtPointer)
			settings_editor_coordinate_button_CB},
		{"seted_name_text_CB",(XtPointer)
			settings_editor_name_text_CB},
		{"seted_exterior_btn_CB",(XtPointer)
			settings_editor_exterior_button_CB},
		{"seted_face_btn_CB",(XtPointer)
			settings_editor_face_button_CB},
		{"seted_face_menu_CB",(XtPointer)
			settings_editor_face_menu_CB},
		{"seted_constant_radius_text_CB",(XtPointer)
			settings_editor_constant_radius_text_CB},
		{"seted_radius_scalar_btn_CB",(XtPointer)
			settings_editor_radius_scalar_field_button_CB},
		{"seted_radius_sfactor_text_CB",(XtPointer)
			settings_editor_radius_scale_factor_text_CB},
		{"seted_iso_value_text_CB",(XtPointer)
			settings_editor_iso_value_text_CB},
		{"seted_discretization_text_CB",(XtPointer)
			settings_editor_discretization_text_CB},
		{"seted_native_disc_btn_CB",(XtPointer)
			settings_editor_native_discretization_button_CB},
		{"seted_glyph_size_text_CB",(XtPointer)
			settings_editor_glyph_size_text_CB},
		{"seted_glyph_centre_text_CB",(XtPointer)
			settings_editor_glyph_centre_text_CB},
		{"seted_glyph_orient_btn_CB",(XtPointer)
			settings_editor_glyph_orientation_scale_button_CB},
		{"seted_glyph_sfactors_text_CB",(XtPointer)
			settings_editor_glyph_scale_factors_text_CB},
		{"seted_glyph_variable_btn_CB",(XtPointer)
			settings_editor_glyph_variable_scale_button_CB},
		{"seted_label_field_btn_CB",(XtPointer)
			settings_editor_label_button_CB},
		{"seted_seed_element_btn_CB",(XtPointer)
			settings_editor_seed_element_button_CB},
		{"seted_seed_xi_text_CB",(XtPointer)
			settings_editor_seed_xi_text_CB},
		{"seted_strline_length_text_CB",(XtPointer)
			settings_editor_streamline_length_text_CB},
		{"seted_strline_width_text_CB",(XtPointer)
			settings_editor_streamline_width_text_CB},
		{"seted_strline_reverse_btn_CB",(XtPointer)
			settings_editor_streamline_reverse_button_CB},
		{"seted_strline_length_text_CB",(XtPointer)
			settings_editor_streamline_length_text_CB},
		{"seted_line_width_text_CB",(XtPointer)
			settings_editor_line_width_text_CB},
		{"seted_texture_coord_btn_CB",(XtPointer)
			settings_editor_texture_coord_field_button_CB},
		{"seted_data_field_btn_CB",(XtPointer)
		   settings_editor_data_button_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"seted_structure",(XtPointer)NULL}
	};
	Widget return_widget;

	ENTER(create_settings_editor_widget);
	return_widget=(Widget)NULL;
	if (settings_editor_widget && parent && computed_field_package &&
		(computed_field_manager=Computed_field_package_get_computed_field_manager(
			computed_field_package)) && root_region &&
		(fe_region = Cmiss_region_get_FE_region(root_region)) &&
		graphical_material_manager && glyph_list && spectrum_manager &&
		volume_texture_manager && user_interface)
	{
		if (MrmOpenHierarchy_base64_string(settings_editor_uidh,
			&settings_editor_hierarchy,&settings_editor_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(settings_editor,struct Settings_editor,1))
			{
				/* initialise the structure */
				settings_editor->current_settings=
					(struct GT_element_settings *)NULL;
				settings_editor->root_region = root_region;
				settings_editor->graphical_material_manager=
					graphical_material_manager;
				settings_editor->glyph_list=glyph_list;
				settings_editor->spectrum_manager=spectrum_manager;
				settings_editor->volume_texture_manager=volume_texture_manager;
				settings_editor->user_interface=user_interface;
				settings_editor->widget_parent=parent;
				settings_editor->widget_address=settings_editor_widget;
				settings_editor->widget=(Widget)NULL;
				/* clear geometry settings widgets */
				settings_editor->main_scroll=(Widget)NULL;
				settings_editor->main_form=(Widget)NULL;
				settings_editor->name_text=(Widget)NULL;
				settings_editor->coordinate_button=(Widget)NULL;
				settings_editor->coordinate_field_form=(Widget)NULL;
				settings_editor->coordinate_field_widget=(Widget)NULL;
				settings_editor->use_element_type_entry=(Widget)NULL;
				settings_editor->use_element_type_form=(Widget)NULL;
				settings_editor->use_element_type_widget=(Widget)NULL;
				settings_editor->exterior_face_entry=(Widget)NULL;
				settings_editor->exterior_button=(Widget)NULL;
				settings_editor->face_button=(Widget)NULL;
				settings_editor->face_option=(Widget)NULL;
				settings_editor->face_menu=(Widget)NULL;
				settings_editor->radius_entry=(Widget)NULL;
				settings_editor->constant_radius_text=(Widget)NULL;
				settings_editor->radius_scalar_field_button=(Widget)NULL;
				settings_editor->radius_scalar_field_form=(Widget)NULL;
				settings_editor->radius_scalar_field_widget=(Widget)NULL;
				settings_editor->radius_scale_factor_entry=(Widget)NULL;
				settings_editor->radius_scale_factor_text=(Widget)NULL;
				settings_editor->iso_surface_entry=(Widget)NULL;
				settings_editor->iso_scalar_field_form=(Widget)NULL;
				settings_editor->iso_scalar_field_widget=(Widget)NULL;
				settings_editor->iso_value_text=(Widget)NULL;
				settings_editor->discretization_entry=(Widget)NULL;
				settings_editor->discretization_text=(Widget)NULL;
				settings_editor->native_discretization_entry=(Widget)NULL;
				settings_editor->native_discretization_button=(Widget)NULL;
				settings_editor->native_discretization_field_form=(Widget)NULL;
				settings_editor->native_discretization_field_widget=(Widget)NULL;
				settings_editor->xi_point_density_field_entry=(Widget)NULL;
				settings_editor->xi_point_density_field_button=(Widget)NULL;
				settings_editor->xi_point_density_field_form=(Widget)NULL;
				settings_editor->xi_point_density_field_widget=(Widget)NULL;
				settings_editor->xi_discretization_mode_entry=(Widget)NULL;
				settings_editor->xi_discretization_mode_form=(Widget)NULL;
				settings_editor->xi_discretization_mode_widget=(Widget)NULL;
				settings_editor->glyph_group_entry=(Widget)NULL;
				settings_editor->glyph_form=(Widget)NULL;
				settings_editor->glyph_widget=(Widget)NULL;
				settings_editor->glyph_centre_text=(Widget)NULL;
				settings_editor->glyph_scaling_mode_entry = (Widget)NULL;
				settings_editor->glyph_scaling_mode_form = (Widget)NULL;
				settings_editor->glyph_scaling_mode_widget = (Widget)NULL;
				settings_editor->glyph_size_text=(Widget)NULL;
				settings_editor->glyph_orientation_scale_button=(Widget)NULL;
				settings_editor->glyph_orientation_scale_field_form=(Widget)NULL;
				settings_editor->glyph_orientation_scale_field_widget=(Widget)NULL;
				settings_editor->glyph_scale_factors_entry=(Widget)NULL;
				settings_editor->glyph_scale_factors_text=(Widget)NULL;
				settings_editor->glyph_variable_scale_entry = (Widget)NULL;
				settings_editor->glyph_variable_scale_button = (Widget)NULL;
				settings_editor->glyph_variable_scale_field_form = (Widget)NULL;
				settings_editor->glyph_variable_scale_field_widget = (Widget)NULL;
				settings_editor->label_field_entry=(Widget)NULL;
				settings_editor->label_field_button=(Widget)NULL;
				settings_editor->label_field_form=(Widget)NULL;
				settings_editor->label_field_widget=(Widget)NULL;
				settings_editor->volume_texture_entry=(Widget)NULL;
				settings_editor->volume_texture_form=(Widget)NULL;
				settings_editor->volume_texture_widget=(Widget)NULL;
				settings_editor->seed_element_entry=(Widget)NULL;
				settings_editor->seed_element_button=(Widget)NULL;
				settings_editor->seed_element_form=(Widget)NULL;
				settings_editor->seed_element_widget=(Widget)NULL;
				settings_editor->seed_xi_entry=(Widget)NULL;
				settings_editor->seed_xi_text=(Widget)NULL;
				settings_editor->streamline_entry=(Widget)NULL;
				settings_editor->streamline_type_form=(Widget)NULL;
				settings_editor->streamline_type_widget=(Widget)NULL;
				settings_editor->streamline_length_text=(Widget)NULL;
				settings_editor->streamline_width_text=(Widget)NULL;
				settings_editor->stream_vector_field_form=(Widget)NULL;
				settings_editor->stream_vector_field_widget=(Widget)NULL;
				settings_editor->streamline_reverse_button=(Widget)NULL;
				settings_editor->select_mode_form=(Widget)NULL;
				settings_editor->select_mode_widget=(Widget)NULL;
				/* clear appearance settings widgets */
				settings_editor->material_form=(Widget)NULL;
				settings_editor->material_widget=(Widget)NULL;
				settings_editor->line_width_entry=(Widget)NULL;
				settings_editor->line_width_text=(Widget)NULL;
				settings_editor->texture_coord_field_entry=(Widget)NULL;
				settings_editor->texture_coord_field_form=(Widget)NULL;
				settings_editor->texture_coord_field_button=(Widget)NULL;
				settings_editor->texture_coord_field_widget=(Widget)NULL;
				settings_editor->streamline_data_type_entry=(Widget)NULL;
				settings_editor->streamline_data_type_form=(Widget)NULL;
				settings_editor->streamline_data_type_widget=(Widget)NULL;
				settings_editor->data_field_button=(Widget)NULL;
				settings_editor->data_field_form=(Widget)NULL;
				settings_editor->data_field_widget=(Widget)NULL;
				settings_editor->render_type_entry=(Widget)NULL;
				settings_editor->render_type_form=(Widget)NULL;
				settings_editor->selected_material_form=(Widget)NULL;
				settings_editor->selected_material_widget=(Widget)NULL;
				settings_editor->spectrum_entry=(Widget)NULL;
				settings_editor->spectrum_form=(Widget)NULL;
				settings_editor->spectrum_widget=(Widget)NULL;
				settings_editor->update_callback.procedure=(Callback_procedure *)NULL;
				settings_editor->update_callback.data=(void *)NULL;
				/* register the callbacks */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
					settings_editor_hierarchy,callback_list,XtNumber(callback_list)))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)settings_editor;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(settings_editor_hierarchy,
						identifier_list,XtNumber(identifier_list)))
					{
						/* fetch settings editor widget */
						if (MrmSUCCESS==MrmFetchWidget(settings_editor_hierarchy,
							"settings_editor_widget",settings_editor->widget_parent,
							&(settings_editor->widget),
							&settings_editor_dialog_class))
						{
							init_widgets=1;
							/* create the subwidgets with default values */
							valid_strings = ENUMERATOR_GET_VALID_STRINGS(Use_element_type)(
								&number_of_valid_strings,
								(ENUMERATOR_CONDITIONAL_FUNCTION(Use_element_type) *)NULL,
								(void *)NULL);
							if (!(settings_editor->use_element_type_widget=
								create_choose_enumerator_widget(
								settings_editor->use_element_type_form,
								number_of_valid_strings,valid_strings,
								ENUMERATOR_STRING(Use_element_type)(USE_ELEMENTS),
								user_interface)))
							{
								init_widgets=0;
							}
							DEALLOCATE(valid_strings);
							if (!(settings_editor->coordinate_field_widget=
								CREATE_CHOOSE_OBJECT_WIDGET(Computed_field)(
								settings_editor->coordinate_field_form,
								(struct Computed_field *)NULL,computed_field_manager,
								Computed_field_has_up_to_3_numerical_components,(void *)NULL,
								user_interface)))
							{
								init_widgets=0;
							}
							if (!(settings_editor->radius_scalar_field_widget=
								CREATE_CHOOSE_OBJECT_WIDGET(Computed_field)(
								settings_editor->radius_scalar_field_form,
								(struct Computed_field *)NULL,computed_field_manager,
								Computed_field_is_scalar, (void *)NULL, user_interface)))
							{
								init_widgets=0;
							}
							if (!(settings_editor->iso_scalar_field_widget=
								CREATE_CHOOSE_OBJECT_WIDGET(Computed_field)(
								settings_editor->iso_scalar_field_form,
								(struct Computed_field *)NULL,computed_field_manager,
								Computed_field_is_scalar, (void *)NULL, user_interface)))
							{
								init_widgets=0;
							}
							if (!(settings_editor->glyph_widget=
								CREATE_CHOOSE_OBJECT_LIST_WIDGET(GT_object)(
								settings_editor->glyph_form,
								(struct GT_object *)NULL,settings_editor->glyph_list,
								(LIST_CONDITIONAL_FUNCTION(GT_object) *)NULL, user_interface)))
							{
								init_widgets=0;
							}
							valid_strings = ENUMERATOR_GET_VALID_STRINGS(Glyph_scaling_mode)(
								&number_of_valid_strings,
								(ENUMERATOR_CONDITIONAL_FUNCTION(Glyph_scaling_mode) *)NULL,
								(void *)NULL);
							if (!(settings_editor->glyph_scaling_mode_widget=
								create_choose_enumerator_widget(
									settings_editor->glyph_scaling_mode_form,
									number_of_valid_strings,valid_strings,
									ENUMERATOR_STRING(Glyph_scaling_mode)(
										GLYPH_SCALING_GENERAL), user_interface)))
							{
								init_widgets=0;
							}
							DEALLOCATE(valid_strings);
							if (!(settings_editor->glyph_orientation_scale_field_widget=
								CREATE_CHOOSE_OBJECT_WIDGET(Computed_field)(
								settings_editor->glyph_orientation_scale_field_form,
								(struct Computed_field *)NULL,computed_field_manager,
								Computed_field_is_orientation_scale_capable, (void *)NULL,
								user_interface)))
							{
								init_widgets=0;
							}
							if (!(settings_editor->glyph_variable_scale_field_widget=
								CREATE_CHOOSE_OBJECT_WIDGET(Computed_field)(
									settings_editor->glyph_variable_scale_field_form,
									(struct Computed_field *)NULL, computed_field_manager,
									Computed_field_has_up_to_3_numerical_components,
									(void *)NULL, user_interface)))
							{
								init_widgets=0;
							}
							if (!(settings_editor->label_field_widget=
								CREATE_CHOOSE_OBJECT_WIDGET(Computed_field)(
								settings_editor->label_field_form,
								(struct Computed_field *)NULL,computed_field_manager,
								(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL,
								(void *)NULL, user_interface)))
							{
								init_widgets=0;
							}
							if (!(settings_editor->native_discretization_field_widget=
								CREATE_FE_REGION_CHOOSE_OBJECT_WIDGET(FE_field)(
								settings_editor->native_discretization_field_form,
								fe_region, (struct FE_field *)NULL,
								(LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL, (void *)NULL,
								user_interface)))
							{
								init_widgets=0;
							}
							if (!(settings_editor->xi_point_density_field_widget =
								CREATE_CHOOSE_OBJECT_WIDGET(Computed_field)(
								settings_editor->xi_point_density_field_form,
								(struct Computed_field *)NULL, computed_field_manager,
								Computed_field_is_scalar, (void *)NULL, user_interface)))
							{
								init_widgets = 0;
							}
							valid_strings =
								ENUMERATOR_GET_VALID_STRINGS(Xi_discretization_mode)(
									&number_of_valid_strings, (ENUMERATOR_CONDITIONAL_FUNCTION(
										Xi_discretization_mode) *)NULL, (void *)NULL);
							if (!(settings_editor->xi_discretization_mode_widget=
								create_choose_enumerator_widget(
								settings_editor->xi_discretization_mode_form,
								number_of_valid_strings,valid_strings,
								ENUMERATOR_STRING(Xi_discretization_mode)(
									XI_DISCRETIZATION_CELL_CENTRES), user_interface)))
							{
								init_widgets=0;
							}
							DEALLOCATE(valid_strings);
							if (!(settings_editor->volume_texture_widget=
								CREATE_CHOOSE_OBJECT_WIDGET(VT_volume_texture)(
								settings_editor->volume_texture_form,
								(struct VT_volume_texture *)NULL,volume_texture_manager,
								(MANAGER_CONDITIONAL_FUNCTION(VT_volume_texture) *)NULL,
								(void *)NULL, user_interface)))
							{
								init_widgets=0;
							}
							if (!(settings_editor->seed_element_widget=
								CREATE_TEXT_CHOOSE_FROM_FE_REGION_WIDGET(FE_element)(
								settings_editor->seed_element_form,
								(struct FE_element *)NULL,fe_region,
								FE_element_is_top_level,(void *)NULL,
								FE_element_to_element_string,
								FE_region_element_string_to_FE_element)))
							{
								init_widgets=0;
							}
							valid_strings = ENUMERATOR_GET_VALID_STRINGS(Streamline_type)(
								&number_of_valid_strings,
								(ENUMERATOR_CONDITIONAL_FUNCTION(Streamline_type) *)NULL,
								(void *)NULL);
							if (!(settings_editor->streamline_type_widget=
								create_choose_enumerator_widget(
								settings_editor->streamline_type_form,
								number_of_valid_strings,valid_strings,
								ENUMERATOR_STRING(Streamline_type)(STREAM_LINE),
								user_interface)))
							{
								init_widgets=0;
							}
							DEALLOCATE(valid_strings);
							if (!(settings_editor->stream_vector_field_widget=
								CREATE_CHOOSE_OBJECT_WIDGET(Computed_field)(
								settings_editor->stream_vector_field_form,
								(struct Computed_field *)NULL,computed_field_manager,
								Computed_field_is_stream_vector_capable, (void *)NULL,
								user_interface)))
							{
								init_widgets=0;
							}
							valid_strings =
								ENUMERATOR_GET_VALID_STRINGS(Graphics_select_mode)(
									&number_of_valid_strings,
									(ENUMERATOR_CONDITIONAL_FUNCTION(Graphics_select_mode) *)NULL,
									(void *)NULL);
							if (!(settings_editor->select_mode_widget=
								create_choose_enumerator_widget(
									settings_editor->select_mode_form,
									number_of_valid_strings,valid_strings,
									ENUMERATOR_STRING(Graphics_select_mode)(GRAPHICS_NO_SELECT),
									user_interface)))
							{
								init_widgets=0;
							}
							DEALLOCATE(valid_strings);
							valid_strings =
								ENUMERATOR_GET_VALID_STRINGS(Streamline_data_type)(
									&number_of_valid_strings,
									(ENUMERATOR_CONDITIONAL_FUNCTION(Streamline_data_type) *)NULL,
									(void *)NULL);
							if (!(settings_editor->streamline_data_type_widget=
								create_choose_enumerator_widget(
								settings_editor->streamline_data_type_form,
								number_of_valid_strings,valid_strings,
								ENUMERATOR_STRING(Streamline_data_type)(STREAM_FIELD_SCALAR),
								user_interface)))
							{
								init_widgets=0;
							}
							DEALLOCATE(valid_strings);
							if (!(settings_editor->material_widget=
								CREATE_CHOOSE_OBJECT_WIDGET(Graphical_material)(
								settings_editor->material_form,
								(struct Graphical_material *)NULL,
								settings_editor->graphical_material_manager,
								(MANAGER_CONDITIONAL_FUNCTION(Graphical_material) *)NULL,
								(void *)NULL, user_interface)))
							{
								init_widgets=0;
							}
							if (!(settings_editor->texture_coord_field_widget=
								CREATE_CHOOSE_OBJECT_WIDGET(Computed_field)(
								settings_editor->texture_coord_field_form,
								(struct Computed_field *)NULL,computed_field_manager,
								Computed_field_has_up_to_3_numerical_components, (void *)NULL,
								user_interface)))
							{
								init_widgets=0;
							}
							if (!(settings_editor->data_field_widget=
								CREATE_CHOOSE_OBJECT_WIDGET(Computed_field)(
								settings_editor->data_field_form,
								(struct Computed_field *)NULL,computed_field_manager,
								Computed_field_has_numerical_components, (void *)NULL,
								user_interface)))
							{
								init_widgets=0;
							}
							if (!(settings_editor->spectrum_widget=
								CREATE_CHOOSE_OBJECT_WIDGET(Spectrum)(
								settings_editor->spectrum_form,
								(struct Spectrum *)NULL,spectrum_manager,
								(MANAGER_CONDITIONAL_FUNCTION(Spectrum) *)NULL, (void *)NULL,
								user_interface)))
							{
								init_widgets=0;
							}
							if (!(settings_editor->selected_material_widget=
								CREATE_CHOOSE_OBJECT_WIDGET(Graphical_material)(
								settings_editor->selected_material_form,
								(struct Graphical_material *)NULL,
								settings_editor->graphical_material_manager,
								(MANAGER_CONDITIONAL_FUNCTION(Graphical_material) *)NULL,
								(void *)NULL, user_interface)))
							{
								init_widgets=0;
							}
							valid_strings = ENUMERATOR_GET_VALID_STRINGS(Render_type)(
								&number_of_valid_strings,
								(ENUMERATOR_CONDITIONAL_FUNCTION(Render_type) *)NULL,
								(void *)NULL);
							if (!(settings_editor->render_type_widget=
								create_choose_enumerator_widget(
									settings_editor->render_type_form,
									number_of_valid_strings, valid_strings,
									ENUMERATOR_STRING(Render_type)(RENDER_TYPE_SHADED),
									user_interface)))
							{
								init_widgets = 0;
							}
							DEALLOCATE(valid_strings);
							if (init_widgets)
							{
								Pixel pixel;
								Widget clip_window = (Widget)NULL;

								/* copy background from main_form into main_scroll */
								XtVaGetValues(settings_editor->main_form,
									XmNbackground, &pixel, NULL);
								XtVaGetValues(settings_editor->main_scroll,
									XmNclipWindow, &clip_window, NULL);
								XtVaSetValues(clip_window, XmNbackground, pixel, NULL);

								XtUnmanageChild(settings_editor->glyph_scaling_mode_entry);
								if (settings)
								{
									settings_editor_set_settings(
										settings_editor->widget, settings);
								}
								return_widget=settings_editor->widget;
							}
							else
							{
								XtDestroyWidget(settings_editor->widget);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_settings_editor_widget.  "
								"Could not fetch settings_editor widget");
							DEALLOCATE(settings_editor);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_settings_editor_widget.  "
							"Could not register identifiers");
						DEALLOCATE(settings_editor);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_settings_editor_widget.  "
						"Could not register callbacks");
					DEALLOCATE(settings_editor);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_settings_editor_widget.  "
					"Could not allocate settings_editor widget structure");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_settings_editor_widget.  Could not open hierarchy");
		}
		*settings_editor_widget=return_widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_settings_editor_widget.  Invalid argument(s)");
	}
	LEAVE;

	return (return_widget);
} /* create_settings_editor_widget */

struct Callback_data *settings_editor_get_callback(
	Widget settings_editor_widget)
/*******************************************************************************
LAST MODIFIED : 4 November 1997

DESCRIPTION :
Returns a pointer to the update_callback item of the
settings_editor_widget.
==============================================================================*/
{
	struct Callback_data *return_address;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_get_callback);
	if (settings_editor_widget)
	{
		/* Get the pointer to the data for the choose_settings dialog */
		XtVaGetValues(settings_editor_widget,XmNuserData,
			&settings_editor,NULL);
		if (settings_editor)
		{
			return_address=&(settings_editor->update_callback);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"settings_editor_get_callback.  Missing widget data");
			return_address=(struct Callback_data *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_get_callback.  Missing widget");
		return_address=(struct Callback_data *)NULL;
	}
	LEAVE;

	return (return_address);
} /* settings_editor_get_callback */

int settings_editor_set_callback(Widget settings_editor_widget,
	struct Callback_data *new_callback)
/*******************************************************************************
LAST MODIFIED : 4 November 1997

DESCRIPTION :
Changes the callback function for the settings_editor_widget, which will
be called when the chosen settings changes in any way.
==============================================================================*/
{
	int return_code;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_set_callback);
	if (settings_editor_widget&&new_callback)
	{
		/* Get the pointer to the data for the choose_settings dialog */
		XtVaGetValues(settings_editor_widget,XmNuserData,
			&settings_editor,NULL);
		if (settings_editor)
		{
			settings_editor->update_callback.procedure=new_callback->procedure;
			settings_editor->update_callback.data=new_callback->data;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"settings_editor_set_callback.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_set_callback.  Missing widget");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* settings_editor_set_callback */

struct GT_element_settings *settings_editor_get_settings(
	Widget settings_editor_widget)
/*******************************************************************************
LAST MODIFIED : 4 November 1997

DESCRIPTION :
Returns the currently chosen settings.
==============================================================================*/
{
	struct GT_element_settings *return_address;
	struct Settings_editor *settings_editor;

	ENTER(settings_editor_get_settings);
	if (settings_editor_widget)
	{
		/* Get the pointer to the data for the choose_settings dialog */
		XtVaGetValues(settings_editor_widget,XmNuserData,
			&settings_editor,NULL);
		if (settings_editor)
		{
			return_address=settings_editor->current_settings;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"settings_editor_get_settings.  Missing widget data");
			return_address=(struct GT_element_settings *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_get_settings.  Missing widget");
		return_address=(struct GT_element_settings *)NULL;
	}
	LEAVE;

	return (return_address);
} /* settings_editor_get_settings */

int settings_editor_set_settings(Widget settings_editor_widget,
	struct GT_element_settings *new_settings)
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Changes the currently chosen settings.
==============================================================================*/
{
	char *name, temp_string[50];
	double iso_value;
	enum Graphics_select_mode select_mode;
	enum GT_element_settings_type settings_type;
	enum Streamline_type streamline_type;
	enum Streamline_data_type streamline_data_type;
	enum Xi_discretization_mode xi_discretization_mode;
	float constant_radius,scale_factor,streamline_length,
		streamline_width;
	int field_set,line_width,return_code,reverse_track;
	struct Callback_data callback;
	struct Computed_field *coordinate_field, *data_field, *iso_scalar_field,
		*label_field, *radius_scalar_field, *stream_vector_field,
		*texture_coord_field, *xi_point_density_field;
	struct Element_discretization discretization;
	struct FE_element *seed_element;
	struct FE_field *native_discretization_field;
	struct Settings_editor *settings_editor;
	struct Spectrum *spectrum;
	Triple seed_xi;

	ENTER(settings_editor_set_settings);
	if (settings_editor_widget)
	{
		/* Get the pointer to the data for the choose_settings dialog */
		XtVaGetValues(settings_editor_widget,XmNuserData,
			&settings_editor,NULL);
		if (settings_editor)
		{
			return_code=1;
			if (new_settings != settings_editor->current_settings)
			{
				/* destroy current settings if any */
				if (settings_editor->current_settings)
				{
					DESTROY(GT_element_settings)
						(&(settings_editor->current_settings));
				}
				if (new_settings)
				{
					/* make current settings a copy of new settings */
					settings_type=GT_element_settings_get_settings_type(new_settings);
					if (settings_editor->current_settings=
						CREATE(GT_element_settings)(settings_type))
					{
						if (GT_element_settings_copy_without_graphics_object(
							settings_editor->current_settings,new_settings))
						{
							XtManageChild(settings_editor->widget);

							GET_NAME(GT_element_settings)(new_settings, &name);
							XtVaSetValues(settings_editor->name_text,
								XmNvalue, name,NULL);
							DEALLOCATE(name);

							/* set values of geometry settings widgets */
							if (coordinate_field=
								GT_element_settings_get_coordinate_field(new_settings))
							{
								CHOOSE_OBJECT_SET_OBJECT(Computed_field)(
									settings_editor->coordinate_field_widget,coordinate_field);
								field_set=True;
							}
							else
							{
								field_set=False;
							}
							XtVaSetValues(settings_editor->coordinate_button,
								XmNset,field_set,NULL);
							XtSetSensitive(settings_editor->coordinate_field_widget,
								field_set);
							/* turn on callbacks */
							callback.data=(void *)settings_editor;
							callback.procedure=settings_editor_update_coordinate_field;
							CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
								settings_editor->coordinate_field_widget,&callback);

							settings_editor_display_dimension_specific(settings_editor);

							/* cylinders */
							if ((GT_ELEMENT_SETTINGS_CYLINDERS==settings_type)&&
								GT_element_settings_get_radius_parameters(new_settings,
									&constant_radius,&scale_factor,&radius_scalar_field))
							{
								/* constant radius */
								sprintf(temp_string,"%g",constant_radius);
								XtVaSetValues(settings_editor->constant_radius_text,
									XmNvalue,temp_string,NULL);
								/* variable radius */
								field_set=((struct Computed_field *)NULL!=radius_scalar_field);
								XtVaSetValues(settings_editor->radius_scalar_field_button,
									XmNset,field_set,NULL);
								if (field_set)
								{
									CHOOSE_OBJECT_SET_OBJECT(Computed_field)(
										settings_editor->radius_scalar_field_widget,
										radius_scalar_field);
								}
								sprintf(temp_string,"%g",scale_factor);
								XtVaSetValues(settings_editor->radius_scale_factor_text,
									XmNvalue,temp_string,NULL);
								/* set (un)grayed status of variable radius widgets */
								XtSetSensitive(settings_editor->radius_scalar_field_widget,
									field_set);
								XtSetSensitive(settings_editor->radius_scale_factor_entry,
									field_set);
								/* turn on callbacks */
								callback.data=(void *)settings_editor;
								callback.procedure=settings_editor_update_radius_scalar_field;
								CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
									settings_editor->radius_scalar_field_widget,&callback);
								XtManageChild(settings_editor->radius_entry);
							}
							else
							{
								/* turn off callbacks */
								callback.procedure=(Callback_procedure *)NULL;
								callback.data=(void *)NULL;
								CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
									settings_editor->radius_scalar_field_widget,&callback);
								XtUnmanageChild(settings_editor->radius_entry);
							}

							/* iso_surfaces */
							if ((GT_ELEMENT_SETTINGS_ISO_SURFACES==settings_type)&&
								GT_element_settings_get_iso_surface_parameters(new_settings,
									&iso_scalar_field,&iso_value)&&iso_scalar_field)
							{
								CHOOSE_OBJECT_SET_OBJECT(Computed_field)(
									settings_editor->iso_scalar_field_widget,
									iso_scalar_field);
								sprintf(temp_string,"%g",iso_value);
								XtVaSetValues(settings_editor->iso_value_text,XmNvalue,
									temp_string,NULL);
								/* turn on callbacks */
								callback.data=(void *)settings_editor;
								callback.procedure=settings_editor_update_iso_scalar_field;
								CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
									settings_editor->iso_scalar_field_widget,&callback);
								XtManageChild(settings_editor->iso_surface_entry);
							}
							else
							{
								/* turn off callbacks */
								callback.procedure=(Callback_procedure *)NULL;
								callback.data=(void *)NULL;
								CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
									settings_editor->iso_scalar_field_widget,&callback);
								XtUnmanageChild(settings_editor->iso_surface_entry);
							}

							/* node_points, data_points, element_points */
							/* glyphs */
							if (((GT_ELEMENT_SETTINGS_NODE_POINTS == settings_type) ||
								(GT_ELEMENT_SETTINGS_DATA_POINTS == settings_type) ||
								(GT_ELEMENT_SETTINGS_ELEMENT_POINTS == settings_type)) &&
								settings_editor_display_glyph_specific(settings_editor))
							{
								/* turn on callbacks */
								callback.data=(void *)settings_editor;
								callback.procedure=settings_editor_update_glyph;
								CHOOSE_OBJECT_LIST_SET_CALLBACK(GT_object)(
									settings_editor->glyph_widget,&callback);
								callback.data = (void *)settings_editor;
								callback.procedure = settings_editor_update_glyph_scaling_mode;
								choose_enumerator_set_callback(
									settings_editor->glyph_scaling_mode_widget, &callback);
								callback.data=(void *)settings_editor;
								callback.procedure=
									settings_editor_update_glyph_orientation_scale_field;
								CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
									settings_editor->glyph_orientation_scale_field_widget,
									&callback);
								callback.data=(void *)settings_editor;
								callback.procedure=
									settings_editor_update_glyph_variable_scale_field;
								CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
									settings_editor->glyph_variable_scale_field_widget,
									&callback);
								XtManageChild(settings_editor->glyph_group_entry);
							}
							else
							{
								/* turn off callbacks */
								callback.procedure=(Callback_procedure *)NULL;
								callback.data=(void *)NULL;
								CHOOSE_OBJECT_LIST_SET_CALLBACK(GT_object)(
									settings_editor->glyph_widget,&callback);
								choose_enumerator_set_callback(
									settings_editor->glyph_scaling_mode_widget, &callback);
								CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
									settings_editor->glyph_orientation_scale_field_widget,
									&callback);
								CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
									settings_editor->glyph_variable_scale_field_widget,
									&callback);
								XtUnmanageChild(settings_editor->glyph_group_entry);
							}

							/* label field */
							if ((GT_ELEMENT_SETTINGS_NODE_POINTS==settings_type)||
								(GT_ELEMENT_SETTINGS_DATA_POINTS==settings_type)||
								(GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings_type))
							{
								label_field=GT_element_settings_get_label_field(new_settings);
								field_set=((struct Computed_field *)NULL != label_field);
								XtVaSetValues(settings_editor->label_field_button,
									XmNset,field_set,NULL);
								if (field_set)
								{
									CHOOSE_OBJECT_SET_OBJECT(Computed_field)(
										settings_editor->label_field_widget,label_field);
								}
								XtSetSensitive(settings_editor->label_field_widget,field_set);
								/* turn on callbacks */
								callback.data=(void *)settings_editor;
								callback.procedure=settings_editor_update_label_field;
								CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
									settings_editor->label_field_widget,&callback);
								XtManageChild(settings_editor->label_field_entry);
							}
							else
							{
								/* turn off callbacks */
								callback.procedure=(Callback_procedure *)NULL;
								callback.data=(void *)NULL;
								CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
									settings_editor->label_field_widget,&callback);
								XtUnmanageChild(settings_editor->label_field_entry);
							}

							/* element_points and iso_surfaces */
							if ((GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings_type)||
								(GT_ELEMENT_SETTINGS_ISO_SURFACES==settings_type))
							{
								choose_enumerator_set_string(
									settings_editor->use_element_type_widget,
									ENUMERATOR_STRING(Use_element_type)(
										GT_element_settings_get_use_element_type(new_settings)));
								XtManageChild(settings_editor->use_element_type_entry);
								/* turn on callbacks */
								callback.data=(void *)settings_editor;
								callback.procedure=settings_editor_update_use_element_type;
								choose_enumerator_set_callback(
									settings_editor->use_element_type_widget,&callback);
							}
							else
							{
								XtUnmanageChild(settings_editor->use_element_type_entry);
								/* turn off callbacks */
								callback.procedure=(Callback_procedure *)NULL;
								callback.data=(void *)NULL;
								choose_enumerator_set_callback(
									settings_editor->use_element_type_widget,&callback);
							}

							/* element_points */
							if (GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings_type)
							{
								GT_element_settings_get_xi_discretization(new_settings,
									&xi_discretization_mode, &xi_point_density_field);
								choose_enumerator_set_string(
									settings_editor->xi_discretization_mode_widget,
									ENUMERATOR_STRING(Xi_discretization_mode)(
										xi_discretization_mode));
								XtManageChild(settings_editor->xi_discretization_mode_entry);

								GT_element_settings_get_discretization(new_settings,
									&discretization);
								sprintf(temp_string,"%d*%d*%d",discretization.number_in_xi1,
									discretization.number_in_xi2,discretization.number_in_xi3);
								XtVaSetValues(settings_editor->discretization_text,
									XmNvalue,temp_string,NULL);
								XtSetSensitive(settings_editor->discretization_entry,
									XI_DISCRETIZATION_EXACT_XI != xi_discretization_mode);
								XtManageChild(settings_editor->discretization_entry);

								native_discretization_field=
									GT_element_settings_get_native_discretization_field(
										settings_editor->current_settings);
								field_set=
									((struct FE_field *)NULL != native_discretization_field);
								XtVaSetValues(settings_editor->native_discretization_button,
									XmNset, field_set, NULL);
								if (field_set)
								{
									FE_REGION_CHOOSE_OBJECT_SET_OBJECT(FE_field)(
										settings_editor->native_discretization_field_widget,
										native_discretization_field);
								}
								XtSetSensitive(settings_editor->
									native_discretization_field_widget,field_set);
								XtSetSensitive(settings_editor->native_discretization_entry,
									XI_DISCRETIZATION_EXACT_XI != xi_discretization_mode);
								XtManageChild(settings_editor->native_discretization_entry);

								field_set =
									((struct Computed_field *)NULL != xi_point_density_field);
								if (field_set)
								{
									CHOOSE_OBJECT_SET_OBJECT(Computed_field)(
										settings_editor->xi_point_density_field_widget,
										xi_point_density_field);
								}
								XtSetSensitive(settings_editor->xi_point_density_field_entry,
									field_set);
								XtManageChild(settings_editor->xi_point_density_field_entry);

								/* turn on callbacks */
								callback.data=(void *)settings_editor;
								callback.procedure=
									settings_editor_update_native_discretization_field;
								FE_REGION_CHOOSE_OBJECT_SET_CALLBACK(FE_field)(
									settings_editor->native_discretization_field_widget,
									&callback);
								callback.procedure =
									settings_editor_update_xi_point_density_field;
								CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
									settings_editor->xi_point_density_field_widget,
									&callback);
								callback.procedure=
									settings_editor_update_xi_discretization_mode;
								choose_enumerator_set_callback(
									settings_editor->xi_discretization_mode_widget,&callback);
							}
							else
							{
								XtUnmanageChild(settings_editor->discretization_entry);
								XtUnmanageChild(settings_editor->native_discretization_entry);
								XtUnmanageChild(settings_editor->xi_point_density_field_entry);
								XtUnmanageChild(settings_editor->xi_discretization_mode_entry);
								/* turn off callbacks */
								callback.procedure=(Callback_procedure *)NULL;
								callback.data=(void *)NULL;
								FE_REGION_CHOOSE_OBJECT_SET_CALLBACK(FE_field)(
									settings_editor->native_discretization_field_widget,
									&callback);
								CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
									settings_editor->xi_point_density_field_widget, &callback);
								choose_enumerator_set_callback(
									settings_editor->xi_discretization_mode_widget,&callback);
							}

							/* volume texture */
							if (GT_ELEMENT_SETTINGS_VOLUMES==settings_type)
							{
								CHOOSE_OBJECT_SET_OBJECT(VT_volume_texture)(
									settings_editor->volume_texture_widget,
									GT_element_settings_get_volume_texture(new_settings));
								XtManageChild(settings_editor->volume_texture_entry);
								/* turn on callbacks */
								callback.data=(void *)settings_editor;
								callback.procedure=settings_editor_update_volume_texture;
								CHOOSE_OBJECT_SET_CALLBACK(VT_volume_texture)(
									settings_editor->volume_texture_widget,&callback);
							}
							else
							{
								XtUnmanageChild(settings_editor->volume_texture_entry);
								/* turn off callbacks */
								callback.procedure=(Callback_procedure *)NULL;
								callback.data=(void *)NULL;
								CHOOSE_OBJECT_SET_CALLBACK(VT_volume_texture)(
									settings_editor->volume_texture_widget,&callback);
							}

							/* seed element */
							if ((GT_ELEMENT_SETTINGS_VOLUMES==settings_type)||
								(GT_ELEMENT_SETTINGS_STREAMLINES==settings_type))
							{
								if (seed_element=
									GT_element_settings_get_seed_element(new_settings))
								{
									TEXT_CHOOSE_FROM_FE_REGION_SET_OBJECT(FE_element)(
										settings_editor->seed_element_widget,seed_element);
								}
								XtVaSetValues(settings_editor->seed_element_button,
									XmNset,(NULL != seed_element),NULL);
								XtSetSensitive(settings_editor->seed_element_widget,
									NULL != seed_element);
								XtManageChild(settings_editor->seed_element_entry);
								/* turn on callbacks */
								callback.data=(void *)settings_editor;
								callback.procedure=settings_editor_update_seed_element;
								TEXT_CHOOSE_FROM_FE_REGION_SET_CALLBACK(FE_element)(
									settings_editor->seed_element_widget,&callback);
							}
							else
							{
								XtUnmanageChild(settings_editor->seed_element_entry);
								/* turn off callbacks */
								callback.procedure=(Callback_procedure *)NULL;
								callback.data=(void *)NULL;
								TEXT_CHOOSE_FROM_FE_REGION_SET_CALLBACK(FE_element)(
									settings_editor->seed_element_widget,&callback);
							}

							/* seed xi */
							if ((GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings_type)||
								(GT_ELEMENT_SETTINGS_STREAMLINES==settings_type))
							{
								GT_element_settings_get_seed_xi(new_settings,seed_xi);
								sprintf(temp_string,"%g,%g,%g",
									seed_xi[0],seed_xi[1],seed_xi[2]);
								XtVaSetValues(settings_editor->seed_xi_text,XmNvalue,
									temp_string,NULL);
								XtManageChild(settings_editor->seed_xi_entry);
								if (GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings_type)
								{
									XtSetSensitive(settings_editor->seed_xi_entry,
										XI_DISCRETIZATION_EXACT_XI == xi_discretization_mode);
								}
								else
								{
									XtSetSensitive(settings_editor->seed_xi_entry,True);
								}
							}
							else
							{
								XtUnmanageChild(settings_editor->seed_xi_entry);
							}

							/* streamlines */
							if (GT_ELEMENT_SETTINGS_STREAMLINES==settings_type)
							{
								GT_element_settings_get_streamline_parameters(new_settings,
									&streamline_type,&stream_vector_field,&reverse_track,
									&streamline_length,&streamline_width);
								choose_enumerator_set_string(
									settings_editor->streamline_type_widget,
									ENUMERATOR_STRING(Streamline_type)(streamline_type));
								sprintf(temp_string,"%g",streamline_length);
								XtVaSetValues(settings_editor->streamline_length_text,XmNvalue,
									temp_string,NULL);
								sprintf(temp_string,"%g",streamline_width);
								XtVaSetValues(settings_editor->streamline_width_text,XmNvalue,
									temp_string,NULL);
								CHOOSE_OBJECT_SET_OBJECT(Computed_field)(
									settings_editor->stream_vector_field_widget,
									stream_vector_field);
								XtVaSetValues(settings_editor->streamline_reverse_button,
									XmNset,reverse_track,NULL);
								XtManageChild(settings_editor->streamline_entry);
								/* turn on callbacks */
								callback.data=(void *)settings_editor;
								callback.procedure=
									settings_editor_update_streamline_type;
								choose_enumerator_set_callback(
									settings_editor->streamline_type_widget,&callback);
								callback.procedure=
									settings_editor_update_stream_vector_field;
								CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
									settings_editor->stream_vector_field_widget,&callback);
							}
							else
							{
								XtUnmanageChild(settings_editor->streamline_entry);
								/* turn off callbacks */
								callback.procedure=(Callback_procedure *)NULL;
								callback.data=(void *)NULL;
								choose_enumerator_set_callback(
									settings_editor->streamline_type_widget,&callback);
								CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
									settings_editor->stream_vector_field_widget,&callback);
							}

							select_mode = GT_element_settings_get_select_mode(new_settings);
							choose_enumerator_set_string(settings_editor->select_mode_widget,
								ENUMERATOR_STRING(Graphics_select_mode)(select_mode));
							callback.data=(void *)settings_editor;
							callback.procedure=settings_editor_update_select_mode;
							choose_enumerator_set_callback(
								settings_editor->select_mode_widget,&callback);

							/* set values of appearance settings widgets */
							/* material */
							CHOOSE_OBJECT_SET_OBJECT(Graphical_material)(
								settings_editor->material_widget,
								GT_element_settings_get_material(new_settings));
							callback.data=(void *)settings_editor;
							callback.procedure=settings_editor_update_material;
							CHOOSE_OBJECT_SET_CALLBACK(Graphical_material)(
								settings_editor->material_widget,&callback);

							/* line_width */
							if (GT_ELEMENT_SETTINGS_LINES==settings_type)
							{
								line_width = GT_element_settings_get_line_width(new_settings);
								sprintf(temp_string,"%d",line_width);
								XtVaSetValues(settings_editor->line_width_text,XmNvalue,
									temp_string,NULL);
								XtManageChild(settings_editor->line_width_entry);
							}
							else
							{
								XtUnmanageChild(settings_editor->line_width_entry);
							}

							if (GT_ELEMENT_SETTINGS_STREAMLINES==settings_type)
							{
								GT_element_settings_get_data_spectrum_parameters_streamlines(
									settings_editor->current_settings,&streamline_data_type,
									&data_field,&spectrum);
								choose_enumerator_set_string(
									settings_editor->streamline_data_type_widget,
									ENUMERATOR_STRING(Streamline_data_type)(
										streamline_data_type));
								XtManageChild(settings_editor->streamline_data_type_entry);
								field_set=((struct Computed_field *)NULL != data_field);
								XtSetSensitive(settings_editor->data_field_widget,field_set);
								XtSetSensitive(settings_editor->spectrum_entry,field_set);
								/* gray out data_field_button */
								XtSetSensitive(settings_editor->data_field_button,0);
								/* turn on callbacks */
								callback.data=(void *)settings_editor;
								callback.procedure=
									settings_editor_update_streamline_data_type;
								choose_enumerator_set_callback(
									settings_editor->streamline_data_type_widget,&callback);
							}
							else
							{
								/* set scalar data field & spectrum */
								GT_element_settings_get_data_spectrum_parameters(new_settings,
									&data_field,&spectrum);
								field_set=((struct Computed_field *)NULL != data_field);
								XtVaSetValues(settings_editor->data_field_button,
									XmNset,field_set,NULL);
								if (field_set)
								{
									CHOOSE_OBJECT_SET_OBJECT(Computed_field)(
										settings_editor->data_field_widget,data_field);
									CHOOSE_OBJECT_SET_OBJECT(Spectrum)(
										settings_editor->spectrum_widget,spectrum);
								}
								XtSetSensitive(settings_editor->data_field_widget,field_set);
								XtSetSensitive(settings_editor->spectrum_entry,field_set);
								/* sort out streamline_data_type stuff */
								XtUnmanageChild(settings_editor->streamline_data_type_entry);
								/* ungray data_field_button */
								XtSetSensitive(settings_editor->data_field_button,1);
								/* turn off callbacks */
								callback.procedure=(Callback_procedure *)NULL;
								callback.data=(void *)NULL;
								choose_enumerator_set_callback(
									settings_editor->streamline_data_type_widget,&callback);
							}

							/* turn on callbacks */
							callback.data=(void *)settings_editor;
							callback.procedure=settings_editor_update_data_field;
							CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
								settings_editor->data_field_widget,&callback);
							callback.data=(void *)settings_editor;
							callback.procedure=settings_editor_update_spectrum;
							CHOOSE_OBJECT_SET_CALLBACK(Spectrum)(
								settings_editor->spectrum_widget,&callback);

							if ((GT_ELEMENT_SETTINGS_CYLINDERS == settings_type) ||
								(GT_ELEMENT_SETTINGS_SURFACES == settings_type) ||
							  (GT_ELEMENT_SETTINGS_ISO_SURFACES == settings_type))
							{
								/* set texture_coordinate field */
								texture_coord_field = GT_element_settings_get_texture_coordinate_field
									(new_settings);
								field_set=((struct Computed_field *)NULL != texture_coord_field);
								XtVaSetValues(settings_editor->texture_coord_field_button,
									XmNset,field_set,NULL);
								if (field_set)
								{
									CHOOSE_OBJECT_SET_OBJECT(Computed_field)(
										settings_editor->texture_coord_field_widget,texture_coord_field);
								}
								XtSetSensitive(settings_editor->texture_coord_field_widget,field_set);
								/* turn on callbacks */
								callback.data=(void *)settings_editor;
								callback.procedure=settings_editor_update_texture_coord_field;
								CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
									settings_editor->texture_coord_field_widget,&callback);
								XtManageChild(settings_editor->texture_coord_field_entry);
							}
							else
							{
								/* turn off callbacks */
								callback.data=(void *)NULL;
								callback.procedure=(Callback_procedure *)NULL;
								CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
									settings_editor->texture_coord_field_widget,&callback);
								XtUnmanageChild(settings_editor->texture_coord_field_entry);
							}

							/* render_type */
							if ((GT_ELEMENT_SETTINGS_SURFACES==settings_type)
							  || (GT_ELEMENT_SETTINGS_VOLUMES==settings_type)
							  || (GT_ELEMENT_SETTINGS_ISO_SURFACES==settings_type))
							{
								choose_enumerator_set_string(
									settings_editor->render_type_widget,
									ENUMERATOR_STRING(Render_type)(
										GT_element_settings_get_render_type(new_settings)));
								XtManageChild(settings_editor->render_type_entry);
								/* turn on callbacks */
								callback.data=(void *)settings_editor;
								callback.procedure=settings_editor_update_render_type;
								choose_enumerator_set_callback(
									settings_editor->render_type_widget,&callback);
							}
							else
							{
								XtUnmanageChild(settings_editor->render_type_entry);
								/* turn off callbacks */
								callback.data=(void *)NULL;
								callback.procedure=(Callback_procedure *)NULL;
								choose_enumerator_set_callback(
									settings_editor->render_type_widget,&callback);
							}
							
							/* selected material */
							CHOOSE_OBJECT_SET_OBJECT(Graphical_material)(
								settings_editor->selected_material_widget,
								GT_element_settings_get_selected_material(new_settings));
							callback.data=(void *)settings_editor;
							callback.procedure=settings_editor_update_selected_material;
							CHOOSE_OBJECT_SET_CALLBACK(Graphical_material)(
								settings_editor->selected_material_widget,&callback);
						}
						else
						{
							DESTROY(GT_element_settings)
								(&(settings_editor->current_settings));
							new_settings=(struct GT_element_settings *)NULL;
							return_code=0;
						}
					}
					else
					{
						new_settings=(struct GT_element_settings *)NULL;
						return_code=0;
					}
				}
				if ((struct GT_element_settings *)NULL==new_settings)
				{
					XtUnmanageChild(settings_editor->widget);
					/* switch off subwidget callbacks */
					callback.procedure=(Callback_procedure *)NULL;
					callback.data=(void *)NULL;
					CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
						settings_editor->coordinate_field_widget,&callback);
					CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
						settings_editor->radius_scalar_field_widget,&callback);
					CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
						settings_editor->iso_scalar_field_widget,&callback);
					CHOOSE_OBJECT_LIST_SET_CALLBACK(GT_object)(
						settings_editor->glyph_widget,&callback);
					CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
						settings_editor->glyph_orientation_scale_field_widget,&callback);
					CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
						settings_editor->label_field_widget,&callback);
					FE_REGION_CHOOSE_OBJECT_SET_CALLBACK(FE_field)(
						settings_editor->native_discretization_field_widget,&callback);
					CHOOSE_OBJECT_SET_CALLBACK(VT_volume_texture)(
						settings_editor->volume_texture_widget,&callback);
					TEXT_CHOOSE_FROM_FE_REGION_SET_CALLBACK(FE_element)(
						settings_editor->seed_element_widget,&callback);
					CHOOSE_OBJECT_SET_CALLBACK(Graphical_material)(
						settings_editor->material_widget,&callback);
					CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
						settings_editor->data_field_widget,&callback);
					CHOOSE_OBJECT_SET_CALLBACK(Spectrum)(
						settings_editor->spectrum_widget,&callback);
					CHOOSE_OBJECT_SET_CALLBACK(Graphical_material)(
						settings_editor->selected_material_widget,&callback);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"settings_editor_set_settings.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_set_settings.  Missing widget");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* settings_editor_set_settings */
