/*******************************************************************************
FILE : graphical_element_editor_dialog.c

LAST MODIFIED : 17 May 2000

DESCRIPTION :
Routines for creating an element group editor dialog shell and standard buttons.
Form set aside for the actual element group editor.
==============================================================================*/
#include <stdio.h>
#include <Xm/ToggleB.h>
#include "general/debug.h"
#include "choose/choose_element_group.h"
#include "choose/choose_scene.h"
#include "graphics/element_group_settings.h"
#include "graphics/graphical_element.h"
#include "graphics/graphical_element_editor.h"
#include "graphics/graphical_element_editor_dialog.h"
#include "graphics/graphical_element_editor_dialog.uidh"
#include "graphics/graphics_object.h"
#include "graphics/material.h"
#include "graphics/spectrum.h"
#include "graphics/volume_texture.h"
#include "graphics/scene.h"
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int graphical_element_editor_dialog_hierarchy_open=0;
static MrmHierarchy graphical_element_editor_dialog_hierarchy;
#endif /* defined (MOTIF) */

struct Graphical_element_editor_dialog
/*******************************************************************************
LAST MODIFIED : 7 April 2000

DESCRIPTION :
Contains all the information carried by the graphical element editor dialog
widget.
==============================================================================*/
{
	/* if autoapply flag is set, any changes to the currently edited graphical
		 element will automatically be applied globally */
	int autoapply,graphical_element_changed;
	struct Callback_data update_callback;
	struct Computed_field_package *computed_field_package;
	struct GROUP(FE_element) *element_group;
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(Graphical_material) *graphical_material_manager;
	struct Graphical_material *default_material;
	struct MANAGER(Scene) *scene_manager;
	struct Scene *scene;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct Spectrum *default_spectrum;
	struct MANAGER(GROUP(FE_element)) *element_group_manager;
	struct User_interface *user_interface;
	struct MANAGER(VT_volume_texture) *volume_texture_manager;
	Widget egroup_form,egroup_widget,scene_form,scene_widget,visibility_button,
		autoapply_button,editor_form,editor_widget,ok_button,apply_button,
		revert_button,cancel_button;
	Widget *dialog_address,dialog,widget,dialog_parent;
}; /* Graphical_element_editor_dialog */

/*
Module functions
----------------
*/

#if defined (OLD_CODE)
static int graphical_element_editor_dialog_update(
	struct Graphical_element_editor_dialog *gelem_editor_dialog)
