/*******************************************************************************
FILE : movie_graphics.c

LAST MODIFIED : 3 February 2000

DESCRIPTION :
==============================================================================*/
#include "general/debug.h"
#include "general/object.h"
#include "general/indexed_list_private.h"
#include "general/manager_private.h"
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
	char *name;
	struct X3d_movie *x3d_movie;
	struct Graphics_window *graphics_window;
	int access_count;
};

FULL_DECLARE_INDEXED_LIST_TYPE(Movie_graphics);

FULL_DECLARE_MANAGER_TYPE(Movie_graphics);


/*
Global functions
----------------
*/
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Movie_graphics,name,char *,strcmp)

DECLARE_LOCAL_MANAGER_FUNCTIONS(Movie_graphics)

struct Movie_graphics *CREATE(Movie_graphics)(char *name,char *filename, 
	enum X3d_movie_create_option create_option)
/*******************************************************************************
LAST MODIFIED : 2 February 2000

DESCRIPTION :
Attempts to create a movie graphics object.
==============================================================================*/
{
	struct Movie_graphics *movie;
	struct X3d_movie *x3d_movie;

	ENTER(CREATE(Movie_graphics));
	if (name&&filename)
	{
		if (x3d_movie = CREATE(X3d_movie)(filename, create_option))
		{
			if (ALLOCATE(movie, struct Movie_graphics, 1)&&
				ALLOCATE(movie->name,char,strlen(name)+1))
			{
				strcpy(movie->name,name);
				movie->x3d_movie = x3d_movie;
				movie->graphics_window = (struct Graphics_window *)NULL;
				movie->access_count = 0;
			}
			else
			{
				DESTROY(X3d_movie)(&x3d_movie);
				display_message(ERROR_MESSAGE,"CREATE(Movie_graphics).  "
					"Unable to allocate movie graphics structure");
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
LAST MODIFIED : 2 February 2000

DESCRIPTION :
Closes a movie instance
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Movie_graphics));
	if (movie && *movie)
	{
		if (0==(*movie)->access_count)
		{
			DEALLOCATE((*movie)->name);
			if ((*movie)->x3d_movie)
			{
				DESTROY(X3d_movie)(&(*movie)->x3d_movie);
			}
			if ((*movie)->graphics_window)
			{
				DEACCESS(Graphics_window)(&(*movie)->graphics_window);
			}
			DEALLOCATE(*movie);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"DESTROY(Movie_graphics).  "
				"Non-zero access_count %d",(*movie)->access_count);
			return_code = 0;
		}
		*movie = (struct Movie_graphics *)NULL;
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

DECLARE_OBJECT_FUNCTIONS(Movie_graphics)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Movie_graphics)

DECLARE_INDEXED_LIST_FUNCTIONS(Movie_graphics)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Movie_graphics,name, \
	char *,strcmp)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Movie_graphics,name)
{
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Movie_graphics,name));
	if (source&&destination)
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITH_IDENTIFIER(Movie_graphics,name).  Not supported");
		return_code=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITH_IDENTIFIER(Movie_graphics,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Movie_graphics,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Movie_graphics,name)
{
	int return_code;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Movie_graphics,name));
	if (source&&destination)
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITHOUT_IDENTIFIER(Movie_graphics,name).  Not supported");
		return_code=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITHOUT_IDENTIFIER(Movie_graphics,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Movie_graphics,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Movie_graphics,name,char *)
{
	char *destination_name;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(Movie_graphics,name));
	if (name&&destination)
	{
		if (name)
		{
			if (ALLOCATE(destination_name,char,strlen(name)+1))
			{
				strcpy(destination_name,name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_IDENTIFIER(Movie_graphics,name).  Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			/* copy name */
			DEALLOCATE(destination->name);
			destination->name=destination_name;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Movie_graphics,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Movie_graphics,name) */

DECLARE_MANAGER_FUNCTIONS(Movie_graphics)
DECLARE_MANAGER_IDENTIFIER_FUNCTIONS(Movie_graphics,name,char *)

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

int Movie_graphics_has_X3d_movie(struct Movie_graphics *movie,
	void *x3d_movie_void)
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Conditional function returning true if <movie> contains the given <x3d_movie>.
==============================================================================*/
{
	int return_code;
	struct X3d_movie *x3d_movie;

	ENTER(Movie_graphics_has_X3d_movie);
	if (movie&&(x3d_movie=(struct X3d_movie *)x3d_movie_void))
	{
		return_code=(movie->x3d_movie == x3d_movie);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Movie_graphics_has_X3d_movie.  Invalid argument(s_");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Movie_graphics_has_X3d_movie */

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

int list_Movie_graphics(struct Movie_graphics *movie,void *dummy)
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Writes the properties of the <movie> to the command window.
==============================================================================*/
{
	int return_code;

	ENTER(list_Movie_graphics);
	USE_PARAMETER(dummy);
	if (movie)
	{
		return_code=1;
		/* write the name */
		display_message(INFORMATION_MESSAGE,"movie : %s\n",movie->name);
	}
	else
	{
		display_message(ERROR_MESSAGE,"list_Movie_graphics.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_Movie_graphics */

int set_Movie_graphics(struct Parse_state *state,void *movie_address_void,
	void *movie_graphics_manager_void)
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Modifier function to set the movie from a command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct MANAGER(Movie_graphics) *movie_graphics_manager;
	struct Movie_graphics *temp_movie,**movie_address;

	ENTER(set_Movie_graphics);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if ((movie_address=(struct Movie_graphics **)movie_address_void)&&
					(movie_graphics_manager=
						(struct MANAGER(Movie_graphics) *)movie_graphics_manager_void))
				{
					if (fuzzy_string_compare(current_token,"NONE"))
					{
						if (*movie_address)
						{
							DEACCESS(Movie_graphics)(movie_address);
							*movie_address=(struct Movie_graphics *)NULL;
						}
						return_code=1;
					}
					else
					{
						if (temp_movie=FIND_BY_IDENTIFIER_IN_MANAGER(Movie_graphics,name)(
							current_token,movie_graphics_manager))
						{
							if (*movie_address!=temp_movie)
							{
								DEACCESS(Movie_graphics)(movie_address);
								*movie_address=ACCESS(Movie_graphics)(temp_movie);
							}
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"set_Movie_graphics.  Movie does not exist");
							return_code=0;
						}
					}
					shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Movie_graphics.  Invalid argument(s)");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," MOVIE_NAME|none");
				if (movie_address=(struct Movie_graphics **)movie_address_void)
				{
					if (temp_movie= *movie_address)
					{
						display_message(INFORMATION_MESSAGE,"[%s]",temp_movie->name);
					}
					else
					{
						display_message(INFORMATION_MESSAGE,"[none]");
					}
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing movie name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Movie_graphics.  Missing state");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* set_Movie_graphics */
