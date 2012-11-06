/***************************************************************************//**
 * FILE : read_fieldml.cpp
 * 
 * Functions for importing regions and fields from FieldML 0.4+ documents.
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
#include "zinc/element.h"
#include "zinc/field.h"
#include "zinc/fieldmodule.h"
#include "zinc/fieldfiniteelement.h"
#include "zinc/node.h"
#include "zinc/region.h"
#include "zinc/status.h"
#include "field_io/cmiss_field_ensemble.h"
#include "field_io/cmiss_field_parameters.h"
#include "field_io/read_fieldml.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"
#include "fieldml_api.h"
#include "FieldmlIoApi.h"

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
	{ "shape.unit.line",        1, CMISS_ELEMENT_SHAPE_LINE },
	{ "shape.unit.square",      2, CMISS_ELEMENT_SHAPE_SQUARE },
	{ "shape.unit.triangle",    2, CMISS_ELEMENT_SHAPE_TRIANGLE },
	{ "shape.unit.cube",        3, CMISS_ELEMENT_SHAPE_CUBE },
	{ "shape.unit.tetrahedron", 3, CMISS_ELEMENT_SHAPE_TETRAHEDRON },
	{ "shape.unit.wedge12",     3, CMISS_ELEMENT_SHAPE_WEDGE12 },
	{ "shape.unit.wedge13",     3, CMISS_ELEMENT_SHAPE_WEDGE13 },
	{ "shape.unit.wedge23",     3, CMISS_ELEMENT_SHAPE_WEDGE23 }
};
const int numLibraryShapes = sizeof(libraryShapes) / sizeof(ShapeType);

const char *libraryChartArgumentNames[] =
{
	0,
	"chart.1d.argument",
	"chart.2d.argument",
	"chart.3d.argument"
};

// map from zienkiewicz winding to cmgui
//const int triquadraticSimplex_zienkiewicz_swizzle[10] = { 1, 5, 2, 7, 10, 4, 6, 8, 9, 3 };
const int triquadraticSimplex_zienkiewicz_swizzle[10] = { 1, 3, 6, 10, 2, 4, 7, 5, 9, 8 };
const int biquadraticSimplex_vtk_swizzle[6] = { 1, 3, 6, 2, 5, 4 };

struct BasisType
{
	int dimension;
	const char *fieldmlBasisEvaluatorName;
	bool homogeneous;
	enum Cmiss_basis_function_type functionType[3];
	const int *swizzle;
};

const BasisType libraryBases[] =
{
	{ 1, "interpolator.1d.unit.linearLagrange",      true, { CMISS_BASIS_FUNCTION_LINEAR_LAGRANGE, CMISS_BASIS_FUNCTION_TYPE_INVALID, CMISS_BASIS_FUNCTION_TYPE_INVALID }, 0 },
	{ 1, "interpolator.1d.unit.quadraticLagrange",   true, { CMISS_BASIS_FUNCTION_QUADRATIC_LAGRANGE, CMISS_BASIS_FUNCTION_TYPE_INVALID, CMISS_BASIS_FUNCTION_TYPE_INVALID }, 0 },
	{ 1, "interpolator.1d.unit.cubicLagrange",       true, { CMISS_BASIS_FUNCTION_CUBIC_LAGRANGE, CMISS_BASIS_FUNCTION_TYPE_INVALID, CMISS_BASIS_FUNCTION_TYPE_INVALID }, 0 },
	{ 2, "interpolator.2d.unit.bilinearLagrange",    true, { CMISS_BASIS_FUNCTION_LINEAR_LAGRANGE, CMISS_BASIS_FUNCTION_LINEAR_LAGRANGE, CMISS_BASIS_FUNCTION_TYPE_INVALID }, 0 },
	{ 2, "interpolator.2d.unit.biquadraticLagrange", true, { CMISS_BASIS_FUNCTION_QUADRATIC_LAGRANGE, CMISS_BASIS_FUNCTION_QUADRATIC_LAGRANGE, CMISS_BASIS_FUNCTION_TYPE_INVALID }, 0 },
	{ 2, "interpolator.2d.unit.bicubicLagrange",     true, { CMISS_BASIS_FUNCTION_CUBIC_LAGRANGE, CMISS_BASIS_FUNCTION_CUBIC_LAGRANGE, CMISS_BASIS_FUNCTION_TYPE_INVALID }, 0 },
	{ 2, "interpolator.2d.unit.bilinearSimplex",     true, { CMISS_BASIS_FUNCTION_LINEAR_SIMPLEX, CMISS_BASIS_FUNCTION_LINEAR_SIMPLEX, CMISS_BASIS_FUNCTION_TYPE_INVALID }, 0 },
	{ 2, "interpolator.2d.unit.biquadraticSimplex",  true, { CMISS_BASIS_FUNCTION_QUADRATIC_SIMPLEX, CMISS_BASIS_FUNCTION_QUADRATIC_SIMPLEX, CMISS_BASIS_FUNCTION_TYPE_INVALID }, 0 },
	{ 2, "interpolator.2d.unit.biquadraticSimplex.vtk",  true, { CMISS_BASIS_FUNCTION_QUADRATIC_SIMPLEX, CMISS_BASIS_FUNCTION_QUADRATIC_SIMPLEX, CMISS_BASIS_FUNCTION_TYPE_INVALID }, biquadraticSimplex_vtk_swizzle },
	{ 3, "interpolator.3d.unit.trilinearLagrange",   true, { CMISS_BASIS_FUNCTION_LINEAR_LAGRANGE, CMISS_BASIS_FUNCTION_LINEAR_LAGRANGE, CMISS_BASIS_FUNCTION_LINEAR_LAGRANGE }, 0 },
	{ 3, "interpolator.3d.unit.triquadraticLagrange",true, { CMISS_BASIS_FUNCTION_QUADRATIC_LAGRANGE, CMISS_BASIS_FUNCTION_QUADRATIC_LAGRANGE, CMISS_BASIS_FUNCTION_QUADRATIC_LAGRANGE }, 0 },
	{ 3, "interpolator.3d.unit.tricubicLagrange",    true, { CMISS_BASIS_FUNCTION_CUBIC_LAGRANGE, CMISS_BASIS_FUNCTION_CUBIC_LAGRANGE, CMISS_BASIS_FUNCTION_CUBIC_LAGRANGE }, 0 },
	{ 3, "interpolator.3d.unit.trilinearSimplex",    true, { CMISS_BASIS_FUNCTION_LINEAR_SIMPLEX, CMISS_BASIS_FUNCTION_LINEAR_SIMPLEX, CMISS_BASIS_FUNCTION_LINEAR_SIMPLEX }, 0 },
	{ 3, "interpolator.3d.unit.triquadraticSimplex", true, { CMISS_BASIS_FUNCTION_QUADRATIC_SIMPLEX, CMISS_BASIS_FUNCTION_QUADRATIC_SIMPLEX, CMISS_BASIS_FUNCTION_QUADRATIC_SIMPLEX }, 0 },
	{ 3, "interpolator.3d.unit.triquadraticSimplex.zienkiewicz", true, { CMISS_BASIS_FUNCTION_QUADRATIC_SIMPLEX, CMISS_BASIS_FUNCTION_QUADRATIC_SIMPLEX, CMISS_BASIS_FUNCTION_QUADRATIC_SIMPLEX }, triquadraticSimplex_zienkiewicz_swizzle },
	{ 3, "interpolator.3d.unit.trilinearWedge12",    false,{ CMISS_BASIS_FUNCTION_LINEAR_SIMPLEX, CMISS_BASIS_FUNCTION_LINEAR_SIMPLEX, CMISS_BASIS_FUNCTION_LINEAR_LAGRANGE }, 0 },
	{ 3, "interpolator.3d.unit.triquadraticWedge12", false,{ CMISS_BASIS_FUNCTION_QUADRATIC_SIMPLEX, CMISS_BASIS_FUNCTION_QUADRATIC_SIMPLEX, CMISS_BASIS_FUNCTION_QUADRATIC_LAGRANGE }, 0 },
	// GRC add Hermite!
	// GRC add vtk, zienkiewicz simplex ordering, swizzle
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
	const int *swizzle;
	int *local_point_indexes;
	int *swizzled_local_point_indexes;
	int *node_identifiers;

	ElementFieldComponent(Cmiss_element_basis_id element_basis,
			Cmiss_field_integer_parameters_id local_point_to_node,
			Cmiss_ensemble_index_id index, int local_point_count,
			const int *swizzle) :
		element_basis(element_basis),
		local_point_to_node(local_point_to_node),
		index(index),
		local_point_count(local_point_count),
		swizzle(swizzle),
		local_point_indexes(new int[local_point_count]),
		swizzled_local_point_indexes(new int[local_point_count]),
		node_identifiers(new int[local_point_count])
	{
	}

	~ElementFieldComponent()
	{
		Cmiss_element_basis_destroy(&element_basis);
		Cmiss_field_integer_parameters_destroy(&local_point_to_node);
		Cmiss_ensemble_index_destroy(&index);
		delete[] local_point_indexes;
		delete[] swizzled_local_point_indexes;
		delete[] node_identifiers;
	}
};

typedef std::map<FmlObjectHandle,ElementFieldComponent*> EvaluatorElementFieldComponentMap;

class FieldMLReader
{
	Cmiss_region *region;
	Cmiss_field_module_id field_module;
	const char *filename;
	FmlSessionHandle fmlSession;
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
		fmlSession(Fieldml_CreateFromFile(filename)),
		meshDimension(0),
		fmlNodesType(FML_INVALID_OBJECT_HANDLE),
		fmlElementsType(FML_INVALID_OBJECT_HANDLE),
		verbose(false),
		nameBufferLength(50),
		nameBuffer(new char[nameBufferLength])
	{
		Fieldml_SetDebug(fmlSession, /*debug*/verbose);
	}

	~FieldMLReader()
	{
		for (EvaluatorElementFieldComponentMap::iterator iter = componentMap.begin(); iter != componentMap.end(); iter++)
		{
			delete (iter->second);
		}
		Fieldml_Destroy(fmlSession);
		Cmiss_field_module_destroy(&field_module);
		Cmiss_region_destroy(&region);
		delete[] nameBuffer;
	}

	/** @return  1 on success, 0 on failure */
	int parse();

