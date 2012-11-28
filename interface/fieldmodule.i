/*******************************************************************************
 * FieldModule.i
 * 
 * Swig interface file for wrapping api functions in api/fieldmodule.hpp
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
 * The Original Code is libZinc.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2012
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

%module(package="zinc") fieldmodule

%include "doublevaluesarraytypemap.i"
%include "fieldarraytypemap.i"

/* Interestingly, swig does not recognise the typemap to Field **, 
	therefore using the following to wrap it around*/
%extend zinc::FieldModule
{
	zinc::FieldConcatenate createConcatenate(int numberOfSourceFields, void **sourceFieldsVoid)
	{
		zinc::Field **temp = (zinc::Field **)sourceFieldsVoid;
		zinc::Field *fieldsArray = new zinc::Field[numberOfSourceFields];
		for (int i = 0; i < numberOfSourceFields; i++)
		{
			fieldsArray[i] = *temp[i];
		}
    	zinc::FieldConcatenate return_field = ($self)->createConcatenate(numberOfSourceFields, fieldsArray);
    	delete[] fieldsArray;
    	return return_field;
	}
	
	zinc::FieldCrossProduct createCrossProduct(int numberOfSourceFields, void **sourceFieldsVoid)
	{
		zinc::Field **temp = (zinc::Field **)sourceFieldsVoid;
		zinc::Field *fieldsArray = new zinc::Field[numberOfSourceFields];
		for (int i = 0; i < numberOfSourceFields; i++)
		{
			fieldsArray[i] = *temp[i];
		}
    	zinc::FieldCrossProduct return_field = ($self)->createCrossProduct(numberOfSourceFields + 1, fieldsArray);
    	delete[] fieldsArray;
    	return return_field;
	}
};

%import "timesequence.i"
%import "optimisation.i"
%import "field.i"

%{
#include "zinc/fieldtypesarithmeticoperators.hpp"
#include "zinc/fieldtypescomposite.hpp"
#include "zinc/fieldtypesconditional.hpp"
#include "zinc/fieldtypesconstant.hpp"
#include "zinc/fieldtypescoordinatetransformation.hpp"
#include "zinc/fieldtypesfiniteelement.hpp"
#include "zinc/fieldtypessubobjectgroup.hpp"
#include "zinc/fieldtypesgroup.hpp"
#include "zinc/fieldtypesimage.hpp"
#include "zinc/fieldtypeslogicaloperators.hpp"
#include "zinc/fieldtypesmatrixoperators.hpp"
#include "zinc/fieldtypesnodesetoperators.hpp"
#include "zinc/fieldtypestime.hpp"
#include "zinc/fieldtypestrigonometry.hpp"
#include "zinc/fieldtypesvectoroperators.hpp"
#include "zinc/fieldmodule.hpp"
%}

%include "zinc/fieldmodule.hpp"

