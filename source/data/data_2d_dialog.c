/*******************************************************************************
FILE : data_2d_dialog.c

LAST MODIFIED : 29 January 1999

DESCRIPTION :
This module creates a free data_2d_dialog input device, using two dof3,
two control and one input widget.  The position is given relative to some
coordinate system, and the returned value is a global one.
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "data/data_2d.h"
#include "data/data_2d_dialog.h"
#include "data/data_2d_dialog.uidh"
#include "general/debug.h"
#include "select/select_finite_element.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
UIL Identifiers
---------------
*/
#define data_2d_dialog_select_form_ID 1
#define data_2d_dialog_editor_form_ID 2

/*
Module types
------------
*/
struct Data_2d_dialog_struct
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Contains all the information carried by the data_2d_dialog widget.
Note that we just hold a pointer to the data_2d_dialog, and must access
and deaccess it.
==============================================================================*/
{
	struct GROUP(FE_node) *current_value;
	struct MANAGER(GROUP(FE_node)) *group_manager;
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(FE_node) *node_manager;
	struct Callback_data callback_array[DATA_2D_DIALOG_NUM_CALLBACKS];
	Widget editor_form,editor_widget,select_form,select_widget;
	Widget *dialog_address,dialog,widget,dialog_parent;
}; /* data_2d_dialog_struct */

/*
Module variables
----------------
*/
static int data_2d_dialog_hierarchy_open=0;
static MrmHierarchy data_2d_dialog_hierarchy;

/*
Module functions
----------------
*/
static void data_2d_dialog_call_callback(
	struct Data_2d_dialog_struct *temp_data_2d_dialog,
	enum data_2d_dialog_data_type callback_number,void *callback_data)
/*******************************************************************************
LAST MODIFIED : 25 September 1994

DESCRIPTION :
Calls a callback
==============================================================================*/
{
	ENTER(data_2d_dialog_call_callback);
	if ((callback_number>=0)&&(callback_number<DATA_2D_DIALOG_NUM_CALLBACKS))
	{
		if (temp_data_2d_dialog->callback_array[callback_number].procedure)
		{
			(temp_data_2d_dialog->callback_array[callback_number].procedure)
				(temp_data_2d_dialog->widget,
					temp_data_2d_dialog->callback_array[callback_number].data,
					callback_data);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"data_2d_dialog_call_callback.  Invalid callback number");
	}
	LEAVE;
} /* data_2d_dialog_call_callback */

static void data_2d_dialog_update_callback(Widget data_2d_widget,
	void *user_data,void *callback_data)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
Tells CMGUI about the current values. Returns a pointer to the material.
==============================================================================*/
{
	struct Data_2d_dialog_struct *temp_data_2d_dialog=
		(struct Data_2d_dialog_struct *)user_data;

	ENTER(data_2d_dialog_update_callback);
	USE_PARAMETER(data_2d_widget);
	if (temp_data_2d_dialog)
	{
		if (temp_data_2d_dialog->current_value!=
			(struct GROUP(FE_node) *)callback_data)
		{
			display_message(WARNING_MESSAGE,
				"data_2d_dialog_select_callback.  Invalid current value");
		}
		data_2d_dialog_call_callback(temp_data_2d_dialog,DATA_2D_DIALOG_UPDATE_CB,
			temp_data_2d_dialog->current_value);
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"data_2d_dialog_select_callback.  Invalid arguments");
	}
	LEAVE;
} /* data_2d_dialog_update_callback */

static void data_2d_dialog_select_callback(Widget data_2d_widget,
	void *user_data,void *callback_data)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
Chains off to the client
==============================================================================*/
{
	struct Data_2d_dialog_struct *temp_data_2d_dialog=
		(struct Data_2d_dialog_struct *)user_data;

	ENTER(data_2d_dialog_select_callback);
	USE_PARAMETER(data_2d_widget);
	if (temp_data_2d_dialog)
	{
		data_2d_dialog_call_callback(temp_data_2d_dialog,DATA_2D_DIALOG_SELECT_CB,
			callback_data);
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"data_2d_dialog_select_callback.  Invalid arguments");
	}
	LEAVE;
} /* data_2d_dialog_select_callback */

