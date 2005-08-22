/*******************************************************************************
FILE : sync_2d_3d.c

LAST MODIFIED : 8 August 2002

DESCRIPTION :
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
#include <stdlib.h>
#include "data/sync_2d_3d.h"
#include "data/sync_2d_3d.uidh"
#include "general/debug.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#include "data/node_transform.h"
#if defined (MOTIF)
#include <Xm/Xm.h>
#include <X11/Xlib.h>
#include <Xm/Text.h>
#include <Mrm/MrmPublic.h>
#include "select/select_finite_element.h"
#endif
#if defined (WIN32_USER_INTERFACE)
#include <windows.h>
#include <windowsx.h>
#endif /* defined (WIN32_USER_INTERFACE) */
#include "user_interface/gui_declaration.h"

/*
Module constants
----------------
*/
#define COORDINATES_2D_FIELD_NAME "coordinates_2d_rc"
#define COORDINATES_3D_FIELD_NAME "coordinates_3d_rc"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int sync_2d_3d_hierarchy_open=0;
static MrmHierarchy sync_2d_3d_hierarchy;
#endif /* defined (MOTIF) */
struct DIALOG_STRUCT(Sync_2d_3d) *global_Sync_2d_3d=
	(struct DIALOG_STRUCT(Sync_2d_3d) *)NULL;
#define Sync_2d_3d_RESTRICTED_NAME "restricted"
#define Sync_2d_3d_UNRESTRICTED_NAME "unrestricted"

/*
Module functions
----------------
*/
static void sync_2d_3d_select_update_CB(Widget widget,
	void *user_data,void *callback_data)
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
The node_group has changed.
==============================================================================*/
{
	DIALOG_WRAPPER_VARIABLES(Sync_2d_3d);

	ENTER(sync_2d_3d_select_update_CB);
	USE_PARAMETER(widget);
	if (temp_dialog=(struct DIALOG_STRUCT(Sync_2d_3d) *)user_data)
	{
		DIALOG_FUNCTION(Sync_2d_3d,merge_nodes)(temp_dialog);
		if (temp_dialog->data.source_node_group)
		{
			DEACCESS(GROUP(FE_node))(&(temp_dialog->data.source_node_group));
		}
		if (callback_data)
		{
			temp_dialog->data.source_node_group=
				ACCESS(GROUP(FE_node))((struct GROUP(FE_node) *)callback_data);
		}
		else
		{
			/* NULL is still valid for this widget */
			temp_dialog->data.source_node_group=(struct GROUP(FE_node) *)NULL;
		}
		/* Synchronise the display */
		DIALOG_FUNCTION(Sync_2d_3d,update_windows)(temp_dialog);
	}
	else
	{
		display_message(WARNING_MESSAGE,"sync_2d_3d_select_update_CB.  %s",
			"Invalid arguments");
	}
	LEAVE;
} /* sync_2d_3d_select_update_CB */

#define dialog_name Sync_2d_3d
DECLARE_LOCAL_GUI_DIALOG_FUNCTIONS(Sync_2d_3d)

int DIALOG_DATA_INITIALISE(Sync_2d_3d)(DIALOG_DATA_PARAM(Sync_2d_3d),
	DIALOG_INITIALISATION_STRUCT(Sync_2d_3d) *initial_data)
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION:
Initialises the user data.
==============================================================================*/
{
	int return_code;

	return_code=0;
	ENTER(DIALOG_DATA_INITIALISE(Sync_2d_3d));
	USE_PARAMETER(temp_dialog);
	if (initial_data)
	{
		/* start off with no prompt */
		if (ALLOCATE(temp_data->image_base_name,char,
			strlen(initial_data->image_base_name)+1))
		{
			strcpy(temp_data->image_base_name,initial_data->image_base_name);
			temp_data->axis_name=initial_data->axis_name;
			temp_data->depth=initial_data->depth;
			temp_data->thickness=initial_data->thickness;
			temp_data->execute_command=initial_data->execute_command;
			temp_data->fe_field_manager=initial_data->fe_field_manager;
			temp_data->node_manager=initial_data->node_manager;
			temp_data->node_group_manager=initial_data->node_group_manager;
			/* NULL these in case we try to use them */
			/* ???GMH. - these are temporaries, and should not be in this structure */
			temp_data->temp_source_node_group=(struct GROUP(FE_node) *)NULL;
			temp_data->temp_destination_node_group=(struct GROUP(FE_node) *)NULL;
			/* dont access, because we get updated if it dies */
			if (initial_data->source_node_group)
			{
				temp_data->source_node_group=
					ACCESS(GROUP(FE_node))(initial_data->source_node_group);
			}
			else
			{
				/* NULL is still valid for this widget */
				temp_data->source_node_group=(struct GROUP(FE_node) *)NULL;
			}
			/* determine if the group already exists */
			if (!(temp_data->destination_node_group=
				FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
					Sync_2d_3d_RESTRICTED_NAME,
					initial_data->node_group_manager)))
			{
				/* no, so create it */
				temp_data->destination_node_group=CREATE_GROUP(FE_node)(
					Sync_2d_3d_RESTRICTED_NAME);
				ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(temp_data->destination_node_group,
					initial_data->node_group_manager);
			}
			if (temp_data->destination_node_group)
			{
				/* make sure we have it */
				ACCESS(GROUP(FE_node))(temp_data->destination_node_group);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"DIALOG_DATA_INITIALISE(Sync_2d_3d).  %s",
					"Could not create destination node group");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"DIALOG_DATA_INITIALISE(Sync_2d_3d).  %s",
				"Could not allocate memory for image_base_name");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"DIALOG_DATA_INITIALISE(Sync_2d_3d).  %s",
			"Invalid arguments");
	}
	LEAVE;

	return (return_code);
} /* DIALOG_DATA_INITIALISE(Sync_2d_3d) */

