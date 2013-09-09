/*****************************************************************************//**
 * FILE : fieldalias.hpp
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDALIAS_HPP__
#define CMZN_FIELDALIAS_HPP__

#include "zinc/fieldalias.h"
#include "zinc/field.hpp"
#include "zinc/fieldmodule.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class FieldAlias : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldAlias(cmzn_field_id field_id) : Field(field_id)
	{ }

	friend FieldAlias FieldModule::createAlias(Field &sourceField);

public:

	FieldAlias() : Field(0)
	{ }

};

inline FieldAlias FieldModule::createAlias(Field &sourceField)
{
	return FieldAlias(cmzn_field_module_create_alias(id, sourceField.getId()));
}

}  // namespace Zinc
}
#endif
