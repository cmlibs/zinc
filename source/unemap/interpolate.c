/*******************************************************************************
FILE : interpolate.c

LAST MODIFIED : 26 June 2003

DESCRIPTION :
Functions for calculating a finite element interpolation to data for a special
case.

The interpolation is assumed to be on a rectangular region which is divided
into rows and columns to form the finite element mesh.  The bottom left corner
of the region is at (x0,y0).  Numbering from the bottom left corner the column
widths are w_j, j=1,M and the row heights are h_i, i=1,N.  There are two sub-
cases.  For a PATCH, no nodes are identified so that there are (N+1)*(M+1)
nodes (4*(N+1)*(M+1) degrees of freedom).  For a SOCK, the left side is
identified with the right side and the nodes on the bottom edge are compressed
to one node so that there are N*M+1 nodes (4*N*M+3 degrees of freedom, because
the global mixed partial, d2fdxdy, is zero at the apex).

Bicubic Hermite basis functions are used.  For the (i,j) element (i=1,N, j=1,M)
the data is interpolated by
F_ij(u,v) =
  H01(u) * H01(v) * f_ij_1       + H02(u) * H01(v) * f_ij_2
+ H01(u) * H02(v) * f_ij_3       + H02(u) * H02(v) * f_ij_4
+ H11(u) * H01(v) * dfdu_ij_1    + H12(u) * H01(v) * dfdu_ij_2
+ H11(u) * H02(v) * dfdu_ij_3    + H12(u) * H02(v) * dfdu_ij_4
+ H01(u) * H11(v) * dfdv_ij_1    + H02(u) * H11(v) * dfdv_ij_3
+ H01(u) * H12(v) * dfdv_ij_3    + H02(u) * H12(v) * dfdv_ij_4
+ H11(u) * H11(v) * d2fdudv_ij_1 + H12(u) * H11(v) * d2fdudv_ij_2
+ H11(u) * H12(v) * d2fdudv_ij_3 + H12(u) * H12(v) * d2fdudv_ij_4
where u and v are the local element coordinates and the one-dimensional cubic
Hermite basis functions are
  H01(w) = 1 - 3*w*w + 2*w*w*w
	H11(w) = w*(w-1)*(w-1)
  H02(w) = w*w*(3-2*w)
  H12(w) = w*w*(w-1)
Because the elements are not the same size, the element derivatives are not the
same for adjacent elements at the same node.  Changing to nodal values and
derivatives
F_i_j(u,v) =
	H01(u)*H01(v)*f_i-1_j-1               + H02(u)*H01(v)*f_i-1_j
+ H01(u)*H02(v)*f_i_j-1                 + H02(u)*H02(v)*f_i_j
+ H11(u)*H01(v)*dfdx_i-1_j-1*w_j        + H12(u)*H01(v)*dfdx_i-1_j*w_j
+ H11(u)*H02(v)*dfdx_i_j-1*w_j          + H12(u)*H02(v)*dfdx_i_j*w_j
+ H01(u)*H11(v)*dfdy_i-1_j-1*h_i        + H02(u)*H11(v)*dfdy_i-1_j*h_i
+ H01(u)*H12(v)*dfdy_i_j-1*h_i          + H02(u)*H12(v)*dfdy_i_j*h_i
+ H11(u)*H11(v)*d2fdxdy_i-1_j-1*h_i*w_j + H12(u)*H11(v)*d2fdxdy_i-1_j*h_i*w_j
+ H11(u)*H12(v)*d2fdxdy_i_j-1*h_i*w_j   + H12(u)*H12(v)*d2fdxdy_i_j*h_i*w_j

Let the data be (x_d,y_d,f_d), d=1,D with weights w_d.  The nodal values and
derivatives, f_i_j, dfdx_i_j, dfdy_i_j and d2fdxdy_i_j, are chosen to minimize
an error function made up of the sum of squares residual

	sum[d=1,D]{w_d * square{sum[i=1,N;j=1,M]{F_i_j(u_d,v_d)} - f_d}}

plus a term representing membrane deflection energy

  alpha*integral[0<=u<=1;0<=v<=1]{square{dfdu}+square{dfdv}}

plus a term representing plate bending energy

  beta*integral[0<=u<=1;0<=v<=1]{square{d2fdu2}+square{d2fdudv}+square{d2fdv2}}

The last two terms (membrane and plate bending) are included in order to smooth
the fitted function (penalize oscillation).  The minimum is found by
differentiating the residual with respect to the nodal values and derivatives
and setting the resulting linear equations equal to 0.

For example, differentiating the sum of squares residual with respect to the
nodal value f_k_l gives
	sum[d=1,D]{w_d*d[F_k_l]d[f_k_l](u_d,v_d)*sum[i=1,N;j=1,M]{F_i_j(u_d,v_d)}
+ sum[d=1,D]{w_d*d[F_k+1_l]d[f_k_l](u_d,v_d)*sum[i=1,N;j=1,M]{F_i_j(u_d,v_d)}
+ sum[d=1,D]{w_d*d[F_k+1_l+1]d[f_k_l](u_d,v_d)*sum[i=1,N;j=1,M]{F_i_j(u_d,v_d)}
+ sum[d=1,D]{w_d*d[F_k_l+1]d[f_k_l](u_d,v_d)*sum[i=1,N;j=1,M]{F_i_j(u_d,v_d)}
= sum[d=1,D]{w_d*d[F_k_l]d[f_k_l](u_d,v_d)*f_d}
+ sum[d=1,D]{w_d*d[F_k+1_l]d[f_k_l](u_d,v_d)*f_d}
+ sum[d=1,D]{w_d*d[F_k+1_l+1]d[f_k_l](u_d,v_d)*f_d}
+ sum[d=1,D]{w_d*d[F_k_l+1]d[f_k_l](u_d,v_d)*f_d}
==============================================================================*/
#include <stddef.h>
#include <math.h>
#include "general/debug.h"
#include "general/geometry.h"
#include "unemap/interpolate.h"
#include "unemap/rig.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*#define FIX_APEX*/
#define DOUBLE_MATRIX

/*
Module functions
----------------
*/
static int assemble_linear_equations(enum Region_type type,int number_of_data,
	float *x,float *y,float *value,float *weight,int number_of_rows,float *y_mesh,
	int number_of_columns,float *x_mesh,int number_of_equations,
#if defined (FIX_APEX)
#if defined (DOUBLE_MATRIX)
	double *cosine_apex,double *sine_apex,
#else
	float *cosine_apex,double *sine_apex,
#endif /* defined (DOUBLE_MATRIX) */
#endif /* defined (FIX_APEX) */
#if defined (DOUBLE_MATRIX)
	double
#else
	float
#endif /* defined (DOUBLE_MATRIX) */
	*matrix_and_right_hand_side,float membrane_smoothing,
	float plate_bending_smoothing)