int DIALOG_DATA_DESTROY(Sync_2d_3d)(DIALOG_DATA_PARAM(Sync_2d_3d))
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION:
Destroys the user data.
==============================================================================*/
{
	int return_code;

	ENTER(DIALOG_DATA_DESTROY(Sync_2d_3d));
	/* ensure the nodes go back to the main group */
	DIALOG_FUNCTION(Sync_2d_3d,merge_nodes)(temp_dialog);
	/* Destroy the prompt */
	DEALLOCATE(temp_data->image_base_name);
	if (temp_data->source_node_group)
	{
		DEACCESS(GROUP(FE_node))(&(temp_data->source_node_group));
	}
	DEACCESS(GROUP(FE_node))(&(temp_data->destination_node_group));
	return_code=1;
	LEAVE;

	return (return_code);
} /* DIALOG_DATA_DESTROY(Sync_2d_3d) */

#if defined (MOTIF)
int DIALOG_WINDOW_INITIALISE(Sync_2d_3d)(DIALOG_LOCAL_PARAM(Sync_2d_3d))
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION:
Identifies components of the dialog.
==============================================================================*/
{
	int return_code;

	return_code=0;
	ENTER(DIALOG_WINDOW_INITIALISE(Sync_2d_3d));
	if (temp_dialog)
	{
		return_code=1;
		if (return_code)
		{
			XmTextSetString(temp_dialog->image_base_name,
				temp_dialog->data.image_base_name);
			/* print the char */
			sprintf(global_temp_string,"%c",temp_dialog->data.axis_name);
			XmTextSetString(temp_dialog->axis_name,
				global_temp_string);
			/* print the depth */
			sprintf(global_temp_string,"%f",temp_dialog->data.depth);
			XmTextSetString(temp_dialog->depth,
				global_temp_string);
			sprintf(global_temp_string,"%f",temp_dialog->data.thickness);
			XmTextSetString(temp_dialog->thickness,
				global_temp_string);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DIALOG_WINDOW_INITIALISE(Sync_2d_3d).  %s",
				"Could not identify windows");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"DIALOG_WINDOW_INITIALISE(Sync_2d_3d).  %s",
			"Invalid arguments");
	}
	LEAVE;

	return (return_code);
} /* DIALOG_WINDOW_INITIALISE(Sync_2d_3d) */
#endif /* MOTIF */

#if defined (WIN32_USER_INTERFACE)
int DIALOG_WINDOW_INITIALISE(Sync_2d_3d)(DIALOG_LOCAL_PARAM(Sync_2d_3d))
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION:
Identifies components of the dialog.
==============================================================================*/
{
	int return_code;

	return_code=0;
	ENTER(DIALOG_WINDOW_INITIALISE(Sync_2d_3d));
	if (temp_dialog)
	{
		return_code=1;
		/* Identify the components of the box */
		if ((temp_dialog->image_base_name=GetDlgItem(temp_dialog->dialog,
			IDC_image_base_name))==NULL)
		{
			return_code=0;
		}
		if (return_code)
		{
		}
		else
		{
			display_message(ERROR_MESSAGE,"DIALOG_WINDOW_INITIALISE(Sync_2d_3d).  %s",
				"Could not identify windows");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"DIALOG_WINDOW_INITIALISE(Sync_2d_3d).  %s",
			"Invalid arguments");
	}
	LEAVE;

	return (return_code);
} /* DIALOG_WINDOW_INITIALISE(Sync_2d_3d) */
#endif /* WIN32_USER_INTERFACE */

int DIALOG_WINDOW_DESTROY(Sync_2d_3d)(DIALOG_LOCAL_PARAM(Sync_2d_3d))
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION:
Identifies components of the dialog.
==============================================================================*/
{
	int return_code;

	return_code=0;
	ENTER(DIALOG_WINDOW_DESTROY(Sync_2d_3d));
	if (temp_dialog)
	{
		/* do nothing */
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DIALOG_WINDOW_DESTROY(Sync_2d_3d).  %s",
			"Invalid arguments");
	}
	LEAVE;

	return (return_code);
} /* DIALOG_WINDOW_DESTROY(Sync_2d_3d) */

