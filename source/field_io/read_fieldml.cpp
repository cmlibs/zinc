/***************************************************************************//**
 * FILE : read_fieldml.cpp
 * 
 * Functions for importing regions and fields from FieldML 0.2+ documents.
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
 * Portions created by the Initial Developer are Copyright (C) 2010
 * the Initial Developer. All Rights Reserved.
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

#include <cstring>
#include <map>
#include <set>
#include <string>
#include <vector>
extern "C" {
#include "api/cmiss_element.h"
#include "api/cmiss_field.h"
#include "api/cmiss_field_ensemble.h"
#include "api/cmiss_field_finite_element.h"
#include "api/cmiss_field_parameters.h"
#include "api/cmiss_node.h"
#include "api/cmiss_region.h"
#include "field_io/read_fieldml.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "fieldml_api.h"
}

namespace {

const FmlObjectHandle FML_INVALID_OBJECT_HANDLE = (const FmlObjectHandle)FML_INVALID_HANDLE;

struct ShapeType
{
	const char *fieldmlName;
	const int dimension;
	enum Cmiss_element_shape_type shape_type;
};

const ShapeType libraryShapes[] =
{
	{ "library.shape.line",        1, CMISS_ELEMENT_SHAPE_LINE },
	{ "library.shape.square",      2, CMISS_ELEMENT_SHAPE_SQUARE },
	{ "library.shape.triangle",    2, CMISS_ELEMENT_SHAPE_TRIANGLE },
	{ "library.shape.cube",        3, CMISS_ELEMENT_SHAPE_CUBE },
	{ "library.shape.tetrahedron", 3, CMISS_ELEMENT_SHAPE_TETRAHEDRON },
	{ "library.shape.wedge12",     3, CMISS_ELEMENT_SHAPE_WEDGE12 },
	{ "library.shape.wedge13",     3, CMISS_ELEMENT_SHAPE_WEDGE13 },
	{ "library.shape.wedge23",     3, CMISS_ELEMENT_SHAPE_WEDGE23 }
};
const int numLibraryShapes = sizeof(libraryShapes) / sizeof(ShapeType);

const char *libraryXiVariableNames[] =
{
	0,
	"library.xi.1d.variable",
	"library.xi.2d.variable",
	"library.xi.3d.variable"
};

struct BasisType
{
	int dimension;
	const char *fieldmlBasisEvaluatorName;
	const char *fieldmlParametersVariableName;
	const char *fieldmlLocalPointsTypeName;
	bool homogeneous;
	enum Cmiss_basis_function_type functionType[3];
};

const BasisType libraryBases[] =
{
	{ 2, "library.fem.bilinear_lagrange",     "library.parameters.bilinear_lagrange.variable",     "library.local_nodes.square.2x2", true, { CMISS_BASIS_FUNCTION_LINEAR_LAGRANGE,    CMISS_BASIS_FUNCTION_LINEAR_LAGRANGE,    CMISS_BASIS_FUNCTION_TYPE_INVALID } },
	{ 2, "library.fem.biquadratic_lagrange",  "library.parameters.biquadratic_lagrange.variable",  "library.local_nodes.square.3x3", true, { CMISS_BASIS_FUNCTION_QUADRATIC_LAGRANGE, CMISS_BASIS_FUNCTION_QUADRATIC_LAGRANGE, CMISS_BASIS_FUNCTION_TYPE_INVALID } },
	{ 3, "library.fem.trilinear_lagrange",    "library.parameters.trilinear_lagrange.variable",    "library.local_nodes.cube.2x2x2", true, { CMISS_BASIS_FUNCTION_LINEAR_LAGRANGE,    CMISS_BASIS_FUNCTION_LINEAR_LAGRANGE,    CMISS_BASIS_FUNCTION_LINEAR_LAGRANGE } },
	{ 3, "library.fem.triquadratic_lagrange", "library.parameters.triquadratic_lagrange.variable", "library.local_nodes.cube.3x3x3", true, { CMISS_BASIS_FUNCTION_QUADRATIC_LAGRANGE, CMISS_BASIS_FUNCTION_QUADRATIC_LAGRANGE, CMISS_BASIS_FUNCTION_QUADRATIC_LAGRANGE } },
};
const int numLibraryBases = sizeof(libraryBases) / sizeof(BasisType);

struct ConnectivityData
{
	int firstLocalNode;
	FmlObjectHandle fmlConnectivityDomain;
	FmlObjectHandle fmlConnectivitySource;
	Cmiss_field_ensemble_id localNodeEnsemble;
};

struct ElementFieldComponent
{
	Cmiss_element_basis_id element_basis;
	Cmiss_field_integer_parameters_id local_point_to_node;
	Cmiss_ensemble_index_id index;
	int local_point_count;
	int *local_point_indexes;
	int *node_identifiers;

	ElementFieldComponent(Cmiss_element_basis_id element_basis,
			Cmiss_field_integer_parameters_id local_point_to_node,
			Cmiss_ensemble_index_id index, int local_point_count) :
		element_basis(element_basis),
		local_point_to_node(local_point_to_node),
		index(index),
		local_point_count(local_point_count),
		local_point_indexes(new int[local_point_count]),
		node_identifiers(new int[local_point_count])
	{
	}

	~ElementFieldComponent()
	{
		Cmiss_element_basis_destroy(&element_basis);
		Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id*>(&local_point_to_node));
		Cmiss_ensemble_index_destroy(&index);
		delete[] local_point_indexes;
		delete[] node_identifiers;
	}
};

typedef std::map<FmlObjectHandle,ElementFieldComponent*> EvaluatorElementFieldComponentMap;

class FieldMLReader
{
	Cmiss_region *region;
	Cmiss_field_module_id field_module;
	const char *filename;
	FmlHandle fmlHandle;
	int meshDimension;
	FmlObjectHandle fmlNodesType;
	FmlObjectHandle fmlElementsType;
	EvaluatorElementFieldComponentMap componentMap;
	bool verbose;
	int nameBufferLength;
	char *nameBuffer; // buffer for reading object names into
	std::set<FmlObjectHandle> processedObjects;

public:
	FieldMLReader(struct Cmiss_region *region, const char *filename) :
		region(Cmiss_region_access(region)),
		field_module(Cmiss_region_get_field_module(region)),
		filename(filename),
		fmlHandle(Fieldml_CreateFromFile(filename)),
		meshDimension(0),
		fmlNodesType(FML_INVALID_OBJECT_HANDLE),
		fmlElementsType(FML_INVALID_OBJECT_HANDLE),
		verbose(false),
		nameBufferLength(50),
		nameBuffer(new char[nameBufferLength])
	{
		Fieldml_SetDebug(fmlHandle, /*debug*/0);
	}
	
	~FieldMLReader()
	{
		for (EvaluatorElementFieldComponentMap::iterator iter = componentMap.begin(); iter != componentMap.end(); iter++)
		{
			delete (iter->second);
		}
		Fieldml_Destroy(fmlHandle);
		Cmiss_field_module_destroy(&field_module);
		Cmiss_region_destroy(&region);
		delete[] nameBuffer;
	}

	/** @return  1 on success, 0 on failure */
	int parse();

private:

	std::string getObjectName(FmlObjectHandle fmlObjectHandle)
	{
		nameBuffer[0] = 0;
		while (true)
		{
			int length = Fieldml_CopyObjectName(fmlHandle, fmlObjectHandle, nameBuffer, nameBufferLength);
			if (length < nameBufferLength - 1)
				break;
			nameBufferLength *= 2;
			delete[] nameBuffer;
			nameBuffer = new char[nameBufferLength];
		}
		return std::string(nameBuffer);
	}

	Cmiss_field_ensemble_id getEnsemble(FmlObjectHandle fmlEnsemble);

	int readSemiDenseParameters(FmlObjectHandle fmlParameters,
		Cmiss_field_id parameters, const char *name);

	Cmiss_field_id getParameters(FmlObjectHandle fmlParameters);

	enum Cmiss_element_shape_type getElementShapeFromName(const char *shapeName);

	int readMeshes();

	ElementFieldComponent *getElementFieldComponent(Cmiss_fe_mesh_id mesh,
		FmlObjectHandle fmlEvaluator, FmlObjectHandle fmlNodeParametersVariable,
		FmlObjectHandle fmlNodeVariable, FmlObjectHandle fmlElementVariable);

	int readField(FmlObjectHandle fmlFieldEvaluator,
		std::vector<FmlObjectHandle> &fmlComponentEvaluators,
		FmlObjectHandle fmlNodeEnsembleType, FmlObjectHandle fmlNodeParameters,
		FmlObjectHandle fmlNodeParametersVariable, FmlObjectHandle fmlNodeVariable,
		FmlObjectHandle fmlElementVariable);

	int readAggregateFields();

	int readReferenceFields();

	bool isProcessed(FmlObjectHandle fmlObjectHandle)
	{
		return (processedObjects.find(fmlObjectHandle) != processedObjects.end());
	}

	void setProcessed(FmlObjectHandle fmlObjectHandle)
	{
		processedObjects.insert(fmlObjectHandle);
	}

};

