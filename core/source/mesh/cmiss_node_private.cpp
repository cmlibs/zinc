/***************************************************************************//**
 * FILE : cmiss_node_private.cpp
 *
 * Implementation of public interface to Cmiss_node.
 *
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
 * Portions created by the Initial Developer are Copyright (C) 2005-2010
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

#include <stdarg.h>
#include "zinc/fieldmodule.h"
#include "zinc/node.h"
#include "zinc/timesequence.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "computed_field/computed_field_finite_element.h"
#include "node/node_operations.h"
#include "general/message.h"
#include "computed_field/field_module.hpp"
#include "general/enumerator_conversion.hpp"
#include "mesh/cmiss_node_private.hpp"
#include <vector>
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_subobject_group_private.hpp"

/*
Global types
------------
*/

/*============================================================================*/

namespace {

class Cmiss_node_field
{
	FE_field *fe_field;
	FE_node_field_creator *node_field_creator;
	FE_time_sequence *time_sequence;

public:

	Cmiss_node_field(FE_field *fe_field) :
		fe_field(ACCESS(FE_field)(fe_field)),
		node_field_creator(
			CREATE(FE_node_field_creator)(get_FE_field_number_of_components(fe_field))),
		time_sequence(NULL)
	{
	}

	~Cmiss_node_field()
	{
		if (time_sequence)
			DEACCESS(FE_time_sequence)(&time_sequence);
		DESTROY(FE_node_field_creator)(&node_field_creator);
		DEACCESS(FE_field)(&fe_field);
	}

	int defineDerivative(int component_number, enum FE_nodal_value_type derivative_type)
	{
		int first = 0;
		int limit = get_FE_field_number_of_components(fe_field);
		if ((component_number < -1) || (component_number == 0) || (component_number > limit))
			return 0;
		if (component_number > 0)
		{
			first = component_number - 1;
			limit = component_number;
		}
		int return_code = 1;
		for (int i = first; i < limit; i++)
		{
			if (!FE_node_field_creator_define_derivative(node_field_creator, i, derivative_type))
				return_code = 0;
		}
		return return_code;
	}

	int defineTimeSequence(FE_time_sequence *in_time_sequence)
	{
		return REACCESS(FE_time_sequence)(&time_sequence, in_time_sequence);
	}

	/** note: does not ACCESS */
	Cmiss_time_sequence_id getTimeSequence()
	{
		return reinterpret_cast<Cmiss_time_sequence_id>(time_sequence);
	}

	int defineVersions(int component_number, int number_of_versions)
	{
		int first = 0;
		int limit = get_FE_field_number_of_components(fe_field);
		if ((component_number < -1) || (component_number == 0) || (component_number > limit))
			return 0;
		if (component_number > 0)
		{
			first = component_number - 1;
			limit = component_number;
		}
		int return_code = 1;
		for (int i = first; i < limit; i++)
		{
			if (!FE_node_field_creator_define_versions(node_field_creator, i, number_of_versions))
				return_code = 0;
		}
		return return_code;
	}

	int defineAtNode(FE_node *node)
	{
		return define_FE_field_at_node(node, fe_field,
			time_sequence, node_field_creator);
	}

	int getNumberOfVersions(int component_number)
	{
		int number_of_components = get_FE_field_number_of_components(fe_field);
		if ((component_number < -1) || (component_number == 0) || (component_number > number_of_components))
			return 0;
		return FE_node_field_creator_get_number_of_versions(node_field_creator, component_number - 1);
	}

	int hasDerivative(int component_number, enum FE_nodal_value_type derivative_type)
	{
		int number_of_components = get_FE_field_number_of_components(fe_field);
		if ((component_number < -1) || (component_number == 0) || (component_number > number_of_components))
			return 0;
		return FE_node_field_creator_has_derivative(node_field_creator, component_number - 1, derivative_type);
	}

	FE_field *getFeField() const { return fe_field; }
};

}

/*============================================================================*/

struct Cmiss_node_template
{
private:
	FE_region *fe_region;
	FE_node *template_node;
	std::vector<Cmiss_node_field*> fields;
	std::vector<FE_field*> undefine_fields; // ACCESSed
	int access_count;

public:
	Cmiss_node_template(FE_region *fe_region) :
		fe_region(ACCESS(FE_region)(fe_region)),
		template_node(NULL),
		access_count(1)
	{
	}

	Cmiss_node_template_id access()
	{
		++access_count;
		return this;
	}

	static int deaccess(Cmiss_node_template_id &node_template)
	{
		if (!node_template)
			return 0;
		--(node_template->access_count);
		if (node_template->access_count <= 0)
			delete node_template;
		node_template = 0;
		return 1;
	}

	int defineField(Cmiss_field_id field)
	{
		if (!checkValidFieldForDefine(field))
			return 0;
		FE_field *fe_field = NULL;
		Computed_field_get_type_finite_element(field, &fe_field);
		if (getNodeField(fe_field))
		{
			return 0;
		}
		if (getUndefineNodeField(fe_field))
		{
			return 0;
		}
		clearTemplateNode();
		Cmiss_node_field *node_field = createNodeField(fe_field);
		return (node_field != NULL);
	}

