/*******************************************************************************
FILE : statistics.c

LAST MODIFIED : 7 May 2001

DESCRIPTION :
Statistical functions, eg. sampling from distributions, used by CMGUI.
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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "general/debug.h"
#include "general/geometry.h"
#include "general/random.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

#define HALF_LOG_2_PI 0.91893853320467267

/* pass integer n to this macro to return Stirling's approximation to log(N!) */
#define STIRLING_APPROXIMATION_LOG_N_FACTORIAL(n) \
((n + 0.5)*log(n) - n + HALF_LOG_2_PI)

int sample_Poisson_distribution(double mean)
/*******************************************************************************
LAST MODIFIED : 7 May 2001

DESCRIPTION :
Returns a single sample from a Poisson distribution with the given <mean>.
Uses Atkinson's method to handle large means, details found at:
http://www.jesus.ox.ac.uk/~clifford/a5/chap1/node13.html
Also verified from book:
"Stochastic Simulation" by Brian. D. Ripley, Wiley 1987 p79.
==============================================================================*/
{
	double a, b, c, k, limit, log_mean, product, u, x, y, z;
	int number;

	ENTER(sample_Poisson_distribution);
	if (0.0 <= mean)
	{
		if (mean < 30.0)
		{
			/* the simple method of sampling from a Poisson distribution is to
				 multiply uniform random variables on (0.0, 1.0) until they are less
				 than the appropirate limit; return the number of random variables
				 calculated less 1. This fails as the value exp(-mean) underflows with
				 fairly moderate mean values, and will also be less accurate as this
				 condition is approached */
			product = CMGUI_RANDOM_NON_INCLUSIVE(double);
			limit = exp(-mean);
			number = 0;
			while (product > limit)
			{
				product *= CMGUI_RANDOM_NON_INCLUSIVE(double);
				number++;
			}
		}
		else
		{
			/* Atkinson's method */
			b = PI / sqrt(3*mean);
			a = b * mean;
			c = .767 - (3.36 / mean);
			k = log(c / b) - mean;
			log_mean = log(mean);
			do
			{
				do
				{
					u = CMGUI_RANDOM_NON_INCLUSIVE(double);
					y = (a - log((1.0 - u) / u)) / b;
				}
				while (y <= -0.5);
				number = (int)(y + 0.5);
				x = (double)number;
				z = k + x*log_mean - STIRLING_APPROXIMATION_LOG_N_FACTORIAL(x);
			}
			while (log(CMGUI_RANDOM_NON_INCLUSIVE(double)*u*(1.0 - u)) > z);
			/* note Ripley algorithm requires > z in the above; clifford uses >= z;
				 I have converted "until (condition)" into "while (!condition)" */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"sample_Poisson_distribution.  Negative mean");
		number = 0;
	}
	LEAVE;

	return (number);
} /* sample_Poisson_distribution */
