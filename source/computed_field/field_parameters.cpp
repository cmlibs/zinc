/***************************************************************************//**
 * FILE : field_parameters.cpp
 * 
 * Implements parameter sets indexed by ensemble domains.
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
 * Portions created by the Initial Developer are Copyright (C) 2010
 * the Initial Developer. All Rights Reserved.
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
#include <map>
#include <typeinfo>
extern "C" {
#include "computed_field/computed_field.h"
#include "field_io/cmiss_field_parameters.h"
}
#include "computed_field/computed_field_private.hpp"
#include "computed_field/field_ensemble.hpp"
#include "general/block_array.hpp"

// GRC pointer size check may not be portable:
#if __SIZEOF_POINTER__ == 8
	typedef unsigned long long int ParameterIndexType;
#else
	typedef unsigned int ParameterIndexType;
#endif

namespace Cmiss
{

template <typename ValueType>
class Field_parameters : public Computed_field_core
{
	bool dense;
	int number_of_ensembles;
	Field_ensemble **ensembles;
	// number of refs allocated per ensemble; can be more or less than ensemble->maxRef 
	EnsembleEntryRef *refSize;
	// memory offset for a unit of each index ensemble:
	// equal to product of following refSizes; largest for first, 1 for last
	ParameterIndexType *offsets;
	// parameter values indexed by refSize[i] refs for each ensembles[i]
	block_array<ParameterIndexType, ValueType> values;
	// for non-dense, flag indexed as for values which is true if value exists, false if not
	bool_array<ParameterIndexType> value_exists;

public:
	Field_parameters(int in_number_of_ensembles, Field_ensemble **in_ensembles) :
		Computed_field_core(),
		dense(true),
		number_of_ensembles(in_number_of_ensembles),
		ensembles(new Field_ensemble*[in_number_of_ensembles]),
		refSize(new EnsembleEntryRef[in_number_of_ensembles]),
		offsets(new ParameterIndexType[in_number_of_ensembles])
	{
		if ((ensembles) && (refSize) && (offsets))
		{
			for (int i = 0; i < number_of_ensembles; i++)
			{
				ensembles[i] = in_ensembles[i];
				refSize[i] = 0;
				// default offset = 1 sets up single index ensemble. Multiple index ensembles will call resize
				offsets[i] = 1;
			}
		}
	}

	virtual bool attach_to_field(Computed_field *parent)
	{
		if (Computed_field_core::attach_to_field(parent))
		{
			if ((0 == number_of_ensembles) || ((ensembles) && (refSize) && (offsets)))
			{
				return true;
			}
		}
		return false;
	}

	~Field_parameters()
	{
		delete[] ensembles;
		delete[] refSize;
		delete[] offsets;
	}

private:
	Computed_field_core *copy()
	{
		return NULL; // not supported
	}

	int compare(Computed_field_core* other_field)
	{
		return (NULL != dynamic_cast<Field_parameters<ValueType> *>(other_field)) ? 1 : 0;
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string()
	{
		return NULL;
	}

	ParameterIndexType getMaxParameterCount() const;
	bool setNotDense();
	bool copyValues(int ensemble_number, ParameterIndexType oldBaseIndex,
		ParameterIndexType newBaseIndex, ParameterIndexType *newOffsets,
		EnsembleEntryRef *copySize, block_array<ParameterIndexType, ValueType>& dest_values,
		bool_array<ParameterIndexType>& dest_value_exists) const;
	bool resize(EnsembleEntryRef *newRefSize);
	int validIndexCount(const Cmiss_ensemble_index *index, unsigned int number_of_values,
		const char *methodName) const;

public:

	Cmiss_ensemble_index *createIndex()
	{
		return Cmiss_ensemble_index::create(field, number_of_ensembles, ensembles);
	}

	int getValues(Cmiss_ensemble_index *index, unsigned int number_of_values, ValueType *outValues) const;

	int getValuesSparse(Cmiss_ensemble_index *index, unsigned int number_of_values, ValueType *outValues,
		int *valueExists, int *valuesRead) const;

	int setValues(Cmiss_ensemble_index *index, unsigned int number_of_values, ValueType *inValues);

};

template <typename ValueType>
int Field_parameters<ValueType>::evaluate_cache_at_location(Field_location* location)
{
	int return_code = 0;
	if (field && location)
	{
		field->values[0] = 0;
#if defined (FUTURE_CODE)
		// GRC update for parameter lookup
		EnsembleEntryRef ref;
		if (location.getEnsembleEntry(this, ref))
		{
			Cmiss_ensemble_entry_identifier identifier = getIdentifier(ref);
			if (0 < identifier)
			{
				field->values[0] = static_cast<FE_value>(identifier);
				return_code = 1;
			}
		}
#endif
		field->derivatives_valid = 0;
	}
	return (return_code);
}

template <typename ValueType>
int Field_parameters<ValueType>::list()
{
	for (int i = 0; i < number_of_ensembles; i++)
	{
		display_message(INFORMATION_MESSAGE, "    Index ensemble %d: %s\n", i+1,
			ensembles[i]->getField()->name);
	}
	return (1);
}

/** @return  number of parameters that would be stored if status were dense */
template <typename ValueType>
ParameterIndexType Field_parameters<ValueType>::getMaxParameterCount() const
{
	ParameterIndexType maxParameterCount = 1;
	for (int i = 0; i < number_of_ensembles; i++)
	{
		maxParameterCount *= refSize[i];
	}
	return maxParameterCount;
}