/***************************************************************************//**
 * Gets ensemble field/domain matching fmlEnsembleType. Definition is read from
 * FieldML only when first requested.
 *
 * @param fmlEnsembleType  Handle of type FHT_ENSEMBLE_TYPE.
 * @return  Accessed handle to ensemble field, or 0 on failure */
Cmiss_field_ensemble_id FieldMLReader::getEnsemble(FmlObjectHandle fmlEnsembleType)
{
	std::string name = getObjectName(fmlEnsembleType);
	if (Fieldml_GetObjectType(fmlHandle, fmlEnsembleType) != FHT_ENSEMBLE_TYPE)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Argument %s is not ensemble type", name.c_str());
		return 0;
	}
	Cmiss_field_id ensemble_field = Cmiss_field_module_find_field_by_name(field_module, name.c_str());
	if (isProcessed(fmlEnsembleType))
	{
		Cmiss_field_ensemble_id ensemble = Cmiss_field_cast_ensemble(ensemble_field);
		Cmiss_field_destroy(&ensemble_field);
		return ensemble;
	}
	if (ensemble_field)
	{
		// current code is not able to merge into existing ensemble because parameters
		// may be densely indexed by ensemble as it is in FieldML document
		display_message(ERROR_MESSAGE, "Read FieldML:  Cannot merge ensemble type %s", name.c_str());
		Cmiss_field_destroy(&ensemble_field);
		return 0;
	}

	TypeBoundsType fmlBounds = Fieldml_GetBoundsType(fmlHandle, fmlEnsembleType);
	if (fmlBounds != BOUNDS_DISCRETE_CONTIGUOUS)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Unsupported bounds type %d for ensemble type %s", fmlBounds, name.c_str());
		return 0;
	}

	if (verbose)
	{
		display_message(INFORMATION_MESSAGE, "Reading ensemble type %s\n", name.c_str());
	}

	ensemble_field = Cmiss_field_module_create_ensemble(field_module);
	Cmiss_field_set_name(ensemble_field, name.c_str());
	Cmiss_field_set_attribute_integer(ensemble_field, CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 1);
	Cmiss_field_ensemble_id ensemble = Cmiss_field_cast_ensemble(ensemble_field);
	Cmiss_field_destroy(&ensemble_field);
	if (!ensemble)
	{
		return 0;
	}

	// GRC Note contiguous bounds 1..Count only!
	// GRC There should be convenience functions for adding identifier ranges to ensemble
	unsigned int entryCount = Fieldml_GetEnsembleTypeElementCount(fmlHandle, fmlEnsembleType);
	for (Cmiss_ensemble_identifier identifier = 1; identifier <= entryCount; identifier++)
	{
		Cmiss_ensemble_iterator_id iterator =
			Cmiss_field_ensemble_find_or_create_entry(ensemble, identifier);
		if (!iterator)
		{
			Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id*>(&ensemble));
			break;
		}
		Cmiss_ensemble_iterator_destroy(&iterator);
	}
	if (ensemble)
	{
		setProcessed(fmlEnsembleType);
	}
	return ensemble;
}

int FieldMLReader::readSemiDenseParameters(FmlObjectHandle fmlParameters,
	Cmiss_field_id parameters, const char *name)
{
	DataDescriptionType dataDescription = Fieldml_GetParameterDataDescription(fmlHandle, fmlParameters);
	if (dataDescription != DESCRIPTION_SEMIDENSE)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Unknown data description type %d for parameters %s", dataDescription, name);
		return 0;
	}
	Cmiss_field_real_parameters_id realParameters = Cmiss_field_cast_real_parameters(parameters);
	Cmiss_field_integer_parameters_id integerParameters = Cmiss_field_cast_integer_parameters(parameters);
	if ((!realParameters) && (!integerParameters))
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Invalid parameters field type", dataDescription, name);
		return 0;
	}
	int return_code = 1;

	const int sparseIndexCount = Fieldml_GetSemidenseIndexCount(fmlHandle, fmlParameters, /*isSparse*/1);
	const int denseIndexCount = Fieldml_GetSemidenseIndexCount(fmlHandle, fmlParameters, /*isSparse*/0);

	// FieldML reader reads all values indexed by innermost dense ensemble if it is marked as 'ComponentType'
	// Other dense indexes must be iterated over
	int valueBufferSize = 1;
	int firstDenseIteratorIndex = 0;
	if (0 < denseIndexCount)
	{
		FmlObjectHandle fmlInnerDenseIndexEvaluator = Fieldml_GetSemidenseIndexEvaluator(fmlHandle, fmlParameters, 1, /*isSparse*/0);
		FmlObjectHandle fmlInnerDenseIndexType = Fieldml_GetValueType(fmlHandle, fmlInnerDenseIndexEvaluator);
		if (Fieldml_IsEnsembleComponentType(fmlHandle, fmlInnerDenseIndexType))
		{
			firstDenseIteratorIndex = 1;
			valueBufferSize = Fieldml_GetEnsembleTypeElementCount(fmlHandle, fmlInnerDenseIndexType);
		}
	}
	if (valueBufferSize == 0)
	{
		return_code = 0;
	}

	FmlReaderHandle fmlReader = Fieldml_OpenReader(fmlHandle, fmlParameters);
	if (fmlReader == FML_INVALID_HANDLE)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Could not open reader for parameters %s", name);
		return_code = 0;
	}

	Cmiss_field_ensemble_id *sparseIndexEnsembles = new Cmiss_field_ensemble_id[sparseIndexCount];
	for (int i = 0; i < sparseIndexCount; i++)
	{
		FmlObjectHandle fmlSparseIndexEvaluator = Fieldml_GetSemidenseIndexEvaluator(fmlHandle, fmlParameters, i + 1, /*isSparse*/1);
		FmlObjectHandle fmlSparseIndexType = Fieldml_GetValueType(fmlHandle, fmlSparseIndexEvaluator);
		sparseIndexEnsembles[i] = getEnsemble(fmlSparseIndexType);
	}

	Cmiss_field_ensemble_id *denseIndexEnsembles = new Cmiss_field_ensemble_id[denseIndexCount];
	Cmiss_ensemble_iterator_id *denseIterators = new Cmiss_ensemble_iterator_id[denseIndexCount];
	for (int i = 0; i < denseIndexCount; i++)
	{
		FmlObjectHandle fmlDenseIndexEvaluator = Fieldml_GetSemidenseIndexEvaluator(fmlHandle, fmlParameters, i + 1, /*isSparse*/0);
		FmlObjectHandle fmlDenseIndexType = Fieldml_GetValueType(fmlHandle, fmlDenseIndexEvaluator);
		denseIndexEnsembles[i] = getEnsemble(fmlDenseIndexType);
		denseIterators[i] = Cmiss_field_ensemble_get_first_entry(denseIndexEnsembles[i]);
	}

	Cmiss_ensemble_index_id index = realParameters ?
		Cmiss_field_real_parameters_create_index(realParameters) :
		Cmiss_field_integer_parameters_create_index(integerParameters);

	int *indexBuffer = new int[sparseIndexCount];
	double *realValueBuffer = new double[valueBufferSize];
	int *integerValueBuffer = new int[valueBufferSize];

	while (return_code)
	{
		// read the next set of sparse indexes
		int result = Fieldml_ReadIndexSet(fmlHandle, fmlReader, indexBuffer);
		if (result != FML_ERR_NO_ERROR)
		{
			if (result != FML_ERR_IO_NO_DATA)
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  Error %d when reading sparse indexes for parameters %s", result, name);
				return_code = 0;
			}
			break;
		}
		for (int i = 0; i < sparseIndexCount; i++)
		{
			// GRC should make convenience function for these three calls
			Cmiss_ensemble_iterator_id entry =
				Cmiss_field_ensemble_find_entry_by_identifier(sparseIndexEnsembles[i], indexBuffer[i]);
			Cmiss_ensemble_index_set_entry(index, entry);
			Cmiss_ensemble_iterator_destroy(&entry);
		}
		bool moreDenseData = true;
		while (moreDenseData)
		{
			int valuesRead = realParameters ?
				Fieldml_ReadDoubleValues(fmlHandle, fmlReader, realValueBuffer, valueBufferSize) :
				Fieldml_ReadIntValues(fmlHandle, fmlReader, integerValueBuffer, valueBufferSize);
			if (valuesRead != valueBufferSize)
			{
				display_message(WARNING_MESSAGE, "Read FieldML:  Values read %d, required %d",
					valuesRead, valueBufferSize);
				return_code = 0;
				break;
			}
			moreDenseData = false;
			for (int i = firstDenseIteratorIndex; i < denseIndexCount; i++)
			{
				Cmiss_ensemble_index_set_entry(index, denseIterators[i]);
				if (Cmiss_ensemble_iterator_increment(denseIterators[i]))
				{
					moreDenseData = true;
					break;
				}
				else
				{
					Cmiss_ensemble_iterator_destroy(&(denseIterators[i]));
					denseIterators[i] = Cmiss_field_ensemble_get_first_entry(denseIndexEnsembles[i]);
				}
			}
			return_code = realParameters ?
				Cmiss_field_real_parameters_set_values(realParameters, index, valueBufferSize, realValueBuffer) :
				Cmiss_field_integer_parameters_set_values(integerParameters, index, valueBufferSize, integerValueBuffer);
			if (!return_code)
			{
				display_message(WARNING_MESSAGE, "Read FieldML:  Failed to set values in parameters field");
				break;
			}
		}
	}

	for (int i = 0; i < sparseIndexCount; i++)
	{
		Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id*>(&(sparseIndexEnsembles[i])));
	}
	delete[] sparseIndexEnsembles;

	for (int i = 0; i < denseIndexCount; i++)
	{
		Cmiss_ensemble_iterator_destroy(&(denseIterators[i]));
		Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id*>(&(denseIndexEnsembles[i])));
	}
	delete[] denseIterators;
	delete[] denseIndexEnsembles;

	Cmiss_ensemble_index_destroy(&index);

	delete[] realValueBuffer;
	delete[] integerValueBuffer;
	delete[] indexBuffer;

	Fieldml_CloseReader(fmlHandle, fmlReader);

	Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id*>(&realParameters));
	Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id*>(&integerParameters));

	return return_code;
}

