/**
 * FILE : field_range.hpp
 *
 * Stores range of a fields values over a chosen domain, the locations at
 * which each component minimum or maximum occurs and the field values there.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (__FIELD_RANGE_HPP__)
#define __FIELD_RANGE_HPP__

#include "opencmiss/zinc/field.h"
#include "opencmiss/zinc/fieldcache.h"
#include "opencmiss/zinc/fieldrange.h"
#include "computed_field/field_location.hpp"

struct cmzn_fieldrange
{
private:
	cmzn_fieldcache *fieldcache; // accessed field cache owning this range
	cmzn_field *field;  // accessed field the range is for
	bool validRange;
	// values and locations of minimum/maximum for each component
	// currently limited to mesh locations
	Field_location_element_xi *componentMinimumLocations, *componentMaximumLocations;
	// starting with field values at the first component minimum/maximum location
	// currently limited to real values
	FE_value *componentMinimumValues, *componentMaximumValues;
	int access_count;

	cmzn_fieldrange(cmzn_fieldcache *fieldcacheIn);

	~cmzn_fieldrange();

	cmzn_fieldrange();  // not implemented
	cmzn_fieldrange(const cmzn_fieldrange &source);  // not implemented
	cmzn_fieldrange& operator=(const cmzn_fieldrange &source);  // not implemented

	void clear();

	void setupForField(cmzn_field *field);

	/** Finds maximum (or minimum) of field component in element and stores it
	 * in its own arrays.
	 * @param componentIndex  Component index starting at 0.
	 * @param findMinimum  True if finding minimum otherwise maximum.
	 * @return  true on success, false on failure */
	bool findComponentLimit(cmzn_field *field, int componentIndex, cmzn_element *element,
		cmzn_fieldcache *fieldcache, FE_value *initialXi, bool findMinimum);

public:

	/** Create range object for field.
	 * @return  Accessed pointer to Fieldrange, or nullptr if failed.
	 */
	static cmzn_fieldrange *create(cmzn_fieldcache *fieldcacheIn);

	cmzn_fieldrange *access()
	{
		++access_count;
		return this;
	}

	static void deaccess(cmzn_fieldrange* &field_range);

	int evaluateElementRange(cmzn_field *field, cmzn_element *element, cmzn_fieldcache *fieldcache);

	/** @param fieldcache  Must have an element/mesh location */
	int evaluateRange(cmzn_field *field, cmzn_fieldcache *fieldcache);

	/** @param componentNumber  Field component number starting at 0 */
	cmzn_element *getComponentMinimumMeshLocation(int componentNumber,
		int coordinatesCount, FE_value *coordinatesOut);

	/** @param componentNumber  Field component number starting at 0 */
	cmzn_element *getComponentMaximumMeshLocation(int componentNumber,
		int coordinatesCount, FE_value *coordinatesOut);

	/** @param componentNumber  Field component number starting at 0 */
	int getComponentMinimumValues(int componentNumber,
		int valuesCount, FE_value *valuesOut);

	/** @param componentNumber  Field component number starting at 0 */
	int getComponentMaximumValues(int componentNumber,
		int valuesCount, FE_value *valuesOut);

	/** @return  Non-accessed field the range is for or nullptr if not a valid range */
	cmzn_field *getField() const
	{
		return this->field;
	}

	int getRange(int valuesCount, FE_value *minimumValuesOut, FE_value *maximumValuesOut) const;

	bool hasValidRange() const
	{
		return this->validRange;
	}

};

#endif /* !defined (__FIELD_RANGE_HPP__) */
