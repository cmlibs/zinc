/*******************************************************************************
FILE : movie_extensions.c

LAST MODIFIED : 10 June 1998

DESCRIPTION :

HISTORY :
==============================================================================*/
#include <dlfcn.h>
#include <Xm/Xm.h>
#include <X11/Xlib.h>
#include <X11/IntrinsicP.h>
#if defined (SGI_MOVIE_FILE)
/*???SAB.  This is defined here because needed in a header file */
	/*???DB.  Which header ?  Must be defined somewhere ? */
#if !defined (_STAMP_T)
#define _STAMP_T
typedef signed long long stamp_t; /* used for USTs and often MSCs */
typedef struct USTMSCpair
{
	stamp_t ust; /* a UST value at which an MSC hit an input or output jack */
	stamp_t msc; /* the MSC which is being described */
} USTMSCpair;
#endif /* !defined (_STAMP_T) */
#if _MIPS_SZLONG == 64
/* SAB  Seems that the 64bit headers are not compiling quite right so I'm
	setting a flag to force it to include a needed declaration */
#define _BSD_TYPES
#endif /* _MIPS_SZLONG == 64 */
#endif /* defined (SGI_MOVIE_FILE) */
#if defined (SGI_MOVIE_FILE)
#include <sys/time.h>
#include <dmedia/moviefile.h>
/*#include <dmedia/movieplay.h>*/
#endif /* defined (SGI_MOVIE_FILE) */
#include "three_d_drawing/ThreeDDraw.h"
#include "three_d_drawing/ThreeDDraP.h"
#include "general/debug.h"
#include "user_interface/user_interface.h"
#define DM_BUFFER_PRIVATE_FUNCTIONS
#include "three_d_drawing/dm_interface.h"
#include "command/parser.h"
#include "three_d_drawing/movie_extensions.h"
#include "time/time.h"
#include "time/time_keeper.h"
#include "user_interface/message.h"

struct X3d_movie_callback_data
{
	X3d_movie_callback callback;
	void *callback_user_data;

	struct X3d_movie_callback_data *next;
};

struct X3d_movie_destroy_callback_data
{
	X3d_movie_destroy_callback callback;
	void *callback_user_data;

	struct X3d_movie_destroy_callback_data *next;
};

struct X3d_movie
{
	enum X3d_movie_create_option create_option;
	int new_movie;
#if defined (SGI_MOVIE_FILE)
	/* for playing movies in scene_viewer */
	MVid movie_id,video_track_id;
#endif /* defined (SGI_MOVIE_FILE) */
	struct Dm_buffer *dmbuffer;
	XtInputId xt_input_id;
	void *image_buffer;
	int image_width;
	int image_height;
	int image_padding;
	/* These time, length and speed values
		are only used for the XtTimeout playback */
	int time;
	double speed;
	XtIntervalId xt_interval_id;

	int timebase;
	struct Time_object *time_object;

	XtAppContext app_context;
	struct X3d_movie_callback_data *callback_list;
	struct X3d_movie_destroy_callback_data *destroy_callback_list;
};

static int X3d_movie_update_callback(struct Time_object *time,
	double current_time, void *movie_void)
/*******************************************************************************
LAST MODIFIED : 1 October 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code, frame_time;
	struct X3d_movie *movie;
	struct X3d_movie_callback_data *callback_data;

	ENTER(X3d_movie_update_callback);
	USE_PARAMETER(time);
#if defined (SGI_MOVIE_FILE)
	if ((movie = (struct X3d_movie *)movie_void) && movie->movie_id)
	{
		frame_time = (int)(current_time * (double)movie->timebase);
		if(movie->dmbuffer)
		{
			Dm_buffer_glx_make_current(movie->dmbuffer);
			X3d_movie_render_to_glx(movie, frame_time, movie->timebase);
		}
		if(movie->image_buffer)
		{
			X3d_movie_render_to_image_buffer(movie, movie->image_buffer,
				movie->image_width, movie->image_height, movie->image_padding,
				frame_time, movie->timebase);
		}
		callback_data = movie->callback_list;
		while(callback_data)
		{
			(callback_data->callback)(movie, frame_time, movie->timebase,
				callback_data->callback_user_data);
			callback_data = callback_data->next;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"X3d_movie_update_callback.  Invalid arguments");
		return_code=0;
	}
#else /* defined (SGI_MOVIE_FILE) */
	display_message(ERROR_MESSAGE,
		"Movie library not available on this machine");
	return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return (return_code);
} /* X3d_movie_update_callback */

struct X3d_movie *CREATE(X3d_movie)(char *filename, 
	enum X3d_movie_create_option create_option)