/*******************************************************************************
LAST MODIFIED : 1 August 1997

DESCRIPTION :
Tells CMGUI about the current values.
???RC Unused at present. Need to return element_group and scene - not easy.
???RC They should eventually be the SAME THING in our graphics hierarchy,
ie. the graphical region is a class derived from a scene.
==============================================================================*/
{
	int return_code;

	ENTER(graphical_element_editor_dialog_update);
	/* checking arguments */
	if (gelem_editor_dialog)
	{
		/* Now send an update to the client if requested */
		if ((gelem_editor_dialog->update_callback).procedure)
		{
			(gelem_editor_dialog->update_callback.procedure)(
				gelem_editor_dialog->widget,gelem_editor_dialog->update_callback.data,
				(void *)NULL);
/*				gelem_editor_dialog->gt_element_group);*/
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_dialog_update.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* graphical_element_editor_dialog_update */
#endif /* defined (OLD_CODE) */

static int graphical_element_editor_dialog_apply_settings(
	struct Graphical_element_editor_dialog *gelem_editor_dialog)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Copies the settings back to the original GT_element_group, rebuilds the
graphics and envokes a scene update.
==============================================================================*/
{
	int return_code;
	struct GT_element_group *gt_element_group,*edited_gt_element_group;

	ENTER(graphical_element_editor_dialog_apply_settings);
	if (gelem_editor_dialog)
	{
		if ((gt_element_group=Scene_get_graphical_element_group(
			gelem_editor_dialog->scene,gelem_editor_dialog->element_group)))
		{
			if (edited_gt_element_group=graphical_element_editor_get_gt_element_group(
				gelem_editor_dialog->editor_widget))
			{
				busy_cursor_on((Widget)NULL,gelem_editor_dialog->user_interface);
				GT_element_group_modify(gt_element_group,edited_gt_element_group);
				gelem_editor_dialog->graphical_element_changed=0;
				busy_cursor_off((Widget)NULL,gelem_editor_dialog->user_interface);
			}
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_dialog_apply_settings.  "
			"Missing graphical_element_editor_dialog");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* graphical_element_editor_dialog_apply_settings */

DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor_dialog, \
	Graphical_element_editor_dialog,egroup_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor_dialog, \
	Graphical_element_editor_dialog,scene_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor_dialog, \
	Graphical_element_editor_dialog,visibility_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor_dialog, \
	Graphical_element_editor_dialog,autoapply_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor_dialog, \
	Graphical_element_editor_dialog,editor_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor_dialog, \
	Graphical_element_editor_dialog,ok_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor_dialog, \
	Graphical_element_editor_dialog,apply_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor_dialog, \
	Graphical_element_editor_dialog,revert_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphical_element_editor_dialog, \
	Graphical_element_editor_dialog,cancel_button)

static int graphical_element_editor_dialog_read_defaults(
	struct Graphical_element_editor_dialog *gelem_editor_dialog)
/*******************************************************************************
LAST MODIFIED : 17 May 2000

DESCRIPTION :
Reads various resources from the Xdefaults file.
==============================================================================*/
{
#define XmNgraphicalElementEditorAutoApply "graphicalElementEditorAutoApply"
#define XmCGraphicalElementEditorAutoApply "GraphicalElementEditorAutoApply"
	int return_code;
	struct Graphical_element_editor_dialog_defaults
	{
		Boolean autoapply_button_set;
	} gelem_editor_dialog_defaults;
	static XtResource resources[]=
	{
		{
			XmNgraphicalElementEditorAutoApply,
			XmCGraphicalElementEditorAutoApply,
			XmRBoolean,
			sizeof(Boolean),
			XtOffsetOf(struct Graphical_element_editor_dialog_defaults,
				autoapply_button_set),
			XmRBoolean,
			"True"
		}
	};

	ENTER(graphical_element_editor_dialog_read_defaults);
	if (gelem_editor_dialog)
	{
		gelem_editor_dialog_defaults.autoapply_button_set=True;
		XtVaGetApplicationResources(
			gelem_editor_dialog->user_interface->application_shell,
			&gelem_editor_dialog_defaults,resources,XtNumber(resources),NULL);
		if (gelem_editor_dialog_defaults.autoapply_button_set)
		{
			gelem_editor_dialog->autoapply=1;
		}
		else
		{
			gelem_editor_dialog->autoapply=0;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_dialog_read_defaults.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return return_code;
} /* graphical_element_editor_dialog_read_defaults */

static void graphical_element_editor_dialog_destroy_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 16 July 1998

DESCRIPTION :
Callback for when the graphical_element_editor_dialog is closed. Tidies up all
details: dynamic allocations, etc.
==============================================================================*/
{
	struct Graphical_element_editor_dialog *gelem_editor_dialog;

	ENTER(graphical_element_editor_dialog_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* get the pointer to the data for the widget */
		XtVaGetValues(widget,XmNuserData,&gelem_editor_dialog,NULL);
		if (gelem_editor_dialog)
		{
			destroy_Shell_list_item_from_shell(&(gelem_editor_dialog->dialog),
				gelem_editor_dialog->user_interface);
			/* clear the graphical_element_editor_dialog for the client */
			*(gelem_editor_dialog->dialog_address)=(Widget)NULL;
			/* deallocate the memory for the user data */
			DEALLOCATE(gelem_editor_dialog);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"graphical_element_editor_dialog_destroy_CB.  "
				"Missing graphical_element_editor_dialog");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_dialog_destroy_CB.  Missing widget");
	}
	LEAVE;
} /* graphical_element_editor_dialog_destroy_CB */

static void graphical_element_editor_dialog_visibility_button_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Visibility toggle for entire graphical element group.
==============================================================================*/
{
	struct Graphical_element_editor_dialog *gelem_editor_dialog;
	enum GT_visibility_type visibility;

	ENTER(graphical_element_editor_dialog_visibility_button_CB);
	USE_PARAMETER(call_data);
	if (widget&&(gelem_editor_dialog=
		(struct Graphical_element_editor_dialog *)client_data))
	{
		if (XmToggleButtonGetState(widget))
		{
			visibility = g_VISIBLE;
		}
		else
		{
			visibility = g_INVISIBLE;
		}
		Scene_set_element_group_visibility(gelem_editor_dialog->scene,
			gelem_editor_dialog->element_group,visibility);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_dialog_visibility_button_CB.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_dialog_visibility_button_CB */

static void graphical_element_editor_dialog_autoapply_button_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 7 April 2000

DESCRIPTION :
Autoapply toggle for entire graphical element group.
==============================================================================*/
{
	struct Graphical_element_editor_dialog *gelem_editor_dialog;

	ENTER(graphical_element_editor_dialog_autoapply_button_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (gelem_editor_dialog=
		(struct Graphical_element_editor_dialog *)client_data)
	{
		if (XmToggleButtonGetState(widget))
		{
			gelem_editor_dialog->autoapply = 1;
			if (gelem_editor_dialog->graphical_element_changed)
			{
				graphical_element_editor_dialog_apply_settings(gelem_editor_dialog);
			}
		}
		else
		{
			gelem_editor_dialog->autoapply = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_dialog_autoapply_button_CB.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_dialog_autoapply_button_CB */

static void graphical_element_editor_dialog_update_egroup(
	Widget surface_settings_editor_widget,
	void *gelem_editor_dialog_void,void *element_group_void)
/*******************************************************************************
LAST MODIFIED : 7 April 2000

DESCRIPTION :
Called when element group is changed.
==============================================================================*/
{
	struct Graphical_element_editor_dialog *gelem_editor_dialog;

	ENTER(graphical_element_editor_dialog_update_egroup);
	USE_PARAMETER(surface_settings_editor_widget);
	if (gelem_editor_dialog=
		(struct Graphical_element_editor_dialog *)gelem_editor_dialog_void)
	{
		if (gelem_editor_dialog->graphical_element_changed)
		{
			graphical_element_editor_dialog_apply_settings(gelem_editor_dialog);
		}
		graphical_element_editor_dialog_set_element_group_and_scene(
			gelem_editor_dialog->dialog,
			(struct GROUP(FE_element) *)element_group_void,
			gelem_editor_dialog->scene);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_dialog_update_egroup.  Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_dialog_update_egroup */

static void graphical_element_editor_dialog_update_scene(
	Widget surface_settings_editor_widget,
	void *gelem_editor_dialog_void,void *scene_void)
/*******************************************************************************
LAST MODIFIED : 7 April 2000

DESCRIPTION :
Called when scene is changed.
==============================================================================*/
{
	struct Graphical_element_editor_dialog *gelem_editor_dialog;

	ENTER(graphical_element_editor_dialog_update_scene);
	USE_PARAMETER(surface_settings_editor_widget);
	if (gelem_editor_dialog=
		(struct Graphical_element_editor_dialog *)gelem_editor_dialog_void)
	{
		if (gelem_editor_dialog->graphical_element_changed)
		{
			graphical_element_editor_dialog_apply_settings(gelem_editor_dialog);
		}
		graphical_element_editor_dialog_set_element_group_and_scene(
			gelem_editor_dialog->dialog,
			gelem_editor_dialog->element_group,
			(struct Scene *)scene_void);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_dialog_update_scene.  Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_dialog_update_scene */

static void graphical_element_editor_dialog_ok_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 16 July 1997

DESCRIPTION :
==============================================================================*/
{
	struct Graphical_element_editor_dialog *gelem_editor_dialog;

	ENTER(graphical_element_editor_dialog_ok_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* get the pointer to the data for the widget */
		XtVaGetValues(widget,XmNuserData,&gelem_editor_dialog,NULL);
		if (gelem_editor_dialog)
		{
			if ((!(gelem_editor_dialog->element_group))||
				(!(gelem_editor_dialog->scene))||
				graphical_element_editor_dialog_apply_settings(gelem_editor_dialog))
			{
				/* close the dialog shell */
				XtDestroyWidget(gelem_editor_dialog->dialog);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"graphical_element_editor_dialog_ok_CB.  "
				"Missing graphical_element_editor_dialog");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_dialog_ok_CB.  Missing widget");
	}
	LEAVE;
} /* graphical_element_editor_dialog_ok_CB */

static void graphical_element_editor_dialog_apply_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 16 July 1997

DESCRIPTION :
==============================================================================*/
{
	struct Graphical_element_editor_dialog *gelem_editor_dialog;

	ENTER(graphical_element_editor_dialog_apply_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* get the pointer to the data for the widget */
		XtVaGetValues(widget,XmNuserData,&gelem_editor_dialog,NULL);
		if (gelem_editor_dialog)
		{
			graphical_element_editor_dialog_apply_settings(gelem_editor_dialog);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"graphical_element_editor_dialog_apply_CB.  "
				"Missing graphical_element_editor_dialog");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_dialog_apply_CB.  Missing widget");
	}
	LEAVE;
} /* graphical_element_editor_dialog_apply_CB */

static void graphical_element_editor_dialog_revert_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 11 July 1997

DESCRIPTION :
Finds the gt_element_group for the current element_group and scene in the
gelem_editor_dialog and passes this to the graphical_element_editor.
==============================================================================*/
{
	struct Graphical_element_editor_dialog *gelem_editor_dialog;
	struct GT_element_group *gt_element_group;

	ENTER(graphical_element_editor_dialog_revert_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* get the pointer to the data for the widget */
		XtVaGetValues(widget,XmNuserData,&gelem_editor_dialog,NULL);
		if (gelem_editor_dialog)
		{
			if (gt_element_group=Scene_get_graphical_element_group(
				gelem_editor_dialog->scene,gelem_editor_dialog->element_group))
			{
				graphical_element_editor_set_gt_element_group(
					gelem_editor_dialog->editor_widget,gt_element_group);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"graphical_element_editor_dialog_revert_CB.  "
					"Could not find gt_element_group to revert to");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"graphical_element_editor_dialog_revert_CB.  "
				"Missing graphical_element_editor_dialog");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_dialog_revert_CB.  Missing widget");
	}
	LEAVE;
} /* graphical_element_editor_dialog_revert_CB */

static void graphical_element_editor_dialog_cancel_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 8 July 1997

DESCRIPTION :
==============================================================================*/
{
	struct Graphical_element_editor_dialog *gelem_editor_dialog;

	ENTER(graphical_element_editor_dialog_cancel_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* get the pointer to the data for the widget */
		XtVaGetValues(widget,XmNuserData,&gelem_editor_dialog,NULL);
		if (gelem_editor_dialog)
		{
			/* close the dialog shell */
			XtDestroyWidget(gelem_editor_dialog->dialog);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"graphical_element_editor_dialog_cancel_CB.  "
				"Missing graphical_element_editor_dialog");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_dialog_cancel_CB.  Missing widget");
	}
	LEAVE;
} /* graphical_element_editor_dialog_cancel_CB */

static void graphical_element_editor_dialog_update_graphical_element(
	Widget widget,void *gelem_editor_dialog_void,void *gt_element_group_void)
/*******************************************************************************
LAST MODIFIED : 7 April 2000

DESCRIPTION :
Callback for when changes are made in the graphical element editor. If autoapply
is on, changes are applied globally, otherwise nothing happens.
==============================================================================*/
{
	struct Graphical_element_editor_dialog *gelem_editor_dialog;

	ENTER(graphical_element_editor_dialog_update_graphical_element);
	USE_PARAMETER(widget);
	USE_PARAMETER(gt_element_group_void);
	if (gelem_editor_dialog=
		(struct Graphical_element_editor_dialog *)gelem_editor_dialog_void)
	{
		gelem_editor_dialog->graphical_element_changed=1;
		if (gelem_editor_dialog->autoapply)
		{
			graphical_element_editor_dialog_apply_settings(gelem_editor_dialog);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_dialog_update_graphical_element.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* graphical_element_editor_dialog_update_graphical_element */

static Widget create_graphical_element_editor_dialog(
	Widget *graphical_element_editor_dialog_address,Widget parent,
	struct Computed_field_package *computed_field_package,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct GROUP(FE_element) *element_group,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_material,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Scene) *scene_manager,
	struct Scene *scene,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *default_spectrum,
	struct MANAGER(VT_volume_texture) *volume_texture_manager,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 17 May 2000

DESCRIPTION :
==============================================================================*/
{
	int init_widgets;
	MrmType graphical_element_editor_dialog_dialog_class;
	struct Callback_data callback;
	struct Graphical_element_editor_dialog *gelem_editor_dialog;
	static MrmRegisterArg callback_list[]=
	{
		{"gelem_ed_d_id_egroup_form",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor_dialog,egroup_form)},
		{"gelem_ed_d_id_scene_form",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor_dialog,scene_form)},
		{"gelem_ed_d_id_visibility_btn",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor_dialog,visibility_button)},
		{"gelem_ed_d_id_autoapply_btn",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor_dialog,autoapply_button)},
		{"gelem_ed_d_id_editor_form",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor_dialog,editor_form)},
		{"gelem_ed_d_id_ok_btn",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor_dialog,ok_button)},
		{"gelem_ed_d_id_apply_btn",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor_dialog,apply_button)},
		{"gelem_ed_d_id_revert_btn",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor_dialog,revert_button)},
		{"gelem_ed_d_id_cancel_btn",(XtPointer)
			DIALOG_IDENTIFY(graphical_element_editor_dialog,cancel_button)},
		{"gelem_ed_d_destroy_CB",(XtPointer)
			graphical_element_editor_dialog_destroy_CB},
		{"gelem_ed_d_visibility_btn_CB",(XtPointer)
			graphical_element_editor_dialog_visibility_button_CB},
		{"gelem_ed_d_autoapply_btn_CB",(XtPointer)
			graphical_element_editor_dialog_autoapply_button_CB},
		{"gelem_ed_d_ok_CB",(XtPointer)
			graphical_element_editor_dialog_ok_CB},
		{"gelem_ed_d_apply_CB",(XtPointer)
			graphical_element_editor_dialog_apply_CB},
		{"gelem_ed_d_revert_CB",(XtPointer)
			graphical_element_editor_dialog_revert_CB},
		{"gelem_ed_d_cancel_CB",(XtPointer)
			graphical_element_editor_dialog_cancel_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"gelem_ed_d_structure",(XtPointer)NULL}
	};
	Widget return_widget;

	ENTER(create_graphical_element_editor_dialog);
	return_widget=(Widget)NULL;
	/* check arguments */
	if (graphical_element_editor_dialog_address&&parent&&element_group_manager&&
		scene_manager&&scene&&user_interface)
	{
		if (MrmOpenHierarchy_base64_string(graphical_element_editor_dialog_uidh,
			&graphical_element_editor_dialog_hierarchy,
			&graphical_element_editor_dialog_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(gelem_editor_dialog,
				struct Graphical_element_editor_dialog,1))
			{
				/* initialise the structure */
				gelem_editor_dialog->autoapply=1;
				gelem_editor_dialog->graphical_element_changed=0;
				gelem_editor_dialog->dialog_parent=parent;
				gelem_editor_dialog->dialog_address=
					graphical_element_editor_dialog_address;
				gelem_editor_dialog->computed_field_package=computed_field_package;
				gelem_editor_dialog->element_group=(struct GROUP(FE_element) *)NULL;
				gelem_editor_dialog->element_group_manager=
					element_group_manager;
				gelem_editor_dialog->fe_field_manager=fe_field_manager;
				gelem_editor_dialog->graphical_material_manager=
					graphical_material_manager;
				gelem_editor_dialog->default_material=default_material;
				gelem_editor_dialog->scene_manager=scene_manager;
				gelem_editor_dialog->scene=scene;
				gelem_editor_dialog->spectrum_manager=spectrum_manager;
				gelem_editor_dialog->default_spectrum=default_spectrum;
				gelem_editor_dialog->user_interface=user_interface;
				gelem_editor_dialog->widget=(Widget)NULL;
				gelem_editor_dialog->dialog=(Widget)NULL;
				gelem_editor_dialog->egroup_form=(Widget)NULL;
				gelem_editor_dialog->egroup_widget=(Widget)NULL;
				gelem_editor_dialog->scene_form=(Widget)NULL;
				gelem_editor_dialog->scene_widget=(Widget)NULL;
				gelem_editor_dialog->visibility_button=(Widget)NULL;
				gelem_editor_dialog->autoapply_button=(Widget)NULL;
				gelem_editor_dialog->editor_form=(Widget)NULL;
				gelem_editor_dialog->editor_widget=(Widget)NULL;
				gelem_editor_dialog->ok_button=(Widget)NULL;
				gelem_editor_dialog->apply_button=(Widget)NULL;
				gelem_editor_dialog->revert_button=(Widget)NULL;
				gelem_editor_dialog->cancel_button=(Widget)NULL;
				gelem_editor_dialog->update_callback.procedure=
					(Callback_procedure *)NULL;
				gelem_editor_dialog->update_callback.data=NULL;
				/* make the dialog shell */
				if (gelem_editor_dialog->dialog=XtVaCreatePopupShell(
					"Graphical Element Editor",topLevelShellWidgetClass,parent,
					XmNallowShellResize,FALSE/*TRUE*/,NULL))
				{
					/* register the callbacks */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						graphical_element_editor_dialog_hierarchy,callback_list,
						XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value=(XtPointer)gelem_editor_dialog;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
							graphical_element_editor_dialog_hierarchy,identifier_list,
							XtNumber(identifier_list)))
						{
							/* fetch dialog widget */
							if (MrmSUCCESS==MrmFetchWidget(
								graphical_element_editor_dialog_hierarchy,
								"gelem_ed_d_widget",gelem_editor_dialog->dialog,
								&(gelem_editor_dialog->widget),
								&graphical_element_editor_dialog_dialog_class))
							{
								XtManageChild(gelem_editor_dialog->widget);
								init_widgets=1;
								/* create NULL subwidgets */
								if (!(gelem_editor_dialog->egroup_widget=
									CREATE_CHOOSE_OBJECT_WIDGET(GROUP(FE_element))(
									gelem_editor_dialog->egroup_form,
									(struct GROUP(FE_element) *)NULL,
									gelem_editor_dialog->element_group_manager,
									(MANAGER_CONDITIONAL_FUNCTION(GROUP(FE_element)) *)NULL,
									(void *)NULL)))
								{
									init_widgets=0;
								}
								if (!(gelem_editor_dialog->scene_widget=
									CREATE_CHOOSE_OBJECT_WIDGET(Scene)(
									gelem_editor_dialog->scene_form,
									(struct Scene *)NULL,
									gelem_editor_dialog->scene_manager,
									(MANAGER_CONDITIONAL_FUNCTION(Scene) *)NULL,(void *)NULL)))
								{
									init_widgets=0;
								}
								if (!create_graphical_element_editor_widget(
									&(gelem_editor_dialog->editor_widget),
									gelem_editor_dialog->editor_form,
									(struct GT_element_group *)NULL,computed_field_package,
									element_manager,fe_field_manager,
									graphical_material_manager,default_material,
									glyph_list,spectrum_manager,default_spectrum,
									volume_texture_manager,user_interface))
								{
									init_widgets=0;
								}
								if (init_widgets)
								{
									/* read in some defaults, incl. autoapply flag */
									graphical_element_editor_dialog_read_defaults(
										gelem_editor_dialog);
									/* make autoapply button match the flag */
									if (gelem_editor_dialog->autoapply)
									{
										XmToggleButtonSetState(
											gelem_editor_dialog->autoapply_button,True,False);
									}
									else
									{
										XmToggleButtonSetState(
											gelem_editor_dialog->autoapply_button,False,False);
									}
									graphical_element_editor_dialog_set_element_group_and_scene(
										gelem_editor_dialog->dialog,element_group,scene);
									/* turn on subwidget callbacks */
									callback.procedure=
										graphical_element_editor_dialog_update_egroup;
									callback.data=(void *)gelem_editor_dialog;
									CHOOSE_OBJECT_SET_CALLBACK(GROUP(FE_element))(
										gelem_editor_dialog->egroup_widget,&callback);
									callback.procedure=
										graphical_element_editor_dialog_update_scene;
									CHOOSE_OBJECT_SET_CALLBACK(Scene)(
										gelem_editor_dialog->scene_widget,&callback);
									XtRealizeWidget(gelem_editor_dialog->dialog);
									XtPopup(gelem_editor_dialog->dialog,XtGrabNone);
									create_Shell_list_item(&(gelem_editor_dialog->dialog),
										gelem_editor_dialog->user_interface);
									return_widget=gelem_editor_dialog->dialog;
									/* get callbacks from graphical element editor for
										 autoapply functionality */
									callback.procedure=
										graphical_element_editor_dialog_update_graphical_element;
									callback.data=(void *)gelem_editor_dialog;
									graphical_element_editor_set_callback(
										gelem_editor_dialog->editor_widget,&callback);
								}
								else
								{
									XtDestroyWidget(gelem_editor_dialog->dialog);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"create_graphical_element_editor_dialog.  "
									"Could not fetch graphical_element_editor_dialog");
								DEALLOCATE(gelem_editor_dialog);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_graphical_element_editor_dialog.  "
								"Could not register identifiers");
							DEALLOCATE(gelem_editor_dialog);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_graphical_element_editor_dialog.  "
							"Could not register callbacks");
						DEALLOCATE(gelem_editor_dialog);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_graphical_element_editor_dialog.  "
						"Could not create popup shell");
					DEALLOCATE(gelem_editor_dialog);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_graphical_element_editor_dialog.  "
					"Could not allocate graphical_element_editor_dialog");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_graphical_element_editor_dialog.  Could not open hierarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_graphical_element_editor_dialog.  Invalid argument(s)");
	}
	if (graphical_element_editor_dialog_address&&return_widget)
	{
		*graphical_element_editor_dialog_address=return_widget;
	}
	LEAVE;

	return (return_widget);
} /* create_graphical_element_editor_dialog */

/*
Global functions
----------------
*/

int graphical_element_editor_dialog_get_element_group_and_scene(
	Widget graphical_element_editor_dialog,
	struct GROUP(FE_element) **element_group,struct Scene **scene)
/*******************************************************************************
LAST MODIFIED : 31 August 1999

DESCRIPTION :
Returns which element_group and scene are looked at with the
graphical_element_editor_dialog widget.
==============================================================================*/
{
	int num_children,return_code;
	struct Graphical_element_editor_dialog *gelem_editor_dialog;
	Widget *child_list;

	ENTER(graphical_element_editor_dialog_get_element_group_and_scene);
	return_code=0;
	if (graphical_element_editor_dialog&&element_group&&scene)
	{
		/* get the pointer to the data for the [single] child widget */
		XtVaGetValues(graphical_element_editor_dialog,XmNchildren,&child_list,
			XmNnumChildren,&num_children,NULL);
		if (num_children==1)
		{
			XtVaGetValues(child_list[0],XmNuserData,&gelem_editor_dialog,NULL);
			if (gelem_editor_dialog&&gelem_editor_dialog->element_group_manager)
			{
				*element_group=CHOOSE_OBJECT_GET_OBJECT(GROUP(FE_element))(
					gelem_editor_dialog->egroup_widget);
				*scene=CHOOSE_OBJECT_GET_OBJECT(Scene)(
					gelem_editor_dialog->scene_widget);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"graphical_element_editor_dialog_get_element_group_and_scene.  "
					"Missing widget data");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"graphical_element_editor_dialog_get_element_group_and_scene.  "
				"Invalid dialog");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_dialog_get_element_group_and_scene.  "
			"Missing dialog");
	}
	LEAVE;

	return (return_code);
} /* graphical_element_editor_dialog_get_element_group_and_scene */

int graphical_element_editor_dialog_set_element_group_and_scene(
	Widget graphical_element_editor_dialog,
	struct GROUP(FE_element) *element_group,struct Scene *scene)
/*******************************************************************************
LAST MODIFIED : 21 March 2000

DESCRIPTION :
Sets which element_group and scene are looked at with the
graphical_element_editor_dialog widget.
==============================================================================*/
{
	int num_children,return_code;
	struct Graphical_element_editor_dialog *gelem_editor_dialog;
	struct GT_element_group *gt_element_group;
	Widget *child_list;

	ENTER(graphical_element_editor_dialog_set_element_group_and_scene);
	return_code=0;
	if (graphical_element_editor_dialog)
	{
		/* get the pointer to the data for the [single] child widget */
		XtVaGetValues(graphical_element_editor_dialog,XmNchildren,&child_list,
			XmNnumChildren,&num_children,NULL);
		if (num_children==1)
		{
			XtVaGetValues(child_list[0],XmNuserData,&gelem_editor_dialog,NULL);
			if (gelem_editor_dialog&&gelem_editor_dialog->element_group_manager)
			{
				/*???RC later: check values are different! */
				/* if element_group NULL or unmanaged, get first one in manager */
				if ((!element_group) || (!(IS_MANAGED(GROUP(FE_element))(element_group,
					gelem_editor_dialog->element_group_manager))))
				{
					element_group=FIRST_OBJECT_IN_MANAGER_THAT(GROUP(FE_element))(
						(MANAGER_CONDITIONAL_FUNCTION(GROUP(FE_element)) *)NULL,
						(void *)NULL,gelem_editor_dialog->element_group_manager);
				}
				if ((!scene) || (!(IS_MANAGED(Scene)(scene,
					gelem_editor_dialog->scene_manager))))
				{
					scene=FIRST_OBJECT_IN_MANAGER_THAT(Scene)(
						(MANAGER_CONDITIONAL_FUNCTION(Scene) *)NULL,
						(void *)NULL,gelem_editor_dialog->scene_manager);
				}
				/* set subwidget values */
				gelem_editor_dialog->element_group=element_group;
				CHOOSE_OBJECT_SET_OBJECT(GROUP(FE_element))(
					gelem_editor_dialog->egroup_widget,element_group);
				gelem_editor_dialog->scene=scene;
				CHOOSE_OBJECT_SET_OBJECT(Scene)(
					gelem_editor_dialog->scene_widget,scene);
				if (element_group&&scene)
				{
					gt_element_group=Scene_get_graphical_element_group(scene,
						element_group);
				}
				else
				{
					gt_element_group=(struct GT_element_group *)NULL;
				}
				if (gt_element_group)
				{
					XmToggleButtonSetState(gelem_editor_dialog->visibility_button,
						g_VISIBLE==Scene_get_element_group_visibility(scene,element_group),
						False);
				}
				XtSetSensitive(gelem_editor_dialog->visibility_button,
					(struct GT_element_group *)NULL != gt_element_group);
				XtSetSensitive(gelem_editor_dialog->autoapply_button,
					(struct GT_element_group *)NULL != gt_element_group);
				XtSetSensitive(gelem_editor_dialog->apply_button,
					(struct GT_element_group *)NULL != gt_element_group);
				XtSetSensitive(gelem_editor_dialog->revert_button,
					(struct GT_element_group *)NULL != gt_element_group);
				graphical_element_editor_set_gt_element_group(
					gelem_editor_dialog->editor_widget,gt_element_group);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"graphical_element_editor_dialog_set_element_group_and_scene.  "
					"Missing widget data");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"graphical_element_editor_dialog_set_element_group_and_scene.  "
				"Invalid dialog");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphical_element_editor_dialog_set_element_group_and_scene.  "
			"Missing dialog");
	}
	LEAVE;

	return (return_code);
} /* graphical_element_editor_dialog_set_element_group_and_scene */

int bring_up_graphical_element_editor_dialog(
	Widget *graphical_element_editor_dialog_address,Widget parent,
	struct Computed_field_package *computed_field_package,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct GROUP(FE_element) *element_group,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_material,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Scene) *scene_manager,
	struct Scene *scene,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *default_spectrum,
	struct MANAGER(VT_volume_texture) *volume_texture_manager,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 16 February 1999

DESCRIPTION :
If there is a graphical_element_editor_dialog in existence, then bring it to
the front with the new element_group and scene, ptherwise create a new one.
==============================================================================*/
{
	int return_code;

	ENTER(bring_up_graphical_element_editor_dialog);
	if (graphical_element_editor_dialog_address)
	{
		if (*graphical_element_editor_dialog_address)
		{
			graphical_element_editor_dialog_set_element_group_and_scene(
				*graphical_element_editor_dialog_address,element_group,scene);
			XtPopup(*graphical_element_editor_dialog_address,XtGrabNone);
			return_code=1;
		}
		else
		{
			if (create_graphical_element_editor_dialog(
				graphical_element_editor_dialog_address,parent,computed_field_package,
				element_manager,element_group_manager,element_group,fe_field_manager,
				graphical_material_manager,default_material,glyph_list,
				scene_manager,scene,spectrum_manager,default_spectrum,
				volume_texture_manager,user_interface))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"bring_up_graphical_element_editor_dialog.  Error creating dialog");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"bring_up_graphical_element_editor_dialog.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* bring_up_graphical_element_editor_dialog */
