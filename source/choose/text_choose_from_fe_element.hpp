/*******************************************************************************
FILE : text_choose_from_fe_element.hpp

LAST MODIFIED : 28 March 2007

DESCRIPTION :
Macros for implementing an option menu dialog control for choosing an object
from its manager. Handles manager messages to keep the menu up-to-date.
Calls the client-specified callback routine if a different object is chosen.
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
#if !defined (TEXT_CHOOSE_FROM_FE_ELEMENT_H)
#define TEXT_CHOOSE_FROM_FE_ELEMENT_H


extern "C" {
#include <stdio.h>
}
#include "wx/wx.h"
#include "general/callback_class.hpp"
extern "C" {
#include "general/debug.h"
#include "choose/text_choose_from_fe_region.h"
#include "finite_element/finite_element_region.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
}

struct FE_element;

class wxFeElementTextChooser : public wxTextCtrl
{
private:
	struct FE_element *current_object,*last_updated_object; 
	struct FE_region *fe_region; 
	LIST_CONDITIONAL_FUNCTION(FE_element) *conditional_function; 
	void *conditional_function_user_data; 
	Callback_base<FE_element*> *callback;

public:
	 wxFeElementTextChooser(wxWindow *parent, 
		 FE_element *initial_object,	FE_region *fe_region,
		 LIST_CONDITIONAL_FUNCTION(FE_element) *conditional_function,
		 void *conditional_function_user_data) :
			wxTextCtrl(parent, /*id*/-1, "" ,wxPoint(0,0), wxSize(-1,-1),wxTE_PROCESS_ENTER),
			fe_region(fe_region), conditional_function(conditional_function),
			conditional_function_user_data(conditional_function_user_data)
/*******************************************************************************
LAST MODIFIED : 28 March 2007

DESCRIPTION :
=============================================================================*/
	{
		 current_object = (FE_element *)NULL;
		 last_updated_object = (FE_element *)NULL;
		 callback = (Callback_base<FE_element*> *)NULL;
		 Connect(wxEVT_COMMAND_TEXT_ENTER,
				wxCommandEventHandler(wxFeElementTextChooser::OnTextEnter));
		 FE_region_add_callback(fe_region,
				wxFeElementTextChooser::object_change,
				(void *)this);
		 select_object(initial_object);
		 wxBoxSizer *sizer = new wxBoxSizer( wxHORIZONTAL );
		 sizer->Add(this,
				wxSizerFlags(1).Align(wxALIGN_CENTER).Expand());
		 parent->SetSizer(sizer);

	Show();
	}

	~wxFeElementTextChooser()
/*******************************************************************************
LAST MODIFIED : 9 January 2003

DESCRIPTION :
==============================================================================*/
	 {
		 FE_region_remove_callback(fe_region,
				wxFeElementTextChooser::object_change,
				(void *)this);
	 }

	int set_callback(Callback_base<FE_element*> *callback_object)
	{
		callback = callback_object;
		return (1);
	}

/*
Module variables
----------------
*/

	int update()
/***************************************************************************** 
LAST MODIFIED : 28 January 2003 

DESCRIPTION : 
Tells CMGUI about the current values. Sends a pointer to the current object. 
Avoids sending repeated updates if the object address has not changed. 
============================================================================*/ 
{ 
	int return_code; 

	ENTER(TEXT_CHOOSE_FROM_FE_REGION_UPDATE(object_type));
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
	LEAVE; 

	return (return_code); 
} /* TEXT_CHOOSE_FROM_FE_REGION_UPDATE(object_type) */

	int select_object(struct FE_element *new_object)
