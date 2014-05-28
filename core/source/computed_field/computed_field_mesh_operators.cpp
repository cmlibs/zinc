/**
 * FILE : computed_field_mesh_operators.cpp
 *
 * Field operators that act on a mesh: integration etc.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <cmath>
#include <iostream>
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_mesh_operators.hpp"
#include "computed_field/field_module.hpp"
#include "mesh/cmiss_element_private.hpp"
#include "zinc/fieldmeshoperators.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_set.h"
#include "element/element_operations.h"
#include "region/cmiss_region.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"
#include "finite_element/finite_element_region.h"

namespace {

const char computed_field_mesh_integral_type_string[] = "mesh_integral";

// assumes there are two source fields: 1. integrand and 2. coordinate
class Computed_field_mesh_integral : public Computed_field_core
{
protected:
	cmzn_mesh_id mesh;
	cmzn_element_quadrature_rule quadratureRule;
	std::vector<int> numbersOfPoints;

public:
	Computed_field_mesh_integral(cmzn_mesh_id meshIn) :
		Computed_field_core(),
		mesh(cmzn_mesh_access(meshIn)),
		quadratureRule(CMZN_ELEMENT_QUADRATURE_RULE_GAUSSIAN)
	{
		numbersOfPoints.push_back(1);
	}

	virtual ~Computed_field_mesh_integral()
	{
		cmzn_mesh_destroy(&mesh);
	}

	Computed_field_core *copy()
	{
		return new Computed_field_mesh_integral(mesh);
	}

	const char *get_type_string()
	{
		return (computed_field_mesh_integral_type_string);
	}

	int compare(Computed_field_core* other_core)
	{
		Computed_field_mesh_integral *other =
			dynamic_cast<Computed_field_mesh_integral*>(other_core);
		if (other)
			return cmzn_mesh_match(mesh, other->mesh);
		return 0;
	}

	virtual FieldValueCache *createValueCache(cmzn_fieldcache& parentCache)
	{
		RealFieldValueCache *valueCache = new RealFieldValueCache(field->number_of_components);
		valueCache->createExtraCache(parentCache, Computed_field_get_region(field));
		return valueCache;
	}

	virtual bool is_defined_at_location(cmzn_fieldcache& cache);

	void appendNumbersOfPointsString(char **theString, int *error) const;

	int list();

	char* get_command_string();

	cmzn_mesh_id getMesh()
	{
		return mesh;
	}

	int getNumbersOfPoints(int valuesCount, int *values)
	{
		if ((0 == valuesCount) || ((0 < valuesCount) && values))
		{
			const int size = static_cast<int>(this->numbersOfPoints.size());
			for (int i = 0; i < valuesCount; ++i)
				values[i] = this->numbersOfPoints[(i < size) ? i : size - 1];
			return size;
		}
		return 0;
	}

	int setNumbersOfPoints(int valuesCount, const int *values)
	{
		if ((0 < valuesCount) && values)
		{
			for (int i = 0; i < valuesCount; ++i)
			{
				if (values[i] < 1)
					return CMZN_ERROR_ARGUMENT;
			}
			bool change = (static_cast<int>(this->numbersOfPoints.size()) != valuesCount);
			this->numbersOfPoints.resize(valuesCount);
			for (int i = 0; i < valuesCount; ++i)
			{
				if (this->numbersOfPoints[i] != values[i])
				{
					this->numbersOfPoints[i] = values[i];
					change = true;
				}
			}
			if (change && this->field)
				Computed_field_changed(this->field);
			return CMZN_OK;
		}
		return CMZN_ERROR_ARGUMENT;
	}

	cmzn_element_quadrature_rule getElementQuadratureRule() const
	{
		return this->quadratureRule;
	}

	int setElementQuadratureRule(cmzn_element_quadrature_rule quadratureRuleIn)
	{
		if ((quadratureRuleIn == CMZN_ELEMENT_QUADRATURE_RULE_GAUSSIAN) ||
			(quadratureRuleIn == CMZN_ELEMENT_QUADRATURE_RULE_MIDPOINT))
		{
			if (this->quadratureRule != quadratureRuleIn)
			{
				this->quadratureRule = quadratureRuleIn;
				Computed_field_changed(this->field);
			}
			return CMZN_OK;
		}
		return CMZN_ERROR_ARGUMENT;
	}

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

protected:
	template <class ProcessTerm> int evaluateTerms(ProcessTerm &processTerm);
};

template <class ProcessTerm> int Computed_field_mesh_integral::evaluateTerms(ProcessTerm &processTerm)
{
	int result = 1;
	cmzn_elementiterator_id iterator = cmzn_mesh_create_elementiterator(mesh);
	cmzn_element_id element = 0;
	IntegrationPointsCache integrationCache(this->quadratureRule, static_cast<int>(this->numbersOfPoints.size()), this->numbersOfPoints.data());
	while (0 != (element = cmzn_elementiterator_next_non_access(iterator)))
	{
		IntegrationShapePoints *shapePoints = integrationCache.getPoints(element);
		if (0 == shapePoints)
		{
			result = 0;
			break;
		}
		processTerm.setElement(element);
		shapePoints->forEachPoint(processTerm);
	}
	cmzn_elementiterator_destroy(&iterator);
	return result;
}

class IntegralTermBase
{
protected:
	Computed_field_mesh_integral& meshIntegral;
	const int dimension;
	const int componentsCount;
	cmzn_fieldcache& cache;
	cmzn_field *integrandField;
	cmzn_field *coordinateField;
	const int coordinatesCount;
	cmzn_element *element;

public:
	IntegralTermBase(Computed_field_mesh_integral& meshIntegralIn, cmzn_fieldcache& parentCache, RealFieldValueCache& valueCache) :
		meshIntegral(meshIntegralIn),
		dimension(cmzn_mesh_get_dimension(meshIntegral.getMesh())),
		componentsCount(meshIntegralIn.getField()->number_of_components),
		cache(*(valueCache.getExtraCache())),
		integrandField(meshIntegral.getSourceField(0)),
		coordinateField(meshIntegral.getSourceField(1)),
		coordinatesCount(coordinateField->number_of_components),
		element(0)
	{
		cache.setTime(parentCache.getTime());
	}

	void setElement(cmzn_element *elementIn)
	{
		element = elementIn;
	}

	/** @return pointer to integrand values */
	inline FE_value *baseProcess(FE_value *xi, FE_value &dLAV)
	{
		this->cache.setMeshLocation(this->element, xi);
		RealFieldValueCache *integrandValueCache = RealFieldValueCache::cast(integrandField->evaluate(cache));
		RealFieldValueCache *coordinateValueCache = coordinateField->evaluateWithDerivatives(cache, dimension);
		if (integrandValueCache && coordinateValueCache)
		{
			// note dx_dxi cycles over xi fastest
			FE_value *dx_dxi = coordinateValueCache->derivatives;
			dLAV = 0.0; // dL (1-D), dA (2-D), dV (3-D)
			switch (this->dimension)
			{
			case 1:
				for (int c = 0; c < coordinatesCount; ++c)
					dLAV += dx_dxi[c]*dx_dxi[c];
				dLAV = sqrt(dLAV);
				break;
			case 2:
				if (coordinatesCount == 2)
					dLAV = fabs(dx_dxi[0]*dx_dxi[3] - dx_dxi[1]*dx_dxi[2]);
				else
				{
					// dA = magnitude of dx_dxi1 (x) dx_dxi2
					FE_value n1 = dx_dxi[2]*dx_dxi[5] - dx_dxi[3]*dx_dxi[4];
					FE_value n2 = dx_dxi[4]*dx_dxi[1] - dx_dxi[5]*dx_dxi[0];
					FE_value n3 = dx_dxi[0]*dx_dxi[3] - dx_dxi[1]*dx_dxi[2];
					dLAV = n1*n1 + n2*n2 + n3*n3;
					dLAV = sqrt(dLAV);
				}
				break;
			case 3:
				dLAV = fabs(
					dx_dxi[0]*(dx_dxi[4]*dx_dxi[8] - dx_dxi[7]*dx_dxi[5]) +
					dx_dxi[3]*(dx_dxi[7]*dx_dxi[2] - dx_dxi[1]*dx_dxi[8]) +
					dx_dxi[6]*(dx_dxi[1]*dx_dxi[5] - dx_dxi[4]*dx_dxi[2]));
				break;
			}
			return integrandValueCache->values;
		}
		// abandon elements where integrand or coordinates not defined
		return 0;
	}
};

