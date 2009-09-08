/*****************************************************************************//**
 * FILE : generate_mesh_netgen.cpp
 * 
 * Implements a cmiss field which is an alias for another field, commonly from a
 * different region to make it available locally.
 *
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

extern "C" {
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include "general/debug.h"
#include "finite_element/generate_mesh_netgen.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_helper.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_wrappers.h"
//#include "api/cmiss_node.h"
}

namespace nglib {
#include <nglib.h>
}


struct mesh_para{
int nothing_now;
};

struct triangular_mesh{
int nothing_now;
};

/*
Module types
------------
*/

/***************************************************************************//**
 * create  tet mesh
 */
//void create_test_tet_mesh(struct FE_region *fe_region, Ng_Mesh * mesh)
//{
//	using namespace nglib;
//        // for efficiency cache changes until all finished
//}/*create_test_tet_mesh*/

//FE_element* FE_element_create_with_simplex_shape(struct FE_region *fe_region, int dimension)
//{
//  struct CM_element_information element_identifier;
//  struct FE_element *element;
//  struct FE_element_shape *element_shape;

//  ENTER(create_FE_element_with_tetrahedron_shape);
//  element = (struct FE_element *)NULL;
//  if (fe_region)
//  {
//      int tetrahedron_shape_type[] = { SIMPLEX_SHAPE, 1, 1, SIMPLEX_SHAPE, 1, SIMPLEX_SHAPE };
//      element_shape = CREATE(FE_element_shape)(dimension, tetrahedron_shape_type, fe_region);
//      ACCESS(FE_element_shape)(element_shape);
//      element_identifier.type = CM_ELEMENT;
//      element_identifier.number = 1;
//      element = CREATE(FE_element)(&element_identifier, element_shape, fe_region, (struct FE_element *)NULL);
//      DEACCESS(FE_element_shape)(&element_shape);
//  }
//  else
//  {
//      display_message(ERROR_MESSAGE,
//          "create_FE_element_with_tetrahedron_shape.  Invalid argument(s)");
//  }
//  LEAVE;

//  return (element);
//} /* FE_element_create_with_simplex_shape */


//struct FE_field *create_coordinate_field(struct FE_region* fe_region)
//{
//  struct Coordinate_system coordinate_system;
//  struct FE_field *coordinate_field;
//  char name[]="coordinates";

//  // create FE_field for interpolated coordinates
//  coordinate_field = CREATE(FE_field)(name, fe_region);
//  set_FE_field_type_general(coordinate_field);
//  set_FE_field_number_of_components(coordinate_field, 3);
//  set_FE_field_value_type(coordinate_field, FE_VALUE_VALUE);
//  set_FE_field_CM_field_type(coordinate_field, CM_COORDINATE_FIELD);
//  coordinate_system.type = RECTANGULAR_CARTESIAN;
//  set_FE_field_coordinate_system(coordinate_field, &coordinate_system);
//  return (coordinate_field);
//}

/*FE_field *FE_field_create_coordinate_3d(struct FE_region* fe_region,
	char *name)
{
	FE_field *return_field = NULL;
	
	ENTER(FE_field_create_coordinate_3d);
	if (name)
	{
		// create FE_field for interpolated coordinates
		FE_field *coordinate_field = CREATE(FE_field)(name, fe_region);
		
		// add a reference count
		ACCESS(FE_field)(coordinate_field);

		// this field has real values (as opposed to integer, string)
		set_FE_field_value_type(coordinate_field, FE_VALUE_VALUE);

		// 3 components named x, y, z; rectangular cartesian
		const int number_of_components = 3;
		set_FE_field_number_of_components(coordinate_field, number_of_components);
                const char *component_names[] = { "x", "y", "z" };
		for (int i = 0; i < number_of_components; i++)
		{
			set_FE_field_component_name(coordinate_field, i, (char*)component_names[i]);
		}
		Coordinate_system coordinate_system;
		coordinate_system.type = RECTANGULAR_CARTESIAN;
		set_FE_field_coordinate_system(coordinate_field, &coordinate_system);

		// field type 'general' is nodal-interpolated.
		set_FE_field_type_general(coordinate_field);

		// set flag indicating this is a coordinate field type
		// (helps cmgui automatically choose this field for that purpose)
		set_FE_field_CM_field_type(coordinate_field, CM_COORDINATE_FIELD);

		// see if a matching field exists in fe_region, or add this one
		return_field = FE_region_merge_FE_field(fe_region, coordinate_field);
		ACCESS(FE_field)(return_field);

		// remove reference count to clean up (handles return_field != coordinate_field)
		DEACCESS(FE_field)(&coordinate_field);
	}
	LEAVE;

	return (return_field);
}*/


/***************************************************************************//**
 * export mesh to netgen
 */
