/***************************************************************************//**
 * FILE : computed_field_subobject_group.cpp
 *
 * Implements region sub object groups, e.g. node group, element group.
 */
/* ***** BEGIN LICENSE BLOCK *****
* Version: MPL 1.1/GPL 2.0/LGPL 2.1
*element_group
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
extern "C" {
#include <stdlib.h>
#include "api/cmiss_element.h"
#include "api/cmiss_node.h"
#include "api/cmiss_field_module.h"
#include "api/cmiss_field_subobject_group.h"
#include "computed_field/computed_field.h"
#if defined (USE_OPENCASCADE)
#include "api/cmiss_field_cad.h"
#endif /* defined (USE_OPENCASCADE) */
}

#include "computed_field/computed_field_subobject_group.hpp"
#include "computed_field/computed_field_private.hpp"
extern "C" {
#include "finite_element/finite_element_region.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "mesh/cmiss_element_private.h"
#include "region/cmiss_region.h"
#include "user_interface/message.h"
}
#if defined (USE_OPENCASCADE)
#include "cad/computed_field_cad_topology.h"
#include "cad/element_identifier.h"
#endif /* defined (USE_OPENCASCADE) */


class Computed_field_sub_group_object_package : public Computed_field_type_package
{
public:
	Cmiss_region *root_region;

	Computed_field_sub_group_object_package(Cmiss_region *root_region)
		: root_region(root_region)
	{
		ACCESS(Cmiss_region)(root_region);
	}
	
	~Computed_field_sub_group_object_package()
	{
		DEACCESS(Cmiss_region)(&root_region);
	}
};


Cmiss_field_node_group *Cmiss_field_cast_node_group(Cmiss_field_id field)
{
	if (field && dynamic_cast<Computed_field_node_group *>(field->core))
	{
		Cmiss_field_access(field);
		return (reinterpret_cast<Cmiss_field_node_group_id>(field));
	}
	else
	{
		return (NULL);
	}
}

inline Computed_field *Computed_field_cast(
	Cmiss_field_node_group *node_group_field)
{
	return (reinterpret_cast<Computed_field*>(node_group_field));
}

int Cmiss_field_node_group_add_node(Cmiss_field_node_group_id node_group,
		Cmiss_node_id node)
{
	int return_code = 1;

	if (node_group && node)
	{
	  Computed_field_node_group *group_core =
	  	Computed_field_node_group_core_cast(node_group);
		group_core->addObject(node);
	}
	else
	{
		return_code = 0;
	}

	return return_code;
}

int Cmiss_field_node_group_remove_node(Cmiss_field_node_group_id node_group,
		Cmiss_node_id node)
{
	int return_code = 1;

	if (node_group && node)
	{
		Computed_field_node_group *group_core =
			Computed_field_node_group_core_cast(node_group);
		group_core->removeObject(node);
	}
	else
	{
		return_code = 0;
	}

	return return_code;
}

int Cmiss_field_node_group_clear(Cmiss_field_node_group_id node_group)
{
	int return_code = 1;

	if (node_group)
	{
		Computed_field_node_group *group_core =
			Computed_field_node_group_core_cast(node_group);
		group_core->clear();
	}
	else
	{
		return_code = 0;
	}

	return return_code;
}

int Cmiss_field_node_group_contains_node(
	Cmiss_field_node_group_id node_group,	Cmiss_node_id node)
{
	int return_code = 0;

	if (node_group && node)
	{
		Computed_field_node_group *group_core =
			Computed_field_node_group_core_cast(node_group);
		return_code = group_core->containsObject(node);
	}

	return return_code;
}

int Cmiss_field_node_group_is_empty(Cmiss_field_node_group_id node_group)
{
	int return_code = 0;
	if (node_group)
	{
		Computed_field_node_group *group_core =
			Computed_field_node_group_core_cast(node_group);
		return_code = group_core->isEmpty();
	}

	return return_code;
}

