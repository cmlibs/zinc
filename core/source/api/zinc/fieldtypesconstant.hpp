/***************************************************************************//**
 * FILE : fieldtypeconstant.hpp
 */
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
 * The Original Code is libZinc.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2012
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
#ifndef __ZN_FIELD_TYPES_CONSTANT_HPP__
#define __ZN_FIELD_TYPES_CONSTANT_HPP__

#include "zinc/fieldconstant.h"
#include "zinc/field.hpp"
#include "zinc/fieldmodule.hpp"

namespace zinc
{

class FieldConstant : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldConstant(Cmiss_field_id field_id) : Field(field_id)
	{ }

	friend FieldConstant FieldModule::createConstant(int valuesCount, const double *values);

public:

	FieldConstant() : Field(0)
	{ }

};

class FieldStringConstant : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldStringConstant(Cmiss_field_id field_id) : Field(field_id)
	{ }

	friend FieldStringConstant FieldModule::createStringConstant(const char *stringConstant);

public:

	FieldStringConstant() : Field(0)
	{ }

};

inline FieldConstant FieldModule::createConstant(int valuesCount, const double *values)
{
	return FieldConstant(Cmiss_field_module_create_constant(id,
		valuesCount, values));
}

inline FieldStringConstant FieldModule::createStringConstant(const char *stringConstant)
{
	return FieldStringConstant(Cmiss_field_module_create_string_constant(id,
		stringConstant));
}

}  // namespace zinc

#endif /* __ZN_FIELD_TYPES_CONSTANT_HPP__ */
