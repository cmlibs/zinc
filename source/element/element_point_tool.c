/*******************************************************************************
FILE : element_point_tool.c

LAST MODIFIED : 7 September 2000

DESCRIPTION :
Interactive tool for selecting element/grid points with mouse and other devices.
==============================================================================*/
#include "command/command.h"
#include "element/element_point_tool.h"
#include "element/element_point_tool.uidh"
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
static int element_point_tool_hierarchy_open=0;
static MrmHierarchy element_point_tool_hierarchy;
#endif /* defined (MOTIF) */


/*
Module types
------------
*/

struct Element_point_tool
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Object storing all the parameters for interactively selecting element points.
==============================================================================*/
{
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct Interactive_tool *interactive_tool;
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct Graphical_material *rubber_band_material;
	/* information about picked element_point_ranges */
	int picked_element_point_was_unselected;
	int motion_detected;
	struct Element_point_ranges *last_picked_element_point;
	struct Interaction_volume *last_interaction_volume;
	struct GT_object *rubber_band;
}; /* struct Element_point_tool */

/*
Module functions
----------------
*/

static void Element_point_tool_interactive_event_handler(void *device_id,
	struct Interactive_event *event,void *element_point_tool_void)
/*******************************************************************************
LAST MODIFIED : 7 September 2000

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
	struct Element_point_ranges *picked_element_point;
	struct Element_point_tool *element_point_tool;
	struct Interaction_volume *interaction_volume,*temp_interaction_volume;
	struct LIST(Element_point_ranges) *element_point_ranges_list;
	struct LIST(Scene_picked_object) *scene_picked_object_list;
	struct Scene *scene;

	ENTER(Element_point_tool_interactive_event_handler);
	if (device_id&&event&&(element_point_tool=
		(struct Element_point_tool *)element_point_tool_void))
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
							Scene_pick_objects(scene,interaction_volume))
						{
							element_point_tool->picked_element_point_was_unselected=0;
							if (picked_element_point=
								Scene_picked_object_list_get_nearest_element_point(
									scene_picked_object_list,(struct GROUP(FE_element) *)NULL,
									(struct Scene_picked_object **)NULL,
									(struct GT_element_group **)NULL,
									(struct GT_element_settings **)NULL))
							{
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
							if (clear_selection=((!shift_pressed)&&((!picked_element_point)||
								(element_point_tool->picked_element_point_was_unselected))))
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
							if (temp_interaction_volume=
								create_Interaction_volume_bounding_box(
									element_point_tool->last_interaction_volume,
									interaction_volume))
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
										Scene_add_graphics_object(scene,
											element_point_tool->rubber_band,/*position*/0,
											"element_point_tool_rubber_band",/*fast_changing*/1);
									}
									Interaction_volume_make_polyline_extents(
										temp_interaction_volume,element_point_tool->rubber_band);
								}
								else
								{
									Scene_remove_graphics_object(scene,
										element_point_tool->rubber_band);
									DEACCESS(GT_object)(&(element_point_tool->rubber_band));
								}
								if (INTERACTIVE_EVENT_BUTTON_RELEASE==event_type)
								{
									if (scene_picked_object_list=
										Scene_pick_objects(scene,temp_interaction_volume))
									{
										if (element_point_ranges_list=
											Scene_picked_object_list_get_picked_element_points(
												scene_picked_object_list))
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
							REACCESS(Element_point_ranges)(
								&(element_point_tool->last_picked_element_point),
								(struct Element_point_ranges *)NULL);
							REACCESS(Interaction_volume)(
								&(element_point_tool->last_interaction_volume),
								(struct Interaction_volume *)NULL);
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

static Widget Element_point_tool_make_interactive_tool_button(
	void *element_point_tool_void,Widget parent)
/*******************************************************************************
LAST MODIFIED : 16 May 2000

DESCRIPTION :
Fetches a ToggleButton with an appropriate icon for the interactive tool
and as a child of <parent>.
==============================================================================*/
{
	MrmType element_point_tool_dialog_class;
	struct Element_point_tool *element_point_tool;
	Widget widget;

	ENTER(Element_point_tool_make_interactive_tool_button);
	widget=(Widget)NULL;
	if ((element_point_tool=
		(struct Element_point_tool *)element_point_tool_void)&&parent)
	{
		if (MrmOpenHierarchy_base64_string(element_point_tool_uidh,
			&element_point_tool_hierarchy,&element_point_tool_hierarchy_open))
		{
			if (MrmSUCCESS == MrmFetchWidget(element_point_tool_hierarchy,
				"element_point_tool_button",parent,&widget,
				&element_point_tool_dialog_class))
			{
				XtVaSetValues(widget,
					XmNuserData,element_point_tool->interactive_tool,NULL);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Element_point_tool_make_interactive_tool_button.  "
					"Could not fetch widget");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Element_point_tool_make_interactive_tool_button.  "
				"Could not open heirarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_tool_make_interactive_tool_button.  Invalid argument(s)");
	}
	LEAVE;

	return (widget);
} /* Element_point_tool_make_interactive_tool_button */

/*
Global functions
----------------
*/

struct Element_point_tool *CREATE(Element_point_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Graphical_material *rubber_band_material)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Creates an Element_point_tool with Interactive_tool in
<interactive_tool_manager>. Selects element/grid points in
<element_point_ranges_selection> in response to interactive_events.
==============================================================================*/
{
	struct Element_point_tool *element_point_tool;

	ENTER(CREATE(Element_point_tool));
	if (interactive_tool_manager&&element_point_ranges_selection&&
		rubber_band_material)
	{
		if (ALLOCATE(element_point_tool,struct Element_point_tool,1))
		{
			element_point_tool->interactive_tool_manager=interactive_tool_manager;
			element_point_tool->element_point_ranges_selection=
				element_point_ranges_selection;
			element_point_tool->rubber_band_material=
				ACCESS(Graphical_material)(rubber_band_material);
			element_point_tool->interactive_tool=CREATE(Interactive_tool)(
				"element_point_tool","Element point tool",
				Element_point_tool_interactive_event_handler,
				Element_point_tool_make_interactive_tool_button,
				(Interactive_tool_bring_up_dialog_function *)NULL,
				(void *)element_point_tool);
			ADD_OBJECT_TO_MANAGER(Interactive_tool)(
				element_point_tool->interactive_tool,
				element_point_tool->interactive_tool_manager);
			element_point_tool->last_picked_element_point=
				(struct Element_point_ranges *)NULL;
			element_point_tool->last_interaction_volume=
				(struct Interaction_volume *)NULL;
			element_point_tool->rubber_band=(struct GT_object *)NULL;
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
LAST MODIFIED : 14 July 2000

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
		REMOVE_OBJECT_FROM_MANAGER(Interactive_tool)(
			element_point_tool->interactive_tool,
			element_point_tool->interactive_tool_manager);
		REACCESS(Interaction_volume)(
			&(element_point_tool->last_interaction_volume),
			(struct Interaction_volume *)NULL);
		REACCESS(GT_object)(&(element_point_tool->rubber_band),
			(struct GT_object *)NULL);
		DEACCESS(Graphical_material)(&(element_point_tool->rubber_band_material));
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
