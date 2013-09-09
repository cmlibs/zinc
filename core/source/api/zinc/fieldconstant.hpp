/***************************************************************************//**
 * FILE : fieldconstant.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDCONSTANT_HPP__
#define CMZN_FIELDCONSTANT_HPP__

#include "zinc/fieldconstant.h"
#include "zinc/field.hpp"
#include "zinc/fieldmodule.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class FieldConstant : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldConstant(cmzn_field_id field_id) : Field(field_id)
	{ }

	friend FieldConstant FieldModule::createConstant(int valuesCount, const double *valuesIn);

public:

	FieldConstant() : Field(0)
	{ }

};

class FieldStringConstant : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldStringConstant(cmzn_field_id field_id) : Field(field_id)
	{ }

	friend FieldStringConstant FieldModule::createStringConstant(const char *stringConstant);

public:

	FieldStringConstant() : Field(0)
	{ }

};

inline FieldConstant FieldModule::createConstant(int valuesCount, const double *valuesIn)
{
	return FieldConstant(cmzn_field_module_create_constant(id,
		valuesCount, valuesIn));
}

inline FieldStringConstant FieldModule::createStringConstant(const char *stringConstant)
{
	return FieldStringConstant(cmzn_field_module_create_string_constant(id,
		stringConstant));
}

}  // namespace Zinc
}

#endif
