/*******************************************************************************
FILE : mapping_work_area.c

LAST MODIFIED : 26 December 1992

DESCRIPTION :
???DB.  Not currently used
==============================================================================*/
#include <stddef.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#define MAPPING_WORK_AREA
#include "mapping_work_area.h"
#include "debug.h"
#include "mymemory.h"
#include "message.h"
#include "user_interface.h"
#include "mapping.h"
#include "mapping_window.h"

/*
Module functions
----------------
*/
static void close_mapping_work_area(Widget widget,XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 26 July 1992

DESCRIPTION :
Closes the windows associated with the mapping work area.
==============================================================================*/
{
	struct System_window *system;

	ENTER(close_mapping_work_area);
	if (system=(struct System_window *)client_data)
	{
		if (system->mapping.window_shell)
		{
			system->mapping.open=1;
			/* close the mapping shell */
			XtPopdown(system->mapping.window_shell);
			/* unghost the system mapping button */
			XtSetSensitive(system->mapping_button,True);
		}
		else
		{
			print_message(1,
				"close_mapping_work_area.  mapping window shell is missing");
		}
	}
	else
	{
		print_message(1,"close_mapping_work_area.  Missing client_data");
	}
	LEAVE;
} /* close_mapping_work_area */

/*
Global functions
----------------
*/
int create_mapping_work_area(struct Mapping_work_area *mapping,Widget parent)
/*******************************************************************************
LAST MODIFIED : 26 December 1992

DESCRIPTION :
Creates the windows associated with the mapping work area.
???system_window_structure
==============================================================================*/
{
	int return_code;
	static MrmRegisterArg callback_list[]={
		{"close_mapping_work_area",(XtPointer)close_mapping_work_area}};
/*#define NUMBER_MAPPING_IDENTIFIERS 1
	static MrmRegisterArg identifier_list[NUMBER_MAPPING_IDENTIFIERS];*/
	MrmType outer_form_class;

	ENTER(create_mapping_work_area);
	if (mapping)
	{
		return_code=1;
		/* if there is not a mapping window shell */
		if (!(mapping->window_shell))
		{
			/* create the mapping window shell */
			if (create_mapping_window_shell(&(mapping->window_shell),parent))
			{
				/* retrieve the outer form */
				if (MrmFetchWidget(hierarchy,"mapping_window_outer_form",
					mapping->window_shell,&(mapping->outer_form),&outer_form_class)==
					MrmSUCCESS)
				{
					/* manage the outer form */
					XtManageChild(mapping->outer_form);
					/* realize the mapping window shell */
					XtRealizeWidget(mapping->window_shell);
					mapping->open=0;
				}
				else
				{
					print_message(1,
						"create_mapping_work_area.  Could not retrieve outer form");
					XtDestroyWidget(mapping->window_shell);
					return_code=0;
				}
			}
			else
			{
				print_message(1,
					"create_mapping_work_area.  Could not create mapping window shell");
				return_code=0;
			}
		}
		/* if there is a mapping window shell */
		if (return_code)
		{
			/* register the callbacks */
			if (MrmRegisterNamesInHierarchy(hierarchy,callback_list,
				XtNumber(callback_list))==MrmSUCCESS)
			{
				/* assign and register the identifiers */
				identifier_list[0].name="system_window_structure";
				identifier_list[0].value=(XtPointer)system;
				if (MrmRegisterNamesInHierarchy(hierarchy,identifier_list,
					NUMBER_MAPPING_IDENTIFIERS)==MrmSUCCESS)
				{
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
									if (!create_acquisition_work_area(system))
									{
										print_message(1,
					"create_mapping_work_area.  Could not create acquisition work area");
										return_code=0;
									}
								}
								/* if there is an acquisition window */
								if (return_code)
								{
									if (create_Mapping_window(
										&(system->acquisition.mapping_window),
										system->mapping_button,system->mapping.outer_form,
										create_Map(NO_COLOUR_MAP,HIDE_COLOUR,HIDE_CONTOURS,
										SHOW_ELECTRODE_NAMES,HAMMER,&(system->acquisition.rig),
										(int *)NULL,(int *)NULL),(int *)NULL,
										&(system->acquisition.rig),
										user_settings.acquisition_colour))
									{
										/* ghost the map button */
										XtSetSensitive(system->acquisition.mapping_window->
											map_button,False);
										if (system->acquisition.rig)
										{
											if (system->acquisition.rig->experiment==EXPERIMENT_ON)
											{
												/* ghost the file button */
												XtSetSensitive(system->acquisition.mapping_window->
													file_button,False);
											}
										}
										else
										{
											/* ghost the save configuration button in the file menu */
											XtSetSensitive(system->acquisition.mapping_window->
												file_menu.save_configuration_button,False);
											/* ghost the set default configuration button in the file
												menu */
											XtSetSensitive(system->acquisition.mapping_window->
												file_menu.set_default_configuration_button,False);
										}
									}
									else
									{
										print_message(1,
			"create_mapping_work_area.  Could not create acquisition mapping window");
										return_code=0;
									}
								}
							}
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
									if (!create_analysis_work_area(system))
									{
										print_message(1,
							"create_mapping_work_area.  Could not create analysis work area");
										return_code=0;
									}
								}
								/* if there is an analysis window */
								if (return_code)
								{
									if (create_Mapping_window(&(system->analysis.mapping_window),
										system->mapping_button,system->mapping.outer_form,
										create_Map(ACTIVATION,HIDE_COLOUR,HIDE_CONTOURS,i
										SHOW_ELECTRODE_NAMES,HAMMER,&(system->analysis.rig),
										(int *)NULL,(int *)NULL),(int *)NULL,
										&(system->analysis.rig),
										user_settings.analysis_colour))
									{
										XtAddCallback(system->analysis.mapping_window->
											map_drawing_area,XmNinputCallback,
											analysis_select_map_drawing_are,
											(XtPointer)&(system->analysis));
										XtAddCallback(system->analysis.mapping_window->
											colour_or_auxiliary_drawing_area,XmNinputCallback,
											analysis_select_auxiliary_drawi,
											(XtPointer)&(system->analysis));
										/* ghost the mapping setup button */
										XtSetSensitive(system->analysis.mapping_window->
											setup_button,False);
										/* ghost the mapping page button */
										XtSetSensitive(system->analysis.mapping_window->page_button,
											False);
										if (system->analysis.rig)
										{
											/* ghost the read configuration button in the mapping file
												menu */
											XtSetSensitive(system->analysis.mapping_window->file_menu.
												read_configuration_button,False);
										}
										else
										{
											/* ghost the mapping file button */
											XtSetSensitive(system->analysis.mapping_window->
												file_button,False);
										}
									}
									else
									{
										print_message(1,
				"create_mapping_work_area.  Could not create analysis mapping window");
									}
								}
							}
						} break;
						default:
						{
							system->mapping.associate=ACQUISITION_ASSOCIATE;
							print_message(1,
								"create_mapping_work_area.  Invalid mapping associate");
						} break;
					}
				}
				else
				{
					print_message(1,
						"create_mapping_work_area.  Could not register identifiers");
				}
			}
			else
			{
				print_message(1,
					"create_mapping_work_area.  Could not register callbacks");
			}
		}
	}
	else
	{
		print_message(1,"create_mapping_work_area.  Missing mapping work area");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* create_mapping_work_area */
