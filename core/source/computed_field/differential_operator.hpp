/***************************************************************************//**
 * FILE : differential_operator.hpp
 *
 * Internal header for class representing a differential differential_operator
 * that can be applied to a field to obtain a derivative.
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
 * Portions created by the Initial Developer are Copyright (C) 2011
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
#if !defined (DIFFERENTIAL_OPERATOR_HPP)
#define DIFFERENTIAL_OPERATOR_HPP

extern "C" {
#include "api/cmiss_differential_operator.h"
#include "finite_element/finite_element_region.h"
}

/**
 * For now can only represent a differential differential_operator give first derivatives
 * with respect to differential_operator elements of given dimension from fe_region.
 */
struct Cmiss_differential_operator
{
private:
	FE_region *fe_region;
	int dimension;
	int term; // which derivative for multiple dimensions, 1 = d/dx1
	int access_count;

public:
	Cmiss_differential_operator(FE_region *fe_region, int dimension, int term) :
		fe_region(ACCESS(FE_region)(fe_region)),
		dimension(dimension),
		term(term),
		access_count(1)
	{
	}

	Cmiss_differential_operator_id access()
	{
		++access_count;
		return this;
	}

	static int deaccess(Cmiss_differential_operator_id &differential_operator)
	{
		if (!differential_operator)
			return 0;
		--(differential_operator->access_count);
		if (differential_operator->access_count <= 0)
			delete differential_operator;
		differential_operator = 0;
		return 1;
	}

	int getDimension() const { return dimension; }
	FE_region *getFeRegion() const { return fe_region; }
	int getTerm() const { return term; }

private:

	~Cmiss_differential_operator()
	{
		DEACCESS(FE_region)(&fe_region);
	}

};

#endif /* !defined (DIFFERENTIAL_OPERATOR_HPP) */
