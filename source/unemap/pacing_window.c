/*******************************************************************************
FILE : pacing_window.c

LAST MODIFIED : 23 May 2004

DESCRIPTION :
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#if defined (MOTIF)
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xlib.h>
#include <X11/Composite.h>
#include <Xm/Xm.h>
#include <Xm/PushBG.h>
#include <Xm/ScrollBar.h>
#include <Mrm/MrmPublic.h>
#include <Mrm/MrmDecls.h>
#endif /* defined (MOTIF) */
#include "unemap/pacing_window.h"
#if defined (MOTIF)
#include "unemap/pacing_window.uidh"
#endif /* defined (MOTIF) */
#include "general/debug.h"
#include "general/mystring.h"
#include "unemap/unemap_hardware.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module types
------------
*/
struct Pacing_window
/*******************************************************************************
LAST MODIFIED : 22 May 2004

DESCRIPTION :
The window associated with the set up button.
==============================================================================*/
{
	char constant_current_stimulation,*decrement_threshold_pairs_string,
		negative_return_signal,pacing,*pacing_electrodes,*return_electrodes;
	float basic_cycle_length,basic_cycle_length_arrow_step,control_voltcur,
		control_voltcur_arrow_step,control_width,control_width_arrow_step,
		*decrement_threshold_pairs,*pacing_voltcurs,resolution,*si_length,sn_length,
		sn_length_change,sn_length_change_save,sn_length_change_factor,
		sn_length_change_factor_save,sn_length_decrement_threshold_pairs,
		sn_s1_pause;
	int decrement_threshold_pair_number,last_decrement_threshold_pairs,
		number_of_pacing_channels,number_of_return_channels,
		number_of_stimulus_types,number_of_stimulus_types_save,*number_of_si,
		number_of_decrement_threshold_pairs,*pacing_channels,*return_channels,
		sn_increment,sn_increment_save,sn_length_count_decrement_threshold_pairs,
		stimulus_number;
	struct Pacing_window **address;
	struct Rig **rig_address;
	struct User_interface *user_interface;
#if defined (MOTIF)
	Widget activation,window,shell;
	Widget basic_cycle_length_form,basic_cycle_length_slider,
		basic_cycle_length_value;
	Widget control_voltcur_form,control_voltcur_label,control_voltcur_slider,
		control_voltcur_value;
	Widget control_width_form,control_width_slider,control_width_value;
	Widget decrement_threshold_pairs_form,decrement_threshold_pairs_value;
	Widget number_of_s_types_decrement,number_of_s_types_form,
		number_of_s_types_increment,number_of_s_types_label,number_of_s_types_value;
	Widget s_number_decrement,s_number_increment,s_number_label;
	Widget number_of_si_decrement,number_of_si_increment,number_of_si_label,
		number_of_si_value,si_form,si_length_value;
	Widget pacing_electrodes_form,pacing_electrodes_value;
	Widget return_electrodes_form,return_electrodes_value;
	Widget sn_length_form,sn_length_value;
	Widget sn_length_label;
	Widget sn_length_change_form,sn_length_change_label,sn_length_change_value;
	Widget sn_length_change_factor_form,sn_length_change_factor_label,
		sn_length_change_factor_value;
	Widget sn_length_decrement_button,sn_length_inc_dec_choice,
		sn_length_increment_button;
	Widget sn_s1_pause_form,sn_s1_pause_label,sn_s1_pause_value;
	Widget basic_cycle_length_pace_toggle,restitution_curve_pace_toggle,
		restitution_time_pace_toggle;
	Widget restitution_time_decrement_button,restitution_time_increment_button,
		restitution_time_repeat_button,restitution_time_reset_button;
#endif /* defined (MOTIF) */
}; /* struct Pacing_window */

