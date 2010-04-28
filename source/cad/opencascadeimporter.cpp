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
#include "opencascadeimporter.h"

#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <Standard_ConstructionError.hxx>
#include <gp_Pnt.hxx>
#include <BRep_Tool.hxx>
#include <TopExp.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <ShapeFix_Wire.hxx>
#include <ShapeBuild_ReShape.hxx>
#include <ShapeFix_Shape.hxx>
#include <BRepLib.hxx>
#include <ShapeFix_Wireframe.hxx>
#include <ShapeFix_FixSmallFace.hxx>
#include <BRepOffsetAPI_Sewing.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <Geom_Surface.hxx>
#include <ShapeAnalysis.hxx>
#include <Geom_Curve.hxx>
#include <TColgp_SequenceOfPnt.hxx>
#include <ShapeAnalysis_Curve.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <TDocStd_Document.hxx>
#include <TDataStd_Name.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_LayerTool.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <TDF_LabelSequence.hxx>
#include <Quantity_Color.hxx>
#include <TDF_Tool.hxx>
#include <TDF_ChildIterator.hxx>
#include <XCAFPrs.hxx>
#include <XCAFPrs_Style.hxx>
#include <XCAFPrs_DataMapOfShapeStyle.hxx>
#include <XCAFPrs_DataMapOfStyleShape.hxx>
#include <XCAFPrs_DataMapIteratorOfDataMapOfStyleShape.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

extern "C" {
#include "user_interface/message.h"
#include "general/debug.h"
#include "general/mystring.h"
}
#include "region/cmiss_region.h"

#include "cad/computed_field_cad_topology.h"
#include "cad/computed_field_cad_geometry.h"
#include "cad/computed_field_cad_colour.h"

#include "cad/opencascadeformatreader.h"

OpenCascadeImporter::OpenCascadeImporter( const char *fileName )
	: CADImporter( fileName )
{
	m_doc.Nullify();
}


OpenCascadeImporter::~OpenCascadeImporter()
{
	//printf( "OpenCascadeImporter::~OpenCascadeImporter()\n" );
}

bool OpenCascadeImporter::import()
{
	bool success = false;
	if ( !m_fileName.empty() )
	{
		clock_t start, end;
		start = clock();
		OpenCascadeFormatReader reader;
		if ( reader.readModel( m_fileName ) )
		{
			end = clock();
#if !defined(OPTIMISED)
			printf( "File read took %.2f seconds\n", ( end - start ) / double( CLOCKS_PER_SEC ) );
#endif
			start = clock();
			if ( reader.hasXDEInformation() )
			{
				success = createGeometricShapes( reader.xDEInformation() );
				end = clock();
#if !defined(OPTIMISED)
				printf( "Shape re-mapping took %.2f seconds\n", ( end - start ) / double( CLOCKS_PER_SEC ) );
#endif
			}
			else
			{
				end = clock();
#if !defined(OPTIMISED)
				printf( "Don't have XDE information, also no fall back plan!!!!\n" );
#endif
			}
		}
	}
	return success;
}

bool OpenCascadeImporter::import( struct Cmiss_region *region )
{
	OpenCascadeFormatReader reader;
	bool success = false;
	if ( !m_fileName.empty() )
	{
		clock_t start, end;
		start = clock();
		if ( reader.readModel( m_fileName ) )
		{
			end = clock();
#if !defined(OPTIMISED)
			printf( "File read took %.2f seconds\n", ( end - start ) / double( CLOCKS_PER_SEC ) );
#endif
			start = clock();
			if ( reader.hasXDEInformation() )
			{
				success = RemapDocToRegionsAndFields( reader.xDEInformation(), region );
				end = clock();
#if !defined(OPTIMISED)
				printf( "Shape re-mapping took %.2f seconds\n", ( end - start ) / double( CLOCKS_PER_SEC ) );
#endif
			}
			else
			{
				end = clock();
#if !defined(OPTIMISED)
				printf( "Don't have XDE information, also no fall back plan!!!!\n" );
#endif
			}
		}
	}
	return success;
}