/*******************************************************************************
LAST MODIFIED : 10 June 1998

DESCRIPTION :
Attempts to create a movie object.
==============================================================================*/
{
	int return_code;
#if defined (SGI_MOVIE_FILE)
	DMparams *params;
#endif /* defined (SGI_MOVIE_FILE) */
	struct X3d_movie *movie;
	void *handleA, *handleB, *handleC;

	ENTER(CREATE(X3d_movie));
	if (filename)
	{
#if defined (SGI_MOVIE_FILE)
		if ((handleA = dlopen("libmoviefile.so", RTLD_LAZY | RTLD_GLOBAL ))
			&& (handleB = dlopen("libmovieplay.so", RTLD_LAZY | RTLD_GLOBAL ))
			&& (handleC = dlopen("libdmedia.so", RTLD_LAZY | RTLD_GLOBAL )))
		{
			/* Actually only need handle if we want to close it again */
			if (ALLOCATE(movie, struct X3d_movie, 1 ))
			{
				movie->callback_list = (struct X3d_movie_callback_data *)NULL;
				movie->destroy_callback_list = 
					(struct X3d_movie_destroy_callback_data *)NULL;
				movie->xt_input_id = (XtInputId)NULL;
				movie->xt_interval_id = (XtIntervalId)NULL;
				movie->time = 0;
				movie->timebase = 15;
				movie->speed = 15.0;
				movie->video_track_id = (MVid)NULL;
				movie->dmbuffer = (struct Dm_buffer *)NULL;
				movie->image_buffer = NULL;
				movie->image_width = 0;
				movie->image_height = 0;
				movie->image_padding = 0;
				movie->create_option = create_option;
				movie->app_context = (XtAppContext)NULL;
				movie->time_object = ACCESS(Time_object)(CREATE(Time_object)(filename));
				Time_object_add_callback(movie->time_object, X3d_movie_update_callback,
					(void *)movie);
				switch ( create_option )
				{
					case X3D_MOVIE_CREATE_FILE_UNCOMPRESSED_SGI_MOVIE3:
					case X3D_MOVIE_CREATE_FILE_APPLE_ANIMATION_QUICKTIME:
					case X3D_MOVIE_CREATE_FILE_CINEPAK_QUICKTIME:
					case X3D_MOVIE_CREATE_FILE_INDEO_QUICKTIME:
					case X3D_MOVIE_CREATE_FILE_UNCOMPRESSED_AVI:
					case X3D_MOVIE_CREATE_FILE_CINEPAK_AVI:
					case X3D_MOVIE_CREATE_FILE_INDEO_AVI:
					case X3D_MOVIE_CREATE_FILE_MVC1_SGI_MOVIE3:
					case X3D_MOVIE_CREATE_FILE_RLE24_SGI_MOVIE3:						
					{
						movie->new_movie = 1;
						return_code = 0;
						if ((DM_SUCCESS==dmParamsCreate(&params)))
						{
							switch(create_option)
							{
								case X3D_MOVIE_CREATE_FILE_UNCOMPRESSED_SGI_MOVIE3:
								case X3D_MOVIE_CREATE_FILE_MVC1_SGI_MOVIE3:
								case X3D_MOVIE_CREATE_FILE_RLE24_SGI_MOVIE3:
								{
									if(DM_SUCCESS==mvSetMovieDefaults(params,MV_FORMAT_SGI_3))
									{
										return_code = 1;
									}
								} break;
								case X3D_MOVIE_CREATE_FILE_APPLE_ANIMATION_QUICKTIME:
								case X3D_MOVIE_CREATE_FILE_CINEPAK_QUICKTIME:
								case X3D_MOVIE_CREATE_FILE_INDEO_QUICKTIME:
								{
									if(DM_SUCCESS==mvSetMovieDefaults(params,MV_FORMAT_QT))
									{
										return_code = 1;
									}
								} break;
								case X3D_MOVIE_CREATE_FILE_UNCOMPRESSED_AVI:
								case X3D_MOVIE_CREATE_FILE_CINEPAK_AVI:
								case X3D_MOVIE_CREATE_FILE_INDEO_AVI:
								{
									if(DM_SUCCESS==mvSetMovieDefaults(params,MV_FORMAT_AVI))
									{
										return_code = 1;
									}
								} break;
							}
						}
						if(return_code)
						{
							if ( DM_SUCCESS==mvCreateFile(filename,params,NULL,
								&(movie->movie_id)))
							{
								mvSetLoopMode(movie->movie_id, MV_LOOP_CONTINUOUSLY);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"CREATE(X3d_movie).  Unable to create movie %s: %s",
									filename,mvGetErrorStr(mvGetErrno()));
								DEALLOCATE(movie);
								movie = (struct X3d_movie *)NULL;
							}
							dmParamsDestroy(params);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"CREATE(X3d_movie).  Unable to set default params");
							DEALLOCATE(movie);
							movie = (struct X3d_movie *)NULL;
						}
					} break;
					case X3D_MOVIE_OPEN_FILE:
					{
						movie->new_movie = 0;
						if ( DM_SUCCESS==mvOpenFile(filename,O_RDONLY,
							&(movie->movie_id)))
						{
							mvSetLoopMode(movie->movie_id, MV_LOOP_CONTINUOUSLY);
							if (DM_SUCCESS==mvFindTrackByMedium(movie->movie_id, DM_IMAGE,
								&(movie->video_track_id)))
							{
								movie->timebase = mvGetImageRate(movie->video_track_id);
								Time_object_set_update_frequency(movie->time_object,
									(double)movie->timebase);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"CREATE(X3d_movie).  Unable to open movie %s: %s",
								filename,mvGetErrorStr(mvGetErrno()));
							DEALLOCATE(movie);
							movie = (struct X3d_movie *)NULL;
						}
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"CREATE(X3d_movie).  Unknown create option",
							filename,mvGetErrorStr(mvGetErrno()));
						DEALLOCATE(movie);
						movie = (struct X3d_movie *)NULL;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(X3d_movie).  Unable to allocate memory for structure");
				movie = (struct X3d_movie *)NULL;
			}
		}
		else
		{
		display_message(ERROR_MESSAGE,
			"Movie library shared object(s) not found on this machine");
		movie = (struct X3d_movie *)NULL;
		}
#else /* defined (SGI_MOVIE_FILE) */
		display_message(ERROR_MESSAGE,
			"Movie library not in this version");
		movie = (struct X3d_movie *)NULL;
#endif /* defined (SGI_MOVIE_FILE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(X3d_movie).  Missing filename");
		movie = (struct X3d_movie *)NULL;
	}
	LEAVE;

	return (movie);
} /* CREATE(X3d_movie) */

