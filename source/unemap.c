/*******************************************************************************
FILE : unemap.c

LAST MODIFIED : 8 August 2002

DESCRIPTION :
Main program for unemap.  Based on cmgui.
???DB.  !defined (NOT_ACQUISITION_ONLY) is for getting a acquisition version for
	Oxford
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#if defined (MOTIF)
#include <Xm/MessageB.h>
#endif /* defined (MOTIF) */
#include "command/command.h"
#include "general/debug.h"
#include "general/error_handler.h"
#if defined (IMAGEMAGICK)
#include "general/image_utilities.h"
#endif /* defined (IMAGEMAGICK) */
#if defined (NOT_ACQUISITION_ONLY)
#if defined (UNEMAP_USE_3D)
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_coordinate.h"
#include "computed_field/computed_field_component_operations.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_vector_operations.h"
#include "finite_element/import_finite_element.h"
#include "graphics/glyph.h"
#include "graphics/light.h"
#include "graphics/light_model.h"
#include "graphics/material.h"
#include "graphics/scene.h"
#include "graphics/spectrum.h"
#include "graphics/transform_tool.h"
#include "interaction/interactive_tool.h"
#include "node/node_tool.h"
#include "selection/element_point_ranges_selection.h"
#include "selection/element_selection.h"
#include "selection/node_selection.h"
#endif /* defined (UNEMAP_USE_3D) */
#include "finite_element/finite_element.h"
#include "time/time_keeper.h"
#include "unemap/system_window.h"
#else /* defined (NOT_ACQUISITION_ONLY) */
#include "unemap/page_window.h"
#include "unemap/rig.h"
#include "unemap/rig_node.h"
#include "unemap/unemap_hardware.h"
#if defined (WIN32_SYSTEM)
#if defined (MIRADA)
#include "unemap/vunemapd.h"
#endif /* defined (MIRADA) */
#endif /* defined (WIN32_SYSTEM) */
#endif /* defined (NOT_ACQUISITION_ONLY) */
#include "unemap/unemap_command.h"
#include "user_interface/confirmation.h"
#include "user_interface/event_dispatcher.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

#if defined (MOTIF) && defined (CMISS)
/*???DB.  Until can remove CMISS link from user_interface */
#include "link/cmiss.h"
struct CMISS_connection *CMISS = (struct CMISS_connection *)NULL;

int DESTROY(CMISS_connection)(struct CMISS_connection **connection_address)
/*******************************************************************************
LAST MODIFIED : 11 April 1997

DESCRIPTION :
Dummy
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(CMISS_connection));
	return_code=0;
	LEAVE;

	return (return_code);
} /* DESTROY(CMISS_connection) */

int CMISS_connection_update(struct CMISS_connection **connection_address)
/*******************************************************************************
LAST MODIFIED : 3 October 2001

DESCRIPTION :
Dummy
==============================================================================*/
{
	int return_code;

	ENTER(CMISS_connection_update);
	return_code=0;
	LEAVE;

	return (return_code);
} /* CMISS_connection_update */
#endif /* defined (MOTIF) && defined (CMISS) */

/*
Module types
------------
*/
#if defined (MOTIF)
typedef struct
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
==============================================================================*/
{
	Pixel identifying_colour;
} User_settings;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
#if !defined (NOT_ACQUISITION_ONLY)
static FILE *fopen_UNEMAP_HARDWARE(char *file_name,char *type)
/*******************************************************************************
LAST MODIFIED : 11 January 1999

DESCRIPTION :
Opens a file in the UNEMAP_HARDWARE directory.
==============================================================================*/
{
	char *hardware_directory,*hardware_file_name;
	FILE *hardware_file;

	ENTER(fopen_UNEMAP_HARDWARE);
	hardware_file=(FILE *)NULL;
	if (file_name&&type)
	{
		hardware_file_name=(char *)NULL;
		if (hardware_directory=getenv("UNEMAP_HARDWARE"))
		{
			if (ALLOCATE(hardware_file_name,char,strlen(hardware_directory)+
				strlen(file_name)+2))
			{
				strcpy(hardware_file_name,hardware_directory);
#if defined (WIN32_SYSTEM)
				if ('\\'!=hardware_file_name[strlen(hardware_file_name)-1])
				{
					strcat(hardware_file_name,"\\");
				}
#else /* defined (WIN32_SYSTEM) */
				if ('/'!=hardware_file_name[strlen(hardware_file_name)-1])
				{
					strcat(hardware_file_name,"/");
				}
#endif /* defined (WIN32_SYSTEM) */
			}
		}
		else
		{
			if (ALLOCATE(hardware_file_name,char,strlen(file_name)+1))
			{
				hardware_file_name[0]='\0';
			}
		}
		if (hardware_file_name)
		{
			strcat(hardware_file_name,file_name);
			hardware_file=fopen(hardware_file_name,type);
			DEALLOCATE(hardware_file_name);
		}
	}
	LEAVE;

	return (hardware_file);
} /* fopen_UNEMAP_HARDWARE */
#endif /* !defined (NOT_ACQUISITION_ONLY) */