/***************************************************************************//**
 * Gets Cmiss_real_parameters or Cmiss_integer_parameters (for ensemble) with
 * data read from fmlParameters object supplied. Data is read from FieldML only
 * when first requested.
 *
 * @param fmlParameters  Handle of type FHT_PARAMETER_EVALUATOR.
 * @return  Accessed handle to parameters field, or 0 on failure */
Cmiss_field_id FieldMLReader::getParameters(FmlObjectHandle fmlParameters)
{
	if (Fieldml_GetObjectType(fmlHandle, fmlParameters) != FHT_PARAMETER_EVALUATOR)
	{
		display_message(ERROR_MESSAGE, "FieldMLReader::getParameters.  Invalid argument");
		return 0;
	}
	std::string name = getObjectName(fmlParameters);

	FmlObjectHandle fmlValueType = Fieldml_GetValueType(fmlHandle, fmlParameters);
	FieldmlHandleType value_class = Fieldml_GetObjectType(fmlHandle, fmlValueType);
	if ((value_class != FHT_CONTINUOUS_TYPE) && ((value_class != FHT_ENSEMBLE_TYPE)))
	{
		display_message(ERROR_MESSAGE, "Read FieldML.  Cannot read parameters %s since not continuous or ensemble type", name.c_str());
		return 0;
	}
	bool isReal = (value_class == FHT_CONTINUOUS_TYPE);
	FmlObjectHandle fmlValueTypeComponentEnsembleType = Fieldml_GetTypeComponentEnsemble(fmlHandle, fmlValueType);
	if (fmlValueTypeComponentEnsembleType != FML_INVALID_OBJECT_HANDLE)
	{
		display_message(WARNING_MESSAGE, "Read FieldML:  Cannot read non-scalar parameters %s", name.c_str());
		return 0;
	}

	Cmiss_field_id parameters = Cmiss_field_module_find_field_by_name(field_module, name.c_str());
	if (isProcessed(fmlParameters))
	{
		return parameters;
	}
	if (parameters)
	{
		bool invalidType = false;
		if (isReal)
		{
			Cmiss_field_real_parameters_id realParameters = Cmiss_field_cast_real_parameters(parameters);
			invalidType = (realParameters == 0);
			Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id*>(&realParameters));
		}
		else
		{
			Cmiss_field_integer_parameters_id integerParameters = Cmiss_field_cast_integer_parameters(parameters);
			invalidType = (integerParameters == 0);
			Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id*>(&integerParameters));
		}
		if (invalidType)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Cannot merge real parameters into existing field %s of different type", name.c_str());
			Cmiss_field_destroy(&parameters);
			return 0;
		}
		// should check matching indexes too
	}

	if (verbose)
	{
		display_message(INFORMATION_MESSAGE, "Reading %s parameters %s\n", (isReal ? "real" : "integer"), name.c_str());
	}

	int indexEnsembleCount = Fieldml_GetIndexCount(fmlHandle, fmlParameters);
	Cmiss_field_ensemble_id *indexEnsembles = new Cmiss_field_ensemble_id[indexEnsembleCount];
	for (int i = 0; i < indexEnsembleCount; i++)
	{
		indexEnsembles[i] = 0;
	}

	int return_code = 1;
	for (int indexEnsembleIndex = 1; indexEnsembleIndex <= indexEnsembleCount; indexEnsembleIndex++)
	{
		FmlObjectHandle fmlIndexEvaluator = Fieldml_GetIndexEvaluator(fmlHandle, fmlParameters, indexEnsembleIndex);
		if (((FHT_ABSTRACT_EVALUATOR != Fieldml_GetObjectType(fmlHandle, fmlIndexEvaluator)) &&
				(FHT_EXTERNAL_EVALUATOR != Fieldml_GetObjectType(fmlHandle, fmlIndexEvaluator))) || // GRC workaround; FHT_EXTERNAL_EVALUATOR is wrong
			(FHT_ENSEMBLE_TYPE != Fieldml_GetObjectType(fmlHandle, Fieldml_GetValueType(fmlHandle, fmlIndexEvaluator))))
		{
			display_message(WARNING_MESSAGE, "Read FieldML:  Index %d (%s) of parameters %s is not an ensemble-valued abstract evaluator",
				indexEnsembleIndex, getObjectName(fmlIndexEvaluator).c_str(), name.c_str());
			return_code = 0;
		}
		indexEnsembles[indexEnsembleIndex - 1] = getEnsemble(Fieldml_GetValueType(fmlHandle, fmlIndexEvaluator));
		if (verbose)
		{
			display_message(INFORMATION_MESSAGE, "  Index ensemble %d = %s (%d %p)\n", indexEnsembleIndex,
				getObjectName(fmlIndexEvaluator).c_str(), fmlIndexEvaluator, indexEnsembles[indexEnsembleIndex - 1]);
		}
	}

	if (return_code)
	{
		if (!parameters)
		{
			parameters = isReal ?
				Cmiss_field_module_create_real_parameters(field_module, indexEnsembleCount, indexEnsembles) :
				Cmiss_field_module_create_integer_parameters(field_module, indexEnsembleCount, indexEnsembles);
			Cmiss_field_set_name(parameters, name.c_str());
			Cmiss_field_set_attribute_integer(parameters, CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 1);
		}
		return_code = readSemiDenseParameters(fmlParameters, parameters, name.c_str());
	}
	for (int i = 0; i < indexEnsembleCount; i++)
	{
		Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id*>(&indexEnsembles[i]));
	}
	delete[] indexEnsembles;
	if (return_code)
	{
		setProcessed(fmlParameters);
	}
	else
	{
		Cmiss_field_destroy(&parameters);
	}
	return parameters;
}

enum Cmiss_element_shape_type FieldMLReader::getElementShapeFromName(const char *shapeName)
{
	for (int i = 0; i < numLibraryShapes; i++)
	{
		if (0 == strcmp(shapeName, libraryShapes[i].fieldmlName))
		{
			return libraryShapes[i].shape_type;
		}
	}
	display_message(ERROR_MESSAGE, "Read FieldML:  Unrecognised shape %s", shapeName);
	return CMISS_ELEMENT_SHAPE_TYPE_INVALID;
}

