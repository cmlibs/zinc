/*******************************************************************************
FILE : input_module.c

LAST MODIFIED : 16 May 1998

DESCRIPTION :
Contains all the code needed to handle input from any of a number of devices,
and sets up callbacks for whatever users are interested.
The callbacks return the relative change since the last call.

NOTE: (If using GL)
The wise guys at GL have made it so that the first time a GL widget is
created, it calls an initialisation routine.  This init routine checks for
devices on the serial port, and thereby undoes all the setup and 'disconnects'
the polhemus.  To rectify the situation, ensure that you create a
GL window first & then destroy it.

NOTE :
Callbacks must have prototype

int *MyCallback(void *identifier,input_module_message message);
(this is the type input_module_callback_proc)

Register the callback for a particular device, and give it a unique identifier -
probably a pointer to your own data area.  This identifier will be passed to the
callback allowing knowledge of 'who you are'.  The return code is not used at
the moment.

NOTE:
The polhemus direction variables are an exception to the 'relative' rule, and
are passed as absolute values.  To check for this, test message->source for
IM_SOURCE_POLHEMUS and then act accordingly.

PROGRAMMERS_NOTE:
Initially, orientation was received from the Polhemus as azimuth, elevation and
roll.  This form is not pleasant to work with however as they are incremental
operations and therefore not commutative.  The system has now been changed over
to using the 9 component direction cosine matrix, thereby increasing the time
to transmit the data, but simplifying the actual operations performed on the
data.
==============================================================================*/
#include <stdio.h>
#include <unistd.h>
#include <math.h>
/* For read(), write(), close() */
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#if !defined (VMS)
#include <termio.h>
#endif
#include <sys/types.h>
#include <X11/Xlib.h>
/*???DB.  Other conditions eg SPACEBALL ? */
/*???GH.  Not yet.... */
#if defined (SGI)
#include <X11/extensions/XI.h>
#include <X11/extensions/XInput.h>
#include <X11/Xutil.h>
#endif
#include "general/debug.h"
#include "io_devices/input_module.h"
#include "user_interface/user_interface.h"
#include "io_devices/haptic_input_module.h"
#include "io_devices/matrix.h"
#include "user_interface/message.h"

#if defined (EXT_INPUT)
/*
Local Constants
---------------
*/
/* Display information as the Polhemus initialises itself */
#define POLHEMUS_DIAG
/* Display information as the Spaceball initialises itself */
#define SPACEBALL_DIAG
/* Display information as the Dial initialises itself */
#define DIALS_DIAG
/* Display information as the Faro initialises itself */
#define FARO_DIAG

/*
Global Variables
----------------
*/
#define INPUT_UPDATE_BUFFER_SIZE (1000)
#define PI (3.14159)

struct Input_module_data_struct *input_module_data = NULL;

int ndevices;
Input_module_callback_info input_module_list[INPUT_MODULE_NUM_DEVICES];
#if defined (SGI)
XDeviceInfoPtr device_list;
#endif
char *input_module_name[INPUT_MODULE_NUM_DEVICES]=
{
	"Spaceball",
	"Polhemus 1",
	"Polhemus 2",
	"Polhemus 3",
	"Polhemus 4",
	"Dials",
	"Haptic",
	"Faro"
};

#if defined (SPACEBALL)
#if defined (IBM)
Atom
#endif
#if defined (SGI)
int
#endif
	spaceball_button_press_event_type,spaceball_button_release_event_type,
	spaceball_motion_event_type;
int spaceball_button_press_event_class,spaceball_button_release_event_class,
	spaceball_motion_event_class;
int spaceball_valid=0;
#if defined (SGI)
XDevice *spaceball_device=(XDevice *)NULL;
XDevice *XOpenDevice(Display *,XID);
XEventClass ListOfSpaceballEventClass[3];
#endif
#endif

#if defined (POLHEMUS)
#define POLHEMUS_INIT_NAME "polhemus.init"
#define POLHEMUS_INIT_HEADER "PolhemusInitialisationFile"
#define POLHEMUS_IDENTIFICATION_SIZE (3+3+3+12+32+2)
#define POLHEMUS_STATION_SIZE (3+4+2)
#define POLHEMUS_GET_RECORD_SIZE (3+2+3*4+9*4+2)
/* this gives us 2 header, button,3xyz,3 x cos,3 y cos,3 z cos,cr/lf */
double global_position[3];/* GMH 6/4/95. these are about to be phased out... */
Gmatrix global_direction;
double polhemus_origin_position[3];
Gmatrix polhemus_origin_direction;
double polhemus_last_position[3];
Gmatrix polhemus_last_direction;
int polhemus_num_stations,polhemus_valid=0,RS232,polhemus_station_present[4],
	polhemus_button_pressed;
static char polhemus_get_record='P';
static char polhemus_identification='S';
static char polhemus_station[]={'l','1','\15'};
	/* '\15'=carriage return */
#endif

#if defined (DIALS)
int dials_motion_event_type,dials_button_press_event_type,
	dials_button_release_event_type;
int dials_motion_event_class,dials_button_press_event_class,
	dials_button_release_event_class;
int dials_valid=0;
int dials_calibrated[6],dials_calibrate_value[6];
XDevice *dials_device=NULL;
XDevice *XOpenDevice(Display *,XID);
XEventClass ListOfDialsEventClass[3];
#endif

#if defined (HAPTIC)
int haptic_valid;
double haptic_position_scale[3], haptic_position_offset[3],
	haptic_position_current[3];
#endif /* defined (HAPTIC) */

#if defined (FARO)
#define FARO_INIT_NAME "faro.init"
#define FARO_INIT_HEADER "FaroInitialisationFile"
#define FARO_INIT_BUFFER_SIZE 200
static int faro_valid, faro_serial_port;
static double faro_angle_current[3];
static double faro_position_current[3];
static char faro_get_record[]="_P\r\n";
static char faro_identification[]="_G\r\n";
static char faro_three_point_calibrate[]="_C\r\n";
static int faro_button_pressed;
#endif /* defined (FARO) */