class IntegralTermSum : public IntegralTermBase
{
	FE_value *values;

public:
	IntegralTermSum(Computed_field_mesh_integral& meshIntegralIn,
			cmzn_fieldcache& parentCache, RealFieldValueCache& valueCache) :
		IntegralTermBase(meshIntegralIn, parentCache, valueCache),
		values(valueCache.values)
	{
		for (int i = 0; i < componentsCount; i++)
			values[i] = 0;
		valueCache.derivatives_valid = 0;
	}

	inline bool operator()(FE_value *xi, FE_value weight)
	{
		FE_value dLAV;
		FE_value *integrandValues = baseProcess(xi, dLAV);
		if (integrandValues)
		{
			const FE_value weight_dLAV = weight*dLAV;
			for (int i = 0; i < this->componentsCount; ++i)
				this->values[i] += integrandValues[i]*weight_dLAV;
			return true;
		}
		return false;
	}

	static inline bool invoke(void *termVoid, FE_value *xi, FE_value weight)
	{
		return (*(reinterpret_cast<IntegralTermSum*>(termVoid)))(xi, weight);
	}
};

int Computed_field_mesh_integral::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	IntegralTermSum sumTerms(*this, cache, RealFieldValueCache::cast(inValueCache));
	return this->evaluateTerms(sumTerms);
}

