/*******************************************************************************
FILE : eimaging_time_dialog.c

LAST MODIFIED : 27 June 2001

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
#include <Xm/Protocols.h>
#include <Xm/ToggleBG.h>
#include <Mrm/MrmPublic.h>
#include <Mrm/MrmDecls.h>
#endif /* defined (MOTIF) */
#include "general/debug.h"
#include "unemap/analysis.h"
#include "unemap/eimaging_time_dialog.h"
#include "unemap/eimaging_time_dialog.uidh"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
static int eimaging_time_dialog_hierarchy_open=0;
static MrmHierarchy eimaging_time_dialog_hierarchy;

/*
Module constants
----------------
*/

/*
Module types
------------
*/
typedef struct Electrical_imaging_time_dialog Electrical_imaging_time_dialog_settings;

/*
Module functions
----------------
*/

static void destroy_eimaging_time_dialog(Widget widget,
	XtPointer eimaging_time_dialog_structure,	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Tidys up when the user destroys the electrical imaging time dialog box.
==============================================================================*/
{
	struct Electrical_imaging_time_dialog *eimaging_time_dialog;

	ENTER(destroy_eimaging_time_dialog);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)
		eimaging_time_dialog_structure)
	{
		busy_cursor_off(eimaging_time_dialog->shell,eimaging_time_dialog->user_interface);
		if (eimaging_time_dialog->address)
		{
			*(eimaging_time_dialog->address)=(struct Electrical_imaging_time_dialog *)NULL;
		}
		destroy_Shell_list_item(&(eimaging_time_dialog->shell_list_item));
		/*???unghost the electrical imaging time button ? */
		DEALLOCATE(eimaging_time_dialog);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_eimaging_time_dialog.  Missing eimaging_time_dialog_structure");
	}
	LEAVE;
} /* destroy_eimaging_time_dialog */

static void id_eimaging_time_dlg_apply_but(Widget *widget_id,
	XtPointer eimaging_time_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Saves the id of the eimaging_time_dialog->apply_button
==============================================================================*/
{

	struct Electrical_imaging_time_dialog *eimaging_time_dialog;

	ENTER(id_eimaging_time_dlg_apply_but);
	USE_PARAMETER(call_data);
	if (eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)
		eimaging_time_dialog_structure)
	{
		eimaging_time_dialog->apply_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_eimaging_time_dlg_apply_but."
			" Missing eimaging_time_dialog_structure");
	}
	LEAVE;
} /* id_eimaging_time_dlg_apply_but */

static void id_eimaging_time_dlg_cancel_but(Widget *widget_id,
	XtPointer eimaging_time_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Saves the id of the eimaging_time_dialog->cancel_button
==============================================================================*/
{

	struct Electrical_imaging_time_dialog *eimaging_time_dialog;

	ENTER(id_eimaging_time_dlg_cancel_but);
	USE_PARAMETER(call_data);
	if (eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)
		eimaging_time_dialog_structure)
	{
		eimaging_time_dialog->cancel_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_eimaging_time_dlg_cancel_but."
			" Missing eimaging_time_dialog_structure");
	}
	LEAVE;
} /* id_eimaging_time_dlg_cancel_but  */

static void id_eimaging_time_dlg_ok_but(Widget *widget_id,
	XtPointer eimaging_time_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Saves the id of the eimaging_time_dialog->ok_button
==============================================================================*/
{

	struct Electrical_imaging_time_dialog *eimaging_time_dialog;

	ENTER(id_eimaging_time_dlg_ok_but);
	USE_PARAMETER(call_data);
	if (eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)
		eimaging_time_dialog_structure)
	{
		eimaging_time_dialog->ok_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_eimaging_time_dlg_ok_but."
			" Missing eimaging_time_dialog_structure");
	}
	LEAVE;
} /* id_eimaging_time_dlg_ok_but  */

static void identify_eimaging_period_menu(Widget *widget_id,
	XtPointer eimaging_time_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Saves the id of the eimaging_time_dialog->period_menu
==============================================================================*/
{

	struct Electrical_imaging_time_dialog *eimaging_time_dialog;

	ENTER(identify_eimaging_period_menu);
	USE_PARAMETER(call_data);
	if (eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)
		eimaging_time_dialog_structure)
	{
		eimaging_time_dialog->period_menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_eimaging_period_menu."
			" Missing eimaging_time_dialog_structure");
	}
	LEAVE;
} /* identify_eimaging_period_menu  */