/*******************************************************************************
LAST MODIFIED : 26 March 1997

DESCRIPTION :
The nodes are numbered from the bottom left corner.  So for a PATCH the
numbering is
  n*(m+1)  n*(m+1)+1  n*(m+1)+2  ...  (n+1)*(m+1)-1
  :        :          :               :
  m+1      m+2        m+3        ...  2*m+1
  0        1          2          ...  m
for a SOCK the numbering is
  (n-1)*m+1  (n-1)*m+2  (n-1)*m+3  ...  n*m  (n-1)*m+1
  :          :          :               :      :
  m+1        m+2        m+3        ...  2*m  m+1
  1          2          3          ...  m    1
  0          0          0          ...  0    0
and for a TORSO the numbering is
  n*m  n*m+1  n*m+2  ...  (n+1)*m-1  n*m
  :    :      :           :          :
  m    m+1    m+2    ...  2*m-1      m
  0    1      2      ...  m-1        0
where n=number_of_rows and m=number_of_columns.
==============================================================================*/
{
	int column,i,i_j,i_jm1,im1_j,im1_jm1,number_of_equations_plus_1,return_code,
		row;
#if defined (DOUBLE_MATRIX)
	double
#else
	float
#endif /* defined (DOUBLE_MATRIX) */
#if defined (FIX_APEX)
		angle,cos_0,cos_1,
		sin_0,sin_1,
#endif /* defined (FIX_APEX) */
		dfdx_i_j,dfdx_i_j_w,dfdx_i_jm1,dfdx_i_jm1_w,dfdx_im1_j,dfdx_im1_j_w,
		dfdx_im1_jm1,dfdx_im1_jm1_w,dfdy_i_j,dfdy_i_j_w,dfdy_i_jm1,dfdy_i_jm1_w,
		dfdy_im1_j,dfdy_im1_j_w,dfdy_im1_jm1,dfdy_im1_jm1_w,dh01dh01,dh01dh02,
		dh01dh11,dh01dh12,dh02dh01,dh02dh02,dh02dh11,dh02dh12,dh11dh01,dh11dh02,
		dh11dh11,dh11dh12,dh12dh01,dh12dh02,dh12dh11,dh12dh12,d2fdxdy_i_j,
		d2fdxdy_i_j_w,d2fdxdy_i_jm1,d2fdxdy_i_jm1_w,d2fdxdy_im1_j,d2fdxdy_im1_j_w,
		d2fdxdy_im1_jm1,d2fdxdy_im1_jm1_w,d2h01d2h01,d2h01d2h02,d2h01d2h11,
		d2h01d2h12,d2h02d2h01,d2h02d2h02,d2h02d2h11,d2h02d2h12,d2h11d2h01,
		d2h11d2h02,d2h11d2h11,d2h11d2h12,d2h12d2h01,d2h12d2h02,d2h12d2h11,
		d2h12d2h12,f_d,f_i_j,f_i_j_w,f_i_jm1,f_i_jm1_w,f_im1_j,f_im1_j_w,f_im1_jm1,
		f_im1_jm1_w,height,height_m1,height_m2,height_m3,height_1,height_2,height_3,
		h01h01,h01h02,h01h11,h01h12,h01_u,h01_v,h02h01,h02h02,h02h11,h02h12,h02_u,
		h02_v,h11h01,h11h02,h11h11,h11h12,h11_u,h11_v,h12_u,h12_v,h12h01,h12h02,
		h12h11,h12h12,*temp,u,v,weight_d,width,width_m1,width_m2,width_m3,width_1,
		width_2,width_3;

	ENTER(assemble_linear_equations);
	return_code=1;
	number_of_equations_plus_1=number_of_equations+1;
	/* initialize the matrix and the right hand side */
	temp=matrix_and_right_hand_side;
	for (i=number_of_equations*number_of_equations_plus_1;i>0;i--)
	{
		*temp=(float)0;
		temp++;
	}
#if defined (FIX_APEX)
	if (SOCK==type)
	{
		cosine_apex[0]=1;
		sine_apex[0]=0;
		angle=0;
		for (i=1;i<number_of_columns;i++)
		{
			angle=angle+x_mesh[i];
			cosine_apex[i]=cos(angle);
			sine_apex[i]=sin(angle);
		}
		cosine_apex[number_of_columns]=1;
		sine_apex[number_of_columns]=0;
	}
#endif /* defined (FIX_APEX) */
	/* calculate the membrane and plate bending smoothing */
	h01h01=(float)(13./35.);
	h01h02=h02h01=(float)(9./70.);
	h01h11=h11h01=(float)(11./210.);
	h01h12=h12h01=(float)(-13./420.);
	h02h02=(float)(13./35.);
	h02h11=h11h02=(float)(13./420.);
	h02h12=h12h02=(float)(-11./210.);
	h11h11=(float)(1./105.);
	h11h12=h12h11=(float)(-1./140.);
	h12h12=(float)(1./105.);
	dh01dh01=(float)(6./5.);
	dh01dh02=dh02dh01=(float)(-6./5.);
	dh01dh11=dh11dh01=(float)(1./10.);
	dh01dh12=dh12dh01=(float)(1./10.);
	dh02dh02=(float)(6./5.);
	dh02dh11=dh11dh02=(float)(-1./10.);
	dh02dh12=dh12dh02=(float)(-1./10.);
	dh11dh11=(float)(2./15.);
	dh11dh12=dh12dh11=(float)(-1./30.);
	dh12dh12=(float)(2./15.);
	d2h01d2h01=(float)12.;
	d2h01d2h02=d2h02d2h01=(float)-12.;
	d2h01d2h11=d2h11d2h01=(float)6.;
	d2h01d2h12=d2h12d2h01=(float)6.;
	d2h02d2h02=(float)12.;
	d2h02d2h11=d2h11d2h02=(float)-6.;
	d2h02d2h12=d2h12d2h02=(float)-6.;
	d2h11d2h11=(float)4.;
	d2h11d2h12=d2h12d2h11=(float)2.;
	d2h12d2h12=(float)4.;
	/* loop over the elements */
	for (row=1;row<=number_of_rows;row++)
	{
		for (column=1;column<=number_of_columns;column++)
		{
			width_1=
#if defined (DOUBLE_MATRIX)
				(double)
#endif
				(x_mesh[column]-x_mesh[column-1]);
			width_2=width_1*width_1;
			width_3=width_2*width_1;
			width_m1=(float)1./width_1;
			width_m2=width_m1*width_m1;
			width_m3=width_m2*width_m1;
			height_1=
#if defined (DOUBLE_MATRIX)
				(double)
#endif /* defined (DOUBLE_MATRIX) */
				(y_mesh[row]-y_mesh[row-1]);
			height_2=height_1*height_1;
			height_3=height_2*height_1;
			height_m1=(float)1./height_1;
			height_m2=height_m1*height_m1;
			height_m3=height_m2*height_m1;
			/* calculate node numbers
				NB the local coordinates have the bottom left corner of the element as
				the origin.  This means that
				(i-1,j-1) is the bottom left corner
				(i,j-1) is the top left corner
				(i,j) is the top right corner
				(i-1,j) is the bottom right corner */
			switch (type)
			{
				case PATCH:
				{
					/* (i-1,j-1) node (bottom left corner) */
					im1_jm1=4*((row-1)*(number_of_columns+1)+column-1);
					/* (i,j-1) node (top left corner) */
					i_jm1=4*(row*(number_of_columns+1)+column-1);
					/* (i,j) node (top right corner) */
					i_j=4*(row*(number_of_columns+1)+column);
					/* (i-1,j) node (bottom right corner) */
					im1_j=4*((row-1)*(number_of_columns+1)+column);
				} break;
				case SOCK:
				{
					if (row==1)
					{
						/* (i-1,j-1) node (bottom left corner) */
						im1_jm1=0;
#if defined (FIX_APEX)
						cos_0=cosine_apex[column-1];
						sin_0=sine_apex[column-1];
#endif /* defined (FIX_APEX) */
						/* (i,j-1) node (top left corner) */
#if defined (FIX_APEX)
						i_jm1=4*column-1;
#else
						i_jm1=4*column;
#endif /* defined (FIX_APEX) */
						/* (i,j) node (top right corner) */
						if (column==number_of_columns)
						{
#if defined (FIX_APEX)
							i_j=3;
#else
							i_j=4;
#endif /* defined (FIX_APEX) */
						}
						else
						{
#if defined (FIX_APEX)
							i_j=4*column+3;
#else
							i_j=4*(column+1);
#endif /* defined (FIX_APEX) */
						}
						/* (i-1,j) node (bottom right corner) */
						im1_j=0;
#if defined (FIX_APEX)
						cos_1=cosine_apex[column];
						sin_1=sine_apex[column];
#endif /* defined (FIX_APEX) */
					}
					else
					{
						/* (i-1,j-1) node (bottom left corner) */
#if defined (FIX_APEX)
						im1_jm1=4*((row-2)*number_of_columns+column)-1;
#else
						im1_jm1=4*((row-2)*number_of_columns+column);
#endif /* defined (FIX_APEX) */
						/* (i,j-1) node (top left corner) */
#if defined (FIX_APEX)
						i_jm1=4*((row-1)*number_of_columns+column)-1;
#else
						i_jm1=4*((row-1)*number_of_columns+column);
#endif /* defined (FIX_APEX) */
						/* (i,j) node (top right corner) */
						if (column==number_of_columns)
						{
#if defined (FIX_APEX)
							i_j=4*(row-1)*number_of_columns+3;
#else
							i_j=4*((row-1)*number_of_columns+1);
#endif /* defined (FIX_APEX) */
						}
						else
						{
#if defined (FIX_APEX)
							i_j=4*((row-1)*number_of_columns+column)+3;
#else
							i_j=4*((row-1)*number_of_columns+column+1);
#endif /* defined (FIX_APEX) */
						}
						/* (i-1,j) node (bottom right corner) */
						if (column==number_of_columns)
						{
#if defined (FIX_APEX)
							im1_j=4*(row-2)*number_of_columns+3;
#else
							im1_j=4*((row-2)*number_of_columns+1);
#endif /* defined (FIX_APEX) */
						}
						else
						{
#if defined (FIX_APEX)
							im1_j=4*((row-2)*number_of_columns+column)+3;
#else
							im1_j=4*((row-2)*number_of_columns+column+1);
#endif /* defined (FIX_APEX) */
						}
					}
				} break;
				case TORSO:
				{
					/* (i-1,j-1) node (bottom left corner) */
					im1_jm1=4*((row-1)*number_of_columns+column-1);
					/* (i,j-1) node (top left corner) */
					i_jm1=4*(row*number_of_columns+column-1);
					if (column==number_of_columns)
					{
						/* (i,j) node (top right corner) */
						i_j=4*row*number_of_columns;
						/* (i-1,j) node (bottom right corner) */
						im1_j=4*(row-1)*number_of_columns;
					}
					else
					{
						/* (i,j) node (top right corner) */
						i_j=4*(row*number_of_columns+column);
						/* (i-1,j) node (bottom right corner) */
						im1_j=4*((row-1)*number_of_columns+column);
					}
				} break;
			}
			/* update the matrix */
			/* start smoothing */
#if defined (FIX_APEX)
			if ((SOCK==type)&&(im1_jm1==0))
			{
				/* d(im1_jm1)/d(im1_jm1) */
				temp=matrix_and_right_hand_side;
				/* d/d(f) */
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh01dh01*h01h01*width_m1*height_1+
					h01h01*dh01dh01*width_1*height_m1)+
					plate_bending_smoothing*
					(d2h01d2h01*h01h01*width_m3*height_1+
					dh01dh01*dh01dh01*width_m1*height_m1+
					h01h01*d2h01d2h01*width_1*height_m3));
				/* dfdx */
				temp[1] += cos_0*
					(membrane_smoothing*
					(dh01dh01*h01h11*width_m1*height_2+
					h01h01*dh01dh11*width_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h01h11*width_m3*height_2+
					dh01dh01*dh01dh11*width_m1+
					h01h01*d2h01d2h11*width_1*height_m2));
				/* dfdy */
				temp[2] += sin_0*
					(membrane_smoothing*
					(dh01dh01*h01h11*width_m1*height_2+
					h01h01*dh01dh11*width_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h01h11*width_m3*height_2+
					dh01dh01*dh01dh11*width_m1+
					h01h01*d2h01d2h11*width_1*height_m2));
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += cos_0*
					(membrane_smoothing*
					(dh01dh01*h11h01*width_m1*height_2+
					h01h01*dh11dh01*width_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h11h01*width_m3*height_2+
					dh01dh01*dh11dh01*width_m1+
					h01h01*d2h11d2h01*width_1*height_m2));
				/* dfdx */
				temp[1] += cos_0*cos_0*
					(membrane_smoothing*
					(dh01dh01*h11h11*width_m1*height_3+
					h01h01*dh11dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h11h11*width_m3*height_3+
					dh01dh01*dh11dh11*width_m1*height_1+
					h01h01*d2h11d2h11*width_1*height_m1));
				/* dfdy */
				temp[2] += sin_0*cos_0*
					(membrane_smoothing*
					(dh01dh01*h11h11*width_m1*height_3+
					h01h01*dh11dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h11h11*width_m3*height_3+
					dh01dh01*dh11dh11*width_m1*height_1+
					h01h01*d2h11d2h11*width_1*height_m1));
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += sin_0*
					(membrane_smoothing*
					(dh01dh01*h11h01*width_m1*height_2+
					h01h01*dh11dh01*width_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h11h01*width_m3*height_2+
					dh01dh01*dh11dh01*width_m1+
					h01h01*d2h11d2h01*width_1*height_m2));
				/* dfdx */
				temp[1] += cos_0*sin_0*
					(membrane_smoothing*
					(dh01dh01*h11h11*width_m1*height_3+
					h01h01*dh11dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h11h11*width_m3*height_3+
					dh01dh01*dh11dh11*width_m1*height_1+
					h01h01*d2h11d2h11*width_1*height_m1));
				/* dfdy */
				temp[2] += sin_0*sin_0*
					(membrane_smoothing*
					(dh01dh01*h11h11*width_m1*height_3+
					h01h01*dh11dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h11h11*width_m3*height_3+
					dh01dh01*dh11dh11*width_m1*height_1+
					h01h01*d2h11d2h11*width_1*height_m1));
				/* d(im1_jm1)/d(im1_j) */
				temp=matrix_and_right_hand_side;
				/* d/d(f) */
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh02dh01*h01h01*width_m1*height_1+
					h02h01*dh01dh01*width_1*height_m1)+
					plate_bending_smoothing*
					(d2h02d2h01*h01h01*width_m3*height_1+
					dh02dh01*dh01dh01*width_m1*height_m1+
					h02h01*d2h01d2h01*width_1*height_m3));
				/* dfdx */
				temp[1] += cos_0*
					(membrane_smoothing*
					(dh02dh01*h01h11*width_m1*height_2+
					h02h01*dh01dh11*width_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h01h11*width_m3*height_2+
					dh02dh01*dh01dh11*width_m1+
					h02h01*d2h01d2h11*width_1*height_m2));
				/* dfdy */
				temp[2] += sin_0*
					(membrane_smoothing*
					(dh02dh01*h01h11*width_m1*height_2+
					h02h01*dh01dh11*width_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h01h11*width_m3*height_2+
					dh02dh01*dh01dh11*width_m1+
					h02h01*d2h01d2h11*width_1*height_m2));
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += cos_1*
					(membrane_smoothing*
					(dh02dh01*h11h01*width_m1*height_2+
					h02h01*dh11dh01*width_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h11h01*width_m3*height_2+
					dh02dh01*dh11dh01*width_m1+
					h02h01*d2h11d2h01*width_1*height_m2));
				/* dfdx */
				temp[1] += cos_0*cos_1*
					(membrane_smoothing*
					(dh02dh01*h11h11*width_m1*height_3+
					h02h01*dh11dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h11h11*width_m3*height_3+
					dh02dh01*dh11dh11*width_m1*height_1+
					h02h01*d2h11d2h11*width_1*height_m1));
				/* dfdy */
				temp[2] += sin_0*cos_1*
					(membrane_smoothing*
					(dh02dh01*h11h11*width_m1*height_3+
					h02h01*dh11dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h11h11*width_m3*height_3+
					dh02dh01*dh11dh11*width_m1*height_1+
					h02h01*d2h11d2h11*width_1*height_m1));
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += sin_1*
					(membrane_smoothing*
					(dh02dh01*h11h01*width_m1*height_2+
					h02h01*dh11dh01*width_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h11h01*width_m3*height_2+
					dh02dh01*dh11dh01*width_m1+
					h02h01*d2h11d2h01*width_1*height_m2));
				/* dfdx */
				temp[1] += cos_0*sin_1*
					(membrane_smoothing*
					(dh02dh01*h11h11*width_m1*height_3+
					h02h01*dh11dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h11h11*width_m3*height_3+
					dh02dh01*dh11dh11*width_m1*height_1+
					h02h01*d2h11d2h11*width_1*height_m1));
				/* dfdy */
				temp[2] += sin_0*sin_1*
					(membrane_smoothing*
					(dh02dh01*h11h11*width_m1*height_3+
					h02h01*dh11dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h11h11*width_m3*height_3+
					dh02dh01*dh11dh11*width_m1*height_1+
					h02h01*d2h11d2h11*width_1*height_m1));
				/* d(im1_jm1)/d(i_j) */
				temp=matrix_and_right_hand_side+
					i_j*number_of_equations_plus_1;
				/* d/d(f) */
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh02dh01*h02h01*width_m1*height_1+
					h02h01*dh02dh01*width_1*height_m1)+
					plate_bending_smoothing*
					(d2h02d2h01*h02h01*width_m3*height_1+
					dh02dh01*dh02dh01*width_m1*height_m1+
					h02h01*d2h02d2h01*width_1*height_m3));
				/* dfdx */
				temp[1] += cos_0*
					(membrane_smoothing*
					(dh02dh01*h02h11*width_m1*height_2+
					h02h01*dh02dh11*width_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h02h11*width_m3*height_2+
					dh02dh01*dh02dh11*width_m1+
					h02h01*d2h02d2h11*width_1*height_m2));
				/* dfdy */
				temp[2] += sin_0*
					(membrane_smoothing*
					(dh02dh01*h02h11*width_m1*height_2+
					h02h01*dh02dh11*width_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h02h11*width_m3*height_2+
					dh02dh01*dh02dh11*width_m1+
					h02h01*d2h02d2h11*width_1*height_m2));
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh12dh01*h02h01*height_1+
					h12h01*dh02dh01*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h12d2h01*h02h01*width_m2*height_1+
					dh12dh01*dh02dh01*height_m1+
					h12h01*d2h02d2h01*width_2*height_m3));
				/* dfdx */
				temp[1] += cos_0*
					(membrane_smoothing*
					(dh12dh01*h02h11*height_2+
					h12h01*dh02dh11*width_2)+
					plate_bending_smoothing*
					(d2h12d2h01*h02h11*width_m2*height_2+
					dh12dh01*dh02dh11+
					h12h01*d2h02d2h11*width_2*height_m2));
				/* dfdy */
				temp[2] += sin_0*
					(membrane_smoothing*
					(dh12dh01*h02h11*height_2+
					h12h01*dh02dh11*width_2)+
					plate_bending_smoothing*
					(d2h12d2h01*h02h11*width_m2*height_2+
					dh12dh01*dh02dh11+
					h12h01*d2h02d2h11*width_2*height_m2));
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh02dh01*h12h01*width_m1*height_2+
					h02h01*dh12dh01*width_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h12h01*width_m3*height_2+
					dh02dh01*dh12dh01*width_m1+
					h02h01*d2h12d2h01*width_1*height_m2));
				/* dfdx */
				temp[1] += cos_0*
					(membrane_smoothing*
					(dh02dh01*h12h11*width_m1*height_3+
					h02h01*dh12dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h12h11*width_m3*height_3+
					dh02dh01*dh12dh11*width_m1*height_1+
					h02h01*d2h12d2h11*width_1*height_m1));
				/* dfdy */
				temp[2] += sin_0*
					(membrane_smoothing*
					(dh02dh01*h12h11*width_m1*height_3+
					h02h01*dh12dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h12h11*width_m3*height_3+
					dh02dh01*dh12dh11*width_m1*height_1+
					h02h01*d2h12d2h11*width_1*height_m1));
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh12dh01*h12h01*height_2+
					h12h01*dh12dh01*width_2)+
					plate_bending_smoothing*
					(d2h12d2h01*h12h01*width_m2*height_2+
					dh12dh01*dh12dh01+
					h12h01*d2h12d2h01*width_2*height_m2));
				/* dfdx */
				temp[1] += cos_0*
					(membrane_smoothing*
					(dh12dh01*h12h11*height_3+
					h12h01*dh12dh11*width_2*height_1)+
					plate_bending_smoothing*
					(d2h12d2h01*h12h11*width_m2*height_3+
					dh12dh01*dh12dh11*height_1+
					h12h01*d2h12d2h11*width_2*height_m1));
				/* dfdy */
				temp[2] += sin_0*
					(membrane_smoothing*
					(dh12dh01*h12h11*height_3+
					h12h01*dh12dh11*width_2*height_1)+
					plate_bending_smoothing*
					(d2h12d2h01*h12h11*width_m2*height_3+
					dh12dh01*dh12dh11*height_1+
					h12h01*d2h12d2h11*width_2*height_m1));
				/* d(im1_jm1)/d(i_jm1) */
				temp=matrix_and_right_hand_side+
					i_jm1*number_of_equations_plus_1;
				/* d/d(f) */
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh01dh01*h02h01*width_m1*height_1+
					h01h01*dh02dh01*width_1*height_m1)+
					plate_bending_smoothing*
					(d2h01d2h01*h02h01*width_m3*height_1+
					dh01dh01*dh02dh01*width_m1*height_m1+
					h01h01*d2h02d2h01*width_1*height_m3));
				/* dfdx */
				temp[1] += cos_0*
					(membrane_smoothing*
					(dh01dh01*h02h11*width_m1*height_2+
					h01h01*dh02dh11*width_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h02h11*width_m3*height_2+
					dh01dh01*dh02dh11*width_m1+
					h01h01*d2h02d2h11*width_1*height_m2));
				/* dfdy */
				temp[2] += sin_0*
					(membrane_smoothing*
					(dh01dh01*h02h11*width_m1*height_2+
					h01h01*dh02dh11*width_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h02h11*width_m3*height_2+
					dh01dh01*dh02dh11*width_m1+
					h01h01*d2h02d2h11*width_1*height_m2));
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh11dh01*h02h01*height_1+
					h11h01*dh02dh01*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h11d2h01*h02h01*width_m2*height_1+
					dh11dh01*dh02dh01*height_m1+
					h11h01*d2h02d2h01*width_2*height_m3));
				/* dfdx */
				temp[1] += cos_0*
					(membrane_smoothing*
					(dh11dh01*h02h11*height_2+
					h11h01*dh02dh11*width_2)+
					plate_bending_smoothing*
					(d2h11d2h01*h02h11*width_m2*height_2+
					dh11dh01*dh02dh11+
					h11h01*d2h02d2h11*width_2*height_m2));
				/* dfdy */
				temp[2] += sin_0*
					(membrane_smoothing*
					(dh11dh01*h02h11*height_2+
					h11h01*dh02dh11*width_2)+
					plate_bending_smoothing*
					(d2h11d2h01*h02h11*width_m2*height_2+
					dh11dh01*dh02dh11+
					h11h01*d2h02d2h11*width_2*height_m2));
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh01dh01*h12h01*width_m1*height_2+
					h01h01*dh12dh01*width_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h12h01*width_m3*height_2+
					dh01dh01*dh12dh01*width_m1+
					h01h01*d2h12d2h01*width_1*height_m2));
				/* dfdx */
				temp[1] += cos_0*
					(membrane_smoothing*
					(dh01dh01*h12h11*width_m1*height_3+
					h01h01*dh12dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h12h11*width_m3*height_3+
					dh01dh01*dh12dh11*width_m1*height_1+
					h01h01*d2h12d2h11*width_1*height_m1));
				/* dfdy */
				temp[2] += sin_0*
					(membrane_smoothing*
					(dh01dh01*h12h11*width_m1*height_3+
					h01h01*dh12dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h12h11*width_m3*height_3+
					dh01dh01*dh12dh11*width_m1*height_1+
					h01h01*d2h12d2h11*width_1*height_m1));
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh11dh01*h12h01*height_2+
					h11h01*dh12dh01*width_2)+
					plate_bending_smoothing*
					(d2h11d2h01*h12h01*width_m2*height_2+
					dh11dh01*dh12dh01+
					h11h01*d2h12d2h01*width_2*height_m2));
				/* dfdx */
				temp[1] += cos_0*
					(membrane_smoothing*
					(dh11dh01*h12h11*height_3+
					h11h01*dh12dh11*width_2*height_1)+
					plate_bending_smoothing*
					(d2h11d2h01*h12h11*width_m2*height_3+
					dh11dh01*dh12dh11*height_1+
					h11h01*d2h12d2h11*width_2*height_m1));
				/* dfdy */
				temp[2] += sin_0*
					(membrane_smoothing*
					(dh11dh01*h12h11*height_3+
					h11h01*dh12dh11*width_2*height_1)+
					plate_bending_smoothing*
					(d2h11d2h01*h12h11*width_m2*height_3+
					dh11dh01*dh12dh11*height_1+
					h11h01*d2h12d2h11*width_2*height_m1));
				/* d(im1_j)/d(im1_jm1) */
				temp=matrix_and_right_hand_side;
				/* d/d(f) */
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh01dh02*h01h01*width_m1*height_1+
					h01h02*dh01dh01*width_1*height_m1)+
					plate_bending_smoothing*
					(d2h01d2h02*h01h01*width_m3*height_1+
					dh01dh02*dh01dh01*width_m1*height_m1+
					h01h02*d2h01d2h01*width_1*height_m3));
				/* dfdx */
				temp[1] += cos_1*
					(membrane_smoothing*
					(dh01dh02*h01h11*width_m1*height_2+
					h01h02*dh01dh11*width_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h01h11*width_m3*height_2+
					dh01dh02*dh01dh11*width_m1+
					h01h02*d2h01d2h11*width_1*height_m2));
				/* dfdy */
				temp[2] += sin_1*
					(membrane_smoothing*
					(dh01dh02*h01h11*width_m1*height_2+
					h01h02*dh01dh11*width_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h01h11*width_m3*height_2+
					dh01dh02*dh01dh11*width_m1+
					h01h02*d2h01d2h11*width_1*height_m2));
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += cos_0*
					(membrane_smoothing*
					(dh01dh02*h11h01*width_m1*height_2+
					h01h02*dh11dh01*width_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h11h01*width_m3*height_2+
					dh01dh02*dh11dh01*width_m1+
					h01h02*d2h11d2h01*width_1*height_m2));
				/* dfdx */
				temp[1] += cos_1*cos_0*
					(membrane_smoothing*
					(dh01dh02*h11h11*width_m1*height_3+
					h01h02*dh11dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h11h11*width_m3*height_3+
					dh01dh02*dh11dh11*width_m1*height_1+
					h01h02*d2h11d2h11*width_1*height_m1));
				/* dfdy */
				temp[2] += sin_1*cos_0*
					(membrane_smoothing*
					(dh01dh02*h11h11*width_m1*height_3+
					h01h02*dh11dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h11h11*width_m3*height_3+
					dh01dh02*dh11dh11*width_m1*height_1+
					h01h02*d2h11d2h11*width_1*height_m1));
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += sin_0*
					(membrane_smoothing*
					(dh01dh02*h11h01*width_m1*height_2+
					h01h02*dh11dh01*width_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h11h01*width_m3*height_2+
					dh01dh02*dh11dh01*width_m1+
					h01h02*d2h11d2h01*width_1*height_m2));
				/* dfdx */
				temp[1] += cos_1*sin_0*
					(membrane_smoothing*
					(dh01dh02*h11h11*width_m1*height_3+
					h01h02*dh11dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h11h11*width_m3*height_3+
					dh01dh02*dh11dh11*width_m1*height_1+
					h01h02*d2h11d2h11*width_1*height_m1));
				/* dfdy */
				temp[2] += sin_1*sin_0*
					(membrane_smoothing*
					(dh01dh02*h11h11*width_m1*height_3+
					h01h02*dh11dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h11h11*width_m3*height_3+
					dh01dh02*dh11dh11*width_m1*height_1+
					h01h02*d2h11d2h11*width_1*height_m1));
				/* d(im1_j)/d(im1_j) */
				temp=matrix_and_right_hand_side;
				/* d/d(f) */
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh02dh02*h01h01*width_m1*height_1+
					h02h02*dh01dh01*width_1*height_m1)+
					plate_bending_smoothing*
					(d2h02d2h02*h01h01*width_m3*height_1+
					dh02dh02*dh01dh01*width_m1*height_m1+
					h02h02*d2h01d2h01*width_1*height_m3));
				/* dfdx */
				temp[1] += cos_1*
					(membrane_smoothing*
					(dh02dh02*h01h11*width_m1*height_2+
					h02h02*dh01dh11*width_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h01h11*width_m3*height_2+
					dh02dh02*dh01dh11*width_m1+
					h02h02*d2h01d2h11*width_1*height_m2));
				/* dfdy */
				temp[2] += sin_1*
					(membrane_smoothing*
					(dh02dh02*h01h11*width_m1*height_2+
					h02h02*dh01dh11*width_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h01h11*width_m3*height_2+
					dh02dh02*dh01dh11*width_m1+
					h02h02*d2h01d2h11*width_1*height_m2));
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += cos_1*
					(membrane_smoothing*
					(dh02dh02*h11h01*width_m1*height_2+
					h02h02*dh11dh01*width_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h11h01*width_m3*height_2+
					dh02dh02*dh11dh01*width_m1+
					h02h02*d2h11d2h01*width_1*height_m2));
				/* dfdx */
				temp[1] += cos_1*cos_1*
					(membrane_smoothing*
					(dh02dh02*h11h11*width_m1*height_3+
					h02h02*dh11dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h11h11*width_m3*height_3+
					dh02dh02*dh11dh11*width_m1*height_1+
					h02h02*d2h11d2h11*width_1*height_m1));
				/* dfdy */
				temp[2] += sin_1*cos_1*
					(membrane_smoothing*
					(dh02dh02*h11h11*width_m1*height_3+
					h02h02*dh11dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h11h11*width_m3*height_3+
					dh02dh02*dh11dh11*width_m1*height_1+
					h02h02*d2h11d2h11*width_1*height_m1));
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += sin_1*
					(membrane_smoothing*
					(dh02dh02*h11h01*width_m1*height_2+
					h02h02*dh11dh01*width_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h11h01*width_m3*height_2+
					dh02dh02*dh11dh01*width_m1+
					h02h02*d2h11d2h01*width_1*height_m2));
				/* dfdx */
				temp[1] += cos_1*sin_1*
					(membrane_smoothing*
					(dh02dh02*h11h11*width_m1*height_3+
					h02h02*dh11dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h11h11*width_m3*height_3+
					dh02dh02*dh11dh11*width_m1*height_1+
					h02h02*d2h11d2h11*width_1*height_m1));
				/* dfdy */
				temp[2] += sin_1*sin_1*
					(membrane_smoothing*
					(dh02dh02*h11h11*width_m1*height_3+
					h02h02*dh11dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h11h11*width_m3*height_3+
					dh02dh02*dh11dh11*width_m1*height_1+
					h02h02*d2h11d2h11*width_1*height_m1));
				/* d(im1_j)/d(i_j) */
				temp=matrix_and_right_hand_side+
					i_j*number_of_equations_plus_1;
				/* d/d(f) */
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh02dh02*h02h01*width_m1*height_1+
					h02h02*dh02dh01*width_1*height_m1)+
					plate_bending_smoothing*
					(d2h02d2h02*h02h01*width_m3*height_1+
					dh02dh02*dh02dh01*width_m1*height_m1+
					h02h02*d2h02d2h01*width_1*height_m3));
				/* dfdx */
				temp[1] += cos_1*
					(membrane_smoothing*
					(dh02dh02*h02h11*width_m1*height_2+
					h02h02*dh02dh11*width_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h02h11*width_m3*height_2+
					dh02dh02*dh02dh11*width_m1+
					h02h02*d2h02d2h11*width_1*height_m2));
				/* dfdy */
				temp[2] += sin_1*
					(membrane_smoothing*
					(dh02dh02*h02h11*width_m1*height_2+
					h02h02*dh02dh11*width_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h02h11*width_m3*height_2+
					dh02dh02*dh02dh11*width_m1+
					h02h02*d2h02d2h11*width_1*height_m2));
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh12dh02*h02h01*height_1+
					h12h02*dh02dh01*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h12d2h02*h02h01*width_m2*height_1+
					dh12dh02*dh02dh01*height_m1+
					h12h02*d2h02d2h01*width_2*height_m3));
				/* dfdx */
				temp[1] += cos_1*
					(membrane_smoothing*
					(dh12dh02*h02h11*height_2+
					h12h02*dh02dh11*width_2)+
					plate_bending_smoothing*
					(d2h12d2h02*h02h11*width_m2*height_2+
					dh12dh02*dh02dh11+
					h12h02*d2h02d2h11*width_2*height_m2));
				/* dfdy */
				temp[2] += sin_1*
					(membrane_smoothing*
					(dh12dh02*h02h11*height_2+
					h12h02*dh02dh11*width_2)+
					plate_bending_smoothing*
					(d2h12d2h02*h02h11*width_m2*height_2+
					dh12dh02*dh02dh11+
					h12h02*d2h02d2h11*width_2*height_m2));
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh02dh02*h12h01*width_m1*height_2+
					h02h02*dh12dh01*width_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h12h01*width_m3*height_2+
					dh02dh02*dh12dh01*width_m1+
					h02h02*d2h12d2h01*width_1*height_m2));
				/* dfdx */
				temp[1] += cos_1*
					(membrane_smoothing*
					(dh02dh02*h12h11*width_m1*height_3+
					h02h02*dh12dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h12h11*width_m3*height_3+
					dh02dh02*dh12dh11*width_m1*height_1+
					h02h02*d2h12d2h11*width_1*height_m1));
				/* dfdy */
				temp[2] += sin_1*
					(membrane_smoothing*
					(dh02dh02*h12h11*width_m1*height_3+
					h02h02*dh12dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h12h11*width_m3*height_3+
					dh02dh02*dh12dh11*width_m1*height_1+
					h02h02*d2h12d2h11*width_1*height_m1));
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh12dh02*h12h01*height_2+
					h12h02*dh12dh01*width_2)+
					plate_bending_smoothing*
					(d2h12d2h02*h12h01*width_m2*height_2+
					dh12dh02*dh12dh01+
					h12h02*d2h12d2h01*width_2*height_m2));
				/* dfdx */
				temp[1] += cos_1*
					(membrane_smoothing*
					(dh12dh02*h12h11*height_3+
					h12h02*dh12dh11*width_2*height_1)+
					plate_bending_smoothing*
					(d2h12d2h02*h12h11*width_m2*height_3+
					dh12dh02*dh12dh11*height_1+
					h12h02*d2h12d2h11*width_2*height_m1));
				/* dfdy */
				temp[2] += sin_1*
					(membrane_smoothing*
					(dh12dh02*h12h11*height_3+
					h12h02*dh12dh11*width_2*height_1)+
					plate_bending_smoothing*
					(d2h12d2h02*h12h11*width_m2*height_3+
					dh12dh02*dh12dh11*height_1+
					h12h02*d2h12d2h11*width_2*height_m1));
				/* d(im1_j)/d(i_jm1) */
				temp=matrix_and_right_hand_side+
					i_jm1*number_of_equations_plus_1;
				/* d/d(f) */
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh01dh02*h02h01*width_m1*height_1+
					h01h02*dh02dh01*width_1*height_m1)+
					plate_bending_smoothing*
					(d2h01d2h02*h02h01*width_m3*height_1+
					dh01dh02*dh02dh01*width_m1*height_m1+
					h01h02*d2h02d2h01*width_1*height_m3));
				/* dfdx */
				temp[1] += cos_1*
					(membrane_smoothing*
					(dh01dh02*h02h11*width_m1*height_2+
					h01h02*dh02dh11*width_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h02h11*width_m3*height_2+
					dh01dh02*dh02dh11*width_m1+
					h01h02*d2h02d2h11*width_1*height_m2));
				/* dfdy */
				temp[2] += sin_1*
					(membrane_smoothing*
					(dh01dh02*h02h11*width_m1*height_2+
					h01h02*dh02dh11*width_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h02h11*width_m3*height_2+
					dh01dh02*dh02dh11*width_m1+
					h01h02*d2h02d2h11*width_1*height_m2));
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh11dh02*h02h01*height_1+
					h11h02*dh02dh01*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h11d2h02*h02h01*width_m2*height_1+
					dh11dh02*dh02dh01*height_m1+
					h11h02*d2h02d2h01*width_2*height_m3));
				/* dfdx */
				temp[1] += cos_1*
					(membrane_smoothing*
					(dh11dh02*h02h11*height_2+
					h11h02*dh02dh11*width_2)+
					plate_bending_smoothing*
					(d2h11d2h02*h02h11*width_m2*height_2+
					dh11dh02*dh02dh11+
					h11h02*d2h02d2h11*width_2*height_m2));
				/* dfdy */
				temp[2] += sin_1*
					(membrane_smoothing*
					(dh11dh02*h02h11*height_2+
					h11h02*dh02dh11*width_2)+
					plate_bending_smoothing*
					(d2h11d2h02*h02h11*width_m2*height_2+
					dh11dh02*dh02dh11+
					h11h02*d2h02d2h11*width_2*height_m2));
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh01dh02*h12h01*width_m1*height_2+
					h01h02*dh12dh01*width_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h12h01*width_m3*height_2+
					dh01dh02*dh12dh01*width_m1+
					h01h02*d2h12d2h01*width_1*height_m2));
				/* dfdx */
				temp[1] += cos_1*
					(membrane_smoothing*
					(dh01dh02*h12h11*width_m1*height_3+
					h01h02*dh12dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h12h11*width_m3*height_3+
					dh01dh02*dh12dh11*width_m1*height_1+
					h01h02*d2h12d2h11*width_1*height_m1));
				/* dfdy */
				temp[2] += sin_1*
					(membrane_smoothing*
					(dh01dh02*h12h11*width_m1*height_3+
					h01h02*dh12dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h12h11*width_m3*height_3+
					dh01dh02*dh12dh11*width_m1*height_1+
					h01h02*d2h12d2h11*width_1*height_m1));
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh11dh02*h12h01*height_2+
					h11h02*dh12dh01*width_2)+
					plate_bending_smoothing*
					(d2h11d2h02*h12h01*width_m2*height_2+
					dh11dh02*dh12dh01+
					h11h02*d2h12d2h01*width_2*height_m2));
				/* dfdx */
				temp[1] += cos_1*
					(membrane_smoothing*
					(dh11dh02*h12h11*height_3+
					h11h02*dh12dh11*width_2*height_1)+
					plate_bending_smoothing*
					(d2h11d2h02*h12h11*width_m2*height_3+
					dh11dh02*dh12dh11*height_1+
					h11h02*d2h12d2h11*width_2*height_m1));
				/* dfdy */
				temp[2] += sin_1*
					(membrane_smoothing*
					(dh11dh02*h12h11*height_3+
					h11h02*dh12dh11*width_2*height_1)+
					plate_bending_smoothing*
					(d2h11d2h02*h12h11*width_m2*height_3+
					dh11dh02*dh12dh11*height_1+
					h11h02*d2h12d2h11*width_2*height_m1));
			}
			else
			{
#endif /* defined (FIX_APEX) */
				/* d(im1_jm1)/d(im1_jm1) */
				temp=matrix_and_right_hand_side+
					im1_jm1*number_of_equations_plus_1+im1_jm1;
				/* d/d(f) */
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh01dh01*h01h01*width_m1*height_1+
					h01h01*dh01dh01*width_1*height_m1)+
					plate_bending_smoothing*
					(d2h01d2h01*h01h01*width_m3*height_1+
					dh01dh01*dh01dh01*width_m1*height_m1+
					h01h01*d2h01d2h01*width_1*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh01dh11*h01h01*height_1+
					h01h11*dh01dh01*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h01d2h11*h01h01*width_m2*height_1+
					dh01dh11*dh01dh01*height_m1+
					h01h11*d2h01d2h01*width_2*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh01dh01*h01h11*width_m1*height_2+
					h01h01*dh01dh11*width_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h01h11*width_m3*height_2+
					dh01dh01*dh01dh11*width_m1+
					h01h01*d2h01d2h11*width_1*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh01dh11*h01h11*height_2+
					h01h11*dh01dh11*width_2)+
					plate_bending_smoothing*
					(d2h01d2h11*h01h11*width_m2*height_2+
					dh01dh11*dh01dh11+
					h01h11*d2h01d2h11*width_2*height_m2));
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh11dh01*h01h01*height_1+
					h11h01*dh01dh01*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h11d2h01*h01h01*width_m2*height_1+
					dh11dh01*dh01dh01*height_m1+
					h11h01*d2h01d2h01*width_2*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh11dh11*h01h01*width_1*height_1+
					h11h11*dh01dh01*width_3*height_m1)+
					plate_bending_smoothing*
					(d2h11d2h11*h01h01*width_m1*height_1+
					dh11dh11*dh01dh01*width_1*height_m1+
					h11h11*d2h01d2h01*width_3*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh11dh01*h01h11*height_2+
					h11h01*dh01dh11*width_2)+
					plate_bending_smoothing*
					(d2h11d2h01*h01h11*width_m2*height_2+
					dh11dh01*dh01dh11+
					h11h01*d2h01d2h11*width_2*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh11dh11*h01h11*width_1*height_2+
					h11h11*dh01dh11*width_3)+
					plate_bending_smoothing*
					(d2h11d2h11*h01h11*width_m1*height_2+
					dh11dh11*dh01dh11*width_1+
					h11h11*d2h01d2h11*width_3*height_m2));
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh01dh01*h11h01*width_m1*height_2+
					h01h01*dh11dh01*width_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h11h01*width_m3*height_2+
					dh01dh01*dh11dh01*width_m1+
					h01h01*d2h11d2h01*width_1*height_m2));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh01dh11*h11h01*height_2+
					h01h11*dh11dh01*width_2)+
					plate_bending_smoothing*
					(d2h01d2h11*h11h01*width_m2*height_2+
					dh01dh11*dh11dh01+
					h01h11*d2h11d2h01*width_2*height_m2));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh01dh01*h11h11*width_m1*height_3+
					h01h01*dh11dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h11h11*width_m3*height_3+
					dh01dh01*dh11dh11*width_m1*height_1+
					h01h01*d2h11d2h11*width_1*height_m1));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh01dh11*h11h11*height_3+
					h01h11*dh11dh11*width_2*height_1)+
					plate_bending_smoothing*
					(d2h01d2h11*h11h11*width_m2*height_3+
					dh01dh11*dh11dh11*height_1+
					h01h11*d2h11d2h11*width_2*height_m1));
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh11dh01*h11h01*height_2+
					h11h01*dh11dh01*width_2)+
					plate_bending_smoothing*
					(d2h11d2h01*h11h01*width_m2*height_2+
					dh11dh01*dh11dh01+
					h11h01*d2h11d2h01*width_2*height_m2));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh11dh11*h11h01*width_1*height_2+
					h11h11*dh11dh01*width_3)+
					plate_bending_smoothing*
					(d2h11d2h11*h11h01*width_m1*height_2+
					dh11dh11*dh11dh01*width_1+
					h11h11*d2h11d2h01*width_3*height_m2));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh11dh01*h11h11*height_3+
					h11h01*dh11dh11*width_2*height_1)+
					plate_bending_smoothing*
					(d2h11d2h01*h11h11*width_m2*height_3+
					dh11dh01*dh11dh11*height_1+
					h11h01*d2h11d2h11*width_2*height_m1));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh11dh11*h11h11*width_1*height_3+
					h11h11*dh11dh11*width_3*height_1)+
					plate_bending_smoothing*
					(d2h11d2h11*h11h11*width_m1*height_3+
					dh11dh11*dh11dh11*width_1*height_1+
					h11h11*d2h11d2h11*width_3*height_m1));
				/* d(im1_jm1)/d(im1_j) */
				temp=matrix_and_right_hand_side+
					im1_j*number_of_equations_plus_1+im1_jm1;
				/* d/d(f) */
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh02dh01*h01h01*width_m1*height_1+
					h02h01*dh01dh01*width_1*height_m1)+
					plate_bending_smoothing*
					(d2h02d2h01*h01h01*width_m3*height_1+
					dh02dh01*dh01dh01*width_m1*height_m1+
					h02h01*d2h01d2h01*width_1*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh02dh11*h01h01*height_1+
					h02h11*dh01dh01*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h02d2h11*h01h01*width_m2*height_1+
					dh02dh11*dh01dh01*height_m1+
					h02h11*d2h01d2h01*width_2*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh02dh01*h01h11*width_m1*height_2+
					h02h01*dh01dh11*width_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h01h11*width_m3*height_2+
					dh02dh01*dh01dh11*width_m1+
					h02h01*d2h01d2h11*width_1*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh02dh11*h01h11*height_2+
					h02h11*dh01dh11*width_2)+
					plate_bending_smoothing*
					(d2h02d2h11*h01h11*width_m2*height_2+
					dh02dh11*dh01dh11+
					h02h11*d2h01d2h11*width_2*height_m2));
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh12dh01*h01h01*height_1+
					h12h01*dh01dh01*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h12d2h01*h01h01*width_m2*height_1+
					dh12dh01*dh01dh01*height_m1+
					h12h01*d2h01d2h01*width_2*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh12dh11*h01h01*width_1*height_1+
					h12h11*dh01dh01*width_3*height_m1)+
					plate_bending_smoothing*
					(d2h12d2h11*h01h01*width_m1*height_1+
					dh12dh11*dh01dh01*width_1*height_m1+
					h12h11*d2h01d2h01*width_3*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh12dh01*h01h11*height_2+
					h12h01*dh01dh11*width_2)+
					plate_bending_smoothing*
					(d2h12d2h01*h01h11*width_m2*height_2+
					dh12dh01*dh01dh11+
					h12h01*d2h01d2h11*width_2*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh12dh11*h01h11*width_1*height_2+
					h12h11*dh01dh11*width_3)+
					plate_bending_smoothing*
					(d2h12d2h11*h01h11*width_m1*height_2+
					dh12dh11*dh01dh11*width_1+
					h12h11*d2h01d2h11*width_3*height_m2));
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh02dh01*h11h01*width_m1*height_2+
					h02h01*dh11dh01*width_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h11h01*width_m3*height_2+
					dh02dh01*dh11dh01*width_m1+
					h02h01*d2h11d2h01*width_1*height_m2));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh02dh11*h11h01*height_2+
					h02h11*dh11dh01*width_2)+
					plate_bending_smoothing*
					(d2h02d2h11*h11h01*width_m2*height_2+
					dh02dh11*dh11dh01+
					h02h11*d2h11d2h01*width_2*height_m2));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh02dh01*h11h11*width_m1*height_3+
					h02h01*dh11dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h11h11*width_m3*height_3+
					dh02dh01*dh11dh11*width_m1*height_1+
					h02h01*d2h11d2h11*width_1*height_m1));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh02dh11*h11h11*height_3+
					h02h11*dh11dh11*width_2*height_1)+
					plate_bending_smoothing*
					(d2h02d2h11*h11h11*width_m2*height_3+
					dh02dh11*dh11dh11*height_1+
					h02h11*d2h11d2h11*width_2*height_m1));
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh12dh01*h11h01*height_2+
					h12h01*dh11dh01*width_2)+
					plate_bending_smoothing*
					(d2h12d2h01*h11h01*width_m2*height_2+
					dh12dh01*dh11dh01+
					h12h01*d2h11d2h01*width_2*height_m2));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh12dh11*h11h01*width_1*height_2+
					h12h11*dh11dh01*width_3)+
					plate_bending_smoothing*
					(d2h12d2h11*h11h01*width_m1*height_2+
					dh12dh11*dh11dh01*width_1+
					h12h11*d2h11d2h01*width_3*height_m2));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh12dh01*h11h11*height_3+
					h12h01*dh11dh11*width_2*height_1)+
					plate_bending_smoothing*
					(d2h12d2h01*h11h11*width_m2*height_3+
					dh12dh01*dh11dh11*height_1+
					h12h01*d2h11d2h11*width_2*height_m1));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh12dh11*h11h11*width_1*height_3+
					h12h11*dh11dh11*width_3*height_1)+
					plate_bending_smoothing*
					(d2h12d2h11*h11h11*width_m1*height_3+
					dh12dh11*dh11dh11*width_1*height_1+
					h12h11*d2h11d2h11*width_3*height_m1));
				/* d(im1_jm1)/d(i_j) */
				temp=matrix_and_right_hand_side+
					i_j*number_of_equations_plus_1+im1_jm1;
				/* d/d(f) */
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh02dh01*h02h01*width_m1*height_1+
					h02h01*dh02dh01*width_1*height_m1)+
					plate_bending_smoothing*
					(d2h02d2h01*h02h01*width_m3*height_1+
					dh02dh01*dh02dh01*width_m1*height_m1+
					h02h01*d2h02d2h01*width_1*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh02dh11*h02h01*height_1+
					h02h11*dh02dh01*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h02d2h11*h02h01*width_m2*height_1+
					dh02dh11*dh02dh01*height_m1+
					h02h11*d2h02d2h01*width_2*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh02dh01*h02h11*width_m1*height_2+
					h02h01*dh02dh11*width_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h02h11*width_m3*height_2+
					dh02dh01*dh02dh11*width_m1+
					h02h01*d2h02d2h11*width_1*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh02dh11*h02h11*height_2+
					h02h11*dh02dh11*width_2)+
					plate_bending_smoothing*
					(d2h02d2h11*h02h11*width_m2*height_2+
					dh02dh11*dh02dh11+
					h02h11*d2h02d2h11*width_2*height_m2));
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh12dh01*h02h01*height_1+
					h12h01*dh02dh01*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h12d2h01*h02h01*width_m2*height_1+
					dh12dh01*dh02dh01*height_m1+
					h12h01*d2h02d2h01*width_2*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh12dh11*h02h01*width_1*height_1+
					h12h11*dh02dh01*width_3*height_m1)+
					plate_bending_smoothing*
					(d2h12d2h11*h02h01*width_m1*height_1+
					dh12dh11*dh02dh01*width_1*height_m1+
					h12h11*d2h02d2h01*width_3*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh12dh01*h02h11*height_2+
					h12h01*dh02dh11*width_2)+
					plate_bending_smoothing*
					(d2h12d2h01*h02h11*width_m2*height_2+
					dh12dh01*dh02dh11+
					h12h01*d2h02d2h11*width_2*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh12dh11*h02h11*width_1*height_2+
					h12h11*dh02dh11*width_3)+
					plate_bending_smoothing*
					(d2h12d2h11*h02h11*width_m1*height_2+
					dh12dh11*dh02dh11*width_1+
					h12h11*d2h02d2h11*width_3*height_m2));
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh02dh01*h12h01*width_m1*height_2+
					h02h01*dh12dh01*width_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h12h01*width_m3*height_2+
					dh02dh01*dh12dh01*width_m1+
					h02h01*d2h12d2h01*width_1*height_m2));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh02dh11*h12h01*height_2+
					h02h11*dh12dh01*width_2)+
					plate_bending_smoothing*
					(d2h02d2h11*h12h01*width_m2*height_2+
					dh02dh11*dh12dh01+
					h02h11*d2h12d2h01*width_2*height_m2));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh02dh01*h12h11*width_m1*height_3+
					h02h01*dh12dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h12h11*width_m3*height_3+
					dh02dh01*dh12dh11*width_m1*height_1+
					h02h01*d2h12d2h11*width_1*height_m1));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh02dh11*h12h11*height_3+
					h02h11*dh12dh11*width_2*height_1)+
					plate_bending_smoothing*
					(d2h02d2h11*h12h11*width_m2*height_3+
					dh02dh11*dh12dh11*height_1+
					h02h11*d2h12d2h11*width_2*height_m1));
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh12dh01*h12h01*height_2+
					h12h01*dh12dh01*width_2)+
					plate_bending_smoothing*
					(d2h12d2h01*h12h01*width_m2*height_2+
					dh12dh01*dh12dh01+
					h12h01*d2h12d2h01*width_2*height_m2));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh12dh11*h12h01*width_1*height_2+
					h12h11*dh12dh01*width_3)+
					plate_bending_smoothing*
					(d2h12d2h11*h12h01*width_m1*height_2+
					dh12dh11*dh12dh01*width_1+
					h12h11*d2h12d2h01*width_3*height_m2));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh12dh01*h12h11*height_3+
					h12h01*dh12dh11*width_2*height_1)+
					plate_bending_smoothing*
					(d2h12d2h01*h12h11*width_m2*height_3+
					dh12dh01*dh12dh11*height_1+
					h12h01*d2h12d2h11*width_2*height_m1));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh12dh11*h12h11*width_1*height_3+
					h12h11*dh12dh11*width_3*height_1)+
					plate_bending_smoothing*
					(d2h12d2h11*h12h11*width_m1*height_3+
					dh12dh11*dh12dh11*width_1*height_1+
					h12h11*d2h12d2h11*width_3*height_m1));
				/* d(im1_jm1)/d(i_jm1) */
				temp=matrix_and_right_hand_side+
					i_jm1*number_of_equations_plus_1+im1_jm1;
				/* d/d(f) */
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh01dh01*h02h01*width_m1*height_1+
					h01h01*dh02dh01*width_1*height_m1)+
					plate_bending_smoothing*
					(d2h01d2h01*h02h01*width_m3*height_1+
					dh01dh01*dh02dh01*width_m1*height_m1+
					h01h01*d2h02d2h01*width_1*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh01dh11*h02h01*height_1+
					h01h11*dh02dh01*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h01d2h11*h02h01*width_m2*height_1+
					dh01dh11*dh02dh01*height_m1+
					h01h11*d2h02d2h01*width_2*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh01dh01*h02h11*width_m1*height_2+
					h01h01*dh02dh11*width_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h02h11*width_m3*height_2+
					dh01dh01*dh02dh11*width_m1+
					h01h01*d2h02d2h11*width_1*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh01dh11*h02h11*height_2+
					h01h11*dh02dh11*width_2)+
					plate_bending_smoothing*
					(d2h01d2h11*h02h11*width_m2*height_2+
					dh01dh11*dh02dh11+
					h01h11*d2h02d2h11*width_2*height_m2));
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh11dh01*h02h01*height_1+
					h11h01*dh02dh01*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h11d2h01*h02h01*width_m2*height_1+
					dh11dh01*dh02dh01*height_m1+
					h11h01*d2h02d2h01*width_2*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh11dh11*h02h01*width_1*height_1+
					h11h11*dh02dh01*width_3*height_m1)+
					plate_bending_smoothing*
					(d2h11d2h11*h02h01*width_m1*height_1+
					dh11dh11*dh02dh01*width_1*height_m1+
					h11h11*d2h02d2h01*width_3*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh11dh01*h02h11*height_2+
					h11h01*dh02dh11*width_2)+
					plate_bending_smoothing*
					(d2h11d2h01*h02h11*width_m2*height_2+
					dh11dh01*dh02dh11+
					h11h01*d2h02d2h11*width_2*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh11dh11*h02h11*width_1*height_2+
					h11h11*dh02dh11*width_3)+
					plate_bending_smoothing*
					(d2h11d2h11*h02h11*width_m1*height_2+
					dh11dh11*dh02dh11*width_1+
					h11h11*d2h02d2h11*width_3*height_m2));
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh01dh01*h12h01*width_m1*height_2+
					h01h01*dh12dh01*width_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h12h01*width_m3*height_2+
					dh01dh01*dh12dh01*width_m1+
					h01h01*d2h12d2h01*width_1*height_m2));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh01dh11*h12h01*height_2+
					h01h11*dh12dh01*width_2)+
					plate_bending_smoothing*
					(d2h01d2h11*h12h01*width_m2*height_2+
					dh01dh11*dh12dh01+
					h01h11*d2h12d2h01*width_2*height_m2));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh01dh01*h12h11*width_m1*height_3+
					h01h01*dh12dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h12h11*width_m3*height_3+
					dh01dh01*dh12dh11*width_m1*height_1+
					h01h01*d2h12d2h11*width_1*height_m1));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh01dh11*h12h11*height_3+
					h01h11*dh12dh11*width_2*height_1)+
					plate_bending_smoothing*
					(d2h01d2h11*h12h11*width_m2*height_3+
					dh01dh11*dh12dh11*height_1+
					h01h11*d2h12d2h11*width_2*height_m1));
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh11dh01*h12h01*height_2+
					h11h01*dh12dh01*width_2)+
					plate_bending_smoothing*
					(d2h11d2h01*h12h01*width_m2*height_2+
					dh11dh01*dh12dh01+
					h11h01*d2h12d2h01*width_2*height_m2));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh11dh11*h12h01*width_1*height_2+
					h11h11*dh12dh01*width_3)+
					plate_bending_smoothing*
					(d2h11d2h11*h12h01*width_m1*height_2+
					dh11dh11*dh12dh01*width_1+
					h11h11*d2h12d2h01*width_3*height_m2));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh11dh01*h12h11*height_3+
					h11h01*dh12dh11*width_2*height_1)+
					plate_bending_smoothing*
					(d2h11d2h01*h12h11*width_m2*height_3+
					dh11dh01*dh12dh11*height_1+
					h11h01*d2h12d2h11*width_2*height_m1));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh11dh11*h12h11*width_1*height_3+
					h11h11*dh12dh11*width_3*height_1)+
					plate_bending_smoothing*
					(d2h11d2h11*h12h11*width_m1*height_3+
					dh11dh11*dh12dh11*width_1*height_1+
					h11h11*d2h12d2h11*width_3*height_m1));
				/* d(im1_j)/d(im1_jm1) */
				temp=matrix_and_right_hand_side+
					im1_jm1*number_of_equations_plus_1+im1_j;
				/* d/d(f) */
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh01dh02*h01h01*width_m1*height_1+
					h01h02*dh01dh01*width_1*height_m1)+
					plate_bending_smoothing*
					(d2h01d2h02*h01h01*width_m3*height_1+
					dh01dh02*dh01dh01*width_m1*height_m1+
					h01h02*d2h01d2h01*width_1*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh01dh12*h01h01*height_1+
					h01h12*dh01dh01*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h01d2h12*h01h01*width_m2*height_1+
					dh01dh12*dh01dh01*height_m1+
					h01h12*d2h01d2h01*width_2*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh01dh02*h01h11*width_m1*height_2+
					h01h02*dh01dh11*width_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h01h11*width_m3*height_2+
					dh01dh02*dh01dh11*width_m1+
					h01h02*d2h01d2h11*width_1*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh01dh12*h01h11*height_2+
					h01h12*dh01dh11*width_2)+
					plate_bending_smoothing*
					(d2h01d2h12*h01h11*width_m2*height_2+
					dh01dh12*dh01dh11+
					h01h12*d2h01d2h11*width_2*height_m2));
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh11dh02*h01h01*height_1+
					h11h02*dh01dh01*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h11d2h02*h01h01*width_m2*height_1+
					dh11dh02*dh01dh01*height_m1+
					h11h02*d2h01d2h01*width_2*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh11dh12*h01h01*width_1*height_1+
					h11h12*dh01dh01*width_3*height_m1)+
					plate_bending_smoothing*
					(d2h11d2h12*h01h01*width_m1*height_1+
					dh11dh12*dh01dh01*width_1*height_m1+
					h11h12*d2h01d2h01*width_3*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh11dh02*h01h11*height_2+
					h11h02*dh01dh11*width_2)+
					plate_bending_smoothing*
					(d2h11d2h02*h01h11*width_m2*height_2+
					dh11dh02*dh01dh11+
					h11h02*d2h01d2h11*width_2*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh11dh12*h01h11*width_1*height_2+
					h11h12*dh01dh11*width_3)+
					plate_bending_smoothing*
					(d2h11d2h12*h01h11*width_m1*height_2+
					dh11dh12*dh01dh11*width_1+
					h11h12*d2h01d2h11*width_3*height_m2));
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh01dh02*h11h01*width_m1*height_2+
					h01h02*dh11dh01*width_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h11h01*width_m3*height_2+
					dh01dh02*dh11dh01*width_m1+
					h01h02*d2h11d2h01*width_1*height_m2));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh01dh12*h11h01*height_2+
					h01h12*dh11dh01*width_2)+
					plate_bending_smoothing*
					(d2h01d2h12*h11h01*width_m2*height_2+
					dh01dh12*dh11dh01+
					h01h12*d2h11d2h01*width_2*height_m2));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh01dh02*h11h11*width_m1*height_3+
					h01h02*dh11dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h11h11*width_m3*height_3+
					dh01dh02*dh11dh11*width_m1*height_1+
					h01h02*d2h11d2h11*width_1*height_m1));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh01dh12*h11h11*height_3+
					h01h12*dh11dh11*width_2*height_1)+
					plate_bending_smoothing*
					(d2h01d2h12*h11h11*width_m2*height_3+
					dh01dh12*dh11dh11*height_1+
					h01h12*d2h11d2h11*width_2*height_m1));
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh11dh02*h11h01*height_2+
					h11h02*dh11dh01*width_2)+
					plate_bending_smoothing*
					(d2h11d2h02*h11h01*width_m2*height_2+
					dh11dh02*dh11dh01+
					h11h02*d2h11d2h01*width_2*height_m2));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh11dh12*h11h01*width_1*height_2+
					h11h12*dh11dh01*width_3)+
					plate_bending_smoothing*
					(d2h11d2h12*h11h01*width_m1*height_2+
					dh11dh12*dh11dh01*width_1+
					h11h12*d2h11d2h01*width_3*height_m2));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh11dh02*h11h11*height_3+
					h11h02*dh11dh11*width_2*height_1)+
					plate_bending_smoothing*
					(d2h11d2h02*h11h11*width_m2*height_3+
					dh11dh02*dh11dh11*height_1+
					h11h02*d2h11d2h11*width_2*height_m1));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh11dh12*h11h11*width_1*height_3+
					h11h12*dh11dh11*width_3*height_1)+
					plate_bending_smoothing*
					(d2h11d2h12*h11h11*width_m1*height_3+
					dh11dh12*dh11dh11*width_1*height_1+
					h11h12*d2h11d2h11*width_3*height_m1));
				/* d(im1_j)/d(im1_j) */
				temp=matrix_and_right_hand_side+
					im1_j*number_of_equations_plus_1+im1_j;
				/* d/d(f) */
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh02dh02*h01h01*width_m1*height_1+
					h02h02*dh01dh01*width_1*height_m1)+
					plate_bending_smoothing*
					(d2h02d2h02*h01h01*width_m3*height_1+
					dh02dh02*dh01dh01*width_m1*height_m1+
					h02h02*d2h01d2h01*width_1*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh02dh12*h01h01*height_1+
					h02h12*dh01dh01*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h02d2h12*h01h01*width_m2*height_1+
					dh02dh12*dh01dh01*height_m1+
					h02h12*d2h01d2h01*width_2*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh02dh02*h01h11*width_m1*height_2+
					h02h02*dh01dh11*width_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h01h11*width_m3*height_2+
					dh02dh02*dh01dh11*width_m1+
					h02h02*d2h01d2h11*width_1*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh02dh12*h01h11*height_2+
					h02h12*dh01dh11*width_2)+
					plate_bending_smoothing*
					(d2h02d2h12*h01h11*width_m2*height_2+
					dh02dh12*dh01dh11+
					h02h12*d2h01d2h11*width_2*height_m2));
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh12dh02*h01h01*height_1+
					h12h02*dh01dh01*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h12d2h02*h01h01*width_m2*height_1+
					dh12dh02*dh01dh01*height_m1+
					h12h02*d2h01d2h01*width_2*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh12dh12*h01h01*width_1*height_1+
					h12h12*dh01dh01*width_3*height_m1)+
					plate_bending_smoothing*
					(d2h12d2h12*h01h01*width_m1*height_1+
					dh12dh12*dh01dh01*width_1*height_m1+
					h12h12*d2h01d2h01*width_3*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh12dh02*h01h11*height_2+
					h12h02*dh01dh11*width_2)+
					plate_bending_smoothing*
					(d2h12d2h02*h01h11*width_m2*height_2+
					dh12dh02*dh01dh11+
					h12h02*d2h01d2h11*width_2*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh12dh12*h01h11*width_1*height_2+
					h12h12*dh01dh11*width_3)+
					plate_bending_smoothing*
					(d2h12d2h12*h01h11*width_m1*height_2+
					dh12dh12*dh01dh11*width_1+
					h12h12*d2h01d2h11*width_3*height_m2));
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh02dh02*h11h01*width_m1*height_2+
					h02h02*dh11dh01*width_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h11h01*width_m3*height_2+
					dh02dh02*dh11dh01*width_m1+
					h02h02*d2h11d2h01*width_1*height_m2));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh02dh12*h11h01*height_2+
					h02h12*dh11dh01*width_2)+
					plate_bending_smoothing*
					(d2h02d2h12*h11h01*width_m2*height_2+
					dh02dh12*dh11dh01+
					h02h12*d2h11d2h01*width_2*height_m2));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh02dh02*h11h11*width_m1*height_3+
					h02h02*dh11dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h11h11*width_m3*height_3+
					dh02dh02*dh11dh11*width_m1*height_1+
					h02h02*d2h11d2h11*width_1*height_m1));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh02dh12*h11h11*height_3+
					h02h12*dh11dh11*width_2*height_1)+
					plate_bending_smoothing*
					(d2h02d2h12*h11h11*width_m2*height_3+
					dh02dh12*dh11dh11*height_1+
					h02h12*d2h11d2h11*width_2*height_m1));
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh12dh02*h11h01*height_2+
					h12h02*dh11dh01*width_2)+
					plate_bending_smoothing*
					(d2h12d2h02*h11h01*width_m2*height_2+
					dh12dh02*dh11dh01+
					h12h02*d2h11d2h01*width_2*height_m2));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh12dh12*h11h01*width_1*height_2+
					h12h12*dh11dh01*width_3)+
					plate_bending_smoothing*
					(d2h12d2h12*h11h01*width_m1*height_2+
					dh12dh12*dh11dh01*width_1+
					h12h12*d2h11d2h01*width_3*height_m2));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh12dh02*h11h11*height_3+
					h12h02*dh11dh11*width_2*height_1)+
					plate_bending_smoothing*
					(d2h12d2h02*h11h11*width_m2*height_3+
					dh12dh02*dh11dh11*height_1+
					h12h02*d2h11d2h11*width_2*height_m1));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh12dh12*h11h11*width_1*height_3+
					h12h12*dh11dh11*width_3*height_1)+
					plate_bending_smoothing*
					(d2h12d2h12*h11h11*width_m1*height_3+
					dh12dh12*dh11dh11*width_1*height_1+
					h12h12*d2h11d2h11*width_3*height_m1));
				/* d(im1_j)/d(i_j) */
				temp=matrix_and_right_hand_side+
					i_j*number_of_equations_plus_1+im1_j;
				/* d/d(f) */
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh02dh02*h02h01*width_m1*height_1+
					h02h02*dh02dh01*width_1*height_m1)+
					plate_bending_smoothing*
					(d2h02d2h02*h02h01*width_m3*height_1+
					dh02dh02*dh02dh01*width_m1*height_m1+
					h02h02*d2h02d2h01*width_1*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh02dh12*h02h01*height_1+
					h02h12*dh02dh01*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h02d2h12*h02h01*width_m2*height_1+
					dh02dh12*dh02dh01*height_m1+
					h02h12*d2h02d2h01*width_2*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh02dh02*h02h11*width_m1*height_2+
					h02h02*dh02dh11*width_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h02h11*width_m3*height_2+
					dh02dh02*dh02dh11*width_m1+
					h02h02*d2h02d2h11*width_1*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh02dh12*h02h11*height_2+
					h02h12*dh02dh11*width_2)+
					plate_bending_smoothing*
					(d2h02d2h12*h02h11*width_m2*height_2+
					dh02dh12*dh02dh11+
					h02h12*d2h02d2h11*width_2*height_m2));
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh12dh02*h02h01*height_1+
					h12h02*dh02dh01*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h12d2h02*h02h01*width_m2*height_1+
					dh12dh02*dh02dh01*height_m1+
					h12h02*d2h02d2h01*width_2*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh12dh12*h02h01*width_1*height_1+
					h12h12*dh02dh01*width_3*height_m1)+
					plate_bending_smoothing*
					(d2h12d2h12*h02h01*width_m1*height_1+
					dh12dh12*dh02dh01*width_1*height_m1+
					h12h12*d2h02d2h01*width_3*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh12dh02*h02h11*height_2+
					h12h02*dh02dh11*width_2)+
					plate_bending_smoothing*
					(d2h12d2h02*h02h11*width_m2*height_2+
					dh12dh02*dh02dh11+
					h12h02*d2h02d2h11*width_2*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh12dh12*h02h11*width_1*height_2+
					h12h12*dh02dh11*width_3)+
					plate_bending_smoothing*
					(d2h12d2h12*h02h11*width_m1*height_2+
					dh12dh12*dh02dh11*width_1+
					h12h12*d2h02d2h11*width_3*height_m2));
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh02dh02*h12h01*width_m1*height_2+
					h02h02*dh12dh01*width_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h12h01*width_m3*height_2+
					dh02dh02*dh12dh01*width_m1+
					h02h02*d2h12d2h01*width_1*height_m2));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh02dh12*h12h01*height_2+
					h02h12*dh12dh01*width_2)+
					plate_bending_smoothing*
					(d2h02d2h12*h12h01*width_m2*height_2+
					dh02dh12*dh12dh01+
					h02h12*d2h12d2h01*width_2*height_m2));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh02dh02*h12h11*width_m1*height_3+
					h02h02*dh12dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h12h11*width_m3*height_3+
					dh02dh02*dh12dh11*width_m1*height_1+
					h02h02*d2h12d2h11*width_1*height_m1));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh02dh12*h12h11*height_3+
					h02h12*dh12dh11*width_2*height_1)+
					plate_bending_smoothing*
					(d2h02d2h12*h12h11*width_m2*height_3+
					dh02dh12*dh12dh11*height_1+
					h02h12*d2h12d2h11*width_2*height_m1));
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh12dh02*h12h01*height_2+
					h12h02*dh12dh01*width_2)+
					plate_bending_smoothing*
					(d2h12d2h02*h12h01*width_m2*height_2+
					dh12dh02*dh12dh01+
					h12h02*d2h12d2h01*width_2*height_m2));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh12dh12*h12h01*width_1*height_2+
					h12h12*dh12dh01*width_3)+
					plate_bending_smoothing*
					(d2h12d2h12*h12h01*width_m1*height_2+
					dh12dh12*dh12dh01*width_1+
					h12h12*d2h12d2h01*width_3*height_m2));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh12dh02*h12h11*height_3+
					h12h02*dh12dh11*width_2*height_1)+
					plate_bending_smoothing*
					(d2h12d2h02*h12h11*width_m2*height_3+
					dh12dh02*dh12dh11*height_1+
					h12h02*d2h12d2h11*width_2*height_m1));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh12dh12*h12h11*width_1*height_3+
					h12h12*dh12dh11*width_3*height_1)+
					plate_bending_smoothing*
					(d2h12d2h12*h12h11*width_m1*height_3+
					dh12dh12*dh12dh11*width_1*height_1+
					h12h12*d2h12d2h11*width_3*height_m1));
				/* d(im1_j)/d(i_jm1) */
				temp=matrix_and_right_hand_side+
					i_jm1*number_of_equations_plus_1+im1_j;
				/* d/d(f) */
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh01dh02*h02h01*width_m1*height_1+
					h01h02*dh02dh01*width_1*height_m1)+
					plate_bending_smoothing*
					(d2h01d2h02*h02h01*width_m3*height_1+
					dh01dh02*dh02dh01*width_m1*height_m1+
					h01h02*d2h02d2h01*width_1*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh01dh12*h02h01*height_1+
					h01h12*dh02dh01*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h01d2h12*h02h01*width_m2*height_1+
					dh01dh12*dh02dh01*height_m1+
					h01h12*d2h02d2h01*width_2*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh01dh02*h02h11*width_m1*height_2+
					h01h02*dh02dh11*width_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h02h11*width_m3*height_2+
					dh01dh02*dh02dh11*width_m1+
					h01h02*d2h02d2h11*width_1*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh01dh12*h02h11*height_2+
					h01h12*dh02dh11*width_2)+
					plate_bending_smoothing*
					(d2h01d2h12*h02h11*width_m2*height_2+
					dh01dh12*dh02dh11+
					h01h12*d2h02d2h11*width_2*height_m2));
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh11dh02*h02h01*height_1+
					h11h02*dh02dh01*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h11d2h02*h02h01*width_m2*height_1+
					dh11dh02*dh02dh01*height_m1+
					h11h02*d2h02d2h01*width_2*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh11dh12*h02h01*width_1*height_1+
					h11h12*dh02dh01*width_3*height_m1)+
					plate_bending_smoothing*
					(d2h11d2h12*h02h01*width_m1*height_1+
					dh11dh12*dh02dh01*width_1*height_m1+
					h11h12*d2h02d2h01*width_3*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh11dh02*h02h11*height_2+
					h11h02*dh02dh11*width_2)+
					plate_bending_smoothing*
					(d2h11d2h02*h02h11*width_m2*height_2+
					dh11dh02*dh02dh11+
					h11h02*d2h02d2h11*width_2*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh11dh12*h02h11*width_1*height_2+
					h11h12*dh02dh11*width_3)+
					plate_bending_smoothing*
					(d2h11d2h12*h02h11*width_m1*height_2+
					dh11dh12*dh02dh11*width_1+
					h11h12*d2h02d2h11*width_3*height_m2));
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh01dh02*h12h01*width_m1*height_2+
					h01h02*dh12dh01*width_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h12h01*width_m3*height_2+
					dh01dh02*dh12dh01*width_m1+
					h01h02*d2h12d2h01*width_1*height_m2));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh01dh12*h12h01*height_2+
					h01h12*dh12dh01*width_2)+
					plate_bending_smoothing*
					(d2h01d2h12*h12h01*width_m2*height_2+
					dh01dh12*dh12dh01+
					h01h12*d2h12d2h01*width_2*height_m2));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh01dh02*h12h11*width_m1*height_3+
					h01h02*dh12dh11*width_1*height_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h12h11*width_m3*height_3+
					dh01dh02*dh12dh11*width_m1*height_1+
					h01h02*d2h12d2h11*width_1*height_m1));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh01dh12*h12h11*height_3+
					h01h12*dh12dh11*width_2*height_1)+
					plate_bending_smoothing*
					(d2h01d2h12*h12h11*width_m2*height_3+
					dh01dh12*dh12dh11*height_1+
					h01h12*d2h12d2h11*width_2*height_m1));
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh11dh02*h12h01*height_2+
					h11h02*dh12dh01*width_2)+
					plate_bending_smoothing*
					(d2h11d2h02*h12h01*width_m2*height_2+
					dh11dh02*dh12dh01+
					h11h02*d2h12d2h01*width_2*height_m2));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh11dh12*h12h01*width_1*height_2+
					h11h12*dh12dh01*width_3)+
					plate_bending_smoothing*
					(d2h11d2h12*h12h01*width_m1*height_2+
					dh11dh12*dh12dh01*width_1+
					h11h12*d2h12d2h01*width_3*height_m2));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh11dh02*h12h11*height_3+
					h11h02*dh12dh11*width_2*height_1)+
					plate_bending_smoothing*
					(d2h11d2h02*h12h11*width_m2*height_3+
					dh11dh02*dh12dh11*height_1+
					h11h02*d2h12d2h11*width_2*height_m1));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh11dh12*h12h11*width_1*height_3+
					h11h12*dh12dh11*width_3*height_1)+
					plate_bending_smoothing*
					(d2h11d2h12*h12h11*width_m1*height_3+
					dh11dh12*dh12dh11*width_1*height_1+
					h11h12*d2h12d2h11*width_3*height_m1));
