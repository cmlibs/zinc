/*******************************************************************************
FILE : write_to_video.c

LAST MODIFIED : 6 October 1998

DESCRIPTION :
Functions for writing images through a socket to another process which will in
turn write them to video.
???DB.  Should use cmiss/Jack's socket routines ?
???DB.  Have enclosed routines involving sockets in #if defined (GL_API) to get
	working on alpha.  Should probably use something else.
==============================================================================*/
/*???DB.  select is not defined for POSIX / _XOPEN_SOURCE .  What should be done
	instead ?  Have #define _X_OPEN_SOURCE for the modules that do conform ? */
#undef _XOPEN_SOURCE
#undef _ANSI_C_SOURCE

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#if defined (GL_API)
#include <unistd.h>
	/* for sleep(), write(), read(), etc... */
#include <signal.h>
#include <X11/Intrinsic.h>
#include <string.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#if defined (IBM)
#include <sys/select.h>
#else
#include <bstring.h>
#endif
#include <netinet/in.h>
#include <netdb.h>
#endif
#include "general/debug.h"
#include "graphics/graphics_library.h"
#include "graphics/write_to_video.h"
#include "user_interface/message.h"

/*
Module constants
----------------
*/
#define SOCKET_BLOCK_SIZE 512
#define WRITE_TO_VIDEO_PORT 2500

/*
Module variables
----------------
*/
/* descriptor for the video socket */
static int video_socket=0;
/* boundaries of screen window */
int bottom,left,right,top;
/* the graphics window being used */
struct Graphics_window *video_graphics_window=(struct Graphics_window *)NULL;

/*
Module functions
----------------
*/
static int open_video_socket(int *width,int *height)
/*******************************************************************************
LAST MODIFIED : 14 November 1994

DESCRIPTION :
Trys to create a socket and connect to the process which is writing the images
to video.  If it connects to the video process it returns the <width> and
<height> of the video frame.
==============================================================================*/
{
	int return_code;
#if defined (GL_API)
	fd_set socket_set;
	int socket_ready;
	struct hostent *host_data;
	struct sockaddr_in server;
	struct timeval time_out;
#endif

	ENTER(open_video_socket);
#if defined (GL_API)
	/* create endpoint for communication */
	if (-1!=(video_socket=socket(AF_INET,SOCK_STREAM,0)))
	{
printf("created socket\n");
		/* find esu4 (machine with the connection to video recorder) */
		if (host_data=gethostbyname("esu4"))
		{
			memmove(&server.sin_addr,host_data->h_addr,host_data->h_length);
			server.sin_port=htons(WRITE_TO_VIDEO_PORT);
			server.sin_family=AF_INET;
			/* connect to the process writing images to video */
			if (0==connect(video_socket,&server,sizeof(server)))
			{
printf("connected to process on esu4\n");
				/* wait until the socket is ready for writing */
				do
				{
					FD_ZERO(&socket_set);
					FD_SET(video_socket,&socket_set);
					time_out.tv_sec=0;
					time_out.tv_usec=0;
				} while (0==(socket_ready=select(video_socket+1,(fd_set *)NULL,
					&socket_set,(fd_set *)NULL,&time_out)));
				/* send start message */
				if ((1==socket_ready)&&FD_ISSET(video_socket,&socket_set)&&
					(1==write(video_socket,"s",1)))
				{
printf("wrote start message\n");
					/* wait until the socket is ready for reading */
					do
					{
						FD_ZERO(&socket_set);
						FD_SET(video_socket,&socket_set);
						time_out.tv_sec=0;
						time_out.tv_usec=0;
					} while (0==(socket_ready=select(video_socket+1,&socket_set,
						(fd_set *)NULL,(fd_set *)NULL,&time_out)));
					if ((1==socket_ready)&&FD_ISSET(video_socket,&socket_set)&&
						(sizeof(int)==read(video_socket,(char *)width,sizeof(int)))&&
						(sizeof(int)==read(video_socket,(char *)height,sizeof(int))))
					{
printf("video width = %d, height = %d\n",*width,*height);
						return_code=1;
					}
					else
					{
printf("could not read video frame size\n");
						display_message(ERROR_MESSAGE,
							"open_video_socket.  Could not read video frame size");
						close(video_socket);
						video_socket=0;
						return_code=0;
					}
				}
				else
				{
printf("could not write start message\n");
					display_message(ERROR_MESSAGE,
						"open_video_socket.  Could not write to socket");
					close(video_socket);
					video_socket=0;
					return_code=0;
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"Could not write connect with process for writing images to video");
				close(video_socket);
				video_socket=0;
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"open_video_socket.  Could not find esu4");
			close(video_socket);
			video_socket=0;
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"open_video_socket.  Could not create sockets");
		video_socket=0;
		return_code=0;
	}
#else
	return_code=1;
#endif
	LEAVE;

	return (return_code);
} /* open_video_socket */