/** Set value_exists flags to true for all parameters.
 * Must call prior to inserting parameters non-densely.
 * @return  true on success, false if failed / status remains dense.
 */
template <typename ValueType>
bool Field_parameters<ValueType>::setNotDense()
{
	if (!dense)
		return true;
	//display_message(INFORMATION_MESSAGE, "In Field_parameters::setNotDense  field %s %d\n", field->name, getMaxParameterCount()); // GRC test
	dense = false;
	if (value_exists.setAllTrue(getMaxParameterCount()))
		return true;
	// failed: restore dense status and return false
	display_message(ERROR_MESSAGE,
		"Field_parameters::setNotDense.  Failed to convert parameters field %s to non-dense",
		field->name);
	value_exists.clear();
	dense = true;
	return false;
}

/** Recursively copies parameters from internal arrays to dest arrays
 * @return  true on complete success, false otherwise.
 */
template <typename ValueType>
bool Field_parameters<ValueType>::copyValues(int ensemble_number,
	ParameterIndexType oldBaseIndex,
	ParameterIndexType newBaseIndex,
	ParameterIndexType *newOffsets,
	EnsembleEntryRef *copySize,
	block_array<ParameterIndexType, ValueType>& dest_values,
	bool_array<ParameterIndexType>& dest_value_exists) const
{
	EnsembleEntryRef thisCopySize = copySize[ensemble_number];
	ParameterIndexType oldIndex = oldBaseIndex;
	ParameterIndexType newIndex = newBaseIndex;
	EnsembleEntryRef index;
	if (ensemble_number == (number_of_ensembles - 1))
	{
		ValueType value;
		for (index = 0; index < thisCopySize; index++)
		{
			if (!dense && !value_exists.getBool(oldIndex))
				continue;
			if (!values.getValue(oldIndex, value))
			{
				display_message(ERROR_MESSAGE, "Field_parameters::copyValues  Field %s is missing a parameter\n",
					field->name);
				return false;
			}
			if (!dest_values.setValue(newIndex, value))
				return false;
			bool oldValue;
			if (!dense && !dest_value_exists.setBool(oldIndex, true, oldValue))
				return false;
      newIndex++;
      oldIndex++;
		}
	}
	else
	{
		for (index = 0; index < thisCopySize; index++)
		{
			if (!copyValues(ensemble_number + 1, oldIndex, newIndex,
				newOffsets, copySize, dest_values, dest_value_exists))
				return false;
			oldIndex += offsets[ensemble_number];
			newIndex += newOffsets[ensemble_number];
		}
	}
	return true;
}

/** Resizes existing parameter array. Very expensive for large parameter sets.
 * Atomic.
 * GRC: Can be made more efficient.
 * @param newRefSize  array of new sizes for each of the {number_of_ensembles} ensembles
 * @return  true on success, false on failure - with arrays unchanged
 */
