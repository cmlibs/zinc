/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __ZINCTEST_UTILITIES_FILEIO_HPP__
#define __ZINCTEST_UTILITIES_FILEIO_HPP__

#include <string>

// Ensures folder of supplied name exists for lifetime of object
// Note doesn't yet clear folder.
class ManageOutputFolder
{
	std::string folderName;
public:
	ManageOutputFolder(const char *folderNameIn);
	~ManageOutputFolder();
};

/** Open and read file into string buffer.
 * @param filename  The name of the file to read.
 * @return  On success, allocated zero-terminated C string buffer, or
 * nullptr if failed. Caller must pass it to free() to deallocate.
 */
char *readFileToString(const char *filename);

#endif // __ZINCTEST_UTILITIES_FILEIO_HPP__
