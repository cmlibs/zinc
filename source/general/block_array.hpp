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


template <typename IndexType, typename EntryType, int blockLength = 1024, EntryType blankEntry = 0 >
	class block_array
{
private:
	class Block
	{
		EntryType *entries;

	public:
		Block() :
			entries(NULL)
		{
		}

		~Block()
		{
			if (entries)
				DEALLOCATE(entries);
		}

		EntryType *createEntries()
		{
			if (NULL == entries)
			{
				ALLOCATE(entries, EntryType, blockLength);
				for (int i = 0; i < blockLength; i++)
				{
					entries[i] = blankEntry;
				}
			}
			return (entries);
		}

		int hasEntries()
		{
			return (NULL != entries);
		}
		
		int setValue(IndexType entryIndex, EntryType value)
		{
#if ! defined (OPTIMISED)
			if (entryIndex >= blockLength)
				exit(EXIT_FAILURE);
#endif /* ! defined (OPTIMISED) */
			if (createEntries())
			{
				entries[entryIndex] = value;
				return 1;
			}
			return 0;
		}

		int getValue(IndexType entryIndex, EntryType& value)
		{
#if ! defined (OPTIMISED)
			if (entryIndex >= blockLength)
				exit(EXIT_FAILURE);
#endif /* ! defined (OPTIMISED) */
			if (entries)
			{
				value = entries[entryIndex];
				return 1;
			}
			return 0;
		}
	};

	
private:
	
	Block *blocks;
	IndexType blockCount;

	Block* getBlock(IndexType blockIndex)
	{
		if (blockIndex < blockCount)
			return (blocks + blockIndex);
		IndexType newBlockCount = blockIndex + 1;
		if (newBlockCount < blockCount*2)
		{
			newBlockCount = blockCount*2;
		}
		Block *newBlocks = new Block[newBlockCount];
		if (NULL == newBlocks)
			return NULL;
		memcpy(newBlocks, blocks, blockCount*sizeof(Block));
		delete[] blocks;
		blocks = newBlocks;
		blockCount = newBlockCount;
		return (blocks + blockIndex);
	}

public:
	
	block_array() :
		blocks(NULL),
		blockCount(0)
	{
	}

	~block_array()
	{
		delete[] blocks;
		blockCount = 0;
	}

	/**
	 * Get a value from the block_array
	 * @param index  The index of the value to retrieve, starting at 0.
	 * @param value  On success, filled with Entry at index
	 * @return  1 if value returned, 0 if no value at index.
	 */
	int getValue(IndexType index, EntryType& value)
	{
		IndexType blockIndex = index / blockLength;
		Block *block = getBlock(blockIndex);
		if (!block)
			return 0;
		IndexType entryIndex = index % blockLength;
		return block->getValue(entryIndex, value);		
	}

	/**
	 * Set a value in the block_array
	 * @param index  The index of the value to set, starting at 0.
	 * @param value  Value to set at index
	 * @return  1 one success, 0 on failure.
	 */
	int setValue(IndexType index, EntryType value)
	{
		IndexType blockIndex = index / blockLength;
		Block *block = getBlock(blockIndex);
		if (!block)
			return 0;
		IndexType entryIndex = index % blockLength;
		return block->setValue(entryIndex, value);
	}

};

#endif /* !defined (BLOCK_ARRAY_HPP) */