static int close_video_socket(void)
/*******************************************************************************
LAST MODIFIED : 14 March 1994

DESCRIPTION :
Closes the video socket.
==============================================================================*/
{
	int return_code;
#if defined (GL_API)
	fd_set socket_set;
	int socket_ready;
	struct timeval time_out;
#endif

	ENTER(close_video_socket);
#if defined (GL_API)
	if (video_socket>0)
	{
		/* wait until the socket is ready for writing */
		do
		{
			FD_ZERO(&socket_set);
			FD_SET(video_socket,&socket_set);
			time_out.tv_sec=0;
			time_out.tv_usec=0;
		} while (0==(socket_ready=select(video_socket+1,(fd_set *)NULL,&socket_set,
			(fd_set *)NULL,&time_out)));
		if ((1==socket_ready)&&FD_ISSET(video_socket,&socket_set))
		{
			/* write end message */
			write(video_socket,"e",1);
		}
		close(video_socket);
		video_socket=0;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"close_video_socket.  Video write socket is not open");
		return_code=0;
	}
#else
	return_code=1;
#endif
	LEAVE;

	return (return_code);
} /* close_video_socket */

static int write_frame_to_video(int number_of_frames,int x1,int x2,int y1,
	int y2)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Writes a frame down the socket to the process writing frames to video and tells
