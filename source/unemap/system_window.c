/*******************************************************************************
FILE : system_window.c

LAST MODIFIED : 16 September 2002

DESCRIPTION :
???DB.  Have to have a proper destroy callback for the system window
UNIMA_ACQUISITION refers to the acquisition window used with the UNIMA/EMAP
	hardware
==============================================================================*/
#include <stddef.h>
#include <string.h>
#if defined (MOTIF)
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xlib.h>
#include <X11/Composite.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/ToggleB.h>
#include <Mrm/MrmPublic.h>
#include <Mrm/MrmDecls.h>
#endif /* defined (MOTIF) */
#if defined (UNIMA)
#include "UNIMA/uim.h"
#endif
#include "general/debug.h"
#include "general/object.h"
#include "time/time_keeper.h"
#include "unemap/acquisition_work_area.h"
#include "unemap/analysis_work_area.h"
#include "unemap/mapping_window.h"
#include "unemap/system_window.h"
#if defined (MOTIF)
#include "unemap/system_window.uidh"
#endif /* defined (MOTIF) */
#include "unemap/unemap_package.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module types
------------
*/
struct System_window
/*******************************************************************************
LAST MODIFIED : 17 July 2002

DESCRIPTION :
Unemap system window object.
==============================================================================*/
{
	Widget window,window_shell;
	Widget acquisition_button;
	struct Acquisition_work_area acquisition;
	Widget analysis_button;
	struct Analysis_work_area analysis;
	Widget mapping_button;
	struct Mapping_work_area mapping;
	Widget close_button;
	struct User_interface *user_interface;
	struct Map_drawing_information *map_drawing_information;
	/* user settings */
	char *configuration_directory,*configuration_file_extension,
		*postscript_file_extension,*signal_file_extension_read,
		*signal_file_extension_write;
	int pointer_sensitivity;
	Pixel acquisition_colour,analysis_colour;
	struct Time_keeper *time_keeper;
	struct Unemap_package *unemap_package;	
	Unemap_system_window_close_callback_procedure *close_callback;
	void *close_callback_data;
}; /* struct System_window */

typedef struct System_window System_window_settings;

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int system_window_hierarchy_open=0;
static MrmHierarchy system_window_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
static void close_mapping_work_area(Widget widget,XtPointer system_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Closes the windows associated with the mapping work area.
==============================================================================*/
{
	struct System_window *system;

	ENTER(close_mapping_work_area);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (system=(struct System_window *)system_window)
	{
		if (system->mapping.window_shell)
		{
			system->mapping.open=0;
			/* close the mapping shell */
			XtPopdown(system->mapping.window_shell);
			/* unghost the system mapping button */
			XtSetSensitive(system->mapping_button,True);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"close_mapping_work_area.  mapping window shell is missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"close_mapping_work_area.  Missing system_window");
	}
	LEAVE;
} /* close_mapping_work_area */

static void identify_system_acquisition_but(Widget *widget_id,
	XtPointer system_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the system acquisition button.
==============================================================================*/
{
	struct System_window *system;

	ENTER(identify_system_acquisition_but);
	USE_PARAMETER(call_data);
	if (system=(struct System_window *)system_window)
	{
		system->acquisition_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_system_acquisition_but.  Missing system_window");
	}
	LEAVE;
} /* identify_system_acquisition_but */

static void associate_mapping_acquisition(Widget widget,XtPointer system_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Associate the mapping window with the acquisition work area
==============================================================================*/
{
	int maintain_aspect_ratio;
	struct Map_drawing_information *drawing_information=
		(struct Map_drawing_information *)NULL;
	struct Spectrum *spectrum=(struct Spectrum *)NULL;
	struct Spectrum *spectrum_to_be_modified_copy=(struct Spectrum *)NULL;
	struct MANAGER(Spectrum) *spectrum_manager=(struct MANAGER(Spectrum) *)NULL;
	struct System_window *system;
	struct User_interface *user_interface;

	ENTER(associate_mapping_acquisition);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
#if !defined (UNEMAP_USE_NODES)
	USE_PARAMETER(spectrum);
	USE_PARAMETER(spectrum_to_be_modified_copy);
	USE_PARAMETER(spectrum_manager);
#endif /* defined (UNEMAP_USE_NODES) */
	if ((system=(struct System_window *)system_window)&&
		(user_interface=system->user_interface)&&(drawing_information=
			system->map_drawing_information))
	{
		/* if the mapping window is open */
		if (system->mapping.open)
		{
			if (drawing_information->maintain_aspect_ratio)
			{
				maintain_aspect_ratio=1;
			}
			else
			{
				maintain_aspect_ratio=0;
			}
#if defined (UNEMAP_USE_NODES)
			spectrum=system->map_drawing_information->spectrum;
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
						Spectrum_set_simple_type(spectrum_to_be_modified_copy,
							BLUE_TO_RED_SPECTRUM);
						MANAGER_MODIFY_NOT_IDENTIFIER(Spectrum,name)(spectrum,
							spectrum_to_be_modified_copy,spectrum_manager);
						DESTROY(Spectrum)(&spectrum_to_be_modified_copy);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"associate_mapping_acquisition. Could not create spectrum copy.");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"associate_mapping_acquisition. Spectrum is not in manager!");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"associate_mapping_acquisition. Spectrum_manager not present");
			}
#else
			Spectrum_set_simple_type(system->map_drawing_information->spectrum,
				BLUE_TO_RED_SPECTRUM);
#endif /* defined (UNEMAP_USE_NODES) */
			open_mapping_window(&(system->acquisition.mapping_window),
				system->mapping_button,system->window_shell,
				&(system->mapping.window_shell),&(system->mapping.outer_form),
				&(system->mapping.current_mapping_window),
				&(system->mapping.open),&(system->mapping.associate),
				&(system->analysis.map_type),/* (enum Map_type *)NULL,*/
				HIDE_COLOUR,HIDE_CONTOURS,SHOW_ELECTRODE_NAMES,
				HIDE_FIBRES,HIDE_LANDMARKS,HIDE_EXTREMA,maintain_aspect_ratio,1,
				HAMMER_PROJECTION,VARIABLE_THICKNESS,&(system->acquisition.rig),
				(int *)NULL,(int *)NULL,(int *)NULL,(int *)NULL,(int *)NULL,
				system->acquisition_colour,ACQUISITION_ASSOCIATE,
				(XtPointer)set_mapping_acquisition_region,(XtPointer)NULL,
				(XtPointer)NULL,(XtPointer)&(system->acquisition),
				User_interface_get_screen_width(user_interface),User_interface_get_screen_height(user_interface),
				system->configuration_file_extension,system->postscript_file_extension,
				system->map_drawing_information,user_interface,
				system->unemap_package,&((system->analysis).first_eimaging_event),
				&((system->analysis).analysis_mode));
		}
		else
		{
			system->mapping.associate=ACQUISITION_ASSOCIATE;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"associate_mapping_acquisition.  Missing system_window");
	}
	LEAVE;
} /* associate_mapping_acquisition */

