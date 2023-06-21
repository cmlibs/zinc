/**
 * FILE : nodetemplate.cpp
 *
 * Implementation of elementtemplate.
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "cmlibs/zinc/fieldfiniteelement.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_finite_element.h"
#include "finite_element/finite_element_private.h"
#include "node/nodetemplate.hpp"


class cmzn_node_field
{
	FE_field *fe_field;
	std::vector<FE_node_field_template> componentNodefieldtemplates;
	FE_time_sequence *timesequence;

public:

	cmzn_node_field(FE_field *fe_field_in) :
		fe_field(fe_field_in->access()),
		componentNodefieldtemplates(fe_field_in->getNumberOfComponents()),
		timesequence(NULL)
	{
	}

	~cmzn_node_field()
	{
		if (timesequence)
			DEACCESS(FE_time_sequence)(&timesequence);
		FE_field::deaccess(fe_field);
	}

	/** Set up node field with a copy of node field in use in node.
	  * @return  Result OK on success, any other value on failure. Client should clean up on failure. */
	int cloneNodeField(cmzn_node *node)
	{
		const FE_node_field *node_field = node->getNodeField(this->fe_field);
		if (!node_field)
		{
			return CMZN_ERROR_NOT_FOUND;
		}
		const int componentCount = get_FE_field_number_of_components(this->fe_field);
		for (int c = 0; c < componentCount; ++c)
		{
			this->componentNodefieldtemplates[c] = *(node_field->getComponent(c));
		}
		if (node_field->getTimeSequence())
		{
			this->setTimesequence(node_field->getTimeSequence());
		}
		return CMZN_OK;
	}

	/** note: does not ACCESS */
	cmzn_timesequence* getTimesequence()
	{
		return reinterpret_cast<cmzn_timesequence*>(timesequence);
	}

	int setTimesequence(FE_time_sequence *in_timesequence)
	{
		return REACCESS(FE_time_sequence)(&timesequence, in_timesequence);
	}

	int defineAtNode(FE_node *node)
	{
		return define_FE_field_at_node(node, this->fe_field, this->componentNodefieldtemplates.data(),
			this->timesequence);
	}

	int getValueNumberOfVersions(int componentNumber, cmzn_node_value_label valueLabel)
	{
		const int componentCount = get_FE_field_number_of_components(this->fe_field);
		if ((valueLabel < CMZN_NODE_VALUE_LABEL_VALUE)
			|| (valueLabel > CMZN_NODE_VALUE_LABEL_D3_DS1DS2DS3)
			|| (componentNumber < -1)
			|| (componentNumber == 0)
			|| (componentNumber > componentCount))
		{
			display_message(ERROR_MESSAGE, "Nodetemplate getValueNumberOfVersions.  Invalid arguments");
			return -1;
		}
		if (componentNumber == -1)
		{
			const int versionsCount = this->componentNodefieldtemplates[0].getValueNumberOfVersions(valueLabel);
			for (int c = 1; c < componentCount; ++c)
			{
				if (this->componentNodefieldtemplates[c].getValueNumberOfVersions(valueLabel) != versionsCount)
				{
					return -1; // not consistent over components
				}
			}
			return versionsCount;
		}
		return this->componentNodefieldtemplates[componentNumber - 1].getValueNumberOfVersions(valueLabel);
	}

	/** @param componentNumber  From 1 to number of components or -1 to set for all
	  * @param numberOfVersions  Number of versions > 1, or 0 to undefine. */
	int setValueNumberOfVersions(int componentNumber, cmzn_node_value_label valueLabel, int numberOfVersions)
	{
		const int componentCount = get_FE_field_number_of_components(this->fe_field);
		if ((valueLabel < CMZN_NODE_VALUE_LABEL_VALUE)
			|| (valueLabel > CMZN_NODE_VALUE_LABEL_D3_DS1DS2DS3)
			|| (componentNumber < -1)
			|| (componentNumber == 0)
			|| (componentNumber > componentCount)
			|| (numberOfVersions < 0))
		{
			display_message(ERROR_MESSAGE, "Nodetemplate setValueNumberOfVersions.  Invalid arguments");
			return CMZN_ERROR_ARGUMENT;
		}
		int first = 0;
		int limit = get_FE_field_number_of_components(fe_field);
		if (componentNumber > 0)
		{
			first = componentNumber - 1;
			limit = componentNumber;
		}
		for (int c = first; c < limit; ++c)
		{
			const int result = this->componentNodefieldtemplates[c].setValueNumberOfVersions(valueLabel, numberOfVersions);
			if (result != CMZN_OK)
			{
				if (((numberOfVersions == 0) && (result != CMZN_ERROR_NOT_FOUND))
					|| ((numberOfVersions > 0) && (result != CMZN_ERROR_ALREADY_EXISTS)))
				{
					display_message(ERROR_MESSAGE, "Nodetemplate setValueNumberOfVersions.  Failed");
					return result;
				}
			}
		}
		return CMZN_OK;
	}		

	FE_field *getFeField() const { return fe_field; }
};


