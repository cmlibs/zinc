/*******************************************************************************
FILE : graphical_element_editor.c

LAST MODIFIED : 7 March 2002

DESCRIPTION :
Provides the widgets to manipulate graphical element group settings.
==============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/FormP.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include "choose/choose_computed_field.h"
#include "choose/choose_enumerator.h"
#include "choose/choose_fe_field.h"
#include "command/parser.h"
#include "general/compare.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "graphics/element_group_settings.h"
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

struct Graphical_element_editor;

struct Settings_item
/*******************************************************************************
LAST MODIFIED : 19 November 2001

DESCRIPTION :
==============================================================================*/
{
	struct GT_element_settings *settings;
	/* cache current settings string so only updated if changed */
	char *settings_string;
	struct Graphical_element_editor *gelem_editor;
	int visible;
	Widget form, parent_form, previous_widget, select_button, visibility_button;
	int in_use;
	int access_count;
};

PROTOTYPE_OBJECT_FUNCTIONS(Settings_item);

DECLARE_LIST_TYPES(Settings_item);

PROTOTYPE_LIST_FUNCTIONS(Settings_item);

PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Settings_item, settings, \
	struct GT_element_settings *);

FULL_DECLARE_INDEXED_LIST_TYPE(Settings_item);

struct Graphical_element_editor
/*******************************************************************************
LAST MODIFIED : 22 January 2002

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
	struct GT_element_settings *current_settings;
	struct Callback_data update_callback;
	int general_settings_expanded;
	struct LIST(Settings_item) *settings_item_list;
	Widget general_button,general_rowcol,default_coordinate_field_form,
		default_coordinate_field_widget,element_disc_text,
		circle_disc_text,native_discretization_button,
		native_discretization_field_form,native_discretization_field_widget,
		settings_type_form,
		settings_type_widget, settings_scroll, settings_list_form, add_button,
		delete_button,up_button,down_button,settings_form,settings_widget,
		cylinders_button;
	Widget *widget_address,widget,widget_parent;
	Pixel select_background_color, select_foreground_color;
}; /* Graphical_element_editor */

/*
Module functions
----------------
*/

static void XmForm_resize(Widget form)
/*******************************************************************************
LAST MODIFIED : 14 November 2001

DESCRIPTION :
Calls a private function of the <form> to force a resize of the widget.
???RC Can't find any other way of getting this to work properly!
???RC Also in <scene_editor>. Put in common place?
==============================================================================*/
{
	ENTER(XmForm_resize);
	if (form && XmIsForm(form))
	{
		(((XmFormWidgetClass)(form->core.widget_class))->composite_class.change_managed)(form);
	}
	else
	{
		display_message(ERROR_MESSAGE, "XmForm_resize.  Missing or invalid form");
	}
	LEAVE;
} /* XmForm_resize */

struct Settings_item *CREATE(Settings_item)(
	struct Graphical_element_editor *gelem_editor,
	struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 19 November 2001

DESCRIPTION :
Creates a Settings_item object for the <settings>.
Note the Settings_item_update function is responsible for
creating and updating widgets.
==============================================================================*/
{
	struct Settings_item *settings_item;

	ENTER(CREATE(Settings_item));
	settings_item = (struct Settings_item *)NULL;
	if (gelem_editor && settings)
	{
		if (ALLOCATE(settings_item, struct Settings_item, 1))
		{
			settings_item->settings = ACCESS(GT_element_settings)(settings);
			settings_item->settings_string = (char *)NULL;
			settings_item->gelem_editor = gelem_editor;
			settings_item->visible = GT_element_settings_get_visibility(settings);
			settings_item->in_use = 0;
			settings_item->access_count = 0;

			settings_item->form = (Widget)NULL;
			settings_item->select_button = (Widget)NULL;
			settings_item->previous_widget = (Widget)NULL;
			settings_item->visibility_button = (Widget)NULL;
		}
		else
		{
			display_message(ERROR_MESSAGE, "CREATE(Settings_item).  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Settings_item).  Invalid argument(s)");
	}
	LEAVE;

	return (settings_item);
} /* CREATE(Settings_item) */

int DESTROY(Settings_item)(struct Settings_item **settings_item_address)
/*******************************************************************************
LAST MODIFIED : 19 November 2001

DESCRIPTION :
==============================================================================*/
{
	struct Settings_item *settings_item;
	int return_code;

	ENTER(DESTROY(Settings_item));
	if (settings_item_address && (settings_item = *settings_item_address))
	{
		if (0 == settings_item->access_count)
		{
			DEACCESS(GT_element_settings)(&(settings_item->settings));
			if (settings_item->settings_string)
			{
				DEALLOCATE(settings_item->settings_string);
			}
			if (settings_item->form)
			{
				XtDestroyWidget(settings_item->form);
			}
			DEALLOCATE(*settings_item_address);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Settings_item).  Non-zero access_count");
			return_code = 0;
		}
		*settings_item_address = (struct Settings_item *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Settings_item).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Settings_item) */

DECLARE_OBJECT_FUNCTIONS(Settings_item)

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Settings_item, settings, \
	struct GT_element_settings *, compare_pointer)

DECLARE_INDEXED_LIST_FUNCTIONS(Settings_item)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Settings_item, settings, \
	struct GT_element_settings *, compare_pointer)

