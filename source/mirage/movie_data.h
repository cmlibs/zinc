/*******************************************************************************
FILE : movie_data.h

LAST MODIFIED : 10 February 1998

DESCRIPTION :
Manipulate the movie data structure using the stuff in Richards movie.c
==============================================================================*/
#if !defined (MOVIE_DATA_H)
#define MOVIE_DATA_H

/*
Global data
-----------
*/

/*
Module functions
----------------
*/

/*
Global functions
----------------
*/
#if defined (OLD_CODE)
int tracking_editor_read_movie(char *filename);
/*******************************************************************************
LAST MODIFIED : 10 February 1998

DESCRIPTION :
Clear a movie structure from memory, then alloc a new one, and read in
from file.
==============================================================================*/
#endif /* defined (OLD_CODE) */

void tracking_editor_limit_values(int *start,int *stop,int *view,int *node);
/*******************************************************************************
LAST MODIFIED : 10 February 1998

DESCRIPTION :
Check that the start,stop,view,node values agree with the movie data structure.
??? SEN node number isn't checked.
==============================================================================*/
#endif /* !defined (MOVIE_DATA_H) */
