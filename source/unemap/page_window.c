/*******************************************************************************
FILE : page_window.c

LAST MODIFIED : 9 May 2002

DESCRIPTION :

CODE SWITCHS :
MOTIF - X/Motif analysis only version (does nothing)
WINDOWS - win32 acquisition only version
	MIRADA - first Oxford interim system, based on Mirada card
	!MIRADA - uses National Instruments PCI or PXI data acquisition cards
		???DB.  Incorporate MIRADA in unemap_hardware ?

???DB.  General design of tools
- have a create_tool that is passed a parent and returns a tool structure
- have an open_tool that is passed the address of a pointer to a tool structure.
	If the point is not NULL then it is displayed/brought to the front/opened
	(assumes that the parent is a shell).  Otherwise a tool and a shell are
	created and it displayed/brought to the front/opened.

TO DO:
???DB.  6 May 2002.  Adding "background" and "ratio" operations for optrodes
1 How do raw signals get transformed currently
1.1 Offset and gain.  gain*(raw-offset)
1.2 Linear combination.  Calculated each time required from raw signals.  Used
		in:
1.2.1 extract_signal_information in rig_node
1.2.2 add_scrolling_device in page_window.c
1.2.3 start_experiment in page_window.c
1.2.4 page_auto_range in page_window.c
1.2.5 page_full_range in page_window.c
1.2.6 create_Page_window in page_window.c
1.2.7 create_processed_rig in analysis_work_area.c
1.2.8 create_Device_description in rig.c
1.2.9 destroy_Device_description in rig.c
1.2.10 read_configuration in rig.c
1.2.11 write_configuration in rig.c
2 Theory
2.1 Measure at two frequencies of light
2.2 Assume that the form of the intensities is
		I1(t)=(a1*Vm(t)+b1)*Decay(t)
		I2(t)=(a2*Vm(t)+b2)*Decay(t)
		Where Vm is the membrane voltage and Decay is related to bleaching of the
		dye etc
2.3 Assume that a1<<b1 and a2<<b2 then
		ratio(t)=I1(t)/I2(t)=(b1/b2)*(1+(a1/b1)*Vm(t))/(1+(a2/b2)*Vm(t))
			~=(b1/b2)*(1+(a1/b1)*Vm(t))*(1-(a2/b2)*Vm(t))
			~=(b1/b2)*(1+(a1/b1-a2/b2)*Vm(t))
3 Background
3.1 Method
3.1.1 Acquire an interval
3.1.2 Calculate the average value for each channel
3.2 Implementation
4 Ratio
4.1 Method
4.1.1 Calculate the average of the laser monitor signal
4.1.2 Calculate
			(channel signal)-(laser monitor signal)*(background average for channel)/
				(average for laser monitor signal)
4.1.2.1 The subtraction should remove the laser noise and get the signal in the
				form of 2.2 (get the aymptote to 0).  So it should be
				(channel signal)-background average for channel-
					noise_factor*(laser monitor signal)
				where is the noise_factor is
					rms((channel signal)-moving_average(channel signal))/
						rms(laser monitor signal)
				but, the signals aren't decaying to the background value.  Fit an
				exponential?
4.1.3 Calculate ratios of channel signals
4.2 Implementation
==============================================================================*/

#define BACKGROUND_SAVING

#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#if defined (BACKGROUND_SAVING) && defined (MOTIF)
#include <pthread.h>
#endif /* defined (BACKGROUND_SAVING) && defined (MOTIF) */
#if defined (MOTIF)
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Composite.h>
#include <Xm/Xm.h>
#include <Xm/ArrowB.h>
#include <Xm/MwmUtil.h>
#include <Xm/ToggleB.h>
#include <Mrm/MrmPublic.h>
#include <Mrm/MrmDecls.h>
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
#include <windows.h>
#include <commctrl.h>
#endif /* defined (WINDOWS) */
#include "general/debug.h"
#include "unemap/pacing_window.h"
#include "unemap/page_window.h"
#if defined (MOTIF)
#include "unemap/page_window.uidh"
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
#include "unemap/page_window.rc"
#endif /* defined (WINDOWS) */
#include "unemap/rig.h"
#include "unemap/unemap_hardware.h"
#include "user_interface/confirmation.h"
#include "user_interface/filedir.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#if defined (WINDOWS)
#if defined (MIRADA)
#include "unemap/vunemapd.h"
#endif /* defined (MIRADA) */
#endif /* defined (WINDOWS) */

/*
Module constants
----------------
*/
#define ARROWS_WIDTH 12

/*
Module types
------------
*/
struct Stimulator
/*******************************************************************************
LAST MODIFIED : 7 January 2001

DESCRIPTION :
==============================================================================*/
{
	float end_values_per_second,*end_values;
	int constant_voltage,end_number_of_values,number_of_channels,on,
		*channel_numbers;
}; /* struct Stimulator */

struct Page_window
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
The page window object.
==============================================================================*/
{
#if defined (MOTIF)
	Widget activation,shell,window;
	Widget auto_range_button,calibrate_button,close_button,experiment_checkbox,
		full_range_button,isolate_checkbox,pacing_button,sample_checkbox,
		scrolling_checkbox,start_all_stimulators_button,stimulate_checkbox,
		stimulator_checkbox,stop_all_stimulators_button,test_checkbox;
	struct
	{
		Widget form,value;
	} electrode;
	struct
	{
		Widget form,value;
	} gain;
	struct
	{
		Widget form,value;
	} low_pass;
	struct
	{
		Widget form,value;
	} maximum;
	struct
	{
		Widget form,value;
	} minimum;
	struct
	{
		Widget button,form,value;
	} save;
	Widget scrolling_area;
	/* graphics contexts */
	struct
	{
		GC background_drawing_colour,foreground_drawing_colour;
	} graphics_context;
#if defined (OLD_CODE)
	Widget activation,shell,window;
	Widget calibrate_button,close_button,experiment_checkbox,isolate_checkbox,
		reset_scale_button,sample_checkbox,save_button,scale_button,test_checkbox;
	struct
	{
		Widget form,value;
	} electrode;
	struct
	{
		Widget form,value;
	} low_pass;
	struct
	{
		Widget form,value;
	} maximum;
	struct
	{
		Widget form,value;
	} minimum;
	struct
	{
		Widget form,value;
	} gain;
	struct
	{
		Widget form,label;
		struct
		{
			Widget arrows,form,checkbox,value;
		} stimulate;
	} stimulator;
	Widget drawing_area;
	/* graphics contexts */
	struct
	{
		GC background_drawing_colour,foreground_drawing_colour;
	} graphics_context;
#endif /* defined (OLD_CODE) */
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	HINSTANCE instance;
	HWND window;
	HWND auto_range_button;
	int auto_range_button_height,auto_range_button_width;
	HWND calibrate_button;
	int calibrate_button_height,calibrate_button_width;
	HWND close_button;
	int close_button_height,close_button_width;
	struct
	{
		HWND arrows,edit,text;
		int arrows_height,arrows_width,edit_height,edit_width,text_height,
			text_width;
	} electrode;
	HWND experiment_checkbox;
	int experiment_checkbox_height,experiment_checkbox_width;
	HWND full_range_button;
	int full_range_button_height,full_range_button_width;
	struct
	{
		HWND edit,text;
		int edit_height,edit_width,text_height,text_width;
	} gain;
	HWND isolate_checkbox;
	int isolate_checkbox_height,isolate_checkbox_width;
	struct
	{
		HWND edit,text;
		int edit_height,edit_width,text_height,text_width;
	} low_pass_filter;
	struct
	{
		HWND edit,text;
		int edit_height,edit_width,text_height,text_width;
	} maximum;
	struct
	{
		HWND edit,text;
		int edit_height,edit_width,text_height,text_width;
	} minimum;
	HWND pacing_button;
	int pacing_button_height,pacing_button_width;
	HWND sample_checkbox;
	int sample_checkbox_height,sample_checkbox_width;
	struct
	{
		HWND button,edit;
		int button_height,button_width,edit_height,edit_width;
	} save;
	HWND scrolling_checkbox;
	int scrolling_checkbox_height,scrolling_checkbox_width;
	HWND start_all_stimulators_button;
	int start_all_stimulators_button_height,start_all_stimulators_button_width;
	HWND stimulate_checkbox;
	int stimulate_checkbox_height,stimulate_checkbox_width;
	HWND stimulator_checkbox;
	int stimulator_checkbox_height,stimulator_checkbox_width;
	HWND stop_all_stimulators_button;
	int stop_all_stimulators_button_height,stop_all_stimulators_button_width;
	HWND test_checkbox;
	int test_checkbox_height,test_checkbox_width;
	int all_devices_menu_width,control_menu_width,current_device_menu_width;
	HWND scrolling_area;
	HBRUSH fill_brush;
#if defined (OLD_CODE)
	HINSTANCE instance;
	HWND window;
	HWND save_button;
	int save_button_height,save_button_width;
	HWND experiment_checkbox;
	int experiment_checkbox_height,experiment_checkbox_width;
	HWND isolate_checkbox;
	int isolate_checkbox_height,isolate_checkbox_width;
	HWND sample_checkbox;
	int sample_checkbox_height,sample_checkbox_width;
	HWND calibrate_button;
	int calibrate_button_height,calibrate_button_width;
	HWND test_checkbox;
	int test_checkbox_height,test_checkbox_width;
	struct
	{
		HWND arrows,edit,text;
		int arrows_height,arrows_width,edit_height,edit_width,text_height,
			text_width;
	} channel;
	HWND exit_button;
	int exit_button_height,exit_button_width;
	struct
	{
		HWND edit,text;
		int edit_height,edit_width,text_height,text_width;
	} low_pass_filter;
	struct
	{
		HWND edit,text;
		int edit_height,edit_width,text_height,text_width;
	} maximum;
	struct
	{
		HWND edit,text;
		int edit_height,edit_width,text_height,text_width;
	} minimum;
	struct
	{
		HWND edit,text;
		int edit_height,edit_width,text_height,text_width;
	} gain;
	HWND reset_scale_button;
	int reset_scale_button_height,reset_scale_button_width;
	HWND scale_button;
	int scale_button_height,scale_button_width;
	struct
	{
		HWND arrows,text;
		int arrows_height,arrows_width,text_height,text_width;
	} stimulator;
	HWND stimulate_checkbox;
	int stimulate_checkbox_height,stimulate_checkbox_width;
	struct
	{
		HWND arrows,edit;
		int arrows_height,arrows_width,edit_height,edit_width;
	} stimulate_channel;
	int control_menu_width,current_device_menu_width;
	HWND drawing_area;
	HBRUSH fill_brush;
#endif /* defined (OLD_CODE) */
#if defined (MIRADA)
	HANDLE device_driver;
	short int *mirada_buffer;
#endif /* defined (MIRADA) */
#endif /* defined (WINDOWS) */
	char *calibration_directory;
	int unemap_hardware_version;
	float initial_gain,sampling_frequency,*scrolling_coefficients;
#if defined (OLD_CODE)
	float display_maximum,display_minimum,signal_maximum,signal_minimum;
#endif /* defined (OLD_CODE) */
	int data_saved,display_device_number,hardware_initialized,
		*last_scrolling_value_device,number_of_scrolling_channels,
		*number_of_scrolling_channels_device,number_of_scrolling_devices,
		number_of_stimulators,pointer_sensitivity,
#if defined (OLD_CODE)
		*number_of_stimulating_channels,**stimulating_channel_numbers,
		*stimulator_on,
#endif /* defined (OLD_CODE) */
#if defined (OLD_CODE)
		*stimulate_device_numbers,stimulator_number,
#endif /* defined (OLD_CODE) */
		synchronization_card;
	struct Channel **scrolling_channels;
	struct Device *display_device,**scrolling_devices;
#if defined (OLD_CODE)
	struct Device **stimulate_devices;
#endif /* defined (OLD_CODE) */
	struct File_open_data *save_file_open_data;
	struct Pacing_window *pacing_window;
	struct Page_window **address;
	struct Mapping_window **mapping_window_address;
	struct Rig **rig_address;
	struct Stimulator *stimulators;
	struct User_interface *user_interface;
	unsigned long number_of_samples,number_of_samples_to_save;
#if defined (MIRADA)
	unsigned long number_of_channels;
#else /* defined (MIRADA) */
	int number_of_channels;
#endif /* defined (MIRADA) */
}; /* struct Page_window */

typedef struct
{
	char *calibration_directory;
	float initial_gain,sampling_frequency;
	int number_of_samples,synchronization_card;
#if defined (MOTIF)
	Boolean restitution_time_pacing;
	Pixel background_colour,foreground_colour;
#endif /* defined (MOTIF) */
} Page_window_settings;

struct Save_write_signal_file_background_data
{
	char *file_name,*temp_file_name;
	int all_channels,number_of_samples;
	short *samples;
	struct Page_window *page_window;
}; /* struct Save_write_signal_file_background_data */

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int page_window_hierarchy_open=0;
static MrmHierarchy page_window_hierarchy;
#endif /* defined (MOTIF) */

/* dependent on whether the card is 12-bit or 16-bit */
long int maximum_signal_value=1,minimum_signal_value=0;

/*
Module functions
----------------
*/
#if defined (MOTIF)
static void identify_page_auto_range_button(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 October 2000

DESCRIPTION :
Finds the id of the page experiment checkbox.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_auto_range_button);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		page->auto_range_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_auto_range_button.  page window missing");
	}
	LEAVE;
} /* identify_page_auto_range_button */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_calibrate_button(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Finds the id of the page calibrate button.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_calibrate_button);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		page->calibrate_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_calibrate_button.  page window missing");
	}
	LEAVE;
} /* identify_page_calibrate_button */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_close_button(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Finds the id of the page close button.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_close_button);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		page->close_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_close_button.  page window missing");
	}
	LEAVE;
} /* identify_page_close_button */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_electrode_form(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Finds the id of the page electrode form widget.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_electrode_form);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		(page->electrode).form= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_electrode_form.  page window missing");
	}
	LEAVE;
} /* identify_page_electrode_form */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_electrode_value(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Finds the id of the page electrode value text widget.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_electrode_value);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		(page->electrode).value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_electrode_value.  page window missing");
	}
	LEAVE;
} /* identify_page_electrode_value */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_experiment_checkb(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Finds the id of the page experiment checkbox.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_experiment_checkb);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		page->experiment_checkbox= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_experiment_checkb.  page window missing");
	}
	LEAVE;
} /* identify_page_experiment_checkb */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_full_range_button(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 October 2000

DESCRIPTION :
Finds the id of the page experiment checkbox.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_full_range_button);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		page->full_range_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_full_range_button.  page window missing");
	}
	LEAVE;
} /* identify_page_full_range_button */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_gain_form(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Finds the id of the page gain form widget.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_gain_form);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		(page->gain).form= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_gain_form.  page window missing");
	}
	LEAVE;
} /* identify_page_gain_form */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_gain_value(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Finds the id of the page gain value text widget.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_gain_value);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		(page->gain).value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_gain_value.  page window missing");
	}
	LEAVE;
} /* identify_page_gain_value */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_isolate_checkbox(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Finds the id of the page isolate checkbox.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_isolate_checkbox);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		page->isolate_checkbox= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_isolate_checkbox.  page window missing");
	}
	LEAVE;
} /* identify_page_isolate_checkbox */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_low_pass_form(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Finds the id of the page low pass form widget.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_low_pass_form);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		(page->low_pass).form= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_low_pass_form.  page window missing");
	}
	LEAVE;
} /* identify_page_low_pass_form */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_low_pass_value(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Finds the id of the page low pass value text widget.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_low_pass_value);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		(page->low_pass).value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_low_pass_value.  page window missing");
	}
	LEAVE;
} /* identify_page_low_pass_value */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_maximum_form(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Finds the id of the page maximum form widget.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_maximum_form);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		(page->maximum).form= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_maximum_form.  page window missing");
	}
	LEAVE;
} /* identify_page_maximum_form */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_maximum_value(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Finds the id of the page maximum value text widget.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_maximum_value);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		(page->maximum).value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_maximum_value.  page window missing");
	}
	LEAVE;
} /* identify_page_maximum_value */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_minimum_form(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Finds the id of the page minimum form widget.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_minimum_form);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		(page->minimum).form= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_minimum_form.  page window missing");
	}
	LEAVE;
} /* identify_page_minimum_form */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_minimum_value(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Finds the id of the page minimum value text widget.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_minimum_value);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		(page->minimum).value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_minimum_value.  page window missing");
	}
	LEAVE;
} /* identify_page_minimum_value */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_pacing_button(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 27 September 1999

DESCRIPTION :
Finds the id of the page test checkbox.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_pacing_button);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		page->pacing_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_pacing_button.  page window missing");
	}
	LEAVE;
} /* identify_page_pacing_button */
#endif /* defined (MOTIF) */

#if defined (OLD_CODE)
#if defined (MOTIF)
static void identify_page_reset_scale_butto(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Finds the id of the page reset scale button.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_reset_scale_butto);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		page->reset_scale_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_reset_scale_butto.  page window missing");
	}
	LEAVE;
} /* identify_page_reset_scale_butto */
#endif /* defined (MOTIF) */
#endif /* defined (OLD_CODE) */

#if defined (MOTIF)
static void identify_page_sample_checkbox(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Finds the id of the page sample checkbox.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_sample_checkbox);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		page->sample_checkbox= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_sample_checkbox.  page window missing");
	}
	LEAVE;
} /* identify_page_sample_checkbox */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_save_button(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Finds the id of the page save button.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_save_button);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		(page->save).button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_save_button.  page window missing");
	}
	LEAVE;
} /* identify_page_save_button */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_save_form(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Finds the id of the page save form.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_save_form);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		(page->save).form= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_save_form.  page window missing");
	}
	LEAVE;
} /* identify_page_save_form */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_save_number_of_sa(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Finds the id of the page save number of samples text widget.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_save_number_of_sa);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		(page->save).value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_save_number_of_sa.  page window missing");
	}
	LEAVE;
} /* identify_page_save_number_of_sa */
#endif /* defined (MOTIF) */

#if defined (OLD_CODE)
#if defined (MOTIF)
static void identify_page_scale_button(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Finds the id of the page scale button.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_scale_button);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		page->scale_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_scale_button.  page window missing");
	}
	LEAVE;
} /* identify_page_scale_button */
#endif /* defined (MOTIF) */
#endif /* defined (OLD_CODE) */

#if defined (MOTIF)
static void identify_page_scrolling_area(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Finds the id of the page scrolling area.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_scrolling_area);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		page->scrolling_area= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_scrolling_area.  page window missing");
	}
	LEAVE;
} /* identify_page_scrolling_area */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_scrolling_checkbo(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 October 2000

DESCRIPTION :
Finds the id of the page scrolling checkbox.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_scrolling_checkbo);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		page->scrolling_checkbox= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_scrolling_checkbo.  page window missing");
	}
	LEAVE;
} /* identify_page_scrolling_checkbo */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_start_all_stimula(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 October 2000

DESCRIPTION :
Finds the id of the page start all stimulators button.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_start_all_stimula);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		page->start_all_stimulators_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_start_all_stimula.  page window missing");
	}
	LEAVE;
} /* identify_page_start_all_stimula */
#endif /* defined (MOTIF) */

#if defined (OLD_CODE)
#if defined (MOTIF)
static void identify_page_stimulate_arrows(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Finds the id of the page stimulate arrows widget.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_stimulate_arrows);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		(page->stimulator).stimulate.arrows= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_stimulate_arrows.  page window missing");
	}
	LEAVE;
} /* identify_page_stimulate_arrows */
#endif /* defined (MOTIF) */
#endif /* defined (OLD_CODE) */

#if defined (MOTIF)
static void identify_page_stimulate_checkbo(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 October 2000

DESCRIPTION :
Finds the id of the page stimulate checkbox widget.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_stimulate_checkbo);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		page->stimulate_checkbox= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_stimulate_checkbo.  page window missing");
	}
	LEAVE;
} /* identify_page_stimulate_checkbo */
#endif /* defined (MOTIF) */

#if defined (OLD_CODE)
#if defined (MOTIF)
static void identify_page_stimulate_form(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Finds the id of the page stimulate form widget.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_stimulate_form);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		(page->stimulator).stimulate.form= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_stimulate_form.  page window missing");
	}
	LEAVE;
} /* identify_page_stimulate_form */
#endif /* defined (MOTIF) */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
#if defined (MOTIF)
static void identify_page_stimulate_value(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Finds the id of the page stimulate value text widget.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_stimulate_value);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		(page->stimulator).stimulate.value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_stimulate_value.  page window missing");
	}
	LEAVE;
} /* identify_page_stimulate_value */
#endif /* defined (MOTIF) */
#endif /* defined (OLD_CODE) */

#if defined (MOTIF)
static void identify_page_stimulator_checkb(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 October 2000

DESCRIPTION :
Finds the id of the page stimulator checkbox widget.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_stimulator_checkb);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		page->stimulator_checkbox= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_stimulator_checkb.  page window missing");
	}
	LEAVE;
} /* identify_page_stimulator_checkb */
#endif /* defined (MOTIF) */

#if defined (OLD_CODE)
#if defined (MOTIF)
static void identify_page_stimulator_form(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Finds the id of the page stimulator form widget.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_stimulator_form);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		(page->stimulator).form= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_stimulator_form.  page window missing");
	}
	LEAVE;
} /* identify_page_stimulator_form */
#endif /* defined (MOTIF) */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
#if defined (MOTIF)
static void identify_page_stimulator_label(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Finds the id of the page stimulator label widget.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_stimulator_label);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		(page->stimulator).label= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_stimulator_label.  page window missing");
	}
	LEAVE;
} /* identify_page_stimulator_label */
#endif /* defined (MOTIF) */
#endif /* defined (OLD_CODE) */

#if defined (MOTIF)
static void identify_page_stop_all_stimulat(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 October 2000

DESCRIPTION :
Finds the id of the page stop all stimulators button.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_stop_all_stimulat);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		page->stop_all_stimulators_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_stop_all_stimulat.  page window missing");
	}
	LEAVE;
} /* identify_page_stop_all_stimulat */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_test_checkbox(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 27 September 1999

DESCRIPTION :
Finds the id of the page test checkbox.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_test_checkbox);
	USE_PARAMETER(call_data);
	if (page=(struct Page_window *)page_window_structure)
	{
		page->test_checkbox= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_test_checkbox.  page window missing");
	}
	LEAVE;
} /* identify_page_test_checkbox */
#endif /* defined (MOTIF) */

#define SCROLLING_BORDER_WIDTH 40
#define SCROLLING_BORDER_HEIGHT 5

#if defined (MOTIF)
static int fill_left,fill_width;
static XPoint scroll_line[2],signal_line[5],x_axis[2];
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
/*???DB.  Created in WM_CREATE of Page_window_scrolling_area_class_proc */
/*???DB.  Created in WM_PAINT of Page_window_scrolling_area_class_proc */
static POINT scroll_line[2],signal_line[5],x_axis[2];
static RECT fill_rectangle;
#endif /* defined (WINDOWS) */

static int draw_scrolling_background(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 8 November 2000

DESCRIPTION :
Clears the display and draws the axes for the scrolling area.
???DB.  Needs changing to draw all the scrolling devices rather than just the
current device
==============================================================================*/
{
	char *string,working_string[21];
	int height,i,length,number_of_scrolling_devices,return_code,x_offset,y_offset,
		width;
#if defined (MOTIF)
	Display *display;
	Drawable drawable;
	int ascent,descent,direction;
	XCharStruct bounds;
	XFontStruct *font;
	XWindowAttributes attributes;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	HDC device_context;
	HWND scrolling_area;
	SIZE bounds;
#endif /* defined (WINDOWS) */

	ENTER(draw_scrolling_background);
	return_code=0;
	if (page_window)
	{
		return_code=1;
		/* set up */
#if defined (MOTIF)
		display=page_window->user_interface->display;
		drawable=XtWindow(page_window->scrolling_area);
		XGetWindowAttributes(display,drawable,&attributes);
		width=attributes.width;
		height=attributes.height;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		scrolling_area=page_window->scrolling_area;
		device_context=GetDC(scrolling_area);
		GetClientRect(scrolling_area,&fill_rectangle);
		width=(fill_rectangle.right)+1;
		height=(fill_rectangle.bottom)+1;
#endif /* defined (WINDOWS) */
		/* clear the window */
#if defined (MOTIF)
		XFillRectangle(display,drawable,
			(page_window->graphics_context).background_drawing_colour,0,0,width,
			height);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		FillRect(device_context,&fill_rectangle,page_window->fill_brush);
			/*???DB.  Seems to return a nonzero on success (not TRUE) */
#endif /* defined (WINDOWS) */
		/* draw the x tick marks */
		scroll_line[1].y=height;
		height -= SCROLLING_BORDER_HEIGHT;
		scroll_line[0].y=height;
		width -= SCROLLING_BORDER_WIDTH;
		for (i=100;i<width;i += 100)
		{
			scroll_line[0].x=i;
			scroll_line[1].x=i;
#if defined (MOTIF)
			XDrawLines(display,drawable,
				(page_window->graphics_context).foreground_drawing_colour,scroll_line,2,
				CoordModeOrigin);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			Polyline(device_context,scroll_line,2);
#endif /* defined (WINDOWS) */
		}
		/* draw the x axis */
		x_axis[0].x=0;
		x_axis[0].y=(short)0;
#if defined (OLD_CODE)
		x_axis[0].y=(short)(((page_window->display_maximum)*(float)height)/
			((page_window->display_maximum)-(page_window->display_minimum)));
#endif /* defined (OLD_CODE) */
		x_axis[1].x=width;
		x_axis[1].y=x_axis[0].y;
#if defined (OLD_CODE)
#if defined (MOTIF)
		XDrawLines(display,drawable,
			(page_window->graphics_context).foreground_drawing_colour,x_axis,2,
			CoordModeOrigin);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		Polyline(device_context,x_axis,2);
#endif /* defined (WINDOWS) */
#endif /* defined (OLD_CODE) */
		/* set up the scrolling cursor */
#if defined (MOTIF)
		fill_left=1;
		fill_width=5;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		fill_rectangle.left=1;
		fill_rectangle.right=5;
		fill_rectangle.bottom -= 4;
#endif /* defined (WINDOWS) */
		x_axis[1].x=4;
		scroll_line[0].x=5;
		scroll_line[0].y=0;
		scroll_line[1].x=5;
		scroll_line[1].y=height;
		if (0<(number_of_scrolling_devices=page_window->
			number_of_scrolling_devices))
		{
			/* write the device names */
			height /= number_of_scrolling_devices;
#if defined (MOTIF)
			font=XQueryFont(display,XGContextFromGC(
				(page_window->graphics_context).foreground_drawing_colour));
#endif /* defined (MOTIF) */
			for (i=0;i<number_of_scrolling_devices;i++)
			{
				/* name */
				string=((page_window->scrolling_devices)[i])->description->name;
				length=strlen(string);
#if defined (MOTIF)
				XTextExtents(font,string,length,&direction,&ascent,&descent,&bounds);
				x_offset=bounds.lbearing+1;
				y_offset=(ascent+1-descent)/2;
				XDrawString(display,drawable,
					(page_window->graphics_context).foreground_drawing_colour,
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				GetTextExtentPoint32(device_context,string,length,&bounds);
				x_offset=0;
				y_offset= -(int)(bounds.cy)/2;
				TextOut(device_context,
#endif /* defined (WINDOWS) */
					width+x_offset,(number_of_scrolling_devices-i)*height-height/2+
					y_offset,string,length);
				/* minimum */
				sprintf(working_string,"%g",((page_window->scrolling_devices)[i])->
					signal_display_minimum);
				length=strlen(working_string);
#if defined (MOTIF)
				XTextExtents(font,working_string,length,&direction,&ascent,&descent,
					&bounds);
				x_offset=bounds.lbearing+1;
				y_offset= -descent;
				XDrawString(display,drawable,
					(page_window->graphics_context).foreground_drawing_colour,
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				GetTextExtentPoint32(device_context,working_string,length,&bounds);
				x_offset=0;
				y_offset= -(int)(bounds.cy);
				TextOut(device_context,
#endif /* defined (WINDOWS) */
					width+x_offset,(number_of_scrolling_devices-i)*height+y_offset,
					working_string,length);
				/* maximum */
				sprintf(working_string,"%g",((page_window->scrolling_devices)[i])->
					signal_display_maximum);
				length=strlen(working_string);
#if defined (MOTIF)
				XTextExtents(font,working_string,length,&direction,&ascent,&descent,
					&bounds);
				x_offset=bounds.lbearing+1;
				y_offset=ascent+1;
				XDrawString(display,drawable,
					(page_window->graphics_context).foreground_drawing_colour,
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				GetTextExtentPoint32(device_context,working_string,length,&bounds);
				x_offset=0;
				y_offset=0;
				TextOut(device_context,
#endif /* defined (WINDOWS) */
					width+x_offset,(number_of_scrolling_devices-i-1)*height+y_offset,
					working_string,length);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_scrolling_background.  Missing page_window");
	}
	LEAVE;

	return (return_code);
} /* draw_scrolling_background */

static void scrolling_hardware_callback(int number_of_channels,
	int *channel_numbers,int number_of_values_per_channel,short *signal_values,
	void *page_window_void)
/*******************************************************************************
LAST MODIFIED : 8 November 2000

DESCRIPTION :
???DB.  Used to be in WM_USER.
==============================================================================*/
{
	float device_maximum,device_minimum,gain,offset,post_filter_gain,
		pre_filter_gain,signal_value,temp_float[4];
	int i,j,j_start,k,height,number_of_channels_device,
		number_of_scrolling_devices,width;
	short int y_offset;
	struct Page_window *page_window;
#if defined (MOTIF)
	Display *display;
	Drawable drawable;
	XWindowAttributes attributes;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	HDC device_context;
	HWND window;
	RECT drawing_rectangle;
#endif /* defined (WINDOWS) */

	ENTER(scrolling_hardware_callback);
	if ((page_window=(struct Page_window *)page_window_void)&&
		(page_window->display_device)&&
		(page_window->number_of_scrolling_channels==number_of_channels)&&
		signal_values&&(4==number_of_values_per_channel))
	{
		/* set up */
#if defined (MOTIF)
		display=page_window->user_interface->display;
		drawable=XtWindow(page_window->scrolling_area);
		XGetWindowAttributes(display,drawable,&attributes);
		width=attributes.width;
		height=attributes.height;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		window=page_window->scrolling_area;
		device_context=GetDC(window);
		GetClientRect(window,&drawing_rectangle);
			/*???DB.  Seems to return a nonzero on success (not TRUE) */
		height=drawing_rectangle.bottom;
		width=drawing_rectangle.right;
#endif /* defined (WINDOWS) */
		height -= SCROLLING_BORDER_HEIGHT;
		width -= SCROLLING_BORDER_WIDTH;
		signal_line[4].x=x_axis[0].x;
		signal_line[3].x=(x_axis[0].x)+1;
		signal_line[2].x=(x_axis[0].x)+2;
		signal_line[1].x=(x_axis[0].x)+3;
		signal_line[0].x=(x_axis[0].x)+4;
#if defined (MOTIF)
		XFillRectangle(display,drawable,
			(page_window->graphics_context).background_drawing_colour,fill_left,0,
			fill_width,height+1);
#if defined (OLD_CODE)
		XDrawLines(display,drawable,
			(page_window->graphics_context).foreground_drawing_colour,x_axis,2,
			CoordModeOrigin);
#endif /* defined (OLD_CODE) */
		fill_left += 4;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		FillRect(device_context,&fill_rectangle,page_window->fill_brush);
#if defined (OLD_CODE)
		Polyline(device_context,x_axis,2);
#endif /* defined (OLD_CODE) */
		fill_rectangle.left += 4;
		fill_rectangle.right += 4;
#endif /* defined (WINDOWS) */
		j_start=0;
		number_of_scrolling_devices=page_window->number_of_scrolling_devices;
		height /= number_of_scrolling_devices;
		for (k=0;k<number_of_scrolling_devices;k++)
		{
			number_of_channels_device=
				(page_window->number_of_scrolling_channels_device)[k];
			device_minimum=(page_window->scrolling_devices)[k]->signal_display_minimum;
			device_maximum=(page_window->scrolling_devices)[k]->signal_display_maximum;
			y_offset=(short)((number_of_scrolling_devices-k-1)*height);
			temp_float[0]=(float)0;
			temp_float[1]=(float)0;
			temp_float[2]=(float)0;
			temp_float[3]=(float)0;
			j=j_start;
			for (i=0;i<number_of_channels_device;i++)
			{
				unemap_get_gain(((page_window->scrolling_devices)[i])->channel->number,
					&pre_filter_gain,&post_filter_gain);
				gain=(page_window->scrolling_coefficients)[i]*
					(((page_window->scrolling_devices)[i])->channel->gain_correction)/
					(pre_filter_gain*post_filter_gain);
				offset=((page_window->scrolling_devices)[i])->channel->offset;
				temp_float[0] += gain*((float)signal_values[j]-offset);
				j++;
				temp_float[1] += gain*((float)signal_values[j]-offset);
				j++;
				temp_float[2] += gain*((float)signal_values[j]-offset);
				j++;
				temp_float[3] += gain*((float)signal_values[j]-offset);
				j++;
			}
			j_start += 4*number_of_channels_device;
			signal_line[4].y=(page_window->last_scrolling_value_device)[k];
			for (i=0;i<4;i++)
			{
				signal_value=temp_float[3-i];
#if defined (OLD_CODE)
				if (page_window->signal_display_maximum<page_window->signal_display_minimum)
				{
					page_window->signal_display_maximum=signal_value;
					page_window->signal_display_minimum=signal_value;
				}
				else
				{
					if (signal_value<page_window->signal_display_minimum)
					{
						page_window->signal_display_minimum=signal_value;
					}
					else
					{
						if (signal_value>page_window->signal_display_maximum)
						{
							page_window->signal_display_maximum=signal_value;
						}
					}
				}
#endif /* defined (OLD_CODE) */
				signal_line[i].y=y_offset+
					(short)(((device_maximum-signal_value)*(float)height)/
					(device_maximum-device_minimum));
			}
			(page_window->last_scrolling_value_device)[k]=signal_line[0].y;
			if (x_axis[0].x>0)
			{
#if defined (MOTIF)
				XDrawLines(display,drawable,
					(page_window->graphics_context).foreground_drawing_colour,signal_line,
					5,CoordModeOrigin);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				Polyline(device_context,signal_line,5);
#endif /* defined (WINDOWS) */
			}
			else
			{
#if defined (MOTIF)
				XDrawLines(display,drawable,
					(page_window->graphics_context).foreground_drawing_colour,signal_line,
					4,CoordModeOrigin);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				Polyline(device_context,signal_line,4);
#endif /* defined (WINDOWS) */
			}
		}
		x_axis[0].x += 4;
		x_axis[1].x += 4;
		if (x_axis[1].x>width)
		{
#if defined (MOTIF)
			fill_left=1;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			fill_rectangle.left=1;
			fill_rectangle.right=5;
#endif /* defined (WINDOWS) */
			x_axis[0].x=0;
			x_axis[1].x=4;
			scroll_line[0].x=5;
			scroll_line[1].x=5;
		}
		else
		{
#if defined (MOTIF)
			XDrawLines(display,drawable,
				(page_window->graphics_context).foreground_drawing_colour,scroll_line,2,
				CoordModeOrigin);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			Polyline(device_context,scroll_line,2);
#endif /* defined (WINDOWS) */
			scroll_line[0].x += 4;
			scroll_line[1].x += 4;
		}
	}
	if (channel_numbers)
	{
		DEALLOCATE(channel_numbers);
	}
	if (signal_values)
	{
		DEALLOCATE(signal_values);
	}
	LEAVE;
} /* scrolling_hardware_callback */

#if defined (OLD_CODE)
static void scrolling_hardware_callback(int number_of_channels,
	int *channel_numbers,int number_of_values_per_channel,short *signal_values,
	void *page_window_void)
/*******************************************************************************
LAST MODIFIED : 5 November 2000

DESCRIPTION :
???DB.  Used to be in WM_USER.
==============================================================================*/
{
	float gain,offset,post_filter_gain,pre_filter_gain,signal_value,temp_float[4];
	int i,j,height,width;
	struct Page_window *page_window;
#if defined (MOTIF)
	Display *display;
	Drawable drawable;
	XWindowAttributes attributes;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	HDC device_context;
	HWND window;
	RECT drawing_rectangle;
#endif /* defined (WINDOWS) */

	ENTER(scrolling_hardware_callback);
	if ((page_window=(struct Page_window *)page_window_void)&&
		(page_window->display_device)&&
		(page_window->number_of_scrolling_channels==number_of_channels)&&
		signal_values&&(4==number_of_values_per_channel))
	{
		/* set up */
#if defined (MOTIF)
		display=page_window->user_interface->display;
		drawable=XtWindow(page_window->scrolling_area);
		XGetWindowAttributes(display,drawable,&attributes);
		width=attributes.width;
		height=attributes.height;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		window=page_window->scrolling_area;
		device_context=GetDC(window);
		GetClientRect(window,&drawing_rectangle);
			/*???DB.  Seems to return a nonzero on success (not TRUE) */
		height=drawing_rectangle.bottom;
		width=drawing_rectangle.right;
#endif /* defined (WINDOWS) */
		height -= 5;
		signal_line[4].x=x_axis[0].x;
		signal_line[3].x=(x_axis[0].x)+1;
		signal_line[2].x=(x_axis[0].x)+2;
		signal_line[1].x=(x_axis[0].x)+3;
		signal_line[0].x=(x_axis[0].x)+4;
		signal_line[4].y=signal_line[0].y;
		temp_float[0]=(float)0;
		temp_float[1]=(float)0;
		temp_float[2]=(float)0;
		temp_float[3]=(float)0;
		j=0;
		for (i=0;i<number_of_channels;i++)
		{
			unemap_get_gain(((page_window->scrolling_devices)[i])->channel->number,
				&pre_filter_gain,&post_filter_gain);
			gain=(page_window->scrolling_coefficients)[i]*
				(((page_window->scrolling_devices)[i])->channel->gain_correction)/
				(pre_filter_gain*post_filter_gain);
#if defined (OLD_CODE)
			gain=(page_window->scrolling_coefficients)[i]*
				(((page_window->scrolling_devices)[i])->channel->gain)/
				(pre_filter_gain*post_filter_gain);
#endif /* defined (OLD_CODE) */
			offset=((page_window->scrolling_devices)[i])->channel->offset;
			temp_float[0] += gain*((float)signal_values[j]-offset);
			j++;
			temp_float[1] += gain*((float)signal_values[j]-offset);
			j++;
			temp_float[2] += gain*((float)signal_values[j]-offset);
			j++;
			temp_float[3] += gain*((float)signal_values[j]-offset);
			j++;
		}
		for (i=0;i<4;i++)
		{
			signal_value=temp_float[3-i];
			if (page_window->signal_display_maximum<page_window->signal_display_minimum)
			{
				page_window->signal_display_maximum=signal_value;
				page_window->signal_display_minimum=signal_value;
			}
			else
			{
				if (signal_value<page_window->signal_display_minimum)
				{
					page_window->signal_display_minimum=signal_value;
				}
				else
				{
					if (signal_value>page_window->signal_display_maximum)
					{
						page_window->signal_display_maximum=signal_value;
					}
				}
			}
			signal_line[i].y=
				(short)((((page_window->display_maximum)-signal_value)*(float)height)/
				((page_window->display_maximum)-(page_window->display_minimum)));
		}
#if defined (MOTIF)
		XFillRectangle(display,drawable,
			(page_window->graphics_context).background_drawing_colour,fill_left,0,
			fill_width,height+1);
		XDrawLines(display,drawable,
			(page_window->graphics_context).foreground_drawing_colour,x_axis,2,
			CoordModeOrigin);
		XDrawLines(display,drawable,
			(page_window->graphics_context).foreground_drawing_colour,scroll_line,2,
			CoordModeOrigin);
		fill_left += 4;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		FillRect(device_context,&fill_rectangle,page_window->fill_brush);
		Polyline(device_context,x_axis,2);
		Polyline(device_context,scroll_line,2);
		fill_rectangle.left += 4;
		fill_rectangle.right += 4;
#endif /* defined (WINDOWS) */
		scroll_line[0].x += 4;
		scroll_line[1].x += 4;
		if (x_axis[0].x>0)
		{
#if defined (MOTIF)
			XDrawLines(display,drawable,
				(page_window->graphics_context).foreground_drawing_colour,signal_line,5,
				CoordModeOrigin);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			Polyline(device_context,signal_line,5);
#endif /* defined (WINDOWS) */
		}
		else
		{
#if defined (MOTIF)
			XDrawLines(display,drawable,
				(page_window->graphics_context).foreground_drawing_colour,signal_line,4,
				CoordModeOrigin);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			Polyline(device_context,signal_line,4);
#endif /* defined (WINDOWS) */
		}
		x_axis[0].x += 4;
		x_axis[1].x += 4;
		if (x_axis[1].x>width)
		{
			x_axis[0].x=0;
			x_axis[1].x=4;
			scroll_line[0].x=5;
			scroll_line[1].x=5;
#if defined (MOTIF)
			fill_left=1;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			fill_rectangle.left=1;
			fill_rectangle.right=5;
#endif /* defined (WINDOWS) */
		}
	}
	if (channel_numbers)
	{
		DEALLOCATE(channel_numbers);
	}
	if (signal_values)
	{
		DEALLOCATE(signal_values);
	}
	LEAVE;
} /* scrolling_hardware_callback */
#endif /* defined (OLD_CODE) */

static int add_scrolling_device(struct Page_window *page_window,
	struct Device *device)
/*******************************************************************************
LAST MODIFIED : 5 November 2000

DESCRIPTION :
==============================================================================*/
{
	float *scrolling_coefficients;
	int i,j,*last_scrolling_value_device,number_of_scrolling_channels,
		*number_of_scrolling_channels_device,return_code;
	struct Auxiliary_properties *auxiliary_properties;
	struct Channel **scrolling_channels;
	struct Device **scrolling_devices;

	ENTER(add_scrolling_device);
	return_code=0;
	if (page_window&&device)
	{
		number_of_scrolling_channels=1;
		if ((device->channel)||
			((device->description)&&(AUXILIARY==device->description->type)&&
			(0<(number_of_scrolling_channels=(device->description->properties).
			auxiliary.number_of_electrodes))))
		{
			if (REALLOCATE(scrolling_devices,page_window->scrolling_devices,
				struct Device *,(page_window->number_of_scrolling_devices)+1))
			{
				page_window->scrolling_devices=scrolling_devices;
			}
			if (REALLOCATE(number_of_scrolling_channels_device,
				page_window->number_of_scrolling_channels_device,int,
				(page_window->number_of_scrolling_devices)+1))
			{
				page_window->number_of_scrolling_channels_device=
					number_of_scrolling_channels_device;
			}
			if (REALLOCATE(last_scrolling_value_device,
				page_window->last_scrolling_value_device,int,
				(page_window->number_of_scrolling_devices)+1))
			{
				page_window->last_scrolling_value_device=last_scrolling_value_device;
			}
			if (REALLOCATE(scrolling_channels,page_window->scrolling_channels,
				struct Channel *,(page_window->number_of_scrolling_channels)+
				number_of_scrolling_channels))
			{
				page_window->scrolling_channels=scrolling_channels;
			}
			if (REALLOCATE(scrolling_coefficients,page_window->scrolling_coefficients,
				float,(page_window->number_of_scrolling_channels)+
				number_of_scrolling_channels))
			{
				page_window->scrolling_coefficients=scrolling_coefficients;
			}
			if (scrolling_channels&&scrolling_devices&&scrolling_coefficients&&
				number_of_scrolling_channels_device&&last_scrolling_value_device)
			{
				scrolling_devices[page_window->number_of_scrolling_devices]=device;
				number_of_scrolling_channels_device[page_window->
					number_of_scrolling_devices]=number_of_scrolling_channels;
				last_scrolling_value_device[page_window->number_of_scrolling_devices]=0;
				if (device->channel)
				{
					scrolling_channels[page_window->number_of_scrolling_channels]=
						device->channel;
					scrolling_coefficients[page_window->number_of_scrolling_channels]=
						(float)1;
					unemap_set_scrolling_channel(device->channel->number);
				}
				else
				{
					j=page_window->number_of_scrolling_channels;
					auxiliary_properties= &((device->description->properties).auxiliary);
					for (i=0;i<number_of_scrolling_channels;i++)
					{
						scrolling_channels[j]=
							((auxiliary_properties->electrodes)[i])->channel;
						scrolling_coefficients[j]=
							(auxiliary_properties->electrode_coefficients)[i];
						unemap_set_scrolling_channel((scrolling_channels[j])->number);
						j++;
					}
				}
				page_window->number_of_scrolling_channels +=
					number_of_scrolling_channels;
				(page_window->number_of_scrolling_devices)++;
				return_code=1;
			}
		}
	}
	LEAVE;

	return (return_code);
} /* add_scrolling_device */

static int remove_scrolling_device(struct Page_window *page_window,
	struct Device *device)
/*******************************************************************************
LAST MODIFIED : 6 November 2000

DESCRIPTION :
==============================================================================*/
{
	int i,j,k,return_code;
	struct Device *scrolling_device;

	ENTER(remove_scrolling_device);
	return_code=0;
	if (page_window&&device)
	{
		/* check if it is a scrolling device */
		i=0;
		j=0;
		while ((i<page_window->number_of_scrolling_devices)&&
			(device!=(scrolling_device=(page_window->scrolling_devices)[i])))
		{
			i++;
			if (scrolling_device->channel)
			{
				j++;
			}
			else
			{
				j += (scrolling_device->description->properties).auxiliary.
					number_of_electrodes;
			}
		}
		if (i<page_window->number_of_scrolling_devices)
		{
			(page_window->number_of_scrolling_devices)--;
			for (k=i;k<page_window->number_of_scrolling_devices;k++)
			{
				(page_window->scrolling_devices)[k]=
					(page_window->scrolling_devices)[k+1];
				(page_window->number_of_scrolling_channels_device)[k]=
					(page_window->number_of_scrolling_channels_device)[k+1];
				(page_window->last_scrolling_value_device)[k]=
					(page_window->last_scrolling_value_device)[k+1];
			}
			if (device->channel)
			{
				k=1;
			}
			else
			{
				k=(device->description->properties).auxiliary.number_of_electrodes;
			}
			page_window->number_of_scrolling_channels -= k;
			for (i=j;i<page_window->number_of_scrolling_channels;i++)
			{
				(page_window->scrolling_channels)[i]=
					(page_window->scrolling_channels)[i+k];
				(page_window->scrolling_coefficients)[i]=
					(page_window->scrolling_coefficients)[i+k];
			}
			unemap_clear_scrolling_channels();
			for (i=0;i<page_window->number_of_scrolling_channels;i++)
			{
				unemap_set_scrolling_channel(((page_window->scrolling_channels)[i])->
					number);
			}
			return_code=1;
		}
	}
	LEAVE;

	return (return_code);
} /* remove_scrolling_device */

#if defined (BACKGROUND_SAVING)
#if defined (MOTIF)
static void *save_write_signal_file_process(
	void *save_write_signal_file_background_data_void)
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
DWORD WINAPI save_write_signal_file_process(
	LPVOID save_write_signal_file_background_data_void)
#endif /* defined (WINDOWS) */
/*******************************************************************************
LAST MODIFIED : 13 January 2002

DESCRIPTION :
Called by unemap_get_samples_acquired_background to actually write the data.
==============================================================================*/
{
	FILE *output_file;
#if defined (OLD_CODE)
	float *gains,post_filter_gain,pre_filter_gain;
#endif /* defined (OLD_CODE) */
	int channel_number,i,j,number_of_channels,number_of_samples,number_of_signals,
		return_code;
	short *destination,*samples,*source;
	struct Device **device;
	struct Page_window *page_window;
	struct Rig *rig;
	struct Save_write_signal_file_background_data
		*save_write_signal_file_background_data;
	struct Signal_buffer *signal_buffer;

	ENTER(save_write_signal_file_process);
#if defined (DEBUG)
	/*???debug */
	printf("enter save_write_signal_file_process %p\n",
		save_write_signal_file_background_data_void);
#endif /* defined (DEBUG) */
	return_code=0;
	/* check arguments */
	if ((save_write_signal_file_background_data=
		(struct Save_write_signal_file_background_data *)
		save_write_signal_file_background_data_void)&&
		(0==save_write_signal_file_background_data->all_channels)&&
		(0<(number_of_samples=save_write_signal_file_background_data->
		number_of_samples))&&
		(samples=save_write_signal_file_background_data->samples)&&
		(page_window=save_write_signal_file_background_data->page_window)&&
		(page_window->rig_address)&&(rig= *(page_window->rig_address))&&
		unemap_get_number_of_channels(&number_of_channels)&&
		(save_write_signal_file_background_data->file_name)&&
		(save_write_signal_file_background_data->temp_file_name)&&
		(output_file=fopen(save_write_signal_file_background_data->file_name,"wb")))
	{
		i=rig->number_of_devices;
		device=rig->devices;
		while (i>0)
		{
			if ((*device)&&((*device)->signal)&&((*device)->channel)&&
				(signal_buffer=(*device)->signal->buffer))
			{
				signal_buffer->start=0;
				signal_buffer->end=(int)(number_of_samples-1);
				channel_number=((*device)->channel->number)-1;
				number_of_signals=signal_buffer->number_of_signals;
				destination=((signal_buffer->signals).short_int_values)+
					((*device)->signal->index);
				if ((0<=channel_number)&&(channel_number<number_of_channels))
				{
					source=(short *)samples+channel_number;
					for (j=(int)number_of_samples;j>0;j--)
					{
						*destination= *source;
						source += number_of_channels;
						destination += number_of_signals;
					}
				}
				else
				{
					for (j=(int)number_of_samples;j>0;j--)
					{
						*destination=0;
						destination += number_of_signals;
					}
				}
			}
			i--;
			device++;
		}
#if defined (DEBUG)
		/*???debug */
		printf("after reordering\n");
#endif /* defined (DEBUG) */
#if !defined (MIRADA)
#if defined (OLD_CODE)
		gains=(float *)NULL;
		i=rig->number_of_devices;
		if (ALLOCATE(gains,float,i))
		{
			device=rig->devices;
			while (i>0)
			{
				if ((*device)&&((*device)->signal)&&((*device)->signal->buffer)&&
					((*device)->channel))
				{
					gains[i-1]=(*device)->channel->gain;
					if (unemap_get_gain((*device)->channel->number,&pre_filter_gain,
						&post_filter_gain))
					{
						/*???DB.  Changes size of scrolling signal when saving in
							background */
						(*device)->channel->gain /= pre_filter_gain*post_filter_gain;
					}
				}
				i--;
				device++;
			}
		}
#endif /* defined (OLD_CODE) */
#endif /* !defined (MIRADA) */
		return_code=write_signal_file(output_file,rig);
#if !defined (MIRADA)
#if defined (OLD_CODE)
		if (gains)
		{
			i=rig->number_of_devices;
			device=rig->devices;
			while (i>0)
			{
				if ((*device)&&((*device)->signal)&&((*device)->signal->buffer))
				{
					(*device)->channel->gain=gains[i-1];
				}
				i--;
				device++;
			}
			DEALLOCATE(gains);
		}
#endif /* defined (OLD_CODE) */
#endif /* !defined (MIRADA) */
		fclose(output_file);
		if (return_code)
		{
			page_window->data_saved=1;
			rename(save_write_signal_file_background_data->temp_file_name,
				save_write_signal_file_background_data->file_name);
		}
		else
		{
			remove(save_write_signal_file_background_data->temp_file_name);
		}
		DEALLOCATE(save_write_signal_file_background_data->samples);
		DEALLOCATE(save_write_signal_file_background_data->temp_file_name);
		DEALLOCATE(save_write_signal_file_background_data->file_name);
		DEALLOCATE(save_write_signal_file_background_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"save_write_signal_file_process.  Invalid arguments %d %p %p",
			number_of_samples,samples,save_write_signal_file_background_data_void);
		return_code=0;
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave save_write_signal_file_process\n");
#endif /* defined (DEBUG) */
	LEAVE;

#if defined (MOTIF)
	return ((void *)NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	return ((DWORD)return_code);
#endif /* defined (WINDOWS) */
} /* save_write_signal_file_process */

static void save_write_signal_file_background(const int all_channels,
	const int number_of_samples,const short *samples,
	void *save_write_signal_file_background_data_void)
/*******************************************************************************
LAST MODIFIED : 26 November 2001

DESCRIPTION :
Called by unemap_get_samples_acquired_background to actually write the data.
==============================================================================*/
{
#if defined (MOTIF)
	pthread_t thread_id;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	DWORD thread_id;
#endif /* defined (WINDOWS) */
	struct Save_write_signal_file_background_data
		*save_write_signal_file_background_data;

	ENTER(save_write_signal_file_background);
#if defined (DEBUG)
	/*???debug */
	printf("enter save_write_signal_file_background.  %p\n",
		save_write_signal_file_background_data_void);
#endif /* defined (DEBUG) */
	/* check arguments */
	if (save_write_signal_file_background_data=
		(struct Save_write_signal_file_background_data *)
		save_write_signal_file_background_data_void)
	{
		save_write_signal_file_background_data->all_channels=all_channels;
		save_write_signal_file_background_data->number_of_samples=number_of_samples;
		save_write_signal_file_background_data->samples=(short *)samples;
#if defined (MOTIF)
		if (pthread_create(&thread_id,(pthread_attr_t *)NULL,
			save_write_signal_file_process,
			save_write_signal_file_background_data_void))
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		if (!CreateThread(/*no security attributes*/NULL,
			/*use default stack size*/0,save_write_signal_file_process,
			(LPVOID)save_write_signal_file_background_data_void,
			/*use default creation flags*/0,&thread_id))
#endif /* defined (WINDOWS) */
		{
			display_message(ERROR_MESSAGE,
#if defined (MOTIF)
				"save_write_signal_file_background.  pthread_create failed");
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				"save_write_signal_file_background.  CreateThread failed");
#endif /* defined (WINDOWS) */
			DEALLOCATE(save_write_signal_file_background_data);
			/*???RC This is very dodgy - deallocating a const short */
			DEALLOCATE(samples);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"save_write_signal_file_background.  Invalid arguments %d %d %p %p",
			all_channels,number_of_samples,samples,
			save_write_signal_file_background_data_void);
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave save_write_signal_file_background\n");
#endif /* defined (DEBUG) */
	LEAVE;
} /* save_write_signal_file_background */
#endif /* defined (BACKGROUND_SAVING) */

#if defined (BACKGROUND_SAVING)
static int save_write_signal_file(char *file_name,void *page_window_void)
/*******************************************************************************
LAST MODIFIED : 13 January 2002

DESCRIPTION :
This function writes the rig configuration and interval of signal data to the
named file.
==============================================================================*/
{
	char *temp_file_name;
	FILE *output_file,*temp_output_file;
	float post_filter_gain,pre_filter_gain;
	int i,return_code;
	struct Device **device;
	struct Page_window *page_window;
	struct Rig *rig;
	struct Save_write_signal_file_background_data
		*save_write_signal_file_background_data;

	ENTER(save_write_signal_file);
	return_code=0;
	/* check that the rig exists */
	if ((page_window=(struct Page_window *)page_window_void)&&
		(page_window->rig_address)&&(rig= *(page_window->rig_address)))
	{
		/* open the output file */
		if (output_file=fopen(file_name,"wb"))
		{
			/* get a temporary file to write to in background */
			if ((temp_file_name=tmpnam((char *)NULL))&&
				(temp_output_file=fopen(temp_file_name,"wb")))
			{
				fclose(output_file);
				remove(file_name);
				fclose(temp_output_file);
				remove(temp_file_name);
				if (ALLOCATE(save_write_signal_file_background_data,
					struct Save_write_signal_file_background_data,1))
				{
					save_write_signal_file_background_data->file_name=(char *)NULL;
					save_write_signal_file_background_data->temp_file_name=(char *)NULL;
					if (ALLOCATE(save_write_signal_file_background_data->file_name,char,
						strlen(file_name)+1)&&
						ALLOCATE(save_write_signal_file_background_data->temp_file_name,
						char,strlen(temp_file_name)+1))
					{
						strcpy(save_write_signal_file_background_data->file_name,file_name);
						strcpy(save_write_signal_file_background_data->temp_file_name,
							temp_file_name);
						save_write_signal_file_background_data->page_window=page_window;
						if (unemap_get_samples_acquired_background(0,
							(int)page_window->number_of_samples_to_save,
							save_write_signal_file_background,
							(void *)save_write_signal_file_background_data))
						{
							i=rig->number_of_devices;
							device=rig->devices;
							while (i>0)
							{
								if ((*device)&&((*device)->signal)&&
									((*device)->signal->buffer)&&((*device)->channel))
								{
									if (unemap_get_gain((*device)->channel->number,
										&pre_filter_gain,&post_filter_gain))
									{
										(*device)->channel->gain=
											((*device)->channel->gain_correction)/
											(pre_filter_gain*post_filter_gain);
									}
								}
								i--;
								device++;
							}
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"save_write_signal_file.  Could not retrieve samples");
							DEALLOCATE(save_write_signal_file_background_data->file_name);
							DEALLOCATE(save_write_signal_file_background_data->
								temp_file_name);
							DEALLOCATE(save_write_signal_file_background_data);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"save_write_signal_file.  Could not allocate file names");
						DEALLOCATE(save_write_signal_file_background_data->file_name);
						DEALLOCATE(save_write_signal_file_background_data->temp_file_name);
						DEALLOCATE(save_write_signal_file_background_data);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
"save_write_signal_file.  Could not allocate save_write_signal_file_background_data");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"save_write_signal_file.  Could not get temporary file");
				fclose(output_file);
				remove(file_name);
				return_code=0;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"save_write_signal_file.  Invalid file: %s",file_name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"save_write_signal_file.  Missing rig");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* save_write_signal_file */
#else /* defined (BACKGROUND_SAVING) */
static int save_write_signal_file(char *file_name,void *page_window_void)
/*******************************************************************************
LAST MODIFIED : 29 July 2000

DESCRIPTION :
This function writes the rig configuration and interval of signal data to the
named file.
==============================================================================*/
{
	FILE *output_file;
	int return_code;
	struct Page_window *page_window;
	struct Rig *rig;
#if !defined (MIRADA)
#if defined (OLD_CODE)
	float *gains;
#endif /* defined (OLD_CODE) */
	float post_filter_gain,pre_filter_gain;
	int i;
	struct Device **device;
#endif /* !defined (MIRADA) */

	ENTER(save_write_signal_file);
	/* check that the rig exists */
	if ((page_window=(struct Page_window *)page_window_void)&&
		(page_window->rig_address)&&(rig= *(page_window->rig_address)))
	{
		/* open the output file */
		if (output_file=fopen(file_name,"wb"))
		{
#if !defined (MIRADA)
			i=rig->number_of_devices;
			device=rig->devices;
			while (i>0)
			{
				if ((*device)&&((*device)->signal)&&((*device)->signal->buffer)&&
					((*device)->channel))
				{
					if (unemap_get_gain((*device)->channel->number,&pre_filter_gain,
						&post_filter_gain))
					{
						(*device)->channel->gain=((*device)->channel->gain_correction)/
							(pre_filter_gain*post_filter_gain);
					}
				}
				i--;
				device++;
			}
#if defined (OLD_CODE)
			gains=(float *)NULL;
			i=rig->number_of_devices;
			if (ALLOCATE(gains,float,i))
			{
				device=rig->devices;
				while (i>0)
				{
					if ((*device)&&((*device)->signal)&&((*device)->signal->buffer)&&
						((*device)->channel))
					{
						gains[i-1]=(*device)->channel->gain;
						if (unemap_get_gain((*device)->channel->number,&pre_filter_gain,
							&post_filter_gain))
						{
							(*device)->channel->gain /= pre_filter_gain*post_filter_gain;
						}
					}
					i--;
					device++;
				}
			}
#endif /* defined (OLD_CODE) */
#endif /* !defined (MIRADA) */
			return_code=write_signal_file(output_file,rig);
			fclose(output_file);
			if (return_code)
			{
				page_window->data_saved=1;
			}
			else
			{
				remove(file_name);
			}
#if !defined (MIRADA)
#if defined (OLD_CODE)
			if (gains)
			{
				i=rig->number_of_devices;
				device=rig->devices;
				while (i>0)
				{
					if ((*device)&&((*device)->signal)&&((*device)->signal->buffer))
					{
						(*device)->channel->gain=gains[i-1];
					}
					i--;
					device++;
				}
				DEALLOCATE(gains);
			}
#endif /* defined (OLD_CODE) */
#endif /* !defined (MIRADA) */
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"save_write_signal_file.  Invalid file: %s",
				file_name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"save_write_signal_file.  Missing rig");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* save_write_signal_file */
#endif /* defined (BACKGROUND_SAVING) */

#if defined (MOTIF)
static void expose_page_scrolling_area(Widget widget,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
The callback for redrawing part of the page scrolling area.
==============================================================================*/
{
	struct Page_window *page_window;
	XmDrawingAreaCallbackStruct *callback;

	ENTER(expose_page_scrolling_area);
	USE_PARAMETER(widget);
	if (page_window=(struct Page_window *)page_window_structure)
	{
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (callback->reason==XmCR_EXPOSE)
			{
				if (callback->event)
				{
					if (callback->event->type==Expose)
					{
						if (page_window->scrolling_area)
						{
							draw_scrolling_background(page_window);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"expose_page_scrolling_area.  Missing drawing area");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"expose_page_scrolling_area.  Incorrect event reason");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"expose_page_scrolling_area.  Missing event");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"expose_page_scrolling_area.  Incorrect reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"expose_page_scrolling_area.  Missing call_data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"expose_page_scrolling_area.  Missing page_window");
	}
	LEAVE;
} /* expose_page_scrolling_area */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void resize_page_scrolling_area(Widget widget,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
The callback for resizing a page drawing area.
==============================================================================*/
{
	struct Page_window *page_window;
	XmDrawingAreaCallbackStruct *callback;

	ENTER(resize_page_scrolling_area);
	USE_PARAMETER(widget);
	if (page_window=(struct Page_window *)page_window_structure)
	{
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (callback->reason==XmCR_RESIZE)
			{
				/*??? during creation there are resize callbacks without windows */
				if (callback->window)
				{
					if (page_window->scrolling_area)
					{
						draw_scrolling_background(page_window);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"resize_page_scrolling_area.  Missing drawing area");
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"resize_page_scrolling_area.  Incorrect reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"resize_page_scrolling_area.  call_data missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"resize_page_scrolling_area.  page_window missing");
	}
	LEAVE;
} /* resize_page_scrolling_area */
#endif /* defined (MOTIF) */

#if defined (OLD_CODE)
static int initialize_hardware(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 21 July 1999

DESCRIPTION :
==============================================================================*/
{
	int return_code;
#if !defined (MIRADA)
	char working_string[21];
	float filter_frequency;
#endif /* !defined (MIRADA) */

	ENTER(initialize_hardware);
	return_code=0;
	if (page_window)
	{
		return_code=1;
#if defined (MIRADA)
		/* start the interrupting */
		if (INVALID_HANDLE_VALUE!=page_window->device_driver)
		{
			start_interrupting(page_window->device_driver,page_window->scrolling_area,
				WM_USER,(LPARAM)page_window);
		}
#else /* defined (MIRADA) */
		if ((page_window->rig_address)&&(*(page_window->rig_address)))
		{
			if (unemap_configure(page_window->sampling_frequency,
				page_window->number_of_samples,
#if defined (MOTIF)
				page_window->user_interface->application_context,
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				(HWND)NULL,(UINT)0,
#endif /* defined (WINDOWS) */
				scrolling_hardware_callback,(void *)page_window,(float)25))
			{
				if (page_window->display_device)
				{
					add_scrolling_device(page_window,page_window->display_device);
				}
				unemap_start_scrolling();
#if defined (OLD_CODE)
/*???DB.  Moved to where initialize_hardware used to be */
				/* set the experiment checkbox */
#if defined (MOTIF)
				XmToggleButtonSetState(page_window->experiment_checkbox,False,False);
				XtSetSensitive(page_window->experiment_checkbox,True);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				CheckDlgButton(page_window->window,EXPERIMENT_CHECKBOX,BST_UNCHECKED);
				EnableWindow(page_window->experiment_checkbox,TRUE);
#endif /* defined (WINDOWS) */
#endif /* defined (OLD_CODE) */
				unemap_get_antialiasing_filter_frequency(1,&filter_frequency);
				sprintf(working_string,"%.0f",filter_frequency);
#if defined (MOTIF)
				XtVaSetValues((page_window->low_pass).value,XmNvalue,working_string,
					NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				Edit_SetText((page_window->low_pass_filter).edit,working_string);
#endif /* defined (WINDOWS) */
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"initialize_hardware.  Missing rig");
		}
#endif /* defined (MIRADA) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"initialize_hardware.  page_window missing");
	}
	LEAVE;

	return (return_code);
} /* initialize_hardware */
#endif /* defined (OLD_CODE) */

#if defined (WINDOWS)
static LRESULT CALLBACK Page_window_scrolling_area_class_proc(HWND window,
	UINT message_identifier,WPARAM first_message,LPARAM second_message)
/*******************************************************************************
LAST MODIFIED : 26 April 1999

DESCRIPTION :
???DB.  Should return 0 if it processes the message.
==============================================================================*/
{
	CREATESTRUCT *wm_create_structure;
	struct Page_window *page_window;
	LRESULT return_code;

	ENTER(Page_window_scrolling_area_class_proc);
	switch (message_identifier)
	{
		case WM_CLOSE:
		{
			/* destroy fill brush */
			if (page_window=(struct Page_window *)GetWindowLong(window,0))
			{
				DeleteObject(page_window->fill_brush);
			}
			DestroyWindow(window);
			return_code=0;
		} break;
		case WM_CREATE:
		{
			/* retrieve the acquisition window out of <window> */
			wm_create_structure=(CREATESTRUCT *)second_message;
			page_window=(struct Page_window *)(wm_create_structure->lpCreateParams);
				/*???DB.  Check for NT - see WM_CREATE */
			page_window->scrolling_area=window;
			/* set the fill colour */
			page_window->fill_brush=CreateSolidBrush(0x00ffffff);
			/* set the data */
			SetWindowLong(window,0,(LONG)page_window);
			/* set the experiment checkbox */
			CheckDlgButton(page_window->window,EXPERIMENT_CHECKBOX,BST_UNCHECKED);
			EnableWindow(page_window->experiment_checkbox,TRUE);
#if defined (OLD_CODE)
/*???DB.  Move to start_experiment ? */
			initialize_hardware(page_window);
#endif /* defined (OLD_CODE) */
			/*???DB.  Any other processing ? */
			return_code=DefWindowProc(window,message_identifier,first_message,
				second_message);
		} break;
		case WM_DESTROY:
		{
			struct Page_window *page_window;

			page_window=(struct Page_window *)GetWindowLong(window,0);
			destroy_Page_window(&page_window);
			PostQuitMessage(0);
			return_code=0;
		} break;
		case WM_PAINT:
		{
			HDC device_context;
			HWND scrolling_area;
			PAINTSTRUCT paint_struct;
			struct Page_window *page_window;

			/* check for update rectangle */
			if ((TRUE==GetUpdateRect(window,(LPRECT)NULL,FALSE))&&
				(page_window=(struct Page_window *)GetWindowLong(window,0)))
			{
				scrolling_area=page_window->scrolling_area;
				device_context=BeginPaint(scrolling_area,&paint_struct);
					/*???DB.  I was using GetDC instead of BeginPaint, but this meant
						that WM_PAINT messages were being continuously sent to the
						scrolling_area.  BeginPaint/EndPaint should only be called in
						response to WM_PAINT (see Win32 SDK).  They must have something to
						do with removing the WM_PAINT message from the message queue */
					/*???DB.  BeginPaint/EndPaint should not be called if GetUpdateRect
						fails */
				draw_scrolling_background(
					(struct Page_window *)GetWindowLong(window,0));
				EndPaint(scrolling_area,&paint_struct);
				return_code=0;
			}
			else
			{
				return_code=DefWindowProc(window,message_identifier,first_message,
					second_message);
			}
		} break;
#if defined (MIRADA)
		case WM_USER:
		{
			HDC device_context;
			int averaging_length,i,j;
			long sum;
			RECT drawing_rectangle;
			short signal_value;
			short *hardware_buffer;
			unsigned long offset,number_of_channels,number_of_samples,sample_number;

			if ((page_window=(struct Page_window *)GetWindowLong(window,0))&&
				(page_window->display_device)&&
				(hardware_buffer=page_window->mirada_buffer))
			{
				device_context=GetDC(window);
				GetClientRect(window,&drawing_rectangle);
					/*???DB.  Seems to return a nonzero on success (not TRUE) */
				sample_number=(unsigned long)second_message;
				number_of_samples=page_window->number_of_samples;
				number_of_channels=page_window->number_of_channels;
				offset=number_of_channels*sample_number+
					(page_window->display_device->signal->index);
				{
					averaging_length=10;
					drawing_rectangle.bottom -= 5;
					signal_line[4].x=x_axis[0].x;
					signal_line[3].x=(x_axis[0].x)+1;
					signal_line[2].x=(x_axis[0].x)+2;
					signal_line[1].x=(x_axis[0].x)+3;
					signal_line[0].x=(x_axis[0].x)+4;
					signal_line[4].y=signal_line[0].y;
					for (j=0;j<4;j++)
					{
						sum=0;
						for (i=averaging_length;i>0;i--)
						{
							sum += (long)(hardware_buffer[offset]);
							if (offset<number_of_channels)
							{
								offset += number_of_channels*(number_of_samples-1);
							}
							else
							{
								offset -= number_of_channels;
							}
						}
#if defined (OLD_CODE)
						if (page_window->signal_display_maximum<page_window->signal_display_minimum)
						{
							page_window->signal_display_maximum=signal_value;
							page_window->signal_display_minimum=signal_value;
						}
						else
						{
							if (signal_value<page_window->signal_display_minimum)
							{
								page_window->signal_display_minimum=signal_value;
							}
							else
							{
								if (signal_value>page_window->signal_display_maximum)
								{
									page_window->signal_display_maximum=signal_value;
								}
							}
						}
#endif /* defined (OLD_CODE) */
						signal_line[j].y=
							(short)((((page_window->display_device->signal_display_maximum)-
							(float)signal_value)*(float)drawing_rectangle.bottom)/
							((page_window->display_device->signal_display_maximum)-
							(page_window->display_device->signal_display_minimum)));
					}
					FillRect(device_context,&fill_rectangle,fill_brush);
					Polyline(device_context,x_axis,2);
					Polyline(device_context,scroll_line,2);
					fill_rectangle.left += 4;
					fill_rectangle.right += 4;
					scroll_line[0].x += 4;
					scroll_line[1].x += 4;
					if (x_axis[0].x>0)
					{
						Polyline(device_context,signal_line,5);
					}
					else
					{
						Polyline(device_context,signal_line,4);
					}
					x_axis[0].x += 4;
					x_axis[1].x += 4;
					if (x_axis[1].x>drawing_rectangle.right)
					{
						x_axis[0].x=0;
						x_axis[1].x=4;
						scroll_line[0].x=5;
						scroll_line[1].x=5;
						fill_rectangle.left=1;
						fill_rectangle.right=5;
					}
				}
			}
			return_code=0;
		} break;
#endif /* defined (MIRADA) */
		default:
		{
			return_code=DefWindowProc(window,message_identifier,first_message,
				second_message);
		} break;
	}
	LEAVE;

	return (return_code);
} /* Page_window_scrolling_area_class_proc */
#endif /* defined (WINDOWS) */

static int show_display_gain(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 23 September 2000

DESCRIPTION :
Writes the gain for the current channel into the gain field.
==============================================================================*/
{
	float post_filter_gain,pre_filter_gain;
	int return_code;
	char number_string[21];

	ENTER(show_display_gain);
	return_code=0;
	if (page_window)
	{
		return_code=1;
		if ((page_window->display_device)&&(page_window->display_device->channel))
		{
			unemap_get_gain(page_window->display_device->channel->number,
				&pre_filter_gain,&post_filter_gain);
			sprintf(number_string,"%g",pre_filter_gain*post_filter_gain);
#if defined (MOTIF)
			XtVaSetValues((page_window->gain).value,XmNvalue,number_string,NULL);
			XtSetSensitive((page_window->gain).value,True);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			Edit_SetText((page_window->gain).edit,number_string);
			EnableWindow((page_window->gain).edit,TRUE);
#endif /* defined (WINDOWS) */
		}
		else
		{
#if defined (MOTIF)
			XtVaSetValues((page_window->gain).value,XmNvalue," ",NULL);
			XtSetSensitive((page_window->gain).value,False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			Edit_SetText((page_window->gain).edit," ");
			EnableWindow((page_window->gain).edit,FALSE);
#endif /* defined (WINDOWS) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"show_display_gain.  Missing page_window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* show_display_gain */

static int show_display_maximum(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 8 November 2000

DESCRIPTION :
Writes the maximum for the current channel into the maximum field.
==============================================================================*/
{
	char number_string[21];
#if defined (OLD_CODE)
	float channel_gain,channel_offset,post_filter_gain,pre_filter_gain;
#endif /* defined (OLD_CODE) */
	int return_code;

	ENTER(show_display_maximum);
	return_code=0;
	if (page_window)
	{
		if (page_window->display_device)
		{
			return_code=1;
#if defined (OLD_CODE)
			channel_gain=page_window->display_device->channel->gain;
			channel_offset=page_window->display_device->channel->offset;
			unemap_get_gain(page_window->display_device->channel->number,
				&pre_filter_gain,&post_filter_gain);
			channel_gain /= pre_filter_gain*post_filter_gain;
			sprintf(number_string,"%g",
				channel_gain*((float)(page_window->display_maximum)-channel_offset));
#endif /* defined (OLD_CODE) */
			sprintf(number_string,"%g",page_window->display_device->signal_display_maximum);
#if defined (MOTIF)
			XtVaSetValues((page_window->maximum).value,XmNvalue,number_string,NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			Edit_SetText((page_window->maximum).edit,number_string);
#endif /* defined (WINDOWS) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"show_display_maximum.  Missing page_window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* show_display_maximum */

static int show_display_minimum(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 2 August 1999

DESCRIPTION :
Writes the minimum for the current channel into the minimum field.
==============================================================================*/
{
	char number_string[21];
#if defined (OLD_CODE)
	float channel_gain,channel_offset,post_filter_gain,pre_filter_gain;
#endif /* defined (OLD_CODE) */
	int return_code;

	ENTER(show_display_minimum);
	return_code=0;
	if (page_window)
	{
		if (page_window->display_device)
		{
			return_code=1;
#if defined (OLD_CODE)
			channel_gain=page_window->display_device->channel->gain;
			channel_offset=page_window->display_device->channel->offset;
			unemap_get_gain(page_window->display_device->channel->number,
				&pre_filter_gain,&post_filter_gain);
			channel_gain /= pre_filter_gain*post_filter_gain;
			sprintf(number_string,"%g",
				channel_gain*((float)(page_window->display_minimum)-channel_offset));
#endif /* defined (OLD_CODE) */
			sprintf(number_string,"%g",page_window->display_device->signal_display_minimum);
#if defined (MOTIF)
			XtVaSetValues((page_window->minimum).value,XmNvalue,number_string,NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			Edit_SetText((page_window->minimum).edit,number_string);
#endif /* defined (WINDOWS) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"show_display_minimum.  Missing page_window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* show_display_minimum */

static int page_window_set_gain(struct Page_window *page_window,
	int channel_number,float gain)
/*******************************************************************************
LAST MODIFIED : 24 September 2000

DESCRIPTION :
A simplified/specialized version of unemap_set_gain.
==============================================================================*/
{
	float post_filter_gain,pre_filter_gain;
	int return_code;

	ENTER(page_window_set_gain);
	return_code=0;
	if (page_window)
	{
		switch (page_window->unemap_hardware_version)
		{
			case UnEmap_1V2:
			default:
			{
				pre_filter_gain=(float)1;
			} break;
			case UnEmap_2V1:
			case UnEmap_2V2:
			case UnEmap_2V1|UnEmap_2V2:
			{
				pre_filter_gain=gain/(float)11;
				if (pre_filter_gain>(float)8)
				{
					pre_filter_gain=(float)8;
				}
			} break;
		}
		post_filter_gain=gain/pre_filter_gain;
		return_code=unemap_set_gain(channel_number,pre_filter_gain,
			post_filter_gain);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"page_window_set_gain.  Missing page_window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* page_window_set_gain */

static int update_display_gain(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 16 May 2000

DESCRIPTION :
Reads the string in the gain field of the <page_window> and updates the
scrolling display.  Returns 1 if it is able to update, otherwise it returns 0
==============================================================================*/
{
	float gain,post_filter_gain,pre_filter_gain,temp;
	int channel_number,return_code;
	char *working_string;
#if defined (WINDOWS)
	int working_string_length;
#endif /* defined (WINDOWS) */

	ENTER(update_display_gain);
	return_code=0;
	if (page_window)
	{
		if ((page_window->display_device)&&(page_window->display_device->channel))
		{
			channel_number=page_window->display_device->channel->number;
			unemap_get_gain(channel_number,&pre_filter_gain,&post_filter_gain);
			gain=pre_filter_gain*post_filter_gain;
#if defined (MOTIF)
			working_string=(char *)NULL;
			XtVaGetValues((page_window->gain).value,XmNvalue,&working_string,NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			working_string_length=Edit_GetTextLength((page_window->gain).edit)+1;
				/*???DB.  GetWindowTextLength seems to give 1 less than the number of
					characters */
			if (ALLOCATE(working_string,char,working_string_length+1))
			{
				Edit_GetText((page_window->gain).edit,working_string,
					working_string_length);
#endif /* defined (WINDOWS) */
				if (1==sscanf(working_string,"%f",&temp))
				{
					page_window_set_gain(page_window,channel_number,temp);
					unemap_get_gain(channel_number,&pre_filter_gain,
						&post_filter_gain);
					if (gain!=pre_filter_gain*post_filter_gain)
					{
						show_display_maximum(page_window);
						show_display_minimum(page_window);
					}
				}
#if defined (WINDOWS)
				DEALLOCATE(working_string);
			}
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
			XtFree(working_string);
#endif /* defined (MOTIF) */
			show_display_gain(page_window);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"update_display_gain.  Missing page_window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* update_display_gain */

#if defined (MOTIF)
static void update_display_gain_callback(Widget *widget_id,
	XtPointer page_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Motif wrapper for update_display_gain.
==============================================================================*/
{
	XmAnyCallbackStruct *text_data;

	ENTER(update_display_gain_callback);
	USE_PARAMETER(widget_id);
	if ((text_data=(XmAnyCallbackStruct *)call_data)&&
		((XmCR_ACTIVATE==text_data->reason)||
		(XmCR_LOSING_FOCUS==text_data->reason)))
	{
		update_display_gain((struct Page_window *)page_window);
	}
	LEAVE;
} /* update_display_gain_callback */
#endif /* defined (MOTIF) */

static int update_display_maximum(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 2 August 1999

DESCRIPTION :
Reads the string in the maximum field of the <page_window> and updates the
scrolling display.  Returns 1 if it is able to update, otherwise it returns 0
==============================================================================*/
{
	char *working_string;
	float display_maximum;
#if defined (OLD_CODE)
	float channel_gain,channel_offset,post_filter_gain,pre_filter_gain,temp;
#endif /* defined (OLD_CODE) */
	int return_code;
#if defined (OLD_CODE)
	long int display_maximum;
#endif /* defined (OLD_CODE) */
#if defined (WINDOWS)
	int working_string_length;
#endif /* defined (WINDOWS) */

	ENTER(update_display_maximum);
	return_code=0;
	if (page_window)
	{
		if (page_window->display_device)
		{
#if defined (OLD_CODE)
			channel_gain=page_window->display_device->channel->gain;
			channel_offset=page_window->display_device->channel->offset;
			unemap_get_gain(page_window->display_device->channel->number,
				&pre_filter_gain,&post_filter_gain);
			channel_gain /= pre_filter_gain*post_filter_gain;
#endif /* defined (OLD_CODE) */
#if defined (MOTIF)
			working_string=(char *)NULL;
			XtVaGetValues((page_window->maximum).value,XmNvalue,&working_string,NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			working_string_length=Edit_GetTextLength((page_window->maximum).edit)+1;
				/*???DB.  GetWindowTextLength seems to give 1 less than the number of
					characters */
			if (ALLOCATE(working_string,char,working_string_length+1))
			{
				Edit_GetText((page_window->maximum).edit,working_string,
					working_string_length);
#endif /* defined (WINDOWS) */
				if (1==sscanf(working_string,"%f",&display_maximum))
#if defined (OLD_CODE)
				if (1==sscanf(working_string,"%f",&temp))
#endif /* defined (OLD_CODE) */
				{
#if defined (OLD_CODE)
					temp=channel_offset+temp/channel_gain;
					if (temp>(float)(LONG_MAX/2))
					{
						temp=(float)(LONG_MAX/2);
					}
					else
					{
						if (temp<(float)(LONG_MIN/2))
						{
							temp=(float)(LONG_MIN/2);
						}
					}
					display_maximum=(long int)temp;
#endif /* defined (OLD_CODE) */
					if (display_maximum!=page_window->display_device->signal_display_maximum)
					{
						if (display_maximum>page_window->display_device->signal_display_minimum)
						{
							page_window->display_device->signal_display_maximum=display_maximum;
							draw_scrolling_background(page_window);
							return_code=1;
						}
					}
				}
#if defined (WINDOWS)
				DEALLOCATE(working_string);
			}
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
			XtFree(working_string);
#endif /* defined (MOTIF) */
			show_display_maximum(page_window);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"update_display_maximum.  Missing page_window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* update_display_maximum */

#if defined (MOTIF)
static void update_display_maximum_callback(Widget *widget_id,
	XtPointer page_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Motif wrapper for update_display_maximum.
==============================================================================*/
{
	XmAnyCallbackStruct *text_data;

	ENTER(update_display_maximum_callback);
	USE_PARAMETER(widget_id);
	if ((text_data=(XmAnyCallbackStruct *)call_data)&&
		((XmCR_ACTIVATE==text_data->reason)||
		(XmCR_LOSING_FOCUS==text_data->reason)))
	{
		update_display_maximum((struct Page_window *)page_window);
	}
	LEAVE;
} /* update_display_maximum_callback */
#endif /* defined (MOTIF) */

static int update_display_minimum(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 2 August 1999

DESCRIPTION :
Reads the string in the minimum field of the <page_window> and updates the
scrolling display.  Returns 1 if it is able to update, otherwise it returns 0
==============================================================================*/
{
	char *working_string;
	float display_minimum;
#if defined (OLD_CODE)
	float channel_gain,channel_offset,post_filter_gain,pre_filter_gain,temp;
#endif /* defined (OLD_CODE) */
	int return_code;
#if defined (OLD_CODE)
	long int display_minimum;
#endif /* defined (OLD_CODE) */
#if defined (WINDOWS)
	int working_string_length;
#endif /* defined (WINDOWS) */

	ENTER(update_display_minimum);
	return_code=0;
	if (page_window)
	{
		if (page_window->display_device)
		{
#if defined (OLD_CODE)
			channel_gain=page_window->display_device->channel->gain;
			channel_offset=page_window->display_device->channel->offset;
			unemap_get_gain(page_window->display_device->channel->number,
				&pre_filter_gain,&post_filter_gain);
			channel_gain /= pre_filter_gain*post_filter_gain;
#endif /* defined (OLD_CODE) */
#if defined (MOTIF)
			working_string=(char *)NULL;
			XtVaGetValues((page_window->minimum).value,XmNvalue,&working_string,NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			working_string_length=Edit_GetTextLength((page_window->minimum).edit)+1;
				/*???DB.  GetWindowTextLength seems to give 1 less than the number of
					characters */
			if (ALLOCATE(working_string,char,working_string_length+1))
			{
				Edit_GetText((page_window->minimum).edit,working_string,
					working_string_length);
#endif /* defined (WINDOWS) */
#if defined (OLD_CODE)
				if (1==sscanf(working_string,"%f",&temp))
#endif /* defined (OLD_CODE) */
				if (1==sscanf(working_string,"%f",&display_minimum))
				{
#if defined (OLD_CODE)
					temp=channel_offset+temp/channel_gain;
					if (temp>(float)(LONG_MAX/2))
					{
						temp=(float)(LONG_MAX/2);
					}
					else
					{
						if (temp<(float)(LONG_MIN/2))
						{
							temp=(float)(LONG_MIN/2);
						}
					}
					display_minimum=(long int)temp;
#endif /* defined (OLD_CODE) */
					if (display_minimum!=page_window->display_device->signal_display_minimum)
					{
						if (display_minimum<page_window->display_device->signal_display_maximum)
						{
							page_window->display_device->signal_display_minimum=display_minimum;
							draw_scrolling_background(page_window);
							return_code=1;
						}
					}
				}
#if defined (WINDOWS)
				DEALLOCATE(working_string);
			}
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
			XtFree(working_string);
#endif /* defined (MOTIF) */
			show_display_minimum(page_window);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"update_display_minimum.  Missing page_window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* update_display_minimum */

#if defined (MOTIF)
static void update_display_minimum_callback(Widget *widget_id,
	XtPointer page_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Motif wrapper for update_display_minimum.
==============================================================================*/
{
	XmAnyCallbackStruct *text_data;

	ENTER(update_display_minimum_callback);
	USE_PARAMETER(widget_id);
	if ((text_data=(XmAnyCallbackStruct *)call_data)&&
		((XmCR_ACTIVATE==text_data->reason)||
		(XmCR_LOSING_FOCUS==text_data->reason)))
	{
		update_display_minimum((struct Page_window *)page_window);
	}
	LEAVE;
} /* update_display_minimum_callback */
#endif /* defined (MOTIF) */

static void update_stimulating_settings(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 11 November 2001

DESCRIPTION :
Updates the settings for the stimulating buttons.
==============================================================================*/
{
	int display_channel_number,i,number_of_off_stimulating_channels,
		number_of_on_stimulators,*stimulating_channel_numbers,stimulator_number;
#if defined (MOTIF)
	Boolean status;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	UINT checkbox_state;
#endif /* defined (WINDOWS) */

	ENTER(update_stimulating_settings);
	if (page_window&&(0<page_window->number_of_stimulators)&&
		!(page_window->pacing_button))
	{
    if (page_window->isolate_checkbox)
		{
#if defined (MOTIF)
			XtVaGetValues(page_window->isolate_checkbox,XmNset,&status,NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			checkbox_state=IsDlgButtonChecked(page_window->window,ISOLATE_CHECKBOX);
#endif /* defined (WINDOWS) */
		}
		else
		{
#if defined (MOTIF)
			status=False;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			checkbox_state=BST_UNCHECKED;
#endif /* defined (WINDOWS) */
		}
		if (
#if defined (MOTIF)
			(False==status)
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			(BST_UNCHECKED==checkbox_state)
#endif /* defined (WINDOWS) */
			)
		{
			number_of_on_stimulators=0;
			number_of_off_stimulating_channels=0;
			for (i=0;i<page_window->number_of_stimulators;i++)
			{
				if (((page_window->stimulators)[i]).on)
				{
					number_of_on_stimulators++;
				}
				else
				{
					number_of_off_stimulating_channels +=
						((page_window->stimulators)[i]).number_of_channels;
				}
			}
			if (0<number_of_on_stimulators)
			{
#if defined (MOTIF)
				XtSetSensitive(page_window->stop_all_stimulators_button,True);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				EnableWindow(page_window->stop_all_stimulators_button,TRUE);
#endif /* defined (WINDOWS) */
			}
			else
			{
#if defined (MOTIF)
				XtSetSensitive(page_window->stop_all_stimulators_button,False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				EnableWindow(page_window->stop_all_stimulators_button,FALSE);
#endif /* defined (WINDOWS) */
			}
			if (0<number_of_off_stimulating_channels)
			{
#if defined (MOTIF)
				XtSetSensitive(page_window->start_all_stimulators_button,True);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				EnableWindow(page_window->start_all_stimulators_button,TRUE);
#endif /* defined (WINDOWS) */
			}
			else
			{
#if defined (MOTIF)
				XtSetSensitive(page_window->start_all_stimulators_button,False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				EnableWindow(page_window->start_all_stimulators_button,FALSE);
#endif /* defined (WINDOWS) */
			}
			if ((page_window->display_device)&&(page_window->display_device->channel))
			{
				display_channel_number=page_window->display_device->channel->number;
				stimulator_number=page_window->number_of_stimulators;
				while ((stimulator_number>0)&&!unemap_channel_valid_for_stimulator(
					stimulator_number,display_channel_number))
				{
					stimulator_number--;
				}
				if (stimulator_number>0)
				{
					stimulator_number--;
					if (0<
						((page_window->stimulators)[stimulator_number]).number_of_channels)
					{
#if defined (MOTIF)
						XtSetSensitive(page_window->stimulator_checkbox,True);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
						EnableWindow(page_window->stimulator_checkbox,TRUE);
#endif /* defined (WINDOWS) */
					}
					else
					{
#if defined (MOTIF)
						XtSetSensitive(page_window->stimulator_checkbox,False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
						EnableWindow(page_window->stimulator_checkbox,FALSE);
#endif /* defined (WINDOWS) */
					}
					if (((page_window->stimulators)[stimulator_number]).on)
					{
#if defined (MOTIF)
						XmToggleButtonSetState(page_window->stimulator_checkbox,True,False);
						XtSetSensitive(page_window->stimulate_checkbox,False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
						CheckDlgButton(page_window->window,STIMULATOR_CHECKBOX,BST_CHECKED);
						EnableWindow(page_window->stimulate_checkbox,FALSE);
#endif /* defined (WINDOWS) */
					}
					else
					{
#if defined (MOTIF)
						XmToggleButtonSetState(page_window->stimulator_checkbox,False,
							False);
						XtSetSensitive(page_window->stimulate_checkbox,True);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
						CheckDlgButton(page_window->window,STIMULATOR_CHECKBOX,
							BST_UNCHECKED);
						EnableWindow(page_window->stimulate_checkbox,TRUE);
#endif /* defined (WINDOWS) */
					}
					if (0<(i=((page_window->stimulators)[stimulator_number]).
						number_of_channels))
					{
						stimulating_channel_numbers=
							((page_window->stimulators)[stimulator_number]).channel_numbers;
						do
						{
							i--;
						} while ((i>0)&&
							(display_channel_number<stimulating_channel_numbers[i]));
						if (display_channel_number==stimulating_channel_numbers[i])
						{
#if defined (MOTIF)
							XmToggleButtonSetState(page_window->stimulate_checkbox,True,
								False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
							CheckDlgButton(page_window->window,STIMULATE_CHECKBOX,
								BST_CHECKED);
#endif /* defined (WINDOWS) */
						}
						else
						{
#if defined (MOTIF)
							XmToggleButtonSetState(page_window->stimulate_checkbox,False,
								False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
							CheckDlgButton(page_window->window,STIMULATE_CHECKBOX,
								BST_UNCHECKED);
#endif /* defined (WINDOWS) */
						}
					}
					else
					{
#if defined (MOTIF)
						XmToggleButtonSetState(page_window->stimulate_checkbox,False,False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
						CheckDlgButton(page_window->window,STIMULATE_CHECKBOX,
							BST_UNCHECKED);
#endif /* defined (WINDOWS) */
					}
				}
				else
				{
#if defined (MOTIF)
					XmToggleButtonSetState(page_window->stimulate_checkbox,False,False);
					XtSetSensitive(page_window->stimulate_checkbox,False);
					XmToggleButtonSetState(page_window->stimulator_checkbox,False,False);
					XtSetSensitive(page_window->stimulator_checkbox,False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
					CheckDlgButton(page_window->window,STIMULATE_CHECKBOX,BST_UNCHECKED);
					EnableWindow(page_window->stimulate_checkbox,FALSE);
					CheckDlgButton(page_window->window,STIMULATOR_CHECKBOX,BST_UNCHECKED);
					EnableWindow(page_window->stimulator_checkbox,FALSE);
#endif /* defined (WINDOWS) */
				}
			}
			else
			{
#if defined (MOTIF)
				XmToggleButtonSetState(page_window->stimulate_checkbox,False,False);
				XtSetSensitive(page_window->stimulate_checkbox,False);
				XmToggleButtonSetState(page_window->stimulator_checkbox,False,False);
				XtSetSensitive(page_window->stimulator_checkbox,False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				CheckDlgButton(page_window->window,STIMULATE_CHECKBOX,BST_UNCHECKED);
				EnableWindow(page_window->stimulate_checkbox,FALSE);
				CheckDlgButton(page_window->window,STIMULATOR_CHECKBOX,BST_UNCHECKED);
				EnableWindow(page_window->stimulator_checkbox,FALSE);
#endif /* defined (WINDOWS) */
			}
		}
		else
		{
#if defined (MOTIF)
			XtSetSensitive(page_window->stop_all_stimulators_button,False);
			XtSetSensitive(page_window->start_all_stimulators_button,False);
			XmToggleButtonSetState(page_window->stimulate_checkbox,False,False);
			XtSetSensitive(page_window->stimulate_checkbox,False);
			XmToggleButtonSetState(page_window->stimulator_checkbox,False,False);
			XtSetSensitive(page_window->stimulator_checkbox,False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			EnableWindow(page_window->stop_all_stimulators_button,FALSE);
			EnableWindow(page_window->start_all_stimulators_button,FALSE);
			CheckDlgButton(page_window->window,STIMULATE_CHECKBOX,BST_UNCHECKED);
			EnableWindow(page_window->stimulate_checkbox,FALSE);
			CheckDlgButton(page_window->window,STIMULATOR_CHECKBOX,BST_UNCHECKED);
			EnableWindow(page_window->stimulator_checkbox,FALSE);
#endif /* defined (WINDOWS) */
		}
	}
	LEAVE;
} /* update_stimulating_settings */

static void change_display_device_number(struct Page_window *page_window,
	int device_number)
/*******************************************************************************
LAST MODIFIED : 13 November 2000

DESCRIPTION :
==============================================================================*/
{
	int i;
	struct Device *device;
#if defined (MOTIF)
	Boolean status;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	UINT checkbox_state;
#endif /* defined (WINDOWS) */

	ENTER(change_display_device_number);
	if (page_window&&(page_window->rig_address)&&
		(*(page_window->rig_address))&&(0<=device_number)&&(device_number<
		((*(page_window->rig_address))->number_of_devices))&&
		(device=((*(page_window->rig_address))->devices)[device_number])&&
#if defined (OLD_CODE)
		(device->signal)&&(device->channel)&&(0<device->channel->number)&&
		(device->channel->number<=(int)(page_window->number_of_channels))&&
#endif /* defined (OLD_CODE) */
		(device->description)&&(device->description->name))
	{
#if defined (MOTIF)
		XtVaGetValues(page_window->scrolling_checkbox,XmNset,&status,NULL);
		if (False==status)
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		checkbox_state=IsDlgButtonChecked(page_window->window,
			SCROLLING_CHECKBOX);
		if (BST_UNCHECKED==checkbox_state)
#endif /* defined (WINDOWS) */
		{
			remove_scrolling_device(page_window,page_window->display_device);
		}
		page_window->display_device=device;
		page_window->display_device_number=device_number;
		/* check if it is a scrolling device */
		i=0;
		while ((i<page_window->number_of_scrolling_devices)&&
			(device!=(page_window->scrolling_devices)[i]))
		{
			i++;
		}
		if (i<page_window->number_of_scrolling_devices)
		{
			/* to make sure that current signal is at the top */
			remove_scrolling_device(page_window,device);
#if defined (MOTIF)
			XmToggleButtonSetState(page_window->scrolling_checkbox,True,
				False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			CheckDlgButton(page_window->window,SCROLLING_CHECKBOX,
				BST_CHECKED);
#endif /* defined (WINDOWS) */
		}
		else
		{
#if defined (MOTIF)
			XmToggleButtonSetState(page_window->scrolling_checkbox,False,
				False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			CheckDlgButton(page_window->window,SCROLLING_CHECKBOX,
				BST_UNCHECKED);
#endif /* defined (WINDOWS) */
		}
		add_scrolling_device(page_window,device);
		update_stimulating_settings(page_window);
#if defined (OLD_CODE)
		/* update means that the size of the signal will change if the gain
			changes */
		update_display_maximum(page_window);
		update_display_minimum(page_window);
#endif /* defined (OLD_CODE) */
		show_display_maximum(page_window);
		show_display_minimum(page_window);
		show_display_gain(page_window);
#if defined (OLD_CODE)
		/* reset range when change channel */
		page_window->signal_display_maximum=(float)0;
		page_window->signal_display_minimum=(float)1;
#endif /* defined (OLD_CODE) */
#if defined (MOTIF)
		XtVaSetValues((page_window->electrode).value,XmNvalue,
			device->description->name,NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		Edit_SetText((page_window->electrode).edit,device->description->name);
#endif /* defined (WINDOWS) */
		draw_scrolling_background(page_window);
#if defined (OLD_CODE)
#if defined (WINDOWS)
		InvalidateRect(page_window->scrolling_area,(CONST RECT *)NULL,FALSE);
#endif /* defined (WINDOWS) */
#endif /* defined (OLD_CODE) */
	}
#if defined (DEBUG)
	else
	{
		display_message(ERROR_MESSAGE,
"change_display_device_number.  Invalid argument(s).  page_window=%p.  device_number=%d",
			page_window,device_number);
	}
#endif /* defined (DEBUG) */
	LEAVE;
} /* change_display_device_number */

static int update_display_device(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 10 November 2000

DESCRIPTION :
Reads the string in the channel field of the <page_window> and updates the
scrolling display.  Returns 1 if it is able to update, otherwise it returns 0
==============================================================================*/
{
	char *working_string;
	int device_number,i,number_of_devices,return_code;
	struct Device **device_address,*display_device;
#if defined (WINDOWS)
	int working_string_length;
#endif /* defined (WINDOWS) */

	ENTER(update_display_device);
	if (page_window)
	{
		if ((page_window->display_device)&&(page_window->rig_address)&&
			(*(page_window->rig_address)))
		{
#if defined (MOTIF)
			working_string=(char *)NULL;
			XtVaGetValues((page_window->electrode).value,XmNvalue,&working_string,
				NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			working_string_length=Edit_GetTextLength((page_window->electrode).edit)+1;
				/*???DB.  GetWindowTextLength seems to give 1 less than the number of
					characters */
			if (ALLOCATE(working_string,char,working_string_length+1))
			{
				Edit_GetText((page_window->electrode).edit,working_string,
					working_string_length);
#endif /* defined (WINDOWS) */
				/* set the display device */
				display_device=(struct Device *)NULL;
				device_address=(*(page_window->rig_address))->devices;
				number_of_devices=(*(page_window->rig_address))->number_of_devices;
				for (i=0;i<number_of_devices;i++)
				{
					if ((*device_address)&&
#if defined (OLD_CODE)
						((*device_address)->signal)&&
						((*device_address)->channel)&&
						(0<(*device_address)->channel->number)&&
#if defined (WINDOWS)
#if defined (MIRADA)
						((*device_address)->channel->number<=
						(int)page_window->number_of_channels)&&
#endif /* defined (MIRADA) */
#endif /* defined (WINDOWS) */
#endif /* defined (OLD_CODE) */
						((*device_address)->description)&&
						(0==strcmp(working_string,(*device_address)->description->name)))
					{
						display_device= *device_address;
						device_number=i;
					}
					device_address++;
				}
				if (display_device)
				{
					if (display_device!=page_window->display_device)
					{
						change_display_device_number(page_window,device_number);
					}
				}
				else
				{
#if defined (MOTIF)
					XtVaSetValues((page_window->electrode).value,XmNvalue,
						page_window->display_device->description->name,NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
					Edit_SetText((page_window->electrode).edit,
						page_window->display_device->description->name);
#endif /* defined (WINDOWS) */
				}
#if defined (OLD_CODE)
				if (channel_number!=page_window->channel_number)
				{
#if defined (WINDOWS)
#if defined (MIRADA)
					if (PCI_SUCCESSFUL==get_mirada_information(page_window->device_driver,
						&number_of_cards,&bus,&device_function,&number_of_channels,
						&number_of_samples,&mirada_buffer))
#endif /* defined (MIRADA) */
					{
						if ((0<channel_number)&&(channel_number<=(int)number_of_channels))
						{
#endif /* defined (WINDOWS) */
							page_window->channel_number=channel_number;
							channel_number--;
							page_window->channel_index=16*(channel_number/16)+
								8*(channel_number%2)+(channel_number%16)/2;
#if defined (OLD_CODE)
							page_window->signal_display_maximum=0;
							page_window->signal_display_minimum=1;
#endif /* defined (OLD_CODE) */
#if defined (WINDOWS)
							InvalidateRect(page_window->scrolling_area,(CONST RECT *)NULL,
								FALSE);
						}
					}
#endif /* defined (WINDOWS) */
				}
#endif /* defined (OLD_CODE) */
#if defined (WINDOWS)
				DEALLOCATE(working_string);
			}
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
			XtFree(working_string);
#endif /* defined (MOTIF) */
		}
		else
		{
#if defined (MOTIF)
			XtVaSetValues((page_window->electrode).value,XmNvalue," ",NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			Edit_SetText((page_window->electrode).edit," ");
#endif /* defined (WINDOWS) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"update_display_device.  Missing page_window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* update_display_device */

#if defined (MOTIF)
static void update_display_device_callback(Widget *widget_id,
	XtPointer page_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Motif wrapper for update_display_device.
==============================================================================*/
{
	XmAnyCallbackStruct *text_data;

	ENTER(update_display_device_callback);
	USE_PARAMETER(widget_id);
	if ((text_data=(XmAnyCallbackStruct *)call_data)&&
		((XmCR_ACTIVATE==text_data->reason)||
		(XmCR_LOSING_FOCUS==text_data->reason)))
	{
		update_display_device((struct Page_window *)page_window);
	}
	LEAVE;
} /* update_display_device_callback */
#endif /* defined (MOTIF) */

#if defined (OLD_CODE)
static int update_stimulate_device(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 8 July 1999

DESCRIPTION :
Reads the string in the stimulate electrode field of the <page_window> and
updates the stimulating device.  Returns 1 if it is able to update, otherwise it
returns 0
==============================================================================*/
{
	char *working_string;
	int device_number,i,number_of_devices,return_code;
	struct Device **device_address,*stimulate_device;
#if defined (WINDOWS)
	int working_string_length;
#endif /* defined (WINDOWS) */

	ENTER(update_stimulate_device);
	if (page_window)
	{
		if ((page_window->stimulate_devices)&&((page_window->stimulate_devices)[
			(page_window->stimulator_number)-1]))
		{
#if defined (MOTIF)
			working_string=(char *)NULL;
			XtVaGetValues((page_window->stimulator).stimulate.value,XmNvalue,
				&working_string,NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			working_string_length=
				Edit_GetTextLength((page_window->stimulate_channel).edit)+1;
				/*???DB.  GetWindowTextLength seems to give 1 less than the number of
					characters */
			if (ALLOCATE(working_string,char,working_string_length+1))
			{
				Edit_GetText((page_window->stimulate_channel).edit,working_string,
					working_string_length);
#endif /* defined (WINDOWS) */
				/* set the stimulating device (can't be an auxiliary device that is a
					linear combination */
				stimulate_device=(struct Device *)NULL;
				device_address=(*(page_window->rig_address))->devices;
				number_of_devices=(*(page_window->rig_address))->number_of_devices;
				for (i=0;i<number_of_devices;i++)
				{
					if ((*device_address)&&((*device_address)->signal)&&
						((*device_address)->channel)&&
						(0<(*device_address)->channel->number)&&
#if defined (WINDOWS)
#if defined (MIRADA)
						((*device_address)->channel->number<=
						(int)page_window->number_of_channels)&&
#endif /* defined (MIRADA) */
#endif /* defined (WINDOWS) */
						((*device_address)->description)&&
						(unemap_channel_valid_for_stimulator(page_window->stimulator_number,
						(*device_address)->channel->number))&&
						(0==strcmp(working_string,(*device_address)->description->name)))
					{
						stimulate_device= *device_address;
						device_number=i;
					}
					device_address++;
				}
				if (stimulate_device)
				{
					if (stimulate_device!=(page_window->stimulate_devices)[
						(page_window->stimulator_number)-1])
					{
						(page_window->stimulate_devices)[
							(page_window->stimulator_number)-1]=stimulate_device;
						(page_window->stimulate_device_numbers)[
							(page_window->stimulator_number)-1]=device_number;
#if defined (MOTIF)
						XtVaSetValues((page_window->stimulator).stimulate.value,XmNvalue,
							stimulate_device->description->name,NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
						Edit_SetText((page_window->stimulate_channel).edit,
							stimulate_device->description->name);
#endif /* defined (WINDOWS) */
					}
				}
				else
				{
#if defined (MOTIF)
					XtVaSetValues((page_window->stimulator).stimulate.value,XmNvalue,
						((page_window->stimulate_devices)[
						(page_window->stimulator_number)-1])->description->name,NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
					Edit_SetText((page_window->stimulate_channel).edit,
						((page_window->stimulate_devices)[
						(page_window->stimulator_number)-1])->description->name);
#endif /* defined (WINDOWS) */
				}
#if defined (WINDOWS)
				DEALLOCATE(working_string);
			}
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
			XtFree(working_string);
#endif /* defined (MOTIF) */
		}
		else
		{
#if defined (MOTIF)
			XtVaSetValues((page_window->stimulator).stimulate.value,XmNvalue," ",
				NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			Edit_SetText((page_window->stimulate_channel).edit," ");
#endif /* defined (WINDOWS) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"update_stimulate_device.  Missing page_window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* update_stimulate_device */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
#if defined (MOTIF)
static void update_stimulate_device_callbac(Widget *widget_id,
	XtPointer page_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Motif wrapper for update_stimulate_device.
==============================================================================*/
{
	XmAnyCallbackStruct *text_data;

	ENTER(update_stimulate_device_callbac);
	USE_PARAMETER(widget_id);
	if ((text_data=(XmAnyCallbackStruct *)call_data)&&
		((XmCR_ACTIVATE==text_data->reason)||
		(XmCR_LOSING_FOCUS==text_data->reason)))
	{
		update_stimulate_device((struct Page_window *)page_window);
	}
	LEAVE;
} /* update_stimulate_device_callbac */
#endif /* defined (MOTIF) */
#endif /* defined (OLD_CODE) */

static int update_filter(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 4 June 1999

DESCRIPTION :
Reads the string in the filter field of the <page_window> and updates the
filter.  Returns 1 if it is able to update, otherwise it returns 0
==============================================================================*/
{
	char number_string[21],*working_string;
	float temp;
	int return_code;
#if defined (WINDOWS)
	int working_string_length;
#endif /* defined (WINDOWS) */

	ENTER(update_filter);
	return_code=0;
	if (page_window)
	{
#if defined (MOTIF)
		working_string=(char *)NULL;
		XtVaGetValues((page_window->low_pass).value,XmNvalue,&working_string,NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		working_string_length=
			Edit_GetTextLength((page_window->low_pass_filter).edit)+1;
			/*???DB.  GetWindowTextLength seems to give 1 less than the number of
				characters */
		if (ALLOCATE(working_string,char,working_string_length+1))
		{
			Edit_GetText((page_window->low_pass_filter).edit,working_string,
				working_string_length);
#endif /* defined (WINDOWS) */
			if (1==sscanf(working_string,"%f",&temp))
			{
				unemap_set_antialiasing_filter_frequency(0,temp);
				unemap_get_antialiasing_filter_frequency(1,&temp);
				sprintf(number_string,"%.0f",temp);
#if defined (MOTIF)
				XtVaSetValues((page_window->low_pass).value,XmNvalue,number_string,
					NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				Edit_SetText((page_window->low_pass_filter).edit,number_string);
#endif /* defined (WINDOWS) */
				return_code=1;
			}
#if defined (WINDOWS)
			DEALLOCATE(working_string);
		}
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
		XtFree(working_string);
#endif /* defined (MOTIF) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"update_filter.  Missing page_window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* update_filter */

#if defined (MOTIF)
static void update_filter_callback(Widget *widget_id,
	XtPointer page_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Motif wrapper for update_filter callback.
==============================================================================*/
{
	XmAnyCallbackStruct *text_data;

	ENTER(update_filter_callback);
	USE_PARAMETER(widget_id);
	if ((text_data=(XmAnyCallbackStruct *)call_data)&&
		((XmCR_ACTIVATE==text_data->reason)||
		(XmCR_LOSING_FOCUS==text_data->reason)))
	{
		update_filter((struct Page_window *)page_window);
	}
	LEAVE;
} /* update_filter_callback */
#endif /* defined (MOTIF) */

static int update_save_number_of_samples(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 4 June 1999

DESCRIPTION :
Reads the string in the filter field of the <page_window> and updates the
filter.  Returns 1 if it is able to update, otherwise it returns 0
==============================================================================*/
{
	char number_string[21],*working_string;
	int return_code,temp;
#if defined (WINDOWS)
	int working_string_length;
#endif /* defined (WINDOWS) */

	ENTER(update_save_number_of_samples);
	return_code=0;
	if (page_window)
	{
#if defined (MOTIF)
		working_string=(char *)NULL;
		XtVaGetValues((page_window->save).value,XmNvalue,&working_string,NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		working_string_length=
			Edit_GetTextLength((page_window->save).edit)+1;
			/*???DB.  GetWindowTextLength seems to give 1 less than the number of
				characters */
		if (ALLOCATE(working_string,char,working_string_length+1))
		{
			Edit_GetText((page_window->save).edit,working_string,
				working_string_length);
#endif /* defined (WINDOWS) */
			if (1==sscanf(working_string,"%d",&temp))
			{
				if (temp<=0)
				{
					temp=(int)(page_window->number_of_samples);
				}
				else
				{
					if (temp>(int)(page_window->number_of_samples))
					{
						temp=(int)(page_window->number_of_samples);
					}
				}
				page_window->number_of_samples_to_save=(unsigned long)temp;
				sprintf(number_string,"%d",temp);
#if defined (MOTIF)
				XtVaSetValues((page_window->save).value,XmNvalue,number_string,
					NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				Edit_SetText((page_window->save).edit,number_string);
#endif /* defined (WINDOWS) */
				return_code=1;
			}
#if defined (WINDOWS)
			DEALLOCATE(working_string);
		}
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
		XtFree(working_string);
#endif /* defined (MOTIF) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"update_save_number_of_samples.  Missing page_window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* update_save_number_of_samples */

#if defined (MOTIF)
static void update_save_number_of_samples_c(Widget *widget_id,
	XtPointer page_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Motif wrapper for update_save_number_of_samples callback.
==============================================================================*/
{
	XmAnyCallbackStruct *text_data;

	ENTER(update_save_number_of_samples_c);
	USE_PARAMETER(widget_id);
	if ((text_data=(XmAnyCallbackStruct *)call_data)&&
		((XmCR_ACTIVATE==text_data->reason)||
		(XmCR_LOSING_FOCUS==text_data->reason)))
	{
		update_save_number_of_samples((struct Page_window *)page_window);
	}
	LEAVE;
} /* update_save_number_of_samples_c */
#endif /* defined (MOTIF) */

static int read_waveform_file(char *waveform_file_name,int *constant_voltage,
	int *start_number_of_values,float *start_values_per_second,
	float **start_values,int *end_number_of_values,float *end_values_per_second,
	float **end_values)
/*******************************************************************************
LAST MODIFIED : 7 January 2000

DESCRIPTION :
Extends unemap_read_waveform_file to allow a starting waveform and an ending
waveform.
==============================================================================*/
{
	char keyword[6];
	FILE *waveform_file;
	int end_constant_voltage,return_code;

	ENTER(read_waveform_file);
	return_code=0;
	/* check arguments */
	if (waveform_file_name&&constant_voltage&&start_number_of_values&&
		start_values_per_second&&start_values&&end_number_of_values&&
		end_values_per_second&&end_values)
	{
		if (waveform_file=fopen(waveform_file_name,"r"))
		{
			if ((1==fscanf(waveform_file,"%5s",keyword))&&
				(('s'==keyword[0])||('S'==keyword[0]))&&
				(('t'==keyword[1])||('T'==keyword[1]))&&
				(('a'==keyword[2])||('A'==keyword[2]))&&
				(('r'==keyword[3])||('R'==keyword[3]))&&
				(('t'==keyword[4])||('T'==keyword[4])))
			{
				fscanf(waveform_file," ");
				if (return_code=unemap_read_waveform_file(waveform_file,(char *)NULL,
					start_number_of_values,start_values_per_second,start_values,
					constant_voltage))
				{
					if ((1==fscanf(waveform_file,"%5s",keyword))&&
						(('e'==keyword[0])||('E'==keyword[0]))&&
						(('n'==keyword[1])||('N'==keyword[1]))&&
						(('d'==keyword[2])||('D'==keyword[2])))
					{
						fscanf(waveform_file," ");
						if (return_code=unemap_read_waveform_file(waveform_file,
							(char *)NULL,end_number_of_values,end_values_per_second,
							end_values,&end_constant_voltage))
						{
							if ((!end_constant_voltage&&(*constant_voltage))||
								(!(*constant_voltage)&&end_constant_voltage))
							{
								display_message(ERROR_MESSAGE,
"Cannot mix constant current and constant voltage for start and end waveforms");
								return_code=0;
								DEALLOCATE(*start_values);
								DEALLOCATE(*end_values);
							}
						}
					}
					else
					{
						*end_number_of_values=0;
						*end_values_per_second=(float)0;
						*end_values=(float *)NULL;
					}
				}
			}
			else
			{
				rewind(waveform_file);
				return_code=unemap_read_waveform_file(waveform_file,(char *)NULL,
					start_number_of_values,start_values_per_second,start_values,
					constant_voltage);
				*end_number_of_values=0;
				*end_values_per_second=(float)0;
				*end_values=(float *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Could not open waveform file %s",
				waveform_file_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_waveform_file.  Invalid argument(s).  %p %p %p %p %p %p %p %p",
			waveform_file_name,constant_voltage,start_number_of_values,
			start_values_per_second,start_values,end_number_of_values,
			end_values_per_second,end_values);
	}
	LEAVE;

	return (return_code);
} /* read_waveform_file */

#if defined (OLD_CODE)
static int start_stimulating(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 2 July 2000

DESCRIPTION :
Called to start stimulating on the <page_window>.
==============================================================================*/
{
	char *hardware_directory,*waveform_file_name;
	float *values,values_per_second;
	int constant_voltage,number_of_values,return_code;

	ENTER(start_stimulating);
	return_code=0;
	if (page_window)
	{
		if ((page_window->stimulate_devices)&&((page_window->stimulate_devices)[
			(page_window->stimulator_number)-1]))
		{
			if (hardware_directory=getenv("UNEMAP_HARDWARE"))
			{
				if (ALLOCATE(waveform_file_name,char,strlen(hardware_directory)+15+
					(int)log10(page_window->stimulator_number)+1))
				{
					strcpy(waveform_file_name,hardware_directory);
#if defined (WIN32)
					if ('\\'!=waveform_file_name[strlen(waveform_file_name)-1])
					{
						strcat(waveform_file_name,"\\");
					}
#else /* defined (WIN32) */
					if ('/'!=waveform_file_name[strlen(waveform_file_name)-1])
					{
						strcat(waveform_file_name,"/");
					}
#endif /* defined (WIN32) */
				}
			}
			else
			{
				if (ALLOCATE(waveform_file_name,char,14+
					(int)log10(page_window->stimulator_number)+1))
				{
					waveform_file_name[0]='\0';
				}
			}
			if (waveform_file_name)
			{
				sprintf(waveform_file_name+strlen(waveform_file_name),
					"stimulate%d.wfm",page_window->stimulator_number);
				if (unemap_read_waveform_file((FILE *)NULL,waveform_file_name,
					&number_of_values,&values_per_second,&values,&constant_voltage))
				{
					/* set the channel to stimulating */
					unemap_set_channel_stimulating((page_window->stimulate_devices)[
						(page_window->stimulator_number)-1]->channel->number,1);
					if (constant_voltage)
					{
						return_code=unemap_load_voltage_stimulating(1,
							&(((page_window->stimulate_devices)[
							(page_window->stimulator_number)-1])->channel->number),
							number_of_values,values_per_second,values,
							(Unemap_stimulation_end_callback *)NULL,(void *)NULL);
#if defined (OLD_CODE)
						return_code=unemap_start_voltage_stimulating(
							((page_window->stimulate_devices)[
							(page_window->stimulator_number)-1])->channel->number,
							number_of_values,values_per_second,values);
#endif /* defined (OLD_CODE) */
					}
					else
					{
						if ((UnEmap_2V1==page_window->unemap_hardware_version)||
							(UnEmap_2V2==page_window->unemap_hardware_version)||
							(UnEmap_2V1|Unemap_2V2==page_window->unemap_hardware_version))
						{
							return_code=unemap_load_current_stimulating(1,
								&(((page_window->stimulate_devices)[
								(page_window->stimulator_number)-1])->channel->number),
								number_of_values,values_per_second,values,
								(Unemap_stimulation_end_callback *)NULL,(void *)NULL);
#if defined (OLD_CODE)
							return_code=unemap_start_current_stimulating(
								((page_window->stimulate_devices)[
								(page_window->stimulator_number)-1])->channel->number,
								number_of_values,values_per_second,values);
#endif /* defined (OLD_CODE) */
						}
						else
						{
							display_message(WARNING_MESSAGE,
						"Constant current stimulation is not available for this hardware");
							unemap_set_channel_stimulating((page_window->stimulate_devices)[
								(page_window->stimulator_number)-1]->channel->number,0);
							return_code=0;
						}
					}
					if (return_code)
					{
						return_code=unemap_start_stimulating();
					}
					DEALLOCATE(values);
				}
				DEALLOCATE(waveform_file_name);
			}
		}
	}
	if (return_code)
	{
		(page_window->stimulator_on)[(page_window->stimulator_number)-1]=1;
#if defined (MOTIF)
		XmToggleButtonSetState((page_window->stimulator).stimulate.checkbox,
			True,False);
		XtSetSensitive((page_window->stimulator).stimulate.value,False);
		XtSetSensitive((page_window->stimulator).stimulate.arrows,False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		CheckDlgButton(page_window->window,STIMULATE_CHECKBOX,BST_CHECKED);
		EnableWindow((page_window->stimulate_channel).edit,FALSE);
		EnableWindow((page_window->stimulate_channel).arrows,FALSE);
#endif /* defined (WINDOWS) */
	}
	else
	{
#if defined (MOTIF)
		XmToggleButtonSetState((page_window->stimulator).stimulate.checkbox,
			False,False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		CheckDlgButton(page_window->window,STIMULATE_CHECKBOX,BST_UNCHECKED);
#endif /* defined (WINDOWS) */
	}
	LEAVE;

	return (return_code);
} /* start_stimulating */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
static int stop_stimulating(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 16 July 1999

DESCRIPTION :
Called to stop stimulating on the <page_window>.
==============================================================================*/
{
	int return_code;
#if defined (OLD_CODE)
	int channel_number,stimulator_number;
#endif /* defined (OLD_CODE) */

	ENTER(stop_stimulating);
	return_code=1;
	if (page_window)
	{
#if defined (OLD_CODE)
		if ((0<(stimulator_number=page_window->stimulator_number))&&
			(page_window->stimulate_devices)[stimulator_number-1])
		{
			channel_number=(page_window->stimulate_devices)[stimulator_number-1]->
				channel->number;
			unemap_stop_stimulating(channel_number);
			unemap_set_channel_stimulating(channel_number,0);
			(page_window->stimulator_on)[stimulator_number-1]=0;
#if defined (OLD_CODE)
#if defined (MOTIF)
			XmToggleButtonSetState((page_window->stimulator).stimulate.checkbox,False,
				False);
			XtSetSensitive((page_window->stimulator).stimulate.value,True);
			XtSetSensitive((page_window->stimulator).stimulate.arrows,True);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			CheckDlgButton(page_window->window,STIMULATE_CHECKBOX,BST_UNCHECKED);
			EnableWindow((page_window->stimulate_channel).edit,TRUE);
			EnableWindow((page_window->stimulate_channel).arrows,TRUE);
#endif /* defined (WINDOWS) */
#endif /* defined (OLD_CODE) */
		}
#endif /* defined (OLD_CODE) */
	}
	LEAVE;

	return (return_code);
} /* stop_stimulating */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
#if defined (MOTIF)
static void start_stop_stimulating_callback(Widget widget,XtPointer page_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Motif wrapper for start_stimulating and stop_stimulating.
==============================================================================*/
{
	Boolean status;

	ENTER(start_stop_stimulating_callback);
	USE_PARAMETER(call_data);
	XtVaGetValues(widget,XmNset,&status,NULL);
	if (True==status)
	{
		start_stimulating((struct Page_window *)page_window);
	}
	else
	{
		stop_stimulating((struct Page_window *)page_window);
	}
	LEAVE;
} /* start_stop_stimulating_callback */
#endif /* defined (MOTIF) */
#endif /* defined (OLD_CODE) */

static int stop_all_stimulating(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 7 January 2001

DESCRIPTION :
Called to stop all stimulating on the <page_window>.
==============================================================================*/
{
	int i,return_code;

	ENTER(stop_all_stimulating);
	return_code=1;
	if (page_window)
	{
		unemap_stop_stimulating(0);
		unemap_set_channel_stimulating(0,0);
		for (i=0;i<page_window->number_of_stimulators;i++)
		{
			((page_window->stimulators)[i]).on=0;
		}
		update_stimulating_settings(page_window);
	}
	LEAVE;

	return (return_code);
} /* stop_all_stimulating */

static int start_stimulator(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 7 January 2001

DESCRIPTION :
Called to start a stimulator.
==============================================================================*/
{
	char *hardware_directory,*waveform_file_name;
	float *values,values_per_second;
	int channel_number,constant_voltage,i,number_of_values,return_code,
		stimulator_number;
	unsigned int number_of_cycles;

	ENTER(start_stimulator);
	return_code=0;
	if (page_window&&(page_window->display_device)&&
		(page_window->display_device->channel))
	{
		stimulator_number=page_window->number_of_stimulators;
		channel_number=page_window->display_device->channel->number;
		while ((stimulator_number>0)&&!unemap_channel_valid_for_stimulator(
			stimulator_number,channel_number))
		{
			stimulator_number--;
		}
		stimulator_number--;
		if ((stimulator_number>=0)&&
			!(((page_window->stimulators)[stimulator_number]).on)&&
			(0<((page_window->stimulators)[stimulator_number]).number_of_channels))
		{
			if (hardware_directory=getenv("UNEMAP_HARDWARE"))
			{
				if (ALLOCATE(waveform_file_name,char,strlen(hardware_directory)+15+
					(int)log10(page_window->number_of_stimulators)+1))
				{
					strcpy(waveform_file_name,hardware_directory);
#if defined (WIN32)
					if ('\\'!=waveform_file_name[strlen(waveform_file_name)-1])
					{
						strcat(waveform_file_name,"\\");
					}
#else /* defined (WIN32) */
					if ('/'!=waveform_file_name[strlen(waveform_file_name)-1])
					{
						strcat(waveform_file_name,"/");
					}
#endif /* defined (WIN32) */
				}
			}
			else
			{
				if (ALLOCATE(waveform_file_name,char,14+
					(int)log10(page_window->number_of_stimulators)+1))
				{
					waveform_file_name[0]='\0';
				}
			}
			if (waveform_file_name)
			{
				sprintf(waveform_file_name+strlen(waveform_file_name),
					"stimulate%d.wfm",stimulator_number+1);
				if (read_waveform_file(waveform_file_name,&constant_voltage,
					&number_of_values,&values_per_second,&values,
					&(((page_window->stimulators)[stimulator_number]).
					end_number_of_values),
					&(((page_window->stimulators)[stimulator_number]).
					end_values_per_second),
					&(((page_window->stimulators)[stimulator_number]).end_values)))
				{
					((page_window->stimulators)[stimulator_number]).constant_voltage=
						constant_voltage;
					if (0<((page_window->stimulators)[stimulator_number]).
						end_number_of_values)
					{
						number_of_cycles=1;
					}
					else
					{
						number_of_cycles=0;
					}
					if (constant_voltage)
					{
						return_code=unemap_load_voltage_stimulating(
							((page_window->stimulators)[stimulator_number]).
							number_of_channels,
							((page_window->stimulators)[stimulator_number]).channel_numbers,
							number_of_values,values_per_second,values,number_of_cycles,
							(Unemap_stimulation_end_callback *)NULL,(void *)NULL);
					}
					else
					{
						if ((UnEmap_2V1==page_window->unemap_hardware_version)||
							(UnEmap_2V2==page_window->unemap_hardware_version)||
							((UnEmap_2V1|UnEmap_2V2)==page_window->unemap_hardware_version))
						{
							return_code=unemap_load_current_stimulating(
								((page_window->stimulators)[stimulator_number]).
								number_of_channels,
								((page_window->stimulators)[stimulator_number]).channel_numbers,
								number_of_values,values_per_second,values,number_of_cycles,
								(Unemap_stimulation_end_callback *)NULL,(void *)NULL);
						}
						else
						{
							display_message(WARNING_MESSAGE,
				"Constant current stimulation is not available for this hardware");
						}
					}
					DEALLOCATE(values);
				}
				if (return_code)
				{
					/* set the channels to stimulating */
					((page_window->stimulators)[stimulator_number]).on=1;
					for (i=0;i<((page_window->stimulators)[stimulator_number]).
						number_of_channels;i++)
					{
						unemap_set_channel_stimulating((((page_window->stimulators)[
							stimulator_number]).channel_numbers)[i],1);
					}
					return_code=unemap_start_stimulating();
				}
				DEALLOCATE(waveform_file_name);
			}
		}
		update_stimulating_settings(page_window);
	}
	LEAVE;

	return (return_code);
} /* start_stimulator */

static int stop_stimulator(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 7 January 2001

DESCRIPTION :
Called to stop a stimulator.
==============================================================================*/
{
	int channel_number,i,return_code,stimulator_number;

	ENTER(stop_stimulator);
	return_code=0;
	if (page_window&&(page_window->display_device)&&
		(page_window->display_device->channel))
	{
		stimulator_number=page_window->number_of_stimulators;
		channel_number=page_window->display_device->channel->number;
		while ((stimulator_number>0)&&!unemap_channel_valid_for_stimulator(
			stimulator_number,channel_number))
		{
			stimulator_number--;
		}
		stimulator_number--;
		if ((stimulator_number>=0)&&
			(((page_window->stimulators)[stimulator_number]).on)&&
			(0<((page_window->stimulators)[stimulator_number]).number_of_channels))
		{
			if (0<((page_window->stimulators)[stimulator_number]).
				end_number_of_values)
			{
				if (((page_window->stimulators)[stimulator_number]).constant_voltage)
				{
					return_code=unemap_load_voltage_stimulating(
						((page_window->stimulators)[stimulator_number]).number_of_channels,
						((page_window->stimulators)[stimulator_number]).channel_numbers,
						((page_window->stimulators)[stimulator_number]).
						end_number_of_values,
						((page_window->stimulators)[stimulator_number]).
						end_values_per_second,
						((page_window->stimulators)[stimulator_number]).end_values,1,
						(Unemap_stimulation_end_callback *)NULL,(void *)NULL);
				}
				else
				{
					if ((UnEmap_2V1==page_window->unemap_hardware_version)||
						(UnEmap_2V2==page_window->unemap_hardware_version)||
						((UnEmap_2V1|UnEmap_2V2)==page_window->unemap_hardware_version))
					{
						return_code=unemap_load_current_stimulating(
							((page_window->stimulators)[stimulator_number]).
							number_of_channels,
							((page_window->stimulators)[stimulator_number]).channel_numbers,
							((page_window->stimulators)[stimulator_number]).
							end_number_of_values,
							((page_window->stimulators)[stimulator_number]).
							end_values_per_second,
							((page_window->stimulators)[stimulator_number]).end_values,1,
							(Unemap_stimulation_end_callback *)NULL,(void *)NULL);
					}
					else
					{
						display_message(WARNING_MESSAGE,
			"Constant current stimulation is not available for this hardware");
						unemap_stop_stimulating(channel_number);
						return_code=0;
					}
				}
				if (return_code)
				{
					unemap_start_stimulating();
				}
				DEALLOCATE(((page_window->stimulators)[stimulator_number]).end_values);
				((page_window->stimulators)[stimulator_number]).end_number_of_values=0;
			}
			else
			{
				return_code=unemap_stop_stimulating(channel_number);
			}
			((page_window->stimulators)[stimulator_number]).on=0;
			for (i=0;i<((page_window->stimulators)[stimulator_number]).
				number_of_channels;i++)
			{
				unemap_set_channel_stimulating((((page_window->stimulators)[
					stimulator_number]).channel_numbers)[i],0);
			}
		}
		update_stimulating_settings(page_window);
	}
	LEAVE;

	return (return_code);
} /* stop_stimulator */

#if defined (MOTIF)
static void stimulator_start_stop_callback(Widget widget,XtPointer page_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Motif wrapper for start_stimulator and stop_stimulator.
==============================================================================*/
{
	Boolean status;

	ENTER(stimulator_start_stop_callback);
	USE_PARAMETER(call_data);
	XtVaGetValues(widget,XmNset,&status,NULL);
	if (True==status)
	{
		start_stimulator((struct Page_window *)page_window);
	}
	else
	{
		stop_stimulator((struct Page_window *)page_window);
	}
	LEAVE;
} /* stimulator_start_stop_callback */
#endif /* defined (MOTIF) */

static int start_sampling(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Called to start sampling on the <page_window>.
==============================================================================*/
{
	int check_data_saved,return_code;
#if defined (MOTIF)
	Boolean status;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	UINT checkbox_state;
#endif /* defined (WINDOWS) */

	ENTER(start_sampling);
	return_code=0;
	if (page_window)
	{
		return_code=1;
		check_data_saved=0;
		if (page_window->isolate_checkbox)
		{
#if defined (MOTIF)
			XtVaGetValues(page_window->isolate_checkbox,XmNset,&status,NULL);
			if (True==status)
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			checkbox_state=IsDlgButtonChecked(page_window->window,ISOLATE_CHECKBOX);
			if (BST_CHECKED==checkbox_state)
#endif /* defined (WINDOWS) */
			{
				if (page_window->test_checkbox)
				{
#if defined (MOTIF)
					XtVaGetValues(page_window->test_checkbox,XmNset,&status,NULL);
					if (False==status)
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
					checkbox_state=IsDlgButtonChecked(page_window->window,TEST_CHECKBOX);
					if (BST_UNCHECKED==checkbox_state)
#endif /* defined (WINDOWS) */
					{
						return_code=confirmation_question_yes_no("Continue ?",
							"In isolate mode without test signal.  Continue ?",
#if defined (MOTIF)
							(Widget)NULL,
#endif /* defined (MOTIF) */
							page_window->user_interface);
					}
				}
			}
			else
			{
				check_data_saved=1;
			}
		}
		else
		{
			check_data_saved=1;
		}
		if (check_data_saved&&!(page_window->data_saved))
		{
			return_code=confirmation_question_yes_no("Continue ?",
				"Previous data will be lost.  Continue ?",
#if defined (MOTIF)
				(Widget)NULL,
#endif /* defined (MOTIF) */
				page_window->user_interface);
		}
		if (return_code)
		{
#if defined (MOTIF)
			XmToggleButtonSetState(page_window->sample_checkbox,True,False);
			XtSetSensitive((page_window->save).button,False);
			XtSetSensitive(page_window->auto_range_button,True);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			CheckDlgButton(page_window->window,SAMPLE_CHECKBOX,BST_CHECKED);
			EnableWindow((page_window->save).button,FALSE);
			EnableWindow(page_window->auto_range_button,TRUE);
#endif /* defined (WINDOWS) */
			unemap_start_sampling();
		}
		else
		{
#if defined (MOTIF)
			XmToggleButtonSetState(page_window->sample_checkbox,False,False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			CheckDlgButton(page_window->window,SAMPLE_CHECKBOX,BST_UNCHECKED);
#endif /* defined (WINDOWS) */
		}
	}
	LEAVE;

	return (return_code);
} /* start_sampling */

static int stop_sampling(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 9 November 2000

DESCRIPTION :
Called to stop sampling on the <page_window>.
==============================================================================*/
{
	int return_code;
	unsigned long number_of_samples;

	ENTER(stop_sampling);
	return_code=0;
	if (page_window)
	{
		return_code=1;
		unemap_stop_sampling();
#if defined (MOTIF)
		XmToggleButtonSetState(page_window->sample_checkbox,False,False);
		XtSetSensitive(page_window->auto_range_button,False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		CheckDlgButton(page_window->window,SAMPLE_CHECKBOX,BST_UNCHECKED);
		EnableWindow(page_window->auto_range_button,FALSE);
#endif /* defined (WINDOWS) */
		if (unemap_get_number_of_samples_acquired(&number_of_samples)&&
			(0<number_of_samples))
		{
			page_window->data_saved=0;
#if defined (MOTIF)
			XtSetSensitive((page_window->save).button,True);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			EnableWindow((page_window->save).button,TRUE);
#endif /* defined (WINDOWS) */
		}
		else
		{
			page_window->data_saved=1;
#if defined (MOTIF)
			XtSetSensitive((page_window->save).button,False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			EnableWindow((page_window->save).button,FALSE);
#endif /* defined (WINDOWS) */
		}
	}
	LEAVE;

	return (return_code);
} /* stop_sampling */

#if defined (MOTIF)
static void start_stop_sampling_callback(Widget widget,XtPointer page_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Motif wrapper for start_sampling and stop_sampling.
==============================================================================*/
{
	Boolean status;

	ENTER(start_stop_sampling_callback);
	USE_PARAMETER(call_data);
	XtVaGetValues(widget,XmNset,&status,NULL);
	if (True==status)
	{
		start_sampling((struct Page_window *)page_window);
	}
	else
	{
		stop_sampling((struct Page_window *)page_window);
	}
	LEAVE;
} /* start_stop_sampling_callback */
#endif /* defined (MOTIF) */

#if defined (OLD_CODE)
static int update_stimulator(struct Page_window *page_window,
	int stimulator_number)
/*******************************************************************************
LAST MODIFIED : 12 July 1999

DESCRIPTION :
==============================================================================*/
{
	char working_string[21];
	int return_code;
#if defined (MOTIF)
	XmString working_xmstring;
#endif /* defined (MOTIF) */

	ENTER(update_stimulator);
	return_code=0;
	if (page_window)
	{
		if (0<page_window->number_of_stimulators)
		{
			if ((1<=stimulator_number)&&(stimulator_number<=page_window->
				number_of_stimulators))
			{
#if defined (OLD_CODE)
#if defined (MOTIF)
				XtSetSensitive((page_window->stimulator).form,True);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				EnableWindow((page_window->stimulator).arrows,TRUE);
				EnableWindow((page_window->stimulator).text,TRUE);
#endif /* defined (WINDOWS) */
#endif /* defined (OLD_CODE) */
				return_code=1;
#if defined (OLD_CODE)
				if (stimulator_number!=page_window->stimulator_number)
				{
					page_window->stimulator_number=stimulator_number;
					sprintf(working_string,"Stimulator %d",stimulator_number);
#if defined (OLD_CODE)
#if defined (MOTIF)
					working_xmstring=XmStringCreateSimple(working_string);
					XtVaSetValues((page_window->stimulator).label,XmNlabelString,
						working_xmstring,NULL);
					XmStringFree(working_xmstring);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
					SetWindowText((page_window->stimulator).text,working_string);
#endif /* defined (WINDOWS) */
#endif /* defined (OLD_CODE) */
					if ((page_window->stimulate_devices)[stimulator_number-1])
					{
#if defined (OLD_CODE)
#if defined (MOTIF)
						XtVaSetValues((page_window->stimulator).stimulate.value,XmNvalue,
							(page_window->stimulate_devices)[stimulator_number-1]->
							description->name,NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
						Edit_SetText((page_window->stimulate_channel).edit,(page_window->
							stimulate_devices)[stimulator_number-1]->description->name);
#endif /* defined (WINDOWS) */
#endif /* defined (OLD_CODE) */
					}
					else
					{
#if defined (OLD_CODE)
#if defined (MOTIF)
						XtVaSetValues((page_window->stimulator).stimulate.value,XmNvalue,"",
							NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
						Edit_SetText((page_window->stimulate_channel).edit,"");
#endif /* defined (WINDOWS) */
#endif /* defined (OLD_CODE) */
					}
					if ((page_window->stimulator_on)[stimulator_number-1])
					{
#if defined (OLD_CODE)
#if defined (MOTIF)
						XmToggleButtonSetState((page_window->stimulator).stimulate.checkbox,
							True,False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
						CheckDlgButton(page_window->window,STIMULATE_CHECKBOX,BST_CHECKED);
#endif /* defined (WINDOWS) */
#endif /* defined (OLD_CODE) */
					}
					else
					{
#if defined (OLD_CODE)
#if defined (MOTIF)
						XmToggleButtonSetState((page_window->stimulator).stimulate.checkbox,
							False,False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
						CheckDlgButton(page_window->window,STIMULATE_CHECKBOX,
							BST_UNCHECKED);
#endif /* defined (WINDOWS) */
#endif /* defined (OLD_CODE) */
					}
				}
				if ((page_window->stimulate_devices)[stimulator_number-1])
				{
#if defined (OLD_CODE)
#if defined (MOTIF)
					XtSetSensitive((page_window->stimulator).stimulate.form,True);
					XtSetSensitive((page_window->stimulator).stimulate.checkbox,True);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
					EnableWindow(page_window->stimulate_checkbox,TRUE);
#endif /* defined (WINDOWS) */
#endif /* defined (OLD_CODE) */
					if ((page_window->stimulator_on)[stimulator_number-1])
					{
#if defined (OLD_CODE)
#if defined (MOTIF)
						XtSetSensitive((page_window->stimulator).stimulate.value,False);
						XtSetSensitive((page_window->stimulator).stimulate.arrows,False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
						EnableWindow((page_window->stimulate_channel).edit,FALSE);
						EnableWindow((page_window->stimulate_channel).arrows,FALSE);
#endif /* defined (WINDOWS) */
#endif /* defined (OLD_CODE) */
					}
					else
					{
#if defined (OLD_CODE)
#if defined (MOTIF)
						XtSetSensitive((page_window->stimulator).stimulate.value,True);
						XtSetSensitive((page_window->stimulator).stimulate.arrows,True);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
						EnableWindow((page_window->stimulate_channel).edit,TRUE);
						EnableWindow((page_window->stimulate_channel).arrows,TRUE);
#endif /* defined (WINDOWS) */
#endif /* defined (OLD_CODE) */
					}
				}
				else
				{
#if defined (OLD_CODE)
#if defined (MOTIF)
					XtSetSensitive((page_window->stimulator).stimulate.form,False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
					EnableWindow(page_window->stimulate_checkbox,FALSE);
					EnableWindow((page_window->stimulate_channel).edit,FALSE);
					EnableWindow((page_window->stimulate_channel).arrows,FALSE);
#endif /* defined (WINDOWS) */
#endif /* defined (OLD_CODE) */
				}
#endif /* defined (OLD_CODE) */
			}
		}
		else
		{
#if defined (OLD_CODE)
#if defined (MOTIF)
			XtSetSensitive((page_window->stimulator).form,False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			EnableWindow((page_window->stimulator).arrows,FALSE);
			EnableWindow((page_window->stimulator).text,FALSE);
			EnableWindow(page_window->stimulate_checkbox,FALSE);
			EnableWindow((page_window->stimulate_channel).edit,FALSE);
			EnableWindow((page_window->stimulate_channel).arrows,FALSE);
#endif /* defined (WINDOWS) */
#endif /* defined (OLD_CODE) */
		}
	}
	LEAVE;

	return (return_code);
} /* update_stimulator */
#endif /* defined (OLD_CODE) */

static int start_testing(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 27 September 1999

DESCRIPTION :
Called to start testing on the <page_window>.
==============================================================================*/
{
	double two_pi;
	float calibrate_voltage[500],maximum_voltage,minimum_voltage,
		testing_frequency;
	int i,return_code;
	/*???debug */
	char *hardware_directory,*testing_file_name;
	FILE *testing_file;

	ENTER(start_testing);
	return_code=0;
	if (page_window&&(page_window->test_checkbox))
	{
		return_code=1;
		stop_all_stimulating(page_window);
		testing_frequency=(float)10;
		/*???debug */
		if (hardware_directory=getenv("UNEMAP_HARDWARE"))
		{
			if (ALLOCATE(testing_file_name,char,strlen(hardware_directory)+13))
			{
				strcpy(testing_file_name,hardware_directory);
#if defined (WIN32)
				if ('\\'!=testing_file_name[strlen(testing_file_name)-1])
				{
					strcat(testing_file_name,"\\");
				}
#else /* defined (WIN32) */
				if ('/'!=testing_file_name[strlen(testing_file_name)-1])
				{
					strcat(testing_file_name,"/");
				}
#endif /* defined (WIN32) */
			}
		}
		else
		{
			if (ALLOCATE(testing_file_name,char,12))
			{
				testing_file_name[0]='\0';
			}
		}
		if (testing_file_name)
		{
			strcat(testing_file_name,"testing.txt");
			if (testing_file=fopen(testing_file_name,"r"))
			{
				fscanf(testing_file," frequency = %f ",&testing_frequency);
				if (testing_frequency<(float)1)
				{
					testing_frequency=(float)1;
				}
				else
				{
					if (testing_frequency>(float)100)
					{
						testing_frequency=(float)100;
					}
				}
				fclose(testing_file);
			}
			DEALLOCATE(testing_file_name);
		}
		/* set up the test signal */
		unemap_get_voltage_range(1,&minimum_voltage,&maximum_voltage);
		maximum_voltage *= (float)0.5;
		two_pi=(double)8*atan((double)1);
		for (i=0;i<500;i++)
		{
			calibrate_voltage[i]=maximum_voltage*
				(float)sin(two_pi*(double)i/(double)500);
		}
		unemap_start_calibrating(0,500,testing_frequency*(float)500,
			calibrate_voltage);
#if defined (MOTIF)
		XmToggleButtonSetState(page_window->test_checkbox,True,False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		CheckDlgButton(page_window->window,TEST_CHECKBOX,BST_CHECKED);
#endif /* defined (WINDOWS) */
	}
	LEAVE;

	return (return_code);
} /* start_testing */

static int stop_testing(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 27 September 1999

DESCRIPTION :
Called to stop testing on the <page_window>.
==============================================================================*/
{
	int return_code;

	ENTER(stop_testing);
	return_code=1;
	unemap_stop_calibrating(0);
	if (page_window&&(page_window->test_checkbox))
	{
#if defined (MOTIF)
		XmToggleButtonSetState(page_window->test_checkbox,False,False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		CheckDlgButton(page_window->window,TEST_CHECKBOX,BST_UNCHECKED);
#endif /* defined (WINDOWS) */
	}
	LEAVE;

	return (return_code);
} /* stop_testing */

#if defined (MOTIF)
static void start_stop_testing_callback(Widget widget,XtPointer page_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Motif wrapper for start_testing and stop_testing.
==============================================================================*/
{
	Boolean status;

	ENTER(start_stop_testing_callback);
	USE_PARAMETER(call_data);
	XtVaGetValues(widget,XmNset,&status,NULL);
	if (True==status)
	{
		start_testing((struct Page_window *)page_window);
	}
	else
	{
		stop_testing((struct Page_window *)page_window);
	}
	LEAVE;
} /* start_stop_testing_callback */
#endif /* defined (MOTIF) */

static int start_isolating(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 14 October 2001

DESCRIPTION :
Called to start isolating on the <page_window>.
==============================================================================*/
{
	int return_code;

	ENTER(start_isolating);
	return_code=0;
	if (page_window)
	{
		return_code=1;
		unemap_set_isolate_record_mode(0,1);
		stop_sampling(page_window);
		stop_testing(page_window);
		stop_all_stimulating(page_window);
		page_window->data_saved=1;
#if defined (MOTIF)
		if (page_window->isolate_checkbox)
		{
			XmToggleButtonSetState(page_window->isolate_checkbox,True,False);
		}
		if (page_window->test_checkbox)
		{
			XtSetSensitive(page_window->test_checkbox,True);
		}
#if defined (OLD_CODE)
		XtSetSensitive((page_window->stimulator).form,False);
#endif /* defined (OLD_CODE) */
		if (page_window->calibrate_button)
		{
			XtSetSensitive(page_window->calibrate_button,True);
		}
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		if (page_window->isolate_checkbox)
		{
			CheckDlgButton(page_window->window,ISOLATE_CHECKBOX,BST_CHECKED);
		}
		if (page_window->test_checkbox)
		{
			EnableWindow(page_window->test_checkbox,TRUE);
		}
#if defined (OLD_CODE)
		EnableWindow((page_window->stimulator).arrows,FALSE);
		EnableWindow((page_window->stimulator).text,FALSE);
		EnableWindow(page_window->stimulate_checkbox,FALSE);
		EnableWindow((page_window->stimulate_channel).edit,FALSE);
		EnableWindow((page_window->stimulate_channel).arrows,FALSE);
#endif /* defined (OLD_CODE) */
		if (page_window->calibrate_button)
		{
			EnableWindow(page_window->calibrate_button,TRUE);
		}
#endif /* defined (WINDOWS) */
		if (page_window->pacing_button)
		{
#if defined (MOTIF)
			XtSetSensitive(page_window->pacing_button,False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			EnableWindow(page_window->pacing_button,FALSE);
#endif /* defined (WINDOWS) */
		}
		update_stimulating_settings(page_window);
	}
	LEAVE;

	return (return_code);
} /* start_isolating */

static int stop_isolating(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 14 October 2001

DESCRIPTION :
Called to stop isolating on the <page_window>.
==============================================================================*/
{
	int return_code;

	ENTER(stop_isolating);
	return_code=0;
	if (page_window)
	{
		return_code=1;
		unemap_set_isolate_record_mode(0,0);
		stop_sampling(page_window);
		stop_testing(page_window);
		stop_all_stimulating(page_window);
		page_window->data_saved=1;
#if defined (MOTIF)
		if (page_window->isolate_checkbox)
		{
			XmToggleButtonSetState(page_window->isolate_checkbox,False,False);
		}
		if (page_window->test_checkbox)
		{
			XtSetSensitive(page_window->test_checkbox,False);
		}
#if defined (OLD_CODE)
		XtSetSensitive((page_window->stimulator).form,True);
#endif /* defined (OLD_CODE) */
		if (page_window->calibrate_button)
		{
			XtSetSensitive(page_window->calibrate_button,False);
		}
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		if (page_window->isolate_checkbox)
		{
			CheckDlgButton(page_window->window,ISOLATE_CHECKBOX,BST_UNCHECKED);
		}
		if (page_window->test_checkbox)
		{
			EnableWindow(page_window->test_checkbox,FALSE);
		}
#if defined (OLD_CODE)
		EnableWindow((page_window->stimulator).arrows,TRUE);
		EnableWindow((page_window->stimulator).text,TRUE);
		EnableWindow(page_window->stimulate_checkbox,TRUE);
		EnableWindow((page_window->stimulate_channel).edit,TRUE);
		EnableWindow((page_window->stimulate_channel).arrows,TRUE);
#endif /* defined (OLD_CODE) */
		if (page_window->calibrate_button)
		{
			EnableWindow(page_window->calibrate_button,FALSE);
		}
#endif /* defined (WINDOWS) */
		if (page_window->pacing_button)
		{
#if defined (MOTIF)
			XtSetSensitive(page_window->pacing_button,True);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			EnableWindow(page_window->pacing_button,TRUE);
#endif /* defined (WINDOWS) */
		}
		update_stimulating_settings(page_window);
	}
	LEAVE;

	return (return_code);
} /* stop_isolating */

#if defined (MOTIF)
static void start_stop_isolating_callback(Widget widget,XtPointer page_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Motif wrapper for start_isolating and stop_isolating.
==============================================================================*/
{
	Boolean status;

	ENTER(start_stop_isolating_callback);
	USE_PARAMETER(call_data);
	XtVaGetValues(widget,XmNset,&status,NULL);
	if (True==status)
	{
		start_isolating((struct Page_window *)page_window);
	}
	else
	{
		stop_isolating((struct Page_window *)page_window);
	}
	LEAVE;
} /* start_stop_isolating_callback */
#endif /* defined (MOTIF) */

static int page_read_calibration_file(char *file_name,struct Rig *rig)
/*******************************************************************************
LAST MODIFIED : 29 July 2000

DESCRIPTION :
Assumes that the calibration file is normalized.
==============================================================================*/
{
	float maximum_voltage,minimum_voltage,post_filter_gain,pre_filter_gain;
	int channel_number,i,return_code;
	struct Device **device_address;

	ENTER(page_read_calibration_file);
	if (rig)
	{
		return_code=read_calibration_file(file_name,rig);
		device_address=rig->devices;
		for (i=rig->number_of_devices;i>0;i--)
		{
			if ((*device_address)&&((*device_address)->channel))
			{
				channel_number=(*device_address)->channel->number;
				if (0==return_code)
				{
					(*device_address)->channel->gain=(float)1;
					(*device_address)->channel->gain_correction=(float)1;
					(*device_address)->channel->offset=(float)0;
				}
				if (unemap_get_voltage_range(channel_number,&minimum_voltage,
					&maximum_voltage)&&unemap_get_gain(channel_number,
					&pre_filter_gain,&post_filter_gain))
				{
					/* also change from V to mV */
					(*device_address)->channel->gain *= (float)1000*pre_filter_gain*
						post_filter_gain*(maximum_voltage-minimum_voltage)/
						(float)(maximum_signal_value-minimum_signal_value);
					(*device_address)->channel->gain_correction=
						(*device_address)->channel->gain;
				}
			}
			device_address++;
		}
	}
	LEAVE;

	return (return_code);
} /* page_read_calibration_file */

static int start_experiment(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 28 March 2002

DESCRIPTION :
Called to start experiment on the <page_window>.
???DB.  .cnfg in confirmation_get_read_filename and configuration directory
???DB.  Ghost read calibration in mapping
???DB.  Set display device etc for new rig
???DB.  Experiment directory (as for UNIMA)
???DB.  Mapping window change scrolling device
==============================================================================*/
{
	char *acquisition_rig_filename,*calibration_file_name;
	float channel_gain,channel_offset,post_filter_gain,pre_filter_gain;
#if defined (OLD_CODE)
	int stimulator_number;
#endif /* defined (OLD_CODE) */
	int channel_number,device_number,*electrodes_in_row,i,index,j,
		number_of_devices,number_of_rows,return_code;
	struct Auxiliary_properties *auxiliary_properties;
	struct Channel *channel;
	struct Device **device_address,*display_device;
	struct Signal_buffer *signal_buffer;
#if !defined (MIRADA)
	char working_string[21];
	float filter_frequency;
#endif /* !defined (MIRADA) */

	ENTER(start_experiment);
#if defined (DEBUG)
	/*???debug */
	printf("enter start_experiment\n");
#endif /* defined (DEBUG) */
	return_code=0;
	if (page_window&&(page_window->rig_address))
	{
		if ((UnEmap_1V2==page_window->unemap_hardware_version)||
			(UnEmap_2V1==page_window->unemap_hardware_version)||
			(UnEmap_2V2==page_window->unemap_hardware_version)||
			((UnEmap_2V1|UnEmap_2V2)==page_window->unemap_hardware_version))
		{
			if (page_window->hardware_initialized)
			{
				return_code=1;
			}
			else
			{
				/* initialize the hardware */
#if defined (MIRADA)
				/* start the interrupting */
				if (INVALID_HANDLE_VALUE!=page_window->device_driver)
				{
					start_interrupting(page_window->device_driver,
						page_window->scrolling_area,WM_USER,(LPARAM)page_window);
					return_code=1;
				}
#else /* defined (MIRADA) */
#if defined (DEBUG)
				/*???debug */
				printf("before unemap_deconfigure\n");
#endif /* defined (DEBUG) */
				/* make sure that unemap closed down */
				unemap_deconfigure();
#if defined (DEBUG)
				/*???debug */
				printf("after unemap_deconfigure\n");
#endif /* defined (DEBUG) */
				if (unemap_configure(page_window->sampling_frequency,
					(int)(page_window->number_of_samples),
#if defined (MOTIF)
					page_window->user_interface->application_context,
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
					(HWND)NULL,(UINT)0,
#endif /* defined (WINDOWS) */
					scrolling_hardware_callback,(void *)page_window,(float)25,
					page_window->synchronization_card))
				{
#if defined (DEBUG)
					/*???debug */
					printf("after unemap_configure\n");
#endif /* defined (DEBUG) */
					return_code=1;
					unemap_get_sampling_frequency(
						&(page_window->sampling_frequency));
					unemap_get_maximum_number_of_samples(
						&(page_window->number_of_samples));
					if ((page_window->number_of_samples_to_save<=0)||
						(page_window->number_of_samples_to_save>
						page_window->number_of_samples))
					{
						page_window->number_of_samples_to_save=
							page_window->number_of_samples;
					}
					unemap_start_scrolling();
					unemap_get_antialiasing_filter_frequency(1,&filter_frequency);
					sprintf(working_string,"%.0f",filter_frequency);
#if defined (MOTIF)
					XtVaSetValues((page_window->low_pass).value,XmNvalue,working_string,
						NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
					Edit_SetText((page_window->low_pass_filter).edit,working_string);
#endif /* defined (WINDOWS) */
					sprintf(working_string,"%d",
						(int)(page_window->number_of_samples_to_save));
#if defined (MOTIF)
					XtVaSetValues((page_window->save).value,XmNvalue,working_string,
						NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
					Edit_SetText((page_window->save).edit,working_string);
#endif /* defined (WINDOWS) */
				}
				else
				{
					display_message(ERROR_MESSAGE,"Could not configure hardware");
					return_code=0;
				}
#endif /* defined (MIRADA) */
			}
			if (return_code)
			{
#if defined (NEW_CODE)
				/*???DB.  Motif isn't picking up the current directory ? */
				confirmation_change_current_working_directory(
					page_window->user_interface);
#endif /* defined (NEW_CODE) */
				/* make sure that there is a rig */
				if (!(*(page_window->rig_address)))
				{
#if defined (OLD_CODE)
					display_message(WARNING_MESSAGE,
					"No rig defined.  Specify configuration file or cancel for default");
#endif /* defined (OLD_CODE) */
					if (acquisition_rig_filename=confirmation_get_read_filename(".cnfg",
						page_window->user_interface))
					{
						read_configuration_file(acquisition_rig_filename,
							page_window->rig_address
#if defined (UNEMAP_USE_3D)
							/*??JW perhaps should pass down from somewhere? */
							,(struct Unemap_package *)NULL
#endif /* defined (UNEMAP_USE_3D) */
							);
					}
					if (!(*(page_window->rig_address)))
					{
						display_message(INFORMATION_MESSAGE,"Creating default rig\n");
						number_of_rows=((int)(page_window->number_of_channels)-1)/8+1;
						if (ALLOCATE(electrodes_in_row,int,number_of_rows))
						{
							index=number_of_rows-1;
							electrodes_in_row[index]=
								(int)(page_window->number_of_channels)-8*index;
							while (index>0)
							{
								index--;
								electrodes_in_row[index]=8;
							}
							if (!(*(page_window->rig_address)=create_standard_Rig("default",
								PATCH,MONITORING_OFF,EXPERIMENT_OFF,number_of_rows,
								electrodes_in_row,1,0,(float)1
#if defined (UNEMAP_USE_3D)
								/*??JWperhaps we should pass this down from above*/
								,(struct Unemap_package *)NULL
#endif /* defined (UNEMAP_USE_3D) */
								)))
							{
								display_message(ERROR_MESSAGE,
									"start_experiment.  Error creating default rig");
							}
							DEALLOCATE(electrodes_in_row);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"start_experiment.  Could not allocate electrodes_in_row");
						}
					}
					if (*(page_window->rig_address))
					{
#if defined (DEBUG)
						/*???debug */
						printf("before page_window_set_gain\n");
#endif /* defined (DEBUG) */
						page_window_set_gain(page_window,0,page_window->initial_gain);
						/* set the display device (deliberately excluding auxiliary devices
							that are linear combinations */
						display_device=(struct Device *)NULL;
#if defined (OLD_CODE)
						page_window->display_maximum=(float)1;
						page_window->display_minimum=(float)-1;
#endif /* defined (OLD_CODE) */
						for (j=0;j<page_window->number_of_stimulators;j++)
						{
#if defined (OLD_CODE)
							(page_window->stimulate_devices)[j]=(struct Device *)NULL;
							(page_window->stimulate_device_numbers)[j]= -1;
#endif /* defined (OLD_CODE) */
							((page_window->stimulators)[j]).on=0;
							DEALLOCATE(((page_window->stimulators)[j]).channel_numbers);
							((page_window->stimulators)[j]).number_of_channels=0;
						}
						if ((0<(number_of_devices=(*(page_window->rig_address))->
							number_of_devices))&&(device_address=
							(*(page_window->rig_address))->devices))
						{
							channel_number=(int)((page_window->number_of_channels)+1);
							for (i=0;i<number_of_devices;i++)
							{
								if ((*device_address)&&(channel=(*device_address)->channel)&&
									(0<channel->number)&&(channel->number<channel_number)&&
									((*device_address)->description)&&
									((*device_address)->description->name))
								{
									display_device= *device_address;
									device_number=i;
									channel_number=channel->number;
								}
								device_address++;
							}
						}
						if (page_window->display_device=display_device)
						{
							page_window->display_device_number=device_number;
#if defined (OLD_CODE)
							if (!unemap_get_voltage_range(display_device->channel->number,
								&(page_window->display_minimum),
								&(page_window->display_maximum)))
							{
								page_window->display_minimum=(float)-1;
								page_window->display_maximum=(float)1;
							}
							page_window->display_minimum *= 1000;
							page_window->display_maximum *= 1000;
#endif /* defined (OLD_CODE) */
#if defined (OLD_CODE)
							channel_gain=display_device->channel->gain;
							channel_offset=display_device->channel->offset;
							if (unemap_get_gain(display_device->channel->number,
								&pre_filter_gain,&post_filter_gain))
							{
								channel_gain /= pre_filter_gain*post_filter_gain;
								page_window->display_maximum=
									channel_gain*((float)maximum_signal_value-channel_offset);
								page_window->display_minimum=
									channel_gain*((float)minimum_signal_value-channel_offset);
							}
#endif /* defined (OLD_CODE) */
#if defined (DEBUG)
							/*???debug */
							printf("before setting stimulate devices %d\n",
								page_window->number_of_stimulators);
#endif /* defined (DEBUG) */
							/* set the stimulate devices */
							device_address=(*(page_window->rig_address))->devices;
							for (i=0;i<number_of_devices;i++)
							{
								if ((*device_address)&&((*device_address)->channel)&&
									(0<(channel_number=(*device_address)->channel->number))&&
									(channel_number<=(int)(page_window->number_of_channels))&&
									((*device_address)->description)&&
									((*device_address)->description->name))
								{
									for (j=0;j<page_window->number_of_stimulators;j++)
									{
										if (unemap_channel_valid_for_stimulator(j+1,channel_number))
										{
#if defined (OLD_CODE)
											if ((-1==(page_window->stimulate_device_numbers)[j])||
												(channel_number<
												(page_window->stimulate_device_numbers)[j]))
											{
#if defined (DEBUG)
												/*???debug */
												printf("%d %d %d\n",channel_number,i,j);
#endif /* defined (DEBUG) */
												(page_window->stimulate_device_numbers)[j]=i;
												(page_window->stimulate_devices)[j]= *device_address;
												(page_window->stimulator_on)[j]=0;
											}
#endif /* defined (OLD_CODE) */
										}
									}
								}
								device_address++;
							}
#if defined (DEBUG)
							/*???debug */
							printf("after setting stimulate devices\n");
#endif /* defined (DEBUG) */
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"No device with a valid channel number in the rig");
							page_window->display_device_number= -1;
						}
					}
				}
				if (*(page_window->rig_address))
				{
					/* set up signal storage assuming that it doesn't exist (removed in
						stop_experiment) */
					/* set up the signal buffer */
					if (signal_buffer=create_Signal_buffer(SHORT_INT_VALUE,
						(int)(page_window->number_of_channels),
						(int)(page_window->number_of_samples),
						page_window->sampling_frequency))
					{
						/* set the times */
						for (index=0;index<(int)(page_window->number_of_samples);index++)
						{
							(signal_buffer->times)[index]=index;
						}
						device_address=(*(page_window->rig_address))->devices;
						i=(*(page_window->rig_address))->number_of_devices;
						return_code=1;
						while (return_code&&(i>0))
						{
							if ((*device_address)->channel)
							{
								channel_number=((*device_address)->channel->number)-1;
								if ((0<=channel_number)&&
									(channel_number<(int)(page_window->number_of_channels)))
								{
#if defined (MIRADA)
									index=16*(channel_number/16)+8*(channel_number%2)+
										(channel_number%16)/2;
#else /* defined (MIRADA) */
									index=channel_number;
#endif /* defined (MIRADA) */
									if (!((*device_address)->signal=create_Signal(index,
										signal_buffer,UNDECIDED,0)))
									{
										display_message(ERROR_MESSAGE,
											"start_experiment.  Could not create signal");
										return_code=0;
									}
								}
							}
							device_address++;
							i--;
						}
						if (!(page_window->hardware_initialized))
						{
							page_window->hardware_initialized=1;
							/* read in the calibration file */
							if ((page_window->calibration_directory)&&
								(0<strlen(page_window->calibration_directory)))
							{
								if (ALLOCATE(calibration_file_name,char,
									strlen(page_window->calibration_directory)+16))
								{
									strcpy(calibration_file_name,
										page_window->calibration_directory);
#if defined (WIN32)
									if ('\\'!=
										calibration_file_name[strlen(calibration_file_name)-1])
									{
										strcat(calibration_file_name,"\\");
									}
#else /* defined (WIN32) */
									if ('/'!=
										calibration_file_name[strlen(calibration_file_name)-1])
									{
										strcat(calibration_file_name,"/");
									}
#endif /* defined (WIN32) */
									strcat(calibration_file_name,"calibrate.dat");
								}
							}
							else
							{
								if (ALLOCATE(calibration_file_name,char,14))
								{
									strcpy(calibration_file_name,"calibrate.dat");
								}
							}
							if (calibration_file_name)
							{
								page_read_calibration_file(calibration_file_name,
									(void *)(*(page_window->rig_address)));
								DEALLOCATE(calibration_file_name);
							}
							else
							{
								display_message(ERROR_MESSAGE,
						"start_experiment.  Insufficient memory for calibration file name");
							}
							/* set the signal minimums and maximums */
							if ((0<(number_of_devices=(*(page_window->rig_address))->
								number_of_devices))&&(device_address=(*(page_window->
								rig_address))->devices))
							{
								for (i=0;i<number_of_devices;i++)
								{
									if ((*device_address)&&(channel=(*device_address)->channel))
									{
										if (unemap_get_gain(channel->number,&pre_filter_gain,
											&post_filter_gain))
										{
											channel_offset=channel->offset;
											channel_gain=(channel->gain_correction)/
												(pre_filter_gain*post_filter_gain);
											(*device_address)->signal_display_minimum=channel_gain*
												((float)minimum_signal_value-channel_offset);
											(*device_address)->signal_display_maximum=channel_gain*
												((float)maximum_signal_value-channel_offset);
										}
										else
										{
											(*device_address)->signal_display_minimum=(float)-1;
											(*device_address)->signal_display_maximum=(float)1;
										}
									}
									device_address++;
								}
								device_address=(*(page_window->rig_address))->devices;
								for (i=0;i<number_of_devices;i++)
								{
									if ((*device_address)&&!((*device_address)->channel))
									{
										(*device_address)->signal_display_minimum=(float)0;
										(*device_address)->signal_display_maximum=(float)0;
										auxiliary_properties= &(((*device_address)->description->
											properties).auxiliary);
										for (j=0;j<auxiliary_properties->number_of_electrodes;j++)
										{
											if (0<(auxiliary_properties->electrode_coefficients)[j])
											{
												(*device_address)->signal_display_minimum +=
													(auxiliary_properties->electrode_coefficients)[j]*
													((auxiliary_properties->electrodes)[j])->
													signal_display_minimum;
												(*device_address)->signal_display_maximum +=
													(auxiliary_properties->electrode_coefficients)[j]*
													((auxiliary_properties->electrodes)[j])->
													signal_display_maximum;
											}
											else
											{
												(*device_address)->signal_display_minimum +=
													(auxiliary_properties->electrode_coefficients)[j]*
													((auxiliary_properties->electrodes)[j])->
													signal_display_maximum;
												(*device_address)->signal_display_maximum +=
													(auxiliary_properties->electrode_coefficients)[j]*
													((auxiliary_properties->electrodes)[j])->
													signal_display_minimum;
											}
										}
									}
									device_address++;
								}
							}
							if (page_window->display_device)
							{
								add_scrolling_device(page_window,page_window->display_device);
								draw_scrolling_background(page_window);
								show_display_maximum(page_window);
								show_display_minimum(page_window);
								show_display_gain(page_window);
#if defined (MOTIF)
								XtVaSetValues((page_window->electrode).value,XmNvalue,
									page_window->display_device->description->name,NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
								Edit_SetText((page_window->electrode).edit,
									page_window->display_device->description->name);
#endif /* defined (WINDOWS) */
							}
#if defined (OLD_CODE)
							if ((0<page_window->number_of_stimulators)&&
								(page_window->stimulate_devices)[(page_window->
								stimulator_number)-1])
							{
#if defined (OLD_CODE)
#if defined (MOTIF)
								XtVaSetValues((page_window->stimulator).stimulate.value,
									XmNvalue,((page_window->stimulate_devices)[(page_window->
									stimulator_number)-1])->description->name,NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
								Edit_SetText((page_window->stimulate_channel).edit,
									((page_window->stimulate_devices)[(page_window->
									stimulator_number)-1])->description->name);
#endif /* defined (WINDOWS) */
#endif /* defined (OLD_CODE) */
							}
#endif /* defined (OLD_CODE) */
						}
						if (return_code)
						{
#if defined (MOTIF)
							XtSetSensitive(page_window->auto_range_button,False);
							if (page_window->calibrate_button)
							{
								XtSetSensitive(page_window->calibrate_button,True);
							}
							XtSetSensitive((page_window->electrode).form,True);
							XmToggleButtonSetState(page_window->experiment_checkbox,True,
								False);
							XtSetSensitive(page_window->full_range_button,True);
							XtSetSensitive((page_window->gain).form,True);
							if (page_window->isolate_checkbox)
							{
								XtSetSensitive(page_window->isolate_checkbox,True);
							}
							if ((page_window->low_pass).form)
							{
								XtSetSensitive((page_window->low_pass).form,True);
							}
							XtSetSensitive((page_window->maximum).form,True);
							XtSetSensitive((page_window->minimum).form,True);
							XtSetSensitive(page_window->sample_checkbox,True);
							XtSetSensitive((page_window->save).button,False);
							XtSetSensitive((page_window->save).value,True);
							XtSetSensitive(page_window->scrolling_checkbox,True);
							if (page_window->test_checkbox)
							{
								XtSetSensitive(page_window->test_checkbox,True);
							}
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
							EnableWindow(page_window->auto_range_button,FALSE);
							if (page_window->calibrate_button)
							{
								EnableWindow(page_window->calibrate_button,TRUE);
							}
							EnableWindow((page_window->electrode).edit,TRUE);
							EnableWindow((page_window->electrode).arrows,TRUE);
							EnableWindow((page_window->electrode).text,TRUE);
							CheckDlgButton(page_window->window,EXPERIMENT_CHECKBOX,
								BST_CHECKED);
							EnableWindow(page_window->full_range_button,TRUE);
							if (page_window->isolate_checkbox)
							{
								EnableWindow(page_window->isolate_checkbox,TRUE);
							}
							EnableWindow((page_window->gain).edit,TRUE);
							EnableWindow((page_window->gain).text,TRUE);
							if ((page_window->low_pass_filter).edit)
							{
								EnableWindow((page_window->low_pass_filter).edit,TRUE);
								EnableWindow((page_window->low_pass_filter).text,TRUE);
							}
							EnableWindow((page_window->maximum).edit,TRUE);
							EnableWindow((page_window->maximum).text,TRUE);
							EnableWindow((page_window->minimum).edit,TRUE);
							EnableWindow((page_window->minimum).text,TRUE);
							EnableWindow(page_window->sample_checkbox,TRUE);
							EnableWindow((page_window->save).button,FALSE);
							EnableWindow((page_window->save).edit,TRUE);
							EnableWindow(page_window->scrolling_checkbox,TRUE);
							if (page_window->test_checkbox)
							{
								EnableWindow(page_window->test_checkbox,TRUE);
							}
#endif /* defined (WINDOWS) */
							/* force update */
#if defined (OLD_CODE)
							stimulator_number=page_window->stimulator_number;
							page_window->stimulator_number=0;
							update_stimulator(page_window,stimulator_number);
#endif /* defined (OLD_CODE) */
							unemap_set_power(1);
							if (UnEmap_1V2==page_window->unemap_hardware_version)
							{
								stop_isolating(page_window);
							}
							else
							{
								start_isolating(page_window);
							}
							stop_sampling(page_window);
							stop_testing(page_window);
							page_window->data_saved=1;
						}
						else
						{
							i++;
							i=(*(page_window->rig_address))->number_of_devices-i;
							while (i>0)
							{
								device_address--;
								destroy_Signal(&((*device_address)->signal));
								i--;
							}
							destroy_Signal_buffer(&signal_buffer);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"start_experiment.  Could not create signal buffer");
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Invalid hardware");
		}
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave start_experiment\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* start_experiment */

static int stop_experiment(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 27 January 2002

DESCRIPTION :
Called to stop experiment on the <page_window>.
==============================================================================*/
{
	int i,return_code;
	struct Device **device;
	struct Signal_buffer *signal_buffer;

	ENTER(stop_experiment);
#if defined (DEBUG)
	/*???debug */
	printf("enter stop_experiment\n");
#endif /* defined (DEBUG) */
	return_code=0;
	if (page_window)
	{
		return_code=1;
		start_isolating(page_window);
		stop_sampling(page_window);
		stop_testing(page_window);
		stop_all_stimulating(page_window);
		unemap_set_power(0);
		/* clear the signals from the acquisition rig assuming that they have been
			set up by start_experiment */
		if ((page_window->rig_address)&&(*(page_window->rig_address))&&
			(device=(*(page_window->rig_address))->devices)&&(*device)&&
			((*device)->signal)&&(signal_buffer=(*device)->signal->buffer))
		{
			i=(*(page_window->rig_address))->number_of_devices;
			while (i>0)
			{
				destroy_Signal(&((*device)->signal));
				i--;
				device++;
			}
			destroy_Signal_buffer(&signal_buffer);
		}
#if defined (MOTIF)
		XtSetSensitive(page_window->auto_range_button,False);
		if (page_window->calibrate_button)
		{
			XtSetSensitive(page_window->calibrate_button,False);
		}
		XtSetSensitive((page_window->electrode).form,False);
		XmToggleButtonSetState(page_window->experiment_checkbox,False,False);
		XtSetSensitive(page_window->full_range_button,False);
		XtSetSensitive((page_window->gain).form,False);
		if (page_window->isolate_checkbox)
		{
			XtSetSensitive(page_window->isolate_checkbox,False);
		}
		if ((page_window->low_pass).form)
		{
			XtSetSensitive((page_window->low_pass).form,False);
		}
		XtSetSensitive((page_window->maximum).form,False);
		XtSetSensitive((page_window->minimum).form,False);
		XtSetSensitive(page_window->sample_checkbox,False);
		XtSetSensitive((page_window->save).button,False);
		XtSetSensitive((page_window->save).value,False);
		XtSetSensitive(page_window->scrolling_checkbox,False);
		if (page_window->test_checkbox)
		{
			XtSetSensitive(page_window->test_checkbox,False);
		}
#if defined (OLD_CODE)
		if ((page_window->stimulator).form)
		{
			XtSetSensitive((page_window->stimulator).form,False);
		}
		XtSetSensitive(page_window->reset_scale_button,False);
		XtSetSensitive(page_window->scale_button,False);
#endif /* defined (OLD_CODE) */
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		EnableWindow(page_window->auto_range_button,FALSE);
		if (page_window->calibrate_button)
		{
			EnableWindow(page_window->calibrate_button,FALSE);
		}
		EnableWindow((page_window->electrode).edit,FALSE);
		EnableWindow((page_window->electrode).arrows,FALSE);
		EnableWindow((page_window->electrode).text,FALSE);
		CheckDlgButton(page_window->window,EXPERIMENT_CHECKBOX,BST_UNCHECKED);
		EnableWindow(page_window->full_range_button,FALSE);
		EnableWindow((page_window->gain).edit,FALSE);
		EnableWindow((page_window->gain).text,FALSE);
		if (page_window->isolate_checkbox)
		{
			EnableWindow(page_window->isolate_checkbox,FALSE);
		}
		if ((page_window->low_pass_filter).edit)
		{
			EnableWindow((page_window->low_pass_filter).edit,FALSE);
			EnableWindow((page_window->low_pass_filter).text,FALSE);
		}
		EnableWindow((page_window->maximum).edit,FALSE);
		EnableWindow((page_window->maximum).text,FALSE);
		EnableWindow((page_window->minimum).edit,FALSE);
		EnableWindow((page_window->minimum).text,FALSE);
		EnableWindow(page_window->sample_checkbox,FALSE);
		EnableWindow((page_window->save).button,FALSE);
		EnableWindow((page_window->save).edit,FALSE);
		EnableWindow(page_window->scrolling_checkbox,FALSE);
		if (page_window->test_checkbox)
		{
			EnableWindow(page_window->test_checkbox,FALSE);
		}
#if defined (OLD_CODE)
		if (page_window->stimulate_checkbox)
		{
			EnableWindow((page_window->stimulator).arrows,FALSE);
			EnableWindow((page_window->stimulator).text,FALSE);
			EnableWindow(page_window->stimulate_checkbox,FALSE);
			EnableWindow((page_window->stimulate_channel).edit,FALSE);
			EnableWindow((page_window->stimulate_channel).arrows,FALSE);
		}
		EnableWindow(page_window->reset_scale_button,FALSE);
		EnableWindow(page_window->scale_button,FALSE);
#endif /* defined (OLD_CODE) */
#endif /* defined (WINDOWS) */
		update_stimulating_settings(page_window);
	}
	LEAVE;

	return (return_code);
} /* stop_experiment */

#if defined (MOTIF)
static void start_stop_experiment_callback(Widget widget,XtPointer page_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 16 December 2001

DESCRIPTION :
Motif wrapper for start_experiment and stop_experiment.
==============================================================================*/
{
	Boolean status;

	ENTER(start_stop_experiment_callback);
	USE_PARAMETER(call_data);
	XtVaGetValues(widget,XmNset,&status,NULL);
	if (True==status)
	{
		if (!start_experiment((struct Page_window *)page_window))
		{
			XtVaSetValues(widget,XmNset,False,NULL);
		}
	}
	else
	{
		stop_experiment((struct Page_window *)page_window);
	}
	LEAVE;
} /* start_stop_experiment_callback */
#endif /* defined (MOTIF) */

static int set_current_electrode_stimulate(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 11 November 2001

DESCRIPTION :
Sets the current electrode to stimulate.
==============================================================================*/
{
	int channel_number,i,j,k,return_code,*stimulating_channel_numbers;

	ENTER(set_current_electrode_stimulate);
	return_code=0;
	if (page_window)
	{
		if ((page_window->display_device)&&(page_window->display_device->channel))
		{
			channel_number=page_window->display_device->channel->number;
			/* find in the list of stimulating electrodes */
			i=page_window->number_of_stimulators;
			while ((i>0)&&!unemap_channel_valid_for_stimulator(i,channel_number))
			{
				i--;
			}
			if (i>0)
			{
				i--;
				stimulating_channel_numbers=
					((page_window->stimulators)[i]).channel_numbers;
				j=((page_window->stimulators)[i]).number_of_channels;
				if (stimulating_channel_numbers&&(j>0))
				{
					do
					{
						j--;
					} while ((j>0)&&(channel_number<stimulating_channel_numbers[j]));
				}
				if ((((page_window->stimulators)[i]).number_of_channels<=0)||
					(channel_number!=stimulating_channel_numbers[j]))
				{
					if (REALLOCATE(stimulating_channel_numbers,
						((page_window->stimulators)[i]).channel_numbers,int,
						((page_window->stimulators)[i]).number_of_channels+1))
					{
						if ((0<((page_window->stimulators)[i]).number_of_channels)&&
							(channel_number>stimulating_channel_numbers[j]))
						{
							j++;
						}
						((page_window->stimulators)[i]).channel_numbers=
							stimulating_channel_numbers;
						k=((page_window->stimulators)[i]).number_of_channels;
						while (k>j)
						{
							stimulating_channel_numbers[k]=stimulating_channel_numbers[k-1];
							k--;
						}
						stimulating_channel_numbers[j]=channel_number;
						(((page_window->stimulators)[i]).number_of_channels)++;
						return_code=1;
					}
				}
				else
				{
					return_code=1;
				}
			}
			update_stimulating_settings(page_window);
		}
	}
	LEAVE;

	return (return_code);
} /* set_current_electrode_stimulate */

static int set_current_electrode_record(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 7 January 2001

DESCRIPTION :
Sets the current electrode to record.
==============================================================================*/
{
	int channel_number,i,j,return_code,*stimulating_channel_numbers;

	ENTER(set_current_electrode_record);
	return_code=0;
	if (page_window)
	{
		if ((page_window->display_device)&&(page_window->display_device->channel))
		{
			channel_number=page_window->display_device->channel->number;
			/* find in the list of stimulating electrodes */
			i=page_window->number_of_stimulators;
			while ((i>0)&&!unemap_channel_valid_for_stimulator(i,channel_number))
			{
				i--;
			}
			if (i>0)
			{
				i--;
				if (0<(j=((page_window->stimulators)[i]).number_of_channels))
				{
					stimulating_channel_numbers=
						((page_window->stimulators)[i]).channel_numbers;
					do
					{
						j--;
					} while ((j>0)&&(channel_number<stimulating_channel_numbers[j]));
					if (channel_number==stimulating_channel_numbers[j])
					{
						(((page_window->stimulators)[i]).number_of_channels)--;
						while (j<((page_window->stimulators)[i]).number_of_channels)
						{
							stimulating_channel_numbers[j]=stimulating_channel_numbers[j+1];
							j++;
						}
					}
				}
			}
			update_stimulating_settings(page_window);
		}
	}
	LEAVE;

	return (return_code);
} /* set_current_electrode_record */

#if defined (MOTIF)
static void electrode_stimulate_record_call(Widget widget,XtPointer page_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Motif wrapper for toggling the current electrode between record and stimulate.
==============================================================================*/
{
	Boolean status;

	ENTER(electrode_stimulate_record_call);
	USE_PARAMETER(call_data);
	XtVaGetValues(widget,XmNset,&status,NULL);
	if (True==status)
	{
		set_current_electrode_stimulate((struct Page_window *)page_window);
	}
	else
	{
		set_current_electrode_record((struct Page_window *)page_window);
	}
	LEAVE;
} /* electrode_stimulate_record_call */
#endif /* defined (MOTIF) */

static void calibration_end_callback(const int number_of_channels,
	const int *channel_numbers,const float *channel_offsets,
	const float *channel_gains,void *page_window_void)
/*******************************************************************************
LAST MODIFIED : 13 November 2000

DESCRIPTION :
Called when calibration ends.
==============================================================================*/
{
	char *calibration_file_name;
	FILE *output_file;
	float maximum_voltage,minimum_voltage;
	int i;
	struct Page_window *page_window;
#if defined (WINDOWS)
	POINT point;
#endif /* defined (WINDOWS) */

	ENTER(calibration_end_callback)
	page_window=(struct Page_window *)page_window_void;
	if (0<number_of_channels)
	{
		/* write the calibration file */
		if ((page_window->calibration_directory)&&
			(0<strlen(page_window->calibration_directory)))
		{
			if (ALLOCATE(calibration_file_name,char,
				strlen(page_window->calibration_directory)+15))
			{
				strcpy(calibration_file_name,page_window->calibration_directory);
				strcat(calibration_file_name,"/calibrate.dat");
			}
		}
		else
		{
			if (ALLOCATE(calibration_file_name,char,14))
			{
				strcpy(calibration_file_name,"calibrate.dat");
			}
		}
		if (calibration_file_name)
		{
			if (output_file=fopen(calibration_file_name,"w"))
			{
				fprintf(output_file,"channel  offset  gain\n");
				for (i=0;i<number_of_channels;i++)
				{
					/* normalize gains */
						/*???DB.  Move normalization into hardware ? */
					unemap_get_voltage_range(i+1,&minimum_voltage,&maximum_voltage);
					fprintf(output_file,"%d %g %g\n",channel_numbers[i],
						channel_offsets[i],channel_gains[i]*(float)(maximum_signal_value-
						minimum_signal_value)/(maximum_voltage-minimum_voltage));
				}
				fclose(output_file);
				if (page_window&&(page_window->rig_address)&&
					(*(page_window->rig_address)))
				{
					/* update the acquisition rig */
					page_read_calibration_file(calibration_file_name,
						*(page_window->rig_address));
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"calibration_end_callback.  Could not write calibration file");
			}
			DEALLOCATE(calibration_file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
		"calibration_end_callback.  Insufficient memory for calibration file name");
		}
		/* set the display minimum and maximum (gains may be changed while creating
			scrolling_area) */
		if (page_window&&(page_window->display_device))
		{
			show_display_maximum(page_window);
			show_display_minimum(page_window);
			show_display_gain(page_window);
		}
	}
	else
	{
		display_message(WARNING_MESSAGE,"Calibration failed");
	}
	if (page_window)
	{
		unemap_start_scrolling();
#if defined (MOTIF)
		XtSetSensitive(page_window->auto_range_button,False);
		if (page_window->calibrate_button)
		{
			XtSetSensitive(page_window->calibrate_button,True);
		}
		XtSetSensitive((page_window->electrode).form,True);
		XtSetSensitive(page_window->experiment_checkbox,True);
		XtSetSensitive(page_window->full_range_button,True);
		XtSetSensitive((page_window->gain).form,True);
		if (page_window->isolate_checkbox)
		{
			XtSetSensitive(page_window->isolate_checkbox,True);
		}
		if ((page_window->low_pass).form)
		{
			XtSetSensitive((page_window->low_pass).form,True);
		}
		XtSetSensitive((page_window->maximum).form,True);
		XtSetSensitive((page_window->minimum).form,True);
		XtSetSensitive(page_window->sample_checkbox,True);
		XtSetSensitive((page_window->save).button,False);
		XtSetSensitive(page_window->scrolling_checkbox,True);
		if (page_window->test_checkbox)
		{
			XtSetSensitive(page_window->test_checkbox,True);
		}
		/* turn off the wait cursor */
			/*???DB.  To be done.  wait cursor */
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		EnableWindow(page_window->auto_range_button,FALSE);
		if (page_window->calibrate_button)
		{
			EnableWindow(page_window->calibrate_button,TRUE);
		}
		EnableWindow((page_window->electrode).edit,TRUE);
		EnableWindow((page_window->electrode).arrows,TRUE);
		EnableWindow((page_window->electrode).text,TRUE);
		EnableWindow(page_window->experiment_checkbox,TRUE);
		EnableWindow(page_window->full_range_button,TRUE);
		if (page_window->isolate_checkbox)
		{
			EnableWindow(page_window->isolate_checkbox,TRUE);
		}
		EnableWindow((page_window->gain).edit,TRUE);
		EnableWindow((page_window->gain).text,TRUE);
		if ((page_window->low_pass_filter).edit)
		{
			EnableWindow((page_window->low_pass_filter).edit,TRUE);
			EnableWindow((page_window->low_pass_filter).text,TRUE);
		}
		EnableWindow((page_window->maximum).edit,TRUE);
		EnableWindow((page_window->maximum).text,TRUE);
		EnableWindow((page_window->minimum).edit,TRUE);
		EnableWindow((page_window->minimum).text,TRUE);
		EnableWindow(page_window->sample_checkbox,TRUE);
		EnableWindow((page_window->save).button,FALSE);
		EnableWindow(page_window->scrolling_checkbox,TRUE);
		if (page_window->test_checkbox)
		{
			EnableWindow(page_window->test_checkbox,TRUE);
		}
		/* turn off the wait cursor */
		SetClassLong(page_window->window,GCL_HCURSOR,
			(LONG)LoadCursor(NULL,IDC_ARROW));
		SetClassLong(page_window->scrolling_area,GCL_HCURSOR,
			(LONG)LoadCursor(NULL,IDC_ARROW));
		/*???DB.  Only way I've found to make the pointer change without the user
			moving the mouse */
		GetCursorPos(&point);
		if (point.x>0)
		{
			SetCursorPos((int)(point.x-1),(int)(point.y));
		}
		else
		{
			SetCursorPos((int)(point.x+1),(int)(point.y));
		}
		SetCursorPos((int)(point.x),(int)(point.y));
#endif /* defined (WINDOWS) */
#if defined (OLD_CODE)
		update_stimulator(page_window,page_window->stimulator_number);
#endif /* defined (OLD_CODE) */
		update_stimulating_settings(page_window);
	}
	LEAVE;
} /* calibration_end_callback */

static void start_calibrating(
#if defined (MOTIF)
	Widget widget,XtPointer page_window_structure,XtPointer call_data
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	LONG page_window_structure
#endif /* defined (WINDOWS) */
	)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Called to start calibrating on the <page_window>.
==============================================================================*/
{
	struct Page_window *page_window;

	ENTER(start_calibrating);
#if defined (MOTIF)
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
#endif /* defined (MOTIF) */
	if (page_window=(struct Page_window *)page_window_structure)
	{
		stop_sampling(page_window);
		stop_testing(page_window);
		stop_all_stimulating(page_window);
		start_isolating(page_window);
		page_window->data_saved=1;
		unemap_stop_scrolling();
#if defined (MOTIF)
		XtSetSensitive(page_window->auto_range_button,False);
		if (page_window->calibrate_button)
		{
			XtSetSensitive(page_window->calibrate_button,False);
		}
		XtSetSensitive((page_window->electrode).form,False);
		XtSetSensitive(page_window->experiment_checkbox,False);
		XtSetSensitive(page_window->full_range_button,False);
		XtSetSensitive((page_window->gain).form,False);
		if (page_window->isolate_checkbox)
		{
			XtSetSensitive(page_window->isolate_checkbox,False);
		}
		if ((page_window->low_pass).form)
		{
			XtSetSensitive((page_window->low_pass).form,False);
		}
		XtSetSensitive((page_window->maximum).form,False);
		XtSetSensitive((page_window->minimum).form,False);
		XtSetSensitive(page_window->sample_checkbox,False);
		XtSetSensitive((page_window->save).button,False);
		XtSetSensitive(page_window->scrolling_checkbox,False);
		if (page_window->test_checkbox)
		{
			XtSetSensitive(page_window->test_checkbox,False);
		}
#if defined (OLD_CODE)
		if ((page_window->stimulator).form)
		{
			XtSetSensitive((page_window->stimulator).form,False);
		}
		XtSetSensitive(page_window->reset_scale_button,False);
		XtSetSensitive(page_window->scale_button,False);
#endif /* defined (OLD_CODE) */
		/* turn on the wait cursor */
			/*???DB.  To be done.  wait cursor */
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		EnableWindow(page_window->auto_range_button,FALSE);
		if (page_window->calibrate_button)
		{
			EnableWindow(page_window->calibrate_button,FALSE);
		}
		EnableWindow((page_window->electrode).edit,FALSE);
		EnableWindow((page_window->electrode).arrows,FALSE);
		EnableWindow((page_window->electrode).text,FALSE);
		EnableWindow(page_window->experiment_checkbox,FALSE);
		EnableWindow(page_window->full_range_button,FALSE);
		EnableWindow((page_window->gain).edit,FALSE);
		EnableWindow((page_window->gain).text,FALSE);
		if (page_window->isolate_checkbox)
		{
			EnableWindow(page_window->isolate_checkbox,FALSE);
		}
		if ((page_window->low_pass_filter).edit)
		{
			EnableWindow((page_window->low_pass_filter).edit,FALSE);
			EnableWindow((page_window->low_pass_filter).text,FALSE);
		}
		EnableWindow((page_window->maximum).edit,FALSE);
		EnableWindow((page_window->maximum).text,FALSE);
		EnableWindow((page_window->minimum).edit,FALSE);
		EnableWindow((page_window->minimum).text,FALSE);
		EnableWindow(page_window->sample_checkbox,FALSE);
		EnableWindow((page_window->save).button,FALSE);
		EnableWindow(page_window->scrolling_checkbox,FALSE);
		if (page_window->test_checkbox)
		{
			EnableWindow(page_window->test_checkbox,FALSE);
		}
#if defined (OLD_CODE)
		if (page_window->stimulate_checkbox)
		{
			EnableWindow((page_window->stimulator).arrows,FALSE);
			EnableWindow((page_window->stimulator).text,FALSE);
			EnableWindow(page_window->stimulate_checkbox,FALSE);
			EnableWindow((page_window->stimulate_channel).edit,FALSE);
			EnableWindow((page_window->stimulate_channel).arrows,FALSE);
		}
		EnableWindow(page_window->reset_scale_button,FALSE);
		EnableWindow(page_window->scale_button,FALSE);
#endif /* defined (OLD_CODE) */
		/* turn on the wait cursor */
		SetClassLong(page_window->window,GCL_HCURSOR,
			(LONG)LoadCursor(NULL,IDC_WAIT));
		SetClassLong(page_window->scrolling_area,GCL_HCURSOR,
			(LONG)LoadCursor(NULL,IDC_WAIT));
/*		SetCursor(LoadCursor(NULL,IDC_WAIT));*/
#endif /* defined (WINDOWS) */
		unemap_calibrate(calibration_end_callback,page_window);
	}
	LEAVE;
} /* start_calibrating */

static void page_save_data(
#if defined (MOTIF)
	Widget widget,XtPointer page_window_structure,XtPointer call_data
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	LONG page_window_structure
#endif /* defined (WINDOWS) */
	)
/*******************************************************************************
LAST MODIFIED : 13 January 2002

DESCRIPTION :
Called when the save button is pressed.
==============================================================================*/
{
#if !defined (BACKGROUND_SAVING)
	short int *destination,*source;
	struct Signal_buffer *signal_buffer;
	unsigned long number_of_samples;
#endif /* !defined (BACKGROUND_SAVING) */
	struct Device **device;
	struct Page_window *page_window;
	struct Rig *rig;
#if defined (MIRADA)
	int i,index;
	short int *mirada_buffer;
	unsigned char *bus,*device_function;
	unsigned long interrupt_count,mirada_start,number_of_cards,number_of_channels;
#else /* defined (MIRADA) */
#if !defined (BACKGROUND_SAVING)
	int channel_number,i,j,number_of_channels,number_of_signals;
	short int *samples;
#endif /* !defined (BACKGROUND_SAVING) */
#endif /* defined (MIRADA) */

	ENTER(page_save_data);
#if defined (MOTIF)
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
#endif /* defined (MOTIF) */
	if (page_window=(struct Page_window *)page_window_structure)
	{
		if ((page_window->rig_address)&&(rig= *(page_window->rig_address))&&
			(device=rig->devices))
		{
#if defined (MIRADA)
			if (PCI_SUCCESSFUL==stop_interrupting(page_window->device_driver,
				&interrupt_count,&mirada_start))
			{
				if (PCI_SUCCESSFUL==get_mirada_information(page_window->device_driver,
					&number_of_cards,&bus,&device_function,&number_of_channels,
					&number_of_samples,&mirada_buffer))
				{
					/* copy the buffer */
					i=rig->number_of_devices;
					signal_buffer=(struct Signal_buffer *)NULL;
					while ((i>0)&&!signal_buffer)
					{
						if ((*device)&&((*device)->signal))
						{
							signal_buffer=(*device)->signal->buffer;
						}
						device++;
						i--;
					}
					if (signal_buffer)
					{
						destination=(signal_buffer->signals).short_int_values;
						signal_buffer->start=0;
						if (interrupt_count>number_of_samples)
						{
							source=mirada_buffer+
								(number_of_channels*(int)mirada_start);
							for (index=number_of_channels*
								(number_of_samples-(int)mirada_start);index>0;
								index--)
							{
								*destination= *source;
								destination++;
								source++;
							}
							signal_buffer->end=(int)number_of_samples-1;
						}
						else
						{
							signal_buffer->end=(int)mirada_start-1;
						}
						source=mirada_buffer;
						for (index=number_of_channels*(int)mirada_start;index>0;
							index--)
						{
							*destination= *source;
							destination++;
							source++;
						}
						/* write out the signal file */
						open_file_and_write(
#if defined (MOTIF)
							(Widget)NULL,(XtPointer)(page_window->save_file_open_data),
							(XtPointer)NULL
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
							page_window->save_file_open_data
#endif /* defined (WINDOWS) */
							);
					}
				}
#if defined (MOTIF)
				/*???DB.  To be done.  May never get Mirada running under unix */
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				InvalidateRect(page_window->scrolling_area,(CONST RECT *)NULL,FALSE);
#endif /* defined (WINDOWS) */
				start_interrupting(page_window->device_driver,
					page_window->scrolling_area,WM_USER,(LPARAM)page_window);
			}
#else /* defined (MIRADA) */
			USE_PARAMETER(device);
#if defined (BACKGROUND_SAVING)
			/* write out the signal file */
			open_file_and_write(
#if defined (MOTIF)
				(Widget)NULL,(XtPointer)(page_window->save_file_open_data),
				(XtPointer)NULL
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				page_window->save_file_open_data
#endif /* defined (WINDOWS) */
				);
#else /* defined (BACKGROUND_SAVING) */
			/* stop continuous sampling */
			unemap_stop_sampling();
			samples=(short int *)NULL;
			i=rig->number_of_devices;
			if (unemap_get_number_of_samples_acquired(&number_of_samples)
			{
				if (number_of_samples>page_window->number_of_samples_to_save)
				{
					number_of_samples=page_window->number_of_samples_to_save;
				}
#if defined (DEBUG)
				/*???debug */
				printf("before unemap_get_samples_acquired\n");
#endif /* defined (DEBUG) */
				if (unemap_get_number_of_channels(&number_of_channels)&&
					ALLOCATE(samples,short int,number_of_samples*number_of_channels)&&
					unemap_get_samples_acquired(0,number_of_samples,samples,(int *)NULL))
				{
#if defined (DEBUG)
					/*???debug */
					printf("before reordering\n");
#endif /* defined (DEBUG) */
					while (i>0)
					{
						if ((*device)&&((*device)->signal)&&((*device)->channel)&&
							(signal_buffer=(*device)->signal->buffer))
						{
							signal_buffer->start=0;
							signal_buffer->end=(int)(number_of_samples-1);
							channel_number=((*device)->channel->number)-1;
							number_of_signals=signal_buffer->number_of_signals;
							destination=((signal_buffer->signals).short_int_values)+
								((*device)->signal->index);
							if ((0<=channel_number)&&(channel_number<number_of_channels))
							{
								source=samples+channel_number;
								for (j=(int)number_of_samples;j>0;j--)
								{
									*destination= *source;
									source += number_of_channels;
									destination += number_of_signals;
								}
							}
							else
							{
								for (j=(int)number_of_samples;j>0;j--)
								{
									*destination=0;
									destination += number_of_signals;
								}
							}
						}
						i--;
						device++;
					}
#if defined (DEBUG)
					/*???debug */
					printf("after reordering\n");
#endif /* defined (DEBUG) */
					/* write out the signal file */
					open_file_and_write(
#if defined (MOTIF)
						(Widget)NULL,(XtPointer)(page_window->save_file_open_data),
						(XtPointer)NULL
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
						page_window->save_file_open_data
#endif /* defined (WINDOWS) */
						);
				}
				DEALLOCATE(samples);
			}
#endif /* defined (BACKGROUND_SAVING) */
#endif /* defined (MIRADA) */
		}
		else
		{
			display_message(ERROR_MESSAGE,"page_save_data.  Invalid rig");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"page_save_data.  page_window missing");
	}
	LEAVE;
} /* page_save_data */

static void page_auto_range(
#if defined (MOTIF)
	Widget widget,XtPointer page_window_structure,XtPointer call_data
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	LONG page_window_structure
#endif /* defined (WINDOWS) */
	)
/*******************************************************************************
LAST MODIFIED : 13 January 2002

DESCRIPTION :
Called when the auto range button is pressed.
==============================================================================*/
{
	float *card_maximum,*card_minimum,channel_gain,channel_offset,pre_filter_gain,
		post_filter_gain,signal_maximum,signal_minimum,temp;
	int card_number,channel_number,i,j,number_of_channels;
	short int maximum,minimum,*samples,*source;
	struct Auxiliary_properties *auxiliary_properties;
	struct Channel *channel;
	struct Device **device;
	struct Page_window *page_window;
	struct Rig *rig;
	unsigned long number_of_samples;

	ENTER(page_auto_range);
#if defined (MOTIF)
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
#endif /* defined (MOTIF) */
	if (page_window=(struct Page_window *)page_window_structure)
	{
		if ((page_window->rig_address)&&(rig= *(page_window->rig_address))&&
			(device=rig->devices))
		{
			/* stop continuous sampling */
			unemap_stop_sampling();
			samples=(short int *)NULL;
			card_maximum=(float *)NULL;
			card_minimum=(float *)NULL;
			/*???DB.  Making assumption about hardware (gain same for each block of
				64) */
			if (unemap_get_number_of_samples_acquired(&number_of_samples)&&
				unemap_get_number_of_channels(&number_of_channels)&&
				ALLOCATE(samples,short int,number_of_samples*number_of_channels)&&
				ALLOCATE(card_maximum,float,number_of_channels/64)&&
				ALLOCATE(card_minimum,float,number_of_channels/64)&&
				unemap_get_samples_acquired(0,0,samples,(int *)NULL))
			{
				/* calculate ranges */
				for (card_number=0;card_number<number_of_channels/64;card_number++)
				{
					card_minimum[card_number]=(float)1;
					card_maximum[card_number]=(float)0;
				}
				i=rig->number_of_devices;
				while (i>0)
				{
					if ((*device)&&(channel=(*device)->channel))
					{
						(*device)->signal_display_minimum=(float)-1;
						(*device)->signal_display_maximum=(float)1;
						channel_number=(channel->number)-1;
						card_number=channel_number/64;
						if ((0<=channel_number)&&(channel_number<number_of_channels))
						{
							source=samples+channel_number;
							minimum= *source;
							maximum=minimum;
							for (j=(int)number_of_samples-1;j>0;j--)
							{
								source += number_of_channels;
								if (*source<minimum)
								{
									minimum= *source;
								}
								else
								{
									if (*source>maximum)
									{
										maximum= *source;
									}
								}
							}
							if (unemap_get_gain(channel_number+1,&pre_filter_gain,
								&post_filter_gain))
							{
								channel_offset=channel->offset;
								channel_gain=(channel->gain_correction)/
									(pre_filter_gain*post_filter_gain);
								signal_minimum=channel_gain*((float)minimum-channel_offset);
								signal_maximum=channel_gain*((float)maximum-channel_offset);
								/* increase range by 20% */
								temp=(float)((2.2*signal_minimum-0.2*signal_maximum)/2);
								signal_maximum=
									(float)((-0.2*signal_minimum+2.2*signal_maximum)/2);
								signal_minimum=temp;
								(*device)->signal_display_minimum=signal_minimum;
								(*device)->signal_display_maximum=signal_maximum;
								if (card_maximum[card_number]<card_minimum[card_number])
								{
									card_minimum[card_number]=signal_minimum;
									card_maximum[card_number]=signal_maximum;
								}
								else
								{
									if (signal_minimum<card_minimum[card_number])
									{
										card_minimum[card_number]=signal_minimum;
									}
									else
									{
										if (signal_maximum>card_maximum[card_number])
										{
											card_maximum[card_number]=signal_maximum;
										}
									}
								}
							}
						}
					}
					i--;
					device++;
				}
				device=rig->devices;
				for (i=rig->number_of_devices;i>0;i--)
				{
					if ((*device)&&!((*device)->channel))
					{
						signal_minimum=(float)0;
						signal_maximum=(float)0;
						auxiliary_properties= &(((*device)->description->properties).
							auxiliary);
						for (j=0;j<auxiliary_properties->number_of_electrodes;j++)
						{
							if (0<(auxiliary_properties->electrode_coefficients)[j])
							{
								signal_minimum +=
									(auxiliary_properties->electrode_coefficients)[j]*
									((auxiliary_properties->electrodes)[j])->signal_display_minimum;
								signal_maximum +=
									(auxiliary_properties->electrode_coefficients)[j]*
									((auxiliary_properties->electrodes)[j])->signal_display_maximum;
							}
							else
							{
								signal_minimum +=
									(auxiliary_properties->electrode_coefficients)[j]*
									((auxiliary_properties->electrodes)[j])->signal_display_maximum;
								signal_maximum +=
									(auxiliary_properties->electrode_coefficients)[j]*
									((auxiliary_properties->electrodes)[j])->signal_display_minimum;
							}
						}
						(*device)->signal_display_minimum=signal_minimum;
						(*device)->signal_display_maximum=signal_maximum;
					}
					device++;
				}
				/* set card gains */
				for (card_number=0;card_number<number_of_channels/64;card_number++)
				{
					/*???DB.  Making assumption about hardware (gain same for each block
						of 64) */
					i=card_number*64+1;
					if ((card_minimum[card_number]<card_maximum[card_number])&&
						unemap_get_voltage_range(i,&signal_minimum,&signal_maximum)&&
						unemap_get_gain(i,&pre_filter_gain,&post_filter_gain))
					{
						if (card_maximum[card_number]<0)
						{
							temp= -card_minimum[card_number];
						}
						else
						{
							temp=card_maximum[card_number];
							if (temp< -card_minimum[card_number])
							{
								temp= -card_minimum[card_number];
							}
						}
						/*???DB.  Assuming that signal_minimum= -signal_maximum */
						page_window_set_gain(page_window,i,signal_maximum*pre_filter_gain*
							post_filter_gain/temp);
					}
				}
			}
			DEALLOCATE(samples);
			DEALLOCATE(card_maximum);
			DEALLOCATE(card_minimum);
			draw_scrolling_background(page_window);
			/* start continuous sampling */
			unemap_start_sampling();
		}
		else
		{
			display_message(ERROR_MESSAGE,"page_save_data.  Invalid rig");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"page_auto_range.  page_window missing");
	}
	LEAVE;
} /* page_auto_range */

static void page_full_range(
#if defined (MOTIF)
	Widget widget,XtPointer page_window_structure,XtPointer call_data
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	LONG page_window_structure
#endif /* defined (WINDOWS) */
	)
/*******************************************************************************
LAST MODIFIED : 12 November 2000

DESCRIPTION :
Called when the full range button is pressed.
==============================================================================*/
{
	float channel_gain,channel_offset,pre_filter_gain,post_filter_gain;
	int i,j,number_of_devices;
	struct Auxiliary_properties *auxiliary_properties;
	struct Channel *channel;
	struct Device **device_address;
	struct Page_window *page_window;

	ENTER(page_full_range);
#if defined (MOTIF)
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
#endif /* defined (MOTIF) */
	if (page_window=(struct Page_window *)page_window_structure)
	{
		/* set gain on all channels as low as possible */
		unemap_set_gain(0,(float)1,(float)1);
		/* set the signal minimums and maximums */
		if ((page_window->rig_address)&&(*(page_window->rig_address))&&
			(0<(number_of_devices=(*(page_window->rig_address))->
			number_of_devices))&&(device_address=(*(page_window->
			rig_address))->devices))
		{
			for (i=0;i<number_of_devices;i++)
			{
				if ((*device_address)&&(channel=(*device_address)->channel))
				{
					if (unemap_get_gain(channel->number,&pre_filter_gain,
						&post_filter_gain))
					{
						channel_offset=channel->offset;
						channel_gain=(channel->gain_correction)/
							(pre_filter_gain*post_filter_gain);
						(*device_address)->signal_display_minimum=channel_gain*
							((float)minimum_signal_value-channel_offset);
						(*device_address)->signal_display_maximum=channel_gain*
							((float)maximum_signal_value-channel_offset);
					}
					else
					{
						(*device_address)->signal_display_minimum=(float)-1;
						(*device_address)->signal_display_maximum=(float)1;
					}
				}
				device_address++;
			}
			device_address=(*(page_window->rig_address))->devices;
			for (i=0;i<number_of_devices;i++)
			{
				if ((*device_address)&&!((*device_address)->channel))
				{
					(*device_address)->signal_display_minimum=(float)0;
					(*device_address)->signal_display_maximum=(float)0;
					auxiliary_properties= &(((*device_address)->description->
						properties).auxiliary);
					for (j=0;j<auxiliary_properties->number_of_electrodes;j++)
					{
						if (0<(auxiliary_properties->electrode_coefficients)[j])
						{
							(*device_address)->signal_display_minimum +=
								(auxiliary_properties->electrode_coefficients)[j]*
								((auxiliary_properties->electrodes)[j])->
								signal_display_minimum;
							(*device_address)->signal_display_maximum +=
								(auxiliary_properties->electrode_coefficients)[j]*
								((auxiliary_properties->electrodes)[j])->
								signal_display_maximum;
						}
						else
						{
							(*device_address)->signal_display_minimum +=
								(auxiliary_properties->electrode_coefficients)[j]*
								((auxiliary_properties->electrodes)[j])->
								signal_display_maximum;
							(*device_address)->signal_display_maximum +=
								(auxiliary_properties->electrode_coefficients)[j]*
								((auxiliary_properties->electrodes)[j])->
								signal_display_minimum;
						}
					}
				}
				device_address++;
			}
			draw_scrolling_background(page_window);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"page_full_range.  page_window missing");
	}
	LEAVE;
} /* page_full_range */

#if defined (OLD_CODE)
static void page_reset_scale(
#if defined (MOTIF)
	Widget widget,XtPointer page_window_structure,XtPointer call_data
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	LONG page_window_structure
#endif /* defined (WINDOWS) */
	)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Called when the reset scale button is pressed.
==============================================================================*/
{
	struct Page_window *page_window;

	ENTER(page_reset_scale);
#if defined (MOTIF)
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
#endif /* defined (MOTIF) */
	if (page_window=(struct Page_window *)page_window_structure)
	{
		page_window->signal_display_maximum=(float)0;
		page_window->signal_display_minimum=(float)1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"page_reset_scale.  page_window missing");
	}
	LEAVE;
} /* page_reset_scale */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
static void page_scale(
#if defined (MOTIF)
	Widget widget,XtPointer page_window_structure,XtPointer call_data
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	LONG page_window_structure
#endif /* defined (WINDOWS) */
	)
/*******************************************************************************
LAST MODIFIED : 7 April 2000

DESCRIPTION :
Called when the scale button is pressed.
==============================================================================*/
{
	struct Page_window *page_window;

	ENTER(page_scale);
#if defined (MOTIF)
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
#endif /* defined (MOTIF) */
	if (page_window=(struct Page_window *)page_window_structure)
	{
		if (page_window->display_device)
		{
			if (page_window->signal_display_minimum<=page_window->signal_display_maximum)
			{
				if (page_window->signal_display_minimum<page_window->signal_display_maximum)
				{
					page_window->display_maximum=page_window->signal_display_maximum;
					page_window->display_minimum=page_window->signal_display_minimum;
				}
				else
				{
					page_window->display_maximum=page_window->signal_display_maximum+1;
					page_window->display_minimum=page_window->signal_display_minimum-1;
				}
				/* reset the scale */
					/*???DB.  Makes the reset scale button superfluous */
				page_window->signal_display_maximum=(float)0;
				page_window->signal_display_minimum=(float)1;
				show_display_maximum(page_window);
				show_display_minimum(page_window);
				draw_scrolling_background(page_window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"page_scale.  display_device missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"page_scale.  page_window missing");
	}
	LEAVE;
} /* page_scale */
#endif /* defined (OLD_CODE) */

static void page_start_all_stimulators(
#if defined (MOTIF)
	Widget widget,XtPointer page_window_structure,XtPointer call_data
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	LONG page_window_structure
#endif /* defined (WINDOWS) */
	)
/*******************************************************************************
LAST MODIFIED : 7 January 2001

DESCRIPTION :
Called when the start all stimulators button is pressed.
==============================================================================*/
{
	char *hardware_directory,*waveform_file_name;
	float *values,values_per_second;
	int constant_voltage,i,length,number_of_values,return_code,stimulator_number;
	struct Page_window *page_window;
	unsigned int number_of_cycles;

	ENTER(page_start_all_stimulators);
#if defined (MOTIF)
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
#endif /* defined (MOTIF) */
	if (page_window=(struct Page_window *)page_window_structure)
	{
		if (0<page_window->number_of_stimulators)
		{
			if (hardware_directory=getenv("UNEMAP_HARDWARE"))
			{
				if (ALLOCATE(waveform_file_name,char,strlen(hardware_directory)+15+
					(int)log10(page_window->number_of_stimulators)+1))
				{
					strcpy(waveform_file_name,hardware_directory);
#if defined (WIN32)
					if ('\\'!=waveform_file_name[strlen(waveform_file_name)-1])
					{
						strcat(waveform_file_name,"\\");
					}
#else /* defined (WIN32) */
					if ('/'!=waveform_file_name[strlen(waveform_file_name)-1])
					{
						strcat(waveform_file_name,"/");
					}
#endif /* defined (WIN32) */
				}
			}
			else
			{
				if (ALLOCATE(waveform_file_name,char,14+
					(int)log10(page_window->number_of_stimulators)+1))
				{
					waveform_file_name[0]='\0';
				}
			}
			if (waveform_file_name)
			{
				return_code=1;
				stimulator_number=page_window->number_of_stimulators;
				length=strlen(waveform_file_name);
				while (return_code&&(stimulator_number>0))
				{
					stimulator_number--;
					if (!(((page_window->stimulators)[stimulator_number]).on)&&
						(0<((page_window->stimulators)[stimulator_number]).
						number_of_channels))
					{
						sprintf(waveform_file_name+length,"stimulate%d.wfm",
							stimulator_number+1);
						if (read_waveform_file(waveform_file_name,&constant_voltage,
							&number_of_values,&values_per_second,&values,
							&(((page_window->stimulators)[stimulator_number]).
							end_number_of_values),
							&(((page_window->stimulators)[stimulator_number]).
							end_values_per_second),
							&(((page_window->stimulators)[stimulator_number]).end_values)))
						{
							((page_window->stimulators)[stimulator_number]).constant_voltage=
								constant_voltage;
							if (0<((page_window->stimulators)[stimulator_number]).
								end_number_of_values)
							{
								number_of_cycles=1;
							}
							else
							{
								number_of_cycles=0;
							}
							if (constant_voltage)
							{
								return_code=unemap_load_voltage_stimulating(
									((page_window->stimulators)[stimulator_number]).
									number_of_channels,
									((page_window->stimulators)[stimulator_number]).
									channel_numbers,number_of_values,values_per_second,
									values,number_of_cycles,
									(Unemap_stimulation_end_callback *)NULL,(void *)NULL);
							}
							else
							{
								if ((UnEmap_2V1==page_window->unemap_hardware_version)||
									(UnEmap_2V2==page_window->unemap_hardware_version)||
									((UnEmap_2V1|UnEmap_2V2)==page_window->unemap_hardware_version))
								{
									return_code=unemap_load_current_stimulating(
										((page_window->stimulators)[stimulator_number]).
										number_of_channels,
										((page_window->stimulators)[stimulator_number]).
										channel_numbers,number_of_values,values_per_second,
										values,number_of_cycles,
										(Unemap_stimulation_end_callback *)NULL,(void *)NULL);
								}
								else
								{
									display_message(WARNING_MESSAGE,
						"Constant current stimulation is not available for this hardware");
									return_code=0;
								}
							}
							DEALLOCATE(values);
						}
					}
				}
				if (return_code)
				{
					/* set the channels to stimulating */
					for (stimulator_number=0;stimulator_number<page_window->
						number_of_stimulators;stimulator_number++)
					{
						if (!(((page_window->stimulators)[stimulator_number]).on)&&
							(0<((page_window->stimulators)[stimulator_number]).
							number_of_channels))
						{
							((page_window->stimulators)[stimulator_number]).on=1;
							for (i=0;i<((page_window->stimulators)[stimulator_number]).
								number_of_channels;i++)
							{
								unemap_set_channel_stimulating((((page_window->stimulators)[
									stimulator_number]).channel_numbers)[i],1);
							}
						}
					}
					return_code=unemap_start_stimulating();
				}
				DEALLOCATE(waveform_file_name);
			}
		}
	}
	update_stimulating_settings(page_window);
#if defined (OLD_CODE)
	/*???DB.  To be done */
	if (return_code)
	{
		(page_window->stimulator_on)[(page_window->stimulator_number)-1]=1;
#if defined (MOTIF)
		XmToggleButtonSetState((page_window->stimulator).stimulate.checkbox,
			True,False);
		XtSetSensitive((page_window->stimulator).stimulate.value,False);
		XtSetSensitive((page_window->stimulator).stimulate.arrows,False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		CheckDlgButton(page_window->window,STIMULATE_CHECKBOX,BST_CHECKED);
		EnableWindow((page_window->stimulate_channel).edit,FALSE);
		EnableWindow((page_window->stimulate_channel).arrows,FALSE);
#endif /* defined (WINDOWS) */
	}
	else
	{
#if defined (MOTIF)
		XmToggleButtonSetState((page_window->stimulator).stimulate.checkbox,
			False,False);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		CheckDlgButton(page_window->window,STIMULATE_CHECKBOX,BST_UNCHECKED);
#endif /* defined (WINDOWS) */
	}
#endif /* defined (OLD_CODE) */
	LEAVE;
} /* page_start_all_stimulators */

static void page_stop_all_stimulators(
#if defined (MOTIF)
	Widget widget,XtPointer page_window_structure,XtPointer call_data
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	LONG page_window_structure
#endif /* defined (WINDOWS) */
	)
/*******************************************************************************
LAST MODIFIED : 7 January 2001

DESCRIPTION :
Called when the stop all stimulators button is pressed.
==============================================================================*/
{
	float *values,values_per_second,zero;
	int constant_voltage,i,number_of_values;
	struct Page_window *page_window;

	ENTER(page_stop_all_stimulators);
#if defined (MOTIF)
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
#endif /* defined (MOTIF) */
	zero=(float)0;
	if (page_window=(struct Page_window *)page_window_structure)
	{
		for (i=0;i<page_window->number_of_stimulators;i++)
		{
			if (((page_window->stimulators)[i]).on)
			{
				if (0<(number_of_values=((page_window->stimulators)[i]).
					end_number_of_values))
				{
					values=((page_window->stimulators)[i]).end_values;
					constant_voltage=((page_window->stimulators)[i]).constant_voltage;
					values_per_second=
						((page_window->stimulators)[i]).end_values_per_second;
				}
				else
				{
					constant_voltage=1;
					values= &zero;
					number_of_values=1;
					values_per_second=(float)1;
				}
				if (constant_voltage)
				{
					unemap_load_voltage_stimulating(
						((page_window->stimulators)[i]).number_of_channels,
						((page_window->stimulators)[i]).channel_numbers,number_of_values,
						values_per_second,values,1,
						(Unemap_stimulation_end_callback *)NULL,(void *)NULL);
				}
				else
				{
					if ((UnEmap_2V1==page_window->unemap_hardware_version)||
						(UnEmap_2V2==page_window->unemap_hardware_version)||
						((UnEmap_2V1|UnEmap_2V2)==page_window->unemap_hardware_version))
					{
						unemap_load_current_stimulating(
							((page_window->stimulators)[i]).number_of_channels,
							((page_window->stimulators)[i]).channel_numbers,number_of_values,
							values_per_second,values,1,
							(Unemap_stimulation_end_callback *)NULL,(void *)NULL);
					}
					else
					{
						display_message(WARNING_MESSAGE,
						"Constant current stimulation is not available for this hardware");
					}
				}
				if (0<(number_of_values=((page_window->stimulators)[i]).
					end_number_of_values))
				{
					DEALLOCATE(((page_window->stimulators)[i]).end_values);
				}
			}
		}
		unemap_start_stimulating();
		unemap_set_channel_stimulating(0,0);
		for (i=0;i<page_window->number_of_stimulators;i++)
		{
			((page_window->stimulators)[i]).on=0;
		}
		update_stimulating_settings(page_window);
	}
	LEAVE;
} /* page_stop_all_stimulators */

static void open_Pacing_window_callback(
#if defined (MOTIF)
	Widget widget,XtPointer page_window_structure,XtPointer call_data
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	LONG page_window_structure
#endif /* defined (WINDOWS) */
	)
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Called when the pacing button is pressed.
==============================================================================*/
{
	struct Page_window *page_window;

	ENTER(open_Pacing_window_callback);
#if defined (MOTIF)
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
#endif /* defined (MOTIF) */
	if (page_window=(struct Page_window *)page_window_structure)
	{
		open_Pacing_window(&(page_window->pacing_window),page_window->rig_address,
#if defined (MOTIF)
			page_window->pacing_button,
#endif /* defined (MOTIF) */
			page_window->user_interface);
	}
	LEAVE;
} /* open_Pacing_window_callback */

#if defined (WINDOWS)
static void Page_window_WM_COMMAND_handler(HWND window,
	int item_control_accelerator_id,HWND control_window,UINT notify_code)
/*******************************************************************************
LAST MODIFIED : 10 October 2001

DESCRIPTION :
==============================================================================*/
{
	ENTER(Page_window_WM_COMMAND_handler);
	switch (item_control_accelerator_id)
		/*???DB.  Switch on <control_window> ? (when have no dialogs) */
	{
		/*???DB.  Will eventually call functions */
		case AUTO_RANGE_BUTTON:
		{
			page_auto_range(GetWindowLong(window,DLGWINDOWEXTRA));
		} break;
		case CALIBRATE_BUTTON:
		{
			start_calibrating(GetWindowLong(window,DLGWINDOWEXTRA));
		} break;
		case CLOSE_BUTTON:
		{
			DestroyWindow(window);
		} break;
		case ELECTRODE_EDIT:
		{
			switch (notify_code)
			{
				case EN_KILLFOCUS:
					/*???DB.  Pick up enter key press ? */
				{
					update_display_device((struct Page_window *)GetWindowLong(window,
						DLGWINDOWEXTRA));
				} break;
			}
		} break;
		case EXPERIMENT_CHECKBOX:
		{
			struct Page_window *page_window;
			UINT checkbox_state;

			page_window=(struct Page_window *)GetWindowLong(window,DLGWINDOWEXTRA);
			checkbox_state=IsDlgButtonChecked(window,EXPERIMENT_CHECKBOX);
			if (BST_CHECKED==checkbox_state)
			{
				start_experiment(page_window);
			}
			else
			{
				stop_experiment(page_window);
			}
		} break;
		case FULL_RANGE_BUTTON:
		{
			page_full_range(GetWindowLong(window,DLGWINDOWEXTRA));
		} break;
		case GAIN_EDIT:
		{
			switch (notify_code)
			{
				case EN_KILLFOCUS:
					/*???DB.  Pick up enter key press ? */
				{
					update_display_gain((struct Page_window *)GetWindowLong(window,
						DLGWINDOWEXTRA));
				} break;
			}
		} break;
		case ISOLATE_CHECKBOX:
		{
			struct Page_window *page_window;
			UINT checkbox_state;

			page_window=(struct Page_window *)GetWindowLong(window,DLGWINDOWEXTRA);
			checkbox_state=IsDlgButtonChecked(window,ISOLATE_CHECKBOX);
			if (BST_CHECKED==checkbox_state)
			{
				start_isolating(page_window);
			}
			else
			{
				stop_isolating(page_window);
			}
		} break;
		case LOW_PASS_FILTER_EDIT:
		{
			switch (notify_code)
			{
				case EN_KILLFOCUS:
					/*???DB.  Pick up enter key press ? */
				{
					update_filter((struct Page_window *)GetWindowLong(window,
						DLGWINDOWEXTRA));
				} break;
			}
		} break;
		case MAXIMUM_EDIT:
		{
			switch (notify_code)
			{
				case EN_KILLFOCUS:
					/*???DB.  Pick up enter key press ? */
				{
					update_display_maximum((struct Page_window *)GetWindowLong(window,
						DLGWINDOWEXTRA));
				} break;
			}
		} break;
		case MINIMUM_EDIT:
		{
			switch (notify_code)
			{
				case EN_KILLFOCUS:
					/*???DB.  Pick up enter key press ? */
				{
					update_display_minimum((struct Page_window *)GetWindowLong(window,
						DLGWINDOWEXTRA));
				} break;
			}
		} break;
		case PACING_BUTTON:
		{
			open_Pacing_window_callback(GetWindowLong(window,DLGWINDOWEXTRA));
		} break;
#if defined (OLD_CODE)
		case RESET_SCALE_BUTTON:
		{
			page_reset_scale(GetWindowLong(window,DLGWINDOWEXTRA));
		} break;
#endif /* defined (OLD_CODE) */
		case SAMPLE_CHECKBOX:
		{
			struct Page_window *page_window;
			UINT checkbox_state;

			page_window=(struct Page_window *)GetWindowLong(window,DLGWINDOWEXTRA);
			checkbox_state=IsDlgButtonChecked(window,SAMPLE_CHECKBOX);
			if (BST_CHECKED==checkbox_state)
			{
				start_sampling(page_window);
			}
			else
			{
				stop_sampling(page_window);
			}
		} break;
		case SAVE_BUTTON:
		{
			page_save_data(GetWindowLong(window,DLGWINDOWEXTRA));
		} break;
#if defined (OLD_CODE)
		case SCALE_BUTTON:
		{
			page_scale(GetWindowLong(window,DLGWINDOWEXTRA));
		} break;
#endif /* defined (OLD_CODE) */
		case START_ALL_STIMULATORS_BUTTON:
		{
			page_start_all_stimulators(GetWindowLong(window,DLGWINDOWEXTRA));
		} break;
#if defined (OLD_CODE)
		case STIMULATE_CHANNEL_EDIT:
		{
			switch (notify_code)
			{
				case EN_KILLFOCUS:
					/*???DB.  Pick up enter key press ? */
				{
					update_stimulate_device((struct Page_window *)GetWindowLong(window,
						DLGWINDOWEXTRA));
				} break;
			}
		} break;
#endif /* defined (OLD_CODE) */
		case STIMULATE_CHECKBOX:
		{
			struct Page_window *page_window;
			UINT checkbox_state;

			page_window=(struct Page_window *)GetWindowLong(window,DLGWINDOWEXTRA);
			checkbox_state=IsDlgButtonChecked(window,STIMULATE_CHECKBOX);
			if (BST_CHECKED==checkbox_state)
			{
				set_current_electrode_stimulate(page_window);
			}
			else
			{
				set_current_electrode_record(page_window);
			}
#if defined (OLD_CODE)
			if (BST_CHECKED==checkbox_state)
			{
				start_stimulating(page_window);
			}
			else
			{
				stop_stimulating(page_window);
			}
#endif /* defined (OLD_CODE) */
		} break;
		case STIMULATOR_CHECKBOX:
		{
			struct Page_window *page_window;
			UINT checkbox_state;

			page_window=(struct Page_window *)GetWindowLong(window,DLGWINDOWEXTRA);
			checkbox_state=IsDlgButtonChecked(window,STIMULATOR_CHECKBOX);
			if (BST_CHECKED==checkbox_state)
			{
				start_stimulator(page_window);
			}
			else
			{
				stop_stimulator(page_window);
			}
		} break;
		case STOP_ALL_STIMULATORS_BUTTON:
		{
			page_stop_all_stimulators(GetWindowLong(window,DLGWINDOWEXTRA));
		} break;
		case TEST_CHECKBOX:
		{
			struct Page_window *page_window;
			UINT checkbox_state;

			page_window=(struct Page_window *)GetWindowLong(window,DLGWINDOWEXTRA);
			checkbox_state=IsDlgButtonChecked(window,TEST_CHECKBOX);
			if (BST_CHECKED==checkbox_state)
			{
				start_testing(page_window);
			}
			else
			{
				stop_testing(page_window);
			}
		} break;
	}
	LEAVE;
} /* Page_window_WM_COMMAND_handler */
#endif /* defined (WINDOWS) */

#if defined (WINDOWS)
WNDPROC old_electrode_edit_wndproc;

static LRESULT CALLBACK electrode_edit_pick_up_enter(HWND window,
	UINT message_identifier,WPARAM first_message,LPARAM second_message)
/*******************************************************************************
LAST MODIFIED : 30 May 1997

DESCRIPTION :
???DB.  Should return 0 if it processes the message.
==============================================================================*/
{
	LRESULT return_code;

	ENTER(electrode_edit_pick_up_enter);
	switch (message_identifier)
	{
		case WM_KEYDOWN:
		{
			switch ((UINT)first_message)
			{
				case VK_RETURN:
				{
					update_display_device((struct Page_window *)GetWindowLong(window,
						GWL_USERDATA));
				} break;
			}
		} break;
	}
	return_code=CallWindowProc(old_electrode_edit_wndproc,window,
		message_identifier,first_message,second_message);
	LEAVE;

	return (return_code);
} /* electrode_edit_pick_up_enter */
#endif /* defined (WINDOWS) */

#if defined (OLD_CODE)
#if defined (WINDOWS)
WNDPROC old_stimulate_channel_edit_wndproc;

static LRESULT CALLBACK stimulate_channel_edit_pick_up_enter(HWND window,
	UINT message_identifier,WPARAM first_message,LPARAM second_message)
/*******************************************************************************
LAST MODIFIED : 19 May 1999

DESCRIPTION :
???DB.  Should return 0 if it processes the message.
==============================================================================*/
{
	LRESULT return_code;

	ENTER(stimulate_channel_edit_pick_up_enter);
	switch (message_identifier)
	{
		case WM_KEYDOWN:
		{
			switch ((UINT)first_message)
			{
				case VK_RETURN:
				{
					update_stimulate_device((struct Page_window *)GetWindowLong(window,
						GWL_USERDATA));
				} break;
			}
		} break;
	}
	return_code=CallWindowProc(old_stimulate_channel_edit_wndproc,window,
		message_identifier,first_message,second_message);
	LEAVE;

	return (return_code);
} /* stimulate_channel_edit_pick_up_enter */
#endif /* defined (WINDOWS) */
#endif /* defined (OLD_CODE) */

#if defined (WINDOWS)
WNDPROC old_low_pass_filter_edit_wndproc;

static LRESULT CALLBACK low_pass_filter_edit_pick_up_enter(HWND window,
	UINT message_identifier,WPARAM first_message,LPARAM second_message)
/*******************************************************************************
LAST MODIFIED : 10 April 1998

DESCRIPTION :
???DB.  Should return 0 if it processes the message.
==============================================================================*/
{
	LRESULT return_code;

	ENTER(low_pass_filter_edit_pick_up_enter);
	switch (message_identifier)
	{
		case WM_KEYDOWN:
		{
			switch ((UINT)first_message)
			{
				case VK_RETURN:
				{
					update_filter((struct Page_window *)GetWindowLong(window,
						GWL_USERDATA));
				} break;
			}
		} break;
	}
	return_code=CallWindowProc(old_low_pass_filter_edit_wndproc,window,
		message_identifier,first_message,second_message);
	LEAVE;

	return (return_code);
} /* low_pass_filter_edit_pick_up_enter */
#endif /* defined (WINDOWS) */

#if defined (WINDOWS)
WNDPROC old_gain_edit_wndproc;

static LRESULT CALLBACK gain_edit_pick_up_enter(HWND window,
	UINT message_identifier,WPARAM first_message,LPARAM second_message)
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
???DB.  Should return 0 if it processes the message.
==============================================================================*/
{
	LRESULT return_code;

	ENTER(gain_edit_pick_up_enter);
	switch (message_identifier)
	{
		case WM_KEYDOWN:
		{
			switch ((UINT)first_message)
			{
				case VK_RETURN:
				{
					update_display_gain((struct Page_window *)GetWindowLong(window,
						GWL_USERDATA));
				} break;
			}
		} break;
	}
	return_code=CallWindowProc(old_gain_edit_wndproc,window,message_identifier,
		first_message,second_message);
	LEAVE;

	return (return_code);
} /* gain_edit_pick_up_enter */
#endif /* defined (WINDOWS) */

#if defined (WINDOWS)
WNDPROC old_maximum_edit_wndproc;

static LRESULT CALLBACK maximum_edit_pick_up_enter(HWND window,
	UINT message_identifier,WPARAM first_message,LPARAM second_message)
/*******************************************************************************
LAST MODIFIED : 30 May 1997

DESCRIPTION :
???DB.  Should return 0 if it processes the message.
==============================================================================*/
{
	LRESULT return_code;

	ENTER(maximum_edit_pick_up_enter);
	switch (message_identifier)
	{
		case WM_KEYDOWN:
		{
			switch ((UINT)first_message)
			{
				case VK_RETURN:
				{
					update_display_maximum((struct Page_window *)GetWindowLong(window,
						GWL_USERDATA));
				} break;
			}
		} break;
	}
	return_code=CallWindowProc(old_maximum_edit_wndproc,window,message_identifier,
		first_message,second_message);
	LEAVE;

	return (return_code);
} /* maximum_edit_pick_up_enter */
#endif /* defined (WINDOWS) */

#if defined (WINDOWS)
WNDPROC old_minimum_edit_wndproc;

static LRESULT CALLBACK minimum_edit_pick_up_enter(HWND window,
	UINT message_identifier,WPARAM first_message,LPARAM second_message)
/*******************************************************************************
LAST MODIFIED : 30 May 1997

DESCRIPTION :
???DB.  Should return 0 if it processes the message.
==============================================================================*/
{
	LRESULT return_code;

	ENTER(minimum_edit_pick_up_enter);
	switch (message_identifier)
	{
		case WM_KEYDOWN:
		{
			switch ((UINT)first_message)
			{
				case VK_RETURN:
				{
					update_display_minimum((struct Page_window *)GetWindowLong(window,
						GWL_USERDATA));
				} break;
			}
		} break;
	}
	return_code=CallWindowProc(old_minimum_edit_wndproc,window,message_identifier,
		first_message,second_message);
	LEAVE;

	return (return_code);
} /* minimum_edit_pick_up_enter */
#endif /* defined (WINDOWS) */

static void increment_electrode(
#if defined (MOTIF)
	Widget widget,XtPointer page_window_structure,XtPointer call_data
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	LONG page_window_structure
#endif /* defined (WINDOWS) */
	)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Called when the electrode up arrow is pressed.
==============================================================================*/
{
	int device_number;
	struct Page_window *page_window;

	ENTER(increment_electrode);
#if defined (MOTIF)
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
#endif /* defined (MOTIF) */
	if (page_window=(struct Page_window *)page_window_structure)
	{
		device_number=page_window->display_device_number;
		device_number++;
		change_display_device_number(page_window,device_number);
	}
	else
	{
		display_message(ERROR_MESSAGE,"increment_electrode.  page_window missing");
	}
	LEAVE;
} /* increment_electrode */

static void decrement_electrode(
#if defined (MOTIF)
	Widget widget,XtPointer page_window_structure,XtPointer call_data
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	LONG page_window_structure
#endif /* defined (WINDOWS) */
	)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Called when the electrode down arrow is pressed.
==============================================================================*/
{
	int device_number;
	struct Page_window *page_window;

	ENTER(decrement_electrode);
#if defined (MOTIF)
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
#endif /* defined (MOTIF) */
	if (page_window=(struct Page_window *)page_window_structure)
	{
		device_number=page_window->display_device_number;
		device_number--;
		change_display_device_number(page_window,device_number);
	}
	else
	{
		display_message(ERROR_MESSAGE,"decrement_electrode.  page_window missing");
	}
	LEAVE;
} /* decrement_electrode */

#if defined (OLD_CODE)
static int change_stimulate_device_number(struct Page_window *page_window,
	int device_number)
/*******************************************************************************
LAST MODIFIED : 8 July 1999

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Device *device;

	ENTER(change_stimulate_device_number);
	return_code=0;
	if (page_window&&(0<page_window->number_of_stimulators)&&(0<=device_number)&&
		(device_number<((*(page_window->rig_address))->number_of_devices))&&
		(device=((*(page_window->rig_address))->devices)[device_number])&&
		(device->signal)&&(device->channel)&&(0<device->channel->number)&&
		(device->channel->number<=(int)(page_window->number_of_channels))&&
		unemap_channel_valid_for_stimulator(page_window->stimulator_number,
		device->channel->number)&&(device->description)&&
		(device->description->name))
	{
		(page_window->stimulate_devices)[(page_window->stimulator_number)-1]=device;
		(page_window->stimulate_device_numbers)[(page_window->stimulator_number)-1]=
			device_number;
#if defined (OLD_CODE)
#if defined (MOTIF)
		XtVaSetValues((page_window->stimulator).stimulate.value,XmNvalue,
			device->description->name,NULL);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		Edit_SetText((page_window->stimulate_channel).edit,
			device->description->name);
#endif /* defined (WINDOWS) */
#endif /* defined (OLD_CODE) */
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* change_stimulate_device_number */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
static void increment_stimulate(
#if defined (MOTIF)
	Widget widget,XtPointer page_window_structure,XtPointer call_data
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	LONG page_window_structure
#endif /* defined (WINDOWS) */
	)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Called when the stimulate up arrow is pressed.
==============================================================================*/
{
	int device_number;
	struct Page_window *page_window;

	ENTER(increment_stimulate);
#if defined (MOTIF)
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
#endif /* defined (MOTIF) */
	if (page_window=(struct Page_window *)page_window_structure)
	{
		device_number=(page_window->stimulate_device_numbers)[
			(page_window->stimulator_number)-1];
		do
		{
			device_number++;
		} while ((device_number<(*(page_window->rig_address))->number_of_devices)&&
			!change_stimulate_device_number(page_window,device_number));
	}
	else
	{
		display_message(ERROR_MESSAGE,"increment_stimulate.  page_window missing");
	}
	LEAVE;
} /* increment_stimulate */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
static void decrement_stimulate(
#if defined (MOTIF)
	Widget widget,XtPointer page_window_structure,XtPointer call_data
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	LONG page_window_structure
#endif /* defined (WINDOWS) */
	)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Called when the stimulate down arrow is pressed.
==============================================================================*/
{
	int device_number;
	struct Page_window *page_window;

	ENTER(decrement_stimulate);
#if defined (MOTIF)
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
#endif /* defined (MOTIF) */
	if (page_window=(struct Page_window *)page_window_structure)
	{
		device_number=(page_window->stimulate_device_numbers)[
			(page_window->stimulator_number)-1];
		do
		{
			device_number--;
		} while ((device_number>=0)&&
			!change_stimulate_device_number(page_window,device_number));
	}
	else
	{
		display_message(ERROR_MESSAGE,"decrement_stimulate.  page_window missing");
	}
	LEAVE;
} /* decrement_stimulate */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
static void increment_stimulator(
#if defined (MOTIF)
	Widget widget,XtPointer page_window_structure,XtPointer call_data
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	LONG page_window_structure
#endif /* defined (WINDOWS) */
	)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Called when the stimulator up arrow is pressed.
==============================================================================*/
{
	struct Page_window *page_window;

	ENTER(increment_stimulator);
#if defined (MOTIF)
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
#endif /* defined (MOTIF) */
	if (page_window=(struct Page_window *)page_window_structure)
	{
		update_stimulator(page_window,(page_window->stimulator_number)+1);
	}
	else
	{
		display_message(ERROR_MESSAGE,"increment_stimulator.  page_window missing");
	}
	LEAVE;
} /* increment_stimulator */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
static void decrement_stimulator(
#if defined (MOTIF)
	Widget widget,XtPointer page_window_structure,XtPointer call_data
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	LONG page_window_structure
#endif /* defined (WINDOWS) */
	)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Called when the stimulator down arrow is pressed.
==============================================================================*/
{
	struct Page_window *page_window;

	ENTER(decrement_stimulator);
#if defined (MOTIF)
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
#endif /* defined (MOTIF) */
	if (page_window=(struct Page_window *)page_window_structure)
	{
		update_stimulator(page_window,(page_window->stimulator_number)-1);
	}
	else
	{
		display_message(ERROR_MESSAGE,"decrement_stimulator.  page_window missing");
	}
	LEAVE;
} /* decrement_stimulator */
#endif /* defined (OLD_CODE) */

#if defined (WINDOWS)
static LRESULT CALLBACK Page_window_class_proc(HWND window,
	UINT message_identifier,WPARAM first_message,LPARAM second_message)
/*******************************************************************************
LAST MODIFIED : 11 November 2001

DESCRIPTION :
???DB.  Should return 0 if it processes the message.
==============================================================================*/
{
	LRESULT return_code;

	ENTER(Page_window_class_proc);
	switch (message_identifier)
	{
		case WM_CLOSE:
		{
			DestroyWindow(window);
			return_code=0;
		} break;
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return_code=0;
		} break;
		case WM_COMMAND:
		{
			return_code=(BOOL)HANDLE_WM_COMMAND(window,first_message,second_message,
				Page_window_WM_COMMAND_handler);
		} break;
		case WM_INITDIALOG:
		{
			BOOL win32_return_code;
			int restitution_time_pacing,widget_spacing;
			RECT rectangle;
			static char *class_name="Page_window_scrolling_area";
			struct Page_window *page_window;
			WNDCLASSEX class_information;

			/*???DB.  Need to create drawing area */
			if (page_window=(struct Page_window *)second_message)
			{
				/*???DB.  Got from Unemap under MOTIF */
				restitution_time_pacing=0;
				SetWindowLong(window,DLGWINDOWEXTRA,(LONG)page_window);
				/* retrieve the control windows and do any setup required */
				widget_spacing=page_window->user_interface->widget_spacing;
				/* auto_range_button */
				page_window->auto_range_button=GetDlgItem(window,AUTO_RANGE_BUTTON);
				GetWindowRect(page_window->auto_range_button,&rectangle);
				page_window->auto_range_button_width=rectangle.right-rectangle.left;
				page_window->auto_range_button_height=rectangle.bottom-rectangle.top;
				EnableWindow(page_window->auto_range_button,FALSE);
				/* calibrate_button */
				page_window->calibrate_button=GetDlgItem(window,CALIBRATE_BUTTON);
#if defined (MIRADA) || defined (UNEMAP_1V1)
				DestroyWindow(page_window->calibrate_button);
				page_window->calibrate_button=(HWND)NULL;
				page_window->calibrate_button_width=0;
				page_window->calibrate_button_height=0;
#else /* defined (MIRADA) || defined (UNEMAP_1V1) */
				switch (page_window->unemap_hardware_version)
				{
					case UnEmap_1V2:
					default:
					{
						DestroyWindow(page_window->calibrate_button);
						page_window->calibrate_button=(HWND)NULL;
						page_window->calibrate_button_width=0;
						page_window->calibrate_button_height=0;
					} break;
					case UnEmap_2V1:
					case UnEmap_2V2:
					case UnEmap_2V1|UnEmap_2V2:
					{
						EnableWindow(page_window->calibrate_button,FALSE);
						GetWindowRect(page_window->calibrate_button,&rectangle);
						page_window->calibrate_button_width=rectangle.right-rectangle.left;
						page_window->calibrate_button_height=rectangle.bottom-rectangle.top;
					} break;
				}
#endif /* defined (MIRADA) || defined (UNEMAP_1V1) */
				/* close_button */
				page_window->close_button=GetDlgItem(window,CLOSE_BUTTON);
				EnableWindow(page_window->close_button,TRUE);
				GetWindowRect(page_window->close_button,&rectangle);
				page_window->close_button_width=rectangle.right-rectangle.left;
				page_window->close_button_height=rectangle.bottom-rectangle.top;
				/* electrode */
				(page_window->electrode).edit=GetDlgItem(window,ELECTRODE_EDIT);
				EnableWindow((page_window->electrode).edit,FALSE);
				GetWindowRect((page_window->electrode).edit,&rectangle);
				(page_window->electrode).edit_width=rectangle.right-rectangle.left;
				(page_window->electrode).edit_height=rectangle.bottom-rectangle.top;
				if (page_window->display_device)
				{
					Edit_SetText((page_window->electrode).edit,
						page_window->display_device->description->name);
				}
				/* replace the window procedure so that enters can be picked up */
				old_electrode_edit_wndproc=(WNDPROC)SetWindowLong(
					(page_window->electrode).edit,GWL_WNDPROC,
					(DWORD)electrode_edit_pick_up_enter);
				SetWindowLong((page_window->electrode).edit,GWL_USERDATA,
					(LONG)page_window);
				/* add up and down arrows for the channel number */
				(page_window->electrode).arrows_width=ARROWS_WIDTH;
				(page_window->electrode).arrows_height=
					(page_window->electrode).edit_height;
				(page_window->electrode).arrows=CreateUpDownControl(
					WS_CHILD|WS_BORDER|WS_VISIBLE|UDS_ALIGNLEFT,0,0,
					(page_window->electrode).arrows_width,
					(page_window->electrode).arrows_height,window,ELECTRODE_ARROWS,
					page_window->instance,(HWND)NULL,page_window->number_of_channels,1,1);
				EnableWindow((page_window->electrode).arrows,FALSE);
				(page_window->electrode).text=GetDlgItem(window,ELECTRODE_TEXT);
				EnableWindow((page_window->electrode).text,FALSE);
				GetWindowRect((page_window->electrode).text,&rectangle);
				(page_window->electrode).text_width=rectangle.right-rectangle.left;
				(page_window->electrode).text_height=rectangle.bottom-rectangle.top;
				/* experiment_checkbox */
				page_window->experiment_checkbox=GetDlgItem(window,EXPERIMENT_CHECKBOX);
				GetWindowRect(page_window->experiment_checkbox,&rectangle);
				EnableWindow(page_window->experiment_checkbox,FALSE);
				page_window->experiment_checkbox_width=rectangle.right-rectangle.left;
				page_window->experiment_checkbox_height=rectangle.bottom-rectangle.top;
				/* full_range_button */
				page_window->full_range_button=GetDlgItem(window,FULL_RANGE_BUTTON);
				GetWindowRect(page_window->full_range_button,&rectangle);
				EnableWindow(page_window->full_range_button,FALSE);
				page_window->full_range_button_width=rectangle.right-rectangle.left;
				page_window->full_range_button_height=rectangle.bottom-rectangle.top;
				/* gain */
				(page_window->gain).edit=GetDlgItem(window,GAIN_EDIT);
				EnableWindow((page_window->gain).edit,FALSE);
				GetWindowRect((page_window->gain).edit,&rectangle);
				(page_window->gain).edit_width=rectangle.right-rectangle.left;
				(page_window->gain).edit_height=rectangle.bottom-rectangle.top;
				/* replace the window procedure so that enters can be picked up */
				old_gain_edit_wndproc=(WNDPROC)SetWindowLong(
					(page_window->gain).edit,GWL_WNDPROC,
					(DWORD)gain_edit_pick_up_enter);
				SetWindowLong((page_window->gain).edit,GWL_USERDATA,
					(LONG)page_window);
				(page_window->gain).text=GetDlgItem(window,GAIN_TEXT);
				EnableWindow((page_window->gain).text,FALSE);
				GetWindowRect((page_window->gain).text,&rectangle);
				(page_window->gain).text_width=rectangle.right-rectangle.left;
				(page_window->gain).text_height=rectangle.bottom-rectangle.top;
				/* isolate_checkbox */
				page_window->isolate_checkbox=GetDlgItem(window,ISOLATE_CHECKBOX);
#if defined (MIRADA) || defined (UNEMAP_1V1)
				DestroyWindow(page_window->isolate_checkbox);
				page_window->isolate_checkbox=(HWND)NULL;
				page_window->isolate_checkbox_width=0;
				page_window->isolate_checkbox_height=0;
#else /* defined (MIRADA) || defined (UNEMAP_1V1) */
				switch (page_window->unemap_hardware_version)
				{
					case UnEmap_1V2:
					default:
					{
						DestroyWindow(page_window->isolate_checkbox);
						page_window->isolate_checkbox=(HWND)NULL;
						page_window->isolate_checkbox_width=0;
						page_window->isolate_checkbox_height=0;
					} break;
					case UnEmap_2V1:
					case UnEmap_2V2:
					case UnEmap_2V1|UnEmap_2V2:
					{
						EnableWindow(page_window->isolate_checkbox,FALSE);
						GetWindowRect(page_window->isolate_checkbox,&rectangle);
						page_window->isolate_checkbox_width=rectangle.right-rectangle.left;
						page_window->isolate_checkbox_height=rectangle.bottom-rectangle.top;
					} break;
				}
#endif /* defined (MIRADA) || defined (UNEMAP_1V1) */
				/* low_pass_filter */
				(page_window->low_pass_filter).edit=GetDlgItem(window,
					LOW_PASS_FILTER_EDIT);
				(page_window->low_pass_filter).text=GetDlgItem(window,
					LOW_PASS_FILTER_TEXT);
#if defined (MIRADA) || defined (UNEMAP_1V1)
				DestroyWindow((page_window->low_pass_filter).edit);
				(page_window->low_pass_filter).edit=(HWND)NULL;
				(page_window->low_pass_filter).edit_width=0;
				(page_window->low_pass_filter).edit_height=0;
				DestroyWindow((page_window->low_pass_filter).text);
				(page_window->low_pass_filter).text=(HWND)NULL;
				(page_window->low_pass_filter).text_width=0;
				(page_window->low_pass_filter).text_height=0;
#else /* defined (MIRADA) || defined (UNEMAP_1V1) */
				EnableWindow((page_window->low_pass_filter).edit,FALSE);
				GetWindowRect((page_window->low_pass_filter).edit,&rectangle);
				(page_window->low_pass_filter).edit_width=
					rectangle.right-rectangle.left;
				(page_window->low_pass_filter).edit_height=
					rectangle.bottom-rectangle.top;
				/* replace the window procedure so that enters can be picked up */
				old_low_pass_filter_edit_wndproc=(WNDPROC)SetWindowLong(
					(page_window->low_pass_filter).edit,GWL_WNDPROC,
					(DWORD)low_pass_filter_edit_pick_up_enter);
				SetWindowLong((page_window->low_pass_filter).edit,GWL_USERDATA,
					(LONG)page_window);
				EnableWindow((page_window->low_pass_filter).text,FALSE);
				GetWindowRect((page_window->low_pass_filter).text,&rectangle);
				(page_window->low_pass_filter).text_width=
					rectangle.right-rectangle.left;
				(page_window->low_pass_filter).text_height=
					rectangle.bottom-rectangle.top;
#endif /* defined (MIRADA) || defined (UNEMAP_1V1) */
				/* maximum */
				(page_window->maximum).edit=GetDlgItem(window,MAXIMUM_EDIT);
				EnableWindow((page_window->maximum).edit,FALSE);
				GetWindowRect((page_window->maximum).edit,&rectangle);
				(page_window->maximum).edit_width=rectangle.right-rectangle.left;
				(page_window->maximum).edit_height=rectangle.bottom-rectangle.top;
				/* replace the window procedure so that enters can be picked up */
				old_maximum_edit_wndproc=(WNDPROC)SetWindowLong(
					(page_window->maximum).edit,GWL_WNDPROC,
					(DWORD)maximum_edit_pick_up_enter);
				SetWindowLong((page_window->maximum).edit,GWL_USERDATA,
					(LONG)page_window);
				(page_window->maximum).text=GetDlgItem(window,MAXIMUM_TEXT);
				EnableWindow((page_window->maximum).text,FALSE);
				GetWindowRect((page_window->maximum).text,&rectangle);
				(page_window->maximum).text_width=rectangle.right-rectangle.left;
				(page_window->maximum).text_height=rectangle.bottom-rectangle.top;
				/* minimum */
				(page_window->minimum).edit=GetDlgItem(window,MINIMUM_EDIT);
				EnableWindow((page_window->minimum).edit,FALSE);
				GetWindowRect((page_window->minimum).edit,&rectangle);
				(page_window->minimum).edit_width=rectangle.right-rectangle.left;
				(page_window->minimum).edit_height=rectangle.bottom-rectangle.top;
				/* replace the window procedure so that enters can be picked up */
				old_minimum_edit_wndproc=(WNDPROC)SetWindowLong(
					(page_window->minimum).edit,GWL_WNDPROC,
					(DWORD)minimum_edit_pick_up_enter);
				SetWindowLong((page_window->minimum).edit,GWL_USERDATA,
					(LONG)page_window);
				(page_window->minimum).text=GetDlgItem(window,MINIMUM_TEXT);
				EnableWindow((page_window->minimum).text,FALSE);
				GetWindowRect((page_window->minimum).text,&rectangle);
				(page_window->minimum).text_width=rectangle.right-rectangle.left;
				(page_window->minimum).text_height=rectangle.bottom-rectangle.top;
				/* pacing_button */
				page_window->pacing_button=GetDlgItem(window,PACING_BUTTON);
#if defined (MIRADA) || defined (UNEMAP_1V1)
				DestroyWindow(page_window->pacing_button);
				page_window->pacing_button=(HWND)NULL;
				page_window->pacing_button_width=0;
				page_window->pacing_button_height=0;
#else /* defined (MIRADA) || defined (UNEMAP_1V1) */
				switch (page_window->unemap_hardware_version)
				{
					case UnEmap_1V2:
					default:
					{
						DestroyWindow(page_window->pacing_button);
						page_window->pacing_button=(HWND)NULL;
						page_window->pacing_button_width=0;
						page_window->pacing_button_height=0;
					} break;
					case UnEmap_2V1:
					case UnEmap_2V2:
					case UnEmap_2V1|UnEmap_2V2:
					{
						if ((0<page_window->number_of_stimulators)&&restitution_time_pacing)
						{
							EnableWindow(page_window->pacing_button,FALSE);
							GetWindowRect(page_window->pacing_button,&rectangle);
							page_window->pacing_button_width=
								rectangle.right-rectangle.left;
							page_window->pacing_button_height=
								rectangle.bottom-rectangle.top;
						}
						else
						{
							DestroyWindow(page_window->pacing_button);
							page_window->pacing_button=(HWND)NULL;
							page_window->pacing_button_width=0;
							page_window->pacing_button_height=0;
						}
					} break;
				}
#endif /* defined (MIRADA) || defined (UNEMAP_1V1) */
				/* sample_checkbox */
				page_window->sample_checkbox=GetDlgItem(window,SAMPLE_CHECKBOX);
				EnableWindow(page_window->sample_checkbox,FALSE);
				GetWindowRect(page_window->sample_checkbox,&rectangle);
				page_window->sample_checkbox_width=rectangle.right-rectangle.left;
				page_window->sample_checkbox_height=rectangle.bottom-rectangle.top;
				/* save */
				(page_window->save).button=GetDlgItem(window,SAVE_BUTTON);
				EnableWindow((page_window->save).button,FALSE);
				GetWindowRect((page_window->save).button,&rectangle);
				(page_window->save).button_width=rectangle.right-rectangle.left;
				(page_window->save).button_height=rectangle.bottom-rectangle.top;
				(page_window->save).edit=GetDlgItem(window,SAVE_NUMBER_OF_SAMPLES_EDIT);
				EnableWindow((page_window->save).edit,FALSE);
				GetWindowRect((page_window->save).edit,&rectangle);
				(page_window->save).edit_width=rectangle.right-rectangle.left;
				(page_window->save).edit_height=rectangle.bottom-rectangle.top;
				/* scrolling_checkbox */
				page_window->scrolling_checkbox=GetDlgItem(window,SCROLLING_CHECKBOX);
				EnableWindow(page_window->scrolling_checkbox,FALSE);
				GetWindowRect(page_window->scrolling_checkbox,&rectangle);
				page_window->scrolling_checkbox_width=rectangle.right-rectangle.left;
				page_window->scrolling_checkbox_height=rectangle.bottom-rectangle.top;
				/* start_all_stimulators_button */
				page_window->start_all_stimulators_button=GetDlgItem(window,
					START_ALL_STIMULATORS_BUTTON);
#if defined (MIRADA) || defined (UNEMAP_1V1)
				DestroyWindow(page_window->start_all_stimulators_button);
				page_window->start_all_stimulators_button=(HWND)NULL;
				page_window->start_all_stimulators_button_width=0;
				page_window->start_all_stimulators_button_height=0;
#else /* defined (MIRADA) || defined (UNEMAP_1V1) */
				switch (page_window->unemap_hardware_version)
				{
					case UnEmap_1V2:
					default:
					{
						DestroyWindow(page_window->start_all_stimulators_button);
						page_window->start_all_stimulators_button=(HWND)NULL;
						page_window->start_all_stimulators_button_width=0;
						page_window->start_all_stimulators_button_height=0;
					} break;
					case UnEmap_2V1:
					case UnEmap_2V2:
					case UnEmap_2V1|UnEmap_2V2:
					{
						if ((0<page_window->number_of_stimulators)&&
							!restitution_time_pacing)
						{
							EnableWindow(page_window->start_all_stimulators_button,FALSE);
							GetWindowRect(page_window->start_all_stimulators_button,
								&rectangle);
							page_window->start_all_stimulators_button_width=
								rectangle.right-rectangle.left;
							page_window->start_all_stimulators_button_height=
								rectangle.bottom-rectangle.top;
						}
						else
						{
							DestroyWindow(page_window->start_all_stimulators_button);
							page_window->start_all_stimulators_button=(HWND)NULL;
							page_window->start_all_stimulators_button_width=0;
							page_window->start_all_stimulators_button_height=0;
						}
					} break;
				}
#endif /* defined (MIRADA) || defined (UNEMAP_1V1) */
				/* stimulate_checkbox */
				page_window->stimulate_checkbox=GetDlgItem(window,STIMULATE_CHECKBOX);
#if defined (MIRADA) || defined (UNEMAP_1V1)
				DestroyWindow(page_window->stimulate_checkbox);
				page_window->stimulate_checkbox=(HWND)NULL;
				page_window->stimulate_checkbox_width=0;
				page_window->stimulate_checkbox_height=0;
#else /* defined (MIRADA) || defined (UNEMAP_1V1) */
				switch (page_window->unemap_hardware_version)
				{
					case UnEmap_1V2:
					default:
					{
						DestroyWindow(page_window->stimulate_checkbox);
						page_window->stimulate_checkbox=(HWND)NULL;
						page_window->stimulate_checkbox_width=0;
						page_window->stimulate_checkbox_height=0;
					} break;
					case UnEmap_2V1:
					case UnEmap_2V2:
					case UnEmap_2V1|UnEmap_2V2:
					{
						if ((0<page_window->number_of_stimulators)&&
							!restitution_time_pacing)
						{
							EnableWindow(page_window->stimulate_checkbox,FALSE);
							GetWindowRect(page_window->stimulate_checkbox,&rectangle);
							page_window->stimulate_checkbox_width=
								rectangle.right-rectangle.left;
							page_window->stimulate_checkbox_height=
								rectangle.bottom-rectangle.top;
						}
						else
						{
							DestroyWindow(page_window->stimulate_checkbox);
							page_window->stimulate_checkbox=(HWND)NULL;
							page_window->stimulate_checkbox_width=0;
							page_window->stimulate_checkbox_height=0;
						}
					} break;
				}
#endif /* defined (MIRADA) || defined (UNEMAP_1V1) */
				/* stimulator_checkbox */
				page_window->stimulator_checkbox=GetDlgItem(window,STIMULATOR_CHECKBOX);
#if defined (MIRADA) || defined (UNEMAP_1V1)
				DestroyWindow(page_window->stimulator_checkbox);
				page_window->stimulator_checkbox=(HWND)NULL;
				page_window->stimulator_checkbox_width=0;
				page_window->stimulator_checkbox_height=0;
#else /* defined (MIRADA) || defined (UNEMAP_1V1) */
				switch (page_window->unemap_hardware_version)
				{
					case UnEmap_1V2:
					default:
					{
						DestroyWindow(page_window->stimulator_checkbox);
						page_window->stimulator_checkbox=(HWND)NULL;
						page_window->stimulator_checkbox_width=0;
						page_window->stimulator_checkbox_height=0;
					} break;
					case UnEmap_2V1:
					case UnEmap_2V2:
					case UnEmap_2V1|UnEmap_2V2:
					{
						if ((0<page_window->number_of_stimulators)&&
							!restitution_time_pacing)
						{
							EnableWindow(page_window->stimulator_checkbox,FALSE);
							GetWindowRect(page_window->stimulator_checkbox,&rectangle);
							page_window->stimulator_checkbox_width=
								rectangle.right-rectangle.left;
							page_window->stimulator_checkbox_height=
								rectangle.bottom-rectangle.top;
						}
						else
						{
							DestroyWindow(page_window->stimulator_checkbox);
							page_window->stimulator_checkbox=(HWND)NULL;
							page_window->stimulator_checkbox_width=0;
							page_window->stimulator_checkbox_height=0;
						}
					} break;
				}
#endif /* defined (MIRADA) || defined (UNEMAP_1V1) */
				/* stop_all_stimulators_button */
				page_window->stop_all_stimulators_button=GetDlgItem(window,
					STOP_ALL_STIMULATORS_BUTTON);
#if defined (MIRADA) || defined (UNEMAP_1V1)
				DestroyWindow(page_window->stop_all_stimulators_button);
				page_window->stop_all_stimulators_button=(HWND)NULL;
				page_window->stop_all_stimulators_button_width=0;
				page_window->stop_all_stimulators_button_height=0;
#else /* defined (MIRADA) || defined (UNEMAP_1V1) */
				switch (page_window->unemap_hardware_version)
				{
					case UnEmap_1V2:
					default:
					{
						DestroyWindow(page_window->stop_all_stimulators_button);
						page_window->stop_all_stimulators_button=(HWND)NULL;
						page_window->stop_all_stimulators_button_width=0;
						page_window->stop_all_stimulators_button_height=0;
					} break;
					case UnEmap_2V1:
					case UnEmap_2V2:
					case UnEmap_2V1|UnEmap_2V2:
					{
						if ((0<page_window->number_of_stimulators)&&
							!restitution_time_pacing)
						{
							EnableWindow(page_window->stop_all_stimulators_button,FALSE);
							GetWindowRect(page_window->stop_all_stimulators_button,
								&rectangle);
							page_window->stop_all_stimulators_button_width=
								rectangle.right-rectangle.left;
							page_window->stop_all_stimulators_button_height=
								rectangle.bottom-rectangle.top;
						}
						else
						{
							DestroyWindow(page_window->stop_all_stimulators_button);
							page_window->stop_all_stimulators_button=(HWND)NULL;
							page_window->stop_all_stimulators_button_width=0;
							page_window->stop_all_stimulators_button_height=0;
						}
					} break;
				}
#endif /* defined (MIRADA) || defined (UNEMAP_1V1) */
				/* test_checkbox */
				page_window->test_checkbox=GetDlgItem(window,TEST_CHECKBOX);
#if defined (MIRADA) || defined (UNEMAP_1V1)
				DestroyWindow(page_window->test_checkbox);
				page_window->test_checkbox=(HWND)NULL;
				page_window->test_checkbox_width=0;
				page_window->test_checkbox_height=0;
#else /* defined (MIRADA) || defined (UNEMAP_1V1) */
				switch (page_window->unemap_hardware_version)
				{
					case UnEmap_1V2:
					default:
					{
						DestroyWindow(page_window->test_checkbox);
						page_window->test_checkbox=(HWND)NULL;
						page_window->test_checkbox_width=0;
						page_window->test_checkbox_height=0;
					} break;
					case UnEmap_2V1:
					case UnEmap_2V2:
					case UnEmap_2V1|UnEmap_2V2:
					{
						EnableWindow(page_window->test_checkbox,FALSE);
						GetWindowRect(page_window->test_checkbox,&rectangle);
						page_window->test_checkbox_width=rectangle.right-rectangle.left;
						page_window->test_checkbox_height=rectangle.bottom-rectangle.top;
					} break;
				}
#endif /* defined (MIRADA) || defined (UNEMAP_1V1) */
#if defined (OLD_CODE)
#if !defined (MIRADA) && !defined (UNEMAP_1V1)
				if (0<page_window->number_of_stimulators)
				{
					update_stimulator(page_window,1);
				}
#endif /* !defined (MIRADA) && !defined (UNEMAP_1V1) */
#endif /* defined (OLD_CODE) */
				page_window->control_menu_width=
					page_window->experiment_checkbox_width+widget_spacing+
					page_window->sample_checkbox_width+widget_spacing+
					(page_window->save).button_width+
					(page_window->save).edit_width+widget_spacing+
					page_window->close_button_width;
				if (page_window->isolate_checkbox)
				{
					page_window->control_menu_width +=
						page_window->isolate_checkbox_width+widget_spacing;
				}
				if (page_window->calibrate_button)
				{
					page_window->control_menu_width +=
						page_window->calibrate_button_width+widget_spacing;
				}
				if (page_window->test_checkbox)
				{
					page_window->control_menu_width +=
						page_window->test_checkbox_width+widget_spacing;
				}
				if ((page_window->low_pass_filter).text)
				{
					page_window->control_menu_width += 
						(page_window->low_pass_filter).text_width+
						(page_window->low_pass_filter).edit_width+widget_spacing;
				}
				page_window->all_devices_menu_width=
					page_window->auto_range_button_width+widget_spacing+
					page_window->full_range_button_width;
				if (page_window->start_all_stimulators_button)
				{
					page_window->all_devices_menu_width +=
						page_window->start_all_stimulators_button_width+widget_spacing;
				}
				if (page_window->stop_all_stimulators_button)
				{
					page_window->all_devices_menu_width +=
						page_window->stop_all_stimulators_button_width+widget_spacing;
				}
				if (page_window->pacing_button)
				{
					page_window->all_devices_menu_width +=
						page_window->pacing_button_width+widget_spacing;
				}
				page_window->current_device_menu_width=
					(page_window->electrode).text_width+
					(page_window->electrode).arrows_width+
					(page_window->electrode).edit_width+widget_spacing+
					page_window->scrolling_checkbox_width+widget_spacing+
					(page_window->gain).text_width+
					(page_window->gain).edit_width+widget_spacing+
					(page_window->minimum).text_width+
					(page_window->minimum).edit_width+widget_spacing+
					(page_window->maximum).text_width+
					(page_window->maximum).edit_width+widget_spacing;
				if (page_window->stimulate_checkbox)
				{
					page_window->current_device_menu_width +=
						page_window->stimulate_checkbox_width+widget_spacing;
				}
				if (page_window->stimulator_checkbox)
				{
					page_window->current_device_menu_width +=
						page_window->stimulator_checkbox_width+widget_spacing;
				}
				/* check if the drawing area class is registered */
				if (TRUE!=(win32_return_code=GetClassInfoEx(page_window->instance,
					class_name,&class_information)))
				{
					class_information.cbClsExtra=0;
					class_information.cbWndExtra=sizeof(struct Page_window *);
					class_information.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
					class_information.hCursor=LoadCursor(NULL,IDC_ARROW);
					class_information.hIcon=(HICON)NULL;
					class_information.hInstance=page_window->instance;
					class_information.lpfnWndProc=Page_window_scrolling_area_class_proc;
					class_information.lpszClassName=class_name;
					class_information.style=CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
					/* allow resource to specify the menu */
					class_information.lpszMenuName=NULL;
					/*???DB.  Extra in WNDCLASSEX over WNDCLASS */
					class_information.cbSize=sizeof(WNDCLASSEX);
					class_information.hIconSm=(HICON)NULL;
					if (RegisterClassEx(&class_information))
					{
						win32_return_code=TRUE;
					}
				}
				/* create the window */
				if (TRUE==win32_return_code)
				{
					GetClientRect(window,&rectangle);
					if (CreateWindowEx(
						WS_EX_OVERLAPPEDWINDOW,
							/* extended window styles */
						class_name,
							/* class name */
						"",
							/* window name */
							/*???DB.  Is this what goes in the title bar ? */
						WS_CHILD|WS_VISIBLE,
							/* window styles */
						0,
							/* horizontal position */
						25,
							/* vertical position */
						rectangle.right-rectangle.left,
							/* width */
						rectangle.bottom-rectangle.top-25,
							/* height */
						window,
							/* parent or owner window */
						(HMENU)NULL,
							/* menu to be used - use class menu */
						page_window->instance,
							/* instance handle */
						page_window
							/* window creation data */
							/*???DB.  Like to have page_window */
						))
					{
						/* make the page window fill the whole screen */
						RECT screen_rectangle;

						if (TRUE==SystemParametersInfo(SPI_GETWORKAREA,0,&screen_rectangle,
							0))
						{
							MoveWindow(window,0,0,
								screen_rectangle.right-screen_rectangle.left,
								screen_rectangle.bottom-screen_rectangle.top,
								TRUE);
							/* set the display minimum and maximum (gains may be changed
								while creating scrolling_area) */
							if (page_window->display_device)
							{
								show_display_maximum(page_window);
								show_display_minimum(page_window);
								show_display_gain(page_window);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Page_window_class_proc.  Could not get system info");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Page_window_class_proc.  Could not create drawing area");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Page_window_class_proc.  Unable to register class information");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Page_window_class_proc.  Missing page_window");
			}
			return_code=DefDlgProc(window,message_identifier,first_message,
				second_message);
		} break;
		case WM_NOTIFY:
		{
			if ((ELECTRODE_ARROWS==(int)first_message)&&
				(UDN_DELTAPOS==((LPNMHDR)second_message)->code))
			{
				if (0<((NM_UPDOWN FAR *)second_message)->iDelta)
				{
					increment_electrode(GetWindowLong(window,DLGWINDOWEXTRA));
				}
				else
				{
					decrement_electrode(GetWindowLong(window,DLGWINDOWEXTRA));
				}
			}
#if defined (OLD_CODE)
			else
			{
				if ((STIMULATE_CHANNEL_ARROWS==(int)first_message)&&
					(UDN_DELTAPOS==((LPNMHDR)second_message)->code))
				{
					if (0<((NM_UPDOWN FAR *)second_message)->iDelta)
					{
						increment_stimulate(GetWindowLong(window,DLGWINDOWEXTRA));
					}
					else
					{
						decrement_stimulate(GetWindowLong(window,DLGWINDOWEXTRA));
					}
				}
				else
				{
					if ((STIMULATOR_ARROWS==(int)first_message)&&
						(UDN_DELTAPOS==((LPNMHDR)second_message)->code))
					{
						if (0<((NM_UPDOWN FAR *)second_message)->iDelta)
						{
							increment_stimulator(GetWindowLong(window,DLGWINDOWEXTRA));
						}
						else
						{
							decrement_stimulator(GetWindowLong(window,DLGWINDOWEXTRA));
						}
					}
				}
			}
#endif /* defined (OLD_CODE) */
		} break;
		case WM_PAINT:
		{
			HWND calibrate_button,isolate_checkbox,low_pass_filter_edit,pacing_button,
				start_all_stimulators_button,stimulate_checkbox,stimulator_checkbox,
				stop_all_stimulators_button,test_checkbox;
			int auto_range_button_width,calibrate_button_width,close_button_width,
				electrode_arrows_width,electrode_edit_width,electrode_text_width,
				full_range_button_width,experiment_checkbox_width,gain_edit_width,
				gain_text_width,isolate_checkbox_width,low_pass_filter_edit_width,
				low_pass_filter_text_width,maximum_edit_width,maximum_text_width,
				menu_bar_height,menu_bar_width,minimum_edit_width,minimum_text_width,
				number_of_widgets,pacing_button_width,sample_checkbox_width,
				save_button_width,save_edit_width,scrolling_checkbox_width,
				start_all_stimulators_button_width,stimulate_checkbox_width,
				stimulator_checkbox_width,stop_all_stimulators_button_width,
				test_checkbox_width,widget_spacing;
			RECT rectangle;
			struct Page_window *page_window;

			if (page_window=(struct Page_window *)GetWindowLong(window,
				DLGWINDOWEXTRA))
			{
				GetClientRect(window,&rectangle);
				/* resize menu bar */
				menu_bar_height=0;
				/* control_menu */
				widget_spacing=page_window->user_interface->widget_spacing;
				number_of_widgets=0;
				experiment_checkbox_width=page_window->experiment_checkbox_width;
				number_of_widgets++;
				if (isolate_checkbox=page_window->isolate_checkbox)
				{
					isolate_checkbox_width=page_window->isolate_checkbox_width;
					number_of_widgets++;
				}
				sample_checkbox_width=page_window->sample_checkbox_width;
				number_of_widgets++;
				save_button_width=(page_window->save).button_width;
				save_edit_width=(page_window->save).edit_width;
				number_of_widgets++;
				if (calibrate_button=page_window->calibrate_button)
				{
					calibrate_button_width=page_window->calibrate_button_width;
					number_of_widgets++;
				}
				if (test_checkbox=page_window->test_checkbox)
				{
					test_checkbox_width=page_window->test_checkbox_width;
					number_of_widgets++;
				}
				if (low_pass_filter_edit=(page_window->low_pass_filter).edit)
				{
					low_pass_filter_edit_width=(page_window->low_pass_filter).edit_width;
					low_pass_filter_text_width=(page_window->low_pass_filter).text_width;
					number_of_widgets++;
				}
				close_button_width=page_window->close_button_width;
				number_of_widgets++;
				if (0<number_of_widgets)
				{
					menu_bar_width=page_window->control_menu_width;
					if (menu_bar_width>rectangle.right-rectangle.left)
					{
						if (menu_bar_width-(number_of_widgets-1)*widget_spacing>
							rectangle.right-rectangle.left)
						{
							menu_bar_width -= (number_of_widgets-1)*widget_spacing;
							widget_spacing=0;
							experiment_checkbox_width=((rectangle.right-rectangle.left)*
								experiment_checkbox_width)/menu_bar_width;
							if (isolate_checkbox)
							{
								isolate_checkbox_width=((rectangle.right-rectangle.left)*
									isolate_checkbox_width)/menu_bar_width;
							}
							sample_checkbox_width=((rectangle.right-rectangle.left)*
								sample_checkbox_width)/menu_bar_width;
							save_button_width=((rectangle.right-rectangle.left)*
								save_button_width)/menu_bar_width;
							save_edit_width=((rectangle.right-rectangle.left)*
								save_edit_width)/menu_bar_width;
							if (calibrate_button)
							{
								calibrate_button_width=((rectangle.right-rectangle.left)*
									calibrate_button_width)/menu_bar_width;
							}
							if (test_checkbox)
							{
								test_checkbox_width=((rectangle.right-rectangle.left)*
									test_checkbox_width)/menu_bar_width;
							}
							if (low_pass_filter_edit)
							{
								low_pass_filter_text_width=((rectangle.right-rectangle.left)*
									(low_pass_filter_edit_width+low_pass_filter_text_width))/
									menu_bar_width-low_pass_filter_edit_width;
								if (low_pass_filter_text_width<0)
								{
									low_pass_filter_edit_width += low_pass_filter_text_width;
									low_pass_filter_text_width=0;
								}
							}
							close_button_width=((rectangle.right-rectangle.left)*
								close_button_width)/menu_bar_width;
						}
						else
						{
							widget_spacing=(rectangle.right-rectangle.left+
								(number_of_widgets-1)*widget_spacing-menu_bar_width)/
								(number_of_widgets-1);
						}
					}
					menu_bar_width=0;
					MoveWindow(page_window->experiment_checkbox,menu_bar_width,
						menu_bar_height+2,experiment_checkbox_width,
						page_window->experiment_checkbox_height,TRUE);
					menu_bar_width += widget_spacing+experiment_checkbox_width;
					if (isolate_checkbox)
					{
						MoveWindow(isolate_checkbox,menu_bar_width,
							menu_bar_height+2,isolate_checkbox_width,
							page_window->isolate_checkbox_height,TRUE);
						menu_bar_width += widget_spacing+isolate_checkbox_width;
					}
					MoveWindow(page_window->sample_checkbox,menu_bar_width,
						menu_bar_height+2,sample_checkbox_width,
						page_window->sample_checkbox_height,TRUE);
					menu_bar_width += widget_spacing+sample_checkbox_width;
					MoveWindow((page_window->save).button,menu_bar_width,
						menu_bar_height+2,save_button_width,
						(page_window->save).button_height,TRUE);
					menu_bar_width += save_button_width;
					MoveWindow((page_window->save).edit,menu_bar_width,
						menu_bar_height+2,save_edit_width,(page_window->save).edit_height,
						TRUE);
					menu_bar_width += widget_spacing+save_edit_width;
					if (calibrate_button)
					{
						MoveWindow(calibrate_button,menu_bar_width,
							menu_bar_height+2,calibrate_button_width,
							page_window->calibrate_button_height,TRUE);
						menu_bar_width += widget_spacing+calibrate_button_width;
					}
					if (test_checkbox)
					{
						MoveWindow(page_window->test_checkbox,menu_bar_width,
							menu_bar_height+2,test_checkbox_width,
							page_window->test_checkbox_height,TRUE);
						menu_bar_width += widget_spacing+test_checkbox_width;
					}
					if (low_pass_filter_edit)
					{
						MoveWindow((page_window->low_pass_filter).text,menu_bar_width,
							menu_bar_height+3,low_pass_filter_text_width,
							(page_window->low_pass_filter).text_height,TRUE);
						menu_bar_width += low_pass_filter_text_width;
						MoveWindow((page_window->low_pass_filter).edit,menu_bar_width,
							menu_bar_height+2,low_pass_filter_edit_width,
							(page_window->low_pass_filter).edit_height,TRUE);
						menu_bar_width += widget_spacing+low_pass_filter_edit_width;
					}
					MoveWindow(page_window->close_button,
						rectangle.right-rectangle.left-close_button_width,
						menu_bar_height+2,close_button_width,
						page_window->close_button_height,TRUE);
					menu_bar_height += 25;
				}
				/* all_devices_menu */
				widget_spacing=page_window->user_interface->widget_spacing;
				number_of_widgets=0;
				auto_range_button_width=page_window->auto_range_button_width;
				number_of_widgets++;
				full_range_button_width=page_window->full_range_button_width;
				number_of_widgets++;
				if (start_all_stimulators_button=
					page_window->start_all_stimulators_button)
				{
					start_all_stimulators_button_width=
						page_window->start_all_stimulators_button_width;
					number_of_widgets++;
				}
				if (stop_all_stimulators_button=
					page_window->stop_all_stimulators_button)
				{
					stop_all_stimulators_button_width=
						page_window->stop_all_stimulators_button_width;
					number_of_widgets++;
				}
				if (pacing_button=page_window->pacing_button)
				{
					pacing_button_width=page_window->pacing_button_width;
					number_of_widgets++;
				}
				if (0<number_of_widgets)
				{
					menu_bar_width=page_window->all_devices_menu_width;
					if (menu_bar_width>rectangle.right-rectangle.left)
					{
						if (menu_bar_width-(number_of_widgets-1)*widget_spacing>
							rectangle.right-rectangle.left)
						{
							menu_bar_width -= (number_of_widgets-1)*widget_spacing;
							widget_spacing=0;
							auto_range_button_width=((rectangle.right-rectangle.left)*
								auto_range_button_width)/menu_bar_width;
							full_range_button_width=((rectangle.right-rectangle.left)*
								full_range_button_width)/menu_bar_width;
							if (start_all_stimulators_button)
							{
								start_all_stimulators_button_width=
									((rectangle.right-rectangle.left)*
									start_all_stimulators_button_width)/menu_bar_width;
							}
							if (stop_all_stimulators_button)
							{
								stop_all_stimulators_button_width=
									((rectangle.right-rectangle.left)*
									stop_all_stimulators_button_width)/menu_bar_width;
							}
							if (pacing_button)
							{
								pacing_button_width=((rectangle.right-rectangle.left)*
									pacing_button_width)/menu_bar_width;
							}
						}
						else
						{
							widget_spacing=(rectangle.right-rectangle.left+
								(number_of_widgets-1)*widget_spacing-menu_bar_width)/
								(number_of_widgets-1);
						}
					}
					menu_bar_width=0;
					MoveWindow(page_window->auto_range_button,menu_bar_width,
						menu_bar_height+2,auto_range_button_width,
						page_window->auto_range_button_height,TRUE);
					menu_bar_width += widget_spacing+auto_range_button_width;
					MoveWindow(page_window->full_range_button,menu_bar_width,
						menu_bar_height+2,full_range_button_width,
						page_window->full_range_button_height,TRUE);
					menu_bar_width += widget_spacing+full_range_button_width;
					if (start_all_stimulators_button)
					{
						MoveWindow(start_all_stimulators_button,menu_bar_width,
							menu_bar_height+2,start_all_stimulators_button_width,
							page_window->start_all_stimulators_button_height,TRUE);
						menu_bar_width += widget_spacing+start_all_stimulators_button_width;
					}
					if (stop_all_stimulators_button)
					{
						MoveWindow(stop_all_stimulators_button,menu_bar_width,
							menu_bar_height+2,stop_all_stimulators_button_width,
							page_window->stop_all_stimulators_button_height,TRUE);
						menu_bar_width += widget_spacing+stop_all_stimulators_button_width;
					}
					if (pacing_button)
					{
						MoveWindow(pacing_button,menu_bar_width,menu_bar_height+2,
							pacing_button_width,page_window->pacing_button_height,TRUE);
						menu_bar_width += widget_spacing+pacing_button_width;
					}
					menu_bar_height += 25;
				}
				/* current_device_menu */
				widget_spacing=page_window->user_interface->widget_spacing;
				number_of_widgets=0;
				electrode_arrows_width=(page_window->electrode).arrows_width;
				electrode_edit_width=(page_window->electrode).edit_width;
				electrode_text_width=(page_window->electrode).text_width;
				number_of_widgets++;
				scrolling_checkbox_width=page_window->scrolling_checkbox_width;
				number_of_widgets++;
				gain_edit_width=(page_window->gain).edit_width;
				gain_text_width=(page_window->gain).text_width;
				number_of_widgets++;
				maximum_edit_width=(page_window->maximum).edit_width;
				maximum_text_width=(page_window->maximum).text_width;
				number_of_widgets++;
				minimum_edit_width=(page_window->minimum).edit_width;
				minimum_text_width=(page_window->minimum).text_width;
				number_of_widgets++;
				if (stimulate_checkbox=page_window->stimulate_checkbox)
				{
					stimulate_checkbox_width=page_window->stimulate_checkbox_width;
					number_of_widgets++;
				}
				if (stimulator_checkbox=page_window->stimulator_checkbox)
				{
					stimulator_checkbox_width=page_window->stimulator_checkbox_width;
					number_of_widgets++;
				}
				if (0<number_of_widgets)
				{
					menu_bar_width=page_window->current_device_menu_width;
					if (menu_bar_width>rectangle.right-rectangle.left)
					{
						if (menu_bar_width-(number_of_widgets-1)*widget_spacing>
							rectangle.right-rectangle.left)
						{
							menu_bar_width -= (number_of_widgets-1)*widget_spacing;
							widget_spacing=0;
							electrode_text_width=((rectangle.right-rectangle.left)*
								(electrode_arrows_width+electrode_edit_width+
								electrode_text_width))/menu_bar_width-
								(electrode_arrows_width+electrode_edit_width);
							if (electrode_text_width<0)
							{
								electrode_arrows_width += electrode_text_width;
								electrode_text_width=0;
								if (electrode_arrows_width<0)
								{
									electrode_edit_width += electrode_arrows_width;
									electrode_arrows_width=0;
								}
							}
							scrolling_checkbox_width=((rectangle.right-rectangle.left)*
								scrolling_checkbox_width)/menu_bar_width;
							gain_text_width=((rectangle.right-rectangle.left)*
								(gain_edit_width+gain_text_width))/menu_bar_width-
								gain_edit_width;
							if (gain_text_width<0)
							{
								gain_edit_width += gain_text_width;
								gain_text_width=0;
							}
							maximum_text_width=((rectangle.right-rectangle.left)*
								(maximum_edit_width+maximum_text_width))/menu_bar_width-
								maximum_edit_width;
							if (maximum_text_width<0)
							{
								maximum_edit_width += maximum_text_width;
								maximum_text_width=0;
							}
							minimum_text_width=((rectangle.right-rectangle.left)*
								(minimum_edit_width+minimum_text_width))/menu_bar_width-
								minimum_edit_width;
							if (minimum_text_width<0)
							{
								minimum_edit_width += minimum_text_width;
								minimum_text_width=0;
							}
							if (stimulate_checkbox)
							{
								stimulate_checkbox_width=((rectangle.right-rectangle.left)*
									stimulate_checkbox_width)/menu_bar_width;
							}
							if (stimulator_checkbox)
							{
								stimulator_checkbox_width=((rectangle.right-rectangle.left)*
									stimulator_checkbox_width)/menu_bar_width;
							}
						}
						else
						{
							widget_spacing=(rectangle.right-rectangle.left+
								(number_of_widgets-1)*widget_spacing-menu_bar_width)/
								(number_of_widgets-1);
						}
					}
					menu_bar_width=0;
					MoveWindow((page_window->electrode).text,menu_bar_width,
						menu_bar_height+3,electrode_text_width,
						(page_window->electrode).text_height,TRUE);
					menu_bar_width += electrode_text_width;
					MoveWindow((page_window->electrode).edit,menu_bar_width,
						menu_bar_height+2,electrode_edit_width,
						(page_window->electrode).edit_height,TRUE);
					menu_bar_width += electrode_edit_width;
					MoveWindow((page_window->electrode).arrows,menu_bar_width,
						menu_bar_height+2,electrode_arrows_width,
						(page_window->electrode).arrows_height,TRUE);
					menu_bar_width += widget_spacing+electrode_arrows_width;
					MoveWindow(page_window->scrolling_checkbox,menu_bar_width,
						menu_bar_height+2,scrolling_checkbox_width,
						page_window->scrolling_checkbox_height,TRUE);
					menu_bar_width += widget_spacing+scrolling_checkbox_width;
					MoveWindow((page_window->gain).text,menu_bar_width,
						menu_bar_height+3,gain_text_width,(page_window->gain).text_height,
						TRUE);
					menu_bar_width += gain_text_width;
					MoveWindow((page_window->gain).edit,menu_bar_width,
						menu_bar_height+2,gain_edit_width,(page_window->gain).edit_height,
						TRUE);
					menu_bar_width += widget_spacing+gain_edit_width;
					MoveWindow((page_window->minimum).text,menu_bar_width,
						menu_bar_height+3,minimum_text_width,
						(page_window->minimum).text_height,TRUE);
					menu_bar_width += minimum_text_width;
					MoveWindow((page_window->minimum).edit,menu_bar_width,
						menu_bar_height+2,minimum_edit_width,
						(page_window->minimum).edit_height,TRUE);
					menu_bar_width += widget_spacing+minimum_edit_width;
					MoveWindow((page_window->maximum).text,menu_bar_width,
						menu_bar_height+3,maximum_text_width,
						(page_window->maximum).text_height,TRUE);
					menu_bar_width += maximum_text_width;
					MoveWindow((page_window->maximum).edit,menu_bar_width,
						menu_bar_height+2,maximum_edit_width,
						(page_window->maximum).edit_height,TRUE);
					menu_bar_width += widget_spacing+maximum_edit_width;
					if (stimulate_checkbox)
					{
						MoveWindow(page_window->stimulate_checkbox,menu_bar_width,
							menu_bar_height+2,stimulate_checkbox_width,
							page_window->stimulate_checkbox_height,TRUE);
						menu_bar_width += widget_spacing+stimulate_checkbox_width;
					}
					if (stimulator_checkbox)
					{
						MoveWindow(page_window->stimulator_checkbox,menu_bar_width,
							menu_bar_height+2,stimulator_checkbox_width,
							page_window->stimulator_checkbox_height,TRUE);
						menu_bar_width += widget_spacing+stimulator_checkbox_width;
					}
					menu_bar_height += 25;
				}
				/* resize drawing area */
				MoveWindow(page_window->scrolling_area,0,menu_bar_height,
					rectangle.right-rectangle.left,
					rectangle.bottom-rectangle.top-menu_bar_height,TRUE);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Page_window_class_proc.  Missing page_window");
			}
			return_code=DefDlgProc(window,message_identifier,first_message,
				second_message);
		} break;
		default:
		{
			return_code=DefDlgProc(window,message_identifier,first_message,
				second_message);
		} break;
	}
	LEAVE;

	return (return_code);
} /* Page_window_class_proc */
#endif /* defined (WINDOWS) */

#if defined (WINDOWS)
static BOOL CALLBACK Page_window_dialog_proc(HWND window,
	UINT message_identifier,WPARAM first_message,LPARAM second_message)
/*******************************************************************************
LAST MODIFIED : 19 April 1997

DESCRIPTION :
???DB.  All in the class proc
==============================================================================*/
{
	BOOL return_code;

	ENTER(Page_window_dialog_proc);
	return_code=FALSE;
	LEAVE;

	return (return_code);
} /* Page_window_dialog_proc */
#endif /* defined (WINDOWS) */

#if defined (MOTIF)
static void destroy_Page_window_callback(Widget *widget_id,
	XtPointer page_window_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
==============================================================================*/
{
	struct Page_window *page_window;

	ENTER(destroy_Page_window_callback);
	USE_PARAMETER(widget_id);
	USE_PARAMETER(call_data);
	if (page_window=(struct Page_window *)page_window_void)
	{
		destroy_Page_window(&page_window);
	}
	LEAVE;
} /* destroy_Page_window_callback */
#endif /* defined (MOTIF) */

/*
Global functions
----------------
*/
int destroy_Page_window(struct Page_window **page_window_address)
/*******************************************************************************
LAST MODIFIED : 10 October 2001

DESCRIPTION :
If the <address> field of the page window is not NULL, <*address> is set to
NULL.  If the <activation> field is not NULL, the <activation> widget is
unghosted.  The function frees the memory associated with the fields of the
page window and frees the memory associated with the page window.
==============================================================================*/
{
	int i,return_code;
	struct Page_window *page_window;

	ENTER(destroy_Page_window);
#if defined (DEBUG)
/*???debug */
printf("enter destroy_Page_window\n");
#endif /* defined (DEBUG) */
	return_code=0;
	if (page_window_address&&(page_window= *page_window_address))
	{
		return_code=1;
#if defined (MIRADA)
/*???debug */
unsigned long interrupt_count,start;

if (INVALID_HANDLE_VALUE!=page_window->device_driver)
{
	stop_interrupting(page_window->device_driver,&interrupt_count,&start);
}
#else /* defined (MIRADA) */
		unemap_stop_sampling();
		unemap_stop_stimulating(0);
		unemap_stop_calibrating(0);
		unemap_set_isolate_record_mode(0,1);
		unemap_set_power(0);
		unemap_deconfigure();
#endif /* defined (MIRADA) */
		if (page_window->pacing_window)
		{
			destroy_Pacing_window(&(page_window->pacing_window));
		}
		/* set the pointer to the page window to NULL */
		if (page_window->address)
		{
			*(page_window->address)=(struct Page_window *)NULL;
		}
#if defined (MOTIF)
		/* unghost the activation button */
		if (page_window->activation)
		{
			XtSetSensitive(page_window->activation,True);
		}
#endif /* defined (MOTIF) */
		/* free the page window memory */
#if defined (OLD_CODE)
		DEALLOCATE(page_window->stimulate_devices);
		DEALLOCATE(page_window->stimulate_device_numbers);
#endif /* defined (OLD_CODE) */
		for (i=0;i<page_window->number_of_stimulators;i++)
		{
			DEALLOCATE(((page_window->stimulators)[i]).channel_numbers);
			DEALLOCATE(((page_window->stimulators)[i]).end_values);
		}
		DEALLOCATE(page_window->stimulators);
		DEALLOCATE(page_window->scrolling_channels);
		DEALLOCATE(page_window->scrolling_devices);
		DEALLOCATE(page_window->number_of_scrolling_channels_device);
		DEALLOCATE(page_window->last_scrolling_value_device);
		DEALLOCATE(page_window->scrolling_coefficients);
		DEALLOCATE(page_window);
		*page_window_address=(struct Page_window *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_Page_window.  Missing page_window");
	}
	LEAVE;

	return (return_code);
} /* destroy_Page_window */

#if defined (MOTIF)
Widget get_page_window_close_button(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
==============================================================================*/
{
	Widget close_button;

	ENTER(get_page_window_close_button);
	close_button=(Widget)NULL;
	if (page_window)
	{
		close_button=page_window->close_button;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_page_window_close_button.  Missing page_window");
	}
	LEAVE;

	return (close_button);
} /* get_page_window_close_button */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
Widget create_page_window_shell(Widget *address,Widget parent,
	int screen_width,int screen_height,struct User_interface *user_interface)
		/*???DB.  Position and size should be passed ? */
/*******************************************************************************
LAST MODIFIED : 16 September 1999

DESCRIPTION :
Creates a popup shell widget for an page window.  If <address> is not NULL,
<*address> is set to the id of the created shell and on destruction <*address>
will be set to NULL.  The id of the created widget is returned.
???If address is NULL, it won't create an entry in the shell list ?
==============================================================================*/
{
	Widget shell;

	ENTER(create_page_window_shell);
	/* create and place the page window shell */
	if (shell=XtVaCreatePopupShell("page_window_shell",
		topLevelShellWidgetClass,parent,
		XmNallowShellResize,False,
		XmNx,0,
		XmNy,0,
		XmNwidth,screen_width/2,
		XmNheight,screen_height/2,
		XmNuserData,user_interface,
		NULL))
	{
		if (address)
		{
			*address=shell;
			/* add the destroy callback */
			XtAddCallback(shell,XmNdestroyCallback,destroy_window_shell,
				(XtPointer)create_Shell_list_item(address,user_interface));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_page_window_shell.  Could create the page window shell");
	}
	LEAVE;

	return (shell);
} /* create_page_window_shell */
#endif /* defined (MOTIF) */

struct Page_window *create_Page_window(struct Page_window **address,
#if defined (MOTIF)
	Widget activation,Widget parent,
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	HWND parent,
#endif /* defined (WINDOWS) */
	struct Rig **rig_address,
#if defined (MOTIF)
	Pixel identifying_colour,
#endif /* defined (MOTIF) */
	struct Mapping_window **mapping_window_address,int pointer_sensitivity,
	char *signal_file_extension_write,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
This function allocates the memory for a page window and sets the fields to the
specified values (<address>, <activation>, <page_address>).  It then retrieves
a page window widget with the specified <parent> and assigns the widget ids to
the appropriate fields of the structure.  If successful it returns a pointer to
the created page window and, if <address> is not NULL, makes <*address> point to
the created page window.  If unsuccessful, NULL is returned.
==============================================================================*/
{
	char *hardware_directory;
	float channel_gain,channel_offset,post_filter_gain,pre_filter_gain;
	int channel_number,device_number,i,j,number_of_devices;
	Page_window_settings settings;
	struct Auxiliary_properties *auxiliary_properties;
	struct Channel *channel;
	struct Device **device_address,*display_device;
	struct Page_window *page_window;
#if defined (MOTIF)
	Display *display;
	int depth;
	MrmType page_window_class;
	Pixmap depth_screen_drawable;
	static MrmRegisterArg callback_list[]=
	{
		{"decrement_electrode",(XtPointer)decrement_electrode},
		{"electrode_stimulate_record_call",
			(XtPointer)electrode_stimulate_record_call},
		{"expose_page_scrolling_area",(XtPointer)expose_page_scrolling_area},
		{"identify_page_calibrate_button",
			(XtPointer)identify_page_calibrate_button},
		{"identify_page_auto_range_button",
			(XtPointer)identify_page_auto_range_button},
		{"identify_page_close_button",(XtPointer)identify_page_close_button},
		{"identify_page_electrode_form",(XtPointer)identify_page_electrode_form},
		{"identify_page_electrode_value",(XtPointer)identify_page_electrode_value},
		{"identify_page_experiment_checkb",
			(XtPointer)identify_page_experiment_checkb},
		{"identify_page_full_range_button",
			(XtPointer)identify_page_full_range_button},
		{"identify_page_gain_form",(XtPointer)identify_page_gain_form},
		{"identify_page_gain_value",(XtPointer)identify_page_gain_value},
		{"identify_page_isolate_checkbox",
			(XtPointer)identify_page_isolate_checkbox},
		{"identify_page_low_pass_form",(XtPointer)identify_page_low_pass_form},
		{"identify_page_low_pass_value",(XtPointer)identify_page_low_pass_value},
		{"identify_page_maximum_form",(XtPointer)identify_page_maximum_form},
		{"identify_page_maximum_value",(XtPointer)identify_page_maximum_value},
		{"identify_page_minimum_form",(XtPointer)identify_page_minimum_form},
		{"identify_page_minimum_value",(XtPointer)identify_page_minimum_value},
		{"identify_page_pacing_button",(XtPointer)identify_page_pacing_button},
		{"identify_page_sample_checkbox",(XtPointer)identify_page_sample_checkbox},
		{"identify_page_save_button",(XtPointer)identify_page_save_button},
		{"identify_page_save_form",(XtPointer)identify_page_save_form},
		{"identify_page_save_number_of_sa",
			(XtPointer)identify_page_save_number_of_sa},
		{"identify_page_scrolling_area",(XtPointer)identify_page_scrolling_area},
		{"identify_page_scrolling_checkbo",
			(XtPointer)identify_page_scrolling_checkbo},
		{"identify_page_start_all_stimula",
			(XtPointer)identify_page_start_all_stimula},
		{"identify_page_stimulate_checkbo",
			(XtPointer)identify_page_stimulate_checkbo},
		{"identify_page_stimulator_checkb",
			(XtPointer)identify_page_stimulator_checkb},
		{"identify_page_stop_all_stimulat",
			(XtPointer)identify_page_stop_all_stimulat},
		{"identify_page_test_checkbox",(XtPointer)identify_page_test_checkbox},
		{"increment_electrode",(XtPointer)increment_electrode},
		{"open_Pacing_window_callback",(XtPointer)open_Pacing_window_callback},
		{"page_auto_range",(XtPointer)page_auto_range},
		{"page_full_range",(XtPointer)page_full_range},
		{"page_save_data",(XtPointer)page_save_data},
		{"page_start_all_stimulators",(XtPointer)page_start_all_stimulators},
		{"page_stop_all_stimulators",(XtPointer)page_stop_all_stimulators},
		{"resize_page_scrolling_area",(XtPointer)resize_page_scrolling_area},
		{"start_calibrating",(XtPointer)start_calibrating},
		{"start_stop_experiment_callback",
			(XtPointer)start_stop_experiment_callback},
		{"start_stop_isolating_callback",(XtPointer)start_stop_isolating_callback},
		{"start_stop_sampling_callback",(XtPointer)start_stop_sampling_callback},
		{"start_stop_testing_callback",(XtPointer)start_stop_testing_callback},
		{"stimulator_start_stop_callback",
			(XtPointer)stimulator_start_stop_callback},
		{"update_display_device_callback",
			(XtPointer)update_display_device_callback},
		{"update_display_gain_callback",
			(XtPointer)update_display_gain_callback},
		{"update_display_maximum_callback",
			(XtPointer)update_display_maximum_callback},
		{"update_display_minimum_callback",
			(XtPointer)update_display_minimum_callback},
		{"update_filter_callback",(XtPointer)update_filter_callback},
		{"update_save_number_of_samples_c",
			(XtPointer)update_save_number_of_samples_c},
		{"destroy_Page_window_callback",(XtPointer)destroy_Page_window_callback}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"page_window_structure",(XtPointer)NULL},
		{"identifying_colour",(XtPointer)NULL}
	};
#define XmNacquisitionIntervalColour "acquisitionIntervalColour"
#define XmCAcquisitionIntervalColour "AcquisitionIntervalColour"
#define XmNcalibrationDirectory "calibrationDirectory"
#define XmCCalibrationDirectory "CalibrationDirectory"
#define XmNdrawingBackgroundColour "drawingBackgroundColour"
#define XmCDrawingBackgroundColour "DrawingBackgroundColour"
#define XmNinitialGain "initialGain"
#define XmCInitialGain "InitialGain"
#define XmNnumberOfSamples "numberOfSamples"
#define XmCNumberOfSamples "NumberOfSamples"
#define XmNrestitutionTimePacing "restitutionTimePacing"
#define XmCRestitutionTimePacing "RestitutionTimePacing"
#define XmNsamplingFrequencyHz "samplingFrequencyHz"
#define XmCSamplingFrequencyHz "SamplingFrequencyHz"
#define XmNsynchronizationCard "synchronizationCard"
#define XmCSynchronizationCard "SynchronizationCard"

	static XtResource resources[]=
	{
		{
			XmNacquisitionIntervalColour,
			XmCAcquisitionIntervalColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Page_window_settings,foreground_colour),
			XmRString,
			"purple"
		},
		{
			XmNcalibrationDirectory,
			XmCCalibrationDirectory,
			XmRString,
			sizeof(char *),
			XtOffsetOf(Page_window_settings,calibration_directory),
			XmRString,
			""
		},
		{
			XmNdrawingBackgroundColour,
			XmCDrawingBackgroundColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Page_window_settings,background_colour),
			XmRString,
			"lightgray"
		},
		{
			XmNinitialGain,
			XmCInitialGain,
			XmRFloat,
			sizeof(float),
			XtOffsetOf(Page_window_settings,initial_gain),
			XmRString,
			"1"
		},
		{
			XmNnumberOfSamples,
			XmCNumberOfSamples,
			XmRInt,
			sizeof(int),
			XtOffsetOf(Page_window_settings,number_of_samples),
			XmRString,
			"5000"
		},
		{
			XmNrestitutionTimePacing,
			XmCRestitutionTimePacing,
			XmRBoolean,
			sizeof(Boolean),
			XtOffsetOf(Page_window_settings,restitution_time_pacing),
			XmRString,
			"false"
		},
		{
			XmNsamplingFrequencyHz,
			XmCSamplingFrequencyHz,
			XmRFloat,
			sizeof(float),
			XtOffsetOf(Page_window_settings,sampling_frequency),
			XmRString,
			"5000"
		},
		{
			XmNsynchronizationCard,
			XmCSynchronizationCard,
			XmRInt,
			sizeof(int),
			XtOffsetOf(Page_window_settings,synchronization_card),
			XmRString,
			"1"
		},
	};
	unsigned long mask;
	XGCValues values;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	BOOL win32_return_code;
	char *settings_file_name;
	FILE *settings_file;
	static char *class_name="Page_window";
#if defined (MIRADA)
	HANDLE device_driver,
	unsigned char *bus,*device_function;
	unsigned long number_of_cards;
#endif /* defined (MIRADA) */
	WNDCLASSEX class_information;
#endif /* defined (WINDOWS) */

	ENTER(create_Page_window);
	/* check arguments */
	if (rig_address&&user_interface)
	{
#if defined (MOTIF)
		if (MrmOpenHierarchy_base64_string(page_window_uidh,&page_window_hierarchy,
			&page_window_hierarchy_open))
		{
#endif /* defined (MOTIF) */
			/* allocate memory */
			if (ALLOCATE(page_window,struct Page_window,1))
			{
				/* assign fields */
				page_window->address=address;
				page_window->mapping_window_address=mapping_window_address;
				page_window->user_interface=user_interface;
				page_window->pointer_sensitivity=pointer_sensitivity;
					/*???DB.  Should pointer_sensitivity be in user_interface? */
#if defined (OLD_CODE)
				page_window->signal_display_maximum=(float)0;
				page_window->signal_display_minimum=(float)1;
#endif /* defined (OLD_CODE) */
				page_window->display_device=(struct Device *)NULL;
				page_window->display_device_number= -1;
				page_window->number_of_samples_to_save=0;
				page_window->number_of_scrolling_devices=0;
				page_window->number_of_scrolling_channels=0;
				page_window->pacing_window=(struct Pacing_window *)NULL;
				page_window->scrolling_channels=(struct Channel **)NULL;
				page_window->scrolling_devices=(struct Device **)NULL;
				page_window->number_of_scrolling_channels_device=(int *)NULL;
				page_window->last_scrolling_value_device=(int *)NULL;
				page_window->scrolling_coefficients=(float *)NULL;
				page_window->number_of_stimulators=0;
				unemap_get_number_of_stimulators(&(page_window->number_of_stimulators));
				page_window->unemap_hardware_version=0;
				unemap_get_hardware_version(&(page_window->unemap_hardware_version));
#if defined (OLD_CODE)
				page_window->stimulator_number=0;
				page_window->stimulate_devices=(struct Device **)NULL;
				page_window->stimulate_device_numbers=(int *)NULL;
#endif /* defined (OLD_CODE) */
				page_window->stimulators=(struct Stimulator *)NULL;
				page_window->data_saved=1;
				if (0<page_window->number_of_stimulators)
				{
					if (ALLOCATE(page_window->stimulators,struct Stimulator,
						page_window->number_of_stimulators))
					{
						for (i=0;i<page_window->number_of_stimulators;i++)
						{
							((page_window->stimulators)[i]).on=0;
							((page_window->stimulators)[i]).number_of_channels=0;
							((page_window->stimulators)[i]).channel_numbers=(int *)NULL;
							((page_window->stimulators)[i]).end_values=(float *)NULL;
						}
					}
#if defined (OLD_CODE)
#if defined (OLD_CODE)
					ALLOCATE(page_window->stimulate_devices,struct Device *,
						page_window->number_of_stimulators);
					ALLOCATE(page_window->stimulate_device_numbers,int,
						page_window->number_of_stimulators);
#endif /* defined (OLD_CODE) */
					ALLOCATE(page_window->stimulator_on,int,
						page_window->number_of_stimulators);
					ALLOCATE(page_window->number_of_stimulating_channels,int,
						page_window->number_of_stimulators);
					ALLOCATE(page_window->stimulating_channel_numbers,int *,
						page_window->number_of_stimulators);
					if (
#if defined (OLD_CODE)
						(page_window->stimulate_devices)&&
						(page_window->stimulate_device_numbers)&&
#endif /* defined (OLD_CODE) */
						(page_window->stimulator_on)&&
						(page_window->number_of_stimulating_channels)&&
						(page_window->stimulating_channel_numbers))
					{
#if defined (OLD_CODE)
						page_window->stimulator_number=0;
#endif /* defined (OLD_CODE) */
						for (i=0;i<page_window->number_of_stimulators;i++)
						{
#if defined (OLD_CODE)
							(page_window->stimulate_devices)[i]=(struct Device *)NULL;
							(page_window->stimulate_device_numbers)[i]= -1;
#endif /* defined (OLD_CODE) */
							(page_window->stimulator_on)[i]=0;
							(page_window->number_of_stimulating_channels)[i]=0;
							(page_window->stimulating_channel_numbers)[i]=(int *)NULL;
						}
					}
					else
					{
						page_window->number_of_stimulators=0;
#if defined (OLD_CODE)
						DEALLOCATE(page_window->stimulate_devices);
						DEALLOCATE(page_window->stimulate_device_numbers);
#endif /* defined (OLD_CODE) */
						DEALLOCATE(page_window->stimulator_on);
						DEALLOCATE(page_window->number_of_stimulating_channels);
						DEALLOCATE(page_window->stimulating_channel_numbers);
					}
#endif /* defined (OLD_CODE) */
				}
				page_window->rig_address=rig_address;
				page_window->save_file_open_data=create_File_open_data(
					signal_file_extension_write,REGULAR,save_write_signal_file,
					page_window,0,user_interface);
				page_window->calibration_directory=(char *)NULL;
				/* use the unemap hardware directory as the calibration directory */
				if (hardware_directory=getenv("UNEMAP_HARDWARE"))
				{
					if (ALLOCATE(page_window->calibration_directory,char,
						strlen(hardware_directory)+2))
					{
						strcpy(page_window->calibration_directory,hardware_directory);
#if defined (WIN32)
						if ('\\'!=hardware_directory[strlen(hardware_directory)-1])
						{
							strcat(page_window->calibration_directory,"\\");
						}
#else /* defined (WIN32) */
						if ('/'!=hardware_directory[strlen(hardware_directory)-1])
						{
							strcat(page_window->calibration_directory,"/");
						}
#endif /* defined (WIN32) */
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_Page_window.  Could not allocate calibration_directory");
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,
						"Environment variable UNEMAP_HARDWARE is not defined.  "
						"Using current directory for calibrate.dat");
				}
				page_window->hardware_initialized=0;
#if defined (MOTIF)
				page_window->activation=activation;
				page_window->shell=parent;
				page_window->window=(Widget)NULL;
				page_window->auto_range_button=(Widget)NULL;
				page_window->calibrate_button=(Widget)NULL;
				page_window->close_button=(Widget)NULL;
				(page_window->electrode).form=(Widget)NULL;
				(page_window->electrode).value=(Widget)NULL;
				page_window->experiment_checkbox=(Widget)NULL;
				page_window->full_range_button=(Widget)NULL;
				(page_window->gain).form=(Widget)NULL;
				(page_window->gain).value=(Widget)NULL;
				page_window->isolate_checkbox=(Widget)NULL;
				(page_window->low_pass).form=(Widget)NULL;
				(page_window->low_pass).value=(Widget)NULL;
				(page_window->maximum).form=(Widget)NULL;
				(page_window->maximum).value=(Widget)NULL;
				(page_window->minimum).form=(Widget)NULL;
				(page_window->minimum).value=(Widget)NULL;
				page_window->pacing_button=(Widget)NULL;
				page_window->sample_checkbox=(Widget)NULL;
				(page_window->save).button=(Widget)NULL;
				(page_window->save).form=(Widget)NULL;
				(page_window->save).value=(Widget)NULL;
				page_window->scrolling_area=(Widget)NULL;
				page_window->scrolling_checkbox=(Widget)NULL;
				page_window->start_all_stimulators_button=(Widget)NULL;
				page_window->stimulate_checkbox=(Widget)NULL;
				page_window->stimulator_checkbox=(Widget)NULL;
				page_window->stop_all_stimulators_button=(Widget)NULL;
				page_window->test_checkbox=(Widget)NULL;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				page_window->instance=user_interface->instance;
				page_window->window=(HWND)NULL;
				page_window->auto_range_button=(HWND)NULL;
				page_window->calibrate_button=(HWND)NULL;
				page_window->close_button=(HWND)NULL;
				(page_window->electrode).edit=(HWND)NULL;
				(page_window->electrode).text=(HWND)NULL;
				(page_window->electrode).arrows=(HWND)NULL;
				page_window->experiment_checkbox=(HWND)NULL;
				page_window->full_range_button=(HWND)NULL;
				(page_window->gain).edit=(HWND)NULL;
				(page_window->gain).text=(HWND)NULL;
				page_window->isolate_checkbox=(HWND)NULL;
				(page_window->low_pass_filter).edit=(HWND)NULL;
				(page_window->low_pass_filter).text=(HWND)NULL;
				(page_window->maximum).edit=(HWND)NULL;
				(page_window->maximum).text=(HWND)NULL;
				(page_window->minimum).edit=(HWND)NULL;
				(page_window->minimum).text=(HWND)NULL;
				page_window->pacing_button=(HWND)NULL;
				page_window->sample_checkbox=(HWND)NULL;
				(page_window->save).button=(HWND)NULL;
				(page_window->save).edit=(HWND)NULL;
				page_window->scrolling_area=(HWND)NULL;
				page_window->scrolling_checkbox=(HWND)NULL;
				page_window->start_all_stimulators_button=(HWND)NULL;
				page_window->stimulate_checkbox=(HWND)NULL;
				page_window->stimulator_checkbox=(HWND)NULL;
				page_window->stop_all_stimulators_button=(HWND)NULL;
				page_window->test_checkbox=(HWND)NULL;
#endif /* defined (WINDOWS) */
				/* retrieve the settings */
#if defined (MIRADA)
				/* open the device driver */
				device_driver=CreateFile("\\\\.\\VUNEMAPD.VXD",0,0,NULL,OPEN_EXISTING,
					FILE_FLAG_DELETE_ON_CLOSE,0);
				page_window->device_driver=device_driver;
				settings.sampling_frequency=(float)1000.;
				settings.initial_gain=(float)1.;
				settings.synchronization_card=1;
				if (INVALID_HANDLE_VALUE!=device_driver)
				{
					if (PCI_SUCCESSFUL!=get_mirada_information(device_driver,
						&number_of_cards,&bus,&device_function,
						&(page_window->number_of_channels),
						&(settings.number_of_samples),&(page_window->mirada_buffer)))
					{
						page_window->mirada_buffer=(short int *)NULL;
						page_window->number_of_channels=0;
						settings.number_of_samples=0;
						display_message(ERROR_MESSAGE,
							"create_Page_window.  Could not get Mirada information");
					}
				}
				else
				{
					page_window->mirada_buffer=(short int *)NULL;
					page_window->number_of_channels=0;
					settings.number_of_samples=0;
				}
#else /* defined (MIRADA) */
#if defined (MOTIF)
				XtVaGetApplicationResources(user_interface->application_shell,
					&settings,resources,XtNumber(resources),NULL);
#if defined (OLD_CODE)
/*???DB.  Always use environment variable */
				/* environment variable has priority */
				if (!(page_window->calibration_directory))
				{
					page_window->calibration_directory=settings.calibration_directory;
				}
#endif /* defined (OLD_CODE) */
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				settings.number_of_samples=5000;
				settings.sampling_frequency=(float)5000.;
				settings.initial_gain=(float)1.;
				settings.synchronization_card=1;
				if (hardware_directory=getenv("UNEMAP_HARDWARE"))
				{
					if (ALLOCATE(settings_file_name,char,strlen(hardware_directory)+13))
					{
						strcpy(settings_file_name,hardware_directory);
#if defined (WIN32)
						if ('\\'!=settings_file_name[strlen(settings_file_name)-1])
						{
							strcat(settings_file_name,"\\");
						}
#else /* defined (WIN32) */
						if ('/'!=settings_file_name[strlen(settings_file_name)-1])
						{
							strcat(settings_file_name,"/");
						}
#endif /* defined (WIN32) */
					}
				}
				else
				{
					if (ALLOCATE(settings_file_name,char,12))
					{
						settings_file_name[0]='\0';
					}
				}
				if (settings_file_name)
				{
					strcat(settings_file_name,"samples.txt");
					if (settings_file=fopen(settings_file_name,"r"))
					{
						if (1==fscanf(settings_file," number_of_samples = %d ",
							&(settings.number_of_samples)))
						{
							if (1==fscanf(settings_file," sampling_frequency = %f ",
								&(settings.sampling_frequency)))
							{
								if (1==fscanf(settings_file," initial_gain = %f ",
									&(settings.initial_gain)))
								{
								}
							}
						}
						fclose(settings_file);
					}
					DEALLOCATE(settings_file_name);
				}
#endif /* defined (WINDOWS) */
				if (!unemap_get_number_of_channels(&(page_window->number_of_channels)))
				{
					page_window->number_of_channels=0;
				}
				unemap_get_sample_range(&minimum_signal_value,&maximum_signal_value);
#endif /* defined (MIRADA) */
				page_window->number_of_samples=settings.number_of_samples;
				page_window->sampling_frequency=settings.sampling_frequency;
				page_window->synchronization_card=settings.synchronization_card;
				page_window->initial_gain=settings.initial_gain;
#if defined (OLD_CODE)
				page_window->display_maximum=(float)1;
				page_window->display_minimum=(float)-1;
#endif /* defined (OLD_CODE) */
				if (*rig_address)
				{
					/* set the display device (deliberately excluding auxiliary devices
						that are linear combinations */
					display_device=(struct Device *)NULL;
					if ((0<(number_of_devices=(*rig_address)->number_of_devices))&&
						(device_address=(*rig_address)->devices))
					{
						channel_number=(int)((page_window->number_of_channels)+1);
						for (i=0;i<number_of_devices;i++)
						{
							if ((*device_address)&&(channel=(*device_address)->channel))
							{
								if (unemap_get_gain(channel->number,&pre_filter_gain,
									&post_filter_gain))
								{
									channel_offset=channel->offset;
									channel_gain=(channel->gain_correction)/
										(pre_filter_gain*post_filter_gain);
									(*device_address)->signal_display_minimum=
										channel_gain*((float)minimum_signal_value-channel_offset);
									(*device_address)->signal_display_maximum=
										channel_gain*((float)maximum_signal_value-channel_offset);
								}
								else
								{
									(*device_address)->signal_display_minimum=(float)-1;
									(*device_address)->signal_display_maximum=(float)1;
								}
								if ((0<channel->number)&&(channel->number<channel_number)&&
									((*device_address)->description)&&
									((*device_address)->description->name))
								{
									display_device= *device_address;
									device_number=i;
									channel_number=channel->number;
								}
							}
							device_address++;
						}
						device_address=(*(page_window->rig_address))->devices;
						for (i=0;i<number_of_devices;i++)
						{
							if ((*device_address)&&!((*device_address)->channel))
							{
								(*device_address)->signal_display_minimum=(float)0;
								(*device_address)->signal_display_maximum=(float)0;
								auxiliary_properties= &(((*device_address)->description->
									properties).auxiliary);
								for (j=0;j<auxiliary_properties->number_of_electrodes;j++)
								{
									if (0<(auxiliary_properties->electrode_coefficients)[j])
									{
										(*device_address)->signal_display_minimum +=
											(auxiliary_properties->electrode_coefficients)[j]*
											((auxiliary_properties->electrodes)[j])->
											signal_display_minimum;
										(*device_address)->signal_display_maximum +=
											(auxiliary_properties->electrode_coefficients)[j]*
											((auxiliary_properties->electrodes)[j])->
											signal_display_maximum;
									}
									else
									{
										(*device_address)->signal_display_minimum +=
											(auxiliary_properties->electrode_coefficients)[j]*
											((auxiliary_properties->electrodes)[j])->
											signal_display_maximum;
										(*device_address)->signal_display_maximum +=
											(auxiliary_properties->electrode_coefficients)[j]*
											((auxiliary_properties->electrodes)[j])->
											signal_display_minimum;
									}
								}
							}
							device_address++;
						}
					}
					if (page_window->display_device=display_device)
					{
						page_window->display_device_number=device_number;
#if defined (OLD_CODE)
						channel_gain=display_device->channel->gain_correction;
#if defined (OLD_CODE)
						channel_gain=display_device->channel->gain;
#endif /* defined (OLD_CODE) */
						channel_offset=display_device->channel->offset;
						unemap_get_gain(display_device->channel->number,
							&pre_filter_gain,&post_filter_gain);
						channel_gain /= pre_filter_gain*post_filter_gain;
						page_window->display_maximum=
							channel_gain*((float)maximum_signal_value-channel_offset);
						page_window->display_minimum=
							channel_gain*((float)minimum_signal_value-channel_offset);
#endif /* defined (OLD_CODE) */
						/* set the stimulate devices */
						device_address=(*rig_address)->devices;
						for (i=0;i<number_of_devices;i++)
						{
							if ((*device_address)&&((*device_address)->channel)&&
								(0<(channel_number=(*device_address)->channel->number))&&
								(channel_number<=(int)(page_window->number_of_channels))&&
								((*device_address)->description)&&
								((*device_address)->description->name))
							{
								for (j=0;j<page_window->number_of_stimulators;j++)
								{
									if (unemap_channel_valid_for_stimulator(j+1,channel_number))
									{
#if defined (OLD_CODE)
										if ((-1==(page_window->stimulate_device_numbers)[j])||
											(channel_number<
											(page_window->stimulate_device_numbers)[j]))
										{
											(page_window->stimulate_device_numbers)[j]=i;
											(page_window->stimulate_devices)[j]= *device_address;
											(page_window->stimulator_on)[j]=0;
										}
#endif /* defined (OLD_CODE) */
									}
								}
							}
							device_address++;
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,
							"No device with a valid channel number in the rig");
					}
				}
#if defined (MOTIF)
				/* register the callbacks */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(page_window_hierarchy,
					callback_list,XtNumber(callback_list)))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)page_window;
					identifier_list[1].value=(XtPointer)identifying_colour;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(page_window_hierarchy,
						identifier_list,XtNumber(identifier_list)))
					{
						/* fetch the page window widget */
						if (MrmSUCCESS==MrmFetchWidget(page_window_hierarchy,"page_window",
							parent,&(page_window->window),&page_window_class))
						{
							/* set the background colour for the page drawing area */
							XtVaSetValues(page_window->scrolling_area,XmNbackground,
								settings.background_colour,NULL);
							/* set ghosting for the buttons */
							XtSetSensitive(page_window->experiment_checkbox,False);
							XtSetSensitive(page_window->sample_checkbox,False);
							XtSetSensitive((page_window->save).button,False);
							XtSetSensitive((page_window->low_pass).form,False);
							XtSetSensitive(page_window->close_button,True);
							switch (page_window->unemap_hardware_version)
							{
								case UnEmap_1V2:
								default:
								{
									XtVaSetValues(page_window->sample_checkbox,
										XmNleftWidget,page_window->experiment_checkbox,NULL);
									XtDestroyWidget(page_window->isolate_checkbox);
									page_window->isolate_checkbox=(Widget)NULL;
									unemap_set_isolate_record_mode(0,0);
									XtVaSetValues((page_window->low_pass).form,
										XmNleftWidget,(page_window->save).form,NULL);
									XtDestroyWidget(page_window->test_checkbox);
									page_window->test_checkbox=(Widget)NULL;
									XtDestroyWidget(page_window->calibrate_button);
									page_window->calibrate_button=(Widget)NULL;
								} break;
								case UnEmap_2V1:
								case UnEmap_2V2:
								case UnEmap_2V1|UnEmap_2V2:
								{
									XtSetSensitive(page_window->isolate_checkbox,False);
									XtSetSensitive(page_window->calibrate_button,False);
									XtSetSensitive(page_window->test_checkbox,False);
								} break;
							}
							XtSetSensitive(page_window->auto_range_button,False);
							XtSetSensitive(page_window->full_range_button,False);
							switch (page_window->unemap_hardware_version)
							{
								case UnEmap_1V2:
								default:
								{
									XtDestroyWidget(page_window->pacing_button);
									page_window->pacing_button=(Widget)NULL;
									XtDestroyWidget(page_window->stop_all_stimulators_button);
									page_window->stop_all_stimulators_button=(Widget)NULL;
									XtDestroyWidget(page_window->start_all_stimulators_button);
									page_window->start_all_stimulators_button=(Widget)NULL;
								} break;
								case UnEmap_2V1:
								case UnEmap_2V2:
								case UnEmap_2V1|UnEmap_2V2:
								{
									if (0<page_window->number_of_stimulators)
									{
										if (True==settings.restitution_time_pacing)
										{
											XtDestroyWidget(page_window->stop_all_stimulators_button);
											page_window->stop_all_stimulators_button=(Widget)NULL;
											XtDestroyWidget(page_window->
												start_all_stimulators_button);
											page_window->start_all_stimulators_button=(Widget)NULL;
											XtSetSensitive(page_window->pacing_button,False);
										}
										else
										{
											XtDestroyWidget(page_window->pacing_button);
											page_window->pacing_button=(Widget)NULL;
											XtSetSensitive(page_window->start_all_stimulators_button,
												False);
											XtSetSensitive(page_window->stop_all_stimulators_button,
												False);
										}
									}
									else
									{
										XtDestroyWidget(page_window->pacing_button);
										page_window->pacing_button=(Widget)NULL;
										XtDestroyWidget(page_window->stop_all_stimulators_button);
										page_window->stop_all_stimulators_button=(Widget)NULL;
										XtDestroyWidget(page_window->start_all_stimulators_button);
										page_window->start_all_stimulators_button=(Widget)NULL;
									}
								} break;
							}
							XtSetSensitive((page_window->electrode).form,False);
							XtSetSensitive((page_window->gain).form,False);
							XtSetSensitive((page_window->maximum).form,False);
							XtSetSensitive((page_window->minimum).form,False);
							XtSetSensitive(page_window->scrolling_checkbox,False);
							switch (page_window->unemap_hardware_version)
							{
								case UnEmap_1V2:
								default:
								{
									XtDestroyWidget(page_window->stimulator_checkbox);
									page_window->stimulator_checkbox=(Widget)NULL;
									XtDestroyWidget(page_window->stimulate_checkbox);
									page_window->stimulate_checkbox=(Widget)NULL;
								} break;
								case UnEmap_2V1:
								case UnEmap_2V2:
								case UnEmap_2V1|UnEmap_2V2:
								{
									if ((0<page_window->number_of_stimulators)&&
										(True!=settings.restitution_time_pacing))
									{
										XtSetSensitive(page_window->stimulate_checkbox,False);
										XtSetSensitive(page_window->stimulator_checkbox,False);
									}
									else
									{
										XtDestroyWidget(page_window->stimulator_checkbox);
										page_window->stimulator_checkbox=(Widget)NULL;
										XtDestroyWidget(page_window->stimulate_checkbox);
										page_window->stimulate_checkbox=(Widget)NULL;
									}
								} break;
							}
							/* create the graphics contexts */
							display=user_interface->display;
							/* the drawable has to have the correct depth and screen */
							XtVaGetValues(user_interface->application_shell,XmNdepth,&depth,
								NULL);
							depth_screen_drawable=XCreatePixmap(display,XRootWindow(display,
								XDefaultScreen(display)),1,1,depth);
							mask=GCLineStyle|GCBackground|GCFont|GCForeground|GCFunction;
							values.font=user_interface->normal_font->fid;
							values.line_style=LineSolid;
							values.background=settings.background_colour;
							values.function=GXcopy;
							values.foreground=settings.foreground_colour;
							(page_window->graphics_context).foreground_drawing_colour=
								XCreateGC(display,depth_screen_drawable,mask,&values);
							values.foreground=settings.background_colour;
							(page_window->graphics_context).background_drawing_colour=
								XCreateGC(display,depth_screen_drawable,mask,&values);
							XFreePixmap(display,depth_screen_drawable);
							/* set the display minimum and maximum (gains may be changed
								while creating scrolling_area) */
							if (page_window->display_device)
							{
								show_display_minimum(page_window);
								show_display_maximum(page_window);
								XtVaSetValues((page_window->electrode).value,XmNvalue,
									page_window->display_device->description->name,NULL);
							}
#if defined (OLD_CODE)
							update_stimulator(page_window,1);
#endif /* defined (OLD_CODE) */
#if defined (OLD_CODE)
							XtSetSensitive((page_window->stimulator).form,False);
#endif /* defined (OLD_CODE) */
							/* set the experiment checkbox */
							XmToggleButtonSetState(page_window->experiment_checkbox,False,
								False);
							XtSetSensitive(page_window->experiment_checkbox,True);
							if (address)
							{
								*address=page_window;
							}
							/* manage the page window */
							XtManageChild(page_window->window);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_Page_window.  Could not fetch page window widget");
							DEALLOCATE(page_window);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_Page_window.  Could not register the identifiers");
						DEALLOCATE(page_window);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Page_window.  Could not register the callbacks");
					DEALLOCATE(page_window);
				}
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				/* check if the class is registered */
				if (TRUE!=(win32_return_code=GetClassInfoEx(user_interface->instance,
					class_name,&class_information)))
				{
					class_information.cbClsExtra=0;
					class_information.cbWndExtra=
						DLGWINDOWEXTRA+sizeof(struct Page_window *);
					class_information.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
					class_information.hCursor=LoadCursor(NULL,IDC_ARROW);
					class_information.hIcon=(HICON)NULL;
/*					class_information.hIcon=LoadIcon(user_interface->instance,class_name);*/
						/*???DB.  Do I need an icon ? */
					class_information.hInstance=user_interface->instance;
					class_information.lpfnWndProc=Page_window_class_proc;
					class_information.lpszClassName=class_name;
					class_information.style=CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
					/* allow resource to specify the menu */
					class_information.lpszMenuName=NULL;
					/*???DB.  Extra in WNDCLASSEX over WNDCLASS */
					class_information.cbSize=sizeof(WNDCLASSEX);
					class_information.hIconSm=(HICON)NULL;
/*					class_information.hIconSm=LoadIcon(user_interface->instance,
						"Page_window" "_small");*/
						/*???DB.  Do I need an icon ? */
					if (RegisterClassEx(&class_information))
					{
						win32_return_code=TRUE;
					}
				}
				/* create the window */
				if (TRUE==win32_return_code)
				{
					if (page_window->window=CreateDialogParam(
						user_interface->instance,"Page_window",parent,
						Page_window_dialog_proc,(LPARAM)page_window))
					{
						if (address)
						{
							*address=page_window;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_Page_window.  Could not create dialog");
						DEALLOCATE(page_window);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Page_window.  Unable to register class information");
					DEALLOCATE(page_window);
				}
#endif /* defined (WINDOWS) */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_Page_window.  Could not allocate page window structure");
			}
#if defined (MOTIF)
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Page_window.  Could not open hierarchy");
			page_window=(struct Page_window *)NULL;
		}
#endif /* defined (MOTIF) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Page_window.  Invalid argument(s).  %p %p",rig_address,
			user_interface);
		page_window=(struct Page_window *)NULL;
	}
	LEAVE;

	return (page_window);
} /* create_Page_window */

int open_Page_window(struct Page_window **address,
	struct Mapping_window **mapping_window_address,struct Rig **rig_address,
#if defined (MOTIF)
	Pixel identifying_colour,
	int screen_width,int screen_height,
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
#if defined (MIRADA)
	HANDLE device_driver,
#endif /* defined (MIRADA) */
#endif /* defined (WINDOWS) */
	int pointer_sensitivity,char *signal_file_extension_write,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
If <*address> is NULL, a page window with the specified <parent> and 
<foreground_colour> is created.  The page window is opened.
???DB.  Change rig_address on the fly ?
==============================================================================*/
{
	int return_code;
	struct Page_window *page_window;
#if defined (MOTIF)
	Widget page_window_shell;
#endif /* defined (MOTIF) */

	ENTER(open_Page_window);
	if (address&&user_interface)
	{
		if (!(page_window= *address))
		{
#if defined (MOTIF)
			if (page_window_shell=create_page_window_shell((Widget *)NULL,
				user_interface->application_shell,screen_width,screen_height,
				user_interface))
			{
#endif /* defined (MOTIF) */
				if (page_window=create_Page_window(address,
#if defined (WINDOWS)
#if defined (MIRADA)
					device_driver,
#endif /* defined (MIRADA) */
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
					(Widget)NULL,page_window_shell,
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
					(HWND)NULL,
#endif /* defined (WINDOWS) */
					rig_address,
#if defined (MOTIF)
					identifying_colour,
#endif /* defined (MOTIF) */
					mapping_window_address,pointer_sensitivity,
					signal_file_extension_write,user_interface))
				{
#if defined (MOTIF)
#if defined (OLD_CODE)
/*???DB.  Moved to create_Page_window */
					/* manage the page window */
					XtManageChild(page_window->window);
#endif /* defined (OLD_CODE) */
					/* pop up the page shell */
/*???debug */
printf("pop up page_window->shell\n");
					XtPopup(page_window->shell,XtGrabNone);
#endif /* defined (MOTIF) */
					*address=page_window;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"open_Page_window.  Could not create window");
#if defined (MOTIF)
					XtDestroyWidget(page_window_shell);
#endif /* defined (MOTIF) */
					return_code=0;
				}
#if defined (MOTIF)
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"open_Page_window.  Could not create shell");
				return_code=0;
			}
#endif /* defined (MOTIF) */
		}
		if (page_window
#if defined (MOTIF)
			&&(page_window->shell)
#endif /* defined (MOTIF) */
			)
		{
#if defined (MOTIF)
			/* pop up the page window shell */
			XtPopup(page_window->shell,XtGrabNone);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			ShowWindow(page_window->window,SW_SHOWDEFAULT);
				/*???DB.  SW_SHOWDEFAULT needs thinking about */
#endif /* defined (WINDOWS) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"open_Page_window.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* open_Page_window */