static int display_error_message(char *message,void *user_interface_void)
/*******************************************************************************
LAST MODIFIED : 28 January 2002

DESCRIPTION :
Display a unemap error message.
==============================================================================*/
{
	int return_code;
#if !defined (NOT_ACQUISITION_ONLY)
	FILE *unemap_debug;
#endif /* !defined (NOT_ACQUISITION_ONLY) */

	ENTER(display_error_message);
	return_code=1;
	if (user_interface_void)
	{
		confirmation_error_ok("ERROR",message,
#if defined (MOTIF)
			(Widget)NULL,
#endif /* defined (MOTIF) */
			(struct User_interface *)user_interface_void);
	}
	else
	{
#if !defined (NOT_ACQUISITION_ONLY)
		if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
		{
			fprintf(unemap_debug,"ERROR: %s\n",message);
			fclose(unemap_debug);
		}
		else
#endif /* !defined (NOT_ACQUISITION_ONLY) */
		{
			printf("ERROR: %s\n",message);
		}
	}
	LEAVE;

	return (return_code);
} /* display_error_message */

static int display_information_message(char *message,void *user_interface_void)
/*******************************************************************************
LAST MODIFIED : 28 January 2002

DESCRIPTION :
Display a unemap information message.
==============================================================================*/
{
	int return_code;
#if !defined (NOT_ACQUISITION_ONLY)
	FILE *unemap_debug;
#endif /* !defined (NOT_ACQUISITION_ONLY) */

	ENTER(display_information_message);
	return_code=1;
	if (user_interface_void)
	{
		confirmation_information_ok("INFORMATION",message,
#if defined (MOTIF)
			(Widget)NULL,
#endif /* defined (MOTIF) */
			(struct User_interface *)user_interface_void);
	}
	else
	{
#if !defined (NOT_ACQUISITION_ONLY)
		if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
		{
			fprintf(unemap_debug,"INFORMATION: %s",message);
			fclose(unemap_debug);
		}
		else
#endif /* !defined (NOT_ACQUISITION_ONLY) */
		{
			printf("INFORMATION: %s",message);
		}
	}
	LEAVE;

	return (return_code);
} /* display_information_message */

static int display_warning_message(char *message,void *user_interface_void)
/*******************************************************************************
LAST MODIFIED : 28 January 2002

DESCRIPTION :
Display a unemap warning message.
==============================================================================*/
{
	int return_code;
#if !defined (NOT_ACQUISITION_ONLY)
	FILE *unemap_debug;
#endif /* !defined (NOT_ACQUISITION_ONLY) */

	ENTER(display_warning_message);
	return_code=1;
	if (user_interface_void)
	{
		confirmation_warning_ok("WARNING",message,
#if defined (MOTIF)
			(Widget)NULL,
#endif /* defined (MOTIF) */
			(struct User_interface *)user_interface_void);
	}
	else
	{
#if !defined (NOT_ACQUISITION_ONLY)
		if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
		{
			fprintf(unemap_debug,"WARNING: %s\n",message);
			fclose(unemap_debug);
		}
		else
#endif /* !defined (NOT_ACQUISITION_ONLY) */
		{
			printf("WARNING: %s\n",message);
		}
	}
	LEAVE;

	return (return_code);
} /* display_warning_message */

void unemap_end_application_loop(
#if defined (NOT_ACQUISITION_ONLY)
	struct System_window *window,
#else /* defined (NOT_ACQUISITION_ONLY) */
	struct Page_window *window,
#endif /* defined (NOT_ACQUISITION_ONLY) */
	void *user_interface_void)
/*******************************************************************************
LAST MODIFIED : 8 August 2002

DESCRIPTION :
Ends the application main loop. Callback function for when the close button is
clicked in the unemap system window -- but only if unemap system window is run
as the main application.
==============================================================================*/
{
	struct User_interface *user_interface;

	ENTER(unemap_end_application_loop);
	USE_PARAMETER(window);
	if (user_interface=(struct User_interface *)user_interface_void)
	{
		User_interface_end_application_loop(user_interface);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_end_application_loop.  Invalid argument(s)");
	}
	LEAVE;
} /* unemap_end_application_loop */

