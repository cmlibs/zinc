/*******************************************************************************
FILE : node_field_editor.c

LAST MODIFIED : 21 September 1999

DESCRIPTION :
This module creates a node_field editor widget.  A node_field editor widget
consists of multiple node field component widgets.
The node_field widget acts as a 'window' on the data - and cannot actually be
changed.
==============================================================================*/
#include <math.h>
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/Text.h>
#include "finite_element/finite_element.h"
#include "general/debug.h"
#include "node/node_field_editor.h"
#include "node/node_field_editor.uidh"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module types
------------
*/
struct Nodal_value_information
{
	enum FE_nodal_value_type type;
	int version;
	struct FE_field_component field_component;
}; /* struct Nodal_value_information */

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int node_field_editor_hierarchy_open=0;
static MrmHierarchy node_field_editor_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
static void node_field_editor_update(
	struct Node_field_editor_struct *temp_node_field_editor)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Tells CMGUI about the current values. Returns a pointer to the node field.
==============================================================================*/
{
	ENTER(node_field_editor_update);
	if (temp_node_field_editor->callback_array[NODE_FIELD_EDITOR_UPDATE_CB].
		procedure)
	{
		/* now call the procedure with the user data and a NULL pointer */
		(temp_node_field_editor->callback_array[NODE_FIELD_EDITOR_UPDATE_CB].
			procedure)(temp_node_field_editor->widget,temp_node_field_editor->
			callback_array[NODE_FIELD_EDITOR_UPDATE_CB].data,NULL);
	}
	LEAVE;
} /* node_field_editor_update */

static void node_field_editor_identify_button(Widget w, int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
Finds the id of the buttons on the node_field_editor widget.
==============================================================================*/
{
	struct Node_field_editor_struct *temp_node_field_editor;

	ENTER(node_field_editor_identify_button);
	USE_PARAMETER(reason);
	/* find out which node_field_editor widget we are in */
	XtVaGetValues(w,XmNuserData,&temp_node_field_editor,NULL);
	switch (button_num)
	{
		case node_field_editor_comp_form_ID:
		{
			temp_node_field_editor->comp_form=w;
		} break;
#if defined (NODE_FIELD_EDITOR_NAME)
		case node_field_editor_name_ID:
		{
			temp_node_field_editor->name=w;
		} break;
#endif
		case node_field_editor_component_ID:
		{
			temp_node_field_editor->component_rowcol=w;
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"node_field_editor_identify_button.  Invalid button number");
		} break;
	}
	LEAVE;
} /* node_field_editor_identify_button */

static void node_field_editor_destroy_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Callback for the node_field_editor dialog - tidies up all details - mem etc
==============================================================================*/
{
	struct Node_field_editor_struct *temp_node_field_editor;

	ENTER(node_field_editor_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* Get the pointer to the data for the node_field_editor widget */
		XtVaGetValues(widget,XmNuserData,&temp_node_field_editor,NULL);
		if (temp_node_field_editor)
		{
			*(temp_node_field_editor->widget_address)=(Widget)NULL;
			/* deallocate the memory for the user data */
			DEALLOCATE(temp_node_field_editor);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"node_field_editor_destroy_CB.  Missing node_field_editor");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_field_editor_destroy_CB.  Missing widget");
	}
	LEAVE;
} /* node_field_editor_destroy_CB */

#if defined (NODE_FIELD_EDITOR_NAME)
static void node_field_editor_set_name(
	struct Node_field_editor_struct *temp_node_field_editor)
/*******************************************************************************
LAST MODIFIED : 1 May 1994

DESCRIPTION :
Writes the correct name on the label.
==============================================================================*/
{
	XmString temp_label;

	ENTER(node_field_editor_set_name);
	temp_label=
		XmStringCreateSimple(temp_node_field_editor->global_value->field->name);
	XtVaSetValues(temp_node_field_editor->name,XmNlabelString,temp_label,NULL);
	XmStringFree(temp_label);
	LEAVE;
} /* node_field_editor_set_name */
#endif /* defined (NODE_FIELD_EDITOR_NAME) */

