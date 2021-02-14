/**
 * FILE : fieldparametersprivate.hpp
 * 
 * Implementation of field parameters class.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (CMZN_FIELDPARAMETERSPRIVATE_HPP)
#define CMZN_FIELDPARAMETERSPRIVATE_HPP

#include "opencmiss/zinc/types/elementid.h"
#include "opencmiss/zinc/types/fieldid.h"
#include "opencmiss/zinc/fieldparameters.h"
#include "finite_element/finite_element_constants.hpp"

class FE_field_parameters;
class FieldDerivative;

struct cmzn_fieldparameters
{
private:
	cmzn_field *field;  // accessed field owning these parameters, currently only finite element type allowed
	FE_field_parameters *feFieldParameters;  // accessed finite element field parameters
	FieldDerivative *fieldDerivatives[MAXIMUM_PARAMETER_DERIVATIVE_ORDER];  // first and second derivatives wrt parameters
	FE_mesh *meshes[MAXIMUM_ELEMENT_XI_DIMENSIONS];  // accessed meshes for which mixed derivatives are held
	// Following are initiall nullptr, but are accessed pointers once used so maintained for the life of the field parameters
	FieldDerivative *mixedFieldDerivatives[MAXIMUM_ELEMENT_XI_DIMENSIONS][MAXIMUM_MESH_DERIVATIVE_ORDER][MAXIMUM_PARAMETER_DERIVATIVE_ORDER];
	int access_count;

	cmzn_fieldparameters(cmzn_field *fieldIn, FE_field_parameters *feFieldParametersIn);

	~cmzn_fieldparameters();

public:

	/** Only to be called by cmzn_field.
	 * Checks fieldIn is real, general finite element type.
	 * @param fieldIn  Finite element type field
	 * @return  Accessed field parameters or nullptr if invalid field or failed. */
	static cmzn_fieldparameters *create(cmzn_field *fieldIn);

	cmzn_fieldparameters *access()
	{
		++access_count;
		return this;
	}

	static int deaccess(cmzn_fieldparameters* &fieldparameters);

	/** @return  Non-accessed field */
	cmzn_field *getField() const
	{
		return this->field;
	}

	/** @return  Non-accessed field derivative w.r.t. parameters of given order or nullptr if invalid order */
	FieldDerivative *getFieldDerivative(int order) const
	{
		if ((order < 1) || (order > 2))
			return nullptr;
		return this->fieldDerivatives[order - 1];
	}

	/** @return  Non-accessed field derivative w.r.t. mesh and parameters
	 * The derivative is held by parameters until parameters object destroyed */
	FieldDerivative *getFieldDerivativeMixed(FE_mesh *mesh, int meshOrder, int parameterOrder);

	int getNumberOfElementParameters(cmzn_element *element) const;

	int getNumberOfParameters() const;

};

#endif /* !defined (CMZN_FIELDPARAMETERSPRIVATE_HPP) */
