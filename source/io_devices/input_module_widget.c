/*******************************************************************************
FILE : input_module_widget.c

LAST MODIFIED : 14 May 1998

DESCRIPTION :
This widget allows the user to accept input from certain devices.  Only valid
devices are displayed on the menu.
==============================================================================*/
#include <math.h>
#include <stdio.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include "general/callback.h"
#include "general/debug.h"
#include "io_devices/input_module_widget.h"
#include "io_devices/input_module_widget.uidh"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int input_module_widget_hierarchy_open=0;
static MrmHierarchy input_module_widget_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
#if defined (EXT_INPUT)
static void input_module_identify_button(Widget w,int button_type,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 27 March 1995

DESCRIPTION :
Finds the id of the buttons on the input_module widget.
==============================================================================*/
{
	int button_num;
	struct Input_module_widget_struct *temp_input_module;

	ENTER(input_module_identify_button);
	/* find out which input_module widget we are in */
	if (button_type==input_module_pulldown_ID)
	{
		XtVaGetValues(w,XmNuserData,&temp_input_module,NULL);
		switch (button_type)
		{
			case input_module_pulldown_ID:
			{
				temp_input_module->pulldown = w;
			}; break;
			default:
			{
				display_message(WARNING_MESSAGE,
					"input_module_identify_button.  Invalid button number");
			} break;
		}
	}
	else
	{
		XtVaGetValues(XtParent(w),XmNuserData,&temp_input_module,NULL);
		XtVaGetValues(w,XmNuserData,&button_num,NULL);
		printf("id\n");
		switch (button_type)
		{
			case input_module_device_button_ID:
			{
				temp_input_module->input[button_num] = w;
			}; break;
#if defined (POLHEMUS)
			case input_module_polhemus_button_ID:
			{
				temp_input_module->polhemus_control[button_num] = w;
			}; break;
#endif
			default:
			{
				display_message(WARNING_MESSAGE,
					"input_module_identify_button.  Invalid button number");
			} break;
		}
	}
	LEAVE;
} /* input_module_identify_button */

static void input_module_destroy_CB(Widget w,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Callback for the input_module widget - tidies up all details - mem etc
==============================================================================*/
{
	struct Input_module_widget_struct *temp_input_module;

	ENTER(input_module_destroy_CB);
	/* Get the pointer to the data for the input_module widget */
	XtVaGetValues(w,XmNuserData,&temp_input_module,NULL);
	*(temp_input_module->widget_address) = (Widget)NULL;
	DEALLOCATE(temp_input_module);
	LEAVE;
} /* input_module_destroy_CB */

static void input_module_device_CB(Widget w,int *tag,
	XmToggleButtonCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 9 April 1997

DESCRIPTION :
Callback for the input_module widget.  Processes which device has been selected
or unselected.
==============================================================================*/
{
	int device_num;
	struct Input_module_widget_struct *temp_input_module;
	struct Input_module_widget_data temp_data;

	ENTER(input_module_device_CB);
	/* Get the pointer to the data for the input_module widget */
	XtVaGetValues(XtParent(w),XmNuserData,&temp_input_module,NULL);
	XtVaGetValues(w,XmNuserData,&device_num,NULL);
	if ((device_num>=0)&&(device_num<INPUT_MODULE_NUM_DEVICES))
	{
		temp_input_module->input_device[device_num] = reason->set;
		temp_data.device = (enum Input_module_device)device_num;
		temp_data.status = reason->set;
		callback_call_list(temp_input_module->callback_list[INPUT_MODULE_DEVICE_CB],
			temp_input_module->widget,&temp_data);
	}
	LEAVE;
} /* input_module_device_CB */

static void input_module_polhemus_CB(Widget w,int *tag,
	XmToggleButtonCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Callback for the input_module widget.  Processes commands specific to the
Polhemus.
==============================================================================*/
{
	int button_num;
	struct Input_module_widget_struct *temp_input_module;

	ENTER(input_module_polhemus_CB);
	/* Get the pointer to the data for the input_module widget */
	XtVaGetValues(XtParent(w),XmNuserData,&temp_input_module,NULL);
	XtVaGetValues(w,XmNuserData,&button_num,NULL);
#if defined (POLHEMUS)
	callback_call_list(temp_input_module->callback_list[INPUT_MODULE_DEVICE_CB],
		temp_input_module->widget,&button_num);
#endif
	LEAVE;
} /* input_module_polhemus_CB */

static void input_module_add_devices(
	struct Input_module_widget_struct *temp_input_module)
/*******************************************************************************
LAST MODIFIED : 9 April 1997

DESCRIPTION :
Finds out external devices that are present, then adds them to the menu.
==============================================================================*/
{
	Arg override_arg;
	char *device_name;
	int i,any_devices;
	MrmType input_module_pulldown_class,input_module_polhemus_class;
	Widget temp_widget;
	XmString new_name;

	ENTER(input_module_add_devices);
	if (input_module_widget_hierarchy_open)
	{
		any_devices = 0;
		for(i=0;i<INPUT_MODULE_NUM_DEVICES;i++)
		{
			if (device_name=input_module_is_device_valid((enum Input_module_device)i))
			{
				any_devices = 1;
				XtSetArg(override_arg,XmNuserData,i);
				if (MrmSUCCESS==MrmFetchWidgetOverride(input_module_widget_hierarchy,
					"input_module_menu_item",temp_input_module->pulldown,NULL,
					&override_arg,1,&temp_widget,&input_module_pulldown_class))
				{
					XtManageChild(temp_widget);
					new_name = XmStringCreateSimple(device_name);
					XtVaSetValues(temp_widget,XmNlabelString,new_name,NULL);
					XmStringFree(new_name);
				}
				else
				{
					display_message(WARNING_MESSAGE,
						"input_module_add_devices.  Could not fetch menu item widget");
				}
			}
		}
		if (!any_devices)
		{
			XtVaSetValues(temp_input_module->widget,XmNsensitive,FALSE,NULL);
		}
#if defined (POLHEMUS)
		/* if the polhemus exists, then add a couple of options */
		if (input_module_is_source_valid(IM_SOURCE_POLHEMUS))
		{
			if (MrmSUCCESS==MrmFetchWidget(input_module_widget_hierarchy,
				"input_module_menu_sep",temp_input_module->pulldown,&temp_widget,
				&input_module_polhemus_class))
			{
				XtManageChild(temp_widget);
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"input_module_add_devices.  Could not fetch menu separator widget");
			}
			if (MrmSUCCESS==MrmFetchWidget(input_module_widget_hierarchy,
				"input_module_menu_pol",temp_input_module->pulldown,&temp_widget,
				&input_module_polhemus_class))
			{
				XtManageChild(temp_widget);
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"input_module_add_devices.  Could not fetch menu polhemus widget");
			}
		}
#endif
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"input_module_add_devices.  Hierarchy is not open");
	}
	LEAVE;
} /* input_module_add_devices */
#endif

/*
Global Functions
----------------
*/
Widget create_input_module_widget(Widget *input_module_widget,Widget parent)
/*******************************************************************************
LAST MODIFIED : 1 March 1997

DESCRIPTION :
Creates a widget that will allow the user to select what input goes to a
certain client.
==============================================================================*/
{
	Widget return_widget;
#if defined (EXT_INPUT)
	int i,n;
	MrmType input_module_dialog_class;
	struct Input_module_widget_struct *temp_input_module = NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"input_module_identify_button",(XtPointer)input_module_identify_button},
		{"input_module_destroy_CB",(XtPointer)input_module_destroy_CB},
		{"input_module_device_CB",(XtPointer)input_module_device_CB},
		{"input_module_polhemus_CB",(XtPointer)input_module_polhemus_CB}
	};
	static MrmRegisterArg identifier_list[] =
	{
		{"Input_module_structure",(XtPointer)NULL},
		{"input_module_device_button_ID",(XtPointer)input_module_device_button_ID},
		{"input_module_polhemus_button_ID",
			(XtPointer)input_module_polhemus_button_ID},
		{"input_module_pulldown_ID",(XtPointer)input_module_pulldown_ID}
	};