private:

	std::string getName(FmlObjectHandle fmlObjectHandle)
	{
		nameBuffer[0] = 0;
		while (true)
		{
			int length = Fieldml_CopyObjectName(fmlSession, fmlObjectHandle, nameBuffer, nameBufferLength);
			if (length < nameBufferLength - 1)
				break;
			nameBufferLength *= 2;
			delete[] nameBuffer;
			nameBuffer = new char[nameBufferLength];
		}
		return std::string(nameBuffer);
	}

	std::string getDeclaredName(FmlObjectHandle fmlObjectHandle)
	{
		nameBuffer[0] = 0;
		while (true)
		{
			int length = Fieldml_CopyObjectDeclaredName(fmlSession, fmlObjectHandle, nameBuffer, nameBufferLength);
			if (length < nameBufferLength - 1)
				break;
			nameBufferLength *= 2;
			delete[] nameBuffer;
			nameBuffer = new char[nameBufferLength];
		}
		return std::string(nameBuffer);
	}
	Cmiss_field_ensemble_id getEnsemble(FmlObjectHandle fmlEnsemble);

	int readParametersArray(FmlObjectHandle fmlParameters,
		Cmiss_field_id parameters, const char *name);

	Cmiss_field_id getParameters(FmlObjectHandle fmlParameters);

	enum Cmiss_element_shape_type getElementShapeFromName(const char *shapeName);

	int readMeshes();

	ElementFieldComponent *getElementFieldComponent(Cmiss_mesh_id mesh,
		FmlObjectHandle fmlEvaluator, FmlObjectHandle fmlNodeParametersArgument,
		FmlObjectHandle fmlNodeArgument, FmlObjectHandle fmlElementArgument);

	int readField(FmlObjectHandle fmlFieldEvaluator,
		std::vector<FmlObjectHandle> &fmlComponentEvaluators,
		FmlObjectHandle fmlNodeEnsembleType, FmlObjectHandle fmlNodeParameters,
		FmlObjectHandle fmlNodeParametersArgument, FmlObjectHandle fmlNodeArgument,
		FmlObjectHandle fmlElementArgument);

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

// GRC This should be in ensemble class & made more efficient
int Cmiss_field_ensemble_add_identifier_range(Cmiss_field_ensemble_id ensemble,
	int min, int max, int stride)
{
#ifdef DEBUG_CODE
	{
		char *name = Cmiss_field_get_name(Cmiss_field_ensemble_base_cast(ensemble));
		display_message(INFORMATION_MESSAGE, "Cmiss_field_ensemble_add_identifier_range: Ensemble %s min %d max %d (stride %d)\n", name, min, max, stride);
		DEALLOCATE(name);
	}
#endif // DEBUG_CODE
	if ((!ensemble) || (max < min) || (stride < 1))
	{
		char *name = Cmiss_field_get_name(Cmiss_field_ensemble_base_cast(ensemble));
		display_message(ERROR_MESSAGE, "Read FieldML:  Invalid range min=%d max=%d stride=%d for ensemble type %s",
			min, max, stride, name);
		DEALLOCATE(name);
		return 0;
	}
	for (FmlEnsembleValue identifier = min; identifier <= max; identifier += stride)
	{
#ifdef DEBUG_CODE
		char *name = Cmiss_field_get_name(Cmiss_field_ensemble_base_cast(ensemble));
		display_message(INFORMATION_MESSAGE, "Read FieldML:  Ensemble %s identifier %d (%d:%d)\n", name, identifier, min, max);
		DEALLOCATE(name);
#endif // DEBUG_CODE
		Cmiss_ensemble_iterator_id iterator =
			Cmiss_field_ensemble_find_or_create_entry(ensemble, identifier);
		if (!iterator)
		{
			char *name = Cmiss_field_get_name(Cmiss_field_ensemble_base_cast(ensemble));
			display_message(ERROR_MESSAGE, "Read FieldML:  Failed to create member %d for ensemble type %s",
				identifier, name);
			DEALLOCATE(name);
			return 0;
		}
		Cmiss_ensemble_iterator_destroy(&iterator);
	}
	return 1;
}

/***************************************************************************//**
 * Gets ensemble type matching fmlEnsembleType. Definition is read from
 * FieldML document only when first requested.
 * ???GRC technically, this should make ensembles from the argument, not the type
 *
 * @param fmlEnsembleType  Handle of type FHT_ENSEMBLE_TYPE.
 * @return  Accessed handle to ensemble field, or 0 on failure */
Cmiss_field_ensemble_id FieldMLReader::getEnsemble(FmlObjectHandle fmlEnsembleType)
{
	std::string name = getName(fmlEnsembleType);
	if (name.length()==0)
	{
		// GRC workaround for ensemble types that have not been imported
		name = "NONIMPORTED_";
		name.append(getDeclaredName(fmlEnsembleType));
	}
	if (Fieldml_GetObjectType(fmlSession, fmlEnsembleType) != FHT_ENSEMBLE_TYPE)
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

	FieldmlEnsembleMembersType fmlEnsembleMembersType = Fieldml_GetEnsembleMembersType(fmlSession, fmlEnsembleType);
	int recordSize = 0;
	switch (fmlEnsembleMembersType)
	{
	case FML_ENSEMBLE_MEMBER_RANGE:
		break;
	case FML_ENSEMBLE_MEMBER_LIST_DATA:
		recordSize = 1;
		break;
	case FML_ENSEMBLE_MEMBER_RANGE_DATA:
		recordSize = 2;
		break;
	case FML_ENSEMBLE_MEMBER_STRIDE_RANGE_DATA:
		recordSize = 3;
		break;
	case FML_ENSEMBLE_MEMBER_UNKNOWN:
	default:
		display_message(ERROR_MESSAGE, "Read FieldML:  Unsupported members type %d for ensemble type %s",
			fmlEnsembleMembersType, name.c_str());
		return 0;
		break;
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
	int return_code = 1;
	if (FML_ENSEMBLE_MEMBER_RANGE == fmlEnsembleMembersType)
	{
		FmlEnsembleValue min = Fieldml_GetEnsembleMembersMin(fmlSession, fmlEnsembleType);
		FmlEnsembleValue max = Fieldml_GetEnsembleMembersMax(fmlSession, fmlEnsembleType);
		int stride = Fieldml_GetEnsembleMembersStride(fmlSession, fmlEnsembleType);
		return_code = Cmiss_field_ensemble_add_identifier_range(ensemble, min, max, stride);
	}
	else
	{
		const int memberCount = Fieldml_GetMemberCount(fmlSession, fmlEnsembleType);
		FmlObjectHandle fmlDataSource = Fieldml_GetDataSource(fmlSession, fmlEnsembleType);
		int arrayRank = 0;
		int arraySizes[2];
		if (fmlDataSource == FML_INVALID_HANDLE)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Could not get data source for ensemble type %s", name.c_str());
			return_code = 0;
		}
		else if (FML_DATA_SOURCE_ARRAY != Fieldml_GetDataSourceType(fmlSession, fmlDataSource))
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Can only define ensemble types from array data source; processing %s", name.c_str());
			return_code = 0;
		}
		else if (2 != (arrayRank = Fieldml_GetArrayDataSourceRank(fmlSession, fmlDataSource)))
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Expected array data source of rank 2; processing %s", name.c_str());
			return_code = 0;
		}
		else if ((FML_ERR_NO_ERROR != Fieldml_GetArrayDataSourceSizes(fmlSession, fmlDataSource, arraySizes)) ||
			(arraySizes[0] < 1) || (arraySizes[1] != recordSize))
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Invalid data source sizes; processing %s", name.c_str());
			return_code = 0;
		}
		else
		{
			FmlReaderHandle fmlReader = Fieldml_OpenReader(fmlSession, fmlDataSource);
			int *rangeData = new int[arraySizes[0]*arraySizes[1]];
			const int arrayOffsets[2] = { 0, 0 };
			FmlIoErrorNumber ioResult = FML_IOERR_NO_ERROR;
			if (fmlReader == FML_INVALID_HANDLE)
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  Could not open reader for ensemble type %s", name.c_str());
				return_code = 0;
			}
			else if (FML_IOERR_NO_ERROR !=
				(ioResult = Fieldml_ReadIntSlab(fmlReader, arrayOffsets, arraySizes, rangeData)))
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  Error reading array data source %s", getName(fmlDataSource).c_str());
				return_code = 0;
			}
			else
			{
				const int recordCount = arraySizes[0];
				for (int i = 0; i < recordCount; i++)
				{
					switch (fmlEnsembleMembersType)
					{
					case FML_ENSEMBLE_MEMBER_LIST_DATA:
						return_code = Cmiss_field_ensemble_add_identifier_range(ensemble,
							/*min*/rangeData[i], /*max*/rangeData[i], /*stride*/1);
						break;
					case FML_ENSEMBLE_MEMBER_RANGE_DATA:
						return_code = Cmiss_field_ensemble_add_identifier_range(ensemble,
							/*min*/rangeData[i*2], /*max*/rangeData[i*2 + 1], /*stride*/1);
						break;
					case FML_ENSEMBLE_MEMBER_STRIDE_RANGE_DATA:
						return_code = Cmiss_field_ensemble_add_identifier_range(ensemble,
							/*min*/rangeData[i*3], /*max*/rangeData[i*3 + 1], /*stride*/rangeData[i*3 + 2]);
						break;
					default:
						// should never happen - see switch above
						display_message(ERROR_MESSAGE, "Read FieldML:  Unexpected ensemble members type");
						return_code = 0;
						break;
					}
					if (!return_code)
					{
						break;
					}
				}
				int ensembleSize = Cmiss_field_ensemble_get_size(ensemble);
				if (return_code && (ensembleSize != memberCount))
				{
					display_message(ERROR_MESSAGE, "Read FieldML:  Ensemble type %s lists member count %d, actual number in data source is %d",
						name.c_str(), memberCount, ensembleSize);
					return_code = 0;
				}
			}
			delete[] rangeData;
			Fieldml_CloseReader(fmlReader);
		}
	}
	if (return_code)
	{
		setProcessed(fmlEnsembleType);
	}
	else
	{
		Cmiss_field_ensemble_destroy(&ensemble);
	}
	return ensemble;
}

