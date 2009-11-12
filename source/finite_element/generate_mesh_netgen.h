/***************************************************************************//**
 * FILE : generate_mesh_netgen.h
 *
 * Convenience functions for making simple finite element fields.
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
 * Portions created by the Initial Developer are Copyright (C) 2009
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
#if !defined (GENERATE_MESN_NETGEN_H)
#define GENERATE_MESH_NETGEN_H

struct Generate_netgen_parameters;

/***************************************************************************//**
 * Calls create_netgen_parameters for creating a parameter set for netgen.
 * 
 * @param para  Pointer to a parameter set for netgen.
 * @return  1 if creating is successful
 *   0 if creating is failed
 */
struct Generate_netgen_parameters * create_netgen_parameters();

/***************************************************************************//**
 * Calls release_netgen_parameters for releasing a parameter set for netgen.
 * 
 * @param para  Pointer to a parameter set for netgen.
 * @return  1 if releasing is successful
 *   0 if releasing is failed
 */
int release_netgen_parameters(struct Generate_netgen_parameters *para);

/***************************************************************************//**
 * Calls set_netgen_parameters_secondorder for setting the secondorder switch
 * 
 * @param para Pointer to a parameter set for netgen.
 * @param secondorder is switch for secondorder
 * @return  1 if setting is successful
 *   0 if setting is failed
 */
int set_netgen_parameters_secondorder(struct Generate_netgen_parameters *para, int secondorder);

/***************************************************************************//**
 * Calls set_netgen_parameters_trimesh for setting the triangular surface mesh 
 * 
 * @param para  Pointer to a parameter set for netgen.
 * @param trimesh  Pointer to a triangular surface mesh for netgen.
 * @return  1 if setting is successful
 *   0 if setting is failed
 */
int set_netgen_parameters_trimesh(struct Generate_netgen_parameters *para, void *trimesh);

/***************************************************************************//**
 * Calls set_netgen_parameters_trimesh for setting the fineness of volumetric mesh
 * 
 * @param para  Pointer to a parameter set for netgen.
 * @param fineness  The fineness of volumetric mesh
 * @return  1 if setting is successful
 *   0 if setting is failed
 */
int set_netgen_parameters_fineness(struct Generate_netgen_parameters *para, double fineness);

/***************************************************************************//**
 * Calls set_netgen_parameters_maxh for setting the maximum length of volumetric element 
 * 
 * @param para  Pointer to a parameter set for netgen.
 * @param maxh  The maximum length of volumetric element
 * @return  1 if setting is successful
 *   0 if setting is failed
 */
int set_netgen_parameters_maxh(struct Generate_netgen_parameters *para, double maxh);

/***************************************************************************//**
 * Calls generate_mesh_netgen for invoking netgen and will visualize the 3D mesh automatically.
 * 
 * @param fe_region  Pointer to finite element which must be initialised to contain the
 *   meshing object. It will not change in this call.
 * @param netgen_para_void  Pointer to parameters for the specification of meshing. It comes with 
 *   such information:pointer to the triangular surface mesh, parameters to control the quality of
 *   volume mesh 
 * @return  1 if meshsing is successful
 *   0 if meshing is failed
 */
int generate_mesh_netgen(struct FE_region *fe_region, void *netgen_para_void);

#endif /* GENERATE_MESH_NETGEN_H */

