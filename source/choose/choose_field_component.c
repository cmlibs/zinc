/*******************************************************************************
FILE : choose_field_component.c

LAST MODIFIED : 16 November 1998

DESCRIPTION :
Specialized version of chooser widget that allows component of an FE_field to
be selected from an option menu.
==============================================================================*/
#include <stdio.h>
#if defined (MOTIF)
#include <Xm/Xm.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#endif /* defined (MOTIF) */
#include "general/debug.h"
#include "choose/choose_field_component.h"
#include "choose/choose_object.uidh"
#include "finite_element/finite_element.h"
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
struct Choose_field_component_struct
/*******************************************************************************
LAST MODIFIED : 29 September 1997

DESCRIPTION :
Contains information required by the choose_field_component_widget.
==============================================================================*/
{
	int current_component_no;
	struct FE_field *current_field;
	Widget widget_parent,widget,option,menu;
	struct Callback_data update_callback;
}; /* struct Choose_field_component_struct */

#if defined (MOTIF)
static int choose_field_component_hierarchy_open=0;
static MrmHierarchy choose_field_component_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
DECLARE_DIALOG_IDENTIFY_FUNCTION(choose_field_component,
	Choose_field_component_struct,option)
DECLARE_DIALOG_IDENTIFY_FUNCTION(choose_field_component,
	Choose_field_component_struct,menu)

static int choose_field_component_update(
	struct Choose_field_component_struct *choose_field_component)