static void identify_eimaging_period_label(Widget *widget_id,
	XtPointer eimaging_time_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Saves the id of the eimaging_time_dialog->period_label
==============================================================================*/
{

	struct Electrical_imaging_time_dialog *eimaging_time_dialog;

	ENTER(identify_eimaging_period_label);
	USE_PARAMETER(call_data);
	if (eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)
		eimaging_time_dialog_structure)
	{
		eimaging_time_dialog->period_label= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_eimaging_period_label."
			" Missing eimaging_time_dialog_structure");
	}
	LEAVE;
} /* identify_eimaging_period_label  */

static void identify_eimaging_period_scroll(Widget *widget_id,
	XtPointer eimaging_time_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Saves the id of the eimaging_time_dialog->period_scroll
==============================================================================*/
{

	struct Electrical_imaging_time_dialog *eimaging_time_dialog;

	ENTER(identify_eimaging_period_scroll);
	USE_PARAMETER(call_data);
	if (eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)
		eimaging_time_dialog_structure)
	{
		eimaging_time_dialog->period_scroll= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_eimaging_period_scroll."
			" Missing eimaging_time_dialog_structure");
	}
	LEAVE;
} /*  identify_eimaging_period_scroll */

static void identify_eimaging_period_value(Widget *widget_id,
	XtPointer eimaging_time_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Saves the id of the eimaging_time_dialog->period_value
==============================================================================*/
{

	struct Electrical_imaging_time_dialog *eimaging_time_dialog;

	ENTER(identify_eimaging_period_value);
	USE_PARAMETER(call_data);
	if (eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)
		eimaging_time_dialog_structure)
	{
		eimaging_time_dialog->period_value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_eimaging_period_value."
			" Missing eimaging_time_dialog_structure");
	}
	LEAVE;
} /* identify_eimaging_period_value  */

int alter_eimaging_button_event_info(Widget widget,
	struct Electrical_imaging_time_dialog *eimaging_time_dialog)