/*
Main program
------------
*/
#if defined (UNIX) /* switch (OPERATING_SYSTEM) */
int main(int argc,char *argv[])
#elif defined (WIN32_USER_INTERFACE) /* switch (OPERATING_SYSTEM) */
int WINAPI WinMain(HINSTANCE current_instance,HINSTANCE previous_instance,
	LPSTR command_line,int initial_main_window_state)
	/* using WinMain as the entry point tells Windows that it is a gui and to use
		the graphics device interface functions for I/O */
	/*???DB.  WINDOWS a zero return code if WinMain does get into the message
		loop.  Other application interfaces may expect something else.  Should this
		failure code be #define'd ? */
	/*???DB. Win32 SDK says that don't have to call it WinMain */
#endif /* switch (OPERATING_SYSTEM) */
/*******************************************************************************
LAST MODIFIED : 22 July 2002

DESCRIPTION :
Main program for unemap
==============================================================================*/
{
#if defined (WIN32_SYSTEM)
#if defined (MIRADA)
	HANDLE device_driver;
#endif /* defined (MIRADA) */
#endif /* defined (WIN32_SYSTEM) */
	int return_code;
	struct Execute_command *execute_command;
#if defined (NOT_ACQUISITION_ONLY)
#if defined (UNEMAP_USE_3D)
	float default_light_direction[3]={0.0,-0.5,-1.0};	
	struct Colour ambient_colour,default_colour;	
	struct Computed_field *computed_field=(struct Computed_field *)NULL;
	struct Computed_field_package *computed_field_package=
		(struct Computed_field_package *)NULL;	
	struct Coordinate_system rect_coord_system;
	struct Element_point_ranges_selection *element_point_ranges_selection=
		(struct Element_point_ranges_selection *)NULL;
	struct FE_element_selection *element_selection=
		(struct FE_element_selection *)NULL;
	struct FE_node_selection *node_selection=(struct FE_node_selection *)NULL;
	struct FE_node_selection *data_selection=(struct FE_node_selection *)NULL;
	struct FE_time *fe_time = (struct FE_time *)NULL;
	struct Graphical_material *default_graphical_material=
		(struct Graphical_material *)NULL;
	struct Graphical_material *default_selected_material=
		(struct Graphical_material *)NULL;	
	struct Graphical_material *electrode_selected_material=
		(struct Graphical_material *)NULL;
	struct Light *default_light=(struct Light *)NULL;
	struct Light_model *default_light_model=(struct Light_model *)NULL;
	struct LIST(GT_object) *glyph_list=(struct LIST(GT_object) *)NULL;
	struct MANAGER(Computed_field) *computed_field_manager=
		(struct MANAGER(Computed_field) *)NULL;
	struct MANAGER(FE_basis) *fe_basis_manager=(struct MANAGER(FE_basis) *)NULL;
	struct MANAGER(FE_field) *fe_field_manager=(struct MANAGER(FE_field) *)NULL;
	struct MANAGER(FE_element) *element_manager=
		(struct MANAGER(FE_element) *)NULL;
	struct MANAGER(FE_node) *data_manager=(struct MANAGER(FE_node) *)NULL;
	struct MANAGER(FE_node) *node_manager=(struct MANAGER(FE_node) *)NULL;
	struct MANAGER(Graphical_material) *graphical_material_manager=
		(struct MANAGER(Graphical_material) *)NULL;
	struct MANAGER(GROUP(FE_element)) *element_group_manager=
		(struct MANAGER(GROUP(FE_element)) *)NULL;
	struct MANAGER(GROUP(FE_node)) *data_group_manager=
		(struct MANAGER(GROUP(FE_node)) *)NULL;
	struct MANAGER(GROUP(FE_node))*node_group_manager=
		(struct MANAGER(GROUP(FE_node))*)NULL;		
	struct MANAGER(Interactive_tool) *interactive_tool_manager=
		(struct MANAGER(Interactive_tool) *)NULL;
	struct MANAGER(Light) *light_manager=(struct MANAGER(Light) *)NULL;
	struct MANAGER(Light_model) *light_model_manager=
		(struct MANAGER(Light_model) *)NULL;
	struct MANAGER(Scene) *scene_manager=(struct MANAGER(Scene) *)NULL;
	struct MANAGER(Spectrum) *spectrum_manager=(struct MANAGER(Spectrum) *)NULL;
	struct MANAGER(Texture) *texture_manager=(struct MANAGER(Texture) *)NULL;	
	struct Node_tool *node_tool=(struct Node_tool *)NULL;
	struct Interactive_tool *transform_tool=(struct Interactive_tool *)NULL;
#endif /* defined (UNEMAP_USE_3D) */
	struct System_window *system;
	struct Time_keeper *time_keeper;
#else /* defined (NOT_ACQUISITION_ONLY) */
	struct Page_window *page_window;
	struct Rig *acquisition_rig;
#if defined (OLD_CODE)
	char *acquisition_rig_filename;
	float sampling_frequency;
	int channel_number,*electrodes_in_row,i,index,number_of_rows;
	struct Device **device;
	struct File_open_data *acquisition_rig_open_data;
	struct Signal_buffer *signal_buffer;
	unsigned long number_of_channels,number_of_samples;
#if defined (WIN32_SYSTEM)
#if defined (MIRADA)
	short int *mirada_buffer;
	unsigned char *bus,*device_function;
	unsigned long number_of_cards;
#endif /* defined (MIRADA) */
#endif /* defined (WIN32_SYSTEM) */
#endif /* defined (OLD_CODE) */
#endif /* defined (NOT_ACQUISITION_ONLY) */
	struct Event_dispatcher *event_dispatcher=
		(struct Event_dispatcher *)NULL;
	struct User_interface *user_interface=
		(struct User_interface *)NULL;
#if defined (MOTIF)
#define XmNidentifyingColour "identifyingColour"
#define XmCIdentifyingColour "IdentifyingColour"
	static XtResource resources[]=
	{		
		XmNidentifyingColour,
		XmCIdentifyingColour,
		XmRPixel,
		sizeof(Pixel),
		XtOffsetOf(User_settings,identifying_colour),
		XmRString,
		"green"	
	};

	User_settings user_settings;
#endif /* defined (MOTIF) */
	struct Unemap_command_data *unemap_command_data;

#if defined (MOTIF)
	ENTER(main);
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
	ENTER(WinMain);
#endif /* defined (WIN32_USER_INTERFACE) */
	/* display the version */
#if defined (MOTIF)
	display_message(INFORMATION_MESSAGE, VERSION "\n");
#endif /* defined (MOTIF) */
	if ((event_dispatcher = CREATE(Event_dispatcher)())&&
		(user_interface = CREATE(User_interface)(
#if defined (UNIX) /* switch (OPERATING_SYSTEM) */
		&argc, argv, event_dispatcher,"Unemap", "unemap"
#elif defined (WIN32_USER_INTERFACE) /* switch (OPERATING_SYSTEM) */
		current_instance,previous_instance,command_line,initial_main_window_state,
		event_dispatcher
#endif /* switch (OPERATING_SYSTEM) */
		)))
	{
		/* set up messages */
		set_display_message_function(ERROR_MESSAGE,display_error_message,
			user_interface);
		/* if using nodes, print information and warning messages to stdout */
#if defined (UNEMAP_USE_3D)
		set_display_message_function(INFORMATION_MESSAGE,
			display_information_message,(void *)NULL);
		set_display_message_function(WARNING_MESSAGE,display_warning_message,
			(void *)NULL);
#else /* defined (UNEMAP_USE_3D) */
		set_display_message_function(INFORMATION_MESSAGE,
			display_information_message,(void *)NULL);
		set_display_message_function(WARNING_MESSAGE,display_warning_message,
			user_interface);
#endif /* defined (UNEMAP_USE_3D) */
		/* retrieve application specific constants */
#if defined (MOTIF)
		XtVaGetApplicationResources(
			User_interface_get_application_shell(user_interface),&user_settings,
			resources,XtNumber(resources),NULL);
			/*???DB.  User settings should be divided among tools */
#endif /* defined (MOTIF) */
#if !defined (NOT_ACQUISITION_ONLY)
#if defined (OLD_CODE)
		number_of_channels=64;
		number_of_samples=5000;
		sampling_frequency=(float)5000.;
#if defined (WIN32_SYSTEM)
#if defined (MIRADA)
		/* open the device driver */
		device_driver=CreateFile("\\\\.\\VUNEMAPD.VXD",0,0,NULL,OPEN_EXISTING,
			FILE_FLAG_DELETE_ON_CLOSE,0);
		if (INVALID_HANDLE_VALUE!=device_driver)
		{
			if (PCI_SUCCESSFUL!=get_mirada_information(device_driver,
				&number_of_cards,&bus,&device_function,&number_of_channels,
				&number_of_samples,&mirada_buffer))
			{
				display_message(ERROR_MESSAGE,
					"WinMain.  Could not get Mirada information");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"WinMain.  Could not open device driver");
		}
#endif /* defined (MIRADA) */
#if defined (NI_DAQ)
		/*???DB.  Temporary */
		{
			FILE *number_of_samples_file;

			if (number_of_samples_file=fopen("samples.txt","r"))
			{
				if (1==fscanf(number_of_samples_file," number_of_samples = %d ",
					&number_of_samples))
				{
					/* make a multiple of 1000 */
					if (number_of_samples<0)
					{
						number_of_samples=1;
					}
					number_of_samples=1000*(((number_of_samples-1)/1000)+1);
					if (1==fscanf(number_of_samples_file," sampling_frequency = %f ",
						&sampling_frequency))
					{
					}
				}
				fclose(number_of_samples_file);
			}
		}
		/*???debug */
		{
			FILE *debug_nidaq;

			if (debug_nidaq=fopen("nidaq.deb","w"))
			{
				fprintf(debug_nidaq,"number_of_samples=%d\n",number_of_samples);
				fprintf(debug_nidaq,"sampling_frequency=%g\n",sampling_frequency);
				fclose(debug_nidaq);
			}
		}
		if (!unemap_get_number_of_channels(&number_of_channels))
#if defined (OLD_CODE)
		/* open the device driver */
		if (!get_ni_information(&number_of_channels))
#endif /* defined (OLD_CODE) */
		{
			display_message(ERROR_MESSAGE,"WinMain.  Could not get NI information");
		}
#endif /* defined (NI_DAQ) */
#endif /* defined (WIN32_SYSTEM) */
#endif /* defined (OLD_CODE) */
		/* set up the acquisition rig */
		acquisition_rig=(struct Rig *)NULL;
#if defined (OLD_CODE)
#if defined (OLD_CODE)
		if (acquisition_rig_open_data=create_File_open_data(".cnfg",REGULAR,
			read_configuration_file,(void *)&acquisition_rig,0,user_interface))
		{
			open_file_and_read(
#if defined (MOTIF)
				(Widget)NULL,(XtPointer)acquisition_rig_open_data,(XtPointer)NULL
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
				acquisition_rig_open_data
#endif /* defined (WIN32_USER_INTERFACE) */
				);
			destroy_File_open_data(&acquisition_rig_open_data);
		}
#endif /* defined (OLD_CODE) */
		if (acquisition_rig_filename=confirmation_get_read_filename(".cnfg",
			user_interface))
		{
			read_configuration_file(acquisition_rig_filename,
				(void *)&acquisition_rig);
		}
#if defined (OLD_CODE)
		/*???DB.  Moved into create_Page_window */
		if (!acquisition_rig)
		{
			display_message(INFORMATION_MESSAGE,"Creating default rig");
			number_of_rows=((int)number_of_channels-1)/8+1;
			if (ALLOCATE(electrodes_in_row,int,number_of_rows))
			{
				index=number_of_rows-1;
				electrodes_in_row[index]=(int)number_of_channels-8*index;
				while (index>0)
				{
					index--;
					electrodes_in_row[index]=8;
				}
				if (acquisition_rig=create_standard_Rig("default",PATCH,MONITORING_OFF,
					EXPERIMENT_OFF,number_of_rows,electrodes_in_row,1,0,(float)1))
				{
					/*???DB.  Calibration directory */
					read_calibration_file("calibrate.dat",(void *)acquisition_rig);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"WinMain.  Error creating default rig");
				}
				DEALLOCATE(electrodes_in_row);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"WinMain.  Could not allocate electrodes_in_row");
			}
		}