static void node_field_editor_update_values(
	struct Node_field_editor_struct *temp_node_field_editor)
/*******************************************************************************
LAST MODIFIED : 3 November 1998

DESCRIPTION :
Updates all widgets in the rowcol to make sure they say the correct value.
==============================================================================*/
{
	char temp_str[TEMP_STRING_SIZE];
	FE_value value;
	int i,num_children;
	struct Nodal_value_information *nodal_value_information;
	Widget *child_list;

	ENTER(node_field_editor_update_values);
	/* get children of the rowcol */
	XtVaGetValues(temp_node_field_editor->component_rowcol,XmNnumChildren,
		&num_children,XmNchildren,&child_list,NULL);
	for (i=0;i<num_children;i++)
	{
		/* get the index */
		XtVaGetValues(child_list[i],XmNuserData,&nodal_value_information,NULL);
		if (nodal_value_information)
		{
			get_FE_nodal_FE_value_value(temp_node_field_editor->node,
				&(nodal_value_information->field_component),
				nodal_value_information->version,nodal_value_information->type,
				&value);
			/* is it a text widget? */
			sprintf(temp_str,FE_VALUE_INPUT_STRING,value);
			XmTextSetString(child_list[i],temp_str);
		}
	}
	LEAVE;
} /* node_field_editor_update_value */

