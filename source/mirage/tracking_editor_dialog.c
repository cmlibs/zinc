/*******************************************************************************
FILE : tracking_editor_dialog.c

LAST MODIFIED : 28 April 2000

DESCRIPTION :
Source code for the tracking editor dialog box.
==============================================================================*/

#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/signal.h>
#include <sys/time.h>
#if defined (SGI)
/* SAB  The alternative probably works on the SGI too
	but I don't want to break it at the moment */
#include <stropts.h>
#include <poll.h>
#else /* defined (SGI) */
#include <sys/time.h>
#include <sys/types.h>
#endif /* defined (SGI) */
#include <unistd.h>

#include <Xm/DialogS.h>
#include <Xm/MwmUtil.h>
#include <Xm/TextF.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/Form.h>
#include <Xm/Protocols.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include "finite_element/finite_element.h"
#include "general/debug.h"
#include "general/object.h"
#include "general/photogrammetry.h"
#include "graphics/graphics_window.h"
#include "graphics/light.h"
#include "graphics/light_model.h"
#include "graphics/material.h"
#include "graphics/spectrum.h"
#include "mirage/digitiser_window.h"
#include "mirage/movie.h"
#include "mirage/movie_data.h"
#include "mirage/tracking_editor_data.h"
#include "mirage/tracking_editor_dialog.h"
#include "mirage/tracking_editor_dialog.uidh"
#include "three_d_drawing/ThreeDDraw.h"
#include "user_interface/confirmation.h"
#include "user_interface/filedir.h"
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

#define bc_scale_width 40 /* a multiple of bc_min_line_spacing */
#define bc_scale_height 25
#define bc_min_line_spacing 4
#define bc_tick_length 4
#define XVG_SOCKET_COMMAND_LEN 4
/* buffer for temporary use - not guaranteed to be unchanged
	after function calls */
#define BUFFER_SIZE 256

/*
Module variables
----------------
*/
static int tracking_editor_dialog_hierarchy_open = 0;
static MrmHierarchy tracking_editor_dialog_hierarchy;
static char buff[BUFFER_SIZE];

/*
Module Types
------------
*/

enum Tracking_editor_tracking_tolerance
/*******************************************************************************
LAST MODIFIED : 13 May 1998

DESCRIPTION :
==============================================================================*/
{
	LOW_TOLERANCE,
	MEDIUM_TOLERANCE,
	HIGH_TOLERANCE
}; /* Tracking_editor_tracking_tolerance */

struct Tracking_editor_dialog
/*******************************************************************************
LAST MODIFIED : 21 March 2000

DESCRIPTION :
==============================================================================*/
{
	struct Tracking_editor_dialog **address;
	Widget file_menu,edit_menu,view_2d_points,view_2d_lines,view_2d_surfaces,
		view_3d_points,view_3d_lines,view_3d_surfaces,view_node_numbers,
		frame_form,frame_text,frame_range,control_form,
		control_mode_rowcol,process_button,abort_button,drawing_form,
		track_button,backtrack_button,substitute_button,interpolate_button;
	Widget dialog,shell;

	struct Mirage_movie *mirage_movie;

	enum Tracking_editor_control_mode control_mode, previous_control_mode;

	/* Settings specific to the different control modes */
	/* Tracking settings */
	enum Tracking_editor_tracking_tolerance tracking_tolerance;
	int tracking_search_radius;
	int tracking_port;

   /* The socket processes input callback id */
	XtInputId input_id;

   /* The kill process timeout callback id */
	XtIntervalId kill_process_interval_id;
	int kill_attempts;

	/* following needed to remotely run and control processes */
	char host[BUFFER_SIZE],remote_host[BUFFER_SIZE];
	int processing,process_socket,process_remote_client,process_ID;

	/* following needed for creating node groups/graphics */
	struct Colour *background_colour;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct MANAGER(Digitiser_window) *digitiser_window_manager;
	struct MANAGER(FE_basis) *basis_manager;
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(GROUP(FE_element)) *element_group_manager;
	struct MANAGER(FE_field) *fe_field_manager;
	struct LIST(GT_object) *glyph_list;
	struct MANAGER(Graphical_material) *graphical_material_manager;
	struct Graphical_material *default_graphical_material;
	struct MANAGER(Light) *light_manager;
	struct Light *default_light;
	struct MANAGER(Light_model) *light_model_manager;
	struct Light_model *default_light_model;
	struct MANAGER(FE_node) *node_manager;
	void *node_manager_callback_id;
	struct MANAGER(Graphics_window) *graphics_window_manager;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
	void *node_group_manager_callback_id;
	struct MANAGER(FE_node) *data_manager;
	struct MANAGER(GROUP(FE_node)) *data_group_manager;
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct FE_element_selection *element_selection;
	struct FE_node_selection *node_selection;
	struct FE_node_selection *data_selection;
	struct MANAGER(Scene) *scene_manager;
	struct Scene *default_scene;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct Spectrum *default_spectrum;
	struct MANAGER(Texture) *texture_manager;
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct User_interface *user_interface;

	/* bar chart display parameters: */
	Widget drawing_widget;
	int bc_left_limit,bc_right_limit,bc_top_limit,bc_bottom_limit;
	double bc_left,bc_top,bc_pixels_per_unit_x,bc_pixels_per_unit_y;
}; /* struct Tracking_editor_dialog */

/*
Module functions
----------------
*/

DECLARE_DIALOG_IDENTIFY_FUNCTION(tracking_editor, \
	Tracking_editor_dialog,file_menu)
DECLARE_DIALOG_IDENTIFY_FUNCTION(tracking_editor, \
	Tracking_editor_dialog,edit_menu)
DECLARE_DIALOG_IDENTIFY_FUNCTION(tracking_editor, \
	Tracking_editor_dialog,view_2d_points)
DECLARE_DIALOG_IDENTIFY_FUNCTION(tracking_editor, \
	Tracking_editor_dialog,view_2d_lines)
DECLARE_DIALOG_IDENTIFY_FUNCTION(tracking_editor, \
	Tracking_editor_dialog,view_2d_surfaces)
DECLARE_DIALOG_IDENTIFY_FUNCTION(tracking_editor, \
	Tracking_editor_dialog,view_3d_points)
DECLARE_DIALOG_IDENTIFY_FUNCTION(tracking_editor, \
	Tracking_editor_dialog,view_3d_lines)
DECLARE_DIALOG_IDENTIFY_FUNCTION(tracking_editor, \
	Tracking_editor_dialog,view_3d_surfaces)
DECLARE_DIALOG_IDENTIFY_FUNCTION(tracking_editor, \
	Tracking_editor_dialog,view_node_numbers)
