/*******************************************************************************
FILE : morphology_functions.h

LAST MODIFIED : 24 June 2004

DESCRIPTION :
Implements morphological parameters extraction.
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
#if !defined (MORPHOLOGY_FUNCTIONS_H)
#define MORPHOLOGUY_FUNCTIONS_H

#include "computed_field/computed_field.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"

int Spatial_gray_tone_histogram(FE_value *data_index, int radius, int xctr, int yctr, int zctr,
         int xsize, int ysize, int zsize, int levels, int *h);
/*****************************************************************************************
   LAST MODIFIED: 23 April 2004
   DESCRIPTION: Compute the gray tone histogram at (xctr,yctr,zctr).
=========================================================================================*/
int Spatial_gray_tone_histogram_cube(FE_value *data_index,
         int xsize, int ysize, int zsize, int levels, int *h);
/*****************************************************************************************
   LAST MODIFIED: 23 April 2004
   DESCRIPTION: Compute the gray tone histogram at (xctr,yctr,zctr).
=========================================================================================*/
int Spatial_Euler_number(FE_value pixsize, int *h, FE_value *euler_number);
/*****************************************************************************************
   LAST MODIFIED: 6 May 2004

   DESCRIPTION: Compute the Euler number density.
=========================================================================================*/
int Spatial_mean_curvature(FE_value pixsize, int *h, FE_value *mean_curvature);
/*****************************************************************************************
   LAST MODIFIED: 6 May 2004
   DESCRIPTION: Compute the integral of mean curvature.
=========================================================================================*/
int Spatial_gaussian_curvature(FE_value pixsize, int *h, FE_value *total_curvature);
/*****************************************************************************************
   LAST MODIFIED: 23 April 2004
   DESCRIPTION: Compute the integral of total curvature.
=========================================================================================*/
int Spatial_total_volume(int *h, FE_value *total_volume);
/*****************************************************************************************
   LAST MODIFIED: 23 April 2004
   DESCRIPTION: Compute the tatol volume.
=========================================================================================*/
int Spatial_volume_fraction(int *h, FE_value *volume_fraction);
/*****************************************************************************************
   LAST MODIFIED: 23 April 2004
   DESCRIPTION: Compute the volume fraction.
=========================================================================================*/
int Spatial_surface_density(FE_value pixsize, int *h, FE_value *surface_density);
/*****************************************************************************************
   LAST MODIFIED: 23 April 2004
   DESCRIPTION: Compute the bone volume and tatol volume at the position (xctr,yctr,zctr).
=========================================================================================*/
int Length_of_test_lines(FE_value dradius,
        int xctr, int yctr, int iline_sp,
	int lmarg, int rmarg, int tlay, int blay,
	FE_value *tline_ln);
/*****************************************************************************************
   LAST MODIFIED: 23 February 2004
   DESCRIPTION: Compute the length of all test lines at
                the position (xctr,yctr,zctr).
=========================================================================================*/
int Solve_linear_system (int dim, int order, FE_value *a, FE_value *b, int *pivot);
/************************************************************************
    LAST MODIFIED: 11 February 2004
    DESCRIPTION: solve system of simultaneous eqns.
===========================================================================*/
int LU_decomp_of_matrix (int dim, int order, FE_value *a, int *pivot, FE_value *condnum);
/**************************************************************************************
    LAST MODIFIED: 11 February 2004

    DESCRIPTION: decompose matrix a into LU matrix
======================================================================================*/
int Inverse_of_matrix (int dim, int order, FE_value *a, FE_value *in);
/*****************************************************************
    LAST MODIFIED: 11 February 2004

    DESCRIPTION: find the inverse of matrix a
=====================================================================*/
int Mil_ellipsoid_tensor(FE_value *MIL, FE_value *rott1, FE_value *rott2,
            int number_of_dirs, FE_value *elvector);
/************************************************************************************
    LAST MODIFIED: 11 February 2004

    DESCRIPTION: fit a ellipsoid, return the ellipsoid coeff.
=====================================================================================*/
int Eigvalue_eigvector_Jacobi (int dim, FE_value *eigvals, FE_value *eigvecs,
           int order, FE_value eps, int *numiters, int maxiters);
/*****************************************************************************
    LAST MODIFIED: 16 February 2004

    DESCRIPTION : Computes the eigenvalues and eigenvectors of
                    a real symmetric matrix via the Jacobi method
============================================================================== */
int Principal_orientations(FE_value *elvector,
	 FE_value *prin1, FE_value *prin2, FE_value *prin3,
	 FE_value *deg1, FE_value *deg2, FE_value *deg3,
	 FE_value *eigvals, FE_value *eigvecs);
/***********************************************************************
    LAST MODIFIED: 27 April 2004

    DESCRIPTION: Find the eigenvalues and eigenvectors of MIL tensor matrix
==========================================================================*/
int Sort_eigenvalue( FE_value *eigvals, FE_value *eigvecs,
	 FE_value *prin1, FE_value *prin2, FE_value *prin3,
	 FE_value *deg1, FE_value *deg2, FE_value *deg3);
/**************************************************************************

      LAST MODIEFIED: 28 April 2004

      DESCRIPTION: Sort the eigenvalues and the corresponding eigenvectore
                    from the largest to the smallest
=========================================================================== */
int Volume_fraction_and_length_of_test_lines(FE_value *data_index, FE_value dradius,
        int xctr, int yctr, int zctr, int iline_sp,
	int xsize, int ysize,
	int lmarg, int rmarg, int tlay, int blay, int top, int bottom,
	FE_value *bv, FE_value *tv, FE_value *tline_ln);
/*****************************************************************************************
   LAST MODIFIED: 23 February 2004
   DESCRIPTION: Compute the bone volume,tatol volume and the length of all test lines at
                the position (xctr,yctr,zctr).
=========================================================================================*/
int MIL_vector(FE_value *data_index, int number_of_dirs, FE_value dradius,
       FE_value *rot1, FE_value *rot2, FE_value tline_ln, int iline_sp, FE_value bvtv,
       int xsize, int ysize, int xctr, int yctr, int zctr,
       int lmarg, int rmarg, int tlay, int blay, int top, int bottom,
       FE_value *mil, int *tintrsctn, int *num_tlines);
/*****************************************************************************************
   LAST MODIFIED: 29 March 2004
   DESCRIPTION: Compute the MIL vector, total number of intersections and total number of lines.
=========================================================================================*/

#endif /* !defined (MORPHOLOGY_FUNCTIONS_H) */