// TODO : Support order
// ???GRC can order cover subset of ensemble?
int FieldMLReader::readParametersArray(FmlObjectHandle fmlParameters,
	Cmiss_field_id parameters, const char *name)
{
	FieldmlDataDescriptionType dataDescription = Fieldml_GetParameterDataDescription(fmlSession, fmlParameters);
	if ((dataDescription != FML_DATA_DESCRIPTION_DENSE_ARRAY) &&
		(dataDescription != FML_DATA_DESCRIPTION_DOK_ARRAY))
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Unknown data description for parameters %s; must be dense array or DOK array", name);
		return 0;
	}
	Cmiss_field_real_parameters_id realParameters = Cmiss_field_cast_real_parameters(parameters);
	Cmiss_field_integer_parameters_id integerParameters = Cmiss_field_cast_integer_parameters(parameters);
	if ((!realParameters) && (!integerParameters))
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Invalid parameters field type");
		return 0;
	}

	int return_code = 1;
	const int recordIndexCount = (dataDescription == FML_DATA_DESCRIPTION_DOK_ARRAY) ? 1 : 0;

	const int denseIndexCount = Fieldml_GetParameterIndexCount(fmlSession, fmlParameters, /*isSparse*/0);
	Cmiss_field_ensemble_id *denseIndexEnsembles = new Cmiss_field_ensemble_id[denseIndexCount];
	Cmiss_ensemble_iterator_id *denseIterators = new Cmiss_ensemble_iterator_id[denseIndexCount];
	for (int i = 0; i < denseIndexCount; i++)
	{
		denseIndexEnsembles[i] = 0;
		denseIterators[i] = 0;
	}
	int arrayRank = 0;
	int *arrayRawSizes = 0;
	int *arrayOffsets = 0;
	int *arraySizes = 0;
	FmlObjectHandle fmlDataSource = Fieldml_GetDataSource(fmlSession, fmlParameters);
	if (fmlDataSource == FML_INVALID_HANDLE)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Could not get data source for parameters %s", name);
		return_code = 0;
	}
	else if (FML_DATA_SOURCE_ARRAY != Fieldml_GetDataSourceType(fmlSession, fmlDataSource))
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Only supports ArrayDataSource for parameters %s", name);
		return_code = 0;
	}
	else if ((arrayRank = Fieldml_GetArrayDataSourceRank(fmlSession, fmlDataSource))
		!= (recordIndexCount + denseIndexCount))
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Data source %s has invalid rank for parameters %s",
			getName(fmlDataSource).c_str(), name);
		return_code = 0;
	}
	else if ((arrayRank > 0) && ((0 == (arrayRawSizes = new int[arrayRank])) ||
		(0 == (arrayOffsets = new int[arrayRank])) || (0 == (arraySizes = new int[arrayRank])) ||
		(FML_ERR_NO_ERROR != Fieldml_GetArrayDataSourceRawSizes(fmlSession, fmlDataSource, arrayRawSizes)) ||
		(FML_ERR_NO_ERROR != Fieldml_GetArrayDataSourceOffsets(fmlSession, fmlDataSource, arrayOffsets)) ||
		(FML_ERR_NO_ERROR != Fieldml_GetArrayDataSourceSizes(fmlSession, fmlDataSource, arraySizes))))
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Failed to get array sizes of data source %s for parameters %s",
			getName(fmlDataSource).c_str(), name);
		return_code = 0;
	}
	else
	{
		// array size of 0 means 'all of raw size after offset', so calculate effective size.
		for (int r = 0; r < arrayRank; r++)
		{
			if (arraySizes[r] == 0)
				arraySizes[r] = arrayRawSizes[r] - arrayOffsets[r];
		}
		for (int i = 0; i < denseIndexCount; i++)
		{
			FmlObjectHandle fmlDenseIndexEvaluator = Fieldml_GetParameterIndexEvaluator(fmlSession, fmlParameters, i + 1, /*isSparse*/0);
			FmlObjectHandle fmlDenseIndexType = Fieldml_GetValueType(fmlSession, fmlDenseIndexEvaluator);
			int count = Fieldml_GetMemberCount(fmlSession, fmlDenseIndexType);
			if (count != arraySizes[recordIndexCount + i])
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  Data source %s size[%d]=%d, differs from size of dense index %s for parameters %s",
					getName(fmlDataSource).c_str(), recordIndexCount + i, arraySizes[recordIndexCount + i],
					getName(fmlDenseIndexEvaluator).c_str(), name);
				return_code = 0;
				break;
			}
			denseIndexEnsembles[i] = getEnsemble(fmlDenseIndexType);
			if (!denseIndexEnsembles[i])
			{
				return_code = 0;
				break;
			}
			denseIterators[i] = Cmiss_field_ensemble_get_first_entry(denseIndexEnsembles[i]);
			FmlObjectHandle fmlOrderDataSource = Fieldml_GetParameterIndexOrder(fmlSession, fmlParameters, i + 1);
			if (fmlOrderDataSource != FML_INVALID_HANDLE)
			{
				display_message(WARNING_MESSAGE, "Read FieldML:  Parameters %s dense index %s specifies order. This is not yet supported; results will be incorrect.",
					name, getName(fmlDenseIndexEvaluator).c_str());
			}
		}
	}

	int sparseIndexCount = 0;
	Cmiss_field_ensemble_id *sparseIndexEnsembles = 0;
	FmlObjectHandle fmlKeyDataSource = FML_INVALID_HANDLE;
	int keyArraySizes[2] = { 1, 0 };
	int keyArrayOffsets[2] = { 0, 0 };
	if (dataDescription == FML_DATA_DESCRIPTION_DOK_ARRAY)
	{
		sparseIndexCount = Fieldml_GetParameterIndexCount(fmlSession, fmlParameters, /*isSparse*/1);
		sparseIndexEnsembles = new Cmiss_field_ensemble_id[sparseIndexCount];
		for (int i = 0; i < sparseIndexCount; i++)
		{
			sparseIndexEnsembles[i] = 0;
		}
		fmlKeyDataSource = Fieldml_GetKeyDataSource(fmlSession, fmlParameters);
		if (fmlKeyDataSource == FML_INVALID_HANDLE)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Could not get key data source for parameters %s", name);
			return_code = 0;
		}
		else if (FML_DATA_SOURCE_ARRAY != Fieldml_GetDataSourceType(fmlSession, fmlKeyDataSource))
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Only supports ArrayDataSource for keys for parameters %s", name);
			return_code = 0;
		}
		else if (Fieldml_GetArrayDataSourceRank(fmlSession, fmlKeyDataSource) != 2)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Key data source %s for parameters %s must be rank 2",
				getName(fmlKeyDataSource).c_str(), name);
			return_code = 0;
		}
		else if ((FML_ERR_NO_ERROR != Fieldml_GetArrayDataSourceSizes(fmlSession, fmlKeyDataSource, keyArraySizes)) ||
			(keyArraySizes[0] != arraySizes[0]) || (keyArraySizes[1] != sparseIndexCount))
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Invalid array sizes for key data source %s for parameters %s",
				getName(fmlKeyDataSource).c_str(), name);
			return_code = 0;
		}
		else
		{
			for (int i = 0; i < sparseIndexCount; i++)
			{
				FmlObjectHandle fmlSparseIndexEvaluator = Fieldml_GetParameterIndexEvaluator(fmlSession, fmlParameters, i + 1, /*isSparse*/1);
				FmlObjectHandle fmlSparseIndexType = Fieldml_GetValueType(fmlSession, fmlSparseIndexEvaluator);
				sparseIndexEnsembles[i] = getEnsemble(fmlSparseIndexType);
				if (!sparseIndexEnsembles[i])
				{
					return_code = 0;
					break;
				}
			}
		}
	}

	Cmiss_ensemble_index_id index = realParameters ?
		Cmiss_field_real_parameters_create_index(realParameters) :
		Cmiss_field_integer_parameters_create_index(integerParameters);
	if (!index)
	{
		return_code = 0;
	}

	int valueBufferSize = 1;
	int totalDenseSize = 1;
	if (arraySizes && arrayOffsets)
	{
		for (int r = 0; r < arrayRank; r++)
		{
			valueBufferSize *= arraySizes[r];
			arrayOffsets[r] = 0;
			if (r >= recordIndexCount)
				totalDenseSize *= arraySizes[r];
		}
	}
	double *realValueBuffer = 0;
	int *integerValueBuffer = 0;
	int *keyBuffer = 0;
	if (return_code)
	{
		if (realParameters)
		{
			realValueBuffer = new double[valueBufferSize];
		}
		else
		{
			integerValueBuffer = new int[valueBufferSize];
		}
		if ((0 == realValueBuffer) && (0 == integerValueBuffer))
		{
			return_code = 0;
		}
		if (dataDescription == FML_DATA_DESCRIPTION_DOK_ARRAY)
		{
			keyBuffer = new int [keyArraySizes[0]*keyArraySizes[1]];
			if (0 == keyBuffer)
			{
				return_code = 0;
			}
		}
	}

	FmlReaderHandle fmlReader = FML_INVALID_HANDLE;
	FmlReaderHandle fmlKeyReader = FML_INVALID_HANDLE;
	if (return_code)
	{
		fmlReader = Fieldml_OpenReader(fmlSession, fmlDataSource);
		if (fmlReader == FML_INVALID_HANDLE)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Could not open reader for parameters %s data source %s",
				name, getName(fmlDataSource).c_str());
			return_code = 0;
		}
		if (dataDescription == FML_DATA_DESCRIPTION_DOK_ARRAY)
		{
			fmlKeyReader = Fieldml_OpenReader(fmlSession, fmlKeyDataSource);
			if (fmlKeyReader == FML_INVALID_HANDLE)
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  Could not open reader for parameters %s key data source %s",
					name, getName(fmlKeyDataSource).c_str());
				return_code = 0;
			}
		}
	}

	if (return_code)
	{
		FmlIoErrorNumber ioResult = FML_IOERR_NO_ERROR;
		if (realValueBuffer)
		{
			ioResult = Fieldml_ReadDoubleSlab(fmlReader, arrayOffsets, arraySizes, realValueBuffer);
		}
		else
		{
			ioResult = Fieldml_ReadIntSlab(fmlReader, arrayOffsets, arraySizes, integerValueBuffer);
		}
		if (ioResult != FML_IOERR_NO_ERROR)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Failed to read data source %s for parameters %s",
				getName(fmlDataSource).c_str(), name);
			return_code = 0;
		}
		if (dataDescription == FML_DATA_DESCRIPTION_DOK_ARRAY)
		{
			ioResult = Fieldml_ReadIntSlab(fmlKeyReader, keyArrayOffsets, keyArraySizes, keyBuffer);
			if (ioResult != FML_IOERR_NO_ERROR)
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  Failed to read key data source %s for parameters %s",
					getName(fmlKeyDataSource).c_str(), name);
				return_code = 0;
			}
		}
	}

	if (return_code)
	{
		const int recordCount = (dataDescription == FML_DATA_DESCRIPTION_DOK_ARRAY) ? arraySizes[0] : 1;
		for (int record = 0; (record < recordCount) && return_code; ++record)
		{
			if (dataDescription == FML_DATA_DESCRIPTION_DOK_ARRAY)
			{
				for (int i = 0; i < sparseIndexCount; i++)
				{
					// should make convenience function for these three calls
					Cmiss_ensemble_iterator_id entry =
						Cmiss_field_ensemble_find_entry_by_identifier(sparseIndexEnsembles[i], keyBuffer[record*sparseIndexCount + i]);
					Cmiss_ensemble_index_set_entry(index, entry);
					Cmiss_ensemble_iterator_destroy(&entry);
				}
			}
			if (realParameters)
			{
				return_code = Cmiss_field_real_parameters_set_values(realParameters,
					index, valueBufferSize, realValueBuffer + record*totalDenseSize);
			}
			else
			{
				return_code = Cmiss_field_integer_parameters_set_values(integerParameters,
					index, valueBufferSize, integerValueBuffer + record*totalDenseSize);
			}
		}
	}

	if (dataDescription == FML_DATA_DESCRIPTION_DOK_ARRAY)
	{
		Fieldml_CloseReader(fmlKeyReader);
	}
	Fieldml_CloseReader(fmlReader);
	if (sparseIndexEnsembles)
	{
		for (int i = 0; i < sparseIndexCount; i++)
		{
			Cmiss_field_ensemble_destroy(&(sparseIndexEnsembles[i]));
		}
		delete[] sparseIndexEnsembles;
	}
	if (denseIndexEnsembles)
	{
		for (int i = 0; i < denseIndexCount; i++)
		{
			Cmiss_field_ensemble_destroy(&(denseIndexEnsembles[i]));
		}
		delete[] denseIndexEnsembles;
	}
	if (denseIterators)
	{
		for (int i = 0; i < denseIndexCount; i++)
		{
			Cmiss_ensemble_iterator_destroy(&(denseIterators[i]));
		}
		delete[] denseIterators;
	}
	Cmiss_ensemble_index_destroy(&index);

	delete[] realValueBuffer;
	delete[] integerValueBuffer;
	delete[] keyBuffer;
	delete[] arraySizes;
	delete[] arrayOffsets;
	delete[] arrayRawSizes;

	if (realParameters)
		Cmiss_field_real_parameters_destroy(&realParameters);
	if (integerParameters)
		Cmiss_field_integer_parameters_destroy(&integerParameters);

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
	if (Fieldml_GetObjectType(fmlSession, fmlParameters) != FHT_PARAMETER_EVALUATOR)
	{
		display_message(ERROR_MESSAGE, "FieldMLReader::getParameters.  Invalid argument");
		return 0;
	}
	std::string name = getName(fmlParameters);

	FmlObjectHandle fmlValueType = Fieldml_GetValueType(fmlSession, fmlParameters);
	FieldmlHandleType value_class = Fieldml_GetObjectType(fmlSession, fmlValueType);
	if ((value_class != FHT_CONTINUOUS_TYPE) && ((value_class != FHT_ENSEMBLE_TYPE)))
	{
		display_message(ERROR_MESSAGE, "Read FieldML.  Cannot read parameters %s since not continuous or ensemble type", name.c_str());
		return 0;
	}
	bool isReal = (value_class == FHT_CONTINUOUS_TYPE);
	if (isReal)
	{
		FmlObjectHandle fmlValueTypeComponentEnsembleType = Fieldml_GetTypeComponentEnsemble(fmlSession, fmlValueType);
		if (fmlValueTypeComponentEnsembleType != FML_INVALID_OBJECT_HANDLE)
		{
			display_message(WARNING_MESSAGE, "Read FieldML:  Cannot read non-scalar parameters %s", name.c_str());
			return 0;
		}
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
			Cmiss_field_real_parameters_destroy(&realParameters);
		}
		else
		{
			Cmiss_field_integer_parameters_id integerParameters = Cmiss_field_cast_integer_parameters(parameters);
			invalidType = (integerParameters == 0);
			Cmiss_field_integer_parameters_destroy(&integerParameters);
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

	int indexEnsembleCount = Fieldml_GetIndexEvaluatorCount(fmlSession, fmlParameters);
	Cmiss_field_ensemble_id *indexEnsembles = new Cmiss_field_ensemble_id[indexEnsembleCount];
	for (int i = 0; i < indexEnsembleCount; i++)
	{
		indexEnsembles[i] = 0;
	}

	int return_code = 1;
	for (int indexEnsembleIndex = 1; indexEnsembleIndex <= indexEnsembleCount; indexEnsembleIndex++)
	{
		FmlObjectHandle fmlIndexEvaluator = Fieldml_GetIndexEvaluator(fmlSession, fmlParameters, indexEnsembleIndex);
		if ((FHT_ARGUMENT_EVALUATOR != Fieldml_GetObjectType(fmlSession, fmlIndexEvaluator)) ||
			(FHT_ENSEMBLE_TYPE != Fieldml_GetObjectType(fmlSession, Fieldml_GetValueType(fmlSession, fmlIndexEvaluator))))
		{
			display_message(WARNING_MESSAGE, "Read FieldML:  Index %d (%s) of parameters %s is not an ensemble-valued argument evaluator",
				indexEnsembleIndex, getName(fmlIndexEvaluator).c_str(), name.c_str());
			return_code = 0;
		}
		indexEnsembles[indexEnsembleIndex - 1] = getEnsemble(Fieldml_GetValueType(fmlSession, fmlIndexEvaluator));
		if (verbose)
		{
			display_message(INFORMATION_MESSAGE, "  Index ensemble %d = %s\n", indexEnsembleIndex,
				getName(fmlIndexEvaluator).c_str());
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
		return_code = readParametersArray(fmlParameters, parameters, name.c_str());
	}
	for (int i = 0; i < indexEnsembleCount; i++)
	{
		Cmiss_field_ensemble_destroy(&indexEnsembles[i]);
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
	display_message(ERROR_MESSAGE, "Read FieldML:  Unrecognised element shape %s", shapeName);
	return CMISS_ELEMENT_SHAPE_TYPE_INVALID;
}

int FieldMLReader::readMeshes()
{
	const int meshCount = Fieldml_GetObjectCount(fmlSession, FHT_MESH_TYPE);
	if (meshCount != 1)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Require 1 mesh type, %d found", meshCount);
		return 0;
	}
	int return_code = 1;
	for (int meshIndex = 1; (meshIndex <= meshCount) && return_code; meshIndex++)
	{
		FmlObjectHandle fmlMeshType = Fieldml_GetObject(fmlSession, FHT_MESH_TYPE, meshIndex);
		std::string name = getName(fmlMeshType);

		FmlObjectHandle fmlMeshChartType = Fieldml_GetMeshChartType(fmlSession, fmlMeshType);
		FmlObjectHandle fmlMeshChartComponentType = Fieldml_GetTypeComponentEnsemble(fmlSession, fmlMeshChartType);
		meshDimension = Fieldml_GetMemberCount(fmlSession, fmlMeshChartComponentType);
		if ((meshDimension < 1) || (meshDimension > 3))
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Invalid dimension %d for mesh type %s", meshDimension, name.c_str());
			return_code = 0;
			break;
		}

		if (verbose)
		{
			display_message(INFORMATION_MESSAGE, "Reading mesh '%s' dimension %d\n", name.c_str(), meshDimension);
		}

		fmlElementsType = Fieldml_GetMeshElementsType(fmlSession, fmlMeshType);
		Cmiss_field_ensemble_id elementsEnsemble = getEnsemble(fmlElementsType);
		if (verbose)
		{
			display_message(INFORMATION_MESSAGE, "Defining %d elements from %s\n",
				Cmiss_field_ensemble_get_size(elementsEnsemble), getName(fmlElementsType).c_str());
		}

		// determine element shape mapping

		FmlObjectHandle fmlShapeEvaluator = Fieldml_GetMeshShapes(fmlSession, fmlMeshType);
		Cmiss_element_shape_type const_shape_type = CMISS_ELEMENT_SHAPE_TYPE_INVALID;
		Cmiss_field_integer_parameters_id elementShapeParameters = 0; // used only if shape evaluator uses indirect map
		if (fmlShapeEvaluator == FML_INVALID_OBJECT_HANDLE)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Missing shape evaluator for mesh type %s", name.c_str());
			return_code = 0;
		}
		else if (FHT_BOOLEAN_TYPE != Fieldml_GetObjectType(fmlSession, Fieldml_GetValueType(fmlSession, fmlShapeEvaluator)))
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Non-boolean-valued shape evaluator for mesh type %s", name.c_str());
			return_code = 0;
		}
		else
		{
			// Note: external evaluator arguments are assumed to be 'used'
			int argumentCount = Fieldml_GetArgumentCount(fmlSession, fmlShapeEvaluator, /*isBound*/0, /*isUsed*/1);
			FmlObjectHandle fmlChartArgument = FML_INVALID_OBJECT_HANDLE;
			FmlObjectHandle fmlChartArgumentValue = FML_INVALID_OBJECT_HANDLE;
			FmlObjectHandle fmlElementsArgument = FML_INVALID_OBJECT_HANDLE;
			FmlObjectHandle fmlElementsArgumentValue = FML_INVALID_OBJECT_HANDLE;
			if (argumentCount > 0)
			{
				fmlChartArgument = Fieldml_GetArgument(fmlSession, fmlShapeEvaluator, 1, /*isBound*/0, /*isUsed*/1);
				fmlChartArgumentValue = Fieldml_GetValueType(fmlSession, fmlChartArgument);
			}
			if (argumentCount == 2)
			{
				fmlElementsArgument = Fieldml_GetArgument(fmlSession, fmlShapeEvaluator, 2, /*isBound*/0, /*isUsed*/1);
				fmlElementsArgumentValue = Fieldml_GetValueType(fmlSession, fmlElementsArgument);
				if (fmlElementsArgumentValue != fmlElementsType)
				{
					FmlObjectHandle tmp = fmlElementsArgument;
					fmlElementsArgument = fmlChartArgument;
					fmlChartArgument = tmp;
					tmp = fmlElementsArgumentValue;
					fmlElementsArgumentValue = fmlChartArgumentValue;
					fmlChartArgumentValue = tmp;
				}
			}
			FieldmlHandleType shapeEvaluatorValueType = Fieldml_GetObjectType(fmlSession, Fieldml_GetValueType(fmlSession, fmlShapeEvaluator));
			FieldmlHandleType chartArgumentValueType = (argumentCount > 0) ? Fieldml_GetObjectType(fmlSession, fmlChartArgumentValue) : FHT_UNKNOWN;
			if (((argumentCount != 1) && (argumentCount != 2)) ||
				(FHT_BOOLEAN_TYPE != shapeEvaluatorValueType) ||
				(FHT_CONTINUOUS_TYPE != chartArgumentValueType) ||
				(Fieldml_GetTypeComponentCount(fmlSession, fmlChartArgumentValue) != meshDimension) ||
				((argumentCount == 2) && (fmlElementsArgumentValue != fmlElementsType)))
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  Shape evaluator %s for mesh type %s must be a boolean evaluator with chart argument, plus optionally mesh elements argument.",
					getName(fmlShapeEvaluator).c_str(), name.c_str());
				return_code = 0;
			}
			else
			{
				FieldmlHandleType shapeEvaluatorType = Fieldml_GetObjectType(fmlSession, fmlShapeEvaluator);
				switch (shapeEvaluatorType)
				{
				case FHT_EXTERNAL_EVALUATOR:
					{
						// Case 1. single recognised shape external evaluator = all elements same shape
						const_shape_type = getElementShapeFromName(getName(fmlShapeEvaluator).c_str());
						if (const_shape_type == CMISS_ELEMENT_SHAPE_TYPE_INVALID)
						{
							display_message(ERROR_MESSAGE, "Read FieldML:  Unrecognised element shape evaluator %s for mesh type %s.",
								getName(fmlShapeEvaluator).c_str(), name.c_str());
							return_code = 0;
							break;
						}
					} break;
				case FHT_PIECEWISE_EVALUATOR:
					{
						FmlObjectHandle fmlIndexArgument = Fieldml_GetIndexEvaluator(fmlSession, fmlShapeEvaluator, 1);
						FmlObjectHandle fmlElementToShapeParameter = FML_INVALID_OBJECT_HANDLE;
						if (fmlIndexArgument == fmlElementsArgument)
						{
							// Case 2. piecewise over elements, directly mapping to recognised shape external evaluators
							// nothing more to do
						}
						else if ((1 != Fieldml_GetBindCount(fmlSession, fmlShapeEvaluator)) ||
							(Fieldml_GetBindArgument(fmlSession, fmlShapeEvaluator, 1) != fmlIndexArgument) ||
							(FML_INVALID_OBJECT_HANDLE == (fmlElementToShapeParameter = Fieldml_GetBindEvaluator(fmlSession, fmlShapeEvaluator, 1))) ||
							(FHT_PARAMETER_EVALUATOR != Fieldml_GetObjectType(fmlSession, fmlElementToShapeParameter)) ||
							(Fieldml_GetValueType(fmlSession, fmlElementToShapeParameter) !=
								Fieldml_GetValueType(fmlSession, fmlIndexArgument)))
						{
							display_message(ERROR_MESSAGE, "Read FieldML:  Shape evaluator %s for mesh type %s has unrecognised piecewise form.",
								getName(fmlShapeEvaluator).c_str(), name.c_str());
							return_code = 0;
						}
						else
						{
							// Case 3. piecewise over 'shape ensemble', indirectly mapping from parameters mapping from element
							Cmiss_field_id field = getParameters(fmlElementToShapeParameter);
							elementShapeParameters = Cmiss_field_cast_integer_parameters(field);
							if (!elementShapeParameters)
							{
								display_message(ERROR_MESSAGE, "Read FieldML:  Invalid element to shape parameters %s for shape evaluator %s of mesh type %s.",
									getName(fmlElementToShapeParameter).c_str(), getName(fmlShapeEvaluator).c_str(), name.c_str());
							}
							Cmiss_field_destroy(&field);
						}
					} break;
				default:
					display_message(ERROR_MESSAGE, "Read FieldML:  Shape evaluator %s for mesh type %s has unrecognised form.",
						getName(fmlShapeEvaluator).c_str(), name.c_str());
					return_code = 0;
					break;
				}
			}
		}

		// create elements in the mesh of given dimension

		Cmiss_mesh_id mesh = Cmiss_field_module_find_mesh_by_dimension(field_module, meshDimension);
		Cmiss_element_template_id element_template = Cmiss_mesh_create_element_template(mesh);

		Cmiss_ensemble_iterator *elementsIterator = Cmiss_field_ensemble_get_first_entry(elementsEnsemble);
		if (!elementsIterator)
		{
			return_code = 0;
		}
		FmlObjectHandle fmlLastElementShapeEvaluator = FML_INVALID_OBJECT_HANDLE;
		Cmiss_element_shape_type last_shape_type = CMISS_ELEMENT_SHAPE_TYPE_INVALID;
		Cmiss_ensemble_index_id elementIndex = elementShapeParameters ?
			Cmiss_field_integer_parameters_create_index(elementShapeParameters) : 0;
		while (return_code)
		{
			int elementIdentifier = Cmiss_ensemble_iterator_get_identifier(elementsIterator);
			Cmiss_element_shape_type shape_type = const_shape_type;
			if (const_shape_type == CMISS_ELEMENT_SHAPE_TYPE_INVALID)
			{
				int shapeIdentifier = elementIdentifier;
				if (elementShapeParameters &&
					((CMISS_OK != Cmiss_ensemble_index_set_entry(elementIndex, elementsIterator)) ||
					 (CMISS_OK != Cmiss_field_integer_parameters_get_values(elementShapeParameters, elementIndex, 1, &shapeIdentifier))))
				{
					display_message(ERROR_MESSAGE, "Read FieldML:  Failed to map shape of element %d in mesh type %s.",
						elementIdentifier, name.c_str());
					return_code = 0;
					break;
				}
				else
				{
					FmlObjectHandle fmlElementShapeEvaluator =
						Fieldml_GetElementEvaluator(fmlSession, fmlShapeEvaluator, shapeIdentifier, /*allowDefault*/1);
					if (fmlElementShapeEvaluator == fmlLastElementShapeEvaluator)
					{
						shape_type = last_shape_type;
					}
					else
					{
						shape_type = getElementShapeFromName(getName(fmlElementShapeEvaluator).c_str());
						fmlLastElementShapeEvaluator = fmlElementShapeEvaluator;
					}
					if (shape_type == CMISS_ELEMENT_SHAPE_TYPE_INVALID)
					{
						display_message(ERROR_MESSAGE, "Read FieldML:  Could not get shape of element %d in mesh type %s.",
							elementIdentifier, name.c_str());
						return_code = 0;
						break;
					}
				}
			}
			if (shape_type != last_shape_type)
			{
				if (!(Cmiss_element_template_set_shape_type(element_template, shape_type)))
				{
					return_code = 0;
					break;
				}
				last_shape_type = shape_type;
			}
			if (!Cmiss_mesh_define_element(mesh, elementIdentifier, element_template))
			{
				return_code = 0;
				break;
			}
			if (!Cmiss_ensemble_iterator_increment(elementsIterator))
				break;
		}

		if (elementShapeParameters)
			Cmiss_field_integer_parameters_destroy(&elementShapeParameters);

		Cmiss_ensemble_iterator_destroy(&elementsIterator);
		Cmiss_field_ensemble_destroy(&elementsEnsemble);

		Cmiss_element_template_destroy(&element_template);
		Cmiss_mesh_destroy(&mesh);
	}
	return return_code;
}