#endif /* defined (OLD_CODE) */
		if (acquisition_rig)
		{
			/* set up the signal buffer */
			if (signal_buffer=create_Signal_buffer(SHORT_INT_VALUE,number_of_channels,
				number_of_samples,sampling_frequency))
			{
				/* set the times */
				for (index=0;index<(int)number_of_samples;index++)
				{
					(signal_buffer->times)[index]=index;
				}
				device=acquisition_rig->devices;
				i=acquisition_rig->number_of_devices;
				while (acquisition_rig&&(i>0))
				{
					channel_number=((*device)->channel->number)-1;
					if ((0<=channel_number)&&(channel_number<(int)number_of_channels))
					{
#if defined (MIRADA)
						index=16*(channel_number/16)+8*(channel_number%2)+
							(channel_number%16)/2;
#endif /* defined (MIRADA) */
#if defined (NI_DAQ)
						index=channel_number;
#endif /* defined (NI_DAQ) */
						if (!((*device)->signal=create_Signal(index,signal_buffer,UNDECIDED,
							0)))
						{
							display_message(ERROR_MESSAGE,
								"WinMain.  Could not create signal");
							destroy_Rig(&acquisition_rig);
						}
					}
					device++;
					i--;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"WinMain.  Could not create signal buffer");
				destroy_Rig(&acquisition_rig);
			}
		}
