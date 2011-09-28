/*****************************************************************************
  FILE : generate_mesh_netgen.cpp
  
  DESCRIPTION :
  This interface is used to call netgen internally inside CMGUI. CMGUI will 
  generate triangular surface and export the surface into netgen. After netgen returns
  the volume mesh, CMGUI will visualize them to the user.
 
==============================================================================*/
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
#include "general/mystring.h"
#include "finite_element/generate_mesh_netgen.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_helper.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_wrappers.h"
}
#include "graphics/triangle_mesh.hpp"


namespace nglib {
#include <nglib.h>
}
using namespace nglib;
// Netgen Meshing Parameters class

struct Generate_netgen_parameters
{   
	double maxh;
	double fineness;
	int secondorder;
	Triangle_mesh *trimesh; 
	char* meshsize_filename;
};/*struct Generate_netgen_parameters*/

int set_netgen_parameters_trimesh(struct Generate_netgen_parameters *para, void *trimesh)
{
    ENTER(set_netgen_parameters_trimesh);
    int return_code=0;
    Triangle_mesh* tri_mesh_tmp;

    if(para==NULL) return return_code;
    if(trimesh==NULL) return return_code;
    tri_mesh_tmp=(Triangle_mesh*)trimesh;
    para->trimesh=tri_mesh_tmp;
    return_code=1;
    
    LEAVE;
    return return_code;
}

int set_netgen_parameters_fineness(struct Generate_netgen_parameters *para, double fineness)
{
    ENTER(set_netgen_parameters_fineness);
    int return_code=0;

    if(para==NULL) return return_code;
     
    para->fineness=fineness;
    return_code=1;
    
    LEAVE;
    return return_code;
}

int set_netgen_parameters_maxh(struct Generate_netgen_parameters *para, double maxh)
{
    ENTER(set_netgen_parameters_maxh);
    int return_code=0;

    if(para==NULL) return return_code;
     
    para->maxh=maxh;
    return_code=1;
    
    LEAVE;
    return return_code;
}

int set_netgen_parameters_secondorder(struct Generate_netgen_parameters *para, int secondorder)
{
    ENTER(set_netgen_parameters_secondorder);
    int return_code=0;

    if(para==NULL) return return_code;
     
    para->secondorder=secondorder;
    return_code=1;
    
    LEAVE;
    return return_code;
}

int set_netgen_parameters_meshsize_filename(struct Generate_netgen_parameters *para,
	const char* meshsize_filename)
{
    int return_code=0;

    ENTER(set_netgen_parameters_meshsize_filename);
    if(para && meshsize_filename)
		{
			para->meshsize_filename=duplicate_string(meshsize_filename);
			return_code=1;
		}
    LEAVE;

    return return_code;
}

/***************************************************************************//**
 * brief Locally restrict the mesh element size at the given point
 *
 * Unlike the function #Ng_RestrictMeshSizeGlobal, this function 
 * allows the user to locally restrict the maximum allowable mesh 
 * size at a given point.
 *
 * The point is specified via its three cartesian co-ordinates.
 *
 * <b>Note</b>: This function only limits the <b>Maximum</b> size 
 * of the elements around the specified point.
 *
 * @param mesh  Pointer to an existing Netgen Mesh structure of 
 *               type #Ng_Mesh
 * @param p  Pointer to an Array of type <i>double</i>, containing 
 *             the three co-ordinates of the point in the form: \n
 *             - p[0] = X co-ordinate
 *             - p[1] = Y co-ordinate
 *             - p[2] = Z co-ordinate
 * @param h  Variable of type <i>double</i>, specifying the maximum
 *             allowable mesh size at that point
 */
int set_netgen_restrict_meshsizepoint(Ng_Mesh *mesh, double *p, double h)
{
    ENTER(set_netgen_restrict_meshsizepoint);
    int return_code=0;

    if(mesh)
		{
			Ng_RestrictMeshSizePoint(mesh, p,h);
			return_code=1;
    }

    LEAVE;
    return return_code;
}

