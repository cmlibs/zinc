/*******************************************************************************
FILE : mapping_window.c

LAST MODIFIED : 29 June 2000

DESCRIPTION :
???DB.  Missing settings ?
==============================================================================*/
#include <stddef.h>
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
#include <Xm/ToggleBG.h>
#include <Xm/TextF.h>
#include <Mrm/MrmPublic.h>
#include <Mrm/MrmDecls.h>
#endif /* defined (MOTIF) */
#include "general/debug.h"
#include "general/image_utilities.h"
#include "general/postscript.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "time/time.h"
#include "time/time_keeper.h"
#include "time/time_editor.h"
#include "time/time_editor_dialog.h"
#include "unemap/bard.h"
#include "unemap/drawing_2d.h"
#include "unemap/map_dialog.h"
#include "unemap/mapping.h"
#include "unemap/mapping_window.h"
#include "unemap/mapping_window.uidh"
#include "unemap/rig.h"
#if defined (UNEMAP_USE_NODES)
#include "unemap/rig_node.h"
#endif /* defined (UNEMAP_USE_NODES) */
#include "unemap/setup_dialog.h"
#include "user_interface/filedir.h"
#include "user_interface/message.h"
#include "user_interface/printer.h"
#include "user_interface/user_interface.h"

/*
Module constants
----------------
*/
#define MAX_SPECTRUM_COLOURS 256

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int mapping_window_hierarchy_open=0;
static MrmHierarchy mapping_window_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
static void identify_mapping_menu(Widget *widget_id,XtPointer mapping_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 December 1996

DESCRIPTION :
Finds the id of the mapping menu.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_menu);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
		mapping->menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_menu.  mapping_window missing");
	}
	LEAVE;
} /* identify_mapping_menu */

static void identify_mapping_map_button(Widget *widget_id,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 December 1996

DESCRIPTION :
Finds the id of the mapping map button.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_map_button);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
		mapping->map_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_map_button.  mapping_window missing");
	}
	LEAVE;
} /* identify_mapping_map_button */

static void mapping_window_update_time_limits(struct Mapping_window *mapping)
/*******************************************************************************
LAST MODIFIED : 30 April 1999

DESCRIPTION :
Sets the minimum and maximum of the timekeeper to relate to the current map
==============================================================================*/
{
	struct Map *map;
	struct Signal_buffer *buffer;

	ENTER(mapping_window_update_time_limits);
	if (mapping)
	{
		if (mapping->potential_time_object&&(map=mapping->map)&&
			(buffer=(*((*(map->rig_pointer))->devices))->signal->buffer))
		{
			switch (*mapping->map->type)
			{
				case POTENTIAL:
				{
					if ((NO_INTERPOLATION!=map->interpolation_type) &&
						(map->frame_end_time > map->frame_start_time))
					{
						Time_keeper_set_minimum(Time_object_get_time_keeper(
							mapping->potential_time_object), map->frame_start_time);
						Time_keeper_set_maximum(Time_object_get_time_keeper(
							mapping->potential_time_object), map->frame_end_time);
					}
					else
					{
						Time_keeper_set_minimum(Time_object_get_time_keeper(
							mapping->potential_time_object),
							(float)buffer->times[0] * 1000.0 / buffer->frequency);
						Time_keeper_set_maximum(Time_object_get_time_keeper(
							mapping->potential_time_object),
							(float)buffer->times[buffer->number_of_samples - 1]
							* 1000.0 / buffer->frequency);
					}
				} break;
				case SINGLE_ACTIVATION:
				{
					Time_keeper_set_minimum(Time_object_get_time_keeper(
						mapping->potential_time_object),
						map->minimum_value + (float)buffer->times[*(map->datum)]
						* 1000.0 / buffer->frequency);
					Time_keeper_set_maximum(Time_object_get_time_keeper(
						mapping->potential_time_object),
						map->maximum_value + (float)buffer->times[*(map->datum)]
						* 1000.0 / buffer->frequency );
				} break;
				default:
				{
					Time_keeper_set_minimum(Time_object_get_time_keeper(
						mapping->potential_time_object),
						(float)buffer->times[0] * 1000.0 / buffer->frequency);
					Time_keeper_set_maximum(Time_object_get_time_keeper(
						mapping->potential_time_object),
						(float)buffer->times[buffer->number_of_samples - 1]
						* 1000.0 / buffer->frequency);
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"mapping_window_update_time_limits.  Invalid_argurment(s)");
	}
	LEAVE;
} /* mapping_window_update_time_limits */

static void update_map_from_dialog(Widget widget,XtPointer mapping_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
Updates the map settings based on the map dialog and redraws the map if
necessary.
==============================================================================*/
{
	char map_settings_changed,*value_string,temp_string[20];
	enum Contour_thickness contour_thickness;
	enum Electrodes_marker_type electrodes_marker_type;
	enum Electrodes_option electrodes_option;
	enum Fibres_option fibres_option;
	enum Interpolation_type interpolation_type;
	float buffer_end_time,buffer_start_time,frame_end_time,frame_start_time,
		maximum_range,minimum_range,value;
	int electrodes_marker_size,frame_number,i,number_of_frames,
		number_of_mesh_columns,number_of_mesh_rows,recalculate;
	struct Map *map;
	struct Map_dialog *map_dialog;
	struct Map_drawing_information *drawing_information;
	struct Map_frame *frame;
	struct Mapping_window *mapping;
	struct Signal_buffer *buffer;
	Widget option_widget;
	struct Spectrum *spectrum=(struct Spectrum *)NULL;
	struct Spectrum *spectrum_to_be_modified_copy=(struct Spectrum *)NULL;
	struct MANAGER(Spectrum) *spectrum_manager=(struct MANAGER(Spectrum) *)NULL;

	ENTER(update_map_from_dialog);
	USE_PARAMETER(call_data);
#if !defined (UNEMAP_USE_NODES)
	USE_PARAMETER(spectrum_manager);
#endif/* defined (UNEMAP_USE_NODES)*/
	if ((mapping=(struct Mapping_window *)mapping_window)&&(map=mapping->map)&&
		(map_dialog=mapping->map_dialog)&&(drawing_information=map->drawing_information))
	{
		map_settings_changed=0;
		recalculate=0;
		XtVaGetValues(map_dialog->range.type_option_menu,
			XmNmenuHistory,&option_widget,
			NULL);
		if (option_widget==map_dialog->range.type_option.fixed)
		{
			map->fixed_range=1;
		}
		else
		{
			map->fixed_range=0;
		}
		XtVaGetValues(map_dialog->spectrum.type_option_menu,
			XmNmenuHistory,&option_widget,
			NULL);
		if (option_widget==map_dialog->spectrum.type_option.none)
		{
			if (map->colour_option!=HIDE_COLOUR)
			{				
				map_settings_changed=1;
				map->colour_option=HIDE_COLOUR;				
			}
		}
		else
		{
			if (map->colour_option!=SHOW_COLOUR)
			{
				map_settings_changed=1;
				map->colour_option=SHOW_COLOUR;
			}
			spectrum=drawing_information->spectrum;
#if defined (UNEMAP_USE_NODES)				
			if(spectrum_manager=get_map_drawing_information_spectrum_manager
				(drawing_information))
			{
				if (IS_MANAGED(Spectrum)(spectrum,spectrum_manager))
				{
					if (spectrum_to_be_modified_copy=CREATE(Spectrum)
						("spectrum_modify_temp"))
					{
						MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name)
							(spectrum_to_be_modified_copy,spectrum);
#else
						spectrum_to_be_modified_copy=spectrum;
#endif /* defined (UNEMAP_USE_NODES) */
						if (option_widget==map_dialog->spectrum.type_option.blue_red)
						{
							if (BLUE_TO_RED_SPECTRUM != Spectrum_get_simple_type(
								spectrum_to_be_modified_copy))
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									BLUE_TO_RED_SPECTRUM);
								map_settings_changed = 1;						
							}
						}
						else if (option_widget==map_dialog->spectrum.type_option.blue_white_red)
						{
							if (BLUE_WHITE_RED_SPECTRUM != Spectrum_get_simple_type(
								spectrum_to_be_modified_copy))
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									BLUE_WHITE_RED_SPECTRUM);
								map_settings_changed = 1;					
							}
						}
						else if (option_widget==map_dialog->spectrum.type_option.log_blue_red)
						{
							if (LOG_BLUE_TO_RED_SPECTRUM != Spectrum_get_simple_type(
								spectrum_to_be_modified_copy))
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									LOG_BLUE_TO_RED_SPECTRUM);
								map_settings_changed = 1;					
							}
						}
						else if (option_widget==map_dialog->spectrum.type_option.log_red_blue)
						{
							if (LOG_RED_TO_BLUE_SPECTRUM != Spectrum_get_simple_type(
								spectrum_to_be_modified_copy))
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									LOG_RED_TO_BLUE_SPECTRUM);
								map_settings_changed = 1;						
							}
						}
						else
						{
							if (RED_TO_BLUE_SPECTRUM != Spectrum_get_simple_type(
								spectrum_to_be_modified_copy))
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									RED_TO_BLUE_SPECTRUM);
								map_settings_changed = 1;					
							}
						}

#if defined (UNEMAP_USE_NODES)	
						MANAGER_MODIFY_NOT_IDENTIFIER(Spectrum,name)(spectrum,
							spectrum_to_be_modified_copy,spectrum_manager);
						DESTROY(Spectrum)(&spectrum_to_be_modified_copy);					
					}
					else
					{
						display_message(ERROR_MESSAGE,
							" update_map_from_dialog. Could not create spectrum copy.");				
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						" update_map_from_dialog. Spectrum is not in manager!");		
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					" update_map_from_dialog. Spectrum_manager not present");
			}