typedef struct Pacing_window Pacing_window_settings;

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int pacing_window_hierarchy_open=0;
static MrmHierarchy pacing_window_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
static int update_sn_length_label(struct Pacing_window *pacing_window)
/*******************************************************************************
LAST MODIFIED : 26 January 2002

DESCRIPTION :
Updates the sn_length_label.
==============================================================================*/
{
	char value_string[101];
	int return_code;

	ENTER(update_sn_length_label);
	return_code=0;
	if (pacing_window)
	{
		sprintf(value_string,"S%d length (ms): %g",
			pacing_window->number_of_stimulus_types,pacing_window->sn_length);
#if defined (MOTIF)
		XtVaSetValues(pacing_window->sn_length_label,
			XmNlabelString,XmStringCreate(value_string,XmSTRING_DEFAULT_CHARSET),
			NULL);
#endif /* defined (MOTIF) */
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* update_sn_length_label */

static int update_sn_length_change_label(struct Pacing_window *pacing_window)
/*******************************************************************************
LAST MODIFIED : 26 January 2002

DESCRIPTION :
Updates the sn_length_change_label.
==============================================================================*/
{
	char value_string[101];
	int return_code;

	ENTER(update_sn_length_change_label);
	return_code=0;
	if (pacing_window)
	{
		sprintf(value_string,"S%d length",pacing_window->number_of_stimulus_types);
#if defined (MOTIF)
		XtVaSetValues(pacing_window->sn_length_change_label,
			XmNlabelString,XmStringCreate(value_string,XmSTRING_DEFAULT_CHARSET),
			NULL);
#endif /* defined (MOTIF) */
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* update_sn_length_change_label */

static int update_sn_length_change_value(struct Pacing_window *pacing_window)
/*******************************************************************************
LAST MODIFIED : 26 January 2002

DESCRIPTION :
Updates the sn_length_change_value.
==============================================================================*/
{
	char value_string[101];
	int return_code;

	ENTER(update_sn_length_change_value);
	return_code=0;
	if (pacing_window)
	{
		sprintf(value_string,"%g",pacing_window->sn_length_change);
#if defined (MOTIF)
		XtVaSetValues(pacing_window->sn_length_change_value,
			XmNcursorPosition,strlen(value_string),
			XmNvalue,value_string,
			NULL);
#endif /* defined (MOTIF) */
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* update_sn_length_change_value */

static int update_sn_length_change_factor_label(
	struct Pacing_window *pacing_window)
/*******************************************************************************
LAST MODIFIED : 26 January 2002

DESCRIPTION :
Updates the sn_length_change_factor_label.
==============================================================================*/
{
	char value_string[101];
	int return_code;

	ENTER(update_sn_length_change_factor_label);
	return_code=0;
	if (pacing_window)
	{
		sprintf(value_string,"S%d length change factor",
			pacing_window->number_of_stimulus_types);
#if defined (MOTIF)
		XtVaSetValues(pacing_window->sn_length_change_factor_label,
			XmNlabelString,XmStringCreate(value_string,XmSTRING_DEFAULT_CHARSET),
			NULL);
#endif /* defined (MOTIF) */
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* update_sn_length_change_factor_label */

static int update_sn_length_change_factor_value(
	struct Pacing_window *pacing_window)
/*******************************************************************************
LAST MODIFIED : 26 January 2002

DESCRIPTION :
Updates the sn_length_change_factor_value.
==============================================================================*/
{
	char value_string[101];
	int return_code;

	ENTER(update_sn_length_change_factor_value);
	return_code=0;
	if (pacing_window)
	{
		sprintf(value_string,"%g",pacing_window->sn_length_change_factor);
#if defined (MOTIF)
		XtVaSetValues(pacing_window->sn_length_change_factor_value,
			XmNcursorPosition,strlen(value_string),
			XmNvalue,value_string,
			NULL);
#endif /* defined (MOTIF) */
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* update_sn_length_change_factor_value */

static int change_si_length(struct Pacing_window *pacing_window)
/*******************************************************************************
LAST MODIFIED : 15 August 2003

DESCRIPTION :
Updates after a change to the cycle length for a stimulus type.
==============================================================================*/
{
	char value_string[21];
	float si_length;
	int return_code;

	ENTER(change_si_length);
	return_code=0;
	/* check arguments */
	if (pacing_window)
	{
		/* must be a multiple of the resolution and longer than the control width */
		si_length=(pacing_window->si_length)[(pacing_window->stimulus_number)-1];
		if (si_length<(pacing_window->control_width)+(pacing_window->resolution))
		{
			si_length=(pacing_window->control_width)+(pacing_window->resolution);
		}
		else
		{
			si_length=(pacing_window->resolution)*
				(float)floor(si_length/(pacing_window->resolution)+0.5);
		}
		(pacing_window->si_length)[(pacing_window->stimulus_number)-1]=si_length;
		sprintf(value_string,"%g",si_length);
#if defined (MOTIF)
		XtVaSetValues(pacing_window->si_length_value,
			XmNcursorPosition,strlen(value_string),
			XmNvalue,value_string,
			NULL);
#endif /* defined (MOTIF) */
		if (pacing_window->stimulus_number==pacing_window->number_of_stimulus_types)
		{
			pacing_window->sn_length=si_length;
			update_sn_length_label(pacing_window);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"change_si_length.  Missing pacing_window");
	}
	LEAVE;

	return (return_code);
} /* change_si_length */

static int change_control_width(struct Pacing_window *pacing_window,
	float control_width)
/*******************************************************************************
LAST MODIFIED : 14 October 2003

DESCRIPTION :
Called to change the control_width.
==============================================================================*/
{
	char *temp_decrement_threshold_pairs_string,value_string[101];
	float *decrement_threshold_pairs;
	int i,number_of_decrement_threshold_pairs,return_code;

	ENTER(change_control_width);
	return_code=0;
	if (pacing_window)
	{
		return_code=1;
		/* must be a multiple of the resolution */
		if (control_width<(pacing_window->resolution))
		{
			pacing_window->control_width=pacing_window->resolution;
		}
		else
		{
			pacing_window->control_width=(pacing_window->resolution)*
				(float)floor(control_width/(pacing_window->resolution)+0.5);
		}
		sprintf(value_string,"%g",pacing_window->control_width);
#if defined (MOTIF)
		XtVaSetValues(pacing_window->control_width_value,
			XmNcursorPosition,strlen(value_string),
			XmNvalue,value_string,
			NULL);
#endif /* defined (MOTIF) */
		DEALLOCATE(pacing_window->decrement_threshold_pairs_string);
		assign_empty_string(&(pacing_window->decrement_threshold_pairs_string));
		i=0;
		number_of_decrement_threshold_pairs=
			pacing_window->number_of_decrement_threshold_pairs;
		decrement_threshold_pairs=pacing_window->decrement_threshold_pairs;
		while (return_code&&(i<number_of_decrement_threshold_pairs))
		{
			/* must be a multiple of the resolution and longer than the control
				width */
			if (decrement_threshold_pairs[2*i+1]<(pacing_window->control_width)+
				(pacing_window->resolution))
			{
				decrement_threshold_pairs[2*i+1]=(pacing_window->control_width)+
					(pacing_window->resolution);
			}
			else
			{
				decrement_threshold_pairs[2*i+1]=(pacing_window->resolution)*
					(float)floor(decrement_threshold_pairs[2*i+1]/
					(pacing_window->resolution)+0.5);
			}
			sprintf(value_string,"%g,%g;",decrement_threshold_pairs[2*i],
				decrement_threshold_pairs[2*i+1]);
			if (REALLOCATE(temp_decrement_threshold_pairs_string,
				pacing_window->decrement_threshold_pairs_string,char,
				strlen(pacing_window->decrement_threshold_pairs_string)+
				strlen(value_string)+1))
			{
				strcat(temp_decrement_threshold_pairs_string,value_string);
				pacing_window->decrement_threshold_pairs_string=
					temp_decrement_threshold_pairs_string;
				i++;
			}
			else
			{
				return_code=0;
			}
		}
#if defined (MOTIF)
		XtVaSetValues(pacing_window->decrement_threshold_pairs_value,
			XmNcursorPosition,strlen(pacing_window->decrement_threshold_pairs_string),
			XmNvalue,pacing_window->decrement_threshold_pairs_string,
			NULL);
#endif /* defined (MOTIF) */
		/* must be a multiple of the resolution and longer than the control width */
		if (pacing_window->basic_cycle_length<(pacing_window->control_width)+
			(pacing_window->resolution))
		{
			pacing_window->basic_cycle_length=(pacing_window->control_width)+
				(pacing_window->resolution);
		}
		else
		{
			pacing_window->basic_cycle_length=(pacing_window->resolution)*
				(float)floor((pacing_window->basic_cycle_length)/
				(pacing_window->resolution)+0.5);
		}
		sprintf(value_string,"%g",pacing_window->basic_cycle_length);
#if defined (MOTIF)
		XtVaSetValues(pacing_window->basic_cycle_length_value,
			XmNcursorPosition,strlen(value_string),
			XmNvalue,value_string,
			NULL);
#endif /* defined (MOTIF) */
		for (i=0;i<pacing_window->number_of_stimulus_types;i++)
		{
			/* must be a multiple of the resolution and longer than the control
				width */
			if ((pacing_window->si_length)[i]<(pacing_window->control_width)+
				(pacing_window->resolution))
			{
				(pacing_window->si_length)[i]=(pacing_window->control_width)+
					(pacing_window->resolution);
			}
			else
			{
				(pacing_window->si_length)[i]=(pacing_window->resolution)*
					(float)floor((pacing_window->si_length)[i]/
					(pacing_window->resolution)+0.5);
			}
		}
		change_si_length(pacing_window);
		/* must be a multiple of the resolution and longer than the control width */
		if (pacing_window->sn_length<(pacing_window->control_width)+
			(pacing_window->resolution))
		{
			pacing_window->sn_length=(pacing_window->control_width)+
				(pacing_window->resolution);
		}
		else
		{
			pacing_window->sn_length=(pacing_window->resolution)*
				(float)floor((pacing_window->sn_length)/
				(pacing_window->resolution)+0.5);
		}
		update_sn_length_label(pacing_window);
		/* must be a multiple of the resolution and longer than the control width */
		if (pacing_window->sn_s1_pause<(pacing_window->control_width)+
			(pacing_window->resolution))
		{
			pacing_window->sn_s1_pause=(pacing_window->control_width)+
				(pacing_window->resolution);
		}
		else
		{
			pacing_window->sn_s1_pause=(pacing_window->resolution)*
				(float)floor((pacing_window->sn_s1_pause)/
				(pacing_window->resolution)+0.5);
		}
		sprintf(value_string,"%g",pacing_window->sn_s1_pause);
#if defined (MOTIF)
		XtVaSetValues(pacing_window->sn_s1_pause_value,
			XmNcursorPosition,strlen(value_string),
			XmNvalue,value_string,
			NULL);
#endif /* defined (MOTIF) */
	}
	LEAVE;

	return (return_code);
} /* change_control_width */

static int set_channels_from_string(char *electrodes,struct Rig *rig,
	int number_of_restricted_channels,int *restricted_channels,
	char **electrodes_result_address,int *number_of_channels_address,
	int **channels_address)
/*******************************************************************************
LAST MODIFIED : 11 November 2001

DESCRIPTION :
Parses the <electrodes> string to get the channels for the <rig>.
==============================================================================*/
{
	char *electrodes_result,*electrodes_result_temp;
	int channel_number,*channels,*channels_temp,electrodes_result_length,found,i,
		j,number_of_channels,number_of_characters,number_of_devices,
		number_of_stimulators,position,*restricted_stimulators,return_code;
	struct Device **device;

	ENTER(set_channels_from_string);
	return_code=0;
	/* check arguments */
	if (electrodes&&rig&&electrodes_result_address&&number_of_channels_address&&
		channels_address)
	{
		return_code=1;
		DEALLOCATE(*electrodes_result_address);
		DEALLOCATE(*channels_address);
		*number_of_channels_address=0;
		if ((0<number_of_restricted_channels)&&restricted_channels&&
			unemap_get_number_of_stimulators(&number_of_stimulators)&&
			(0<number_of_stimulators)&&ALLOCATE(restricted_stimulators,int,
			number_of_stimulators))
		{
			for (i=0;i<number_of_stimulators;i++)
			{
				restricted_stimulators[i]=0;
			}
			for (i=0;i<number_of_restricted_channels;i++)
			{
				j=number_of_stimulators;
				while ((j>0)&&
					!unemap_channel_valid_for_stimulator(j,restricted_channels[i]))
				{
					j--;
				}
				if (j>0)
				{
					restricted_stimulators[j-1]=1;
				}
			}
		}
		else
		{
			number_of_stimulators=0;
			restricted_stimulators=(int *)NULL;
		}
		channels=(int *)NULL;
		number_of_channels=0;
		electrodes_result=(char *)NULL;
		electrodes_result_length=0;
		position=0;
		number_of_characters=0;
		sscanf(electrodes+position,"%*[ ,]%n",&number_of_characters);
		if (0<number_of_characters)
		{
			position=number_of_characters;
		}
		while (return_code&&electrodes[position])
		{
			number_of_characters=0;
			sscanf(electrodes+position,"%*[^ ,]%n",&number_of_characters);
			if (0<number_of_characters)
			{
				device=rig->devices;
				number_of_devices=rig->number_of_devices;
				found=0;
				while (!found&&(0<number_of_devices))
				{
					if ((ELECTRODE==(*device)->description->type)&&
						(number_of_characters==(int)strlen((*device)->description->name))&&
						(0==strncmp(electrodes+position,(*device)->description->name,
						number_of_characters)))
					{
						found=1;
					}
					else
					{
						device++;
						number_of_devices--;
					}
				}
				if (found)
				{
					/* check that don't already have channel */
					i=0;
					while ((i<number_of_channels)&&(channels[i]!=
						(*device)->channel->number))
					{
						i++;
					}
					if (i>=number_of_channels)
					{
						channel_number=(*device)->channel->number;
						/* check that not restricted */
						if (restricted_stimulators)
						{
							i=0;
							while ((i<number_of_stimulators)&&!(restricted_stimulators[i]&&
								unemap_channel_valid_for_stimulator(i+1,channel_number)))
							{
								i++;
							}
						}
						else
						{
							i=0;
						}
						if (i==number_of_stimulators)
						{
							if (REALLOCATE(channels_temp,channels,int,number_of_channels+1)&&
								REALLOCATE(electrodes_result_temp,electrodes_result,char,
								electrodes_result_length+number_of_characters+2))
							{
								channels=channels_temp;
								channels[number_of_channels]=channel_number;
								number_of_channels++;
								electrodes_result=electrodes_result_temp;
								if (number_of_channels>1)
								{
									electrodes_result[electrodes_result_length]=',';
									electrodes_result_length++;
								}
								strncpy(electrodes_result+electrodes_result_length,
									electrodes+position,number_of_characters);
								electrodes_result_length += number_of_characters;
								electrodes_result[electrodes_result_length]='\0';
							}
							else
							{
								if (channels_temp)
								{
									channels=channels_temp;
								}
								return_code=0;
							}
						}
					}
				}
				position += number_of_characters;
				number_of_characters=0;
				sscanf(electrodes+position,"%*[ ,]%n",&number_of_characters);
				if (0<number_of_characters)
				{
					position += number_of_characters;
				}
			}
			else
			{
				return_code=0;
			}
		}
		DEALLOCATE(restricted_stimulators);
		*number_of_channels_address=number_of_channels;
		*channels_address=channels;
		*electrodes_result_address=electrodes_result;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_channels_from_string.  Invalid argument(s).  %p %p %p %p %p",
			electrodes,rig,electrodes_result_address,number_of_channels_address,
			channels_address);
	}
	LEAVE;

	return (return_code);
} /* set_channels_from_string */

static int set_pacing_channels_from_string(struct Pacing_window *pacing_window,
	char *pacing_electrodes)
/*******************************************************************************
LAST MODIFIED : 11 November 2001

DESCRIPTION :
Parses the <pacing_electrodes> string to get the pacing channels for the
<pacing_window>.
==============================================================================*/
{
	int return_code;

	ENTER(set_pacing_channels_from_string);
	return_code=0;
	/* check arguments */
	if (pacing_window&&pacing_electrodes&&(pacing_window->rig_address))
	{
		return_code=set_channels_from_string(pacing_electrodes,
			*(pacing_window->rig_address),pacing_window->number_of_return_channels,
			pacing_window->return_channels,&(pacing_window->pacing_electrodes),
			&(pacing_window->number_of_pacing_channels),
			&(pacing_window->pacing_channels));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_pacing_channels_from_string.  Invalid argument(s).  %p %p",
			pacing_window,pacing_electrodes);
	}
	LEAVE;

	return (return_code);
} /* set_pacing_channels_from_string */

static int set_return_channels_from_string(struct Pacing_window *pacing_window,
	char *return_electrodes)
/*******************************************************************************
LAST MODIFIED : 11 November 2001

DESCRIPTION :
Parses the <return_electrodes> string to get the return channels for the
<pacing_window>.
==============================================================================*/
{
	int return_code;

	ENTER(set_return_channels_from_string);
	return_code=0;
	/* check arguments */
	if (pacing_window&&return_electrodes&&(pacing_window->rig_address))
	{
		return_code=set_channels_from_string(return_electrodes,
			*(pacing_window->rig_address),pacing_window->number_of_pacing_channels,
			pacing_window->pacing_channels,&(pacing_window->return_electrodes),
			&(pacing_window->number_of_return_channels),
			&(pacing_window->return_channels));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_return_channels_from_string.  Invalid argument(s).  %p %p",
			pacing_window,return_electrodes);
	}
	LEAVE;

	return (return_code);
} /* set_return_channels_from_string */

static int set_numbers_of_stimuli_from_string(
	struct Pacing_window *pacing_window,char *numbers_of_stimuli)
/*******************************************************************************
LAST MODIFIED : 25 May 2003

DESCRIPTION :
Parses the <numbers_of_stimuli> string to get the values for the
<pacing_window>.
==============================================================================*/
{
	float *si_length;
	int i,number_of_characters,*number_of_si,position,return_code,stimulus_number;

	ENTER(set_numbers_of_stimuli_from_string);
	return_code=0;
	/* check arguments */
	if (pacing_window&&(1<=pacing_window->number_of_stimulus_types)&&
		numbers_of_stimuli)
	{
		ALLOCATE(number_of_si,int,pacing_window->number_of_stimulus_types);
		ALLOCATE(si_length,float,pacing_window->number_of_stimulus_types);
		if (number_of_si&&si_length)
		{
			DEALLOCATE(pacing_window->number_of_si);
			pacing_window->number_of_si=number_of_si;
			if (pacing_window->number_of_stimulus_types<
				pacing_window->number_of_stimulus_types_save)
			{
				for (i=0;i<pacing_window->number_of_stimulus_types;i++)
				{
					si_length[i]=(pacing_window->si_length)[i];
				}
			}
			else
			{
				for (i=0;i<pacing_window->number_of_stimulus_types_save;i++)
				{
					si_length[i]=(pacing_window->si_length)[i];
				}
				for (i=pacing_window->number_of_stimulus_types_save;
					i<pacing_window->number_of_stimulus_types;i++)
				{
					si_length[i]=si_length[i-1];
				}
			}
			DEALLOCATE(pacing_window->si_length);
			pacing_window->si_length=si_length;
			pacing_window->number_of_stimulus_types_save=
				pacing_window->number_of_stimulus_types;
			stimulus_number=0;
			position=0;
			number_of_characters=0;
			sscanf(numbers_of_stimuli+position,"%*[ ,]%n",&number_of_characters);
			position += number_of_characters;
			return_code=1;
			while (return_code&&numbers_of_stimuli[position]&&
				(stimulus_number<pacing_window->number_of_stimulus_types))
			{
				if (1==sscanf(numbers_of_stimuli+position,"%d%n",
					number_of_si+stimulus_number,&number_of_characters))
				{
					position += number_of_characters;
					stimulus_number++;
					sscanf(numbers_of_stimuli+position,"%*[ ,]%n",&number_of_characters);
					position += number_of_characters;
				}
				else
				{
					return_code=0;
				}
			}
			if (0==stimulus_number)
			{
				number_of_si[stimulus_number]=9;
				stimulus_number++;
			}
			while (stimulus_number<pacing_window->number_of_stimulus_types)
			{
				number_of_si[stimulus_number]=number_of_si[stimulus_number-1];
				stimulus_number++;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_numbers_of_stimuli_from_string.  "
				"Could not ALLOCATE number_of_si (%p) or si_length (%p)",number_of_si,
				si_length);
			DEALLOCATE(number_of_si);
			DEALLOCATE(si_length);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_numbers_of_stimuli_from_string.  Invalid argument(s).  %p %p",
			pacing_window,numbers_of_stimuli);
	}
	LEAVE;

	return (return_code);
} /* set_numbers_of_stimuli_from_string */

static int set_stimuli_lengths_from_string(
	struct Pacing_window *pacing_window,char *stimuli_lengths)
/*******************************************************************************
LAST MODIFIED : 15 August 2003

DESCRIPTION :
Parses the <stimuli_lengths> string to get the values for the
<pacing_window>.
==============================================================================*/
{
	float *si_length;
	int i,number_of_characters,*number_of_si,position,return_code,stimulus_number;

	ENTER(set_stimuli_lengths_from_string);
	return_code=0;
	/* check arguments */
	if (pacing_window&&(1<=pacing_window->number_of_stimulus_types)&&
		stimuli_lengths)
	{
		ALLOCATE(number_of_si,int,pacing_window->number_of_stimulus_types);
		ALLOCATE(si_length,float,pacing_window->number_of_stimulus_types);
		if (number_of_si&&si_length)
		{
			if (pacing_window->number_of_stimulus_types<
				pacing_window->number_of_stimulus_types_save)
			{
				for (i=0;i<pacing_window->number_of_stimulus_types;i++)
				{
					number_of_si[i]=(pacing_window->number_of_si)[i];
				}
			}
			else
			{
				for (i=0;i<pacing_window->number_of_stimulus_types_save;i++)
				{
					number_of_si[i]=(pacing_window->number_of_si)[i];
				}
				for (i=pacing_window->number_of_stimulus_types_save;
					i<pacing_window->number_of_stimulus_types;i++)
				{
					number_of_si[i]=number_of_si[i-1];
				}
			}
			DEALLOCATE(pacing_window->number_of_si);
			pacing_window->number_of_si=number_of_si;
			DEALLOCATE(pacing_window->si_length);
			pacing_window->si_length=si_length;
			pacing_window->number_of_stimulus_types_save=
				pacing_window->number_of_stimulus_types;
			stimulus_number=0;
			position=0;
			number_of_characters=0;
			sscanf(stimuli_lengths+position,"%*[ ,]%n",&number_of_characters);
			position += number_of_characters;
			return_code=1;
			while (return_code&&stimuli_lengths[position]&&
				(stimulus_number<pacing_window->number_of_stimulus_types))
			{
				if (1==sscanf(stimuli_lengths+position,"%f%n",
					si_length+stimulus_number,&number_of_characters))
				{
					/* must be a multiple of the resolution and longer than the control
						width */
					if (si_length[stimulus_number]<(pacing_window->control_width)+
						(pacing_window->resolution))
					{
						si_length[stimulus_number]=(pacing_window->control_width)+
							(pacing_window->resolution);
					}
					else
					{
						si_length[stimulus_number]=(pacing_window->resolution)*
							(float)floor(si_length[stimulus_number]/
							(pacing_window->resolution)+0.5);
					}
					position += number_of_characters;
					stimulus_number++;
					sscanf(stimuli_lengths+position,"%*[ ,]%n",&number_of_characters);
					position += number_of_characters;
				}
				else
				{
					return_code=0;
				}
			}
			if (0==stimulus_number)
			{
				si_length[stimulus_number]=500;
				stimulus_number++;
			}
			while (stimulus_number<pacing_window->number_of_stimulus_types)
			{
				si_length[stimulus_number]=si_length[stimulus_number-1];
				stimulus_number++;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_stimuli_lengths_from_string.  "
				"Could not ALLOCATE number_of_si (%p) or si_length (%p)",number_of_si,
				si_length);
			DEALLOCATE(number_of_si);
			DEALLOCATE(si_length);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_stimuli_lengths_from_string.  Invalid argument(s).  %p %p",
			pacing_window,stimuli_lengths);
	}
	LEAVE;

	return (return_code);
} /* set_stimuli_lengths_from_string */

static int set_decrement_threshold_pairs_from_string(
	struct Pacing_window *pacing_window,char *decrement_threshold_pairs_string)
/*******************************************************************************
LAST MODIFIED : 15 August 2003

DESCRIPTION :
Parses the <decrement_threshold_pairs_string> to get the steps to take for the
restitution curve protocol.
==============================================================================*/
{
	char *temp_decrement_threshold_pairs_string,value_string[101];
	float decrement,*decrement_threshold_pairs,*temp_decrement_threshold_pairs,
		threshold;
	int i,j,number_of_characters,number_of_decrement_threshold_pairs,position,
		return_code;

	ENTER(set_decrement_threshold_pairs_from_string);
	return_code=0;
	/* check arguments */
	if (pacing_window&&decrement_threshold_pairs_string)
	{
		return_code=1;
		DEALLOCATE(pacing_window->decrement_threshold_pairs_string);
		assign_empty_string(&(pacing_window->decrement_threshold_pairs_string));
		DEALLOCATE(pacing_window->decrement_threshold_pairs);
		pacing_window->number_of_decrement_threshold_pairs=0;
		number_of_decrement_threshold_pairs=0;
		decrement_threshold_pairs=(float *)NULL;
		position=0;
		/* skip separators */
		number_of_characters=0;
		sscanf(decrement_threshold_pairs_string+position,"%*[ ,;()]%n",
			&number_of_characters);
		if (0<number_of_characters)
		{
			position=number_of_characters;
		}
		while (return_code&&decrement_threshold_pairs_string[position])
		{
			/* read decrement */
			number_of_characters=0;
			sscanf(decrement_threshold_pairs_string+position,"%*[^ ,;()]%n",
				&number_of_characters);
			if ((0<number_of_characters)&&
				(1==sscanf(decrement_threshold_pairs_string+position,"%f",&decrement)))
			{
				position += number_of_characters;
				/* must be a multiple of the resolution */
				if (decrement<pacing_window->resolution)
				{
					decrement=pacing_window->resolution;
				}
				else
				{
					decrement=(pacing_window->resolution)*
						(float)floor(decrement/(pacing_window->resolution)+0.5);
				}
				/* skip separators */
				number_of_characters=0;
				sscanf(decrement_threshold_pairs_string+position,"%*[ ,;()]%n",
					&number_of_characters);
				if (0<number_of_characters)
				{
					position += number_of_characters;
				}
				if (decrement_threshold_pairs_string[position])
				{
					/* read threshold */
					number_of_characters=0;
					sscanf(decrement_threshold_pairs_string+position,"%*[^ ,;()]%n",
						&number_of_characters);
					if ((0<number_of_characters)&&(1==sscanf(
						decrement_threshold_pairs_string+position,"%f",&threshold)))
					{
						position += number_of_characters;
						/* must be a multiple of the resolution and longer than the control
							width */
						if (threshold<(pacing_window->control_width)+
							(pacing_window->resolution))
						{
							threshold=(pacing_window->control_width)+
								(pacing_window->resolution);
						}
						else
						{
							threshold=(pacing_window->resolution)*
								(float)floor(threshold/(pacing_window->resolution)+0.5);
						}
						/* add to array */
						i=0;
						while ((i<number_of_decrement_threshold_pairs)&&
							(decrement_threshold_pairs[2*i+1]>threshold))
						{
							i++;
						}
						if ((i<number_of_decrement_threshold_pairs)&&
							(decrement_threshold_pairs[2*i+1]==threshold))
						{
							/* threshold already in list.  Use the new decrement */
							decrement_threshold_pairs[2*i]=decrement;
						}
						else
						{
							/* new pair */
							if (REALLOCATE(temp_decrement_threshold_pairs,
								decrement_threshold_pairs,float,
								2*number_of_decrement_threshold_pairs+2))
							{
								decrement_threshold_pairs=temp_decrement_threshold_pairs;
								/* make room for the new pair */
								j=number_of_decrement_threshold_pairs;
								while (j>i)
								{
									decrement_threshold_pairs[2*j+1]=
										decrement_threshold_pairs[2*j-1];
									decrement_threshold_pairs[2*j]=
									decrement_threshold_pairs[2*j-2];
									j--;
								}
								decrement_threshold_pairs[2*i+1]=threshold;
								decrement_threshold_pairs[2*i]=decrement;
								number_of_decrement_threshold_pairs++;
							}
							else
							{
								return_code=0;
							}
						}
						/* skip separators */
						number_of_characters=0;
						sscanf(decrement_threshold_pairs_string+position,"%*[ ,;()]%n",
							&number_of_characters);
						if (0<number_of_characters)
						{
							position += number_of_characters;
						}
					}
					else
					{
						return_code=0;
					}
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
				return_code=0;
			}
		}
		return_code=1;
		i=0;
		while (return_code&&(i<number_of_decrement_threshold_pairs))
		{
			sprintf(value_string,"%g,%g;",decrement_threshold_pairs[2*i],
				decrement_threshold_pairs[2*i+1]);
			if (REALLOCATE(temp_decrement_threshold_pairs_string,
				pacing_window->decrement_threshold_pairs_string,char,
				strlen(pacing_window->decrement_threshold_pairs_string)+
				strlen(value_string)+1))
			{
				strcat(temp_decrement_threshold_pairs_string,value_string);
				pacing_window->decrement_threshold_pairs_string=
					temp_decrement_threshold_pairs_string;
				i++;
			}
			else
			{
				return_code=0;
			}
		}
		pacing_window->number_of_decrement_threshold_pairs=i;
		pacing_window->decrement_threshold_pairs=decrement_threshold_pairs;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_decrement_threshold_pairs_from_string.  Invalid argument(s).  %p %p",
			pacing_window,decrement_threshold_pairs_string);
	}
	LEAVE;

	return (return_code);
} /* set_decrement_threshold_pairs_from_string */

#if defined (MOTIF)
static void destroy_Pacing_window_callback(Widget widget,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 10 October 2001

DESCRIPTION :
Tidys up when the user destroys the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(destroy_Pacing_window_callback);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		destroy_Pacing_window(&pacing_window);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_Pacing_window_callback.  Missing pacing_window_structure");
	}
	LEAVE;
} /* destroy_Pacing_window_callback */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_electrodes_form(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 October 2001

DESCRIPTION :
Finds the id of the pacing electrodes form in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_electrodes_form);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->pacing_electrodes_form= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_electrodes_form.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_electrodes_form */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_electrodes_value(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 October 2001

DESCRIPTION :
Finds the id of the pacing electrodes value in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_electrodes_value);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->pacing_electrodes_value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_electrodes_value.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_electrodes_value */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void ch_pacing_electrodes_value(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 October 2001

DESCRIPTION :
Called when the pacing electrodes widget is changed.
==============================================================================*/
{
	char *new_value,*pacing_electrodes;
	struct Pacing_window *pacing_window;
	XmAnyCallbackStruct *text_data

	ENTER(ch_pacing_electrodes_value);
	USE_PARAMETER(widget_id);
	if ((text_data=(XmAnyCallbackStruct *)call_data)&&
		((XmCR_ACTIVATE==text_data->reason)||
		(XmCR_LOSING_FOCUS==text_data->reason)))
	{
		if (pacing_window=(struct Pacing_window *)pacing_window_structure)
		{
			if (!(pacing_window->pacing))
			{
				XtVaGetValues(pacing_window->pacing_electrodes_value,
					XmNvalue,&new_value,
					NULL);
				set_pacing_channels_from_string(pacing_window,new_value);
			}
			if (!(pacing_electrodes=pacing_window->pacing_electrodes))
			{
				pacing_electrodes="";
			}
			XtVaSetValues(pacing_window->pacing_electrodes_value,
				XmNcursorPosition,strlen(pacing_electrodes),
				XmNvalue,pacing_electrodes,
				NULL);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"ch_pacing_electrodes_value.  Missing pacing_window_structure");
		}
	}
	LEAVE;
} /* ch_pacing_electrodes_value */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_return_electrodes_for(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 11 November 2001

DESCRIPTION :
Finds the id of the return electrodes form in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_return_electrodes_for);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->return_electrodes_form= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_return_electrodes_for.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_return_electrodes_for */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_return_electrodes_val(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 11 November 2001

DESCRIPTION :
Finds the id of the return_ electrodes value in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_return_electrodes_val);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->return_electrodes_value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_return_electrodes_val.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_return_electrodes_val */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void ch_pacing_return_electrodes_val(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 11 November 2001

DESCRIPTION :
Called when the return electrodes widget is changed.
==============================================================================*/
{
	char *new_value,*return_electrodes;
	struct Pacing_window *pacing_window;
	XmAnyCallbackStruct *text_data

	ENTER(ch_pacing_return_electrodes_val);
	USE_PARAMETER(widget_id);
	if ((text_data=(XmAnyCallbackStruct *)call_data)&&
		((XmCR_ACTIVATE==text_data->reason)||
		(XmCR_LOSING_FOCUS==text_data->reason)))
	{
		if (pacing_window=(struct Pacing_window *)pacing_window_structure)
		{
			if (!(pacing_window->pacing))
			{
				XtVaGetValues(pacing_window->return_electrodes_value,
					XmNvalue,&new_value,
					NULL);
				set_return_channels_from_string(pacing_window,new_value);
			}
			if (!(return_electrodes=pacing_window->return_electrodes))
			{
				return_electrodes="";
			}
			XtVaSetValues(pacing_window->return_electrodes_value,
				XmNcursorPosition,strlen(return_electrodes),
				XmNvalue,return_electrodes,
				NULL);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"ch_pacing_return_electrodes_val.  Missing pacing_window_structure");
		}
	}
	LEAVE;
} /* ch_pacing_return_electrodes_val */
#endif /* defined (MOTIF) */

static int start_basic_cycle_length_pacing(struct Pacing_window *pacing_window)
/*******************************************************************************
LAST MODIFIED : 22 May 2004

DESCRIPTION :
Starts or re-starts the basic cycle length pacing.
==============================================================================*/
{
#if defined (MOTIF)
	Boolean status;
#endif /* defined (MOTIF) */
	float *pacing_voltcurs,voltcurs_per_second;
	int i,number_of_control_voltcurs,number_of_pacing_voltcurs,return_code;

	ENTER(start_basic_cycle_length_pacing);
	return_code=0;
	if (pacing_window)
	{
		number_of_pacing_voltcurs=(int)((pacing_window->basic_cycle_length)/
			(pacing_window->resolution)+0.5);
		if (REALLOCATE(pacing_voltcurs,pacing_window->pacing_voltcurs,float,
			number_of_pacing_voltcurs))
		{
			pacing_window->pacing_voltcurs=pacing_voltcurs;
			/* set control sensitivity */
#if defined (MOTIF)
			XtSetSensitive(pacing_window->restitution_time_pace_toggle,False);
			XtSetSensitive(pacing_window->restitution_curve_pace_toggle,False);
#endif /* defined (MOTIF) */
			/* set up pacing */
			pacing_voltcurs[0]=pacing_window->control_voltcur;
			for (i=0;i<number_of_pacing_voltcurs;i++)
			{
				pacing_voltcurs[i]=0;
			}
			number_of_control_voltcurs=(int)((pacing_window->control_width)/
				(pacing_window->resolution)+0.5);
			for (i=(number_of_pacing_voltcurs-number_of_control_voltcurs)/2;
				i<(number_of_pacing_voltcurs+number_of_control_voltcurs)/2;i++)
			{
				pacing_voltcurs[i]=pacing_window->control_voltcur;
			}
			voltcurs_per_second=(float)1000./(pacing_window->resolution);
			if (0<pacing_window->number_of_pacing_channels)
			{
				for (i=0;i<pacing_window->number_of_pacing_channels;i++)
				{
					unemap_set_channel_stimulating((pacing_window->pacing_channels)[i],
						1);
				}
				if (pacing_window->constant_current_stimulation)
				{
					unemap_load_current_stimulating(
						pacing_window->number_of_pacing_channels,
						pacing_window->pacing_channels,number_of_pacing_voltcurs,
						voltcurs_per_second,pacing_voltcurs,(unsigned int)0,
						(Unemap_stimulation_end_callback *)NULL,(void *)NULL);
				}
				else
				{
					unemap_load_voltage_stimulating(
						pacing_window->number_of_pacing_channels,
						pacing_window->pacing_channels,number_of_pacing_voltcurs,
						voltcurs_per_second,pacing_voltcurs,(unsigned int)0,
						(Unemap_stimulation_end_callback *)NULL,(void *)NULL);
				}
			}
			if (0<pacing_window->number_of_return_channels)
			{
				for (i=0;i<pacing_window->number_of_return_channels;i++)
				{
					unemap_set_channel_stimulating((pacing_window->return_channels)[i],
						1);
				}
				if (pacing_window->negative_return_signal)
				{
					for (i=0;i<number_of_pacing_voltcurs;i++)
					{
						pacing_voltcurs[i]= -pacing_voltcurs[i];
					}
					if (pacing_window->constant_current_stimulation)
					{
						unemap_load_current_stimulating(
							pacing_window->number_of_return_channels,
							pacing_window->return_channels,number_of_pacing_voltcurs,
							voltcurs_per_second,pacing_voltcurs,(unsigned int)0,
							(Unemap_stimulation_end_callback *)NULL,(void *)NULL);
					}
					else
					{
						unemap_load_voltage_stimulating(
							pacing_window->number_of_return_channels,
							pacing_window->return_channels,number_of_pacing_voltcurs,
							voltcurs_per_second,pacing_voltcurs,(unsigned int)0,
							(Unemap_stimulation_end_callback *)NULL,(void *)NULL);
					}
				}
				else
				{
					if (pacing_window->constant_current_stimulation)
					{
						unemap_load_current_stimulating(
							pacing_window->number_of_return_channels,
							pacing_window->return_channels,(int)0,(float)0,(float *)NULL,
							(unsigned int)0,(Unemap_stimulation_end_callback *)NULL,
							(void *)NULL);
					}
					else
					{
						unemap_load_voltage_stimulating(
							pacing_window->number_of_return_channels,
							pacing_window->return_channels,(int)0,(float)0,(float *)NULL,
							(unsigned int)0,(Unemap_stimulation_end_callback *)NULL,
							(void *)NULL);
					}
				}
			}
			/* start pacing */
			unemap_start_stimulating();
			pacing_window->pacing=1;
			return_code=1;
		}
		else
		{
#if defined (MOTIF)
			status=False;
			XtVaSetValues(pacing_window->basic_cycle_length_pace_toggle,
				XmNset,status,
				NULL);
#endif /* defined (MOTIF) */
		}
	}
	LEAVE;

	return (return_code);
} /* start_basic_cycle_length_pacing */

#if defined (MOTIF)
static void id_pacing_control_voltcur_form(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 October 2001

DESCRIPTION :
Finds the id of the control voltage/current form in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_control_voltcur_form);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->control_voltcur_form= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_control_voltcur_form.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_control_voltcur_form */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_control_voltcur_label(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 May 2004

DESCRIPTION :
Finds the id of the control voltage/current label in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_control_voltcur_label);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->control_voltcur_label= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_control_voltcur_label.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_control_voltcur_label */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_control_voltcur_value(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 October 2001

DESCRIPTION :
Finds the id of the control voltage/current value in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_control_voltcur_value);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->control_voltcur_value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_control_voltcur_value.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_control_voltcur_value */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void ch_pacing_control_voltcur_value(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2004

DESCRIPTION :
Called when the control voltage/current widget is changed.
==============================================================================*/
{
	Boolean status;
	char *new_value,value_string[20];
	float control_voltcur;
	struct Pacing_window *pacing_window;
	XmAnyCallbackStruct *text_data

	ENTER(ch_pacing_control_voltcur_value);
	USE_PARAMETER(widget_id);
	if ((text_data=(XmAnyCallbackStruct *)call_data)&&
		((XmCR_ACTIVATE==text_data->reason)||
		(XmCR_LOSING_FOCUS==text_data->reason)))
	{
		if (pacing_window=(struct Pacing_window *)pacing_window_structure)
		{
			XtVaGetValues(pacing_window->basic_cycle_length_pace_toggle,
				XmNset,&status,
				NULL);
			if (!(pacing_window->pacing)||(True==status))
			{
				XtVaGetValues(pacing_window->control_voltcur_value,
					XmNvalue,&new_value,
					NULL);
				if ((1==sscanf(new_value,"%f",&control_voltcur))&&(0<control_voltcur))
				{
					if (pacing_window->constant_current_stimulation)
					{
						if (control_voltcur>1)
						{
							control_voltcur=1;
						}
					}
					pacing_window->control_voltcur=control_voltcur;
					if (True==status)
					{
						unemap_stop_stimulating(0);
						start_basic_cycle_length_pacing(pacing_window);
					}
				}
			}
			sprintf(value_string,"%g",pacing_window->control_voltcur);
			XtVaSetValues(pacing_window->control_voltcur_value,
				XmNcursorPosition,strlen(value_string),
				XmNvalue,value_string,
				NULL);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"ch_pacing_control_voltcur_value.  Missing pacing_window_structure");
		}
	}
	LEAVE;
} /* ch_pacing_control_voltcur_value */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_control_voltcur_slide(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 26 January 2003

DESCRIPTION :
Sets the id of the control voltage/current slider in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_control_voltcur_slide);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->control_voltcur_slider= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_control_voltcur_slide.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_control_voltcur_slide */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void ch_pacing_control_voltcur_slide(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2004

DESCRIPTION :
Called when the control voltage/current slider is changed.
==============================================================================*/
{
	Boolean status;
	char value_string[20];
	float control_voltcur;
	int slider_maximum,slider_minumum,slider_size;
	struct Pacing_window *pacing_window;
	XmScrollBarCallbackStruct *scroll_data;

	ENTER(ch_pacing_control_voltcur_slide);
	USE_PARAMETER(widget_id);
	if ((scroll_data=(XmScrollBarCallbackStruct *)call_data)&&
		(XmCR_VALUE_CHANGED==scroll_data->reason))
	{
		if (pacing_window=(struct Pacing_window *)pacing_window_structure)
		{
			XtVaGetValues(pacing_window->control_voltcur_slider,
				XmNminimum,&slider_minumum,
				XmNmaximum,&slider_maximum,
				XmNsliderSize,&slider_size,
				NULL);
			XtVaGetValues(pacing_window->basic_cycle_length_pace_toggle,
				XmNset,&status,
				NULL);
			if (!(pacing_window->pacing)||(True==status))
			{
				control_voltcur=(pacing_window->control_voltcur)+
					(pacing_window->control_voltcur_arrow_step)*
					((float)(scroll_data->value)-
					(float)(slider_maximum-slider_minumum-slider_size)/2);
				if (control_voltcur>0)
				{
					if (pacing_window->constant_current_stimulation)
					{
						if (control_voltcur>1)
						{
							control_voltcur=1;
						}
					}
					pacing_window->control_voltcur=control_voltcur;
					if (True==status)
					{
						unemap_stop_stimulating(0);
						start_basic_cycle_length_pacing(pacing_window);
					}
				}
			}
			sprintf(value_string,"%g",pacing_window->control_voltcur);
			XtVaSetValues(pacing_window->control_voltcur_value,
				XmNcursorPosition,strlen(value_string),
				XmNvalue,value_string,
				NULL);
			XtVaSetValues(pacing_window->control_voltcur_slider,
				XmNvalue,(slider_maximum-slider_minumum-slider_size)/2,
				NULL);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"ch_pacing_control_voltcur_slide.  Missing pacing_window_structure");
		}
	}
	LEAVE;
} /* ch_pacing_control_voltcur_slide */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_control_width_form(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 26 January 2003

DESCRIPTION :
Finds the id of the control width form in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_control_width_form);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->control_width_form= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_control_width_form.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_control_width_form */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_control_width_value(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 26 January 2003

DESCRIPTION :
Finds the id of the control width value in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_control_width_value);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->control_width_value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_control_width_value.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_control_width_value */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void ch_pacing_control_width_value(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 26 January 2003

DESCRIPTION :
Called when the control width widget is changed.
==============================================================================*/
{
	Boolean status;
	char *new_value,value_string[20];
	float control_width;
	int changed;
	struct Pacing_window *pacing_window;
	XmAnyCallbackStruct *text_data

	ENTER(ch_pacing_control_width_value);
	USE_PARAMETER(widget_id);
	if ((text_data=(XmAnyCallbackStruct *)call_data)&&
		((XmCR_ACTIVATE==text_data->reason)||
		(XmCR_LOSING_FOCUS==text_data->reason)))
	{
		if (pacing_window=(struct Pacing_window *)pacing_window_structure)
		{
			changed=0;
			XtVaGetValues(pacing_window->basic_cycle_length_pace_toggle,
				XmNset,&status,
				NULL);
			if (!(pacing_window->pacing)||(True==status))
			{
				XtVaGetValues(pacing_window->control_width_value,
					XmNvalue,&new_value,
					NULL);
				if ((1==sscanf(new_value,"%f",&control_width))&&(0<control_width))
				{
					change_control_width(pacing_window,control_width);
					changed=1;
					if (True==status)
					{
						unemap_stop_stimulating(0);
						start_basic_cycle_length_pacing(pacing_window);
					}
				}
			}
			if (!changed)
			{
				sprintf(value_string,"%g",pacing_window->control_width);
				XtVaSetValues(pacing_window->control_width_value,
					XmNcursorPosition,strlen(value_string),
					XmNvalue,value_string,
					NULL);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"ch_pacing_control_width_value.  Missing pacing_window_structure");
		}
	}
	LEAVE;
} /* ch_pacing_control_width_value */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_control_width_slider(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 26 January 2003

DESCRIPTION :
Sets the id of the control width slider in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_control_width_slider);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->control_width_slider= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_control_width_slider.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_control_width_slider */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void ch_pacing_control_width_slider(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2003

DESCRIPTION :
Called when the control width slider is changed.
==============================================================================*/
{
	Boolean status;
	char value_string[20];
	float control_width;
	int changed,slider_maximum,slider_minumum,slider_size;
	struct Pacing_window *pacing_window;
	XmScrollBarCallbackStruct *scroll_data;

	ENTER(ch_pacing_control_width_slider);
	USE_PARAMETER(widget_id);
	if ((scroll_data=(XmScrollBarCallbackStruct *)call_data)&&
		(XmCR_VALUE_CHANGED==scroll_data->reason))
	{
		if (pacing_window=(struct Pacing_window *)pacing_window_structure)
		{
			changed=0;
			XtVaGetValues(pacing_window->control_width_slider,
				XmNminimum,&slider_minumum,
				XmNmaximum,&slider_maximum,
				XmNsliderSize,&slider_size,
				NULL);
			XtVaGetValues(pacing_window->basic_cycle_length_pace_toggle,
				XmNset,&status,
				NULL);
			if (!(pacing_window->pacing)||(True==status))
			{
				control_width=(pacing_window->control_width)+
					(pacing_window->control_width_arrow_step)*
					((float)(scroll_data->value)-
					(float)(slider_maximum-slider_minumum-slider_size)/2);
				if (0<control_width)
				{
					change_control_width(pacing_window,control_width);
					changed=1;
					if (True==status)
					{
						unemap_stop_stimulating(0);
						start_basic_cycle_length_pacing(pacing_window);
					}
				}
			}
			if (!changed)
			{
				sprintf(value_string,"%g",pacing_window->control_width);
				XtVaSetValues(pacing_window->control_width_value,
					XmNcursorPosition,strlen(value_string),
					XmNvalue,value_string,
					NULL);
			}
			XtVaSetValues(pacing_window->control_width_slider,
				XmNvalue,(slider_maximum-slider_minumum-slider_size)/2,
				NULL);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"ch_pacing_control_width_slider.  Missing pacing_window_structure");
		}
	}
	LEAVE;
} /* ch_pacing_control_width_slider */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_basic_cycle_length_fo(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 October 2001

DESCRIPTION :
Finds the id of the basic cycle length form in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_basic_cycle_length_fo);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->basic_cycle_length_form= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_basic_cycle_length_fo.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_basic_cycle_length_fo */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_basic_cycle_length_va(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 October 2001

DESCRIPTION :
Finds the id of the basic cycle length value in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_basic_cycle_length_va);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->basic_cycle_length_value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_basic_cycle_length_va.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_basic_cycle_length_va */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void ch_pacing_basic_cycle_length_va(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 August 2003

DESCRIPTION :
Called when the basic cycle length widget is changed.
==============================================================================*/
{
	Boolean status;
	char *new_value,value_string[20];
	float basic_cycle_length;
	struct Pacing_window *pacing_window;
	XmAnyCallbackStruct *text_data

	ENTER(ch_pacing_basic_cycle_length_va);
	USE_PARAMETER(widget_id);
	if ((text_data=(XmAnyCallbackStruct *)call_data)&&
		((XmCR_ACTIVATE==text_data->reason)||
		(XmCR_LOSING_FOCUS==text_data->reason)))
	{
		if (pacing_window=(struct Pacing_window *)pacing_window_structure)
		{
			XtVaGetValues(pacing_window->basic_cycle_length_pace_toggle,
				XmNset,&status,
				NULL);
			if (!(pacing_window->pacing)||(True==status))
			{
				XtVaGetValues(pacing_window->basic_cycle_length_value,
					XmNvalue,&new_value,
					NULL);
				if (1==sscanf(new_value,"%f",&basic_cycle_length))
				{
					/* must be a multiple of the resolution and longer than the control
						width */
					if (basic_cycle_length<(pacing_window->control_width)+
						(pacing_window->resolution))
					{
						basic_cycle_length=(pacing_window->control_width)+
							(pacing_window->resolution);
					}
					else
					{
						basic_cycle_length=(pacing_window->resolution)*
							(float)floor(basic_cycle_length/(pacing_window->resolution)+0.5);
					}
					pacing_window->basic_cycle_length=basic_cycle_length;
					if (True==status)
					{
						unemap_stop_stimulating(0);
						start_basic_cycle_length_pacing(pacing_window);
					}
				}
			}
			sprintf(value_string,"%g",pacing_window->basic_cycle_length);
			XtVaSetValues(pacing_window->basic_cycle_length_value,
				XmNcursorPosition,strlen(value_string),
				XmNvalue,value_string,
				NULL);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"ch_pacing_basic_cycle_length_va.  Missing pacing_window_structure");
		}
	}
	LEAVE;
} /* ch_pacing_basic_cycle_length_va */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_basic_cycle_length_sl(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 26 January 2003

DESCRIPTION :
Sets the id of the basic cycle length slider in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_basic_cycle_length_sl);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->basic_cycle_length_slider= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_basic_cycle_length_sl.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_basic_cycle_length_sl */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void ch_pacing_basic_cycle_length_sl(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 August 2003

DESCRIPTION :
Called when the basic cycle length slider is changed.
==============================================================================*/
{
	Boolean status;
	char value_string[20];
	float basic_cycle_length;
	int slider_maximum,slider_minumum,slider_size;
	struct Pacing_window *pacing_window;
	XmScrollBarCallbackStruct *scroll_data;

	ENTER(ch_pacing_basic_cycle_length_sl);
	USE_PARAMETER(widget_id);
	if ((scroll_data=(XmScrollBarCallbackStruct *)call_data)&&
		(XmCR_VALUE_CHANGED==scroll_data->reason))
	{
		if (pacing_window=(struct Pacing_window *)pacing_window_structure)
		{
			XtVaGetValues(pacing_window->basic_cycle_length_slider,
				XmNminimum,&slider_minumum,
				XmNmaximum,&slider_maximum,
				XmNsliderSize,&slider_size,
				NULL);
			XtVaGetValues(pacing_window->basic_cycle_length_pace_toggle,
				XmNset,&status,
				NULL);
			if (!(pacing_window->pacing)||(True==status))
			{
				basic_cycle_length=(pacing_window->basic_cycle_length)+
					(pacing_window->basic_cycle_length_arrow_step)*
					((float)(scroll_data->value)-
					(float)(slider_maximum-slider_minumum-slider_size)/2);
				/* must be a multiple of the resolution and longer than the control
					width */
				if (basic_cycle_length<(pacing_window->control_width)+
					(pacing_window->resolution))
				{
					basic_cycle_length=(pacing_window->control_width)+
						(pacing_window->resolution);
				}
				else
				{
					basic_cycle_length=(pacing_window->resolution)*
						(float)floor(basic_cycle_length/(pacing_window->resolution)+0.5);
				}
				pacing_window->basic_cycle_length=basic_cycle_length;
				if (True==status)
				{
					unemap_stop_stimulating(0);
					start_basic_cycle_length_pacing(pacing_window);
				}
			}
			sprintf(value_string,"%g",pacing_window->basic_cycle_length);
			XtVaSetValues(pacing_window->basic_cycle_length_value,
				XmNcursorPosition,strlen(value_string),
				XmNvalue,value_string,
				NULL);
			XtVaSetValues(pacing_window->basic_cycle_length_slider,
				XmNvalue,(slider_maximum-slider_minumum-slider_size)/2,
				NULL);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"ch_pacing_basic_cycle_length_sl.  Missing pacing_window_structure");
		}
	}
	LEAVE;
} /* ch_pacing_basic_cycle_length_sl */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_basic_cycle_length_pa(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 12 October 2001

DESCRIPTION :
Finds the id of the basic cycle length pace button in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_basic_cycle_length_pa);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->basic_cycle_length_pace_toggle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_basic_cycle_length_pa.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_basic_cycle_length_pa */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void ch_pacing_basic_cycle_length_pa(Widget widget,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 26 January 2003

DESCRIPTION :
Called when the basic cycle length pace button is toggled.
==============================================================================*/
{
	Boolean status;
	int i;
	struct Pacing_window *pacing_window;

	ENTER(ch_pacing_basic_cycle_length_pa);
	USE_PARAMETER(call_data);
	unemap_stop_stimulating(0);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		XtVaGetValues(widget,XmNset,&status,NULL);
		if (True==status)
		{
			start_basic_cycle_length_pacing(pacing_window);
		}
		else
		{
			/* stop pacing */
			unemap_stop_stimulating(0);
			for (i=0;i<pacing_window->number_of_pacing_channels;i++)
			{
				unemap_set_channel_stimulating((pacing_window->pacing_channels)[i],0);
			}
			for (i=0;i<pacing_window->number_of_return_channels;i++)
			{
				unemap_set_channel_stimulating((pacing_window->return_channels)[i],0);
			}
			pacing_window->pacing=0;
			/* set control sensitivity */
			XtSetSensitive(pacing_window->restitution_time_pace_toggle,True);
			XtSetSensitive(pacing_window->restitution_curve_pace_toggle,True);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"ch_pacing_basic_cycle_length_pa.  Missing pacing_window_structure");
	}
	LEAVE;
} /* ch_pacing_basic_cycle_length_pa */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_si_form(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 May 2003

DESCRIPTION :
Finds the id of the Si form in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_si_form);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->si_form= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_si_form.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_si_form */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_s_number_dec(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 May 2003

DESCRIPTION :
Sets the id of the stimulus number decrement in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_s_number_dec);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->s_number_decrement= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_s_number_dec.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_s_number_dec */
#endif /* defined (MOTIF) */

static int change_number_of_si(struct Pacing_window *pacing_window)
/*******************************************************************************
LAST MODIFIED : 14 May 2004

DESCRIPTION :
Updates after a change to the number of cycles for a stimulus type.
==============================================================================*/
{
#if defined (MOTIF)
	char value_string[21];
#endif /* defined (MOTIF) */
	int return_code;

	ENTER(change_number_of_si);
	return_code=0;
	/* check arguments */
	if (pacing_window)
	{
#if defined (MOTIF)
		if (1>=(pacing_window->number_of_si)[(pacing_window->stimulus_number)-1])
		{
			(pacing_window->number_of_si)[(pacing_window->stimulus_number)-1]=1;
			if (True==XtIsManaged(pacing_window->number_of_si_decrement))
			{
				XtVaSetValues(pacing_window->number_of_si_value,
					XmNleftWidget,pacing_window->number_of_si_label,
					NULL);
				XtUnmanageChild(pacing_window->number_of_si_decrement);
			}
		}
		else
		{
			if (False==XtIsManaged(pacing_window->number_of_si_decrement))
			{
				XtVaSetValues(pacing_window->number_of_si_value,
					XmNleftWidget,pacing_window->number_of_si_decrement,
					NULL);
				XtManageChild(pacing_window->number_of_si_decrement);
			}
		}
		sprintf(value_string,"%d",(pacing_window->number_of_si)[(pacing_window->
			stimulus_number)-1]);
		XtVaSetValues(pacing_window->number_of_si_value,
			XmNcursorPosition,strlen(value_string),
			XmNvalue,value_string,
			NULL);
#endif /* defined (MOTIF) */
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"change_number_of_si.  Missing pacing_window");
	}
	LEAVE;

	return (return_code);
} /* change_number_of_si */

static int change_stimulus_number(struct Pacing_window *pacing_window)
/*******************************************************************************
LAST MODIFIED : 25 May 2003

DESCRIPTION :
Updates after a change to the number of stimulus types.
==============================================================================*/
{
#if defined (MOTIF)
	char value_string[21];
#endif /* defined (MOTIF) */
	int return_code;

	ENTER(change_stimulus_number);
	return_code=0;
	/* check arguments */
	if (pacing_window)
	{
#if defined (MOTIF)
		if (1>=pacing_window->stimulus_number)
		{
			pacing_window->stimulus_number=1;
			if (True==XtIsManaged(pacing_window->s_number_decrement))
			{
				XtVaSetValues(pacing_window->s_number_label,
					XmNleftAttachment,XmATTACH_FORM,
					NULL);
				XtUnmanageChild(pacing_window->s_number_decrement);
			}
		}
		else
		{
			if (False==XtIsManaged(pacing_window->s_number_decrement))
			{
				XtVaSetValues(pacing_window->s_number_label,
					XmNleftAttachment,XmATTACH_WIDGET,
					XmNleftWidget,pacing_window->s_number_decrement,
					NULL);
				XtManageChild(pacing_window->s_number_decrement);
			}
		}
		if (pacing_window->stimulus_number>=pacing_window->number_of_stimulus_types)
		{
			pacing_window->stimulus_number=pacing_window->number_of_stimulus_types;
			if (True==XtIsManaged(pacing_window->s_number_increment))
			{
				XtVaSetValues(pacing_window->number_of_si_label,
					XmNleftWidget,pacing_window->s_number_label,
					NULL);
				XtUnmanageChild(pacing_window->s_number_increment);
			}
		}
		else
		{
			if (False==XtIsManaged(pacing_window->s_number_increment))
			{
				XtVaSetValues(pacing_window->number_of_si_label,
					XmNleftWidget,pacing_window->s_number_increment,
					NULL);
				XtManageChild(pacing_window->s_number_increment);
			}
		}
		sprintf(value_string,"S%d",pacing_window->stimulus_number);
		XtVaSetValues(pacing_window->s_number_label,
			XmNlabelString,XmStringCreate(value_string,XmSTRING_DEFAULT_CHARSET),
			NULL);
		return_code=change_number_of_si(pacing_window);
		return_code=change_si_length(pacing_window);
#endif /* defined (MOTIF) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"change_stimulus_number.  Missing pacing_window");
	}
	LEAVE;

	return (return_code);
} /* change_stimulus_number */

#if defined (MOTIF)
static void decrement_s_number(Widget widget,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2003

DESCRIPTION :
Decrements the stimulus number.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(decrement_s_number);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		(pacing_window->stimulus_number)--;
		change_stimulus_number(pacing_window);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"decrement_s_number.  Missing pacing_window_structure");
	}
	LEAVE;
} /* decrement_s_number */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_s_number_label(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 May 2003

DESCRIPTION :
Finds the id of the stimulus number label in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_s_number_label);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->s_number_label= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_s_number_label.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_s_number_label */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_s_number_inc(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2003

DESCRIPTION :
Sets the id of the stimulus number increment in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_s_number_inc);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->s_number_increment= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_s_number_inc.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_s_number_inc */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void increment_s_number(Widget widget,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 May 2003

DESCRIPTION :
Increments the stimulus number.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(increment_s_number);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		(pacing_window->stimulus_number)++;
		change_stimulus_number(pacing_window);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"increment_s_number.  Missing pacing_window_structure");
	}
	LEAVE;
} /* increment_s_number */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_number_of_si_label(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 May 2003

DESCRIPTION :
Sets the id of the number of Si beats label in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_number_of_si_dec);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->number_of_si_label= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_number_of_si_dec.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_number_of_si_dec */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_number_of_si_dec(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2003

DESCRIPTION :
Sets the id of the number of Si beats decrement in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_number_of_si_dec);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->number_of_si_decrement= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_number_of_si_dec.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_number_of_si_dec */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void decrement_number_of_si(Widget widget,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 May 2003

DESCRIPTION :
Decrements the number of Si beats.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(decrement_number_of_si);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		((pacing_window->number_of_si)[(pacing_window->stimulus_number)-1])--;
		change_number_of_si(pacing_window);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"decrement_number_of_si.  Missing pacing_window_structure");
	}
	LEAVE;
} /* decrement_number_of_si */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_number_of_si_value(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 May 2003

DESCRIPTION :
Sets the id of the number of Si beats value in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_number_of_si_value);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->number_of_si_value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_number_of_si_value.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_number_of_si_value */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void ch_pacing_number_of_si_value(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 May 2004

DESCRIPTION :
Called when the number of Si widget is changed.
==============================================================================*/
{
	char *new_value;
	int number_of_si;
	struct Pacing_window *pacing_window;
	XmAnyCallbackStruct *text_data

	ENTER(ch_pacing_number_of_si_value);
	USE_PARAMETER(widget_id);
	if ((text_data=(XmAnyCallbackStruct *)call_data)&&
		((XmCR_ACTIVATE==text_data->reason)||
		(XmCR_LOSING_FOCUS==text_data->reason)))
	{
		if (pacing_window=(struct Pacing_window *)pacing_window_structure)
		{
			XtVaGetValues(pacing_window->number_of_si_value,
				XmNvalue,&new_value,
				NULL);
			if (1==sscanf(new_value,"%d",&number_of_si))
			{
				(pacing_window->number_of_si)[(pacing_window->stimulus_number)-1]=
					number_of_si;
			}
			change_number_of_si(pacing_window);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"ch_pacing_number_of_si_value.  Missing pacing_window_structure");
		}
	}
	LEAVE;
} /* ch_pacing_number_of_si_value */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_number_of_si_inc(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2003

DESCRIPTION :
Sets the id of the number of Si beats increment in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_number_of_si_inc);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->number_of_si_increment= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_number_of_si_inc.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_number_of_si_inc */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void increment_number_of_si(Widget widget,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2003

DESCRIPTION :
Increments the number of Si beats.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(increment_number_of_si);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		((pacing_window->number_of_si)[(pacing_window->stimulus_number)-1])++;
		change_number_of_si(pacing_window);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"increment_number_of_si.  Missing pacing_window_structure");
	}
	LEAVE;
} /* increment_number_of_si */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_si_length_value(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 May 2003

DESCRIPTION :
Finds the id of the length of Si beats value in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_si_length_value);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->si_length_value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_si_length_value.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_si_length_value */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void ch_pacing_si_length_value(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 May 2003

DESCRIPTION :
Called when the length of si widget is changed.
==============================================================================*/
{
	char *new_value;
	float si_length;
	struct Pacing_window *pacing_window;
	XmAnyCallbackStruct *text_data

	ENTER(ch_pacing_si_length_value);
	USE_PARAMETER(widget_id);
	if ((text_data=(XmAnyCallbackStruct *)call_data)&&
		((XmCR_ACTIVATE==text_data->reason)||
		(XmCR_LOSING_FOCUS==text_data->reason)))
	{
		if (pacing_window=(struct Pacing_window *)pacing_window_structure)
		{
			if (!(pacing_window->pacing))
			{
				XtVaGetValues(pacing_window->si_length_value,
					XmNvalue,&new_value,
					NULL);
				if ((1==sscanf(new_value,"%f",&si_length))&&(0<si_length))
				{
					(pacing_window->si_length)[(pacing_window->stimulus_number)-1]=
						si_length;
				}
			}
			change_si_length(pacing_window);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"ch_pacing_si_length_value.  Missing pacing_window_structure");
		}
	}
	LEAVE;
} /* ch_pacing_si_length_value */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_number_of_s_types_for(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2003

DESCRIPTION :
Sets the id of the number of stimulus types form in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_number_of_s_types_for);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->number_of_s_types_form= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_number_of_s_types_for.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_number_of_s_types_for */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_number_of_s_types_lab(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2003

DESCRIPTION :
Sets the id of the number of stimulus types label in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_number_of_s_types_lab);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->number_of_s_types_label= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_number_of_s_types_lab.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_number_of_s_types_lab */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_number_of_s_types_dec(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2003

DESCRIPTION :
Sets the id of the number of stimulus types decrement arrow in the pacing
window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_number_of_s_types_dec);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->number_of_s_types_decrement= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_number_of_s_types_dec.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_number_of_s_types_dec */
#endif /* defined (MOTIF) */

