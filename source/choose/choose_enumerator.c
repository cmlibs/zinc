/*******************************************************************************
FILE : choose_enumerator.c

LAST MODIFIED : 22 March 1999

DESCRIPTION :
Widgets for editing a FE_field_scalar object = scalar function of a field.
==============================================================================*/
#include <stdio.h>
#if defined (MOTIF)
#include <Xm/Xm.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#endif /* defined (MOTIF) */
#include "general/debug.h"
#include "choose/choose_enumerator.h"
#include "choose/choose_object.uid64"
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
struct Choose_enumerator_struct
/*******************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Contains information required by the choose_enumerator_widget.
==============================================================================*/
{
	char *current_enumerator_string;
	char **valid_strings;
	int number_of_valid_strings;
	Widget parent,widget,option,menu;
	struct Callback_data update_callback;
}; /* struct Choose_enumerator_struct */

#if defined (MOTIF)
static int choose_enumerator_hierarchy_open=0;
static MrmHierarchy choose_enumerator_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
DECLARE_DIALOG_IDENTIFY_FUNCTION(choose_enumerator,
	Choose_enumerator_struct,option)
DECLARE_DIALOG_IDENTIFY_FUNCTION(choose_enumerator,
	Choose_enumerator_struct,menu)

static int choose_enumerator_make_menu(
	struct Choose_enumerator_struct *choose_enumerator)
