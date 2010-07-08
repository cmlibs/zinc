/***************************************************************************//**
 * FILE : read_fieldml_02.cpp
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

#include <map>
extern "C" {
#include "api/cmiss_field_ensemble.h"
#include "api/cmiss_field_parameters.h"
#include "api/cmiss_region.h"
#include "field_io/read_fieldml_02.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_helper.h"
#include "finite_element/finite_element_region.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "fieldml_api.h"
#include "user_interface/message.h"
}

namespace {

const FmlObjectHandle FML_INVALID_OBJECT_HANDLE = (const FmlObjectHandle)FML_INVALID_HANDLE;

struct ShapeType
{
	const char *fieldmlName;
	const int dimension;
	const int shape_type[6];
};

const ShapeType libraryShapes[] =
{
	{ "library.shape.line",        1, { LINE_SHAPE, 0, 0, 0, 0, 0 } },
	{ "library.shape.square",      2, { LINE_SHAPE, 0, LINE_SHAPE, 0, 0, 0 } },
	{ "library.shape.triangle",    2, { SIMPLEX_SHAPE, 1, SIMPLEX_SHAPE, 0, 0, 0 } },
	{ "library.shape.cube",        3, { LINE_SHAPE, 0, 0, LINE_SHAPE, 0, LINE_SHAPE } },
	{ "library.shape.tetrahedron", 3, { SIMPLEX_SHAPE, 1, 1, SIMPLEX_SHAPE, 1, SIMPLEX_SHAPE } },
	{ "library.shape.wedge12",     3, { SIMPLEX_SHAPE, 1, 0, SIMPLEX_SHAPE, 0, LINE_SHAPE } }
};
const int numLibraryShapes = sizeof(libraryShapes) / sizeof(ShapeType);

struct BasisType
{
	const char *fieldmlBasis;
	const char *fieldmlXiDomain;
	const char *fieldmlParamatersDomain;
	const char *fieldmlLocalNodesDomain;
	const int basis_type[7];
};

const BasisType libraryBases[] =
{
	{ "library.fem.bilinear_lagrange",     "library.xi.2d", "library.parameters.bilinear_lagrange",     "library.local_nodes.square.2x2", { 2, LINEAR_LAGRANGE, 0, LINEAR_LAGRANGE, 0, 0, 0 } },
	{ "library.fem.biquadratic_lagrange",  "library.xi.2d", "library.parameters.biquadratic_lagrange",  "library.local_nodes.square.3x3", { 2, QUADRATIC_LAGRANGE, 0, QUADRATIC_LAGRANGE, 0, 0, 0 } },
	{ "library.fem.trilinear_lagrange",    "library.xi.3d", "library.parameters.trilinear_lagrange",    "library.local_nodes.cube.2x2x2", { 3, LINEAR_LAGRANGE, 0, 0, LINEAR_LAGRANGE, 0, LINEAR_LAGRANGE } },
	{ "library.fem.triquadratic_lagrange", "library.xi.3d", "library.parameters.triquadratic_lagrange", "library.local_nodes.cube.3x3x3", { 3, QUADRATIC_LAGRANGE, 0, 0, QUADRATIC_LAGRANGE, 0, QUADRATIC_LAGRANGE } },
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
	struct FE_element_field_component *fe_element_field_component;
	FmlObjectHandle fmlParametersSource;
};

typedef std::map<FmlObjectHandle,ElementFieldComponent> EvaluatorElementFieldComponentMap;
typedef std::map<FmlObjectHandle,ElementFieldComponent>::iterator EvaluatorElementFieldComponentMapIterator;

class FieldMLReader
{
	Cmiss_region *region;
	FE_region *fe_region;
	Cmiss_field_module *field_module;
	const char *filename;
	FmlHandle fmlHandle;
	FE_element_shape *fe_element_shapes[numLibraryShapes];
	int meshDimension;
	ConnectivityData *connectivityData;
	int connectivityCount;
	FmlObjectHandle fmlNodesEnsemble;
	FmlObjectHandle fmlElementsEnsemble;
	EvaluatorElementFieldComponentMap componentMap;
	bool verbose;

public:
	FieldMLReader(struct Cmiss_region *region, const char *filename) :
		region(Cmiss_region_access(region)),
		fe_region(ACCESS(FE_region)(Cmiss_region_get_FE_region(region))),
		field_module(Cmiss_region_get_field_module(region)),
		filename(filename),
		fmlHandle(Fieldml_CreateFromFile(filename)),
		meshDimension(0),
		connectivityData(NULL),
		connectivityCount(0),
		fmlNodesEnsemble(FML_INVALID_OBJECT_HANDLE),
		fmlElementsEnsemble(FML_INVALID_OBJECT_HANDLE),
		verbose(false)
	{
		for (int i = 0; i < numLibraryShapes; i++)
		{
			fe_element_shapes[i] = NULL;
		}
	}
	
	~FieldMLReader()
	{
		for (EvaluatorElementFieldComponentMapIterator iter = componentMap.begin(); iter != componentMap.end(); iter++)
		{
			DESTROY(FE_element_field_component)(&(iter->second.fe_element_field_component));
		}
		for (int i = 0; i < connectivityCount; i++)
		{
			Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id*>(&(connectivityData[i].localNodeEnsemble)));
		}
		delete[] connectivityData;
		for (int i = 0; i < numLibraryShapes; i++)
		{
			if (fe_element_shapes[i])
			{
				DEACCESS(FE_element_shape)(&(fe_element_shapes[i]));
			}
		}
		Fieldml_Destroy(fmlHandle);
		Cmiss_field_module_destroy(&field_module);
		DEACCESS(FE_region)(&fe_region);
		Cmiss_region_destroy(&region);
	}

	Cmiss_field_ensemble_id getEnsemble(FmlObjectHandle fmlEnsemble, bool create=false);

	int readEnsembles();

	int readSemiDenseParameters(FmlObjectHandle fmlParameters,
		Cmiss_field_real_parameters_id realParameters, const char *name,
		int indexEnsembleCount, Cmiss_field_ensemble_id *indexEnsembles);

	int readParameters();
	
	FE_element_shape *getElementShapeFromName(const char *shapeName);

	int readConnectivityData();

	int readMeshes();

	FE_element_field_component *get_FE_element_field_component(FmlObjectHandle fmlEvaluator,
		FmlObjectHandle& fmlParametersSource);

	int readAggregates();

	/** @return  1 on success, 0 on failure */
	int parse();
};

/**
 * @param create  If true, ensemble is created if not found.
 * @return  ensemble matching handle */
