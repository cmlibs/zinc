/**
 * FILE : datastore/maparray.hpp
 * 
 * Template implementing a map from a single datastore labels set to fixed size
 * array of given type.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (CMZN_DATASTORE_MAPARRAY_HPP)
#define CMZN_DATASTORE_MAPARRAY_HPP

#include "datastore/labels.hpp"
#include "general/block_array.hpp"

// First value in array is initialised to UnallocatedValue if not allocated,
// any other value if it is in use. Hence this cannot be a valid value for the
// supplied ValueType.
// InvalidValue is not used yet.
// IndexType is normally DsLabelIndex, but must be raised to 64-bit integer
// once arraySize*number-of-lables exceeds 2^31.
template <typename IndexType, typename ValueType, ValueType UnallocatedValue, ValueType InvalidValue>
class DsMapArray
{
	DsLabels *labels;
	const IndexType arraySize;
	const IndexType arraysPerBlock;
	block_array<IndexType, ValueType> values;

	DsMapArray(const DsMapArray&); // not implemented
	DsMapArray& operator=(const DsMapArray&); // not implemented

public:

	DsMapArray(DsLabels *labelsIn, DsLabelIndex arraySizeIn) :
		labels(Access(labelsIn)),
		// array blocks must be multiple of arraySize
		// use exact size if large enough to prevent wastage when sparse
		arraySize(static_cast<IndexType>(arraySizeIn)),
		arraysPerBlock(arraySizeIn >= 32 ? 1 : (256 / arraySizeIn)),
		values(arraysPerBlock*arraySizeIn)
	{
	}

	virtual ~DsMapArray()
	{
		Deaccess(this->labels);
	}

	/**
	 * @return address of array for the given labels index, or 0 if unallocated.
	 */
	ValueType *getArray(DsLabelIndex index) const
	{
		IndexType arrayIndex = index*this->arraySize;
		ValueType *array = this->values.getAddress(arrayIndex);
		if (array && (*array != UnallocatedValue))
			return array;
		return 0;
	}

	// caller must get 0 return for getArray first
	// caller must initialise contents of array, particularly setting first
	// element to anything other than UnallocatedValue
	ValueType *createArray(DsLabelIndex index)
	{
		IndexType arrayIndex = index*this->arraySize;
		return this->values.createAddressInitialise(arrayIndex, UnallocatedValue, this->arraySize);
	}

};

typedef DsMapArray<DsLabelIndex, DsLabelIndex, DS_LABEL_INDEX_UNALLOCATED, DS_LABEL_INDEX_INVALID> DsMapArrayLabelIndex;

#endif // CMZN_DATASTORE_MAPARRAY_HPP
