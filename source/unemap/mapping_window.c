/*******************************************************************************
FILE : mapping_window.c

LAST MODIFIED : 7 May 2004

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
#if defined (UNEMAP_USE_3D)
#include "interaction/interactive_toolbar_widget.h"
#endif /* defined (UNEMAP_USE_3D) */
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
#if defined (UNEMAP_USE_3D)
#include "unemap/rig_node.h"
#include "unemap/unemap_package.h"
#endif /* defined (UNEMAP_USE_3D) */
#include "user_interface/confirmation.h"
#include "unemap/setup_dialog.h"
#include "user_interface/event_dispatcher.h"
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

static int update_movie_frames_information(struct Mapping_window *mapping,
	int *recalculate,int *map_settings_changed)
/*******************************************************************************
LAST MODIFIED : 21 April 2004

DESCRIPTION :
For for map in <mapping>, dealing with precalculated movies, update the number
of frames, map dialog information, etc. and set <recalculate>
<map_settings_changed>
==============================================================================*/
{
	char *value_string;
	float buffer_end_time,buffer_start_time,frame_end_time,frame_start_time;
	int frame_number,number_of_frames,return_code;
	struct Map *map;
	struct Map_dialog *map_dialog;
	struct Signal_buffer *buffer;

	ENTER(update_movie_frames_information);
	map=(struct Map *)NULL;
	map_dialog=(struct Map_dialog *)NULL;
	buffer=(struct Signal_buffer *)NULL;
	if (mapping&&(map_dialog=mapping->map_dialog)&&(map=mapping->map))
	{
		return_code=1;
		XtVaGetValues((map_dialog->animation).number_of_frames_text,
			XmNvalue,&value_string,NULL);
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
			XmNvalue,&value_string,NULL);
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
				XmNvalue,&value_string,NULL);
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
					*map_settings_changed=1;
				}
			}
			else
			{
				*map_settings_changed=1;
				if (*recalculate<2)
				{
					*recalculate=2;
				}
			}
		}
		else
		{
			*map_settings_changed=1;
			if (*recalculate<3)
			{
				*recalculate=3;
			}
		}
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
			XtSetSensitive(mapping->print_menu.animate_jpg_button,True);
			XtSetSensitive(mapping->print_menu.animate_bmp_button,True);
		}
		else
		{
			XtSetSensitive(mapping->animate_button,False);
			XtSetSensitive(mapping->print_menu.animate_rgb_button,False);
			XtSetSensitive(mapping->print_menu.animate_tiff_button,False);
			XtSetSensitive(mapping->print_menu.animate_jpg_button,False);
			XtSetSensitive(mapping->print_menu.animate_bmp_button,False);
		}
		map->sub_map_number=frame_number-1;
		if (frame_start_time!=map_dialog->start_time)
		{
			map->start_time=frame_start_time;
			map_dialog->start_time=frame_start_time;
			*map_settings_changed=2;
		}
		if (frame_end_time!=map_dialog->end_time)
		{
			map->end_time=frame_end_time;
			map_dialog->end_time=frame_end_time;
		}
		map_dialog->number_of_frames=number_of_frames;
		map_dialog->frame_number=frame_number;
	}
	else
	{
		display_message(ERROR_MESSAGE,"update_movie_frames_information.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* update_movie_frames_information */

int mapping_window_stop_time_keeper(struct Mapping_window *mapping)
/*******************************************************************************
LAST MODIFIED : 14 December 2001

DESCRIPTION :
If the <mapping> has a time keeper, stop it .
==============================================================================*/
{
	int return_code;
	struct Time_keeper *time_keeper;

	ENTER(mapping_window_stop_time_keeper);
	time_keeper=(struct Time_keeper *)NULL;
	return_code=0;
	if (mapping)
	{
		return_code=1;

		/*stop the time keeper and get rid of the editor widget*/
		if ((mapping->potential_time_object)&&
			(time_keeper=Time_object_get_time_keeper(mapping->potential_time_object)))
		{
			Time_keeper_stop(time_keeper);
		}
	}
	LEAVE;
	
	return (return_code);
}/* mapping_window_stop_time_keeper */

int mapping_window_kill_time_keeper_editor(struct Mapping_window *mapping)
/*******************************************************************************
LAST MODIFIED : 17 May 2002

DESCRIPTION :
If the <mapping> has a time keeper, stop it and destroy it's editor widget.
==============================================================================*/
{
	int return_code;

	ENTER(mapping_window_kill_time_keeper_editor);
	if (mapping)
	{
		return_code=1;
		if (mapping->time_editor_dialog)
		{
			/*stop the time keeper and get rid of the editor widget*/
			mapping_window_stop_time_keeper(mapping);
			DESTROY(Time_editor_dialog)(&(mapping->time_editor_dialog));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"mapping_window_kill_time_keeper_editor. Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* mapping_window_kill_time_keeper_editor */

static void redraw_map_from_dialog(Widget widget,XtPointer mapping_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 April 2004

DESCRIPTION :
Updates the map settings based on the map dialog and redraws the map if
necessary.
==============================================================================*/
{
	char *value_string,temp_string[20];
	enum Contour_thickness contour_thickness;
	enum Electrodes_marker_type electrodes_marker_type;
	enum Electrodes_label_type electrodes_label_type;
	enum Fibres_option fibres_option;
	enum Interpolation_type interpolation_type;
	enum Spectrum_simple_type spectrum_simple_type;
	float contour_maximum,contour_minimum,contour_step,maximum_range,
		minimum_range,value;
	int electrodes_marker_size,map_settings_changed,number_of_contours,
		number_of_mesh_columns,number_of_mesh_rows,recalculate;
	struct Map *map;
	struct Map_dialog *map_dialog;
	struct Map_drawing_information *drawing_information;
	struct Mapping_window *mapping;
	Widget option_widget;
	struct Spectrum *spectrum;
	struct Spectrum *spectrum_to_be_modified_copy;
#if defined (UNEMAP_USE_3D)
	struct MANAGER(Spectrum) *spectrum_manager;
#endif /* defined (UNEMAP_USE_3D) */
	struct Time_keeper *time_keeper;

	ENTER(redraw_map_from_dialog);
	time_keeper=(struct Time_keeper *)NULL;
	spectrum=(struct Spectrum *)NULL;
	spectrum_to_be_modified_copy=(struct Spectrum *)NULL;
	USE_PARAMETER(call_data);
	if ((mapping=(struct Mapping_window *)mapping_window)&&(map=mapping->map)&&
		(map_dialog=mapping->map_dialog)&&
		(drawing_information=map->drawing_information))
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
			if (map->fixed_range)
			{
				map->fixed_range=0;
				map_settings_changed=1;
				if (recalculate<2)
				{
					recalculate=2;
				}
			}
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
			if (option_widget==map_dialog->spectrum.type_option.blue_red)
			{
				spectrum_simple_type=BLUE_TO_RED_SPECTRUM;
			}
			else if (option_widget==map_dialog->spectrum.type_option.blue_white_red)
			{
				spectrum_simple_type=BLUE_WHITE_RED_SPECTRUM;
			}
			else if (option_widget==map_dialog->spectrum.type_option.log_blue_red)
			{
				spectrum_simple_type=LOG_BLUE_TO_RED_SPECTRUM;
			}
			else if (option_widget==map_dialog->spectrum.type_option.log_red_blue)
			{
				spectrum_simple_type=LOG_RED_TO_BLUE_SPECTRUM;
			}
			else
			{
				spectrum_simple_type=RED_TO_BLUE_SPECTRUM;
			}
			if (spectrum_simple_type!=Spectrum_get_simple_type(spectrum))
			{
#if defined (UNEMAP_USE_3D)
				if (spectrum_manager=get_map_drawing_information_spectrum_manager
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
#endif /* defined (UNEMAP_USE_3D) */
							Spectrum_set_simple_type(spectrum_to_be_modified_copy,
								spectrum_simple_type);
							map_settings_changed=1;
							recalculate=2;
#if defined (UNEMAP_USE_3D)
							MANAGER_MODIFY_NOT_IDENTIFIER(Spectrum,name)(spectrum,
								spectrum_to_be_modified_copy,spectrum_manager);
							DESTROY(Spectrum)(&spectrum_to_be_modified_copy);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								" redraw_map_from_dialog. Could not create spectrum copy.");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							" redraw_map_from_dialog. Spectrum is not in manager!");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						" redraw_map_from_dialog. Spectrum_manager not present");
				}
#endif /* defined (UNEMAP_USE_3D) */
			}
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
			else if (option_widget==map_dialog->interpolation.option.direct)
			{
				interpolation_type=DIRECT_INTERPOLATION;
			}
		}
		if (map->interpolation_type!=interpolation_type)
		{
			/* time keeper editor may now be invalid, eg when moving from direct to
				bicubic maps, so get rid of it ??JW may wish to decied to leave it for
				some transitions of interpolation_type and map->type */
			mapping_window_kill_time_keeper_editor(mapping);
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
			sprintf(temp_string,"%d",number_of_mesh_rows);
			XtVaSetValues((map_dialog->interpolation.mesh_rows_text),
				XmNvalue,temp_string,
				NULL);
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
			sprintf(temp_string,"%d",number_of_mesh_columns);
			XtVaSetValues((map_dialog->interpolation.mesh_columns_text),
				XmNvalue,temp_string,
				NULL);
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
				if (recalculate<1)
				{
					recalculate=1;
				}
			}
		}
		else
		{
			if (map->contours_option!=SHOW_CONTOURS)
			{
				map_settings_changed=1;
				map->contours_option=SHOW_CONTOURS;
				if (recalculate<2)
				{
					recalculate=2;
				}
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
			if (contour_thickness!=map->contour_thickness)
			{
				map->contour_thickness=contour_thickness;
				map_settings_changed=1;
				if (recalculate<2)
				{
					recalculate=2;
				}
			}
			value_string=(char *)NULL;
			XtVaGetValues((map_dialog->contours.minimum_text),
				XmNvalue,&value_string,
				NULL);
			if (1!=sscanf(value_string,"%f",&contour_minimum))
			{
				contour_minimum=map->contour_minimum;
				sprintf(temp_string,"%g",contour_minimum);
				XtVaSetValues((map_dialog->contours.minimum_text),
					XmNvalue,temp_string,
					NULL);
			}
			XtFree(value_string);
			value_string=(char *)NULL;
			XtVaGetValues((map_dialog->contours.step_text),
				XmNvalue,&value_string,
				NULL);
			if (1!=sscanf(value_string,"%f",&contour_step))
			{
				contour_step=0;
				sprintf(temp_string,"%g",contour_step);
				XtVaSetValues((map_dialog->contours.step_text),
					XmNvalue,temp_string,
					NULL);
			}
			XtFree(value_string);
			value_string=(char *)NULL;
			XtVaGetValues((map_dialog->contours.number_text),
				XmNvalue,&value_string,
				NULL);
			if (1!=sscanf(value_string,"%d",&number_of_contours))
			{
				number_of_contours=map->number_of_contours;
				sprintf(temp_string,"%d",number_of_contours);
				XtVaSetValues((map_dialog->contours.number_text),
					XmNvalue,temp_string,
					NULL);
			}
			if (contour_step<=0)
			{
				contour_step=0;
				sprintf(temp_string,"%g",contour_step);
				XtVaSetValues((map_dialog->contours.step_text),
					XmNvalue,temp_string,
					NULL);
				number_of_contours=1;
				sprintf(temp_string,"%d",number_of_contours);
				XtVaSetValues((map_dialog->contours.number_text),
					XmNvalue,temp_string,
					NULL);
			}
			else
			{
				if (number_of_contours<1)
				{
					number_of_contours=1;
					sprintf(temp_string,"%d",number_of_contours);
					XtVaSetValues((map_dialog->contours.number_text),
						XmNvalue,temp_string,
						NULL);
				}
			}
			XtFree(value_string);
			contour_maximum=
				contour_minimum+contour_step*(float)(number_of_contours-1);
			if (!((number_of_contours==map->number_of_contours)&&
				(contour_minimum==map->contour_minimum)&&
				(contour_maximum==map->contour_maximum)))
			{
				map_settings_changed=1;
				map->number_of_contours=number_of_contours;
				map->contour_minimum=contour_minimum;
				map->contour_maximum=contour_maximum;
				if (recalculate<2)
				{
					recalculate=2;
				}
			}
		}
		value_string=(char *)NULL;
		XtVaGetValues((map_dialog->range.maximum_value),
			XmNvalue,&value_string,
			NULL);
		if (1!=sscanf(value_string,"%f",&maximum_range))
		{
			maximum_range=map_dialog->range_maximum;
			sprintf(temp_string,"%g",maximum_range);
			XtVaSetValues((map_dialog->range.maximum_value),
				XmNvalue,temp_string,
				NULL);
		}
		XtFree(value_string);
		value_string=(char *)NULL;
		XtVaGetValues((map_dialog->range.minimum_value),
			XmNvalue,&value_string,
			NULL);
		if (1!=sscanf(value_string,"%f",&minimum_range))
		{
			minimum_range=map_dialog->range_minimum;
			sprintf(temp_string,"%g",minimum_range);
			XtVaSetValues((map_dialog->range.minimum_value),
				XmNvalue,temp_string,
				NULL);
		}
		XtFree(value_string);
		if (minimum_range>maximum_range)
		{
			value=minimum_range;
			minimum_range=maximum_range;
			maximum_range=value;
			sprintf(temp_string,"%g",minimum_range);
			XtVaSetValues((map_dialog->range.minimum_value),
				XmNvalue,temp_string,
				NULL);
			sprintf(temp_string,"%g",maximum_range);
			XtVaSetValues((map_dialog->range.maximum_value),
				XmNvalue,temp_string,
				NULL);
		}
		if (minimum_range!=map_dialog->range_minimum)
		{
			map->range_changed=1;
			map->minimum_value=minimum_range;
			map_dialog->range_minimum=minimum_range;
			map_settings_changed=1;
			if (recalculate<2)
			{
				recalculate=2;
			}
		}
		if (maximum_range!=map_dialog->range_maximum)
		{
			map->range_changed=1;
			map->maximum_value=maximum_range;
			map_dialog->range_maximum=maximum_range;
			map_settings_changed=1;
			if (recalculate<2)
			{
				recalculate=2;
			}
		}
		XtVaGetValues(map_dialog->electrodes.label_menu,
			XmNmenuHistory,&option_widget,
			NULL);
		if (option_widget==map_dialog->electrodes.label.name)
		{
			electrodes_label_type=SHOW_ELECTRODE_NAMES;
		}
		else
		{
			if (option_widget==map_dialog->electrodes.label.value)
			{
				electrodes_label_type=SHOW_ELECTRODE_VALUES;
			}
			else
			{
				if (option_widget==map_dialog->electrodes.label.channel)
				{
					electrodes_label_type=SHOW_CHANNEL_NUMBERS;
				}
				else
				{
					if (option_widget==map_dialog->electrodes.label.hide)
					{
						electrodes_label_type=HIDE_ELECTRODE_LABELS;
					}
				}
			}
		}
		if (map->electrodes_label_type!=electrodes_label_type)
		{
			map->electrodes_label_type=electrodes_label_type;
			map_settings_changed=1;
		}
		if (XmToggleButtonGadgetGetState(
			map_dialog->electrodes.marker_colour_toggle))
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
		}
		value_string=(char *)NULL;
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
			sprintf(temp_string,"%d",electrodes_marker_size);
			XtVaSetValues((map_dialog->electrodes.marker_size_text),
				XmNvalue,temp_string,
				NULL);
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
		if (XmToggleButtonGadgetGetState(
			map_dialog->regions_use_same_coordinates_toggle))
		{
			if (!(map->regions_use_same_coordinates))
			{
				map_settings_changed=1;
				if (recalculate<2)
				{
					recalculate=2;
				}
				map->regions_use_same_coordinates=1;
			}
		}
		else
		{
			if (map->regions_use_same_coordinates)
			{
				map_settings_changed=1;
				if (recalculate<2)
				{
					recalculate=2;
				}
				map->regions_use_same_coordinates=0;
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
		if (POTENTIAL== *(map->type))
		{
			switch (map->interpolation_type)
			{
				case BICUBIC_INTERPOLATION:
				{
					update_movie_frames_information(mapping,&recalculate,
						&map_settings_changed);
				} break; /*case BICUBIC_INTERPOLATION:*/
				case NO_INTERPOLATION:
				case DIRECT_INTERPOLATION:
				default:
				{
					if (ELECTRICAL_IMAGING==(*map->analysis_mode))
					{
						XtSetSensitive(mapping->animate_button,False);
						XtSetSensitive(mapping->print_menu.animate_rgb_button,False);
						XtSetSensitive(mapping->print_menu.animate_tiff_button,False);
						XtSetSensitive(mapping->print_menu.animate_jpg_button,False);
						XtSetSensitive(mapping->print_menu.animate_bmp_button,False);
					}
					else
					{
						if (map->projection_type==THREED_PROJECTION)
						{
							/*not sure what we're going to do with 3D movies yet*/
							/*for now make them behave like bicubic 2D movies */
							update_movie_frames_information(mapping,&recalculate,
								&map_settings_changed);
						}
						else
						{
							XtSetSensitive(mapping->animate_button,True);
							XtSetSensitive(mapping->print_menu.animate_rgb_button,True);
							XtSetSensitive(mapping->print_menu.animate_tiff_button,True);
							XtSetSensitive(mapping->print_menu.animate_jpg_button,True);
							XtSetSensitive(mapping->print_menu.animate_bmp_button,True);
						}
					}
				} break;
			}/*switch (map->interpolation_type) */
		}	/* if (POTENTIAL== *(map->type)) */
		if (MULTIPLE_ACTIVATION== *(map->type))
		{
			if (NO_INTERPOLATION!=map->interpolation_type)
			{
				XtSetSensitive(mapping->animate_button,False);
				XtSetSensitive(mapping->print_menu.animate_rgb_button,False);
				XtSetSensitive(mapping->print_menu.animate_tiff_button,False);
				XtSetSensitive(mapping->print_menu.animate_jpg_button,False);
				XtSetSensitive(mapping->print_menu.animate_bmp_button,False);
			}
			else
			{
				XtSetSensitive(mapping->animate_button,True);
				XtSetSensitive(mapping->print_menu.animate_rgb_button,True);
				XtSetSensitive(mapping->print_menu.animate_tiff_button,True);
				XtSetSensitive(mapping->print_menu.animate_jpg_button,True);
				XtSetSensitive(mapping->print_menu.animate_bmp_button,True);
			}
		}
		if (map_settings_changed)
		{
			/* there's more than one map, so we'll need to recalculate them */
			if (*map->first_eimaging_event&&(ELECTRICAL_IMAGING==*map->analysis_mode))
			{
				recalculate=2;
			}
#if defined (UNEMAP_USE_3D)
			set_map_drawing_information_electrodes_accepted_or_rejected(
				map->drawing_information,1);
#endif
			ensure_map_projection_type_matches_region_type(map);
			mapping_window_update_time_limits(mapping);
			if (2==map_settings_changed)
			{
				/* the time has changed so update the time_keeper */
				time_keeper=Time_object_get_time_keeper(mapping->potential_time_object);
				Time_keeper_request_new_time(time_keeper,map->start_time/1000.0);
			}
			else
			{
				update_mapping_drawing_area(mapping,recalculate);
				update_mapping_colour_or_auxili(mapping);
			}
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
			"redraw_map_from_dialog.  Invalid or missing mapping_window");
	}
	LEAVE;
} /* redraw_map_from_dialog */

static void update_dialog_elec_options(Widget widget,XtPointer mapping_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 31 October 2001

DESCRIPTION :
Dims the map dialog electrode label menu if we've hidden the electrode markers,
as if not showinf markers, not showing labels either.
==============================================================================*/
{
	struct Map_dialog *map_dialog;
	struct Mapping_window *mapping;
	Widget option_widget;
	ENTER(update_dialog_elec_options);
	map_dialog=(struct Map_dialog *)NULL;
	mapping=(struct Mapping_window *)NULL;
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if ((mapping=(struct Mapping_window *)mapping_window)&&
		(map_dialog=mapping->map_dialog))
	{
		XtVaGetValues(map_dialog->electrodes.marker_type_menu,XmNmenuHistory,
			&option_widget,NULL);
		if (option_widget==map_dialog->electrodes.marker_type.none)
		{
			/* If hiding the marker, hiding the label too, so dim the label select*/
			XtSetSensitive(map_dialog->electrodes.label_menu,False);
		}
		else
		{
			XtSetSensitive(map_dialog->electrodes.label_menu,True);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"update_dialog_elec_options. Invalid or missing mapping_window");
	}
	LEAVE;
} /* update_dialog_elec_options */

static int configure_map_dialog(struct Mapping_window *mapping_window)
/*******************************************************************************
LAST MODIFIED : 27 November 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static MrmRegisterArg identifier_list[]=
	{
		{"mapping_window_structure",(XtPointer)NULL}
	};

	ENTER(configure_map_dialog);
	return_code=0;
	if (mapping_window&&(mapping_window->map))
	{
		if (mapping_window->map_dialog)
		{
			return_code=1;
		}
		else
		{
			/* assign and register the identifiers */
				/*???DB.  Have to put in global name list because the map dialog
					hierarchy may not be open */
			identifier_list[0].value=(XtPointer)mapping_window;
			if (MrmSUCCESS==MrmRegisterNames(identifier_list,
				XtNumber(identifier_list)))
			{
				if (create_Map_dialog(&(mapping_window->map_dialog),
					&(mapping_window->map),mapping_window->map_button,
					mapping_window->user_interface))
				{
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"configure_map_dialog.  Could not register identifiers");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"configure_map_dialog.  Missing mapping_window");
	}
	LEAVE;

	return (return_code);
} /* configure_map_dialog */

static void configure_map_dialog_and_open(Widget widget,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 27 November 2003

DESCRIPTION :
Opens the dialog box associated with the map button in the mapping window.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(configure_map_dialog_and_open);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	mapping=(struct Mapping_window *)mapping_window;
	if (configure_map_dialog(mapping))
	{
		open_map_dialog(mapping->map_dialog);
	}
	LEAVE;
} /* configure_map_dialog_and_open */

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

static void map3d_id_interactive_tool_form(Widget *widget_id,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 September 2000

DESCRIPTION :
Finds the id of the interactive_tool_form
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(map3d_id_interactive_tool_form);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
#if defined (UNEMAP_USE_3D)
		mapping->map3d_interactive_tool_form= *widget_id;
#else
		USE_PARAMETER(widget_id);
		USE_PARAMETER(mapping);
#endif /* defined (UNEMAP_USE_3D) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"map3d_id_interactive_tool_form.  Missing mapping_window");
	}
	LEAVE;
} /* map3d_id_interactive_tool_form */