ElementFieldComponent *FieldMLReader::getElementFieldComponent(Cmiss_mesh_id mesh,
	FmlObjectHandle fmlEvaluator, FmlObjectHandle fmlNodeParametersArgument,
	FmlObjectHandle fmlNodeArgument, FmlObjectHandle fmlElementArgument)
{
	EvaluatorElementFieldComponentMap::iterator iter = componentMap.find(fmlEvaluator);
	if (iter != componentMap.end())
	{
		return iter->second;
	}

	USE_PARAMETER(mesh); // GRC should remove altogether
	std::string evaluatorName = getName(fmlEvaluator);
	FmlObjectHandle fmlEvaluatorType = Fieldml_GetValueType(fmlSession, fmlEvaluator);
	if ((FHT_REFERENCE_EVALUATOR != Fieldml_GetObjectType(fmlSession, fmlEvaluator)) ||
		(FHT_CONTINUOUS_TYPE != Fieldml_GetObjectType(fmlSession, fmlEvaluatorType)))
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  getElementFieldComponent argument %s is not a real-valued reference evaluator.",
			evaluatorName.c_str());
		return 0;
	}
	FmlObjectHandle fmlInterpolator = Fieldml_GetReferenceSourceEvaluator(fmlSession, fmlEvaluator);
	std::string interpolatorName = getDeclaredName(fmlInterpolator);
	std::string interpolatorLocalName = getName(fmlInterpolator);
	const char *interpolator_name = interpolatorName.c_str();
	int basis_index = -1;
	for (int i = 0; i < numLibraryBases; i++)
	{
		if (0 == strcmp(interpolator_name, libraryBases[i].fieldmlBasisEvaluatorName))
		{
			basis_index = i;
			break;
		}
	}
	if (basis_index < 0)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Reference evaluator %s references unknown basis interpolator %s (local name %s).",
			evaluatorName.c_str(), interpolator_name, interpolatorLocalName.c_str());
		return 0;
	}

	// Note: external evaluator arguments are assumed to be 'used'
	int interpolatorArgumentCount = Fieldml_GetArgumentCount(fmlSession, fmlInterpolator, /*isBound*/0, /*isUsed*/1);
	if (interpolatorArgumentCount != 2)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Reference evaluator %s source %s (local name %s) has %d argument(s); 2 are expected.",
			evaluatorName.c_str(), interpolator_name, interpolatorLocalName.c_str(), interpolatorArgumentCount);
		return 0;
	}

	FmlObjectHandle chartArgument = FML_INVALID_OBJECT_HANDLE;
	FmlObjectHandle parametersArgument = FML_INVALID_OBJECT_HANDLE;
	for (int i = 1; i <= interpolatorArgumentCount; i++)
	{
		FmlObjectHandle arg = Fieldml_GetArgument(fmlSession, fmlInterpolator, i, /*isBound*/0, /*isUsed*/1);
		std::string argName = getDeclaredName(arg);
		if (0 == argName.compare(libraryChartArgumentNames[meshDimension]))
		{
			if (chartArgument != FML_INVALID_OBJECT_HANDLE)
			{
				chartArgument = FML_INVALID_OBJECT_HANDLE;
				break;
			}
			chartArgument = arg;
		}
		else
		{
			// GRC more logic needed here for Hermite
			parametersArgument = arg;
		}
	}
	if ((FML_INVALID_OBJECT_HANDLE == chartArgument) ||
		(FML_INVALID_OBJECT_HANDLE == parametersArgument))
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Reference evaluator %s source %s (local name %s) is not a regular basis interpolator over %s.",
			evaluatorName.c_str(), interpolator_name, interpolatorLocalName.c_str(), libraryChartArgumentNames[meshDimension]);
		return 0;
	}

	FmlObjectHandle fmlLocalChartEvaluator = Fieldml_GetBindByArgument(fmlSession, fmlEvaluator, chartArgument);
	FmlObjectHandle fmlElementParametersEvaluator = Fieldml_GetBindByArgument(fmlSession, fmlEvaluator, parametersArgument);
	// exactly one mesh expected
	if (1 != Fieldml_GetObjectCount(fmlSession, FHT_MESH_TYPE))
	{
		display_message(ERROR_MESSAGE, "FieldMLReader::getElementFieldComponent:  Only supports 1 mesh type");
		return 0;
	}
	int return_code = 1;
	FmlObjectHandle fmlMeshType = Fieldml_GetObject(fmlSession, FHT_MESH_TYPE, /*meshIndex*/1);
	FmlObjectHandle fmlMeshChartType = Fieldml_GetMeshChartType(fmlSession, fmlMeshType);
	if ((fmlLocalChartEvaluator == FML_INVALID_OBJECT_HANDLE) ||
		(Fieldml_GetObjectType(fmlSession, fmlLocalChartEvaluator) != FHT_ARGUMENT_EVALUATOR) ||
		(Fieldml_GetValueType(fmlSession, fmlLocalChartEvaluator) != fmlMeshChartType))
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Evaluator %s does not bind local mesh chart argument to generic chart argument %s.",
			evaluatorName.c_str(), libraryChartArgumentNames[meshDimension]);
		return_code = 0;
	}
	if (fmlElementParametersEvaluator == FML_INVALID_OBJECT_HANDLE)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Evaluator %s does not bind to parameters argument %s for basis interpolator %s.",
			evaluatorName.c_str(), getDeclaredName(parametersArgument).c_str(), interpolator_name);
		return_code = 0;
	}
	int evaluatorBindCount = Fieldml_GetBindCount(fmlSession, fmlEvaluator);
	// GRC update for scaled Hermite
	if (2 != evaluatorBindCount)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Evaluator %s has %d bindings; interpolator %s requires 2 bindings to chart and parameters.",
			evaluatorName.c_str(), evaluatorBindCount, interpolator_name);
		return_code = 0;
	}
	if (!return_code)
		return 0;

	std::string elementParametersName = getName(fmlElementParametersEvaluator);

	//FmlObjectHandle fmlParametersType = Fieldml_GetValueType(fmlSession, parametersArgument);
	//int parametersComponentCount = Fieldml_GetTypeComponentCount(fmlSession, fmlParametersType);
	if ((Fieldml_GetObjectType(fmlSession, fmlElementParametersEvaluator) != FHT_AGGREGATE_EVALUATOR) ||
		(1 != Fieldml_GetIndexEvaluatorCount(fmlSession, fmlElementParametersEvaluator)))
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Expect evaluator %s parameter source %s to be an AggregateEvaluator with 1 index",
			evaluatorName.c_str(), elementParametersName.c_str());
		return 0;
	}
	FmlObjectHandle fmlLocalPointArgument = Fieldml_GetIndexEvaluator(fmlSession, fmlElementParametersEvaluator, 1);
	if (Fieldml_GetObjectType(fmlSession, fmlLocalPointArgument) != FHT_ARGUMENT_EVALUATOR)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Aggregate %s index %s must be an ArgumentEvaluator",
			elementParametersName.c_str(), getName(fmlLocalPointArgument).c_str());
		return 0;
	}
	FmlObjectHandle fmlTempNodeParameters = Fieldml_GetDefaultEvaluator(fmlSession, fmlElementParametersEvaluator);
	if ((fmlTempNodeParameters == FML_INVALID_OBJECT_HANDLE) ||
		(0 != Fieldml_GetEvaluatorCount(fmlSession, fmlElementParametersEvaluator)))
	{
		display_message(ERROR_MESSAGE, "Read FieldML (Current Limitation):  Evaluator %s element parameter source %s must use only default component evaluator",
			evaluatorName.c_str(), elementParametersName.c_str());
		return 0;
	}
	int localParametersBindCount = Fieldml_GetBindCount(fmlSession, fmlElementParametersEvaluator);
	if (1 != localParametersBindCount)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Evaluator %s element parameter source %s has %d bindings, expect 1 for local-to-global map.",
			evaluatorName.c_str(), elementParametersName.c_str(), localParametersBindCount);
		return 0;
	}

	FmlObjectHandle fmlTempNodeArgument = Fieldml_GetBindArgument(fmlSession, fmlElementParametersEvaluator, 1);
	FmlObjectHandle fmlLocalPointToNode = Fieldml_GetBindEvaluator(fmlSession, fmlElementParametersEvaluator, 1);

	if (fmlTempNodeParameters != fmlNodeParametersArgument)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Evaluator %s element parameter source %s default evaluator %s does not match nodal parameters argument %s",
			evaluatorName.c_str(), elementParametersName.c_str(), getName(fmlTempNodeParameters).c_str(), getName(fmlNodeParametersArgument).c_str());
		return_code = 0;
	}
	if (fmlTempNodeArgument != fmlNodeArgument)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Evaluator %s element parameter source %s should bind to node argument %s",
			evaluatorName.c_str(), elementParametersName.c_str(), getName(fmlNodeArgument).c_str());
		return_code = 0;
	}
	if (2 != Fieldml_GetIndexEvaluatorCount(fmlSession, fmlLocalPointToNode))
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Evaluator %s evaluator %s needs to indexes to be a local point to node map.",
			elementParametersName.c_str(), getName(fmlLocalPointToNode).c_str());
		return_code = 0;
	}
	else
	{
		FmlObjectHandle fmlLocalPointToNodeIndex1 = Fieldml_GetIndexEvaluator(fmlSession, fmlLocalPointToNode, 1);
		FmlObjectHandle fmlLocalPointToNodeIndex2 = Fieldml_GetIndexEvaluator(fmlSession, fmlLocalPointToNode, 2);
		if (!(((fmlLocalPointToNodeIndex1 == fmlElementArgument) && (fmlLocalPointToNodeIndex2 == fmlLocalPointArgument)) ||
			((fmlLocalPointToNodeIndex2 == fmlElementArgument) && (fmlLocalPointToNodeIndex1 == fmlLocalPointArgument))))
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Evaluator %s evaluator %s cannot be interpreted as an element, local point to node map.",
				elementParametersName.c_str(), getName(fmlLocalPointToNode).c_str());
			return_code = 0;
		}
	}

	if (!return_code)
		return 0;

	// Structure now validated

	if (verbose)
	{
		display_message(INFORMATION_MESSAGE, "Read FieldML:  Interpreting evaluator %s as nodal/element interpolator using basis %s.\n",
			evaluatorName.c_str(), interpolator_name);
	}

	Cmiss_field_id local_point_to_node_field = getParameters(fmlLocalPointToNode);
	Cmiss_field_integer_parameters_id local_point_to_node = Cmiss_field_cast_integer_parameters(local_point_to_node_field);
	Cmiss_field_destroy(&local_point_to_node_field);
	Cmiss_ensemble_index_id index = Cmiss_field_integer_parameters_create_index(local_point_to_node);

	FmlObjectHandle fmlLocalPointType = Fieldml_GetValueType(fmlSession, fmlLocalPointArgument);
	int local_point_count = Fieldml_GetMemberCount(fmlSession, fmlLocalPointType);

	Cmiss_element_basis_id element_basis = Cmiss_field_module_create_element_basis(field_module, meshDimension, libraryBases[basis_index].functionType[0]);
	if (!libraryBases[basis_index].homogeneous)
	{
		for (int dimension = 2; dimension <= meshDimension; dimension++)
		{
			Cmiss_element_basis_set_function_type(element_basis, dimension,
				libraryBases[basis_index].functionType[dimension - 1]);
		}
	}
	int basis_number_of_nodes = Cmiss_element_basis_get_number_of_nodes(element_basis);
	ElementFieldComponent *component = new ElementFieldComponent(element_basis, local_point_to_node, index, local_point_count, libraryBases[basis_index].swizzle);
	if (local_point_to_node && index && local_point_count && (local_point_count == basis_number_of_nodes))
	{
		componentMap[fmlEvaluator] = component;
	}
	else
	{
		if (local_point_count != basis_number_of_nodes)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Evaluator %s basis %s requires %d DOFs which does not match number of local points (%d)",
				evaluatorName.c_str(), interpolator_name, basis_number_of_nodes, local_point_count);
		}
		delete component;
		component = 0;
	}
	return component;
}