/***************************************************************************//**
 *
 *  Similar to the function set_netgen_restrict_meshsize_Point, this function 
 *  allows the size of elements within a mesh to be locally limited.
 *
 *  However, rather than limit the mesh size at a single point, this 
 *  utility restricts the local mesh size within a 3D Box region, specified 
 *  via the co-ordinates of the two diagonally opposite points of a cuboid.
 *
 *  <b>Note</b>: This function only limits the <b>Maximum</b> size 
 *  of the elements within the specified region.
 *
 *  @param mesh Pointer to an existing Netgen Mesh structure of 
 *              type #Ng_Mesh
 *  @param pmin Pointer to an Array of type <i>double</i>, containing 
 *             the three co-ordinates of the first point of the cuboid: \n
 *              - pmin[0] = X co-ordinate
 *              - pmin[1] = Y co-ordinate
 *              - pmin[2] = Z co-ordinate
 *  @param pmax Pointer to an Array of type <i>double</i>, containing 
 *              the three co-ordinates of the opposite point of the 
 *              cuboid: \n

 *              - pmax[0] = X co-ordinate
 *              - pmax[1] = Y co-ordinate
 *              - pmax[2] = Z co-ordinate
 *  @param h    Variable of type <i>double</i>, specifying the maximum
 *              allowable mesh size at that point
 */
int set_netgen_restrict_meshsizebox(Ng_Mesh *mesh, double *pmin, double *pmax, double h)
{
    ENTER(set_netgen_restrict_meshsizebox);
    int return_code=0;

    if(mesh)
		{     
			Ng_RestrictMeshSizeBox(mesh, pmin, pmax,h);
			return_code=1;
		}
    LEAVE;
    return return_code;
}

struct Generate_netgen_parameters * create_netgen_parameters()
{
    ENTER(create_netgen_parameters);
    struct Generate_netgen_parameters *para;
    
    para= new struct Generate_netgen_parameters();
    
    LEAVE;
    return para;
}

int release_netgen_parameters(struct Generate_netgen_parameters *para)
{
    int return_code=0;

    ENTER(release_netgen_parameters);
    if(para)
		{
			if (para->meshsize_filename)
			{
				DEALLOCATE(para->meshsize_filename);
			}
			delete para;
			return_code=1;
		}
    LEAVE;

    return return_code;
}

