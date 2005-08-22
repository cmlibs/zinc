/*******************************************************************************
FILE : select_control_curve.c

LAST MODIFIED : 20 April 2000

DESCRIPTION :
Declares select widget functions for Control_curve objects.
==============================================================================*/
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
 * Portions created by the Initial Developer are Copyright (C) 2005
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

#include "curve/control_curve.h"
#include "select/select_control_curve.h"
#include "select/select_private.h"

/*
Module types
------------
*/
FULL_DECLARE_SELECT_STRUCT_TYPE(Control_curve);

/*
Module functions
----------------
*/
/*
Module functions
----------------
*/
DECLARE_DEFAULT_SELECT_MANAGER_MODIFY_IDENTIFIER_AS_NAME_FUNCTION(Control_curve)
DECLARE_DEFAULT_SELECT_FIND_BY_IDENTIFIER_AS_NAME_IN_MANAGER_FUNCTION( \
	Control_curve)

PROTOTYPE_SELECT_MANAGER_CREATE_FUNCTION(Control_curve)
/*******************************************************************************
LAST MODIFIED : 20 April 2000

DESCRIPTION :
Creates a new struct Control_curve with a unique identifier in <object_manager>.
It <template_object> is supplied, the new object will be a copy of it and its
identifier may be derived from it.
???RC Should be part of manager.h
==============================================================================*/
{
	char *new_curve_name, *temp_name;
	static char copy_string[]="+";
	static char default_name[]="new";
	struct Control_curve *new_curve;

	ENTER(SELECT_MANAGER_CREATE(Control_curve));
	/* 1. Get a new, unique identifier for the curve */
	if (template_object)
	{
		GET_NAME(Control_curve)(template_object,&new_curve_name);
	}
	else
	{
		/* Give the new curve some default name */
		if (ALLOCATE(new_curve_name,char,strlen(default_name)+1))
		{
			strcpy(new_curve_name,default_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"SELECT_MANAGER_CREATE(Control_curve).  Could not allocate new name");
		}
	}
	/* 3. Ensure the new identifier is not used by some curve in the manager */
	/*    and copy it to the new curve. */
	/* Keep on appending the copy_string onto the name until it is unique: */
	while ((new_curve_name)&&(FIND_BY_IDENTIFIER_IN_MANAGER(
		Control_curve,name)(new_curve_name,object_manager)))
	{
		if (REALLOCATE(temp_name,new_curve_name,char,
			strlen(new_curve_name)+strlen(copy_string)+1))
		{
			new_curve_name=temp_name;
			strcat(new_curve_name,copy_string);
		}
		else
		{
			DEALLOCATE(new_curve_name);
			display_message(ERROR_MESSAGE,
				"SELECT_MANAGER_CREATE(Control_curve"
				").  Could not give curve a unique name");
		}
	}
	if (new_curve_name)
	{
		new_curve=CREATE(Control_curve)(new_curve_name,LINEAR_LAGRANGE,1);
		/* copy template_object contents into new object */
		if (template_object)
		{
			MANAGER_COPY_WITHOUT_IDENTIFIER(Control_curve,name)(
				new_curve,template_object);
		}
		DEALLOCATE(new_curve_name);
	}
	else
	{
		new_curve=(struct Control_curve *)NULL;
	}
	if (!new_curve)
	{
		display_message(ERROR_MESSAGE,
			"SELECT_MANAGER_CREATE(Control_curve).  Unable to create new curve");
	}
	LEAVE;

	return (new_curve);
} /* SELECT_MANAGER_CREATE(Control_curve) */

DECLARE_SELECT_MODULE_FUNCTIONS(Control_curve,"Curve named:")

/*
Global functions
----------------
*/
DECLARE_SELECT_GLOBAL_FUNCTIONS(Control_curve)