	int defineFieldFromNode(Cmiss_field_id field, Cmiss_node_id node)
	{
		if (!checkValidFieldForDefine(field))
			return 0;
		if (!FE_region_contains_FE_node(fe_region, node))
			return 0;

		FE_field *fe_field = NULL;
		Computed_field_get_type_finite_element(field, &fe_field);
		if (getNodeField(fe_field))
		{
			return 0;
		}
		if (getUndefineNodeField(fe_field))
		{
			return 0;
		}

		const enum FE_nodal_value_type all_fe_nodal_value_types[] = {
			FE_NODAL_VALUE,
			FE_NODAL_D_DS1,
			FE_NODAL_D_DS2,
			FE_NODAL_D_DS3,
			FE_NODAL_D2_DS1DS2,
			FE_NODAL_D2_DS1DS3,
			FE_NODAL_D2_DS2DS3,
			FE_NODAL_D3_DS1DS2DS3
		};
		const int number_of_fe_value_types = sizeof(all_fe_nodal_value_types) / sizeof(enum FE_nodal_value_type);
		clearTemplateNode();
		Cmiss_node_field *node_field = createNodeField(fe_field);
		int number_of_components = Cmiss_field_get_number_of_components(field);
		for (int component_number = 1; component_number <= number_of_components; ++component_number)
		{
			for (int i = 1; i < number_of_fe_value_types; ++i)
			{
				enum FE_nodal_value_type fe_nodal_value_type = all_fe_nodal_value_types[i];
				if (FE_nodal_value_version_exists(node, fe_field,
					component_number - 1, /*version*/0, fe_nodal_value_type))
				{
					node_field->defineDerivative(component_number, fe_nodal_value_type);
				}
			}
			// versions should be per-nodal-value-type, but are not currently
			int number_of_versions = get_FE_node_field_component_number_of_versions(node, fe_field, component_number - 1);
			if (number_of_versions > 1)
			{
				node_field->defineVersions(component_number, number_of_versions);
			}
		}
		struct FE_time_sequence *time_sequence = get_FE_node_field_FE_time_sequence(node, fe_field);
		if (time_sequence)
		{
			node_field->defineTimeSequence(time_sequence);
		}
		return (node_field != NULL);
	}