static void map3d_id_viewing_form(Widget *widget_id,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 September 2000

DESCRIPTION :
Finds the id of the viewing
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(map3d_id_viewing_form);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)mapping_window)
	{
#if defined (UNEMAP_USE_3D)
		mapping->map3d_viewing_form= *widget_id;
#else
		USE_PARAMETER(widget_id);
		USE_PARAMETER(mapping);
#endif /* defined (UNEMAP_USE_3D) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"map3d_id_viewing_form.  Missing mapping_window");
	}
	LEAVE;
} /* map3d_id_viewing_form */

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

static int draw_activation_animation_frame(void *mapping_window)
/*******************************************************************************
LAST MODIFIED : 24 April 2004

DESCRIPTION :
Draws a frame in the activation map animation.
==============================================================================*/
{
	Colormap colour_map;
	Display *display;
	float contour_maximum,contour_minimum,maximum_value,minimum_value;
	int cell_number,i,number_of_contours,number_of_spectrum_colours,return_code;
	Pixel *spectrum_pixels;
	struct Drawing_2d *drawing;
	struct Map *map;
	struct Map_drawing_information *drawing_information;
	struct Mapping_window *mapping;
	XColor colour,spectrum_rgb[MAX_SPECTRUM_COLOURS];

	ENTER(draw_activation_animation_frame);
	if ((mapping=(struct Mapping_window *)mapping_window)&&(map=mapping->map)&&
		(map->type)&&((SINGLE_ACTIVATION== *(map->type))||
		((MULTIPLE_ACTIVATION== *(map->type))&&
		(NO_INTERPOLATION==map->interpolation_type))||
		(POTENTIAL== *(map->type)))&&
		(drawing_information=map->drawing_information)&&(mapping->user_interface)&&
		(mapping->user_interface==drawing_information->user_interface)&&
		(drawing=mapping->map_drawing))
	{
		return_code = 1;
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
					display=User_interface_get_display(
						drawing_information->user_interface);
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
							if (1<number_of_contours)
							{
								cell_number=(int)(((contour_maximum*(float)i+contour_minimum*
									(float)(number_of_contours-1-i))/
									(float)(number_of_contours-1)-minimum_value)/
									(maximum_value-minimum_value)*
									(float)(number_of_spectrum_colours-1)+0.5);
							}
							else
							{
								cell_number=(int)((contour_minimum-
									minimum_value)/(maximum_value-minimum_value)*
									(float)(number_of_spectrum_colours-1)+0.5);
							}
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
					if (drawing_information->boundary_colour)
					{
						colour.pixel=drawing_information->boundary_colour;
						colour.flags=DoRed|DoGreen|DoBlue;
						XStoreColor(display,colour_map,&colour);
					}
				}
				(map->activation_front)++;
				if (map->activation_front<number_of_spectrum_colours)
				{
					Event_dispatcher_add_timeout_callback(
						User_interface_get_event_dispatcher(mapping->user_interface),
						/*seconds*/0,/*nanoseconds*/100000000,
						draw_activation_animation_frame,(void *)mapping_window);
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
					Event_dispatcher_add_timeout_callback(
						User_interface_get_event_dispatcher(mapping->user_interface),
						/*seconds*/0,/*nanoseconds*/10000000,
						draw_activation_animation_frame,(void *)mapping_window);
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
					switch (map->interpolation_type)
					{
						case BICUBIC_INTERPOLATION:
						{
							update_mapping_drawing_area(mapping,0);
							(map->sub_map_number)++;
							if (map->sub_map_number<map->number_of_sub_maps)
							{
								Event_dispatcher_add_timeout_callback(
									User_interface_get_event_dispatcher(mapping->user_interface),
									/*seconds*/0,/*nanoseconds*/100000000,
									draw_activation_animation_frame,(void *)mapping_window);
							}
							else
							{
								map->sub_map_number=map->activation_front;
								map->activation_front= -1;
								XtSetSensitive(mapping->animate_button,True);
								update_mapping_drawing_area(mapping,0);
							}
						} break;
						case NO_INTERPOLATION:
						case DIRECT_INTERPOLATION:
						default:
						{
							update_mapping_drawing_area(mapping,2);
							update_mapping_colour_or_auxili(mapping);
							/*???DB.  What about the trace window ? */
							(*(map->potential_time))++;
							if (*(map->potential_time)< *(map->end_search_interval))
							{
								Event_dispatcher_add_timeout_callback(
									User_interface_get_event_dispatcher(mapping->user_interface),
									/*seconds*/0,/*nanoseconds*/10000000,
									draw_activation_animation_frame,(void *)mapping_window);
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
						} break;
					}/*switch (map->interpolation_type) */
				}/* if (POTENTIAL== *(map->type)) */
			}
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* draw_activation_animation_frame */

static void animate_activation_map(Widget widget,XtPointer mapping_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 February 2002

DESCRIPTION :
Starts the activation map animation.
==============================================================================*/
{
	struct Device *device;
	struct Mapping_window *mapping;
	struct Map *map;
	struct Rig *rig;
	struct Signal *signal;
	struct Signal_buffer *buffer;
	struct Time_keeper *time_keeper;

	ENTER(animate_activation_map);
	mapping=(struct Mapping_window *)NULL;
	map=(struct Map *)NULL;
	time_keeper=(struct Time_keeper *)NULL;
	buffer=(struct Signal_buffer *)NULL;
	signal=(struct Signal *)NULL;
	rig=(struct Rig *)NULL;
	device=(struct Device *)NULL;
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((mapping=(struct Mapping_window *)mapping_window)&&(map=mapping->map)&&
		(rig=*map->rig_pointer)&&(device=*(rig->devices))&&
		(signal=device->signal)&&(buffer=signal->buffer)&&
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
						map->activation_front=0;
						switch (map->interpolation_type)
						{
							case BICUBIC_INTERPOLATION:
							{
								/* Jump to the start of the animated sequence */
								Time_keeper_request_new_time(time_keeper,
									map->start_time/1000.0);
							} break;
							case NO_INTERPOLATION:
							case DIRECT_INTERPOLATION:
							default:
							{
								Time_keeper_request_new_time(time_keeper,
									map->start_time/1000.0);
							} break;
						}/* map->interpolation_type */
					} break;
					case SINGLE_ACTIVATION:
					{
						map->activation_front=0;
						Time_keeper_request_new_time(time_keeper,
							(map->minimum_value)/1000.0+
							(float)(buffer->times[*(map->datum)])/(buffer->frequency));
					} break;
					case MULTIPLE_ACTIVATION:
					{
						map->activation_front= *(map->datum);
						Time_keeper_request_new_time(time_keeper,
							(float)buffer->times[*(map->start_search_interval)]/
							buffer->frequency);
					} break;
				}
				bring_up_time_editor_dialog(&mapping->time_editor_dialog,
					User_interface_get_application_shell(mapping->user_interface),
					time_keeper,mapping->user_interface);
				Time_keeper_play(time_keeper,TIME_KEEPER_PLAY_FORWARD);
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
							map->activation_front=map->sub_map_number;
							map->sub_map_number=0;
						}
					}
				}
			}
			/* only one animation at a time */
			XtSetSensitive(mapping->animate_button,False);
			draw_activation_animation_frame(mapping_window);
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
LAST MODIFIED : 4 October 2002

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
#if defined (UNEMAP_USE_3D)
				,mapping->map->unemap_package
