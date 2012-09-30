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
#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (1) */
#include "api/cmiss_rendition.h"
#include "command/command.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_group.h"
#include "api/cmiss_field_subobject_group.h"
#include "element/element_operations.h"
#include "element/element_tool.h"
#include "finite_element/finite_element_discretization.h"
#include "finite_element/finite_element_region.h"
#include "general/debug.h"
#include "graphics/scene.h"
#include "interaction/interaction_graphics.h"
#include "interaction/interaction_volume.h"
#include "interaction/interactive_event.h"
#include "graphics/rendition.h"
#include "graphics/graphic.h"
#include "region/cmiss_region.h"
#include "time/time_keeper.h"
#include "general/message.h"
#if defined (WX_USER_INTERFACE)
#include "wx/wx.h"
#include <wx/tglbtn.h>
#include "wx/xrc/xmlres.h"
#include "element/element_tool.xrch"
#include "graphics/graphics_window_private.hpp"
#include "choose/choose_manager_class.hpp"
#endif /* defined (WX_USER_INTERFACE)*/
#include <map>
typedef std::multimap<Cmiss_region *, Cmiss_element_id> Region_element_map;

/*
Module variables
----------------
*/

static char Interactive_tool_element_type_string[] = "element_tool";


/*
Module types
------------
*/
#if defined (WX_USER_INTERFACE)
class wxElementTool;
#endif /* defined (WX_USER_INTERFACE) */

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
	struct Element_point_ranges_selection *element_point_ranges_selection;
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
	struct Cmiss_rendition *rendition;

#if defined (WX_USER_INTERFACE)
	wxElementTool *wx_element_tool;
	wxPoint tool_position;
#endif /* defined (WX_USER_INTERFACE) */
}; /* struct Element_tool */

/*
Module functions
----------------
*/

static int Cmiss_field_group_destroy_all_elements(Cmiss_field_group_id group, void *dummy_void)
{
	USE_PARAMETER(dummy_void);
	int return_code = 0;
	if (group)
	{
		Cmiss_field_module_id field_module = Cmiss_field_get_field_module(Cmiss_field_group_base_cast(group));
		for (int i = 1; i <= 3; i++)
		{
			Cmiss_mesh_id master_mesh =
				Cmiss_field_module_find_mesh_by_dimension(field_module, /*dimension*/i);
			Cmiss_field_element_group_id element_group = Cmiss_field_group_get_element_group(group, master_mesh);
			Cmiss_mesh_destroy(&master_mesh);
			if (element_group)
			{
				Cmiss_mesh_group_id mesh_group = Cmiss_field_element_group_get_mesh(element_group);
				Cmiss_mesh_destroy_all_elements(Cmiss_mesh_group_base_cast(mesh_group));
				Cmiss_mesh_group_destroy(&mesh_group);
				Cmiss_field_element_group_destroy(&element_group);
			}
		}
		Cmiss_field_module_destroy(&field_module);
		return_code = 1;
	}
	return return_code;
}

int Element_tool_destroy_selected_elements(struct Element_tool *element_tool)
{
	int return_code = 0;
	if (element_tool->region)
	{
		return_code = 1;
		Cmiss_rendition *root_rendition = Cmiss_region_get_rendition_internal(
			element_tool->region);
		Cmiss_field_group_id selection_group = Cmiss_rendition_get_selection_group(root_rendition);
		if (selection_group)
		{
			return_code = Cmiss_field_group_for_each_group_hierarchical(selection_group,
				Cmiss_field_group_destroy_all_elements, /*user_data*/(void *)0);
			Cmiss_field_group_clear_region_tree_element(selection_group);
			Cmiss_rendition_flush_tree_selections(root_rendition);
			Cmiss_field_group_destroy(&selection_group);
		}
		Cmiss_rendition_destroy(&root_rendition);
	}
	return return_code;
}