static int change_number_of_stimulus_types(struct Pacing_window *pacing_window)
/*******************************************************************************
LAST MODIFIED : 25 May 2003

DESCRIPTION :
Updates after a change to the number of stimulus types.
==============================================================================*/
{
#if defined (MOTIF)
	char value_string[101];
#endif /* defined (MOTIF) */
	float *si_length;
	int i,*number_of_si,return_code;

	ENTER(change_number_of_stimulus_types);
	return_code=0;
	/* check arguments */
	if (pacing_window)
	{
#if defined (MOTIF)
		if (1>=pacing_window->number_of_stimulus_types)
		{
			pacing_window->number_of_stimulus_types=1;
			if (True==XtIsManaged(pacing_window->number_of_s_types_decrement))
			{
				XtVaSetValues(pacing_window->number_of_s_types_value,
					XmNleftWidget,pacing_window->number_of_s_types_label,
					NULL);
				XtUnmanageChild(pacing_window->number_of_s_types_decrement);
			}
		}
		else
		{
			if (False==XtIsManaged(pacing_window->number_of_s_types_decrement))
			{
				XtVaSetValues(pacing_window->number_of_s_types_value,
					XmNleftWidget,pacing_window->number_of_s_types_decrement,
					NULL);
				XtManageChild(pacing_window->number_of_s_types_decrement);
			}
		}
		sprintf(value_string,"%d",pacing_window->number_of_stimulus_types);
		XtVaSetValues(pacing_window->number_of_s_types_value,
			XmNlabelString,XmStringCreate(value_string,XmSTRING_DEFAULT_CHARSET),
			NULL);
#endif /* defined (MOTIF) */
		ALLOCATE(number_of_si,int,pacing_window->number_of_stimulus_types);
		ALLOCATE(si_length,float,pacing_window->number_of_stimulus_types);
		if (number_of_si&&si_length)
		{
			if (pacing_window->number_of_stimulus_types>
				pacing_window->number_of_stimulus_types_save)
			{
				for (i=0;i<pacing_window->number_of_stimulus_types_save;i++)
				{
					number_of_si[i]=(pacing_window->number_of_si)[i];
					si_length[i]=(pacing_window->si_length)[i];
				}
				for (i=pacing_window->number_of_stimulus_types_save;
					i<pacing_window->number_of_stimulus_types;i++)
				{
					number_of_si[i]=number_of_si[i-1];
					si_length[i]=si_length[i-1];
				}
			}
			else
			{
				for (i=0;i<pacing_window->number_of_stimulus_types;i++)
				{
					number_of_si[i]=(pacing_window->number_of_si)[i];
					si_length[i]=(pacing_window->si_length)[i];
				}
			}
			DEALLOCATE(pacing_window->number_of_si);
			pacing_window->number_of_si=number_of_si;
			DEALLOCATE(pacing_window->si_length);
			pacing_window->si_length=si_length;
			pacing_window->number_of_stimulus_types_save=
				pacing_window->number_of_stimulus_types;
		}
		else
		{
			DEALLOCATE(number_of_si);
			DEALLOCATE(si_length);
		}
		pacing_window->sn_length=(pacing_window->si_length)[
			(pacing_window->number_of_stimulus_types)-1];
		update_sn_length_label(pacing_window);
		update_sn_length_change_label(pacing_window);
		update_sn_length_change_factor_label(pacing_window);
		if (pacing_window->stimulus_number>pacing_window->number_of_stimulus_types)
		{
			pacing_window->stimulus_number=pacing_window->number_of_stimulus_types;
		}
		return_code=change_stimulus_number(pacing_window);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"change_number_of_stimulus_types.  Missing pacing_window");
	}
	LEAVE;

	return (return_code);
} /* change_number_of_stimulus_types */