#endif /* defined (UNEMAP_USE_NODES) */		
		}
		XtVaGetValues(map_dialog->interpolation.option_menu,
			XmNmenuHistory,&option_widget,
			NULL);
		if (option_widget==map_dialog->interpolation.option.none)
		{
			interpolation_type=NO_INTERPOLATION;
		}
		else
		{
			if (option_widget==map_dialog->interpolation.option.bicubic)
			{
				interpolation_type=BICUBIC_INTERPOLATION;
			}
		}
		if (map->interpolation_type!=interpolation_type)
		{
			map->interpolation_type=interpolation_type;
			map_settings_changed=1;
			if (recalculate<2)
			{
				recalculate=2;
			}
		}
		value_string=(char *)NULL;
		XtVaGetValues((map_dialog->interpolation.mesh_rows_text),
			XmNvalue,&value_string,
			NULL);
		if (1==sscanf(value_string,"%d",&number_of_mesh_rows))
		{
			if (number_of_mesh_rows<1)
			{
				number_of_mesh_rows=1;
			}
		}
		else
		{
			number_of_mesh_rows=map_dialog->number_of_mesh_rows;
		}
		XtFree(value_string);
		if (map->finite_element_mesh_rows!=number_of_mesh_rows)
		{
			map->finite_element_mesh_rows=number_of_mesh_rows;
			if (NO_INTERPOLATION!=map->interpolation_type)
			{
				map_settings_changed=1;
				if (recalculate<2)
				{
					recalculate=2;
				}
			}
		}
		value_string=(char *)NULL;
		XtVaGetValues((map_dialog->interpolation.mesh_columns_text),
			XmNvalue,&value_string,
			NULL);
		if (1==sscanf(value_string,"%d",&number_of_mesh_columns))
		{
			if (number_of_mesh_columns<1)
			{
				number_of_mesh_columns=1;
			}
		}
		else
		{
			number_of_mesh_columns=map_dialog->number_of_mesh_columns;
		}
		XtFree(value_string);
		if (map->finite_element_mesh_columns!=number_of_mesh_columns)
		{
			map->finite_element_mesh_columns=number_of_mesh_columns;
			if (NO_INTERPOLATION!=map->interpolation_type)
			{
				map_settings_changed=1;
				if (recalculate<2)
				{
					recalculate=2;
				}
			}
		}
		XtVaGetValues(map_dialog->contours.type_option_menu,
			XmNmenuHistory,&option_widget,
			NULL);
		if (option_widget==map_dialog->contours.type_option.none)
		{
			if (map->contours_option!=HIDE_CONTOURS)
			{
				map_settings_changed=1;
				map->contours_option=HIDE_CONTOURS;
			}
		}
		else
		{
			if (map->contours_option!=SHOW_CONTOURS)
			{
				map_settings_changed=1;
				map->contours_option=SHOW_CONTOURS;
			}
			if (option_widget==map_dialog->contours.type_option.constant_thickness)
			{
				contour_thickness=CONSTANT_THICKNESS;
			}
			else
			{
				if (option_widget==map_dialog->contours.type_option.variable_thickness)
				{
					contour_thickness=VARIABLE_THICKNESS;
				}
			}
			if (map->contour_thickness!=contour_thickness)
			{
				map->contour_thickness=contour_thickness;
				map_settings_changed=1;
			}
		}
		value_string=(char *)NULL;
		XtVaGetValues((map_dialog->range.maximum_value),
			XmNvalue,&value_string,
			NULL);
		if (1!=sscanf(value_string,"%f",&maximum_range))
		{
			maximum_range=map_dialog->range_maximum;
		}
		XtFree(value_string);
		value_string=(char *)NULL;
		XtVaGetValues((map_dialog->range.minimum_value),
			XmNvalue,&value_string,
			NULL);
		if (1!=sscanf(value_string,"%f",&minimum_range))
		{
			minimum_range=map_dialog->range_minimum;
		}
		XtFree(value_string);
		if (minimum_range>maximum_range)
		{
			value=minimum_range;
			minimum_range=maximum_range;
			maximum_range=value;
		}
		if (minimum_range!=map_dialog->range_minimum)
		{
			map->range_changed=1;
			map->minimum_value=minimum_range;
			map->contour_minimum=minimum_range;
			map_dialog->range_minimum=minimum_range;
			if (minimum_range>map->contour_maximum)
			{
				map->contour_maximum=maximum_range;
			}
			map_settings_changed=1;
			if (recalculate<1)
			{
				recalculate=1;
			}
		}
		if (maximum_range!=map_dialog->range_maximum)
		{
			map->range_changed=1;
			map->maximum_value=maximum_range;
			map->contour_maximum=maximum_range;
			map_dialog->range_maximum=maximum_range;
			if (maximum_range<map->contour_minimum)
			{
				map->contour_minimum=minimum_range;
			}
			map_settings_changed=1;
			if (recalculate<1)
			{
				recalculate=1;
			}
		}
		if (map->number_of_contours!=map_dialog->number_of_contours)
		{
			map->number_of_contours=map_dialog->number_of_contours;
			if (SHOW_CONTOURS==map->contours_option)
			{
				map_settings_changed=1;
			}
		}
		XtVaGetValues(map_dialog->electrodes.option_menu,
			XmNmenuHistory,&option_widget,
			NULL);
		if (option_widget==map_dialog->electrodes.option.name)
		{
			electrodes_option=SHOW_ELECTRODE_NAMES;
		}
		else
		{
			if (option_widget==map_dialog->electrodes.option.value)
			{
				electrodes_option=SHOW_ELECTRODE_VALUES;
			}
			else
			{
				if (option_widget==map_dialog->electrodes.option.channel)
				{
					electrodes_option=SHOW_CHANNEL_NUMBERS;
				}
				else
				{
					if (option_widget==map_dialog->electrodes.option.hide)
					{
						electrodes_option=HIDE_ELECTRODES;
					}
				}
			}
		}
		if (map->electrodes_option!=electrodes_option)
		{
			map->electrodes_option=electrodes_option;
			map_settings_changed=1;
		}
		if (XmToggleButtonGadgetGetState(map_dialog->electrodes.marker_colour_toggle))
		{
			if (!(map->colour_electrodes_with_signal))
			{
				map_settings_changed=1;
				map->colour_electrodes_with_signal=1;
			}
		}
		else
		{
			if (map->colour_electrodes_with_signal)
			{
				map_settings_changed=1;
				map->colour_electrodes_with_signal=0;
			}
		}		value_string=(char *)NULL;
		XtVaGetValues((map_dialog->electrodes.marker_size_text),
			XmNvalue,&value_string,
			NULL);
		if (1==sscanf(value_string,"%d",&electrodes_marker_size))
		{
			if (electrodes_marker_size<1)
			{
				electrodes_marker_size=1;
			}
		}
		else
		{
			electrodes_marker_size=map_dialog->electrodes_marker_size;
		}
		XtFree(value_string);
		if (map->electrodes_marker_size!=electrodes_marker_size)
		{
			map->electrodes_marker_size=electrodes_marker_size;
			map_settings_changed=1;
		}
		XtVaGetValues(map_dialog->electrodes.marker_type_menu,
			XmNmenuHistory,&option_widget,
			NULL);
		if (option_widget==map_dialog->electrodes.marker_type.circle)
		{
			electrodes_marker_type=CIRCLE_ELECTRODE_MARKER;
		}
		else
		{
			if (option_widget==map_dialog->electrodes.marker_type.plus)
			{
				electrodes_marker_type=PLUS_ELECTRODE_MARKER;
			}
			else
			{
				if (option_widget==map_dialog->electrodes.marker_type.square)
				{
					electrodes_marker_type=SQUARE_ELECTRODE_MARKER;					
				}
				else
				{
					if (option_widget==map_dialog->electrodes.marker_type.none)
					{
						electrodes_marker_type=HIDE_ELECTRODE_MARKER;
					}
					else
					{
						electrodes_marker_type=HIDE_ELECTRODE_MARKER;
					}
				}
			}
		}
		if (map->electrodes_marker_type!=electrodes_marker_type)
		{
			map->electrodes_marker_type=electrodes_marker_type;
			map_settings_changed=1;
		}
		XtVaGetValues(map_dialog->fibres_option_menu,
			XmNmenuHistory,&option_widget,
			NULL);
		if (option_widget==map_dialog->fibres_option.hide)
		{
			fibres_option=HIDE_FIBRES;
		}
		else
		{
			if (option_widget==map_dialog->fibres_option.fine)
			{
				fibres_option=SHOW_FIBRES_FINE;
			}
			else
			{
				if (option_widget==map_dialog->fibres_option.medium)
				{
					fibres_option=SHOW_FIBRES_MEDIUM;
				}
				else
				{
					if (option_widget==map_dialog->fibres_option.coarse)
					{
						fibres_option=SHOW_FIBRES_COARSE;
					}
				}
			}
		}
		if (map->fibres_option!=fibres_option)
		{
			map->fibres_option=fibres_option;
			map_settings_changed=1;
		}
		if (XmToggleButtonGadgetGetState(map_dialog->show_landmarks_toggle))
		{
			if (map->landmarks_option!=SHOW_LANDMARKS)
			{
				map_settings_changed=1;
				map->landmarks_option=SHOW_LANDMARKS;
			}
		}
		else
		{
			if (map->landmarks_option!=HIDE_LANDMARKS)
			{
				map_settings_changed=1;
				map->landmarks_option=HIDE_LANDMARKS;
			}
		}
		if (XmToggleButtonGadgetGetState(map_dialog->show_extrema_toggle))
		{
			if (map->extrema_option!=SHOW_EXTREMA)
			{
				map_settings_changed=1;
				map->extrema_option=SHOW_EXTREMA;
			}
		}
		else
		{
			if (map->extrema_option!=HIDE_EXTREMA)
			{
				map_settings_changed=1;
				map->extrema_option=HIDE_EXTREMA;
			}
		}
		if (XmToggleButtonGadgetGetState(map_dialog->maintain_aspect_ratio_toggle))
		{
			if (!(map->maintain_aspect_ratio))
			{
				map_settings_changed=1;
				if (recalculate<2)
				{
					recalculate=2;
				}
				map->maintain_aspect_ratio=1;
			}
		}
		else
		{
			if (map->maintain_aspect_ratio)
			{
				map_settings_changed=1;
				if (recalculate<2)
				{
					recalculate=2;
				}
				map->maintain_aspect_ratio=0;
			}
		}
		if (XmToggleButtonGadgetGetState(map_dialog->print_spectrum_toggle))
		{
			if (!(map->print_spectrum))
			{
				map->print_spectrum=1;
			}
		}
		else
		{
			if (map->print_spectrum)
			{
				map->print_spectrum=0;
			}
		}
		if ((POTENTIAL== *(map->type))&&(NO_INTERPOLATION!=map->interpolation_type))
		{
			XtVaGetValues((map_dialog->animation).number_of_frames_text,
				XmNvalue,&value_string,
				NULL);
			if (1==sscanf(value_string,"%d",&number_of_frames))
			{
				if (number_of_frames<1)
				{
					number_of_frames=1;
				}
			}
			else
			{
				number_of_frames=map_dialog->number_of_frames;
			}
			XtFree(value_string);
			XtVaGetValues((map_dialog->animation).frame_number_text,
				XmNvalue,&value_string,
				NULL);
			if (1==sscanf(value_string,"%d",&frame_number))
			{
				if (frame_number<1)
				{
					frame_number=1;
				}
				else
				{
					if (frame_number>number_of_frames)
					{
						frame_number=number_of_frames;
					}
				}
			}
			else
			{
				frame_number=map_dialog->frame_number;
			}
			XtFree(value_string);
			if ((map->rig_pointer)&&(*(map->rig_pointer))&&
				((*(map->rig_pointer))->devices)&&(*((*(map->rig_pointer))->devices))&&
				((*((*(map->rig_pointer))->devices))->signal)&&
				(buffer=(*((*(map->rig_pointer))->devices))->signal->buffer))
			{
				buffer_start_time=(float)((buffer->times)[buffer->start])*1000./
					(buffer->frequency);
				buffer_end_time=(float)((buffer->times)[buffer->end])*1000./
					(buffer->frequency);
				XtVaGetValues((map_dialog->animation).start_time_text,
					XmNvalue,&value_string,
					NULL);
				if (1==sscanf(value_string,"%f",&frame_start_time))
				{
					if (frame_start_time<buffer_start_time)
					{
						frame_start_time=buffer_start_time;
					}
				}
				else
				{
					frame_start_time=map_dialog->start_time;
				}
				XtFree(value_string);
				XtVaGetValues((map_dialog->animation).end_time_text,
					XmNvalue,&value_string,
					NULL);
				if (1==sscanf(value_string,"%f",&frame_end_time))
				{
					if (frame_end_time>buffer_end_time)
					{
						frame_end_time=buffer_end_time;
					}
				}
				else
				{
					frame_end_time=map_dialog->end_time;
				}
				XtFree(value_string);
			}
			else
			{
				frame_start_time=map_dialog->start_time;
				frame_end_time=map_dialog->end_time;
			}
			if (frame_start_time>=frame_end_time)
			{
				frame_end_time=frame_start_time;
				number_of_frames=1;
				frame_number=1;
			}
			if (number_of_frames==map_dialog->number_of_frames)
			{
				if ((frame_start_time==map_dialog->start_time)&&
					(frame_end_time==map_dialog->end_time))
				{
					if (frame_number!=map_dialog->frame_number)
					{
						map_settings_changed=1;
					}
				}
				else
				{
					map_settings_changed=1;
					if (recalculate<2)
					{
						recalculate=2;
					}
				}
			}
			else
			{
				map_settings_changed=1;
				if (recalculate<3)
				{
					recalculate=3;
				}
				if (frame=map->frames)
				{
					if (number_of_frames<(i=map->number_of_frames))
					{
						i -= number_of_frames;
						frame += number_of_frames;
						while (i>0)
						{
							DEALLOCATE(frame->contour_x);
							DEALLOCATE(frame->contour_y);
							DEALLOCATE(frame->pixel_values);
							if(frame->image)
							{
								if(frame->image->data)
								{
									DEALLOCATE(frame->image->data);
								}
							}
							XFree((char *)(frame->image));
							frame++;
							i--;
						}
						if (REALLOCATE(frame,map->frames,struct Map_frame,number_of_frames))
						{
							map->frames=frame;
						}
						else
						{
							frame=map->frames;
							while (number_of_frames>0)
							{
								DEALLOCATE(frame->contour_x);
								DEALLOCATE(frame->contour_y);
								DEALLOCATE(frame->pixel_values);
								DEALLOCATE(frame->image->data);
								XFree((char *)(frame->image));
								frame++;
								number_of_frames--;
							}
							DEALLOCATE(map->frames);
							frame_number=1;
							display_message(ERROR_MESSAGE,
								"update_map_from_dialog.  Could not reallocate frames");
						}
					}
					else
					{
						if (REALLOCATE(frame,map->frames,struct Map_frame,number_of_frames))
						{
							map->frames=frame;
							frame += i;
							i=number_of_frames-i;
							while (i>0)
							{
								frame->contour_x=(short int *)NULL;
								frame->contour_y=(short int *)NULL;
								frame->pixel_values=(float *)NULL;
								frame->image=(XImage *)NULL;
								frame++;
								i--;
							}
						}
						else
						{
							frame=map->frames;
							while (i>0)
							{
								DEALLOCATE(frame->contour_x);
								DEALLOCATE(frame->contour_y);
								DEALLOCATE(frame->pixel_values);
								DEALLOCATE(frame->image->data);
								XFree((char *)(frame->image));
								frame++;
								i--;
							}
							DEALLOCATE(map->frames);
							number_of_frames=0;
							frame_number=1;
							display_message(ERROR_MESSAGE,
								"update_map_from_dialog.  Could not reallocate frames");
						}
					}
				}
				else
				{
					number_of_frames=0;
					frame_number=1;
				}
			}
#if defined (OLD_CODE)
			if (number_of_frames!=map->number_of_frames)
			{
				map->number_of_frames=number_of_frames;
				if (1<number_of_frames)
				{
					XtSetSensitive(mapping->animate_button,True);
					XtSetSensitive(mapping->print_menu.animate_rgb_button,True);
					XtSetSensitive(mapping->print_menu.animate_tiff_button,True);
				}
				else
				{
					XtSetSensitive(mapping->animate_button,False);
					XtSetSensitive(mapping->print_menu.animate_rgb_button,False);
					XtSetSensitive(mapping->print_menu.animate_tiff_button,False);
				}
			}
#else
		if (number_of_frames!=map->number_of_frames)
			{
				map->number_of_frames=number_of_frames;
			
			}	
		/* no animation if only one frame*/
		if (1<number_of_frames)
		{
			XtSetSensitive(mapping->animate_button,True);
			XtSetSensitive(mapping->print_menu.animate_rgb_button,True);
			XtSetSensitive(mapping->print_menu.animate_tiff_button,True);
		}
		else
		{
			XtSetSensitive(mapping->animate_button,False);
			XtSetSensitive(mapping->print_menu.animate_rgb_button,False);
			XtSetSensitive(mapping->print_menu.animate_tiff_button,False);
		}
#endif
			map->frame_number=frame_number-1;
			if (frame_start_time!=map_dialog->start_time)
			{
				map->frame_start_time=frame_start_time;
				map_dialog->start_time=frame_start_time;
			}
			if (frame_end_time!=map_dialog->end_time)
			{
				map->frame_end_time=frame_end_time;
				map_dialog->end_time=frame_end_time;
			}
			map_dialog->number_of_frames=number_of_frames;
			map_dialog->frame_number=frame_number;
		}
		if ((POTENTIAL== *(map->type))&&(NO_INTERPOLATION==map->interpolation_type))
		{
			XtSetSensitive(mapping->animate_button,True);
			XtSetSensitive(mapping->print_menu.animate_rgb_button,True);
			XtSetSensitive(mapping->print_menu.animate_tiff_button,True);
		}
		if (MULTIPLE_ACTIVATION== *(map->type))
		{
			if (NO_INTERPOLATION!=map->interpolation_type)
			{
				XtSetSensitive(mapping->animate_button,False);
				XtSetSensitive(mapping->print_menu.animate_rgb_button,False);
				XtSetSensitive(mapping->print_menu.animate_tiff_button,False);
			}
			else
			{
				XtSetSensitive(mapping->animate_button,True);
				XtSetSensitive(mapping->print_menu.animate_rgb_button,True);
				XtSetSensitive(mapping->print_menu.animate_tiff_button,True);
			}
		}
		if (map_settings_changed)
		{
			update_mapping_drawing_area(mapping,recalculate);
			update_mapping_colour_or_auxili(mapping);
			mapping_window_update_time_limits(mapping);
		}
		if (widget==map_dialog->ok_button)
		{
			/* close the map dialog */
			close_map_dialog((Widget)NULL,(XtPointer)map_dialog,(XtPointer)NULL);
		}
		else
		{
			if (map->maximum_value!=map_dialog->range_maximum)
			{
				map->range_changed=1;
				sprintf(temp_string,"%g",map->maximum_value);
				sscanf(temp_string,"%f",&(map_dialog->range_maximum));
				XtVaSetValues(map_dialog->range.maximum_value,
					XmNvalue,temp_string,
					NULL);
			}
			if (map->minimum_value!=map_dialog->range_minimum)
			{
				map->range_changed=1;
				sprintf(temp_string,"%g",map->minimum_value);
				sscanf(temp_string,"%f",&(map_dialog->range_minimum));
				XtVaSetValues(map_dialog->range.minimum_value,
					XmNvalue,temp_string,
					NULL);
			}
			if (map->electrodes_marker_size!=map_dialog->electrodes_marker_size)
			{
				sprintf(temp_string,"%d",map->electrodes_marker_size);
				sscanf(temp_string,"%d",&(map_dialog->electrodes_marker_size));
				XtVaSetValues(map_dialog->electrodes.marker_size_text,
					XmNvalue,temp_string,
					NULL);
			}
			if (map->finite_element_mesh_rows!=map_dialog->number_of_mesh_rows)
			{
				sprintf(temp_string,"%d",map->finite_element_mesh_rows);
				sscanf(temp_string,"%d",&(map_dialog->number_of_mesh_rows));
				XtVaSetValues(map_dialog->interpolation.mesh_rows_text,
					XmNvalue,temp_string,
					NULL);
			}
			if (map->finite_element_mesh_columns!=map_dialog->number_of_mesh_columns)
			{
				sprintf(temp_string,"%d",map->finite_element_mesh_columns);
				sscanf(temp_string,"%d",&(map_dialog->number_of_mesh_columns));
				XtVaSetValues(map_dialog->interpolation.mesh_columns_text,
					XmNvalue,temp_string,
					NULL);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"update_map_from_dialog.  Invalid or missing mapping_window");
	}
	LEAVE;
} /* update_map_from_dialog */

static void configure_map(Widget widget,XtPointer mapping_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 December 1996

DESCRIPTION :
Opens the dialog box associated with the map button in the mapping window.
==============================================================================*/
{
	struct Map_dialog *map_dialog;
	struct Mapping_window *mapping;
	static MrmRegisterArg identifier_list[]=
	{
		{"mapping_window_structure",(XtPointer)NULL}
	};

	ENTER(configure_map);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((mapping=(struct Mapping_window *)mapping_window)&&(mapping->map))
	{
		if (!(map_dialog=mapping->map_dialog))
		{
			/* assign and register the identifiers */
				/*???DB.  Have to put in global name list because the map dialog
					hierarchy may not be open */
			identifier_list[0].value=(XtPointer)mapping;
			if (MrmSUCCESS==MrmRegisterNames(identifier_list,
				XtNumber(identifier_list)))
			{
				map_dialog=create_Map_dialog(&(mapping->map_dialog),&(mapping->map),
					mapping->map_button,mapping->user_interface);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"configure_map.  Could not register identifiers");
			}
		}
		if (map_dialog)
		{
			open_map_dialog(map_dialog);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"configure_map.  Missing mapping_window");
	}
	LEAVE;
} /* configure_map */

static void identify_mapping_area(Widget *widget_id,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 May 2000

DESCRIPTION :
Finds the id of the mapping area
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_area);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
		mapping->mapping_area= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_area.  Missing mapping_window");
	}
	LEAVE;
} /* identify_mapping_area */

static void identify_mapping_area_2d(Widget *widget_id,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 May 2000

DESCRIPTION :
Finds the id of the mapping area_2d
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_area_2d);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
		mapping->mapping_area_2d= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_area_2d.  Missing mapping_window");
	}
	LEAVE;
} /* identify_mapping_area_2d */

static void identify_mapping_area_3d(Widget *widget_id,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 May 2000

DESCRIPTION :
Finds the id of the mapping area_3d
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_area_3d);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
		mapping->mapping_area_3d= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_area_3d.  Missing mapping_window");
	}
	LEAVE;
} /* identify_mapping_area_3d */

static void identify_mapping_drawing_area_2d(Widget *widget_id,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 May 2000

DESCRIPTION :
Finds the id of the mapping area
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_drawing_area_2d);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
		mapping->map_drawing_area_2d= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_drawing_area_2d.  Missing mapping_window");
	}
	LEAVE;
} /* identify_mapping_drawing_area_2d */

static void identify_mapping_animate_button(Widget *widget_id,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 December 1996

DESCRIPTION :
Finds the id of the mapping animate button.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_animate_button);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
		mapping->animate_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_animate_button.  Missing mapping_window");
	}
	LEAVE;
} /* identify_mapping_animate_button */

static void draw_activation_animation_frame(XtPointer mapping_window,
	XtIntervalId *timer_id)
