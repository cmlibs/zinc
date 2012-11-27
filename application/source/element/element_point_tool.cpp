/*******************************************************************************
FILE : element_point_tool.c

LAST MODIFIED : 5 July 2002

DESCRIPTION :
Interactive tool for selecting element/grid points with mouse and other devices.
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
#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (1) */

#include "zinc/fieldmodule.h"
#include "command/command.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "element/element_point_tool.h"
#include "finite_element/finite_element_discretization.h"
#include "general/debug.h"
#include "graphics/scene.h"
#include "interaction/interaction_graphics.h"
#include "interaction/interaction_volume.h"
#include "interaction/interactive_event.h"
#include "general/message.h"

#if defined (WX_USER_INTERFACE)
#include "wx/wx.h"
#include <wx/tglbtn.h>
#include "wx/xrc/xmlres.h"
#include "element/element_point_tool.xrch"
#include "graphics/graphics_window_private.hpp"
#include "choose/choose_manager_class.hpp"
#endif /* defined (WX_USER_INTERFACE)*/


/*
Module variables
----------------
*/

static char Interactive_tool_element_point_type_string[] = "element_point_tool";


/*
Module types
------------
*/


#if defined (WX_USER_INTERFACE)
class wxElementPointTool;
#endif /* defined (WX_USER_INTERFACE) */


struct Element_point_tool
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Object storing all the parameters for interactively selecting element points.
==============================================================================*/
{
	struct Execute_command *execute_command;
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct Interactive_tool *interactive_tool;
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct Graphical_material *rubber_band_material;
	struct Time_keeper *time_keeper;
	struct User_interface *user_interface;
	/* user settings */
	struct Computed_field *command_field;
	/* information about picked element_point_ranges */
	int picked_element_point_was_unselected;
	int motion_detected;
	struct Element_point_ranges *last_picked_element_point;
	struct Interaction_volume *last_interaction_volume;
	struct GT_object *rubber_band;
	struct Cmiss_region *region;
	//struct Graphics_window *graphics_window;

#if defined (WX_USER_INTERFACE)
	 wxElementPointTool *wx_element_point_tool;
	 wxPoint tool_position;
#endif /* defined (WX_USER_INTERFACE) */

}; /* struct Element_point_tool */

/*
Module functions
----------------
*/