Cmiss_field_ensemble_id FieldMLReader::getEnsemble(FmlObjectHandle fmlEnsemble, bool create)
{
	if (Fieldml_GetObjectType(fmlHandle, fmlEnsemble) != FHT_ENSEMBLE_DOMAIN)
		return 0;
	const char *name = Fieldml_GetObjectName(fmlHandle, fmlEnsemble);
	Cmiss_field_id ensemble_field = Cmiss_field_module_find_field_by_name(field_module, name);
	if ((!ensemble_field) && !create)
		return 0;
	if (!ensemble_field)
	{
		ensemble_field = Cmiss_field_module_create_ensemble(field_module);
		Cmiss_field_set_name(ensemble_field, name);
		Cmiss_field_set_persistent(ensemble_field, 1);
	}
	Cmiss_field_ensemble_id ensemble = Cmiss_field_cast_ensemble(ensemble_field);
	if (!ensemble)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Cannot merge ensemble into existing field %s of different type", name);
	}
	Cmiss_field_destroy(&ensemble_field);
	return ensemble;
}

int FieldMLReader::readEnsembles()
{
	int return_code = 1;
	const int ensembleCount = Fieldml_GetObjectCount(fmlHandle, FHT_ENSEMBLE_DOMAIN);
	for (int ensembleIndex = 1; (ensembleIndex <= ensembleCount) && return_code; ensembleIndex++)
	{
		FmlObjectHandle fmlEnsemble = Fieldml_GetObject(fmlHandle, FHT_ENSEMBLE_DOMAIN, ensembleIndex);
		if (fmlEnsemble == FML_INVALID_OBJECT_HANDLE)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Invalid ensemble handle %d",fmlEnsemble);
			return_code = 0;
		}
		const char *name = Fieldml_GetObjectName(fmlHandle, fmlEnsemble);
		DomainBoundsType fmlBounds = Fieldml_GetDomainBoundsType(fmlHandle, fmlEnsemble);
		unsigned int entryCount = Fieldml_GetEnsembleDomainElementCount(fmlHandle, fmlEnsemble);
		if (verbose)
		{
			display_message(INFORMATION_MESSAGE, "Reading ensemble %s, count %d, bounds %d\n", name, entryCount, fmlBounds);
		}

		// read markup
		const int markupCount = Fieldml_GetMarkupCount(fmlHandle, fmlEnsemble);
		for (int markupIndex = 1; markupIndex <= markupCount; markupIndex++)
		{
			const char *markupAttribute = Fieldml_GetMarkupAttribute(fmlHandle, fmlEnsemble, markupIndex);
			const char *markupValue = Fieldml_GetMarkupValue(fmlHandle, fmlEnsemble, markupIndex);
			if (verbose)
			{
				display_message(INFORMATION_MESSAGE, "    Markup %s = %s\n", markupAttribute, markupValue);
			}
		}

		switch (fmlBounds)
		{
		case BOUNDS_DISCRETE_CONTIGUOUS:
		{
			Cmiss_field_ensemble_id ensemble = getEnsemble(fmlEnsemble, /*create*/true);
			if (ensemble)
			{
				Cmiss_ensemble_identifier identifier;
				for (identifier = 1; (identifier <= entryCount) && return_code; identifier++)
				{
					Cmiss_ensemble_iterator_id iterator =
						Cmiss_field_ensemble_find_or_create_entry(ensemble, identifier);
					if (!iterator)
					{
						return_code = 0;
					}
					Cmiss_ensemble_iterator_destroy(&iterator);
				}
				Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id*>(&ensemble));
			}
			break;
		}
		case BOUNDS_UNKNOWN:
		case BOUNDS_DISCRETE_ARBITRARY:
		default:
			display_message(ERROR_MESSAGE, "Read FieldML:  Unsupported bounds type %d for ensemble %s", fmlBounds, name);
			return_code = 0;
			break;
		}
	}
	return return_code;
}

int FieldMLReader::readSemiDenseParameters(FmlObjectHandle fmlParameters,
	Cmiss_field_real_parameters_id realParameters, const char *name,
	int indexEnsembleCount, Cmiss_field_ensemble_id *indexEnsembles)
{
	DataDescriptionType dataDescription = Fieldml_GetParameterDataDescription(fmlHandle, fmlParameters);
	if (dataDescription != DESCRIPTION_SEMIDENSE)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Unknown data description type %d for real parameters %s", dataDescription, name);
		return 0;
	}
	const int sparseIndexCount = Fieldml_GetSemidenseIndexCount(fmlHandle, fmlParameters, /*isSparse*/1);
	if (0 < sparseIndexCount)
	{
		display_message(WARNING_MESSAGE, "Read FieldML:  Can't read sparsely-indexed continuous parameters yet");
		return 0;
	}
	const int denseIndexCount = Fieldml_GetSemidenseIndexCount(fmlHandle, fmlParameters, /*isSparse*/0);
	// FieldML reader currently only reads slices of the innermost dense ensemble
	// It returns indexes for the sparse ensembles, not for intermediate ensembles which
	// must be iterated over
	unsigned int realBufferSize = 1;
	if (0 < denseIndexCount)
	{
		realBufferSize = Cmiss_field_ensemble_get_size(indexEnsembles[indexEnsembleCount - 1]);
	}
	if (realBufferSize == 0)
		return 0;

	int return_code = 1;

	FmlReaderHandle fmlReader = Fieldml_OpenReader(fmlHandle, fmlParameters);
	if (fmlReader == 0)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Could not open reader for continuous parameters %s", name);
		return 0;
	}
	Cmiss_ensemble_index_id index = Cmiss_field_real_parameters_create_index(realParameters);
	const int numIterators = indexEnsembleCount - 1;
	Cmiss_ensemble_iterator_id *iterators = new Cmiss_ensemble_iterator_id[numIterators];
	int i;
	for (i = 0; i < numIterators; i++)
	{
		iterators[i] = Cmiss_field_ensemble_get_first_entry(indexEnsembles[i]);
		Cmiss_ensemble_index_set_entry(index, iterators[i]);
	}
	int *indexBuffer = new int[sparseIndexCount];
	double *valueBuffer = new double[realBufferSize];
	
	bool lastSlice = (numIterators == 0);
	do
	{
		for (i = numIterators - 1; 0 <= i; i--)
		{
			Cmiss_ensemble_index_set_entry(index, iterators[i]);
			if (Cmiss_ensemble_iterator_increment(iterators[i]))
			{
				break;
			}
			else
			{
				if (i == 0)
				{
					lastSlice = true;
					break;
				}
				iterators[i] = Cmiss_field_ensemble_get_first_entry(indexEnsembles[i]);
			}
		}
		int result = Fieldml_ReadDoubleSlice(fmlHandle, fmlReader, indexBuffer, valueBuffer);
		if (result == FML_ERR_NO_ERROR)
		{
			if (!Cmiss_field_real_parameters_set_values(realParameters, index, realBufferSize, valueBuffer))
			{
				display_message(WARNING_MESSAGE, "Read FieldML:  Cmiss_field_real_parameters_set_values failed");
				return_code = 0;
				break;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE, "Read FieldML:  Fieldml_ReadDoubleSlice failed with code %d", result);
			return_code = 0;
			break;
		}
	}
	while (!lastSlice);

	Fieldml_CloseReader(fmlHandle, fmlReader);
	delete[] valueBuffer;
	delete[] indexBuffer;
	for (int i = 0; i < numIterators; i++)
	{
		Cmiss_ensemble_iterator_destroy(&(iterators[i]));
	}
	delete[] iterators;
	Cmiss_ensemble_index_destroy(&index);
	return return_code;
}

