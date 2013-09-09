/*******************************************************************************
FILE : decimate_voltex.h

LAST MODIFIED : 11 November 2005

DESCRIPTION :
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (DECIMATE_VOLTEX_H)
#define DECIMATE_VOLTEX_H

struct GT_voltex;

int GT_voltex_decimate_triangles(struct GT_voltex *voltex, 
	double threshold_distance);
/*******************************************************************************
LAST MODIFIED : 11 November 2005

DESCRIPTION :
Decimates triangle mesh
Implementing edge collapses following Garland and Heckbert 
"Surface Simplification Using Quadric Error Metrics" SIGGRAPH 97

Currently this routine will allow collapsed triangles which have hanging nodes.
The storage of the could be generalised to allow it to work on triangular meshes
other than just VT_voltex, maybe finite elements.  The normals probably
are not being updated properly either (if the collapsed triangles were fixed then
the normals should be calclated after this step rather than before).  The data
structures, Decimation_quadrics (vertices) and Decimation_cost (edges) should be
better merged with the rest of the storage and so lists could be used instead of
the current arrays of triangle pointers.
The decimation_cost could include a penalty according to the rate at which the 
data variables vary along the edge, so that where a display data value is varying
fastest the edge is less likely to collapse.
==============================================================================*/

#endif /* !defined (DECIMATE_VOLTEX_H) */
