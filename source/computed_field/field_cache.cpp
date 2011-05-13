/***************************************************************************//**
 * FILE : field_cache.cpp
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

extern "C" {
#include "api/cmiss_field.h"
#include "finite_element/finite_element.h"
#include "region/cmiss_region.h"
}
#include "computed_field/computed_field_private.hpp"
#include "computed_field/field_cache.hpp"


Cmiss_field_cache::~Cmiss_field_cache()
{
	// clear cache for any evaluated fields
	// but, don't know which have been evaluated, so clear all in region
	// This will be fixed when field_cache actually owns its cached values.
	MANAGER(Computed_field) *manager = Cmiss_region_get_Computed_field_manager(region);
	const Cmiss_set_Cmiss_field& field_set = Computed_field_manager_get_fields(manager);
	Cmiss_set_Cmiss_field::const_iterator iter = field_set.begin();
	for (; iter != field_set.end(); ++iter)
	{
		Cmiss_field *field = *iter;
		Cmiss_field_clear_cache_non_recursive(field);
	}
	delete location;
	Cmiss_region_destroy(&region);
}

/*
Global functions
----------------
*/

Cmiss_field_cache_id Cmiss_field_module_create_cache(Cmiss_field_module_id field_module)
{
	return Cmiss_field_cache::create(field_module);
}

Cmiss_field_cache_id Cmiss_field_cache_access(Cmiss_field_cache_id cache)
{
	return cache->access();
}

int Cmiss_field_cache_destroy(Cmiss_field_cache_id *cache_address)
{
	if (!cache_address)
		return 0;
	return Cmiss_field_cache::deaccess(*cache_address);
}

int Cmiss_field_cache_set_time(Cmiss_field_cache_id cache, double time)
{
	if (!cache)
		return 0;
	cache->get_location()->set_time(time);
	cache->increment_cache_counter();
	return 1;
}

int Cmiss_field_cache_set_element_location_with_parent(
	Cmiss_field_cache_id cache, Cmiss_element_id element,
	int number_of_chart_coordinates, double *chart_coordinates,
	Cmiss_element_id top_level_element)
{
	if (!cache)
		return 0;
	Field_element_xi_location *element_xi_location =
		dynamic_cast<Field_element_xi_location *>(cache->get_location());
	bool new_element_xi_location = !element_xi_location;
	if (new_element_xi_location)
	{
		element_xi_location = new Field_element_xi_location(cache->get_location()->get_time());
	}
	if (element_xi_location->set_element_xi(element, number_of_chart_coordinates,
		chart_coordinates, top_level_element))
	{
		if (new_element_xi_location)
		{
			cache->replace_location(element_xi_location);
		}
		cache->increment_cache_counter();
		return 1;
	}
	if (new_element_xi_location)
	{
		delete element_xi_location;
	}
	return 0;
}

int Cmiss_field_cache_set_element_location(Cmiss_field_cache_id cache,
	Cmiss_element_id element, int number_of_chart_coordinates,
	double *chart_coordinates)
{
	return Cmiss_field_cache_set_element_location_with_parent(cache, element,
		number_of_chart_coordinates, chart_coordinates, /*top_level_element*/0);
}

int Cmiss_field_cache_set_node(Cmiss_field_cache_id cache, Cmiss_node_id node)
{
	if ((!cache) || (!node))
		return 0;
	Field_node_location *node_location =
		dynamic_cast<Field_node_location *>(cache->get_location());
	if (node_location)
	{
		node_location->set_node(node);
	}
	else
	{
		node_location = new Field_node_location(node, cache->get_location()->get_time());
		cache->replace_location(node_location);
	}
	cache->increment_cache_counter();
	return 1;
}

int Cmiss_field_cache_set_field_real(Cmiss_field_cache_id cache,
	Cmiss_field_id reference_field, int number_of_values, double *values)
{
	if (!cache)
		return 0;
	Field_coordinate_location *field_location =
		dynamic_cast<Field_coordinate_location *>(cache->get_location());
	bool new_field_location = !field_location;
	if (new_field_location)
	{
		field_location = new Field_coordinate_location(cache->get_location()->get_time());
	}
	if (field_location->set_field_values(reference_field, number_of_values, values))
	{
		if (new_field_location)
		{
			cache->replace_location(field_location);
		}
		cache->increment_cache_counter();
		return 1;
	}
	if (new_field_location)
	{
		delete field_location;
	}
	return 0;
}