cmzn_nodetemplate::cmzn_nodetemplate(FE_nodeset* feNodesetIn) :
	fe_nodeset(cmzn::Access(feNodesetIn)),
	fe_node_template(0),
	access_count(1)
{
}

cmzn_nodetemplate::~cmzn_nodetemplate()
{
	for (unsigned int i = 0; i < fields.size(); i++)
	{
		delete fields[i];
	}
	for (unsigned int i = 0; i < undefine_fields.size(); i++)
	{
		FE_field::deaccess(undefine_fields[i]);
	}
	cmzn::Deaccess(this->fe_node_template);
	cmzn::Deaccess(this->fe_nodeset);
}

cmzn_node_field* cmzn_nodetemplate::getNodeField(FE_field* fe_field) const
{
	for (unsigned int i = 0; i < this->fields.size(); ++i)
		if (this->fields[i]->getFeField() == fe_field)
			return this->fields[i];
	return nullptr;
}

bool cmzn_nodetemplate::removeDefineField(FE_field* fe_field)
{
	for (std::vector<cmzn_node_field*>::iterator iter = this->fields.begin(); iter != this->fields.end(); ++iter)
		if ((*iter)->getFeField() == fe_field)
		{
			delete* iter;
			this->fields.erase(iter);
			return true;
		}
	return false;
}

cmzn_node_field* cmzn_nodetemplate::createNodeField(FE_field* fe_field)
{
	cmzn_node_field* node_field = new cmzn_node_field(fe_field);
	// replace existing field in fields list
	for (unsigned int i = 0; i < fields.size(); ++i)
		if (this->fields[i]->getFeField() == fe_field)
		{
			delete this->fields[i];
			fields[i] = node_field;
			return node_field;
		}
	this->removeUndefineField(fe_field);
	this->fields.push_back(node_field);
	return node_field;
}

bool cmzn_nodetemplate::isUndefineNodeField(FE_field* fe_field)
{
	for (unsigned int i = 0; i < undefine_fields.size(); ++i)
		if (undefine_fields[i] == fe_field)
			return true;
	return false;
}

bool cmzn_nodetemplate::removeUndefineField(FE_field* fe_field)
{
	for (std::vector<FE_field*>::iterator iter = this->undefine_fields.begin(); iter != this->undefine_fields.end(); ++iter)
		if ((*iter) == fe_field)
		{
			FE_field::deaccess(fe_field);
			this->undefine_fields.erase(iter);
			return true;
		}
	return false;
}

void cmzn_nodetemplate::setUndefineNodeField(FE_field* fe_field)
{
	// if already in undefine list, nothing to do
	for (unsigned int i = 0; i < this->undefine_fields.size(); ++i)
		if (this->undefine_fields[i] == fe_field)
			return;
	this->removeDefineField(fe_field);
	fe_field->access();
	this->undefine_fields.push_back(fe_field);
}

void cmzn_nodetemplate::invalidate()
{
	cmzn::Deaccess(this->fe_node_template);
}

int cmzn_nodetemplate::checkValidFieldForDefine(cmzn_field* field) const
{
	FE_field* fe_field = 0;
	Computed_field_get_type_finite_element(field, &fe_field);
	if (!fe_field)
		return CMZN_ERROR_ARGUMENT;
	if (fe_field->get_FE_region() != this->fe_nodeset->get_FE_region())
		return CMZN_ERROR_INCOMPATIBLE_DATA;
	return CMZN_OK;
}


int cmzn_nodetemplate::defineField(cmzn_field* field)
{
	int result = this->checkValidFieldForDefine(field);
	if (result != CMZN_OK)
	{
		display_message(ERROR_MESSAGE, "Nodetemplate defineField.  Invalid field");
		return result;
	}
	this->invalidate();
	FE_field* fe_field = 0;
	Computed_field_get_type_finite_element(field, &fe_field);
	cmzn_node_field* node_field = this->createNodeField(fe_field);
	if (!node_field)
	{
		this->removeDefineField(fe_field);
		return CMZN_ERROR_GENERAL;
	}
	// External behaviour defaults to having VALUE defined, which must removed if not wanted
	result = node_field->setValueNumberOfVersions(-1, CMZN_NODE_VALUE_LABEL_VALUE, 1);
	if (result != CMZN_OK)
	{
		return result;
	}
	return CMZN_OK;
}