static void open_acquisition_work_area(Widget widget,XtPointer system_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Opens the windows associated with the acquisition work area.
==============================================================================*/
{
	char *default_configuration_file_name;
	int read_default_configuration;
	static MrmRegisterArg callback_list[]={
		{"associate_mapping_acquisition",(XtPointer)associate_mapping_acquisition}};
	struct Mapping_window *mapping;
	struct System_window *system;
#if defined (UNIMA_ACQUISITION)
	struct Rig *rig;
	struct Acquisition_window *acquisition_window;
#endif /* defined (UNIMA_ACQUISITION) */

	ENTER(open_acquisition_work_area);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (system=(struct System_window *)system_window)
	{
		read_default_configuration=0;
		if (!(system->acquisition.rig))
		{
			/* allocate memory for the default configuration file name */
			default_configuration_file_name=(char *)NULL;
			if ((system->configuration_directory)&&
				(0<strlen(system->configuration_directory)))
			{
				if ((system->configuration_file_extension)&&
					(0<strlen(system->configuration_file_extension)))
				{
					if (ALLOCATE(default_configuration_file_name,char,
						strlen(system->configuration_directory)+
						strlen(system->configuration_file_extension)+10))
					{
						strcpy(default_configuration_file_name,
							system->configuration_directory);
						strcat(default_configuration_file_name,"/default");
						strcat(default_configuration_file_name,
							system->configuration_file_extension);
					}
				}
				else
				{
					if (ALLOCATE(default_configuration_file_name,char,
						strlen(system->configuration_directory)+14))
					{
						strcpy(default_configuration_file_name,
							system->configuration_directory);
						strcat(default_configuration_file_name,"/default.cnfg");
					}
				}
			}
			else
			{
				if ((system->configuration_file_extension)&&
					(0<strlen(system->configuration_file_extension)))
				{
					if (ALLOCATE(default_configuration_file_name,char,
						strlen(system->configuration_file_extension)+9))
					{
						strcpy(default_configuration_file_name,"default");
						strcat(default_configuration_file_name,
							system->configuration_file_extension);
					}
				}
				else
				{
					if (ALLOCATE(default_configuration_file_name,char,13))
					{
						strcpy(default_configuration_file_name,"default.cnfg");
					}
				}
			}
			if (default_configuration_file_name)
			{
				/* read in the default rig configuration */
				if (read_configuration_file(default_configuration_file_name,
					&(system->acquisition.rig)
#if defined (UNEMAP_USE_3D)
					,system->unemap_package
#endif /* defined (UNEMAP_USE_3D) */
					))
				{
					read_default_configuration=1;
				}
#if defined (OLD_CODE)
				else
				{
					display_message(ERROR_MESSAGE,
	"open_acquisition_work_area.  Could not open default configuration file %s",
						default_configuration_file_name);
				}
#endif /* defined (OLD_CODE) */
				DEALLOCATE(default_configuration_file_name);
			}
			else
			{
				display_message(ERROR_MESSAGE,
"open_acquisition_work_area.  Could not create default configuration file name"
					);
			}
		}
		if (!(system->acquisition.window_shell))
		{
			/* register the callbacks */
				/*???DB.  Used by more than set of uid files so has to be in global
					name list */
			if (MrmSUCCESS==MrmRegisterNames(callback_list,XtNumber(callback_list)))
			{
				if (!create_acquisition_work_area(&(system->acquisition),
					system->acquisition_button,system->window_shell,
					system->pointer_sensitivity,system->acquisition_colour,
					system->signal_file_extension_write,system->user_interface))
				{
					display_message(ERROR_MESSAGE,
		"open_acquisition_work_area.  Could not create the acquisition work area");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"open_acquisition_work_area.  Could not register callbacks");
			}
		}
		if (system->acquisition.window_shell)
		{
			/* pop up the acquisition window shell */
			XtPopup(system->acquisition.window_shell,XtGrabNone);
			/* ghost the acquisition activation button */
			XtSetSensitive(system->acquisition.activation,False);
			if (read_default_configuration&&
				(mapping=system->acquisition.mapping_window))
			{
				/*??? same as mapping_read_configuration_file */
				XtSetSensitive(mapping->file_menu.save_configuration_button,True);
				XtSetSensitive(
					mapping->file_menu.set_default_configuration_button,True);
				update_mapping_window_menu(mapping);
				update_mapping_drawing_area(mapping,2);
				update_mapping_colour_or_auxili(mapping);
			}
#if defined (OLD_CODE)
			if (!(system->acquisition.rig))
			{
				/* allocate memory for the default configuration file name */
				default_configuration_file_name=(char *)NULL;
				if ((system->configuration_directory)&&
					(0<strlen(system->configuration_directory)))
				{
					if ((system->configuration_file_extension)&&
						(0<strlen(system->configuration_file_extension)))
					{
						if (ALLOCATE(default_configuration_file_name,char,
							strlen(system->configuration_directory)+
							strlen(system->configuration_file_extension)+10))
						{
							strcpy(default_configuration_file_name,
								system->configuration_directory);
							strcat(default_configuration_file_name,"/default");
							strcat(default_configuration_file_name,
								system->configuration_file_extension);
						}
					}
					else
					{
						if (ALLOCATE(default_configuration_file_name,char,
							strlen(system->configuration_directory)+14))
						{
							strcpy(default_configuration_file_name,
								system->configuration_directory);
							strcat(default_configuration_file_name,"/default.cnfg");
						}
					}
				}
				else
				{
					if ((system->configuration_file_extension)&&
						(0<strlen(system->configuration_file_extension)))
					{
						if (ALLOCATE(default_configuration_file_name,char,
							strlen(system->configuration_file_extension)+9))
						{
							strcpy(default_configuration_file_name,"default");
							strcat(default_configuration_file_name,
								system->configuration_file_extension);
						}
					}
					else
					{
						if (ALLOCATE(default_configuration_file_name,char,13))
						{
							strcpy(default_configuration_file_name,"default.cnfg");
						}
					}
				}
				if (default_configuration_file_name)
				{
					/* read in the default rig configuration */
					if (read_configuration_file(default_configuration_file_name,
						&(system->acquisition.rig)))
					{
						if (mapping=system->acquisition.mapping_window)
						{
							/*??? same as mapping_read_configuration_file */
							XtSetSensitive(mapping->file_menu.save_configuration_button,True);
							XtSetSensitive(
								mapping->file_menu.set_default_configuration_button,True);
							update_mapping_window_menu(mapping);
							update_mapping_drawing_area(mapping,2);
							update_mapping_colour_or_auxili(mapping);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
		"open_acquisition_work_area.  Could not open default configuration file %s",
							default_configuration_file_name);
					}
					DEALLOCATE(default_configuration_file_name);
				}
				else
				{
					display_message(ERROR_MESSAGE,
"open_acquisition_work_area.  Could not create default configuration file name"
						);
				}
			}
#endif /* defined (OLD_CODE) */
#if defined (UNIMA_ACQUISITION)
			acquisition_window=system->acquisition.window;
			if (rig=system->acquisition.rig)
			{
				XtSetSensitive(acquisition_window->experiment_toggle,True);
				if (EXPERIMENT_ON==rig->experiment)
				{
					XtSetSensitive(acquisition_window->monitoring_toggle,True);
					if (rig->page_list)
					{
						XtSetSensitive(acquisition_window->page_button,True);
					}
					else
					{
						XtSetSensitive(acquisition_window->page_button,False);
					}
					if (MONITORING_ON==rig->monitoring)
					{
						XtSetSensitive(acquisition_window->acquire_button,True);
					}
					else
					{
						XtSetSensitive(acquisition_window->acquire_button,False);
					}
				}
				else
				{
					XtSetSensitive(acquisition_window->acquire_button,False);
					XtSetSensitive(acquisition_window->page_button,False);
					XtSetSensitive(acquisition_window->monitoring_toggle,False);
				}
			}
			else
			{
				XtSetSensitive(acquisition_window->acquire_button,False);
				XtSetSensitive(acquisition_window->page_button,False);
				XtSetSensitive(acquisition_window->monitoring_toggle,False);
				XtSetSensitive(acquisition_window->experiment_toggle,False);
			}
#endif /* defined (UNIMA_ACQUISITION) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"open_acquisition_work_area.  Missing system_window");
	}
	LEAVE;
} /* open_acquisition_work_area */

static void identify_system_analysis_button(Widget *widget_id,
	XtPointer system_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the system analysis button.
==============================================================================*/
{
	struct System_window *system;

	ENTER(identify_system_analysis_button);
	USE_PARAMETER(call_data);
	if (system=(struct System_window *)system_window)
	{
		system->analysis_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_system_analysis_button.  Missing system_window");
	}
	LEAVE;
} /* identify_system_analysis_button */

static void associate_mapping_analysis(Widget widget,XtPointer system_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Associate the mapping window with the analysis work area
==============================================================================*/
{
	int maintain_aspect_ratio;
	struct Map_drawing_information *drawing_information
		=(struct Map_drawing_information *)NULL;
	struct System_window *system;
	struct User_interface *user_interface;
	struct Spectrum *spectrum=(struct Spectrum *)NULL;
	struct Spectrum *spectrum_to_be_modified_copy=(struct Spectrum *)NULL;
	struct MANAGER(Spectrum) *spectrum_manager=(struct MANAGER(Spectrum) *)NULL;

	ENTER(associate_mapping_analysis);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
#if ! defined (UNEMAP_USE_NODES)
	USE_PARAMETER(spectrum_to_be_modified_copy);
	USE_PARAMETER(spectrum_manager);
#endif /* !defined (UNEMAP_USE_NODES) */
	if ((system=(struct System_window *)system_window)&&
		(user_interface=system->user_interface)&&
		(drawing_information=system->map_drawing_information))
	{
		/* if the mapping window is open */
		if (system->mapping.open)
		{
			if (drawing_information->maintain_aspect_ratio)
			{
				maintain_aspect_ratio=1;
			}
			else
			{
				maintain_aspect_ratio=0;
			}
			/* Don't want to reset the spectrum when using the analysis window */
			if (system->mapping.associate!=ANALYSIS_ASSOCIATE)
			{

				spectrum=system->map_drawing_information->spectrum;
#if defined (UNEMAP_USE_NODES)
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
#endif /* defined (UNEMAP_USE_NODES) */

							switch (system->analysis.map_type)
							{
								case SINGLE_ACTIVATION:
								case MULTIPLE_ACTIVATION:
								{
									Spectrum_set_simple_type(spectrum,
										RED_TO_BLUE_SPECTRUM);
								} break;
								default:
								{
									Spectrum_set_simple_type(spectrum,
										BLUE_TO_RED_SPECTRUM);
								} break;
							}
#if defined (UNEMAP_USE_NODES)
							MANAGER_MODIFY_NOT_IDENTIFIER(Spectrum,name)(spectrum,
								spectrum_to_be_modified_copy,spectrum_manager);
							DESTROY(Spectrum)(&spectrum_to_be_modified_copy);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"associate_mapping_analysis. Could not create spectrum copy.");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"associate_mapping_analysis. Spectrum is not in manager!");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"associate_mapping_analysis. Spectrum_manager not present");
				}
#endif /* defined (UNEMAP_USE_NODES) */
			}
			open_mapping_window(&(system->analysis.mapping_window),
				system->mapping_button,system->window_shell,
				&(system->mapping.window_shell),&(system->mapping.outer_form),
				&(system->mapping.current_mapping_window),
				&(system->mapping.open),&(system->mapping.associate),
				&(system->analysis.map_type),HIDE_COLOUR,HIDE_CONTOURS,
				SHOW_ELECTRODE_NAMES,HIDE_FIBRES,HIDE_LANDMARKS,HIDE_EXTREMA,
				maintain_aspect_ratio,1,HAMMER_PROJECTION,VARIABLE_THICKNESS,
				&(system->analysis.rig),&(system->analysis.event_number),
				&(system->analysis.potential_time),&(system->analysis.datum),
				&(system->analysis.start_search_interval),
				&(system->analysis.end_search_interval),system->analysis_colour,
				ANALYSIS_ASSOCIATE,(XtPointer)set_mapping_analysis_region,
				(XtPointer)analysis_select_map_drawing_are,
				(XtPointer)analysis_select_auxiliary_drawi,
				(XtPointer)&(system->analysis),User_interface_get_screen_width(user_interface),
				User_interface_get_screen_height(user_interface),system->configuration_file_extension,
				system->postscript_file_extension,system->map_drawing_information,
				user_interface,system->unemap_package,
				&((system->analysis).first_eimaging_event),
				&((system->analysis).analysis_mode));
		}
		else
		{
			system->mapping.associate=ANALYSIS_ASSOCIATE;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"associate_mapping_analysis.  Missing system_window");
	}
	LEAVE;
} /* associate_mapping_analysis */

