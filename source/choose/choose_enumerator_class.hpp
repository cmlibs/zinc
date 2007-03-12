/*******************************************************************************
FILE : choose_enumerator_class.hpp

LAST MODIFIED : 10 March 2007

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
#if !defined (CHOOSE_ENUMERATOR_CLASS_H)
#define CHOOSE_ENUMERATOR_CLASS_H

template < class Enumerator > class wxEnumeratorChooser : public wxChoice
/*****************************************************************************
LAST MODIFIED : 8 February 2007

DESCRIPTION :
============================================================================*/
{
private:
	Callback_base< typename Enumerator::Enumerator_type > *callback;

public:
	wxEnumeratorChooser<Enumerator>(wxPanel *parent, 
		int number_of_items, 
		char **item_names, typename Enumerator::Enumerator_type current_value,
		User_interface *user_interface) :
		wxChoice(parent, /*id*/-1, wxPoint(0,0), wxSize(-1,-1))
	{
		build_main_menu(number_of_items, item_names, current_value);

		Connect(wxEVT_COMMAND_CHOICE_SELECTED,
			wxCommandEventHandler(wxEnumeratorChooser::OnChoiceSelected));

		wxBoxSizer *sizer = new wxBoxSizer( wxHORIZONTAL );
		sizer->Add(this,
			wxSizerFlags(1).Align(wxALIGN_CENTER).Expand());
		parent->SetSizer(sizer);

		Show();
	}

	void OnChoiceSelected(wxCommandEvent& Event)
	{
		if (callback)
		{
			callback->callback_function(get_item());
		}
   }
	
	typename Enumerator::Enumerator_type get_item()
	{
		return (static_cast<typename Enumerator::Enumerator_type>
			(GetSelection()));
	}

	int set_callback(Callback_base< typename Enumerator::Enumerator_type >
		*callback_object)
	{
		callback = callback_object;
		return (1);
	}

	int set_item(typename Enumerator::Enumerator_type new_value)
	{
		unsigned int return_code;
		
		SetSelection(new_value);

		// Could check to see that the value was actually set
		return_code = 1;

		return (return_code);
	}

	int build_main_menu(int number_of_items, 
		char **item_names, 
		typename Enumerator::Enumerator_type current_value)
	{
		int i;
		Clear();
		for (i = 0 ; i < number_of_items ; i++)
		{
			Append(item_names[i]);
		}
		SetSelection(current_value);
		return 1;
	}
};

template < class Enumerator > class Enumerator_chooser
/*****************************************************************************
LAST MODIFIED : 10 March 2007

DESCRIPTION :
============================================================================*/
{
private:
	Enumerator *enumerator;
	wxPanel *parent;
	typename Enumerator::Conditional_function *conditional_function;
	void *conditional_function_user_data;
	wxEnumeratorChooser<Enumerator> *chooser;
	Callback_base< typename Enumerator::Enumerator_type > *update_callback;
   int number_of_items;
   char **item_names;

public:
	Enumerator_chooser(wxPanel *parent,
		typename Enumerator::Enumerator_type current_value,
		typename Enumerator::Conditional_function *conditional_function,
		void *conditional_function_user_data,
		User_interface *user_interface) :
		enumerator(new Enumerator()), parent(parent),
		conditional_function(conditional_function),
		conditional_function_user_data(conditional_function_user_data)
/*****************************************************************************
LAST MODIFIED : 10 March 2007

DESCRIPTION :
============================================================================*/
	{
		chooser = (wxEnumeratorChooser<Enumerator> *)NULL;
		update_callback = (Callback_base< typename Enumerator::Enumerator_type > *)NULL;
		number_of_items = 0;
		item_names = (char **)NULL;
		if (build_items())
		{
			chooser = new wxEnumeratorChooser<Enumerator>
				(parent, number_of_items,
				item_names, current_value,
				user_interface);
			typedef int (Enumerator_chooser::*Member_function)(typename Enumerator::Enumerator_type);
			Callback_base<typename Enumerator::Enumerator_type> *callback = 
				new Callback_member_callback< typename Enumerator::Enumerator_type, 
				Enumerator_chooser, Member_function>(
				this, &Enumerator_chooser::chooser_callback);
				
			chooser->set_callback(callback);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Enumerator_chooser::Enumerator_chooser.   "
				" Could not get items");
		}

	} /* Enumerator_chooser::Enumerator_chooser */

	~Enumerator_chooser()