int cmzn_nodetemplate::defineFieldFromNode(cmzn_field* field, cmzn_node* node)
{
	if (!((node) && this->fe_nodeset->containsNode(node)))
	{
		display_message(ERROR_MESSAGE, "Nodetemplate defineFieldFromNode.  Invalid node");
		return CMZN_ERROR_ARGUMENT;
	}
	int result = this->checkValidFieldForDefine(field);
	if (result != CMZN_OK)
	{
		display_message(ERROR_MESSAGE, "Nodetemplate defineFieldFromNode.  Invalid field");
		return result;
	}
	this->invalidate();
	FE_field* fe_field = 0;
	Computed_field_get_type_finite_element(field, &fe_field);
	if (!(node->getNodeField(fe_field)))
	{
		this->removeDefineField(fe_field);
		this->removeUndefineField(fe_field);
		return CMZN_ERROR_NOT_FOUND;
	}
	cmzn_node_field* node_field = this->createNodeField(fe_field);
	if (!node_field)
	{
		return CMZN_ERROR_GENERAL;
	}
	result = node_field->cloneNodeField(node);
	if (result != CMZN_OK)
	{
		this->removeDefineField(fe_field);
		return result;
	}
	return CMZN_OK;
}

cmzn_timesequence* cmzn_nodetemplate::getTimesequence(cmzn_field* field)
{
	// only numerical valued fields can have a time sequence, currently all reported as real value type on the public API
	if (CMZN_FIELD_VALUE_TYPE_REAL != field->getValueType())
	{
		return nullptr;
	}
	FE_field* feField = nullptr;
	if (!Computed_field_get_type_finite_element(field, &feField))
	{
		return nullptr;
	}
	cmzn_node_field* nodeField = getNodeField(feField);
	if (!nodeField)
	{
		return nullptr;
	}
	return nodeField->getTimesequence();
}

int cmzn_nodetemplate::setTimesequence(cmzn_field* field, cmzn_timesequence* timesequence)
{
	cmzn_field_finite_element* finite_element_field = cmzn_field_cast_finite_element(field);
	if (!finite_element_field)
	{
		display_message(ERROR_MESSAGE,
			"cmzn_nodetemplate_set_timesequence.  Field must be real finite_element type");
		return CMZN_ERROR_ARGUMENT;
	}
	cmzn_field_finite_element_destroy(&finite_element_field);
	FE_field* fe_field = NULL;
	Computed_field_get_type_finite_element(field, &fe_field);
	cmzn_node_field* node_field = getNodeField(fe_field);
	if (!node_field)
		return CMZN_ERROR_NOT_FOUND;
	this->invalidate();
	return node_field->setTimesequence(reinterpret_cast<struct FE_time_sequence*>(timesequence));
}

int cmzn_nodetemplate::getValueNumberOfVersions(cmzn_field* field, int componentNumber,
	enum cmzn_node_value_label nodeValueLabel)
{
	cmzn_field_finite_element* finite_element_field = cmzn_field_cast_finite_element(field);
	if (!finite_element_field)
	{
		return -1;
	}
	cmzn_field_finite_element_destroy(&finite_element_field);
	FE_field* fe_field = NULL;
	Computed_field_get_type_finite_element(field, &fe_field);
	cmzn_node_field* node_field = this->getNodeField(fe_field);
	if (!node_field)
	{
		return -1;
	}
	return node_field->getValueNumberOfVersions(componentNumber, nodeValueLabel);
}

int cmzn_nodetemplate::setValueNumberOfVersions(cmzn_field* field, int componentNumber,
	cmzn_node_value_label nodeValueLabel, int numberOfVersions)
{
	cmzn_field_finite_element* finite_element_field = cmzn_field_cast_finite_element(field);
	if (!finite_element_field)
	{
		display_message(ERROR_MESSAGE,
			"cmzn_nodetemplate_set_value_number_of_versions.  Field must be real finite element type");
		return CMZN_ERROR_ARGUMENT;
	}
	cmzn_field_finite_element_destroy(&finite_element_field);
	FE_field* fe_field = NULL;
	Computed_field_get_type_finite_element(field, &fe_field);
	cmzn_node_field* node_field = this->getNodeField(fe_field);
	if (!node_field)
		return CMZN_ERROR_NOT_FOUND;
	return node_field->setValueNumberOfVersions(componentNumber, nodeValueLabel, numberOfVersions);
}

int cmzn_nodetemplate::removeField(cmzn_field* field)
{
	int result = this->checkValidFieldForDefine(field);
	if (result != CMZN_OK)
		return result;
	FE_field* fe_field = NULL;
	Computed_field_get_type_finite_element(field, &fe_field);
	this->invalidate();
	if (this->removeDefineField(fe_field) || this->removeUndefineField(fe_field))
		return CMZN_OK;
	return CMZN_ERROR_NOT_FOUND;
}

