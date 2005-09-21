/*******************************************************************************
FILE : geometry.h

LAST MODIFIED : 17 January 2000

DESCRIPTION :
The types and global variables for describing geometry.  Prototypes of functions
for performing coordinate transformations.
???DB.  What about PI ?
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#if !defined (GEOMETRY_H)
#define GEOMETRY_H


#include "general/enumerator.h"
#include "general/object.h"
#include "command/parser.h"


#if !defined (PI)
#define PI 3.14159265358979323846
#endif

/*
Global types
------------
*/
enum Coordinate_system_type
/*******************************************************************************
LAST MODIFIED : 24 August 1999

DESCRIPTION :
The possible coordinate systems.  Values specified to correspond to CMISS
==============================================================================*/
{
	CYLINDRICAL_POLAR=2,
	/*ELEMENT_XI=7,*/
	FIBRE=6,
	NOT_APPLICABLE=8, /* a field with no coordinate system*/
	OBLATE_SPHEROIDAL=5,
	PROLATE_SPHEROIDAL=4,
	RECTANGULAR_CARTESIAN=1,
	SPHERICAL_POLAR=3,
	/*TIME_SERIES=7,*/
	/* NORMALISED_WINDOW_COORDINATES is a rectangular coordinate system
		but indicates that it should be fixed relative to the window viewport 
		rather than in model cordinate space.  The convention is that it goes
		from -1 to 1 in x and y screen space and 0 to 1 in depth. */
	NORMALISED_WINDOW_COORDINATES=9,
	UNKNOWN_COORDINATE_SYSTEM=0
}; /* enum Coordinate_system_type */

struct Coordinate_system
/*******************************************************************************
LAST MODIFIED : 24 August 1999

DESCRIPTION :
==============================================================================*/
{
  enum Coordinate_system_type type;
  union
  {
    float focus;
  } parameters;
}; /* struct Coordinate_system */

enum Projection_type
/*******************************************************************************
LAST MODIFIED : 31 May 2000

DESCRIPTION :
???DB.  There will be different types of CYLINDRICAL, depending on where the
cut is and if the projection should overlap (3/2 times round).
==============================================================================*/
{
	CYLINDRICAL_PROJECTION,
	HAMMER_PROJECTION,
	POLAR_PROJECTION,
	THREED_PROJECTION
}; /* enum Projection_type */

typedef struct
/*******************************************************************************
LAST MODIFIED : 14 July 1995

DESCRIPTION :
Represents a linear transformation  translate + T*x .
==============================================================================*/
{
	/* translate */
	float translate_x,translate_y,translate_z;
	/* transformation */
	float txx,txy,txz,tyx,tyy,tyz,tzx,tzy,tzz;
} Linear_transformation;

/*
Global functions
----------------
*/
int linear_transformation(Linear_transformation *linear_transformation,float x,
	float y,float z,float *result_x,float *result_y,float *result_z);
/*******************************************************************************
LAST MODIFIED : 14 July 1995

DESCRIPTION :
Performs a <linear_transformation>.
==============================================================================*/

int cartesian_to_cylindrical_polar(float x,float y,float z_in,float *r,
	float *theta,float *z,float *jacobian);
/*******************************************************************************
LAST MODIFIED : 21 August 1995

DESCRIPTION :
For transforming from cylindrical polar to cartesian coordinates.
x = r*cos(theta)
y = r*sin(theta)
z = z_in
==============================================================================*/

int cylindrical_polar_to_cartesian(float r,float theta,float z_in,float *x,
	float *y,float *z,float *jacobian);
/*******************************************************************************
LAST MODIFIED : 21 August 1995

DESCRIPTION :
For transforming from cylindrical polar to cartesian coordinates.
x = r*cos(theta)
y = r*sin(theta)
z = z_in
==============================================================================*/

int cartesian_to_spherical_polar(float x,float y,float z,float *r,float *theta,
	float *phi,float *jacobian);
/*******************************************************************************
LAST MODIFIED : 17 January 2000

DESCRIPTION :
For transforming from spherical polar to cartesian coordinates.
x = r*cos(phi)*cos(theta)
y = r*cos(phi)*sin(theta)
z = r*sin(phi)
==============================================================================*/

int spherical_polar_to_cartesian(float r,float theta,float phi,float *x,
	float *y,float *z,float *jacobian);