#endif

	ENTER(create_input_module_widget);
	return_widget = (Widget)NULL;
#if defined (EXT_INPUT)
	if (MrmOpenHierarchy_base64_string(input_module_widget_uidh,
		&input_module_widget_hierarchy,&input_module_widget_hierarchy_open))
	{
		/* allocate memory */
		if (ALLOCATE(temp_input_module,struct Input_module_widget_struct,1))
		{
			/* initialise the structure */
			temp_input_module->widget_parent = parent;
			temp_input_module->widget = (Widget)NULL;
			temp_input_module->widget_address = input_module_widget;
			temp_input_module->pulldown = (Widget)NULL;
			for (n=0;n<INPUT_MODULE_NUM_DEVICES;n++)
			{
				temp_input_module->input[n] = (Widget)NULL;
				temp_input_module->input_device[n] = 0;
			};
#if defined (POLHEMUS)
			for (n=0;n<2;n++)
			{
				temp_input_module->polhemus_control[n] = (Widget)NULL;
			};
#endif
			for(i=0;i<INPUT_MODULE_NUM_CALLBACKS;i++)
			{
				temp_input_module->callback_list[i]=CREATE_LIST(Callback_data)();
			}
			/* register the callbacks */
			if (MrmSUCCESS==MrmRegisterNamesInHierarchy(input_module_widget_hierarchy,
				callback_list,XtNumber(callback_list)))
			{
				/* assign and register the identifiers */
				identifier_list[0].value=(XtPointer)temp_input_module;
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
					input_module_widget_hierarchy,identifier_list,
					XtNumber(identifier_list)))
				{
					/* fetch input_module window widget */
					if (MrmSUCCESS==MrmFetchWidget(input_module_widget_hierarchy,
						"input_module_widget",temp_input_module->widget_parent,
						&(temp_input_module->widget),&input_module_dialog_class))
					{
						XtManageChild(temp_input_module->widget);
						input_module_add_devices(temp_input_module);
						return_widget = temp_input_module->widget;
						*(temp_input_module->widget_address) = return_widget;
					}
					else
					{
						display_message(ERROR_MESSAGE,
						"create_input_module_widget.  Could not fetch input_module widget");
						DEALLOCATE(temp_input_module);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_input_module_widget.  Could not register identifiers");
					DEALLOCATE(temp_input_module);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_input_module_widget.  Could not register callbacks");
				DEALLOCATE(temp_input_module);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
"create_input_module_widget.  Could not allocate input_module widget structure"
				);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_input_module_widget.  Could not open hierarchy");
	}
#endif /* defined (EXT_INPUT) */
	LEAVE;

	return (return_widget);
} /* create_input_module_widget */

