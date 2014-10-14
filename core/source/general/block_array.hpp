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

template <typename IndexType, typename EntryType, int blockLength = 256 >
	class block_array
{
private:
	// Note: any new attributes must be handled by swap()
	EntryType **blocks;
	IndexType blockCount;

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
	
	block_array() :
		blocks(0),
		blockCount(0)
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

	/** Swaps all data with other block_array. Cannot fail. */
	void swap(block_array& other)
	{
		EntryType **temp_blocks = blocks;
		IndexType temp_blockCount = blockCount;
		blocks = other.blocks;
		blockCount = other.blockCount;
		other.blocks = temp_blocks;
		other.blockCount = temp_blockCount;
	}

	/**
	 * Get a value from the block_array.
	 * @param index  The index of the value to retrieve, starting at 0.
	 * @param value  On success, filled with value held at index.
	 * @return  1 if value returned, 0 if no value at index.
	 */
	int getValue(IndexType index, EntryType& value) const
	{
		IndexType blockIndex = index / blockLength;
		if (blockIndex < blockCount)
		{
			EntryType *block = blocks[blockIndex];
			if (block)
			{
				IndexType entryIndex = index % blockLength;
				value = block[entryIndex];
				return 1;
			}
		}
		return 0;
	}

	/**
	 * Set a value in the block_array.
	 * @param index  The index of the value to set, starting at 0.
	 * @param value  Value to set at index.
	 * @return  1 if value set, 0 if failed.
	 */
	int setValue(IndexType index, EntryType value)
	{
		IndexType blockIndex = index / blockLength;
		EntryType* block = getOrCreateBlock(blockIndex);
		if (!block)
			return 0;
		IndexType entryIndex = index % blockLength;
		block[entryIndex] = value;
		return 1;
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
template <typename IndexType, int intBlockLength = 32>
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

	using block_array<IndexType, unsigned int, intBlockLength>::getValue;
	using block_array<IndexType, unsigned int, intBlockLength>::setValue;
	using block_array<IndexType, unsigned int, intBlockLength>::setValues;

	/** @param oldValue  Returns old value so client can determine if status changed */
	int setBool(IndexType index, bool value, bool& oldValue)
	{
		IndexType intIndex = index >> 5;
		unsigned int intValue = 0;
		int hasValue = getValue(intIndex, intValue);
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
		return 1;
	}

	bool getBool(IndexType index) const
	{
		IndexType intIndex = index >> 5;
		unsigned int intValue = 0;
		if (getValue(intIndex, intValue))
		{
			unsigned int mask = (1 << (index & 0x1F));
			return (0 != (intValue & mask));
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
};

#endif /* !defined (BLOCK_ARRAY_HPP) */
