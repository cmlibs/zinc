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
#ifndef OPENCASCADEIMPORTER_H
#define OPENCASCADEIMPORTER_H

#include <TopoDS_Shape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <Handle_TopTools_HSequenceOfShape.hxx>
#include <Handle_TDocStd_Document.hxx>
#include <Handle_XCAFDoc_ShapeTool.hxx>
#include <Handle_XCAFDoc_ColorTool.hxx>
#include <Quantity_Color.hxx>
#include <TDF_Label.hxx>

#include <vector>

#include "cad/entity.h"
#include "cad/point.h"
#include "cad/curve.h"
#include "cad/surface.h"
#include "cad/geometricshape.h"
#include "cad/topologicalshape.h"
#include "cad/topologicalshaperoot.h"
#include "cad/cadimporter.h"

/**
 * A class to facilitate importing data from files via OpenCASCADE into a region.
 */
class OpenCascadeImporter : public CADImporter
{
public:
	/**
	 * Create an OpenCASCADE importer, optionally given a file name.
	 * @param fileName a file name to import data from, optionally set here.
	 */
	OpenCascadeImporter( const char *fileName = "" );
	~OpenCascadeImporter();

	/**
	 * Import the file passed to the constructor or set with fileName(const char *name)
	 * into the given region.
	 * @param the region to import the data from the file into
	 * @returns true if successful, false otherwise
	 * @see #fileName(const char *name)
	 */
	bool import( Cmiss_region_id region );

private:
	/**
	 * Convert an OpenCASCADE Extended Data Exchange (XDE) document into regions and fields.
	 * An XDE document is internally managed as a DOM document, there exists a limited API to
	 * query, analyse and extract information contained in the document.  New version using
	 * shape colour search.
	 * @param xdeDoc a handle to a valid XDE document
	 * @param region the region to create cad defined regions and fields into
	 * @returns true if successful, false otherwise
	 */
	bool convertDocToRegionsAndFields( Handle_TDocStd_Document xdeDoc, Cmiss_region_id region );
	/**
	 * Add a computed field cad topology to the given region.  Here we also add the geometric representation
	 * of the shape attached to the given label, for displaying, and also we create a colour representation 
	 * for the shape (if available) so that the shape can be visualised as expected.
	 * @param xdeDoc a handle to a valid XDE document
	 * @param label the current document label
	 * @param location the current location
	 * @param parent the region to create cad defined regions and fields into
	 * @returns true if successful, false otherwise
	 */
	bool addShapeToRegion( Handle_TDocStd_Document xdeDoc, const TDF_Label& label, TopLoc_Location location , Cmiss_region_id parent );
	/**
	 * Traverse the labels in an XDE document.  This is necessary when dealing with assemblies, an assembly doesn't contain any shape 
	 * information it is only a container for other assemblies or shapes.
	 * @param xdeDoc a handle to a valid XDE document
	 * @param label the current document label
	 * @param location the current location
	 * @param parent the region to create cad defined regions and fields into
	 * @returns true if successful, false otherwise
	 */
	bool labelTraversal(Handle_TDocStd_Document xdeDoc, const TDF_Label& label, TopLoc_Location location, Cmiss_region_id parent );

	/**
	 * Get the label name if it exists, this may return an empty string ""
	 * @param label the label to get name attribute from
	 * @returns a char * that points to the name that has been allocated, must be freed
	 */
	char *getLabelName(const TDF_Label& label);
};

#endif
