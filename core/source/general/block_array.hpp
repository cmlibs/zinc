/***************************************************************************//**
 * FILE : block_array.hpp
 * 
 * Implements an array container allocated in blocks.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (BLOCK_ARRAY_HPP)
#define BLOCK_ARRAY_HPP

#include "general/debug.h"

template <typename IndexType, typename EntryType, IndexType defaultBlockLength = 256 >
	class block_array
{
private:
	// Note: any new attributes must be handled by swap()
	EntryType **blocks;
	IndexType blockCount;
	IndexType blockLength;

	EntryType* getOrCreateBlock(IndexType blockIndex)
	{
		if (blockIndex >= blockCount)
		{
			IndexType newBlockCount = blockIndex + 1;
			if (newBlockCount < blockCount*2)
			{
				newBlockCount = blockCount*2;
			}
			EntryType **newBlocks;
			if (!REALLOCATE(newBlocks, blocks, EntryType *, newBlockCount))
				return 0;
			for (IndexType i = blockCount; i < newBlockCount; i++)
			{
				newBlocks[i] = 0;
			}
			blocks = newBlocks;
			blockCount = newBlockCount;
		}
		EntryType *block = blocks[blockIndex];
		if (!block)
		{
			if (ALLOCATE(block, EntryType, blockLength))
			{
				for (IndexType i = 0; i < blockLength; i++)
				{
					block[i] = 0; // only works for numeric or pointer types
				}
				blocks[blockIndex] = block;
			}
		}
		return block;
	}

public:
	
	block_array(IndexType blockLengthIn = defaultBlockLength) :
		blocks(0),
		blockCount(0),
		blockLength(blockLengthIn)
	{
	}

	~block_array()
	{
		clear();
	}

	void clear()
	{
		for (IndexType i = 0; i < blockCount; i++)
		{
			if (blocks[i])
			{
				DEALLOCATE(blocks[i]);
			}
		}
		if (blocks)
		{
			DEALLOCATE(blocks);
		}
		blockCount = 0;
	}

	IndexType getBlockCount() const
	{
		return this->blockCount;
	}

	IndexType getBlockLength() const
	{
		return this->blockLength;
	}

	/** Swaps all data with other block_array. Cannot fail. */
	void swap(block_array& other)
	{
		EntryType **temp_blocks = this->blocks;
		IndexType temp_blockCount = this->blockCount;
		IndexType temp_blockLength = this->blockLength;
		this->blocks = other.blocks;
		this->blockCount = other.blockCount;
		this->blockLength = other.blockLength;
		other.blocks = temp_blocks;
		other.blockCount = temp_blockCount;
		other.blockLength = temp_blockLength;
	}

	/**
	 * @param index  The index of the address to retrieve, starting at 0.
	 * @return  The address for value for given index, or 0 if none.
	 */
	EntryType *getAddress(IndexType index) const
	{
		IndexType blockIndex = index / blockLength;
		if (blockIndex < blockCount)
		{
			EntryType *block = blocks[blockIndex];
			if (block)
				return block + (index % blockLength);
		}
		return 0;
	}

	/**
	 * Iff no block exists for index, create and initialise it as described below
	 * then return the address of value for index as for getAddress(). Caller
	 * is expected to have called and obtained 0 for getAddress() first.
	 * @param index  The index of the address to create.
	 * @param initValue  Value to initialise entries in block to.
	 * @param initIndexSpacing  Spacing of value to be initialised; 0 or negative to
	 * not initialise values, 1 to set all, any other value is amount offset from 0 to
	 * blockLength to set to initValue, e.g. 10 initialises:
	 * 0 10 20 ... blockLength (truncated to nearest multiple of 10)
	 * @return  The address for value for given index, or 0 if already exists of failed.
	 */
	EntryType *createAddressInitialise(IndexType index, EntryType initValue = 0, IndexType initIndexSpacing = 0)
	{
		IndexType blockIndex = index / blockLength;
		if (blockIndex < blockCount)
		{
			EntryType *block = blocks[blockIndex];
			if (!block)
			{
				block = getOrCreateBlock(blockIndex);
				if (block)
				{
					if (initIndexSpacing > 0)
					{
						for (IndexType i = 0; i < blockLength; i += initIndexSpacing)
							block[i] = initValue;
					}
					return block + (index % blockLength);
				}
			}
		}
		return 0;
	}

	/**
	 * Get a value from the block_array.
	 * @param index  The index of the value to retrieve, starting at 0.
	 * @param value  On success, filled with value held at index.
	 * @return  Boolean true if value returned, false if no value at index.
	 */
	bool getValue(IndexType index, EntryType& value) const
	{
		IndexType blockIndex = index / blockLength;
		if (blockIndex < blockCount)
		{
			EntryType *block = blocks[blockIndex];
			if (block)
			{
				IndexType entryIndex = index % blockLength;
				value = block[entryIndex];
				return true;
			}
		}
		return false;
	}

	/**
	 * Set a value in the block_array.
	 * @param index  The index of the value to set, starting at 0.
	 * @param value  Value to set at index.
	 * @return  Boolean true on success, false on failure.
	 */
	bool setValue(IndexType index, EntryType value)
	{
		IndexType blockIndex = index / blockLength;
		EntryType* block = getOrCreateBlock(blockIndex);
		if (!block)
			return false;
		IndexType entryIndex = index % blockLength;
		block[entryIndex] = value;
		return true;
	}

	bool setValues(IndexType minIndex, IndexType maxIndex, EntryType value)
	{
		// GRC: can be made faster
		for (IndexType index = minIndex; index <= maxIndex; index++)
		{
			if (!setValue(index, value))
				return false;
		}
		return true;
	}
	
};

