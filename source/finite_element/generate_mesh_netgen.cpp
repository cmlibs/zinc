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

#include "graphics/triangle_mesh.hpp"
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



/***************************************************************************//**
 * export surface to netgen and take the mesh back to cmgui
 */
int generate_mesh_netgen(struct FE_region *fe_region, void *trimesh_void)//(struct mesh_para* mesh_para,struct triangular_mesh* triangular_mesh)
{
   ENTER(generate_mesh_netgen);
   using namespace nglib;
   //if(mesh_para==NULL||triangular_mesh==NULL) return -1;
   Triangle_mesh *trimesh = static_cast<Triangle_mesh *>(trimesh_void);
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
   geom=Ng_STL_NewGeometry(); 


   const Mesh_triangle_list  triangle_list = trimesh->get_triangle_list();
   Mesh_triangle_list_const_iterator triangle_iter;

   float coord1[3], coord2[3],coord3[3];
	double dcoord1[3], dcoord2[3], dcoord3[3];

   const Triangle_vertex *vertex1, *vertex2, *vertex3;

   for (triangle_iter = triangle_list.begin(); triangle_iter!=triangle_list.end(); ++triangle_iter)
   {
	(*triangle_iter)->get_vertexes(&vertex1,&vertex2,&vertex3);
	vertex1->get_coordinates(coord1, coord1+1,coord1+2);
	vertex2->get_coordinates(coord2, coord2+1,coord2+2);
	vertex3->get_coordinates(coord3, coord3+1,coord3+2);
	dcoord1[0] = (double)coord1[0];
	dcoord1[1] = (double)coord1[1];
	dcoord1[2] = (double)coord1[2];
	dcoord2[0] = (double)coord2[0];
	dcoord2[1] = (double)coord2[1];
	dcoord2[2] = (double)coord2[2];
	dcoord3[0] = (double)coord3[0];
	dcoord3[1] = (double)coord3[1];
	dcoord3[2] = (double)coord3[2];
       Ng_STL_AddTriangle(geom, dcoord1, dcoord2, dcoord3);//no normal now
   }

   ////////////////////////////////
   //for testing
   //geom=Ng_STL_LoadGeometry("part1.stl");
   ////////////////////////////////

   return_code=Ng_STL_InitSTLGeometry(geom);
   if(return_code!=NG_OK) return return_code;

   mesh = Ng_NewMesh (); 
   return_code=Ng_STL_MakeEdges(geom, mesh, &mp);
   if(return_code!=NG_OK) return return_code;

   return_code=Ng_STL_GenerateSurfaceMesh(geom, mesh, &mp);
   if(return_code!=NG_OK) return return_code;
 
   ///////////////////////////////////////////////////////////////////////////
   //this line might be delted in the future
   Ng_SaveMesh (mesh, "surface.vol");
   return_code=Ng_GenerateVolumeMesh(mesh,&mp);
   if(return_code!=NG_OK) return return_code;
   //this line might be delted in the future
   Ng_SaveMesh (mesh, "volume.vol");
   ///////////////////////////////////////////////////////////////////////////

   FE_region_begin_change(fe_region);

   // create a 3-D coordinate field
   FE_field *coordinate_field = FE_field_create_coordinate_3d(fe_region,(char*)"coordinate");
    
   ACCESS(FE_field)(coordinate_field);
	
   // create and fill nodes
   struct FE_node *template_node = CREATE(FE_node)(/*cm_node_identifier*/1, fe_region, /*template_node*/NULL);
   return_code = define_FE_field_at_node_simple(template_node, coordinate_field, /*number_of_derivatives*/0, /*derivative_value_types*/NULL);
   
	
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


