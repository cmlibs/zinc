/*******************************************************************************
FILE : element_point_viewer_widget.c

LAST MODIFIED : 20 March 2003

DESCRIPTION :
Widget for editing field values stored at an element point with multiple text
fields, visible one field at a time. Any computed field may be viewed. Most can
be modified too.
Note the element passed to this widget should be a non-managed local copy.
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
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Mrm/MrmPublic.h>
#include "choose/choose_computed_field.h"
#include "general/debug.h"
#include "element/element_point_field_viewer_widget.h"
#include "element/element_point_viewer_widget.h"
static char element_point_viewer_widget_uidh[] =
#include "element/element_point_viewer_widget.uidh"
	;
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int element_point_viewer_hierarchy_open=0;
static MrmHierarchy element_point_viewer_hierarchy;
#endif /* defined (MOTIF) */

/*
Module types
------------
*/

struct Element_point_viewer_widget_struct
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION :
Contains all the information carried by the element_point_viewer widget.
==============================================================================*/
{
	struct Computed_field_package *computed_field_package;
	void *computed_field_manager_callback_id;
	/* information about the element point being edited; note element in
		 identifier is not accessed, and should not be managed */
	struct Element_point_ranges_identifier element_point_identifier;
	int element_point_number;
	/* field components whose values have been modified stored in following -
		 note however that this list is not owned by the widget */
	struct LIST(Field_value_index_ranges) *modified_field_components;
	/* local copy of the element from the identifier */
	struct FE_element *template_element;
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct Callback_data update_callback;
	Widget choose_field_form,choose_field_widget,field_viewer_form,
		field_viewer_widget;
	Widget widget,*widget_address,widget_parent;
}; /* Element_point_viewer_widget_struct */

/*
Module functions
----------------
*/

DECLARE_DIALOG_IDENTIFY_FUNCTION(element_point_viewer_widget, \
	Element_point_viewer_widget_struct,choose_field_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_point_viewer_widget, \
	Element_point_viewer_widget_struct,field_viewer_form)