static void sync_2d_3d_identify_button(Widget dialog,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Identifies all controls of the dialog.
==============================================================================*/
{
	DIALOG_WRAPPER_VARIABLES(dialog_name);

	ENTER(sync_2d_3d_identify_button);
	USE_PARAMETER(reason);
	if (DIALOG_WRAPPER_INITIALISE_VARIABLES(dialog_name))
	{
		switch (button_num)
		{
			case IDC_image_base_name:
			{
				temp_dialog->image_base_name=dialog;
			} break;
			case IDC_axis_name:
			{
				temp_dialog->axis_name=dialog;
			} break;
			case IDC_depth:
			{
				temp_dialog->depth=dialog;
			} break;
			case IDC_thickness:
			{
				temp_dialog->thickness=dialog;
			} break;
			case IDC_node_group_form:
			{
				temp_dialog->node_group_form=dialog;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"sync_2d_3d_identify_button.  %s",
					"Invalid button number");
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"sync_2d_3d_identify_button.  %s",
			"Could not initialise dialog variables");
	}
	LEAVE;
} /* sync_2d_3d_identify_button */

static void sync_2d_3d_item_CB(Widget dialog,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Identifies all controls of the dialog.
==============================================================================*/
{
	DIALOG_WRAPPER_VARIABLES(dialog_name);
	char *temp_string;

	ENTER(sync_2d_3d_item_CB);
	USE_PARAMETER(reason);
	if (DIALOG_WRAPPER_INITIALISE_VARIABLES(dialog_name))
	{
		temp_string=XmTextGetString(dialog);
		switch (button_num)
		{
			case IDC_image_base_name:
			{
				if (REALLOCATE(temp_dialog->data.image_base_name,
					temp_dialog->data.image_base_name,char,
					strlen(temp_string)+1))
				{
					strcpy(temp_dialog->data.image_base_name,temp_string);
				}
				else
				{
					display_message(ERROR_MESSAGE,"sync_2d_3d_item_CB.  %s",
						"Could not reallocate string");
				}
			} break;
			case IDC_axis_name:
			{
				DIALOG_FUNCTION(Sync_2d_3d,set_axis_name)(temp_dialog,temp_string[0]);
			} break;
			case IDC_depth:
			{
				DIALOG_FUNCTION(Sync_2d_3d,set_depth)(temp_dialog,atof(temp_string));
			} break;
			case IDC_thickness:
			{
				DIALOG_FUNCTION(Sync_2d_3d,set_thickness)(temp_dialog,
					atof(temp_string));
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,"sync_2d_3d_item_CB.  %s",
					"Invalid button number");
			} break;
		}
		XtFree(temp_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,"sync_2d_3d_item_CB.  %s",
			"Could not initialise dialog variables");
	}
	LEAVE;
} /* sync_2d_3d_item_CB */

static void sync_2d_3d_destroy_CB(Widget dialog,int *tag,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Callback for the sync_2d_3d dialog - tidies up all memory allocation
==============================================================================*/
{
	DIALOG_WRAPPER_VARIABLES(dialog_name);

	ENTER(sync_2d_3d_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (DIALOG_WRAPPER_INITIALISE_VARIABLES(dialog_name))
	{
		if (DIALOG_DATA_DESTROY(dialog_name)(temp_dialog,&(temp_dialog->data)))
		{
			if (DIALOG_WINDOW_DESTROY(dialog_name)(temp_dialog))
			{
				DEALLOCATE(temp_dialog);
			}
			else
			{
				display_message(ERROR_MESSAGE,"sync_2d_3d_destroy_CB.  %s",
					"Could not destroy window data");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"sync_2d_3d_destroy_CB.  %s",
				"Could not destroy dialog data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"sync_2d_3d_destroy_CB.  %s",
			"Could not initialise dialog variables");
	}
	LEAVE;
} /* sync_2d_3d_destroy_CB */

static void sync_2d_3d_control_CB(Widget dialog,int button_num,
	XmAnyCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Performs actions depending upon the button pressed.
==============================================================================*/
{
	DIALOG_WRAPPER_VARIABLES(dialog_name);

	ENTER(sync_2d_3d_control_CB);
	USE_PARAMETER(reason);
	if (DIALOG_WRAPPER_INITIALISE_VARIABLES(dialog_name))
	{
		switch (button_num)
		{
			case IDC_sync_button:
			{
				DIALOG_FUNCTION(Sync_2d_3d,merge_nodes)(temp_dialog);
				DIALOG_FUNCTION(Sync_2d_3d,update_windows)(temp_dialog);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,"sync_2d_3d_control_CB.  %s",
					"Invalid button number");
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"sync_2d_3d_control_CB.  %s",
			"Could not initialise dialog variables");
	}
	LEAVE;
} /* sync_2d_3d_control_CB */

/*
Global Functions
---------------
*/
PROTOTYPE_DIALOG_CREATE_FUNCTION(dialog_name)
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Creates a sync_2d_3d widget that will allow the user to choose an object based
upon its name.
==============================================================================*/
{
	int result;
	MrmType sync_2d_3d_dialog_class;
	static MrmRegisterArg callback_list[]=
	{
		{"sync_2d_3d_identify_button",(XtPointer)sync_2d_3d_identify_button},
		{"sync_2d_3d_destroy_CB",(XtPointer)sync_2d_3d_destroy_CB},
		{"sync_2d_3d_control_CB",(XtPointer)sync_2d_3d_control_CB},
		{"sync_2d_3d_item_CB",(XtPointer)sync_2d_3d_item_CB},
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"Sync_2d_3d_structure",(XtPointer)NULL},
		{"sync_2d_3d_image_base_name_ID",(XtPointer)IDC_image_base_name},
		{"sync_2d_3d_axis_name_ID",(XtPointer)IDC_axis_name},
		{"sync_2d_3d_depth_ID",(XtPointer)IDC_depth},
		{"sync_2d_3d_thickness_ID",(XtPointer)IDC_thickness},
		{"sync_2d_3d_node_group_form_ID",(XtPointer)IDC_node_group_form},
		{"sync_2d_3d_sync_button_ID",(XtPointer)IDC_sync_button},
	};
	struct Callback_data callback;
	struct DIALOG_STRUCT(dialog_name) *return_struct;

	return_struct=(struct DIALOG_STRUCT(dialog_name) *)NULL;
	ENTER(DIALOG_CREATE(dialog_name));
	if (MrmOpenHierarchy_base64_string(sync_2d_3d_uidh,
		&sync_2d_3d_hierarchy,&sync_2d_3d_hierarchy_open))
	{
		/* allocate memory for the structure */
		if (ALLOCATE(return_struct,struct DIALOG_STRUCT(dialog_name),1))
		{
			result=0;
			/* Set everything to zero (cheats way) */
			memset(return_struct,0,sizeof(struct DIALOG_STRUCT(dialog_name)));
			return_struct->parent=parent;
			/* make the dialog shell */
			if (return_struct->dialog=XtVaCreatePopupShell("2D Digitiser",
				topLevelShellWidgetClass,return_struct->parent,XmNallowShellResize,
				TRUE,NULL))
			{
				/* these bits go into the window initialise bit */
				/* register the callbacks */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(sync_2d_3d_hierarchy,
					callback_list,XtNumber(callback_list)))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)return_struct;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(sync_2d_3d_hierarchy,
						identifier_list,XtNumber(identifier_list)))
					{
						/* fetch sync_2d_3d control widget */
						if (MrmSUCCESS==MrmFetchWidget(sync_2d_3d_hierarchy,
							"sync_2d_3d_dialog",return_struct->dialog,
							&(return_struct->window),&sync_2d_3d_dialog_class))
						{
							XtManageChild(return_struct->window);
							/* initialise the data */
							if (DIALOG_DATA_INITIALISE(dialog_name)(return_struct,
								&(return_struct->data),initial_data))
							{
								if (DIALOG_WINDOW_INITIALISE(dialog_name)(return_struct))
								{
									/* this whole area needs recoding (properly) */
									if (!CREATE_SELECT_WIDGET(GROUP(FE_node))(
										&return_struct->node_group,
										return_struct->node_group_form,
										SELECT_LIST,
										return_struct->data.source_node_group,
										initial_data->node_group_manager))
									{
										display_message(ERROR_MESSAGE,
											"DIALOG_CREATE(dialog_name).  %s",
											"Could not create node group selection");
									}
									callback.procedure=sync_2d_3d_select_update_CB;
									callback.data=return_struct;
									SELECT_SET_UPDATE_CB(GROUP(FE_node))(
										return_struct->node_group,&callback);
									XtRealizeWidget(return_struct->dialog);
									XtPopup(return_struct->dialog, XtGrabNone);
									result=1;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"DIALOG_CREATE(dialog_name).  %s",
										"Could not initialise dialog window");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"DIALOG_CREATE(dialog_name).  %s",
									"Could not initialise dialog data");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"DIALOG_CREATE(dialog_name).  %s",
								"Could not fetch dialog");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"DIALOG_CREATE(dialog_name).  %s",
							"Could not register identifiers in hierachy");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"DIALOG_CREATE(dialog_name).  %s",
						"Could not register callbacks in hierachy");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"DIALOG_CREATE(dialog_name).  %s",
					"Could not create popup shell");
			}
			if (!result)
			{
				if (DIALOG_DATA_DESTROY(dialog_name)(return_struct,
					&(return_struct->data)))
				{
					if (DIALOG_WINDOW_DESTROY(dialog_name)(return_struct))
					{
						DEALLOCATE(return_struct);
					}
					else
					{
						return_struct=(struct DIALOG_STRUCT(dialog_name) *)NULL;
						display_message(ERROR_MESSAGE,"DIALOG_CREATE(dialog_name).  %s",
							"Could not destroy dialog data");
					}
				}
				else
				{
					return_struct=(struct DIALOG_STRUCT(dialog_name) *)NULL;
					display_message(ERROR_MESSAGE,"DIALOG_CREATE(dialog_name).  %s",
						"Could not destroy dialog data");
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"DIALOG_CREATE(dialog_name).  %s",
				"Could not allocate dialog structure");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"DIALOG_CREATE(dialog_name).  %s",
			"Could not open hierarchy");
	}
	LEAVE;

	return (return_struct);
} /* DIALOG_CREATE(dialog_name) */

