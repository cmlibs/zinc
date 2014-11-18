/**
 * FILE : write_fieldml.cpp
 * 
 * FieldML 0.5 model writer implementation.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <cstring>
#include <map>
#include <string>
#include "zinc/core.h"
#include "zinc/element.h"
#include "zinc/field.h"
#include "zinc/fieldmodule.h"
#include "zinc/node.h"
#include "zinc/region.h"
#include "zinc/status.h"
#include "datastore/labels.hpp"
#include "datastore/map.hpp"
#include "datastore/mapindexing.hpp"
#include "field_io/fieldml_common.hpp"
#include "field_io/write_fieldml.hpp"
#include "finite_element/finite_element.h"
#include "general/debug.h"
#include "general/message.h"
#include "general/mystring.h"
#include "fieldml_api.h"
#include "FieldmlIoApi.h"

namespace {

const FmlObjectHandle FML_INVALID_OBJECT_HANDLE = (const FmlObjectHandle)FML_INVALID_HANDLE;

class FieldMLWriter
{
	cmzn_region *region;
	cmzn_fieldmodule_id fieldmodule;
	const char *location;
	const char *filename;
	char *regionName;
	FmlSessionHandle fmlSession;
	bool verbose;
	int libraryImportSourceIndex;
	std::map<cmzn_field_domain_type,HDsLabels> nodesetLabels;
	std::vector<HDsLabels> meshLabels; // indexed by dimension
	std::map<FmlObjectHandle,FmlObjectHandle> typeArgument;

public:
	FieldMLWriter(struct cmzn_region *region, const char *locationIn, const char *filenameIn) :
		region(cmzn_region_access(region)),
		fieldmodule(cmzn_region_get_fieldmodule(region)),
		location(locationIn),
		filename(filenameIn),
		fmlSession(Fieldml_Create(location, /*regionName*/"/")),
		verbose(false),
		libraryImportSourceIndex(-1),
		meshLabels(4)
	{
		Fieldml_SetDebug(fmlSession, /*debug*/verbose);
	}

	~FieldMLWriter()
	{
		Fieldml_Destroy(fmlSession);
		cmzn_fieldmodule_destroy(&fieldmodule);
		cmzn_region_destroy(&region);
	}

	int writeNodeset(cmzn_field_domain_type domainType, bool writeIfEmpty);
	int writeNodesets();

	int writeMesh(int dimension, bool writeIfEmpty);
	int writeMeshes();

	int writeFile(const char *pathandfilename);

private:
	FmlObjectHandle libraryImport(const char *remoteName);
	FmlObjectHandle getArgumentForType(FmlObjectHandle fmlType);
	int defineEnsembleFromLabels(FmlObjectHandle fmlEnsembleType, DsLabels& nodeLabels);
	template <typename VALUETYPE> FmlObjectHandle defineParametersFromMap(
		DsMap<VALUETYPE>& parameterMap, FmlObjectHandle fmlValueType);

};

FmlObjectHandle FieldMLWriter::libraryImport(const char *remoteName)
{
	if (-1 == this->libraryImportSourceIndex)
	{
		this->libraryImportSourceIndex = Fieldml_AddImportSource(this->fmlSession,
			"http://www.fieldml.org/resources/xml/0.5/FieldML_Library_0.5.xml", "library");
	}
	FmlObjectHandle fmlImport = Fieldml_AddImport(this->fmlSession, libraryImportSourceIndex, remoteName, remoteName);
	if (fmlImport == FML_INVALID_OBJECT_HANDLE)
		display_message(ERROR_MESSAGE, "Failed to import %s from library", remoteName);
	return fmlImport;
}

FmlObjectHandle FieldMLWriter::getArgumentForType(FmlObjectHandle fmlType)
{
	FieldmlHandleType objectType = Fieldml_GetObjectType(this->fmlSession, fmlType);
	if ((objectType != FHT_ENSEMBLE_TYPE) &&
		(objectType != FHT_CONTINUOUS_TYPE) &&
		(objectType != FHT_MESH_TYPE))
		return FML_INVALID_OBJECT_HANDLE;
	std::map<FmlObjectHandle,FmlObjectHandle>::iterator iter = this->typeArgument.find(fmlType);
	if (iter != this->typeArgument.end())
		return iter->second;
	char *objectName = Fieldml_GetObjectName(this->fmlSession, fmlType);
	if (!objectName)
		return FML_INVALID_OBJECT_HANDLE;
	std::string argumentName(objectName);
	argumentName += ".argument";
	FmlObjectHandle fmlArgument = Fieldml_CreateArgumentEvaluator(this->fmlSession, argumentName.c_str(), fmlType);
	this->typeArgument[fmlType] = fmlArgument;
	Fieldml_FreeString(objectName);
	return fmlArgument;
}