template <typename ValueType>
bool Field_parameters<ValueType>::resize(EnsembleEntryRef *newRefSize)
{
#if defined (DEBUG_CODE)
	display_message(INFORMATION_MESSAGE, "Field_parameters::resize  Field %s\n", field->name);	// GRC test
	for (int i = 0; i < number_of_ensembles; i++)
	{
		display_message(INFORMATION_MESSAGE, "    ensemble %s: %u -> %u\n",
			ensembles[i]->getField()->name, refSize[i], newRefSize[i]);	// GRC test
	}
#endif /* defined (DEBUG_CODE) */

	// values to copy is minimum of refSize and newRefSize
	EnsembleEntryRef *copySize = new EnsembleEntryRef[number_of_ensembles];
	ParameterIndexType *newOffsets = new ParameterIndexType[number_of_ensembles];
	if ((!copySize) || (!newOffsets))
		return false;
	for (int i = number_of_ensembles - 1; 0 <= i; i--)
	{
		copySize[i] = refSize[i];
		if (newRefSize[i] < copySize[i])
			copySize[i] = newRefSize[i];
		if (i == (number_of_ensembles - 1))
			newOffsets[i] = 1;
		else
			newOffsets[i] = newOffsets[i+1]*newRefSize[i+1];
	}

	// no copy if currently empty, but need to set refSize and offsets
	if (getMaxParameterCount() > 0)
	{
		block_array<ParameterIndexType, ValueType> dest_values;
		bool_array<ParameterIndexType> dest_value_exists;
		if (!copyValues(/*ensemble_number*/0, /*oldBaseIndex*/0, /*newBaseIndex*/0,
			newOffsets, copySize, dest_values, dest_value_exists))
		{
			display_message(WARNING_MESSAGE,
				"Field_parameters::resize  Not enough memory to resize parameters for field %s\n",
				field->name);
			delete[] copySize;
			delete[] newOffsets;
			return false;
		}
		values.swap(dest_values);
		value_exists.swap(dest_value_exists);
	}
	for (int i = 0; i < number_of_ensembles; i++)
	{
		refSize[i] = newRefSize[i];
		offsets[i] = newOffsets[i];
	}
	delete[] copySize;
	delete[] newOffsets;
	return true;
}

template <typename ValueType>
inline int Field_parameters<ValueType>::validIndexCount(const Cmiss_ensemble_index *index, unsigned int number_of_values,
	const char *methodName) const
{
	if (!index->indexesField(field))
	{
		display_message(ERROR_MESSAGE, "%s.  Invalid index for field %s",
			methodName, field->name);
		return 0;
	}
	unsigned int index_entry_count = index->getEntryCount();
	if (0 == index_entry_count)
	{
		display_message(ERROR_MESSAGE, "%s.  Invalid index specifies zero values.",
			methodName, field->name);
		return 0;
	}
	if (number_of_values != index_entry_count)
	{
		display_message(ERROR_MESSAGE,
			"%s.  Index specifies %d values, %d supplied for field %s.",
			methodName, index_entry_count, number_of_values, field->name);
		return 0;
	}
	return 1;
}

template <typename ValueType>
int Field_parameters<ValueType>::getValues(
	Cmiss_ensemble_index *index, unsigned int number_of_values, ValueType *outValues) const
{
	if (!validIndexCount(index, number_of_values, "Field_parameters::getValues"))
		return 0;

	// iterate to get values in identifier order for each ensemble
	if (!index->iterationBegin())
	{
		display_message(ERROR_MESSAGE,
			"Field_parameters::getValues  Failed to begin iteration over index for field %s\n",
			field->name);
		return 0;
	}
	int i;
	int return_code = 1;
	ParameterIndexType valueIndex = 0;
	unsigned int value_number;
	EnsembleEntryRef ref;
	bool iterResult = true;
	for (value_number = 0; iterResult && (value_number < number_of_values); value_number++)
	{
		valueIndex = 0;
		for (i = 0; i < number_of_ensembles; i++)
		{
			ref = index->iterationRef(i);
			if (ref >= refSize[i])
			{
				return_code = 0;
				break;
			}
			valueIndex += ref*offsets[i];
		}
		if (!return_code)
			break;
		if (!dense && !value_exists.getBool(valueIndex))
		{
#if 0 // GRC DEBUG_CODE
			display_message(ERROR_MESSAGE,
				"Field_parameters::getValues  Value doesn't exist at index %d", valueIndex);
			for (int i = 0; i < number_of_ensembles; i++)
			{
				ref = index->iterationRef(i);
				display_message(INFORMATION_MESSAGE, "Ensemble %d %s : ref %d refSize = %d (#values %d)\n",
					i, this->ensembles[i]->getField()->name, ref, refSize[i], number_of_values);
			}
			if (number_of_ensembles == 2)
			{
				for (int i = 0; i < refSize[0]; i++)
				{
					display_message(INFORMATION_MESSAGE, "%3d :", i);
					int ix = i*offsets[0];
					for (int j = 0; j < refSize[1]; j++)
					{
						ix += offsets[1];
						display_message(INFORMATION_MESSAGE, " %d", (int)value_exists.getBool(ix));
					}
					display_message(INFORMATION_MESSAGE, "\n");
				}
			}
#endif
			return_code = 0;
			break;
		}
		if (!values.getValue(valueIndex, outValues[value_number]))
		{
			return_code = 0;
			break;
		}
		iterResult = index->iterationNext();
	}
	index->iterationEnd();

	if (return_code && (value_number < number_of_values))
	{
		display_message(ERROR_MESSAGE,
			"Field_parameters::getValues  Only %u out of %u values iterated for field %s\n",
			value_number, number_of_values, field->name);
		return_code = 0;
	}
	else if (iterResult && (value_number >= number_of_values))
	{
		display_message(ERROR_MESSAGE,
			"Field_parameters::getValues  Iteration past end of values for field %s\n",
			field->name);
		return_code = 0;
	}
	return return_code;
}

