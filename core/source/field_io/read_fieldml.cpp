/**
 * FILE : read_fieldml.cpp
 * 
 * FieldML 0.5 model reader implementation.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <cstring>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "zinc/element.h"
#include "zinc/field.h"
#include "zinc/fieldcache.h"
#include "zinc/fieldmodule.h"
#include "zinc/fieldfiniteelement.h"
#include "zinc/node.h"
#include "zinc/region.h"
#include "zinc/status.h"
#include "field_io/fieldml_common.hpp"
#include "field_io/read_fieldml.hpp"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"
#include "general/refcounted.hpp"
#include "FieldmlIoApi.h"

namespace {

const char *libraryChartArgumentNames[] =
{
	0,
	"chart.1d.argument",
	"chart.2d.argument",
	"chart.3d.argument"
};

struct ElementFieldComponent
{
	cmzn_elementbasis_id element_basis;
	HDsMapInt local_point_to_node;
	HDsMapIndexing indexing;
	int local_point_count;
	const int *swizzle;
	int *local_point_indexes;
	int *swizzled_local_point_indexes;
	int *node_identifiers;

	ElementFieldComponent(cmzn_elementbasis_id element_basis,
			HDsMapInt local_point_to_nodeIn,
			HDsMapIndexing indexingIn, int local_point_countIn,
			const int *swizzleIn) :
		element_basis(element_basis),
		local_point_to_node(local_point_to_nodeIn),
		indexing(indexingIn),
		local_point_count(local_point_countIn),
		swizzle(swizzleIn),
		local_point_indexes(new int[local_point_count]),
		swizzled_local_point_indexes(new int[local_point_count]),
		node_identifiers(new int[local_point_count])
	{
	}

	~ElementFieldComponent()
	{
		cmzn_elementbasis_destroy(&element_basis);
		delete[] local_point_indexes;
		delete[] swizzled_local_point_indexes;
		delete[] node_identifiers;
	}
};

typedef std::map<FmlObjectHandle,ElementFieldComponent*> EvaluatorElementFieldComponentMap;
typedef std::map<FmlObjectHandle,HDsLabels> FmlObjectLabelsMap;
typedef std::map<FmlObjectHandle,HDsMapInt> FmlObjectIntParametersMap;
typedef std::map<FmlObjectHandle,HDsMapDouble> FmlObjectDoubleParametersMap;

class FieldMLReader
{
	cmzn_region *region;
	cmzn_fieldmodule_id field_module;
	const char *filename;
	FmlSessionHandle fmlSession;
	int meshDimension;
	FmlObjectHandle fmlNodesType;
	FmlObjectHandle fmlElementsType;
	EvaluatorElementFieldComponentMap componentMap;
	FmlObjectLabelsMap labelsMap;
	FmlObjectIntParametersMap intParametersMap;
	FmlObjectDoubleParametersMap doubleParametersMap;
	bool verbose;
	int nameBufferLength;
	char *nameBuffer; // buffer for reading object names into
	std::set<FmlObjectHandle> processedObjects;

public:
	FieldMLReader(struct cmzn_region *region, const char *filename) :
		region(cmzn_region_access(region)),
		field_module(cmzn_region_get_fieldmodule(region)),
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
		cmzn_fieldmodule_destroy(&field_module);
		cmzn_region_destroy(&region);
		delete[] nameBuffer;
	}

	/** @return  1 on success, 0 on failure */
	int parse();

private:

	std::string getName(FmlObjectHandle fmlObjectHandle)
	{
		if (FML_INVALID_HANDLE == fmlObjectHandle)
			return std::string("INVALID");
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
		if (FML_INVALID_HANDLE == fmlObjectHandle)
			return std::string("INVALID");
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

	DsLabels *getLabelsForEnsemble(FmlObjectHandle fmlEnsemble);

	template <typename VALUETYPE> int readParametersArray(FmlObjectHandle fmlParameters,
		DsMap<VALUETYPE>& parameters);

	DsMap<int> *getEnsembleParameters(FmlObjectHandle fmlParameters);
	DsMap<double> *getContinuousParameters(FmlObjectHandle fmlParameters);

	int readMeshes();

	ElementFieldComponent *getElementFieldComponent(cmzn_mesh_id mesh,
		FmlObjectHandle fmlEvaluator, FmlObjectHandle fmlNodeParametersArgument,
		FmlObjectHandle fmlNodeArgument, FmlObjectHandle fmlElementArgument);

	int readField(FmlObjectHandle fmlFieldEvaluator,
		std::vector<FmlObjectHandle> &fmlComponentEvaluators,
		FmlObjectHandle fmlNodeEnsembleType, FmlObjectHandle fmlNodeParameters,
		FmlObjectHandle fmlNodeParametersArgument, FmlObjectHandle fmlNodeArgument,
		FmlObjectHandle fmlElementArgument);

	bool evaluatorIsScalarContinuousPiecewiseOverElements(FmlObjectHandle fmlEvaluator,
		FmlObjectHandle &fmlElementArgument);

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

/**
 * Gets handle to DsLabels matching fmlEnsembleType. Definition is read from
 * FieldML document only when first requested.
 * ???GRC Assumes there is only one argument for each type; consider having a
 * separate class to represent each argument.
 *
 * @param fmlEnsembleType  Handle of type FHT_ENSEMBLE_TYPE.
 * @return  Accessed pointer to labels, or 0 on failure */
DsLabels *FieldMLReader::getLabelsForEnsemble(FmlObjectHandle fmlEnsembleType)
{
	FmlObjectLabelsMap::iterator iterator = this->labelsMap.find(fmlEnsembleType);
	if (iterator != labelsMap.end())
		return cmzn::Access(cmzn::GetImpl(iterator->second));

	std::string name = this->getName(fmlEnsembleType);
	if (name.length()==0)
	{
		// GRC workaround for ensemble types that have not been imported
		name = "NONIMPORTED_";
		name.append(getDeclaredName(fmlEnsembleType));
	}
	if (Fieldml_GetObjectType(fmlSession, fmlEnsembleType) != FHT_ENSEMBLE_TYPE)
	{
		display_message(ERROR_MESSAGE, "FieldMLReader::getLabelsForEnsemble:  Argument %s is not ensemble type", name.c_str());
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
	HDsLabels labels(new DsLabels());
	labels->setName(name);
	this->setProcessed(fmlEnsembleType);
	this->labelsMap[fmlEnsembleType] = labels;

	int return_code = 1;
	if (FML_ENSEMBLE_MEMBER_RANGE == fmlEnsembleMembersType)
	{
		FmlEnsembleValue min = Fieldml_GetEnsembleMembersMin(fmlSession, fmlEnsembleType);
		FmlEnsembleValue max = Fieldml_GetEnsembleMembersMax(fmlSession, fmlEnsembleType);
		int stride = Fieldml_GetEnsembleMembersStride(fmlSession, fmlEnsembleType);
		int result = labels->addLabelsRange(min, max, stride);
		if (result != CMZN_OK)
			return 0;
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
					{
						int result = labels->findOrCreateLabel(rangeData[i]);
						if (result != CMZN_OK)
							return_code = 0;
					} break;
					case FML_ENSEMBLE_MEMBER_RANGE_DATA:
					{
						int result = labels->addLabelsRange(/*min*/rangeData[i*2], /*max*/rangeData[i*2 + 1]);
						if (result != CMZN_OK)
							return_code = 0;
					} break;
					case FML_ENSEMBLE_MEMBER_STRIDE_RANGE_DATA:
					{
						int result = labels->addLabelsRange(/*min*/rangeData[i*3], /*max*/rangeData[i*3 + 1], /*stride*/rangeData[i*3 + 2]);
						if (result != CMZN_OK)
							return_code = 0;
					} break;
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
				if (return_code && (labels->getSize() != memberCount))
				{
					display_message(ERROR_MESSAGE, "Read FieldML:  Ensemble type %s lists member count %d, actual number in data source is %d",
						name.c_str(), memberCount, labels->getSize());
					return_code = 0;
				}
			}
			delete[] rangeData;
			Fieldml_CloseReader(fmlReader);
		}
	}
	if (!return_code)
		return 0;
	return cmzn::Access(cmzn::GetImpl(labels));
}