int FieldMLReader::readField(FmlObjectHandle fmlFieldEvaluator,
	std::vector<FmlObjectHandle> &fmlComponentEvaluators,
	FmlObjectHandle fmlNodeEnsembleType, FmlObjectHandle fmlNodeParameters,
	FmlObjectHandle fmlNodeParametersArgument, FmlObjectHandle fmlNodeArgument,
	FmlObjectHandle fmlElementArgument)
{
	int return_code = 1;
	const int componentCount = fmlComponentEvaluators.size();

	std::string fieldName = getName(fmlFieldEvaluator);

	if (verbose)
	{
		display_message(INFORMATION_MESSAGE, "\n==> Defining field from evaluator %s, components = %d\n",
			fieldName.c_str(), componentCount);
	}

	Cmiss_field_id field = Cmiss_field_module_create_finite_element(field_module, componentCount);
	Cmiss_field_set_name(field, fieldName.c_str());
	Cmiss_field_set_attribute_integer(field, CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 1);
	if ((componentCount >= meshDimension) && (componentCount <= 3))
	{
		// overzealous to help ensure there is at least one 'coordinate' field to define faces
		Cmiss_field_set_attribute_integer(field, CMISS_FIELD_ATTRIBUTE_IS_COORDINATE, 1);
	}

	// create nodes and set node parameters

	Cmiss_nodeset_id nodes = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");
	Cmiss_field_ensemble_id nodesEnsemble = getEnsemble(fmlNodeEnsembleType);
	if (fmlNodesType == FML_INVALID_OBJECT_HANDLE)
	{
		fmlNodesType = fmlNodeEnsembleType;
		// create the nodes
		Cmiss_node_template_id node_template = Cmiss_nodeset_create_node_template(nodes);
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
			fieldName.c_str(), getName(fmlNodeParameters).c_str());
		return_code = 0;
	}
	else
	{
		Cmiss_node_template_id node_template = Cmiss_nodeset_create_node_template(nodes);
		Cmiss_node_template_define_field(node_template, field);
		return_code = 1;
		Cmiss_ensemble_index_id index = Cmiss_field_real_parameters_create_index(node_parameters);
		// GRC inefficient to iterate over sparse parameters this way
		Cmiss_ensemble_iterator_id nodesIterator = Cmiss_field_ensemble_get_first_entry(nodesEnsemble);
		double *values = new double[componentCount];
		int *valueExists = new int[componentCount];
		Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
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
					Cmiss_field_cache_set_node(field_cache, node);
					Cmiss_field_assign_real(field, field_cache, componentCount, values);
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
		Cmiss_field_cache_destroy(&field_cache);
		delete[] valueExists;
		delete[] values;
		Cmiss_ensemble_iterator_destroy(&nodesIterator);
		Cmiss_field_real_parameters_destroy(&node_parameters);
		Cmiss_ensemble_index_destroy(&index);
		Cmiss_node_template_destroy(&node_template);
	}

	// define element fields

	Cmiss_mesh_id mesh =
		Cmiss_field_module_find_mesh_by_dimension(field_module, meshDimension);
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
			FmlObjectHandle fmlElementEvaluator = Fieldml_GetElementEvaluator(fmlSession, fmlComponentEvaluators[ic],
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
			element_template = Cmiss_mesh_create_element_template(mesh);
			// do not want to override shape of existing elements:
			Cmiss_element_template_set_shape_type(element_template, CMISS_ELEMENT_SHAPE_TYPE_INVALID);
			int total_local_point_count = 0;
			for (int ic = 0; ic < componentCount; ic++)
			{
				components[ic] = getElementFieldComponent(mesh, fmlElementEvaluators[ic],
					fmlNodeParametersArgument, fmlNodeArgument, fmlElementArgument);
				if (!components[ic])
				{
					display_message(ERROR_MESSAGE, "Read FieldML:  Aggregate %s component %d element %d evaluator %s does not reference a supported basis function or mapping",
						fieldName.c_str(), ic + 1, elementIdentifier, getName(fmlElementEvaluators[ic]).c_str());
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
					const int *swizzle = components[ic]->swizzle;
					for (int i = 0; i < components[ic]->local_point_count; i++)
					{
						components[ic]->local_point_indexes[i] = total_local_point_count + i + 1;
						if (swizzle)
						{
							components[ic]->swizzled_local_point_indexes[i] = total_local_point_count + swizzle[i];
						}
						else
						{
							components[ic]->swizzled_local_point_indexes[i] = components[ic]->local_point_indexes[i];
						}
					}
					total_local_point_count += components[ic]->local_point_count;
					Cmiss_element_template_set_number_of_nodes(element_template, total_local_point_count);
				}
				if (!Cmiss_element_template_define_field_simple_nodal(element_template, field,
					/*component*/ic + 1, components[ic]->element_basis, components[ic]->local_point_count,
					components[ic]->local_point_indexes))
				{
					return_code = 0;
					break;
				}
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
						char *local_point_to_node_name = Cmiss_field_get_name(
							Cmiss_field_integer_parameters_base_cast(component->local_point_to_node));
						display_message(ERROR_MESSAGE, "Read FieldML:  Cannot find node %d for element %d local point %d in local point to node map %s",
							component->node_identifiers[i], elementIdentifier, i + 1, local_point_to_node_name);
						DEALLOCATE(local_point_to_node_name);
						return_code = 0;
						break;
					}
					Cmiss_element_template_set_node(element_template, component->swizzled_local_point_indexes[i], node);
					Cmiss_node_destroy(&node);
				}
			}
		}
		if (return_code)
		{
			Cmiss_element_id element = Cmiss_mesh_find_element_by_identifier(mesh, elementIdentifier);
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
	Cmiss_field_ensemble_destroy(&elementsEnsemble);
	Cmiss_field_ensemble_destroy(&nodesEnsemble);
	if (element_template)
		Cmiss_element_template_destroy(&element_template);
	Cmiss_mesh_destroy(&mesh);
	Cmiss_nodeset_destroy(&nodes);
	Cmiss_field_destroy(&field);

	return return_code;
}