#if defined (FIX_APEX)
			}
#endif /* defined (FIX_APEX) */
#if defined (FIX_APEX)
			if ((SOCK==type)&&(im1_jm1==0))
			{
				/* d(i_j)/d(im1_jm1) */
				temp=matrix_and_right_hand_side+i_j;
				/* d/d(f) */
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh01dh02*h01h02*width_m1*height_1+
					h01h02*dh01dh02*width_1*height_m1)+
					plate_bending_smoothing*
					(d2h01d2h02*h01h02*width_m3*height_1+
					dh01dh02*dh01dh02*width_m1*height_m1+
					h01h02*d2h01d2h02*width_1*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh01dh12*h01h02*height_1+
					h01h12*dh01dh02*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h01d2h12*h01h02*width_m2*height_1+
					dh01dh12*dh01dh02*height_m1+
					h01h12*d2h01d2h02*width_2*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh01dh02*h01h12*width_m1*height_2+
					h01h02*dh01dh12*width_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h01h12*width_m3*height_2+
					dh01dh02*dh01dh12*width_m1+
					h01h02*d2h01d2h12*width_1*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh01dh12*h01h12*height_2+
					h01h12*dh01dh12*width_2)+
					plate_bending_smoothing*
					(d2h01d2h12*h01h12*width_m2*height_2+
					dh01dh12*dh01dh12+
					h01h12*d2h01d2h12*width_2*height_m2));
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += cos_0*
					(membrane_smoothing*
					(dh01dh02*h11h02*width_m1*height_2+
					h01h02*dh11dh02*width_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h11h02*width_m3*height_2+
					dh01dh02*dh11dh02*width_m1+
					h01h02*d2h11d2h02*width_1*height_m2));
				/* dfdx */
				temp[1] += cos_0*
					(membrane_smoothing*
					(dh01dh12*h11h02*height_2+
					h01h12*dh11dh02*width_2)+
					plate_bending_smoothing*
					(d2h01d2h12*h11h02*width_m2*height_2+
					dh01dh12*dh11dh02+
					h01h12*d2h11d2h02*width_2*height_m2));
				/* dfdy */
				temp[2] += cos_0*
					(membrane_smoothing*
					(dh01dh02*h11h12*width_m1*height_3+
					h01h02*dh11dh12*width_1*height_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h11h12*width_m3*height_3+
					dh01dh02*dh11dh12*width_m1*height_1+
					h01h02*d2h11d2h12*width_1*height_m1));
				/* d2fdxdy */
				temp[3] += cos_0*
					(membrane_smoothing*
					(dh01dh12*h11h12*height_3+
					h01h12*dh11dh12*width_2*height_1)+
					plate_bending_smoothing*
					(d2h01d2h12*h11h12*width_m2*height_3+
					dh01dh12*dh11dh12*height_1+
					h01h12*d2h11d2h12*width_2*height_m1));
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += sin_0*
					(membrane_smoothing*
					(dh01dh02*h11h02*width_m1*height_2+
					h01h02*dh11dh02*width_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h11h02*width_m3*height_2+
					dh01dh02*dh11dh02*width_m1+
					h01h02*d2h11d2h02*width_1*height_m2));
				/* dfdx */
				temp[1] += sin_0*
					(membrane_smoothing*
					(dh01dh12*h11h02*height_2+
					h01h12*dh11dh02*width_2)+
					plate_bending_smoothing*
					(d2h01d2h12*h11h02*width_m2*height_2+
					dh01dh12*dh11dh02+
					h01h12*d2h11d2h02*width_2*height_m2));
				/* dfdy */
				temp[2] += sin_0*
					(membrane_smoothing*
					(dh01dh02*h11h12*width_m1*height_3+
					h01h02*dh11dh12*width_1*height_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h11h12*width_m3*height_3+
					dh01dh02*dh11dh12*width_m1*height_1+
					h01h02*d2h11d2h12*width_1*height_m1));
				/* d2fdxdy */
				temp[3] += sin_0*
					(membrane_smoothing*
					(dh01dh12*h11h12*height_3+
					h01h12*dh11dh12*width_2*height_1)+
					plate_bending_smoothing*
					(d2h01d2h12*h11h12*width_m2*height_3+
					dh01dh12*dh11dh12*height_1+
					h01h12*d2h11d2h12*width_2*height_m1));
				/* d(i_j)/d(im1_j) */
				temp=matrix_and_right_hand_side+i_j;
				/* d/d(f) */
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh02dh02*h01h02*width_m1*height_1+
					h02h02*dh01dh02*width_1*height_m1)+
					plate_bending_smoothing*
					(d2h02d2h02*h01h02*width_m3*height_1+
					dh02dh02*dh01dh02*width_m1*height_m1+
					h02h02*d2h01d2h02*width_1*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh02dh12*h01h02*height_1+
					h02h12*dh01dh02*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h02d2h12*h01h02*width_m2*height_1+
					dh02dh12*dh01dh02*height_m1+
					h02h12*d2h01d2h02*width_2*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh02dh02*h01h12*width_m1*height_2+
					h02h02*dh01dh12*width_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h01h12*width_m3*height_2+
					dh02dh02*dh01dh12*width_m1+
					h02h02*d2h01d2h12*width_1*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh02dh12*h01h12*height_2+
					h02h12*dh01dh12*width_2)+
					plate_bending_smoothing*
					(d2h02d2h12*h01h12*width_m2*height_2+
					dh02dh12*dh01dh12+
					h02h12*d2h01d2h12*width_2*height_m2));
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += cos_1*
					(membrane_smoothing*
					(dh02dh02*h11h02*width_m1*height_2+
					h02h02*dh11dh02*width_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h11h02*width_m3*height_2+
					dh02dh02*dh11dh02*width_m1+
					h02h02*d2h11d2h02*width_1*height_m2));
				/* dfdx */
				temp[1] += cos_1*
					(membrane_smoothing*
					(dh02dh12*h11h02*height_2+
					h02h12*dh11dh02*width_2)+
					plate_bending_smoothing*
					(d2h02d2h12*h11h02*width_m2*height_2+
					dh02dh12*dh11dh02+
					h02h12*d2h11d2h02*width_2*height_m2));
				/* dfdy */
				temp[2] += cos_1*
					(membrane_smoothing*
					(dh02dh02*h11h12*width_m1*height_3+
					h02h02*dh11dh12*width_1*height_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h11h12*width_m3*height_3+
					dh02dh02*dh11dh12*width_m1*height_1+
					h02h02*d2h11d2h12*width_1*height_m1));
				/* d2fdxdy */
				temp[3] += cos_1*
					(membrane_smoothing*
					(dh02dh12*h11h12*height_3+
					h02h12*dh11dh12*width_2*height_1)+
					plate_bending_smoothing*
					(d2h02d2h12*h11h12*width_m2*height_3+
					dh02dh12*dh11dh12*height_1+
					h02h12*d2h11d2h12*width_2*height_m1));
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += sin_1*
					(membrane_smoothing*
					(dh02dh02*h11h02*width_m1*height_2+
					h02h02*dh11dh02*width_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h11h02*width_m3*height_2+
					dh02dh02*dh11dh02*width_m1+
					h02h02*d2h11d2h02*width_1*height_m2));
				/* dfdx */
				temp[1] += sin_1*
					(membrane_smoothing*
					(dh02dh12*h11h02*height_2+
					h02h12*dh11dh02*width_2)+
					plate_bending_smoothing*
					(d2h02d2h12*h11h02*width_m2*height_2+
					dh02dh12*dh11dh02+
					h02h12*d2h11d2h02*width_2*height_m2));
				/* dfdy */
				temp[2] += sin_1*
					(membrane_smoothing*
					(dh02dh02*h11h12*width_m1*height_3+
					h02h02*dh11dh12*width_1*height_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h11h12*width_m3*height_3+
					dh02dh02*dh11dh12*width_m1*height_1+
					h02h02*d2h11d2h12*width_1*height_m1));
				/* d2fdxdy */
				temp[3] += sin_1*
					(membrane_smoothing*
					(dh02dh12*h11h12*height_3+
					h02h12*dh11dh12*width_2*height_1)+
					plate_bending_smoothing*
					(d2h02d2h12*h11h12*width_m2*height_3+
					dh02dh12*dh11dh12*height_1+
					h02h12*d2h11d2h12*width_2*height_m1));
			}
			else
			{
#endif /* defined (FIX_APEX) */
				/* d(i_j)/d(im1_jm1) */
				temp=matrix_and_right_hand_side+
					im1_jm1*number_of_equations_plus_1+i_j;
				/* d/d(f) */
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh01dh02*h01h02*width_m1*height_1+
					h01h02*dh01dh02*width_1*height_m1)+
					plate_bending_smoothing*
					(d2h01d2h02*h01h02*width_m3*height_1+
					dh01dh02*dh01dh02*width_m1*height_m1+
					h01h02*d2h01d2h02*width_1*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh01dh12*h01h02*height_1+
					h01h12*dh01dh02*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h01d2h12*h01h02*width_m2*height_1+
					dh01dh12*dh01dh02*height_m1+
					h01h12*d2h01d2h02*width_2*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh01dh02*h01h12*width_m1*height_2+
					h01h02*dh01dh12*width_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h01h12*width_m3*height_2+
					dh01dh02*dh01dh12*width_m1+
					h01h02*d2h01d2h12*width_1*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh01dh12*h01h12*height_2+
					h01h12*dh01dh12*width_2)+
					plate_bending_smoothing*
					(d2h01d2h12*h01h12*width_m2*height_2+
					dh01dh12*dh01dh12+
					h01h12*d2h01d2h12*width_2*height_m2));
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh11dh02*h01h02*height_1+
					h11h02*dh01dh02*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h11d2h02*h01h02*width_m2*height_1+
					dh11dh02*dh01dh02*height_m1+
					h11h02*d2h01d2h02*width_2*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh11dh12*h01h02*width_1*height_1+
					h11h12*dh01dh02*width_3*height_m1)+
					plate_bending_smoothing*
					(d2h11d2h12*h01h02*width_m1*height_1+
					dh11dh12*dh01dh02*width_1*height_m1+
					h11h12*d2h01d2h02*width_3*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh11dh02*h01h12*height_2+
					h11h02*dh01dh12*width_2)+
					plate_bending_smoothing*
					(d2h11d2h02*h01h12*width_m2*height_2+
					dh11dh02*dh01dh12+
					h11h02*d2h01d2h12*width_2*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh11dh12*h01h12*width_1*height_2+
					h11h12*dh01dh12*width_3)+
					plate_bending_smoothing*
					(d2h11d2h12*h01h12*width_m1*height_2+
					dh11dh12*dh01dh12*width_1+
					h11h12*d2h01d2h12*width_3*height_m2));
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh01dh02*h11h02*width_m1*height_2+
					h01h02*dh11dh02*width_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h11h02*width_m3*height_2+
					dh01dh02*dh11dh02*width_m1+
					h01h02*d2h11d2h02*width_1*height_m2));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh01dh12*h11h02*height_2+
					h01h12*dh11dh02*width_2)+
					plate_bending_smoothing*
					(d2h01d2h12*h11h02*width_m2*height_2+
					dh01dh12*dh11dh02+
					h01h12*d2h11d2h02*width_2*height_m2));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh01dh02*h11h12*width_m1*height_3+
					h01h02*dh11dh12*width_1*height_1)+
					plate_bending_smoothing*
					(d2h01d2h02*h11h12*width_m3*height_3+
					dh01dh02*dh11dh12*width_m1*height_1+
					h01h02*d2h11d2h12*width_1*height_m1));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh01dh12*h11h12*height_3+
					h01h12*dh11dh12*width_2*height_1)+
					plate_bending_smoothing*
					(d2h01d2h12*h11h12*width_m2*height_3+
					dh01dh12*dh11dh12*height_1+
					h01h12*d2h11d2h12*width_2*height_m1));
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh11dh02*h11h02*height_2+
					h11h02*dh11dh02*width_2)+
					plate_bending_smoothing*
					(d2h11d2h02*h11h02*width_m2*height_2+
					dh11dh02*dh11dh02+
					h11h02*d2h11d2h02*width_2*height_m2));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh11dh12*h11h02*width_1*height_2+
					h11h12*dh11dh02*width_3)+
					plate_bending_smoothing*
					(d2h11d2h12*h11h02*width_m1*height_2+
					dh11dh12*dh11dh02*width_1+
					h11h12*d2h11d2h02*width_3*height_m2));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh11dh02*h11h12*height_3+
					h11h02*dh11dh12*width_2*height_1)+
					plate_bending_smoothing*
					(d2h11d2h02*h11h12*width_m2*height_3+
					dh11dh02*dh11dh12*height_1+
					h11h02*d2h11d2h12*width_2*height_m1));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh11dh12*h11h12*width_1*height_3+
					h11h12*dh11dh12*width_3*height_1)+
					plate_bending_smoothing*
					(d2h11d2h12*h11h12*width_m1*height_3+
					dh11dh12*dh11dh12*width_1*height_1+
					h11h12*d2h11d2h12*width_3*height_m1));
				/* d(i_j)/d(im1_j) */
				temp=matrix_and_right_hand_side+
					im1_j*number_of_equations_plus_1+i_j;
				/* d/d(f) */
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh02dh02*h01h02*width_m1*height_1+
					h02h02*dh01dh02*width_1*height_m1)+
					plate_bending_smoothing*
					(d2h02d2h02*h01h02*width_m3*height_1+
					dh02dh02*dh01dh02*width_m1*height_m1+
					h02h02*d2h01d2h02*width_1*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh02dh12*h01h02*height_1+
					h02h12*dh01dh02*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h02d2h12*h01h02*width_m2*height_1+
					dh02dh12*dh01dh02*height_m1+
					h02h12*d2h01d2h02*width_2*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh02dh02*h01h12*width_m1*height_2+
					h02h02*dh01dh12*width_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h01h12*width_m3*height_2+
					dh02dh02*dh01dh12*width_m1+
					h02h02*d2h01d2h12*width_1*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh02dh12*h01h12*height_2+
					h02h12*dh01dh12*width_2)+
					plate_bending_smoothing*
					(d2h02d2h12*h01h12*width_m2*height_2+
					dh02dh12*dh01dh12+
					h02h12*d2h01d2h12*width_2*height_m2));
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh12dh02*h01h02*height_1+
					h12h02*dh01dh02*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h12d2h02*h01h02*width_m2*height_1+
					dh12dh02*dh01dh02*height_m1+
					h12h02*d2h01d2h02*width_2*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh12dh12*h01h02*width_1*height_1+
					h12h12*dh01dh02*width_3*height_m1)+
					plate_bending_smoothing*
					(d2h12d2h12*h01h02*width_m1*height_1+
					dh12dh12*dh01dh02*width_1*height_m1+
					h12h12*d2h01d2h02*width_3*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh12dh02*h01h12*height_2+
					h12h02*dh01dh12*width_2)+
					plate_bending_smoothing*
					(d2h12d2h02*h01h12*width_m2*height_2+
					dh12dh02*dh01dh12+
					h12h02*d2h01d2h12*width_2*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh12dh12*h01h12*width_1*height_2+
					h12h12*dh01dh12*width_3)+
					plate_bending_smoothing*
					(d2h12d2h12*h01h12*width_m1*height_2+
					dh12dh12*dh01dh12*width_1+
					h12h12*d2h01d2h12*width_3*height_m2));
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh02dh02*h11h02*width_m1*height_2+
					h02h02*dh11dh02*width_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h11h02*width_m3*height_2+
					dh02dh02*dh11dh02*width_m1+
					h02h02*d2h11d2h02*width_1*height_m2));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh02dh12*h11h02*height_2+
					h02h12*dh11dh02*width_2)+
					plate_bending_smoothing*
					(d2h02d2h12*h11h02*width_m2*height_2+
					dh02dh12*dh11dh02+
					h02h12*d2h11d2h02*width_2*height_m2));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh02dh02*h11h12*width_m1*height_3+
					h02h02*dh11dh12*width_1*height_1)+
					plate_bending_smoothing*
					(d2h02d2h02*h11h12*width_m3*height_3+
					dh02dh02*dh11dh12*width_m1*height_1+
					h02h02*d2h11d2h12*width_1*height_m1));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh02dh12*h11h12*height_3+
					h02h12*dh11dh12*width_2*height_1)+
					plate_bending_smoothing*
					(d2h02d2h12*h11h12*width_m2*height_3+
					dh02dh12*dh11dh12*height_1+
					h02h12*d2h11d2h12*width_2*height_m1));
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh12dh02*h11h02*height_2+
					h12h02*dh11dh02*width_2)+
					plate_bending_smoothing*
					(d2h12d2h02*h11h02*width_m2*height_2+
					dh12dh02*dh11dh02+
					h12h02*d2h11d2h02*width_2*height_m2));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh12dh12*h11h02*width_1*height_2+
					h12h12*dh11dh02*width_3)+
					plate_bending_smoothing*
					(d2h12d2h12*h11h02*width_m1*height_2+
					dh12dh12*dh11dh02*width_1+
					h12h12*d2h11d2h02*width_3*height_m2));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh12dh02*h11h12*height_3+
					h12h02*dh11dh12*width_2*height_1)+
					plate_bending_smoothing*
					(d2h12d2h02*h11h12*width_m2*height_3+
					dh12dh02*dh11dh12*height_1+
					h12h02*d2h11d2h12*width_2*height_m1));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh12dh12*h11h12*width_1*height_3+
					h12h12*dh11dh12*width_3*height_1)+
					plate_bending_smoothing*
					(d2h12d2h12*h11h12*width_m1*height_3+
					dh12dh12*dh11dh12*width_1*height_1+
					h12h12*d2h11d2h12*width_3*height_m1));