void OpenCascadeImporter::recurseTopologicalShapesAndBuildGeometricShapes( TopologicalShape* shape )
{
	//std::cout << "OpenCascadeImporter::recurseTopologicalShape" << std::endl;
	// first clear shape map
	clearMaps();
	// then build shape map
	buildMaps( shape->shape() );
	// create geometric shape from topological shape map
	GeometricShape* geoShape = new GeometricShape();
	geoShape->surfaceColour( shape->surfaceColour() );
	geoShape->curveColour( shape->curveColour() );
	extractPoints( geoShape );
	extractCurvesAndSurfaces( geoShape );
	m_geometricShapes.push_back( geoShape );
	printf( "Warning attempting to use dead code, don't!!! Danger Will Robinson\n" );
	// then recurse into children
	//std::list<TopologicalShape*> children = shape->children();
	//std::cout << "child count: " << children.size() << std::endl;
	//std::list<TopologicalShape*>::iterator it = children.begin();
	//for ( ;it!=children.end();it++)
	//{
		//std::cout << "shape : " << *it << std::endl;
	//	recurseTopologicalShapesAndBuildGeometricShapes( *it );
	//}

}

void OpenCascadeImporter::buildGeometricShapes()
{
	// create geometric shape for every topological shape
	std::list<TopologicalShape*> children = m_rootShape.children();
	std::list<TopologicalShape*>::iterator it = children.begin();
	for ( ;it!=children.end();it++)
		recurseTopologicalShapesAndBuildGeometricShapes( *it );

}

bool OpenCascadeImporter::labelTraversal(const TDF_Label& aLabel,
	const TopLoc_Location& aLocation )
{
	//first label itself
	/** @todo May need to check if a string is defined to 'Name' and use
	 * the current code as the fallback position
	 */
	/**
	 * This could be my primary source for region name
	 * Handle_TDataStd_Name N;
	 * if ( label.FindAttribute( TDataStd_Name::GetID(), N ) )
	 * {
	 *   char *string = (char *) malloc( ( N->Get().LengthOfCString() + 1 ) * sizeof( char ) );
	 *   N->Get().ToUTF8CString( string );
	 *   printf( "name[%d]: %s", N->Get().LengthOfCString(), string );
	 *   free( string );
	 * }
	 */
	TCollection_AsciiString entry;
	TDF_Tool::Entry( aLabel, entry );
	bool addedShape = addGeometricShapeToList( aLabel, aLocation );

	TDF_LabelSequence seq;

	//then attributes of subshapes
	if (XCAFDoc_ShapeTool::GetSubShapes (aLabel, seq))
	{
		Standard_Integer i = 1;
		for (i = 1; i <= seq.Length(); i++)
		{
			TDF_Label aL = seq.Value (i);
			addedShape = addedShape || labelTraversal(aL, aLocation); //suppose that subshapes do not contain locations
		}
	}

	//then attributes of components
	seq.Clear();
	if (XCAFDoc_ShapeTool::GetComponents (aLabel, seq))
	{
		Standard_Integer i = 1;
		for (i = 1; i <= seq.Length(); i++)
		{
			TDF_Label aL = seq.Value (i);
			addedShape = addedShape || labelTraversal(aL, aLocation);
		}
	}

	return addedShape;
}

bool OpenCascadeImporter::labelTraversal(const TDF_Label& aLabel,
	const TopLoc_Location& aLocation, struct Cmiss_region *parent )
{
	//first label itself
	/** @todo May need to check if a string is defined to 'Name' and use
	 * the current code as the fallback position
	 */
	/**
	 * This could be my primary source for region name
	 * Handle_TDataStd_Name N;
	 * if ( label.FindAttribute( TDataStd_Name::GetID(), N ) )
	 * {
	 *   char *string = (char *) malloc( ( N->Get().LengthOfCString() + 1 ) * sizeof( char ) );
	 *   N->Get().ToUTF8CString( string );
	 *   printf( "name[%d]: %s", N->Get().LengthOfCString(), string );
	 *   free( string );
	 * }
	 */
	TCollection_AsciiString entry;
	TDF_Tool::Entry( aLabel, entry );
	//printf( "Creating region '%s'\n", entry.ToCString() );
	//struct Cmiss_region *region = Cmiss_region_create_region( parent );
	//int return_code = Cmiss_region_append_child(parent, region);
	//Cmiss_region_set_name(region, entry.ToCString());
	struct Cmiss_region *region = Cmiss_region_create_child(parent, entry.ToCString());

	if ( !region )
	{
		display_message(ERROR_MESSAGE, "OpenCascadeImporter.  "
			"Unable to create child region.");
		return false;
	}
	bool addedShape = addShapeToRegion( aLabel, aLocation, region );

	TDF_LabelSequence seq;

	//then attributes of subshapes
	if (XCAFDoc_ShapeTool::GetSubShapes (aLabel, seq))
	{
		Standard_Integer i = 1;
		for (i = 1; i <= seq.Length(); i++)
		{
			TDF_Label aL = seq.Value (i);
			addedShape = addedShape || labelTraversal(aL, aLocation, region); //suppose that subshapes do not contain locations
		}
	}

	//then attributes of components
	seq.Clear();
	if (XCAFDoc_ShapeTool::GetComponents (aLabel, seq))
	{
		Standard_Integer i = 1;
		for (i = 1; i <= seq.Length(); i++)
		{
			TDF_Label aL = seq.Value (i);
			addedShape = addedShape || labelTraversal(aL, aLocation, region);
		}
	}

	Cmiss_region_destroy(&region);
	return addedShape;
}