bool Computed_field_mesh_integral::is_defined_at_location(cmzn_fieldcache& cache)
{
	return true;
}

void Computed_field_mesh_integral::appendNumbersOfPointsString(char **theString, int *error) const
{
	if (theString && error)
	{
		char temp[20];
		for (size_t i = 0; i < this->numbersOfPoints.size(); ++i)
		{
			if (i)
				append_string(theString, "*", error);
			sprintf(temp, "%d", this->numbersOfPoints[i]);
			append_string(theString, temp, error);
		}
	}
}

/** Lists a description of the mesh_operator arguments */
int Computed_field_mesh_integral::list()
{
	if (field)
	{
		display_message(INFORMATION_MESSAGE, "    integrand field : %s\n",
			getSourceField(0)->name);
		display_message(INFORMATION_MESSAGE, "    coordinate field : %s\n",
			getSourceField(1)->name);
		char *mesh_name = cmzn_mesh_get_name(mesh);
		display_message(INFORMATION_MESSAGE, "    mesh : %s\n", mesh_name);
		DEALLOCATE(mesh_name);
		display_message(INFORMATION_MESSAGE, "    rule: %s\n",
			ENUMERATOR_STRING(cmzn_element_quadrature_rule)(this->quadratureRule));
		char *numbersOfPointsString = 0;
		int error = 0;
		this->appendNumbersOfPointsString(&numbersOfPointsString, &error);
		display_message(INFORMATION_MESSAGE, "    numbers of points: %s\n", numbersOfPointsString);
		DEALLOCATE(numbersOfPointsString);
		return 1;
	}
	return 0;
}