int generate_mesh_netgen(struct FE_region *fe_region, void *netgen_para_void)
{
   ENTER(generate_mesh_netgen);
   
   Generate_netgen_parameters *generate_netgen_para = static_cast<Generate_netgen_parameters *>(netgen_para_void);
   Triangle_mesh *trimesh=generate_netgen_para->trimesh;
   int return_code;
   Ng_Mesh * mesh;
   Ng_STL_Geometry * geom;
   Ng_Meshing_Parameters *mp=new Ng_Meshing_Parameters();
   mp->maxh=generate_netgen_para->maxh;
   mp->fineness=generate_netgen_para->fineness;
   mp->secondorder=generate_netgen_para->secondorder;
   mp->meshsize_filename=generate_netgen_para->meshsize_filename;

   Ng_Init();
   geom=Ng_STL_NewGeometry(); 

   const Mesh_triangle_list  triangle_list = trimesh->get_triangle_list();
   Mesh_triangle_list_const_iterator triangle_iter;

   float coord1[3], coord2[3],coord3[3];
	 double dcoord1[3], dcoord2[3], dcoord3[3];

   const Triangle_vertex *vertex1, *vertex2, *vertex3;
   for (triangle_iter = triangle_list.begin(); triangle_iter!=triangle_list.end(); ++triangle_iter)
   {
		 (*triangle_iter)->get_vertexes(&vertex1/*point 1*/,&vertex2/*point 2*/,&vertex3/*point 3*/);
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
		 Ng_STL_AddTriangle(geom/*new geometry*/, dcoord1/*point 1*/, dcoord2/*point 2*/, dcoord3/*point 3*/);
   }

   return_code=Ng_STL_InitSTLGeometry(geom);
   if(return_code!=NG_OK)
	 {
		 Ng_Exit();
		 return 0;
	 }
   mesh = Ng_NewMesh (); 
   return_code=Ng_STL_MakeEdges(geom, mesh, mp);
   if(return_code!=NG_OK)
	 {
		 Ng_Exit();
		 return 0;
	 }

   return_code=Ng_STL_GenerateSurfaceMesh(geom, mesh, mp);
   if(return_code!=NG_OK)
	 {
		 Ng_Exit();
		 return 0;
	 }

   return_code=Ng_GenerateVolumeMesh(mesh,mp);
   if(return_code!=NG_OK)
	 {
		 Ng_Exit();
		 return 0;
	 }

   FE_region_begin_change(fe_region);

   /* create a 3-D coordinate field*/
   FE_field *coordinate_field = FE_field_create_coordinate_3d(fe_region,(char*)"coordinate");
	
   /* create and fill nodes*/
   struct FE_node *template_node = CREATE(FE_node)(
		 /*cm_node_identifier*/1, fe_region, /*template_node*/NULL);
   return_code = define_FE_field_at_node_simple(
		 template_node, coordinate_field, /*number_of_derivatives*/0, /*derivative_value_types*/NULL);
   
   const int number_of_nodes = Ng_GetNP(mesh);
   FE_value coordinates[3];
   double coor_tmp[3];
	 int initial_identifier = FE_region_get_last_FE_node_identifier(fe_region)+1;
   int i;
   for (i = 0; i < number_of_nodes; i++)
   {
		 Ng_GetPoint (mesh, i+1/*index in netgen starts from 1*/, coor_tmp);
		 coordinates[0] = coor_tmp[0];
		 coordinates[1] = coor_tmp[1];
		 coordinates[2] = coor_tmp[2];
		 int identifier = i + initial_identifier;
		 struct FE_node *node = CREATE(FE_node)(identifier, /*fe_region*/NULL, template_node);
		 FE_region_merge_FE_node(fe_region, node);
		 ACCESS(FE_node)(node);
		 int number_of_values_confirmed;
		 return_code=set_FE_nodal_field_FE_value_values(
			 coordinate_field, node, coordinates, &number_of_values_confirmed);
		 DEACCESS(FE_node)(&node);
   }
   DESTROY(FE_node)(&template_node);

   /* establish mode which automates creation of shared faces*/
   FE_region_begin_define_faces(fe_region);

   struct CM_element_information element_identifier;
   FE_element *element;
   FE_element *template_element;
   /* create a tetrahedron with linear simplex field*/
   template_element = FE_element_create_with_simplex_shape(fe_region, /*dimension*/3);
   set_FE_element_number_of_nodes(template_element, 4);
   FE_element_define_field_simple(template_element, coordinate_field, LINEAR_SIMPLEX);

   const int number_of_elements = Ng_GetNE(mesh);
   int nodal_idx[4];   
	 for (i = 0 ; i < number_of_elements; i++)
   {
		 Ng_GetVolumeElement (mesh, i+1, nodal_idx);
		 element_identifier.type = CM_ELEMENT;
		 element_identifier.number = FE_region_get_next_FE_element_identifier(fe_region, /*dimension*/3, i + 1);
		 element = CREATE(FE_element)(&element_identifier, (struct FE_element_shape *)NULL,
			 (struct FE_region *)NULL, template_element);
		 ACCESS(FE_element)(element);
		 /* netgen tet node order gives a left handed element coordinate system,
				hence swap node indices 2 and 3 */
		 return_code=set_FE_element_node(element, 0, FE_region_get_FE_node_from_identifier(fe_region,nodal_idx[0]));
		 return_code=set_FE_element_node(element, 1, FE_region_get_FE_node_from_identifier(fe_region,nodal_idx[1]));
		 return_code=set_FE_element_node(element, 2, FE_region_get_FE_node_from_identifier(fe_region,nodal_idx[3]));
		 return_code=set_FE_element_node(element, 3, FE_region_get_FE_node_from_identifier(fe_region,nodal_idx[2]));
		 FE_region_merge_FE_element_and_faces_and_nodes(fe_region, element);
		 DEACCESS(FE_element)(&element);
   }
   DEACCESS(FE_element)(&template_element);            


   /* must remember to end define faces mode*/
   FE_region_end_define_faces(fe_region);


   DEACCESS(FE_field)(&coordinate_field);

   FE_region_end_change(fe_region);

	 if (mesh)
		 Ng_DeleteMesh (mesh); 
   Ng_Exit();
   LEAVE;
   return return_code;
} /* generate_mesh_netgen */