int FieldMLReader::readParameters()
{
	int return_code = 1;
	const int parametersCount = Fieldml_GetObjectCount(fmlHandle, FHT_CONTINUOUS_PARAMETERS);
	for (int parametersIndex = 1; (parametersIndex <= parametersCount) && return_code; parametersIndex++)
	{
		FmlObjectHandle fmlParameters = Fieldml_GetObject(fmlHandle, FHT_CONTINUOUS_PARAMETERS, parametersIndex);
		if (fmlParameters == FML_INVALID_OBJECT_HANDLE)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Invalid continuous parameters handle %d",fmlParameters);
			return_code = 0;
		}
		const char *name = Fieldml_GetObjectName(fmlHandle, fmlParameters);
		if (verbose)
		{
			display_message(INFORMATION_MESSAGE, "Reading continuous parameters %s\n", name);
		}

		int indexEnsembleCount = Fieldml_GetIndexCount(fmlHandle, fmlParameters);
		Cmiss_field_ensemble_id *indexEnsembles = new Cmiss_field_ensemble_id[indexEnsembleCount];
		
		for (int indexEnsembleIndex = 1; indexEnsembleIndex <= indexEnsembleCount; indexEnsembleIndex++)
		{
			FmlObjectHandle fmlIndexEnsemble = Fieldml_GetIndexDomain(fmlHandle, fmlParameters, indexEnsembleIndex);
			const char *ensembleName = Fieldml_GetObjectName(fmlHandle, fmlIndexEnsemble);
			if (verbose)
			{
				display_message(INFORMATION_MESSAGE, "  Index ensemble %d = %s\n", indexEnsembleIndex, ensembleName);
			}
			indexEnsembles[indexEnsembleCount - indexEnsembleIndex] = getEnsemble(fmlIndexEnsemble);
		}
		// read markup
		const int markupCount = Fieldml_GetMarkupCount(fmlHandle, fmlParameters);
		for (int markupIndex = 1; markupIndex <= markupCount; markupIndex++)
		{
			const char *markupAttribute = Fieldml_GetMarkupAttribute(fmlHandle, fmlParameters, markupIndex);
			const char *markupValue = Fieldml_GetMarkupValue(fmlHandle, fmlParameters, markupIndex);
			if (verbose)
			{
				display_message(INFORMATION_MESSAGE, "    Markup %s = %s\n", markupAttribute, markupValue);
			}
		}
		
		Cmiss_field_id realParametersField = Cmiss_field_module_find_field_by_name(field_module, name);
		if (!realParametersField)
		{
			realParametersField = Cmiss_field_module_create_real_parameters(field_module, indexEnsembleCount, indexEnsembles);
			Cmiss_field_set_name(realParametersField, name);
			Cmiss_field_set_persistent(realParametersField, 1);
		}
		Cmiss_field_real_parameters_id realParameters = Cmiss_field_cast_real_parameters(realParametersField);
		if (realParameters)
		{
			// check field is using same index ensembles in same order
			Cmiss_ensemble_index_id index = Cmiss_field_real_parameters_create_index(realParameters);
			if (!Cmiss_ensemble_index_has_index_ensembles(index, indexEnsembleCount, indexEnsembles))
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  Cannot merge into existing real parameters field %s with different index ensembles", name);
				return_code = 0;
			}
			Cmiss_ensemble_index_destroy(&index);
		}
		else
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Cannot merge real parameters into existing field %s of different type", name);
		}
		if (return_code)
		{
			return_code = readSemiDenseParameters(fmlParameters, realParameters, name, indexEnsembleCount, indexEnsembles);
		}
		Cmiss_field_destroy(&realParametersField);
		for (int i = 0; i < indexEnsembleCount; i++)
		{
			Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id*>(&indexEnsembles[i]));
		}
		delete[] indexEnsembles;
		Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id*>(&realParameters));
	}
	return return_code;
}

FE_element_shape *FieldMLReader::getElementShapeFromName(const char *shapeName)
{
	for (int i = 0; i < numLibraryShapes; i++)
	{
		if (0 == strcmp(shapeName, libraryShapes[i].fieldmlName))
		{
			if (!fe_element_shapes[i])
			{
				fe_element_shapes[i] = ACCESS(FE_element_shape)(
					CREATE(FE_element_shape)(libraryShapes[i].dimension,
						libraryShapes[i].shape_type, fe_region));				
			}
			return fe_element_shapes[i];
		}
	}
	display_message(ERROR_MESSAGE, "Read FieldML:  Unrecognised shape %s", shapeName);
	return NULL;
}