static int add_node_to_group(struct FE_node *node,void *user_data)
/*******************************************************************************
LAST MODIFIED : 31 August 2001

DESCRIPTION :
Adds the node to the group specified by user_data.
???GMH.  What we want to do is add a third component to the coordinate field of
the 2d node.  This current code is not robust, as it adds a new coordinate field
to the node in addition to its current one.
==============================================================================*/
{
	int return_code;
	DIALOG_VARIABLES(Sync_2d_3d);
	/* these variables needed for creating new node */
	FE_value position[3],old_position[3];
	int i;
	struct FE_node *copy_node,*template_node;
	char *component_names[3]=
	{
		"X","Y","Z"
	};
	enum FE_nodal_value_type *components_nodal_value_types[3]=
	{
		{
			FE_NODAL_VALUE
		},
		{
			FE_NODAL_VALUE
		},
		{
			FE_NODAL_VALUE
		}
	};
	int components_number_of_derivatives[3]={0,0,0},
		components_number_of_versions[3]={1,1,1};
	struct FE_field *coordinate_field,*digitised_field;
	struct Coordinate_system rect_cart_coords;

	rect_cart_coords.type =  RECTANGULAR_CARTESIAN;

	ENTER(add_node_to_group);
	return_code=0;
	if (node&&(temp_data=(struct DIALOG_DATA_STRUCT(Sync_2d_3d) *)user_data))
	{
		/* if we are 2d, then we have been added, so promote to 3d with correct */
		/* depth */
		if ((digitised_field=get_FE_field_manager_matched_field(
			temp_data->fe_field_manager,COORDINATES_2D_FIELD_NAME,
			GENERAL_FE_FIELD,/*indexer_field*/(struct FE_field *)NULL,
			/*number_of_indexed_values*/0,CM_COORDINATE_FIELD,
			/*number_of_components*/&rect_cart_coords,FE_VALUE_VALUE,
			2,component_names,
			/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
			(struct FE_field_external_information *)NULL))&&
			(coordinate_field=get_FE_field_manager_matched_field(
			temp_data->fe_field_manager,COORDINATES_3D_FIELD_NAME,
			GENERAL_FE_FIELD,/*indexer_field*/(struct FE_field *)NULL,
			/*number_of_indexed_values*/0,CM_COORDINATE_FIELD,
			/*number_of_components*/&rect_cart_coords,FE_VALUE_VALUE,
			3,component_names,
			/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
			(struct FE_field_external_information *)NULL)))
		{
			/* look for the digitised coordinates */
			if (FE_node_get_position_cartesian(node,digitised_field,old_position,
				old_position+1,old_position+2,(FE_value *)NULL))
			{
				if (FE_node_get_position_cartesian(node,coordinate_field,position,
					position+1,position+2,(FE_value *)NULL))
				{
					/* pick off the depth from the existing coordinates */
					/* assumes that axis_name is 'x','y','z' (ie lowercase) */
					old_position[2]=position[temp_data->axis_name-'x'];
				}
				else
				{
					/* pick off the image depth */
					old_position[2]=temp_data->depth;
				}
				/* exchange the coordinates */
				switch (temp_data->axis_name)
				{
					case 'x':
					{
						position[0]=old_position[2];
						position[1]=old_position[0];
						position[2]=old_position[1];
					} break;
					case 'y':
					{
						position[0]=old_position[1];
						position[1]=old_position[2];
						position[2]=old_position[0];
					} break;
					case 'z':
					{
						for (i=0;i<3;i++)
						{
							position[i]=old_position[i];
						}
					} break;
					default:
					{
						for (i=0;i<3;i++)
						{
							position[i]=old_position[i];
						}
						display_message(WARNING_MESSAGE,"add_node_to_group.  %s",
							"Invalid axis name");
					}
				}
				if ((template_node=CREATE(FE_node)(0,(struct FE_node *)NULL))
					&&define_FE_field_at_node(template_node,
					coordinate_field,components_number_of_derivatives,
					components_number_of_versions,components_nodal_value_types))
				{
					if (copy_node=CREATE(FE_node)(0,template_node))
					{					
						int length;
						set_FE_nodal_field_FE_value_values(digitised_field,copy_node,
							position,&length);

						if (MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)(node,
							copy_node,temp_data->node_manager))
						{
							return_code=ADD_OBJECT_TO_GROUP(FE_node)(node,
								temp_data->temp_source_node_group);
						}
						DESTROY(FE_node)(&copy_node);
					}
					else
					{
						display_message(ERROR_MESSAGE,"add_node_to_group.  %s",
							"Could not create FE_node");
					}
					DESTROY(FE_node)(&template_node);
				}
				else
				{
					display_message(ERROR_MESSAGE,"add_node_to_group.  %s",
						"Could not create template_node");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"add_node_to_group.  %s",
					"Invalid digitised node field");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"add_node_to_group.  %s",
				"Could not create fields");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"add_node_to_group.  %s",
			"Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* add_node_to_group */

int DIALOG_FUNCTION(Sync_2d_3d,merge_nodes)(DIALOG_GLOBAL_PARAM(Sync_2d_3d))
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Merges the nodes from the destination node group to the source (ie includes
any changes made).
==============================================================================*/
{
	int return_code;
	DIALOG_VARIABLES(Sync_2d_3d);

	ENTER(DIALOG_FUNCTION(Sync_2d_3d,merge_nodes));
	return_code=0;
	if (DIALOG_INITIALISE_VARIABLES(Sync_2d_3d))
	{
		if (temp_data->temp_destination_node_group=
			CREATE(GROUP(FE_node))((char *)NULL))
		{
			if (COPY(GROUP(FE_node))(temp_data->temp_destination_node_group,
				temp_data->destination_node_group))
			{
				/* do we have a source group */
				if (temp_data->source_node_group)
				{
					/* cache the node manager */
					MANAGER_BEGIN_CACHE(FE_node)(temp_data->node_manager);
					if (temp_data->temp_source_node_group=
						CREATE(GROUP(FE_node))((char *)NULL))
					{
						if (COPY(GROUP(FE_node))(temp_data->temp_source_node_group,
							temp_data->source_node_group))
						{
							/* this will affect source and destination groups, so we */
							/* make copies, and then tell manager to modify */
							FOR_EACH_OBJECT_IN_GROUP(FE_node)(add_node_to_group,
								(void *)temp_data,
								temp_data->temp_destination_node_group);
							/* notify the manager */
							if (!MANAGER_MODIFY_NOT_IDENTIFIER(GROUP(FE_node),name)(
								temp_data->source_node_group,
								temp_data->temp_source_node_group,
								temp_data->node_group_manager))
							{
								display_message(ERROR_MESSAGE,
									"DIALOG_FUNCTION(Sync_2d_3d,merge_nodes).  %s",
									"Could not notify source manager");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"DIALOG_FUNCTION(Sync_2d_3d,merge_nodes).  %s",
								"Could not copy temporary source node group");
						}
						DESTROY(GROUP(FE_node))(&temp_data->temp_source_node_group);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"DIALOG_FUNCTION(Sync_2d_3d,update_windows).  %s",
							"Could not create temporary source node group");
					}
				}
				/* remove all nodes in the destination group */
				REMOVE_ALL_OBJECTS_FROM_GROUP(FE_node)(
					temp_data->temp_destination_node_group);
				/* notify the manager */
				if (MANAGER_MODIFY_NOT_IDENTIFIER(GROUP(FE_node),name)(
					temp_data->destination_node_group,
					temp_data->temp_destination_node_group,
					temp_data->node_group_manager))
				{
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"DIALOG_FUNCTION(Sync_2d_3d,merge_nodes).  %s",
						"Could not notify destination manager");
				}
				/* if we cached, then we uncache */
				if (temp_data->source_node_group)
				{
					MANAGER_END_CACHE(FE_node)(temp_data->node_manager);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"DIALOG_FUNCTION(Sync_2d_3d,merge_nodes).  %s",
					"Could not copy temporary destination node group");
			}
			DESTROY(GROUP(FE_node))(&temp_data->temp_destination_node_group);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DIALOG_FUNCTION(Sync_2d_3d,merge_nodes).  %s",
				"Could not create temporary destination node group");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DIALOG_FUNCTION(Sync_2d_3d,merge_nodes).  %s",
			"Could not initialise dialog variables");
	}
	LEAVE;

	return (return_code);
} /* DIALOG_FUNCTION(Sync_2d_3d,merge_nodes) */

