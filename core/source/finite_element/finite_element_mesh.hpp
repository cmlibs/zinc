/**
 * FILE : finite_element_mesh.hpp
 *
 * Class defining a domain consisting of a set of finite elements.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (FINITE_ELEMENT_MESH_HPP)
#define FINITE_ELEMENT_MESH_HPP

#include "opencmiss/zinc/status.h"
#include "datastore/labels.hpp"
#include "datastore/labelschangelog.hpp"
#include "datastore/maparray.hpp"
#include "finite_element/element_field_template.hpp"
#include "finite_element/finite_element.h"
#include "general/block_array.hpp"
#include "general/list.h"
#include <list>
#include <set>
#include <vector>

class FE_mesh;

struct cmzn_mesh_scale_factor_set;

/** Handle to a set of scale factors for a mesh.
 * Scale factors are used to scale global field parameters before use.
 * Actual values are stored under this handle in each element. */
typedef struct cmzn_mesh_scale_factor_set *cmzn_mesh_scale_factor_set_id;

/**
 * Identifier of set of scale factors, under which scale factors are stored,
 * e.g. in elements.
 */
struct cmzn_mesh_scale_factor_set
{
private:
	FE_mesh *fe_mesh; // non-accessed pointer to owner
	char *name;
	int access_count;

	cmzn_mesh_scale_factor_set();

	~cmzn_mesh_scale_factor_set();

	cmzn_mesh_scale_factor_set(FE_mesh *fe_meshIn, const char *nameIn);

public:

	static cmzn_mesh_scale_factor_set *create(FE_mesh *fe_meshIn, const char *nameIn)
	{
		return new cmzn_mesh_scale_factor_set(fe_meshIn, nameIn);
	}

	cmzn_mesh_scale_factor_set *access()
	{
		++access_count;
		return this;
	}

	static int deaccess(cmzn_mesh_scale_factor_set* &scale_factor_set)
	{
		if (!scale_factor_set)
			return CMZN_ERROR_ARGUMENT;
		--(scale_factor_set->access_count);
		if (scale_factor_set->access_count <= 0)
			delete scale_factor_set;
		scale_factor_set = 0;
		return CMZN_OK;
	}

	/** @return  Internal name, not a copy */
	const char *getName() const
	{
		return this->name;
	}

	int setName(const char *nameIn);
};

class FE_mesh_field_template;

/**
 * Template for creating a new element in the given FE_mesh, or redefining
 * an existing element by merging into it.
 * Constist of element shape and definition of fields.
 * @see FE_mesh::create_FE_element()
 * @see FE_mesh::merge_FE_element_template()
 */
class FE_element_template : public cmzn::RefCounted
{
	friend class FE_mesh;

public:

	class FE_field_data
	{
		friend class FE_mesh;

		FE_field *field;
		const int componentCount;
		FE_element_field_template **efts; // allocated if defining, NULL if undefining
		FE_mesh_field_template **cacheComponentMeshFieldTemplates; // cached during merge: not accessed, not thread safe

	public:
		FE_field_data(FE_field *fieldIn);

		~FE_field_data();

		/** Define field/component with element field template. Template is not valid
		  * until all components are defined.
		  * @param componentNumber  Component number to define from 0 to count - 1,
		  * or negative for all components. */
		int define(int componentNumber, FE_element_field_template *eft);

		/** @param componentNumber  Component number to get template for, from 0 to count - 1
		  * @return  Non-accessed pointer to element field template. 0 if field is set to undefine or not found. */
		FE_element_field_template *getComponentElementfieldtemplate(int componentNumber) const
		{
			if ((this->efts) && (0 <= componentNumber) && (componentNumber < this->componentCount))
				return this->efts[componentNumber];
			return 0;
		}

		/** @param componentNumber  Component number from 0 to count - 1. Not checked. */
		FE_mesh_field_template *getCacheComponentMeshFieldTemplate(int componentNumber)
		{
			return this->cacheComponentMeshFieldTemplates[componentNumber];
		}

		/** Temporarily cache current field template for field component. Cleared after merge.
		  * @param componentNumber  Component number from 0 to count - 1. Not checked.
		  * @see FE_mesh::mergeFieldsFromElementTemplate */
		void setCacheComponentMeshFieldTemplate(int componentNumber, FE_mesh_field_template *mft)
		{
			this->cacheComponentMeshFieldTemplates[componentNumber] = mft;
		}

		/** @return  Non-accessed field */
		FE_field *getField() const
		{
			return this->field;
		}

		bool isUndefine() const
		{
			return (0 != this->efts);
		}

		/** Mark field to be undefined */
		void undefine();

		/** @return  True if all components defined, or is undefine */
		bool validate() const;

		/** merge all EFTs into mesh, replacing them with any identical already merged EFTs
		  * @return  True on success, otherwise false. */
		bool mergeElementfieldtemplatesIntoMesh(FE_mesh &mesh);

	};

private:

	FE_mesh *mesh; // Note: accessed: prevents mesh from being destroyed
	FE_element_shape *elementShape; // shape to set for new elements, or 0 to merge without setting shape
	int fieldCount;
	FE_field_data **fields;
	bool fieldsValidated;

	/** Caller must ensure element_shape is of same dimension as mesh */
	FE_element_template(FE_mesh *meshIn, FE_element_shape *elementShapeIn = 0);

	/** Creates a template copying from an existing element */
	//FE_element_template(FE_mesh *mesh_in, cmzn_element *element); // GRC implement?

	~FE_element_template();

	FE_element_template(); // not implemented
	FE_element_template(const FE_element_template&); // not implemented

	/** Add field to list of fields, initially set for undefine.
	  * @return  Index of new field, or -1 if failed. */
	int addField(FE_field *field);

	/** @return index of field in template or -1 if not found*/
	int getFieldIndex(FE_field *field) const;

	/** Define field/component with internal element field template.
	  * Template is not valid until all components are defined.
	  * @param componentNumber  Component number to define from 0 to count - 1,
	  * or negative for all components.
	  * @param eft  Internal element field template: must be locked */
	int defineField(FE_field *field, int componentNumber, FE_element_field_template *eft);

public:

	FE_mesh *getMesh() const
	{
		return this->mesh;
	}