static void open_analysis_work_area(Widget widget,XtPointer system_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Opens the windows associated with the analysis work area.
==============================================================================*/
{
	static MrmRegisterArg callback_list[]={
		{"associate_mapping_analysis",(XtPointer)associate_mapping_analysis}};
	struct System_window *system;
	struct User_interface *user_interface;

	ENTER(open_analysis_work_area);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((system=(struct System_window *)system_window)&&
		(user_interface=system->user_interface))
	{
		/* register the callbacks */
			/*???DB.  Used by more than set of uid files so has to be in global
				name list */
		if (MrmSUCCESS==MrmRegisterNames(callback_list,XtNumber(callback_list)))
		{
			if (!create_analysis_work_area(&(system->analysis),
				system->analysis_button,system->window_shell,
				system->pointer_sensitivity,system->signal_file_extension_read,
				system->signal_file_extension_write,system->postscript_file_extension,
				system->configuration_file_extension,system->analysis_colour,
				system->map_drawing_information,user_interface,system->time_keeper,
				system->unemap_package))
			{
				display_message(ERROR_MESSAGE,
					"open_analysis_work_area.  Could not create analysis work area");
			}
			if (system->analysis.window_shell)
			{
				/* pop up the analysis window shell */
				XtPopup(system->analysis.window_shell,XtGrabNone);
				/* ghost the analysis activation button */
				XtSetSensitive(system->analysis.activation,False);
				if (system->analysis.trace)
				{
					/* pop up the trace window shell */
					XtPopup(system->analysis.trace->shell,XtGrabNone);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"open_analysis_work_area.  Could not register callbacks");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"open_analysis_work_area.  Missing system_window");
	}
	LEAVE;
} /* open_analysis_work_area */

static void identify_system_mapping_button(Widget *widget_id,
	XtPointer system_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the system mapping button.
==============================================================================*/
{
	struct System_window *system;

	ENTER(identify_system_mapping_button);
	USE_PARAMETER(call_data);
	if (system=(struct System_window *)system_window)
	{
		system->mapping_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_system_mapping_button.  Missing system_window");
	}
	LEAVE;
} /* identify_system_mapping_button */

static void open_mapping_work_area(Widget widget,XtPointer system_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Opens the windows associated with the mapping work area.
==============================================================================*/
{
	int maintain_aspect_ratio;
	static MrmRegisterArg callback_list[]={
		{"associate_mapping_analysis",(XtPointer)associate_mapping_analysis},
		{"associate_mapping_acquisition",(XtPointer)associate_mapping_acquisition}};
	static MrmRegisterArg identifier_list[]=
	{
		{"system_window_structure",(XtPointer)NULL}
	};
	struct Map_drawing_information *drawing_information
		=(struct Map_drawing_information *)NULL;
	struct Spectrum *spectrum=(struct Spectrum *)NULL;
	struct Spectrum *spectrum_to_be_modified_copy=(struct Spectrum *)NULL;
	struct MANAGER(Spectrum) *spectrum_manager=(struct MANAGER(Spectrum) *)NULL;
	struct System_window *system;
	struct User_interface *user_interface;

	ENTER(open_mapping_work_area);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
#if !defined (UNEMAP_USE_NODES)
	USE_PARAMETER(spectrum_manager);
#endif /*defined (UNEMAP_USE_NODES) */
	if ((system=(struct System_window *)system_window)&&
		(user_interface=system->user_interface)&&(drawing_information=
			system->map_drawing_information))
	{
		/* register the callbacks */
			/*???DB.  Used by more than set of uid files so has to be in global
				name list */
		if (MrmSUCCESS==MrmRegisterNames(callback_list,XtNumber(callback_list)))
		{
			/* assign and register the identifiers */
			identifier_list[0].value=(XtPointer)system;
				/*???DB.  Used by more than set of uid files so has to be in global
					name list */
			if (MrmSUCCESS==MrmRegisterNames(identifier_list,
				XtNumber(identifier_list)))
			{
				if (drawing_information->maintain_aspect_ratio)
				{
					maintain_aspect_ratio=1;
				}
				else
				{
					maintain_aspect_ratio=0;
				}
				switch (system->mapping.associate)
				{
					case ACQUISITION_ASSOCIATE:
					{
						/* if there is not an acquisition mapping window */
						if (!(system->acquisition.mapping_window))
						{
							/* if there is not an acquisition window */
							if (!(system->acquisition.window))
							{
								/* create the acquisition work area */
								if (!create_acquisition_work_area(&(system->acquisition),
									system->acquisition_button,system->window_shell,
									system->pointer_sensitivity,system->acquisition_colour,
									system->signal_file_extension_write,user_interface))
								{
									display_message(ERROR_MESSAGE,
					"open_mapping_work_area.  Could not create acquisition work area");
								}
							}
						}
#if defined (UNEMAP_USE_NODES)
						spectrum=drawing_information->spectrum;
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
									Spectrum_set_simple_type(spectrum_to_be_modified_copy,
										BLUE_TO_RED_SPECTRUM);
									MANAGER_MODIFY_NOT_IDENTIFIER(Spectrum,name)(spectrum,
										spectrum_to_be_modified_copy,spectrum_manager);
									DESTROY(Spectrum)(&spectrum_to_be_modified_copy);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"open_mapping_work_area . Could not create spectrum copy.");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"open_mapping_work_area . Spectrum is not in manager!");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"open_mapping_work_area . Spectrum_manager not present");
						}
#else
						Spectrum_set_simple_type(drawing_information->spectrum,
							BLUE_TO_RED_SPECTRUM);
#endif /* defined (UNEMAP_USE_NODES) */

						/*???system->window_shell as parent ? */
						open_mapping_window(&(system->acquisition.mapping_window),
							system->mapping_button,system->window_shell,
							&(system->mapping.window_shell),&(system->mapping.outer_form),
							&(system->mapping.current_mapping_window),
							&(system->mapping.open),&(system->mapping.associate),
							&(system->analysis.map_type),HIDE_COLOUR,HIDE_CONTOURS,
							SHOW_ELECTRODE_NAMES,HIDE_FIBRES,HIDE_LANDMARKS,HIDE_EXTREMA,
							maintain_aspect_ratio,1,HAMMER_PROJECTION,VARIABLE_THICKNESS,
							&(system->acquisition.rig),(int *)NULL,(int *)NULL,(int *)NULL,
							(int *)NULL,(int *)NULL,system->acquisition_colour,
							ACQUISITION_ASSOCIATE,(XtPointer)set_mapping_acquisition_region,
							(XtPointer)NULL,(XtPointer)NULL,(XtPointer)&(system->acquisition),
							User_interface_get_screen_width(user_interface),User_interface_get_screen_height(user_interface),
							system->configuration_file_extension,
							system->postscript_file_extension,system->map_drawing_information,
							user_interface,system->unemap_package,
							&((system->analysis).first_eimaging_event),
							&((system->analysis).analysis_mode));
					} break;
					case ANALYSIS_ASSOCIATE:
					{
						/* if there is not an analysis mapping window */
						if (!(system->analysis.mapping_window))
						{
							/* if there is not an analysis window */
							if (!(system->analysis.window))
							{
								/* create the analysis work area */
								if (!create_analysis_work_area(&(system->analysis),
									system->analysis_button,system->window_shell,
									system->pointer_sensitivity,
									system->signal_file_extension_read,
									system->signal_file_extension_write,
									system->postscript_file_extension,
									system->configuration_file_extension,
									system->analysis_colour,system->map_drawing_information,
									user_interface, system->time_keeper,
									system->unemap_package))
								{
									display_message(ERROR_MESSAGE,
							"open_mapping_work_area.  Could not create analysis work area");
								}
							}
						}
						spectrum=drawing_information->spectrum;
#if defined (UNEMAP_USE_NODES)
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
#endif /* defined (UNEMAP_USE_NODES) */

									/*???system->window_shell as parent ? */
									switch (system->analysis.map_type)
									{
										case SINGLE_ACTIVATION:
										case MULTIPLE_ACTIVATION:
										{
											Spectrum_set_simple_type(
												spectrum_to_be_modified_copy,
												RED_TO_BLUE_SPECTRUM);
										} break;
										default:
										{
											Spectrum_set_simple_type(
												spectrum_to_be_modified_copy,
												BLUE_TO_RED_SPECTRUM);
										} break;
									}
#if defined (UNEMAP_USE_NODES)
									MANAGER_MODIFY_NOT_IDENTIFIER(Spectrum,name)(spectrum,
										spectrum_to_be_modified_copy,spectrum_manager);
									DESTROY(Spectrum)(&spectrum_to_be_modified_copy);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"open_mapping_work_area. Could not create spectrum copy.");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"open_mapping_work_area. Spectrum is not in manager!");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"open_mapping_work_area. Spectrum_manager not present");
						}
#endif /* defined (UNEMAP_USE_NODES) */
						open_mapping_window(&(system->analysis.mapping_window),
							system->mapping_button,system->window_shell,
							&(system->mapping.window_shell),&(system->mapping.outer_form),
							&(system->mapping.current_mapping_window),
							&(system->mapping.open),&(system->mapping.associate),
							&(system->analysis.map_type),HIDE_COLOUR,HIDE_CONTOURS,
							SHOW_ELECTRODE_NAMES,HIDE_FIBRES,HIDE_LANDMARKS,HIDE_EXTREMA,
							maintain_aspect_ratio,1,HAMMER_PROJECTION,VARIABLE_THICKNESS,
							&(system->analysis.rig),&(system->analysis.event_number),
							&(system->analysis.potential_time),&(system->analysis.datum),
							&(system->analysis.start_search_interval),
							&(system->analysis.end_search_interval),
							system->analysis_colour,ANALYSIS_ASSOCIATE,
							(XtPointer)set_mapping_analysis_region,
							(XtPointer)analysis_select_map_drawing_are,
							(XtPointer)analysis_select_auxiliary_drawi,
							(XtPointer)&(system->analysis),User_interface_get_screen_width(user_interface),
							User_interface_get_screen_height(user_interface),
							system->configuration_file_extension,
							system->postscript_file_extension,system->map_drawing_information,
							user_interface,system->unemap_package,
							&((system->analysis).first_eimaging_event),
							&((system->analysis).analysis_mode));
					} break;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"open_mapping_work_area.  Could not create mapping work area");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"open_acquisition_work_area.  Could not register callbacks");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"open_mapping_work_area.  Missing system_window");
	}
	LEAVE;
} /* open_mapping_work_area */