/***************************************************************************** 
LAST MODIFIED : 30 April 2003 

DESCRIPTION : 
Makes sure the <new_object> is valid for this text chooser, then calls an 
update in case it has changed, and writes the new object string in the widget. 
============================================================================*/ \
	{ 
	const char *current_string; 
	static char *null_object_name="<NONE>"; 
	char *object_name; 
	int return_code; 

	ENTER(TEXT_CHOOSE_FROM_FE_REGION_SELECT_OBJECT(object_type));

	if (current_string = const_cast<char *>(GetValue().c_str())) 
	{ 
		 if (new_object && ((!FE_region_contains_FE_element( 
			 fe_region, new_object)) || 
				(conditional_function && 
				!(conditional_function(new_object, 
					conditional_function_user_data))))) 
		 { 
				display_message(ERROR_MESSAGE, 
					 "TEXT_CHOOSE_FROM_FE_REGION_SELECT_OBJECT(object_type).  Invalid object"); 
				new_object=(struct FE_element *)NULL; 
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
							FE_region_get_first_FE_element_that(
								 fe_region, 
								 conditional_function, 
								 conditional_function_user_data); 
				}
		 }
		 /* write out the current_object */ 
		 if (current_object) 
		 { 
				if (FE_element_to_element_string( 
							 current_object,&object_name)) 
				{ 
					 if (strcmp(object_name,current_string)) 
					 { 
							SetValue(object_name); 
					 } 
					 DEALLOCATE(object_name);
				} 
		 } 
		 else 
		 { 
				if (strcmp(null_object_name,current_string)) 
				{ 
					 SetValue(null_object_name); 
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
	enum CHANGE_LOG_CHANGE(FE_element) change; 
	wxFeElementTextChooser *chooser;
	 
	ENTER(TEXT_CHOOSE_FROM_FE_REGION_GLOBAL_OBJECT_CHANGE(FE_element));

	if (chooser = static_cast<wxFeElementTextChooser *>(text_choose_object_void))
	{
		if (chooser->current_object) 
		{ 
			if (CHANGE_LOG_QUERY(FE_element)( 
				FE_region_changes_get_FE_element_changes(changes), 
				chooser->current_object, &change)) 
				{ 
						if (change & (CHANGE_LOG_OBJECT_CHANGED(FE_element) | 
										  CHANGE_LOG_OBJECT_REMOVED(FE_element))) 
					{ 
						chooser->select_object((struct FE_element *)NULL); 
					} 
				} 
		}
	}

	LEAVE; 
} /* TEXT_CHOOSE_FROM_FE_REGION_GLOBAL_OBJECT_CHANGE(FE_element) */

int GetCallback(struct FE_element *new_object)
/***************************************************************************** 
LAST MODIFIED : 28 January 2003 

DESCRIPTION : 
Returns a pointer to the callback item of the text_choose_object_widget. 
============================================================================*/ 
{ 
	//	struct Callback_data *return_address; 

	ENTER(TEXT_CHOOSE_FROM_FE_REGION_GET_CALLBACK(FE_element)); 
	
		/* Get the pointer to the data for the text_choose_object dialog */ 
	//		text_choose_object = wxFeElementTextChooser->GetValue();
#if defined (NEW_CODE)
		if (text_choose_object) 
		{ 
			//			return_address=&(update_callback); 
		} 
		else 
		{ 
// 			display_message(ERROR_MESSAGE, 
// 				"TEXT_CHOOSE_FROM_FE_REGION_GET_CALLBACK(" #object_type 
// 				").  Missing widget data"); 
			//		return_address=(struct FE_element *)NULL; 
		} 
#endif
	LEAVE; 

	return 1; 
} /* TEXT_CHOOSE_FROM_FE_REGION_GET_CALLBACK(object_type) */

int SetCallback(struct FE_element *new_object)
/***************************************************************************** \
LAST MODIFIED : 28 January 2003 \
\
DESCRIPTION : \
Changes the callback item of the text_choose_object_widget. \
============================================================================*/
{ 
	int return_code; 
	ENTER(TEXT_CHOOSE_FROM_FE_REGION_SET_CALLBACK(object_type)); 
	
	/* Get the pointer to the data for the text_choose_object dialog */ 
	//	text_choose_object = wxFeElementTextChooser->GetValue();
#if defined (NEW_CODE)
	if (text_choose_object) 
	{ 
		update_callback.procedure= 
			new_callback->procedure; 
		update_callback.data=new_callback->data; 
		return_code=1;
	} 
	else 
		{ 
			display_message(ERROR_MESSAGE, 
			 "TEXT_CHOOSE_FROM_FE_REGION_SET_CALLBACK(" #object_type 
				").  Missing widget data"); 
			return_code=0; 
		} 
	} 
#endif

	LEAVE; 

	return (return_code); 
} /* TEXT_CHOOSE_FROM_FE_REGION_SET_CALLBACK(object_type) */


	struct FE_element *get_object()
/***************************************************************************** \
LAST MODIFIED : 28 January 2003 \
\
DESCRIPTION : \
Returns the currently chosen object in the text_choose_object_widget. \
============================================================================*/ 
{ 
	 struct FE_element *new_object,*return_address; 
	 ENTER(TEXT_CHOOSE_FROM_FE_REGION_GET_OBJECT(object_type)); 
	 new_object = FE_region_element_string_to_FE_element(fe_region,
			const_cast<char *>(GetValue().c_str()));
	 select_object(new_object);
	 return_address = current_object;
	LEAVE; 

	return (return_address); 
} /* TEXT_CHOOSE_FROM_FE_REGION_GET_OBJECT(object_type) */

int set_object(struct FE_element *new_object)
/***************************************************************************** 
LAST MODIFIED : 28 January 2003 \
\
DESCRIPTION : \
Changes the chosen object in the text_choose_object_widget. \
============================================================================*/ 
{ 
	int return_code; 

	ENTER(TEXT_CHOOSE_FROM_FE_REGION_SET_OBJECT(object_type)); 
	last_updated_object=new_object;
	return_code = select_object(new_object);
	
	LEAVE;
	return (return_code);

} /* TEXT_CHOOSE_FROM_FE_REGION_SET_OBJECT(object_type) */

void OnTextEnter(wxCommandEvent& Event)
{
	notify_callback();
}

int notify_callback()
{
	if (callback)
	{
		callback->callback_function(get_object());
	}
	return (1);
}

};
#endif /*defined(TEXT_CHOOSE_FROM_FE_ELEMENT_H)*/
