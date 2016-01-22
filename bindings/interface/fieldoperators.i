/**
 * fieldoperators.i
 * 
 * Swig interface file for wrapping overloading operators of fields
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
 
%extend OpenCMISS::Zinc::Field
{
	OpenCMISS::Zinc::FieldAdd operator+(OpenCMISS::Zinc::Field& operand)
	{
		return ($self)->getFieldmodule().createFieldAdd(*($self), operand);
	}
	
	OpenCMISS::Zinc::FieldSubtract operator-(OpenCMISS::Zinc::Field& operand)
	{
		return ($self)->getFieldmodule().createFieldSubtract(*($self), operand);
	}
	
	OpenCMISS::Zinc::FieldMultiply operator*(OpenCMISS::Zinc::Field& operand)
	{
		return ($self)->getFieldmodule().createFieldMultiply(*($self), operand);
	}
	
	OpenCMISS::Zinc::FieldDivide operator/(OpenCMISS::Zinc::Field& operand)
	{
		return ($self)->getFieldmodule().createFieldDivide(*($self), operand);
	}

	OpenCMISS::Zinc::FieldGreaterThan operator>(OpenCMISS::Zinc::Field& operand)
	{
		return ($self)->getFieldmodule().createFieldGreaterThan(*($self), operand);
	}

	OpenCMISS::Zinc::FieldLessThan operator<(OpenCMISS::Zinc::Field& operand)
	{
		return ($self)->getFieldmodule().createFieldLessThan(*($self), operand);
	}

	bool operator==(const OpenCMISS::Zinc::Field& other) const
	{
		return *($self) == other;
	}
};
