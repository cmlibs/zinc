/*******************************************************************************
FILE : movie_data.c

LAST MODIFIED : 10 February 1998

DESCRIPTION :
Manipulate the movie data structure using the stuff in Richards movie.c
==============================================================================*/
#include "general/debug.h"
#include "mirage/movie.h"
#include "mirage/movie_data.h"


/*
Global data
-----------
*/
struct Mirage_movie *our_movie = (struct Mirage_movie *) NULL;

/*
Module functions
----------------
*/
static int minmax(int a,int b,int c)
/*******************************************************************************
LAST MODIFIED : 10 February 1998

DESCRIPTION :
Returns a st a > b && a < c.
==============================================================================*/
{
	int retval;

	ENTER(minmax);

	retval = a > c ? c : (a < b ? b : a);

	LEAVE;

	return retval;
} /* minmax */


/*
Global functions
----------------
*/
#if defined (OLD_CODE)
int tracking_editor_read_movie(char *filename)
/*******************************************************************************
LAST MODIFIED : 10 February 1998

DESCRIPTION :
Clear a movie structure from memory, then alloc a new one, and read in
from file.
==============================================================================*/
{
	int retval;
	struct Mirage_movie *tmp_movie;

	ENTER(tracking_editor_read_movie);

	if ((tmp_movie = read_Mirage_movie(filename)) != NULL)
	{
		if (our_movie != NULL)
		{
			DESTROY(Mirage_movie)(&our_movie);
		}
		our_movie = tmp_movie;
		retval = 1;
	}
	else
	{
		retval = 0;
	}

	LEAVE;

	return retval;
} /* tracking_editor_read_movie */
#endif /* defined (OLD_CODE) */

void tracking_editor_limit_values(int *start,int *stop,int *view,int *node)
/*******************************************************************************
LAST MODIFIED : 10 February 1998

DESCRIPTION :
Check that the start,stop,view,node values agree with the movie data structure.
??? SEN node number isn't checked.
==============================================================================*/
{
	int frame0,frame1;

	ENTER(tracking_editor_limit_values);

	if (our_movie != NULL)
	{
		frame0 = our_movie->start_frame_no;
		frame1 = our_movie->number_of_frames+our_movie->start_frame_no-1;

		if (*stop < *start)
		{
			*stop = frame1;
		}

		*view  = minmax(*view,0,our_movie->number_of_views);
		*start = minmax(*start,frame0,frame1);
		*stop  = minmax(*stop,*start,frame1);
	}

	LEAVE;
} /* tracking_editor_limit_values */
