/*******************************************************************************
FILE : input_module_widget.c

LAST MODIFIED : 23 March 2000

DESCRIPTION :
This widget allows the user to accept input from certain devices.  Only valid
devices are displayed on the menu.
==============================================================================*/
#include <math.h>
#include <stdio.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include "general/callback_private.h"
#include "general/debug.h"
#include "io_devices/input_module_widget.h"
#if defined (EXT_INPUT)
#include "io_devices/input_module_widget.uidh"
#endif /* defined (EXT_INPUT) */
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (EXT_INPUT)
static int input_module_widget_hierarchy_open=0;
static MrmHierarchy input_module_widget_hierarchy;
#endif /* defined (EXT_INPUT) */

/*
Module types
------------
*/

FULL_DECLARE_CALLBACK_TYPES(Input_module_device_change,Widget, \
	struct Input_module_widget_data *);
FULL_DECLARE_CALLBACK_TYPES(Input_module_polhemus_change,Widget, \
	int/*button_num*/);

/*
Module functions
----------------
*/
DEFINE_CALLBACK_MODULE_FUNCTIONS(Input_module_device_change)
DEFINE_CALLBACK_MODULE_FUNCTIONS(Input_module_polhemus_change)

DEFINE_CALLBACK_FUNCTIONS(Input_module_device_change,Widget, \
	struct Input_module_widget_data *)
DEFINE_CALLBACK_FUNCTIONS(Input_module_polhemus_change,Widget,int/*button_num*/)