bool OpenCascadeImporter::addGeometricShapeToList( const TDF_Label& aLabel, const TopLoc_Location& aLocation )
{
	USE_PARAMETER(aLocation);
	bool addedShape = false;
	TopoDS_Shape shape;
	if ( ! XCAFDoc_ShapeTool::GetShape ( aLabel, shape ) || shape.IsNull() )
		return addedShape;

	// Catch compounds with no topological shapes
	if (shape.ShapeType() == TopAbs_COMPOUND)
	{
		TopoDS_Iterator anExplor(shape);
		if (!anExplor.More())
			return addedShape;
	}

	//collect information on colored subshapes
	TopLoc_Location L;
	XCAFPrs_DataMapOfShapeStyle settings;
	XCAFPrs::CollectStyleSettings( aLabel, L, settings );
	//std::cout << "Styles collected" << std::endl;

	// dispatch (sub)shapes by their styles
	XCAFPrs_DataMapOfStyleShape items;
	XCAFPrs_Style DefStyle;
	Quantity_Color White( Quantity_NOC_WHITE );
	DefStyle.SetColorSurf( White );
	DefStyle.SetColorCurv( White );
	XCAFPrs::DispatchStyles( shape, settings, items, DefStyle );
	//std::cout << "Dispatch done" << std::endl;

	// add subshapes to shape store (one shape per style)
	XCAFPrs_DataMapIteratorOfDataMapOfStyleShape it ( items );
	Standard_Integer i=1;
	for ( ; it.More(); it.Next() )
	{
		XCAFPrs_Style s = it.Key();
		/*std::cout << "Style " << i << ": [" <<
			( s.IsSetColorSurf() ? Quantity_Color::StringName ( s.GetColorSurf().Name() ) : "" ) << ", " <<
			( s.IsSetColorCurv() ? Quantity_Color::StringName ( s.GetColorCurv().Name() ) : "" ) << "]" <<
			" --> si_" << i << ( s.IsVisible() ? "" : " <invisible>" ) << std::endl;*/
		i++;
		// Add styled shape to store
		TopologicalShape* ts = new TopologicalShape( it.Value() );
		if ( s.IsSetColorSurf() )
			ts->surfaceColour( s.GetColorSurf() );
		if ( s.IsSetColorCurv() )
			ts->curveColour( s.GetColorCurv() );

		// Create field and add to current region
		//printf( "Creating field in region\n" );
		if ( ts )
		{
			GeometricShape* gs = new GeometricShape();
			gs->surfaceColour( ts->surfaceColour() );
			gs->curveColour( ts->curveColour() );
			//extractPoints( gs );
			//extractCurvesAndSurfaces( gs );
			ts->tessellate( gs );
			m_geometricShapes.push_back( gs );

			addedShape = true;
		}
	}

	return addedShape;
}