int FieldMLReader::readMeshes()
{
	const int meshCount = Fieldml_GetObjectCount(fmlHandle, FHT_MESH_TYPE);
	if (meshCount != 1)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Require 1 mesh type, %d found", meshCount);
		return 0;
	}
	int return_code = 1;
	for (int meshIndex = 1; (meshIndex <= meshCount) && return_code; meshIndex++)
	{
		FmlObjectHandle fmlMeshType = Fieldml_GetObject(fmlHandle, FHT_MESH_TYPE, meshIndex);
		std::string name = getObjectName(fmlMeshType);

		FmlObjectHandle fmlMeshXiType = Fieldml_GetMeshXiType(fmlHandle, fmlMeshType);
		FmlObjectHandle fmlMeshXiComponentType = Fieldml_GetTypeComponentEnsemble(fmlHandle, fmlMeshXiType);
		meshDimension = Fieldml_GetEnsembleTypeElementCount(fmlHandle, fmlMeshXiComponentType);
		if ((meshDimension < 1) || (meshDimension > 3))
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Invalid dimension %d for mesh type %s", meshDimension, name.c_str());
			return_code = 0;
			break;
		}

		if (verbose)
		{
			display_message(INFORMATION_MESSAGE, "Reading mesh %s dimension %d\n", name.c_str(), meshDimension);
		}

		// define elements and shapes

		fmlElementsType = Fieldml_GetMeshElementType(fmlHandle, fmlMeshType);
		if (verbose)
		{
			display_message(INFORMATION_MESSAGE, "Reading elements from %s\n",
				getObjectName(fmlElementsType).c_str());
		}
		Cmiss_field_ensemble_id elementsEnsemble = getEnsemble(fmlElementsType);

		Cmiss_fe_mesh_id mesh = Cmiss_region_get_fe_mesh_by_name(region,
			(meshDimension == 3 ? "cmiss_mesh_3d" :
				(meshDimension == 2 ? "cmiss_mesh_2d" : "cmiss_mesh_1d")));
		Cmiss_element_template_id element_template = Cmiss_fe_mesh_create_element_template(mesh);

		// make FE_elements out of elements ensemble with shape from mesh
		Cmiss_ensemble_iterator *elementsIterator = Cmiss_field_ensemble_get_first_entry(elementsEnsemble);
		if (!elementsIterator)
		{
			return_code = 0;
		}
		int shapeNameLength = 50;
		char *shapeName = new char[shapeNameLength];
		enum Cmiss_element_shape_type last_shape_type = CMISS_ELEMENT_SHAPE_TYPE_INVALID;
		while (return_code)
		{
			Cmiss_ensemble_identifier elementIdentifier = Cmiss_ensemble_iterator_get_identifier(elementsIterator);
			int elementNumber = static_cast<int>(elementIdentifier);
			while (true)
			{
				int length = Fieldml_CopyMeshElementShape(fmlHandle, fmlMeshType, elementNumber, /*allowDefault*/1, shapeName, shapeNameLength);
				if (length < shapeNameLength)
					break;
				shapeNameLength *= 2;
				delete[] shapeName;
				shapeName = new char[shapeNameLength];
			}
			enum Cmiss_element_shape_type shape_type = getElementShapeFromName(shapeName);
			if (shape_type == CMISS_ELEMENT_SHAPE_TYPE_INVALID)
			{
				return_code = 0;
				break;
			}
			if (shape_type != last_shape_type)
			{
				if (!(Cmiss_element_template_set_shape_type(element_template, shape_type) &&
					(Cmiss_element_template_finalise(element_template))))
				{
					return_code = 0;
					break;
				}
				last_shape_type = shape_type;
			}
			if (!Cmiss_fe_mesh_define_element(mesh, elementNumber, element_template))
			{
				return_code = 0;
				break;
			}
			if (!Cmiss_ensemble_iterator_increment(elementsIterator))
				break;
		}
		delete[] shapeName;
		Cmiss_ensemble_iterator_destroy(&elementsIterator);
		Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id*>(&elementsEnsemble));

		Cmiss_element_template_destroy(&element_template);
		Cmiss_fe_mesh_destroy(&mesh);
	}
	return return_code;
}