#endif /* defined (OLD_CODE) */
#endif /* !defined (NOT_ACQUISITION_ONLY) */
#if defined (IMAGEMAGICK)
		Open_image_environment(*argv);
#endif /* defined (IMAGEMAGICK) */
		execute_command=CREATE(Execute_command)();
#if defined (NOT_ACQUISITION_ONLY)
		time_keeper=ACCESS(Time_keeper)(CREATE(Time_keeper)("default",
			event_dispatcher,user_interface));
#if defined (UNEMAP_USE_3D)
		texture_manager=CREATE_MANAGER(Texture)();	
		fe_field_manager=CREATE_MANAGER(FE_field)();
		element_group_manager=CREATE_MANAGER(GROUP(FE_element))();
		data_manager=CREATE_MANAGER(FE_node)();
		node_manager=CREATE_MANAGER(FE_node)();
		element_manager=CREATE_MANAGER(FE_element)();
		node_group_manager=CREATE_MANAGER(GROUP(FE_node))();
		data_group_manager=CREATE_MANAGER(GROUP(FE_node))();
		fe_basis_manager=CREATE_MANAGER(FE_basis)(); 
		/* global list of selected objects */
		element_point_ranges_selection=CREATE(Element_point_ranges_selection)();
		element_selection=CREATE(FE_element_selection)();
		node_selection=CREATE(FE_node_selection)();
		data_selection=CREATE(FE_node_selection)();
		/* interactive_tool manager */
		interactive_tool_manager=CREATE(MANAGER(Interactive_tool))();
		fe_time=ACCESS(FE_time)(CREATE(FE_time)());
		scene_manager=CREATE_MANAGER(Scene)();
		if (light_model_manager=CREATE_MANAGER(Light_model)())
		{
			if (default_light_model=CREATE(Light_model)("default"))
			{
				ambient_colour.red=0.2;
				ambient_colour.green=0.2;
				ambient_colour.blue=0.2;
				Light_model_set_ambient(default_light_model,&ambient_colour);
				Light_model_set_side_mode(default_light_model,LIGHT_MODEL_TWO_SIDED);		
				/*???DB.  Include default as part of manager ? */
				ACCESS(Light_model)(default_light_model);		
				if (!ADD_OBJECT_TO_MANAGER(Light_model)(
					default_light_model,light_model_manager))
				{
					DEACCESS(Light_model)(&(default_light_model));
				}			
			}
		}
		if (light_manager=CREATE_MANAGER(Light)())
		{
			if (default_light=CREATE(Light)("default"))
			{
				set_Light_type(default_light,INFINITE_LIGHT);
				default_colour.red=1.0;
				default_colour.green=1.0;
				default_colour.blue=1.0;		
				set_Light_colour(default_light,&default_colour); 				
				set_Light_direction(default_light,default_light_direction);			
				/*???DB.  Include default as part of manager ? */
				ACCESS(Light)(default_light);
				if (!ADD_OBJECT_TO_MANAGER(Light)(default_light,light_manager))
				{
					DEACCESS(Light)(&(default_light));
				}
			}
		}
		spectrum_manager=CREATE_MANAGER(Spectrum)();
		if (graphical_material_manager=CREATE_MANAGER(Graphical_material)())
		{
			struct Colour colour;

			if (default_graphical_material=
				CREATE(Graphical_material)("default"))
			{
				/* ACCESS so can never be destroyed */
				ACCESS(Graphical_material)(default_graphical_material);
				if (!ADD_OBJECT_TO_MANAGER(Graphical_material)(
					default_graphical_material,graphical_material_manager))
				{
					DEACCESS(Graphical_material)(&(default_graphical_material));
				}
			}
			/* create material "default_selected" to be bright red for highlighting
				 selected graphics */
			if (default_selected_material=CREATE(Graphical_material)(
				"default_selected"))
			{
				colour.red=1.0;
				colour.green=0.0;
				colour.blue=0.0;
				Graphical_material_set_ambient(default_selected_material,&colour);
				Graphical_material_set_diffuse(default_selected_material,&colour);
				/* ACCESS so can never be destroyed */
				ACCESS(Graphical_material)(default_selected_material);
				if (!ADD_OBJECT_TO_MANAGER(Graphical_material)(
					default_selected_material,graphical_material_manager))
				{
					DEACCESS(Graphical_material)(&default_selected_material);
				}
			}			
			/* create material "electrode_selected" to be bright white for
				 highlighting electrode graphics */
			if (electrode_selected_material=CREATE(Graphical_material)(
				"electrode_selected"))
			{
				colour.red=1.0;
				colour.green=1.0;
				colour.blue=1.0;
				Graphical_material_set_ambient(electrode_selected_material,&colour);
				Graphical_material_set_diffuse(electrode_selected_material,&colour);
				/* ACCESS so can never be destroyed */
				ACCESS(Graphical_material)(electrode_selected_material);
				if (!ADD_OBJECT_TO_MANAGER(Graphical_material)(
					electrode_selected_material,graphical_material_manager))
				{
					DEACCESS(Graphical_material)(&electrode_selected_material);
				}
			}						
		}
		data_manager=CREATE_MANAGER(FE_node)();
		if ((computed_field_package=CREATE(Computed_field_package)(
			fe_field_manager,element_manager))&&
		(computed_field_manager=Computed_field_package_get_computed_field_manager(
			computed_field_package)))
		{
			if (!((computed_field=CREATE(Computed_field)("cmiss_number"))&&
				Computed_field_set_coordinate_system(computed_field,
					&rect_coord_system)&&
				Computed_field_set_type_cmiss_number(computed_field)&&
				Computed_field_set_read_only(computed_field)&&
				ADD_OBJECT_TO_MANAGER(Computed_field)(computed_field,
					computed_field_manager)))
			{
				DESTROY(Computed_field)(&computed_field);
			}
			if (!((computed_field=CREATE(Computed_field)("xi"))&&
				Computed_field_set_coordinate_system(computed_field,
					&rect_coord_system)&&
				Computed_field_set_type_xi_coordinates(computed_field)&&
				Computed_field_set_read_only(computed_field)&&
				ADD_OBJECT_TO_MANAGER(Computed_field)(computed_field,
					computed_field_manager)))
			{
				DESTROY(Computed_field)(&computed_field);
			}
			/* Add Computed_fields to the Computed_field_package */			
			if (computed_field_package)
			{
				Computed_field_register_types_coordinate(
					computed_field_package);
				Computed_field_register_types_vector_operations(
					computed_field_package);
				Computed_field_register_types_component_operations(
					computed_field_package);
				if (fe_field_manager)
				{
						Computed_field_register_types_finite_element(
							computed_field_package,
							fe_field_manager, fe_time);
				}
			}
		}
		glyph_list=make_standard_glyphs();
		node_tool=CREATE(Node_tool)(interactive_tool_manager,
			node_manager,/*use_data*/0,node_group_manager,element_manager,
			node_selection,computed_field_package,default_graphical_material,
			user_interface, time_keeper, execute_command);		
		transform_tool=create_Interactive_tool_transform(user_interface);
		ADD_OBJECT_TO_MANAGER(Interactive_tool)(transform_tool,
			interactive_tool_manager);
		all_FE_element_field_info=CREATE_LIST(FE_element_field_info)();
		/* FE_element_shape manager */
			/*???DB.  To be done */
		all_FE_element_shape=CREATE_LIST(FE_element_shape)();
