/*******************************************************************************
FILE : spectrum_editor_dialog.c

LAST MODIFIED : 10 September 1998

DESCRIPTION :
This module creates a spectrum_editor_dialog.
???SAB.  Basically pillaged from material/material_editor_dialog.c
==============================================================================*/
#include <stdio.h>
#include "choose/choose_scene.h"
#include "general/debug.h"
#include "general/manager.h"
#include "general/object.h"
#include "graphics/light.h"
#include "graphics/material.h"
#include "graphics/scene.h"
#include "graphics/spectrum.h"
#include "graphics/spectrum_editor.h"
#include "graphics/spectrum_editor_dialog.h"
#include "graphics/spectrum_editor_dialog.uidh"
#include "select/select_spectrum.h"
#include "user_interface/message.h"

/*
Module Constants
----------------
*/
/* UIL Identifiers */
#define spectrum_editor_dialog_select_form_ID 1
#define spectrum_editor_dialog_editor_form_ID 2
#define spectrum_editor_dialog_ok_ID          3
#define spectrum_editor_dialog_apply_ID       4
#define spectrum_editor_dialog_revert_ID      5
#define spectrum_editor_dialog_cancel_ID      6
#define spectrum_editor_dialog_autorange_scene_ID      7

/*
Global Types
------------
*/
struct Spectrum_editor_dialog
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Contains all the information carried by the spectrum_editor_dialog widget.
Note that we just hold a pointer to the spectrum_editor_dialog, and must access
and deaccess it.
==============================================================================*/
{
	struct Callback_data update_callback;
	struct Spectrum *current_value;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct Scene *autorange_scene;
	Widget editor_form,editor_widget,select_form,select_widget;
	Widget apply_button,cancel_button,ok_button,revert_button;
	Widget *dialog_address,dialog,widget,dialog_parent,
		autorange_scene_form, autorange_scene_widget;
}; /* spectrum_editor_dialog */

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int spectrum_editor_dialog_hierarchy_open=0;
static MrmHierarchy spectrum_editor_dialog_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
static void spectrum_editor_dialog_update(
	struct Spectrum_editor_dialog *spectrum_editor_dialog)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Tells CMGUI about the current values. Returns a pointer to the spectrum.
