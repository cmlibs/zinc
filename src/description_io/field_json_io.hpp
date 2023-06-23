/***************************************************************************//**
 * FILE : field_json_io.hpp
 *
 * The interface to field_json_io.
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (FIELD_JSON_IO_HPP)
#define FIELD_JSON_IO_HPP

#include "cmlibs/zinc/field.h"
#include "cmlibs/zinc/field.hpp"
#include "cmlibs/zinc/fieldmodule.h"
#include "cmlibs/zinc/fieldmodule.hpp"
#include "description_io/fieldmodule_json_io.hpp"
#include "jsoncpp/json.h"
#include <string>

CMLibs::Zinc::Field importTypeSpecificField(
	CMLibs::Zinc::Fieldmodule &fieldmodule, const Json::Value &fieldSettings,
	FieldmoduleJsonImport *jsonImport);

class FieldJsonIO
{

public:

	/*
	 * With order set to positive integer, order will be use as key.
	 */

	enum IOMode
	{
		IO_MODE_INVALID = 0,
		IO_MODE_IMPORT = 1,
		IO_MODE_EXPORT= 2
	};

	FieldJsonIO(cmzn_field_id field_in, cmzn_fieldmodule_id fieldmodule_in, IOMode mode_in) :
		field(cmzn_field_access(field_in)), fieldmodule(cmzn_fieldmodule_access(fieldmodule_in)),
		mode(mode_in)
	{  }

	FieldJsonIO(const CMLibs::Zinc::Field &field_in,
		const CMLibs::Zinc::Fieldmodule &fieldmodule_in, IOMode mode_in) :
		field(field_in), fieldmodule(fieldmodule_in), mode(mode_in)
	{	}

	/* serialise fields definition to json format */
	void exportEntries(Json::Value &typeSettings);

	/** deserialise fields definition from json format
	 * @return  true if field successfully given name, false if failed due to existing field of name found */
	void importEntries(const Json::Value &typeSettings);

	/* methods to customise output based on field type */
	void exportTypeSpecificParameters(Json::Value &fieldSettings);

protected:

	CMLibs::Zinc::Field field;
	CMLibs::Zinc::Fieldmodule fieldmodule;
	IOMode mode;

};

#endif