#endif /* defined (UNEMAP_USE_3D) */
#else /* defined (NOT_ACQUISITION_ONLY) */
		page_window=(struct Page_window *)NULL;
#endif /* defined (NOT_ACQUISITION_ONLY) */
		/* create the main window */
#if defined (NOT_ACQUISITION_ONLY)
		if (system=CREATE(System_window)(
			User_interface_get_application_shell(user_interface),
			unemap_end_application_loop,(void *)user_interface,
#if defined (UNEMAP_USE_3D)
			element_point_ranges_selection,element_selection,
			fe_field_manager,node_selection,data_selection,
			fe_time,fe_basis_manager,
			element_manager,data_manager,node_manager,
			element_group_manager,data_group_manager,node_group_manager,
			texture_manager,interactive_tool_manager,scene_manager,
			light_model_manager,light_manager,spectrum_manager,
			graphical_material_manager,glyph_list,
			default_graphical_material,computed_field_package,default_light,
			default_light_model,
#endif /* defined (UNEMAP_USE_3D) */
			time_keeper, user_interface))
#else /* defined (NOT_ACQUISITION_ONLY) */
		if (open_Page_window(&page_window,
			unemap_end_application_loop,(void *)user_interface,
			(struct Mapping_window **)NULL,&acquisition_rig,
#if defined (MOTIF)
			user_settings.identifying_colour,
			User_interface_get_screen_width(user_interface),
			User_interface_get_screen_height(user_interface),
#endif /* defined (MOTIF) */
#if defined (WIN32_SYSTEM)
#if defined (MIRADA)
			device_driver,
#endif /* defined (MIRADA) */
#endif /* defined (WIN32_SYSTEM) */
			5,".sig",user_interface))