static void node_field_editor_value_CB(Widget w,int *tag,
	XmAnyCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 3 November 1998

DESCRIPTION :
Called when the user has changed the data in the text widget.  Processes the
data, and then changes the correct value in the array structure.
==============================================================================*/
{
	char *temp_str_ptr;
	FE_value temp_value;
	struct Nodal_value_information *nodal_value_information;
	struct Node_field_editor_struct *temp_node_field_editor;

	ENTER(node_field_editor_value_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the node_field_editor dialog */
	XtVaGetValues(XtParent(w),XmNuserData,&temp_node_field_editor,NULL);
	XtVaGetValues(w,XmNuserData,&nodal_value_information,NULL);
	/* get the string */
	XtVaGetValues(w,XmNvalue,&temp_str_ptr,NULL);
	sscanf(temp_str_ptr,FE_VALUE_INPUT_STRING,&temp_value);
	XtFree(temp_str_ptr);
	set_FE_nodal_FE_value_value(temp_node_field_editor->node,
		&(nodal_value_information->field_component),
		nodal_value_information->version,nodal_value_information->type,
		temp_value);
	/* now write the correct values to all the sub-widgets */
	node_field_editor_update_values(temp_node_field_editor);
	/* inform any clients */
	node_field_editor_update(temp_node_field_editor);
	LEAVE;
} /* node_field_editor_value_CB */

struct Count_nodal_values_data
{
	enum FE_nodal_value_type *nodal_value_types;
	int number_of_nodal_values,num_widgets,unknown_count;
	struct Node_field_editor_struct *node_field_editor;
	Widget *widgets;
}; /* struct Count_nodal_values_data */

static int count_nodal_values(struct FE_node *node,struct FE_field *field,
	void *count_nodal_values_data_void)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
==============================================================================*/
{
	enum FE_nodal_value_type nodal_value_type,*nodal_value_types,
		*temp_nodal_value_types;
	int i,j,k,l,number_of_components,number_of_derivatives,return_code;
	struct Count_nodal_values_data *count_nodal_values_data;

	ENTER(count_nodal_values);
	/* check arguments */
	if (node&&field&&(count_nodal_values_data=
		(struct Count_nodal_values_data *)count_nodal_values_data_void))
	{
		return_code=1;
		number_of_components=get_FE_field_number_of_components(field);
		for (i=0;(i<number_of_components)&&return_code;i++)
		{
			number_of_derivatives=
				get_FE_node_field_component_number_of_derivatives(node,field,i);
			if (nodal_value_types=
				get_FE_node_field_component_nodal_value_types(node,field,i))
			{
				for (j=0;(j<=number_of_derivatives)&&return_code;j++)
				{
					nodal_value_type=nodal_value_types[j];
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
						while ((k<count_nodal_values_data->number_of_nodal_values)&&
							(nodal_value_type>(count_nodal_values_data->nodal_value_types)[k]))
						{
							k++;
						}
						if ((k>=count_nodal_values_data->number_of_nodal_values)||
							(nodal_value_type!=(count_nodal_values_data->nodal_value_types)[k]))
						{
							/* add to list in numerical order */
							if (REALLOCATE(temp_nodal_value_types,
								count_nodal_values_data->nodal_value_types,
								enum FE_nodal_value_type,
								(count_nodal_values_data->number_of_nodal_values)+1))
							{
								count_nodal_values_data->nodal_value_types=
									temp_nodal_value_types;
								l=count_nodal_values_data->number_of_nodal_values;
								while (l>k)
								{
									temp_nodal_value_types[l]=temp_nodal_value_types[l-1];
									l--;
								}
								temp_nodal_value_types[k]=nodal_value_type;
								(count_nodal_values_data->number_of_nodal_values)++;
							}
							else
							{
								display_message(ERROR_MESSAGE,"count_nodal_values.  "
									"Could not reallocate nodal_values_information");
								return_code=0;
							}
						}
					}
					else
					{
						(count_nodal_values_data->unknown_count)++;
					}
				}
				DEALLOCATE(nodal_value_types);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"count_nodal_values.  Could not get nodal_value_types");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"count_nodal_values.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* count_nodal_values */

static int create_rows(struct FE_node *node,struct FE_field *field,
	void *count_nodal_values_data_void)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Writes the correct name on the label.
==============================================================================*/
{
	Arg override_arg;
	char *name;
	enum FE_nodal_value_type nodal_value_type,*nodal_value_types;
	int i,j,k,number_of_components,number_of_derivatives,return_code;
	MrmType node_field_editor_class;
	struct Count_nodal_values_data *count_nodal_values_data;
	struct Nodal_value_information *nodal_value_information;
	Widget widget;
	XmString new_string;

	ENTER(create_rows);
	if (node&&field&&(count_nodal_values_data=
		(struct Count_nodal_values_data *)count_nodal_values_data_void))
	{
		return_code=1;
		number_of_components=get_FE_field_number_of_components(field);
		for (i=0;(i<number_of_components)&&return_code;i++)
		{
			/* fetch the name widget */
			widget=(Widget)NULL;
			if (MrmSUCCESS==MrmFetchWidget(node_field_editor_hierarchy,
				"node_field_component_label",
				count_nodal_values_data->node_field_editor->component_rowcol,
				&widget,&node_field_editor_class))
			{
				(count_nodal_values_data->widgets)[count_nodal_values_data->
					num_widgets]=widget;
				(count_nodal_values_data->num_widgets)++;
				/* now we have to set the name */
				name=(char *)NULL;
				if (name=get_FE_field_component_name(field,i))
				{
					new_string=XmStringCreateSimple(name);
					XtVaSetValues(widget,XmNlabelString,new_string,NULL);
					XmStringFree(new_string);
					DEALLOCATE(name);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_rows.  Could not get component name");
					return_code=0;
				}
				/* fetch text and blank widgets, depending on whether or not the
					derivative is used */
				number_of_derivatives=
					get_FE_node_field_component_number_of_derivatives(node,field,i);
				if (nodal_value_types=
					get_FE_node_field_component_nodal_value_types(node,field,i))
				{
					for (j=0;(j<count_nodal_values_data->number_of_nodal_values)&&
						return_code;j++)
					{
						k=0;
						nodal_value_type=(count_nodal_values_data->nodal_value_types)[j];
						while ((k<=number_of_derivatives)&&
							(nodal_value_type != nodal_value_types[k]))
						{
							k++;
						}
						if ((k<=number_of_derivatives)&&
							(nodal_value_type==nodal_value_types[k]))
						{
							if (ALLOCATE(nodal_value_information,
								struct Nodal_value_information,1))
							{
								nodal_value_information->type=nodal_value_type;
								nodal_value_information->version=0;
								(nodal_value_information->field_component).field=field;
								(nodal_value_information->field_component).number=i;
								XtSetArg(override_arg,XmNuserData,nodal_value_information);
								widget=(Widget)NULL;
								if (MrmSUCCESS==MrmFetchWidgetOverride(
									node_field_editor_hierarchy,
									"node_field_component_value",
									count_nodal_values_data->node_field_editor->component_rowcol,
									NULL,&override_arg,1,&widget,&node_field_editor_class))
								{
									(count_nodal_values_data->widgets)[count_nodal_values_data->
										num_widgets]=widget;
									(count_nodal_values_data->num_widgets)++;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"create_rows.  Could not fetch text widget");
									return_code=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"create_rows.  Could not allocate nodal_value_information");
								return_code=0;
							}
						}
						else
						{
							widget=(Widget)NULL;
							if (MrmSUCCESS==MrmFetchWidget(
								node_field_editor_hierarchy,
								"node_field_component_label",
								count_nodal_values_data->node_field_editor->component_rowcol,
								&widget,&node_field_editor_class))
							{
								(count_nodal_values_data->widgets)[count_nodal_values_data->
									num_widgets]=widget;
								(count_nodal_values_data->num_widgets)++;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"create_rows.  Could not fetch blank widget");
								return_code=0;
							}
						}
					}
					DEALLOCATE(nodal_value_types);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_rows.  Could not get nodal value types");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_rows.  Could not fetch label widget");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_rows.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* create_rows */

static int node_field_editor_setup_components(
	struct Node_field_editor_struct *temp_node_field_editor)
/*******************************************************************************
LAST MODIFIED : 18 November 1998

DESCRIPTION :
Writes the correct name on the label.
==============================================================================*/
{
	Arg override_arg;
	char *nodal_value_name;
	int i,number_of_components,num_widgets,return_code;
	MrmType node_field_editor_class;
	struct Count_nodal_values_data count_nodal_values_data;
	Widget *temp_widget,rowcol_widget;
	XmString new_string;

	ENTER(node_field_editor_setup_components);
	return_code=0;
	if (node_field_editor_hierarchy_open)
	{
		/* destroy the main rowcol */
		XtUnmanageChild(temp_node_field_editor->component_rowcol);
		/* unmanaged so that it will not figure in geom requests */
		XtDestroyWidget(temp_node_field_editor->component_rowcol);
		/* now create another */
		XtSetArg(override_arg,XmNuserData,temp_node_field_editor);
		rowcol_widget=(Widget)NULL;
		if (MrmSUCCESS==MrmFetchWidgetOverride(node_field_editor_hierarchy,
			"node_field_editor_component",temp_node_field_editor->comp_form,NULL,
			&override_arg,1,&rowcol_widget,&node_field_editor_class))
		{
			/* rowcol_widget is component_rowcol, but would already have been
				assigned by MrmCreateCallback */
			XtManageChild(rowcol_widget);
			/* find out how many different derivatives there are */
			temp_node_field_editor->total_derivatives=0;
			if (temp_node_field_editor->global_value)
			{
				/* count the number of nodal values (ignoring unknown */
				count_nodal_values_data.number_of_nodal_values=0;
				count_nodal_values_data.nodal_value_types=
					(enum FE_nodal_value_type *)NULL;
				count_nodal_values_data.unknown_count=0;
				count_nodal_values_data.node_field_editor=temp_node_field_editor;
				if (return_code=for_FE_field_at_node(
					temp_node_field_editor->global_value,count_nodal_values,
					&count_nodal_values_data,temp_node_field_editor->node))
				{
					temp_node_field_editor->total_derivatives=
						count_nodal_values_data.number_of_nodal_values;
					if (0<count_nodal_values_data.unknown_count)
					{
						display_message(WARNING_MESSAGE,"Unknown nodal derivative types");
					}
					number_of_components=get_FE_field_number_of_components(
						temp_node_field_editor->global_value);
					XtVaSetValues(temp_node_field_editor->component_rowcol,XmNnumColumns,
						number_of_components+1,NULL);
					/* allocate memory for the widget list */
					if (ALLOCATE(temp_widget,Widget,
						(temp_node_field_editor->total_derivatives+1)*
						(number_of_components+1)))
					{
						num_widgets=0;
						/* start with a blank widget in the corner */
						temp_widget[num_widgets]=(Widget)NULL;
						if (MrmSUCCESS==MrmFetchWidget(node_field_editor_hierarchy,
							"node_field_component_label",
							temp_node_field_editor->component_rowcol,
							&temp_widget[num_widgets],&node_field_editor_class))
						{
							num_widgets++;
							/* now bring up all the names */
							i=0;
							while (return_code&&(i<temp_node_field_editor->total_derivatives))
							{
								temp_widget[num_widgets]=(Widget)NULL;
								if (MrmSUCCESS==MrmFetchWidget(node_field_editor_hierarchy,
									"node_field_component_label",
									temp_node_field_editor->component_rowcol,
									&temp_widget[num_widgets],&node_field_editor_class))
								{
									/* now we have to set the name */
									switch ((count_nodal_values_data.nodal_value_types)[i])
									{
										case FE_NODAL_VALUE:
										{
											nodal_value_name="value";
										} break;
										case FE_NODAL_D_DS1:
										{
											nodal_value_name="d/ds1";
										} break;
										case FE_NODAL_D_DS2:
										{
											nodal_value_name="d/ds2";
										} break;
										case FE_NODAL_D_DS3:
										{
											nodal_value_name="d/ds3";
										} break;
										case FE_NODAL_D2_DS1DS2:
										{
											nodal_value_name="d2/ds1ds2";
										} break;
										case FE_NODAL_D2_DS1DS3:
										{
											nodal_value_name="d2/ds1ds3";
										} break;
										case FE_NODAL_D2_DS2DS3:
										{
											nodal_value_name="d2/ds2ds3";
										} break;
										case FE_NODAL_D3_DS1DS2DS3:
										{
											nodal_value_name="d3/ds1ds2ds3";
										} break;
										default:
										{
											nodal_value_name="ERROR - unknown";
										} break;
									}
									new_string=XmStringCreateSimple(nodal_value_name);
									XtVaSetValues(temp_widget[num_widgets],XmNlabelString,
										new_string,NULL);
									XmStringFree(new_string);
									num_widgets++;
								}
								else
								{
									display_message(ERROR_MESSAGE,
					"node_field_editor_setup_components.  Could not fetch label widget");
									return_code=0;
								}
								i++;
							}
							/* now create all the rows */
							count_nodal_values_data.widgets=temp_widget;
							count_nodal_values_data.num_widgets=num_widgets;
							if (for_FE_field_at_node(temp_node_field_editor->global_value,
								create_rows,&count_nodal_values_data,temp_node_field_editor->
								node))
							{
								XtManageChildren(temp_widget,
									count_nodal_values_data.num_widgets);
							}
							DEALLOCATE(temp_widget);
							/* now write the correct values to all the sub-widgets */
/*			  	     XtManageChild(rowcol_widget); */
/*				       XtManageChild(XtParent(rowcol_widget)); */
/*???GMH.  Values may not necessarily be correct */
/*							node_field_editor_update_values(temp_node_field_editor);*/
						}
						else
						{
							display_message(ERROR_MESSAGE,
					"node_field_editor_setup_components.  Could not fetch blank widget");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
			"node_field_editor_setup_components.  Could not allocate widget memory");
						return_code=0;
					}
				}
				DEALLOCATE(count_nodal_values_data.nodal_value_types);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
			"node_field_editor_setup_components.  Could not fetch component widget");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_field_editor_setup_components.  Hierarchy not open");
	}
	LEAVE;

	return (return_code);
} /* node_field_editor_setup_components */

/*
Global functions
----------------
*/
Widget create_node_field_editor_widget(Widget *node_field_editor_widget,
	Widget parent,struct FE_field *init_data,struct FE_node *init_value)
/*******************************************************************************
LAST MODIFIED : 2 November 1998

DESCRIPTION :
Creates a node_field_editor widget that gets a position and orientation from the
user.
==============================================================================*/
{
	int i,init_widgets;
	MrmType node_field_editor_dialog_class;
	struct Node_field_editor_struct *temp_node_field_editor=NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"node_field_editor_id_button",
		(XtPointer)node_field_editor_identify_button},
		{"node_field_editor_value_CB",(XtPointer)node_field_editor_value_CB},
		{"node_field_editor_destroy_CB",(XtPointer)node_field_editor_destroy_CB},
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"Node_field_editor_structure",(XtPointer)NULL},
		{"node_field_editor_comp_form_ID",
			(XtPointer)node_field_editor_comp_form_ID},
#if defined (NODE_FIELD_EDITOR_NAME)
		{"node_field_editor_name_ID",(XtPointer)node_field_editor_name_ID},
#endif
		{"node_field_editor_component_ID",
			(XtPointer)node_field_editor_component_ID},
	};
	Widget return_widget;

	ENTER(create_node_field_editor_widget);
	/* check arguments */
	if (node_field_editor_widget&&parent)
	{
		/* need some serious checking on init_data */
		return_widget=(Widget)NULL;
		if (MrmOpenHierarchy_base64_string(node_field_editor_uidh,
			&node_field_editor_hierarchy,&node_field_editor_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(temp_node_field_editor,struct Node_field_editor_struct,1))
			{
				/* initialise the structure */
				temp_node_field_editor->widget_parent=parent;
				temp_node_field_editor->widget_address=node_field_editor_widget;
				temp_node_field_editor->widget=(Widget)NULL;
				temp_node_field_editor->comp_form=(Widget)NULL;
				temp_node_field_editor->component_rowcol=(Widget)NULL;
#if defined (NODE_FIELD_EDITOR_NAME)
				temp_node_field_editor->name=(Widget)NULL;
#endif /* defined (NODE_FIELD_EDITOR_NAME) */
				temp_node_field_editor->global_value=init_data;
				temp_node_field_editor->node=init_value;
				for (i=0;i<NODE_FIELD_EDITOR_NUM_CALLBACKS;i++)
				{
					temp_node_field_editor->callback_array[i].procedure=
						(Callback_procedure *)NULL;
					temp_node_field_editor->callback_array[i].data=NULL;
				}
				/* register the callbacks */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(node_field_editor_hierarchy,
					callback_list,XtNumber(callback_list)))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)temp_node_field_editor;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						node_field_editor_hierarchy,identifier_list,
						XtNumber(identifier_list)))
					{
						/* fetch position window widget */
						if (MrmSUCCESS==MrmFetchWidget(node_field_editor_hierarchy,
							"node_field_editor_widget",temp_node_field_editor->widget_parent,
							&(temp_node_field_editor->widget),
							&node_field_editor_dialog_class))
						{
							XtManageChild(temp_node_field_editor->widget);
							/* set the mode toggle to the correct position */
							init_widgets=1;
							if (init_widgets)
							{
#if defined (NODE_FIELD_EDITOR_NAME)
								/* set the name of the node_field_editor */
								node_field_editor_set_name(temp_node_field_editor);
#endif
								/* create the components of the field */
								node_field_editor_setup_components(temp_node_field_editor);
								return_widget=temp_node_field_editor->widget;
							}
							else
							{
								DEALLOCATE(temp_node_field_editor);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
	"create_node_field_editor_widget.  Could not fetch node_field_editor dialog");
							DEALLOCATE(temp_node_field_editor);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
						"create_node_field_editor_widget.  Could not register identifiers");
						DEALLOCATE(temp_node_field_editor);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_node_field_editor_widget.  Could not register callbacks");
					DEALLOCATE(temp_node_field_editor);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
"create_node_field_editor_widget.  Could not allocate node_field_editor widget structure");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_node_field_editor_widget.  Could not open hierarchy");
		}
		*node_field_editor_widget=return_widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_node_field_editor_widget.  Invalid argument(s)");
	}
	LEAVE;

	return (return_widget);
} /* create_node_field_editor_widget */

int node_field_editor_set_data(Widget node_field_editor_widget,
	enum Node_field_editor_data_type data_type,void *data)
/*******************************************************************************
LAST MODIFIED : 3 November 1998

DESCRIPTION :
Changes a data item of the node_field_editor widget.
==============================================================================*/
{
	int return_code;
	struct Node_field_editor_struct *temp_node_field_editor;

	ENTER(node_field_editor_set_data);
	/* Get the pointer to the data for the node_field_editor dialog */
	XtVaGetValues(node_field_editor_widget,XmNuserData,&temp_node_field_editor,
		NULL);
	switch (data_type)
	{
		case NODE_FIELD_EDITOR_UPDATE_CB:
		{
			temp_node_field_editor->callback_array[NODE_FIELD_EDITOR_UPDATE_CB].
				procedure=((struct Callback_data *)data)->procedure;
			temp_node_field_editor->callback_array[NODE_FIELD_EDITOR_UPDATE_CB].data=
				((struct Callback_data *)data)->data;
			return_code=1;
		} break;
		case NODE_FIELD_EDITOR_DATA:
		{
			if (temp_node_field_editor->global_value!=(struct FE_field *)data)
			{
				temp_node_field_editor->global_value=(struct FE_field *)data;
#if defined (NODE_FIELD_EDITOR_NAME)
				/* set the name of the node_field_editor */
				node_field_editor_set_name(temp_node_field_editor);
#endif
				/* create the components of the field */
				node_field_editor_setup_components(temp_node_field_editor);
			}
		} break;
		case NODE_FIELD_EDITOR_VALUE:
		{
			temp_node_field_editor->node=(struct FE_node *)data;
			/* update the displayed values of the field */
			node_field_editor_update_values(temp_node_field_editor);
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"node_field_editor_set_data.  Invalid data type.");
			return_code=0;
		} break;
	}
	LEAVE;

	return (return_code);
} /* node_field_editor_set_data */

