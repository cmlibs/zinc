/*******************************************************************************
FILE : node_viewer_wx.cpp

LAST MODIFIED : 23 April 2007

DESCRIPTION :
Dialog for selecting nodes and viewing and/or editing field values. Works with
FE_node_selection to display the last selected node, or set it if entered in
this dialog.
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
extern "C" {
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "finite_element/finite_element.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "node/node_viewer_wx.h"
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#include "time/time.h"
}
#if defined (WX_USER_INTERFACE)
#include <wx/collpane.h>
#include <wx/grid.h>
#include "wx/wx.h"
#include "wx/xrc/xmlres.h"
#include "node/node_viewer_wx.xrch"
#include "choose/text_FE_choose_class.hpp"
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
	 struct Computed_field_package *computed_field_package;
	 struct Node_viewer **node_viewer_address;
	 struct FE_node *node_copy, *initial_node, *current_node, *template_node;
	 struct Cmiss_region *region;
	 struct FE_region *fe_region;
	 struct FE_node_selection *node_selection;
	 struct User_interface *user_interface;	
	 struct MANAGER(Computed_field) *computed_field_manager;
	 struct Time_object *time_object;
	 int time_object_callback;
	struct Computed_field *current_field;
#if defined (WX_USER_INTERFACE)
	 wxNodeViewer *wx_node_viewer;
	 wxScrolledWindow *collpane;
	 wxWindow *win;
	 wxBoxSizer *paneSz;
	 wxFrame *frame;
	 wxGridSizer *grid_field;
	 wxSize frame_size;
#endif /* (WX_USER_INTERFACE) */
}; /* node_viewer_struct */

/*
Prototype
------------
*/
static void Node_viewer_node_selection_change(
	struct FE_node_selection *node_selection,
	struct FE_node_selection_changes *changes,void *node_viewer_void);
/*******************************************************************************
LAST MODIFIED : 24 April 2007

DESCRIPTION :
Callback for change in the global node selection.
==============================================================================*/

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

static int node_viewer_add_collpane(struct Computed_field *current_field, void *node_viewer_void);
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

