/*******************************************************************************
FILE : material_editor_dialog.c

LAST MODIFIED : 30 May 2001

DESCRIPTION :
This module creates a free material_editor_dialog input device, using two dof3,
two control and one input widget.  The position is given relative to some
coordinate system, and the returned value is a global one.
==============================================================================*/
#include <stdio.h>
#include "general/debug.h"
#include "graphics/material.h"
#include "material/material_editor.h"
#include "material/material_editor_dialog.h"
#include "material/material_editor_dialog.uidh"
#include "select/select_graphical_material.h"
#include "user_interface/message.h"

/*
Module Constants
----------------
*/
/* UIL Identifiers */
#define material_editor_dialog_select_form_ID 1
#define material_editor_dialog_editor_form_ID 2
#define material_editor_dialog_ok_ID          3
#define material_editor_dialog_apply_ID       4
#define material_editor_dialog_revert_ID      5
#define material_editor_dialog_cancel_ID      6

/*
Global Types
------------
*/
struct Material_editor_dialog_struct
/*******************************************************************************
LAST MODIFIED : 1 December 1997

DESCRIPTION :
Contains all the information carried by the material_editor_dialog widget.
Note that we just hold a pointer to the material_editor_dialog, and must access
and deaccess it.
==============================================================================*/
{
	struct Callback_data update_callback;
	struct Graphical_material *current_value;
	struct MANAGER(Graphical_material) *graphical_material_manager;
	Widget editor_form,editor_widget,select_form,select_widget;
	Widget apply_button,cancel_button,ok_button,revert_button;
	Widget *dialog_address,dialog,widget,dialog_parent;
}; /* material_editor_dialog_struct */

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int material_editor_dialog_hierarchy_open=0;
static MrmHierarchy material_editor_dialog_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
static void material_editor_dialog_update(
	struct Material_editor_dialog_struct *temp_material_editor_dialog)