/*******************************************************************************
LAST MODIFIED : 25 June 1998

DESCRIPTION :
Draws a frame in the activation map animation.
==============================================================================*/
{
	Colormap colour_map;
	Display *display;
	float contour_maximum,contour_minimum,maximum_value,minimum_value;
	int cell_number,i,number_of_contours,number_of_spectrum_colours;
	Pixel *spectrum_pixels;
	struct Drawing_2d *drawing;
	struct Map *map;
	struct Map_drawing_information *drawing_information;
	struct Mapping_window *mapping;
	XColor colour,spectrum_rgb[MAX_SPECTRUM_COLOURS];

	ENTER(draw_activation_animation_frame);
	USE_PARAMETER(timer_id);
	if ((mapping=(struct Mapping_window *)mapping_window)&&(map=mapping->map)&&
		(map->type)&&((SINGLE_ACTIVATION== *(map->type))||
		((MULTIPLE_ACTIVATION== *(map->type))&&
		(NO_INTERPOLATION==map->interpolation_type))||
		(POTENTIAL== *(map->type)))&&
		(drawing_information=map->drawing_information)&&(mapping->user_interface)&&
		(mapping->user_interface==drawing_information->user_interface)&&
		(drawing=mapping->map_drawing))
	{
		if (SINGLE_ACTIVATION== *(map->type))
		{
			number_of_spectrum_colours=
				drawing_information->number_of_spectrum_colours;
			if ((0<=map->activation_front)&&
				(map->activation_front<number_of_spectrum_colours))
			{
				if (drawing_information->read_only_colour_map)
				{
					update_mapping_drawing_area(mapping,0);
					update_mapping_colour_or_auxili(mapping);
				}
				else
				{
					display=drawing_information->user_interface->display;
					colour_map=drawing_information->colour_map;
					spectrum_pixels=drawing_information->spectrum_colours;
					/* use background drawing colour for the whole spectrum */
					colour.pixel=drawing_information->background_drawing_colour;
					XQueryColor(display,colour_map,&colour);
					for (i=0;i<number_of_spectrum_colours;i++)
					{
						spectrum_rgb[i].pixel=spectrum_pixels[i];
						spectrum_rgb[i].flags=DoRed|DoGreen|DoBlue;
						spectrum_rgb[i].red=colour.red;
						spectrum_rgb[i].blue=colour.blue;
						spectrum_rgb[i].green=colour.green;
					}
					if ((SHOW_CONTOURS==map->contours_option)&&
						(VARIABLE_THICKNESS==map->contour_thickness))
					{
						colour.pixel=drawing_information->contour_colour;
						XQueryColor(display,colour_map,&colour);
						number_of_contours=map->number_of_contours;
						maximum_value=map->maximum_value;
						minimum_value=map->minimum_value;
						contour_maximum=map->contour_maximum;
						contour_minimum=map->contour_minimum;
						number_of_contours=map->number_of_contours;
						for (i=0;i<number_of_contours;i++)
						{
							cell_number=(int)(((contour_maximum*(float)i+contour_minimum*
								(float)(number_of_contours-1-i))/(float)(number_of_contours-1)-
								minimum_value)/(maximum_value-minimum_value)*
								(float)(number_of_spectrum_colours-1)+0.5);
							spectrum_rgb[cell_number].pixel=spectrum_pixels[cell_number];
							spectrum_rgb[cell_number].flags=DoRed|DoGreen|DoBlue;
							spectrum_rgb[cell_number].red=colour.red;
							spectrum_rgb[cell_number].blue=colour.blue;
							spectrum_rgb[cell_number].green=colour.green;
						}
					}
					/* show the activation front */
					colour.pixel=drawing_information->contour_colour;
					XQueryColor(display,colour_map,&colour);
					i=map->activation_front;
					spectrum_rgb[i].pixel=spectrum_pixels[i];
					spectrum_rgb[i].flags=DoRed|DoGreen|DoBlue;
					spectrum_rgb[i].red=colour.red;
					spectrum_rgb[i].blue=colour.blue;
					spectrum_rgb[i].green=colour.green;
					XStoreColors(display,colour_map,spectrum_rgb,
						number_of_spectrum_colours);
					/* show the map boundary */
					colour.pixel=drawing_information->boundary_colour;
					colour.flags=DoRed|DoGreen|DoBlue;
					XStoreColor(display,colour_map,&colour);
				}
				(map->activation_front)++;
				if (map->activation_front<number_of_spectrum_colours)
				{
					(void)XtAppAddTimeOut(mapping->user_interface->application_context,
						(long unsigned)100,draw_activation_animation_frame,mapping_window);
				}
				else
				{
					map->activation_front= -1;
					XtSetSensitive(mapping->animate_button,True);
					if (drawing_information->read_only_colour_map)
					{
						update_mapping_drawing_area(mapping,0);
						update_mapping_colour_or_auxili(mapping);
					}
					else
					{
						(void)update_colour_map_unemap(map,drawing);
					}
				}
			}
		}
		else
		{
			if (MULTIPLE_ACTIVATION== *(map->type))
			{
				update_mapping_drawing_area(mapping,2);
				update_mapping_colour_or_auxili(mapping);
				/*???DB.  What about the trace window ? */
				(*(map->datum))++;
				if (*(map->datum)< *(map->end_search_interval))
				{
					(void)XtAppAddTimeOut(
						mapping->user_interface->application_context,(long unsigned)10,
						draw_activation_animation_frame,mapping_window);
				}
				else
				{
					*(map->datum)=map->activation_front;
					map->activation_front= -1;
					XtSetSensitive(mapping->animate_button,True);
					update_mapping_drawing_area(mapping,2);
					update_mapping_colour_or_auxili(mapping);
					/*???DB.  What about the trace window ? */
				}
			}
			else
			{
				if (POTENTIAL== *(map->type))
				{
					if (NO_INTERPOLATION==map->interpolation_type)
					{
						update_mapping_drawing_area(mapping,2);
						update_mapping_colour_or_auxili(mapping);
						/*???DB.  What about the trace window ? */
						(*(map->potential_time))++;
						if (*(map->potential_time)< *(map->end_search_interval))
						{
							(void)XtAppAddTimeOut(
								mapping->user_interface->application_context,(long unsigned)10,
								draw_activation_animation_frame,mapping_window);
						}
						else
						{
							*(map->potential_time)=map->activation_front;
							map->activation_front= -1;
							XtSetSensitive(mapping->animate_button,True);
							update_mapping_drawing_area(mapping,2);
							update_mapping_colour_or_auxili(mapping);
							/*???DB.  What about the trace window ? */
						}
					}
					else
					{
						update_mapping_drawing_area(mapping,0);
						(map->frame_number)++;
						if (map->frame_number<map->number_of_frames)
						{
							(void)XtAppAddTimeOut(
								mapping->user_interface->application_context,(long unsigned)100,
								draw_activation_animation_frame,mapping_window);
						}
						else
						{
							map->frame_number=map->activation_front;
							map->activation_front= -1;
							XtSetSensitive(mapping->animate_button,True);
							update_mapping_drawing_area(mapping,0);
						}
					}
				}
			}
		}
	}
	LEAVE;
} /* draw_activation_animation_frame */

static void animate_activation_map(Widget widget,XtPointer mapping_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 June 1998

DESCRIPTION :
Starts the activation map animation.
==============================================================================*/
{
	struct Mapping_window *mapping;
	struct Map *map;
	struct Time_keeper *time_keeper;

	ENTER(animate_activation_map);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((mapping=(struct Mapping_window *)mapping_window)&&(map=mapping->map)&&
		(map->type)&&((SINGLE_ACTIVATION== *(map->type))||
		((MULTIPLE_ACTIVATION== *(map->type))&&
		(NO_INTERPOLATION==map->interpolation_type))||(POTENTIAL== *(map->type))))
	{
		/* start the animation */
		if ((mapping->potential_time_object)&&
			(time_keeper=Time_object_get_time_keeper(mapping->potential_time_object)))
		{
			if (Time_keeper_is_playing(time_keeper))
			{
				Time_keeper_stop(time_keeper);
			}
			else
			{
				/* The activation button remains active now */
				switch (*(map->type))
				{
					case POTENTIAL:
					{
						map->activation_front= 0;
						if (NO_INTERPOLATION!=map->interpolation_type)
						{
							/* Jump to the start of the animated sequence */
							Time_keeper_request_new_time(time_keeper,
								map->frame_start_time);
						}
					} break;
					case SINGLE_ACTIVATION:
					{
						map->activation_front= 0;
						Time_keeper_request_new_time(time_keeper,
								map->minimum_value + (float)*(map->datum));
					} break;
					case MULTIPLE_ACTIVATION:
					{
						map->activation_front= *(map->datum);
						Time_keeper_request_new_time(time_keeper,
								*(map->start_search_interval));
					} break;
				}
				bring_up_time_editor_dialog(
					&(mapping->time_editor_dialog),
					mapping->user_interface->application_shell,
					time_keeper, mapping->user_interface);
				Time_keeper_play(time_keeper, TIME_KEEPER_PLAY_FORWARD);
			}
		}
		else
		{
			/* initialize the activation front */
			if (SINGLE_ACTIVATION== *(map->type))
			{
				map->activation_front=0;
			}
			else
			{
				if (MULTIPLE_ACTIVATION== *(map->type))
				{
					map->activation_front= *(map->datum);
					*(map->datum)= *(map->start_search_interval);
				}
				else
				{
					if (POTENTIAL== *(map->type))
					{
						if (NO_INTERPOLATION==map->interpolation_type)
						{
							map->activation_front= *(map->potential_time);
							*(map->potential_time)= *(map->start_search_interval);
						}
						else
						{
							map->activation_front=map->frame_number;
							map->frame_number=0;
						}
					}
				}
			}
			/* only one animation at a time */
			XtSetSensitive(mapping->animate_button,False);

			draw_activation_animation_frame(mapping_window,(XtIntervalId *)NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"animate_activation_map.  Invalid mapping_window");
	}
	LEAVE;
} /* animate_activation_map */

static void identify_mapping_setup_button(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 December 1996

DESCRIPTION :
Finds the id of the mapping set up button.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_setup_button);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)client_data)
	{
		mapping->setup_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_setup_button.  client_data missing");
	}
	LEAVE;
} /* identify_mapping_setup_button */

static void create_simple_rig_from_dialog(Widget widget,
	XtPointer mapping_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 June 1998

DESCRIPTION :
Creates a new rig using the values specified in the setup dialog.
==============================================================================*/
{
	char *rig_name;
	enum Region_type region_type;
	int *number_in_row,row_number;
	struct Electrodes_in_row *electrodes_in_row;
	struct Mapping_window *mapping;
	struct Rig **map_rig_pointer,*rig;
	struct Setup_dialog *setup;
	Widget region_type_widget;

	ENTER(create_simple_rig_from_dialog);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((mapping=(struct Mapping_window *)mapping_window_structure)&&
		(setup=mapping->setup_dialog)&&(mapping->map)&&
		(map_rig_pointer=mapping->map->rig_pointer))
	{
		/* determine the rig type */
		XtVaGetValues(setup->rig_choice,
			XmNmenuHistory,&region_type_widget,
			NULL);
		if (region_type_widget==setup->region_type.sock)
		{
			region_type=SOCK;
			rig_name="sock";
		}
		else
		{
			if (region_type_widget==setup->region_type.patch)
			{
				region_type=PATCH;
				rig_name="patch";
			}
			else
			{
				if (region_type_widget==setup->region_type.torso)
				{
					region_type=TORSO;
					rig_name="torso";
				}
			}
		}
		if (((row_number=setup->number_of_rows)>0)&&
			(ALLOCATE(number_in_row,int,row_number)))
		{
			electrodes_in_row=setup->electrodes_in_row;
			while (electrodes_in_row&&(row_number>0))
			{
				row_number--;
				number_in_row[row_number]=electrodes_in_row->number;
				electrodes_in_row=electrodes_in_row->next;
			}
			if ((!electrodes_in_row)&&(0==row_number)&&
				(rig=create_standard_Rig(rig_name /*??? enter name ? */,
				region_type,MONITORING_OFF,EXPERIMENT_OFF,setup->number_of_rows,
				number_in_row,setup->number_of_regions,
				setup->number_of_auxiliary_devices,30./*??? enter focus ? */
#if defined (UNEMAP_USE_NODES)		
				,mapping->map->unemap_package
#endif /* defined (UNEMAP_USE_NODES) */
					)))
			{
				/* destroy the present rig */
				destroy_Rig(map_rig_pointer);
				/* assign the new rig */
				*(map_rig_pointer)=rig;
				/* update the mapping window */
				update_mapping_drawing_area(mapping,0);
				update_mapping_colour_or_auxili(mapping);
				update_mapping_window_menu(mapping);
				XtSetSensitive(mapping->file_menu.save_configuration_button,True);
				XtSetSensitive(mapping->file_menu.set_default_configuration_button,
					True);
				/* close the setup dialog */
				close_setup_dialog((Widget)NULL,(XtPointer)setup,(XtPointer)NULL);
			}
			DEALLOCATE(number_in_row);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_simple_rig_from_dialog.  Invalid or missing mapping_window");
	}
	LEAVE;
} /* create_simple_rig_from_dialog */

static void setup_simple_rig(Widget widget,XtPointer mapping_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 May 1998

DESCRIPTION :
Opens the dialog box associated with the setup button in the mapping window.
==============================================================================*/
{
	struct Setup_dialog *setup_dialog;
	struct Mapping_window *mapping;
	static MrmRegisterArg identifier_list[]=
	{
		{"mapping_window_structure",(XtPointer)NULL}
	};

	ENTER(setup_simple_rig);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
		if (!(setup_dialog=mapping->setup_dialog))
		{
			/* assign and register the identifiers */
				/*???DB.  Have to put in global name list because the setup dialog
					hierarchy may not be open */
			identifier_list[0].value=(XtPointer)mapping;
			if (MrmSUCCESS==MrmRegisterNames(identifier_list,
				XtNumber(identifier_list)))
			{
				setup_dialog=create_Setup_dialog(&(mapping->setup_dialog),
					mapping->setup_button,mapping->user_interface);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"setup_simple_rig.  Could not register identifiers");
			}
		}
		if (setup_dialog)
		{
			/* ghost the activation button */
			XtSetSensitive(setup_dialog->activation,False);
			busy_cursor_on(setup_dialog->shell,mapping->user_interface);
			/* pop up the setup dialog */
			XtManageChild(setup_dialog->dialog);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"setup_simple_rig.  Missing mapping_window");
	}
	LEAVE;
} /* setup_simple_rig */


static int Mapping_window_make_drawing_area_2d(
	struct Mapping_window *mapping_window)
/*******************************************************************************
LAST MODIFIED : 29 May 2000

DESCRIPTION :
Removes the Scene_viewer, if any, and replaces it with a 2-D XmDrawingArea in
the mapping_area of the <mapping_window>.
==============================================================================*/
{
	int return_code;
	ENTER(Mapping_window_make_drawing_area_2d);
	if (mapping_window&&mapping_window->mapping_area)
	{
#if defined (UNEMAP_USE_NODES)			
		XtUnmanageChild(mapping_window->mapping_area_3d);
#endif /* defined (UNEMAP_USE_NODES) */			
		XtManageChild(mapping_window->mapping_area_2d);		
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mapping_window_make_drawing_area_2d.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mapping_window_make_drawing_area_2d */

#if defined (UNEMAP_USE_NODES)
static int Mapping_window_make_drawing_area_3d(struct Mapping_window *mapping_window)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Removes the  2-D XmDrawingArea if any, and replaces it with a Scene_viewer,
the mapping_area of the <mapping_window>.
==============================================================================*/
{
	int return_code;
	struct Map_drawing_information *drawing_information=
		(struct Map_drawing_information *)NULL;
	struct Unemap_package *unemap_package=(struct Unemap_package *)NULL;

	ENTER(Mapping_window_make_drawing_area_3d);
	if (mapping_window&&mapping_window->mapping_area&&
		(unemap_package=mapping_window->map->unemap_package)&&
		(drawing_information=mapping_window->map->drawing_information))
	{	 
		XtUnmanageChild(mapping_window->mapping_area_2d);				
		map_drawing_information_make_map_scene(drawing_information,unemap_package);	
		if (!mapping_window->scene_viewer)
		{
			mapping_window->scene_viewer=
				CREATE(Scene_viewer)(mapping_window->mapping_area_3d,
					get_map_drawing_information_background_colour(drawing_information),
					SCENE_VIEWER_DOUBLE_BUFFER,
					get_map_drawing_information_Light_manager(drawing_information),
					get_map_drawing_information_light(drawing_information),
					get_map_drawing_information_Light_model_manager(drawing_information),
					get_map_drawing_information_light_model(drawing_information),
					get_map_drawing_information_Scene_manager(drawing_information),
					get_map_drawing_information_scene(drawing_information),
					get_map_drawing_information_Texture_manager(drawing_information),
					get_map_drawing_information_user_interface(drawing_information));
			set_map_drawing_information_scene_viewer(drawing_information,
				mapping_window->scene_viewer);
		}
		set_map_drawing_information_viewed_scene(drawing_information,0);
		XtManageChild(mapping_window->mapping_area_3d);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mapping_window_make_drawing_area_3d.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mapping_window_make_drawing_area_3d */
#endif /* defined (UNEMAP_USE_NODES) */

static void identify_mapping_modify_button(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 December 1996

DESCRIPTION :
Finds the id of the mapping modify button.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_modify_button);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)client_data)
	{
		mapping->modify_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_modify_button.  client_data missing");
	}
	LEAVE;
} /* identify_mapping_modify_button */

static void identify_mapping_page_button(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 December 1996

DESCRIPTION :
Finds the id of the mapping page button.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_page_button);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)client_data)
	{
		mapping->page_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_page_button.  client_data missing");
	}
	LEAVE;
} /* identify_mapping_page_button */

static void identify_mapping_file_button(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 December 1996

DESCRIPTION :
Finds the id of the mapping file button.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_file_button);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)client_data)
	{
		mapping->file_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_file_button.  client_data missing");
	}
	LEAVE;
} /* identify_mapping_file_button */

static void identify_mapping_file_save_butt(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 December 1996

DESCRIPTION :
Finds the id of the mapping file save button.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_file_save_butt);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)client_data)
	{
		mapping->file_menu.save_configuration_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_file_save_butt.  client_data missing");
	}
	LEAVE;
} /* identify_mapping_file_save_butt */

static void identify_mapping_file_read_butt(Widget *widget_id,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 December 1996

DESCRIPTION :
Finds the id of the mapping file read button.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_file_read_butt);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
		mapping->file_menu.read_configuration_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_file_read_butt.  Missing mapping_window");
	}
	LEAVE;
} /* identify_mapping_file_read_butt */


static int mapping_read_configuration_file(char *file_name,void *mapping_window)
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Reads in the configuration file and makes the mapping window consistent with the
new rig.
==============================================================================*/
{
	int return_code;
	struct Mapping_window *mapping;	
#if defined (UNEMAP_USE_NODES)
	struct Unemap_package *unemap_package;		
#endif
	ENTER(mapping_read_configuration_file);
	if ((mapping=(struct Mapping_window *)mapping_window)&&(mapping->map))
	{
#if defined (UNEMAP_USE_NODES)
		unemap_package = mapping->map->unemap_package;
#endif /* defined (UNEMAP_USE_NODES) */		
		/* destroy the existing configuration */
		destroy_Rig(mapping->map->rig_pointer);
		/* read the configuration file */
		if (return_code=read_configuration_file(file_name,
			(void *)(mapping->map->rig_pointer)
#if defined (UNEMAP_USE_NODES)			
			,unemap_package
#endif /* defined (UNEMAP_USE_NODES) */
			 ))
		{
#if defined (UNEMAP_USE_NODES)
			/* read the configuration file into nodes */		
			file_read_config_FE_node_group(file_name,unemap_package,*(mapping->map->rig_pointer));
#endif /* defined (UNEMAP_USE_NODES) */
			/* unghost the save configuration and set default configuration buttons */
			XtSetSensitive(mapping->file_menu.save_configuration_button,True);
			XtSetSensitive(mapping->file_menu.set_default_configuration_button,True);
			update_mapping_window_menu(mapping);
			update_mapping_drawing_area(mapping,2);
			update_mapping_colour_or_auxili(mapping);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"mapping_read_configuration_file.  Invalid mapping_window");
	}
	LEAVE;

	return (return_code);
} /* mapping_read_configuration_file */

static void identify_mapping_file_bard_elec(Widget *widget_id,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 December 1996

DESCRIPTION :
Finds the id of the mapping file Bard electrode button.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_file_bard_elec);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
		mapping->file_menu.read_bard_electrode_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_file_bard_elec.  Missing mapping_window");
	}
	LEAVE;
} /* identify_mapping_file_bard_elec */