int FieldMLWriter::defineEnsembleFromLabels(FmlObjectHandle fmlEnsembleType, DsLabels& labels)
{
	int return_code = CMZN_OK;
	FmlErrorNumber fmlError;
	if (labels.isContiguous())
	{
		DsLabelIdentifier firstIdentifier = labels.getLabelIdentifier(0);
		DsLabelIdentifier lastIdentifier = firstIdentifier + labels.getSize() - 1;
		fmlError = Fieldml_SetEnsembleMembersRange(this->fmlSession, fmlEnsembleType,
			firstIdentifier, lastIdentifier, /*stride*/1);
		if (fmlError != FML_OK)
			return_code = CMZN_ERROR_GENERAL;
	}
	else
	{
		// for non-contiguous use inline range data source
		std::string dataResourceName(labels.getName());
		dataResourceName += ".data.resource";
		FmlObjectHandle fmlDataResource = Fieldml_CreateInlineDataResource(this->fmlSession, dataResourceName.c_str());
		std::string dataSourceName(labels.getName());
		dataSourceName += ".data.source";
		FmlObjectHandle fmlDataSource = Fieldml_CreateArrayDataSource(this->fmlSession, dataSourceName.c_str(), fmlDataResource, /*location*/"0", /*rank*/2);
		DsLabelIdentifierRanges ranges;
		labels.getIdentifierRanges(ranges);
		int sizes[2] = { static_cast<int>(ranges.size()), 2 };
		Fieldml_SetArrayDataSourceRawSizes(this->fmlSession, fmlDataSource, sizes);
		Fieldml_SetArrayDataSourceSizes(this->fmlSession, fmlDataSource, sizes);
		FmlWriterHandle fmlArrayWriter = Fieldml_OpenArrayWriter(this->fmlSession,
			/*objectHandle*/fmlDataSource,
			/*typeHandle*/fmlEnsembleType,
			/*append*/false,
			sizes, // GRC OUTVALUE?
			/*rank*/2
			);
		if (fmlArrayWriter == FML_INVALID_OBJECT_HANDLE)
			return_code = CMZN_ERROR_GENERAL;
		if (CMZN_OK == return_code)
		{
			int numberOfRanges = static_cast<int>(ranges.size());
			for (int i = 0; i < numberOfRanges; ++i)
			{
				const int slabOffsets[] = { i, 0 };
				const int slabSizes[] = { 1, 2 };
				const int range[2] = { ranges[i].first, ranges[i].last };
				FmlIoErrorNumber fmlIoError = Fieldml_WriteIntSlab(fmlArrayWriter, slabOffsets, slabSizes, range);
				if (FML_IOERR_NO_ERROR != fmlIoError)
				{
					return_code = CMZN_ERROR_GENERAL;
					break;
				}
			}
		}
		Fieldml_CloseWriter(fmlArrayWriter);
		if (CMZN_OK == return_code)
		{
			fmlError = Fieldml_SetEnsembleMembersDataSource(this->fmlSession, fmlEnsembleType,
				FML_ENSEMBLE_MEMBER_RANGE_DATA, labels.getSize(), fmlDataSource);
			if (fmlError != FML_OK)
				return_code = CMZN_ERROR_GENERAL;
		}
	}
	return return_code;
}

// template and full specialisations to write different types with template
template <typename VALUETYPE> FmlIoErrorNumber FieldML_WriteSlab(
	FmlWriterHandle writerHandle, const int *offsets, const int *sizes, const VALUETYPE *valueBuffer);

template <> inline FmlIoErrorNumber FieldML_WriteSlab(
	FmlWriterHandle writerHandle, const int *offsets, const int *sizes, const double *valueBuffer)
{
	return Fieldml_WriteDoubleSlab(writerHandle, offsets, sizes, valueBuffer);
}

template <> inline FmlIoErrorNumber FieldML_WriteSlab(
	FmlWriterHandle writerHandle, const int *offsets, const int *sizes, const int *valueBuffer)
{
	return Fieldml_WriteIntSlab(writerHandle, offsets, sizes, valueBuffer);
}

