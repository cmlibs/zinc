/**
 * FILE : nodetemplate.hpp
 *
 * Interface to nodetemplate implementation.
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "cmlibs/zinc/nodetemplate.h"
#include "cmlibs/zinc/timesequence.h"
#include "computed_field/computed_field_finite_element.h"
#include "finite_element/finite_element_field.hpp"
#include "finite_element/finite_element_nodeset.hpp"
#include "finite_element/finite_element_region.h"
#include <vector>


class cmzn_node_field;

struct cmzn_nodetemplate
{
private:
	FE_nodeset* fe_nodeset;
	FE_node_template* fe_node_template;
	std::vector<cmzn_node_field*> fields;
	std::vector<FE_field*> undefine_fields; // ACCESSed
	int access_count;

	cmzn_nodetemplate(FE_nodeset* feNodesetIn);

	~cmzn_nodetemplate();

	cmzn_node_field* getNodeField(FE_field* fe_field) const;

	bool removeDefineField(FE_field* fe_field);

	/** Replaces any existing define or undefine field for fe_field */
	cmzn_node_field* createNodeField(FE_field* fe_field);

	bool isUndefineNodeField(FE_field* fe_field);

	bool removeUndefineField(FE_field* fe_field);

	/** Replaces any existing define or undefine field for fe_field */
	void setUndefineNodeField(FE_field* fe_field);

	void invalidate();

	int checkValidFieldForDefine(cmzn_field* field) const;

public:

	static cmzn_nodetemplate* create(FE_nodeset* feNodesetIn)
	{
		if (feNodesetIn)
		{
			return new cmzn_nodetemplate(feNodesetIn);
		}
		return nullptr;
	}

	cmzn_nodetemplate* access()
	{
		++access_count;
		return this;
	}

	static void deaccess(cmzn_nodetemplate*& nodetemplate)
	{
		if (nodetemplate)
		{
			--(nodetemplate->access_count);
			if (nodetemplate->access_count <= 0)
			{
				delete nodetemplate;
			}
			nodetemplate = nullptr;
		}
	}

	int defineField(cmzn_field* field);

	int defineFieldFromNode(cmzn_field* field, cmzn_node* node);

	/** @return  Non-accessed timesequence, or nullptr if none. */
	cmzn_timesequence* getTimesequence(cmzn_field* field);

	/** @param timesequence  Time sequence from this region/fieldmodule, or nullptr to clear. GRC check */
	int setTimesequence(cmzn_field* field, cmzn_timesequence* timesequence);

	int getValueNumberOfVersions(cmzn_field* field, int componentNumber,
		enum cmzn_node_value_label nodeValueLabel);

	int setValueNumberOfVersions(cmzn_field* field, int componentNumber,
		cmzn_node_value_label nodeValueLabel, int numberOfVersions);

	int removeField(cmzn_field* field);

	int undefineField(cmzn_field* field);

	int validate();

	int mergeIntoNode(cmzn_node* node);

	FE_node_template* get_FE_node_template()
	{
		return this->fe_node_template;
	}

};
