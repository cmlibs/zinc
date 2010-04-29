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
	@author user <hsorby@eggzachary>
*/
class OpenCascadeImporter : public CADImporter
{
public:
	OpenCascadeImporter( const char *fileName = "" );
	~OpenCascadeImporter();

	bool import( struct Cmiss_region *region );
	bool import();

	void buildGeometricShapes();
	const std::vector<GeometricShape*>& geometricShapes() const { return m_geometricShapes; }

private:
	// Getting topology related functions
	void mapShapes( Handle_TopTools_HSequenceOfShape shapes );
	bool RemapDocToRegionsAndFields( Handle_TDocStd_Document doc, struct Cmiss_region *region );
	bool createGeometricShapes( Handle_TDocStd_Document doc );
	void mapShape( TopoDS_Shape shape );
	bool addShapeToRegion( const TDF_Label& label, const TopLoc_Location& aLocation, struct Cmiss_region *parent );
	bool addGeometricShapeToList( const TDF_Label& label, const TopLoc_Location& aLocation );
	bool labelTraversal(const TDF_Label& aLabel, const TopLoc_Location& aLocation, struct Cmiss_region *parent );
	bool labelTraversal(const TDF_Label& aLabel, const TopLoc_Location& aLocation );

	// Getting geometry related functions
	void healGeometry( TopoDS_Shape shape, double tolerance, bool fixsmalledges,
		bool fixspotstripfaces, bool sewfaces,
		bool makesolids = false);
	void clearMaps();
	void recurseTopologicalShapesAndBuildGeometricShapes( TopologicalShape* shape );
	void buildMaps( const TopoDS_Shape& shape );
	void extractPoints( GeometricShape* geoShape );
	void extractCurvesAndSurfaces( GeometricShape* geoShape );

private:
	std::vector<GeometricShape*> m_geometricShapes;

	TopologicalShapeRoot m_rootShape;

	TopTools_IndexedMapOfShape m_cmap, m_csmap, m_somap, m_shmap, m_fmap, m_emap, m_vmap;
	Handle_TDocStd_Document m_doc;

};

#endif