/*
Module functions
----------------
*/
static int input_module_do_callback(Input_module_message message,int device,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
Calls the callback, given the message.  Searches through a linked list, on the
basis that if only one callback is registered then give it to it, else, give
it to the window with the mouse on it.
==============================================================================*/
{
	Input_module_callback_info temp;
	int return_code,root_x_ret,root_y_ret,win_x_ret,win_y_ret;
	unsigned int mask_ret;
	Window child_ret,root_ret;

	ENTER(input_module_do_callback);
	/* check arguments */
	if (user_interface)
	{
		return_code=1;
		/* is there a callback to call ? */
		if (temp=input_module_list[device])
		{
			while(temp)
			{
				(temp->callback)(temp->identifier,message);
				temp = temp->next;
			}
#if defined (OLD_CODE)
			/* SAB I don't want this behaviour, I want to keep all clients in
				sync. */
			/* are there more than one? */
			if (!temp->next)
			{
				/* no so send the message to it */
				(temp->callback)(temp->identifier,message);
			}
			else
			{
				/* we have to send the message to the one with the mouse */
				do
				{
					if (XQueryPointer(user_interface->display,temp->client_window,
						&root_ret,&child_ret,&root_x_ret,&root_y_ret,&win_x_ret,&win_y_ret,
						&mask_ret))
					{
						if (child_ret!=0)
						{

							(temp->callback)(temp->identifier,message);
						}
					}
					/* point to the next one */
					temp=temp->next;
				}
				while (temp);
			}
#endif /* defined (OLD_CODE) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"input_module_do_callback.  Missing user_interface");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* input_module_do_callback */

#if defined (SPACEBALL)
static int input_module_spaceball_init(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
Sets the spaceball so that any input is recognised
==============================================================================*/
{
	int i,return_code;

	ENTER(input_module_spaceball_init);
	/* check arguments */
	if (user_interface)
	{
#if defined (IBM)
		spaceball_motion_event_type=XInternAtom(display,"SpaceballMotionEventType",
			False);
		spaceball_button_press_event_type=XInternAtom(user_interface->display,
			"SpaceballButtonPressEventType",False);
		spaceball_button_release_event_type=XInternAtom(user_interface->display,
			"SpaceballButtonReleaseEventType",False);
#if defined (SPACEBALL_DIAG)
/*???debug */
printf("input_module_spaceball_init.  Spaceball initialised\n");
#endif
		return_code=1;
#endif
#if defined (SGI)
		/* find the spaceball */
		for (i=0;i<ndevices;i++)
		{
			if (0==strcmp(device_list[i].name,"spaceball"))
			{
				/* open the spaceball */
				spaceball_device=XOpenDevice(user_interface->display,device_list[i].id);
			};
		};
		if (spaceball_device)
		{
			DeviceMotionNotify(spaceball_device,spaceball_motion_event_type,
				spaceball_motion_event_class);
			ListOfSpaceballEventClass[0]=spaceball_motion_event_class;
			DeviceButtonPress(spaceball_device,spaceball_button_press_event_type,
				spaceball_button_press_event_class);
			ListOfSpaceballEventClass[1]=spaceball_button_press_event_class;
			DeviceButtonRelease(spaceball_device,spaceball_button_release_event_type,
				spaceball_button_release_event_class);
			ListOfSpaceballEventClass[2]=spaceball_button_release_event_class;
#if defined (SPACEBALL_DIAG)
/*???debug */
printf("input_module_spaceball_init.  Spaceball initialised\n");
#endif
			return_code=1;
		}
#endif
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"input_module_spaceball_init.  Missing user_interface");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* input_module_spaceball_init */
#endif

#if defined (SPACEBALL)
static int input_module_spaceball_close(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
Closes the spaceball.
==============================================================================*/
{
	int return_code;

	ENTER(input_module_spaceball_close);
	if (user_interface)
	{
#if defined (SGI)
		XCloseDevice(user_interface->display,spaceball_device);
#endif
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"input_module_spaceball_init.  Missing user_interface");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* input_module_spaceball_close */
#endif

#if defined (POLHEMUS)
static int input_module_polhemus_init(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
Initialises the polhemus via the RS232C, and gets initial data.
==============================================================================*/
{
	char polhemus_buf[100],temp_str[100];
	FILE *polhemus_data;
	int baudrate,i,num_read,return_code,serialport;
	struct termio Setup;

	ENTER(input_module_polhemus_init);
	return_code=0;
	for (i=0;i<3;i++)
	{
		global_position[i]=0.0;
		polhemus_origin_position[i]=0.0;
		polhemus_last_position[i]=0.0;
	}
	polhemus_button_pressed=0;
	matrix_I(&global_direction);
	matrix_I(&polhemus_last_direction);
	matrix_I(&polhemus_origin_direction);
	/* open an initialisation file to get initial conditions */
	if (polhemus_data=fopen(POLHEMUS_INIT_NAME,"r"))
	{
		/* is it a genuine file */
		fscanf(polhemus_data,"%s",temp_str);
		if (!strcmp(POLHEMUS_INIT_HEADER,temp_str))
		{
			/* read the com port number and baud rate */
			fscanf(polhemus_data,"%s %i",temp_str,&serialport);
			fscanf(polhemus_data,"%s %i",temp_str,&baudrate);
			/* initialise the serial port */
			sprintf(temp_str,"/dev/ttyd%i",serialport);
			RS232=open(temp_str,O_RDWR|O_NOCTTY,0666);
			if (RS232!=-1)
			{
				/* read in the current settings for the port so that we dont stuff up
					anything impt */
				if (1!=ioctl(RS232,TCGETA,&Setup))
				{
					Setup.c_iflag=IGNBRK;
					Setup.c_oflag=0;
					switch (baudrate)
					{
						case 9600:
						{
							Setup.c_cflag=B9600|CS8|CREAD|CLOCAL;
						} break;
						case 19200:
						{
							Setup.c_cflag=B19200|CS8|CREAD|CLOCAL;
						} break;
						case 38400:
						{
							Setup.c_cflag=B38400|CS8|CREAD|CLOCAL;
						} break;
						case 57600:
						{
							Setup.c_cflag=B57600|CS8|CREAD|CLOCAL;
						} break;
						case 115200:
						{
							Setup.c_cflag=B115200|CS8|CREAD|CLOCAL;
						} break;
						default:
						{
							Setup.c_cflag=B9600|CS8|CREAD|CLOCAL;
							display_message(WARNING_MESSAGE,
							"input_module_polhemus_init.  Unknown baud rate - set to 9600.");
						} break;
					}
					Setup.c_lflag=0;
					Setup.c_cc[VINTR]=0;
					Setup.c_cc[VQUIT]=0;
					Setup.c_cc[VERASE]=0;
					Setup.c_cc[VKILL]=0;
					Setup.c_cc[VEOF]=0;
					Setup.c_cc[VMIN]=0;
					Setup.c_cc[VEOL]=0;
					/* max. timeout: 1.0 sec */
					Setup.c_cc[VTIME]=10;
					Setup.c_cc[VEOL2]=0;
					if (-1!=ioctl(RS232,TCSETA,&Setup))
					{
						/* see if it is a polhemus */
						num_read=write(RS232,&polhemus_identification,1);
						if (-1!=num_read)
						{
							/* read the first byte to see if the polhemus is there or not */
							num_read=read(RS232,&polhemus_buf[0],1);
							if (-1!=num_read)
							{
								if (num_read==1)
								{ /* we did get a response */
#if defined (POLHEMUS_DIAG)
/*???debug */
printf("input_module_polhemus_init. Received response\n");
#endif
									/* now read the rest */
									num_read=1;
									for (i=1;(i<POLHEMUS_IDENTIFICATION_SIZE) && (num_read);i++)
									{
										num_read=read(RS232,&polhemus_buf[i],1);
									}
									if ((polhemus_buf[0]=='2') &&
										(polhemus_buf[2]=='S'))
									{
										polhemus_buf[53]='\0';
#if defined (POLHEMUS_DIAG)
/*???debug */
printf("input_module_polhemus_init.  Polhemus ID: %s\n",&polhemus_buf[21]);
#endif
										/* check the file for initialisation data */
										if (!feof(polhemus_data))
										{
											int temp_length;
											do
											{
												fscanf(polhemus_data,"%s\n",polhemus_buf);
												temp_length=strlen(polhemus_buf);
												/* add a carriage return */
												polhemus_buf[temp_length++]='\15';
												write(RS232,polhemus_buf,temp_length);
#if defined (POLHEMUS_DIAG)
/*???debug */
polhemus_buf[temp_length]='\0';
printf("input_module_polhemus_init.  Polhemus data: %s\n",polhemus_buf);
#endif
												num_read=read(RS232,polhemus_buf,1);
											} while (!feof(polhemus_data));
										}
										fclose(polhemus_data);
										/* now find out how many stations are connected */
										num_read=write(RS232,polhemus_station,3);
										for (i=0;i<POLHEMUS_STATION_SIZE;i++)
										{
											num_read=read(RS232,&polhemus_buf[i],1);
										}
										if ((polhemus_buf[0]=='2') &&
											(polhemus_buf[2]=='l'))
										{
											polhemus_num_stations=0;
											for (i=0;i<4;i++)
											{
												polhemus_station_present[i]=polhemus_buf[3+i]-'0';
												polhemus_num_stations += polhemus_station_present[i];
											}
#if defined (POLHEMUS_DIAG)
/*???debug */
printf("input_module_polhemus_init.  Number of stations found: %i\n",
	polhemus_num_stations);
#endif
											return_code=1;
										}
										else
										{
											display_message(WARNING_MESSAGE,
				"input_module_polhemus_init.  Could not determine number of stations.");
										}
									}
									else
									{
										display_message(WARNING_MESSAGE,
											"input_module_polhemus_init.  Incorrect data record.");
									}
								}
								else
								{
									display_message(WARNING_MESSAGE,
						"input_module_polhemus_init.  No response to polhemus ID request.");
								}
							}
							else
							{
								fprintf(stderr,"Error number was %i\n",errno);
								perror("read");
								display_message(WARNING_MESSAGE,
									"input_module_polhemus_init.  Error reading from port.");
							}
						}
						else
						{
							fprintf(stderr,"Error number was %i\n",errno);
							perror("write");
							display_message(WARNING_MESSAGE,
								"input_module_polhemus_init.  Error writing to port.");
						}
					}
					else
					{
						fprintf(stderr,"Error number was %i\n",errno);
						perror("ioctl");
						display_message(WARNING_MESSAGE,
							"input_module_polhemus_init.  Error sending serial setup data.");
					}
				}
				else
				{
					fprintf(stderr,"Error number was %i\n",errno);
					perror("ioctl");
					display_message(WARNING_MESSAGE,
						"input_module_polhemus_init.  Error receiving serial setup data.");
				}
			}
			else
			{
				fprintf(stderr,"Error number was %i\n",errno);
				perror("open");
				display_message(WARNING_MESSAGE,
					"input_module_polhemus_init.  Error opening serial port.");
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"input_module_polhemus_init.  Not a valid initialisation file.");
			fclose(polhemus_data);
		}
	}
	LEAVE;

	return (return_code);
} /* input_module_polhemus_init */
#endif

#if defined (POLHEMUS)
static int input_module_polhemus_close(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 26 June 1996

DESCRIPTION :
Closes the polhemus RS232 port.
==============================================================================*/
{
	ENTER(input_module_polhemus_close);
	close(RS232);
	LEAVE;

	/*???DB.  What should it be ? */
	return (1);
} /* input_module_polhemus_close */
#endif

#if defined (FARO)
static int input_module_faro_init(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Initialises the faro arm via the RS232C, and gets initial data.
==============================================================================*/
{
	char faro_buf[FARO_INIT_BUFFER_SIZE],temp_str[100];
	FILE *faro_data;
	int baudrate,i,num_read,return_code,serialport;
	struct termio Setup;

	ENTER(input_module_faro_init);

	return_code=0;

	faro_button_pressed=0;

	faro_position_current[0] = 0.0;
	faro_position_current[1] = 0.0;
	faro_position_current[2] = 0.0;
#if defined (PHANTOM_FARO)
	/* SAB Code to see if the Faro Arm widgets, etc. are working without
		having the Faro Arm available */
	return_code = 1;
#else /* defined (PHANTOM_FARO) */	
	/* open an initialisation file to get initial conditions */
	if (faro_data=fopen(FARO_INIT_NAME,"r"))
	{
		/* is it a genuine file */
		fscanf(faro_data,"%s",temp_str);
		if (!strcmp(FARO_INIT_HEADER,temp_str))
		{
			/* read the com port number and baud rate */
			fscanf(faro_data,"%s %i",temp_str,&serialport);
			fscanf(faro_data,"%s %i",temp_str,&baudrate);
			/* initialise the serial port */
			sprintf(temp_str,"/dev/ttyd%i",serialport);
			faro_serial_port=open(temp_str,O_RDWR|O_NOCTTY,0666);
			if (faro_serial_port!=-1)
			{
				/* read in the current settings for the port so that we dont stuff up
					anything important */
				if (1!=ioctl(faro_serial_port,TCGETA,&Setup))
				{
					Setup.c_iflag=IGNBRK;
					Setup.c_oflag=0;
					switch (baudrate)
					{
						case 9600:
						{
							Setup.c_cflag=CS8|CREAD|CLOCAL;
							Setup.c_ospeed=B9600;
						} break;
						case 19200:
						{
							Setup.c_cflag=CS8|CREAD|CLOCAL;
							Setup.c_ospeed=B19200;
						} break;
						case 38400:
						{
							Setup.c_cflag=CS8|CREAD|CLOCAL;
							Setup.c_ospeed=B38400;
						} break;
						case 57600:
						{
							Setup.c_cflag=CS8|CREAD|CLOCAL;
							Setup.c_ospeed=B57600;
						} break;
						case 115200:
						{
							Setup.c_cflag=CS8|CREAD|CLOCAL;
							Setup.c_ospeed=B115200;
						} break;
						default:
						{
							Setup.c_cflag=CS8|CREAD|CLOCAL;
							Setup.c_ospeed=B9600;
							display_message(WARNING_MESSAGE,
							"input_module_faro_init.  Unknown baud rate - set to 9600.");
						} break;
					}
					Setup.c_lflag=0;
					Setup.c_cc[VINTR]=0;
					Setup.c_cc[VQUIT]=0;
					Setup.c_cc[VERASE]=0;
					Setup.c_cc[VKILL]=0;
					Setup.c_cc[VEOF]=0;
					Setup.c_cc[VMIN]=0;
					Setup.c_cc[VEOL]=0;
					/* max. timeout: 1.0 sec */
					Setup.c_cc[VTIME]=10;
					Setup.c_cc[VEOL2]=0;
					if (-1!=ioctl(faro_serial_port,TCSETA,&Setup))
					{
						/* see if it is a faro */
						num_read=write(faro_serial_port,faro_identification,4);
#if defined (FARO_DIAG)
/*???debug */
printf("input_module_faro_init. sending _G\n");
#endif
						if (-1!=num_read)
						{
							/* read the first byte to see if the faro is there or not */
							num_read=read(faro_serial_port,&faro_buf[0],1);
#if defined (OLD_CODE)
							while(!num_read)
							{
								num_read=read(faro_serial_port,&faro_buf[0],1);
							}
#endif /* defined (OLD_CODE) */
							if (-1!=num_read)
							{
								if (num_read==1)
								{ /* we did get a response */
#if defined (FARO_DIAG)
/*???debug */
printf("input_module_faro_init. Received response\n");
#endif
									/* now read the rest */
									num_read=1;
									for (i=1;(i<FARO_INIT_BUFFER_SIZE) && (num_read) &&
										(faro_buf[i-1] != 10) ;i++)
									{
										num_read=read(faro_serial_port,&faro_buf[i],1);
									}
									if (faro_buf[0]=='+')
									{
										faro_buf[i]='\0';
#if defined (FARO_DIAG)
										/*???debug */
										printf("input_module_faro_init.  Faro ID: %s\n",faro_buf);
#endif
                              /* now read the next two strings */
										num_read=1;
										for (i=1;(i<FARO_INIT_BUFFER_SIZE) && (num_read) &&
												  (faro_buf[i-1] != 10) ;i++)
										{
											num_read=read(faro_serial_port,&faro_buf[i],1);
										}
										num_read=1;
										for (i=1;(i<FARO_INIT_BUFFER_SIZE) && (num_read) &&
												  (faro_buf[i-1] != 10) ;i++)
										{
											num_read=read(faro_serial_port,&faro_buf[i],1);
										}

										/* check the file for initialisation data */
										if (!feof(faro_data))
										{
											int temp_length;
											do
											{
												fscanf(faro_data,"%s\n",faro_buf);
												temp_length=strlen(faro_buf);
												/* add a carriage return and line feed */
												faro_buf[temp_length++]='\r';
												faro_buf[temp_length++]='\n';
												write(faro_serial_port,faro_buf,temp_length);
#if defined (FARO_DIAG)
/*???debug */
faro_buf[temp_length]='\0';
printf("input_module_faro_init.  Faro data: %s\n",faro_buf);
#endif
											} while (!feof(faro_data));
										}
										return_code = 1;
									}
									else
									{
										faro_buf[i]='\0';
										display_message(WARNING_MESSAGE,
											"input_module_faro_init.  Incorrect response to faro ID request..\n%s", faro_buf);
									}
								}
								else
								{
									display_message(WARNING_MESSAGE,
						"input_module_faro_init.  No response to faro ID request.");
								}
							}
							else
							{
								fprintf(stderr,"Error number was %i\n",errno);
								perror("read");
								display_message(WARNING_MESSAGE,
									"input_module_faro_init.  Error reading from port.");
							}
						}
						else
						{
							fprintf(stderr,"Error number was %i\n",errno);
							perror("write");
							display_message(WARNING_MESSAGE,
								"input_module_faro_init.  Error writing to port.");
						}
					}
					else
					{
						fprintf(stderr,"Error number was %i\n",errno);
						perror("ioctl");
						display_message(WARNING_MESSAGE,
							"input_module_faro_init.  Error sending serial setup data.");
					}
				}
				else
				{
					fprintf(stderr,"Error number was %i\n",errno);
					perror("ioctl");
					display_message(WARNING_MESSAGE,
						"input_module_faro_init.  Error receiving serial setup data.");
				}
			}
			else
			{
				fprintf(stderr,"Error number was %i\n",errno);
				perror("open");
				display_message(WARNING_MESSAGE,
					"input_module_faro_init.  Error opening serial port.");
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"input_module_faro_init.  Not a valid initialisation file.");
		}
		fclose(faro_data);
	}
#endif /* defined (PHANTOM_FARO) */	
	LEAVE;

	return (return_code);
} /* input_module_faro_init */
#endif

#if defined (FARO)
int input_module_faro_calibrate(void)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Sends commands to calibrate the faro arm
==============================================================================*/
{
	int return_code;

	ENTER(input_module_faro_calibrate);

	return_code=0;

#if defined (PHANTOM_FARO)
	return_code = 1;
#else /* defined (PHANTOM_FARO) */	
	write(faro_serial_port,&faro_three_point_calibrate,4);
#endif /* defined (PHANTOM_FARO) */	
	LEAVE;

	return (return_code);
} /* input_module_faro_calibrate */
#endif /* defined (FARO) */

#if defined (FARO)
static int input_module_faro_close(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 26 June 1996

DESCRIPTION :
Closes the faro faro_serial_port port.
==============================================================================*/
{
	ENTER(input_module_faro_close);
	close(faro_serial_port);
	LEAVE;

	/*???DB.  What should it be ? */
	return (1);
} /* input_module_faro_close */
#endif /* defined (FARO) */

#if defined (DIALS)
static int input_module_dials_init(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 26 June 1996

DESCRIPTION :
Sets the dials so that any input is recognised, and messages are received.
==============================================================================*/
{
	int i,return_code;

	ENTER(input_module_dials_init);
	if (user_interface)
	{
		/* find the dials */
		for (i=0;i<ndevices;i++)
		{
			if (0==strcmp(device_list[i].name,"dial+buttons"))
			{
				/* open the dials */
				dials_device=XOpenDevice(user_interface->display,device_list[i].id);
			};
		};
		if (dials_device)
		{
			DeviceMotionNotify(dials_device,dials_motion_event_type,
				dials_motion_event_class);
			ListOfDialsEventClass[0]=dials_motion_event_class;
			DeviceButtonPress(dials_device,dials_button_press_event_type,
				dials_button_press_event_class);
			ListOfDialsEventClass[1]=dials_button_press_event_class;
			DeviceButtonRelease(dials_device,dials_button_release_event_type,
				dials_button_release_event_class);
			ListOfDialsEventClass[2]=dials_button_release_event_class;
#if defined (DIALS_DIAG)
/*???debug */
printf("input_module_dials_init.  Dials initialised.\n");
#endif
			/* Now show that none of the dials have been calibrated */
			for (i=0;i<6;i++)
			{
				dials_calibrated[i]=0;
				dials_calibrate_value[i]=0;
			}
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"input_module_dials_init.  Missing user_interface");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* input_module_dials_init */
#endif

#if defined (DIALS)
static int input_module_dials_close(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 26 June 1996

DESCRIPTION :
Closes the dials.
==============================================================================*/
{
	int return_code;

	ENTER(input_module_dials_close);
	if (user_interface)
	{
		XCloseDevice(user_interface->display,dials_device);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"input_module_dials_init.  Missing user_interface");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* input_module_dials_close */
#endif

/*
Global functions
----------------
*/
int input_module_init(struct User_interface *user_interface)
/*****************************************************************************
LAST MODIFIED : 6 January 1998

DESCRIPTION :
Sets up any devices correctly, and initialises the callback stack.
============================================================================*/
{
	int i,return_code;

	ENTER(input_module_init);
	/* check arguments */
	if (user_interface)
	{
		/* initialise the list of callbacks */
		for (i=0;i<INPUT_MODULE_NUM_DEVICES;i++)
		{
			input_module_list[i]=NULL;
		}
		/* find out which devices are present */
#if defined (SGI)
		device_list=(XDeviceInfoPtr)XListInputDevices(user_interface->display,
			&ndevices);
#endif
		return_code=1;
#if defined (SPACEBALL)
		spaceball_valid=input_module_spaceball_init(user_interface);
		return_code=return_code&&spaceball_valid;
#endif
#if defined (POLHEMUS)
		polhemus_valid=input_module_polhemus_init(user_interface);
		return_code=return_code&&polhemus_valid;
#endif
#if defined (DIALS)
		dials_valid=input_module_dials_init(user_interface);
		return_code=return_code&&dials_valid;
#endif
#if defined (HAPTIC)
		haptic_valid = input_module_haptic_init(user_interface);
		haptic_position_scale[0] = 1.0;
		haptic_position_scale[1] = 1.0;
		haptic_position_scale[2] = 1.0;
		haptic_position_offset[0] = 0.0;
		haptic_position_offset[1] = 0.0;
		haptic_position_offset[2] = 0.0;
		haptic_position_current[0] = 0.0;
		haptic_position_current[1] = 0.0;
		haptic_position_current[2] = 0.0;
		return_code=return_code&&haptic_valid;
#endif /* defined (HAPTIC) */
#if defined (FARO)
		faro_valid=input_module_faro_init(user_interface);
		return_code=return_code&&faro_valid;
#endif
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"input_module_init.  Missing user_interface");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* input_module_init */

char *input_module_is_device_valid(enum Input_module_device device)
/*****************************************************************************
LAST MODIFIED : 6 January 1998

DESCRIPTION :
Returns a pointer to a name for the device.  If the device is non-valid, it
returns NULL.
============================================================================*/
{
	char *return_code;
#if defined (POLHEMUS)
	int polhemus_number;
#endif

	ENTER(input_module_is_device_valid);
	return_code=(char *)NULL;
	switch (device)
	{
		case IM_DEVICE_SPACEBALL:
		{
#if defined (SPACEBALL)
			if (spaceball_valid)
			{
				return_code=input_module_name[device];
			}
#endif
		} break;
		case IM_DEVICE_POLHEMUS1:
		case IM_DEVICE_POLHEMUS2:
		case IM_DEVICE_POLHEMUS3:
		case IM_DEVICE_POLHEMUS4:
		{
#if defined (POLHEMUS)
			polhemus_number=device-IM_DEVICE_POLHEMUS1;
			if ((polhemus_valid)&&(polhemus_station_present[polhemus_number]))
			{
				return_code=input_module_name[device];
			}
#endif
		} break;
		case IM_DEVICE_DIALS:
		{
#if defined (DIALS)
			if (dials_valid)
			{
				return_code=input_module_name[device];
			}
#endif
		} break;
		case IM_DEVICE_HAPTIC:
		{
#if defined (HAPTIC)
			if (haptic_valid)
			{
				return_code=input_module_name[device];
			}
#endif /* defined (HAPTIC) */
		} break;
		case IM_DEVICE_FARO:
		{
#if defined (FARO)
			if (faro_valid)
			{
				return_code=input_module_name[device];
			}
#endif /* defined (HAPTIC) */
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"input_module_is_device_valid.  Invalid device");
		} break;
	}
	LEAVE;

	return (return_code);
} /* input_module_is_device_valid */

int input_module_is_source_valid(enum Input_module_source source)
/*****************************************************************************
LAST MODIFIED : 6 January 1998

DESCRIPTION :
Returns a true if the source is valid.
============================================================================*/
{
	int i,return_code;

	ENTER(input_module_is_source_valid);
	return_code=0;
	switch (source)
	{
		case IM_SOURCE_SPACEBALL:
		{
#if defined (SPACEBALL)
			if (spaceball_valid)
			{
				return_code=1;
			}
#endif
		} break;
		case IM_SOURCE_POLHEMUS:
		{
#if defined (POLHEMUS)
			for (i=0;i<4;i++)
			{
				if (polhemus_station_present[i])
				{
					return_code=1;
				}
			}
#endif
		} break;
		case IM_SOURCE_DIALS:
		{
#if defined (DIALS)
			if (dials_valid)
			{
				return_code=1;
			}
#endif
		} break;
		case IM_SOURCE_HAPTIC:
		{
#if defined (HAPTIC)
			if (haptic_valid)
			{
				return_code=1;
			}
#endif /* defined (HAPTIC) */
		} break;
		case IM_SOURCE_FARO:
		{
#if defined (HAPTIC)
			if (faro_valid)
			{
				return_code=1;
			}
#endif /* defined (HAPTIC) */
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"input_module_is_source_valid.  Invalid source");
		} break;
	}
	LEAVE;

	return (return_code);
} /* input_module_is_source_valid */

int input_module_update(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 6 January 1998

DESCRIPTION :
Forces the input_module to see if there are any periodic updates
(ie polhemus etc).
==============================================================================*/
{
	Input_module_message message;
	int return_code;
	static char buf[INPUT_UPDATE_BUFFER_SIZE];
#if defined (POLHEMUS)
	char *temp;
	float temp_values[12];
	int i,j,num_read,station;
#endif /* defined (POLHEMUS) */
#if defined (FARO)
	double sinA, cosA, sinB, cosB, sinC, cosC;
	int faro_state;
#endif /* defined (FARO) */
	ENTER(input_module_update);
	return_code=0;
	/* check arguments */
	if (user_interface)
	{
		/* first allocate memory for a message */
		if (ALLOCATE(message,struct Input_module_message_struct,1))
		{
#if defined (POLHEMUS)
			if (polhemus_valid)
			{
				/* send request for data record */
				write(RS232,&polhemus_get_record,1);
				/* try and read the record */
				for (i=0;i<(polhemus_num_stations*POLHEMUS_GET_RECORD_SIZE);i++)
				{
					num_read=read(RS232,&buf[i],1);
					if (num_read!=1)
					{
						display_message(WARNING_MESSAGE,
							"input_module_update(polhemus).  Insufficient bytes read\n");
					}
				}
				if (buf[0]=='0') /* this is message type (0) */
				{
					for (j=0;j<polhemus_num_stations;j++)
					{
						/* first three bytes are a header, with an error code */
						if (buf[2+j*POLHEMUS_GET_RECORD_SIZE]!=' ')
						{
							buf[1+polhemus_num_stations*POLHEMUS_GET_RECORD_SIZE]='\0';
							display_message(WARNING_MESSAGE,
								"input_module_update(polhemus).  Error occurred.");
							printf("Error was %c %i\n",buf[2+j*POLHEMUS_GET_RECORD_SIZE],
								buf[2+j*POLHEMUS_GET_RECORD_SIZE]);
						}
						station=buf[1+j*POLHEMUS_GET_RECORD_SIZE]-'1';
						/* bytes 4-15 are the positions */
						/* bytes 16-27 are the directions */
						/* the intel byte ordering is different to indigo, so change it */
						message->source=IM_SOURCE_POLHEMUS;
						message->type=IM_TYPE_MOTION;
						temp=(char *)temp_values;
						for (i=0;i<12;i++)
						{
							temp[0+4*i]=buf[5+j*POLHEMUS_GET_RECORD_SIZE+3+4*i];
							temp[1+4*i]=buf[5+j*POLHEMUS_GET_RECORD_SIZE+2+4*i];
							temp[2+4*i]=buf[5+j*POLHEMUS_GET_RECORD_SIZE+1+4*i];
							temp[3+4*i]=buf[5+j*POLHEMUS_GET_RECORD_SIZE+0+4*i];
						}
						/* tell the clients the absolute position of the receiver */
						for (i=1;i<4;i++)
						{
							message->data[i*3+0]=temp_values[1*3+i-1];
							message->data[i*3+1]=temp_values[2*3+i-1];
							message->data[i*3+2]=temp_values[3*3+i-1];
						}
						matrix_copy(&polhemus_last_direction,(Gmatrix *)&message->data[3]);
						/* transform the position by the origin... */
						for (i=0;i<3;i++)
						{
							polhemus_last_position[i]=temp_values[i];
							message->data[i]=
							polhemus_last_position[i]-polhemus_origin_position[i];
						}
						matrix_premult_vector(message->data,&polhemus_origin_direction);
						matrix_premult((Gmatrix *)&message->data[3],
						&polhemus_origin_direction);
						/* now send the clients the data */
						input_module_do_callback(message,IM_DEVICE_POLHEMUS1+station,
							user_interface);
						/* Check for the button */
						if (buf[4+j*POLHEMUS_GET_RECORD_SIZE]-'0')
						{
							if (!polhemus_button_pressed)
							{
								polhemus_button_pressed=1;
								message->source=IM_SOURCE_POLHEMUS;
								message->type=IM_TYPE_BUTTON_PRESS;
								input_module_do_callback(message,IM_DEVICE_POLHEMUS1,
									user_interface);
							}
						}
						else
						{
							if (polhemus_button_pressed)
							{
								polhemus_button_pressed=0;
								message->source=IM_SOURCE_POLHEMUS;
								message->type=IM_TYPE_BUTTON_RELEASE;
								input_module_do_callback(message,IM_DEVICE_POLHEMUS1,
									user_interface);
							}
						}
						return_code=1;
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,
						"input_module_update(polhemus).  Incorrect data record.");
				}
			}
#endif
#if defined (FARO)
			if (faro_valid)
			{
#if defined (PHANTOM_FARO)
			  /* SAB Generates a series of numbers for testing when Faro Arm
				  isn't available */
			  message->source=IM_SOURCE_FARO;
			  message->type=IM_TYPE_MOTION;
			  message->data[0] = faro_position_current[0];
			  message->data[1] = faro_position_current[1];
			  message->data[2] = faro_position_current[2];
			  faro_position_current[0] += 0.1;
			  faro_position_current[1] *= 1.0001;
			  faro_position_current[2] -= 0.01;
			  /* tangent */
			  message->data[3] = 0;
			  message->data[4] = 4;
			  message->data[5] = 8;
			  /* normal */
			  message->data[6] = 1;
			  message->data[7] = 3;
			  message->data[8] = 7;

			  /* now send the clients the data */
			  input_module_do_callback(message,IM_DEVICE_FARO,user_interface);
			  
			  return_code=1;
#else /* defined (PHANTOM_FARO) */
				/* send request for data record */
				write(faro_serial_port,&faro_get_record,4);
				num_read=read(faro_serial_port,&buf[0],1);
				/* try and read the record */
				for (i=1;i<INPUT_UPDATE_BUFFER_SIZE && (buf[i-1] != '\n') ; i++)
				{
					num_read=read(faro_serial_port,&buf[i],1);
					if (num_read!=1)
					{
						display_message(WARNING_MESSAGE,
							"input_module_update(faro).  Insufficient bytes read\n");
					}
				}
				if (buf[0]=='+')
				{
					buf[i-2] = 0;
					sscanf(buf+1, "%d%lf%lf%lf%lf%lf%lf", &faro_state,
						&(faro_position_current[0]),
						&(faro_position_current[1]), &(faro_position_current[2]),
						&(faro_angle_current[0]), &(faro_angle_current[1]),
						&(faro_angle_current[2]));

					if (faro_state & 0xfffc)
					{
						display_message(WARNING_MESSAGE,
							"input_module_update(faro).  Faro Arm error state %d\n",
							((faro_state & 0xfc) >> 2));						
					}

					message->source=IM_SOURCE_FARO;
					message->type=IM_TYPE_MOTION;
					if (faro_state & 0x0003)
					{
						if(!faro_button_pressed)
						{
							message->type=IM_TYPE_BUTTON_PRESS;
							faro_button_pressed = 1;
						}
					}
					else
					{
						if(faro_button_pressed)
						{
							message->type=IM_TYPE_BUTTON_RELEASE;
							faro_button_pressed = 0;
						}
					}
					message->data[0] = faro_position_current[0];
					message->data[1] = faro_position_current[1];
					message->data[2] = faro_position_current[2];
					sinA = sin(faro_angle_current[0] * PI / 180.0);
					sinB = sin(faro_angle_current[1] * PI / 180.0);
					sinC = sin(faro_angle_current[2] * PI / 180.0);
					cosA = cos(faro_angle_current[0] * PI / 180.0);
					cosB = cos(faro_angle_current[1] * PI / 180.0);
					cosC = cos(faro_angle_current[2] * PI / 180.0);
					/* tangent */
					message->data[3] = sinA * sinB;
					message->data[4] = -cosA * sinB;
					message->data[5] = cosB;
					/* normal */
					message->data[6] = cosC * cosA +
					   sinC * cosB * sinA;
					message->data[7] = - cosC * sinA +
					   sinC * cosB * cosA;
					message->data[8] = sinC * sinB;

#if defined (OLD_CODE)
					printf("input_module_update(faro). %d %lf %lf %lf\n\t%lf %lf %lf\n\t%lf %lf %lf\n",
						faro_state, faro_position_current[0],
						faro_position_current[1], faro_position_current[2],
						faro_angle_current[0], faro_angle_current[1],
						faro_angle_current[2],
						message->data[3], message->data[4], message->data[5]);
#endif /* defined (OLD_CODE) */

					/* now send the clients the data */
					input_module_do_callback(message,IM_DEVICE_FARO,user_interface);

					return_code=1;
				}
				else
				{
					display_message(WARNING_MESSAGE,
						"input_module_update(faro).  Incorrect response from faro arm.");
				}
#endif /* defined (PHANTOM_FARO) */
			}
#endif /* defined (FARO) */
#if defined (HAPTIC)
			if (haptic_valid)
			{
				return_code=input_module_haptic_position(user_interface,message);
				/* debug */
						/*printf("x coord %f,y coord %f,z coord %f\n",message->data[0],
					message->data[1],message->data[2]);*/
				message->data[0] = (message->data[0] - haptic_position_offset[0]) * haptic_position_scale[0];
				message->data[1] = (message->data[1] - haptic_position_offset[1]) * haptic_position_scale[1];
				message->data[2] = (message->data[2] - haptic_position_offset[2]) * haptic_position_scale[2];
				haptic_position_current[0] = message->data[0];
				haptic_position_current[1] = message->data[1];
				haptic_position_current[2] = message->data[2];
				/* now send the clients the data */
				input_module_do_callback(message,IM_DEVICE_HAPTIC,user_interface);
			}
#endif /* defined (HAPTIC) */
			DEALLOCATE(message);
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"input_module_update.  Could not allocate memory for message");
		}
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"input_module_update.  Missing user_interface");
	}
	LEAVE;

	return (return_code);
} /* input_module_update */

int input_module_process(XEvent *event,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
Checks all events coming into the clients event loop, and searches for any
that are from devices it is interested in.  If any are found, it creates a
message and then sends it to do_callback.
==============================================================================*/
{
	Input_module_message message;
	int i,mess_pos,return_code,temp_spaceball_motion[6];
#if defined (SPACEBALL)
#if defined (IBM)
	XClientMessageEvent *CM=(XClientMessageEvent *)event;
#endif
#if defined (SGI)
	static int sbdata[7]={0};
	XDeviceMotionEvent *M=(XDeviceMotionEvent *)event;
#endif
#endif

	ENTER(input_module_process);
	/* check arguments */
	if (event&&user_interface)
	{
		/* first allocate memory for a message */
		if (ALLOCATE(message,struct Input_module_message_struct,1))
		{
#if defined (SPACEBALL)
#if defined (IBM)
			if (ClientMessage==event->type)
			{
#endif
#if defined (IBM)
				if ((spaceball_motion_event_type==CM->message_type)&&
					!XtAppPending(app_context))
#endif
#if defined (SGI)
				/*    if (event->type==spaceball_motion_event_type) */
				if ((spaceball_motion_event_type==event->type)&&
					!XtAppPending(user_interface->application_context))
				/*      !(XtIMAlternateInput&XtAppPending(app_context)))*/
					/*???DB.  Need to think some more about reducing the number of
						events */
#endif
				{
#if defined (SGI)
					if (M->deviceid==spaceball_device->device_id)
#endif
					{
#if defined (SGI)
						for (i=0;i<M->axes_count;i++)
						{
							sbdata[(M->first_axis)+i]=(M->axis_data)[i];
						}
#endif
						/* go through and set values (any value less than 20 is 0, ie not
							registered. Values range approx -500->500 */
						for (i=0;i<6;i++)
						{
							temp_spaceball_motion[i]=
#if defined (IBM)
								(((CM->data).s)[i+2])/IM_SPACEBALL_SCALE;
#endif
#if defined (SGI)
								(sbdata[i])/IM_SPACEBALL_SCALE;
#endif
						}
						message->source=IM_SOURCE_SPACEBALL;
						message->type=IM_TYPE_MOTION;
						message->data[0]=(double)temp_spaceball_motion[0];
						message->data[1]=(double)temp_spaceball_motion[1];
						message->data[2]= -(double)temp_spaceball_motion[2];
						message->data[3]= -(double)temp_spaceball_motion[4];
						message->data[4]=(double)temp_spaceball_motion[3];
						message->data[5]=(double)temp_spaceball_motion[5];
						/* make sure we need to send the message */
						if (fabs(message->data[0])+fabs(message->data[1])+
							fabs(message->data[2])+fabs(message->data[3])+
							fabs(message->data[4])+fabs(message->data[5])>0)
						{
							input_module_do_callback(message,IM_DEVICE_SPACEBALL,
								user_interface);
						}
						return_code=1;
					}
#if defined (SGI)
					else
					{
						display_message(WARNING_MESSAGE,
							"input_module_process.  Spaceball device id's don't match");
						return_code=0;
					}
#endif
				}
#if defined (IBM)
				else if (spaceball_button_press_event_type==CM->message_type)
#endif
#if defined (SGI)
				else if (spaceball_button_press_event_type==event->type)
#endif
				{
					message->source=IM_SOURCE_SPACEBALL;
					message->type=IM_TYPE_BUTTON_PRESS;
#if defined (IBM)
					message->data[0]=((CM->data).s)[2];
#endif
#if defined (SGI)
					message->data[0]=((XDeviceButtonEvent *)event)->button;
#endif
					input_module_do_callback(message,IM_DEVICE_SPACEBALL,
						user_interface);
					return_code=1;
				}
#if defined (IBM)
				else if (spaceball_button_release_event_type==CM->message_type)
#endif
#if defined (SGI)
				else if (spaceball_button_release_event_type==event->type)
#endif
				{
					message->source=IM_SOURCE_SPACEBALL;
					message->type=IM_TYPE_BUTTON_RELEASE;
#if defined (IBM)
					message->data[0]=((CM->data).s)[2];
#endif
#if defined (SGI)
					message->data[0]=((XDeviceButtonEvent *)event)->button;
#endif
					input_module_do_callback(message,IM_DEVICE_SPACEBALL,
						user_interface);
					return_code=1;
				}
#if defined (IBM)
			}
#endif
			else
#endif /* SPACEBALL */
#if defined (DIALS)
			if ((event->type==dials_motion_event_type)&&
				!XtAppPending(user_interface->application_context))
/*        !(XtIMAlternateInput&XtAppPending(user_interface->application_context)))*/
			/*???DB.  Need to think some more about reducing the number of events */
			{
				XDeviceMotionEvent *M=(XDeviceMotionEvent *)event;
				if (M->deviceid==dials_device->device_id)
				{
					/* There are eight dials on the box, we only want to use dials
					0-5 (6 dof) */
					message->source=IM_SOURCE_DIALS;
					message->type=IM_TYPE_MOTION;
					for (i=0;i<6;i++)
					{
						message->data[i]=0.0;
					}
					for (i=0;i<M->axes_count;i++)
					{
						if (M->first_axis+i<6)
						{
							/* we actually have a translation to perform here, as we need
							dials 0,2,4 to be message positions 0,1,2 and
							dials 1,3,5 to be message positions 3,4,5 */
							mess_pos=(M->first_axis+i)/2;
							if (2*mess_pos!=M->first_axis+i)
							{
								mess_pos=(M->first_axis+i+5)/2;
								mess_pos=8-mess_pos;
								/* make it so that the order of the dials goes downwards */
							}
							else
							{
								mess_pos=2-mess_pos;
							}
							if (dials_calibrated[M->first_axis+i])
							{
								message->data[mess_pos]=(M->axis_data)[i]-
									dials_calibrate_value[M->first_axis+i];
							}
							else
							{
								message->data[mess_pos]=0.0;
								/* we dont know which way it was turned first */
								dials_calibrated[M->first_axis+i]=1;
							}
							dials_calibrate_value[M->first_axis+i]=(M->axis_data)[i];
						}
					}
					/* make sure we need to send the message */
					if (fabs(message->data[0])+fabs(message->data[1])+
						fabs(message->data[2])+fabs(message->data[3])+
						fabs(message->data[4])+fabs(message->data[5])>0)
					{
						input_module_do_callback(message,IM_DEVICE_DIALS,
							user_interface);
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,
						"input_module_process.  Dial device id's don't match");
					return_code=0;
				}
			}
			else
#endif /* DIALS */
			if (event->type==ButtonPress)
			{
/*        XButtonPressedEvent *B=(XButtonPressedEvent *)event;*/
				return_code=1;
			}
			else if (event->type==ButtonRelease)
			{
/*        XButtonReleasedEvent *B=(XButtonReleasedEvent *)event;*/
				return_code=1;
			}
			else
#if defined (KEYBOARD)
			if (event->type==KeyRelease)
			{
				/* do some stuff here ..... */
/*???debug */
/*if (XK_Escape==XKeycodeToKeysym(display,((XKeyReleasedEvent *)event)->keycode,
	0))
{
	printf("ESC released\n");
}
else
{
	printf("key released\n");
}*/
				return_code=1;
			}
			else
#endif /* KEYBOARD */
			{
/*???debug.  This will print out every other x event */
/*printf("unclassified event type = %d\n",event->type);*/
				return_code=0;
			}
			DEALLOCATE(message);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"input_module_process.  Could not allocate memory for message");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"input_module_process.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* input_module_process */

int input_module_add_input_window(Widget widget,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 23 June 1995

DESCRIPTION :
Let us receive events (ie from spaceball) if the mouse is in this window.
==============================================================================*/
{
	int return_code;

	ENTER(input_module_add_input_window);
	/* check arguments */
	if (widget&&user_interface)
	{
		return_code=1;
#if defined (SPACEBALL)
		if (spaceball_valid && spaceball_device)
		{
#if defined (SGI)
			XSelectExtensionEvent(user_interface->display,XtWindow(widget),
				ListOfSpaceballEventClass,3);
#endif
		}
#endif
#if defined (DIALS)
		if (dials_valid && dials_device)
		{
			XSelectExtensionEvent(user_interface->display,XtWindow(widget),
				ListOfDialsEventClass,3);
		}
#endif
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"input_module_add_input_window.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* input_module_add_input_window */


int input_module_register(enum Input_module_device device_req,void *ident,
	Widget w,Input_module_callback_proc callback_func)
/*******************************************************************************
LAST MODIFIED : 11 May 1994

DESCRIPTION :
Add a callback to the linked list for the particular device.
==============================================================================*/
{
	Input_module_callback_info temp_info,temp;
	int return_code;

	ENTER(input_module_register);
	return_code=0;
	/* valid request */
	if (device_req<INPUT_MODULE_NUM_DEVICES)
	{
		/* get mem for the structure */
		if (ALLOCATE(temp_info,struct Input_module_callback_info_struct,1))
		{
			temp_info->identifier=ident;
			temp_info->callback=callback_func;
			temp_info->next=NULL;
			temp_info->client_window=XtWindow(w);
			/* is this the first callback */
			if (!input_module_list[device_req])
			{
				input_module_list[device_req]=temp_info;
			}
			else
			{
				/* get to the end of the list */
				temp=input_module_list[device_req];
				while (temp->next)
				{
					temp=temp->next;
				}
				temp->next=temp_info;
			}
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"input_module_register.  Invalid device number");
	}
	LEAVE;

	return (return_code);
} /* input_module_register */


int input_module_deregister(enum Input_module_device device_req,void *ident)
/*******************************************************************************
LAST MODIFIED : 11 May 1994

DESCRIPTION :
Remove a callback from the linked list of a particular device.
==============================================================================*/
{
	int return_code;
	Input_module_callback_info prev,temp;

	ENTER(input_module_deregister);
	return_code=0;
	/* valid request */
	if (device_req<INPUT_MODULE_NUM_DEVICES)
	{
		/* start at the beginning */
		temp=input_module_list[device_req];
		prev=NULL;
		while (temp)
		{
			if (temp->identifier==ident)
			{
				/* found it so join the list, then destroy it */
				/* is it the first? */
				if (!prev)
				{
					input_module_list[device_req]=temp->next;
					prev=temp;
					DEALLOCATE(temp);
					temp=prev->next;
					prev=NULL;
				}
				else
				{
					prev->next=temp->next;
					DEALLOCATE(temp);
				}
				return_code=1;
			}
			else
			{
				prev=temp;
				temp=temp->next;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"input_module_deregister.  Invalid device number");
	}
	LEAVE;

	return (return_code);
} /* input_module_deregister */

int input_module_set( enum Input_module_device device,
	enum Input_module_data_types mode, void *data )
/*******************************************************************************
LAST MODIFIED : 16 May 1998

DESCRIPTION :
Changes the setup of the input module device.
Currently supports a scaling and an offset of HAPTIC for mapping it
to world coordinates.
==============================================================================*/
{
	double *double_data;
	int return_code;

	switch ( mode )
	{
		case IM_POSITION_SCALE:
		{
			if ( data )
			{
				double_data = (double *)data;

				return_code = 1;
				switch ( device )
				{
#if defined (HAPTIC)
					case IM_DEVICE_HAPTIC:
					{
						haptic_set_scale(double_data[0], double_data[1], double_data[2]);
						/* SAB The haptic environment only supports a single scale factor
							so I have made them all work off the x */
						haptic_position_scale[0] = double_data[0];
						haptic_position_scale[1] = double_data[0];
						haptic_position_scale[2] = double_data[0];
					} break;
#endif /* defined (HAPTIC) */
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"input_module_set. Invalid data");
			}
		} break;
		case IM_POSITION_OFFSET:
		{
			if ( data )
			{
				double_data = (double *)data;

				return_code = 1;
				switch ( device )
				{
#if defined (HAPTIC)
					case IM_DEVICE_HAPTIC:
					{
						haptic_position_offset[0] = double_data[0];
						haptic_position_offset[1] = double_data[1];
						haptic_position_offset[2] = double_data[2];
					} break;
#endif /* defined (HAPTIC) */
				}
			}
			else
			{
				return_code = 0;
				display_message(ERROR_MESSAGE,
					"input_module_set. Invalid data");
			}
		} break;
		case IM_POSITION_CURRENT:
		{
			return_code = 0;
			display_message(WARNING_MESSAGE,
				"input_module_set. Cannot set IM_POSITION_CURRENT, set the offset instead");
		} break;
		case IM_SET_CURRENT_AS_ORIGIN:
		{
			return_code = 1;
			switch ( device )
			{
#if defined (HAPTIC)
				case IM_DEVICE_HAPTIC:
				{
					if ( haptic_position_scale[0] )
					{
						haptic_position_offset[0] = (haptic_position_current[0]) / haptic_position_scale[0] + haptic_position_offset[0];
					}
					else
					{
						haptic_position_offset[0] = 0.0;
					}
					if ( haptic_position_scale[1] )
					{
						haptic_position_offset[1] = (haptic_position_current[1]) / haptic_position_scale[1] + haptic_position_offset[1];
					}
					else
					{
						haptic_position_offset[1] = 0.0;
					}
					if ( haptic_position_scale[2] )
					{
						haptic_position_offset[2] = (haptic_position_current[2]) / haptic_position_scale[2] + haptic_position_offset[2];
					}
					else
					{
						haptic_position_offset[2] = 0.0;
					}
				} break;
#endif /* defined (HAPTIC) */
			}
		} break;
	}
	LEAVE;

	return (return_code);
} /* input_module_set */

int input_module_get( enum Input_module_device device,
	enum Input_module_data_types mode, void *data )
/*******************************************************************************
LAST MODIFIED : 16 May 1998

DESCRIPTION :
Retrieves the setup of the input device.
Currently supports a scaling and an offset of HAPTIC for mapping it
to world coordinates.
==============================================================================*/
{
	double *double_data;
	int return_code;

	switch ( mode )
	{
		case IM_POSITION_SCALE:
		{
			if ( data )
			{
				double_data = (double *)data;

				return_code = 1;
				switch ( device )
				{
#if defined (HAPTIC)
					case IM_DEVICE_HAPTIC:
					{
						double_data[0] = haptic_position_scale[0];
						double_data[1] = haptic_position_scale[1];
						double_data[2] = haptic_position_scale[2];
					} break;
#endif /* defined (HAPTIC) */
				}
			}
			else
			{
				return_code = 0;
				display_message(ERROR_MESSAGE,
					"input_module_set. Invalid data");
			}
		} break;
		case IM_POSITION_OFFSET:
		{
			if ( data )
			{
				double_data = (double *)data;

				return_code = 1;
				switch ( device )
				{
#if defined (HAPTIC)
					case IM_DEVICE_HAPTIC:
					{
						double_data[0] = haptic_position_offset[0];
						double_data[1] = haptic_position_offset[1];
						double_data[2] = haptic_position_offset[2];
					} break;
#endif /* defined (HAPTIC) */
				}
			}
			else
			{
				return_code = 0;
				display_message(ERROR_MESSAGE,
					"input_module_get. Invalid data");
			}
		} break;
		case IM_POSITION_CURRENT:
		{
			if ( data )
			{
				double_data = (double *)data;

				return_code = 1;
				switch ( device )
				{
#if defined (HAPTIC)
					case IM_DEVICE_HAPTIC:
					{
						double_data[0] = haptic_position_current[0];
						double_data[1] = haptic_position_current[1];
						double_data[2] = haptic_position_current[2];
					} break;
#endif /* defined (HAPTIC) */
				}
			}
			else
			{
				return_code = 0;
				display_message(ERROR_MESSAGE,
					"input_module_get. Invalid data");
			}
		} break;
		case IM_SET_CURRENT_AS_ORIGIN:
		{
			return_code = 0;
			display_message(WARNING_MESSAGE,
				"input_module_get. Cannot get IM_SET_CURRENT_AS_ORIGIN");
		}
	}
	LEAVE;

	return (return_code);
} /* input_module_get */

int input_module_close(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
Close the input devices.
==============================================================================*/
{
	Input_module_callback_info last,temp;
	int i;

	ENTER(input_module_close);
#if defined (SPACEBALL)
	if (spaceball_valid)
	{
		input_module_spaceball_close(user_interface);
	}
#endif
#if defined (POLHEMUS)
	if (polhemus_valid)
	{
		input_module_polhemus_close(user_interface);
	}
#endif
#if defined (DIALS)
	if (dials_valid)
	{
		input_module_dials_close(user_interface);
	}
#endif
	/* now destroy the linked lists of callbacks */
	for (i=0;i<INPUT_MODULE_NUM_DEVICES;i++)
	{
		temp=input_module_list[i];
		while (temp)
		{
			last=temp;
			temp=temp->next;
			DEALLOCATE(last);
		}
	}

	LEAVE;

	/*???DB.  What should it be ? */
	return (1);
} /* input_module_close */

#if defined (POLHEMUS)
void input_module_polhemus_set_origin(void)
/*******************************************************************************
LAST MODIFIED : 06 April 1995

DESCRIPTION :
Sets the polhemus so that the current position and origin is defined as the
origin.
==============================================================================*/
{
	int i;

	ENTER(input_module_polhemus_set_origin);
	/*???debug */
	printf("setting origin\n");

	/* origin pos is the last pos */
	for (i=0;i<3;i++)
	{
		polhemus_origin_position[i]=polhemus_last_position[i];
	}
	matrix_copy(&polhemus_origin_direction,&polhemus_last_direction);
	LEAVE;
} /* input_module_polhemus_set_origin */

void input_module_polhemus_reset_pos(double *position)
/*******************************************************************************
LAST MODIFIED : 8 December 1994

DESCRIPTION :
Define the global origin and orientation for the polhemus so that all stations
will be able to be reset to the same values.
==============================================================================*/
{
	int i;

	for (i=0;i<3;i++)
	{
		global_position[i]=position[i];
	}
} /* input_module_polhemus_reset_pos */

void input_module_polhemus_reset_dir(Gmatrix *direction)
/*******************************************************************************
LAST MODIFIED : 8 December 1994

DESCRIPTION :
Define the global origin and orientation for the polhemus so that all stations
will be able to be reset to the same values.
==============================================================================*/
{

	matrix_copy(&global_direction,direction);
} /* input_module_polhemus_reset_dir */

void input_module_polhemus_revert_pos(double *position)
/*******************************************************************************
LAST MODIFIED : 8 December 1994

DESCRIPTION :
Read the global origin and orientation for the polhemus so that all stations
will be able to be reset to the same values.
==============================================================================*/
{
	int i;

	for (i=0;i<3;i++)
	{
		position[i]=global_position[i];
	}
} /* input_module_polhemus_revert_pos */

void input_module_polhemus_revert_dir(Gmatrix *direction)
/*******************************************************************************
LAST MODIFIED : 8 December 1994

DESCRIPTION :
Read the global origin and orientation for the polhemus so that all stations
will be able to be reset to the same values.
==============================================================================*/
{

	matrix_copy(direction,&global_direction);
} /* input_module_polhemus_revert_dir */
#endif /* EXT_INPUT */

#endif