template <typename ValueType>
int Field_parameters<ValueType>::getValuesSparse(Cmiss_ensemble_index *index,
	unsigned int number_of_values, ValueType *outValues, int *valueExists, int *valuesRead) const
{
	if (!validIndexCount(index, number_of_values, "Field_parameters::getValuesSparse"))
		return 0;

	// iterate to get values in identifier order for each ensemble
	if (!index->iterationBegin())
	{
		display_message(ERROR_MESSAGE,
			"Field_parameters::getValuesSparse  Failed to begin iteration over index for field %s\n",
			field->name);
		return 0;
	}
	int i;
	*valuesRead = 0;
	int return_code = 1;
	ParameterIndexType valueIndex = 0;
	unsigned int value_number;
	EnsembleEntryRef ref;
	bool iterResult = true;
	for (value_number = 0; iterResult && (value_number < number_of_values); value_number++)
	{
		valueIndex = 0;
		for (i = 0; i < number_of_ensembles; i++)
		{
			ref = index->iterationRef(i);
			if (ref >= refSize[i])
			{
				return_code = 0;
				break;
			}
			valueIndex += index->iterationRef(i)*offsets[i];
		}
		if (!return_code)
			break;
		if (!dense && !value_exists.getBool(valueIndex))
		{
			valueExists[value_number] = 0;
		}
		else if (values.getValue(valueIndex, outValues[value_number]))
		{
			valueExists[value_number] = 1;
			++(*valuesRead);
		}
		else
		{
			return_code = 0;
			break;
		}
		iterResult = index->iterationNext();
	}
	index->iterationEnd();

	if (return_code && (value_number < number_of_values))
	{
		display_message(ERROR_MESSAGE,
			"Field_parameters::getValuesSparse  Only %u out of %u values iterated for field %s\n",
			value_number, number_of_values, field->name);
		return_code = 0;
	}
	else if (iterResult && (value_number >= number_of_values))
	{
		display_message(ERROR_MESSAGE,
			"Field_parameters::getValuesSparse  Iteration past end of values for field %s\n",
			field->name);
		return_code = 0;
	}
	return return_code;
}

