/*******************************************************************************
FILE : node_viewer_wx.cpp

LAST MODIFIED : 23 April 2007

DESCRIPTION :
Dialog for selecting nodes and viewing and/or editing field values. Works with
selection to display the last selected node, or set it if entered in
this dialog.
 */
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
#include "configure/zinc_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */

extern "C" {
#include "api/cmiss_field_subobject_group.h"
#include "api/cmiss_status.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_group.h"
#include "finite_element/finite_element.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "graphics/rendition.h"
#include "node/node_viewer_wx.h"
#include "user_interface/message.h"
#include "time/time.h"
}
#include "computed_field/computed_field_subobject_group_private.hpp"
#if defined (WX_USER_INTERFACE)
#include <wx/collpane.h>
#include "wx/wx.h"
#include "wx/xrc/xmlres.h"
#include "node/node_viewer_wx.xrch"
#include "region/cmiss_region_chooser_wx.hpp"
#include "choose/text_FE_choose_class.hpp"
#include "icon/cmiss_icon.xpm"
#endif /*defined (WX_USER_INTERFACE)*/

/*
Module variables
----------------
*/

struct Nodal_value_information
{
	enum FE_nodal_value_type type;
	int component_number,version;
	struct Computed_field *field;
}; /* struct Nodal_value_information */

#if defined (WX_USER_INTERFACE)
class wxNodeViewer;
#endif /* defined (WX_USER_INTERFACE) */

struct Node_viewer
/*******************************************************************************
LAST MODIFIED : 6 June 2001

DESCRIPTION :
Contains all the information carried by the node_viewer widget.
==============================================================================*/
{
	 enum FE_nodal_value_type *nodal_value_types;
	 int number_of_nodal_value_types;
	/* remember number of components to detect redefinition of computed fields */
	int number_of_components;
	 struct Node_viewer **node_viewer_address;
	 struct FE_node *node_copy, *initial_node, *current_node, *template_node;
	 struct Cmiss_region *region;
	 int use_data;
	 struct FE_region *fe_region;
	 struct MANAGER(Computed_field) *computed_field_manager;
	 void *computed_field_manager_callback_id;
	 struct Time_object *time_object;
	 int time_object_callback;
	struct Computed_field *current_field;
#if defined (WX_USER_INTERFACE)
	 wxNodeViewer *wx_node_viewer;
	 wxScrolledWindow *collpane;
	 wxWindow *win;
	 wxFrame *frame;
	 wxGridSizer *grid_field;
	 int init_width, init_height, frame_width, frame_height;
#endif /* (WX_USER_INTERFACE) */
}; /* node_viewer_struct */

/*
Prototype
------------
*/
static int Node_viewer_set_Cmiss_region(struct Node_viewer *node_viewer,
	struct Cmiss_region *region);

static int Node_viewer_set_viewer_node(struct Node_viewer *node_viewer);
/*******************************************************************************
LAST MODIFIED : 24 April 2007

DESCRIPTION :
Gets the current node from the select widget, makes a copy of it if not NULL,
and passes it to the node_viewer.
==============================================================================*/

static void Node_viewer_FE_region_change(struct FE_region *fe_region,
	 struct FE_region_changes *changes, void *node_viewer_void);
/*******************************************************************************
LAST MODIFIED : 24 April 2007

DESCRIPTION :
Note that we do not have to handle add, delete and identifier change messages
here as the select widget does this for us. Only changes to the content of the
object cause updates.
==============================================================================*/

enum FE_nodal_value_type *get_FE_node_field_value_types(
	struct FE_node *node,struct FE_field *field,int *number_of_nodal_value_types,
	int *number_of_unknown_nodal_value_types);
/*******************************************************************************
LAST MODIFIED : 27 April 2007

DESCRIPTION :
Returns an array containing the list of named nodal value/derivative types
defined for any - but not necessarily all - components of the <field> at <node>.
Nodal value types are returned in ascending order.
On return <number_of_unknown_nodal_value_types> contains the number of nodal
values whose value_type was not recognized.
==============================================================================*/

