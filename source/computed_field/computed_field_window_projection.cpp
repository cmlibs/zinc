/*******************************************************************************
FILE : computed_field_window_projection.c

LAST MODIFIED : 25 August 2006

DESCRIPTION :
Implements a computed_field which maintains a graphics transformation 
equivalent to the scene_viewer assigned to it.
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
extern "C" {
#include "computed_field/computed_field.h"
}
#include "computed_field/computed_field_private.hpp"
extern "C" {
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/matrix_vector.h"
#include "general/mystring.h"
#include "graphics/graphics_window.h"
#include "graphics/scene_viewer.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_window_projection.h"
}

class Computed_field_window_projection_package : public Computed_field_type_package
{
public:
	struct MANAGER(Computed_field) *computed_field_manager;
	struct MANAGER(Graphics_window) *graphics_window_manager;
};

namespace {

char computed_field_window_projection_type_string[] = "window_projection";

void Computed_field_window_projection_scene_viewer_callback(
	struct Scene_viewer *scene_viewer, void *dummy_void, void *field_void);

void Computed_field_window_projection_scene_viewer_destroy_callback(
	struct Scene_viewer *scene_viewer, void *dummy_void, void *field_void);

class Computed_field_window_projection : public Computed_field_core
{
public:
	/* Hold onto the graphics window name so we can write out the command. */
	char *graphics_window_name;
	int pane_number;

	/* The scene_viewer is not accessed by the computed field so that we
		can still destroy the window and so the computed field responds to
		a destroy callback and then must evaluate correctly with a NULL scene_viewer */
	struct Scene_viewer *scene_viewer;

	enum Computed_field_window_projection_type projection_type;

	struct MANAGER(Computed_field) *computed_field_manager;

	/* This flag indicates if the field has registered for callbacks with the
		scene_viewer */
	int scene_viewer_callback_flag;

	double *projection_matrix;

	Computed_field_window_projection(Computed_field *field, 
		char *graphics_window_name, int pane_number, 
		Scene_viewer *scene_viewer, 
		enum Computed_field_window_projection_type projection_type,
		struct MANAGER(Computed_field) *computed_field_manager) : 
		Computed_field_core(field), 
		graphics_window_name(duplicate_string(graphics_window_name)),
		pane_number(pane_number), scene_viewer(scene_viewer),
		projection_type(projection_type), computed_field_manager(computed_field_manager)
	{
		scene_viewer_callback_flag = 0;
		projection_matrix = (double*)NULL;
		Scene_viewer_add_destroy_callback(scene_viewer, 
			Computed_field_window_projection_scene_viewer_destroy_callback,
			(void *)field);
	};

	~Computed_field_window_projection();

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_window_projection(new_parent,
			graphics_window_name, pane_number, scene_viewer, projection_type,
			computed_field_manager);
	}

	char *get_type_string()
	{
		return(computed_field_window_projection_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	char *projection_type_string(
		enum Computed_field_window_projection_type projection_type);

	int calculate_matrix();

	int evaluate_projection_matrix(
		int element_dimension, int calculate_derivatives);

	int set_values_at_location(Field_location* location, FE_value *values);

	int find_element_xi( 
		FE_value *values, int number_of_values, struct FE_element **element,
		FE_value *xi, int element_dimension, struct Cmiss_region *search_region);
};

char *Computed_field_window_projection::projection_type_string(
	enum Computed_field_window_projection_type projection_type)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Returns a string label for the <projection_type>, used in widgets and parsing.
NOTE: Calling function must not deallocate returned string.
==============================================================================*/
{
	char *return_string;

	ENTER(Computed_field_window_projection_type_string);
	switch (projection_type)
	{
		case NDC_PROJECTION:
		{
			return_string="ndc_projection";
		} break;
		case TEXTURE_PROJECTION:
		{
			return_string="texture_projection";
		} break;
		case VIEWPORT_PROJECTION:
		{
			return_string="viewport_projection";
		} break;
		case INVERSE_NDC_PROJECTION:
		{
			return_string="inverse_ndc_projection";
		} break;
		case INVERSE_TEXTURE_PROJECTION:
		{
			return_string="inverse_texture_projection";
		} break;
		case INVERSE_VIEWPORT_PROJECTION:
		{
			return_string="inverse_viewport_projection";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_window_projection_type_string.  "
				"Unknown projection type");
			return_string=(char *)NULL;
		}
	}
	LEAVE;

	return (return_string);
} /* Computed_field_window_projection::projection_type_string */

