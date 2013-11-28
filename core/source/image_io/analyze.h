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
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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
#ifndef IMAGE_IO_ANALYZE_H_
#define IMAGE_IO_ANALYZE_H_

#include "zinc/zincconfigure.h"

#include "analyze_header.h"

struct Cmgui_image_information;
struct Cmgui_image;

class AnalyzeImageHandler
{
public:
	explicit AnalyzeImageHandler(const char *filename);
	~AnalyzeImageHandler();

	bool readHeader();
	void readImageData();
	int getNumberOfDimensions();
	int getWidth();
	int getHeight();
	int getDepth();
	int getComponentDepth();
	int getNumberOfComponents();
	struct Cmgui_image_information_memory_block getImageMemoryBlock(int index);
	int getGlMax() const;
	int getGlMin() const;
	const char *getQuantumFormat() const;
	bool isBigEndian() const { return bigEndian; }

	/**
	 * @brief getOrientation
	 * Get the orientation of the data as an integer:
	 *  0 for transverse
	 *  1 for coronal
	 *  2 for sagittal
	 *  -1 for any other
	 *
	 * @return orientation value as an int
	 */
	int getOrientation() const;

private:
	void setFileEndian();
	int determineDimensions();
	bool testFileEndianSystemEndianMatch();
	void swapBytesIfEndianessDifferent();

	const char *filename;
	bool bigEndian;
	int numberOfDimensions;
	int width;
	int height;
	struct dsr hdr;
	void *data;
};

/**
 * Creates a Cmgui_image containing the images listed in the image information.
 * Specific function for the Analyze format.
 *
 * @param cmgui_image_information
 * @return A Cmgui_image structure if successful, 0 otherwise.
 */
struct Cmgui_image *Cmgui_image_read_analyze(
	struct Cmgui_image_information *cmgui_image_information);

#endif /* IMAGE_IO_ANALYZE_H_ */