/*******************************************************************************
LAST MODIFIED : 29 September 1997

DESCRIPTION :
Tells client that the component has changed.
==============================================================================*/
{
	int return_code;

	ENTER(choose_field_component_update);
	if (choose_field_component)
	{
		if (choose_field_component->update_callback.procedure)
		{
			(choose_field_component->update_callback.procedure)(
				choose_field_component->widget,
				choose_field_component->update_callback.data,NULL);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_field_component_update.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* choose_field_component_update */

static void choose_field_component_destroy_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 23 July 1997

DESCRIPTION :
Callback for the choose_field_component dialog: tidies up all memory allocation.
==============================================================================*/
{
	struct Choose_field_component_struct *choose_field_component;

	ENTER(choose_field_component_destroy_CB);
	if (widget)
	{
		/* Get the pointer to the data for the choose_field_component dialog */
		XtVaGetValues(widget,XmNuserData,&choose_field_component,NULL);
		if (choose_field_component)
		{
			DEALLOCATE(choose_field_component);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"choose_field_component_destroy_CB.  Missing widget data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_field_component_destroy_CB.  Missing widget");
	}
	LEAVE;
} /* choose_field_component_destroy_CB */

static int choose_field_component_make_menu(
	struct Choose_field_component_struct *choose_field_component)
/*******************************************************************************
LAST MODIFIED : 16 November 1998

DESCRIPTION :
Destroys the menu widget and all items in it, refills the menu with all the
components in the field.
==============================================================================*/
{
	char *field_component_name;
	static char *temp_menu_item="<NONE>";
	int return_code,i,num_children,number_of_components,net_children;
	Widget *child_list,*child_copy,temp_widget;
	Arg args[2];
	struct FE_field *field;
	struct FE_field_component field_component;
	XmString new_string;

	ENTER(choose_field_component_make_menu);
	/* check arguments */
	if (choose_field_component&&choose_field_component->option&&
		choose_field_component->menu)
	{
		/* unmanage and destroy current menu items */
		XtVaGetValues(choose_field_component->menu,XmNnumChildren,&num_children,
			XmNchildren,&child_list,NULL);
		net_children=num_children;
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
					net_children--;
				}
				DEALLOCATE(child_copy);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"choose_field_component_make_menu.  "
					"Could not allocate space for copy of menu items");
				return_code=0;
			}
		}
		else
		{
			return_code=1;
		}
		if (return_code)
		{
			if (field=choose_field_component->current_field)
			{
				/* insert menu items for each component */
				number_of_components=get_FE_field_number_of_components(field);
				field_component.field=field;
				for (i=0;i<number_of_components;i++)
				{
					field_component.number=i;
					XtSetArg(args[0],XmNuserData,(XtPointer)i);
					if (GET_NAME(FE_field_component)(&field_component,
						&field_component_name))
					{
						new_string=XmStringCreateSimple(field_component_name);
						XtSetArg(args[1],XmNlabelString,(XtPointer)new_string);
						if (temp_widget=XmCreatePushButtonGadget(
							choose_field_component->menu,field_component_name,args,2))
						{
/*						XtVaSetValues(temp_widget,XmNlabelString,new_string,NULL);*/
							if (i==choose_field_component->current_component_no)
							{
								XtVaSetValues(choose_field_component->option,
									XmNmenuHistory,temp_widget,NULL);
							}
							net_children++;
							XtManageChild(temp_widget);
						}
						else
						{
							display_message(ERROR_MESSAGE,
							"choose_field_component_make_menu.  Could not create menu item");
							return_code=0;
						}
						DEALLOCATE(field_component_name);
						XmStringFree(new_string);
					}
					else
					{
						display_message(ERROR_MESSAGE,
			"choose_field_component_make_menu.  Could not get field component name");
						return_code=0;
					}
				}
			}
			else
			{
				/* add a single dummy menu item */
				XtSetArg(args[0],XmNuserData,(XtPointer)0);
				new_string=XmStringCreateSimple(temp_menu_item);
				XtSetArg(args[1],XmNlabelString,(XtPointer)new_string);
				if (temp_widget=XmCreatePushButtonGadget(
					choose_field_component->menu,temp_menu_item,args,2))
				{
					XtVaSetValues(choose_field_component->option,
						XmNmenuHistory,temp_widget,NULL);
					net_children++;
					XtManageChild(temp_widget);
				}
				XmStringFree(new_string);
			}
			/* only manage menu if it has children */
			if (0<net_children)
			{
				XtManageChild(choose_field_component->option);
				return_code=1;
			}
			else
			{
				XtUnmanageChild(choose_field_component->option);
			}
		}
		else
		{
			XtUnmanageChild(choose_field_component->option);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_field_component_make_menu.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* choose_field_component_make_menu */

static void choose_field_component_menu_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 16 November 1998

DESCRIPTION :
Callback for the option menu - change of scalar component.
==============================================================================*/
{
	struct Choose_field_component_struct *choose_field_component;
	struct FE_field *field;
	int new_component_no;
	Widget menu_item_widget;

	ENTER(choose_field_component_menu_CB);
	if (widget&&(choose_field_component=
		(struct Choose_field_component_struct *)client_data)&&
		(field=choose_field_component->current_field))
	{
		/* get the widget from the call data */
		if (menu_item_widget=((XmRowColumnCallbackStruct *)call_data)->widget)
		{
			/* Get the material this menu item represents and make it current */
			XtVaGetValues(menu_item_widget,XmNuserData,&new_component_no,NULL);
			if ((0<=new_component_no)&&(new_component_no<
				get_FE_field_number_of_components(field)))
			{
				if (new_component_no != choose_field_component->current_component_no)
				{
					choose_field_component->current_component_no=new_component_no;
					/* inform the client of the change */
					choose_field_component_update(choose_field_component);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"choose_field_component_menu_CB.  "
					"Invalid component number");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"choose_field_component_menu_CB.  "
				"Could not find the activated menu item");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_field_component_menu_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* choose_field_component_menu_CB */

/*
Global functions
----------------
*/
Widget create_choose_field_component_widget(Widget parent,
	struct FE_field *field,int component_no)
/*******************************************************************************
LAST MODIFIED : 29 September 1997

DESCRIPTION :
Creates an option menu from which a component of the field may be chosen.
==============================================================================*/
{
	MrmType choose_field_component_dialog_class;
	struct Choose_field_component_struct *choose_field_component;
	static MrmRegisterArg callback_list[]=
	{
		{"choose_object_identify_option",(XtPointer)
			DIALOG_IDENTIFY(choose_field_component,option)},
		{"choose_object_identify_menu",(XtPointer)
			DIALOG_IDENTIFY(choose_field_component,menu)},
		{"choose_object_destroy_CB",(XtPointer)
			choose_field_component_destroy_CB},
		{"choose_object_menu_CB",(XtPointer)
			choose_field_component_menu_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"Choose_object_structure",(XtPointer)NULL}
	};
	Widget return_widget;

	ENTER(create_choose_field_component_widget);
	return_widget=(Widget)NULL;
	/* check arguments */
	if (parent)
	{
		if (MrmOpenHierarchy_base64_string(choose_object_uidh,
			&choose_field_component_hierarchy,&choose_field_component_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(choose_field_component,
				struct Choose_field_component_struct,1))
			{
				/* initialise the structure */
				choose_field_component->widget_parent=parent;
				choose_field_component->widget=(Widget)NULL;
				choose_field_component->option=(Widget)NULL;
				choose_field_component->menu=(Widget)NULL;
				choose_field_component->update_callback.procedure=
					(Callback_procedure *)NULL;
				choose_field_component->update_callback.data=(void *)NULL;
				choose_field_component->current_field=field;
				choose_field_component->current_component_no=component_no;
				/* register the callbacks */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
					choose_field_component_hierarchy,callback_list,
					XtNumber(callback_list)))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)choose_field_component;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						choose_field_component_hierarchy,identifier_list,
						XtNumber(identifier_list)))
					{
						/* fetch choose_field_component control widget */
						if (MrmSUCCESS==MrmFetchWidget(choose_field_component_hierarchy,
							"choose_object_option",choose_field_component->widget_parent,
							&(choose_field_component->widget),
							&choose_field_component_dialog_class))
						{
							choose_field_component_set_field_component(
								choose_field_component->widget,field,component_no);
							return_widget=choose_field_component->widget;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_choose_field_component_widget.  "
								"Could not fetch choose_field_component dialog");
							DEALLOCATE(choose_field_component);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_choose_field_component_widget.  "
							"Could not register identifiers");
						DEALLOCATE(choose_field_component);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_choose_field_component_widget.  "
						"Could not register callbacks");
					DEALLOCATE(choose_field_component);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_choose_field_component_widget.  "
					"Could not allocate control window structure");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_choose_field_component_widget.  "
				"Could not open hierarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_choose_field_component_widget.  Invalid argument(s)");
	}
	LEAVE;

	return (return_widget);
} /* CREATE_choose_field_component_WIDGET */