static int mapping_read_bard_electrode_fil(char *file_name,
	void *mapping_window)
/*******************************************************************************
LAST MODIFIED : 24 May 1997

DESCRIPTION :
Reads in the Bard electrode file and makes the mapping window consistent with
the new rig.
???Almost identical to mapping_read_configuration_file
==============================================================================*/
{
	int return_code;
	struct Mapping_window *mapping;

	ENTER(mapping_read_bard_electrode_fil);
	if ((mapping=(struct Mapping_window *)mapping_window)&&(mapping->map))
	{
		/* destroy the existing configuration */
		destroy_Rig(mapping->map->rig_pointer);
		/* read the Bard electrode file */
		if (return_code=read_bard_electrode_file(file_name,
			(void *)(mapping->map->rig_pointer)))
		{
			/* unghost the save configuration and set default configuration buttons */
			XtSetSensitive(mapping->file_menu.save_configuration_button,True);
			XtSetSensitive(mapping->file_menu.set_default_configuration_button,True);
			update_mapping_window_menu(mapping);
			update_mapping_drawing_area(mapping,2);
			update_mapping_colour_or_auxili(mapping);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"mapping_read_bard_electrode_fil.  Invalid mapping_window");
	}
	LEAVE;

	return (return_code);
} /* mapping_read_bard_electrode_fil */

static void identify_mapping_file_default_b(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 December 1996

DESCRIPTION :
Finds the id of the mapping file default button.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_file_default_b);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)client_data)
	{
		mapping->file_menu.set_default_configuration_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_file_default_b.  client_data missing");
	}
	LEAVE;
} /* identify_mapping_file_default_b */

static void set_default_configuration_file(Widget widget,XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 December 1996

DESCRIPTION :
Writes the current mapping rig to the default configuration file.
==============================================================================*/
{
	struct Rig **rig_pointer;

	ENTER(set_default_configuration_file);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (rig_pointer=(struct Rig **)client_data)
	{
		write_configuration_file("default.cnfg",(void *)rig_pointer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_default_configuration_file.  client_data missing");
	}
	LEAVE;
} /* set_default_configuration_file */

static void identify_mapping_print_button(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 December 1996

DESCRIPTION :
Finds the id of the mapping print button.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_print_button);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)client_data)
	{
		mapping->print_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_print_button.  client_data missing");
	}
	LEAVE;
} /* identify_mapping_print_button */

static void identify_mapping_print_postscri(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 May 1997

DESCRIPTION :
Finds the id of the mapping print postscript button.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_print_postscri);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)client_data)
	{
		mapping->print_menu.postscript_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_print_postscri.  client_data missing");
	}
	LEAVE;
} /* identify_mapping_print_postscri */

static void identify_mapping_print_rgb_butt(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 May 1997

DESCRIPTION :
Finds the id of the mapping print rgb button.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_print_rgb_butt);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)client_data)
	{
		mapping->print_menu.rgb_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_print_rgb_butt.  client_data missing");
	}
	LEAVE;
} /* identify_mapping_print_rgb_butt */

static void identify_mapping_print_tiff_but(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 July 1998

DESCRIPTION :
Finds the id of the mapping print tiff button.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_print_tiff_but);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)client_data)
	{
		mapping->print_menu.tiff_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_print_tiff_but.  client_data missing");
	}
	LEAVE;
} /* identify_mapping_print_tiff_but */

static void id_mapping_print_animation_rgb(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 October 1997

DESCRIPTION :
Finds the id of the mapping print animation rgb button.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(id_mapping_print_animation_rgb);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)client_data)
	{
		mapping->print_menu.animate_rgb_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_mapping_print_animation_rgb.  client_data missing");
	}
	LEAVE;
} /* id_mapping_print_animation_rgb */

static void id_mapping_print_animation_tiff(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 July 1998

DESCRIPTION :
Finds the id of the mapping print animation tiff button.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(id_mapping_print_animation_tiff);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)client_data)
	{
		mapping->print_menu.animate_tiff_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_mapping_print_animation_tiff.  client_data missing");
	}
	LEAVE;
} /* id_mapping_print_animation_tiff */

static void identify_mapping_projection_cho(Widget *widget_id,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 December 1996

DESCRIPTION :
Finds the id of the mapping projection choice button.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_projection_cho);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
		mapping->projection_choice= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_projection_cho.  mapping_window missing");
	}
	LEAVE;
} /* identify_mapping_projection_cho */

static void identify_mapping_projection_cyl(Widget *widget_id,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 2 June 2000

DESCRIPTION :
Finds the id of the mapping projection cylinder button.
This is a member of the mapping projection choice menu
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_projection_cyl);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
		mapping->projection_cylinder= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_projection_cyl.  mapping_window missing");
	}
	LEAVE;
} /* identify_mapping_projection_cyl */

static void identify_mapping_projection_ham(Widget *widget_id,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED :  2 June 2000 

DESCRIPTION :
Finds the id of the mapping projection hammer button.
This is a member of the mapping projection choice menu
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_projection_ham);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
		mapping->projection_hammer= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_projection_ham.  mapping_window missing");
	}
	LEAVE;
} /* identify_mapping_projection_ham */

static void identify_mapping_projection_pol(Widget *widget_id,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 2 June 2000 

DESCRIPTION :
Finds the id of the mapping projection polar button.
This is a member of the mapping projection choice menu
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_projection_pol);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
		mapping->projection_polar= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_projection_pol.  mapping_window missing");
	}
	LEAVE;
} /* identify_mapping_projection_pol */

static void identify_mapping_projection_pat(Widget *widget_id,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 2 June 2000 

DESCRIPTION :
Finds the id of the mapping projection patch button.
This is a member of the mapping projection choice menu
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_projection_pat);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
		mapping->projection_patch= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_projection_pat.  mapping_window missing");
	}
	LEAVE;
} /* identify_mapping_projection_pat */

static void identify_mapping_projection_3d(Widget *widget_id,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 2 June 2000

DESCRIPTION :
Finds the id of the mapping projection 3d button.
This is a member of the mapping projection choice menu
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_projection_3d);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
		mapping->projection_3d= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_projection_3d.  mapping_window missing");
	}
	LEAVE;
} /* identify_mapping_projection_3d */

static void identify_mapping_region_choice(Widget *widget_id,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 December 1996

DESCRIPTION :
Finds the id of the mapping region choice button.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_region_choice);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
		mapping->region_choice= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_region_choice.  mapping_window missing");
	}
	LEAVE;
} /* identify_mapping_region_choice */

static void identify_mapping_region_pull_do(Widget *widget_id,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 December 1996

DESCRIPTION :
Finds the id of the mapping region pull down menu.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_region_pull_do);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
		mapping->region_pull_down_menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_region_pull_do.  mapping_window missing");
	}
	LEAVE;
} /* identify_mapping_region_pull_do */

static void identify_mapping_region_place_h(Widget *widget_id,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 December 1996

DESCRIPTION :
In order for the region choice pull-down to be in the right place, there needs
to be a button in the pull-down when the mapping window is first realized.  A
place holder is used.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_region_place_h);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
		if (ALLOCATE(mapping->regions,Widget,1))
		{
			mapping->number_of_regions=1;
			*(mapping->regions)= *widget_id;
			mapping->current_region= *widget_id;
		}
		else
		{
			display_message(ERROR_MESSAGE,
			"identify_mapping_region_place_h.  Insufficient memory for region list");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_region_place_h.  mapping_window missing");
	}
	LEAVE;
} /* identify_mapping_region_place_h */

static void identify_mapping_close_button(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 December 1996

DESCRIPTION :
Finds the id of the mapping close button.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_close_button);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)client_data)
	{
		mapping->close_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_close_button.  client_data missing");
	}
	LEAVE;
} /* identify_mapping_close_button */