ElementFieldComponent *FieldMLReader::getElementFieldComponent(Cmiss_fe_mesh_id mesh,
	FmlObjectHandle fmlEvaluator, FmlObjectHandle fmlNodeParametersVariable,
	FmlObjectHandle fmlNodeVariable, FmlObjectHandle fmlElementVariable)
{
	EvaluatorElementFieldComponentMap::iterator iter = componentMap.find(fmlEvaluator);
	if (iter != componentMap.end())
	{
		return iter->second;
	}

	std::string evaluatorName = getObjectName(fmlEvaluator);
	if ((FHT_REFERENCE_EVALUATOR != Fieldml_GetObjectType(fmlHandle, fmlEvaluator)) ||
		(FHT_CONTINUOUS_TYPE != Fieldml_GetObjectType(fmlHandle, Fieldml_GetValueType(fmlHandle, fmlEvaluator))))
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  getElementFieldComponent argument %s is not a real-valued reference evaluator.",
			evaluatorName.c_str());
		return 0;
	}

	FmlObjectHandle fmlBasisEvaluator = Fieldml_GetReferenceRemoteEvaluator(fmlHandle, fmlEvaluator);
	std::string basisName = getObjectName(fmlBasisEvaluator);
	const char *basis_name = basisName.c_str();
	FmlObjectHandle fmlRemoteXiVariable = Fieldml_GetObjectByName(fmlHandle, libraryXiVariableNames[meshDimension]);
	FmlObjectHandle fmlRemoteParametersVariable = FML_INVALID_OBJECT_HANDLE;
	FmlObjectHandle fmlLocalPointsType = FML_INVALID_OBJECT_HANDLE;
	int basis_index = -1;
	for (int i = 0; i < numLibraryBases; i++)
	{
		if ((libraryBases[i].dimension == meshDimension) &&
			(0 == strcmp(basis_name, libraryBases[i].fieldmlBasisEvaluatorName)))
		{
			basis_index = i;
			fmlRemoteParametersVariable = Fieldml_GetObjectByName(fmlHandle, libraryBases[i].fieldmlParametersVariableName);
			fmlLocalPointsType = Fieldml_GetObjectByName(fmlHandle, libraryBases[i].fieldmlLocalPointsTypeName);
			break;
		}
	}
	if (basis_index < 0)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Reference evaluator %s references unsupported basis %s for mesh dimension %d.",
			evaluatorName.c_str(), basis_name, meshDimension);
		return 0;
	}
	int return_code = 1;
	if (fmlRemoteXiVariable == FML_INVALID_OBJECT_HANDLE)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Missing remote xi variable %s for basis %s.",
			libraryXiVariableNames[meshDimension], basis_name);
		return_code = 0;
	}
	if (fmlRemoteParametersVariable == FML_INVALID_OBJECT_HANDLE)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Missing remote parameters variable %s for basis %s.",
			libraryBases[basis_index].fieldmlParametersVariableName, basis_name);
		return_code = 0;
	}
	if (fmlLocalPointsType == FML_INVALID_OBJECT_HANDLE)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Missing point layout type %s expected for basis %s.",
			libraryBases[basis_index].fieldmlLocalPointsTypeName, basis_name);
		return_code = 0;
	}
	FmlObjectHandle fmlLocalXiEvaluator = Fieldml_GetBindByVariable(fmlHandle, fmlEvaluator, fmlRemoteXiVariable);
	FmlObjectHandle fmlElementParametersEvaluator = Fieldml_GetBindByVariable(fmlHandle, fmlEvaluator, fmlRemoteParametersVariable);

	// exactly one mesh expected
	if (1 != Fieldml_GetObjectCount(fmlHandle, FHT_MESH_TYPE))
	{
		display_message(ERROR_MESSAGE, "FieldMLReader::getElementFieldComponent:  Only supports 1 mesh type");
		return_code = 0;
	}
	FmlObjectHandle fmlMeshType = Fieldml_GetObject(fmlHandle, FHT_MESH_TYPE, /*meshIndex*/1);
	FmlObjectHandle fmlMeshXiType = Fieldml_GetMeshXiType(fmlHandle, fmlMeshType);
	if ((fmlLocalXiEvaluator == FML_INVALID_OBJECT_HANDLE) ||
		((Fieldml_GetObjectType(fmlHandle, fmlLocalXiEvaluator) != FHT_ABSTRACT_EVALUATOR) &&
			(Fieldml_GetObjectType(fmlHandle, fmlLocalXiEvaluator) != FHT_EXTERNAL_EVALUATOR)) || // GRC workaround; FHT_EXTERNAL_EVALUATOR is wrong
		(Fieldml_GetValueType(fmlHandle, fmlLocalXiEvaluator) != fmlMeshXiType))
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Evaluator %s does not bind local mesh xi variable to generic xi variable %s.",
			evaluatorName.c_str(), libraryXiVariableNames[meshDimension]);
		return_code = 0;
	}
	if (fmlElementParametersEvaluator == FML_INVALID_OBJECT_HANDLE)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Evaluator %s does not bind to parameters variable %s for basis %s.",
			evaluatorName.c_str(), libraryBases[basis_index].fieldmlParametersVariableName, basis_name);
		return_code = 0;
	}
	int evaluatorBindCount = Fieldml_GetBindCount(fmlHandle, fmlEvaluator);
	if (2 != evaluatorBindCount)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Evaluator %s has %d bindings. Basis evaluator must bind xi and parameters.",
			evaluatorName.c_str(), evaluatorBindCount);
		return_code = 0;
	}

	if (!return_code)
		return 0;

	std::string elementParametersName = getObjectName(fmlElementParametersEvaluator);
	FmlObjectHandle fmlLocalPointVariable = Fieldml_GetIndexEvaluator(fmlHandle, fmlElementParametersEvaluator, 1);
	if ((Fieldml_GetObjectType(fmlHandle, fmlElementParametersEvaluator) != FHT_AGGREGATE_EVALUATOR) ||
		/*(1 != Fieldml_GetIndexCount(fmlHandle, fmlElementParametersEvaluator)) || */ // GRC workaround. Erroneously returns -1 so can't check
		(Fieldml_GetValueType(fmlHandle, fmlLocalPointVariable) != fmlLocalPointsType))
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Evaluator %s element parameter source %s is not an aggregate over ensemble type %s.",
			evaluatorName.c_str(), elementParametersName.c_str(), libraryBases[basis_index].fieldmlLocalPointsTypeName);
		return 0;
	}

	FmlObjectHandle fmlTempNodeParameters = Fieldml_GetDefaultEvaluator(fmlHandle, fmlElementParametersEvaluator);
	if ((fmlTempNodeParameters == FML_INVALID_OBJECT_HANDLE) ||
		(0 != Fieldml_GetEvaluatorCount(fmlHandle, fmlElementParametersEvaluator)))
	{
		display_message(ERROR_MESSAGE, "Read FieldML (Limitation):  Evaluator %s element parameter source %s must use only default component evaluator",
			evaluatorName.c_str(), elementParametersName.c_str());
		return 0;
	}

	int localParametersBindCount = Fieldml_GetBindCount(fmlHandle, fmlElementParametersEvaluator);
	if (1 != localParametersBindCount)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Evaluator %s element parameter source %s has %d bindings, expect 1 for local-to-global map.",
			evaluatorName.c_str(), elementParametersName.c_str(), localParametersBindCount);
		return 0;
	}

	FmlObjectHandle fmlTempNodeVariable = Fieldml_GetBindVariable(fmlHandle, fmlElementParametersEvaluator, 1);
	FmlObjectHandle fmlLocalPointToNode = Fieldml_GetBindEvaluator(fmlHandle, fmlElementParametersEvaluator, 1);

	if (fmlTempNodeParameters != fmlNodeParametersVariable)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Evaluator %s element parameter source %s default evaluator %s does not match nodal parameters variable %s",
			evaluatorName.c_str(), elementParametersName.c_str(), getObjectName(fmlTempNodeParameters).c_str(), getObjectName(fmlNodeParametersVariable).c_str());
		return_code = 0;
	}
	if (fmlTempNodeVariable != fmlNodeVariable)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Evaluator %s element parameter source %s should bind to node variable %s",
			evaluatorName.c_str(), elementParametersName.c_str(), getObjectName(fmlNodeVariable).c_str());
		return_code = 0;
	}
	if ((fmlLocalPointToNode == FML_INVALID_OBJECT_HANDLE) ||
		(Fieldml_GetValueType(fmlHandle, fmlLocalPointToNode) != Fieldml_GetValueType(fmlHandle, fmlNodeVariable)))
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Evaluator %s binds incompatibly-typed evaluator %s to variable %s",
			elementParametersName.c_str(), getObjectName(fmlLocalPointToNode).c_str(), getObjectName(fmlNodeVariable).c_str());
		return_code = 0;
	}
	FmlObjectHandle fmlLocalPointToNodeIndex1 = Fieldml_GetIndexEvaluator(fmlHandle, fmlLocalPointToNode, 1);
	FmlObjectHandle fmlLocalPointToNodeIndex2 = Fieldml_GetIndexEvaluator(fmlHandle, fmlLocalPointToNode, 2);
	if ((2 != Fieldml_GetIndexCount(fmlHandle, fmlLocalPointToNode)) ||
		!(((fmlLocalPointToNodeIndex1 == fmlElementVariable) && (fmlLocalPointToNodeIndex2 == fmlLocalPointVariable)) ||
			((fmlLocalPointToNodeIndex2 == fmlElementVariable) && (fmlLocalPointToNodeIndex1 == fmlLocalPointVariable))))
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Evaluator %s evaluator %s cannot be interpreted as an element, local point to node map.",
			elementParametersName.c_str(), getObjectName(fmlLocalPointToNode).c_str());
		return_code = 0;
	}

	if (!return_code)
		return 0;

	// Structure now validated

	if (verbose)
	{
		display_message(INFORMATION_MESSAGE, "Read FieldML:  Interpreting evaluator %s as nodal/element interpolator using basis %s.\n",
			evaluatorName.c_str(), basis_name);
	}

	Cmiss_field_id local_point_to_node_field = getParameters(fmlLocalPointToNode);
	Cmiss_field_integer_parameters_id local_point_to_node = Cmiss_field_cast_integer_parameters(local_point_to_node_field);
	Cmiss_field_destroy(&local_point_to_node_field);
	Cmiss_ensemble_index_id index = Cmiss_field_integer_parameters_create_index(local_point_to_node);
	int local_point_count = Fieldml_GetEnsembleTypeElementCount(fmlHandle, fmlLocalPointsType);

	Cmiss_element_basis_id element_basis = Cmiss_fe_mesh_create_element_basis(mesh, libraryBases[basis_index].functionType[0]);
	if (!libraryBases[basis_index].homogeneous)
	{
		for (int dimension = 2; dimension < meshDimension; dimension++)
		{
			Cmiss_element_basis_set_function_type(element_basis, dimension,
				libraryBases[basis_index].functionType[dimension - 1]);
		}
	}
	int basis_number_of_nodes = Cmiss_element_basis_get_number_of_nodes(element_basis);
	ElementFieldComponent *component = new ElementFieldComponent(element_basis, local_point_to_node, index, local_point_count);
	if (local_point_to_node && index && local_point_count && (local_point_count == basis_number_of_nodes))
	{
		componentMap[fmlEvaluator] = component;
	}
	else
	{
		if (local_point_count != basis_number_of_nodes)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Evaluator %s basis %s requires %d DOFs which does not match number of local points (%d)",
				evaluatorName.c_str(), basis_name, basis_number_of_nodes, local_point_count);
		}
		delete component;
		component = 0;
	}
	return component;
}

