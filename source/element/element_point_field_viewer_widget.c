/*******************************************************************************
FILE : element_point_field_viewer_widget.c

LAST MODIFIED : 12 October 2001

DESCRIPTION :
Widget for displaying and editing component values of computed fields defined
over a element_point. One field at a time can be viewed/edited.
Note the element_point passed to this widget should be a non-managed local copy.
==============================================================================*/
#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>
#include <Xm/TextF.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_discretization.h"
#include "finite_element/finite_element_to_graphics_object.h"
#include "general/debug.h"
#include "element/element_point_field_viewer_widget.h"
#include "time/time.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module constants
----------------
*/

/* following must be big enough to hold an element_point_xi value */
#define VALUE_STRING_SIZE 100

/*
Module types
------------
*/

struct Element_point_field_viewer_widget_struct
/*******************************************************************************
LAST MODIFIED : 30 June 2000

DESCRIPTION :
==============================================================================*/
{
	/* remember number of components to detect redefinition of computed fields */
	int number_of_components;
	struct Callback_data update_callback;
	/* information about the element point being edited; note element in
		 identifier is not accessed, and should not be managed */
	struct Element_point_ranges_identifier element_point_identifier;
	int element_point_number;
	/* field components whose values have been modified stored in following -
		 note however that this list is not owned by the widget */
	struct LIST(Field_value_index_ranges) *modified_field_components;
	/* xi value of element point being edited */
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	/* field being edited */
	struct Computed_field *current_field;
	Pixel label_background_color,label_foreground_color,
		textfield_background_color,textfield_bottom_shadow_color,
		textfield_foreground_color,textfield_top_shadow_color;
	Widget component_rowcol,widget,*widget_address,widget_parent;
	struct Time_object *time_object;
	/* Flag to record whether the time callback is active or not */
	int time_object_callback;
}; /* element_point_field_viewer_struct */

/*
Module functions
----------------
*/

static void element_point_field_viewer_widget_update(
	struct Element_point_field_viewer_widget_struct *element_point_field_viewer)