	FE_element_shape *getElementShape() const
	{
		return this->elementShape;
	}

	int setElementShape(FE_element_shape *elementShapeIn);

	/** Define field/component with external element field template. The EFT is validated
	  * and locked before use. Template is not valid until all components are defined.
	  * @param componentNumber  Component number to define from 0 to count - 1,
	  * or negative for all components.
	  * @param eftExt  External element field template */
	int defineField(FE_field *field, int componentNumber, cmzn_elementfieldtemplate *eftExt);

	/** @param componentNumber  Component number to get template for, from 0 to count - 1
	  * @return  Non-accessed pointer to element field template. 0 if field is set to undefine or not found. */
	FE_element_field_template *getElementfieldtemplate(FE_field *field, int componentNumber) const;

	/** Remove field from list so neither defining nor undefining it */
	int removeField(FE_field *field);

	/** Mark field for undefining */
	int undefineField(FE_field *field);

	/** Checks all field definitions are valid, and if so registers all efts with the mesh
	  * @return  True if validated, false if not. */
	bool validate();

	/** @return  True if validate() has passed, false if not */
	bool isValidated() const
	{
		return this->fieldsValidated;
	}
};

/**
 * External handle to a finite element, a region of a mesh charted by a single
 * local 'xi' coordinate system with bounds given by its shape.
 * Note the shape is now held by the owning FE_mesh at the index.
 */
struct cmzn_element
{
	friend FE_mesh;
private:
	// non-accessed pointer to owning mesh; cleared if mesh is destroyed or element orphaned from mesh
	FE_mesh *mesh;
	// index into mesh labels, maps to unique identifier
	DsLabelIndex index;
	// the number of references held to this element; destroyed once reduces to 0
	int access_count;

	cmzn_element(FE_mesh *meshIn, DsLabelIndex indexIn) :
		mesh(meshIn),
		index(indexIn),
		access_count(1)
	{
	}

	~cmzn_element();

	/** safely disconnect from this mesh. Nodes, scale factors, field DOFs are freed elsewhere when underlying element destroyed */
	void invalidate()
	{
		this->mesh = 0;
		this->index = DS_LABEL_INDEX_INVALID;
	}

public:

	inline cmzn_element *access()
	{
		++(this->access_count);
		return this;
	}

	static inline int deaccess(cmzn_element* &element)
	{
		if (element)
		{
			if (--(element->access_count) == 0)
				delete element;
			element = 0;
			return CMZN_OK;
		}
		return CMZN_ERROR_ARGUMENT;
	}

	static inline void reaccess(cmzn_element* &element, cmzn_element *newElement)
	{
		if (newElement)
			++(newElement->access_count);
		if ((element) && (--(element->access_count) == 0))
			delete element;
		element = newElement;
	}

	inline int getAccessCount() const
	{
		return this->access_count;
	}

	inline DsLabelIdentifier getIdentifier() const;

	inline 	int setIdentifier(int identifier);

	inline DsLabelIndex getIndex() const
	{
		return this->index;
	}

	inline int getDimension() const;

	inline FE_element_shape *getElementShape() const;

	/** @return  Non-accessed mesh, or 0 if invalid. */
	inline FE_mesh *getMesh() const
	{
		return this->mesh;
	}

};


/** Stores FE_element_field_template and its element-varying local-to-global map data */
class FE_mesh_element_field_template_data
{
	friend class FE_mesh;

	FE_element_field_template *eft;
	const int localNodeCount; // cached from eft to guarantee efficiency
	DsMapArrayLabelIndex localToGlobalNodes; // map from element[local node index] -> global nodes index
	const int localScaleFactorCount; // cached from eft to guarantee efficiency
	// temporary: store actual scale factors, until management of global scale factors finalised
	block_array<DsLabelIndex, FE_value*> localScaleFactors;
	// future DsMapArrayLabelIndex localToGlobalScaleFactors; // map from element[local node index] -> global nodes index
	// for each element, the number of field components using this eft. Clear per-element data when this drops to zero:
	typedef unsigned short MeshfieldtemplateUsageCountType; // internal use only
	block_array<DsLabelIndex, MeshfieldtemplateUsageCountType> meshfieldtemplateUsageCount;

	FE_mesh_element_field_template_data(const FE_mesh_element_field_template_data &source); // not implemented
	FE_mesh_element_field_template_data& operator=(const FE_mesh_element_field_template_data &source); // not implemented

	void clearElementVaryingData(DsLabelIndex elementIndex);
	void clearAllElementVaryingData();

public:

	FE_mesh_element_field_template_data(FE_element_field_template *eftIn, DsLabels *elementLabels) :
		eft(eftIn->access()),
		localNodeCount(eftIn->getNumberOfLocalNodes()),
		localToGlobalNodes(elementLabels, (localNodeCount > 0) ? localNodeCount : 2, DS_LABEL_INDEX_UNALLOCATED, DS_LABEL_INDEX_INVALID),
		localScaleFactorCount(eftIn->getNumberOfLocalScaleFactors())
		//localToGlobalScaleFactors(elementLabels, (localScaleFactorCount > 0) ? localScaleFactorCount : 2, DS_LABEL_INDEX_UNALLOCATED, DS_LABEL_INDEX_INVALID)
	{
	}

	~FE_mesh_element_field_template_data()
	{
		clearAllElementVaryingData();
		FE_element_field_template::deaccess(this->eft);
	}

	FE_element_field_template *getElementfieldtemplate() const
	{
		return this->eft;
	}

	/** @return  Pointer to array of global node indexes for element, or 0 if none. NOT to be freed.
	  * @param elementIndex  Label index for element. Caller must ensure this is non-negative. */
	DsLabelIndex *getElementNodeIndexes(DsLabelIndex elementIndex)
	{
		return this->localToGlobalNodes.getArray(elementIndex);
	}

	/** @return  Pointer to array of global node indexes for element, or 0 if none. NOT to be freed.
	  * @param elementIndex  Label index for element. Caller must ensure this is non-negative. */
	const DsLabelIndex *getElementNodeIndexes(DsLabelIndex elementIndex) const
	{
		return this->localToGlobalNodes.getArray(elementIndex);
	}

#if 0
	/** @return  Pointer to array of global scale factor indexes for element, or 0 if none. NOT to be freed.
	  * @param elementIndex  Label index for element. Caller must ensure this is non-negative. */
	DsLabelIndex *getElementScaleFactorIndexes(DsLabelIndex elementIndex)
	{
		return this->localToGlobalScaleFactors.getArray(elementIndex);
	}

