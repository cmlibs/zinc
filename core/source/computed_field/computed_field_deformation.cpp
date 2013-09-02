/*******************************************************************************
FILE : computed_field_deformation.c

LAST MODIFIED : 24 August 2006

DESCRIPTION :
Implements a number of basic continuum mechanics deformation operations on
computed fields.
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
#include <math.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_deformation.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"

class Computed_field_deformation_package : public Computed_field_type_package
{
};

namespace {

char computed_field_2d_strain_type_string[] = "2d_strain";

class Computed_field_2d_strain : public Computed_field_core
{
public:
	Computed_field_2d_strain() : Computed_field_core()
	{
	};

private:
	virtual bool is_defined_at_location(cmzn_field_cache& cache);

	Computed_field_core *copy()
	{
		return new Computed_field_2d_strain();
	}

	const char *get_type_string()
	{
		return(computed_field_2d_strain_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_2d_strain*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate(cmzn_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();
};

bool Computed_field_2d_strain::is_defined_at_location(cmzn_field_cache& cache)
{
	Field_element_xi_location* element_xi_location;
	// only works for element_xi locations & at least 2-D
	if ((element_xi_location = dynamic_cast<Field_element_xi_location*>(cache.getLocation()))
		&& (2 <= get_FE_element_dimension(element_xi_location->get_element())))
	{
		// check the source fields
		return Computed_field_core::is_defined_at_location(cache);
	}
	return false;
}

int Computed_field_2d_strain::evaluate(cmzn_field_cache& cache, FieldValueCache& inValueCache)
{
	Field_element_xi_location* element_xi_location;
	/* Only works for element_xi locations */
	if (0 != (element_xi_location = dynamic_cast<Field_element_xi_location*>(cache.getLocation())))
	{
		FE_element* element = element_xi_location->get_element();
		int element_dimension = get_FE_element_dimension(element);
		RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
		RealFieldValueCache *deformedCache = RealFieldValueCache::cast(getSourceField(0)->evaluateWithDerivatives(cache, element_dimension));
		RealFieldValueCache *undeformedCache = RealFieldValueCache::cast(getSourceField(1)->evaluateWithDerivatives(cache, element_dimension));
		RealFieldValueCache *fibreCache = RealFieldValueCache::cast(getSourceField(2)->evaluate(cache));
		if (deformedCache && undeformedCache && fibreCache)
		{
			double A,A2,B,B2,cos_fibre_angle,C2,D,dxi_dnu[6],E[4],fibre_angle,
				F_x[6],F_X[6],sin_fibre_angle;
			FE_value def_derivative_xi[9], undef_derivative_xi[9];
			valueCache.derivatives_valid = 0;
			switch(element_dimension)
			{
				case 2:
				{
					def_derivative_xi[0] = deformedCache->derivatives[0];
					def_derivative_xi[1] = deformedCache->derivatives[1];
					def_derivative_xi[3] = deformedCache->derivatives[2];
					def_derivative_xi[4] = deformedCache->derivatives[3];
					def_derivative_xi[6] = deformedCache->derivatives[4];
					def_derivative_xi[7] = deformedCache->derivatives[5];
					undef_derivative_xi[0] = undeformedCache->derivatives[0];
					undef_derivative_xi[1] = undeformedCache->derivatives[1];
					undef_derivative_xi[3] = undeformedCache->derivatives[2];
					undef_derivative_xi[4] = undeformedCache->derivatives[3];
					undef_derivative_xi[6] = undeformedCache->derivatives[4];
					undef_derivative_xi[7] = undeformedCache->derivatives[5];
				} break;
				case 3:
				{
					/* Convert to 2D ignoring xi3, should be able to choose the
						direction that is ignored */
					def_derivative_xi[0] = deformedCache->derivatives[0];
					def_derivative_xi[1] = deformedCache->derivatives[1];
					def_derivative_xi[3] = deformedCache->derivatives[3];
					def_derivative_xi[4] = deformedCache->derivatives[4];
					def_derivative_xi[6] = deformedCache->derivatives[6];
					def_derivative_xi[7] = deformedCache->derivatives[7];
					undef_derivative_xi[0] = undeformedCache->derivatives[0];
					undef_derivative_xi[1] = undeformedCache->derivatives[1];
					undef_derivative_xi[3] = undeformedCache->derivatives[3];
					undef_derivative_xi[4] = undeformedCache->derivatives[4];
					undef_derivative_xi[6] = undeformedCache->derivatives[6];
					undef_derivative_xi[7] = undeformedCache->derivatives[7];
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_evaluate_2d_strain.  Unknown element dimension");
					return 0;
				} break;
			}
			/* do 2D_strain calculation */
			fibre_angle = fibreCache->values[0];

			/* X is the undeformed coordinates
				x is the deformed coordinates
				nu is the fibre coordinate system (2-D,within sheet) */
			/* calculate F_X=dX_dnu and dxi_dnu */
			cos_fibre_angle=cos(fibre_angle);
			sin_fibre_angle=sin(fibre_angle);
			A2=undef_derivative_xi[0]*undef_derivative_xi[0]+
				undef_derivative_xi[3]*undef_derivative_xi[3]+
				undef_derivative_xi[6]*undef_derivative_xi[6];
			A=sqrt(A2);
			B2=undef_derivative_xi[1]*undef_derivative_xi[1]+
				undef_derivative_xi[4]*undef_derivative_xi[4]+
				undef_derivative_xi[7]*undef_derivative_xi[7];
			B=sqrt(B2);
			C2=undef_derivative_xi[0]*undef_derivative_xi[1]+
				undef_derivative_xi[3]*undef_derivative_xi[4]+
				undef_derivative_xi[6]*undef_derivative_xi[7];
			D=sin_fibre_angle*B/(A2*B2-C2*C2);
			dxi_dnu[0] = cos_fibre_angle/A-sin_fibre_angle*C2*D;
			dxi_dnu[1]=D*A2;
			F_X[0]=dxi_dnu[0]*undef_derivative_xi[0]+
				dxi_dnu[1]*undef_derivative_xi[1];
			F_X[2]=dxi_dnu[0]*undef_derivative_xi[3]+
				dxi_dnu[1]*undef_derivative_xi[4];
			F_X[4]=dxi_dnu[0]*undef_derivative_xi[6]+
				dxi_dnu[1]*undef_derivative_xi[7];
			D=cos_fibre_angle*B/(A2*B2-C2*C2);
			dxi_dnu[2]=
				-(sin_fibre_angle/A+cos_fibre_angle*C2*D);
			dxi_dnu[3]=D*A2;
			F_X[1]=dxi_dnu[2]*undef_derivative_xi[0]+
				dxi_dnu[3]*undef_derivative_xi[1];
			F_X[3]=dxi_dnu[2]*undef_derivative_xi[3]+
				dxi_dnu[3]*undef_derivative_xi[4];
			F_X[5]=dxi_dnu[2]*undef_derivative_xi[6]+
				dxi_dnu[3]*undef_derivative_xi[7];
			/* calculate F_x=dx_dnu=dx_dxi*dxi_dnu */
			F_x[0]=dxi_dnu[0]*def_derivative_xi[0]+
				dxi_dnu[1]*def_derivative_xi[1];
			F_x[1]=dxi_dnu[2]*def_derivative_xi[0]+
				dxi_dnu[3]*def_derivative_xi[1];
			F_x[2]=dxi_dnu[0]*def_derivative_xi[3]+
				dxi_dnu[1]*def_derivative_xi[4];
			F_x[3]=dxi_dnu[2]*def_derivative_xi[3]+
				dxi_dnu[3]*def_derivative_xi[4];
			F_x[4]=dxi_dnu[0]*def_derivative_xi[6]+
				dxi_dnu[1]*def_derivative_xi[7];
			F_x[5]=dxi_dnu[2]*def_derivative_xi[6]+
				dxi_dnu[3]*def_derivative_xi[7];
			/* calculate the strain tensor
				E=0.5*(trans(F_x)*F_x-trans(F_X)*F_X) */
			E[0]=0.5*((F_x[0]*F_x[0]+F_x[2]*F_x[2]+
					F_x[4]*F_x[4])-
				(F_X[0]*F_X[0]+F_X[2]*F_X[2]+
					F_X[4]*F_X[4]));
			E[1]=0.5*((F_x[0]*F_x[1]+F_x[2]*F_x[3]+
					F_x[4]*F_x[5])-
				(F_X[0]*F_X[1]+F_X[2]*F_X[3]+
					F_X[4]*F_X[5]));
			E[2]=E[1];
			E[3]=0.5*((F_x[1]*F_x[1]+F_x[3]*F_x[3]+
					F_x[5]*F_x[5])-
				(F_X[1]*F_X[1]+F_X[3]*F_X[3]+
					F_X[5]*F_X[5]));
			valueCache.values[0] = E[0];
			valueCache.values[1] = E[1];
			valueCache.values[2] = E[2];
			valueCache.values[3] = E[3];
			return 1;
		}
	}
	return 0;
}