	int defineDerivative(Cmiss_field_id field, int component_number,
		enum Cmiss_nodal_value_type derivative_type)
	{
		Cmiss_field_finite_element_id finite_element_field = Cmiss_field_cast_finite_element(field);
		if (!finite_element_field)
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_node_template_define_derivative.  Field must be real finite_element type");
			return 0;
		}
		Cmiss_field_finite_element_destroy(&finite_element_field);
		FE_field *fe_field = NULL;
		Computed_field_get_type_finite_element(field, &fe_field);
		Cmiss_node_field *node_field = getNodeField(fe_field);
		if (!node_field)
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_node_template_define_derivative.  Field is not defined yet");
			return 0;
		}
		enum FE_nodal_value_type fe_nodal_value_type =
			Cmiss_nodal_value_type_to_FE_nodal_value_type(derivative_type);
		if (FE_NODAL_UNKNOWN == fe_nodal_value_type)
			return 0;
		clearTemplateNode();
		return node_field->defineDerivative(component_number, fe_nodal_value_type);
	}

	int defineTimeSequence(Cmiss_field_id field,
		Cmiss_time_sequence_id time_sequence)
	{
		Cmiss_field_finite_element_id finite_element_field = Cmiss_field_cast_finite_element(field);
		if (!finite_element_field)
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_node_template_define_time_sequence.  Field must be real finite_element type");
			return 0;
		}
		Cmiss_field_finite_element_destroy(&finite_element_field);
		FE_field *fe_field = NULL;
		Computed_field_get_type_finite_element(field, &fe_field);
		Cmiss_node_field *node_field = getNodeField(fe_field);
		if (!node_field)
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_node_template_define_time_sequence.  Field is not defined yet");
			return 0;
		}
		clearTemplateNode();
		return node_field->defineTimeSequence(reinterpret_cast<struct FE_time_sequence *>(time_sequence));
	}

	int defineVersions(Cmiss_field_id field, int component_number,
		int number_of_versions)
	{
		Cmiss_field_finite_element_id finite_element_field = Cmiss_field_cast_finite_element(field);
		if (!finite_element_field)
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_node_template_define_versions.  Field must be real finite_element type");
			return 0;
		}
		Cmiss_field_finite_element_destroy(&finite_element_field);
		FE_field *fe_field = NULL;
		Computed_field_get_type_finite_element(field, &fe_field);
		Cmiss_node_field *node_field = getNodeField(fe_field);
		if (!node_field)
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_node_template_define_versions.  Field is not defined yet");
			return 0;
		}
		clearTemplateNode();
		return node_field->defineVersions(component_number, number_of_versions);
	}

	int getNumberOfVersions(Cmiss_field_id field, int component_number)
	{
		Cmiss_field_finite_element_id finite_element_field = Cmiss_field_cast_finite_element(field);
		if (!finite_element_field)
			return 0;
		Cmiss_field_finite_element_destroy(&finite_element_field);
		FE_field *fe_field = NULL;
		Computed_field_get_type_finite_element(field, &fe_field);
		Cmiss_node_field *node_field = getNodeField(fe_field);
		if (!node_field)
			return 0;
		return node_field->getNumberOfVersions(component_number);
	}

	Cmiss_time_sequence_id getTimeSequence(Cmiss_field_id field)
	{
		Cmiss_field_finite_element_id finite_element_field = Cmiss_field_cast_finite_element(field);
		if (!finite_element_field)
			return 0;
		Cmiss_field_finite_element_destroy(&finite_element_field);
		FE_field *fe_field = NULL;
		Computed_field_get_type_finite_element(field, &fe_field);
		Cmiss_node_field *node_field = getNodeField(fe_field);
		if (!node_field)
			return 0;
		Cmiss_time_sequence_id timeSequence = node_field->getTimeSequence();
		if (timeSequence)
		{
			Cmiss_time_sequence_access(timeSequence);
		}
		return timeSequence;
	}

	int hasDerivative(Cmiss_field_id field, int component_number,
		enum Cmiss_nodal_value_type derivative_type)
	{
		Cmiss_field_finite_element_id finite_element_field = Cmiss_field_cast_finite_element(field);
		if (!finite_element_field)
			return 0;
		Cmiss_field_finite_element_destroy(&finite_element_field);
		FE_field *fe_field = NULL;
		Computed_field_get_type_finite_element(field, &fe_field);
		Cmiss_node_field *node_field = getNodeField(fe_field);
		if (!node_field)
			return 0;
		enum FE_nodal_value_type fe_nodal_value_type =
			Cmiss_nodal_value_type_to_FE_nodal_value_type(derivative_type);
		if (FE_NODAL_UNKNOWN == fe_nodal_value_type)
			return 0;
		return node_field->hasDerivative(component_number, fe_nodal_value_type);
	}

	int undefineField(Cmiss_field_id field)
	{
		Cmiss_field_finite_element_id finite_element_field = Cmiss_field_cast_finite_element(field);
		Cmiss_field_stored_mesh_location_id stored_mesh_location_field = Cmiss_field_cast_stored_mesh_location(field);
		Cmiss_field_stored_string_id stored_string_field = Cmiss_field_cast_stored_string(field);
		if (!(finite_element_field || stored_mesh_location_field || stored_string_field))
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_node_template_undefine_field.  "
				"Field must be finite_element, stored_mesh_location or stored_string type");
			return 0;
		}
		int return_code = 1;
		FE_field *fe_field = NULL;
		Computed_field_get_type_finite_element(field, &fe_field);
		FE_region *compare_fe_region = fe_region;
		if (FE_region_is_data_FE_region(fe_region))
		{
			 FE_region_get_immediate_master_FE_region(fe_region, &compare_fe_region);
		}
		if (FE_field_get_FE_region(fe_field) != compare_fe_region)
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_node_template_undefine_field.  Field is from another region");
			return_code = 0;
		}
		if (getNodeField(fe_field))
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_node_template_undefine_field.  Field is already being defined");
			return_code = 0;
		}
		if (getUndefineNodeField(fe_field))
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_node_template_undefine_field.  Field is already being undefined");
			return_code = 0;
		}
		Cmiss_field_finite_element_destroy(&finite_element_field);
		Cmiss_field_stored_mesh_location_destroy(&stored_mesh_location_field);
		Cmiss_field_stored_string_destroy(&stored_string_field);
		if (!return_code)
			return 0;
		clearTemplateNode();
		setUndefineNodeField(fe_field);
		return 1;
	}

	int validate()
	{
		if (template_node)
			return 1;
		template_node = ACCESS(FE_node)(
			CREATE(FE_node)(0, fe_region, (struct FE_node *)NULL));
		for (unsigned int i = 0; i < fields.size(); i++)
		{
			if (!fields[i]->defineAtNode(template_node))
			{
				DEACCESS(FE_node)(&template_node);
				break;
			}
		}
		if (!template_node)
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_node_template_validate.  Failed to create template node");
			return 0;
		}
		return 1;
	}

	// can be made more efficient
	int mergeIntoNode(Cmiss_node_id node)
	{
		int return_code = 1;
		if (validate())
		{
			if (0 < undefine_fields.size())
			{
				for (unsigned int i = 0; i < undefine_fields.size(); i++)
				{
					if (FE_field_is_defined_at_node(undefine_fields[i], node) &&
						!undefine_FE_field_at_node(node, undefine_fields[i]))
					{
						return_code = 0;
						break;
					}
				}
			}
			if ((0 < fields.size() &&
				!FE_region_merge_FE_node_existing(fe_region, node, template_node)))
			{
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_node_merge.  Node template is not valid");
			return_code = 0;
		}
		return return_code;
	}

	FE_node *getTemplateNode() { return template_node; }

private:
	~Cmiss_node_template()
	{
		for (unsigned int i = 0; i < fields.size(); i++)
		{
			delete fields[i];
		}
		for (unsigned int i = 0; i < undefine_fields.size(); i++)
		{
			DEACCESS(FE_field)(&(undefine_fields[i]));
		}
		REACCESS(FE_node)(&template_node, NULL);
		DEACCESS(FE_region)(&fe_region);
	}

	Cmiss_node_field *getNodeField(FE_field *fe_field)
	{
		for (unsigned int i = 0; i < fields.size(); i++)
		{
			if (fields[i]->getFeField() == fe_field)
			{
				return fields[i];
			}
		}
		return NULL;
	}

	/** Must call getNodeField first to confirm not already being defined */
	Cmiss_node_field *createNodeField(FE_field *fe_field)
	{
		Cmiss_node_field *node_field = new Cmiss_node_field(fe_field);
		fields.push_back(node_field);
		return node_field;
	}

	bool getUndefineNodeField(FE_field *fe_field)
	{
		for (unsigned int i = 0; i < undefine_fields.size(); i++)
		{
			if (undefine_fields[i] == fe_field)
			{
				return true;
			}
		}
		return false;
	}

	/** Must call getUndefineNodeField first to confirm not already being undefined */
	void setUndefineNodeField(FE_field *fe_field)
	{
		ACCESS(FE_field)(fe_field);
		undefine_fields.push_back(fe_field);
	}

	void clearTemplateNode()
	{
		REACCESS(FE_node)(&template_node, NULL);
	}

	bool checkValidFieldForDefine(Cmiss_field_id field)
	{
		bool result = true;
		Cmiss_field_finite_element_id finite_element_field = Cmiss_field_cast_finite_element(field);
		Cmiss_field_stored_mesh_location_id stored_mesh_location_field = Cmiss_field_cast_stored_mesh_location(field);
		Cmiss_field_stored_string_id stored_string_field = Cmiss_field_cast_stored_string(field);
		if (finite_element_field || stored_mesh_location_field || stored_string_field)
		{
			FE_field *fe_field = 0;
			Computed_field_get_type_finite_element(field, &fe_field);
			FE_region *compare_fe_region = fe_region;
			if (FE_region_is_data_FE_region(fe_region))
			{
				 FE_region_get_immediate_master_FE_region(fe_region, &compare_fe_region);
			}
			if (FE_field_get_FE_region(fe_field) != compare_fe_region)
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_node_template_define_field.  "
					"Field is from another region");
				result = false;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_node_template_define_field.  "
				"Field must be finite_element, stored_mesh_location or stored_string type");
			result = false;
		}
		Cmiss_field_finite_element_destroy(&finite_element_field);
		Cmiss_field_stored_mesh_location_destroy(&stored_mesh_location_field);
		Cmiss_field_stored_string_destroy(&stored_string_field);
		return result;
	}

	FE_nodal_value_type Cmiss_nodal_value_type_to_FE_nodal_value_type(
		enum Cmiss_nodal_value_type nodal_value_type)
	{
		FE_nodal_value_type fe_nodal_value_type = FE_NODAL_UNKNOWN;
		switch (nodal_value_type)
		{
			case CMISS_NODAL_VALUE_TYPE_INVALID:
				fe_nodal_value_type = FE_NODAL_UNKNOWN;
				break;
			case CMISS_NODAL_VALUE:
				fe_nodal_value_type = FE_NODAL_VALUE;
				break;
			case CMISS_NODAL_D_DS1:
				fe_nodal_value_type = FE_NODAL_D_DS1;
				break;
			case CMISS_NODAL_D_DS2:
				fe_nodal_value_type = FE_NODAL_D_DS2;
				break;
			case CMISS_NODAL_D_DS3:
				fe_nodal_value_type = FE_NODAL_D_DS3;
				break;
			case CMISS_NODAL_D2_DS1DS2:
				fe_nodal_value_type = FE_NODAL_D2_DS1DS2;
				break;
			case CMISS_NODAL_D2_DS1DS3:
				fe_nodal_value_type = FE_NODAL_D2_DS1DS3;
				break;
			case CMISS_NODAL_D2_DS2DS3:
				fe_nodal_value_type = FE_NODAL_D2_DS2DS3;
				break;
			case CMISS_NODAL_D3_DS1DS2DS3:
				fe_nodal_value_type = FE_NODAL_D3_DS1DS2DS3;
				break;
		}
		return fe_nodal_value_type;
	}
};

