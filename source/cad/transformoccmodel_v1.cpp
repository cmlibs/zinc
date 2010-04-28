#include "transformoccmodel.h"

#include <stack>

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

extern "C" {
#include "user_interface/message.h"
}

TransformOCCModel::TransformOCCModel()
{
	m_doc.Nullify();
}


TransformOCCModel::~TransformOCCModel()
{
	for ( unsigned int i = 0; i < m_vertices.size(); i++ )
	{
		delete m_vertices[i];
	}
	for ( unsigned int i = 0; i < m_edges.size(); i++ )
	{
		delete m_edges[i];
	}
	for ( unsigned int i = 0; i < m_surfaces.size(); i++ )
	{
		delete m_surfaces[i];
	}
}

void printTDFLabel( const TDF_Label& label )
{
	Handle_XCAFDoc_ColorTool colourTool = XCAFDoc_DocumentTool::ColorTool( label );
	Handle_TDataStd_Name N;
	if ( label.FindAttribute( TDataStd_Name::GetID(), N ) )
	{
		char *string = (char *) malloc( ( N->Get().LengthOfCString() + 1 ) * sizeof( char ) );
		N->Get().ToUTF8CString( string );
		printf( "name[%d]: %s", N->Get().LengthOfCString(), string );
		free( string );
	}
	TCollection_AsciiString entry;
	TDF_Tool::Entry( label, entry );
	std::cout << ", " << entry;
	TDF_Label colourLabel;
	if( colourTool->GetColor( label, XCAFDoc_ColorGen, colourLabel) ||
		colourTool->GetColor( label, XCAFDoc_ColorSurf, colourLabel) ||
		colourTool->GetColor( label, XCAFDoc_ColorCurv, colourLabel) )
	{
		Quantity_Color colour;
		if ( colourTool->GetColor( colourLabel, colour ) )
		{
			std::cout << ", " << Quantity_Color::StringName( colour.Name() );
		}
	}
	std::cout << std::endl;
}

Standard_Boolean TransformOCCModel::extractColour( const TDF_Label& label, const Handle_XCAFDoc_ColorTool& colourTool, Quantity_Color &colour )
{
	//Quantity_Color colour = Quantity_Color();
	Standard_Boolean ok = false;

	ok = colourTool->GetColor( label, XCAFDoc_ColorCurv, colour);
	if ( ok )
	{
		//printf( "Curve has a good colour %s\n", Quantity_Color::StringName( colour.Name() ) );
		return ok;
	}

	ok = colourTool->GetColor( label, XCAFDoc_ColorSurf, colour);
	if ( ok )
	{
		//printf( "Surface has a good colour %s\n", Quantity_Color::StringName( colour.Name() ) );
		return ok;
	}

	ok = colourTool->GetColor( label, XCAFDoc_ColorGen, colour);
	{
		//printf( "General has a good colour %s\n", Quantity_Color::StringName( colour.Name() ) );
	}

	return ok;
}

Standard_Boolean TransformOCCModel::extractColour( const TopoDS_Shape& shape, const Handle_XCAFDoc_ColorTool& colourTool, Quantity_Color &colour )
{
	//Quantity_Color colour = Quantity_Color();
	Standard_Boolean ok = false;

	ok = colourTool->GetColor( shape, XCAFDoc_ColorCurv, colour);
	if ( ok )
	{
		//printf( "Curve has a good colour %s\n", Quantity_Color::StringName( colour.Name() ) );
		return ok;
	}

	ok = colourTool->GetColor( shape, XCAFDoc_ColorSurf, colour);
	if ( ok )
	{
		//printf( "Surface has a good colour %s\n", Quantity_Color::StringName( colour.Name() ) );
		return ok;
	}

	ok = colourTool->GetColor( shape, XCAFDoc_ColorGen, colour);
	{
		//printf( "General has a good colour %s\n", Quantity_Color::StringName( colour.Name() ) );
	}

	return ok;
}
/*
void TransformOCCModel::plumbLabel( const TDF_Label& label,
	const Handle_XCAFDoc_ShapeTool& shapeTool, const Handle_XCAFDoc_ColorTool& colourTool )
{

	//Quantity_Color origColour = m_colour;
	bool ok;
	Quantity_Color colour = extractColour( label, colourTool, &ok );
	//if ( ok )
	//	m_colour = colour;

	if ( label.HasChild() )
	{
		for ( Standard_Integer i = 1; i <= label.NbChildren(); i++ )
		{
			//plumbLabel( label.FindChild( i, Standard_False ), shapeTool, colourTool );
		}
	}
	TopoDS_Shape S = shapeTool->GetShape( label );
	mapShape( S );

	//m_colour = origColour;
}
*/
void TransformOCCModel::traverseShape( const TopoDS_Shape& shape )
{
	TopoDS_Iterator it( shape );
	for ( ; it.More(); it.Next() )
	{
		const TopoDS_Shape& child = it.Value();
		if ( child.ShapeType() < TopAbs_EDGE )
			traverseShape( child );

		mapShape( child );
	}
}