/** continuous-valued aggregates of piecewise varying with mesh elements are
 * interpreted as vector-valued finite element fields */
int FieldMLReader::readAggregateFields()
{
	const int aggregateCount = Fieldml_GetObjectCount(fmlSession, FHT_AGGREGATE_EVALUATOR);
	int return_code = 1;
	for (int aggregateIndex = 1; (aggregateIndex <= aggregateCount) && return_code; aggregateIndex++)
	{
		FmlObjectHandle fmlAggregate = Fieldml_GetObject(fmlSession, FHT_AGGREGATE_EVALUATOR, aggregateIndex);
		std::string fieldName = getName(fmlAggregate);

		FmlObjectHandle fmlValueType = Fieldml_GetValueType(fmlSession, fmlAggregate);
		if (FHT_CONTINUOUS_TYPE != Fieldml_GetObjectType(fmlSession, fmlValueType))
		{
			display_message(WARNING_MESSAGE, "Read FieldML:  Ignore aggregate %s as not continuous type\n", fieldName.c_str());
			continue;
		}
		FmlObjectHandle fmlValueTypeComponentEnsembleType = Fieldml_GetTypeComponentEnsemble(fmlSession, fmlValueType);
		const int componentCount = (fmlValueTypeComponentEnsembleType == FML_INVALID_OBJECT_HANDLE) ? 1 :
			Fieldml_GetMemberCount(fmlSession, fmlValueTypeComponentEnsembleType);

		// check aggregate components are piecewise over elements

		bool piecewiseOverElements = true;
		std::vector<FmlObjectHandle> fmlComponentEvaluators(componentCount, FML_INVALID_OBJECT_HANDLE);

		FmlObjectHandle fmlElementArgument = FML_INVALID_OBJECT_HANDLE;
		for (int componentIndex = 1; componentIndex <= componentCount; componentIndex++)
		{
			// Component ensemble identifiers are always 1..N:
			int componentIdentifier = componentIndex;
			int ic = componentIndex - 1;
			fmlComponentEvaluators[ic] = Fieldml_GetElementEvaluator(fmlSession, fmlAggregate, componentIdentifier, /*allowDefault*/1);
			if (fmlComponentEvaluators[ic] == FML_INVALID_OBJECT_HANDLE)
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  Aggregate %s component %d evaluator is missing", fieldName.c_str(), componentIndex);
				return_code = 0;
				break;
			}
			FmlObjectHandle fmlIndexEvaluator = FML_INVALID_OBJECT_HANDLE;
			if (FHT_PIECEWISE_EVALUATOR == Fieldml_GetObjectType(fmlSession, fmlComponentEvaluators[ic]))
			{
				fmlIndexEvaluator = Fieldml_GetIndexEvaluator(fmlSession, fmlComponentEvaluators[ic], /*evaluatorIndex*/1);
			}
			if (fmlElementArgument == FML_INVALID_OBJECT_HANDLE)
			{
				fmlElementArgument = fmlIndexEvaluator;
			}
			if ((FHT_PIECEWISE_EVALUATOR != Fieldml_GetObjectType(fmlSession, fmlComponentEvaluators[ic])) ||
				(FHT_CONTINUOUS_TYPE != Fieldml_GetObjectType(fmlSession, Fieldml_GetValueType(fmlSession, fmlComponentEvaluators[ic]))) ||
				(Fieldml_GetValueType(fmlSession, fmlIndexEvaluator) != fmlElementsType))
			{
				if (verbose)
				{
					display_message(WARNING_MESSAGE, "Read FieldML:  Aggregate %s component %d is not piecewise indexed by mesh elements ensemble",
						fieldName.c_str(), componentIndex);
				}
				piecewiseOverElements = false;
				break;
			}
			else if ((Fieldml_GetObjectType(fmlSession, fmlIndexEvaluator) != FHT_ARGUMENT_EVALUATOR) ||
				(fmlIndexEvaluator != fmlElementArgument))
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  Aggregate %s components must use same argument evaluator for element index",
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

		int bindCount = Fieldml_GetBindCount(fmlSession, fmlAggregate);
		if (1 != bindCount)
		{
			if (verbose)
			{
				display_message(WARNING_MESSAGE,
					"Read FieldML:  Aggregate %s does not have exactly 1 binding (for nodal parameters). Skipping.",
					fieldName.c_str());
			}
			continue;
		}
		FmlObjectHandle fmlNodeParametersArgument = Fieldml_GetBindArgument(fmlSession, fmlAggregate, 1);
		FmlObjectHandle fmlNodeParameters = Fieldml_GetBindEvaluator(fmlSession, fmlAggregate, 1);
		if ((Fieldml_GetObjectType(fmlSession, fmlNodeParameters) != FHT_PARAMETER_EVALUATOR) ||
			(Fieldml_GetObjectType(fmlSession, Fieldml_GetValueType(fmlSession, fmlNodeParameters)) != FHT_CONTINUOUS_TYPE))
		{
			if (verbose)
			{
				display_message(WARNING_MESSAGE,
					"Read FieldML:  Aggregate %s does not bind a continuous parameters source. Skipping.",
					fieldName.c_str());
			}
			continue;
		}
		int nodeParametersIndexCount = Fieldml_GetIndexEvaluatorCount(fmlSession, fmlNodeParameters);
		FmlObjectHandle fmlNodeParametersIndex1 = FML_INVALID_OBJECT_HANDLE;
		FmlObjectHandle fmlNodeParametersIndex2 = FML_INVALID_OBJECT_HANDLE;
		if ((2 != nodeParametersIndexCount) ||
			((fmlNodeParametersIndex1 = Fieldml_GetIndexEvaluator(fmlSession, fmlNodeParameters, 1)) == FML_INVALID_OBJECT_HANDLE) ||
			((fmlNodeParametersIndex2 = Fieldml_GetIndexEvaluator(fmlSession, fmlNodeParameters, 2)) == FML_INVALID_OBJECT_HANDLE))
		{
			if (verbose)
			{
				display_message(WARNING_MESSAGE,
					"Read FieldML:  Aggregate %s bound source %s has %d indexes, 2 are expected for nodal parameters. Skipping.",
					fieldName.c_str(), getName(fmlNodeParameters).c_str(), nodeParametersIndexCount);
			}
			continue;
		}
		FmlObjectHandle fmlNodeParametersIndex1Type = Fieldml_GetValueType(fmlSession, fmlNodeParametersIndex1);
		FmlObjectHandle fmlNodeParametersIndex2Type = Fieldml_GetValueType(fmlSession, fmlNodeParametersIndex2);
		FmlObjectHandle fmlNodeEnsembleType = FML_INVALID_OBJECT_HANDLE;
		FmlObjectHandle fmlNodeArgument = FML_INVALID_OBJECT_HANDLE;
		if (fmlNodeParametersIndex1Type == fmlValueTypeComponentEnsembleType)
		{
			fmlNodeArgument = fmlNodeParametersIndex2;
			fmlNodeEnsembleType = fmlNodeParametersIndex2Type;
		}
		else if (fmlNodeParametersIndex2Type == fmlValueTypeComponentEnsembleType)
		{
			fmlNodeArgument = fmlNodeParametersIndex1;
			fmlNodeEnsembleType = fmlNodeParametersIndex1Type;
		}
		if (fmlNodeEnsembleType == FML_INVALID_OBJECT_HANDLE)
		{
			display_message(ERROR_MESSAGE,
				"Read FieldML:  Aggregate %s binds parameters %s that do not vary with component.",
				fieldName.c_str(), getName(fmlNodeParameters).c_str());
			break;
		}
		if ((fmlNodesType != FML_INVALID_OBJECT_HANDLE) &&
			(fmlNodeEnsembleType != fmlNodesType))
		{
			display_message(ERROR_MESSAGE,
				"Read FieldML:  Aggregate %s binds parameters %s indexed by an unknown ensemble %s not matching 'nodes' %s used for other fields.",
				fieldName.c_str(), getName(fmlNodeParameters).c_str(), getName(fmlNodeEnsembleType).c_str(), getName(fmlNodesType).c_str());
			break;
		}

		return_code = readField(fmlAggregate, fmlComponentEvaluators, fmlNodeEnsembleType, fmlNodeParameters,
			fmlNodeParametersArgument, fmlNodeArgument, fmlElementArgument);
	}
	return return_code;
}

