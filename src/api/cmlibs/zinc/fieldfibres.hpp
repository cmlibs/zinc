/**
 * @file fieldfibres.hpp
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDFIBRES_HPP__
#define CMZN_FIELDFIBRES_HPP__

#include "cmlibs/zinc/fieldfibres.h"
#include "cmlibs/zinc/field.hpp"
#include "cmlibs/zinc/fieldmodule.hpp"

namespace CMLibs
{
namespace Zinc
{

class FieldFibreAxes : public Field
{
private:
	// takes ownership of C handle, and responsibility for destroying it
	explicit FieldFibreAxes(cmzn_field_id field_id) : Field(field_id)
	{ }

	friend FieldFibreAxes Fieldmodule::createFieldFibreAxes(const Field& fibreField, const Field& coordinateField);

public:

	FieldFibreAxes() : Field(0)
	{	}

};

inline FieldFibreAxes Fieldmodule::createFieldFibreAxes(const Field& fibreField, const Field& coordinateField)
{
	return FieldFibreAxes(cmzn_fieldmodule_create_field_fibre_axes(id,
		fibreField.getId(), coordinateField.getId()));
}

}  // namespace Zinc
}

#endif