template <typename VALUETYPE> FmlObjectHandle FieldMLWriter::defineParametersFromMap(
	DsMap<VALUETYPE>& parameterMap, FmlObjectHandle fmlValueType)
{
	std::string name = parameterMap.getName();
	std::vector<HDsLabels> sparseLabelsArray;
	std::vector<HDsLabels> denseLabelsArray;
	const int labelsArraySize = parameterMap.getLabelsArraySize();
	for (int labelsNumber = 0; labelsNumber < labelsArraySize; ++labelsNumber)
	{
		HDsLabels labels(parameterMap.getLabels(labelsNumber));
		if (parameterMap.isDenseAndCompleteOnLabels(labelsNumber))
			denseLabelsArray.push_back(labels);
		else
			sparseLabelsArray.push_back(labels);
	}
	if (sparseLabelsArray.size() > 0)
	{
		display_message(INFORMATION_MESSAGE, "FieldMLWriter::defineParametersFromMap.  "
			"Sparse parameters not implemented, can't write %s", name.c_str());
		return FML_INVALID_OBJECT_HANDLE;
	}

	int return_code = CMZN_OK;
	FmlErrorNumber fmlError;
	std::string dataResourceName(name + ".data.resource");
	FmlObjectHandle fmlDataResource = Fieldml_CreateInlineDataResource(this->fmlSession, dataResourceName.c_str());
	std::string dataSourceName(name + ".data.source");
	const int denseLabelsCount = static_cast<int>(denseLabelsArray.size());
	FmlObjectHandle fmlDataSource = Fieldml_CreateArrayDataSource(this->fmlSession, dataSourceName.c_str(),
		fmlDataResource, /*location*/"0", /*rank*/denseLabelsCount);
	int *sizes = new int[denseLabelsCount];
	int *offsets = new int[denseLabelsCount];
	for (int i = 0; i < denseLabelsCount; ++i)
	{
		sizes[i] = denseLabelsArray[i]->getSize();
		offsets[i] = 0;
	}
	Fieldml_SetArrayDataSourceRawSizes(this->fmlSession, fmlDataSource, sizes);
	Fieldml_SetArrayDataSourceSizes(this->fmlSession, fmlDataSource, sizes);

	FmlWriterHandle fmlArrayWriter = Fieldml_OpenArrayWriter(this->fmlSession,
		fmlDataSource, fmlValueType, /*append*/false,
		sizes, // GRC OUTVALUE?
		/*rank*/denseLabelsCount
		);

	HDsMapIndexing mapIndexing(parameterMap.createIndexing());
	DsMapAddressType denseValuesCount = mapIndexing->getEntryCount();
	VALUETYPE *values = new VALUETYPE[denseValuesCount];
	if (fmlArrayWriter == FML_INVALID_OBJECT_HANDLE)
		return_code = CMZN_ERROR_GENERAL;
	if (CMZN_OK == return_code)
	{
		if (!parameterMap.getValues(*mapIndexing, denseValuesCount, values))
			return_code = CMZN_ERROR_GENERAL;
		else
		{
			FmlIoErrorNumber fmlIoError = FieldML_WriteSlab(fmlArrayWriter, offsets, sizes, values);
			if (FML_IOERR_NO_ERROR != fmlIoError)
				return_code = CMZN_ERROR_GENERAL;
		}
	}
	Fieldml_CloseWriter(fmlArrayWriter);

	FmlObjectHandle fmlParameters = FML_INVALID_OBJECT_HANDLE;
	if (CMZN_OK == return_code)
	{
		fmlParameters = Fieldml_CreateParameterEvaluator(this->fmlSession, name.c_str(), fmlValueType);
		fmlError = Fieldml_SetParameterDataDescription(this->fmlSession, fmlParameters, FML_DATA_DESCRIPTION_DENSE_ARRAY);
		if (FML_OK != fmlError)
			return_code = CMZN_ERROR_GENERAL;
		fmlError = Fieldml_SetDataSource(this->fmlSession, fmlParameters, fmlDataSource);
		if (FML_OK != fmlError)
			return_code = CMZN_ERROR_GENERAL;
		for (int i = 0; i < denseLabelsCount; ++i)
		{
			std::string labelsName = denseLabelsArray[i]->getName();
			FmlObjectHandle fmlLabelsType = Fieldml_GetObjectByName(this->fmlSession, labelsName.c_str());
			FmlObjectHandle fmlIndexArgument = this->getArgumentForType(fmlLabelsType);
			fmlError = Fieldml_AddDenseIndexEvaluator(this->fmlSession, fmlParameters,
				fmlIndexArgument, /*orderHandle*/FML_INVALID_OBJECT_HANDLE);
			if (FML_OK != fmlError)
			{
				return_code = CMZN_ERROR_GENERAL;
				break;
			}
		}
	}
	delete[] values;
	delete[] offsets;
	delete[] sizes;
	if (CMZN_OK != return_code)
		return FML_INVALID_OBJECT_HANDLE;
	return fmlParameters;
}