/*****************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Clears and fills the option menu with all the available enumerator strings.
============================================================================*/
{
	Arg args[2];
	char *enumerator_string;
	int i,num_children,return_code;
	Widget *child_list,*child_copy,temp_widget;
	XmString new_string;

	ENTER(choose_enumerator_make_menu);
	if (choose_enumerator&&choose_enumerator->valid_strings&&
		(0<choose_enumerator->number_of_valid_strings))
	{
		/* unmanage and destroy current menu items */
		XtVaGetValues(choose_enumerator->menu,XmNnumChildren,&num_children,
			XmNchildren,&child_list,NULL);
		if (0<num_children)
		{
			if (ALLOCATE(child_copy,Widget,num_children))
			{
				for (i=0;i<num_children;i++)
				{
					child_copy[i]=child_list[i];
				}
				for (i=0;i<num_children;i++)
				{
					XtUnmanageChild(child_copy[i]);
					XtDestroyWidget(child_copy[i]);
				}
				DEALLOCATE(child_copy);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"choose_enumerator_make_menu.  "
					"Could not allocate space for copy of menu items");
				return_code=0;
			}
		}
		else
		{
			return_code=1;
		}
		for (i=0;return_code&&(i<choose_enumerator->number_of_valid_strings);i++)
		{
			if (enumerator_string=choose_enumerator->valid_strings[i])
			{
				XtSetArg(args[0],XmNuserData,(XtPointer)enumerator_string);
				new_string=XmStringCreateSimple(enumerator_string);
				XtSetArg(args[1],XmNlabelString,(XtPointer)new_string);
				if (temp_widget=XmCreatePushButtonGadget(
					choose_enumerator->menu,enumerator_string,args,2))
				{
					XtManageChild(temp_widget);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"choose_enumerator_make_menu.  Could not create menu item");
					return_code=0;
				}
				XmStringFree(new_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"choose_enumerator_make_menu.  Missing enumerator string");
				return_code=0;
			}
		}
		if (return_code)
		{
			XtManageChild(choose_enumerator->option);
		}
		else
		{
			XtUnmanageChild(choose_enumerator->option);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_enumerator_make_menu.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* choose_enumerator_make_menu */

static int choose_enumerator_update(
	struct Choose_enumerator_struct *choose_enumerator)
/*******************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Tells client that the component has changed.
==============================================================================*/
{
	int return_code;

	ENTER(choose_enumerator_update);
	if (choose_enumerator)
	{
		if (choose_enumerator->update_callback.procedure)
		{
			(choose_enumerator->update_callback.procedure)(
				choose_enumerator->widget,choose_enumerator->update_callback.data,
				(void *)choose_enumerator->current_enumerator_string);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_enumerator_update.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* choose_enumerator_update */

static void choose_enumerator_destroy_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Callback for the choose_enumerator dialog: tidies up all memory allocation.
==============================================================================*/
{
	struct Choose_enumerator_struct *choose_enumerator;

	ENTER(choose_enumerator_destroy_CB);
	if (widget)
	{
		/* Get the pointer to the data for the choose_enumerator dialog */
		XtVaGetValues(widget,XmNuserData,&choose_enumerator,NULL);
		if (choose_enumerator)
		{
			if (choose_enumerator->valid_strings)
			{
				DEALLOCATE(choose_enumerator->valid_strings);
			}
			DEALLOCATE(choose_enumerator);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"choose_enumerator_destroy_CB.  Missing widget data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_enumerator_destroy_CB.  Missing widget");
	}
	LEAVE;
} /* choose_enumerator_destroy_CB */

static void choose_enumerator_menu_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*****************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Callback for the option menu - change of object.
============================================================================*/
{
	struct Choose_enumerator_struct *choose_enumerator;
	Widget menu_item_widget;

	ENTER(choose_enumerator_menu_CB);
	if (widget&&(choose_enumerator=
		(struct Choose_enumerator_struct *)client_data))
	{
		/* get the widget from the call data */
		if (menu_item_widget=((XmRowColumnCallbackStruct *)call_data)->widget)
		{
			/* Get the object this menu item represents and make it current */
			XtVaGetValues(menu_item_widget,XmNuserData,
				&(choose_enumerator->current_enumerator_string),NULL);
			/* inform the client of the change */
			choose_enumerator_update(choose_enumerator);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"choose_enumerator_menu_CB.  Could not find the activated menu item");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_enumerator_menu_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* choose_enumerator_menu_CB */

/*
Global functions
----------------
*/
Widget create_choose_enumerator_widget(Widget parent,char **valid_strings,
	int number_of_valid_strings,char *enumerator_string)
/*******************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Creates an editor for specifying a string out of the <valid_strings>, with the
<enumerator_string> chosen as the current_value. The string should be converted
to its appropriate type by a function like:
enum Enumerated_type Enumerated_type_from_string(char *string);
The chooser makes its own copy of the <valid_strings> array.
==============================================================================*/
{
	int i;
	MrmType choose_enumerator_dialog_class;
	struct Choose_enumerator_struct *choose_enumerator;
	static MrmRegisterArg callback_list[]=
	{
		{"choose_object_identify_option",(XtPointer)
			DIALOG_IDENTIFY(choose_enumerator,option)},
		{"choose_object_identify_menu",(XtPointer)
			DIALOG_IDENTIFY(choose_enumerator,menu)},
		{"choose_object_destroy_CB",(XtPointer)choose_enumerator_destroy_CB},
		{"choose_object_menu_CB",(XtPointer)choose_enumerator_menu_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"Choose_object_structure",(XtPointer)NULL}
	};
	Widget return_widget;

	ENTER(create_choose_enumerator_widget);
	return_widget=(Widget)NULL;
	if (parent&&valid_strings&&(0<number_of_valid_strings)&&enumerator_string)
	{
		if (MrmOpenHierarchy_base64_string(choose_object_uid64,
			&choose_enumerator_hierarchy,&choose_enumerator_hierarchy_open))
		{
			/* allocate memory, incl. copy of valid_strings */
			if (ALLOCATE(choose_enumerator,struct Choose_enumerator_struct,1)&&
				ALLOCATE(choose_enumerator->valid_strings,char *,
					number_of_valid_strings))
			{
				/* initialise the structure */
				for (i=0;i<number_of_valid_strings;i++)
				{
					choose_enumerator->valid_strings[i]=valid_strings[i];
				}
				choose_enumerator->number_of_valid_strings=number_of_valid_strings;
				choose_enumerator->current_enumerator_string=enumerator_string;
				choose_enumerator->parent=parent;
				choose_enumerator->widget=(Widget)NULL;
				choose_enumerator->option=(Widget)NULL;
				choose_enumerator->menu=(Widget)NULL;
				choose_enumerator->update_callback.procedure=
					(Callback_procedure *)NULL;
				choose_enumerator->update_callback.data=(void *)NULL;
				/* register the callbacks */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
					choose_enumerator_hierarchy,callback_list,
					XtNumber(callback_list)))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)choose_enumerator;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						choose_enumerator_hierarchy,identifier_list,
						XtNumber(identifier_list)))
					{
						/* fetch choose_object control widget */
						if (MrmSUCCESS==MrmFetchWidget(choose_enumerator_hierarchy,
							"choose_object_option",choose_enumerator->parent,
							&(choose_enumerator->widget),&choose_enumerator_dialog_class))
						{
							if (choose_enumerator_make_menu(choose_enumerator)&&
								choose_enumerator_set_string(choose_enumerator->widget,
									enumerator_string))
							{
								XtManageChild(choose_enumerator->widget);
								return_widget=choose_enumerator->widget;
							}
							else
							{
								XtDestroyWidget(choose_enumerator->widget);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_choose_enumerator_widget.  "
								"Could not fetch choose_enumerator dialog");
							DEALLOCATE(choose_enumerator);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_choose_enumerator_widget.  "
							"Could not register identifiers");
						DEALLOCATE(choose_enumerator);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_choose_enumerator_widget.  "
						"Could not register callbacks");
					DEALLOCATE(choose_enumerator);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_choose_enumerator_widget.  "
					"Could not allocate control window structure");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_choose_enumerator_widget.  "
				"Could not open hierarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_choose_enumerator_widget.  Invalid argument(s)");
	}
	LEAVE;

	return (return_widget);
} /* create_choose_enumerator_widget */

struct Callback_data *choose_enumerator_get_callback(
	Widget choose_enumerator_widget)
/*****************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Returns a pointer to the callback item of the choose_enumerator_widget.
============================================================================*/
{
	struct Callback_data *return_address;
	struct Choose_enumerator_struct *choose_enumerator;

	ENTER(choose_enumerator_get_callback);
	if (choose_enumerator_widget)
	{
		/* Get the pointer to the data for the choose_enumerator dialog */
		XtVaGetValues(choose_enumerator_widget,XmNuserData,
			&choose_enumerator,NULL);
		if (choose_enumerator)
		{
			return_address=&(choose_enumerator->update_callback);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"choose_enumerator_get_callback.  Missing widget data");
			return_address=(struct Callback_data *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_enumerator_get_callback.  Missing widget");
		return_address=(struct Callback_data *)NULL;
	}
	LEAVE;

	return (return_address);
} /* choose_enumerator_get_callback */

int choose_enumerator_set_callback(Widget choose_enumerator_widget,
	struct Callback_data *new_callback)
/*****************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Changes the callback item of the choose_enumerator_widget.
============================================================================*/
{
	int return_code;
	struct Choose_enumerator_struct *choose_enumerator;

	ENTER(choose_enumerator_set_callback);
	if (choose_enumerator_widget&&new_callback)
	{
		/* Get the pointer to the data for the choose_enumerator dialog */
		XtVaGetValues(choose_enumerator_widget,XmNuserData,
			&choose_enumerator,NULL);
		if (choose_enumerator)
		{
			choose_enumerator->update_callback.procedure=new_callback->procedure;
			choose_enumerator->update_callback.data=new_callback->data;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"choose_enumerator_set_callback.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_enumerator_set_callback.  Missing widget");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* choose_enumerator_set_callback */

char *choose_enumerator_get_string(Widget choose_enumerator_widget)
/*****************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Returns the current enumerator_string in use by the editor. Calling function
must not destroy or modify the returned static string.
============================================================================*/
{
	char *enumerator_string;
	struct Choose_enumerator_struct *choose_enumerator;

	ENTER(choose_enumerator_get_string);
	if (choose_enumerator_widget)
	{
		/* Get the pointer to the data for the choose_enumerator dialog */
		XtVaGetValues(choose_enumerator_widget,XmNuserData,
			&choose_enumerator,NULL);
		if (choose_enumerator)
		{
			enumerator_string=choose_enumerator->current_enumerator_string;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"choose_enumerator_get_string.  Missing widget data");
			enumerator_string=(char *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_enumerator_get_string.  Missing widget");
		enumerator_string=(char *)NULL;
	}
	LEAVE;

	return (enumerator_string);
} /* choose_enumerator_get_string */

int choose_enumerator_set_string(Widget choose_enumerator_widget,
	char *enumerator_string)
/*****************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Changes the enumerator_string in the choose_enumerator_widget.
============================================================================*/
{
	char *temp_string;
	int i,num_children,return_code;
	struct Choose_enumerator_struct *choose_enumerator;
	Widget *child_list,chosen_widget;

	ENTER(choose_enumerator_set_string);
	if (choose_enumerator_widget&&enumerator_string)
	{
		/* Get the pointer to the data for the choose_enumerator dialog */
		XtVaGetValues(choose_enumerator_widget,XmNuserData,
			&choose_enumerator,NULL);
		if (choose_enumerator)
		{
			/* get children of the menu so that one may be selected */
			XtVaGetValues(choose_enumerator->menu,XmNnumChildren,
				&num_children,XmNchildren,&child_list,NULL);
			chosen_widget=(Widget)NULL;
			for (i=0;i<num_children;i++)
			{
				XtVaGetValues(child_list[i],XmNuserData,&temp_string,NULL);
				if (enumerator_string==temp_string)
				{
					chosen_widget=child_list[i];
				}
			}
			if (chosen_widget)
			{
				XtVaSetValues(choose_enumerator->option,
					XmNmenuHistory,chosen_widget,NULL);
				choose_enumerator->current_enumerator_string=enumerator_string;
				XtManageChild(choose_enumerator->option);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"choose_enumerator_set_string.  Unknown enumerator_string");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"choose_enumerator_set_string.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_enumerator_set_string.  Missing widget");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* choose_enumerator_set_string */

int choose_enumerator_set_valid_strings(Widget choose_enumerator_widget,
	char **valid_strings,int number_of_valid_strings,char *enumerator_string)
/*****************************************************************************
LAST MODIFIED : 23 March 1999

DESCRIPTION :
Changes the list of <valid_strings> in the choose_enumerator_widget.
============================================================================*/
{
	char **temp_valid_strings;
	int i,return_code;
	struct Choose_enumerator_struct *choose_enumerator;
	Widget *child_list,chosen_widget;

	ENTER(choose_enumerator_set_valid_strings);
	if (choose_enumerator_widget&&valid_strings&&(0<number_of_valid_strings)&&
		enumerator_string)
	{
		/* Get the pointer to the data for the choose_enumerator dialog */
		XtVaGetValues(choose_enumerator_widget,XmNuserData,
			&choose_enumerator,NULL);
		if (choose_enumerator)
		{
			/* ensure the enumerator_string is in the valid_strings */
			return_code=0;
			for (i=0;i<number_of_valid_strings;i++)
			{
				if (enumerator_string==valid_strings[i])
				{
					return_code=1;
				}
			}
			/* ensure all valid_strings are non-NULL */
			for (i=0;i<number_of_valid_strings;i++)
			{
				if ((char *)NULL==valid_strings[i])
				{
					return_code=0;
				}
			}
			if (return_code)
			{
				if (ALLOCATE(temp_valid_strings,char *,number_of_valid_strings))
				{
					for (i=0;i<number_of_valid_strings;i++)
					{
						temp_valid_strings[i]=valid_strings[i];
					}
					DEALLOCATE(choose_enumerator->valid_strings);
					choose_enumerator->valid_strings=temp_valid_strings;
					choose_enumerator->number_of_valid_strings=number_of_valid_strings;
					choose_enumerator->current_enumerator_string=enumerator_string;
					return_code=choose_enumerator_make_menu(choose_enumerator)&&
						choose_enumerator_set_string(choose_enumerator_widget,
							enumerator_string);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"choose_enumerator_set_valid_strings.  Not enough memory");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"choose_enumerator_set_valid_strings.  Invalid string(s)");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"choose_enumerator_set_valid_strings.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_enumerator_set_valid_strings.  Missing widget");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* choose_enumerator_set_valid_strings */
