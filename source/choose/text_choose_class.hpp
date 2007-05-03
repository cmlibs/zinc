/*******************************************************************************
FILE : text_choose_class.hpp

LAST MODIFIED : 18 April 2007

DESCRIPTION :
Class for implementing an wxList dialog control for choosing an object
from FE.
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
#if !defined (TEXT_CHOOSE_CLASS_H)
#define TEXT_CHOOSE_CLASS_H

#include "general/callback_class.hpp"
#include "text_FE_choose_class.hpp"
extern "C" {
#include "finite_element/finite_element_region.h"
}
template < class Object , class FE_region_method_class > class wxTextChooser : public wxTextCtrl
/*****************************************************************************
LAST MODIFIED : 18 April 2007

DESCRIPTION :
============================================================================*/
{
private:
	Callback_base< Object > *callback;
	 FE_region *fe_region;


public:
   wxTextChooser<Object, FE_region_method_class >(wxPanel *parent, FE_region *fe_region, Object initial_object, Object current_object,LIST_CONDITIONAL_FUNCTION(FE_node) *conditional_function, void *conditional_function_user_data) : wxTextCtrl(parent, /*id*/-1, "" ,wxPoint(0,0), wxSize(-1,-1),wxTE_PROCESS_ENTER), fe_region(fe_region)
   {
			Connect(wxEVT_COMMAND_TEXT_ENTER ,
				 wxCommandEventHandler(wxTextChooser::OnTextEnter));
// 			FE_region_add_callback(fe_region,
// 				 wxTextChooser::object_change,
// 				 (void *)this);
			select_object(initial_object,current_object, fe_region, conditional_function,conditional_function_user_data);
			wxBoxSizer *sizer = new wxBoxSizer( wxHORIZONTAL );
			sizer->Add(this,
				 wxSizerFlags(1).Align(wxALIGN_CENTER).Expand());
			parent->SetSizer(sizer);
// 			new_initial_object = initial_object;
// 			new_current_object = current_object;
// 			new_fe_region = fe_region;
// 			new_conditional_function = conditional_function;
// 			new_conditional_function_user_data = conditional_function_user_data;
			Show();
	 }

	 int select_object(Object new_object, Object current_object,FE_region *fe_region,LIST_CONDITIONAL_FUNCTION(FE_node) *conditional_function, void *conditional_function_user_data)
/***************************************************************************** 
LAST MODIFIED : 30 April 2003 

DESCRIPTION : 
Makes sure the <new_object> is valid for this text chooser, then calls an 
update in case it has changed, and writes the new object string in the widget. 
============================================================================*/
	{ 
	const char *current_string; 
	static char *null_object_name="<NONE>"; 
	char *object_name; 
	int return_code; 

	if (current_string = const_cast<char *>(GetValue().c_str()))
	{ 
		 if (new_object && ((!(FE_region_method_class::FE_region_contains_object(
															fe_region, new_object)) || 
							(conditional_function && 
								 !(conditional_function(new_object, 
											 conditional_function_user_data))))))
		 { 
				display_message(ERROR_MESSAGE, 
					 "TEXT_CHOOSE_FROM_FE_REGION_SELECT_OBJECT(object_type).  Invalid object"); 
				new_object=(Object )NULL; 
		 } 
		 if (new_object) 
		 { 
 				current_object=new_object;
		 } 
		 else 
		 { 
				if (!current_object) 
				{ 
					 current_object= 
							FE_region_method_class::get_first_object_that(
								 fe_region, 
								 conditional_function, 
								 conditional_function_user_data); 
				}
		 }
		 /* write out the current_object */ 
		 if (current_object) 
		 { 
				if (FE_region_method_class::FE_object_to_string
							(current_object,&object_name)) 
				{ 
					 if (strcmp(object_name,current_string)) 
					 { 
							set_item(object_name); 
					 } 
					 DEALLOCATE(object_name);
				} 
		 } 
		 else 
		 { 
				if (strcmp(null_object_name,current_string)) 
				{ 
					 set_item(null_object_name); 
				} 
		 } 
		 /* inform the client of any change */
		 //		 update();
		 return_code=1;
	}
	else
	{
		 return_code = 0;
	}
 
	return (return_code); 
} /* TEXT_CHOOSE_FROM_FE_REGION_SELECT_OBJECT(object_type) */

	void OnTextEnter(wxCommandEvent& Event)
	{
		if (callback)
		{
			 callback->callback_function(get_object());
		}
	}

	 char *get_item()
	{
		 return (const_cast<char *>(GetValue().c_str()));
	}

	 Object get_object()
/*****************************************************************************
LAST MODIFIED : 18 April 2007

DESCRIPTION :
Returns the currently chosen object.
============================================================================*/
	{
		 Object new_object,  return_address; 
 		 new_object = FE_region_method_class::string_to_object(fe_region,
 				get_item());

		 return_address = new_object;		 
		 return (return_address); 
	} /* FE_object_chooser::get_object */

   int set_callback(Callback_base< Object > *callback_object)
	{
		callback = callback_object;
		return (1);
	}

	int set_item(char *new_item)
	{
		unsigned int return_code;
		return_code = 0;
		
		if (new_item !=NULL)
		{
			 SetValue(new_item);
		}

		return (return_code);
	}
	

};
#endif /* !defined (TEXT_CHOOSE_CLASS_H) */