#endif /* defined (UNEMAP_USE_3D) */
					)))
			{
				/* destroy the present rig */
				destroy_Rig(map_rig_pointer);
				/* assign the new rig */
				*(map_rig_pointer)=rig;
				/* update the mapping window */
				update_mapping_drawing_area(mapping,2);
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
#if defined (UNEMAP_USE_3D)
		XtUnmanageChild(mapping_window->mapping_area_3d);
#endif /* defined (UNEMAP_USE_3D) */
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

#if defined (UNEMAP_USE_3D)
static int Mapping_window_set_interactive_tool(
	struct Mapping_window *mapping_window,
	struct Interactive_tool *interactive_tool)
/*******************************************************************************
LAST MODIFIED : 4 September 2000

DESCRIPTION :
Sets the <interactive_tool> in use in the <mapping_window>. Updates the
toolbar to match the selection.
==============================================================================*/
{
	int return_code;

	ENTER(Mapping_window_set_interactive_tool);
	if (mapping_window)
	{
		if (interactive_toolbar_widget_set_current_interactive_tool(
			mapping_window->interactive_toolbar_widget,interactive_tool))
		{
			mapping_window->interactive_tool=interactive_tool;
			if (interactive_tool==mapping_window->transform_tool)
			{
				Scene_viewer_set_input_mode(mapping_window->scene_viewer,
					SCENE_VIEWER_TRANSFORM);
				interactive_tool=mapping_window->transform_tool;
			}
			else
			{
				Scene_viewer_set_input_mode(mapping_window->scene_viewer,
					SCENE_VIEWER_SELECT);
			}
			Scene_viewer_set_interactive_tool(mapping_window->scene_viewer,
				interactive_tool);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Mapping_window_set_interactive_tool.  Could not update toolbar");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mapping_window_set_interactive_tool.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mapping_window_set_interactive_tool */
#endif /*  defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static void Mapping_window_update_interactive_tool(Widget widget,
	void *mapping_window_void,void *interactive_tool_void)
/*******************************************************************************
LAST MODIFIED : 4 September 2000

DESCRIPTION :
Called when a new tool is chosen in the interactive_toolbar_widget.
==============================================================================*/
{
	struct Mapping_window *mapping_window;

	ENTER(Mapping_window_update_interactive_tool);
	USE_PARAMETER(widget);
	if (mapping_window=(struct Mapping_window *)mapping_window_void)
	{
		Mapping_window_set_interactive_tool(mapping_window,
			(struct Interactive_tool *)interactive_tool_void);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mapping_window_update_interactive_tool.  Invalid argument(s)");
	}
	LEAVE;
} /* Mapping_window_update_interactive_tool */
#endif /*  defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int Mapping_window_make_drawing_area_3d(struct Mapping_window *mapping_window)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Removes the  2-D XmDrawingArea if any, and replaces it with a Scene_viewer,
the mapping_area of the <mapping_window>.
==============================================================================*/
{
	int return_code;
	struct Graphics_buffer *graphics_buffer;
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
			graphics_buffer = create_Graphics_buffer_X3d(mapping_window->map3d_viewing_form,
				GRAPHICS_BUFFER_DOUBLE_BUFFERING, GRAPHICS_BUFFER_MONO,
				/*minimum_colour_buffer_depth*/0,
				/*minimum_depth_buffer_depth*/0,
				/*minimum_accumulation_buffer_depth*/0,
				User_interface_get_specified_visual_id(
				get_map_drawing_information_user_interface(drawing_information)));
			mapping_window->scene_viewer=
				/* map3d_viewing_form is a sub-form of mapping_area_3d */
				CREATE(Scene_viewer)(graphics_buffer,
					get_map_drawing_information_background_colour(drawing_information),
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
			Mapping_window_set_interactive_tool(mapping_window,
				mapping_window->interactive_tool);
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
#endif /* defined (UNEMAP_USE_3D) */

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
#if defined (UNEMAP_USE_3D)
	struct FE_node_order_info *all_devices_node_order_info;
	struct Unemap_package *unemap_package;
#endif
	ENTER(mapping_read_configuration_file);
#if defined (UNEMAP_USE_3D)
	all_devices_node_order_info=(struct FE_node_order_info *)NULL;
#endif/* defined (UNEMAP_USE_3D) */
	if ((mapping=(struct Mapping_window *)mapping_window)&&(mapping->map))
	{
#if defined (UNEMAP_USE_3D)
		unemap_package=mapping->map->unemap_package;
#endif /* defined (UNEMAP_USE_3D) */
		/* destroy the existing configuration */
		destroy_Rig(mapping->map->rig_pointer);
		/* read the configuration file */
		if (return_code=read_configuration_file(file_name,
			(void *)(mapping->map->rig_pointer)
#if defined (UNEMAP_USE_3D)
			,unemap_package
#endif /* defined (UNEMAP_USE_3D) */
			))
		{
#if defined (UNEMAP_USE_3D)
			convert_config_rig_to_nodes(*(mapping->map->rig_pointer),
				&all_devices_node_order_info);
			/* not needed */
			DEACCESS(FE_node_order_info)(&all_devices_node_order_info);
#endif /* defined (UNEMAP_USE_3D) */
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

static void identify_mapping_file_save_elec(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 July 2001

DESCRIPTION :
Finds the id of the mapping file save electrode values button.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_file_save_elec);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)client_data)
	{
		mapping->file_menu.save_electrode_values_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_file_save_elec.  client_data missing");
	}
	LEAVE;
} /* identify_mapping_file_save_elec */

static int write_electrode_values_file(char *file_name,void *mapping_window)
/*******************************************************************************
LAST MODIFIED : 7 May 2004

DESCRIPTION :
Write the electrode values for the current map to a file.
==============================================================================*/
{
	FILE *output_file;
	float *value;
	int i,j,number_of_electrodes,return_code;
	struct Device **electrode;
	struct Map *map;
	struct Mapping_window *mapping;
	struct Sub_map *sub_map;

	ENTER(write_electrode_values_file);
	return_code=0;
	/*check we have at least one sub map. Multiple sub maps dealt with below*/
	if ((mapping=(struct Mapping_window *)mapping_window)&&(map=mapping->map)&&
		(map->sub_map)&&(map->type)&&file_name)
	{
		if ((0<(number_of_electrodes=map->number_of_electrodes))&&
			(electrode=map->electrodes)&&
			(value=(*(map->sub_map))->electrode_value)&&
			(NO_MAP_FIELD!= *(map->type)))
		{
			if (output_file=fopen(file_name,"wt"))
			{
				/* write the table format */
				fprintf(output_file,"table format : comma separated\n");
				fprintf(output_file,"Electrode");
				/* write the column headings */
				switch (*(map->type))
				{
					case SINGLE_ACTIVATION:
					case MULTIPLE_ACTIVATION:
					{
						fprintf(output_file,",Event_time\n");
					} break;
					case INTEGRAL:
					{
						fprintf(output_file,",Integral\n");
					} break;
					case POTENTIAL:
					{
						if (1<map->number_of_sub_maps)
						{
							for (j=1;j<=map->number_of_sub_maps;j++)
							{
								fprintf(output_file,",Potential_%d\n",j);
							}
						}
						else
						{
							fprintf(output_file,",Potential\n");
						}
					} break;
					case ACTIVATION_POTENTIAL:
					{
						fprintf(output_file,",Activation_potential\n");
					} break;
					default:
					{
						fprintf(output_file,",Unknown\n");
					} break;
				}
				/* write the values */
				switch (*(map->type))
				{
					case POTENTIAL:
					{
						for (i=0;i<number_of_electrodes;i++)
						{
							/* write the electrode name */
							fprintf(output_file,"%s",(*electrode)->description->name);
							/* for each the sub map */
							for (j=0;j<map->number_of_sub_maps;j++)
							{
								sub_map=map->sub_map[j];
								/* write value */
								fprintf(output_file,",%g",sub_map->electrode_value[i]);
							}
							fprintf(output_file,"\n");
							electrode++;
						}
					} break;
					default:
					{
						/* write the electrode names and values */
						for (i=number_of_electrodes;i>0;i--)
						{
							fprintf(output_file,"%s,%g\n",
								(*electrode)->description->name,*value);
							electrode++;
							value++;
						}
					} break;
				}
				fclose(output_file);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"Could not open: %s",file_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"No electrode values to write");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"write_electrode_values_file.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* write_electrode_values_file */

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

static void identify_mapping_print_jpg_but(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2001

DESCRIPTION :
Finds the id of the mapping print jpg button.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_print_jpg_but);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)client_data)
	{
		mapping->print_menu.jpg_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_print_jpg_but.  client_data missing");
	}
	LEAVE;
} /* identify_mapping_print_jpg_but */

static void identify_mapping_print_bmp_but(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 August 2002

DESCRIPTION :
Finds the id of the mapping print bmp button.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(identify_mapping_print_bmp_but);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)client_data)
	{
		mapping->print_menu.bmp_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_mapping_print_bmp_but.  client_data missing");
	}
	LEAVE;
} /* identify_mapping_print_bmp_but */

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

static void id_mapping_print_animation_jpg(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 May 2001

DESCRIPTION :
Finds the id of the mapping print animation jpg button.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(id_mapping_print_animation_jpg);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)client_data)
	{
		mapping->print_menu.animate_jpg_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_mapping_print_animation_jpg.  client_data missing");
	}
	LEAVE;
} /* id_mapping_print_animation_jpg */

