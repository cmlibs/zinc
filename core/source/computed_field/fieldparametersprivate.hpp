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

#include "opencmiss/zinc/field.h"
#include "opencmiss/zinc/fieldparameters.h"

struct cmzn_fieldparameters
{
private:
	cmzn_field *field;  // accessed field owning these parameters, currently must be finite element type
	int access_count;

	cmzn_fieldparameters(cmzn_field *fieldIn);

	~cmzn_fieldparameters();

public:

	/** @param fieldIn  Finite element type field */
	static cmzn_fieldparameters *create(cmzn_field *fieldIn);

	cmzn_fieldparameters_id access()
	{
		++access_count;
		return this;
	}

	static int deaccess(cmzn_fieldparameters_id &fieldparameters);

	/** @return  Non-accessed field */
	cmzn_field *getField() const
	{
		return this->field;
	}

	int getNumberOfParameters() const;

};

#endif /* !defined (CMZN_FIELDPARAMETERSPRIVATE_HPP) */
