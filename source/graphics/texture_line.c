/*******************************************************************************
FILE : texture_line.c

LAST MODIFIED : 30 August 1996

DESCRIPTION :
==============================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <Xm/Xm.h>
#include <X11/Intrinsic.h>
#include <Mrm/MrmAppl.h>
#include <Mrm/MrmPublic.h>
#include <Xm/MwmUtil.h>
#include <Xm/Protocols.h>
#include "general/debug.h"
#include "graphics/complex.h"
#include "general/geometry.h"
#include "graphics/graphics_library.h"
#include "graphics/laguer.h"
#include "graphics/mcubes.h"
#include "graphics/texture_line.h"
#include "graphics/volume_texture.h"
#include "graphics/volume_texture_editor.h"
#include "user_interface/message.h"

/*
Module constants
----------------
*/
/*???DB.  Get rid of ? */
#define INFINITY 10000000000.0

/*
Global functions
----------------
*/
struct VT_texture_curve *get_curve_from_list(struct VT_texture_curve **ptrfirst,
	struct VT_texture_curve *curve)
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
==============================================================================*/
{
	struct VT_texture_curve *p;

	ENTER(get_curve_from_list);
	/* default return value */
	p=(struct VT_texture_curve *)NULL;
	/* check arguments */
	if (ptrfirst&&curve)
	{
		if (p= *ptrfirst)
		{
			while (p&&
				!(((p->point1[0]==curve->point1[0])&&(p->point1[1]==curve->point1[1])&&
				(p->point1[2]==curve->point1[2])&&(p->point2[0]==curve->point2[0])
				&&(p->point2[1]==curve->point2[1])&&(p->point2[2]==curve->point2[2]))||
				((p->point2[0]==curve->point1[0])&&(p->point2[1]==curve->point1[1])&&
				(p->point2[2]==curve->point1[2])&&(p->point1[0]==curve->point2[0])&&
				(p->point1[1]==curve->point2[1])&&(p->point1[2]==curve->point2[2]))))
			{
				p=p->ptrnext;
			}
			if (!p)
			{
				display_message(ERROR_MESSAGE,"get_curve_from_list.  No such curve");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"get_curve_from_list.  Empty list");
			p=(struct VT_texture_curve *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_curve_from_list.  Invalid argument(s)");
		p=(struct VT_texture_curve *)NULL;
	}
	LEAVE;

	return (p);
} /* get_curve_from_list */

int add_curve_to_list(struct VT_texture_curve **ptrfirst,
	struct VT_texture_curve *newcurve)
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct VT_texture_curve **p;

	ENTER(add_curve_to_list);
/*???debug */
printf("adding curve to list\n");
	/* default return value */
	return_code=0;
	/* checking arguments */
	if (newcurve&&ptrfirst)
	{
		newcurve->ptrnext=(struct VT_texture_curve *)NULL;
		p=ptrfirst;
		while (*p)
		{
			p= &((*p)->ptrnext);
		}
		*p=newcurve;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"add_curve_to_list.  Invaild argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* add_curve_to_list */

int remove_curve_from_list(struct VT_texture_curve **ptrfirst,
	struct VT_texture_curve *curve)
/*******************************************************************************
LAST MODIFIED : 31 July 1996

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct VT_texture_curve *p;

	ENTER(remove_curve_from_list);
/*???debug */
printf("removing curve from list\n");
	/* default return code */
	return_code=0;
	/* checking arguments */
	if (curve&&ptrfirst)
	{
		if (p= *ptrfirst)
		{
			if (p==curve)
			{
				*ptrfirst=curve->ptrnext;
				return_code=1;
			}
			else
			{
				while ((p->ptrnext)&&(p->ptrnext!=curve))
				{
					p=p->ptrnext;
				}
				if (p->ptrnext==curve)
				{
					p->ptrnext=curve->ptrnext;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"remove_curve_from_list.  Missing curve");
					return_code=0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"remove_curve_from_list.  Empty_list");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"remove_curve_from_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* remove_curve_from_list */

void select_line(struct Texture_window *tw)
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
Here the 1D line texture structure is invoked - if no line is currently being
edited (edit_line.index = 0) the first point is selected, and a line is drawn to
the current node cursor position. If a line is being edited then the second
point is recorded, the curve is complete and stored in the volume_texturedata
structure. edit_curve.index is reset to zero.
==============================================================================*/
{
	int i;
	struct VT_texture_curve *new_curve;

	ENTER(select_line);
	/* checking argument */
	if (tw)
	{
		(tw->edit_curve).type=1;
		if (0==(tw->edit_curve).index)
		{
			/* new curve to edit */
			(tw->edit_curve).index=1;
			(tw->edit_curve).scalar_value[0]=tw->select_value;
			for (i=0;i<3;i++)
			{
				(tw->edit_curve).point1[i]=tw->xival[i];
			}
		}
		else
		{
			/* curve completed */
			(tw->edit_curve).index=0;
			(tw->edit_curve).scalar_value[1]=tw->select_value;
			for (i=0;i<3;i++)
			{
				(tw->edit_curve).point2[i]=tw->xival[i];
			}
			/* store curve in volume_texture data structure */
			ALLOCATE(new_curve,struct VT_texture_curve,1);
			new_curve->type=1;
			for (i=0;i<3;i++)
			{
				new_curve->point1[i]=(tw->edit_curve).point1[i];
				new_curve->point2[i]=(tw->edit_curve).point2[i];
				new_curve->point3[i]=0;
				new_curve->point4[i]=0;
			}
			new_curve->scalar_value[0]=(tw->edit_curve).scalar_value[0];
			new_curve->scalar_value[1]=(tw->edit_curve).scalar_value[1];
			add_curve_to_list(tw->current_texture->texture_curve_list,new_curve);
/*??? debug */
printf("new curve : scalar values %lf,%lf\n",new_curve->scalar_value[0],
	new_curve->scalar_value[1]);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"select_line.  Invalid argument");
	}
	LEAVE;

} /* select_line */

void select_blob(struct Texture_window *tw)
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
Here the 1D blob texture structure is invoked - if no line is currently being
edited (edit_line.index = 0) the first point is selected, and a line is drawn to
the current node cursor position. If a line is being edited then the second
point is recorded, the curve is complete and stored in the volume_texture data
structure. edit_curve.index is reset to zero.
==============================================================================*/
{
	int i;
	struct VT_texture_curve *new_curve;

	ENTER(select_blob);
	/* checking argument */
	if (tw)
	{
		(tw->edit_curve).type=0;
		if (0==(tw->edit_curve).index)
		{
			/* new curve to edit */
			(tw->edit_curve).index=1;
			(tw->edit_curve).scalar_value[0]=tw->select_value;
			for (i=0;i<3;i++)
			{
				(tw->edit_curve).point1[i]=tw->xival[i];
			}
		}
		else
		{
			/* curve completed */
			(tw->edit_curve).index=0;
			(tw->edit_curve).scalar_value[1]=tw->select_value;
			for (i=0;i<3;i++)
			{
				(tw->edit_curve).point2[i]=tw->xival[i];
			}
			/* store curve in volume_texture data structure */
			ALLOCATE(new_curve,struct VT_texture_curve,1);
			new_curve->type=0;
			for (i=0;i<3;i++)
			{
				new_curve->point1[i]=(tw->edit_curve).point1[i];
				new_curve->point2[i]=(tw->edit_curve).point2[i];
				new_curve->point3[i]=0;
				new_curve->point4[i]=0;
			}
			new_curve->scalar_value[0]=(tw->edit_curve).scalar_value[0];
			new_curve->scalar_value[1]=(tw->edit_curve).scalar_value[1];
			add_curve_to_list(tw->current_texture->texture_curve_list,new_curve);
/*???debug */
printf("new blob : scalar values %lf,%lf\n",new_curve->scalar_value[0],
	new_curve->scalar_value[1]);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"select_blob.  Invalid argument");
	}
	LEAVE;
} /* select_blob */

void select_soft(struct Texture_window *tw)
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
Here the 1D soft texture structure is invoked - if no line is currently being
edited (edit_line.index = 0) the first point is selected, and a line is drawn to
the current node cursor position. If a line is being edited then the second
point is recorded, the curve is complete and stored in the volume_texture data
structure. edit_curve.index is reset to zero.
==============================================================================*/
{
	int i;
	struct VT_texture_curve *new_curve;

	ENTER(select_soft);
	/* checking argument */
	if (tw)
	{
		(tw->edit_curve).type=3;
		if (0==(tw->edit_curve).index)
		{
			/* new curve to edit */
			(tw->edit_curve).index=0;
			(tw->edit_curve).scalar_value[0]=tw->select_value;
			(tw->edit_curve).scalar_value[1]=tw->select_value2;
			for (i=0;i<3;i++)
			{
				(tw->edit_curve).point1[i]=tw->xival[i];
			}

			/* store curve in volume_texture data structure */
			ALLOCATE(new_curve,struct VT_texture_curve,1);
			new_curve->type=3;
			for (i=0;i<3;i++)
			{
				new_curve->point1[i]=(tw->edit_curve).point1[i];
				new_curve->point2[i]=0;
				new_curve->point3[i]=0;
				new_curve->point4[i]=0;
			}
			new_curve->scalar_value[0]=(tw->edit_curve).scalar_value[0];
			new_curve->scalar_value[1]=(tw->edit_curve).scalar_value[1];
			add_curve_to_list(tw->current_texture->texture_curve_list,new_curve);
/*???debug */
printf("new soft object: scalar value %lf\n",new_curve->scalar_value[0]);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"select_soft.  Invalid argument");
	}
	LEAVE;
} /* select_soft */

void deselect_curve(struct Texture_window *tw)
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
Two points are selected and the coordinates stored in edit_line.  After two
deselect operations have been performed, the list is searched for the line
segment and it is removed.
==============================================================================*/
{
	int i;
	struct VT_texture_curve *curve;

	ENTER(deselect_curve);
	/* checking arguments */
	if (tw)
	{
	     if (tw->soft_mode_on)
	     {
		curve=get_curve_from_list(tw->current_texture->texture_curve_list,
				&tw->edit_curve);
		if (curve)
			{
			remove_curve_from_list(tw->current_texture->texture_curve_list,curve);
			}
	     }
	     else
	     {
		
		if ((tw->edit_curve).index == 0)
		{
			/* new curve to edit */
			(tw->edit_curve).index=1;
			for (i=0;i<3;i++)
			{
				(tw->edit_curve).point1[i]=tw->xival[i];
			}
		}
		else
		{
			/* curve completed */
			(tw->edit_curve).index=0;
			for (i=0;i<3;i++)
			{
				(tw->edit_curve).point2[i]=tw->xival[i];
			}
			curve=get_curve_from_list(tw->current_texture->texture_curve_list,
				&tw->edit_curve);
			if (curve)
			{
			remove_curve_from_list(tw->current_texture->texture_curve_list,curve);
			}
		}
	    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"deselect_curve.  Invalid argument");
	}
	LEAVE;
} /* deselect_curve */

double line_segment_potential(double k,double r1,double r2,double L,double q1,
	double q2)
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
Calculates potential at a point from a line segment (length L) of charge (q1,q2)
(charge/unit length) with q1,q2 being distances r1,r2 from the point
respectively. (The charge/unit length varies linearly from q1 to q2)
==============================================================================*/
{
	double a,b,c,d,return_code;

	ENTER(line_segment_potential);
	/* default return value */
	return_code=0.0;
	a=q1;
	b=(q2-q1)/L;
	c=(r2*r2-r1*r1-L*L)/(2.0*L);
	d=sqrt(r1*r1 - c);
	if (0==d)
	{
/*???debug */
printf("d=0 : ND \n");
		return_code=1.0;
	}
	else
	{
		if ((0==r1)||(0==r2)||(0==L))
		{
			return_code=0.0;
		}
		else
		{
			return_code=(k*((a-c*b)*log((L+c+sqrt((L+c)*(L+c)+d*d))/d)+b*(sqrt((L+c)*
				(L+c)+d*d)-d)));
		}
	}
	LEAVE;

	return (return_code);
} /* line_segment_potential */

double line_segment_distance(double k,double *p1,double *p2,double *p,double q1,
	double q2)
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
Calculates distance potential at a point p from a line segment p1-p2 of charge
(q1,q2)
==============================================================================*/
{
	double a,a1,b[3],dist,r[3],return_code,v[3];
	int i;

	ENTER(line_segment_distance);
	/* default return value */
	return_code=0.0;
	/* checking arguments */
	if (p1&&p2&&p)
	{
		for (i=0;i<3;i++)
		{
			b[i]=p[i]-p1[i];
			v[i]=p2[i]-p1[i];
		}
		if (0!=(a1=dot_product(v,v)))
		{
			a=dot_product(b,v)/a1;
		}
		if ((a>=0.0)&&(a<=1.0))
		{
			/* point does not extend further than line segment */
			for(i=0;i<3;i++)
			{
				r[i]=b[i]-a*v[i];
			}
			if ((dist=vector_modulus(r)) != 0)
			{
				return_code=k/(dist*dist)*(q1+a*(q2-q1));
			}
			else
			{
				return_code=INFINITY;
			}
		}
		else
		{
			return_code=0.0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"line_segment_distance.  Invalid argument(s)");
		return_code=0.0;
	}
	LEAVE;

	return (return_code);
} /* line_segment_distance */

double blob_segment_distance(double k,double *p1,double *p2,double *p,double q1,
	double q2)
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
Calculates distance potential at a point p from a blob segment p1-p2 of charge
(q1,q2)
==============================================================================*/
{
	double b[3],dist,r1,r2;
	double return_code,v[3];
	int i;

	ENTER(blob_segment_distance);
	/* default return value */
	return_code=0.0;
	/* checking arguments */
	if (p1&&p2&&p)
	{
		for (i=0;i<3;i++)
		{
			b[i]=p[i]-p1[i];
			v[i]=p[i]-p2[i];
		}
		r1=vector_modulus(b);
		r2=vector_modulus(v);
		if (0==r1*r2)
		{
			return_code=INFINITY;
		}
		else
		{
			dist=(r1*r2)/(r1+r2);
			return_code=k/(dist*dist)*(r2/(r1+r2)*q1+r1/(r1+r2)*q2);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"blob_segment_distance.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* blob_segment_distance */

double soft_object_distance(double WR,double *p1,double *p,double q1)
/*******************************************************************************
LAST MODIFIED : 4 March 1997

DESCRIPTION :
==============================================================================*/
{
	double b[3],dist,r1,r2, w1;
	double wr2, wr4, wr6, WR2, WR4, WR6;
	double return_code,v[3];
	int i;

	ENTER(soft_object_distance);
	WR2 = WR*WR;
	WR4 = WR2*WR2;
	WR6 = WR2*WR4;
	
	/* default return value */
	return_code=0.0;
	/* checking arguments */
	if (p1&&p)
	{
		for (i=0;i<3;i++)
		{
			b[i]=p[i]-p1[i];
		}
		r1=vector_modulus(b);
		/* wyvill function on distance */
		if (r1 > WR)
		{
			return_code=0;
		}
		else
		{
			wr2=  r1*r1;
			wr4 = wr2*wr2;
			wr6 = wr2*wr4;
			
			w1 = q1*(1-4/9*wr6/WR6+17/9*wr4/WR4-22/9*wr2/WR2);
			return_code=w1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"blob_segment_distance.  Invalid argument(s)");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* soft_distance */

void select_curve(struct Texture_window *tw,int next)
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
If next = 0, this just updates the appropriate point, otherwise it stores it and
if index = 3 the line is complete and it is stored in the list. The points are
p1,p2 and the slope control points p3,p4.
==============================================================================*/
{
	int i;
	struct VT_texture_curve *new_curve;

	ENTER(select_curve);
	/* checking arguments */
	if (tw)
	{
		tw->edit_curve.type=2;
		switch ((tw->edit_curve).index)
		{
			case 0:
			{
				/* new curve to edit */
				/* first point */
				if (next)
				{
					(tw->edit_curve).index=1;
				}
				(tw->edit_curve).scalar_value[0]=tw->select_value;
				for (i=0;i<3;i++)
				{
					(tw->edit_curve).point1[i]=tw->xival[i];
					/* put in temporary values for bezier points */
					(tw->edit_curve).point3[i]=tw->xival[i];
				}
			} break;
			case 1:
			{
				/* second point */
				if (next)
				{
					(tw->edit_curve).index=2;
				}
				(tw->edit_curve).scalar_value[1]=tw->select_value;
				for (i=0;i<3;i++)
				{
					(tw->edit_curve).point2[i]=tw->xival[i];
					/* put in temporary values for bezier points */
					(tw->edit_curve).point4[i]=tw->xival[i];
				}
			} break;
			case 2:
			{
				/* bezier slope adjuster for first point */
				if (next)
				{
					(tw->edit_curve).index=3;
				}
				for (i=0;i<3;i++)
				{
					(tw->edit_curve).point3[i]=tw->xival[i];
				}
			} break;
			case 3:
			{
				/* curve completed */
				/* bezier slope adjuster for second point */
				for (i=0;i<3;i++)
				{
					(tw->edit_curve).point4[i]=tw->xival[i];
				}
				if (next)
				{
					(tw->edit_curve).index=0;
					/* store curve in volume_texture data structure */
					ALLOCATE(new_curve,struct VT_texture_curve,1);
					new_curve->type=2;
					for (i=0;i<3;i++)
					{
						new_curve->point1[i]=(tw->edit_curve).point1[i];
						new_curve->point2[i]=(tw->edit_curve).point2[i];
						new_curve->point3[i]=(tw->edit_curve).point3[i];
						new_curve->point4[i]=(tw->edit_curve).point4[i];
					}
					new_curve->scalar_value[0]=(tw->edit_curve).scalar_value[0];
					new_curve->scalar_value[1]=(tw->edit_curve).scalar_value[1];
					add_curve_to_list(tw->current_texture->texture_curve_list,new_curve);
/*???debug */
printf("new curve : scalar values %lf,%lf\n",new_curve->
						scalar_value[0],new_curve->scalar_value[1]);
				}
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"select_curve.  Invalid argument");
	}
	LEAVE;
} /* select_curve */

double curve_segment_distance(double k,double *p1,double *p2,double *p3,
	double *p4,double *p,double q1,double q2)
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
Calculates distance potential at a point p from a curve segment defined by
p1-p2, slope p3,p4 with charge(q1,q2) the distance Q(t)-p is minimized by
translating to the origin and solving the minimum sum Q(t)*Q(t) =>
0 = sum Q'(t).Q(t)
==============================================================================*/
{
	double at,bt,ct,dist,dt,min_dist,min_t,pt1[3],pt2[3],pt3[3],pt4[3],t;
	/* intermediate calculations */
	double a[3],b[3],c[3],d[3],e[3],f[3],g[3],r[3];
	/* coefficients of Q'(t).Q(t) */
	double return_code;
	fcomplex coeff[6];
	/* roots */
	fcomplex roots[6];
	int real_roots[6];
	int i,j,min,n_real_roots;
	static double val;
	/* the last solution and the next best guess */
	static double x;

	ENTER(curve_segment_distance);
	/* default return value */
	return_code=0.0;
	/* checking arguments */
	if (p1&&p2&&p3&&p4&&p)
	{
		for (i=0;i<6;i++)
		{
			coeff[i].r=0;
			coeff[i].i=0;
			roots[i].r=0;
			roots[i].i=0;
		}
		for (i=0;i<3;i++)
		{
			/* translate to origin */
			pt1[i]=p1[i]-p[i];
			pt2[i]=p2[i]-p[i];
			pt3[i]=p3[i]-p[i] ;
			pt4[i]=p4[i]-p[i];
			/* Q(t) */
			a[i]=-pt1[i]+3*pt2[i]-3*pt3[i]+pt4[i];
			b[i]=3*pt1[i]-6*pt2[i]+3*pt3[i];
			c[i]=-3*pt1[i]+3*pt2[i];
			d[i]=pt1[i];
			/* Q'(t) */
			e[i]=-3*pt1[i]+9*pt2[i]-9*pt3[i]+3*pt4[i];
			f[i]=6*pt1[i]-12*pt2[i]+6*pt3[i];
			g[i]=-3*pt1[i]+3*pt2[i];
		}
		/* calculate coefficients of minimized polynomial */
		for (i=0;i<3;i++)
		{
			coeff[0].r += (float)d[i]*g[i];
			coeff[1].r += (float)c[i]*g[i]+d[i]*f[i];
			coeff[2].r += (float)b[i]*g[i]+c[i]*f[i]+d[i]*e[i];
			coeff[3].r += (float)a[i]*g[i]+b[i]*f[i]+c[i]*e[i];
			coeff[4].r += (float)a[i]*f[i]+b[i]*e[i];
			coeff[5].r += (float)a[i]*e[i];
		}
		/* find roots of polynomial */
		/* the real root is the t parameter value of the curve pt nearest p */
		zroots(coeff,5,roots,1);
		min=1;
		n_real_roots=0;
		/* choose closest to real root if no real root exists*/
		for (i=1;i<6;i++)
		{
			/* printf"Root[%d] = (%lf + %lfi)\n",i,roots[i].r,roots[i].i);*/
			if ((roots[i].i*roots[i].i<roots[min].i*roots[min].i)||(roots[i].i == 0))
			{
				if (roots[i].i == 0)
			{
					real_roots[n_real_roots]=i;
					n_real_roots++;
				}
				min=i;
			}
		}
		if (n_real_roots == 0)
		{
			real_roots[n_real_roots]=min;
			n_real_roots=1;
		}
		/* we have our best selection of real roots - now must find */
		/* which one is the minimum */
		min_dist=0.;
		for (i=0;i<n_real_roots;i++)
		{
			t=(double)roots[real_roots[i]].r;
			at=(1-t)*(1-t)*(1-t);
			bt=3.0*t*(1-t)*(1-t);
			ct=3.0*t*t*(1-t);
			dt=t*t*t;
			for (j=0;j<3;j++)
			{
				r[j]=at*pt1[j]+bt*pt2[j]+ct*pt3[j]+dt*pt4[j];
			}
			dist=vector_modulus(r);
			if (dist<min_dist||i==0)
			{
				min_dist=dist;
				min_t=t;
			}
		}
		if ((min_t>=0)&&(min_t<=1.0))
		{
			if (0!=min_dist)
			{
				val=k/(min_dist*min_dist)*(q1+min_t*(q2-q1));
				return_code=val;
			}
			else
			{
				return_code=INFINITY;
			}
		}
		else
		{
			return_code=0.0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"curve_segment_distance.  Invalid argument(s)");
		return_code=0.0;
	}
	LEAVE;

	return (return_code);
} /* curve_segment_distance */