static void id_mapping_print_animation_bmp(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 August 2002

DESCRIPTION :
Finds the id of the mapping print animation bmp button.
==============================================================================*/
{
	struct Mapping_window *mapping;

	ENTER(id_mapping_print_animation_bmp);
	USE_PARAMETER(call_data);
	if (mapping=(struct Mapping_window *)client_data)
	{
		mapping->print_menu.animate_bmp_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_mapping_print_animation_bmp.  client_data missing");
	}
	LEAVE;
} /* id_mapping_print_animation_bmp */

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
						display=User_interface_get_display(mapping->user_interface);
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

static int update_mapping_window_file_menu(struct Mapping_window *mapping)
/*******************************************************************************
LAST MODIFIED : 24 July 2001

DESCRIPTION :
Updates the sensitivity of the "Save electrode values" button in the mapping
window file menu.
==============================================================================*/
{
	int return_code;
	struct Map *map;

	ENTER(update_mapping_window_file_menu);
	return_code=0;
	if (mapping)
	{
		return_code=1;
		if ((map=mapping->map)&&(map->type)&&(0<map->number_of_electrodes)&&
			/* at least one sub_map*/
			(map->electrodes)&&(map->sub_map)&&
			((*map->sub_map)->electrode_value)&&(NO_MAP_FIELD!= *(map->type)))
		{
			XtSetSensitive(mapping->file_menu.save_electrode_values_button,True);
		}
		else
		{
			XtSetSensitive(mapping->file_menu.save_electrode_values_button,False);
		}
	}
	LEAVE;

	return (return_code);
} /* update_mapping_window_file_menu */

int mapping_window_set_animation_buttons(struct Mapping_window *mapping)
/*******************************************************************************
LAST MODIFIED : 18 August 2002

DESCRIPTION :
Sets the animation buttons of the mapping window based upon map information
==============================================================================*/
{
	int return_code;
	struct Map *map;

	ENTER(mapping_window_set_animation_buttons);
	return_code=0;
	if (mapping)
	{
		return_code=1;
		if ((map=mapping->map)&&(map->type)&&
			(ELECTRICAL_IMAGING!=(*map->analysis_mode))&&
			((SINGLE_ACTIVATION== *(map->type)||
			((MULTIPLE_ACTIVATION== *(map->type))&&
			(NO_INTERPOLATION==map->interpolation_type))||
			((POTENTIAL== *(map->type))&&
			((NO_INTERPOLATION==map->interpolation_type)||
			(DIRECT_INTERPOLATION==map->interpolation_type))))))
		{
			XtSetSensitive(mapping->animate_button,True);
			XtSetSensitive(mapping->print_menu.animate_rgb_button,True);
			XtSetSensitive(mapping->print_menu.animate_tiff_button,True);
			XtSetSensitive(mapping->print_menu.animate_jpg_button,True);
			XtSetSensitive(mapping->print_menu.animate_bmp_button,True);
		}
		else
		{
			XtSetSensitive(mapping->animate_button,False);
			XtSetSensitive(mapping->print_menu.animate_rgb_button,False);
			XtSetSensitive(mapping->print_menu.animate_tiff_button,False);
			XtSetSensitive(mapping->print_menu.animate_jpg_button,False);
			XtSetSensitive(mapping->print_menu.animate_bmp_button,False);
			/*stop the time keeper and get rid of the editor widget*/
			mapping_window_kill_time_keeper_editor(mapping);
		}
	}
	LEAVE;
	
	return (return_code);
} /* mapping_window_set_animation_buttons */

static void expose_mapping_drawing_area_2d(Widget widget,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 12 December 2001

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
						display=User_interface_get_display(mapping->user_interface);
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
									create_Drawing_2d(mapping->map_drawing_area_2d,
									attributes.width,attributes.height,NO_DRAWING_IMAGE,
									mapping->user_interface))
								{
									mapping->map_drawing=drawing;
									/* clear the map drawing area */
									XFillRectangle(display,drawing->pixel_map,(mapping->map->
										drawing_information->graphics_context).
										background_drawing_colour,0,0,drawing->width,
										drawing->height);
									/* draw the map */
									draw_map(mapping->map,2/*recalculate*/,drawing);
									update_mapping_colour_or_auxili(mapping);
									/* set the sensitivity of the save electrode values button */
									update_mapping_window_file_menu(mapping);
									mapping_window_update_time_limits(mapping);
									mapping_window_set_animation_buttons(mapping);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"expose_mapping_drawing_area_2d.  "
										"Could not create drawing");
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

static void resize_mapping_drawing_area_2d(Widget widget,
	XtPointer mapping_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 July 2001

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
						display=User_interface_get_display(mapping->user_interface);
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

							if (mapping->map->activation_front==0)
							{
								/*playing a movie*/
								draw_map(mapping->map,2,drawing);
							}
							else
							{
								/*manual time update*/
								update_map_from_manual_time_update(mapping);
							}
							/* set the sensitivity of the save electrode values button */
							update_mapping_window_file_menu(mapping);
							mapping_window_set_animation_buttons(mapping);
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
#if defined (UNEMAP_USE_3D)
				set_map_drawing_information_electrodes_accepted_or_rejected(
					mapping->map->drawing_information,1);
#endif
				mapping->map->projection_type=HAMMER_PROJECTION;
				Mapping_window_make_drawing_area_2d(mapping);
				update_mapping_drawing_area(mapping,2);
				/*need to regenerate maps if change projection type*/
				XtSetSensitive(mapping->animate_button,False);
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
#if defined (UNEMAP_USE_3D)
				set_map_drawing_information_electrodes_accepted_or_rejected(
					mapping->map->drawing_information,1);
#endif
				mapping->map->projection_type=POLAR_PROJECTION;
				Mapping_window_make_drawing_area_2d(mapping);
				update_mapping_drawing_area(mapping,2);
				mapping_window_set_animation_buttons(mapping);
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
#if defined (UNEMAP_USE_3D)
				set_map_drawing_information_electrodes_accepted_or_rejected(
					mapping->map->drawing_information,1);
#endif
			/*must set the rojection_type to something*/
			mapping->map->projection_type=CYLINDRICAL_PROJECTION;
			Mapping_window_make_drawing_area_2d(mapping);
			update_mapping_drawing_area(mapping,2);
			mapping_window_set_animation_buttons(mapping);
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
#if defined (UNEMAP_USE_3D)
				set_map_drawing_information_electrodes_accepted_or_rejected(
					mapping->map->drawing_information,1);
#endif
				mapping->map->projection_type=CYLINDRICAL_PROJECTION;
				Mapping_window_make_drawing_area_2d(mapping);
				update_mapping_drawing_area(mapping,2);
				mapping_window_set_animation_buttons(mapping);
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
#if defined (UNEMAP_USE_3D)
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
#if defined (UNEMAP_USE_3D)
				set_map_drawing_information_electrodes_accepted_or_rejected(
					mapping->map->drawing_information,1);
#endif
				mapping->map->projection_type=THREED_PROJECTION;
				Mapping_window_make_drawing_area_3d(mapping);
				update_mapping_drawing_area(mapping,2);
				/*need to regenerate maps if change projection type*/
				XtSetSensitive(mapping->animate_button,False);
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
#endif /* defined (UNEMAP_USE_3D) */
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
						display=User_interface_get_display(mapping->user_interface);
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
LAST MODIFIED : 1 February 2002

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
#if defined (UNEMAP_USE_3D)
		if (mapping->scene_viewer)
		{
			DESTROY(Scene_viewer)(&(mapping->scene_viewer));
		}
#if defined (NEW_CODE)
		/*??JW should really do this, but at the moment CMGUI doesn't destroy */
		/* the mapping window, so deaccess never gets called. See ACCESS */
		DEACCESS(Interactive_tool)(&(mapping->transform_tool));
#endif /*	defined (NEW_CODE) */
		if (mapping->spectrum_manager_callback_id)
		{
			MANAGER_DEREGISTER(Spectrum)(
				mapping->spectrum_manager_callback_id,
				get_map_drawing_information_spectrum_manager(
				mapping->map->drawing_information));
		}
#endif /*defined (UNEMAP_USE_3D)*/
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
LAST MODIFIED : 3 May 2004

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
			if (XGetWindowAttributes(
				User_interface_get_display(mapping->user_interface),
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
								User_interface_get_display(mapping->user_interface));
							if ((float)(mapping->map_drawing->height)*pixel_aspect_ratio*
								postscript_page_width<0.85*postscript_page_height*
								(float)(mapping->map_drawing->width))
							{
								page_height=postscript_page_width*pixel_aspect_ratio*
									(float)(mapping->map_drawing->height)/
									(float)(mapping->map_drawing->width);
								set_postscript_display_transfor (0,0.85*postscript_page_height-
									page_height,postscript_page_width,page_height,0,0,
									(float)(mapping->map_drawing->width),
									(float)(mapping->map_drawing->height));
							}
							else
							{
								page_width=0.85*postscript_page_height*
									(float)(mapping->map_drawing->width)/(pixel_aspect_ratio*
									(float)(mapping->map_drawing->height));
								set_postscript_display_transfor (
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
						set_postscript_display_transfor (0,0.9*postscript_page_height,
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
								User_interface_get_display(mapping->user_interface));
							if ((float)(mapping->map_drawing->height)*pixel_aspect_ratio*
								postscript_page_width<postscript_page_height*
								(float)(mapping->map_drawing->width))
							{
								page_height=postscript_page_width*pixel_aspect_ratio*
									(float)(mapping->map_drawing->height)/
									(float)(mapping->map_drawing->width);
								set_postscript_display_transfor (0,postscript_page_height-
									page_height,postscript_page_width,page_height,0,0,
									(float)(mapping->map_drawing->width),
									(float)(mapping->map_drawing->height));
							}
							else
							{
								page_width=postscript_page_height*
									(float)(mapping->map_drawing->width)/(pixel_aspect_ratio*
									(float)(mapping->map_drawing->height));
								set_postscript_display_transfor (
									(postscript_page_width-page_width)/2,0,page_width,
									postscript_page_height,0,0,
									(float)(mapping->map_drawing->width),
									(float)(mapping->map_drawing->height));
							}
						}
						else
						{
							set_postscript_display_transfor (0,0,postscript_page_width,
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

static int write_map_file(char *file_name,
	enum Image_file_format image_file_format,struct Mapping_window *mapping)
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
This function writes the image map associated with the <mapping> window to
<file_name> using the requested <image_file_format>.
==============================================================================*/
{
	int bytes_per_pixel,height,number_of_bytes_per_component,
		number_of_components,return_code,width;
	unsigned long *image;
	struct Cmgui_image *cmgui_image;
	struct Cmgui_image_information *cmgui_image_information;

	ENTER(write_map_file);
	if (file_name && mapping)
	{
		if (image=get_Drawing_2d_image(mapping->map_drawing))
		{
			width=mapping->map_drawing->width;
			height=mapping->map_drawing->height;
			number_of_components=3;
			number_of_bytes_per_component=1;
			bytes_per_pixel=number_of_components*number_of_bytes_per_component;
			if (cmgui_image=Cmgui_image_constitute(width,height,number_of_components,
				number_of_bytes_per_component,width*bytes_per_pixel,
				(unsigned char *)image))
			{
				cmgui_image_information=CREATE(Cmgui_image_information)();
				Cmgui_image_information_add_file_name(cmgui_image_information,
					file_name);
				Cmgui_image_information_set_image_file_format(cmgui_image_information,
					image_file_format);
				return_code=Cmgui_image_write(cmgui_image,cmgui_image_information);
				DESTROY(Cmgui_image_information)(&cmgui_image_information);
				DESTROY(Cmgui_image)(&cmgui_image);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"write_map_file.  Could not constitute image");
				return_code=0;
			}
			DEALLOCATE(image);
		}
		else
		{
			display_message(ERROR_MESSAGE,"write_map_file.  Could not get image");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_map_file.  Missing file_name or mapping_window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_map_file */

static int write_map_rgb_file(char *file_name,void *mapping_window)
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
This function writes the rgb for drawing the map associated with the
mapping_window.
==============================================================================*/
{
	int return_code;

	ENTER(write_map_rgb_file);
	return_code=write_map_file(file_name,RGB_FILE_FORMAT,
		(struct Mapping_window *)mapping_window);
	LEAVE;

	return (return_code);
} /* write_map_rgb_file */

static int write_map_tiff_file(char *file_name,void *mapping_window)
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
This function writes the tiff for drawing the map associated with the
mapping_window.
==============================================================================*/
{
	int return_code;

	ENTER(write_map_tiff_file);
	return_code=write_map_file(file_name,TIFF_FILE_FORMAT,
		(struct Mapping_window *)mapping_window);
	LEAVE;

	return (return_code);
} /* write_map_tiff_file */

static int write_map_jpg_file(char *file_name,void *mapping_window)
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
This function writes the jpg for drawing the map associated with the
mapping_window.
==============================================================================*/
{
	int return_code;

	ENTER(write_map_jpg_file);
	return_code=write_map_file(file_name,JPG_FILE_FORMAT,
		(struct Mapping_window *)mapping_window);
	LEAVE;

	return (return_code);
} /* write_map_jpg_file */

static int write_map_bmp_file(char *file_name,void *mapping_window)
/*******************************************************************************
LAST MODIFIED : 18 August 2002

DESCRIPTION :
This function writes the bmp for drawing the map associated with the
mapping_window.
==============================================================================*/
{
	int return_code;

	ENTER(write_map_bmp_file);
	return_code=write_map_file(file_name,BMP_FILE_FORMAT,
		(struct Mapping_window *)mapping_window);
	LEAVE;

	return (return_code);
} /* write_map_bmp_file */

static int write_map_animation_files(char *file_name,void *mapping_window,
	enum Image_file_format image_file_format)
/*******************************************************************************
LAST MODIFIED : 24 April 2004

DESCRIPTION :
This function writes the files for drawing the animation associated with the
mapping_window.
==============================================================================*/
{
	char number_str[4],question_str[80],success_str[80],*temp_char1,*temp_char2,
		*temp_file_name,warning_str[80];
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
			display=User_interface_get_display(drawing_information->user_interface);
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
					switch (map->interpolation_type)
					{
						case BICUBIC_INTERPOLATION:
						{
							number_of_frames=map->number_of_sub_maps;
							map->activation_front=map->sub_map_number;
							map->sub_map_number=0;
						} break;
						case NO_INTERPOLATION:
						case DIRECT_INTERPOLATION:
						default:
						{
							number_of_frames= *(map->end_search_interval)-
								*(map->start_search_interval);
							map->activation_front= *(map->potential_time);
							*(map->potential_time)= *(map->start_search_interval);
						} break;
					}/*switch (map->interpolation_type)*/
				}
			}
		}
		number_of_digits=0;
		i=number_of_frames;
		/* Confirm (possibly lengthy) frame writing */
		sprintf(number_str,"%d",number_of_frames-1);
		strcpy(warning_str,"Writing ");
		strcat(warning_str,number_str);
		strcat(warning_str," map frames.");
			strcpy(question_str,"Could take some time. Do you want to do this? (A completion message will appear.)");
		if (confirmation_question_yes_no(warning_str,question_str,
#if defined (MOTIF)
			(Widget)(NULL),
#endif /* defined (MOTIF) */
			drawing_information->user_interface))
		{
			do
			{
				i /= 10;
				number_of_digits++;
			} while (i>0);
			if (ALLOCATE(temp_file_name,char,strlen(file_name)+number_of_digits+1))
			{
				busy_cursor_on((Widget)NULL,drawing_information->user_interface);
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
				for (frame_number=0;frame_number<number_of_frames-1;frame_number++)
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
									if (1<number_of_contours)
									{
										cell_number=(int)(((contour_maximum*(float)i+
											contour_minimum*(float)(number_of_contours-1-i))/
											(float)(number_of_contours-1)-minimum_value)/
											(maximum_value-minimum_value)*
											(float)(number_of_spectrum_colours-1)+0.5);
									}
									else
									{
										cell_number=(int)((contour_minimum-minimum_value)/
											(maximum_value-minimum_value)*
											(float)(number_of_spectrum_colours-1)+0.5);
									}
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
							if (drawing_information->boundary_colour)
							{
								colour.pixel=drawing_information->boundary_colour;
								colour.flags=DoRed|DoGreen|DoBlue;
								XStoreColor(display,colour_map,&colour);
							}
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
								switch (map->interpolation_type)
								{
									case BICUBIC_INTERPOLATION:
									{
										update_mapping_drawing_area(mapping,0);
										(map->sub_map_number)++;
									} break;
									case NO_INTERPOLATION:
									case DIRECT_INTERPOLATION:
									default:
									{
										update_mapping_drawing_area(mapping,2);
										update_mapping_colour_or_auxili(mapping);
										/*???DB.  What about the trace window ? */
										(*(map->potential_time))++;
									} break;
								}/* map->interpolation_type */
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
						case JPG_FILE_FORMAT:
						{
							write_map_jpg_file(temp_file_name,mapping_window);
						} break;
						case BMP_FILE_FORMAT:
						{
							write_map_bmp_file(temp_file_name,mapping_window);
						} break;
					}
				}
				DEALLOCATE(temp_file_name);
				busy_cursor_off((Widget)NULL,drawing_information->user_interface);
				/* write success message */
				strcpy(success_str," Map frames ");
				strcat(success_str," successfully written .");
				confirmation_information_ok("Success!",success_str,
#if defined (MOTIF)
					(Widget)(NULL),
#endif /* defined (MOTIF) */
					drawing_information->user_interface);
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
						map->activation_front= -1;
						update_mapping_drawing_area(mapping,2);
						update_mapping_colour_or_auxili(mapping);
						/*???DB.  What about the trace window ? */
					}
					else
					{
						map->sub_map_number=map->activation_front;
						map->activation_front= -1;
						update_mapping_drawing_area(mapping,0);
					}
				}
			}
		}/* if (confirmation_question_yes_no */
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

static int write_map_animation_jpg_file(char *file_name,void *mapping_window)
/*******************************************************************************
LAST MODIFIED : 23 May 2001

DESCRIPTION :
This function writes the jpg files for drawing the animation associated with
the mapping_window.
==============================================================================*/
{
	int return_code;

	ENTER(write_map_animation_jpg_file);
	return_code=write_map_animation_files(file_name,mapping_window,
		JPG_FILE_FORMAT);
	LEAVE;

	return (return_code);
} /* write_map_animation_jpg_file */

static int write_map_animation_bmp_file(char *file_name,void *mapping_window)
/*******************************************************************************
LAST MODIFIED : 18 August 2002

DESCRIPTION :
This function writes the bmp files for drawing the animation associated with
the mapping_window.
==============================================================================*/
{
	int return_code;

	ENTER(write_map_animation_bmp_file);
	return_code=write_map_animation_files(file_name,mapping_window,
		BMP_FILE_FORMAT);
	LEAVE;

	return (return_code);
} /* write_map_animation_bmp_file */

#if defined (UNEMAP_USE_3D)
static void mapping_window_spectrum_change(
	struct MANAGER_MESSAGE(Spectrum) *message,void *mapping_void)
/*******************************************************************************
LAST MODIFIED : 23 April 2004

DESCRIPTION :
==============================================================================*/
{
	float minimum_value,maximum_value;
	struct Mapping_window *mapping;
	struct Spectrum *spectrum;

	ENTER(mapping_window_spectrum_change);
	if (message&&(mapping=(struct Mapping_window *)mapping_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_OBJECT(Spectrum):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Spectrum):
			{
				spectrum=mapping->map->drawing_information->spectrum;
				if (IS_OBJECT_IN_LIST(Spectrum)(spectrum,
					message->changed_object_list))
				{
					minimum_value=get_Spectrum_minimum(spectrum);
					maximum_value=get_Spectrum_maximum(spectrum);
					if ((minimum_value!=mapping->map->minimum_value)||
						(maximum_value!=mapping->map->maximum_value))
					{
						mapping->map->minimum_value=minimum_value;
						mapping->map->maximum_value=maximum_value;
						if (mapping->colour_or_auxiliary_drawing)
						{
							/* clear the colour or auxiliary area */
							XFillRectangle(User_interface_get_display(
								mapping->map->drawing_information->user_interface),
								mapping->colour_or_auxiliary_drawing->pixel_map,
								(mapping->map->drawing_information->graphics_context).background_drawing_colour,
								0,0,mapping->colour_or_auxiliary_drawing->width,
								mapping->colour_or_auxiliary_drawing->height);
							draw_colour_or_auxiliary_area(mapping->map,
								mapping->colour_or_auxiliary_drawing);
							XCopyArea(User_interface_get_display(
								mapping->map->drawing_information->user_interface),
								mapping->colour_or_auxiliary_drawing->pixel_map,
								XtWindow(mapping->colour_or_auxiliary_drawing_area),
								(mapping->map->drawing_information->graphics_context).copy,0,0,
								mapping->colour_or_auxiliary_drawing->width,
								mapping->colour_or_auxiliary_drawing->height,0,0);
						}
#if defined (UNEMAP_USE_3D)
						if (mapping->map->projection_type==THREED_PROJECTION)
						{
							map_draw_contours(mapping->map,spectrum,
								mapping->map->unemap_package);
						}
#endif /* defined (UNEMAP_USE_3D)*/
					}
				}
			} break;
			case MANAGER_CHANGE_ADD(Spectrum):
			case MANAGER_CHANGE_REMOVE(Spectrum):
			case MANAGER_CHANGE_IDENTIFIER(Spectrum):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"mapping_window_spectrum_change.  Invalid argument(s)");
	}
	LEAVE;
} /* mapping_window_spectrum_change */
#endif /* defined (UNEMAP_USE_3D)*/

static struct Mapping_window *create_Mapping_window(
	struct Mapping_window **address,char *open,
	struct Mapping_window **current_address,Widget activation,Widget parent,
	struct Map *map,struct Rig **rig_pointer,Pixel identifying_colour,
	int screen_height,char *configuration_file_extension,
	char *postscript_file_extension,
	struct Map_drawing_information *map_drawing_information,
	struct User_interface *user_interface
#if defined (UNEMAP_USE_3D)
	,struct MANAGER(Interactive_tool) *interactive_tool_manager
#endif /*  defined (UNEMAP_USE_3D) */
	)
/*******************************************************************************
LAST MODIFIED : 27 November 2003

DESCRIPTION :
This function allocates the memory for a mapping_window and sets the fields to
the specified values (<address>,<map>).  It then retrieves a mapping window
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
			{"configure_map_dialog_and_open",
				(XtPointer)configure_map_dialog_and_open},
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
			{"identify_mapping_file_save_elec",
				(XtPointer)identify_mapping_file_save_elec},
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
			{"identify_mapping_print_jpg_but",
				(XtPointer)identify_mapping_print_jpg_but},
			{"identify_mapping_print_bmp_but",
				(XtPointer)identify_mapping_print_bmp_but},
			{"id_mapping_print_animation_rgb",
				(XtPointer)id_mapping_print_animation_rgb},
			{"id_mapping_print_animation_tiff",
				(XtPointer)id_mapping_print_animation_tiff},
			{"id_mapping_print_animation_jpg",
				(XtPointer)id_mapping_print_animation_jpg},
			{"id_mapping_print_animation_bmp",
				(XtPointer)id_mapping_print_animation_bmp},
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
			/*The following 2 are only used for unemap_nodes, but only have one uil file*/
			/* so need these functions in all versions */
			{"map3d_id_interactive_tool_form",(XtPointer)map3d_id_interactive_tool_form},
			{"map3d_id_viewing_form",(XtPointer)map3d_id_viewing_form},
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
			{"redraw_map_from_dialog",(XtPointer)redraw_map_from_dialog},
			{"update_dialog_elec_options",(XtPointer)update_dialog_elec_options}
		},
		identifier_list[]=
		{
			{"mapping_window_structure",(XtPointer)NULL},
			{"read_configuration_file_data",(XtPointer)NULL},
			{"read_bard_electrode_file_data",(XtPointer)NULL},
			{"write_configuration_file_data",(XtPointer)NULL},
			{"write_electrode_values_data",(XtPointer)NULL},
			{"write_map_postscript_file_data",(XtPointer)NULL},
			{"write_map_rgb_file_data",(XtPointer)NULL},
			{"write_map_tiff_file_data",(XtPointer)NULL},
			{"write_map_jpg_file_data",(XtPointer)NULL},
			{"write_map_bmp_file_data",(XtPointer)NULL},
			{"write_map_animate_rgb_file_data",(XtPointer)NULL},
			{"write_map_animate_tiff_file_dat",(XtPointer)NULL},
			{"write_map_animate_jpg_file_dat",(XtPointer)NULL},
			{"write_map_animate_bmp_file_dat",(XtPointer)NULL},
			{"mapping_rig",(XtPointer)NULL},
			{"identifying_colour",(XtPointer)NULL}
		};
