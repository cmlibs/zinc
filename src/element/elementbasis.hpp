/**
 * FILE : elementbasis.hpp
 *
 * Interface to elementbasis implementation.
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "cmlibs/zinc/elementbasis.h"
#include "finite_element/finite_element_region.h"


struct cmzn_elementbasis
{
private:
	FE_region* fe_region; // needed to get basis manager
	int dimension;
	cmzn_elementbasis_function_type* function_types;
	int access_count;

	cmzn_elementbasis(FE_region* feRegionIn, int meshDimensionIn,
		cmzn_elementbasis_function_type functionType);

	~cmzn_elementbasis();

public:
	static cmzn_elementbasis* create(FE_region* feRegionIn, int meshDimensionIn,
		cmzn_elementbasis_function_type functionType);

	static cmzn_elementbasis* create(FE_region* feRegionIn, FE_basis* feBasis);

	cmzn_elementbasis_id access()
	{
		++access_count;
		return this;
	}

	static int deaccess(cmzn_elementbasis_id& basis)
	{
		if (!basis)
			return CMZN_ERROR_ARGUMENT;
		--(basis->access_count);
		if (basis->access_count <= 0)
			delete basis;
		basis = 0;
		return CMZN_OK;
	}

	int getDimension() const
	{
		return dimension;
	}

	/** @return  number of dimension using supplied function_type */
	int getDimensionsUsingFunction(cmzn_elementbasis_function_type function_type) const;

	/** @return  1 if all function types set & at least 2 chart components linked for each simplex basis */
	int isValid() const;

	/** @return  Accessed FE_basis, or NULL on error */
	FE_basis* getFeBasis() const;

	enum cmzn_elementbasis_function_type getFunctionType(int chart_component) const;

	int setFunctionType(int chart_component, cmzn_elementbasis_function_type function_type);

	int getNumberOfNodes() const;

	int getNumberOfFunctions() const;

	int getNumberOfFunctionsPerNode(int basisNodeIndex) const;

};

/**
 * Internal function for getting the FE_basis for elementbasis in its
 * current state.
 * @return  Non-accessed FE_basis on success, 0 on failure. Note FE_basis
 * should remain in existence until shutdown.
 */
FE_basis* cmzn_elementbasis_get_FE_basis(cmzn_elementbasis_id elementbasis);
