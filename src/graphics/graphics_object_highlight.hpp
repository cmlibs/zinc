/*******************************************************************************
FILE : graphics_object_highlight.hpp

==============================================================================*/
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once


class GraphicsHighlightFunctor
{
public:
	virtual ~GraphicsHighlightFunctor()
	{
	}

	/** Override to return true if object with index is selected for highlighting */
	virtual bool query(int index) = 0;

};


class AllSelectedGraphicsHighlightFunctor : public GraphicsHighlightFunctor
{
public:
	inline bool query(int)
	{
		return true;
	}

};


/** For nodeset group, mesh group which support bool containsIndex(int) */
template <typename DOMAIN_GROUP>
class DomainGroupGraphicsHighlightFunctor : public GraphicsHighlightFunctor
{
	DOMAIN_GROUP* domainGroup;

public:
	DomainGroupGraphicsHighlightFunctor(DOMAIN_GROUP* domainGroupIn)
		: domainGroup(domainGroupIn)
	{
	}

	virtual bool query(int index)
	{
		return domainGroup->containsIndex(index);
	}

};