#if defined (OPENGL_API)
static void Element_tool_reset(void *element_tool_void)
/*******************************************************************************
LAST MODIFIED : 25 February 2008

DESCRIPTION :
Resets current edit. Called on button release or when tool deactivated.
==============================================================================*/
{
	struct Element_tool *element_tool;

	ENTER(Element_tool_reset);
	element_tool = (struct Element_tool *)element_tool_void;
	if (element_tool != 0)
	{
		REACCESS(FE_element)(&(element_tool->last_picked_element),
			(struct FE_element *)NULL);
		REACCESS(Cmiss_rendition)(&(element_tool->rendition),
			(struct Cmiss_rendition *)NULL);
		REACCESS(Interaction_volume)(
			&(element_tool->last_interaction_volume),
			(struct Interaction_volume *)NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Element_tool_reset.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_tool_reset */
#endif /* defined (OPENGL_API) */

#if defined (OPENGL_API)
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
	enum Interactive_event_type event_type;
	FE_value time, xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int clear_selection, element_dimension, i, input_modifier,
		number_of_xi_points, shift_pressed;
	int number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct FE_element *picked_element;
	struct FE_element_shape *element_shape;
	struct Element_tool *element_tool;
	struct Interaction_volume *interaction_volume,*temp_interaction_volume;
	struct LIST(Scene_picked_object) *scene_picked_object_list;
	struct Scene *scene;
	struct Cmiss_rendition *rendition = NULL;
	FE_value_triple *xi_points;

	ENTER(Element_tool_interactive_event_handler);
	if (device_id&&event&&(element_tool=
		(struct Element_tool *)element_tool_void))
	{
		Cmiss_region_begin_hierarchical_change(element_tool->region);
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
							element_tool->picked_element_was_unselected=1;
							if (0 != (picked_element=Scene_picked_object_list_get_nearest_element(
								scene_picked_object_list,(struct Cmiss_region *)NULL,
								element_tool->select_elements_enabled,
								element_tool->select_faces_enabled,
								element_tool->select_lines_enabled,
								(struct Scene_picked_object **)NULL,
								&rendition,	(struct Cmiss_graphic **)NULL)))
							{
								/* Open command_field of picked_element in browser */
								if (element_tool->command_field)
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
										Cmiss_field_module_id field_module = Cmiss_field_get_field_module(element_tool->command_field);
										Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
										Cmiss_field_cache_set_time(field_cache, time);
										Cmiss_field_cache_set_mesh_location(field_cache, picked_element, element_dimension, xi);
										char *command_string = Cmiss_field_evaluate_string(element_tool->command_field, field_cache);
										if (command_string)
										{
											Execute_command_execute_string(element_tool->execute_command, command_string);
											DEALLOCATE(command_string);
										}
										Cmiss_field_cache_destroy(&field_cache);
										Cmiss_field_module_destroy(&field_module);
										DEALLOCATE(xi_points);
									}
								}
								Cmiss_field_group_id group = Cmiss_rendition_get_selection_group(rendition);
								if (group)
								{
									Cmiss_region_id temp_region = Cmiss_rendition_get_region(rendition);
									Cmiss_field_module_id field_module = Cmiss_region_get_field_module(temp_region);
									int dimension = Cmiss_element_get_dimension(picked_element);
									Cmiss_mesh_id master_mesh = Cmiss_field_module_find_mesh_by_dimension(field_module, dimension);
									Cmiss_field_element_group_id element_group = Cmiss_field_group_get_element_group(group, master_mesh);
									Cmiss_mesh_destroy(&master_mesh);
									if (element_group)
									{
										Cmiss_mesh_group_id mesh_group = Cmiss_field_element_group_get_mesh(element_group);
										element_tool->picked_element_was_unselected =
											!Cmiss_mesh_contains_element(Cmiss_mesh_group_base_cast(mesh_group), picked_element);
										Cmiss_mesh_group_destroy(&mesh_group);
										Cmiss_field_element_group_destroy(&element_group);
									}
									Cmiss_field_group_destroy(&group);
									Cmiss_field_module_destroy(&field_module);
								}
							}
							REACCESS(FE_element)(&(element_tool->last_picked_element),
								picked_element);
							/*(if ((clear_selection = !shift_pressed)
								&&((!picked_element)||
								(element_tool->picked_element_was_unselected)))*/
							clear_selection = !shift_pressed;
							if (clear_selection)
							{
								if (element_tool->region)
								{
									Cmiss_rendition *root_rendition =
										Cmiss_region_get_rendition_internal(element_tool->region);
									Cmiss_field_group_id root_group =
										Cmiss_rendition_get_selection_group(root_rendition);
									if (root_group)
									{
										Cmiss_field_group_clear_region_tree_element(root_group);
										Cmiss_field_group_destroy(&root_group);
									}
									Cmiss_rendition_destroy(&root_rendition);
								}
							}
							if (picked_element)
							{
								REACCESS(Cmiss_rendition)(&(element_tool->rendition),
									rendition);
								Cmiss_region *sub_region = NULL;
								Cmiss_field_group_id sub_group = NULL;
								Cmiss_mesh_group_id mesh_group = 0;
								if (element_tool->rendition)
								{
									sub_region = Cmiss_rendition_get_region(element_tool->rendition);
									sub_group = Cmiss_rendition_get_or_create_selection_group(element_tool->rendition);
									if (sub_group)
									{
										int dimension = Cmiss_element_get_dimension(picked_element);
										Cmiss_field_module_id field_module = Cmiss_region_get_field_module(sub_region);
										Cmiss_mesh_id temp_mesh =
											Cmiss_field_module_find_mesh_by_dimension(field_module, dimension);
										Cmiss_field_element_group_id element_group = Cmiss_field_group_get_element_group(sub_group, temp_mesh);
										if (!element_group)
											element_group = Cmiss_field_group_create_element_group(sub_group, temp_mesh);
										mesh_group = Cmiss_field_element_group_get_mesh(element_group);
										Cmiss_field_element_group_destroy(&element_group);
										Cmiss_mesh_destroy(&temp_mesh);
										Cmiss_field_module_destroy(&field_module);
									}
								}
								if (mesh_group)
								{
									Cmiss_mesh_group_add_element(mesh_group, picked_element);
									Cmiss_mesh_group_destroy(&mesh_group);
								}
								if (sub_group)
								{
									Cmiss_field_group_destroy(&sub_group);
								}
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
								struct LIST(FE_element) *temp_element_list = CREATE(LIST(FE_element))();
								ADD_OBJECT_TO_LIST(FE_element)(element_tool->last_picked_element, temp_element_list);
								Cmiss_rendition_remove_selection_from_element_list_of_dimension(element_tool->rendition,
									temp_element_list, Cmiss_element_get_dimension(element_tool->last_picked_element));
								DESTROY(LIST(FE_element))(&temp_element_list);
							}
						}
						else if (element_tool->motion_detected)
						{
							/* rubber band select */
							temp_interaction_volume=
								create_Interaction_volume_bounding_box(
								element_tool->last_interaction_volume,interaction_volume);
							if (temp_interaction_volume != 0)
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
#if defined (USE_SCENE_OBJECT)
										Scene_add_graphics_object(scene,element_tool->rubber_band,
											/*position*/0,"element_tool_rubber_band",
											/*fast_changing*/1);
#endif
									}
									Interaction_volume_make_polyline_extents(
										temp_interaction_volume,element_tool->rubber_band);
								}
								else
								{
#if defined (USE_SCENE_OBJECT)
									Scene_remove_graphics_object(scene,element_tool->rubber_band);
#endif
									DEACCESS(GT_object)(&(element_tool->rubber_band));
								}
								if (INTERACTIVE_EVENT_BUTTON_RELEASE==event_type)
								{
									scene_picked_object_list=
										Scene_pick_objects(scene,temp_interaction_volume,
										graphics_buffer);
									if (scene_picked_object_list != 0)
									{
										Region_element_map *element_map =
											(Region_element_map *)Scene_picked_object_list_get_picked_region_sorted_elements(
											scene_picked_object_list,
											element_tool->select_elements_enabled,
											element_tool->select_faces_enabled,
											element_tool->select_lines_enabled);
										if (element_map)
										{
											Cmiss_region *sub_region = NULL;
											Cmiss_field_group_id sub_group = NULL;
											Cmiss_rendition *region_rendition = NULL;
											Cmiss_mesh_group_id mesh_group[MAXIMUM_ELEMENT_XI_DIMENSIONS];
											int iter = 0;
											for (iter = 0; iter < MAXIMUM_ELEMENT_XI_DIMENSIONS; iter++)
											{
												mesh_group[iter] = 0;
											}
											Region_element_map::iterator pos;
											for (pos = element_map->begin(); pos != element_map->end(); ++pos)
											{
												if (pos->first != sub_region)
												{
													if (sub_group)
													{
														Cmiss_field_group_destroy(&sub_group);
													}
													for (iter = 0; iter < MAXIMUM_ELEMENT_XI_DIMENSIONS; iter++)
													{
														if (mesh_group[iter])
														{
															Cmiss_mesh_group_destroy(&(mesh_group[iter]));
														}
													}
													if (region_rendition)
													{
														Cmiss_rendition_destroy(&region_rendition);
													}
													sub_region = pos->first;
													if (sub_region)
													{
														region_rendition= Cmiss_region_get_rendition_internal(sub_region);
														sub_group = Cmiss_rendition_get_or_create_selection_group(region_rendition);
													}
												}
												if (sub_region && sub_group)
												{
													Cmiss_field_module_id field_module = Cmiss_region_get_field_module(sub_region);
													int dimension = Cmiss_element_get_dimension(pos->second);
													if (dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS)
													{
														if (!mesh_group[dimension - 1])
														{
															Cmiss_mesh_id temp_mesh =
																Cmiss_field_module_find_mesh_by_dimension(field_module, dimension);
															Cmiss_field_element_group_id element_group =
																Cmiss_field_group_get_element_group(sub_group, temp_mesh);
															if (!element_group)
															{
																element_group = Cmiss_field_group_create_element_group(sub_group, temp_mesh);
															}
															mesh_group[dimension - 1] = Cmiss_field_element_group_get_mesh(element_group);
															Cmiss_field_element_group_destroy(&element_group);
															Cmiss_mesh_destroy(&temp_mesh);
														}
														Cmiss_mesh_group_add_element(mesh_group[dimension - 1], pos->second);
													}
													Cmiss_field_module_destroy(&field_module);
												}
											}
											if (sub_group)
											{
												Cmiss_field_group_destroy(&sub_group);
											}
											for (iter = 0; iter < MAXIMUM_ELEMENT_XI_DIMENSIONS; iter++)
											{
												if (mesh_group[iter])
												{
													Cmiss_mesh_group_destroy(&(mesh_group[iter]));
												}
											}
											if (region_rendition)
											{
												Cmiss_rendition_destroy(&region_rendition);
											}
											delete element_map;
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
							Element_tool_reset((void *)element_tool);
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
		if (element_tool->region)
		{
			Cmiss_rendition *root_rendition = Cmiss_region_get_rendition_internal(
				element_tool->region);
			Cmiss_rendition_flush_tree_selections(root_rendition);
			Cmiss_rendition_destroy(&root_rendition);
		}
		Cmiss_region_end_hierarchical_change(element_tool->region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_interactive_event_handler.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_tool_interactive_event_handler */
#endif /* defined (OPENGL_API) */

#if defined (OPENGL_API)
static int Element_tool_bring_up_interactive_tool_dialog(
	void *element_tool_void,struct Graphics_window *graphics_window)
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
		Element_tool_pop_up_dialog((struct Element_tool *)element_tool_void,graphics_window);
	LEAVE;

	return (return_code);
} /* Element_tool_bring_up_interactive_tool_dialog */
#endif /* defined (OPENGL_API) */

#if defined (OPENGL_API)
static int Element_tool_destroy_element_tool(void **element_tool_void)
/*******************************************************************************
LAST MODIFIED : 6 July 2007

DESCRIPTION :
Function to call DESTROY
==============================================================================*/
{
	ENTER(element_point_tool_destroy_element_point_tool);
	Element_tool *element_tool;
	int return_code;
	return_code=0;

	element_tool = (struct Element_tool *)*element_tool_void;
	if (element_tool != 0)
	{
		return_code = DESTROY(Element_tool)(&element_tool);
	}
	LEAVE;
	return (return_code);
}
#endif

#if defined (WX_USER_INTERFACE)
class wxElementTool : public wxPanel
{
	Element_tool *element_tool;
	wxCheckBox *button_element;
	wxCheckBox *button_face;
	wxCheckBox *button_line;
	wxCheckBox *elementcommandfieldcheckbox;
	wxPanel *element_command_field_chooser_panel;

	wxButton *button_destroy;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct FE_region *fe_region;
	struct Computed_field *command_field;
	DEFINE_MANAGER_CLASS(Computed_field);
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
		*element_command_field_chooser;

public:

	wxElementTool(Element_tool *element_tool, wxPanel *parent):
	element_tool(element_tool)
	{
		wxXmlInit_element_tool();
		wxXmlResource::Get()->LoadPanel(this,parent,_T("CmguiElementTool"));
		elementcommandfieldcheckbox = XRCCTRL(*this, "ElementCommandFieldCheckBox",wxCheckBox);
		element_command_field_chooser_panel = XRCCTRL(*this, "ElementCommandFieldChooserPanel", wxPanel);
		if (element_tool->region)
		{
			computed_field_manager=
				Cmiss_region_get_Computed_field_manager(element_tool->region);
		}
		else
		{
			computed_field_manager=NULL;
		}
		element_command_field_chooser =
			new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
			(element_command_field_chooser_panel, element_tool->command_field, computed_field_manager,
			Computed_field_has_string_value_type, (void *)NULL, element_tool->user_interface);
		Callback_base< Computed_field* > *element_command_field_callback =
			new Callback_member_callback< Computed_field*,
			wxElementTool, int (wxElementTool::*)(Computed_field *) >
			(this, &wxElementTool::element_command_field_callback);
		element_command_field_chooser->set_callback(element_command_field_callback);
		if (element_tool != NULL)
		{
			command_field = Element_tool_get_command_field(element_tool);
			element_command_field_chooser->set_object(command_field);
			if (command_field == NULL)
				{
					elementcommandfieldcheckbox->SetValue(0);
					element_command_field_chooser_panel->Disable();
				}
			else
				{
					elementcommandfieldcheckbox->SetValue(1);
					element_command_field_chooser_panel->Enable();
				}
		}
		button_face = XRCCTRL(*this, "ButtonFace", wxCheckBox);
		button_element = XRCCTRL(*this, "ButtonElement", wxCheckBox);
		button_line = XRCCTRL(*this, "ButtonLine", wxCheckBox);
		button_element->SetValue(element_tool->select_elements_enabled);
		button_face->SetValue(element_tool->select_faces_enabled);
		button_line->SetValue(element_tool->select_lines_enabled);
	};

	wxElementTool()
	{
	};

	~ wxElementTool()
	{
		delete element_command_field_chooser;
	};
	int element_command_field_callback(Computed_field *command_field)
	{
		Element_tool_set_command_field(element_tool, command_field);
		return 1;
	}

	void OnButtonElementpressed(wxCommandEvent& event)
	{
		USE_PARAMETER(event);
		button_element = XRCCTRL(*this, "ButtonElement", wxCheckBox);
		Element_tool_set_select_elements_enabled(element_tool,
			button_element->IsChecked());
	}

	void OnButtonFacepressed(wxCommandEvent& event)
	{
		USE_PARAMETER(event);
		button_face = XRCCTRL(*this, "ButtonFace", wxCheckBox);
		Element_tool_set_select_faces_enabled(element_tool,
			button_face->IsChecked());
	}

	void OnButtonLinepressed(wxCommandEvent& event)
	{
		USE_PARAMETER(event);
		button_line = XRCCTRL(*this, "ButtonLine", wxCheckBox);
		Element_tool_set_select_lines_enabled(element_tool,
			button_line->IsChecked());
	}

	void OnButtonDestroypressed(wxCommandEvent& event)
	{
		USE_PARAMETER(event);
		Element_tool_destroy_selected_elements(element_tool);
	}

	void ElementToolInterfaceRenew(Element_tool *destination_element_tool)
	{
		button_element = XRCCTRL(*this, "ButtonElement", wxCheckBox);
		button_face = XRCCTRL(*this, "ButtonFace", wxCheckBox);
		button_line = XRCCTRL(*this, "ButtonLine", wxCheckBox);
		button_element->SetValue(destination_element_tool-> select_elements_enabled);
		button_face->SetValue(destination_element_tool->select_faces_enabled);
		button_line->SetValue(destination_element_tool->select_lines_enabled);
	}

	void ElementCommandFieldChecked(wxCommandEvent &event)
	{
		USE_PARAMETER(event);
		struct Computed_field *command_field;
		elementcommandfieldcheckbox = XRCCTRL(*this, "ElementCommandFieldCheckBox",wxCheckBox);
		element_command_field_chooser_panel = XRCCTRL(*this, "ElementCommandFieldChooserPanel", wxPanel);
		if (elementcommandfieldcheckbox->IsChecked())
		{
			if (element_tool)
			{
				if (Element_tool_get_command_field(element_tool))
				{
					Element_tool_set_command_field(element_tool, (struct Computed_field *)NULL);
					element_command_field_chooser_panel->Enable();
				}
				else
				{
					/* get label field from widget */
					if (element_command_field_chooser->get_number_of_object() > 0)
					{
						command_field = element_command_field_chooser->get_object();
						if (command_field)
							 {
								 Element_tool_set_command_field(element_tool, command_field);
							 }
					}
					else
					{
						elementcommandfieldcheckbox->SetValue(0);
						element_command_field_chooser_panel->Disable();
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Elementt_tool_command_field_button_CB.  Invalid argument(s)");
			}
		}
		else
		{
			Element_tool_set_command_field(element_tool, (struct Computed_field *)NULL);
			elementcommandfieldcheckbox->SetValue(0);
			element_command_field_chooser_panel->Disable();
		}
	}

	DECLARE_DYNAMIC_CLASS(wxElementTool);
	DECLARE_EVENT_TABLE();
};

IMPLEMENT_DYNAMIC_CLASS(wxElementTool, wxPanel)

BEGIN_EVENT_TABLE(wxElementTool, wxPanel)
	EVT_CHECKBOX(XRCID("ButtonElement"),wxElementTool::OnButtonElementpressed)
	EVT_CHECKBOX(XRCID("ButtonFace"),wxElementTool::OnButtonFacepressed)
	EVT_CHECKBOX(XRCID("ButtonLine"),wxElementTool::OnButtonLinepressed)
	EVT_CHECKBOX(XRCID("ElementCommandFieldCheckBox"),wxElementTool::ElementCommandFieldChecked)
	EVT_BUTTON(XRCID("ButtonDestroy"),wxElementTool::OnButtonDestroypressed)
END_EVENT_TABLE()


#endif /* defined (WX_USER_INTERFACE) */

/*
Global functions
----------------
*/
static int Element_tool_copy_function(
	void *destination_tool_void, void *source_tool_void,
	struct MANAGER(Interactive_tool) *destination_tool_manager)
/*******************************************************************************
LAST MODIFIED : 29 March 2007

DESCRIPTION :
Copies the state of one element tool to another.WX only
==============================================================================*/
{
	int return_code;
	struct Element_tool *destination_element_tool, *source_element_tool;
	ENTER(Element_tool_copy_function);
	if ((destination_tool_void || destination_tool_manager) &&
		(source_element_tool=(struct Element_tool *)source_tool_void))
	{
		if (destination_tool_void)
		{
			destination_element_tool = (struct Element_tool *)destination_tool_void;
		}
		else
		{
			destination_element_tool = CREATE(Element_tool)
				(destination_tool_manager,
				source_element_tool->region,
				source_element_tool->element_point_ranges_selection,
				source_element_tool->rubber_band_material,
				source_element_tool->user_interface,
				source_element_tool->time_keeper);
			Element_tool_set_execute_command(destination_element_tool,
				source_element_tool->execute_command);
		}
		if (destination_element_tool)
		{
			destination_element_tool->select_elements_enabled = source_element_tool->select_elements_enabled;
			destination_element_tool->select_faces_enabled = source_element_tool->select_faces_enabled;
			destination_element_tool->select_lines_enabled = source_element_tool->select_lines_enabled;
			destination_element_tool->command_field = source_element_tool->command_field;
#if defined (WX_USER_INTERFACE)
			if (destination_element_tool->wx_element_tool != (wxElementTool *) NULL)
			{
				destination_element_tool->wx_element_tool->ElementToolInterfaceRenew(destination_element_tool);
			}
#endif /*defined (WX_USER_INTERFACE)*/
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Element_tool_copy_function.  Could not create copy.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_copy_function.  Invalid argument(s)");
		return_code=0;
	}
	return (return_code);
}

struct Element_tool *CREATE(Element_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct Cmiss_region *region,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Graphical_material *rubber_band_material,
	struct User_interface *user_interface,
	struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Creates an Element_tool with Interactive_tool in <interactive_tool_manager>.
Selects elements in <element_selection> in response to interactive_events.
==============================================================================*/
{
	struct Element_tool *element_tool;
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(CREATE(Element_tool));
	element_tool=(struct Element_tool *)NULL;
	if (interactive_tool_manager && region &&(NULL != (computed_field_manager=
		Cmiss_region_get_Computed_field_manager(region)))
		&&rubber_band_material&&user_interface)
	{
		if (ALLOCATE(element_tool,struct Element_tool,1))
		{
			element_tool->execute_command=NULL;
			element_tool->interactive_tool_manager=interactive_tool_manager;
			element_tool->region = region;
			element_tool->element_point_ranges_selection=
				element_point_ranges_selection;
			element_tool->rubber_band_material=
				ACCESS(Graphical_material)(rubber_band_material);
			element_tool->user_interface=user_interface;
			element_tool->time_keeper = (struct Time_keeper *)NULL;
			element_tool->rendition=(struct Cmiss_rendition *)NULL;
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
#if defined (OPENGL_API)
			element_tool->interactive_tool=CREATE(Interactive_tool)(
				"element_tool","Element tool",
				Interactive_tool_element_type_string,
				Element_tool_interactive_event_handler,
				Element_tool_bring_up_interactive_tool_dialog,
				Element_tool_reset,
				Element_tool_destroy_element_tool,
				Element_tool_copy_function,
				(void *)element_tool);
#else /* defined (OPENGL_API) */
			element_tool->interactive_tool=CREATE(Interactive_tool)(
				"element_tool","Element tool",
				Interactive_tool_element_type_string,
				(Interactive_event_handler*)NULL,
				(Interactive_tool_bring_up_dialog_function*)NULL,
				(Interactive_tool_reset_function*)NULL,
				(Interactive_tool_destroy_tool_data_function *)NULL,
				Element_tool_copy_function,
				(void *)element_tool);
#endif /* defined (OPENGL_API) */
			ADD_OBJECT_TO_MANAGER(Interactive_tool)(
				element_tool->interactive_tool,
				element_tool->interactive_tool_manager);
			element_tool->last_picked_element=(struct FE_element *)NULL;
			element_tool->last_interaction_volume=(struct Interaction_volume *)NULL;
			element_tool->rubber_band=(struct GT_object *)NULL;
#if defined (WX_USER_INTERFACE) /* switch (USER_INTERFACE) */
			element_tool->wx_element_tool=(wxElementTool *)NULL;
#endif /* defined (WX_USER_INTERFACE) */
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
		REACCESS(Interaction_volume)(&(element_tool->last_interaction_volume),
			(struct Interaction_volume *)NULL);
		REACCESS(GT_object)(&(element_tool->rubber_band),(struct GT_object *)NULL);
		DEACCESS(Graphical_material)(&(element_tool->rubber_band_material));
		if (element_tool->time_keeper)
		{
			DEACCESS(Time_keeper)(&(element_tool->time_keeper));
		}
#if defined (WX_USER_INTERFACE)
		if (element_tool->wx_element_tool)
			element_tool->wx_element_tool->Destroy();
#endif /*(WX_USER_INTERFACE)*/
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

int Element_tool_pop_up_dialog(struct Element_tool *element_tool, struct Graphics_window *graphics_window)
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
#if defined (WX_USER_INTERFACE) /* switch (USER_INTERFACE) */
		wxPanel *pane;
		if (!element_tool->wx_element_tool)
		{
			element_tool->wx_element_tool = new wxElementTool(element_tool,
				Graphics_window_get_interactive_tool_panel(graphics_window));
			pane = XRCCTRL(*element_tool->wx_element_tool, "CmguiElementTool", wxPanel);
			element_tool->tool_position = pane->GetPosition();
			element_tool->wx_element_tool->Show();
		}
		else
		{
			pane = XRCCTRL(*element_tool->wx_element_tool, "CmguiElementTool", wxPanel);
			pane->SetPosition(element_tool->tool_position);
			element_tool->wx_element_tool->Show();
		}
#else /* switch (USER_INTERFACE) */
		USE_PARAMETER(graphics_window);
		display_message(ERROR_MESSAGE, "Element_tool_pop_up_dialog.  "
			"No dialog implemented for this User Interface");
#endif /*  switch (USER_INTERFACE) */
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
	int return_code;

	ENTER(Element_tool_set_command_field);
	if (element_tool && ((!command_field) ||
		Computed_field_has_string_value_type(command_field, (void *)NULL)))
	{
		return_code = 1;
		if (command_field != element_tool->command_field)
		{
			element_tool->command_field = command_field;
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

struct Interactive_tool *Element_tool_get_interactive_tool(
struct Element_tool *element_tool)
	/*******************************************************************************
	LAST MODIFIED : 29 March 2007

	DESCRIPTION :
	Returns the generic interactive_tool the represents the <element_tool>.
	==============================================================================*/
{
	struct Interactive_tool *interactive_tool;

	ENTER(Element_tool_get_interactive_tool);
	if (element_tool)
	{
		interactive_tool=element_tool->interactive_tool;
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

int Element_tool_set_execute_command(struct Element_tool *element_tool,
struct Execute_command *execute_command)
{
	int return_code = 0;
	if (element_tool)
	{
		element_tool->execute_command = execute_command;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_set_execute_command.  Invalid argument(s)");
	}

	return return_code;
}