static int restrict_nodes(struct FE_node *node,void *user_data)
/*******************************************************************************
LAST MODIFIED : 31 August 2001

DESCRIPTION :
Adds the node to the group specified by user_data.
==============================================================================*/
{
	int return_code,coord;
	DIALOG_VARIABLES(Sync_2d_3d);
	FE_value position[2],old_position[3];
	struct FE_node *copy_node,*template_node;
	char *component_names[3]=
	{
		"X","Y","Z"
	};
	enum FE_nodal_value_type *components_nodal_value_types[3]=
	{
		{
			FE_NODAL_VALUE
		},
		{
			FE_NODAL_VALUE
		},
		{
			FE_NODAL_VALUE
		}
	};
	int components_number_of_derivatives[3]={0,0,0},
		components_number_of_versions[3]={1,1,1};
	struct FE_field *digitised_field,*coordinate_field;
	struct Coordinate_system rect_cart_coords;

	rect_cart_coords.type =  RECTANGULAR_CARTESIAN;

	ENTER(restrict_nodes);
	return_code=0;
	if (node&&(temp_data=(struct DIALOG_DATA_STRUCT(Sync_2d_3d) *)user_data))
	{
		if ((digitised_field=get_FE_field_manager_matched_field(
			temp_data->fe_field_manager,COORDINATES_2D_FIELD_NAME,
			GENERAL_FE_FIELD,/*indexer_field*/(struct FE_field *)NULL,
			/*number_of_indexed_values*/0,CM_COORDINATE_FIELD,
			&rect_cart_coords,FE_VALUE_VALUE,
			/*number_of_components*/2,component_names,
			/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
			(struct FE_field_external_information *)NULL))&&
			(coordinate_field=get_FE_field_manager_matched_field(
			temp_data->fe_field_manager,COORDINATES_3D_FIELD_NAME,
			GENERAL_FE_FIELD,/*indexer_field*/(struct FE_field *)NULL,
			/*number_of_indexed_values*/0,CM_COORDINATE_FIELD,
			&rect_cart_coords,FE_VALUE_VALUE,
			/*number_of_components*/3,component_names,
			/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
			(struct FE_field_external_information *)NULL)))
		{
			if (FE_node_get_position_cartesian(node,coordinate_field,old_position,
				old_position+1,old_position+2,(FE_value *)NULL))
			{
				coord=-1;
				switch (temp_data->axis_name)
				{
					case 'x':
					{
						coord=0;
						position[0]=old_position[1];
						position[1]=old_position[2];
					} break;
					case 'y':
					{
						coord=1;
						position[0]=old_position[2];
						position[1]=old_position[0];
					} break;
					case 'z':
					{
						coord=2;
						position[0]=old_position[0];
						position[1]=old_position[1];
					} break;
					default:
					{
						position[0]=old_position[0];
						position[1]=old_position[1];
						display_message(WARNING_MESSAGE,"restrict_nodes.  %s",
							"Invalid axis name");
					} break;
				}
				if (coord!=-1)
				{
					if ((old_position[coord]<(temp_data->depth+
						temp_data->thickness/2.0))&&
						(old_position[coord]>(temp_data->depth-temp_data->thickness/2.0)))
					{
						/* add the new field with the digitised position */
						if ((template_node=CREATE(FE_node)(0,(struct FE_node *)NULL))
							&&define_FE_field_at_node(template_node,
							digitised_field,components_number_of_derivatives,
							components_number_of_versions,components_nodal_value_types))
						{
							if (copy_node=CREATE(FE_node)(
								get_FE_node_cm_node_identifier(node),template_node))
							{								
								int length;
								set_FE_nodal_field_FE_value_values(digitised_field,copy_node,
									position,&length);

								/* merge the old node to the new node */
								merge_FE_node(copy_node,node);
								if (MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)(
									node,copy_node,temp_data->node_manager))
								{
									return_code=ADD_OBJECT_TO_GROUP(FE_node)(node,
										temp_data->temp_destination_node_group);
								}
								else
								{
									display_message(ERROR_MESSAGE,"restrict_nodes.  %s",
										"Could not modify node in manager");
								}
								DESTROY(FE_node)(&copy_node);
							}
							else
							{
								display_message(ERROR_MESSAGE,"restrict_nodes.  %s",
									"Could not create FE_node");
							}
							DESTROY(FE_node)(&template_node);
						}
						else
						{
							display_message(ERROR_MESSAGE,"restrict_nodes.  %s",
								"Could not create template_node");
						}
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"restrict_nodes.  %s",
					"Invalid coordinate node field");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"restrict_nodes.  %s",
				"Could not create fields");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"restrict_nodes.  %s",
			"Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* restrict_nodes */

int DIALOG_FUNCTION(Sync_2d_3d,update_windows)(DIALOG_GLOBAL_PARAM(Sync_2d_3d))
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Loads the correct image into xvg and filters out the correct nodes.
==============================================================================*/
{
	int return_code;
	DIALOG_VARIABLES(Sync_2d_3d);
	char *temp_string,*command_string;

	ENTER(DIALOG_FUNCTION(Sync_2d_3d,update_windows));
	return_code=0;
	if (DIALOG_INITIALISE_VARIABLES(Sync_2d_3d))
	{
		if (ALLOCATE(temp_string,char,9+strlen(temp_data->image_base_name)+1))
		{
			sprintf(temp_string,"imp load %s",temp_data->image_base_name);
			sprintf(global_temp_string,temp_string,temp_data->axis_name,
				temp_data->depth);
			/* we dont want to use a global string in the execute call */
			if (ALLOCATE(command_string,char,strlen(global_temp_string)+1))
			{
				strcpy(command_string,global_temp_string);
				Execute_command_execute_string(temp_data->execute_command, command_string);
				/* do we have a source group */
				if (temp_data->source_node_group)
				{
					if ((temp_data->temp_source_node_group=
						CREATE(GROUP(FE_node))((char *)NULL))&&
						(temp_data->temp_destination_node_group=
							CREATE(GROUP(FE_node))((char *)NULL)))
					{
						if (COPY(GROUP(FE_node))(temp_data->temp_source_node_group,
							temp_data->source_node_group)&&
							COPY(GROUP(FE_node))(temp_data->temp_destination_node_group,
							temp_data->destination_node_group))
						{
							/* cache the node manager */
							MANAGER_BEGIN_CACHE(FE_node)(temp_data->node_manager);
							/* this will affect source and destination groups, so we */
							/* make copies, and then tell manager to modify */
							REMOVE_OBJECTS_FROM_GROUP_THAT(FE_node)(restrict_nodes,
								(void *)temp_data,temp_data->temp_source_node_group);
							/* notify the manager */
							if (MANAGER_MODIFY_NOT_IDENTIFIER(GROUP(FE_node),name)(
								temp_data->source_node_group,
								temp_data->temp_source_node_group,
								temp_data->node_group_manager)&&
								MANAGER_MODIFY_NOT_IDENTIFIER(GROUP(FE_node),name)(
								temp_data->destination_node_group,
								temp_data->temp_destination_node_group,
								temp_data->node_group_manager))
							{
								return_code=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"DIALOG_FUNCTION(Sync_2d_3d,update_windows).  %s",
									"Could not notify manager");
							}
							MANAGER_END_CACHE(FE_node)(temp_data->node_manager);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"DIALOG_FUNCTION(Sync_2d_3d,update_windows).  %s",
								"Could not copy temporary node groups");
						}
						DESTROY(GROUP(FE_node))(&temp_data->temp_source_node_group);
						DESTROY(GROUP(FE_node))(&temp_data->temp_destination_node_group);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"DIALOG_FUNCTION(Sync_2d_3d,update_windows).  %s",
							"Could not create temporary node groups");
					}
				}
				DEALLOCATE(command_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"DIALOG_FUNCTION(Sync_2d_3d,update_windows).  %s",
					"Could not allocate memory for command string");
			}
			DEALLOCATE(temp_string);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DIALOG_FUNCTION(Sync_2d_3d,update_windows).  %s",
				"Could not allocate memory for temporary string");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DIALOG_FUNCTION(Sync_2d_3d,update_windows).  %s",
			"Could not initialise dialog variables");
	}
	LEAVE;

	return (return_code);
} /* DIALOG_FUNCTION(Sync_2d_3d,udpate_windows) */