int X3d_movie_bind_to_dmbuffer(struct X3d_movie *movie, 
	struct Dm_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 7 September 1998

DESCRIPTION :
Sets the dmbuffer that is associated with this movie instance.
==============================================================================*/
{
	int return_code;

	ENTER(X3d_movie_bind_to_dmbuffer);
#if defined (SGI_MOVIE_FILE)
	if (movie && buffer)
	{
		return_code = 1;
		movie->dmbuffer = buffer;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"X3d_movie_bind_to_dmbuffer.  Missing movie or widget");
		return_code=0;
	}
#else /* defined (SGI_MOVIE_FILE) */
	display_message(ERROR_MESSAGE,
		"Movie library not available on this machine");
	return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return (return_code);
} /* X3d_movie_bind_to_dmbuffer */

int X3d_movie_unbind_from_dmbuffer(struct X3d_movie *movie)
/*******************************************************************************
LAST MODIFIED : 17 September 1998

DESCRIPTION :
Disassociates the dmbuffer that is associated with this movie instance.
==============================================================================*/
{
	int return_code;

	ENTER(X3d_movie_unbind_from_dmbuffer);
#if defined (SGI_MOVIE_FILE)
	if (movie)
	{
		X3d_movie_stop(movie);
		movie->dmbuffer = (struct Dm_buffer *)NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"X3d_movie_unbind_from_dmbuffer.  Missing movie");
		return_code=0;
	}
#else /* defined (SGI_MOVIE_FILE) */
	display_message(ERROR_MESSAGE,
		"Movie library not available on this machine");
	return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return (return_code);
} /* X3d_movie_unbind_from_dmbuffer */

int X3d_movie_bind_to_image_buffer(struct X3d_movie *movie, void *image_data, 
	int image_width, int image_height, int image_padding)
/*******************************************************************************
LAST MODIFIED : 17 September 1998

DESCRIPTION :
Sets an image buffer that is associated with this movie instance.
==============================================================================*/
{
	int return_code;

	ENTER(X3d_movie_bind_to_image_buffer);
#if defined (SGI_MOVIE_FILE)
	if (movie && image_data)
	{
		return_code = 1;
		movie->image_buffer = image_data;
		movie->image_width = image_width;
		movie->image_height = image_height;
		movie->image_padding = image_padding;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"X3d_movie_bind_to_image_buffer.  Missing movie or widget");
		return_code=0;
	}
#else /* defined (SGI_MOVIE_FILE) */
	display_message(ERROR_MESSAGE,
		"Movie library not available on this machine");
	return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return (return_code);
} /* X3d_movie_bind_to_image_buffer */

int X3d_movie_unbind_from_image_buffer(struct X3d_movie *movie)
/*******************************************************************************
LAST MODIFIED : 17 September 1998

DESCRIPTION :
Disassociates the image buffer that is associated with this movie instance.
==============================================================================*/
{
	int return_code;

	ENTER(X3d_movie_unbind_from_image_buffer);
#if defined (SGI_MOVIE_FILE)
	if (movie)
	{
		X3d_movie_stop(movie);
		movie->image_buffer = NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"X3d_movie_unbind_from_image_buffer.  Missing movie");
		return_code=0;
	}
#else /* defined (SGI_MOVIE_FILE) */
	display_message(ERROR_MESSAGE,
		"Movie library not available on this machine");
	return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return (return_code);
} /* X3d_movie_unbind_from_image_buffer */

#if defined (OLD_CODE)
static void X3d_movie_event_handler(XtPointer movie_void,
	int *file_descriptor, XtInputId *id)