// template and full specialisations to read different types with template
template <typename VALUETYPE> FmlIoErrorNumber FieldML_ReadSlab(
	FmlReaderHandle readerHandle, const int *offsets, const int *sizes, VALUETYPE *valueBuffer);

template <> inline FmlIoErrorNumber FieldML_ReadSlab(
	FmlReaderHandle readerHandle, const int *offsets, const int *sizes, double *valueBuffer)
{
	return Fieldml_ReadDoubleSlab(readerHandle, offsets, sizes, valueBuffer);
}

template <> inline FmlIoErrorNumber FieldML_ReadSlab(
	FmlReaderHandle readerHandle, const int *offsets, const int *sizes, int *valueBuffer)
{
	return Fieldml_ReadIntSlab(readerHandle, offsets, sizes, valueBuffer);
}

// TODO : Support order
// ???GRC can order cover subset of ensemble?
template <typename VALUETYPE> int FieldMLReader::readParametersArray(FmlObjectHandle fmlParameters,
	DsMap<VALUETYPE>& parameters)
{
	std::string name = this->getName(fmlParameters);
	FieldmlDataDescriptionType dataDescription = Fieldml_GetParameterDataDescription(fmlSession, fmlParameters);
	if ((dataDescription != FML_DATA_DESCRIPTION_DENSE_ARRAY) &&
		(dataDescription != FML_DATA_DESCRIPTION_DOK_ARRAY))
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Unknown data description for parameters %s; must be dense array or DOK array", name.c_str());
		return 0;
	}

	int return_code = 1;
	const int recordIndexCount = (dataDescription == FML_DATA_DESCRIPTION_DOK_ARRAY) ? 1 : 0;

	const int denseIndexCount = Fieldml_GetParameterIndexCount(fmlSession, fmlParameters, /*isSparse*/0);

	std::vector<HDsLabels> denseIndexLabels(denseIndexCount);
	const int expectedArrayRank = (dataDescription == FML_DATA_DESCRIPTION_DOK_ARRAY) ? 2 : denseIndexCount;
	int *arrayRawSizes = new int[expectedArrayRank];
	int *arrayOffsets = new int[expectedArrayRank];
	int *arraySizes = new int[expectedArrayRank];
	int arrayRank = 0;

	FmlObjectHandle fmlDataSource = Fieldml_GetDataSource(fmlSession, fmlParameters);
	if (fmlDataSource == FML_INVALID_HANDLE)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Could not get data source for parameters %s", name.c_str());
		return_code = 0;
	}
	else if (FML_DATA_SOURCE_ARRAY != Fieldml_GetDataSourceType(fmlSession, fmlDataSource))
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Only supports ArrayDataSource for parameters %s", name.c_str());
		return_code = 0;
	}
	else if ((arrayRank = Fieldml_GetArrayDataSourceRank(fmlSession, fmlDataSource)) != expectedArrayRank)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Data source %s has invalid rank for parameters %s",
			getName(fmlDataSource).c_str(), name.c_str());
		return_code = 0;
	}
	else if ((arrayRank > 0) && (
		(FML_ERR_NO_ERROR != Fieldml_GetArrayDataSourceRawSizes(fmlSession, fmlDataSource, arrayRawSizes)) ||
		(FML_ERR_NO_ERROR != Fieldml_GetArrayDataSourceOffsets(fmlSession, fmlDataSource, arrayOffsets)) ||
		(FML_ERR_NO_ERROR != Fieldml_GetArrayDataSourceSizes(fmlSession, fmlDataSource, arraySizes))))
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Failed to get array sizes of data source %s for parameters %s",
			getName(fmlDataSource).c_str(), name.c_str());
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
					getName(fmlDenseIndexEvaluator).c_str(), name.c_str());
				return_code = 0;
				break;
			}
			denseIndexLabels[i] = HDsLabels(this->getLabelsForEnsemble(fmlDenseIndexType));
			if (!denseIndexLabels[i])
			{
				return_code = 0;
				break;
			}
			FmlObjectHandle fmlOrderDataSource = Fieldml_GetParameterIndexOrder(fmlSession, fmlParameters, i + 1);
			if (fmlOrderDataSource != FML_INVALID_HANDLE)
			{
				display_message(WARNING_MESSAGE, "Read FieldML:  Parameters %s dense index %s specifies order. This is not yet supported; results will be incorrect.",
					name.c_str(), getName(fmlDenseIndexEvaluator).c_str());
			}
		}
	}

	int sparseIndexCount = 0;
	std::vector<HDsLabels> sparseIndexLabels;
	FmlObjectHandle fmlKeyDataSource = FML_INVALID_HANDLE;
	int keyArrayRawSizes[2];
	int keyArraySizes[2];
	int keyArrayOffsets[2];
	if (dataDescription == FML_DATA_DESCRIPTION_DOK_ARRAY)
	{
		sparseIndexCount = Fieldml_GetParameterIndexCount(fmlSession, fmlParameters, /*isSparse*/1);
		fmlKeyDataSource = Fieldml_GetKeyDataSource(fmlSession, fmlParameters);
		if (fmlKeyDataSource == FML_INVALID_HANDLE)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Could not get key data source for parameters %s", name.c_str());
			return_code = 0;
		}
		else if (FML_DATA_SOURCE_ARRAY != Fieldml_GetDataSourceType(fmlSession, fmlKeyDataSource))
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Only supports ArrayDataSource for keys for parameters %s", name.c_str());
			return_code = 0;
		}
		else if (Fieldml_GetArrayDataSourceRank(fmlSession, fmlKeyDataSource) != 2)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Key data source %s for parameters %s must be rank 2",
				getName(fmlKeyDataSource).c_str(), name.c_str());
			return_code = 0;
		}
		else if ((FML_ERR_NO_ERROR != Fieldml_GetArrayDataSourceRawSizes(fmlSession, fmlKeyDataSource, keyArrayRawSizes)) ||
			(FML_ERR_NO_ERROR != Fieldml_GetArrayDataSourceSizes(fmlSession, fmlKeyDataSource, keyArraySizes)) ||
			(FML_ERR_NO_ERROR != Fieldml_GetArrayDataSourceOffsets(fmlSession, fmlKeyDataSource, keyArrayOffsets)))
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Failed to get array sizes for key data source %s for parameters %s",
				getName(fmlKeyDataSource).c_str(), name.c_str());
			return_code = 0;
		}
		else
		{
			// zero array size means use raw size less offset
			for (int r = 0; r < 2; ++r)
			{
				if (keyArraySizes[r] == 0)
					keyArraySizes[r] = keyArrayRawSizes[r] - keyArrayOffsets[r];
			}
			if ((keyArraySizes[0] != arraySizes[0]) || (keyArraySizes[1] != sparseIndexCount))
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  Invalid array sizes for key data source %s for parameters %s",
					getName(fmlKeyDataSource).c_str(), name.c_str());
				return_code = 0;
			}
		}
		if (return_code)
		{
			for (int i = 0; i < sparseIndexCount; i++)
			{
				FmlObjectHandle fmlSparseIndexEvaluator = Fieldml_GetParameterIndexEvaluator(fmlSession, fmlParameters, i + 1, /*isSparse*/1);
				FmlObjectHandle fmlSparseIndexType = Fieldml_GetValueType(fmlSession, fmlSparseIndexEvaluator);
				sparseIndexLabels.push_back(HDsLabels(getLabelsForEnsemble(fmlSparseIndexType)));
				if (!sparseIndexLabels[i])
				{
					return_code = 0;
					break;
				}
			}
		}
	}

	HDsMapIndexing indexing(parameters.createIndexing());

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
	VALUETYPE *valueBuffer = new VALUETYPE[valueBufferSize];
	int *keyBuffer = 0;
	if (return_code)
	{
		if (0 == valueBuffer)
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
				name.c_str(), getName(fmlDataSource).c_str());
			return_code = 0;
		}
		if (dataDescription == FML_DATA_DESCRIPTION_DOK_ARRAY)
		{
			fmlKeyReader = Fieldml_OpenReader(fmlSession, fmlKeyDataSource);
			if (fmlKeyReader == FML_INVALID_HANDLE)
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  Could not open reader for parameters %s key data source %s",
					name.c_str(), getName(fmlKeyDataSource).c_str());
				return_code = 0;
			}
		}
	}

	if (return_code)
	{
		FmlIoErrorNumber ioResult = FML_IOERR_NO_ERROR;
		ioResult = FieldML_ReadSlab(fmlReader, arrayOffsets, arraySizes, valueBuffer);
		if (ioResult != FML_IOERR_NO_ERROR)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Failed to read data source %s for parameters %s",
				getName(fmlDataSource).c_str(), name.c_str());
			return_code = 0;
		}
		if (dataDescription == FML_DATA_DESCRIPTION_DOK_ARRAY)
		{
			int keyArrayReadOffsets[2] = { 0, 0 };
			ioResult = Fieldml_ReadIntSlab(fmlKeyReader, keyArrayReadOffsets, keyArraySizes, keyBuffer);
			if (ioResult != FML_IOERR_NO_ERROR)
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  Failed to read key data source %s for parameters %s",
					getName(fmlKeyDataSource).c_str(), name.c_str());
				return_code = 0;
			}
		}
	}

	if (return_code)
	{
		const int recordCount = (dataDescription == FML_DATA_DESCRIPTION_DOK_ARRAY) ? arraySizes[0] : 1;
		for (int record = 0; (record < recordCount) && return_code; ++record)
		{
			for (int i = 0; i < sparseIndexCount; i++)
				indexing->setEntryIdentifier(*(sparseIndexLabels[i]), keyBuffer[record*sparseIndexCount + i]);
			return_code = parameters.setValues(*indexing, totalDenseSize, valueBuffer + record*totalDenseSize);
		}
	}

	if (dataDescription == FML_DATA_DESCRIPTION_DOK_ARRAY)
		Fieldml_CloseReader(fmlKeyReader);
	Fieldml_CloseReader(fmlReader);
	delete[] valueBuffer;
	delete[] keyBuffer;
	delete[] arraySizes;
	delete[] arrayOffsets;
	delete[] arrayRawSizes;
	return return_code;
}