#if defined (FIX_APEX)
			}
#endif /* defined (FIX_APEX) */
			/* d(i_j)/d(i_j) */
			temp=matrix_and_right_hand_side+
				i_j*number_of_equations_plus_1+i_j;
			/* d/d(f) */
			/* f */
			*temp += 
				(membrane_smoothing*
				(dh02dh02*h02h02*width_m1*height_1+
				h02h02*dh02dh02*width_1*height_m1)+
				plate_bending_smoothing*
				(d2h02d2h02*h02h02*width_m3*height_1+
				dh02dh02*dh02dh02*width_m1*height_m1+
				h02h02*d2h02d2h02*width_1*height_m3));
			/* dfdx */
			temp[1] += 
				(membrane_smoothing*
				(dh02dh12*h02h02*height_1+
				h02h12*dh02dh02*width_2*height_m1)+
				plate_bending_smoothing*
				(d2h02d2h12*h02h02*width_m2*height_1+
				dh02dh12*dh02dh02*height_m1+
				h02h12*d2h02d2h02*width_2*height_m3));
			/* dfdy */
			temp[2] += 
				(membrane_smoothing*
				(dh02dh02*h02h12*width_m1*height_2+
				h02h02*dh02dh12*width_1)+
				plate_bending_smoothing*
				(d2h02d2h02*h02h12*width_m3*height_2+
				dh02dh02*dh02dh12*width_m1+
				h02h02*d2h02d2h12*width_1*height_m2));
			/* d2fdxdy */
			temp[3] += 
				(membrane_smoothing*
				(dh02dh12*h02h12*height_2+
				h02h12*dh02dh12*width_2)+
				plate_bending_smoothing*
				(d2h02d2h12*h02h12*width_m2*height_2+
				dh02dh12*dh02dh12+
				h02h12*d2h02d2h12*width_2*height_m2));
			/* d/d(dfdx) */
			temp += number_of_equations_plus_1;
			/* f */
			*temp += 
				(membrane_smoothing*
				(dh12dh02*h02h02*height_1+
				h12h02*dh02dh02*width_2*height_m1)+
				plate_bending_smoothing*
				(d2h12d2h02*h02h02*width_m2*height_1+
				dh12dh02*dh02dh02*height_m1+
				h12h02*d2h02d2h02*width_2*height_m3));
			/* dfdx */
			temp[1] += 
				(membrane_smoothing*
				(dh12dh12*h02h02*width_1*height_1+
				h12h12*dh02dh02*width_3*height_m1)+
				plate_bending_smoothing*
				(d2h12d2h12*h02h02*width_m1*height_1+
				dh12dh12*dh02dh02*width_1*height_m1+
				h12h12*d2h02d2h02*width_3*height_m3));
			/* dfdy */
			temp[2] += 
				(membrane_smoothing*
				(dh12dh02*h02h12*height_2+
				h12h02*dh02dh12*width_2)+
				plate_bending_smoothing*
				(d2h12d2h02*h02h12*width_m2*height_2+
				dh12dh02*dh02dh12+
				h12h02*d2h02d2h12*width_2*height_m2));
			/* d2fdxdy */
			temp[3] += 
				(membrane_smoothing*
				(dh12dh12*h02h12*width_1*height_2+
				h12h12*dh02dh12*width_3)+
				plate_bending_smoothing*
				(d2h12d2h12*h02h12*width_m1*height_2+
				dh12dh12*dh02dh12*width_1+
				h12h12*d2h02d2h12*width_3*height_m2));
			/* d/d(dfdy) */
			temp += number_of_equations_plus_1;
			/* f */
			*temp += 
				(membrane_smoothing*
				(dh02dh02*h12h02*width_m1*height_2+
				h02h02*dh12dh02*width_1)+
				plate_bending_smoothing*
				(d2h02d2h02*h12h02*width_m3*height_2+
				dh02dh02*dh12dh02*width_m1+
				h02h02*d2h12d2h02*width_1*height_m2));
			/* dfdx */
			temp[1] += 
				(membrane_smoothing*
				(dh02dh12*h12h02*height_2+
				h02h12*dh12dh02*width_2)+
				plate_bending_smoothing*
				(d2h02d2h12*h12h02*width_m2*height_2+
				dh02dh12*dh12dh02+
				h02h12*d2h12d2h02*width_2*height_m2));
			/* dfdy */
			temp[2] += 
				(membrane_smoothing*
				(dh02dh02*h12h12*width_m1*height_3+
				h02h02*dh12dh12*width_1*height_1)+
				plate_bending_smoothing*
				(d2h02d2h02*h12h12*width_m3*height_3+
				dh02dh02*dh12dh12*width_m1*height_1+
				h02h02*d2h12d2h12*width_1*height_m1));
			/* d2fdxdy */
			temp[3] += 
				(membrane_smoothing*
				(dh02dh12*h12h12*height_3+
				h02h12*dh12dh12*width_2*height_1)+
				plate_bending_smoothing*
				(d2h02d2h12*h12h12*width_m2*height_3+
				dh02dh12*dh12dh12*height_1+
				h02h12*d2h12d2h12*width_2*height_m1));
			/* d/d(d2fdxdy) */
			temp += number_of_equations_plus_1;
			/* f */
			*temp += 
				(membrane_smoothing*
				(dh12dh02*h12h02*height_2+
				h12h02*dh12dh02*width_2)+
				plate_bending_smoothing*
				(d2h12d2h02*h12h02*width_m2*height_2+
				dh12dh02*dh12dh02+
				h12h02*d2h12d2h02*width_2*height_m2));
			/* dfdx */
			temp[1] += 
				(membrane_smoothing*
				(dh12dh12*h12h02*width_1*height_2+
				h12h12*dh12dh02*width_3)+
				plate_bending_smoothing*
				(d2h12d2h12*h12h02*width_m1*height_2+
				dh12dh12*dh12dh02*width_1+
				h12h12*d2h12d2h02*width_3*height_m2));
			/* dfdy */
			temp[2] += 
				(membrane_smoothing*
				(dh12dh02*h12h12*height_3+
				h12h02*dh12dh12*width_2*height_1)+
				plate_bending_smoothing*
				(d2h12d2h02*h12h12*width_m2*height_3+
				dh12dh02*dh12dh12*height_1+
				h12h02*d2h12d2h12*width_2*height_m1));
			/* d2fdxdy */
			temp[3] += 
				(membrane_smoothing*
				(dh12dh12*h12h12*width_1*height_3+
				h12h12*dh12dh12*width_3*height_1)+
				plate_bending_smoothing*
				(d2h12d2h12*h12h12*width_m1*height_3+
				dh12dh12*dh12dh12*width_1*height_1+
				h12h12*d2h12d2h12*width_3*height_m1));
			/* d(i_j)/d(i_jm1) */
			temp=matrix_and_right_hand_side+
				i_jm1*number_of_equations_plus_1+i_j;
			/* d/d(f) */
			/* f */
			*temp += 
				(membrane_smoothing*
				(dh01dh02*h02h02*width_m1*height_1+
				h01h02*dh02dh02*width_1*height_m1)+
				plate_bending_smoothing*
				(d2h01d2h02*h02h02*width_m3*height_1+
				dh01dh02*dh02dh02*width_m1*height_m1+
				h01h02*d2h02d2h02*width_1*height_m3));
			/* dfdx */
			temp[1] += 
				(membrane_smoothing*
				(dh01dh12*h02h02*height_1+
				h01h12*dh02dh02*width_2*height_m1)+
				plate_bending_smoothing*
				(d2h01d2h12*h02h02*width_m2*height_1+
				dh01dh12*dh02dh02*height_m1+
				h01h12*d2h02d2h02*width_2*height_m3));
			/* dfdy */
			temp[2] += 
				(membrane_smoothing*
				(dh01dh02*h02h12*width_m1*height_2+
				h01h02*dh02dh12*width_1)+
				plate_bending_smoothing*
				(d2h01d2h02*h02h12*width_m3*height_2+
				dh01dh02*dh02dh12*width_m1+
				h01h02*d2h02d2h12*width_1*height_m2));
			/* d2fdxdy */
			temp[3] += 
				(membrane_smoothing*
				(dh01dh12*h02h12*height_2+
				h01h12*dh02dh12*width_2)+
				plate_bending_smoothing*
				(d2h01d2h12*h02h12*width_m2*height_2+
				dh01dh12*dh02dh12+
				h01h12*d2h02d2h12*width_2*height_m2));
			/* d/d(dfdx) */
			temp += number_of_equations_plus_1;
			/* f */
			*temp += 
				(membrane_smoothing*
				(dh11dh02*h02h02*height_1+
				h11h02*dh02dh02*width_2*height_m1)+
				plate_bending_smoothing*
				(d2h11d2h02*h02h02*width_m2*height_1+
				dh11dh02*dh02dh02*height_m1+
				h11h02*d2h02d2h02*width_2*height_m3));
			/* dfdx */
			temp[1] += 
				(membrane_smoothing*
				(dh11dh12*h02h02*width_1*height_1+
				h11h12*dh02dh02*width_3*height_m1)+
				plate_bending_smoothing*
				(d2h11d2h12*h02h02*width_m1*height_1+
				dh11dh12*dh02dh02*width_1*height_m1+
				h11h12*d2h02d2h02*width_3*height_m3));
			/* dfdy */
			temp[2] += 
				(membrane_smoothing*
				(dh11dh02*h02h12*height_2+
				h11h02*dh02dh12*width_2)+
				plate_bending_smoothing*
				(d2h11d2h02*h02h12*width_m2*height_2+
				dh11dh02*dh02dh12+
				h11h02*d2h02d2h12*width_2*height_m2));
			/* d2fdxdy */
			temp[3] += 
				(membrane_smoothing*
				(dh11dh12*h02h12*width_1*height_2+
				h11h12*dh02dh12*width_3)+
				plate_bending_smoothing*
				(d2h11d2h12*h02h12*width_m1*height_2+
				dh11dh12*dh02dh12*width_1+
				h11h12*d2h02d2h12*width_3*height_m2));
			/* d/d(dfdy) */
			temp += number_of_equations_plus_1;
			/* f */
			*temp += 
				(membrane_smoothing*
				(dh01dh02*h12h02*width_m1*height_2+
				h01h02*dh12dh02*width_1)+
				plate_bending_smoothing*
				(d2h01d2h02*h12h02*width_m3*height_2+
				dh01dh02*dh12dh02*width_m1+
				h01h02*d2h12d2h02*width_1*height_m2));
			/* dfdx */
			temp[1] += 
				(membrane_smoothing*
				(dh01dh12*h12h02*height_2+
				h01h12*dh12dh02*width_2)+
				plate_bending_smoothing*
				(d2h01d2h12*h12h02*width_m2*height_2+
				dh01dh12*dh12dh02+
				h01h12*d2h12d2h02*width_2*height_m2));
			/* dfdy */
			temp[2] += 
				(membrane_smoothing*
				(dh01dh02*h12h12*width_m1*height_3+
				h01h02*dh12dh12*width_1*height_1)+
				plate_bending_smoothing*
				(d2h01d2h02*h12h12*width_m3*height_3+
				dh01dh02*dh12dh12*width_m1*height_1+
				h01h02*d2h12d2h12*width_1*height_m1));
			/* d2fdxdy */
			temp[3] += 
				(membrane_smoothing*
				(dh01dh12*h12h12*height_3+
				h01h12*dh12dh12*width_2*height_1)+
				plate_bending_smoothing*
				(d2h01d2h12*h12h12*width_m2*height_3+
				dh01dh12*dh12dh12*height_1+
				h01h12*d2h12d2h12*width_2*height_m1));
			/* d/d(d2fdxdy) */
			temp += number_of_equations_plus_1;
			/* f */
			*temp += 
				(membrane_smoothing*
				(dh11dh02*h12h02*height_2+
				h11h02*dh12dh02*width_2)+
				plate_bending_smoothing*
				(d2h11d2h02*h12h02*width_m2*height_2+
				dh11dh02*dh12dh02+
				h11h02*d2h12d2h02*width_2*height_m2));
			/* dfdx */
			temp[1] += 
				(membrane_smoothing*
				(dh11dh12*h12h02*width_1*height_2+
				h11h12*dh12dh02*width_3)+
				plate_bending_smoothing*
				(d2h11d2h12*h12h02*width_m1*height_2+
				dh11dh12*dh12dh02*width_1+
				h11h12*d2h12d2h02*width_3*height_m2));
			/* dfdy */
			temp[2] += 
				(membrane_smoothing*
				(dh11dh02*h12h12*height_3+
				h11h02*dh12dh12*width_2*height_1)+
				plate_bending_smoothing*
				(d2h11d2h02*h12h12*width_m2*height_3+
				dh11dh02*dh12dh12*height_1+
				h11h02*d2h12d2h12*width_2*height_m1));
			/* d2fdxdy */
			temp[3] += 
				(membrane_smoothing*
				(dh11dh12*h12h12*width_1*height_3+
				h11h12*dh12dh12*width_3*height_1)+
				plate_bending_smoothing*
				(d2h11d2h12*h12h12*width_m1*height_3+
				dh11dh12*dh12dh12*width_1*height_1+
				h11h12*d2h12d2h12*width_3*height_m1));
