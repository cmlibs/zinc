/*******************************************************************************
FILE : node_field_viewer_widget.c

LAST MODIFIED : 6 June 2001

DESCRIPTION :
Widget for displaying and editing component values of computed fields defined
over a node. One field at a time can be viewed/edited.
Note the node passed to this widget should be a non-managed local copy.
==============================================================================*/
#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/LabelG.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>
#include <Xm/TextF.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "finite_element/finite_element.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "node/node_field_viewer_widget.h"
#include "time/time.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module constants
----------------
*/

/* following must be big enough to hold an element_xi value */
#define VALUE_STRING_SIZE 100

/*
Module types
------------
*/

struct Nodal_value_information
{
	enum FE_nodal_value_type type;
	int component_number,version;
	struct Computed_field *field;
}; /* struct Nodal_value_information */

struct Node_field_viewer_widget_struct
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
==============================================================================*/
{
	enum FE_nodal_value_type *nodal_value_types;
	int number_of_nodal_value_types;
	/* remember number of components to detect redefinition of computed fields */
	int number_of_components;
	struct Callback_data update_callback;
	struct Computed_field *current_field;
	struct FE_node *current_node;
	struct Time_object *time_object;
	/* Flag to record whether the time callback is active or not */
	int time_object_callback;
	Widget component_rowcol,widget,*widget_address,widget_parent;
}; /* node_field_viewer_struct */

/*
Module functions
----------------
*/

static void node_field_viewer_widget_update(
	struct Node_field_viewer_widget_struct *node_field_viewer)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Tells CMGUI about the current values. Returns a pointer to the node field.