int cmzn_nodetemplate::undefineField(cmzn_field* field)
{
	int result = this->checkValidFieldForDefine(field);
	if (result != CMZN_OK)
		return result;
	FE_field* fe_field = NULL;
	Computed_field_get_type_finite_element(field, &fe_field);
	this->invalidate();
	setUndefineNodeField(fe_field);
	return CMZN_OK;
}

int cmzn_nodetemplate::validate()
{
	if (this->fe_node_template)
		return 1;
	this->fe_node_template = this->fe_nodeset->create_FE_node_template();
	for (unsigned int i = 0; i < fields.size(); i++)
	{
		if (!fields[i]->defineAtNode(this->fe_node_template->get_template_node()))
		{
			cmzn::Deaccess(this->fe_node_template);
			break;
		}
	}
	if (!this->fe_node_template)
	{
		display_message(ERROR_MESSAGE,
			"cmzn_nodetemplate_validate.  Failed to create fe_node_template");
		return 0;
	}
	return 1;
}

// can be made more efficient
int cmzn_nodetemplate::mergeIntoNode(cmzn_node* node)
{
	FE_nodeset* target_fe_nodeset = FE_node_get_FE_nodeset(node);
	if (target_fe_nodeset == this->fe_nodeset)
	{
		if (this->validate())
		{
			int return_code = CMZN_OK;
			if (0 < undefine_fields.size())
			{
				FE_region_begin_change(this->fe_nodeset->get_FE_region());
				for (unsigned int i = 0; i < undefine_fields.size(); i++)
				{
					int result = this->fe_nodeset->undefineFieldAtNode(node, undefine_fields[i]);
					if ((result != CMZN_OK) && (result != CMZN_ERROR_NOT_FOUND))
					{
						return_code = result;
						break;
					}
				}
			}
			if ((return_code == CMZN_OK) && (0 < fields.size()))
				return_code = this->fe_nodeset->merge_FE_node_template(node, this->fe_node_template);
			if (0 < undefine_fields.size())
				FE_region_end_change(this->fe_nodeset->get_FE_region());
			return return_code;
		}
		else
			display_message(ERROR_MESSAGE, "cmzn_node_merge.  Node template is not valid");
	}
	else
		display_message(ERROR_MESSAGE, "cmzn_node_merge.  Incompatible template");
	return CMZN_ERROR_ARGUMENT;
}

/*
Global functions
----------------
*/

cmzn_nodetemplate_id cmzn_nodetemplate_access(cmzn_nodetemplate_id node_template)
{
	if (node_template)
		return node_template->access();
	return 0;
}

int cmzn_nodetemplate_destroy(cmzn_nodetemplate_id* nodetemplate_address)
{
	if (nodetemplate_address)
	{
		cmzn_nodetemplate::deaccess(*nodetemplate_address);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_nodetemplate_define_field(cmzn_nodetemplate_id node_template,
	cmzn_field_id field)
{
	if (node_template)
		return node_template->defineField(field);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_nodetemplate_define_field_from_node(
	cmzn_nodetemplate_id node_template, cmzn_field_id field,
	cmzn_node_id node)
{
	if (node_template)
		return node_template->defineFieldFromNode(field, node);
	return CMZN_ERROR_ARGUMENT;
}

cmzn_timesequence_id cmzn_nodetemplate_get_timesequence(
	cmzn_nodetemplate_id node_template, cmzn_field_id field)
{
	if (node_template && field)
	{
		cmzn_timesequence* timesequence = node_template->getTimesequence(field);
		if (timesequence)
		{
			return cmzn_timesequence_access(timesequence);
		}
	}
	return nullptr;
}

int cmzn_nodetemplate_set_timesequence(
	cmzn_nodetemplate_id node_template, cmzn_field_id field,
	cmzn_timesequence_id timesequence)
{
	if (node_template && field && timesequence)
		return node_template->setTimesequence(field, timesequence);
	return 0;
}

int cmzn_nodetemplate_get_value_number_of_versions(
	cmzn_nodetemplate_id node_template, cmzn_field_id field, int component_number,
	enum cmzn_node_value_label node_value_label)
{
	if (node_template)
		return node_template->getValueNumberOfVersions(field, component_number, node_value_label);
	return -1;
}

int cmzn_nodetemplate_set_value_number_of_versions(
	cmzn_nodetemplate_id node_template, cmzn_field_id field, int component_number,
	enum cmzn_node_value_label node_value_label, int number_of_versions)
{
	if (node_template)
		return node_template->setValueNumberOfVersions(field, component_number, node_value_label, number_of_versions);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_nodetemplate_remove_field(cmzn_nodetemplate_id node_template,
	cmzn_field_id field)
{
	if (node_template)
		return node_template->removeField(field);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_nodetemplate_undefine_field(cmzn_nodetemplate_id node_template,
	cmzn_field_id field)
{
	if (node_template)
		return node_template->undefineField(field);
	return CMZN_ERROR_ARGUMENT;
}