template <typename ValueType>
int Field_parameters<ValueType>::setValues(
	Cmiss_ensemble_index *index, unsigned int number_of_values, ValueType *inValues)
{
	if (!validIndexCount(index, number_of_values, "Field_parameters::setValues"))
		return 0;

	int i, j;
	index->calculateIndexRefLimits();
	bool resizeInnerEnsemble = false;
	bool clearDense = false;
	for (i = 0; i < number_of_ensembles; i++)
	{
		if (index->indexRefLimit(i) > refSize[i])
		{
			if (i > 0)
			{
				resizeInnerEnsemble = true;
			}
			if (dense && !clearDense)
			{
				if (index->isDenseOnEnsemble(i))
				{
					for (j = 0; j < number_of_ensembles; j++)
					{
						if ((i != j) && ((index->indexRefLimit(j) < refSize[j]) ||
							(!index->isDenseOnEnsemble(j))))
						{
							clearDense = true;
							break;
						}
					}
				}
				else if (index->isDenseOnEnsembleAbove(i, refSize[i] - 1))
				{
					for (j = 0; j < number_of_ensembles; j++)
					{
						if ((i != j) && ((index->indexRefLimit(j) != refSize[j]) ||
							(!index->isDenseOnEnsemble(j))))
						{
							clearDense = true;
							break;
						}
					}					
				}
				else
				{
					clearDense = true;
				}
			}
		}
	}

	if (clearDense)
	{
		if (!setNotDense())
			return 0;
	}

	if (resizeInnerEnsemble)
	{
		EnsembleEntryRef *newRefSize = new EnsembleEntryRef[number_of_ensembles];
		if (!newRefSize)
			return 0;
		for (i = 0; i < number_of_ensembles; i++)
		{
			newRefSize[i] = refSize[i];
			// first ensemble: never resize since it can freely grow
			// inner ensembles: resize to MAX(refSize, indexRefLimit)
			// GRC in some cases it may be better to go straight to ensemble->maxRef
			if ((i > 0) && (index->indexRefLimit(i) > newRefSize[i]))
				newRefSize[i] = index->indexRefLimit(i);
		}
		if (!resize(newRefSize))
			return 0;
		delete[] newRefSize;
	}
	if (0 < number_of_ensembles)
	{
		EnsembleEntryRef newRefSize0 = index->indexRefLimit(0);
		if (newRefSize0 > refSize[0])
			refSize[0] = newRefSize0;
	}

	// iterate to set values in identifier order for each ensemble
	ParameterIndexType valueIndex = 0;
	unsigned int value_number;

	if (!index->iterationBegin())
	{
		display_message(ERROR_MESSAGE,
			"Field_parameters::setValues  Failed to begin iteration over index for field %s\n",
			field->name);
		return 0;
	}
	bool oldValue;
	int return_code = 1;
	bool iterResult = true;
	for (value_number = 0; iterResult && (value_number < number_of_values); value_number++)
	{
		valueIndex = 0;
		for (i = 0; i < number_of_ensembles; i++)
		{
			valueIndex += index->iterationRef(i)*offsets[i];
		}
		if (!values.setValue(valueIndex, inValues[value_number]))
		{
			display_message(ERROR_MESSAGE,
				"Field_parameters::setValues  Failed to set parameter value for field %s\n",
				field->name);
			return_code = 0;
			break;
		}
		if (!dense)
		{
			if (!value_exists.setBool(valueIndex, true, oldValue))
			{
				display_message(ERROR_MESSAGE,
					"Field_parameters::setValues  Failed to set parameter exists flag for field %s\n",
					field->name);
				return_code = 0;
				break;
			}
		}
		iterResult = index->iterationNext();
	}
	index->iterationEnd();

	if (return_code && (value_number < number_of_values))
	{
		display_message(ERROR_MESSAGE,
			"Field_parameters::setValues  Only %u out of %u values iterated for field %s\n",
			value_number, number_of_values, field->name);
		return_code = 0;
	}
	else if (iterResult && (value_number >= number_of_values))
	{
		display_message(ERROR_MESSAGE,
			"Field_parameters::setValues  Iteration past end of values for field %s\n",
			field->name);
		return_code = 0;
	}

	if (return_code)
	{
		// GRC to do: send manager change message
	}

	return return_code;
}

char field_real_parameters_type_string[] = "real_parameters";

class Field_real_parameters : public Field_parameters<double>
{
public:
	Field_real_parameters(int in_number_of_ensembles, Field_ensemble **in_ensembles) :
		Field_parameters<double>(in_number_of_ensembles, in_ensembles)
	{
	}

	const char *get_type_string()
	{
		return (field_real_parameters_type_string);
	}

};

char field_integer_parameters_type_string[] = "integer_parameters";

class Field_integer_parameters : public Field_parameters<int>
{
public:
	Field_integer_parameters(int in_number_of_ensembles, Field_ensemble **in_ensembles) :
		Field_parameters<int>(in_number_of_ensembles, in_ensembles)
	{
	}

	const char *get_type_string()
	{
		return (field_integer_parameters_type_string);
	}

};

} // namespace Cmiss

