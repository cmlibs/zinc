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
    	OpenCMISS::Zinc::Fieldmodule fieldModule(*($self));
    	return fieldModule.createFieldAdd(*($self), operand);
	}
	
	OpenCMISS::Zinc::FieldSubtract operator-(OpenCMISS::Zinc::Field& operand)
	{
    	OpenCMISS::Zinc::Fieldmodule fieldModule(*($self));
    	return fieldModule.createFieldSubtract(*($self), operand);
	}
	
	OpenCMISS::Zinc::FieldMultiply operator*(OpenCMISS::Zinc::Field& operand)
	{
	    OpenCMISS::Zinc::Fieldmodule fieldModule(*($self));
 		return fieldModule.createFieldMultiply(*($self), operand);
	}
	
	OpenCMISS::Zinc::FieldDivide operator/(OpenCMISS::Zinc::Field& operand)
	{
	    OpenCMISS::Zinc::Fieldmodule fieldModule(*($self));
 		return fieldModule.createFieldDivide(*($self), operand);
	}

	OpenCMISS::Zinc::FieldGreaterThan operator>(OpenCMISS::Zinc::Field& operand)
	{
    	OpenCMISS::Zinc::Fieldmodule fieldModule(*($self));
    	return fieldModule.createFieldGreaterThan(*($self), operand);
	}

	OpenCMISS::Zinc::FieldLessThan operator<(OpenCMISS::Zinc::Field& operand)
	{
    	OpenCMISS::Zinc::Fieldmodule fieldModule(*($self));
    	return fieldModule.createFieldLessThan(*($self), operand);
	}

};
