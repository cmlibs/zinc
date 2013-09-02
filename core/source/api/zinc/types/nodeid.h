/***************************************************************************//**
 * FILE : cmiss_node_id.h
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
 * Portions created by the Initial Developer are Copyright (C) 2005-2011
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

/** Handle to a nodeset, the container for nodes in a region. */
#ifndef CMZN_NODESETID_H__
#define CMZN_NODESETID_H__

	struct cmzn_nodeset;
	typedef struct cmzn_nodeset *cmzn_nodeset_id;

	struct cmzn_nodeset_group;
	typedef struct cmzn_nodeset_group *cmzn_nodeset_group_id;

	/** Handle to a template for creating or defining fields at a node. */
	struct cmzn_node_template;
	typedef struct cmzn_node_template *cmzn_node_template_id;

	struct cmzn_node;
	/** Handle to a single node object */
	typedef struct cmzn_node *cmzn_node_id;

	struct cmzn_node_iterator;
	typedef struct cmzn_node_iterator * cmzn_node_iterator_id;

	/***************************************************************************//**
	 * The type of a nodal parameter value.
	 */
	enum cmzn_nodal_value_type
	{
		CMISS_NODAL_VALUE_TYPE_INVALID = 0,
		CMISS_NODAL_VALUE = 1,         /* literal field value */
		CMISS_NODAL_D_DS1 = 2,         /* derivative w.r.t. arc length S1 */
		CMISS_NODAL_D_DS2 = 3,         /* derivative w.r.t. arc length S2 */
		CMISS_NODAL_D_DS3 = 4,         /* derivative w.r.t. arc length S3 */
		CMISS_NODAL_D2_DS1DS2 = 5,     /* cross derivative w.r.t. arc lengths S1,S2 */
		CMISS_NODAL_D2_DS1DS3 = 6,     /* cross derivative w.r.t. arc lengths S1,S3 */
		CMISS_NODAL_D2_DS2DS3 = 7,     /* cross derivative w.r.t. arc lengths S2,S3 */
		CMISS_NODAL_D3_DS1DS2DS3 = 8   /* triple cross derivative w.r.t. arc lengths S1,S2,S3 */
	};

#endif
