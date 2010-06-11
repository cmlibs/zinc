extern "C" {
#include <stdlib.h>
#include "api/cmiss_element.h"
#include "api/cmiss_node.h"
#include "api/cmiss_field_module.h"
#include "api/cmiss_field_sub_group_template.h"
#include "computed_field/computed_field.h"
}
#include "computed_field/computed_field_sub_group_template.hpp"
#include "computed_field/computed_field_private.hpp"
extern "C" {
#include "finite_element/finite_element_to_graphics_object.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "region/cmiss_region.h"
#include "user_interface/message.h"
}


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


Cmiss_field_node_group_template *Cmiss_field_cast_node_group_template(Cmiss_field_id field)
{
	if (dynamic_cast<Computed_field_sub_group_object<Cmiss_node_id>*>(field->core))
	{
		Cmiss_field_access(field);
		return (reinterpret_cast<Cmiss_field_node_group_template_id>(field));
	}
	else
	{
		return (NULL);
	}
}

inline Computed_field *Computed_field_cast(
	Cmiss_field_node_group_template *node_group_field)
{
	return (reinterpret_cast<Computed_field*>(node_group_field));
}

int Cmiss_field_node_group_template_add_node(Cmiss_field_node_group_template_id node_group,
		Cmiss_node_id node)
{
	int return_code = 1;

	if (node_group && node)
	{
	  int identifier = get_FE_node_identifier(node);
		Computed_field_sub_group_object<Cmiss_node_id> *group_core =
			Computed_field_sub_group_object_core_cast<Cmiss_node_id,
			Cmiss_field_node_group_template_id>(node_group);
		group_core->add_object(identifier, node);
	}
	else
	{
		return_code = 0;
	}

	return return_code;
}

int Cmiss_field_node_group_template_clear(Cmiss_field_node_group_template_id node_group)
{
	int return_code = 1;

	if (node_group)
	{
		Computed_field_sub_group_object<Cmiss_node_id> *group_core =
			Computed_field_sub_group_object_core_cast<Cmiss_node_id,
			Cmiss_field_node_group_template_id>(node_group);
		group_core->clear();
	}
	else
	{
		return_code = 0;
	}

	return return_code;
}

int Cmiss_field_node_group_template_is_node_selected(
	Cmiss_field_node_group_template_id node_group,	Cmiss_node_id node)
{
	int return_code = 0;

	if (node_group && node)
	{
    int identifier = get_FE_node_identifier(node);
		Computed_field_sub_group_object<Cmiss_node_id> *group_core =
			Computed_field_sub_group_object_core_cast<Cmiss_node_id, 
			Cmiss_field_node_group_template_id>(node_group);
		return_code = group_core->get_object_selected(identifier, node);
	}

	return return_code;
}

Computed_field *Cmiss_field_module_create_node_group_template(Cmiss_field_module_id field_module)
{
	Computed_field *field;

	ENTER(Computed_field_create_group);
	field = (Computed_field *)NULL;
	if (field_module)
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/false, 1,
			/*number_of_source_fields*/0, NULL,
			/*number_of_source_values*/0, NULL,
			new Computed_field_sub_group_object<Cmiss_node_id>());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_group.  Invalid argument(s)");
	}
	LEAVE;

	return (field);
} /* Cmiss_field_module_create_group */

Cmiss_field_element_group_template *Cmiss_field_cast_element_group_template(Cmiss_field_id field)
{
	if (dynamic_cast<Computed_field_sub_group_object<Cmiss_element_id>*>(field->core))
	{
		Cmiss_field_access(field);
		return (reinterpret_cast<Cmiss_field_element_group_template_id>(field));
	}
	else
	{
		return (NULL);
	}
}

inline Computed_field *Computed_field_cast(
	Cmiss_field_element_group_template *element_group_field)
{
	return (reinterpret_cast<Computed_field*>(element_group_field));
}

int Cmiss_field_element_group_template_add_element(Cmiss_field_element_group_template_id element_group,
		Cmiss_element_id element)
{
	int return_code = 1;
        struct CM_element_information cm_identifier;
	if (element_group && element)
	{
	  if (get_FE_element_identifier(element, &cm_identifier))
    {
	  	Computed_field_sub_group_object<Cmiss_element_id> *group_core =
	  		Computed_field_sub_group_object_core_cast<Cmiss_element_id,
	  		Cmiss_field_element_group_template_id>(element_group);
	  	int identifier = CM_element_information_to_graphics_name(&cm_identifier);
	  	group_core->add_object(identifier, element);
    }
	}
	else
	{
		return_code = 0;
	}

	return return_code;
}

int Cmiss_field_element_group_template_clear(Cmiss_field_element_group_template_id element_group)
{
	int return_code = 1;

	if (element_group)
	{
		Computed_field_sub_group_object<Cmiss_element_id> *group_core =
			Computed_field_sub_group_object_core_cast<Cmiss_element_id,
			Cmiss_field_element_group_template_id>(element_group);
		group_core->clear();
	}
	else
	{
		return_code = 0;
	}

	return return_code;
}

int Cmiss_field_element_group_template_is_element_selected(
	Cmiss_field_element_group_template_id element_group,	Cmiss_element_id element)
{
	int return_code = 0;
	struct CM_element_information cm_identifier;
	if (element_group && element)
	{
		if (get_FE_element_identifier(element, &cm_identifier))
    {
			Computed_field_sub_group_object<Cmiss_element_id> *group_core =
				Computed_field_sub_group_object_core_cast<Cmiss_element_id,
				Cmiss_field_element_group_template_id>(element_group);
			int identifier = CM_element_information_to_graphics_name(&cm_identifier);
			return_code = group_core->get_object_selected(identifier, element);
    }
	}

	return return_code;
}

Computed_field *Cmiss_field_module_create_element_group_template(Cmiss_field_module_id field_module)
{
	Computed_field *field;

	ENTER(Computed_field_create_group);
	field = (Computed_field *)NULL;
	if (field_module)
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/false, 1,
			/*number_of_source_fields*/0, NULL,
			/*number_of_source_values*/0, NULL,
			new Computed_field_sub_group_object<Cmiss_element_id>());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_group.  Invalid argument(s)");
	}
	LEAVE;

	return (field);
} /* Cmiss_field_module_create_group */
