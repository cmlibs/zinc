/*******************************************************************************
FILE : debug.h

LAST MODIFIED : 26 November 2001

DESCRIPTION :
Function definitions for debugging.
==============================================================================*/
#if !defined (DEBUG_H)
#define DEBUG_H

#include <stdarg.h>

/*
Macros
------
*/

#define ENTER( function_name )

#define LEAVE

/* Following are the only question mark operators allowed in CMGUI, needed
	 because it is machine dependent whether a pointer or NULL is returned for
	 an allocation of zero size. Inlined for optimisation */
#if defined (OPTIMISED)

#define ALLOCATE( result , type , number ) \
 ( result = ( 0 < ( number ) ) ? ( type * )malloc( ( number ) * sizeof( type ) ) : ( type * )NULL )

#define DEALLOCATE( ptr ) { if ( ptr ) { free( (char *)( ptr ) ); ( ptr ) = NULL; } }

#define REALLOCATE( final , initial , type , number ) \
 ( final = ( 0 < ( number ) ) ? ( type * )realloc( (char *)( initial ) , ( number ) * sizeof( type ) ) : ( type * )NULL )

#else /* defined (OPTIMISED) */

#define ALLOCATE( result , type , number ) \
( result = ( type *) allocate( ( number ) * sizeof( type ) , __FILE__ , \
	__LINE__, #type ))

#define DEALLOCATE( ptr ) \
{ deallocate((char *) ptr , __FILE__ , __LINE__ ); ( ptr )=NULL;}

#define REALLOCATE( final , initial , type , number ) \
( final = ( type *) reallocate( (char *)( initial ) , \
	( number ) * sizeof( type ) , __FILE__ , __LINE__, #type ))

#endif /* defined (OPTIMISED) */

/*
Global variables
----------------
*/

/* temporary storage string */
#define GLOBAL_TEMP_STRING_SIZE 1000
extern char global_temp_string[GLOBAL_TEMP_STRING_SIZE];

/*
Global functions
----------------
*/
#if defined (USE_PARAMETER_ON)
void use_parameter(int dummy, ... );
/*******************************************************************************
LAST MODIFIED : 10 June 1999

DESCRIPTION :
Prototype for function which is called in the development stage (when
USE_PARAMETER_ON is defined) to swallow unused parameters to functions which
would otherwise cause compiler warnings. For example, parameter <dummy_void>
is swallowed with the call USE_PARAMETER(dummy_void); at the start of function.
==============================================================================*/
#define USE_PARAMETER(dummy) use_parameter(0,dummy)
#else /* defined (USE_PARAMETER_ON) */
#define USE_PARAMETER(dummy)
#endif /* defined (USE_PARAMETER_ON) */

#if !defined (OPTIMISED)

char *allocate(unsigned size,char *file_name,int line_number, char *type);
/*******************************************************************************
LAST MODIFIED : 7 January 1998

DESCRIPTION :
Wrapper for malloc.
==============================================================================*/

void deallocate(char *ptr,char *file_name,int line_number);
/*******************************************************************************
LAST MODIFIED : 7 January 1998

DESCRIPTION :
Wrapper for free.
==============================================================================*/

char *reallocate(char *ptr,unsigned size,char *file_name,int line_number, char *type);
/*******************************************************************************
LAST MODIFIED : 7 January 1998

DESCRIPTION :
Wrapper for realloc.
==============================================================================*/

#endif /* !defined (OPTIMISED) */

int list_memory(int count, int show_pointers, int increment_counter, 
	int show_structures);
/*******************************************************************************
LAST MODIFIED : 29 February 2000

DESCRIPTION :
Writes out memory blocks currently allocated.  Each time this is called an internal
counter is incremented and all subsequent ALLOCATIONS marked with this new
count_number.  i.e. To find a leak run till before the leak (all these allocations
will be marked count 1), call list_memory increment, 
do the leaky thing several times (these have count 2), call list_memory increment, 
do the leaky thing once (so that any old stuff with count 2 should have been deallocated 
and recreated with count 3) and then list_memory 2.  This should list no memory.
If <count_number> is zero all the memory allocated is written out.
If <count_number> is negative no memory is written out, just the total.
If <count_number> is positive only the memory with that count is written out.
???DB.  printf used because want to make sure that no allocation is going on
	while printing.
<show_pointers> toggles the output format to include the actual memory addresses
or not.  (It isn't useful for testing and output to record the changing addresses).
If <show_structures> is set then for known types the objects are cast to the
actual object type and then the appropriate list function is called.
==============================================================================*/

#endif /* !defined (DEBUG_H) */