void TransformOCCModel::mapShapes( Handle_TDocStd_Document doc )
{
	m_doc.Nullify();
	m_doc = doc;
	Handle_XCAFDoc_ShapeTool shapeTool = XCAFDoc_DocumentTool::ShapeTool( doc->Main() );
	Handle_XCAFDoc_ColorTool colourTool = XCAFDoc_DocumentTool::ColorTool( doc->Main() );
	Handle_XCAFDoc_LayerTool layerTool = XCAFDoc_DocumentTool::LayerTool( doc->Main() );

	Quantity_Color colour;
	TCollection_ExtendedString layer;
	TDF_LabelSequence root_labels;
	shapeTool->GetFreeShapes( root_labels );
	TopoDS_Shape S;
	
	printf( "Shape count: %d\n", root_labels.Length() );
	for ( Standard_Integer i = 1; i <= root_labels.Length(); i++ )
	{
		//plumbLabel( root_labels.Value( i ), shapeTool, colourTool );
		TDF_Label label = root_labels.Value( i );
		TDF_ChildIterator it( label );
		int count = 0;
		for ( ; it.More(); it.Next() )
		{
			count++;
			TDF_Label L = it.Value();
		}
		if ( shapeTool->GetShape( label, S ) )//&& !shapeTool->IsFree( label ) )
		{
			mapShape( S );
		}
		bool ok = extractColour( S, colourTool, colour );
		printf( "Label information: IsShape( %d ), IsAssembly( %d ), IsFree( %d ), IsSimpleShape( %d ), IsReference( %d )\n",
			shapeTool->IsShape( label ), shapeTool->IsAssembly( label ), shapeTool->IsFree( label ), shapeTool->IsSimpleShape( label ),
			shapeTool->IsReference( label ) );
		if ( !ok && shapeTool->IsReference( label ) )
		{
			TDF_Label refLabel;
			shapeTool->GetReferredShape( label, refLabel );
			ok = extractColour( label, colourTool, colour );
			printf( "Referred shape colour (label) (%d): %s\n", ok, Quantity_Color::StringName( colour.Name() ) );
		}
		else
		{
			printf( "Shape colour (shape) (%d): %s\n", ok, Quantity_Color::StringName( colour.Name() ) );
		}
		//std::cout << label.Dump( std::cout ) << std::endl;
		printf( "Children count: %d\n", count );
		//std::cout << "m_colour has this name: " << Quantity_Color::StringName( m_colour.Name() ) << std::endl;
		//std::cout << "shape type: " << S.ShapeType() << std::endl;
		//gp_XYZ loc = S.Location().Transformation().TranslationPart();
		//std::cout << "location: " << loc.X() << " " << loc.Y() << " " << loc.Z() << std::endl;
		//if ( !shapeTool->IsAssembly( label ) )
		/*
		TDF_LabelSequence subShapeLabels;
		shapeTool->GetComponents( label, subShapeLabels, Standard_True );
		Standard_Integer nbShapes = subShapeLabels.Length();
		printf( "========== Shape %d of %d ===========\n", i, root_labels.Length() ); 
		bool ok = extractColour( S, colourTool, colour );
		printf( "Label information: IsShape( %d ), IsAssembly( %d ), IsFree( %d ), IsSimpleShape( %d ), IsReference( %d )\n",
			shapeTool->IsShape( label ), shapeTool->IsAssembly( label ), shapeTool->IsFree( label ), shapeTool->IsSimpleShape( label ),
			shapeTool->IsReference( label ) );
		printf( "Shape colour (shape) (%d): %s\n", ok, Quantity_Color::StringName( colour.Name() ) );
		printTDFLabel( label );
		if ( shapeTool->IsReference( label ) )
		{
			TDF_Label refLabel;
			shapeTool->GetReferredShape( label, refLabel );
			ok = extractColour( label, colourTool, colour );
			printf( "Referred shape colour (label) (%d): %s\n", ok, Quantity_Color::StringName( colour.Name() ) );
		}
		for (Standard_Integer j = 1; j <= nbShapes; j++)
		{
			TDF_Label subLabel = subShapeLabels.Value( j );
			TopoDS_Shape S = shapeTool->GetShape( subLabel );
			printf( "========== Sub-Shape %d of %d of %d of %d ===========\n", j, nbShapes, i, root_labels.Length() ); 
			bool ok = extractColour( S, colourTool, colour );
			printf( "Label information: IsShape( %d ), IsAssembly( %d ), IsFree( %d ), IsSimpleShape( %d ), IsReference( %d )\n",
				shapeTool->IsShape( subLabel ), shapeTool->IsAssembly( subLabel ), shapeTool->IsFree( subLabel ), shapeTool->IsSimpleShape( subLabel ),
				shapeTool->IsReference( subLabel ) );
			printf( "Shape colour (shape) (%d): %s\n", ok, Quantity_Color::StringName( colour.Name() ) );
			if ( shapeTool->IsReference( subLabel ) )
			{
				TDF_Label refLabel;
				shapeTool->GetReferredShape( subLabel, refLabel );
				ok = extractColour( refLabel, colourTool, colour );
				printf( "    Referred Label information: IsShape( %d ), IsAssembly( %d ), IsFree( %d ), IsSimpleShape( %d ), IsReference( %d )\n",
					shapeTool->IsShape( refLabel ), shapeTool->IsAssembly( refLabel ), shapeTool->IsFree( refLabel ), shapeTool->IsSimpleShape( refLabel ),
					shapeTool->IsReference( refLabel ) );
				printf( "    Referred shape colour (label) (%d): %s\n", ok, Quantity_Color::StringName( colour.Name() ) );
				printTDFLabel( refLabel );
			}
		}*/
		//else
		//{
		//	printf( "Shape %d of %d is an assembly\n", i, root_labels.Length() );
		//}
		//B.Add(C,S);
	}

	TDF_LabelSequence colourLabels;
	colourTool->GetColors( colourLabels );
	
	for ( int i = 1; i <= colourLabels.Length(); i++ )
	{
		//Quantity_Color colour;
		//TCollection_AsciiString entry;
		//TDF_Tool::Entry( colourLabels.Value( i ), entry );
		//std::cout << entry << std::endl;
		if ( colourTool->GetColor( colourLabels.Value( i ), colour ) )
		{
			std::cout << Quantity_Color::StringName( colour.Name() ) << std::endl;
		}
		//std::cout << colourLabels.Value( i ).Dump( std::cout ) << std::endl;
	}
	
	if ( colourLabels.Length() == 0 )
	{
		std::cout << "Damn, no colours at all" << std::endl;
	}
	TDF_LabelSequence layerLabels;
	layerTool->GetLayerLabels( layerLabels );
	for ( int i = 1; i <= layerLabels.Length(); i++ )
	{
		if ( layerTool->GetLayer( layerLabels.Value( i ), layer ) )
		{
			std::cout << layer << std::endl;
		}
	}
	if ( layerLabels.Length() == 0 )
	{
		std::cout << "Damn, no layers at all" << std::endl;
	}

}