/** stores boolean values as individual bits, with no value equivalent to false */
template <typename IndexType, IndexType intBlockLength = 32>
	class bool_array : private block_array<IndexType, unsigned int, intBlockLength>
{
public:
	void clear()
	{
		block_array<IndexType, unsigned int, intBlockLength>::clear();
	}

	void swap(bool_array& other)
	{
		block_array<IndexType, unsigned int, intBlockLength>::swap(other);
	}

	using block_array<IndexType, unsigned int, intBlockLength>::getBlockCount;
	using block_array<IndexType, unsigned int, intBlockLength>::getBlockLength;
	using block_array<IndexType, unsigned int, intBlockLength>::getValue;
	using block_array<IndexType, unsigned int, intBlockLength>::setValue;
	using block_array<IndexType, unsigned int, intBlockLength>::setValues;

	/** @param oldValue  Returns old value so client can determine if status changed */
	bool setBool(IndexType index, bool value, bool& oldValue)
	{
		IndexType intIndex = index >> 5;
		unsigned int intValue = 0;
		bool hasValue = getValue(intIndex, intValue);
		if (hasValue || value)
		{
			unsigned int mask = (1 << (index & 0x1F));
			oldValue = (0 != (intValue & mask));
			if (oldValue != value)
			{
				return setValue(intIndex, intValue ^ mask);
			}
		}
		else
		{
			oldValue = false;
		}
		return true;
	}

	bool getBool(IndexType index) const
	{
		IndexType intIndex = index >> 5;
		unsigned int intValue;
		if (getValue(intIndex, intValue))
		{
			unsigned int mask = (1 << (index & 0x1F));
			return (0 != (intValue & mask));
		}
		return false;
	}

	/**
	 * Advance index while bool array value is false.
	 * Efficiently skips whole blocks, and 32-bit zeroes within blocks.
	 * @param index  The index to advance while bool value is false.
	 * @param limit  One past the last index to check.
	 * @return  True if index found, false if reached limit.
	 */
	bool advanceIndexWhileFalse(IndexType& index, IndexType limit)
	{
		const IndexType blockIndexSize = this->getBlockLength()*32;
		const IndexType blockLimit = this->getBlockCount()*blockIndexSize;
		const IndexType useLimit = (limit < blockLimit) ? limit : blockLimit;
		while (index < useLimit)
		{
			const IndexType intIndex = index >> 5;
			unsigned int intValue;
			if (!getValue(intIndex, intValue))
				index = ((index / blockIndexSize) + 1)*blockIndexSize; // advance to next block
			else
			{
				unsigned int mask = (1 << (index & 0x1F));
				if (intValue < mask)
					index = (intIndex + 1) << 5; // advance to next int index
				else
				{
					do
					{
						if (intValue & mask)
							return true;
						++index;
						mask <<= 1;
					}
					while (index & 0x1F);
				}
			}
		}
		return false;
	}

	/**
	 * @param lastTrueIndex  Updated to equal or next lower index with true value.  
	 * @return  true if found, false if none.
	 */
	bool updateLastTrueIndex(IndexType& lastTrueIndex)
	{
		// GRC this can be made much more efficient
		while (!getBool(lastTrueIndex))
		{
			--lastTrueIndex;
			if (lastTrueIndex < 0)
				return false;
		}
		return true;
	}

	/** @return  true if values for all indexes in range are true; false otherwise */
	bool isRangeTrue(IndexType minIndex, IndexType maxIndex)
	{
		if (minIndex > maxIndex)
			return false;
		// GRC this can be made much more efficient
		IndexType index = minIndex;
		while (getBool(index))
		{
			if (index == maxIndex)
				return true;
			index++;
		}
		return false;
	}

	/** Sets all entries from index 0..indexCount-1 to true.
	 * @return  true if completely successful, false otherwise */
	bool setAllTrue(IndexType indexCount)
	{
		IndexType intIndexCount = indexCount >> 5;
		// bulk set the flags in lots of 32 bits
		if (intIndexCount > 0)
		{
			if (!setValues(0, intIndexCount-1, 0xFFFFFFFF))
				return false;
		}
		// individually set remaining bits
		for (IndexType index = intIndexCount*32; index < indexCount; index++)
		{
			bool oldValue;
			if (!setBool(index, true, oldValue))
				return false;
		}
		return true;
	}

	/**
	 * @return  true if all bits in bool array are either all on or all off over
	 * all consecutive subarrays of the given size, otherwise false. Used to
	 * determine sparsity of map indexing.
	 */
	bool isBanded(IndexType bandSize, IndexType numberOfBands)
	{
		for (IndexType band = 0; band < numberOfBands; ++band)
		{
			// GRC this can be made much more efficient
			IndexType index = band*bandSize;
			bool firstBool = getBool(index);
			IndexType indexLimit = (band + 1)*bandSize;
			for (++index; index < indexLimit; ++index)
				if (getBool(index) != firstBool)
					return false;
		}
		return true;
	}

};

#endif /* !defined (BLOCK_ARRAY_HPP) */
