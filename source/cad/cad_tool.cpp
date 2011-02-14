/*******************************************************************************
FILE : cad_tool.cpp

CREATED : 20 June 2010

DESCRIPTION :
Interactive tool for selecting cad primitives with a mouse and other devices.
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
#if defined (BUILD_WITH_CMAKE)
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */
extern "C" {
#include "api/cmiss_rendition.h"
#include "command/command.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_group.h"
#include "api/cmiss_field_sub_group.h"
#include "element/element_operations.h"
#include "finite_element/finite_element_discretization.h"
#include "finite_element/finite_element_region.h"
#include "general/debug.h"
#include "graphics/scene.h"
#include "help/help_interface.h"
#include "interaction/interaction_graphics.h"
#include "interaction/interaction_volume.h"
#include "interaction/interactive_event.h"
#include "graphics/rendition.h"
#include "graphics/graphic.h"
#include "region/cmiss_region.h"
#include "time/time_keeper.h"
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"
}
#if defined (USE_OPENCASCADE)
#include "api/cmiss_field_cad.h"
#include "cad/cad_tool.h"
#include "cad/element_identifier.h"
#include "cad/computed_field_cad_topology.h"
#endif /* defined (USE_OPENCASCADE) */
#include "computed_field/computed_field_private.hpp"
#if defined (WX_USER_INTERFACE)
#include "wx/wx.h"
#include <wx/tglbtn.h>
#include "wx/xrc/xmlres.h"
#include "cad/cad_tool.xrch"
#include "graphics/graphics_window_private.hpp"
#include "choose/choose_manager_class.hpp"
#endif /* defined (WX_USER_INTERFACE)*/

#include <map>
typedef std::multimap<Cmiss_region *, Cmiss_cad_identifier_id> Region_cad_primitive_map;

/*
Module variables
----------------
*/

static char Interactive_tool_element_type_string[] = "cad_tool";


/*
Module types
------------
*/
#if defined (WX_USER_INTERFACE)
class wxCadTool;
#endif /* defined (WX_USER_INTERFACE) */


struct Cad_tool
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
	int select_surfaces_enabled,select_lines_enabled;
	struct Computed_field *command_field;
	/* information about picked element */
	int picked_element_was_unselected;
	int motion_detected;
	struct FE_element *last_picked_element;
	struct Interaction_volume *last_interaction_volume;
	struct GT_object *rubber_band;
	struct Cmiss_rendition *rendition;

#if defined (WX_USER_INTERFACE)
	 wxCadTool *wx_cad_tool;
	 wxPoint tool_position;
#endif /* defined (WX_USER_INTERFACE) */
}; /* struct Cad_tool */