/** Returns allocated command string for reproducing field. Includes type. */
char *Computed_field_mesh_integral::get_command_string()
{
	char *command_string = 0;
	if (field)
	{
		int error = 0;
		append_string(&command_string, get_type_string(), &error);
		append_string(&command_string, " integrand_field ", &error);
		append_string(&command_string, getSourceField(0)->name, &error);
		append_string(&command_string, " coordinate_field ", &error);
		append_string(&command_string, getSourceField(1)->name, &error);
		char *mesh_name = cmzn_mesh_get_name(mesh);
		append_string(&command_string, " mesh ", &error);
		make_valid_token(&mesh_name);
		append_string(&command_string, mesh_name, &error);
		DEALLOCATE(mesh_name);
		append_string(&command_string, " ", &error);
		append_string(&command_string,
			ENUMERATOR_STRING(cmzn_element_quadrature_rule)(this->quadratureRule), &error);
		append_string(&command_string, " numbers_of_points \"", &error);
		this->appendNumbersOfPointsString(&command_string, &error);
		append_string(&command_string, "\"", &error);
	}
	return (command_string);
}

const char computed_field_mesh_integral_squares_type_string[] = "mesh_integral_squares";

class Computed_field_mesh_integral_squares : public Computed_field_mesh_integral
{
public:
	Computed_field_mesh_integral_squares(cmzn_mesh_id meshIn) :
		Computed_field_mesh_integral(meshIn)
	{
	}

	Computed_field_core *copy()
	{
		return new Computed_field_mesh_integral_squares(mesh);
	}

	const char *get_type_string()
	{
		return (computed_field_mesh_integral_squares_type_string);
	}

	int compare(Computed_field_core* other_core)
	{
		Computed_field_mesh_integral_squares *other =
			dynamic_cast<Computed_field_mesh_integral_squares*>(other_core);
		if (other)
			return Computed_field_mesh_integral::compare(other_core);
		return 0;
	}

	virtual bool supports_sum_square_terms() const
	{
		return true;
	}

	virtual int get_number_of_sum_square_terms(cmzn_fieldcache& cache) const;

	int evaluate_sum_square_terms(cmzn_fieldcache& cache, RealFieldValueCache& valueCache,
		int number_of_values, FE_value *values);

	int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);
};

int Computed_field_mesh_integral_squares::get_number_of_sum_square_terms(cmzn_fieldcache& cache) const
{
	int number_of_terms = 0;
	cmzn_field_id integrandField = getSourceField(0);
	cmzn_field_id coordinateField = getSourceField(1);
	IntegrationPointsCache integrationCache(this->quadratureRule,
		static_cast<int>(this->numbersOfPoints.size()), this->numbersOfPoints.data());
	cmzn_elementiterator_id iterator = cmzn_mesh_create_elementiterator(mesh);
	cmzn_element_id element = 0;
	while (0 != (element = cmzn_elementiterator_next_non_access(iterator)))
	{
		cache.setElement(element);
		if (cmzn_field_is_defined_at_location(integrandField, &cache) &&
			cmzn_field_is_defined_at_location(coordinateField, &cache))
		{
			IntegrationShapePoints *shapePoints = integrationCache.getPoints(element);
			if (shapePoints)
				number_of_terms += shapePoints->getNumPoints();
			else
			{
				number_of_terms = 0;
				break;
			}
		}
	}
	cmzn_elementiterator_destroy(&iterator);
	return number_of_terms;
}

class IntegralTermAppendSquares : public IntegralTermBase
{
	int remainingValuesCount;
	FE_value *termValues;

public:
	IntegralTermAppendSquares(Computed_field_mesh_integral& meshIntegralIn,
			cmzn_fieldcache& parentCache, RealFieldValueCache& valueCache,
			int termValuesCountIn, FE_value *termValuesIn) :
		IntegralTermBase(meshIntegralIn, parentCache, valueCache),
		remainingValuesCount(termValuesCountIn),
		termValues(termValuesIn)
	{
	}

	int getRemainingValuesCount() const
	{
		return this->remainingValuesCount;
	}