void TransformOCCModel::mapShapes(  Handle_TopTools_HSequenceOfShape shapes  )
{
	m_doc.Nullify();
	for ( int i = 1; i <= shapes->Length(); i++ )
	{
		mapShape( shapes->Value( i ) );
	}
}

void TransformOCCModel::mapShape( TopoDS_Shape shape )
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
		extractShapeInfo();
	}
}

void TransformOCCModel::buildMaps( TopoDS_Shape shape )
{
	m_fmap.Clear();
	m_emap.Clear();
	m_vmap.Clear();
	m_shmap.Clear();
	m_somap.Clear();
	m_csmap.Clear();
	m_cmap.Clear();

	//int nrc = 0, nrcs = 0, nfc = 0;
	//TopExp_Explorer e, e1;
	//for(e.Init(shape, TopAbs_COMPOUND); e.More(); e.Next())
	//{
	//	nrc++;
	//	for ( e1.Init( e.Current(), TopAbs_FACE); e1.More(); e1.Next() )
	//		nfc++;
	//}
	//for(e.Init(shape, TopAbs_COMPSOLID); e.More(); e.Next()) nrcs++;
	//printf( " Compound count = %d\n", nrc );
	//printf( " Faces in coumpound cout = %d\n", nfc );
	//printf( " Compound solid count = %d\n", nrcs );

	TopExp::MapShapes( shape, TopAbs_VERTEX, m_vmap );
	TopExp::MapShapes( shape, TopAbs_EDGE, m_emap );
	TopExp::MapShapes( shape, TopAbs_FACE, m_fmap );
	TopExp::MapShapes( shape, TopAbs_SHELL, m_shmap );
	TopExp::MapShapes( shape, TopAbs_SOLID, m_somap );
	TopExp::MapShapes( shape, TopAbs_COMPSOLID, m_csmap );
	TopExp::MapShapes( shape, TopAbs_COMPOUND, m_cmap );

}