	/** @return  Pointer to existing or new array of global scale factor indexes for element, or 0 if failed. NOT to be freed.
	  * @param elementIndex  Label index for element. Note: caller must ensure this is non-negative. */
	DsLabelIndex *getOrCreateElementScaleFactorIndexes(DsLabelIndex elementIndex)
	{
		if (this->localScaleFactorCount > 0)
			return this->localToGlobalScaleFactors.getOrCreateArray(elementIndex);
		return 0;
	}
#endif

	void incrementMeshfieldtemplateUsageCount(DsLabelIndex elementIndex);

	void decrementMeshfieldtemplateUsageCount(DsLabelIndex elementIndex);

	DsLabelIndex getElementIndexLimit() const;

	DsLabelIndex getElementIndexStart() const;

	bool hasElementVaryingData() const
	{
		return (this->localNodeCount > 0) || (this->localScaleFactorCount > 0);
	}

	/** @return  Pointer to array of global node indexes for element, or 0 if none. NOT to be freed.
	  * @param elementIndex  Label index for element. Caller must ensure this is non-negative. */
	const FE_value *getElementScaleFactors(DsLabelIndex elementIndex) const
	{
		return this->localScaleFactors.getValue(elementIndex);
	}

	/** @return  Pointer to array of global node indexes for element, or 0 if none. NOT to be freed.
	  * @param elementIndex  Label index for element. Caller must ensure this is non-negative. */
	FE_value *getElementScaleFactors(DsLabelIndex elementIndex)
	{
		return this->localScaleFactors.getValue(elementIndex);
	}

	bool setScaleFactors(DsLabelIndex elementIndex, const FE_value *values);

	bool setElementLocalNodes(DsLabelIndex elementIndex, DsLabelIndex *nodeIndexes, bool& localNodeChange);

	bool mergeElementVaryingData(const FE_mesh_element_field_template_data& source);
};


/** Stores the mapping of element to element field template and data suitable for
  * defining a scalar field or field component over elements of the mesh.
  * Undefined or -1 value for the eft data map means the field is not defined */
class FE_mesh_field_template
{
public:
	friend FE_mesh;

	// short integer to save memory in FE_mesh_field_template. Signed to store unset value -1
	typedef short EFTIndexType;
	static const EFTIndexType ELEMENT_FIELD_TEMPLATE_DATA_INDEX_INVALID = -1;

private:
	FE_mesh *mesh; // owning mesh: not ACCESSed since these objects are owned by the mesh
	block_array<DsLabelIndex, EFTIndexType> eftDataMap;
	int mapSize; // number of valid entries in map, for trivial comparisons
	int access_count; // maintained as number of field components using this template

	FE_mesh_field_template(FE_mesh *meshIn) :
		mesh(meshIn),
		eftDataMap(/*blockLengthIn*/CMZN_BLOCK_ARRAY_DEFAULT_BLOCK_SIZE_BYTES/sizeof(EFTIndexType),
			/*allocInitValueIn*/ELEMENT_FIELD_TEMPLATE_DATA_INDEX_INVALID),
		mapSize(0),
		access_count(1)
	{
	}

	FE_mesh_field_template(const FE_mesh_field_template &source) :
		mesh(source.mesh),
		eftDataMap(source.eftDataMap),
		mapSize(source.mapSize),
		access_count(1)
	{
	}

	~FE_mesh_field_template();

	FE_mesh_field_template(); //  not implemented
	FE_mesh_field_template& operator=(const FE_mesh_field_template &source); // not implemented
public:

	FE_mesh_field_template *access()
	{
		++access_count;
		return this;
	}

	static int deaccess(FE_mesh_field_template* &ft)
	{
		if (!ft)
			return CMZN_ERROR_ARGUMENT;
		--(ft->access_count);
		if (ft->access_count <= 0)
			delete ft;
		ft = 0;
		return CMZN_OK;
	}

	/** @return  Non-accessed mesh owning this mesh field template. */
	FE_mesh *getMesh() const
	{
		return this->mesh;
	}

	/** @return  True if no element field templates have been set */
	bool isBlank() const
	{
		return (this->mapSize == 0);
	}

	DsLabelIndex getElementIndexLimit() const;

	DsLabelIndex getElementIndexStart() const;

	// Actually just equals access count
	int getFieldComponentUsageCount() const
	{
		return this->access_count;
	}

	inline EFTIndexType FE_mesh_field_template::getElementEFTIndex(DsLabelIndex elementIndex) const
	{
		return this->eftDataMap.getValue(elementIndex);
	}

	inline FE_element_field_template *getElementfieldtemplate(DsLabelIndex elementIndex) const;

	/** @param elementIndex  Not checked; assume valid
	  * @return  True if template uses eft at element index */
	inline bool hasElementfieldtemplate(DsLabelIndex elementIndex, FE_element_field_template *eft) const
	{
		return this->eftDataMap.hasValue(elementIndex,
			static_cast<EFTIndexType>(eft->getIndexInMesh()));
	}

	bool setElementfieldtemplateIndex(DsLabelIndex elementIndex, EFTIndexType eftIndex);

	bool setElementfieldtemplate(DsLabelIndex elementIndex, FE_element_field_template *eftIn);

	bool matchesWithEFTIndexMap(const FE_mesh_field_template &source,
		const std::vector<EFTIndexType> &sourceEFTIndexMap, bool superset) const;

	bool mergeWithEFTIndexMap(const FE_mesh_field_template &source,
		const std::vector<EFTIndexType> &sourceEFTIndexMap);

	bool usesNonLinearBasis() const;

};


/** Stores field component definition on a mesh as a FE_mesh_field_template plus
  * per-element parameters. */
class FE_mesh_field_data
{
	friend class FE_mesh;

public:
	class ComponentBase
	{
		friend class FE_mesh;
		friend class FE_mesh_field_data;

		FE_mesh_field_template *meshFieldTemplate; // accessed

