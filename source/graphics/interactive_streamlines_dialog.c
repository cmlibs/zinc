/*******************************************************************************
FILE : interactive_streamlines_dialog.c

LAST MODIFIED : 18 April 2000

DESCRIPTION :
This module creates a free streamline_editor_dialog input device, using two
dof3, two control and one input widget.  The position is given relative to some
coordinate system, and the returned value is a global one.
==============================================================================*/
#include <stdio.h>
#include <Xm/Xm.h>
#include "choose/choose_scene.h"
#include "choose/choose_graphical_material.h"
#include "graphics/graphics_object.h"
#include "finite_element/finite_element_to_streamlines.h"
#include "general/debug.h"
#include "graphics/interactive_streamlines_dialog.h"
#include "graphics/interactive_streamlines_dialog.uidh"
#include "graphics/scene.h"
#include "io_devices/input_module.h"
#include "io_devices/input_module_widget.h"
#include "select/select_interactive_streamline.h"
#include "user_interface/message.h"

/*
Module Constants
----------------
*/
/* UIL Identifiers */
#define interactive_streamline_dialog_select_form_ID 101
#define interactive_streamline_dialog_scene_chooser_form_ID 102
#define interactive_streamline_dialog_device_chooser_form_ID 103

/*
Global Types
------------
*/
struct Interactive_streamline_dialog_struct
/*******************************************************************************
LAST MODIFIED : 3 March 1998

DESCRIPTION :
Contains all the information carried by the interactive_streamline_dialog widget.
Note that we just hold a pointer to the interactive_streamline_dialog, and must access
and deaccess it.
==============================================================================*/
{
	enum Input_module_device device;
	struct Callback_data update_callback;
	struct Interactive_streamline *current_value;
	struct MANAGER(Interactive_streamline) *interactive_streamline_manager;
	struct Scene *scene;
	Widget input_device_form, input_device_widget,
		scene_form, scene_widget, select_form, select_widget;
	Widget *dialog_address, dialog, widget, dialog_parent;
}; /* Interactive_streamline_dialog_struct */

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int interactive_streamline_dialog_hierarchy_open=0;
static MrmHierarchy interactive_streamline_dialog_hierarchy;
#endif /* defined (MOTIF) */


/*
Module functions
----------------
*/
static void interactive_streamline_dialog_update_selection(Widget select_widget,
	void *user_data, void *streamline_void)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Finds which streamline is selected, and informs the editor widget.
==============================================================================*/
{
	struct Interactive_streamline_dialog_struct *interactive_streamline_dialog;
	struct Interactive_streamline *streamline;

	ENTER(interactive_streamline_dialog_update_selection);
	USE_PARAMETER(select_widget);
	if (user_data && streamline_void )
	{
		interactive_streamline_dialog =
			(struct Interactive_streamline_dialog_struct *) user_data;
		streamline = (struct Interactive_streamline *) streamline_void;
		interactive_streamline_dialog->current_value = streamline;
/*		interactive_streamline_set_streamline(interactive_streamline_dialog->editor_widget,
			interactive_streamline_dialog->current_value);*/
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"interactive_streamline_dialog_update_selection.  Invalid argument(s)");
	}
	LEAVE;
} /* interactive_streamline_dialog_update_selection */

static void interactive_streamline_dialog_identify(Widget w,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 3 March 1998

DESCRIPTION :
Finds the id of the buttons on the interactive_streamline_dialog widget.
==============================================================================*/
{
	struct Interactive_streamline_dialog_struct *interactive_streamline_dialog;

	ENTER(interactive_streamline_dialog_identify_button);
	USE_PARAMETER(reason);
	/* find out which interactive_streamline_dialog widget we are in */
	XtVaGetValues(w,XmNuserData,&interactive_streamline_dialog,NULL);
	switch (button_num)
	{
		case interactive_streamline_dialog_select_form_ID:
		{
			interactive_streamline_dialog->select_form=w;
		} break;
		case interactive_streamline_dialog_scene_chooser_form_ID:
		{
			interactive_streamline_dialog->scene_form=w;
		} break;
		case interactive_streamline_dialog_device_chooser_form_ID:
		{
			interactive_streamline_dialog->input_device_form=w;
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"interactive_streamline_dialog_identify_button.  Invalid ID number");
		} break;
	}
	LEAVE;
} /* interactive_streamline_dialog_identify_button */