int Computed_field_window_projection::calculate_matrix()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	double lu_d, modelview_matrix[16], window_projection_matrix[16],
		texture_projection_matrix[16], total_projection_matrix[16], 
		viewport_matrix[16], viewport_left, viewport_top, 
		viewport_pixels_per_unit_x, viewport_pixels_per_unit_y,
		bk_texture_left, bk_texture_top, bk_texture_width, bk_texture_height,
		bk_texture_max_pixels_per_polygon;
	float distortion_centre_x, distortion_centre_y, distortion_factor_k1,
		texture_width, texture_height, texture_depth;
	int bk_texture_undistort_on, i, j, k, lu_index[4], return_code,
		viewport_width, viewport_height;
	struct Texture *texture;

	ENTER(Computed_field_window_projection_calculate_matrix);
	if (field)
	{
		if (projection_matrix || ALLOCATE(projection_matrix, double, 16))
		{
			return_code = 1;
			/* make sure we are getting scene viewer callbacks once the
				 projection matrix exists */
			if (!scene_viewer_callback_flag)
			{
				scene_viewer_callback_flag = 
					Scene_viewer_add_transform_callback(scene_viewer, 
						Computed_field_window_projection_scene_viewer_callback,
						(void *)field);
			}
			if (Scene_viewer_get_modelview_matrix(scene_viewer,modelview_matrix)&&
				Scene_viewer_get_window_projection_matrix(scene_viewer,
					window_projection_matrix))
			{
				/* Multiply these matrices */
				for (i=0;i<4;i++)
				{
					for (j=0;j<4;j++)
					{
						total_projection_matrix[i * 4 + j] = 0.0;
						for (k=0;k<4;k++)
						{
							total_projection_matrix[i * 4 + j] += 
								window_projection_matrix[i * 4 + k]	*
								modelview_matrix[k * 4 + j];
						}
					}
				}
				if (projection_type == NDC_PROJECTION)
				{
					/* ndc_projection */
					for (i=0;i<16;i++)
					{
						projection_matrix[i] = total_projection_matrix[i];
					}
				}
				else if (projection_type == INVERSE_NDC_PROJECTION)
				{
					if (LU_decompose(/* dimension */4, total_projection_matrix,
						lu_index, &lu_d,/*singular_tolerance*/1.0e-12))
					{
						for (i = 0 ; i < 4 ; i++)
						{
							for (j = 0 ; j < 4 ; j++)
							{
								projection_matrix[i * 4 + j] = 0.0;
							}
							projection_matrix[i * 4 + i] = 1.0;
							LU_backsubstitute(/* dimension */4, total_projection_matrix,
								lu_index, projection_matrix + i * 4);
						}
						/* transpose */
						for (i = 0 ; i < 4 ; i++)
						{
							for (j = i + 1 ; j < 4 ; j++)
							{
								lu_d = projection_matrix[i * 4 + j];
								projection_matrix[i * 4 + j] =
									projection_matrix[j * 4 + i];
								projection_matrix[j * 4 + i] = lu_d;
							}
						}
					}
				}
				else
				{
					/* Get the viewport transformation too */
					Scene_viewer_get_viewport_info(scene_viewer,
						&viewport_left, &viewport_top, 
						&viewport_pixels_per_unit_x, &viewport_pixels_per_unit_y);
					Scene_viewer_get_viewport_size(scene_viewer,
						&viewport_width, &viewport_height);
					/* Multiply total_projection by viewport matrices */
					for (i=0;i<4;i++)
					{
						for (j=0;j<4;j++)
						{
							viewport_matrix[i * 4 + j] = 0.0;
							for (k=0;k<4;k++)
							{
								if ((i == 0) && (k == 0))
								{
									viewport_matrix[i * 4 + j] += 
										0.5 * viewport_width / viewport_pixels_per_unit_x  *
										total_projection_matrix[k * 4 + j];
								}
								else if((i == 1) && (k == 1))
								{
									viewport_matrix[i * 4 + j] += 
										0.5 * viewport_height / viewport_pixels_per_unit_y *
										total_projection_matrix[k * 4 + j];
								}
								else if((i == 0) && (k == 3))
								{
									viewport_matrix[i * 4 + j] += 
										(viewport_left + 0.5 * viewport_width 
											/ viewport_pixels_per_unit_x) *
										total_projection_matrix[k * 4 + j];
								}
								else if((i == 1) && (k == 3))
								{
									viewport_matrix[i * 4 + j] += 
										(viewport_top - 0.5 * viewport_height
											/ viewport_pixels_per_unit_y) *
										total_projection_matrix[k * 4 + j];
								}
								else if((i == 2) && (k == 2))
								{
									viewport_matrix[i * 4 + j] += 
										total_projection_matrix[k * 4 + j];
								}
								else if((i == 3) && (k == 3))
								{
									viewport_matrix[i * 4 + j] += 
										total_projection_matrix[k * 4 + j];
								}
								else
								{
									viewport_matrix[i * 4 + j] += 0.0;
								}
							}
						}
					}
					if (projection_type == VIEWPORT_PROJECTION)
					{
						/* viewport_projection */
						for (i=0;i<16;i++)
						{
							projection_matrix[i] = viewport_matrix[i];
						}
					}
					else if (projection_type == INVERSE_VIEWPORT_PROJECTION)
					{
						if (LU_decompose(/* dimension */4, viewport_matrix,
							lu_index, &lu_d,/*singular_tolerance*/1.0e-12))
						{
							for (i = 0 ; i < 4 ; i++)
							{
								for (j = 0 ; j < 4 ; j++)
								{
									projection_matrix[i * 4 + j] = 0.0;
								}
								projection_matrix[i * 4 + i] = 1.0;
								LU_backsubstitute(/* dimension */4, viewport_matrix,
									lu_index, projection_matrix + i * 4);
							}
							/* transpose */
							for (i = 0 ; i < 4 ; i++)
							{
								for (j = i + 1 ; j < 4 ; j++)
								{
									lu_d = projection_matrix[i * 4 + j];
									projection_matrix[i * 4 + j] =
										projection_matrix[j * 4 + i];
									projection_matrix[j * 4 + i] = lu_d;
								}
							}
						}
					}
					else
					{
						/* texture_projection */
						Scene_viewer_get_background_texture_info(scene_viewer,
							&bk_texture_left, &bk_texture_top, &bk_texture_width, &bk_texture_height,
							&bk_texture_undistort_on, &bk_texture_max_pixels_per_polygon);
						if (texture = Scene_viewer_get_background_texture(scene_viewer))
						{
							Texture_get_distortion_info(texture, &distortion_centre_x, 
								&distortion_centre_y, &distortion_factor_k1);
							Texture_get_physical_size(texture, &texture_width, 
								&texture_height, &texture_depth);
					
							if (bk_texture_undistort_on && distortion_factor_k1)
							{
								display_message(ERROR_MESSAGE,
									"gfx_apply_projection.  Distortion corrected textures are not supported yet");
								return_code=0;
								/* identity_projection */
								for (i=0;i<16;i++)
								{
									if (i % 5)
									{
										projection_matrix[i] = 0;
									}
									else
									{
										projection_matrix[i] = 1;
									}
								}
							}
							else
							{
								/* Multiply viewport_matrix by background texture */
								for (i=0;i<4;i++)
								{
									for (j=0;j<4;j++)
									{
										texture_projection_matrix[i * 4 + j] = 0.0;
										for (k=0;k<4;k++)
										{
											if ((i == 0) && (k == 0))
											{
												texture_projection_matrix[i * 4 + j] += 
													(texture_width / bk_texture_width) *
													viewport_matrix[k * 4 + j];
											}
											else if((i == 1) && (k == 1))
											{
												texture_projection_matrix[i * 4 + j] += 
													(texture_height / bk_texture_height) *
													viewport_matrix[k * 4 + j];
											}
											else if((i == 0) && (k == 3))
											{
												texture_projection_matrix[i * 4 + j] += 
													(-bk_texture_left *
														(texture_width / bk_texture_width)) *
													viewport_matrix[k * 4 + j];
											}
											else if((i == 1) && (k == 3))
											{
												texture_projection_matrix[i * 4 + j] += 
													(- bk_texture_top * 
														(texture_height / bk_texture_height)
														+ texture_height) *
													viewport_matrix[k * 4 + j];
											}
											else if((i == 2) && (k == 2))
											{
												texture_projection_matrix[i * 4 + j] += 
													viewport_matrix[k * 4 + j];
											}
											else if((i == 3) && (k == 3))
											{
												texture_projection_matrix[i * 4 + j] += 
													viewport_matrix[k * 4 + j];
											}
											else
											{
												texture_projection_matrix[i * 4 + j] += 0.0;
											}
										}
									}
								}
								if (projection_type == TEXTURE_PROJECTION)
								{
									for (i=0;i<16;i++)
									{
										projection_matrix[i] = texture_projection_matrix[i];
									}
								}
								else if (projection_type == INVERSE_TEXTURE_PROJECTION)
								{
									if (LU_decompose(/* dimension */4, texture_projection_matrix,
										lu_index, &lu_d,/*singular_tolerance*/1.0e-12))
									{
										for (i = 0 ; i < 4 ; i++)
										{
											for (j = 0 ; j < 4 ; j++)
											{
												projection_matrix[i * 4 + j] = 0.0;
											}
											projection_matrix[i * 4 + i] = 1.0;
											LU_backsubstitute(/* dimension */4, 
												texture_projection_matrix,
												lu_index, projection_matrix + i * 4);
										}
										/* transpose */
										for (i = 0 ; i < 4 ; i++)
										{
											for (j = i + 1 ; j < 4 ; j++)
											{
												lu_d = projection_matrix[i * 4 + j];
												projection_matrix[i * 4 + j] =
													projection_matrix[j * 4 + i];
												projection_matrix[j * 4 + i] = lu_d;
											}
										}
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Computed_field_window_projection_calculate_matrix.  "
										"Unknown projection type.");
									return_code = 0;
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_window_projection_calculate_matrix.  "
								"No background texture defined.");
							return_code = 0;
							/* identity_projection */
							for (i=0;i<16;i++)
							{
								if (i % 5)
								{
									projection_matrix[i] = 0;
								}
								else
								{
									projection_matrix[i] = 1;
								}
							}
						}
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_window_projection_calculate_matrix.  "
				"Not enough memory for matrix");
			return_code = 0;
		}
		if (!return_code)
		{
			/* Only keep projection_matrix if valid */
			if (projection_matrix)
			{
				DEALLOCATE(projection_matrix);
				projection_matrix = (double *)NULL;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_window_projection_calculate_matrix.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_window_projection_calculate_matrix */

Computed_field_window_projection::~Computed_field_window_projection()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{

	ENTER(Computed_field_window_projection::~Computed_field_window_projection);
	if (field)
	{
		if (scene_viewer_callback_flag)
		{
			Scene_viewer_remove_transform_callback(scene_viewer, 
			  Computed_field_window_projection_scene_viewer_callback,
			  (void *)field);
			scene_viewer_callback_flag = 0;
		}
		if (projection_matrix)
		{
			DEALLOCATE(projection_matrix);
		}
		if (scene_viewer)
		{
			Scene_viewer_remove_destroy_callback(scene_viewer, 
				Computed_field_window_projection_scene_viewer_destroy_callback,
				(void *)field);
		}
		if (graphics_window_name)
		{
			DEALLOCATE(graphics_window_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_window_projection::~Computed_field_window_projection.  "
			"Invalid arguments.");
	}
	LEAVE;

} /* Computed_field_window_projection::~Computed_field_window_projection */

int Computed_field_window_projection::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	Computed_field_window_projection *other;
	int return_code;

	ENTER(Computed_field_window_projection::compare);
	if (field && (other = dynamic_cast<Computed_field_window_projection*>(other_core)))
	{
		if ((!strcmp(graphics_window_name, other->graphics_window_name)) &&
			(scene_viewer == other->scene_viewer) &&
			(pane_number == other->pane_number) &&
			(projection_type == other->projection_type))
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_window_projection::compare */

int Computed_field_window_projection::evaluate_projection_matrix(
	int element_dimension, int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Function called by Computed_field_evaluate_in_element to compute a field
transformed by a projection matrix.
NOTE: Assumes that values and derivatives arrays are already allocated in
<field>, and that its source_fields are already computed (incl. derivatives)
for the same element, with the given <element_dimension> = number of Xi coords.
==============================================================================*/
{
	double dhdxi, dh1dxi, perspective;
	int coordinate_components, i, j, k,return_code;
	
	ENTER(Computed_field_evaluate_projection_matrix);
	if (field)
	{
		if (scene_viewer)
		{
			if (projection_matrix || calculate_matrix())
			{
				return_code = 1;
				if (calculate_derivatives)
				{
					field->derivatives_valid=1;
				}
				else
				{
					field->derivatives_valid=0;
				}

				/* Calculate the transformed coordinates */
				coordinate_components=field->source_fields[0]->number_of_components;
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					field->values[i] = 0.0;
					for (j = 0 ; j < coordinate_components ; j++)
					{
						field->values[i] += projection_matrix[i * (coordinate_components + 1) + j]
							* field->source_fields[0]->values[j];
					}
					/* The last source value is fixed at 1 */
					field->values[i] += 
						projection_matrix[i * (coordinate_components + 1) + coordinate_components];
				}

				/* The last calculated value is the perspective value which divides through
					all the other components */
				perspective = 0.0;
				for (j = 0 ; j < coordinate_components ; j++)
				{
					perspective += projection_matrix[field->number_of_components
						* (coordinate_components + 1) + j] * field->source_fields[0]->values[j];
				}
				perspective += projection_matrix[field->number_of_components 
					* (coordinate_components + 1) + coordinate_components];
		
				if (calculate_derivatives)
				{
					for (k=0;k<element_dimension;k++)
					{
						/* Calculate the coordinate derivatives without perspective */
						for (i = 0 ; i < field->number_of_components ; i++)
						{
							field->derivatives[i * element_dimension + k] = 0.0;
							for (j = 0 ; j < coordinate_components ; j++)
							{
								field->derivatives[i * element_dimension + k] += 
									projection_matrix[i * (coordinate_components + 1) + j]
									* field->source_fields[0]->derivatives[j * element_dimension + k];
							}
						}

						/* Calculate the perspective derivative */
						dhdxi = 0.0;
						for (j = 0 ; j < coordinate_components ; j++)
						{
							dhdxi += projection_matrix[field->number_of_components 
								* (coordinate_components + 1) + j]
								* field->source_fields[0]->derivatives[j *element_dimension + k];
						}

						/* Calculate the perspective reciprocal derivative using chain rule */
						dh1dxi = (-1.0) / (perspective * perspective) * dhdxi;

						/* Calculate the derivatives of the perspective scaled transformed coordinates,
							which is ultimately what we want */
						for (i = 0 ; i < field->number_of_components ; i++)
						{
							field->derivatives[i * element_dimension + k] = 
								field->derivatives[i * element_dimension + k] / perspective
								+ field->values[i] * dh1dxi;
						}
					}
				}

				/* Now apply the perspective scaling to the non derivative transformed coordinates */
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					field->values[i] /= perspective;
				}
			}
			else
			{
				return_code=0;
			}
		}
		else
		{
			/* Just set everything to zero */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = 0.0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_projection_matrix.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_evaluate_projection_matrix */

int Computed_field_window_projection::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_window_projection::evaluate_cache_at_location);
	if (field && location)
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location))
		{
			/* 2. Calculate the field */
			return_code = evaluate_projection_matrix(
				location->get_number_of_derivatives(),
				(0 < location->get_number_of_derivatives()));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_window_projection::evaluate_cache_at_location.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_window_projection::evaluate_cache_at_location */

int Computed_field_window_projection::set_values_at_location(
   Field_location* location, FE_value *values)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Sets the <values> of the computed <field> at <location>.
==============================================================================*/
{
	double d,lu_matrix[16],result[4];
	int indx[4],return_code;
	FE_value source_values[3];
	
	ENTER(Computed_field_window_projection::set_values_at_location);
	if (field && location && values)
	{
		field->derivatives_valid = 0;
		if (scene_viewer)
		{
			if (projection_matrix || calculate_matrix())
			{
				copy_matrix(4,4,projection_matrix,lu_matrix);
				result[0] = (double)values[0];
				result[1] = (double)values[1];
				result[2] = (double)values[2];
				result[3] = 1.0;
				if (LU_decompose(4,lu_matrix,indx,&d,/*singular_tolerance*/1.0e-12) &&
					LU_backsubstitute(4,lu_matrix,indx,result) &&
					(0.0 != result[3]))
				{
					source_values[0] = (result[0] / result[3]);
					source_values[1] = (result[1] / result[3]);
					source_values[2] = (result[2] / result[3]);
					return_code=Computed_field_set_values_at_location(
						field->source_fields[0],location,source_values);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_window_projection::set_values_at_location.  "
						"Could not invert field %s",field->name);
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_window_projection::set_values_at_location.  "
					"Missing projection matrix for field %s",field->name);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_window_projection::set_values_at_location.  "
				"Scene_viewer invalid.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_window_projection::set_values_at_location.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_window_projection::set_values_at_location */

int Computed_field_window_projection::find_element_xi( 
	FE_value *values, int number_of_values, struct FE_element **element,
	FE_value *xi, int element_dimension, struct Cmiss_region *search_region)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	double d,lu_matrix[16],result[4];
	int indx[4],return_code;
	FE_value source_values[3];

	ENTER(Computed_field_window_projection::find_element_xi);
	if (field && values
		&&(number_of_values==field->number_of_components)&&element&&xi&&
		search_region)
	{
		if (scene_viewer)
		{
			if (projection_matrix || calculate_matrix())
			{
				copy_matrix(4,4,projection_matrix,lu_matrix);
				result[0] = (double)values[0];
				result[1] = (double)values[1];
				result[2] = (double)values[2];
				result[3] = 1.0;
				if (LU_decompose(4,lu_matrix,indx,&d,/*singular_tolerance*/1.0e-12) &&
					LU_backsubstitute(4,lu_matrix,indx,result) &&
					(0.0 != result[3]))
				{
					source_values[0] = (result[0] / result[3]);
					source_values[1] = (result[1] / result[3]);
					source_values[2] = (result[2] / result[3]);
					return_code=Computed_field_find_element_xi(
						field->source_fields[0], source_values, 3, element,
						xi, element_dimension, search_region, /*propagate_field*/1,
						/*find_nearest_location*/0);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_window_projection::find_element_xi.  "
						"Could not invert field %s",field->name);
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_window_projection::find_element_xi.  "
					"Missing projection matrix for field %s",field->name);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_window_projection::find_element_xi.  "
				"Scene_viewer invalid.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_window_projection::find_element_xi.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_window_projection::find_element_xi */

int Computed_field_window_projection::list()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_window_projection);
	if (field)
	{
		return_code = 1;
		display_message(INFORMATION_MESSAGE,"    source field : %s\n",
			field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,"    window : %s\n",
			graphics_window_name);
		display_message(INFORMATION_MESSAGE,"    pane number : %d\n",
			pane_number + 1);
		display_message(INFORMATION_MESSAGE,"    projection type : %s\n",
			projection_type_string(projection_type));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_window_projection.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_window_projection */

char *Computed_field_window_projection::get_command_string()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error;

	ENTER(Computed_field_window_projection::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_window_projection_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " window ", &error);
		append_string(&command_string, graphics_window_name, &error);
		sprintf(temp_string, " pane_number %d ", pane_number + 1);
		append_string(&command_string, temp_string, &error);
		append_string(&command_string,
			projection_type_string(projection_type),
			&error);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_window_projection::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_window_projection::get_command_string */

void Computed_field_window_projection_scene_viewer_callback(
	struct Scene_viewer *scene_viewer, void *dummy_void, void *field_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Clear the projection matrix as it is no longer valid and notify the manager
that the computed field has changed.
==============================================================================*/
{
	Computed_field* field;
	Computed_field_window_projection* core;

	USE_PARAMETER(dummy_void);
	ENTER(Computed_field_window_projection_scene_viewer_callback);
	if (scene_viewer && (field = (Computed_field *)field_void) && 
		(core = dynamic_cast<Computed_field_window_projection*>(field->core)))
	{
		if (core->projection_matrix)
		{
			DEALLOCATE(core->projection_matrix);
			core->projection_matrix = (double *)NULL;
		}
		Computed_field_changed(field, core->computed_field_manager);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_window_projection_scene_viewer_callback.  "
			"Invalid arguments.");
	}
	LEAVE;
} /* Computed_field_window_projection_scene_viewer_callback */

void Computed_field_window_projection_scene_viewer_destroy_callback(
	struct Scene_viewer *scene_viewer, void *dummy_void, void *field_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Clear the scene viewer reference when it is no longer valid.
==============================================================================*/
{
	Computed_field* field;
	Computed_field_window_projection* core;

	USE_PARAMETER(dummy_void);
	ENTER(Computed_field_window_projection_scene_viewer_destroy_callback);
	if (scene_viewer && (field = (Computed_field *)field_void) && 
		(core = dynamic_cast<Computed_field_window_projection*>(field->core)))
	{
		if (core->scene_viewer_callback_flag)
		{
			Scene_viewer_remove_transform_callback(scene_viewer, 
			  Computed_field_window_projection_scene_viewer_callback,
			  (void *)field);
			core->scene_viewer_callback_flag = 0;
		}
		if (core->graphics_window_name)
		{
			DEALLOCATE(core->graphics_window_name);
		}
		core->pane_number = 0;
		core->scene_viewer = (struct Scene_viewer *)NULL;
		Computed_field_changed(field, core->computed_field_manager);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_window_projection_scene_viewer_callback.  "
			"Invalid arguments.");
	}
	LEAVE;
} /* Computed_field_window_projection_scene_viewer_callback */

} //namespace

int Computed_field_set_type_window_projection(struct Computed_field *field,
	struct Computed_field *source_field, struct Scene_viewer *scene_viewer,
	char *graphics_window_name, int pane_number,
	enum Computed_field_window_projection_type projection_type,
	struct MANAGER(Computed_field) *computed_field_manager)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_WINDOW_PROJECTION, returning the 
<source_field> with each component multiplied by the perspective transformation
of the <scene_viewer>.  The <graphics_window_name> and <pane_number> are stored
so that the command to reproduce this field can be written out.
The <computed_field_manager> is notified by the <field> if the <scene_viewer> closes.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_window_projection);
	if (field && source_field && Computed_field_has_3_components(source_field, NULL)
		&& scene_viewer)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->number_of_components = 3;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->core = new Computed_field_window_projection(field,
				graphics_window_name, pane_number, scene_viewer,
				projection_type, computed_field_manager);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_window_projection.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_window_projection */

int Computed_field_get_type_window_projection(struct Computed_field *field,
	struct Computed_field **source_field, struct Scene_viewer **scene_viewer,
	char **graphics_window_name, int *pane_number, 
	enum Computed_field_window_projection_type *projection_type)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_WINDOW_PROJECTION, the <source_field>,
<graphics_window>, <pane_number> and <projection_type> used by it are returned.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	Computed_field_window_projection* core;
	int return_code;

	ENTER(Computed_field_get_type_window_projection);
	if (field&&(core = dynamic_cast<Computed_field_window_projection*>(field->core))
		&& source_field && scene_viewer && 
		graphics_window_name && pane_number && projection_type)
	{
		*source_field = field->source_fields[0];
		*scene_viewer = core->scene_viewer;
		*graphics_window_name = duplicate_string(core->graphics_window_name);
		*pane_number = core->pane_number;
		*projection_type = core->projection_type;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_window_projection.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_window_projection */

int define_Computed_field_type_window_projection(struct Parse_state *state,
	void *field_void,void *computed_field_window_projection_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_WINDOW_PROJECTION (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	char *graphics_window_name, *projection_type_string;
	enum Computed_field_window_projection_type projection_type;
	int pane_number, return_code;
	struct Computed_field *field,*source_field;
	Computed_field_window_projection_package 
		*computed_field_window_projection_package;
	struct Graphics_window *graphics_window;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;
	struct Scene_viewer *scene_viewer;
	static char *projection_type_strings[] =
	{
	  "ndc_projection",
	  "texture_projection",
	  "viewport_projection",
	  "inverse_ndc_projection",
	  "inverse_texture_projection",
	  "inverse_viewport_projection"	  
	};

	ENTER(define_Computed_field_type_window_projection);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_window_projection_package=
		(Computed_field_window_projection_package *)
		computed_field_window_projection_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		pane_number = 1;
		projection_type = TEXTURE_PROJECTION;
		source_field = (struct Computed_field *)NULL;
		graphics_window = (struct Graphics_window *)NULL;
		if (dynamic_cast<Computed_field_window_projection*>(field->core))
		{
			return_code=Computed_field_get_type_window_projection(field,
				&source_field, &scene_viewer, &graphics_window_name, &pane_number, &projection_type);
			pane_number++;
			if (graphics_window_name)
			{
				graphics_window = FIND_BY_IDENTIFIER_IN_MANAGER(Graphics_window,name)(graphics_window_name,
					computed_field_window_projection_package->graphics_window_manager);
				DEALLOCATE(graphics_window_name);
			}
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			if (graphics_window)
			{
				ACCESS(Graphics_window)(graphics_window);
			}
			projection_type_string=projection_type_strings[1];

			option_table = CREATE(Option_table)();
			/* field */
			set_source_field_data.computed_field_manager=
				computed_field_window_projection_package->computed_field_manager;
			set_source_field_data.conditional_function=Computed_field_has_3_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"field",&source_field,
				&set_source_field_data,set_Computed_field_conditional);
			/* ndc_projection, texture_projection, viewport_projection */
			Option_table_add_enumerator(option_table,
			  sizeof(projection_type_strings)/sizeof(char *),
			  projection_type_strings,&projection_type_string);
			/* pane_number */
			Option_table_add_entry(option_table,"pane_number",&pane_number,
				NULL,set_int_positive);
			/* window */
			Option_table_add_entry(option_table,"window",&graphics_window,
				computed_field_window_projection_package->graphics_window_manager,
				set_Graphics_window);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
				if (projection_type_string == projection_type_strings[0])
				{
					projection_type = NDC_PROJECTION;
				}
				else if (projection_type_string == projection_type_strings[1])
				{
					projection_type = TEXTURE_PROJECTION;
				}
				else if (projection_type_string == projection_type_strings[2])
				{
					projection_type = VIEWPORT_PROJECTION;
				}
				else if (projection_type_string == projection_type_strings[3])
				{
					projection_type = INVERSE_NDC_PROJECTION;
				}
				else if (projection_type_string == projection_type_strings[4])
				{
					projection_type = INVERSE_TEXTURE_PROJECTION;
				}
				else if (projection_type_string == projection_type_strings[5])
				{
					projection_type = INVERSE_VIEWPORT_PROJECTION;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_window_projection.  "
						"No projection type.");
					return_code = 0;
				}
			}
			if (return_code)
			{
				if (!(scene_viewer = Graphics_window_get_Scene_viewer(graphics_window, 
					pane_number - 1)))
				{
					return_code = 0;
				}
			}
			if (return_code)
			{
				GET_NAME(Graphics_window)(graphics_window, &graphics_window_name);
				return_code = Computed_field_set_type_window_projection(field,
					source_field, scene_viewer, graphics_window_name, pane_number,
					projection_type, computed_field_window_projection_package->computed_field_manager);
				DEALLOCATE(graphics_window_name);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_window_projection.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			if (graphics_window)
			{
				DEACCESS(Graphics_window)(&graphics_window);
			}
			DESTROY(Option_table)(&option_table);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_window_projection.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_window_projection */

int Computed_field_register_type_window_projection(
	struct Computed_field_package *computed_field_package, 
	struct MANAGER(Graphics_window) *graphics_window_manager)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	Computed_field_window_projection_package
		*computed_field_window_projection_package =
		new Computed_field_window_projection_package;

	ENTER(Computed_field_register_type_window_projection);
	if (computed_field_package && graphics_window_manager)
	{
		computed_field_window_projection_package->computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		computed_field_window_projection_package->graphics_window_manager =
			graphics_window_manager;
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_window_projection_type_string, 
			define_Computed_field_type_window_projection,
			computed_field_window_projection_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_type_window_projection.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_type_window_projection */