int FieldMLWriter::writeMesh(int dimension, bool writeIfEmpty)
{
	int return_code = CMZN_OK;
	cmzn_mesh_id mesh = cmzn_fieldmodule_find_mesh_by_dimension(this->fieldmodule, dimension);
	char *name = cmzn_mesh_get_name(mesh);
	FmlErrorNumber fmlError;
	int meshSize = cmzn_mesh_get_size(mesh);
	if (writeIfEmpty || (0 < meshSize))
	{
		int *shapeIds = 0;
		FmlObjectHandle fmlMeshType = Fieldml_CreateMeshType(this->fmlSession, name);
		const char *chartName = "xi";
		FmlObjectHandle fmlMeshChartType = Fieldml_CreateMeshChartType(this->fmlSession, fmlMeshType, chartName);
		FmlObjectHandle fmlMeshChartComponentsType = FML_INVALID_OBJECT_HANDLE;
		if (fmlMeshChartType == FML_INVALID_OBJECT_HANDLE)
			return_code = CMZN_ERROR_GENERAL;
		else
		{
			if (dimension > 1)
			{
				const char *chartComponentsName = "mesh3d.xi.components";
				fmlMeshChartComponentsType = Fieldml_CreateContinuousTypeComponents(
					this->fmlSession, fmlMeshChartType, chartComponentsName, dimension);
				fmlError = Fieldml_SetEnsembleMembersRange(this->fmlSession, fmlMeshChartComponentsType,
					1, dimension, /*stride*/1);
				if (fmlMeshChartComponentsType == FML_INVALID_OBJECT_HANDLE)
					return_code = CMZN_ERROR_GENERAL;
			}
		}
		const char *meshElementsName = "elements";
		FmlObjectHandle fmlMeshElementsType = Fieldml_CreateMeshElementsType(this->fmlSession, fmlMeshType, meshElementsName);

		cmzn_element_shape_type lastShapeType = CMZN_ELEMENT_SHAPE_TYPE_INVALID;
		int lastShapeId = 0;
		std::vector<cmzn_element_shape_type> shapeTypes;
		HDsLabels elementLabels(new DsLabels());
		if (fmlMeshElementsType == FML_INVALID_OBJECT_HANDLE)
			return_code = CMZN_ERROR_GENERAL;
		else
		{
			shapeIds = new int[meshSize];
			elementLabels->setName(std::string(name) + "." + meshElementsName);
			cmzn_element_id element = 0;
			cmzn_elementiterator_id iter = cmzn_mesh_create_elementiterator(mesh);
			int eIndex = 0;
			while (0 != (element = cmzn_elementiterator_next_non_access(iter)))
			{
				if (DS_LABEL_INDEX_INVALID == elementLabels->createLabel(cmzn_element_get_identifier(element)))
				{
					return_code = CMZN_ERROR_MEMORY;
					break;
				}
				cmzn_element_shape_type shapeType = cmzn_element_get_shape_type(element);
				if (shapeType != lastShapeType)
				{
					const int shapeTypesSize = static_cast<int>(shapeTypes.size());
					for (int i = 0; i < shapeTypesSize; ++i)
						if (shapeTypes[i] == shapeType)
						{
							lastShapeType = shapeType;
							lastShapeId = i + 1;
						}
					if (shapeType != lastShapeType)
					{
						shapeTypes.push_back(shapeType);
						lastShapeType = shapeType;
						lastShapeId = static_cast<int>(shapeTypes.size());
					}
				}
				shapeIds[eIndex] = lastShapeId;
				++eIndex;
			}
			cmzn_elementiterator_destroy(&iter);
			this->meshLabels[dimension] = elementLabels;
			if (CMZN_OK == return_code)
				return_code = this->defineEnsembleFromLabels(fmlMeshElementsType, *elementLabels);			
		}
		if (CMZN_OK == return_code)
		{
			// ensure we have argument for mesh type and can find argument for elements type
			// since it uses a special naming pattern e.g. mesh3d.argument.elements
			this->getArgumentForType(fmlMeshType);

			std::string meshElementsArgumentName(name);
			meshElementsArgumentName += ".argument.";
			meshElementsArgumentName += meshElementsName;
			FmlObjectHandle fmlMeshElementsArgument = Fieldml_GetObjectByName(this->fmlSession, meshElementsArgumentName.c_str());
			if (fmlMeshElementsArgument == FML_INVALID_OBJECT_HANDLE)
				return_code = CMZN_ERROR_GENERAL;
			else
				this->typeArgument[fmlMeshElementsType] = fmlMeshElementsArgument;

			// set up shape evaluator, single fixed or indirectly mapped
			if (1 == shapeTypes.size())
			{
				const char *shapeName = getFieldmlNameFromElementShape(shapeTypes[0]);
				FmlObjectHandle fmlMeshShapeEvaluator = this->libraryImport(shapeName);
				fmlError = Fieldml_SetMeshShapes(this->fmlSession, fmlMeshType, fmlMeshShapeEvaluator);
				if (fmlError != FML_OK)
					return_code = CMZN_ERROR_GENERAL;
			}
			else
			{
				HDsLabels meshShapeLabels(new DsLabels());
				std::string meshShapeIdsName(name);
				meshShapeIdsName += ".shapeids";
				meshShapeLabels->setName(meshShapeIdsName);
				if (CMZN_OK != meshShapeLabels->addLabelsRange(1, static_cast<int>(shapeTypes.size())))
					return_code = CMZN_ERROR_MEMORY;
				FmlObjectHandle fmlMeshShapeIdsType = Fieldml_CreateEnsembleType(this->fmlSession, meshShapeIdsName.c_str());
				if (CMZN_OK == return_code)
					return_code = this->defineEnsembleFromLabels(fmlMeshShapeIdsType, *meshShapeLabels);
				DsLabels *tmpElementLabels = cmzn::GetImpl(elementLabels);
				HDsMapInt meshShapeMap(DsMap<int>::create(1, &tmpElementLabels));
				std::string meshShapeMapName(name);
				meshShapeMapName += ".shapeids.map";
				meshShapeMap->setName(meshShapeMapName);
				HDsMapIndexing meshShapeIndexing(meshShapeMap->createIndexing());
				if (!meshShapeMap->setValues(*meshShapeIndexing, meshSize, shapeIds))
					return_code = CMZN_ERROR_MEMORY;
				FmlObjectHandle fmlMeshShapeIdsParameters = this->defineParametersFromMap<int>(*meshShapeMap, fmlMeshShapeIdsType);
				if (fmlMeshShapeIdsParameters == FML_INVALID_OBJECT_HANDLE)
					return_code = CMZN_ERROR_GENERAL;
				else
				{
					std::string meshShapeEvaluatorName(name);
					meshShapeEvaluatorName += ".shape";
					FmlObjectHandle fmlBooleanType = this->libraryImport("boolean");
					FmlObjectHandle fmlMeshShapeEvaluator = Fieldml_CreatePiecewiseEvaluator(this->fmlSession,
						meshShapeEvaluatorName.c_str(), fmlBooleanType);
					FmlObjectHandle fmlMeshShapeIdsArgument = this->getArgumentForType(fmlMeshShapeIdsType);
					fmlError = Fieldml_SetIndexEvaluator(this->fmlSession, fmlMeshShapeEvaluator, /*index*/1, fmlMeshShapeIdsArgument);
					if (FML_OK != fmlError)
						return_code = CMZN_ERROR_GENERAL;
					const int shapeTypesSize = static_cast<int>(shapeTypes.size());
					for (int i = 0; i < shapeTypesSize; ++i)
					{
						const char *shapeName = getFieldmlNameFromElementShape(shapeTypes[i]);
						FmlObjectHandle fmlShapeEvaluator = this->libraryImport(shapeName);
						fmlError = Fieldml_SetEvaluator(this->fmlSession, fmlMeshShapeEvaluator, i + 1, fmlShapeEvaluator);
						if (FML_OK != fmlError)
							return_code = CMZN_ERROR_GENERAL;
					}
					fmlError = Fieldml_SetBind(this->fmlSession, fmlMeshShapeEvaluator, fmlMeshShapeIdsArgument, fmlMeshShapeIdsParameters);
					if (FML_OK != fmlError)
						return_code = CMZN_ERROR_GENERAL;
					fmlError = Fieldml_SetMeshShapes(this->fmlSession, fmlMeshType, fmlMeshShapeEvaluator);
					if (fmlError != FML_OK)
						return_code = CMZN_ERROR_GENERAL;
				}
			}
		}
		delete[] shapeIds;
	}
	cmzn_deallocate(name);
	cmzn_mesh_destroy(&mesh);
	return return_code;
}