/** continuous-valued references to piecewise varying with mesh elements are
 * interpreted as scalar-valued finite element fields */
int FieldMLReader::readReferenceFields()
{
	const int referenceCount = Fieldml_GetObjectCount(fmlSession, FHT_REFERENCE_EVALUATOR);
	int return_code = 1;
	for (int referenceIndex = 1; (referenceIndex <= referenceCount) && return_code; referenceIndex++)
	{
		FmlObjectHandle fmlReference = Fieldml_GetObject(fmlSession, FHT_REFERENCE_EVALUATOR, referenceIndex);
		std::string fieldName = getName(fmlReference);

		FmlObjectHandle fmlValueType = Fieldml_GetValueType(fmlSession, fmlReference);
		if (FHT_CONTINUOUS_TYPE != Fieldml_GetObjectType(fmlSession, fmlValueType))
		{
			//display_message(WARNING_MESSAGE, "Read FieldML:  Ignore reference %s as not continuous type\n", fieldName.c_str());
			continue;
		}

		// check reference evaluator is piecewise over elements
		// GRC should check it is scalar too

		bool piecewiseOverElements = true;

		FmlObjectHandle fmlElementArgument = FML_INVALID_OBJECT_HANDLE;

		FmlObjectHandle fmlComponentEvaluator = Fieldml_GetReferenceSourceEvaluator(fmlSession, fmlReference);
		if (fmlComponentEvaluator == FML_INVALID_OBJECT_HANDLE)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Reference %s remove evaluator is missing", fieldName.c_str());
			return_code = 0;
			break;
		}

		FmlObjectHandle fmlIndexEvaluator = FML_INVALID_OBJECT_HANDLE;
		if (FHT_PIECEWISE_EVALUATOR == Fieldml_GetObjectType(fmlSession, fmlComponentEvaluator))
		{
			fmlIndexEvaluator = Fieldml_GetIndexEvaluator(fmlSession, fmlComponentEvaluator, /*evaluatorIndex*/1);
		}
		if (fmlElementArgument == FML_INVALID_OBJECT_HANDLE)
		{
			fmlElementArgument = fmlIndexEvaluator;
		}
		if ((FHT_PIECEWISE_EVALUATOR != Fieldml_GetObjectType(fmlSession, fmlComponentEvaluator)) ||
			(FHT_CONTINUOUS_TYPE != Fieldml_GetObjectType(fmlSession, Fieldml_GetValueType(fmlSession, fmlComponentEvaluator))) ||
			(Fieldml_GetValueType(fmlSession, fmlIndexEvaluator) != fmlElementsType))
		{
			if (verbose)
			{
				display_message(WARNING_MESSAGE, "Read FieldML:  Reference %s remote evaluator %s is not piecewise indexed by mesh elements ensemble",
					fieldName.c_str(), getName(fmlComponentEvaluator).c_str());
			}
			piecewiseOverElements = false;
		}
		else if (Fieldml_GetObjectType(fmlSession, fmlIndexEvaluator) != FHT_ARGUMENT_EVALUATOR)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Reference %s does not use an argument evaluator for element index",
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

		int bindCount = Fieldml_GetBindCount(fmlSession, fmlReference);
		FmlObjectHandle fmlNodeParametersArgument = Fieldml_GetBindArgument(fmlSession, fmlReference, 1);
		FmlObjectHandle fmlNodeParameters = Fieldml_GetBindEvaluator(fmlSession, fmlReference, 1);
		int nodeParametersIndexCount = Fieldml_GetIndexEvaluatorCount(fmlSession, fmlNodeParameters);
		FmlObjectHandle fmlNodeParametersIndex1 = Fieldml_GetIndexEvaluator(fmlSession, fmlNodeParameters, 1);
		if ((1 != bindCount) ||
			(Fieldml_GetObjectType(fmlSession, fmlNodeParameters) != FHT_PARAMETER_EVALUATOR) ||
			(Fieldml_GetObjectType(fmlSession, Fieldml_GetValueType(fmlSession, fmlNodeParameters)) != FHT_CONTINUOUS_TYPE) ||
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
		FmlObjectHandle fmlNodeParametersIndex1Type = Fieldml_GetValueType(fmlSession, fmlNodeParametersIndex1);
		FmlObjectHandle fmlNodeArgument = fmlNodeParametersIndex1;
		FmlObjectHandle fmlNodeEnsembleType = fmlNodeParametersIndex1Type;
		if ((fmlNodesType != FML_INVALID_OBJECT_HANDLE) &&
			(fmlNodeEnsembleType != fmlNodesType))
		{
			display_message(ERROR_MESSAGE,
				"Read FieldML:  Reference %s binds parameters %s indexed by an unknown ensemble %s not matching 'nodes' %s used for other fields.",
				fieldName.c_str(), getName(fmlNodeParameters).c_str(), getName(fmlNodeEnsembleType).c_str(), getName(fmlNodesType).c_str());
			break;
		}

		std::vector<FmlObjectHandle> fmlComponentEvaluators(1, fmlComponentEvaluator);
		return_code = readField(fmlReference, fmlComponentEvaluators, fmlNodeEnsembleType, fmlNodeParameters,
			fmlNodeParametersArgument, fmlNodeArgument, fmlElementArgument);
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
	if (fmlSession == (const FmlSessionHandle)FML_INVALID_HANDLE)
	{
		display_message(ERROR_MESSAGE, "Read FieldML: could not parse file %s", filename);
		return_code = 0;
	}
	else
	{
		int parseErrorCount = Fieldml_GetErrorCount(fmlSession);
		return_code = (parseErrorCount == 0);
		for (int i = 1; i <= parseErrorCount; i++)
		{
			char *error_string = Fieldml_GetError(fmlSession, i);
			display_message(ERROR_MESSAGE, "FieldML Parse error: %s", error_string);
			Fieldml_FreeString(error_string);
		}
	}
	if (!return_code)
		return 0;
	Cmiss_region_begin_change(region);
	return_code = return_code && readMeshes();
	return_code = return_code && readAggregateFields();
	return_code = return_code && readReferenceFields();
	Cmiss_region_end_change(region);
	return return_code;
}

} // anonymous namespace

int is_FieldML_file(const char *filename)
{
	int return_code = 0;
	FILE *stream = fopen(filename, "r");
	if (stream)
	{
		char block[200];
		size_t size = fread((void *)block, sizeof(char), sizeof(block), stream);
		if (size > 0)
		{
			block[size-1] = '\0';
			if (NULL != strstr(block, "<Fieldml"))
			{
				return_code = 1;
			}
		}
		fclose(stream);
	}
	return return_code;
}

int parse_fieldml_file(struct Cmiss_region *region, const char *filename)
{
	FieldMLReader fmlReader(region, filename);
	return fmlReader.parse();
}