/** reads into FE_element nodes; call only after setting up ConnectivityData in readMeshes */
int FieldMLReader::readConnectivityData()
{
	if (verbose)
	{
		display_message(INFORMATION_MESSAGE, "Reading connectivity count= %d\n", connectivityCount);
	}
	int return_code = 1;
	ConnectivityData *connectivity = connectivityData;
	for (int connectivityIndex = 1; connectivityIndex <= connectivityCount; connectivityIndex++)
	{
		FmlObjectHandle fmlEnsembleParameters = connectivity->fmlConnectivitySource;
		const char *name = Fieldml_GetObjectName(fmlHandle, fmlEnsembleParameters);
		if (FHT_ENSEMBLE_PARAMETERS != Fieldml_GetObjectType(fmlHandle, fmlEnsembleParameters))
		{
			display_message(ERROR_MESSAGE, "Read FieldML: Connectivity source %s is not ensemble parameters type", name);
			return_code = 0;
			break;
		}
		int indexEnsembleCount = Fieldml_GetIndexCount(fmlHandle, fmlEnsembleParameters);
		if (2 != indexEnsembleCount)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Connectivity source %s not indexed by 2 ensembles", name);
			return_code = 0;
			break;
		}
		if (Fieldml_GetIndexDomain(fmlHandle, fmlEnsembleParameters, 2) != fmlElementsEnsemble)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Connectivity source %s does not have mesh elements ensemble as outer index", name);
			return_code = 0;
			break;
		}
		DataDescriptionType dataDescription = Fieldml_GetParameterDataDescription(fmlHandle, fmlEnsembleParameters);
		if (dataDescription != DESCRIPTION_SEMIDENSE)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Unknown data description type %d for ensemble parameters %s", dataDescription, name);
			return_code = 0;
			break;
		}
		const int sparseIndexCount = Fieldml_GetSemidenseIndexCount(fmlHandle, fmlEnsembleParameters, /*isSparse*/1);
		if (0 < sparseIndexCount)
		{
			display_message(WARNING_MESSAGE, "Read FieldML:  Can't read sparsely-indexed ensemble parameters yet");
			return 0;
			break;
		}

		if (verbose)
		{
			display_message(INFORMATION_MESSAGE, "Reading ensemble parameters %s into element nodes from index %d\n", name,
				connectivity->firstLocalNode);
		}

		FmlReaderHandle fmlReader = Fieldml_OpenReader(fmlHandle, fmlEnsembleParameters);
		if (fmlReader == (FmlReaderHandle)FML_INVALID_HANDLE)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Could not open reader for ensemble parameters %s", name);
			return 0;
			break;
		}

		Cmiss_field_ensemble_id *indexEnsembles = new Cmiss_field_ensemble_id[indexEnsembleCount];
		for (int indexEnsembleIndex = 1; indexEnsembleIndex <= indexEnsembleCount; indexEnsembleIndex++)
		{
			FmlObjectHandle fmlIndexEnsemble = Fieldml_GetIndexDomain(fmlHandle, fmlEnsembleParameters, indexEnsembleIndex);
			const char *ensembleName = Fieldml_GetObjectName(fmlHandle, fmlIndexEnsemble);
			if (verbose)
			{
				display_message(INFORMATION_MESSAGE, "  Index ensemble %d = %s\n", indexEnsembleIndex, ensembleName);
			}
			indexEnsembles[indexEnsembleCount - indexEnsembleIndex] = getEnsemble(fmlIndexEnsemble);
		}

		Cmiss_ensemble_iterator_id elementsIterator = Cmiss_field_ensemble_get_first_entry(indexEnsembles[0]);
		if (!elementsIterator)
		{
			return_code = 0;
		}
		int numberOfLocalNodes = (int)Cmiss_field_ensemble_get_size(indexEnsembles[1]);
		int *indexBuffer = new int[sparseIndexCount];
		int *valueBuffer = new int[numberOfLocalNodes];
		CM_element_information cm;
		cm.type = CM_ELEMENT;
		struct FE_node *fe_node;
		struct FE_element *fe_element;
		int localNode;
		while (return_code)
		{
			Cmiss_ensemble_identifier elementIdentifier = Cmiss_ensemble_iterator_get_identifier(elementsIterator);
			cm.number = static_cast<int>(elementIdentifier);
			fe_element = FE_region_get_FE_element_from_identifier(fe_region, &cm);
			ACCESS(FE_element)(fe_element);
			int result = Fieldml_ReadIntSlice(fmlHandle, fmlReader, indexBuffer, valueBuffer);
			if (result == FML_ERR_NO_ERROR)
			{
				for (localNode = 0; localNode < numberOfLocalNodes; localNode++)
				{
					fe_node = FE_region_get_FE_node_from_identifier(fe_region, valueBuffer[localNode]);
					if (fe_node)
					{
						if (!set_FE_element_node(fe_element, connectivity->firstLocalNode + localNode, fe_node))
						{
							return_code = 0;
							break;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE, "Read FieldML:  Ensemble parameters %s refers to nonexistent entry %d",
							name, valueBuffer[localNode]);
						return_code = 0;
						break;
					}
				}
			}
			else
			{
				display_message(WARNING_MESSAGE, "Read FieldML:  Fieldml_ReadIntSlice failed with code %d", result);
				return_code = 0;
			}
			DEACCESS(FE_element)(&fe_element);
			if (!Cmiss_ensemble_iterator_increment(elementsIterator))
				break;
		}
		delete[] valueBuffer;
		delete[] indexBuffer;
		Cmiss_ensemble_iterator_destroy(&elementsIterator);
		for (int i = 0; i < indexEnsembleCount; i++)
		{
			Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id*>(&indexEnsembles[i]));
		}
		delete[] indexEnsembles;
		Fieldml_CloseReader(fmlHandle, fmlReader);
		connectivity++;
	}
	return (return_code);
}

