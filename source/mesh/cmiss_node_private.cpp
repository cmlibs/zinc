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

extern "C" {
#include <stdarg.h>
#include "api/cmiss_node.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "computed_field/computed_field_finite_element.h"
#include "node/node_operations.h"
#include "user_interface/message.h"
}
#include "computed_field/field_module.hpp"
#include "general/enumerator_conversion.hpp"
#include "mesh/cmiss_node_private.hpp"
#include <vector>
#include "computed_field/computed_field_private.hpp"

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
	int access_count;

public:
	Cmiss_node_template(FE_region *fe_region) :
		fe_region(ACCESS(FE_region)(fe_region)),
		template_node(NULL),
		access_count(1)
	{
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
		Cmiss_field_finite_element_id finite_element_field = Cmiss_field_cast_finite_element(field);
		Cmiss_field_stored_mesh_location_id stored_mesh_location_field = Cmiss_field_cast_stored_mesh_location(field);
		Cmiss_field_stored_string_id stored_string_field = Cmiss_field_cast_stored_string(field);
		if (!(finite_element_field || stored_mesh_location_field || stored_string_field))
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_node_template_define_field.  "
				"Field must be finite_element, stored_mesh_location or stored_string type");
			return 0;
		}
		int return_code = 1;
		FE_field *fe_field = NULL;
		Computed_field_get_type_finite_element(field, &fe_field);
		if (FE_field_get_FE_region(fe_field) != fe_region)
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_node_template_define_field.  "
				"Field is from another region");
			return_code = 0;
		}
		if (getNodeField(fe_field))
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_node_template_define_field.  Field is already defined");
			return_code = 0;
		}
		Cmiss_field_finite_element_destroy(&finite_element_field);
		Cmiss_field_stored_mesh_location_destroy(&stored_mesh_location_field);
		Cmiss_field_stored_string_destroy(&stored_string_field);
		if (!return_code)
			return 0;
		clearTemplateNode();
		Cmiss_node_field *node_field = createNodeField(fe_field);
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
		enum FE_nodal_value_type fe_nodal_value_type = FE_NODAL_UNKNOWN;
		switch (derivative_type)
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
		if (FE_NODAL_UNKNOWN == fe_nodal_value_type)
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_node_template_define_derivative.  Invalid derivative type");
			return 0;
		}
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

	int finalise()
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
				"Cmiss_node_template_finalise.  Failed to create template node");
			return 0;
		}
		return 1;
	}

	int isFinalised() const { return (NULL != template_node); }

	int mergeIntoNode(Cmiss_node_id node)
	{
		int return_code = 0;
		if (isFinalised())
		{
			return_code = FE_region_merge_FE_node_existing(fe_region, node, template_node);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_node_merge.  Node template is not finalised");
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

	/** Must call getNodeField first to confirm not already defined */
	Cmiss_node_field *createNodeField(FE_field *fe_field)
	{
		Cmiss_node_field *node_field = new Cmiss_node_field(fe_field);
		fields.push_back(node_field);
		return node_field;
	}

	int hasFields() { return fields.size(); }

	void clearTemplateNode()
	{
		REACCESS(FE_node)(&template_node, NULL);
	}
};

/*
Global functions
----------------
*/

Cmiss_nodeset_id Cmiss_field_module_find_nodeset_by_name(
	Cmiss_field_module_id field_module, const char *nodeset_name)
{
	FE_region *fe_region = NULL;
	if (field_module && nodeset_name)
	{
		Cmiss_region_id region = Cmiss_field_module_get_region_internal(field_module);
		const char *dotpos = strrchr(nodeset_name, '.');
		const char *trimmed_nodeset_name = 0;
		if (dotpos)
		{
			Cmiss_region_id master_region = Cmiss_field_module_get_master_region_internal(field_module);
			char *group_name = duplicate_string(nodeset_name);
			group_name[dotpos - nodeset_name] = '\0';
			Cmiss_region_id child = Cmiss_region_find_child_by_name(master_region, group_name);
			if (child)
			{
				if (Cmiss_region_is_group(child))
				{
					region = child;
					trimmed_nodeset_name = dotpos + 1;
				}
				Cmiss_region_destroy(&child);
			}
			DEALLOCATE(group_name);
		}
		else
		{
			trimmed_nodeset_name = nodeset_name;
		}
		if (trimmed_nodeset_name)
		{
			fe_region = Cmiss_region_get_FE_region(region);
			if (0 == strcmp(trimmed_nodeset_name, "cmiss_nodes"))
			{
				ACCESS(FE_region)(fe_region);
			}
			else if (0 == strcmp(trimmed_nodeset_name, "cmiss_data"))
			{
				fe_region = ACCESS(FE_region)(FE_region_get_data_FE_region(fe_region));
			}
		}
	}
	return reinterpret_cast<Cmiss_nodeset_id>(fe_region);
}

Cmiss_nodeset_id Cmiss_nodeset_access(Cmiss_nodeset_id nodeset)
{
	ACCESS(FE_region)(reinterpret_cast<FE_region *>(nodeset));
	return nodeset;
}

int Cmiss_nodeset_destroy(Cmiss_nodeset_id *nodeset_address)
{
	return DEACCESS(FE_region)(reinterpret_cast<FE_region **>(nodeset_address));
}

int Cmiss_nodeset_contains_node(Cmiss_nodeset_id nodeset, Cmiss_node_id node)
{
	return FE_region_contains_FE_node(reinterpret_cast<FE_region *>(nodeset), node);
}

Cmiss_node_template_id Cmiss_nodeset_create_node_template(
	Cmiss_nodeset_id nodeset)
{
	if (nodeset)
	{
		FE_region *fe_region = reinterpret_cast<FE_region *>(nodeset);
		FE_region_get_ultimate_master_FE_region(fe_region, &fe_region);
		return new Cmiss_node_template(fe_region);
	}
	return NULL;
}

Cmiss_node_id Cmiss_nodeset_create_node(Cmiss_nodeset_id nodeset,
	int identifier, Cmiss_node_template_id node_template)
{
	if (nodeset && node_template)
	{
		FE_node *node = NULL;
		if (node_template->isFinalised())
		{
			FE_region *fe_region = reinterpret_cast<FE_region *>(nodeset);
			FE_node *template_node = node_template->getTemplateNode();
			node = ACCESS(FE_node)(FE_region_create_FE_node_copy(
				fe_region, identifier, template_node));
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_nodeset_create_node.  Node template is not finalised");
		}
		return node;
	}
	return NULL;
}

Cmiss_node_iterator_id Cmiss_nodeset_create_node_iterator(
	Cmiss_nodeset_id nodeset)
{
	if (nodeset)
	{
		FE_region *fe_region = reinterpret_cast<FE_region *>(nodeset);
		return FE_region_create_node_iterator(fe_region);
	}
	return NULL;
}

Cmiss_node_id Cmiss_nodeset_find_node_by_identifier(Cmiss_nodeset_id nodeset,
	int identifier)
{
	Cmiss_node_id node = FE_region_get_FE_node_from_identifier(
		reinterpret_cast<FE_region *>(nodeset), identifier);
	if (node)
		ACCESS(FE_node)(node);
	return node;
}

char *Cmiss_nodeset_get_name(Cmiss_nodeset_id nodeset)
{
	char *name = 0;
	if (nodeset)
	{
		FE_region *fe_region = reinterpret_cast<FE_region *>(nodeset);
		int error = 0;
		Cmiss_region_id region = FE_region_get_Cmiss_region(fe_region);
		if (Cmiss_region_is_group(region))
		{
			char *group_name = Cmiss_region_get_name(region);
			append_string(&name, group_name, &error);
			append_string(&name, ".", &error);
			DEALLOCATE(group_name);
		}
		if (FE_region_is_data_FE_region(fe_region))
		{
			append_string(&name, "cmiss_data", &error);
		}
		else
		{
			append_string(&name, "cmiss_nodes", &error);
		}
	}
	return name;
}

int Cmiss_nodeset_get_size(Cmiss_nodeset_id nodeset)
{
	if (nodeset)
	{
		FE_region *fe_region = reinterpret_cast<FE_region *>(nodeset);
		return FE_region_get_number_of_FE_nodes(fe_region);
	}
	return 0;
}

int Cmiss_nodeset_destroy_node(Cmiss_nodeset_id nodeset, Cmiss_node_id node)
{
	int return_code = 0;
	if (nodeset && node)
	{
		FE_region *fe_region = reinterpret_cast<FE_region *>(nodeset);
		FE_region *master_fe_region = fe_region;
		FE_region_get_ultimate_master_FE_region(fe_region, &master_fe_region);
		return_code = FE_region_remove_FE_node(master_fe_region, node);
	}

	return return_code;
}

int Cmiss_nodeset_destroy_nodes_conditional(Cmiss_nodeset_id nodeset,
	Cmiss_field_id conditional_field)
{
	int return_code = 0;
	if (nodeset && conditional_field)
	{
		FE_region *fe_region = reinterpret_cast<FE_region *>(nodeset);
		struct LIST(FE_node) *node_list =	FE_node_list_from_conditional_field(
			fe_region, conditional_field, 0);
		FE_region *master_fe_region = fe_region;
		FE_region_get_ultimate_master_FE_region(fe_region, &master_fe_region);
		return_code = FE_region_remove_FE_node_list(master_fe_region, node_list);
		DESTROY(LIST(FE_node))(&node_list);
	}

	return return_code;
}

Cmiss_nodeset_id Cmiss_nodeset_get_master(Cmiss_nodeset_id nodeset)
{
	FE_region *fe_region = Cmiss_nodeset_get_master_FE_region_internal(nodeset);
	return reinterpret_cast<Cmiss_nodeset_id>(ACCESS(FE_region)(fe_region));
}

int Cmiss_nodeset_match(Cmiss_nodeset_id nodeset1, Cmiss_nodeset_id nodeset2)
{
	return (nodeset1 && (nodeset1 == nodeset2));
}

FE_region *Cmiss_nodeset_get_FE_region_internal(Cmiss_nodeset_id nodeset)
{
	return reinterpret_cast<FE_region *>(nodeset);
}

FE_region *Cmiss_nodeset_get_master_FE_region_internal(Cmiss_nodeset_id nodeset)
{
	FE_region *fe_region = reinterpret_cast<FE_region *>(nodeset);
	FE_region_get_ultimate_master_FE_region(fe_region, &fe_region);
	return fe_region;
}

Cmiss_region_id Cmiss_nodeset_get_region_internal(Cmiss_nodeset_id nodeset)
{
	if (!nodeset)
		return 0;
	return FE_region_get_Cmiss_region(reinterpret_cast<FE_region *>(nodeset));
}

Cmiss_region_id Cmiss_nodeset_get_master_region_internal(Cmiss_nodeset_id nodeset)
{
	if (!nodeset)
		return 0;
	return FE_region_get_master_Cmiss_region(reinterpret_cast<FE_region *>(nodeset));
}

int Cmiss_nodeset_is_data_internal(Cmiss_nodeset_id nodeset)
{
	return FE_region_is_data_FE_region(reinterpret_cast<FE_region *>(nodeset));
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

int Cmiss_node_template_finalise(Cmiss_node_template_id node_template)
{
	if (node_template)
		return node_template->finalise();
	return 0;
}

Cmiss_node_id Cmiss_node_access(Cmiss_node_id node)
{
	return ACCESS(FE_node)(node);
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
