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

class FE_field_parameters;

struct cmzn_fieldparameters
{
private:
	cmzn_field *field;  // accessed field owning these parameters, currently only finite element type allowed
	FE_field_parameters *feFieldParameters;  // accessed finite element field parameters
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

	int getNumberOfElementParameters(cmzn_element *element) const;

	int getNumberOfParameters() const;

};

#endif /* !defined (CMZN_FIELDPARAMETERSPRIVATE_HPP) */
