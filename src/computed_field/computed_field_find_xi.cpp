/*******************************************************************************
FILE : computed_field_find_xi.cpp

LAST MODIFIED : 24 August 2006

DESCRIPTION :
Implements a special version of find_xi that uses OpenGL to accelerate the
lookup of the element.
==============================================================================*/
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <cstdio>
#include <cmath>

#include "general/debug.h"
#include "general/matrix_vector.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_find_xi_private.hpp"
#include "finite_element/finite_element_discretization.h"
#include "finite_element/finite_element_mesh_field_ranges.hpp"
#include "finite_element/finite_element_region.h"
#include "general/message.h"
#include "mesh/mesh.hpp"

#define MAX_FIND_XI_ITERATIONS 50


Computed_field_find_element_xi_cache::Computed_field_find_element_xi_cache(cmzn_field *fieldIn) :
	field(fieldIn),
	searchMesh(nullptr),
	element(nullptr),
	componentsCount(field->getNumberOfComponents()),
	time(0),
	values(new FE_value[this->componentsCount]),
	workingValues(new FE_value[this->componentsCount])
{
}

Computed_field_find_element_xi_cache::~Computed_field_find_element_xi_cache()
{
	if (this->searchMesh)
	{
		cmzn_mesh_destroy(&this->searchMesh);
	}
	delete[] this->values;
	delete[] this->workingValues;
}


int Computed_field_iterative_element_conditional(struct FE_element *element,
	struct Computed_field_iterative_find_element_xi_data *data)
{
	double a[MAXIMUM_ELEMENT_XI_DIMENSIONS*MAXIMUM_ELEMENT_XI_DIMENSIONS],
		b[MAXIMUM_ELEMENT_XI_DIMENSIONS], b2[MAXIMUM_ELEMENT_XI_DIMENSIONS], d, sum;
	FE_value last_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int i, indx[MAXIMUM_ELEMENT_XI_DIMENSIONS], iterations, j, k,
		number_of_xi_points_created[MAXIMUM_ELEMENT_XI_DIMENSIONS], return_code;
	struct FE_element_shape *shape;