/**
 * Returns integer map for supplied parameters, reading it from the data source
 * if encountered for the first time.
 *
 * @param fmlParameters  Handle of type FHT_PARAMETER_EVALUATOR.
 * @return  Accessed pointer to integer map, or 0 on failure */
DsMap<int> *FieldMLReader::getEnsembleParameters(FmlObjectHandle fmlParameters)
{
	FmlObjectIntParametersMap::iterator iter = this->intParametersMap.find(fmlParameters);
	if (iter != this->intParametersMap.end())
		return cmzn::Access(cmzn::GetImpl(iter->second));

	std::string name = getName(fmlParameters);
	if (Fieldml_GetObjectType(this->fmlSession, fmlParameters) != FHT_PARAMETER_EVALUATOR)
	{
		display_message(ERROR_MESSAGE, "FieldMLReader::getEnsembleParameters.  %s is not a parameter evaluator", name.c_str());
		return 0;
	}
	FmlObjectHandle fmlValueType = Fieldml_GetValueType(fmlSession, fmlParameters);
	FieldmlHandleType valueClass = Fieldml_GetObjectType(fmlSession, fmlValueType);
	if (valueClass != FHT_ENSEMBLE_TYPE)
	{
		display_message(ERROR_MESSAGE, "FieldMLReader::getEnsembleParameters.  %s is not ensemble-valued", name.c_str());
		return 0;
	}

	if (verbose)
		display_message(INFORMATION_MESSAGE, "Reading ensemble parameters %s\n", name.c_str());
	int indexCount = Fieldml_GetIndexEvaluatorCount(this->fmlSession, fmlParameters);
	std::vector<HDsLabels> indexingLabelsVector;
	int return_code = 1;
	for (int indexNumber = 1; indexNumber <= indexCount; ++indexNumber)
	{
		FmlObjectHandle fmlIndexEvaluator = Fieldml_GetIndexEvaluator(this->fmlSession, fmlParameters, indexNumber);
		FmlObjectHandle fmlEnsembleType = Fieldml_GetValueType(this->fmlSession, fmlIndexEvaluator);
		if ((FHT_ARGUMENT_EVALUATOR != Fieldml_GetObjectType(this->fmlSession, fmlIndexEvaluator)) ||
			(FHT_ENSEMBLE_TYPE != Fieldml_GetObjectType(this->fmlSession, fmlEnsembleType)))
		{
			display_message(WARNING_MESSAGE, "FieldMLReader::getEnsembleParameters:  Index %d (%s) of parameters %s is not an ensemble-valued argument evaluator",
				indexNumber, getName(fmlIndexEvaluator).c_str(), name.c_str());
			return_code = 0;
		}
		if (verbose)
			display_message(INFORMATION_MESSAGE, "  Index %d = %s\n", indexNumber, getName(fmlIndexEvaluator).c_str());
		indexingLabelsVector.push_back(HDsLabels(this->getLabelsForEnsemble(fmlEnsembleType)));
	}
	if (!return_code)
		return 0;
	DsMap<int> *parameters = DsMap<int>::create(indexingLabelsVector);
	parameters->setName(name);
	return_code = this->readParametersArray(fmlParameters, *parameters);
	this->setProcessed(fmlParameters);
	if (!return_code)
		cmzn::Deaccess(parameters);
	return parameters;
}