Cmiss_node_iterator_id Cmiss_field_node_group_create_node_iterator(Cmiss_field_node_group_id node_group)
{
	Cmiss_node_iterator_id iterator = NULL;
	if (node_group)
	{
		Computed_field_node_group *group_core =
			Computed_field_node_group_core_cast(node_group);
		iterator = group_core->createIterator();
	}

	return iterator;
}

int Cmiss_field_node_group_destroy(Cmiss_field_node_group_id *node_group_address)
{
	return Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id *>(node_group_address));
}

Computed_field *Cmiss_field_module_create_node_group(Cmiss_field_module_id field_module, Cmiss_nodeset_id nodeset)
{
	Computed_field *field;

	ENTER(Computed_field_create_group);
	field = (Computed_field *)NULL;
	if (field_module && nodeset)
	{
		FE_region *fe_region = reinterpret_cast<FE_region *>(nodeset);
		struct LIST(FE_node) *fe_node_list = FE_region_create_related_node_list(fe_region);
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/false, 1,
			/*number_of_source_fields*/0, NULL,
			/*number_of_source_values*/0, NULL,
			new Computed_field_node_group(fe_node_list));

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_group.  Invalid argument(s)");
	}
	LEAVE;

	return (field);
} /* Cmiss_field_module_create_group */

Cmiss_field_element_group *Cmiss_field_cast_element_group(Cmiss_field_id field)
{
	if (field && dynamic_cast<Computed_field_element_group*>(field->core))
	{
		Cmiss_field_access(field);
		return (reinterpret_cast<Cmiss_field_element_group_id>(field));
	}
	else
	{
		return (NULL);
	}
}

inline Computed_field *Computed_field_cast(
	Cmiss_field_element_group *element_group_field)
{
	return (reinterpret_cast<Computed_field*>(element_group_field));
}

int Cmiss_field_element_group_add_element(Cmiss_field_element_group_id element_group,
	Cmiss_element_id element)
{
	int return_code = 1;
	if (element_group && element)
	{
		Computed_field_element_group *group_core =
			Computed_field_element_group_core_cast(element_group);
		group_core->addObject(element);
	}
	else
	{
		return_code = 0;
	}

	return return_code;
}

int Cmiss_field_element_group_remove_element(Cmiss_field_element_group_id element_group,
		Cmiss_element_id element)
{
	int return_code = 1;
	if (element_group && element)
	{
		Computed_field_element_group *group_core =
			Computed_field_element_group_core_cast(element_group);
		group_core->removeObject(element);
	}
	else
	{
		return_code = 0;
	}

	return return_code;
}

int Cmiss_field_element_group_clear(Cmiss_field_element_group_id element_group)
{
	int return_code = 1;

	if (element_group)
	{
		Computed_field_element_group *group_core =
			Computed_field_element_group_core_cast(element_group);
		group_core->clear();
	}
	else
	{
		return_code = 0;
	}

	return return_code;
}

int Cmiss_field_element_group_contains_element(
	Cmiss_field_element_group_id element_group, Cmiss_element_id element)
{
	int return_code = 0;
	if (element_group && element)
	{
		Computed_field_element_group *group_core =
			Computed_field_element_group_core_cast(element_group);
		return_code = group_core->containsObject(element);
	}

	return return_code;
}

int Cmiss_field_element_group_is_empty(Cmiss_field_element_group_id element_group)
{
	int return_code = 0;
	if (element_group)
	{
		Computed_field_element_group *group_core =
			Computed_field_element_group_core_cast(element_group);
		return_code = group_core->isEmpty();
	}

	return return_code;
}

Cmiss_element_iterator_id Cmiss_field_element_group_create_element_iterator(Cmiss_field_element_group_id element_group)
{
	Cmiss_element_iterator_id iterator = NULL;
	if (element_group)
	{
		Computed_field_element_group *group_core =
			Computed_field_element_group_core_cast(element_group);
		iterator = group_core->createIterator();
	}

	return iterator;
}

