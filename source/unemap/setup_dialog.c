/*******************************************************************************
FILE : setup_dialog.c

LAST MODIFIED : 24 November 1999

DESCRIPTION :
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#if defined (MOTIF)
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xlib.h>
#include <X11/Composite.h>
#include <X11/Shell.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/LabelG.h>
#include <Xm/DialogS.h>
#include <Xm/DrawingA.h>
#include <Xm/RowColumn.h>
#include <Xm/PushBG.h>
#include <Xm/SeparatoG.h>
#include <Mrm/MrmPublic.h>
#include <Mrm/MrmDecls.h>
#endif /* defined (MOTIF) */
#include "unemap/setup_dialog.h"
#if defined (MOTIF)
#include "unemap/setup_dialog.uidh"
#endif /* defined (MOTIF) */
#include "general/debug.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int setup_dialog_hierarchy_open=0;
static MrmHierarchy setup_dialog_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
static struct Electrodes_in_row *create_Electrodes_in_row(Widget parent,
	Widget above,Widget below,int row,int number_of_electrodes,
	struct Electrodes_in_row *next,int widget_spacing)
/*******************************************************************************
LAST MODIFIED : 23 August 1996

DESCRIPTION :
Allocates the memory for a Electrodes_in_row structure.  Retrieves the necessary
widgets and initializes the appropriate fields.
==============================================================================*/
{
	char label_string[23],number_string[3];
	MrmType electrodes_in_row_class;
	static MrmRegisterArg identifier_list[]=
	{
		{"electrodes_in_row_structure",(XtPointer)NULL}
	};
	struct Electrodes_in_row *electrodes_in_row;
	XtWidgetGeometry geometry_reply;

	ENTER(create_Electrodes_in_row);
	if (setup_dialog_hierarchy_open)
	{
		if (ALLOCATE(electrodes_in_row,struct Electrodes_in_row,1))
		{
			electrodes_in_row->form=(Widget)NULL;
			electrodes_in_row->label=(Widget)NULL;
			electrodes_in_row->down_arrow=(Widget)NULL;
			electrodes_in_row->number_label=(Widget)NULL;
			electrodes_in_row->up_arrow=(Widget)NULL;
			electrodes_in_row->separator=(Widget)NULL;
			electrodes_in_row->row=row;
			if (number_of_electrodes<1)
			{
				electrodes_in_row->number=1;
			}
			else
			{
				if (number_of_electrodes>99)
				{
					electrodes_in_row->number=99;
				}
				else
				{
					electrodes_in_row->number=number_of_electrodes;
				}
			}
			electrodes_in_row->next=next;
			/* assign and register the identifiers */
			identifier_list[0].value=(XtPointer)electrodes_in_row;
			if (MrmSUCCESS==MrmRegisterNamesInHierarchy(setup_dialog_hierarchy,
				identifier_list,XtNumber(identifier_list)))
			{
				/* fetch the electrodes in row "widget" */
				if (MrmSUCCESS==MrmFetchWidget(setup_dialog_hierarchy,
					"setup_electrodes_in_row",parent,&(electrodes_in_row->form),
					&electrodes_in_row_class))
				{
					/* create the separator */
					if (electrodes_in_row->separator=XmCreateSeparatorGadget(parent,
						"electrodes_in_row_separator",(ArgList)NULL,0))
					{
						XtVaSetValues(electrodes_in_row->separator,
							XmNseparatorType,XmNO_LINE,
							XmNleftWidget,electrodes_in_row->form,
							XmNleftAttachment,XmATTACH_WIDGET,
							XmNleftOffset,0,
							XmNrightAttachment,XmATTACH_FORM,
							XmNrightOffset,widget_spacing,
							XmNtopAttachment,XmATTACH_NONE,
							XmNbottomAttachment,XmATTACH_NONE,
							NULL);
						/* configure the electrodes in row "widget"*/
						if (above)
						{
							XtVaSetValues(electrodes_in_row->form,
								XmNtopWidget,above,
								NULL);
						}
						if (below)
						{
							XtVaSetValues(below,
								XmNtopWidget,electrodes_in_row->form,
								NULL);
						}
						sprintf(label_string,"Electrodes in row %1d :",
							electrodes_in_row->row);
						XtVaSetValues(electrodes_in_row->label,
							XmNlabelString,XmStringCreate(label_string,
							XmSTRING_DEFAULT_CHARSET),NULL);
						if (1==electrodes_in_row->number)
						{
							XtUnmanageChild(electrodes_in_row->down_arrow);
						}
						sprintf(number_string,"%2d",electrodes_in_row->number);
						XtVaSetValues(electrodes_in_row->number_label,
							XmNlabelString,XmStringCreate(number_string,
							XmSTRING_DEFAULT_CHARSET),NULL);
						XtManageChild(electrodes_in_row->form);
						XtManageChild(electrodes_in_row->separator);
						XtQueryGeometry(electrodes_in_row->form,(XtWidgetGeometry *)NULL,
							&geometry_reply);
						XtVaSetValues(electrodes_in_row->form,
							XmNwidth,geometry_reply.width,
							XmNheight,geometry_reply.height,
							NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_Electrodes_in_row.  Could not fetch the separator");
						XtDestroyWidget(electrodes_in_row->form);
						DEALLOCATE(electrodes_in_row);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Electrodes_in_row.  Could not fetch the widget");
					DEALLOCATE(electrodes_in_row);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_Electrodes_in_row.  Could not register identifiers");
				DEALLOCATE(electrodes_in_row);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Electrodes_in_row.  Insufficient memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Electrodes_in_row.  Hierarchy not open");
	}
	LEAVE;

	return (electrodes_in_row);
} /* create_Electrodes_in_row */

static int destroy_Electrodes_in_row(
	struct Electrodes_in_row **electrodes_in_row)
/*******************************************************************************
LAST MODIFIED : 31 December 1996

DESCRIPTION :
Frees the memory for <**electrodes_in_row>, destroys the associated widgets and
changes <*electrodes_in_row> to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(destroy_Electrodes_in_row);
	if (electrodes_in_row&&(*electrodes_in_row))
	{
		XtDestroyWidget((*electrodes_in_row)->form);
		XtDestroyWidget((*electrodes_in_row)->separator);
		DEALLOCATE(*electrodes_in_row);
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* destroy_Electrodes_in_row */

static void destroy_setup_dialog(Widget widget,XtPointer setup_dialog_structure,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Tidys up when the user destroys the setup dialog box.
==============================================================================*/
{
	struct Electrodes_in_row *electrodes_in_row,*electrodes_in_row_next;
	struct Setup_dialog *setup_dialog;

	ENTER(destroy_setup_dialog);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (setup_dialog=(struct Setup_dialog *)setup_dialog_structure)
	{
		busy_cursor_off(setup_dialog->shell,setup_dialog->user_interface);
		/* free the memory for the number of electrodes in each row */
		electrodes_in_row_next=setup_dialog->electrodes_in_row;
		while (electrodes_in_row_next)
		{
			electrodes_in_row=electrodes_in_row_next;
			electrodes_in_row_next=electrodes_in_row->next;
			destroy_Electrodes_in_row(&electrodes_in_row);
		}
		if (setup_dialog->address)
		{
			*(setup_dialog->address)=(struct Setup_dialog *)NULL;
		}
		destroy_Shell_list_item(&(setup_dialog->shell_list_item));
		/*???unghost the mapping_setup_button */
		DEALLOCATE(setup_dialog);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_setup_dialog.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* destroy_setup_dialog */

static void identify_electrodes_in_row_form(Widget *widget_id,
	XtPointer electrodes_in_row_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the form for the widget group which controls the number of
electrodes in a row of a rig.
==============================================================================*/
{
	struct Electrodes_in_row *electrodes_in_row;

	ENTER(identify_electrodes_in_row_form);
	USE_PARAMETER(call_data);
	if (electrodes_in_row=(struct Electrodes_in_row *)electrodes_in_row_structure)
	{
		electrodes_in_row->form= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_electrodes_in_row_form.  Missing electrodes_in_row_structure");
	}
	LEAVE;
} /* identify_electrodes_in_row_form */

static void identify_electrodes_in_row_labe(Widget *widget_id,
	XtPointer electrodes_in_row_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the label for the widget group which controls the number of
electrodes in a row of a rig.
==============================================================================*/
{
	struct Electrodes_in_row *electrodes_in_row;

	ENTER(identify_electrodes_in_row_labe);
	USE_PARAMETER(call_data);
	if (electrodes_in_row=(struct Electrodes_in_row *)electrodes_in_row_structure)
	{
		electrodes_in_row->label= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_electrodes_in_row_labe.  Missing electrodes_in_row_structure");
	}
	LEAVE;
} /* identify_electrodes_in_row_labe */

static void identify_electrodes_in_row_down(Widget *widget_id,
	XtPointer electrodes_in_row_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the down arrow for the widget group which controls the number of
electrodes in a row of a rig.
==============================================================================*/
{
	struct Electrodes_in_row *electrodes_in_row;

	ENTER(identify_electrodes_in_row_down);
	USE_PARAMETER(call_data);
	if (electrodes_in_row=(struct Electrodes_in_row *)electrodes_in_row_structure)
	{
		electrodes_in_row->down_arrow= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_electrodes_in_row_down.  Missing electrodes_in_row_structure");
	}
	LEAVE;
} /* identify_electrodes_in_row_down */

static void decrement_electrodes_in_row(Widget widget,
	XtPointer electrodes_in_row_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Decrement the number of electrodes in a row.
==============================================================================*/
{
	struct Electrodes_in_row *electrodes_in_row;
	char number_string[3];

	ENTER(decrement_electrodes_in_row);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (electrodes_in_row=(struct Electrodes_in_row *)electrodes_in_row_structure)
	{
		if (99==electrodes_in_row->number)
		{
			XtManageChild(electrodes_in_row->up_arrow);
		}
		(electrodes_in_row->number)--;
		sprintf(number_string,"%2d",electrodes_in_row->number);
		XtVaSetValues(electrodes_in_row->number_label,
			XmNlabelString,XmStringCreate(number_string,XmSTRING_DEFAULT_CHARSET),
			NULL);
		if (1==electrodes_in_row->number)
		{
			XtUnmanageChild(electrodes_in_row->down_arrow);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"decrement_electrodes_in_row.  Missing electrodes_in_row_structure");
	}
	LEAVE;
} /* decrement_electrodes_in_row */

static void identify_electrodes_in_row_numb(Widget *widget_id,
	XtPointer electrodes_in_row_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the label that gives the number for the widget group which
controls the number of electrodes in a row of a rig.
==============================================================================*/
{
	struct Electrodes_in_row *electrodes_in_row;

	ENTER(identify_electrodes_in_row_numb);
	USE_PARAMETER(call_data);
	if (electrodes_in_row=(struct Electrodes_in_row *)electrodes_in_row_structure)
	{
		electrodes_in_row->number_label= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_electrodes_in_row_numb.  Missing electrodes_in_row_structure");
	}
	LEAVE;
} /* identify_electrodes_in_row_numb */

static void identify_electrodes_in_row_up(Widget *widget_id,
	XtPointer electrodes_in_row_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the up arrow for the widget group which controls the number of
electrodes in a row of a rig.
==============================================================================*/
{
	struct Electrodes_in_row *electrodes_in_row;

	ENTER(identify_electrodes_in_row_up);
	USE_PARAMETER(call_data);
	if (electrodes_in_row=(struct Electrodes_in_row *)electrodes_in_row_structure)
	{
		electrodes_in_row->up_arrow= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_electrodes_in_row_up.  Missing electrodes_in_row_structure");
	}
	LEAVE;
} /* identify_electrodes_in_row_up */

static void increment_electrodes_in_row(Widget widget,
	XtPointer electrodes_in_row_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Increment the number of electrodes in a row.
==============================================================================*/
{
	struct Electrodes_in_row *electrodes_in_row;
	char number_string[3];

	ENTER(increment_electrodes_in_row);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (electrodes_in_row=(struct Electrodes_in_row *)electrodes_in_row_structure)
	{
		if (1==electrodes_in_row->number)
		{
			XtManageChild(electrodes_in_row->down_arrow);
		}
		(electrodes_in_row->number)++;
		sprintf(number_string,"%2d",electrodes_in_row->number);
		XtVaSetValues(electrodes_in_row->number_label,
			XmNlabelString,XmStringCreate(number_string,XmSTRING_DEFAULT_CHARSET),
			NULL);
		if (99==electrodes_in_row->number)
		{
			XtUnmanageChild(electrodes_in_row->up_arrow);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"increment_electrodes_in_row.  Missing electrodes_in_row_structure");
	}
	LEAVE;
} /* increment_electrodes_in_row */

static void identify_setup_rig_choice(Widget *widget_id,
	XtPointer setup_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the rig choice in the setup dialog.
==============================================================================*/
{
	struct Setup_dialog *setup_dialog;

	ENTER(identify_setup_rig_choice);
	USE_PARAMETER(call_data);
	if (setup_dialog=(struct Setup_dialog *)setup_dialog_structure)
	{
		setup_dialog->rig_choice= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_setup_rig_choice.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* identify_setup_rig_choice */

static void identify_setup_rig_sock(Widget *widget_id,
	XtPointer setup_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the sock option in the rig choice in the setup dialog.
==============================================================================*/
{
	struct Setup_dialog *setup_dialog;

	ENTER(identify_setup_rig_sock);
	USE_PARAMETER(call_data);
	if (setup_dialog=(struct Setup_dialog *)setup_dialog_structure)
	{
		setup_dialog->region_type.sock= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_setup_rig_sock.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* identify_setup_rig_sock */

static void identify_setup_rig_patch(Widget *widget_id,
	XtPointer setup_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the patch option in the rig choice in the setup dialog.
==============================================================================*/
{
	struct Setup_dialog *setup_dialog;

	ENTER(identify_setup_rig_patch);
	USE_PARAMETER(call_data);
	if (setup_dialog=(struct Setup_dialog *)setup_dialog_structure)
	{
		setup_dialog->region_type.patch= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_setup_rig_patch.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* identify_setup_rig_patch */

static void identify_setup_rig_torso(Widget *widget_id,
	XtPointer setup_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the torso option in the rig choice in the setup dialog.
==============================================================================*/
{
	struct Setup_dialog *setup_dialog;

	ENTER(identify_setup_rig_torso);
	USE_PARAMETER(call_data);
	if (setup_dialog=(struct Setup_dialog *)setup_dialog_structure)
	{
		setup_dialog->region_type.torso= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_setup_rig_torso.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* identify_setup_rig_torso */

static void identify_setup_auxiliary_down(Widget *widget_id,
	XtPointer setup_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the arrow for decrementing the number of auxiliary devices in
the setup dialog.
==============================================================================*/
{
	struct Setup_dialog *setup_dialog;

	ENTER(identify_setup_auxiliary_down);
	USE_PARAMETER(call_data);
	if (setup_dialog=(struct Setup_dialog *)setup_dialog_structure)
	{
		setup_dialog->auxiliary.down_arrow= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_setup_auxiliary_down.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* identify_setup_auxiliary_down */

static void decrement_number_of_auxiliary(Widget widget,
	XtPointer setup_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Decrement the number of auxiliary devices.
==============================================================================*/
{
	struct Setup_dialog *setup_dialog;
	char number_string[3];

	ENTER(decrement_number_of_auxiliary);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (setup_dialog=(struct Setup_dialog *)setup_dialog_structure)
	{
		if (99==setup_dialog->number_of_auxiliary_devices)
		{
			XtManageChild(setup_dialog->auxiliary.up_arrow);
		}
		(setup_dialog->number_of_auxiliary_devices)--;
		sprintf(number_string,"%2d",setup_dialog->number_of_auxiliary_devices);
		XtVaSetValues(setup_dialog->auxiliary.number,
			XmNlabelString,XmStringCreate(number_string,XmSTRING_DEFAULT_CHARSET),
			NULL);
		if (0==setup_dialog->number_of_auxiliary_devices)
		{
			XtUnmanageChild(setup_dialog->auxiliary.down_arrow);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"decrement_number_of_auxiliary.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* decrement_number_of_auxiliary */

static void identify_setup_auxiliary_number(Widget *widget_id,
	XtPointer setup_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the label for the number of auxiliary devices in the setup
dialog.
==============================================================================*/
{
	struct Setup_dialog *setup_dialog;

	ENTER(identify_setup_auxiliary_number);
	USE_PARAMETER(call_data);
	if (setup_dialog=(struct Setup_dialog *)setup_dialog_structure)
	{
		setup_dialog->auxiliary.number= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_setup_auxiliary_number.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* identify_setup_auxiliary_number */

static void identify_setup_auxiliary_up(Widget *widget_id,
	XtPointer setup_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the arrow for incrementing the number of auxiliary devices in
the setup dialog.
==============================================================================*/
{
	struct Setup_dialog *setup_dialog;

	ENTER(identify_setup_auxiliary_up);
	USE_PARAMETER(call_data);
	if (setup_dialog=(struct Setup_dialog *)setup_dialog_structure)
	{
		setup_dialog->auxiliary.up_arrow= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_setup_auxiliary_up.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* identify_setup_auxiliary_up */

static void increment_number_of_auxiliary(Widget widget,
	XtPointer setup_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Increment the number of auxiliary devices.
==============================================================================*/
{
	struct Setup_dialog *setup_dialog;
	char number_string[3];

	ENTER(increment_number_of_auxiliary);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (setup_dialog=(struct Setup_dialog *)setup_dialog_structure)
	{
		if (0==setup_dialog->number_of_auxiliary_devices)
		{
			XtManageChild(setup_dialog->auxiliary.down_arrow);
		}
		(setup_dialog->number_of_auxiliary_devices)++;
		sprintf(number_string,"%2d",setup_dialog->number_of_auxiliary_devices);
		XtVaSetValues(setup_dialog->auxiliary.number,
			XmNlabelString,XmStringCreate(number_string,XmSTRING_DEFAULT_CHARSET),
			NULL);
		if (99==setup_dialog->number_of_auxiliary_devices)
		{
			XtUnmanageChild(setup_dialog->auxiliary.up_arrow);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"increment_number_of_auxiliary.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* increment_number_of_auxiliary */

static void identify_setup_rows_form(Widget *widget_id,
	XtPointer setup_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the form that contains the widgets for controlling the number of
rows.
==============================================================================*/
{
	struct Setup_dialog *setup_dialog;

	ENTER(identify_setup_rows_form);
	USE_PARAMETER(call_data);
	if (setup_dialog=(struct Setup_dialog *)setup_dialog_structure)
	{
		setup_dialog->rows.form= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_setup_rows_form.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* identify_setup_rows_form */

static void identify_setup_rows_down(Widget *widget_id,
	XtPointer setup_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the arrow for decrementing the number of rows in the setup
dialog.
==============================================================================*/
{
	struct Setup_dialog *setup_dialog;

	ENTER(identify_setup_rows_down);
	USE_PARAMETER(call_data);
	if (setup_dialog=(struct Setup_dialog *)setup_dialog_structure)
	{
		setup_dialog->rows.down_arrow= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_setup_rows_down.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* identify_setup_rows_down */

static void decrement_number_of_rows(Widget widget,
	XtPointer setup_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Decrement the number of rows.
==============================================================================*/
{
	char number_string[3];
	struct Electrodes_in_row *electrodes_in_row_remove;
	struct Setup_dialog *setup_dialog;

	ENTER(decrement_number_of_rows);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (setup_dialog=(struct Setup_dialog *)setup_dialog_structure)
	{
		if (99==setup_dialog->number_of_rows)
		{
			XtManageChild(setup_dialog->rows.up_arrow);
		}
		(setup_dialog->number_of_rows)--;
		/* remove the structure containing the number of electrodes for the removed
			row */
		electrodes_in_row_remove=setup_dialog->electrodes_in_row;
		setup_dialog->electrodes_in_row=setup_dialog->electrodes_in_row->next;
		destroy_Electrodes_in_row(&electrodes_in_row_remove);
		XtVaSetValues(setup_dialog->separator,
			XmNtopWidget,setup_dialog->electrodes_in_row->form,
			NULL);
		sprintf(number_string,"%2d",setup_dialog->number_of_rows);
		XtVaSetValues(setup_dialog->rows.number,
			XmNlabelString,XmStringCreate(number_string,XmSTRING_DEFAULT_CHARSET),
			NULL);
		if (1==setup_dialog->number_of_rows)
		{
			XtUnmanageChild(setup_dialog->rows.down_arrow);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"decrement_number_of_rows.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* decrement_number_of_rows */

static void identify_setup_rows_number(Widget *widget_id,
	XtPointer setup_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the label for the number of rows in the setup dialog.
==============================================================================*/
{
	struct Setup_dialog *setup_dialog;

	ENTER(identify_setup_rows_number);
	USE_PARAMETER(call_data);
	if (setup_dialog=(struct Setup_dialog *)setup_dialog_structure)
	{
		setup_dialog->rows.number= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_setup_rows_number.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* identify_setup_rows_number */

static void identify_setup_rows_up(Widget *widget_id,
	XtPointer setup_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the arrow for incrementing the number of rows in the setup
dialog.
==============================================================================*/
{
	struct Setup_dialog *setup_dialog;

	ENTER(identify_setup_rows_up);
	USE_PARAMETER(call_data);
	if (setup_dialog=(struct Setup_dialog *)setup_dialog_structure)
	{
		setup_dialog->rows.up_arrow= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_setup_rows_up.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* identify_setup_rows_up */

static void increment_number_of_rows(Widget widget,
	XtPointer setup_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Increment the number of rows.
==============================================================================*/
{
	struct Setup_dialog *setup_dialog;
	char number_string[3];

	ENTER(increment_number_of_rows);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((setup_dialog=(struct Setup_dialog *)setup_dialog_structure)&&
		(setup_dialog->user_interface))
	{
		if (1==setup_dialog->number_of_rows)
		{
			XtManageChild(setup_dialog->rows.down_arrow);
		}
		(setup_dialog->number_of_rows)++;
		/* add a new structure for containing the number of electrodes in the new
			row */
		setup_dialog->electrodes_in_row=create_Electrodes_in_row(
			setup_dialog->dialog,setup_dialog->electrodes_in_row->form,
			setup_dialog->separator,setup_dialog->number_of_rows,
			setup_dialog->electrodes_in_row->number,setup_dialog->electrodes_in_row,
			setup_dialog->user_interface->widget_spacing);
		sprintf(number_string,"%2d",setup_dialog->number_of_rows);
		XtVaSetValues(setup_dialog->rows.number,
			XmNlabelString,XmStringCreate(number_string,XmSTRING_DEFAULT_CHARSET),
			NULL);
		if (99==setup_dialog->number_of_rows)
		{
			XtUnmanageChild(setup_dialog->rows.up_arrow);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"increment_number_of_rows.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* increment_number_of_rows */

static void identify_setup_regions_down(Widget *widget_id,
	XtPointer setup_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the arrow for decrementing the number of regions in the setup
dialog.
==============================================================================*/
{
	struct Setup_dialog *setup_dialog;

	ENTER(identify_setup_regions_down);
	USE_PARAMETER(call_data);
	if (setup_dialog=(struct Setup_dialog *)setup_dialog_structure)
	{
		setup_dialog->regions.down_arrow= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_setup_regions_down.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* identify_setup_regions_down */

static void decrement_number_of_regions(Widget widget,
	XtPointer setup_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Decrement the number of regions.
==============================================================================*/
{
	struct Setup_dialog *setup_dialog;
	char number_string[3];

	ENTER(decrement_number_of_regions);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (setup_dialog=(struct Setup_dialog *)setup_dialog_structure)
	{
		if (99==setup_dialog->number_of_regions)
		{
			XtManageChild(setup_dialog->regions.up_arrow);
		}
		(setup_dialog->number_of_regions)--;
		sprintf(number_string,"%2d",setup_dialog->number_of_regions);
		XtVaSetValues(setup_dialog->regions.number,
			XmNlabelString,XmStringCreate(number_string,XmSTRING_DEFAULT_CHARSET),
			NULL);
		if (1==setup_dialog->number_of_regions)
		{
			XtUnmanageChild(setup_dialog->regions.down_arrow);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"decrement_number_of_regions.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* decrement_number_of_regions */

static void identify_setup_regions_number(Widget *widget_id,
	XtPointer setup_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the label for the number of regions in the setup dialog.
==============================================================================*/
{
	struct Setup_dialog *setup_dialog;

	ENTER(identify_setup_regions_number);
	USE_PARAMETER(call_data);
	if (setup_dialog=(struct Setup_dialog *)setup_dialog_structure)
	{
		setup_dialog->regions.number= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_setup_regions_number.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* identify_setup_regions_number */

static void identify_setup_regions_up(Widget *widget_id,
	XtPointer setup_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the arrow for incrementing the number of regions in the setup
dialog.
==============================================================================*/
{
	struct Setup_dialog *setup_dialog;

	ENTER(identify_setup_regions_up);
	USE_PARAMETER(call_data);
	if (setup_dialog=(struct Setup_dialog *)setup_dialog_structure)
	{
		setup_dialog->regions.up_arrow= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_setup_regions_up.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* identify_setup_regions_up */

static void increment_number_of_regions(Widget widget,
	XtPointer setup_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Increment the number of regions.
==============================================================================*/
{
	struct Setup_dialog *setup_dialog;
	char number_string[3];

	ENTER(increment_number_of_regions);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (setup_dialog=(struct Setup_dialog *)setup_dialog_structure)
	{
		if (1==setup_dialog->number_of_regions)
		{
			XtManageChild(setup_dialog->regions.down_arrow);
		}
		(setup_dialog->number_of_regions)++;
		sprintf(number_string,"%2d",setup_dialog->number_of_regions);
		XtVaSetValues(setup_dialog->regions.number,
			XmNlabelString,XmStringCreate(number_string,XmSTRING_DEFAULT_CHARSET),
			NULL);
		if (99==setup_dialog->number_of_regions)
		{
			XtUnmanageChild(setup_dialog->regions.up_arrow);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"increment_number_of_regions.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* increment_number_of_regions */

static void identify_setup_separator(Widget *widget_id,
	XtPointer setup_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the separator between the widgets controlling the number of
electrodes in the last row and the setup and cancel buttons.
==============================================================================*/
{
	struct Setup_dialog *setup_dialog;

	ENTER(identify_setup_separator);
	USE_PARAMETER(call_data);
	if (setup_dialog=(struct Setup_dialog *)setup_dialog_structure)
	{
		setup_dialog->separator= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_setup_separator.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* identify_setup_separator */

static void identify_setup_setup_button(Widget *widget_id,
	XtPointer setup_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the setup button in the setup dialog.
==============================================================================*/
{
	struct Setup_dialog *setup_dialog;

	ENTER(identify_setup_setup_button);
	USE_PARAMETER(call_data);
	if (setup_dialog=(struct Setup_dialog *)setup_dialog_structure)
	{
		setup_dialog->setup_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_setup_setup_button.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* identify_setup_setup_button */

static void identify_setup_cancel_button(Widget *widget_id,
	XtPointer setup_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the cancel button in the setup dialog.
==============================================================================*/
{
	struct Setup_dialog *setup_dialog;

	ENTER(identify_setup_cancel_button);
	USE_PARAMETER(call_data);
	if (setup_dialog=(struct Setup_dialog *)setup_dialog_structure)
	{
		setup_dialog->cancel_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_setup_cancel_button.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* identify_setup_cancel_button */

/*
Global functions
----------------
*/
struct Setup_dialog *create_Setup_dialog(
	struct Setup_dialog **setup_dialog_address,Widget activation,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 19 May 1998

DESCRIPTION :
Allocates the memory for a setup dialog.  Retrieves the necessary widgets and
initializes the appropriate fields.
==============================================================================*/
{
	char number_string[3];
	int i;
	MrmType setup_dialog_class;
	static MrmRegisterArg callback_list[]={
		{"identify_electrodes_in_row_form",
			(XtPointer)identify_electrodes_in_row_form},
		{"identify_electrodes_in_row_labe",
			(XtPointer)identify_electrodes_in_row_labe},
		{"identify_electrodes_in_row_down",
			(XtPointer)identify_electrodes_in_row_down},
		{"decrement_electrodes_in_row",(XtPointer)decrement_electrodes_in_row},
		{"identify_electrodes_in_row_numb",
			(XtPointer)identify_electrodes_in_row_numb},
		{"identify_electrodes_in_row_up",(XtPointer)identify_electrodes_in_row_up},
		{"increment_electrodes_in_row",(XtPointer)increment_electrodes_in_row},
		{"identify_setup_rig_choice",(XtPointer)identify_setup_rig_choice},
		{"identify_setup_rig_sock",(XtPointer)identify_setup_rig_sock},
		{"identify_setup_rig_patch",(XtPointer)identify_setup_rig_patch},
		{"identify_setup_rig_torso",(XtPointer)identify_setup_rig_torso},
		{"identify_setup_auxiliary_down",(XtPointer)identify_setup_auxiliary_down},
		{"decrement_number_of_auxiliary",(XtPointer)decrement_number_of_auxiliary},
		{"identify_setup_auxiliary_number",
			(XtPointer)identify_setup_auxiliary_number},
		{"identify_setup_auxiliary_up",(XtPointer)identify_setup_auxiliary_up},
		{"increment_number_of_auxiliary",(XtPointer)increment_number_of_auxiliary},
		{"identify_setup_rows_form",(XtPointer)identify_setup_rows_form},
		{"identify_setup_rows_down",(XtPointer)identify_setup_rows_down},
		{"decrement_number_of_rows",(XtPointer)decrement_number_of_rows},
		{"identify_setup_rows_number",(XtPointer)identify_setup_rows_number},
		{"identify_setup_rows_up",(XtPointer)identify_setup_rows_up},
		{"increment_number_of_rows",(XtPointer)increment_number_of_rows},
		{"identify_setup_regions_down",(XtPointer)identify_setup_regions_down},
		{"decrement_number_of_regions",(XtPointer)decrement_number_of_regions},
		{"identify_setup_regions_number",(XtPointer)identify_setup_regions_number},
		{"identify_setup_regions_up",(XtPointer)identify_setup_regions_up},
		{"increment_number_of_regions",(XtPointer)increment_number_of_regions},
		{"identify_setup_separator",(XtPointer)identify_setup_separator},
		{"identify_setup_setup_button",(XtPointer)identify_setup_setup_button},
		{"identify_setup_cancel_button",(XtPointer)identify_setup_cancel_button},
		{"close_setup_dialog",(XtPointer)close_setup_dialog}};
	static MrmRegisterArg identifier_list[]=
	{
		{"setup_dialog_structure",(XtPointer)NULL}
	};
	struct Setup_dialog *setup_dialog;
	Widget above,parent;

	ENTER(create_Setup_dialog);
	/* check arguments */
	if (user_interface)
	{
		if (MrmOpenHierarchy_base64_string(setup_dialog_uidh,
			&setup_dialog_hierarchy,&setup_dialog_hierarchy_open))
		{
			if (ALLOCATE(setup_dialog,struct Setup_dialog,1))
			{
				setup_dialog->activation=activation;
				setup_dialog->address=setup_dialog_address;
				setup_dialog->user_interface=user_interface;
				setup_dialog->dialog=(Widget)NULL;
				setup_dialog->shell=(Widget)NULL;
				setup_dialog->shell_list_item=(struct Shell_list_item *)NULL;
				setup_dialog->rig_choice=(Widget)NULL;
				setup_dialog->auxiliary.down_arrow=(Widget)NULL;
				setup_dialog->auxiliary.number=(Widget)NULL;
				setup_dialog->auxiliary.up_arrow=(Widget)NULL;
				setup_dialog->number_of_auxiliary_devices=0;
				setup_dialog->rows.form=(Widget)NULL;
				setup_dialog->rows.down_arrow=(Widget)NULL;
				setup_dialog->rows.number=(Widget)NULL;
				setup_dialog->rows.up_arrow=(Widget)NULL;
				setup_dialog->number_of_rows=8;
				setup_dialog->regions.down_arrow=(Widget)NULL;
				setup_dialog->regions.number=(Widget)NULL;
				setup_dialog->regions.up_arrow=(Widget)NULL;
				setup_dialog->number_of_regions=8;
				setup_dialog->separator=(Widget)NULL;
				setup_dialog->setup_button=(Widget)NULL;
				setup_dialog->cancel_button=(Widget)NULL;
				/* create the dialog shell */
				if (!(parent=activation)||(True!=XtIsWidget(parent)))
				{
					parent=user_interface->application_shell;
				}
				if (setup_dialog->shell=XtVaCreatePopupShell(
					"setup_dialog_shell",
					xmDialogShellWidgetClass,parent,
					XmNmwmDecorations,MWM_DECOR_ALL|MWM_DECOR_RESIZEH,
					XmNmwmFunctions,MWM_FUNC_MOVE|MWM_FUNC_CLOSE,
					XmNtitle,"Basic configuration",
					XmNallowShellResize,True,
					NULL))
				{
					setup_dialog->shell_list_item=
						create_Shell_list_item(&(setup_dialog->shell),user_interface);
					/* add the destroy callback */
					XtAddCallback(setup_dialog->shell,XmNdestroyCallback,
						destroy_setup_dialog,(XtPointer)setup_dialog);
					/* register the other callbacks */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(setup_dialog_hierarchy,
						callback_list,XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value=(XtPointer)setup_dialog;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(setup_dialog_hierarchy,
							identifier_list,XtNumber(identifier_list)))
						{
							/* fetch the dialog widget */
							if (MrmSUCCESS==MrmFetchWidget(setup_dialog_hierarchy,
								"setup_dialog",setup_dialog->shell,&(setup_dialog->dialog),
								&setup_dialog_class))
							{
								/* create controls for the number of electrodes in each row */
								above=setup_dialog->rows.form;
								setup_dialog->electrodes_in_row=
									(struct Electrodes_in_row *)NULL;
								i=1;
								while ((i<=setup_dialog->number_of_rows)&&
									(setup_dialog->electrodes_in_row=create_Electrodes_in_row(
									setup_dialog->dialog,above,setup_dialog->separator,i,8,
									setup_dialog->electrodes_in_row,
									user_interface->widget_spacing)))
								{
									above=setup_dialog->electrodes_in_row->form;
									i++;
								}
								if (i>setup_dialog->number_of_rows)
								{
									XtManageChild(setup_dialog->dialog);
									XtRealizeWidget(setup_dialog->shell);
									if (setup_dialog_address)
									{
										*setup_dialog_address=setup_dialog;
									}
									/* configure the dialog box */
									sprintf(number_string,"%2d",
										setup_dialog->number_of_auxiliary_devices);
									XtVaSetValues(setup_dialog->auxiliary.number,
										XmNlabelString,XmStringCreate(number_string,
											XmSTRING_DEFAULT_CHARSET),NULL);
									if (0==setup_dialog->number_of_auxiliary_devices)
									{
										XtUnmanageChild(setup_dialog->auxiliary.down_arrow);
									}
									sprintf(number_string,"%2d",setup_dialog->number_of_rows);
									XtVaSetValues(setup_dialog->rows.number,
										XmNlabelString,XmStringCreate(number_string,
										XmSTRING_DEFAULT_CHARSET),NULL);
									sprintf(number_string,"%2d",setup_dialog->number_of_regions);
									XtVaSetValues(setup_dialog->regions.number,
										XmNlabelString,XmStringCreate(number_string,
										XmSTRING_DEFAULT_CHARSET),NULL);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"create_Setup_dialog.  Could not fetch the dialog widget");
									XtDestroyWidget(setup_dialog->shell);
									setup_dialog=(struct Setup_dialog *)NULL;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"create_Setup_dialog.  Could not fetch the dialog widget");
								XtDestroyWidget(setup_dialog->shell);
								setup_dialog=(struct Setup_dialog *)NULL;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_Setup_dialog.  Could not register identifiers");
							XtDestroyWidget(setup_dialog->shell);
							setup_dialog=(struct Setup_dialog *)NULL;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_Setup_dialog.  Could not register callbacks");
						XtDestroyWidget(setup_dialog->shell);
						setup_dialog=(struct Setup_dialog *)NULL;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Setup_dialog.  Could not create dialog shell");
					DEALLOCATE(setup_dialog);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_Setup_dialog.  Insufficient memory for setup dialog");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Setup_dialog.  Could not open hierarchy");
			setup_dialog=(struct Setup_dialog *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Setup_dialog.  Missing user_interface");
		setup_dialog=(struct Setup_dialog *)NULL;
	}
	LEAVE;

	return (setup_dialog);
} /* create_Setup_dialog */

void close_setup_dialog(Widget widget,XtPointer setup_dialog_structure,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Closes the windows associated with the setup dialog box.
==============================================================================*/
{
	struct Setup_dialog *setup_dialog;

	ENTER(close_setup_dialog);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (setup_dialog=(struct Setup_dialog *)setup_dialog_structure)
	{
		busy_cursor_off(setup_dialog->shell,setup_dialog->user_interface);
		/* close the setup dialog box */
		XtUnmanageChild(setup_dialog->dialog);
		/* unghost the activation button */
		XtSetSensitive(setup_dialog->activation,True);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"close_setup_dialog.  Missing setup_dialog_structure");
	}
	LEAVE;
} /* close_setup_dialog */
