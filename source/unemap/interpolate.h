/*******************************************************************************
FILE : interpolate.h

LAST MODIFIED : 13 October 1997

DESCRIPTION :
Function prototypes for calculating a finite element interpolation to data for a
special case.
==============================================================================*/
#include <stddef.h>
#include "unemap/rig.h"
#include "unemap/mapping.h"

/*
Global functions
----------------
*/
struct Interpolation_function *calculate_interpolation_functio(
	enum Map_type map_type,struct Rig *rig,struct Region *region,
	int *event_number_address,float potential_time,int *datum_address,
	int *start_search_interval_address,int *end_search_interval_address,
	char undecided_accepted,int finite_element_mesh_rows,
	int finite_element_mesh_columns,float membrane_smoothing,
	float plate_bending_smoothing);
/*******************************************************************************
LAST MODIFIED : 13 October 1997

DESCRIPTION :
There are three groups of arguments for this function
???Put into structures ?
Input
1. data - <number_of_data>, <x>, <y>, <value> and <weight>
2. finite element mesh - <x_left>, <y_bottom>, <number_of_rows>, <row_height>,
   <number_of_columns> and <column_width>
Output
3. interpolation function - <f>, <dfdx>, <dfdy> and <d2fdxdy>
???I'm not sure if memory should be assigned inside this function for these or
if they should be 1-D or 2-D arrays ?
==============================================================================*/

int destroy_Interpolation_function(struct Interpolation_function **function);
/*******************************************************************************
LAST MODIFIED : 31 August 1992

DESCRIPTION :
This function deallocates the memory associated with the fields of <**function>,
deallocates the memory for <**function> and sets <*function> to NULL;
==============================================================================*/