/*******************************************************************************
LAST MODIFIED : 26 June 2001

DESCRIPTION :
Alters the dialog's button informtion used to generate the event time markers.
The event time markers are updated in update_eimaging_evnts_frm_dlg
==============================================================================*/
{
	int return_code;

	ENTER(alter_eimaging_button_event_info);
	if(eimaging_time_dialog)
	{
		return_code=1;
		if(widget==eimaging_time_dialog->clear_button)
		{
			if(eimaging_time_dialog->reference_event!=EVENT_CLEAR)
			{
				eimaging_time_dialog->reference_event=EVENT_CLEAR;
				eimaging_time_dialog->settings_changed=1;
			}
		}	
		else if(widget==eimaging_time_dialog->current_button)
		{	
			if(eimaging_time_dialog->reference_event!=EVENT_CURRENT)
			{
				eimaging_time_dialog->reference_event=EVENT_CURRENT;				
				eimaging_time_dialog->settings_changed=1;
			}
		}
		else if(widget==eimaging_time_dialog->p_wave_start_button)
		{
			if(eimaging_time_dialog->reference_event!=EVENT_P_WAVE_START)
			{
				eimaging_time_dialog->reference_event=EVENT_P_WAVE_START;		
				eimaging_time_dialog->settings_changed=1;
			}
		}
		else if(widget==eimaging_time_dialog->p_wave_peak_or_trough_button)
		{
			if(eimaging_time_dialog->reference_event!=EVENT_P_WAVE_PEAK_OR_TROUGH)
			{
				eimaging_time_dialog->reference_event=EVENT_P_WAVE_PEAK_OR_TROUGH;
				eimaging_time_dialog->settings_changed=1;
			}
		}
		else if(widget==eimaging_time_dialog->p_wave_end_button)
		{
			if(eimaging_time_dialog->reference_event!=EVENT_P_WAVE_END)
			{
				eimaging_time_dialog->reference_event=EVENT_P_WAVE_END;
				eimaging_time_dialog->settings_changed=1;
			}
		}
		else if(widget==eimaging_time_dialog->qrs_wave_start_button)
		{
			if(eimaging_time_dialog->reference_event!=EVENT_QRS_WAVE_START)
			{
				eimaging_time_dialog->reference_event=EVENT_QRS_WAVE_START;
				eimaging_time_dialog->settings_changed=1;
			}
		}
		else if(widget==eimaging_time_dialog->qrs_wave_peak_or_trough_button)
		{
			if(eimaging_time_dialog->reference_event!=EVENT_QRS_WAVE_PEAK_OR_TROUGH)
			{
				eimaging_time_dialog->reference_event=EVENT_QRS_WAVE_PEAK_OR_TROUGH;
				eimaging_time_dialog->settings_changed=1;
			}
		}
		else if(widget==eimaging_time_dialog->qrs_wave_end_button)
		{			
			if(eimaging_time_dialog->reference_event!=EVENT_QRS_WAVE_END)
			{
				eimaging_time_dialog->reference_event=EVENT_QRS_WAVE_END;
				eimaging_time_dialog->settings_changed=1;
			}
		}
		else if(widget==eimaging_time_dialog->t_wave_start_button)
		{
			if(eimaging_time_dialog->reference_event!=EVENT_T_WAVE_START)
			{
				eimaging_time_dialog->reference_event=EVENT_T_WAVE_START;
				eimaging_time_dialog->settings_changed=1;
			}
		}
		else if(widget==eimaging_time_dialog->t_wave_peak_or_trough_button)
		{
			if(eimaging_time_dialog->reference_event!=EVENT_T_WAVE_PEAK_OR_TROUGH)
			{
				eimaging_time_dialog->reference_event=EVENT_T_WAVE_PEAK_OR_TROUGH;
				eimaging_time_dialog->settings_changed=1;
			}
		}		
		else if(widget==eimaging_time_dialog->t_wave_end_button)
		{
			if(eimaging_time_dialog->reference_event!=EVENT_T_WAVE_END)
			{
				eimaging_time_dialog->reference_event=EVENT_T_WAVE_END;				
				eimaging_time_dialog->settings_changed=1;
			}
		}
	}
	else
	{
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/*alter_eimaging_button_event_info */

void alter_eimaging_event_info(Widget widget,
	XtPointer eimaging_time_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 June 2001

DESCRIPTION :
Alters the dialog's informtion used to generate the event time markers.
The event time markers are updated in update_eimaging_evnts_frm_dlg
==============================================================================*/
{
	char *new_value,value_string[20];
	float period;
	struct Electrical_imaging_time_dialog *eimaging_time_dialog;
	XmAnyCallbackStruct *text_data;
	XmScrollBarCallbackStruct *scroll_data;

	ENTER(alter_eimaging_event_info);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (eimaging_time_dialog=(struct Electrical_imaging_time_dialog  *)
		eimaging_time_dialog_structure)
	{
		/* change value from scroll bar */
		if (widget==eimaging_time_dialog->period_scroll)
		{
			if ((scroll_data=(XmScrollBarCallbackStruct *)call_data)&&
				(XmCR_VALUE_CHANGED==scroll_data->reason))
			{
				eimaging_time_dialog->settings_changed=1;
				period=(float)(scroll_data->value)*eimaging_time_dialog->max_period/
					eimaging_time_dialog->scroll_width;
				if (period<0.)
				{
					period=0.;
				}
				eimaging_time_dialog->marker_period=period;
				sprintf(value_string,"%g",eimaging_time_dialog->marker_period);
				XtVaSetValues(eimaging_time_dialog->period_value,XmNvalue,
					value_string,NULL);			
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"alter_eimaging_event_info. Error from period scroll bar");
			}			
		}
		else if(widget==eimaging_time_dialog->period_value)
		{
			if ((text_data=(XmAnyCallbackStruct *)call_data)&&
				((XmCR_ACTIVATE==text_data->reason)||
					(XmCR_LOSING_FOCUS==text_data->reason)))
			{
				eimaging_time_dialog->settings_changed=1;
				XtVaGetValues(eimaging_time_dialog->period_value,XmNvalue,&new_value,
					NULL);
				if (1==sscanf(new_value,"%f",&period))
				{
					if (period<0.) 
					{
						period=0.; 
					}
					else if(period>eimaging_time_dialog->max_period)
					{
						period=eimaging_time_dialog->max_period;
					}
				}	
				eimaging_time_dialog->marker_period=period;		
				sprintf(value_string,"%g",eimaging_time_dialog->marker_period);
				XtVaSetValues(eimaging_time_dialog->period_value,XmNvalue,
					value_string,NULL);
				XtVaSetValues(eimaging_time_dialog->period_scroll,XmNvalue,
					(int)(eimaging_time_dialog->marker_period*eimaging_time_dialog->scroll_width/
						eimaging_time_dialog->max_period),NULL);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"alter_eimaging_event_info. Error id period_value");
			}
		}
		else
		{
			alter_eimaging_button_event_info(widget,eimaging_time_dialog);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"alter_eimaging_event_info. Missing eimaging_time_dialog_structure");
	}
	LEAVE;
} /*alter_eimaging_event_info  */

void close_eimaging_time_dialog(Widget widget,
	XtPointer eimaging_time_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Closes the windows associated with the imaging_time_dialog box.
==============================================================================*/
{
	struct Electrical_imaging_time_dialog *eimaging_time_dialog;

	ENTER(close_eimaging_time_dialog);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (eimaging_time_dialog=(struct Electrical_imaging_time_dialog  *)
		eimaging_time_dialog_structure)
	{
		busy_cursor_off(eimaging_time_dialog->shell,eimaging_time_dialog->user_interface);
		/* close the map dialog box */
		XtUnmanageChild(eimaging_time_dialog->dialog);
		/* unghost the activation button */
		XtSetSensitive(eimaging_time_dialog->activation,TRUE);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"close_eimaging_time_dialog. Missing eimaging_time_dialog_structure");
	}
	LEAVE;
} /*close_eimaging_time_dialog  */

static void id_eimaging_event_menu(Widget *widget_id,
	XtPointer eimaging_time_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Saves the id of the eimaging_time_dialog->event_menu
==============================================================================*/
{

	struct Electrical_imaging_time_dialog *eimaging_time_dialog;

	ENTER(id_eimaging_event_menu);
	USE_PARAMETER(call_data);
	if (eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)
		eimaging_time_dialog_structure)
	{
		eimaging_time_dialog->event_menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_eimaging_event_menu."
			" Missing eimaging_time_dialog_structure");
	}
	LEAVE;
} /* id_eimaging_event_menu  */