int DIALOG_FUNCTION(Sync_2d_3d,set_depth)(DIALOG_GLOBAL_PARAM(Sync_2d_3d),
	double depth)
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Sets the depth of the image.
==============================================================================*/
{
	int return_code;
	DIALOG_VARIABLES(Sync_2d_3d);

	return_code=0;
	ENTER(DIALOG_FUNCTION(Sync_2d_3d,set_depth));
	if (DIALOG_INITIALISE_VARIABLES(Sync_2d_3d))
	{
		DIALOG_FUNCTION(Sync_2d_3d,merge_nodes)(temp_dialog);
		temp_data->depth=depth;
		sprintf(global_temp_string,"%f",temp_data->depth);
		XmTextSetString(temp_dialog->depth,
			global_temp_string);
		/* Synchronise the display */
		DIALOG_FUNCTION(Sync_2d_3d,update_windows)(temp_dialog);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DIALOG_FUNCTION(Sync_2d_3d,set_depth).  %s",
			"Could not initialise dialog variables");
	}
	LEAVE;

	return (return_code);
} /* DIALOG_FUNCTION(Sync_2d_3d,set_depth) */

int DIALOG_FUNCTION(Sync_2d_3d,set_thickness)(DIALOG_GLOBAL_PARAM(Sync_2d_3d),
	double thickness)
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Sets the depth of the image.
==============================================================================*/
{
	int return_code;
	DIALOG_VARIABLES(Sync_2d_3d);

	return_code=0;
	ENTER(DIALOG_FUNCTION(Sync_2d_3d,set_thickness));
	if (DIALOG_INITIALISE_VARIABLES(Sync_2d_3d))
	{
		DIALOG_FUNCTION(Sync_2d_3d,merge_nodes)(temp_dialog);
		temp_data->thickness=thickness;
		sprintf(global_temp_string,"%f",temp_data->thickness);
		XmTextSetString(temp_dialog->thickness,
			global_temp_string);
		/* Synchronise the display */
		DIALOG_FUNCTION(Sync_2d_3d,update_windows)(temp_dialog);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DIALOG_FUNCTION(Sync_2d_3d,set_thickness).  %s",
			"Could not initialise dialog variables");
	}
	LEAVE;

	return (return_code);
} /* DIALOG_FUNCTION(Sync_2d_3d,set_thickness) */

