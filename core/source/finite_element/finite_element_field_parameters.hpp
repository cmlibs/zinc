/**
 * FILE : finite_element_field_parameters.hpp
 *
 * Records field parameter indexing.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (FINITE_ELEMENT_FIELD_PARAMETERS_HPP)
#define FINITE_ELEMENT_FIELD_PARAMETERS_HPP

#include "datastore/labels.hpp"
#include "general/block_array.hpp"

struct cmzn_element;
struct FE_field;

/**
 * Records field parameter indexing, giving a unique 0-based parameter index
 * for each node/component/derivative/version
 * Currently limited to node parameters.
 */
class FE_field_parameters
{
	FE_field *field;  // accessed field owning these parameters
	int parametersCount;
	// map from node index to first parameter held at that node or -1 if none
	// all node parameters are consecutive, components varying slowest
	block_array<DsLabelIndex, DsLabelIndex> nodeParameterMap;
	// map from parameter to node
	block_array<DsLabelIndex, DsLabelIndex> parameterNodeMap;
	int fieldModifyCounter;  // incremented when field structure changes; rebuild maps when non-zero
	int access_count;

	FE_field_parameters(FE_field *fieldIn);

	~FE_field_parameters();

	void generateMaps();

	/** call before using maps to generate on demand */
	inline void checkMaps()
	{
		if (this->fieldModifyCounter)
			this->generateMaps();
	}

public:

	/** Only to be called by FE_field
	 * Checks field is real, general type
	 * @return  Accessed finite element field parameters or nullptr if invalid field or failed. */
	static FE_field_parameters *create(FE_field *fieldIn);

	FE_field_parameters *access()
	{
		++access_count;
		return this;
	}

	static int deaccess(FE_field_parameters* &fe_field_parameters);

	/** @return  Non-accessed FE_field */
	FE_field *getField() const
	{
		return this->field;
	}

	int getNumberOfElementParameters(cmzn_element *element);

	int getNumberOfParameters();

};

#endif /* !defined (FINITE_ELEMENT_FIELD_PARAMETERS_HPP) */