int FieldMLWriter::writeMeshes()
{
	const int dimension = 3;
	int return_code = this->writeMesh(dimension, /*writeIfEmpty*/false);
	return return_code;
}

int FieldMLWriter::writeNodeset(cmzn_field_domain_type domainType, bool writeIfEmpty)
{
	int return_code = CMZN_OK;
	cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(
		this->fieldmodule, domainType);
	char *name = cmzn_nodeset_get_name(nodeset);
	if (writeIfEmpty || (0 < cmzn_nodeset_get_size(nodeset)))
	{
		FmlObjectHandle fmlEnsembleType = Fieldml_CreateEnsembleType(this->fmlSession, name);
		if (fmlEnsembleType == FML_INVALID_OBJECT_HANDLE)
			return_code = CMZN_ERROR_GENERAL;
		else
		{
			HDsLabels nodeLabels(new DsLabels());
			nodeLabels->setName(name);
			cmzn_node_id node = 0;
			cmzn_nodeiterator_id iter = cmzn_nodeset_create_nodeiterator(nodeset);
			while (0 != (node = cmzn_nodeiterator_next_non_access(iter)))
			{
				if (DS_LABEL_INDEX_INVALID == nodeLabels->createLabel(cmzn_node_get_identifier(node)))
				{
					return_code = CMZN_ERROR_MEMORY;
					break;
				}
			}
			cmzn_nodeiterator_destroy(&iter);
			this->nodesetLabels[domainType] = nodeLabels;
			if (CMZN_OK == return_code)
				return_code = this->defineEnsembleFromLabels(fmlEnsembleType, *nodeLabels);			
		}
	}
	cmzn_deallocate(name);
	cmzn_nodeset_destroy(&nodeset);
	return return_code;
}

