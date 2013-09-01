/***************************************************************************//**
 * FILE : cmiss_field_id.h
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

#ifndef CMZN_FIELDID_H__
#define CMZN_FIELDID_H__

	struct Cmiss_field;
	typedef struct Cmiss_field *Cmiss_field_id;

	struct Cmiss_field_iterator;
	typedef struct Cmiss_field_iterator * Cmiss_field_iterator_id;

	struct Cmiss_field_cache;
	typedef struct Cmiss_field_cache *Cmiss_field_cache_id;

/**
 * An enum specifying the field domain (without domain objects)
 */
enum Cmiss_field_domain_type
{
	CMISS_FIELD_DOMAIN_TYPE_INVALID = 0,
	CMISS_FIELD_DOMAIN_POINT = 1,
		/*!< A single point for the region */
	CMISS_FIELD_DOMAIN_NODES = 2,
		/*!< The set of node points */
	CMISS_FIELD_DOMAIN_DATA = 4,
		/*!< The set of data points */
	CMISS_FIELD_DOMAIN_MESH_1D = 8,
		/*!< The set of 1-D elements and edge lines of 2-D or 3-D elements */
	CMISS_FIELD_DOMAIN_MESH_2D = 16,
		/*!< The set of 2-D elements and faces of 3-D elements */
	CMISS_FIELD_DOMAIN_MESH_3D = 32,
		/*!< The set of 3-D elements */
	CMISS_FIELD_DOMAIN_MESH_HIGHEST_DIMENSION = 64
		/*!< The set of elements of highest dimension in region */
};

#endif