int node_viewer_set_node_field(void *node_viewer_void,
	 struct FE_node *node,struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 27 April 2007

DESCRIPTION :
Sets the node/field being edited in the <node_field_viewer_widget>. Note that
the viewer works on the node itself, not a local copy. Hence, only pass
unmanaged nodes to this widget.
==============================================================================*/

int Node_viewer_update_collpane(struct Node_viewer *node_viewer);
char *node_viewer_update_value(struct Node_viewer *node_viewer, Computed_field *field, int component_number, FE_nodal_value_type type, int version);

int node_viewer_widget_set_node(Node_viewer *node_viewer,
	 struct FE_region *fe_region, struct FE_node *node);

/*
Module constants
----------------
*/

/* following must be big enough to hold an element_xi value */
#define VALUE_STRING_SIZE 100

/*
Module functions
----------------
*/

static int Node_viewer_apply_changes(struct Node_viewer *node_viewer)
/*******************************************************************************
LAST MODIFIED : 2 May 2007

DESCRIPTION :
Makes the node change global.
==============================================================================*/
{
	int return_code;

	if (node_viewer)
	{
		if (node_viewer->node_copy)
		{
			if (FE_region_merge_FE_node(node_viewer->fe_region,
				node_viewer->node_copy))
			{
				return_code = 1;
			}
			else
			{
				return_code = 0;
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"Node_viewer_apply_changes.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Node_viewer_apply_changes */

class wxNodeViewer : public wxFrame
{	
	 Node_viewer *node_viewer;
	 wxPanel *node_text_panel;
	 wxScrolledWindow *variable_viewer_panel;
	 DEFINE_FE_region_FE_object_method_class(node);
 	 FE_object_text_chooser< FE_node, FE_region_FE_object_method_class(node) > *node_text_chooser;
	 wxFrame *frame;
	 wxRegionChooser *region_chooser;
public:
	 
	 wxNodeViewer(Node_viewer *node_viewer): 
			node_viewer(node_viewer)
	 {
			wxXmlInit_node_viewer_wx();
			node_viewer->wx_node_viewer = this;
			wxXmlResource::Get()->LoadFrame(this,
				(wxWindow *)NULL, _T("CmguiNodeViewer"));
			this->SetIcon(cmiss_icon_xpm);
			wxPanel *node_text_panel =
				XRCCTRL(*this, "NodeTextPanel",wxPanel);
			node_text_chooser =
				new FE_object_text_chooser< FE_node, FE_region_FE_object_method_class(node) >(node_text_panel, node_viewer->initial_node, node_viewer->fe_region, (LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL, NULL);
			Callback_base< FE_node* > *node_text_callback = 
				(new Callback_member_callback< FE_node*, 
				wxNodeViewer, int (wxNodeViewer::*)(FE_node *) >
				(this, &wxNodeViewer::node_text_callback));
			node_text_chooser->set_callback(node_text_callback);
		wxPanel *region_chooser_panel =
			 XRCCTRL(*this, "RegionChooserPanel", wxPanel);
		char *initial_path;
		initial_path = Cmiss_region_get_root_region_path();
		region_chooser = new wxRegionChooser(region_chooser_panel,
			node_viewer->region, initial_path);
		DEALLOCATE(initial_path);
		Callback_base<Cmiss_region* > *Node_viewer_wx_region_callback =
			new Callback_member_callback< Cmiss_region*,
			wxNodeViewer, int (wxNodeViewer::*)(Cmiss_region *) >
			(this, &wxNodeViewer::Node_viewer_wx_region_callback);
		region_chooser->set_callback(Node_viewer_wx_region_callback);
			Show();
			frame = XRCCTRL(*this, "CmguiNodeViewer",wxFrame);
			frame->GetSize(&(node_viewer->init_width), &(node_viewer->init_height));
			frame->SetSize(node_viewer->frame_width,node_viewer->frame_height+100);
			frame->GetSize(&(node_viewer->frame_width), &(node_viewer->frame_height));
			frame->Layout();
	 };
	 
	 wxNodeViewer()
	 {
	 };

  ~wxNodeViewer()
	 {
			delete node_text_chooser;
			delete region_chooser;
	 }

	int Node_viewer_wx_region_callback(Cmiss_region *region)
/*******************************************************************************
LAST MODIFIED : 9 February 2007

DESCRIPTION :
Callback from wxChooser<Computed_field> when choice is made.
==============================================================================*/
	{
		USE_PARAMETER(region);
		if (region)
		{
			FE_region *fe_region = Cmiss_region_get_FE_region(region);
			node_text_chooser->set_region(fe_region);
		}
		if (Node_viewer_set_Cmiss_region(node_viewer, region) &&
			node_viewer->wx_node_viewer && node_viewer->collpane)
		{
			Node_viewer_update_collpane(node_viewer);
		}
		return 1;
	}


	 int node_text_callback(FE_node *node)
/*******************************************************************************
LAST MODIFIED : 20 April 2007

DESCRIPTION :
Callback from wxTextChooser when text is entered.
==============================================================================*/
	 {
			if (node_viewer)
			{
				 if (node)
				 {
						node_viewer->current_node = node;
						Node_viewer_update_collpane(node_viewer);
				 }
				 else
				 {
						Node_viewer_set_viewer_node(node_viewer);
						if (node_viewer->wx_node_viewer && node_viewer->collpane)
						{
							Node_viewer_update_collpane(node_viewer);
						}
				 }
			}
			else
			{
				 display_message(WARNING_MESSAGE,
						"Node_text_callback.  Invalid argument(s)");
			}
			LEAVE;
			return 1;
	 }

	 void RenewNodeViewer(wxCollapsiblePaneEvent& event)
	 {
	USE_PARAMETER(event);
			wxScrolledWindow *VariableViewer = XRCCTRL(*this,"VariableViewerPanel",wxScrolledWindow);
			VariableViewer->Layout();	
			frame = XRCCTRL(*this, "CmguiNodeViewer", wxFrame);
			frame->SetMinSize(wxSize(50,100));
			frame->SetMaxSize(wxSize(2000,2000));
	 }

	void OnOKpressed(wxCommandEvent &event)
	 {
	USE_PARAMETER(event);
			if (Node_viewer_apply_changes(node_viewer))
			{
				 DESTROY(Node_viewer)(node_viewer->node_viewer_address);
			}
	 }

	 void OnApplypressed(wxCommandEvent &event)
	 {
	USE_PARAMETER(event);
			Node_viewer_apply_changes(node_viewer);
	 }

	 void OnRevertpressed(wxCommandEvent &event)
	 {
		 USE_PARAMETER(event);
		 if (node_viewer->current_node)
		{
			Node_viewer_set_viewer_node(node_viewer);
			if (node_viewer->wx_node_viewer && node_viewer->collpane)
			{
				Node_viewer_update_collpane(node_viewer);
			}
		}
	}

	 void OnCancelpressed(wxCommandEvent &event)
	 {
	USE_PARAMETER(event);
			DESTROY(Node_viewer)(node_viewer->node_viewer_address);
	 }

	 void FrameSetSize(wxSizeEvent &event)
/*******************************************************************************
LAST MODIFIED : 19 June 2007

DESCRIPTION :
Callback of size event to prevent minising the size of the windows
after a collapsible pane is opened/closed.
==============================================================================*/
	 {
			int temp_width, temp_height;
			
	USE_PARAMETER(event);
			frame = XRCCTRL(*this, "CmguiNodeViewer",wxFrame);
			frame->Freeze();
			frame->GetSize(&temp_width, &temp_height);
			if (temp_height !=node_viewer->frame_height || temp_width !=node_viewer->frame_width)
			{
				 if (temp_width>node_viewer->init_width || temp_height>node_viewer->init_height)
				 {
						node_viewer->frame_width = temp_width;
						node_viewer->frame_height = temp_height;
				 }
				 else
				 {
						frame->SetSize(node_viewer->frame_width,node_viewer->frame_height);
				 }
			}
			frame->Thaw();
			frame->Layout();		
	 }

void Terminate(wxCloseEvent& event)
{
	USE_PARAMETER(event);
	 DESTROY(Node_viewer)(node_viewer->node_viewer_address);
} 

	 void NodeViewerTextEntered(wxTextCtrl *textctrl , Node_viewer *node_viewer,
			Computed_field *field, int component_number, FE_nodal_value_type type, int version)
	 {
			char *value_string, *temp_string;
			FE_value time;
			struct FE_field *fe_field;
			struct FE_node *node;
			if (field&&node_viewer&&
				 (node=node_viewer->current_node))
			{
				 time = Time_object_get_current_time(node_viewer->time_object);			 
				 node_viewer->current_field = field;
				 if (node_viewer->current_field != NULL) 
				 {
						value_string=const_cast<char *>((textctrl->GetValue()).c_str());
						if (value_string != NULL)
						{
							 if (Computed_field_is_type_finite_element(field))
							 {
									Computed_field_get_type_finite_element(field,&fe_field);
									switch (get_FE_field_value_type(fe_field))
									{
										 case ELEMENT_XI_VALUE:
										 {
												display_message(ERROR_MESSAGE,
													 "Cannot set element:xi values yet");
										 } break;
										 case FE_VALUE_VALUE:
										 {
												FE_value fe_value_value;

												sscanf(value_string,FE_VALUE_INPUT_STRING,&fe_value_value);
												set_FE_nodal_FE_value_value(node,
													 fe_field, component_number,
													 version,
													 type, time, fe_value_value);
										 } break;
										 case INT_VALUE:
										 {
												int int_value;
												
												sscanf(value_string,"%d",&int_value);
												set_FE_nodal_int_value(node,
													 fe_field, component_number,
													 version,
													 type,time, int_value);
										 } break;
										 case STRING_VALUE:
										 {
												display_message(WARNING_MESSAGE,"Cannot set string values yet");
										 } break; 
										 default:
										 {
												display_message(ERROR_MESSAGE,
													 "node_field_viewer_widget_update_values.  "
													 "Unsupported value_type for FE_field");
										 } break;
									}
							 }
							 else
							 {
									if (Computed_field_is_type_cmiss_number(field))
									{
										 display_message(WARNING_MESSAGE,
												"CMISS number cannot be changed here");
									}
									else if (!Computed_field_has_numerical_components(field, NULL))
									{
										 display_message(WARNING_MESSAGE,
												"Unable to set string values in a computed field.");
									}
									else
									{
										 FE_value *values;
										 int number_of_components;
										 
										 number_of_components=Computed_field_get_number_of_components(field);
										 if (ALLOCATE(values,FE_value,number_of_components))
										 {
												Cmiss_field_module_id field_module = Cmiss_field_get_field_module(field);
												Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
												Cmiss_field_cache_set_time(field_cache, time);
												Cmiss_field_cache_set_node(field_cache, node);
												if (Cmiss_field_evaluate_real(field, field_cache, number_of_components, values))
												{
													 sscanf(value_string,FE_VALUE_INPUT_STRING,
															&values[component_number]);
													 Cmiss_field_assign_real(field, field_cache, number_of_components, values);
												}
												Cmiss_field_cache_destroy(&field_cache);
												Cmiss_field_module_destroy(&field_module);
												DEALLOCATE(values);
										 }
									}
							 }
						}
						/* refresh value shown in the text field widgets */
						temp_string = node_viewer_update_value(node_viewer, field, component_number, type, version);
						if (temp_string != NULL)
						{
							 textctrl->SetValue(temp_string);
						}
				 }
			}
			else
			{
				 display_message(ERROR_MESSAGE,
						"node_field_viewer_value_CB.  Invalid argument(s)");
			}							
	 }
	 
	 struct FE_node  *get_selected_node()
	 {
			return node_text_chooser->get_object();
	 }

	 int set_selected_node(FE_node *new_node)
	 {
		  return node_text_chooser->set_object(new_node);
	 }

	 

  DECLARE_DYNAMIC_CLASS(wxNodeViewer);
  DECLARE_EVENT_TABLE();
};

IMPLEMENT_DYNAMIC_CLASS(wxNodeViewer, wxFrame)
BEGIN_EVENT_TABLE(wxNodeViewer, wxFrame)
	 EVT_COLLAPSIBLEPANE_CHANGED(wxID_ANY,wxNodeViewer::RenewNodeViewer)
	 EVT_BUTTON(XRCID("OKButton"),wxNodeViewer::OnOKpressed)
	 EVT_BUTTON(XRCID("ApplyButton"),wxNodeViewer::OnApplypressed)
	 EVT_BUTTON(XRCID("RevertButton"),wxNodeViewer::OnRevertpressed)
	 EVT_BUTTON(XRCID("CancelButton"),wxNodeViewer::OnCancelpressed)
#if !defined (__WXGTK__)
	 EVT_SIZE(wxNodeViewer::FrameSetSize)
#endif /*!defined (__WXGTK__)*/
	 EVT_CLOSE(wxNodeViewer::Terminate)
END_EVENT_TABLE()

class wxNodeViewerTextCtrl : public wxTextCtrl
{
	 Node_viewer *node_viewer;
	 struct Computed_field *field;
	 int component_number;
	 enum FE_nodal_value_type type;
	 int version;

public:

  wxNodeViewerTextCtrl(Node_viewer *node_viewer, 
		 Computed_field *field,
		 int component_number,
		 enum FE_nodal_value_type type,
		 int version) :
		 node_viewer(node_viewer), field(field), component_number(component_number),
		 type(type), version(version)
  {
  }

  ~wxNodeViewerTextCtrl()
  {
  }

  void OnNodeViewerTextCtrlEntered(wxCommandEvent& event)
  {
	USE_PARAMETER(event);
		 node_viewer->wx_node_viewer->NodeViewerTextEntered
				(this, node_viewer, field, component_number, type, version);
   }

};

static int node_viewer_add_collpane(struct Computed_field *current_field, void *node_viewer_void)
{
	 char *field_name;
	 Node_viewer *node_viewer = (struct Node_viewer *)node_viewer_void;
	 field_name = (char *)NULL;
	 GET_NAME(Computed_field)(current_field, &field_name);
	 node_viewer->current_field = current_field;
	 int condition;
	 if (node_viewer->current_node)
	 {
			condition = Computed_field_is_defined_at_node_conditional(
				 current_field, (void *)node_viewer->current_node);
	 }
	 else
	 {
			condition = 1;
	 }
	 if (condition)
	 {
			wxScrolledWindow *panel = node_viewer->collpane;

			// identifier is the name of the panel in the collapsible pane
			// field_name is the name of the CollapsiblePane
			node_viewer->win = panel->FindWindowByName(field_name);
			if (node_viewer->win != NULL)
			{
				 node_viewer->win->DestroyChildren();
			}
			else
			{
				 wxCollapsiblePane *collapsiblepane = new wxCollapsiblePane(panel, /*id*/-1, field_name);
				 wxSizer *sizer = panel->GetSizer();
				 sizer->Add(collapsiblepane, 0,wxALL, 5);
				 node_viewer->win = collapsiblepane->GetPane();
				 node_viewer->win->SetName(field_name);
			}

 			if (node_viewer_set_node_field((void *)node_viewer,
						node_viewer->current_node, node_viewer->current_field))
			{
				 if (node_viewer->grid_field != NULL)
				 {
						node_viewer->win->SetSizer(node_viewer->grid_field);
						node_viewer->grid_field->SetSizeHints(node_viewer->win);
						node_viewer->grid_field->Layout();
						node_viewer->win->Layout();
				 }
			}
			panel->FitInside();
			panel->SetScrollbars(20, 20, 50, 50);
			panel->Layout();
			wxFrame *frame;
			frame = XRCCTRL(*node_viewer->wx_node_viewer, "CmguiNodeViewer", wxFrame);
			frame->Layout();
			frame->SetMinSize(wxSize(50,100));
	 }
	 if (field_name)
	 {
			DEALLOCATE(field_name);
	 }
	 return 1;
}

int Node_viewer_remove_unused_collpane(struct Node_viewer *node_viewer)
{
	int return_code = 0;
	if (node_viewer)
	{
		wxWindowList list = node_viewer->collpane->GetChildren();
		wxWindowList::iterator iter;
    for (iter = list.begin(); iter != list.end(); ++iter)
    {
        wxWindow *current = *iter;
        wxWindow *child = ((wxCollapsiblePane *)current)->GetPane();
        const char *field_name = child->GetName().GetData();
        struct Computed_field *field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)(
        	field_name, node_viewer->computed_field_manager);
        if (!field)
        {
        	current->Destroy();
        }
    }
	}
	return return_code;
}

int Node_viewer_update_collpane(struct Node_viewer *node_viewer)
{
	int return_code = 0;
	if (node_viewer)
	{
		return_code = FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
			node_viewer_add_collpane, (void *) node_viewer,
			node_viewer->computed_field_manager);
		Node_viewer_remove_unused_collpane(node_viewer);
	}
	return return_code;
}

static void Node_viewer_Computed_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message,void *node_viewer_void)
{
	struct Node_viewer *node_viewer;
	struct LIST(Computed_field) *changed_field_list=NULL;

	ENTER(Node_viewer_Computed_field_change);
	node_viewer =	(struct Node_viewer *)node_viewer_void;
	if (message && node_viewer)
	{
		Cmiss_rendition *rendition = Cmiss_region_get_rendition_internal(node_viewer->region);
		if (rendition)
		{
			Cmiss_field_group_id selection_group = Cmiss_rendition_get_selection_group(rendition);
			changed_field_list =
				MANAGER_MESSAGE_GET_CHANGE_LIST(Computed_field)(message,
					MANAGER_CHANGE_RESULT(Computed_field));
			if (selection_group && changed_field_list && Computed_field_or_ancestor_satisfies_condition(
				Cmiss_field_group_base_cast(selection_group), Computed_field_is_in_list, (void *)changed_field_list))
			{
				Cmiss_field_module_id field_module = Cmiss_region_get_field_module(node_viewer->region);
				Cmiss_nodeset_id master_nodeset = Cmiss_field_module_find_nodeset_by_name(
					field_module, node_viewer->use_data ? "cmiss_data" : "cmiss_nodes");
				Cmiss_field_module_destroy(&field_module);
				Cmiss_field_node_group_id node_group = Cmiss_field_group_get_node_group(selection_group, master_nodeset);
				Cmiss_nodeset_destroy(&master_nodeset);
				Cmiss_nodeset_group_id nodeset_group = Cmiss_field_node_group_get_nodeset(node_group);
				Cmiss_field_node_group_destroy(&node_group);
				/* make sure there is only one node selected in group */
				if (1 == Cmiss_nodeset_get_size(Cmiss_nodeset_group_base_cast(nodeset_group)))
				{
					Cmiss_node_iterator_id iterator = Cmiss_nodeset_create_node_iterator(
						Cmiss_nodeset_group_base_cast(nodeset_group));
					Cmiss_node_id node = Cmiss_node_iterator_next_non_access(iterator);
					Cmiss_node_iterator_destroy(&iterator);
					if (node != node_viewer->current_node)
					{
						if (node_viewer->wx_node_viewer)
						{
							node_viewer->wx_node_viewer->set_selected_node(node);
							node_viewer->current_node = node;
						}
						if (node_viewer->wx_node_viewer && node_viewer->collpane)
						{
							Node_viewer_update_collpane(node_viewer);
						}
					}
				}
				Cmiss_nodeset_group_destroy(&nodeset_group);
			}
			if (changed_field_list)
				DESTROY(LIST(Computed_field))(&changed_field_list);
			Cmiss_rendition_destroy(&rendition);
			if (selection_group)
			{
				Cmiss_field_group_destroy(&selection_group);
			}
		}
	}
}

struct Node_viewer *CREATE(Node_viewer)(
	struct Node_viewer **node_viewer_address,
	const char *dialog_title,
	struct FE_node *initial_node,
	struct Cmiss_region *region, int use_data,
	struct Time_object *time_object)
/*******************************************************************************
LAST MODIFIED : 30 April 2003

DESCRIPTION :
Creates a dialog for choosing nodes and displaying and editing their fields.
Nodes, starting with <initial_node> may be chosen from <this_node_manager>.
Pass <initial_data> and <data_manager> in these parameters for data viewer.
Since both nodes and data can depend on embedded fields, the
<actual_node_manager> and <actual_element_manager> also need to be passed.
==============================================================================*/
{
	struct Node_viewer *node_viewer;
	ENTER(CREATE(Node_viewer));
	node_viewer = (struct Node_viewer *)NULL;
	if (node_viewer_address && dialog_title && region)
	{
		 /* allocate memory */
		 if (ALLOCATE(node_viewer,struct Node_viewer,1))
		 {
				node_viewer->region = region;
				node_viewer->computed_field_manager =
					Cmiss_region_get_Computed_field_manager(node_viewer->region);
				node_viewer->nodal_value_types=(enum FE_nodal_value_type *)NULL;
				node_viewer->node_copy = (struct FE_node *)NULL;
				node_viewer->node_viewer_address = node_viewer_address;
				node_viewer->use_data = use_data;
				node_viewer->fe_region = Cmiss_region_get_FE_region(region);
				if (use_data)
				{
					node_viewer->fe_region = FE_region_get_data_FE_region(node_viewer->fe_region);
				}
				node_viewer->collpane = NULL;
				node_viewer->current_node=(struct FE_node *)NULL;

				node_viewer->time_object = ACCESS(Time_object)(time_object);
				node_viewer->time_object_callback = 0;
				node_viewer->current_field=(struct Computed_field *)NULL;
				node_viewer->number_of_components=-1;
				node_viewer->number_of_nodal_value_types=0;
				node_viewer->grid_field = NULL;
#if defined (WX_USER_INTERFACE)
				node_viewer->grid_field = NULL;
				node_viewer->wx_node_viewer = (wxNodeViewer *)NULL;
#endif /* defined (WX_USER_INTERFACE) */	
				if (!initial_node)
				{
					initial_node = FE_region_get_first_FE_node_that(
						node_viewer->fe_region,
						(LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL, (void *)NULL);
					if (initial_node != NULL)
					{
						/* select the node to be displayed in dialog; note this is ok
							here as we are not receiving selection callbacks yet */
						Node_viewer_set_viewer_node(node_viewer);
						if (node_viewer->wx_node_viewer && node_viewer->collpane)
						{
							Node_viewer_update_collpane(node_viewer);
						}
					}
				}
				node_viewer->computed_field_manager_callback_id =
					MANAGER_REGISTER(Computed_field)(Node_viewer_Computed_field_change,
						(void *)node_viewer,	node_viewer->computed_field_manager);
				FE_region_add_callback(node_viewer->fe_region,
					Node_viewer_FE_region_change, (void *)node_viewer);
				/* make the dialog shell */
#if defined (WX_USER_INTERFACE)
				node_viewer->initial_node = initial_node;
				node_viewer->frame_width = 0;
				node_viewer->frame_height = 0;
				node_viewer->init_width = 0;
				node_viewer->init_height=0;
				if (initial_node)
				{
					 node_viewer->template_node = ACCESS(FE_node)(
							CREATE(FE_node)(0, (struct FE_region *)NULL, initial_node));
				}
				else
				{
					 node_viewer->template_node = (struct FE_node *)NULL;
				}
				wxLogNull logNo;
				node_viewer->wx_node_viewer = new wxNodeViewer(node_viewer);
				node_viewer->collpane = 
					 XRCCTRL(*node_viewer->wx_node_viewer, "VariableViewerPanel", wxScrolledWindow);
				node_viewer->win=NULL;
				node_viewer->current_node= initial_node;
				wxBoxSizer *Collpane_sizer = new wxBoxSizer( wxVERTICAL );
				node_viewer->collpane->SetSizer(Collpane_sizer);
				Node_viewer_update_collpane(node_viewer);
				node_viewer->frame=
					 XRCCTRL(*node_viewer->wx_node_viewer, "CmguiNodeViewer", wxFrame);
				node_viewer->frame->SetTitle(dialog_title);
				node_viewer->frame->Layout();
				node_viewer->frame->SetMinSize(wxSize(50,100));
				node_viewer->collpane->Layout();
#endif /* defined (WX_USER_INTERFACE) */	
		 }
		 else
		 {
				display_message(ERROR_MESSAGE,
					 "CREATE(Node_viewer).  Could not open hierarchy");
		 }
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"CREATE(Node_viewer).  Invalid argument(s)");
	}
	if (node_viewer_address)
	{
		 *node_viewer_address=node_viewer;
	}
	LEAVE;
	
	return (node_viewer);
} /* CREATE(Node_viewer) */

int DESTROY(Node_viewer)(struct Node_viewer **node_viewer_address)
/*******************************************************************************
LAST MODIFIED : 6 June 2001

DESCRIPTION:
Destroys the Node_viewer. See also Node_viewer_close_CB.
==============================================================================*/
{
	int return_code;
	struct Node_viewer *node_viewer;

	ENTER(DESTROY(node_viewer));
	if (node_viewer_address&&
		(node_viewer= *node_viewer_address))
	{
		/* end callback from region */
		FE_region_remove_callback(node_viewer->fe_region,
			Node_viewer_FE_region_change, (void *)node_viewer);
		if (node_viewer->computed_field_manager_callback_id)
		{
			MANAGER_DEREGISTER(Computed_field)(
					node_viewer->computed_field_manager_callback_id,
					node_viewer->computed_field_manager);
			node_viewer->computed_field_manager_callback_id = NULL;
		}
		if (node_viewer->nodal_value_types)
		{
			 DEALLOCATE(node_viewer->nodal_value_types);
		}
		if (node_viewer->wx_node_viewer)
		{
			 delete node_viewer->wx_node_viewer;
		}
		/* deaccess the local node_copy */
		REACCESS(FE_node)(&(node_viewer->node_copy),(struct FE_node *)NULL);
		DEACCESS(FE_node)(&(node_viewer->template_node));
		DEACCESS(Time_object)(&(node_viewer->time_object));
		DEALLOCATE(*node_viewer_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Node_viewer).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Node_viewer) */

struct FE_node *Node_viewer_get_node(struct Node_viewer *node_viewer)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns the node currently in the <node_viewer>.
==============================================================================*/
{
	struct FE_node *node = NULL;

	ENTER(node_viewer_get_node);
	if (node_viewer)
	{
		 if (node_viewer->wx_node_viewer)
		 {
				node = node_viewer->wx_node_viewer->get_selected_node();
		 }
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_viewer_get_node.  Missing dialog.");
	}
	LEAVE;

	return (node);
} /* node_viewer_get_node */

int Node_viewer_set_node(struct Node_viewer *node_viewer,struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Makes <node> the current_node in <node_viewer>.
==============================================================================*/
{
	int return_code = 0;

	ENTER(Node_viewer_set_node);
	if (node_viewer&&node)
	{
		 if (node_viewer->wx_node_viewer)
		 {
				return_code = node_viewer->wx_node_viewer->set_selected_node(node);
				Node_viewer_set_viewer_node(node_viewer);
				if (node_viewer->wx_node_viewer && node_viewer->collpane)
				{
					Node_viewer_update_collpane(node_viewer);
				}
		 }
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_viewer_set_node.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Node_viewer_set_node */

int Node_viewer_bring_window_to_front(struct Node_viewer *node_viewer)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Pops the window for <node_viewer> to the front of those visible.
??? De-iconify as well?
==============================================================================*/
{
	int return_code;

	ENTER(Node_viewer_bring_window_to_front);
	if (node_viewer->wx_node_viewer)
	{
		 node_viewer->wx_node_viewer->Raise();
		 return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_viewer_bring_window_to_front.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_viewer_bring_window_to_front */

static int Node_viewer_set_viewer_node(struct Node_viewer *node_viewer)
/*******************************************************************************
LAST MODIFIED : 24 April 2007

DESCRIPTION :
Gets the current node from the select widget, makes a copy of it if not NULL,
and passes it to the node_viewer.
==============================================================================*/
{
	int return_code;
	struct FE_node *node, *node_copy = NULL;
	return_code = 1;
	if (node_viewer)
	{
		if (node_viewer->wx_node_viewer != NULL)
		{
			node = node_viewer->wx_node_viewer->get_selected_node();
			if (node != NULL)
			{
				node_copy = ACCESS(FE_node)(CREATE(FE_node)(get_FE_node_identifier(node),
					(struct FE_region *)NULL, node));
			}
		}
		else
		{
			node_copy = (struct FE_node *)NULL;
		}
		REACCESS(FE_node)(&(node_viewer->node_copy), node_copy);
		node_viewer_widget_set_node(node_viewer, node_viewer->fe_region, node_copy);
		if (node_copy)
		{
			DEACCESS(FE_node)(&node_copy);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_viewer_set_viewer_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* Node_viewer_set_viewer_node */

static void Node_viewer_FE_region_change(struct FE_region *fe_region,
	struct FE_region_changes *changes, void *node_viewer_void)
/*******************************************************************************
LAST MODIFIED : 24 April 2007

DESCRIPTION :
Note that we do not have to handle add, delete and identifier change messages
here as the select widget does this for us. Only changes to the content of the
object cause updates.
==============================================================================*/
{
	int fe_node_change;
	struct Node_viewer *node_viewer;

	ENTER(Node_viewer_Cmiss_region_change);
	if (fe_region && changes && 
		(node_viewer = (struct Node_viewer *)node_viewer_void))
	{
		 if (node_viewer->wx_node_viewer)
		 {
				if (CHANGE_LOG_QUERY(FE_node)(changes->fe_node_changes,
							node_viewer->wx_node_viewer->get_selected_node(),
							&fe_node_change))
				{
					 if (fe_node_change | CHANGE_LOG_OBJECT_CHANGED(FE_node))
					 {
							Node_viewer_set_viewer_node(node_viewer);
					 }
				}		
		 }
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"Node_viewer_Cmiss_region_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_viewer_Cmiss_region_change */

char *node_viewer_update_value(Node_viewer *node_viewer, Computed_field *field,
	int component_number, FE_nodal_value_type type, int version)
{
/*******************************************************************************
LAST MODIFIED : 24 April 2007

DESCRIPTION :
Give the textctrl a value.
==============================================================================*/
	char *cmiss_number_string, *new_value_string, temp_value_string[VALUE_STRING_SIZE];
	int element_dimension, j, number_of_components = 0;
	struct CM_element_information cm_identifier;
	struct FE_node *node = NULL;
	struct Computed_field *temp_field  = NULL;

	ENTER(node_viewer_update_value);
	if (node_viewer&&(node = node_viewer->current_node) &&
		(temp_field=node_viewer->current_field))
		number_of_components = Computed_field_get_number_of_components(temp_field);
	FE_field *fe_field = 0;
	FE_value time = Time_object_get_current_time(node_viewer->time_object);
	FE_value *values = 0;
	cmiss_number_string = (char *)NULL;
	Cmiss_field_module_id field_module = Cmiss_field_get_field_module(temp_field);
	Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
	Cmiss_field_cache_set_time(field_cache, time);
	Cmiss_field_cache_set_node(field_cache, node);
	 if (Computed_field_is_type_finite_element(temp_field))
	 {
			Computed_field_get_type_finite_element(temp_field, &fe_field);
	 }
	 else
	 {
			if (Computed_field_is_type_cmiss_number(temp_field))
			{
				 cmiss_number_string = Cmiss_field_evaluate_string(temp_field, field_cache);
			}
			else if (Computed_field_has_numerical_components(temp_field, NULL))
			{				
				 if (ALLOCATE(values, FE_value, number_of_components))
				 {
						if (CMISS_OK != Cmiss_field_evaluate_real(temp_field, field_cache, number_of_components, values))
						{
							 DEALLOCATE(values);
						}
				 }
			}
	 }
	 if (field)
 	 {
			new_value_string = (char *)NULL;
			if (fe_field)
			{
				 switch (get_FE_field_value_type(fe_field))
				 {
						case ELEMENT_XI_VALUE:
						{
							 char element_char = '\0', xi_string[30];
							 FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
							 struct FE_element *element;
							 if (get_FE_nodal_element_xi_value(
											node, fe_field,
											component_number,
											version,type, &element, xi))
							 {
									get_FE_element_identifier(element,
										 &cm_identifier);
									switch (cm_identifier.type)
									{
										 case CM_FACE:
										 {
												element_char = 'F';
										 } break;
											case CM_LINE:
											{
												 element_char = 'L';
											} break;
										 default:
										 {
												element_char = 'E';
										 } break;
									}				
							 }
							 element_dimension = get_FE_element_dimension(element);
							 sprintf(temp_value_string, " %c %d %d", element_char,
									cm_identifier.number, element_dimension);
							 for (j = 0; j < element_dimension; j++)
							 {
									sprintf(xi_string," %g",xi[j]);
									strcat(temp_value_string, xi_string);
							 }
							 new_value_string = duplicate_string(temp_value_string);
						}break;
						case FE_VALUE_VALUE:
						{
							 FE_value fe_value_value;
							 
							 get_FE_nodal_FE_value_value(node,
										fe_field, component_number,
									version,type,time,&fe_value_value);
							 sprintf(temp_value_string, FE_VALUE_INPUT_STRING,
									fe_value_value);
							 new_value_string = duplicate_string(temp_value_string);
						} break;
						case INT_VALUE:
						{
							 int int_value;
							 
							 get_FE_nodal_int_value(node,
									fe_field, component_number,
									version,type,time,&int_value);
							 sprintf(temp_value_string, "%d", int_value);
							 new_value_string = duplicate_string(temp_value_string);
						} break;	
						case STRING_VALUE:
						{
							 get_FE_nodal_string_value(node,
									fe_field, component_number,
									version,type, &new_value_string);
						} break;
						default:
						{
							 display_message(ERROR_MESSAGE,
									"node_field_viewer_widget_update_values.  "
									"Unsupported value_type for FE_field");
						} break;
				 }
			}
			else if (cmiss_number_string)
			{
				 /* copy and clear cmiss_number_string to avoid allocate */
				 new_value_string = cmiss_number_string;
				 cmiss_number_string = (char *)NULL;
			}
			else if (!Computed_field_has_numerical_components(temp_field, NULL))
			{
				// assume all non-numeric value types are scalar
				new_value_string = Cmiss_field_evaluate_string(temp_field, field_cache);
			}
			else /* all other types of computed field */
			{
				 if (values !=NULL)
				 {
						sprintf(temp_value_string, FE_VALUE_INPUT_STRING,
							 values[component_number]);
				 }
				 else
				 {
						sprintf(temp_value_string, "%s", "nan"
							 );
				 }		
				 new_value_string = duplicate_string(temp_value_string);
			}
	 }
	 if (values)
	 {
			DEALLOCATE(values);
	 }
	Cmiss_field_cache_destroy(&field_cache);
	Cmiss_field_module_destroy(&field_module);
	 return(new_value_string);
	 LEAVE;
}


int node_viewer_add_textctrl(Node_viewer *node_viewer, Computed_field *field, int component_number, FE_nodal_value_type type, int version)
/*******************************************************************************
LAST MODIFIED : 24 April 2007

DESCRIPTION :
Add textctrl box onto the viewer.
==============================================================================*/
{
	char *temp_string;
	wxNodeViewerTextCtrl *node_viewer_text = 
		new wxNodeViewerTextCtrl(node_viewer, field, component_number, type, version);
	temp_string = node_viewer_update_value(
		node_viewer, field, component_number, type, version);
	if (temp_string != NULL)
	{
		node_viewer_text ->Create (node_viewer->win, -1, temp_string,wxDefaultPosition,
			wxDefaultSize,wxTE_PROCESS_ENTER);
		DEALLOCATE(temp_string);
	}
	else
	{
		node_viewer_text->Create (node_viewer->win, -1, "ERROR",wxDefaultPosition,
			wxDefaultSize,wxTE_PROCESS_ENTER);	
	}
	node_viewer_text->Connect(wxEVT_COMMAND_TEXT_ENTER,
		wxCommandEventHandler(wxNodeViewerTextCtrl::OnNodeViewerTextCtrlEntered));
	node_viewer_text->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxNodeViewerTextCtrl::OnNodeViewerTextCtrlEntered));
	node_viewer->grid_field->Add(node_viewer_text, 0,
		wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxADJUST_MINSIZE, 0); 
	return (1);
}

static int node_viewer_setup_components(
	struct Node_viewer *node_viewer)
/*******************************************************************************
LAST MODIFIED : 5 December 2000

DESCRIPTION :
Creates the array of cells containing field component values and derivatives
and their labels.
==============================================================================*/
{
	const char *component_name, * new_string;
	enum FE_nodal_value_type *component_nodal_value_types,nodal_value_type;
	int col,comp_no,k,number_of_components,number_of_derivatives,
		number_of_unknown_nodal_value_types,return_code;
	struct Computed_field *field;
	struct FE_field *fe_field;
	struct FE_node *node;
	struct Nodal_value_information *nodal_value_information;

	int tmp, count, indicator;
	wxString tmp_string;
	count = 1;												
	indicator =0;
	tmp = 0;
	number_of_derivatives = 0;
	if (node_viewer)
	{
		 return_code=1;
		 node_viewer->number_of_components=-1;
		 
		 if ((node=node_viewer->current_node)&&
				(field=node_viewer->current_field)&&
				Computed_field_is_defined_at_node(field,node))
		 {
				number_of_components=Computed_field_get_number_of_components(field);
				node_viewer->number_of_components=number_of_components;
				if (node_viewer->nodal_value_types)
				{
					 DEALLOCATE(node_viewer->nodal_value_types);
				}
				/* Computed_fields default to 1 nodal_value_type */
				node_viewer->number_of_nodal_value_types=1;
				/* now create another */
				if (node_viewer)
				{
					 if ((!Computed_field_is_type_finite_element(field))||
							(Computed_field_get_type_finite_element(field,&fe_field)&&
								 (node_viewer->nodal_value_types=get_FE_node_field_value_types(
										 node,fe_field,&(node_viewer->number_of_nodal_value_types),
										 &number_of_unknown_nodal_value_types))))
					 {
							if (Computed_field_is_type_finite_element(field)&&
								 (0<number_of_unknown_nodal_value_types))
							{
								 display_message(WARNING_MESSAGE,"Unknown nodal derivative types");
							}
							component_nodal_value_types=(enum FE_nodal_value_type *)NULL;
							for (comp_no=0;(comp_no<=number_of_components)&&return_code;comp_no++)
							{
								 if (0 < comp_no)
								 {
										if (Computed_field_is_type_finite_element(field))
										{
											 number_of_derivatives=
													get_FE_node_field_component_number_of_derivatives(
														 node,fe_field,comp_no-1);
											 if (!(component_nodal_value_types=
														 get_FE_node_field_component_nodal_value_types(
																node,fe_field,comp_no-1)))
											 {
													return_code=0;
											 }
										}
										else
										{
											 number_of_derivatives=0;
										}
								 }						
								 for (col=0;col<=(node_viewer->number_of_nodal_value_types)&&
												 return_code;col++)
								 {
										nodal_value_information=(struct Nodal_value_information *)NULL;
										new_string=NULL;
										if (0 == col)
										{
											 nodal_value_type=FE_NODAL_UNKNOWN;
										}
										else
										{
											 if (node_viewer->nodal_value_types)
											 {
													nodal_value_type=node_viewer->nodal_value_types[col-1];
											 }
											 else
											 {
													nodal_value_type=FE_NODAL_VALUE;
											 }
										}
										if (0 == comp_no)
										{
											 /* nodal value type labels; note blank cell in the corner */
											 if (0 < col)
											 {
													new_string=ENUMERATOR_STRING(FE_nodal_value_type)(nodal_value_type);
													tmp_string = new_string;
											 }
										}
										else
										{
											 if (0 == col)
											 {
													/* component label */
													if ((Computed_field_is_type_finite_element(field)&&
																(component_name=get_FE_field_component_name(fe_field,
																	 comp_no-1)))||
														 (component_name=
																Computed_field_get_component_name(field,comp_no-1)))
													{
														 new_string = component_name;
														 tmp_string = new_string;
														 DEALLOCATE(component_name);
													}
													else
													{
														 display_message(ERROR_MESSAGE,
																"node_viewer_widget_setup_components.  "
																"Could not get field component name");
														 return_code=0;
													}
											 }
											 else
											 {
													/* work out if nodal_value_type defined for this component */
													k=0;
													if (component_nodal_value_types)
													{
														 while ((k<=number_of_derivatives)&&
																(nodal_value_type != component_nodal_value_types[k]))
														 {
																k++;
														 }
													}
													if (k<=number_of_derivatives)
													{
														 if (ALLOCATE(nodal_value_information,
																	 struct Nodal_value_information,1))
														 {
																nodal_value_information->field=field;
																nodal_value_information->component_number=comp_no-1;
																nodal_value_information->type=nodal_value_type;
																nodal_value_information->version=0;
														 }
														 else
														 {
																display_message(ERROR_MESSAGE,
																	 "node_viewer_setup_components.  "
																	 "Could not allocate nodal_value_information");
																return_code=0;
														 }
													}
											 }
										}
										if (tmp == 0)
										{
											 node_viewer->grid_field = new wxGridSizer(
													number_of_components+1, node_viewer->number_of_nodal_value_types+1,1,1);		
											 node_viewer->grid_field->Add(new wxStaticText(
													node_viewer->win, -1, wxT("")),1,wxEXPAND|wxADJUST_MINSIZE, 0);	
											 tmp = 1;
										}
										if (return_code)
										{
											 if (nodal_value_information)
											 {
													node_viewer_add_textctrl(node_viewer, field, (comp_no - 1),nodal_value_type, 0);
													count = count +1;
													indicator =1;	
													/* string and element_xi fields should be shown wider */
													DEALLOCATE(nodal_value_information);
											 }
											 else
											 {
													if (new_string)
													{
														 /* now create another */
														 if ((indicator == 1) && (count < node_viewer->number_of_nodal_value_types+1))
														 {
																for (int n=count; n < node_viewer->number_of_nodal_value_types+1; n++) 
																{ 
																	 node_viewer->grid_field->Add(new wxStaticText(node_viewer->win, -1, wxT("")),1,
																			wxEXPAND|wxALIGN_CENTER_VERTICAL|
																			wxALIGN_CENTER_HORIZONTAL|wxADJUST_MINSIZE, 0);
																}
														 }
														 node_viewer->grid_field->Add(new wxStaticText(node_viewer->win, -1, wxT(tmp_string)),1,
																wxALIGN_CENTER_VERTICAL|
																wxALIGN_CENTER_HORIZONTAL|wxADJUST_MINSIZE, 0);
														 indicator =0;
														 count =1;
													}
											 }
										}	
								 }
								 if (component_nodal_value_types)
								 {
										DEALLOCATE(component_nodal_value_types);
								 }
							}
					 }
					 else
					 {
							display_message(ERROR_MESSAGE,
								 "node_viewer_setup_components.  "
								 "Could not get nodal value types");
							return_code=0;
					 }
				}
				else
				{
					 display_message(ERROR_MESSAGE,
							"node_viewer_setup_components.  "
							"Could not make component_rowcol");
					 return_code=0;
				}
		 }
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"node_viewer_setup_components.  Invalid argument(s)");
	}

	return (return_code);
} /* node_viewer_widget_setup_components */

static int node_field_time_change_callback(
	struct Time_object *time_object, double current_time,
	void *node_viewer_void)
/*******************************************************************************
LAST MODIFIED : 5 December 2001

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Node_viewer *node_viewer;

	USE_PARAMETER(current_time);
	if(time_object && (node_viewer = 
			(struct Node_viewer *)node_viewer_void))
	{
// 		node_viewer_widget_update_values(node_viewer);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_field_viewer_widget_time_change_callback.  Invalid argument(s)");
		return_code = 0;
	}
	return(return_code);
} /* node_field_viewer_widget_time_change_callback */

enum FE_nodal_value_type *get_FE_node_field_value_types(
	struct FE_node *node,struct FE_field *field,int *number_of_nodal_value_types,
	int *number_of_unknown_nodal_value_types)
/*******************************************************************************
LAST MODIFIED : 27 April 2007

DESCRIPTION :
Returns an array containing the list of named nodal value/derivative types
defined for any - but not necessarily all - components of the <field> at <node>.
Nodal value types are returned in ascending order.
On return <number_of_unknown_nodal_value_types> contains the number of nodal
values whose value_type was not recognized.
==============================================================================*/
{
	enum FE_nodal_value_type *component_nodal_value_types,nodal_value_type,
		*nodal_value_types;
	int i,j,k,l,number_of_components,number_of_derivatives,return_code;

	ENTER(get_FE_node_field_value_types);
	nodal_value_types=(enum FE_nodal_value_type *)NULL;
	if (node&&field&&number_of_nodal_value_types&&
		number_of_unknown_nodal_value_types)
	{
		*number_of_nodal_value_types=0;
		*number_of_unknown_nodal_value_types=0;
		return_code=1;
		number_of_components=get_FE_field_number_of_components(field);
		for (i=0;(i<number_of_components)&&return_code;i++)
		{
			number_of_derivatives=
				get_FE_node_field_component_number_of_derivatives(node,field,i);
			component_nodal_value_types=
				get_FE_node_field_component_nodal_value_types(node,field,i);
			if (component_nodal_value_types != NULL)
			{
				for (j=0;(j<=number_of_derivatives)&&return_code;j++)
				{
					nodal_value_type=component_nodal_value_types[j];
					/* ignore unknown types */
					if ((FE_NODAL_VALUE==nodal_value_type)||
						(FE_NODAL_D_DS1==nodal_value_type)||
						(FE_NODAL_D_DS2==nodal_value_type)||
						(FE_NODAL_D_DS3==nodal_value_type)||
						(FE_NODAL_D2_DS1DS2==nodal_value_type)||
						(FE_NODAL_D2_DS1DS3==nodal_value_type)||
						(FE_NODAL_D2_DS2DS3==nodal_value_type)||
						(FE_NODAL_D3_DS1DS2DS3==nodal_value_type))
					{
						/* search to find this type */
						k=0;
						while ((k < (*number_of_nodal_value_types))&&
							(nodal_value_type > nodal_value_types[k]))
						{
							k++;
						}
						if ((k >= (*number_of_nodal_value_types))||
							(nodal_value_type != nodal_value_types[k]))
						{
							 enum FE_nodal_value_type *temp_nodal_value_types;
							/* add to list in numerical order */
							if (REALLOCATE(temp_nodal_value_types,nodal_value_types,
								enum FE_nodal_value_type ,(*number_of_nodal_value_types)+1))
							{
								nodal_value_types=temp_nodal_value_types;
								for (l = *number_of_nodal_value_types;l>k;l--)
								{
									nodal_value_types[l]=nodal_value_types[l-1];
								}
								nodal_value_types[k]=nodal_value_type;
								(*number_of_nodal_value_types)++;
							}
							else
							{
								display_message(ERROR_MESSAGE,"get_FE_node_field_value_types.  "
									"Could not reallocate nodal_values_information");
								DEALLOCATE(nodal_value_types);
								*number_of_nodal_value_types = 0;
								return_code=0;
							}
						}
					}
					else
					{
						(*number_of_unknown_nodal_value_types)++;
					}
				}
				DEALLOCATE(component_nodal_value_types);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"get_FE_node_field_value_types.  Could not get nodal_value_types");
					 DEALLOCATE(nodal_value_types);
					 *number_of_nodal_value_types = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_node_field_value_types.  Invalid argument(s)");
	}
	LEAVE;

	return (nodal_value_types);
} /* get_FE_node_field_value_types */

int node_viewer_set_node_field(void *node_viewer_void,
	struct FE_node *node,struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 30 April 2003

DESCRIPTION :
Sets the node/field being edited in the <node_field_viewer_widget>. Note that
the viewer works on the node itself, not a local copy. Hence, only pass
unmanaged nodes to this widget.
==============================================================================*/
{
	int return_code;
	struct Node_viewer *node_viewer;
	if ((node_viewer = static_cast<Node_viewer*>(node_viewer_void)) && ((!node)||(!field)||
				Computed_field_is_defined_at_node(field,node)))
	{
		if (node_viewer)
		{
			REACCESS(FE_node)(&(node_viewer->current_node), node);
			REACCESS(Computed_field)(&(node_viewer->current_field), field);
 			node_viewer_setup_components(node_viewer);
			if (node && field)
			{
				if (node_viewer->time_object)
				{
					if (Computed_field_has_multiple_times(field))
					{
						if (!node_viewer->time_object_callback)
						{
							node_viewer->time_object_callback = 
								Time_object_add_callback(node_viewer->time_object,
									node_field_time_change_callback,(void *)node_viewer);
						}
					}
					else
					{
						if (node_viewer->time_object_callback)
						{
							Time_object_remove_callback(node_viewer->time_object,
								node_field_time_change_callback,
								(void *)node_viewer);
							node_viewer->time_object_callback = 0;
						}					
					}
				}
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"node_viewer_set_node_field.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_viewer_set_node_field.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* node_field_viewer_widget_set_node_field */

int node_viewer_widget_set_node(Node_viewer *node_viewer,
	struct FE_region *fe_region, struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 30 April 2003

DESCRIPTION :
Sets the node being edited in the <node_viewer_widget>. Note that the viewer
works on the node itself, not a local copy. Hence, only pass unmanaged nodes to
this widget.
==============================================================================*/
{
	int change_conditional_function,return_code = 0;
	MANAGER_CONDITIONAL_FUNCTION(Computed_field)
		*choose_field_conditional_function;
	struct Computed_field *field;
	struct FE_node *template_node = NULL;

	if (node_viewer)
	{
		 change_conditional_function=0;
		 if (node)
		 {
				field = node_viewer->current_field;
				if (!(node_viewer->template_node)||
					 (!equivalent_computed_fields_at_nodes(node,
							node_viewer->template_node)))
				{
					 choose_field_conditional_function=
							Computed_field_is_defined_at_node_conditional;
					 change_conditional_function=1;
					 if ((!field)||(!Computed_field_is_defined_at_node(field,node)))
					 {
							field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
								choose_field_conditional_function,(void *)node,
								node_viewer->computed_field_manager);
					 }
					 template_node = CREATE(FE_node)(0, (struct FE_region *)NULL, node);
				}
		 }
		 else
		 {
				field=(struct Computed_field *)NULL;
				if (node_viewer->current_node)
				{
					 choose_field_conditional_function=
							(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
					 change_conditional_function=1;
					 template_node=(struct FE_node *)NULL;
				}
		 }
		 node_viewer->current_node=node;
		 node_viewer->fe_region=fe_region;
		 if (change_conditional_function)
		 {
				REACCESS(FE_node)(&(node_viewer->template_node),template_node);
		 }
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"node_viewer_widget_set_node.  Missing widget data");
		 return_code=0;
	}
	
	return (return_code);
} /* node_viewer_widget_set_node */

static int Node_viewer_set_Cmiss_region(struct Node_viewer *node_viewer,
	struct Cmiss_region *region)
/*******************************************************************************
LAST MODIFIED : 8 September 2008

DESCRIPTION :
Sets the <region> used by <node_viewer> and update the chooser to include fields
in this region only.
==============================================================================*/
{
	int return_code = 1;
	FE_node *initial_node = NULL;
	ENTER(Node_viewer_set_Cmiss_region);
	if (node_viewer)
	{
		if (region != node_viewer->region)
		{
			node_viewer->region = region;
			FE_region_remove_callback(node_viewer->fe_region,
				Node_viewer_FE_region_change, (void *)node_viewer);
			if (region)
			{
				node_viewer->fe_region = Cmiss_region_get_FE_region(region);

				if (node_viewer->use_data)
				{
					node_viewer->fe_region = FE_region_get_data_FE_region(
						node_viewer->fe_region);
				}
				FE_region_add_callback(node_viewer->fe_region,
						Node_viewer_FE_region_change, (void *) node_viewer);
				initial_node = FE_region_get_first_FE_node_that(node_viewer->fe_region,
					(LIST_CONDITIONAL_FUNCTION(FE_node) *) NULL, (void *) NULL);
				if (initial_node)
				{
					node_viewer->current_node=initial_node;
					Node_viewer_set_node(node_viewer, initial_node);
				}
				if (node_viewer->computed_field_manager_callback_id)
				{
					MANAGER_DEREGISTER(Computed_field)(
							node_viewer->computed_field_manager_callback_id,
							node_viewer->computed_field_manager);
					node_viewer->computed_field_manager_callback_id = NULL;
				}
				node_viewer->computed_field_manager	=
					Cmiss_region_get_Computed_field_manager(region);
				if (node_viewer->computed_field_manager)
				{
					node_viewer->computed_field_manager_callback_id =
						MANAGER_REGISTER(Computed_field)(Node_viewer_Computed_field_change,
							(void *)node_viewer,	node_viewer->computed_field_manager);
				}
			}
			else
			{
				return_code=0;
				Node_viewer_set_node(node_viewer, NULL);
				node_viewer->current_node=NULL;
				node_viewer->region = NULL;
				node_viewer->fe_region = (struct FE_region *)NULL;
			}
		}
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"Node_tool_set_Cmiss_region.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Node_tool_set_Cmiss_region */


