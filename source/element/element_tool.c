/*******************************************************************************
FILE : element_tool.c

LAST MODIFIED : 18 July 2000

DESCRIPTION :
Interactive tool for selecting elements with mouse and other devices.
==============================================================================*/
#include "command/command.h"
#include "element/element_tool.h"
#include "element/element_tool.uidh"
#include "general/debug.h"
#include "graphics/scene.h"
#include "interaction/interaction_graphics.h"
#include "interaction/interaction_volume.h"
#include "interaction/interactive_event.h"
#include "user_interface/message.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int element_tool_hierarchy_open=0;
static MrmHierarchy element_tool_hierarchy;
#endif /* defined (MOTIF) */


/*
Module types
------------
*/

struct Element_tool
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Object storing all the parameters for converting scene input messages into
changes in node position and derivatives etc.
==============================================================================*/
{
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct Interactive_tool *interactive_tool;
	struct FE_element_selection *element_selection;
	struct Graphical_material *rubber_band_material;
	/* information about picked element */
	int picked_element_was_unselected;
	int motion_detected;
	struct FE_element *last_picked_element;
	struct Interaction_volume *last_interaction_volume;
	struct GT_object *rubber_band;
}; /* struct Element_tool */

/*
Module functions
----------------
*/

static void Element_tool_interactive_event_handler(void *device_id,
	struct Interactive_event *event,void *element_tool_void)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

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
	struct FE_element *picked_element;
	struct Element_tool *element_tool;
	struct Interaction_volume *interaction_volume,*temp_interaction_volume;
	struct LIST(FE_element) *element_list;
	struct LIST(Scene_picked_object) *scene_picked_object_list;
	struct Scene *scene;

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
					if (scene_picked_object_list=
						Scene_pick_objects(scene,interaction_volume))
					{
						element_tool->picked_element_was_unselected=0;
						if (picked_element=Scene_picked_object_list_get_nearest_element(
							scene_picked_object_list,(struct GROUP(FE_element) *)NULL,
							(struct Scene_picked_object **)NULL,
							(struct GT_element_group **)NULL,
							(struct GT_element_settings **)NULL))
						{
							if (!FE_element_selection_is_element_selected(
								element_tool->element_selection,picked_element))
							{
								element_tool->picked_element_was_unselected=1;
							}
						}
						REACCESS(FE_element)(&(element_tool->last_picked_element),
							picked_element);
						if (clear_selection=((!shift_pressed)&&((!picked_element)||
							(element_tool->picked_element_was_unselected))))
						{
							FE_element_selection_begin_cache(element_tool->element_selection);
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
				} break;
				case INTERACTIVE_EVENT_MOTION_NOTIFY:
				case INTERACTIVE_EVENT_BUTTON_RELEASE:
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
						if (temp_interaction_volume=create_Interaction_volume_bounding_box(
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
									Scene_pick_objects(scene,temp_interaction_volume))
								{
									if (element_list=Scene_picked_object_list_get_picked_elements(
										scene_picked_object_list))
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

static Widget Element_tool_make_interactive_tool_button(
	void *element_tool_void,Widget parent)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Fetches a ToggleButton with an appropriate icon for the interactive tool
and as a child of <parent>.
==============================================================================*/
{
	MrmType element_tool_dialog_class;
	struct Element_tool *element_tool;
	Widget widget;

	ENTER(Element_tool_make_interactive_tool_button);
	widget=(Widget)NULL;
	if ((element_tool=(struct Element_tool *)element_tool_void)&&parent)
	{
		if (MrmOpenHierarchy_base64_string(element_tool_uidh,
			&element_tool_hierarchy,&element_tool_hierarchy_open))
		{
			if (MrmSUCCESS == MrmFetchWidget(element_tool_hierarchy,
				"element_tool_button",parent,&widget,&element_tool_dialog_class))
			{
				XtVaSetValues(widget,XmNuserData,element_tool->interactive_tool,NULL);
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"Element_tool_make_interactive_tool_button.  Could not fetch widget");
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"Element_tool_make_interactive_tool_button.  Could not open heirarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_make_interactive_tool_button.  Invalid argument(s)");
	}
	LEAVE;

	return (widget);
} /* Element_tool_make_interactive_tool_button */

/*
Global functions
----------------
*/

struct Element_tool *CREATE(Element_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct FE_element_selection *element_selection,
	struct Graphical_material *rubber_band_material)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Creates an Element_tool with Interactive_tool in <interactive_tool_manager>.
Selects elements in <element_selection> in response to interactive_events.
==============================================================================*/
{
	struct Element_tool *element_tool;

	ENTER(CREATE(Element_tool));
	if (interactive_tool_manager&&element_selection&&rubber_band_material)
	{
		if (ALLOCATE(element_tool,struct Element_tool,1))
		{
			element_tool->interactive_tool_manager=interactive_tool_manager;
			element_tool->element_selection=element_selection;
			element_tool->rubber_band_material=
				ACCESS(Graphical_material)(rubber_band_material);
			element_tool->interactive_tool=CREATE(Interactive_tool)(
				"element_tool","Element point tool",
				Element_tool_interactive_event_handler,
				Element_tool_make_interactive_tool_button,
				(Interactive_tool_bring_up_dialog_function *)NULL,
				(void *)element_tool);
			ADD_OBJECT_TO_MANAGER(Interactive_tool)(
				element_tool->interactive_tool,
				element_tool->interactive_tool_manager);
			element_tool->last_picked_element=(struct FE_element *)NULL;
			element_tool->last_interaction_volume=(struct Interaction_volume *)NULL;
			element_tool->rubber_band=(struct GT_object *)NULL;
		}
		else
		{
			display_message(ERROR_MESSAGE,"CREATE(Element_tool).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Element_tool).  Invalid argument(s)");
		element_tool=(struct Element_tool *)NULL;
	}
	LEAVE;

	return (element_tool);
} /* CREATE(Element_tool) */

int DESTROY(Element_tool)(struct Element_tool **element_tool_address)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

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