bool OpenCascadeImporter::addShapeToRegion( const TDF_Label& aLabel, const TopLoc_Location& aLocation, struct Cmiss_region *parent )
{
	USE_PARAMETER(aLocation);
	bool addedShape = false;
	TopoDS_Shape shape;
	if ( ! XCAFDoc_ShapeTool::GetShape ( aLabel, shape ) || shape.IsNull() )
		return addedShape;

	// Catch compounds with no topological shapes
	if (shape.ShapeType() == TopAbs_COMPOUND)
	{
		TopoDS_Iterator anExplor(shape);
		if (!anExplor.More())
			return addedShape;
	}

	//collect information on colored subshapes
	TopLoc_Location L;
	XCAFPrs_DataMapOfShapeStyle settings;
	XCAFPrs::CollectStyleSettings ( aLabel, L, settings );
	//std::cout << "Styles collected" << std::endl;

	// dispatch (sub)shapes by their styles
	XCAFPrs_DataMapOfStyleShape items;
	XCAFPrs_Style DefStyle;
	Quantity_Color White ( Quantity_NOC_WHITE );
	DefStyle.SetColorSurf ( White );
	DefStyle.SetColorCurv ( White );
	XCAFPrs::DispatchStyles ( shape, settings, items, DefStyle );
	//std::cout << "Dispatch done" << std::endl;

	// add subshapes to shape store (one shape per style)
	XCAFPrs_DataMapIteratorOfDataMapOfStyleShape it ( items );
	Standard_Integer i=1;
	for ( ; it.More(); it.Next() )
	{
		XCAFPrs_Style s = it.Key();
		/*std::cout << "Style " << i << ": [" <<
			( s.IsSetColorSurf() ? Quantity_Color::StringName ( s.GetColorSurf().Name() ) : "" ) << ", " <<
			( s.IsSetColorCurv() ? Quantity_Color::StringName ( s.GetColorCurv().Name() ) : "" ) << "]" <<
			" --> si_" << i << ( s.IsVisible() ? "" : " <invisible>" ) << std::endl;*/
		i++;
		// Add styled shape to store
		TopologicalShape* ts = new TopologicalShape( it.Value() );
		if ( s.IsSetColorSurf() )
			ts->surfaceColour( s.GetColorSurf() );
		if ( s.IsSetColorCurv() )
			ts->curveColour( s.GetColorCurv() );

		// Create field and add to current region
		char *name, *geo_name = 0, *col_name = 0;
		int status = 0;
		//printf( "Creating field in region\n" );
		Cmiss_field_module_id field_module = Cmiss_region_get_field_module(parent);
		Cmiss_field_id cad_topology_field = Cmiss_field_create_cad_topology( field_module, ts );
		//Cmiss_field_set_name(cad_topology_field, NAME);
		Cmiss_field_set_persistent(cad_topology_field, 1);
		Cmiss_field_cad_topology_id cad_topology = Cmiss_field_cast_cad_topology( cad_topology_field );
		// Create shape gs
		GeometricShape* gs = new GeometricShape();
		ts->tessellate(gs);
		Cmiss_field_cad_topology_set_geometric_shape(cad_topology, gs);
		// sketchy:
		Cmiss_field_destroy((Cmiss_field_id *)&cad_topology);

		GET_NAME(Computed_field)(cad_topology_field, &name);
		append_string(&geo_name, name, &status);
		append_string(&geo_name, "-geo", &status);
		append_string(&col_name, name, &status);
		append_string(&col_name, "-col", &status);

		Cmiss_field_id cad_geometry_field = Computed_field_create_cad_geometry(field_module, cad_topology_field);
		Cmiss_field_set_name(cad_geometry_field, geo_name);
		Cmiss_field_set_persistent(cad_geometry_field, 1);

		Cmiss_field_id cad_colour_field = Computed_field_create_cad_colour(field_module, cad_topology_field);
		Cmiss_field_set_name(cad_colour_field, col_name);
		Cmiss_field_set_persistent(cad_colour_field, 1);

		if ( cad_topology_field && cad_geometry_field && cad_colour_field )
		{
			addedShape = true;
		}
		Cmiss_field_destroy(&cad_colour_field);
		Cmiss_field_destroy(&cad_geometry_field);
		Cmiss_field_destroy(&cad_topology_field);

		DEALLOCATE(name);
		DEALLOCATE(geo_name);
		DEALLOCATE(col_name);

		Cmiss_field_module_destroy(&field_module);

	}

	return addedShape;
}