/*****************************************************************************
LAST MODIFIED : 8 February 2007

DESCRIPTION :
============================================================================*/
	{
		int i;
	
		if (chooser)
		{
			delete chooser;
		}
		delete enumerator;
		if (item_names)
		{
			for (i=0;i<number_of_items;i++)
			{
				DEALLOCATE(item_names[i]);
			}
			DEALLOCATE(item_names);
		}
	} /* Enumerator_chooser::~Enumerator_chooser() */

	int chooser_callback(typename Enumerator::Enumerator_type value)
/*****************************************************************************
LAST MODIFIED : 8 February 2007

DESCRIPTION :
Called by the 
============================================================================*/
	{
		int return_code;
		
		if (update_callback)
		{
			/* now call the procedure with the user data */
			update_callback->callback_function(value);
		}
		return_code=1;
		
		return (return_code);
	} /* Enumerator_chooser::get_callback */

	Callback_base< typename Enumerator::Enumerator_type > *get_callback()
/*****************************************************************************
LAST MODIFIED : 8 February 2007

DESCRIPTION :
Returns a pointer to the callback item.
============================================================================*/
	{
		return update_callback;
	} /* Enumerator_chooser::get_callback */

	int set_callback(Callback_base< typename Enumerator::Enumerator_type > *new_callback)
/*****************************************************************************
LAST MODIFIED : 8 February 2007

DESCRIPTION :
Changes the callback item.
============================================================================*/
	{
		update_callback = new_callback;
		return(1);
	} /* Enumerator_chooser::set_callback */

	typename Enumerator::Enumerator_type get_value()
/*****************************************************************************
LAST MODIFIED : 8 February 2007

DESCRIPTION :
Returns the currently chosen object.
============================================================================*/
	{
		return(chooser->get_item());
	} /* Enumerator_chooser::get_object */

	int set_value(typename Enumerator::Enumerator_type new_value)
/*****************************************************************************
LAST MODIFIED : 8 February 2007

DESCRIPTION :
Changes the chosen object in the choose_object_widget.
============================================================================*/
	{
		return(chooser->set_item(new_value));
	} /* Enumerator_chooser::set_object */

	int set_conditional_function(
		typename Enumerator::Conditional_function *conditional_function,
		void *conditional_function_user_data, typename Enumerator::Enumerator_type new_value)
/*****************************************************************************
LAST MODIFIED : 10 March 2007

DESCRIPTION :
Changes the conditional_function and user_data limiting the available
selection of objects. Also allows new_object to be set simultaneously.
============================================================================*/
	{
		int return_code;

		conditional_function = conditional_function;
		conditional_function_user_data = conditional_function_user_data;
		if (build_items())
		{
			return_code=chooser->build_main_menu(
				number_of_items, item_names, new_value);
		}
		else
		{
			return_code=0;
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Enumerator_chooser::set_conditional_function"
				"Could not update menu");
		}
		return (return_code);
	} /* Enumerator_chooser::set_conditional_function */

private:

	int is_item_in_chooser(typename Enumerator::Enumerator_type value)
/*****************************************************************************
LAST MODIFIED : 8 February 2007

DESCRIPTION :
============================================================================*/
	{
		int i, return_code;

		if ((value > 0) && (value <= number_of_items))
		{
			return_code = 1;
		}
		else
		{
			return_code=0;
		}

		return (return_code);
	} /* Enumerator_chooser::is_item_in_chooser */

	int build_items()
/*****************************************************************************
LAST MODIFIED : 8 February 2007

DESCRIPTION :
Updates the arrays of all the choosable objects and their names.
============================================================================*/
	{
		int i,return_code;

		if (item_names)
		{
			for (i=0;i<number_of_items;i++)
			{
				DEALLOCATE(item_names[i]);
			}
			DEALLOCATE(item_names);
		}

		item_names = enumerator->get_valid_strings(&number_of_items,
			conditional_function, conditional_function_user_data);
		return_code = 1;

		return (return_code);
	}  /* Enumerator_chooser::build_items */

}; /* template < class Managed_object > class Enumerator_chooser */

#endif /* !defined (CHOOSE_ENUMERATOR_CLASS_H) */