/**
 * Returns double map for supplied parameters, reading it from the data source
 * if encountered for the first time.
 *
 * @param fmlParameters  Handle of type FHT_PARAMETER_EVALUATOR.
 * @return  Accessed pointer to double map, or 0 on failure */
DsMap<double> *FieldMLReader::getContinuousParameters(FmlObjectHandle fmlParameters)
{
	FmlObjectDoubleParametersMap::iterator iter = this->doubleParametersMap.find(fmlParameters);
	if (iter != this->doubleParametersMap.end())
		return cmzn::Access(cmzn::GetImpl(iter->second));

	std::string name = getName(fmlParameters);
	if (Fieldml_GetObjectType(this->fmlSession, fmlParameters) != FHT_PARAMETER_EVALUATOR)
	{
		display_message(ERROR_MESSAGE, "FieldMLReader::getContinuousParameters.  %s is not a parameter evaluator", name.c_str());
		return 0;
	}
	FmlObjectHandle fmlValueType = Fieldml_GetValueType(fmlSession, fmlParameters);
	FieldmlHandleType valueClass = Fieldml_GetObjectType(fmlSession, fmlValueType);
	if (valueClass != FHT_CONTINUOUS_TYPE)
	{
		display_message(ERROR_MESSAGE, "FieldMLReader::getContinuousParameters.  %s is not continuous-valued", name.c_str());
		return 0;
	}
	FmlObjectHandle fmlValueTypeComponentEnsembleType = Fieldml_GetTypeComponentEnsemble(fmlSession, fmlValueType);
	if (fmlValueTypeComponentEnsembleType != FML_INVALID_OBJECT_HANDLE)
	{
		display_message(WARNING_MESSAGE, "FieldMLReader::getContinuousParameters:  Cannot read non-scalar parameters %s", name.c_str());
		return 0;
	}

	if (verbose)
		display_message(INFORMATION_MESSAGE, "Reading continuous parameters %s\n", name.c_str());
	int indexCount = Fieldml_GetIndexEvaluatorCount(this->fmlSession, fmlParameters);
	std::vector<HDsLabels> indexingLabelsVector;
	int return_code = 1;
	for (int indexNumber = 1; indexNumber <= indexCount; ++indexNumber)
	{
		FmlObjectHandle fmlIndexEvaluator = Fieldml_GetIndexEvaluator(this->fmlSession, fmlParameters, indexNumber);
		FmlObjectHandle fmlEnsembleType = Fieldml_GetValueType(this->fmlSession, fmlIndexEvaluator);
		if ((FHT_ARGUMENT_EVALUATOR != Fieldml_GetObjectType(this->fmlSession, fmlIndexEvaluator)) ||
			(FHT_ENSEMBLE_TYPE != Fieldml_GetObjectType(this->fmlSession, fmlEnsembleType)))
		{
			display_message(WARNING_MESSAGE, "FieldMLReader::getContinuousParameters:  Index %d (%s) of parameters %s is not an ensemble-valued argument evaluator",
				indexNumber, getName(fmlIndexEvaluator).c_str(), name.c_str());
			return_code = 0;
		}
		if (verbose)
			display_message(INFORMATION_MESSAGE, "  Index %d = %s\n", indexNumber, getName(fmlIndexEvaluator).c_str());
		indexingLabelsVector.push_back(HDsLabels(this->getLabelsForEnsemble(fmlEnsembleType)));
	}
	if (!return_code)
		return 0;
	DsMap<double> *parameters = DsMap<double>::create(indexingLabelsVector);
	parameters->setName(name);
	return_code = this->readParametersArray(fmlParameters, *parameters);
	this->setProcessed(fmlParameters);
	if (!return_code)
		cmzn::Deaccess(parameters);
	return parameters;
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
		HDsLabels elementsLabels(getLabelsForEnsemble(fmlElementsType));
		if (verbose)
		{
			display_message(INFORMATION_MESSAGE, "Defining %d elements from %s\n",
				elementsLabels->getSize(), getName(fmlElementsType).c_str());
		}

		// determine element shape mapping

		FmlObjectHandle fmlShapeEvaluator = Fieldml_GetMeshShapes(fmlSession, fmlMeshType);
		cmzn_element_shape_type const_shape_type = CMZN_ELEMENT_SHAPE_TYPE_INVALID;
		HDsMapInt elementShapeParameters; // used only if shape evaluator uses indirect map
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
						const_shape_type = getElementShapeFromFieldmlName(getName(fmlShapeEvaluator).c_str());
						if (const_shape_type == CMZN_ELEMENT_SHAPE_TYPE_INVALID)
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
							elementShapeParameters = HDsMapInt(this->getEnsembleParameters(fmlElementToShapeParameter));
							if (!elementShapeParameters)
							{
								display_message(ERROR_MESSAGE, "Read FieldML:  Invalid element to shape parameters %s for shape evaluator %s of mesh type %s.",
									getName(fmlElementToShapeParameter).c_str(), getName(fmlShapeEvaluator).c_str(), name.c_str());
							}
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

		cmzn_mesh_id mesh = cmzn_fieldmodule_find_mesh_by_dimension(field_module, meshDimension);
		cmzn_elementtemplate_id elementtemplate = cmzn_mesh_create_elementtemplate(mesh);

		FmlObjectHandle fmlLastElementShapeEvaluator = FML_INVALID_OBJECT_HANDLE;
		cmzn_element_shape_type last_shape_type = CMZN_ELEMENT_SHAPE_TYPE_INVALID;

		HDsMapIndexing elementShapeParametersIndexing(elementShapeParameters ? elementShapeParameters->createIndexing() : 0);

		DsLabelIterator *elementsLabelIterator = elementsLabels->createLabelIterator();
		if (!elementsLabelIterator)
			return_code = 0;
		else
		{
			while (elementsLabelIterator->increment())
			{
				int elementIdentifier = elementsLabelIterator->getIdentifier();
				cmzn_element_shape_type shape_type = const_shape_type;
				if (const_shape_type == CMZN_ELEMENT_SHAPE_TYPE_INVALID)
				{
					int shapeIdentifier = elementIdentifier;
					if (elementShapeParameters && (
						(CMZN_OK != elementShapeParametersIndexing->setEntry(*elementsLabelIterator)) ||
						(CMZN_OK != elementShapeParameters->getValues(*elementShapeParametersIndexing, 1, &shapeIdentifier))))
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
							shape_type = getElementShapeFromFieldmlName(getName(fmlElementShapeEvaluator).c_str());
							fmlLastElementShapeEvaluator = fmlElementShapeEvaluator;
						}
						if (shape_type == CMZN_ELEMENT_SHAPE_TYPE_INVALID)
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
					if (!(cmzn_elementtemplate_set_element_shape_type(elementtemplate, shape_type)))
					{
						return_code = 0;
						break;
					}
					last_shape_type = shape_type;
				}
				if (!cmzn_mesh_define_element(mesh, elementIdentifier, elementtemplate))
				{
					return_code = 0;
					break;
				}
			}
		}
		cmzn::Deaccess(elementsLabelIterator);
		cmzn_elementtemplate_destroy(&elementtemplate);
		cmzn_mesh_destroy(&mesh);
	}
	return return_code;
}