bool OpenCascadeImporter::createGeometricShapes( Handle_TDocStd_Document doc )
{
	bool addedShape = false;
	if ( !doc )
		return addedShape;

	Handle_XCAFDoc_ShapeTool shapeTool = XCAFDoc_DocumentTool::ShapeTool( doc->Main() );

	TDF_LabelSequence root_labels;
	shapeTool->GetFreeShapes( root_labels );
	TopLoc_Location location;

	for ( Standard_Integer i = 1; i <= root_labels.Length(); i++ )
	{
		TDF_Label label = root_labels.Value( i );
		addedShape = labelTraversal( label, location );
	}

	return addedShape;
}

bool OpenCascadeImporter::RemapDocToRegionsAndFields( Handle_TDocStd_Document doc, struct Cmiss_region *region )
{
	bool addedShape = false;
	if ( !doc )
		return addedShape;

	Handle_XCAFDoc_ShapeTool shapeTool = XCAFDoc_DocumentTool::ShapeTool( doc->Main() );

	TDF_LabelSequence root_labels;
	shapeTool->GetFreeShapes( root_labels );
	TopLoc_Location location;
	//shapeTool

	for ( Standard_Integer i = 1; i <= root_labels.Length(); i++ )
	{
		TDF_Label label = root_labels.Value( i );

		TCollection_AsciiString entry;
		TDF_Tool::Entry( label, entry );
#if !defined(OPTIMISED)
		printf( "root region '%s'\n", entry.ToCString() );
#endif
		addedShape = labelTraversal( label, location, region );
	}

	return addedShape;
}

void OpenCascadeImporter::mapShapes(  Handle_TopTools_HSequenceOfShape shapes  )
{
	m_doc.Nullify();
	clearMaps();
	for ( int i = 1; i <= shapes->Length(); i++ )
	{
		mapShape( shapes->Value( i ) );
	}
}

void OpenCascadeImporter::mapShape( TopoDS_Shape shape )
{
	bool fix_shape = false;
	if ( !shape.IsNull() )
	{
		if ( fix_shape )
		{
			ShapeFix_Shape fixer( shape );
			fixer.Perform();
			BRepBuilderAPI_Sewing sew;
			sew.Add( fixer.Shape() );
			sew.Perform();
			shape = sew.SewedShape();
		}

		buildMaps( shape );

		//healGeometry( shape, 1.e-6, true, true, false );
		//extractShapeInfo();
	}
}

void OpenCascadeImporter::clearMaps()
{
	m_fmap.Clear();
	m_emap.Clear();
	m_vmap.Clear();
	m_shmap.Clear();
	m_somap.Clear();
	m_csmap.Clear();
	m_cmap.Clear();
}

void OpenCascadeImporter::buildMaps( const TopoDS_Shape& shape )
{
	TopExp::MapShapes( shape, TopAbs_VERTEX, m_vmap );
	TopExp::MapShapes( shape, TopAbs_EDGE, m_emap );
	TopExp::MapShapes( shape, TopAbs_FACE, m_fmap );
	TopExp::MapShapes( shape, TopAbs_SHELL, m_shmap );
	TopExp::MapShapes( shape, TopAbs_SOLID, m_somap );
	TopExp::MapShapes( shape, TopAbs_COMPSOLID, m_csmap );
	TopExp::MapShapes( shape, TopAbs_COMPOUND, m_cmap );
}

void OpenCascadeImporter::extractPoints( GeometricShape* geoShape )
{
	clock_t start, end;
	start = clock();
	int nvertices = m_vmap.Extent();
	for ( int i = 0; i < nvertices; i++ )
	{
		TopoDS_Vertex vertex = TopoDS::Vertex( m_vmap( i + 1 ) );
		gp_Pnt pnt = BRep_Tool::Pnt( vertex );
		geoShape->append( new Point( pnt ) );
	}
	end = clock();
	//printf( "Vertex (%d) extraction took %.2f seconds\n", nvertices, ( end - start ) / double( CLOCKS_PER_SEC ) );
}