/*============================================================================*/

struct Cmiss_nodeset
{
protected:
	FE_region *fe_region;
	Cmiss_field_node_group_id group;
	int access_count;

	Cmiss_nodeset(Cmiss_field_node_group_id group) :
		fe_region(ACCESS(FE_region)(
			Computed_field_node_group_core_cast(group)->getMasterNodeset()->fe_region)),
		group(group),
		access_count(1)
	{
		// GRC Cmiss_field_node_group_access missing:
		Cmiss_field_access(Cmiss_field_node_group_base_cast(group));
	}

public:
	Cmiss_nodeset(FE_region *fe_region_in) :
		fe_region(ACCESS(FE_region)(fe_region_in)),
		group(0),
		access_count(1)
	{
	}

	Cmiss_nodeset_id access()
	{
		++access_count;
		return this;
	}

	static int deaccess(Cmiss_nodeset_id &nodeset)
	{
		if (!nodeset)
			return 0;
		--(nodeset->access_count);
		if (nodeset->access_count <= 0)
			delete nodeset;
		nodeset = 0;
		return 1;
	}

	int containsNode(Cmiss_node_id node)
	{
		if (group)
			return Computed_field_node_group_core_cast(group)->containsObject(node);
		return FE_region_contains_FE_node(fe_region, node);
	}