/*******************************************************************************
LAST MODIFIED : 6 July 1994

DESCRIPTION :
For transforming from spherical polar to cartesian coordinates.
x = r*cos(phi)*cos(theta)
y = r*cos(phi)*sin(theta)
z = r*sin(phi)
==============================================================================*/

int cartesian_to_prolate_spheroidal(float x,float y,float z,float focus,
	float *lambda,float *mu,float *theta,float *jacobian);
/*******************************************************************************
LAST MODIFIED : 18 October 1995

DESCRIPTION :
For transforming from cartesian to prolate spheroidal coordinates.
x = focus*cosh(lambda)*cos(mu)
y = focus*sinh(lambda)*sin(mu)*cos(theta)
z = focus*sinh(lambda)*sin(mu)*sin(theta)
==============================================================================*/

int prolate_spheroidal_to_cartesian(float lambda,float mu,float theta,
	float focus,float *x,float *y,float *z,float *jacobian);
/*******************************************************************************
LAST MODIFIED : 17 October 1995

DESCRIPTION :
For transforming from prolate spheroidal to cartesian coordinates.
x = focus*cosh(lambda)*cos(mu)
y = focus*sinh(lambda)*sin(mu)*cos(theta)
z = focus*sinh(lambda)*sin(mu)*sin(theta)
==============================================================================*/

int Hammer_projection(float mu,float theta,float *x,float *y,float *jacobian);
/*******************************************************************************
LAST MODIFIED : 19 October 1995

DESCRIPTION :
For performing the Hammer projection starting from prolate spheroidal
coordinates.
==============================================================================*/

int polar_projection(float mu,float theta,float *x,float *y,float *jacobian);
/*******************************************************************************
LAST MODIFIED : 19 October 1995

DESCRIPTION :
For performing the polar projection starting from prolate spheroidal
coordinates.
==============================================================================*/

int oblate_spheroidal_to_cartesian(float lambda,float mu,float theta,
	float focus,float *x,float *y,float *z,float *jacobian);
/*******************************************************************************
LAST MODIFIED : 8 September 1998

DESCRIPTION :
For transforming from oblate spheroidal to cartesian coordinates.
???DB.  What are the formulae.
==============================================================================*/

enum Coordinate_system_type get_coordinate_system_type(
	struct Coordinate_system *coordinate_system);
/*******************************************************************************
LAST MODIFIED : 25 January 1999

DESCRIPTION :
Returns the type of the coordinate system passed to it.
==============================================================================*/

PROTOTYPE_ENUMERATOR_FUNCTIONS(Coordinate_system_type);

PROTOTYPE_COPY_OBJECT_FUNCTION(Coordinate_system);

int set_Coordinate_system(struct Parse_state *state,
	void *coordinate_system_void,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 22 January 1999

DESCRIPTION :
Currently only allows rectangular cartesian to be set.
???RC JW to change to struct Coordinate_system, handle parsing of focus for
prolate, etc.
==============================================================================*/

char *Coordinate_system_string(
	struct Coordinate_system *coordinate_system);
/*******************************************************************************
LAST MODIFIED : 28 January 1999

DESCRIPTION :
Allocates and returns a pointer to a string containing the type and focus (if 
any) for the given coordinate_system. eg "rectangular_cartesian" or 
"prolate_spheroidal focus 1.0". Must Deallocate the string in the calling
function.
==============================================================================*/

int Coordinate_systems_match(struct Coordinate_system *coordinate_system1,
	struct Coordinate_system *coordinate_system2);
/*******************************************************************************
LAST MODIFIED : 25 August 1999

DESCRIPTION :
Returns true if the two coordinate systems are the same - includes comparing
focus for prolate and oblate spheroidal systems.
==============================================================================*/

int convert_Coordinate_system(
  struct Coordinate_system *source_coordinate_system,
  int number_of_source_coordinates, float *source_coordinates,
  struct Coordinate_system *destination_coordinate_system,
  int number_of_destinations_coordinates, float *destination_coordinates,
  float *jacobian);
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
Convert the <source_coordinates> into the <destination_coordinates>.
Calculate the <jacobian> if not NULL.
???DB.  Should the dimension be part of struct Coordinate_system ?
???DB.  Can we get rid of most of io_devices/conversion ?
==============================================================================*/
#endif /* !defined (GEOMETRY_H) */