void OpenCascadeImporter::extractCurvesAndSurfaces( GeometricShape* geoShape )
{
	clock_t start, end;
	start = clock();
	TopExp_Explorer edgeEx;
	TopTools_IndexedMapOfShape edgeMap;
	Surface* surface = 0;
	Curve* curve = 0;
	int nfaces = m_fmap.Extent();
#if !defined(OPTIMISED)
	static int called_count = 0;
#endif
	for ( int i = 1; i <= nfaces; i++ )
	{
		TopoDS_Face face = TopoDS::Face( m_fmap( i ) );
		surface = new Surface( face );
		//surface->setColour( geoShape->surfaceColour() );
		//surface->buildOpenGLRep();
		geoShape->append( surface );
		for( edgeEx.Init(face, TopAbs_EDGE); edgeEx.More(); edgeEx.Next() )
		{
			if ( !edgeMap.Contains( edgeEx.Current() ) )
			{
				TopoDS_Edge topoDSEdge = TopoDS::Edge( edgeEx.Current() );
				edgeMap.Add( topoDSEdge );
				curve = new Curve( topoDSEdge );
				curve->setColour( geoShape->curveColour() );
				if ( !curve->is3D() )
				{
					curve->setTrimmed( face );
				}

				if ( curve->buildOpenGLRep() )
				{
					if ( curve->points() == 0 )
					{
#if !defined(OPTIMISED)
						printf( " wtf %d, values: %p, colours: %p, is3D(%d), is2D(%d)\n", curve->pointCount(), curve->points(), curve->colours(), curve->is3D(), curve->is2D() );
#endif
					}
					geoShape->append( curve );
				}
				else
				{
#if !defined(OPTIMISED)
					printf( "Curve failed to build OpenGL representation !!!!\n" );
#endif
				}
			}
		}
	}
	end = clock();
	if ( nfaces )
	{
		//printf( "Surface (%d) and curve extraction took %.2f seconds [average: %.4f]\n", nfaces, ( end - start ) / double( CLOCKS_PER_SEC ), ( end - start ) / double( CLOCKS_PER_SEC * nfaces ) );
		//printf( "curves running total: %d\n", (int)m_curves.size() );
	}
	else
	{
#if !defined(OPTIMISED)
		printf( "No faces found!!\n" );
#endif
	}
#if !defined(OPTIMISED)
	printf( "." );
	if ( ++called_count % 10 == 0 )
		printf( "\n" );
#endif
}