int FieldMLReader::readField(FmlObjectHandle fmlFieldEvaluator,
	std::vector<FmlObjectHandle> &fmlComponentEvaluators,
	FmlObjectHandle fmlNodeEnsembleType, FmlObjectHandle fmlNodeParameters,
	FmlObjectHandle fmlNodeParametersVariable, FmlObjectHandle fmlNodeVariable,
	FmlObjectHandle fmlElementVariable)
{
	int return_code = 1;
	const int componentCount = fmlComponentEvaluators.size();

	std::string fieldName = getObjectName(fmlFieldEvaluator);

	if (verbose)
	{
		display_message(INFORMATION_MESSAGE, "Defining field from evaluator %s, components = %d\n",
			fieldName.c_str(), componentCount);
	}

	Cmiss_field_id field = Cmiss_field_module_create_finite_element(field_module, fieldName.c_str(), componentCount);
	Cmiss_field_set_attribute_integer(field, CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 1);
	if ((componentCount >= meshDimension) && (componentCount <= 3))
	{
		// overzealous to help ensure there is at least one 'coordinate' field to define faces
		Cmiss_field_set_attribute_integer(field, CMISS_FIELD_ATTRIBUTE_IS_COORDINATE, 1);
	}
	Cmiss_field_finite_element_id fe_field = Cmiss_field_cast_finite_element(field);
	Cmiss_field_destroy(&field);

	// create nodes and set node parameters

	Cmiss_nodeset_id nodes = Cmiss_region_get_nodeset_by_name(region, "cmiss_nodes");
	Cmiss_field_ensemble_id nodesEnsemble = getEnsemble(fmlNodeEnsembleType);
	if (fmlNodesType == FML_INVALID_OBJECT_HANDLE)
	{
		fmlNodesType = fmlNodeEnsembleType;
		// create the nodes
		Cmiss_node_template_id node_template = Cmiss_nodeset_create_node_template(nodes);
		return_code = Cmiss_node_template_finalise(node_template);
		Cmiss_ensemble_iterator_id nodesIterator = Cmiss_field_ensemble_get_first_entry(nodesEnsemble);
		while (return_code)
		{
			Cmiss_ensemble_identifier nodeIdentifier = Cmiss_ensemble_iterator_get_identifier(nodesIterator);
			Cmiss_node_id node = Cmiss_nodeset_create_node(nodes, nodeIdentifier, node_template);
			Cmiss_node_destroy(&node);
			if (!Cmiss_ensemble_iterator_increment(nodesIterator))
				break;
		}
		Cmiss_ensemble_iterator_destroy(&nodesIterator);
		Cmiss_node_template_destroy(&node_template);
	}

	Cmiss_field_id node_parameters_field = getParameters(fmlNodeParameters);
	Cmiss_field_real_parameters_id node_parameters = Cmiss_field_cast_real_parameters(node_parameters_field);
	Cmiss_field_destroy(&node_parameters_field);
	if (!node_parameters)
	{
		display_message(ERROR_MESSAGE,
			"Read FieldML:  Field %s nodal parameters %s unable to be read.",
			fieldName.c_str(), getObjectName(fmlNodeParameters).c_str());
		return_code = 0;
	}
	else
	{
		Cmiss_node_template_id node_template = Cmiss_nodeset_create_node_template(nodes);
		Cmiss_node_template_define_field(node_template, fe_field);
		return_code = Cmiss_node_template_finalise(node_template);
		Cmiss_ensemble_index_id index = Cmiss_field_real_parameters_create_index(node_parameters);
		// GRC inefficient to iterate over sparse parameters this way
		Cmiss_ensemble_iterator_id nodesIterator = Cmiss_field_ensemble_get_first_entry(nodesEnsemble);
		double *values = new double[componentCount];
		int *valueExists = new int[componentCount];
		while (return_code)
		{
			Cmiss_ensemble_identifier nodeIdentifier = Cmiss_ensemble_iterator_get_identifier(nodesIterator);
			Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodes, nodeIdentifier);
			Cmiss_ensemble_index_set_entry(index, nodesIterator);
			int valuesRead = 0;
			if (Cmiss_field_real_parameters_get_values_sparse(
				node_parameters, index, componentCount, values, valueExists, &valuesRead))
			{
				if (0 < valuesRead)
				{
					// A limitation of cmgui is that all nodal component values must be set if any are set.
					// Set the dummy values to zero.
					if (valuesRead < componentCount)
					{
						for (int i = 0; i < componentCount; i++)
						{
							if (!valueExists[i])
								values[i] = 0.0;
						}
					}
					Cmiss_node_merge(node, node_template);
					Cmiss_field_set_values_at_node(reinterpret_cast<Cmiss_field_id>(fe_field), node, /*time*/0.0, componentCount, values);
				}
			}
			else
			{
				return_code = 0;
			}
			Cmiss_node_destroy(&node);
			if (!Cmiss_ensemble_iterator_increment(nodesIterator))
				break;
		}
		delete[] valueExists;
		delete[] values;
		Cmiss_ensemble_iterator_destroy(&nodesIterator);
		Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id*>(&node_parameters));
		Cmiss_ensemble_index_destroy(&index);
		Cmiss_node_template_destroy(&node_template);
	}

	// define element fields

	Cmiss_fe_mesh_id mesh = Cmiss_region_get_fe_mesh_by_name(region,
		(meshDimension == 3 ? "cmiss_mesh_3d" :
			(meshDimension == 2 ? "cmiss_mesh_2d" : "cmiss_mesh_1d")));
	Cmiss_element_template_id element_template = 0;
	Cmiss_field_ensemble_id elementsEnsemble = getEnsemble(fmlElementsType);
	Cmiss_ensemble_iterator_id elementsIterator = Cmiss_field_ensemble_get_first_entry(elementsEnsemble);
	if (!elementsIterator)
	{
		return_code = 0;
	}
	std::vector<FmlObjectHandle> fmlElementEvaluators(componentCount, FML_INVALID_OBJECT_HANDLE);
	std::vector<ElementFieldComponent*> components(componentCount, (ElementFieldComponent*)0);
	while (return_code)
	{
		int elementIdentifier = Cmiss_ensemble_iterator_get_identifier(elementsIterator);
		bool newElementTemplate = (element_template != 0);
		bool definedOnAllComponents = true;
		for (int ic = 0; ic < componentCount; ic++)
		{
			FmlObjectHandle fmlElementEvaluator = Fieldml_GetElementEvaluator(fmlHandle, fmlComponentEvaluators[ic],
				static_cast<int>(elementIdentifier), /*allowDefault*/1);
			if (fmlElementEvaluator != fmlElementEvaluators[ic])
			{
				fmlElementEvaluators[ic] = fmlElementEvaluator;
				newElementTemplate = true;
			}
			if (fmlElementEvaluator == FML_INVALID_OBJECT_HANDLE)
			{
				definedOnAllComponents = false;
				break;
			}
		}
		if (!definedOnAllComponents)
		{
			if (!Cmiss_ensemble_iterator_increment(elementsIterator))
				break;
			continue;
		}
		if (newElementTemplate)
		{
			if (element_template)
				Cmiss_element_template_destroy(&element_template);
			element_template = Cmiss_fe_mesh_create_element_template(mesh);
			// do not want to override shape of existing elements:
			Cmiss_element_template_set_shape_type(element_template, CMISS_ELEMENT_SHAPE_TYPE_INVALID);
			int total_local_point_count = 0;
			for (int ic = 0; ic < componentCount; ic++)
			{
				components[ic] = getElementFieldComponent(mesh, fmlElementEvaluators[ic],
					fmlNodeParametersVariable, fmlNodeVariable, fmlElementVariable);
				if (!components[ic])
				{
					display_message(ERROR_MESSAGE, "Read FieldML:  Aggregate %s component %d element %d evaluator %s does not reference a supported basis function or mapping",
						fieldName.c_str(), ic + 1, elementIdentifier, getObjectName(fmlElementEvaluators[ic]).c_str());
					return_code = 0;
					break;
				}
				bool new_local_point_to_node = true;
				for (int jc = 0; jc < ic; jc++)
				{
					if (components[jc]->local_point_to_node == components[ic]->local_point_to_node)
					{
						new_local_point_to_node = false;
						break;
					}
				}
				if (new_local_point_to_node)
				{
					for (int i = 0; i < components[ic]->local_point_count; i++)
					{
						components[ic]->local_point_indexes[i] = total_local_point_count + i + 1;
					}
					total_local_point_count += components[ic]->local_point_count;
					Cmiss_element_template_set_number_of_nodes(element_template, total_local_point_count);
				}
				if (!Cmiss_element_template_define_field_simple_nodal(element_template, fe_field,
					/*component*/ic + 1, components[ic]->element_basis, components[ic]->local_point_count,
					components[ic]->local_point_indexes))
				{
					return_code = 0;
					break;
				}
			}
			if (!Cmiss_element_template_finalise(element_template))
			{
				return_code = 0;
			}
		}
		if (!return_code)
			break;

		int total_local_point_count = 0;
		for (int ic = 0; (ic < componentCount) && return_code; ic++)
		{
			ElementFieldComponent *component = components[ic];
			if ((total_local_point_count + 1) == component->local_point_indexes[0])
			{
				total_local_point_count += component->local_point_count;
				Cmiss_ensemble_index_set_entry(component->index, elementsIterator);
				if (!Cmiss_field_integer_parameters_get_values(component->local_point_to_node, component->index,
					component->local_point_count, component->node_identifiers))
				{
					display_message(ERROR_MESSAGE, "Read FieldML:  Incomplete local to global map for field %s", fieldName.c_str());
					return_code = 0;
					break;
				}
				for (int i = 0; i < component->local_point_count; i++)
				{
					Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodes, component->node_identifiers[i]);
					if (!node)
					{
						char *local_point_to_node_name = Cmiss_field_get_name(reinterpret_cast<Cmiss_field_id>(component->local_point_to_node));
						display_message(ERROR_MESSAGE, "Read FieldML:  Cannot find node %d for element %d local point %d in local point to node map %s",
							component->node_identifiers[i], elementIdentifier, i + 1, local_point_to_node_name);
						DEALLOCATE(local_point_to_node_name);
						return_code = 0;
						break;
					}
					Cmiss_element_template_set_node(element_template, component->local_point_indexes[i], node);
					Cmiss_node_destroy(&node);
				}
			}
		}
		if (return_code)
		{
			Cmiss_element_id element = Cmiss_fe_mesh_find_element_by_identifier(mesh, elementIdentifier);
			if (!Cmiss_element_merge(element, element_template))
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  Could not merge element %d", elementIdentifier);
				return_code = 0;
			}
			Cmiss_element_destroy(&element);
		}
		if (!Cmiss_ensemble_iterator_increment(elementsIterator))
			break;
	}
	Cmiss_ensemble_iterator_destroy(&elementsIterator);
	Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id*>(&elementsEnsemble));
	Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id*>(&nodesEnsemble));
	if (element_template)
		Cmiss_element_template_destroy(&element_template);
	Cmiss_fe_mesh_destroy(&mesh);
	Cmiss_nodeset_destroy(&nodes);
	Cmiss_field_finite_element_destroy(&fe_field);

	return return_code;
}