int input_module_widget_set_data(Widget input_module_widget,
	enum Input_module_widget_data_type data_type,void *data)
/*******************************************************************************
LAST MODIFIED : 26 February 1998

DESCRIPTION :
Changes a data item of the input_module widget.
==============================================================================*/
{
	int return_code;
#if defined (EXT_INPUT)
	struct Input_module_widget_struct *temp_input_module;
	struct Callback_data *new_callback;
#endif

	ENTER(input_module_widget_set_data);
#if defined (EXT_INPUT)
	/* Get the pointer to the data for the input_module dialog */
	XtVaGetValues(input_module_widget,XmNuserData,&temp_input_module,NULL);
	switch (data_type)
	{
		case INPUT_MODULE_DEVICE_CB:
		{
			return_code = 0;
			if (ALLOCATE(new_callback,struct Callback_data,1))
			{
				new_callback->procedure = ((struct Callback_data *)data)->procedure;
				new_callback->data = ((struct Callback_data *)data)->data;
				ADD_OBJECT_TO_LIST(Callback_data)(new_callback,
					temp_input_module->callback_list[INPUT_MODULE_DEVICE_CB]);
				return_code = 1;
			}
		}; break;
		case INPUT_MODULE_POLHEMUS_CB:
		{
			return_code = 0;
			if (ALLOCATE(new_callback,struct Callback_data,1))
			{
				new_callback->procedure = ((struct Callback_data *)data)->procedure;
				new_callback->data = ((struct Callback_data *)data)->data;
				ADD_OBJECT_TO_LIST(Callback_data)(new_callback,
					temp_input_module->callback_list[INPUT_MODULE_POLHEMUS_CB]);
				return_code = 1;
			}
		}; break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"input_module_widget_set_data.  Invalid data type.");
			return_code = 0;
		}; break;
	}
#else
	return_code = 0;
#endif
	LEAVE;

	return (return_code);
} /* input_module_widget_set_data */