/* GRC note defaults to 1 component */
Cmiss_field *Cmiss_field_module_create_real_parameters(
	Cmiss_field_module *field_module, 
	int number_of_index_ensembles, Cmiss_field_ensemble **index_ensemble_fields)
{
	Cmiss_field *field = NULL;
	if ((0 == number_of_index_ensembles) || (index_ensemble_fields))
	{
		// check all ensembles are present and none are repeated
		for (int i = 0; i < number_of_index_ensembles; i++)
		{
			if (NULL == index_ensemble_fields[i])
			{
				display_message(ERROR_MESSAGE, "Cmiss_field_module_create_real_parameters.  Missing ensemble");
				return NULL;
			}
			for (int j = i + 1; j < number_of_index_ensembles; j++)
			{
				if (index_ensemble_fields[j] == index_ensemble_fields[i])
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_field_module_create_real_parameters.  Repeated ensemble '%s'",
						reinterpret_cast<Cmiss_field*>(index_ensemble_fields[i])->name);
					return NULL;
				}
			}
		}
		Cmiss::Field_ensemble **index_ensembles =
			new Cmiss::Field_ensemble *[number_of_index_ensembles];
		for (int i = 0; i < number_of_index_ensembles; i++)
		{
			index_ensembles[i] = Cmiss_field_ensemble_core_cast(index_ensemble_fields[i]);
		}
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			/*number_of_components*/1,
			/*number_of_source_fields*/number_of_index_ensembles,
				reinterpret_cast<Cmiss_field **>(index_ensemble_fields),
			/*number_of_source_values*/0, NULL,
			new Cmiss::Field_real_parameters(number_of_index_ensembles, index_ensembles));
		delete[] index_ensembles;
	}
	return (field);
}

inline Cmiss::Field_real_parameters *Cmiss_field_real_parameters_core_cast(
	Cmiss_field_real_parameters *real_parameters_field)
{
	return (static_cast<Cmiss::Field_real_parameters*>(
		reinterpret_cast<Cmiss_field*>(real_parameters_field)->core));
}

Cmiss_field_real_parameters *Cmiss_field_cast_real_parameters(Cmiss_field* field)
{
	if (field && (dynamic_cast<Cmiss::Field_real_parameters*>(field->core)))
	{
		Cmiss_field_access(field);
		return (reinterpret_cast<Cmiss_field_real_parameters *>(field));
	}
	else
	{
		return (NULL);
	}
}

int Cmiss_field_real_parameters_destroy(
	Cmiss_field_real_parameters_id *real_parameters_field_address)
{
	return Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id *>(real_parameters_field_address));
}

Cmiss_ensemble_index *Cmiss_field_real_parameters_create_index(
	Cmiss_field_real_parameters *real_parameters_field)
{
	if (NULL == real_parameters_field)
		return NULL;
	return Cmiss_field_real_parameters_core_cast(real_parameters_field)->createIndex();
}

int Cmiss_field_real_parameters_get_values(
	Cmiss_field_real_parameters *real_parameters_field,
	Cmiss_ensemble_index *index, unsigned int number_of_values, double *values)
{
	if ((NULL == real_parameters_field) || (NULL == index) ||
			(number_of_values == 0) || (NULL == values))
		return 0;
	return Cmiss_field_real_parameters_core_cast(real_parameters_field)->
		getValues(index, number_of_values, values);
}

int Cmiss_field_real_parameters_get_values_sparse(
	Cmiss_field_real_parameters_id real_parameters_field,
	Cmiss_ensemble_index_id index, unsigned int number_of_values, double *values,
	int *value_exists, int *number_of_values_read)
{
	if ((NULL == real_parameters_field) || (NULL == index) ||
			(number_of_values == 0) || (NULL == values) || (NULL == value_exists) ||
			(NULL == number_of_values_read))
		return 0;
	return Cmiss_field_real_parameters_core_cast(real_parameters_field)->
		getValuesSparse(index, number_of_values, values, value_exists, number_of_values_read);
}

/* unsigned int may eventually be too small for number_of_values.
 * Workaround: Clients with big data can always set in blocks
 * Note: npruntime only handles 32-bit integers so an issue for API
 * Could pass in two integers */