int Computed_field_2d_strain::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_2d_strain);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    deformed coordinate field : %s\n",field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,
			"    undeformed coordinate field : %s\n",field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE,
			"    fibre angle field : %s\n",field->source_fields[2]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_2d_strain.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_2d_strain */

char *Computed_field_2d_strain::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_2d_strain::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_2d_strain_type_string, &error);
		append_string(&command_string, " deformed_coordinate ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " undeformed_coordinate ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " fibre_angle ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[2], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_2d_strain::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_2d_strain::get_command_string */

} //namespace

/***************************************************************************//**
 * Creates a field which preforms a 2-D strain calculation, combining a
 * deformed_coordinate_field, undeformed_coordinate_field and
 * fibre_angle_field.  Sets the number of components to 4.
 * The <coordinate_field>s must have no more than 3 components.
 */
struct Computed_field *Computed_field_create_2d_strain(
	struct cmzn_field_module *field_module,
	struct Computed_field *deformed_coordinate_field,
	struct Computed_field *undeformed_coordinate_field,
	struct Computed_field *fibre_angle_field)
{
	Computed_field *field = NULL;
	if (field_module && deformed_coordinate_field &&
		(3 >= deformed_coordinate_field->number_of_components) &&
		undeformed_coordinate_field &&
		(3 >= undeformed_coordinate_field->number_of_components) &&
		fibre_angle_field)
	{
		Computed_field *source_fields[3];
		source_fields[0] = deformed_coordinate_field;
		source_fields[1] = undeformed_coordinate_field;
		source_fields[2] = fibre_angle_field;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			/*number_of_components*/4,
			/*number_of_source_fields*/3, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_2d_strain());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_2d_strain.  Invalid argument(s)");
	}

	return (field);
}

int Computed_field_get_type_2d_strain(struct Computed_field *field,
	struct Computed_field **deformed_coordinate_field,
	struct Computed_field **undeformed_coordinate_field,
	struct Computed_field **fibre_angle_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_2D_STRAIN, the undeformed and deformed
coordinate fields and the fibre angle field used by it are returned 
- otherwise an error is reported.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_2d_strain);
	if (field && (dynamic_cast<Computed_field_2d_strain*>(field->core)) &&
		undeformed_coordinate_field && deformed_coordinate_field &&
		fibre_angle_field)
	{
		/* source_fields: 0=deformed_coordinate_field,
			1=undeformed_coordinate_field,
			2=fibre_angle_field */
		*deformed_coordinate_field=field->source_fields[0];
		*undeformed_coordinate_field=field->source_fields[1];
		*fibre_angle_field=field->source_fields[2];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_2d_strain.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_2d_strain */

