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

#if !defined (QUATERNION_HPP)
#define QUATERNION_HPP

extern "C" {
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

	 int matrix_to_quaternion(float *source, float *destination);

private:
	 
	 double w, x, y, z;

};
#endif /* !defined (QUATERNION_HPP) */