int choose_field_component_set_callback(Widget choose_field_component_widget,
	struct Callback_data *new_callback)
/*******************************************************************************
LAST MODIFIED : 29 September 1997

DESCRIPTION :
Changes the callback item of the choose_field_component_widget.
==============================================================================*/
{
	int return_code;
	struct Choose_field_component_struct *choose_field_component;

	ENTER(choose_field_component_set_callback);
	/* check arguments */
	if (choose_field_component_widget&&new_callback)
	{
		/* Get the pointer to the data for the choose_field_component dialog */
		XtVaGetValues(choose_field_component_widget,XmNuserData,
			&choose_field_component,NULL);
		if (choose_field_component)
		{
			choose_field_component->update_callback.procedure=new_callback->procedure;
			choose_field_component->update_callback.data=new_callback->data;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"choose_field_component_set_callback.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_field_component_set_callback.  Missing widget");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* choose_field_component_set_callback */

int choose_field_component_set_field_component(
	Widget choose_field_component_widget,
	struct FE_field *field,int component_no)
/*******************************************************************************
LAST MODIFIED : 16 November 1998

DESCRIPTION :
Changes the field component in the choose_field_component_widget.
==============================================================================*/
{
	int return_code;
	struct Choose_field_component_struct *choose_field_component;

	ENTER(choose_field_component_set_field_component);
	/* check arguments */
	if (choose_field_component_widget)
	{
		/* Get the pointer to the data for the choose_field_component dialog */
		XtVaGetValues(choose_field_component_widget,XmNuserData,
			&choose_field_component,NULL);
		if (choose_field_component)
		{
			if ((!field)||(field&&(0<=component_no)&&
				(component_no<get_FE_field_number_of_components(field))))
			{
				choose_field_component->current_field=field;
				choose_field_component->current_component_no=component_no;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"choose_field_component_set_field_component.  Invalid component");
				choose_field_component->current_component_no=0;
				return_code=0;
			}
			choose_field_component_make_menu(choose_field_component);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"choose_field_component_set_field_component.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_field_component_set_field_component.  Missing widget");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* choose_field_component_set_field_component */

struct Callback_data *choose_field_component_get_callback(
	Widget choose_field_component_widget)
/*******************************************************************************
LAST MODIFIED : 29 September 1997

DESCRIPTION :
Returns a pointer to the callback item of the choose_field_component_widget.
==============================================================================*/
{
	struct Callback_data *return_address;
	struct Choose_field_component_struct *choose_field_component;

	ENTER(choose_field_component_get_callback);
	/* check arguments */
	if (choose_field_component_widget)
	{
		/* Get the pointer to the data for the choose_field_component dialog */
		XtVaGetValues(choose_field_component_widget,XmNuserData,
			&choose_field_component,NULL);
		if (choose_field_component)
		{
			return_address=&(choose_field_component->update_callback);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"choose_field_component_get_callback.  Missing widget data");
			return_address=(struct Callback_data *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_field_component_get_callback.  Missing widget");
		return_address=(struct Callback_data *)NULL;
	}
	LEAVE;

	return (return_address);
} /* choose_field_component_get_callback */

int choose_field_component_get_field_component(
	Widget choose_field_component_widget,struct FE_field **field,
	int *component_no)
/*******************************************************************************
LAST MODIFIED : 29 September 1997

DESCRIPTION :
Returns the current field and component number in the
choose_field_component_widget.
==============================================================================*/
{
	int return_code;
	struct Choose_field_component_struct *choose_field_component;

	ENTER(choose_field_component_get_field_component);
	/* check arguments */
	if (choose_field_component_widget)
	{
		/* Get the pointer to the data for the choose_field_component dialog */
		XtVaGetValues(choose_field_component_widget,XmNuserData,
			&choose_field_component,NULL);
		if (choose_field_component)
		{
			*field=choose_field_component->current_field;
			*component_no=choose_field_component->current_component_no;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"choose_field_component_get_field_component.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_field_component_get_field_component.  Missing widget");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* choose_field_component_get_field_component */
