/*******************************************************************************
FILE : unemap.c

LAST MODIFIED : 28 January 2002

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
#include "unemap/unemap_package.h"
#include "unemap/system_window.h"
#else /* defined (NOT_ACQUISITION_ONLY) */
#include "unemap/page_window.h"
#include "unemap/rig.h"
#include "unemap/rig_node.h"
#include "unemap/unemap_hardware.h"
#if defined (WINDOWS)
#if defined (MIRADA)
#include "unemap/vunemapd.h"
#endif /* defined (MIRADA) */
#endif /* defined (WINDOWS) */
#endif /* defined (NOT_ACQUISITION_ONLY) */
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
#if defined (WIN32)
				if ('\\'!=hardware_file_name[strlen(hardware_file_name)-1])
				{
					strcat(hardware_file_name,"\\");
				}
#else /* defined (WIN32) */
				if ('/'!=hardware_file_name[strlen(hardware_file_name)-1])
				{
					strcat(hardware_file_name,"/");
				}
#endif /* defined (WIN32) */
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

#if defined (MOTIF)
static void exit_unemap(Widget widget,XtPointer user_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 March 2001

DESCRIPTION :
Exits unemap
==============================================================================*/
{
	struct Page_window *page_window;
#if defined (NOT_ACQUISITION_ONLY)
	struct System_window *system_window;
#endif /* defined (NOT_ACQUISITION_ONLY) */

	ENTER(exit_unemap);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
#if defined (NOT_ACQUISITION_ONLY)
	if (system_window=(struct System_window *)user_data)
	{
		struct User_interface *user_interface;

		if(user_interface=system_window->user_interface)
		{			
			User_interface_end_application_loop(user_interface);
		}
		if(page_window=(system_window->acquisition).window)
		{
			destroy_Page_window(&page_window);
		}
	}
#else /* defined (NOT_ACQUISITION_ONLY) */
	if (page_window=(struct Page_window *)user_data)	
	{		 
		destroy_Page_window(&page_window);
	}
#endif /* defined (NOT_ACQUISITION_ONLY) */
	LEAVE;
} /* exit_unemap */
#endif /* defined (MOTIF) */

/*
Main program
------------
*/
#if defined (MOTIF)
int main(int argc,char *argv[])
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
int WINAPI WinMain(HINSTANCE current_instance,HINSTANCE previous_instance,
	LPSTR command_line,int initial_main_window_state)
	/* using WinMain as the entry point tells Windows that it is a gui and to use
		the graphics device interface functions for I/O */
	/*???DB.  WINDOWS a zero return code if WinMain does get into the message
		loop.  Other application interfaces may expect something else.  Should this
		failure code be #define'd ? */
	/*???DB. Win32 SDK says that don't have to call it WinMain */
#endif /* defined (WINDOWS) */
/*******************************************************************************
LAST MODIFIED : 19 Julyy 2000

DESCRIPTION :
Main program for unemap
==============================================================================*/
{
#if defined (MOTIF)
	Dimension window_height,window_width;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
#if defined (MIRADA)
	HANDLE device_driver;
#endif /* defined (MIRADA) */
#endif /* defined (WINDOWS) */
	int return_code;
#if defined (NOT_ACQUISITION_ONLY)
	struct Unemap_package *unemap_package;
#if defined (UNEMAP_USE_3D)
	float default_light_direction[3]={0.0,-0.5,-1.0};	
	struct Colour ambient_colour,default_colour;	
	struct Computed_field *computed_field=(struct Computed_field *)NULL;
	struct Computed_field_package *computed_field_package=
		(struct Computed_field_package *)NULL;	
	struct Coordinate_system rect_coord_system;
	struct Element_point_ranges_selection *element_point_ranges_selection=
		(struct Element_point_ranges_selection *)NULL;
	struct FE_element_selection *element_selection=(struct FE_element_selection *)NULL;
	struct FE_field *map_fit_field=(struct FE_field *)NULL;
	struct FE_node_selection *node_selection=(struct FE_node_selection *)NULL;
	struct FE_node_selection *data_selection=(struct FE_node_selection *)NULL;
	struct FE_time *fe_time = (struct FE_time *)NULL;
	struct Graphical_material *default_graphical_material=(struct Graphical_material *)NULL;
	struct Graphical_material *default_selected_material=(struct Graphical_material *)NULL;	
	struct Graphical_material *electrode_selected_material=(struct Graphical_material *)NULL;
	struct GROUP(FE_element) *torso_element_group=(struct GROUP(FE_element) *)NULL;	
	struct GT_object *glyph=(struct GT_object *)NULL;
	struct Light *default_light=(struct Light *)NULL;
	struct Light_model *default_light_model=(struct Light_model *)NULL;
	struct LIST(GT_object) *glyph_list=(struct LIST(GT_object) *)NULL;
	struct MANAGER(Computed_field) *computed_field_manager=
		(struct MANAGER(Computed_field) *)NULL;
	struct MANAGER(FE_basis) *fe_basis_manager=
		(struct MANAGER(FE_basis) *)NULL;
	struct MANAGER(FE_field) *fe_field_manager=(struct MANAGER(FE_field) *)NULL;
	struct MANAGER(FE_element) *element_manager=(struct MANAGER(FE_element) *)NULL;
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
	struct Standard_torso_defaults
	{
		char *standard_torso_file;
	};
#define XmNstandardTorso "standardTorso"
#define XmCStandardTorso "StandardTorso"
	struct Standard_torso_defaults standard_torso_defaults;
	char *default_torso_group_name=(char *)NULL;
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
#if defined (WINDOWS)
#if defined (MIRADA)
	short int *mirada_buffer;
	unsigned char *bus,*device_function;
	unsigned long number_of_cards;
#endif /* defined (MIRADA) */
#endif /* defined (WINDOWS) */
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

#if defined (UNEMAP_USE_3D)
	static XtResource standard_torso_resources[]=
	{
		XmNstandardTorso,
		XmCStandardTorso,
		XmRString,
		sizeof(char *),
		XtOffsetOf(struct Standard_torso_defaults,standard_torso_file),
		XmRString,
		""
	};
#endif/* defined (UNEMAP_USE_3D) */

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
	User_settings user_settings;
#endif /* defined (MOTIF) */

#if defined (MOTIF)
	ENTER(main);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	ENTER(WinMain);
#endif /* defined (WINDOWS) */
	/* display the version */
#if defined (MOTIF)
	display_message(INFORMATION_MESSAGE, VERSION "\n");
#endif /* defined (MOTIF) */
	if ((event_dispatcher = CREATE(Event_dispatcher)())
		&& (user_interface = CREATE(User_interface)(&argc, argv, event_dispatcher,
		"Unemap", "unemap")))
	{
		/* set up messages */
		set_display_message_function(ERROR_MESSAGE,display_error_message,
			&user_interface);
		/*if using nodes, print information and warning messages to stdout*/
#if defined(UNEMAP_USE_3D)
		set_display_message_function(INFORMATION_MESSAGE,
			display_information_message,(void *)NULL);
		set_display_message_function(WARNING_MESSAGE,display_warning_message,
			(void *)NULL);
#else
		set_display_message_function(INFORMATION_MESSAGE,
			display_information_message,(void *)NULL);
		set_display_message_function(WARNING_MESSAGE,display_warning_message,
			&user_interface);
#endif /* defined(UNEMAP_USE_3D) */
		/* used to output information and warnings to windows*/
#if defined(OLD_CODE)
		set_display_message_function(INFORMATION_MESSAGE,
			display_information_message,&user_interface);
		set_display_message_function(WARNING_MESSAGE,display_warning_message,
			&user_interface);
#endif
		/* retrieve application specific constants */
#if defined (MOTIF)
		XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),
			&user_settings,resources,XtNumber(resources),NULL);
			/*???DB.  User settings should be divided among tools */
#endif /* defined (MOTIF) */
#if !defined (NOT_ACQUISITION_ONLY)
#if defined (OLD_CODE)
		number_of_channels=64;
		number_of_samples=5000;
		sampling_frequency=(float)5000.;
#if defined (WINDOWS)
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
#endif /* defined (WINDOWS) */
#endif /* defined (OLD_CODE) */
		/* set up the acquisition rig */
		acquisition_rig=(struct Rig *)NULL;
#if defined (OLD_CODE)
#if defined (OLD_CODE)
		if (acquisition_rig_open_data=create_File_open_data(".cnfg",REGULAR,
			read_configuration_file,(void *)&acquisition_rig,0,&user_interface))
		{
			open_file_and_read(
#if defined (MOTIF)
				(Widget)NULL,(XtPointer)acquisition_rig_open_data,(XtPointer)NULL
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				acquisition_rig_open_data
#endif /* defined (WINDOWS) */
				);
			destroy_File_open_data(&acquisition_rig_open_data);
		}
#endif /* defined (OLD_CODE) */
		if (acquisition_rig_filename=confirmation_get_read_filename(".cnfg",
			&user_interface))
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

		/* create the main window */
#if defined (NOT_ACQUISITION_ONLY)
		time_keeper=ACCESS(Time_keeper)(
			CREATE(Time_keeper)("default",user_interface));
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
		element_point_ranges_selection=
			CREATE(Element_point_ranges_selection)();
		element_selection=CREATE(FE_element_selection)();
		node_selection=CREATE(FE_node_selection)();
		data_selection=CREATE(FE_node_selection)();

		/* interactive_tool manager */
		interactive_tool_manager=CREATE(MANAGER(Interactive_tool))();

		fe_time = ACCESS(FE_time)(CREATE(FE_time)());

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
				if (!ADD_OBJECT_TO_MANAGER(Graphical_material)(default_selected_material,
					graphical_material_manager))
				{
					DEACCESS(Graphical_material)(&default_selected_material);
				}
			}			
			/* create material "electrode_selected" to be bright white for highlighting
				 electrode graphics */
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
				if (!ADD_OBJECT_TO_MANAGER(Graphical_material)(electrode_selected_material,
					graphical_material_manager))
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
		if (glyph_list=CREATE(LIST(GT_object))())
		{
			/* add standard glyphs */
			if (glyph=make_glyph_arrow_line("arrow_line",0.25,0.125))
			{
				ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
			}
			if (glyph=make_glyph_arrow_solid("arrow_solid",12,2./3.,1./6.))
			{
				ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
			}
			if (glyph=make_glyph_axes("axes",0.1,0.025,0.1))
			{
				ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
			}
			if (glyph=make_glyph_cone("cone",12))
			{
				ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
			}
			if (glyph=make_glyph_cross("cross"))
			{
				ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
			}
			if (glyph=make_glyph_cylinder("cylinder6",6))
			{
				ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
			}
			if (glyph=make_glyph_cylinder("cylinder",12))
			{
				ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
			}
			if (glyph=make_glyph_cylinder("cylinder_hires",48))
			{
				ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
			}
			if (glyph=make_glyph_sphere("diamond",4,2))
			{
				ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
			}
			if (glyph=make_glyph_line("line"))
			{
				ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
			}
			if (glyph=make_glyph_point("point",g_POINT_MARKER,0))
			{
				ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
			}
			if (glyph=make_glyph_sheet("sheet"))
			{
				ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
			}
			if (glyph=make_glyph_sphere("sphere",12,6))
			{
				ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
			}
			if (glyph=make_glyph_sphere("sphere_hires",48,24))
			{
				ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
			}
		}
		node_tool=CREATE(Node_tool)(interactive_tool_manager,
			node_manager,/*use_data*/0,node_group_manager,element_manager,
			node_selection,computed_field_package,default_graphical_material,
			user_interface, time_keeper);		
		transform_tool=create_Interactive_tool_transform(user_interface);
		ADD_OBJECT_TO_MANAGER(Interactive_tool)(transform_tool,interactive_tool_manager);
		all_FE_element_field_info=CREATE_LIST(FE_element_field_info)();
		/* FE_element_shape manager */
		/*???DB.  To be done */
		all_FE_element_shape=CREATE_LIST(FE_element_shape)();
		unemap_package=CREATE(Unemap_package)(fe_field_manager, fe_time,
			element_group_manager,node_manager,data_manager,data_group_manager,
			node_group_manager,fe_basis_manager,element_manager,computed_field_manager,
			interactive_tool_manager,node_selection);	
