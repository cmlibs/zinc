/*******************************************************************************
FILE : movie_graphics.c

LAST MODIFIED : 21 July 1999

DESCRIPTION :
==============================================================================*/
#include "general/debug.h"
#include "general/object.h"
#include "graphics/graphics_window.h"
#include "graphics/movie_graphics.h"
#include "three_d_drawing/movie_extensions.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

struct Movie_graphics
{
	struct X3d_movie *x3d_movie;
	struct Graphics_window *graphics_window;
};

/*
Global functions
----------------
*/

struct Movie_graphics *CREATE(Movie_graphics)(char *filename, 
	enum X3d_movie_create_option create_option)
/*******************************************************************************
LAST MODIFIED : 21 July 1999

DESCRIPTION :
Attempts to create a movie graphics object.
==============================================================================*/
{
	struct Movie_graphics *movie;
	struct X3d_movie *x3d_movie;

	ENTER(CREATE(Movie_graphics));
	if (filename)
	{
		if (x3d_movie = CREATE(X3d_movie)(filename, create_option))
		{
			if (ALLOCATE(movie, struct Movie_graphics, 1))
			{
				movie->x3d_movie = x3d_movie;
				movie->graphics_window = (struct Graphics_window *)NULL;
			}
			else
			{
				DESTROY(X3d_movie)(&x3d_movie);
				display_message(ERROR_MESSAGE,
					"CREATE(Movie_graphics).  Unable to allocate movie graphics structure");
				movie = (struct Movie_graphics *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Movie_graphics).  Unable to initialise movie extensions");
			movie = (struct Movie_graphics *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Movie_graphics).  Missing filename");
		movie = (struct Movie_graphics *)NULL;
	}
	LEAVE;

	return (movie);
} /* CREATE(Movie_graphics) */

int DESTROY(Movie_graphics)(struct Movie_graphics **movie)
/*******************************************************************************
LAST MODIFIED : 22 July 1999

DESCRIPTION :
Closes a movie instance
x==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Movie_graphics));
	if (movie && *movie)
	{
		if ((*movie)->x3d_movie)
		{
			DESTROY(X3d_movie)(&(*movie)->x3d_movie);
		}
		if ((*movie)->graphics_window)
		{
			DEACCESS(Graphics_window)(&(*movie)->graphics_window);
		}
		DEALLOCATE(*movie);
		*movie = (struct Movie_graphics *)NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Movie_graphics).  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Movie_graphics) */

struct X3d_movie *Movie_graphics_get_X3d_movie(
	struct Movie_graphics *movie)
/*******************************************************************************
LAST MODIFIED : 21 July 1999

DESCRIPTION :
==============================================================================*/
{
	struct X3d_movie *return_x3d_movie;

	ENTER(Movie_graphics_get_X3d_movie);
	if (movie)
	{
		return_x3d_movie = movie->x3d_movie;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Movie_graphics_get_X3d_movie:  Invalid arguments");
		return_x3d_movie = (struct X3d_movie *)NULL;
	}
	LEAVE;

	return (return_x3d_movie);
} /* Movie_graphics_get_X3d_movie */

int Movie_graphics_set_Graphics_window(
	struct Movie_graphics *movie, struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 22 July 1999

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Movie_graphics_set_Graphics_window);
	if (movie)
	{
		REACCESS(Graphics_window)(&movie->graphics_window,graphics_window);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Movie_graphics_set_Graphics_window:  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Movie_graphics_set_Graphics_window */

int Movie_graphics_add_frame_to_movie(struct Movie_graphics *movie,
	int width, int height, int force_onscreen)
/*******************************************************************************
LAST MODIFIED : 30 November 1999

DESCRIPTION :
Ensures that the source is up to date and then adds a frame to the X3d_movie.
The <width> and <height> specified are requested from the graphics window
but not necessarily respected by it, requesting zero size matches the 
graphics_window size.  If <force_onscreen> is non zero the pixels will always
be grabbed from the graphics window on screen.
==============================================================================*/
{
	char *frame_data;
	int mleft, mbottom, mwidth, mheight;
	int return_code;

	ENTER(Movie_graphics_add_frame_to_movie);
	if (movie)
	{
		if (movie->graphics_window)
		{
			if(X3d_movie_get_bounding_rectangle(movie->x3d_movie, &mleft, &mbottom, &mwidth, &mheight))
			{
				/* If the movie already has frames override the width and height */
				width = mwidth;
				height = mheight;
			}
			if (Graphics_window_get_frame_pixels(movie->graphics_window, TEXTURE_ABGR,
				&width, &height, &frame_data, force_onscreen))
			{
				X3d_movie_add_frame(movie->x3d_movie, width, height, frame_data);
				DEALLOCATE(frame_data);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Movie_graphics_add_frame_to_movie:  Could not allocate array for frame_data");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Movie_graphics_add_frame_to_movie:  Must bind movie to a graphics_window before adding frames");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Movie_graphics_add_frame_to_movie:  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Movie_graphics_add_frame_to_movie */
