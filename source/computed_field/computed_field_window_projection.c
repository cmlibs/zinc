/*******************************************************************************
FILE : computed_field_window_projection.c

LAST MODIFIED : 21 January 2002

DESCRIPTION :
Implements a computed_field which maintains a graphics transformation 
equivalent to the scene_viewer assigned to it.
==============================================================================*/
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/matrix_vector.h"
#include "general/mystring.h"
#include "graphics/graphics_window.h"
#include "graphics/scene_viewer.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_window_projection.h"

enum Computed_field_window_projection_type
{
	NDC_PROJECTION,
	TEXTURE_PROJECTION,
	VIEWPORT_PROJECTION,
	INVERSE_NDC_PROJECTION,
	INVERSE_TEXTURE_PROJECTION,
	INVERSE_VIEWPORT_PROJECTION
};

struct Computed_field_window_projection_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
	struct MANAGER(Graphics_window) *graphics_window_manager;
};

struct Computed_field_window_projection_type_specific_data
{
	double *projection_matrix;
	enum Computed_field_window_projection_type projection_type;
	/* We need to hold on to the graphics_window as well as the 
		scene_viewer as the scene_viewer is not accessed, the graphics_window is */
	struct Graphics_window *graphics_window;
	int pane_number;
	struct Scene_viewer *scene_viewer;
	/* This flag indicates if the field has registered for callbacks with the
		scene_viewer */
	int scene_viewer_callback_flag;
	struct Computed_field_window_projection_package *package;
};

static char computed_field_window_projection_type_string[] = "window_projection";

static char *Computed_field_window_projection_type_string(
	enum Computed_field_window_projection_type projection_type)
/*******************************************************************************
LAST MODIFIED : 6 October 2000

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
} /* Computed_field_window_projection_type_string */