Computed_field *Cmiss_field_module_create_element_group(Cmiss_field_module_id field_module,
		Cmiss_fe_mesh_id mesh)
{
	Computed_field *field;

	ENTER(Cmiss_field_module_create_element_group);
	field = (Computed_field *)NULL;
	if (field_module && mesh)
	{
		FE_region *fe_region = Cmiss_fe_mesh_get_fe_region(mesh);
		struct LIST(FE_element) *fe_element_list = FE_region_create_related_element_list_for_dimension(fe_region,
			Cmiss_fe_mesh_get_dimension(mesh));
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/false, 1,
			/*number_of_source_fields*/0, NULL,
			/*number_of_source_values*/0, NULL,
			new Computed_field_element_group(fe_element_list, Cmiss_fe_mesh_get_dimension(mesh)));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_group.  Invalid argument(s)");
	}
	LEAVE;

	return (field);
} /* Cmiss_field_module_create_group */

int Cmiss_field_element_group_destroy(Cmiss_field_element_group_id *element_group_address)
{
	return Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id *>(element_group_address));
}


#if defined (USE_OPENCASCADE)

Cmiss_field_id Cmiss_field_module_create_cad_primitive_group_template(Cmiss_field_module_id field_module)
{
	Computed_field *field = (struct Computed_field *)NULL;

	if (field_module)
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/false, 1,
			/*number_of_source_fields*/0, NULL,
			/*number_of_source_values*/0, NULL,
			new Computed_field_sub_group_object<Cmiss_cad_identifier_id>());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_group.  Invalid argument(s)");
	}

	return (field);
}

int Cmiss_field_cad_primitive_group_template_destroy(Cmiss_field_cad_primitive_group_template_id *cad_primitive_group_address)
{
	return Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id *>(cad_primitive_group_address));
}

Cmiss_field_cad_primitive_group_template_id Cmiss_field_cast_cad_primitive_group_template(Cmiss_field_id field)
{
	if (dynamic_cast<Computed_field_sub_group_object<Cmiss_cad_identifier_id>*>(field->core))
	{
		Cmiss_field_access(field);
		return (reinterpret_cast<Cmiss_field_cad_primitive_group_template_id>(field));
	}
	else
	{
		return (NULL);
	}
}

int Cmiss_field_cad_primitive_group_template_add_cad_primitive(Cmiss_field_cad_primitive_group_template_id cad_primitive_group,
	Cmiss_cad_identifier_id cad_primitive)
{
	int return_code = 0;
	if (cad_primitive_group && cad_primitive)
	{
		Computed_field_sub_group_object<Cmiss_cad_identifier_id> *group_core =
			Computed_field_sub_group_object_core_cast<Cmiss_cad_identifier_id,
			Cmiss_field_cad_primitive_group_template_id>(cad_primitive_group);
		int identifier = cad_primitive->identifier.number;
		Cmiss_cad_identifier_id cad_primitive_copy = new Cmiss_cad_identifier(cad_primitive->cad_topology, cad_primitive->identifier);
		//DEBUG_PRINT("=== Adding cad primitive object %p %d %d\n", cad_primitive_copy, cad_primitive->identifier.type, cad_primitive->identifier.number);
		group_core->add_object(identifier, cad_primitive_copy);
		return_code = 1;
	}

	return return_code;
}

int Cmiss_field_cad_primitive_group_template_remove_cad_primitive(Cmiss_field_cad_primitive_group_template_id cad_primitive_group,
	Cmiss_cad_identifier_id cad_primitive)
{
	int return_code = 0;
	if (cad_primitive_group && cad_primitive)
	{
		Computed_field_sub_group_object<Cmiss_cad_identifier_id> *group_core =
			Computed_field_sub_group_object_core_cast<Cmiss_cad_identifier_id,
				Cmiss_field_cad_primitive_group_template_id>(cad_primitive_group);
		int identifier = cad_primitive->identifier.number;
		//DEBUG_PRINT("=== Removing cad primitive object %p %d %d\n", cad_primitive, cad_primitive->identifier.type, cad_primitive->identifier.number);
		Cmiss_cad_identifier_id cad_identifier = group_core->get_object(identifier);
		if (cad_identifier)
		{
			//DEBUG_PRINT("Deleting %p\n", cad_identifier);
			delete cad_identifier;
			group_core->remove_object(identifier);
			return_code = 1;
		}
	}

	return return_code;

}