#if defined (FIX_APEX)
			if ((SOCK==type)&&(im1_jm1==0))
			{
				/* d(i_jm1)/d(im1_jm1) */
				temp=matrix_and_right_hand_side+i_jm1;
				/* d/d(f) */
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh01dh01*h01h02*width_m1*height_1+
					h01h01*dh01dh02*width_1*height_m1)+
					plate_bending_smoothing*
					(d2h01d2h01*h01h02*width_m3*height_1+
					dh01dh01*dh01dh02*width_m1*height_m1+
					h01h01*d2h01d2h02*width_1*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh01dh11*h01h02*height_1+
					h01h11*dh01dh02*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h01d2h11*h01h02*width_m2*height_1+
					dh01dh11*dh01dh02*height_m1+
					h01h11*d2h01d2h02*width_2*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh01dh01*h01h12*width_m1*height_2+
					h01h01*dh01dh12*width_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h01h12*width_m3*height_2+
					dh01dh01*dh01dh12*width_m1+
					h01h01*d2h01d2h12*width_1*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh01dh11*h01h12*height_2+
					h01h11*dh01dh12*width_2)+
					plate_bending_smoothing*
					(d2h01d2h11*h01h12*width_m2*height_2+
					dh01dh11*dh01dh12+
					h01h11*d2h01d2h12*width_2*height_m2));
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += cos_0*
					(membrane_smoothing*
					(dh01dh01*h11h02*width_m1*height_2+
					h01h01*dh11dh02*width_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h11h02*width_m3*height_2+
					dh01dh01*dh11dh02*width_m1+
					h01h01*d2h11d2h02*width_1*height_m2));
				/* dfdx */
				temp[1] += cos_0*
					(membrane_smoothing*
					(dh01dh11*h11h02*height_2+
					h01h11*dh11dh02*width_2)+
					plate_bending_smoothing*
					(d2h01d2h11*h11h02*width_m2*height_2+
					dh01dh11*dh11dh02+
					h01h11*d2h11d2h02*width_2*height_m2));
				/* dfdy */
				temp[2] += cos_0*
					(membrane_smoothing*
					(dh01dh01*h11h12*width_m1*height_3+
					h01h01*dh11dh12*width_1*height_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h11h12*width_m3*height_3+
					dh01dh01*dh11dh12*width_m1*height_1+
					h01h01*d2h11d2h12*width_1*height_m1));
				/* d2fdxdy */
				temp[3] += cos_0*
					(membrane_smoothing*
					(dh01dh11*h11h12*height_3+
					h01h11*dh11dh12*width_2*height_1)+
					plate_bending_smoothing*
					(d2h01d2h11*h11h12*width_m2*height_3+
					dh01dh11*dh11dh12*height_1+
					h01h11*d2h11d2h12*width_2*height_m1));
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += sin_0*
					(membrane_smoothing*
					(dh01dh01*h11h02*width_m1*height_2+
					h01h01*dh11dh02*width_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h11h02*width_m3*height_2+
					dh01dh01*dh11dh02*width_m1+
					h01h01*d2h11d2h02*width_1*height_m2));
				/* dfdx */
				temp[1] += sin_0*
					(membrane_smoothing*
					(dh01dh11*h11h02*height_2+
					h01h11*dh11dh02*width_2)+
					plate_bending_smoothing*
					(d2h01d2h11*h11h02*width_m2*height_2+
					dh01dh11*dh11dh02+
					h01h11*d2h11d2h02*width_2*height_m2));
				/* dfdy */
				temp[2] += sin_0*
					(membrane_smoothing*
					(dh01dh01*h11h12*width_m1*height_3+
					h01h01*dh11dh12*width_1*height_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h11h12*width_m3*height_3+
					dh01dh01*dh11dh12*width_m1*height_1+
					h01h01*d2h11d2h12*width_1*height_m1));
				/* d2fdxdy */
				temp[3] += sin_0*
					(membrane_smoothing*
					(dh01dh11*h11h12*height_3+
					h01h11*dh11dh12*width_2*height_1)+
					plate_bending_smoothing*
					(d2h01d2h11*h11h12*width_m2*height_3+
					dh01dh11*dh11dh12*height_1+
					h01h11*d2h11d2h12*width_2*height_m1));
				/* d(i_jm1)/d(im1_j) */
				temp=matrix_and_right_hand_side+i_jm1;
				/* d/d(f) */
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh02dh01*h01h02*width_m1*height_1+
					h02h01*dh01dh02*width_1*height_m1)+
					plate_bending_smoothing*
					(d2h02d2h01*h01h02*width_m3*height_1+
					dh02dh01*dh01dh02*width_m1*height_m1+
					h02h01*d2h01d2h02*width_1*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh02dh11*h01h02*height_1+
					h02h11*dh01dh02*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h02d2h11*h01h02*width_m2*height_1+
					dh02dh11*dh01dh02*height_m1+
					h02h11*d2h01d2h02*width_2*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh02dh01*h01h12*width_m1*height_2+
					h02h01*dh01dh12*width_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h01h12*width_m3*height_2+
					dh02dh01*dh01dh12*width_m1+
					h02h01*d2h01d2h12*width_1*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh02dh11*h01h12*height_2+
					h02h11*dh01dh12*width_2)+
					plate_bending_smoothing*
					(d2h02d2h11*h01h12*width_m2*height_2+
					dh02dh11*dh01dh12+
					h02h11*d2h01d2h12*width_2*height_m2));
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += cos_1*
					(membrane_smoothing*
					(dh02dh01*h11h02*width_m1*height_2+
					h02h01*dh11dh02*width_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h11h02*width_m3*height_2+
					dh02dh01*dh11dh02*width_m1+
					h02h01*d2h11d2h02*width_1*height_m2));
				/* dfdx */
				temp[1] += cos_1*
					(membrane_smoothing*
					(dh02dh11*h11h02*height_2+
					h02h11*dh11dh02*width_2)+
					plate_bending_smoothing*
					(d2h02d2h11*h11h02*width_m2*height_2+
					dh02dh11*dh11dh02+
					h02h11*d2h11d2h02*width_2*height_m2));
				/* dfdy */
				temp[2] += cos_1*
					(membrane_smoothing*
					(dh02dh01*h11h12*width_m1*height_3+
					h02h01*dh11dh12*width_1*height_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h11h12*width_m3*height_3+
					dh02dh01*dh11dh12*width_m1*height_1+
					h02h01*d2h11d2h12*width_1*height_m1));
				/* d2fdxdy */
				temp[3] += cos_1*
					(membrane_smoothing*
					(dh02dh11*h11h12*height_3+
					h02h11*dh11dh12*width_2*height_1)+
					plate_bending_smoothing*
					(d2h02d2h11*h11h12*width_m2*height_3+
					dh02dh11*dh11dh12*height_1+
					h02h11*d2h11d2h12*width_2*height_m1));
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += sin_1*
					(membrane_smoothing*
					(dh02dh01*h11h02*width_m1*height_2+
					h02h01*dh11dh02*width_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h11h02*width_m3*height_2+
					dh02dh01*dh11dh02*width_m1+
					h02h01*d2h11d2h02*width_1*height_m2));
				/* dfdx */
				temp[1] += sin_1*
					(membrane_smoothing*
					(dh02dh11*h11h02*height_2+
					h02h11*dh11dh02*width_2)+
					plate_bending_smoothing*
					(d2h02d2h11*h11h02*width_m2*height_2+
					dh02dh11*dh11dh02+
					h02h11*d2h11d2h02*width_2*height_m2));
				/* dfdy */
				temp[2] += sin_1*
					(membrane_smoothing*
					(dh02dh01*h11h12*width_m1*height_3+
					h02h01*dh11dh12*width_1*height_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h11h12*width_m3*height_3+
					dh02dh01*dh11dh12*width_m1*height_1+
					h02h01*d2h11d2h12*width_1*height_m1));
				/* d2fdxdy */
				temp[3] += sin_1*
					(membrane_smoothing*
					(dh02dh11*h11h12*height_3+
					h02h11*dh11dh12*width_2*height_1)+
					plate_bending_smoothing*
					(d2h02d2h11*h11h12*width_m2*height_3+
					dh02dh11*dh11dh12*height_1+
					h02h11*d2h11d2h12*width_2*height_m1));
			}
			else
			{
#endif /* defined (FIX_APEX) */
				/* d(i_jm1)/d(im1_jm1) */
				temp=matrix_and_right_hand_side+
					im1_jm1*number_of_equations_plus_1+i_jm1;
				/* d/d(f) */
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh01dh01*h01h02*width_m1*height_1+
					h01h01*dh01dh02*width_1*height_m1)+
					plate_bending_smoothing*
					(d2h01d2h01*h01h02*width_m3*height_1+
					dh01dh01*dh01dh02*width_m1*height_m1+
					h01h01*d2h01d2h02*width_1*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh01dh11*h01h02*height_1+
					h01h11*dh01dh02*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h01d2h11*h01h02*width_m2*height_1+
					dh01dh11*dh01dh02*height_m1+
					h01h11*d2h01d2h02*width_2*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh01dh01*h01h12*width_m1*height_2+
					h01h01*dh01dh12*width_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h01h12*width_m3*height_2+
					dh01dh01*dh01dh12*width_m1+
					h01h01*d2h01d2h12*width_1*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh01dh11*h01h12*height_2+
					h01h11*dh01dh12*width_2)+
					plate_bending_smoothing*
					(d2h01d2h11*h01h12*width_m2*height_2+
					dh01dh11*dh01dh12+
					h01h11*d2h01d2h12*width_2*height_m2));
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh11dh01*h01h02*height_1+
					h11h01*dh01dh02*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h11d2h01*h01h02*width_m2*height_1+
					dh11dh01*dh01dh02*height_m1+
					h11h01*d2h01d2h02*width_2*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh11dh11*h01h02*width_1*height_1+
					h11h11*dh01dh02*width_3*height_m1)+
					plate_bending_smoothing*
					(d2h11d2h11*h01h02*width_m1*height_1+
					dh11dh11*dh01dh02*width_1*height_m1+
					h11h11*d2h01d2h02*width_3*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh11dh01*h01h12*height_2+
					h11h01*dh01dh12*width_2)+
					plate_bending_smoothing*
					(d2h11d2h01*h01h12*width_m2*height_2+
					dh11dh01*dh01dh12+
					h11h01*d2h01d2h12*width_2*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh11dh11*h01h12*width_1*height_2+
					h11h11*dh01dh12*width_3)+
					plate_bending_smoothing*
					(d2h11d2h11*h01h12*width_m1*height_2+
					dh11dh11*dh01dh12*width_1+
					h11h11*d2h01d2h12*width_3*height_m2));
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh01dh01*h11h02*width_m1*height_2+
					h01h01*dh11dh02*width_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h11h02*width_m3*height_2+
					dh01dh01*dh11dh02*width_m1+
					h01h01*d2h11d2h02*width_1*height_m2));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh01dh11*h11h02*height_2+
					h01h11*dh11dh02*width_2)+
					plate_bending_smoothing*
					(d2h01d2h11*h11h02*width_m2*height_2+
					dh01dh11*dh11dh02+
					h01h11*d2h11d2h02*width_2*height_m2));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh01dh01*h11h12*width_m1*height_3+
					h01h01*dh11dh12*width_1*height_1)+
					plate_bending_smoothing*
					(d2h01d2h01*h11h12*width_m3*height_3+
					dh01dh01*dh11dh12*width_m1*height_1+
					h01h01*d2h11d2h12*width_1*height_m1));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh01dh11*h11h12*height_3+
					h01h11*dh11dh12*width_2*height_1)+
					plate_bending_smoothing*
					(d2h01d2h11*h11h12*width_m2*height_3+
					dh01dh11*dh11dh12*height_1+
					h01h11*d2h11d2h12*width_2*height_m1));
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh11dh01*h11h02*height_2+
					h11h01*dh11dh02*width_2)+
					plate_bending_smoothing*
					(d2h11d2h01*h11h02*width_m2*height_2+
					dh11dh01*dh11dh02+
					h11h01*d2h11d2h02*width_2*height_m2));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh11dh11*h11h02*width_1*height_2+
					h11h11*dh11dh02*width_3)+
					plate_bending_smoothing*
					(d2h11d2h11*h11h02*width_m1*height_2+
					dh11dh11*dh11dh02*width_1+
					h11h11*d2h11d2h02*width_3*height_m2));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh11dh01*h11h12*height_3+
					h11h01*dh11dh12*width_2*height_1)+
					plate_bending_smoothing*
					(d2h11d2h01*h11h12*width_m2*height_3+
					dh11dh01*dh11dh12*height_1+
					h11h01*d2h11d2h12*width_2*height_m1));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh11dh11*h11h12*width_1*height_3+
					h11h11*dh11dh12*width_3*height_1)+
					plate_bending_smoothing*
					(d2h11d2h11*h11h12*width_m1*height_3+
					dh11dh11*dh11dh12*width_1*height_1+
					h11h11*d2h11d2h12*width_3*height_m1));
				/* d(i_jm1)/d(im1_j) */
				temp=matrix_and_right_hand_side+
					im1_j*number_of_equations_plus_1+i_jm1;
				/* d/d(f) */
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh02dh01*h01h02*width_m1*height_1+
					h02h01*dh01dh02*width_1*height_m1)+
					plate_bending_smoothing*
					(d2h02d2h01*h01h02*width_m3*height_1+
					dh02dh01*dh01dh02*width_m1*height_m1+
					h02h01*d2h01d2h02*width_1*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh02dh11*h01h02*height_1+
					h02h11*dh01dh02*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h02d2h11*h01h02*width_m2*height_1+
					dh02dh11*dh01dh02*height_m1+
					h02h11*d2h01d2h02*width_2*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh02dh01*h01h12*width_m1*height_2+
					h02h01*dh01dh12*width_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h01h12*width_m3*height_2+
					dh02dh01*dh01dh12*width_m1+
					h02h01*d2h01d2h12*width_1*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh02dh11*h01h12*height_2+
					h02h11*dh01dh12*width_2)+
					plate_bending_smoothing*
					(d2h02d2h11*h01h12*width_m2*height_2+
					dh02dh11*dh01dh12+
					h02h11*d2h01d2h12*width_2*height_m2));
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh12dh01*h01h02*height_1+
					h12h01*dh01dh02*width_2*height_m1)+
					plate_bending_smoothing*
					(d2h12d2h01*h01h02*width_m2*height_1+
					dh12dh01*dh01dh02*height_m1+
					h12h01*d2h01d2h02*width_2*height_m3));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh12dh11*h01h02*width_1*height_1+
					h12h11*dh01dh02*width_3*height_m1)+
					plate_bending_smoothing*
					(d2h12d2h11*h01h02*width_m1*height_1+
					dh12dh11*dh01dh02*width_1*height_m1+
					h12h11*d2h01d2h02*width_3*height_m3));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh12dh01*h01h12*height_2+
					h12h01*dh01dh12*width_2)+
					plate_bending_smoothing*
					(d2h12d2h01*h01h12*width_m2*height_2+
					dh12dh01*dh01dh12+
					h12h01*d2h01d2h12*width_2*height_m2));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh12dh11*h01h12*width_1*height_2+
					h12h11*dh01dh12*width_3)+
					plate_bending_smoothing*
					(d2h12d2h11*h01h12*width_m1*height_2+
					dh12dh11*dh01dh12*width_1+
					h12h11*d2h01d2h12*width_3*height_m2));
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh02dh01*h11h02*width_m1*height_2+
					h02h01*dh11dh02*width_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h11h02*width_m3*height_2+
					dh02dh01*dh11dh02*width_m1+
					h02h01*d2h11d2h02*width_1*height_m2));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh02dh11*h11h02*height_2+
					h02h11*dh11dh02*width_2)+
					plate_bending_smoothing*
					(d2h02d2h11*h11h02*width_m2*height_2+
					dh02dh11*dh11dh02+
					h02h11*d2h11d2h02*width_2*height_m2));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh02dh01*h11h12*width_m1*height_3+
					h02h01*dh11dh12*width_1*height_1)+
					plate_bending_smoothing*
					(d2h02d2h01*h11h12*width_m3*height_3+
					dh02dh01*dh11dh12*width_m1*height_1+
					h02h01*d2h11d2h12*width_1*height_m1));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh02dh11*h11h12*height_3+
					h02h11*dh11dh12*width_2*height_1)+
					plate_bending_smoothing*
					(d2h02d2h11*h11h12*width_m2*height_3+
					dh02dh11*dh11dh12*height_1+
					h02h11*d2h11d2h12*width_2*height_m1));
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += 
					(membrane_smoothing*
					(dh12dh01*h11h02*height_2+
					h12h01*dh11dh02*width_2)+
					plate_bending_smoothing*
					(d2h12d2h01*h11h02*width_m2*height_2+
					dh12dh01*dh11dh02+
					h12h01*d2h11d2h02*width_2*height_m2));
				/* dfdx */
				temp[1] += 
					(membrane_smoothing*
					(dh12dh11*h11h02*width_1*height_2+
					h12h11*dh11dh02*width_3)+
					plate_bending_smoothing*
					(d2h12d2h11*h11h02*width_m1*height_2+
					dh12dh11*dh11dh02*width_1+
					h12h11*d2h11d2h02*width_3*height_m2));
				/* dfdy */
				temp[2] += 
					(membrane_smoothing*
					(dh12dh01*h11h12*height_3+
					h12h01*dh11dh12*width_2*height_1)+
					plate_bending_smoothing*
					(d2h12d2h01*h11h12*width_m2*height_3+
					dh12dh01*dh11dh12*height_1+
					h12h01*d2h11d2h12*width_2*height_m1));
				/* d2fdxdy */
				temp[3] += 
					(membrane_smoothing*
					(dh12dh11*h11h12*width_1*height_3+
					h12h11*dh11dh12*width_3*height_1)+
					plate_bending_smoothing*
					(d2h12d2h11*h11h12*width_m1*height_3+
					dh12dh11*dh11dh12*width_1*height_1+
					h12h11*d2h11d2h12*width_3*height_m1));
#if defined (FIX_APEX)
			}
