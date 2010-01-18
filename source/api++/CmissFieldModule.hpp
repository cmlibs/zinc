/***************************************************************************//**
 * FILE : CmissFieldModule.hpp
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
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2010
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
#ifndef __CMISS_FIELD_MODULE_HPP__
#define __CMISS_FIELD_MODULE_HPP__

extern "C" {
#include "api/cmiss_field_module.h"
}
#include "api++/CmissField.hpp"
#include "api++/CmissFieldTypesArithmeticOperators.hpp"
#include "api++/CmissFieldTypesComposite.hpp"
#include "api++/CmissFieldTypesImage.hpp"
#include "api++/CmissFieldTypesTrigonometry.hpp"

namespace Cmiss
{

class FieldAdd;
class FieldConstant;
class FieldImage;
class FieldSin;

class FieldModule
{
private:
	Cmiss_field_module_id id;

public:
	FieldModule() : id(NULL)
	{	}

	// takes ownership of C-style field module reference
	FieldModule(Cmiss_field_module_id field_module_id) :
		id(field_module_id)
	{ }

	FieldModule(const FieldModule& fieldModule) :
		id(Cmiss_field_module_access(fieldModule.id))
	{ }

	FieldModule& operator=(const FieldModule& fieldModule)
	{
		Cmiss_field_module_id temp_id = Cmiss_field_module_access(fieldModule.id);
		if (NULL != id)
		{
			Cmiss_field_module_destroy(&id);
		}
		id = temp_id;
		return *this;
	}
	
	~FieldModule()
	{
		if (NULL != id)
		{
			Cmiss_field_module_destroy(&id);
		}
	}

	Field findFieldByName(const char *field_name)
	{
		return Field(Cmiss_field_module_find_field_by_name(id, field_name));
	}

	// factory methods for creating new fields
	FieldAdd createAdd(const Field& operand1, const Field& operand2)
	{
		return FieldAdd(reinterpret_cast<Cmiss_field_add_id>(
			Cmiss_field_module_create_add(id, operand1.id, operand2.id)));
	}

	FieldConstant createConstant(int numValues, const double *values)
	{
		return FieldConstant(reinterpret_cast<Cmiss_field_constant_id>(
			Cmiss_field_module_create_constant(id, numValues, values)));
	}

	FieldImage createImage(const Field& domain)
	{
		return FieldImage(reinterpret_cast<Cmiss_field_image_id>(
			Cmiss_field_module_create_image(id, domain.id, /*source*/NULL)));
	}

	FieldImage createImageFromSource(const Field& source)
	{
		return FieldImage(reinterpret_cast<Cmiss_field_image_id>(
			Cmiss_field_module_create_image(id, /*domain*/NULL, source.id)));
	}

	FieldSin createSin(const Field& operand)
	{
		return FieldSin(reinterpret_cast<Cmiss_field_sin_id>(
			Cmiss_field_module_create_sin(id, operand.id)));
	}

};

} // namespace Cmiss

#endif /* __CMISS_FIELD_MODULE_HPP__ */