#endif /* defined (NOT_ACQUISITION_ONLY) */
		{
			/* create the Unemap_command_data */
			unemap_command_data=CREATE(Unemap_command_data)(
				event_dispatcher,
				execute_command,
				user_interface,
#if defined (NOT_ACQUISITION_ONLY)
#if defined (UNEMAP_USE_3D)
#if defined (MOTIF)
				node_tool,
				transform_tool,
#endif /* defined (MOTIF) */
				glyph_list,
				fe_time,
				computed_field_package,
				fe_basis_manager,
				element_manager,
				fe_field_manager,
				data_manager,
				node_manager,
				graphical_material_manager,
				default_graphical_material,
				element_group_manager,
				data_group_manager,
				node_group_manager,
				interactive_tool_manager,
				light_manager,
				default_light,
				light_model_manager,
				default_light_model,
				texture_manager,
				scene_manager,
				spectrum_manager,
				element_point_ranges_selection,
				element_selection,
				data_selection,
				node_selection,
#endif /* defined (UNEMAP_USE_3D) */
				system,
				time_keeper
#else /* defined (NOT_ACQUISITION_ONLY) */
				page_window
#endif /* defined (NOT_ACQUISITION_ONLY) */
				);
			Execute_command_set_command_function(execute_command,
				unemap_execute_command,unemap_command_data);
			/* error handling */
			switch (signal_code)
			{
#if !defined (WIN32_SYSTEM)
				/*???DB.  SIGBUS is not POSIX */
				case SIGBUS:
				{
					printf("Bus error occured\n");
					display_message(ERROR_MESSAGE,"Bus error occured");
				} break;
#endif /* !defined (WIN32_SYSTEM) */
				case SIGFPE:
				{
					printf("Floating point exception occured\n");
					display_message(ERROR_MESSAGE,
						"Floating point exception occured");
				} break;
				case SIGILL:
				{
					printf("Illegal instruction occured\n");
					display_message(ERROR_MESSAGE,
						"Illegal instruction occured");
				} break;
				case SIGSEGV:
				{
					printf("Invalid memory reference occured\n");
					display_message(ERROR_MESSAGE,
						"Invalid memory reference occured");
				} break;
			}
			/* event loop */
			return_code=Event_dispatcher_main_loop(event_dispatcher);
			/* clean up application memory */
			DESTROY(Unemap_command_data)(&unemap_command_data);
#if defined (NOT_ACQUISITION_ONLY)
			DESTROY(System_window)(&system);
#else /* defined (NOT_ACQUISITION_ONLY) */
			destroy_Page_window(&page_window);
#endif /* defined (NOT_ACQUISITION_ONLY) */

		}
		else
		{
			display_message(ERROR_MESSAGE, "Unable to create system window");
			return_code = 0;
		}
		/* clean up application memory */
