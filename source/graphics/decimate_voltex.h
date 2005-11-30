/*******************************************************************************
FILE : decimate_voltex.h

LAST MODIFIED : 11 November 2005

DESCRIPTION :
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