int DIALOG_FUNCTION(Sync_2d_3d,set_axis_name)(DIALOG_GLOBAL_PARAM(Sync_2d_3d),
	char axis_name)
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Sets the depth of the image.
==============================================================================*/
{
	int return_code;
	DIALOG_VARIABLES(Sync_2d_3d);
	char correct_axis_name;

	return_code=0;
	ENTER(DIALOG_FUNCTION(Sync_2d_3d,set_axis_name));
	if (DIALOG_INITIALISE_VARIABLES(Sync_2d_3d))
	{
		switch (axis_name)
		{
			case 'x':
			case 'X':
			{
				correct_axis_name='x';
			} break;
			case 'y':
			case 'Y':
			{
				correct_axis_name='y';
			} break;
			case 'z':
			case 'Z':
			{
				correct_axis_name='z';
			} break;
			default:
			{
				display_message(WARNING_MESSAGE,
					"DIALOG_FUNCTION(Sync_2d_3d,set_axis_name).  %s",
					"Axis name not implemented");
				correct_axis_name='z';
			} break;
		}
		DIALOG_FUNCTION(Sync_2d_3d,merge_nodes)(temp_dialog);
		/* sanity checking? */
		temp_data->axis_name=correct_axis_name;
		sprintf(global_temp_string,"%c",temp_data->axis_name);
		/* this should be a macro */
		XmTextSetString(temp_dialog->axis_name,
			global_temp_string);
		/* Synchronise the display */
		DIALOG_FUNCTION(Sync_2d_3d,update_windows)(temp_dialog);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DIALOG_FUNCTION(Sync_2d_3d,set_axis_name).  %s",
			"Could not initialise dialog variables");
	}
	LEAVE;

	return (return_code);
} /* DIALOG_FUNCTION(Sync_2d_3d,set_axis_name) */

