/*******************************************************************************
FILE : movie_extensions.h

LAST MODIFIED : 25 July 1998

DESCRIPTION :

HISTORY :
==============================================================================*/
#if !defined (MOVIE_EXTENSIONS_H)
#define MOVIE_EXTENSIONS_H

#include "command/parser.h"
#include "general/object.h"

struct Dm_buffer; /* Instead I could #include dm_interface.h but that is yucky */
struct X3d_movie;

enum X3d_movie_create_option
{
	X3D_MOVIE_CREATE_FILE_UNCOMPRESSED_SGI_MOVIE3,
	X3D_MOVIE_CREATE_FILE_APPLE_ANIMATION_QUICKTIME,
	X3D_MOVIE_CREATE_FILE_CINEPAK_QUICKTIME,
	X3D_MOVIE_CREATE_FILE_INDEO_QUICKTIME,
	X3D_MOVIE_CREATE_FILE_UNCOMPRESSED_AVI,
	X3D_MOVIE_CREATE_FILE_CINEPAK_AVI,
	X3D_MOVIE_CREATE_FILE_INDEO_AVI,
	X3D_MOVIE_CREATE_FILE_MVC1_SGI_MOVIE3,
	X3D_MOVIE_CREATE_FILE_RLE24_SGI_MOVIE3,
	X3D_MOVIE_CREATE_FILE_RLE24_SGI_MOVIE3_RGBA,
	X3D_MOVIE_OPEN_FILE
	/* Other future options could be create in memory */
};

typedef int (*X3d_movie_callback)(struct X3d_movie *movie,
	int time, int time_scale, void *user_data);

typedef int (*X3d_movie_destroy_callback)(struct X3d_movie *movie, void *user_data);

struct X3d_movie *CREATE(X3d_movie)(char *filename, 
	enum X3d_movie_create_option create_option);
/*******************************************************************************
LAST MODIFIED : 10 June 1998

DESCRIPTION :
Attempts to create a movie object.

POSSIBLE DEVELOPMENTS :
Different file and compression formats.
==============================================================================*/

int X3d_movie_bind_to_dmbuffer(struct X3d_movie *movie, 
	struct Dm_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 7 September 1998

DESCRIPTION :
Sets the dmbuffer that is associated with this movie instance.
==============================================================================*/

int X3d_movie_unbind_from_dmbuffer(struct X3d_movie *movie);
/*******************************************************************************
LAST MODIFIED : 17 September 1998

DESCRIPTION :
Disassociates the dmbuffer that is associated with this movie instance.
==============================================================================*/

int X3d_movie_bind_to_image_buffer(struct X3d_movie *movie, void *image_data, 
	int image_width, int image_height, int image_padding);
/*******************************************************************************
LAST MODIFIED : 17 September 1998

DESCRIPTION :
Sets an image buffer that is associated with this movie instance.
==============================================================================*/

int X3d_movie_unbind_from_image_buffer(struct X3d_movie *movie);
/*******************************************************************************
LAST MODIFIED : 17 September 1998

DESCRIPTION :
Disassociates the image buffer that is associated with this movie instance.
==============================================================================*/

int X3d_movie_add_callback(struct X3d_movie *movie, XtAppContext app_context,
	X3d_movie_callback callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 15 September 1998

DESCRIPTION :
Adds a callback routine which is triggered every time a new valid frame
is ready.  This enables external routines to monitor the play progress of a 
movie.
==============================================================================*/

int X3d_movie_remove_callback(struct X3d_movie *movie,
	X3d_movie_callback callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 15 September 1998

DESCRIPTION :
Removes a callback which was added previously
==============================================================================*/

int X3d_movie_add_destroy_callback(struct X3d_movie *movie,
	X3d_movie_destroy_callback callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 18 September 1998

DESCRIPTION :
Adds a callback routine which is notified when a movie is being destroyed.
==============================================================================*/

int X3d_movie_remove_destroy_callback(struct X3d_movie *movie,
	X3d_movie_destroy_callback callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 18 September 1998

DESCRIPTION :
Removes a destroy callback which was added previously
==============================================================================*/

int X3d_movie_render_to_image_buffer(struct X3d_movie *movie, 
	void *buffer, int width, int height, int padding, int time, int time_scale);
/*******************************************************************************
LAST MODIFIED : 8 September 1998

DESCRIPTION :
==============================================================================*/

int X3d_movie_render_to_glx(struct X3d_movie *movie, int time, int timescale);
/*******************************************************************************
LAST MODIFIED : 10 September 1998

DESCRIPTION :
==============================================================================*/

int X3d_movie_add_frame(struct X3d_movie *movie, int width, int height,
	char *frame_data);
/*******************************************************************************
LAST MODIFIED : 22 July 1999

DESCRIPTION :
Adds the supplied <frame_data> to the video_track in the movie.  If no video_track 
exists a new one is created.  The frame data must be XBGR bytes.
==============================================================================*/

int X3d_movie_play(struct X3d_movie *movie);
/*******************************************************************************
LAST MODIFIED : 22 July 1999

DESCRIPTION :
Binds the OpenGL window and starts it playing.
==============================================================================*/

int X3d_movie_playing(struct X3d_movie *movie);
/*******************************************************************************
LAST MODIFIED : 8 September 1998

DESCRIPTION :
Tells you whether a movie is currently playing or not.
==============================================================================*/

int X3d_movie_stop(struct X3d_movie *movie);
/*******************************************************************************
LAST MODIFIED : 10 June 1998

DESCRIPTION :
Stops playing
==============================================================================*/

int X3d_movie_set_play_loop(struct X3d_movie *movie,
	int loop);
/*******************************************************************************
LAST MODIFIED : 10 June 1998

DESCRIPTION :
Sets a movie so that it loops or not
==============================================================================*/

int X3d_movie_set_play_every_frame(struct X3d_movie *movie,
	int play_every_frame);
/*******************************************************************************
LAST MODIFIED : 10 June 1998

DESCRIPTION :
Sets a movie so that it plays every frame or maintains the frame rate by
dropping frames
==============================================================================*/

int X3d_movie_set_play_speed(struct X3d_movie *movie,
	double play_speed);
/*******************************************************************************
LAST MODIFIED : 10 June 1998

DESCRIPTION :
Sets a movie so that it plays at a speed <play_speed> times the frame rate
==============================================================================*/

double X3d_movie_get_actual_framerate(struct X3d_movie *movie);
/*******************************************************************************
LAST MODIFIED : 10 June 1998

DESCRIPTION :
Gets the frame rate for the last second of a movie playing
==============================================================================*/

int X3d_movie_get_bounding_rectangle(
	struct X3d_movie *movie, int *left, int *bottom, int *width, int *height);
/*******************************************************************************
LAST MODIFIED : 9 September 1998

DESCRIPTION :
Gets a rectangle which will enclose the rendered movie.  Any unwanted dimensions
can just be passed a NULL ptr
==============================================================================*/

struct Time_object *X3d_movie_get_time_object(struct X3d_movie *movie);
/*******************************************************************************
LAST MODIFIED : 1 October 1998

DESCRIPTION :
==============================================================================*/

int DESTROY(X3d_movie)(struct X3d_movie **movie);
/*******************************************************************************
LAST MODIFIED : 10 June 1998

DESCRIPTION :
Closes a movie instance
==============================================================================*/

#endif /* !defined (MOVIE_EXTENSIONS_H) */