static int Settings_item_clear_in_use(struct Settings_item *settings_item,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 19 November 2001

DESCRIPTION :
Clears settings_item->in_use flag to zero.
==============================================================================*/
{
	int return_code;

	ENTER(Settings_item_clear_in_use);
	USE_PARAMETER(dummy_void);
	if (settings_item)
	{
		settings_item->in_use = 0;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Settings_item_clear_in_use.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Settings_item_clear_in_use */

static int Settings_item_not_in_use(struct Settings_item *settings_item,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 19 November 2001

DESCRIPTION :
Returns true if settings_item->in_use is not set.
==============================================================================*/
{
	int return_code;

	ENTER(Settings_item_not_in_use);
	USE_PARAMETER(dummy_void);
	if (settings_item)
	{
		return_code = !(settings_item->in_use);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Settings_item_not_in_use.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Settings_item_not_in_use */

static int graphical_element_editor_update(
	struct Graphical_element_editor *gelem_editor)
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

int Graphical_element_editor_set_object_highlight(
	struct Graphical_element_editor *gelem_editor,
	struct GT_element_settings *settings, int state)
/*******************************************************************************
LAST MODIFIED : 22 January 2002

DESCRIPTION :
Function adds/removes highlighting of selected object by inverting colours.
==============================================================================*/
{
	int return_code;
	struct Settings_item *settings_item;

	ENTER(Graphical_element_editor_set_object_highlight);
	if (gelem_editor && settings)
	{
		if (settings_item = FIND_BY_IDENTIFIER_IN_LIST(Settings_item, settings)(
			settings, gelem_editor->settings_item_list))
		{
			if (state)
			{
				XtVaSetValues(settings_item->select_button, XmNbackground,
					gelem_editor->select_foreground_color, NULL);
				XtVaSetValues(settings_item->select_button, XmNforeground,
					gelem_editor->select_background_color, NULL);
			}
			else
			{
				XtVaSetValues(settings_item->select_button, XmNbackground,
					gelem_editor->select_background_color, NULL);
				XtVaSetValues(settings_item->select_button, XmNforeground,
					gelem_editor->select_foreground_color, NULL);
			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_element_editor_set_object_highlight.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_element_editor_set_object_highlight */

static int Graphical_element_editor_set_current_settings(
	struct Graphical_element_editor *gelem_editor,
	struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 22 January 2002

DESCRIPTION :
Sets the current_settings in the <gelem_editor> for editing. Updates widgets.
==============================================================================*/
{
	enum GT_element_settings_type settings_type;
	int have_settings, return_code;

	ENTER(Graphical_element_editor_set_current_settings);
	if (gelem_editor)
	{
		if (settings != gelem_editor->current_settings)
		{
			if (gelem_editor->current_settings)
			{
				Graphical_element_editor_set_object_highlight(gelem_editor,
					gelem_editor->current_settings, /*state*/FALSE);
			}
			REACCESS(GT_element_settings)(&(gelem_editor->current_settings),
				settings);
			if (settings)
			{
				Graphical_element_editor_set_object_highlight(gelem_editor,
					settings, /*state*/TRUE);
				/* if settings_type changed, select it in settings_type option menu */
				settings_type = GT_element_settings_get_settings_type(settings);
				if (settings_type != gelem_editor->current_settings_type)
				{
					gelem_editor->current_settings_type = settings_type;
					choose_enumerator_set_string(gelem_editor->settings_type_widget,
						ENUMERATOR_STRING(GT_element_settings_type)(settings_type));
					XmForm_resize(gelem_editor->settings_type_form);
				}
			}
			have_settings =
				((struct GT_element_settings *)NULL != gelem_editor->current_settings);
			/* Grey delete and move buttons if no current_settings */
			XtSetSensitive(gelem_editor->delete_button, have_settings);
			XtSetSensitive(gelem_editor->up_button, have_settings);
			XtSetSensitive(gelem_editor->down_button, have_settings);
			/* send selected object to settings editor */
			settings_editor_set_settings(gelem_editor->settings_widget,
				gelem_editor->current_settings);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_element_editor_set_current_settings.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	
	return (return_code);
} /* Graphical_element_editor_set_current_settings */

static int Graphical_element_editor_set_current_settings_from_position(
	struct Graphical_element_editor *gelem_editor, int position)
/*******************************************************************************
LAST MODIFIED : 19 November 2001

DESCRIPTION :
Sets the current_settings in the <gelem_editor> for editing. Updates widgets.
==============================================================================*/
{
	int max_position, return_code;
	struct GT_element_settings *settings;

	ENTER(Graphical_element_editor_set_current_settings_from_position);
	if (gelem_editor)
	{
		settings = (struct GT_element_settings *)NULL;
		if (gelem_editor->edit_gt_element_group)
		{
			max_position = GT_element_group_get_number_of_settings(
				gelem_editor->edit_gt_element_group);
			if ((1 > position) || (max_position < position))
			{
				position = max_position;
			}
			settings = get_settings_at_position_in_GT_element_group(
				gelem_editor->edit_gt_element_group, position);
		}
		return_code =
			Graphical_element_editor_set_current_settings(gelem_editor, settings);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_element_editor_set_current_settings_from_position.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	
	return (return_code);
} /* Graphical_element_editor_set_current_settings_from_position */

static int Graphical_element_editor_set_current_settings_from_type(
	struct Graphical_element_editor *gelem_editor)
/*******************************************************************************
LAST MODIFIED : 20 November 2001

DESCRIPTION :
Sets the current_settings in the <gelem_editor> for editing. Updates widgets.
==============================================================================*/
{
	int return_code;
	struct GT_element_settings *settings;

	ENTER(Graphical_element_editor_set_current_settings_from_type);
	if (gelem_editor)
	{
		settings = (struct GT_element_settings *)NULL;
		if (gelem_editor->edit_gt_element_group)
		{
			settings = first_settings_in_GT_element_group_that(
				gelem_editor->edit_gt_element_group,
				GT_element_settings_type_matches,
				(void *)gelem_editor->current_settings_type);
		}
		return_code =
			Graphical_element_editor_set_current_settings(gelem_editor, settings);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_element_editor_set_current_settings_from_type.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	
	return (return_code);
} /* Graphical_element_editor_set_current_settings_from_type */

static int Graphical_element_editor_update_Settings_item(
	struct Graphical_element_editor *gelem_editor,
	struct GT_element_settings *settings,
	Widget *previous_widget_address);
/*******************************************************************************
LAST MODIFIED : 19 November 2001

DESCRIPTION :
Tries to find by name a Settings_item in <settings_item_list> that
matches the string for <settings>. If there is none, one is created and added
to the settings_item_list. Then goes through each widget in the settings_item
and makes sure it is created and displaying the values from the settings and has
the correct previous widget. Marks the object as "in_use" so that those
that are not are removed.
Prototype.
==============================================================================*/

static void graphical_element_editor_settings_visibility_CB(Widget widget,
	XtPointer client_data, XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 November 2001

DESCRIPTION :
Called when a settings visibility toggle button is selected.
==============================================================================*/
{
	struct Settings_item *settings_item;
	Widget previous_widget;

	ENTER(graphical_element_editor_settings_visibility_CB);
	USE_PARAMETER(call_data);
	if (widget && (settings_item = (struct Settings_item *)client_data))
	{
		GT_element_settings_set_visibility(settings_item->settings,
			XmToggleButtonGetState(widget));
		previous_widget = settings_item->previous_widget;
		Graphical_element_editor_update_Settings_item(settings_item->gelem_editor,
			settings_item->settings, &previous_widget);
		/* ensure this settings is the currently selected one */
		Graphical_element_editor_set_current_settings(settings_item->gelem_editor,
			settings_item->settings);
		/* inform the client of the changes */
		graphical_element_editor_update(settings_item->gelem_editor);
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
LAST MODIFIED : 22 January 2002

DESCRIPTION :
Called when a settings select toggle button is selected.
==============================================================================*/
{
	struct Settings_item *settings_item;

	ENTER(graphical_element_editor_settings_select_CB);
	USE_PARAMETER(call_data);
	if (widget && (settings_item = (struct Settings_item *)client_data))
	{
		Graphical_element_editor_set_current_settings(settings_item->gelem_editor,
			settings_item->settings);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_settings_select_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_settings_select_CB */

static int Graphical_element_editor_update_Settings_item(
	struct Graphical_element_editor *gelem_editor,
	struct GT_element_settings *settings,
	Widget *previous_widget_address)
/*******************************************************************************
LAST MODIFIED : 22 January 2002

DESCRIPTION :
Tries to find by name a Settings_item in <settings_item_list> that
matches the string for <settings>. If there is none, one is created and added
to the settings_item_list. Then goes through each widget in the settings_item
and makes sure it is created and displaying the values from the settings and has
the correct previous widget. Marks the object as "in_use" so that those
that are not are removed.
==============================================================================*/
{
	Arg args[12];
	char *settings_string;
	int num_args, return_code, visible;
	struct Settings_item *settings_item;
	XmString label_string;

	ENTER(Graphical_element_editor_update_Settings_item);
	if (gelem_editor && settings &&	previous_widget_address)
	{
		return_code = 1;
		if (settings_string = GT_element_settings_string(settings,
			SETTINGS_STRING_COMPLETE_PLUS))
		{
			if (!(settings_item = FIND_BY_IDENTIFIER_IN_LIST(Settings_item, settings)(
				settings, gelem_editor->settings_item_list)))
			{
				settings_item = CREATE(Settings_item)(gelem_editor, settings);
				if (!ADD_OBJECT_TO_LIST(Settings_item)(settings_item,
					gelem_editor->settings_item_list))
				{
					DESTROY(Settings_item)(&settings_item);
					settings_item = (struct Settings_item *)NULL;
				}
			}
			if (settings_item)
			{
				/* create/update the widgets */

				/* form widget */
				if ((!(settings_item->form)) ||
					(settings_item->previous_widget != *previous_widget_address))
				{
					num_args = 0;
					if (*previous_widget_address)
					{
						XtSetArg(args[num_args], XmNtopAttachment, XmATTACH_WIDGET);
						num_args++;
						XtSetArg(args[num_args], XmNtopWidget,
							(XtPointer)(*previous_widget_address));
						num_args++;
					}
					else
					{
						XtSetArg(args[num_args], XmNtopAttachment, XmATTACH_FORM);
						num_args++;
					}
					if (settings_item->form)
					{
						XtSetValues(settings_item->form, args, num_args);
					}
					else
					{
						XtSetArg(args[num_args], XmNleftAttachment, XmATTACH_FORM);
						num_args++;
						settings_item->form = XmCreateForm(
							gelem_editor->settings_list_form, "form", args, num_args);
						XtManageChild(settings_item->form);
					}
					settings_item->previous_widget = *previous_widget_address;
				}
				*previous_widget_address = settings_item->form;

				/* visibility_button */
				visible = GT_element_settings_get_visibility(settings);
				if (settings_item->visibility_button)
				{
					if (visible != settings_item->visible)
					{
						XmToggleButtonSetState(settings_item->visibility_button,
							/*state*/visible, /*notify*/FALSE);
					}
				}
				else
				{
					num_args = 0;
					XtSetArg(args[num_args], XmNleftAttachment, XmATTACH_FORM);
					num_args++;
					XtSetArg(args[num_args], XmNtopAttachment, XmATTACH_FORM);
					num_args++;
					XtSetArg(args[num_args], XmNset, visible);
					num_args++;
					XtSetArg(args[num_args], XmNindicatorOn, TRUE);
					num_args++;
					XtSetArg(args[num_args], XmNindicatorSize, 16);
					num_args++;
					XtSetArg(args[num_args], XmNmarginWidth, 0);
					num_args++;
					XtSetArg(args[num_args], XmNmarginHeight, 0);
					num_args++;
					XtSetArg(args[num_args], XmNspacing, 0);
					num_args++;
					XtSetArg(args[num_args], XmNfontList,
						(XtPointer)User_interface_get_normal_fontlist(gelem_editor->user_interface));
					num_args++;
					settings_item->visibility_button = XmCreateToggleButton(
						settings_item->form, "", args, num_args);
					XtAddCallback(settings_item->visibility_button,
						XmNvalueChangedCallback,
						graphical_element_editor_settings_visibility_CB,
						(XtPointer)settings_item);
					XtManageChild(settings_item->visibility_button);
				}
				settings_item->visible = visible;

				/* select_button */
				if (!(settings_item->select_button))
				{
					num_args = 0;
					XtSetArg(args[num_args], XmNtopAttachment, XmATTACH_FORM);
					num_args++;
					XtSetArg(args[num_args], XmNleftAttachment, XmATTACH_WIDGET);
					num_args++;
					XtSetArg(args[num_args], XmNleftWidget,
						settings_item->visibility_button);
					num_args++;
					XtSetArg(args[num_args], XmNshadowThickness, 0);
					num_args++;
					XtSetArg(args[num_args], XmNborderWidth, 0);
					num_args++;
					XtSetArg(args[num_args], XmNmarginHeight, 2);
					num_args++;
					XtSetArg(args[num_args], XmNmarginWidth, 2);
					num_args++;
					XtSetArg(args[num_args], XmNbackground,
						gelem_editor->select_background_color);
					num_args++;
					XtSetArg(args[num_args], XmNforeground,
						gelem_editor->select_foreground_color);
					num_args++;
						XtSetArg(args[num_args], XmNmarginWidth, 2);
						num_args++;
					XtSetArg(args[num_args], XmNfontList,
						(XtPointer)User_interface_get_normal_fontlist(gelem_editor->user_interface));
					num_args++;
					settings_item->select_button = XmCreatePushButton(
						settings_item->form, settings_string, args, num_args);
					XtAddCallback(settings_item->select_button,
						XmNactivateCallback,
						graphical_element_editor_settings_select_CB,
						(XtPointer)settings_item);
					XtManageChild(settings_item->select_button);
				}
				else
				{
					if (strcmp(settings_string, settings_item->settings_string))
					{
						label_string = XmStringCreateSimple(settings_string);
						XtVaSetValues(settings_item->select_button,
							XmNlabelString, label_string, NULL);
						XmStringFree(label_string);
					}
					DEALLOCATE(settings_item->settings_string);
				}
				settings_item->settings_string = settings_string;

				settings_item->in_use = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE, "Settings_item_update.  "
					"Could not find or create settings_item");
				return_code = 0;
				DEALLOCATE(settings_string);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Graphical_element_editor_update_Settings_item.  "
				"Could not get settings_string");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_element_editor_update_Settings_item.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_element_editor_update_Settings_item */

struct GT_element_settings_update_Settings_item_data
/*******************************************************************************
LAST MODIFIED : 19 November 2001

DESCRIPTION :
Data for function GT_element_settings_update_Settings_item.
==============================================================================*/
{
	struct Graphical_element_editor *gelem_editor;
	Widget previous_widget;
};

static int GT_element_settings_update_Settings_item(
	struct GT_element_settings *settings, void *update_data_void)
/*******************************************************************************
LAST MODIFIED : 19 November 2001

DESCRIPTION :
Iterator function for Graphical_element_editor_update_Settings_item.
==============================================================================*/
{
	int return_code;
	struct GT_element_settings_update_Settings_item_data *update_data;

	ENTER(GT_element_settings_update_Settings_item);
	if (settings && (update_data =
		(struct GT_element_settings_update_Settings_item_data *)update_data_void))
	{
		return_code = Graphical_element_editor_update_Settings_item(
			update_data->gelem_editor, settings, &(update_data->previous_widget));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_update_Settings_item.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_update_Settings_item */

static int Graphical_element_editor_update_settings_list(
	struct Graphical_element_editor *gelem_editor)
/*******************************************************************************
LAST MODIFIED : 19 November 2001

DESCRIPTION :
Clears then fills the settings list RowColumn with descriptions of the settings
of the type gelem_editor->settings_type.
==============================================================================*/
{
	int return_code;
	struct GT_element_settings_update_Settings_item_data update_data;

	ENTER(Graphical_element_editor_update_settings_list);
	if (gelem_editor && gelem_editor->edit_gt_element_group &&
		gelem_editor->settings_list_form)
	{
		return_code = 1;

		/* clear in-use flags so we know which ones are used later */
		FOR_EACH_OBJECT_IN_LIST(Settings_item)(Settings_item_clear_in_use,
			(void *)NULL, gelem_editor->settings_item_list);

		update_data.gelem_editor = gelem_editor;
		update_data.previous_widget = (Widget)NULL;

		for_each_settings_in_GT_element_group(gelem_editor->edit_gt_element_group,
			GT_element_settings_update_Settings_item, (void *)&update_data);

		REMOVE_OBJECTS_FROM_LIST_THAT(Settings_item)(
			Settings_item_not_in_use, (void *)NULL, gelem_editor->settings_item_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_element_editor_update_settings_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_element_editor_update_settings_list */

DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor,general_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor,general_rowcol)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor,default_coordinate_field_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor,element_disc_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor,circle_disc_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor,native_discretization_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor,native_discretization_field_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor,settings_type_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor,add_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor,delete_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor,up_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor,down_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor,settings_scroll)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor,settings_list_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor, \
	Graphical_element_editor,settings_form)

static void graphical_element_editor_destroy_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 November 2001

DESCRIPTION :
Callback for the gelem_editor dialog - tidies up all details - memory etc.
==============================================================================*/
{
	struct Graphical_element_editor *gelem_editor;

	ENTER(graphical_element_editor_destroy_CB);
	USE_PARAMETER(call_data);
	if (widget&&
		(gelem_editor=(struct Graphical_element_editor *)client_data))
	{
		if (gelem_editor->current_settings)
		{
			DEACCESS(GT_element_settings)(&(gelem_editor->current_settings));
		}
		DESTROY(LIST(Settings_item))(&(gelem_editor->settings_item_list));

		/* destroy edit_gt_element_group */
		if (gelem_editor->edit_gt_element_group)
		{
			DEACCESS(GT_element_group)(
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
	struct Graphical_element_editor *gelem_editor)
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
LAST MODIFIED : 20 November 2001

DESCRIPTION :
Toggle for switching the general settings forms on and off.
==============================================================================*/
{
	struct Graphical_element_editor *gelem_editor;
	XmString label_string;

	ENTER(graphical_element_editor_general_button_CB);
	USE_PARAMETER(call_data);
	if (widget&&
		(gelem_editor=(struct Graphical_element_editor *)client_data))
	{
		if (gelem_editor->general_settings_expanded)
		{
			gelem_editor->general_settings_expanded = 0;
			label_string = XmStringCreateSimple("+");
			XtUnmanageChild(gelem_editor->general_rowcol);
		}
		else
		{
			gelem_editor->general_settings_expanded = 1;
			label_string = XmStringCreateSimple("-");
			XtManageChild(gelem_editor->general_rowcol);
		}
		XmForm_resize(gelem_editor->widget);
		XtVaSetValues(gelem_editor->general_button,
			XmNlabelString, label_string, NULL);
		XmStringFree(label_string);
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
	struct Graphical_element_editor *gelem_editor;

	ENTER(graphical_element_editor_update_default_coordinate_field);
	USE_PARAMETER(widget);
	if (default_coordinate_field_void&&
		(gelem_editor=(struct Graphical_element_editor *)gelem_editor_void))
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
LAST MODIFIED : 7 March 2002

DESCRIPTION :
Called when entry is made into the element discretization text field.
==============================================================================*/
{
	char *disc_text;
	struct Graphical_element_editor *gelem_editor;
	struct Parse_state *temp_state;
	struct Element_discretization element_discretization,
		old_element_discretization;

	ENTER(graphical_element_editor_element_disc_text_CB);
	USE_PARAMETER(call_data);
	if (widget&&
		(gelem_editor=(struct Graphical_element_editor *)client_data))
	{
		/* Get the pointer to the text */
		XtVaGetValues(widget,XmNvalue,&disc_text,NULL);
		if (disc_text)
		{
			if (temp_state=create_Parse_state(disc_text))
			{
				if (GT_element_group_get_element_discretization(
					gelem_editor->edit_gt_element_group, &old_element_discretization) &&
					set_Element_discretization(temp_state,
						(void *)&element_discretization,
						(void *)gelem_editor->user_interface) &&
					((element_discretization.number_in_xi1 !=
						old_element_discretization.number_in_xi1) ||
						(element_discretization.number_in_xi2 !=
							old_element_discretization.number_in_xi2) ||
						(element_discretization.number_in_xi3 !=
							old_element_discretization.number_in_xi3)) &&
					GT_element_group_set_element_discretization(
						gelem_editor->edit_gt_element_group,
						&element_discretization, gelem_editor->user_interface))
				{
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
LAST MODIFIED : 7 March 2002

DESCRIPTION :
Called when entry is made into the circle discretization text field.
==============================================================================*/
{
	char *disc_text;
	int circle_discretization;
	struct Graphical_element_editor *gelem_editor;

	ENTER(graphical_element_editor_circle_disc_text_CB);
	USE_PARAMETER(call_data);
	/* check arguments */
	if (widget&&
		(gelem_editor=(struct Graphical_element_editor *)client_data))
	{
		/* Get the pointer to the text */
		XtVaGetValues(widget,XmNvalue,&disc_text,NULL);
		if (disc_text)
		{
			circle_discretization = atoi(disc_text);
			if ((circle_discretization != GT_element_group_get_circle_discretization(
				gelem_editor->edit_gt_element_group)) &&
				GT_element_group_set_circle_discretization(
					gelem_editor->edit_gt_element_group, circle_discretization,
					gelem_editor->user_interface))
			{
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
	struct Graphical_element_editor *gelem_editor;
	struct GT_element_group *gt_element_group;

	ENTER(graphical_element_editor_native_discretization_button_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(reason);
	if ((gelem_editor=(struct Graphical_element_editor *)gelem_editor_void)
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
	struct Graphical_element_editor *gelem_editor;

	ENTER(graphical_element_editor_update_native_discretization_field);
	USE_PARAMETER(widget);
	if (gelem_editor=(struct Graphical_element_editor *)gelem_editor_void)
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

static void graphical_element_editor_update_settings_type(Widget widget,
	XtPointer client_data, XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 November 2001

DESCRIPTION :
Called when switching between Points/Lines/Surfaces/Iso-surfaces etc.
==============================================================================*/
{
	struct Graphical_element_editor *gelem_editor;
	enum GT_element_settings_type settings_type;

	ENTER(graphical_element_editor_update_settings_type);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (gelem_editor = (struct Graphical_element_editor *)client_data)
	{
		if (STRING_TO_ENUMERATOR(GT_element_settings_type)(
			choose_enumerator_get_string(gelem_editor->settings_type_widget),
			&settings_type) &&
			(settings_type != gelem_editor->current_settings_type))
		{
			gelem_editor->current_settings_type = settings_type;
			XmForm_resize(gelem_editor->settings_type_form);
			Graphical_element_editor_set_current_settings_from_type(gelem_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_update_settings_type.  Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_update_settings_type */

static void graphical_element_editor_add_button_CB(Widget widget,
	XtPointer client_data, XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 November 2001

DESCRIPTION :
Add button press: create new settings of the current type.
==============================================================================*/
{
	enum Glyph_scaling_mode glyph_scaling_mode;
	enum Streamline_type streamline_type;
	float streamline_length,streamline_width;
	int return_code, reverse_track;
	struct Computed_field *default_coordinate_field,*element_xi_coordinate_field,
		*iso_scalar_field,*orientation_scale_field,*stream_vector_field,
		*variable_scale_field;
	struct Graphical_element_editor *gelem_editor;
	struct GROUP(FE_node) *data_group;
	struct GT_object *glyph, *old_glyph;
	struct GT_element_settings *settings;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct VT_volume_texture *volume_texture;
	Triple glyph_centre,glyph_scale_factors,glyph_size;

	ENTER(graphical_element_editor_add_button_CB);
	USE_PARAMETER(call_data);
	if (widget &&
		(gelem_editor=(struct Graphical_element_editor *)client_data) &&
		gelem_editor->edit_gt_element_group &&
		(computed_field_manager = Computed_field_package_get_computed_field_manager(
			gelem_editor->computed_field_package)))
	{
		if (settings =
			CREATE(GT_element_settings)(gelem_editor->current_settings_type))
		{
			return_code = 1;
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
				/* set materials for all settings */
				GT_element_settings_set_material(settings,
					gelem_editor->default_material);
				GT_element_settings_set_selected_material(settings,
					FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
						"default_selected",gelem_editor->graphical_material_manager));
				/* for data_points, ensure either there are points with
					 default_coordinate defined at them. If not, and any have
					 the element_xi_coordinate field defined over them, use that */
				if (GT_ELEMENT_SETTINGS_DATA_POINTS==
					gelem_editor->current_settings_type)
				{
					data_group=GT_element_group_get_data_group(
						gelem_editor->edit_gt_element_group);
					default_coordinate_field=
						GT_element_group_get_default_coordinate_field(
							gelem_editor->edit_gt_element_group);
					if (!FIRST_OBJECT_IN_GROUP_THAT(FE_node)(
						FE_node_has_Computed_field_defined,
						(void *)default_coordinate_field,data_group))
					{
						if ((element_xi_coordinate_field=
							FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
								"element_xi_coordinate",computed_field_manager))&&
							FIRST_OBJECT_IN_GROUP_THAT(FE_node)(
								FE_node_has_Computed_field_defined,
								(void *)element_xi_coordinate_field,data_group))
						{
							GT_element_settings_set_coordinate_field(settings,
								element_xi_coordinate_field);
						}
					}
				}
				/* set iso_scalar_field for iso_surfaces */
				if (GT_ELEMENT_SETTINGS_ISO_SURFACES==
					gelem_editor->current_settings_type)
				{
					if (iso_scalar_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
						Computed_field_is_scalar,(void *)NULL,computed_field_manager))
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
						&old_glyph, &glyph_scaling_mode ,glyph_centre, glyph_size,
						&orientation_scale_field, glyph_scale_factors,
						&variable_scale_field) &&
						GT_element_settings_set_glyph_parameters(settings,glyph,
							glyph_scaling_mode, glyph_centre, glyph_size,
							orientation_scale_field, glyph_scale_factors,
							variable_scale_field)))
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
							computed_field_manager))
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
			/* set use_element_type for element_points */
			if (return_code && (GT_ELEMENT_SETTINGS_ELEMENT_POINTS ==
				gelem_editor->current_settings_type))
			{
				GT_element_settings_set_use_element_type(settings,USE_ELEMENTS);
			}
			if (return_code && GT_element_group_add_settings(
				gelem_editor->edit_gt_element_group, settings, 0))
			{
				Graphical_element_editor_update_settings_list(gelem_editor);
				Graphical_element_editor_set_current_settings(gelem_editor, settings);
				/* inform the client of the changes */
				graphical_element_editor_update(gelem_editor);
			}
			if (!return_code)
			{
				DESTROY(GT_element_settings)(&settings);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_add_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_add_button_CB */

static void graphical_element_editor_delete_button_CB(Widget widget,
	XtPointer client_data, XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 November 2001

DESCRIPTION :
Delete button press: delete current_settings.
==============================================================================*/
{
	int position;
	struct Graphical_element_editor *gelem_editor;

	ENTER(graphical_element_editor_delete_button_CB);
	USE_PARAMETER(call_data);
	if (widget &&
		(gelem_editor=(struct Graphical_element_editor *)client_data) &&
		gelem_editor->edit_gt_element_group)
	{
		position = GT_element_group_get_settings_position(
			gelem_editor->edit_gt_element_group, gelem_editor->current_settings);
		GT_element_group_remove_settings(
			gelem_editor->edit_gt_element_group, gelem_editor->current_settings);
		Graphical_element_editor_update_settings_list(gelem_editor);
		Graphical_element_editor_set_current_settings_from_position(gelem_editor,
			position);
		/* inform the client of the changes */
		graphical_element_editor_update(gelem_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_delete_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_delete_button_CB */

static void graphical_element_editor_up_button_CB(Widget widget,
	XtPointer client_data, XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 November 2001

DESCRIPTION :
Up button press: decrease position of current_settings by 1.
==============================================================================*/
{
	int position;
	struct Graphical_element_editor *gelem_editor;
	struct GT_element_settings *settings;

	ENTER(graphical_element_editor_up_button_CB);
	USE_PARAMETER(call_data);
	if (widget &&
		(gelem_editor=(struct Graphical_element_editor *)client_data) &&
		gelem_editor->edit_gt_element_group)
	{
		if (1 < (position = GT_element_group_get_settings_position(
			gelem_editor->edit_gt_element_group, gelem_editor->current_settings)))
		{
			settings = gelem_editor->current_settings;
			ACCESS(GT_element_settings)(settings);
			GT_element_group_remove_settings(gelem_editor->edit_gt_element_group,
				gelem_editor->current_settings);
			GT_element_group_add_settings(gelem_editor->edit_gt_element_group,
				gelem_editor->current_settings, position - 1);
			DEACCESS(GT_element_settings)(&settings);
			Graphical_element_editor_update_settings_list(gelem_editor);
			/* inform the client of the change */
			graphical_element_editor_update(gelem_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_up_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_up_button_CB */

static void graphical_element_editor_down_button_CB(Widget widget,
	XtPointer client_data, XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 November 2001

DESCRIPTION :
Down button press: increase position of current_settings by 1.
==============================================================================*/
{
	int position;
	struct Graphical_element_editor *gelem_editor;
	struct GT_element_settings *settings;

	ENTER(graphical_element_editor_down_button_CB);
	USE_PARAMETER(call_data);
	if (widget &&
		(gelem_editor=(struct Graphical_element_editor *)client_data) &&
		gelem_editor->edit_gt_element_group)
	{
		if (GT_element_group_get_number_of_settings(
			gelem_editor->edit_gt_element_group) >
			(position = GT_element_group_get_settings_position(
				gelem_editor->edit_gt_element_group, gelem_editor->current_settings)))
		{
			settings = gelem_editor->current_settings;
			ACCESS(GT_element_settings)(settings);
			GT_element_group_remove_settings(gelem_editor->edit_gt_element_group,
				gelem_editor->current_settings);
			GT_element_group_add_settings(gelem_editor->edit_gt_element_group,
				gelem_editor->current_settings, position + 1);
			DEACCESS(GT_element_settings)(&settings);
			Graphical_element_editor_update_settings_list(gelem_editor);
			/* inform the client of the change */
			graphical_element_editor_update(gelem_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_down_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_down_button_CB */

static void graphical_element_editor_update_settings(
	Widget surface_settings_editor_widget,
	void *gelem_editor_void,void *settings_void)
/*******************************************************************************
LAST MODIFIED : 11 December 2001

DESCRIPTION :
Callback for when changes are made in the settings editor.
==============================================================================*/
{
	int visibility;
	struct Graphical_element_editor *gelem_editor;
	struct GT_element_settings *settings;
	struct Settings_item *settings_item;
	Widget previous_widget;

	ENTER(graphical_element_editor_update_settings);
	USE_PARAMETER(surface_settings_editor_widget);
	if ((gelem_editor=
			(struct Graphical_element_editor *)gelem_editor_void)
		&&(settings=(struct GT_element_settings *)settings_void))
	{
		/* keep visibility of current_settings */
		visibility=
			GT_element_settings_get_visibility(gelem_editor->current_settings);
		GT_element_group_modify_settings(gelem_editor->edit_gt_element_group,
			gelem_editor->current_settings,settings);
		GT_element_settings_set_visibility(gelem_editor->current_settings,
			visibility);
		if (settings_item = FIND_BY_IDENTIFIER_IN_LIST(Settings_item, settings)(
			gelem_editor->current_settings, gelem_editor->settings_item_list))
		{
			previous_widget = settings_item->previous_widget;
			Graphical_element_editor_update_Settings_item(settings_item->gelem_editor,
				settings_item->settings, &previous_widget);
		}
		/* inform the client of the changes */
		graphical_element_editor_update(gelem_editor);
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
LAST MODIFIED : 19 November 2001

DESCRIPTION :
Something has changed globally in the material manager. If the event has
changed the name of a material, must remake the menu.
==============================================================================*/
{
	struct Graphical_element_editor *gelem_editor;

	ENTER(graphical_element_editor_material_manager_message);
	/* checking arguments */
	if (message&&(gelem_editor=(struct Graphical_element_editor *)data))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_IDENTIFIER(Graphical_material):
			case MANAGER_CHANGE_OBJECT(Graphical_material):
			{
				Graphical_element_editor_update_settings_list(gelem_editor);
			} break;
			case MANAGER_CHANGE_ADD(Graphical_material):
			case MANAGER_CHANGE_REMOVE(Graphical_material):
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
LAST MODIFIED : 19 November 2001

DESCRIPTION :
Something has changed globally in the spectrum manager. If the event has
changed the name of a spectrum, must remake the menu.
==============================================================================*/
{
	struct Graphical_element_editor *gelem_editor;

	ENTER(graphical_element_editor_spectrum_manager_message);
	/* checking arguments */
	if (message&&(gelem_editor=(struct Graphical_element_editor *)data))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_IDENTIFIER(Spectrum):
			case MANAGER_CHANGE_OBJECT(Spectrum):
			{
				Graphical_element_editor_update_settings_list(gelem_editor);
			} break;
			case MANAGER_CHANGE_ADD(Spectrum):
			case MANAGER_CHANGE_REMOVE(Spectrum):
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
LAST MODIFIED : 22 January 2002

DESCRIPTION :
Creates a graphical_element_editor widget.
==============================================================================*/
{
	char **valid_strings;
	int init_widgets,number_of_valid_strings;
	MrmType graphical_element_editor_dialog_class;
	struct Callback_data callback;
	struct Graphical_element_editor *gelem_editor=NULL;
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
		{"gelem_ed_id_settings_list_form",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor,settings_list_form)},
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
		{"gelem_ed_settings_visibility_CB",(XtPointer)
			graphical_element_editor_settings_visibility_CB},
		{"gelem_ed_settings_select_CB",(XtPointer)
			graphical_element_editor_settings_select_CB},
		{"gelem_ed_add_btn_CB",(XtPointer)
			graphical_element_editor_add_button_CB},
		{"gelem_ed_delete_btn_CB",(XtPointer)
			graphical_element_editor_delete_button_CB},
		{"gelem_ed_up_btn_CB",(XtPointer)
			graphical_element_editor_up_button_CB},
		{"gelem_ed_down_btn_CB",(XtPointer)
			graphical_element_editor_down_button_CB}
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
			if (ALLOCATE(gelem_editor, struct Graphical_element_editor, 1) &&
				(gelem_editor->settings_item_list = CREATE(LIST(Settings_item))()))
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
				gelem_editor->current_settings_type = GT_ELEMENT_SETTINGS_LINES;
				gelem_editor->current_settings = (struct GT_element_settings *)NULL;
				gelem_editor->settings_type_widget=(Widget)NULL;
				gelem_editor->general_settings_expanded = 0;
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
				gelem_editor->settings_type_form=(Widget)NULL;
				gelem_editor->add_button=(Widget)NULL;
				gelem_editor->delete_button=(Widget)NULL;
				gelem_editor->up_button=(Widget)NULL;
				gelem_editor->down_button=(Widget)NULL;
				gelem_editor->settings_scroll=(Widget)NULL;
				gelem_editor->settings_list_form=(Widget)NULL;
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
								Computed_field_has_up_to_3_numerical_components, (void *)NULL,
								user_interface)))
							{
								init_widgets=0;
							}
							/* create chooser for native_discretization_field */
							if (!(gelem_editor->native_discretization_field_widget=
								CREATE_CHOOSE_OBJECT_WIDGET(FE_field)(
								gelem_editor->native_discretization_field_form,
								(struct FE_field *)NULL,fe_field_manager,
								(MANAGER_CONDITIONAL_FUNCTION(FE_field) *)NULL, (void *)NULL,
								user_interface)))
							{
								init_widgets=0;
							}
							/* create chooser for settings_type enumeration */
							valid_strings =
								ENUMERATOR_GET_VALID_STRINGS(GT_element_settings_type)(
									&number_of_valid_strings, (ENUMERATOR_CONDITIONAL_FUNCTION(
										GT_element_settings_type) *)NULL, (void *)NULL);
							if (!(gelem_editor->settings_type_widget=
								create_choose_enumerator_widget(
									gelem_editor->settings_type_form,
									number_of_valid_strings,valid_strings,
									ENUMERATOR_STRING(GT_element_settings_type)(
										GT_ELEMENT_SETTINGS_LINES), user_interface)))
							{
								init_widgets = 0;
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
								Pixel pixel;
								Widget clip_window = (Widget)NULL;

								/* copy background from settings_rowcol into settings_scroll */
								XtVaGetValues(gelem_editor->settings_list_form,
									XmNbackground, &pixel, NULL);
								XtVaGetValues(gelem_editor->settings_scroll,
									XmNclipWindow, &clip_window, NULL);
								XtVaSetValues(clip_window, XmNbackground, pixel, NULL);

								/* get foreground/background colours for inverting to highlight
									 currently selected object */
								XtVaGetValues(gelem_editor->settings_list_form, XmNbackground,
									&(gelem_editor->select_background_color),NULL);
								XtVaGetValues(gelem_editor->settings_list_form, XmNforeground,
									&(gelem_editor->select_foreground_color),NULL);

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
				DEALLOCATE(gelem_editor);
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
	struct Graphical_element_editor *gelem_editor;

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
	struct Graphical_element_editor *gelem_editor;

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
	struct Graphical_element_editor *gelem_editor;

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
LAST MODIFIED : 20 November 2001

DESCRIPTION :
Sets the gt_element_group to be edited by the graphical_element_editor widget.
==============================================================================*/
{
	int return_code;
	struct Callback_data callback;
	struct Graphical_element_editor *gelem_editor;
	struct GT_element_group *edit_gt_element_group;

	ENTER(graphical_element_editor_set_gt_element_group);
	if (graphical_element_editor_widget)
	{
		/* Get the pointer to the data for the graphical_element_editor_widget */
		XtVaGetValues(graphical_element_editor_widget,XmNuserData,&gelem_editor,
			NULL);
		if (gelem_editor)
		{
			return_code = 1;
			if (gt_element_group)
			{
				edit_gt_element_group =
					create_editor_copy_GT_element_group(gt_element_group);
				if (!edit_gt_element_group)
				{
					display_message(ERROR_MESSAGE,
						"graphical_element_editor_set_gt_element_group.  "
						"Could not copy graphical element");
					return_code = 0;
				}
			}
			else
			{
				edit_gt_element_group = (struct GT_element_group *)NULL;
			}
			REACCESS(GT_element_group)(&(gelem_editor->edit_gt_element_group),
				edit_gt_element_group);

			if (edit_gt_element_group)
			{
				gelem_editor_set_general_settings(gelem_editor);
				Graphical_element_editor_update_settings_list(gelem_editor);
				/* select the first settings item in the list of the current type */
				Graphical_element_editor_set_current_settings_from_type(
					gelem_editor);
				XtManageChild(gelem_editor->widget);

				/* turn on callbacks from settings editor */
				callback.procedure = graphical_element_editor_update_settings;
				callback.data = (void *)gelem_editor;
				settings_editor_set_callback(gelem_editor->settings_widget, &callback);
				/* register for any material changes */
				if (!gelem_editor->material_manager_callback_id)
				{
					gelem_editor->material_manager_callback_id =
						MANAGER_REGISTER(Graphical_material)(
							graphical_element_editor_material_manager_message,
							(void *)gelem_editor, gelem_editor->graphical_material_manager);
				}
				/* register for any material changes */
				if (!gelem_editor->spectrum_manager_callback_id)
				{
					gelem_editor->spectrum_manager_callback_id =
						MANAGER_REGISTER(Spectrum)(
							graphical_element_editor_spectrum_manager_message,
							(void *)gelem_editor, gelem_editor->spectrum_manager);
				}
			}
			else
			{
				/* turn off settings editor by passing NULL settings */
				Graphical_element_editor_set_current_settings(gelem_editor,
					(struct GT_element_settings *)NULL);
				XtUnmanageChild(gelem_editor->widget);
				/* deregister material manager callbacks */
				if (gelem_editor->material_manager_callback_id)
				{
					MANAGER_DEREGISTER(Graphical_material)(
						gelem_editor->material_manager_callback_id,
						gelem_editor->graphical_material_manager);
					gelem_editor->material_manager_callback_id = (void *)NULL;
				}
				/* deregister spectrum manager callbacks */
				if (gelem_editor->spectrum_manager_callback_id)
				{
					MANAGER_DEREGISTER(Spectrum)(
						gelem_editor->spectrum_manager_callback_id,
						gelem_editor->spectrum_manager);
					gelem_editor->spectrum_manager_callback_id = (void *)NULL;
				}
				/* turn off callbacks from settings editors */
				callback.procedure = (Callback_procedure *)NULL;
				callback.data = (void *)NULL;
				settings_editor_set_callback(gelem_editor->settings_widget, &callback);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"graphical_element_editor_set_gt_element_group.  "
				"Missing graphical_element_editor struct");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_set_gt_element_group.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* graphical_element_editor_set_gt_element_group */
