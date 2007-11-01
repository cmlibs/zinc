/*******************************************************************************
FILE : quaternion.hpp

LAST MODIFIED : 17 October 2007

DESCRIPTION : A class of quaternion operations, any new quaternion
operations should be added into this class.
==============================================================================*/
#if !defined (QUATERNION_HPP)
#define QUATERNION_HPP

extern "C" {
#include <math.h>
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
}

class Quaternion
{

public:

	 Quaternion(const double quat_w, const double quat_x, const double quat_y, const double quat_z)
	 {
			set(quat_w, quat_x, quat_y, quat_z);
	 };

	 Quaternion()
	 {
			set(1, 0, 0, 0);
	 };

	 ~Quaternion();

	 void set(const double quat_w, const double quat_x, const double quat_y, const double quat_z);

	 void get(double *values) const;

	 void normalise();

	 void interpolated_with_SLERP(const Quaternion &from, const Quaternion &to, const double normalised_time);

	 int quaternion_to_matrix(float *values);

private:
	 
	 double w, x, y, z;

};
#endif /* !defined (QUATERNION_HPP) */