static void expose_mapping_colour_or_auxili(Widget widget,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 9 February 1997

DESCRIPTION :
The callback for redrawing part of a colour bar or auxiliary device drawing
area.
==============================================================================*/
{
	Display *display;
	struct Drawing_2d *drawing;
	struct Mapping_window *mapping;
	XmDrawingAreaCallbackStruct *callback;
	XExposeEvent *event;
	XWindowAttributes attributes;

	ENTER(expose_mapping_colour_or_auxili);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if ((mapping=(struct Mapping_window *)mapping_window)&&
		(mapping->user_interface))
	{
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (XmCR_EXPOSE==callback->reason)
			{
				if (callback->event)
				{
					if (Expose==callback->event->type)
					{
						display=mapping->user_interface->display;
						event= &(callback->event->xexpose);
						if (mapping->colour_or_auxiliary_drawing_area)
						{
							if (!(mapping->colour_or_auxiliary_drawing))
							{
								/* determine the size of the drawing area */
								XGetWindowAttributes(display,
									XtWindow(mapping->colour_or_auxiliary_drawing_area),
									&attributes);
								/* create a pixel map */
								if (drawing=
									create_Drawing_2d(mapping->colour_or_auxiliary_drawing_area,
									attributes.width,attributes.height,NO_DRAWING_IMAGE,
									mapping->user_interface))
								{
									mapping->colour_or_auxiliary_drawing=drawing;
									/* clear the colour or auxiliary area */
									XFillRectangle(display,drawing->pixel_map,(mapping->map->
										drawing_information->graphics_context).
										background_drawing_colour,0,0,drawing->width,
										drawing->height);
									/* draw the colour bar or the auxiliary devices */
									draw_colour_or_auxiliary_area(mapping->map,drawing);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"expose_mapping_colour_or_auxi.  Could not create drawing");
								}
							}
							/* redisplay the specified part of the pixmap */
							if (mapping->colour_or_auxiliary_drawing)
							{
								if ((mapping->map)&&(mapping->map->drawing_information))
								{
									XCopyArea(display,
										mapping->colour_or_auxiliary_drawing->pixel_map,
										XtWindow(mapping->colour_or_auxiliary_drawing_area),
										(mapping->map->drawing_information->graphics_context).copy,
										event->x,event->y,event->width,event->height,event->x,
										event->y);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"expose_mapping_colour_or_auxili.  Missing copy GC");
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"expose_mapping_colour_or_auxili.  Missing drawing area");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"expose_mapping_colour_or_auxili.  Incorrect event reason");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"expose_mapping_colour_or_auxili.  event missing");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"expose_mapping_colour_or_auxili.  Incorrect reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"expose_mapping_colour_or_auxili.  call_data missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"expose_mapping_colour_or_auxili.  mapping window missing");
	}
	LEAVE;
} /* expose_mapping_colour_or_auxili */

static void expose_mapping_drawing_area_2d(Widget widget,XtPointer mapping_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 7 October 1997

DESCRIPTION :
The callback for redrawing part of a mapping drawing area.
==============================================================================*/
{
	Display *display;
	struct Drawing_2d *drawing;
	struct Mapping_window *mapping;
	XmDrawingAreaCallbackStruct *callback;
	XExposeEvent *event;
	XWindowAttributes attributes;

	ENTER(expose_mapping_drawing_area_2d);
	USE_PARAMETER(widget);
	if ((mapping=(struct Mapping_window *)mapping_window)&&
		(mapping->user_interface))
	{
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (XmCR_EXPOSE==callback->reason)
			{
				if (callback->event)
				{
					if (Expose==callback->event->type)
					{
						display=mapping->user_interface->display;
						event= &(callback->event->xexpose);
						if (mapping->map_drawing_area_2d)
						{
							if (!(mapping->map_drawing))
							{
								/* determine the size of the drawing area */
								XGetWindowAttributes(display,
									XtWindow(mapping->map_drawing_area_2d),&attributes);
								/* create a pixel map */
								if (drawing=
									create_Drawing_2d(mapping->map_drawing_area_2d,attributes.width,
									attributes.height,NO_DRAWING_IMAGE,mapping->user_interface))
								{
									mapping->map_drawing=drawing;
									/* clear the map drawing area */
									XFillRectangle(display,drawing->pixel_map,(mapping->map->
										drawing_information->graphics_context).
										background_drawing_colour,0,0,drawing->width,
										drawing->height);
									/* draw the map */
									draw_map(mapping->map,3,drawing);
									update_mapping_colour_or_auxili(mapping);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"expose_mapping_drawing_area_2d.  Could not create drawing");
								}
							}
							/* redisplay the specified part of the pixmap */
							if (mapping->map_drawing)
							{
								if ((mapping->map)&&(mapping->map->drawing_information))
								{
									XCopyArea(display,mapping->map_drawing->pixel_map,
										XtWindow(mapping->map_drawing_area_2d),
										(mapping->map->drawing_information->graphics_context).copy,
										event->x,event->y,event->width,event->height,event->x,
										event->y);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"expose_mapping_drawing_area_2d.  Missing copy GC");
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"expose_mapping_drawing_area_2d.  Missing drawing area");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"expose_mapping_drawing_area_2d.  Incorrect event reason");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"expose_mapping_drawing_area_2d.  event missing");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"expose_mapping_drawing_area_2d.  Incorrect reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"expose_mapping_drawing_area_2d.  call_data missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"expose_mapping_drawing_area_2d.  mapping window missing");
	}
	LEAVE;
} /* expose_mapping_drawing_area_2d */

static void resize_mapping_drawing_area_2d(Widget widget,XtPointer mapping_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 7 October 1997

DESCRIPTION :
The callback for resizing a mapping drawing area.
==============================================================================*/
{
	Display *display;
	int width,height;
	struct Drawing_2d *drawing;
	struct Mapping_window *mapping;
	XmDrawingAreaCallbackStruct *callback;
	XWindowAttributes attributes;

	ENTER(resize_mapping_drawing_area_2d);
	USE_PARAMETER(widget);
	if ((mapping=(struct Mapping_window *)mapping_window)&&
		(mapping->user_interface))
	{
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (XmCR_RESIZE==callback->reason)
			{
				/*??? during creation there are resize callbacks without windows */
				if (callback->window)
				{
					if (mapping->map_drawing_area_2d)
					{
						display=mapping->user_interface->display;
						/* find the size of the old rectangle */
						if (mapping->map_drawing)
						{
							width=mapping->map_drawing->width;
							height=mapping->map_drawing->height;
							/* destroy the old pixmap */
							destroy_Drawing_2d(&(mapping->map_drawing));
						}
						else
						{
							width=0;
							height=0;
						}
						/* find the size of the new rectangle */
						XGetWindowAttributes(display,callback->window,&attributes);
						/* create a new pixmap */
						if (drawing=create_Drawing_2d(
							mapping->map_drawing_area_2d,attributes.width,attributes.height,
							NO_DRAWING_IMAGE,mapping->user_interface))
						{
							mapping->map_drawing=drawing;
							/* clear the map drawing area */
							XFillRectangle(display,drawing->pixel_map,(mapping->map->
								drawing_information->graphics_context).
								background_drawing_colour,0,0,drawing->width,drawing->height);
							/* redraw the map */
							draw_map(mapping->map,3,drawing);
							/* display the intersection of the old rectangle and the new
								rectangle */
							if (attributes.width<width)
							{
								width=attributes.width;
							}
							if (attributes.height<height)
							{
								height=attributes.height;
							}
							if ((mapping->map)&&(mapping->map->drawing_information))
							{
								XCopyArea(display,drawing->pixel_map,
									XtWindow(mapping->map_drawing_area_2d),
									(mapping->map->drawing_information->graphics_context).copy,
									0,0,width,height,0,0);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"resize_mapping_drawing_area_2d.  Missing copy GC");
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"resize_mapping_drawing_area_2d.  Missing drawing area");
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"resize_mapping_drawing_area_2d.  Incorrect reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"resize_mapping_drawing_area_2d.  call_data missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"resize_mapping_drawing_area_2d.  mapping window missing");
	}
	LEAVE;
} /* resize_mapping_drawing_area_2d */

static void set_projection_Hammer(Widget *widget_id,XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 May 1997

DESCRIPTION :
Sets the projection to be the Hammer projection.
???DB.  MIXED ?
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(set_projection_Hammer);
	USE_PARAMETER(widget_id);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)client_data)
	{
		if (mapping->map)
		{
			if (mapping->map->projection_type!=HAMMER_PROJECTION)
			{
				mapping->map->projection_type=HAMMER_PROJECTION;	
				Mapping_window_make_drawing_area_2d(mapping);
				update_mapping_drawing_area(mapping,2);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_projection_Hammer.  map missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_projection_Hammer.  client_data missing");
	}
	LEAVE;
} /* set_projection_Hammer */

static void set_projection_polar(Widget *widget_id,XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 May 1997

DESCRIPTION :
Sets the projection to be the polar projection.
???DB.  MIXED ?
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(set_projection_polar);
	USE_PARAMETER(widget_id);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)client_data)
	{
		if (mapping->map)
		{
			if (mapping->map->projection_type!=POLAR_PROJECTION)
			{
				mapping->map->projection_type=POLAR_PROJECTION;				
				Mapping_window_make_drawing_area_2d(mapping);
				update_mapping_drawing_area(mapping,2);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_projection_polar.  map missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_projection_polar.  client_data missing");
	}
	LEAVE;
} /* set_projection_polar */


static void set_projection_patch(Widget *widget_id,XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 2 June 2000

DESCRIPTION :
There isn't actually a patch projection type, this is the X callback to do the 
drawing
???DB.  MIXED ?
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(set_projection_patch);
	USE_PARAMETER(widget_id);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)client_data)
	{
		if (mapping->map)
		{		
			/*must set the rojection_type to something*/
			mapping->map->projection_type=CYLINDRICAL_PROJECTION;
			Mapping_window_make_drawing_area_2d(mapping);
			update_mapping_drawing_area(mapping,2);		
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_projection_patch.  map missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_projection_patch.  client_data missing");
	}
	LEAVE;
} /* set_projection_patch */

static void set_projection_cylinder(Widget *widget_id,XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 May 1997

DESCRIPTION :
Sets the projection to be the cylinder projection.
???DB.  MIXED ?
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(set_projection_cylinder);
	USE_PARAMETER(widget_id);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)client_data)
	{
		if (mapping->map)
		{
			if (mapping->map->projection_type!=CYLINDRICAL_PROJECTION)
			{
				mapping->map->projection_type=CYLINDRICAL_PROJECTION;				
				Mapping_window_make_drawing_area_2d(mapping);
				update_mapping_drawing_area(mapping,2);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_projection_cylinder.  map missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_projection_cylinder.  client_data missing");
	}
	LEAVE;
} /* set_projection_cylinder */

static void set_projection_3d(Widget *widget_id,XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 31 May 2000

DESCRIPTION :
Sets the projection to be the 3D projection.
???DB.  MIXED ?
==============================================================================*/
{
#if defined (UNEMAP_USE_NODES)
	struct Mapping_window *mapping;

	ENTER(set_projection_3d);
	USE_PARAMETER(widget_id);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)client_data)
	{
		if (mapping->map)
		{
			if (mapping->map->projection_type!=THREED_PROJECTION)
			{
				mapping->map->projection_type=THREED_PROJECTION;			
				Mapping_window_make_drawing_area_3d(mapping);
				update_mapping_drawing_area(mapping,2);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_projection_3d.  map missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_projection_3d.  client_data missing");
	}
	LEAVE;
#else
	ENTER(set_projection_3d);
	USE_PARAMETER(widget_id);
	USE_PARAMETER(call_data);
	USE_PARAMETER(client_data);
	LEAVE;
#endif /* defined (UNEMAP_USE_NODES) */
} /* set_projection_3d */

static void identify_mapping_colour_or_auxi(Widget *widget_id,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 December 1996

DESCRIPTION :
Finds the id of the colour_or_auxiliary_drawing_area.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_colour_or_auxi);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
		mapping->colour_or_auxiliary_drawing_area= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_colour_or_auxi.  mapping_window missing");
	}
	LEAVE;
} /* identify_mapping_colour_or_auxi */

static void resize_mapping_colour_or_auxili(Widget widget,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 9 February 1997

DESCRIPTION :
The callback for resizing a colour bar or auxiliary devices drawing area.
==============================================================================*/
{
	Display *display;
	int width,height;
	struct Drawing_2d *drawing;
	struct Mapping_window *mapping;
	XmDrawingAreaCallbackStruct *callback;
	XWindowAttributes attributes;

	ENTER(resize_mapping_colour_or_auxili);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if ((mapping=(struct Mapping_window *)mapping_window)&&
		(mapping->user_interface))
	{
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (XmCR_RESIZE==callback->reason)
			{
				/*??? during creation there are resize callbacks without windows */
				if (callback->window)
				{
					if (mapping->colour_or_auxiliary_drawing_area)
					{
						display=mapping->user_interface->display;
						/* find the size of the old rectangle */
						if (mapping->colour_or_auxiliary_drawing)
						{
							width=mapping->colour_or_auxiliary_drawing->width;
							height=mapping->colour_or_auxiliary_drawing->height;
							/* destroy the old pixmap */
							destroy_Drawing_2d(&(mapping->colour_or_auxiliary_drawing));
						}
						else
						{
							width=0;
							height=0;
						}
						/* find the size of the new rectangle */
						XGetWindowAttributes(display,callback->window,&attributes);
						/* create a new pixmap */
						if (drawing=create_Drawing_2d(
							mapping->colour_or_auxiliary_drawing_area,
							attributes.width,attributes.height,NO_DRAWING_IMAGE,
							mapping->user_interface))
						{
							mapping->colour_or_auxiliary_drawing=drawing;
							/* clear the colour or auxiliary area */
							XFillRectangle(display,drawing->pixel_map,(mapping->map->
								drawing_information->graphics_context).
								background_drawing_colour,0,0,drawing->width,drawing->height);
							/* redraw the colour bar or the auxiliary devices */
							draw_colour_or_auxiliary_area(mapping->map,drawing);
							/* display the intersection of the old rectangle and the new
								rectangle */
							if (attributes.width<width)
							{
								width=attributes.width;
							}
							if (attributes.height<height)
							{
								height=attributes.height;
							}
							if ((mapping->map)&&(mapping->map->drawing_information))
							{
								XCopyArea(display,drawing->pixel_map,
									XtWindow(mapping->colour_or_auxiliary_drawing_area),
									(mapping->map->drawing_information->graphics_context).copy,
									0,0,width,height,0,0);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"resize_mapping_colour_or_auxili.  Missing copy GC");
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"resize_mapping_colour_or_auxili.  Missing drawing area");
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"resize_mapping_colour_or_auxili.  Incorrect reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"resize_mapping_colour_or_auxili.  call_data missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"resize_mapping_colour_or_auxili.  mapping window missing");
	}
	LEAVE;
} /* resize_mapping_colour_or_auxili */

static void identify_mapping_colour_scroll(Widget *widget_id,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 December 1996

DESCRIPTION :
Finds the id of the colour_or_auxiliary_scroll_bar.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_colour_scroll);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
		mapping->colour_or_auxiliary_scroll_bar= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_colour_scroll.  mapping_window missing");
	}
	LEAVE;
} /* identify_mapping_colour_scroll */

static void destroy_Mapping_window(Widget widget,XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 February 2000

DESCRIPTION :
This function expects <client_data> to be a pointer to a mapping window.  If the
<address> field of the mapping window is not NULL, <*address> is set to NULL.
If the <activation> field is not NULL, the <activation> widget is unghosted.
The function frees the memory associated with the fields of the mapping window
and frees the memory for the mapping window.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(destroy_Mapping_window);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)client_data)
	{
#if defined (UNEMAP_USE_NODES)
		if(mapping->scene_viewer)
		{
			DESTROY(Scene_viewer)(&(mapping->scene_viewer));
		}
#endif /*defined (UNEMAP_USE_NODES)*/
		if (mapping->potential_time_object)
		{
			DEACCESS(Time_object)(&(mapping->potential_time_object));
		}
		/* set the pointer to the mapping_window to NULL */
		if (mapping->address)
		{
			*(mapping->address)=(struct Mapping_window *)NULL;
		}
		if ((mapping->current_mapping_window_address)&&
			(mapping== *(mapping->current_mapping_window_address)))
		{
			*(mapping->current_mapping_window_address)=(struct Mapping_window *)NULL;
			if (mapping->open)
			{
				*(mapping->open)=0;
			}
		}
		/* unghost the activation widget */
		if (mapping->activation)
		{
			XtSetSensitive(mapping->activation,True);
		}
		/*??? destroy map dialog */
		/*??? destroy setup dialog */
		/* destroy the map */
		destroy_Map(&(mapping->map));
		/* destroy map drawing */
		destroy_Drawing_2d(&(mapping->map_drawing));
		/* free the mapping window memory */
		DEALLOCATE(mapping);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_Mapping_window.  client_data missing");
	}
	LEAVE;
} /* destroy_Mapping_window */

static int write_map_postscript_file(char *file_name,void *mapping_window)
/*******************************************************************************
LAST MODIFIED : 21 June 1997

DESCRIPTION :
This function writes the postscript for drawing the map associated with the
mapping_window.
==============================================================================*/
{
	float page_height,page_width,pixel_aspect_ratio,postscript_page_height,
		postscript_page_width;
	int return_code;
	struct Map *map;
	struct Mapping_window *mapping;
	struct Printer printer;
	XWindowAttributes window_attributes;

	ENTER(write_map_postscript_file);
	if ((mapping=(struct Mapping_window *)mapping_window)&&(map=mapping->map)&&
		(mapping->user_interface)&&(map->drawing_information))
	{
		if (open_printer(&printer,mapping->user_interface))
		{
			if (XGetWindowAttributes(mapping->user_interface->display,
				XtWindow(mapping->map_drawing_area_2d),&window_attributes))
			{
				/* open the postscript file */
				if (open_postscript(file_name,PORTRAIT,window_attributes.colormap,
					map->drawing_information->spectrum_colours,
					map->drawing_information->number_of_spectrum_colours,
					map->drawing_information->background_drawing_colour,
					&(printer.background_colour),printer.foreground_colour_pixel,
					map->drawing_information->font,(float)(printer.page_width_mm),
					(float)(printer.page_height_mm),(float)(printer.page_left_margin_mm),
					(float)(printer.page_right_margin_mm),
					(float)(printer.page_top_margin_mm),
					(float)(printer.page_bottom_margin_mm),mapping->user_interface))
				{
					map->print=1;
					get_postscript_page_size(&postscript_page_width,
						&postscript_page_height);
					if ((SHOW_COLOUR==map->colour_option)&&(map->print_spectrum))
					{
						/* show colour bar */
						/* set the area of the postscript page for the map */
						if (map->maintain_aspect_ratio)
						{
							pixel_aspect_ratio=get_pixel_aspect_ratio(
								mapping->user_interface->display);
							if ((float)(mapping->map_drawing->height)*pixel_aspect_ratio*
								postscript_page_width<0.85*postscript_page_height*
								(float)(mapping->map_drawing->width))
							{
								page_height=postscript_page_width*pixel_aspect_ratio*
									(float)(mapping->map_drawing->height)/
									(float)(mapping->map_drawing->width);
								set_postscript_display_transfor(0,0.85*postscript_page_height-
									page_height,postscript_page_width,page_height,0,0,
									(float)(mapping->map_drawing->width),
									(float)(mapping->map_drawing->height));
							}
							else
							{
								page_width=0.85*postscript_page_height*
									(float)(mapping->map_drawing->width)/(pixel_aspect_ratio*
									(float)(mapping->map_drawing->height));
								set_postscript_display_transfor(
									(postscript_page_width-page_width)/2,0,page_width,
									0.85*postscript_page_height,0,0,
									(float)(mapping->map_drawing->width),
									(float)(mapping->map_drawing->height));
							}
						}
						else
						{
							set_postscript_display_transfor(0,0,postscript_page_width,
								0.85*postscript_page_height,0,0,
								(float)(mapping->map_drawing->width),
								(float)(mapping->map_drawing->height));
						}
						/* draw the map to the postscript page */
						draw_map(map,0,mapping->map_drawing);
						/* set the area of the postscript page for the colour bar */
						set_postscript_display_transfor(0,0.9*postscript_page_height,
							postscript_page_width,0.1*postscript_page_height,0,0,
							(float)(mapping->colour_or_auxiliary_drawing->width),
							(float)(mapping->colour_or_auxiliary_drawing->height));
						/* draw the colour bar to the postscript page */
						draw_colour_or_auxiliary_area(map,
							mapping->colour_or_auxiliary_drawing);
					}
					else
					{
						/* no colour bar */
						/* set the area of the postscript page for the map */
						if (map->maintain_aspect_ratio)
						{
							pixel_aspect_ratio=get_pixel_aspect_ratio(
								mapping->user_interface->display);
							if ((float)(mapping->map_drawing->height)*pixel_aspect_ratio*
								postscript_page_width<postscript_page_height*
								(float)(mapping->map_drawing->width))
							{
								page_height=postscript_page_width*pixel_aspect_ratio*
									(float)(mapping->map_drawing->height)/
									(float)(mapping->map_drawing->width);
								set_postscript_display_transfor(0,postscript_page_height-
									page_height,postscript_page_width,page_height,0,0,
									(float)(mapping->map_drawing->width),
									(float)(mapping->map_drawing->height));
							}
							else
							{
								page_width=postscript_page_height*
									(float)(mapping->map_drawing->width)/(pixel_aspect_ratio*
									(float)(mapping->map_drawing->height));
								set_postscript_display_transfor(
									(postscript_page_width-page_width)/2,0,page_width,
									postscript_page_height,0,0,
									(float)(mapping->map_drawing->width),
									(float)(mapping->map_drawing->height));
							}
						}
						else
						{
							set_postscript_display_transfor(0,0,postscript_page_width,
								postscript_page_height,0,0,(float)(mapping->map_drawing->width),
								(float)(mapping->map_drawing->height));
						}
						/* draw the map to the postscript page */
						draw_map(map,0,mapping->map_drawing);
					}
					/* write the ending for the postscript file */
					(void)close_postscript();
					map->print=0;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"write_map_postscript_file.  Could not open file: %s",file_name);
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"write_map_postscript_file.  Could not get colour map");
				return_code=0;
			}
			close_printer(&printer);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_map_postscript_file.  Could not open printer");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_map_postscript_file.  Missing mapping_window or map");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_map_postscript_file */

static int write_map_rgb_file(char *file_name,void *mapping_window)
/*******************************************************************************
LAST MODIFIED : 10 August 1998

DESCRIPTION :
This function writes the rgb for drawing the map associated with the
mapping_window.
==============================================================================*/
{
	int return_code;
	struct Mapping_window *mapping;
	unsigned long *image;

	ENTER(write_map_rgb_file);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
		if (image=get_Drawing_2d_image(mapping->map_drawing))
		{
			/* write the file */
			return_code=write_rgb_image_file(file_name,/*components*/3,
				/*bytes_per_component*/1,
				mapping->map_drawing->height,mapping->map_drawing->width,0,image);
			DEALLOCATE(image);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_map_rgb_file.  Could not get image");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_map_rgb_file.  Missing mapping_window or map");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_map_rgb_file */

static int write_map_tiff_file(char *file_name,void *mapping_window)
/*******************************************************************************
LAST MODIFIED : 10 August 1998

DESCRIPTION :
This function writes the tiff for drawing the map associated with the
mapping_window.
==============================================================================*/
{
	int return_code;
	struct Mapping_window *mapping;
	unsigned long *image;

	ENTER(write_map_tiff_file);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
		if (image=get_Drawing_2d_image(mapping->map_drawing))
		{
			/* write the file */
			return_code=write_tiff_image_file(file_name,/*components*/3,
				/*bytes_per_component*/1,
				mapping->map_drawing->height,mapping->map_drawing->width,
				0,TIFF_PACK_BITS_COMPRESSION,image);
			DEALLOCATE(image);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_map_tiff_file.  Could not get image");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_map_tiff_file.  Missing mapping_window or map");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_map_tiff_file */

static int write_map_animation_files(char *file_name,void *mapping_window,
	enum Image_file_format image_file_format)
/*******************************************************************************
LAST MODIFIED : 23 July 1998

DESCRIPTION :
This function writes the rgb files for drawing the animation associated with the
mapping_window.
==============================================================================*/
{
	char *temp_char1,*temp_char2,*temp_file_name;
	Colormap colour_map;
	Display *display;
	float contour_maximum,contour_minimum,maximum_value,minimum_value;
	int cell_number,frame_number,i,number_of_contours,number_of_digits,
		number_of_frames,number_of_spectrum_colours,return_code;
	Pixel *spectrum_pixels;
	struct Drawing_2d *drawing;
	struct Map *map;
	struct Map_drawing_information *drawing_information;
	struct Mapping_window *mapping;
	XColor colour,spectrum_rgb[MAX_SPECTRUM_COLOURS];

	ENTER(write_map_animation_files);
	if ((mapping=(struct Mapping_window *)mapping_window)&&(map=mapping->map)&&
		(map->type)&&((SINGLE_ACTIVATION== *(map->type))||
		((MULTIPLE_ACTIVATION== *(map->type))&&
		(NO_INTERPOLATION==map->interpolation_type))||
		(POTENTIAL== *(map->type)))&&
		(drawing_information=map->drawing_information)&&(mapping->user_interface)&&
		(mapping->user_interface==drawing_information->user_interface)&&
		(drawing=mapping->map_drawing)&&file_name)
	{
		return_code=1;
		if (SINGLE_ACTIVATION== *(map->type))
		{
			number_of_frames=drawing_information->number_of_spectrum_colours;
			number_of_spectrum_colours=
				drawing_information->number_of_spectrum_colours;
			display=drawing_information->user_interface->display;
			colour_map=drawing_information->colour_map;
			spectrum_pixels=drawing_information->spectrum_colours;
			map->activation_front=0;
		}
		else
		{
			if (MULTIPLE_ACTIVATION== *(map->type))
			{
				number_of_frames= *(map->end_search_interval)-
					*(map->start_search_interval);
				map->activation_front= *(map->datum);
				*(map->datum)= *(map->start_search_interval);
			}
			else
			{
				if (POTENTIAL== *(map->type))
				{
					if (NO_INTERPOLATION==map->interpolation_type)
					{
						number_of_frames= *(map->end_search_interval)-
							*(map->start_search_interval);
						map->activation_front= *(map->potential_time);
						*(map->potential_time)= *(map->start_search_interval);
					}
					else
					{
						number_of_frames=map->number_of_frames;
						map->activation_front=map->frame_number;
						map->frame_number=0;
					}
				}
			}
		}
		number_of_digits=0;
		i=number_of_frames;
		do
		{
			i /= 10;
			number_of_digits++;
		} while (i>0);
		if (ALLOCATE(temp_file_name,char,strlen(file_name)+number_of_digits+1))
		{
			strcpy(temp_file_name,file_name);
			if (temp_char1=strchr(temp_file_name,'.'))
			{
				temp_char2=strchr(file_name,'.');
			}
			else
			{
				temp_char1=temp_file_name+strlen(temp_file_name);
				temp_char2=(char *)NULL;
			}
			for (frame_number=0;frame_number<number_of_frames;frame_number++)
			{
				if (SINGLE_ACTIVATION== *(map->type))
				{
					if (drawing_information->read_only_colour_map)
					{
						update_mapping_drawing_area(mapping,0);
						update_mapping_colour_or_auxili(mapping);
					}
					else
					{
						/* use background drawing colour for the whole spectrum */
						colour.pixel=drawing_information->background_drawing_colour;
						XQueryColor(display,colour_map,&colour);
						for (i=0;i<number_of_spectrum_colours;i++)
						{
							spectrum_rgb[i].pixel=spectrum_pixels[i];
							spectrum_rgb[i].flags=DoRed|DoGreen|DoBlue;
							spectrum_rgb[i].red=colour.red;
							spectrum_rgb[i].blue=colour.blue;
							spectrum_rgb[i].green=colour.green;
						}
						if ((SHOW_CONTOURS==map->contours_option)&&
							(VARIABLE_THICKNESS==map->contour_thickness))
						{
							colour.pixel=drawing_information->contour_colour;
							XQueryColor(display,colour_map,&colour);
							number_of_contours=map->number_of_contours;
							maximum_value=map->maximum_value;
							minimum_value=map->minimum_value;
							contour_maximum=map->contour_maximum;
							contour_minimum=map->contour_minimum;
							number_of_contours=map->number_of_contours;
							for (i=0;i<number_of_contours;i++)
							{
								cell_number=(int)(((contour_maximum*(float)i+contour_minimum*
									(float)(number_of_contours-1-i))/
									(float)(number_of_contours-1)-minimum_value)/
									(maximum_value-minimum_value)*
									(float)(number_of_spectrum_colours-1)+0.5);
								spectrum_rgb[cell_number].pixel=spectrum_pixels[cell_number];
								spectrum_rgb[cell_number].flags=DoRed|DoGreen|DoBlue;
								spectrum_rgb[cell_number].red=colour.red;
								spectrum_rgb[cell_number].blue=colour.blue;
								spectrum_rgb[cell_number].green=colour.green;
							}
						}
						/* show the activation front */
						colour.pixel=drawing_information->contour_colour;
						XQueryColor(display,colour_map,&colour);
						i=frame_number;
						spectrum_rgb[i].pixel=spectrum_pixels[i];
						spectrum_rgb[i].flags=DoRed|DoGreen|DoBlue;
						spectrum_rgb[i].red=colour.red;
						spectrum_rgb[i].blue=colour.blue;
						spectrum_rgb[i].green=colour.green;
						XStoreColors(display,colour_map,spectrum_rgb,
							number_of_spectrum_colours);
						/* show the map boundary */
						colour.pixel=drawing_information->boundary_colour;
						colour.flags=DoRed|DoGreen|DoBlue;
						XStoreColor(display,colour_map,&colour);
					}
					(map->activation_front)++;
				}
				else
				{
					if (MULTIPLE_ACTIVATION== *(map->type))
					{
						update_mapping_drawing_area(mapping,2);
						update_mapping_colour_or_auxili(mapping);
						/*???DB.  What about the trace window ? */
						(*(map->datum))++;
					}
					else
					{
						if (POTENTIAL== *(map->type))
						{
							if (NO_INTERPOLATION==map->interpolation_type)
							{
								update_mapping_drawing_area(mapping,2);
								update_mapping_colour_or_auxili(mapping);
								/*???DB.  What about the trace window ? */
								(*(map->potential_time))++;
							}
							else
							{
								update_mapping_drawing_area(mapping,0);
								(map->frame_number)++;
							}
						}
					}
				}
				if (temp_char2)
				{
					sprintf(temp_char1,"%0*d%s",number_of_digits,frame_number+1,
						temp_char2);
				}
				else
				{
					sprintf(temp_char1,"%0*d",number_of_digits,frame_number+1);
				}
				switch (image_file_format)
				{
					case POSTSCRIPT_FILE_FORMAT:
					{
						write_map_postscript_file(temp_file_name,mapping_window);
					} break;
					case RGB_FILE_FORMAT:
					{
						write_map_rgb_file(temp_file_name,mapping_window);
					} break;
					case TIFF_FILE_FORMAT:
					{
						write_map_tiff_file(temp_file_name,mapping_window);
					} break;
				}
			}
			DEALLOCATE(temp_file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_map_animation_files.  Could not allocate file name");
			return_code=0;
		}
		if (SINGLE_ACTIVATION== *(map->type))
		{
			map->activation_front= -1;
			if (drawing_information->read_only_colour_map)
			{
				update_mapping_drawing_area(mapping,0);
				update_mapping_colour_or_auxili(mapping);
			}
			else
			{
				(void)update_colour_map_unemap(map,drawing);
			}
		}
		else
		{
			if (MULTIPLE_ACTIVATION== *(map->type))
			{
				*(map->datum)=map->activation_front;
				map->activation_front= -1;
				update_mapping_drawing_area(mapping,2);
				update_mapping_colour_or_auxili(mapping);
				/*???DB.  What about the trace window ? */
			}
			else
			{
				if (POTENTIAL== *(map->type))
				{
					*(map->potential_time)=map->activation_front;
					map->activation_front= -1;
					update_mapping_drawing_area(mapping,2);
					update_mapping_colour_or_auxili(mapping);
					/*???DB.  What about the trace window ? */
				}
				else
				{
					map->frame_number=map->activation_front;
					map->activation_front= -1;
					update_mapping_drawing_area(mapping,0);
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_map_animation_files.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_map_animation_files */

static int write_map_animation_rgb_file(char *file_name,void *mapping_window)
/*******************************************************************************
LAST MODIFIED : 26 June 1998

DESCRIPTION :
This function writes the rgb files for drawing the animation associated with the
mapping_window.
==============================================================================*/
{
	int return_code;

	ENTER(write_map_animation_rgb_file);
	return_code=write_map_animation_files(file_name,mapping_window,
		RGB_FILE_FORMAT);
	LEAVE;

	return (return_code);
} /* write_map_animation_rgb_file */

static int write_map_animation_tiff_file(char *file_name,void *mapping_window)
/*******************************************************************************
LAST MODIFIED : 26 June 1998

DESCRIPTION :
This function writes the tiff files for drawing the animation associated with
the mapping_window.
==============================================================================*/
{
	int return_code;

	ENTER(write_map_animation_tiff_file);
	return_code=write_map_animation_files(file_name,mapping_window,
		TIFF_FILE_FORMAT);
	LEAVE;

	return (return_code);
} /* write_map_animation_tiff_file */


static struct Mapping_window *create_Mapping_window(
	struct Mapping_window **address,char *open,
	struct Mapping_window **current_address,Widget activation,Widget parent,
	struct Map *map,struct Rig **rig_pointer,Pixel identifying_colour,
	int screen_height,char *configuration_file_extension,
	char *postscript_file_extension,
	struct Map_drawing_information *map_drawing_information,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 29 May 2000

DESCRIPTION :
This function allocates the memory for a mapping_window and sets the fields to
the specified values (<address>, <map>).  It then retrieves a mapping window
widget with the specified <parent> and assigns the widget ids to the
appropriate fields of the structure.  If successful it returns a pointer to the
created mapping window and, if <address> is not NULL, makes <*address> point to
the created mapping window.  If unsuccessful, NULL is returned.
==============================================================================*/
{
	Dimension left_margin,right_margin;
	int widget_spacing;
	MrmType mapping_window_class;
	static MrmRegisterArg
		callback_list[]=
		{
			{"destroy_Mapping_window",(XtPointer)destroy_Mapping_window},
			{"identify_mapping_menu",(XtPointer)identify_mapping_menu},
			{"identify_mapping_map_button",(XtPointer)identify_mapping_map_button},
			{"configure_map",(XtPointer)configure_map},
			{"identify_mapping_animate_button",
				(XtPointer)identify_mapping_animate_button},
			{"animate_activation_map",(XtPointer)animate_activation_map},
			{"identify_mapping_setup_button",
				(XtPointer)identify_mapping_setup_button},
			{"setup_simple_rig",(XtPointer)setup_simple_rig},
			{"identify_mapping_page_button",(XtPointer)identify_mapping_page_button},
			{"identify_mapping_modify_button",
				(XtPointer)identify_mapping_modify_button},
			{"identify_mapping_file_button",(XtPointer)identify_mapping_file_button},
			{"identify_mapping_file_save_butt",
				(XtPointer)identify_mapping_file_save_butt},
			{"open_file_and_write",(XtPointer)open_file_and_write},
			{"identify_mapping_file_read_butt",
				(XtPointer)identify_mapping_file_read_butt},
			{"open_file_and_read",(XtPointer)open_file_and_read},
			{"identify_mapping_file_bard_elec",
				(XtPointer)identify_mapping_file_bard_elec},
			{"identify_mapping_file_default_b",
				(XtPointer)identify_mapping_file_default_b},
			{"set_default_configuration_file",
				(XtPointer)set_default_configuration_file},
			{"identify_mapping_print_button",
				(XtPointer)identify_mapping_print_button},
			{"identify_mapping_print_postscri",
				(XtPointer)identify_mapping_print_postscri},
			{"identify_mapping_print_rgb_butt",
				(XtPointer)identify_mapping_print_rgb_butt},
			{"identify_mapping_print_tiff_but",
				(XtPointer)identify_mapping_print_tiff_but},
			{"id_mapping_print_animation_rgb",
				(XtPointer)id_mapping_print_animation_rgb},
			{"id_mapping_print_animation_tiff",
				(XtPointer)id_mapping_print_animation_tiff},
			{"identify_mapping_region_choice",
				(XtPointer)identify_mapping_region_choice},
			{"identify_mapping_region_pull_do",
				(XtPointer)identify_mapping_region_pull_do},
			{"identify_mapping_region_place_h",
				(XtPointer)identify_mapping_region_place_h},
			{"identify_mapping_projection_cho",
				(XtPointer)identify_mapping_projection_cho},	
			{"identify_mapping_projection_cyl",
				(XtPointer)identify_mapping_projection_cyl},
			{"identify_mapping_projection_ham",
				(XtPointer)identify_mapping_projection_ham},
			{"identify_mapping_projection_pol",
				(XtPointer)identify_mapping_projection_pol},
			{"identify_mapping_projection_pat",
				(XtPointer)identify_mapping_projection_pat},
			{"identify_mapping_projection_3d",
				(XtPointer)identify_mapping_projection_3d},
			{"set_projection_Hammer",(XtPointer)set_projection_Hammer},
			{"set_projection_polar",(XtPointer)set_projection_polar},
			{"set_projection_patch",(XtPointer)set_projection_patch},
			{"set_projection_3d",(XtPointer)set_projection_3d},
			{"set_projection_cylinder",(XtPointer)set_projection_cylinder},
			{"identify_mapping_close_button",
				(XtPointer)identify_mapping_close_button},
			{"identify_mapping_area",(XtPointer)identify_mapping_area},
			{"identify_mapping_area_2d",(XtPointer)identify_mapping_area_2d},
			{"identify_mapping_area_3d",(XtPointer)identify_mapping_area_3d},
			{"identify_map_drawing_area_2d",(XtPointer)identify_mapping_drawing_area_2d},
			{"expose_map_drawing_area_2d",(XtPointer)expose_mapping_drawing_area_2d},
			{"resize_map_drawing_area_2d",(XtPointer)resize_mapping_drawing_area_2d},
			{"identify_mapping_colour_or_auxi",
				(XtPointer)identify_mapping_colour_or_auxi},
			{"expose_mapping_colour_or_auxili",
				(XtPointer)expose_mapping_colour_or_auxili},
			{"resize_mapping_colour_or_auxili",
				(XtPointer)resize_mapping_colour_or_auxili},
			{"identify_mapping_colour_scroll",
				(XtPointer)identify_mapping_colour_scroll}
		},
		global_callback_list[]=
		{
			{"create_simple_rig_from_dialog",
				(XtPointer)create_simple_rig_from_dialog},
			{"update_map_from_dialog",(XtPointer)update_map_from_dialog}
		},
		identifier_list[]=
		{
			{"mapping_window_structure",(XtPointer)NULL},
			{"read_configuration_file_data",(XtPointer)NULL},
			{"read_bard_electrode_file_data",(XtPointer)NULL},
			{"write_configuration_file_data",(XtPointer)NULL},
			{"write_map_postscript_file_data",(XtPointer)NULL},
			{"write_map_rgb_file_data",(XtPointer)NULL},
			{"write_map_tiff_file_data",(XtPointer)NULL},
			{"write_map_animate_rgb_file_data",(XtPointer)NULL},
			{"write_map_animate_tiff_file_dat",(XtPointer)NULL},
			{"mapping_rig",(XtPointer)NULL},
			{"identifying_colour",(XtPointer)NULL}
		};
	struct Mapping_window *mapping;
	Widget child_widget;

	ENTER(create_Mapping_window);
	if (map_drawing_information&&user_interface)
	{
		if (mapping_window_hierarchy_open)
		{
			/* allocate memory */
			if (ALLOCATE(mapping,struct Mapping_window,1))
			{
				/* assign fields */
				mapping->user_interface=user_interface;
				mapping->address=address;
				mapping->open=open;
				mapping->current_mapping_window_address=current_address;
				mapping->activation=activation;
				mapping->map=map;
				mapping->window=(Widget)NULL;
				mapping->menu=(Widget)NULL;
				mapping->map_button=(Widget)NULL;
				mapping->map_dialog=(struct Map_dialog *)NULL;
				mapping->setup_button=(Widget)NULL;
				mapping->setup_dialog=(struct Setup_dialog *)NULL;
				mapping->modify_button=(Widget)NULL;
				mapping->page_button=(Widget)NULL;
				mapping->file_button=(Widget)NULL;
				mapping->file_menu.save_configuration_button=(Widget)NULL;
				mapping->file_menu.read_configuration_button=(Widget)NULL;
				mapping->file_menu.read_bard_electrode_button=(Widget)NULL;
				mapping->file_menu.set_default_configuration_button=(Widget)NULL;
				mapping->projection_choice=(Widget)NULL;
				mapping->projection_cylinder=(Widget)NULL;
				mapping->projection_hammer=(Widget)NULL;
				mapping->projection_polar=(Widget)NULL;
				mapping->projection_patch=(Widget)NULL;
				mapping->projection_3d=(Widget)NULL;
				mapping->region_choice=(Widget)NULL;
				mapping->region_pull_down_menu=(Widget)NULL;
				mapping->number_of_regions=0;
				mapping->regions=(Widget *)NULL;
				mapping->potential_time_object = (struct Time_object *)NULL;
				mapping->time_editor_dialog = (Widget)NULL;
				mapping->print_button=(Widget)NULL;
				(mapping->print_menu).postscript_button=(Widget)NULL;
				(mapping->print_menu).rgb_button=(Widget)NULL;
				(mapping->print_menu).tiff_button=(Widget)NULL;
				(mapping->print_menu).animate_rgb_button=(Widget)NULL;
				(mapping->print_menu).animate_tiff_button=(Widget)NULL;
				mapping->close_button=(Widget)NULL;
				mapping->mapping_area=(Widget)NULL;
				mapping->mapping_area_2d=(Widget)NULL;
				mapping->mapping_area_3d=(Widget)NULL;
				mapping->map_drawing_area_2d=(Widget)NULL;
				mapping->scene_viewer=(struct Scene_viewer *)NULL;
				mapping->map_drawing=(struct Drawing_2d *)NULL;
				mapping->colour_or_auxiliary_drawing_area=(Widget)NULL;
				mapping->colour_or_auxiliary_drawing=(struct Drawing_2d *)NULL;
				mapping->colour_or_auxiliary_scroll_bar=(Widget)NULL;
				/* register the callbacks */
				if ((MrmSUCCESS==MrmRegisterNamesInHierarchy(mapping_window_hierarchy,
					callback_list,XtNumber(callback_list)))&&
					(MrmSUCCESS==MrmRegisterNames(global_callback_list,
					XtNumber(global_callback_list))))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)mapping;
					identifier_list[1].value=(XtPointer)create_File_open_data(
						configuration_file_extension,REGULAR,
						mapping_read_configuration_file,(void *)mapping,0,user_interface);
					identifier_list[2].value=(XtPointer)create_File_open_data(".ele",
						REGULAR,mapping_read_bard_electrode_fil,(void *)mapping,0,
						user_interface);
					identifier_list[3].value=(XtPointer)create_File_open_data(
						configuration_file_extension,REGULAR,write_configuration_file,
						(void *)rig_pointer,0,user_interface);
					identifier_list[4].value=(XtPointer)create_File_open_data(
						postscript_file_extension,REGULAR,write_map_postscript_file,
						(void *)mapping,1,user_interface);
					identifier_list[5].value=(XtPointer)create_File_open_data(".rgb",
						REGULAR,write_map_rgb_file,(void *)mapping,1,user_interface);
					identifier_list[6].value=(XtPointer)create_File_open_data(".tif",
						REGULAR,write_map_tiff_file,(void *)mapping,1,user_interface);
					identifier_list[7].value=(XtPointer)create_File_open_data(".rgb",
						REGULAR,write_map_animation_rgb_file,(void *)mapping,1,
						user_interface);
					identifier_list[8].value=(XtPointer)create_File_open_data(".tif",
						REGULAR,write_map_animation_tiff_file,(void *)mapping,1,
						user_interface);
					identifier_list[9].value=(XtPointer)rig_pointer;
					identifier_list[10].value=(XtPointer)identifying_colour;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(mapping_window_hierarchy,
						identifier_list,XtNumber(identifier_list)))
					{
						/* fetch mapping window widget */
						if (MrmSUCCESS==MrmFetchWidget(mapping_window_hierarchy,
							"mapping_window",parent,&(mapping->window),&mapping_window_class))
						{
							/* other Xt stuff set up in */						
							widget_spacing=user_interface->widget_spacing;
							/* set the height and background colour of the interval drawing
								area */
							XtVaSetValues(mapping->colour_or_auxiliary_drawing_area,
								XmNheight,screen_height/16,XmNbackground,
								map_drawing_information->background_drawing_colour,NULL);
							/* set the background colour of the map drawing area */
							XtVaSetValues(mapping->map_drawing_area_2d,XmNbackground,
								map_drawing_information->background_drawing_colour,NULL);
							/* adjust the projection choice */
							child_widget=XmOptionLabelGadget(mapping->projection_choice);
							XtVaSetValues(child_widget,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							XtVaGetValues(child_widget,
								XmNmarginLeft,&left_margin,
								NULL);
							if (left_margin>widget_spacing)
							{
								XtVaSetValues(mapping->projection_choice,
									XmNleftOffset,0,
									NULL);
							}
							else
							{
								XtVaSetValues(mapping->projection_choice,
									XmNleftOffset,widget_spacing-left_margin,
									NULL);
							}
							child_widget=XmOptionButtonGadget(mapping->projection_choice);
							XtVaSetValues(child_widget,
								XmNshadowThickness,0,
								XmNhighlightThickness,0,
/*???DB								XmNcascadePixmap,no_cascade_pixmap,*/
								XmNalignment,XmALIGNMENT_BEGINNING,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							XtVaGetValues(child_widget,
								XmNmarginLeft,&right_margin,
								NULL);
							/* adjust the region choice */
							child_widget=XmOptionLabelGadget(mapping->region_choice);
							XtVaSetValues(child_widget,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							XtVaGetValues(child_widget,
								XmNmarginLeft,&left_margin,
								NULL);
							if (left_margin+right_margin>widget_spacing)
							{
								XtVaSetValues(mapping->region_choice,
									XmNleftOffset,0,
									NULL);
							}
							else
							{
								XtVaSetValues(mapping->region_choice,
									XmNleftOffset,widget_spacing-(left_margin+right_margin),
									NULL);
							}
							child_widget=XmOptionButtonGadget(mapping->region_choice);
							XtVaSetValues(child_widget,
								XmNshadowThickness,0,
								XmNhighlightThickness,0,
/*???DB								XmNcascadePixmap,no_cascade_pixmap,*/
								XmNalignment,XmALIGNMENT_BEGINNING,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							update_mapping_window_menu(mapping);
							/*??? more to do ? */
							if (address)
							{
								*address=mapping;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_Mapping_window.  Could not fetch mapping window");
							DEALLOCATE(mapping);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_Mapping_window.  Could not register identifiers");
						DEALLOCATE(mapping);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Mapping_window.  Could not register callbacks");
					DEALLOCATE(mapping);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
				"create_Mapping_window.  Could not allocate mapping window structure");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Mapping_window.  Hierarchy not open");
			mapping=(struct Mapping_window *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Mapping_window.  Missing user_interface");
		mapping=(struct Mapping_window *)NULL;
	}
	LEAVE;

	return (mapping);
} /* create_Mapping_window */

static Widget create_mapping_window_shell(Widget *address,Widget parent,
	int screen_width,int screen_height,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 19 May 1998

DESCRIPTION :
Creates a popup shell widget for a mapping window.  If <address> is not NULL,
<*address> is set to the id of the created widget and on destruction <*address>
is set to NULL.  The id of the created widget is returned.
???If <address> is NULL, the shell won't be added to the shell list ?
???DB.  screen_width and screen_height should be replaced by shell dimensions
and location.
==============================================================================*/
{
	Widget shell;

	ENTER(create_mapping_window_shell);
	/* create and place the mapping window shell */
	if (shell=XtVaCreatePopupShell(
		"mapping_window_shell",topLevelShellWidgetClass,parent,
		XmNallowShellResize,False,
		XmNx,0,
		XmNy,screen_height/2,
		XmNwidth,screen_width/2,
		XmNheight,screen_height/2,
		XmNuserData,user_interface,
		NULL))
	{
		if (address)
		{
			/* add the destroy callback */
			*address=shell;
			XtAddCallback(shell,XmNdestroyCallback,destroy_window_shell,
				(XtPointer)create_Shell_list_item(address,user_interface));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_mapping_window_shell.  Could not create mapping window shell");
	}
	LEAVE;

	return (shell);
} /* create_mapping_window_shell */

/*
Global functions
----------------
*/
int open_mapping_window(struct Mapping_window **mapping_address,
#if defined (MOTIF)
	Widget activation,Widget parent,Widget *shell,Widget *outer_form,
#endif /* defined (MOTIF) */
	struct Mapping_window **current_mapping_window_address,char *open,
	enum Mapping_associate *current_associate,enum Map_type *map_type,
	enum Colour_option colour_option,enum Contours_option contours_option,
	enum Electrodes_option electrodes_option,enum Fibres_option fibres_option,
	enum Landmarks_option landmarks_option,enum Extrema_option extrema_option,
	int maintain_aspect_ratio,int print_spectrum,
	enum Projection_type projection_type,enum Contour_thickness contour_thickness,
	struct Rig **rig_address,int *event_number_address,
	int *potential_time_address,int *datum_address,int *start_search_interval,
	int *end_search_interval,
#if defined (MOTIF)
	Pixel identifying_colour,
#endif /* defined (MOTIF) */
	enum Mapping_associate associate,
#if defined (MOTIF)
	XtPointer set_mapping_region,XtPointer select_map_drawing_area,
	XtPointer select_colour_or_auxiliary_draw,XtPointer work_area,
#endif /* defined (MOTIF) */
	int screen_width,int screen_height,
	char *configuration_file_extension,char *postscript_file_extension,
	struct Map_drawing_information *map_drawing_information,
	struct User_interface *user_interface,struct Unemap_package *unemap_package)
/*******************************************************************************
LAST MODIFIED : 1 February 2000

DESCRIPTION :
If the mapping window does not exist then it is created with the specified
properties.  Then the mapping window is opened.
==============================================================================*/
{
	int return_code;
	struct Mapping_window *mapping;
	MrmType outer_form_class;

	ENTER(open_mapping_window);
	if (mapping_address&&parent&&rig_address&&shell&&outer_form&&
		current_mapping_window_address&&open&&current_associate&&user_interface
#if defined (UNEMAP_USE_NODES)
		&&unemap_package
#endif /* defined (UNEMAP_USE_NODES) */
		)
	{
		return_code=1;
		if (!(mapping= *mapping_address))
		{
			/* if there is not a mapping window shell */
			if (!(*shell))
			{
				/* create the mapping window shell */
				if (create_mapping_window_shell(shell,parent,screen_width,
					screen_height,user_interface))
				{
					if (MrmOpenHierarchy_base64_string(mapping_window_uidh,
						&mapping_window_hierarchy,
						&mapping_window_hierarchy_open))
							/*???DB.  Should this be in create_mapping_window_shell ? */
					{
						/* retrieve the outer form */
						if (MrmSUCCESS==MrmFetchWidget(mapping_window_hierarchy,
							"mapping_window_outer_form",*shell,outer_form,&outer_form_class))
						{
							/* manage the outer form */
							XtManageChild(*outer_form);
							/* realize the mapping window shell */
							XtRealizeWidget(*shell);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"open_mapping_window.  Could not retrieve outer form");
							XtDestroyWidget(*shell);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"open_mapping_window.  Could not open hierarchy");
						XtDestroyWidget(*shell);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"open_mapping_window.  Could not create mapping window shell");
					return_code=0;
				}
			}
			/* if there is a mapping window shell */
			if (return_code)
			{
				if (create_Mapping_window(mapping_address,open,
					current_mapping_window_address,activation,*outer_form,
					create_Map(map_type,colour_option,contours_option,electrodes_option,
					fibres_option,landmarks_option,extrema_option,maintain_aspect_ratio,
					print_spectrum,projection_type,contour_thickness,rig_address,
					event_number_address,potential_time_address,datum_address,
					start_search_interval,end_search_interval,map_drawing_information,
					user_interface,unemap_package),
					rig_address,identifying_colour,screen_height,
					configuration_file_extension,postscript_file_extension,
					map_drawing_information,user_interface))
				{
					mapping= *mapping_address;
					XtAddCallback(mapping->region_pull_down_menu,
						XmNentryCallback,(XtCallbackProc)set_mapping_region,work_area);
					switch (associate)
					{
						case ACQUISITION_ASSOCIATE:
						{
							/* unmanage the map button */
							XtUnmanageChild(mapping->map_button);
							/* unmanage the animate button */
							XtUnmanageChild(mapping->animate_button);
							/* adjust the parent of the modify button */
							XtVaSetValues(XtParent(mapping->modify_button),
								XmNleftWidget,mapping->setup_button,
								NULL);
							if (*rig_address)
							{
								if (EXPERIMENT_ON==(*rig_address)->experiment)
								{
									/* ghost the file button */
									XtSetSensitive(mapping->file_button,False);
								}
							}
							else
							{
								/* ghost the save configuration button in the file menu */
								XtSetSensitive(mapping->file_menu.save_configuration_button,
									False);
								/* ghost the set default configuration button in the file
									menu */
								XtSetSensitive(mapping->file_menu.
									set_default_configuration_button,False);
							}
							/* ghost the print animation button */
							XtSetSensitive(mapping->print_menu.animate_rgb_button,False);
							XtSetSensitive(mapping->print_menu.animate_tiff_button,False);
						} break;
						case ANALYSIS_ASSOCIATE:
						{
							/*??? will eventually be for both analysis and acquisition */
							XtAddCallback(mapping->map_drawing_area_2d,XmNinputCallback,
								(XtCallbackProc)select_map_drawing_area,work_area);
							XtAddCallback(mapping->colour_or_auxiliary_drawing_area,
								XmNinputCallback,
								(XtCallbackProc)select_colour_or_auxiliary_draw,work_area);
							/* unmanage the mapping setup button */
							XtUnmanageChild(mapping->setup_button);
							/* adjust the parent of the modify button */
							XtVaSetValues(XtParent(mapping->modify_button),
								XmNleftWidget,mapping->animate_button,
								NULL);
							/* unmanage the mapping modify button */
							XtUnmanageChild(mapping->modify_button);
							/* unmanage the mapping page button */
							XtUnmanageChild(mapping->page_button);
							/* unmanage the read configuration button in the mapping
								file menu */
							XtUnmanageChild(mapping->file_menu.read_configuration_button);
							if (*rig_address)
							{
								/* ghost the mapping file button */
								XtSetSensitive(mapping->file_button,True);
							}
							else
							{
								/* ghost the mapping file button */
								XtSetSensitive(mapping->file_button,False);
							}
							if (!(mapping->map)||!(mapping->map->type)||
								!((SINGLE_ACTIVATION== *(mapping->map->type))||
								((MULTIPLE_ACTIVATION== *(mapping->map->type))&&
								(NO_INTERPOLATION==mapping->map->interpolation_type))||
								(POTENTIAL== *(mapping->map->type))))
							{
								XtSetSensitive(mapping->animate_button,False);
								/* ghost the print animation button */
								XtSetSensitive(mapping->print_menu.animate_rgb_button,False);
								XtSetSensitive(mapping->print_menu.animate_tiff_button,False);
							}
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"open_mapping_window.  Invalid mapping associate");
						} break;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"open_mapping_window.  Could not create mapping window");
					return_code=0;
				}
			}
		}
		if (mapping&&return_code)
		{
			*current_associate=associate;
			if (*current_mapping_window_address!=mapping)
			{
				if (*current_mapping_window_address)
				{
					/* unmanage the current mapping window */
					XtUnmanageChild((*current_mapping_window_address)->window);
				}
				/* manage the new mapping window */
				XtManageChild(mapping->window);
				*current_mapping_window_address=mapping;
			}
			/* check if the mapping window shell has been popped up */
			if (!(*open))
			{
				*open=1;
				/* pop up the mapping window shell */
				XtPopup(*shell,XtGrabNone);
				/* ghost the system mapping button */
				XtSetSensitive(activation,False);
			}
			mapping_window_update_time_limits(mapping);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"open_mapping_window.  Missing argument(s)");
	}
	LEAVE;

	return (return_code);
} /* open_mapping_window */

static int update_mapping_drawing_area_2d(struct Mapping_window *mapping,int recalculate)
/*******************************************************************************
LAST MODIFIED : 30 April 1999

DESCRIPTION :
This function for redrawing the <mapping> drawing area.  If <recalculate> is >0
then the colours for the pixels are recalculated.  If <recalculate> is >1 then
the interpolation functions are also recalculated.
==============================================================================*/
{
	int return_code;
	struct Drawing_2d *drawing;
	struct Map_drawing_information *drawing_information;

	ENTER(update_mapping_drawing_area);
	if (mapping&&
		(drawing=mapping->map_drawing)&&
		(mapping->map_drawing_area_2d)&&
		(drawing_information=mapping->map->drawing_information)&&
		(drawing_information->user_interface))
	{
		/* clear the map drawing area */
		XFillRectangle(drawing_information->user_interface->display,
			drawing->pixel_map,(drawing_information->graphics_context).
			background_drawing_colour,0,0,drawing->width,drawing->height);
		/* draw the map */
		draw_map(mapping->map,recalculate,drawing);
		XCopyArea(drawing_information->user_interface->display,
			drawing->pixel_map,XtWindow(mapping->map_drawing_area_2d),
			(drawing_information->graphics_context).copy,0,0,
			drawing->width,drawing->height,0,0);
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* update_mapping_drawing_area_2d */

int update_mapping_drawing_area(struct Mapping_window *mapping,int recalculate)
/*******************************************************************************
LAST MODIFIED : 30 May 2000

DESCRIPTION :
Calls draw_map_3d or update_mapping_drawing_area_2d depending upon
<mapping> ->map->projection_type
==============================================================================*/
{
	int return_code;

	ENTER(update_mapping_drawing_area);
	if(mapping)
	{
#if defined (UNEMAP_USE_NODES) 
		/* 3d map for 3d projection */
		if(mapping->map->projection_type==THREED_PROJECTION)
		{			
			return_code=draw_map(mapping->map,recalculate,mapping->map_drawing);
		}
		else
		{
			return_code=update_mapping_drawing_area_2d(mapping,recalculate);
		}	
#else
		/* old, 2d map*/
		update_mapping_drawing_area_2d(mapping,recalculate);
#endif /*defined( UNEMAP_USE_NODES) */	
	}
	else
	{		
		return_code=0;
	}	
	LEAVE;
	return(return_code);
}/* update_mapping_drawing_area */

int update_mapping_colour_or_auxili(struct Mapping_window *mapping)
/*******************************************************************************
LAST MODIFIED : 9 February 1997

DESCRIPTION :
The callback for redrawing the colour bar or auxiliary devices drawing area.
==============================================================================*/
{
	int return_code;
	struct Drawing_2d *drawing;
	struct Map_drawing_information *drawing_information;

	ENTER(update_mapping_colour_or_auxili);
	if (mapping&&(mapping->map)&&(drawing=mapping->colour_or_auxiliary_drawing)&&
		(mapping->colour_or_auxiliary_drawing_area)&&
		(drawing_information=mapping->map->drawing_information)&&
		(drawing_information->user_interface))
	{
		/* clear the colour or auxiliary area */
		XFillRectangle(drawing_information->user_interface->display,
			drawing->pixel_map,
			(drawing_information->graphics_context).background_drawing_colour,
			0,0,drawing->width,drawing->height);
		/* draw the colour bar or the auxiliary devices */
		draw_colour_or_auxiliary_area(mapping->map,drawing);
		XCopyArea(drawing_information->user_interface->display,
			mapping->colour_or_auxiliary_drawing->pixel_map,
			XtWindow(mapping->colour_or_auxiliary_drawing_area),
			(drawing_information->graphics_context).copy,0,0,
			mapping->colour_or_auxiliary_drawing->width,
			mapping->colour_or_auxiliary_drawing->height,0,0);
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* update_mapping_colour_or_auxili */

int update_mapping_window_menu(struct Mapping_window *mapping)
/*******************************************************************************
LAST MODIFIED : 21 June 1997

DESCRIPTION :
Updates the mapping region pull down menu to be consistent with the current rig.
==============================================================================*/
{
	Widget current_projection,current_region;
	WidgetList regions;
	int number_of_regions,return_code;
#define NUMBER_OF_ATTRIBUTES 2
	Arg attributes[NUMBER_OF_ATTRIBUTES];
	struct Region *region=(struct Region *)NULL;
	struct Region_list_item *region_item;
	struct Rig *rig;
	Window mapping_menu_window;

	ENTER(update_mapping_window_menu);
	if (mapping)
	{
		if (mapping_menu_window=XtWindow(mapping->menu))
		{
			XtUnrealizeWidget(mapping->menu);
		}
		/* remove the current entries from the option menu */
		if (((number_of_regions=mapping->number_of_regions)>0)&&
			(regions=mapping->regions))
		{
			while (number_of_regions>0)
			{
				XtUnmanageChild(*regions);
				XtDestroyWidget(*regions);
				regions++;
				number_of_regions--;
			}
			DEALLOCATE(mapping->regions);
			mapping->number_of_regions=0;
			mapping->current_region=(Widget)NULL;
			return_code=1;
		}
		else
		{
			return_code=1;
		}
		if (return_code)
		{
			if ((rig= *(mapping->map->rig_pointer))&&
				((number_of_regions=rig->number_of_regions)>0))
			{
				/* put the regions for the new rig into the option menu */
				if (number_of_regions>1)
				{
					number_of_regions++;
				}
				if (ALLOCATE(regions,Widget,number_of_regions))
				{
					mapping->number_of_regions=number_of_regions;
					mapping->regions=regions;
					current_region=(Widget)NULL;
					XtSetArg(attributes[1],XmNfontList,XmFontListCreate(mapping->
						user_interface->button_font,XmSTRING_DEFAULT_CHARSET));
					if (number_of_regions>1)
					{
						XtSetArg(attributes[0],XmNlabelString,
							(XtPointer)XmStringCreate("All regions",
							XmSTRING_DEFAULT_CHARSET));
						if (*regions=XmCreatePushButtonGadget(
							mapping->region_pull_down_menu,"All regions",attributes,
							NUMBER_OF_ATTRIBUTES))
						{
							XtManageChild(*regions);
							if (!(get_Rig_current_region(rig)))
							{
								current_region= *regions;
							}
							regions++;
						}
						else
						{
							return_code=0;
						}
					}
					region_item=get_Rig_region_list(rig);
					while (region_item&&return_code)
					{
						region=get_Region_list_item_region(region_item);
						XtSetArg(attributes[0],XmNlabelString,
							(XtPointer)XmStringCreate(
							region->name,XmSTRING_DEFAULT_CHARSET));
						if (*regions=XmCreatePushButtonGadget(
							mapping->region_pull_down_menu,region->name,
							attributes,NUMBER_OF_ATTRIBUTES))
						{
							XtManageChild(*regions);
							if ((get_Rig_current_region(rig)==region)||
								((!current_region)&&(!(get_Rig_current_region(rig)))))
							{
								current_region= *regions;
							}
							region_item=get_Region_list_item_next(region_item);
							regions++;
						}
						else
						{
							return_code=0;
						}
					}
					if (return_code)
					{

#if defined (OLD_CODE)
						/* update projection choice */
						if ((get_Rig_current_region(rig))&&(SOCK==rig->current_region->type))
						{
							XtManageChild(mapping->projection_choice);
							XtVaSetValues(mapping->region_choice,
								XmNleftWidget,mapping->projection_choice,
								NULL);
						}
						else
						{
							XtUnmanageChild(mapping->projection_choice);
							XtVaSetValues(mapping->region_choice,
								XmNleftWidget,mapping->print_button,
								NULL);
						}					
#endif /* defined (OLD_CODE)	*/
#if defined (UNEMAP_USE_NODES)						
						XtManageChild(mapping->projection_choice);
						XtVaSetValues(mapping->region_choice,
							XmNleftWidget,mapping->projection_choice,
							NULL);
						/* now need to hide the inappropriate choices */
						/*all off*/
						XtSetSensitive(mapping->projection_cylinder,False);
						XtSetSensitive(mapping->projection_hammer,False);
						XtSetSensitive(mapping->projection_polar,False);
						XtSetSensitive(mapping->projection_3d,False);
						XtSetSensitive(mapping->projection_patch,False);
						if(rig->current_region)
						{
							switch(rig->current_region->type)
							{
								case SOCK:
								{
									XtSetSensitive(mapping->projection_hammer,True);
									XtSetSensitive(mapping->projection_polar,True);
									XtSetSensitive(mapping->projection_3d,True);
									/* set the projection choic */
									XtVaGetValues(mapping->projection_choice,XmNmenuHistory,
										&current_projection,NULL);
									if ((current_projection!=mapping->projection_hammer)&&
										(current_projection!=mapping->projection_polar)
										&&(current_projection!=mapping->projection_3d))
									{
										XtVaSetValues(mapping->projection_choice,
											XmNmenuHistory,mapping->projection_hammer,
											NULL);
									}
								}break;
								case TORSO:
								{
									XtSetSensitive(mapping->projection_cylinder,True);
									XtSetSensitive(mapping->projection_3d,True);	
									/* set the projection choice */
									XtVaGetValues(mapping->projection_choice,XmNmenuHistory,
										&current_projection,NULL);
									if ((current_projection!=mapping->projection_cylinder)
										&&(current_projection!=mapping->projection_3d))
									{
										XtVaSetValues(mapping->projection_choice,
											XmNmenuHistory,mapping->projection_cylinder,
											NULL);
									}
								}break;
								case PATCH:
								{									
									XtSetSensitive(mapping->projection_patch,True);
									XtSetSensitive(mapping->projection_3d,True);
									/* set the projection choice */
									XtVaGetValues(mapping->projection_choice,XmNmenuHistory,
										&current_projection,NULL);
									if ((current_projection!=mapping->projection_patch)									
										&&(current_projection!=mapping->projection_3d))
									{
										XtVaSetValues(mapping->projection_choice,
											XmNmenuHistory,mapping->projection_patch,
											NULL);
									}
								}break;
							}/* switch(rig->current_region->type) */ 
						}/*  if(rig->current_region) */
						else
						/*This is a mixed rig */
						{
							XtSetSensitive(mapping->projection_hammer,True);
							XtSetSensitive(mapping->projection_polar,True);
							XtSetSensitive(mapping->projection_3d,True);
							/* set the projection choic */
							XtVaGetValues(mapping->projection_choice,XmNmenuHistory,
								&current_projection,NULL);
							if ((current_projection!=mapping->projection_hammer)&&
								(current_projection!=mapping->projection_polar)
								&&(current_projection!=mapping->projection_3d))
							{
								XtVaSetValues(mapping->projection_choice,
									XmNmenuHistory,mapping->projection_hammer,
									NULL);
							}
						}
#else 
						XtManageChild(mapping->projection_choice);
						XtVaSetValues(mapping->region_choice,
							XmNleftWidget,mapping->projection_choice,NULL);
						/* now need to hide the inappropriate choices */
						/*all off*/
						XtSetSensitive(mapping->projection_cylinder,False);
						XtSetSensitive(mapping->projection_hammer,False);
						XtSetSensitive(mapping->projection_polar,False);
						XtSetSensitive(mapping->projection_3d,False);
						XtSetSensitive(mapping->projection_patch,False);
						if(rig->current_region)
						{
							switch(rig->current_region->type)
							{
								case SOCK:
								{
									XtSetSensitive(mapping->projection_hammer,True);
									XtSetSensitive(mapping->projection_polar,True);							
									/* set the projection choic */
									XtVaGetValues(mapping->projection_choice,XmNmenuHistory,
										&current_projection,NULL);
									if ((current_projection!=mapping->projection_hammer)&&
										(current_projection!=mapping->projection_polar))
									{
										XtVaSetValues(mapping->projection_choice,
											XmNmenuHistory,mapping->projection_hammer,
											NULL);
									}
								}break;
								case TORSO:
								{
									XtSetSensitive(mapping->projection_cylinder,True);								
									/* set the projection choice */
									XtVaGetValues(mapping->projection_choice,XmNmenuHistory,
										&current_projection,NULL);
									if ((current_projection!=mapping->projection_cylinder))
									{
										XtVaSetValues(mapping->projection_choice,
											XmNmenuHistory,mapping->projection_cylinder,
											NULL);
									}
								}break;
								case PATCH:
								{									
									XtSetSensitive(mapping->projection_patch,True);							
									/* set the projection choice */
									XtVaGetValues(mapping->projection_choice,XmNmenuHistory,
										&current_projection,NULL);
									if ((current_projection!=mapping->projection_patch))
									{
										XtVaSetValues(mapping->projection_choice,
											XmNmenuHistory,mapping->projection_patch,
											NULL);
									}
								}break;
							}/* switch(rig->current_region->type) */
						}
						else
						/*This is a mixed rig */
						{
							XtSetSensitive(mapping->projection_hammer,True);
							XtSetSensitive(mapping->projection_polar,True);						
							/* set the projection choic */
							XtVaGetValues(mapping->projection_choice,XmNmenuHistory,
								&current_projection,NULL);
							if ((current_projection!=mapping->projection_hammer)&&
								(current_projection!=mapping->projection_polar))
							{
								XtVaSetValues(mapping->projection_choice,
									XmNmenuHistory,mapping->projection_hammer,
									NULL);
							}
						}
#endif /* defined (UNEMAP_USE_NODES) */
						XtVaSetValues(mapping->region_choice,
							XmNmenuHistory,current_region,
							NULL);
						mapping->current_region=current_region;
						XtManageChild(mapping->region_choice);
					}
					else
					{
						display_message(ERROR_MESSAGE,
			"update_mapping_window_menu.  Could not create region PushButtonGadget");
						/*???What about destroying PushButtons already created ? */
						DEALLOCATE(mapping->regions);
						mapping->number_of_regions=0;
						mapping->current_region=(Widget)NULL;
						/* update projection choice */
						XtUnmanageChild(mapping->projection_choice);
						XtVaSetValues(mapping->region_choice,
							XmNleftWidget,mapping->print_button,
							NULL);
						/* update region choice */
						XtUnmanageChild(mapping->region_choice);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"update_mapping_window_menu.  Insufficient memory for region list");
					/* update projection choice */
					XtUnmanageChild(mapping->projection_choice);
					XtVaSetValues(mapping->region_choice,
						XmNleftWidget,mapping->print_button,
						NULL);
					/* update region choice */
					XtUnmanageChild(mapping->region_choice);
					return_code=0;
				}
			}
			else
			{
				/* update projection choice */
				XtUnmanageChild(mapping->projection_choice);
				XtVaSetValues(mapping->region_choice,
					XmNleftWidget,mapping->print_button,
					NULL);
				/* update region choice */
				XtUnmanageChild(mapping->region_choice);
			}
		}
		if (mapping_menu_window&&return_code)
		{
			XtRealizeWidget(mapping->menu);
			XtManageChild(mapping->menu);
		}
	}
	LEAVE;

	return (return_code);
} /* update_mapping_window_menu */

int highlight_electrode_or_auxiliar(struct Device *device,int electrode_number,
	int auxiliary_number,struct Map *map,struct Mapping_window *mapping)
/*******************************************************************************
LAST MODIFIED : 23 July 1998

DESCRIPTION :
Highlights/dehighlights an electrode or an auxiliary device in the <mapping>
window.
==============================================================================*/
{
	char electrode_drawn,*name,value_string[11];
	Display *display;
	enum Map_type map_type;
	float f_value,max_f,min_f,range_f;
	GC graphics_context;
	int ascent,descent,direction,marker_size,name_length,
		number_of_spectrum_colours,return_code,xmax,xmin,xpos,xstart,ymax,ymin,ypos,
		ystart;
	Pixel *spectrum_pixels;
	struct Map_drawing_information *drawing_information;
	XCharStruct bounds;
	XFontStruct *font;

	ENTER(highlight_electrode_or_auxiliar);
	return_code=0;
	if (map&&mapping&&(drawing_information=map->drawing_information)&&
		(drawing_information->user_interface))
	{
		display=drawing_information->user_interface->display;
		font=drawing_information->font;
		number_of_spectrum_colours=drawing_information->number_of_spectrum_colours;
		spectrum_pixels=drawing_information->spectrum_colours;
		map_type= *(map->type);
		if (map&&(map->electrodes_option!=HIDE_ELECTRODES)&&
			(electrode_number>=0)&&(map->electrode_drawn)&&
			((map->electrode_drawn)[electrode_number])&&mapping&&
			(mapping->map_drawing_area_2d)&&(mapping->map_drawing))
		{
			switch (map->electrodes_option)
			{
				case SHOW_ELECTRODE_NAMES:
				case SHOW_CHANNEL_NUMBERS:
				{
					electrode_drawn=1;
					if (SHOW_ELECTRODE_NAMES==map->electrodes_option)
					{
						name=device->description->name;
					}
					else
					{
						sprintf(value_string,"%d",device->channel->number);
						name=value_string;
					}
					if (device->highlight)
					{
						graphics_context=(drawing_information->graphics_context).
							highlighted_colour;
					}
					else
					{
						graphics_context=(drawing_information->graphics_context).
							unhighlighted_colour;
					}
				} break;
				case SHOW_ELECTRODE_VALUES:
				{
					f_value=(map->electrode_value)[electrode_number];
					electrode_drawn=(map->electrode_drawn)[electrode_number];
					if (electrode_drawn)
					{
						if (HIDE_COLOUR==map->colour_option)
						{
							sprintf(value_string,"%.4g",f_value);
							name=value_string;
						}
						else
						{
							name=(char *)NULL;
						}
						if (device->highlight)
						{
							graphics_context=(drawing_information->graphics_context).
								highlighted_colour;
						}
						else
						{
							if ((HIDE_COLOUR==map->colour_option)&&
								(SHOW_CONTOURS==map->contours_option))
							{
								graphics_context=(drawing_information->graphics_context).
									unhighlighted_colour;
							}
							else
							{
								min_f=map->minimum_value;
								max_f=map->maximum_value;
								graphics_context=(drawing_information->graphics_context).
									spectrum;
								if (f_value<=min_f)
								{
									XSetForeground(display,graphics_context,
										spectrum_pixels[0]);
								}
								else
								{
									if (f_value>=max_f)
									{
										XSetForeground(display,graphics_context,
											spectrum_pixels[number_of_spectrum_colours-1]);
									}
									else
									{
										if ((range_f=max_f-min_f)<=0)
										{
											range_f=1;
										}
										XSetForeground(display,graphics_context,
											spectrum_pixels[(int)((f_value-min_f)*
											(float)(number_of_spectrum_colours-1)/range_f)]);
									}
								}
							}
						}
					}
				} break;
				default:
				{
					name=(char *)NULL;
				} break;
			}
			if (electrode_drawn)
			{
				/* draw marker */
				xpos=(map->electrode_x)[electrode_number];
				ypos=(map->electrode_y)[electrode_number];
				marker_size=map->electrodes_marker_size;
				xmin=xpos-marker_size;
				xmax=xpos+marker_size;
				ymin=ypos-marker_size;
				ymax=ypos+marker_size;
				switch (map->electrodes_marker_type)
				{
					case CIRCLE_ELECTRODE_MARKER:
					{
						/* draw circle */
						XFillArc(display,mapping->map_drawing->pixel_map,graphics_context,
							xmin,ymin,2*marker_size+1,2*marker_size+1,(int)0,(int)(360*64));
					} break;
					case PLUS_ELECTRODE_MARKER:
					{
						/* draw plus */
						XDrawLine(display,mapping->map_drawing->pixel_map,graphics_context,
							xmin,ypos,xmax,ypos);
						XDrawLine(display,mapping->map_drawing->pixel_map,graphics_context,
							xpos,ymin,xpos,ymax);
					} break;
					case SQUARE_ELECTRODE_MARKER:
					{
						/* draw square */
						XFillRectangle(display,mapping->map_drawing->pixel_map,
							graphics_context,xmin,ymin,2*marker_size+1,2*marker_size+1);
					} break;
				}
				if (name)
				{
					/* write name */
					name_length=strlen(name);
					XTextExtents(font,name,name_length,&direction,&ascent,&descent,
						&bounds);
					xstart=xpos+(bounds.lbearing-bounds.rbearing+1)/2;
					ystart=ypos-descent-1;
					if (xstart-bounds.lbearing<0)
					{
						xstart=bounds.lbearing;
					}
					else
					{
						if (xstart+bounds.rbearing>mapping->map_drawing->width)
						{
							xstart=mapping->map_drawing->width-bounds.rbearing;
						}
					}
					if (ystart-ascent<0)
					{
						ystart=ascent;
					}
					else
					{
						if (ystart+descent>mapping->map_drawing->height)
						{
							ystart=mapping->map_drawing->height-descent;
						}
					}
					if (device->highlight)
					{
						XDrawString(display,mapping->map_drawing->pixel_map,
							(drawing_information->graphics_context).highlighted_colour,xstart,
							ystart,name,name_length);
					}
					else
					{
						XDrawString(display,mapping->map_drawing->pixel_map,
							(drawing_information->graphics_context).node_text_colour,xstart,
							ystart,name,name_length);
					}
					if (xstart+bounds.lbearing<xmin)
					{
						xmin=xstart+bounds.lbearing;
					}
					if (xstart+bounds.rbearing>xmax)
					{
						xmax=xstart+bounds.rbearing;
					}
					if (ystart-ascent<ymin)
					{
						ymin=ystart-ascent;
					}
					if (ystart+descent>ymax)
					{
						ymax=ystart+descent;
					}
				}
				XCopyArea(display,mapping->map_drawing->pixel_map,
					XtWindow(mapping->map_drawing_area_2d),
					(drawing_information->graphics_context).copy,xmin,ymin,xmax-xmin+1,
					ymax-ymin+1,xmin,ymin);
				return_code=1;
			}
		}
		if ((NO_MAP_FIELD==map_type)&&(auxiliary_number>=0)&&
			(mapping->colour_or_auxiliary_drawing_area)&&
			(mapping->colour_or_auxiliary_drawing))
		{
			xpos=(map->auxiliary_x)[auxiliary_number];
			ypos=(map->auxiliary_y)[auxiliary_number];
			xmin=xpos-2;
			xmax=xpos+2;
			ymin=ypos-2;
			ymax=ypos+2;
			if (device->highlight)
			{
				XFillRectangle(display,
					mapping->colour_or_auxiliary_drawing->pixel_map,
					(drawing_information->graphics_context).highlighted_colour,xmin,ymin,
					5,5);
			}
			else
			{
				XFillRectangle(display,
					mapping->colour_or_auxiliary_drawing->pixel_map,
					(drawing_information->graphics_context).unhighlighted_colour,xmin,
					ymin,5,5);
			}
			if (device->description->name)
			{
				name_length=strlen(device->description->name);
				XTextExtents(font,device->description->name,name_length,
					&direction,&ascent,&descent,&bounds);
				xstart=xpos+6-bounds.lbearing;
				ystart=ypos+ascent-4;
				if (device->highlight)
				{
					XDrawString(display,mapping->colour_or_auxiliary_drawing->pixel_map,
						(drawing_information->graphics_context).highlighted_colour,xstart,
						ystart,device->description->name,name_length);
				}
				else
				{
					XDrawString(display,mapping->colour_or_auxiliary_drawing->pixel_map,
						(drawing_information->graphics_context).node_text_colour,xstart,
						ystart,device->description->name,name_length);
				}
				if (xstart+bounds.lbearing<xmin)
				{
					xmin=xstart+bounds.lbearing;
				}
				if (xstart+bounds.rbearing>xmax)
				{
					xmax=xstart+bounds.rbearing;
				}
				if (ystart-ascent<ymin)
				{
					ymin=ystart-ascent;
				}
				if (ystart+descent>ymax)
				{
					ymax=ystart+descent;
				}
			}
			XCopyArea(display,mapping->colour_or_auxiliary_drawing->pixel_map,
				XtWindow(mapping->colour_or_auxiliary_drawing_area),
				(drawing_information->graphics_context).copy,xmin,ymin,xmax-xmin+1,
				ymax-ymin+1,xmin,ymin);
			return_code=1;
		}
	}
	LEAVE;

	return (return_code);
} /* highlight_electrode_or_auxiliar */

int Mapping_window_set_potential_time_object(struct Mapping_window *mapping,
	struct Time_object *potential_time_object)
/*******************************************************************************
LAST MODIFIED : 15 October 1998
DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Mapping_window_set_potential_time_object);
	if (mapping && potential_time_object)
	{
		if (mapping->potential_time_object != potential_time_object)
		{
			if (mapping->potential_time_object)
			{
				DEACCESS(Time_object)(&(mapping->potential_time_object));
			}
			mapping->potential_time_object=ACCESS(Time_object)(potential_time_object);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mapping_window_set_potential_time_object.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mapping_window_set_potential_time_object */
