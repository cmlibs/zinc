/***************************************************************************//**
 * FILE : field_cache.hpp
 * 
 * Implements a cache for prescribed domain locations and field evaluations.
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
 * Portions created by the Initial Developer are Copyright (C) 2011
 * the Initial Developer. All Rights Reserved.
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

#if !defined (FIELD_CACHE_HPP)
#define FIELD_CACHE_HPP

extern "C" {
#include "api/cmiss_field.h"
#include "api/cmiss_field_module.h"
#include "api/cmiss_region.h"
}
#include "computed_field/field_location.hpp"

struct Cmiss_field_cache
{
private:
	Cmiss_region_id region;
	unsigned int cache_counter;
	Field_location *location;
	int access_count;

	Cmiss_field_cache(Cmiss_field_module_id field_module) :
		region(Cmiss_field_module_get_region(field_module)),
		cache_counter(0),
		location(new Field_time_location()),
		access_count(1)
	{
	}

	~Cmiss_field_cache();

public:

	static Cmiss_field_cache_id create(Cmiss_field_module_id field_module)
	{
		// GRC Add to region's list

		return new Cmiss_field_cache(field_module);
	}

	Cmiss_field_cache_id access()
	{
		++access_count;
		return this;
	}

	static int deaccess(Cmiss_field_cache_id &cache)
	{
		if (!cache)
			return 0;
		--(cache->access_count);
		if (cache->access_count <= 0)
			delete cache;
		cache = 0;
		return 1;
	}

	/** caller is allowed to modify location */
	Field_location *get_location()
	{
		return location;
	}

	/** takes ownership of the supplied location - be careful */
	void replace_location(Field_location *new_location)
	{
		delete location;
		location = new_location;
	}

	void increment_cache_counter()
	{
		++cache_counter;
	}
};

#endif /* !defined (FIELD_CACHE_HPP) */