	public:

		ComponentBase(FE_mesh_field_template *meshFieldTemplateIn) :
			meshFieldTemplate(meshFieldTemplateIn->access())
		{
		}

		virtual ~ComponentBase()
		{
			FE_mesh_field_template::deaccess(this->meshFieldTemplate);
		}

		virtual void clearElementData(DsLabelIndex elementIndex) = 0;

		/** @return  Non-accessed field template */
		const FE_mesh_field_template *getMeshfieldtemplate() const
		{
			return this->meshFieldTemplate;
		}

		/** @return  Non-accessed field template */
		FE_mesh_field_template *getMeshfieldtemplate()
		{
			return this->meshFieldTemplate;
		}

		/** @param fieldTemplateIn  Field template: not checked; must be valid for mesh */
		void setMeshfieldtemplate(FE_mesh_field_template *meshFieldTemplateIn)
		{
			meshFieldTemplateIn->access();
			FE_mesh_field_template::deaccess(this->meshFieldTemplate);
			this->meshFieldTemplate = meshFieldTemplateIn;
		}

		virtual bool mergeElementValues(const ComponentBase *sourceBase) = 0;

	};

	template <typename ValueType> class Component : public ComponentBase
	{
		friend class FE_mesh;

		block_array<DsLabelIndex, ValueType> elementScalarDOFs;
		block_array<DsLabelIndex, ValueType*> elementVectorDOFs;

	public:

		Component(FE_mesh_field_template *fieldTemplateIn) :
			ComponentBase(fieldTemplateIn)
		{
		}

		virtual ~Component()
		{
			clearDynamicBlockArray(this->elementVectorDOFs);
		}

		/** @param elementIndex  Element index >= 0. Not checked. */
		virtual void clearElementData(DsLabelIndex elementIndex)
		{
			ValueType** valuePtrAddress = this->elementVectorDOFs.getAddress(elementIndex);
			if (valuePtrAddress)
			{
				delete[] *valuePtrAddress;
				*valuePtrAddress = 0;
			}
		}

		/** @param elementIndex  The element to get values for.
		  * @param valuesCount  The number of values expected. Used to distinguish scalar/vector storage.
		  * @return  Address of values, or 0 if none. Note can return address of a scalar when it doesn't
		  * exist. */
		const ValueType *getElementValues(DsLabelIndex elementIndex, int valuesCount) const
		{
			if (valuesCount == 1)
				return this->elementScalarDOFs.getAddress(elementIndex); // can be false positive
			return this->elementVectorDOFs.getValue(elementIndex);
		}

		/** @param elementIndex  The element to get values for.
		  * @param valuesCount  The number of values expected. Used to distinguish scalar/vector storage.
		  * @return  Address of values, or 0 if none. Note can return address of a scalar when it doesn't
		  * exist. */
		ValueType *getElementValues(DsLabelIndex elementIndex, int valuesCount)
		{
			if (valuesCount == 1)
				return this->elementScalarDOFs.getAddress(elementIndex); // can be false positive
			return this->elementVectorDOFs.getValue(elementIndex);
		}

		/** @param elementIndex  The element to set values for.
		  * @param valuesCount  Size of values array; must be >= 1.
		  * @param values  Array of values to set. Must be non-null.
		  * @return  True on success, false on failure. */
		bool setElementValues(DsLabelIndex elementIndex, int valuesCount, const ValueType *values)
		{
			if (valuesCount == 1)
				return this->elementScalarDOFs.setValue(elementIndex, *values);
			ValueType **existingValuesAddress = this->elementVectorDOFs.getAddress(elementIndex);
			ValueType *targetValues;
			if (existingValuesAddress)
				targetValues = *existingValuesAddress;
			if (!targetValues)
			{
				targetValues = new ValueType[valuesCount];
				if (!targetValues)
					return false;
				if (existingValuesAddress)
					*existingValuesAddress = targetValues;
				else if (!this->elementVectorDOFs.setValue(elementIndex, targetValues))
					return false;
			}
			memcpy(targetValues, values, valuesCount*sizeof(ValueType));
			return true;
		}

		/** Copy element values from source component, finding target elements by identifier.
		  * Used only by FE_mesh::merge to merge values from another region.
		  * Must have already merged mesh field template for component, so target element
		  * field template is guaranteed to be the same for each element in source.
		  * @param sourceBase  Source component as base class. Must of same ValueType.
		  * @return  True on success, false on failure. */
		virtual bool mergeElementValues(const ComponentBase *sourceBase)
		{
			const Component *source = dynamic_cast<const Component<ValueType> *>(sourceBase);
			if (!source)
			{
				display_message(ERROR_MESSAGE, "FE_mesh_field_data::Component::mergeElementDOFs.  Invalid source");
				return false;
			}
			const FE_mesh *sourceMesh = source->meshFieldTemplate->getMesh();
			const FE_mesh *targetMesh = this->meshFieldTemplate->getMesh();
			DsLabelIndex sourceElementIndexLimit = source->meshFieldTemplate->getElementIndexLimit();
			for (DsLabelIndex sourceElementIndex = source->meshFieldTemplate->getElementIndexStart();
				sourceElementIndex < sourceElementIndexLimit; ++sourceElementIndex)
			{
				const DsLabelIdentifier elementIdentifier = sourceMesh->getElementIdentifier(sourceElementIndex);
				if (DS_LABEL_IDENTIFIER_INVALID == elementIdentifier)
					continue; // no element at that index
				FE_element_field_template *sourceElementfieldtemplate = source->meshFieldTemplate->getElementfieldtemplate(sourceElementIndex);
				if (!sourceElementfieldtemplate)
					continue;
				const int valuesCount = sourceElementfieldtemplate->getNumberOfElementDOFs();
				if (0 == valuesCount)
					continue;
				const ValueType *sourceValues = source->getElementValues(sourceElementIndex, valuesCount);
				if (!sourceValues)
				{
					display_message(ERROR_MESSAGE, "FE_mesh_field_data::Component::mergeElementDOFs.  Missing source DOFs");
					return false;
				}
				const DsLabelIndex targetElementIndex = targetMesh->findIndexByIdentifier(elementIdentifier);
				// the above must be valid if merge has got to this point, so not testing.
				if (!this->setElementValues(targetElementIndex, valuesCount, sourceValues))
					return false;
			}
			return true;
		}

	};

private:
	FE_field *field; // not accessed; structure is owned by field and dies with it
	const int componentCount; // cached from field
	const Value_type valueType;
	ComponentBase **components;