DECLARE_DIALOG_IDENTIFY_FUNCTION(tracking_editor, \
	Tracking_editor_dialog,frame_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(tracking_editor, \
	Tracking_editor_dialog,frame_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(tracking_editor, \
	Tracking_editor_dialog,frame_range)
DECLARE_DIALOG_IDENTIFY_FUNCTION(tracking_editor, \
	Tracking_editor_dialog,control_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(tracking_editor, \
	Tracking_editor_dialog,control_mode_rowcol)
DECLARE_DIALOG_IDENTIFY_FUNCTION(tracking_editor, \
	Tracking_editor_dialog,track_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(tracking_editor, \
	Tracking_editor_dialog,backtrack_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(tracking_editor, \
	Tracking_editor_dialog,substitute_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(tracking_editor, \
	Tracking_editor_dialog,interpolate_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(tracking_editor, \
	Tracking_editor_dialog,drawing_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(tracking_editor, \
	Tracking_editor_dialog,process_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(tracking_editor, \
	Tracking_editor_dialog,abort_button)

static int tracking_editor_exit_process_mode(
	struct Tracking_editor_dialog *track_ed);
/*******************************************************************************
LAST MODIFIED : 26 June 2000

DESCRIPTION :
Prototype declaration.
==============================================================================*/

static void tracking_editor_kill_process_timeout_cb(XtPointer track_ed_void,
	XtIntervalId *interval_id)
/*******************************************************************************
LAST MODIFIED : 26 June 2000

DESCRIPTION :
Callback set up by XtAppAddTimeout.  If this timeout is called then the kill
didn't work in the expected time.  For five attempts this routine will attempt
to kill the process again.
==============================================================================*/
{
	struct Tracking_editor_dialog *track_ed;

	ENTER(tracking_editor_kill_process_timeout_cb);
	USE_PARAMETER(interval_id);
	if (track_ed=(struct Tracking_editor_dialog *)track_ed_void)
	{
		if (track_ed->remote_host&&track_ed->process_ID)
		{ 
			if (track_ed->kill_attempts < 5)
			{
				track_ed->kill_attempts++;
				printf("Attempt %d killing process %i on remote host %s\n",
					track_ed->kill_attempts, track_ed->process_ID, track_ed->remote_host);
				/* Also kill in tracking_editor_kill_process */
				sprintf(buff, "rsh %s kill -INT %d",
					track_ed->remote_host, track_ed->process_ID);
				system(buff);

				/* Add timeout callback so we can try again if process doesn't respond */
				track_ed->kill_process_interval_id = XtAppAddTimeOut(
					track_ed->user_interface->application_context, /*milliseconds*/5000,
					tracking_editor_kill_process_timeout_cb, (XtPointer)track_ed);
			}
			else
			{
				display_message(ERROR_MESSAGE, "tracking_editor_kill_process_timeout_cb.  "
					"No reponse from remote process after five attempts.  Ignoring process.");
				track_ed->processing=0;
				track_ed->process_ID=0;
				if (track_ed->mirage_movie)
				{
					Mirage_movie_refresh_node_groups(track_ed->mirage_movie);
					/* must read frame in case it was one changed */
					Mirage_movie_read_frame_nodes(track_ed->mirage_movie,
						track_ed->mirage_movie->current_frame_no);
				}
				tracking_editor_exit_process_mode(track_ed);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "tracking_editor_kill_process_timeout_cb.  "
				"Callback enabled but no process ID or host.");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "tracking_editor_kill_process_timeout_cb.  "
			"Invalid argument(s)");
	}
	LEAVE;

} /* tracking_editor_kill_process_timeout_cb */

static void tracking_editor_kill_process(struct Tracking_editor_dialog *track_ed)
/*******************************************************************************
LAST MODIFIED : 26 June 2000

DESCRIPTION :
Kills the process running on the remote host.
==============================================================================*/
{
	if (track_ed&&track_ed->remote_host&&track_ed->process_ID)
	{
		printf("Killing process %i on remote host %s\n",track_ed->process_ID,
			track_ed->remote_host);
		/* Also kill in tracking_editor_kill_process_timeout_cb */
		sprintf(buff, "rsh %s kill -INT %d",
			track_ed->remote_host,track_ed->process_ID);
#if defined (OLD_CODE)
		/* Only reset this when the confirmation comes through the socket or we give up */
		track_ed->process_ID=0;
#endif /* defined (OLD_CODE) */
		system(buff);

		track_ed->kill_attempts = 1;

		/* Add timeout callback so we can try again if process doesn't respond */
		track_ed->kill_process_interval_id = XtAppAddTimeOut(
			track_ed->user_interface->application_context, /*milliseconds*/5000,
			tracking_editor_kill_process_timeout_cb, (XtPointer)track_ed);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_kill_process.  Invalid argument(s)");
	}
}

static int getPortFromProtocol(char *protocol_name)
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct servent *serv;

	/* fetch the port to connect to */
	if ((serv = getservbyname(protocol_name, NULL)) == NULL)
	{
		display_message(ERROR_MESSAGE,"getPortFromProtocol.  "
			"Failed to find %s service in /etc/services in getservbyname()",protocol_name);
		return_code = 0;
	}
	else
	{
		return_code = ntohs(serv->s_port);
	}

	return (return_code);
}

static int SocketInitialize(int *theSocket, int port)
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION :
==============================================================================*/
{
	struct sockaddr_in socketDescriptor;
	int optionValue;


	/* Create the (internet, stream) socket */
	if ((*theSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
		perror("socket() ");
		return(0);
	}
	printf("SocketInitialize() : Created socket.\n");

#if defined (OLD_CODE)
	/* Indicate that socket re-use is allowed */
	optionValue = 1;
	if (setsockopt(*theSocket, SOL_SOCKET, SO_REUSEADDR, &optionValue, sizeof(optionValue)) == -1 ) {
		perror("setsockopt() ");
		close(*theSocket);
		return(0);
	}
#endif /* defined (OLD_CODE) */

	/* Bind this socket to all incoming addresses on the port */
	memset(&socketDescriptor, 0, sizeof(socketDescriptor));
	socketDescriptor.sin_family      = AF_INET; /* Protocol Family */
	socketDescriptor.sin_port        = htons(port); /* Port */
	socketDescriptor.sin_addr.s_addr = INADDR_ANY; /* listen to all */
#if defined (OLD_CODE)
	/* The port number (so that it is actually 2000) needs to be converted
		through htons and back again! */
	socketDescriptor.sin_port        = htons(serv->s_port); /* Port */
	socketDescriptor.sin_addr.s_addr = htonl(INADDR_ANY); /* listen to all */
#endif /* defined (OLD_CODE) */
	if (bind(*theSocket,(struct sockaddr *)&socketDescriptor,
		sizeof(struct sockaddr)) == -1 )
	{
		perror("bind() ");
		close(*theSocket);
		return(0);
	}

	/* Indicate willingness to accept connections */
	if (listen(*theSocket, 1) == -1 ) {
		perror("listen() ");
		close(*theSocket);
		return(0);
	}

	/* return success */
	return(1);
}

static int tracking_editor_send_message(int remoteClient,char *com,char *args,
	int n)
/*******************************************************************************
LAST MODIFIED : 8 April 1998

DESCRIPTION :
Writes a message to a socket in XVG format. Originally from Paul Charette.
==============================================================================*/
{
	int return_code;

	ENTER(tracking_editor_send_message);
	if (com&&(0<=n)&&((0==n)||args))
	{
		/* write command */
		if (write(remoteClient,com,XVG_SOCKET_COMMAND_LEN)==XVG_SOCKET_COMMAND_LEN)
		{
			/* write number of args */
			if (return_code=(write(remoteClient,&n,sizeof(int))==sizeof(int)))
			{
				/* write args if required */
				if (0<n)
				{
					return_code=(write(remoteClient,args,n) == n);
				}
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"tracking_editor_send_message.  Could not write");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_send_message.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* tracking_editor_send_message */

static int tracking_editor_get_message(int remoteClient,char *InCom,
	char **args)
/*******************************************************************************
LAST MODIFIED : 8 April 1998

DESCRIPTION :
Reads a message from a socket in XVG format. Originally from Paul Charette.
Returns a 4 letter command (eg. "DONE") in InCom, and a string of other
arguments in <args>, or NULL if there are none.
It is up to the calling routine to DEALLOCATE the returned args!
???RC Yet to be converted to cmgui format!
==============================================================================*/
{
	char arg_lengthc[4];
	int arg_length,return_code;

	ENTER(tracking_editor_get_message);
	if (InCom&&args)
	{
		return_code=1;
		/* read 4 byte command */
		if (read(remoteClient, InCom,XVG_SOCKET_COMMAND_LEN) == -1)
		{
			perror("read(command) ");
			return(0);
		}
		InCom[XVG_SOCKET_COMMAND_LEN] = '\0';

		/* read in the length of the arguments to follow */
		if (read(remoteClient,arg_lengthc,4) == -1)
		{
			perror("read(ArgsLen) ");
			return(0);
		}
		arg_length = (arg_lengthc[3] << 24) + (arg_lengthc[2] << 16) +
			(arg_lengthc[1] << 8) + arg_lengthc[0];
		printf(" Arg %s arg_length %d\n", InCom, arg_length);

		/* if the ArgsLen count is non-zero, read in the arguments */
		if (0<arg_length)
		{
		  if (arg_length > 100000)
		  {
			  display_message(ERROR_MESSAGE,
				  "tracking_editor_get_message.  Very large argument space %d requested by xvg with message %s",
				  arg_length, InCom);
			  arg_length = 100000;
		  }
		  if (ALLOCATE(*args,char,arg_length+1))
		  {
			  /* read in the data */
			  if (read(remoteClient,*args,arg_length) == -1)
			  {
				  DEALLOCATE(*args);
				  perror("read(args) ");
				  return(0);
			  }
			}
			else
			{
				tracking_editor_send_message(remoteClient,"NOK ",
					"Error allocating memory",strlen("Error allocating memory")+1);
				return(0);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_get_message.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* tracking_editor_get_message */

static int Node_status_is_at_index(struct Node_status *node_status,
	void *index_void)
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
Setting index to a positive number, this routine returns true once it has
been called index times, effectively returning the node_status at index when
called using FIRST_OBJECT_IN_LIST_THAT().
==============================================================================*/
{
	int return_code,*index;

	ENTER(Node_status_is_at_index);
	if (node_status&&(index=(int *)index_void))
	{
		return_code=(0 == *index);
		(*index)--;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_is_at_index */

struct Tracking_editor_draw_status_bar_data
{
	double bar_height,bar_z;
	int first_frame,last_frame,index,min_index,max_index;
	int nodes_per_line,nodes_per_label,full_left;
}; /* Tracking_editor_draw_status_bar_data */

static int tracking_editor_draw_node_status_bar(struct Node_status *node_status,
	void *draw_data_void)
/*******************************************************************************
LAST MODIFIED : 6 April 1998

DESCRIPTION :
Draws the bar chart if there is a movie.
==============================================================================*/
{
	double bar_left,bar_right,bar_bottom,bar_top,bar_z;
	int return_code,start,stop,number_of_ranges,range_no;
	struct Tracking_editor_draw_status_bar_data *draw_data;

	ENTER(tracking_editor_draw_node_status_bar);
	if (node_status&&
		(draw_data=(struct Tracking_editor_draw_status_bar_data *)draw_data_void))
	{
		return_code=1;
		/* only draw the visible status bars */
		if ((draw_data->index >= draw_data->min_index)&&
			(draw_data->index <= draw_data->max_index))
		{
			bar_bottom=(double)(draw_data->index);
			bar_top=bar_bottom+draw_data->bar_height;
			bar_z=draw_data->bar_z;
			if (0<(number_of_ranges=Node_status_get_number_of_ranges(node_status)))
			{
				glBegin(GL_QUADS);
				for (range_no=0;return_code&&(range_no<number_of_ranges);range_no++)
				{
					if (return_code=
						Node_status_get_range(node_status,range_no,&start,&stop))
					{
						if ((start<=draw_data->last_frame)&&(stop>=draw_data->first_frame))
						{
							bar_left=(double)start;
							bar_right=(double)(stop+1);
							glVertex3d(bar_left,bar_top,bar_z);
							glVertex3d(bar_right,bar_top,bar_z);
							glVertex3d(bar_right,bar_bottom,bar_z);
							glVertex3d(bar_left,bar_bottom,bar_z);
						}
					}
				}
				glEnd();
			}
		}
		draw_data->index++;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_draw_node_status_bar.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* tracking_editor_draw_node_status_bar */

static int tracking_editor_draw_node_lines(struct Node_status *node_status,
	void *draw_data_void)
/*******************************************************************************
LAST MODIFIED : 6 April 1998

DESCRIPTION :
Draws lines between the vertical scale and the node status bars.
==============================================================================*/
{
	int return_code;
	struct Tracking_editor_draw_status_bar_data *draw_data;

	ENTER(tracking_editor_draw_node_lines);
	if (node_status&&
		(draw_data=(struct Tracking_editor_draw_status_bar_data *)draw_data_void))
	{
		return_code=1;
		/* only draw the visible status bars */
		if ((draw_data->index >= draw_data->min_index)&&
			(draw_data->index <= draw_data->max_index)&&
			(draw_data->last_frame > draw_data->full_left))
		{
			if (0==(draw_data->index % draw_data->nodes_per_line))
			{
				if (0==(draw_data->index % draw_data->nodes_per_label))
				{
					glColor3f(1,1,1);
				}
				else
				{
					glColor3f(0.5,0.5,0.5);
				}
				glVertex2i(draw_data->full_left,draw_data->index);
				glVertex2i(draw_data->last_frame,draw_data->index);
			}
		}
		draw_data->index++;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_draw_node_lines.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* tracking_editor_draw_node_lines */

static int tracking_editor_draw_node_scale_lines(
	struct Node_status *node_status,
	void *draw_data_void)
/*******************************************************************************
LAST MODIFIED : 6 April 1998

DESCRIPTION :
Draws lines between the vertical scale and the node status bars.
==============================================================================*/
{
	int return_code,tick_length;
	struct Tracking_editor_draw_status_bar_data *draw_data;

	ENTER(tracking_editor_draw_node_scale_lines);
	if (node_status&&
		(draw_data=(struct Tracking_editor_draw_status_bar_data *)draw_data_void))
	{
		return_code=1;
		/* only draw the visible status bars */
		if ((draw_data->index >= draw_data->min_index)&&
			(draw_data->index <= draw_data->max_index))
		{
			if (0==(draw_data->index % draw_data->nodes_per_line))
			{
				if (0==(draw_data->index % draw_data->nodes_per_label))
				{
					tick_length=bc_scale_width;
				}
				else
				{
					tick_length=bc_tick_length;
				}
				glVertex2i(-tick_length,draw_data->index);
				glVertex2i(0,draw_data->index);
			}
		}
		draw_data->index++;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_draw_node_scale_lines.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* tracking_editor_draw_node_scale_lines */

static int tracking_editor_draw_node_numbers(
	struct Node_status *node_status,void *draw_data_void)
/*******************************************************************************
LAST MODIFIED : 6 April 1998

DESCRIPTION :
==============================================================================*/
{
	static char tmp_string[20];
	int return_code;
	struct Tracking_editor_draw_status_bar_data *draw_data;

	ENTER(tracking_editor_draw_node_numbers);
	if (node_status&&
		(draw_data=(struct Tracking_editor_draw_status_bar_data *)draw_data_void))
	{
		return_code=1;
		/* only draw the visible status bars */
		if ((draw_data->index >= draw_data->min_index)&&
			(draw_data->index <= draw_data->max_index))
		{
			if (0==(draw_data->index % draw_data->nodes_per_label))
			{
				glRasterPos2d(-bc_scale_width+bc_tick_length,draw_data->index+0.35);
				sprintf(tmp_string,"%i",Node_status_get_node_no(node_status));
				wrapperPrintText(tmp_string);
			}
		}
		draw_data->index++;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_draw_node_numbers.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* tracking_editor_draw_node_numbers */

static int tracking_editor_draw_bar_chart(
	struct Tracking_editor_dialog *track_ed)
/*******************************************************************************
LAST MODIFIED : 6 April 1998

DESCRIPTION :
Draws the bar chart if there is a movie.
==============================================================================*/
{
	static char tmp_string[20];
	Dimension width,height;
	double bc_right,bc_bottom;
	GLint gl_width,gl_height;
	int return_code,frames_per_line,frame_no,tick_length,frames_per_label,
		nodes_per_line,nodes_per_label;
	int full_left,full_right,full_bottom,full_top;
	struct Mirage_movie *movie;
	struct Tracking_editor_draw_status_bar_data draw_data;

	ENTER(tracking_editor_draw_bar_chart);
	if (track_ed)
	{
		return_code=1;
		XtVaGetValues(track_ed->drawing_widget,
			XmNwidth,&width,XmNheight,&height,NULL);
		glDepthRange((GLclampd)0,(GLclampd)1);
		glEnable(GL_DEPTH_TEST);
		glViewport(0,0,width,height);
		/* clear the screen: colour buffer and depth buffer */
		glClearColor(0,0,0,0);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (movie=track_ed->mirage_movie)
		{
			gl_width=(GLint)(width-bc_scale_width);
			gl_height=(GLint)(height-bc_scale_height);
			if ((0<gl_width)&&(0<gl_height))
			{
				glViewport(bc_scale_width,0,gl_width,gl_height);
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				bc_right=track_ed->bc_left+gl_width/track_ed->bc_pixels_per_unit_x;
				bc_bottom=track_ed->bc_top-gl_height/track_ed->bc_pixels_per_unit_y;
				glOrtho(track_ed->bc_left,bc_right,bc_bottom,track_ed->bc_top,
					-1.0,1.0);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();

				/* minimise the amount of redrawn space */
				full_left=draw_data.first_frame=(int)track_ed->bc_left;
				full_right=draw_data.last_frame=(int)bc_right+1;
				full_bottom=draw_data.min_index=(int)bc_bottom;
				full_top=draw_data.max_index=(int)track_ed->bc_top+1;
				if (draw_data.first_frame < track_ed->bc_left_limit)
				{
					draw_data.first_frame=track_ed->bc_left_limit;
				}
				if (draw_data.last_frame > track_ed->bc_right_limit)
				{
					draw_data.last_frame=track_ed->bc_right_limit;
				}
				if (draw_data.min_index < track_ed->bc_bottom_limit)
				{
					draw_data.min_index=track_ed->bc_bottom_limit;
				}
				if (draw_data.max_index > track_ed->bc_top_limit)
				{
					draw_data.max_index=track_ed->bc_top_limit;
				}

				draw_data.bar_height=1.0;
				if (bc_min_line_spacing <= track_ed->bc_pixels_per_unit_y)
				{
					draw_data.bar_height=0.875;
				}
				draw_data.bar_z=0.1;
				draw_data.index=0;
				glColor3f(0,1,0);
				FOR_EACH_OBJECT_IN_LIST(Node_status)(
					tracking_editor_draw_node_status_bar,(void *)&draw_data,
					movie->placed_list);

				if (bc_min_line_spacing <= track_ed->bc_pixels_per_unit_y)
				{
					draw_data.bar_height=0.75;
				}
				draw_data.bar_z=0.2;
				draw_data.index=0;
				glColor3f(1,0,0);
				FOR_EACH_OBJECT_IN_LIST(Node_status)(
					tracking_editor_draw_node_status_bar,(void *)&draw_data,
					movie->problem_list);

				draw_data.bar_z=0.3;
				if (bc_min_line_spacing <= track_ed->bc_pixels_per_unit_y)
				{
					draw_data.bar_height=0.625;
				}
				draw_data.index=0;
				glColor3f(1,1,0);
				FOR_EACH_OBJECT_IN_LIST(Node_status)(
					tracking_editor_draw_node_status_bar,(void *)&draw_data,
					movie->pending_list);


				/* draw horizontal lines up to node status bars */
				nodes_per_line=1;
				while (nodes_per_line*track_ed->bc_pixels_per_unit_y <
					bc_min_line_spacing)
				{
					nodes_per_line *= 5;
				}
				nodes_per_label=nodes_per_line;
				while (nodes_per_label*track_ed->bc_pixels_per_unit_y <
					bc_scale_height)
				{
					nodes_per_label *= 5;
				}
				draw_data.nodes_per_line=nodes_per_line;
				draw_data.nodes_per_label=nodes_per_label;
				draw_data.full_left=full_left;
				draw_data.bar_z=0.0;
				draw_data.index=0;
				glBegin(GL_LINES);
				FOR_EACH_OBJECT_IN_LIST(Node_status)(
					tracking_editor_draw_node_lines,(void *)&draw_data,
					movie->placed_list);
				glEnd();

				/* draw vertical lines to demark frames */
				frames_per_line=1;
				while (frames_per_line*track_ed->bc_pixels_per_unit_x <
					bc_min_line_spacing)
				{
					frames_per_line *= 10;
				}
				frames_per_label=frames_per_line;
				while (frames_per_label*track_ed->bc_pixels_per_unit_x <
					bc_scale_width)
				{
					frames_per_label *= 10;
				}
				glBegin(GL_LINES);
				for (frame_no=draw_data.first_frame;frame_no<=draw_data.last_frame;
					frame_no++)
				{
					if (0==(frame_no % frames_per_line))
					{
						if ((0==(frame_no % frames_per_label))||
							(5==((frame_no/frames_per_line) % 10)))
						{
							glColor3f(1,1,1);
						}
						else
						{
							glColor3f(0.5,0.5,0.5);
						}
						glVertex2i(frame_no,draw_data.min_index);
						glVertex2i(frame_no,full_top);
					}
				}
				glEnd();

				/* draw the frame scale */
				glColor3f(1,1,1);
				glViewport(bc_scale_width,gl_height,gl_width,bc_scale_height);
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glOrtho(track_ed->bc_left,bc_right,0,bc_scale_height,-1.0,1.0);
				glBegin(GL_LINES);
				glVertex2i(full_left,0.5);
				glVertex2i(full_right,0.5);
				for (frame_no=full_left;frame_no<=full_right;frame_no++)
				{
					if (0==(frame_no % frames_per_line))
					{
						if (0==(frame_no % frames_per_label))
						{
							tick_length=bc_scale_height;
						}
						else
						{
							if (5==((frame_no/frames_per_line) % 10))
							{
								tick_length=2*bc_tick_length;
							}
							else
							{
								tick_length=bc_tick_length;
							}
						}
						glVertex2i(frame_no,0);
						glVertex2i(frame_no,tick_length);
					}
				}
				/* draw coloured indication of current frame */
				glColor3f(0,5.5,1.0);
				glVertex3f(movie->current_frame_no+0.25,bc_tick_length,0.5);
				glVertex3f(movie->current_frame_no+0.50,0.0,0.5);
				glVertex3f(movie->current_frame_no+0.50,0.0,0.5);
				glVertex3f(movie->current_frame_no+0.75,bc_tick_length,0.5);
				glVertex3f(movie->current_frame_no+0.25,bc_tick_length,0.5);
				glVertex3f(movie->current_frame_no+0.75,bc_tick_length,0.5);
				glEnd();
				glRasterPos3f(movie->current_frame_no+0.25,bc_tick_length+2,0.5);
				sprintf(tmp_string,"%i",movie->current_frame_no);
				wrapperPrintText(tmp_string);
				glColor3f(1,1,1);
				for (frame_no=full_left;frame_no<=full_right;frame_no++)
				{
					if (0==(frame_no % frames_per_label))
					{
						glRasterPos2f(frame_no+0.25,bc_tick_length+2);
						sprintf(tmp_string,"%i",frame_no);
						wrapperPrintText(tmp_string);
					}
				}

				glViewport(0,0,bc_scale_width,gl_height);
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glOrtho(-bc_scale_width,0,bc_bottom,track_ed->bc_top,-1.0,1.0);
				glBegin(GL_LINES);
				glVertex2i(-0.5,full_bottom);
				glVertex2i(-0.5,full_top);
				/* show the node numbers up the vertical scale */
				draw_data.nodes_per_line=nodes_per_line;
				draw_data.nodes_per_label=nodes_per_label;
				draw_data.full_left=full_left;
				draw_data.bar_z=0.0;
				draw_data.index=0;
				glColor3f(1,1,1);
				FOR_EACH_OBJECT_IN_LIST(Node_status)(
					tracking_editor_draw_node_scale_lines,(void *)&draw_data,
					movie->placed_list);
				glEnd();
				draw_data.index=0;
				FOR_EACH_OBJECT_IN_LIST(Node_status)(
					tracking_editor_draw_node_numbers,(void *)&draw_data,
					movie->placed_list);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_draw_bar_chart.  Missing track_ed");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* tracking_editor_draw_bar_chart */

static int tracking_editor_update_bar_chart(
	struct Tracking_editor_dialog *track_ed)
/*******************************************************************************
LAST MODIFIED : 6 April 1998

DESCRIPTION :
Redraws the bar chart with MakeCurrent and SwapBuffer instructions.
Except for in the expose callback, to draw the chart this routine and not
tracking_editor_draw_bar_chart should be called.
==============================================================================*/
{
	int return_code;

	ENTER(tracking_editor_update_bar_chart);
	/* checking arguments */
	if (track_ed)
	{
		X3dThreeDDrawingMakeCurrent(track_ed->drawing_widget);
		return_code=tracking_editor_draw_bar_chart(track_ed);
		X3dThreeDDrawingSwapBuffers();
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_update_bar_chart.  Missing tracking editor dialog");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* tracking_editor_update_bar_chart */

static int tracking_editor_node_pending_change(
	struct Tracking_editor_dialog *track_ed,int node_no)
/*******************************************************************************
LAST MODIFIED : 3 April 1998

DESCRIPTION :
Call after changing the pending status to make the pending node groups up to
date at the current_frame_no of the movie.
==============================================================================*/
{
	int return_code,placed,pending,view_no;
	struct FE_node *node;
	struct Node_status *node_status;
	struct Mirage_movie *movie;
	struct Mirage_view *view;

	ENTER(tracking_editor_node_pending_change);
	if (track_ed&&(movie=track_ed->mirage_movie))
	{
		return_code=1;
		if (node_status=FIND_BY_IDENTIFIER_IN_LIST(Node_status,node_no)(
			node_no,movie->placed_list))
		{
			placed=
				Node_status_is_value_in_range(node_status,movie->current_frame_no);
		}
		else
		{
			return_code=0;
		}
		if (node_status=FIND_BY_IDENTIFIER_IN_LIST(Node_status,node_no)(
			node_no,movie->pending_list))
		{
			pending=
				Node_status_is_value_in_range(node_status,movie->current_frame_no);
		}
		else
		{
			return_code=0;
		}
		if (return_code)
		{
			/* make sure node is in pending groups for each placed view/3-D */
			if (node=FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
				node_no,movie->placed_nodes_3d))
			{
				if (FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
					node_no,movie->pending_nodes_3d))
				{
					if (!pending)
					{
						REMOVE_OBJECT_FROM_GROUP(FE_node)(node,movie->pending_nodes_3d);
					}
				}
				else
				{
					if (pending)
					{
						ADD_OBJECT_TO_GROUP(FE_node)(node,movie->pending_nodes_3d);
					}
				}
			}
			for (view_no=0;view_no<movie->number_of_views;view_no++)
			{
				if (view=movie->views[view_no])
				{
					if (node=FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
						node_no,view->placed_nodes))
					{
						if (FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
							node_no,view->pending_nodes))
						{
							if (!pending)
							{
								REMOVE_OBJECT_FROM_GROUP(FE_node)(node,view->pending_nodes);
							}
						}
						else
						{
							if (pending)
							{
								ADD_OBJECT_TO_GROUP(FE_node)(node,view->pending_nodes);
							}
						}
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_node_pending_change.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* tracking_editor_node_pending_change */

static int tracking_editor_iterator_node_pending_change(
	struct Node_status *node_status,void *track_ed_void)
/*******************************************************************************
LAST MODIFIED : 28 April 1998

DESCRIPTION :
Node_status iterator for calling tracking_editor_node_pending_change.
Must cache all pending node groups (3-D and in each view) before calling
as an iterator.
==============================================================================*/
{
	int return_code;
	struct Tracking_editor_dialog *track_ed;

	ENTER(tracking_editor_iterator_node_pending_change);
	if (node_status&&(track_ed=(struct Tracking_editor_dialog *)track_ed_void))
	{
		return_code=tracking_editor_node_pending_change(
			track_ed,Node_status_get_node_no(node_status));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_iterator_node_pending_change.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* tracking_editor_iterator_node_pending_change */

static int tracking_editor_refresh_pending_groups(
	struct Tracking_editor_dialog *track_ed)
/*******************************************************************************
LAST MODIFIED : 28 April 1998

DESCRIPTION :
Makes the pending node groups reflect the ranges in the pending lists for the
current frame_no of the movie. Takes care of cacheing groups for efficiency.
==============================================================================*/
{
	int return_code,view_no;
	struct Mirage_movie *movie;
	struct Mirage_view *view;

	ENTER(tracking_editor_iterator_node_pending_change);
	if (track_ed&&(movie=track_ed->mirage_movie))
	{
		MANAGED_GROUP_BEGIN_CACHE(FE_node)(movie->pending_nodes_3d);
		for (view_no=0;view_no<movie->number_of_views;view_no++)
		{
			if (view=movie->views[view_no])
			{
				MANAGED_GROUP_BEGIN_CACHE(FE_node)(view->pending_nodes);
			}
		}
		FOR_EACH_OBJECT_IN_LIST(Node_status)(
			tracking_editor_iterator_node_pending_change,
			(void *)track_ed,movie->placed_list);
		for (view_no=0;view_no<movie->number_of_views;view_no++)
		{
			if (view=movie->views[view_no])
			{
				MANAGED_GROUP_END_CACHE(FE_node)(view->pending_nodes);
			}
		}
		MANAGED_GROUP_END_CACHE(FE_node)(movie->pending_nodes_3d);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_refresh_pending_groups.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* tracking_editor_refresh_pending_groups */

static int tracking_editor_node_problem_change(
	struct Tracking_editor_dialog *track_ed,int node_no)
/*******************************************************************************
LAST MODIFIED : 3 April 1998

DESCRIPTION :
Call after changing the problem status to make the problem node groups up to
date at the current_frame_no of the movie.
==============================================================================*/
{
	int return_code,placed,problem,view_no;
	struct FE_node *node;
	struct Node_status *node_status;
	struct Mirage_movie *movie;
	struct Mirage_view *view;

	ENTER(tracking_editor_node_problem_change);
	if (track_ed&&(movie=track_ed->mirage_movie))
	{
		return_code=1;
		if (node_status=FIND_BY_IDENTIFIER_IN_LIST(Node_status,node_no)(
			node_no,movie->placed_list))
		{
			placed=
				Node_status_is_value_in_range(node_status,movie->current_frame_no);
		}
		else
		{
			return_code=0;
		}
		if (node_status=FIND_BY_IDENTIFIER_IN_LIST(Node_status,node_no)(
			node_no,movie->problem_list))
		{
			problem=
				Node_status_is_value_in_range(node_status,movie->current_frame_no);
		}
		else
		{
			return_code=0;
		}
		if (return_code)
		{
			/* make sure node is in problem groups for each placed view/3-D */
			if (node=FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
				node_no,movie->placed_nodes_3d))
			{
				if (FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
					node_no,movie->problem_nodes_3d))
				{
					if (!problem)
					{
						REMOVE_OBJECT_FROM_GROUP(FE_node)(node,movie->problem_nodes_3d);
					}
				}
				else
				{
					if (problem)
					{
						ADD_OBJECT_TO_GROUP(FE_node)(node,movie->problem_nodes_3d);
					}
				}
			}
			for (view_no=0;view_no<movie->number_of_views;view_no++)
			{
				if (view=movie->views[view_no])
				{
					if (node=FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
						node_no,view->placed_nodes))
					{
						if (FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
							node_no,view->problem_nodes))
						{
							if (!problem)
							{
								REMOVE_OBJECT_FROM_GROUP(FE_node)(node,view->problem_nodes);
							}
						}
						else
						{
							if (problem)
							{
								ADD_OBJECT_TO_GROUP(FE_node)(node,view->problem_nodes);
							}
						}
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_node_problem_change.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* tracking_editor_node_problem_change */

static int tracking_editor_iterator_node_problem_change(
	struct Node_status *node_status,void *track_ed_void)
/*******************************************************************************
LAST MODIFIED : 28 April 1998

DESCRIPTION :
Node_status iterator for calling tracking_editor_node_problem_change.
Must cache all problem node groups (3-D and in each view) before calling
as an iterator.
==============================================================================*/
{
	int return_code;
	struct Tracking_editor_dialog *track_ed;

	ENTER(tracking_editor_iterator_node_problem_change);
	if (node_status&&(track_ed=(struct Tracking_editor_dialog *)track_ed_void))
	{
		return_code=tracking_editor_node_problem_change(
			track_ed,Node_status_get_node_no(node_status));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_iterator_node_problem_change.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* tracking_editor_iterator_node_problem_change */

static int tracking_editor_refresh_problem_groups(
	struct Tracking_editor_dialog *track_ed)
/*******************************************************************************
LAST MODIFIED : 28 April 1998

DESCRIPTION :
Makes the problem node groups reflect the ranges in the problem lists for the
current frame_no of the movie. Takes care of cacheing groups for efficiency.
==============================================================================*/
{
	int return_code,view_no;
	struct Mirage_movie *movie;
	struct Mirage_view *view;

	ENTER(tracking_editor_iterator_node_problem_change);
	if (track_ed&&(movie=track_ed->mirage_movie))
	{
		MANAGED_GROUP_BEGIN_CACHE(FE_node)(movie->problem_nodes_3d);
		for (view_no=0;view_no<movie->number_of_views;view_no++)
		{
			if (view=movie->views[view_no])
			{
				MANAGED_GROUP_BEGIN_CACHE(FE_node)(view->problem_nodes);
			}
		}
		FOR_EACH_OBJECT_IN_LIST(Node_status)(
			tracking_editor_iterator_node_problem_change,
			(void *)track_ed,movie->placed_list);
		for (view_no=0;view_no<movie->number_of_views;view_no++)
		{
			if (view=movie->views[view_no])
			{
				MANAGED_GROUP_END_CACHE(FE_node)(view->problem_nodes);
			}
		}
		MANAGED_GROUP_END_CACHE(FE_node)(movie->problem_nodes_3d);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_refresh_problem_groups.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* tracking_editor_refresh_problem_groups */

enum Tracking_editor_select_mode
{
	TRACK_ED_SELECT_DEFAULT,
	TRACK_ED_SELECT_UNMARK,
	TRACK_ED_SELECT_BADONLY
};

static int tracking_editor_select_node_frame(
	struct Tracking_editor_dialog *track_ed,int node_no,int frame_no,
	int use_left_range,int use_right_range,
	enum Tracking_editor_select_mode select_mode)
/*******************************************************************************
LAST MODIFIED : 28 April 1998

DESCRIPTION :
This routine is to be called after a node/frame combination is selected (by
placing it in 3-D or selecting it in the bar-chart window. It then creates
pending ranges appropriate to the control mode in effect. The use_left_range
and use_right_range parameters signal whether the pending range should expand
to the left and right to the end of a bad/unplaced region.  Setting unmark
to one causes the pending ranges to be unmarked.
==============================================================================*/
{
	int return_code,placed,pending,problem,left_limit,right_limit,limit_found,
		start,stop,valid_start,valid_stop;
	struct Node_status *placed_status,*pending_status,*problem_status;
	struct Mirage_movie *movie;

	ENTER(tracking_editor_select_node_frame);
	if (track_ed&&(movie=track_ed->mirage_movie))
	{
		return_code=1;
		/* get placed, pending and problem status of node_no at frame_no */
		if (placed_status=FIND_BY_IDENTIFIER_IN_LIST(Node_status,node_no)(
			node_no,movie->placed_list))
		{
			placed=Node_status_is_value_in_range(placed_status,frame_no);
		}
		else
		{
			return_code=0;
		}
		if (pending_status=FIND_BY_IDENTIFIER_IN_LIST(Node_status,node_no)(
			node_no,movie->pending_list))
		{
			pending=Node_status_is_value_in_range(pending_status,frame_no);
		}
		else
		{
			return_code=0;
		}
		if (problem_status=FIND_BY_IDENTIFIER_IN_LIST(Node_status,node_no)(
			node_no,movie->problem_list))
		{
			problem=Node_status_is_value_in_range(problem_status,frame_no);
		}
		else
		{
			return_code=0;
		}
		
		/* If BADONLY mode don't do anything if the node isn't a problem node */
		if (return_code && (select_mode!=TRACK_ED_SELECT_BADONLY || problem))
		{
			/* get left and right limits of possible pending ranges */
			left_limit=frame_no;
			/* printf("use_left_range: %i\n",use_left_range);*/
			if (use_left_range || 
				((TRACK_MODE==track_ed->control_mode) && (TRACK_ED_SELECT_UNMARK!=select_mode))
				|| ((BACKTRACK_MODE==track_ed->control_mode) && (TRACK_ED_SELECT_UNMARK==select_mode))
				|| (INTERPOLATE_MODE==track_ed->control_mode))
			{
				switch (track_ed->control_mode)
				{
				case TRACK_MODE:
				case BACKTRACK_MODE:
				case INTERPOLATE_MODE:
				case SUBSTITUTE_MODE:
					{
						/* get left limit of bad or unplaced sequence, or start of last
							bad/unplaced sequence if node is good at this frame_no */
						limit_found=0;
						while (!limit_found)
						{
							valid_stop=Node_status_get_last_stop_value(placed_status,
								left_limit,&stop);
							valid_start=Node_status_get_last_start_value(problem_status,
								left_limit+1,&start);
							/*printf("L: Start %i(%i) : Stop %i(%i)\n",
								start,valid_start,stop,valid_stop);*/
							if (valid_start||valid_stop)
							{
								if ((!valid_stop)||(valid_start&&(start>=stop)))
								{
									stop=start-1;
								}
								if (Node_status_is_value_in_range(placed_status,stop)&&
									!Node_status_is_value_in_range(problem_status,stop))
								{
									left_limit=stop+1;
									limit_found=1;
								}
								else
								{
									left_limit=stop;
								}
							}
							else
							{
								left_limit=movie->start_frame_no;
								limit_found=1;
							}
						}
					} break;
				case MAKE_BAD_MODE:
					{
						/* get left limit of bad sequence */
						if (Node_status_get_last_start_value(placed_status,
							left_limit+1,&start))
						{
							if (Node_status_get_last_stop_value(problem_status,
								left_limit,&stop)&&(stop>start))
							{
								left_limit=stop+1;
							}
							else
							{
								left_limit=start;
							}
						}
					} break;
				case MAKE_GOOD_MODE:
					{
						/* get left limit of current bad range */
						if (Node_status_get_last_start_value(problem_status,
							left_limit+1,&start))
						{
							left_limit=start;
						}
					} break;
				default:
					{
					} break;
				}
			}
			right_limit=frame_no;
			/*printf("use_right_range: %i\n",use_right_range);*/
			if (use_right_range || 
				((BACKTRACK_MODE==track_ed->control_mode) && (TRACK_ED_SELECT_UNMARK!=select_mode))
				|| ((TRACK_MODE==track_ed->control_mode) && (TRACK_ED_SELECT_UNMARK==select_mode))
				|| (INTERPOLATE_MODE==track_ed->control_mode))
			{
				switch (track_ed->control_mode)
				{
				case TRACK_MODE:
				case BACKTRACK_MODE:
				case INTERPOLATE_MODE:
				case SUBSTITUTE_MODE:
					{
						/* get right limit of bad or unplaced sequence, or end of next
							bad/unplaced sequence if node is good at this frame_no */
						limit_found=0;
						while (!limit_found)
						{
							valid_start=Node_status_get_next_start_value(placed_status,
								right_limit,&start);
							valid_stop=Node_status_get_next_stop_value(problem_status,
								right_limit-1,&stop);
							/*printf("R: Start %i(%i) : Stop %i(%i)\n",
								start,valid_start,stop,valid_stop);*/
							if (valid_start||valid_stop)
							{
								if ((!valid_start)||(valid_stop&&(stop<=start)))
								{
									start=stop+1;
								}
								if (Node_status_is_value_in_range(placed_status,start)&&
									!Node_status_is_value_in_range(problem_status,start))
								{
									right_limit=start-1;
									limit_found=1;
								}
								else
								{
									right_limit=start;
								}
							}
							else
							{
								right_limit=movie->start_frame_no+movie->number_of_frames-1;
								limit_found=1;
							}
						}
					} break;
				case MAKE_BAD_MODE:
					{
						/* get right limit of bad sequence */
						if (Node_status_get_next_stop_value(placed_status,
							right_limit-1,&stop))
						{
							if (Node_status_get_next_start_value(problem_status,
								right_limit,&start)&&(start<stop))
							{
								right_limit=start-1;
							}
							else
							{
								right_limit=stop;
							}
						}
					} break;
				case MAKE_GOOD_MODE:
					{
						/* get right limit of current bad range */
						if (Node_status_get_next_stop_value(problem_status,
							right_limit-1,&stop))
						{
							right_limit=stop;
						}
					} break;
				default:
					{
					} break;
				}
			}
			switch (track_ed->control_mode)
			{
				case TRACK_MODE:
				{
					/* start from nearest good, placed frame before or at frame_no */
					if (placed&&(!problem))
					{
						left_limit=frame_no;
					}
					else
					{
						left_limit--;
					}
					if ( select_mode==TRACK_ED_SELECT_UNMARK )
					{
						Node_status_remove_range(pending_status,left_limit,right_limit);
					}
					else
					{
						/* left limit must be good, placed */
						if (Node_status_is_value_in_range(placed_status,left_limit)&&
							!Node_status_is_value_in_range(problem_status,left_limit))
						{
							/* if the node is already pending at the left_limit, clear the
								pending range from then on */
							if (Node_status_is_value_in_range(pending_status,left_limit))
							{
								if (Node_status_get_range_containing_value(pending_status,
									left_limit,&start,&stop))
								{
									if (stop > right_limit)
									{
										Node_status_remove_range(pending_status,right_limit+1,stop);
									}
								}
							}
							Node_status_add_range(pending_status,left_limit,right_limit);
						}
					}
				} break;
				case BACKTRACK_MODE:
				{
					/* start from nearest good, placed frame after or at frame_no */
					if (placed&&(!problem))
					{
						right_limit=frame_no;
					}
					else
					{
						right_limit++;
					}
					if ( select_mode==TRACK_ED_SELECT_UNMARK )
					{
						Node_status_remove_range(pending_status,left_limit,right_limit);
					}
					else
					{
						/* right limit must be good, placed */
						if (Node_status_is_value_in_range(placed_status,right_limit)&&
							!Node_status_is_value_in_range(problem_status,right_limit))
						{
							/* if the node is already pending at the right_limit, clear the
								pending range from then back */
							if (Node_status_is_value_in_range(pending_status,right_limit))
							{
								if (Node_status_get_range_containing_value(pending_status,
									right_limit,&start,&stop))
								{
									if (start < left_limit)
									{
										Node_status_remove_range(pending_status,start,left_limit-1);
									}
								}
							}
							Node_status_add_range(pending_status,left_limit,right_limit);
						}
					}
				} break;
				case SUBSTITUTE_MODE:
				{
					if ( select_mode==TRACK_ED_SELECT_UNMARK )
					{
						Node_status_remove_range(pending_status,left_limit,right_limit);
					}
					else
					{
					/* first remove any overlapping pending range */
					if (Node_status_is_value_in_range(pending_status,frame_no))
					{
						if (Node_status_get_range_containing_value(pending_status,
							frame_no,&start,&stop))
						{
							Node_status_remove_range(pending_status,start,stop);
						}
					}
					if ((!placed)||problem)
					{
						Node_status_add_range(pending_status,left_limit,right_limit);
					}
					}
				} break;
				case INTERPOLATE_MODE:
				{
					if ( select_mode==TRACK_ED_SELECT_UNMARK )
					{
						Node_status_remove_range(pending_status,left_limit,right_limit);
					}
					else
					{
						/* first remove any overlapping pending range */
						if (Node_status_is_value_in_range(pending_status,frame_no))
						{
							if (Node_status_get_range_containing_value(pending_status,
								frame_no,&start,&stop))
							{
								Node_status_remove_range(pending_status,start,stop);
							}
						}
						if (((!placed)||problem)&&
							Node_status_is_value_in_range(placed_status,left_limit-1)&&
							Node_status_is_value_in_range(placed_status,right_limit+1))
						{
							Node_status_add_range(pending_status,left_limit-1,right_limit+1);
						}
					}
				} break;
				case MAKE_BAD_MODE:
				{
					if (placed&&(!problem)&&(left_limit<=right_limit))
					{
						Node_status_add_range(problem_status,left_limit,right_limit);
					}
#if defined (OLD_CODE)
					/* first remove any overlapping pending range */
					if (Node_status_is_value_in_range(pending_status,frame_no))
					{
						if (Node_status_get_range_containing_value(pending_status,
							frame_no,&start,&stop))
						{
							Node_status_remove_range(pending_status,start,stop);
						}
					}
					/* the node clicked on must be good already */
					if (placed&&(!problem)&&(left_limit<=right_limit))
					{
						Node_status_add_range(pending_status,left_limit,right_limit);
					}
#endif /* defined (OLD_CODE) */
				} break;
				case MAKE_GOOD_MODE:
				{
					if (placed&&problem&&(left_limit<=right_limit))
					{
						Node_status_remove_range(problem_status,left_limit,right_limit);
					}
#if defined (OLD_CODE)
					/* first remove any overlapping pending range */
					if (Node_status_is_value_in_range(pending_status,frame_no))
					{
						if (Node_status_get_range_containing_value(pending_status,
							frame_no,&start,&stop))
						{
							Node_status_remove_range(pending_status,start,stop);
						}
					}
					if (placed&&problem&&(left_limit<=right_limit))
					{
						Node_status_add_range(pending_status,left_limit,right_limit);
					}
#endif /* defined (OLD_CODE) */
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"tracking_editor_select_node_frame.  Unknown control mode");
					return_code=0;
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_select_node_frame.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* tracking_editor_select_node_frame */

struct Select_node_frame_data
{
	enum Tracking_editor_select_mode select_mode;
	int frame_no,use_left_range,use_right_range;
	struct Tracking_editor_dialog *track_ed;
}; /* Select_node_frame_data */

static int tracking_editor_iterator_select_node_frame(
	struct Node_status *node_status,void *select_data_void)
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
Iterator function for calling tracking_editor_select_node_frame.
==============================================================================*/
{
	int return_code,node_no;
	struct Select_node_frame_data *select_data;

	ENTER(tracking_editor_iterator_select_node_frame);
	if (node_status&&
		(select_data=(struct Select_node_frame_data *)select_data_void))
	{
		node_no=Node_status_get_node_no(node_status);
		return_code=tracking_editor_select_node_frame(select_data->track_ed,
			node_no,select_data->frame_no,select_data->use_left_range,
			select_data->use_right_range, select_data->select_mode);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_iterator_select_node_frame.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* tracking_editor_iterator_select_node_frame */

static int tracking_editor_place_node_in_view(
	struct Tracking_editor_dialog *track_ed,int node_no,int add_view_no)
/*******************************************************************************
LAST MODIFIED : 2 April 1998

DESCRIPTION :
Adds the node to the placed_list for view <view_no> at the current frame_no.
If the node is placed in more than one view, it is placed in 3-D and set
pending for some operation depending on the current mode of operation of the
tracking editor. Note that in placing a node in 3-D, it will be added to the
placed list for the movie and removed from the placed lists for each view.
Furthermore, the routine will make sure the node is in the placed node group
for all views that use the node - needed when there are more than 2.
==============================================================================*/
{
	int return_code,view_no,views_placed,this_frame;
	struct Add_elements_with_node_data add_data;
	struct FE_node *node;
	struct Mirage_movie *movie;
	struct Mirage_view *view,*add_view;
	struct Node_status *node_status;

	ENTER(tracking_editor_place_node_in_view);
	if (track_ed&&(movie=track_ed->mirage_movie)&&
		(add_view=movie->views[add_view_no])&&
		(node=FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(node_no,
		movie->all_node_group)))
	{
		return_code=1;
		this_frame=movie->current_frame_no;

		/* find out how many views the node is currently placed in */
		views_placed=0;
		for (view_no=0;return_code&&(view_no<movie->number_of_views);view_no++)
		{
			if (view=movie->views[view_no])
			{
				if (node_status=FIND_BY_IDENTIFIER_IN_LIST(
					Node_status,node_no)(node_no,view->placed_list))
				{
					if (Node_status_is_value_in_range(node_status,this_frame))
					{
						views_placed++;
					}
				}
			}
			else
			{
				return_code=0;
			}
		}

		if (return_code)
		{
			if (0==views_placed)
			{
				/* add it to the placed list for view at this_frame */
				if (node_status=FIND_BY_IDENTIFIER_IN_LIST(
					Node_status,node_no)(node_no,add_view->placed_list))
				{
					return_code=Node_status_add_range(node_status,this_frame,this_frame);
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
				/* place the node in 3-D */
				/* first remove it from the placed list in each view */
				for (view_no=0;return_code&&(view_no<movie->number_of_views);view_no++)
				{
					view=movie->views[view_no];
					if (node_status=FIND_BY_IDENTIFIER_IN_LIST(
						Node_status,node_no)(node_no,view->placed_list))
					{
						return_code=Node_status_remove_range(node_status,this_frame,
							this_frame);
					}
				}

				/* add it to the placed list in 3-D */
				if (return_code&&(node_status=FIND_BY_IDENTIFIER_IN_LIST(
					Node_status,node_no)(node_no,movie->placed_list)))
				{
					return_code=Node_status_add_range(node_status,this_frame,this_frame);
				}
				else
				{
					return_code=0;
				}

				/* add the node to the placed group in 3-D, and elements too */
				if (return_code&&(return_code=ADD_OBJECT_TO_GROUP(FE_node)(node,
					movie->placed_nodes_3d)))
				{
					/* add any elements that use this node and all nodes placed in 3d */
					add_data.max_dimension=2;
					add_data.node=node;
					add_data.node_group=movie->placed_nodes_3d;
					add_data.element_group=movie->placed_elements_3d;
					MANAGED_GROUP_BEGIN_CACHE(FE_element)(movie->placed_elements_3d);
					FOR_EACH_OBJECT_IN_GROUP(FE_element)(
						add_elements_with_node_to_group,(void *)&add_data,
						movie->all_element_group);
					MANAGED_GROUP_END_CACHE(FE_element)(movie->placed_elements_3d);
				}

				/* add the node to the placed group for each view that can see this
				* node - if not already there - and the elements with it.
				*/
				for (view_no=0;return_code&&(view_no<movie->number_of_views);view_no++)
				{
					view=movie->views[view_no];
					/* if there is a node_status, the node can be placed in view */
					if (node_status=FIND_BY_IDENTIFIER_IN_LIST(
						Node_status,node_no)(node_no,view->placed_list))
					{
						if (!FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
							node_no,view->placed_nodes))
						{
							if (return_code=ADD_OBJECT_TO_GROUP(FE_node)(node,
								view->placed_nodes))
							{
								/* add elements with this node and all nodes placed in view */
								add_data.max_dimension=2;
								add_data.node=node;
								add_data.node_group=view->placed_nodes;
								add_data.element_group=view->placed_elements;
								MANAGED_GROUP_BEGIN_CACHE(FE_element)(view->placed_elements);
								FOR_EACH_OBJECT_IN_GROUP(FE_element)(
									add_elements_with_node_to_group,(void *)&add_data,
									movie->all_element_group);
								MANAGED_GROUP_END_CACHE(FE_element)(view->placed_elements);
							}
						}
					}
				}

				/* select the node and set pending ranges from it */
				tracking_editor_select_node_frame(track_ed,node_no,this_frame,1,1,
					TRACK_ED_SELECT_DEFAULT);
				tracking_editor_node_pending_change(track_ed,node_no);
				tracking_editor_update_bar_chart(track_ed);
			}
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"tracking_editor_place_node_in_view.  Error placing node");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_place_node_in_view.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* tracking_editor_place_node_in_view */

static int tracking_editor_unplace_node(
	struct Tracking_editor_dialog *track_ed,int node_no)
/*******************************************************************************
LAST MODIFIED : 1 April 1998

DESCRIPTION :
Removes the node from any lists (for the current frame) and groups it is in.
Pending lists are not affected with the following exceptions:
1. In TRACK_MODE when the node being unplaced is the first node of a pending
	range.
2. In BACKTRACK_MODE when the node being unplaced is the last node of a pending
	range.
3. In INTERPOLATE_MODE when the node is at either end of a pending range.
In both the above 2 cases the affected pending range is cleared.
==============================================================================*/
{
	int return_code,view_no,this_frame,start,stop;
	struct FE_node *node;
	struct Mirage_movie *movie;
	struct Mirage_view *view;
	struct Node_status *node_status;
	struct Remove_elements_with_node_data rem_data;

	ENTER(tracking_editor_unplace_node);
	if (track_ed&&(movie=track_ed->mirage_movie)&&
		(node=FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(node_no,
		movie->all_node_group)))
	{
		return_code=1;
		this_frame=movie->current_frame_no;

		if (node_status=FIND_BY_IDENTIFIER_IN_LIST(
			Node_status,node_no)(node_no,movie->placed_list))
		{
			if (Node_status_is_value_in_range(node_status,this_frame))
			{
				return_code=Node_status_remove_range(node_status,this_frame,
					this_frame);

				/* ensure not in problem range at this frame_no */
				if (return_code&&(node_status=FIND_BY_IDENTIFIER_IN_LIST(
					Node_status,node_no)(node_no,movie->problem_list)))
				{
					return_code=Node_status_remove_range(node_status,this_frame,
						this_frame);
				}
				else
				{
					return_code=0;
				}

				/* remove pending range in certain cases */
				if (return_code&&(node_status=FIND_BY_IDENTIFIER_IN_LIST(
					Node_status,node_no)(node_no,movie->pending_list)))
				{
					if (Node_status_is_value_in_range(node_status,this_frame)&&
						Node_status_get_range_containing_value(node_status,this_frame,
						&start,&stop))
					{
						switch (track_ed->control_mode)
						{
							case TRACK_MODE:
							{
								if (this_frame==start)
								{
									Node_status_remove_range(node_status,start,stop);
								}
							} break;
							case BACKTRACK_MODE:
							{
								if (this_frame==stop)
								{
									Node_status_remove_range(node_status,start,stop);
								}
							} break;
							case INTERPOLATE_MODE:
							{
								if ((this_frame==start)||(this_frame==stop))
								{
									Node_status_remove_range(node_status,start,stop);
								}
							} break;
							default:
							{
								/* do nothing */
							} break;
						}
					}
				}
				else
				{
					return_code=0;
				}

				if (FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
					node_no,movie->placed_nodes_3d))
				{
					if (return_code=REMOVE_OBJECT_FROM_GROUP(FE_node)(
						node,movie->placed_nodes_3d))
					{
						rem_data.node=node;
						rem_data.element_group=movie->placed_elements_3d;
						MANAGED_GROUP_BEGIN_CACHE(FE_element)(movie->placed_elements_3d);
						FOR_EACH_OBJECT_IN_GROUP(FE_element)(
							remove_elements_with_node_from_group,(void *)&rem_data,
							movie->all_element_group);
						MANAGED_GROUP_END_CACHE(FE_element)(movie->placed_elements_3d);
					}
				}
				if (FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
					node_no,movie->pending_nodes_3d))
				{
					REMOVE_OBJECT_FROM_GROUP(FE_node)(node,movie->pending_nodes_3d);
				}
				if (FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
					node_no,movie->problem_nodes_3d))
				{
					REMOVE_OBJECT_FROM_GROUP(FE_node)(node,movie->problem_nodes_3d);
				}
				tracking_editor_update_bar_chart(track_ed);
			}
			else
			{
				/* remove the node from the placed list in each view */
				for (view_no=0;return_code&&(view_no<movie->number_of_views);view_no++)
				{
					view=movie->views[view_no];
					if (node_status=FIND_BY_IDENTIFIER_IN_LIST(
						Node_status,node_no)(node_no,view->placed_list))
					{
						return_code=Node_status_remove_range(node_status,this_frame,
							this_frame);
					}
				}
			}

			/* make sure the node is not in any groups for the views. If need to
			* remove from the placed group in any view, remove the elements too.
			*/
			for (view_no=0;return_code&&(view_no<movie->number_of_views);view_no++)
			{
				view=movie->views[view_no];
				if (FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
					node_no,view->placed_nodes))
				{
					if (return_code=REMOVE_OBJECT_FROM_GROUP(FE_node)(
						node,view->placed_nodes))
					{
						rem_data.node=node;
						rem_data.element_group=view->placed_elements;
						MANAGED_GROUP_BEGIN_CACHE(FE_element)(view->placed_elements);
						FOR_EACH_OBJECT_IN_GROUP(FE_element)(
							remove_elements_with_node_from_group,(void *)&rem_data,
							movie->all_element_group);
						MANAGED_GROUP_END_CACHE(FE_element)(view->placed_elements);
					}
				}
				if (FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
					node_no,view->pending_nodes))
				{
					REMOVE_OBJECT_FROM_GROUP(FE_node)(node,view->pending_nodes);
				}
				if (FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
					node_no,view->problem_nodes))
				{
					REMOVE_OBJECT_FROM_GROUP(FE_node)(node,view->problem_nodes);
				}
			}
		}
		else
		{
			return_code=0;
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"tracking_editor_unplace_node.  Error unplacing node");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_unplace_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* tracking_editor_unplace_node */

static void Tracking_editor_dialog_node_group_change(
	struct MANAGER_MESSAGE(GROUP(FE_node)) *message,
	void *tracking_editor_dialog_void)
/*******************************************************************************
LAST MODIFIED : 28 April 1998

DESCRIPTION :
Node group manager change callback. Updates placed, pending and problem lists
to reflect members of the changed node group/s.
==============================================================================*/
{
	static int locked=0;
	int view_no,i,node_no,node_placed_in_view;
	struct Mirage_movie *movie;
	struct Mirage_view *view;
	struct Node_status *node_status;
	struct Tracking_editor_dialog *track_ed;

	ENTER(Tracking_editor_dialog_node_group_change);
	/* checking arguments */
	if (message&&(track_ed=(struct Tracking_editor_dialog *)
		tracking_editor_dialog_void)&&(movie=track_ed->mirage_movie)&&
		(!(track_ed->processing)))
	{
		if (!locked)
		{
			switch (message->change)
			{
				case MANAGER_CHANGE_OBJECT(GROUP(FE_node)):
				case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(GROUP(FE_node)):
				{
					for (view_no=0;view_no<movie->number_of_views;view_no++)
					{
						if (view=movie->views[view_no])
						{
							if (message->object_changed==view->placed_nodes)
							{
								locked=1;
								for (i=0;i<view->number_of_nodes;i++)
								{
									node_no=view->node_numbers[i];
									/* find out if node is listed as placed in this view: */
									if (node_status=FIND_BY_IDENTIFIER_IN_LIST(
										Node_status,node_no)(node_no,movie->placed_list))
									{
										if (!(node_placed_in_view=Node_status_is_value_in_range(
											node_status,movie->current_frame_no)))
										{
											if (node_status=FIND_BY_IDENTIFIER_IN_LIST(
												Node_status,node_no)(node_no,view->placed_list))
											{
												node_placed_in_view=Node_status_is_value_in_range(
													node_status,movie->current_frame_no);
											}
										}
									}
									if (node_status)
									{
										if (FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
											node_no,view->placed_nodes))
										{
											if (!node_placed_in_view)
											{
												tracking_editor_place_node_in_view(track_ed,node_no,
													view_no);
											}
										}
										else
										{
											if (node_placed_in_view)
											{
												tracking_editor_unplace_node(track_ed,node_no);
											}
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"Tracking_editor_dialog_node_group_change.  "
											"Missing node status information");
									}
								}
								locked=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Tracking_editor_dialog_node_group_change.  Missing view");
						}
					}
				} break;
				case MANAGER_CHANGE_ALL(GROUP(FE_node)):
				case MANAGER_CHANGE_DELETE(GROUP(FE_node)):
				case MANAGER_CHANGE_ADD(GROUP(FE_node)):
				case MANAGER_CHANGE_IDENTIFIER(GROUP(FE_node)):
				{
					/* do nothing */
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Tracking_editor_dialog_node_group_change.  Invalid argument(s)");
	}

	LEAVE;
} /* Tracking_editor_dialog_node_group_change */

static void Tracking_editor_dialog_node_change(
	struct MANAGER_MESSAGE(FE_node) *message,
	void *tracking_editor_dialog_void)
/*******************************************************************************
LAST MODIFIED : 4 November 1998

DESCRIPTION :
Node manager change callback. Puts changed nodes in the pending list.
==============================================================================*/
{
	int use_left_range,use_right_range,return_code,placed,problem,pending,
		node_no,frame_no,set_pending_range;
	struct Node_status *placed_status,*pending_status,*problem_status;
	struct FE_node *node;
	struct Mirage_movie *movie;
	struct Tracking_editor_dialog *track_ed;

	ENTER(Tracking_editor_dialog_node_change);
	/* checking arguments */
	if (message&&(track_ed=(struct Tracking_editor_dialog *)
		tracking_editor_dialog_void)&&(movie=track_ed->mirage_movie)&&
		((!(track_ed->processing)) || (track_ed->control_mode == MAKE_GOOD_MODE) ||
		(track_ed->control_mode == MAKE_BAD_MODE)))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(FE_node):
			{
				if (node=message->object_changed)
				{
					/* select the node and set pending ranges from it */
					/* have different use of left and right ranges depending on mode */
					return_code=1;
					node_no=get_FE_node_cm_node_identifier(node);
					frame_no=movie->current_frame_no;
					/* get placed, pending and problem status of node_no at frame_no */
					if (placed_status=FIND_BY_IDENTIFIER_IN_LIST(Node_status,node_no)(
						node_no,movie->placed_list))
					{
						placed=Node_status_is_value_in_range(placed_status,frame_no);
					}
					else
					{
						return_code=0;
					}
					if (pending_status=FIND_BY_IDENTIFIER_IN_LIST(Node_status,node_no)(
						node_no,movie->pending_list))
					{
						pending=Node_status_is_value_in_range(pending_status,frame_no);
					}
					else
					{
						return_code=0;
					}
					if (problem_status=FIND_BY_IDENTIFIER_IN_LIST(Node_status,node_no)(
						node_no,movie->problem_list))
					{
						problem=Node_status_is_value_in_range(problem_status,frame_no);
					}
					else
					{
						return_code=0;
					}

					if (return_code&&placed)
					{
						use_left_range=0;
						use_right_range=0;
						set_pending_range=1;
						switch (track_ed->control_mode)
						{
						case TRACK_MODE:
							{
								if (problem)
								{
									Node_status_remove_range(problem_status,frame_no,frame_no);
									tracking_editor_node_problem_change(track_ed,node_no);
								}
								use_right_range=1;
							} break;
						case BACKTRACK_MODE:
							{
								if (problem)
								{
									Node_status_remove_range(problem_status,frame_no,frame_no);
									tracking_editor_node_problem_change(track_ed,node_no);
								}
								use_left_range=1;
							} break;
						case INTERPOLATE_MODE:
							{
								use_left_range=1;
								use_right_range=1;
							} break;
#if defined (OLD_CODE)
						case SUBSTITUTE_MODE:
							{
							} break;
#endif /* defined (OLD_CODE) */
						case MAKE_BAD_MODE:
							{
								if (!problem)
								{
									Node_status_add_range(problem_status,frame_no,frame_no);
									tracking_editor_node_problem_change(track_ed,node_no);
								}
							} break;
						case MAKE_GOOD_MODE:
							{
								if (problem)
								{
									Node_status_remove_range(problem_status,frame_no,frame_no);
									tracking_editor_node_problem_change(track_ed,node_no);
								}
							} break;
						default:
							{
								/* do nothing */
							} break;
						}
						if(movie->modifiers & MIRAGE_MOVIE_MODIFIERS_TOGGLE_SELECT)
						{
							if(pending)
							{
								tracking_editor_select_node_frame(track_ed,node_no,frame_no,
									use_left_range,use_right_range,TRACK_ED_SELECT_UNMARK);
							}
							else
							{
								tracking_editor_select_node_frame(track_ed,node_no,frame_no,
									use_left_range,use_right_range,TRACK_ED_SELECT_DEFAULT);
							}
						}
						else
						{
							tracking_editor_select_node_frame(track_ed,node_no,frame_no,
								use_left_range,use_right_range,TRACK_ED_SELECT_DEFAULT);
						}
						tracking_editor_node_pending_change(track_ed,node_no);
						tracking_editor_update_bar_chart(track_ed);
					}
				}
			} break;
			case MANAGER_CHANGE_ALL(FE_node):
			case MANAGER_CHANGE_OBJECT(FE_node):
			case MANAGER_CHANGE_DELETE(FE_node):
			case MANAGER_CHANGE_ADD(FE_node):
			case MANAGER_CHANGE_IDENTIFIER(FE_node):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Tracking_editor_dialog_node_change.  Invalid argument(s)");
	}

	LEAVE;
} /* Tracking_editor_dialog_node_change */

static void tracking_editor_process_input_cb(XtPointer track_ed_void,
	int *source, XtInputId *input_id)
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Callback set up by XtAppAddInput. When XVG is running this callback is active
and called when there are imcoming messages on the socket. 
If there are it processes them, updating the bar chart accordingly.
==============================================================================*/
{
	char *args=NULL;
	static char InCom[XVG_SOCKET_COMMAND_LEN+1],cbuf[1024];
	int poll_value,PollReadEvents,frame_no,view_no;
#if defined (SGI)
	struct pollfd pfd;
#else /* defined (SGI) */
	int pfd;
	fd_set read_fdset;
	struct timeval timeout_struct;
#endif /* defined (SGI) */
	struct Mirage_movie *movie;
	struct Mirage_view *view;
	struct Tracking_editor_dialog *track_ed;

	ENTER(tracking_editor_process_input_cb);
	USE_PARAMETER(input_id);
	if ((track_ed=(struct Tracking_editor_dialog *)track_ed_void)&&
		(movie=track_ed->mirage_movie))
	{
#if defined (SGI)
		/* set polling bits */
		PollReadEvents = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI;
		pfd.fd = track_ed->process_remote_client;
		pfd.events = PollReadEvents;
#else /* defined (SGI) */
		pfd = track_ed->process_remote_client;
		FD_ZERO(&read_fdset);
		FD_SET(pfd, &read_fdset);
#endif /* defined (SGI) */

		args=(char *)NULL;
#if defined (SGI)
		if ((0 < (poll_value=poll(&pfd,(unsigned long)1,0)))&&
			tracking_editor_get_message(track_ed->process_remote_client,InCom,&args))
#else /* defined (SGI) */
		  /* A zero timeout is a poll */
		timeout_struct.tv_sec = 0;
		timeout_struct.tv_usec = 0;
		if ((0 < (poll_value=select(FD_SETSIZE, &read_fdset,NULL,NULL,&timeout_struct)))&&
			tracking_editor_get_message(track_ed->process_remote_client,InCom,&args))
#endif /* defined (SGI) */
		{
			/* Suspend messages until the end of the routine as confirmation routines
				allow X callbacks */
			if (track_ed->input_id)
			{
				XtRemoveInput(track_ed->input_id);
				track_ed->input_id = 0;
			}

			/********* decode incoming message **********/
			/* abort */
			if (strcmp("ABRT",InCom) == 0)
			{
				/*tracking_editor_send_message(remoteClient, "OK  ", NULL, 0);*/
				if (track_ed->kill_process_interval_id)
				{
					/* This is the response from the kill that we want. */
					XtRemoveTimeOut(track_ed->kill_process_interval_id);
					track_ed->kill_process_interval_id = 0;
				}
				track_ed->processing=0;
				track_ed->process_ID=0;
				if (args)
				{
					printf("ABRT command received... %s\n",args);
				}
				else
				{
					printf("ABRT command received...\n");
				}
				Mirage_movie_refresh_node_groups(movie);
				/* must read frame in case it was one changed */
				Mirage_movie_read_frame_nodes(movie,movie->current_frame_no);
				printf("\a\a\a");
				fflush(stdout);
				if (args)
				{
					confirmation_warning_ok("Process aborted",args,track_ed->dialog,
						track_ed->user_interface);
				}
				else
				{
					confirmation_warning_ok("Process aborted","No reason given",
						track_ed->dialog, track_ed->user_interface);
				}
			}
			/* done */
			else if (strcmp("DONE",InCom) == 0)
			{
				/*tracking_editor_send_message(remoteClient, "OK  ", NULL, 0);*/
				track_ed->processing=0;
				track_ed->process_ID=0;
				printf("DONE command received...\n");
				/*???RC some of this redundant if TEXT works - keep just in case */
				Node_status_list_add(movie->placed_list,movie->pending_list);
				Node_status_list_subtract(movie->problem_list,movie->pending_list);
				if ((TRACK_MODE==track_ed->control_mode)||
					(BACKTRACK_MODE==track_ed->control_mode))
				{
					sprintf(cbuf,"%s_req_lost",movie->name);
					Node_status_list_read(movie->problem_list,cbuf);
				}
				Node_status_list_clear(movie->pending_list);
				tracking_editor_update_bar_chart(track_ed);
				Mirage_movie_refresh_node_groups(movie);
				/* must read frame in case it was one changed */
				Mirage_movie_read_frame_nodes(movie,movie->current_frame_no);
				/* tell the user the job is completed */
				printf("\a\a\a");
				fflush(stdout);
				if (args)
				{
					confirmation_warning_ok("Process done",
						"Process successfully completed",track_ed->dialog,
						track_ed->user_interface );
				}
			}
			/* text message */
			else if (strcmp("TEXT",InCom) == 0)
			{
				if (args)
				{
					printf("TEXT : \"%s\"\n",args);
					tracking_editor_send_message(track_ed->process_remote_client,
						"OK  ",NULL,0);
					if (1==sscanf(args,"Frame %d",&frame_no))
					{
						Node_status_list_add_at_value(movie->placed_list,
							movie->pending_list,frame_no);
						/* also remove from placed lists in individual views */
						for (view_no=0;view_no<movie->number_of_views;view_no++)
						{
							if (view=movie->views[view_no])
							{
								Node_status_list_subtract_at_value(view->placed_list,
									movie->placed_list,frame_no);
							}
						}
						Node_status_list_subtract_at_value(movie->problem_list,
							movie->pending_list,frame_no);
						/* reduce pending ranges as you track */
						switch (track_ed->control_mode)
						{
						case TRACK_MODE:
							{
								Node_status_list_subtract_at_value(movie->pending_list,
									movie->pending_list,frame_no-1);
							} break;
						case BACKTRACK_MODE:
							{
								Node_status_list_subtract_at_value(movie->pending_list,
									movie->pending_list,frame_no+1);
							} break;
						case SUBSTITUTE_MODE:
							{
								Node_status_list_subtract_at_value(movie->pending_list,
									movie->pending_list,frame_no);
							} break;
						}
						if ((TRACK_MODE==track_ed->control_mode)||
							(BACKTRACK_MODE==track_ed->control_mode))
						{
							/* append the lost nodes on to the problem list */
							sprintf(cbuf,"%s_req_lost",movie->name);
							Node_status_list_read(movie->problem_list,cbuf);
						}
						tracking_editor_update_bar_chart(track_ed);
						/* auto save the node status lists that have changed */
						/*???RC remove if this affects performance? */
						Mirage_movie_write_node_status_lists(movie,"");
						if (frame_no==movie->current_frame_no)
						{
							Mirage_movie_read_frame_nodes(movie,movie->current_frame_no);
						}
					}
				}
				else
				{
					printf("TEXT : !NO ARGS!\n");
				}
			}
			/* PID */
			else if (strcmp("PID ",InCom) == 0)
			{
				track_ed->process_ID=atoi(args);
				printf("XVG PID : %d\n", track_ed->process_ID);
				tracking_editor_send_message(track_ed->process_remote_client,
					"OK  ",NULL,0);
			}
			/* unsupported message */
			else
			{
				sprintf(cbuf, "Unknown command \"%s\"...\n", InCom);
				tracking_editor_send_message(track_ed->process_remote_client,
					"NOK ",cbuf,strlen(cbuf)+1);
			}
			if (args)
			{
				DEALLOCATE(args);
			}
		}
		else
		{
			if (poll_value)
			{
				display_message(ERROR_MESSAGE,
					"tracking_editor_process_cb.  poll()");
				tracking_editor_kill_process(track_ed);
				/*???RC close the socket? */
			}
		}
		if (track_ed->processing)
		{
			/* Reinstate the callback */
			track_ed->input_id = XtAppAddInput(track_ed->user_interface->application_context,
				track_ed->process_remote_client, (XtPointer) XtInputReadMask,
				tracking_editor_process_input_cb, (XtPointer) track_ed);
		}
		else
		{
			tracking_editor_exit_process_mode(track_ed);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_process_input_cb.  Invalid argument(s)");
	}
	LEAVE;
} /* tracking_editor_process_input_cb */

static int tracking_editor_enter_process_mode(
	struct Tracking_editor_dialog *track_ed)
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Turns off widgets and callbacks that should not be active during processing.
Turns on the Abort button.
==============================================================================*/
{
	int return_code;

	ENTER(tracking_editor_enter_process_mode);
	/* checking arguments */
	if (track_ed)
	{
		XtSetSensitive(track_ed->file_menu,False);
#if defined (OLD_CODE)
		XtSetSensitive(track_ed->edit_menu,False);
#endif /* defined (OLD_CODE) */

		XtSetSensitive(track_ed->track_button,False);
		XtSetSensitive(track_ed->backtrack_button,False);
		XtSetSensitive(track_ed->substitute_button,False);
		XtSetSensitive(track_ed->interpolate_button,False);

		XtSetSensitive(track_ed->process_button,False);
		XtSetSensitive(track_ed->abort_button,True);
		track_ed->processing=1;
		/* no node/group manager messages during processing */
		if (track_ed->node_manager_callback_id)
		{
			MANAGER_DEREGISTER(FE_node)(
				track_ed->node_manager_callback_id,
					track_ed->node_manager);
			track_ed->node_manager_callback_id=NULL;
		}
		if (track_ed->node_group_manager_callback_id)
		{
			MANAGER_DEREGISTER(GROUP(FE_node))(
				track_ed->node_group_manager_callback_id,
				track_ed->node_group_manager);
			track_ed->node_group_manager_callback_id=NULL;
		}

		track_ed->input_id = XtAppAddInput(track_ed->user_interface->application_context,
			track_ed->process_remote_client, (XtPointer) XtInputReadMask,
			tracking_editor_process_input_cb, (XtPointer) track_ed);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_enter_process_mode.  Missing tracking editor dialog");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* tracking_editor_enter_process_mode */

static int tracking_editor_exit_process_mode(
	struct Tracking_editor_dialog *track_ed)
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Turns off widgets and callbacks that should not be active during processing.
Turns on the Abort button.
==============================================================================*/
{
	int return_code;

	ENTER(tracking_editor_exit_process_mode);
	/* checking arguments */
	if (track_ed)
	{
		if (track_ed->input_id)
		{
			XtRemoveInput(track_ed->input_id);
			track_ed->input_id = 0;
		}
		if (track_ed->process_remote_client)
		{
			close(track_ed->process_remote_client);
			track_ed->process_remote_client = 0;
		}
		if (track_ed->process_socket)
		{
			close(track_ed->process_socket);
			track_ed->process_socket = 0;
		}

		XtSetSensitive(track_ed->file_menu,True);
#if defined (OLD_CODE)
		XtSetSensitive(track_ed->edit_menu,True);
#endif /* defined (OLD_CODE) */

		XtSetSensitive(track_ed->track_button,True);
		XtSetSensitive(track_ed->backtrack_button,True);
		XtSetSensitive(track_ed->substitute_button,True);
		XtSetSensitive(track_ed->interpolate_button,True);

		XtSetSensitive(track_ed->process_button,True);
		XtSetSensitive(track_ed->abort_button,False);
		track_ed->processing=0;
		/* node/node group manager messages back on */
		if (NULL==track_ed->node_manager_callback_id)
		{
			track_ed->node_manager_callback_id=
				MANAGER_REGISTER(FE_node)
				(Tracking_editor_dialog_node_change,
					(void *)track_ed,track_ed->node_manager);
		}
		if (NULL==track_ed->node_group_manager_callback_id)
		{
			track_ed->node_group_manager_callback_id=
				MANAGER_REGISTER(GROUP(FE_node))
				(Tracking_editor_dialog_node_group_change,
					(void *)track_ed,track_ed->node_group_manager);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_exit_process_mode.  Missing tracking editor dialog");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* tracking_editor_exit_process_mode */

static int tracking_editor_file_read_movie(char *movie_file_name,
	void *track_ed_void)
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Reads a movie file into the tracking editor.
==============================================================================*/
{
	char *title,tmp_string[40];
	int return_code,max_nodes;
	struct Mirage_movie *tmp_movie;
	struct Tracking_editor_dialog *track_ed;
	XmString new_string;

	ENTER(tracking_editor_file_read_movie);
	if (movie_file_name&&
		(track_ed=(struct Tracking_editor_dialog *)track_ed_void))
	{
		busy_cursor_on((Widget)NULL, track_ed->user_interface );
		if (tmp_movie=read_Mirage_movie(movie_file_name))
		{
			/* must remove all current digitiser windows */
			if (track_ed->digitiser_window_manager)
			{
				REMOVE_ALL_OBJECTS_FROM_MANAGER(Digitiser_window)(
					track_ed->digitiser_window_manager);
			}
			/* no messages until new movie read in */
			if (track_ed->node_manager_callback_id)
			{
				MANAGER_DEREGISTER(FE_node)(
					track_ed->node_manager_callback_id,
					track_ed->node_manager);
				track_ed->node_manager_callback_id=NULL;
			}
			if (track_ed->node_group_manager_callback_id)
			{
				MANAGER_DEREGISTER(GROUP(FE_node))(
					track_ed->node_group_manager_callback_id,
					track_ed->node_group_manager);
				track_ed->node_group_manager_callback_id=NULL;
			}
			if (track_ed->mirage_movie)
			{
				DESTROY(Mirage_movie)(&(track_ed->mirage_movie));
			}
			track_ed->mirage_movie=tmp_movie;
			if (return_code=(enable_Mirage_movie_graphics(tmp_movie,
				track_ed->basis_manager,track_ed->computed_field_manager,
				track_ed->element_manager,track_ed->element_group_manager,
				track_ed->fe_field_manager,track_ed->glyph_list,
				track_ed->graphical_material_manager,
				track_ed->default_graphical_material,track_ed->light_manager,
				track_ed->node_manager,track_ed->node_group_manager,
				track_ed->data_manager,track_ed->data_group_manager,
				track_ed->element_point_ranges_selection,
				track_ed->element_selection,track_ed->node_selection,
				track_ed->data_selection,
				track_ed->scene_manager,track_ed->default_scene,
				track_ed->spectrum_manager,track_ed->default_spectrum,
				track_ed->texture_manager,track_ed->user_interface)&&
				(0<(max_nodes=NUMBER_IN_LIST(Node_status)(tmp_movie->placed_list)))))
			{
				if (tmp_movie->name&&ALLOCATE(title,char,20+strlen(tmp_movie->name)))
				{
					sprintf(title,"Tracking Editor: %s",tmp_movie->name);
					/* put the name of the movie in the title of the dialog */
					XtVaSetValues(track_ed->shell,XmNtitle,tmp_movie->name,NULL);
					DEALLOCATE(title);
				}
				/* read the first frame */
				if (return_code=
					read_Mirage_movie_frame(tmp_movie,tmp_movie->start_frame_no))
				{
					/* update views in all digitiser windows so textures correctly
						 displayed */
					FOR_EACH_OBJECT_IN_MANAGER(Digitiser_window)(
						Digitiser_window_update_view,(void *)NULL,
						track_ed->digitiser_window_manager);
					/* show graphics for the first frame */
					if (XmToggleButtonGetState(track_ed->view_2d_points))
					{
						Mirage_movie_graphics_show_2d_points(tmp_movie,0);
					}
					if (XmToggleButtonGetState(track_ed->view_2d_lines))
					{
						Mirage_movie_graphics_show_2d_lines(tmp_movie);
					}
					if (XmToggleButtonGetState(track_ed->view_2d_surfaces))
					{
						Mirage_movie_graphics_show_2d_surfaces(tmp_movie);
					}
					if (XmToggleButtonGetState(track_ed->view_3d_points))
					{
						Mirage_movie_graphics_show_3d_points(tmp_movie,0);
					}
					if (XmToggleButtonGetState(track_ed->view_3d_lines))
					{
						Mirage_movie_graphics_show_3d_lines(tmp_movie);
					}
					if (XmToggleButtonGetState(track_ed->view_3d_surfaces))
					{
						Mirage_movie_graphics_show_3d_surfaces(tmp_movie);
					}

					/* clear the pending ranges since they may be invalid under the
						current mode of operation */
					/*???RC better to restart in the last mode instead? */
					Node_status_list_clear(tmp_movie->pending_list);
					/* rebuild the placed, pending and problem node & element groups */
					Mirage_movie_refresh_node_groups(tmp_movie);
					/* show the current frame and range */
					sprintf(tmp_string,"%i",tmp_movie->current_frame_no);
					XtVaSetValues(track_ed->frame_text,XmNvalue,tmp_string,NULL);
					sprintf(tmp_string,"from range %i..%i",tmp_movie->start_frame_no,
						tmp_movie->start_frame_no+tmp_movie->number_of_frames-1);
					if (new_string=XmStringCreateSimple(tmp_string))
					{
						XtVaSetValues(track_ed->frame_range,
							XmNlabelString,new_string,NULL);
						XmStringFree(new_string);
					}
					/* SAB Save the current pending lists in case we need to revert */
					Mirage_movie_write_node_status_lists(tmp_movie,
						"_last_frame_change");

					/* now allow the user to see the frame info */
					XtManageChild(track_ed->frame_form);

					/* turn on node/group manager messages by resetting to state
						after processing has ended */
					tracking_editor_exit_process_mode(track_ed);

					/* set up bar chart ranges */
					track_ed->bc_left_limit=tmp_movie->start_frame_no;
					track_ed->bc_right_limit=
						tmp_movie->start_frame_no+tmp_movie->number_of_frames;
					track_ed->bc_bottom_limit=0;
					track_ed->bc_top_limit=max_nodes;
					track_ed->bc_top=track_ed->bc_top_limit;
					track_ed->bc_left=track_ed->bc_left_limit;
					track_ed->bc_pixels_per_unit_x=1;
					track_ed->bc_pixels_per_unit_y=1;
					tracking_editor_update_bar_chart(track_ed);
				}
			}
			if (!return_code&&track_ed->mirage_movie)
			{
				DESTROY(Mirage_movie)(&(track_ed->mirage_movie));
				confirmation_warning_ok("Error!","Could not read parts of the movie",
					track_ed->dialog, track_ed->user_interface);
			}
		}
		else
		{
			confirmation_warning_ok("Error!","Could not read the movie",
				track_ed->dialog, track_ed->user_interface);
			return_code=0;
		}
		busy_cursor_off((Widget)NULL, track_ed->user_interface );
	}
	else
	{
		display_message(INFORMATION_MESSAGE,
			"tracking_editor_file_read_movie.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* tracking_editor_file_read_movie */

static void tracking_editor_open_movie_cb(Widget dialog,XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 March 1998

DESCRIPTION :
Callback for the file menu dialogs
==============================================================================*/
{
	int return_code;
	struct File_open_data *file_open_data;
	struct Tracking_editor_dialog *track_ed;

	ENTER(tracking_editor_save_movie_cb);
	USE_PARAMETER(dialog);
	USE_PARAMETER(call_data);
	if (track_ed=(struct Tracking_editor_dialog *)client_data)
	{
		return_code=1;
		if (track_ed->mirage_movie)
		{
			/* save the current movie here */
			if (confirmation_question_yes_no("Open Movie...",
				"Save current movie first?",track_ed->dialog,
				track_ed->user_interface))
			{
				if (!Mirage_movie_full_save(track_ed->mirage_movie,""))
				{
					return_code=0;
					confirmation_warning_ok("Error!","Could not save the movie",
						track_ed->dialog, track_ed->user_interface);
				}
			}
		}
		if (return_code)
		{
			/* open a file selection box */
			if (file_open_data=create_File_open_data(".cmmov",REGULAR,
				tracking_editor_file_read_movie,(void *)track_ed,0,
				track_ed->user_interface))
			{
				open_file_and_read((Widget)NULL,(XtPointer)file_open_data,
					(XtPointer)NULL);
				/*???DB.  How to free file_open_data and data ? */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"tracking_editor_open_movie_cb.  Could not allocate file open data");
			}
		}
	}
	else
	{
		display_message(INFORMATION_MESSAGE,
			"tracking_editor_open_movie_cb.  Invalid argument(s)");
	}
	LEAVE;
} /* tracking_editor_open_movie_cb */

static void tracking_editor_save_movie_cb(Widget w,XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 April 1998

DESCRIPTION :
Callback for file|save movie menu button.
==============================================================================*/
{
	struct Tracking_editor_dialog *track_ed;

	ENTER(tracking_editor_save_movie_cb);
	USE_PARAMETER(w);
	USE_PARAMETER(call_data);
	if (track_ed=(struct Tracking_editor_dialog *)client_data)
	{
		if (!Mirage_movie_full_save(track_ed->mirage_movie,""))
		{
			confirmation_warning_ok("Error!","Could not save the movie",
				track_ed->dialog, track_ed->user_interface);
		}
	}
	else
	{
		display_message(INFORMATION_MESSAGE,
			"tracking_editor_save_movie_cb.  Invalid argument(s)");
	}
	LEAVE;
} /* tracking_editor_save_movie_cb */

static void tracking_editor_write_2d_cb(Widget widget,XtPointer track_ed_void,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 November 1998

DESCRIPTION :
Writes to file the list of 2-D positions of all the placed points in each view
at the current frame_no. The filename will be given the name of the movie with
the extension .#####.2d appended on to it, where ##### will be replaced with
the current_frame_no of the movie, eg. .01320.2d for frame 1320.
The format of the file will be:
!Any line starting in ! should be ignored.
Movie_file_name
Number_of_views
Number_of_points_in_view_2
!List of x,y coordinates of nodes in view 1
node_no x y
node_no x y
node_no x y
	.   .
	.   .
Number_of_points_in_view_2
node_no x y
	.   .
etc.
The 2-D file can be used to calibrate the photogrammetry. The movie file is
included in case it is needed to help locate how the 2-D points were generated
and to choose the 3-D set needed for calibration.
==============================================================================*/
{
	char *file_name_template,*file_name;
	double pos3[3],pos2[2];
	FE_value node_x,node_y,node_z;
	FILE *out_file;
	int view_no,i,node_no;
	struct FE_node *node;
	struct Tracking_editor_dialog *track_ed;
	struct Mirage_movie *movie;
	struct Mirage_view *view;

	ENTER(tracking_editor_write_2d_cb);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((track_ed=(struct Tracking_editor_dialog *)track_ed_void)&&
		(movie=track_ed->mirage_movie)&&movie->name)
	{
		if (ALLOCATE(file_name_template,char,strlen(movie->name)+12))
		{
			sprintf(file_name_template,"%s.#####.2d",movie->name);
			if (file_name=
				make_Mirage_file_name(file_name_template,movie->current_frame_no))
			{
				printf("Writing 2-D points to file '%s'\n",file_name);
				if (out_file=fopen(file_name,"w"))
				{
					fprintf(out_file,"!2-D points for frame %i of movie:\n",
						movie->current_frame_no);
					fprintf(out_file,"%s\n",movie->name);
					fprintf(out_file,"!Number of views:\n");
					fprintf(out_file,"%i\n",movie->number_of_views);
					for (view_no=0;view_no<movie->number_of_views;view_no++)
					{
						if (view=movie->views[view_no])
						{
							fprintf(out_file,"!Number of points in view %i\n",view_no+1);
							fprintf(out_file,"%i\n",
								NUMBER_IN_GROUP(FE_node)(view->placed_nodes));
							fprintf(out_file,"!Node_no x y\n");
							for (i=0;i<view->number_of_nodes;i++)
							{
								node_no=view->node_numbers[i];
								if (node=FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
									node_no,view->placed_nodes))
								{
									if (FE_node_get_position_cartesian(node,
										(struct FE_field *)NULL,&node_x,&node_y,&node_z,
										(FE_value *)NULL))
									{
										pos3[0]=(double)node_x;
										pos3[1]=(double)node_y;
										pos3[2]=(double)node_z;
										if (point_3d_to_2d_view(view->transformation43,pos3,pos2))
										{
											fprintf(out_file," %6i %14.6e %14.6e\n",
												get_FE_node_cm_node_identifier(node),pos2[0],pos2[1]);
										}
									}
								}
							}
						}
					}
					fclose(out_file);
				}
				DEALLOCATE(file_name);
			}
			DEALLOCATE(file_name_template);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_write_2d_cb.  Invalid argument(s)");
	}
	LEAVE;
} /* tracking_editor_write_2d_cb */

static void tracking_editor_frame_text_cb(Widget w,XtPointer track_ed_void,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
Callback specifying change of frame.
==============================================================================*/
{
	char tmp_string[20],*frame_text;
	int frame_no;
	struct Mirage_movie *movie;
	struct Tracking_editor_dialog *track_ed;

	ENTER(tracking_editor_frame_text_cb);
	USE_PARAMETER(w);
	USE_PARAMETER(call_data);
	if ((track_ed=(struct Tracking_editor_dialog *)track_ed_void)&&
		(movie=track_ed->mirage_movie))
	{
		XtVaGetValues(track_ed->frame_text,XmNvalue,&frame_text,NULL);
		if (frame_text)
		{
			frame_no=atoi(frame_text);
			/* do not write nodes while processing */
			if (track_ed->processing||
				Mirage_movie_write_frame_nodes(movie,movie->current_frame_no))
			{
				if (read_Mirage_movie_frame(movie,frame_no))
				{
					/* update views in all digitiser windows so textures correctly
						 displayed */
					FOR_EACH_OBJECT_IN_MANAGER(Digitiser_window)(
						Digitiser_window_update_view,(void *)NULL,
						track_ed->digitiser_window_manager);
					Mirage_movie_refresh_node_groups(movie);
					tracking_editor_update_bar_chart(track_ed);
				}
			}
		}
		/* reshow the current frame */
		sprintf(tmp_string,"%i",movie->current_frame_no);
		XtVaSetValues(track_ed->frame_text,XmNvalue,tmp_string,NULL);
	}
	else
	{
		display_message(INFORMATION_MESSAGE,
			"tracking_editor_frame_text_cb.  Invalid argument(s)");
	}
	LEAVE;
} /* tracking_editor_frame_text_cb */

static void tracking_editor_clear_pending_cb(Widget widget,
	XtPointer track_ed_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 10 April 1998

DESCRIPTION :
Callback for clearing all pending ranges.
==============================================================================*/
{
	int view_no;
	struct Mirage_movie *movie;
	struct Mirage_view *view;
	struct Tracking_editor_dialog *track_ed;

	ENTER(tracking_editor_clear_pending_cb);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((track_ed=(struct Tracking_editor_dialog *)track_ed_void)&&
		(movie=track_ed->mirage_movie))
	{
		/* no need to clear if already empty */
		if (FIRST_OBJECT_IN_LIST_THAT(Node_status)(Node_status_not_clear,
			(void *)NULL,movie->pending_list))
		{
			if (confirmation_warning_ok_cancel("Clear pending ranges...",
				"Proceed to clear all pending ranges?",track_ed->dialog,
				track_ed->user_interface))
			{
				Node_status_list_clear(movie->pending_list);
				tracking_editor_update_bar_chart(track_ed);
				REMOVE_ALL_OBJECTS_FROM_GROUP(FE_node)(movie->pending_nodes_3d);
				for (view_no=0;view_no<movie->number_of_views;view_no++)
				{
					if (view=movie->views[view_no])
					{
						REMOVE_ALL_OBJECTS_FROM_GROUP(FE_node)(view->pending_nodes);
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_clear_pending_cb.  Invalid argument(s)");
	}
	LEAVE;
} /* tracking_editor_clear_pending_cb */

static void tracking_editor_revert_cb(Widget widget,
	XtPointer track_ed_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 10 April 1998

DESCRIPTION :
Callback for clearing all pending ranges.
==============================================================================*/
{
	struct Mirage_movie *movie;
	struct Tracking_editor_dialog *track_ed;

	ENTER(tracking_editor_revert_cb);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((track_ed=(struct Tracking_editor_dialog *)track_ed_void)&&
		(movie=track_ed->mirage_movie))
	{
		if (confirmation_warning_ok_cancel("Revert nodes...",
			"Do you want to revert the node positions and status?",track_ed->dialog,
			track_ed->user_interface))
		{
			/* SAB Save the current pending lists in case we need to revert */
			Mirage_movie_read_node_status_lists(movie,
				"_last_frame_change");
			Mirage_movie_read_frame_nodes(movie,movie->current_frame_no);
			Mirage_movie_refresh_node_groups(movie);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_revert_cb.  Invalid argument(s)");
	}
	LEAVE;
} /* tracking_editor_clear_revert_cb */

static void tracking_editor_digitiser_cb(Widget w,XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Creates a new digitiser window.
==============================================================================*/
{
	char *digitiser_window_name;
	struct Digitiser_window *digitiser_window;
	struct Tracking_editor_dialog *dialog;

	ENTER(tracking_editor_digitiser_cb);
	USE_PARAMETER(w);
	USE_PARAMETER(call_data);
	if ((dialog=(struct Tracking_editor_dialog *)client_data)&&
		dialog->mirage_movie)
	{
		if (digitiser_window_name=Digitiser_window_manager_get_new_name(
			dialog->digitiser_window_manager))
		{
			if (digitiser_window=CREATE(Digitiser_window)(
				digitiser_window_name,dialog->mirage_movie,0,
				DIGITISER_WINDOW_DOUBLE_BUFFER,dialog->background_colour,
				dialog->light_manager,dialog->default_light,
				dialog->light_model_manager,dialog->default_light_model,
				dialog->scene_manager,dialog->texture_manager,dialog->user_interface))
			{
				if (!ADD_OBJECT_TO_MANAGER(Digitiser_window)(
					digitiser_window,dialog->digitiser_window_manager))
				{
					DESTROY(Digitiser_window)(&digitiser_window);
				}
			}
			DEALLOCATE(digitiser_window_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_digitiser_cb.  Invalid argument(s)");
	}
	LEAVE;
} /* tracking_editor_digitiser_cb */

static void tracking_editor_3d_window_cb(Widget w,XtPointer track_ed_void,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 11 April 2000

DESCRIPTION :
Creates a new 3-D window.
==============================================================================*/
{
	char *name;
	struct Graphics_window *window;
	struct Tracking_editor_dialog *track_ed;

	ENTER(tracking_editor_3d_window_cb);
	USE_PARAMETER(w);
	USE_PARAMETER(call_data);
	if (track_ed=(struct Tracking_editor_dialog *)track_ed_void)
	{
		if (name=Graphics_window_manager_get_new_name(
			track_ed->graphics_window_manager))
		{
			if (window=CREATE(Graphics_window)(name,
				SCENE_VIEWER_DOUBLE_BUFFER,track_ed->background_colour,
				track_ed->light_manager,track_ed->default_light,
				track_ed->light_model_manager,track_ed->default_light_model,
				track_ed->scene_manager,track_ed->default_scene,
				track_ed->texture_manager,track_ed->interactive_tool_manager,
				track_ed->user_interface))
			{
				if (!ADD_OBJECT_TO_MANAGER(Graphics_window)(window,
					track_ed->graphics_window_manager))
				{
					DESTROY(Graphics_window)(&window);
				}
			}
			DEALLOCATE(name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_3d_window_cb.  Invalid argument(s)");
	}
	LEAVE;
} /* tracking_editor_3d_window_cb */

static void tracking_editor_view_objects_cb(Widget widget,
	XtPointer track_ed_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 April 1998

DESCRIPTION :
Toggles graphics object on and off: 3-D surfaces, etc.
==============================================================================*/
{
	struct Tracking_editor_dialog *track_ed;
	struct Mirage_movie *movie;
	int toggled_on,show_node_numbers;

	ENTER(tracking_editor_view_objects_cb);
	USE_PARAMETER(call_data);
	if (widget&&(track_ed=(struct Tracking_editor_dialog *)track_ed_void)&&
		(movie=track_ed->mirage_movie))
	{
		show_node_numbers=XmToggleButtonGetState(track_ed->view_node_numbers);
		toggled_on=XmToggleButtonGetState(widget);
		if (widget==track_ed->view_2d_points)
		{
			if (toggled_on)
			{
				Mirage_movie_graphics_show_2d_points(movie,show_node_numbers);
			}
			else
			{
				Mirage_movie_graphics_hide_2d_points(movie);
			}
		}
		if (widget==track_ed->view_2d_lines)
		{
			if (toggled_on)
			{
				Mirage_movie_graphics_show_2d_lines(movie);
			}
			else
			{
				Mirage_movie_graphics_hide_2d_lines(movie);
			}
		}
		if (widget==track_ed->view_2d_surfaces)
		{
			if (toggled_on)
			{
				Mirage_movie_graphics_show_2d_surfaces(movie);
			}
			else
			{
				Mirage_movie_graphics_hide_2d_surfaces(movie);
			}
		}
		if (widget==track_ed->view_3d_points)
		{
			if (toggled_on)
			{
				Mirage_movie_graphics_show_3d_points(movie,show_node_numbers);
			}
			else
			{
				Mirage_movie_graphics_hide_3d_points(movie);
			}
		}
		if (widget==track_ed->view_3d_lines)
		{
			if (toggled_on)
			{
				Mirage_movie_graphics_show_3d_lines(movie);
			}
			else
			{
				Mirage_movie_graphics_hide_3d_lines(movie);
			}
		}
		if (widget==track_ed->view_3d_surfaces)
		{
			if (toggled_on)
			{
				Mirage_movie_graphics_show_3d_surfaces(movie);
			}
			else
			{
				Mirage_movie_graphics_hide_3d_surfaces(movie);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_view_objects_cb.  Invalid argument(s)");
	}
	LEAVE;
} /* tracking_editor_view_objects_cb */

static void tracking_editor_view_node_numbers_cb(Widget widget,
	XtPointer track_ed_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Turns on node numbers of the placed points in the 2-D and 3-D views.
==============================================================================*/
{
	struct Tracking_editor_dialog *track_ed;
	struct Mirage_movie *movie;
	int show_node_numbers;

	ENTER(tracking_editor_view_node_numbers_cb);
	USE_PARAMETER(call_data);
	if (widget&&(track_ed=(struct Tracking_editor_dialog *)track_ed_void)&&
		(movie=track_ed->mirage_movie))
	{
		show_node_numbers=XmToggleButtonGetState(widget);
		if (XmToggleButtonGetState(track_ed->view_2d_points))
		{
			Mirage_movie_graphics_hide_2d_points(movie);
			Mirage_movie_graphics_show_2d_points(movie,show_node_numbers);
		}
		if (XmToggleButtonGetState(track_ed->view_3d_points))
		{
			Mirage_movie_graphics_hide_3d_points(movie);
			Mirage_movie_graphics_show_3d_points(movie,show_node_numbers);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_view_node_numbers_cb.  Invalid argument(s)");
	}
	LEAVE;
} /* tracking_editor_view_node_numbers_cb */

static void tracking_editor_process_tracking_tolerance_cb(Widget widget,
	XtPointer track_ed_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 May 1998

DESCRIPTION :
==============================================================================*/
{
	char *button_text;
	struct Tracking_editor_dialog *track_ed;
	XmRowColumnCallbackStruct *rowcolumn_data;
	XmString button_label;

	ENTER(tracking_editor_tracking_tolerance_cb);
	if (widget&&(track_ed=(struct Tracking_editor_dialog *)track_ed_void)
		&&(rowcolumn_data=(XmRowColumnCallbackStruct *)call_data))
	{
		XtVaGetValues ( rowcolumn_data->widget,
			XmNlabelString, &button_label,
			NULL );
		XmStringGetLtoR(button_label, XmSTRING_DEFAULT_CHARSET, &button_text);
		if ( !strcmp(button_text, "Predictive"))
		{
			track_ed->tracking_tolerance = LOW_TOLERANCE;
		}
		else if ( !strcmp(button_text, "Normal"))
		{
			track_ed->tracking_tolerance = MEDIUM_TOLERANCE;
		}
#if defined (OLD_CODE)
		else if ( !strcmp(button_text, "Refine"))
		{
			track_ed->tracking_tolerance = HIGH_TOLERANCE;
		}
#endif /* defined (OLD_CODE) */

		XtFree(button_text);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_process_tracking_tolerance_cb.  Invalid argument(s)");
	}
	LEAVE;
} /* tracking_editor_process_tracking_tolerance_cb */

#if defined (OLD_CODE)
static void tracking_editor_process_tracking_second_pass_cb(Widget widget,
	XtPointer track_ed_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 May 1998

DESCRIPTION :
==============================================================================*/
{
	Boolean flag_set;
	struct Tracking_editor_dialog *track_ed;

	ENTER(tracking_editor_tracking_second_pass_cb);
	if (widget&&(track_ed=(struct Tracking_editor_dialog *)track_ed_void))
	{
		XtVaGetValues ( widget,
			XmNset, &flag_set,
			NULL );
		if ( flag_set )
		{
			track_ed->tracking_second_pass_flag = 1;
		}
		else
		{
			track_ed->tracking_second_pass_flag = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_process_tracking_second_pass_cb.  Invalid argument(s)");
	}
	LEAVE;
} /* tracking_editor_process_tracking_second_pass_cb */
#endif /* defined (OLD_CODE) */

static void tracking_editor_process_tracking_search_radius_cb(Widget widget,
	XtPointer track_ed_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 May 1998

DESCRIPTION :
==============================================================================*/
{
	char *text, new_text[15];
	struct Tracking_editor_dialog *track_ed;

	ENTER(tracking_editor_tracking_second_pass_cb);
	USE_PARAMETER(call_data);
	if (widget&&(track_ed=(struct Tracking_editor_dialog *)track_ed_void))
	{
		track_ed->tracking_search_radius = 16;
		XtVaGetValues ( widget,
			XmNvalue, &text,
			NULL );
		sscanf( text, "%d",&(track_ed->tracking_search_radius) );
		sprintf( new_text, "%d", track_ed->tracking_search_radius );
		XtVaSetValues( widget,
			XmNvalue, new_text,
			NULL );
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_process_tracking_second_pass_cb.  Invalid argument(s)");
	}
	LEAVE;
} /* tracking_editor_process_tracking_second_pass_cb */

static void tracking_editor_process_tracking_port_cb(Widget widget,
	XtPointer track_ed_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 May 1998

DESCRIPTION :
==============================================================================*/
{
	char *text, new_text[15];
	struct Tracking_editor_dialog *track_ed;

	ENTER(tracking_editor_tracking_second_pass_cb);
	USE_PARAMETER(call_data);
	if (widget&&(track_ed=(struct Tracking_editor_dialog *)track_ed_void))
	{
		track_ed->tracking_port = 0;
		XtVaGetValues ( widget,
			XmNvalue, &text,
			NULL );
		sscanf( text, "%d",&(track_ed->tracking_port) );
		sprintf( new_text, "%d", track_ed->tracking_port );
		XtVaSetValues( widget,
			XmNvalue, new_text,
			NULL );
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_process_tracking_port_cb.  Invalid argument(s)");
	}
	LEAVE;
} /* tracking_editor_process_tracking_port_cb */

static int tracking_editor_process_tracking_options( Widget parent,
	void *track_ed_void )
/*******************************************************************************
LAST MODIFIED : 13 May 1998

DESCRIPTION :
==============================================================================*/
{
	Arg option_menu_args[] = {
		{XmNoptionLabel, NULL},
		{XmNbuttonCount, 2},
		{XmNbuttons, NULL},
		{XmNbuttonSet, 0},
		{XmNentryCallback, NULL},
		{XmNleftAttachment,XmATTACH_POSITION},
		{XmNleftOffset,0},
		{XmNleftPosition,10},
		{XmNtopAttachment,XmATTACH_FORM},
		{XmNtopOffset,10}};
	char search_radius_value[15], port_value[15];
	int return_code;
	struct Tracking_editor_dialog *track_ed;
	struct User_interface *user_interface;
	Widget option_form, search_radius_label, search_radius_text,
		port_label, port_text, tolerance_option_menu;
	XmString button_labels[3], option_label_string, search_radius_label_string,
		port_label_string;
	XtCallbackRec callback_list[2];

	ENTER(tracking_editor_process_tracking_options);
	if (parent&&(track_ed=(struct Tracking_editor_dialog *)track_ed_void)
		&& (user_interface = track_ed->user_interface))
	{
		if ( option_form = XtVaCreateManagedWidget("tracking_options_form",
			xmFormWidgetClass, parent,
			XmNfractionBase, 120,
			NULL))
		{
			option_label_string = XmStringCreateSimple("Tracking mode: ");
			option_menu_args[0].value = (XtArgVal)option_label_string;
			/* The button_labels strings should correspond exactly to the strings
				in the XmNentryCallback callback function and their order should
				correspond to the order of the respective values in the
				Tracking_editor_tracking_tolerance enum. */
			button_labels[0] = XmStringCreateSimple("Predictive");
			button_labels[1] = XmStringCreateSimple("Normal");
			option_menu_args[2].value = (XtArgVal)button_labels;
			option_menu_args[3].value = (XtArgVal)track_ed->tracking_tolerance;
			callback_list[0].callback = tracking_editor_process_tracking_tolerance_cb;
			callback_list[0].closure = (XtPointer)track_ed;
			callback_list[1].callback = NULL;
			callback_list[1].closure = NULL;
			option_menu_args[4].value = (XtArgVal)callback_list;

			tolerance_option_menu = XmCreateSimpleOptionMenu(option_form,
				"tracking_tolerance_option",
				option_menu_args, XtNumber( option_menu_args ));

			XmStringFree( option_label_string );
			XmStringFree( button_labels[0] );
			XmStringFree( button_labels[1] );
			XtManageChild ( tolerance_option_menu );

			search_radius_label_string = XmStringCreateSimple( "Search radius");
			search_radius_label = XtVaCreateManagedWidget("tracking_search_radius_label",
				xmLabelWidgetClass, option_form,
				XmNleftAttachment,XmATTACH_POSITION,
				XmNleftOffset,0,
				XmNleftPosition, 10,
				XmNtopAttachment,XmATTACH_WIDGET,
				XmNtopOffset,user_interface->widget_spacing,
				XmNtopWidget,tolerance_option_menu,
				XmNlabelString, search_radius_label_string,
				NULL);
			XmStringFree( search_radius_label_string );
			sprintf(search_radius_value, "%d", track_ed->tracking_search_radius );
			search_radius_text = XtVaCreateManagedWidget("tracking_search_radius_label",
				xmTextWidgetClass, option_form,
				XmNleftAttachment,XmATTACH_WIDGET,
				XmNleftOffset,user_interface->widget_spacing,
				XmNleftWidget, search_radius_label,
				XmNtopAttachment,XmATTACH_WIDGET,
				XmNtopOffset,user_interface->widget_spacing,
				XmNtopWidget,tolerance_option_menu,
				XmNvalue, search_radius_value,
				XmNwidth, 50,
				NULL);
			XtAddCallback(search_radius_text, XmNactivateCallback,
				tracking_editor_process_tracking_search_radius_cb, track_ed_void);
			XtAddCallback(search_radius_text, XmNlosingFocusCallback,
				tracking_editor_process_tracking_search_radius_cb, track_ed_void);
			
			port_label_string = XmStringCreateSimple( "Port number"); 
			port_label = XtVaCreateManagedWidget("tracking_port_label",
				xmLabelWidgetClass, option_form,
				XmNleftAttachment,XmATTACH_POSITION,
				XmNleftOffset,0,
				XmNleftPosition, 10,
				XmNtopAttachment,XmATTACH_WIDGET,
				XmNtopOffset,user_interface->widget_spacing,
				XmNtopWidget,search_radius_text,
				XmNlabelString, port_label_string,
				NULL);
			XmStringFree( port_label_string );
			sprintf(port_value, "%d", track_ed->tracking_port );
			port_text = XtVaCreateManagedWidget("tracking_port_label",
				xmTextWidgetClass, option_form,
				XmNleftAttachment,XmATTACH_WIDGET,
				XmNleftOffset,user_interface->widget_spacing,
				XmNleftWidget, port_label,
				XmNtopAttachment,XmATTACH_WIDGET,
				XmNtopOffset,user_interface->widget_spacing,
				XmNtopWidget,search_radius_text,
				XmNvalue, port_value,
				XmNwidth, 90,
				NULL);
			XtAddCallback(port_text, XmNactivateCallback,
				tracking_editor_process_tracking_port_cb, track_ed_void);
			XtAddCallback(port_text, XmNlosingFocusCallback,
				tracking_editor_process_tracking_port_cb, track_ed_void);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"tracking_editor_process_tracking_options.  Unable to create option form");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_process_tracking_options.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* tracking_editor_process_tracking_options */

static void tracking_editor_process_cb(Widget widget,
	XtPointer track_ed_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Sets tracking or other process running if in one of these modes.
==============================================================================*/
{
	struct sockaddr_in socketDescriptor;
	char *req_file_name, *good_req_file_name, sys_command[4*BUFFER_SIZE];
	int return_code,addressLength,error_code,run_process;
	struct Mirage_movie *movie;
	struct Tracking_editor_dialog *track_ed;
	struct LIST(Node_status) *good_node_list;
	unsigned long ip_address;

	ENTER(tracking_editor_process_cb);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((track_ed=(struct Tracking_editor_dialog *)track_ed_void)&&
		(movie=track_ed->mirage_movie))
	{
		return_code = 0;
		/* proceed only if there are pending ranges to process and
			the user wants to proceed. */
		if (FIRST_OBJECT_IN_LIST_THAT(Node_status)(Node_status_not_clear,
			(void *)NULL,movie->pending_list))
		{
			switch (track_ed->control_mode )
			{
				case TRACK_MODE:
				case BACKTRACK_MODE:
				{
					if ( confirmation_warning_ok_cancel_plus_options(
						"Run tracking...",
						"Ready to start tracking?",track_ed->dialog,
						tracking_editor_process_tracking_options, (void *)track_ed,
						track_ed->user_interface))
					{
						return_code = 1;
					}
				} break;
				default:
				{
					if ( confirmation_warning_ok_cancel("Run process...",
						"Ready to start process?",track_ed->dialog, track_ed->user_interface))
					{
						return_code = 1;
					}
				} break;
			}
		}
		if ( return_code )
		{
			/* autosave the movie before processing */
			if (!Mirage_movie_full_save(track_ed->mirage_movie,""))
			{
				confirmation_warning_ok("Error!",
					"Could not save the movie before processing",track_ed->dialog,
					track_ed->user_interface);
			}
			else
			{
				/* write the pending lists to the request file */
				if (ALLOCATE(req_file_name,char,strlen(movie->name)+5))
				{
					sprintf(req_file_name,"%s_req",movie->name);

					/* good and bad modes do not use request files */
					if ((MAKE_BAD_MODE==track_ed->control_mode)||
						(MAKE_GOOD_MODE==track_ed->control_mode)||
						Node_status_list_write(movie->pending_list,req_file_name,
						"!Op\tNode\tStart\tStop\n","T\t%i\t%i\t%i\n"))
					{
						run_process=0;
						return_code=1;
						switch (track_ed->control_mode)
						{
						case TRACK_MODE:
							{
								sprintf(sys_command,"track.process %s %s %s %d %d %d",
									movie->name,track_ed->host,req_file_name,
									track_ed->tracking_tolerance,
									track_ed->tracking_search_radius,
									track_ed->tracking_port);
								run_process=return_code=
									SocketInitialize(&(track_ed->process_socket),
									  track_ed->tracking_port);
							} break;
						case BACKTRACK_MODE:
							{
								sprintf(sys_command,"backtrack.process %s %s %s %d %d %d",
									movie->name,track_ed->host,req_file_name,
									track_ed->tracking_tolerance,
									track_ed->tracking_search_radius,
									track_ed->tracking_port);
								run_process=return_code=
									SocketInitialize(&(track_ed->process_socket),
									  track_ed->tracking_port);
							} break;
						case SUBSTITUTE_MODE:
							{
								if ( good_node_list = Node_status_list_duplicate(movie->placed_list ))
								{
									if ( Node_status_list_subtract( good_node_list, movie->problem_list ))
									{
										if (ALLOCATE(good_req_file_name,char,strlen(movie->name)+15))
										{
											sprintf(good_req_file_name,"%s_good_req",movie->name);

											if (Node_status_list_write(good_node_list,good_req_file_name,
												"!Op\tNode\tStart\tStop\n","T\t%i\t%i\t%i\n"))
											{
												sprintf(sys_command,"substitute.process %s %s %s %s",
													track_ed->host,req_file_name,
													good_req_file_name,
													movie->node_file_name_template);
												run_process=return_code=
													SocketInitialize(&(track_ed->process_socket),
													  getPortFromProtocol("cmgui-substnode"));
											}
											DEALLOCATE(good_req_file_name);
										}
									}
									DESTROY(LIST(Node_status))(&good_node_list);
								}
							} break;
						case INTERPOLATE_MODE:
							{
								sprintf(sys_command,"interpolate.process %s %s %s",
									track_ed->host,req_file_name,
									movie->node_file_name_template);
								run_process=return_code=
									SocketInitialize(&(track_ed->process_socket),
									  getPortFromProtocol("cmgui-lininterp"));
							} break;
						case MAKE_BAD_MODE:
							{
								/* add pending ranges to bad */
								Node_status_list_add(movie->problem_list,movie->pending_list);
								tracking_editor_refresh_problem_groups(track_ed);
								Node_status_list_clear(movie->pending_list);
								tracking_editor_refresh_pending_groups(track_ed);
								tracking_editor_update_bar_chart(track_ed);
							} break;
						case MAKE_GOOD_MODE:
							{
								/* subtract pending ranges from bad */
								Node_status_list_subtract(movie->problem_list,
									movie->pending_list);
								tracking_editor_refresh_problem_groups(track_ed);
								Node_status_list_clear(movie->pending_list);
								tracking_editor_refresh_pending_groups(track_ed);
								tracking_editor_update_bar_chart(track_ed);
							} break;
						default:
							{
							} break;
						}
						if (run_process)
						{
							/* wait for host to connect */
							printf("running: %s\n",sys_command);
							if (error_code=system(sys_command))
							{
								display_message(ERROR_MESSAGE,"tracking_editor_process_cb.  "
									"error executing process: error code=%d",error_code);
								close(track_ed->process_socket);
								track_ed->process_socket = 0;
							}
							else
							{
								/* wait for the remote host to connect */
								printf("SocketInitialize() : Waiting for the remote host to connect...\n");
								addressLength = sizeof(socketDescriptor);
								memset(&socketDescriptor,0,addressLength);
	/*???debug*/fprintf(stderr,"process_cb #1\n");
								if (-1==(track_ed->process_remote_client=
									accept(track_ed->process_socket,
										(struct sockaddr *)&socketDescriptor,&addressLength)))
								{
									display_message(ERROR_MESSAGE,
										"tracking_editor_process_cb.  accept()");
									tracking_editor_kill_process(track_ed);
									close(track_ed->process_socket);
									track_ed->process_socket = 0;
									track_ed->process_remote_client = 0;
								}
								else
								{
									ip_address = ntohl(socketDescriptor.sin_addr.s_addr);
									sprintf(track_ed->remote_host,"%d.%d.%d.%d",
										(ip_address & 0xff000000) >> 24,
										(ip_address & 0x00ff0000) >> 16,
										(ip_address & 0x0000ff00) >> 8,
										(ip_address & 0x000000ff));
									printf("SocketInitialize() : host at IP address %s has connected...\n",
										track_ed->remote_host);

	/*???debug*/fprintf(stderr,"process_cb #2\n");
									/* request timeout to check socket messages */
									tracking_editor_enter_process_mode(track_ed);
#if defined (OLD_CODE)
									XtAppAddTimeOut(track_ed->user_interface->application_context,
										20,tracking_editor_process_input_cb,(XtPointer)track_ed);
#endif /* defined (OLD_CODE) */
								}
							}
						}
						else
						{
							if (!return_code)
							{
								display_message(ERROR_MESSAGE,"tracking_editor_process_cb.  "
									"Could not initialize socket");
							}
						}
					}
					DEALLOCATE(req_file_name);
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_control_process_cb.  Invalid argument(s)");
	}
	LEAVE;
} /* tracking_editor_process_cb */

static void tracking_editor_abort_cb(Widget widget,XtPointer track_ed_void,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Aborts tracking.
==============================================================================*/
{
	struct Tracking_editor_dialog *track_ed;

	ENTER(tracking_editor_abort_cb);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((track_ed=(struct Tracking_editor_dialog *)track_ed_void)&&
		track_ed->processing)
	{
		if (track_ed->process_ID)
		{
			printf("Aborting process...\n");
			tracking_editor_kill_process(track_ed);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"tracking_editor_abort_cb.  No process running");
			XtSetSensitive(track_ed->file_menu,True);
			XtSetSensitive(track_ed->abort_button,False);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_abort_cb.  Invalid argument(s)");
	}
	LEAVE;
} /* tracking_editor_abort_cb */

static int tracking_editor_change_mode_pending_status_iterator(
	struct Node_status *pending_status,void *track_ed_void)
/*******************************************************************************
LAST MODIFIED : 19 November 1998

DESCRIPTION :
Redirects the node range when the track mode changes.
==============================================================================*/
{
	int node_no, pending, return_code, use_left_range, use_right_range;
	struct Mirage_movie *movie;
	struct Tracking_editor_dialog *track_ed;
 
	ENTER(tracking_editor_change_mode_node_status_iterator);
	if (pending_status && (track_ed = (struct Tracking_editor_dialog *)track_ed_void)
		&& (movie = track_ed->mirage_movie))
	{
		node_no = Node_status_get_node_no(pending_status);
		pending=Node_status_is_value_in_range(pending_status,movie->current_frame_no);
		Node_status_clear(pending_status, NULL);
		if(pending)
		{
			use_left_range = 0;
			use_right_range = 0;
			switch (track_ed->control_mode)
			{
				case TRACK_MODE:
				{
					use_right_range=1;
				} break;
				case BACKTRACK_MODE:
				{
					use_left_range=1;
				} break;
				case INTERPOLATE_MODE:
				{
					use_left_range=1;
					use_right_range=1;
				} break;
			}
			tracking_editor_select_node_frame(track_ed,node_no,movie->current_frame_no,
				use_left_range,use_right_range,TRACK_ED_SELECT_DEFAULT);	
			return_code=1;
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"tracking_editor_change_mode_node_status_iterator."
			" Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* tracking_editor_change_mode_node_status_iterator */

static int tracking_editor_set_control_mode(
	struct Tracking_editor_dialog *track_ed,
	enum Tracking_editor_control_mode control_mode,int update_menu)
/*******************************************************************************
LAST MODIFIED : 9 April 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code,i,num_children;
	struct Mirage_movie *movie;
	Widget *child_list;
	XtPointer dummy;

	ENTER(tracking_editor_set_control_mode);
	if (track_ed)
	{
		return_code=1;
		if ((control_mode<TRACK_MODE)||(control_mode>MAKE_GOOD_MODE))
		{
			control_mode=TRACK_MODE;
		}
		/*???debug*/
		/*printf("tracking_editor_set_control_mode: %i\n",control_mode);*/
		if (movie=track_ed->mirage_movie)
		{
			if (control_mode != track_ed->control_mode)
			{
				/* can't change mode if there is anything pending... */
				if (FIRST_OBJECT_IN_LIST_THAT(Node_status)(Node_status_not_clear,
					(void *)NULL,movie->pending_list))
				{
					if ((control_mode == MAKE_BAD_MODE ||
						control_mode == MAKE_GOOD_MODE))
					{
						/* Allow the pending lists to remain if you go to MAKE_BAD
							or MAKE_GOOD */
							track_ed->control_mode=control_mode;
					}
					else
					{
						if ( ((track_ed->control_mode == MAKE_BAD_MODE)
							|| (track_ed->control_mode == MAKE_GOOD_MODE) ) &&
							(control_mode == track_ed->previous_control_mode ) )
						{
							/* Allow the pending lists to remain if you are changing
								back from MAKE_BAD or MAKE_GOOD to the previous mode */
							track_ed->control_mode=control_mode;
						}
						else
						{
							track_ed->previous_control_mode = track_ed->control_mode;
							track_ed->control_mode=control_mode;

							FOR_EACH_OBJECT_IN_LIST(Node_status)(
								tracking_editor_change_mode_pending_status_iterator,
								(void *)track_ed, movie->pending_list);
							tracking_editor_refresh_pending_groups(track_ed);
							tracking_editor_update_bar_chart(track_ed);
#if defined (OLD_CODE)
							if (confirmation_warning_ok_cancel("Change mode...",
								"This will clear pending lists!",track_ed->dialog,
								track_ed->user_interface))
							{
								track_ed->previous_control_mode = track_ed->control_mode;
								track_ed->control_mode=control_mode;

								Node_status_list_clear(movie->pending_list);
								tracking_editor_update_bar_chart(track_ed);
								REMOVE_ALL_OBJECTS_FROM_GROUP(FE_node)(movie->pending_nodes_3d);
								for (view_no=0;view_no<movie->number_of_views;view_no++)
								{
									if (view=movie->views[view_no])
									{
										REMOVE_ALL_OBJECTS_FROM_GROUP(FE_node)(view->pending_nodes);
									}
								}
							}
							else
							{
								update_menu=1;
							}
#endif /* defined (OLD_CODE) */
						}
					}
				}
				else
				{
					/* If the mode is a major mode then record what it was so that
						a change to bad or good can be changed back to that mode
						without clearing the pending lists */
					if ((track_ed->control_mode == TRACK_MODE)
						|| (track_ed->control_mode == BACKTRACK_MODE)
						|| (track_ed->control_mode == SUBSTITUTE_MODE)
						|| (track_ed->control_mode == INTERPOLATE_MODE))
					{
						track_ed->previous_control_mode = track_ed->control_mode;
					}
					track_ed->control_mode=control_mode;
				}
			}
		}
		else
		{
			track_ed->control_mode=control_mode;
		}
		if (update_menu)
		{
			XtVaGetValues(track_ed->control_mode_rowcol,
				XmNnumChildren,&num_children,XmNchildren,&child_list,NULL);
			for (i=0;i<num_children;i++)
			{
				XtVaGetValues(child_list[i],XmNuserData,&dummy,NULL);
				if ((enum Tracking_editor_control_mode)dummy == track_ed->control_mode)
				{
					XmToggleButtonSetState(child_list[i],True,False);
				}
				else
				{
					XmToggleButtonSetState(child_list[i],False,False);
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
				"tracking_editor_set_control_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* tracking_editor_set_control_mode */

static void tracking_editor_control_mode_cb(Widget widget,XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 March 1998

DESCRIPTION :
==============================================================================*/
{
	struct Tracking_editor_dialog *track_ed;
	Widget control_mode_button;
	XtPointer dummy;

	ENTER(tracking_editor_control_mode_cb);
	if (widget&&(track_ed=(struct Tracking_editor_dialog *)client_data))
	{
		/* get the widget from the call data */
		if (control_mode_button=((XmRowColumnCallbackStruct *)call_data)->widget)
		{
			if (XmToggleButtonGetState(control_mode_button))
			{
				XtVaGetValues(control_mode_button,XmNuserData,&dummy,NULL);
				tracking_editor_set_control_mode(track_ed,
					(enum Tracking_editor_control_mode)dummy,0);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"tracking_editor_control_mode_cb.  "
				"Could not find the button pressed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_control_mode_cb.  Invalid argument(s)");
	}
	LEAVE;
} /* tracking_editor_control_mode_cb */

static void tracking_editor_destroy_cb(Widget widget,
	XtPointer track_ed_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Tidys up when the user destroys the map dialog box.
==============================================================================*/
{
	struct Tracking_editor_dialog *track_ed;

	ENTER(tracking_editor_destroy_cb);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (track_ed=(struct Tracking_editor_dialog *)track_ed_void)
	{
		destroy_Shell_list_item_from_shell (&(track_ed->shell),
			track_ed->user_interface );

		if (track_ed->node_manager_callback_id)
		{
			MANAGER_DEREGISTER(FE_node)(
				track_ed->node_manager_callback_id,
				track_ed->node_manager);
			track_ed->node_manager_callback_id=NULL;
		}
		if (track_ed->node_group_manager_callback_id)
		{
			MANAGER_DEREGISTER(GROUP(FE_node))(
				track_ed->node_group_manager_callback_id,
				track_ed->node_group_manager);
			track_ed->node_group_manager_callback_id=NULL;
		}

		if (track_ed->digitiser_window_manager)
		{
			REMOVE_ALL_OBJECTS_FROM_MANAGER(Digitiser_window)(
				track_ed->digitiser_window_manager);
			DESTROY(MANAGER(Digitiser_window))(&(track_ed->digitiser_window_manager));
		}

		if (track_ed->mirage_movie)
		{
			DESTROY(Mirage_movie)(&(track_ed->mirage_movie));
		}

		/* deaccess defaults */
		if (track_ed->default_graphical_material)
		{
			DEACCESS(Graphical_material)(&(track_ed->default_graphical_material));
		}
		if (track_ed->default_light)
		{
			DEACCESS(Light)(&(track_ed->default_light));
		}
		if (track_ed->default_light_model)
		{
			DEACCESS(Light_model)(&(track_ed->default_light_model));
		}
		if (track_ed->default_scene)
		{
			DEACCESS(Scene)(&(track_ed->default_scene));
		}
		if (track_ed->default_spectrum)
		{
			DEACCESS(Spectrum)(&(track_ed->default_spectrum));
		}

		if (track_ed->address)
		{
			*(track_ed->address) = (struct Tracking_editor_dialog *)NULL;
		}

		DEALLOCATE(track_ed);
	}
	else
	{
		display_message(ERROR_MESSAGE,"tracking_editor_destroy_cb()"
			" Missing tracking_editor_dialog");
	}

	LEAVE;
} /* tracking_editor_destroy_cb */

static void tracking_editor_bar_chart_initialize_callback(
	Widget drawing_widget,XtPointer track_ed_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 3 April 1998

DESCRIPTION :
This is the configuration callback for the GL widget.
==============================================================================*/
{
	struct Tracking_editor_dialog *track_ed;

	ENTER(tracking_editor_bar_chart_initialize_callback);
	USE_PARAMETER(drawing_widget);
	USE_PARAMETER(call_data);
	if (track_ed=(struct Tracking_editor_dialog *)track_ed_void)
	{
		/* initialize graphics library to load XFont */
		initialize_graphics_library(track_ed->user_interface);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_bar_chart_initialize_callback.  Missing track_ed");
	}
	LEAVE;
} /* tracking_editor_bar_chart_initialize_callback */

static void tracking_editor_bar_chart_expose_callback(Widget drawing_widget,
	XtPointer track_ed_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 3 April 1998

DESCRIPTION :
Redraws the bar chart if there are no more expose events pending.
==============================================================================*/
{
	struct Tracking_editor_dialog *track_ed;
	XExposeEvent *expose_event;
	X3dThreeDDrawCallbackStruct *expose_callback_data;

	ENTER(tracking_editor_bar_chart_expose_callback);
	if (drawing_widget&&
		(track_ed=(struct Tracking_editor_dialog *)track_ed_void)&&
		(expose_callback_data=(X3dThreeDDrawCallbackStruct *)call_data)&&
		(X3dCR_EXPOSE==expose_callback_data->reason)&&
		(expose_event=(XExposeEvent *)(expose_callback_data->event)))
	{
		/* if no more expose events in series */
		if (0==expose_event->count)
		{
			tracking_editor_draw_bar_chart(track_ed);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_bar_chart_expose_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* tracking_editor_bar_chart_expose_callback */

enum Tracking_editor_drag_mode
{
	TRACK_ED_DRAG_NOTHING,
	TRACK_ED_DRAG_FRAME,
	TRACK_ED_DRAG_MARK,
	TRACK_ED_DRAG_TRANSLATE,
	TRACK_ED_DRAG_ZOOM,
	TRACK_ED_DRAG_ZOOM_WIDTH,
	TRACK_ED_DRAG_MARKBADONLY,
	TRACK_ED_DRAG_UNMARK
};

static void tracking_editor_bar_chart_input_callback(Widget drawing_widget,
	XtPointer track_ed_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 April 1998

DESCRIPTION :
==============================================================================*/
{
	static char tmp_string[20];
	Dimension height,width; /* X widget dimensions */
	double dx,dy,zoom_ratio,fact;
	static enum Tracking_editor_drag_mode drag_mode=TRACK_ED_DRAG_NOTHING;
	static int old_pointer_x,old_pointer_y;
	int pointer_x,pointer_y,i,frame_no,index,mark_left,mark_right,mark_all_nodes,
		old_current_frame_no;
	static int mark_frame_no,mark_node_no,mark_x,mark_min_x,mark_max_x,
		mark_min_y,mark_max_y,last_frame_no_read;
	struct Node_status *node_status;
	struct Mirage_movie *movie;
	struct Select_node_frame_data select_data;
	struct Tracking_editor_dialog *track_ed;
	X3dThreeDDrawCallbackStruct *input_callback_data;
	XButtonEvent *button_event;
	XEvent *event;
	XKeyEvent *key_event;
	XMotionEvent *motion_event;
	/*
	int charcount;
	*/

	ENTER(tracking_editor_bar_chart_input_callback);
	USE_PARAMETER(drawing_widget);
	if ((track_ed=(struct Tracking_editor_dialog *)track_ed_void)&&
		(input_callback_data=(X3dThreeDDrawCallbackStruct *)call_data)&&
		(X3dCR_INPUT==input_callback_data->reason)&&
		(event=(XEvent *)(input_callback_data->event)))
	{
		if (movie=(struct Mirage_movie *)track_ed->mirage_movie)
		{
			X3dThreeDDrawingMakeCurrent(track_ed->drawing_widget);
			XtVaGetValues(track_ed->drawing_widget,
				XmNwidth,&width,XmNheight,&height,NULL);
			switch (event->type)
			{
			case ButtonPress:
				{
					/* must not be able to change drag_mode mid-way since this will
						screw up DRAG_FRAME */
					if (TRACK_ED_DRAG_NOTHING==drag_mode)
					{
						button_event=&(event->xbutton);
						pointer_x=button_event->x;
						pointer_y=button_event->y;
						switch (button_event->button)
						{
						case Button1:
							{
								/*printf("Button 1 pressed at %i %i!\n",
									pointer_x,pointer_y);*/
								if (pointer_x>bc_scale_width)
								{
									/* change frame */
									frame_no=(int)(track_ed->bc_left+(pointer_x-bc_scale_width)/
										track_ed->bc_pixels_per_unit_x);
									if (frame_no < movie->start_frame_no)
									{
										frame_no=movie->start_frame_no;
									}
									if (frame_no >=
										movie->start_frame_no+movie->number_of_frames)
									{
										frame_no=movie->start_frame_no+movie->number_of_frames-1;
									}
									if (pointer_y<bc_scale_height)
									{
										/* change of frame */
										/*printf("Read frame %i...\n",frame_no);*/
										/* !!!WARNING: Take care writing frame change code
											otherwise good frames may be written over! */
										if (Mirage_movie_write_frame_nodes(
											movie,movie->current_frame_no))
										{
											if (Mirage_movie_read_frame_nodes(movie,frame_no))
											{
												drag_mode=TRACK_ED_DRAG_FRAME;
												last_frame_no_read=frame_no;
											}
											else
											{
												/* go back to the last one that was OK */
												Mirage_movie_read_frame_nodes(movie,
													movie->current_frame_no);
												last_frame_no_read=movie->current_frame_no;
											}
										}
									}
									else
									{
										/* may not change pending ranges during processing */
										if (!track_ed->processing || track_ed->control_mode == MAKE_GOOD_MODE
											|| track_ed->control_mode == MAKE_BAD_MODE )
										{
											/* get node_no clicked on for marking */
											index=(int)(track_ed->bc_top-(pointer_y-bc_scale_height)/
												track_ed->bc_pixels_per_unit_y);
											if (node_status=FIRST_OBJECT_IN_LIST_THAT(Node_status)(
												Node_status_is_at_index,(void *)&index,
												movie->placed_list))
											{
												mark_frame_no=frame_no;
												if (Mod1Mask & button_event->state)
												{
													drag_mode=TRACK_ED_DRAG_UNMARK;
												}
												else
												{
													if (ControlMask & button_event->state )
													{
														drag_mode=TRACK_ED_DRAG_MARKBADONLY;
													}
													else
													{
														drag_mode=TRACK_ED_DRAG_MARK;
													}
												}
												mark_node_no=Node_status_get_node_no(node_status);
												mark_x=pointer_x;
												mark_min_x=mark_max_x=pointer_x;
												mark_min_y=mark_max_y=pointer_y;
											}
										}
									}
								}
							} break;
						case Button2:
							{
								drag_mode=TRACK_ED_DRAG_TRANSLATE;
							} break;
						case Button3:
							{
								if (ShiftMask&(button_event->state))
								{
									drag_mode=TRACK_ED_DRAG_ZOOM_WIDTH;
								}
								else
								{
									drag_mode=TRACK_ED_DRAG_ZOOM;
								}
							} break;
						default:
							{
							} break;
						}
						old_pointer_x=pointer_x;
						old_pointer_y=pointer_y;
					}
				} break;
			case MotionNotify:
				{
					motion_event= &(event->xmotion);
					pointer_x=motion_event->x;
					pointer_y=motion_event->y;
					switch (drag_mode)
					{
					case TRACK_ED_DRAG_NOTHING:
						{
						} break;
					case TRACK_ED_DRAG_MARK:
					case TRACK_ED_DRAG_MARKBADONLY:
					case TRACK_ED_DRAG_UNMARK:
						{
							if (pointer_x<mark_min_x)
							{
								mark_min_x=pointer_x;
							}
							if (pointer_x>mark_max_x)
							{
								mark_max_x=pointer_x;
							}
							if (pointer_y<mark_min_y)
							{
								mark_min_y=pointer_y;
							}
							if (pointer_y>mark_max_y)
							{
								mark_max_y=pointer_y;
							}
						} break;
					case TRACK_ED_DRAG_FRAME:
						{
							/* change frame */
							frame_no=(int)(track_ed->bc_left+
								(pointer_x-bc_scale_width)/track_ed->bc_pixels_per_unit_x);
							if (frame_no < movie->start_frame_no)
							{
								frame_no=movie->start_frame_no;
							}
							if (frame_no >= movie->start_frame_no+
								movie->number_of_frames)
							{
								frame_no = movie->start_frame_no+
									movie->number_of_frames-1;
							}
							if (frame_no != last_frame_no_read)
							{
								if (Mirage_movie_read_frame_nodes(movie,frame_no))
								{
									last_frame_no_read=frame_no;
								}
								else
								{
									/* go back to the last one that was OK */
									Mirage_movie_read_frame_nodes(movie,last_frame_no_read);
								}
								sprintf(tmp_string,"%i",last_frame_no_read);
								XtVaSetValues(track_ed->frame_text,XmNvalue,tmp_string,NULL);
							}
						} break;
					case TRACK_ED_DRAG_TRANSLATE:
						{
							dx=(pointer_x-old_pointer_x)/track_ed->bc_pixels_per_unit_x;
							dy=(old_pointer_y-pointer_y)/track_ed->bc_pixels_per_unit_y;
							track_ed->bc_left -= dx;
							track_ed->bc_top -= dy;
#if defined (OLD_CODE)
							if (track_ed->bc_left < track_ed->bc_left_limit)
							{
								track_ed->bc_left=movie->start_frame_no;
							}
							if (track_ed->bc_top > track_ed->bc_top_limit)
							{
								track_ed->bc_top=track_ed->bc_top_limit;
							}
#endif /* defined (OLD_CODE) */
							tracking_editor_update_bar_chart(track_ed);
						} break;
					case TRACK_ED_DRAG_ZOOM:
						{
							zoom_ratio=1.0;
							fact=1.01;
							i=pointer_y;
							while (i>old_pointer_y)
							{
								zoom_ratio *= fact;
								i--;
							}
							while (i<old_pointer_y)
							{
								zoom_ratio /= fact;
								i++;
							}
							/* limit vertical pixels per unit to >= 1 */
							if (1.0 > track_ed->bc_pixels_per_unit_y*zoom_ratio)
							{
								zoom_ratio=1.0/track_ed->bc_pixels_per_unit_y;
							}
							track_ed->bc_pixels_per_unit_x *= zoom_ratio;
							track_ed->bc_pixels_per_unit_y *= zoom_ratio;
							/* adjust top,left so that zoom comes from centre of viewport */
							if (0 < width-bc_scale_width)
							{
								track_ed->bc_left += 0.5*(zoom_ratio-1.0)*
									((width-bc_scale_width)/track_ed->bc_pixels_per_unit_x);
							}
							if (0 < height-bc_scale_height)
							{
								track_ed->bc_top -= 0.5*(zoom_ratio-1.0)*
									((height-bc_scale_height)/track_ed->bc_pixels_per_unit_y);
							}
							tracking_editor_update_bar_chart(track_ed);
						} break;
						case TRACK_ED_DRAG_ZOOM_WIDTH:
						{
							zoom_ratio=1.0;
							fact=1.01;
							i=pointer_x;
							while (i>old_pointer_x)
							{
								zoom_ratio *= fact;
								i--;
							}
							while (i<old_pointer_x)
							{
								zoom_ratio /= fact;
								i++;
							}
							if (track_ed->bc_pixels_per_unit_x*zoom_ratio <
								track_ed->bc_pixels_per_unit_y)
							{
								zoom_ratio=track_ed->bc_pixels_per_unit_y/
									track_ed->bc_pixels_per_unit_x;
							}
							track_ed->bc_pixels_per_unit_x *= zoom_ratio;
							/* adjust left so that zoom comes from centre of viewport */
							if (0 < width-bc_scale_width)
							{
								track_ed->bc_left += 0.5*(zoom_ratio-1.0)*
									((width-bc_scale_width)/track_ed->bc_pixels_per_unit_x);
							}
							tracking_editor_update_bar_chart(track_ed);
#if defined (OLD_CODE)
							aspect=current_aspect=(int)((track_ed->bc_pixels_per_unit_x+0.5)/
								track_ed->bc_pixels_per_unit_y);
							if (50 <= pointer_x-old_pointer_x)
							{
								aspect++;
							}
							if ((1 < aspect)&&(50 <= old_pointer_x-pointer_x))
							{
								aspect--;
							}
							if (aspect != current_aspect)
							{
								track_ed->bc_pixels_per_unit_x=
									aspect*track_ed->bc_pixels_per_unit_y;
								tracking_editor_update_bar_chart(track_ed);
							}
							else
							{
								pointer_x=old_pointer_x;
							}
#endif /* defined (OLD_CODE) */
						} break;
						default:
						{
						} break;
					}
					old_pointer_x=pointer_x;
					old_pointer_y=pointer_y;
				} break;
				case ButtonRelease:
				{
					button_event=&(event->xbutton);
					/* printf("button %d release at %d %d\n",button_event->button,
						button_event->x,button_event->y); */
					pointer_x=button_event->x;
					pointer_y=button_event->y;
					switch (drag_mode)
					{
					case TRACK_ED_DRAG_MARK:
					case TRACK_ED_DRAG_MARKBADONLY:
					case TRACK_ED_DRAG_UNMARK:
						{
							/* gesture input: */
							/* drag up or down enough and all nodes are selected */
							mark_all_nodes=(200 < mark_max_y-mark_min_y);
							/* drag left enough and left ranges are selected */
							mark_left=(100 < mark_x-mark_min_x);
							/* drag right enough and right ranges are selected */
							mark_right=(100 < mark_max_x-mark_x);
							if (mark_all_nodes)
							{
								select_data.track_ed=track_ed;
								select_data.frame_no=mark_frame_no;
								select_data.use_left_range=mark_left;
								select_data.use_right_range=mark_right;
								switch( drag_mode )
								{
									case TRACK_ED_DRAG_MARK:
									{
										select_data.select_mode = TRACK_ED_SELECT_DEFAULT;
									} break;
									case TRACK_ED_DRAG_UNMARK:
									{
										select_data.select_mode = TRACK_ED_SELECT_UNMARK;
									} break;
									case TRACK_ED_DRAG_MARKBADONLY:
									{
										select_data.select_mode = TRACK_ED_SELECT_BADONLY;
									} break;
								}
								FOR_EACH_OBJECT_IN_LIST(Node_status)(
									tracking_editor_iterator_select_node_frame,
									(void *)&select_data,movie->placed_list);
								switch (track_ed->control_mode)
								{
									case TRACK_MODE:
									case BACKTRACK_MODE:
									case SUBSTITUTE_MODE:
									case INTERPOLATE_MODE:
									{
										tracking_editor_refresh_pending_groups(track_ed);
									} break;
									case MAKE_GOOD_MODE:
									case MAKE_BAD_MODE:
									{
										tracking_editor_refresh_problem_groups(track_ed);
									} break;
									default:
									{
										display_message(ERROR_MESSAGE,
											"tracking_editor_bar_chart_input_callback.  Unknown control mode");
									} break;
								}
							}
							else
							{
								switch(drag_mode)
								{
									case TRACK_ED_DRAG_MARK:
									{
										tracking_editor_select_node_frame(track_ed,
											mark_node_no,mark_frame_no,mark_left,mark_right,
											TRACK_ED_SELECT_DEFAULT);
									} break;
									case TRACK_ED_DRAG_UNMARK:
									{
										tracking_editor_select_node_frame(track_ed,
											mark_node_no,mark_frame_no,mark_left,mark_right,
											TRACK_ED_SELECT_UNMARK);
									} break;
									case TRACK_ED_DRAG_MARKBADONLY:
									{
										tracking_editor_select_node_frame(track_ed,
											mark_node_no,mark_frame_no,mark_left,mark_right,
											TRACK_ED_SELECT_BADONLY);
									} break;
								}
								switch (track_ed->control_mode)
								{
									case TRACK_MODE:
									case BACKTRACK_MODE:
									case SUBSTITUTE_MODE:
									case INTERPOLATE_MODE:
									{
										tracking_editor_node_pending_change(track_ed,mark_node_no);
									} break;
									case MAKE_GOOD_MODE:
									case MAKE_BAD_MODE:
									{
										tracking_editor_node_problem_change(track_ed,mark_node_no);
									} break;
									default:
									{
										display_message(ERROR_MESSAGE,
											"tracking_editor_bar_chart_input_callback.  Unknown control mode");
									} break;
								}
							}
							tracking_editor_update_bar_chart(track_ed);
						} break;
					case TRACK_ED_DRAG_FRAME:
						{
							if (last_frame_no_read != movie->current_frame_no)
							{
								busy_cursor_on((Widget)NULL, track_ed->user_interface );
								old_current_frame_no=movie->current_frame_no;
								if (!read_Mirage_movie_frame(movie,last_frame_no_read))
								{
									read_Mirage_movie_frame(movie,old_current_frame_no);
									last_frame_no_read=movie->current_frame_no;
								}
								/* update views in all digitiser windows so textures correctly
									 displayed */
								FOR_EACH_OBJECT_IN_MANAGER(Digitiser_window)(
									Digitiser_window_update_view,(void *)NULL,
									track_ed->digitiser_window_manager);

								/* SAB Save the current pending lists in case we need to revert */
								Mirage_movie_write_node_status_lists(movie,
									"_last_frame_change");

								/* reshow the current frame number */
								Mirage_movie_refresh_node_groups(movie);
								sprintf(tmp_string,"%i",movie->current_frame_no);
								XtVaSetValues(track_ed->frame_text,XmNvalue,tmp_string,NULL);
								tracking_editor_update_bar_chart(track_ed);
								busy_cursor_off((Widget)NULL, track_ed->user_interface );
							}
						} break;
					default:
						{
							/* do nothing */
						} break;
					}
					drag_mode=TRACK_ED_DRAG_NOTHING;
				} break;
				case KeyPress:
				{
					key_event= &(event->xkey);
					printf("Track_ed_bc key %d press at %d %d\n",key_event->keycode,
						key_event->x,key_event->y);
#if defined (OLD_CODE)
					charcount=XLookupString(key_event,buffer,bufsize,&keysym,&compose);
					switch (keysym)
					{
						case XK_Delete:
						{
							printf("* Delete pressed!\n");
						} break;
					}
#endif /* defined (OLD_CODE) */
				} break;
				case KeyRelease:
				{
					key_event= &(event->xkey);
					printf("Track_ed_bc key %d release at %d %d\n",key_event->keycode,
						key_event->x,key_event->y);
				} break;
				default:
				{
					printf("tracking_editor_bar_chart_input_callback.  Invalid X event");
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tracking_editor_bar_chart_input_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* tracking_editor_bar_chart_input_callback */

/*
Global functions
----------------
*/
void tracking_editor_close_cb(Widget widget_id,XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 27 March 1998

DESCRIPTION :
Closes the dialog window, and any children dialogs that may be open.
???RC Why is this global?
==============================================================================*/
{
	struct Tracking_editor_dialog *track_ed;

	ENTER(tracking_editor_close_cb);
	USE_PARAMETER(widget_id);
	USE_PARAMETER(call_data);
	if (track_ed=(struct Tracking_editor_dialog *)client_data)
	{
		XtUnmanageChild(track_ed->dialog);
#if defined (OLD_CODE)
		XtDestroyWidget((*scene_viewer)->drawing_widget);
#endif /* defined (OLD_CODE) */
	}
	else
	{
		display_message(INFORMATION_MESSAGE,
			"tracking_editor_close_cb.  Invalid argument(s)");
	}
	LEAVE;
} /* tracking_editor_close_cb */

int open_tracking_editor_dialog(struct Tracking_editor_dialog **address,
	XtCallbackProc exit_button_callback,
	struct Colour *background_colour,
	struct MANAGER(FE_basis) *basis_manager,
	struct MANAGER(Computed_field) *computed_field_manager,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct MANAGER(FE_field) *fe_field_manager,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_graphical_material,
	struct MANAGER(Graphics_window) *graphics_window_manager,
	struct MANAGER(Light) *light_manager,
	struct Light *default_light,
	struct MANAGER(Light_model) *light_model_manager,
	struct Light_model *default_light_model,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(FE_node) *data_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct FE_element_selection *element_selection,
	struct FE_node_selection *node_selection,
	struct FE_node_selection *data_selection,
	struct MANAGER(Scene) *scene_manager,
	struct Scene *default_scene,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *default_spectrum,
	struct MANAGER(Texture) *texture_manager,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 28 April 2000

DESCRIPTION :
==============================================================================*/
{
	Atom WM_DELETE_WINDOW;
	int return_code;
	MrmType dummy_class;
	static MrmRegisterArg callback_list[]=
	{
		{"track_ed_destroy_cb",(XtPointer)tracking_editor_destroy_cb},
		{"track_ed_id_file_menu",(XtPointer)
			DIALOG_IDENTIFY(tracking_editor,file_menu)},
		{"track_ed_id_edit_menu",(XtPointer)
			DIALOG_IDENTIFY(tracking_editor,edit_menu)},
		{"track_ed_id_view_2d_points",(XtPointer)
			DIALOG_IDENTIFY(tracking_editor,view_2d_points)},
		{"track_ed_id_view_2d_lines",(XtPointer)
			DIALOG_IDENTIFY(tracking_editor,view_2d_lines)},
		{"track_ed_id_view_2d_surfaces",(XtPointer)
			DIALOG_IDENTIFY(tracking_editor,view_2d_surfaces)},
		{"track_ed_id_view_3d_points",(XtPointer)
			DIALOG_IDENTIFY(tracking_editor,view_3d_points)},
		{"track_ed_id_view_3d_lines",(XtPointer)
			DIALOG_IDENTIFY(tracking_editor,view_3d_lines)},
		{"track_ed_id_view_3d_surfaces",(XtPointer)
			DIALOG_IDENTIFY(tracking_editor,view_3d_surfaces)},
		{"track_ed_id_view_node_numbers",(XtPointer)
			DIALOG_IDENTIFY(tracking_editor,view_node_numbers)},
		{"track_ed_id_frame_form",(XtPointer)
			DIALOG_IDENTIFY(tracking_editor,frame_form)},
		{"track_ed_id_frame_text",(XtPointer)
			DIALOG_IDENTIFY(tracking_editor,frame_text)},
		{"track_ed_id_frame_range",(XtPointer)
			DIALOG_IDENTIFY(tracking_editor,frame_range)},
		{"track_ed_id_control_form",(XtPointer)
			DIALOG_IDENTIFY(tracking_editor,control_form)},
		{"track_ed_id_control_mode_rc",(XtPointer)
			DIALOG_IDENTIFY(tracking_editor,control_mode_rowcol)},
		{"track_ed_id_track_btn",(XtPointer)
			DIALOG_IDENTIFY(tracking_editor,track_button)},
		{"track_ed_id_backtrack_btn",(XtPointer)
			DIALOG_IDENTIFY(tracking_editor,backtrack_button)},
		{"track_ed_id_substitute_btn",(XtPointer)
			DIALOG_IDENTIFY(tracking_editor,substitute_button)},
		{"track_ed_id_interpolate_btn",(XtPointer)
			DIALOG_IDENTIFY(tracking_editor,interpolate_button)},
		{"track_ed_id_process_btn",(XtPointer)
			DIALOG_IDENTIFY(tracking_editor,process_button)},
		{"track_ed_id_abort_btn",(XtPointer)
			DIALOG_IDENTIFY(tracking_editor,abort_button)},
		{"track_ed_id_drawing_form",(XtPointer)
			DIALOG_IDENTIFY(tracking_editor,drawing_form)},
		{"track_ed_open_movie_cb",(XtPointer)tracking_editor_open_movie_cb},
		{"track_ed_save_movie_cb",(XtPointer)tracking_editor_save_movie_cb},
		{"track_ed_write_2d_cb",(XtPointer)tracking_editor_write_2d_cb},
		{"track_ed_close_cb",(XtPointer)tracking_editor_close_cb},
		{"track_ed_clear_pending_cb",(XtPointer)tracking_editor_clear_pending_cb},
		{"track_ed_revert_cb",(XtPointer)tracking_editor_revert_cb},
		{"track_ed_view_objects_cb",(XtPointer)tracking_editor_view_objects_cb},
		{"track_ed_view_node_numbers_cb",
		(XtPointer)tracking_editor_view_node_numbers_cb},
		{"track_ed_digitiser_cb",(XtPointer)tracking_editor_digitiser_cb},
		{"track_ed_3d_window_cb",(XtPointer)tracking_editor_3d_window_cb},
		{"track_ed_frame_text_cb",(XtPointer)tracking_editor_frame_text_cb},
		{"track_ed_control_mode_cb",(XtPointer)tracking_editor_control_mode_cb},
		{"track_ed_process_cb",(XtPointer)tracking_editor_process_cb},
		{"track_ed_abort_cb",(XtPointer)tracking_editor_abort_cb}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"track_ed_structure",(XtPointer)NULL}
	};
	struct Tracking_editor_dialog *track_ed;
	Widget drawing_widget;

	ENTER(open_tracking_editor_dialog);
	USE_PARAMETER(exit_button_callback);
	if (address&&background_colour&&basis_manager&&computed_field_manager&&
		element_manager&&element_group_manager&&fe_field_manager&&
		glyph_list&&graphical_material_manager&&default_graphical_material&&
		graphics_window_manager&&light_manager&&default_light&&
		light_model_manager&&default_light_model&&
		node_manager&&node_group_manager&&data_manager&&data_group_manager&&
		element_point_ranges_selection&&element_selection&&node_selection&&
		data_selection&&
		scene_manager&&default_scene&&spectrum_manager&&default_spectrum&&
		texture_manager&&interactive_tool_manager&&user_interface)
	{
		if (!(track_ed = *address))
		{
			/* create the dialog */
			if (MrmOpenHierarchy_base64_string(tracking_editor_dialog_uidh,
				&tracking_editor_dialog_hierarchy,
				&tracking_editor_dialog_hierarchy_open))
			{
				if (ALLOCATE(track_ed,struct Tracking_editor_dialog,1))
				{
					/* initially no movie */
					track_ed->mirage_movie=
						(struct Mirage_movie *)NULL;
					track_ed->node_manager_callback_id=NULL;
					track_ed->node_group_manager_callback_id=NULL;

					track_ed->address = address;

					track_ed->background_colour=background_colour;
					track_ed->digitiser_window_manager=
						CREATE(MANAGER(Digitiser_window))();
					track_ed->basis_manager=basis_manager;
					track_ed->computed_field_manager=computed_field_manager;
					track_ed->element_manager=element_manager;
					track_ed->element_group_manager=element_group_manager;
					track_ed->fe_field_manager=fe_field_manager;
					track_ed->glyph_list=glyph_list;
					track_ed->graphical_material_manager=
						graphical_material_manager;
					track_ed->default_graphical_material=
						ACCESS(Graphical_material)(default_graphical_material);
					track_ed->graphics_window_manager=
						graphics_window_manager;
					track_ed->light_manager=light_manager;
					track_ed->default_light=ACCESS(Light)(default_light);
					track_ed->light_model_manager=light_model_manager;
					track_ed->default_light_model=
						ACCESS(Light_model)(default_light_model);
					track_ed->node_manager=node_manager;
					track_ed->node_group_manager=node_group_manager;
					track_ed->data_manager=data_manager;
					track_ed->data_group_manager=data_group_manager;
					track_ed->element_point_ranges_selection=
						element_point_ranges_selection;
					track_ed->element_selection=element_selection;
					track_ed->node_selection=node_selection;
					track_ed->data_selection=data_selection;
					track_ed->scene_manager=scene_manager;
					track_ed->default_scene=ACCESS(Scene)(default_scene);
					track_ed->spectrum_manager=spectrum_manager;
					track_ed->default_spectrum=
						ACCESS(Spectrum)(default_spectrum);
					track_ed->texture_manager=texture_manager;
					track_ed->interactive_tool_manager=interactive_tool_manager;
					track_ed->user_interface = user_interface;
					track_ed->process_ID=0;
					track_ed->processing=0;

					track_ed->input_id=0;
					track_ed->process_remote_client=0;
					track_ed->process_socket=0;

					track_ed->kill_process_interval_id=0;
					track_ed->kill_attempts=0;

					/* default values for process mode settings */
					track_ed->tracking_tolerance = MEDIUM_TOLERANCE;
					track_ed->tracking_search_radius = 16;
					track_ed->tracking_port = getPortFromProtocol("cmgui-xvg");
					/* clear widgets */
					track_ed->file_menu=(Widget)NULL;
					track_ed->edit_menu=(Widget)NULL;
					track_ed->view_2d_points=(Widget)NULL;
					track_ed->view_2d_lines=(Widget)NULL;
					track_ed->view_2d_surfaces=(Widget)NULL;
					track_ed->view_3d_points=(Widget)NULL;
					track_ed->view_3d_lines=(Widget)NULL;
					track_ed->view_3d_surfaces=(Widget)NULL;
					track_ed->view_node_numbers=(Widget)NULL;
					track_ed->frame_form=(Widget)NULL;
					track_ed->frame_text=(Widget)NULL;
					track_ed->frame_range=(Widget)NULL;
					track_ed->control_form=(Widget)NULL;
					track_ed->control_mode_rowcol=(Widget)NULL;
					track_ed->process_button=(Widget)NULL;
					track_ed->abort_button=(Widget)NULL;
					track_ed->drawing_form=(Widget)NULL;
					track_ed->drawing_widget=(Widget)NULL;
					track_ed->dialog=(Widget)NULL;
					track_ed->track_button=(Widget)NULL;
					track_ed->backtrack_button=(Widget)NULL;
					track_ed->substitute_button=(Widget)NULL;
					track_ed->interpolate_button=(Widget)NULL;

					/* create shell */
					if (track_ed->shell = XtVaCreatePopupShell(
						"tracking_editor_dialog_shell",xmDialogShellWidgetClass,
						user_interface->application_shell,
						XmNmwmDecorations,MWM_DECOR_ALL,
						XmNmwmFunctions,MWM_FUNC_ALL,
						XmNtitle,"Tracking Editor",
						NULL))
					{
						/* Register the shell for the busy cursor */
						create_Shell_list_item(&(track_ed->shell),
							user_interface);

						/* Set up window manager callback for close window message */
						WM_DELETE_WINDOW=XmInternAtom(
							XtDisplay(track_ed->shell),
							"WM_DELETE_WINDOW",False);
						XmAddWMProtocolCallback(track_ed->shell,
							WM_DELETE_WINDOW,tracking_editor_close_cb,track_ed);

						/* register the callbacks */
						if (MrmSUCCESS == MrmRegisterNamesInHierarchy(
							tracking_editor_dialog_hierarchy,callback_list,
							XtNumber(callback_list)))
						{
							/* register the identifiers */
							identifier_list[0].value = track_ed;
							if (MrmSUCCESS == MrmRegisterNamesInHierarchy(
								tracking_editor_dialog_hierarchy,identifier_list,
								XtNumber(identifier_list)))
							{
								/* fetch the widget */
								if (MrmSUCCESS == MrmFetchWidget(
									tracking_editor_dialog_hierarchy,"tracking_editor_dialog",
									track_ed->shell,
									&(track_ed->dialog),&dummy_class))
								{
									/* create the Three_D_drawing widget */
									if (drawing_widget=
										XtVaCreateWidget("tracking_editor_bar_chart",
										threeDDrawingWidgetClass,
										track_ed->drawing_form,
										X3dNbufferColourMode,X3dCOLOUR_RGB_MODE,
										X3dNbufferingMode,X3dDOUBLE_BUFFERING,
										XmNleftAttachment,XmATTACH_FORM,
										XmNrightAttachment,XmATTACH_FORM,
										XmNbottomAttachment,XmATTACH_FORM,
										XmNtopAttachment,XmATTACH_FORM,
										NULL))
									{
										track_ed->drawing_widget=drawing_widget;
										/* add callbacks to the drawing widget */
										XtAddCallback(drawing_widget,X3dNinitializeCallback,
											tracking_editor_bar_chart_initialize_callback,
											track_ed);
										/*XtAddCallback(drawing_widget,X3dNresizeCallback,
											tracking_editor_bar_chart_resize_callback,
											track_ed);*/
										XtAddCallback(drawing_widget,X3dNexposeCallback,
											tracking_editor_bar_chart_expose_callback,
											track_ed);
										XtAddCallback(drawing_widget,X3dNinputCallback,
											tracking_editor_bar_chart_input_callback,
											track_ed);
										XtManageChild(drawing_widget);

										/* initialize */
										track_ed->control_mode=TRACK_MODE;
										track_ed->previous_control_mode=TRACK_MODE;
										tracking_editor_set_control_mode(track_ed,TRACK_MODE,1);
										/* need the host name to pass to xvg child process */
										gethostname(track_ed->host,BUFFER_SIZE);
										gethostname(track_ed->remote_host,BUFFER_SIZE);

										/* don't want frame stuff visible until movie read in */
										XtUnmanageChild(track_ed->frame_form);
										XtManageChild(track_ed->dialog);
										/* turn off abort button until processing */
										XtSetSensitive(track_ed->abort_button,False);
										XtRealizeWidget(track_ed->shell);
										/* want only some graphics visible at the start */
										XmToggleButtonSetState(track_ed->view_2d_points,  True, False);
										XmToggleButtonSetState(track_ed->view_2d_lines,   True, False);
										XmToggleButtonSetState(track_ed->view_2d_surfaces,False,False);
										XmToggleButtonSetState(track_ed->view_3d_points,  False,False);
										XmToggleButtonSetState(track_ed->view_3d_lines,   True, False);
										XmToggleButtonSetState(track_ed->view_3d_surfaces,True, False);

										if (address)
										{
											*address = track_ed;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"open_tracking_editor_dialog.  "
											"Could not create the drawing widget");
										DEALLOCATE(track_ed);
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"open_tracking_editor_dialog.  Could not fetch the widget");
									DEALLOCATE(track_ed);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"open_tracking_editor_dialog.  "
									"Could not register identifiers");
								DEALLOCATE(track_ed);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"open_tracking_editor_dialog.  "
								"Could not register callbacks");
							DEALLOCATE(track_ed);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"open_tracking_editor_dialog.  "
							"Could not create shell");
						DEALLOCATE(track_ed);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"open_tracking_editor_dialog.  "
						"Insufficient memory for dialog data");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"open_tracking_editor_dialog.  Could not open hierarchy");
			}
		}

		if (track_ed)
		{
			XtManageChild(track_ed->dialog);
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"open_tracking_editor_dialog.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* open_tracking_editor_dialog */


#if defined (STANDALONE_SOCKET_TEST)
int main ()
{
	char remote_host[BUFFER_SIZE], sys_command[4*BUFFER_SIZE];
	int addressLength, error_code, process_remote_client, return_code, socket;
	struct sockaddr_in socketDescriptor;

	return_code = 0;
	sprintf(sys_command,"track.process /usr/people/blackett/cmgui/test/xvglinux/anger/tracking/anger.cmmov esu29 /usr/people/blackett/cmgui/test/xvglinux/anger/tracking/anger.cmmov_req 1 16 0");
	if (SocketInitialize(&socket, "xvg-cmgui"))
	{
		printf("running: %s\n",sys_command);
		if (error_code=system(sys_command))
		{
			display_message(ERROR_MESSAGE,"tracking_editor_process_cb.  "
				"error executing process: error code=%d",error_code);
			close(socket);
		}
		else
		{
			/* wait for the remote host to connect */
			printf("SocketInitialize() : Waiting for the remote host to connect...\n");
			addressLength = sizeof(socketDescriptor);
			memset(&socketDescriptor,0,addressLength);
			/*???debug*/fprintf(stderr,"process_cb #1\n");
			if (-1==(process_remote_client=
				accept(socket,
					(struct sockaddr *)&socketDescriptor,&addressLength)))
			{
				display_message(ERROR_MESSAGE,
					"tracking_editor_process_cb.  accept()");
				/* tracking_editor_kill_process(track_ed); */
				close(socket);
			}
			else
			{
				sprintf(remote_host,"%d.%d.%d.%d",
					(socketDescriptor.sin_addr.s_addr & 0xff000000) >> 24,
					(socketDescriptor.sin_addr.s_addr & 0x00ff0000) >> 16,
					(socketDescriptor.sin_addr.s_addr & 0x0000ff00) >> 8,
					(socketDescriptor.sin_addr.s_addr & 0x000000ff));
				printf("SocketInitialize() : host at IP address %s has connected...\n",
					remote_host);
			
				/*???debug*/fprintf(stderr,"process_cb #2\n");
				/* request timeout to check socket messages */
				/* tracking_editor_enter_process_mode(track_ed); */
				/* XtAppAddTimeOut(track_ed->user_interface->application_context,
					20,tracking_editor_process_input_cb,(XtPointer)track_ed); */
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"main.STANDALONE_SOCKET_TEST  (Unable to initialise socket)");
		/* tracking_editor_kill_process(track_ed); */
		close(socket);
	}

	return(return_code);
}
#endif /* defined (STANDALONE_SOCKET_TEST) */
