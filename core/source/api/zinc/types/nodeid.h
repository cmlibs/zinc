/**
 * FILE : nodeid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_NODESETID_H__
#define CMZN_NODESETID_H__

	struct cmzn_nodeset;
	/** Handle to a nodeset, the container for nodes in a region. */
	typedef struct cmzn_nodeset *cmzn_nodeset_id;

	struct cmzn_nodeset_group;
	/** Handle to a nodeset group, a subset of a nodeset */
	typedef struct cmzn_nodeset_group *cmzn_nodeset_group_id;

	struct cmzn_nodetemplate;
	/** Handle to a template for creating or defining fields at a node. */
	typedef struct cmzn_nodetemplate *cmzn_nodetemplate_id;

	struct cmzn_node;
	/** Handle to a single node object */
	typedef struct cmzn_node *cmzn_node_id;

	struct cmzn_nodeiterator;
	/** Handle to an iterator for iterating over a nodeset */
	typedef struct cmzn_nodeiterator * cmzn_nodeiterator_id;

	/**
	 * Enumerated labels for field value/derivative parameters held at nodes.
	 */
	enum cmzn_node_value_label
	{
		CMZN_NODE_VALUE_LABEL_INVALID = 0,
		CMZN_NODE_VALUE_LABEL_VALUE = 1,       /*!< literal field value */
		CMZN_NODE_VALUE_LABEL_D_DS1 = 2,       /*!< derivative w.r.t. arc length S1 */
		CMZN_NODE_VALUE_LABEL_D_DS2 = 3,       /*!< derivative w.r.t. arc length S2 */
		CMZN_NODE_VALUE_LABEL_D_DS3 = 4,       /*!< derivative w.r.t. arc length S3 */
		CMZN_NODE_VALUE_LABEL_D2_DS1DS2 = 5,   /*!< cross derivative w.r.t. arc lengths S1,S2 */
		CMZN_NODE_VALUE_LABEL_D2_DS1DS3 = 6,   /*!< cross derivative w.r.t. arc lengths S1,S3 */
		CMZN_NODE_VALUE_LABEL_D2_DS2DS3 = 7,   /*!< cross derivative w.r.t. arc lengths S2,S3 */
		CMZN_NODE_VALUE_LABEL_D3_DS1DS2DS3 = 8 /*!< triple cross derivative w.r.t. arc lengths S1,S2,S3 */
	};

#endif