int Cmiss_field_cad_primitive_group_template_clear(Cmiss_field_cad_primitive_group_template_id cad_primitive_group)
{
	int return_code = 0;

	if (cad_primitive_group)
	{
		Computed_field_sub_group_object<Cmiss_cad_identifier_id> *group_core =
			Computed_field_sub_group_object_core_cast<Cmiss_cad_identifier_id,
				Cmiss_field_cad_primitive_group_template_id>(cad_primitive_group);
		Cmiss_cad_identifier_id cad_identifier = group_core->getFirstObject();
		while (cad_identifier != NULL)
		{
			delete cad_identifier;
			cad_identifier = group_core->getNextObject();
		}
		return_code = group_core->clear();
	}

	return return_code;
}

int Cmiss_field_cad_primitive_group_template_is_cad_primitive_selected(
	Cmiss_field_cad_primitive_group_template_id cad_primitive_group, Cmiss_cad_identifier_id cad_primitive)
{
	int return_code = 0;
	//struct CM_element_information cm_identifier;
	if (cad_primitive_group && cad_primitive)
	{
		Computed_field_sub_group_object<Cmiss_cad_identifier_id> *group_core =
			Computed_field_sub_group_object_core_cast<Cmiss_cad_identifier_id,
			Cmiss_field_cad_primitive_group_template_id>(cad_primitive_group);
		int identifier = cad_primitive->identifier.number;
		return_code = group_core->get_object_selected(identifier, cad_primitive);
	}

	return return_code;
}

Cmiss_cad_identifier_id Cmiss_field_cad_primitive_group_template_get_first_cad_primitive(
	Cmiss_field_cad_primitive_group_template_id cad_primitive_group)
{
	Cmiss_cad_identifier_id cad_identifier = NULL;
	if (cad_primitive_group)
	{
		Computed_field_sub_group_object<Cmiss_cad_identifier_id> *group_core =
			Computed_field_sub_group_object_core_cast<Cmiss_cad_identifier_id,
			Cmiss_field_cad_primitive_group_template_id>(cad_primitive_group);
		Cmiss_cad_identifier_id cad_identifier_from_group = group_core->getFirstObject();
		if (cad_identifier_from_group)
		{
			cad_identifier = new Cmiss_cad_identifier(*cad_identifier_from_group);
			//Cmiss_field_cad_topology_access(cad_identifier->cad_topology);
		}
	}

	return cad_identifier;
}

Cmiss_cad_identifier_id Cmiss_field_cad_primitive_group_template_get_next_cad_primitive(
	Cmiss_field_cad_primitive_group_template_id cad_primitive_group)
{
	Cmiss_cad_identifier_id cad_identifier = NULL;
	if (cad_primitive_group)
	{
		Computed_field_sub_group_object<Cmiss_cad_identifier_id> *group_core =
			Computed_field_sub_group_object_core_cast<Cmiss_cad_identifier_id,
			Cmiss_field_cad_primitive_group_template_id>(cad_primitive_group);
		Cmiss_cad_identifier_id cad_identifier_from_group = group_core->getNextObject();
		if (cad_identifier_from_group)
		{
			cad_identifier = new Cmiss_cad_identifier(*cad_identifier_from_group);
			//Cmiss_field_cad_topology_access(cad_identifier->cad_topology);
		}
	}

	return cad_identifier;
}

#endif /* defined (USE_OPENCASCADE) */

