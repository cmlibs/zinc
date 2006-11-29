/*******************************************************************************
FILE : element_tool.c

LAST MODIFIED : 20 March 2003

DESCRIPTION :
Interactive tool for selecting elements with mouse and other devices.
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
#if defined (MOTIF)
#include <Xm/Protocols.h>
#include <Xm/MwmUtil.h>
#include <Xm/Xm.h>
#include <Xm/ToggleBG.h>
#include "choose/choose_computed_field.h"
#endif /* defined (MOTIF) */
#include "command/command.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "element/element_operations.h"
#include "element/element_tool.h"
#if defined (MOTIF)
static char element_tool_uidh[] =
#include "element/element_tool.uidh"
	;
#endif /* defined (MOTIF) */
#include "finite_element/finite_element_discretization.h"
#include "finite_element/finite_element_region.h"
#include "general/debug.h"
#include "graphics/scene.h"
#include "help/help_interface.h"
#include "interaction/interaction_graphics.h"
#include "interaction/interaction_volume.h"
#include "interaction/interactive_event.h"
#if defined (MOTIF)
#include "motif/image_utilities.h"
#endif /* defined (MOTIF) */
#include "region/cmiss_region.h"
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"

/*
Module variables
----------------
*/

#if defined (MOTIF)
static int element_tool_hierarchy_open=0;
static MrmHierarchy element_tool_hierarchy;
#endif /* defined (MOTIF) */

static char Interactive_tool_element_type_string[] = "element_tool";


/*
Module types
------------
*/

struct Element_tool
/*******************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Object storing all the parameters for interactively selecting elements.
==============================================================================*/
{
	struct Execute_command *execute_command;
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct Interactive_tool *interactive_tool;
	/* needed for destroy button */
	struct Cmiss_region *region;
	struct FE_element_selection *element_selection;
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct Computed_field_package *computed_field_package;
	struct Graphical_material *rubber_band_material;
	struct Time_keeper *time_keeper;
	struct User_interface *user_interface;
	/* user-settable flags */
	int select_elements_enabled,select_faces_enabled,select_lines_enabled;
	struct Computed_field *command_field;
	/* information about picked element */
	int picked_element_was_unselected;
	int motion_detected;
	struct FE_element *last_picked_element;
	struct Interaction_volume *last_interaction_volume;
	struct GT_object *rubber_band;

#if defined (MOTIF)
	Display *display;
	Widget select_elements_button,select_faces_button,select_lines_button,
		command_field_button,command_field_form,command_field_widget;
	Widget widget,window_shell;
#endif /* defined (MOTIF) */
}; /* struct Element_tool */

/*
Module functions
----------------
*/

#if defined (MOTIF)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_tool,Element_tool, \
	select_elements_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_tool,Element_tool, \
	select_faces_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_tool,Element_tool, \
	select_lines_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_tool,Element_tool,command_field_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_tool,Element_tool,command_field_form)

static void Element_tool_close_CB(Widget widget,void *element_tool_void,
	void *call_data)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Callback when "close" is selected from the window menu, or it is double