	Cmiss_node_id createNode(int identifier,
		Cmiss_node_template_id node_template)
	{
		Cmiss_node_id node = 0;
		if (node_template->validate())
		{
			Cmiss_node_id template_node = node_template->getTemplateNode();
			node = ACCESS(FE_node)(FE_region_create_FE_node_copy(
				fe_region, identifier, template_node));
			if (group)
				Computed_field_node_group_core_cast(group)->addObject(node);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_nodeset_create_node.  Node template is not valid");
		}
		return node;
	}

	Cmiss_node_template_id createNodeTemplate()
	{
		FE_region *master_fe_region = fe_region;
		FE_region_get_ultimate_master_FE_region(fe_region, &master_fe_region);
		return new Cmiss_node_template(master_fe_region);
	}

	Cmiss_node_iterator_id createIterator()
	{
		if (group)
			return Computed_field_node_group_core_cast(group)->createIterator();
		return FE_region_create_node_iterator(fe_region);
	}

	int destroyAllNodes()
	{
		return destroyNodesConditional(/*conditional_field*/0);
	}

	int destroyNode(Cmiss_node_id node)
	{
		if (containsNode(node))
		{
			FE_region *master_fe_region = fe_region;
			FE_region_get_ultimate_master_FE_region(fe_region, &master_fe_region);
			return FE_region_remove_FE_node(master_fe_region, node);
		}
		return 0;
	}

	int destroyNodesConditional(Cmiss_field_id conditional_field)
	{
		struct LIST(FE_node) *node_list = createNodeListWithCondition(conditional_field);
		FE_region *master_fe_region = fe_region;
		FE_region_get_ultimate_master_FE_region(fe_region, &master_fe_region);
		int return_code = FE_region_remove_FE_node_list(master_fe_region, node_list);
		DESTROY(LIST(FE_node))(&node_list);
		return return_code;
	}

	Cmiss_node_id findNodeByIdentifier(int identifier) const
	{
		Cmiss_node_id node = 0;
		if (group)
		{
			node = Computed_field_node_group_core_cast(group)->findNodeByIdentifier(identifier);
		}
		else
		{
			node = FE_region_get_FE_node_from_identifier(fe_region, identifier);
		}
		if (node)
			ACCESS(FE_node)(node);
		return node;
	}

	FE_region *getFeRegion() const { return fe_region; }

	char *getName()
	{
		char *name = 0;
		if (group)
		{
			name = Cmiss_field_get_name(Cmiss_field_node_group_base_cast(group));
		}
		else if (FE_region_is_data_FE_region(fe_region))
		{
			name = duplicate_string("cmiss_data");
		}
		else
		{
			name = duplicate_string("cmiss_nodes");
		}
		return name;
	}

	Cmiss_nodeset_id getMaster()
	{
		if (!isGroup())
			return access();
		FE_region *master_fe_region = fe_region;
		if (FE_region_get_ultimate_master_FE_region(fe_region, &master_fe_region) && master_fe_region)
			return new Cmiss_nodeset(master_fe_region);
		return 0;
	}

	int getSize() const
	{
		if (group)
			return Computed_field_node_group_core_cast(group)->getSize();
		return FE_region_get_number_of_FE_nodes(fe_region);
	}

	int isGroup()
	{
		return (0 != group);
	}

	int match(Cmiss_nodeset& other_nodeset)
	{
		return ((fe_region == other_nodeset.fe_region) &&
			(group == other_nodeset.group));
	}

protected:
	~Cmiss_nodeset()
	{
		if (group)
			Cmiss_field_node_group_destroy(&group);
		DEACCESS(FE_region)(&fe_region);
	}

	struct LIST(FE_node) *createNodeListWithCondition(Cmiss_field_id conditional_field)
	{
		Cmiss_region_id region = FE_region_get_master_Cmiss_region(fe_region);
		Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
		Cmiss_field_cache_id cache = Cmiss_field_module_create_cache(field_module);
		Cmiss_node_iterator_id iterator = createIterator();
		Cmiss_node_id node = 0;
		struct LIST(FE_node) *node_list = FE_region_create_related_node_list(fe_region);
		while (0 != (node = Cmiss_node_iterator_next_non_access(iterator)))
		{
			Cmiss_field_cache_set_node(cache, node);
			if ((!conditional_field) || Cmiss_field_evaluate_boolean(conditional_field, cache))
				ADD_OBJECT_TO_LIST(FE_node)(node, node_list);
		}
		Cmiss_node_iterator_destroy(&iterator);
		Cmiss_field_cache_destroy(&cache);
		Cmiss_field_module_destroy(&field_module);
		return node_list;
	}

};

struct Cmiss_nodeset_group : public Cmiss_nodeset
{
public:

	Cmiss_nodeset_group(Cmiss_field_node_group_id group) :
		Cmiss_nodeset(group)
	{
	}

	int addNode(Cmiss_node_id node)
	{
		return Computed_field_node_group_core_cast(group)->addObject(node);
	}

	int removeAllNodes()
	{
		return Computed_field_node_group_core_cast(group)->clear();
	}