	if (element && data)
	{
		const int number_of_xi = element->getDimension();
		if (number_of_xi <= data->number_of_values)
		{
			return_code = 1;
			shape = get_FE_element_shape(element);
			if (data->start_with_data_xi)
			{
				for (i = 0; i < number_of_xi; i++)
				{
					last_xi[i] = data->xi[i];
				}
			}
			else
			{
				/* Find a good estimate of the centre to start with */
				int number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS] = { 1, 1, 1 };
				FE_value_triple *xi_points;
				if (FE_element_shape_get_xi_points_cell_centres(shape,
					number_in_xi, number_of_xi_points_created, &xi_points))
				{
					for (i = 0; i < number_of_xi; i++)
					{
						data->xi[i] = xi_points[0][i];
						last_xi[i] = xi_points[0][i];
					}
					DEALLOCATE(xi_points);
				}
				else
				{
					for (i = 0; i < number_of_xi; i++)
					{
						data->xi[i] = 0.5;
						last_xi[i] = 0.5;
					}
				}
			}
			FE_value maxDeltaXi = 0.2;  // reduced after certain iterations
			FE_value dxi2[MAXIMUM_ELEMENT_XI_DIMENSIONS];
			FE_value *faceNormal = data->workingValues;
			const FieldDerivative *fieldDerivative = element->getMesh()->getFieldDerivative(/*order*/1);
			const FE_value *values = nullptr;
			const FE_value *derivatives = nullptr;
			bool converged = false;
			iterations = 0;
			while (true)
			{
				const int result = data->field_cache->setMeshLocation(element, data->xi);
				const RealFieldValueCache *realValueCache = RealFieldValueCache::cast(data->field->evaluate(*data->field_cache));
				const DerivativeValueCache *derivativeValueCache = data->field->evaluateDerivative(*data->field_cache, *fieldDerivative);
				if (!((CMZN_OK == result) && (realValueCache) && (derivativeValueCache)))
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_iterative_element_conditional.  "
						"Could not evaluate field");
					return_code = 0;
					break;
				}
				values = realValueCache->values;
				derivatives = derivativeValueCache->values;
				/* least-squares approach: make the derivatives / right hand side
					vector into square system to solve for delta-Xi */
				for (i = 0; i < number_of_xi; i++)
				{
					for (j = 0; j < number_of_xi; j++)
					{
						sum = 0.0;
						for (k = 0; k < data->number_of_values; k++)
						{
							sum += (double)derivatives[k*number_of_xi + j] *
								(double)derivatives[k*number_of_xi + i];
						}
						a[i*number_of_xi + j] = sum;
					}
					sum = 0.0;
					for (k = 0; k < data->number_of_values; k++)
					{
						sum += (double)derivatives[k*number_of_xi + i] *
							((double)data->values[k] - (double)values[k]);
					}
					b[i] = sum;
				}
				if (!(LU_decompose(number_of_xi, a, indx, &d,/*singular_tolerance*/1.0e-12) &&
					LU_backsubstitute(number_of_xi, a, indx, b)))
				{
					/* Probably singular matrix, no longer report error
						as a collapsed element gets reported on every
						pixel making it unusable.*/
					return_code = 0;
					break;
				}
				// have limits on delta xi to handle high field curvature
				if (iterations == 6)
				{
					maxDeltaXi *= 0.5;
				}
				else if (iterations == 21)
				{
					maxDeltaXi *= 0.25;
				}
				double mag_dxi = 0.0;
				for (i = 0; i < number_of_xi; ++i)
				{
					mag_dxi += b[i]*b[i];
				}
				mag_dxi = sqrt(mag_dxi);
				const FE_value dxi_scale = (data->find_nearest_location) && (mag_dxi > maxDeltaXi) ? (maxDeltaXi / mag_dxi) : 1.0;
				for (i = 0; i < number_of_xi; i++)
				{
					data->xi[i] += dxi_scale * b[i];
				}
				bool xiLimited = FE_element_shape_limit_xi_to_element(shape, data->xi, 0.0);
				converged = true;
				for (i = 0; i < number_of_xi; i++)
				{
					/* converged if all xi increments on or within tolerance */
					if (fabs(b[i]) > data->xi_tolerance)
					{
						converged = false;
						break;
					}
				}
				++iterations;
				if (converged)
				{
					break;
				}
				// only want to do boundary optimisation if this step started and finished on a boundary of dimension > 0
				if (xiLimited && (number_of_xi > 1))
				{
					for (i = 0; i < number_of_xi; i++)
					{
						dxi2[i] = 0.0;
					}
					int faceNumber = -1;
					int faceCount = 0;
					// try all faces the point could be on, otherwise it can get stuck on the wrong face. Accumulate xi increments
					// note that displacement on one face may move it off another so subsequent faces are not searched - this is wanted behaviour
					while ((0 <= (faceNumber = FE_element_shape_find_face_number_for_xi(shape, last_xi, data->xi_tolerance, faceNumber)))
						&& FE_element_shape_get_face_outward_normal(shape, faceNumber, data->number_of_values, derivatives, faceNormal))
					{
						// sliding along boundary face: subtract component of delta x normal to surface and recompute dxi
						++faceCount;
						FE_value coeffNormal = 0.0;
						for (k = 0; k < data->number_of_values; k++)
						{
							coeffNormal += (data->values[k] - values[k]) * faceNormal[k];
						}
						for (i = 0; i < number_of_xi; i++)
						{
							sum = 0.0;
							for (k = 0; k < data->number_of_values; k++)
							{
								sum += (double)derivatives[k*number_of_xi + i] *
									((double)data->values[k] - (double)values[k] - coeffNormal * faceNormal[k]);
							}
							b2[i] = sum;
						}
						if (!LU_backsubstitute(number_of_xi, a, indx, b2))
						{
							return_code = 0;
							break;
						}
						FE_value scaleNext = 1.0 / static_cast<FE_value>(faceCount);
						FE_value scalePrev = 1.0 - scaleNext;
						for (i = 0; i < number_of_xi; i++)
						{
							dxi2[i] = scalePrev * dxi2[i] + scaleNext * b2[i];
						}
						FE_value mag_dxi2 = 0.0;
						for (i = 0; i < number_of_xi; ++i)
						{
							mag_dxi2 += dxi2[i] * dxi2[i];
						}
						mag_dxi2 = sqrt(mag_dxi2);
						const FE_value dxi2_scale = (mag_dxi2 > maxDeltaXi) ? (maxDeltaXi / mag_dxi2) : 1.0;
						for (i = 0; i < number_of_xi; i++)
						{
							data->xi[i] = last_xi[i] + dxi2_scale * dxi2[i];
						}
						xiLimited = FE_element_shape_limit_xi_to_element(shape, data->xi, 0.0);
					}
				}
				if (iterations == MAX_FIND_XI_ITERATIONS)
				{
					/* too many iterations; give up */
					return_code = 0;
					break;
				}
				else if (1 < iterations)
				{
					/* give up if xi not changed this iteration; usually means
						the solution is outside this element */
					return_code = 0;
					for (i = 0; i < number_of_xi; i++)
					{
						if (fabs(data->xi[i] - last_xi[i]) > data->xi_tolerance)
						{
							return_code = 1;
							break;
						}
					}
					if (!return_code)
					{
						break;
					}
				}
				for (i = 0; i < number_of_xi; i++)
				{
					last_xi[i] = data->xi[i];
				}
			}
			/* if field has more components than xi-directions, must
				check all components have converged */
			if (converged && (data->number_of_values > number_of_xi))
			{
				/* strategy for determining convergence is to multiply
					the derivative for each component by the last increment
					in xi, stored in b, and ensuring it is larger than the
					difference between target and current field values.
					Broadly, this means the given component is not wishing
					the solution to shift more than the last xi increment.
					Note that the increment in xi is doubled to make this
					test slightly less strict than original convergence */
				for (k = 0; k < data->number_of_values; k++)
				{
					sum = 0.0;
					for (i = 0; i < number_of_xi; i++)
					{
						sum += (double)derivatives[k*number_of_xi + i] * b[i];
					}
					if (2.0*fabs(sum) <
						fabs((double)data->values[k] - (double)values[k]))
					{
						return_code = 0;
					}
				}
			}
			if (data->find_nearest_location)
			{
				sum = 0.0;
				for (k = 0; k < data->number_of_values; k++)
				{
					sum += ((double)data->values[k] - (double)values[k]) *
						((double)data->values[k] - (double)values[k]);
				}
				if (!data->nearest_element ||
					(sum < data->nearest_element_distance_squared))
				{
					data->nearest_element = element;
					for (i = 0; i < number_of_xi; i++)
					{
						data->nearest_xi[i] = data->xi[i];
					}
					data->nearest_element_distance_squared = sum;
				}
			}