#if defined (MOTIF)
static void decrement_number_of_s_types(Widget widget,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 May 2003

DESCRIPTION :
Decrements the number of stimulus types.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(decrement_number_of_s_types);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		(pacing_window->number_of_stimulus_types)--;
		change_number_of_stimulus_types(pacing_window);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"decrement_number_of_s_types.  Missing pacing_window_structure");
	}
	LEAVE;
} /* decrement_number_of_s_types */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_number_of_s_types_val(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2003

DESCRIPTION :
Sets the id of the number of stimulus types in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_number_of_s_types_val);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->number_of_s_types_value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_number_of_s_types_val.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_number_of_s_types_val */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_number_of_s_types_inc(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2003

DESCRIPTION :
Sets the id of the number of stimulus types increment arrow in the pacing
window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_number_of_s_types_inc);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->number_of_s_types_increment= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_number_of_s_types_inc.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_number_of_s_types_inc */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void increment_number_of_s_types(Widget widget,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2003

DESCRIPTION :
Decrements the number of stimulus types.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(increment_number_of_s_types);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		(pacing_window->number_of_stimulus_types)++;
		change_number_of_stimulus_types(pacing_window);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"increment_number_of_s_types.  Missing pacing_window_structure");
	}
	LEAVE;
} /* increment_number_of_s_types */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_sn_length_label(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2003

DESCRIPTION :
Finds the id of the Sn length label in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_sn_length_label);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->sn_length_label= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_sn_length_label.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_sn_length_label */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_sn_length_change_form(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2003

DESCRIPTION :
Finds the id of the Sn length change form in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_sn_length_change_form);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->sn_length_change_form= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_sn_length_change_form.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_sn_length_change_form */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_sn_length_change_labe(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 May 2003

DESCRIPTION :
Finds the id of the Sn length change label in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_sn_length_change_labe);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->sn_length_change_label= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_sn_length_change_labe.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_sn_length_change_labe */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_sn_length_change_valu(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2003

DESCRIPTION :
Finds the id of the Sn length change value in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_sn_length_change_valu);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->sn_length_change_value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_sn_length_change_valu.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_sn_length_change_valu */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void ch_pacing_sn_length_change_valu(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 August 2003

DESCRIPTION :
Called when the sn length change widget is changed.
==============================================================================*/
{
	char *new_value;
	float sn_length_change;
	struct Pacing_window *pacing_window;
	XmAnyCallbackStruct *text_data

	ENTER(ch_pacing_sn_length_change_valu);
	USE_PARAMETER(widget_id);
	if ((text_data=(XmAnyCallbackStruct *)call_data)&&
		((XmCR_ACTIVATE==text_data->reason)||
		(XmCR_LOSING_FOCUS==text_data->reason)))
	{
		if (pacing_window=(struct Pacing_window *)pacing_window_structure)
		{
			XtVaGetValues(pacing_window->sn_length_change_value,
				XmNvalue,&new_value,
				NULL);
			if (1==sscanf(new_value,"%f",&sn_length_change))
			{
				/* must be a multiple of the resolution */
				if (sn_length_change<pacing_window->resolution)
				{
					sn_length_change=pacing_window->resolution;
				}
				else
				{
					sn_length_change=(pacing_window->resolution)*
						floor(sn_length_change/(pacing_window->resolution)+0.5);
				}
				pacing_window->sn_length_change=sn_length_change;
				pacing_window->sn_length_change_save=sn_length_change;
			}
			update_sn_length_change_value(pacing_window);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"ch_pacing_sn_length_change_valu.  Missing pacing_window_structure");
		}
	}
	LEAVE;
} /* ch_pacing_sn_length_change_valu */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_sn_length_factor_form(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2003

DESCRIPTION :
Finds the id of the Sn length change factor form in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_sn_length_factor_form);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->sn_length_change_factor_form= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_sn_length_factor_form.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_sn_length_factor_form */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_sn_length_factor_labe(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 May 2003

DESCRIPTION :
Finds the id of the Sn length change factor label in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_sn_length_factor_labe);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->sn_length_change_factor_label= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_sn_length_factor_labe.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_sn_length_factor_labe */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_sn_length_factor_valu(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2003

DESCRIPTION :
Finds the id of the Sn length change factor value in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_sn_length_factor_valu);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->sn_length_change_factor_value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_sn_length_factor_valu.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_sn_length_factor_valu */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void ch_pacing_sn_length_factor_valu(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 2 June 2003

DESCRIPTION :
Called when the Sn length change factor widget is changed.
==============================================================================*/
{
	char *new_value;
	float sn_length_change_factor;
	struct Pacing_window *pacing_window;
	XmAnyCallbackStruct *text_data

	ENTER(ch_pacing_sn_length_factor_valu);
	USE_PARAMETER(widget_id);
	if ((text_data=(XmAnyCallbackStruct *)call_data)&&
		((XmCR_ACTIVATE==text_data->reason)||
		(XmCR_LOSING_FOCUS==text_data->reason)))
	{
		if (pacing_window=(struct Pacing_window *)pacing_window_structure)
		{
			XtVaGetValues(pacing_window->sn_length_change_factor_value,
				XmNvalue,&new_value,
				NULL);
			if ((1==sscanf(new_value,"%f",&sn_length_change_factor))&&
				(1<sn_length_change_factor))
			{
				pacing_window->sn_length_change_factor=sn_length_change_factor;
				pacing_window->sn_length_change_factor_save=sn_length_change_factor;
			}
			update_sn_length_change_factor_value(pacing_window);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"ch_pacing_sn_length_factor_valu.  Missing pacing_window_structure");
		}
	}
	LEAVE;
} /* ch_pacing_sn_length_factor_valu */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_sn_length_inc_dec_cho(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 May 2003

DESCRIPTION :
Finds the id of the Sn length change increment/decrement choice in the pacing
window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_sn_length_inc_dec_cho);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->sn_length_inc_dec_choice= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_sn_length_inc_dec_cho.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_sn_length_inc_dec_cho */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_sn_length_increment_b(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2003

DESCRIPTION :
Finds the id of the Sn length change increment button in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_sn_length_increment_b);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->sn_length_increment_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_sn_length_increment_b.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_sn_length_increment_b */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void pacing_set_sn_length_increment(Widget widget,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 2 June 2003

DESCRIPTION :
Sets Sn length change to increment.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(pacing_set_sn_length_increment);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->sn_increment=1;
		pacing_window->sn_increment_save=1;
		XtVaSetValues(pacing_window->sn_length_inc_dec_choice,
			XmNmenuHistory,pacing_window->sn_length_increment_button,
			NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"pacing_set_sn_length_increment.  Missing pacing_window_structure");
	}
	LEAVE;
} /* pacing_set_sn_length_increment */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_sn_length_decrement_b(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2003

DESCRIPTION :
Finds the id of the Sn length change decrement button in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_sn_length_decrement_b);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->sn_length_decrement_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_sn_length_decrement_b.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_sn_length_decrement_b */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void pacing_set_sn_length_decrement(Widget widget,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 2 June 2003

DESCRIPTION :
Sets Sn length change to decrement.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(pacing_set_sn_length_decrement);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->sn_increment=0;
		pacing_window->sn_increment_save=0;
		XtVaSetValues(pacing_window->sn_length_inc_dec_choice,
			XmNmenuHistory,pacing_window->sn_length_decrement_button,
			NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"pacing_set_sn_length_decrement.  Missing pacing_window_structure");
	}
	LEAVE;
} /* pacing_set_sn_length_decrement */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_restitution_time_pace(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 12 October 2001

DESCRIPTION :
Finds the id of the restitution time pace button in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_restitution_time_pace);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->restitution_time_pace_toggle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_restitution_time_pace.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_restitution_time_pace */
#endif /* defined (MOTIF) */