ElementFieldComponent *FieldMLReader::getElementFieldComponent(cmzn_mesh_id mesh,
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

	HDsMapInt local_point_to_node(this->getEnsembleParameters(fmlLocalPointToNode));
	HDsMapIndexing indexing(local_point_to_node->createIndexing());

	FmlObjectHandle fmlLocalPointType = Fieldml_GetValueType(fmlSession, fmlLocalPointArgument);
	int local_point_count = Fieldml_GetMemberCount(fmlSession, fmlLocalPointType);

	cmzn_elementbasis_id element_basis = cmzn_fieldmodule_create_elementbasis(field_module, meshDimension, libraryBases[basis_index].functionType[0]);
	if (!libraryBases[basis_index].homogeneous)
	{
		for (int dimension = 2; dimension <= meshDimension; dimension++)
		{
			cmzn_elementbasis_set_function_type(element_basis, dimension,
				libraryBases[basis_index].functionType[dimension - 1]);
		}
	}
	int basis_number_of_nodes = cmzn_elementbasis_get_number_of_nodes(element_basis);
	ElementFieldComponent *component = new ElementFieldComponent(element_basis, local_point_to_node, indexing, local_point_count, libraryBases[basis_index].swizzle);
	if (local_point_to_node && indexing && local_point_count && (local_point_count == basis_number_of_nodes))
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
	const int componentCount = static_cast<int>(fmlComponentEvaluators.size());

	std::string fieldName = getName(fmlFieldEvaluator);

	if (verbose)
	{
		display_message(INFORMATION_MESSAGE, "\n==> Defining field from evaluator %s, %d components\n",
			fieldName.c_str(), componentCount);
	}

	cmzn_field_id field = cmzn_fieldmodule_create_field_finite_element(field_module, componentCount);
	cmzn_field_set_name(field, fieldName.c_str());
	cmzn_field_set_managed(field, true);
	if ((componentCount >= meshDimension) && (componentCount <= 3))
	{
		// if field value type is RC coordinates, set field 'type coordinate' flag.
		// Needed to define faces, and by cmgui to find default coordinate field.
		FmlObjectHandle fmlValueType = Fieldml_GetValueType(fmlSession, fmlFieldEvaluator);
		std::string valueTypeName = this->getDeclaredName(fmlValueType);
		if ((valueTypeName == "coordinates.rc.3d") ||
			(valueTypeName == "coordinates.rc.2d") ||
			(valueTypeName == "coordinates.rc.1d"))
		{
			cmzn_field_set_type_coordinate(field, true);
		}
	}

	// create nodes and set node parameters

	cmzn_nodeset_id nodes = cmzn_fieldmodule_find_nodeset_by_field_domain_type(field_module, CMZN_FIELD_DOMAIN_TYPE_NODES);
	HDsLabels nodesLabels(getLabelsForEnsemble(fmlNodeEnsembleType));
	if (this->fmlNodesType == FML_INVALID_OBJECT_HANDLE)
	{
		this->fmlNodesType = fmlNodeEnsembleType;
		// create the nodes
		// GRC: Could be made more efficient with bulk call
		cmzn_nodetemplate_id nodetemplate = cmzn_nodeset_create_nodetemplate(nodes);
		DsLabelIterator *nodesLabelIterator = nodesLabels->createLabelIterator();
		if (!nodesLabelIterator)
			return_code = CMZN_ERROR_MEMORY;
		else
		{
			while (nodesLabelIterator->increment())
			{
				DsLabelIdentifier nodeIdentifier = nodesLabelIterator->getIdentifier();
				cmzn_node_id node = cmzn_nodeset_create_node(nodes, nodeIdentifier, nodetemplate);
				cmzn_node_destroy(&node);
			}
		}
		cmzn::Deaccess(nodesLabelIterator);
		cmzn_nodetemplate_destroy(&nodetemplate);
	}

	HDsMapDouble nodeParameters(this->getContinuousParameters(fmlNodeParameters));
	if (!nodeParameters)
	{
		display_message(ERROR_MESSAGE,
			"Read FieldML:  Field %s nodal parameters %s unable to be read.",
			fieldName.c_str(), getName(fmlNodeParameters).c_str());
		return_code = 0;
	}
	if (return_code)
	{
		cmzn_nodetemplate_id nodetemplate = cmzn_nodeset_create_nodetemplate(nodes);
		cmzn_nodetemplate_define_field(nodetemplate, field);
		HDsMapIndexing nodeParametersIndexing(nodeParameters->createIndexing());
		double *values = new double[componentCount];
		int *valueExists = new int[componentCount];
		cmzn_fieldcache_id field_cache = cmzn_fieldmodule_create_fieldcache(field_module);
		// GRC inefficient to iterate over sparse parameters this way
		DsLabelIterator *nodesLabelIterator = nodesLabels->createLabelIterator();
		if (!nodesLabelIterator)
			return_code = CMZN_ERROR_MEMORY;
		else
		{
			while (nodesLabelIterator->increment())
			{
				DsLabelIdentifier nodeIdentifier = nodesLabelIterator->getIdentifier();
				cmzn_node_id node = cmzn_nodeset_find_node_by_identifier(nodes, nodeIdentifier);
				nodeParametersIndexing->setEntry(*nodesLabelIterator);
				int valuesRead = 0;
				if (nodeParameters->getValuesSparse(*nodeParametersIndexing, componentCount, values, valueExists, valuesRead))
				{
					if (0 < valuesRead)
					{
						// A current limitation of Zinc is that all nodal component values
						// must be set if any are set. Set the dummy values to zero.
						if (valuesRead < componentCount)
						{
							for (int i = 0; i < componentCount; i++)
							{
								if (!valueExists[i])
									values[i] = 0.0;
							}
						}
						cmzn_node_merge(node, nodetemplate);
						cmzn_fieldcache_set_node(field_cache, node);
						cmzn_field_assign_real(field, field_cache, componentCount, values);
					}
				}
				else
				{
					return_code = 0;
					break;
				}
				cmzn_node_destroy(&node);
			}
		}
		cmzn::Deaccess(nodesLabelIterator);
		cmzn_fieldcache_destroy(&field_cache);
		delete[] valueExists;
		delete[] values;
		cmzn_nodetemplate_destroy(&nodetemplate);
	}

	// define element fields

	cmzn_mesh_id mesh =
		cmzn_fieldmodule_find_mesh_by_dimension(field_module, meshDimension);
	cmzn_elementtemplate_id elementtemplate = 0;
	HDsLabels elementsLabels(getLabelsForEnsemble(fmlElementsType));
	DsLabelIterator *elementsLabelIterator = elementsLabels->createLabelIterator();
	if (!elementsLabelIterator)
		return_code = 0;
	std::vector<FmlObjectHandle> fmlElementEvaluators(componentCount, FML_INVALID_OBJECT_HANDLE);
	std::vector<ElementFieldComponent*> components(componentCount, (ElementFieldComponent*)0);
	std::vector<HDsMapInt> componentFunctionMap(componentCount);
	std::vector<HDsMapIndexing> componentFunctionIndex(componentCount);
	for (int ic = 0; ic < componentCount; ic++)
	{
		int bindCount = Fieldml_GetBindCount(fmlSession, fmlComponentEvaluators[ic]);
		if (1 == bindCount)
		{
			// recognised indirect form: a parameter mapping element to piecewise index
			FmlObjectHandle fmlComponentFunctionMapEvaluator =
				Fieldml_GetBindEvaluator(fmlSession, fmlComponentEvaluators[ic], 1);
			HDsMapInt parameterMap(this->getEnsembleParameters(fmlComponentFunctionMapEvaluator));
			if (parameterMap)
			{
				componentFunctionMap[ic] = parameterMap;
				componentFunctionIndex[ic] = HDsMapIndexing(parameterMap->createIndexing());
			}
			else
				return_code = 0;
		}
	}
	while (return_code && elementsLabelIterator->increment())
	{
		DsLabelIdentifier elementIdentifier = elementsLabelIterator->getIdentifier();
		bool newElementtemplate = (elementtemplate == 0);
		bool definedOnAllComponents = true;
		for (int ic = 0; ic < componentCount; ic++)
		{
			int functionId = elementIdentifier;
			if (componentFunctionMap[ic])
			{
				// handle indirect element to function map
				if (!(componentFunctionIndex[ic]->setEntry(*elementsLabelIterator) &&
					componentFunctionMap[ic]->getValues(*(componentFunctionIndex[ic]), 1, &functionId)))
				{
					definedOnAllComponents = false;
					break;
				}
			}
			FmlObjectHandle fmlElementEvaluator = Fieldml_GetElementEvaluator(fmlSession,
				fmlComponentEvaluators[ic], functionId, /*allowDefault*/1);
			if (fmlElementEvaluator != fmlElementEvaluators[ic])
			{
				fmlElementEvaluators[ic] = fmlElementEvaluator;
				newElementtemplate = true;
			}
			if (fmlElementEvaluator == FML_INVALID_OBJECT_HANDLE)
			{
				definedOnAllComponents = false;
				break;
			}
		}
		if (definedOnAllComponents)
		{
			if (newElementtemplate)
			{
				if (elementtemplate)
					cmzn_elementtemplate_destroy(&elementtemplate);
				elementtemplate = cmzn_mesh_create_elementtemplate(mesh);
				// do not want to override shape of existing elements:
				cmzn_elementtemplate_set_element_shape_type(elementtemplate, CMZN_ELEMENT_SHAPE_TYPE_INVALID);
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
						cmzn_elementtemplate_set_number_of_nodes(elementtemplate, total_local_point_count);
					}
					if (!cmzn_elementtemplate_define_field_simple_nodal(elementtemplate, field,
						/*component*/ic + 1, components[ic]->element_basis, components[ic]->local_point_count,
						components[ic]->local_point_indexes))
					{
						return_code = 0;
						break;
					}
				}
			}
			int total_local_point_count = 0;
			for (int ic = 0; (ic < componentCount) && return_code; ic++)
			{
				ElementFieldComponent *component = components[ic];
				if ((total_local_point_count + 1) == component->local_point_indexes[0])
				{
					total_local_point_count += component->local_point_count;
					component->indexing->setEntry(*elementsLabelIterator);
					if (!component->local_point_to_node->getValues(*(component->indexing),
						component->local_point_count, component->node_identifiers))
					{
						display_message(ERROR_MESSAGE, "Read FieldML:  Incomplete local to global map for field %s", fieldName.c_str());
						return_code = 0;
						break;
					}
					for (int i = 0; i < component->local_point_count; i++)
					{
						cmzn_node_id node = cmzn_nodeset_find_node_by_identifier(nodes, component->node_identifiers[i]);
						if (!node)
						{
							display_message(ERROR_MESSAGE, "Read FieldML:  Cannot find node %d for element %d local point %d in local point to node map %s",
								component->node_identifiers[i], elementIdentifier, i + 1, component->local_point_to_node->getName().c_str());
							return_code = 0;
							break;
						}
						cmzn_elementtemplate_set_node(elementtemplate, component->swizzled_local_point_indexes[i], node);
						cmzn_node_destroy(&node);
					}
				}
			}
			if (return_code)
			{
				cmzn_element_id element = cmzn_mesh_find_element_by_identifier(mesh, elementIdentifier);
				if (!cmzn_element_merge(element, elementtemplate))
				{
					display_message(ERROR_MESSAGE, "Read FieldML:  Could not merge element %d", elementIdentifier);
					return_code = 0;
				}
				cmzn_element_destroy(&element);
			}
		}
	}
	if (elementtemplate)
		cmzn_elementtemplate_destroy(&elementtemplate);
	cmzn::Deaccess(elementsLabelIterator);
	cmzn_mesh_destroy(&mesh);
	cmzn_nodeset_destroy(&nodes);
	cmzn_field_destroy(&field);

	return return_code;
}

