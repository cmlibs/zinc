//******************************************************************************
// FILE : field_location.hpp
//
// LAST MODIFIED : 9 August 2006
//
// DESCRIPTION :
// An class hierarchy for specifying locations at which to evaluate fields.
// These are transient objects used internally in Computed_fields and so do not
// access, copy or allocate their members.
//==============================================================================
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
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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
#if !defined (__FIELD_LOCATION_HPP__)
#define __FIELD_LOCATION_HPP__

class Field_location
{
protected:
	FE_value time;
	int number_of_derivatives;

   Field_location(FE_value time = 0.0, int number_of_derivatives = 0) : 
		time(time), number_of_derivatives(number_of_derivatives)
	{};

	/* Abstract virtual destructor as we will not make objects of this
		parent class */
	virtual ~Field_location()
	{};

public:
	FE_value get_time()
	{
		return time;
	}

	int get_number_of_derivatives()
	{
		return number_of_derivatives;
	}
};

class Field_element_xi_location : public Field_location
{
private:
	struct FE_element *element;
	FE_value *xi;
	struct FE_element *top_level_element;

public:
	Field_element_xi_location(struct FE_element *element, 
		FE_value *xi = NULL, FE_value time = 0.0, 
		struct FE_element *top_level_element = NULL, int number_of_derivatives = 0):
		Field_location(time, number_of_derivatives),
		element(element), xi(xi), top_level_element(top_level_element)
	{
	}
	
   ~Field_element_xi_location()
	{
	}

	struct FE_element *get_element()
	{
		return element;
	}

	FE_value *get_xi()
	{
		return xi;
	}

	FE_element *get_top_level_element()
	{
		return top_level_element;
	}
};

class Field_node_location : public Field_location
{
private:
	struct FE_node *node;

public:
	Field_node_location(struct FE_node *node,
		FE_value time = 0, int number_of_derivatives = 0):
		Field_location(time, number_of_derivatives), node(node)
	{
	}
	
   ~Field_node_location()
	{
	}

	FE_node *get_node()
	{
		return node;
	}

};

#endif /* !defined (__FIELD_LOCATION_HPP__) */