#if defined (UNEMAP_USE_3D)
	struct Callback_data callback;
	struct Interactive_tool *node_tool;
#endif
	struct Mapping_window *mapping;
	Widget child_widget;

	ENTER(create_Mapping_window);
#if defined (UNEMAP_USE_3D)
	node_tool=(struct Interactive_tool *)NULL;
#endif
	if (map_drawing_information&&
#if defined (UNEMAP_USE_3D)
		interactive_tool_manager&&
#endif /* defined (UNEMAP_USE_3D) */
		user_interface)
	{
		if (mapping_window_hierarchy_open)
		{
			/* allocate memory */
			if (ALLOCATE(mapping,struct Mapping_window,1))
			{
#if defined (UNEMAP_USE_3D)
				/* set up interactive tools*/
				mapping->interactive_tool_manager=interactive_tool_manager;
#if defined (NEW_CODE)
				/*??JW should really do this, but at the moment CMGUI doesn't destroy */
				/* the mapping window, so deaccess never gets called. See DEACCESS */
				mapping->transform_tool=ACCESS(Interactive_tool)(
					FIND_BY_IDENTIFIER_IN_MANAGER(Interactive_tool,name)(
						"transform_tool",mapping->interactive_tool_manager));
#else
				mapping->transform_tool=FIND_BY_IDENTIFIER_IN_MANAGER(Interactive_tool,
					name)("transform_tool",mapping->interactive_tool_manager);
#endif /* defined (NEW_CODE)	*/
				mapping->interactive_tool=mapping->transform_tool;
#endif /* defined (UNEMAP_USE_3D) */
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
				mapping->file_menu.save_electrode_values_button=(Widget)NULL;
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
				mapping->potential_time_object=(struct Time_object *)NULL;
				mapping->print_button=(Widget)NULL;
				(mapping->print_menu).postscript_button=(Widget)NULL;
				(mapping->print_menu).rgb_button=(Widget)NULL;
				(mapping->print_menu).tiff_button=(Widget)NULL;
				(mapping->print_menu).jpg_button=(Widget)NULL;
				(mapping->print_menu).bmp_button=(Widget)NULL;
				(mapping->print_menu).animate_rgb_button=(Widget)NULL;
				(mapping->print_menu).animate_tiff_button=(Widget)NULL;
				(mapping->print_menu).animate_jpg_button=(Widget)NULL;
				(mapping->print_menu).animate_bmp_button=(Widget)NULL;
				mapping->close_button=(Widget)NULL;
				mapping->mapping_area=(Widget)NULL;
				mapping->mapping_area_2d=(Widget)NULL;
				mapping->mapping_area_3d=(Widget)NULL;
#if defined (UNEMAP_USE_3D)
				mapping->map3d_interactive_tool_form=(Widget)NULL;
				mapping->interactive_toolbar_widget=(Widget)NULL;
				mapping->map3d_viewing_form=(Widget)NULL;
#endif /* defined (UNEMAP_USE_3D) */
				mapping->time_editor_dialog=(struct Time_editor_dialog *)NULL;
				mapping->map_drawing_area_2d=(Widget)NULL;
				mapping->scene_viewer=(struct Scene_viewer *)NULL;
				mapping->map_drawing=(struct Drawing_2d *)NULL;
				mapping->colour_or_auxiliary_drawing_area=(Widget)NULL;
				mapping->colour_or_auxiliary_drawing=(struct Drawing_2d *)NULL;
				mapping->colour_or_auxiliary_scroll_bar=(Widget)NULL;
#if defined (UNEMAP_USE_3D)
				mapping->spectrum_manager_callback_id=
					MANAGER_REGISTER(Spectrum)(mapping_window_spectrum_change,
					(void *)mapping,get_map_drawing_information_spectrum_manager(
					mapping->map->drawing_information));
#endif /* defined (UNEMAP_USE_3D) */
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
					identifier_list[4].value=(XtPointer)create_File_open_data(".val",
						REGULAR,write_electrode_values_file,(void *)mapping,0,
						user_interface);
					identifier_list[5].value=(XtPointer)create_File_open_data(
						postscript_file_extension,REGULAR,write_map_postscript_file,
						(void *)mapping,1,user_interface);
					identifier_list[6].value=(XtPointer)create_File_open_data(".rgb",
						REGULAR,write_map_rgb_file,(void *)mapping,1,user_interface);
					identifier_list[7].value=(XtPointer)create_File_open_data(".tif",
						REGULAR,write_map_tiff_file,(void *)mapping,1,user_interface);
					identifier_list[8].value=(XtPointer)create_File_open_data(".jpg",
						REGULAR,write_map_jpg_file,(void *)mapping,1,user_interface);
					identifier_list[9].value=(XtPointer)create_File_open_data(".bmp",
						REGULAR,write_map_bmp_file,(void *)mapping,1,user_interface);
					identifier_list[10].value=(XtPointer)create_File_open_data(".rgb",
						REGULAR,write_map_animation_rgb_file,(void *)mapping,1,
						user_interface);
					identifier_list[11].value=(XtPointer)create_File_open_data(".tif",
						REGULAR,write_map_animation_tiff_file,(void *)mapping,1,
						user_interface);
					identifier_list[12].value=(XtPointer)create_File_open_data(".jpg",
						REGULAR,write_map_animation_jpg_file,(void *)mapping,1,
						user_interface);
					identifier_list[13].value=(XtPointer)create_File_open_data(".bmp",
						REGULAR,write_map_animation_bmp_file,(void *)mapping,1,
						user_interface);
					identifier_list[14].value=(XtPointer)rig_pointer;
					identifier_list[15].value=(XtPointer)identifying_colour;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(mapping_window_hierarchy,
						identifier_list,XtNumber(identifier_list)))
					{
						/* fetch mapping window widget */
						if (MrmSUCCESS==MrmFetchWidget(mapping_window_hierarchy,
							"mapping_window",parent,&(mapping->window),&mapping_window_class))
						{
							/* other Xt stuff set up in */						
							widget_spacing=User_interface_get_widget_spacing(user_interface);
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
#if defined (UNEMAP_USE_3D)
							/* create the subwidgets with default values */
							if (mapping->interactive_toolbar_widget=
								create_interactive_toolbar_widget(
								mapping->map3d_interactive_tool_form,interactive_tool_manager,
								INTERACTIVE_TOOLBAR_VERTICAL))
							{
								/* add tools to toolbar */
								add_interactive_tool_to_interactive_toolbar_widget(
									mapping->transform_tool,
									(void *)mapping->interactive_toolbar_widget);
								node_tool=FIND_BY_IDENTIFIER_IN_MANAGER(Interactive_tool,name)(
									"node_tool",mapping->interactive_tool_manager);
								add_interactive_tool_to_interactive_toolbar_widget(node_tool,
									(void *)mapping->interactive_toolbar_widget);

								/* make sure the transform_tool is currently set */
								interactive_toolbar_widget_set_current_interactive_tool(
									mapping->interactive_toolbar_widget,
									mapping->transform_tool);
							}
							/*set up callback*/
							callback.data=mapping;
							callback.procedure=Mapping_window_update_interactive_tool;
							interactive_toolbar_widget_set_callback(
								mapping->interactive_toolbar_widget,&callback);
#endif /*  defined (UNEMAP_USE_3D) */
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
	enum Electrodes_label_type electrodes_label_type,
	enum Fibres_option fibres_option,enum Landmarks_option landmarks_option,
	enum Extrema_option extrema_option,int maintain_aspect_ratio,
	int regions_use_same_coordinates,int print_spectrum,
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
	struct User_interface *user_interface,struct Unemap_package *unemap_package,
	struct Electrical_imaging_event **first_eimaging_event,
	enum Signal_analysis_mode *analysis_mode)
/*******************************************************************************
LAST MODIFIED : 3 May 2004

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
#if defined (UNEMAP_USE_3D)
		&&unemap_package
#endif /* defined (UNEMAP_USE_3D) */
		)
	{
		return_code=1;
		if (mapping= *mapping_address)
		{
			if ((mapping->map)&&(mapping->map_dialog)&&
				(mapping->map->drawing_information))
			{
				redraw_map_from_dialog((Widget)NULL,(XtPointer)mapping,(XtPointer)NULL);
			}
		}
		else
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
					create_Map(map_type,colour_option,contours_option,
					electrodes_label_type,fibres_option,landmarks_option,extrema_option,
					maintain_aspect_ratio,regions_use_same_coordinates,
					print_spectrum,projection_type,contour_thickness,rig_address,
					event_number_address,potential_time_address,datum_address,
					start_search_interval,end_search_interval,map_drawing_information,
					user_interface,unemap_package,first_eimaging_event,analysis_mode),
					rig_address,identifying_colour,screen_height,
					configuration_file_extension,postscript_file_extension,
					map_drawing_information,user_interface
#if defined (UNEMAP_USE_3D)
					,get_unemap_package_interactive_tool_manager(unemap_package)
#endif /* defined (UNEMAP_USE_3D) */
						))
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
							XtSetSensitive(mapping->print_menu.animate_jpg_button,False);
							XtSetSensitive(mapping->print_menu.animate_bmp_button,False);
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
								XtSetSensitive(mapping->print_menu.animate_jpg_button,False);
								XtSetSensitive(mapping->print_menu.animate_bmp_button,False);
							}
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"open_mapping_window.  Invalid mapping associate");
						} break;
					}
					configure_map_dialog(mapping);
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

