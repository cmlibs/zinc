/*******************************************************************************
FILE : splines.c

LAST MODIFIED : 26 November 2001

DESCRIPTION:
==============================================================================*/
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include "general/debug.h"
#include "graphics/splines.h"
#include "user_interface/message.h"

/*
Module functions
----------------
*/
static int set_up_system(double *knot,int l,double *alpha,double *beta,
	double *gamma)
/*******************************************************************************
LAST MODIFIED : 29 August 1996

DESCRIPTION :
Given the knot sequence the linear system for clamped end condition
B spline interpolation is set up.
Input: knot knot sequence (all knots are simple, but knot[0] & knot[l]
	have multiplicity three.
	points   points to be interpolated
	l        # intervals
output:alpha, beta, gamma 1D arrays that const. the elements of the
	interpolation matrix.
Note no data pts needed so far!
==============================================================================*/
{
	int i,l1,return_code;
	double delta_im2,delta_im1,delta_i,delta_ip1,sum;

	ENTER(set_up_system);
	/* default return code */
	return_code=0;
	/* checking arguments */
	if (knot&&alpha&&beta&&gamma)
	{
		l1=l-1;
		/* some special cases */
		if (l==1)
		{
			alpha[0]=0.0;
			alpha[1]=0.0;
			beta[0]=1.0;
			beta[1]=1.0;
			gamma[0]=0.0;
			gamma[1]=0.0;
			return_code=1;
		}
		else
		{
			if (l==2)
			{
				beta[0]=1.0;
				alpha[0]=0.0;
				gamma[0]=0.0;
				delta_im1=(knot[1]-knot[0]);
				delta_i=(knot[2]-knot[1]);
				delta_ip1=(knot[3]-knot[2]);
				sum=delta_im1+delta_i;
				alpha[1]=delta_i*delta_i/sum;
				beta[1]=(delta_i*delta_im1)/sum+delta_im1*delta_i/sum;
				gamma[1]=delta_im1*delta_im1/sum;
				alpha[1]=alpha[1]/sum;
				beta[1] =beta[1]/sum;
				gamma[1]=gamma[1]/sum;
				beta[2]=1.0;
				alpha[2]=0.0;
				gamma[2]=0.0;
				return_code=1;
			}
			else
			{
				/* the rest does cases l > 2 */
				delta_im1=(knot[1]-knot[0]);
				delta_i=(knot[2]-knot[1]);
				delta_ip1=(knot[3]-knot[2]);
				sum=delta_im1+delta_i;
				beta[0]=1.0;
				/* 1st row of matrix */
				gamma[0]=0.0;
				alpha[1]=delta_i*delta_i/sum;
				beta[1]=(delta_i*delta_im1)/sum+delta_im1*(delta_i+delta_ip1)/
					(sum+delta_ip1);
				gamma[1]=delta_im1*delta_im1/(sum+delta_ip1);
				alpha[1]=alpha[1]/sum;
				beta[1]=beta[1]/sum;
				gamma[1]=gamma[1]/sum;
				/* now for the main loop */
				for (i=2;i<l1;i++)
				{
					delta_im2=(knot[i-1]-knot[i-2]); /* delta_i_minus_2 */
					delta_im1=(knot[i]-knot[i-1]); /* delta_i_minus_1 */
					delta_i=(knot[i+1]-knot[i]);  /* delta_i */
					delta_ip1=(knot[i+2]-knot[i+1]); /* delta_i_plus_i */
					sum=delta_im1+delta_i;
					alpha[i] = delta_i*delta_i/(delta_im2 + sum);
					beta[i]=delta_i*(delta_im2+delta_im1)/(delta_im2+sum)
						+delta_im1*(delta_i+delta_ip1)/(sum+delta_ip1);
					gamma[i]=delta_im1*delta_im1/(sum+delta_ip1);
					alpha[i]=alpha[i]/sum; /* this creates ith row of matrix */
					beta[i]=beta[i]/sum;
					gamma[i]=gamma[i]/sum;
				}
				/* special care at end */
				delta_im2=knot[l-2]-knot[l-3];
				delta_im1=knot[l1]-knot[l-2];
				delta_i=knot[l]-knot[l1];
				sum=delta_im1+delta_i;
				alpha[l1]=delta_i*delta_i/(delta_im2+sum);
				beta[l1]=delta_i*(delta_im2+delta_im1)/(delta_im2+sum)
					+delta_im1*delta_i/sum;
				gamma[l1]=delta_im1*delta_im1/sum;
				alpha[l1]=alpha[l1]/sum;
				beta[l1]=beta[l1]/sum;
				gamma[l1]=gamma[l1]/sum;
				alpha[l]=0.0;
				beta[l]=1.0;
				gamma[l]=0.0; /* last row */
				return_code=1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_up_system.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_up_system */

static int l_u_system(double *alpha,double *beta,double *gamma,int l,double *up,
	double *low)
/*******************************************************************************
LAST MODIFIED : 29 August 1996

DESCRIPTION :
perform LU decomposition of tridiagonal system with lower diagonal alpha
diag btea, upper diag gamma
Input: alpha,beta,gamma: coeffeicient matrix entries
	l matrix size [0,l]x[0,l]
Output: low L matrix entries
	up U matrix entries
==============================================================================*/
{
	int i,return_code;

	ENTER(l_u_system);
	/* checking arbuments */
	if (alpha&&beta&&gamma&&up&&low)
	{
		up[0]=beta[0];
		for (i=1;i<=l;i++)
		{
			low[i]=alpha[i]/up[i-1];
			up[i]=beta[i]-low[i]*gamma[i-1];
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"l_u_system.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* l_u_system */

static int solve_system(double *up,double *low,double *gamma,int l,double *rhs,
	double *d)
/*******************************************************************************
LAST MODIFIED : 29 August 1996

DESCRIPTION :
solve tridiagonal linear system of size (l+1)(l+1) whose LU decomposition
has entries up and low and whose right hand side is rhs and whose original
matrix had gamma as its upper diagonal. Solution is d[0],....,d[l+2].
(ctlpts)
Input : up,low,gamma: as above
	l size of system: l+1 eqns in l+1 unknowns
	rhs     right hand side ie data pts with end 'tangent' bezier
		in rhs[1] & rhs [l+1]
Output: d       solution vector
Note shift in indexing from text both rhs & d are from 0 -> l+2
==============================================================================*/
{
	double aux[MAXPOINTS];
	int i,return_code;

	ENTER(solve_system);
	/* checking arguments */
	if (up&&low&&gamma&&rhs&&d)
	{
		d[0]=rhs[0]; /* 1st data pt = 1st B spline vertex */
		d[1]=rhs[1]; /* second data pt = second Bspline vertex */
		/* forward substitution */
		aux[0]=rhs[1];
		for (i=1;i<=l;i++)
		{
			aux[i]=rhs[i+1]-low[i]*aux[i-1];
		}
		/* backward substitution */
			d[l+1]=aux[l]/up[l];
		for (i=l-1;i>0;i--)
		{
			d[i+1]=(aux[i]-gamma[i]*d[i+2])/up[i];
		}
		/* last 2 data pts = last two B spline vertices */
		d[l+2]=rhs[l+2];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"solve_system. Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* solve_system */

static int bessel_ends(double *data,double *knot,int l)
/*******************************************************************************
LAST MODIFIED : 29 August 1996

DESCRIPTION :
	Computes B-spline points data[1] and data[l+]
	according to Bessel end condition.

input:  data:  sequence of data coordinates data[0] to data[l+2].
		Note that data[1] and data[l+1] are expected to
		be empty, as they will be filled by this routine.
	knot:  knot sequence
	l:  number of intervals

output:  data:  completed, as above.
==============================================================================*/
{
	double alpha,beta;
	int return_code;

	ENTER(bessel_ends);
	/* checking arguments */
	if (data&&knot)
	{
		if (l==1)
		{
			/*  This is not really Bessel, but then what do you do
					when you have only one interval? -- make it linear!
			*/
			data[1]=(2.0*data[0]+data[3])/3.0;
			data[2]=(2.0*data[3]+data[0])/3.0;
		}
		else
		{
			if (l==2)
			{
				/* beginning */
				alpha=(knot[2]-knot[1])/(knot[2]-knot[0]);
				beta=1.0-alpha;
				data[1]=(data[2]-alpha*alpha*data[0]-beta*beta*data[4])
					/(2.0*alpha*beta);
				data[1]=2.0*(alpha*data[0]+beta*data[1])/3.0+data[0]/3.0;
				/* end */
				alpha=(knot[2]-knot[1])/(knot[2]-knot[0]);
				beta=1.0-alpha;
				data[3]=(data[2]-alpha*alpha*data[0]-beta*beta*data[4])/
					(2.0*alpha*beta);
				data[3]=2.0*(alpha*data[3]+beta*data[4])/3.0+data[4]/3.0;
			}
			else
			{
				/* beginning */
				alpha=(knot[2]-knot[1])/(knot[2]-knot[0]);
				beta=1.0-alpha;
				data[1]=(data[2]-alpha*alpha*data[0]-beta*beta*data[3])
					/(2.0*alpha*beta);
				data[1]=2.0*(alpha*data[0]+beta*data[1])/3.0+data[0]/3.0;
				/* end */
				alpha=(knot[l]-knot[l-1])/(knot[l]-knot[l-2]);
				beta=1.0-alpha;
				data[l+1]=(data[l]-alpha*alpha*data[l-1]-beta*beta*data[l+2])/
					(2.0*alpha*beta);
				data[l+1]=2.0*(alpha*data[l+1]+beta*data[l+2])/3.0+data[l+2]/3.0;
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"bessel_ends.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* bessel_ends */

#if defined (OLD_CODE)
static int parameters(double *data_x,double *data_y,int l,double *knot)
/*******************************************************************************
LAST MODIFIED : 29 August 1996

DESCRIPTION :
Finds a centripetal parametrization for a given set of 2D data pts
Input: data_x,data_y    input pts, numbered from 0 to l+2
			l                # intervals
Output: knot            knot sequence. Note not (knot[l] = 1.0) !
Note data_x[1],data_x[l+1] are not used - same for data_y
???DB.  Not used.
==============================================================================*/
{
	double delta;
	int i,l2,return_code;

	ENTER(parameters);
	/* checking arguments */
	if (data_x&&data_y&&knot)
	{
		/* in the following , special care must be taken at the ends because of the
			data structure used.  See note above */
		l2=l+2;
		/* initialize - arbitrary */
		knot[0]=0.0;
		delta=sqrt((data_x[2]-data_x[0])*(data_x[2]-data_x[0])
			+(data_y[2]-data_y[0])*(data_y[2]-data_y[0]));
		/* leave out this sqrt if you want chord length */
#if defined (OLD_CODE)
		knot[1]=sqrt(delta);
#endif /* defined (OLD_CODE) */
		knot[1]=delta;
		for (i=2;i<l;i++)
		{
			delta=sqrt((data_x[i+1]-data_x[i])*(data_x[i+1]-data_x[i])
				+(data_y[i+1]-data_y[i])*(data_y[i+1]-data_y[i]));
			/* leave out this sqrt if you want chord length */
#if defined (OLD_CODE)
			knot[i]=knot[i-1]+sqrt(delta);
#endif /* defined (OLD_CODE) */
			knot[i]=knot[i-1]+delta;
		}
		delta=sqrt((data_x[l2]-data_x[l])*(data_x[l2]-data_x[l])
			+(data_y[l2]-data_y[l])*(data_y[l2]-data_y[l]));
		/* leave out this sqrt if you want chord length */
#if defined (OLD_CODE)
		knot[l]=knot[l-1]+sqrt(delta);
#endif /* defined (OLD_CODE) */
		knot[l]=knot[l-1]+delta;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* parameters */
#endif /* defined (OLD_CODE) */

static double deboor(int degree,double *coeff,int n_coeff,int coord,
	double *knot,int n_knot,double u,int i)
/*******************************************************************************
LAST MODIFIED : 29 August 1996

DESCRIPTION :
Farin 10.10
uses deboor alg to compute one coord. on B-spline curve for
param value u in interval i
Input : degree  polyn. degree of each piece of curve
	coeff   B-spline control points
	knot    knot sequence
	u       evaluation abcissa
	i       u's interval u[i]<= u < u[i+1]
Output  coord value
==============================================================================*/
{
	double *coeffa,return_code,t1,t2;
	int j,k;

	ENTER(deboor);
	USE_PARAMETER(n_knot);
	if (coeff&&knot)
	{
		if (ALLOCATE(coeffa,double,n_coeff+1))
		{
			/* save data */
			for (j=i-degree+1;j<=i+1;j++)
			{
				coeffa[j]=coeff[coord+3*j];
			}
			for (k=1;k<=degree;k++)
			{
				for (j=i+1;j>=i-degree+k+1;j--)
				{
					t1=(knot[j+degree-k]-u)/(knot[j+degree-k]-knot[j-1]);
					t2=1.0-t1;
					coeffa[j]=t1*coeffa[j-1]+t2*coeffa[j];
				}
			}
			DEALLOCATE(coeffa);
			return_code=(coeffa[i+1]);
		}
		else
		{
			display_message(ERROR_MESSAGE,"deboor.  Could not allocate memory");
			return_code=0.0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"deboor.  Invalid argument(s)");
		return_code=0.0;
	}
	LEAVE;

	return (return_code);
} /* deboor */

static int bspl_to_points(int degree,int l,double *coeff,int n_coeff,int coord,
	double *knot,int n_knot,int dense,double *points,int *point_num)
/*******************************************************************************
LAST MODIFIED : 29 August 1996

DESCRIPTION :
Farin 10.10
generates points on B-spline curve (one coordinate)
Input   degree  polyn. degree of each piece of curve
	l       # of active intervals
	coeff   B-spline control pts
	knot    knot sequence knot[0] ...knot[l + 2*degree -2]
	dense   how many pts/segment
Output  points  output array with fn values
	point_num
		how many pts are generated.
		No pts are generated between multiple knots
================================++++++++++++++++==============================*/
{
	double u,ustep;
	int i,return_code;

	ENTER(bspl_to_points);
	/* checking arguments */
	if (coeff&&knot&&points&&point_num)
	{
		*point_num=0;
#if defined (INTERPOLATE_EACH_SEGMENT)
		for (i=degree-1;i<l+degree-1;i++)
		{
			if (knot[i+1]>knot[i])  /* skip zero length intervals */
			{
				for (ii=0;ii<=dense;ii++)
				{
					u=knot[i]+ii*(knot[i+1]-knot[i])/dense;
/*??? debug */
printf("calling deboor i = %d\n",i);
					points[*point_num]=deboor(degree,coeff,n_coeff,coord,
						knot,n_knot,u,i);
					*point_num=(*point_num)+1;
				}
			}
		}
#else
		/* MODIFICATION : 1/10/93 calculate only 'dense' # of points,
			skipping segments if necessary */
		ustep=(knot[l+degree-1]-knot[degree-1])/(double)dense;
		for (u=knot[degree-1],i=degree-1;u<knot[l+degree];u+=ustep)
		{
			if (u > knot[i+1])
			{
				while (u > knot[i+1])
				{
					i++;
				}
			}
			if (knot[i+1]>knot[i])  /* skip zero length intervals */
			{
				points[*point_num]=deboor(degree,coeff,n_coeff,coord,
					knot,n_knot,u,i);
				*point_num=(*point_num)+1;
			}
		}
		/*add last point */
		u=knot[l+degree-1];
		points[*point_num]=deboor(degree,coeff,n_coeff,coord,
			knot,n_knot,u,l+degree-2);
		*point_num=(*point_num)+1;
#endif /* defined (INTERPOLATE_EACH_SEGMENT) */
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"bspl_to_points.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* bspl_to_points */

/*
Global functions
----------------
*/
int interpolate_spline(int n_data,double *xpoints,double *knot,int n_steps,
	double *icurvex)
/*******************************************************************************
LAST MODIFIED : 14 March 1994

DESCRIPTION :
Interpolates parametrized data with a spline and produces n_step+1 points on the
resulting curve.
Input :
	n_data - # of data
	xpoints - 1D array of data coords
	iknot - 1D array of corresponding parameters
	n_steps - discretization of generated curve
Output :
	icurvex - the interpolated curve. NOTE: icurve should have been assigned
		memory externally
==============================================================================*/
{
	double *ialpha,*ibeta,*ibspl_x,*icp,*idata_x,*igamma,*iknot,*ilow,*iup;
	int degree=3,i,j,l,point_num,return_code;

	ENTER(interpolate_spline);
	/* checking arguments */
	if (xpoints&&knot&&icurvex)
	{
		idata_x=(double *)NULL;
		iknot=(double *)NULL;
		ialpha=(double *)NULL;
		ibeta=(double *)NULL;
		igamma=(double *)NULL;
		ilow=(double *)NULL;
		iup=(double *)NULL;
		icp=(double *)NULL;
		ibspl_x=(double *)NULL;
		if (ALLOCATE(idata_x,double,n_data+2)&&
			ALLOCATE(iknot,double,n_data+4)&&
			ALLOCATE(ialpha,double,n_data+2)&&
			ALLOCATE(ibeta,double,n_data+2)&&
			ALLOCATE(igamma,double,n_data+2)&&
			ALLOCATE(ilow,double,n_data+2)&&
			ALLOCATE(iup,double,n_data+2)&&
			ALLOCATE(icp,double,4*(n_data+2))&&
			ALLOCATE(ibspl_x,double,n_data+2))
		{
			/* copy points into array which will have 2nd & 2nd to last points
				calculated to give bessel end conditions */
			l=n_data-1;
			for (i=0;i<n_data;i++)
			{
				iknot[i]=knot[i];
			}
			for (i=0,j=0;i<=l+2;i++)
			{
				if ((i!=1)&&(i!=l+1))
				{
					idata_x[i]=xpoints[j];
					j++;
				}
			}
#if defined (OLD_CODE)
			/* for linear ends */
			idata_x[1]=(idata_x[0]+idata_x[2])/2.0;
			idata_x[l+1]=(idata_x[l]+idata_x[l+2])/2.0;
#endif /* defined (OLD_CODE) */
			bessel_ends(idata_x,iknot,l);
			set_up_system(iknot,l,ialpha,ibeta,igamma);
			l_u_system(ialpha,ibeta,igamma,l,iup,ilow);
			solve_system(iup,ilow,igamma,l,idata_x,ibspl_x);
			/* put control polygon (ibspl) into form for drawing (icp) */
			for (i=0;i<=l+2;i++)
			{
				icp[3*i]=ibspl_x[i];
			}
			/* add in extra multiplicity */
			for (i=l;i>=0;i--)
			{
				iknot[i+2]=iknot[i];
			}
			iknot[l+4]=iknot[l+3]=iknot[l+2];
			iknot[0]=iknot[1]=iknot[2];
			bspl_to_points(degree,l,icp,l+2,0,iknot,l+2*degree-2,n_steps,icurvex,
				&point_num);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"interpolate_spline.  Insufficient memory");
			return_code=0;
		}
		/* deallocate storage */
		DEALLOCATE(idata_x);
		DEALLOCATE(iknot);
		DEALLOCATE(ialpha);
		DEALLOCATE(ibeta);
		DEALLOCATE(igamma);
		DEALLOCATE(ilow);
		DEALLOCATE(iup);
		DEALLOCATE(icp);
		DEALLOCATE(ibspl_x);
	}
	else
	{
		display_message(ERROR_MESSAGE,"interpolate_spline.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* interpolate_spline */
