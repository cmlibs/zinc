/*******************************************************************************
FILE : choose_list_class.hpp

LAST MODIFIED : 12 March 2007

DESCRIPTION :
Class for implementing an wxList dialog control for choosing an object
from its list (subject to an optional conditional function). Handles list
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
#if !defined (CHOOSE_LIST_CLASS_H)
#define CHOOSE_LIST_CLASS_H

#include "choose/choose_class.hpp"

template < class Object, class List > class List_chooser
/*****************************************************************************
LAST MODIFIED : 12 March 2007

DESCRIPTION :
============================================================================*/
{
private:
	List *list;
	wxPanel *parent;
	typename List::List_conditional_function *conditional_function;
	void *conditional_function_user_data;
	void *list_callback_id;
	wxChooser<Object*> *chooser;
	Callback_base<Object *> *update_callback;
   int number_of_items;
   Object **items;
   char **item_names;

public:
	List_chooser(wxPanel *parent,
		Object *current_object,
		typename List::List_type *struct_list,
		typename List::List_conditional_function *conditional_function,
	    void *conditional_function_user_data,
		User_interface *user_interface) :
		list(new List(struct_list)), parent(parent),
		conditional_function(conditional_function),
		conditional_function_user_data(conditional_function_user_data)
/*****************************************************************************
LAST MODIFIED : 8 February 2007

DESCRIPTION :
============================================================================*/
	{
		chooser = (wxChooser<Object*> *)NULL;
		update_callback = (Callback_base<Object*> *)NULL;
		number_of_items = 0;
		items = (Object **)NULL;
		item_names = (char **)NULL;
		if (build_items())
		{
			chooser = new wxChooser<Object*>
				(parent, number_of_items,
				items, item_names, current_object,
				user_interface);
			typedef int (List_chooser::*Member_function)(Object *);
			Callback_base<Object*> *callback = 
			   new Callback_member_callback<Object*, 
				List_chooser, Member_function>(
				this, &List_chooser::chooser_callback);
				
			chooser->set_callback(callback);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"List_chooser::List_chooser.   "
				" Could not get items");
		}

	} /* List_chooser::List_chooser */

	~List_chooser()
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
		delete list;
		if (items)
		{
			DEALLOCATE(items);
		}
		if (item_names)
		{
			for (i=0;i<number_of_items;i++)
			{
				DEALLOCATE(item_names[i]);
			}
			DEALLOCATE(item_names);
		}
	} /* List_chooser::~List_chooser() */

	int chooser_callback(Object *object)
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
			update_callback->callback_function(object);
		}
		return_code=1;
		
		return (return_code);
	} /* List_chooser::get_callback */

	Callback_base<Object*> *get_callback()
/*****************************************************************************
LAST MODIFIED : 8 February 2007

DESCRIPTION :
Returns a pointer to the callback item.
============================================================================*/
	{
		return update_callback;
	} /* List_chooser::get_callback */

	int set_callback(Callback_base<Object*> *new_callback)
/*****************************************************************************
LAST MODIFIED : 8 February 2007

DESCRIPTION :
Changes the callback item.
============================================================================*/
	{
		update_callback = new_callback;
		return(1);
	} /* List_chooser::set_callback */

	Object *get_object()
/*****************************************************************************
LAST MODIFIED : 8 February 2007

DESCRIPTION :
Returns the currently chosen object.
============================================================================*/
	{
		return(chooser->get_item());
	} /* List_chooser::get_object */

	int set_object(Object *new_object)
/*****************************************************************************
LAST MODIFIED : 8 February 2007

DESCRIPTION :
Changes the chosen object in the choose_object_widget.
============================================================================*/
	{
		return(chooser->set_item(new_object));
	} /* List_chooser::set_object */

	int set_conditional_function(
		typename List::List_conditional_function *conditional_function,
		void *conditional_function_user_data, Object *new_object)
/*****************************************************************************
LAST MODIFIED : 9 June 2000

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
				number_of_items, items, item_names, new_object);
		}
		else
		{
			return_code=0;
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"List_chooser::set_conditional_function"
				"Could not update menu");
		}
		return (return_code);
	} /* List_chooser::set_conditional_function */

private:

	int is_item_in_chooser(Object *object)
/*****************************************************************************
LAST MODIFIED : 8 February 2007

DESCRIPTION :
============================================================================*/
	{
		int i, return_code;

		if (object)
		{
			return_code = 0;
			for (i = 0 ; !return_code && (i < number_of_items) ; i++)
			{
				if (object == items[i])
				{
					return_code=1;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"List_chooser::is_item_in_chooser.  Invalid argument(s)");
			return_code=0;
		}

		return (return_code);
	} /* List_chooser::is_item_in_chooser */

	static int add_object_to_list(Object *object, void *chooser_object_void)
/*****************************************************************************
LAST MODIFIED : 8 February 2007

DESCRIPTION :
A list iterator which adds each object to the chooser.
============================================================================*/
	{
		int return_code;
		List_chooser* chooser_object;

		return_code = 1;
		if (chooser_object = (List_chooser*)chooser_object_void)
		{
			if (!(chooser_object->conditional_function) || 
				(chooser_object->conditional_function)(object,
					chooser_object->conditional_function_user_data))
			{
				if (chooser_object->list->get_object_name(object,
						chooser_object->item_names + chooser_object->number_of_items))
				{
					chooser_object->items[chooser_object->number_of_items] = object;
					chooser_object->number_of_items++;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"List_chooser::add_object_to_list.  "
						"Could not get name of object");
					return_code=0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"List_chooser::add_object_to_list.  "
				"Invalid object");
			return_code = 0;
		}
		return (return_code);
	} /* List_chooser::add_object_to_list */

	int build_items()
/*****************************************************************************
LAST MODIFIED : 8 February 2007

DESCRIPTION :
Updates the arrays of all the choosable objects and their names.
============================================================================*/
	{
		char **new_item_names;
		int i,max_number_of_objects,return_code;
		Object **new_items;

		return_code=0;

		max_number_of_objects= list->number_in_list();
		if ((0==max_number_of_objects) ||
			(ALLOCATE(new_items,Object *,max_number_of_objects) &&
				ALLOCATE(new_item_names,char *,max_number_of_objects)))
		{
			if (items)
			{
				DEALLOCATE(items);
			}
			if (item_names)
			{
				for (i=0;i<number_of_items;i++)
				{
					DEALLOCATE(item_names[i]);
				}
				DEALLOCATE(item_names);
			}
			items = new_items;
			item_names = new_item_names;
			number_of_items = 0;
			list->for_each_object_in_list(
				add_object_to_list, this);
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
		return (return_code);
	}  /* List_chooser::build_items */

}; /* template < class Object > class List_chooser */


#endif /* !defined (CHOOSE_LIST_CLASS_H) */
