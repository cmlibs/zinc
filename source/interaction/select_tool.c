/*******************************************************************************
FILE : select_tool.c

LAST MODIFIED : 18 September 2000

DESCRIPTION :
Interactive tool for selecting Any_objects associated with Scene_objects with
mouse and other devices.
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
#include "command/command.h"
#include "interaction/select_tool.h"
static char select_tool_uidh[] =
#include "interaction/select_tool.uidh"
	;
#include "general/debug.h"
#include "graphics/scene.h"
#include "interaction/interaction_graphics.h"
#include "interaction/interaction_volume.h"
#include "interaction/interactive_event.h"
#include "motif/image_utilities.h"
#include "user_interface/message.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int select_tool_hierarchy_open=0;
static MrmHierarchy select_tool_hierarchy;
#endif /* defined (MOTIF) */

static char Interactive_tool_select_type_string[] = "select_tool";


/*
Module types
------------
*/

struct Select_tool
/*******************************************************************************
LAST MODIFIED : 24 August 2000

DESCRIPTION :
Object storing all the parameters for interactively selecting Any_objects.
==============================================================================*/
{
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct Interactive_tool *interactive_tool;
	struct Any_object_selection *any_object_selection;
	struct Graphical_material *rubber_band_material;
	/* information about picked any_object */
	int picked_any_object_was_unselected;
	int motion_detected;
	struct Any_object *last_picked_any_object;
	struct Interaction_volume *last_interaction_volume;
	struct GT_object *rubber_band;
#if defined (MOTIF)
	Display *display;
#endif /* defined (MOTIF) */
}; /* struct Select_tool */

/*
Module functions
----------------
*/

static void Select_tool_interactive_event_handler(void *device_id,
	struct Interactive_event *event,void *select_tool_void,
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
	int clear_selection,input_modifier,shift_pressed;
	struct Any_object *picked_any_object;
	struct Select_tool *select_tool;
	struct Interaction_volume *interaction_volume,*temp_interaction_volume;
	struct LIST(Any_object) *any_object_list;
	struct LIST(Scene_picked_object) *scene_picked_object_list;
	struct Scene *scene;

	ENTER(Select_tool_interactive_event_handler);
	if (device_id&&event&&(select_tool=
		(struct Select_tool *)select_tool_void))
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
							select_tool->picked_any_object_was_unselected=0;
							if (picked_any_object=
								Scene_picked_object_list_get_nearest_any_object(
									scene_picked_object_list,(struct Scene_picked_object **)NULL))
							{
								if (!Any_object_selection_is_any_object_selected(
									select_tool->any_object_selection,picked_any_object))
								{
									select_tool->picked_any_object_was_unselected=1;
								}
							}
							REACCESS(Any_object)(
								&(select_tool->last_picked_any_object),picked_any_object);
							if (clear_selection = !shift_pressed)
#if defined (OLD_CODE)
								&&((!picked_any_object)||
									(select_tool->picked_any_object_was_unselected))))