int bring_up_sync_2d_3d_dialog(Widget *sync_2d_3d_dialog_address,
	Widget parent,struct Execute_command *execute_command,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager)
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
If there is a sync_2d_3d dialog in existence, then bring it to the front,
else create a new one.
==============================================================================*/
{
	int return_code;
	DIALOG_INITIALISATION_STRUCT(Sync_2d_3d) initial_data;
	struct DIALOG_STRUCT(dialog_name) *return_struct;
	char name[]="full_mri_%c_%04.0f.rgb";

	return_code=0;
	ENTER(bring_up_sync_2d_3d_dialog);
	if (sync_2d_3d_dialog_address)
	{
		if (*sync_2d_3d_dialog_address)
		{
			XtPopup(*sync_2d_3d_dialog_address,XtGrabNone);
			return_code=1;
		}
		else
		{
			initial_data.image_base_name=name;
			initial_data.axis_name='z';
			initial_data.depth=100.0;
			initial_data.thickness=20.0;
			initial_data.source_node_group=
				FIRST_OBJECT_IN_MANAGER_THAT(GROUP(FE_node))(
					(MANAGER_CONDITIONAL_FUNCTION(GROUP(FE_node)) *)NULL,(void *)NULL,
					node_group_manager);
			initial_data.fe_field_manager=fe_field_manager;
			initial_data.node_manager=node_manager;
			initial_data.node_group_manager=node_group_manager;
			initial_data.execute_command=execute_command;
			if (return_struct=DIALOG_CREATE(dialog_name)(parent,
				&initial_data))
			{
				*sync_2d_3d_dialog_address=return_struct->dialog;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"bring_up_sync_2d_3d_dialog.  %s",
					"Error creating dialog");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"bring_up_sync_2d_3d_dialog.  %s",
			"Invalid argument");
	}
	LEAVE;

	return (return_code);
} /* bring_up_sync_2d_3d_dialog */