#else /* defined (UNEMAP_USE_3D) */
		unemap_package=(struct Unemap_package *)NULL;
#endif /* defined (UNEMAP_USE_NODES) */
		if (system=create_System_window(User_interface_get_application_shell(user_interface),
			exit_unemap,time_keeper,user_interface,unemap_package
#if defined (UNEMAP_USE_3D)
			,element_point_ranges_selection,element_selection,node_selection,
			data_selection,texture_manager,interactive_tool_manager,scene_manager,
			light_model_manager,light_manager,spectrum_manager,
			graphical_material_manager,data_manager,glyph_list,
			default_graphical_material,computed_field_package,default_light,
			default_light_model
#endif /* defined (UNEMAP_USE_3D) */
			))
#else /* defined (NOT_ACQUISITION_ONLY) */
		page_window=(struct Page_window *)NULL;
		if (open_Page_window(&page_window,
			(struct Mapping_window **)NULL,&acquisition_rig,
#if defined (MOTIF)
			user_settings.identifying_colour,User_interface_get_screen_width(user_interface),
			User_interface_get_screen_height(user_interface),
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
#if defined (MIRADA)
			device_driver,
#endif /* defined (MIRADA) */
#endif /* defined (WINDOWS) */
			5,".sig",user_interface))
#endif /* defined (NOT_ACQUISITION_ONLY) */
		{
#if defined (NOT_ACQUISITION_ONLY)
#if defined (MOTIF)
			create_Shell_list_item(&(system->window_shell),user_interface);
#endif /* defined (MOTIF) */
#if defined (MOTIF)		
			XtAddCallback(system->window_shell,XmNdestroyCallback,close_emap,
				(XtPointer)system);
			/* manage the system window */
			XtManageChild(system->window);
			/* realize the system window shell */
			XtRealizeWidget(system->window_shell);
			/* determine placement */
			XtVaGetValues(system->window_shell,
				XmNwidth,&window_width,
				XmNheight,&window_height,
				NULL);
			/* do this to allow backward compatibility but still allow the resources
				to be set */
			/* these defaults match with the default resources above */
			system_window_data.x = -1;
			system_window_data.y = -1;
#if defined (UNEMAP_USE_3D)
			/* create and store the map fit field  */
			map_fit_field=create_mapping_type_fe_field("fit",fe_field_manager,
				fe_time);
			set_unemap_package_map_fit_field(unemap_package,map_fit_field);
			/* get the location of the default_torso file from Xresoures*/
			standard_torso_defaults.standard_torso_file= "";			
			XtVaGetApplicationResources(system->window_shell,
				&standard_torso_defaults,standard_torso_resources,
				XtNumber(standard_torso_resources),NULL);
			/* do nothing if no default torso file specified */
			if(strcmp(standard_torso_defaults.standard_torso_file,""))
			{									
				/* read in the default torso node and element groups */
				if(read_FE_node_and_elem_groups_and_return_name_given_file_name(
					standard_torso_defaults.standard_torso_file,fe_field_manager,
					fe_time, node_manager,element_manager,node_group_manager,
					data_group_manager,element_group_manager,fe_basis_manager,
					&default_torso_group_name))
				{
					/* offset default torso  node and element groups */
					offset_FE_node_and_element_identifiers_in_group(default_torso_group_name,
						(INT_MAX/2),node_manager,element_manager,node_group_manager,
						element_group_manager);									
					/*put in name unemap_package*/
					set_unemap_package_default_torso_name(unemap_package,default_torso_group_name);
					/* define the fit field on  the defaut torso*/
					torso_element_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),name)
						(default_torso_group_name,element_group_manager);
					define_fit_field_at_quad_elements_and_nodes(torso_element_group,
						map_fit_field,fe_basis_manager,element_manager,node_manager);
					/* add cylindrical field infor for texture mapping to defaut torso*/
					add_cylindrical_info_to_cartesian_torso(default_torso_group_name,
						unemap_package);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Can't find standardTorso specified in Cmgui/Unemap file");
				}
			}