static int update_mapping_drawing_area_2d(struct Mapping_window *mapping,
	int recalculate)
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
		XFillRectangle(
			User_interface_get_display(drawing_information->user_interface),
			drawing->pixel_map,(drawing_information->graphics_context).
			background_drawing_colour,0,0,drawing->width,drawing->height);
		/* draw the map */
		draw_map(mapping->map,recalculate,drawing);
		XCopyArea(User_interface_get_display(drawing_information->user_interface),
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
LAST MODIFIED : 24 July 2001

DESCRIPTION :
Calls draw_map_3d or update_mapping_drawing_area_2d depending upon
<mapping> ->map->projection_type
==============================================================================*/
{
	int return_code;

	ENTER(update_mapping_drawing_area);
	if (mapping)
	{
#if defined (UNEMAP_USE_3D)
		/* 3d map for 3d projection */
		if ((mapping->map)&&(THREED_PROJECTION==mapping->map->projection_type))
		{
			return_code=draw_map(mapping->map,recalculate,mapping->map_drawing);
			/* Force the 3d windows to be refreshed */
#if defined (MOTIF)
			Scene_viewer_redraw_now(mapping->scene_viewer);
#endif /* defined (MOTIF) */
		}
		else
		{
			return_code=update_mapping_drawing_area_2d(mapping,recalculate);
		}
#else /* defined (UNEMAP_USE_3D) */
		/* old, 2d map */
		update_mapping_drawing_area_2d(mapping,recalculate);
#endif /* defined (UNEMAP_USE_3D) */
		/* set the sensitivity of the save electrode values button */
		update_mapping_window_file_menu(mapping);
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* update_mapping_drawing_area */

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
		XFillRectangle(User_interface_get_display(drawing_information->user_interface),
			drawing->pixel_map,
			(drawing_information->graphics_context).background_drawing_colour,
			0,0,drawing->width,drawing->height);
		/* draw the colour bar or the auxiliary devices */
		draw_colour_or_auxiliary_area(mapping->map,drawing);
		XCopyArea(User_interface_get_display(drawing_information->user_interface),
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
	struct Region *region,*the_current_region;
	struct Region_list_item *region_item;
	struct Rig *rig;
	Window mapping_menu_window;

	ENTER(update_mapping_window_menu);
	the_current_region=(struct Region *)NULL;
	region=(struct Region *)NULL;
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
					XtSetArg(attributes[1],XmNfontList,
						User_interface_get_button_fontlist(mapping->user_interface));
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
#if defined (UNEMAP_USE_3D)
						/*all use 3d*/
						XtSetSensitive(mapping->projection_3d,True);
#endif /*	defined (UNEMAP_USE_3D)	*/
						if (rig->current_region)
						{
							switch (rig->current_region->type)
							{
								case SOCK:
								{
									XtSetSensitive(mapping->projection_hammer,True);
									XtSetSensitive(mapping->projection_polar,True);
									/* set the projection choic */
									XtVaGetValues(mapping->projection_choice,XmNmenuHistory,
										&current_projection,NULL);
									if ((current_projection!=mapping->projection_hammer)&&
#if defined (UNEMAP_USE_3D)
										(current_projection!=mapping->projection_3d)&&
#endif /*	defined (UNEMAP_USE_3D)	*/
										(current_projection!=mapping->projection_polar))
									{
										XtVaSetValues(mapping->projection_choice,
											XmNmenuHistory,mapping->projection_hammer,
											NULL);
									}
								} break;
								case TORSO:
								{
									XtSetSensitive(mapping->projection_cylinder,True);
									/* set the projection choice */
									XtVaGetValues(mapping->projection_choice,XmNmenuHistory,
										&current_projection,NULL);
									if ((current_projection!=mapping->projection_cylinder)
#if defined (UNEMAP_USE_3D)
										&&(current_projection!=mapping->projection_3d)
#endif /*	defined (UNEMAP_USE_3D)	*/
											)
									{
										XtVaSetValues(mapping->projection_choice,
											XmNmenuHistory,mapping->projection_cylinder,
											NULL);
									}
								} break;
								case PATCH:
								{
									XtSetSensitive(mapping->projection_patch,True);
									/* set the projection choice */
									XtVaGetValues(mapping->projection_choice,XmNmenuHistory,
										&current_projection,NULL);
									if ((current_projection!=mapping->projection_patch)
#if defined (UNEMAP_USE_3D)
										&&(current_projection!=mapping->projection_3d)
#endif /*	defined (UNEMAP_USE_3D)	*/
											)
									{
										XtVaSetValues(mapping->projection_choice,
											XmNmenuHistory,mapping->projection_patch,
											NULL);
									}
								} break;
							}/* switch (rig->current_region->type) */
						}/*  if (rig->current_region) */
						else
						/*This is a mixed rig */
						{
							/*need to scan for all the region types */
							region_item=get_Rig_region_list(rig);
							while (region_item)
							{
								the_current_region=get_Region_list_item_region(region_item);
								switch (the_current_region->type)
								{
									case SOCK:
									{
										XtSetSensitive(mapping->projection_hammer,True);
										XtSetSensitive(mapping->projection_polar,True);
										/* set the projection choice */
										XtVaGetValues(mapping->projection_choice,XmNmenuHistory,
											&current_projection,NULL);
										if ((current_projection!=mapping->projection_hammer)&&
#if defined (UNEMAP_USE_3D)
											(current_projection!=mapping->projection_3d)&&
#endif /*	defined (UNEMAP_USE_3D)	*/
											(current_projection!=mapping->projection_polar))
										{
											XtVaSetValues(mapping->projection_choice,
												XmNmenuHistory,mapping->projection_hammer,
												NULL);
										}
									} break;
									case PATCH:
									{
										XtSetSensitive(mapping->projection_patch,True);
										/* set the projection choice */
										XtVaGetValues(mapping->projection_choice,XmNmenuHistory,
											&current_projection,NULL);
										if ((current_projection!=mapping->projection_patch)
#if defined (UNEMAP_USE_3D)
											&&(current_projection!=mapping->projection_3d)
#endif /*	defined (UNEMAP_USE_3D)	*/
												)
										{
											XtVaSetValues(mapping->projection_choice,
												XmNmenuHistory,mapping->projection_patch,
												NULL);
										}
									} break;
									case TORSO:
									{
										XtSetSensitive(mapping->projection_cylinder,True);
										/* set the projection choice */
										XtVaGetValues(mapping->projection_choice,XmNmenuHistory,
											&current_projection,NULL);
										if ((current_projection!=mapping->projection_cylinder)
#if defined (UNEMAP_USE_3D)
											&&(current_projection!=mapping->projection_3d)
#endif /*	defined (UNEMAP_USE_3D)	*/
												)
										{
											XtVaSetValues(mapping->projection_choice,
												XmNmenuHistory,mapping->projection_cylinder,
												NULL);
										}
									} break;
								}	/* switch (type)	*/
								region_item=get_Region_list_item_next(region_item);
							} /* while */
						}

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

int update_map_from_manual_time_update(struct Mapping_window *mapping)
/*******************************************************************************
LAST MODIFIED : 21 April 2004

DESCRIPTION :
Sets recalculate and map->interpolation_type from
map->draw_map_on_manual_time_update, and update the map.
Reset map->interpolation_type to it's initial value.
Note: 3D maps always fully recalculate on manual time updates.
==============================================================================*/
{
#if defined (OLD_CODE)
	enum Interpolation_type interpolation;
	struct Map *map;
#endif /* defined (OLD_CODE) */
	int recalculate,return_code;

	ENTER(update_map_from_manual_time_update);
	return_code=0;
	if (mapping
#if defined (OLD_CODE)
		&&(map=mapping->map)
#endif /* defined (OLD_CODE) */
		)
	{
		return_code=1;
#if defined (OLD_CODE)
		interpolation=map->interpolation_type;
		if ((map->draw_map_on_manual_time_update)||
			(map->projection_type==THREED_PROJECTION))
		{
#endif /* defined (OLD_CODE) */
			recalculate=2;
#if defined (OLD_CODE)
		}
		else
		{
			recalculate=1;
			map->interpolation_type=NO_INTERPOLATION;
		}
#endif /* defined (OLD_CODE) */
		update_mapping_drawing_area(mapping,recalculate);
		update_mapping_colour_or_auxili(mapping);
#if defined (OLD_CODE)
		map->interpolation_type=interpolation;
#endif /* defined (OLD_CODE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"update_map_from_manual_time_update. Invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* update_map_from_manual_time_update */

int highlight_electrode_or_auxiliar(struct Device *device,
#if defined (UNEMAP_USE_NODES)
	struct FE_node *device_node,
#endif
	int electrode_number,	int auxiliary_number,struct Map *map,
	struct Mapping_window *mapping)
/*******************************************************************************
LAST MODIFIED : 20 September 2001

DESCRIPTION :
Highlights/dehighlights an electrode or an auxiliary device in the <mapping>
window.
==============================================================================*/
{
	char electrode_drawn,*device_name,*name,value_string[11];
	Display *display;
	enum Map_type map_type;
	float f_value,max_f,min_f,range_f;
	GC graphics_context;
	int ascent,descent,device_channel_number,device_highlighted,direction,j,
		marker_size,name_length,number_of_spectrum_colours,return_code,xmax,
		xmin,xpos,xstart,ymax,ymin,ypos,ystart;
	Pixel *spectrum_pixels;
	struct Map_drawing_information *drawing_information;
	struct Sub_map *sub_map;
	XCharStruct bounds;
	XFontStruct *font;
#if defined (UNEMAP_USE_NODES)
	struct FE_field *channel_number_field,*device_name_field,*highlight_field;
	struct FE_field_component component;
#endif /* defined (UNEMAP_USE_NODES) */

	ENTER(highlight_electrode_or_auxiliar);
	device_name=(char *)NULL;
	name=(char *)NULL;
#if defined (UNEMAP_USE_NODES)
	channel_number_field=(struct FE_field *)NULL;
	device_name_field=(struct FE_field *)NULL;
	highlight_field=(struct FE_field *)NULL;
#endif /* defined (UNEMAP_USE_NODES) */
	return_code=0;
	if (map&&&mapping&&(drawing_information=map->drawing_information)&&
		(drawing_information->user_interface)&&
#if defined (UNEMAP_USE_NODES)
		((device&&!device_node)||(!device&&device_node)))
#else
		device)
#endif /* defined (UNEMAP_USE_NODES) */
	{
		display=User_interface_get_display(drawing_information->user_interface);
		font=drawing_information->font;
		number_of_spectrum_colours=drawing_information->number_of_spectrum_colours;
		spectrum_pixels=drawing_information->spectrum_colours;
		map_type= *(map->type);
#if defined (UNEMAP_USE_NODES)
		if (device_node)
		{
			device_name_field=get_unemap_package_device_name_field(map->unemap_package);
			highlight_field=get_unemap_package_highlight_field(map->unemap_package);
			channel_number_field=get_unemap_package_channel_number_field(map->unemap_package);
			get_FE_nodal_string_value(device_node,device_name_field,0,0,FE_NODAL_VALUE,
				&device_name);
			component.number=0;
			component.field=highlight_field;
			get_FE_nodal_int_value(device_node,&component,0,FE_NODAL_VALUE,
						/*time*/0,&device_highlighted);
			component.field=channel_number_field;
			get_FE_nodal_int_value(device_node,&component,0,FE_NODAL_VALUE,
						/*time*/0,&device_channel_number);
		}
		else
#endif /* defined (UNEMAP_USE_NODES) */
		{
			device_name=device->description->name;
			device_highlighted=device->highlight;
			/* i.e an auxiliary device*/
			if (device->channel)
			{
				device_channel_number=device->channel->number;
			}
			else
			{
				device_channel_number=0;
			}
		}
		if (map&&(electrode_number>=0)&&(map->electrode_drawn)&&
			((map->electrode_drawn)[electrode_number])&&mapping&&
			(mapping->map_drawing_area_2d)&&(mapping->map_drawing)
			&&map->electrodes_marker_type!=HIDE_ELECTRODE_MARKER)
		{
			/* loop through sub maps */
			for (j=0;j<map->number_of_sub_maps;j++)
			{
				sub_map=map->sub_map[j];

				f_value=(sub_map->electrode_value)[electrode_number];
				switch (map->electrodes_label_type)
				{
					case HIDE_ELECTRODE_LABELS:
					{
						electrode_drawn=1;
						/*do nothing at the moment*/
					} break;
					case SHOW_ELECTRODE_NAMES:
					{
						electrode_drawn=1;
						name=device_name;
					} break;
					case SHOW_CHANNEL_NUMBERS:
					{
						electrode_drawn=1;
						sprintf(value_string,"%d",device_channel_number);
						name=value_string;
					} break;
					case SHOW_ELECTRODE_VALUES:
					{
						electrode_drawn=(map->electrode_drawn)[electrode_number];
						if (electrode_drawn)
						{
							if (HIDE_COLOUR!=map->colour_option)
							{
								sprintf(value_string,"%.4g",f_value);
								name=value_string;
							}
							else
							{
								name=(char *)NULL;
							}
						}
					} break;
					default:
					{
						name=(char *)NULL;
					} break;
				}/* switch */
				if (device_highlighted)
				{
					graphics_context=(drawing_information->graphics_context).
						highlighted_colour;
				}
				else
				{
					if ((map->electrodes_label_type==SHOW_ELECTRODE_VALUES)&&
						(HIDE_COLOUR==map->colour_option)&&
						(SHOW_CONTOURS==map->contours_option))
					{
						graphics_context=(drawing_information->graphics_context).
							unhighlighted_colour;
					}
					else
					{
						min_f=map->minimum_value;
						max_f=map->maximum_value;
						if (map->colour_electrodes_with_signal)
						{
							graphics_context=(drawing_information->graphics_context).spectrum;
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
						else
						{
							graphics_context=(drawing_information->graphics_context).
								unhighlighted_colour;
						}
					}
				}
				if (electrode_drawn)
				{
					/* draw marker */
					xpos=(sub_map->electrode_x)[electrode_number];
					ypos=(sub_map->electrode_y)[electrode_number];
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
						if (device_highlighted)
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
			}/* for (j=0;j<map->number_of_sub_maps;j++)*/
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
			if (device_highlighted)
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
			if (device_name)
			{
				name_length=strlen(device_name);
				XTextExtents(font,device_name,name_length,
					&direction,&ascent,&descent,&bounds);
				xstart=xpos+6-bounds.lbearing;
				ystart=ypos+ascent-4;
				if (device_highlighted)
				{
					XDrawString(display,mapping->colour_or_auxiliary_drawing->pixel_map,
						(drawing_information->graphics_context).highlighted_colour,xstart,
						ystart,device_name,name_length);
				}
				else
				{
					XDrawString(display,mapping->colour_or_auxiliary_drawing->pixel_map,
						(drawing_information->graphics_context).node_text_colour,xstart,
						ystart,device_name,name_length);
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
#if defined (UNEMAP_USE_NODES)
		if (device_name)
		{
			DEALLOCATE(device_name);
		}
#endif /* defined (UNEMAP_USE_NODES) */
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
		if (mapping->potential_time_object!=potential_time_object)
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

int mapping_window_update_time_limits(struct Mapping_window *mapping)
/*******************************************************************************
LAST MODIFIED : 15 January 2003

DESCRIPTION :
Sets the minimum and maximum of the time_keeper to relate to the current map.
==============================================================================*/
{
	int return_code;
	struct Map *map;
	struct Signal_buffer *buffer;

	ENTER(mapping_window_update_time_limits);
	return_code=0;
	if (mapping)
	{
		if (mapping->potential_time_object&&(map=mapping->map)&&
			(*(map->rig_pointer))&&((*(map->rig_pointer))->devices)&&
			((*((*(map->rig_pointer))->devices))->signal)&&
			(buffer=(*((*(map->rig_pointer))->devices))->signal->buffer))
		{
			return_code=1;
			switch (*mapping->map->type)
			{
				case POTENTIAL:
				{
					switch (map->interpolation_type)
					{
						case BICUBIC_INTERPOLATION:
						{
							Time_keeper_set_minimum(Time_object_get_time_keeper(
								mapping->potential_time_object),(map->start_time)/1000.0-
								0.5/(buffer->frequency));
							Time_keeper_set_maximum(Time_object_get_time_keeper(
								mapping->potential_time_object),(map->end_time)/1000.0+
								0.5/(buffer->frequency));
						} break;
						case NO_INTERPOLATION:
						case DIRECT_INTERPOLATION:
						default:
						{
							Time_keeper_set_minimum(Time_object_get_time_keeper(
								mapping->potential_time_object),
								(float)buffer->times[*(map->start_search_interval)]/
								(buffer->frequency));
							Time_keeper_set_maximum(Time_object_get_time_keeper(
								mapping->potential_time_object),
								(float)buffer->times[*(map->end_search_interval)]/
								buffer->frequency);
						} break;
					}/*switch (map->interpolation_type)*/
				} break;
				case SINGLE_ACTIVATION:
				{
#if defined (NEW_CODE)
/*???DB.  Shouldn't start_time, end_time and number_of_frames be updated? */
					if (map->drawing_information)
					{
						map->number_of_frames=
							map->drawing_information->number_of_spectrum_colours;
					}
					else
					{
						map->number_of_frames=1;
						display_message(ERROR_MESSAGE,"mapping_window_update_time_limits.  "
							"Missing map drawing information");
					}
					map->start_time=(map->minimum_value)/1000.0+
						(float)(buffer->times[*(map->datum)])/(buffer->frequency);
					map->end_time=(map->maximum_value)/1000.0+
						(float)(buffer->times[*(map->datum)])/(buffer->frequency);
					Time_keeper_set_minimum(Time_object_get_time_keeper(
						mapping->potential_time_object),map->start_time);
					Time_keeper_set_maximum(Time_object_get_time_keeper(
						mapping->potential_time_object),map->end_time);
#else /* defined (NEW_CODE) */
					Time_keeper_set_minimum(Time_object_get_time_keeper(
						mapping->potential_time_object),(map->minimum_value)/1000.0+
						((float)(buffer->times)[*(map->datum)]-0.5)/(buffer->frequency));
					Time_keeper_set_maximum(Time_object_get_time_keeper(
						mapping->potential_time_object),(map->maximum_value)/1000.0+
						((float)(buffer->times)[*(map->datum)]+0.5)/(buffer->frequency));
#endif /* defined (NEW_CODE) */
				} break;
				default:
				{
					Time_keeper_set_minimum(Time_object_get_time_keeper(
						mapping->potential_time_object),
						(float)buffer->times[*(map->start_search_interval)]/
						(buffer->frequency));
					Time_keeper_set_maximum(Time_object_get_time_keeper(
						mapping->potential_time_object),
						(float)buffer->times[*(map->end_search_interval)]/
						buffer->frequency);
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"mapping_window_update_time_limits.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* mapping_window_update_time_limits */