	int removeNode(Cmiss_node_id node)
	{
		return Computed_field_node_group_core_cast(group)->removeObject(node);
	}

	int removeNodesConditional(Cmiss_field_id conditional_field)
	{
		return Computed_field_node_group_core_cast(group)->removeNodesConditional(conditional_field);
	}

	int addElementNodes(Cmiss_element_id element)
	{
		return Computed_field_node_group_core_cast(group)->addElementNodes(element);
	}

	int removeElementNodes(Cmiss_element_id element)
	{
		return Computed_field_node_group_core_cast(group)->removeElementNodes(element);
	}

};

/*
Global functions
----------------
*/

Cmiss_nodeset_id Cmiss_field_module_find_nodeset_by_domain_type(
	Cmiss_field_module_id field_module, enum Cmiss_field_domain_type domain_type)
{
	Cmiss_nodeset_id nodeset = 0;
	if (field_module)
	{
		Cmiss_region_id region = Cmiss_field_module_get_region_internal(field_module);
		FE_region *fe_region = 0;
		if (CMISS_FIELD_DOMAIN_NODES == domain_type)
		{
			fe_region = Cmiss_region_get_FE_region(region);
		}
		else if (CMISS_FIELD_DOMAIN_DATA == domain_type)
		{
			fe_region = FE_region_get_data_FE_region(Cmiss_region_get_FE_region(region));
		}
		if (fe_region)
		{
			nodeset = new Cmiss_nodeset(fe_region);
		}
	}
	return nodeset;
}

Cmiss_nodeset_id Cmiss_field_module_find_nodeset_by_name(
	Cmiss_field_module_id field_module, const char *nodeset_name)
{
	Cmiss_nodeset_id nodeset = 0;
	if (field_module && nodeset_name)
	{
		Cmiss_field_id field = Cmiss_field_module_find_field_by_name(field_module, nodeset_name);
		if (field)
		{
			Cmiss_field_node_group_id node_group_field = Cmiss_field_cast_node_group(field);
			if (node_group_field)
			{
				nodeset = Cmiss_nodeset_group_base_cast(Cmiss_field_node_group_get_nodeset(node_group_field));
				Cmiss_field_node_group_destroy(&node_group_field);
			}
			Cmiss_field_destroy(&field);
		}
		else
		{
			if (0 == strcmp(nodeset_name, "cmiss_nodes"))
			{
				nodeset = Cmiss_field_module_find_nodeset_by_domain_type(field_module, CMISS_FIELD_DOMAIN_NODES);
			}
			else if (0 == strcmp(nodeset_name, "cmiss_data"))
			{
				nodeset = Cmiss_field_module_find_nodeset_by_domain_type(field_module, CMISS_FIELD_DOMAIN_DATA);
			}
		}
	}
	return nodeset;
}

Cmiss_nodeset_id Cmiss_nodeset_access(Cmiss_nodeset_id nodeset)
{
	if (nodeset)
		return nodeset->access();
	return 0;
}

int Cmiss_nodeset_destroy(Cmiss_nodeset_id *nodeset_address)
{
	if (nodeset_address)
		return Cmiss_nodeset::deaccess(*nodeset_address);
	return 0;
}

int Cmiss_nodeset_contains_node(Cmiss_nodeset_id nodeset, Cmiss_node_id node)
{
	if (nodeset && node)
		return nodeset->containsNode(node);
	return 0;
}

Cmiss_node_template_id Cmiss_nodeset_create_node_template(
	Cmiss_nodeset_id nodeset)
{
	if (nodeset)
		return nodeset->createNodeTemplate();
	return 0;
}

Cmiss_node_id Cmiss_nodeset_create_node(Cmiss_nodeset_id nodeset,
	int identifier, Cmiss_node_template_id node_template)
{
	if (nodeset && node_template)
		return nodeset->createNode(identifier, node_template);
	return 0;
}

Cmiss_node_iterator_id Cmiss_nodeset_create_node_iterator(
	Cmiss_nodeset_id nodeset)
{
	if (nodeset)
		return nodeset->createIterator();
	return 0;
}

Cmiss_node_id Cmiss_nodeset_find_node_by_identifier(Cmiss_nodeset_id nodeset,
	int identifier)
{
	if (nodeset)
		return nodeset->findNodeByIdentifier(identifier);
	return 0;
}

char *Cmiss_nodeset_get_name(Cmiss_nodeset_id nodeset)
{
	if (nodeset)
		return nodeset->getName();
	return 0;
}

int Cmiss_nodeset_get_size(Cmiss_nodeset_id nodeset)
{
	if (nodeset)
		return nodeset->getSize();
	return 0;
}

int Cmiss_nodeset_destroy_all_nodes(Cmiss_nodeset_id nodeset)
{
	if (nodeset)
		nodeset->destroyAllNodes();
	return 0;
}

int Cmiss_nodeset_destroy_node(Cmiss_nodeset_id nodeset, Cmiss_node_id node)
{
	if (nodeset && node)
		nodeset->destroyNode(node);
	return 0;
}

int Cmiss_nodeset_destroy_nodes_conditional(Cmiss_nodeset_id nodeset,
	Cmiss_field_id conditional_field)
{
	if (nodeset && conditional_field)
		return nodeset->destroyNodesConditional(conditional_field);
	return 0;
}

