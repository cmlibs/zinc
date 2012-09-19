/*******************************************************************************
FILE : texture_line.h

LAST MODIFIED : 30 August 1996

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
#if !defined (TEXTURE_LINE_H)
#define TEXTURE_LINE_H

/*
Global functions
----------------
*/
struct VT_texture_curve *get_curve_from_list(struct VT_texture_curve **ptrfirst,
	struct VT_texture_curve *curve);
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
==============================================================================*/

int add_curve_to_list(struct VT_texture_curve **ptrfirst,
	struct VT_texture_curve *newcurve);
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
==============================================================================*/

int remove_curve_from_list(struct VT_texture_curve **ptrfirst,
	struct VT_texture_curve *curve);
/*******************************************************************************
LAST MODIFIED : 31 July 1996

DESCRIPTION :
==============================================================================*/

double line_segment_potential(double k,double r1,double r2,double L,double q1,
	double q2);
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
Calculates potential at a point from a line segment (length L) of charge (q1,q2)
(charge/unit length) with q1,q2 being distances r1,r2 from the point
respectively. (The charge/unit length varies linearly from q1 to q2)
==============================================================================*/

double line_segment_distance(double k,double *p1,double *p2,double *p,double q1,
	double q2);
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
Calculates distance potential at a point p from a line segment p1-p2 of charge
(q1,q2)
==============================================================================*/

double blob_segment_distance(double k,double *p1,double *p2,double *p,double q1,
	double q2);
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
Calculates distance potential at a point p from a blob segment p1-p2 of charge
(q1,q2)
==============================================================================*/

double curve_segment_distance(double k,double *p1,double *p2,double *p3,
	double *p4,double *p,double q1,double q2);
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
Calculates distance potential at a point p from a curve segment defined by
p1-p2, slope p3,p4 with charge(q1,q2) the distance Q(t)-p is minimized by
translating to the origin and solving the minimum sum Q(t)*Q(t) =>
0 = sum Q'(t).Q(t)
==============================================================================*/

double soft_object_distance(double WR,double *p1,double *p,double q1);
/*******************************************************************************
LAST MODIFIED : 4 March 1997

DESCRIPTION :
==============================================================================*/
#endif