void TransformOCCModel::extractShapeInfo()
{
    // building geom vertices
    // vertices can just be considered as vertices
	clock_t start, end;
	start = clock();
	int nvertices = m_vmap.Extent();
	for ( int i = 0; i < nvertices; i++ )
	{
		TopoDS_Vertex vertex = TopoDS::Vertex( m_vmap( i + 1 ) );
		gp_Pnt pnt = BRep_Tool::Pnt( vertex );
		m_vertices.push_back( new Vertex( pnt ) );
	}
	end = clock();
	printf( "Vertex (%d) extraction took %.2f seconds\n", nvertices, ( end - start ) / double( CLOCKS_PER_SEC ) );

	Handle_XCAFDoc_ShapeTool shapeTool;
	Handle_XCAFDoc_ColorTool colourTool;
	if ( !m_doc.IsNull() )
	{
		shapeTool = XCAFDoc_DocumentTool::ShapeTool( m_doc->Main() );
		colourTool = XCAFDoc_DocumentTool::ColorTool( m_doc->Main() );
	}
	start = clock();
	TopExp_Explorer exEdge;
	Surface* surface = 0;
	Edge* edge = 0;
	Quantity_Color colour;
	int nfaces = m_fmap.Extent();
	int nsolids = m_somap.Extent();
	int nshells = m_shmap.Extent();
	int ncs = m_csmap.Extent();
	int nc = m_cmap.Extent();
	bool ok;
	bool surface_has_colour = false;
	static int edge_count = 0;
	//printf( "compound count %d\n", nc );
	std::stack<Quantity_Color> colourStack;
	for ( int k = 1; k <= nc; k++ )
	{
		ok = extractColour( m_cmap( k ), colourTool, colour );
		if ( ok )
		{
			colourStack.push( colour );
			printf( "Adding Compound shape colour (%d): %s\n", int( colourStack.size() ), Quantity_Color::StringName( colour.Name() ) );
		}
	}
	//printf( "Compound solid count %d\n", ncs );
	for ( int k = 1; k <= ncs; k++ )
	{
		ok = extractColour( m_csmap( k ), colourTool, colour );
		if ( ok )
		{
			colourStack.push( colour );
			printf( "Adding Compound solid shape colour (%d): %s\n", int( colourStack.size() ), Quantity_Color::StringName( colour.Name() ) );
		}
	}
	//printf( "solid count %d\n", nsolids );
	for ( int k = 1; k <= nsolids; k++ )
	{
		ok = extractColour( m_somap( k ), colourTool, colour );
		if ( ok )
		{
			colourStack.push( colour );
			printf( "Adding Solid shape colour (%d): %s\n", int( colourStack.size() ), Quantity_Color::StringName( colour.Name() ) );
		}
	//	printf( "Solid shape colour (%d): %s\n", ok, Quantity_Color::StringName( colour.Name() ) );
	}
	//printf( "shell count %d\n", nshells );
	for ( int j = 1; j <= nshells; j++ )
	{
		ok = extractColour( m_shmap( j ), colourTool, colour );
		if ( ok )
		{
			colourStack.push( colour );
			printf( "Adding Shell shape colour (%d): %s\n", int( colourStack.size() ), Quantity_Color::StringName( colour.Name() ) );
		}
//		printf( "Shell shape colour (%d): %s\n", ok, Quantity_Color::StringName( colour.Name() ) );
	}
	for ( int i = 1; i <= nfaces; i++ )
	{
		TopoDS_Face face = TopoDS::Face( m_fmap( i ) );
		surface = new Surface( face );
		if ( !m_doc.IsNull() )
		{
			surface_has_colour = (colourStack.size() > 0 );//extractColour( face, colourTool, colour );
			if ( !surface_has_colour )
			{
				// Search for a referred shape which may have a colour
				TDF_Label associatedLabel;
				TopoDS_Shape S;
				// get all shapes
				TDF_LabelSequence labels;
				TopTools_IndexedDataMapOfShapeListOfShape myMap;
				shapeTool->GetShapes( labels );
				//printf( "number of shapes: %d ", labels.Length() );
				// iterate through shapes making an ancestors map
				//for ( int i = 1; i <= labels.Length(); i++ )
				
				int i = 1;
				bool found = false;
				while ( !found && i <= labels.Length() )
				{
					if ( shapeTool->GetShape( labels.Value( i ), S ) )
					{
						TopExp_Explorer Ex;
						for ( Ex.Init( S, TopAbs_FACE ); Ex.More() && !found ; Ex.Next() )
						{
							TopoDS_Shape curShape = Ex.Current();
							if ( curShape.IsPartner( face ) && ( curShape != face ) )
							{
								if ( S.ShapeType() != 0 )
								{
									found = true;
									//printf( " -- Found a partner!!! %d %d\n", S.ShapeType(), S.HashCode( 100 ) );
									//break;
								}
							}
						} 
					/*
						TopLoc_Location loc;
						S.Location( loc );
						TopExp::MapShapesAndAncestors( S, TopAbs_FACE, S.ShapeType(), myMap);
						if ( myMap.Contains( face ) )
						{
							const TopTools_ListOfShape& listOfShapes = myMap.FindFromKey( face );
							nbAncestors = listOfShapes.Extent();
							printf( "number of ancestors: %d %d %d\n", nbAncestors, S.ShapeType(), S.HashCode( 100 ) );
							if ( nbAncestors > 0 )
								printf( "ancestor: %d %d\n", listOfShapes.First().ShapeType(), listOfShapes.First().HashCode( 100 ) );
						}
						*/
						// myMap.Clear();
					}
					i++;
				}
				
				// check if face is an ancestor of shape
				// search shape for associated label
				if ( shapeTool->Search( S, associatedLabel ) )
				{
					//printf( "found shape associated label\n" );
					if ( shapeTool->IsReference( associatedLabel ) )
					{
						printf( "associated shape is a reference\n" );
						TDF_Label referencedLabel;
						shapeTool->GetReferredShape( associatedLabel, referencedLabel );
						surface_has_colour = extractColour( referencedLabel, colourTool, colour );
					}
					else
					{
						surface_has_colour = extractColour( associatedLabel, colourTool, colour );
					}
				}
			}
			if ( surface_has_colour )
			{
				if ( !colourStack.empty() )
					colour = colourStack.top();
				surface->setColour( colour );
				printf( "Setting face colour to (%d): %s\n", surface_has_colour, Quantity_Color::StringName( colour.Name() ) );
			}
		}
		surface->buildOpenGLRep();
		m_surfaces.push_back( surface );
		for( exEdge.Init(face, TopAbs_EDGE); exEdge.More(); exEdge.Next() )
		{
			TopoDS_Edge topoDSEdge = TopoDS::Edge( exEdge.Current() );
			edge_count++;
			edge = new Edge( topoDSEdge );
			if ( !m_doc.IsNull() )
			{
				if ( extractColour( exEdge.Current(), colourTool, colour ) || surface_has_colour )
				{
					edge->setColour( colour );
					//printf( "Setting edge colour to (%d): %s\n", surface_has_colour, Quantity_Color::StringName( colour.Name() ) );
				}
			}
			//edge->setColour( m_colour );
			// If not a 3D line then maybe a Line on a surface, hopefully!
			if ( !edge->is3D() )
			{
				edge->setTrimmed( face );
			}

			if ( edge->buildOpenGLRep() )
			{
			if ( edge->vertices() == 0 )
				printf( " wtf %d, values: %p, colours: %p, is3D(%d), is2D(%d)\n", edge->count(), edge->vertices(), edge->colours(), edge->is3D(), edge->is2D() );
				m_edges.push_back( edge );
			}
			else
				printf( "Edge failed to build OpenGL representation !!!!\n" );
		}
	}
	//}
	//}
	end = clock();
	if ( nfaces )
	{
		printf( "Face (%d) and edge extraction took %.2f seconds [average: %.4f]\n", nfaces, ( end - start ) / double( CLOCKS_PER_SEC ), ( end - start ) / double( CLOCKS_PER_SEC * nfaces ) );
		printf( "edges running total: %d\n", (int)m_edges.size() );
	}
	else
	{
		printf( "No faces found!!\n" );
	}
}

void TransformOCCModel::healGeometry( TopoDS_Shape shape, double tolerance, bool fixsmalledges,
										 bool fixspotstripfaces, bool sewfaces,
		bool makesolids)
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
					/** \todo write out warning message */;
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

