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

#include <time.h>

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
#include <TDataStd_TreeNode.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFDoc_Location.hxx>
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
//#include "api/cmiss_field_cad.h"
//#include "user_interface/message.h"
#include "general/debug.h"
#include "general/mystring.h"
//#include "region/cmiss_region.h"
}

#include "cad/computed_field_cad_topology.h"
#include "cad/computed_field_cad_geometry.h"
#include "cad/computed_field_cad_colour.h"

#include "cad/opencascadeformatreader.h"
//#include "cad/element_identifier.h"

void performLabelAnalysis( Handle_TDocStd_Document doc, const TDF_Label& aLabel );

OpenCascadeImporter::OpenCascadeImporter( const char *fileName )
	: CADImporter( fileName )
{
}


OpenCascadeImporter::~OpenCascadeImporter()
{
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
				//success = remapDocToRegionsAndFields( reader.xDEInformation(), region );
				success = convertDocToRegionsAndFields( reader.xDEInformation(), region );
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

bool OpenCascadeImporter::labelTraversal( Handle_TDocStd_Document xdeDoc, const TDF_Label& label
	, TopLoc_Location location, Cmiss_region_id parent )
{
	Handle_XCAFDoc_ShapeTool shapeTool = XCAFDoc_DocumentTool::ShapeTool( xdeDoc->Main() );
	
	//TCollection_AsciiString entry;
	//TDF_Tool::Entry( label, entry );
	//const char *parent_name = Cmiss_region_get_name(parent);
	////printf( "Creating region '%s', parent name '%s'\n", entry.ToCString(), parent_name );
	//Cmiss_region_id region = 0;
	//if (parent_name != 0 && strstr(entry.ToCString(), parent_name) != 0)
	//{
	//	char *entry_name = duplicate_string(entry.ToCString());
	//	int parent_name_length = strlen(parent_name);
	//	/** Invalid to have non-alpha-numeric character first, so the name pointer is moved on one*/
	//	char *entry_name_short = duplicate_string((entry_name+parent_name_length+1));
	//	region = Cmiss_region_create_child(parent, entry_name_short);
	//	DEALLOCATE(entry_name);
	//	DEALLOCATE(entry_name_short);
	//}
	//else
	//{
	//	region = Cmiss_region_create_child(parent, entry.ToCString());
	//}

	bool addedShape = false;
	TDF_Label referenceLabel;
	shapeTool->GetReferredShape(label, referenceLabel);
	TDF_LabelSequence components;
	if ( shapeTool->GetComponents(label, components, (Standard_Boolean)0) )
	{
		/** If this label is an assembly */
		char *name = getLabelName(label);
		//Cmiss_region_id region = Cmiss_region_create_child(parent, name);
		Cmiss_region_id region = Cmiss_region_create_subregion(parent, name);
		free(name);
		if ( !region )
		{
			display_message(ERROR_MESSAGE, "OpenCascadeImporter.  "
				"Unable to create child region.");
			return false;
		}

		for ( int i = 1; i <= components.Length(); i++ )
		{
			addedShape = labelTraversal(xdeDoc, components.Value(i), location, region);
		}
		DEACCESS(Cmiss_region)(&region);
	}
	else if ( !referenceLabel.IsNull() && shapeTool->GetComponents(referenceLabel, components) )
	{
		/** else if this label is a reference for an assembly */
		/** I'm expecting to have to propogate a location here */
		char *name = getLabelName(referenceLabel);
		//Cmiss_region_id region = Cmiss_region_create_child(parent, name);
		Cmiss_region_id region = Cmiss_region_create_subregion(parent, name);
		free(name);
		if ( !region )
		{
			display_message(ERROR_MESSAGE, "OpenCascadeImporter.  "
				"Unable to create child region.");
			return false;
		}

		TopLoc_Location currentLocation = shapeTool->GetLocation(label);
		location = location.Multiplied(currentLocation);

		for ( int i = 1; i <= components.Length(); i++ )
		{
			addedShape = labelTraversal(xdeDoc, components.Value(i), location, region);
		}
		DEACCESS(Cmiss_region)(&region);
	}
	else
	{
		/** else this label is just a shape and we can add it */
		addedShape = addShapeToRegion( xdeDoc, label, location, parent );
	}

	return addedShape;
}

void performLabelAnalysis( Handle_TDocStd_Document xdeDoc, const TDF_Label& aLabel )
{
	Handle_XCAFDoc_ShapeTool shapeTool = XCAFDoc_DocumentTool::ShapeTool( xdeDoc->Main() );
	Handle_XCAFDoc_ColorTool colorTool = XCAFDoc_DocumentTool::ColorTool( xdeDoc->Main() );
	//XCAFDoc_Location::Get();
	TCollection_AsciiString entry;
	TDF_Tool::Entry( aLabel, entry );
	std::cout << entry.ToCString() << " ";
	if ( shapeTool->IsAssembly(aLabel) )
	{
		std::cout << "- Assembly ";
	}
	if ( shapeTool->IsComponent(aLabel) )
	{
		std::cout << "- Component ";
	}
	if ( shapeTool->IsCompound(aLabel) )
	{
		std::cout << "- Compound ";
	}
	if ( shapeTool->IsFree(aLabel) )
	{
		std::cout << "- Free ";
	}
	if ( shapeTool->IsReference(aLabel) )
	{
		std::cout << "- Reference ";
	}
	if ( shapeTool->IsShape(aLabel) )
	{
		std::cout << "- Shape ";
	}
	if ( shapeTool->IsSimpleShape(aLabel) )
	{
		std::cout << "- SimpleShape ";
	}
	if ( shapeTool->IsSubShape(aLabel) )
	{
		std::cout << "- SubShape ";
	}
	if ( shapeTool->IsTopLevel(aLabel) )
	{
		std::cout << "- TopLevel ";
	}
	TDF_LabelSequence labels;
	if ( shapeTool->GetUsers(aLabel, labels) )
	{
		std::cout << "- Users ";
	}
	TopLoc_Location location = shapeTool->GetLocation(aLabel);
	if ( !location.IsIdentity() )
	{
		std::cout << "- Location ";
	}
	Quantity_Color color;
	if ( colorTool->GetColor(aLabel, color) )
	{
		std::cout << "- color ";
	}
	if ( aLabel.HasChild() )
	{
		std::cout << "- Parent ";
	}
	if ( aLabel.HasAttribute() )
	{
		
		std::cout << "- Attribute " << aLabel.NbAttributes() << " ";
		//for (int i = 1; i <= aLabel.NbAttributes(); i++ )
		//{
		
			Handle_TDataStd_Name name = 0;
			if ( aLabel.FindAttribute(TDataStd_Name::GetID(), name) )
			{
				std::cout << " Found " << aLabel.Depth() << " " << name->Get();
			}
			//delete name;
		//}
	}
	std::cout << std::endl;
}

Cmiss_cad_colour::Cmiss_cad_colour_type examineShapeForColor( Handle_TDocStd_Document xdeDoc, const TopoDS_Shape& shape, TopAbs_ShapeEnum type )
{
	XCAFDoc_ColorType colorTypes[3] = {XCAFDoc_ColorGen, XCAFDoc_ColorSurf, XCAFDoc_ColorCurv};
	Handle_XCAFDoc_ColorTool colorTool = XCAFDoc_DocumentTool::ColorTool( xdeDoc->Main() );
	Quantity_Color aColor;
	for ( int j = 0; j < 3; j++ )
	{
		if ( colorTool->IsSet(shape, colorTypes[j]) )
		{
			colorTool->GetColor(shape, colorTypes[j], aColor);
			//std::cout << "Yes we have a colour " << j << " " << type << " and it is ";
			//std::cout << "[ " << aColor._CSFDB_GetQuantity_ColorMyRed() << ", ";
			//std::cout << aColor._CSFDB_GetQuantity_ColorMyGreen() << ", ";
			//std::cout << aColor._CSFDB_GetQuantity_ColorMyBlue() << " ]" << std::endl;
			return (Cmiss_cad_colour::Cmiss_cad_colour_type)j;
		}
	}

	return Cmiss_cad_colour::CMISS_CAD_COLOUR_NOT_DEFINED;
}

Cmiss_cad_colour::Cmiss_cad_colour_type examineShapeForColor( Handle_TDocStd_Document xdeDoc, const TopoDS_Shape& shape, Quantity_Color &aColor )
{
	XCAFDoc_ColorType colorTypes[3] = {XCAFDoc_ColorGen, XCAFDoc_ColorSurf, XCAFDoc_ColorCurv};
	Handle_XCAFDoc_ColorTool colorTool = XCAFDoc_DocumentTool::ColorTool( xdeDoc->Main() );
	//Quantity_Color aColor;
	for ( int j = 0; j < 3; j++ )
	{
		if ( colorTool->IsSet(shape, colorTypes[j]) )
		{
			colorTool->GetColor(shape, colorTypes[j], aColor);
			std::cout << "Colour type " << j << " and it's value is ";
			std::cout << "[ " << aColor.Red() << ", ";
			std::cout << aColor.Green() << ", ";
			std::cout << aColor.Blue() << " ]" << std::endl;
			return (Cmiss_cad_colour::Cmiss_cad_colour_type)j;
		}
	}

	return Cmiss_cad_colour::CMISS_CAD_COLOUR_NOT_DEFINED;
}

void updateColourMapFromShape( Handle_TDocStd_Document xdeDoc, Cad_colour_map& colourMap, const TopoDS_Shape& shape, bool usingReferredShape = false)
{
	//Cad_colour_map colourMap;
	Quantity_Color color = Quantity_NOC_WHITE;
	Cmiss_cad_colour::Cmiss_cad_colour_type colour_type = Cmiss_cad_colour::CMISS_CAD_COLOUR_NOT_DEFINED;
	{
		//std::cout << "Color hunt (referred " << usingReferredShape << " )" << std::endl;
		TopExp_Explorer Ex1, Ex2, Ex3, Ex4, Ex5, Ex6;
		int solid_index = 0;
		for (Ex1.Init(shape,TopAbs_SOLID); Ex1.More(); Ex1.Next()) 
		{
			examineShapeForColor(xdeDoc, Ex1.Current(), TopAbs_SOLID);
			colour_type = examineShapeForColor(xdeDoc, Ex1.Current(), color);
			//printf("Solid colour type: %d\n", colour_type);
			if (colour_type != Cmiss_cad_colour::CMISS_CAD_COLOUR_NOT_DEFINED)
			{
				Cad_topology_primitive_identifier cad_id;
				Cmiss_cad_colour cad_colour(colour_type, color);
				//printf("Adding to colour map\n");
				colourMap.insert(std::pair<Cad_topology_primitive_identifier,Cmiss_cad_colour>(cad_id, cad_colour));
			}
			solid_index++;
			for (Ex2.Init(Ex1.Current(),TopAbs_SHELL); Ex2.More(); Ex2.Next()) 
			{
				examineShapeForColor(xdeDoc, Ex2.Current(), TopAbs_SHELL);
				int face_index = 0;
				for (Ex3.Init(Ex2.Current(),TopAbs_FACE); Ex3.More(); Ex3.Next()) 
				{
					printf("Checking face %d, on surface %d\n", face_index, solid_index);
					examineShapeForColor(xdeDoc, Ex3.Current(), TopAbs_FACE);
					colour_type = examineShapeForColor(xdeDoc, Ex3.Current(), color);
					//printf("Face colour type: %d\n", colour_type);
					if (colour_type != Cmiss_cad_colour::CMISS_CAD_COLOUR_NOT_DEFINED)
					{
						Cad_topology_primitive_identifier cad_id(face_index);
						Cmiss_cad_colour cad_colour(colour_type, color);
						//printf("adding to map face_index %d", face_index);
						colourMap.insert(std::pair<Cad_topology_primitive_identifier,Cmiss_cad_colour>(cad_id, cad_colour));
					}
					face_index++;
					for (Ex4.Init(Ex3.Current(),TopAbs_WIRE); Ex4.More(); Ex4.Next()) 
					{
						examineShapeForColor(xdeDoc, Ex4.Current(), TopAbs_WIRE);
						int edge_index = 0;
						for (Ex5.Init(Ex4.Current(),TopAbs_EDGE); Ex5.More(); Ex5.Next()) 
						{
							examineShapeForColor(xdeDoc, Ex5.Current(), TopAbs_EDGE);
							colour_type = examineShapeForColor(xdeDoc, Ex4.Current(), color);
							//printf("Edge colour type: %d\n", colour_type);
							if (colour_type != Cmiss_cad_colour::CMISS_CAD_COLOUR_NOT_DEFINED)
							{
								Cad_topology_primitive_identifier cad_id(face_index, edge_index);
								Cmiss_cad_colour cad_colour(colour_type, color);
								//printf("adding to map face_index %d, %d\n", face_index, edge_index);
								colourMap.insert(std::pair<Cad_topology_primitive_identifier,Cmiss_cad_colour>(cad_id, cad_colour));
							}
							edge_index++;
							for (Ex6.Init(Ex5.Current(),TopAbs_VERTEX); Ex6.More(); Ex6.Next()) 
							{
								examineShapeForColor(xdeDoc, Ex6.Current(), TopAbs_VERTEX);
							}
						}
					}
				}
			}
		}
	}

	//printf("Colour map size %d\n", colourMap.size());
	//return colourMap;
}

Cad_colour_map createColourMap( Handle_TDocStd_Document xdeDoc, const TDF_Label& label )
{
	Handle_XCAFDoc_ShapeTool shapeTool = XCAFDoc_DocumentTool::ShapeTool( xdeDoc->Main() );

	Cad_colour_map colourMap;
	// First look at shape itself
	TopoDS_Shape shape = shapeTool->GetShape(label);
	updateColourMapFromShape(xdeDoc, colourMap, shape);

	// Second look at referred shape
	TDF_Label referenceLabel;
	shapeTool->GetReferredShape(label, referenceLabel);
	if ( !referenceLabel.IsNull() )
	{
		TopoDS_Shape referredShape = shapeTool->GetShape(referenceLabel);
		updateColourMapFromShape(xdeDoc, colourMap, referredShape, true);
	}
	
	printf("colour map size %d\n", colourMap.size());

	return colourMap;
}

char *OpenCascadeImporter::getLabelName(const TDF_Label& label)
{
	char *string_name = 0;
	Handle_TDataStd_Name name = 0;
	if ( label.FindAttribute(TDataStd_Name::GetID(), name) )
	{
		string_name = (char *) malloc( ( name->Get().LengthOfCString() + 1 ) * sizeof( char ) );
		name->Get().ToUTF8CString( string_name );
	}

	return string_name;
}

bool OpenCascadeImporter::addShapeToRegion( Handle_TDocStd_Document xdeDoc, const TDF_Label& label, TopLoc_Location location, Cmiss_region_id parent )
{
	bool addedShape = false;
	Handle_XCAFDoc_ShapeTool shapeTool = XCAFDoc_DocumentTool::ShapeTool( xdeDoc->Main() );
	TopoDS_Shape shape;
	if ( ! shapeTool->GetShape(label, shape) || shape.IsNull() )
		return addedShape;

	// Catch compounds with no topological shapes
	if (shape.ShapeType() == TopAbs_COMPOUND)
	{
		display_message(INFORMATION_MESSAGE, "addShapeToRegion() test for COMPOUND shape, shouldn't get here because I am already looking for it earlier???\n");
		performLabelAnalysis(xdeDoc, label);
		TDF_Label referenceLabel;
		shapeTool->GetReferredShape(label, referenceLabel);
		performLabelAnalysis(xdeDoc, referenceLabel);
		TopoDS_Iterator anExplor(shape);
		if (!anExplor.More())
		{
			display_message(INFORMATION_MESSAGE, "and it contains no shapes\n");
			return addedShape;
		}
	}

	char *string_name = 0;
	if (shapeTool->IsReference(label))
	{
		printf("Getting a name from a referred label.\n");
		TDF_Label referenceLabel;
		shapeTool->GetReferredShape(label, referenceLabel);
		string_name = getLabelName(referenceLabel);
	}
	else
	{
		string_name = getLabelName(label);
	}

	Cad_colour_map colourMap = createColourMap(xdeDoc, label);
	{
		shape.Move(location);
		TopologicalShape* ts = new TopologicalShape( shape, colourMap );

		// Create field and add to current region
		char *top_name = 0, *geo_name = 0, *col_name = 0;
		int status = 0;
		//printf( "Creating field in region\n" );
		Cmiss_field_module_id field_module = Cmiss_region_get_field_module(parent);
		Cmiss_field_id cad_topology_field = Cmiss_field_module_create_cad_topology( field_module, ts );
		if (string_name && (strlen(string_name) > 0) )
		{
			Cmiss_field_set_name(cad_topology_field, string_name);
		}
		else
		{
			int field_number = 1;
			char name_buffer[50];
			int n = sprintf(name_buffer, "cad_field%d", field_number);
			while (Cmiss_field_set_name(cad_topology_field, name_buffer) == 0)
			{
				field_number++;
				n = sprintf(name_buffer, "cad_field%d", field_number);
			}
		}
		Cmiss_field_set_persistent(cad_topology_field, 1);
		Cmiss_field_cad_topology_id cad_topology = Cmiss_field_cast_cad_topology( cad_topology_field );

		// Create shape gs
		GeometricShape* gs = new GeometricShape();
		ts->tessellate(gs);
		Cmiss_field_cad_topology_set_geometric_shape(cad_topology, gs);
		// sketchy:
		Cmiss_field_destroy((Cmiss_field_id *)&cad_topology);

		GET_NAME(Computed_field)(cad_topology_field, &top_name);
		append_string(&geo_name, top_name, &status);
		append_string(&geo_name, "-geo", &status);
		append_string(&col_name, top_name, &status);
		append_string(&col_name, "-col", &status);

		Cmiss_field_id cad_geometry_field = Computed_field_module_create_cad_geometry(field_module, cad_topology_field);
		Cmiss_field_set_name(cad_geometry_field, geo_name);
		Cmiss_field_set_persistent(cad_geometry_field, 1);

		Cmiss_field_id cad_colour_field = Computed_field_module_create_cad_colour(field_module, cad_topology_field);
		Cmiss_field_set_name(cad_colour_field, col_name);
		Cmiss_field_set_persistent(cad_colour_field, 1);

		if ( cad_topology_field && cad_geometry_field && cad_colour_field )
		{
			addedShape = true;
		}
		Cmiss_field_destroy(&cad_colour_field);
		Cmiss_field_destroy(&cad_geometry_field);
		Cmiss_field_destroy(&cad_topology_field);

		DEALLOCATE(top_name);
		DEALLOCATE(geo_name);
		DEALLOCATE(col_name);

		Cmiss_field_module_destroy(&field_module);

	}

	if (string_name)
		free(string_name);

	return addedShape;
}

int iterateOverLabel( Handle_TDocStd_Document xdeDoc, const TDF_Label& label, int depth )
{
	depth++;

	Handle_XCAFDoc_ShapeTool shapeTool = XCAFDoc_DocumentTool::ShapeTool( xdeDoc->Main() );
	TDF_LabelSequence labels;
	performLabelAnalysis(xdeDoc, label);
	if ( shapeTool->IsAssembly(label) )
	{
		TDF_LabelSequence components;
		Standard_Boolean isAssembly = shapeTool->GetComponents(label, components, (Standard_Boolean)0);

		for ( int i = 1; i <= components.Length(); i++ )
		{
			if ( depth < 10 )
				iterateOverLabel(xdeDoc, components.Value(i), depth);
		}
	}
	//else if ( shapeTool->IsReference(label) )
	//{
	//	TDF_Label referenceLabel;
	//	shapeTool->GetReferredShape(label, referenceLabel);
	//	if ( depth < 10 )
	//		iterateOverLabel(xdeDoc, referenceLabel, depth);
	//}
	//else if ( shapeTool->GetSubShapes(label, labels) )
	//{
	//	std::cout << "Sub-shapes" << std::endl;
	//	for ( int i = 1; i <= labels.Length(); i++ )
	//	{
	//		if ( depth < 10 )
	//			iterateOverLabel(xdeDoc, labels.Value(i), depth);
	//	}
	//}

	{
		std::cout << "Color hunt ";
		performLabelAnalysis(xdeDoc, label);
		TopExp_Explorer Ex1, Ex2, Ex3, Ex4, Ex5, Ex6;
		TopoDS_Shape shape = shapeTool->GetShape(label);
		for (Ex1.Init(shape,TopAbs_SOLID); Ex1.More(); Ex1.Next()) 
		{
			examineShapeForColor(xdeDoc, Ex1.Current(), TopAbs_SOLID);
			if ( shapeTool->IsReference(label) )
			{
				TDF_Label referenceLabel;
				shapeTool->GetReferredShape(label, referenceLabel);
				TopoDS_Shape referenceShape = shapeTool->GetShape(referenceLabel);
				examineShapeForColor(xdeDoc, referenceShape, TopAbs_SOLID);
			}
			for (Ex2.Init(Ex1.Current(),TopAbs_SHELL); Ex2.More(); Ex2.Next()) 
			{
				examineShapeForColor(xdeDoc, Ex2.Current(), TopAbs_SHELL);
				for (Ex3.Init(Ex2.Current(),TopAbs_FACE); Ex3.More(); Ex3.Next()) 
				{
					examineShapeForColor(xdeDoc, Ex3.Current(), TopAbs_FACE);
					for (Ex4.Init(Ex3.Current(),TopAbs_WIRE); Ex4.More(); Ex4.Next()) 
					{
						examineShapeForColor(xdeDoc, Ex4.Current(), TopAbs_WIRE);
						for (Ex5.Init(Ex4.Current(),TopAbs_EDGE); Ex5.More(); Ex5.Next()) 
						{
							examineShapeForColor(xdeDoc, Ex5.Current(), TopAbs_EDGE);
							for (Ex6.Init(Ex5.Current(),TopAbs_VERTEX); Ex6.More(); Ex6.Next()) 
							{
								examineShapeForColor(xdeDoc, Ex6.Current(), TopAbs_VERTEX);
							}
						}
					}
				}
			}
		}
	}

	return depth;
}

bool OpenCascadeImporter::convertDocToRegionsAndFields( Handle_TDocStd_Document xdeDoc, Cmiss_region_id region )
{
	bool addedShape = false;
	if ( !xdeDoc )
		return addedShape;

	Handle_XCAFDoc_ShapeTool shapeTool = XCAFDoc_DocumentTool::ShapeTool( xdeDoc->Main() );
	Handle_XCAFDoc_ColorTool colorTool = XCAFDoc_DocumentTool::ColorTool( xdeDoc->Main() );

	TopLoc_Location location;
	TDF_LabelSequence freeShapes;
	shapeTool->GetFreeShapes( freeShapes );

#if defined(DEBUG)
	Quantity_Color aColor;
	TDF_LabelSequence colLabels;
	colorTool->GetColors(colLabels);
	printf( "Colours in xdeDoc\n");
	for ( int i = 1; i <= colLabels.Length(); i++)
	{
		const TDF_Label& aLabel = colLabels.Value(i);
		if ( colorTool->GetColor(aLabel, aColor) )
		{
			TCollection_AsciiString entry;
			TDF_Tool::Entry( aLabel, entry );
			Standard_CString cString = entry.ToCString();
			printf("%d. %5.3f %5.3f %5.3f (%s)\n",i , aColor._CSFDB_GetQuantity_ColorMyRed(),
				aColor._CSFDB_GetQuantity_ColorMyGreen(), aColor._CSFDB_GetQuantity_ColorMyBlue(), cString);
			//std::cout << "(" << cString << ")" << std::endl;
		}
	}
	printf("\n\n");
#endif /* defined(DEBUG) */

	Cmiss_region_begin_change(region);
	for ( Standard_Integer i = 1; i <= freeShapes.Length(); i++ )
	{
		TDF_Label label = freeShapes.Value( i );

		TCollection_AsciiString entry;
		TDF_Tool::Entry( label, entry );
#if defined(DEBUG)
		printf( "root region '%s'\n", entry.ToCString() );
#endif
		if ( label.IsNull() )
			continue;

		//std::cout << std::endl << std::endl << "Iterate over label" << std::endl << std::endl;
		//iterateOverLabel( xdeDoc, label, 0 );
		//printf("\n\n");

		addedShape = labelTraversal( xdeDoc, label, location, region );
	}
	Cmiss_region_end_change(region);

	return addedShape;
}

int Cmiss_region_import_cad_file(struct Cmiss_region *region, const char *file_name)
{
	int return_code = 0;

	if (region && file_name)
	{
		OpenCascadeImporter occImporter( file_name );
		if ( !occImporter.import( region ) )
		{
			display_message( ERROR_MESSAGE, "Failed to load %s into region %s", file_name, Cmiss_region_get_name(region) );
		}
		else
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_import_cad_file.  Invalid argument(s)\n");
		return_code = 0;
	}
	return return_code;
}