/*******************************************************************************
LAST MODIFIED : 8 September 1998

DESCRIPTION :
Responds to XEvents generated by the movie event queue.
==============================================================================*/
{
	int return_code;
#if defined (SGI_MOVIE_FILE)
	int frame, timebase;
	MVevent event;
	MVframeevent frame_event;
	struct X3d_movie *movie;
#endif /* defined (SGI_MOVIE_FILE) */
	struct X3d_movie_callback_data *callback_data;

	ENTER(X3d_movie_event_handler);
#if defined (SGI_MOVIE_FILE)
	if (movie = (struct X3d_movie *)movie_void)
	{
		if(movie->xt_input_id)
		{
			/* This is an mvEvent based callback */
			/* Only MV_EVENT_FRAME events are selected at the moment
				so we can just go for the last event */
			while(mvPendingMovieEvents(movie->movie_id))
			{
				mvNextMovieEvent(movie->movie_id, &event);
			}

			if ((event.type==MV_EVENT_FRAME) && movie->callback_list && movie->video_track_id)
			{
				frame_event = event.mvframe;
				frame = frame_event.frame;
				timebase = mvGetImageRate(movie->video_track_id);
				callback_data = movie->callback_list;
				while(callback_data)
				{
					(callback_data->callback)(movie, frame, timebase,
						callback_data->callback_user_data);
					callback_data = callback_data->next;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"X3d_movie_event_handler.  Incorrect event, could not get video track or no callbacks defined");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"X3d_movie_event_handler.  Unknown source for event callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"X3d_movie_event_handler.  Missing movie");
		return_code=0;
	}
#else /* defined (SGI_MOVIE_FILE) */
	display_message(ERROR_MESSAGE,
		"Movie library not available on this machine");
	return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return;
} /* X3d_movie_event_handler */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
static void X3d_movie_xttimer_event_handler(XtPointer movie_void,
	XtIntervalId *id)
/*******************************************************************************
LAST MODIFIED : 18 September 1998

DESCRIPTION :
Responds to XXttimer_Events generated by the movie xttimer_event queue.
==============================================================================*/
{
	int return_code;
	struct X3d_movie *movie;
	struct X3d_movie_callback_data *callback_data;

	ENTER(X3d_movie_xttimer_event_handler);
#if defined (SGI_MOVIE_FILE)
	if ((movie = (struct X3d_movie *)movie_void) && movie->app_context)
	{
		if(movie->xt_interval_id)
		{
			movie->time++;
			callback_data = movie->callback_list;
			while(callback_data)
			{
				(callback_data->callback)(movie, movie->time, movie->timebase,
					callback_data->callback_user_data);
				callback_data = callback_data->next;
			}
			return_code = 1;
			if(movie->dmbuffer)
			{
				Dm_buffer_glx_make_current(movie->dmbuffer);
				glClearColor(0.0,0.0,0.0,1.0);
				glClearDepth(1.0);
				glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
				if(!X3d_movie_render_to_glx(movie, movie->time, movie->timebase))
				{
					movie->time = 0;
					if(!X3d_movie_render_to_glx(movie, movie->time, movie->timebase))
					{
						display_message(ERROR_MESSAGE,
							"X3d_movie_event_handler.  Unable to render movie frame");
						return_code=0;							
					}
				}
				X3dThreeDDrawingRemakeCurrent();
			}
			if(movie->image_buffer)
			{
				if(!X3d_movie_render_to_image_buffer(movie, movie->image_buffer,
					movie->image_width, movie->image_height, movie->image_padding,
					movie->time, movie->timebase))
				{
					movie->time = 0;
					if(!X3d_movie_render_to_image_buffer(movie, movie->image_buffer,
						movie->image_width, movie->image_height, movie->image_padding,
						movie->time, movie->timebase))
					{
						display_message(ERROR_MESSAGE,
							"X3d_movie_xttimer_event_handler.  Unable to render movie frame");
						return_code=0;							
					}
				}
			}
			if(return_code)
			{
				/* This is an xt timer event callback */
				movie->xt_interval_id = XtAppAddTimeOut(movie->app_context, 
					(1000.0 / movie->speed),
					X3d_movie_xttimer_event_handler, (void *)movie);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"X3d_movie_xttimer_event_handler.  Unknown source for callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"X3d_movie_xttimer_event_handler.  Missing movie or app_context");
		return_code=0;
	}
#else /* defined (SGI_MOVIE_FILE) */
	display_message(ERROR_MESSAGE,
		"Movie library not available on this machine");
	return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return;
} /* X3d_movie_xttimer_event_handler */
#endif /* defined (OLD_CODE) */

int X3d_movie_add_callback(struct X3d_movie *movie, XtAppContext app_context,
	X3d_movie_callback callback, void *user_data)
/*******************************************************************************
LAST MODIFIED : 15 September 1998

DESCRIPTION :
Adds a callback routine which is triggered every time a new valid frame
is ready.  This enables external routines to monitor the play progress of a 
movie.
==============================================================================*/
{
	int return_code;
	struct X3d_movie_callback_data *callback_data, *previous;

	ENTER(X3d_movie_add_callback);
#if defined (SGI_MOVIE_FILE)
	if (movie && callback)
	{
		movie->app_context = app_context;
		if(ALLOCATE(callback_data, struct X3d_movie_callback_data, 1))
		{
			callback_data->callback = callback;
			callback_data->callback_user_data = user_data;
			callback_data->next = (struct X3d_movie_callback_data *)NULL;
			if(movie->callback_list)
			{
				previous = movie->callback_list;
				while(previous->next)
				{
					previous = previous->next;
				}
				previous->next = callback_data;
			}
			else
			{
				movie->callback_list = callback_data;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"X3d_movie_add_callback.  Unable to allocate callback data structure");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"X3d_movie_add_callback.  Missing movie or callback");
		return_code=0;
	}
#else /* defined (SGI_MOVIE_FILE) */
	display_message(ERROR_MESSAGE,
		"Movie library not available on this machine");
	return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return (return_code);
} /* X3d_movie_add_callback */

int X3d_movie_remove_callback(struct X3d_movie *movie,
	X3d_movie_callback callback, void *user_data)
/*******************************************************************************
LAST MODIFIED : 15 September 1998

DESCRIPTION :
Removes a callback which was added previously
==============================================================================*/
{
	int return_code;
	struct X3d_movie_callback_data *callback_data, *previous;

	ENTER(X3d_movie_remove_callback);
#if defined (SGI_MOVIE_FILE)
	if (movie && callback && movie->callback_list)
	{
		callback_data = movie->callback_list;
		if((callback_data->callback == callback)
			&& (callback_data->callback_user_data == user_data))
		{
			movie->callback_list = callback_data->next;
			DEALLOCATE(callback_data);
			return_code = 1;
		}
		else
		{
			return_code = 0;
			while(!return_code && callback_data->next)
			{
				previous = callback_data;
				callback_data = callback_data->next;
				if((callback_data->callback == callback)
					&& (callback_data->callback_user_data == user_data))
				{
					previous->next = callback_data->next;
					DEALLOCATE(callback_data);
					return_code = 1;		
				}
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"X3d_movie_remove_callback.  Unable to find callback and user_data specified");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"X3d_movie_remove_callback.  Missing movie, callback or callback list");
		return_code=0;
	}
#else /* defined (SGI_MOVIE_FILE) */
	display_message(ERROR_MESSAGE,
		"Movie library not available on this machine");
	return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return (return_code);
} /* X3d_movie_remove_callback */

int X3d_movie_add_destroy_callback(struct X3d_movie *movie,
	X3d_movie_destroy_callback callback, void *user_data)
/*******************************************************************************
LAST MODIFIED : 18 September 1998

DESCRIPTION :
Adds a callback routine which is notified when a movie is being destroyed.
==============================================================================*/
{
	int return_code;
	struct X3d_movie_destroy_callback_data *callback_data, *previous;

	ENTER(X3d_movie_add_destroy_callback);
#if defined (SGI_MOVIE_FILE)
	if (movie && callback)
	{
		if(ALLOCATE(callback_data, struct X3d_movie_destroy_callback_data, 1))
		{
			callback_data->callback = callback;
			callback_data->callback_user_data = user_data;
			callback_data->next = (struct X3d_movie_destroy_callback_data *)NULL;
			if(movie->destroy_callback_list)
			{
				previous = movie->destroy_callback_list;
				while(previous->next)
				{
					previous = previous->next;
				}
				previous->next = callback_data;
			}
			else
			{
				movie->destroy_callback_list = callback_data;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"X3d_movie_add_callback.  Unable to allocate callback data structure");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"X3d_movie_add_destroy_callback.  Missing movie or callback");
		return_code=0;
	}
#else /* defined (SGI_MOVIE_FILE) */
	display_message(ERROR_MESSAGE,
		"Movie library not available on this machine");
	return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return (return_code);
} /* X3d_movie_add_destroy_callback */

int X3d_movie_remove_destroy_callback(struct X3d_movie *movie,
	X3d_movie_destroy_callback callback, void *user_data)
/*******************************************************************************
LAST MODIFIED : 18 September 1998

DESCRIPTION :
Removes a destroy callback which was added previously
==============================================================================*/
{
	int return_code;
	struct X3d_movie_destroy_callback_data *callback_data, *previous;

	ENTER(X3d_movie_remove_destroy_callback);
#if defined (SGI_MOVIE_FILE)
	if (movie && callback && movie->destroy_callback_list)
	{
		callback_data = movie->destroy_callback_list;
		if((callback_data->callback == callback)
			&& (callback_data->callback_user_data == user_data))
		{
			movie->destroy_callback_list = callback_data->next;
			DEALLOCATE(callback_data);
			return_code = 1;
		}
		else
		{
			return_code = 0;
			while(!return_code && callback_data->next)
			{
				previous = callback_data;
				callback_data = callback_data->next;
				if((callback_data->callback == callback)
					&& (callback_data->callback_user_data == user_data))
				{
					previous->next = callback_data->next;
					DEALLOCATE(callback_data);
					return_code = 1;		
				}
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"X3d_movie_remove_destroy_callback.  Unable to find callback and user_data specified");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"X3d_movie_remove_destroy_callback.  Missing movie, callback or callback list");
		return_code=0;
	}
#else /* defined (SGI_MOVIE_FILE) */
	display_message(ERROR_MESSAGE,
		"Movie library not available on this machine");
	return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return (return_code);
} /* X3d_movie_remove_destroy_callback */

int X3d_movie_render_to_image_buffer(struct X3d_movie *movie, 
	void *buffer, int width, int height, int padding, int time, int timescale)
/*******************************************************************************
LAST MODIFIED : 8 September 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;
#if defined (SGI_MOVIE_FILE)
	DMparams *image_params
#endif /* defined (SGI_MOVIE_FILE) */

	ENTER(X3d_movie_render_to_image_buffer);
#if defined (SGI_MOVIE_FILE)
	if (movie && buffer && (width > 0) && (height > 0))
	{
		if(DM_SUCCESS==dmParamsCreate(&image_params))
		{
			if ((DM_SUCCESS==dmSetImageDefaults(image_params,width,height,
				DM_IMAGE_PACKING_XBGR)) &&
				(DM_SUCCESS==dmParamsSetInt(image_params, "MV_IMAGE_STRIDE",
				padding)))
			{
				if (DM_SUCCESS==mvRenderMovieToImageBuffer(movie->movie_id,
					time, timescale, buffer, image_params))
				{
					return_code=1;
				}
				else
				{
					if(MV_NOTHING_RENDERED!=mvGetErrno())
						/* No error is reported if the time is out of range so that the
							X3d_event_handler can restart from the movie start */
					{
						display_message(ERROR_MESSAGE,
							"X3d_movie_render_to_image_buffer.  Unable to render movie");
					}
					return_code=0;
				}				
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"X3d_movie_render_to_image_buffer.  Unable to set image parameters");
				return_code=0;
			}
			dmParamsDestroy(image_params);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"X3d_movie_render_to_image_buffer.  Unable to create image parameters");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"X3d_movie_render_to_image_buffer.  Invalid arguments");
		return_code=0;
	}
#else /* defined (SGI_MOVIE_FILE) */
	display_message(ERROR_MESSAGE,
		"Movie library not available on this machine");
	return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return (return_code);
} /* X3d_movie_render_to_image_buffer */

int X3d_movie_render_to_glx(struct X3d_movie *movie, int time, int timescale)
/*******************************************************************************
LAST MODIFIED : 10 September 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(X3d_movie_render_to_glx);
#if defined (SGI_MOVIE_FILE)
	if (movie)
	{
		if (DM_SUCCESS==mvRenderMovieToOpenGL(movie->movie_id,
			time, timescale))
		{
			return_code=1;
		}
		else
		{
			if(MV_NOTHING_RENDERED!=mvGetErrno())
				/* No error is reported if the time is out of range so that the
					X3d_event_handler can restart from the movie start */
			{
				display_message(ERROR_MESSAGE,
					"X3d_movie_render_to_glx.  Unable to render movie");
			}
			return_code=0;
		}				
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"X3d_movie_render_to_glx.  Invalid arguments");
		return_code=0;
	}
