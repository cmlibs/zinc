/***************************************************************************//**
 * FILE : cmiss_element_private.hpp
 *
 * Private header file of cmzn_element, finite element meshes.
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (CMZN_ELEMENT_PRIVATE_HPP)
#define CMZN_ELEMENT_PRIVATE_HPP

#include "cmlibs/zinc/element.h"
#include "cmlibs/zinc/region.h"
#include "datastore/labelschangelog.hpp"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_mesh.hpp"


struct cmzn_meshchanges
{
private:
	cmzn_fieldmoduleevent *event; // accessed
	DsLabelsChangeLog *changeLog; // Accessed from object obtained from correct mesh
	int access_count;

	cmzn_meshchanges(cmzn_fieldmoduleevent *eventIn, cmzn_mesh *meshIn);
	~cmzn_meshchanges();

public:

	static cmzn_meshchanges *create(cmzn_fieldmoduleevent *eventIn, cmzn_mesh *meshIn);

	cmzn_meshchanges *access()
	{
		++(this->access_count);
		return this;
	}

	static int deaccess(cmzn_meshchanges* &meshchanges);

	/** Also checks related changes to parent elements and nodes */
	cmzn_element_change_flags getElementChangeFlags(cmzn_element *element);

	int getNumberOfChanges()
	{
		if (this->changeLog->isAllChange())
			return -1;
		return this->changeLog->getChangeCount();
	}

	cmzn_element_change_flags getSummaryElementChangeFlags()
	{
		return this->changeLog->getChangeSummary();
	}
};

#endif /* !defined (CMZN_ELEMENT_PRIVATE_HPP) */