#endif /* defined (UNEMAP_USE_3D) */
			XtVaGetApplicationResources(system->window_shell,
				&system_window_data,System_window_resources,
				XtNumber(System_window_resources),NULL);
			if (-1==system_window_data.x)
			{
				system_window_data.x = ((User_interface_get_screen_width(user_interface))
					-window_width)/2;
			}
			if (-1==system_window_data.y)
			{
				system_window_data.y = ((User_interface_get_screen_height(user_interface))
					-window_height)/2;
			}
			XtVaSetValues(system->window_shell,
				XmNx,system_window_data.x,
				XmNy,system_window_data.y,
				XmNmappedWhenManaged,True,
				NULL);
#endif /* defined (MOTIF) */
#else /* defined (NOT_ACQUISITION_ONLY) */
#if defined (MOTIF)
			XtAddCallback(get_page_window_close_button(page_window),
				XmNactivateCallback,exit_unemap,(XtPointer)page_window);
#endif /* defined (MOTIF) */
#if defined (OLD_CODE)
#if defined (MOTIF)
			create_Shell_list_item(&(acquisition.window_shell));
#endif /* defined (MOTIF) */
#endif /* defined (OLD_CODE) */
#endif /* defined (NOT_ACQUISITION_ONLY) */
/*???debug */
/*			START_ERROR_HANDLING;*/
#if defined (OLD_CODE)
/*???DB.  create_System_window should do its own popping */
			/* pop up the system window shell */
			XtPopup(system->window_shell,XtGrabNone);