int FieldMLReader::readMeshes()
{
	const int meshCount = Fieldml_GetObjectCount(fmlHandle, FHT_MESH_DOMAIN);
	if (meshCount != 1)
	{
		display_message(ERROR_MESSAGE, "Read FieldML:  Require 1 mesh, %d found", meshCount);
		return 0;
	}
	int return_code = 1;
	for (int meshIndex = 1; (meshIndex <= meshCount) && return_code; meshIndex++)
	{
		FmlObjectHandle fmlMesh = Fieldml_GetObject(fmlHandle, FHT_MESH_DOMAIN, meshIndex);
		if (fmlMesh == FML_INVALID_OBJECT_HANDLE)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Invalid mesh handle %d", fmlMesh);
			return_code = 0;
			break;
		}
		const char *name = Fieldml_GetObjectName(fmlHandle, fmlMesh);

		FmlObjectHandle fmlMeshXi = Fieldml_GetMeshXiDomain(fmlHandle, fmlMesh);
		FmlObjectHandle fmlMeshXiComponentEnsemble = Fieldml_GetDomainComponentEnsemble(fmlHandle, fmlMeshXi);
		if (fmlMeshXiComponentEnsemble == FML_INVALID_OBJECT_HANDLE)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Invalid xi domain for mesh %s", name);
			return_code = 0;
			break;
		}
		else
		{
			meshDimension = Fieldml_GetEnsembleDomainElementCount(fmlHandle, fmlMeshXiComponentEnsemble);
			if ((meshDimension < 1) || (meshDimension > 3))
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  Invalid dimension %d for mesh %s", meshDimension, name);
				return_code = 0;
				break;
			}
		}

		if (verbose)
		{
			display_message(INFORMATION_MESSAGE, "Reading mesh %s dimension %d\n", name, meshDimension);
		}

		// Read connectivity sources information & determine nodes ensemble
		
		connectivityCount = Fieldml_GetMeshConnectivityCount(fmlHandle, fmlMesh);
		connectivityData = new ConnectivityData[connectivityCount];
		int maxlocalNodeCount = 0;
		ConnectivityData *connectivity = connectivityData;
		fmlNodesEnsemble = FML_INVALID_OBJECT_HANDLE;
		for (int connectivityIndex = 1; connectivityIndex <= connectivityCount; connectivityIndex++)
		{
			connectivity->firstLocalNode = maxlocalNodeCount;
			connectivity->fmlConnectivityDomain =
				Fieldml_GetMeshConnectivityDomain(fmlHandle, fmlMesh, connectivityIndex);
			connectivity->fmlConnectivitySource =
				Fieldml_GetMeshConnectivitySource(fmlHandle, fmlMesh, connectivityIndex);
			if (connectivityIndex == 1)
			{
				fmlNodesEnsemble = Fieldml_GetValueDomain(fmlHandle, connectivity->fmlConnectivitySource);
			}
			else if (Fieldml_GetValueDomain(fmlHandle, connectivity->fmlConnectivitySource) != fmlNodesEnsemble)
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  Mesh point connectivities must use a common nodes domain");
				return_code = 0;
			}
			if (verbose)
			{
				display_message(INFORMATION_MESSAGE, "  Connectivity %d: %s -> %s\n", connectivityIndex,
					Fieldml_GetObjectName(fmlHandle, connectivity->fmlConnectivityDomain),
					Fieldml_GetObjectName(fmlHandle, connectivity->fmlConnectivitySource));
			}
			connectivity->localNodeEnsemble = getEnsemble(connectivity->fmlConnectivityDomain);
			if (!connectivity->localNodeEnsemble)
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  Invalid mesh connectivity local nodes domain");
				return_code = 0;
			}
			maxlocalNodeCount += Cmiss_field_ensemble_get_size(connectivity->localNodeEnsemble);
			connectivity++;
		}

		// Create nodes with no fields

		if (verbose)
		{
			display_message(INFORMATION_MESSAGE, "Reading nodes from %s\n",
				Fieldml_GetObjectName(fmlHandle, fmlNodesEnsemble));
		}
		Cmiss_field_ensemble_id nodesEnsemble = getEnsemble(fmlNodesEnsemble);
		Cmiss_ensemble_iterator *nodesIterator = Cmiss_field_ensemble_get_first_entry(nodesEnsemble);
		if (!nodesIterator)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Could not determine nodes ensemble from mesh connectivity");
			return_code = 0;
		}
		int cm_node_identifier;
		while (return_code)
		{
			Cmiss_ensemble_identifier nodeIdentifier = Cmiss_ensemble_iterator_get_identifier(nodesIterator);
			cm_node_identifier = static_cast<int>(nodeIdentifier);
			FE_node *fe_node = CREATE(FE_node)(cm_node_identifier, fe_region, /*template*/NULL);
			ACCESS(FE_node)(fe_node);
			if (!FE_region_merge_FE_node(fe_region, fe_node))
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  Could not merge node into region");
				return_code = 0;
				break;
			}
			DEACCESS(FE_node)(&fe_node);
			if (!Cmiss_ensemble_iterator_increment(nodesIterator))
				break;
		}
		Cmiss_ensemble_iterator_destroy(&nodesIterator);
		Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id*>(&nodesEnsemble));
		
		// Create elements with shapes and maxlocalNodeCount, but no fields

		fmlElementsEnsemble = Fieldml_GetMeshElementDomain(fmlHandle, fmlMesh);
		if (verbose)
		{
			display_message(INFORMATION_MESSAGE, "Reading elements from %s\n",
				Fieldml_GetObjectName(fmlHandle, fmlElementsEnsemble));
		}
		Cmiss_field_ensemble_id elementsEnsemble = getEnsemble(fmlElementsEnsemble);
		// make FE_elements out of elements ensemble with shape from mesh
		CM_element_information cm;
		cm.type = CM_ELEMENT;
		Cmiss_ensemble_iterator *elementsIterator = Cmiss_field_ensemble_get_first_entry(elementsEnsemble);
		if (!elementsIterator)
		{
			return_code = 0;
		}
		while (return_code)
		{
			Cmiss_ensemble_identifier elementIdentifier = Cmiss_ensemble_iterator_get_identifier(elementsIterator);
			const char *shapeName = Fieldml_GetMeshElementShape(fmlHandle, fmlMesh, static_cast<int>(elementIdentifier), /*allowDefault*/1);
			FE_element_shape *fe_element_shape = getElementShapeFromName(shapeName);
			cm.number = static_cast<int>(elementIdentifier);
			FE_element *fe_element = CREATE(FE_element)(&cm, fe_element_shape, fe_region, /*template*/NULL);
			ACCESS(FE_element)(fe_element);
			set_FE_element_number_of_nodes(fe_element, maxlocalNodeCount);
			if (!FE_region_merge_FE_element(fe_region, fe_element))
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  Could not merge element into region");
				return_code = 0;
				break;
			}
			DEACCESS(FE_element)(&fe_element);
			if (!Cmiss_ensemble_iterator_increment(elementsIterator))
				break;
		}
		Cmiss_ensemble_iterator_destroy(&elementsIterator);
		Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id*>(&elementsEnsemble));

		// Read connectivity data into element nodes
		
	}
	return return_code;
}