it to write it <number_of_frames> times to the video.
==============================================================================*/
{
	int return_code;
#if defined (GL_API)
	char *frame_buffer_block,message;
	fd_set socket_set;
	int socket_ready,window,xmaxscreen,xorg,xsize,ymaxscreen,yorg,ysize,
		local_number_of_frames;
	long int frame_buffer_size;
	long unsigned *frame_buffer;
	struct timeval time_out;
#endif

	ENTER(write_frame_to_video);
#if defined (GL_API)
	if (video_socket>0)
	{
#if defined (GL_API) && !defined (IBM) && !defined (VAX)
		xmaxscreen = getgdesc(GD_XPMAX)-1;
		ymaxscreen = getgdesc(GD_YPMAX)-1;
		foreground();
		/* Paul Haeberli.  ratfool, 7/22/91:  inserting explicit prefposition call
			to implement workaround to bug #56310 as explained to me by myoung */
		prefposition(0,xmaxscreen-1,0,ymaxscreen-1);
		noport();
		window=winopen("scrsave");
		/* check the coordinates passed in */
		if (x1<0)
		{
			x1=0;
		}
		else
		{
			if (x1>xmaxscreen)
			{
				x1=xmaxscreen;
			}
		}
		if (x2<0)
		{
			x2=0;
		}
		else
		{
			if (x2>xmaxscreen)
			{
				x2=xmaxscreen;
			}
		}
		if (y1<0)
		{
			y1=0;
		}
		else
		{
			if (y1>ymaxscreen)
			{
				y1=ymaxscreen;
			}
		}
		if (y2<0)
		{
			y2=0;
		}
		else
		{
			if (y2>ymaxscreen)
			{
				y2=ymaxscreen;
			}
		}
		if (x1>x2)
		{
			xorg=x2;
			xsize=x1-x2+1;
		}
		else
		{
			xorg=x1;
			xsize=x2-x1+1;
		}
		if (y1>y2)
		{
			yorg=ymaxscreen-y1;
			ysize=y1-y2+1;
		}
		else
		{
			yorg=ymaxscreen-y2;
			ysize=y2-y1+1;
		}
/*???debug */
printf("xsize=%d, ysize=%d, %d %d\n",xsize,ysize,xsize*ysize,
	xsize*ysize*sizeof(unsigned long));
		/* allocate memory for frame buffer */
		if (ALLOCATE(frame_buffer,long unsigned,xsize*ysize))
		{
			/* read the display */
				/*???DB.  readdisplay is relative to bottom left */
/*???debug */
printf("yorg = %d, ymaxscreen = %d\n",yorg,ymaxscreen);
printf("x1 = %d, y1 = %d, x2 = %d, y2 = %d\n",xorg,yorg,xorg+xsize-1,
	yorg+ysize-1);
			readdisplay((Screencoord)xorg,(Screencoord)yorg,
				(Screencoord)(xorg+xsize-1),(Screencoord)(yorg+ysize-1),frame_buffer,
				RD_FREEZE);
/*???debug */
printf("read display\n");
			/* wait until the socket is ready for writing */
			do
			{
				FD_ZERO(&socket_set);
				FD_SET(video_socket,&socket_set);
				time_out.tv_sec=0;
				time_out.tv_usec=0;
			} while (0==(socket_ready=select(video_socket+1,(fd_set *)NULL,
				&socket_set,(fd_set *)NULL,&time_out)));
/*???debug */
printf("socket ready\n");
			/* send the frame down the socket */
			local_number_of_frames=number_of_frames;
			if ((1==socket_ready)&&FD_ISSET(video_socket,&socket_set)&&
				(1==write(video_socket,"f",1))&&
				(sizeof(int)==write(video_socket,(char *)&local_number_of_frames,
				sizeof(int)))&&
				(sizeof(int)==write(video_socket,(char *)&xsize,sizeof(int)))&&
				(sizeof(int)==write(video_socket,(char *)&ysize,sizeof(int))))
			{
/*???debug */
printf("frame size sent\n");
				/* wait until the socket is ready for reading */
				do
				{
					FD_ZERO(&socket_set);
					FD_SET(video_socket,&socket_set);
					time_out.tv_sec=0;
					time_out.tv_usec=0;
				} while (0==(socket_ready=select(video_socket+1,&socket_set,
					(fd_set *)NULL,(fd_set *)NULL,&time_out)));
/*???debug */
printf("socket ready\n");
				if ((1==socket_ready)&&FD_ISSET(video_socket,&socket_set)&&
					(1==read(video_socket,&message,1))&&('y'==message))
				{
/*???debug */
printf("frame size ok\n");
					/* write the frame buffer block by block */
					frame_buffer_block=(char *)frame_buffer;
					frame_buffer_size=xsize*ysize*sizeof(long unsigned);
/*???debug */
printf("frame buffer size = %d\n",frame_buffer_size);
					return_code=1;
					while (return_code&&(frame_buffer_size>SOCKET_BLOCK_SIZE))
					{
						/* wait until the socket is ready for writing */
						do
						{
							FD_ZERO(&socket_set);
							FD_SET(video_socket,&socket_set);
							time_out.tv_sec=0;
							time_out.tv_usec=0;
						} while (0==(socket_ready=select(video_socket+1,(fd_set *)NULL,
							&socket_set,(fd_set *)NULL,&time_out)));
						if ((1==socket_ready)&&FD_ISSET(video_socket,&socket_set)&&
							(SOCKET_BLOCK_SIZE==write(video_socket,frame_buffer_block,
							SOCKET_BLOCK_SIZE)))
						{
							/* wait until the socket is ready for reading */
							do
							{
								FD_ZERO(&socket_set);
								FD_SET(video_socket,&socket_set);
								time_out.tv_sec=0;
								time_out.tv_usec=0;
							} while (0==(socket_ready=select(video_socket+1,&socket_set,
								(fd_set *)NULL,(fd_set *)NULL,&time_out)));
							if ((1==socket_ready)&&FD_ISSET(video_socket,&socket_set)&&
								(1==read(video_socket,&message,1))&&('y'==message))
							{
								frame_buffer_block += SOCKET_BLOCK_SIZE;
								frame_buffer_size -= SOCKET_BLOCK_SIZE;
							}
							else
							{
printf("acknowledgement error.  blocks remaining = %d\n",frame_buffer_size);
								display_message(ERROR_MESSAGE,
			"write_frame_to_video.  Error reading acknowledgement from video socket");
								return_code=0;
							}
						}
						else
						{
printf("write error.  blocks remaining = %d\n",frame_buffer_size);
							display_message(ERROR_MESSAGE,
						"write_frame_to_video.  Error writing frame block to video socket");
							return_code=0;
						}
					}
					if (return_code&&(frame_buffer_size>0))
					{
						/* wait until the socket is ready for writing */
						do
						{
							FD_ZERO(&socket_set);
							FD_SET(video_socket,&socket_set);
							time_out.tv_sec=0;
							time_out.tv_usec=0;
						} while (0==(socket_ready=select(video_socket+1,(fd_set *)NULL,
							&socket_set,(fd_set *)NULL,&time_out)));
						if ((1==socket_ready)&&FD_ISSET(video_socket,&socket_set)&&
							(frame_buffer_size==write(video_socket,
							(char *)frame_buffer_block,frame_buffer_size)))
						{
							/* wait until the socket is ready for reading */
							do
							{
								FD_ZERO(&socket_set);
								FD_SET(video_socket,&socket_set);
								time_out.tv_sec=0;
								time_out.tv_usec=0;
							} while (0==(socket_ready=select(video_socket+1,&socket_set,
								(fd_set *)NULL,(fd_set *)NULL,&time_out)));
							if ((1!=socket_ready)||!FD_ISSET(video_socket,&socket_set)||
								(1!=read(video_socket,&message,1))||('y'!=message))
							{
								display_message(ERROR_MESSAGE,
			"write_frame_to_video.  Error reading acknowledgement from video socket");
								return_code=0;
							}
						}
						else
						{
printf("write error.  blocks remaining = %d\n",frame_buffer_size);
							display_message(ERROR_MESSAGE,
						"write_frame_to_video.  Error writing frame block to video socket");
							return_code=0;
						}
					}
#if defined (CODE_FRAGMENTS)
					if (return_code)
					{
						/* wait for the recording to finish */
						if ((1==read(video_socket,&message,1))||('y'!=message))
{
	printf("finished writing frame\n");
}
else
						{
							display_message(ERROR_MESSAGE,
						"write_frame_to_video.  Error waiting for frame writing to finish");
							return_code=0;
						}
					}
#endif
				}
				else
				{
/*???debug */
printf("frame size NOT ok\n");
					display_message(ERROR_MESSAGE,
						"write_frame_to_video.  Invalid window size");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"write_frame_to_video.  Error writing to video socket");
				return_code=0;
			}
			DEALLOCATE(frame_buffer);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_frame_to_video.  Could not allocate frame buffer");
			return_code=0;
		}
		winclose(window);
#else
		display_message(WARNING_MESSAGE,
			"write_frame_to_video.  Display read only implemented for GL");
		return_code=0;
#endif
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_frame_to_video.  Video socket is not open");
		return_code=0;
	}
#else
	return_code=1;
#endif
	LEAVE;

	return (return_code);
} /* write_frame_to_video */

