/*******************************************************************************
FILE : texture_line.h

LAST MODIFIED : 30 August 1996

DESCRIPTION :
==============================================================================*/
#if !defined (TEXTURE_LINE_H)
#define TEXTURE_LINE_H

#include "graphics/volume_texture_editor.h"

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

void select_line(struct Texture_window *tw);
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
Here the 1D line texture structure is invoked - if no line is currently being
edited (edit_line.index = 0) the first point is selected, and a line is drawn to
the current node cursor position. If a line is being edited then the second
point is recorded, the curve is complete and stored in the volume_texturedata
structure. edit_curve.index is reset to zero.
==============================================================================*/

void select_blob(struct Texture_window *tw);
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
Here the 1D blob texture structure is invoked - if no line is currently being
edited (edit_line.index = 0) the first point is selected, and a line is drawn to
the current node cursor position. If a line is being edited then the second
point is recorded, the curve is complete and stored in the volume_texture data
structure. edit_curve.index is reset to zero.
==============================================================================*/

void deselect_curve(struct Texture_window *tw);
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
Two points are selected and the coordinates stored in edit_line.  After two
deselect operations have been performed, the list is searched for the line
segment and it is removed.
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

void select_curve(struct Texture_window *tw,int next);
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
If next = 0, this just updates the appropriate point, otherwise it stores it and
if index = 3 the line is complete and it is stored in the list. The points are
p1,p2 and the slope control points p3,p4.
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

void select_soft(struct Texture_window *tw);
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
Here the 1D soft texture structure is invoked - if no line is currently being
edited (edit_line.index = 0) the first point is selected, and a line is drawn to
the current node cursor position. If a line is being edited then the second
point is recorded, the curve is complete and stored in the volume_texture data
structure. edit_curve.index is reset to zero.
==============================================================================*/
#endif
