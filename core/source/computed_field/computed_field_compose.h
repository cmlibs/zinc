/*******************************************************************************
FILE : computed_field_compose.h

LAST MODIFIED : 7 January 2003

DESCRIPTION :
==============================================================================*/
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
 * Portions created by the Initial Developer are Copyright (C) 2005
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
#if !defined (COMPUTED_FIELD_COMPOSE_H)
#define COMPUTED_FIELD_COMPOSE_H

#include "finite_element/finite_element.h"
#include "region/cmiss_region.h"
#include "computed_field/computed_field_private.hpp"


class Computed_field_compose_package : public Computed_field_type_package
{
public:
	struct cmzn_region *root_region;
};

const char computed_field_compose_type_string[] = "compose";

class Computed_field_compose : public Computed_field_core
{
private:
	cmzn_mesh_id mesh;
	int find_nearest;
	int use_point_five_when_out_of_bounds;

public:
	Computed_field_compose(cmzn_mesh_id search_mesh,
			int find_nearest, int use_point_five_when_out_of_bounds = 0) :
		Computed_field_core(),
		mesh(cmzn_mesh_access(search_mesh)),
		find_nearest(find_nearest),
		use_point_five_when_out_of_bounds(use_point_five_when_out_of_bounds)
	{
	}

	virtual ~Computed_field_compose();

	virtual void inherit_source_field_attributes()
	{
		if (field)
		{
			/* inherit coordinate system from third source field */
			Computed_field *calculate_values_field = field->source_fields[2];
			Computed_field_set_coordinate_system(field,
				Computed_field_get_coordinate_system(calculate_values_field));
		}
	}

	int get_type(
		struct Computed_field **texture_coordinate_field_address,
		struct Computed_field **find_element_xi_field_address,
		struct Computed_field **calculate_values_field_address,
		cmzn_mesh_id *mesh_address, int *find_nearest_address,
		int *use_point_five_when_out_of_bounds_address);

private:
	Computed_field_core *copy();

	const char *get_type_string()
	{
		return(computed_field_compose_type_string);
	}

	int compare(Computed_field_core* other_field);

	virtual bool is_defined_at_location(cmzn_field_cache& cache);

	virtual FieldValueCache *createValueCache(cmzn_field_cache& parentCache)
	{
		RealFieldValueCache *valueCache = new RealFieldValueCache(field->number_of_components);
		valueCache->createExtraCache(parentCache, Computed_field_get_region(field));
		return valueCache;
	}

	int evaluate(cmzn_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();
};

#endif /* !defined (COMPUTED_FIELD_COMPOSE_H) */