FE_element_field_component *FieldMLReader::get_FE_element_field_component(FmlObjectHandle fmlEvaluator,
	FmlObjectHandle& fmlParametersSource)
{
	EvaluatorElementFieldComponentMapIterator iter = componentMap.find(fmlEvaluator);
	if (iter != componentMap.end())
	{
		fmlParametersSource = iter->second.fmlParametersSource;
		return iter->second.fe_element_field_component;
	}
	FE_element_field_component *fe_element_field_component = NULL;
	const char *evaluatorName = Fieldml_GetObjectName(fmlHandle, fmlEvaluator);
	if (FHT_CONTINUOUS_REFERENCE == Fieldml_GetObjectType(fmlHandle, fmlEvaluator))
	{
		FmlObjectHandle fmlBasisEvaluator = Fieldml_GetReferenceRemoteEvaluator(fmlHandle, fmlEvaluator);
		const char *basisName = Fieldml_GetObjectName(fmlHandle, fmlBasisEvaluator);
		FmlObjectHandle fmlRemoteXiDomain = FML_INVALID_OBJECT_HANDLE;
		FmlObjectHandle fmlRemoteParametersDomain = FML_INVALID_OBJECT_HANDLE;
		FmlObjectHandle fmlLocalNodesDomain = FML_INVALID_OBJECT_HANDLE;
		FE_basis *fe_basis = NULL;
		for (int i = 0; i < numLibraryBases; i++)
		{
			if (0 == strcmp(basisName, libraryBases[i].fieldmlBasis))
			{
				int *basis_type_copy = new int[7];
				for (int j = 0; j < 7; j++)
				{
					basis_type_copy[j] = libraryBases[i].basis_type[j];
				}
				fe_basis = ACCESS(FE_basis)(FE_region_get_FE_basis_matching_basis_type(fe_region, basis_type_copy));
				delete[] basis_type_copy;
				fmlRemoteXiDomain = Fieldml_GetNamedObject(fmlHandle, libraryBases[i].fieldmlXiDomain);
				fmlRemoteParametersDomain = Fieldml_GetNamedObject(fmlHandle, libraryBases[i].fieldmlParamatersDomain);
				fmlLocalNodesDomain = Fieldml_GetNamedObject(fmlHandle, libraryBases[i].fieldmlLocalNodesDomain);
				break;
			}
		}
		FmlObjectHandle fmlLocalXiDomain = Fieldml_GetAliasByRemote(fmlHandle, fmlEvaluator, fmlRemoteXiDomain);
		FmlObjectHandle fmlLocalParametersDomain = Fieldml_GetAliasByRemote(fmlHandle, fmlEvaluator, fmlRemoteParametersDomain);
		if ((fe_basis) && (fmlRemoteXiDomain != FML_INVALID_OBJECT_HANDLE) &&
			(fmlRemoteParametersDomain != FML_INVALID_OBJECT_HANDLE) &&
			(fmlLocalNodesDomain != FML_INVALID_OBJECT_HANDLE) &&
			(2 == Fieldml_GetAliasCount(fmlHandle, fmlEvaluator)) &&
			(fmlLocalXiDomain != FML_INVALID_OBJECT_HANDLE) &&
			(fmlLocalParametersDomain != FML_INVALID_OBJECT_HANDLE))
		{
			// GRC: exactly one mesh expected
			FmlObjectHandle fmlMesh = Fieldml_GetObject(fmlHandle, FHT_MESH_DOMAIN, /*meshIndex*/1);
			FmlObjectHandle fmlMeshXiDomain = Fieldml_GetMeshXiDomain(fmlHandle, fmlMesh);
			if ((1 != Fieldml_GetObjectCount(fmlHandle, FHT_MESH_DOMAIN)) || (fmlLocalXiDomain != fmlMeshXiDomain))
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  ImportedEvaluator %s (for FEM basis %s) does not alias %s to mesh xi",
					evaluatorName, basisName, Fieldml_GetObjectName(fmlHandle, fmlLocalXiDomain));
			}
			else
			{
				fmlParametersSource = Fieldml_GetReferenceRemoteEvaluator(fmlHandle, fmlLocalParametersDomain);
				FieldmlHandleType parameterSourceType = Fieldml_GetObjectType(fmlHandle, fmlParametersSource);
				if ((parameterSourceType == FHT_CONTINUOUS_VARIABLE) ||
					(parameterSourceType == FHT_CONTINUOUS_PARAMETERS))
				{
					FmlObjectHandle fmlNodesSource = Fieldml_GetAliasByRemote(fmlHandle, fmlLocalParametersDomain, fmlNodesEnsemble);
					if ((1 == Fieldml_GetAliasCount(fmlHandle, fmlLocalParametersDomain)) && (fmlNodesSource != FML_INVALID_OBJECT_HANDLE))
					{
						int firstLocalNode = -1;
						for (int i = 0; i < connectivityCount; i++)
						{
							if (connectivityData[i].fmlConnectivitySource == fmlNodesSource)
							{
								firstLocalNode = connectivityData[i].firstLocalNode;
							}
						}
						if (0 <= firstLocalNode)
						{
							int nodeCount = Fieldml_GetEnsembleDomainElementCount(fmlHandle, fmlLocalNodesDomain);
							if (verbose)
							{
								display_message(INFORMATION_MESSAGE, "---> Read FieldML:  Basis %s, map parameters from %s, %d nodes\n",
										basisName, Fieldml_GetObjectName(fmlHandle, fmlParametersSource), nodeCount);
							}
							fe_element_field_component = CREATE(FE_element_field_component)(
								STANDARD_NODE_TO_ELEMENT_MAP, nodeCount, fe_basis, (FE_element_field_component_modify)NULL);
							if (fe_element_field_component)
							{
								for (int nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
								{
									Standard_node_to_element_map *standard_node_map =
										CREATE(Standard_node_to_element_map)(firstLocalNode + nodeIndex, /*number_of_values*/1);
									if (!(Standard_node_to_element_map_set_nodal_value_index(
											standard_node_map, /*value_number*/0, /*nodal_value_index*/0) &&
										Standard_node_to_element_map_set_scale_factor_index(
											standard_node_map, /*value_number*/0, /*no_scale_factor*/-1) &&
										FE_element_field_component_set_standard_node_map(fe_element_field_component,
											nodeIndex, standard_node_map)))
									{
										DESTROY(FE_element_field_component)(&fe_element_field_component);
										fe_element_field_component = NULL;
										break;
									}
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE, "Read FieldML:  Parameters source %s has mapping %s not listed under mesh connectivity ",
								Fieldml_GetObjectName(fmlHandle, fmlLocalParametersDomain),
								Fieldml_GetObjectName(fmlHandle, fmlNodesSource));
						}
					}
					else
					{
						display_message(ERROR_MESSAGE, "Read FieldML:  Unsupported basis parameters mapping %s",
							Fieldml_GetObjectName(fmlHandle, fmlLocalParametersDomain));
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "Read FieldML:  ImportedEvaluator %s does not reference a continuous variable or parameters evaluator",
						Fieldml_GetObjectName(fmlHandle, fmlParametersSource));
				}
			}
		}
		REACCESS(FE_basis)(&fe_basis, NULL);
	}
	if (fe_element_field_component)
	{
		ElementFieldComponent temp;
		temp.fe_element_field_component = fe_element_field_component;
		temp.fmlParametersSource = fmlParametersSource;
		componentMap[fmlEvaluator] = temp;
	}
	return fe_element_field_component;
}