Cmiss_nodeset_id Cmiss_nodeset_get_master(Cmiss_nodeset_id nodeset)
{
	if (nodeset)
		return nodeset->getMaster();
	return 0;
}

int Cmiss_nodeset_match(Cmiss_nodeset_id nodeset1, Cmiss_nodeset_id nodeset2)
{
	return (nodeset1 && nodeset2 && nodeset1->match(*nodeset2));
}

Cmiss_nodeset_group_id Cmiss_nodeset_cast_group(Cmiss_nodeset_id nodeset)
{
	if (nodeset && nodeset->isGroup())
		return static_cast<Cmiss_nodeset_group_id>(nodeset->access());
	return 0;
}

int Cmiss_nodeset_group_destroy(Cmiss_nodeset_group_id *nodeset_group_address)
{
	if (nodeset_group_address)
		return Cmiss_nodeset::deaccess(*(reinterpret_cast<Cmiss_nodeset_id*>(nodeset_group_address)));
	return 0;
}

int Cmiss_nodeset_group_add_node(Cmiss_nodeset_group_id nodeset_group, Cmiss_node_id node)
{
	if (nodeset_group && node)
		return nodeset_group->addNode(node);
	return 0;
}

int Cmiss_nodeset_group_remove_all_nodes(Cmiss_nodeset_group_id nodeset_group)
{
	if (nodeset_group)
		return nodeset_group->removeAllNodes();
	return 0;
}

int Cmiss_nodeset_group_remove_node(Cmiss_nodeset_group_id nodeset_group, Cmiss_node_id node)
{
	if (nodeset_group && node)
		return nodeset_group->removeNode(node);
	return 0;
}

int Cmiss_nodeset_group_remove_nodes_conditional(Cmiss_nodeset_group_id nodeset_group,
	Cmiss_field_id conditional_field)
{
	if (nodeset_group && conditional_field)
		return nodeset_group->removeNodesConditional(conditional_field);
	return 0;
}

int Cmiss_nodeset_group_add_element_nodes(
	Cmiss_nodeset_group_id nodeset_group, Cmiss_element_id element)
{
	if (nodeset_group && element)
		return nodeset_group->addElementNodes(element);
	return 0;
}

int Cmiss_nodeset_group_remove_element_nodes(
	Cmiss_nodeset_group_id nodeset_group, Cmiss_element_id element)
{
	if (nodeset_group && element)
		return nodeset_group->removeElementNodes(element);
	return 0;
}

Cmiss_nodeset_group_id Cmiss_field_node_group_get_nodeset(
	Cmiss_field_node_group_id node_group)
{
	if (node_group)
		return new Cmiss_nodeset_group(node_group);
	return 0;
}

struct LIST(FE_node) *Cmiss_nodeset_create_node_list_internal(Cmiss_nodeset_id nodeset)
{
	if (nodeset)
		return FE_region_create_related_node_list(nodeset->getFeRegion());
	return 0;
}

FE_region *Cmiss_nodeset_get_FE_region_internal(Cmiss_nodeset_id nodeset)
{
	if (nodeset)
		return nodeset->getFeRegion();
	return 0;
}

Cmiss_region_id Cmiss_nodeset_get_region_internal(Cmiss_nodeset_id nodeset)
{
	if (nodeset)
		return FE_region_get_Cmiss_region(nodeset->getFeRegion());
	return 0;
}

Cmiss_region_id Cmiss_nodeset_get_master_region_internal(Cmiss_nodeset_id nodeset)
{
	if (nodeset)
		return FE_region_get_master_Cmiss_region(nodeset->getFeRegion());
	return 0;
}

int Cmiss_nodeset_is_data_internal(Cmiss_nodeset_id nodeset)
{
	if (nodeset)
		return FE_region_is_data_FE_region(nodeset->getFeRegion());
	return 0;
}

Cmiss_nodeset_group_id Cmiss_field_module_create_nodeset_group_from_name_internal(
	Cmiss_field_module_id field_module, const char *nodeset_group_name)
{
	Cmiss_nodeset_group_id nodeset_group = 0;
	if (field_module && nodeset_group_name)
	{
		Cmiss_field_id existing_field = Cmiss_field_module_find_field_by_name(field_module, nodeset_group_name);
		if (existing_field)
		{
			Cmiss_field_destroy(&existing_field);
		}
		else
		{
			char *group_name = duplicate_string(nodeset_group_name);
			char *nodeset_name = strrchr(group_name, '.');
			if (nodeset_name)
			{
				*nodeset_name = '\0';
				++nodeset_name;
				Cmiss_nodeset_id master_nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, nodeset_name);
				Cmiss_field_id field = Cmiss_field_module_find_field_by_name(field_module, group_name);
				Cmiss_field_group_id group = Cmiss_field_cast_group(field);
				Cmiss_field_node_group_id node_group = Cmiss_field_group_create_node_group(group, master_nodeset);
				nodeset_group = Cmiss_field_node_group_get_nodeset(node_group);
				Cmiss_field_node_group_destroy(&node_group);
				Cmiss_field_group_destroy(&group);
				Cmiss_field_destroy(&field);
				Cmiss_nodeset_destroy(&master_nodeset);
			}
			DEALLOCATE(group_name);
		}
	}
	return nodeset_group;
}