#endif /* defined (FIX_APEX) */
			/* d(i_jm1)/d(i_j) */
			temp=matrix_and_right_hand_side+
				i_j*number_of_equations_plus_1+i_jm1;
			/* d/d(f) */
			/* f */
			*temp += 
				(membrane_smoothing*
				(dh02dh01*h02h02*width_m1*height_1+
				h02h01*dh02dh02*width_1*height_m1)+
				plate_bending_smoothing*
				(d2h02d2h01*h02h02*width_m3*height_1+
				dh02dh01*dh02dh02*width_m1*height_m1+
				h02h01*d2h02d2h02*width_1*height_m3));
			/* dfdx */
			temp[1] += 
				(membrane_smoothing*
				(dh02dh11*h02h02*height_1+
				h02h11*dh02dh02*width_2*height_m1)+
				plate_bending_smoothing*
				(d2h02d2h11*h02h02*width_m2*height_1+
				dh02dh11*dh02dh02*height_m1+
				h02h11*d2h02d2h02*width_2*height_m3));
			/* dfdy */
			temp[2] += 
				(membrane_smoothing*
				(dh02dh01*h02h12*width_m1*height_2+
				h02h01*dh02dh12*width_1)+
				plate_bending_smoothing*
				(d2h02d2h01*h02h12*width_m3*height_2+
				dh02dh01*dh02dh12*width_m1+
				h02h01*d2h02d2h12*width_1*height_m2));
			/* d2fdxdy */
			temp[3] += 
				(membrane_smoothing*
				(dh02dh11*h02h12*height_2+
				h02h11*dh02dh12*width_2)+
				plate_bending_smoothing*
				(d2h02d2h11*h02h12*width_m2*height_2+
				dh02dh11*dh02dh12+
				h02h11*d2h02d2h12*width_2*height_m2));
			/* d/d(dfdx) */
			temp += number_of_equations_plus_1;
			/* f */
			*temp += 
				(membrane_smoothing*
				(dh12dh01*h02h02*height_1+
				h12h01*dh02dh02*width_2*height_m1)+
				plate_bending_smoothing*
				(d2h12d2h01*h02h02*width_m2*height_1+
				dh12dh01*dh02dh02*height_m1+
				h12h01*d2h02d2h02*width_2*height_m3));
			/* dfdx */
			temp[1] += 
				(membrane_smoothing*
				(dh12dh11*h02h02*width_1*height_1+
				h12h11*dh02dh02*width_3*height_m1)+
				plate_bending_smoothing*
				(d2h12d2h11*h02h02*width_m1*height_1+
				dh12dh11*dh02dh02*width_1*height_m1+
				h12h11*d2h02d2h02*width_3*height_m3));
			/* dfdy */
			temp[2] += 
				(membrane_smoothing*
				(dh12dh01*h02h12*height_2+
				h12h01*dh02dh12*width_2)+
				plate_bending_smoothing*
				(d2h12d2h01*h02h12*width_m2*height_2+
				dh12dh01*dh02dh12+
				h12h01*d2h02d2h12*width_2*height_m2));
			/* d2fdxdy */
			temp[3] += 
				(membrane_smoothing*
				(dh12dh11*h02h12*width_1*height_2+
				h12h11*dh02dh12*width_3)+
				plate_bending_smoothing*
				(d2h12d2h11*h02h12*width_m1*height_2+
				dh12dh11*dh02dh12*width_1+
				h12h11*d2h02d2h12*width_3*height_m2));
			/* d/d(dfdy) */
			temp += number_of_equations_plus_1;
			/* f */
			*temp += 
				(membrane_smoothing*
				(dh02dh01*h12h02*width_m1*height_2+
				h02h01*dh12dh02*width_1)+
				plate_bending_smoothing*
				(d2h02d2h01*h12h02*width_m3*height_2+
				dh02dh01*dh12dh02*width_m1+
				h02h01*d2h12d2h02*width_1*height_m2));
			/* dfdx */
			temp[1] += 
				(membrane_smoothing*
				(dh02dh11*h12h02*height_2+
				h02h11*dh12dh02*width_2)+
				plate_bending_smoothing*
				(d2h02d2h11*h12h02*width_m2*height_2+
				dh02dh11*dh12dh02+
				h02h11*d2h12d2h02*width_2*height_m2));
			/* dfdy */
			temp[2] += 
				(membrane_smoothing*
				(dh02dh01*h12h12*width_m1*height_3+
				h02h01*dh12dh12*width_1*height_1)+
				plate_bending_smoothing*
				(d2h02d2h01*h12h12*width_m3*height_3+
				dh02dh01*dh12dh12*width_m1*height_1+
				h02h01*d2h12d2h12*width_1*height_m1));
			/* d2fdxdy */
			temp[3] += 
				(membrane_smoothing*
				(dh02dh11*h12h12*height_3+
				h02h11*dh12dh12*width_2*height_1)+
				plate_bending_smoothing*
				(d2h02d2h11*h12h12*width_m2*height_3+
				dh02dh11*dh12dh12*height_1+
				h02h11*d2h12d2h12*width_2*height_m1));
			/* d/d(d2fdxdy) */
			temp += number_of_equations_plus_1;
			/* f */
			*temp += 
				(membrane_smoothing*
				(dh12dh01*h12h02*height_2+
				h12h01*dh12dh02*width_2)+
				plate_bending_smoothing*
				(d2h12d2h01*h12h02*width_m2*height_2+
				dh12dh01*dh12dh02+
				h12h01*d2h12d2h02*width_2*height_m2));
			/* dfdx */
			temp[1] += 
				(membrane_smoothing*
				(dh12dh11*h12h02*width_1*height_2+
				h12h11*dh12dh02*width_3)+
				plate_bending_smoothing*
				(d2h12d2h11*h12h02*width_m1*height_2+
				dh12dh11*dh12dh02*width_1+
				h12h11*d2h12d2h02*width_3*height_m2));
			/* dfdy */
			temp[2] += 
				(membrane_smoothing*
				(dh12dh01*h12h12*height_3+
				h12h01*dh12dh12*width_2*height_1)+
				plate_bending_smoothing*
				(d2h12d2h01*h12h12*width_m2*height_3+
				dh12dh01*dh12dh12*height_1+
				h12h01*d2h12d2h12*width_2*height_m1));
			/* d2fdxdy */
			temp[3] += 
				(membrane_smoothing*
				(dh12dh11*h12h12*width_1*height_3+
				h12h11*dh12dh12*width_3*height_1)+
				plate_bending_smoothing*
				(d2h12d2h11*h12h12*width_m1*height_3+
				dh12dh11*dh12dh12*width_1*height_1+
				h12h11*d2h12d2h12*width_3*height_m1));
			/* d(i_jm1)/d(i_jm1) */
			temp=matrix_and_right_hand_side+
				i_jm1*number_of_equations_plus_1+i_jm1;
			/* d/d(f) */
			/* f */
			*temp += 
				(membrane_smoothing*
				(dh01dh01*h02h02*width_m1*height_1+
				h01h01*dh02dh02*width_1*height_m1)+
				plate_bending_smoothing*
				(d2h01d2h01*h02h02*width_m3*height_1+
				dh01dh01*dh02dh02*width_m1*height_m1+
				h01h01*d2h02d2h02*width_1*height_m3));
			/* dfdx */
			temp[1] += 
				(membrane_smoothing*
				(dh01dh11*h02h02*height_1+
				h01h11*dh02dh02*width_2*height_m1)+
				plate_bending_smoothing*
				(d2h01d2h11*h02h02*width_m2*height_1+
				dh01dh11*dh02dh02*height_m1+
				h01h11*d2h02d2h02*width_2*height_m3));
			/* dfdy */
			temp[2] += 
				(membrane_smoothing*
				(dh01dh01*h02h12*width_m1*height_2+
				h01h01*dh02dh12*width_1)+
				plate_bending_smoothing*
				(d2h01d2h01*h02h12*width_m3*height_2+
				dh01dh01*dh02dh12*width_m1+
				h01h01*d2h02d2h12*width_1*height_m2));
			/* d2fdxdy */
			temp[3] += 
				(membrane_smoothing*
				(dh01dh11*h02h12*height_2+
				h01h11*dh02dh12*width_2)+
				plate_bending_smoothing*
				(d2h01d2h11*h02h12*width_m2*height_2+
				dh01dh11*dh02dh12+
				h01h11*d2h02d2h12*width_2*height_m2));
			/* d/d(dfdx) */
			temp += number_of_equations_plus_1;
			/* f */
			*temp += 
				(membrane_smoothing*
				(dh11dh01*h02h02*height_1+
				h11h01*dh02dh02*width_2*height_m1)+
				plate_bending_smoothing*
				(d2h11d2h01*h02h02*width_m2*height_1+
				dh11dh01*dh02dh02*height_m1+
				h11h01*d2h02d2h02*width_2*height_m3));
			/* dfdx */
			temp[1] += 
				(membrane_smoothing*
				(dh11dh11*h02h02*width_1*height_1+
				h11h11*dh02dh02*width_3*height_m1)+
				plate_bending_smoothing*
				(d2h11d2h11*h02h02*width_m1*height_1+
				dh11dh11*dh02dh02*width_1*height_m1+
				h11h11*d2h02d2h02*width_3*height_m3));
			/* dfdy */
			temp[2] += 
				(membrane_smoothing*
				(dh11dh01*h02h12*height_2+
				h11h01*dh02dh12*width_2)+
				plate_bending_smoothing*
				(d2h11d2h01*h02h12*width_m2*height_2+
				dh11dh01*dh02dh12+
				h11h01*d2h02d2h12*width_2*height_m2));
			/* d2fdxdy */
			temp[3] += 
				(membrane_smoothing*
				(dh11dh11*h02h12*width_1*height_2+
				h11h11*dh02dh12*width_3)+
				plate_bending_smoothing*
				(d2h11d2h11*h02h12*width_m1*height_2+
				dh11dh11*dh02dh12*width_1+
				h11h11*d2h02d2h12*width_3*height_m2));
			/* d/d(dfdy) */
			temp += number_of_equations_plus_1;
			/* f */
			*temp += 
				(membrane_smoothing*
				(dh01dh01*h12h02*width_m1*height_2+
				h01h01*dh12dh02*width_1)+
				plate_bending_smoothing*
				(d2h01d2h01*h12h02*width_m3*height_2+
				dh01dh01*dh12dh02*width_m1+
				h01h01*d2h12d2h02*width_1*height_m2));
			/* dfdx */
			temp[1] += 
				(membrane_smoothing*
				(dh01dh11*h12h02*height_2+
				h01h11*dh12dh02*width_2)+
				plate_bending_smoothing*
				(d2h01d2h11*h12h02*width_m2*height_2+
				dh01dh11*dh12dh02+
				h01h11*d2h12d2h02*width_2*height_m2));
			/* dfdy */
			temp[2] += 
				(membrane_smoothing*
				(dh01dh01*h12h12*width_m1*height_3+
				h01h01*dh12dh12*width_1*height_1)+
				plate_bending_smoothing*
				(d2h01d2h01*h12h12*width_m3*height_3+
				dh01dh01*dh12dh12*width_m1*height_1+
				h01h01*d2h12d2h12*width_1*height_m1));
			/* d2fdxdy */
			temp[3] += 
				(membrane_smoothing*
				(dh01dh11*h12h12*height_3+
				h01h11*dh12dh12*width_2*height_1)+
				plate_bending_smoothing*
				(d2h01d2h11*h12h12*width_m2*height_3+
				dh01dh11*dh12dh12*height_1+
				h01h11*d2h12d2h12*width_2*height_m1));
			/* d/d(d2fdxdy) */
			temp += number_of_equations_plus_1;
			/* f */
			*temp += 
				(membrane_smoothing*
				(dh11dh01*h12h02*height_2+
				h11h01*dh12dh02*width_2)+
				plate_bending_smoothing*
				(d2h11d2h01*h12h02*width_m2*height_2+
				dh11dh01*dh12dh02+
				h11h01*d2h12d2h02*width_2*height_m2));
			/* dfdx */
			temp[1] += 
				(membrane_smoothing*
				(dh11dh11*h12h02*width_1*height_2+
				h11h11*dh12dh02*width_3)+
				plate_bending_smoothing*
				(d2h11d2h11*h12h02*width_m1*height_2+
				dh11dh11*dh12dh02*width_1+
				h11h11*d2h12d2h02*width_3*height_m2));
			/* dfdy */
			temp[2] += 
				(membrane_smoothing*
				(dh11dh01*h12h12*height_3+
				h11h01*dh12dh12*width_2*height_1)+
				plate_bending_smoothing*
				(d2h11d2h01*h12h12*width_m2*height_3+
				dh11dh01*dh12dh12*height_1+
				h11h01*d2h12d2h12*width_2*height_m1));
			/* d2fdxdy */
			temp[3] += 
				(membrane_smoothing*
				(dh11dh11*h12h12*width_1*height_3+
				h11h11*dh12dh12*width_3*height_1)+
				plate_bending_smoothing*
				(d2h11d2h11*h12h12*width_m1*height_3+
				dh11dh11*dh12dh12*width_1*height_1+
				h11h11*d2h12d2h12*width_3*height_m1));
			/* end smoothing */
		}
	}
	/* loop over the data points */
	for (i=0;i<number_of_data;i++)
	{
		/* determine which element the data point is in */
		u=x[i];
		column=0;
		while ((column<number_of_columns)&&(u>=x_mesh[column]))
		{
			column++;
		}
		if ((column>0)&&(u<=x_mesh[column]))
		{
			v=y[i];
			row=0;
			while ((row<number_of_rows)&&(v>=y_mesh[row]))
			{
				row++;
			}
			if ((row>0)&&(v<=y_mesh[row]))
			{
				f_d=value[i];
				weight_d=weight[i];
				/* calculate basis function values */
				width=x_mesh[column]-x_mesh[column-1];
				h11_u=h12_u=u-x_mesh[column-1];
				u=h11_u/width;
				h01_u=(2*u-3)*u*u+1;
				h11_u *= (u-1)*(u-1);
				h02_u=u*u*(3-2*u);
				h12_u *= u*(u-1);
				height=y_mesh[row]-y_mesh[row-1];
				h11_v=h12_v=v-y_mesh[row-1];
				v=h11_v/height;
				h01_v=(2*v-3)*v*v+1;
				h11_v *= (v-1)*(v-1);
				h02_v=v*v*(3-2*v);
				h12_v *= v*(v-1);
				/* calculate the interpolation function coefficients */
				f_im1_jm1=h01_u*h01_v;
				f_im1_j=h02_u*h01_v;
				f_i_j=h02_u*h02_v;
				f_i_jm1=h01_u*h02_v;
				dfdx_im1_jm1=h11_u*h01_v;
				dfdx_im1_j=h12_u*h01_v;
				dfdx_i_j=h12_u*h02_v;
				dfdx_i_jm1=h11_u*h02_v;
				dfdy_im1_jm1=h01_u*h11_v;
				dfdy_im1_j=h02_u*h11_v;
				dfdy_i_j=h02_u*h12_v;
				dfdy_i_jm1=h01_u*h12_v;
				d2fdxdy_im1_jm1=h11_u*h11_v;
				d2fdxdy_im1_j=h12_u*h11_v;
				d2fdxdy_i_j=h12_u*h12_v;
				d2fdxdy_i_jm1=h11_u*h12_v;
				f_im1_jm1_w=weight_d*f_im1_jm1;
				f_i_jm1_w=weight_d*f_i_jm1;
				f_i_j_w=weight_d*f_i_j;
				f_im1_j_w=weight_d*f_im1_j;
				dfdx_im1_jm1_w=weight_d*dfdx_im1_jm1;
				dfdx_i_jm1_w=weight_d*dfdx_i_jm1;
				dfdx_i_j_w=weight_d*dfdx_i_j;
				dfdx_im1_j_w=weight_d*dfdx_im1_j;
				dfdy_im1_jm1_w=weight_d*dfdy_im1_jm1;
				dfdy_i_jm1_w=weight_d*dfdy_i_jm1;
				dfdy_i_j_w=weight_d*dfdy_i_j;
				dfdy_im1_j_w=weight_d*dfdy_im1_j;
				d2fdxdy_im1_jm1_w=weight_d*d2fdxdy_im1_jm1;
				d2fdxdy_i_jm1_w=weight_d*d2fdxdy_i_jm1;
				d2fdxdy_i_j_w=weight_d*d2fdxdy_i_j;
				d2fdxdy_im1_j_w=weight_d*d2fdxdy_im1_j;
				/* calculate node numbers
					NB the local coordinates have the bottom left corner of the element as
					the origin.  This means that
					(i-1,j-1) is the bottom left corner
					(i,j-1) is the top left corner
					(i,j) is the top right corner
					(i-1,j) is the bottom right corner */
				switch (type)
				{
					case PATCH:
					{
						/* (i-1,j-1) node (bottom left corner) */
						im1_jm1=4*((row-1)*(number_of_columns+1)+column-1);
						/* (i,j-1) node (top left corner) */
						i_jm1=4*(row*(number_of_columns+1)+column-1);
						/* (i,j) node (top right corner) */
						i_j=4*(row*(number_of_columns+1)+column);
						/* (i-1,j) node (bottom right corner) */
						im1_j=4*((row-1)*(number_of_columns+1)+column);
					} break;
					case SOCK:
					{
						if (row==1)
						{
							/* (i-1,j-1) node (bottom left corner) */
							im1_jm1=0;
#if defined (FIX_APEX)
							cos_0=cosine_apex[column-1];
							sin_0=sine_apex[column-1];
							dfdx_im1_jm1=sin_0*dfdy_im1_jm1;
							dfdy_im1_jm1 *= cos_0;
							dfdx_im1_jm1_w=sin_0*dfdy_im1_jm1_w;
							dfdy_im1_jm1_w *= cos_0;
#endif /* defined (FIX_APEX) */
							/* (i,j-1) node (top left corner) */
#if defined (FIX_APEX)
							i_jm1=4*column-1;
#else
							i_jm1=4*column;
#endif /* defined (FIX_APEX) */
							/* (i,j) node (top right corner) */
							if (column==number_of_columns)
							{
#if defined (FIX_APEX)
								i_j=3;
#else
								i_j=4;
#endif /* defined (FIX_APEX) */
							}
							else
							{
#if defined (FIX_APEX)
								i_j=4*column+3;
#else
								i_j=4*(column+1);
#endif /* defined (FIX_APEX) */
							}
							/* (i-1,j) node (bottom right corner) */
							im1_j=0;
#if defined (FIX_APEX)
							cos_1=cosine_apex[column];
							sin_1=sine_apex[column];
							dfdx_im1_j=sin_1*dfdy_im1_j;
							dfdy_im1_j *= cos_1;
							dfdx_im1_j_w=sin_1*dfdy_im1_j_w;
							dfdy_im1_j_w *= cos_1;
#endif /* defined (FIX_APEX) */
						}
						else
						{
							/* (i-1,j-1) node (bottom left corner) */
#if defined (FIX_APEX)
							im1_jm1=4*((row-2)*number_of_columns+column)-1;
#else
							im1_jm1=4*((row-2)*number_of_columns+column);
#endif /* defined (FIX_APEX) */
							/* (i,j-1) node (top left corner) */
#if defined (FIX_APEX)
							i_jm1=4*((row-1)*number_of_columns+column)-1;
#else
							i_jm1=4*((row-1)*number_of_columns+column);
#endif /* defined (FIX_APEX) */
							/* (i,j) node (top right corner) */
							if (column==number_of_columns)
							{
#if defined (FIX_APEX)
								i_j=4*(row-1)*number_of_columns+3;
#else
								i_j=4*((row-1)*number_of_columns+1);
#endif /* defined (FIX_APEX) */
							}
							else
							{
#if defined (FIX_APEX)
								i_j=4*((row-1)*number_of_columns+column)+3;
#else
								i_j=4*((row-1)*number_of_columns+column+1);
#endif /* defined (FIX_APEX) */
							}
							/* (i-1,j) node (bottom right corner) */
							if (column==number_of_columns)
							{
#if defined (FIX_APEX)
								im1_j=4*(row-2)*number_of_columns+3;
#else
								im1_j=4*((row-2)*number_of_columns+1);
#endif /* defined (FIX_APEX) */
							}
							else
							{
#if defined (FIX_APEX)
								im1_j=4*((row-2)*number_of_columns+column)+3;
#else
								im1_j=4*((row-2)*number_of_columns+column+1);
#endif /* defined (FIX_APEX) */
							}
						}
					} break;
					case TORSO:
					{
						/* (i-1,j-1) node (bottom left corner) */
						im1_jm1=4*((row-1)*number_of_columns+column-1);
						/* (i,j-1) node (top left corner) */
						i_jm1=4*(row*number_of_columns+column-1);
						if (column==number_of_columns)
						{
							/* (i,j) node (top right corner) */
							i_j=4*row*number_of_columns;
							/* (i-1,j) node (bottom right corner) */
							im1_j=4*(row-1)*number_of_columns;
						}
						else
						{
							/* (i,j) node (top right corner) */
							i_j=4*(row*number_of_columns+column);
							/* (i-1,j) node (bottom right corner) */
							im1_j=4*((row-1)*number_of_columns+column);
						}
					} break;
				}
				/* update the matrix */
				/* start data */
#if defined (FIX_APEX)
				if ((SOCK==type)&&(im1_jm1==0))
				{
				/* d(im1_jm1)/d(im1_jm1) */
				temp=matrix_and_right_hand_side+
					im1_jm1*number_of_equations_plus_1+im1_jm1;
				/* d/d(f) */
				/* f */
				*temp += f_im1_jm1_w*f_im1_jm1;
				/* dfdx */
				temp[1] += f_im1_jm1_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += f_im1_jm1_w*dfdy_im1_jm1;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_im1_jm1_w*f_im1_jm1;
				/* dfdx */
				temp[1] += dfdx_im1_jm1_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += dfdx_im1_jm1_w*dfdy_im1_jm1;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_im1_jm1_w*f_im1_jm1;
				/* dfdx */
				temp[1] += dfdy_im1_jm1_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += dfdy_im1_jm1_w*dfdy_im1_jm1;
				/* d(im1_jm1)/d(im1_j) */
				temp=matrix_and_right_hand_side+
					im1_j*number_of_equations_plus_1+im1_jm1;
				/* d/d(f) */
				/* f */
				*temp += f_im1_j_w*f_im1_jm1;
				/* dfdx */
				temp[1] += f_im1_j_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += f_im1_j_w*dfdy_im1_jm1;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_im1_j_w*f_im1_jm1;
				/* dfdx */
				temp[1] += dfdx_im1_j_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += dfdx_im1_j_w*dfdy_im1_jm1;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_im1_j_w*f_im1_jm1;
				/* dfdx */
				temp[1] += dfdy_im1_j_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += dfdy_im1_j_w*dfdy_im1_jm1;
				/* d(im1_jm1)/d(i_j) */
				temp=matrix_and_right_hand_side+
					i_j*number_of_equations_plus_1+im1_jm1;
				/* d/d(f) */
				/* f */
				*temp += f_i_j_w*f_im1_jm1;
				/* dfdx */
				temp[1] += f_i_j_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += f_i_j_w*dfdy_im1_jm1;
				/* d2fdxdy */
				temp[3] += f_i_j_w*d2fdxdy_im1_jm1;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_i_j_w*f_im1_jm1;
				/* dfdx */
				temp[1] += dfdx_i_j_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += dfdx_i_j_w*dfdy_im1_jm1;
				/* d2fdxdy */
				temp[3] += dfdx_i_j_w*d2fdxdy_im1_jm1;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_i_j_w*f_im1_jm1;
				/* dfdx */
				temp[1] += dfdy_i_j_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += dfdy_i_j_w*dfdy_im1_jm1;
				/* d2fdxdy */
				temp[3] += dfdy_i_j_w*d2fdxdy_im1_jm1;
				/* d(im1_jm1)/d(i_jm1) */
				temp=matrix_and_right_hand_side+
					i_jm1*number_of_equations_plus_1+im1_jm1;
				/* d/d(f) */
				/* f */
				*temp += f_i_jm1_w*f_im1_jm1;
				/* dfdx */
				temp[1] += f_i_jm1_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += f_i_jm1_w*dfdy_im1_jm1;
				/* d2fdxdy */
				temp[3] += f_i_jm1_w*d2fdxdy_im1_jm1;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_i_jm1_w*f_im1_jm1;
				/* dfdx */
				temp[1] += dfdx_i_jm1_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += dfdx_i_jm1_w*dfdy_im1_jm1;
				/* d2fdxdy */
				temp[3] += dfdx_i_jm1_w*d2fdxdy_im1_jm1;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_i_jm1_w*f_im1_jm1;
				/* dfdx */
				temp[1] += dfdy_i_jm1_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += dfdy_i_jm1_w*dfdy_im1_jm1;
				/* d2fdxdy */
				temp[3] += dfdy_i_jm1_w*d2fdxdy_im1_jm1;
				/* d(im1_j)/d(im1_jm1) */
				temp=matrix_and_right_hand_side+
					im1_jm1*number_of_equations_plus_1+im1_j;
				/* d/d(f) */
				/* f */
				*temp += f_im1_jm1_w*f_im1_j;
				/* dfdx */
				temp[1] += f_im1_jm1_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += f_im1_jm1_w*dfdy_im1_j;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_im1_jm1_w*f_im1_j;
				/* dfdx */
				temp[1] += dfdx_im1_jm1_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += dfdx_im1_jm1_w*dfdy_im1_j;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_im1_jm1_w*f_im1_j;
				/* dfdx */
				temp[1] += dfdy_im1_jm1_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += dfdy_im1_jm1_w*dfdy_im1_j;
				/* d(im1_j)/d(im1_j) */
				temp=matrix_and_right_hand_side+
					im1_j*number_of_equations_plus_1+im1_j;
				/* d/d(f) */
				/* f */
				*temp += f_im1_j_w*f_im1_j;
				/* dfdx */
				temp[1] += f_im1_j_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += f_im1_j_w*dfdy_im1_j;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_im1_j_w*f_im1_j;
				/* dfdx */
				temp[1] += dfdx_im1_j_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += dfdx_im1_j_w*dfdy_im1_j;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_im1_j_w*f_im1_j;
				/* dfdx */
				temp[1] += dfdy_im1_j_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += dfdy_im1_j_w*dfdy_im1_j;
				/* d(im1_j)/d(i_j) */
				temp=matrix_and_right_hand_side+
					i_j*number_of_equations_plus_1+im1_j;
				/* d/d(f) */
				/* f */
				*temp += f_i_j_w*f_im1_j;
				/* dfdx */
				temp[1] += f_i_j_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += f_i_j_w*dfdy_im1_j;
				/* d2fdxdy */
				temp[3] += f_i_j_w*d2fdxdy_im1_j;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_i_j_w*f_im1_j;
				/* dfdx */
				temp[1] += dfdx_i_j_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += dfdx_i_j_w*dfdy_im1_j;
				/* d2fdxdy */
				temp[3] += dfdx_i_j_w*d2fdxdy_im1_j;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_i_j_w*f_im1_j;
				/* dfdx */
				temp[1] += dfdy_i_j_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += dfdy_i_j_w*dfdy_im1_j;
				/* d2fdxdy */
				temp[3] += dfdy_i_j_w*d2fdxdy_im1_j;
				/* d(im1_j)/d(i_jm1) */
				temp=matrix_and_right_hand_side+
					i_jm1*number_of_equations_plus_1+im1_j;
				/* d/d(f) */
				/* f */
				*temp += f_i_jm1_w*f_im1_j;
				/* dfdx */
				temp[1] += f_i_jm1_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += f_i_jm1_w*dfdy_im1_j;
				/* d2fdxdy */
				temp[3] += f_i_jm1_w*d2fdxdy_im1_j;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_i_jm1_w*f_im1_j;
				/* dfdx */
				temp[1] += dfdx_i_jm1_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += dfdx_i_jm1_w*dfdy_im1_j;
				/* d2fdxdy */
				temp[3] += dfdx_i_jm1_w*d2fdxdy_im1_j;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_i_jm1_w*f_im1_j;
				/* dfdx */
				temp[1] += dfdy_i_jm1_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += dfdy_i_jm1_w*dfdy_im1_j;
				/* d2fdxdy */
				temp[3] += dfdy_i_jm1_w*d2fdxdy_im1_j;
				}
				else
				{
#endif /* defined (FIX_APEX) */
				/* d(im1_jm1)/d(im1_jm1) */
				temp=matrix_and_right_hand_side+
					im1_jm1*number_of_equations_plus_1+im1_jm1;
				/* d/d(f) */
				/* f */
				*temp += f_im1_jm1_w*f_im1_jm1;
				/* dfdx */
				temp[1] += f_im1_jm1_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += f_im1_jm1_w*dfdy_im1_jm1;
				/* d2fdxdy */
				temp[3] += f_im1_jm1_w*d2fdxdy_im1_jm1;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_im1_jm1_w*f_im1_jm1;
				/* dfdx */
				temp[1] += dfdx_im1_jm1_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += dfdx_im1_jm1_w*dfdy_im1_jm1;
				/* d2fdxdy */
				temp[3] += dfdx_im1_jm1_w*d2fdxdy_im1_jm1;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_im1_jm1_w*f_im1_jm1;
				/* dfdx */
				temp[1] += dfdy_im1_jm1_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += dfdy_im1_jm1_w*dfdy_im1_jm1;
				/* d2fdxdy */
				temp[3] += dfdy_im1_jm1_w*d2fdxdy_im1_jm1;
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += d2fdxdy_im1_jm1_w*f_im1_jm1;
				/* dfdx */
				temp[1] += d2fdxdy_im1_jm1_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += d2fdxdy_im1_jm1_w*dfdy_im1_jm1;
				/* d2fdxdy */
				temp[3] += d2fdxdy_im1_jm1_w*d2fdxdy_im1_jm1;
				/* d(im1_jm1)/d(im1_j) */
				temp=matrix_and_right_hand_side+
					im1_j*number_of_equations_plus_1+im1_jm1;
				/* d/d(f) */
				/* f */
				*temp += f_im1_j_w*f_im1_jm1;
				/* dfdx */
				temp[1] += f_im1_j_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += f_im1_j_w*dfdy_im1_jm1;
				/* d2fdxdy */
				temp[3] += f_im1_j_w*d2fdxdy_im1_jm1;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_im1_j_w*f_im1_jm1;
				/* dfdx */
				temp[1] += dfdx_im1_j_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += dfdx_im1_j_w*dfdy_im1_jm1;
				/* d2fdxdy */
				temp[3] += dfdx_im1_j_w*d2fdxdy_im1_jm1;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_im1_j_w*f_im1_jm1;
				/* dfdx */
				temp[1] += dfdy_im1_j_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += dfdy_im1_j_w*dfdy_im1_jm1;
				/* d2fdxdy */
				temp[3] += dfdy_im1_j_w*d2fdxdy_im1_jm1;
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += d2fdxdy_im1_j_w*f_im1_jm1;
				/* dfdx */
				temp[1] += d2fdxdy_im1_j_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += d2fdxdy_im1_j_w*dfdy_im1_jm1;
				/* d2fdxdy */
				temp[3] += d2fdxdy_im1_j_w*d2fdxdy_im1_jm1;
				/* d(im1_jm1)/d(i_j) */
				temp=matrix_and_right_hand_side+
					i_j*number_of_equations_plus_1+im1_jm1;
				/* d/d(f) */
				/* f */
				*temp += f_i_j_w*f_im1_jm1;
				/* dfdx */
				temp[1] += f_i_j_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += f_i_j_w*dfdy_im1_jm1;
				/* d2fdxdy */
				temp[3] += f_i_j_w*d2fdxdy_im1_jm1;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_i_j_w*f_im1_jm1;
				/* dfdx */
				temp[1] += dfdx_i_j_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += dfdx_i_j_w*dfdy_im1_jm1;
				/* d2fdxdy */
				temp[3] += dfdx_i_j_w*d2fdxdy_im1_jm1;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_i_j_w*f_im1_jm1;
				/* dfdx */
				temp[1] += dfdy_i_j_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += dfdy_i_j_w*dfdy_im1_jm1;
				/* d2fdxdy */
				temp[3] += dfdy_i_j_w*d2fdxdy_im1_jm1;
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += d2fdxdy_i_j_w*f_im1_jm1;
				/* dfdx */
				temp[1] += d2fdxdy_i_j_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += d2fdxdy_i_j_w*dfdy_im1_jm1;
				/* d2fdxdy */
				temp[3] += d2fdxdy_i_j_w*d2fdxdy_im1_jm1;
				/* d(im1_jm1)/d(i_jm1) */
				temp=matrix_and_right_hand_side+
					i_jm1*number_of_equations_plus_1+im1_jm1;
				/* d/d(f) */
				/* f */
				*temp += f_i_jm1_w*f_im1_jm1;
				/* dfdx */
				temp[1] += f_i_jm1_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += f_i_jm1_w*dfdy_im1_jm1;
				/* d2fdxdy */
				temp[3] += f_i_jm1_w*d2fdxdy_im1_jm1;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_i_jm1_w*f_im1_jm1;
				/* dfdx */
				temp[1] += dfdx_i_jm1_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += dfdx_i_jm1_w*dfdy_im1_jm1;
				/* d2fdxdy */
				temp[3] += dfdx_i_jm1_w*d2fdxdy_im1_jm1;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_i_jm1_w*f_im1_jm1;
				/* dfdx */
				temp[1] += dfdy_i_jm1_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += dfdy_i_jm1_w*dfdy_im1_jm1;
				/* d2fdxdy */
				temp[3] += dfdy_i_jm1_w*d2fdxdy_im1_jm1;
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += d2fdxdy_i_jm1_w*f_im1_jm1;
				/* dfdx */
				temp[1] += d2fdxdy_i_jm1_w*dfdx_im1_jm1;
				/* dfdy */
				temp[2] += d2fdxdy_i_jm1_w*dfdy_im1_jm1;
				/* d2fdxdy */
				temp[3] += d2fdxdy_i_jm1_w*d2fdxdy_im1_jm1;
				/* d(im1_j)/d(im1_jm1) */
				temp=matrix_and_right_hand_side+
					im1_jm1*number_of_equations_plus_1+im1_j;
				/* d/d(f) */
				/* f */
				*temp += f_im1_jm1_w*f_im1_j;
				/* dfdx */
				temp[1] += f_im1_jm1_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += f_im1_jm1_w*dfdy_im1_j;
				/* d2fdxdy */
				temp[3] += f_im1_jm1_w*d2fdxdy_im1_j;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_im1_jm1_w*f_im1_j;
				/* dfdx */
				temp[1] += dfdx_im1_jm1_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += dfdx_im1_jm1_w*dfdy_im1_j;
				/* d2fdxdy */
				temp[3] += dfdx_im1_jm1_w*d2fdxdy_im1_j;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_im1_jm1_w*f_im1_j;
				/* dfdx */
				temp[1] += dfdy_im1_jm1_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += dfdy_im1_jm1_w*dfdy_im1_j;
				/* d2fdxdy */
				temp[3] += dfdy_im1_jm1_w*d2fdxdy_im1_j;
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += d2fdxdy_im1_jm1_w*f_im1_j;
				/* dfdx */
				temp[1] += d2fdxdy_im1_jm1_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += d2fdxdy_im1_jm1_w*dfdy_im1_j;
				/* d2fdxdy */
				temp[3] += d2fdxdy_im1_jm1_w*d2fdxdy_im1_j;
				/* d(im1_j)/d(im1_j) */
				temp=matrix_and_right_hand_side+
					im1_j*number_of_equations_plus_1+im1_j;
				/* d/d(f) */
				/* f */
				*temp += f_im1_j_w*f_im1_j;
				/* dfdx */
				temp[1] += f_im1_j_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += f_im1_j_w*dfdy_im1_j;
				/* d2fdxdy */
				temp[3] += f_im1_j_w*d2fdxdy_im1_j;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_im1_j_w*f_im1_j;
				/* dfdx */
				temp[1] += dfdx_im1_j_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += dfdx_im1_j_w*dfdy_im1_j;
				/* d2fdxdy */
				temp[3] += dfdx_im1_j_w*d2fdxdy_im1_j;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_im1_j_w*f_im1_j;
				/* dfdx */
				temp[1] += dfdy_im1_j_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += dfdy_im1_j_w*dfdy_im1_j;
				/* d2fdxdy */
				temp[3] += dfdy_im1_j_w*d2fdxdy_im1_j;
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += d2fdxdy_im1_j_w*f_im1_j;
				/* dfdx */
				temp[1] += d2fdxdy_im1_j_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += d2fdxdy_im1_j_w*dfdy_im1_j;
				/* d2fdxdy */
				temp[3] += d2fdxdy_im1_j_w*d2fdxdy_im1_j;
				/* d(im1_j)/d(i_j) */
				temp=matrix_and_right_hand_side+
					i_j*number_of_equations_plus_1+im1_j;
				/* d/d(f) */
				/* f */
				*temp += f_i_j_w*f_im1_j;
				/* dfdx */
				temp[1] += f_i_j_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += f_i_j_w*dfdy_im1_j;
				/* d2fdxdy */
				temp[3] += f_i_j_w*d2fdxdy_im1_j;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_i_j_w*f_im1_j;
				/* dfdx */
				temp[1] += dfdx_i_j_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += dfdx_i_j_w*dfdy_im1_j;
				/* d2fdxdy */
				temp[3] += dfdx_i_j_w*d2fdxdy_im1_j;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_i_j_w*f_im1_j;
				/* dfdx */
				temp[1] += dfdy_i_j_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += dfdy_i_j_w*dfdy_im1_j;
				/* d2fdxdy */
				temp[3] += dfdy_i_j_w*d2fdxdy_im1_j;
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += d2fdxdy_i_j_w*f_im1_j;
				/* dfdx */
				temp[1] += d2fdxdy_i_j_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += d2fdxdy_i_j_w*dfdy_im1_j;
				/* d2fdxdy */
				temp[3] += d2fdxdy_i_j_w*d2fdxdy_im1_j;
				/* d(im1_j)/d(i_jm1) */
				temp=matrix_and_right_hand_side+
					i_jm1*number_of_equations_plus_1+im1_j;
				/* d/d(f) */
				/* f */
				*temp += f_i_jm1_w*f_im1_j;
				/* dfdx */
				temp[1] += f_i_jm1_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += f_i_jm1_w*dfdy_im1_j;
				/* d2fdxdy */
				temp[3] += f_i_jm1_w*d2fdxdy_im1_j;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_i_jm1_w*f_im1_j;
				/* dfdx */
				temp[1] += dfdx_i_jm1_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += dfdx_i_jm1_w*dfdy_im1_j;
				/* d2fdxdy */
				temp[3] += dfdx_i_jm1_w*d2fdxdy_im1_j;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_i_jm1_w*f_im1_j;
				/* dfdx */
				temp[1] += dfdy_i_jm1_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += dfdy_i_jm1_w*dfdy_im1_j;
				/* d2fdxdy */
				temp[3] += dfdy_i_jm1_w*d2fdxdy_im1_j;
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += d2fdxdy_i_jm1_w*f_im1_j;
				/* dfdx */
				temp[1] += d2fdxdy_i_jm1_w*dfdx_im1_j;
				/* dfdy */
				temp[2] += d2fdxdy_i_jm1_w*dfdy_im1_j;
				/* d2fdxdy */
				temp[3] += d2fdxdy_i_jm1_w*d2fdxdy_im1_j;
#if defined (FIX_APEX)
				}
#endif /* defined (FIX_APEX) */
#if defined (FIX_APEX)
				if ((SOCK==type)&&(im1_jm1==0))
				{
				/* d(i_j)/d(im1_jm1) */
				temp=matrix_and_right_hand_side+
					im1_jm1*number_of_equations_plus_1+i_j;
				/* d/d(f) */
				/* f */
				*temp += f_im1_jm1_w*f_i_j;
				/* dfdx */
				temp[1] += f_im1_jm1_w*dfdx_i_j;
				/* dfdy */
				temp[2] += f_im1_jm1_w*dfdy_i_j;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_im1_jm1_w*f_i_j;
				/* dfdx */
				temp[1] += dfdx_im1_jm1_w*dfdx_i_j;
				/* dfdy */
				temp[2] += dfdx_im1_jm1_w*dfdy_i_j;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_im1_jm1_w*f_i_j;
				/* dfdx */
				temp[1] += dfdy_im1_jm1_w*dfdx_i_j;
				/* dfdy */
				temp[2] += dfdy_im1_jm1_w*dfdy_i_j;
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += d2fdxdy_im1_jm1_w*f_i_j;
				/* dfdx */
				temp[1] += d2fdxdy_im1_jm1_w*dfdx_i_j;
				/* dfdy */
				temp[2] += d2fdxdy_im1_jm1_w*dfdy_i_j;
				/* d(i_j)/d(im1_j) */
				temp=matrix_and_right_hand_side+
					im1_j*number_of_equations_plus_1+i_j;
				/* d/d(f) */
				/* f */
				*temp += f_im1_j_w*f_i_j;
				/* dfdx */
				temp[1] += f_im1_j_w*dfdx_i_j;
				/* dfdy */
				temp[2] += f_im1_j_w*dfdy_i_j;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_im1_j_w*f_i_j;
				/* dfdx */
				temp[1] += dfdx_im1_j_w*dfdx_i_j;
				/* dfdy */
				temp[2] += dfdx_im1_j_w*dfdy_i_j;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_im1_j_w*f_i_j;
				/* dfdx */
				temp[1] += dfdy_im1_j_w*dfdx_i_j;
				/* dfdy */
				temp[2] += dfdy_im1_j_w*dfdy_i_j;
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += d2fdxdy_im1_j_w*f_i_j;
				/* dfdx */
				temp[1] += d2fdxdy_im1_j_w*dfdx_i_j;
				/* dfdy */
				temp[2] += d2fdxdy_im1_j_w*dfdy_i_j;
				}
				else
				{
#endif /* defined (FIX_APEX) */
				/* d(i_j)/d(im1_jm1) */
				temp=matrix_and_right_hand_side+
					im1_jm1*number_of_equations_plus_1+i_j;
				/* d/d(f) */
				/* f */
				*temp += f_im1_jm1_w*f_i_j;
				/* dfdx */
				temp[1] += f_im1_jm1_w*dfdx_i_j;
				/* dfdy */
				temp[2] += f_im1_jm1_w*dfdy_i_j;
				/* d2fdxdy */
				temp[3] += f_im1_jm1_w*d2fdxdy_i_j;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_im1_jm1_w*f_i_j;
				/* dfdx */
				temp[1] += dfdx_im1_jm1_w*dfdx_i_j;
				/* dfdy */
				temp[2] += dfdx_im1_jm1_w*dfdy_i_j;
				/* d2fdxdy */
				temp[3] += dfdx_im1_jm1_w*d2fdxdy_i_j;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_im1_jm1_w*f_i_j;
				/* dfdx */
				temp[1] += dfdy_im1_jm1_w*dfdx_i_j;
				/* dfdy */
				temp[2] += dfdy_im1_jm1_w*dfdy_i_j;
				/* d2fdxdy */
				temp[3] += dfdy_im1_jm1_w*d2fdxdy_i_j;
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += d2fdxdy_im1_jm1_w*f_i_j;
				/* dfdx */
				temp[1] += d2fdxdy_im1_jm1_w*dfdx_i_j;
				/* dfdy */
				temp[2] += d2fdxdy_im1_jm1_w*dfdy_i_j;
				/* d2fdxdy */
				temp[3] += d2fdxdy_im1_jm1_w*d2fdxdy_i_j;
				/* d(i_j)/d(im1_j) */
				temp=matrix_and_right_hand_side+
					im1_j*number_of_equations_plus_1+i_j;
				/* d/d(f) */
				/* f */
				*temp += f_im1_j_w*f_i_j;
				/* dfdx */
				temp[1] += f_im1_j_w*dfdx_i_j;
				/* dfdy */
				temp[2] += f_im1_j_w*dfdy_i_j;
				/* d2fdxdy */
				temp[3] += f_im1_j_w*d2fdxdy_i_j;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_im1_j_w*f_i_j;
				/* dfdx */
				temp[1] += dfdx_im1_j_w*dfdx_i_j;
				/* dfdy */
				temp[2] += dfdx_im1_j_w*dfdy_i_j;
				/* d2fdxdy */
				temp[3] += dfdx_im1_j_w*d2fdxdy_i_j;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_im1_j_w*f_i_j;
				/* dfdx */
				temp[1] += dfdy_im1_j_w*dfdx_i_j;
				/* dfdy */
				temp[2] += dfdy_im1_j_w*dfdy_i_j;
				/* d2fdxdy */
				temp[3] += dfdy_im1_j_w*d2fdxdy_i_j;
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += d2fdxdy_im1_j_w*f_i_j;
				/* dfdx */
				temp[1] += d2fdxdy_im1_j_w*dfdx_i_j;
				/* dfdy */
				temp[2] += d2fdxdy_im1_j_w*dfdy_i_j;
				/* d2fdxdy */
				temp[3] += d2fdxdy_im1_j_w*d2fdxdy_i_j;
#if defined (FIX_APEX)
				}
#endif /* defined (FIX_APEX) */
				/* d(i_j)/d(i_j) */
				temp=matrix_and_right_hand_side+
					i_j*number_of_equations_plus_1+i_j;
				/* d/d(f) */
				/* f */
				*temp += f_i_j_w*f_i_j;
				/* dfdx */
				temp[1] += f_i_j_w*dfdx_i_j;
				/* dfdy */
				temp[2] += f_i_j_w*dfdy_i_j;
				/* d2fdxdy */
				temp[3] += f_i_j_w*d2fdxdy_i_j;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_i_j_w*f_i_j;
				/* dfdx */
				temp[1] += dfdx_i_j_w*dfdx_i_j;
				/* dfdy */
				temp[2] += dfdx_i_j_w*dfdy_i_j;
				/* d2fdxdy */
				temp[3] += dfdx_i_j_w*d2fdxdy_i_j;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_i_j_w*f_i_j;
				/* dfdx */
				temp[1] += dfdy_i_j_w*dfdx_i_j;
				/* dfdy */
				temp[2] += dfdy_i_j_w*dfdy_i_j;
				/* d2fdxdy */
				temp[3] += dfdy_i_j_w*d2fdxdy_i_j;
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += d2fdxdy_i_j_w*f_i_j;
				/* dfdx */
				temp[1] += d2fdxdy_i_j_w*dfdx_i_j;
				/* dfdy */
				temp[2] += d2fdxdy_i_j_w*dfdy_i_j;
				/* d2fdxdy */
				temp[3] += d2fdxdy_i_j_w*d2fdxdy_i_j;
				/* d(i_j)/d(i_jm1) */
				temp=matrix_and_right_hand_side+
					i_jm1*number_of_equations_plus_1+i_j;
				/* d/d(f) */
				/* f */
				*temp += f_i_jm1_w*f_i_j;
				/* dfdx */
				temp[1] += f_i_jm1_w*dfdx_i_j;
				/* dfdy */
				temp[2] += f_i_jm1_w*dfdy_i_j;
				/* d2fdxdy */
				temp[3] += f_i_jm1_w*d2fdxdy_i_j;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_i_jm1_w*f_i_j;
				/* dfdx */
				temp[1] += dfdx_i_jm1_w*dfdx_i_j;
				/* dfdy */
				temp[2] += dfdx_i_jm1_w*dfdy_i_j;
				/* d2fdxdy */
				temp[3] += dfdx_i_jm1_w*d2fdxdy_i_j;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_i_jm1_w*f_i_j;
				/* dfdx */
				temp[1] += dfdy_i_jm1_w*dfdx_i_j;
				/* dfdy */
				temp[2] += dfdy_i_jm1_w*dfdy_i_j;
				/* d2fdxdy */
				temp[3] += dfdy_i_jm1_w*d2fdxdy_i_j;
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += d2fdxdy_i_jm1_w*f_i_j;
				/* dfdx */
				temp[1] += d2fdxdy_i_jm1_w*dfdx_i_j;
				/* dfdy */
				temp[2] += d2fdxdy_i_jm1_w*dfdy_i_j;
				/* d2fdxdy */
				temp[3] += d2fdxdy_i_jm1_w*d2fdxdy_i_j;
#if defined (FIX_APEX)
				if ((SOCK==type)&&(im1_jm1==0))
				{
				/* d(i_jm1)/d(im1_jm1) */
				temp=matrix_and_right_hand_side+
					im1_jm1*number_of_equations_plus_1+i_jm1;
				/* d/d(f) */
				/* f */
				*temp += f_im1_jm1_w*f_i_jm1;
				/* dfdx */
				temp[1] += f_im1_jm1_w*dfdx_i_jm1;
				/* dfdy */
				temp[2] += f_im1_jm1_w*dfdy_i_jm1;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_im1_jm1_w*f_i_jm1;
				/* dfdx */
				temp[1] += dfdx_im1_jm1_w*dfdx_i_jm1;
				/* dfdy */
				temp[2] += dfdx_im1_jm1_w*dfdy_i_jm1;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_im1_jm1_w*f_i_jm1;
				/* dfdx */
				temp[1] += dfdy_im1_jm1_w*dfdx_i_jm1;
				/* dfdy */
				temp[2] += dfdy_im1_jm1_w*dfdy_i_jm1;
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += d2fdxdy_im1_jm1_w*f_i_jm1;
				/* dfdx */
				temp[1] += d2fdxdy_im1_jm1_w*dfdx_i_jm1;
				/* dfdy */
				temp[2] += d2fdxdy_im1_jm1_w*dfdy_i_jm1;
				/* d(i_jm1)/d(im1_j) */
				temp=matrix_and_right_hand_side+
					im1_j*number_of_equations_plus_1+i_jm1;
				/* d/d(f) */
				/* f */
				*temp += f_im1_j_w*f_i_jm1;
				/* dfdx */
				temp[1] += f_im1_j_w*dfdx_i_jm1;
				/* dfdy */
				temp[2] += f_im1_j_w*dfdy_i_jm1;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_im1_j_w*f_i_jm1;
				/* dfdx */
				temp[1] += dfdx_im1_j_w*dfdx_i_jm1;
				/* dfdy */
				temp[2] += dfdx_im1_j_w*dfdy_i_jm1;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_im1_j_w*f_i_jm1;
				/* dfdx */
				temp[1] += dfdy_im1_j_w*dfdx_i_jm1;
				/* dfdy */
				temp[2] += dfdy_im1_j_w*dfdy_i_jm1;
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += d2fdxdy_im1_j_w*f_i_jm1;
				/* dfdx */
				temp[1] += d2fdxdy_im1_j_w*dfdx_i_jm1;
				/* dfdy */
				temp[2] += d2fdxdy_im1_j_w*dfdy_i_jm1;
				}
				else
				{
#endif /* defined (FIX_APEX) */
				/* d(i_jm1)/d(im1_jm1) */
				temp=matrix_and_right_hand_side+
					im1_jm1*number_of_equations_plus_1+i_jm1;
				/* d/d(f) */
				/* f */
				*temp += f_im1_jm1_w*f_i_jm1;
				/* dfdx */
				temp[1] += f_im1_jm1_w*dfdx_i_jm1;
				/* dfdy */
				temp[2] += f_im1_jm1_w*dfdy_i_jm1;
				/* d2fdxdy */
				temp[3] += f_im1_jm1_w*d2fdxdy_i_jm1;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_im1_jm1_w*f_i_jm1;
				/* dfdx */
				temp[1] += dfdx_im1_jm1_w*dfdx_i_jm1;
				/* dfdy */
				temp[2] += dfdx_im1_jm1_w*dfdy_i_jm1;
				/* d2fdxdy */
				temp[3] += dfdx_im1_jm1_w*d2fdxdy_i_jm1;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_im1_jm1_w*f_i_jm1;
				/* dfdx */
				temp[1] += dfdy_im1_jm1_w*dfdx_i_jm1;
				/* dfdy */
				temp[2] += dfdy_im1_jm1_w*dfdy_i_jm1;
				/* d2fdxdy */
				temp[3] += dfdy_im1_jm1_w*d2fdxdy_i_jm1;
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += d2fdxdy_im1_jm1_w*f_i_jm1;
				/* dfdx */
				temp[1] += d2fdxdy_im1_jm1_w*dfdx_i_jm1;
				/* dfdy */
				temp[2] += d2fdxdy_im1_jm1_w*dfdy_i_jm1;
				/* d2fdxdy */
				temp[3] += d2fdxdy_im1_jm1_w*d2fdxdy_i_jm1;
				/* d(i_jm1)/d(im1_j) */
				temp=matrix_and_right_hand_side+
					im1_j*number_of_equations_plus_1+i_jm1;
				/* d/d(f) */
				/* f */
				*temp += f_im1_j_w*f_i_jm1;
				/* dfdx */
				temp[1] += f_im1_j_w*dfdx_i_jm1;
				/* dfdy */
				temp[2] += f_im1_j_w*dfdy_i_jm1;
				/* d2fdxdy */
				temp[3] += f_im1_j_w*d2fdxdy_i_jm1;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_im1_j_w*f_i_jm1;
				/* dfdx */
				temp[1] += dfdx_im1_j_w*dfdx_i_jm1;
				/* dfdy */
				temp[2] += dfdx_im1_j_w*dfdy_i_jm1;
				/* d2fdxdy */
				temp[3] += dfdx_im1_j_w*d2fdxdy_i_jm1;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_im1_j_w*f_i_jm1;
				/* dfdx */
				temp[1] += dfdy_im1_j_w*dfdx_i_jm1;
				/* dfdy */
				temp[2] += dfdy_im1_j_w*dfdy_i_jm1;
				/* d2fdxdy */
				temp[3] += dfdy_im1_j_w*d2fdxdy_i_jm1;
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += d2fdxdy_im1_j_w*f_i_jm1;
				/* dfdx */
				temp[1] += d2fdxdy_im1_j_w*dfdx_i_jm1;
				/* dfdy */
				temp[2] += d2fdxdy_im1_j_w*dfdy_i_jm1;
				/* d2fdxdy */
				temp[3] += d2fdxdy_im1_j_w*d2fdxdy_i_jm1;
#if defined (FIX_APEX)
				}
#endif /* defined (FIX_APEX) */
				/* d(i_jm1)/d(i_j) */
				temp=matrix_and_right_hand_side+
					i_j*number_of_equations_plus_1+i_jm1;
				/* d/d(f) */
				/* f */
				*temp += f_i_j_w*f_i_jm1;
				/* dfdx */
				temp[1] += f_i_j_w*dfdx_i_jm1;
				/* dfdy */
				temp[2] += f_i_j_w*dfdy_i_jm1;
				/* d2fdxdy */
				temp[3] += f_i_j_w*d2fdxdy_i_jm1;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_i_j_w*f_i_jm1;
				/* dfdx */
				temp[1] += dfdx_i_j_w*dfdx_i_jm1;
				/* dfdy */
				temp[2] += dfdx_i_j_w*dfdy_i_jm1;
				/* d2fdxdy */
				temp[3] += dfdx_i_j_w*d2fdxdy_i_jm1;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_i_j_w*f_i_jm1;
				/* dfdx */
				temp[1] += dfdy_i_j_w*dfdx_i_jm1;
				/* dfdy */
				temp[2] += dfdy_i_j_w*dfdy_i_jm1;
				/* d2fdxdy */
				temp[3] += dfdy_i_j_w*d2fdxdy_i_jm1;
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += d2fdxdy_i_j_w*f_i_jm1;
				/* dfdx */
				temp[1] += d2fdxdy_i_j_w*dfdx_i_jm1;
				/* dfdy */
				temp[2] += d2fdxdy_i_j_w*dfdy_i_jm1;
				/* d2fdxdy */
				temp[3] += d2fdxdy_i_j_w*d2fdxdy_i_jm1;
				/* d(i_jm1)/d(i_jm1) */
				temp=matrix_and_right_hand_side+
					i_jm1*number_of_equations_plus_1+i_jm1;
				/* d/d(f) */
				/* f */
				*temp += f_i_jm1_w*f_i_jm1;
				/* dfdx */
				temp[1] += f_i_jm1_w*dfdx_i_jm1;
				/* dfdy */
				temp[2] += f_i_jm1_w*dfdy_i_jm1;
				/* d2fdxdy */
				temp[3] += f_i_jm1_w*d2fdxdy_i_jm1;
				/* d/d(dfdx) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdx_i_jm1_w*f_i_jm1;
				/* dfdx */
				temp[1] += dfdx_i_jm1_w*dfdx_i_jm1;
				/* dfdy */
				temp[2] += dfdx_i_jm1_w*dfdy_i_jm1;
				/* d2fdxdy */
				temp[3] += dfdx_i_jm1_w*d2fdxdy_i_jm1;
				/* d/d(dfdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += dfdy_i_jm1_w*f_i_jm1;
				/* dfdx */
				temp[1] += dfdy_i_jm1_w*dfdx_i_jm1;
				/* dfdy */
				temp[2] += dfdy_i_jm1_w*dfdy_i_jm1;
				/* d2fdxdy */
				temp[3] += dfdy_i_jm1_w*d2fdxdy_i_jm1;
				/* d/d(d2fdxdy) */
				temp += number_of_equations_plus_1;
				/* f */
				*temp += d2fdxdy_i_jm1_w*f_i_jm1;
				/* dfdx */
				temp[1] += d2fdxdy_i_jm1_w*dfdx_i_jm1;
				/* dfdy */
				temp[2] += d2fdxdy_i_jm1_w*dfdy_i_jm1;
				/* d2fdxdy */
				temp[3] += d2fdxdy_i_jm1_w*d2fdxdy_i_jm1;
				/* end data */
				/* update the right hand side */
#if defined (FIX_APEX)
				if ((SOCK==type)&&(im1_jm1==0))
				{
					/* d/d(im1_jm1) */
					temp=matrix_and_right_hand_side+number_of_equations;
					/* f */
					*temp += f_im1_jm1_w*f_d;
					temp += number_of_equations_plus_1;
					/* dfdx */
					*temp += dfdx_im1_jm1_w*f_d;
					temp += number_of_equations_plus_1;
					/* dfdy */
					*temp += dfdy_im1_jm1_w*f_d;
					temp += number_of_equations_plus_1;
					/* d/d(im1_j) */
					temp=matrix_and_right_hand_side+number_of_equations;
					/* f */
					*temp += f_im1_j_w*f_d;
					temp += number_of_equations_plus_1;
					/* dfdx */
					*temp += dfdx_im1_j_w*f_d;
					temp += number_of_equations_plus_1;
					/* dfdy */
					*temp += dfdy_im1_j_w*f_d;
					temp += number_of_equations_plus_1;
				}
				else
				{
#endif /* defined (FIX_APEX) */
					/* d/d(im1_jm1) */
					temp=matrix_and_right_hand_side+
						im1_jm1*number_of_equations_plus_1+number_of_equations;
					/* f */
					*temp += f_im1_jm1_w*f_d;
					temp += number_of_equations_plus_1;
					/* dfdx */
					*temp += dfdx_im1_jm1_w*f_d;
					temp += number_of_equations_plus_1;
					/* dfdy */
					*temp += dfdy_im1_jm1_w*f_d;
					temp += number_of_equations_plus_1;
					/* d2fdxdy */
					*temp += d2fdxdy_im1_jm1_w*f_d;
					/* d/d(im1_j) */
					temp=matrix_and_right_hand_side+
						im1_j*number_of_equations_plus_1+number_of_equations;
					/* f */
					*temp += f_im1_j_w*f_d;
					temp += number_of_equations_plus_1;
					/* dfdx */
					*temp += dfdx_im1_j_w*f_d;
					temp += number_of_equations_plus_1;
					/* dfdy */
					*temp += dfdy_im1_j_w*f_d;
					temp += number_of_equations_plus_1;
					/* d2fdxdy */
					*temp += d2fdxdy_im1_j_w*f_d;
#if defined (FIX_APEX)
				}
#endif /* defined (FIX_APEX) */
				/* d/d(i_j) */
				temp=matrix_and_right_hand_side+
					i_j*number_of_equations_plus_1+number_of_equations;
				/* f */
				*temp += f_i_j_w*f_d;
				temp += number_of_equations_plus_1;
				/* dfdx */
				*temp += dfdx_i_j_w*f_d;
				temp += number_of_equations_plus_1;
				/* dfdy */
				*temp += dfdy_i_j_w*f_d;
				temp += number_of_equations_plus_1;
				/* d2fdxdy */
				*temp += d2fdxdy_i_j_w*f_d;
				/* d/d(i_jm1) */
				temp=matrix_and_right_hand_side+
					i_jm1*number_of_equations_plus_1+number_of_equations;
				/* f */
				*temp += f_i_jm1_w*f_d;
				temp += number_of_equations_plus_1;
				/* dfdx */
				*temp += dfdx_i_jm1_w*f_d;
				temp += number_of_equations_plus_1;
				/* dfdy */
				*temp += dfdy_i_jm1_w*f_d;
				temp += number_of_equations_plus_1;
				/* d2fdxdy */
				*temp += d2fdxdy_i_jm1_w*f_d;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"assemble_linear_equations.  Data point outside of mesh");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"assemble_linear_equations.  Data point outside of mesh");
		}
	}
	LEAVE;

	return (return_code);
} /* assemble_linear_equations */

