/*******************************************************************************
FILE : material_editor_dialog.c

LAST MODIFIED : 4 May 2004

DESCRIPTION :
This module creates a free material_editor_dialog input device, using two dof3,
two control and one input widget.  The position is given relative to some
coordinate system, and the returned value is a global one.
==============================================================================*/
#include <stdio.h>
#include <Xm/Protocols.h>
#include "three_d_drawing/graphics_buffer.h"
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
struct Material_editor_dialog
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Contains all the information carried by the material_editor_dialog widget.
Note that we just hold a pointer to the material_editor_dialog, and must access
and deaccess it.
==============================================================================*/
{
	struct Callback_data update_callback;
	struct Graphical_material *current_value;
	struct MANAGER(Graphical_material) *graphical_material_manager;
	struct Material_editor *material_editor;
	struct Material_editor_dialog **material_editor_dialog_address;
	struct User_interface *user_interface;
	Widget editor_form, select_form, select_widget;
	Widget apply_button,cancel_button,ok_button,revert_button;
	Widget dialog,widget,dialog_parent;
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
	struct Material_editor_dialog *material_editor_dialog)
/*******************************************************************************
LAST MODIFIED : 24 September 1996

DESCRIPTION :
Tells CMGUI about the current values. Returns a pointer to the material.
==============================================================================*/
{
	ENTER(material_editor_dialog_update);
	if (material_editor_dialog)
	{
		if (material_editor_dialog->update_callback.procedure)
		{
			/* now call the procedure with the user data and the position data */
			(material_editor_dialog->update_callback.procedure)
				(material_editor_dialog->widget,
				material_editor_dialog->update_callback.data,
				material_editor_dialog->current_value);
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
	void *user_data, void *material_void)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Finds which material is selected, and informs the editor widget.
???DB.  Not checking arguments ?
==============================================================================*/
{
	struct Material_editor_dialog *material_editor_dialog;
	struct Graphical_material *material;

	ENTER(material_editor_dialog_update_selection);
	USE_PARAMETER(select_widget);
	if ((material_editor_dialog=(struct Material_editor_dialog *)
		user_data)&&(material=(struct Graphical_material *)material_void))
	{
		material_editor_dialog->current_value=material;
		material_editor_set_material(material_editor_dialog->material_editor,
			material_editor_dialog->current_value);
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
	struct Material_editor_dialog *material_editor_dialog;

	ENTER(material_editor_dialog_identify_button);
	USE_PARAMETER(reason);
	/* find out which material_editor_dialog widget we are in */
	XtVaGetValues(w,XmNuserData,&material_editor_dialog,NULL);
	switch (button_num)
	{
		case material_editor_dialog_editor_form_ID:
		{
			material_editor_dialog->editor_form=w;
		} break;
		case material_editor_dialog_select_form_ID:
		{
			material_editor_dialog->select_form=w;
		} break;
		case material_editor_dialog_ok_ID:
		{
			material_editor_dialog->ok_button=w;
		} break;
		case material_editor_dialog_apply_ID:
		{
			material_editor_dialog->apply_button=w;
		} break;
		case material_editor_dialog_revert_ID:
		{
			material_editor_dialog->revert_button=w;
		} break;
		case material_editor_dialog_cancel_ID:
		{
			material_editor_dialog->cancel_button=w;
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"material_editor_dialog_identify_button.  Invalid button number");
		} break;
	}
	LEAVE;
} /* material_editor_dialog_identify_button */

static void material_editor_dialog_close_CB(Widget caller,
	void *material_editor_dialog_void,void *cbs)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Callback for the material_editor_dialog dialog
==============================================================================*/
{
	struct Material_editor_dialog *material_editor_dialog;

	ENTER(material_editor_dialog_close_CB);
	USE_PARAMETER(caller);
	USE_PARAMETER(cbs);
	if (material_editor_dialog =
		(struct Material_editor_dialog *)material_editor_dialog_void)
	{
		DESTROY(Material_editor_dialog)(
			material_editor_dialog->material_editor_dialog_address);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_dialog_close_CB.  Missing material_editor_dialog");
	}
	LEAVE;
} /* material_editor_dialog_close_CB */

static void material_editor_dialog_control_CB(Widget w,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Finds the id of the buttons on the material_editor_dialog widget.
==============================================================================*/
{
	struct Graphical_material *edit_material;
	struct Material_editor_dialog *material_editor_dialog;

	ENTER(material_editor_dialog_control_CB);
	USE_PARAMETER(reason);
	/* find out which material_editor_dialog widget we are in */
	XtVaGetValues(w,XmNuserData,&material_editor_dialog,NULL);
	if ((button_num==material_editor_dialog_cancel_ID)||
		(button_num==material_editor_dialog_revert_ID))
	{
		material_editor_set_material(material_editor_dialog->material_editor,
			material_editor_dialog->current_value);
	}
	/* if it is Apply or OK, do a global update */
	if ((button_num==material_editor_dialog_apply_ID)||
		(button_num==material_editor_dialog_ok_ID))
	{
		if (material_editor_dialog->current_value&&
			(edit_material=material_editor_get_material(
				material_editor_dialog->material_editor)))
		{
			MANAGER_MODIFY_NOT_IDENTIFIER(Graphical_material,name)(
				material_editor_dialog->current_value,edit_material,
				material_editor_dialog->graphical_material_manager);
			material_editor_dialog_update(material_editor_dialog);
		}
	}
	/* if it is an OK or a cancel, we have to kill the dialog */
	if ((button_num==material_editor_dialog_ok_ID)||
		(button_num==material_editor_dialog_cancel_ID))
	{
		DESTROY(Material_editor_dialog)(
			material_editor_dialog->material_editor_dialog_address);
	}
	LEAVE;
} /* material_editor_dialog_control_CB */

static struct Material_editor_dialog *CREATE(Material_editor_dialog)(
	struct Material_editor_dialog **material_editor_dialog_address,
	Widget parent,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(Texture) *texture_manager,struct Graphical_material *init_data,
	struct Graphics_buffer_package *graphics_buffer_package,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Creates a dialog widget that allows the user to edit the properties of any of
the materials contained in the global list.
==============================================================================*/
{
	Atom WM_DELETE_WINDOW;
	int init_widgets;
	MrmType material_editor_dialog_dialog_class;
	struct Callback_data callback;
	struct Material_editor_dialog *material_editor_dialog;
	static MrmRegisterArg callback_list[]=
	{
		{"mat_editor_d_identify_button",
		 (XtPointer)material_editor_dialog_identify_button},
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

	ENTER(CREATE(Material_editor_dialog));
	material_editor_dialog = (struct Material_editor_dialog *)NULL;
	if (material_editor_dialog_address && graphical_material_manager &&
		texture_manager && user_interface)
	{
		if (MrmOpenHierarchy_base64_string(material_editor_dialog_uidh,
			&material_editor_dialog_hierarchy,&material_editor_dialog_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(material_editor_dialog,
				struct Material_editor_dialog,1))
			{
				/* initialise the structure */
				material_editor_dialog->dialog_parent=parent;
				material_editor_dialog->material_editor_dialog_address=
					material_editor_dialog_address;
				material_editor_dialog->graphical_material_manager=
					graphical_material_manager;
				/* current_value set in material_editor_dialog_set_material */
				material_editor_dialog->current_value=
					(struct Graphical_material *)NULL;
				material_editor_dialog->widget=(Widget)NULL;
				material_editor_dialog->dialog=(Widget)NULL;
				material_editor_dialog->select_form=(Widget)NULL;
				material_editor_dialog->select_widget=(Widget)NULL;
				material_editor_dialog->editor_form=(Widget)NULL;
				material_editor_dialog->material_editor =
					(struct Material_editor *)NULL;
				material_editor_dialog->ok_button=(Widget)NULL;
				material_editor_dialog->apply_button=(Widget)NULL;
				material_editor_dialog->revert_button=(Widget)NULL;
				material_editor_dialog->cancel_button=(Widget)NULL;
				material_editor_dialog->update_callback.procedure=
					(Callback_procedure *)NULL;
				material_editor_dialog->update_callback.data=NULL;
				material_editor_dialog->user_interface=user_interface;
				/* make the dialog shell */
				if (material_editor_dialog->dialog=XtVaCreatePopupShell(
					"Material Editor",topLevelShellWidgetClass,parent,XmNallowShellResize,
					TRUE,NULL))
				{
					/* register the callbacks */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						material_editor_dialog_hierarchy,callback_list,
						XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value=(XtPointer)material_editor_dialog;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
							material_editor_dialog_hierarchy,identifier_list,
							XtNumber(identifier_list)))
						{
							/* fetch position window widget */
							if (MrmSUCCESS==MrmFetchWidget(material_editor_dialog_hierarchy,
								"mat_editor_dialog_widget",material_editor_dialog->dialog,
								&(material_editor_dialog->widget),
								&material_editor_dialog_dialog_class))
							{
								XtManageChild(material_editor_dialog->widget);
								/* set the mode toggle to the correct position */
								init_widgets=1;
								if (!CREATE_SELECT_WIDGET(Graphical_material)(
									&material_editor_dialog->select_widget,
									material_editor_dialog->select_form,
									SELECT_LIST,
									material_editor_dialog->current_value,
									graphical_material_manager))
								{
									display_message(ERROR_MESSAGE,
						"CREATE(Material_editor_dialog).  Could not create select widget.");
									init_widgets=0;
								}
								if (!(material_editor_dialog->material_editor =
									CREATE(Material_editor)(
										material_editor_dialog->editor_form, texture_manager,
										(struct Graphical_material *)NULL, graphics_buffer_package,
										user_interface)))
								{
									display_message(ERROR_MESSAGE,
						"CREATE(Material_editor_dialog).  Could not create editor widget.");
									init_widgets=0;
								}
								if (init_widgets)
								{
									/* set current_value to init_data */
									material_editor_dialog_set_material(
										material_editor_dialog, init_data);
									callback.procedure=material_editor_dialog_update_selection;
									callback.data=material_editor_dialog;
									SELECT_SET_UPDATE_CB(Graphical_material)(
										material_editor_dialog->select_widget,&callback);
									/* Set up window manager callback for close window message */
									WM_DELETE_WINDOW = XmInternAtom(
										XtDisplay(material_editor_dialog->dialog),
										"WM_DELETE_WINDOW", False);
									XmAddWMProtocolCallback(material_editor_dialog->dialog,
										WM_DELETE_WINDOW,material_editor_dialog_close_CB,
										material_editor_dialog);
									create_Shell_list_item(&material_editor_dialog->dialog,
										user_interface);
									XtRealizeWidget(material_editor_dialog->dialog);
									XtPopup(material_editor_dialog->dialog, XtGrabNone);
								}
								else
								{
									DEALLOCATE(material_editor_dialog);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
			"CREATE(Material_editor_dialog).  Could not fetch material_editor_dialog");
								DEALLOCATE(material_editor_dialog);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
							"CREATE(Material_editor_dialog).  Could not register identifiers");
							DEALLOCATE(material_editor_dialog);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"CREATE(Material_editor_dialog).  Could not register callbacks");
						DEALLOCATE(material_editor_dialog);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CREATE(Material_editor_dialog).  Could not create popup shell.");
					DEALLOCATE(material_editor_dialog);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
	"CREATE(Material_editor_dialog).  Could not allocate material_editor_dialog");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Material_editor_dialog).  Could not open hierarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Material_editor_dialog).  Invalid argument(s)");
	}
	if (material_editor_dialog_address && material_editor_dialog)
	{
		*material_editor_dialog_address = material_editor_dialog;
	}
	LEAVE;

	return (material_editor_dialog);
} /* CREATE(Material_editor_dialog) */

/*
Global functions
----------------
*/

int DESTROY(Material_editor_dialog)(
	struct Material_editor_dialog **material_editor_dialog_address)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Destroys the <*material_editor_dialog_address> and sets
<*material_editor_dialog_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct Material_editor_dialog *material_editor_dialog;

	ENTER(DESTROY(Material_editor_dialog));
	if (material_editor_dialog_address &&
		(material_editor_dialog = *material_editor_dialog_address))
	{
		DESTROY(Material_editor)(&(material_editor_dialog->material_editor));
		if (material_editor_dialog->dialog)
		{
			destroy_Shell_list_item_from_shell(&material_editor_dialog->dialog,
				material_editor_dialog->user_interface);
			XtDestroyWidget(material_editor_dialog->dialog);
		}
		DEALLOCATE(*material_editor_dialog_address);
		*material_editor_dialog_address = (struct Material_editor_dialog *)NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Material_editor_dialog).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Material_editor_dialog) */

int material_editor_dialog_get_callback(
	struct Material_editor_dialog *material_editor_dialog,
	struct Callback_data *callback)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Get the update <callback> information for the <material_editor_dialog>.
==============================================================================*/
{
	int return_code;

	ENTER(material_editor_dialog_get_callback);
	return_code=0;
	/* check arguments */
	if (material_editor_dialog&&callback)
	{
		callback->procedure=material_editor_dialog->update_callback.procedure;
		callback->data=material_editor_dialog->update_callback.data;
		return_code=1;
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

int material_editor_dialog_set_callback(
	struct Material_editor_dialog *material_editor_dialog,
	struct Callback_data *callback)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Set the update <callback> information for the <material_editor_dialog>.
==============================================================================*/
{
	int return_code;

	ENTER(material_editor_dialog_set_callback);
	return_code=0;
	/* check arguments */
	if (material_editor_dialog&&callback)
	{
		material_editor_dialog->update_callback.procedure=callback->procedure;
		material_editor_dialog->update_callback.data=callback->data;
		return_code=1;
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
	struct Material_editor_dialog *material_editor_dialog)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Returns the material edited by the <material_editor_dialog>.
==============================================================================*/
{
	struct Graphical_material *material;

	ENTER(material_editor_dialog_get_material);
	if (material_editor_dialog)
	{
		material = material_editor_dialog->current_value;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_dialog_get_material.  Invalid argument(s)");
		material = (struct Graphical_material *)NULL;
	}
	LEAVE;

	return (material);
} /* material_editor_dialog_get_material */

int material_editor_dialog_set_material(
	struct Material_editor_dialog *material_editor_dialog,
	struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Set the <material> for the <material_editor_dialog>.
==============================================================================*/
{
	int return_code;

	ENTER(material_editor_dialog_set_material);
	return_code=0;
	/* check arguments */
	if (material_editor_dialog)
	{
		return_code=1;
		if (material)
		{
			if (!IS_MANAGED(Graphical_material)(material,
				material_editor_dialog->graphical_material_manager))
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
				material_editor_dialog->graphical_material_manager);
		}
		if (return_code=SELECT_SET_SELECT_ITEM(Graphical_material)(
			material_editor_dialog->select_widget,material))
		{
			material_editor_dialog->current_value=material;
			material_editor_set_material(
				material_editor_dialog->material_editor,
				material_editor_dialog->current_value);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_dialog_set_material.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* material_editor_dialog_set_material */

int bring_up_material_editor_dialog(
	struct Material_editor_dialog **material_editor_dialog_address,
	Widget parent, struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(Texture) *texture_manager,struct Graphical_material *material,
	struct Graphics_buffer_package *graphics_buffer_package,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
If there is a material_editor dialog in existence, then bring it to the front,
else create a new one.
==============================================================================*/
{
	int return_code;
	struct Material_editor_dialog *material_editor_dialog;

	ENTER(bring_up_material_editor_dialog);
	if (material_editor_dialog_address)
	{
		if (material_editor_dialog = *material_editor_dialog_address)
		{
			material_editor_dialog_set_material(material_editor_dialog, material);
			XtPopup(material_editor_dialog->dialog, XtGrabNone);
			XtVaSetValues(material_editor_dialog->dialog, XmNiconic, False, NULL);
			return_code = 1;
		}
		else
		{
			if (CREATE(Material_editor_dialog)(material_editor_dialog_address, parent,
				graphical_material_manager, texture_manager, material, 
				graphics_buffer_package, user_interface))
			{
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"bring_up_material_editor_dialog.  Error creating dialog");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"bring_up_material_editor_dialog.  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* bring_up_material_editor_dialog */