#if defined (EXT_INPUT)
static void input_module_identify_button(Widget w,int button_type,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Finds the id of the buttons on the input_module widget.
==============================================================================*/
{
	int button_num;
	struct Input_module_widget_struct *temp_input_module;

	ENTER(input_module_identify_button);
	USE_PARAMETER(reason);
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
LAST MODIFIED : 23 March 2000

DESCRIPTION :
Callback for the input_module widget - tidies up all details - mem etc
==============================================================================*/
{
	struct Input_module_widget_struct *temp_input_module;

	ENTER(input_module_destroy_CB);
	USE_PARAMETER(user_data);
	USE_PARAMETER(call_data);
	/* Get the pointer to the data for the input_module widget */
	XtVaGetValues(w,XmNuserData,&temp_input_module,NULL);
	if (temp_input_module)
	{
		*(temp_input_module->widget_address) = (Widget)NULL;
		DESTROY(LIST(CALLBACK_ITEM(Input_module_device_change)))(
			&(temp_input_module->device_change_callback_list));
		DESTROY(LIST(CALLBACK_ITEM(Input_module_polhemus_change)))(
			&(temp_input_module->polhemus_change_callback_list));
		DEALLOCATE(temp_input_module);
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"input_module_destroy_CB.  Missing input_module");
	}
	LEAVE;
} /* input_module_destroy_CB */

static void input_module_device_CB(Widget w,int *tag,
	XmToggleButtonCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Callback for the input_module widget.  Processes which device has been selected
or unselected.
==============================================================================*/
{
	int device_num;
	struct Input_module_widget_struct *temp_input_module;
	struct Input_module_widget_data temp_data;

	ENTER(input_module_device_CB);
	USE_PARAMETER(tag);
	/* Get the pointer to the data for the input_module widget */
	XtVaGetValues(XtParent(w),XmNuserData,&temp_input_module,NULL);
	XtVaGetValues(w,XmNuserData,&device_num,NULL);
	if ((device_num>=0)&&(device_num<INPUT_MODULE_NUM_DEVICES))
	{
		temp_input_module->input_device[device_num] = reason->set;
		temp_data.device = (enum Input_module_device)device_num;
		temp_data.status = reason->set;
		CALLBACK_LIST_CALL(Input_module_device_change)(
			temp_input_module->device_change_callback_list,
			temp_input_module->widget,&temp_data);
	}
	LEAVE;
} /* input_module_device_CB */

static void input_module_polhemus_CB(Widget w,int *tag,
	XmToggleButtonCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Callback for the input_module widget.  Processes commands specific to the
Polhemus.
==============================================================================*/
{
	int button_num;
	struct Input_module_widget_struct *temp_input_module;

	ENTER(input_module_polhemus_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the input_module widget */
	XtVaGetValues(XtParent(w),XmNuserData,&temp_input_module,NULL);
	XtVaGetValues(w,XmNuserData,&button_num,NULL);
#if defined (POLHEMUS)
	CALLBACK_LIST_CALL(Input_module_polhemus_change)(
		temp_input_module->polhemus_change_callback_list,
		temp_input_module->widget,button_num);
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
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Creates a widget that will allow the user to select what input goes to a
certain client.
==============================================================================*/
{
	Widget return_widget;
#if defined (EXT_INPUT)
	int n;
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
			temp_input_module->device_change_callback_list=
				CREATE(LIST(CALLBACK_ITEM(Input_module_device_change)))();
			temp_input_module->polhemus_change_callback_list=
				CREATE(LIST(CALLBACK_ITEM(Input_module_polhemus_change)))();
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
#else /* defined (EXT_INPUT) */
	USE_PARAMETER(input_module_widget);
	USE_PARAMETER(parent);
#endif /* defined (EXT_INPUT) */
	LEAVE;

	return (return_widget);
} /* create_input_module_widget */

int Input_module_add_device_change_callback(Widget input_module_widget,
	CALLBACK_FUNCTION(Input_module_device_change) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Asks for callbacks to <function> with <user_data> when device change events
occur in the <input_module_widget>.
==============================================================================*/
{
	int return_code;

	ENTER(Input_module_add_device_change_callback);
#if defined (EXT_INPUT)
	if (input_module_widget&&function)
	{
		struct Input_module_widget_struct *temp_input_module;

		/* Get the pointer to the data for the input_module dialog */
		XtVaGetValues(input_module_widget,XmNuserData,&temp_input_module,NULL);
		if (temp_input_module&&
			CALLBACK_LIST_ADD_CALLBACK(Input_module_device_change)(
				temp_input_module->device_change_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Input_module_add_device_change_callback.  Could not add callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Input_module_add_device_change_callback.  Invalid argument(s)");
		return_code=0;
	}
#else
	USE_PARAMETER(input_module_widget);
	USE_PARAMETER(function);
	USE_PARAMETER(user_data);
	return_code = 0;
#endif
	LEAVE;

	return (return_code);
} /* Input_module_add_device_change_callback */

int Input_module_remove_device_change_callback(Widget input_module_widget,
	CALLBACK_FUNCTION(Input_module_device_change) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Stops getting callbacks to <function> with <user_data> when device change events
occur in the <input_module_widget>.
==============================================================================*/
{
	int return_code;

	ENTER(Input_module_remove_device_change_callback);
#if defined (EXT_INPUT)
	if (input_module_widget&&function)
	{
		struct Input_module_widget_struct *temp_input_module;

		/* Get the pointer to the data for the input_module dialog */
		XtVaGetValues(input_module_widget,XmNuserData,&temp_input_module,NULL);
		if (temp_input_module&&
			CALLBACK_LIST_REMOVE_CALLBACK(Input_module_device_change)(
				temp_input_module->device_change_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Input_module_remove_device_change_callback.  "
				"Could not remove callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Input_module_remove_device_change_callback.  Invalid argument(s)");
		return_code=0;
	}
#else
	USE_PARAMETER(input_module_widget);
	USE_PARAMETER(function);
	USE_PARAMETER(user_data);
	return_code = 0;
#endif
	LEAVE;

	return (return_code);
} /* Input_module_remove_device_change_callback */

int Input_module_add_polhemus_change_callback(Widget input_module_widget,
	CALLBACK_FUNCTION(Input_module_polhemus_change) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Asks for callbacks to <function> with <user_data> when polhemus change events
occur in the <input_module_widget>.
==============================================================================*/
{
	int return_code;

	ENTER(Input_module_add_polhemus_change_callback);
#if defined (EXT_INPUT)
	if (input_module_widget&&function)
	{
		struct Input_module_widget_struct *temp_input_module;

		/* Get the pointer to the data for the input_module dialog */
		XtVaGetValues(input_module_widget,XmNuserData,&temp_input_module,NULL);
		if (temp_input_module&&
			CALLBACK_LIST_ADD_CALLBACK(Input_module_polhemus_change)(
				temp_input_module->polhemus_change_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Input_module_add_polhemus_change_callback.  Could not add callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Input_module_add_polhemus_change_callback.  Invalid argument(s)");
		return_code=0;
	}
#else
	USE_PARAMETER(input_module_widget);
	USE_PARAMETER(function);
	USE_PARAMETER(user_data);
	return_code = 0;
#endif
	LEAVE;

	return (return_code);
} /* Input_module_add_polhemus_change_callback */

int Input_module_remove_polhemus_change_callback(Widget input_module_widget,
	CALLBACK_FUNCTION(Input_module_polhemus_change) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Stops getting callbacks to <function> with <user_data> when polhemus change
events occur in the <input_module_widget>.
==============================================================================*/
{
	int return_code;

	ENTER(Input_module_remove_polhemus_change_callback);
#if defined (EXT_INPUT)
	if (input_module_widget&&function)
	{
		struct Input_module_widget_struct *temp_input_module;

		/* Get the pointer to the data for the input_module dialog */
		XtVaGetValues(input_module_widget,XmNuserData,&temp_input_module,NULL);
		if (temp_input_module&&
			CALLBACK_LIST_REMOVE_CALLBACK(Input_module_polhemus_change)(
				temp_input_module->polhemus_change_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Input_module_remove_polhemus_change_callback.  "
				"Could not remove callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Input_module_remove_polhemus_change_callback.  Invalid argument(s)");
		return_code=0;
	}
#else
	USE_PARAMETER(input_module_widget);
	USE_PARAMETER(function);
	USE_PARAMETER(user_data);
	return_code = 0;
#endif
	LEAVE;

	return (return_code);
} /* Input_module_remove_polhemus_change_callback */
