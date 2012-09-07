/***************************************************************************//**
 * FILE : block_array.hpp
 * 
 * Implements an array container allocated in blocks.
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
				return NULL;
			for (IndexType i = blockCount; i < newBlockCount; i++)
			{
				newBlocks[i] = NULL;
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
		blocks(NULL),
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
		if (!setValues(0, intIndexCount-1, 0xFFFFFFFF))
			return false;
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