/*******************************************************************************
LAST MODIFIED : 22 May 2000

DESCRIPTION :
Tells CMGUI about the current values. Returns NULL as the object changes since
element point cannot be represented by a single object.
==============================================================================*/
{
	ENTER(element_point_field_viewer_widget_update);
	if (element_point_field_viewer)
	{
		if (element_point_field_viewer->update_callback.procedure)
		{
			/* now call the procedure with the user data and a NULL pointer */
			(element_point_field_viewer->update_callback.procedure)(
				element_point_field_viewer->widget,
				element_point_field_viewer->update_callback.data,NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_field_viewer_widget_update.  Invalid argument(s)");
	}
	LEAVE;
} /* element_point_field_viewer_widget_update */

static void element_point_field_viewer_widget_update_values(
	struct Element_point_field_viewer_widget_struct *element_point_field_viewer)
/*******************************************************************************
LAST MODIFIED : 30 June 2000

DESCRIPTION :
Updates all widgets in the rowcol to make sure they say the correct value.
==============================================================================*/
{
	char *value_string;
	FE_value time, *xi;
	int comp_no,num_children,number_of_components;
	Pixel bottom_shadow_color,top_shadow_color;
	struct Computed_field *field;
	struct FE_element *element,*top_level_element;
	struct Field_value_index_ranges *modified_ranges;
	Widget *child_list,widget;

	ENTER(element_point_field_viewer_widget_update_values);
	if (element_point_field_viewer&&element_point_field_viewer->component_rowcol&&
		(element=element_point_field_viewer->element_point_identifier.element)&&
		(top_level_element=
			element_point_field_viewer->element_point_identifier.top_level_element)&&
		(xi=element_point_field_viewer->xi)&&
		(field=element_point_field_viewer->current_field))
	{
		modified_ranges=FIND_BY_IDENTIFIER_IN_LIST(Field_value_index_ranges,field)(
			field,element_point_field_viewer->modified_field_components);
		number_of_components=Computed_field_get_number_of_components(field);
		time = Time_object_get_current_time(element_point_field_viewer->time_object);
		/* get children of the rowcol */
		child_list=(Widget *)NULL;
		XtVaGetValues(element_point_field_viewer->component_rowcol,XmNnumChildren,
			&num_children,XmNchildren,&child_list,NULL);
		if (child_list&&(num_children==(number_of_components*2)))
		{
			for (comp_no=0;comp_no<number_of_components;comp_no++)
			{
				widget=child_list[comp_no*2+1];
				if (value_string=Computed_field_evaluate_as_string_in_element(
					field,comp_no,element,xi,time,top_level_element))
				{
					/* set background colour to modified or unmodified if editable */
					XtVaGetValues(widget,XmNtopShadowColor,&top_shadow_color,NULL);
					XtVaGetValues(widget,XmNbottomShadowColor,&bottom_shadow_color,NULL);
					if (top_shadow_color != bottom_shadow_color)
					{
						if (modified_ranges&&Field_value_index_ranges_is_index_in_range(
							modified_ranges,comp_no))
						{
							/* invert colours of modified components */
							XtVaSetValues(widget,XmNforeground,
								element_point_field_viewer->textfield_background_color,NULL);
							XtVaSetValues(widget,XmNbackground,
								element_point_field_viewer->textfield_foreground_color,NULL);
						}
						else
						{
							XtVaSetValues(widget,XmNforeground,
								element_point_field_viewer->textfield_foreground_color,NULL);
							XtVaSetValues(widget,XmNbackground,
								element_point_field_viewer->textfield_background_color,NULL);
						}
					}
					XmTextFieldSetString(widget,value_string);
					DEALLOCATE(value_string);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"element_point_field_viewer_widget_update_values.  "
						"Could not get component as string");
				}
			}
			Computed_field_clear_cache(field);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"element_point_field_viewer_widget_update_values.  Missing children");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_field_viewer_widget_update_values.  Invalid argument(s)");
	}
	LEAVE;
} /* element_point_field_viewer_widget_update_values */

static int element_point_field_viewer_widget_time_change_callback(
	struct Time_object *time_object, double current_time,
	void *element_point_field_viewer_void)
/*******************************************************************************
LAST MODIFIED : 5 December 2001

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Element_point_field_viewer_widget_struct *element_point_field_viewer;

	ENTER(element_point_field_viewer_widget_time_change_callback);
	USE_PARAMETER(current_time);
	if (time_object && (element_point_field_viewer = 
		(struct Element_point_field_viewer_widget_struct *)
		element_point_field_viewer_void))
	{
		element_point_field_viewer_widget_update_values(element_point_field_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_field_viewer_widget_time_change_callback.  "
			"Invalid argument(s)");
	}
	LEAVE;
	return(return_code);
} /* element_point_field_viewer_widget_time_change_callback */

static void element_point_field_viewer_widget_destroy_CB(Widget widget,
	void *element_point_field_viewer_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 5 December 2001

DESCRIPTION :
Callback for when the element_point_field_viewer widget is destroyed. Tidies up
all dynamic memory allocations and pointers.
==============================================================================*/
{
	struct Element_point_field_viewer_widget_struct *element_point_field_viewer;

	ENTER(element_point_field_viewer_widget_destroy_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (element_point_field_viewer=
		(struct Element_point_field_viewer_widget_struct *)
		element_point_field_viewer_void)
	{
		if (element_point_field_viewer->time_object_callback)
		{
			Time_object_remove_callback(element_point_field_viewer->time_object,
				element_point_field_viewer_widget_time_change_callback,
				(void *)element_point_field_viewer);
		}					
		DEACCESS(Time_object)(&(element_point_field_viewer->time_object));
		*(element_point_field_viewer->widget_address)=(Widget)NULL;
		DEALLOCATE(element_point_field_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_field_viewer_widget_destroy_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* element_point_field_viewer_widget_destroy_CB */

static void element_point_field_viewer_widget_value_CB(Widget widget,
	void *element_point_field_viewer_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 6 June 2001

DESCRIPTION :
Called when the user has changed the data in the text widget.  Processes the
data, and then changes the correct value in the array structure.
==============================================================================*/
{
	char *field_value_string,*value_string;
	FE_value time,value,*values,*xi;
	int component_number,dimension,element_point_number,i,int_value,*int_values,
		number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS],number_of_grid_values,
		return_code;
	struct Computed_field *field;
	struct Element_point_field_viewer_widget_struct *element_point_field_viewer;
	XmAnyCallbackStruct *any_callback;
	struct FE_element *element,*top_level_element;
	struct FE_field *fe_field;

	ENTER(element_point_field_viewer_value_CB);
	component_number=-1;
	XtVaGetValues(widget,XmNuserData,&component_number,NULL);
	if ((element_point_field_viewer=
		(struct Element_point_field_viewer_widget_struct *)
		element_point_field_viewer_void)&&
		(element=element_point_field_viewer->element_point_identifier.element)&&
		(top_level_element=
			element_point_field_viewer->element_point_identifier.top_level_element)&&
		(xi=element_point_field_viewer->xi)&&
		(field=element_point_field_viewer->current_field)&&
		(0<=component_number)&&
		(component_number<Computed_field_get_number_of_components(field))&&
		(any_callback=(XmAnyCallbackStruct *)call_data))
	{
		time = Time_object_get_current_time(element_point_field_viewer->time_object);
		/* get old_value_string to prevent needless updating and preserve text
			 selections for cut-and-paste */
		if (value_string=XmTextFieldGetString(widget))
		{
			if (XmCR_ACTIVATE == any_callback->reason)
			{
				if ((XI_DISCRETIZATION_CELL_CORNERS==element_point_field_viewer->
					element_point_identifier.xi_discretization_mode)&&
					Computed_field_get_native_discretization_in_element(field,element,
						number_in_xi))
				{
					/* get the number of values that are stored in the grid */
					dimension=get_FE_element_dimension(element);
					number_of_grid_values=1;
					for (i=0;i<dimension;i++)
					{
						number_of_grid_values *= (number_in_xi[i]+1);
					}
					element_point_number=element_point_field_viewer->element_point_number;
					/* check the element_point_number is valid */
					if ((0<=element_point_number)&&
						(element_point_number<number_of_grid_values))
					{
						return_code=0;
						if (Computed_field_is_type_finite_element(field)&&
							Computed_field_get_type_finite_element(field,&fe_field)&&
							(INT_VALUE==get_FE_field_value_type(fe_field)))
						{
							/* handle integer value_type separately to avoid inaccuracies of
								 real->integer conversion */
							if (1==sscanf(value_string,"%d",&int_value))
							{
								if (get_FE_element_field_component_grid_int_values(element,
									fe_field,component_number,&int_values))
								{
									/* change the value for this component */
									int_values[element_point_number]=int_value;
									return_code=set_FE_element_field_component_grid_int_values(
										element,fe_field,component_number,int_values);
									DEALLOCATE(int_values);
								}
							}
						}
						else
						{
							if (1==sscanf(value_string,"%g",&value))
							{
								if (Computed_field_get_values_in_element(field,element,
									number_in_xi,&values,time))
								{
									/* change the value for this component */
									values[component_number*number_of_grid_values+
										element_point_number]=value;
									return_code=Computed_field_set_values_in_element(
										field,element,number_in_xi,values);
									DEALLOCATE(values);
								}
								/* note must clear cache so correct values are shown */
								Computed_field_clear_cache(field);
							}
						}
						if (return_code)
						{
							/* add this field component to the modified list */
							Field_value_index_ranges_list_add_field_value_index(
								element_point_field_viewer->modified_field_components,
								field,component_number);
							/* invert colours in widget to show it is modified */
							XtVaSetValues(widget,XmNforeground,
								element_point_field_viewer->textfield_background_color,NULL);
							XtVaSetValues(widget,XmNbackground,
								element_point_field_viewer->textfield_foreground_color,NULL);
							/* inform any clients of the changes */
							element_point_field_viewer_widget_update(
								element_point_field_viewer);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"element_point_field_viewer_value_CB.  "
							"element_point_number out of range");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"element_point_field_viewer_value_CB.  "
						"Cannot set values for non-grid-based field");
				}
			}
			/* redisplay the actual value for the field component */
			if (field_value_string=
				Computed_field_evaluate_as_string_in_element(
					field,component_number,element,xi,time,top_level_element))
			{
				/* only set string from field if different from that shown */
				if (strcmp(field_value_string,value_string))
				{
					XmTextFieldSetString(widget,field_value_string);
				}
				DEALLOCATE(field_value_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"element_point_field_viewer_value_CB.  "
					"Could not get component as string");
			}
			Computed_field_clear_cache(field);
			XtFree(value_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_field_viewer_value_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* element_point_field_viewer_value_CB */

static int element_point_field_is_editable(
	struct Element_point_ranges_identifier *element_point_identifier,
	struct Computed_field *field,int *number_in_xi)
/*******************************************************************************
LAST MODIFIED : 6 June 2000

DESCRIPTION :
Returns true if <field> is editable at <element_point_identifier>, and if so
returns the <number_in_xi>. Returns 0 with no error if no field or no element
is supplied.
==============================================================================*/
{
	int dimension,i,return_code;

	ENTER(element_point_field_is_editable);
	if (element_point_identifier&&number_in_xi)
	{
		if (element_point_identifier->element&&field&&
			(XI_DISCRETIZATION_CELL_CORNERS==
				element_point_identifier->xi_discretization_mode)&&
			Computed_field_get_native_discretization_in_element(field,
				element_point_identifier->element,number_in_xi))
		{
			return_code=1;
			/* check matching discretization */
			dimension=get_FE_element_dimension(element_point_identifier->element);
			for (i=0;(i<dimension)&&return_code;i++)
			{
				if (element_point_identifier->number_in_xi[i] != number_in_xi[i])
				{
					return_code=0;
				}
			}
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_field_is_editable.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* element_point_field_is_editable */

static int element_point_field_viewer_widget_setup_components(
	struct Element_point_field_viewer_widget_struct *element_point_field_viewer)
/*******************************************************************************
LAST MODIFIED : 20 June 2000

DESCRIPTION :
Creates the array of cells containing field component names and values.
==============================================================================*/
{
	Arg args[4];
	char *component_name;
	int comp_no,editable,i,num_children,number_of_components,
		number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS],return_code;
	struct Computed_field *field;
	struct FE_element *element;
	Widget *child_list,widget;
	XmString new_string;

	ENTER(element_point_field_viewer_setup_components);
	if (element_point_field_viewer)
	{
		return_code=1;
		element_point_field_viewer->number_of_components=-1;
		if (element_point_field_viewer->component_rowcol)
		{
			/* remove losing focus callbacks for all child widgets to prevent them
				 being called while widgets are being destroyed - this happens */
			num_children=0;
			child_list=(Widget *)NULL;
			XtVaGetValues(element_point_field_viewer->component_rowcol,
				XmNnumChildren,&num_children,XmNchildren,&child_list,NULL);
			if (child_list)
			{
				for (i=0;i<num_children;i++)
				{
					if (1==(i%2))
					{
						XtRemoveAllCallbacks(child_list[i],XmNlosingFocusCallback);
					}
				}
			}
			/* unmanage component_rowcol to avoid geometry requests, then destroy */
			XtUnmanageChild(element_point_field_viewer->component_rowcol);
			XtDestroyWidget(element_point_field_viewer->component_rowcol);
			/* must clear widget as we compare against it later */
			element_point_field_viewer->component_rowcol = (Widget)NULL;
		}
		if ((element=element_point_field_viewer->element_point_identifier.element)&&
			(field=element_point_field_viewer->current_field)&&
			Computed_field_is_defined_in_element(field,element))
		{
			editable=element_point_field_is_editable(
				&(element_point_field_viewer->element_point_identifier),
				field,number_in_xi);
			number_of_components=Computed_field_get_number_of_components(field);
			element_point_field_viewer->number_of_components=number_of_components;
			/* now create another */
			XtSetArg(args[0],XmNpacking,XmPACK_COLUMN);
			XtSetArg(args[1],XmNorientation,XmHORIZONTAL);
			XtSetArg(args[2],XmNentryAlignment,XmALIGNMENT_CENTER);
			XtSetArg(args[3],XmNnumColumns,number_of_components);
			if (element_point_field_viewer->component_rowcol=
				XmCreateRowColumn(element_point_field_viewer->widget,
					"element_point_field_viewer_rowcol",args,4))
			{
				XtManageChild(element_point_field_viewer->component_rowcol);
				for (comp_no=0;(comp_no<number_of_components)&&return_code;comp_no++)
				{
					/* component name label */
					if (component_name=Computed_field_get_component_name(field,comp_no))
					{
						new_string=XmStringCreateSimple(component_name);
						XtSetArg(args[0],XmNlabelString,new_string);
						XtSetArg(args[1],XmNalignment,XmALIGNMENT_CENTER);
						if (widget=XmCreateLabel(
							element_point_field_viewer->component_rowcol,"",args,2))
						{
							/* get label colours for non-editable text fields */
							XtVaGetValues(widget,XmNforeground,
								&(element_point_field_viewer->label_foreground_color),NULL);
							XtVaGetValues(widget,XmNbackground,
								&(element_point_field_viewer->label_background_color),NULL);
							XtManageChild(widget);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"element_point_field_viewer_widget_setup_components.  "
								"Could not create label gadget");
							return_code=0;
						}
						XmStringFree(new_string);
						DEALLOCATE(component_name);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"element_point_field_viewer_widget_setup_components.  "
							"Could not get field component name");
						return_code=0;
					}
					/* value text field */
					XtSetArg(args[0],XmNuserData,(XtPointer)comp_no);
					XtSetArg(args[1],XmNeditMode,XmSINGLE_LINE_EDIT);
					XtSetArg(args[2],XmNcolumns,10);
					XtSetArg(args[3],XmNeditable,editable);
					if (widget=XmCreateTextField(
						element_point_field_viewer->component_rowcol,"value",args,4))
					{
						/* get colours for editable text fields */
						XtVaGetValues(widget,XmNforeground,
							&(element_point_field_viewer->textfield_foreground_color),NULL);
						XtVaGetValues(widget,XmNbackground,
							&(element_point_field_viewer->textfield_background_color),NULL);
						XtVaGetValues(widget,XmNbottomShadowColor,
							&(element_point_field_viewer->textfield_bottom_shadow_color),
							NULL);
						XtVaGetValues(widget,XmNtopShadowColor,
							&(element_point_field_viewer->textfield_top_shadow_color),NULL);

						if (editable)
						{
							/* add callbacks for data input */
							XtAddCallback(widget,XmNactivateCallback,
								element_point_field_viewer_widget_value_CB,
								(XtPointer)element_point_field_viewer);
							XtAddCallback(widget,XmNlosingFocusCallback,
								element_point_field_viewer_widget_value_CB,
								(XtPointer)element_point_field_viewer);
						}
						else
						{
							/* make textfield look like a label to show value is uneditable */
							XtVaSetValues(widget,XmNforeground,
								element_point_field_viewer->label_foreground_color,NULL);
							XtVaSetValues(widget,XmNbackground,
								element_point_field_viewer->label_background_color,NULL);
							XtVaSetValues(widget,XmNbottomShadowColor,
								element_point_field_viewer->textfield_bottom_shadow_color,NULL);
							XtVaSetValues(widget,XmNtopShadowColor,
								element_point_field_viewer->textfield_bottom_shadow_color,NULL);
						}
						XtManageChild(widget);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"element_point_field_viewer_widget_setup_components.  "
							"Could not create text field widget");
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"element_point_field_viewer_widget_setup_components.  "
					"Could not make component_rowcol");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_field_viewer_widget_setup_components.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* element_point_field_viewer_widget_setup_components */

/*
Global functions
----------------
*/

Widget create_element_point_field_viewer_widget(
	Widget *element_point_field_viewer_widget_address,Widget parent,
	struct LIST(Field_value_index_ranges) *modified_field_components,
	struct Element_point_ranges_identifier *initial_element_point_identifier,
	int initial_element_point_number,struct Computed_field *initial_field,
	struct Time_object *time_object)
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Widget for displaying and editing computed field components at element points.
<initial_element_point_identifier> must be passed, but no initial element point
is referred to if its element is NULL. The element in the identifier, if any,
should be a local copy of a global element; up to the parent dialog to make
changes global.
==============================================================================*/
{
	Arg args[7];
	struct Element_point_field_viewer_widget_struct *element_point_field_viewer;
	struct FE_element *initial_element;
	Widget return_widget;

	ENTER(create_element_point_field_viewer_widget);
	return_widget=(Widget)NULL;
	if (element_point_field_viewer_widget_address&&parent&&
		modified_field_components&&initial_element_point_identifier&&
		(((struct FE_element *)NULL==
			(initial_element=initial_element_point_identifier->element))||
			(Element_point_ranges_identifier_element_point_number_is_valid(
				initial_element_point_identifier,initial_element_point_number)&&
				initial_field&&
				Computed_field_is_defined_in_element(initial_field,initial_element))))
	{
		/* allocate memory */
		if (ALLOCATE(element_point_field_viewer,
			struct Element_point_field_viewer_widget_struct,1))
		{
			/* initialise the structure */
			element_point_field_viewer->modified_field_components=
				modified_field_components;
			element_point_field_viewer->number_of_components=-1;
			element_point_field_viewer->element_point_identifier.element=
				(struct FE_element *)NULL;
			element_point_field_viewer->current_field=(struct Computed_field *)NULL;
			element_point_field_viewer->update_callback.procedure=
				(Callback_procedure *)NULL;
			element_point_field_viewer->update_callback.data=NULL;
			element_point_field_viewer->time_object=ACCESS(Time_object)(
				time_object);
			/* initialise widgets */
			element_point_field_viewer->component_rowcol=(Widget)NULL;
			element_point_field_viewer->widget=(Widget)NULL;
			element_point_field_viewer->widget_address=
				element_point_field_viewer_widget_address;
			element_point_field_viewer->widget_parent=parent;
			/* create the main scrolled window */
			XtSetArg(args[0],XmNleftAttachment,XmATTACH_FORM);
			XtSetArg(args[1],XmNrightAttachment,XmATTACH_FORM);
			XtSetArg(args[2],XmNtopAttachment,XmATTACH_FORM);
			XtSetArg(args[3],XmNbottomAttachment,XmATTACH_FORM);
			XtSetArg(args[4],XmNscrollingPolicy,XmAUTOMATIC);
			XtSetArg(args[5],XmNuserData,element_point_field_viewer);
			/*???RC yep, somewhat arbitrary height, but preferable to default */
			XtSetArg(args[6],XmNheight,186);
			if (element_point_field_viewer->widget=XmCreateScrolledWindow(parent,
				"element_point_field_viewer_widget",args,7))
			{
				/* add destroy callback for widget */
				XtAddCallback(element_point_field_viewer->widget,XmNdestroyCallback,
					element_point_field_viewer_widget_destroy_CB,
					(XtPointer)element_point_field_viewer);
				/* create the components of the field */
				element_point_field_viewer_widget_set_element_point_field(
					element_point_field_viewer->widget,initial_element_point_identifier,
					initial_element_point_number,initial_field);
				XtManageChild(element_point_field_viewer->widget);
				return_widget=element_point_field_viewer->widget;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_element_point_field_viewer_widget.  Could not create widget");
				DEALLOCATE(element_point_field_viewer);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_element_point_field_viewer_widget.  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_element_point_field_viewer_widget.  Invalid argument(s)");
	}
	if (element_point_field_viewer_widget_address)
	{
		*element_point_field_viewer_widget_address=return_widget;
	}
	LEAVE;

	return (return_widget);
} /* create_element_point_field_viewer_widget */

int element_point_field_viewer_widget_set_callback(
	Widget element_point_field_viewer_widget,struct Callback_data *callback)
/*******************************************************************************
LAST MODIFIED : 22 May 2000

DESCRIPTION :
Sets the callback for updates when the field of the element_point in the viewer
has been modified.
==============================================================================*/
{
	int return_code;
	struct Element_point_field_viewer_widget_struct *element_point_field_viewer;

	ENTER(element_point_field_viewer_widget_set_callback);
	if (element_point_field_viewer_widget&&callback)
	{
		element_point_field_viewer=
			(struct Element_point_field_viewer_widget_struct *)NULL;
		/* get the element_point_field_viewer structure from the widget */
		XtVaGetValues(element_point_field_viewer_widget,
			XmNuserData,&element_point_field_viewer,NULL);
		if (element_point_field_viewer)
		{
			element_point_field_viewer->update_callback.procedure=callback->procedure;
			element_point_field_viewer->update_callback.data=callback->data;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"element_point_field_viewer_widget_set_callback.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_field_viewer_widget_set_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* element_point_field_viewer_widget_set_callback */

int element_point_field_viewer_widget_get_element_point_field(
	Widget element_point_field_viewer_widget,
	struct Element_point_ranges_identifier *element_point_identifier,
	int *element_point_number_address,struct Computed_field **field_address)
/*******************************************************************************
LAST MODIFIED : 31 May 2000

DESCRIPTION :
Returns the element_point/field being edited in the
<element_point_field_viewer_widget>.
==============================================================================*/
{
	int return_code;
	struct Element_point_field_viewer_widget_struct *element_point_field_viewer;

	ENTER(element_point_field_viewer_widget_get_element_point_field);
	if (element_point_field_viewer_widget&&element_point_identifier&&
		element_point_number_address&&field_address)
	{
		element_point_field_viewer=
			(struct Element_point_field_viewer_widget_struct *)NULL;
		/* get the element_point_field_viewer structure from the widget */
		XtVaGetValues(element_point_field_viewer_widget,
			XmNuserData,&element_point_field_viewer,NULL);
		if (element_point_field_viewer)
		{
			COPY(Element_point_ranges_identifier)(element_point_identifier,
				&(element_point_field_viewer->element_point_identifier));
			*element_point_number_address=
				element_point_field_viewer->element_point_number;
			*field_address = element_point_field_viewer->current_field;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"element_point_field_viewer_widget_get_element_point_field.  "
				"Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_field_viewer_widget_get_element_point_field.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* element_point_field_viewer_widget_get_element_point_field */

int element_point_field_viewer_widget_set_element_point_field(
	Widget element_point_field_viewer_widget,
	struct Element_point_ranges_identifier *element_point_identifier,
	int element_point_number,struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 23 April 2001

DESCRIPTION :
Sets the element_point/field being edited in the
<element_point_field_viewer_widget>.
Note that the viewer works on the element itself, not a local copy. Hence, only
pass unmanaged elements in the element_point_identifier to this widget.
==============================================================================*/
{
	int new_editable,number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS],old_editable,
		return_code,setup_components;
	struct Element_point_field_viewer_widget_struct *element_point_field_viewer;

	ENTER(element_point_field_viewer_widget_set_element_point_field);
	if (element_point_field_viewer_widget&&
		(((struct FE_element *)NULL==element_point_identifier->element)||
			(Element_point_ranges_identifier_element_point_number_is_valid(
				element_point_identifier,element_point_number)&&field&&
				Computed_field_is_defined_in_element(field,
					element_point_identifier->element))))
	{
		element_point_field_viewer=
			(struct Element_point_field_viewer_widget_struct *)NULL;
		/* get the element_point_field_viewer structure from the widget */
		XtVaGetValues(element_point_field_viewer_widget,
			XmNuserData,&element_point_field_viewer,NULL);
		if (element_point_field_viewer)
		{
			/* rebuild viewer widgets if nature of element_point or field changed */
			old_editable=element_point_field_is_editable(
				&(element_point_field_viewer->element_point_identifier),
				element_point_field_viewer->current_field,number_in_xi);
			new_editable=element_point_field_is_editable(
				element_point_identifier,field,number_in_xi);
			if ((!element_point_identifier->element)||
				(!(element_point_field_viewer->element_point_identifier.element))||
				(field != element_point_field_viewer->current_field)||
				(field&&((element_point_field_viewer->number_of_components !=
					Computed_field_get_number_of_components(field))||
					(new_editable != old_editable))))
			{
				setup_components=1;
			}
			else
			{
				setup_components=0;
			}
			if (element_point_identifier->element&&field)
			{
				COPY(Element_point_ranges_identifier)(
					&(element_point_field_viewer->element_point_identifier),
					element_point_identifier);
				element_point_field_viewer->element_point_number=element_point_number;
				element_point_field_viewer->current_field=field;
				FE_element_get_numbered_xi_point(
					element_point_identifier->element,
					element_point_identifier->xi_discretization_mode,
					element_point_identifier->number_in_xi,
					element_point_identifier->exact_xi,
					/*coordinate_field*/(struct Computed_field *)NULL,
					/*density_field*/(struct Computed_field *)NULL,
					element_point_number, element_point_field_viewer->xi,
					/*time*/0);
			}
			else
			{
				element_point_field_viewer->element_point_identifier.element=
					(struct FE_element *)NULL;
				element_point_field_viewer->current_field=(struct Computed_field *)NULL;
			}
			if (setup_components)
			{
				element_point_field_viewer_widget_setup_components(
					element_point_field_viewer);
			}
			if (element_point_identifier->element&&field)
			{
				if (Computed_field_has_multiple_times(field))
				{
					if (!element_point_field_viewer->time_object_callback)
					{
						element_point_field_viewer->time_object_callback = 
							Time_object_add_callback(element_point_field_viewer->time_object,
							element_point_field_viewer_widget_time_change_callback,
							(void *)element_point_field_viewer);
					}
				}
				else
				{
					if (element_point_field_viewer->time_object_callback)
					{
						Time_object_remove_callback(element_point_field_viewer->time_object,
							element_point_field_viewer_widget_time_change_callback,
							(void *)element_point_field_viewer);
						element_point_field_viewer->time_object_callback = 0;
					}					
				}

				element_point_field_viewer_widget_update_values(
					element_point_field_viewer);
				XtManageChild(element_point_field_viewer->widget);
			}
			else
			{
				XtUnmanageChild(element_point_field_viewer->widget);
			}
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"element_point_field_viewer_widget_set_element_point_field.  "
				"Missing widget data");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_field_viewer_widget_set_element_point_field.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* element_point_field_viewer_widget_set_element_point_field */