/*YUP*/
static int solve_linear_equations(int number_of_equations,
#if defined (DOUBLE_MATRIX)
	double
#else
  float
#endif /* defined (DOUBLE_MATRIX) */
    *matrix_and_right_hand_side,enum Region_type region_type,int number_of_nodes,
  int number_of_mesh_rows,int number_of_mesh_columns,
#if defined (FIX_APEX)
#if defined (DOUBLE_MATRIX)
  double *cosine_apex,double *sine_apex,
#else
  float *cosine_apex,float *sine_apex,
#endif /* defined (DOUBLE_MATRIX) */
#endif /* defined (FIX_APEX) */
  float **f,float **dfdx,float **dfdy,float **d2fdxdy)
/*******************************************************************************
LAST MODIFIED : 26 March 1997

DESCRIPTION :
==============================================================================*/
{
	int *column_index,i,j,k,*k_index,number_of_equations_plus_1,offset,*pivot,
		return_code,*row_index,*row_offset,*row_offset_base,temp_int;
	float *dfdx_n,*dfdy_n,*d2fdxdy_n,*f_n;
#if defined (DOUBLE_MATRIX)
	double
#else
		float
#endif /* defined (DOUBLE_MATRIX) */
		abs,*column,column_max,*matrix,*right_hand_side,*row,row_max,
		*row_scaling,*row_scaling_base,temp_real;
#if defined (DOUBLE_MATRIX) || defined (DOUBLE_INNER_PRODUCT)
	double
#else
		float
#endif /* defined (DOUBLE_MATRIX) || defined (DOUBLE_INNER_PRODUCT) */
		sum;

	ENTER(solve_linear_equations);
	if ((ALLOCATE(row_offset_base,int,number_of_equations))&&
#if defined (DOUBLE_MATRIX)
		(ALLOCATE(row_scaling_base,double,number_of_equations)))

#endif /* defined (DOUBLE_MATRIX) */
	{
		number_of_equations_plus_1=number_of_equations+1;
		/* find the row maximums (for implicit scaling) and set up the row offset
			 array (for partial pivotting) */
		matrix=matrix_and_right_hand_side;
		i=number_of_equations;
		return_code=1;
		offset=0;
		row_offset=row_offset_base;
		row_scaling=row_scaling_base;
		while ((i>0)&&return_code)
		{
			*row_offset=offset;
			row_offset++;
			offset += number_of_equations_plus_1;
#if defined (DOUBLE_MATRIX)
			row_max=0.;
#else
			row_max=(float)0.;
#endif /* defined (DOUBLE_MATRIX) */
			for (j=number_of_equations;j>0;j--)
			{
				if ((abs= *matrix)<0)
				{
					abs= -abs;
				}
				if (abs>row_max)
				{
					row_max=abs;
				}
				matrix++;
			}
			if (row_max==0)
			{
				display_message(ERROR_MESSAGE,
					"solve_linear_equations.  Singular matrix");
				return_code=0;
			}
			else
			{
				*row_scaling=1/row_max;
				row_scaling++;
                                /* skip right hand side */
				matrix++;
				i--;
			}
		}
		if (return_code)
		{
			/* Crout LU factorization */
			column=matrix_and_right_hand_side;
			column_index=row_offset_base;
			/* for j=1 to n */
			j=number_of_equations;
			while ((j>0)&&return_code)
			{
				row_index=row_offset_base;
                                /* for i=1 to j-1 */
				for (i=number_of_equations-j;i>0;i--)
				{
					row=matrix_and_right_hand_side+(*row_index);
					k_index=row_offset_base;
#if defined (DOUBLE_MATRIX) || defined (DOUBLE_INNER_PRODUCT)
					sum=0.;
#else
					sum=(float)0.;
#endif /* defined (DOUBLE_MATRIX) || defined (DOUBLE_INNER_PRODUCT) */
					/* for k=1 to i-1 */
					for (k=number_of_equations-j-i;k>0;k--)
					{
						/* sum=sum+a(i,k)*a(k,j) */
#if defined (DOUBLE_INNER_PRODUCT)
						sum += (double)(*row)*(double)column[*k_index];
#else
						sum += (*row)*column[*k_index];
#endif /* defined (DOUBLE_INNER_PRODUCT) */
						row++;
						k_index++;
					}
					/* a(i,j)=a(i,j)-sum */
					column[*k_index] -=
#if defined (DOUBLE_INNER_PRODUCT)
						(float)
#endif /* defined (DOUBLE_INNER_PRODUCT) */
						sum;
					row_index++;
				}
                                /* initialize search for pivot */
#if defined (DOUBLE_MATRIX)
				column_max=0.;
#else
				column_max=(float)0.;
#endif /* defined (DOUBLE_MATRIX) */
                                /* for i=j to n */
				for (i=j;i>0;i--)
				{
					row=matrix_and_right_hand_side+(*row_index);
					k_index=row_offset_base;
#if defined (DOUBLE_MATRIX) || defined (DOUBLE_INNER_PRODUCT)
					sum=0.;
#else
					sum=(float)0.;
#endif /* defined (DOUBLE_MATRIX) || defined (DOUBLE_INNER_PRODUCT) */
					/* for k=1 to j-1 */
					for (k=number_of_equations-j;k>0;k--)
					{
						/* sum=sum+a(i,k)*a(k,j) */
#if defined (DOUBLE_INNER_PRODUCT)
						sum += (double)(*row)*(double)column[*k_index];
#else
						sum += (*row)*column[*k_index];
#endif /* defined (DOUBLE_INNER_PRODUCT) */
						row++;
						k_index++;
					}
					/* a(i,j)=a(i,j)-sum */
					*row -=
#if defined (DOUBLE_INNER_PRODUCT)
						(float)
#endif /* defined (DOUBLE_INNER_PRODUCT) */
						sum;
					/* update pivot */
					if ((abs= *row)<0)
					{
						abs= -abs;
					}
					abs *= row_scaling_base[(*row_index)/number_of_equations_plus_1];
					if (abs>=column_max)
					{
						column_max=abs;
						pivot=row_index;
					}
					row_index++;
				}
                                /* if pivot_row<>j */
				if (*pivot!= *column_index)
				{
					/* perform pivot */
					temp_int= *pivot;
					*pivot= *column_index;
					*column_index=temp_int;
				}
				if ((temp_real=column[*column_index])==0)
				{
					display_message(ERROR_MESSAGE,
						"solve_linear_equations.  Singular matrix");
					return_code=0;
				}
				else
				{
					temp_real=1/temp_real;
					row_index=column_index+1;
					/* for i=j+1 to n */
					for (i=j-1;i>0;i--)
					{
						/* a(i,j)=a(i,j)/a(j,j) */
						column[*row_index] *= temp_real;
						row_index++;
					}
					column++;
					column_index++;
					j--;
				}
			}
			if (return_code)
			{
				right_hand_side=column;
                                /* forward substitution */
                                /* for i=2 to n */
				row_index=row_offset_base+1;
				for (i=number_of_equations-1;i>0;i--)
				{
					column_index=row_offset_base;
#if defined (DOUBLE_MATRIX) || defined (DOUBLE_INNER_PRODUCT)
					sum=0.;
#else
					sum=(float)0.;
#endif /* defined (DOUBLE_MATRIX) || defined (DOUBLE_INNER_PRODUCT) */
					row=matrix_and_right_hand_side+(*row_index);
					/* for j=1 to i-1 */
					for (j=number_of_equations-i;j>0;j--)
					{
						/* sum=sum+a(i,j)*b(j) */
#if defined (DOUBLE_INNER_PRODUCT)
						sum += (double)(*row)*(double)right_hand_side[*column_index];
#else
						sum += (*row)*right_hand_side[*column_index];
#endif /* defined (DOUBLE_INNER_PRODUCT) */
						column_index++;
						row++;
					}
					/* b(i)=b(i)-sum */
					right_hand_side[*column_index] -=
#if defined (DOUBLE_INNER_PRODUCT)
						(float)
#endif /* defined (DOUBLE_INNER_PRODUCT) */
						sum;
					row_index++;
				}
                                /* backward substitution */
                                /* for i=n to 1 step -1 */
				row_index=row_offset_base+number_of_equations-1;
				for (i=number_of_equations;i>0;i--)
				{
					column_index=row_offset_base+number_of_equations-1;
#if defined (DOUBLE_MATRIX) || defined (DOUBLE_INNER_PRODUCT)
					sum=0.;
#else
					sum=(float)0.;
#endif /* defined (DOUBLE_MATRIX) || defined (DOUBLE_INNER_PRODUCT) */
					row=matrix_and_right_hand_side+(*row_index)+number_of_equations-1;
					/* for j=i+1 to n */
					for (j=number_of_equations-i;j>0;j--)
					{
						/* sum=sum+a(i,j)*b(j) */
#if defined (DOUBLE_INNER_PRODUCT)
						sum += (double)(*row)*(double)right_hand_side[*column_index];
#else
						sum += (*row)*right_hand_side[*column_index];
#endif /* defined (DOUBLE_INNER_PRODUCT) */
						column_index--;
						row--;
					}
					/* b(i)=(b(i)-sum)/a(i,i) */
					right_hand_side[*column_index] -=
#if defined (DOUBLE_INNER_PRODUCT)
						(float)
#endif /* defined (DOUBLE_INNER_PRODUCT) */
						sum;
					right_hand_side[*column_index] /= (*row);
					row_index--;
				}
                                /* separate out the nodal values */
				if ((ALLOCATE(f_n,float,number_of_nodes))&&
					(ALLOCATE(dfdx_n,float,number_of_nodes))&&
					(ALLOCATE(dfdy_n,float,number_of_nodes))&&
					(ALLOCATE(d2fdxdy_n,float,number_of_nodes)))
				{
					column_index=row_offset_base;
					*f=f_n;
					*dfdx=dfdx_n;
					*dfdy=dfdy_n;
					*d2fdxdy=d2fdxdy_n;
					switch (region_type)
					{
						case SOCK:
						{
							for (j=0;j<=number_of_mesh_columns;j++)
							{
								*f_n=(float)(right_hand_side[*column_index]);
								f_n++;
#if defined (FIX_APEX)
								*dfdx_n=0;
								dfdx_n++;
								*dfdy_n=(float)(sine_apex[j]*right_hand_side[column_index[1]]+
									cosine_apex[j]*right_hand_side[column_index[2]]);
								dfdy_n++;
								*d2fdxdy_n=0;
								d2fdxdy_n++;
#else
								*dfdx_n=(float)(right_hand_side[column_index[1]]);
								dfdx_n++;
								*dfdy_n=(float)(right_hand_side[column_index[2]]);
								dfdy_n++;
								*d2fdxdy_n=(float)(right_hand_side[column_index[3]]);
								d2fdxdy_n++;
#endif /* defined (FIX_APEX) */
							}
#if defined (FIX_APEX)
							column_index += 3;
#else
							column_index += 4;
#endif /* defined (FIX_APEX) */
							for (i=1;i<=number_of_mesh_rows;i++)
							{
								*f_n=(float)(right_hand_side[*column_index]);
								f_n[number_of_mesh_columns]= *f_n;
								column_index++;
								f_n++;
								*dfdx_n=(float)(right_hand_side[*column_index]);
								dfdx_n[number_of_mesh_columns]= *dfdx_n;
								column_index++;
								dfdx_n++;
								*dfdy_n=(float)(right_hand_side[*column_index]);
								dfdy_n[number_of_mesh_columns]= *dfdy_n;
								column_index++;
								dfdy_n++;
								*d2fdxdy_n=(float)(right_hand_side[*column_index]);
								d2fdxdy_n[number_of_mesh_columns]= *d2fdxdy_n;
								column_index++;
								d2fdxdy_n++;
								for (j=1;j<number_of_mesh_columns;j++)
								{
									*f_n=(float)(right_hand_side[*column_index]);
									column_index++;
									f_n++;
									*dfdx_n=(float)(right_hand_side[*column_index]);
									column_index++;
									dfdx_n++;
									*dfdy_n=(float)(right_hand_side[*column_index]);
									column_index++;
									dfdy_n++;
									*d2fdxdy_n=(float)(right_hand_side[*column_index]);
									column_index++;
									d2fdxdy_n++;
								}
								f_n++;
								dfdx_n++;
								dfdy_n++;
								d2fdxdy_n++;
							}
						} break;
						case PATCH:
						{
							for (i=number_of_nodes;i>0;i--)
							{
								*f_n=(float)(right_hand_side[*column_index]);
								column_index++;
								f_n++;
								*dfdx_n=(float)(right_hand_side[*column_index]);
								column_index++;
								dfdx_n++;
								*dfdy_n=(float)(right_hand_side[*column_index]);
								column_index++;
								dfdy_n++;
								*d2fdxdy_n=(float)(right_hand_side[*column_index]);
								column_index++;
								d2fdxdy_n++;
							}
						} break;
						case TORSO:
						{
							for (i=0;i<=number_of_mesh_rows;i++)
							{
								*f_n=(float)(right_hand_side[*column_index]);
								f_n[number_of_mesh_columns]= *f_n;
								column_index++;
								f_n++;
								*dfdx_n=(float)(right_hand_side[*column_index]);
								dfdx_n[number_of_mesh_columns]= *dfdx_n;
								column_index++;
								dfdx_n++;
								*dfdy_n=(float)(right_hand_side[*column_index]);
								dfdy_n[number_of_mesh_columns]= *dfdy_n;
								column_index++;
								dfdy_n++;
								*d2fdxdy_n=(float)(right_hand_side[*column_index]);
								d2fdxdy_n[number_of_mesh_columns]= *d2fdxdy_n;
								column_index++;
								d2fdxdy_n++;
								for (j=1;j<number_of_mesh_columns;j++)
								{
									*f_n=(float)(right_hand_side[*column_index]);
									column_index++;
									f_n++;
									*dfdx_n=(float)(right_hand_side[*column_index]);
									column_index++;
									dfdx_n++;
									*dfdy_n=(float)(right_hand_side[*column_index]);
									column_index++;
									dfdy_n++;
									*d2fdxdy_n=(float)(right_hand_side[*column_index]);
									column_index++;
									d2fdxdy_n++;
								}
								f_n++;
								dfdx_n++;
								dfdy_n++;
								d2fdxdy_n++;
							}
						} break;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"solve_linear_equations.  %s",
						"Could not allocate memory for nodal values and derivatives");
					return_code=0;
				}
			}
		}
		/* free the memory for the working arrays */
		DEALLOCATE(row_offset_base);
		DEALLOCATE(row_scaling_base);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"solve_linear_equations.  Could not allocate memory for working arrays");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* solve_linear_equations */

/*
Global functions
----------------
*/
struct Interpolation_function *calculate_interpolation_functio(
	enum Map_type map_type,struct Rig *rig,struct Region *region,
	int *event_number_address,float potential_time,int *datum_address,
	int *start_search_interval_address,int *end_search_interval_address,
	float half_peak_to_peak_interval_width,char undecided_accepted,
	int finite_element_mesh_rows,int finite_element_mesh_columns,
	float membrane_smoothing,float plate_bending_smoothing)
/*******************************************************************************
LAST MODIFIED : 26 June 2003

DESCRIPTION :
There are three groups of arguments for this function
???Put into structures ?
Input
1. data - <number_of_data>, <x>, <y>, <value> and <weight>
2. finite element mesh - <number_of_rows>, <x_mesh>, <number_of_columns> and
   <y_mesh>
Output
3. interpolation function - <f>, <dfdx>, <dfdy> and <d2fdxdy>
???I'm not sure if memory should be assigned inside this function for these or
if they should be 1-D or 2-D arrays ?
==============================================================================*/
{
	double integral;
#if defined (FIX_APEX)
#if defined (DOUBLE_MATRIX)
	double *cosine_apex=(double *)NULL;
	double *sine_apex=(double *)NULL;
#else
	float *cosine_apex=(float *)NULL;
	float *sine_apex=(float *)NULL;,
#endif /* defined (DOUBLE_MATRIX) */	  
#endif /* defined (FIX_APEX) */
#if defined (DOUBLE_MATRIX)
		double *matrix_and_right_hand_side=(double *)NULL;
#else
		float *matrix_and_right_hand_side=(float *)NULL;
#endif /* defined (DOUBLE_MATRIX) */		
		int after,before,column,datum,end_search_interval,event_number,found,
			half_peak_to_peak_interval_width_samples,i,i_j,i_jm1,im1_j,im1_jm1,j,
			middle,number_of_columns,number_of_data,number_of_devices,
			number_of_equations,number_of_nodes,number_of_rows,number_of_signals,
			number_of_valid_data,row,start_search_interval;
		int *times=(int *)NULL;
		float absolute_error,average_absolute_error,dfdx_i_j,dfdx_i_jm1,
			dfdx_im1_j,dfdx_im1_jm1,dfdy_i_j,dfdy_i_jm1,dfdy_im1_j,dfdy_im1_jm1,
			d2fdxdy_i_j,d2fdxdy_i_jm1,d2fdxdy_im1_j,d2fdxdy_im1_jm1,
			f_approx,f_i_j,f_i_jm1,f_im1_j,f_im1_jm1,f_max,f_min,focus,
			frequency,height,h01_u,h01_v,h02_u,h02_v,h11_u,h11_v,h12_u,h12_v,lambda,
			max_data_x,max_data_y,maximum_absolute_error,min_data_x,min_data_y,
			peak_max_float,peak_min_float,peak_val_float,potential_time_freq,
			proportion,r,step_x,step_y,u,v,width,x,y,z;
		float *dfdx=(float *)NULL;
		float *dfdy=(float *)NULL;
		float *d2fdxdy=(float *)NULL;
		float *f=(float *)NULL;
		float *float_value=(float *)NULL;
		float *value=(float *)NULL;
		float *value_base=(float *)NULL;
		float *weight=(float *)NULL;
		float *weight_base=(float *)NULL;
		float *x_data=(float *)NULL;
		float *x_data_base=(float *)NULL;
		float *x_mesh=(float *)NULL;
		float *y_data=(float *)NULL;
		float *y_data_base=(float *)NULL;
		float *y_mesh=(float *)NULL;
		Linear_transformation *linear_trans=(Linear_transformation *)NULL;
		short int peak_max_short,peak_min_short,peak_val_short,
			*short_int_value=(short int *)NULL;
		struct Interpolation_function
			*function=(struct Interpolation_function *)NULL;
		struct Device **device=(struct Device **)NULL;
		struct Position *position=(struct Position *)NULL;
		struct Device_description *description=(struct Device_description *)NULL;
		struct Event *event=(struct Event *)NULL;
		struct Signal *signal=(struct Signal *)NULL;
		struct Signal_buffer *buffer=(struct Signal_buffer *)NULL;

		ENTER(calculate_interpolation_functio);
		if (rig&&region&&(
			((map_type==SINGLE_ACTIVATION)&&
			((event_number= *event_number_address)>=1)&&
			((datum= *datum_address)>=0))||
			((map_type==ACTIVATION_POTENTIAL)&&
			((event_number= *event_number_address)>=1))||
			((map_type==MULTIPLE_ACTIVATION)&&((datum= *datum_address)>=0))||
			(map_type==POTENTIAL)||
			(((map_type==INTEGRAL)&&
			((start_search_interval= *start_search_interval_address)>=0)&&
			((end_search_interval= *end_search_interval_address)>=
			start_search_interval)))
			))
		{	
	
			/* count the number of data points */
			number_of_devices=rig->number_of_devices;
			device=rig->devices;
			number_of_data=0;
			switch (map_type)
			{
				case SINGLE_ACTIVATION:
				case ACTIVATION_POTENTIAL:
				{
					for (i=number_of_devices;i>0;i--)
					{
						if (((description=(*device)->description)->type==ELECTRODE)&&
							(description->region==region)&&(signal=(*device)->signal)&&
							(buffer=signal->buffer)&&(times=buffer->times)&&
							(event=signal->first_event))
						{
							while (event&&(event->number<event_number))
							{
								event=event->next;
							}
							if (event&&(event->number==event_number)&&
								((event->status==ACCEPTED)||(undecided_accepted&&
								(event->status==UNDECIDED)))&&(0<=event->time)&&
								(event->time<buffer->number_of_samples))
							{
								number_of_data++;
							}
						}
						device++;
					}
				} break;
				case MULTIPLE_ACTIVATION:
				{
					for (i=number_of_devices;i>0;i--)
					{
						if (((description=(*device)->description)->type==ELECTRODE)&&
							(description->region==region)&&
							(event=(*device)->signal->first_event))
						{
							while (event&&
								!((event->status==ACCEPTED)||(undecided_accepted&&
									(event->status==UNDECIDED))))
							{
								event=event->next;
							}
							if (event)
							{
								number_of_data++;
							}
						}
						device++;
					}
				} break;
				case POTENTIAL:
				{
					for (i=number_of_devices;i>0;i--)
					{
						if (((description=(*device)->description)->type==ELECTRODE)&&
							(description->region==region)&&(signal=(*device)->signal)&&
							((signal->status==ACCEPTED)||
							(undecided_accepted&&(signal->status==UNDECIDED)))&&
							(buffer=signal->buffer)&&(times=buffer->times)&&
							((float)(times[0])*1000/(buffer->frequency)<=potential_time)&&
							(potential_time<=(float)(times[(buffer->number_of_samples)-1])*
							1000/(buffer->frequency)))
						{
							number_of_data++;
						}
						device++;
					}
				} break;
				case INTEGRAL:
				{
					for (i=number_of_devices;i>0;i--)
					{
						if (((description=(*device)->description)->type==ELECTRODE)&&
							(description->region==region)&&(signal=(*device)->signal)&&
							(0<=start_search_interval)&&
							(start_search_interval<signal->buffer->number_of_samples)&&
							((signal->status==ACCEPTED)||
							(undecided_accepted&&(signal->status==UNDECIDED))))
						{
							number_of_data++;
						}
						device++;
					}
				} break;
			}
			if (number_of_data>0)
			{
				/* allocate memory for storing the data */
				if ((ALLOCATE(x_data_base,float,number_of_data))&&
					(ALLOCATE(y_data_base,float,number_of_data))&&
					(ALLOCATE(value_base,float,number_of_data))&&
					(ALLOCATE(weight_base,float,number_of_data)))
				{
					/* assign data */
					x_data=x_data_base;
					y_data=y_data_base;
					value=value_base;
					weight=weight_base;
					device=rig->devices;
					frequency=(*device)->signal->buffer->frequency;
					times=(*device)->signal->buffer->times;
					if (region->type==SOCK)
					{
						focus=region->properties.sock.focus;
						linear_trans=region->properties.sock.linear_transformation;
					}
					if (half_peak_to_peak_interval_width>0)
					{
						half_peak_to_peak_interval_width_samples=(int)(frequency*
							half_peak_to_peak_interval_width/(float)1000.+0.5);
					}
					else
					{
						half_peak_to_peak_interval_width_samples=0;
					}
					i=number_of_data;
					switch (map_type)
					{
						case SINGLE_ACTIVATION:
						case ACTIVATION_POTENTIAL:
						{
							while (i>0)
							{
								if (((description=(*device)->description)->type==ELECTRODE)&&
									(description->region==region)&&(signal=(*device)->signal)&&
									(buffer=signal->buffer)&&(times=buffer->times)&&
									(event=signal->first_event))
								{
									while (event&&(event->number<event_number))
									{
										event=event->next;
									}
									if (event&&(event->number==event_number)&&
										((event->status==ACCEPTED)||(undecided_accepted&&
										(event->status==UNDECIDED)))&&(0<=event->time)&&
										(event->time<buffer->number_of_samples))
									{
										/* perform projection */
										position= &(description->properties.electrode.position);
										switch (region->type)
										{
											case SOCK:
											{
												linear_transformation(linear_trans,position->x,
													position->y,position->z,&x,&y,&z);
												cartesian_to_prolate_spheroidal(x,y,z,focus,&lambda,
													y_data,x_data,(float *)NULL);
											} break;
											case PATCH:
											{
												*x_data=position->x;
												*y_data=position->y;
											} break;
											case TORSO:
											{
												cartesian_to_cylindrical_polar(position->x,position->y,
													position->z,&r,x_data,y_data,(float *)NULL);
											} break;
										}
										switch (map_type)
										{
											case SINGLE_ACTIVATION:
											{
												/* calculate activation time */
												*value=(float)(times[event->time]-times[datum])*
													(float)1000./frequency;
											} break;
											case ACTIVATION_POTENTIAL:
											{
#if defined (OLD_CODE)
												switch (signal->buffer->value_type)
												{
													case SHORT_INT_VALUE:
													{
														*value=(float)(signal->buffer->signals.
															short_int_values)[(event->time)*
															(signal->buffer->number_of_signals)+
															(signal->index)];
													} break;
													case FLOAT_VALUE:
													{
														*value=(signal->buffer->signals.float_values)[
															(event->time)*(signal->buffer->number_of_signals)+
															(signal->index)];
													} break;
												}
												*value=((*value)-((*device)->channel->offset))*
													((*device)->channel->gain);
#endif /* defined (OLD_CODE) */
												/* calculate peak to peak voltage for an interval
													centred on the activation time */
												start_search_interval=(event->time)-
													half_peak_to_peak_interval_width_samples;
												if (start_search_interval<0)
												{
													start_search_interval=0;
												}
												end_search_interval=(event->time)+
													half_peak_to_peak_interval_width_samples;
												if (end_search_interval>
													signal->buffer->number_of_samples)
												{
													end_search_interval=
														signal->buffer->number_of_samples;
												}
												number_of_signals=signal->buffer->number_of_signals;
												switch (signal->buffer->value_type)
												{
													case SHORT_INT_VALUE:
													{
														short_int_value=
															(signal->buffer->signals.short_int_values)+
															(start_search_interval*number_of_signals+
															(signal->index));
														peak_min_short= *short_int_value;
														peak_max_short=peak_min_short;
														for (j=end_search_interval-start_search_interval;
															j>0;j--)
														{
															short_int_value += number_of_signals;
															peak_val_short= *short_int_value;
															if (peak_val_short<peak_min_short)
															{
																peak_min_short=peak_val_short;
															}
															else
															{
																if (peak_val_short>peak_max_short)
																{
																	peak_max_short=peak_val_short;
																}
															}
														}
														*value=(float)(peak_max_short-peak_min_short);
													} break;
													case FLOAT_VALUE:
													{
														float_value=
															(signal->buffer->signals.float_values)+
															(start_search_interval*number_of_signals+
															(signal->index));
														peak_min_float= *float_value;
														peak_max_float=peak_min_float;
														for (j=end_search_interval-start_search_interval;
															j>0;j--)
														{
															float_value += number_of_signals;
															peak_val_float= *float_value;
															if (peak_val_float<peak_min_float)
															{
																peak_min_float=peak_val_float;
															}
															else
															{
																if (peak_val_float>peak_max_float)
																{
																	peak_max_float=peak_val_float;
																}
															}
														}
														*value=peak_max_float-peak_min_float;
													} break;
												}
												*value *= (*device)->channel->gain;
											} break;
										}
										x_data++;
										y_data++;
										value++;
										/* assign weight */
										*weight=(float)1;
										weight++;
										i--;
									}
								}
								device++;
							}
						} break;
						case MULTIPLE_ACTIVATION:
						{
							while (i>0)
							{
								if (((description=(*device)->description)->type==ELECTRODE)&&
									(description->region==region)&&
									(event=(*device)->signal->first_event))
								{
									found=0;
									while (event)
									{
										if ((event->status==ACCEPTED)||(undecided_accepted&&
											(event->status==UNDECIDED)))
										{
											u=(float)(times[datum]-times[event->time])*(float)1000./
												frequency;
											if (found)
											{
												if (((v<0)&&(v<u))||((v>=0)&&(0<=u)&&(u<v)))
												{
													v=u;
												}
											}
											else
											{
												v=u;
												found=1;
											}
										}
										event=event->next;
									}
									if (found)
									{
										/* perform projection */
										position= &(description->properties.electrode.position);
										switch (region->type)
										{
											case SOCK:
											{
												linear_transformation(linear_trans,position->x,
													position->y,position->z,&x,&y,&z);
												cartesian_to_prolate_spheroidal(x,y,z,focus,&lambda,
													y_data,x_data,(float *)NULL);
											} break;
											case PATCH:
											{
												*x_data=position->x;
												*y_data=position->y;
											} break;
											case TORSO:
											{
												cartesian_to_cylindrical_polar(position->x,position->y,
													position->z,&r,x_data,y_data,(float *)NULL);
											} break;
										}
										/* calculate activation time */
										*value=v;
										x_data++;
										y_data++;
										value++;
										/* assign weight */
										*weight=(float)1;
										weight++;
										i--;
									}
								}
								device++;
							}
						} break;
						case POTENTIAL:
						{
							while (i>0)
							{
								if (((description=(*device)->description)->type==ELECTRODE)&&
									(description->region==region)&&(signal=(*device)->signal)&&
									((signal->status==ACCEPTED)||
									(undecided_accepted&&(signal->status==UNDECIDED)))&&
									(buffer=signal->buffer)&&(times=buffer->times)&&
									((float)(times[0])*1000/(buffer->frequency)<=potential_time)&&
									(potential_time<=
									(float)(times[(buffer->number_of_samples)-1])*1000/
									(buffer->frequency)))
								{
									/* perform projection */
									position= &(description->properties.electrode.position);
									switch (region->type)
									{
										case SOCK:
										{
											linear_transformation(linear_trans,position->x,
												position->y,position->z,&x,&y,&z);
											cartesian_to_prolate_spheroidal(x,y,z,focus,&lambda,
												y_data,x_data,(float *)NULL);
										} break;
										case PATCH:
										{
											*x_data=position->x;
											*y_data=position->y;
										} break;
										case TORSO:
										{
											cartesian_to_cylindrical_polar(position->x,position->y,
												position->z,&r,x_data,y_data,(float *)NULL);
										} break;
									}
									/* calculate potential */
									before=0;
									after=(buffer->number_of_samples)-1;
									potential_time_freq=potential_time*(buffer->frequency)/1000;
									while (before+1<after)
									{
										middle=(before+after)/2;
										if (potential_time_freq<times[middle])
										{
											after=middle;
										}
										else
										{
											before=middle;
										}
									}
									if (before==after)
									{
										proportion=0.5;
									}
									else
									{
										proportion=((float)(times[after])-potential_time_freq)/
											(float)(times[after]-times[before]);
									}
									switch (signal->buffer->value_type)
									{
										case SHORT_INT_VALUE:
										{
											*value=(proportion*
												(float)((signal->buffer->signals.short_int_values)[
												before*
												(signal->buffer->number_of_signals)+(signal->index)])+
												(1-proportion)*
												(float)((signal->buffer->signals.short_int_values)[
												after*
												(signal->buffer->number_of_signals)+(signal->index)])-
												((*device)->channel->offset))*
												((*device)->channel->gain);
										} break;
										case FLOAT_VALUE:
										{
											*value=(proportion*
												(signal->buffer->signals.float_values)[before*
												(signal->buffer->number_of_signals)+(signal->index)]+
												(1-proportion)*
												(signal->buffer->signals.float_values)[after*
												(signal->buffer->number_of_signals)+(signal->index)]-
												((*device)->channel->offset))*
												((*device)->channel->gain);
										} break;
									}
									x_data++;
									y_data++;
									value++;
									/* assign weight */
									*weight=(float)1;
									weight++;
									i--;
								}
								device++;
							}
						} break;
						case INTEGRAL:
						{
							while (i>0)
							{
								if (((description=(*device)->description)->type==ELECTRODE)&&
									(description->region==region)&&(signal=(*device)->signal)&&
									(0<=start_search_interval)&&
									(end_search_interval<signal->buffer->number_of_samples)&&
									((signal->status==ACCEPTED)||
									(undecided_accepted&&(signal->status==UNDECIDED))))
								{
									/* perform projection */
									position= &(description->properties.electrode.position);
									switch (region->type)
									{
										case SOCK:
										{
											linear_transformation(linear_trans,position->x,
												position->y,position->z,&x,&y,&z);
											cartesian_to_prolate_spheroidal(x,y,z,focus,&lambda,
												y_data,x_data,(float *)NULL);
										} break;
										case PATCH:
										{
											*x_data=position->x;
											*y_data=position->y;
										} break;
										case TORSO:
										{
											cartesian_to_cylindrical_polar(position->x,position->y,
												position->z,&r,x_data,y_data,(float *)NULL);
										} break;
									}
									/* calculate integral */
									integral= -(double)((*device)->channel->offset)*
										(double)(end_search_interval-start_search_interval+1);
									number_of_signals=signal->buffer->number_of_signals;
									switch (signal->buffer->value_type)
									{
										case SHORT_INT_VALUE:
										{
											short_int_value=
												(signal->buffer->signals.short_int_values)+
												(start_search_interval*number_of_signals+
												(signal->index));
											for (j=end_search_interval-start_search_interval;j>=0;j--)
											{
												integral += (double)(*short_int_value);
												short_int_value += number_of_signals;
											}
										} break;
										case FLOAT_VALUE:
										{
											float_value=(signal->buffer->signals.float_values)+
												(start_search_interval*number_of_signals+
												(signal->index));
											for (j=end_search_interval-start_search_interval;j>=0;j--)
											{
												integral += (double)(*float_value);
												float_value += number_of_signals;
											}
										} break;
									}
									integral *= (double)((*device)->channel->gain)/
										(double)(signal->buffer->frequency);
									*value=(float)integral;
									x_data++;
									y_data++;
									value++;
									/* assign weight */
									*weight=1;
									weight++;
									i--;
								}
								device++;
							}
						} break;
					}
					/* calculate spatial range of data */
					x_data=x_data_base;
					y_data=y_data_base;
					min_data_x= *x_data;
					max_data_x= min_data_x;
					min_data_y= *y_data;
					max_data_y= min_data_y;
					for (i=number_of_data-1;i>0;i--)
					{
						x_data++;
						y_data++;
						if (*x_data<min_data_x)
						{
							min_data_x= *x_data;
						}
						else
						{
							if (*x_data>max_data_x)
							{
								max_data_x= *x_data;
							}
						}
						if (*y_data<min_data_y)
						{
							min_data_y= *y_data;
						}
						else
						{
							if (*y_data>max_data_y)
							{
								max_data_y= *y_data;
							}
						}
					}
					/*??? adjust range for sock ? */
					switch (region->type)
					{
						case SOCK:
						{
							min_data_x=(float)0;
							/* set max_data_x to 2*pi */
							max_data_x=(float)(8*atan(1));
							min_data_y=(float)0;
						} break;
						case TORSO:
						{
							max_data_x=(float)(4*atan(1));
							min_data_x= -max_data_x;
						} break;
					}
					if (ALLOCATE(function,struct Interpolation_function,1))
					{
						function->region_type=region->type;
						function->x_mesh=(float *)NULL;
						function->y_mesh=(float *)NULL;
						function->f=(float *)NULL;
						function->dfdx=(float *)NULL;
						function->dfdy=(float *)NULL;
						function->d2fdxdy=(float *)NULL;
						/* create mesh description */
						number_of_rows=function->number_of_rows=finite_element_mesh_rows;
						number_of_columns=function->number_of_columns=
							finite_element_mesh_columns;
						if ((ALLOCATE(y_mesh,float,number_of_rows+1))&&
							(ALLOCATE(x_mesh,float,number_of_columns+1)))
						{
							function->x_mesh=x_mesh;
							function->y_mesh=y_mesh;
							step_x=(max_data_x-min_data_x)/(float)(number_of_columns);
							step_y=(max_data_y-min_data_y)/(float)(number_of_rows);
							y_mesh[0]=min_data_y;
							for (i=1;i<number_of_rows;i++)
							{
								y_mesh[i]=y_mesh[i-1]+step_y;
							}
							y_mesh[i]=max_data_y;
							x_mesh[0]=min_data_x;
							for (i=1;i<number_of_columns;i++)
							{
								x_mesh[i]=x_mesh[i-1]+step_x;
							}
							x_mesh[i]=max_data_x;
							number_of_nodes=(number_of_rows+1)*(number_of_columns+1);
							switch (region->type)
							{
								case PATCH:
								{
									number_of_equations=4*number_of_nodes;
								} break;
								case SOCK:
								{
#if defined (FIX_APEX)
									number_of_equations=4*number_of_rows*number_of_columns+3;
#else
									number_of_equations=4*(number_of_rows*number_of_columns+1);
#endif /* defined (FIX_APEX) */
								} break;
								case TORSO:
								{
									number_of_equations=4*(number_of_rows+1)*number_of_columns;
								} break;
							}
							function->number_of_nodes=number_of_nodes;
							if (
#if defined (DOUBLE_MATRIX)
								(ALLOCATE(matrix_and_right_hand_side,double,
									number_of_equations*(number_of_equations+1)))
#if defined (FIX_APEX)
								&&(ALLOCATE(cosine_apex,double,number_of_columns+1))&&
								(ALLOCATE(sine_apex,double,number_of_columns+1))
#endif /* defined (FIX_APEX) */
#else
								(ALLOCATE(matrix_and_right_hand_side,float,
									number_of_equations*(number_of_equations+1)))
#if defined (FIX_APEX)
								&&(ALLOCATE(cosine_apex,float,number_of_columns+1))&&
								(ALLOCATE(sine_apex,float,number_of_columns+1))
#endif /* defined (FIX_APEX) */
#endif /* defined (DOUBLE_MATRIX) */
								)
							{
								if (assemble_linear_equations(region->type,number_of_data,
									x_data_base,y_data_base,value_base,weight_base,number_of_rows,
									y_mesh,number_of_columns,x_mesh,number_of_equations,
#if defined (FIX_APEX)
									cosine_apex,sine_apex,
#endif /* defined (FIX_APEX) */
									matrix_and_right_hand_side,membrane_smoothing,
									plate_bending_smoothing))
								{
									if (solve_linear_equations(number_of_equations,
										matrix_and_right_hand_side,region->type,number_of_nodes,
										number_of_rows,number_of_columns,
#if defined (FIX_APEX)
										cosine_apex,sine_apex,
#endif /* defined (FIX_APEX) */
										&(function->f),&(function->dfdx),&(function->dfdy),
										&(function->d2fdxdy)))
									{
										/* calculate the range of values (used for drawing colour map)
											 and calculate error */
										/* loop over nodes */
										f=function->f;
										f_min=f_max= *f;
										for (i=1;i<number_of_nodes;i++)
										{
											f++;
											if (*f>f_max)
											{
												f_max= *f;
											}
											else
											{
												if (*f<f_min)
												{
													f_min= *f;
												}
											}
										}
										/* loop over the data points */
										f=function->f;
										dfdx=function->dfdx;
										dfdy=function->dfdy;
										d2fdxdy=function->d2fdxdy;
										number_of_valid_data=0;
										average_absolute_error=(float)0;
										maximum_absolute_error=(float)0;
										for (i=0;i<number_of_data;i++)
										{
											/* determine which element the data point is in */
											u=x_data_base[i];
											column=0;
											while ((column<number_of_columns)&&(u>=x_mesh[column]))
											{
												column++;
											}
											if ((column>0)&&(u<=x_mesh[column]))
											{
												v=y_data_base[i];
												row=0;
												while ((row<number_of_rows)&&(v>=y_mesh[row]))
												{
													row++;
												}
												if ((row>0)&&(v<=y_mesh[row]))
												{
													number_of_valid_data++;
													/* calculate basis function values */
													width=x_mesh[column]-x_mesh[column-1];
													h11_u=h12_u=u-x_mesh[column-1];
													u=h11_u/width;
													h01_u=(2*u-3)*u*u+1;
													h11_u *= (u-1)*(u-1);
													h02_u=u*u*(3-2*u);
													h12_u *= u*(u-1);
													height=y_mesh[row]-y_mesh[row-1];
													h11_v=h12_v=v-y_mesh[row-1];
													v=h11_v/height;
													h01_v=(2*v-3)*v*v+1;
													h11_v *= (v-1)*(v-1);
													h02_v=v*v*(3-2*v);
													h12_v *= v*(v-1);
													/* calculate the interpolation function coefficients */
													f_im1_jm1=h01_u*h01_v;
													f_im1_j=h02_u*h01_v;
													f_i_j=h02_u*h02_v;
													f_i_jm1=h01_u*h02_v;
													dfdx_im1_jm1=h11_u*h01_v;
													dfdx_im1_j=h12_u*h01_v;
													dfdx_i_j=h12_u*h02_v;
													dfdx_i_jm1=h11_u*h02_v;
													dfdy_im1_jm1=h01_u*h11_v;
													dfdy_im1_j=h02_u*h11_v;
													dfdy_i_j=h02_u*h12_v;
													dfdy_i_jm1=h01_u*h12_v;
													d2fdxdy_im1_jm1=h11_u*h11_v;
													d2fdxdy_im1_j=h12_u*h11_v;
													d2fdxdy_i_j=h12_u*h12_v;
													d2fdxdy_i_jm1=h11_u*h12_v;
													/* calculate node numbers
														 NB the local coordinates have the top right corner of
														 the element as the origin.  This means that
														 (i-1,j-1) is the bottom left corner
														 (i,j-1) is the top left corner
														 (i,j) is the top right corner
														 (i-1,j) is the bottom right corner */
													/* (i-1,j-1) node (bottom left corner) */
													im1_jm1=(row-1)*(number_of_columns+1)+column-1;
													/* (i,j-1) node (top left corner) */
													i_jm1=row*(number_of_columns+1)+column-1;
													/* (i,j) node (top right corner) */
													i_j=row*(number_of_columns+1)+column;
													/* (i-1,j) node (bottom right corner) */
													im1_j=(row-1)*(number_of_columns+1)+column;
													f_approx=
														f[im1_jm1]*f_im1_jm1+
														f[i_jm1]*f_i_jm1+
														f[i_j]*f_i_j+
														f[im1_j]*f_im1_j+
														dfdx[im1_jm1]*dfdx_im1_jm1+
														dfdx[i_jm1]*dfdx_i_jm1+
														dfdx[i_j]*dfdx_i_j+
														dfdx[im1_j]*dfdx_im1_j+
														dfdy[im1_jm1]*dfdy_im1_jm1+
														dfdy[i_jm1]*dfdy_i_jm1+
														dfdy[i_j]*dfdy_i_j+
														dfdy[im1_j]*dfdy_im1_j+
														d2fdxdy[im1_jm1]*d2fdxdy_im1_jm1+
														d2fdxdy[i_jm1]*d2fdxdy_i_jm1+
														d2fdxdy[i_j]*d2fdxdy_i_j+
														d2fdxdy[im1_j]*d2fdxdy_im1_j;
													if (f_approx>f_max)
													{
														f_max=f_approx;
													}
													else
													{
														if (f_approx<f_min)
														{
															f_min=f_approx;
														}
													}
													absolute_error=f_approx-value_base[i];
													if (absolute_error<0)
													{
														absolute_error= -absolute_error;
													}
													average_absolute_error += absolute_error;
													if (absolute_error>maximum_absolute_error)
													{
														maximum_absolute_error=absolute_error;
													}
												}
											}
										}
										if (number_of_valid_data>0)
										{
											average_absolute_error /= number_of_valid_data;
											if (POTENTIAL==map_type)
											{
												display_message(INFORMATION_MESSAGE,"frame time = %g, ",
													potential_time);										
											}
											display_message(INFORMATION_MESSAGE,
												"average absolute error = %.4g, maximum absolute error = %.4g\n",
												average_absolute_error,maximum_absolute_error);
										}
										function->f_min=f_min;
										function->f_max=f_max;
									}
									else
									{
										destroy_Interpolation_function(&function);
									}
								}
								else
								{
									destroy_Interpolation_function(&function);
								}
								/* free working storage */
#if defined (FIX_APEX)
								DEALLOCATE(cosine_apex);
								DEALLOCATE(sine_apex);
#endif /* defined (FIX_APEX) */
								DEALLOCATE(matrix_and_right_hand_side);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"calculate_interpolation_functio.  Could not assign %s",
									"memory for linear equations");
								destroy_Interpolation_function(&function);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"calculate_interpolation_functio.  %s",
								"Could not allocate memory for mesh description");
							DEALLOCATE(x_mesh);
							destroy_Interpolation_function(&function);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"calculate_interpolation_functio.  %s",
							"Could not allocate memory for function");
					}
					/* free data arrays */
					DEALLOCATE(x_data_base);
					DEALLOCATE(y_data_base);
					DEALLOCATE(value_base);
					DEALLOCATE(weight_base);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"calculate_interpolation_functio.  Could not allocate memory for data");
					DEALLOCATE(y_data_base);
					DEALLOCATE(value_base);
					DEALLOCATE(weight_base);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"calculate_interpolation_functio.  No data to map for %s",
					region->name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"calculate_interpolation_functio.  Invalid arguments");
		}
		LEAVE;

		return (function);
} /* calculate_interpolation_functio */

int destroy_Interpolation_function(struct Interpolation_function **function)
/*******************************************************************************
LAST MODIFIED : 27 December 1996

DESCRIPTION :
This function deallocates the memory associated with the fields of <**function>,
deallocates the memory for <**function> and sets <*function> to NULL;
==============================================================================*/
{
	int return_code;

	ENTER(destroy_Interpolation_function);
	return_code=1;
	if (function&&(*function))
	{
		DEALLOCATE((*function)->x_mesh);
		DEALLOCATE((*function)->y_mesh);
		DEALLOCATE((*function)->f);
		DEALLOCATE((*function)->dfdx);
		DEALLOCATE((*function)->dfdy);
		DEALLOCATE((*function)->d2fdxdy);
		DEALLOCATE(*function);
	}
	LEAVE;

	return (return_code);
} /* destroy_Interpolation_function */