	inline bool operator()(FE_value *xi, FE_value weight)
	{
		FE_value dLAV;
		FE_value *integrandValues = baseProcess(xi, dLAV);
		if (integrandValues)
		{
			this->remainingValuesCount -= this->componentsCount;
			if (this->remainingValuesCount < 0)
				return false;
			const FE_value sqrt_weight_dLAV = (weight < 0.0) ? -sqrt(-weight*dLAV) : sqrt(weight*dLAV);
			for (int i = 0; i < this->componentsCount; ++i)
				this->termValues[i] = integrandValues[i]*sqrt_weight_dLAV;
			this->termValues += this->componentsCount;
			return true;
		}
		return false;
	}

	static inline bool invoke(void *termVoid, FE_value *xi, FE_value weight)
	{
		return (*(reinterpret_cast<IntegralTermAppendSquares*>(termVoid)))(xi, weight);
	}
};

int Computed_field_mesh_integral_squares::evaluate_sum_square_terms(
	cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, int number_of_values, FE_value *values)
{
	IntegralTermAppendSquares appendSquares(*this, cache,
		RealFieldValueCache::cast(inValueCache), number_of_values, values);
	int result = this->evaluateTerms(appendSquares);
	if (result && (appendSquares.getRemainingValuesCount() != 0))
	{
		display_message(ERROR_MESSAGE, "Computed_field_mesh_integral_squares.evaluate_sum_square_terms  "
			"Field %s: expected %d values; actual number %d\n",
			this->field->name, number_of_values, number_of_values - appendSquares.getRemainingValuesCount());
		result = 0;
	}
	return result;
}

class IntegralTermSumSquares : public IntegralTermBase
{
	FE_value *values;

public:
	IntegralTermSumSquares(Computed_field_mesh_integral& meshIntegralIn,
			cmzn_fieldcache& parentCache, RealFieldValueCache& valueCache) :
		IntegralTermBase(meshIntegralIn, parentCache, valueCache),
		values(valueCache.values)
	{
		for (int i = 0; i < componentsCount; i++)
			values[i] = 0;
		valueCache.derivatives_valid = 0;
	}

	inline bool operator()(FE_value *xi, FE_value weight)
	{
		FE_value dLAV;
		FE_value *integrandValues = baseProcess(xi, dLAV);
		if (integrandValues)
		{
			const FE_value weight_dLAV = weight*dLAV;
			for (int i = 0; i < this->componentsCount; ++i)
				this->values[i] += (integrandValues[i]*integrandValues[i])*weight_dLAV;
			return true;
		}
		return false;
	}

	static inline bool invoke(void *termVoid, FE_value *xi, FE_value weight)
	{
		return (*(reinterpret_cast<IntegralTermSumSquares*>(termVoid)))(xi, weight);
	}
};

int Computed_field_mesh_integral_squares::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	IntegralTermSumSquares sumSquares(*this, cache, RealFieldValueCache::cast(inValueCache));
	return this->evaluateTerms(sumSquares);
}

} // namespace