/*
Module functions
----------------
*/
#if defined (OPENGL_API)
static void Cad_tool_reset(void *cad_tool_void)
{
	struct Cad_tool *cad_tool;

	ENTER(Cad_tool_reset);
	if (NULL != (cad_tool = (struct Cad_tool *)cad_tool_void))
	{
		REACCESS(FE_element)(&(cad_tool->last_picked_element),
			(struct FE_element *)NULL);
		REACCESS(Cmiss_rendition)(&(cad_tool->rendition),
			(struct Cmiss_rendition *)NULL);
		REACCESS(Interaction_volume)(
			&(cad_tool->last_interaction_volume),
			(struct Interaction_volume *)NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cad_tool_reset.  Invalid argument(s)");
	}
	LEAVE;
} /* Cad_tool_reset */
#endif /* defined (OPENGL_API) */

#if defined (OPENGL_API)
static void Cad_tool_interactive_event_handler(void *device_id,
	struct Interactive_event *event,void *cad_tool_void,
	struct Graphics_buffer *graphics_buffer)
{
	enum Interactive_event_type event_type;
	//char *command_string;
	//FE_value time, xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int clear_selection, /*element_dimension, i,*/ input_modifier,
		/*number_of_xi_points,*/ shift_pressed;
	//int number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	//struct FE_element_shape *element_shape;
	//FE_value_triple *xi_points;
	Cmiss_cad_identifier_id picked_element;
	struct Cad_tool *cad_tool;
	struct Interaction_volume *interaction_volume,*temp_interaction_volume;
	struct LIST(Scene_picked_object) *scene_picked_object_list;
	struct Scene *scene;
	struct Cmiss_rendition *rendition = NULL;

	ENTER(Cad_tool_interactive_event_handler);
	if (device_id&&event&&(cad_tool=
		(struct Cad_tool *)cad_tool_void))
	{
		//Cmiss_region_begin_hierarchical_change(cad_tool->region);
		interaction_volume=Interactive_event_get_interaction_volume(event);
		if (NULL != (scene=Interactive_event_get_scene(event)))
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
						if (NULL != (scene_picked_object_list=
							Scene_pick_objects(scene,interaction_volume,graphics_buffer)))
						{
							clear_selection = !shift_pressed;
							if (clear_selection)
							{
								printf("--- clear selection\n");
								if (cad_tool->region)
								{
									Cmiss_rendition *rendition = 
										Cmiss_region_get_rendition_internal(cad_tool->region);
									Cmiss_field_group_id group = 
										Cmiss_rendition_get_or_create_selection_group(rendition);
									if (group)
									{
										Cmiss_field_group_clear_region_tree_cad_primitive(group);
										Cmiss_field_group_destroy(&group);
									}
									Cmiss_rendition_destroy(&rendition);
								}
							}
							cad_tool->picked_element_was_unselected=1;
							picked_element = Scene_picked_object_list_get_cad_primitive(
								scene_picked_object_list,(struct Cmiss_region *)NULL,
								cad_tool->select_surfaces_enabled,
								cad_tool->select_lines_enabled,
								(struct Scene_picked_object **)NULL,
								&rendition, (struct Cmiss_graphic **)NULL);
							if (picked_element)
							{
								//printf("Picked element = %d\n", picked_element->identifier.number);
								REACCESS(Cmiss_rendition)(&(cad_tool->rendition),
									rendition);
								Cmiss_region *sub_region = NULL;
								Cmiss_field_group_id sub_group = NULL;
								Cmiss_field_cad_primitive_group_template_id cad_primitive_group = NULL;
								if (cad_tool->rendition)
								{
									sub_region = Cmiss_rendition_get_region(cad_tool->rendition);
									sub_group = Cmiss_rendition_get_or_create_selection_group(cad_tool->rendition);
									if (sub_group)
									{
										//cad_primitive_group_field = Cmiss_field_group_get_subgroup_for_domain(sub_group,
										//	reinterpret_cast<Cmiss_field_id>(picked_element->cad_topology));
										cad_primitive_group = Cmiss_field_group_get_cad_primitive_group(sub_group, picked_element->cad_topology);
										if (cad_primitive_group == NULL)
											cad_primitive_group = Cmiss_field_group_create_cad_primitive_group(sub_group, picked_element->cad_topology);
									}
								}

								if (sub_region && cad_primitive_group)
								{
									if (Cmiss_field_cad_primitive_group_template_is_cad_primitive_selected(cad_primitive_group, picked_element))
										Cmiss_field_cad_primitive_group_template_remove_cad_primitive(cad_primitive_group, picked_element);
									else
										Cmiss_field_cad_primitive_group_template_add_cad_primitive(cad_primitive_group, picked_element);
									Computed_field_changed(Cmiss_field_group_base_cast(sub_group));
								}
								if (sub_group)
								{
									Cmiss_field_group_destroy(&sub_group);
								}
								if (cad_primitive_group)
								{
									Cmiss_field_cad_primitive_group_template_destroy(&cad_primitive_group);
								}
								//DEBUG_PRINT("picked element %d\n", Cmiss_field_get_access_count(reinterpret_cast<Cmiss_field_id>(picked_element->cad_topology)));
								delete picked_element;
							}
							DESTROY(LIST(Scene_picked_object))(&(scene_picked_object_list));
						}
						cad_tool->motion_detected=0;
						REACCESS(Interaction_volume)(
							&(cad_tool->last_interaction_volume),interaction_volume);
					}
				} break;
				case INTERACTIVE_EVENT_MOTION_NOTIFY:
				case INTERACTIVE_EVENT_BUTTON_RELEASE:
				{
					if (cad_tool->last_interaction_volume&&
						((INTERACTIVE_EVENT_MOTION_NOTIFY==event_type) ||
						(1==Interactive_event_get_button_number(event))))
					{
						if (INTERACTIVE_EVENT_MOTION_NOTIFY==event_type)
						{
							cad_tool->motion_detected=1;
						}
						if (cad_tool->last_picked_element)
						{
							//DEBUG_PRINT("last_picked_element\n");
						}
						else if (cad_tool->motion_detected)
						{
							/* rubber band select */
							if (NULL != (temp_interaction_volume=
								create_Interaction_volume_bounding_box(
								cad_tool->last_interaction_volume,interaction_volume)))
							{
								if (INTERACTIVE_EVENT_MOTION_NOTIFY==event_type)
								{
									if (!cad_tool->rubber_band)
									{
										/* create rubber_band object and put in scene */
										cad_tool->rubber_band=CREATE(GT_object)(
											"cad_tool_rubber_band",g_POLYLINE,
											cad_tool->rubber_band_material);
										ACCESS(GT_object)(cad_tool->rubber_band);
#if defined (USE_SCENE_OBJECT)
										Scene_add_graphics_object(scene,cad_tool->rubber_band,
											/*position*/0,"cad_tool_rubber_band",
											/*fast_changing*/1);
#endif
									}
									Interaction_volume_make_polyline_extents(
										temp_interaction_volume,cad_tool->rubber_band);
								}
								else
								{
#if defined (USE_SCENE_OBJECT)
									Scene_remove_graphics_object(scene,cad_tool->rubber_band);
#endif
									DEACCESS(GT_object)(&(cad_tool->rubber_band));
								}
								if (INTERACTIVE_EVENT_BUTTON_RELEASE==event_type)
								{
									if (NULL != (scene_picked_object_list=
										Scene_pick_objects(scene,temp_interaction_volume,
										graphics_buffer)))
									{
										Region_cad_primitive_map *cad_primitive_map = 
											(Region_cad_primitive_map *)Scene_picked_object_list_get_picked_region_cad_primitives(
												scene_picked_object_list,
												cad_tool->select_surfaces_enabled,
												cad_tool->select_lines_enabled);
										if (cad_primitive_map)
										{
											//DEBUG_PRINT("Hello region element map ------------------------\n");
											Cmiss_region *sub_region = NULL;
											Cmiss_field_group_id sub_group = NULL;
											Cmiss_rendition *region_rendition = NULL;
											Cmiss_field_cad_primitive_group_template_id cad_primitive_group = NULL;
											Region_cad_primitive_map::iterator pos;
											int count = 0;
											for (pos = cad_primitive_map->begin(); pos != cad_primitive_map->end(); ++pos)
											{
												//DEBUG_PRINT("\tprimitive %d\n", ++count);
												if (pos->first != sub_region)
												{
													if (sub_region && sub_group)
													{
														Computed_field_changed(Cmiss_field_group_base_cast(sub_group));
														Cmiss_field_group_destroy(&sub_group);
													}
													if (cad_primitive_group)
													{
														Cmiss_field_cad_primitive_group_template_destroy(&cad_primitive_group);
													}
													if (region_rendition)
													{
														Cmiss_rendition_begin_cache(region_rendition);
														Cmiss_rendition_end_cache(region_rendition);
														Cmiss_rendition_destroy(&region_rendition);
													}
													sub_region = pos->first;
													if (sub_region)
													{
														region_rendition = Cmiss_region_get_rendition_internal(sub_region);
														sub_group = Cmiss_rendition_get_or_create_selection_group(region_rendition);
													}
													if (sub_group)
													{
														cad_primitive_group = Cmiss_field_group_get_cad_primitive_group(sub_group, pos->second->cad_topology);
														if (cad_primitive_group == NULL)
															cad_primitive_group = Cmiss_field_group_create_cad_primitive_group(sub_group, pos->second->cad_topology);
													}
												}
												if (sub_region && cad_primitive_group)
												{
													Cmiss_field_cad_primitive_group_template_add_cad_primitive(cad_primitive_group,
														pos->second);
													delete pos->second;
												}
											}
											if (sub_region && sub_group)
											{
												Computed_field_changed(Cmiss_field_group_base_cast(sub_group));
												Cmiss_field_group_destroy(&sub_group);
											}
											if (cad_primitive_group)
											{
												Cmiss_field_cad_primitive_group_template_destroy(&cad_primitive_group);
											}
											if (region_rendition)
											{
												Cmiss_rendition_destroy(&region_rendition);
											}
											delete cad_primitive_map;
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
							Cad_tool_reset((void *)cad_tool);
						}
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Cad_tool_interactive_event_handler.  Unknown event type");
				} break;
			}
		}
		//Cmiss_region_end_hierarchical_change(cad_tool->region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cad_tool_interactive_event_handler.  Invalid argument(s)");
	}
	LEAVE;
} /* Cad_tool_interactive_event_handler */
#endif /* defined (OPENGL_API) */

#if defined (OPENGL_API)
static int Cad_tool_bring_up_interactive_tool_dialog(
	void *cad_tool_void,struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Brings up a dialog for editing settings of the Cad_tool - in a standard
format for passing to an Interactive_toolbar.
==============================================================================*/
{
	int return_code;

	ENTER(Cad_tool_bring_up_interactive_tool_dialog);
	return_code =
		Cad_tool_pop_up_dialog((struct Cad_tool *)cad_tool_void,graphics_window);
	LEAVE;

	return (return_code);
} /* Cad_tool_bring_up_interactive_tool_dialog */
#endif /* defined (OPENGL_API) */

static struct Cmgui_image *Cad_tool_get_icon(struct Colour *foreground, 
	struct Colour *background, void *cad_tool_void)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Fetches a ToggleButton with an appropriate icon for the interactive tool
and as a child of <parent>.
==============================================================================*/
{
	struct Cmgui_image *image = NULL;
	struct Cad_tool *cad_tool;

	ENTER(Cad_tool_get_icon);
	if ((cad_tool=(struct Cad_tool *)cad_tool_void))
	{
		USE_PARAMETER(foreground);
		USE_PARAMETER(background);
		USE_PARAMETER(cad_tool);
		display_message(WARNING_MESSAGE, "Cad_tool_get_icon.  "
			"Not implemented for this user interface.");
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cad_tool_get_icon.  Invalid argument(s)");
		image = (struct Cmgui_image *)NULL;
	}
	LEAVE;

	return (image);
} /* Cad_tool_get_icon */

#if defined (OPENGL_API)
static int Cad_tool_destroy_cad_tool(void **cad_tool_void)
/*******************************************************************************
LAST MODIFIED : 6 July 2007

DESCRIPTION :
Function to call DESTROY
==============================================================================*/
{
	ENTER(element_point_tool_destroy_element_point_tool);
	Cad_tool *cad_tool;
	int return_code;
	return_code=0;

	if (NULL != (cad_tool = (struct Cad_tool *)*cad_tool_void))
	{
		 return_code = DESTROY(Cad_tool)(&cad_tool);
	}
	LEAVE;
	return (return_code);
}
#endif

#if defined (WX_USER_INTERFACE)
class wxCadTool : public wxPanel
{
	Cad_tool *cad_tool;
	wxCheckBox *button_surface;
	wxCheckBox *button_line;
	wxCheckBox *cad_command_field_checkbox;
	wxPanel *cad_command_field_chooser_panel;

	wxButton *button_destroy;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct FE_region *fe_region;
	struct Computed_field *command_field;
	DEFINE_MANAGER_CLASS(Computed_field);
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	*cad_command_field_chooser;

public:

	wxCadTool(Cad_tool *cad_tool, wxPanel *parent)
		: cad_tool(cad_tool)
	{
		wxXmlInit_cad_tool();
		wxXmlResource::Get()->LoadPanel(this,parent,_T("CmguiCadTool"));
		cad_command_field_checkbox = XRCCTRL(*this, "CadCommandFieldCheckBox",wxCheckBox);
		cad_command_field_chooser_panel = XRCCTRL(*this, "CadCommandFieldChooserPanel", wxPanel);
		if (cad_tool->region)
		{
			computed_field_manager=
				Cmiss_region_get_Computed_field_manager(cad_tool->region);
		}
		else
		{
			computed_field_manager=NULL;
		}
		cad_command_field_chooser =
			new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
				(cad_command_field_chooser_panel, cad_tool->command_field, computed_field_manager,
				Computed_field_has_string_value_type, (void *)NULL, cad_tool->user_interface);
		Callback_base< Computed_field* > *cad_command_field_callback = 
			new Callback_member_callback< Computed_field*, wxCadTool, int (wxCadTool::*)(Computed_field *) >
				(this, &wxCadTool::cad_command_field_callback);
		cad_command_field_chooser->set_callback(cad_command_field_callback);
		if (cad_tool != NULL)
		{
			command_field = Cad_tool_get_command_field(cad_tool);
			cad_command_field_chooser->set_object(command_field);
			if (command_field == NULL)
			{
				cad_command_field_checkbox->SetValue(0);
				cad_command_field_chooser_panel->Disable();
			}
			else
			{
				cad_command_field_checkbox->SetValue(1);
				cad_command_field_chooser_panel->Enable();
			}
		}
		button_surface = XRCCTRL(*this, "ButtonSurface", wxCheckBox);
		button_line = XRCCTRL(*this, "ButtonLine", wxCheckBox);
		button_surface->SetValue((cad_tool->select_surfaces_enabled==1));
		button_line->SetValue((cad_tool->select_lines_enabled==1));
	}

	wxCadTool()
	{
	}

	~wxCadTool()
	{
		//		 delete element_command_field_chooser;
	}

	int cad_command_field_callback(Computed_field *command_field)
	{
		Cad_tool_set_command_field(cad_tool, command_field);
		return 1;
	}

	void OnButtonSurfacePressed(wxCommandEvent& event)
	{
		USE_PARAMETER(event);
		button_surface = XRCCTRL(*this, "ButtonSurface", wxCheckBox);
		Cad_tool_set_select_surfaces_enabled(cad_tool,
			button_surface->IsChecked());
	}

	void OnButtonLinePressed(wxCommandEvent& event)
	{
		USE_PARAMETER(event);
		button_line = XRCCTRL(*this, "ButtonLine", wxCheckBox);
		Cad_tool_set_select_lines_enabled(cad_tool,
			button_line->IsChecked());
	}


	void OnButtonDestroyPressed(wxCommandEvent& event)
	{
		USE_PARAMETER(event);

		/* Implement new code here */
	}

	void CadToolInterfaceRenew(Cad_tool *destination_cad_tool)
	{
		button_surface = XRCCTRL(*this, "ButtonSurface", wxCheckBox);
		button_line = XRCCTRL(*this, "ButtonLine", wxCheckBox);
		button_surface->SetValue((destination_cad_tool->select_surfaces_enabled==1));
		button_line->SetValue((destination_cad_tool->select_lines_enabled==1));
	}

	void CadCommandFieldChecked(wxCommandEvent &event)
	{
		USE_PARAMETER(event);
		struct Computed_field *command_field;
		cad_command_field_checkbox = XRCCTRL(*this, "CadCommandFieldCheckBox",wxCheckBox);
		cad_command_field_chooser_panel = XRCCTRL(*this, "CadCommandFieldChooserPanel", wxPanel);
		if (cad_command_field_checkbox->IsChecked())
		{
			if (cad_tool)
			{
				if (Cad_tool_get_command_field(cad_tool))
				{
					Cad_tool_set_command_field(cad_tool, (struct Computed_field *)NULL);
					cad_command_field_chooser_panel->Enable();
				}
				else
				{
					/* get label field from widget */
					if (cad_command_field_chooser->get_number_of_object() > 0)
					{
						command_field = cad_command_field_chooser->get_object();
						if (command_field)
						{
							Cad_tool_set_command_field(cad_tool, command_field);
						}
					}
					else
					{
						cad_command_field_checkbox->SetValue(0);
						cad_command_field_chooser_panel->Disable();
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
			Cad_tool_set_command_field(cad_tool, (struct Computed_field *)NULL);
			cad_command_field_checkbox->SetValue(0);
			cad_command_field_chooser_panel->Disable();
		}
	}

	DECLARE_DYNAMIC_CLASS(wxCadTool);
	DECLARE_EVENT_TABLE();
};

IMPLEMENT_DYNAMIC_CLASS(wxCadTool, wxPanel)

BEGIN_EVENT_TABLE(wxCadTool, wxPanel)
	EVT_CHECKBOX(XRCID("ButtonSurface"),wxCadTool::OnButtonSurfacePressed)
	EVT_CHECKBOX(XRCID("ButtonLine"),wxCadTool::OnButtonLinePressed)
	EVT_CHECKBOX(XRCID("CadCommandFieldCheckBox"),wxCadTool::CadCommandFieldChecked)
	EVT_BUTTON(XRCID("ButtonDestroy"),wxCadTool::OnButtonDestroyPressed)
END_EVENT_TABLE()


#endif /* defined (WX_USER_INTERFACE) */

/*
Global functions
----------------
*/
static int Cad_tool_copy_function(
	void *destination_tool_void, void *source_tool_void,
	struct MANAGER(Interactive_tool) *destination_tool_manager) 
{
	int return_code;
	struct Cad_tool *destination_cad_tool, *source_cad_tool;
	ENTER(Cad_tool_copy_function);
	if ((destination_tool_void || destination_tool_manager) &&
		(source_cad_tool=(struct Cad_tool *)source_tool_void))
	{
		if (destination_tool_void)
		{
			destination_cad_tool = (struct Cad_tool *)destination_tool_void;
		}
		else
		{
			destination_cad_tool = CREATE(Cad_tool)
				(destination_tool_manager,
				source_cad_tool->region,
				source_cad_tool->element_point_ranges_selection,
				source_cad_tool->rubber_band_material,
				source_cad_tool->user_interface,
				source_cad_tool->time_keeper);
			Cad_tool_set_execute_command(destination_cad_tool,
				source_cad_tool->execute_command);
		}
		if (destination_cad_tool)
		{
			destination_cad_tool->select_surfaces_enabled = source_cad_tool->select_surfaces_enabled;
			destination_cad_tool->select_lines_enabled = source_cad_tool->select_lines_enabled;
			destination_cad_tool->command_field = source_cad_tool->command_field;
#if defined (WX_USER_INTERFACE)
			if (destination_cad_tool->wx_cad_tool != (wxCadTool *) NULL)
			{
				destination_cad_tool->wx_cad_tool->CadToolInterfaceRenew(destination_cad_tool);
			}
#endif /*defined (WX_USER_INTERFACE)*/
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cad_tool_copy_function.  Could not create copy.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cad_tool_copy_function.  Invalid argument(s)");
		return_code=0;
	}
	return (return_code);
}

struct Cad_tool *CREATE(Cad_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct Cmiss_region *region,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Graphical_material *rubber_band_material,
	struct User_interface *user_interface,
	struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Creates an Cad_tool with Interactive_tool in <interactive_tool_manager>.
==============================================================================*/
{
	struct Cad_tool *cad_tool;
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(CREATE(Cad_tool));
	cad_tool=(struct Cad_tool *)NULL;
	if (interactive_tool_manager && region &&
			(NULL != (computed_field_manager=
				Cmiss_region_get_Computed_field_manager(region)))
		&&rubber_band_material&&user_interface)
	{
		if (ALLOCATE(cad_tool,struct Cad_tool,1))
		{
			cad_tool->execute_command=NULL;
			cad_tool->interactive_tool_manager=interactive_tool_manager;
			cad_tool->region = region;
			cad_tool->element_point_ranges_selection=
				element_point_ranges_selection;
			cad_tool->rubber_band_material=
				ACCESS(Graphical_material)(rubber_band_material);
			cad_tool->user_interface=user_interface;
			cad_tool->time_keeper = (struct Time_keeper *)NULL;
			cad_tool->rendition=(struct Cmiss_rendition *)NULL;
			if (time_keeper)
			{
				cad_tool->time_keeper = ACCESS(Time_keeper)(time_keeper);
			}
			/* user-settable flags */
			cad_tool->select_surfaces_enabled=1;
			cad_tool->select_lines_enabled=1;
			cad_tool->command_field = (struct Computed_field *)NULL;
			/* interactive_tool */
#if defined (OPENGL_API)
			cad_tool->interactive_tool=CREATE(Interactive_tool)(
				"cad_tool","Cad tool",
				Interactive_tool_element_type_string,
				Cad_tool_interactive_event_handler,
				Cad_tool_get_icon,
				Cad_tool_bring_up_interactive_tool_dialog,
				Cad_tool_reset,
 				Cad_tool_destroy_cad_tool,
				Cad_tool_copy_function,
				(void *)cad_tool);
#else /* defined (OPENGL_API) */
			cad_tool->interactive_tool=CREATE(Interactive_tool)(
				"cad_tool","Cad tool",
				Interactive_tool_element_type_string,
				(Interactive_event_handler*)NULL,
				Cad_tool_get_icon,
				(Interactive_tool_bring_up_dialog_function*)NULL,
				(Interactive_tool_reset_function*)NULL,				
				(Interactive_tool_destroy_tool_data_function *)NULL,
				Cad_tool_copy_function,
				(void *)cad_tool);
#endif /* defined (OPENGL_API) */
			ADD_OBJECT_TO_MANAGER(Interactive_tool)(
				cad_tool->interactive_tool,
				cad_tool->interactive_tool_manager);
			cad_tool->last_picked_element=(struct FE_element *)NULL;
			cad_tool->last_interaction_volume=(struct Interaction_volume *)NULL;
			cad_tool->rubber_band=(struct GT_object *)NULL;

#if defined (WX_USER_INTERFACE)
			cad_tool->wx_cad_tool=(wxCadTool *)NULL;
#endif /* defined (WX_USER_INTERFACE) */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Cad_tool).  Not enough memory");
			DEALLOCATE(cad_tool);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Cad_tool).  Invalid argument(s)");
	}
	LEAVE;

	return (cad_tool);
} /* CREATE(Cad_tool) */

int DESTROY(Cad_tool)(struct Cad_tool **cad_tool_address)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Frees and deaccesses objects in the <cad_tool> and deallocates the
structure itself.
==============================================================================*/
{
	struct Cad_tool *cad_tool;
	int return_code;

	ENTER(DESTROY(Cad_tool));
	if (cad_tool_address&&(cad_tool= *cad_tool_address))
	{
		REACCESS(FE_element)(&(cad_tool->last_picked_element),
			(struct FE_element *)NULL);
		REACCESS(Interaction_volume)(&(cad_tool->last_interaction_volume),
			(struct Interaction_volume *)NULL);
		REACCESS(GT_object)(&(cad_tool->rubber_band),(struct GT_object *)NULL);
		DEACCESS(Graphical_material)(&(cad_tool->rubber_band_material));
		if (cad_tool->time_keeper)
		{
			DEACCESS(Time_keeper)(&(cad_tool->time_keeper));
		}
#if defined (WX_USER_INTERFACE)
		if (cad_tool->wx_cad_tool)
			 cad_tool->wx_cad_tool->Destroy();
#endif /*(WX_USER_INTERFACE)*/
		DEALLOCATE(*cad_tool_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cad_tool).  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* DESTROY(Cad_tool) */

int Cad_tool_pop_up_dialog(struct Cad_tool *cad_tool, struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 20 June 2001

DESCRIPTION :
Pops up a dialog for editing settings of the Cad_tool.
==============================================================================*/
{
	int return_code;

	ENTER(Cad_tool_pop_up_dialog);
	if (cad_tool)
	{
#if defined (WX_USER_INTERFACE)
		wxPanel *pane;
		if (!cad_tool->wx_cad_tool)
		{
			 cad_tool->wx_cad_tool = new wxCadTool(cad_tool,
				Graphics_window_get_interactive_tool_panel(graphics_window));
			 pane = XRCCTRL(*cad_tool->wx_cad_tool, "CmguiCadTool", wxPanel);
			 cad_tool->tool_position = pane->GetPosition();
			 cad_tool->wx_cad_tool->Show();
		}
		else
		{
			 pane = XRCCTRL(*cad_tool->wx_cad_tool, "CmguiCadTool", wxPanel);
			 pane->SetPosition(cad_tool->tool_position);
			 cad_tool->wx_cad_tool->Show();
		}
#else /* defined (WX_USER_INTERFACE) */
		USE_PARAMETER(graphics_window);
		display_message(ERROR_MESSAGE, "Cad_tool_pop_up_dialog.  "
			"No dialog implemented for this User Interface");
#endif /*  defined (WX_USER_INTERFACE) */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cad_tool_pop_up_dialog.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cad_tool_pop_up_dialog */

int Cad_tool_pop_down_dialog(struct Cad_tool *cad_tool)
/*******************************************************************************
LAST MODIFIED : 20 June 2001

DESCRIPTION :
Hides the dialog for editing settings of the Cad_tool.
==============================================================================*/
{
	int return_code;

	ENTER(Cad_tool_pop_down_dialog);
	if (cad_tool)
	{
		display_message(ERROR_MESSAGE, "Cad_tool_pop_down_dialog.  "
			"No dialog implemented for this User Interface");
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cad_tool_pop_down_dialog.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cad_tool_pop_down_dialog */

int Cad_tool_get_select_surfaces_enabled(struct Cad_tool *cad_tool)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns flag controlling whether face & 2-D top-level elements can be selected.
==============================================================================*/
{
	int select_surfaces_enabled;

	ENTER(Cad_tool_get_select_surfaces_enabled);
	if (cad_tool)
	{
		select_surfaces_enabled=cad_tool->select_surfaces_enabled;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cad_tool_get_select_surfaces_enabled.  Invalid argument(s)");
		select_surfaces_enabled=0;
	}
	LEAVE;

	return (select_surfaces_enabled);
} /* Cad_tool_get_select_surfaces_enabled */

int Cad_tool_set_select_surfaces_enabled(struct Cad_tool *cad_tool,
	int select_surfaces_enabled)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns flag controlling whether face & 2-D top-level elements can be selected.
==============================================================================*/
{
#if defined (MOTIF_USER_INTERFACE)
	int button_state;
#endif /* defined (MOTIF_USER_INTERFACE) */
	int return_code;

	ENTER(Cad_tool_set_select_surfaces_enabled);
	if (cad_tool)
	{
		return_code=1;
		if (select_surfaces_enabled)
		{
			/* make sure value of flag is exactly 1 */
			select_surfaces_enabled=1;
		}
		if (select_surfaces_enabled != cad_tool->select_surfaces_enabled)
		{
			cad_tool->select_surfaces_enabled=select_surfaces_enabled;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cad_tool_set_select_surfaces_enabled.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cad_tool_set_select_surfaces_enabled */

int Cad_tool_get_select_lines_enabled(struct Cad_tool *cad_tool)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns flag controlling whether line & 1-D top-level elements can be selected.
==============================================================================*/
{
	int select_lines_enabled;

	ENTER(Cad_tool_get_select_lines_enabled);
	if (cad_tool)
	{
		select_lines_enabled=cad_tool->select_lines_enabled;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cad_tool_get_select_lines_enabled.  Invalid argument(s)");
		select_lines_enabled=0;
	}
	LEAVE;

	return (select_lines_enabled);
} /* Cad_tool_get_select_lines_enabled */

int Cad_tool_set_select_lines_enabled(struct Cad_tool *cad_tool,
	int select_lines_enabled)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns flag controlling whether line & 1-D top-level elements can be selected.
==============================================================================*/
{
#if defined (MOTIF_USER_INTERFACE)
	int button_state;
#endif /* defined (MOTIF_USER_INTERFACE) */
	int return_code;

	ENTER(Cad_tool_set_select_lines_enabled);
	if (cad_tool)
	{
		return_code=1;
		if (select_lines_enabled)
		{
			/* make sure value of flag is exactly 1 */
			select_lines_enabled=1;
		}
		if (select_lines_enabled != cad_tool->select_lines_enabled)
		{
			cad_tool->select_lines_enabled=select_lines_enabled;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cad_tool_set_select_lines_enabled.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cad_tool_set_select_lines_enabled */

struct Computed_field *Cad_tool_get_command_field(
	struct Cad_tool *cad_tool)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Returns the command_field to be looked up in a web browser when the element is
clicked on in the <cad_tool>.
==============================================================================*/
{
	struct Computed_field *command_field;

	ENTER(Cad_tool_get_command_field);
	if (cad_tool)
	{
		command_field=cad_tool->command_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cad_tool_get_command_field.  Invalid argument(s)");
		command_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (command_field);
} /* Cad_tool_get_command_field */

int Cad_tool_set_command_field(struct Cad_tool *cad_tool,
	struct Computed_field *command_field)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Sets the command_field to be looked up in a web browser when the element is clicked
on in the <cad_tool>.
==============================================================================*/
{
#if defined (MOTIF_USER_INTERFACE)
	int field_set;
#endif /* defined (MOTIF_USER_INTERFACE) */
	int return_code;

	ENTER(Cad_tool_set_command_field);
	if (cad_tool && ((!command_field) ||
		Computed_field_has_string_value_type(command_field, (void *)NULL)))
	{
		return_code = 1;
		if (command_field != cad_tool->command_field)
		{
			cad_tool->command_field = command_field;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cad_tool_set_command_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cad_tool_set_command_field */

struct Interactive_tool *Cad_tool_get_interactive_tool(
	struct Cad_tool *cad_tool)
/*******************************************************************************
LAST MODIFIED : 29 March 2007

DESCRIPTION :
Returns the generic interactive_tool the represents the <cad_tool>.
==============================================================================*/
{
	struct Interactive_tool *interactive_tool;

	ENTER(Cad_tool_get_interactive_tool);
	if (cad_tool)
	{
		interactive_tool=cad_tool->interactive_tool;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cad_tool_get_interactive_tool.  Invalid argument(s)");
		interactive_tool=(struct Interactive_tool *)NULL;
	}
	LEAVE;

	return (interactive_tool);
} /* Cad_tool_get_interactive_tool */

int Cad_tool_set_execute_command(struct Cad_tool *cad_tool, 
	struct Execute_command *execute_command)
{
	int return_code = 0;
	if (cad_tool)
	{
		cad_tool->execute_command = execute_command;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cad_tool_set_execute_command.  Invalid argument(s)");
	}
	
	return return_code;
}