int generate_mesh_netgen(struct FE_region *fe_region)//(struct mesh_para* mesh_para,struct triangular_mesh* triangular_mesh)
{
   ENTER(generate_mesh_netgen);
   using namespace nglib;
   //if(mesh_para==NULL||triangular_mesh==NULL) return -1;
   
   int return_code;
   Ng_Mesh * mesh;
   Ng_STL_Geometry * geom;
   Ng_Meshing_Parameters mp;

   ////////////////////////////////
   //for testing
   mp.maxh=100000;
   mp.fineness = 0.5;
   mp.secondorder = 0;
   ////////////////////////////////

   Ng_Init();
   //geom=Ng_STL_NewGeometry();
  

   ////////////////////////////////
   //for testing
   geom=Ng_STL_LoadGeometry("part1.stl");
   ////////////////////////////////

   return_code=Ng_STL_InitSTLGeometry(geom);
   if(return_code!=NG_OK) return return_code;

   mesh = Ng_NewMesh (); 
   return_code=Ng_STL_MakeEdges(geom, mesh, &mp);
   if(return_code!=NG_OK) return return_code;

   return_code=Ng_STL_GenerateSurfaceMesh(geom, mesh, &mp);
   if(return_code!=NG_OK) return return_code;
 
   //this line might be delted in the future
   Ng_SaveMesh (mesh, "surface.vol");
 
   return_code=Ng_GenerateVolumeMesh(mesh,&mp);
   if(return_code!=NG_OK) return return_code;
   
   //this line might be delted in the future
   Ng_SaveMesh (mesh, "volume.vol");

   FE_region_begin_change(fe_region);

   // create a 3-D coordinate field
   FE_field *coordinate_field = FE_field_create_coordinate_3d(fe_region,(char*)"coordinate");
    
   ACCESS(FE_field)(coordinate_field);
   //// create a scalar field
   //char name[]="scalar";
   //FE_field *temp_field = CREATE(FE_field)(name, fe_region);
   //ACCESS(FE_field)(temp_field);
   //set_FE_field_value_type(temp_field, FE_VALUE_VALUE);
   //set_FE_field_number_of_components(temp_field, 1);
   //set_FE_field_type_general(temp_field);
   //set_FE_field_CM_field_type(temp_field, CM_GENERAL_FIELD);
   //FE_field *scalar_field = FE_region_merge_FE_field(fe_region, temp_field);
   //ACCESS(FE_field)(scalar_field);
   //DEACCESS(FE_field)(&temp_field);
	
   // create and fill nodes
   struct FE_node *template_node = CREATE(FE_node)(/*cm_node_identifier*/1, fe_region, /*template_node*/NULL);
   return_code = define_FE_field_at_node_simple(template_node, coordinate_field, /*number_of_derivatives*/0, /*derivative_value_types*/NULL);
   
   // set field values
   //return_code=set_FE_nodal_FE_value_value(template_node, coordinate_field,
   //    /*component_number*/0, /*version*/0, FE_NODAL_VALUE, /*time*/0, 0);
   //return_code=set_FE_nodal_FE_value_value(template_node, coordinate_field,
   //    /*component_number*/1, /*version*/0, FE_NODAL_VALUE, /*time*/0, 0);
   //return_code=set_FE_nodal_FE_value_value(template_node, coordinate_field,
   //    /*component_number*/2, /*version*/0, FE_NODAL_VALUE, /*time*/0, 0);         
	
   const int number_of_nodes = Ng_GetNP(mesh);
   FE_value coordinates[3];
   double coor_tmp[3];
   int i;
   for (i = 0 ; i < number_of_nodes; i++)
   {
       Ng_GetPoint (mesh, i+1/*index in netgen starts from 1*/, coor_tmp);
       coordinates[0] = coor_tmp[0];
       coordinates[1] = coor_tmp[1];
       coordinates[2] = coor_tmp[2];
       int identifier = i + 1;
       struct FE_node *node = CREATE(FE_node)(identifier, /*fe_region*/NULL, template_node);
       FE_region_merge_FE_node(fe_region, node);
       ACCESS(FE_node)(node);
       int number_of_values_confirmed;
       return_code=set_FE_nodal_field_FE_value_values(coordinate_field, node, coordinates, &number_of_values_confirmed);
       DEACCESS(FE_node)(&node);
   }
   DESTROY(FE_node)(&template_node);

   // establish mode which automates creation of shared faces
   FE_region_begin_define_faces(fe_region);

   struct CM_element_information element_identifier;
   FE_element *element;
   FE_element *template_element;
   // create a tetrahedron with linear simplex field
   template_element = FE_element_create_with_simplex_shape(fe_region, /*dimension*/3);
   set_FE_element_number_of_nodes(template_element, 4);
   FE_element_define_field_simple(template_element, coordinate_field, LINEAR_SIMPLEX);

   const int number_of_elements = Ng_GetNE(mesh);
   int nodal_idx[4];
   for (i = 0 ; i < number_of_elements; i++)
   {
       Ng_GetVolumeElement (mesh, i+1, nodal_idx);
       element_identifier.type = CM_ELEMENT;
       element_identifier.number = FE_region_get_next_FE_element_identifier(fe_region, CM_ELEMENT, i);
       element = CREATE(FE_element)(&element_identifier, (struct FE_element_shape *)NULL,
	        (struct FE_region *)NULL, template_element);
       ACCESS(FE_element)(element);
       return_code=set_FE_element_node(element, 0, FE_region_get_FE_node_from_identifier(fe_region,nodal_idx[0]));
       return_code=set_FE_element_node(element, 1, FE_region_get_FE_node_from_identifier(fe_region,nodal_idx[1]));
       return_code=set_FE_element_node(element, 2, FE_region_get_FE_node_from_identifier(fe_region,nodal_idx[2]));
       return_code=set_FE_element_node(element, 3, FE_region_get_FE_node_from_identifier(fe_region,nodal_idx[3]));
       FE_region_merge_FE_element_and_faces_and_nodes(fe_region, element);
       DEACCESS(FE_element)(&element);
   }
   DEACCESS(FE_element)(&template_element);            


   // must remember to end define faces mode
   FE_region_end_define_faces(fe_region);

   //DEACCESS(FE_field)(&scalar_field);
   DEACCESS(FE_field)(&coordinate_field);

   FE_region_end_change(fe_region);

 
   Ng_Exit();
   LEAVE;
   return return_code;
} /* generate_mesh_netgen */


