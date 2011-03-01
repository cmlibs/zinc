/***************************************************************************//**
 * FILE : CmissField.hpp
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
#ifndef __CMISS_FIELD_HPP__
#define __CMISS_FIELD_HPP__

extern "C" {
#include "api/cmiss_field.h"
}

namespace Cmiss
{

class FieldModule;
class FieldImage;

class Field
{
friend class FieldModule;

protected:
	Cmiss_field_id id;

public:
	Field() : id(NULL)
	{ }

	// takes ownership of C-style region reference
	Field(Cmiss_field_id field_id) : id(field_id)
	{ }

	Field(const Field& field) :
		id(Cmiss_field_access(field.id))
	{ }

	Field& operator=(const Field& field)
	{
		Cmiss_field_id temp_id = Cmiss_field_access(field.id);
		if (NULL != id)
		{
			Cmiss_field_destroy(&id);
		}
		id = temp_id;
		return *this;
	}
	
	~Field()
	{
		if (NULL != id)
		{
			Cmiss_field_destroy(&id);
		}
	}
	
	bool isValid() const
	{
		return (NULL != id);
	}

	char *getName()
	{
		return Cmiss_field_get_name(id);
	}

	int setName(const char *name)
	{
		return Cmiss_field_set_name(id, name);
	}

	int getAttributeInteger(Cmiss_field_attribute_id attribute_id)
	{
		return Cmiss_field_get_attribute_integer(id, attribute_id);
	}

	int setAttributeInteger(Cmiss_field_attribute_id attribute_id, int value)
	{
		return Cmiss_field_set_attribute_integer(id, attribute_id, value);
	}

	template<class FieldType> FieldType cast()
	{
		return FieldType(*this);
	}

	// needed for casting constructors: see FieldImage(Field&)
	Cmiss_field_id getId()
	{
		return id;
	}
};

} // namespace Cmiss

#endif /* __CMISS_FIELD_HPP__ */