	FE_mesh_field_data(FE_field *fieldIn, ComponentBase **componentsIn) :
		field(fieldIn),
		componentCount(get_FE_field_number_of_components(fieldIn)),
		valueType(get_FE_field_value_type(fieldIn)),
		components(componentsIn) // takes ownership of passed-in array
	{
	}

public:

	/** @return  New mesh field data with blank FieldTemplates for field on mesh, or 0 if failed */
	static FE_mesh_field_data *create(FE_field *field, FE_mesh *mesh);

	~FE_mesh_field_data()
	{
		for (int i = 0; i < this->componentCount; ++i)
			delete this->components[i];
		delete[] this->components;
	}

	/** @param componentNumber  From 0 to componentCount, not checked.
	  * @param elementIndex  Element index >= 0. Not checked. */
	void clearComponentElementData(int componentNumber, DsLabelIndex elementIndex)
	{
		this->components[componentNumber]->clearElementData(elementIndex);
	}

	/** @param componentNumber  From 0 to componentCount, not checked
	  * @return  Non-accessed component base */
	ComponentBase *getComponentBase(int componentNumber) const
	{
		return this->components[componentNumber];
	}

	/** @param componentNumber  From 0 to componentCount, not checked
	  * @return  Non-accessed field template */
	const FE_mesh_field_template *getComponentMeshfieldtemplate(int componentNumber) const
	{
		return this->components[componentNumber]->getMeshfieldtemplate();
	}

	/** @param componentNumber  From 0 to componentCount, not checked
	  * @return  Non-accessed field template */
	FE_mesh_field_template *getComponentMeshfieldtemplate(int componentNumber)
	{
		return this->components[componentNumber]->getMeshfieldtemplate();
	}

	/** @param componentNumber  From 0 to componentCount, not checked */
	void setComponentMeshfieldtemplate(int componentNumber, FE_mesh_field_template *meshFieldTemplate)
	{
		this->components[componentNumber]->setMeshfieldtemplate(meshFieldTemplate);
	}

	bool isDefinedOnElements()
	{
		return !this->components[0]->getMeshfieldtemplate()->isBlank();
	}

	/** @return  True if any element of any component uses a non-linear basis in any direction */
	bool usesNonLinearBasis() const
	{
		for (int i = 0; i < this->componentCount; ++i)
			if (this->components[0]->getMeshfieldtemplate()->usesNonLinearBasis())
				return true;
		return false;
	}
};

/**
 * A set of elements in the FE_region.
 */
class FE_mesh
{
public:

	/** Stores element shape and face maps for that shape */
	class ElementShapeFaces
	{
		FE_element_shape *shape;
		const cmzn_element_shape_type shape_type;
		const int faceCount;
		DsMapArrayLabelIndex faces; // map to connected element indexes from face mesh

	public:
		ElementShapeFaces(DsLabels *labels, FE_element_shape *shapeIn) :
			shape(ACCESS(FE_element_shape)(shapeIn)),
			shape_type(FE_element_shape_get_simple_type(shapeIn)),
			faceCount(FE_element_shape_get_number_of_faces(shapeIn)),
			faces(labels, faceCount > 0 ? faceCount : 2, DS_LABEL_INDEX_UNALLOCATED, DS_LABEL_INDEX_INVALID)
		{
		}

		~ElementShapeFaces()
		{
			DEACCESS(FE_element_shape)(&this->shape);
		}

		FE_element_shape *getShape() const
		{
			return this->shape;
		}

		cmzn_element_shape_type getShapeType() const
		{
			return this->shape_type;
		}

		int getFaceCount() const
		{
			return this->faceCount;
		}

		/** return face number for face type, or negative if invalid */
		int faceTypeToNumber(cmzn_element_face_type faceType)
		{
			// temporary, until other element shape face type enums and mappings to face numbers are supported:
			int faceNumber = static_cast<int>(faceType) - CMZN_ELEMENT_FACE_TYPE_XI1_0;
			if (faceNumber < this->faceCount)
				return faceNumber;
			return -1;
		}

		/** clear face element indexes held for elementIndex */
		void destroyElementFaces(DsLabelIndex elementIndex)
		{
			this->faces.destroyArray(elementIndex);
		}

		/** @return  Array of faceCount face element indexes, or 0 if not allocated. NOT to be freed. */
		DsLabelIndex *getElementFaces(DsLabelIndex elementIndex)
		{
			return this->faces.getArray(elementIndex);
		}

		/** @return  Array of faceCount face element indexes, or 0 if not allocated. NOT to be freed. */
		const DsLabelIndex *getElementFaces(DsLabelIndex elementIndex) const
		{
			return this->faces.getArray(elementIndex);
		}

		/** @return  Existing or new array of faceCount face element indexes, or 0 if failed. NOT to be freed.
		 * New array contents are initialised to DS_LABEL_INDEX_INVALID. */
		DsLabelIndex *getOrCreateElementFaces(DsLabelIndex elementIndex)
		{
			return this->faces.getOrCreateArray(elementIndex);
		}

		/** convenient function for getting a single face for an element */
		DsLabelIndex getElementFace(DsLabelIndex elementIndex, int faceNumber);

		/** convenient function for setting a single face for an element */
		int setElementFace(DsLabelIndex elementIndex, int faceNumber, DsLabelIndex faceIndex);

	};

private:

	FE_region *fe_region; // not accessed
	int dimension;

	DsLabels labels; // element identifiers

	// element shape and face/parent mappings
	unsigned int elementShapeFacesCount;
	ElementShapeFaces **elementShapeFacesArray;
	block_array<DsLabelIndex, unsigned char> elementShapeMap; // map element index -> shape faces index, if not all elements have same shape
	block_array<DsLabelIndex,DsLabelIndex*> parents; // map to number of parents followed by parent element indexes in order

	// map element index -> element object (accessed)
	block_array<DsLabelIndex, cmzn_element*> fe_elements;