void OpenCascadeImporter::healGeometry( TopoDS_Shape shape, double tolerance, bool fixsmalledges,
										 bool fixspotstripfaces, bool sewfaces,	bool makesolids)
{
	int nrc = 0, nrcs = 0;
	TopExp_Explorer e;
	for(e.Init(shape, TopAbs_COMPOUND); e.More(); e.Next()) nrc++;
	for(e.Init(shape, TopAbs_COMPSOLID); e.More(); e.Next()) nrcs++;

	double surfacecont = 0;

	for(int i = 1; i <= m_fmap.Extent(); i++){
		GProp_GProps system;
		BRepGProp::LinearProperties(m_fmap(i), system);
		surfacecont += system.Mass();
	}

	if(fixsmalledges){

		Handle(ShapeFix_Wire) sfw;
		Handle_ShapeBuild_ReShape rebuild = new ShapeBuild_ReShape;
		rebuild->Apply(shape);

		for(int i = 1; i <= m_fmap.Extent(); i++){
			TopExp_Explorer exp1;
			for(exp1.Init(m_fmap(i), TopAbs_WIRE); exp1.More(); exp1.Next()){
				TopoDS_Wire oldwire = TopoDS::Wire(exp1.Current());
				sfw = new ShapeFix_Wire(oldwire, TopoDS::Face(m_fmap(i)), tolerance);
				sfw->ModifyTopologyMode() = Standard_True;

				if(sfw->FixSmall(false, tolerance)){
					TopoDS_Wire newwire = sfw->Wire();
					rebuild->Replace(oldwire, newwire, Standard_False);
				}
				if((sfw->StatusSmall(ShapeExtend_FAIL1)) ||
								(sfw->StatusSmall(ShapeExtend_FAIL2)) ||
								(sfw->StatusSmall(ShapeExtend_FAIL3)))
					/** \todo write out warning message */{}
			}
		}
		shape = rebuild->Apply(shape);

		{
			Handle_ShapeBuild_ReShape rebuild = new ShapeBuild_ReShape;
			rebuild->Apply(shape);
			TopExp_Explorer exp1;
			for(exp1.Init(shape, TopAbs_EDGE); exp1.More(); exp1.Next()){
				TopoDS_Edge edge = TopoDS::Edge(exp1.Current());
				if(m_vmap.FindIndex(TopExp::FirstVertex(edge)) ==
							   m_vmap.FindIndex(TopExp::LastVertex(edge))){
					GProp_GProps system;
					BRepGProp::LinearProperties(edge, system);
					if(system.Mass() < tolerance)
					{
						rebuild->Remove(edge, false);
					}
				}
			}
			shape = rebuild->Apply(shape);
		}

		Handle(ShapeFix_Wireframe) sfwf = new ShapeFix_Wireframe;
		sfwf->SetPrecision(tolerance);
		sfwf->Load(shape);

		/** \todo replace debug messages with warnings
		if(sfwf->FixSmallEdges()){
			qDebug("- fixing wire frames");
			if(sfwf->StatusSmallEdges(ShapeExtend_OK)) qDebug("no small edges found");
			if(sfwf->StatusSmallEdges(ShapeExtend_DONE1)) qDebug("some small edges fixed");
			if(sfwf->StatusSmallEdges(ShapeExtend_FAIL1)) qDebug("failed to fix some small edges");
		}

		if(sfwf->FixWireGaps()){
			qDebug("- fixing wire gaps");
			if(sfwf->StatusWireGaps(ShapeExtend_OK)) qDebug("no gaps found");
			if(sfwf->StatusWireGaps(ShapeExtend_DONE1)) qDebug("some 2D gaps fixed");
			if(sfwf->StatusWireGaps(ShapeExtend_DONE2)) qDebug("some 3D gaps fixed");
			if(sfwf->StatusWireGaps(ShapeExtend_FAIL1)) qDebug("failed to fix some 2D gaps");
			if(sfwf->StatusWireGaps(ShapeExtend_FAIL2)) qDebug("failed to fix some 3D gaps");
		}
		*/
		shape = sfwf->Shape();
	}

	if(fixspotstripfaces)
	{
		Handle(ShapeFix_FixSmallFace) sffsm = new ShapeFix_FixSmallFace;
		sffsm->Init(shape);
		sffsm->SetPrecision(tolerance);
		sffsm->Perform();

		shape = sffsm->FixShape();
	}

	if(sewfaces)
	{
		TopExp_Explorer exp0;

		BRepOffsetAPI_Sewing sewedObj(tolerance);

		for(exp0.Init(shape, TopAbs_FACE); exp0.More(); exp0.Next())
		{
			TopoDS_Face face = TopoDS::Face(exp0.Current());
			sewedObj.Add(face);
		}

		sewedObj.Perform();

		if(!sewedObj.SewedShape().IsNull())
			shape = sewedObj.SewedShape();
	}

	if(makesolids)
	{
		TopExp_Explorer exp0;

		BRepBuilderAPI_MakeSolid ms;
		int count = 0;
		for(exp0.Init(shape, TopAbs_SHELL); exp0.More(); exp0.Next())
		{
			count++;
			ms.Add(TopoDS::Shell(exp0.Current()));
		}

		if ( count )
		{
			BRepCheck_Analyzer ba(ms);
			if(ba.IsValid())
			{
				Handle(ShapeFix_Shape) sfs = new ShapeFix_Shape;
				sfs->Init(ms);
				sfs->SetPrecision(tolerance);
				sfs->SetMaxTolerance(tolerance);
				sfs->Perform();
				shape = sfs->Shape();

				for(exp0.Init(shape, TopAbs_SOLID); exp0.More(); exp0.Next())
				{
					TopoDS_Solid solid = TopoDS::Solid(exp0.Current());
					TopoDS_Solid newsolid = solid;
					BRepLib::OrientClosedSolid(newsolid);
					Handle_ShapeBuild_ReShape rebuild = new ShapeBuild_ReShape;
					// rebuild->Apply(shape);
					rebuild->Replace(solid, newsolid, Standard_False);
					TopoDS_Shape newshape = rebuild->Apply(shape, TopAbs_COMPSOLID, 1);
					// TopoDS_Shape newshape = rebuild->Apply(shape);
					shape = newshape;
				}
			}
		}
	}
}