static void element_point_viewer_widget_update(
	struct Element_point_viewer_widget_struct *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Tells the parent dialog the current_element has changed.
==============================================================================*/
{
	ENTER(element_point_viewer_widget_update);
	if (element_point_viewer)
	{
		if (element_point_viewer->update_callback.procedure)
		{
			/* now call the procedure with the user data and a NULL pointer */
			(element_point_viewer->update_callback.procedure)(
				element_point_viewer->widget,
				element_point_viewer->update_callback.data,
				(void *)NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_viewer_widget_update.  Invalid argument(s)");
	}
	LEAVE;
} /* element_point_viewer_widget_update */

static void element_point_viewer_widget_destroy_CB(Widget widget,
	void *element_point_viewer_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Callback for when the element_point_viewer widget is destroyed. Tidies up all
dynamic memory allocations and pointers.
==============================================================================*/
{
	struct Element_point_viewer_widget_struct *element_point_viewer;

	ENTER(element_point_viewer_widget_destroy_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (element_point_viewer=
		(struct Element_point_viewer_widget_struct *)element_point_viewer_void)
	{
		REACCESS(FE_element)(&(element_point_viewer->template_element),
			(struct FE_element *)NULL);
		if (element_point_viewer->computed_field_manager_callback_id)
		{
			MANAGER_DEREGISTER(Computed_field)(
				element_point_viewer->computed_field_manager_callback_id,
				Computed_field_package_get_computed_field_manager(
					element_point_viewer->computed_field_package));
		}
		*(element_point_viewer->widget_address)=(Widget)NULL;
		DEALLOCATE(element_point_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_viewer_widget_destroy_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* element_point_viewer_widget_destroy_CB */

static void element_point_viewer_widget_update_choose_field(Widget widget,
	void *element_point_viewer_void,void *field_void)
/*******************************************************************************
LAST MODIFIED : 31 May 2000

DESCRIPTION :
Callback for change of field.
==============================================================================*/
{
	struct Computed_field *field;
	struct Element_point_viewer_widget_struct *element_point_viewer;

	ENTER(element_point_viewer_widget_update_choose_field);
	USE_PARAMETER(widget);
	if (element_point_viewer=
		(struct Element_point_viewer_widget_struct *)element_point_viewer_void)
	{
		field=(struct Computed_field *)field_void;
		element_point_field_viewer_widget_set_element_point_field(
			element_point_viewer->field_viewer_widget,
			&(element_point_viewer->element_point_identifier),
			element_point_viewer->element_point_number,field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_viewer_widget_update_choose_field.  Invalid argument(s)");
	}
	LEAVE;
} /* element_point_viewer_widget_update_choose_field */

static void element_point_viewer_widget_computed_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message,
	void *element_point_viewer_void)
/*******************************************************************************
LAST MODIFIED : 28 May 2001

DESCRIPTION :
One or more of the computed_fields have changed in the manager. If the
current_field in the <element_point_viewer> has changed, re-send it to the
element_point_field_viewer to update widgets/values etc.
Note that delete/add messages are handled by the field chooser.
???RC Review Manager Messages Here
==============================================================================*/
{
	struct Computed_field *field;
	struct Element_point_viewer_widget_struct *element_point_viewer;

	ENTER(element_point_viewer_widget_computed_field_change);
	if (message&&(element_point_viewer=
		(struct Element_point_viewer_widget_struct *)element_point_viewer_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Computed_field):
			case MANAGER_CHANGE_OBJECT(Computed_field):
			{
				field = CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
					element_point_viewer->choose_field_widget);
				if (Computed_field_depends_on_Computed_field_in_list(
					field, message->changed_object_list))
				{
					element_point_field_viewer_widget_set_element_point_field(
						element_point_viewer->field_viewer_widget,
						&(element_point_viewer->element_point_identifier),
						element_point_viewer->element_point_number,field);
				}
			} break;
			case MANAGER_CHANGE_ADD(Computed_field):
			case MANAGER_CHANGE_NONE(Computed_field):
			case MANAGER_CHANGE_REMOVE(Computed_field):
			case MANAGER_CHANGE_IDENTIFIER(Computed_field):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_viewer_widget_computed_field_change.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* element_point_viewer_widget_computed_field_change */

static void element_point_viewer_widget_update_field_viewer(
	Widget element_point_field_viewer_widget,void *element_point_viewer_void,
	void *call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Callback for change of values in the <element_point_field_viewer_widget>.
Notify parent dialog of change to element_point.
==============================================================================*/
{
	struct Element_point_viewer_widget_struct *element_point_viewer;

	ENTER(element_point_viewer_widget_update_field_viewer);
	USE_PARAMETER(element_point_field_viewer_widget);
	USE_PARAMETER(call_data);
	if (element_point_viewer=
		(struct Element_point_viewer_widget_struct *)element_point_viewer_void)
	{
		element_point_viewer_widget_update(element_point_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_viewer_widget_update_field_viewer.  Invalid argument(s)");
	}
	LEAVE;
} /* element_point_viewer_widget_update_field_viewer */

/*
Global functions
----------------
*/

Widget create_element_point_viewer_widget(
	Widget *element_point_viewer_widget_address,
	Widget parent,struct Computed_field_package *computed_field_package,
	struct LIST(Field_value_index_ranges) *modified_field_components,
	struct Element_point_ranges_identifier *initial_element_point_identifier,
	int initial_element_point_number, struct Time_object *time_object,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Creates a widget for displaying and editing the contents of the element point
at <initial_element_point_identifier> <initial_element_point_number>.
<initial_element_point_identifier> must be passed, but no initial element point
is referred to if its element is NULL. The element in the identifier, if any,
should be a local copy of a global element; up to the parent dialog to make
changes global.
==============================================================================*/
{
	int init_widgets;
	MANAGER_CONDITIONAL_FUNCTION(Computed_field)
		*choose_field_conditional_function;
	MrmType element_point_viewer_dialog_class;
	struct CM_element_information element_identifier;
	struct Computed_field *initial_field;
	struct Callback_data callback;
	struct Element_point_viewer_widget_struct *element_point_viewer=NULL;
	struct FE_element *initial_element;
	static MrmRegisterArg callback_list[]=
	{
		{"elem_pt_vw_id_choose_field_form",(XtPointer)
			DIALOG_IDENTIFY(element_point_viewer_widget,choose_field_form)},
		{"elem_pt_vw_id_field_viewer_form",(XtPointer)
			DIALOG_IDENTIFY(element_point_viewer_widget,field_viewer_form)},
		{"elem_pt_vw_destroy_CB",(XtPointer)element_point_viewer_widget_destroy_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"elem_pt_vw_structure",(XtPointer)NULL}
	};
	Widget return_widget;

	ENTER(create_element_point_viewer_widget);
	return_widget=(Widget)NULL;
	if (element_point_viewer_widget_address&&parent&&computed_field_package&&
		modified_field_components&&initial_element_point_identifier&&
		user_interface &&
		(((struct FE_element *)NULL==initial_element_point_identifier->element)||
			Element_point_ranges_identifier_element_point_number_is_valid(
				initial_element_point_identifier,initial_element_point_number)))
	{
		if (MrmOpenHierarchy_binary_string(element_point_viewer_widget_uidh,sizeof(element_point_viewer_widget_uidh),
			&element_point_viewer_hierarchy,&element_point_viewer_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(element_point_viewer,
				struct Element_point_viewer_widget_struct,1))
			{
				/* initialise the structure */
				if (initial_element=initial_element_point_identifier->element)
				{
					choose_field_conditional_function=
						Computed_field_is_defined_in_element_conditional;
				}
				else
				{
					choose_field_conditional_function=
						(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
				}
				initial_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					choose_field_conditional_function,(void *)initial_element,
					Computed_field_package_get_computed_field_manager(
						computed_field_package));
				element_point_viewer->computed_field_package=computed_field_package;
				element_point_viewer->computed_field_manager_callback_id=(void *)NULL;
				element_point_viewer->modified_field_components=
					modified_field_components;
				COPY(Element_point_ranges_identifier)(
					&(element_point_viewer->element_point_identifier),
					initial_element_point_identifier);
				element_point_viewer->element_point_number=initial_element_point_number;
				if (initial_element &&
					get_FE_element_identifier(initial_element, &element_identifier))
				{
					element_point_viewer->template_element = ACCESS(FE_element)(
						CREATE(FE_element)(&element_identifier,
							(struct FE_element_shape *)NULL, (struct FE_region *)NULL,
							initial_element));
				}
				else
				{
					element_point_viewer->template_element=(struct FE_element *)NULL;
				}
				element_point_viewer->update_callback.procedure=
					(Callback_procedure *)NULL;
				element_point_viewer->update_callback.data=NULL;
				/* initialise the widgets */
				element_point_viewer->widget=(Widget)NULL;
				element_point_viewer->widget_address=
					element_point_viewer_widget_address;
				element_point_viewer->widget_parent=parent;
				element_point_viewer->choose_field_form=(Widget)NULL;
				element_point_viewer->field_viewer_form=(Widget)NULL;
				element_point_viewer->field_viewer_form=(Widget)NULL;
				element_point_viewer->field_viewer_widget=(Widget)NULL;
				/* register the callbacks */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
					element_point_viewer_hierarchy,callback_list,XtNumber(callback_list)))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)element_point_viewer;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						element_point_viewer_hierarchy,identifier_list,
						XtNumber(identifier_list)))
					{
						/* fetch position window widget */
						if (MrmSUCCESS==MrmFetchWidget(element_point_viewer_hierarchy,
							"element_point_viewer_widget",element_point_viewer->widget_parent,
							&(element_point_viewer->widget),
							&element_point_viewer_dialog_class))
						{
							XtManageChild(element_point_viewer->widget);
							/* create subwidgets */
							init_widgets=1;
							if (!(element_point_viewer->choose_field_widget=
								CREATE_CHOOSE_OBJECT_WIDGET(Computed_field)(
								element_point_viewer->choose_field_form,initial_field,
								Computed_field_package_get_computed_field_manager(
									computed_field_package),
								choose_field_conditional_function,
								(void *)(element_point_viewer->template_element),
								user_interface)))
							{
								init_widgets=0;
							}
							if (!(create_element_point_field_viewer_widget(
								&(element_point_viewer->field_viewer_widget),
								element_point_viewer->field_viewer_form,
								modified_field_components,
								&(element_point_viewer->element_point_identifier),
								initial_element_point_number,
								initial_field, time_object)))
							{
								init_widgets=0;
							}
							if (init_widgets)
							{
								callback.procedure=
									element_point_viewer_widget_update_field_viewer;
								callback.data=element_point_viewer;
								element_point_field_viewer_widget_set_callback(
									element_point_viewer->field_viewer_widget,&callback);
								callback.procedure=
									element_point_viewer_widget_update_choose_field;
								CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
									element_point_viewer->choose_field_widget,&callback);
								element_point_viewer->computed_field_manager_callback_id=
									MANAGER_REGISTER(Computed_field)(
										element_point_viewer_widget_computed_field_change,
										(void *)element_point_viewer,
										Computed_field_package_get_computed_field_manager(
											element_point_viewer->computed_field_package));
								return_widget=element_point_viewer->widget;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"create_element_point_viewer_widget.  "
									"Could not create subwidgets");
								DEALLOCATE(element_point_viewer);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_element_point_viewer_widget.  "
								"Could not fetch element_point_viewer dialog");
							DEALLOCATE(element_point_viewer);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_element_point_viewer_widget.  "
							"Could not register identifiers");
						DEALLOCATE(element_point_viewer);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_element_point_viewer_widget.  "
						"Could not register callbacks");
					DEALLOCATE(element_point_viewer);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"create_element_point_viewer_widget.  "
					"Could not allocate element_point_viewer widget structure");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_element_point_viewer_widget.  Could not open hierarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_element_point_viewer_widget.  Invalid argument(s)");
	}
	if (element_point_viewer_widget_address)
	{
		*element_point_viewer_widget_address=return_widget;
	}
	LEAVE;

	return (return_widget);
} /* create_element_point_viewer_widget */

int element_point_viewer_widget_set_callback(Widget element_point_viewer_widget,
	struct Callback_data *callback)
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Sets the callback for updates when the contents of the element point in the
viewer have changed.
==============================================================================*/
{
	int return_code;
	struct Element_point_viewer_widget_struct *element_point_viewer;

	ENTER(element_point_viewer_widget_set_callback);
	if (element_point_viewer_widget&&callback)
	{
		element_point_viewer=(struct Element_point_viewer_widget_struct *)NULL;
		/* get the element_point_viewer structure from the widget */
		XtVaGetValues(element_point_viewer_widget,
			XmNuserData,&element_point_viewer,NULL);
		if (element_point_viewer)
		{
			element_point_viewer->update_callback.procedure=callback->procedure;
			element_point_viewer->update_callback.data=callback->data;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"element_point_viewer_widget_set_callback.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_viewer_widget_set_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* element_point_viewer_widget_set_callback */

int element_point_viewer_widget_get_element_point(
	Widget element_point_viewer_widget,
	struct Element_point_ranges_identifier *element_point_identifier,
	int *element_point_number_address)
/*******************************************************************************
LAST MODIFIED : 31 May 2000

DESCRIPTION :
Returns the element_point being edited in the <element_point_viewer_widget>.
==============================================================================*/
{
	int return_code;
	struct Element_point_viewer_widget_struct *element_point_viewer;

	ENTER(element_point_viewer_widget_get_element_point);
	if (element_point_viewer_widget&&element_point_identifier&&
		element_point_number_address)
	{
		element_point_viewer=(struct Element_point_viewer_widget_struct *)NULL;
		/* get the element_point_viewer structure from the widget */
		XtVaGetValues(element_point_viewer_widget,
			XmNuserData,&element_point_viewer,NULL);
		if (element_point_viewer)
		{
			COPY(Element_point_ranges_identifier)(element_point_identifier,
				&(element_point_viewer->element_point_identifier));
			*element_point_number_address=element_point_viewer->element_point_number;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"element_point_viewer_widget_get_element_point.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_viewer_widget_get_element_point.  Invalid argument(s)");
			return_code=0;
	}
	LEAVE;

	return (return_code);
} /* element_point_viewer_widget_get_element_point */

int element_point_viewer_widget_set_element_point(
	Widget element_point_viewer_widget,
	struct Element_point_ranges_identifier *element_point_identifier,
	int element_point_number)
/*******************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Sets the element point being edited in the <element_point_viewer_widget>. Note
that the viewer works on the element itself, not a local copy. Hence, only pass
unmanaged elements in the identifier to this widget.
==============================================================================*/
{
	int change_conditional_function,return_code;
	MANAGER_CONDITIONAL_FUNCTION(Computed_field)
		*choose_field_conditional_function;
	struct CM_element_information element_identifier;
	struct Computed_field *field;
	struct FE_element *element,*template_element;
	struct Element_point_viewer_widget_struct *element_point_viewer;

	ENTER(element_point_viewer_widget_set_element_point);
	if (element_point_viewer_widget&&element_point_identifier&&
		(((struct FE_element *)NULL==element_point_identifier->element)||
			Element_point_ranges_identifier_element_point_number_is_valid(
				element_point_identifier,element_point_number)))
	{
		element_point_viewer=(struct Element_point_viewer_widget_struct *)NULL;
		/* get the element_point_viewer structure from the widget */
		XtVaGetValues(element_point_viewer_widget,
			XmNuserData,&element_point_viewer,NULL);
		if (element_point_viewer)
		{
			change_conditional_function=0;
			if (element = element_point_identifier->element)
			{
				field=CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
					element_point_viewer->choose_field_widget);
				if (!(element_point_viewer->template_element) ||
					(!equivalent_computed_fields_at_elements(element,
						element_point_viewer->template_element)))
				{
					choose_field_conditional_function=
						Computed_field_is_defined_in_element_conditional;
					change_conditional_function=1;
					if ((!field)||
						(!Computed_field_is_defined_in_element(field,element)))
					{
						field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
							choose_field_conditional_function,(void *)element,
							Computed_field_package_get_computed_field_manager(
								element_point_viewer->computed_field_package));
					}
					get_FE_element_identifier(element, &element_identifier);
					template_element = CREATE(FE_element)(&element_identifier,
						(struct FE_element_shape *)NULL, (struct FE_region *)NULL, element);
				}
			}
			else
			{
				field=(struct Computed_field *)NULL;
				choose_field_conditional_function=
					(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
				change_conditional_function=1;
				template_element=(struct FE_element *)NULL;
			}
			COPY(Element_point_ranges_identifier)(
				&(element_point_viewer->element_point_identifier),
				element_point_identifier);
			element_point_viewer->element_point_number=element_point_number;
			if (change_conditional_function)
			{
				REACCESS(FE_element)(&(element_point_viewer->template_element),
					template_element);
				CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION(Computed_field)(
					element_point_viewer->choose_field_widget,
					choose_field_conditional_function,
					(void *)element_point_viewer->template_element,field);
			}
			element_point_field_viewer_widget_set_element_point_field(
				element_point_viewer->field_viewer_widget,
				&(element_point_viewer->element_point_identifier),
				element_point_number,field);
			if (element)
			{
				XtManageChild(element_point_viewer->widget);
			}
			else
			{
				XtUnmanageChild(element_point_viewer->widget);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"element_point_viewer_widget_set_element_point.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_viewer_widget_set_element_point.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* element_point_viewer_widget_set_element_point */