#else /* defined (SGI_MOVIE_FILE) */
	display_message(ERROR_MESSAGE,
		"Movie library not available on this machine");
	return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return (return_code);
} /* X3d_movie_render_to_glx */

int X3d_movie_add_frame(struct X3d_movie *movie, int width, int height,
	char *frame_data)
/*******************************************************************************
LAST MODIFIED : 22 July 1999

DESCRIPTION :
Adds the supplied <frame_data> to the video_track in the movie.  If no video_track 
exists a new one is created.  The frame data must be XBGR bytes.
==============================================================================*/
{
	int return_code;
#if defined (SGI_MOVIE_FILE)
	DMparams *image_params;
	int insertFrame;
#endif /* defined (SGI_MOVIE_FILE) */

	ENTER(X3d_movie_add_frame);
#if defined (SGI_MOVIE_FILE)
	if (movie && movie->movie_id)
	{
		if (movie->new_movie)
		{
			return_code = 1;
			if (!movie->video_track_id)
			{
				/* Create a video track */
				if ((DM_SUCCESS==dmParamsCreate(&image_params)) &&
					(DM_SUCCESS==dmSetImageDefaults(image_params,width,height,
					DM_PACKING_RGBX)))
				{
					switch(movie->create_option)
					{
						case X3D_MOVIE_CREATE_FILE_MVC1_SGI_MOVIE3:
						{
							dmParamsSetString  (image_params, DM_IMAGE_COMPRESSION,
								DM_IMAGE_MVC1 );
						} break;
						case X3D_MOVIE_CREATE_FILE_RLE24_SGI_MOVIE3:
						{
							dmParamsSetString  (image_params, DM_IMAGE_COMPRESSION,
								DM_IMAGE_RLE24 );
						} break;
						case X3D_MOVIE_CREATE_FILE_APPLE_ANIMATION_QUICKTIME:
						{
							dmParamsSetString  (image_params, DM_IMAGE_COMPRESSION,
								DM_IMAGE_QT_ANIM );
						} break;
						case X3D_MOVIE_CREATE_FILE_CINEPAK_QUICKTIME:
						case X3D_MOVIE_CREATE_FILE_CINEPAK_AVI:
						{
							dmParamsSetString  (image_params, DM_IMAGE_COMPRESSION,
								DM_IMAGE_QT_CVID /* COMPACT VIDEO is CINEPAK */ );
						} break;
						case X3D_MOVIE_CREATE_FILE_INDEO_QUICKTIME:
						case X3D_MOVIE_CREATE_FILE_INDEO_AVI:
						{
							dmParamsSetString  (image_params, DM_IMAGE_COMPRESSION,
								DM_IMAGE_INDEO );
						} break;
					}
					if (DM_SUCCESS!=mvAddTrack(movie->movie_id,DM_IMAGE,
						image_params,NULL,&(movie->video_track_id)))
					{
						display_message(ERROR_MESSAGE,
							"X3d_movie_bind_to_widget.  Unable to add track");
						movie->video_track_id = (MVid)NULL;
						return_code = 0;
					}
					dmParamsDestroy(image_params);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"X3d_movie_bind_to_widget.  Unable to set track parameters");
					movie->video_track_id = (MVid)NULL;
					return_code = 0;
				}
			}
			if (return_code)
			{
				insertFrame=(int)mvGetTrackLength(movie->video_track_id);
				if (DM_SUCCESS!=mvInsertFrames( movie->video_track_id,insertFrame,1,
					4*width*height,frame_data))
				{
					display_message(ERROR_MESSAGE,
						"X3d_movie_add_frame.  Could not add frame");
					return_code=0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"X3d_movie_add_frame.  Must be a new movie");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"X3d_movie_add_frame.  Missing movie or movie_id");
		return_code=0;
	}
#else /* defined (SGI_MOVIE_FILE) */
	display_message(ERROR_MESSAGE,
		"Movie library not available on this machine");
	return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return (return_code);
} /* X3d_movie_add_frame */