/*
Global functions
----------------
*/
int set_video_on_off(struct Graphics_window *graphics)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Toggles writing to video on and off.
==============================================================================*/
{
	Dimension Width,Height,width,width_1,width_2,height,height_1,height_2;
	int return_code,video_height,video_width;
	Position x1,y1,X1,Y1;
	Widget widget;

	ENTER(set_video_on_off);
	display_message(INFORMATION_MESSAGE,
		"set_video_on_off temporarily disabled\n");
#if defined (OLD_GFX_WINDOW)
	/* check argument */
	if (graphics)
	{
		if (video_graphics_window)
		{
			/* video is on.  Turn off */
			return_code=close_video_socket();
			video_graphics_window=(struct Graphics_window *)NULL;
		}
		else
		{
			/* video is off.  Turn on */
			video_graphics_window=graphics;
			/* check if a video is being made */
			if (return_code=open_video_socket(&video_width,&video_height))
			{
				/* set the window to the correct size for the video */
				width=(Dimension)video_width;
				height=(Dimension)video_height;
				XtVaGetValues(graphics->viewing_form,
					XmNwidth,&width_1,
					XmNheight,&height_1,
					NULL);
				XtVaGetValues(graphics->main_window,
					XmNwidth,&width_2,
					XmNheight,&height_2,
					NULL);
				width += width_2-width_1;
				height += height_2-height_1;
				XtVaSetValues(graphics->main_window,
					XmNwidth,width,
					XmNheight,height,
					NULL);
				XtVaSetValues(graphics->window_shell,
					XmNx,20,
					XmNy,20,
					NULL);
#if defined (OLD_CODE)
					/*???DB.  Should really be operating on the drawing area.  May need to
						change allowResize ? */
					/*???DB.  Should us video_width and video_height */
				XtVaGetValues(graphics->window_shell,
					XmNwidth,&shell_width,
					XmNheight,&shell_height,
					NULL);
				if ((shell_width!=896)||(shell_height!=/*857*/850))
				{
					XtVaSetValues(graphics->window_shell,
						XmNwidth,896,
						XmNheight,/*857*/850,
						XmNx,10,
						XmNy,30,
						NULL);
					/* wait for the GL window resize */
						/*???DB.  GL widget needs looking at */
					drawing->resize_completed=0;
					while (!drawing->resize_completed)
					{
						XtAppNextEvent(application_context,&event);
							/*???DB.  application_context should be passed ? */
						XtDispatchEvent(&event);
					}
				}
#endif
				/* get the position and size of the drawing area */
				XtVaGetValues(graphics->main_window,
					XmNx,&X1,
					XmNy,&Y1,
					NULL);
				widget=graphics->viewing_form;
				XtVaGetValues(widget,
					XmNwidth,&width,
					XmNheight,&height,
					NULL);
				while (widget!=graphics->main_window)
				{
					XtVaGetValues(widget,
						XmNx,&x1,
						XmNy,&y1,
						NULL);
					X1 += x1;
					Y1 += y1;
printf("x1 = %d, y1 = %d\n",x1,y1);
					widget=XtParent(widget);
				}
				left=(int)(X1+(width-video_width)/2);
				right=(int)(left+video_width-1);
				top=(int)(Y1+(height-video_height)/2);
				bottom=(int)(top+video_height-1);
printf("left = %d, right = %d, top = %d, bottom = %d\n",left,right,top,bottom);
XtVaGetValues(graphics->main_window,
	XmNx,&X1,
	XmNy,&Y1,
	XmNwidth,&Width,
	XmNheight,&Height,
	NULL);
printf("left = %d, width = %d, top = %d, height = %d\n",X1,Width,Y1,Height);
#if defined (CODE_FRAGMENTS)
				/* get the position and size of the main window */
				XtVaGetValues(graphics->main_window,
					XmNx,&X1,
					XmNy,&Y1,
					XmNwidth,&Width,
					XmNheight,&Height,
					NULL);
				/* get the position and size of the drawing area */
					/*???DB.  Not relative to screen.  Relative to parent widget */
				XtVaGetValues(graphics->drawing_area,
					XmNx,&x1,
					XmNy,&y1,
					XmNwidth,&width,
					XmNheight,&height,
					NULL);
				/*???DB.  This crops the image ! */
				ymaxscreen=getgdesc(GD_YPMAX)-1;
				x2 = X1 + Width;
				y2 = Y1 + Height;
				y2 -= height*0.125;
				x1 = x2 - width ;
				y1 = y2 - 0.75*height;
				y1 = ymaxscreen - y1;
				y1 += height*0.125;
				x2=x1+video_width-1;
				y2=y1-video_height+1;
				/* call animation test loop */
					/*???DB.  Is animation flag needed ? */
				left=(int)x1;
				right=(int)x2;
				top=(int)y2;
				bottom=(int)y1;
#endif
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_video_on_off.  Missing graphics window");
		return_code=0;
	}
#endif /* defined (OLD_GFX_WINDOW) */
	return_code=0;
	LEAVE;

	return (return_code);
} /* set_video_on_off */

int grab_frame(int number_of_frames)
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Grabs a frame from the screen and writes it <number_of_frames> times to the
video recorder.
==============================================================================*/
{
	int return_code;

	ENTER(grab_frame);
	if (video_graphics_window)
	{
		/* updates should already be automatic */
		Graphics_window_update(video_graphics_window);
		if (video_socket>0)
		{
			return_code=write_frame_to_video(number_of_frames,left,right,top,bottom);
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* grab_frame */
