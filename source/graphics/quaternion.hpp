/*******************************************************************************
FILE : quaternion.h

LAST MODIFIED : 17 October 2007

DESCRIPTION :
==============================================================================*/
extern "C" {
#include <math.h>
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
}

class Quaternion
{

	 double w, x, y, z;
public:

	 Quaternion(double quat_w, double quat_x, double quat_y, double quat_z)
	 {
			set(quat_w, quat_x, quat_y, quat_z);
	 };

	 ~Quaternion();

	 void set(double quat_w, double quat_x, double quat_y, double quat_z);

	 void get(double *values);

	 void normalise();

	 void interpolated_with_SLERP(Quaternion &from, Quaternion &to, double normalised_time);

	 int quaternion_to_matrix(float *values);

};
	 
void Quaternion::set(double quat_w, double quat_x, double quat_y, double quat_z)
{
	 ENTER(Quaternion::set);
	 w = quat_w;
	 x = quat_x;
	 y = quat_y;
	 z = quat_z;
	 LEAVE;
}

 void Quaternion::get(double *values)
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
		double tolerance, magnitude;

		ENTER(Quaternion::evaluate_cache_at_location);
		tolerance = 0.00000001;
		magnitude = sqrt(w*w+ x*x + y*y + z*z);
		if (fabs(magnitude - 1.0) > tolerance)
		{
			 w = w / magnitude;
			 x = x / magnitude;
			 y = y / magnitude;
			 z = z / magnitude;
		}
		LEAVE;
 }

 void Quaternion::interpolated_with_SLERP(Quaternion &from, Quaternion &to, double normalised_time)
 {
		float tol[4];
		double omega, cosOmega, sinOmega, scale0, scale1, tolerance;

		ENTER(Quaternion::interpolated_with_SLERP);
		tolerance = 0.00000001;
		cosOmega = from.x * to.x + from.y * to.y + from.z * to.z + from.w * to.w;
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
		if (fabs(1.0-cosOmega)>tolerance)
		{
			 omega = acos(cosOmega);
			 sinOmega = sin(omega);
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
 {
		int return_code;
		double wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

		ENTER(Quaternion::quaternion_to_matrix);
		return_code = 0;
		if (values)
		{
			 normalise();
			 x2 = x + x;
			 y2 = y + y;
			 z2 = z + z;
			 xx = x * x2;
			 xy = x * y2;
			 xz = x * z2;
			 yy = y * y2;
			 yz = y * z2;
			 zz = z * z2;
			 wx = w * x2;
			 wy = w * y2;
			 wz = w * z2;
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

		
