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

#if !defined(CAD_FIELD_LOCATION_HPP)
#define CAD_FIELD_LOCATION_HPP

#include "api/cmiss_field_cad.h"
#include "computed_field/field_location.hpp"
#include "cad/computed_field_cad_topology.h"
#include "cad/element_identifier.h"

class Field_cad_geometry_location : public Field_location
{
protected:
	Cmiss_field_cad_topology_id id;

public:
	Field_cad_geometry_location(Cmiss_field_cad_topology_id id, FE_value time = 0.0, int number_of_derivatives = 0);
	~Field_cad_geometry_location() = 0;

	inline Cmiss_field_cad_topology_id get_id() const {return id;}
	

};

class Field_cad_geometry_surface_location : public Field_cad_geometry_location
{
protected:
	Cmiss_cad_surface_identifier identifier;
	double u;
	double v;

public:
	Field_cad_geometry_surface_location(Cmiss_field_cad_topology_id id, Cmiss_cad_surface_identifier identifier, double u, double v, FE_value time = 0.0, int number_of_derivatives = 0);
	~Field_cad_geometry_surface_location();

   virtual Field_location *clone()
   {
   	return new Field_cad_geometry_surface_location(id, identifier, u, v, time);
   }

	inline Cmiss_cad_surface_identifier get_identifier() const {return identifier;}
	inline double get_u() const {return u;}
	inline double get_v() const {return v;}

};

class Field_cad_geometry_curve_location : public Field_cad_geometry_location
{
protected:
	Cmiss_cad_curve_identifier identifier;
	double s;

public:
	Field_cad_geometry_curve_location(Cmiss_field_cad_topology_id id, Cmiss_cad_curve_identifier identifier, double s, FE_value time = 0.0, int number_of_derivatives = 0);
	~Field_cad_geometry_curve_location();

   virtual Field_location *clone()
   {
   	return new Field_cad_geometry_curve_location(id, identifier, s, time);
   }

	inline Cmiss_cad_curve_identifier get_identifier() const {return identifier;}
	inline double get_s() const {return s;}
};

#endif // !defined(CAD_FIELD_LOCATION_HPP)