==============================================================================*/
{
	ENTER(spectrum_editor_dialog_update);
	if (spectrum_editor_dialog)
	{
		if (spectrum_editor_dialog->update_callback.procedure)
		{
			/* now call the procedure with the user data and the position data */
			(spectrum_editor_dialog->update_callback.procedure)
				(spectrum_editor_dialog->widget,
				spectrum_editor_dialog->update_callback.data,
				spectrum_editor_dialog->current_value);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_dialog_update.  Invalid argument(s)");
	}
	LEAVE;
} /* spectrum_editor_dialog_update */

static void spectrum_editor_dialog_update_selection(Widget select_widget,
	void *user_data,void *spec)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Finds which spectrum is selected, and informs the editor widget.
???DB.  Not checking arguments ?
==============================================================================*/
{
	struct Spectrum_editor_dialog *spectrum_editor_dialog;
	struct Spectrum *spectrum;

	ENTER(spectrum_editor_dialog_update_selection);
	if ((spectrum_editor_dialog=(struct Spectrum_editor_dialog *)
		user_data)&&(spectrum=(struct Spectrum *)spec))
	{
		spectrum_editor_dialog->current_value=spectrum;
		spectrum_editor_set_Spectrum(spectrum_editor_dialog->editor_widget,
			spectrum_editor_dialog->current_value);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_dialog_update_selection.  Invalid argument(s)");
	}
	LEAVE;
} /* spectrum_editor_dialog_update_selection */

static void spectrum_editor_dialog_identify_button(Widget w,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Finds the id of the buttons on the spectrum_editor_dialog widget.
==============================================================================*/
{
	struct Spectrum_editor_dialog *spectrum_editor_dialog;

	ENTER(spectrum_editor_dialog_identify_button);
	/* find out which spectrum_editor_dialog widget we are in */
	XtVaGetValues(w,XmNuserData,&spectrum_editor_dialog,NULL);
	switch (button_num)
	{
		case spectrum_editor_dialog_editor_form_ID:
		{
			spectrum_editor_dialog->editor_form=w;
		} break;
		case spectrum_editor_dialog_select_form_ID:
		{
			spectrum_editor_dialog->select_form=w;
		} break;
		case spectrum_editor_dialog_ok_ID:
		{
			spectrum_editor_dialog->ok_button=w;
		} break;
		case spectrum_editor_dialog_apply_ID:
		{
			spectrum_editor_dialog->apply_button=w;
		} break;
		case spectrum_editor_dialog_revert_ID:
		{
			spectrum_editor_dialog->revert_button=w;
		} break;
		case spectrum_editor_dialog_cancel_ID:
		{
			spectrum_editor_dialog->cancel_button=w;
		} break;
		case spectrum_editor_dialog_autorange_scene_ID:
		{
			spectrum_editor_dialog->autorange_scene_form=w;
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"spectrum_editor_dialog_identify_button.  Invalid button number");
		} break;
	}
	LEAVE;
} /* spectrum_editor_dialog_identify_button */

static void spectrum_editor_dialog_control_CB(Widget w,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Finds the id of the buttons on the spectrum_editor_dialog widget.
==============================================================================*/
{
	struct Spectrum *edit_spectrum;
	struct Spectrum_editor_dialog *spectrum_editor_dialog;

	ENTER(spectrum_editor_dialog_control_CB);
	/* find out which spectrum_editor_dialog widget we are in */
	XtVaGetValues(w,XmNuserData,&spectrum_editor_dialog,NULL);
	if ((button_num==spectrum_editor_dialog_cancel_ID)||
		(button_num==spectrum_editor_dialog_revert_ID))
	{
		spectrum_editor_set_Spectrum(spectrum_editor_dialog->editor_widget,
			spectrum_editor_dialog->current_value);
	}
	/* if it is Apply or OK, do a global update */
	if ((button_num==spectrum_editor_dialog_apply_ID)||
		(button_num==spectrum_editor_dialog_ok_ID))
	{
		if (spectrum_editor_dialog->current_value&&
			(edit_spectrum=spectrum_editor_get_Spectrum(
				spectrum_editor_dialog->editor_widget)))
		{
			if ( MANAGER_MODIFY_NOT_IDENTIFIER(Spectrum,name)(
				spectrum_editor_dialog->current_value,edit_spectrum,
				spectrum_editor_dialog->spectrum_manager))
			{
				spectrum_editor_refresh( spectrum_editor_dialog->editor_widget );
			}
			spectrum_editor_dialog_update(spectrum_editor_dialog);
		}
	}
	/* if it is an OK or a cancel, we have to kill the dialog */
	if ((button_num==spectrum_editor_dialog_ok_ID)||
		(button_num==spectrum_editor_dialog_cancel_ID))
	{
		XtDestroyWidget(spectrum_editor_dialog->dialog);
	}
	LEAVE;
} /* spectrum_editor_dialog_control_CB */

static void spectrum_editor_dialog_autorange_button_CB(Widget widget, int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 22 July 1998

DESCRIPTION :
Called when the autorange push button is activated.
==============================================================================*/
{
	float minimum, maximum;
	int range_set;
	struct Spectrum_editor_dialog *spectrum_editor_dialog;

	ENTER(spectrum_editor_dialog_autorange_button_CB);
	/* check arguments */
	if (widget)
	{
		/* Get the pointer to the data for the widget */
		XtVaGetValues(widget,XmNuserData,&spectrum_editor_dialog,NULL);
		if (spectrum_editor_dialog)
		{
			range_set = 0;
			Scene_get_data_range_for_spectrum(spectrum_editor_dialog->autorange_scene,
				spectrum_editor_dialog->current_value,
				&minimum, &maximum, &range_set);
			if ( range_set )
			{
				Spectrum_set_minimum_and_maximum(
					spectrum_editor_get_Spectrum(spectrum_editor_dialog->editor_widget),
					minimum, maximum );
				spectrum_editor_update_changes(spectrum_editor_dialog->editor_widget);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_dialog_autorange_button_CB.  "
				"Missing spectrum_editor_dialog struct");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_dialog_autorange_button_CB.  Missing widget");
	}
	LEAVE;
} /* spectrum_editor_dialog_autorange_button_CB */

static void spectrum_editor_dialog_set_autorange_scene(
	Widget widget, void *spectrum_editor_dialog_void, void *scene_void)
/*******************************************************************************
LAST MODIFIED : 22 July 1998

DESCRIPTION :
Called when scene is changed.
==============================================================================*/
{
	struct Spectrum_editor_dialog *spectrum_editor_dialog;

	ENTER(spectrum_editor_dialog_set_autorange_scene);
	if (spectrum_editor_dialog = (struct Spectrum_editor_dialog *)spectrum_editor_dialog_void)
	{
		spectrum_editor_dialog->autorange_scene = (struct Scene *)scene_void;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_dialog_set_autorange_scene.  Invalid argument(s)");
	}
	LEAVE;
} /* spectrum_editor_dialog_set_autorange_scene */

static void spectrum_editor_dialog_destroy_CB(Widget w,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Callback for the spectrum_editor_dialog dialog - tidies up all details - mem etc
==============================================================================*/
{
	struct Spectrum_editor_dialog *spectrum_editor_dialog;

	ENTER(spectrum_editor_dialog_destroy_CB);
	/* Get the pointer to the data for the spectrum_editor_dialog widget */
	XtVaGetValues(w,XmNuserData,&spectrum_editor_dialog,NULL);
	/* deaccess the spectrum_editor_dialog */
	*(spectrum_editor_dialog->dialog_address)=(Widget)NULL;
	/* deallocate the memory for the user data */
	DEALLOCATE(spectrum_editor_dialog);
	LEAVE;
} /* spectrum_editor_dialog_destroy_CB */

static Widget create_spectrum_editor_dialog(
	Widget *spectrum_editor_dialog_widget,Widget parent,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *init_data, struct User_interface *user_interface,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(Light) *light_manager,
	struct MANAGER(Texture) *texture_manager,
	struct MANAGER(Scene) *scene_manager)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Creates a dialog widget that allows the user to edit the properties of any of
the spectrums contained in the global list.
==============================================================================*/
{
	int i,init_widgets;
	struct Callback_data callback;
	MrmType spectrum_editor_dialog_dialog_class;
	struct Spectrum_editor_dialog *spectrum_editor_dialog=NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"spec_editor_d_identify_button",
		(XtPointer)spectrum_editor_dialog_identify_button},
		{"spec_editor_d_destroy_CB",
			(XtPointer)spectrum_editor_dialog_destroy_CB},
		{"spec_editor_d_control_CB",
			(XtPointer)spectrum_editor_dialog_control_CB},
		{"spec_ed_d_autorange_btn_CB",
			(XtPointer)spectrum_editor_dialog_autorange_button_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"spec_editor_d_structure",(XtPointer)NULL},
		{"spec_editor_d_editor_form_ID",
		(XtPointer)spectrum_editor_dialog_editor_form_ID},
		{"spec_editor_d_select_form_ID",
		(XtPointer)spectrum_editor_dialog_select_form_ID},
		{"spec_editor_d_ok_ID",(XtPointer)spectrum_editor_dialog_ok_ID},
		{"spec_editor_d_apply_ID",(XtPointer)spectrum_editor_dialog_apply_ID},
		{"spec_editor_d_revert_ID",(XtPointer)spectrum_editor_dialog_revert_ID},
		{"spec_editor_d_cancel_ID",(XtPointer)spectrum_editor_dialog_cancel_ID},
		{"spec_ed_d_autorange_scene_ID",(XtPointer)spectrum_editor_dialog_autorange_scene_ID}
	};
	Widget return_widget;

	ENTER(create_spectrum_editor_dialog);
	return_widget=(Widget)NULL;
	/* check arguments */
	if (spectrum_manager&&user_interface)
	{
		if (MrmOpenHierarchy_base64_string(spectrum_editor_dialog_uidh,
			&spectrum_editor_dialog_hierarchy,&spectrum_editor_dialog_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(spectrum_editor_dialog,
				struct Spectrum_editor_dialog,1))
			{
				/* initialise the structure */
				spectrum_editor_dialog->dialog_parent=parent;
				spectrum_editor_dialog->dialog_address=
					spectrum_editor_dialog_widget;
				spectrum_editor_dialog->spectrum_manager=
					spectrum_manager;
				/* current_value set in spectrum_editor_dialog_set_spectrum */
				spectrum_editor_dialog->current_value=
					(struct Spectrum *)NULL;
				spectrum_editor_dialog->widget=(Widget)NULL;
				spectrum_editor_dialog->dialog=(Widget)NULL;
				spectrum_editor_dialog->select_form=(Widget)NULL;
				spectrum_editor_dialog->select_widget=(Widget)NULL;
				spectrum_editor_dialog->editor_form=(Widget)NULL;
				spectrum_editor_dialog->editor_widget=(Widget)NULL;
				spectrum_editor_dialog->ok_button=(Widget)NULL;
				spectrum_editor_dialog->apply_button=(Widget)NULL;
				spectrum_editor_dialog->revert_button=(Widget)NULL;
				spectrum_editor_dialog->cancel_button=(Widget)NULL;
				spectrum_editor_dialog->autorange_scene_form=(Widget)NULL;
				spectrum_editor_dialog->autorange_scene_widget=(Widget)NULL;
				spectrum_editor_dialog->autorange_scene = (struct Scene *)NULL;
				spectrum_editor_dialog->update_callback.procedure=
					(Callback_procedure *)NULL;
				spectrum_editor_dialog->update_callback.data=NULL;
				/* make the dialog shell */
				if (spectrum_editor_dialog->dialog=XtVaCreatePopupShell(
					"Spectrum Editor",topLevelShellWidgetClass,parent,XmNallowShellResize,
					TRUE,NULL))
				{
					/* register the callbacks */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						spectrum_editor_dialog_hierarchy,callback_list,
						XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value=(XtPointer)spectrum_editor_dialog;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
							spectrum_editor_dialog_hierarchy,identifier_list,
							XtNumber(identifier_list)))
						{
							/* fetch position window widget */
							if (MrmSUCCESS==MrmFetchWidget(spectrum_editor_dialog_hierarchy,
								"spec_editor_dialog_widget",spectrum_editor_dialog->dialog,
								&(spectrum_editor_dialog->widget),
								&spectrum_editor_dialog_dialog_class))
							{
								XtManageChild(spectrum_editor_dialog->widget);
								/* set the mode toggle to the correct position */
								init_widgets=1;
								if (!CREATE_SELECT_WIDGET(Spectrum)(
									&spectrum_editor_dialog->select_widget,
									spectrum_editor_dialog->select_form,
									SELECT_LIST,
									spectrum_editor_dialog->current_value,
									spectrum_manager))
								{
									display_message(ERROR_MESSAGE,
						"create_spectrum_editor_dialog.  Could not create select widget.");
									init_widgets=0;
								}
								if (!create_spectrum_editor_widget(
									&(spectrum_editor_dialog->editor_widget),
									spectrum_editor_dialog->editor_form,
									(struct Spectrum *)NULL,user_interface,
									glyph_list,
									graphical_material_manager, light_manager,
									spectrum_manager, texture_manager ))
								{
									display_message(ERROR_MESSAGE,
						"create_spectrum_editor_dialog.  Could not create editor widget.");
									init_widgets=0;
								}
								if (init_widgets && (spectrum_editor_dialog->autorange_scene_widget=
									CREATE_CHOOSE_OBJECT_WIDGET(Scene)(
										spectrum_editor_dialog->autorange_scene_form,
										(struct Scene *)NULL, scene_manager,
										(MANAGER_CONDITIONAL_FUNCTION(Scene) *)NULL)))
								{
									spectrum_editor_dialog->autorange_scene =
										CHOOSE_OBJECT_GET_OBJECT(Scene)
										(spectrum_editor_dialog->autorange_scene_widget);
								}
								else
								{
									init_widgets = 0;
								}
								if (init_widgets)
								{
									callback.data=(void *)spectrum_editor_dialog;
									callback.procedure=
										spectrum_editor_dialog_set_autorange_scene;
									CHOOSE_OBJECT_SET_CALLBACK(Scene)(
									spectrum_editor_dialog->autorange_scene_widget,&callback);
									/* set current_value to init_data */
									spectrum_editor_dialog_set_spectrum(
										spectrum_editor_dialog->dialog,init_data);
									callback.procedure=spectrum_editor_dialog_update_selection;
									callback.data=spectrum_editor_dialog;
									SELECT_SET_UPDATE_CB(Spectrum)(
										spectrum_editor_dialog->select_widget,&callback);
									XtRealizeWidget(spectrum_editor_dialog->dialog);
									XtPopup(spectrum_editor_dialog->dialog, XtGrabNone);
									return_widget=spectrum_editor_dialog->dialog;
								}
								else
								{
									DEALLOCATE(spectrum_editor_dialog);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
			"create_spectrum_editor_dialog.  Could not fetch spectrum_editor_dialog");
								DEALLOCATE(spectrum_editor_dialog);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
							"create_spectrum_editor_dialog.  Could not register identifiers");
							DEALLOCATE(spectrum_editor_dialog);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_spectrum_editor_dialog.  Could not register callbacks");
						DEALLOCATE(spectrum_editor_dialog);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_spectrum_editor_dialog.  Could not create popup shell.");
					DEALLOCATE(spectrum_editor_dialog);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
	"create_spectrum_editor_dialog.  Could not allocate spectrum_editor_dialog");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_spectrum_editor_dialog.  Could not open hierarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_spectrum_editor_dialog.  Invalid argument(s)");
	}
	if (spectrum_editor_dialog_widget&&return_widget)
	{
		*spectrum_editor_dialog_widget=return_widget;
	}
	LEAVE;

	return (return_widget);
} /* create_spectrum_editor_dialog */

/*
Global functions
----------------
*/
int spectrum_editor_dialog_get_callback(Widget spectrum_editor_dialog_widget,
	struct Callback_data *callback)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Returns the update_callback for the spectrum editor_dialog widget.
==============================================================================*/
{
	int return_code,num_children;
	struct Spectrum_editor_dialog *spectrum_editor_dialog;
	Widget *child_list;

	ENTER(spectrum_editor_dialog_get_callback);
	/* check arguments */
	if (spectrum_editor_dialog_widget&&callback)
	{
		/* get the pointer to the data for the dialog */
		XtVaGetValues(spectrum_editor_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&num_children,
			NULL);
		if (num_children==1)
		{
			XtVaGetValues(child_list[0],XmNuserData,&spectrum_editor_dialog,
				NULL);
			if (spectrum_editor_dialog)
			{
				callback->procedure=
					spectrum_editor_dialog->update_callback.procedure;
				callback->data=spectrum_editor_dialog->update_callback.data;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"spectrum_editor_dialog_get_callback.  Missing widget data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_dialog_get_callback.  Invalid dialog");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_dialog_get_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_dialog_get_callback */

int spectrum_editor_dialog_set_callback(Widget spectrum_editor_dialog_widget,
	struct Callback_data *callback)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Changes the update_callback for the spectrum editor_dialog widget.
==============================================================================*/
{
	int return_code,num_children;
	struct Spectrum_editor_dialog *spectrum_editor_dialog;
	Widget *child_list;

	ENTER(spectrum_editor_dialog_set_callback);
	if (spectrum_editor_dialog_widget&&callback)
	{
		/* get the pointer to the data for the dialog */
		XtVaGetValues(spectrum_editor_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&num_children,
			NULL);
		if (num_children==1)
		{
			XtVaGetValues(child_list[0],XmNuserData,&spectrum_editor_dialog,
				NULL);
			if (spectrum_editor_dialog)
			{
				spectrum_editor_dialog->update_callback.procedure=
					callback->procedure;
				spectrum_editor_dialog->update_callback.data=callback->data;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"spectrum_editor_dialog_set_callback.  Missing widget data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_dialog_set_callback.  Invalid dialog");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_dialog_set_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_dialog_set_callback */

struct Spectrum *spectrum_editor_dialog_get_spectrum(
	Widget spectrum_editor_dialog_widget)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
If <spectrum_editor_dialog_widget> is not NULL, then get the data item from
<spectrum_editor_dialog widget>.  Otherwise, get the data item from
<spectrum_editor_dialog>.
==============================================================================*/
{
	int num_children;
	struct Spectrum_editor_dialog *spectrum_editor_dialog;
	struct Spectrum *return_spectrum;
	Widget *child_list;

	ENTER(spectrum_editor_dialog_get_spectrum);
	if (spectrum_editor_dialog_widget)
	{
		XtVaGetValues(spectrum_editor_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&num_children,
			NULL);
		if (num_children==1)
		{
			XtVaGetValues(child_list[0],XmNuserData,&spectrum_editor_dialog,
				NULL);
			if (spectrum_editor_dialog)
			{
				return_spectrum=spectrum_editor_dialog->current_value;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"spectrum_editor_dialog_get_spectrum.  Missing widget data");
				return_spectrum=(struct Spectrum *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_dialog_get_spectrum.  Invalid dialog");
			return_spectrum=(struct Spectrum *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_dialog_get_spectrum.  Missing dialog");
		return_spectrum=(struct Spectrum *)NULL;
	}
	LEAVE;

	return (return_spectrum);
} /* spectrum_editor_dialog_get_spectrum */

int spectrum_editor_dialog_set_spectrum(Widget spectrum_editor_dialog_widget,
	struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
If <spectrum_editor_dialog_widget> is not NULL, then change the data item on
<spectrum_editor_dialog widget>.  Otherwise, change the data item on
<spectrum_editor_dialog>.
==============================================================================*/
{
	int num_children,return_code;
	struct Spectrum_editor_dialog *spectrum_editor_dialog;
	Widget *child_list;

	ENTER(spectrum_editor_dialog_set_spectrum);
	if (spectrum_editor_dialog_widget)
	{
		/* get the pointer to the data for the dialog */
		XtVaGetValues(spectrum_editor_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&num_children,
			NULL);
		if (num_children==1)
		{
			XtVaGetValues(child_list[0],XmNuserData,&spectrum_editor_dialog,
				NULL);
			if (spectrum_editor_dialog)
			{
				return_code=1;
				if (spectrum)
				{
					if (!IS_MANAGED(Spectrum)(spectrum,
						spectrum_editor_dialog->spectrum_manager))
					{
						display_message(ERROR_MESSAGE,
							"spectrum_editor_dialog_set_spectrum.  Spectrum not managed");
						spectrum=(struct Spectrum *)NULL;
						return_code=0;
					}
				}
				if (!spectrum)
				{
					spectrum=FIRST_OBJECT_IN_MANAGER_THAT(Spectrum)(
						(MANAGER_CONDITIONAL_FUNCTION(Spectrum) *)NULL,
						(void *)NULL,
						spectrum_editor_dialog->spectrum_manager);
				}
				if (return_code=SELECT_SET_SELECT_ITEM(Spectrum)(
					spectrum_editor_dialog->select_widget,spectrum))
				{
					spectrum_editor_dialog->current_value=spectrum;
					spectrum_editor_set_Spectrum(
						spectrum_editor_dialog->editor_widget,
						spectrum_editor_dialog->current_value);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"spectrum_editor_dialog_set_spectrum.  Missing widget data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_dialog_set_spectrum.  Invalid dialog");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_dialog_set_spectrum.  Missing dialog");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_dialog_set_spectrum */

int bring_up_spectrum_editor_dialog(Widget *spectrum_editor_dialog_address,
	Widget parent,struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *spectrum, struct User_interface *user_interface,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(Light) *light_manager,
	struct MANAGER(Texture) *texture_manager,
	struct MANAGER(Scene) *scene_manager)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
If there is a spectrum_editor dialog in existence, then bring it to the front,
else create a new one.
==============================================================================*/
{
	int return_code;

	ENTER(bring_up_spectrum_editor_dialog);
	if (spectrum_editor_dialog_address)
	{
		if (*spectrum_editor_dialog_address)
		{
			spectrum_editor_dialog_set_spectrum(*spectrum_editor_dialog_address,
				spectrum);
			XtPopup(*spectrum_editor_dialog_address,XtGrabNone);
			return_code=1;
		}
		else
		{
			if (create_spectrum_editor_dialog(spectrum_editor_dialog_address,parent,
				spectrum_manager, spectrum,user_interface,
				glyph_list,
				graphical_material_manager, light_manager,
				texture_manager, scene_manager))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"bring_up_spectrum_editor_dialog.  Error creating dialog");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"bring_up_spectrum_editor_dialog.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* bring_up_spectrum_editor_dialog */