#if defined (DEBUG_CODE)
			display_message(INFORMATION_MESSAGE,
				"Computed_field_iterative_element_conditional.  "
				"Converged %d iterations %d\n", converged, iterations);
#endif /* defined (DEBUG_CODE) */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_iterative_element_conditional.  "
				"Unable to solve underdetermined system");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_iterative_element_conditional.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

#undef MAX_FIND_XI_ITERATIONS

/** Determine if element is within tolerance of element */
inline bool checkElement(int componentsCount, const FE_value *values, cmzn_element *element,
	const FeMeshFieldRanges *meshFieldRanges, FE_value tolerance)
{
	if ((meshFieldRanges) && (tolerance >= 0.0))
	{
		const FeElementFieldRange *elementFieldRange = meshFieldRanges->getElementFieldRange(element->getIndex());
		if (elementFieldRange)
		{
			return elementFieldRange->valuesInRange(componentsCount, values, tolerance);
		}
	}
	return true;
}

int Computed_field_find_element_xi(struct Computed_field *field,
	cmzn_fieldcache_id field_cache,
	Computed_field_find_element_xi_cache *findElementXiCache,
	const FeMeshFieldRanges *meshFieldRanges,
	const FE_value *values, int number_of_values,
	struct FE_element **element_address, FE_value *xi,
	cmzn_mesh *searchMesh, int find_nearest)
{
	struct Computed_field_iterative_find_element_xi_data find_element_xi_data;
	int i, number_of_xi = -1;
	int return_code = 1;

	RealFieldValueCache *valueCache = dynamic_cast<RealFieldValueCache*>(field->getValueCache(*field_cache));
	const int element_dimension = searchMesh ?
		cmzn_mesh_get_dimension(searchMesh) : cmzn_element_get_dimension(*element_address);
	if (field && (valueCache) && (findElementXiCache) &&
		(findElementXiCache->getField() == field) &&
		(values) && (number_of_values == field->getNumberOfComponents()) &&
		(element_address) && (xi) && ((searchMesh) || (*element_address)) &&
		(number_of_values >= element_dimension))
	{
		findElementXiCache->time = field_cache->getTime();
		if (return_code)
		{
			find_element_xi_data.values = findElementXiCache->values;
			/* copy the source values */
			for (i = 0; i < number_of_values; i++)
			{
				find_element_xi_data.values[i] = values[i];
			}
			find_element_xi_data.workingValues = findElementXiCache->workingValues;
			find_element_xi_data.field_cache = field_cache;
			find_element_xi_data.field = field;
			find_element_xi_data.number_of_values = number_of_values;
			find_element_xi_data.xi_tolerance = 1e-05;
			find_element_xi_data.find_nearest_location = find_nearest;
			find_element_xi_data.nearest_element = (struct FE_element *)NULL;
			find_element_xi_data.nearest_element_distance_squared = 0.0;
			find_element_xi_data.start_with_data_xi = 0;

			if (searchMesh)
			{
				FE_value tolerance = (meshFieldRanges) && (!find_nearest) ? meshFieldRanges->getTolerance() : -1.0;
				*element_address = (struct FE_element *)NULL;

				/* Try the cached element first if it is in the mesh */
				cmzn_element *element = findElementXiCache->element;
				if ((element) && cmzn_mesh_contains_element(searchMesh, element))
				{
					if (checkElement(number_of_values, values, element, meshFieldRanges, tolerance))
					{
						if (Computed_field_iterative_element_conditional(element, &find_element_xi_data))
						{
							*element_address = element;
						}
						else if (find_element_xi_data.nearest_element == element)
						{
							tolerance = sqrt(find_element_xi_data.nearest_element_distance_squared);
						}
					}
				}
				/* Now try every element */
				if (!*element_address)
				{
					cmzn_elementiterator *iterator = searchMesh->createElementiterator();
					while (0 != (element = cmzn_elementiterator_next_non_access(iterator)))
					{
						if (element != findElementXiCache->element)  // since already tried it
						{
							if (checkElement(number_of_values, values, element, meshFieldRanges, tolerance))
							{
								if (Computed_field_iterative_element_conditional(element, &find_element_xi_data))
								{
									*element_address = element;
									break;
								}
								else if (find_element_xi_data.nearest_element == element)
								{
									tolerance = sqrt(find_element_xi_data.nearest_element_distance_squared);
								}
							}
						}
					}
					cmzn::Deaccess(iterator);
				}
			}
			else
			{
				// don't expect to use mesh field ranges for single element case:
				if ((!Computed_field_iterative_element_conditional(
					*element_address, &find_element_xi_data)))
				{
					*element_address = (struct FE_element *)NULL;
				}
			}
			/* If an exact match is not found then accept the closest one */
			if (!*element_address && find_nearest)
			{
				if (find_element_xi_data.nearest_element)
				{
					*element_address = find_element_xi_data.nearest_element;
					number_of_xi = get_FE_element_dimension(*element_address);
					for (i = 0 ; i < number_of_xi ; i++)
					{
						xi[i] = find_element_xi_data.nearest_xi[i];
					}
				}
			}
			else if (*element_address)
			{
				number_of_xi = get_FE_element_dimension(*element_address);
				for (i = 0 ; i < number_of_xi ; i++)
				{
					xi[i] = find_element_xi_data.xi[i];
				}
			}
			/* The search is valid even if the element wasn't found */
			return_code = 1;
			/* Remember the element and search mesh in the findElementXiCache */
			findElementXiCache->element = *element_address;
			findElementXiCache->setSearchMesh(searchMesh);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_find_element_xi.  "
				"Unable to allocate value memory.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_find_element_xi.  Invalid argument(s)");
		return_code=0;
	}
	return (return_code);
}
