/*******************************************************************************
FILE : quaternion.hpp

LAST MODIFIED : 17 October 2007

DESCRIPTION : A class of quaternion operations, any new quaternion
operations should be added into this class.
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

#include "quaternion.hpp"
extern "C" {
#include <math.h>
}

void Quaternion::set(const double quat_w, const double quat_x, const double quat_y, const double quat_z)
{
	 ENTER(Quaternion::set);
	 w = quat_w;
	 x = quat_x;
	 y = quat_y;
	 z = quat_z;
	 LEAVE;
}

 void Quaternion::get(double *values) const
/*******************************************************************************
LAST MODIFIED : 18 Oct 2007

DESCRIPTION :
get the values stored in this quaternion and put it into the argument
values, must allocate values before calling this function.
==============================================================================*/
 {
		ENTER(Quaternion::get);
		values[0] = w;
		values[1] = x;
		values[2] = y;
		values[3] = z;
		LEAVE;
 }
 
 void Quaternion::normalise()
 {
		ENTER(Quaternion::normalise);
		const double tolerance = 0.00000001;
		const double magnitude = sqrt(w*w+ x*x + y*y + z*z);
		if (fabs(magnitude - 1.0) > tolerance)
		{
			 w = w / magnitude;
			 x = x / magnitude;
			 y = y / magnitude;
			 z = z / magnitude;
		}
		LEAVE;
 }

 void Quaternion::interpolated_with_SLERP(const Quaternion &from, const Quaternion &to, const double normalised_time)
/*******************************************************************************
LAST MODIFIED : 18 Oct 2007

DESCRIPTION :
Spherical linear interpolation of two quaternions at time index
(normalised time between these two quaternion).
==============================================================================*/
 {
		ENTER(Quaternion::interpolated_with_SLERP);
		const double tolerance = 0.00000001;
		double cosOmega = from.x * to.x + from.y * to.y + from.z * to.z + from.w * to.w;
		float tol[4];
		if (cosOmega < 0.0)
		{
			 cosOmega = -cosOmega;
			 tol[0] = - to.x;
			 tol[1] = - to.y;
			 tol[2] = - to.z;
			 tol[3] = - to.w;
		}
		else
		{
			 tol[0] = to.x;
			 tol[1] = to.y;
			 tol[2] = to.z;
			 tol[3] = to.w;
		}

		double scale0, scale1;
		if (fabs(1.0-cosOmega)>tolerance)
		{
			 const double omega = acos(cosOmega);
			 const double sinOmega = sin(omega);
			 scale0 = sin((1.0-normalised_time) * omega) / sinOmega;
			 scale1 = sin(normalised_time * omega) / sinOmega;
		}
		else
		{
			 scale0 = 1.0 - normalised_time;
			 scale1 = normalised_time;
		}
		x = scale0 * from.x + scale1 * tol[0];
		y = scale0 * from.y + scale1 * tol[1]; 
		z = scale0 * from.z + scale1 * tol[2];
		w = scale0 * from.w +scale1 * tol[3];
		normalise();

		LEAVE;
 }

 int Quaternion::quaternion_to_matrix(float *values)
/*******************************************************************************
LAST MODIFIED : 18 Oct 2007

DESCRIPTION :
Convert the quaternion to a matrix, the argument values must be allocated before this function.
==============================================================================*/
 {
		int return_code;

		ENTER(Quaternion::quaternion_to_matrix);
		return_code = 0;
		if (values)
		{
			 normalise();
			 const double x2 = x + x;
			 const double y2 = y + y;
			 const double z2 = z + z;
			 const double xx = x * x2;
			 const double xy = x * y2;
			 const double xz = x * z2;
			 const double yy = y * y2;
			 const double yz = y * z2;
			 const double zz = z * z2;
			 const double wx = w * x2;
			 const double wy = w * y2;
			 const double wz = w * z2;
			 values[0] = 1 - yy - zz;
			 values[1] = xy - wz;
			 values[2] = xz+wy;
			 values[3] = 0;
			 values[4] = xy + wz;
			 values[5] = 1 - xx - zz;
			 values[6] = yz-  wx;
			 values[7] = 0;
			 values[8] = xz - wy;
			 values[9] = yz + wx;
			 values[10] = 1 - xx - yy;
			 values[11] = 0;
			 values[12] = 0;
			 values[13] = 0;
			 values[14] = 0;
			 values[15] = 1;
			 return_code = 1;
		}
		LEAVE;

		return (return_code);
 }

int Quaternion::matrix_to_quaternion(float *source, float *destination)
/*******************************************************************************
LAST MODIFIED : 22 Feb 2008

DESCRIPTION :
Convert the matrix to a quaternion, the argument values must be allocated before this function.
==============================================================================*/
 {
		int return_code;
		float trace, s;

		ENTER(Quaternion::matrix_to_quaternion);
		return_code = 0;
		trace = source[0] + source[5] + source[10] + 1;
		if (trace > 0)
		{
			 s = 0.5f /sqrtf(trace);
			 w = 0.25f / s;
			 x = (source[9] - source[6]) *s;
 			 y = (source[2] - source[8]) *s;
 			 z = (source[4] - source[1]) *s;
			 return_code = 1;
		}
		else
		{
			 if ((source[0]>source[5]) && (source[0]>source[10]))
			 {
					s = 2.0f * sqrtf(1.0f+source[0]-source[5]-source[10]);
					w = (source[6] - source[9])/s;
					x = 0.25f * s;
					y = (source[1] + source[4])/s;
					z = (source[2] + source[8])/s;
			 }
			 else if (source[5]>source[10])
			 {
					s = 2.0f * sqrtf(1.0f+source[5]-source[0]-source[10]);
					w = (source[2] - source[8])/s;
					x = (source[1] + source[4])/s;
					y = 0.25f * s;
					z = (source[6] + source[9])/s;
			 }
			 else
			 {
					s = 2.0f * sqrtf(1.0f+source[10]-source[0]-source[5]);
					w = (source[1] - source[4])/s;
					x = (source[2] + source[8])/s;
					y = (source[6] + source[9])/s;
					z = 0.25f * s;
			 }
			 return_code = 1;
		}
 		destination[0] = w;
 		destination[1] = x;
 		destination[2] = y;
 		destination[3] = z;
		LEAVE;

		return (return_code);
 }

