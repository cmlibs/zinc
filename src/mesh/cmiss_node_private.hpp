/***************************************************************************//**
 * FILE : cmiss_node_private.hpp
 *
 * Private header file of cmzn_node, cmzn_nodeset.
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (CMZN_NODE_PRIVATE_HPP)
#define CMZN_NODE_PRIVATE_HPP

#include "cmlibs/zinc/types/regionid.h"
#include "cmlibs/zinc/node.h"
#include "finite_element/finite_element.h"
#include "datastore/labelschangelog.hpp"


class cmzn_node_value_label_conversion
{
public:
	static const char* to_string(enum cmzn_node_value_label label)
	{
		const char* enum_string = nullptr;
		switch (label)
		{
		case CMZN_NODE_VALUE_LABEL_INVALID:
			break;
		case CMZN_NODE_VALUE_LABEL_VALUE:
			enum_string = "VALUE";
			break;
		case CMZN_NODE_VALUE_LABEL_D_DS1:
			enum_string = "D_DS1";
			break;
		case CMZN_NODE_VALUE_LABEL_D_DS2:
			enum_string = "D_DS2";
			break;
		case CMZN_NODE_VALUE_LABEL_D2_DS1DS2:
			enum_string = "D2_DS1DS2";
			break;
		case CMZN_NODE_VALUE_LABEL_D_DS3:
			enum_string = "D_DS3";
			break;
		case CMZN_NODE_VALUE_LABEL_D2_DS1DS3:
			enum_string = "D2_DS1DS3";
			break;
		case CMZN_NODE_VALUE_LABEL_D2_DS2DS3:
			enum_string = "D2_DS2DS3";
			break;
		case CMZN_NODE_VALUE_LABEL_D3_DS1DS2DS3:
			enum_string = "D3_DS1DS2DS3";
			break;
		}
		return enum_string;
	}
};

struct cmzn_fieldmoduleevent;
struct FE_region;
class FE_nodeset;

struct cmzn_nodesetchanges
{
private:
	cmzn_fieldmoduleevent *event; // accessed
	DsLabelsChangeLog *changeLog; // Accessed from object obtained from correct mesh
	int access_count;

	cmzn_nodesetchanges(cmzn_fieldmoduleevent *eventIn, cmzn_nodeset *nodesetIn);
	~cmzn_nodesetchanges();

public:

	static cmzn_nodesetchanges *create(cmzn_fieldmoduleevent *eventIn, cmzn_nodeset *nodesetIn);

	cmzn_nodesetchanges *access()
	{
		++(this->access_count);
		return this;
	}

	static int deaccess(cmzn_nodesetchanges* &nodesetchanges);

	cmzn_node_change_flags getNodeChangeFlags(cmzn_node *node);

	int getNumberOfChanges()
	{
		if (this->changeLog->isAllChange())
			return -1;
		return this->changeLog->getChangeCount();
	}

	cmzn_node_change_flags getSummaryNodeChangeFlags()
	{
		return this->changeLog->getChangeSummary();
	}
};

#endif /* !defined (CMZN_NODE_PRIVATE_HPP) */