#if defined (OLD_CODE)
void *node_field_editor_get_data(Widget node_field_editor_widget,
	enum Node_field_editor_data_type data_type)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Changes a data item of the node_field_editor widget.
==============================================================================*/
{
	void *return_code;
	struct Node_field_editor_struct *temp_node_field_editor;
	static struct Callback_data dat_callback;
	static Widget dat_widget;

	ENTER(node_field_editor_set_data);
	/* Get the pointer to the data for the node_field_editor dialog */
	XtVaGetValues(node_field_editor_widget,XmNuserData,&temp_node_field_editor,NULL);
	switch (data_type)
	{
		case NODE_FIELD_EDITOR_UPDATE_CB:
		{
			dat_callback.procedure=temp_node_field_editor->
				callback_array[NODE_FIELD_EDITOR_UPDATE_CB].procedure;
			dat_callback.data=temp_node_field_editor->
				callback_array[NODE_FIELD_EDITOR_UPDATE_CB].data;
			return_code= &dat_callback;
		} break;
		case NODE_FIELD_EDITOR_DATA:
		{
			return_code=temp_node_field_editor->global_value;
		} break;
		case NODE_FIELD_EDITOR_VALUE:
		{
			return_code=temp_node_field_editor->values;
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"node_field_editor_get_data.  Invalid data type.");
			return_code=NULL;
		} break;
	}
	LEAVE;

	return (return_code);
} /* node_field_editor_get_data */
#endif /* defined (OLD_CODE) */