int X3d_movie_play(struct X3d_movie *movie)
/*******************************************************************************
LAST MODIFIED : 22 July 1999

DESCRIPTION :
Binds the OpenGL window and starts it playing.
==============================================================================*/
{
	int return_code;
	struct Time_keeper *time_keeper;

	ENTER(X3d_movie_play);
#if defined (SGI_MOVIE_FILE)
	if (movie&&(movie->movie_id))
	{
		if(movie->video_track_id)
		{
			movie->timebase = (int)mvGetImageRate(movie->video_track_id);
		}
		else
		{
			movie->timebase = 15;
		}
		if(movie->time_object &&
			(time_keeper = Time_object_get_time_keeper(movie->time_object)))
		{
			Time_keeper_play(time_keeper, TIME_KEEPER_PLAY_FORWARD);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"X3d_movie_play.  No time object or associated time_keeper");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
	"X3d_movie_play.  Missing movie, movie_id or nothing is bound to movie");
		return_code=0;
	}
#else /* defined (SGI_MOVIE_FILE) */
	display_message(ERROR_MESSAGE,
		"Movie library not available on this machine");
	return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return (return_code);
} /* X3d_movie_play */

int X3d_movie_playing(struct X3d_movie *movie)
/*******************************************************************************
LAST MODIFIED : 8 September 1998

DESCRIPTION :
Tells you whether a movie is currently playing or not.
==============================================================================*/
{
	int return_code;
	struct Time_keeper *time_keeper;

	ENTER(X3d_movie_playing);
#if defined (SGI_MOVIE_FILE)
	if (movie&&(movie->movie_id))
	{
		if(movie->time_object &&
			(time_keeper = Time_object_get_time_keeper(movie->time_object)))
		{
			return_code = Time_keeper_is_playing(time_keeper);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"X3d_movie_stop.  No time object or associated time_keeper");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"X3d_movie_playing.  Missing movie or movie_id");
		return_code=0;
	}