cmzn_field_id cmzn_fieldmodule_create_field_mesh_integral(
	cmzn_fieldmodule_id field_module, cmzn_field_id integrand_field,
	cmzn_field_id coordinate_field, cmzn_mesh_id mesh)
{
	cmzn_field_id field = 0;
	if (integrand_field && integrand_field->isNumerical() &&
		coordinate_field && coordinate_field->isNumerical() && mesh)
	{
		int numCoordinates = Computed_field_get_number_of_components(coordinate_field);
		int meshDimension = cmzn_mesh_get_dimension(mesh);
		if ((numCoordinates >= meshDimension) && (numCoordinates <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
		{
			cmzn_field_id source_fields[2] = { integrand_field, coordinate_field };
			field = Computed_field_create_generic(field_module,
				/*check_source_field_regions*/true,
				integrand_field->number_of_components,
				/*number_of_source_fields*/2, source_fields,
				/*number_of_source_values*/0, NULL,
				new Computed_field_mesh_integral(mesh));
		}
	}
	return field;
}

cmzn_field_mesh_integral_id cmzn_field_cast_mesh_integral(cmzn_field_id field)
{
	if (field && (dynamic_cast<Computed_field_mesh_integral*>(field->core)))
	{
		cmzn_field_access(field);
		return (reinterpret_cast<cmzn_field_mesh_integral_id>(field));
	}
	return 0;
}

int cmzn_field_mesh_integral_destroy(
	cmzn_field_mesh_integral_id *mesh_integral_field_address)
{
	return cmzn_field_destroy(reinterpret_cast<cmzn_field_id *>(mesh_integral_field_address));
}

inline Computed_field_mesh_integral *Computed_field_mesh_integral_core_cast(
	cmzn_field_mesh_integral *mesh_integral_field)
{
	return (static_cast<Computed_field_mesh_integral*>(
		reinterpret_cast<Computed_field*>(mesh_integral_field)->core));
}

int cmzn_field_mesh_integral_get_numbers_of_points(
	cmzn_field_mesh_integral_id mesh_integral_field,
	int values_count, int *values)
{
	if (mesh_integral_field)
	{
		Computed_field_mesh_integral *mesh_integral_core = Computed_field_mesh_integral_core_cast(mesh_integral_field);
		return mesh_integral_core->getNumbersOfPoints(values_count, values);
	}
	return 0;
}

int cmzn_field_mesh_integral_set_numbers_of_points(
	cmzn_field_mesh_integral_id mesh_integral_field,
	int values_count, const int *numbers_of_points)
{
	if (mesh_integral_field)
	{
		Computed_field_mesh_integral *mesh_integral_core = Computed_field_mesh_integral_core_cast(mesh_integral_field);
		return mesh_integral_core->setNumbersOfPoints(values_count, numbers_of_points);
	}
	return CMZN_ERROR_ARGUMENT;
}

enum cmzn_element_quadrature_rule cmzn_field_mesh_integral_get_element_quadrature_rule(
	cmzn_field_mesh_integral_id mesh_integral_field)
{
	if (mesh_integral_field)
	{
		Computed_field_mesh_integral *mesh_integral_core = Computed_field_mesh_integral_core_cast(mesh_integral_field);
		return mesh_integral_core->getElementQuadratureRule();
	}
	return CMZN_ELEMENT_QUADRATURE_RULE_INVALID;
}

int cmzn_field_mesh_integral_set_element_quadrature_rule(
	cmzn_field_mesh_integral_id mesh_integral_field,
	enum cmzn_element_quadrature_rule quadrature_rule)
{
	if (mesh_integral_field)
	{
		Computed_field_mesh_integral *mesh_integral_core = Computed_field_mesh_integral_core_cast(mesh_integral_field);
		return mesh_integral_core->setElementQuadratureRule(quadrature_rule);
	}
	return CMZN_ERROR_ARGUMENT;
}

cmzn_field_id cmzn_fieldmodule_create_field_mesh_integral_squares(
	cmzn_fieldmodule_id field_module, cmzn_field_id integrand_field,
	cmzn_field_id coordinate_field, cmzn_mesh_id mesh)
{
	cmzn_field_id field = 0;
	if (integrand_field && integrand_field->isNumerical() &&
		coordinate_field && coordinate_field->isNumerical() && mesh)
	{
		int numCoordinates = Computed_field_get_number_of_components(coordinate_field);
		int meshDimension = cmzn_mesh_get_dimension(mesh);
		if ((numCoordinates >= meshDimension) && (numCoordinates <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
		{
			cmzn_field_id source_fields[2] = { integrand_field, coordinate_field };
			field = Computed_field_create_generic(field_module,
				/*check_source_field_regions*/true,
				integrand_field->number_of_components,
				/*number_of_source_fields*/2, source_fields,
				/*number_of_source_values*/0, NULL,
				new Computed_field_mesh_integral_squares(mesh));
		}
	}
	return field;
}