Cmiss_node_template_id Cmiss_node_template_access(Cmiss_node_template_id node_template)
{
	if (node_template)
		return node_template->access();
	return 0;
}


int Cmiss_node_template_destroy(Cmiss_node_template_id *node_template_address)
{
	if (node_template_address)
		return Cmiss_node_template::deaccess(*node_template_address);
	return 0;
}

int Cmiss_node_template_define_field(Cmiss_node_template_id node_template,
	Cmiss_field_id field)
{
	if (node_template && field)
	{
		return node_template->defineField(field);
	}
	return 0;
}

int Cmiss_node_template_define_field_from_node(
	Cmiss_node_template_id node_template, Cmiss_field_id field,
	Cmiss_node_id node)
{
	if (node_template && field && node)
	{
		return node_template->defineFieldFromNode(field, node);
	}
	return 0;
}

int Cmiss_node_template_define_derivative(Cmiss_node_template_id node_template,
	Cmiss_field_id field, int component_number,
	enum Cmiss_nodal_value_type derivative_type)
{
	if (node_template && field)
	{
		return node_template->defineDerivative(field, component_number, derivative_type);
	}
	return 0;
}

int Cmiss_node_template_define_time_sequence(
	Cmiss_node_template_id node_template, Cmiss_field_id field,
	Cmiss_time_sequence_id time_sequence)
{
	if (node_template && field && time_sequence)
	{
		return node_template->defineTimeSequence(field, time_sequence);
	}
	return 0;
}

int Cmiss_node_template_define_versions(Cmiss_node_template_id node_template,
	Cmiss_field_id field, int component_number,
	int number_of_versions)
{
	if (node_template && field)
	{
		return node_template->defineVersions(field, component_number, number_of_versions);
	}
	return 0;
}

int Cmiss_node_template_get_number_of_versions(Cmiss_node_template_id node_template,
	Cmiss_field_id field, int component_number)
{
	if (node_template && field)
	{
		return node_template->getNumberOfVersions(field, component_number);
	}
	return 0;
}

Cmiss_time_sequence_id Cmiss_node_template_get_time_sequence(
	Cmiss_node_template_id node_template, Cmiss_field_id field)
{
	if (node_template && field)
	{
		return node_template->getTimeSequence(field);
	}
	return 0;
}

int Cmiss_node_template_has_derivative(Cmiss_node_template_id node_template,
	Cmiss_field_id field, int component_number,
	enum Cmiss_nodal_value_type derivative_type)
{
	if (node_template && field)
	{
		return node_template->hasDerivative(field, component_number, derivative_type);
	}
	return 0;
}

int Cmiss_node_template_undefine_field(Cmiss_node_template_id node_template,
	Cmiss_field_id field)
{
	if (node_template && field)
	{
		return node_template->undefineField(field);
	}
	return 0;
}

Cmiss_node_id Cmiss_node_access(Cmiss_node_id node)
{
	if (node)
		return ACCESS(FE_node)(node);
	return 0;
}

int Cmiss_node_destroy(Cmiss_node_id *node_address)
{
	return DEACCESS(FE_node)(node_address);
}

int Cmiss_node_get_identifier(Cmiss_node_id node)
{
	return get_FE_node_identifier(node);
}

int Cmiss_node_merge(Cmiss_node_id node, Cmiss_node_template_id node_template)
{
	if (node && node_template)
		return node_template->mergeIntoNode(node);
	return 0;
}

class Cmiss_nodal_value_type_conversion
{
public:
	static const char *to_string(enum Cmiss_nodal_value_type type)
	{
		const char *enum_string = 0;
		switch (type)
		{
			case CMISS_NODAL_VALUE:
				enum_string = "VALUE";
				break;
			case CMISS_NODAL_D_DS1:
				enum_string = "D_DS1";
				break;
			case CMISS_NODAL_D_DS2:
				enum_string = "D_DS2";
				break;
			case CMISS_NODAL_D_DS3:
				enum_string = "D_DS3";
				break;
			case CMISS_NODAL_D2_DS1DS2:
				enum_string = "D2_DS1DS2";
				break;
			case CMISS_NODAL_D2_DS1DS3:
				enum_string = "_D2_DS1DS3";
				break;
			case CMISS_NODAL_D2_DS2DS3:
				enum_string = "D2_DS2DS3";
				break;
			case CMISS_NODAL_D3_DS1DS2DS3:
				enum_string = "D3_DS1DS2DS3";
				break;
			default:
				break;
		}
		return enum_string;
	}
};

enum Cmiss_nodal_value_type Cmiss_nodal_value_type_enum_from_string(
	const char *string)
{
	return string_to_enum<enum Cmiss_nodal_value_type,	Cmiss_nodal_value_type_conversion>(string);
}

char *Cmiss_nodal_value_type_enum_to_string(enum Cmiss_nodal_value_type type)
{
	const char *type_string = Cmiss_nodal_value_type_conversion::to_string(type);
	return (type_string ? duplicate_string(type_string) : 0);
}