clicked. How this is made to occur is as follows. The dialog has its
XmNdeleteResponse == XmDO_NOTHING, and a window manager protocol callback for
WM_DELETE_WINDOW has been set up with XmAddWMProtocolCallback to call this
function in response to the close command. See CREATE for more details.
Function pops down dialog as a response,
==============================================================================*/
{
	struct Element_tool *element_tool;

	ENTER(Element_tool_close_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (element_tool=(struct Element_tool *)element_tool_void)
	{
		XtPopdown(element_tool->window_shell);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_close_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_tool_close_CB */

static void Element_tool_select_elements_button_CB(Widget widget,
	void *element_tool_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Callback from toggle button controlling whether top-level & 3-D elements can be
selected.
==============================================================================*/
{
	struct Element_tool *element_tool;

	ENTER(Element_tool_select_elements_button_CB);
	USE_PARAMETER(call_data);
	if (element_tool=(struct Element_tool *)element_tool_void)
	{
		Element_tool_set_select_elements_enabled(element_tool,
			XmToggleButtonGadgetGetState(widget));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_select_elements_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_tool_select_elements_button_CB */

static void Element_tool_select_faces_button_CB(Widget widget,
	void *element_tool_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Callback from toggle button controlling whether face & 2-D top-level elements
can be selected.
==============================================================================*/
{
	struct Element_tool *element_tool;

	ENTER(Element_tool_select_faces_button_CB);
	USE_PARAMETER(call_data);
	if (element_tool=(struct Element_tool *)element_tool_void)
	{
		Element_tool_set_select_faces_enabled(element_tool,
			XmToggleButtonGadgetGetState(widget));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_select_faces_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_tool_select_faces_button_CB */

static void Element_tool_select_lines_button_CB(Widget widget,
	void *element_tool_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Callback from toggle button controlling whether line & 1-D top-level elements
can be selected.
==============================================================================*/
{
	struct Element_tool *element_tool;

	ENTER(Element_tool_select_lines_button_CB);
	USE_PARAMETER(call_data);
	if (element_tool=(struct Element_tool *)element_tool_void)
	{
		Element_tool_set_select_lines_enabled(element_tool,
			XmToggleButtonGadgetGetState(widget));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_select_lines_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_tool_select_lines_button_CB */

static void Element_tool_command_field_button_CB(Widget widget,
	void *element_tool_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Callback from toggle button enabling a command_field to be selected.
==============================================================================*/
{
	struct Computed_field *command_field;
	struct Element_tool *element_tool;

	ENTER(Element_tool_command_field_button_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (element_tool=(struct Element_tool *)element_tool_void)
	{
		if (Element_tool_get_command_field(element_tool))
		{
			Element_tool_set_command_field(element_tool, (struct Computed_field *)NULL);
		}
		else
		{
			/* get label field from widget */
			command_field = CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
				element_tool->command_field_widget);
			if (command_field)
			{
				Element_tool_set_command_field(element_tool, command_field);
			}
			else
			{
				XtVaSetValues(element_tool->command_field_button, XmNset, False, NULL);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_command_field_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_tool_command_field_button_CB */

static void Element_tool_update_command_field(Widget widget,
	void *element_tool_void, void *command_field_void)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Callback for change of command_field.
==============================================================================*/
{
	struct Element_tool *element_tool;

	ENTER(Element_tool_update_command_field);
	USE_PARAMETER(widget);
	if (element_tool = (struct Element_tool *)element_tool_void)
	{
		/* skip messages from chooser if it is grayed out */
		if (XtIsSensitive(element_tool->command_field_widget))
		{
			Element_tool_set_command_field(element_tool,
				(struct Computed_field *)command_field_void);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_update_command_field.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_tool_update_command_field */

static void Element_tool_destroy_selected_CB(Widget widget,
	void *element_tool_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Attempts to destroy all the top-level elements currently in the global
selection. ???RC could change to allow faces and lines to be destroyed, but
need other safeguard controls before allowing this.
==============================================================================*/
{
	int number_not_destroyed;
	struct Element_tool *element_tool;
	struct FE_region *fe_region;
	struct LIST(FE_element) *destroy_element_list;

	ENTER(Element_tool_destroy_selected_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (element_tool = (struct Element_tool *)element_tool_void)
	{
		if (destroy_element_list = CREATE(LIST(FE_element))())
		{
			COPY_LIST(FE_element)(destroy_element_list,
				FE_element_selection_get_element_list(
					element_tool->element_selection));
			fe_region = Cmiss_region_get_FE_region(element_tool->region);
			FE_region_begin_change(fe_region);
			FE_region_remove_FE_element_list(fe_region, destroy_element_list);
			if (0 < (number_not_destroyed =
				NUMBER_IN_LIST(FE_element)(destroy_element_list)))
			{
				display_message(WARNING_MESSAGE,
					"%d of the selected element(s) could not be destroyed",
					number_not_destroyed);
			}
			FE_region_end_change(fe_region);
			DESTROY(LIST(FE_element))(&destroy_element_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_destroy_selected_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_tool_destroy_selected_CB */
#endif /* defined (MOTIF) */

static void Element_tool_interactive_event_handler(void *device_id,
	struct Interactive_event *event,void *element_tool_void,
	struct Graphics_buffer *graphics_buffer)
/*******************************************************************************
LAST MODIFIED  18 November 2005

DESCRIPTION :
Input handler for input from devices. <device_id> is a unique address enabling
the editor to handle input from more than one device at a time. The <event>
describes the type of event, button numbers and key modifiers, and the volume
of space affected by the interaction. Main events are button press, movement and
release.
==============================================================================*/
{
	char *command_string;
	enum Interactive_event_type event_type;
	FE_value time, xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int clear_selection, element_dimension, i, input_modifier,
		number_of_xi_points, shift_pressed;
	int number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct FE_element *picked_element;
	struct FE_element_shape *element_shape;
	struct Element_tool *element_tool;
	struct Interaction_volume *interaction_volume,*temp_interaction_volume;
	struct LIST(FE_element) *element_list;
	struct LIST(Scene_picked_object) *scene_picked_object_list;
	struct Scene *scene;
	Triple *xi_points;

	ENTER(Element_tool_interactive_event_handler);
	if (device_id&&event&&(element_tool=
		(struct Element_tool *)element_tool_void))
	{
		interaction_volume=Interactive_event_get_interaction_volume(event);
		if (scene=Interactive_event_get_scene(event))
		{
			event_type=Interactive_event_get_type(event);
			input_modifier=Interactive_event_get_input_modifier(event);
			shift_pressed=(INTERACTIVE_EVENT_MODIFIER_SHIFT & input_modifier);
			switch (event_type)
			{
				case INTERACTIVE_EVENT_BUTTON_PRESS:
				{
					/* interaction only works with first mouse button */
					if (1==Interactive_event_get_button_number(event))
					{
						if (scene_picked_object_list=
							Scene_pick_objects(scene,interaction_volume,graphics_buffer))
						{
							element_tool->picked_element_was_unselected=0;
							if (picked_element=Scene_picked_object_list_get_nearest_element(
								scene_picked_object_list,(struct Cmiss_region *)NULL,
								element_tool->select_elements_enabled,
								element_tool->select_faces_enabled,
								element_tool->select_lines_enabled,
								(struct Scene_picked_object **)NULL,
								(struct GT_element_group **)NULL,
								(struct GT_element_settings **)NULL))
							{
								/* Open command_field of picked_element in browser */
								if (element_tool->command_field)
								{
									if (Computed_field_is_defined_in_element(element_tool->command_field,
										picked_element))
									{
										if (element_tool->time_keeper)
										{
											time = Time_keeper_get_time(element_tool->time_keeper);
										}
										else
										{
											time = 0;
										}
										/* since we don't really have fields constant over an
											 element, evaluate at its centre */
										element_dimension =
											get_FE_element_dimension(picked_element);
										for (i = 0; i < element_dimension; i++)
										{
											number_in_xi[i] = 1;
										}
										get_FE_element_shape(picked_element, &element_shape);
										if (FE_element_shape_get_xi_points_cell_centres(
											element_shape, number_in_xi,
											&number_of_xi_points, &xi_points))
										{
											/*???debug*/printf("element_tool: xi =");
											for (i = 0; i < element_dimension; i++)
											{
												xi[i] = xi_points[0][i];
												/*???debug*/printf(" %g",xi[i]);
											}
											/*???debug*/printf("\n");
											if (command_string =
												Computed_field_evaluate_as_string_in_element(
													element_tool->command_field, /*component_number*/-1,
													picked_element, xi, time, (struct FE_element *)NULL))
											{
												Execute_command_execute_string(element_tool->execute_command,
													command_string);
												DEALLOCATE(command_string);
											}
											DEALLOCATE(xi_points);
										}
									}
								}
								if (!FE_element_selection_is_element_selected(
									element_tool->element_selection,picked_element))
								{
									element_tool->picked_element_was_unselected=1;
								}
							}
							REACCESS(FE_element)(&(element_tool->last_picked_element),
								picked_element);
							if (clear_selection = !shift_pressed)
#if defined (OLD_CODE)
								&&((!picked_element)||
									(element_tool->picked_element_was_unselected))))
#endif /*defined (OLD_CODE) */
							{
								FE_element_selection_begin_cache(
									element_tool->element_selection);
								FE_element_selection_clear(element_tool->element_selection);
							}
							if (picked_element)
							{
								FE_element_selection_select_element(
									element_tool->element_selection,picked_element);
							}
							if (clear_selection)
							{
								FE_element_selection_end_cache(element_tool->element_selection);
							}
							DESTROY(LIST(Scene_picked_object))(&(scene_picked_object_list));
						}
						element_tool->motion_detected=0;
						REACCESS(Interaction_volume)(
							&(element_tool->last_interaction_volume),interaction_volume);
					}
				} break;
				case INTERACTIVE_EVENT_MOTION_NOTIFY:
				case INTERACTIVE_EVENT_BUTTON_RELEASE:
				{
					if (element_tool->last_interaction_volume&&
						((INTERACTIVE_EVENT_MOTION_NOTIFY==event_type) ||
						(1==Interactive_event_get_button_number(event))))
					{
						if (INTERACTIVE_EVENT_MOTION_NOTIFY==event_type)
						{
							element_tool->motion_detected=1;
						}
						if (element_tool->last_picked_element)
						{
							/* unselect last_picked_element if not just added */
							if ((INTERACTIVE_EVENT_BUTTON_RELEASE==event_type)&&
								shift_pressed&&(!(element_tool->picked_element_was_unselected)))
							{
								FE_element_selection_unselect_element(
									element_tool->element_selection,
									element_tool->last_picked_element);
							}
						}
						else if (element_tool->motion_detected)
						{
							/* rubber band select */
							if (temp_interaction_volume=
								create_Interaction_volume_bounding_box(
								element_tool->last_interaction_volume,interaction_volume))
							{
								if (INTERACTIVE_EVENT_MOTION_NOTIFY==event_type)
								{
									if (!element_tool->rubber_band)
									{
										/* create rubber_band object and put in scene */
										element_tool->rubber_band=CREATE(GT_object)(
											"element_tool_rubber_band",g_POLYLINE,
											element_tool->rubber_band_material);
										ACCESS(GT_object)(element_tool->rubber_band);
										Scene_add_graphics_object(scene,element_tool->rubber_band,
											/*position*/0,"element_tool_rubber_band",
											/*fast_changing*/1);
									}
									Interaction_volume_make_polyline_extents(
										temp_interaction_volume,element_tool->rubber_band);
								}
								else
								{
									Scene_remove_graphics_object(scene,element_tool->rubber_band);
									DEACCESS(GT_object)(&(element_tool->rubber_band));
								}
								if (INTERACTIVE_EVENT_BUTTON_RELEASE==event_type)
								{
									if (scene_picked_object_list=
										Scene_pick_objects(scene,temp_interaction_volume,
										graphics_buffer))
									{
										if (element_list=
											Scene_picked_object_list_get_picked_elements(
												scene_picked_object_list,
												element_tool->select_elements_enabled,
												element_tool->select_faces_enabled,
												element_tool->select_lines_enabled))
										{
											FE_element_selection_begin_cache(
												element_tool->element_selection);
											FOR_EACH_OBJECT_IN_LIST(FE_element)(
												FE_element_select_in_FE_element_selection,
												(void *)element_tool->element_selection,element_list);
											FE_element_selection_end_cache(
												element_tool->element_selection);
											DESTROY(LIST(FE_element))(&element_list);
										}
										DESTROY(LIST(Scene_picked_object))(
											&(scene_picked_object_list));
									}
								}
								DESTROY(Interaction_volume)(&temp_interaction_volume);
							}
						}
						if (INTERACTIVE_EVENT_BUTTON_RELEASE==event_type)
						{
							REACCESS(FE_element)(&(element_tool->last_picked_element),
								(struct FE_element *)NULL);
							REACCESS(Interaction_volume)(
								&(element_tool->last_interaction_volume),
								(struct Interaction_volume *)NULL);
						}
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Element_tool_interactive_event_handler.  Unknown event type");
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_interactive_event_handler.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_tool_interactive_event_handler */

static int Element_tool_bring_up_interactive_tool_dialog(
	void *element_tool_void)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Brings up a dialog for editing settings of the Element_tool - in a standard
format for passing to an Interactive_toolbar.
==============================================================================*/
{
	int return_code;

	ENTER(Element_tool_bring_up_interactive_tool_dialog);
	return_code =
		Element_tool_pop_up_dialog((struct Element_tool *)element_tool_void);
	LEAVE;

	return (return_code);
} /* Element_tool_bring_up_interactive_tool_dialog */

static struct Cmgui_image *Element_tool_get_icon(struct Colour *foreground, 
	struct Colour *background, void *element_tool_void)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Fetches a ToggleButton with an appropriate icon for the interactive tool
and as a child of <parent>.
==============================================================================*/
{
#if defined (MOTIF)
	Display *display;
	Pixel background_pixel, foreground_pixel;
	Pixmap pixmap;
#endif /* defined (MOTIF) */
	struct Cmgui_image *image;
	struct Element_tool *element_tool;

	ENTER(Element_tool_get_icon);
	if ((element_tool=(struct Element_tool *)element_tool_void))
	{
#if defined (MOTIF)
		if (MrmOpenHierarchy_binary_string(element_tool_uidh,sizeof(element_tool_uidh),
			&element_tool_hierarchy,&element_tool_hierarchy_open))
		{
			display = element_tool->display;
			convert_Colour_to_Pixel(display, foreground, &foreground_pixel);
			convert_Colour_to_Pixel(display, background, &background_pixel);
			if (MrmSUCCESS == MrmFetchIconLiteral(element_tool_hierarchy,
				"element_tool_icon",DefaultScreenOfDisplay(display),display,
				foreground_pixel, background_pixel, &pixmap))
			{ 
				image = create_Cmgui_image_from_Pixmap(display, pixmap);
			}
			else
			{
				display_message(WARNING_MESSAGE, "Element_tool_get_icon.  "
					"Could not fetch widget");
				image = (struct Cmgui_image *)NULL;
			}			
		}
		else
		{
			display_message(WARNING_MESSAGE, "Element_tool_get_icon.  "
				"Could not open heirarchy");
			image = (struct Cmgui_image *)NULL;
		}
#else /* defined (MOTIF) */
		USE_PARAMETER(foreground);
		USE_PARAMETER(background);
		USE_PARAMETER(element_tool);
		display_message(WARNING_MESSAGE, "Element_tool_get_icon.  "
			"Not implemented for this user interface.");
#endif /* defined (MOTIF) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_get_icon.  Invalid argument(s)");
		image = (struct Cmgui_image *)NULL;
	}
	LEAVE;

	return (image);
} /* Element_tool_get_icon */

/*
Global functions
----------------
*/

struct Element_tool *CREATE(Element_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct Cmiss_region *region,
	struct FE_element_selection *element_selection,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Computed_field_package *computed_field_package,
	struct Graphical_material *rubber_band_material,
	struct User_interface *user_interface,
	struct Time_keeper *time_keeper,
	struct Execute_command *execute_command)
/*******************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Creates an Element_tool with Interactive_tool in <interactive_tool_manager>.
Selects elements in <element_selection> in response to interactive_events.
==============================================================================*/
{
#if defined (MOTIF)
	Atom WM_DELETE_WINDOW;
	int init_widgets;
	MrmType element_tool_dialog_class;
	static MrmRegisterArg callback_list[]=
	{
		{"elem_tool_id_select_elems_btn",(XtPointer)
			DIALOG_IDENTIFY(element_tool,select_elements_button)},
		{"elem_tool_id_select_faces_btn",(XtPointer)
			DIALOG_IDENTIFY(element_tool,select_faces_button)},
		{"elem_tool_id_select_lines_btn",(XtPointer)
			DIALOG_IDENTIFY(element_tool,select_lines_button)},
		{"elem_tool_id_command_field_btn",(XtPointer)
			DIALOG_IDENTIFY(element_tool,command_field_button)},
		{"elem_tool_id_command_field_form",(XtPointer)
			DIALOG_IDENTIFY(element_tool,command_field_form)},
		{"elem_tool_select_elems_btn_CB",
		 (XtPointer)Element_tool_select_elements_button_CB},
		{"elem_tool_select_faces_btn_CB",
		 (XtPointer)Element_tool_select_faces_button_CB},
		{"elem_tool_select_lines_btn_CB",
		 (XtPointer)Element_tool_select_lines_button_CB},
		{"elem_tool_command_field_btn_CB",
		 (XtPointer)Element_tool_command_field_button_CB},
		{"elem_tool_destroy_selected_CB",
		 (XtPointer)Element_tool_destroy_selected_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"elem_tool_structure",(XtPointer)NULL}
	};
	struct Callback_data callback;
#endif /* defined (MOTIF) */
	struct Element_tool *element_tool;
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(CREATE(Element_tool));
	element_tool=(struct Element_tool *)NULL;
	if (interactive_tool_manager && region &&
		element_selection&&computed_field_package&&(computed_field_manager=
			Computed_field_package_get_computed_field_manager(computed_field_package))
		&&rubber_band_material&&user_interface&&execute_command)
	{
		if (ALLOCATE(element_tool,struct Element_tool,1))
		{
			element_tool->execute_command=execute_command;
			element_tool->interactive_tool_manager=interactive_tool_manager;
			element_tool->region = region;
			element_tool->element_selection=element_selection;
			element_tool->element_point_ranges_selection=
				element_point_ranges_selection;
			element_tool->computed_field_package=computed_field_package;
			element_tool->rubber_band_material=
				ACCESS(Graphical_material)(rubber_band_material);
			element_tool->user_interface=user_interface;
			element_tool->time_keeper = (struct Time_keeper *)NULL;
			if (time_keeper)
			{
				element_tool->time_keeper = ACCESS(Time_keeper)(time_keeper);
			}
			/* user-settable flags */
			element_tool->select_elements_enabled=1;
			element_tool->select_faces_enabled=1;
			element_tool->select_lines_enabled=1;
			element_tool->command_field = (struct Computed_field *)NULL;
			/* interactive_tool */
			element_tool->interactive_tool=CREATE(Interactive_tool)(
				"element_tool","Element tool",
				Interactive_tool_element_type_string,
				Element_tool_interactive_event_handler,
				Element_tool_get_icon,
				Element_tool_bring_up_interactive_tool_dialog,
				(Interactive_tool_destroy_tool_data_function *)NULL,
				(void *)element_tool);
			ADD_OBJECT_TO_MANAGER(Interactive_tool)(
				element_tool->interactive_tool,
				element_tool->interactive_tool_manager);
			element_tool->last_picked_element=(struct FE_element *)NULL;
			element_tool->last_interaction_volume=(struct Interaction_volume *)NULL;
			element_tool->rubber_band=(struct GT_object *)NULL;

#if defined (MOTIF)
			element_tool->display = User_interface_get_display(user_interface);

			/* initialise widgets */
			element_tool->select_elements_button=(Widget)NULL;
			element_tool->select_faces_button=(Widget)NULL;
			element_tool->select_lines_button=(Widget)NULL;
			element_tool->command_field_button=(Widget)NULL;
			element_tool->command_field_form=(Widget)NULL;
			element_tool->command_field_widget=(Widget)NULL;
			element_tool->widget=(Widget)NULL;
			element_tool->window_shell=(Widget)NULL;

			if (MrmOpenHierarchy_binary_string(element_tool_uidh,sizeof(element_tool_uidh),
					&element_tool_hierarchy,&element_tool_hierarchy_open))
			{
				/* make the dialog shell */
				if (element_tool->window_shell=
					XtVaCreatePopupShell("Element tool",
						topLevelShellWidgetClass,
						User_interface_get_application_shell(user_interface),
						XmNdeleteResponse,XmDO_NOTHING,
						XmNmwmDecorations,MWM_DECOR_ALL,
						XmNmwmFunctions,MWM_FUNC_ALL,
						/*XmNtransient,FALSE,*/
						XmNallowShellResize,False,
						XmNtitle,"Element tool",
						NULL))
				{
					/* Set up window manager callback for close window message */
					WM_DELETE_WINDOW=XmInternAtom(XtDisplay(element_tool->window_shell),
						"WM_DELETE_WINDOW",False);
					XmAddWMProtocolCallback(element_tool->window_shell,
						WM_DELETE_WINDOW,Element_tool_close_CB,element_tool);
					/* Register the shell with the busy signal list */
					create_Shell_list_item(&(element_tool->window_shell),user_interface);
					/* register the callbacks */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
							 element_tool_hierarchy,callback_list,XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value=(XtPointer)element_tool;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
								 element_tool_hierarchy,identifier_list,XtNumber(identifier_list)))
						{
							/* fetch element tool widgets */
							if (MrmSUCCESS==MrmFetchWidget(element_tool_hierarchy,
									"element_tool",element_tool->window_shell,
									&(element_tool->widget),&element_tool_dialog_class))
							{
								init_widgets=1;
								if (element_tool->command_field_widget =
									CREATE_CHOOSE_OBJECT_WIDGET(Computed_field)(
										element_tool->command_field_form,
										element_tool->command_field, computed_field_manager,
										Computed_field_has_string_value_type,
										(void *)NULL, user_interface))
								{
									callback.data = (void *)element_tool;
									callback.procedure = Element_tool_update_command_field;
									CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
										element_tool->command_field_widget, &callback);
								}
								else
								{
									init_widgets=0;
								}
								if (init_widgets)
								{
									XmToggleButtonGadgetSetState(
										element_tool->select_elements_button,
										/*state*/element_tool->select_elements_enabled,
										/*notify*/False);
									XmToggleButtonGadgetSetState(
										element_tool->select_faces_button,
										/*state*/element_tool->select_faces_enabled,
										/*notify*/False);
									XmToggleButtonGadgetSetState(
										element_tool->select_lines_button,
										/*state*/element_tool->select_lines_enabled,
										/*notify*/False);
									XmToggleButtonGadgetSetState(element_tool->command_field_button,
										/*state*/False, /*notify*/False);
									XtSetSensitive(element_tool->command_field_widget,
										/*state*/False);
									XtManageChild(element_tool->widget);
									XtRealizeWidget(element_tool->window_shell);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"CREATE(Element_tool).  Could not init widgets");
									DESTROY(Element_tool)(&element_tool);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"CREATE(Element_tool).  Could not fetch element_tool");
								DESTROY(Element_tool)(&element_tool);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"CREATE(Element_tool).  Could not register identifiers");
							DESTROY(Element_tool)(&element_tool);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"CREATE(Element_tool).  Could not register callbacks");
						DESTROY(Element_tool)(&element_tool);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CREATE(Element_tool).  Could not create Shell");
					DESTROY(Element_tool)(&element_tool);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Element_tool).  Could not open hierarchy");
			}
#endif /* defined (MOTIF) */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Element_tool).  Not enough memory");
			DEALLOCATE(element_tool);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Element_tool).  Invalid argument(s)");
	}
	LEAVE;

	return (element_tool);
} /* CREATE(Element_tool) */

int DESTROY(Element_tool)(struct Element_tool **element_tool_address)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Frees and deaccesses objects in the <element_tool> and deallocates the
structure itself.
==============================================================================*/
{
	struct Element_tool *element_tool;
	int return_code;

	ENTER(DESTROY(Element_tool));
	if (element_tool_address&&(element_tool= *element_tool_address))
	{
		REACCESS(FE_element)(&(element_tool->last_picked_element),
			(struct FE_element *)NULL);
		REMOVE_OBJECT_FROM_MANAGER(Interactive_tool)(element_tool->interactive_tool,
			element_tool->interactive_tool_manager);
		REACCESS(Interaction_volume)(&(element_tool->last_interaction_volume),
			(struct Interaction_volume *)NULL);
		REACCESS(GT_object)(&(element_tool->rubber_band),(struct GT_object *)NULL);
		DEACCESS(Graphical_material)(&(element_tool->rubber_band_material));
		if (element_tool->time_keeper)
		{
			DEACCESS(Time_keeper)(&(element_tool->time_keeper));
		}
#if defined (MOTIF)
		if (element_tool->window_shell)
		{
			destroy_Shell_list_item_from_shell(&(element_tool->window_shell),
				element_tool->user_interface);
			XtDestroyWidget(element_tool->window_shell);
		}
#endif /* defined (MOTIF) */
		DEALLOCATE(*element_tool_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Element_tool).  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* DESTROY(Element_tool) */

int Element_tool_pop_up_dialog(struct Element_tool *element_tool)
/*******************************************************************************
LAST MODIFIED : 20 June 2001

DESCRIPTION :
Pops up a dialog for editing settings of the Element_tool.
==============================================================================*/
{
	int return_code;

	ENTER(Element_tool_pop_up_dialog);
	if (element_tool)
	{
#if defined (MOTIF)
		XtPopup(element_tool->window_shell, XtGrabNone);
		/* make sure in addition that it is not shown as an icon */
		XtVaSetValues(element_tool->window_shell, XmNiconic, False, NULL);
#else /* defined (MOTIF) */
		display_message(ERROR_MESSAGE, "Element_tool_pop_up_dialog.  "
			"No dialog implemented for this User Interface");
#endif /* defined (MOTIF) */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_pop_up_dialog.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Element_tool_pop_up_dialog */

int Element_tool_pop_down_dialog(struct Element_tool *element_tool)
/*******************************************************************************
LAST MODIFIED : 20 June 2001

DESCRIPTION :
Hides the dialog for editing settings of the Element_tool.
==============================================================================*/
{
	int return_code;

	ENTER(Element_tool_pop_down_dialog);
	if (element_tool)
	{
#if defined (MOTIF)
		XtPopdown(element_tool->window_shell);
#else /* defined (MOTIF) */
		display_message(ERROR_MESSAGE, "Element_tool_pop_down_dialog.  "
			"No dialog implemented for this User Interface");
#endif /* defined (MOTIF) */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_pop_down_dialog.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Element_tool_pop_down_dialog */

int Element_tool_get_select_elements_enabled(struct Element_tool *element_tool)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns flag controlling whether top-level & 3-D elements can be selected.
==============================================================================*/
{
	int select_elements_enabled;

	ENTER(Element_tool_get_select_elements_enabled);
	if (element_tool)
	{
		select_elements_enabled=element_tool->select_elements_enabled;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_get_select_elements_enabled.  Invalid argument(s)");
		select_elements_enabled=0;
	}
	LEAVE;

	return (select_elements_enabled);
} /* Element_tool_get_select_elements_enabled */

int Element_tool_set_select_elements_enabled(struct Element_tool *element_tool,
	int select_elements_enabled)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Sets flag controlling whether top-level & 3-D elements can be selected.
==============================================================================*/
{
#if defined (MOTIF)
	int button_state;
#endif /* defined (MOTIF) */
	int return_code;

	ENTER(Element_tool_set_select_elements_enabled);
	if (element_tool)
	{
		return_code=1;
		if (select_elements_enabled)
		{
			/* make sure value of flag is exactly 1 */
			select_elements_enabled=1;
		}
		if (select_elements_enabled != element_tool->select_elements_enabled)
		{
			element_tool->select_elements_enabled=select_elements_enabled;
#if defined (MOTIF)
			/* make sure button shows current state */
			if (XmToggleButtonGadgetGetState(element_tool->select_elements_button))
			{
				button_state=1;
			}
			else
			{
				button_state=0;
			}
			if (button_state != element_tool->select_elements_enabled)
			{
				XmToggleButtonGadgetSetState(element_tool->select_elements_button,
					/*state*/element_tool->select_elements_enabled,/*notify*/False);
			}
#endif /* defined (MOTIF) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_set_select_elements_enabled.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_tool_set_select_elements_enabled */

int Element_tool_get_select_faces_enabled(struct Element_tool *element_tool)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns flag controlling whether face & 2-D top-level elements can be selected.
==============================================================================*/
{
	int select_faces_enabled;

	ENTER(Element_tool_get_select_faces_enabled);
	if (element_tool)
	{
		select_faces_enabled=element_tool->select_faces_enabled;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_get_select_faces_enabled.  Invalid argument(s)");
		select_faces_enabled=0;
	}
	LEAVE;

	return (select_faces_enabled);
} /* Element_tool_get_select_faces_enabled */

int Element_tool_set_select_faces_enabled(struct Element_tool *element_tool,
	int select_faces_enabled)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns flag controlling whether face & 2-D top-level elements can be selected.
==============================================================================*/
{
#if defined (MOTIF)
	int button_state;
#endif /* defined (MOTIF) */
	int return_code;

	ENTER(Element_tool_set_select_faces_enabled);
	if (element_tool)
	{
		return_code=1;
		if (select_faces_enabled)
		{
			/* make sure value of flag is exactly 1 */
			select_faces_enabled=1;
		}
		if (select_faces_enabled != element_tool->select_faces_enabled)
		{
			element_tool->select_faces_enabled=select_faces_enabled;
#if defined (MOTIF)
			/* make sure button shows current state */
			if (XmToggleButtonGadgetGetState(element_tool->select_faces_button))
			{
				button_state=1;
			}
			else
			{
				button_state=0;
			}
			if (button_state != element_tool->select_faces_enabled)
			{
				XmToggleButtonGadgetSetState(element_tool->select_faces_button,
					/*state*/element_tool->select_faces_enabled,/*notify*/False);
			}
#endif /* defined (MOTIF) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_set_select_faces_enabled.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_tool_set_select_faces_enabled */

int Element_tool_get_select_lines_enabled(struct Element_tool *element_tool)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns flag controlling whether line & 1-D top-level elements can be selected.
==============================================================================*/
{
	int select_lines_enabled;

	ENTER(Element_tool_get_select_lines_enabled);
	if (element_tool)
	{
		select_lines_enabled=element_tool->select_lines_enabled;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_get_select_lines_enabled.  Invalid argument(s)");
		select_lines_enabled=0;
	}
	LEAVE;

	return (select_lines_enabled);
} /* Element_tool_get_select_lines_enabled */

int Element_tool_set_select_lines_enabled(struct Element_tool *element_tool,
	int select_lines_enabled)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns flag controlling whether line & 1-D top-level elements can be selected.
==============================================================================*/
{
#if defined (MOTIF)
	int button_state;
#endif /* defined (MOTIF) */
	int return_code;

	ENTER(Element_tool_set_select_lines_enabled);
	if (element_tool)
	{
		return_code=1;
		if (select_lines_enabled)
		{
			/* make sure value of flag is exactly 1 */
			select_lines_enabled=1;
		}
		if (select_lines_enabled != element_tool->select_lines_enabled)
		{
			element_tool->select_lines_enabled=select_lines_enabled;
#if defined (MOTIF)
			/* make sure button shows current state */
			if (XmToggleButtonGadgetGetState(element_tool->select_lines_button))
			{
				button_state=1;
			}
			else
			{
				button_state=0;
			}
			if (button_state != element_tool->select_lines_enabled)
			{
				XmToggleButtonGadgetSetState(element_tool->select_lines_button,
					/*state*/element_tool->select_lines_enabled,/*notify*/False);
			}
#endif /* defined (MOTIF) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_set_select_lines_enabled.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_tool_set_select_lines_enabled */

struct Computed_field *Element_tool_get_command_field(
	struct Element_tool *element_tool)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Returns the command_field to be looked up in a web browser when the element is
clicked on in the <element_tool>.
==============================================================================*/
{
	struct Computed_field *command_field;

	ENTER(Element_tool_get_command_field);
	if (element_tool)
	{
		command_field=element_tool->command_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_get_command_field.  Invalid argument(s)");
		command_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (command_field);
} /* Element_tool_get_command_field */

int Element_tool_set_command_field(struct Element_tool *element_tool,
	struct Computed_field *command_field)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Sets the command_field to be looked up in a web browser when the element is clicked
on in the <element_tool>.
==============================================================================*/
{
#if defined (MOTIF)
	int field_set;
#endif /* defined (MOTIF) */
	int return_code;

	ENTER(Element_tool_set_command_field);
	if (element_tool && ((!command_field) ||
		Computed_field_has_string_value_type(command_field, (void *)NULL)))
	{
		return_code = 1;
		if (command_field != element_tool->command_field)
		{
			element_tool->command_field = command_field;
#if defined (MOTIF)
			if (command_field)
			{
				CHOOSE_OBJECT_SET_OBJECT(Computed_field)(
					element_tool->command_field_widget,element_tool->command_field);
			}
			field_set = ((struct Computed_field *)NULL != command_field);
			XtVaSetValues(element_tool->command_field_button,
				XmNset, field_set, NULL);
			XtSetSensitive(element_tool->command_field_widget, field_set);
#endif /* defined (MOTIF) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_set_command_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_tool_set_command_field */
