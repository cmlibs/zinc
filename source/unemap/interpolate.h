/*******************************************************************************
FILE : interpolate.h

LAST MODIFIED : 12 May 2004

DESCRIPTION :
Function prototypes for calculating a finite element interpolation to data for a
special case.
==============================================================================*/
#if !defined (INTERPOLATE_H)
#define INTERPOLATE_H

#include <stddef.h>
#include "unemap/rig.h"
#include "unemap/mapping.h"

/*
Global functions
----------------
*/
struct Interpolation_function *calculate_interpolation_functio(
	enum Region_type region_type,int number_of_data,float *x_data_base,
	float *y_data_base,float *value_data_base,float *weight_data_base,
	int finite_element_mesh_rows,int finite_element_mesh_columns,
	float membrane_smoothing,float plate_bending_smoothing);
/*******************************************************************************
LAST MODIFIED : 12 May 2004

DESCRIPTION :
Creates an interpolation function from
1. data - <region_type>, <number_of_data>, <x_data_base>, <y_data_base>,
	<value_data_base> and <weight_data_base>
2. mesh - <finite_element_mesh_rows> and <finite_element_mesh_columns>
3. fitting - <membrane_smoothing> and <plate_bending_smoothing>
==============================================================================*/

int destroy_Interpolation_function(struct Interpolation_function **function);
/*******************************************************************************
LAST MODIFIED : 31 August 1992

DESCRIPTION :
This function deallocates the memory associated with the fields of <**function>,
deallocates the memory for <**function> and sets <*function> to NULL;
==============================================================================*/
#endif /* !defined (INTERPOLATE_H) */