#endif /*defined (OLD_CODE) */
							{
								Any_object_selection_begin_cache(
									select_tool->any_object_selection);
								Any_object_selection_clear(select_tool->any_object_selection);
							}
							if (picked_any_object)
							{
								Any_object_selection_select_any_object(
									select_tool->any_object_selection,picked_any_object);
							}
							if (clear_selection)
							{
								Any_object_selection_end_cache(
									select_tool->any_object_selection);
							}
							DESTROY(LIST(Scene_picked_object))(&(scene_picked_object_list));
						}
						select_tool->motion_detected=0;
						REACCESS(Interaction_volume)(
							&(select_tool->last_interaction_volume),interaction_volume);
					}
				} break;
				case INTERACTIVE_EVENT_MOTION_NOTIFY:
				case INTERACTIVE_EVENT_BUTTON_RELEASE:
				{
					if (select_tool->last_interaction_volume&&
						((INTERACTIVE_EVENT_MOTION_NOTIFY==event_type) ||
						(1==Interactive_event_get_button_number(event))))
					{
						if (INTERACTIVE_EVENT_MOTION_NOTIFY==event_type)
						{
							select_tool->motion_detected=1;
						}
						if (select_tool->last_picked_any_object)
						{
							/* unselect last_picked_any_object if not just added */
							if ((INTERACTIVE_EVENT_BUTTON_RELEASE==event_type)&&
								shift_pressed&&!(select_tool->picked_any_object_was_unselected))
							{
								Any_object_selection_unselect_any_object(
									select_tool->any_object_selection,
									select_tool->last_picked_any_object);
							}
						}
						else if (select_tool->motion_detected)
						{
							/* rubber band select */
							if (temp_interaction_volume=
								create_Interaction_volume_bounding_box(
									select_tool->last_interaction_volume,interaction_volume))
							{
								if (INTERACTIVE_EVENT_MOTION_NOTIFY==event_type)
								{
									if (!select_tool->rubber_band)
									{
										/* create rubber_band object and put in scene */
										select_tool->rubber_band=CREATE(GT_object)(
											"select_tool_rubber_band",g_POLYLINE,
											select_tool->rubber_band_material);
										ACCESS(GT_object)(select_tool->rubber_band);
										Scene_add_graphics_object(scene,
											select_tool->rubber_band,/*position*/0,
											"select_tool_rubber_band",/*fast_changing*/1);
									}
									Interaction_volume_make_polyline_extents(
										temp_interaction_volume,select_tool->rubber_band);
								}
								else
								{
									Scene_remove_graphics_object(scene,select_tool->rubber_band);
									DEACCESS(GT_object)(&(select_tool->rubber_band));
								}
								if (INTERACTIVE_EVENT_BUTTON_RELEASE==event_type)
								{
									if (scene_picked_object_list=
										Scene_pick_objects(scene,temp_interaction_volume,graphics_buffer))
									{
										if (any_object_list=
											Scene_picked_object_list_get_picked_any_objects(
												scene_picked_object_list))
										{
											Any_object_selection_begin_cache(
												select_tool->any_object_selection);
											FOR_EACH_OBJECT_IN_LIST(Any_object)(
												Any_object_select_in_Any_object_selection,(void *)
												select_tool->any_object_selection,any_object_list);
											Any_object_selection_end_cache(
												select_tool->any_object_selection);
											DESTROY(LIST(Any_object))(&any_object_list);
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
							REACCESS(Any_object)(&(select_tool->last_picked_any_object),
								(struct Any_object *)NULL);
							REACCESS(Interaction_volume)(
								&(select_tool->last_interaction_volume),
								(struct Interaction_volume *)NULL);
						}
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Select_tool_interactive_event_handler.  Unknown event type");
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Select_tool_interactive_event_handler.  Invalid argument(s)");
	}
	LEAVE;
} /* Select_tool_interactive_event_handler */

static struct Cmgui_image *Select_tool_get_icon(struct Colour *foreground, 
	struct Colour *background, void *select_tool_void)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Fetches a ToggleButton with an appropriate icon for the interactive tool
and as a child of <parent>.
==============================================================================*/
{
	Display *display;
	Pixel background_pixel, foreground_pixel;
	Pixmap pixmap;
	struct Cmgui_image *image;
	struct Select_tool *select_tool;

	ENTER(Select_tool_get_icon);
	if ((select_tool=(struct Select_tool *)select_tool_void))
	{
		if (MrmOpenHierarchy_binary_string(select_tool_uidh, sizeof(select_tool_uidh),
			&select_tool_hierarchy,&select_tool_hierarchy_open))
		{
			display = select_tool->display;
			convert_Colour_to_Pixel(display, foreground, &foreground_pixel);
			convert_Colour_to_Pixel(display, background, &background_pixel);
			if (MrmSUCCESS == MrmFetchIconLiteral(select_tool_hierarchy,
				"select_tool_icon",DefaultScreenOfDisplay(display),display,
				foreground_pixel, background_pixel, &pixmap))
			{ 
				image = create_Cmgui_image_from_Pixmap(display, pixmap);
			}
			else
			{
				display_message(WARNING_MESSAGE, "Select_tool_get_icon.  "
					"Could not fetch widget");
				image = (struct Cmgui_image *)NULL;
			}			
		}
		else
		{
			display_message(WARNING_MESSAGE, "Select_tool_get_icon.  "
				"Could not open heirarchy");
			image = (struct Cmgui_image *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Select_tool_get_icon.  Invalid argument_point(s)");
		image = (struct Cmgui_image *)NULL;
	}
	LEAVE;

	return (image);
} /* Select_tool_get_icon */

/*
Global functions
----------------
*/

struct Select_tool *CREATE(Select_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct Any_object_selection *any_object_selection,
	struct Graphical_material *rubber_band_material,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Creates an Select_tool with Interactive_tool in
<interactive_tool_manager>. Selects Any_objects represented by scene_objects in
<any_object_selection> in response to interactive_events.
==============================================================================*/
{
	struct Select_tool *select_tool;

	ENTER(CREATE(Select_tool));
	if (interactive_tool_manager&&any_object_selection&&
		rubber_band_material)
	{
		if (ALLOCATE(select_tool,struct Select_tool,1))
		{
			select_tool->display = User_interface_get_display(user_interface);
			select_tool->interactive_tool_manager=interactive_tool_manager;
			select_tool->any_object_selection=any_object_selection;
			select_tool->rubber_band_material=
				ACCESS(Graphical_material)(rubber_band_material);
			select_tool->interactive_tool=CREATE(Interactive_tool)(
				"select_tool","Select tool",
				Interactive_tool_select_type_string,
				Select_tool_interactive_event_handler,
				Select_tool_get_icon,
				(Interactive_tool_bring_up_dialog_function *)NULL,
				(Interactive_tool_reset_function *)NULL,
				(Interactive_tool_destroy_tool_data_function *)NULL,
				(Interactive_tool_copy_function *)NULL,
				(void *)select_tool);
			ADD_OBJECT_TO_MANAGER(Interactive_tool)(
				select_tool->interactive_tool,
				select_tool->interactive_tool_manager);
			select_tool->last_picked_any_object=
				(struct Any_object *)NULL;
			select_tool->last_interaction_volume=
				(struct Interaction_volume *)NULL;
			select_tool->rubber_band=(struct GT_object *)NULL;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Select_tool).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Select_tool).  Invalid argument(s)");
		select_tool=(struct Select_tool *)NULL;
	}
	LEAVE;

	return (select_tool);
} /* CREATE(Select_tool) */

int DESTROY(Select_tool)(struct Select_tool **select_tool_address)
/*******************************************************************************
LAST MODIFIED : 24 August 2000

DESCRIPTION :
Frees and deaccesses objects in the <select_tool> and deallocates the
structure itself.
==============================================================================*/
{
	struct Select_tool *select_tool;
	int return_code;

	ENTER(DESTROY(Select_tool));
	if (select_tool_address&&
		(select_tool= *select_tool_address))
	{
		REACCESS(Any_object)(&(select_tool->last_picked_any_object),
			(struct Any_object *)NULL);
		REMOVE_OBJECT_FROM_MANAGER(Interactive_tool)(select_tool->interactive_tool,
			select_tool->interactive_tool_manager);
		REACCESS(Interaction_volume)(&(select_tool->last_interaction_volume),
			(struct Interaction_volume *)NULL);
		REACCESS(GT_object)(&(select_tool->rubber_band),(struct GT_object *)NULL);
		DEACCESS(Graphical_material)(&(select_tool->rubber_band_material));
		DEALLOCATE(*select_tool_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Select_tool).  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* DESTROY(Select_tool) */