#else /* defined (SGI_MOVIE_FILE) */
	display_message(ERROR_MESSAGE,
		"Movie library not available on this machine");
	return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return (return_code);
} /* X3d_movie_playing */

int X3d_movie_stop(struct X3d_movie *movie)
/*******************************************************************************
LAST MODIFIED : 10 June 1998

DESCRIPTION :
Stops playing and unbinds the OpenGL window
==============================================================================*/
{
	int return_code;
	struct Time_keeper *time_keeper;

	ENTER(X3d_movie_stop);
#if defined (SGI_MOVIE_FILE)
	if (movie&&(movie->movie_id))
	{
		if(movie->time_object &&
			(time_keeper = Time_object_get_time_keeper(movie->time_object)))
		{
			Time_keeper_stop(time_keeper);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"X3d_movie_stop.  No time object or associated time_keeper");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"X3d_movie_stop.  Missing movie or movie_id");
		return_code=0;
	}
#else /* defined (SGI_MOVIE_FILE) */
	display_message(ERROR_MESSAGE,
		"Movie library not available on this machine");
	return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return (return_code);
} /* X3d_movie_stop */

double X3d_movie_get_actual_framerate(
	struct X3d_movie *movie)
/*******************************************************************************
LAST MODIFIED : 10 June 1998

DESCRIPTION :
Gets the frame rate for the last second of a movie playing
==============================================================================*/
{
	double frame_rate;

	ENTER(X3d_movie_get_actual_framerate);
#if defined (SGI_MOVIE_FILE)
	if (movie&&(movie->movie_id))
	{
		display_message(ERROR_MESSAGE,
		"X3d_movie_get_actual_framerate.  broken");
		frame_rate=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
		"X3d_movie_get_actual_framerate.  Missing movie or movie id");
		frame_rate=0;
	}
#else /* defined (SGI_MOVIE_FILE) */
	display_message(ERROR_MESSAGE,
		"Movie library not available on this machine");
	frame_rate=0;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return (frame_rate);
} /* X3d_movie_get_actual_framerate */

int X3d_movie_get_bounding_rectangle(
	struct X3d_movie *movie, int *left, int *bottom, int *width, int *height)
/*******************************************************************************
LAST MODIFIED : 9 September 1998

DESCRIPTION :
Gets a rectangle which will enclose the rendered movie.
==============================================================================*/
{
	int return_code;
#if defined (SGI_MOVIE_FILE)
	MVrect rect;
#endif /* defined (SGI_MOVIE_FILE) */
 
	ENTER(X3d_movie_get_bounding_rectangle);
#if defined (SGI_MOVIE_FILE)
	if (movie&&(movie->movie_id))
	{
		if ((movie->video_track_id) &&
			(DM_SUCCESS==mvGetMovieBoundingRect(movie->movie_id, &rect)))
		{
			if(left)
			{
				*left = rect.left;
			}
			if(bottom)
			{
				*bottom = rect.bottom;
			}
			if(width)
			{
				*width = rect.right - rect.left;
			}
			if(height)
			{
				*height = rect.top - rect.bottom;
			}
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
		"X3d_movie_get_bounding_rectangle.  Missing movie or movie id");
		return_code=0;
	}
#else /* defined (SGI_MOVIE_FILE) */
	display_message(ERROR_MESSAGE,
		"Movie library not available on this machine");
	return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return (return_code);
} /* X3d_movie_get_bounding_rectangle */

int X3d_movie_set_play_loop(struct X3d_movie *movie,
	int loop)
/*******************************************************************************
LAST MODIFIED : 10 June 1998

DESCRIPTION :
Sets a movie so that it loops of not
==============================================================================*/
{
	int return_code;
#if defined (SGI_MOVIE_FILE)
	MVtime duration;
	MVtimescale timescale;
#endif /* defined (SGI_MOVIE_FILE) */
	struct Time_keeper *time_keeper;

	ENTER(X3d_movie_set_play_loop);
#if defined (SGI_MOVIE_FILE)
	if (movie&&(movie->movie_id))
	{
		if(movie->time_object &&
			(time_keeper = Time_object_get_time_keeper(movie->time_object)))
		{
			if(loop)
			{
				timescale = 30;
				duration = mvGetMovieDuration(movie->movie_id, timescale);
				if(duration > -1)
				{
					Time_keeper_set_minimum(time_keeper, 0.0);
					Time_keeper_set_maximum(time_keeper, (double)duration / (double)timescale);
					Time_keeper_set_play_loop(time_keeper);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"X3d_movie_set_play_loop.  Unable to get movie duration. (MPEG length is unknown till played)");
					return_code=0;
				}
			}
			else
			{
				Time_keeper_set_play_once(time_keeper);
			}
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"X3d_movie_stop.  No time object or associated time_keeper");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"X3d_movie_set_play_loop.  Missing movie or movie id");
		return_code=0;
	}