static void data_2d_dialog_update_selection(Widget select_widget,
	void *user_data,void *temp_nod)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
Finds which material is selected, and informs the editor widget.
???DB.  Not checking arguments ?
???GMH. The new FE_node may be NULL.  Should really check userdata.
==============================================================================*/
{
	struct Data_2d_dialog_struct *temp_data_2d_dialog=user_data;
	struct GROUP(FE_node) *temp_node=temp_nod;

	ENTER(data_2d_dialog_update_selection);
	USE_PARAMETER(select_widget);
	if (temp_data_2d_dialog)
	{
		temp_data_2d_dialog->current_value=temp_node;
		data_2d_set_data(temp_data_2d_dialog->editor_widget,
			DATA_2D_DATA,temp_data_2d_dialog->current_value);
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"data_2d_dialog_update_selection.  Invalid dialog.");
	}
	LEAVE;
} /* data_2d_dialog_update_selection */

static void data_2d_dialog_identify_button(Widget w,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
Finds the id of the buttons on the data_2d_dialog widget.
==============================================================================*/
{
	struct Data_2d_dialog_struct *temp_data_2d_dialog;

	ENTER(data_2d_dialog_identify_button);
	USE_PARAMETER(reason);
	/* find out which data_2d_dialog widget we are in */
	XtVaGetValues(w,XmNuserData,&temp_data_2d_dialog,NULL);
	switch (button_num)
	{
		case data_2d_dialog_editor_form_ID:
		{
			temp_data_2d_dialog->editor_form=w;
		} break;
		case data_2d_dialog_select_form_ID:
		{
			temp_data_2d_dialog->select_form=w;
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"data_2d_dialog_identify_button.  Invalid button number");
		} break;
	}
	LEAVE;
} /* data_2d_dialog_identify_button */