static void interactive_streamline_dialog_control_CB(Widget w,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Responds to uid callbacks.
???RC does nothing!
==============================================================================*/
{
	struct Interactive_streamline_dialog_struct *interactive_streamline_dialog;

	ENTER(interactive_streamline_dialog_control_CB);
	USE_PARAMETER(button_num);
	USE_PARAMETER(reason);
	XtVaGetValues(w,XmNuserData,&interactive_streamline_dialog,NULL);
	LEAVE;
} /* interactive_streamline_dialog_control_CB */

static void interactive_streamline_dialog_destroy_CB(Widget w, int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Callback for the interactive_streamline_dialog dialog - tidies up all details - mem etc
==============================================================================*/
{
	struct Interactive_streamline_dialog_struct *interactive_streamline_dialog;

	ENTER(interactive_streamline_dialog_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);

	XtVaGetValues(w,XmNuserData,&interactive_streamline_dialog,NULL);

	/* deaccess the interactive_streamline_dialog */
	*(interactive_streamline_dialog->dialog_address)=(Widget)NULL;

	/* deallocate the memory for the user data */
	DEALLOCATE(interactive_streamline_dialog);

	LEAVE;
} /* interactive_streamline_dialog_destroy_CB */

static void interactive_streamline_dialog_update_scene(
	Widget widget,
	void *interactive_streamline_dialog_void,void *scene_void)
/*******************************************************************************
LAST MODIFIED : 3 March 1998

DESCRIPTION :
Called when scene is changed.
==============================================================================*/
{
	struct Interactive_streamline_dialog_struct *interactive_streamline_dialog;

	ENTER(interactive_streamline_dialog_update_scene);
	USE_PARAMETER(widget);
	if ( interactive_streamline_dialog_void && scene_void )
	{
		interactive_streamline_dialog =
		(struct Interactive_streamline_dialog_struct *)interactive_streamline_dialog_void;

		interactive_streamline_dialog->scene = (struct Scene *)scene_void;

		display_message(WARNING_MESSAGE,
			"interactive_streamline_dialog_update_scene.  Changing scene not enabled");
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"interactive_streamline_dialog_update_scene.  Invalid argument(s)");
	}
	LEAVE;
} /* interactive_streamline_dialog_update_scene */

static int interactive_streamline_dialog_newdata_CB(void *identifier,
	Input_module_message message)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Accepts input from one of the devices.
==============================================================================*/
{
	int return_code;
	FE_value data[3];
	struct Interactive_streamline_dialog_struct *interactive_streamline_dialog;

	ENTER(interactive_streamline_dialog_newdata_CB);
	if ( identifier && message )
	{
		interactive_streamline_dialog = identifier;
		switch (message->type)
		{
			case IM_TYPE_MOTION:
			{
				switch (message->source)
				{
					case IM_SOURCE_SPACEBALL:
					case IM_SOURCE_HAPTIC:
					{
						data[0] = message->data[0];
						data[1] = message->data[1];
						data[2] = message->data[2];
						update_interactive_streamline( interactive_streamline_dialog->current_value,
							data, interactive_streamline_dialog->interactive_streamline_manager );
					} break;
				}
			}
		}
		return_code = 1;
	}
	else
	{
		return_code = 0;
		display_message(ERROR_MESSAGE,
			"interactive_streamline_dialog_newdata_CB.  Invalid argument(s)");
	}

	return ( return_code );

	LEAVE;
} /* interactive_streamline_dialog_newdata_CB */

static void interactive_streamline_dialog_change_device(
	Widget widget,struct Input_module_widget_data *device_change,
	void *interactive_streamline_dialog_void)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
The selected device has been changed
==============================================================================*/
{
	struct Interactive_streamline_dialog_struct *interactive_streamline_dialog;

	ENTER(interactive_streamline_dialog_change_device);
	USE_PARAMETER(widget);
	if (device_change&&(interactive_streamline_dialog=
		(struct Interactive_streamline_dialog_struct *)
		interactive_streamline_dialog_void))
	{
		if (device_change->status)
		{
			interactive_streamline_dialog->device = device_change->device;
#ifdef EXT_INPUT
			input_module_register(device_change->device,
				interactive_streamline_dialog,
				interactive_streamline_dialog->widget,
				interactive_streamline_dialog_newdata_CB);
#endif /* EXT_INPUT */
		}
		else
		{
#ifdef EXT_INPUT
			input_module_deregister(device_change->device,interactive_streamline_dialog);
#endif /* EXT_INPUT */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"interactive_streamline_dialog_change_device. Invalid arguments");
	}

	LEAVE;
} /* interactive_streamline_dialog_change_device */

static Widget create_interactive_streamline_dialog(
	Widget *interactive_streamline_dialog_widget,Widget parent,
	struct MANAGER(Interactive_streamline) *interactive_streamline_manager,
	struct Interactive_streamline *init_data,
	struct User_interface *user_interface,struct MANAGER(Scene) *scene_manager)
/*******************************************************************************
LAST MODIFIED : 18 April 2000

DESCRIPTION :
Creates a dialog widget that allows the user to manipulate
the interactive streamlines.
==============================================================================*/
{
	int init_widgets;
	MrmType interactive_streamline_dialog_dialog_class;
	struct Callback_data callback;
	struct Interactive_streamline_dialog_struct *interactive_streamline_dialog=NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"streamline_d_identify",
		(XtPointer)interactive_streamline_dialog_identify},
		{"streamline_d_destroy_CB",
			(XtPointer)interactive_streamline_dialog_destroy_CB},
		{"streamline_d_control_CB",
			(XtPointer)interactive_streamline_dialog_control_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"streamline_d_structure",(XtPointer)NULL},
		{"streamline_d_select_form_ID",
		(XtPointer)interactive_streamline_dialog_select_form_ID},
		{"streamline_d_scene_form_ID",
		(XtPointer)interactive_streamline_dialog_scene_chooser_form_ID},
		{"streamline_d_device_form_ID",
		(XtPointer)interactive_streamline_dialog_device_chooser_form_ID},
	};
	Widget return_widget;

	ENTER(create_interactive_streamline_dialog);
	return_widget=(Widget)NULL;
	/* check arguments */
	if (interactive_streamline_manager&&user_interface)
	{
		if (MrmOpenHierarchy_base64_string(interactive_streamlines_dialog_uidh,
			&interactive_streamline_dialog_hierarchy,&interactive_streamline_dialog_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(interactive_streamline_dialog,
				struct Interactive_streamline_dialog_struct,1))
			{
				/* initialise the structure */
				interactive_streamline_dialog->dialog_parent = parent;
				interactive_streamline_dialog->dialog_address =
					interactive_streamline_dialog_widget;
				interactive_streamline_dialog->interactive_streamline_manager =
					interactive_streamline_manager;
				/* current_value set in interactive_streamline_dialog_set_streamline */
				interactive_streamline_dialog->current_value =
					init_data;
				interactive_streamline_dialog->widget = (Widget)NULL;
				interactive_streamline_dialog->dialog = (Widget)NULL;
				interactive_streamline_dialog->select_form = (Widget)NULL;
				interactive_streamline_dialog->select_widget = (Widget)NULL;
				interactive_streamline_dialog->input_device_form = (Widget)NULL;
				interactive_streamline_dialog->input_device_widget = (Widget)NULL;
				interactive_streamline_dialog->scene_form = (Widget)NULL;
				interactive_streamline_dialog->scene_widget = (Widget)NULL;
				/* make the dialog shell */
				if (interactive_streamline_dialog->dialog=XtVaCreatePopupShell(
					"Interactive Streamline Controller",topLevelShellWidgetClass,parent,XmNallowShellResize,
					TRUE,NULL))
				{
					/* register the callbacks */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						interactive_streamline_dialog_hierarchy,callback_list,
						XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value=(XtPointer)interactive_streamline_dialog;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
							interactive_streamline_dialog_hierarchy,identifier_list,
							XtNumber(identifier_list)))
						{
							/* fetch position window widget */
							if (MrmSUCCESS==MrmFetchWidget(interactive_streamline_dialog_hierarchy,
								"streamline_dialog_widget",interactive_streamline_dialog->dialog,
								&(interactive_streamline_dialog->widget),
								&interactive_streamline_dialog_dialog_class))
							{
								XtManageChild(interactive_streamline_dialog->widget);
								/* set the mode toggle to the correct position */
								init_widgets=1;
								if (!CREATE_SELECT_WIDGET(Interactive_streamline)(
									&interactive_streamline_dialog->select_widget,
									interactive_streamline_dialog->select_form,
									SELECT_LIST,
									interactive_streamline_dialog->current_value,
									interactive_streamline_manager))
								{
									display_message(ERROR_MESSAGE,
						"create_interactive_streamline_dialog.  Could not create select widget.");
									init_widgets=0;
								}
								/* add the input menu */
								if (create_input_module_widget(
									&(interactive_streamline_dialog->input_device_widget),
									interactive_streamline_dialog->input_device_form ))
								{
									Input_module_add_device_change_callback(
										interactive_streamline_dialog->input_device_widget,
										interactive_streamline_dialog_change_device,
										(void *)interactive_streamline_dialog);
									if (interactive_streamline_dialog->scene_widget =
										CREATE_CHOOSE_OBJECT_WIDGET(Scene)(
										interactive_streamline_dialog->scene_form,
										interactive_streamline_dialog->scene,
										scene_manager,(MANAGER_CONDITIONAL_FUNCTION(Scene) *)NULL,
										(void *)NULL))
									{
										callback.procedure=
											interactive_streamline_dialog_update_scene;
										callback.data = (void *)interactive_streamline_dialog;
										CHOOSE_OBJECT_SET_CALLBACK(Scene)(
											interactive_streamline_dialog->scene_widget,&callback);
									}
									else
									{
										init_widgets = 0;
										display_message(ERROR_MESSAGE,
												"create_interactive_streamline_dialog.  Could not create scene chooser");
										DEALLOCATE(interactive_streamline_dialog);
									}
								}
								else
								{
									init_widgets = 0;
									display_message(ERROR_MESSAGE,
												"create_interactive_streamline_dialog.  Could not create input module chooser widget");
									DEALLOCATE(interactive_streamline_dialog);
								}
								if (init_widgets)
								{
									/* set current_value to init_data */
/*									interactive_streamline_dialog_set_streamline(
										interactive_streamline_dialog->dialog, init_data);*/
									callback.procedure = interactive_streamline_dialog_update_selection;
									callback.data = interactive_streamline_dialog;
									SELECT_SET_UPDATE_CB(Interactive_streamline)(
										interactive_streamline_dialog->select_widget, &callback);
									XtRealizeWidget(interactive_streamline_dialog->dialog);
									XtPopup(interactive_streamline_dialog->dialog, XtGrabNone);
									return_widget=interactive_streamline_dialog->dialog;
								}
								else
								{
									DEALLOCATE(interactive_streamline_dialog);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
			"create_interactive_streamline_dialog.  Could not fetch interactive_streamline_dialog");
								DEALLOCATE(interactive_streamline_dialog);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
							"create_interactive_streamline_dialog.  Could not register identifiers");
							DEALLOCATE(interactive_streamline_dialog);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_interactive_streamline_dialog.  Could not register callbacks");
						DEALLOCATE(interactive_streamline_dialog);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_interactive_streamline_dialog.  Could not create popup shell.");
					DEALLOCATE(interactive_streamline_dialog);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
	"create_interactive_streamline_dialog.  Could not allocate interactive_streamline_dialog");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_interactive_streamline_dialog.  Could not open hierarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_interactive_streamline_dialog.  Invalid argument(s)");
	}
	if (interactive_streamline_dialog_widget&&return_widget)
	{
		*interactive_streamline_dialog_widget=return_widget;
	}
	LEAVE;

	return (return_widget);
} /* create_interactive_streamline_dialog */

/*
Global functions
----------------
*/
int bring_up_interactive_streamline_dialog(
	Widget *interactive_streamline_dialog_address,Widget parent,
	struct MANAGER(Interactive_streamline) *interactive_streamline_manager,
	struct Interactive_streamline *streamline,
	struct User_interface *user_interface,struct MANAGER(Scene) *scene_manager)
/*******************************************************************************
LAST MODIFIED : 9 November 1998

DESCRIPTION :
If there is a interactive_streamline dialog in existence, then bring it to the
front, else create a new one.
==============================================================================*/
{
	int return_code;

	ENTER(bring_up_interactive_streamline_dialog);
	if (interactive_streamline_dialog_address)
	{
		if (*interactive_streamline_dialog_address)
		{
/*			interactive_streamline_dialog_set_streamline(*interactive_streamline_dialog_address,
				streamline);*/
			XtPopup(*interactive_streamline_dialog_address,XtGrabNone);
			return_code=1;
		}
		else
		{
			if (create_interactive_streamline_dialog(interactive_streamline_dialog_address,parent,
				interactive_streamline_manager,streamline,user_interface,
				scene_manager))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"bring_up_interactive_streamline_dialog.  Error creating dialog");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"bring_up_interactive_streamline_dialog.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* bring_up_interactive_streamline_dialog */
