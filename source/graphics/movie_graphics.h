/*******************************************************************************
FILE : movie_graphics.h

LAST MODIFIED : 3 February 2000

DESCRIPTION :
==============================================================================*/
#if !defined (MOVIE_GRAPHICS_H)
#define MOVIE_GRAPHICS_H
#include "general/object.h"
#include "general/list.h"
#include "general/manager.h"
#include "graphics/graphics_window.h"
#include "three_d_drawing/movie_extensions.h"

/*
Module types
------------
*/

struct Movie_graphics;

DECLARE_LIST_TYPES(Movie_graphics);

DECLARE_MANAGER_TYPES(Movie_graphics);

/*
Global functions
----------------
*/

struct Movie_graphics *CREATE(Movie_graphics)(char *name,char *filename, 
	enum X3d_movie_create_option create_option);
/*******************************************************************************
LAST MODIFIED : 2 February 2000

DESCRIPTION :
Attempts to create a movie graphics object.
==============================================================================*/

int DESTROY(Movie_graphics)(struct Movie_graphics **movie);
/*******************************************************************************
LAST MODIFIED : 21 July 1999

DESCRIPTION :
Closes a movie instance
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Movie_graphics);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Movie_graphics);

PROTOTYPE_LIST_FUNCTIONS(Movie_graphics);

PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Movie_graphics,name,char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(Movie_graphics,name,char *);

PROTOTYPE_MANAGER_FUNCTIONS(Movie_graphics);

PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Movie_graphics,name,char *);

struct X3d_movie *Movie_graphics_get_X3d_movie(
	struct Movie_graphics *movie);
/*******************************************************************************
LAST MODIFIED : 21 July 1999

DESCRIPTION :
==============================================================================*/

int Movie_graphics_has_X3d_movie(struct Movie_graphics *movie,
	void *x3d_movie_void);
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Conditional function returning true if <movie> contains the given <x3d_movie>.
==============================================================================*/

int Movie_graphics_set_Graphics_window(
	struct Movie_graphics *movie, struct Graphics_window *graphics_window);
/*******************************************************************************
LAST MODIFIED : 22 July 1999

DESCRIPTION :
==============================================================================*/

int Movie_graphics_add_frame_to_movie(struct Movie_graphics *movie,
	int width, int height, int force_onscreen);
/*******************************************************************************
LAST MODIFIED : 30 November 1999

DESCRIPTION :
Ensures that the source is up to date and then adds a frame to the X3d_movie.
The <width> and <height> specified are requested from the graphics window
but not necessarily respected by it, requesting zero size matches the 
graphics_window size.  If <force_onscreen> is non zero the pixels will always
be grabbed from the graphics window on screen.
==============================================================================*/

int list_Movie_graphics(struct Movie_graphics *movie,void *dummy);
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Writes the properties of the <movie> to the command window.
==============================================================================*/

int set_Movie_graphics(struct Parse_state *state,void *movie_address_void,
	void *movie_graphics_manager_void);
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Modifier function to set the movie from a command.
==============================================================================*/

#endif /* !defined (MOVIE_GRAPHICS_H) */
