
#include "topologicalshape.h"

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepMesh.hxx>
#include <BRepMesh_FastDiscret.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopExp_Explorer.hxx>

#include "geometricshape.h"
#include "point.h"
#include "curve.h"
#include "surface.h"

TopologicalShape::TopologicalShape( const TopoDS_Shape& s )
	: m_shape( s )
	, m_surfaceColour( Quantity_NOC_WHITE )
	, m_curveColour( Quantity_NOC_WHITE )
{
}

TopologicalShape::~TopologicalShape()
{
}

void TopologicalShape::tessellate( GeometricShape* geoShape )
{
	// First type of tessellation is just to triangulate the shape
	/*
  Bnd_Box boundingBox;
	Standard_Boolean withShare = Standard_True;
	Standard_Boolean inShape = Standard_True;
	Standard_Boolean relative = Standard_True;
	Standard_Boolean shapeTrigultn = Standard_False;
	Standard_Real deflection, aXmin, aYmin, aZmin, aXmax, aYmax, aZmax;
	Standard_Real dX, dY, dZ, dMax, aCoeff;
	Standard_Real angle = 0.5;

	BRepBndLib::Add( m_shape, boundingBox );
	boundingBox.Get( aXmin, aYmin, aZmin, aXmax, aYmax, aZmax );
	dX = aXmax-aXmin;
	dY = aYmax-aYmin;
	dZ = aZmax-aZmin;
	dMax = dX;
	if(dY > dMax) {
		dMax = dY;
	}
	if(dZ > dMax) {
		dMax = dZ;
	}

	aCoeff = 0.01;
	deflection = aCoeff * dMax;
  BRepMesh_FastDiscret aMesher( deflection, m_shape, boundingBox, angle, withShare, inShape, relative, shapeTrigultn );
	*/
	// Trying out alternative triangulation method
	double pixels_per_mm = 5.71;
	BRepTools::Clean(m_shape);
	BRepMesh::Mesh(m_shape, 1.0/pixels_per_mm);


	// build shape maps vertex, edge and face
	clearMaps();
	buildMaps();
	extractPoints( geoShape );
	extractCurvesAndSurfaces( geoShape );

	//BRepTools::Clean( m_shape );
}

void TopologicalShape::extractPoints( GeometricShape* geoShape )
{
	Standard_Integer nvertices = m_vmap.Extent();
	for ( Standard_Integer i = 1; i <= nvertices; i++ )
	{
		TopoDS_Vertex vertex = TopoDS::Vertex( m_vmap( i ) );
		gp_Pnt pnt = BRep_Tool::Pnt( vertex );
		geoShape->append( new Point( pnt ) );
	}
}

void TopologicalShape::extractCurvesAndSurfaces( GeometricShape* geoShape )
{
	TopExp_Explorer edgeEx;
	TopTools_IndexedMapOfShape edgeMap;
	Surface* surface = 0;
	Curve* curve = 0;
	Standard_Integer nfaces = m_fmap.Extent();
	for ( Standard_Integer i = 1; i <= nfaces; i++ )
	{
		TopoDS_Face face = TopoDS::Face( m_fmap( i ) );
		surface = new Surface( face );
		//surface->buildOpenGLRep();
		//surface->setColour( geoShape->surfaceColour() );
		surface->tessellate();
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

				curve->tessellate();
				//if ( curve->buildOpenGLRep() )
				//{
				//	if ( curve->points() == 0 )
				//		printf( " wtf %d, values: %p, colours: %p, is3D(%d), is2D(%d)\n", curve->count(), curve->points(), curve->colours(), curve->is3D(), curve->is2D() );
				geoShape->append( curve );
				//}
				//else
				//{
				//	printf( "Curve failed to build OpenGL representation !!!!\n" );
				//}
			}
		}
	}
}

int TopologicalShape::surfaceColour(double *colour) const
{
	colour[0] = m_surfaceColour.Red();
	colour[1] = m_surfaceColour.Green();
	colour[2] = m_surfaceColour.Blue();
	return 1;
}

void TopologicalShape::buildMaps()
{
	TopExp::MapShapes( m_shape, TopAbs_VERTEX, m_vmap );
	TopExp::MapShapes( m_shape, TopAbs_EDGE, m_emap );
	TopExp::MapShapes( m_shape, TopAbs_FACE, m_fmap );
}

void TopologicalShape::clearMaps()
{
	m_vmap.Clear();
	m_emap.Clear();
	m_fmap.Clear();
}