#if defined (NOT_ACQUISITION_ONLY)
#if defined (UNEMAP_USE_3D )
		DESTROY(MANAGER(Scene))(&scene_manager);
		/* destroy Interactive_tools and manager */
		DESTROY(Node_tool)(&node_tool);		
		/*???DB.  Node_tool uses computed fields, so can't destroy after
			destroying computed fields */
		/*???DB.  Should destroy in the reverse of creation order */
		DESTROY(MANAGER(Interactive_tool))(&interactive_tool_manager);
		DESTROY(Computed_field_package)(&computed_field_package);
		DESTROY(FE_node_selection)(&data_selection);
		DESTROY(FE_node_selection)(&node_selection);
		DESTROY(FE_element_selection)(&element_selection);
		DESTROY(Element_point_ranges_selection)(&element_point_ranges_selection);
		DESTROY(LIST(GT_object))(&glyph_list);
		DESTROY(MANAGER(FE_field))(&fe_field_manager);
		DESTROY(MANAGER(GROUP(FE_node)))(&node_group_manager);
		DESTROY(MANAGER(FE_node))(&node_manager);
		DESTROY(MANAGER(GROUP(FE_node)))(&data_group_manager);
		DESTROY(MANAGER(FE_node))(&data_manager);
		DESTROY(MANAGER(GROUP(FE_element)))(&element_group_manager);
		DESTROY(MANAGER(FE_element))(&element_manager);	
		DESTROY(MANAGER(FE_basis))(&fe_basis_manager);
		DESTROY_LIST(FE_element_field_info)(&all_FE_element_field_info);
		DESTROY_LIST(FE_element_shape)(&all_FE_element_shape);
		DESTROY(MANAGER(Spectrum))(&spectrum_manager);
		DEACCESS(Graphical_material)(&default_graphical_material);			
		DEACCESS(Graphical_material)(&default_selected_material);
		DEACCESS(Graphical_material)(&electrode_selected_material);			
		DESTROY(MANAGER(Graphical_material))(&graphical_material_manager);
		DESTROY(MANAGER(Texture))(&texture_manager);
		DEACCESS(FE_time)(&fe_time);
		DEACCESS(Light_model)(&default_light_model);
		DESTROY(MANAGER(Light_model))(&light_model_manager);
		DEACCESS(Light)(&default_light);
		DESTROY(MANAGER(Light))(&light_manager);
#endif /* defined (UNEMAP_USE_3D) */
		DEACCESS(Time_keeper)(&time_keeper);
#else /* defined (NOT_ACQUISITION_ONLY) */
#endif /* defined (NOT_ACQUISITION_ONLY) */
		DESTROY(Execute_command)(&execute_command);
#if defined (WIN32_SYSTEM)
#if defined (MIRADA)
		if (INVALID_HANDLE_VALUE!=device_driver)
		{
			CloseHandle(device_driver);
		}
#endif /* defined (MIRADA) */
#endif /* defined (WIN32_SYSTEM) */
		/* close the user interface */
		DESTROY(User_interface)(&user_interface);
		DESTROY(Event_dispatcher)(&event_dispatcher);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Could not open user interface");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* main */