static void identify_system_close_button(Widget *widget_id,
	XtPointer system_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the system close button.
==============================================================================*/
{
	struct System_window *system;

	ENTER(identify_system_close_button);
	USE_PARAMETER(call_data);
	if (system=(struct System_window *)system_window)
	{
		system->close_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_system_close_button.  Missing client_data");
	}
	LEAVE;
} /* identify_system_close_button */

static void System_window_close_callback(Widget widget, XtPointer system_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 July 2002

DESCRIPTION :
Calls the prescribed system_window's close_callback.
==============================================================================*/
{
	struct System_window *system;

	ENTER(close_emap);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((system = (struct System_window *)system_window) &&
		system->close_callback)
	{
		(system->close_callback)(system, system->close_callback_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"System_window_close_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* System_window_close_callback */

/*
Global functions
----------------
*/
struct System_window *CREATE(System_window)(Widget shell,
	Unemap_system_window_close_callback_procedure *close_callback,
	void *close_callback_data,
#if defined (UNEMAP_USE_3D)
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct FE_element_selection *element_selection,
	struct MANAGER(FE_field) *fe_field_manager,
	struct FE_node_selection *node_selection,
	struct FE_node_selection *data_selection,
	struct FE_time *fe_time,
	struct MANAGER(FE_basis) *fe_basis_manager,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(FE_node) *data_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(Texture) *texture_manager,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct MANAGER(Scene) *scene_manager,
	struct MANAGER(Light_model) *light_model_manager,
	struct MANAGER(Light) *light_manager,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct LIST(GT_object) *glyph_list,
	struct Graphical_material *graphical_material,
	struct Computed_field_package *computed_field_package,
	struct Light *light,
	struct Light_model *light_model,
#endif /* defined (UNEMAP_USE_3D) */
	struct Time_keeper *time_keeper,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 13 September 2002

DESCRIPTION :
This function allocates the memory for a system window structure.  It then
retrieves a system window widget with the specified parent/<shell> and assigns
the widget ids to the appropriate fields of the structure.  It returns a
pointer to the created structure if successful and NULL if unsuccessful.
Note the system window is automatically popped-up in this function.
The <close_callback> and associated <close_callback_data> are called when the
user closes the system window. These must be supplied, and are responsible for
the handling the differences in cleaning up Unemap when it is run in Cmgui and
as a standalone application.
==============================================================================*/
{
	char *temp_string;
	Dimension window_width = 0, window_height = 0;
	MrmType system_window_class;
	static MrmRegisterArg
		callback_list[]=
		{
			{"close_emap",(XtPointer)System_window_close_callback},
			{"identify_system_acquisition_but",
				(XtPointer)identify_system_acquisition_but},
			{"open_acquisition_work_area",(XtPointer)open_acquisition_work_area},
			{"identify_system_analysis_button",
				(XtPointer)identify_system_analysis_button},
			{"open_analysis_work_area",(XtPointer)open_analysis_work_area},
			{"identify_system_mapping_button",
				(XtPointer)identify_system_mapping_button},
			{"open_mapping_work_area",(XtPointer)open_mapping_work_area},
			{"identify_system_close_button",(XtPointer)identify_system_close_button},
		},
		global_callback_list[]=
		{
			{"close_mapping_work_area",(XtPointer)close_mapping_work_area},
		},
		identifier_list[]=
		{
			{"system_window_structure",(XtPointer)NULL}
		};
#define XmNacquisitionColour "acquisitionColour"
#define XmCAcquisitionColour "AcquisitionColour"
#define XmNanalysisColour "analysisColour"
#define XmCAnalysisColour "AnalysisColour"
#define XmNconfigurationDirectory "configurationDirectory"
#define XmCConfigurationDirectory "ConfigurationDirectory"
#define XmNconfigurationFileExtension "configurationFileExtension"
#define XmCConfigurationFileExtension "ConfigurationFileExtension"
#define XmNeventDetectionAlgorithm "eventDetectionAlgorithm"
#define XmCEventDetectionAlgorithm "EventDetectionAlgorithm"
#define XmNeventDetectionObjective "eventDetectionObjective"
#define XmCEventDetectionObjective "EventDetectionObjective"
#define XmNgradientAverageWidth "gradientAverageWidth"
#define XmCGradientAverageWidth "GradientAverageWidth"
#define XmNlevelValue "levelValue"
#define XmCLevelValue "LevelValue"
#define XmNpointerSensitivity "pointerSensitivity"
#define XmCPointerSensitivity "PointerSensitivity"
#define XmNpostscriptFileExtension "postscriptFileExtension"
#define XmCPostscriptFileExtension "PostscriptFileExtension"
#define XmNsignalFileExtension "signalFileExtension"
#define XmCSignalFileExtension "SignalFileExtension"
#define XmNsignalFileExtensionSaveas "signalFileExtensionSaveas"
#define XmCSignalFileExtensionSaveas "SignalFileExtensionSaveas"
#define XmNthresholdMinimumSeparation "thresholdMinimumSeparation"
#define XmCThresholdMinimumSeparation "ThresholdMinimumSeparation"
#define XmNthresholdThreshold "thresholdThreshold"
#define XmCThresholdThreshold "ThresholdThreshold"
	static XtResource resources_1[]=
	{
		{
			XmNacquisitionColour,
			XmCAcquisitionColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(System_window_settings,acquisition_colour),
			XmRString,
			"lightsteelblue"
		},
		{
			XmNanalysisColour,
			XmCAnalysisColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(System_window_settings,analysis_colour),
			XmRString,
			"aquamarine"
		},
		{
			XmNconfigurationDirectory,
			XmCConfigurationDirectory,
			XmRString,
			sizeof(char *),
			XtOffsetOf(System_window_settings,configuration_directory),
			XmRString,
			""
		},
		{
			XmNconfigurationFileExtension,
			XmCConfigurationFileExtension,
			XmRString,
			sizeof(char *),
			XtOffsetOf(System_window_settings,configuration_file_extension),
			XmRString,
			"cnfg"
		},
		{
			XmNgradientAverageWidth,
			XmCGradientAverageWidth,
			XmRInt,
			sizeof(int),
			XtOffsetOf(System_window_settings,analysis.average_width),
			XmRString,
			"6"
		},
		{
			XmNlevelValue,
			XmCLevelValue,
			XmRFloat,
			sizeof(float),
			XtOffsetOf(System_window_settings,analysis.level),
			XmRString,
			"0"
		},
		{
			XmNpointerSensitivity,
			XmCPointerSensitivity,
			XmRInt,
			sizeof(int),
			XtOffsetOf(System_window_settings,pointer_sensitivity),
			XmRString,
			"2"
		},
		{
			XmNpostscriptFileExtension,
			XmCPostscriptFileExtension,
			XmRString,
			sizeof(char *),
			XtOffsetOf(System_window_settings,postscript_file_extension),
			XmRString,
			"ps"
		},
		{
			XmNsignalFileExtension,
			XmCSignalFileExtension,
			XmRString,
			sizeof(char *),
			XtOffsetOf(System_window_settings,signal_file_extension_read),
			XmRString,
			"signal"
		},
		{
			XmNsignalFileExtensionSaveas,
			XmCSignalFileExtensionSaveas,
			XmRString,
			sizeof(char *),
			XtOffsetOf(System_window_settings,signal_file_extension_write),
			XmRString,
			"signal"
		},
		{
			XmNthresholdMinimumSeparation,
			XmCThresholdMinimumSeparation,
			XmRInt,
			sizeof(int),
			XtOffsetOf(System_window_settings,analysis.minimum_separation),
			XmRString,
			"100"
		},
		{
			XmNthresholdThreshold,
			XmCThresholdThreshold,
			XmRInt,
			sizeof(int),
			XtOffsetOf(System_window_settings,analysis.threshold),
			XmRString,
			"90"
		},
	};
	static XtResource resources_2[]=
	{
		{
			XmNeventDetectionAlgorithm,
			XmCEventDetectionAlgorithm,
			XmRString,
			sizeof(char *),
			0,
			XmRString,
			"interval"
		},
	};
	static XtResource resources_3[]=
	{
		{
			XmNeventDetectionObjective,
			XmCEventDetectionObjective,
			XmRString,
			sizeof(char *),
			0,
			XmRString,
			"absolute slope"
		},
	};
#if defined (UNEMAP_USE_3D)
	struct Standard_torso_defaults
	{
		char *standard_torso_file;
	};
#define XmNstandardTorso "standardTorso"
#define XmCStandardTorso "StandardTorso"
	struct FE_field *map_fit_field;
	struct Standard_torso_defaults standard_torso_defaults;
	static XtResource standard_torso_resources[]=
	{
		{
			XmNstandardTorso,
			XmCStandardTorso,
			XmRString,
			sizeof(char *),
			XtOffsetOf(struct Standard_torso_defaults,standard_torso_file),
			XmRString,
			""
		}
	};
#endif /* defined (UNEMAP_USE_3D) */
	struct System_window *system;
	struct System_window_data
	{
		Position x;
		Position y;
	} system_window_data;
	static XtResource System_window_resources[]=
	{
		{
			XmNx,
			XmCPosition,
			XmRPosition,
			sizeof(Position),
			XtOffsetOf(struct System_window_data,x),
			XmRImmediate,
			(XtPointer) -1
		},
		{
			XmNy,
			XmCPosition,
			XmRPosition,
			sizeof(Position),
			XtOffsetOf(struct System_window_data,y),
			XmRImmediate,
			(XtPointer) -1
		}
	};

	ENTER(CREATE(System_window));
	if (user_interface && close_callback && close_callback_data &&
#if defined (UNEMAP_USE_3D)
		element_point_ranges_selection && element_selection &&
		node_selection && data_selection && texture_manager &&
		interactive_tool_manager && scene_manager && light_model_manager &&
		light_manager && spectrum_manager && graphical_material_manager &&
		data_manager && glyph_list && graphical_material &&
		computed_field_package && light && light_model &&
#endif /* defined (UNEMAP_USE_3D) */
		time_keeper && user_interface)
	{
		if (MrmOpenHierarchy_base64_string(system_window_uidh,
			&system_window_hierarchy, &system_window_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(system, struct System_window, 1))
			{
				system->close_callback = close_callback;
				system->close_callback_data = close_callback_data;
				system->unemap_package = (struct Unemap_package *)NULL;
				system->window_shell = shell;
				system->user_interface = user_interface;
				system->map_drawing_information =
					create_Map_drawing_information(user_interface
#if defined (UNEMAP_USE_3D)
						, element_point_ranges_selection, element_selection,
						node_selection, data_selection, texture_manager,
						interactive_tool_manager, scene_manager, light_model_manager,
						light_manager, spectrum_manager, graphical_material_manager,
						data_manager, glyph_list, graphical_material,
						computed_field_package, light, light_model
#endif /* defined (UNEMAP_USE_3D) */
						);
#if defined (UNEMAP_USE_3D)
				set_map_drawing_information_user_interface(
					system->map_drawing_information,user_interface);
				set_map_drawing_information_time_keeper(system->map_drawing_information,
					time_keeper);
#endif /* defined (UNEMAP_USE_3D) */
				system->configuration_directory=(char *)NULL;
				system->configuration_file_extension=(char *)NULL;
				system->postscript_file_extension=(char *)NULL;
				system->signal_file_extension_read=(char *)NULL;
				system->signal_file_extension_write=(char *)NULL;
				system->pointer_sensitivity=0;
				system->window=(Widget)NULL;
				system->acquisition_button=(Widget)NULL;
				system->acquisition.window_shell=(Widget)NULL;
				system->acquisition.activation=(Widget)NULL;
#if defined (UNIMA_ACQUISITION)
				system->acquisition.window=(struct Acquisition_window *)NULL;
#else /* defined (UNIMA_ACQUISITION) */
				system->acquisition.window=(struct Page_window *)NULL;
#endif /* defined (UNIMA_ACQUISITION) */
				system->acquisition.mapping_work_area= &(system->mapping);
				system->acquisition.mapping_window=(struct Mapping_window *)NULL;
				system->acquisition.rig=(struct Rig *)NULL;
				system->analysis_button=(Widget)NULL;
				system->analysis.window_shell=(Widget)NULL;
				system->analysis.activation=(Widget)NULL;
				system->analysis.window=(struct Analysis_window *)NULL;
				system->analysis.trace=(struct Trace_window *)NULL;
				system->analysis.mapping_work_area= &(system->mapping);
				system->analysis.mapping_window=(struct Mapping_window *)NULL;
				system->analysis.raw_rig=(struct Rig *)NULL;
				system->analysis.rig=(struct Rig *)NULL;
				system->analysis.highlight=(struct Device **)NULL;
				system->analysis.datum=0;
				system->analysis.objective=ABSOLUTE_SLOPE;
				system->analysis.detection=EDA_INTERVAL;
				system->analysis.objective=ABSOLUTE_SLOPE;
				system->analysis.datum_type=AUTOMATIC_DATUM;
				system->analysis.edit_order=DEVICE_ORDER;
				system->analysis.signal_order=CHANNEL_ORDER;
				system->analysis.calculate_events=0;
				system->analysis.number_of_events=1;
				system->analysis.event_number=1;
				system->analysis.threshold=90;
				system->analysis.minimum_separation=100;
				system->analysis.level=0;
				system->analysis.map_type=NO_MAP_FIELD;
				system->analysis.bard_signal_file_data=(struct File_open_data *)NULL;
				system->analysis.cardiomapp_signal_file_data=
					(struct File_open_data *)NULL;
				system->analysis.neurosoft_signal_file_data=
					(struct File_open_data *)NULL;
				system->mapping_button=(Widget)NULL;
				system->mapping.activation= &(system->mapping_button);
				system->mapping.outer_form=(Widget)NULL;
				system->mapping.parent= &(system->window_shell);
				system->mapping.window_shell=(Widget)NULL;
				system->mapping.current_mapping_window=(struct Mapping_window *)NULL;
				system->mapping.open=0;
				system->mapping.associate=ACQUISITION_ASSOCIATE;
				if (time_keeper)
				{
					system->time_keeper=ACCESS(Time_keeper)(time_keeper);
				}
				else
				{
					system->time_keeper=(struct Time_keeper *)NULL;
				}
				system->close_button=(Widget)NULL;
				/* retrieve the settings */
				XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),
					system,resources_1,XtNumber(resources_1),NULL);
				if (system->configuration_file_extension)
				{
					if (0<strlen(system->configuration_file_extension))
					{
						if (ALLOCATE(temp_string,char,
							strlen(system->configuration_file_extension)+2))
						{
							strcpy(temp_string,".");
							strcat(temp_string,system->configuration_file_extension);
							system->configuration_file_extension=temp_string;
						}
					}
					else
					{
						system->configuration_file_extension=(char *)NULL;
					}
				}
				if (system->postscript_file_extension)
				{
					if (0<strlen(system->postscript_file_extension))
					{
						if (ALLOCATE(temp_string,char,
							strlen(system->postscript_file_extension)+2))
						{
							strcpy(temp_string,".");
							strcat(temp_string,system->postscript_file_extension);
							system->postscript_file_extension=temp_string;
						}
					}
					else
					{
						system->postscript_file_extension=(char *)NULL;
					}
				}
				if (system->signal_file_extension_read)
				{
					if (0<strlen(system->signal_file_extension_read))
					{
						if (ALLOCATE(temp_string,char,
							strlen(system->signal_file_extension_read)+2))
						{
							strcpy(temp_string,".");
							strcat(temp_string,system->signal_file_extension_read);
							system->signal_file_extension_read=temp_string;
						}
					}
					else
					{
						system->signal_file_extension_read=(char *)NULL;
					}
				}
				if (system->signal_file_extension_write)
				{
					if (0<strlen(system->signal_file_extension_write))
					{
						if (ALLOCATE(temp_string,char,
							strlen(system->signal_file_extension_write)+2))
						{
							strcpy(temp_string,".");
							strcat(temp_string,system->signal_file_extension_write);
							system->signal_file_extension_write=temp_string;
						}
					}
					else
					{
						system->signal_file_extension_write=(char *)NULL;
					}
				}
				XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),
					&temp_string,resources_2,XtNumber(resources_2),NULL);
				if (fuzzy_string_compare(temp_string,"threshold"))
				{
					system->analysis.detection=EDA_THRESHOLD;
				}
				else
				{
					if (fuzzy_string_compare(temp_string,"level"))
					{
						system->analysis.detection=EDA_LEVEL;
					}
					else
					{
						system->analysis.detection=EDA_INTERVAL;
					}
				}
				XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),
					&temp_string,resources_3,XtNumber(resources_3),NULL);
				if (fuzzy_string_compare(temp_string,"negative slope"))
				{
					system->analysis.objective=NEGATIVE_SLOPE;
				}
				else
				{
					if (fuzzy_string_compare(temp_string,"positive slope"))
					{
						system->analysis.objective=POSITIVE_SLOPE;
					}
					else
					{
						/* for backwards compatability */
						if (fuzzy_string_compare(temp_string,"value"))
						{
							system->analysis.objective=POSITIVE_VALUE;
						}
						else
						{
							if (fuzzy_string_compare(temp_string,"absolute value"))
							{
								system->analysis.objective=ABSOLUTE_VALUE;
							}
							else
							{
								if (fuzzy_string_compare(temp_string,"positive value"))
								{
									system->analysis.objective=POSITIVE_VALUE;
								}
								else
								{
									if (fuzzy_string_compare(temp_string,"negative value"))
									{
										system->analysis.objective=NEGATIVE_VALUE;
									}
									else
									{
										system->analysis.objective=ABSOLUTE_SLOPE;
									}
								}
							}
						}
					}
				}
				/*???DB.  Retrieve settings */
				/* register the callbacks */
				if ((MrmSUCCESS==MrmRegisterNamesInHierarchy(system_window_hierarchy,
					callback_list,XtNumber(callback_list)))&&(MrmSUCCESS==
					MrmRegisterNames(global_callback_list,
					XtNumber(global_callback_list))))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)system;
						/*???DB.  Has to be in global because more than one set of uids use
							it.  Problems with multiple system_windows ? */
					if (MrmSUCCESS==MrmRegisterNames(identifier_list,
						XtNumber(identifier_list)))
					{
						/* fetch system window widget */
						if (MrmSUCCESS==MrmFetchWidget(system_window_hierarchy,
							"system_window",system->window_shell,&(system->window),
							&system_window_class))
						{
							create_Shell_list_item(&(system->window_shell), user_interface);
							/*!!!!*/
#if defined (UNEMAP_USE_3D)
							if (system->unemap_package = CREATE(Unemap_package)(
								fe_field_manager, fe_time, element_group_manager, node_manager,
								data_manager, data_group_manager, node_group_manager,
								fe_basis_manager, element_manager,
								Computed_field_package_get_computed_field_manager(
									computed_field_package),
								interactive_tool_manager, node_selection))
							{
								ACCESS(Unemap_package)(system->unemap_package);
								/* create and store the map fit field  */
								map_fit_field = create_mapping_type_fe_field("fit",
									fe_field_manager, fe_time);
								set_unemap_package_map_fit_field(
									system->unemap_package, map_fit_field);
#if defined (MOTIF)		
								/* get the location of the default_torso file from XResources */
								standard_torso_defaults.standard_torso_file = "";			
								XtVaGetApplicationResources(system->window_shell,
									&standard_torso_defaults, standard_torso_resources,
									XtNumber(standard_torso_resources), NULL);
								/* do nothing if no default torso file specified */
								if (strcmp(standard_torso_defaults.standard_torso_file, ""))
								{
									unemap_package_read_torso_file(system->unemap_package,
										standard_torso_defaults.standard_torso_file);
								}
#endif /* defined (MOTIF)	*/
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"CREATE(System_window).  Could not create unemap package");
							}
#endif /* defined (UNEMAP_USE_3D) */
#if defined (MOTIF)		
							/* determine placement */
							XtVaGetValues(system->window_shell,
								XmNwidth,&window_width,
								XmNheight,&window_height,
								NULL);
							/* do this to allow backward compatibility but still allow the
								 resources to be set */
							/* these defaults match with the default resources above */
							system_window_data.x = -1;
							system_window_data.y = -1;
							XtVaGetApplicationResources(system->window_shell,
								&system_window_data, System_window_resources,
								XtNumber(System_window_resources),NULL);
							if (-1 == system_window_data.x)
							{
								system_window_data.x =
									((User_interface_get_screen_width(user_interface))
										-window_width)/2;
							}
							if (-1 == system_window_data.y)
							{
								system_window_data.y =
									((User_interface_get_screen_height(user_interface))
										-window_height)/2;
							}
							XtVaSetValues(system->window_shell,
								XmNx, system_window_data.x,
								XmNy, system_window_data.y,
								XmNmappedWhenManaged, True,
								NULL);
#endif /* defined (MOTIF) */
							/* manage the system window */
							XtManageChild(system->window);
							/* realize the system window shell */
							XtRealizeWidget(system->window_shell);
							/* pop up the system window shell */
							XtPopup(system->window_shell, XtGrabNone);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"CREATE(System_window).  Could not fetch system window");
							DEALLOCATE(system);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"CREATE(System_window).  Could not register identifiers");
						DEALLOCATE(system);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CREATE(System_window).  Could not register callbacks");
					DEALLOCATE(system);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(System_window).  Could not allocate system window structure");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(System_window).  Could not open hierarchy");
			system=(struct System_window *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(System_window). Invalid arguments");
		system=(struct System_window *)NULL;
	}
	LEAVE;

	return (system);
} /* CREATE(System_window) */