/*******************************************************************************
LAST MODIFIED : 24 September 1996

DESCRIPTION :
Tells CMGUI about the current values. Returns a pointer to the material.
==============================================================================*/
{
	ENTER(material_editor_dialog_update);
	if (temp_material_editor_dialog)
	{
		if (temp_material_editor_dialog->update_callback.procedure)
		{
			/* now call the procedure with the user data and the position data */
			(temp_material_editor_dialog->update_callback.procedure)
				(temp_material_editor_dialog->widget,
				temp_material_editor_dialog->update_callback.data,
				temp_material_editor_dialog->current_value);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_dialog_update.  Invalid argument(s)");
	}
	LEAVE;
} /* material_editor_dialog_update */

static void material_editor_dialog_update_selection(Widget select_widget,
	void *user_data, void *temp_mat)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Finds which material is selected, and informs the editor widget.
???DB.  Not checking arguments ?
==============================================================================*/
{
	struct Material_editor_dialog_struct *temp_material_editor_dialog;
	struct Graphical_material *temp_material;

	ENTER(material_editor_dialog_update_selection);
	USE_PARAMETER(select_widget);
	if ((temp_material_editor_dialog=(struct Material_editor_dialog_struct *)
		user_data)&&(temp_material=(struct Graphical_material *)temp_mat))
	{
		temp_material_editor_dialog->current_value=temp_material;
		material_editor_set_material(temp_material_editor_dialog->editor_widget,
			temp_material_editor_dialog->current_value);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_dialog_update_selection.  Invalid argument(s)");
	}
	LEAVE;
} /* material_editor_dialog_update_selection */

static void material_editor_dialog_identify_button(Widget w,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Finds the id of the buttons on the material_editor_dialog widget.
==============================================================================*/
{
	struct Material_editor_dialog_struct *temp_material_editor_dialog;

	ENTER(material_editor_dialog_identify_button);
	USE_PARAMETER(reason);
	/* find out which material_editor_dialog widget we are in */
	XtVaGetValues(w,XmNuserData,&temp_material_editor_dialog,NULL);
	switch (button_num)
	{
		case material_editor_dialog_editor_form_ID:
		{
			temp_material_editor_dialog->editor_form=w;
		} break;
		case material_editor_dialog_select_form_ID:
		{
			temp_material_editor_dialog->select_form=w;
		} break;
		case material_editor_dialog_ok_ID:
		{
			temp_material_editor_dialog->ok_button=w;
		} break;
		case material_editor_dialog_apply_ID:
		{
			temp_material_editor_dialog->apply_button=w;
		} break;
		case material_editor_dialog_revert_ID:
		{
			temp_material_editor_dialog->revert_button=w;
		} break;
		case material_editor_dialog_cancel_ID:
		{
			temp_material_editor_dialog->cancel_button=w;
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"material_editor_dialog_identify_button.  Invalid button number");
		} break;
	}
	LEAVE;
} /* material_editor_dialog_identify_button */

static void material_editor_dialog_control_CB(Widget w,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Finds the id of the buttons on the material_editor_dialog widget.
==============================================================================*/
{
	struct Graphical_material *edit_material;
	struct Material_editor_dialog_struct *temp_material_editor_dialog;

	ENTER(material_editor_dialog_control_CB);
	USE_PARAMETER(reason);
	/* find out which material_editor_dialog widget we are in */
	XtVaGetValues(w,XmNuserData,&temp_material_editor_dialog,NULL);
	if ((button_num==material_editor_dialog_cancel_ID)||
		(button_num==material_editor_dialog_revert_ID))
	{
		material_editor_set_material(temp_material_editor_dialog->editor_widget,
			temp_material_editor_dialog->current_value);
	}
	/* if it is Apply or OK, do a global update */
	if ((button_num==material_editor_dialog_apply_ID)||
		(button_num==material_editor_dialog_ok_ID))
	{
		if (temp_material_editor_dialog->current_value&&
			(edit_material=material_editor_get_material(
				temp_material_editor_dialog->editor_widget)))
		{
			MANAGER_MODIFY_NOT_IDENTIFIER(Graphical_material,name)(
				temp_material_editor_dialog->current_value,edit_material,
				temp_material_editor_dialog->graphical_material_manager);
			material_editor_dialog_update(temp_material_editor_dialog);
		}
	}
	/* if it is an OK or a cancel, we have to kill the dialog */
	if ((button_num==material_editor_dialog_ok_ID)||
		(button_num==material_editor_dialog_cancel_ID))
	{
		XtDestroyWidget(temp_material_editor_dialog->dialog);
	}
	LEAVE;
} /* material_editor_dialog_control_CB */

static void material_editor_dialog_destroy_CB(Widget w,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Callback for the material_editor_dialog dialog - tidies up all details - mem etc
==============================================================================*/
{
	struct Material_editor_dialog_struct *temp_material_editor_dialog;

	ENTER(material_editor_dialog_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the material_editor_dialog widget */
	XtVaGetValues(w,XmNuserData,&temp_material_editor_dialog,NULL);
	/* deaccess the material_editor_dialog */
	*(temp_material_editor_dialog->dialog_address)=(Widget)NULL;
	/* deallocate the memory for the user data */
	DEALLOCATE(temp_material_editor_dialog);
	LEAVE;
} /* material_editor_dialog_destroy_CB */

static Widget create_material_editor_dialog(
	Widget *material_editor_dialog_widget,Widget parent,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(Texture) *texture_manager,struct Graphical_material *init_data,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 13 December 1996

DESCRIPTION :
Creates a dialog widget that allows the user to edit the properties of any of
the materials contained in the global list.
==============================================================================*/
{
	int init_widgets;
	MrmType material_editor_dialog_dialog_class;
	struct Callback_data callback;
	struct Material_editor_dialog_struct *temp_material_editor_dialog=NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"mat_editor_d_identify_button",
		(XtPointer)material_editor_dialog_identify_button},
		{"mat_editor_d_destroy_CB",
			(XtPointer)material_editor_dialog_destroy_CB},
		{"mat_editor_d_control_CB",
			(XtPointer)material_editor_dialog_control_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"mat_editor_d_structure",(XtPointer)NULL},
		{"mat_editor_d_editor_form_ID",
		(XtPointer)material_editor_dialog_editor_form_ID},
		{"mat_editor_d_select_form_ID",
		(XtPointer)material_editor_dialog_select_form_ID},
		{"mat_editor_d_ok_ID",(XtPointer)material_editor_dialog_ok_ID},
		{"mat_editor_d_apply_ID",(XtPointer)material_editor_dialog_apply_ID},
		{"mat_editor_d_revert_ID",(XtPointer)material_editor_dialog_revert_ID},
		{"mat_editor_d_cancel_ID",(XtPointer)material_editor_dialog_cancel_ID}
	};
	Widget return_widget;

	ENTER(create_material_editor_dialog);
	return_widget=(Widget)NULL;
	if (graphical_material_manager&&texture_manager&&user_interface)
	{
		if (MrmOpenHierarchy_base64_string(material_editor_dialog_uidh,
			&material_editor_dialog_hierarchy,&material_editor_dialog_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(temp_material_editor_dialog,
				struct Material_editor_dialog_struct,1))
			{
				/* initialise the structure */
				temp_material_editor_dialog->dialog_parent=parent;
				temp_material_editor_dialog->dialog_address=
					material_editor_dialog_widget;
				temp_material_editor_dialog->graphical_material_manager=
					graphical_material_manager;
				/* current_value set in material_editor_dialog_set_material */
				temp_material_editor_dialog->current_value=
					(struct Graphical_material *)NULL;
				temp_material_editor_dialog->widget=(Widget)NULL;
				temp_material_editor_dialog->dialog=(Widget)NULL;
				temp_material_editor_dialog->select_form=(Widget)NULL;
				temp_material_editor_dialog->select_widget=(Widget)NULL;
				temp_material_editor_dialog->editor_form=(Widget)NULL;
				temp_material_editor_dialog->editor_widget=(Widget)NULL;
				temp_material_editor_dialog->ok_button=(Widget)NULL;
				temp_material_editor_dialog->apply_button=(Widget)NULL;
				temp_material_editor_dialog->revert_button=(Widget)NULL;
				temp_material_editor_dialog->cancel_button=(Widget)NULL;
				temp_material_editor_dialog->update_callback.procedure=
					(Callback_procedure *)NULL;
				temp_material_editor_dialog->update_callback.data=NULL;
				/* make the dialog shell */
				if (temp_material_editor_dialog->dialog=XtVaCreatePopupShell(
					"Material Editor",topLevelShellWidgetClass,parent,XmNallowShellResize,
					TRUE,NULL))
				{
					/* register the callbacks */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						material_editor_dialog_hierarchy,callback_list,
						XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value=(XtPointer)temp_material_editor_dialog;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
							material_editor_dialog_hierarchy,identifier_list,
							XtNumber(identifier_list)))
						{
							/* fetch position window widget */
							if (MrmSUCCESS==MrmFetchWidget(material_editor_dialog_hierarchy,
								"mat_editor_dialog_widget",temp_material_editor_dialog->dialog,
								&(temp_material_editor_dialog->widget),
								&material_editor_dialog_dialog_class))
							{
								XtManageChild(temp_material_editor_dialog->widget);
								/* set the mode toggle to the correct position */
								init_widgets=1;
								if (!CREATE_SELECT_WIDGET(Graphical_material)(
									&temp_material_editor_dialog->select_widget,
									temp_material_editor_dialog->select_form,
									SELECT_LIST,
									temp_material_editor_dialog->current_value,
									graphical_material_manager))
								{
									display_message(ERROR_MESSAGE,
						"create_material_editor_dialog.  Could not create select widget.");
									init_widgets=0;
								}
								if (!create_material_editor_widget(
									&(temp_material_editor_dialog->editor_widget),
									temp_material_editor_dialog->editor_form,texture_manager,
									(struct Graphical_material *)NULL,user_interface))
								{
									display_message(ERROR_MESSAGE,
						"create_material_editor_dialog.  Could not create editor widget.");
									init_widgets=0;
								}
								if (init_widgets)
								{
									/* set current_value to init_data */
									material_editor_dialog_set_material(
										temp_material_editor_dialog->dialog,init_data);
									callback.procedure=material_editor_dialog_update_selection;
									callback.data=temp_material_editor_dialog;
									SELECT_SET_UPDATE_CB(Graphical_material)(
										temp_material_editor_dialog->select_widget,&callback);
									XtRealizeWidget(temp_material_editor_dialog->dialog);
									XtPopup(temp_material_editor_dialog->dialog, XtGrabNone);
									return_widget=temp_material_editor_dialog->dialog;
								}
								else
								{
									DEALLOCATE(temp_material_editor_dialog);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
			"create_material_editor_dialog.  Could not fetch material_editor_dialog");
								DEALLOCATE(temp_material_editor_dialog);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
							"create_material_editor_dialog.  Could not register identifiers");
							DEALLOCATE(temp_material_editor_dialog);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_material_editor_dialog.  Could not register callbacks");
						DEALLOCATE(temp_material_editor_dialog);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_material_editor_dialog.  Could not create popup shell.");
					DEALLOCATE(temp_material_editor_dialog);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
	"create_material_editor_dialog.  Could not allocate material_editor_dialog");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_material_editor_dialog.  Could not open hierarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_material_editor_dialog.  Invalid argument(s)");
	}
	if (material_editor_dialog_widget&&return_widget)
	{
		*material_editor_dialog_widget=return_widget;
	}
	LEAVE;

	return (return_widget);
} /* create_material_editor_dialog */

/*
Global functions
----------------
*/
int material_editor_dialog_get_callback(Widget material_editor_dialog_widget,
	struct Callback_data *callback)
/*******************************************************************************
LAST MODIFIED : 1 December 1997

DESCRIPTION :
Returns the update_callback for the material editor_dialog widget.
==============================================================================*/
{
	int return_code,num_children;
	struct Material_editor_dialog_struct *temp_material_editor_dialog;
	Widget *child_list;

	ENTER(material_editor_dialog_get_callback);
	/* check arguments */
	if (material_editor_dialog_widget&&callback)
	{
		/* get the pointer to the data for the dialog */
		XtVaGetValues(material_editor_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&num_children,
			NULL);
		if (num_children==1)
		{
			XtVaGetValues(child_list[0],XmNuserData,&temp_material_editor_dialog,
				NULL);
			if (temp_material_editor_dialog)
			{
				callback->procedure=
					temp_material_editor_dialog->update_callback.procedure;
				callback->data=temp_material_editor_dialog->update_callback.data;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"material_editor_dialog_get_callback.  Missing widget data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"material_editor_dialog_get_callback.  Invalid dialog");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_dialog_get_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* material_editor_dialog_get_callback */

int material_editor_dialog_set_callback(Widget material_editor_dialog_widget,
	struct Callback_data *callback)
/*******************************************************************************
LAST MODIFIED : 1 December 1997

DESCRIPTION :
Changes the update_callback for the material editor_dialog widget.
==============================================================================*/
{
	int return_code,num_children;
	struct Material_editor_dialog_struct *temp_material_editor_dialog;
	Widget *child_list;

	ENTER(material_editor_dialog_set_callback);
	if (material_editor_dialog_widget&&callback)
	{
		/* get the pointer to the data for the dialog */
		XtVaGetValues(material_editor_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&num_children,
			NULL);
		if (num_children==1)
		{
			XtVaGetValues(child_list[0],XmNuserData,&temp_material_editor_dialog,
				NULL);
			if (temp_material_editor_dialog)
			{
				temp_material_editor_dialog->update_callback.procedure=
					callback->procedure;
				temp_material_editor_dialog->update_callback.data=callback->data;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"material_editor_dialog_set_callback.  Missing widget data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"material_editor_dialog_set_callback.  Invalid dialog");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_dialog_set_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* material_editor_dialog_set_callback */

struct Graphical_material *material_editor_dialog_get_material(
	Widget material_editor_dialog_widget)
/*******************************************************************************
LAST MODIFIED : 1 December 1997

DESCRIPTION :
If <material_editor_dialog_widget> is not NULL, then get the data item from
<material_editor_dialog widget>.  Otherwise, get the data item from
<material_editor_dialog>.
==============================================================================*/
{
	int num_children;
	struct Material_editor_dialog_struct *temp_material_editor_dialog;
	struct Graphical_material *return_material;
	Widget *child_list;

	ENTER(material_editor_dialog_get_material);
	if (material_editor_dialog_widget)
	{
		XtVaGetValues(material_editor_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&num_children,
			NULL);
		if (num_children==1)
		{
			XtVaGetValues(child_list[0],XmNuserData,&temp_material_editor_dialog,
				NULL);
			if (temp_material_editor_dialog)
			{
				return_material=temp_material_editor_dialog->current_value;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"material_editor_dialog_get_material.  Missing widget data");
				return_material=(struct Graphical_material *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"material_editor_dialog_get_material.  Invalid dialog");
			return_material=(struct Graphical_material *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_dialog_get_material.  Missing dialog");
		return_material=(struct Graphical_material *)NULL;
	}
	LEAVE;

	return (return_material);
} /* material_editor_dialog_get_material */

int material_editor_dialog_set_material(Widget material_editor_dialog_widget,
	struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 1 December 1997

DESCRIPTION :
If <material_editor_dialog_widget> is not NULL, then change the data item on
<material_editor_dialog widget>.  Otherwise, change the data item on
<material_editor_dialog>.
==============================================================================*/
{
	int num_children,return_code;
	struct Material_editor_dialog_struct *temp_material_editor_dialog;
	Widget *child_list;

	ENTER(material_editor_dialog_set_material);
	if (material_editor_dialog_widget)
	{
		/* get the pointer to the data for the dialog */
		XtVaGetValues(material_editor_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&num_children,
			NULL);
		if (num_children==1)
		{
			XtVaGetValues(child_list[0],XmNuserData,&temp_material_editor_dialog,
				NULL);
			if (temp_material_editor_dialog)
			{
				return_code=1;
				if (material)
				{
					if (!IS_MANAGED(Graphical_material)(material,
						temp_material_editor_dialog->graphical_material_manager))
					{
						display_message(ERROR_MESSAGE,
							"material_editor_dialog_set_material.  Material not managed");
						material=(struct Graphical_material *)NULL;
						return_code=0;
					}
				}
				if (!material)
				{
					material=FIRST_OBJECT_IN_MANAGER_THAT(Graphical_material)(
						(MANAGER_CONDITIONAL_FUNCTION(Graphical_material) *)NULL,
						(void *)NULL,
						temp_material_editor_dialog->graphical_material_manager);
				}
				if (return_code=SELECT_SET_SELECT_ITEM(Graphical_material)(
					temp_material_editor_dialog->select_widget,material))
				{
					temp_material_editor_dialog->current_value=material;
					material_editor_set_material(
						temp_material_editor_dialog->editor_widget,
						temp_material_editor_dialog->current_value);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"material_editor_dialog_set_material.  Missing widget data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"material_editor_dialog_set_material.  Invalid dialog");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_dialog_set_material.  Missing dialog");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* material_editor_dialog_set_material */

int bring_up_material_editor_dialog(Widget *material_editor_dialog_address,
	Widget parent,struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(Texture) *texture_manager,struct Graphical_material *material,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 1 December 1997

DESCRIPTION :
If there is a material_editor dialog in existence, then bring it to the front,
else create a new one.
==============================================================================*/
{
	int return_code;

	ENTER(bring_up_material_editor_dialog);
	if (material_editor_dialog_address)
	{
		if (*material_editor_dialog_address)
		{
			material_editor_dialog_set_material(*material_editor_dialog_address,
				material);
			XtPopup(*material_editor_dialog_address,XtGrabNone);
			return_code=1;
		}
		else
		{
			if (create_material_editor_dialog(material_editor_dialog_address,parent,
				graphical_material_manager,texture_manager,material,user_interface))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"bring_up_material_editor_dialog.  Error creating dialog");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"bring_up_material_editor_dialog.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* bring_up_material_editor_dialog */
