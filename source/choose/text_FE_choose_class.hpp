/*******************************************************************************
FILE : text_FE_choose_class.hpp

LAST MODIFIED : 18 April 2007

DESCRIPTION :
Macros for implementing an text dialog control for choosing an object
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
#if !defined ( TEXT_FE_CHOOSE_CLASS_HPP)
#define TEXT_FE_CHOOSE_CLASS_HPP
extern "C" {
#include <stdio.h>
}
#include "wx/wx.h"
#include "general/callback_class.hpp"
// #include "choose/text_choose_class.hpp"
extern "C" {
#include "general/debug.h"
#include "finite_element/finite_element_region.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
}

template < class FE_object, class FE_region_method_class > class FE_object_text_chooser : public wxTextCtrl
/*****************************************************************************
LAST MODIFIED : 18 April 2007

DESCRIPTION :
============================================================================*/
{
private:
	 FE_object *current_object, *last_updated_object;
	 FE_region *fe_region; 
	 LIST_CONDITIONAL_FUNCTION(FE_node) *conditional_function;
	 void *conditional_function_user_data;
	 Callback_base<FE_object*> *update_callback;

public:
	 FE_object_text_chooser(wxWindow *parent, 
			FE_object *initial_object,	FE_region *fe_region,
			LIST_CONDITIONAL_FUNCTION(FE_node) *conditional_function,
			void *conditional_function_user_data) :
			wxTextCtrl(parent, /*id*/-1, "" ,wxPoint(0,0), wxSize(-1,-1),wxTE_PROCESS_ENTER),
			fe_region(fe_region), conditional_function(conditional_function),
			conditional_function_user_data(conditional_function_user_data)
/*******************************************************************************
LAST MODIFIED : 28 March 2007

DESCRIPTION :
=============================================================================*/
	 {
			 current_object = (FE_object *)NULL;
			 last_updated_object = (FE_object *)NULL;
			 update_callback = (Callback_base<FE_object*> *)NULL;
			 Connect(wxEVT_COMMAND_TEXT_ENTER,
					wxCommandEventHandler(FE_object_text_chooser::OnTextEnter));
			 FE_region_add_callback(fe_region,
					FE_object_text_chooser::object_change,
					(void *)this);
			 select_object(initial_object);
			 wxBoxSizer *sizer = new wxBoxSizer( wxHORIZONTAL );
			 sizer->Add(this,
					wxSizerFlags(1).Align(wxALIGN_CENTER).Expand());
			 parent->SetSizer(sizer);
			 typedef int (FE_object_text_chooser::*Member_function)(FE_object *);
			 Callback_base<FE_object*> *callback = 
					new Callback_member_callback<FE_object*, 
				 FE_object_text_chooser, Member_function>(
						this, &FE_object_text_chooser::text_chooser_callback);
			 set_callback(callback);
			 Show();
	 }

	~FE_object_text_chooser()
/*******************************************************************************
LAST MODIFIED : 18 April 2007

DESCRIPTION :
==============================================================================*/
	{
		 FE_region_remove_callback(fe_region,
				FE_object_text_chooser::object_change,
				this);
	}/* FE_object_text_chooser::~FE_object_text_chooser() */
	 
	 int text_chooser_callback(FE_object *object)
/*****************************************************************************
LAST MODIFIED : 18 April 2007

DESCRIPTION :
Called by the 
============================================================================*/
	{
		int return_code;
		
		if (update_callback)
		{
			/* now call the procedure with the user data */
			update_callback->callback_function(object);
		}
		return_code=1;
		
		return (return_code);
	} /* FE_object_chooser::get_callback */

	Callback_base<FE_object*> *get_callback()
/*****************************************************************************
LAST MODIFIED : 18 April 2007

DESCRIPTION :
Returns a pointer to the callback item.
============================================================================*/
	{
		return update_callback;
	} /* FE_object_chooser::get_callback */
	 
	 int set_callback(Callback_base<FE_object*> *new_callback)
/*****************************************************************************
LAST MODIFIED : 18 April 2007

DESCRIPTION :
Changes the callback item.
============================================================================*/
	 {
			update_callback = new_callback;
			return (1);
	 } /* FE_object_chooser::set_callback */

	FE_object *get_object()
/*****************************************************************************
LAST MODIFIED : 18 April 2007

DESCRIPTION :
Returns the currently chosen object.
============================================================================*/
	{
		 FE_object *new_object, *return_address; 
		 new_object = FE_region_method_class::string_to_object(fe_region,
				get_item());
		 select_object(new_object);
		 return_address = current_object;
		 
		 return (return_address); 
	} /* FE_object_chooser::get_object */

	int set_object(FE_object *new_object)
/*****************************************************************************
LAST MODIFIED : 18 April 2007

DESCRIPTION :
Changes the chosen object in the choose_object_widget.
============================================================================*/
	{
	int return_code; 

	last_updated_object=new_object;
	return_code = select_object(new_object);
	
	return (return_code);
	} /* FE_object_chooser::set_object */
	 
	int update()
/***************************************************************************** 
LAST MODIFIED : 28 January 2003 

DESCRIPTION : 
Tells CMGUI about the current values. Sends a pointer to the current object. 
Avoids sending repeated updates if the object address has not changed. 
============================================================================*/ 
{ 
	int return_code; 

		if (current_object != last_updated_object)
		{ 
#if defined (NEW_CODE)
			if (update_callback.procedure) 
			{ 
				/* now call the procedure with the user data and the position data */ 
				(update_callback.procedure)( 
					widget, update_callback.data, 
					current_object); 
			} 
#endif // defined (NEW_CODE)
			last_updated_object = current_object; 
		} 
		return_code=1; 

	return (return_code); 
} /* FE_object_chooser::update */

	int select_object(FE_object *new_object)
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

	ENTER(TEXT_CHOOSE_FROM_FE_REGION_SELECT_OBJECT(object_type));

	if (current_string = get_item()) 
	{ 
		 if (new_object && ((!(FE_region_method_class::FE_region_contains_object(
															fe_region, new_object)) || 
				(conditional_function && 
				!(conditional_function(new_object, 
							conditional_function_user_data))))))
		 { 
				display_message(ERROR_MESSAGE, 
					 "TEXT_CHOOSE_FROM_FE_REGION_SELECT_OBJECT(object_type).  Invalid object"); 
				new_object=(FE_object *)NULL; 
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
		 update();
		 return_code=1;
	}
	else
	{
		 return_code = 0;
	}
	
	LEAVE; 
	
	return (return_code); 
} /* TEXT_CHOOSE_FROM_FE_REGION_SELECT_OBJECT(object_type) */


static void object_change(struct FE_region *fe_region, struct FE_region_changes *changes, 
	 void *text_choose_object_void)
/***************************************************************************** 
LAST MODIFIED : 28 March 2003 

DESCRIPTION : 
Updates the chosen object and text field in response to messages. 
============================================================================*/
{ 
 	 typename FE_region_method_class::change_log_change_object_type change; 
	 FE_object_text_chooser *chooser;

	 if (chooser = static_cast<FE_object_text_chooser *>(text_choose_object_void))
	 {
			if (chooser->current_object)
			{
				 if (FE_region_method_class::change_log_query( 
								FE_region_method_class::get_object_changes(changes), 
								chooser->current_object, &change)) 
				 { 
						if (change & (FE_region_method_class::change_log_object_changed | 
									FE_region_method_class::change_log_object_removed)) 
						{ 
							 chooser->select_object((FE_object *)NULL); 
						} 
				 } 
			}
	 }
	 
} /* FE_object_chooser::object_change */

   int set_callback(Callback_base< FE_object > *callback_object)
	{
		update_callback = callback_object;
		return (1);
	}
	 char *get_item()
	{
		 return (const_cast<char *>(GetValue().c_str()));
	}

	int set_item(char *new_item)
	{
		unsigned int return_code;
		return_code = 0;
		
		if (new_item !=NULL)
		{
			 SetValue(new_item);
			 return_code = 1;
		}
		return (return_code);
	}

	void OnTextEnter(wxCommandEvent& Event)
	{
		if (update_callback)
		{
			 update_callback->callback_function(get_object());
		}
	}
};

#endif /*defined(TEXT_CHOOSE_FROM_FE_ELEMENT_H)*/
