/*******************************************************************************
FILE : element_tool.c

LAST MODIFIED : 20 June 2001

DESCRIPTION :
Interactive tool for selecting elements with mouse and other devices.
==============================================================================*/
#include <Xm/Protocols.h>
#include <Xm/MwmUtil.h>
#include <Xm/Xm.h>
#include <Xm/ToggleBG.h>
#include "command/command.h"
#include "element/element_tool.h"
#include "element/element_tool.uidh"
#include "general/debug.h"
#include "graphics/scene.h"
#include "interaction/interaction_graphics.h"
#include "interaction/interaction_volume.h"
#include "interaction/interactive_event.h"
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
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Object storing all the parameters for interactively selecting elements.
==============================================================================*/
{
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct Interactive_tool *interactive_tool;
	/* needed for destroy button */
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(GROUP(FE_element)) *element_group_manager;
	struct FE_element_selection *element_selection;
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct Graphical_material *rubber_band_material;
	struct User_interface *user_interface;
	/* user-settable flags */
	int select_elements_enabled,select_faces_enabled,select_lines_enabled;
	/* information about picked element */
	int picked_element_was_unselected;
	int motion_detected;
	struct FE_element *last_picked_element;
	struct Interaction_volume *last_interaction_volume;
	struct GT_object *rubber_band;

	Widget select_elements_button,select_faces_button,select_lines_button;
	Widget widget,window_shell;
}; /* struct Element_tool */

/*
Module functions
----------------
*/

DECLARE_DIALOG_IDENTIFY_FUNCTION(element_tool,Element_tool, \
	select_elements_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_tool,Element_tool, \
	select_faces_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_tool,Element_tool, \
	select_lines_button)

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

