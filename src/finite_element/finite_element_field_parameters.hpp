/**
 * FILE : finite_element_field_parameters.hpp
 *
 * Records field parameter indexing.
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (FINITE_ELEMENT_FIELD_PARAMETERS_HPP)
#define FINITE_ELEMENT_FIELD_PARAMETERS_HPP

#include "cmlibs/zinc/types/elementid.h"
#include "cmlibs/zinc/types/nodeid.h"
#include "cmlibs/zinc/zincconfigure.h"
#include "datastore/labels.hpp"
#include "general/block_array.hpp"

struct FE_field;

/**
 * Records field parameter indexing, giving a unique 0-based parameter index
 * for each node/component/derivative/version
 * Currently limited to node parameters.
 */
class FE_field_parameters
{
	FE_field *field;  // accessed field owning these parameters
	FE_value time;  // time at which parameters are held
	int parameterCount;  // total number of parameters
	// map from node index to first parameter held at that node or -1 if none
	// all node parameters are consecutive, components varying slowest
	block_array<DsLabelIndex, DsLabelIndex> nodeParameterMap;
	// map from parameter to node
	block_array<DsLabelIndex, DsLabelIndex> parameterNodeMap;
	FE_value perturbationDelta;
	int access_count;

	FE_field_parameters(FE_field *fieldIn);

	~FE_field_parameters();

	void generateMaps();

	/** Generic method implementing common parts of add/get/set field parameters methods.
	 * @param processValues  Class performing operation on values, implementing methods:
	 * bool checkValues(int minimumValueCount)
	 * operator()(node, field, componentNumber, time, processValuesCount, valueIndex)
	 * const char *getApiName() */
	template <class ProcessValuesOperator> int processParameters(ProcessValuesOperator& processValues);

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

	/** Get array of global parameter indexes for field parameters in element.
	 * @param element  The element to query.
	 * @param topLevelElement  Top-level element to inherit field from (pass nullptr for default first ancestor).
	 * @param valuesCount  Size of valuesOut >= number of parameters in element.
	 * @param valuesOut  Location to put parameter indexes.
	 * @param startIndex  Start index for returned parameters: 0 for zero-based indexes, 1 for one-based.
	 * @return Result OK on success, ERROR_NOT_FOUND if field not defined/no parameters on element,
	 * otherwise any other error code. */
	int getElementParameterIndexes(cmzn_element *element, cmzn_element *topLevelElement, int valuesCount, int *valuesOut, int startIndex = 0);

	/** If parameter is node-based, return the node, field component, value label and version.
	 * @param parameterIndex  Zero-based parameter parameter index.
	 * @param fieldComponent  Return field component starting at 0 for node parameter, or -1 if not.
	 * @param valueLabel  Return value label for node parameter, or INVALID if not.
	 * @param version  Return version starting at 0 for node parameter, or -1 if not.
	 * @return  Non-accessed node if node parameter, or nullptr if not or failed.
	 */
	cmzn_node *getNodeParameter(int parameterIndex, int &fieldComponent, cmzn_node_value_label& valueLabel, int& version);

	/**
	 * @param element  Element to get field parameters for.
	 * @param topLevelElement  Optional top-level element to inherit field from.
	 * @return  Number of parameters >= 0, or -1 if error
	 */
	int getNumberOfElementParameters(cmzn_element *element, cmzn_element *topLevelElement = nullptr);

	/** Currently also triggers creation of parameter map */
	int getNumberOfParameters();

	/* Add incremental values to all field parameters.
	 * @param valuesCount  The size of the valuesIn array >= total number of parameters.
	 * @param valuesIn  Array containing increments to add, in index order.
	 * @return Result OK on success, or error code. */
	int addParameters(int valuesCount, const FE_value *valuesIn);

	/* Get values of all field parameters.
	 * @param valuesCount  The size of the valuesOut array >= total number of parameters.
	 * @param valuesOut  Array to fill with parameter values, in index order.
	 * @return Result OK on success, or error code. */
	int getParameters(int valuesCount, FE_value *valuesOut);

	/* Assign values to all field parameters.
	 * @param valuesCount  The size of the valuesIn array >= total number of parameters.
	 * @param valuesIn  Array containing new parameter values, in index order.
	 * @return Result OK on success, or error code. */
	int setParameters(int valuesCount, const FE_value *valuesIn);

	/** @return  Positive delta to apply when perturbing parameters to calculate numerical derivatives */
	FE_value getPerturbationDelta() const
	{
		return this->perturbationDelta;
	}

	/** Get time at which parameters are being used. */
	FE_value getTime() const
	{
		return this->time;
	}

	/** Set time at which parameters are to be used, must match a time held at nodes.
	 * Client should call getNumberOfParameters() afterwards to re-generate map. */
	int setTime(FE_value timeIn);
};

#endif /* !defined (FINITE_ELEMENT_FIELD_PARAMETERS_HPP) */
