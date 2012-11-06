/***************************************************************************//**
 * FILE : spectrum.hpp
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
#ifndef __ZN_SPECTRUM_HPP__
#define __ZN_SPECTRUM_HPP__

#include "zinc/spectrum.h"

namespace zinc
{

class Spectrum
{
protected:
	Cmiss_spectrum_id id;

public:

	Spectrum() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Spectrum(Cmiss_spectrum_id in_spectrum_id) :
		id(in_spectrum_id)
	{  }

	Spectrum(const Spectrum& spectrum) :
		id(Cmiss_spectrum_access(spectrum.id))
	{  }

	Spectrum& operator=(const Spectrum& spectrum)
	{
		Cmiss_spectrum_id temp_id = Cmiss_spectrum_access(spectrum.id);
		if (0 != id)
		{
			Cmiss_spectrum_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Spectrum()
	{
		if (0 != id)
		{
			Cmiss_spectrum_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Cmiss_spectrum_id getId()
	{
		return id;
	}

	enum Attribute
	{
		ATTRIBUTE_INVALID = CMISS_SPECTRUM_ATTRIBUTE_INVALID,
		ATTRIBUTE_IS_MANAGED = CMISS_SPECTRUM_ATTRIBUTE_IS_MANAGED,
	};

	int getAttributeInteger(Attribute attribute)
	{
		return Cmiss_spectrum_get_attribute_integer(id,
			static_cast<Cmiss_spectrum_attribute>(attribute));
	}

	int setAttributeInteger(Attribute attribute, int value)
	{
		return Cmiss_spectrum_set_attribute_integer(id,
			static_cast<Cmiss_spectrum_attribute>(attribute), value);
	}

	char *getName()
	{
		return Cmiss_spectrum_get_name(id);
	}

	int setName(const char *name)
	{
		return Cmiss_spectrum_set_name(id, name);
	}

	int executeCommand(const char *commandString)
	{
		return Cmiss_spectrum_execute_command(id, commandString);
	}

};

}  // namespace zinc

#endif /* __ZN_SPECTRUM_HPP__ */
