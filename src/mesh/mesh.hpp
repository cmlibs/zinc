/**
 * FILE : mesh.hpp
 *
 * Interface to mesh implementation.
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "cmlibs/zinc/element.h"
#include "cmlibs/zinc/elementtemplate.h"
#include "cmlibs/zinc/mesh.h"
#include "finite_element/finite_element_mesh.hpp"

struct cmzn_mesh
{
	friend cmzn_region;

protected:
	FE_mesh* feMesh;
	int access_count;

	cmzn_mesh(FE_mesh* feMeshIn) :
		feMesh(cmzn::Access(feMeshIn)),
		access_count(1)
	{
	}

	virtual ~cmzn_mesh()
	{
		cmzn::Deaccess(this->feMesh);
	}

public:

	cmzn_mesh* access()
	{
		++access_count;
		return this;
	}

	static void deaccess(cmzn_mesh*& mesh);

	/** @return  Number of references held to object */
	int getAccessCount() const
	{
		return this->access_count;
	}

	/** @return  Accessed new element or nullptr if failed */
	virtual cmzn_element* createElement(int identifier, cmzn_elementtemplate* elementtemplate);

	virtual bool containsElement(cmzn_element* element) const
	{
		return this->feMesh->containsElement(element);
	}

	/** @return  Accessed element template, or nullptr if failed */
	cmzn_elementtemplate* createElementtemplate() const;

	virtual cmzn_elementiterator* createElementiterator() const
	{
		return this->feMesh->createElementiterator();
	}

	virtual int destroyAllElements()
	{
		return this->feMesh->destroyAllElements();
	}

	int destroyElement(cmzn_element* element)
	{
		if (this->containsElement(element))
		{
			return this->feMesh->destroyElement(element);
		}
		return CMZN_ERROR_ARGUMENT;
	}

	int destroyElementsConditional(cmzn_field* conditional_field);

	/** @return  Non-accessed element, or nullptr if not found */
	virtual cmzn_element* findElementByIdentifier(int identifier) const
	{
		return this->feMesh->findElementByIdentifier(identifier);
	}

	int getDimension() const
	{
		return this->feMesh->getDimension();
	}

	FE_mesh* getFeMesh() const
	{
		return this->feMesh;
	}

	/** @return  Non-allocated name of master mesh */
	const char* getMasterMeshName() const
	{
		return this->feMesh->getName();
	}

	/** @return  Allocated name, or nullptr if failed */
	virtual char* getName() const;
	
	/** @return  Non-accessed master mesh */
	cmzn_mesh* getMasterMesh() const;

	/** @return  Non-accessed owning region */
	cmzn_region* getRegion() const;

	virtual int getSize() const
	{
		return this->feMesh->getSize();
	}

	/** @return  True if mesh has recorded changes in membership */
	virtual bool hasMembershipChanges() const;

};