static int restitution_time_pace(struct Pacing_window *pacing_window)
/*******************************************************************************
LAST MODIFIED : 22 May 2004

DESCRIPTION :
Restarts the restitution time pacing.
==============================================================================*/
{
	float *pacing_voltcur,voltcurs_per_second;
	int i,j,k,number_of_control_voltcurs,number_of_pacing_voltcurs,
		number_of_si_voltcurs,number_of_sn_s1_voltcurs,return_code;

	ENTER(restitution_time_pace);
	return_code=0;
	unemap_stop_stimulating(0);
	if (pacing_window)
	{
		number_of_control_voltcurs=(int)((pacing_window->control_width)/
			(pacing_window->resolution)+0.5);
		number_of_sn_s1_voltcurs=(int)((pacing_window->sn_s1_pause)/
			(pacing_window->resolution)+0.5);
		number_of_pacing_voltcurs=number_of_sn_s1_voltcurs;
		for (i=0;i<pacing_window->number_of_stimulus_types-1;i++)
		{
			number_of_si_voltcurs=(int)(((pacing_window->si_length)[i])/
				(pacing_window->resolution)+0.5);
			number_of_pacing_voltcurs += (pacing_window->number_of_si)[i]*
				number_of_si_voltcurs;
		}
		number_of_si_voltcurs=(int)((pacing_window->sn_length)/
			(pacing_window->resolution)+0.5);
		number_of_pacing_voltcurs += (pacing_window->number_of_si)[
			pacing_window->number_of_stimulus_types-1]*number_of_si_voltcurs;
		number_of_pacing_voltcurs += number_of_control_voltcurs+1;
		if (REALLOCATE(pacing_voltcur,pacing_window->pacing_voltcurs,float,
			number_of_pacing_voltcurs))
		{
			pacing_window->pacing_voltcurs=pacing_voltcur;
			/* set up pacing */
			for (i=number_of_sn_s1_voltcurs;i>0;i--)
			{
				*pacing_voltcur=0;
				pacing_voltcur++;
			}
			for (i=0;i<pacing_window->number_of_stimulus_types-1;i++)
			{
				number_of_si_voltcurs=(int)(((pacing_window->si_length)[i])/
					(pacing_window->resolution)+0.5);
				for (j=0;j<(pacing_window->number_of_si)[i];j++)
				{
					if ((i>0)||(j>0))
					{
						for (k=number_of_si_voltcurs-number_of_control_voltcurs;k>0;k--)
						{
							*pacing_voltcur=0;
							pacing_voltcur++;
						}
					}
					for (k=number_of_control_voltcurs;k>0;k--)
					{
						*pacing_voltcur=pacing_window->control_voltcur;
						pacing_voltcur++;
					}
				}
			}
			number_of_si_voltcurs=(int)((pacing_window->sn_length)/
				(pacing_window->resolution)+0.5);
			for (j=(pacing_window->number_of_si)[
				(pacing_window->number_of_stimulus_types)-1];j>0;j--)
			{
				for (k=number_of_si_voltcurs-number_of_control_voltcurs;k>0;k--)
				{
					*pacing_voltcur=0;
					pacing_voltcur++;
				}
				for (k=number_of_control_voltcurs;k>0;k--)
				{
					*pacing_voltcur=pacing_window->control_voltcur;
					pacing_voltcur++;
				}
			}
			*pacing_voltcur=0;
			pacing_voltcur++;
			number_of_pacing_voltcurs=
				pacing_voltcur-(pacing_window->pacing_voltcurs);
			voltcurs_per_second=(float)1000./(pacing_window->resolution);
			if (0<pacing_window->number_of_pacing_channels)
			{
				for (i=0;i<pacing_window->number_of_pacing_channels;i++)
				{
					unemap_set_channel_stimulating((pacing_window->pacing_channels)[i],
						1);
				}
				if (pacing_window->constant_current_stimulation)
				{
					unemap_load_current_stimulating(
						pacing_window->number_of_pacing_channels,
						pacing_window->pacing_channels,number_of_pacing_voltcurs,
						voltcurs_per_second,pacing_window->pacing_voltcurs,(unsigned int)1,
						(Unemap_stimulation_end_callback *)NULL,(void *)NULL);
				}
				else
				{
					unemap_load_voltage_stimulating(
						pacing_window->number_of_pacing_channels,
						pacing_window->pacing_channels,number_of_pacing_voltcurs,
						voltcurs_per_second,pacing_window->pacing_voltcurs,(unsigned int)1,
						(Unemap_stimulation_end_callback *)NULL,(void *)NULL);
				}
			}
			if (0<pacing_window->number_of_return_channels)
			{
				for (i=0;i<pacing_window->number_of_return_channels;i++)
				{
					unemap_set_channel_stimulating((pacing_window->return_channels)[i],
						1);
				}
				if (pacing_window->negative_return_signal)
				{
					for (i=0;i<number_of_pacing_voltcurs;i++)
					{
						(pacing_window->pacing_voltcurs)[i]=
							-(pacing_window->pacing_voltcurs[i]);
					}
					if (pacing_window->constant_current_stimulation)
					{
						unemap_load_current_stimulating(
							pacing_window->number_of_return_channels,
							pacing_window->return_channels,number_of_pacing_voltcurs,
							voltcurs_per_second,pacing_window->pacing_voltcurs,
							(unsigned int)1,(Unemap_stimulation_end_callback *)NULL,
							(void *)NULL);
					}
					else
					{
						unemap_load_voltage_stimulating(
							pacing_window->number_of_return_channels,
							pacing_window->return_channels,number_of_pacing_voltcurs,
							voltcurs_per_second,pacing_window->pacing_voltcurs,
							(unsigned int)1,(Unemap_stimulation_end_callback *)NULL,
							(void *)NULL);
					}
					for (i=0;i<number_of_pacing_voltcurs;i++)
					{
						(pacing_window->pacing_voltcurs)[i]=
							-(pacing_window->pacing_voltcurs[i]);
					}
				}
				else
				{
					if (pacing_window->constant_current_stimulation)
					{
						unemap_load_current_stimulating(
							pacing_window->number_of_return_channels,
							pacing_window->return_channels,(int)0,(float)0,(float *)NULL,
							(unsigned int)0,(Unemap_stimulation_end_callback *)NULL,
							(void *)NULL);
					}
					else
					{
						unemap_load_voltage_stimulating(
							pacing_window->number_of_return_channels,
							pacing_window->return_channels,(int)0,(float)0,(float *)NULL,
							(unsigned int)0,(Unemap_stimulation_end_callback *)NULL,
							(void *)NULL);
					}
				}
			}
			/* start pacing */
			unemap_start_stimulating();
			if (0==pacing_window->pacing)
			{
				/* print information to standard out */
				printf("Restitution Pacing on\n");
			}
			pacing_window->pacing=1;
			/* print information to standard out */
			for (i=0;i<pacing_window->number_of_stimulus_types-1;i++)
			{
				printf("S%d.  Cycle length = %g.  Number of stimulations = %d\n",
					i+1,(pacing_window->si_length)[i],(pacing_window->number_of_si)[i]);
			}
			printf("S%d.  Cycle length = %g.  Number of stimulations = %d\n",
				i+1,pacing_window->sn_length,(pacing_window->number_of_si)[i]);
			printf("S%d length change = %g ms\n",
				pacing_window->number_of_stimulus_types,
				pacing_window->sn_length_change);
			printf("Length factor = %g\n",pacing_window->sn_length_change_factor);
			printf("Control width = %g ms\n",pacing_window->control_width);
			return_code=1;
		}
		else
		{
			if (pacing_window->pacing)
			{
				/* print information to standard out */
				printf("Restitution Pacing off\n");
			}
			pacing_window->pacing=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"restitution_time_pace.  Missing pacing_window");
	}
	LEAVE;

	return (return_code);
} /* restitution_time_pace */

#if defined (MOTIF)
static void ch_pacing_restitution_time_pace(Widget widget,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 2 June 2003

DESCRIPTION :
Called when the restitution time pace button is toggled.
==============================================================================*/
{
	Boolean status;
	int i;
	struct Pacing_window *pacing_window;

	ENTER(ch_pacing_restitution_time_pace);
	USE_PARAMETER(call_data);
	unemap_stop_stimulating(0);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		XtVaGetValues(widget,XmNset,&status,NULL);
		if (True==status)
		{
			/* start pacing */
			if (restitution_time_pace(pacing_window))
			{
				/* set control sensitivity */
				XtSetSensitive(pacing_window->restitution_time_decrement_button,True);
				XtSetSensitive(pacing_window->restitution_time_increment_button,True);
				XtSetSensitive(pacing_window->restitution_time_repeat_button,True);
				XtSetSensitive(pacing_window->restitution_time_reset_button,False);
				XtSetSensitive(pacing_window->basic_cycle_length_pace_toggle,False);
				XtSetSensitive(pacing_window->restitution_curve_pace_toggle,False);
			}
			else
			{
				status=False;
				XtVaSetValues(widget,XmNset,status,NULL);
			}
		}
		else
		{
			/* stop pacing */
			unemap_stop_stimulating(0);
			for (i=0;i<pacing_window->number_of_pacing_channels;i++)
			{
				unemap_set_channel_stimulating((pacing_window->pacing_channels)[i],0);
			}
			for (i=0;i<pacing_window->number_of_return_channels;i++)
			{
				unemap_set_channel_stimulating((pacing_window->return_channels)[i],0);
			}
			/* set control sensitivity */
			XtSetSensitive(pacing_window->restitution_time_decrement_button,False);
			XtSetSensitive(pacing_window->restitution_time_increment_button,False);
			XtSetSensitive(pacing_window->restitution_time_repeat_button,False);
			XtSetSensitive(pacing_window->restitution_time_reset_button,True);
			XtSetSensitive(pacing_window->basic_cycle_length_pace_toggle,True);
			XtSetSensitive(pacing_window->restitution_curve_pace_toggle,True);
			if (pacing_window->pacing)
			{
				/* print information to standard out */
				printf("Restitution Pacing off\n");
			}
			pacing_window->pacing=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"ch_pacing_restitution_time_pace.  Missing pacing_window_structure");
	}
	LEAVE;
} /* ch_pacing_restitution_time_pace */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_restitution_time_decr(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 2 June 2003

DESCRIPTION :
Finds the id of the restitution time decrement button in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_restitution_time_decr);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->restitution_time_decrement_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_restitution_time_decr.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_restitution_time_decr */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void ac_pacing_restitution_time_decr(Widget widget,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 August 2003

DESCRIPTION :
Called when the restitution time decrement button is pressed.
==============================================================================*/
{
	float sn_length;
	struct Pacing_window *pacing_window;

	ENTER(ac_pacing_restitution_time_decr);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	unemap_stop_stimulating(0);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		if (pacing_window->sn_increment)
		{
			/* update pacing */
			if (pacing_window->sn_length_change>pacing_window->resolution)
			{
				pacing_window->sn_length_change /=
					pacing_window->sn_length_change_factor;
				/* must be a multiple of the resolution */
				if (pacing_window->sn_length_change<pacing_window->resolution)
				{
					pacing_window->sn_length_change=pacing_window->resolution;
				}
				else
				{
					pacing_window->sn_length_change=(pacing_window->resolution)*
						floor((pacing_window->sn_length_change)/
						(pacing_window->resolution)+0.5);
				}
			}
			else
			{
				pacing_window->sn_length_change=pacing_window->resolution;
			}
			XtVaSetValues(pacing_window->sn_length_inc_dec_choice,
				XmNmenuHistory,pacing_window->sn_length_decrement_button,
				NULL);
			pacing_window->sn_increment=0;
			update_sn_length_change_value(pacing_window);
		}
		sn_length=(pacing_window->sn_length)-(pacing_window->sn_length_change);
		if ((pacing_window->control_width)+(pacing_window->resolution)<=sn_length)
		{
			pacing_window->sn_length=sn_length;
			update_sn_length_label(pacing_window);
			restitution_time_pace(pacing_window);
		}
		else
		{
			XtVaSetValues(pacing_window->restitution_time_pace_toggle,
				XmNset,False,NULL);
			/* print information to standard out */
			printf("Restitution Pacing off\n");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"ac_pacing_restitution_time_decr.  Missing pacing_window_structure");
	}
	LEAVE;
} /* ac_pacing_restitution_time_decr */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_restitution_time_incr(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 2 June 2003

DESCRIPTION :
Finds the id of the restitution time increment button in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_restitution_time_incr);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->restitution_time_increment_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_restitution_time_incr.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_restitution_time_incr */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void ac_pacing_restitution_time_incr(Widget widget,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 August 2003

DESCRIPTION :
Called when the restitution time increment button is pressed.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(ac_pacing_restitution_time_incr);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	unemap_stop_stimulating(0);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		if (!(pacing_window->sn_increment))
		{
			/* update pacing */
			if (pacing_window->sn_length_change>pacing_window->resolution)
			{
				pacing_window->sn_length_change /=
					pacing_window->sn_length_change_factor;
				/* must be a multiple of the resolution */
				if (pacing_window->sn_length_change<pacing_window->resolution)
				{
					pacing_window->sn_length_change=pacing_window->resolution;
				}
				else
				{
					pacing_window->sn_length_change=(pacing_window->resolution)*
						floor((pacing_window->sn_length_change)/
						(pacing_window->resolution)+0.5);
				}
			}
			else
			{
				pacing_window->sn_length_change=pacing_window->resolution;
			}
			XtVaSetValues(pacing_window->sn_length_inc_dec_choice,
				XmNmenuHistory,pacing_window->sn_length_increment_button,
				NULL);
			pacing_window->sn_increment=1;
			update_sn_length_change_value(pacing_window);
		}
		pacing_window->sn_length=
			(pacing_window->sn_length)+(pacing_window->sn_length_change);
		update_sn_length_label(pacing_window);
		restitution_time_pace(pacing_window);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"ac_pacing_restitution_time_incr.  Missing pacing_window_structure");
	}
	LEAVE;
} /* ac_pacing_restitution_time_incr */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_restitution_time_repe(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 2 June 2003

DESCRIPTION :
Finds the id of the restitution time repeat button in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_restitution_time_repe);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->restitution_time_repeat_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_restitution_time_repe.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_restitution_time_repe */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void ac_pacing_restitution_time_repe(Widget widget,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 2 June 2003

DESCRIPTION :
Called when the restitution time repeat button is pressed.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(ac_pacing_restitution_time_repe);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		restitution_time_pace(pacing_window);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"ac_pacing_restitution_time_repe.  Missing pacing_window_structure");
	}
	LEAVE;
} /* ac_pacing_restitution_time_repe */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_restitution_time_rese(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 2 June 2003

DESCRIPTION :
Finds the id of the restitution time reset button in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_restitution_time_rese);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->restitution_time_reset_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_restitution_time_rese.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_restitution_time_rese */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void ac_pacing_restitution_time_rese(Widget widget,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 26 May 2003

DESCRIPTION :
Called when the restitution time reset button is pressed.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(ac_pacing_restitution_time_rese);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		if (pacing_window->sn_increment_save)
		{
			pacing_set_sn_length_increment(widget,pacing_window_structure,call_data);
		}
		else
		{
			pacing_set_sn_length_decrement(widget,pacing_window_structure,call_data);
		}
		pacing_window->sn_length=(pacing_window->si_length)[
			(pacing_window->number_of_stimulus_types)-1];
		pacing_window->sn_length_change=pacing_window->sn_length_change_save;
		pacing_window->sn_length_change_factor=
			pacing_window->sn_length_change_factor_save;
		update_sn_length_label(pacing_window);
		update_sn_length_change_value(pacing_window);
		update_sn_length_change_factor_value(pacing_window);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"ac_pacing_restitution_time_rese.  Missing pacing_window_structure");
	}
	LEAVE;
} /* ac_pacing_restitution_time_rese */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_sn_s1_pause_form(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2003

DESCRIPTION :
Finds the id of the Sn-S1 pause form in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_sn_s1_pause_form);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->sn_s1_pause_form= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_sn_s1_pause_form.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_sn_s1_pause_form */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_sn_s1_pause_label(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2003

DESCRIPTION :
Finds the id of the Sn-S1 pause label in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_sn_s1_pause_label);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->sn_s1_pause_label= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_sn_s1_pause_label.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_sn_s1_pause_label */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_sn_s1_pause_value(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2003

DESCRIPTION :
Finds the id of the Sn-S1 pause value in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_sn_s1_pause_value);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->sn_s1_pause_value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_sn_s1_pause_value.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_sn_s1_pause_value */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void ch_pacing_sn_s1_pause_value(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 August 2003

DESCRIPTION :
Called when the Sn-S1 pause widget is changed.
==============================================================================*/
{
	char *new_value,value_string[20];
	float sn_s1_pause;
	struct Pacing_window *pacing_window;
	XmAnyCallbackStruct *text_data

	ENTER(ch_pacing_sn_s1_pause_value);
	USE_PARAMETER(widget_id);
	if ((text_data=(XmAnyCallbackStruct *)call_data)&&
		((XmCR_ACTIVATE==text_data->reason)||
		(XmCR_LOSING_FOCUS==text_data->reason)))
	{
		if (pacing_window=(struct Pacing_window *)pacing_window_structure)
		{
			if (!(pacing_window->pacing))
			{
				XtVaGetValues(pacing_window->sn_s1_pause_value,
					XmNvalue,&new_value,
					NULL);
				if (1==sscanf(new_value,"%f",&sn_s1_pause))
				{
					/* must be a multiple of the resolution and longer than the control
						width */
					if (sn_s1_pause<(pacing_window->control_width)+
						(pacing_window->resolution))
					{
						sn_s1_pause=(pacing_window->control_width)+
							(pacing_window->resolution);
					}
					else
					{
						sn_s1_pause=(pacing_window->resolution)*
							(float)floor(sn_s1_pause/(pacing_window->resolution)+0.5);
					}
					pacing_window->sn_s1_pause=sn_s1_pause;
				}
			}
			sprintf(value_string,"%g",pacing_window->sn_s1_pause);
			XtVaSetValues(pacing_window->sn_s1_pause_value,
				XmNcursorPosition,strlen(value_string),
				XmNvalue,value_string,
				NULL);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"ch_pacing_sn_s1_pause_value.  Missing pacing_window_structure");
		}
	}
	LEAVE;
} /* ch_pacing_sn_s1_pause_value */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_decrement_threshold_f(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 November 2001

DESCRIPTION :
Finds the id of the pacing decrement threshold pairs form in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_decrement_threshold_f);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->decrement_threshold_pairs_form= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_decrement_threshold_f.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_decrement_threshold_f */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_decrement_threshold_v(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 November 2001

DESCRIPTION :
Finds the id of the pacing decrement threshold pairs value in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_decrement_threshold_value);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->decrement_threshold_pairs_value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_decrement_threshold_v.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_decrement_threshold_v */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void ch_pacing_decrement_threshold_v(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 November 2001

DESCRIPTION :
Called when the pacing decrement threshold pairs widget is changed.
==============================================================================*/
{
	char *new_value,*decrement_threshold_pairs_string;
	struct Pacing_window *pacing_window;
	XmAnyCallbackStruct *text_data

	ENTER(ch_pacing_decrement_threshold_v);
	USE_PARAMETER(widget_id);
	if ((text_data=(XmAnyCallbackStruct *)call_data)&&
		((XmCR_ACTIVATE==text_data->reason)||
		(XmCR_LOSING_FOCUS==text_data->reason)))
	{
		if (pacing_window=(struct Pacing_window *)pacing_window_structure)
		{
			if (!(pacing_window->pacing))
			{
				XtVaGetValues(pacing_window->decrement_threshold_pairs_value,
					XmNvalue,&new_value,
					NULL);
				set_decrement_threshold_pairs_from_string(pacing_window,new_value);
			}
			if (!(decrement_threshold_pairs_string=
				pacing_window->decrement_threshold_pairs_string))
			{
				decrement_threshold_pairs_string="";
			}
			XtVaSetValues(pacing_window->decrement_threshold_pairs_value,
				XmNcursorPosition,strlen(decrement_threshold_pairs_string),
				XmNvalue,decrement_threshold_pairs_string,
				NULL);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"ch_pacing_decrement_threshold_v.  Missing pacing_window_structure");
		}
	}
	LEAVE;
} /* ch_pacing_decrement_threshold_v */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void id_pacing_restitution_curve_pac(Widget *widget_id,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 November 2001

DESCRIPTION :
Finds the id of the restitution curve pace button in the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(id_pacing_restitution_curve_pac);
	USE_PARAMETER(call_data);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		pacing_window->restitution_curve_pace_toggle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_pacing_restitution_curve_pac.  Missing pacing_window_structure");
	}
	LEAVE;
} /* id_pacing_restitution_curve_pac */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void ch_pacing_restitution_curve_pac(Widget widget,
	XtPointer pacing_window_structure,XtPointer call_data);

int restitution_curve_pacing_end_callback(void *pacing_window_void)
/*******************************************************************************
LAST MODIFIED : 26 May 2003

DESCRIPTION :
Called when the restitution curve pacing ends.
==============================================================================*/
{
	int return_code;
	struct Pacing_window *pacing_window;

	ENTER(restitution_curve_pacing_end_callback);
	return_code=0;
	if (pacing_window=(struct Pacing_window *)pacing_window_void)
	{
		XtVaSetValues(pacing_window->restitution_curve_pace_toggle,
			XmNset,False,NULL);
		ch_pacing_restitution_curve_pac(
			pacing_window->restitution_curve_pace_toggle,(XtPointer)pacing_window,
			(XtPointer)NULL);
	}
	LEAVE;

	return (return_code);
} /* restitution_curve_pacing_end_callback */
#endif /* defined (MOTIF) */

