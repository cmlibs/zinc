/*******************************************************************************
FILE : cell_unemap_interface.c

LAST MODIFIED : 1 November 2001

DESCRIPTION :
The interface between Cell and UnEMAP
==============================================================================*/

#include <stdio.h>
#include <string.h>

#include "cell/cell_unemap_interface.h"
#include "unemap/analysis_work_area.h"
#include "user_interface/filedir.h"

/*
Module objects
--------------
*/
struct Cell_unemap_interface
/*******************************************************************************
LAST MODIFIED : 30 October 2000

DESCRIPTION :
A data object which stores all the information which enables Cell to access
UnEMAP
==============================================================================*/
{
  struct Analysis_work_area *analysis_work_area;
  struct Rig **saved_rigs;
  int number_of_saved_rigs;
  int save_signals;
}; /* struct Cell_unemap_interface */

typedef struct
/*******************************************************************************
LAST MODIFIED : 31 October 2000

DESCRIPTION :
UnEMAP user settings.
==============================================================================*/
{
  Pixel analysis_colour;
} User_settings;

/*
Module functions
----------------
*/
static void associate_mapping_analysis(Widget widget,XtPointer system_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 31 October 2000

DESCRIPTION :
need this for analysis window focus callback (analysis_window.uil)
??? CELL doesn't do any mapping ???
==============================================================================*/
{
  ENTER(associate_mapping_analysis);
  USE_PARAMETER(widget);
  USE_PARAMETER(system_window);
  USE_PARAMETER(call_data);
  LEAVE;
} /* associate_mapping_analysis */

static int overlay_rig(struct Analysis_work_area *analysis,
  struct Rig *overlay_rig)
/*******************************************************************************
LAST MODIFIED : 13 November 2000

DESCRIPTION :
Overlays <overlay_rig> onto <analysis> - see analysis_overlay_signal_file() in
unemap/analysis_work_area.c
==============================================================================*/
{
	float *float_values,frequency,gain,offset,*temp_float_values,temp_frequency,
		temp_gain,temp_offset,temp_time,temp_time2,time;
	int found,i,index,j,k,l,maximum_signal_number,number_of_common_devices,
		number_of_samples,number_of_signals,return_code,temp_number_of_samples,
		temp_number_of_signals;
	short int *short_int_values,*temp_short_int_values;
	struct Device **device,**temp_device;
	struct Rig *rig,*temp_rig;
	struct Signal *signal;
	struct Signal_buffer *buffer,*temp_buffer;

	ENTER(overlay_rig);
	return_code=0;
	/* check the arguments */
	if (analysis && overlay_rig)
	{
    /* set the original rig */
		if (rig = analysis->rig)
		{
			/* set the overlay rig */
			if (temp_rig = overlay_rig)
			{
				/* assume that analysis is not done on overlay signals, so don't read
					the event detection settings (see analysis_read_signal_file) */
				/* check for common devices and determine the maximum signal number */
				number_of_common_devices=0;
				maximum_signal_number= -1;
				if ((device=rig->devices)&&(temp_rig->devices))
				{
					for (i=rig->number_of_devices;i>0;i--)
					{
						if ((*device)&&(signal=(*device)->signal))
						{
							do
							{
								if (signal->number>maximum_signal_number)
								{
									maximum_signal_number=signal->number;
								}
								signal=signal->next;
							} while (signal);
							temp_device=temp_rig->devices;
							j=temp_rig->number_of_devices;
							found=0;
							while ((j>0)&&!found)
							{
								if ((*temp_device)&&!strcmp((*device)->description->name,
									(*temp_device)->description->name))
								{
									found=1;
								}
								else
								{
									temp_device++;
									j--;
								}
							}
							if (found&&((*temp_device)->signal)&&
								get_Device_signal_buffer(*temp_device))
							{
								number_of_common_devices++;
							}
						}
						device++;
					}
				}
				if (0<number_of_common_devices)
				{
          /* reset the signal range to automatic */
          device=rig->devices;
          for (i=rig->number_of_devices;i>0;i--)
          {
            (*device)->signal_display_maximum=0;
            (*device)->signal_display_minimum=1;
            device++;
          }
					if (rig&&(device=rig->devices)&&(*device)&&
						(buffer=get_Device_signal_buffer(*device)))
					{
						/* extend the buffer to include the overlay signals */
						number_of_signals=buffer->number_of_signals;
						number_of_samples=buffer->number_of_samples;
						switch (buffer->value_type)
						{
							case SHORT_INT_VALUE:
							{
								if (REALLOCATE(short_int_values,(buffer->signals).
									short_int_values,short int,number_of_samples*
									(number_of_signals+number_of_common_devices)))
								{
									(buffer->signals).short_int_values=short_int_values;
									/* expand signals */
									short_int_values += number_of_signals*number_of_samples;
									temp_short_int_values=short_int_values+number_of_samples*
										number_of_common_devices;
									for (i=number_of_samples;i>0;i--)
									{
										for (j=number_of_common_devices;j>0;j--)
										{
											temp_short_int_values--;
											*temp_short_int_values=(short int)0;
										}
										for (j=number_of_signals;j>0;j--)
										{
											temp_short_int_values--;
											short_int_values--;
											*temp_short_int_values= *short_int_values;
										}
									}
									return_code=1;
								}
							} break;
							case FLOAT_VALUE:
							{
								if (REALLOCATE(float_values,(buffer->signals).float_values,
									float,number_of_samples*(number_of_signals+
									number_of_common_devices)))
								{
									(buffer->signals).float_values=float_values;
									/* expand signals */
									float_values += number_of_signals*number_of_samples;
									temp_float_values=float_values+number_of_samples*
										number_of_common_devices;
									for (i=number_of_samples;i>0;i--)
									{
										for (j=number_of_common_devices;j>0;j--)
										{
											temp_float_values--;
											*temp_float_values=(float)0;
										}
										for (j=number_of_signals;j>0;j--)
										{
											temp_float_values--;
											float_values--;
											*temp_float_values= *float_values;
										}
									}
									return_code=1;
								}
							} break;
						}
						if (return_code)
						{
							/* add the new signals */
							index=number_of_signals;
							number_of_signals += number_of_common_devices;
							buffer->number_of_signals=number_of_signals;
							maximum_signal_number++;
							frequency=buffer->frequency;
							device=rig->devices;
							i=rig->number_of_devices;
							while (return_code&&(i>0))
							{
								if ((*device)&&(signal=(*device)->signal))
								{
									temp_device=temp_rig->devices;
									j=temp_rig->number_of_devices;
									found=0;
									while (return_code&&(j>0)&&!found)
									{
										if ((*temp_device)&&!strcmp((*device)->description->name,
											(*temp_device)->description->name))
										{
											found=1;
										}
										else
										{
											temp_device++;
											j--;
										}
									}
									if (return_code&&found&&((*temp_device)->signal)&&
										(temp_buffer=get_Device_signal_buffer(*temp_device)))
									{
										while (signal->next)
										{
											signal=signal->next;
										}
										if (signal->next=create_Signal(index,buffer,REJECTED,
											maximum_signal_number))
										{
											/* fill in values */
											switch (buffer->value_type)
											{
												case SHORT_INT_VALUE:
												{
													short_int_values=((buffer->signals).short_int_values)+
														index;
												} break;
												case FLOAT_VALUE:
												{
													float_values=((buffer->signals).float_values)+index;
												} break;
											}
											switch (temp_buffer->value_type)
											{
												case SHORT_INT_VALUE:
												{
													temp_short_int_values=((temp_buffer->signals).
														short_int_values)+((*temp_device)->signal->index);
												} break;
												case FLOAT_VALUE:
												{
													temp_float_values=((temp_buffer->signals).
														float_values)+((*temp_device)->signal->index);
												} break;
											}
											offset=(*device)->channel->offset;
											gain=(*device)->channel->gain;
											temp_offset=(*temp_device)->channel->offset;
											temp_gain=(*temp_device)->channel->gain;
											temp_frequency=temp_buffer->frequency;
											temp_number_of_signals=temp_buffer->number_of_signals;
											temp_number_of_samples=temp_buffer->number_of_samples;
											k=0;
											time=(float)(buffer->times)[0]/frequency;
											l=0;
											temp_time=(float)(temp_buffer->times)[0]/temp_frequency;
											while ((k<buffer->number_of_samples)&&
												((float)(buffer->times)[k]/frequency<temp_time))
											{
												k++;
											}
											while ((k<number_of_samples)&&
												(l<temp_number_of_samples-1))
											{
												time=(float)(buffer->times)[k]/frequency;
												while ((l<temp_number_of_samples-1)&&
													((float)(temp_buffer->times)[l+1]/temp_frequency<
													time))
												{
													l++;
												}
												if (l<temp_number_of_samples-1)
												{
													temp_time=(float)(temp_buffer->times)[l]/
														temp_frequency;
													temp_time2=(float)(temp_buffer->times)[l+1]/
														temp_frequency;
													switch (buffer->value_type)
													{
														case SHORT_INT_VALUE:
														{
															switch (temp_buffer->value_type)
															{
																case SHORT_INT_VALUE:
																{
																	short_int_values[k*number_of_signals]=
																		(short int)(offset+(temp_gain/gain)*
																		(((temp_time2-time)*
																		(float)temp_short_int_values[l*
																		temp_number_of_signals]+(time-temp_time)*
																		(float)temp_short_int_values[(l+1)*
																		temp_number_of_signals])/
																		(temp_time2-temp_time)-temp_offset));
																} break;
																case FLOAT_VALUE:
																{
																	short_int_values[k*number_of_signals]=
																		(short int)(offset+(temp_gain/gain)*
																		(((temp_time2-time)*temp_float_values[l*
																		temp_number_of_signals]+(time-temp_time)*
																		temp_float_values[(l+1)*
																		temp_number_of_signals])/
																		(temp_time2-temp_time)-temp_offset));
																} break;
															}
														} break;
														case FLOAT_VALUE:
														{
															switch (temp_buffer->value_type)
															{
																case SHORT_INT_VALUE:
																{
																	float_values[k*number_of_signals]=offset+
																		(temp_gain/gain)*(((temp_time2-time)*
																		(float)temp_short_int_values[l*
																		temp_number_of_signals]+(time-temp_time)*
																		(float)temp_short_int_values[(l+1)*
																		temp_number_of_signals])/
																		(temp_time2-temp_time)-temp_offset);
																} break;
																case FLOAT_VALUE:
																{
																	float_values[k*number_of_signals]=offset+
																		(temp_gain/gain)*(((temp_time2-time)*
																		temp_float_values[l*
																		temp_number_of_signals]+(time-temp_time)*
																		temp_float_values[(l+1)*
																		temp_number_of_signals])/
																		(temp_time2-temp_time)-temp_offset);
																} break;
															}
														} break;
													}
												}
												k++;
											}
											index++;
										}
										else
										{
											display_message(ERROR_MESSAGE,"overlay_rig.  "
                        "Could not create overlay signal");
											return_code=0;
										}
									}
								}
								device++;
								i--;
							}
							if (!return_code)
							{
								display_message(ERROR_MESSAGE,"overlay_rig.  "
									"Error merging signals");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"overlay_rig.  "
								"Could not extend buffer");
						}
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,"No common devices");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"overlay_rig"
					"Missing overlay rig");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"overlay_rig.  "
				"Missing rig");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"overlay_rig.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* overlay_rig() */

static int merge_rigs(struct Analysis_work_area *analysis,
  struct Rig **rigs,int number_of_rigs)
/*******************************************************************************
LAST MODIFIED : 13 November 2000

DESCRIPTION :
Overlays all the devices/signals from all the <rigs> onto the <analysis> rig.
==============================================================================*/
{
  int i,return_code;
  
  ENTER(merge_rigs);
  if (analysis)
  {
    if ((number_of_rigs > 0) && rigs)
    {
      /* Need to merge ("overlay") the provided rigs */
      return_code = 1;
      for (i=0;(i<number_of_rigs)&&return_code;i++)
      {
        /* overlay each of the rigs onto the one in the analysis work area */
        return_code = overlay_rig(analysis,rigs[i]);
      }
    }
    else
    {
      /* do nothing */
      return_code = 1;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"merge_rigs.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* merge_rigs() */

/*
Global functions
----------------
*/
struct Cell_unemap_interface *CREATE(Cell_unemap_interface)(
  Widget cell_interface_shell,Widget cell_interface_window,
  struct Time_keeper *time_keeper,
  struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 31 October 2000

DESCRIPTION :
Creates a Cell_unemap_interface object.
==============================================================================*/
{
  struct Cell_unemap_interface *cell_unemap_interface;
  static MrmRegisterArg callback_list[]={
    {"associate_mapping_analysis",(XtPointer)associate_mapping_analysis}
  };
  static MrmRegisterArg identifier_list[]= {
    {"system_window_structure",(XtPointer)NULL}
  };
#define XmNanalysisColour "analysisColour"
#define XmCAnalysisColour "AnalysisColour"
  static XtResource resources[] = {
    {
      XmNanalysisColour,
			XmCAnalysisColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(User_settings,analysis_colour),
			XmRString,
			"aquamarine"
    }
  }; /* resources */
  User_settings *user_settings = (User_settings *)NULL;

  ENTER(CREATE(Cell_unemap_interface));
  if (cell_interface_shell && cell_interface_window && time_keeper &&
    user_interface)
  {
    if (ALLOCATE(cell_unemap_interface,struct Cell_unemap_interface,1))
    {
      /* Initialise data objects */
      cell_unemap_interface->analysis_work_area =
        (struct Analysis_work_area *)NULL;
      cell_unemap_interface->saved_rigs = (struct Rig **)NULL;
      cell_unemap_interface->number_of_saved_rigs = 0;
      cell_unemap_interface->save_signals = 0;
      /* Create the data objects */
      if (ALLOCATE(cell_unemap_interface->analysis_work_area,
        struct Analysis_work_area,1))
      {
        /* ?? need to intialise the analysis work area - should be a proper
         * ?? CREATE(Analysis_work_area)() ??
         */
        cell_unemap_interface->analysis_work_area->activation = (Widget)NULL;
        cell_unemap_interface->analysis_work_area->window_shell = (Widget)NULL;
        cell_unemap_interface->analysis_work_area->window =
          (struct Analysis_window *)NULL;
        cell_unemap_interface->analysis_work_area->trace =
          (struct Trace_window *)NULL;
        cell_unemap_interface->analysis_work_area->mapping_work_area =
          (struct Mapping_work_area *)NULL;
        cell_unemap_interface->analysis_work_area->mapping_window =
          (struct Mapping_window *)NULL;
        cell_unemap_interface->analysis_work_area->raw_rig =
          (struct Rig *)NULL;
        cell_unemap_interface->analysis_work_area->rig =
          (struct Rig *)NULL;
        cell_unemap_interface->analysis_work_area->signal_drawing_package =
          (struct Signal_drawing_package *)NULL;
        cell_unemap_interface->analysis_work_area->highlight =
          (struct Device **)NULL;
        cell_unemap_interface->analysis_work_area->signal_drawing_information =
          (struct Signal_drawing_information *)NULL;
        cell_unemap_interface->analysis_work_area->map_drawing_information =
          (struct Map_drawing_information *)NULL;
        cell_unemap_interface->analysis_work_area->user_interface =
          (struct User_interface *)NULL;
        cell_unemap_interface->analysis_work_area->potential_time_object =
          (struct Time_object *)NULL;
        cell_unemap_interface->analysis_work_area->time_keeper =
          (struct Time_keeper *)NULL;
        cell_unemap_interface->analysis_work_area->datum_time_object =
          (struct Time_object *)NULL;
        cell_unemap_interface->analysis_work_area->unemap_package =
          (struct Unemap_package *)NULL;
        cell_unemap_interface->analysis_work_area->datum = 0;
				cell_unemap_interface->analysis_work_area->objective = ABSOLUTE_SLOPE;
				cell_unemap_interface->analysis_work_area->detection = EDA_INTERVAL;
				cell_unemap_interface->analysis_work_area->datum_type = AUTOMATIC_DATUM;
				cell_unemap_interface->analysis_work_area->edit_order = DEVICE_ORDER;
				cell_unemap_interface->analysis_work_area->signal_order = CHANNEL_ORDER;
				cell_unemap_interface->analysis_work_area->calculate_events = 0;
				cell_unemap_interface->analysis_work_area->number_of_events = 1;
				cell_unemap_interface->analysis_work_area->event_number = 1;
				cell_unemap_interface->analysis_work_area->threshold = 90;
				cell_unemap_interface->analysis_work_area->minimum_separation = 100;
				cell_unemap_interface->analysis_work_area->average_width = 6;
				cell_unemap_interface->analysis_work_area->level = 0;
				cell_unemap_interface->analysis_work_area->map_type = NO_MAP_FIELD;
				cell_unemap_interface->analysis_work_area->bard_signal_file_data =
          (struct File_open_data *)NULL;
				cell_unemap_interface->analysis_work_area->cardiomapp_signal_file_data =
					(struct File_open_data *)NULL;
				cell_unemap_interface->analysis_work_area->neurosoft_signal_file_data =
					(struct File_open_data *)NULL;
        /* register the callbacks */
        /* the focus callback on the analysis window requires
           the system_window_structure identifier to be registered
           ??? should check if these are already registered (ie. unemap
           system window has been created) ??? */
        if ((MrmSUCCESS==MrmRegisterNames(callback_list,
          XtNumber(callback_list))) && (MrmSUCCESS ==
            MrmRegisterNames(identifier_list,
              XtNumber(identifier_list))))
        {
          /* Get the user settings */
          if (ALLOCATE(user_settings,User_settings,1))
          {
            XtVaGetApplicationResources(cell_interface_window,
              (XtPointer)user_settings,resources,XtNumber(resources),
              NULL);
            if (create_analysis_work_area(
              cell_unemap_interface->analysis_work_area,
              cell_interface_window,cell_interface_shell,
              /*pointer_sensitivity*/2,
              /*signal_file_extension_read*/".sig*",
              /*signal_file_extension_write*/".signal",
              /*postscript_file_extension*/".ps",
              /*configuration_file_extension*/".cnfg",
              user_settings->analysis_colour,
              (struct Map_drawing_information *)NULL,
              user_interface,time_keeper,
              (struct Unemap_package *)NULL))
            {
              DEALLOCATE(user_settings);
            }
            else
            {
              display_message(ERROR_MESSAGE,"CREATE(Cell_unemap_interface).  "
                "Unable to create the UnEMAP analysis work area");
              DESTROY(Cell_unemap_interface)(&cell_unemap_interface);
              cell_unemap_interface = (struct Cell_unemap_interface *)NULL;
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,"CREATE(Cell_unemap_interface).  "
              "Unable to allocate memory for the UnEMAP user settings");
            DESTROY(Cell_unemap_interface)(&cell_unemap_interface);
            cell_unemap_interface = (struct Cell_unemap_interface *)NULL;
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"CREATE(Cell_unemap_interface).  "
            "Unable to register the callbacks and/or identifiers");
          DESTROY(Cell_unemap_interface)(&cell_unemap_interface);
          cell_unemap_interface = (struct Cell_unemap_interface *)NULL;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"CREATE(Cell_unemap_interface).  "
          "Unable to allocate memory for the analysis work area object");
        DESTROY(Cell_unemap_interface)(&cell_unemap_interface);
        cell_unemap_interface = (struct Cell_unemap_interface *)NULL;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"CREATE(Cell_unemap_interface).  "
        "Unable to allocate memory for the Cell UnEMAP interface object");
      cell_unemap_interface = (struct Cell_unemap_interface *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Cell_unemap_interface).  "
      "Invalid argument(s)");
    cell_unemap_interface = (struct Cell_unemap_interface *)NULL;
  }
  LEAVE;
  return(cell_unemap_interface);
} /* CREATE(Cell_unemap_interface) */

int DESTROY(Cell_unemap_interface)(
  struct Cell_unemap_interface **cell_unemap_interface_address)
/*******************************************************************************
LAST MODIFIED : 19 June 2001

DESCRIPTION :
Destroys a Cell_unemap_interface object.
==============================================================================*/
{
	int return_code;
  struct Cell_unemap_interface *cell_unemap_interface;

	ENTER(DESTROY(Cell_unemap_interface));
	if (cell_unemap_interface_address &&
    (cell_unemap_interface = *cell_unemap_interface_address))
	{
    /* Make sure everything is cleared first */
    Cell_unemap_interface_clear_analysis_work_area(cell_unemap_interface);
    /* Then destroy the UnEmap analysis work area */
    if (cell_unemap_interface->analysis_work_area)
    {
      close_analysis_work_area((Widget)NULL,
        (XtPointer)(cell_unemap_interface->analysis_work_area),(XtPointer)NULL);
      destroy_analysis_work_area(cell_unemap_interface->analysis_work_area);
      DEALLOCATE(cell_unemap_interface->analysis_work_area);
    }
    /* And get rid of the saved rigs */
    if (cell_unemap_interface->saved_rigs)
    {
      /* All the actual rigs should already be destroyed... */
      DEALLOCATE(cell_unemap_interface->saved_rigs);
    }
    DEALLOCATE(*cell_unemap_interface_address);
    *cell_unemap_interface_address = (struct Cell_unemap_interface *)NULL;
    return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cell_unemap_interface).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* DESTROY(Cell_unemap_interface) */

int Cell_unemap_interface_close(
  struct Cell_unemap_interface *cell_unemap_interface)
/*******************************************************************************
LAST MODIFIED : 05 November 2000

DESCRIPTION :
Closes a Cell_unemap_interface object.
==============================================================================*/
{
	int return_code;

	ENTER(Cell_unemap_interface_close);
	if (cell_unemap_interface)
	{
    if (cell_unemap_interface->analysis_work_area)
    {
      /* need to destroy any existing rig to force the devices to be re-created
         for the new model */
      Cell_unemap_interface_clear_analysis_work_area(cell_unemap_interface);
    }
    return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cell_unemap_interface_close.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* Cell_unemap_interface_close() */

int Cell_unemap_interface_pop_up_analysis_window(
  struct Cell_unemap_interface *cell_unemap_interface)
/*******************************************************************************
LAST MODIFIED : 01 November 2000

DESCRIPTION :
Pops up the UnEmap analysis window.
==============================================================================*/
{
	int return_code;

	ENTER(Cell_unemap_interface_pop_up_analysis_window);
	if (cell_unemap_interface)
	{
    if (cell_unemap_interface->analysis_work_area &&
      cell_unemap_interface->analysis_work_area->window_shell)
    {
      XtPopup(cell_unemap_interface->analysis_work_area->window_shell,
        XtGrabNone);
      if (cell_unemap_interface->analysis_work_area->trace &&
        cell_unemap_interface->analysis_work_area->trace->shell)
      {
        XtPopup(cell_unemap_interface->analysis_work_area->trace->shell,
          XtGrabNone);
      }
      /* Ghost the analysis window activation widget */
      XtSetSensitive(cell_unemap_interface->analysis_work_area->activation,
        False);
      return_code=1;
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "Cell_unemap_interface_pop_up_analysis_window.  "
        "Missing analysis work area window shell");
      return_code=0;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,
      "Cell_unemap_interface_pop_up_analysis_window.  "
      "Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* Cell_unemap_interface_pop_up_analysis_window() */

int Cell_unemap_interface_pop_down_analysis_window(
  struct Cell_unemap_interface *cell_unemap_interface)
/*******************************************************************************
LAST MODIFIED : 22 January 2001

DESCRIPTION :
Pops down the UnEmap analysis window.
==============================================================================*/
{
	int return_code;

	ENTER(Cell_unemap_interface_pop_down_analysis_window);
	if (cell_unemap_interface)
	{
    if (cell_unemap_interface->analysis_work_area &&
      cell_unemap_interface->analysis_work_area->window_shell)
    {
      XtPopdown(cell_unemap_interface->analysis_work_area->window_shell);
      if (cell_unemap_interface->analysis_work_area->trace &&
        cell_unemap_interface->analysis_work_area->trace->shell)
      {
        XtPopdown(cell_unemap_interface->analysis_work_area->trace->shell);
      }
      /* Unghost the analysis window activation widget */
      XtSetSensitive(cell_unemap_interface->analysis_work_area->activation,
        True);
      return_code = 1;
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "Cell_unemap_interface_pop_down_analysis_window.  "
        "Missing analysis work area window shell");
      return_code = 0;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,
      "Cell_unemap_interface_pop_down_analysis_window.  "
      "Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return(return_code);
} /* Cell_unemap_interface_pop_down_analysis_window() */

int Cell_unemap_interface_add_signals(
  struct Cell_unemap_interface *cell_unemap_interface,int number_of_signals,
  struct Cell_variable_unemap_interface **variable_unemap_interfaces,
  float tabT)
/*******************************************************************************
LAST MODIFIED : 1 November 2001

DESCRIPTION :
Adds the signals defined by the array of <variable_unemap_interfaces> to the
analysis work area given in the <cell_unemap_interface>. If the save signals
toggle is set to true, then the new values are added to any existing devices
where matches are found and new devices are created for any missing variables.
If the save signals toggle is set to false, then any existins buffers and
devices are destroyed and new ones are created.
==============================================================================*/
{
  int return_code = 0;
  struct Analysis_work_area *analysis;
  struct Signal_buffer *buffer;
  struct Signal *signal;
  struct Device **devices,**device;
  struct Device_description *description;
  struct Channel *channel;
  struct Map *map;
  struct Region *region;
  struct Region_list_item *region_list;
  float frequency;
  int number_of_samples,i,*time,signal_number,device_number,save_signals;
  int number_of_saved_rigs;
  struct Rig **saved_rigs;
  struct Cell_variable_unemap_interface *cell_variable_unemap_interface;
  char *name;

  ENTER(Cell_unemap_interface_add_signals);
  if (cell_unemap_interface && (number_of_signals > 0) &&
    variable_unemap_interfaces)
  {
    /* Set the save signals flag */
    save_signals = cell_unemap_interface->save_signals;
    /* Grab a pointer to the UnEmap analysis work area */
    if (analysis = cell_unemap_interface->analysis_work_area)
    {
      /* Clear previous calculation(s) if required */
      number_of_saved_rigs = cell_unemap_interface->number_of_saved_rigs;
      saved_rigs = cell_unemap_interface->saved_rigs;
      if (save_signals)
      {
        /* want to save the previous calculation, so increase the size of the
           saved rigs array */
        number_of_saved_rigs++;
        REALLOCATE(saved_rigs,saved_rigs,struct Rig *,number_of_saved_rigs);
      }
      else
      {
        /* want to throw away any previous calculation */
        Cell_unemap_interface_clear_analysis_work_area(cell_unemap_interface);
        /* and create the saved rigs array */
        number_of_saved_rigs = 1;
        ALLOCATE(saved_rigs,struct Rig *,number_of_saved_rigs);
      } /* if (save_signals) ... else ... */
      if (saved_rigs)
      {
        /* Successfully (re)created the saved rigs array, so add the new
           information */
        cell_unemap_interface->number_of_saved_rigs =
          number_of_saved_rigs;
        cell_unemap_interface->saved_rigs = saved_rigs;
        /*
         * Create the signal buffer for the new set of signals
         */
        /* ?? For now, asssume that all the variables have the same number of
           ?? values - which might not be true once we start keeping old
           ?? calculations in the rig
        */
        number_of_samples = Cell_variable_unemap_interface_get_number_of_values(
          variable_unemap_interfaces[0]);
        if (number_of_samples > 0)
        {
          frequency = 1.0 / tabT;
          /* create the buffer */
          if (buffer = create_Signal_buffer(FLOAT_VALUE,number_of_signals,
            number_of_samples,frequency))
          {
            /* Set the time array for the buffer */
            time = buffer->times;
            for (i=0;i<number_of_samples;i++)
            {
              *time = i;
              time++;
            }
            /* Create the new rig and set-up all the required devices - first
             * create the devices for the rig
             */
            if ((ALLOCATE(devices,struct Device *,number_of_signals) &&
              (region = create_Region("cell",PATCH,0,0
#if defined (UNEMAP_USE_3D)
                ,(struct Unemap_package *)NULL
#endif /* defined (UNEMAP_USE_3D) */
                )) && (region_list =
                  create_Region_list_item(region,
                    (struct Region_list_item *)NULL))))
            {
              /* Create the devices */
              device_number = 0;
              device = devices;
              for (signal_number = 0;
                   (signal_number < number_of_signals) && devices;
                   signal_number++)
              {
                cell_variable_unemap_interface =
                  variable_unemap_interfaces[signal_number];
                /* Create the device for this signal (variable) */
                if (name = Cell_variable_unemap_interface_get_name(
                  cell_variable_unemap_interface))
                {
                  if ((description =
                    create_Device_description(name,
                      AUXILIARY,region)) &&
                    (channel = create_Channel(signal_number+1,0,1)) &&
                    (signal = create_Signal(signal_number,buffer,REJECTED,
                      number_of_saved_rigs-1)) &&
                    (*device = create_Device(signal_number,description,channel,
                      signal)))
                  {
                    device++;
                    device_number++;
                  }
                  else
                  {
                    /* Couldn't create the device, so destroy stuff */
                    if (description)
                    {
                      destroy_Device_description(&description);
                      if (channel)
                      {
                        destroy_Channel(&channel);
                        if (signal)
                        {
                          destroy_Signal(&signal);
                        }
                      }
                    }
                    while (device_number>0)
                    {
                      device--;
                      device_number--;
                      destroy_Device(device);
                    }
                    DEALLOCATE(devices);
                    display_message(ERROR_MESSAGE,
                      "Cell_unemap_interface_add_signals.  "
                      "Could not create device");
                  }
                  DEALLOCATE(name);
                }
                else
                {
                  display_message(WARNING_MESSAGE,
                    "Cell_unemap_interface_add_signals.  "
                    "Failed to get the device name");
                }
              } /* for (signal_number = 0 -> number_of_signals) */
            }
            else
            {
              display_message(ERROR_MESSAGE,
                "Cell_unemap_interface_add_signals.  "
                "Unable to allocate memory for the devices, region, or "
                "region list");
              if (devices)
              {
                DEALLOCATE(devices);
                if (region)
                {
                  destroy_Region(&region);
                }
              }
            }
            if (devices)
            {
              /* Now create the new rig */
              if (saved_rigs[number_of_saved_rigs-1] =
                create_Rig("cell",MONITORING_OFF,
                  EXPERIMENT_OFF,number_of_signals,devices,
                  (struct Page_list_item *)NULL,0,
                  region_list,(struct Region *)NULL
#if defined (UNEMAP_USE_3D)
                  ,(struct Unemap_package *)NULL
#endif /* defined (UNEMAP_USE_3D) */
                  ))
              {
                /* Get the values from the variable UnEmap interface's
                 * and put them into the signal buffer
                 */
                for (signal_number=0;signal_number<number_of_signals;
                     signal_number++)
                {
                  for (i=0;i<number_of_samples;i++)
                  {
                    buffer->signals.float_values[signal_number+
                      (i*number_of_signals)] =
                      Cell_variable_unemap_interface_get_value_as_float_at_position(
                        variable_unemap_interfaces[signal_number],i);
                  }
                }
                /* initialise the new analysis */
                analysis->rig = saved_rigs[0];
                if (merge_rigs(analysis,saved_rigs+1,number_of_saved_rigs-1))
                {
                  /* clear the existing analysis work area rig */
                  analysis->datum=0;
                  analysis->potential_time=0;
                  analysis->highlight=(struct Device **)NULL;
                  analysis->map_type=NO_MAP_FIELD;
                  if (analysis->mapping_window)
                  {
                    /*analysis->mapping_window->activation_front= -1;*/
                    XtSetSensitive(
                      analysis->mapping_window->animate_button,False);
                    if (map=analysis->mapping_window->map)
                    {
                      map->colour_option=HIDE_COLOUR;
                      map->contours_option=HIDE_CONTOURS;
                      map->electrodes_label_type=SHOW_ELECTRODE_NAMES;
                      /*map->projection=HAMMER;*/
                    }
                    /* unghost the mapping window file button */
                    XtSetSensitive(analysis->mapping_window->file_button,
                      True);
                  }
                  /* unghost the write interval button */
                  XtSetSensitive(
                    analysis->window->file_menu.save_interval_button,True);
                  /* unghost the write interval as button */
                  XtSetSensitive(
                    analysis->window->file_menu.save_interval_as_button,
                    True);
                  /* unghost the print selected signals button */
                  XtSetSensitive(
                    analysis->window->print_menu.selected_button,True);
                  /* unghost the print all signals button */
                  XtSetSensitive(analysis->window->print_menu.all_button,
                    True);
                  /* unghost the display potential map button */
                  XtSetSensitive(
                    analysis->window->map_menu.potential_button,True);
                  /* higlight the first device */
                  if (analysis->highlight=analysis->rig->devices)
                  {
                    if (buffer=(*(analysis->highlight))->signal->buffer)
                    {
                      /* initialize the search interval */
                      analysis->start_search_interval=buffer->start;
                      analysis->end_search_interval=buffer->end;
                      /* initialize potential time */
                      analysis->potential_time=
                        (buffer->number_of_samples-1)/3;
                      /* initialize datum */
                      analysis->datum=2*(analysis->potential_time);
                    }
                  }
                  XtSetSensitive(
                    analysis->window->map_button,False);
                  return_code = 1;
                }
                else
                {
                  display_message(ERROR_MESSAGE,
                    "Cell_unemap_interface_add_signals.  "
                    "Could not merge the saved rigs");
                  while (device_number>0)
                  {
                    device--;
                    device_number--;
                    destroy_Device(device);
                  }
                  DEALLOCATE(devices);
                  return_code = 0;
                }
              }
              else
              {
                display_message(ERROR_MESSAGE,
                  "Cell_unemap_interface_add_signals.  "
                  "Could not create rig");
                while (device_number>0)
                {
                  device--;
                  device_number--;
                  destroy_Device(device);
                }
                DEALLOCATE(devices);
                return_code = 0;
              }
            }
            else
            {
              display_message(ERROR_MESSAGE,
                "Cell_unemap_interface_add_signals.  "
                "Could not create/get the devices");
              return_code = 0;
              Cell_unemap_interface_clear_analysis_work_area(
                cell_unemap_interface);
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,"Cell_unemap_interface_add_signals.  "
              "Unable to create the signal buffer");
            cell_unemap_interface->number_of_saved_rigs--;
            Cell_unemap_interface_clear_analysis_work_area(
              cell_unemap_interface);
            return_code = 0;
          }
        } /* if (number_of_samples > 0) */
      }
      else
      {
        display_message(ERROR_MESSAGE,"Cell_unemap_interface_add_signals.  "
          "Unable to save the buffer");
        Cell_unemap_interface_clear_analysis_work_area(cell_unemap_interface);
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_unemap_interface_add_signals.  "
        "Missing analysis work area");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_unemap_interface_add_signals.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_unemap_interface_add_signals() */

int Cell_unemap_interface_clear_analysis_work_area(
  struct Cell_unemap_interface *cell_unemap_interface)
/*******************************************************************************
LAST MODIFIED : 13 November 2000

DESCRIPTION :
Clears the analysis work area and destroys all the signals associated with it.
==============================================================================*/
{
  int return_code = 0;
  struct Analysis_work_area *analysis;
  int i,number_of_saved_rigs;
  struct Rig **saved_rigs;
  struct Device *device;
  struct Signal_buffer *buffer;

  ENTER(Cell_unemap_interface_clear_analysis_work_area);
  if (cell_unemap_interface &&
    (analysis = cell_unemap_interface->analysis_work_area))
  {
    /* Clear the saved rigs - Need to keep the first saved rig, as this
     * is the one that the others are overlayed on
     */
    number_of_saved_rigs = cell_unemap_interface->number_of_saved_rigs;
    saved_rigs = cell_unemap_interface->saved_rigs;
    for (i=1;i<number_of_saved_rigs;i++)
    {
      destroy_Rig(saved_rigs+i);
    }
    DEALLOCATE(saved_rigs);
    cell_unemap_interface->number_of_saved_rigs = 0;
    cell_unemap_interface->saved_rigs = (struct Rig **)NULL;
    if (analysis->rig)
    {
      if ((device = *(analysis->rig->devices)) &&
        (buffer = get_Device_signal_buffer(device)))
      {
        destroy_Signal_buffer(&buffer);
      }
      destroy_Rig(&(analysis->rig));
      analysis->rig = (struct Rig *)NULL;
      analysis->datum=0;
      analysis->potential_time=0;
      analysis->highlight=(struct Device **)NULL;
      analysis->map_type=NO_MAP_FIELD;
      update_signals_drawing_area(analysis->window);
      update_interval_drawing_area(analysis->window);
      trace_change_rig(analysis->trace);
    }
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_unemap_interface_clear_analysis_work_area.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_unemap_interface_clear_analysis_work_area() */

int Cell_unemap_interface_update_analysis_work_area(
  struct Cell_unemap_interface *cell_unemap_interface)
/*******************************************************************************
LAST MODIFIED : 05 November 2000

DESCRIPTION :
updates the analysis work area widgets.
==============================================================================*/
{
  int return_code = 0;
  struct Analysis_work_area *analysis;

  ENTER(Cell_unemap_interface_update_analysis_work_area);
  if (cell_unemap_interface &&
    (analysis = cell_unemap_interface->analysis_work_area))
  {
    if (analysis->rig)
    {
      update_signals_drawing_area(analysis->window);
      update_interval_drawing_area(analysis->window);
      trace_change_rig(analysis->trace);
      return_code = Cell_unemap_interface_pop_up_analysis_window(
        cell_unemap_interface);
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "Cell_unemap_interface_update_analysis_work_area.  "
        "Missing rig");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_unemap_interface_update_analysis_work_area.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_unemap_interface_update_analysis_work_area() */

int Cell_unemap_interface_set_save_signals(
  struct Cell_unemap_interface *cell_unemap_interface,int save)
/*******************************************************************************
LAST MODIFIED : 05 November 2000

DESCRIPTION :
Sets the save signals toggle in the <cell_unemap_interface> object.
==============================================================================*/
{
  int return_code = 0;
  
  ENTER(Cell_unemap_interface_update_set_save_signals);
  if (cell_unemap_interface)
  {
    cell_unemap_interface->save_signals = save;
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_unemap_interface_set_save_signals.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_unemap_interface_update_set_save_signals() */

int Cell_unemap_interface_get_save_signals(
  struct Cell_unemap_interface *cell_unemap_interface)
/*******************************************************************************
LAST MODIFIED : 03 April 2001

DESCRIPTION :
Gets the save signals toggle in the <cell_unemap_interface> object.
==============================================================================*/
{
  int save = 0;
  
  ENTER(Cell_unemap_interface_update_get_save_signals);
  if (cell_unemap_interface)
  {
    save = cell_unemap_interface->save_signals;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_unemap_interface_get_save_signals.  "
      "Invalid argument(s)");
    save = -1;
  }
  LEAVE;
  return(save);
} /* Cell_unemap_interface_update_get_save_signals() */

int Cell_unemap_interface_check_analysis_window(
  struct Cell_unemap_interface *cell_unemap_interface,Widget parent,
  struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 07 May 2001

DESCRIPTION :
Checks the analysis work area for a analysis window and if one is not found try
to create one. <parent> is used as the parent widget if the window needs to be
created.
==============================================================================*/
{
  int return_code = 0;
  struct Analysis_work_area *analysis;
  Widget activation;
  struct Time_keeper *time_keeper;
  Pixel identifying_colour;
  
  ENTER(Cell_unemap_interface_check_analysis_window);
  if (cell_unemap_interface && parent && user_interface)
  {
    if (analysis = cell_unemap_interface->analysis_work_area)
    {
      if (analysis->window_shell && analysis->window)
      {
        /* The analysis window exists */
        return_code = 1;
      }
      else
      {
        /* There is no analysis window, it must have been destroyed  - so
           clear up the rest of the analysis work area and start again */
        close_analysis_work_area((Widget)NULL,(XtPointer)(analysis),
          (XtPointer)NULL);
        /* Re-initialise the fields ?? */
        /* save the current activation widget and time keeper and identifying
           colour */
        activation = analysis->activation;
        time_keeper = analysis->time_keeper;
        identifying_colour = analysis->identifying_colour;
        analysis->activation = (Widget)NULL;
        analysis->window_shell = (Widget)NULL;
        analysis->window = (struct Analysis_window *)NULL;
        analysis->trace = (struct Trace_window *)NULL;
        analysis->mapping_work_area = (struct Mapping_work_area *)NULL;
        analysis->mapping_window = (struct Mapping_window *)NULL;
        analysis->raw_rig = (struct Rig *)NULL;
        analysis->rig = (struct Rig *)NULL;
        analysis->signal_drawing_package =
          (struct Signal_drawing_package *)NULL;
        analysis->highlight = (struct Device **)NULL;
        analysis->signal_drawing_information =
          (struct Signal_drawing_information *)NULL;
        analysis->map_drawing_information =
          (struct Map_drawing_information *)NULL;
        analysis->user_interface = (struct User_interface *)NULL;
        analysis->potential_time_object = (struct Time_object *)NULL;
        analysis->time_keeper = (struct Time_keeper *)NULL;
        analysis->datum_time_object = (struct Time_object *)NULL;
        analysis->unemap_package = (struct Unemap_package *)NULL;
        analysis->datum = 0;
				analysis->objective = ABSOLUTE_SLOPE;
				analysis->detection = EDA_INTERVAL;
				analysis->datum_type = AUTOMATIC_DATUM;
				analysis->edit_order = DEVICE_ORDER;
				analysis->signal_order = CHANNEL_ORDER;
				analysis->calculate_events = 0;
				analysis->number_of_events = 1;
				analysis->event_number = 1;
				analysis->threshold = 90;
				analysis->minimum_separation = 100;
				analysis->average_width = 6;
				analysis->level = 0;
				analysis->map_type = NO_MAP_FIELD;
				analysis->bard_signal_file_data = (struct File_open_data *)NULL;
				analysis->cardiomapp_signal_file_data = (struct File_open_data *)NULL;
				analysis->neurosoft_signal_file_data = (struct File_open_data *)NULL;
        /* and re-create the analysis work area */
        if (create_analysis_work_area(
          analysis,activation,parent,
          /*pointer_sensitivity*/2,
          /*signal_file_extension_read*/".sig*",
          /*signal_file_extension_write*/".signal",
          /*postscript_file_extension*/".ps",
          /*configuration_file_extension*/".cnfg",
          identifying_colour,
          (struct Map_drawing_information *)NULL,
          user_interface,time_keeper,
          (struct Unemap_package *)NULL))
        {
          return_code = 1;
        }
        else
        {
          display_message(ERROR_MESSAGE,
            "Cell_unemap_interface_check_analysis_window.  "
            "Unable to re-create the analysis work area");
          return_code = 0;
        }
      }
    }
    else
    {
      display_message(WARNING_MESSAGE,
        "Cell_unemap_interface_check_analysis_window.  "
        "Missing UnEMAP interface");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_unemap_interface_check_analysis_window.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_unemap_interface_update_check_analysis_window() */