static void id_eimaging_event_choice(Widget *widget_id,
	XtPointer eimaging_time_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Saves the id of the eimaging_time_dialog->event_choice
==============================================================================*/
{

	struct Electrical_imaging_time_dialog *eimaging_time_dialog;

	ENTER(id_eimaging_event_choice);
	USE_PARAMETER(call_data);
	if (eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)
		eimaging_time_dialog_structure)
	{
		eimaging_time_dialog->event_choice= *widget_id;			
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_eimaging_event_choice."
			" Missing eimaging_time_dialog_structure");
	}
	LEAVE;
} /* id_eimaging_event_choice  */

static void id_eimaging_event_pulldn(Widget *widget_id,
	XtPointer eimaging_time_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Saves the id of the eimaging_time_dialog->event_pull_down
==============================================================================*/
{

	struct Electrical_imaging_time_dialog *eimaging_time_dialog;

	ENTER(id_eimaging_event_pulldn);
	USE_PARAMETER(call_data);
	if (eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)
		eimaging_time_dialog_structure)
	{
		eimaging_time_dialog->event_pull_down= *widget_id;
		/* to match first entry in uil file*/
		eimaging_time_dialog->reference_event=EVENT_CLEAR; 		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_eimaging_event_pulldn."
			" Missing eimaging_time_dialog_structure");
	}
	LEAVE;
} /* id_eimaging_event_pulldn  */

static void id_eimaging_clear_butt(Widget *widget_id,
	XtPointer eimaging_time_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Saves the id of the eimaging_time_dialog->clear_button
==============================================================================*/
{

	struct Electrical_imaging_time_dialog *eimaging_time_dialog;

	ENTER(id_eimaging_clear_butt);
	USE_PARAMETER(call_data);
	if (eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)
		eimaging_time_dialog_structure)
	{
		eimaging_time_dialog->clear_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_eimaging_clear_butt."
			" Missing eimaging_time_dialog_structure");
	}
	LEAVE;
} /* id_eimaging_clear_butt  */

static void id_eimaging_current_butt(Widget *widget_id,
	XtPointer eimaging_time_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Saves the id of the eimaging_time_dialog->current_button
==============================================================================*/
{

	struct Electrical_imaging_time_dialog *eimaging_time_dialog;

	ENTER(id_eimaging_current_butt);
	USE_PARAMETER(call_data);
	if (eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)
		eimaging_time_dialog_structure)
	{
		eimaging_time_dialog->current_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_eimaging_current_butt."
			" Missing eimaging_time_dialog_structure");
	}
	LEAVE;
} /* id_eimaging_current_butt  */