#else /* defined (SGI_MOVIE_FILE) */
	display_message(ERROR_MESSAGE,
		"Movie library not available on this machine");
	return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return (return_code);
} /* X3d_movie_set_play_loop */

int X3d_movie_set_play_every_frame(struct X3d_movie *movie,
	int play_every_frame)
/*******************************************************************************
LAST MODIFIED : 10 June 1998

DESCRIPTION :
Sets a movie so that it plays every frame or maintains the frame rate by
dropping frames
==============================================================================*/
{
	int return_code;
	struct Time_keeper *time_keeper;

	ENTER(X3d_movie_stop);
#if defined (SGI_MOVIE_FILE)
	if (movie&&(movie->movie_id))
	{
		if(movie->time_object &&
			(time_keeper = Time_object_get_time_keeper(movie->time_object)))
		{
			if(play_every_frame)
			{
				Time_keeper_set_play_every_frame(time_keeper);
			}
			else
			{
				Time_keeper_set_play_skip_frames(time_keeper);
			}
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"X3d_movie_stop.  No time object or associated time_keeper");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
		"X3d_movie_set_play_every_frame.  Missing movie or movie");
		return_code=0;
	}
#else /* defined (SGI_MOVIE_FILE) */
	display_message(ERROR_MESSAGE,
		"Movie library not available on this machine");
	return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return (return_code);
} /* X3d_movie_set_play_every_frame */

int X3d_movie_set_play_speed(struct X3d_movie *movie,
	double play_speed)
/*******************************************************************************
LAST MODIFIED : 10 June 1998

DESCRIPTION :
Sets a movie so that it plays at a speed <play_speed> frames per second.
==============================================================================*/
{
	int return_code;
	struct Time_keeper *time_keeper;

	ENTER(X3d_movie_set_play_speed);
#if defined (SGI_MOVIE_FILE)
	if (movie&&(movie->movie_id))
	{
		if(movie->time_object &&
			(time_keeper = Time_object_get_time_keeper(movie->time_object)))
		{
			Time_keeper_set_speed(time_keeper, play_speed);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"X3d_movieset_play_speed.  No time object or associated time_keeper");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"X3d_movie_set_play_speed.  Missing movie or movie");
		return_code=0;
	}
#else /* defined (SGI_MOVIE_FILE) */
	display_message(ERROR_MESSAGE,
		"Movie library not available on this machine");
	return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return (return_code);
} /* X3d_movie_set_play_speed */

struct Time_object *X3d_movie_get_time_object(struct X3d_movie *movie)
/*******************************************************************************
LAST MODIFIED : 1 October 1998

DESCRIPTION :
==============================================================================*/
{
	struct Time_object *return_code;

	ENTER(X3d_movie_get_time_object);
#if defined (SGI_MOVIE_FILE)
	if (movie)
	{
		return_code = movie->time_object;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"X3d_movie_get_time_object.  Missing movie or movie");
		return_code=(struct Time_object *)NULL;
	}
#else /* defined (SGI_MOVIE_FILE) */
	display_message(ERROR_MESSAGE,
		"Movie library not available on this machine");
	return_code=(struct Time_object *)NULL;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return (return_code);
} /* X3d_movie_get_time_object */

int DESTROY(X3d_movie)(struct X3d_movie **movie)
/*******************************************************************************
LAST MODIFIED : 10 June 1998

DESCRIPTION :
Closes a movie instance
x==============================================================================*/
{
	int return_code;
	struct X3d_movie_callback_data *callback_data, *next;
	struct X3d_movie_destroy_callback_data *destroy_callback_data, *destroy_next;

	ENTER(DESTROY(X3d_movie));
#if defined (SGI_MOVIE_FILE)
	if (movie && *movie)
	{
		X3d_movie_stop(*movie);
				
		if((*movie)->time_object)
		{
			DEACCESS(Time_object)(&((*movie)->time_object));
		}

		/* Remove input */
		if((*movie)->xt_input_id)
		{
			XtRemoveInput((*movie)->xt_input_id);
		}

		destroy_callback_data = (*movie)->destroy_callback_list;
		while(destroy_callback_data)
		{
			(destroy_callback_data->callback)(*movie, 
				destroy_callback_data->callback_user_data);
			destroy_next = destroy_callback_data->next;
			DEALLOCATE(destroy_callback_data);
			destroy_callback_data = destroy_next;
		}

		callback_data = (*movie)->callback_list;
		while(callback_data)
		{
			next = callback_data->next;
			DEALLOCATE(callback_data);
			callback_data = next;
		}

		return_code=1;
		if( (*movie)->movie_id )
		{
			if (DM_SUCCESS!=mvClose((*movie)->movie_id))
			{
				display_message(ERROR_MESSAGE,
					"DESTROY(X3d_movie).  Unable to close movie");
				return_code=0;
			}
		}
		DEALLOCATE(*movie);
		*movie = (struct X3d_movie *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(X3d_movie).  Missing movie");
		return_code=0;
	}
#else /* defined (SGI_MOVIE_FILE) */
	display_message(ERROR_MESSAGE,
		"DESTROY(X3d_movie). Movie library not available on this machine");
	return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return (return_code);
} /* DESTROY(X3d_movie) */