public:
	 
	 wxNodeViewer(Node_viewer *node_viewer): 
			node_viewer(node_viewer)
	 {
			wxXmlInit_node_viewer_wx();
			node_viewer->wx_node_viewer = (wxNodeViewer *)NULL;
			wxXmlResource::Get()->LoadFrame(this,
				 (wxWindow *)NULL, _T("CmguiNodeViewer"));
			
			wxPanel *node_text_panel =
				 XRCCTRL(*this, "NodeTextPanel",wxPanel);
			node_text_chooser =
				 new FE_object_text_chooser<FE_node, FE_region_FE_object_method_class(node) >
				 (node_text_panel, node_viewer->initial_node, node_viewer->fe_region, 
						(LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL,
						NULL);
			Callback_base< FE_node* > *node_text_callback = 
				 new Callback_member_callback< FE_node*, 
				 wxNodeViewer, int (wxNodeViewer::*)(FE_node *) >
				 (this, &wxNodeViewer::node_text_callback);
			node_text_chooser->set_callback(node_text_callback);
			Show();
	 };
	 
	 wxNodeViewer()
	 {
	 };

  ~wxNodeViewer()
	 {
			delete node_text_chooser;
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
				 FE_node_selection_begin_cache(node_viewer->node_selection);
				 FE_node_selection_clear(node_viewer->node_selection);
				 if (node)
				 {
						node_viewer->current_node = node;
						FE_node_selection_select_node(node_viewer->node_selection, node);
						FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
							 node_viewer_add_collpane,
							 (void *)node_viewer,
							 node_viewer->computed_field_manager);
				 }
				 else
				 {
						Node_viewer_set_viewer_node(node_viewer);
						if (node_viewer->wx_node_viewer && node_viewer->collpane)
						{
							 FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
									node_viewer_add_collpane,
									(void *)node_viewer,
									node_viewer->computed_field_manager);
						}
				 }
				 FE_node_selection_end_cache(node_viewer->node_selection);
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
			frame = XRCCTRL(*this, "CmguiNodeViewer", wxFrame);
			frame->SetMinSize(wxSize(50,100));
			frame->SetMaxSize(wxSize(2000,2000));
	 }

	void OnOKpressed(wxCommandEvent &event)
	 {
			if (Node_viewer_apply_changes(node_viewer))
			{
				 DESTROY(Node_viewer)(node_viewer->node_viewer_address);
			}
	 }

	 void OnApplypressed(wxCommandEvent &event)
	 {
			Node_viewer_apply_changes(node_viewer);
	 }

	 void OnRevertpressed(wxCommandEvent &event)
	 {
			Node_viewer_set_viewer_node(node_viewer);
			if (node_viewer->wx_node_viewer && node_viewer->collpane)
			{
				 FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
						node_viewer_add_collpane,
						(void *)node_viewer,
						node_viewer->computed_field_manager);
			}
	 }

	 void OnCancelpressed(wxCommandEvent &event)
	 {
			DESTROY(Node_viewer)(node_viewer->node_viewer_address);
	 }

	 void NodeViewerTextEntered(wxTextCtrl *textctrl , Node_viewer *node_viewer,Computed_field *field, int component_number, FE_nodal_value_type type, int version)
	 {
			char *value_string, *temp_string;
			FE_value time;
			struct FE_field *fe_field;
			struct FE_node *node;
			if (field&&node_viewer&&
				 (node=node_viewer->current_node))
			{
				 time = Time_object_get_current_time(node_viewer->time_object);			 
				 if (node_viewer->current_field = field)
				 {
						if (value_string=const_cast<char *>((textctrl->GetValue()).c_str()))
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
												if (Computed_field_evaluate_at_node(field,node,time,values))
												{
													 sscanf(value_string,FE_VALUE_INPUT_STRING,
															&values[component_number]);
													 Computed_field_set_values_at_node(field,node,time,values);
												}
												Computed_field_clear_cache(field);
												DEALLOCATE(values);
										 }
									}
							 }
						}
						/* refresh value shown in the text field widgets */
						if (temp_string = node_viewer_update_value(node_viewer, field, component_number, type, version))
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
	 
	 struct FE_node  *get_node_selection()
	 {
			return node_text_chooser->get_object();
	 }

	 int set_node_selection(FE_node *new_node)
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

  void OnNodeViewerTextCtrlEntered(wxCommandEvent& Event)
  {
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
			condition = Computed_field_is_defined_at_node_conditional(current_field, (void *)node_viewer->current_node);
	 }
	 else
	 {
			condition = 1;
	 }
	 if (condition)
	 {
			wxScrolledWindow *panel = node_viewer->collpane;
			char *identifier;
			int length;
			identifier = (char *)NULL;
			length = 	strlen(field_name);
			if (ALLOCATE(identifier,char,length+length+2))
			{
				 strcpy(identifier, field_name);
				 strcat(identifier, field_name);
				 identifier[length+length+1]='\0';
			}
			if (node_viewer->win = panel->FindWindowByName(identifier))
			{
				 node_viewer->win->DestroyChildren();
				 node_viewer->paneSz = new wxBoxSizer(wxVERTICAL);
			}
			else
			{
				 wxCollapsiblePane *collapsiblepane = new wxCollapsiblePane;
				 wxSizer *sizer = panel->GetSizer();
				 collapsiblepane->Create(panel, /*id*/-1, field_name);
				 sizer->Add(collapsiblepane, 0,wxALL, 5);
				 node_viewer->win = collapsiblepane->GetPane();
				 node_viewer->win->SetName(identifier);
				 node_viewer->paneSz = new wxBoxSizer(wxVERTICAL);
			}
			if (identifier)
			{
				 DEALLOCATE(identifier);
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
			frame = 
				 XRCCTRL(*node_viewer->wx_node_viewer, "CmguiNodeViewer", wxFrame);
			frame->Layout();
			frame->SetMinSize(wxSize(50,100));
	 }
	 if (field_name)
	 {
			DEALLOCATE(field_name);
	 }
	 return 1;
}

