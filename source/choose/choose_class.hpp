/*******************************************************************************
FILE : choose_class.hpp

LAST MODIFIED : 9 February 2007

DESCRIPTION :
Class for implementing an wxList dialog control for choosing an object
from its manager (subject to an optional conditional function). Handles manager
messages to keep the menu up-to-date.
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
#if !defined (CHOOSE_CLASS_H)
#define CHOOSE_CLASS_H

#include "general/callback_class.hpp"

template < class Object > class wxChooser : public wxChoice
/*****************************************************************************
LAST MODIFIED : 8 February 2007

DESCRIPTION :
============================================================================*/
{
private:
	Callback_base< Object > *callback;

public:
	wxChooser<Object>(wxPanel *parent, 
		int number_of_items, Object *items, 
		char **item_names, Object current_object,
		User_interface *user_interface) :
		wxChoice(parent, /*id*/-1, wxPoint(0,0), wxSize(-1,-1))
	{
		build_main_menu(number_of_items, items, item_names, current_object);

		Connect(wxEVT_COMMAND_CHOICE_SELECTED,
			wxCommandEventHandler(wxChooser::OnChoiceSelected));

		wxBoxSizer *sizer = new wxBoxSizer( wxHORIZONTAL );
		sizer->Add(this,
			wxSizerFlags(1).Align(wxALIGN_CENTER).Expand());
		parent->SetSizer(sizer);

		Show();
	}

// 	 ~wxChooser()
// 	 {
// 	 }

	void OnChoiceSelected(wxCommandEvent& Event)
	{
		if (callback)
		{
			callback->callback_function(get_item());
		}
   }
	
	Object get_item()
	{
		return (static_cast<Object>(GetClientData(GetSelection())));
	}

	int set_callback(Callback_base< Object > *callback_object)
	{
		callback = callback_object;
		return (1);
	}

	int set_item(Object new_item)
	{
		unsigned int i, return_code;
		return_code = 0;
		for (i = 0 ; !return_code && (i < GetCount()) ; i++)
		{
			if (new_item == GetClientData(i))
			{
				SetSelection(i);
				return_code = 1;
			}
		}
		return (return_code);
	}

	 int get_number_of_item()
	{
		 unsigned int return_code;
		 return_code = 0;
		 return_code = GetCount();
		return (return_code);
	}

	int build_main_menu(int number_of_items, 
		Object *items, char **item_names, 
		Object current_item)
	{
		int current_item_index, i;
		Clear();
		current_item_index = 0;
		for (i = 0 ; i < number_of_items ; i++)
		{
			Append(item_names[i], items[i]);
			if (current_item == items[i])
			{
				current_item_index = i;
			}
		}
		SetSelection(current_item_index);
		return 1;
	}
};

#endif /* !defined (CHOOSE_CLASS_H) */