int DESTROY(System_window)(struct System_window **system_window_address)
/*******************************************************************************
LAST MODIFIED : 13 September 2002

DESCRIPTION :
Destroys the Unemap system window and all its dependent windows.
==============================================================================*/
{
	int return_code;
	struct System_window *system;

	ENTER(DESTROY(System_window));
	if (system_window_address && (system = *system_window_address))
	{
		if (system->time_keeper)
		{
			DEACCESS(Time_keeper)(&(system->time_keeper));
		}
		/* close acquisition */
		if ((system->acquisition).window_shell)
		{
			close_acquisition_work_area((Widget)NULL,
				(XtPointer)&(system->acquisition),(XtPointer)NULL);
		}
#if defined (UNIMA)
		if (((system->acquisition).window)&&
			(((system->acquisition).window)->experiment_toggle)&&
			(True==XmToggleButtonGetState(((system->acquisition).window)->
			experiment_toggle)))
		{
			/* reset the Unima system */
			U00SysReset();
			/* reset the Unima adapter */
			UAReset();
			UASetAdapter(DISABLE);
		}
#endif
		/* destroy acquisition */
		if ((system->acquisition).window)
		{
			destroy_Page_window(&((system->acquisition).window));
		}

		/* close analysis */
		if ((system->analysis).window_shell)
		{
			close_analysis_work_area((Widget)NULL, (XtPointer)&(system->analysis),
				(XtPointer)NULL);
			destroy_analysis_work_area(&(system->analysis));
		}
		/* close mapping */
		if ((system->mapping).window_shell)
		{
			close_mapping_work_area((Widget)NULL, (XtPointer)system, (XtPointer)NULL);
		}
		/* close system */
		if (system->window_shell !=
			User_interface_get_application_shell(system->user_interface))
		{
			/*???RC Used to just popdown */
			/*XtPopdown(system->window_shell);*/
			XtDestroyWidget(system->window_shell);
		}
	#if defined (UNEMAP_USE_3D)
		DEACCESS(Unemap_package)(&(system->unemap_package));
	#endif /* defined (UNEMAP_USE_3D) */
		*system_window_address = (struct System_window *)NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(System_window).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(System_window) */

int System_window_pop_up(struct System_window *system_window)
/*******************************************************************************
LAST MODIFIED : 17 July 2002

DESCRIPTION :
De-iconifies and brings the unemap system window to the front.
==============================================================================*/
{
	int return_code;

	ENTER(System_window_pop_up);
	if (system_window)
	{
		XtPopup(system_window->window_shell,XtGrabNone);
		/* make sure it is not iconic */
		XtVaSetValues(system_window->window_shell, XmNiconic, False, NULL);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"System_window_pop_up.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* System_window_pop_up */