static int restitution_curve_pace(void *pacing_window_void)
/*******************************************************************************
LAST MODIFIED : 22 May 2004

DESCRIPTION :
Does one stimulus train in the restitution curve creation.
==============================================================================*/
{
	float *pacing_voltcurs,voltcurs_per_second;
	int finished,j,k,l,number_of_control_voltcurs,number_of_pacing_voltcurs,
		number_of_si_pacing_voltcurs,number_of_sn_s1_pacing_voltcurs,return_code;
	struct Pacing_window *pacing_window;

	ENTER(restitution_curve_pace);
	return_code=0;
	unemap_stop_stimulating(0);
	if (pacing_window=(struct Pacing_window *)pacing_window_void)
	{
		finished=0;
		if ((pacing_window->last_decrement_threshold_pairs)||
			(pacing_window->decrement_threshold_pair_number<
			pacing_window->number_of_decrement_threshold_pairs))
		{
			if ((pacing_window->last_decrement_threshold_pairs)||
				((pacing_window->decrement_threshold_pair_number<
				pacing_window->number_of_decrement_threshold_pairs)&&
				(pacing_window->sn_length_decrement_threshold_pairs>
				(pacing_window->decrement_threshold_pairs)[
				2*(pacing_window->decrement_threshold_pair_number)+1])))
			{
				number_of_control_voltcurs=(int)((pacing_window->control_width)/
					(pacing_window->resolution)+0.5);
				pacing_window->last_decrement_threshold_pairs=0;
				number_of_sn_s1_pacing_voltcurs=(int)((pacing_window->sn_s1_pause)/
					(pacing_window->resolution)+0.5);
				number_of_pacing_voltcurs=number_of_sn_s1_pacing_voltcurs+
					(int)((pacing_window->sn_length_decrement_threshold_pairs)/
					(pacing_window->resolution)+0.5)*(pacing_window->number_of_si)[
					(pacing_window->number_of_stimulus_types)-1];
				for (l=0;l<(pacing_window->number_of_stimulus_types)-1;l++)
				{
					number_of_pacing_voltcurs +=
						(int)(((pacing_window->si_length)[l])/
						(pacing_window->resolution)+0.5)*
						(pacing_window->number_of_si)[l];
				}
				number_of_pacing_voltcurs += number_of_control_voltcurs+1;
				if (REALLOCATE(pacing_voltcurs,pacing_window->pacing_voltcurs,
					float,number_of_pacing_voltcurs))
				{
					(pacing_window->sn_length_count_decrement_threshold_pairs)++;
					/* print information to standard out */
					printf("%d) S%d length = %g ms\n",
						pacing_window->sn_length_count_decrement_threshold_pairs,
						pacing_window->number_of_stimulus_types,
						pacing_window->sn_length_decrement_threshold_pairs);
					pacing_window->pacing_voltcurs=pacing_voltcurs;
					for (l=number_of_sn_s1_pacing_voltcurs;l>0;l--)
					{
						*pacing_voltcurs=0;
						pacing_voltcurs++;
					}
					for (l=0;l<(pacing_window->number_of_stimulus_types)-1;l++)
					{
						number_of_si_pacing_voltcurs=
							(int)(((pacing_window->si_length)[l])/
							(pacing_window->resolution)+0.5);
						for (j=0;j<(pacing_window->number_of_si)[l];j++)
						{
							if ((l>0)||(j>0))
							{
								for (k=number_of_si_pacing_voltcurs-number_of_control_voltcurs;
									k>0;k--)
								{
									*pacing_voltcurs=0;
									pacing_voltcurs++;
								}
							}
							for (k=number_of_control_voltcurs;k>0;k--)
							{
								*pacing_voltcurs=pacing_window->control_voltcur;
								pacing_voltcurs++;
							}
						}
					}
					number_of_si_pacing_voltcurs=
						(int)((pacing_window->sn_length_decrement_threshold_pairs)/
						(pacing_window->resolution)+0.5);
					for (j=(pacing_window->number_of_si)[
						(pacing_window->number_of_stimulus_types)-1];j>0;j--)
					{
						for (k=number_of_si_pacing_voltcurs-number_of_control_voltcurs;k>0;
							k--)
						{
							*pacing_voltcurs=0;
							pacing_voltcurs++;
						}
						for (k=number_of_control_voltcurs;k>0;k--)
						{
							*pacing_voltcurs=pacing_window->control_voltcur;
							pacing_voltcurs++;
						}
					}
					*pacing_voltcurs=0;
					pacing_voltcurs++;
					number_of_pacing_voltcurs=
						pacing_voltcurs-(pacing_window->pacing_voltcurs);
					pacing_window->sn_length_decrement_threshold_pairs -=
						(pacing_window->decrement_threshold_pairs)[
						2*(pacing_window->decrement_threshold_pair_number)];
					/* must be a multiple of the resolution and longer than the control
						width */
					if (pacing_window->sn_length_decrement_threshold_pairs<
						(pacing_window->control_width)+(pacing_window->resolution))
					{
						pacing_window->sn_length_decrement_threshold_pairs=
							(pacing_window->control_width)+(pacing_window->resolution);
					}
					else
					{
						pacing_window->sn_length_decrement_threshold_pairs=
							(pacing_window->resolution)*
							(float)floor((pacing_window->sn_length_decrement_threshold_pairs)/
							(pacing_window->resolution)+0.5);
					}
					if (pacing_window->sn_length_decrement_threshold_pairs<=
						(pacing_window->decrement_threshold_pairs)[
						2*(pacing_window->decrement_threshold_pair_number)+1])
					{
						pacing_window->sn_length_decrement_threshold_pairs=
							(pacing_window->decrement_threshold_pairs)[
							2*(pacing_window->decrement_threshold_pair_number)+1];
						/* must be a multiple of the resolution and longer than the control
							width */
						if (pacing_window->sn_length_decrement_threshold_pairs<
							(pacing_window->control_width)+(pacing_window->resolution))
						{
							pacing_window->sn_length_decrement_threshold_pairs=
								(pacing_window->control_width)+(pacing_window->resolution);
						}
						else
						{
							pacing_window->sn_length_decrement_threshold_pairs=
								(pacing_window->resolution)*(float)floor((pacing_window->
								sn_length_decrement_threshold_pairs)/
								(pacing_window->resolution)+0.5);
						}
						(pacing_window->decrement_threshold_pair_number)++;
						if (pacing_window->decrement_threshold_pair_number==
							pacing_window->number_of_decrement_threshold_pairs)
						{
							pacing_window->last_decrement_threshold_pairs=1;
						}
					}
					if (0<number_of_pacing_voltcurs)
					{
						/* set up pacing */
						voltcurs_per_second=(float)1000./(pacing_window->resolution);
						if (0<pacing_window->number_of_pacing_channels)
						{
							for (l=0;l<pacing_window->number_of_pacing_channels;l++)
							{
								unemap_set_channel_stimulating(
									(pacing_window->pacing_channels)[l],1);
							}
							if (pacing_window->constant_current_stimulation)
							{
								unemap_load_current_stimulating(
									pacing_window->number_of_pacing_channels,
									pacing_window->pacing_channels,number_of_pacing_voltcurs,
									voltcurs_per_second,pacing_window->pacing_voltcurs,
									(unsigned int)1,restitution_curve_pace,
									pacing_window_void);
							}
							else
							{
								unemap_load_voltage_stimulating(
									pacing_window->number_of_pacing_channels,
									pacing_window->pacing_channels,number_of_pacing_voltcurs,
									voltcurs_per_second,pacing_window->pacing_voltcurs,
									(unsigned int)1,restitution_curve_pace,
									pacing_window_void);
							}
						}
						if (0<pacing_window->number_of_return_channels)
						{
							for (l=0;l<pacing_window->number_of_return_channels;l++)
							{
								unemap_set_channel_stimulating(
									(pacing_window->return_channels)[l],1);
							}
							if (pacing_window->negative_return_signal)
							{
								for (l=0;l<number_of_pacing_voltcurs;l++)
								{
									(pacing_window->pacing_voltcurs)[l]=
										-(pacing_window->pacing_voltcurs[l]);
								}
								if (pacing_window->constant_current_stimulation)
								{
									unemap_load_current_stimulating(
										pacing_window->number_of_return_channels,
										pacing_window->return_channels,number_of_pacing_voltcurs,
										voltcurs_per_second,pacing_window->pacing_voltcurs,
										(unsigned int)1,(Unemap_stimulation_end_callback *)NULL,
										(void *)NULL);
								}
								else
								{
									unemap_load_voltage_stimulating(
										pacing_window->number_of_return_channels,
										pacing_window->return_channels,number_of_pacing_voltcurs,
										voltcurs_per_second,pacing_window->pacing_voltcurs,
										(unsigned int)1,(Unemap_stimulation_end_callback *)NULL,
										(void *)NULL);
								}
								for (l=0;l<number_of_pacing_voltcurs;l++)
								{
									(pacing_window->pacing_voltcurs)[l]=
										-(pacing_window->pacing_voltcurs[l]);
								}
							}
							else
							{
								if (pacing_window->constant_current_stimulation)
								{
									unemap_load_current_stimulating(
										pacing_window->number_of_return_channels,
										pacing_window->return_channels,(int)0,(float)0,
										(float *)NULL,(unsigned int)0,
										(Unemap_stimulation_end_callback *)NULL,(void *)NULL);
								}
								else
								{
									unemap_load_voltage_stimulating(
										pacing_window->number_of_return_channels,
										pacing_window->return_channels,(int)0,(float)0,
										(float *)NULL,(unsigned int)0,
										(Unemap_stimulation_end_callback *)NULL,(void *)NULL);
								}
							}
						}
						/* start pacing */
						unemap_start_stimulating();
						return_code=1;
					}
					else
					{
						finished=1;
					}
				}
				else
				{
					finished=1;
				}
			}
			else
			{
				finished=1;
			}
		}
		else
		{
			finished=1;
		}
		if (finished)
		{
#if defined (MOTIF)
			XtVaSetValues(pacing_window->restitution_curve_pace_toggle,
				XmNset,False,
				NULL);
			ch_pacing_restitution_curve_pac(
				pacing_window->restitution_curve_pace_toggle,(XtPointer)pacing_window,
				(XtPointer)NULL);
#endif /* defined (MOTIF) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"restitution_curve_pace.  Missing pacing_window");
	}
	LEAVE;

	return (return_code);
} /* restitution_curve_pace */

#if defined (MOTIF)
static void ch_pacing_restitution_curve_pac(Widget widget,
	XtPointer pacing_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 May 2004

DESCRIPTION :
Called when the restitution curve pace button is toggled.
==============================================================================*/
{
	Boolean status;
	float *pacing_voltcurs,sn_length,voltcurs_per_second;
	int decrement_threshold_pair_number,j,k,l,last,number_of_control_voltcurs,
		number_of_pacing_voltcurs,number_of_si_pacing_voltcurs,
		number_of_sn_pacing_voltcurs,number_of_sn_s1_pacing_voltcurs,return_code,
		sn_length_count,software_version;
	struct Pacing_window *pacing_window;

	ENTER(ch_pacing_restitution_curve_pac);
	USE_PARAMETER(call_data);
	unemap_stop_stimulating(0);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		XtVaGetValues(widget,XmNset,&status,NULL);
		if (True==status)
		{
			return_code=0;
			/* print information to standard out */
			printf("Restitution Curve Pacing start with\n");
			for (l=0;l<pacing_window->number_of_stimulus_types;l++)
			{
				printf("S%d.  Cycle length = %g.  Number of stimulations = %d\n",l+1,
					(pacing_window->si_length)[l],(pacing_window->number_of_si)[l]);
			}
			printf("Control width = %g ms\n",pacing_window->control_width);
			printf("S%d-S1 pause = %g ms\n",pacing_window->number_of_stimulus_types,
				pacing_window->sn_s1_pause);
			for (l=0;l<pacing_window->number_of_decrement_threshold_pairs;l++)
			{
				printf("%d.  S%d length decrement = %g ms.  "
					"S%d length threshold = %g ms\n",l+1,
					pacing_window->number_of_stimulus_types,
					(pacing_window->decrement_threshold_pairs)[2*l],
					pacing_window->number_of_stimulus_types,
					(pacing_window->decrement_threshold_pairs)[2*l+1]);
			}
			if (unemap_get_software_version(&software_version)&&(3<=software_version))
			{
				/* stimulation end callbacks */
				pacing_window->decrement_threshold_pair_number=0;
				pacing_window->last_decrement_threshold_pairs=0;
				pacing_window->sn_length_decrement_threshold_pairs=(pacing_window->
					si_length)[(pacing_window->number_of_stimulus_types)-1];
				pacing_window->sn_length_count_decrement_threshold_pairs=0;
				if (restitution_curve_pace((void *)pacing_window))
				{
					/* set control sensitivity */
					XtSetSensitive(pacing_window->restitution_time_pace_toggle,False);
					XtSetSensitive(pacing_window->basic_cycle_length_pace_toggle,False);
					pacing_window->pacing=1;
				}
			}
			else
			{
				/* no stimulation end callbacks */
				number_of_control_voltcurs=(int)((pacing_window->control_width)/
					(pacing_window->resolution)+0.5);
				number_of_sn_s1_pacing_voltcurs=(int)((pacing_window->sn_s1_pause)/
					(pacing_window->resolution)+0.5);
				if (REALLOCATE(pacing_voltcurs,pacing_window->pacing_voltcurs,float,
					number_of_sn_s1_pacing_voltcurs))
				{
					pacing_window->pacing_voltcurs=pacing_voltcurs;
					for (l=number_of_sn_s1_pacing_voltcurs;l>0;l--)
					{
						*pacing_voltcurs=0;
						pacing_voltcurs++;
					}
					number_of_pacing_voltcurs=number_of_sn_s1_pacing_voltcurs;
					sn_length=(pacing_window->si_length)[
						(pacing_window->number_of_stimulus_types)-1];
					return_code=1;
					decrement_threshold_pair_number=0;
					last=0;
					sn_length_count=0;
					while (return_code&&(last||(decrement_threshold_pair_number<
						pacing_window->number_of_decrement_threshold_pairs)))
					{
						while (return_code&&(last||((decrement_threshold_pair_number<
							pacing_window->number_of_decrement_threshold_pairs)&&
							(sn_length>(pacing_window->decrement_threshold_pairs)[
							2*decrement_threshold_pair_number+1]))))
						{
							number_of_sn_pacing_voltcurs=(int)(sn_length/
								(pacing_window->resolution)+0.5)*
								(pacing_window->number_of_si)[
								(pacing_window->number_of_stimulus_types)-1];
							for (l=0;l<(pacing_window->number_of_stimulus_types)-1;l++)
							{
								number_of_sn_pacing_voltcurs +=
									(int)(((pacing_window->si_length)[l])/
									(pacing_window->resolution)+0.5)*
									(pacing_window->number_of_si)[l];
							}
							if (REALLOCATE(pacing_voltcurs,pacing_window->pacing_voltcurs,
								float,number_of_pacing_voltcurs+number_of_sn_pacing_voltcurs+
								number_of_sn_s1_pacing_voltcurs))
							{
								/* print information to standard out */
								sn_length_count++;
								printf("%d) S%d length = %g ms\n",sn_length_count,
									pacing_window->number_of_stimulus_types,sn_length);
								pacing_window->pacing_voltcurs=pacing_voltcurs;
								pacing_voltcurs += number_of_pacing_voltcurs;
								for (l=0;l<(pacing_window->number_of_stimulus_types)-1;l++)
								{
									number_of_si_pacing_voltcurs=
										(int)(((pacing_window->si_length)[l])/
										(pacing_window->resolution)+0.5);
									for (j=0;j<(pacing_window->number_of_si)[l];j++)
									{
										if ((l>0)||(j>0))
										{
											for (k=number_of_si_pacing_voltcurs-
												number_of_control_voltcurs;k>0;k--)
											{
												*pacing_voltcurs=0;
												pacing_voltcurs++;
											}
										}
										for (k=number_of_control_voltcurs;k>0;k--)
										{
											*pacing_voltcurs=pacing_window->control_voltcur;
											pacing_voltcurs++;
										}
									}
								}
								number_of_si_pacing_voltcurs=(int)(sn_length/
									(pacing_window->resolution)+0.5);
								for (j=(pacing_window->number_of_si)[
									(pacing_window->number_of_stimulus_types)-1];j>0;j--)
								{
									for (k=number_of_si_pacing_voltcurs-
										number_of_control_voltcurs;k>0;k--)
									{
										*pacing_voltcurs=0;
										pacing_voltcurs++;
									}
									for (k=number_of_control_voltcurs;k>0;k--)
									{
										*pacing_voltcurs=pacing_window->control_voltcur;
										pacing_voltcurs++;
									}
								}
								for (l=number_of_sn_s1_pacing_voltcurs;l>0;l--)
								{
									*pacing_voltcurs=0;
									pacing_voltcurs++;
								}
								number_of_pacing_voltcurs=
									pacing_voltcurs-(pacing_window->pacing_voltcurs);
								sn_length -= (pacing_window->decrement_threshold_pairs)[
									2*decrement_threshold_pair_number];
								/* must be a multiple of the resolution and longer than the
									control width */
								if (sn_length<(pacing_window->control_width)+
									(pacing_window->resolution))
								{
									sn_length=(pacing_window->control_width)+
										(pacing_window->resolution);
								}
								else
								{
									sn_length=(pacing_window->resolution)*
										(float)floor(sn_length/(pacing_window->resolution)+0.5);
								}
							}
							else
							{
								return_code=0;
							}
							last=0;
						}
						if (decrement_threshold_pair_number<
							pacing_window->number_of_decrement_threshold_pairs)
						{
							sn_length=(pacing_window->decrement_threshold_pairs)[
								2*decrement_threshold_pair_number+1];
							if (decrement_threshold_pair_number+1==
								pacing_window->number_of_decrement_threshold_pairs)
							{
								last=1;
							}
						}
						decrement_threshold_pair_number++;
					}
				}
				if (return_code&&(0<number_of_pacing_voltcurs))
				{
					/* set control sensitivity */
					XtSetSensitive(pacing_window->restitution_time_pace_toggle,False);
					XtSetSensitive(pacing_window->basic_cycle_length_pace_toggle,False);
					/* set up pacing */
					voltcurs_per_second=1000./(pacing_window->resolution);
					if (0<pacing_window->number_of_pacing_channels)
					{
						for (l=0;l<pacing_window->number_of_pacing_channels;l++)
						{
							unemap_set_channel_stimulating(
								(pacing_window->pacing_channels)[l],1);
						}
						if (pacing_window->constant_current_stimulation)
						{
							unemap_load_current_stimulating(
								pacing_window->number_of_pacing_channels,
								pacing_window->pacing_channels,number_of_pacing_voltcurs,
								voltcurs_per_second,pacing_window->pacing_voltcurs,
								(unsigned int)1,(Unemap_stimulation_end_callback *)NULL,
								(void *)NULL);
						}
						else
						{
							unemap_load_voltage_stimulating(
								pacing_window->number_of_pacing_channels,
								pacing_window->pacing_channels,number_of_pacing_voltcurs,
								voltcurs_per_second,pacing_window->pacing_voltcurs,
								(unsigned int)1,(Unemap_stimulation_end_callback *)NULL,
								(void *)NULL);
						}
					}
					if (0<pacing_window->number_of_return_channels)
					{
						for (l=0;l<pacing_window->number_of_return_channels;l++)
						{
							unemap_set_channel_stimulating(
								(pacing_window->return_channels)[l],1);
						}
						if (pacing_window->negative_return_signal)
						{
							for (l=0;l<number_of_pacing_voltcurs;l++)
							{
								(pacing_window->pacing_voltcurs)[l]=
									-(pacing_window->pacing_voltcurs[l]);
							}
							if (pacing_window->constant_current_stimulation)
							{
								unemap_load_current_stimulating(
									pacing_window->number_of_return_channels,
									pacing_window->return_channels,number_of_pacing_voltcurs,
									voltcurs_per_second,pacing_window->pacing_voltcurs,
									(unsigned int)1,(Unemap_stimulation_end_callback *)NULL,
									(void *)NULL);
							}
							else
							{
								unemap_load_voltage_stimulating(
									pacing_window->number_of_return_channels,
									pacing_window->return_channels,number_of_pacing_voltcurs,
									voltcurs_per_second,pacing_window->pacing_voltcurs,
									(unsigned int)1,(Unemap_stimulation_end_callback *)NULL,
									(void *)NULL);
							}
							for (l=0;l<number_of_pacing_voltcurs;l++)
							{
								(pacing_window->pacing_voltcurs)[l]=
									-(pacing_window->pacing_voltcurs[l]);
							}
						}
						else
						{
							if (pacing_window->constant_current_stimulation)
							{
								unemap_load_current_stimulating(
									pacing_window->number_of_return_channels,
									pacing_window->return_channels,(int)0,(float)0,(float *)NULL,
									(unsigned int)0,(Unemap_stimulation_end_callback *)NULL,
									(void *)NULL);
							}
							else
							{
								unemap_load_voltage_stimulating(
									pacing_window->number_of_return_channels,
									pacing_window->return_channels,(int)0,(float)0,(float *)NULL,
									(unsigned int)0,(Unemap_stimulation_end_callback *)NULL,
									(void *)NULL);
							}
						}
					}
					/* start pacing */
					unemap_start_stimulating();
					pacing_window->pacing=1;
				}
				else
				{
					status=False;
					XtVaSetValues(widget,XmNset,status,NULL);
				}
			}
		}
		else
		{
			/* print information to standard out */
			printf("Restitution Curve Pacing finish\n");
			/* stop pacing */
			unemap_stop_stimulating(0);
			for (l=0;l<pacing_window->number_of_pacing_channels;l++)
			{
				unemap_set_channel_stimulating((pacing_window->pacing_channels)[l],0);
			}
			for (l=0;l<pacing_window->number_of_return_channels;l++)
			{
				unemap_set_channel_stimulating((pacing_window->return_channels)[l],0);
			}
			pacing_window->pacing=0;
			/* set control sensitivity */
			XtSetSensitive(pacing_window->restitution_time_pace_toggle,True);
			XtSetSensitive(pacing_window->basic_cycle_length_pace_toggle,True);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"ch_pacing_restitution_curve_pac.  Missing pacing_window_structure");
	}
	LEAVE;
} /* ch_pacing_restitution_curve_pac */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void close_Pacing_window(Widget widget,XtPointer pacing_window_structure,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 October 2001

DESCRIPTION :
Closes the windows associated with the pacing window.
==============================================================================*/
{
	struct Pacing_window *pacing_window;

	ENTER(close_Pacing_window);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	unemap_stop_stimulating(0);
	if (pacing_window=(struct Pacing_window *)pacing_window_structure)
	{
		/* close the pacing window box */
		XtPopdown(pacing_window->shell);
		/* unghost the activation button */
		XtSetSensitive(pacing_window->activation,True);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"close_Pacing_window.  Missing pacing_window_structure");
	}
	LEAVE;
} /* close_Pacing_window */
#endif /* defined (MOTIF) */

static struct Pacing_window *create_Pacing_window(
	struct Pacing_window **pacing_window_address,struct Rig **rig_address,
#if defined (MOTIF)
	Widget activation,
#endif /* defined (MOTIF) */
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 22 May 2004

DESCRIPTION :
Allocates the memory for a pacing window.  Retrieves the necessary widgets and
initializes the appropriate fields.
==============================================================================*/
{
#if defined (MOTIF)
	Boolean constant_current_stimulation,negative_return_signal,sn_increment;
	char *decrement_threshold_pairs_string,*numbers_of_stimuli,*pacing_electrodes,
		*return_electrodes,*stimuli_lengths,temp_string[20];
	MrmType pacing_window_class;
	static MrmRegisterArg callback_list[]={
		{"close_Pacing_window",(XtPointer)close_Pacing_window},
		{"destroy_Pacing_window_callback",
		(XtPointer)destroy_Pacing_window_callback},
		{"id_pacing_electrodes_form",(XtPointer)id_pacing_electrodes_form},
		{"id_pacing_electrodes_value",(XtPointer)id_pacing_electrodes_value},
		{"ch_pacing_electrodes_value",(XtPointer)ch_pacing_electrodes_value},
		{"id_pacing_return_electrodes_for",
		(XtPointer)id_pacing_return_electrodes_for},
		{"id_pacing_return_electrodes_val",
		(XtPointer)id_pacing_return_electrodes_val},
		{"ch_pacing_return_electrodes_val",
		(XtPointer)ch_pacing_return_electrodes_val},
		{"id_pacing_control_voltcur_form",
		(XtPointer)id_pacing_control_voltcur_form},
		{"id_pacing_control_voltcur_label",
		(XtPointer)id_pacing_control_voltcur_label},
		{"id_pacing_control_voltcur_value",
		(XtPointer)id_pacing_control_voltcur_value},
		{"ch_pacing_control_voltcur_value",
		(XtPointer)ch_pacing_control_voltcur_value},
		{"id_pacing_control_voltcur_slide",
		(XtPointer)id_pacing_control_voltcur_slide},
		{"ch_pacing_control_voltcur_slide",
		(XtPointer)ch_pacing_control_voltcur_slide},
		{"id_pacing_control_width_form",(XtPointer)id_pacing_control_width_form},
		{"id_pacing_control_width_value",(XtPointer)id_pacing_control_width_value},
		{"ch_pacing_control_width_value",(XtPointer)ch_pacing_control_width_value},
		{"id_pacing_control_width_slider",
		(XtPointer)id_pacing_control_width_slider},
		{"ch_pacing_control_width_slider",
		(XtPointer)ch_pacing_control_width_slider},
		{"id_pacing_basic_cycle_length_fo",
		(XtPointer)id_pacing_basic_cycle_length_fo},
		{"id_pacing_basic_cycle_length_va",
		(XtPointer)id_pacing_basic_cycle_length_va},
		{"ch_pacing_basic_cycle_length_va",
		(XtPointer)ch_pacing_basic_cycle_length_va},
		{"id_pacing_basic_cycle_length_sl",
		(XtPointer)id_pacing_basic_cycle_length_sl},
		{"ch_pacing_basic_cycle_length_sl",
		(XtPointer)ch_pacing_basic_cycle_length_sl},
		{"id_pacing_basic_cycle_length_pa",
		(XtPointer)id_pacing_basic_cycle_length_pa},
		{"ch_pacing_basic_cycle_length_pa",
		(XtPointer)ch_pacing_basic_cycle_length_pa},
		{"id_pacing_si_form",(XtPointer)id_pacing_si_form},
		{"id_pacing_s_number_dec",(XtPointer)id_pacing_s_number_dec},
		{"decrement_s_number",(XtPointer)decrement_s_number},
		{"id_pacing_s_number_label",(XtPointer)id_pacing_s_number_label},
		{"id_pacing_s_number_inc",(XtPointer)id_pacing_s_number_inc},
		{"increment_s_number",(XtPointer)increment_s_number},
		{"id_pacing_number_of_si_label",(XtPointer)id_pacing_number_of_si_label},
		{"id_pacing_number_of_si_inc",(XtPointer)id_pacing_number_of_si_inc},
		{"increment_number_of_si",(XtPointer)increment_number_of_si},
		{"id_pacing_number_of_si_value",(XtPointer)id_pacing_number_of_si_value},
		{"ch_pacing_number_of_si_value",(XtPointer)ch_pacing_number_of_si_value},
		{"id_pacing_number_of_si_dec",(XtPointer)id_pacing_number_of_si_dec},
		{"decrement_number_of_si",(XtPointer)decrement_number_of_si},
		{"id_pacing_si_length_value",(XtPointer)id_pacing_si_length_value},
		{"ch_pacing_si_length_value",(XtPointer)ch_pacing_si_length_value},
		{"id_pacing_number_of_s_types_for",
		(XtPointer)id_pacing_number_of_s_types_for},
		{"id_pacing_number_of_s_types_lab",
		(XtPointer)id_pacing_number_of_s_types_lab},
		{"id_pacing_number_of_s_types_dec",
		(XtPointer)id_pacing_number_of_s_types_dec},
		{"decrement_number_of_s_types",(XtPointer)decrement_number_of_s_types},
		{"id_pacing_number_of_s_types_val",
		(XtPointer)id_pacing_number_of_s_types_val},
		{"id_pacing_number_of_s_types_inc",
		(XtPointer)id_pacing_number_of_s_types_inc},
		{"increment_number_of_s_types",(XtPointer)increment_number_of_s_types},
		{"id_pacing_sn_length_label",(XtPointer)id_pacing_sn_length_label},
		{"id_pacing_sn_length_change_form",
		(XtPointer)id_pacing_sn_length_change_form},
		{"id_pacing_sn_length_change_labe",
		(XtPointer)id_pacing_sn_length_change_labe},
		{"id_pacing_sn_length_change_valu",
		(XtPointer)id_pacing_sn_length_change_valu},
		{"ch_pacing_sn_length_change_valu",
		(XtPointer)ch_pacing_sn_length_change_valu},
		{"id_pacing_sn_length_factor_form",
		(XtPointer)id_pacing_sn_length_factor_form},
		{"id_pacing_sn_length_factor_labe",
		(XtPointer)id_pacing_sn_length_factor_labe},
		{"id_pacing_sn_length_factor_valu",
		(XtPointer)id_pacing_sn_length_factor_valu},
		{"id_pacing_sn_length_inc_dec_cho",
		(XtPointer)id_pacing_sn_length_inc_dec_cho},
		{"id_pacing_sn_length_increment_b",
		(XtPointer)id_pacing_sn_length_increment_b},
		{"pacing_set_sn_length_increment",
		(XtPointer)pacing_set_sn_length_increment},
		{"id_pacing_sn_length_decrement_b",
		(XtPointer)id_pacing_sn_length_decrement_b},
		{"pacing_set_sn_length_decrement",
		(XtPointer)pacing_set_sn_length_decrement},
		{"ch_pacing_sn_length_factor_valu",
		(XtPointer)ch_pacing_sn_length_factor_valu},
		{"id_pacing_restitution_time_pace",
		(XtPointer)id_pacing_restitution_time_pace},
		{"ch_pacing_restitution_time_pace",
		(XtPointer)ch_pacing_restitution_time_pace},
		{"id_pacing_restitution_time_incr",
		(XtPointer)id_pacing_restitution_time_incr},
		{"ac_pacing_restitution_time_incr",
		(XtPointer)ac_pacing_restitution_time_incr},
		{"id_pacing_restitution_time_decr",
		(XtPointer)id_pacing_restitution_time_decr},
		{"ac_pacing_restitution_time_decr",
		(XtPointer)ac_pacing_restitution_time_decr},
		{"id_pacing_restitution_time_repe",
		(XtPointer)id_pacing_restitution_time_repe},
		{"ac_pacing_restitution_time_repe",
		(XtPointer)ac_pacing_restitution_time_repe},
		{"id_pacing_restitution_time_rese",
		(XtPointer)id_pacing_restitution_time_rese},
		{"ac_pacing_restitution_time_rese",
		(XtPointer)ac_pacing_restitution_time_rese},
		{"id_pacing_sn_s1_pause_form",(XtPointer)id_pacing_sn_s1_pause_form},
		{"id_pacing_sn_s1_pause_label",(XtPointer)id_pacing_sn_s1_pause_label},
		{"id_pacing_sn_s1_pause_value",(XtPointer)id_pacing_sn_s1_pause_value},
		{"ch_pacing_sn_s1_pause_value",(XtPointer)ch_pacing_sn_s1_pause_value},
		{"id_pacing_decrement_threshold_f",
		(XtPointer)id_pacing_decrement_threshold_f},
		{"id_pacing_decrement_threshold_v",
		(XtPointer)id_pacing_decrement_threshold_v},
		{"ch_pacing_decrement_threshold_v",
		(XtPointer)ch_pacing_decrement_threshold_v},
		{"id_pacing_restitution_curve_pac",
		(XtPointer)id_pacing_restitution_curve_pac},
		{"ch_pacing_restitution_curve_pac",
		(XtPointer)ch_pacing_restitution_curve_pac}
		};
	static MrmRegisterArg identifier_list[]=
	{
		{"pacing_window_structure",(XtPointer)NULL}
	};
#define XmNrestitutionBasicCycleLength "restitutionBasicCycleLength"
#define XmCRestitutionBasicCycleLength "RestitutionBasicCycleLength"
#define XmNrestitutionBasicCycleLengthArrowStep "restitutionBasicCycleLengthArrowStep"
#define XmCRestitutionBasicCycleLengthArrowStep "RestitutionBasicCycleLengthArrowStep"
#define XmNrestitutionConstantCurrentStimulation "restitutionConstantCurrentStimulation"
#define XmCRestitutionConstantCurrentStimulation "RestitutionConstantCurrentStimulation"
#define XmNrestitutionControlCurrent "restitutionControlCurrent"
#define XmCRestitutionControlCurrent "RestitutionControlCurrent"
#define XmNrestitutionControlCurrentArrowStep "restitutionControlCurrentArrowStep"
#define XmCRestitutionControlCurrentArrowStep "RestitutionControlCurrentArrowStep"
#define XmNrestitutionControlVoltage "restitutionControlVoltage"
#define XmCRestitutionControlVoltage "RestitutionControlVoltage"
#define XmNrestitutionControlVoltageArrowStep "restitutionControlVoltageArrowStep"
#define XmCRestitutionControlVoltageArrowStep "RestitutionControlVoltageArrowStep"
#define XmNrestitutionControlWidth "restitutionControlWidth"
#define XmCRestitutionControlWidth "RestitutionControlWidth"
#define XmNrestitutionControlWidthArrowStep "restitutionControlWidthArrowStep"
#define XmCRestitutionControlWidthArrowStep "RestitutionControlWidthArrowStep"
#define XmNrestitutionDecrementThresholdPairs "restitutionDecrementThresholdPairs"
#define XmCRestitutionDecrementThresholdPairs "RestitutionDecrementThresholdPairs"
#define XmNrestitutionNegativeReturnSignal "restitutionNegativeReturnSignal"
#define XmCRestitutionNegativeReturnSignal "RestitutionNegativeReturnSignal"
#define XmNrestitutionNumberOfStimulusTypes "restitutionNumberOfStimulusTypes"
#define XmCRestitutionNumberOfStimulusTypes "RestitutionNumberOfStimulusTypes"
#define XmNrestitutionNumberOfSi "restitutionNumberOfSi"
#define XmCRestitutionNumberOfSi "RestitutionNumberOfSi"
#define XmNrestitutionPacingElectrodes "restitutionPacingElectrodes"
#define XmCRestitutionPacingElectrodes "RestitutionPacingElectrodes"
#define XmNrestitutionReturnElectrodes "restitutionReturnElectrodes"
#define XmCRestitutionReturnElectrodes "RestitutionReturnElectrodes"
#define XmNrestitutionSiLength "restitutionSiLength"
#define XmCRestitutionSiLength "RestitutionSiLength"
#define XmNrestitutionSnIncrement "restitutionSnIncrement"
#define XmCRestitutionSnIncrement "RestitutionSnIncrement"
#define XmNrestitutionSnLengthChange "restitutionSnLengthChange"
#define XmCRestitutionSnLengthChange "RestitutionSnLengthChange"
#define XmNrestitutionSnLengthChangeFactor "restitutionSnLengthChangeFactor"
#define XmCRestitutionSnLengthChangeFactor "RestitutionSnLengthChangeFactor"
#define XmNrestitutionResolution "restitutionResolution"
#define XmCRestitutionResolution "RestitutionResolution"
#define XmNrestitutionSnS1Pause "restitutionSnS1Pause"
#define XmCRestitutionSnS1Pause "RestitutionSnS1Pause"
	static XtResource resources[]=
	{
		{
			XmNrestitutionBasicCycleLength,
			XmCRestitutionBasicCycleLength,
			XmRFloat,
			sizeof(float),
			XtOffsetOf(Pacing_window_settings,basic_cycle_length),
			XmRString,
			"600"
		},
		{
			XmNrestitutionBasicCycleLengthArrowStep,
			XmCRestitutionBasicCycleLengthArrowStep,
			XmRFloat,
			sizeof(float),
			XtOffsetOf(Pacing_window_settings,basic_cycle_length_arrow_step),
			XmRString,
			"10"
		},
		{
			XmNrestitutionControlVoltage,
			XmCRestitutionControlVoltage,
			XmRFloat,
			sizeof(float),
			XtOffsetOf(Pacing_window_settings,control_voltcur),
			XmRString,
			"1"
		},
		{
			XmNrestitutionControlVoltageArrowStep,
			XmCRestitutionControlVoltageArrowStep,
			XmRFloat,
			sizeof(float),
			XtOffsetOf(Pacing_window_settings,control_voltcur_arrow_step),
			XmRString,
			"0.01"
		},
		{
			XmNrestitutionNumberOfStimulusTypes,
			XmCRestitutionNumberOfStimulusTypes,
			XmRInt,
			sizeof(int),
			XtOffsetOf(Pacing_window_settings,number_of_stimulus_types),
			XmRString,
			"2"
		},
		{
			XmNrestitutionResolution,
			XmCRestitutionResolution,
			XmRFloat,
			sizeof(float),
			XtOffsetOf(Pacing_window_settings,resolution),
			XmRString,
			"10"
		},
		{
			XmNrestitutionSnLengthChange,
			XmCRestitutionSnLengthChange,
			XmRFloat,
			sizeof(float),
			XtOffsetOf(Pacing_window_settings,sn_length_change),
			XmRString,
			"100"
		},
		{
			XmNrestitutionSnLengthChangeFactor,
			XmCRestitutionSnLengthChangeFactor,
			XmRFloat,
			sizeof(float),
			XtOffsetOf(Pacing_window_settings,sn_length_change_factor),
			XmRString,
			"2"
		},
		{
			XmNrestitutionSnS1Pause,
			XmCRestitutionSnS1Pause,
			XmRFloat,
			sizeof(float),
			XtOffsetOf(Pacing_window_settings,sn_s1_pause),
			XmRString,
			"800"
		},
	};
	static XtResource sn_increment_resources[]=
	{
		{
			XmNrestitutionSnIncrement,
			XmCRestitutionSnIncrement,
			XmRBoolean,
			sizeof(Boolean),
			0,
			XmRString,
			"false"
		},
	};
	static XtResource constant_current_stimulation_resources[]=
	{
		{
			XmNrestitutionConstantCurrentStimulation,
			XmCRestitutionConstantCurrentStimulation,
			XmRBoolean,
			sizeof(Boolean),
			0,
			XmRString,
			"false"
		},
	};
	static XtResource control_current_resources[]=
	{
		{
			XmNrestitutionControlCurrent,
			XmCRestitutionControlCurrent,
			XmRFloat,
			sizeof(float),
			XtOffsetOf(Pacing_window_settings,control_voltcur),
			XmRString,
			"1"
		},
		{
			XmNrestitutionControlCurrentArrowStep,
			XmCRestitutionControlCurrentArrowStep,
			XmRFloat,
			sizeof(float),
			XtOffsetOf(Pacing_window_settings,control_voltcur_arrow_step),
			XmRString,
			"0.01"
		},
	};
	static XtResource negative_return_signal_resources[]=
	{
		{
			XmNrestitutionNegativeReturnSignal,
			XmCRestitutionNegativeReturnSignal,
			XmRBoolean,
			sizeof(Boolean),
			0,
			XmRString,
			"false"
		},
	};
	static XtResource control_width_resources[]=
	{
		{
			XmNrestitutionControlWidth,
			XmCRestitutionControlWidth,
			XmRFloat,
			sizeof(float),
			XtOffsetOf(Pacing_window_settings,control_width),
			XmRString,
			"10"
		},
		{
			XmNrestitutionControlWidthArrowStep,
			XmCRestitutionControlWidthArrowStep,
			XmRFloat,
			sizeof(float),
			XtOffsetOf(Pacing_window_settings,control_width_arrow_step),
			XmRString,
			"1"
		},
	};
	static XtResource pacing_electrodes_resources[]=
	{
		{
			XmNrestitutionPacingElectrodes,
			XmCRestitutionPacingElectrodes,
			XmRString,
			sizeof(String),
			0,
			XmRString,
			""
		},
	};
	static XtResource return_electrodes_resources[]=
	{
		{
			XmNrestitutionReturnElectrodes,
			XmCRestitutionReturnElectrodes,
			XmRString,
			sizeof(String),
			0,
			XmRString,
			""
		},
	};
	static XtResource numbers_of_stimuli_resources[]=
	{
		{
			XmNrestitutionNumberOfSi,
			XmCRestitutionNumberOfSi,
			XmRString,
			sizeof(String),
			0,
			XmRString,
			"9"
		},
	};
	static XtResource stimuli_lengths_resources[]=
	{
		{
			XmNrestitutionSiLength,
			XmCRestitutionSiLength,
			XmRString,
			sizeof(String),
			0,
			XmRString,
			"500"
		},
	};
	static XtResource decrement_threshold_pairs_resources[]=
	{
		{
			XmNrestitutionDecrementThresholdPairs,
			XmCRestitutionDecrementThresholdPairs,
			XmRString,
			sizeof(String),
			0,
			XmRString,
			""
		},
	};
#endif /* defined (MOTIF) */
	struct Pacing_window *pacing_window;

	ENTER(create_Pacing_window);
	pacing_window=(struct Pacing_window *)NULL;
	/* check arguments */
	if (rig_address&&user_interface)
	{
#if defined (MOTIF)
		if (MrmOpenHierarchy_base64_string(pacing_window_uidh,
			&pacing_window_hierarchy,&pacing_window_hierarchy_open))
		{
#endif /* defined (MOTIF) */
			if (ALLOCATE(pacing_window,struct Pacing_window,1))
			{
				pacing_window->address=pacing_window_address;
				pacing_window->rig_address=rig_address;
				pacing_window->user_interface=user_interface;
				pacing_window->pacing=0;
				pacing_window->pacing_voltcurs=(float *)NULL;
				pacing_window->number_of_pacing_channels=0;
				pacing_window->pacing_channels=(int *)NULL;
				pacing_window->pacing_electrodes=(char *)NULL;
				pacing_window->number_of_return_channels=0;
				pacing_window->return_channels=(int *)NULL;
				pacing_window->return_electrodes=(char *)NULL;
				pacing_window->constant_current_stimulation=0;
				pacing_window->negative_return_signal=0;
				pacing_window->number_of_decrement_threshold_pairs=0;
				pacing_window->decrement_threshold_pairs=(float *)NULL;
				pacing_window->decrement_threshold_pairs_string=(char *)NULL;
				pacing_window->decrement_threshold_pair_number=0;
				pacing_window->last_decrement_threshold_pairs=0;
				pacing_window->sn_length_decrement_threshold_pairs=(float)0;
				pacing_window->sn_length_count_decrement_threshold_pairs=0;
				pacing_window->stimulus_number=1;
				pacing_window->number_of_si=(int *)NULL;
				pacing_window->si_length=(float *)NULL;
				pacing_window->number_of_stimulus_types_save=0;
				pacing_window->sn_increment=0;
				pacing_window->sn_increment_save=0;
				pacing_window->sn_length=0;
				pacing_window->sn_length_change=0;
				pacing_window->sn_length_change_save=0;
				pacing_window->sn_length_change_factor=0;
				pacing_window->sn_length_change_factor_save=0;
#if defined (MOTIF)
				pacing_window->activation=activation;
				pacing_window->window=(Widget)NULL;
				pacing_window->shell=(Widget)NULL;
				pacing_window->basic_cycle_length_pace_toggle=(Widget)NULL;
				pacing_window->restitution_time_pace_toggle=(Widget)NULL;
				pacing_window->restitution_time_repeat_button=(Widget)NULL;
				pacing_window->restitution_time_reset_button=(Widget)NULL;
				pacing_window->restitution_time_decrement_button=(Widget)NULL;
				pacing_window->restitution_time_increment_button=(Widget)NULL;
				pacing_window->basic_cycle_length_form=(Widget)NULL;
				pacing_window->basic_cycle_length_value=(Widget)NULL;
				pacing_window->basic_cycle_length_slider=(Widget)NULL;
				pacing_window->control_voltcur_form=(Widget)NULL;
				pacing_window->control_voltcur_label=(Widget)NULL;
				pacing_window->control_voltcur_value=(Widget)NULL;
				pacing_window->control_voltcur_slider=(Widget)NULL;
				pacing_window->control_width_form=(Widget)NULL;
				pacing_window->control_width_value=(Widget)NULL;
				pacing_window->control_width_slider=(Widget)NULL;
				pacing_window->number_of_s_types_form=(Widget)NULL;
				pacing_window->number_of_s_types_label=(Widget)NULL;
				pacing_window->number_of_s_types_decrement=(Widget)NULL;
				pacing_window->number_of_s_types_value=(Widget)NULL;
				pacing_window->number_of_s_types_increment=(Widget)NULL;
				pacing_window->si_form=(Widget)NULL;
				pacing_window->s_number_label=(Widget)NULL;
				pacing_window->s_number_decrement=(Widget)NULL;
				pacing_window->s_number_increment=(Widget)NULL;
				pacing_window->number_of_si_label=(Widget)NULL;
				pacing_window->number_of_si_value=(Widget)NULL;
				pacing_window->number_of_si_decrement=(Widget)NULL;
				pacing_window->number_of_si_increment=(Widget)NULL;
				pacing_window->si_length_value=(Widget)NULL;
				pacing_window->pacing_electrodes_form=(Widget)NULL;
				pacing_window->pacing_electrodes_value=(Widget)NULL;
				pacing_window->return_electrodes_form=(Widget)NULL;
				pacing_window->return_electrodes_value=(Widget)NULL;
				pacing_window->sn_length_form=(Widget)NULL;
				pacing_window->sn_length_value=(Widget)NULL;
				pacing_window->sn_length_label=(Widget)NULL;
				pacing_window->sn_length_change_form=(Widget)NULL;
				pacing_window->sn_length_change_label=(Widget)NULL;
				pacing_window->sn_length_change_value=(Widget)NULL;
				pacing_window->sn_length_change_factor_form=(Widget)NULL;
				pacing_window->sn_length_change_factor_label=(Widget)NULL;
				pacing_window->sn_length_change_factor_value=(Widget)NULL;
				pacing_window->sn_length_inc_dec_choice=(Widget)NULL;
				pacing_window->sn_length_increment_button=(Widget)NULL;
				pacing_window->sn_length_decrement_button=(Widget)NULL;
				pacing_window->sn_s1_pause_form=(Widget)NULL;
				pacing_window->sn_s1_pause_label=(Widget)NULL;
				pacing_window->sn_s1_pause_value=(Widget)NULL;
				pacing_window->decrement_threshold_pairs_form=(Widget)NULL;
				pacing_window->decrement_threshold_pairs_value=(Widget)NULL;
				pacing_window->restitution_curve_pace_toggle=(Widget)NULL;
				/* retrieve the settings */
				XtVaGetApplicationResources(
					User_interface_get_application_shell(user_interface),
					pacing_window,resources,XtNumber(resources),NULL);
				pacing_window->sn_length_change_save=pacing_window->sn_length_change;
				pacing_window->sn_length_change_factor_save=
					pacing_window->sn_length_change_factor;
				XtVaGetApplicationResources(
					User_interface_get_application_shell(user_interface),&sn_increment,
					sn_increment_resources,XtNumber(sn_increment_resources),NULL);
				if (True==sn_increment)
				{
					pacing_window->sn_increment=1;
				}
				else
				{
					pacing_window->sn_increment=0;
				}
				pacing_window->sn_increment_save=pacing_window->sn_increment;
				if (pacing_window->resolution<=0)
				{
					pacing_window->resolution=10;
				}
				sprintf(temp_string,"%g",pacing_window->resolution);
				control_width_resources[0].default_addr=temp_string;
				control_width_resources[1].default_addr=temp_string;
				XtVaGetApplicationResources(
					User_interface_get_application_shell(user_interface),
					pacing_window,control_width_resources,
					XtNumber(control_width_resources),NULL);
				pacing_electrodes=(char *)NULL;
				XtVaGetApplicationResources(
					User_interface_get_application_shell(user_interface),
					&pacing_electrodes,pacing_electrodes_resources,
					XtNumber(pacing_electrodes_resources),NULL);
				/* NB.  XtVaGetApplicationResources does not allocate memory for
					pacing_electrodes, so it does not need to be free'd */
				set_pacing_channels_from_string(pacing_window,pacing_electrodes);
				return_electrodes=(char *)NULL;
				XtVaGetApplicationResources(
					User_interface_get_application_shell(user_interface),
					&return_electrodes,return_electrodes_resources,
					XtNumber(return_electrodes_resources),NULL);
				/* NB.  XtVaGetApplicationResources does not allocate memory for
					return_electrodes, so it does not need to be free'd */
				set_return_channels_from_string(pacing_window,return_electrodes);
				XtVaGetApplicationResources(
					User_interface_get_application_shell(user_interface),
					&constant_current_stimulation,constant_current_stimulation_resources,
					XtNumber(constant_current_stimulation_resources),NULL);
				if (True==constant_current_stimulation)
				{
					pacing_window->constant_current_stimulation=1;
					XtVaGetApplicationResources(
						User_interface_get_application_shell(user_interface),
						pacing_window,control_current_resources,
						XtNumber(control_current_resources),NULL);
				}
				else
				{
					pacing_window->constant_current_stimulation=0;
				}
				XtVaGetApplicationResources(
					User_interface_get_application_shell(user_interface),
					&negative_return_signal,negative_return_signal_resources,
					XtNumber(negative_return_signal_resources),NULL);
				if (True==negative_return_signal)
				{
					pacing_window->negative_return_signal=1;
				}
				else
				{
					pacing_window->negative_return_signal=0;
				}
				numbers_of_stimuli=(char *)NULL;
				XtVaGetApplicationResources(
					User_interface_get_application_shell(user_interface),
					&numbers_of_stimuli,numbers_of_stimuli_resources,
					XtNumber(numbers_of_stimuli_resources),NULL);
				/* NB.  XtVaGetApplicationResources does not allocate memory for
					numbers_of_stimuli, so it does not need to be free'd */
				set_numbers_of_stimuli_from_string(pacing_window,numbers_of_stimuli);
				stimuli_lengths=(char *)NULL;
				XtVaGetApplicationResources(
					User_interface_get_application_shell(user_interface),
					&stimuli_lengths,stimuli_lengths_resources,
					XtNumber(stimuli_lengths_resources),NULL);
				/* NB.  XtVaGetApplicationResources does not allocate memory for
					stimuli_lengths, so it does not need to be free'd */
				set_stimuli_lengths_from_string(pacing_window,stimuli_lengths);
				pacing_window->sn_length=(pacing_window->si_length)[
					(pacing_window->number_of_stimulus_types)-1];
				decrement_threshold_pairs_string=(char *)NULL;
				XtVaGetApplicationResources(
					User_interface_get_application_shell(user_interface),
					&decrement_threshold_pairs_string,decrement_threshold_pairs_resources,
					XtNumber(decrement_threshold_pairs_resources),NULL);
				/* NB.  XtVaGetApplicationResources does not allocate memory for
					decrement_threshold_pairs, so it does not need to be free'd */
				set_decrement_threshold_pairs_from_string(pacing_window,
					decrement_threshold_pairs_string);
#endif /* defined (MOTIF) */
#if defined (MOTIF)
				/* create the window shell */
				if (pacing_window->shell=XtVaCreatePopupShell(
					"pacing_window_shell",
					topLevelShellWidgetClass,User_interface_get_application_shell(
					user_interface),
					XmNallowShellResize,False,
					XmNuserData,user_interface,
					NULL))
				{
					/*???DB.  What is this for? */
					XtAddCallback(pacing_window->shell,XmNdestroyCallback,
						destroy_window_shell,(XtPointer)create_Shell_list_item(
						&(pacing_window->shell),user_interface));
					/* register the other callbacks */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(pacing_window_hierarchy,
						callback_list,XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value=(XtPointer)pacing_window;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(pacing_window_hierarchy,
							identifier_list,XtNumber(identifier_list)))
						{
							/* fetch the window widget */
							if (MrmSUCCESS==MrmFetchWidget(pacing_window_hierarchy,
								"pacing_window",pacing_window->shell,&(pacing_window->window),
								&pacing_window_class))
							{
								if (pacing_window->constant_current_stimulation)
								{
									XtVaSetValues(pacing_window->control_voltcur_label,
										XmNlabelString,XmStringCreate(
										"Control square wave amplitude (proportion of maximum current)",
										XmSTRING_DEFAULT_CHARSET),
										NULL);
								}
								XtManageChild(pacing_window->window);
#endif /* defined (MOTIF) */
								if (pacing_window_address)
								{
									*pacing_window_address=pacing_window;
								}
								/*???DB.  Anything else? */
#if defined (MOTIF)
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"create_Pacing_window.  Could not fetch the window widget");
								XtDestroyWidget(pacing_window->shell);
								DEALLOCATE(pacing_window);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_Pacing_window.  Could not register identifiers");
							XtDestroyWidget(pacing_window->shell);
							DEALLOCATE(pacing_window);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_Pacing_window.  Could not register callbacks");
						XtDestroyWidget(pacing_window->shell);
						DEALLOCATE(pacing_window);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Pacing_window.  Could not create window shell");
					DEALLOCATE(pacing_window);
				}
#endif /* defined (MOTIF) */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_Pacing_window.  Insufficient memory for pacing window");
			}
#if defined (MOTIF)
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Pacing_window.  Could not open hierarchy");
		}
#endif /* defined (MOTIF) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Pacing_window.  Invalid argument(s).  %p %p",rig_address,
			user_interface);
	}
	LEAVE;

	return (pacing_window);
} /* create_Pacing_window */

/*
Global functions
----------------
*/
struct Pacing_window *open_Pacing_window(
	struct Pacing_window **pacing_window_address,struct Rig **rig_address,
#if defined (MOTIF)
	Widget activation,
#endif /* defined (MOTIF) */
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 15 August 2003

DESCRIPTION :
If the pacing window does not exist, it is created.  The pacing window is
opened.
==============================================================================*/
{
	int i;
	struct Pacing_window *pacing_window;
#if defined (MOTIF)
	char *decrement_threshold_pairs_string,*pacing_electrodes,
		*return_electrodes,value_string[101];
#endif /* defined (MOTIF) */

	ENTER(open_pacing_window);
	pacing_window=(struct Pacing_window *)NULL;
	if (pacing_window_address)
	{
		if (!(pacing_window= *pacing_window_address))
		{
			pacing_window=create_Pacing_window(pacing_window_address,rig_address,
#if defined (MOTIF)
				activation,
#endif /* defined (MOTIF) */
				user_interface);
		}
		if (pacing_window)
		{
			/* make sure that the window is consistent with its values */
			if (pacing_window->resolution<=0)
			{
				pacing_window->resolution=10;
			}
			if (pacing_window->control_width<pacing_window->resolution)
			{
				pacing_window->control_width=pacing_window->resolution;
			}
			if (pacing_window->control_width_arrow_step<pacing_window->resolution)
			{
				pacing_window->control_width_arrow_step=pacing_window->resolution;
			}
#if defined (MOTIF)
			sprintf(value_string,"%g",pacing_window->control_width);
			XtVaSetValues(pacing_window->control_width_value,
				XmNcursorPosition,strlen(value_string),
				XmNvalue,value_string,
				NULL);
#endif /* defined (MOTIF) */
#if defined (MOTIF)
			if (!(pacing_electrodes=pacing_window->pacing_electrodes))
			{
				pacing_electrodes="";
			}
			XtVaSetValues(pacing_window->pacing_electrodes_value,
				XmNcursorPosition,strlen(pacing_electrodes),
				XmNvalue,pacing_electrodes,
				NULL);
#endif /* defined (MOTIF) */
#if defined (MOTIF)
			if (!(return_electrodes=pacing_window->return_electrodes))
			{
				return_electrodes="";
			}
			XtVaSetValues(pacing_window->return_electrodes_value,
				XmNcursorPosition,strlen(return_electrodes),
				XmNvalue,return_electrodes,
				NULL);
#endif /* defined (MOTIF) */
			if (pacing_window->basic_cycle_length<=0)
			{
				pacing_window->basic_cycle_length=600;
			}
			if (pacing_window->basic_cycle_length_arrow_step<=0)
			{
				pacing_window->basic_cycle_length_arrow_step=10;
			}
#if defined (MOTIF)
			sprintf(value_string,"%g",pacing_window->basic_cycle_length);
			XtVaSetValues(pacing_window->basic_cycle_length_value,
				XmNcursorPosition,strlen(value_string),
				XmNvalue,value_string,
				NULL);
#endif /* defined (MOTIF) */
			if (pacing_window->control_voltcur<=0)
			{
				pacing_window->control_voltcur=1;
			}
			if (pacing_window->control_voltcur_arrow_step<=0)
			{
				pacing_window->control_voltcur_arrow_step=(float)0.01;
			}
#if defined (MOTIF)
			sprintf(value_string,"%g",pacing_window->control_voltcur);
			XtVaSetValues(pacing_window->control_voltcur_value,
				XmNcursorPosition,strlen(value_string),
				XmNvalue,value_string,
				NULL);
#endif /* defined (MOTIF) */
			if (pacing_window->number_of_stimulus_types<1)
			{
				pacing_window->number_of_stimulus_types=2;
			}
			change_number_of_stimulus_types(pacing_window);
			if (pacing_window->stimulus_number<1)
			{
				pacing_window->stimulus_number=2;
			}
			for (i=0;i<pacing_window->number_of_stimulus_types;i++)
			{
				if ((pacing_window->number_of_si)[i]<1)
				{
					(pacing_window->number_of_si)[i]=1;
				}
				if ((pacing_window->si_length)[i]<=0)
				{
					(pacing_window->si_length)[i]=500;
				}
				/* must be a multiple of the resolution and longer than the control
					width */
				if ((pacing_window->si_length)[i]<(pacing_window->control_width)+
					(pacing_window->resolution))
				{
					(pacing_window->si_length)[i]=(pacing_window->control_width)+
						(pacing_window->resolution);
				}
				else
				{
					(pacing_window->si_length)[i]=(pacing_window->resolution)*
						(float)floor((pacing_window->si_length)[i]/
						(pacing_window->resolution)+0.5);
				}
			}
			change_stimulus_number(pacing_window);
#if defined (MOTIF)
			sprintf(value_string,"%d",(pacing_window->number_of_si)[
				(pacing_window->stimulus_number)-1]);
			XtVaSetValues(pacing_window->number_of_si_value,
				XmNcursorPosition,strlen(value_string),
				XmNvalue,value_string,
				NULL);
#endif /* defined (MOTIF) */
			pacing_window->sn_length=(pacing_window->si_length)[
				(pacing_window->number_of_stimulus_types)-1];
			update_sn_length_label(pacing_window);
#if defined (MOTIF)
			if (pacing_window->sn_increment)
			{
				pacing_set_sn_length_increment((Widget)NULL,(XtPointer)pacing_window,
					(XtPointer)NULL);
			}
			else
			{
				pacing_set_sn_length_decrement((Widget)NULL,(XtPointer)pacing_window,
					(XtPointer)NULL);
			}
#endif /* defined (MOTIF) */
			if (pacing_window->sn_length_change<=0)
			{
				pacing_window->sn_length_change=100;
			}
			/* must be a multiple of the resolution */
			if (pacing_window->sn_length_change<pacing_window->resolution)
			{
				pacing_window->sn_length_change=pacing_window->resolution;
			}
			else
			{
				pacing_window->sn_length_change=(pacing_window->resolution)*
					(float)floor((pacing_window->sn_length_change)/
					(pacing_window->resolution)+0.5);
			}
			pacing_window->sn_length_change_save=pacing_window->sn_length_change;
			update_sn_length_change_value(pacing_window);
			if (pacing_window->sn_length_change_factor<=1)
			{
				pacing_window->sn_length_change_factor=2;
			}
			pacing_window->sn_length_change_factor_save=
				pacing_window->sn_length_change_factor;
			update_sn_length_change_factor_value(pacing_window);
#if defined (MOTIF)
			XtSetSensitive(pacing_window->restitution_time_decrement_button,False);
			XtSetSensitive(pacing_window->restitution_time_increment_button,False);
			XtSetSensitive(pacing_window->restitution_time_repeat_button,False);
			XtSetSensitive(pacing_window->restitution_time_reset_button,True);
#endif /* defined (MOTIF) */
			if (pacing_window->sn_s1_pause<=0)
			{
				pacing_window->sn_s1_pause=800;
			}
			/* must be a multiple of the resolution and longer than the control
				width */
			if (pacing_window->sn_s1_pause<(pacing_window->control_width)+
				(pacing_window->resolution))
			{
				pacing_window->sn_s1_pause=(pacing_window->control_width)+
					(pacing_window->resolution);
			}
			else
			{
				pacing_window->sn_s1_pause=(pacing_window->resolution)*
					(float)floor((pacing_window->sn_s1_pause)/
					(pacing_window->resolution)+0.5);
			}
#if defined (MOTIF)
			sprintf(value_string,"%g",pacing_window->sn_s1_pause);
			XtVaSetValues(pacing_window->sn_s1_pause_value,
				XmNcursorPosition,strlen(value_string),
				XmNvalue,value_string,
				NULL);
#endif /* defined (MOTIF) */
#if defined (MOTIF)
			if (!(decrement_threshold_pairs_string=
				pacing_window->decrement_threshold_pairs_string))
			{
				decrement_threshold_pairs_string="";
			}
			XtVaSetValues(pacing_window->decrement_threshold_pairs_value,
				XmNcursorPosition,strlen(decrement_threshold_pairs_string),
				XmNvalue,decrement_threshold_pairs_string,
				NULL);
#endif /* defined (MOTIF) */
#if defined (MOTIF)
			if (pacing_window->activation)
			{
				/* ghost the activation button */
				XtSetSensitive(pacing_window->activation,False);
				/* pop up the pacing window */
				XtPopup(pacing_window->shell,XtGrabNone);
			}
#endif /* defined (MOTIF) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"open_pacing_window.  Missing pacing_window_address");
	}
	LEAVE;

	return (pacing_window);
} /* open_pacing_window */

int destroy_Pacing_window(struct Pacing_window **pacing_window_address)
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
If the <address> field of the pacing window is not NULL, <*address> is set to
NULL.  If the <activation> field is not NULL, the <activation> widget is
unghosted.  The function frees the memory associated with the fields of the
pacing window and frees the memory associated with the pacing window.
==============================================================================*/
{
	int return_code;
	struct Pacing_window *pacing_window;

	ENTER(destroy_Pacing_window);
	return_code=0;
	unemap_stop_stimulating(0);
	if (pacing_window_address)
	{
		if (pacing_window= *pacing_window_address)
		{
			return_code=1;
			if (pacing_window->address)
			{
				*(pacing_window->address)=(struct Pacing_window *)NULL;
			}
#if defined (MOTIF)
			if (pacing_window->shell)
			{
				XtPopdown(pacing_window->shell);
			}
			/* unghost the activation button */
			XtSetSensitive(pacing_window->activation,True);
#endif /* defined (MOTIF) */
			DEALLOCATE(pacing_window->pacing_voltcurs);
			DEALLOCATE(pacing_window->pacing_channels);
			DEALLOCATE(pacing_window->pacing_electrodes);
			DEALLOCATE(pacing_window->return_channels);
			DEALLOCATE(pacing_window->return_electrodes);
			DEALLOCATE(pacing_window->decrement_threshold_pairs_string);
			DEALLOCATE(pacing_window->decrement_threshold_pairs);
			DEALLOCATE(pacing_window);
			*pacing_window_address=(struct Pacing_window *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_Pacing_window.  Missing pacing_window_address");
	}
	LEAVE;

	return (return_code);
} /* destroy_Pacing_window */