/** continuous-valued aggregates of piecewise varying with mesh elements are
 * interpreted as vector-valued finite element fields */
int FieldMLReader::readAggregateFields()
{
	const int aggregateCount = Fieldml_GetObjectCount(fmlHandle, FHT_AGGREGATE_EVALUATOR);
	int return_code = 1;
	for (int aggregateIndex = 1; (aggregateIndex <= aggregateCount) && return_code; aggregateIndex++)
	{
		FmlObjectHandle fmlAggregate = Fieldml_GetObject(fmlHandle, FHT_AGGREGATE_EVALUATOR, aggregateIndex);
		std::string fieldName = getObjectName(fmlAggregate);

		FmlObjectHandle fmlValueType = Fieldml_GetValueType(fmlHandle, fmlAggregate);
		if (FHT_CONTINUOUS_TYPE != Fieldml_GetObjectType(fmlHandle, fmlValueType))
		{
			display_message(WARNING_MESSAGE, "Read FieldML:  Ignore aggregate %s as not continuous type\n", fieldName.c_str());
			continue;
		}
		FmlObjectHandle fmlValueTypeComponentEnsembleType = Fieldml_GetTypeComponentEnsemble(fmlHandle, fmlValueType);
		const int componentCount = (fmlValueTypeComponentEnsembleType == FML_INVALID_OBJECT_HANDLE) ? 1 :
			Fieldml_GetEnsembleTypeElementCount(fmlHandle, fmlValueTypeComponentEnsembleType);

		// check aggregate components are piecewise over elements

		bool piecewiseOverElements = true;
		std::vector<FmlObjectHandle> fmlComponentEvaluators(componentCount, FML_INVALID_OBJECT_HANDLE);

		// GRC must change to iterate over actual identifiers once non-contiguous bounds supported
		// Note: can iterate over ensemble in FieldML API since it is a full implementation

		FmlObjectHandle fmlElementVariable = FML_INVALID_OBJECT_HANDLE;
		for (int componentIndex = 1; componentIndex <= componentCount; componentIndex++)
		{
			int componentIdentifier = componentIndex;
			int ic = componentIndex - 1;
			fmlComponentEvaluators[ic] = Fieldml_GetElementEvaluator(fmlHandle, fmlAggregate, componentIdentifier, /*allowDefault*/1);
			if (fmlComponentEvaluators[ic] == FML_INVALID_OBJECT_HANDLE)
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  Aggregate %s component %d evaluator is missing", fieldName.c_str(), componentIndex);
				return_code = 0;
				break;
			}
			FmlObjectHandle fmlIndexEvaluator = Fieldml_GetIndexEvaluator(fmlHandle, fmlComponentEvaluators[ic], /*evaluatorIndex*/1);
			if (fmlElementVariable == FML_INVALID_OBJECT_HANDLE)
			{
				fmlElementVariable = fmlIndexEvaluator;
			}
			if ((FHT_PIECEWISE_EVALUATOR != Fieldml_GetObjectType(fmlHandle, fmlComponentEvaluators[ic])) ||
				(FHT_CONTINUOUS_TYPE != Fieldml_GetObjectType(fmlHandle, Fieldml_GetValueType(fmlHandle, fmlComponentEvaluators[ic]))) ||
				(Fieldml_GetValueType(fmlHandle, fmlIndexEvaluator) != fmlElementsType))
			{
				if (verbose)
				{
					display_message(WARNING_MESSAGE, "Read FieldML:  Aggregate %s component %d is not piecewise indexed by mesh elements ensemble",
						fieldName.c_str(), componentIndex);
				}
				piecewiseOverElements = false;
				break;
			}
			else if (((Fieldml_GetObjectType(fmlHandle, fmlIndexEvaluator) != FHT_ABSTRACT_EVALUATOR) &&
				(Fieldml_GetObjectType(fmlHandle, fmlIndexEvaluator) != FHT_EXTERNAL_EVALUATOR)) || // GRC workaround; FHT_EXTERNAL_EVALUATOR is wrong
				(fmlIndexEvaluator != fmlElementVariable))
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  Aggregate %s components must use same abstract evaluator for element index",
					fieldName.c_str());
				return_code = 0;
				break;
			}
		}
		if (!return_code)
		{
			break;
		}
		if (!piecewiseOverElements)
		{
			if (verbose)
			{
				display_message(WARNING_MESSAGE,
					"Read FieldML:  Aggregate %s cannot be interpreted as a field defined over a mesh. Skipping.",
					fieldName.c_str());
			}
			continue;
		}

		// determine if exactly one binding of 'nodal parameters'

		int bindCount = Fieldml_GetBindCount(fmlHandle, fmlAggregate);
		FmlObjectHandle fmlNodeParametersVariable = Fieldml_GetBindVariable(fmlHandle, fmlAggregate, 1);
		FmlObjectHandle fmlNodeParameters = Fieldml_GetBindEvaluator(fmlHandle, fmlAggregate, 1);
		int nodeParametersIndexCount = Fieldml_GetIndexCount(fmlHandle, fmlNodeParameters);
		FmlObjectHandle fmlNodeParametersIndex1 = Fieldml_GetIndexEvaluator(fmlHandle, fmlNodeParameters, 1);
		FmlObjectHandle fmlNodeParametersIndex2 = Fieldml_GetIndexEvaluator(fmlHandle, fmlNodeParameters, 2);
		if ((1 != bindCount) ||
			(Fieldml_GetObjectType(fmlHandle, fmlNodeParameters) != FHT_PARAMETER_EVALUATOR) ||
			(Fieldml_GetObjectType(fmlHandle, Fieldml_GetValueType(fmlHandle, fmlNodeParameters)) != FHT_CONTINUOUS_TYPE) ||
			(2 != nodeParametersIndexCount) ||
			(fmlNodeParametersIndex1 == FML_INVALID_OBJECT_HANDLE) ||
			(fmlNodeParametersIndex2 == FML_INVALID_OBJECT_HANDLE))
		{
			if (verbose)
			{
				display_message(WARNING_MESSAGE,
					"Read FieldML:  Aggregate %s does not bind exactly 1 nodal parameters object. Skipping.",
					fieldName.c_str());
			}
			continue;
		}
		FmlObjectHandle fmlNodeParametersIndex1Type = Fieldml_GetValueType(fmlHandle, fmlNodeParametersIndex1);
		FmlObjectHandle fmlNodeParametersIndex2Type = Fieldml_GetValueType(fmlHandle, fmlNodeParametersIndex2);
		FmlObjectHandle fmlNodeEnsembleType = FML_INVALID_OBJECT_HANDLE;
		FmlObjectHandle fmlNodeVariable = FML_INVALID_OBJECT_HANDLE;
		if (fmlNodeParametersIndex1Type == fmlValueTypeComponentEnsembleType)
		{
			fmlNodeVariable = fmlNodeParametersIndex2;
			fmlNodeEnsembleType = fmlNodeParametersIndex2Type;
		}
		else if (fmlNodeParametersIndex2Type == fmlValueTypeComponentEnsembleType)
		{
			fmlNodeVariable = fmlNodeParametersIndex1;
			fmlNodeEnsembleType = fmlNodeParametersIndex1Type;
		}
		if (fmlNodeEnsembleType == FML_INVALID_OBJECT_HANDLE)
		{
			display_message(ERROR_MESSAGE,
				"Read FieldML:  Aggregate %s binds parameters %s that do not vary with component.",
				fieldName.c_str());
			break;
		}
		if ((fmlNodesType != FML_INVALID_OBJECT_HANDLE) &&
			(fmlNodeEnsembleType != fmlNodesType))
		{
			display_message(ERROR_MESSAGE,
				"Read FieldML:  Aggregate %s binds parameters %s indexed by an unknown ensemble %s not matching 'nodes' %s used for other fields.",
				fieldName.c_str(), getObjectName(fmlNodeParameters).c_str(), getObjectName(fmlNodeEnsembleType).c_str(), getObjectName(fmlNodesType).c_str());
			break;
		}

		return_code = readField(fmlAggregate, fmlComponentEvaluators, fmlNodeEnsembleType, fmlNodeParameters,
			fmlNodeParametersVariable, fmlNodeVariable, fmlElementVariable);
	}
	return return_code;
}