int FieldMLWriter::writeNodesets()
{
	int return_code = CMZN_OK;
	if (CMZN_OK == return_code)
		return_code = this->writeNodeset(CMZN_FIELD_DOMAIN_TYPE_NODES, /*writeIfEmpty*/true);
	if (CMZN_OK == return_code)
		return_code = this->writeNodeset(CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS, /*writeIfEmpty*/false);
	return return_code;
}

int FieldMLWriter::writeFile(const char *pathandfilename)
{
	FmlErrorNumber fmlError = Fieldml_WriteFile(this->fmlSession, pathandfilename);
	if (FML_OK == fmlError)
		return CMZN_OK;
	return CMZN_ERROR_GENERAL;
}

} // namespace

int write_fieldml_file(struct cmzn_region *region, const char *pathandfilename)
{
	int return_code = CMZN_OK;
	if (region && pathandfilename && (*pathandfilename != '\0'))
	{
		char *location = duplicate_string(pathandfilename);
		char *lastDirSep = strrchr(location, '/');
		char *lastDirSepWin = strrchr(location, '\\');
		if (lastDirSepWin > lastDirSep)
			lastDirSep = lastDirSepWin;
		const char *filename;
		if (lastDirSep)
		{
			*lastDirSep = '\0';
			filename = lastDirSep + 1;
		}
		else
		{
			location[0] = '\0';
			filename = pathandfilename;
		}
		FieldMLWriter fmlWriter(region, location, filename);
		if (CMZN_OK == return_code)
			return_code = fmlWriter.writeNodesets();
		if (CMZN_OK == return_code)
			return_code = fmlWriter.writeMeshes();
		if (CMZN_OK == return_code)
			return_code = fmlWriter.writeFile(pathandfilename);
	}
	else
		return_code = CMZN_ERROR_ARGUMENT;
	return return_code;
}