static void data_2d_dialog_destroy_CB(Widget w,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Callback for the data_2d_dialog dialog - tidies up all details - mem etc
==============================================================================*/
{
	struct Data_2d_dialog_struct *temp_data_2d_dialog;

	ENTER(data_2d_dialog_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the data_2d_dialog widget */
	XtVaGetValues(w,XmNuserData,&temp_data_2d_dialog,NULL);
	/* call any destroy callbacks */
	data_2d_dialog_call_callback(temp_data_2d_dialog,DATA_2D_DIALOG_DESTROY_CB,
		NULL);
	/* deaccess the data_2d_dialog */
	*(temp_data_2d_dialog->dialog_address)=(Widget)NULL;
	/* deallocate the memory for the user data */
	DEALLOCATE(temp_data_2d_dialog);
	LEAVE;
} /* data_2d_dialog_destroy_CB */

/*
Global functions
----------------
*/
Widget create_data_2d_dialog(Widget *data_2d_dialog_widget,
	Widget parent,int dimension,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_node)) *group_manager,
	struct GROUP(FE_node) *init_data,DATA_2D_PRECISION select_radius)
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Creates a dialog widget that allows the user to edit the properties of any of
the materials contained in the global list.
==============================================================================*/
{
	int i,init_widgets;
	MrmType data_2d_dialog_dialog_class;
	struct Callback_data callback;
	struct Data_2d_dialog_struct *temp_data_2d_dialog=NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"data_2d_d_identify_button",
		(XtPointer)data_2d_dialog_identify_button},
		{"data_2d_d_destroy_CB",
		(XtPointer)data_2d_dialog_destroy_CB},
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"Data_2d_d_structure",(XtPointer)NULL},
		{"data_2d_d_editor_form_ID",
		(XtPointer)data_2d_dialog_editor_form_ID},
		{"data_2d_d_select_form_ID",
		(XtPointer)data_2d_dialog_select_form_ID},
	};
	Widget return_widget;

	ENTER(create_data_2d_dialog);
	return_widget=(Widget)NULL;
	if (MrmOpenHierarchy_base64_string(data_2d_dialog_uidh,
		&data_2d_dialog_hierarchy,&data_2d_dialog_hierarchy_open))
	{
		/* check arguments */
		if (fe_field_manager&&node_manager&&group_manager)
		{
			/* allocate memory */
			if (ALLOCATE(temp_data_2d_dialog,
				struct Data_2d_dialog_struct,1))
			{
				/* initialise the structure */
				temp_data_2d_dialog->dialog_parent=parent;
				temp_data_2d_dialog->dialog_address=data_2d_dialog_widget;
				if (init_data)
				{
					temp_data_2d_dialog->current_value=init_data;
				}
				else
				{
					temp_data_2d_dialog->current_value=FIRST_OBJECT_IN_MANAGER_THAT(
						GROUP(FE_node))(
						(MANAGER_CONDITIONAL_FUNCTION(GROUP(FE_node)) *)NULL,(void *)NULL,
						group_manager);
				}
				temp_data_2d_dialog->group_manager=group_manager;
				temp_data_2d_dialog->fe_field_manager=fe_field_manager;
				temp_data_2d_dialog->node_manager=node_manager;
				temp_data_2d_dialog->widget=(Widget)NULL;
				temp_data_2d_dialog->dialog=(Widget)NULL;
				temp_data_2d_dialog->select_form=(Widget)NULL;
				temp_data_2d_dialog->select_widget=(Widget)NULL;
				temp_data_2d_dialog->editor_form=(Widget)NULL;
				temp_data_2d_dialog->editor_widget=(Widget)NULL;
				for (i=0;i<DATA_2D_DIALOG_NUM_CALLBACKS;i++)
				{
					temp_data_2d_dialog->callback_array[i].procedure=
						(Callback_procedure *)NULL;
					temp_data_2d_dialog->callback_array[i].data=NULL;
				}
				/* make the dialog shell */
				if (temp_data_2d_dialog->dialog=XtVaCreatePopupShell(
					"2D Digitiser",topLevelShellWidgetClass,parent,XmNallowShellResize,
					TRUE,NULL))
				{
					/* register the callbacks */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(data_2d_dialog_hierarchy,
						callback_list,XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value=(XtPointer)temp_data_2d_dialog;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
							data_2d_dialog_hierarchy,identifier_list,
							XtNumber(identifier_list)))
						{
							/* fetch position window widget */
							if (MrmSUCCESS==MrmFetchWidget(data_2d_dialog_hierarchy,
								"data_2d_dialog_widget",temp_data_2d_dialog->dialog,
								&(temp_data_2d_dialog->widget),&data_2d_dialog_dialog_class))
							{
								XtManageChild(temp_data_2d_dialog->widget);
								/* set the mode toggle to the correct position */
								init_widgets=1;
								if (!CREATE_SELECT_WIDGET(GROUP(FE_node))(
									&temp_data_2d_dialog->select_widget,
									temp_data_2d_dialog->select_form,SELECT_LIST,
									temp_data_2d_dialog->current_value,group_manager))
								{
									display_message(ERROR_MESSAGE,
										"create_data_2d_dialog.  Could not create select widget.");
									init_widgets=0;
								}
								if (!create_data_2d_widget(
									&temp_data_2d_dialog->editor_widget,
									temp_data_2d_dialog->editor_form,dimension,
									fe_field_manager,node_manager,group_manager,
									temp_data_2d_dialog->current_value,select_radius))
								{
									display_message(ERROR_MESSAGE,
										"create_data_2d_dialog.  Could not create editor widget.");
									init_widgets=0;
								}
								if (init_widgets)
								{
									callback.procedure=data_2d_dialog_update_selection;
									callback.data=temp_data_2d_dialog;
									SELECT_SET_UPDATE_CB(GROUP(FE_node))(
										temp_data_2d_dialog->select_widget,&callback);
									/* register chaining callbacks */
									callback.procedure=data_2d_dialog_select_callback;
									callback.data=temp_data_2d_dialog;
									data_2d_set_data(temp_data_2d_dialog->editor_widget,
										DATA_2D_SELECT_CB,&callback);
									callback.procedure=data_2d_dialog_update_callback;
									callback.data=temp_data_2d_dialog;
									data_2d_set_data(temp_data_2d_dialog->editor_widget,
										DATA_2D_UPDATE_CB,&callback);
									/* realise etc */
									XtRealizeWidget(temp_data_2d_dialog->dialog);
									XtPopup(temp_data_2d_dialog->dialog, XtGrabNone);
									return_widget=temp_data_2d_dialog->dialog;
								}
								else
								{
									DEALLOCATE(temp_data_2d_dialog);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"create_data_2d_dialog.  Could not fetch data_2d_dialog");
								DEALLOCATE(temp_data_2d_dialog);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_data_2d_dialog.  Could not register identifiers");
							DEALLOCATE(temp_data_2d_dialog);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_data_2d_dialog.  Could not register callbacks");
						DEALLOCATE(temp_data_2d_dialog);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_data_2d_dialog.  Could not create popup shell.");
					DEALLOCATE(temp_data_2d_dialog);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_data_2d_dialog.  Could not allocate data_2d_dialog");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_data_2d_dialog.  Invalid argument(s)");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_data_2d_widget.  Could not open hierarchy");
	}
	if (data_2d_dialog_widget)
	{
		*data_2d_dialog_widget=return_widget;
	}
	LEAVE;

	return (return_widget);
} /* create_data_2d_dialog */

int data_2d_dialog_set_data(Widget data_2d_dialog_widget,
	enum data_2d_dialog_data_type data_type,void *data)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
If <data_2d_dialog_widget> is not NULL, then change the data item on
<data_2d_dialog widget>.  Otherwise, change the data item on
<data_2d_dialog>.
==============================================================================*/
{
	int num_children,return_code;
	struct Data_2d_dialog_struct *temp_data_2d_dialog;
	Widget *child_list;

	ENTER(data_2d_dialog_set_data);
	if (data_2d_dialog_widget)
	{
		/* get the pointer to the data for the dialog */
		XtVaGetValues(data_2d_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&num_children,
			NULL);
		if (1==num_children)
		{
			XtVaGetValues(child_list[0],
				XmNuserData,&temp_data_2d_dialog,
				NULL);
			switch (data_type)
			{
				case DATA_2D_DIALOG_UPDATE_CB:
				{
					temp_data_2d_dialog->
						callback_array[DATA_2D_DIALOG_UPDATE_CB].procedure=
						((struct Callback_data *)data)->procedure;
					temp_data_2d_dialog->
						callback_array[DATA_2D_DIALOG_UPDATE_CB].data=
						((struct Callback_data *)data)->data;
					return_code=1;
				} break;
				case DATA_2D_DIALOG_SELECT_CB:
				{
					temp_data_2d_dialog->
						callback_array[DATA_2D_DIALOG_SELECT_CB].procedure=
						((struct Callback_data *)data)->procedure;
					temp_data_2d_dialog->
						callback_array[DATA_2D_DIALOG_SELECT_CB].data=
						((struct Callback_data *)data)->data;
					return_code=1;
				} break;
				case DATA_2D_DIALOG_DESTROY_CB:
				{
					temp_data_2d_dialog->
						callback_array[DATA_2D_DIALOG_DESTROY_CB].procedure=
						((struct Callback_data *)data)->procedure;
					temp_data_2d_dialog->
						callback_array[DATA_2D_DIALOG_DESTROY_CB].data=
						((struct Callback_data *)data)->data;
					return_code=1;
				} break;
				case DATA_2D_DIALOG_DATA:
				{
					if (data)
					{
						temp_data_2d_dialog->current_value=
							(struct GROUP(FE_node) *)data;
						if (!IS_MANAGED(GROUP(FE_node))(temp_data_2d_dialog->current_value,
							temp_data_2d_dialog->group_manager))
						{
							display_message(ERROR_MESSAGE,
								"data_2d_dialog_set_data.  Invalid FE_node.");
						}
					}
					else
					{
						temp_data_2d_dialog->current_value=
							FIRST_OBJECT_IN_MANAGER_THAT(GROUP(FE_node))(
								(MANAGER_CONDITIONAL_FUNCTION(GROUP(FE_node)) *)NULL,
								(void *)NULL,temp_data_2d_dialog->group_manager);
					}
					SELECT_SET_SELECT_ITEM(GROUP(FE_node))(
						temp_data_2d_dialog->select_widget,
						temp_data_2d_dialog->current_value);
					data_2d_set_data(temp_data_2d_dialog->editor_widget,
						DATA_2D_DATA,temp_data_2d_dialog->current_value);
				} break;
				case DATA_2D_DIALOG_SELECT_RADIUS:
				{
					return_code=data_2d_set_data(temp_data_2d_dialog->editor_widget,
						DATA_2D_SELECT_RADIUS,data);
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"data_2d_dialog_set_data.  Invalid data type.");
					return_code=0;
				} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"data_2d_dialog_set_data.  Invalid dialog.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"data_2d_dialog_set_data.  Missing dialog.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* data_2d_dialog_set_data */

void *data_2d_dialog_get_data(Widget data_2d_dialog_widget,
	enum data_2d_dialog_data_type data_type)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
If <data_2d_dialog_widget> is not NULL, then get the data item from
<data_2d_dialog widget>.  Otherwise, get the data item from
<data_2d_dialog>.
==============================================================================*/
{
	int num_children;
	void *return_code;
	struct Data_2d_dialog_struct *temp_data_2d_dialog;
	static struct Callback_data dat_callback;
	Widget *child_list;

	ENTER(data_2d_dialog_get_data);
	if (data_2d_dialog_widget)
	{
		/* Get the pointer to the data for the data_2d_dialog dialog */
		XtVaGetValues(data_2d_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&num_children,
			NULL);
		if (1==num_children)
		{
			XtVaGetValues(child_list[0],XmNuserData,&temp_data_2d_dialog,
				NULL);
			switch (data_type)
			{
				case DATA_2D_DIALOG_UPDATE_CB:
				{
					dat_callback.procedure=temp_data_2d_dialog->callback_array
						[DATA_2D_DIALOG_UPDATE_CB].procedure;
					dat_callback.data=temp_data_2d_dialog->callback_array
						[DATA_2D_DIALOG_UPDATE_CB].data;
					return_code=&dat_callback;
				} break;
				case DATA_2D_DIALOG_SELECT_CB:
				{
					dat_callback.procedure=temp_data_2d_dialog->callback_array
						[DATA_2D_DIALOG_SELECT_CB].procedure;
					dat_callback.data=temp_data_2d_dialog->callback_array
						[DATA_2D_DIALOG_SELECT_CB].data;
					return_code=&dat_callback;
				} break;
				case DATA_2D_DIALOG_DESTROY_CB:
				{
					dat_callback.procedure=temp_data_2d_dialog->callback_array
						[DATA_2D_DIALOG_DESTROY_CB].procedure;
					dat_callback.data=temp_data_2d_dialog->callback_array
						[DATA_2D_DIALOG_DESTROY_CB].data;
					return_code=&dat_callback;
				} break;
				case DATA_2D_DIALOG_DATA:
				{
					return_code=temp_data_2d_dialog->current_value;
				} break;
				case DATA_2D_DIALOG_SELECT_RADIUS:
				{
					return_code=data_2d_get_data(temp_data_2d_dialog->editor_widget,
						DATA_2D_SELECT_RADIUS);
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"data_2d_dialog_get_data.  Invalid data type.");
					return_code=NULL;
				} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"data_2d_dialog_get_data.  Invalid dialog.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"data_2d_dialog_get_data.  Missing dialog.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* data_2d_dialog_get_data */

int data_2d_dialog_selection(Widget data_2d_dialog_widget,
	DATA_2D_PRECISION *data,int button_down,int single)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Notifies the grabber about the selection process.
==============================================================================*/
{
	int num_children,return_code;
	struct Data_2d_dialog_struct *temp_data_2d_dialog;
	Widget *child_list;

	ENTER(data_2d_dialog_set_data);
	return_code=0;
	if (data_2d_dialog_widget)
	{
		/* get the pointer to the data for the dialog */
		XtVaGetValues(data_2d_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&num_children,
			NULL);
		if (1==num_children)
		{
			XtVaGetValues(child_list[0],
				XmNuserData,&temp_data_2d_dialog,
				NULL);
			/* just call the editor */
			return_code=data_2d_selection(temp_data_2d_dialog->editor_widget,data,
				button_down,single);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"data_2d_dialog_selection.  Invalid dialog.");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"data_2d_dialog_selection.  Missing dialog.");
	}
	LEAVE;

	return (return_code);
} /* data_2d_dialog_selection */