#endif /* defined (OLD_CODE) */
			switch (signal_code)
			{
#if !defined (WINDOWS)
				/*???DB.  SIGBUS is not POSIX */
				case SIGBUS:
				{
					printf("Bus error occured\n");
					display_message(ERROR_MESSAGE,"Bus error occured");
				} break;
#endif /* !defined (WINDOWS) */
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
			/* user interface loop */
			return_code=Event_dispatcher_main_loop(event_dispatcher);
#if defined (NOT_ACQUISITION_ONLY)
#if defined (UNEMAP_USE_3D )
			DESTROY(Unemap_package)(&unemap_package);	
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
#endif /* defined (NOT_ACQUISITION_ONLY) */
/*???debug */
/*			END_ERROR_HANDLING;*/
			/* free application memory */
			/* close the user interface */
			DESTROY(User_interface)(&user_interface);
				/*???DB.  Should this actually be inside the application and be
					used to set a flag that terminates the main loop ? */
		}
		else
		{
			display_message(ERROR_MESSAGE,"Unable to create system window");
			return_code=0;
		}
#if defined (WINDOWS)
#if defined (MIRADA)
		if (INVALID_HANDLE_VALUE!=device_driver)
		{
			CloseHandle(device_driver);
		}
#endif /* defined (MIRADA) */
#endif /* defined (WINDOWS) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"Could not open user interface");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* main */