static void id_eimaging_p_start_butt(Widget *widget_id,
	XtPointer eimaging_time_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Saves the id of the eimaging_time_dialog->p_wave_start_button
==============================================================================*/
{

	struct Electrical_imaging_time_dialog *eimaging_time_dialog;

	ENTER(id_eimaging_p_start_butt);
	USE_PARAMETER(call_data);
	if (eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)
		eimaging_time_dialog_structure)
	{
		eimaging_time_dialog->p_wave_start_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_eimaging_p_start_butt."
			" Missing eimaging_time_dialog_structure");
	}
	LEAVE;
} /* id_eimaging_p_start_butt  */

static void id_eimaging_p_pt_butt(Widget *widget_id,
	XtPointer eimaging_time_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Saves the id of the eimaging_time_dialog->p_wave_peak_or_trough_button
==============================================================================*/
{

	struct Electrical_imaging_time_dialog *eimaging_time_dialog;

	ENTER(id_eimaging_p_pt_butt);
	USE_PARAMETER(call_data);
	if (eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)
		eimaging_time_dialog_structure)
	{
		eimaging_time_dialog->p_wave_peak_or_trough_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_eimaging_p_pt_butt."
			" Missing eimaging_time_dialog_structure");
	}
	LEAVE;
} /* id_eimaging_p_pt_butt  */

static void id_eimaging_p_end_butt(Widget *widget_id,
	XtPointer eimaging_time_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Saves the id of the eimaging_time_dialog->p_wave_end_button
==============================================================================*/
{

	struct Electrical_imaging_time_dialog *eimaging_time_dialog;

	ENTER(id_eimaging_p_end_butt);
	USE_PARAMETER(call_data);
	if (eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)
		eimaging_time_dialog_structure)
	{
		eimaging_time_dialog->p_wave_end_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_eimaging_p_end_butt."
			" Missing eimaging_time_dialog_structure");
	}
	LEAVE;
} /* id_eimaging_p_end_butt  */

static void id_eimaging_qrs_start_butt(Widget *widget_id,
	XtPointer eimaging_time_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Saves the id of the eimaging_time_dialog->qrs_wave_start_button
==============================================================================*/
{

	struct Electrical_imaging_time_dialog *eimaging_time_dialog;

	ENTER(id_eimaging_qrs_start_butt);
	USE_PARAMETER(call_data);
	if (eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)
		eimaging_time_dialog_structure)
	{
		eimaging_time_dialog->qrs_wave_start_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_eimaging_qrs_start_butt."
			" Missing eimaging_time_dialog_structure");
	}
	LEAVE;
} /* id_eimaging_qrs_start_butt  */

static void id_eimaging_qrs_pt_butt(Widget *widget_id,
	XtPointer eimaging_time_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Saves the id of the eimaging_time_dialog->qrs_wave_peak_or_trough_button
==============================================================================*/
{

	struct Electrical_imaging_time_dialog *eimaging_time_dialog;

	ENTER(id_eimaging_qrs_pt_butt);
	USE_PARAMETER(call_data);
	if (eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)
		eimaging_time_dialog_structure)
	{
		eimaging_time_dialog->qrs_wave_peak_or_trough_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_eimaging_qrs_pt_butt."
			" Missing eimaging_time_dialog_structure");
	}
	LEAVE;
} /* id_eimaging_qrs_pt_butt  */

static void id_eimaging_qrs_end_butt(Widget *widget_id,
	XtPointer eimaging_time_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Saves the id of the eimaging_time_dialog->qrs_wave_end_button
==============================================================================*/
{

	struct Electrical_imaging_time_dialog *eimaging_time_dialog;

	ENTER(id_eimaging_qrs_end_butt);
	USE_PARAMETER(call_data);
	if (eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)
		eimaging_time_dialog_structure)
	{
		eimaging_time_dialog->qrs_wave_end_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_eimaging_qrs_end_butt."
			" Missing eimaging_time_dialog_structure");
	}
	LEAVE;
} /* id_eimaging_qrs_end_butt  */

static void id_eimaging_t_start_butt(Widget *widget_id,
	XtPointer eimaging_time_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Saves the id of the eimaging_time_dialog->t_wave_start_button
==============================================================================*/
{

	struct Electrical_imaging_time_dialog *eimaging_time_dialog;

	ENTER(id_eimaging_t_start_butt);
	USE_PARAMETER(call_data);
	if (eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)
		eimaging_time_dialog_structure)
	{
		eimaging_time_dialog->t_wave_start_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_eimaging_t_start_butt."
			" Missing eimaging_time_dialog_structure");
	}
	LEAVE;
} /* id_eimaging_t_start_butt  */