/** continuous-valued references to piecewise varying with mesh elements are
 * interpreted as scalar-valued finite element fields */
int FieldMLReader::readReferenceFields()
{
	const int referenceCount = Fieldml_GetObjectCount(fmlHandle, FHT_REFERENCE_EVALUATOR);
	int return_code = 1;
	for (int referenceIndex = 1; (referenceIndex <= referenceCount) && return_code; referenceIndex++)
	{
		FmlObjectHandle fmlReference = Fieldml_GetObject(fmlHandle, FHT_REFERENCE_EVALUATOR, referenceIndex);
		std::string fieldName = getObjectName(fmlReference);

		FmlObjectHandle fmlValueType = Fieldml_GetValueType(fmlHandle, fmlReference);
		if (FHT_CONTINUOUS_TYPE != Fieldml_GetObjectType(fmlHandle, fmlValueType))
		{
			//display_message(WARNING_MESSAGE, "Read FieldML:  Ignore reference %s as not continuous type\n", fieldName.c_str());
			continue;
		}

		// check reference evaluator is piecewise over elements
		// GRC should check it is scalar too

		bool piecewiseOverElements = true;

		// GRC must change to iterate over actual identifiers once non-contiguous bounds supported
		// Note: can iterate over ensemble in FieldML API since it is a full implementation

		FmlObjectHandle fmlElementVariable = FML_INVALID_OBJECT_HANDLE;

		FmlObjectHandle fmlComponentEvaluator = Fieldml_GetReferenceRemoteEvaluator(fmlHandle, fmlReference);
		if (fmlComponentEvaluator == FML_INVALID_OBJECT_HANDLE)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Reference %s remove evaluator is missing", fieldName.c_str());
			return_code = 0;
			break;
		}

		FmlObjectHandle fmlIndexEvaluator = Fieldml_GetIndexEvaluator(fmlHandle, fmlComponentEvaluator, /*evaluatorIndex*/1);
		if (fmlElementVariable == FML_INVALID_OBJECT_HANDLE)
		{
			fmlElementVariable = fmlIndexEvaluator;
		}
		if ((FHT_PIECEWISE_EVALUATOR != Fieldml_GetObjectType(fmlHandle, fmlComponentEvaluator)) ||
			(FHT_CONTINUOUS_TYPE != Fieldml_GetObjectType(fmlHandle, Fieldml_GetValueType(fmlHandle, fmlComponentEvaluator))) ||
			(Fieldml_GetValueType(fmlHandle, fmlIndexEvaluator) != fmlElementsType))
		{
			if (verbose)
			{
				display_message(WARNING_MESSAGE, "Read FieldML:  Reference %s remote evaluator %s is not piecewise indexed by mesh elements ensemble",
					fieldName.c_str(), getObjectName(fmlComponentEvaluator).c_str());
			}
			piecewiseOverElements = false;
		}
		else if (((Fieldml_GetObjectType(fmlHandle, fmlIndexEvaluator) != FHT_ABSTRACT_EVALUATOR) &&
			(Fieldml_GetObjectType(fmlHandle, fmlIndexEvaluator) != FHT_EXTERNAL_EVALUATOR))) // GRC workaround; FHT_EXTERNAL_EVALUATOR is wrong
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Reference %s does not use an abstract evaluator for element index",
				fieldName.c_str());
			return_code = 0;
			break;
		}
		if (!piecewiseOverElements)
		{
			if (verbose)
			{
				display_message(WARNING_MESSAGE,
					"Read FieldML:  Reference %s cannot be interpreted as a field defined over a mesh. Skipping.",
					fieldName.c_str());
			}
			continue;
		}

		// determine if exactly one binding of 'nodal parameters'

		int bindCount = Fieldml_GetBindCount(fmlHandle, fmlReference);
		FmlObjectHandle fmlNodeParametersVariable = Fieldml_GetBindVariable(fmlHandle, fmlReference, 1);
		FmlObjectHandle fmlNodeParameters = Fieldml_GetBindEvaluator(fmlHandle, fmlReference, 1);
		int nodeParametersIndexCount = Fieldml_GetIndexCount(fmlHandle, fmlNodeParameters);
		FmlObjectHandle fmlNodeParametersIndex1 = Fieldml_GetIndexEvaluator(fmlHandle, fmlNodeParameters, 1);
		if ((1 != bindCount) ||
			(Fieldml_GetObjectType(fmlHandle, fmlNodeParameters) != FHT_PARAMETER_EVALUATOR) ||
			(Fieldml_GetObjectType(fmlHandle, Fieldml_GetValueType(fmlHandle, fmlNodeParameters)) != FHT_CONTINUOUS_TYPE) ||
			(1 != nodeParametersIndexCount) ||
			(fmlNodeParametersIndex1 == FML_INVALID_OBJECT_HANDLE))
		{
			if (verbose)
			{
				display_message(WARNING_MESSAGE,
					"Read FieldML:  Reference %s does not bind exactly 1 nodal parameters object. Skipping.",
					fieldName.c_str());
			}
			continue;
		}
		FmlObjectHandle fmlNodeParametersIndex1Type = Fieldml_GetValueType(fmlHandle, fmlNodeParametersIndex1);
		FmlObjectHandle fmlNodeVariable = fmlNodeParametersIndex1;
		FmlObjectHandle fmlNodeEnsembleType = fmlNodeParametersIndex1Type;
		if ((fmlNodesType != FML_INVALID_OBJECT_HANDLE) &&
			(fmlNodeEnsembleType != fmlNodesType))
		{
			display_message(ERROR_MESSAGE,
				"Read FieldML:  Reference %s binds parameters %s indexed by an unknown ensemble %s not matching 'nodes' %s used for other fields.",
				fieldName.c_str(), getObjectName(fmlNodeParameters).c_str(), getObjectName(fmlNodeEnsembleType).c_str(), getObjectName(fmlNodesType).c_str());
			break;
		}

		std::vector<FmlObjectHandle> fmlComponentEvaluators(1, fmlComponentEvaluator);
		return_code = readField(fmlReference, fmlComponentEvaluators, fmlNodeEnsembleType, fmlNodeParameters,
			fmlNodeParametersVariable, fmlNodeVariable, fmlElementVariable);
	}
	return return_code;
}

int FieldMLReader::parse()
{
	int return_code = 1;
	if ((!region) || (!filename) || (!field_module))
	{
		display_message(ERROR_MESSAGE, "FieldMLReader::parse.  Invalid construction arguments");
		return_code = 0;
	}
	if (fmlHandle == (const FmlHandle)FML_INVALID_HANDLE)
	{
		display_message(ERROR_MESSAGE, "Read FieldML: could not parse file %s", filename);
		return_code = 0;
	}
	Cmiss_region_begin_change(region);
	return_code = return_code && readMeshes();
	return_code = return_code && readAggregateFields();
	return_code = return_code && readReferenceFields();
	Cmiss_region_end_change(region);
	return return_code;
}

} // anonymous namespace

int parse_fieldml_file(struct Cmiss_region *region, const char *filename)
{
	FieldMLReader fmlReader(region, filename);
	return fmlReader.parse();
}
