/*******************************************************************************
FILE : splines.h

LAST MODIFIED : 29 August 1996

DESCRIPTION:
==============================================================================*/
#if !defined (SPLINES_H)
#define SPLINES_H

/*???DB.  Get rid of ? */
#define MAXPOINTS 10000
#define SIZE 100

/*
Global functions
----------------
*/
int interpolate_spline(int n_data,double *xpoints,double *knot,int n_steps,
	double *icurvex);
/*******************************************************************************
LAST MODIFIED : 29 August 1996

DESCRIPTION :
Interpolates parametrized data with a spline and produces n_step+1 points on the
resulting curve.
Input :
	n_data -    # of data
	xpoints -  1D array of data coords
	iknot - 1D array of corresponding parameters
	n_steps - discretization of generated curve
Output :
	icurvex - the interpolated curve. NOTE: icurve should have been assigned
		memory externally
==============================================================================*/
#endif