static void id_eimaging_t_pt_butt(Widget *widget_id,
	XtPointer eimaging_time_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Saves the id of the eimaging_time_dialog->t_wave_peak_or_trough_button
==============================================================================*/
{

	struct Electrical_imaging_time_dialog *eimaging_time_dialog;

	ENTER(id_eimaging_t_pt_butt);
	USE_PARAMETER(call_data);
	if (eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)
		eimaging_time_dialog_structure)
	{
		eimaging_time_dialog->t_wave_peak_or_trough_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_eimaging_t_pt_butt."
			" Missing eimaging_time_dialog_structure");
	}
	LEAVE;
} /* id_eimaging_t_pt_butt  */

static void id_eimaging_t_end_butt(Widget *widget_id,
	XtPointer eimaging_time_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Saves the id of the eimaging_time_dialog->t_wave_end_button
==============================================================================*/
{

	struct Electrical_imaging_time_dialog *eimaging_time_dialog;

	ENTER(id_eimaging_t_end_butt);
	USE_PARAMETER(call_data);
	if (eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)
		eimaging_time_dialog_structure)
	{
		eimaging_time_dialog->t_wave_end_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_eimaging_t_end_butt."
			" Missing eimaging_time_dialog_structure");
	}
	LEAVE;
} /* id_eimaging_t_end_butt  */