==============================================================================*/
{
	ENTER(node_field_viewer_widget_update);
	if (node_field_viewer)
	{
		if (node_field_viewer->update_callback.procedure)
		{
			/* now call the procedure with the user data and a NULL pointer */
			(node_field_viewer->update_callback.procedure)(node_field_viewer->widget,
				node_field_viewer->update_callback.data,NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_field_viewer_widget_update.  Invalid argument(s)");
	}
	LEAVE;
} /* node_field_viewer_widget_update */

static void node_field_viewer_widget_update_values(
	struct Node_field_viewer_widget_struct *node_field_viewer)
/*******************************************************************************
LAST MODIFIED : 6 June 2001

DESCRIPTION :
Updates all widgets in the rowcol to make sure they say the correct value.
==============================================================================*/
{
	char *cmiss_number_string, *new_value_string, *old_value_string,
		temp_value_string[VALUE_STRING_SIZE];
	FE_value *values, time;
	int i, j, num_children, number_of_components;
	struct Computed_field *field;
	struct FE_field *fe_field;
	struct FE_field_component fe_field_component;
	struct FE_node *node;
	struct Nodal_value_information *nodal_value_information;
	Widget *child_list, text_field_widget;

	ENTER(node_field_viewer_widget_update_values);
	if (node_field_viewer&&node_field_viewer->component_rowcol&&
		(node=node_field_viewer->current_node)&&
		(field=node_field_viewer->current_field))
	{
		number_of_components = Computed_field_get_number_of_components(field);
		fe_field = (struct FE_field *)NULL;
		cmiss_number_string = (char *)NULL;
		time = Time_object_get_current_time(node_field_viewer->time_object);
		values = (FE_value *)NULL;
		/* get children of the rowcol */
		XtVaGetValues(node_field_viewer->component_rowcol,XmNnumChildren,
			&num_children,XmNchildren,&child_list,NULL);
		if (Computed_field_is_type_finite_element(field))
		{
			Computed_field_get_type_finite_element(field, &fe_field);
		}
		else
		{
			if (Computed_field_is_type_cmiss_number(field))
			{
				cmiss_number_string = Computed_field_evaluate_as_string_at_node(field,
					/*component_number*/-1, node, time);
			}
			else
			{
				if (ALLOCATE(values, FE_value, number_of_components))
				{
					if (!Computed_field_evaluate_at_node(field, node, time, values))
					{
						DEALLOCATE(values);
					}
					Computed_field_clear_cache(field);
				}
			}
		}
		if (fe_field || cmiss_number_string || values)
		{
			for (i = 0; i < num_children; i++)
			{
				text_field_widget = child_list[i];
				/* get the index */
				XtVaGetValues(text_field_widget,
					XmNuserData, &nodal_value_information,NULL);
				if (nodal_value_information)
				{
					/* get the current string from the widget so we don't overwrite it
						 when it is not changing - this allows cut & paste to work */
					if (old_value_string = XmTextFieldGetString(text_field_widget))
					{
						new_value_string = (char *)NULL;
						if (fe_field)
						{
							fe_field_component.field = fe_field;
							fe_field_component.number =
								nodal_value_information->component_number;
							switch (get_FE_field_value_type(fe_field))
							{
								case ELEMENT_XI_VALUE:
								{
									char element_char, xi_string[30];
									FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
									struct FE_element *element;

									if (get_FE_nodal_element_xi_value(
										node_field_viewer->current_node, fe_field,
										nodal_value_information->component_number,
										nodal_value_information->version,
										nodal_value_information->type, &element, xi))
									{
										switch (element->cm.type)
										{
											case CM_FACE:
											{
												element_char = 'F';
											} break;
											case CM_LINE:
											{
												element_char = 'L';
											} break;
											default:
											{
												element_char = 'E';
											} break;
										}
										sprintf(temp_value_string, " %c %d %d", element_char,
											element->cm.number, element->shape->dimension);
										for (j = 0; j < element->shape->dimension; j++)
										{
											sprintf(xi_string," %g",xi[j]);
											strcat(temp_value_string, xi_string);
										}
										new_value_string = duplicate_string(temp_value_string);
									}
								} break;
								case FE_VALUE_VALUE:
								{
									FE_value fe_value_value;

									get_FE_nodal_FE_value_value(node_field_viewer->current_node,
										&fe_field_component,nodal_value_information->version,
										nodal_value_information->type,time,&fe_value_value);
									sprintf(temp_value_string, FE_VALUE_INPUT_STRING,
										fe_value_value);
									new_value_string = duplicate_string(temp_value_string);
								} break;
								case INT_VALUE:
								{
									int int_value;

									get_FE_nodal_int_value(node_field_viewer->current_node,
										&fe_field_component,nodal_value_information->version,
										nodal_value_information->type,time,&int_value);
									sprintf(temp_value_string, "%d", int_value);
									new_value_string = duplicate_string(temp_value_string);
								} break;
								case STRING_VALUE:
								{
									get_FE_nodal_string_value(node_field_viewer->current_node,
										fe_field,nodal_value_information->component_number,
										nodal_value_information->version,
										nodal_value_information->type, &new_value_string);
								} break;
								default:
								{
									display_message(ERROR_MESSAGE,
										"node_field_viewer_widget_update_values.  "
										"Unsupported value_type for FE_field");
								} break;
							}
						}
						else if (cmiss_number_string)
						{
							/* copy and clear cmiss_number_string to avoid allocate */
							new_value_string = cmiss_number_string;
							cmiss_number_string = (char *)NULL;
						}
						else /* all other types of computed field */
						{
							sprintf(temp_value_string, FE_VALUE_INPUT_STRING,
								values[nodal_value_information->component_number]);
							new_value_string = duplicate_string(temp_value_string);
						}
						/* only change text if different */
						if (new_value_string)
						{
							if (strcmp(new_value_string, old_value_string))
							{
								XmTextFieldSetString(text_field_widget, new_value_string);
							}
							DEALLOCATE(new_value_string);
						}
						else
						{
							XmTextFieldSetString(text_field_widget, "ERROR");
						}
						XtFree(old_value_string);
					}
					else
					{
						XmTextFieldSetString(text_field_widget, "ERROR");
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"node_field_viewer_widget_update_values.  Failed");
		}
		if (cmiss_number_string)
		{
			DEALLOCATE(cmiss_number_string);
		}
		if (values)
		{
			DEALLOCATE(values);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_field_viewer_widget_update_values.  Invalid argument(s)");
	}
	LEAVE;
} /* node_field_viewer_widget_update_values */

static int node_field_viewer_widget_time_change_callback(
	struct Time_object *time_object, double current_time,
	void *node_field_viewer_void)
/*******************************************************************************
LAST MODIFIED : 5 December 2001

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Node_field_viewer_widget_struct *node_field_viewer;

	ENTER(node_field_viewer_widget_time_change_callback);
	USE_PARAMETER(current_time);
	if(time_object && (node_field_viewer = 
			(struct Node_field_viewer_widget_struct *)node_field_viewer_void))
	{
		node_field_viewer_widget_update_values(node_field_viewer);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_field_viewer_widget_time_change_callback.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return(return_code);
} /* node_field_viewer_widget_time_change_callback */

static void node_field_viewer_widget_destroy_CB(Widget widget,
	void *node_field_viewer_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Callback for when the node_field_viewer widget is destroyed. Tidies up all
dynamic memory allocations and pointers.
==============================================================================*/
{
	struct Node_field_viewer_widget_struct *node_field_viewer;

	ENTER(node_field_viewer_widget_destroy_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (node_field_viewer=
		(struct Node_field_viewer_widget_struct *)node_field_viewer_void)
	{
		if (node_field_viewer->nodal_value_types)
		{
			DEALLOCATE(node_field_viewer->nodal_value_types);
		}
		if (node_field_viewer->time_object_callback)
		{
			Time_object_remove_callback(node_field_viewer->time_object,
				node_field_viewer_widget_time_change_callback,
				(void *)node_field_viewer);
		}					
		DEACCESS(Time_object)(&(node_field_viewer->time_object));
		*(node_field_viewer->widget_address)=(Widget)NULL;
		DEALLOCATE(node_field_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_field_viewer_widget_destroy_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* node_field_viewer_widget_destroy_CB */

static void node_field_viewer_widget_value_destroy_CB(Widget widget,
	void *user_data,void *call_data)
/*******************************************************************************
LAST MODIFIED : 22 May 2000

DESCRIPTION :
Cleans up the nodal_value_information user_data for the node value text field.
==============================================================================*/
{
	struct Nodal_value_information *nodal_value_information;

	ENTER(node_field_viewer_widget_value_destroy_CB);
	USE_PARAMETER(user_data);
	USE_PARAMETER(call_data);
	nodal_value_information=(struct Nodal_value_information *)NULL;
	XtVaGetValues(widget,XmNuserData,&nodal_value_information,NULL);
	DEALLOCATE(nodal_value_information);
	LEAVE;
} /* node_field_viewer_widget_value_destroy_CB */

static void node_field_viewer_widget_value_CB(Widget widget,
	void *node_field_viewer_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Called when the user has changed the data in the text widget.  Processes the
data, and then changes the correct value in the array structure.
==============================================================================*/
{
	char *value_string;
	FE_value time;
	struct Computed_field *field;
	struct FE_field *fe_field;
	struct FE_field_component fe_field_component;
	struct FE_node *node;
	struct Nodal_value_information *nodal_value_information;
	struct Node_field_viewer_widget_struct *node_field_viewer;

	ENTER(node_field_viewer_value_CB);
	USE_PARAMETER(call_data);
	nodal_value_information=(struct Nodal_value_information *)NULL;
	XtVaGetValues(widget,XmNuserData,&nodal_value_information,NULL);
	if (nodal_value_information&&(node_field_viewer=
		(struct Node_field_viewer_widget_struct *)node_field_viewer_void)&&
		(node=node_field_viewer->current_node)&&
		(field=nodal_value_information->field))
	{
		/* check if field is the current field for the widget, since after choosing
			 the next field in the node_viewer, focus returns to the text_field then
			 is lost as it is destroyed, causing another callback here */
		time = Time_object_get_current_time(node_field_viewer->time_object);
		if (field == node_field_viewer->current_field)
		{
			if (value_string=XmTextFieldGetString(widget))
			{
				if (Computed_field_is_type_finite_element(field))
				{
					Computed_field_get_type_finite_element(field,&fe_field);
					switch (get_FE_field_value_type(fe_field))
					{
						case ELEMENT_XI_VALUE:
						{
							display_message(ERROR_MESSAGE,
								"Cannot set element:xi values yet");
						} break;
						case FE_VALUE_VALUE:
						{
							FE_value fe_value_value;

							sscanf(value_string,FE_VALUE_INPUT_STRING,&fe_value_value);
							fe_field_component.field=fe_field;
							fe_field_component.number=
								nodal_value_information->component_number;
							set_FE_nodal_FE_value_value(node_field_viewer->current_node,
								&fe_field_component,nodal_value_information->version,
								nodal_value_information->type, time, fe_value_value);
						} break;
						case INT_VALUE:
						{
							int int_value;

							sscanf(value_string,"%d",&int_value);
							fe_field_component.field=fe_field;
							fe_field_component.number=
								nodal_value_information->component_number;
							set_FE_nodal_int_value(node_field_viewer->current_node,
								&fe_field_component,nodal_value_information->version,
								nodal_value_information->type,time, int_value);
						} break;
						case STRING_VALUE:
						{
							display_message(WARNING_MESSAGE,"Cannot set string values yet");
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"node_field_viewer_widget_update_values.  "
								"Unsupported value_type for FE_field");
						} break;
					}
				}
				else
				{
					if (Computed_field_is_type_cmiss_number(field))
					{
						display_message(WARNING_MESSAGE,
							"CMISS number cannot be changed here");
					}
					else
					{
						FE_value *values;
						int number_of_components;
						
						number_of_components=Computed_field_get_number_of_components(field);
						if (ALLOCATE(values,FE_value,number_of_components))
						{
							if (Computed_field_evaluate_at_node(field,node,time,values))
							{
								sscanf(value_string,FE_VALUE_INPUT_STRING,
									&values[nodal_value_information->component_number]);
								Computed_field_set_values_at_node(field,node,values);
							}
							Computed_field_clear_cache(field);
							DEALLOCATE(values);
						}
					}
				}
				XtFree(value_string);
			}
			/* refresh value shown in the text field widgets */
			node_field_viewer_widget_update_values(node_field_viewer);
			/* inform any clients of the changes */
			node_field_viewer_widget_update(node_field_viewer);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_field_viewer_value_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* node_field_viewer_value_CB */

enum FE_nodal_value_type *get_FE_node_field_value_types(
	struct FE_node *node,struct FE_field *field,int *number_of_nodal_value_types,
	int *number_of_unknown_nodal_value_types)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns an array containing the list of named nodal value/derivative types
defined for any - but not necessarily all - components of the <field> at <node>.
Nodal value types are returned in ascending order.
On return <number_of_unknown_nodal_value_types> contains the number of nodal
values whose value_type was not recognized.
==============================================================================*/
{
	enum FE_nodal_value_type *component_nodal_value_types,nodal_value_type,
		*nodal_value_types,*temp_nodal_value_types;
	int i,j,k,l,number_of_components,number_of_derivatives,return_code;

	ENTER(get_FE_node_field_value_types);
	nodal_value_types=(enum FE_nodal_value_type *)NULL;
	if (node&&field&&number_of_nodal_value_types&&
		number_of_unknown_nodal_value_types)
	{
		*number_of_nodal_value_types=0;
		*number_of_unknown_nodal_value_types=0;
		return_code=1;
		number_of_components=get_FE_field_number_of_components(field);
		for (i=0;(i<number_of_components)&&return_code;i++)
		{
			number_of_derivatives=
				get_FE_node_field_component_number_of_derivatives(node,field,i);
			if (component_nodal_value_types=
				get_FE_node_field_component_nodal_value_types(node,field,i))
			{
				for (j=0;(j<=number_of_derivatives)&&return_code;j++)
				{
					nodal_value_type=component_nodal_value_types[j];
					/* ignore unknown types */
					if ((FE_NODAL_VALUE==nodal_value_type)||
						(FE_NODAL_D_DS1==nodal_value_type)||
						(FE_NODAL_D_DS2==nodal_value_type)||
						(FE_NODAL_D_DS3==nodal_value_type)||
						(FE_NODAL_D2_DS1DS2==nodal_value_type)||
						(FE_NODAL_D2_DS1DS3==nodal_value_type)||
						(FE_NODAL_D2_DS2DS3==nodal_value_type)||
						(FE_NODAL_D3_DS1DS2DS3==nodal_value_type))
					{
						/* search to find this type */
						k=0;
						while ((k < (*number_of_nodal_value_types))&&
							(nodal_value_type > nodal_value_types[k]))
						{
							k++;
						}
						if ((k >= (*number_of_nodal_value_types))||
							(nodal_value_type != nodal_value_types[k]))
						{
							/* add to list in numerical order */
							if (REALLOCATE(temp_nodal_value_types,nodal_value_types,
								enum FE_nodal_value_type,(*number_of_nodal_value_types)+1))
							{
								nodal_value_types=temp_nodal_value_types;
								for (l = *number_of_nodal_value_types;l>k;l--)
								{
									nodal_value_types[l]=nodal_value_types[l-1];
								}
								nodal_value_types[k]=nodal_value_type;
								(*number_of_nodal_value_types)++;
							}
							else
							{
								display_message(ERROR_MESSAGE,"get_FE_node_field_value_types.  "
									"Could not reallocate nodal_values_information");
								DEALLOCATE(nodal_value_types);
								*number_of_nodal_value_types = 0;
								return_code=0;
							}
						}
					}
					else
					{
						(*number_of_unknown_nodal_value_types)++;
					}
				}
				DEALLOCATE(component_nodal_value_types);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"get_FE_node_field_value_types.  Could not get nodal_value_types");
				DEALLOCATE(nodal_value_types);
				*number_of_nodal_value_types = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_node_field_value_types.  Invalid argument(s)");
	}
	LEAVE;

	return (nodal_value_types);
} /* get_FE_node_field_value_types */

static int node_field_viewer_widget_setup_components(
	struct Node_field_viewer_widget_struct *node_field_viewer)
/*******************************************************************************
LAST MODIFIED : 5 December 2000

DESCRIPTION :
Creates the array of cells containing field component values and derivatives
and their labels.
==============================================================================*/
{
	Arg args[4];
	char *component_name;
	enum FE_nodal_value_type *component_nodal_value_types,nodal_value_type;
	int col,comp_no,k,num_args,number_of_components,number_of_derivatives,
		number_of_unknown_nodal_value_types,return_code;
	struct Computed_field *field;
	struct FE_field *fe_field;
	struct FE_node *node;
	struct Nodal_value_information *nodal_value_information;
	Widget widget;
	XmString new_string;

	ENTER(node_field_viewer_setup_components);
	if (node_field_viewer)
	{
		return_code=1;
		node_field_viewer->number_of_components=-1;
		if (node_field_viewer->component_rowcol)
		{
			/* unmanage component_rowcol to avoid geometry requests, then destroy */
			XtUnmanageChild(node_field_viewer->component_rowcol);
			XtDestroyWidget(node_field_viewer->component_rowcol);
			/* must clear the pointer! */
			node_field_viewer->component_rowcol = (Widget)NULL;
		}
		if ((node=node_field_viewer->current_node)&&
			(field=node_field_viewer->current_field)&&
			Computed_field_is_defined_at_node(field,node))
		{
			number_of_components=Computed_field_get_number_of_components(field);
			node_field_viewer->number_of_components=number_of_components;
			if (node_field_viewer->nodal_value_types)
			{
				DEALLOCATE(node_field_viewer->nodal_value_types);
			}
			/* Computed_fields default to 1 nodal_value_type */
			node_field_viewer->number_of_nodal_value_types=1;
			/* now create another */
			XtSetArg(args[0],XmNpacking,XmPACK_COLUMN);
			XtSetArg(args[1],XmNorientation,XmHORIZONTAL);
			XtSetArg(args[2],XmNentryAlignment,XmALIGNMENT_CENTER);
			XtSetArg(args[3],XmNnumColumns,number_of_components+1);
			if (node_field_viewer->component_rowcol=XmCreateRowColumn(
				node_field_viewer->widget,"node_field_viewer_rowcol",args,4))
			{
				XtManageChild(node_field_viewer->component_rowcol);
				if ((!Computed_field_is_type_finite_element(field))||
					(Computed_field_get_type_finite_element(field,&fe_field)&&
						(node_field_viewer->nodal_value_types=get_FE_node_field_value_types(
							node,fe_field,&(node_field_viewer->number_of_nodal_value_types),
							&number_of_unknown_nodal_value_types))))
				{
					if (Computed_field_is_type_finite_element(field)&&
						(0<number_of_unknown_nodal_value_types))
					{
						display_message(WARNING_MESSAGE,"Unknown nodal derivative types");
					}
					component_nodal_value_types=(enum FE_nodal_value_type *)NULL;
					for (comp_no=0;(comp_no<=number_of_components)&&return_code;comp_no++)
					{
						if (0 < comp_no)
						{
							if (Computed_field_is_type_finite_element(field))
							{
								number_of_derivatives=
									get_FE_node_field_component_number_of_derivatives(
										node,fe_field,comp_no-1);
								if (!(component_nodal_value_types=
									get_FE_node_field_component_nodal_value_types(
										node,fe_field,comp_no-1)))
								{
									return_code=0;
								}
							}
							else
							{
								number_of_derivatives=0;
							}
						}
						for (col=0;col<=(node_field_viewer->number_of_nodal_value_types)&&
									 return_code;col++)
						{
							nodal_value_information=(struct Nodal_value_information *)NULL;
							new_string=(XmString)NULL;
							if (0 == col)
							{
								nodal_value_type=FE_NODAL_UNKNOWN;
							}
							else
							{
								if (node_field_viewer->nodal_value_types)
								{
									nodal_value_type=node_field_viewer->nodal_value_types[col-1];
								}
								else
								{
									nodal_value_type=FE_NODAL_VALUE;
								}
							}
							if (0 == comp_no)
							{
								/* nodal value type labels; note blank cell in the corner */
								if (0 < col)
								{
									new_string=XmStringCreateSimple(
										ENUMERATOR_STRING(FE_nodal_value_type)(nodal_value_type));
								}
							}
							else
							{
								if (0 == col)
								{
									/* component label */
									if ((Computed_field_is_type_finite_element(field)&&
										(component_name=get_FE_field_component_name(fe_field,
											comp_no-1)))||
										(component_name=
											Computed_field_get_component_name(field,comp_no-1)))
									{
										new_string=XmStringCreateSimple(component_name);
										DEALLOCATE(component_name);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"node_field_viewer_widget_setup_components.  "
											"Could not get field component name");
										return_code=0;
									}
								}
								else
								{
									/* work out if nodal_value_type defined for this component */
									k=0;
									if (component_nodal_value_types)
									{
										while ((k<=number_of_derivatives)&&
											(nodal_value_type != component_nodal_value_types[k]))
										{
											k++;
										}
									}
									if (k<=number_of_derivatives)
									{
										if (ALLOCATE(nodal_value_information,
											struct Nodal_value_information,1))
										{
											nodal_value_information->field=field;
											nodal_value_information->component_number=comp_no-1;
											nodal_value_information->type=nodal_value_type;
											nodal_value_information->version=0;
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"node_field_viewer_widget_setup_components.  "
												"Could not allocate nodal_value_information");
											return_code=0;
										}
									}
								}
							}
							if (return_code)
							{
								if (nodal_value_information)
								{
									XtSetArg(args[0],XmNuserData,nodal_value_information);
									XtSetArg(args[1],XmNeditMode,XmSINGLE_LINE_EDIT);
									/* string and element_xi fields should be shown wider */
									if (Computed_field_is_type_finite_element(field)&&(
										(ELEMENT_XI_VALUE==get_FE_field_value_type(fe_field))||
										(STRING_VALUE==get_FE_field_value_type(fe_field))))
									{
										XtSetArg(args[2],XmNcolumns,20);
									}
									else
									{
										XtSetArg(args[2],XmNcolumns,10);
									}
									if (widget=XmCreateTextField(
										node_field_viewer->component_rowcol,"value",args,3))
									{
										/* add callbacks for data input, destroy */
										XtAddCallback(widget,XmNdestroyCallback,
											node_field_viewer_widget_value_destroy_CB,
											(XtPointer)NULL);
										XtAddCallback(widget,XmNactivateCallback,
											node_field_viewer_widget_value_CB,
											(XtPointer)node_field_viewer);
										XtAddCallback(widget,XmNlosingFocusCallback,
											node_field_viewer_widget_value_CB,
											(XtPointer)node_field_viewer);
										XtManageChild(widget);
									}
									else
									{
										DEALLOCATE(nodal_value_information);
										display_message(ERROR_MESSAGE,
											"node_field_viewer_widget_setup_components.  "
											"Could not create text field widget");
										return_code=0;
									}
								}
								else
								{
									if (new_string)
									{
										num_args=2;
										XtSetArg(args[0],XmNlabelString,new_string);
										XtSetArg(args[1],XmNalignment,XmALIGNMENT_CENTER);
									}
									else
									{
										num_args=0;
									}
									if (widget=XmCreateLabelGadget(
										node_field_viewer->component_rowcol,"",args,num_args))
									{
										XtManageChild(widget);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"node_field_viewer_widget_setup_components.  "
											"Could not create label gadget");
										return_code=0;
									}
								}
							}
						}
						if (new_string)
						{
							XmStringFree(new_string);
						}
						if (component_nodal_value_types)
						{
							DEALLOCATE(component_nodal_value_types);
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"node_field_viewer_widget_setup_components.  "
						"Could not get nodal value types");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"node_field_viewer_widget_setup_components.  "
					"Could not make component_rowcol");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_field_viewer_widget_setup_components.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* node_field_viewer_widget_setup_components */

/*
Global functions
----------------
*/

Widget create_node_field_viewer_widget(Widget *node_field_viewer_widget_address,
	Widget parent,struct FE_node *node,struct Computed_field *field,
	struct Time_object *time_object)
/*******************************************************************************
LAST MODIFIED : 22 November 2001

DESCRIPTION :
Widget for displaying and editing computed field components/derivatives at a
node.
==============================================================================*/
{
	Arg args[6];
	struct Node_field_viewer_widget_struct *node_field_viewer;
	Widget return_widget;

	ENTER(create_node_field_viewer_widget);
	return_widget=(Widget)NULL;
	if (node_field_viewer_widget_address&&parent)
	{
		/* allocate memory */
		if (ALLOCATE(node_field_viewer,struct Node_field_viewer_widget_struct,1))
		{
			/* initialise the structure */
			node_field_viewer->nodal_value_types=(enum FE_nodal_value_type *)NULL;
			node_field_viewer->number_of_nodal_value_types=0;
			node_field_viewer->number_of_components=-1;
			node_field_viewer->current_node=(struct FE_node *)NULL;
			node_field_viewer->current_field=(struct Computed_field *)NULL;
			node_field_viewer->update_callback.procedure=(Callback_procedure *)NULL;
			node_field_viewer->update_callback.data=NULL;
			node_field_viewer->time_object = ACCESS(Time_object)(time_object);
			node_field_viewer->time_object_callback = 0;
			/* initialise widgets */
			node_field_viewer->component_rowcol=(Widget)NULL;
			node_field_viewer->widget=(Widget)NULL;
			node_field_viewer->widget_address=node_field_viewer_widget_address;
			node_field_viewer->widget_parent=parent;
			/* create the main scrolled window */
			XtSetArg(args[0],XmNleftAttachment,XmATTACH_FORM);
			XtSetArg(args[1],XmNrightAttachment,XmATTACH_FORM);
			XtSetArg(args[2],XmNtopAttachment,XmATTACH_FORM);
			XtSetArg(args[3],XmNbottomAttachment,XmATTACH_FORM);
			XtSetArg(args[4],XmNscrollingPolicy,XmAUTOMATIC);
			XtSetArg(args[5],XmNuserData,node_field_viewer);
			if (node_field_viewer->widget=XmCreateScrolledWindow(parent,
				"node_field_viewer_widget",args,6))
			{
				/* add destroy callback for widget */
				XtAddCallback(node_field_viewer->widget,XmNdestroyCallback,
					node_field_viewer_widget_destroy_CB,(XtPointer)node_field_viewer);
				/* create the components of the field */
				node_field_viewer_widget_set_node_field(node_field_viewer->widget,
					node,field);
				XtManageChild(node_field_viewer->widget);
				return_widget=node_field_viewer->widget;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_node_field_viewer_widget.  Could not create widget");
				DEALLOCATE(node_field_viewer);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_node_field_viewer_widget.  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_node_field_viewer_widget.  Invalid argument(s)");
	}
	if (node_field_viewer_widget_address)
	{
		*node_field_viewer_widget_address=return_widget;
	}
	LEAVE;

	return (return_widget);
} /* create_node_field_viewer_widget */

int node_field_viewer_widget_set_callback(Widget node_field_viewer_widget,
	struct Callback_data *callback)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets the callback for updates when the field of the node in the viewer has been
modified.
==============================================================================*/
{
	int return_code;
	struct Node_field_viewer_widget_struct *node_field_viewer;

	ENTER(node_field_viewer_widget_set_callback);
	if (node_field_viewer_widget&&callback)
	{
		node_field_viewer=(struct Node_field_viewer_widget_struct *)NULL;
		/* get the node_field_viewer structure from the widget */
		XtVaGetValues(node_field_viewer_widget,XmNuserData,&node_field_viewer,NULL);
		if (node_field_viewer)
		{
			node_field_viewer->update_callback.procedure=callback->procedure;
			node_field_viewer->update_callback.data=callback->data;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"node_field_viewer_widget_set_callback.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_field_viewer_widget_set_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* node_field_viewer_widget_set_callback */

int node_field_viewer_widget_get_node_field(Widget node_field_viewer_widget,
	struct FE_node **node,struct Computed_field **field)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns the node/field being edited in the <node_field_viewer_widget>.
==============================================================================*/
{
	int return_code;
	struct Node_field_viewer_widget_struct *node_field_viewer;

	ENTER(node_field_viewer_widget_get_node_field);
	if (node_field_viewer_widget&&node&&field)
	{
		node_field_viewer=(struct Node_field_viewer_widget_struct *)NULL;
		/* get the node_field_viewer structure from the widget */
		XtVaGetValues(node_field_viewer_widget,XmNuserData,&node_field_viewer,NULL);
		if (node_field_viewer)
		{
			*node = node_field_viewer->current_node;
			*field = node_field_viewer->current_field;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"node_field_viewer_widget_get_node_field.  Missing widget data");
			*node = (struct FE_node *)NULL;
			*field = (struct Computed_field *)NULL;
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_field_viewer_widget_get_node_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* node_field_viewer_widget_get_node_field */

int node_field_viewer_widget_set_node_field(Widget node_field_viewer_widget,
	struct FE_node *node,struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets the node/field being edited in the <node_field_viewer_widget>. Note that
the viewer works on the node itself, not a local copy. Hence, only pass
unmanaged nodes to this widget.
==============================================================================*/
{
	int return_code,setup_components;
	struct Node_field_viewer_widget_struct *node_field_viewer;

	ENTER(node_field_viewer_widget_set_node_field);
	if (node_field_viewer_widget&&((!node)||(!field)||
		Computed_field_is_defined_at_node(field,node)))
	{
		node_field_viewer=(struct Node_field_viewer_widget_struct *)NULL;
		/* get the node_field_viewer structure from the widget */
		XtVaGetValues(node_field_viewer_widget,XmNuserData,&node_field_viewer,NULL);
		if (node_field_viewer)
		{
			/* rebuild viewer widgets if nature of node or field changed */
			if ((!node)||(!(node_field_viewer->current_node))||
				(!equivalent_computed_fields_at_nodes(node,
					node_field_viewer->current_node))||
				(field != node_field_viewer->current_field)||
				(field&&(node_field_viewer->number_of_components !=
					Computed_field_get_number_of_components(field))))
			{
				setup_components=1;
			}
			else
			{
				setup_components=0;
			}
			if (node&&field)
			{
				node_field_viewer->current_node=node;
				node_field_viewer->current_field=field;
			}
			else
			{
				node_field_viewer->current_node=(struct FE_node *)NULL;
				node_field_viewer->current_field=(struct Computed_field *)NULL;
			}
			if (setup_components)
			{
				node_field_viewer_widget_setup_components(node_field_viewer);
			}
			if (node&&field)
			{
				if (node_field_viewer->time_object)
				{
					if (Computed_field_has_multiple_times(field))
					{
						if (!node_field_viewer->time_object_callback)
						{
							node_field_viewer->time_object_callback = 
								Time_object_add_callback(node_field_viewer->time_object,
									node_field_viewer_widget_time_change_callback,
									(void *)node_field_viewer);
						}
					}
					else
					{
						if (node_field_viewer->time_object_callback)
						{
							Time_object_remove_callback(node_field_viewer->time_object,
								node_field_viewer_widget_time_change_callback,
								(void *)node_field_viewer);
							node_field_viewer->time_object_callback = 0;
						}					
					}
				}
				node_field_viewer_widget_update_values(node_field_viewer);
				XtManageChild(node_field_viewer->widget);
			}
			else
			{
				XtUnmanageChild(node_field_viewer->widget);
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"node_field_viewer_widget_set_node_field.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_field_viewer_widget_set_node_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* node_field_viewer_widget_set_node_field */