/** aggregate evaluators are interpreted as finite element fields */
int FieldMLReader::readAggregates()
{
	const int aggregateCount = Fieldml_GetObjectCount(fmlHandle, FHT_CONTINUOUS_AGGREGATE);
	int return_code = 1;
	for (int aggregateIndex = 1; (aggregateIndex <= aggregateCount) && return_code; aggregateIndex++)
	{
		FmlObjectHandle fmlAggregate = Fieldml_GetObject(fmlHandle, FHT_CONTINUOUS_AGGREGATE, aggregateIndex);
		if (fmlAggregate == FML_INVALID_OBJECT_HANDLE)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Invalid aggregate handle %d", fmlAggregate);
			return_code = 0;
			break;
		}
		const char *fieldName = Fieldml_GetObjectName(fmlHandle, fmlAggregate);
		FmlObjectHandle fmlValueDomain = Fieldml_GetValueDomain(fmlHandle, fmlAggregate);
		FmlObjectHandle fmlValueDomainComponentEnsemble =
			Fieldml_GetDomainComponentEnsemble(fmlHandle, fmlValueDomain);
		const int componentCount = (fmlValueDomainComponentEnsemble == FML_INVALID_OBJECT_HANDLE) ? 1 :
				Fieldml_GetEnsembleDomainElementCount(fmlHandle, fmlValueDomainComponentEnsemble);
		const int aggregateDelegationCount = Fieldml_GetEvaluatorCount(fmlHandle, fmlAggregate);
		if (aggregateDelegationCount != componentCount)
		{
			display_message(ERROR_MESSAGE,
				"Read FieldML:  Aggregate %s number of source field delegations %d "
				"is not equal to number of components from value type %d",
				fieldName, aggregateDelegationCount, componentCount);
			return_code = 0;
			break;
		}
		// read markup
		bool isField = false;
		const int markupCount = Fieldml_GetMarkupCount(fmlHandle, fmlAggregate);
		for (int markupIndex = 1; markupIndex <= markupCount; markupIndex++)
		{
			const char *markupAttribute = Fieldml_GetMarkupAttribute(fmlHandle, fmlAggregate, markupIndex);
			const char *markupValue = Fieldml_GetMarkupValue(fmlHandle, fmlAggregate, markupIndex);
			if ((0 == strcmp(markupAttribute, "field")) && (0 == strcmp(markupValue, "true")))
			{
				isField = true;
			}
		}
		if (!isField)
		{
			display_message(WARNING_MESSAGE,
				"Read FieldML:  Aggregate %s does not have markup \"field\" = \"true\". Skipping.",
				fieldName);
			continue;
		}

		// check aggregate components are piecewise over elements

		FmlObjectHandle *aggregateComponents = new FmlObjectHandle[componentCount];
		int componentIndex;
		bool piecewiseOverElements = true;
		for (componentIndex = 1; componentIndex <= componentCount; componentIndex++)
		{
			FmlObjectHandle aggregateComponent = Fieldml_GetEvaluator(fmlHandle, fmlAggregate, componentIndex);
			aggregateComponents[componentIndex-1] = aggregateComponent;
			if (aggregateComponent == FML_INVALID_OBJECT_HANDLE)
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  Aggregate %s component %d evaluator is missing", fieldName, componentIndex);
				return_code = 0;				
			}
			else if ((FHT_CONTINUOUS_PIECEWISE != Fieldml_GetObjectType(fmlHandle, aggregateComponent)) ||
				(Fieldml_GetIndexDomain(fmlHandle, aggregateComponent, /*evaluatorIndex*/1) != fmlElementsEnsemble))
			{
				display_message(WARNING_MESSAGE, "Read FieldML:  Aggregate %s component %d is not piecewise over mesh elements ensemble",
					fieldName, componentIndex);
				piecewiseOverElements = false;
			}
		}
		if (!return_code)
			break;
		if (!piecewiseOverElements)
		{
			display_message(WARNING_MESSAGE,
				"Read FieldML:  Aggregate %s is not of a form that can be read into a field. Skipping.",
				fieldName);
			delete[] aggregateComponents;
			continue;
		}

		// define FE_field

		if (verbose)
		{
			display_message(INFORMATION_MESSAGE, "Reading aggregate %s, domain %s (ensemble %s), components = %d\n", fieldName,
				Fieldml_GetObjectName(fmlHandle, fmlValueDomain),
				Fieldml_GetObjectName(fmlHandle, fmlValueDomainComponentEnsemble), componentCount);
		}
		enum CM_field_type cm_field_type = CM_GENERAL_FIELD;
		if ((componentCount >= meshDimension) && (componentCount <= 3))
		{
			// GRC: not perfect logic, but there has to be at least one coordinate field to define faces
			cm_field_type = CM_COORDINATE_FIELD;
		}
		struct Coordinate_system coordinate_system;
		coordinate_system.type = RECTANGULAR_CARTESIAN;
		char *field_name_copy = duplicate_string(fieldName);
		FE_field *fe_field = FE_region_get_FE_field_with_properties(fe_region, field_name_copy, GENERAL_FE_FIELD,
			/*indexer_field*/NULL, /*number_of_indexed_values*/0, cm_field_type, &coordinate_system,
			/*value_type*/FE_VALUE_VALUE, componentCount, /*component_names*/(char **)NULL,
			/*number_of_times*/0, /*time_value_type*/FE_VALUE_VALUE,
			(struct FE_field_external_information *)NULL);
		DEALLOCATE(field_name_copy);
		ACCESS(FE_field)(fe_field);

		// define element fields

		// Will determine the parameter source from parameter mappings:
		FmlObjectHandle fmlParametersSource = FML_INVALID_OBJECT_HANDLE;
		FE_element_field_component **fe_element_field_components = new FE_element_field_component*[componentCount];
		Cmiss_field_ensemble_id elementsEnsemble = getEnsemble(fmlElementsEnsemble);
		CM_element_information cm;
		cm.type = CM_ELEMENT;
		struct FE_element *fe_element;
		Cmiss_ensemble_iterator *elementsIterator = Cmiss_field_ensemble_get_first_entry(elementsEnsemble);
		if (!elementsIterator)
		{
			return_code = 0;
		}
		while (return_code)
		{
			Cmiss_ensemble_identifier elementIdentifier = Cmiss_ensemble_iterator_get_identifier(elementsIterator);
			for (componentIndex = 0; componentIndex < componentCount; componentIndex++)
			{
				FmlObjectHandle fmlEvaluator = Fieldml_GetElementEvaluator(fmlHandle, aggregateComponents[componentIndex],
					static_cast<int>(elementIdentifier), /*allowDefault*/1);
				if (fmlEvaluator == FML_INVALID_OBJECT_HANDLE)
				{
					display_message(ERROR_MESSAGE, "Read FieldML:  Piecewise %s has no evaluator for element %d",
							Fieldml_GetObjectName(fmlHandle, aggregateComponents[componentIndex]), elementIdentifier);
					return_code = 0;
					break;
				}
				FmlObjectHandle fmlThisParametersSource = FML_INVALID_OBJECT_HANDLE;
				fe_element_field_components[componentIndex] = get_FE_element_field_component(fmlEvaluator, fmlThisParametersSource);
				if (NULL == fe_element_field_components[componentIndex])
				{
					display_message(ERROR_MESSAGE, "Read FieldML:  Aggregate %s component %d element %d evaluator %s does not reference a supported basis function or mapping",
						fieldName, componentIndex+1, elementIdentifier, Fieldml_GetObjectName(fmlHandle, fmlEvaluator));
					return_code = 0;
				}
				else if ((fmlThisParametersSource != FML_INVALID_OBJECT_HANDLE) && (fmlThisParametersSource != fmlParametersSource))
				{
					if (fmlParametersSource == FML_INVALID_OBJECT_HANDLE)
					{
						fmlParametersSource = fmlThisParametersSource;
					}
					else
					{
						display_message(ERROR_MESSAGE, "Read FieldML:  Aggregate %s has multiple parameter sources which is not yet supported",
							fieldName);
						return_code = 0;
					}
				}
			}
			if (!return_code)
				break;
			cm.number = static_cast<int>(elementIdentifier);
			fe_element = FE_region_get_FE_element_from_identifier(fe_region, &cm);
			return_code = define_FE_field_at_element(fe_element, fe_field, fe_element_field_components);
			if (!Cmiss_ensemble_iterator_increment(elementsIterator))
				break;
		}
		Cmiss_ensemble_iterator_destroy(&elementsIterator);
		Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id*>(&elementsEnsemble));
		delete[] fe_element_field_components;
		delete[] aggregateComponents;
		
		// read nodal parameters from fmlParametersSource

		if (!return_code)
			break;
		if (fmlParametersSource == FML_INVALID_OBJECT_HANDLE)
		{
			display_message(ERROR_MESSAGE, "Read FieldML:  Field %s has no parameters");
			return_code = 0;			
		}
		FmlObjectHandle fmlFieldParametersSource = fmlParametersSource;
		if (Fieldml_GetObjectType(fmlHandle, fmlParametersSource) == FHT_CONTINUOUS_VARIABLE)
		{
			fmlFieldParametersSource = Fieldml_GetAliasByRemote(fmlHandle, fmlAggregate, fmlParametersSource);
		}
		if (FHT_CONTINUOUS_PARAMETERS == Fieldml_GetObjectType(fmlHandle, fmlFieldParametersSource))
		{
			int indexCount = Fieldml_GetIndexCount(fmlHandle, fmlFieldParametersSource);
			FmlObjectHandle fmlIndexDomain1 = Fieldml_GetIndexDomain(fmlHandle, fmlFieldParametersSource, 1);
			FmlObjectHandle fmlIndexDomain2 = Fieldml_GetIndexDomain(fmlHandle, fmlFieldParametersSource, 2);
			if (((1 == indexCount) && (1 == componentCount) && (fmlNodesEnsemble == fmlIndexDomain1))
				|| ((2 == indexCount) &&
					(((fmlNodesEnsemble == fmlIndexDomain1) && (fmlValueDomainComponentEnsemble == fmlIndexDomain2)) ||
					((fmlNodesEnsemble == fmlIndexDomain2) && (fmlValueDomainComponentEnsemble == fmlIndexDomain1)))))
			{
				// Read parameters into nodes
				Cmiss_field_id realParametersField = Cmiss_field_module_find_field_by_name(field_module,
					Fieldml_GetObjectName(fmlHandle, fmlFieldParametersSource));
				Cmiss_field_real_parameters_id realParameters = Cmiss_field_cast_real_parameters(realParametersField);
				Cmiss_ensemble_index_id parametersIndex = Cmiss_field_real_parameters_create_index(realParameters);
				Cmiss_field_ensemble_id nodesEnsemble = getEnsemble(fmlNodesEnsemble);
				Cmiss_ensemble_iterator *nodesIterator = Cmiss_field_ensemble_get_first_entry(nodesEnsemble);
				if (!nodesIterator)
				{
					display_message(ERROR_MESSAGE, "Read FieldML:  Could not determine nodes ensemble from mesh connectivity");
					return_code = 0;
				}
				double *valueBuffer = new double[componentCount];
				FE_node_field_creator *fe_node_field_creator = CREATE(FE_node_field_creator)(componentCount);
				FE_node *fe_node_template = CREATE(FE_node)(/*cm_node_identifier*/1, fe_region, /*template*/NULL);
				ACCESS(FE_node)(fe_node_template);
				if (!define_FE_field_at_node(fe_node_template, fe_field, (FE_time_sequence *)NULL, fe_node_field_creator))
				{
					return_code = 0;
					break;
				}
				while (return_code)
				{
					Cmiss_ensemble_index_set_entry(parametersIndex, nodesIterator);
					Cmiss_ensemble_identifier nodeIdentifier = Cmiss_ensemble_iterator_get_identifier(nodesIterator);
					FE_node *fe_node = CREATE(FE_node)(static_cast<int>(nodeIdentifier), (FE_region *)NULL, fe_node_template);
					ACCESS(FE_node)(fe_node);
					if (!Cmiss_field_real_parameters_get_values(realParameters, parametersIndex, componentCount, valueBuffer))
					{
						display_message(ERROR_MESSAGE, "Read FieldML:  Could not extract nodal values");
						return_code = 0;
					}
					int length = 0;
					if ((!set_FE_nodal_field_FE_value_values(fe_field, fe_node, valueBuffer, &length)) ||
						(length != componentCount))
					{
						return_code = 0;
					}
					if (!FE_region_merge_FE_node(fe_region, fe_node))
					{
						return_code = 0;
					}
					DEACCESS(FE_node)(&fe_node);
					if (!Cmiss_ensemble_iterator_increment(nodesIterator))
						break;
				}
				DEACCESS(FE_node)(&fe_node_template);
				delete[] valueBuffer;
				DESTROY(FE_node_field_creator)(&fe_node_field_creator);
				Cmiss_ensemble_iterator_destroy(&nodesIterator);
				Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id*>(&nodesEnsemble));
				Cmiss_ensemble_index_destroy(&parametersIndex);
				Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id*>(&realParameters));
				Cmiss_field_destroy(&realParametersField);
			}
			else
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  Only supports parameters indexed by nodes ensemble and components");
				return_code = 0;					
			}
		}
		else
		{
			if (fmlFieldParametersSource == FML_INVALID_OBJECT_HANDLE)
			{
				display_message(WARNING_MESSAGE, "Read FieldML:  Field %s parameters are missing; not aliased from variable %s",
					fieldName, Fieldml_GetObjectName(fmlHandle, fmlParametersSource));
			}
			else
			{
				display_message(ERROR_MESSAGE, "Read FieldML:  Field %s parameters are not continuous parameters type", fieldName);
				return_code = 0;
			}
		}
		
		DEACCESS(FE_field)(&fe_field);
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
	return_code = return_code && readEnsembles();
	return_code = return_code && readParameters();
	return_code = return_code && readMeshes();
	return_code = return_code && readConnectivityData();
	return_code = return_code && readAggregates();
	Cmiss_region_end_change(region);
	return return_code;
}

} // anonymous namespace

int parse_fieldml_02_file(struct Cmiss_region *region, const char *filename)
{
	FieldMLReader fmlReader(region, filename);
	return fmlReader.parse();
}