/*
Global functions
----------------
*/
struct Electrical_imaging_time_dialog *create_Electrical_imaging_time_dialog(
	struct Electrical_imaging_time_dialog **eimaging_time_dialog_address,
	Widget activation,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 26 June 2000

DESCRIPTION :
Allocates the memory for an Electrical_imaging_time dialog.  Retrieves the 
necessary widgets and initializes the appropriate fields.
==============================================================================*/
{
#define XmNeimagingTimeMaxPeriod "eimagingTimeMaxPeriod"
#define XmCEimagingTimeMaxPeriod "EimagingTimeMaxPeriod"

	Atom WM_DELETE_WINDOW;
	char value_string[20];
	int scroll_max,scroll_min,scroll_silder_size;
	MrmType eimaging_time_dialog_class;
	Pixmap no_cascade_pixmap;
	struct Electrical_imaging_time_dialog *eimaging_time_dialog;
	Widget child_widget,parent;
	static MrmRegisterArg callback_list[]={
		{"id_eimaging_time_dlg_apply_but",
			(XtPointer)id_eimaging_time_dlg_apply_but},
		{"id_eimaging_time_dlg_cancel_but",
			(XtPointer)id_eimaging_time_dlg_cancel_but},
		{"id_eimaging_time_dlg_ok_but",
			(XtPointer)id_eimaging_time_dlg_ok_but},
		{"close_eimaging_time_dialog",
		 (XtPointer)close_eimaging_time_dialog},
		{"identify_eimaging_period_menu",
		 (XtPointer)identify_eimaging_period_menu},
		{"identify_eimaging_period_scroll",
		 (XtPointer)identify_eimaging_period_scroll},
		{"identify_eimaging_period_label",
		 (XtPointer)identify_eimaging_period_label},
		{"identify_eimaging_period_value",
		 (XtPointer)identify_eimaging_period_value},
		{"id_eimaging_event_menu",
			 (XtPointer)id_eimaging_event_menu},
		{"id_eimaging_event_choice",
			 (XtPointer)id_eimaging_event_choice},
		{"id_eimaging_event_pulldn",
			 (XtPointer)id_eimaging_event_pulldn},
		{"id_eimaging_clear_butt",
			 (XtPointer)id_eimaging_clear_butt},
		{"id_eimaging_current_butt",
			 (XtPointer)id_eimaging_current_butt},
		{"id_eimaging_p_start_butt",
			 (XtPointer)id_eimaging_p_start_butt},
		{"id_eimaging_p_pt_butt",
			 (XtPointer)id_eimaging_p_pt_butt},
		{"id_eimaging_p_end_butt",
			 (XtPointer)id_eimaging_p_end_butt},
		{"id_eimaging_qrs_start_butt",
			 (XtPointer)id_eimaging_qrs_start_butt},
		{"id_eimaging_qrs_pt_butt",
			 (XtPointer)id_eimaging_qrs_pt_butt},
		{"id_eimaging_qrs_end_butt",
			 (XtPointer)id_eimaging_qrs_end_butt},
		{"id_eimaging_t_start_butt",
			 (XtPointer)id_eimaging_t_start_butt},
		{"id_eimaging_t_pt_butt",
			 (XtPointer)id_eimaging_t_pt_butt},
		{"id_eimaging_t_end_butt",
			 (XtPointer)id_eimaging_t_end_butt},		
		{"alter_eimaging_event_info",
		 (XtPointer)alter_eimaging_event_info}};
	static MrmRegisterArg identifier_list[]=
	{
		{"eimaging_time_dialog_structure",(XtPointer)NULL}
	};	

	static XtResource resources_1[]=
	{
		{		
			XmNeimagingTimeMaxPeriod,
			XmCEimagingTimeMaxPeriod,	
			XmRInt,
			sizeof(int),
			XtOffsetOf(Electrical_imaging_time_dialog_settings,max_period),
			XmRString,
			"500"		
		}
	};

	ENTER(create_Electrical_imaging_time_dialog);
	/* check arguments */
	if (user_interface)
	{
		no_cascade_pixmap=user_interface->no_cascade_pixmap;
		if (MrmOpenHierarchy_base64_string(eimaging_time_dialog_uidh,
			&eimaging_time_dialog_hierarchy,&eimaging_time_dialog_hierarchy_open))
		{
			if (ALLOCATE(eimaging_time_dialog,struct Electrical_imaging_time_dialog,1))
			{				
				eimaging_time_dialog->settings_changed=0;
				/*widget stuff*/
				eimaging_time_dialog->user_interface=user_interface;
				eimaging_time_dialog->activation=activation;			
				eimaging_time_dialog->dialog=(Widget)NULL;
				eimaging_time_dialog->shell=(Widget)NULL;
				eimaging_time_dialog->ok_button=(Widget)NULL;
				eimaging_time_dialog->apply_button=(Widget)NULL;
				eimaging_time_dialog->cancel_button=(Widget)NULL;
				eimaging_time_dialog->period_menu=(Widget)NULL;
				eimaging_time_dialog->period_label=(Widget)NULL;
				eimaging_time_dialog->period_scroll=(Widget)NULL;
				eimaging_time_dialog->period_value=(Widget)NULL;
				eimaging_time_dialog->event_menu=(Widget)NULL;			
				eimaging_time_dialog->event_choice=(Widget)NULL;
				eimaging_time_dialog->event_pull_down=(Widget)NULL;
				eimaging_time_dialog->clear_button=(Widget)NULL;
				eimaging_time_dialog->current_button=(Widget)NULL;
				eimaging_time_dialog->p_wave_start_button=(Widget)NULL;
				eimaging_time_dialog->p_wave_peak_or_trough_button=(Widget)NULL;	
				eimaging_time_dialog->p_wave_end_button=(Widget)NULL;
				eimaging_time_dialog->qrs_wave_start_button=(Widget)NULL;
				eimaging_time_dialog->qrs_wave_peak_or_trough_button=(Widget)NULL;	
				eimaging_time_dialog->qrs_wave_end_button=(Widget)NULL;
				eimaging_time_dialog->t_wave_start_button=(Widget)NULL;
				eimaging_time_dialog->t_wave_peak_or_trough_button=(Widget)NULL;	
				eimaging_time_dialog->t_wave_end_button=(Widget)NULL;
				eimaging_time_dialog->shell_list_item=(struct Shell_list_item *)NULL;					
				eimaging_time_dialog->address=eimaging_time_dialog_address;			
				/* create the dialog shell */
				if (!(parent=activation)||(True!=XtIsWidget(parent)))
				{
					parent=user_interface->application_shell;
				}
				if (eimaging_time_dialog->shell=XtVaCreatePopupShell(
					"eimaging_time_dialog_shell",
					xmDialogShellWidgetClass,parent,
					XmNdeleteResponse,XmDO_NOTHING,
					XmNmwmDecorations,MWM_DECOR_ALL|MWM_DECOR_RESIZEH,
					XmNmwmFunctions,MWM_FUNC_MOVE|MWM_FUNC_CLOSE,
					XmNtitle,"Electrical Imaging Event Settings",
					NULL))
				{
					eimaging_time_dialog->shell_list_item=
						create_Shell_list_item(&(eimaging_time_dialog->shell),user_interface);
					/* Set up window manager callback for close window message */
					WM_DELETE_WINDOW=XmInternAtom(
						XtDisplay(eimaging_time_dialog->shell),"WM_DELETE_WINDOW",False);
					XmAddWMProtocolCallback(eimaging_time_dialog->shell,
						WM_DELETE_WINDOW,close_eimaging_time_dialog,eimaging_time_dialog);
					/* add the destroy callback */
					XtAddCallback(eimaging_time_dialog->shell,XmNdestroyCallback,
						destroy_eimaging_time_dialog,(XtPointer)eimaging_time_dialog);
					/* register the other callbacks */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(eimaging_time_dialog_hierarchy,
						callback_list,XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value=(XtPointer)eimaging_time_dialog;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(eimaging_time_dialog_hierarchy,
							identifier_list,XtNumber(identifier_list)))
						{
							/* fetch the dialog widget */
							if (MrmSUCCESS==MrmFetchWidget(eimaging_time_dialog_hierarchy,
								"eimaging_time_dialog",
								eimaging_time_dialog->shell,&(eimaging_time_dialog->dialog),
								&eimaging_time_dialog_class))
							{

							child_widget=
								XmOptionButtonGadget(eimaging_time_dialog->event_choice);
							XtVaSetValues(child_widget,
								XmNshadowThickness,0,
								XmNhighlightThickness,0,
								XmNcascadePixmap,no_cascade_pixmap,
								XmNalignment,XmALIGNMENT_BEGINNING,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);		
								XtManageChild(eimaging_time_dialog->dialog);
								XtRealizeWidget(eimaging_time_dialog->shell);
								if (eimaging_time_dialog_address)
								{
									*eimaging_time_dialog_address=eimaging_time_dialog;
								}

								/* retrieve the settings (max_period) */
								XtVaGetApplicationResources(user_interface->application_shell,
									eimaging_time_dialog,resources_1,XtNumber(resources_1),NULL);
								XtVaGetValues(eimaging_time_dialog->period_scroll,
									XmNminimum,&scroll_min,NULL);
								XtVaGetValues(eimaging_time_dialog->period_scroll,
									XmNmaximum,&scroll_max,NULL);
								XtVaGetValues(eimaging_time_dialog->period_scroll,
									XmNsliderSize,&scroll_silder_size,NULL);
								eimaging_time_dialog->scroll_width=scroll_max-scroll_min-scroll_silder_size;
								eimaging_time_dialog->marker_period=10; 							
								sprintf(value_string,"%g",eimaging_time_dialog->marker_period);
								XtVaSetValues(eimaging_time_dialog->period_value,XmNvalue,
									value_string,NULL);								
								XtVaSetValues(eimaging_time_dialog->period_scroll,XmNvalue,
									(int)(eimaging_time_dialog->marker_period*
										eimaging_time_dialog->scroll_width/
										eimaging_time_dialog->max_period),NULL);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"create_Electrical_imaging_time_dialog. "
									"Could not fetch the dialog widget");
								XtDestroyWidget(eimaging_time_dialog->shell);
								eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)NULL;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_Electrical_imaging_time_dialog.  Could not register identifiers");
							XtDestroyWidget(eimaging_time_dialog->shell);
							eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)NULL;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_Electrical_imaging_time_dialog.  Could not register callbacks");
						XtDestroyWidget(eimaging_time_dialog->shell);
						eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)NULL;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Electrical_imaging_time_dialog.  Could not create dialog shell");
					DEALLOCATE(eimaging_time_dialog);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_Electrical_imaging_time_dialog.  Could not allocate memory");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Electrical_imaging_time_dialog.  Could not open hierarchy");
			eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Electrical_imaging_time_dialog. Missing user_interface");
		eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)NULL;
	}
	LEAVE;
	return (eimaging_time_dialog);
} /* create_Electrical_imaging_time_dialog */

int open_eimaging_time_dialog(
	struct Electrical_imaging_time_dialog *eimaging_time_dialog)
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
Opens the <eimaging_time_dialog>.
==============================================================================*/
{	
	int return_code;

	ENTER(open_eimaging_time_dialog);
	if (eimaging_time_dialog)
	{					
		/* ghost the activation button */
		XtSetSensitive(eimaging_time_dialog->activation,FALSE);		
		/* pop up the map dialog */
		busy_cursor_on(eimaging_time_dialog->shell,eimaging_time_dialog->user_interface);
		XtManageChild(eimaging_time_dialog->dialog);
		return_code=1;
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"open_eimaging_time_dialog. Missing eimaging_time_dialog");
	}
	LEAVE;

	return (return_code);
} /* open_eimaging_time_dialog */