static void Element_point_tool_reset(void *element_point_tool_void)
/*******************************************************************************
LAST MODIFIED : 25 February 2008

DESCRIPTION :
Resets current edit. Called on button release or when tool deactivated.
==============================================================================*/
{
	struct Element_point_tool *element_point_tool;

	ENTER(Element_point_tool_reset);
	element_point_tool = (struct Element_point_tool *)element_point_tool_void;
	if (element_point_tool != 0)
	{
		REACCESS(Element_point_ranges)(
			&(element_point_tool->last_picked_element_point),
			(struct Element_point_ranges *)NULL);
		REACCESS(Interaction_volume)(
			&(element_point_tool->last_interaction_volume),
			(struct Interaction_volume *)NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Node_tool_reset.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_tool_reset */

static void Element_point_tool_interactive_event_handler(void *device_id,
	struct Interactive_event *event,void *element_point_tool_void,
	struct Graphics_buffer *graphics_buffer)
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
Input handler for input from devices. <device_id> is a unique address enabling
the editor to handle input from more than one device at a time. The <event>
describes the type of event, button numbers and key modifiers, and the volume
of space affected by the interaction. Main events are button press, movement and
release.
==============================================================================*/
{
	enum Interactive_event_type event_type;
	FE_value time, xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int clear_selection, input_modifier, shift_pressed, start, stop;
	struct Element_point_ranges *picked_element_point;
	struct Element_point_ranges_identifier element_point_ranges_identifier;
	struct Element_point_tool *element_point_tool;
	struct Interaction_volume *interaction_volume,*temp_interaction_volume;
	struct LIST(Element_point_ranges) *element_point_ranges_list;
	struct LIST(Scene_picked_object) *scene_picked_object_list;
	struct Multi_range *multi_range;
	struct Scene *scene;

	ENTER(Element_point_tool_interactive_event_handler);
	if (device_id&&event&&(element_point_tool=
		(struct Element_point_tool *)element_point_tool_void))
	{
		interaction_volume=Interactive_event_get_interaction_volume(event);
		scene=Interactive_event_get_scene(event);
		if (scene != 0)
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
						scene_picked_object_list=
							Scene_pick_objects(scene,interaction_volume,graphics_buffer);
						if (scene_picked_object_list != 0)
						{
							element_point_tool->picked_element_point_was_unselected=0;
							if (0 != (picked_element_point=
								Scene_picked_object_list_get_nearest_element_point(
									scene_picked_object_list,(struct Cmiss_region *)NULL,
									(struct Scene_picked_object **)NULL,
									(struct Cmiss_rendition **)NULL,
									(struct Cmiss_graphic **)NULL)))
							{
								/* Execute command_field of picked_element_point */
								if (element_point_tool->command_field)
								{
									if (Element_point_ranges_get_identifier(
											 picked_element_point, &element_point_ranges_identifier))
									{
										if (element_point_tool->time_keeper)
										{
											time =
												Time_keeper_get_time(element_point_tool->time_keeper);
										}
										else
										{
											time = 0;
										}
										/* need to get the xi for the picked_element_point
											 in order to evaluate the command_field there */
										if ((multi_range = Element_point_ranges_get_ranges(
											picked_element_point)) &&
											(Multi_range_get_range(multi_range, /*range_no*/0,
												&start, &stop)) &&
											FE_element_get_numbered_xi_point(
												element_point_ranges_identifier.element,
												element_point_ranges_identifier.xi_discretization_mode,
												element_point_ranges_identifier.number_in_xi,
												element_point_ranges_identifier.exact_xi,
												(Cmiss_field_cache_id)0,
												/*coordinate_field*/(struct Computed_field *)NULL,
												/*density_field*/(struct Computed_field *)NULL,
												start, xi))
										{
											Cmiss_field_module_id field_module = Cmiss_field_get_field_module(element_point_tool->command_field);
											Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
											Cmiss_field_cache_set_time(field_cache, time);
											Cmiss_field_cache_set_mesh_location_with_parent(field_cache,
												element_point_ranges_identifier.element, Cmiss_element_get_dimension(element_point_ranges_identifier.element),
												xi, element_point_ranges_identifier.top_level_element);
											char *command_string = Cmiss_field_evaluate_string(element_point_tool->command_field, field_cache);
											if (command_string)
											{
												Execute_command_execute_string(element_point_tool->execute_command, command_string);
												DEALLOCATE(command_string);
											}
											Cmiss_field_cache_destroy(&field_cache);
											Cmiss_field_module_destroy(&field_module);
										}
									}
								}
								if (!Element_point_ranges_selection_is_element_point_ranges_selected(
									element_point_tool->element_point_ranges_selection,
									picked_element_point))
								{
									element_point_tool->picked_element_point_was_unselected=1;
								}
							}
							REACCESS(Element_point_ranges)(
								&(element_point_tool->last_picked_element_point),
								picked_element_point);
							clear_selection = !shift_pressed;
							if (clear_selection)
							{
								Element_point_ranges_selection_begin_cache(
									element_point_tool->element_point_ranges_selection);
								Element_point_ranges_selection_clear(
									element_point_tool->element_point_ranges_selection);
							}
							if (picked_element_point)
							{
								Element_point_ranges_selection_select_element_point_ranges(
									element_point_tool->element_point_ranges_selection,
									picked_element_point);
							}
							if (clear_selection)
							{
								Element_point_ranges_selection_end_cache(
									element_point_tool->element_point_ranges_selection);
							}
							DESTROY(LIST(Scene_picked_object))(&(scene_picked_object_list));
						}
						element_point_tool->motion_detected=0;
						REACCESS(Interaction_volume)(
							&(element_point_tool->last_interaction_volume),
							interaction_volume);
					}
				} break;
				case INTERACTIVE_EVENT_MOTION_NOTIFY:
				case INTERACTIVE_EVENT_BUTTON_RELEASE:
				{
					if (element_point_tool->last_interaction_volume&&
						((INTERACTIVE_EVENT_MOTION_NOTIFY==event_type) ||
							(1==Interactive_event_get_button_number(event))))
					{
						if (INTERACTIVE_EVENT_MOTION_NOTIFY==event_type)
						{
							element_point_tool->motion_detected=1;
						}
						if (element_point_tool->last_picked_element_point)
						{
							/* unselect last_picked_element_point if not just added */
							if ((INTERACTIVE_EVENT_BUTTON_RELEASE==event_type)&&
								shift_pressed&&
								(!(element_point_tool->picked_element_point_was_unselected)))
							{
								Element_point_ranges_selection_unselect_element_point_ranges(
									element_point_tool->element_point_ranges_selection,
									element_point_tool->last_picked_element_point);
							}
						}
						else if (element_point_tool->motion_detected)
						{
							/* rubber band select */
							temp_interaction_volume=
								create_Interaction_volume_bounding_box(
									element_point_tool->last_interaction_volume,
									interaction_volume);
							if (temp_interaction_volume != 0)
							{
								if (INTERACTIVE_EVENT_MOTION_NOTIFY==event_type)
								{
									if (!element_point_tool->rubber_band)
									{
										/* create rubber_band object and put in scene */
										element_point_tool->rubber_band=CREATE(GT_object)(
											"element_point_tool_rubber_band",g_POLYLINE,
											element_point_tool->rubber_band_material);
										ACCESS(GT_object)(element_point_tool->rubber_band);
									}
									Interaction_volume_make_polyline_extents(
										temp_interaction_volume,element_point_tool->rubber_band);
								}
								else
								{
#if defined (USE_SCENE_OBJECTS)
									Scene_remove_graphics_object(scene,
										element_point_tool->rubber_band);
#endif
									DEACCESS(GT_object)(&(element_point_tool->rubber_band));
								}
								if (INTERACTIVE_EVENT_BUTTON_RELEASE==event_type)
								{
									scene_picked_object_list=
										Scene_pick_objects(scene,temp_interaction_volume,graphics_buffer);
									if (scene_picked_object_list != 0)
									{
										element_point_ranges_list=
											Scene_picked_object_list_get_picked_element_points(
												scene_picked_object_list);
										if (element_point_ranges_list != 0)
										{
											Element_point_ranges_selection_begin_cache(
												element_point_tool->element_point_ranges_selection);
											FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
												Element_point_ranges_select,(void *)
												element_point_tool->element_point_ranges_selection,
												element_point_ranges_list);
											Element_point_ranges_selection_end_cache(
												element_point_tool->element_point_ranges_selection);
											DESTROY(LIST(Element_point_ranges))(
												&element_point_ranges_list);
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
							Element_point_tool_reset((void *)element_point_tool);
						}
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Element_point_tool_interactive_event_handler.  "
						"Unknown event type");
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_tool_interactive_event_handler.  Invalid argument(s)");
	}
	LEAVE;

} /* Element_point_tool_interactive_event_handler */

static int Element_point_tool_bring_up_interactive_tool_dialog(
 void *element_point_tool_void, struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Brings up a dialog for editing settings of the Element_point_tool - in a standard
format for passing to an Interactive_toolbar.
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_tool_bring_up_interactive_tool_dialog);
	return_code =
		Element_point_tool_pop_up_dialog((struct Element_point_tool *)element_point_tool_void,graphics_window);
	LEAVE;

	return (return_code);
} /* Element_point_tool_bring_up_interactive_tool_dialog */

#if defined (WX_USER_INTERFACE)
class wxElementPointTool : public wxPanel
{
	Element_point_tool *element_point_tool;
	wxCheckBox *elementpointcommandfieldcheckbox;
	 wxPanel *element_point_command_field_chooser_panel;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Computed_field *command_field;
	DEFINE_MANAGER_CLASS(Computed_field);
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	 *element_point_command_field_chooser;
public:

  wxElementPointTool(Element_point_tool *element_point_tool,wxPanel *parent ):
	element_point_tool(element_point_tool)
  {
		{
			wxXmlInit_element_point_tool();
			wxXmlResource::Get()->LoadPanel(this,parent,_T("CmguiElementPointTool"));
			elementpointcommandfieldcheckbox = XRCCTRL(*this, "ElementPointCommandFieldCheckBox",wxCheckBox);
			element_point_command_field_chooser_panel = XRCCTRL(*this, "ElementPointCommandFieldChooserPanel", wxPanel);
			if (element_point_tool->region)
			{
				computed_field_manager=
					Cmiss_region_get_Computed_field_manager(element_point_tool->region);
			}
			element_point_command_field_chooser =
				 new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
				 (element_point_command_field_chooser_panel, element_point_tool->command_field, computed_field_manager,
						Computed_field_has_string_value_type, (void *)NULL, element_point_tool->user_interface);
			Callback_base< Computed_field* > *element_point_command_field_callback =
				 new Callback_member_callback< Computed_field*,
					wxElementPointTool, int (wxElementPointTool::*)(Computed_field *) >
				 (this, &wxElementPointTool::element_point_command_field_callback);
			element_point_command_field_chooser->set_callback(element_point_command_field_callback);
			if (element_point_tool != NULL)
			{
				 command_field = Element_point_tool_get_command_field(element_point_tool);
				 element_point_command_field_chooser->set_object(command_field);
				 if (command_field == NULL)
				 {
						elementpointcommandfieldcheckbox->SetValue(0);
						element_point_command_field_chooser_panel->Disable();
				 }
				 else
				 {
						elementpointcommandfieldcheckbox->SetValue(1);
						element_point_command_field_chooser_panel->Enable();
				 }
			}
		}
  };

  wxElementPointTool()
  {
  };

 ~ wxElementPointTool()
  {
		 //		 delete element_point_command_field_chooser;
  };
	 int element_point_command_field_callback(Computed_field *command_field)
	 {
			Element_point_tool_set_command_field(element_point_tool, command_field);
			return 1;
	 }

	 void ElementPointCommandFieldChecked(wxCommandEvent &event)
	 {
		USE_PARAMETER(event);
			struct Computed_field *command_field;
			elementpointcommandfieldcheckbox = XRCCTRL(*this, "ElementPointCommandFieldCheckBox",wxCheckBox);
			element_point_command_field_chooser_panel = XRCCTRL(*this, "ElementPointCommandFieldChooserPanel", wxPanel);
			if (elementpointcommandfieldcheckbox->IsChecked())
			{
				 if (element_point_tool)
				 {
						if (Element_point_tool_get_command_field(element_point_tool))
						{
							 Element_point_tool_set_command_field(element_point_tool, (struct Computed_field *)NULL);
							 element_point_command_field_chooser_panel->Enable();
						}
						else
						{
							 /* get label field from widget */
							 if (element_point_command_field_chooser->get_number_of_object() > 0)
							 {
									command_field = element_point_command_field_chooser->get_object();
									if (command_field)
									{
										 Element_point_tool_set_command_field(element_point_tool, command_field);
									}
							 }
							 else
							 {
									elementpointcommandfieldcheckbox->SetValue(0);
									element_point_command_field_chooser_panel->Disable();
							 }
						}
				 }
				 else
				 {
						display_message(ERROR_MESSAGE,
							 "Element_point_tool_command_field_button_CB.  Invalid argument(s)");
				 }
			}
			else
			{
				 Element_point_tool_set_command_field(element_point_tool, (struct Computed_field *)NULL);
				 elementpointcommandfieldcheckbox->SetValue(0);
				 element_point_command_field_chooser_panel->Disable();
			}
	 }
  DECLARE_DYNAMIC_CLASS(wxElementPointTool);
  DECLARE_EVENT_TABLE();
};

IMPLEMENT_DYNAMIC_CLASS(wxElementPointTool, wxPanel)

BEGIN_EVENT_TABLE(wxElementPointTool, wxPanel)
	 EVT_CHECKBOX(XRCID("ElementPointCommandFieldCheckBox"),wxElementPointTool::ElementPointCommandFieldChecked)
END_EVENT_TABLE()

#endif /* defined (WX_USER_INTERFACE) */


static int Element_point_tool_destroy_element_point_tool(void **element_point_tool_void)
/*******************************************************************************
LAST MODIFIED : 6 July 2007

DESCRIPTION :
Function to call DESTROY
==============================================================================*/
{
	ENTER(element_point_tool_destroy_element_point_tool);
	Element_point_tool *element_point_tool;
	int return_code;
	return_code=0;

	element_point_tool = (struct Element_point_tool *)*element_point_tool_void;
	if (element_point_tool != 0)
	{
		 return_code = DESTROY(Element_point_tool)(&element_point_tool);
	}
	LEAVE;
	return (return_code);
}

static int Element_point_tool_copy_function(
	void *destination_tool_void, void *source_tool_void,
	struct MANAGER(Interactive_tool) *destination_tool_manager)
/*******************************************************************************
LAST MODIFIED : 29 March 2007

DESCRIPTION :
Copies the state of one element_point tool to another.
==============================================================================*/
{
	int return_code;
	struct Element_point_tool *destination_element_point_tool, *source_element_point_tool;

	ENTER(Element_point_tool_copy_function);
	if ((destination_tool_void || destination_tool_manager) &&
		(source_element_point_tool=(struct Element_point_tool *)source_tool_void))
	{
		if (destination_tool_void)
		{
			destination_element_point_tool = (struct Element_point_tool *)destination_tool_void;
		}
		else
		{
			destination_element_point_tool = CREATE(Element_point_tool)
				(destination_tool_manager,
				source_element_point_tool->region,
				source_element_point_tool->element_point_ranges_selection,
				source_element_point_tool->rubber_band_material,
				source_element_point_tool->user_interface,
				source_element_point_tool->time_keeper);
			Element_point_tool_set_execute_command(destination_element_point_tool,
				source_element_point_tool->execute_command);
		}
		if (destination_element_point_tool)
		{
			destination_element_point_tool->command_field = source_element_point_tool->command_field;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Element_point_tool_copy_function.  Could not create copy.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_tool_copy_function.  Invalid argument(s)");
		return_code=0;
	}
	return (return_code);
}

struct Element_point_tool *CREATE(Element_point_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct Cmiss_region *region,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Graphical_material *rubber_band_material,
	struct User_interface *user_interface,
	struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Creates an Element_point_tool with Interactive_tool in
<interactive_tool_manager>. Selects element/grid points in
<element_point_ranges_selection> in response to interactive_events.
==============================================================================*/
{
	struct Element_point_tool *element_point_tool;
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(CREATE(Element_point_tool));
	if (interactive_tool_manager&&element_point_ranges_selection&&region&&
		(NULL != (computed_field_manager=
			Cmiss_region_get_Computed_field_manager(region)))
		&&rubber_band_material&&user_interface)
	{
		if (ALLOCATE(element_point_tool,struct Element_point_tool,1))
		{
			element_point_tool->region = region;
			element_point_tool->execute_command=NULL;
			element_point_tool->interactive_tool_manager=interactive_tool_manager;
			element_point_tool->element_point_ranges_selection=
				element_point_ranges_selection;
			element_point_tool->rubber_band_material=
				ACCESS(Graphical_material)(rubber_band_material);
			element_point_tool->user_interface=user_interface;
			element_point_tool->time_keeper = (struct Time_keeper *)NULL;
			if (time_keeper)
			{
				element_point_tool->time_keeper = ACCESS(Time_keeper)(time_keeper);
			}
			/* user settings */
			element_point_tool->command_field = (struct Computed_field *)NULL;
			/* interactive_tool */
			element_point_tool->interactive_tool=CREATE(Interactive_tool)(
				"element_point_tool","Element point tool",
				Interactive_tool_element_point_type_string,
				Element_point_tool_interactive_event_handler,
				Element_point_tool_bring_up_interactive_tool_dialog,
				Element_point_tool_reset,
				Element_point_tool_destroy_element_point_tool,
				Element_point_tool_copy_function,
				(void *)element_point_tool);
			ADD_OBJECT_TO_MANAGER(Interactive_tool)(
				element_point_tool->interactive_tool,
				element_point_tool->interactive_tool_manager);
			element_point_tool->last_picked_element_point=
				(struct Element_point_ranges *)NULL;
			element_point_tool->last_interaction_volume=
				(struct Interaction_volume *)NULL;
			element_point_tool->rubber_band=(struct GT_object *)NULL;
#if defined (WX_USER_INTERFACE) /* switch (USER_INTERFACE) */
			element_point_tool->wx_element_point_tool = (wxElementPointTool *)NULL;
#endif /* switch (USER_INTERFACE) */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Element_point_tool).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Element_point_tool).  Invalid argument(s)");
		element_point_tool=(struct Element_point_tool *)NULL;
	}
	LEAVE;

	return (element_point_tool);
} /* CREATE(Element_point_tool) */

int DESTROY(Element_point_tool)(
	struct Element_point_tool **element_point_tool_address)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Frees and deaccesses objects in the <element_point_tool> and deallocates the
structure itself.
==============================================================================*/
{
	struct Element_point_tool *element_point_tool;
	int return_code;

	ENTER(DESTROY(Element_point_tool));
	if (element_point_tool_address&&
		(element_point_tool= *element_point_tool_address))
	{
		REACCESS(Element_point_ranges)(
			&(element_point_tool->last_picked_element_point),
			(struct Element_point_ranges *)NULL);
		REACCESS(Interaction_volume)(
			&(element_point_tool->last_interaction_volume),
			(struct Interaction_volume *)NULL);
		REACCESS(GT_object)(&(element_point_tool->rubber_band),
			(struct GT_object *)NULL);
		DEACCESS(Graphical_material)(&(element_point_tool->rubber_band_material));
		if (element_point_tool->time_keeper)
		{
			DEACCESS(Time_keeper)(&(element_point_tool->time_keeper));
		}
		DEALLOCATE(*element_point_tool_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Element_point_tool).  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* DESTROY(Element_point_tool) */

int Element_point_tool_pop_up_dialog(
																		 struct Element_point_tool *element_point_tool, struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Pops up a dialog for editing settings of the Element_point_tool.
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_tool_pop_up_dialog);
	if (element_point_tool)
	{
#if defined (WX_USER_INTERFACE) /* switch (USER_INTERFACE) */
		wxPanel *pane;
		if (!element_point_tool->wx_element_point_tool)
		{
			 element_point_tool->wx_element_point_tool= new wxElementPointTool(element_point_tool,
					Graphics_window_get_interactive_tool_panel(graphics_window));
			 pane = XRCCTRL(*element_point_tool->wx_element_point_tool, "CmguiElementPointTool", wxPanel);
			 element_point_tool->tool_position = pane->GetPosition();
			 element_point_tool->wx_element_point_tool->Show();
		}
		else
		{
			 pane = XRCCTRL(*element_point_tool->wx_element_point_tool, "CmguiElementPointTool", wxPanel);
			 pane->SetPosition(element_point_tool->tool_position);
			 element_point_tool->wx_element_point_tool->Show();
		}
#else /* switch (USER_INTERFACE) */
		USE_PARAMETER(graphics_window);
		display_message(ERROR_MESSAGE, "Element_point_tool_pop_up_dialog.  "
			"No dialog implemented for this User Interface");
#endif /* defined (USER_INTERFACE) */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_tool_pop_up_dialog.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_tool_pop_up_dialog */

struct Computed_field *Element_point_tool_get_command_field(
	struct Element_point_tool *element_point_tool)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Returns the command_field to be executed when the element is clicked on in the
<element_point_tool>.
==============================================================================*/
{
	struct Computed_field *command_field;

	ENTER(Element_point_tool_get_command_field);
	if (element_point_tool)
	{
		command_field=element_point_tool->command_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_tool_get_command_field.  Invalid argument(s)");
		command_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (command_field);
} /* Element_point_tool_get_command_field */

int Element_point_tool_set_command_field(
	struct Element_point_tool *element_point_tool,
	struct Computed_field *command_field)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Sets the command_field to be executed when the element is clicked on in the
<element_point_tool>.
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_tool_set_command_field);
	if (element_point_tool && ((!command_field) ||
		Computed_field_has_string_value_type(command_field, (void *)NULL)))
	{
		return_code = 1;
		if (command_field != element_point_tool->command_field)
		{
			element_point_tool->command_field = command_field;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_tool_set_command_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_tool_set_command_field */

struct Interactive_tool *Element_point_tool_get_interactive_tool(
	struct Element_point_tool *element_point_tool)
/*******************************************************************************
LAST MODIFIED : 29 March 2007

DESCRIPTION :
Returns the generic interactive_tool the represents the <element_point_tool>.
==============================================================================*/
{
	struct Interactive_tool *interactive_tool;

	ENTER(Element_point_tool_get_interactive_tool);
	if (element_point_tool)
	{
		interactive_tool=element_point_tool->interactive_tool;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_get_interactive_tool.  Invalid argument(s)");
		interactive_tool=(struct Interactive_tool *)NULL;
	}
	LEAVE;

	return (interactive_tool);
} /* Element_tool_get_interactive_tool */

int Element_point_tool_set_execute_command(struct Element_point_tool *element_point_tool,
	struct Execute_command *execute_command)
{
	int return_code = 0;
	if (element_point_tool)
	{
		element_point_tool->execute_command = execute_command;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_tool_set_execute_command.  Invalid argument(s)");
	}

	return return_code;
}
