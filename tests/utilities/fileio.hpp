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

#endif // __ZINCTEST_UTILITIES_FILEIO_HPP__