int Cmiss_field_real_parameters_set_values(
	Cmiss_field_real_parameters *real_parameters_field,
	Cmiss_ensemble_index *index, unsigned int number_of_values, double *values)
{
	if ((NULL == real_parameters_field) || (NULL == index) ||
			(number_of_values == 0) || (NULL == values))
		return 0;
	return Cmiss_field_real_parameters_core_cast(real_parameters_field)->
		setValues(index, number_of_values, values);
}


/* GRC note defaults to 1 component */
Cmiss_field *Cmiss_field_module_create_integer_parameters(
	Cmiss_field_module *field_module,
	int number_of_index_ensembles, Cmiss_field_ensemble **index_ensemble_fields)
{
	Cmiss_field *field = NULL;
	if ((0 == number_of_index_ensembles) || (index_ensemble_fields))
	{
		// check all ensembles are present and none are repeated
		for (int i = 0; i < number_of_index_ensembles; i++)
		{
			if (NULL == index_ensemble_fields[i])
			{
				display_message(ERROR_MESSAGE, "Cmiss_field_module_create_integer_parameters.  Missing ensemble");
				return NULL;
			}
			for (int j = i + 1; j < number_of_index_ensembles; j++)
			{
				if (index_ensemble_fields[j] == index_ensemble_fields[i])
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_field_module_create_integer_parameters.  Repeated ensemble '%s'",
						reinterpret_cast<Cmiss_field*>(index_ensemble_fields[i])->name);
					return NULL;
				}
			}
		}
		Cmiss::Field_ensemble **index_ensembles =
			new Cmiss::Field_ensemble *[number_of_index_ensembles];
		for (int i = 0; i < number_of_index_ensembles; i++)
		{
			index_ensembles[i] = Cmiss_field_ensemble_core_cast(index_ensemble_fields[i]);
		}
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			/*number_of_components*/1,
			/*number_of_source_fields*/number_of_index_ensembles,
				reinterpret_cast<Cmiss_field **>(index_ensemble_fields),
			/*number_of_source_values*/0, NULL,
			new Cmiss::Field_integer_parameters(number_of_index_ensembles, index_ensembles));
		delete[] index_ensembles;
	}
	return (field);
}

inline Cmiss::Field_integer_parameters *Cmiss_field_integer_parameters_core_cast(
	Cmiss_field_integer_parameters *integer_parameters_field)
{
	return (static_cast<Cmiss::Field_integer_parameters*>(
		reinterpret_cast<Cmiss_field*>(integer_parameters_field)->core));
}

Cmiss_field_integer_parameters *Cmiss_field_cast_integer_parameters(Cmiss_field* field)
{
	if (field && (dynamic_cast<Cmiss::Field_integer_parameters*>(field->core)))
	{
		Cmiss_field_access(field);
		return (reinterpret_cast<Cmiss_field_integer_parameters *>(field));
	}
	else
	{
		return (NULL);
	}
}

int Cmiss_field_integer_parameters_destroy(
	Cmiss_field_integer_parameters_id *integer_parameters_field_address)
{
	return Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id *>(integer_parameters_field_address));
}

Cmiss_ensemble_index *Cmiss_field_integer_parameters_create_index(
	Cmiss_field_integer_parameters *integer_parameters_field)
{
	if (NULL == integer_parameters_field)
		return NULL;
	return Cmiss_field_integer_parameters_core_cast(integer_parameters_field)->createIndex();
}

int Cmiss_field_integer_parameters_get_values(
	Cmiss_field_integer_parameters *integer_parameters_field,
	Cmiss_ensemble_index *index, unsigned int number_of_values, int *values)
{
	if ((NULL == integer_parameters_field) || (NULL == index) ||
			(number_of_values == 0) || (NULL == values))
		return 0;
	return Cmiss_field_integer_parameters_core_cast(integer_parameters_field)->
		getValues(index, number_of_values, values);
}

/* unsigned int may eventually be too small for number_of_values.
 * Workaround: Clients with big data can always set in blocks
 * Note: npruntime only handles 32-bit integers so an issue for API
 * Could pass in two integers */
int Cmiss_field_integer_parameters_set_values(
	Cmiss_field_integer_parameters *integer_parameters_field,
	Cmiss_ensemble_index *index, unsigned int number_of_values, int *values)
{
	if ((NULL == integer_parameters_field) || (NULL == index) ||
			(number_of_values == 0) || (NULL == values))
		return 0;
	return Cmiss_field_integer_parameters_core_cast(integer_parameters_field)->
		setValues(index, number_of_values, values);
}