int Computed_field_is_type_window_projection(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_window_projection);
	if (field)
	{
		return_code = (field->type_string == computed_field_window_projection_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_window_projection.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_is_type_window_projection */

static void Computed_field_window_projection_scene_viewer_callback(
	struct Scene_viewer *scene_viewer, void *dummy_void, void *field_void)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Clear the projection matrix as it is no longer valid and notify the manager
that the computed field has changed.
==============================================================================*/
{
	struct Computed_field *field;
	struct Computed_field_window_projection_type_specific_data *data;

	USE_PARAMETER(dummy_void);
	ENTER(Computed_field_window_projection_scene_viewer_callback);
	if (scene_viewer && (field = (struct Computed_field *)field_void) && 
		(data = (struct Computed_field_window_projection_type_specific_data *)
		field->type_specific_data))
	{
		if (data->projection_matrix)
		{
			DEALLOCATE(data->projection_matrix);
			data->projection_matrix = (double *)NULL;
		}
		Computed_field_changed(field, data->package->computed_field_manager);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_window_projection_scene_viewer_callback.  "
			"Invalid arguments.");
	}
	LEAVE;
} /* Computed_field_window_projection_scene_viewer_callback */

static int Computed_field_window_projection_calculate_matrix(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 27 February 2001

DESCRIPTION :
==============================================================================*/
{
	Dimension viewport_width, viewport_height;
	double lu_d, modelview_matrix[16], window_projection_matrix[16],
		texture_projection_matrix[16], total_projection_matrix[16], 
		viewport_matrix[16], viewport_left, viewport_top, 
		viewport_pixels_per_unit_x, viewport_pixels_per_unit_y,
		bk_texture_left, bk_texture_top, bk_texture_width, bk_texture_height,
		bk_texture_max_pixels_per_polygon;
	float distortion_centre_x, distortion_centre_y, distortion_factor_k1,
		texture_width, texture_height;
	int bk_texture_undistort_on, i, j, k, lu_index[4], return_code;
	struct Computed_field_window_projection_type_specific_data *data;
	struct Scene_viewer *scene_viewer;
	struct Texture *texture;

	ENTER(Computed_field_window_projection_calculate_matrix);
	if (field && (data = 
		(struct Computed_field_window_projection_type_specific_data *)
		field->type_specific_data) && (scene_viewer = data->scene_viewer))
	{
		if (data->projection_matrix ||
			ALLOCATE(data->projection_matrix, double, 16))
		{
			return_code = 1;
			/* make sure we are getting scene viewer callbacks once the
				 projection matrix exists */
			if (!data->scene_viewer_callback_flag)
			{
				data->scene_viewer_callback_flag = 
					Scene_viewer_transform_add_callback(data->scene_viewer, 
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
				if (data->projection_type == NDC_PROJECTION)
				{
					/* ndc_projection */
					for (i=0;i<16;i++)
					{
						data->projection_matrix[i] = total_projection_matrix[i];
					}
				}
				else if (data->projection_type == INVERSE_NDC_PROJECTION)
				{
					LU_decompose(/* dimension */4, total_projection_matrix,
						lu_index, &lu_d);
					for (i = 0 ; i < 4 ; i++)
					{
						for (j = 0 ; j < 4 ; j++)
						{
							data->projection_matrix[i * 4 + j] = 0.0;
						}
						data->projection_matrix[i * 4 + i] = 1.0;
						LU_backsubstitute(/* dimension */4, total_projection_matrix,
							lu_index, data->projection_matrix + i * 4);
					}
					/* transpose */
					for (i = 0 ; i < 4 ; i++)
					{
						for (j = i + 1 ; j < 4 ; j++)
						{
							lu_d = data->projection_matrix[i * 4 + j];
							data->projection_matrix[i * 4 + j] =
								data->projection_matrix[j * 4 + i];
							data->projection_matrix[j * 4 + i] = lu_d;
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
					if (data->projection_type == VIEWPORT_PROJECTION)
					{
						/* viewport_projection */
						for (i=0;i<16;i++)
						{
							data->projection_matrix[i] = viewport_matrix[i];
						}
					}
					else if (data->projection_type == INVERSE_VIEWPORT_PROJECTION)
					{
						LU_decompose(/* dimension */4, viewport_matrix,
							lu_index, &lu_d);
						for (i = 0 ; i < 4 ; i++)
						{
							for (j = 0 ; j < 4 ; j++)
							{
								data->projection_matrix[i * 4 + j] = 0.0;
							}
							data->projection_matrix[i * 4 + i] = 1.0;
							LU_backsubstitute(/* dimension */4, viewport_matrix,
								lu_index, data->projection_matrix + i * 4);
						}
						/* transpose */
						for (i = 0 ; i < 4 ; i++)
						{
							for (j = i + 1 ; j < 4 ; j++)
							{
								lu_d = data->projection_matrix[i * 4 + j];
								data->projection_matrix[i * 4 + j] =
									data->projection_matrix[j * 4 + i];
								data->projection_matrix[j * 4 + i] = lu_d;
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
								&texture_height);
					
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
										data->projection_matrix[i] = 0;
									}
									else
									{
										data->projection_matrix[i] = 1;
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
								if (data->projection_type == TEXTURE_PROJECTION)
								{
									for (i=0;i<16;i++)
									{
										data->projection_matrix[i] = texture_projection_matrix[i];
									}
								}
								else if (data->projection_type == INVERSE_TEXTURE_PROJECTION)
								{
									LU_decompose(/* dimension */4, texture_projection_matrix,
										lu_index, &lu_d);
									for (i = 0 ; i < 4 ; i++)
									{
										for (j = 0 ; j < 4 ; j++)
										{
											data->projection_matrix[i * 4 + j] = 0.0;
										}
										data->projection_matrix[i * 4 + i] = 1.0;
										LU_backsubstitute(/* dimension */4, 
											texture_projection_matrix,
											lu_index, data->projection_matrix + i * 4);
									}
									/* transpose */
									for (i = 0 ; i < 4 ; i++)
									{
										for (j = i + 1 ; j < 4 ; j++)
										{
											lu_d = data->projection_matrix[i * 4 + j];
											data->projection_matrix[i * 4 + j] =
												data->projection_matrix[j * 4 + i];
											data->projection_matrix[j * 4 + i] = lu_d;
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
									data->projection_matrix[i] = 0;
								}
								else
								{
									data->projection_matrix[i] = 1;
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
			if (data->projection_matrix)
			{
				DEALLOCATE(data->projection_matrix);
				data->projection_matrix = (double *)NULL;
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

static int Computed_field_window_projection_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_window_projection_type_specific_data *data;

	ENTER(Computed_field_window_projection_clear_type_specific);
	if (field && (data = 
		(struct Computed_field_window_projection_type_specific_data *)
		field->type_specific_data))
	{
		if (data->scene_viewer_callback_flag)
		{
			Scene_viewer_transform_remove_callback(data->scene_viewer, 
			  Computed_field_window_projection_scene_viewer_callback,
			  (void *)field);
			data->scene_viewer_callback_flag = 0;
		}
		if (data->projection_matrix)
		{
			DEALLOCATE(data->projection_matrix);
		}
		if (data->graphics_window)
		{
			DEACCESS(Graphics_window)(&(data->graphics_window));
		}
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_window_projection_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_window_projection_clear_type_specific */

static void *Computed_field_window_projection_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_window_projection_type_specific_data *destination,
		*source;

	ENTER(Computed_field_window_projection_copy_type_specific);
	if (field && (source = 
		(struct Computed_field_window_projection_type_specific_data *)
		field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_window_projection_type_specific_data, 1))
		{
			destination->projection_matrix = (double *)NULL;
			destination->graphics_window = ACCESS(Graphics_window)
				(source->graphics_window);
			destination->scene_viewer = source->scene_viewer;
			destination->pane_number = source->pane_number;
			destination->projection_type = source->projection_type;
			destination->package = source->package;
			destination->scene_viewer_callback_flag = 0;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_window_projection_copy_type_specific.  "
				"Unable to allocate memory.");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_window_projection_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_window_projection_copy_type_specific */

#define Computed_field_window_projection_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_window_projection_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;
	struct Computed_field_window_projection_type_specific_data *data,
		*other_data;

	ENTER(Computed_field_window_projection_type_specific_contents_match);
	if (field && other_computed_field && (data = 
		(struct Computed_field_window_projection_type_specific_data *)
		field->type_specific_data) && (other_data =
		(struct Computed_field_window_projection_type_specific_data *)
		other_computed_field->type_specific_data))
	{
		if ((data->graphics_window == other_data->graphics_window) &&
			(data->scene_viewer == other_data->scene_viewer) &&
			(data->pane_number == other_data->pane_number) &&
			(data->projection_type == other_data->projection_type))
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
} /* Computed_field_window_projection_type_specific_contents_match */

#define Computed_field_window_projection_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_window_projection_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_window_projection_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_window_projection_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_evaluate_projection_matrix(
	struct Computed_field *field,
	int element_dimension, int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 10 October 2000

DESCRIPTION :
Function called by Computed_field_evaluate_in_element to compute a field
transformed by a projection matrix.
NOTE: Assumes that values and derivatives arrays are already allocated in
<field>, and that its source_fields are already computed (incl. derivatives)
for the same element, with the given <element_dimension> = number of Xi coords.
==============================================================================*/
{
	double dhdxi, dh1dxi, perspective, *projection_matrix;
	int coordinate_components, i, j, k,return_code;
	struct Computed_field_window_projection_type_specific_data *data;
	
	ENTER(Computed_field_evaluate_projection_matrix);
	if (field&&(COMPUTED_FIELD_NEW_TYPES==field->type)&&
		(computed_field_window_projection_type_string==field->type_string)&&
		(data = (struct Computed_field_window_projection_type_specific_data *)
		field->type_specific_data))
	{
		if (data->projection_matrix ||
			Computed_field_window_projection_calculate_matrix(field))
		{
			return_code = 1;
			projection_matrix = data->projection_matrix;
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
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_projection_matrix.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_evaluate_projection_matrix */

static int Computed_field_window_projection_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_window_projection_evaluate_cache_at_node);
	if (field && node)
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			return_code = Computed_field_evaluate_projection_matrix(
				field, /*element_dimension*/0, /*calculate_derivatives*/0);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_window_projection_evaluate_cache_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_window_projection_evaluate_cache_at_node */

static int Computed_field_window_projection_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int element_dimension, return_code;

	ENTER(Computed_field_window_projection_evaluate_cache_in_element);
	if (field && element && xi)
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			element_dimension=get_FE_element_dimension(element);
			return_code = Computed_field_evaluate_projection_matrix(
				field, element_dimension, calculate_derivatives);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_window_projection_evaluate_cache_in_element.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_window_projection_evaluate_cache_in_element */

#define Computed_field_window_projection_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_window_projection_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

static int Computed_field_window_projection_set_values_at_node(
	struct Computed_field *field,struct FE_node *node,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 10 October 2000

DESCRIPTION :
Sets the <values> of the computed <field> at <node>.
==============================================================================*/
{
	double d,lu_matrix[16],result[4];
	int indx[4],return_code;
	FE_value source_values[3];
	struct Computed_field_window_projection_type_specific_data *data;
	
	ENTER(Computed_field_window_projection_set_values_at_node);
	if (field && node && values && (COMPUTED_FIELD_NEW_TYPES==field->type) &&
		(computed_field_window_projection_type_string == field->type_string) &&
		(data = (struct Computed_field_window_projection_type_specific_data *)
			field->type_specific_data))
	{
		if (data->projection_matrix ||
			Computed_field_window_projection_calculate_matrix(field))
		{
			copy_matrix(4,4,data->projection_matrix,lu_matrix);
			result[0] = (double)values[0];
			result[1] = (double)values[1];
			result[2] = (double)values[2];
			result[3] = 1.0;
			if (LU_decompose(4,lu_matrix,indx,&d) &&
				LU_backsubstitute(4,lu_matrix,indx,result) &&
				(0.0 != result[3]))
			{
				source_values[0] = (result[0] / result[3]);
				source_values[1] = (result[1] / result[3]);
				source_values[2] = (result[2] / result[3]);
				return_code=Computed_field_set_values_at_node(
					field->source_fields[0],node,source_values);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_window_projection_set_values_at_node.  "
					"Could not invert field %s",field->name);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_window_projection_set_values_at_node.  "
				"Missing projection matrix for field %s",field->name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_window_projection_set_values_at_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_window_projection_set_values_at_node */

#define Computed_field_window_projection_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_window_projection_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

static int Computed_field_window_projection_find_element_xi(struct Computed_field *field, 
	FE_value *values, int number_of_values, struct FE_element **element,
	FE_value *xi, struct GROUP(FE_element) *search_element_group)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
==============================================================================*/
{
	double d,lu_matrix[16],result[4];
	int indx[4],return_code;
	FE_value source_values[3];
	struct Computed_field_window_projection_type_specific_data *data;

	ENTER(Computed_field_window_projection_find_element_xi);
	if (field&&values&&(number_of_values==field->number_of_components)&&element&&xi&&
		search_element_group)
	{
		if (data->projection_matrix ||
			Computed_field_window_projection_calculate_matrix(field))
		{
			copy_matrix(4,4,data->projection_matrix,lu_matrix);
			result[0] = (double)values[0];
			result[1] = (double)values[1];
			result[2] = (double)values[2];
			result[3] = 1.0;
			if (LU_decompose(4,lu_matrix,indx,&d) &&
				LU_backsubstitute(4,lu_matrix,indx,result) &&
				(0.0 != result[3]))
			{
				source_values[0] = (result[0] / result[3]);
				source_values[1] = (result[1] / result[3]);
				source_values[2] = (result[2] / result[3]);
				return_code=Computed_field_find_element_xi(
					field->source_fields[0], source_values, 3, element,
					xi, search_element_group);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_window_projection_find_element_xi.  "
					"Could not invert field %s",field->name);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_window_projection_find_element_xi.  "
				"Missing projection matrix for field %s",field->name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_window_projection_find_element_xi.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_window_projection_find_element_xi */

static int list_Computed_field_window_projection(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 6 October 2000

DESCRIPTION :
==============================================================================*/
{
	char *window_name;
	int return_code;
	struct Computed_field_window_projection_type_specific_data *data;

	ENTER(List_Computed_field_window_projection);
	if (field && (data = 
		(struct Computed_field_window_projection_type_specific_data *)
		field->type_specific_data))
	{
		return_code = 1;
		display_message(INFORMATION_MESSAGE,"    source field : %s\n",
			field->source_fields[0]->name);
		if (GET_NAME(Graphics_window)(data->graphics_window, &window_name))
		{
			display_message(INFORMATION_MESSAGE,"    window : %s\n",
				window_name);
			DEALLOCATE(window_name);
		}
		display_message(INFORMATION_MESSAGE,"    pane number : %d\n",
			data->pane_number + 1);
		display_message(INFORMATION_MESSAGE,"    projection type : %s\n",
			Computed_field_window_projection_type_string(data->projection_type));
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

static char *Computed_field_window_projection_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40], *window_name;
	int error;
	struct Computed_field_window_projection_type_specific_data *data;

	ENTER(Computed_field_window_projection_get_command_string);
	command_string = (char *)NULL;
	if (field && (data = 
		(struct Computed_field_window_projection_type_specific_data *)
		field->type_specific_data))
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
		if (GET_NAME(Graphics_window)(data->graphics_window, &window_name))
		{
			make_valid_token(&window_name);
			append_string(&command_string, window_name, &error);
			DEALLOCATE(window_name);
		}
		sprintf(temp_string, " pane_number %d ", data->pane_number + 1);
		append_string(&command_string, temp_string, &error);
		append_string(&command_string,
			Computed_field_window_projection_type_string(data->projection_type),
			&error);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_window_projection_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_window_projection_get_command_string */

#define Computed_field_window_projection_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_window_projection(struct Computed_field *field,
	struct Computed_field *source_field, struct Graphics_window *graphics_window,
	int pane_number, enum Computed_field_window_projection_type projection_type,
	struct Computed_field_window_projection_package *package)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_WINDOW_PROJECTION, returning the 
<source_field> with each component multiplied by the perspective transformation
of the <graphics_window>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;
	struct Computed_field_window_projection_type_specific_data *data;
	struct Scene_viewer *scene_viewer;

	ENTER(Computed_field_set_type_window_projection);
	if (field&&source_field&&
		Computed_field_has_3_components(source_field, NULL)
		&&graphics_window)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields)&&
			ALLOCATE(data,struct Computed_field_window_projection_type_specific_data, 1)&&
			(scene_viewer = Graphics_window_get_Scene_viewer(graphics_window, 
			pane_number)))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_NEW_TYPES;
			field->type_string = computed_field_window_projection_type_string;
			field->number_of_components = 3;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)data;
			data->projection_matrix = (double *)NULL;
			data->graphics_window = ACCESS(Graphics_window)(graphics_window);
			data->pane_number = pane_number;
			data->scene_viewer = scene_viewer;
			data->projection_type = projection_type;
			data->scene_viewer_callback_flag = 0;
			data->package = package;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(window_projection);
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
	struct Computed_field **source_field, struct Graphics_window **graphics_window,
	int *pane_number, enum Computed_field_window_projection_type *projection_type)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_WINDOW_PROJECTION, the <source_field>,
<graphics_window>, <pane_number> and <projection_type> used by it are returned.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_window_projection_type_specific_data *data;

	ENTER(Computed_field_get_type_window_projection);
	if (field&&(COMPUTED_FIELD_NEW_TYPES==field->type)&&
		(field->type_string==computed_field_window_projection_type_string)
		&&(data = 
		(struct Computed_field_window_projection_type_specific_data *)
		field->type_specific_data)&&source_field&&graphics_window&&
	   pane_number&&projection_type)
	{
		*source_field = field->source_fields[0];
		*graphics_window = data->graphics_window;
		*pane_number = data->pane_number;
		*projection_type = data->projection_type;
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

static int define_Computed_field_type_window_projection(struct Parse_state *state,
	void *field_void,void *computed_field_window_projection_package_void)
/*******************************************************************************
LAST MODIFIED : 18 December 2001

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_WINDOW_PROJECTION (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	char *projection_type_string;
	enum Computed_field_window_projection_type projection_type;
	int pane_number, return_code;
	struct Computed_field *field,*source_field;
	struct Computed_field_window_projection_package 
		*computed_field_window_projection_package;
	struct Graphics_window *graphics_window;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;
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
		(struct Computed_field_window_projection_package *)
		computed_field_window_projection_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		pane_number = 1;
		projection_type = TEXTURE_PROJECTION;
		source_field = (struct Computed_field *)NULL;
		graphics_window = (struct Graphics_window *)NULL;
		if (computed_field_window_projection_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code=Computed_field_get_type_window_projection(field,
				&source_field, &graphics_window, &pane_number, &projection_type);
			pane_number++;
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
				return_code = Computed_field_set_type_window_projection(field,
					source_field, graphics_window, pane_number - 1, projection_type,
					computed_field_window_projection_package);
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
LAST MODIFIED : 5 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_window_projection_package 
		computed_field_window_projection_package;

	ENTER(Computed_field_register_type_window_projection);
	if (computed_field_package && graphics_window_manager)
	{
		computed_field_window_projection_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		computed_field_window_projection_package.graphics_window_manager =
			graphics_window_manager;
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_window_projection_type_string, 
			define_Computed_field_type_window_projection,
			&computed_field_window_projection_package);
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