/*
Global functions
----------------
*/
struct Node_viewer *CREATE(Node_viewer)(
	struct Node_viewer **node_viewer_address,
	char *dialog_title,
	struct FE_node *initial_node,
	struct Cmiss_region *region,
	struct FE_node_selection *node_selection,
	struct Computed_field_package *computed_field_package,
	struct Time_object *time_object, struct User_interface *user_interface)
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
	struct Computed_field *initial_field;
	struct Node_viewer *node_viewer;
	MANAGER_CONDITIONAL_FUNCTION(Computed_field)
		*choose_field_conditional_function;
	ENTER(CREATE(Node_viewer));
	node_viewer = (struct Node_viewer *)NULL;
	if (node_viewer_address && dialog_title && region && node_selection &&
		computed_field_package && user_interface)
	{
		 /* allocate memory */
		 if (ALLOCATE(node_viewer,struct Node_viewer,1))
		 {
				if (initial_node)
				{
					choose_field_conditional_function=
						Computed_field_is_defined_at_node_conditional;
				}
				else
				{
					choose_field_conditional_function=
						(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
				}
				initial_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
						choose_field_conditional_function,(void *)initial_node,
					Computed_field_package_get_computed_field_manager(
						computed_field_package));
				node_viewer->nodal_value_types=(enum FE_nodal_value_type *)NULL;
				node_viewer->node_copy = (struct FE_node *)NULL;
				node_viewer->node_viewer_address = node_viewer_address;
				node_viewer->region = region;
				node_viewer->fe_region = Cmiss_region_get_FE_region(region);
				node_viewer->node_selection=node_selection;
				node_viewer->computed_field_package=computed_field_package;
				node_viewer->collpane = NULL;
				node_viewer->user_interface=user_interface;
				node_viewer->current_node=(struct FE_node *)NULL;
				node_viewer->computed_field_manager=Computed_field_package_get_computed_field_manager(computed_field_package);
			node_viewer->time_object = ACCESS(Time_object)(time_object);
			node_viewer->time_object_callback = 0;
			node_viewer->current_field=(struct Computed_field *)NULL;
			node_viewer->number_of_components=-1;
			node_viewer->number_of_nodal_value_types=0;
			node_viewer->grid_field = NULL;
#if defined (WX_USER_INTERFACE)
				node_viewer->wx_node_viewer = (wxNodeViewer *)NULL;
#endif /* defined (WX_USER_INTERFACE) */	
				if (!initial_node)
				{
					 if (!(initial_node = FIRST_OBJECT_IN_LIST_THAT(FE_node)(
										(LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
										FE_node_selection_get_node_list(node_selection))))
					 {
							if (initial_node = FE_region_get_first_FE_node_that(
										 node_viewer->fe_region,
										 (LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL, (void *)NULL))
							{
								 /* select the node to be displayed in dialog; note this is ok
										here as we are not receiving selection callbacks yet */
								 if (initial_node)
								 {
										node_viewer->template_node = ACCESS(FE_node)(
											 CREATE(FE_node)(0, (struct FE_region *)NULL, initial_node));
								 }
								 else
								 {
										node_viewer->template_node = (struct FE_node *)NULL;
								 }
								 FE_node_selection_select_node(node_selection,initial_node);
									FE_region_add_callback(node_viewer->fe_region,
										Node_viewer_FE_region_change, (void *)node_viewer);
									node_viewer->initial_node = initial_node;
									Node_viewer_set_viewer_node(node_viewer);
									if (node_viewer->wx_node_viewer && node_viewer->collpane)
									{
										 FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
												node_viewer_add_collpane,
												(void *)node_viewer,
												node_viewer->computed_field_manager);
									}
							}
					 }
				}
				/* get callbacks from global node selection */
				FE_node_selection_add_callback(node_selection,
					 Node_viewer_node_selection_change,(void *)node_viewer);
				/* make the dialog shell */
#if defined (WX_USER_INTERFACE)
				node_viewer->wx_node_viewer = new wxNodeViewer(node_viewer);
				node_viewer->collpane = 
					 XRCCTRL(*node_viewer->wx_node_viewer, "VariableViewerPanel", wxScrolledWindow);
				node_viewer->win=NULL;
				node_viewer->paneSz = NULL;
				node_viewer->current_node= initial_node;
				wxBoxSizer *Collpane_sizer = new wxBoxSizer( wxVERTICAL );
				node_viewer->collpane->SetSizer(Collpane_sizer);
				FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
					 node_viewer_add_collpane,
					 (void *)node_viewer,
					 node_viewer->computed_field_manager);
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
		/* end callbacks from global node selection */
		FE_node_selection_remove_callback(node_viewer->node_selection,
			Node_viewer_node_selection_change,(void *)node_viewer);
		/* deaccess the local node_copy */
		if (node_viewer->wx_node_viewer)
		{
			 delete node_viewer->wx_node_viewer;
		}
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
	struct FE_node *node;

	ENTER(node_viewer_get_node);
	if (node_viewer)
	{
		 if (node_viewer->wx_node_viewer)
		 {
				node = node_viewer->wx_node_viewer->get_node_selection();
		 }
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_viewer_get_node.  Missing dialog.");
		node=(struct FE_node *)NULL;
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
	int return_code;

	ENTER(Node_viewer_set_node);
	if (node_viewer&&node)
	{
		 if (node_viewer->wx_node_viewer)
		 {
				return_code= node_viewer->wx_node_viewer->set_node_selection(node);
				Node_viewer_set_viewer_node(node_viewer);
				if (node_viewer->wx_node_viewer && node_viewer->collpane)
				{
					 FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
							node_viewer_add_collpane,
							(void *)node_viewer,
							node_viewer->computed_field_manager);
				}
		 }
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_viewer_set_node.  Invalid argument(s)");
		return_code=0;
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
	struct FE_node *node, *node_copy;
	 return_code = 1;
	if (node_viewer)
	{
		 if (node_viewer->wx_node_viewer != NULL)
		 {
				if (node = node_viewer->wx_node_viewer->get_node_selection())
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
		 node_viewer_widget_set_node(node_viewer,
				node_viewer->fe_region, node_copy);
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

static void Node_viewer_node_selection_change(
	struct FE_node_selection *node_selection,
	struct FE_node_selection_changes *changes,void *node_viewer_void)
/*******************************************************************************
LAST MODIFIED : 24 April 2007

DESCRIPTION :
Callback for change in the global node selection.
==============================================================================*/
{
	 struct FE_node *last_selected_node, *select_node;
	 struct Node_viewer *node_viewer;

	ENTER(Node_viewer_node_selection_change);

	if (node_selection&&changes&&
		(node_viewer=(struct Node_viewer *)node_viewer_void))
	{
		/* get the last selected node and put it in the viewer */
		 if (node_viewer->wx_node_viewer)
		 {
				select_node = node_viewer->wx_node_viewer->get_node_selection();
		 }
		 if ((last_selected_node = FIRST_OBJECT_IN_LIST_THAT(FE_node)(
						 (LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL, (void *)NULL,
						 changes->newly_selected_node_list)) ||
				((last_selected_node = select_node) && IS_OBJECT_IN_LIST(FE_node)(
						select_node, FE_node_selection_get_node_list(node_selection))) ||
				(last_selected_node = FIRST_OBJECT_IN_LIST_THAT(FE_node)(
						(LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL, (void *)NULL,
						FE_node_selection_get_node_list(node_selection))))
		 {
				if ((last_selected_node != select_node) && (node_viewer->wx_node_viewer))
				{
					 node_viewer->wx_node_viewer->set_node_selection(last_selected_node);
					 node_viewer->current_node = last_selected_node;
				}
				Node_viewer_set_viewer_node(node_viewer);
				if (node_viewer->wx_node_viewer && node_viewer->collpane)
				{
					 FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
							node_viewer_add_collpane,
							(void *)node_viewer,
							node_viewer->computed_field_manager);
				}
		 }
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"Node_viewer_node_selection_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_viewer_node_selection_change */

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
	enum CHANGE_LOG_CHANGE(FE_node) change;
	struct Node_viewer *node_viewer;

	ENTER(Node_viewer_Cmiss_region_change);
	if (fe_region && changes && 
		(node_viewer = (struct Node_viewer *)node_viewer_void))
	{
		 if (node_viewer->wx_node_viewer)
		 {
				if (CHANGE_LOG_QUERY(FE_node)(changes->fe_node_changes,
							node_viewer->wx_node_viewer->get_node_selection(),
							&change))
				{
					 if (change | CHANGE_LOG_OBJECT_CHANGED(FE_node))
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

char *node_viewer_update_value(Node_viewer *node_viewer, Computed_field *field, int component_number, FE_nodal_value_type type, int version)
{
	 char *cmiss_number_string, *new_value_string, temp_value_string[VALUE_STRING_SIZE];
	 int element_dimension, j, number_of_components, return_code;
	 struct CM_element_information cm_identifier;
	 FE_value *values, time;
	 struct FE_field *fe_field;
	 struct FE_node *node;
	 struct Computed_field *temp_field;
	 if (node_viewer&&(node = node_viewer->current_node) &&
			(temp_field=node_viewer->current_field))
	 number_of_components = Computed_field_get_number_of_components(temp_field);
	 fe_field = (struct FE_field *)NULL;
	 time = Time_object_get_current_time(node_viewer->time_object);
	 values = (FE_value *)NULL;
	 cmiss_number_string = (char *)NULL;
	 return_code = 1;
	 if (Computed_field_is_type_finite_element(temp_field))
	 {
			return_code = Computed_field_get_type_finite_element(temp_field, &fe_field);
	 }
	 else
	 {
			if (Computed_field_is_type_cmiss_number(temp_field))
			{
				 if (!(cmiss_number_string = Computed_field_evaluate_as_string_at_node(temp_field,
									/*component_number*/-1, node, time)))
				 {
						return_code = 0;
				 }
			}
			else if (Computed_field_has_numerical_components(temp_field, NULL))
			{				
				 if (ALLOCATE(values, FE_value, number_of_components))
				 {
						if (!Computed_field_evaluate_at_node(temp_field, node, time, values))
						{
							 DEALLOCATE(values);
							 return_code = 0;
						}
						Computed_field_clear_cache(field);
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
							 char element_char, xi_string[30];
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
						}
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
				 new_value_string = Computed_field_evaluate_as_string_at_node(
						temp_field, component_number, node, time);
			}
			else /* all other types of computed field */
			{
				 sprintf(temp_value_string, FE_VALUE_INPUT_STRING,
						values[component_number]);
				 new_value_string = duplicate_string(temp_value_string);
			}
	 }
	 return(new_value_string);
}


int node_viewer_add_textctrl(Node_viewer *node_viewer, Computed_field *field, int component_number, FE_nodal_value_type type, int version)
{
	 char *temp_string;
	 wxNodeViewerTextCtrl *node_viewer_text = new wxNodeViewerTextCtrl(node_viewer, field, component_number, type, version);
	 if (temp_string = node_viewer_update_value(node_viewer, field, component_number, type, version))
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
	 node_viewer->grid_field->Add(node_viewer_text,0,
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
	 char *component_name, * new_string;
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
											 node_viewer->grid_field = new wxGridSizer(number_of_components+1, node_viewer->number_of_nodal_value_types+1,1,1);		
											 node_viewer->grid_field->Add(new wxStaticText(node_viewer->win, -1, wxT("")),1,wxEXPAND|wxADJUST_MINSIZE, 0);	
											 tmp = 1;
										}
										if (return_code)
										{
											 if (nodal_value_information)
											 {
													node_viewer_add_textctrl(node_viewer, field, (comp_no-1),nodal_value_type, 0);
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
																			wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxADJUST_MINSIZE, 0);
																}
														 }
														 node_viewer->grid_field->Add(new wxStaticText(node_viewer->win, -1, wxT(tmp_string)),1,
																wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxADJUST_MINSIZE, 0);
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
		*nodal_value_types,*temp_nodal_value_types;
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
			if (component_nodal_value_types=
				get_FE_node_field_component_nodal_value_types(node,field,i))
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
							/* add to list in numerical order */
							if (REALLOCATE(temp_nodal_value_types,nodal_value_types,
								enum FE_nodal_value_type,(*number_of_nodal_value_types)+1))
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
				if (temp_nodal_value_types)
					 DEALLOCATE(temp_nodal_value_types);
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
		/* get the node_field_viewer structure from the widget */
		 //		 XtVaGetValues(node_field_viewer_widget,XmNuserData,&node_field_viewer,NULL);
		if (node_viewer)
		{
			/* rebuild viewer widgets if nature of node or field changed */
// 			if ((!node)||(!(node_viewer->current_node))||
// 				(!equivalent_computed_fields_at_nodes(node,
// 					node_viewer->current_node))||
// 				(field != node_viewer->current_field)||
// 				(field&&(node_viewer->number_of_components !=
// 					Computed_field_get_number_of_components(field))))
// 			{
// 				setup_components = 1;
// 			}
// 			else
// 			{
// 				 setup_components = 0;
// 			}
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
									node_field_time_change_callback,
									(void *)node_viewer);
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
	int change_conditional_function,return_code;
	MANAGER_CONDITIONAL_FUNCTION(Computed_field)
		*choose_field_conditional_function;
	struct Computed_field *field;
	struct FE_node *template_node;

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
								 Computed_field_package_get_computed_field_manager(
										node_viewer->computed_field_package));
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