	// maintain list of EFTs for this mesh, whether merged in elementFieldTemplateData
	// or externally held, so can clear their mesh pointer on mesh destruction
	std::set<FE_element_field_template *> allElementfieldtemplates;

	// array of element field templates with common element mapping data
	// mesh field templates store indexes into this array
	int elementFieldTemplateDataCount;
	FE_mesh_element_field_template_data **elementFieldTemplateData;

	std::list<FE_mesh_field_template*> meshFieldTemplates;

	// cache last element template merged in, so not expensively recording field changes
	// must clear when element template changed or deleted, and in extractChangeLog()
	// not accessed, since FE_element_template accesses mesh
	FE_element_template *lastMergedElementTemplate;

	// following are cleared when detached from owning FE_region (i.e. when it is destroyed)
	FE_mesh *parentMesh; // not accessed
	FE_mesh *faceMesh; // not accessed
	FE_nodeset *nodeset; // not accessed

	/* log of elements added, removed or otherwise changed */
	DsLabelsChangeLog *changeLog;

	/* information for defining faces */
	/* existence of element_type_node_sequence_list can tell us whether faces
		 are being defined */
	struct LIST(FE_element_type_node_sequence) *element_type_node_sequence_list;
	bool definingFaces;

	// scale factor sets in use. Used as identifier for finding scale factor
	// arrays stored with elements 
	std::vector<cmzn_mesh_scale_factor_set*> scale_factor_sets;

	// list of element iterators to invalidate when mesh destroyed
	cmzn_elementiterator *activeElementIterators;

	int access_count;

private:

	FE_mesh(FE_region *fe_regionIn, int dimensionIn);

	~FE_mesh();

	void createChangeLog();

	int findOrCreateFace(DsLabelIndex parentIndex, int faceNumber, DsLabelIndex& faceIndex);

	int removeElementPrivate(DsLabelIndex elementIndex);

	inline ElementShapeFaces *getElementShapeFaces(DsLabelIndex elementIndex)
	{
		if (elementIndex >= 0)
		{
			if (this->elementShapeFacesCount > 1)
			{
				unsigned char shapeIndex;
				if (this->elementShapeMap.getValue(elementIndex, shapeIndex))
					return this->elementShapeFacesArray[static_cast<unsigned int>(shapeIndex)];
			}
			else if (this->elementShapeFacesArray)
				return this->elementShapeFacesArray[0];
		}
		return 0;
	}

	inline const ElementShapeFaces *getElementShapeFaces(DsLabelIndex elementIndex) const
	{
		if (elementIndex >= 0)
		{
			if (this->elementShapeFacesCount > 1)
			{
				unsigned char shapeIndex;
				if (this->elementShapeMap.getValue(elementIndex, shapeIndex))
					return this->elementShapeFacesArray[static_cast<unsigned int>(shapeIndex)];
			}
			else if (this->elementShapeFacesArray)
				return this->elementShapeFacesArray[0];
		}
		return 0;
	}

	int addElementParent(DsLabelIndex elementIndex, DsLabelIndex parentIndex);

	int removeElementParent(DsLabelIndex elementIndex, DsLabelIndex parentIndex);

	void clearElementParents(DsLabelIndex elementIndex);

	void clearElementFaces(DsLabelIndex elementIndex);

	ElementShapeFaces *setElementShape(DsLabelIndex elementIndex, FE_element_shape *element_shape);

	bool setElementShapeFromElementTemplate(DsLabelIndex elementIndex, FE_element_template *element_template);

	bool mergeFieldsFromElementTemplate(DsLabelIndex elementIndex, FE_element_template *elementTemplate);

	void destroyElement(DsLabelIndex elementIndex);

	DsLabelIndex createElement(DsLabelIdentifier identifier);

	cmzn_element *createElementObject(DsLabelIdentifier identifier);

	FE_mesh_field_template *createBlankMeshFieldTemplate();

	FE_mesh_field_template *cloneMeshFieldTemplate(const FE_mesh_field_template *source);

	void clearElementFieldData();

public:

	static FE_mesh *create(FE_region *fe_region, int dimension)
	{
		if (fe_region && (0 < dimension) && (dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
			return new FE_mesh(fe_region, dimension);
		return 0;
	}

	void detach_from_FE_region();

	int addElementfieldtemplate(FE_element_field_template *eft);

	int removeElementfieldtemplate(FE_element_field_template *eft);

	/** @param eftIndex  Index from 0 to number in mesh - 1. Not checked.
	  * @return  Non-accessed element field template. */
	FE_element_field_template *getElementfieldtemplateFromIndex(int eftIndex) const
	{
		return this->elementFieldTemplateData[eftIndex]->getElementfieldtemplate();
	}

	/** @return  Number of element field template data. */
	int getElementfieldtemplateDataCount() const
	{
		return this->elementFieldTemplateDataCount;
	}

	/** @param eftIndex  Index of EFT in this mesh. Not checked.
	  * @return  Non-accessed mesh element field template data, or 0 if none. */
	const FE_mesh_element_field_template_data *getElementfieldtemplateData(int eftIndex) const
	{
		return this->elementFieldTemplateData[eftIndex];
	}

	/** @param eftIndex  Index of EFT in this mesh. Not checked.
	  * @return  Non-accessed mesh element field template data, or 0 if none. */
	FE_mesh_element_field_template_data *getElementfieldtemplateData(int eftIndex)
	{
		return this->elementFieldTemplateData[eftIndex];
	}

	/** @param eft  Element field template from this mesh.
	  * @return  Non-accessed mesh element field template data. */
	const FE_mesh_element_field_template_data *getElementfieldtemplateData(const FE_element_field_template *eft) const
	{
		if (eft && (eft->getMesh() == this) && (eft->isMergedInMesh()))
			return this->elementFieldTemplateData[eft->getIndexInMesh()];
		return 0;
	}

	/** @param eft  Element field template from this mesh.
	  * @return  Non-accessed mesh element field template data. */
	FE_mesh_element_field_template_data *getElementfieldtemplateData(const FE_element_field_template *eft)
	{
		if (eft && (eft->getMesh() == this) && (eft->isMergedInMesh()))
			return this->elementFieldTemplateData[eft->getIndexInMesh()];
		return 0;
	}

	FE_element_field_template *mergeElementfieldtemplate(FE_element_field_template *eftIn);

	FE_mesh_field_template *getOrCreateBlankMeshFieldTemplate();

	void removeMeshFieldTemplate(FE_mesh_field_template *meshFieldTemplate);

	bool equivalentFieldsInElements(DsLabelIndex elementIndex1, DsLabelIndex elementIndex2) const;

	FE_mesh *access()
	{
		++(this->access_count);
		return this;
	}

	static void deaccess(FE_mesh *&mesh)
	{
		if (mesh)
		{
			--(mesh->access_count);
			if (mesh->access_count <= 0)
				delete mesh;
			mesh = 0;
		}
	}

	void elementChange(DsLabelIndex elementIndex, int change);

	int getDimension() const
	{
		return this->dimension;
	}

	FE_region *get_FE_region() const
	{
		return this->fe_region;
	}

	FE_mesh *getFaceMesh() const
	{
		return this->faceMesh;
	}

	void setFaceMesh(FE_mesh *faceMeshIn)
	{
		this->faceMesh = faceMeshIn;
	}

	FE_mesh *getParentMesh() const
	{
		return this->parentMesh;
	}

	void setParentMesh(FE_mesh *parentMeshIn)
	{
		this->parentMesh = parentMeshIn;
	}

	void setNodeset(FE_nodeset *nodesetIn)
	{
		this->nodeset = nodesetIn;
	}

	/** @return  Nodeset for this mesh, or 0 if detached from FE_region. */
	FE_nodeset *getNodeset() const
	{
		return this->nodeset;
	}

	void clear();

	/** @return Accessed changes */
	DsLabelsChangeLog *extractChangeLog();

	/** @retrun non-Accessed changes */
	DsLabelsChangeLog *getChangeLog()
	{
		return this->changeLog;
	}

	inline const ElementShapeFaces *getElementShapeFacesConst(DsLabelIndex elementIndex) const
	{
		return const_cast<FE_mesh*>(this)->getElementShapeFaces(elementIndex);
	}

	FE_element_shape *getElementShape(DsLabelIndex elementIndex)
	{
		ElementShapeFaces *elementShapeFaces = this->getElementShapeFaces(elementIndex);
		if (elementShapeFaces)
			return elementShapeFaces->getShape();
		return 0;
	}

	cmzn_element_shape_type getElementShapeType(DsLabelIndex elementIndex)
	{
		ElementShapeFaces *elementShapeFaces = this->getElementShapeFaces(elementIndex);
		if (elementShapeFaces)
			return elementShapeFaces->getShapeType();
		return CMZN_ELEMENT_SHAPE_TYPE_INVALID;
	}

	cmzn_mesh_scale_factor_set *find_scale_factor_set_by_name(const char *name);

	cmzn_mesh_scale_factor_set *create_scale_factor_set();

	/** get size i.e. number of elements in mesh */
	int getSize() const
	{
		return this->labels.getSize();
	}

	/** get labels index size, gives index limit for iterating in index order */
	DsLabelIndex getLabelsIndexSize() const
	{
		return this->labels.getIndexSize();
	}

	inline bool hasElement(DsLabelIndex elementIndex) const
	{
		return this->labels.hasIndex(elementIndex);
	}

	inline DsLabelIdentifier getElementIdentifier(DsLabelIndex elementIndex) const
	{
		return this->labels.getIdentifier(elementIndex);
	}

	/** @return  Non-accessed element object at index */
	inline cmzn_element *getElement(DsLabelIndex elementIndex) const
	{
		cmzn_element *element = 0;
		if (elementIndex >= 0)
			this->fe_elements.getValue(elementIndex, element);
		return element;
	}

	DsLabelIdentifier get_next_FE_element_identifier(DsLabelIdentifier start_identifier)
	{
		return this->labels.getFirstFreeIdentifier(start_identifier);
	}

	void list_btree_statistics();

	bool containsElement(cmzn_element *element) const
	{
		return (element) && (element->getMesh() == this) && (element->getIndex() >= 0);
	}

	DsLabelIndex findIndexByIdentifier(DsLabelIdentifier identifier) const
	{
		return this->labels.findLabelByIdentifier(identifier);
	}

	/** @return  Non-accessed element */
	cmzn_element *findElementByIdentifier(DsLabelIdentifier identifier) const
	{
		return this->getElement(this->labels.findLabelByIdentifier(identifier));
	}

	void removeElementiterator(cmzn_elementiterator *iterator); // private, but needed by cmzn_elementiterator

	cmzn_elementiterator *createElementiterator(DsLabelsGroup *labelsGroup = 0);

	struct FE_element *get_first_FE_element_that(
		LIST_CONDITIONAL_FUNCTION(FE_element) *conditional_function, void *user_data_void);

	int for_each_FE_element(LIST_ITERATOR_FUNCTION(FE_element) iterator_function, void *user_data_void);

	DsLabelsGroup *createLabelsGroup();

	int setElementIdentifier(DsLabelIndex elementIndex, int identifier);

	FE_element_template *create_FE_element_template(FE_element_shape *element_shape = 0);

	cmzn_element *create_FE_element(int identifier, FE_element_template *elementTemplate);

	cmzn_element *get_or_create_FE_element_with_identifier(int identifier,
		struct FE_element_shape *element_shape);

	int merge_FE_element_template(cmzn_element *destination, FE_element_template *elementTemplate);

	/** Call if element template is destroyed or modified to ensure mesh is
	  * not caching it for efficient field change notification */
	void noteChangedElementTemplate(FE_element_template *elementTemplate)
	{
		if (elementTemplate == this->lastMergedElementTemplate)
			this->lastMergedElementTemplate = 0;
	}

	DsLabelIndex getElementFace(DsLabelIndex elementIndex, int faceNumber)
	{
		ElementShapeFaces *elementShapeFaces = this->getElementShapeFaces(elementIndex);
		if (!elementShapeFaces)
			return DS_LABEL_INDEX_INVALID;
		return elementShapeFaces->getElementFace(elementIndex, faceNumber);
	}

	int setElementFace(DsLabelIndex elementIndex, int faceNumber, DsLabelIndex faceIndex);

	int getElementFaceNumber(DsLabelIndex elementIndex, DsLabelIndex faceIndex) const;

	/** @return  Number of parent elements.
	  * @param parents  If has parents, pointer to parent element indexes. Not to be freed */
	int getElementParents(DsLabelIndex elementIndex, const DsLabelIndex *&parents) const
	{
		if (elementIndex >= 0)
		{
			DsLabelIndex *parentsArray;
			if (this->parents.getValue(elementIndex, parentsArray) && parentsArray)
			{
				parents = parentsArray + 1;
				return *parentsArray;
			}
		}
		return 0;
	}

	bool elementHasParentInGroup(DsLabelIndex elementIndex, const DsLabelsGroup& parentGroup) const
	{
		const DsLabelIndex *parents;
		const int parentCount = this->getElementParents(elementIndex, parents);
		for (int p = 0; p < parentCount; ++p)
			if (parentGroup.hasIndex(parents[p]))
				return true;
		return false;
	}

	bool elementHasNodeInGroup(DsLabelIndex elementIndex, const DsLabelsGroup& nodeLabelsGroup) const;

	bool addElementNodesToGroup(DsLabelIndex elementIndex, DsLabelsGroup& nodeLabelsGroup);

	bool removeElementNodesFromGroup(DsLabelIndex elementIndex, DsLabelsGroup& nodeLabelsGroup);

	/** @return  Number of parent elements. */
	inline int getElementParentsCount(DsLabelIndex elementIndex) const
	{
		if (this->parentMesh)
		{
			const DsLabelIndex *parents;
			return this->getElementParents(elementIndex, parents);
		}
		return 0;
	}

	// GRC restore implementation from descendant up
	bool isElementAncestor(DsLabelIndex elementIndex, const FE_mesh *descendantMesh, DsLabelIndex descendantIndex);

	/** return true if element or parent has exactly 1 parent element */
	bool isElementExterior(DsLabelIndex elementIndex);

	/** return true if elementIndex has faceIndex on the given face */
	bool isElementFaceOfType(DsLabelIndex elementIndex, DsLabelIndex faceIndex, cmzn_element_face_type faceType)
	{
		ElementShapeFaces *elementShapeFaces = this->getElementShapeFaces(elementIndex);
		if (elementShapeFaces)
		{
			const DsLabelIndex *faces = elementShapeFaces->getElementFaces(elementIndex);
			if (faces)
			{
				const int faceNumber = elementShapeFaces->faceTypeToNumber(faceType);
				if ((0 <= faceNumber) && (faces[faceNumber] == faceIndex))
					return true;
			}
		}
		return false;
	}

	/** return index of parent element that either this element is on the given face of,
		* or the parent is on the given face of a top-level element. */
	DsLabelIndex getElementParentOnFace(DsLabelIndex elementIndex, cmzn_element_face_type faceType);

	DsLabelIndex getElementFirstNeighbour(DsLabelIndex elementIndex, int faceNumber, int &newFaceNumber);

	int defineElementFaces(DsLabelIndex elementIndex);

	int begin_define_faces();

	void end_define_faces();

	int define_faces();

	int destroyElement(cmzn_element *element);

	int destroyAllElements();

	int destroyElementsInGroup(DsLabelsGroup& labelsGroup);

	bool checkConvertLegacyNodeParameters(FE_nodeset *targetNodeset);

	bool canMerge(const FE_mesh &source) const;

	int merge(const FE_mesh &source);
};

inline DsLabelIdentifier cmzn_element::getIdentifier() const
{
	if (this->mesh)
		return this->mesh->getElementIdentifier(this->index);
	return DS_LABEL_IDENTIFIER_INVALID;
}

inline int cmzn_element::setIdentifier(int identifier)
{
	if (this->mesh)
		return this->mesh->setElementIdentifier(this->index, identifier);
	return CMZN_ERROR_GENERAL;
}

inline int cmzn_element::getDimension() const
{
	if (this->mesh)
		return this->mesh->getDimension();
	display_message(ERROR_MESSAGE, "Element getDimension.  Invalid element");
	return 0;
}

inline FE_element_shape *cmzn_element::getElementShape() const
{
	if (this->mesh)
		return this->mesh->getElementShape(this->index);
	display_message(ERROR_MESSAGE, "Element getElementShape.  Invalid element");
	return 0;
}

/** @param elementIndex  Not checked; assume valid */
inline FE_element_field_template *FE_mesh_field_template::getElementfieldtemplate(DsLabelIndex elementIndex) const
{
	const EFTIndexType eftIndex = this->eftDataMap.getValue(elementIndex);
	if (eftIndex < 0)
		return 0;
	return this->mesh->getElementfieldtemplateFromIndex(eftIndex);
}


struct cmzn_elementiterator : public cmzn::RefCounted
{
	friend class FE_mesh;

private:
	FE_mesh *fe_mesh;
	DsLabelIterator *iter;
	cmzn_elementiterator *nextIterator; // for linked list of active iterators in FE_mesh

	// takes ownership of iter_in access count
	cmzn_elementiterator(FE_mesh *fe_mesh_in, DsLabelIterator *iter_in) :
		fe_mesh(fe_mesh_in),
		iter(iter_in),
		nextIterator(0)
	{
	}

	virtual ~cmzn_elementiterator()
	{
		if (this->fe_mesh)
			this->fe_mesh->removeElementiterator(this);
		cmzn::Deaccess(this->iter);
	}

	void detachFromMesh()
	{
		// sufficient to clear fe_mesh as only called from fe_mesh destructor
		this->fe_mesh = 0;
	}

	template<class REFCOUNTED>
		friend inline void cmzn::Deaccess(REFCOUNTED*& elementIterator);

public:

	/** @return  Non-accessed element or 0 if iteration ended or iterator invalidated */
	cmzn_element *nextElement()
	{
		if (this->fe_mesh)
		{
			const DsLabelIndex elementIndex = this->iter->nextIndex();
			return this->fe_mesh->getElement(elementIndex);
		}
		return 0;
	}

	void setIndex(DsLabelIndex index)
	{
		this->iter->setIndex(index);
	}
};

#endif /* !defined (FINITE_ELEMENT_MESH_HPP) */