/**
 * Test whether the evaluator is scalar, continuous and piecewise over elements
 * of the mesh, directly or indirectly via a map to an intermediate ensemble,
 * and a function of the same element argument evaluator.
 * @param fmlEvaluator  The evaluator to check.
 * @param fmlElementArgument  On true result, set to the element argument the
 * piecewise evaluator is ultimately indexed over.
 * @return  Boolean true if in recognised form, false if not.
 */
bool FieldMLReader::evaluatorIsScalarContinuousPiecewiseOverElements(FmlObjectHandle fmlEvaluator,
	FmlObjectHandle &fmlElementArgument)
{
	if (FHT_PIECEWISE_EVALUATOR != Fieldml_GetObjectType(this->fmlSession, fmlEvaluator))
		return false;
	FmlObjectHandle fmlValueType = Fieldml_GetValueType(this->fmlSession, fmlEvaluator);
	if (FHT_CONTINUOUS_TYPE != Fieldml_GetObjectType(this->fmlSession, fmlValueType))
		return false;
	if (1 != Fieldml_GetTypeComponentCount(this->fmlSession, fmlValueType))
		return false;
	FmlObjectHandle fmlPiecewiseIndex = Fieldml_GetIndexEvaluator(this->fmlSession, fmlEvaluator, /*evaluatorIndex*/1);
	if (fmlPiecewiseIndex == FML_INVALID_OBJECT_HANDLE)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Piecewise Evaluator %s has no index evaluator",
			this->getName(fmlEvaluator).c_str());
		return false;
	}
	// can either be directly indexed by elements, or indirectly by map
	// from elements to intermediate ensemble (i.e. a function ID)
	int bindCount = Fieldml_GetBindCount(fmlSession, fmlEvaluator);
	if (1 == bindCount)
	{
		// test for recognised indirect form: a parameter mapping element to piecewise index
		FmlObjectHandle fmlBindArgument = Fieldml_GetBindArgument(fmlSession, fmlEvaluator, 1);
		FmlObjectHandle fmlBindEvaluator = Fieldml_GetBindEvaluator(fmlSession, fmlEvaluator, 1);
		if (fmlBindArgument != fmlPiecewiseIndex)
			return false;
		if (FHT_PARAMETER_EVALUATOR != Fieldml_GetObjectType(fmlSession, fmlBindEvaluator))
			return false;
		int parameterIndexCount = Fieldml_GetIndexEvaluatorCount(fmlSession, fmlBindEvaluator);
		if (1 != parameterIndexCount)
			return false;
		fmlPiecewiseIndex = Fieldml_GetIndexEvaluator(this->fmlSession, fmlBindEvaluator, /*evaluatorIndex*/1);
		if (fmlPiecewiseIndex == FML_INVALID_OBJECT_HANDLE)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Parameter Evaluator %s has no index evaluator",
				this->getName(fmlBindEvaluator).c_str());
			return false;
		}
	}
	else if (1 < bindCount)
		return false;
	FmlObjectHandle fmlIndexType = Fieldml_GetValueType(this->fmlSession, fmlPiecewiseIndex);
	if (fmlIndexType != this->fmlElementsType)
		return false;
	fmlElementArgument = fmlPiecewiseIndex;
	return true;
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

		// check components are scalar, piecewise over elements
		bool validComponents = true;
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
			FmlObjectHandle fmlComponentElementArgument = FML_INVALID_OBJECT_HANDLE;
			if (!this->evaluatorIsScalarContinuousPiecewiseOverElements(
				fmlComponentEvaluators[ic], fmlComponentElementArgument))
			{
				if (verbose)
					display_message(WARNING_MESSAGE, "Read FieldML:  Aggregate %s component %d is not piecewise over elements",
						fieldName.c_str(), componentIndex);
				validComponents = false;
				break;
			}
			if (fmlElementArgument == FML_INVALID_OBJECT_HANDLE)
				fmlElementArgument = fmlComponentElementArgument;
			else if (fmlComponentElementArgument != fmlElementArgument)
			{
				if (verbose)
					display_message(ERROR_MESSAGE, "Read FieldML:  Aggregate %s components must use same element argument",
						fieldName.c_str());
				validComponents = false;
				break;
			}
		}
		if (!return_code)
			break;
		if (!validComponents)
		{
			if (verbose)
				display_message(WARNING_MESSAGE, "Read FieldML:  Aggregate %s cannot be interpreted as a field defined over a mesh. Skipping.",
					fieldName.c_str());
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

		// check reference evaluator is scalar, piecewise over elements
		FmlObjectHandle fmlComponentEvaluator = Fieldml_GetReferenceSourceEvaluator(fmlSession, fmlReference);
		if (fmlComponentEvaluator == FML_INVALID_OBJECT_HANDLE)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Reference %s source evaluator is missing", fieldName.c_str());
			return_code = 0;
			break;
		}
		FmlObjectHandle fmlElementArgument = FML_INVALID_OBJECT_HANDLE;
		if (!this->evaluatorIsScalarContinuousPiecewiseOverElements(
			fmlComponentEvaluator, fmlElementArgument))
		{
			if (verbose)
				display_message(WARNING_MESSAGE,
					"Read FieldML:  Reference %s cannot be interpreted as a field defined over a mesh. Skipping.",
					fieldName.c_str());
			continue;
		}

		// determine if exactly one binding of 'nodal parameters'

		int bindCount = Fieldml_GetBindCount(fmlSession, fmlReference);
		if (1 != bindCount)
		{
			if (verbose)
			{
				display_message(WARNING_MESSAGE,
					"Read FieldML:  Reference %s does not have exactly 1 binding (for nodal parameters). Skipping.",
					fieldName.c_str());
			}
			continue;
		}
		FmlObjectHandle fmlNodeParametersArgument = Fieldml_GetBindArgument(fmlSession, fmlReference, 1);
		FmlObjectHandle fmlNodeParameters = Fieldml_GetBindEvaluator(fmlSession, fmlReference, 1);
		int nodeParametersIndexCount = Fieldml_GetIndexEvaluatorCount(fmlSession, fmlNodeParameters);
		FmlObjectHandle fmlNodeParametersIndex1 = Fieldml_GetIndexEvaluator(fmlSession, fmlNodeParameters, 1);
		if ((Fieldml_GetObjectType(fmlSession, fmlNodeParameters) != FHT_PARAMETER_EVALUATOR) ||
			(Fieldml_GetObjectType(fmlSession, Fieldml_GetValueType(fmlSession, fmlNodeParameters)) != FHT_CONTINUOUS_TYPE) ||
			(1 != nodeParametersIndexCount) ||
			(fmlNodeParametersIndex1 == FML_INVALID_OBJECT_HANDLE))
		{
			if (verbose)
			{
				display_message(WARNING_MESSAGE,
					"Read FieldML:  Reference %s does not bind exactly 1 scalar nodal parameters object. Skipping.",
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
	cmzn_region_begin_change(region);
	return_code = return_code && readMeshes();
	return_code = return_code && readAggregateFields();
	return_code = return_code && readReferenceFields();
	cmzn_region_end_change(region);
	return return_code;
}

bool string_contains_FieldML_tag(const char *text)
{
	return (0 != strstr(text, "<Fieldml"));
}

} // anonymous namespace

bool is_FieldML_file(const char *filename)
{
	bool result = false;
	FILE *stream = fopen(filename, "r");
	if (stream)
	{
		char block[200];
		size_t size = fread((void *)block, sizeof(char), sizeof(block), stream);
		if (size > 0)
		{
			block[size - 1] = '\0';
			result = string_contains_FieldML_tag(block);
		}
		fclose(stream);
	}
	return result;
}

bool is_FieldML_memory_block(unsigned int memory_buffer_size, const void *memory_buffer)
{
	if ((0 == memory_buffer_size) || (!memory_buffer))
		return false;
	unsigned int size = memory_buffer_size;
	if (size > 200)
		size = 200;
	char block[200];
	memcpy(block, memory_buffer, size);
	block[size - 1] = '\0';
	return string_contains_FieldML_tag(block);
}

int parse_fieldml_file(struct cmzn_region *region, const char *filename)
{
	FieldMLReader fmlReader(region, filename);
	return fmlReader.parse();
}