static void Element_tool_destroy_selected_CB(Widget widget,
	void *element_tool_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 2 March 2001

DESCRIPTION :
Attempts to destroy all the top-level elements currently in the global
selection. ???RC could change to allow faces and lines to be destroyed, but
need other safeguard controls before allowing this.
==============================================================================*/
{
	struct Element_tool *element_tool;
	struct FE_element_list_CM_element_type_data element_list_type_data;
	struct LIST(FE_element) *destroy_element_list;

	ENTER(Element_tool_destroy_selected_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (element_tool = (struct Element_tool *)element_tool_void)
	{
		if (destroy_element_list = CREATE(LIST(FE_element))())
		{
			element_list_type_data.cm_element_type = CM_ELEMENT;
			element_list_type_data.element_list = destroy_element_list;
			FOR_EACH_OBJECT_IN_LIST(FE_element)(
				add_FE_element_of_CM_element_type_to_list,
				(void *)&element_list_type_data,
				FE_element_selection_get_element_list(element_tool->element_selection));
			destroy_listed_elements(destroy_element_list,
				element_tool->element_manager,element_tool->element_group_manager,
				element_tool->element_selection,
				element_tool->element_point_ranges_selection);
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

static void Element_tool_interactive_event_handler(void *device_id,
	struct Interactive_event *event,void *element_tool_void)
/*******************************************************************************
LAST MODIFIED : 18 September 2000

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
					/* interaction only works with first mouse button */
					if (1==Interactive_event_get_button_number(event))
					{
						if (scene_picked_object_list=
							Scene_pick_objects(scene,interaction_volume))
						{
							element_tool->picked_element_was_unselected=0;
							if (picked_element=Scene_picked_object_list_get_nearest_element(
								scene_picked_object_list,(struct GROUP(FE_element) *)NULL,
								element_tool->select_elements_enabled,
								element_tool->select_faces_enabled,
								element_tool->select_lines_enabled,
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
										Scene_pick_objects(scene,temp_interaction_volume))
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

static Widget Element_tool_make_interactive_tool_button(
	void *element_tool_void,Widget parent)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

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
		if (element_tool_hierarchy_open)
		{
			if (MrmSUCCESS == MrmFetchWidget(element_tool_hierarchy,
				"element_tool_button",parent,&widget,&element_tool_dialog_class))
			{
				XtVaSetValues(widget,XmNuserData,element_tool->interactive_tool,NULL);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Element_tool_make_interactive_tool_button.  Could not fetch widget");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Element_tool_make_interactive_tool_button.  Heirarchy not open");
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
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct FE_element_selection *element_selection,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Graphical_material *rubber_band_material,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Creates an Element_tool with Interactive_tool in <interactive_tool_manager>.
Selects elements in <element_selection> in response to interactive_events.
==============================================================================*/
{
	Atom WM_DELETE_WINDOW;
	MrmType element_tool_dialog_class;
	static MrmRegisterArg callback_list[]=
	{
		{"elem_tool_id_select_elems_btn",(XtPointer)
			DIALOG_IDENTIFY(element_tool,select_elements_button)},
		{"elem_tool_id_select_faces_btn",(XtPointer)
			DIALOG_IDENTIFY(element_tool,select_faces_button)},
		{"elem_tool_id_select_lines_btn",(XtPointer)
			DIALOG_IDENTIFY(element_tool,select_lines_button)},
		{"elem_tool_select_elems_btn_CB",
		 (XtPointer)Element_tool_select_elements_button_CB},
		{"elem_tool_select_faces_btn_CB",
		 (XtPointer)Element_tool_select_faces_button_CB},
		{"elem_tool_select_lines_btn_CB",
		 (XtPointer)Element_tool_select_lines_button_CB},
		{"elem_tool_destroy_selected_CB",
		 (XtPointer)Element_tool_destroy_selected_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"elem_tool_structure",(XtPointer)NULL}
	};
	struct Element_tool *element_tool;

	ENTER(CREATE(Element_tool));
	element_tool=(struct Element_tool *)NULL;
	if (interactive_tool_manager&&element_manager&&element_group_manager&&
		element_selection&&rubber_band_material&&user_interface)
	{
		if (MrmOpenHierarchy_base64_string(element_tool_uidh,
			&element_tool_hierarchy,&element_tool_hierarchy_open))
		{
			if (ALLOCATE(element_tool,struct Element_tool,1))
			{
				element_tool->interactive_tool_manager=interactive_tool_manager;
				element_tool->element_manager=element_manager;
				element_tool->element_group_manager=element_group_manager;
				element_tool->element_selection=element_selection;
				element_tool->element_point_ranges_selection=
					element_point_ranges_selection;
				element_tool->rubber_band_material=
					ACCESS(Graphical_material)(rubber_band_material);
				element_tool->user_interface=user_interface;
				/* user-settable flags */
				element_tool->select_elements_enabled=1;
				element_tool->select_faces_enabled=1;
				element_tool->select_lines_enabled=1;
				/* interactive_tool */
				element_tool->interactive_tool=CREATE(Interactive_tool)(
					"element_tool","Element tool",
					Interactive_tool_element_type_string,
					Element_tool_interactive_event_handler,
					Element_tool_make_interactive_tool_button,
					Element_tool_bring_up_interactive_tool_dialog,
					(Interactive_tool_destroy_tool_data_function *)NULL,
					(void *)element_tool);
				ADD_OBJECT_TO_MANAGER(Interactive_tool)(
					element_tool->interactive_tool,
					element_tool->interactive_tool_manager);
				element_tool->last_picked_element=(struct FE_element *)NULL;
				element_tool->last_interaction_volume=(struct Interaction_volume *)NULL;
				element_tool->rubber_band=(struct GT_object *)NULL;
				/* initialise widgets */
				element_tool->select_elements_button=(Widget)NULL;
				element_tool->select_faces_button=(Widget)NULL;
				element_tool->select_lines_button=(Widget)NULL;
				element_tool->widget=(Widget)NULL;
				element_tool->window_shell=(Widget)NULL;

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
								XmToggleButtonGadgetSetState(
									element_tool->select_elements_button,
									/*state*/element_tool->select_elements_enabled,
									/*notify*/False);
								XmToggleButtonGadgetSetState(element_tool->select_faces_button,
									/*state*/element_tool->select_faces_enabled,/*notify*/False);
								XmToggleButtonGadgetSetState(element_tool->select_lines_button,
									/*state*/element_tool->select_lines_enabled,/*notify*/False);
								XtManageChild(element_tool->widget);
								XtRealizeWidget(element_tool->window_shell);
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
					"CREATE(Element_tool).  Not enough memory");
				DEALLOCATE(element_tool);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Element_tool).  Could not open hierarchy");
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
LAST MODIFIED : 20 July 2000

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
		if (element_tool->window_shell)
		{
			destroy_Shell_list_item_from_shell(&(element_tool->window_shell),
				element_tool->user_interface);
			XtDestroyWidget(element_tool->window_shell);
		}
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
		XtPopup(element_tool->window_shell, XtGrabNone);
		/* make sure in addition that it is not shown as an icon */
		XtVaSetValues(element_tool->window_shell, XmNiconic, False, NULL);
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
		XtPopdown(element_tool->window_shell);
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
	int button_state,return_code;

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
	int button_state,return_code;

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
	int button_state,return_code;

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

int destroy_listed_elements(struct LIST(FE_element) *element_list,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct FE_element_selection *element_selection,
	struct Element_point_ranges_selection *element_point_ranges_selection)
/*******************************************************************************
LAST MODIFIED : 2 March 2001

DESCRIPTION :
Destroys all the elements in <element_list> that are not accessed outside
<element_manager>, the groups in <element_group_manager>,
<element_selection> and <element_point_ranges_selection>.
<element_group_manager>, <element_selection> and
<element_point_ranges_selection> are optional. Upon return <element_list>
contains all the elements that could not be destroyed.
???RC Should really be in its own module.
Note: currently requires all elements in the <element_list> to be of the same
CM_element_type, otherwise likely to fail. ???RC Fix this by filtering out
elements with all parents also in the list?
==============================================================================*/
{
	int number_of_elements_destroyed, number_of_elements_not_destroyed,
		return_code;
	struct Element_point_ranges *element_point_ranges;
	struct FE_element *element;
	struct GROUP(FE_element) *element_group;
	struct LIST(FE_element) *not_destroyed_element_list, *selected_element_list;
	struct LIST(Element_point_ranges) *selected_element_point_ranges_list;

	ENTER(destroy_listed_elements);
	if (element_list && element_manager)
	{
		return_code = 1;
		/* build list of elements that could not be destroyed */
		not_destroyed_element_list = CREATE(LIST(FE_element))();
		if (element_group_manager)
		{
			/* remove the elements - and their faces recursively - from all
				 groups they are in */
			while (return_code && (element_group =
				FIRST_OBJECT_IN_MANAGER_THAT(GROUP(FE_element))(
					FE_element_group_intersects_list, (void *)element_list,
					element_group_manager)))
			{
				MANAGED_GROUP_BEGIN_CACHE(FE_element)(element_group);
				while (return_code && (element = FIRST_OBJECT_IN_GROUP_THAT(FE_element)(
					FE_element_is_in_list, (void *)element_list, element_group)))
				{
					return_code = remove_FE_element_and_faces_from_group(element,
						element_group, RECURSIVE_REMOVE_ELEMENT_AND_PARENTLESS_FACES);
				}
				MANAGED_GROUP_END_CACHE(FE_element)(element_group);
			}
		}
		if (element_selection)
		{
			/* remove elements - and their faces and lines - from the
				 global element_selection */
			FE_element_selection_begin_cache(element_selection);
			selected_element_list =
				FE_element_selection_get_element_list(element_selection);
			while (return_code && (element = FIRST_OBJECT_IN_LIST_THAT(FE_element)(
				FE_element_is_wholly_within_element_list_tree, (void *)element_list,
				selected_element_list)))
			{
				return_code =
					FE_element_selection_unselect_element(element_selection, element);
			}
			FE_element_selection_end_cache(element_selection);
		}
		if (element_point_ranges_selection)
		{
			/* remove all references to elements being removed from the global
				 element_point_ranges_selection */
			Element_point_ranges_selection_begin_cache(
				element_point_ranges_selection);
			selected_element_point_ranges_list=
				Element_point_ranges_selection_get_element_point_ranges_list(
					element_point_ranges_selection);
			while (return_code&&(element_point_ranges=
				FIRST_OBJECT_IN_LIST_THAT(Element_point_ranges)(
					Element_point_ranges_is_wholly_within_element_list_tree,
					(void *)element_list, selected_element_point_ranges_list)))
			{
				return_code =
					Element_point_ranges_selection_unselect_element_point_ranges(
						element_point_ranges_selection, element_point_ranges);
			}
			Element_point_ranges_selection_end_cache(element_point_ranges_selection);
		}
		/* now remove the elements from the manager */
		MANAGER_BEGIN_CACHE(FE_element)(element_manager);
		number_of_elements_destroyed = 0;
		while (return_code && (element = FIRST_OBJECT_IN_LIST_THAT(FE_element)(
			(LIST_CONDITIONAL_FUNCTION(FE_element) *)NULL, (void *)NULL,
			element_list)))
		{
			/* element cannot be destroyed while it is in a list */
			if (REMOVE_OBJECT_FROM_LIST(FE_element)(element, element_list))
			{
				if (MANAGED_OBJECT_NOT_IN_USE(FE_element)(element, element_manager))
				{
					if (return_code =
						remove_FE_element_and_faces_from_manager(element, element_manager))
					{
						number_of_elements_destroyed++;
					}
				}
				else
				{
					/* add it to not_destroyed_element_list for reporting */
					ADD_OBJECT_TO_LIST(FE_element)(element, not_destroyed_element_list);
				}
			}
			else
			{
				return_code = 0;
			}
		}
		MANAGER_END_CACHE(FE_element)(element_manager);
		if (0 < (number_of_elements_not_destroyed =
			NUMBER_IN_LIST(FE_element)(not_destroyed_element_list)))
		{
			display_message(WARNING_MESSAGE, "%d element(s) destroyed; "
				"%d element(s) could not be destroyed because in use",
				number_of_elements_destroyed,number_of_elements_not_destroyed);
			return_code = 0;
		}
		FOR_EACH_OBJECT_IN_LIST(FE_element)(ensure_FE_element_is_in_list,
			(void *)element_list, not_destroyed_element_list);
		DESTROY(LIST(FE_element))(&not_destroyed_element_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_listed_elements.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* destroy_listed_elements */
