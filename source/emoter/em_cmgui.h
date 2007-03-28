/*******************************************************************************
FILE: em_cmgui.h

LAST MODIFIED : 25 Febuary 1998

DESCRIPTION :
Header file for EM stuff for DB and cmgui.
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

#if !defined (EM_CMGUI_H)
#define EM_CMGUI_H


struct EM_Object
/*******************************************************************************
LAST MODIFIED : 25 February 1998

DESCRIPTION : Contains the data read in from the EM basis file.
	A = U.w.V^T
==============================================================================*/
{
	double *u;
	double *w;
	double *v;
	double mode_one_std;
	int *index;
	int n_modes;
	int n_nodes;
	int m;
	int *minimum_nodes;
	int number_of_minimum_nodes;
}; /* struct EM_Object */


/*******************************************************************************
LAST MODIFIED : 27 Febuary 1998

DESCRIPTION :
Allocate the object that contains the EM data.
==============================================================================*/
struct EM_Object *create_EM_Object(int n_modes,int n_nodes);

/*******************************************************************************
LAST MODIFIED : 27 Febuary 1998

DESCRIPTION :
Destroy the object that contains the EM data.
==============================================================================*/
void destroy_EM_Object(struct EM_Object **em_object);

/*******************************************************************************
LAST MODIFIED : 26 Febuary 1998

DESCRIPTION :
Write out a basis data file.
==============================================================================*/
void EM_write_basis(char *filename,struct EM_Object *em_object);

struct EM_Object *EM_read_basis(char *filename,
	struct IO_stream_package *io_stream_package, struct EM_Object **em_object,
	int *node_index_list, int number_in_node_list);
/*******************************************************************************
LAST MODIFIED : 28 March 2007

DESCRIPTION :
Read in a file containing a basis function. Creates a EM_Object and returns
it, deallocating the old em_object and pointing it to the new one.
The <node_index_list> and <number_in_node_list> are required when the basis_file
is a version two file as these basis files do not include information about
the corresponding nodes.
==============================================================================*/

/*******************************************************************************
LAST MODIFIED : 26 Febuary 1998

DESCRIPTION :
Returns the number of modes.
==============================================================================*/
int EM_number_of_modes(struct EM_Object *em_object);

/*******************************************************************************
LAST MODIFIED : 26 Febuary 1998

DESCRIPTION :
Returns the number of nodes.
==============================================================================*/
int EM_number_of_nodes(struct EM_Object *em_object);

/*******************************************************************************
LAST MODIFIED : 26 Febuary 1998

DESCRIPTION :
Reconstructs a face from a EM_Object and a set of weights. Returns a
pointer to a vector of double, of size 3*n_nodes, contains the x,y,z
coordinates of the reconstructed face, in order x1,y1,z1,x2,y2,z2, ...
Weights must be of size n_modes.
==============================================================================*/
double *EM_reconstruct(double *weights,struct EM_Object *em_object);

/*******************************************************************************
LAST MODIFIED : 26 Febuary 1998

DESCRIPTION :
Returns the max/min weights for the em data. The arrays max and min must be
of size n_modes.
==============================================================================*/
void EM_get_weights(double *max,double *min,struct EM_Object *em_object);

/*******************************************************************************
LAST MODIFIED : 27 Febuary 1998

DESCRIPTION :
Projects a face onto a basis function. x must be of size n_nodes*3, and
contains the coordinates of the face to be deconstructed, in the order
x1,y1,z1,x2,y2,z2,.... index contains the node numbers for face points.
An array of size n_modes is allocated and returned. This contains the
weights for each face mode.
==============================================================================*/
double *EM_project(double *x,int *index,struct EM_Object *em_object);

/*******************************************************************************
LAST MODIFIED : 14 April 1998

DESCRIPTION :
Calculate the mean value of the V*w for mode one, it is then used as a
reference offset.
==============================================================================*/
double EM_standard_mode_one(struct EM_Object *em_object);

int EM_calculate_minimum_nodeset(struct EM_Object *em_object, int number_of_modes);
/*******************************************************************************
LAST MODIFIED : 7 September 1999

DESCRIPTION :
Finds a independent set of nodes that are sufficient to distinguish the first
<number_of_modes> modes in the EM
==============================================================================*/
#endif
