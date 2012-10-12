/*******************************************************************************
 * ZnFieldModule.i
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

%module FieldModule
%include "ZnDoubleValuesArrayTypemap.i"
%include "ZnFieldArrayTypemap.i"

/* Interestingly, swig does not recognise the typemap to Field **, 
	therefore using the following to wrap it around*/
%extend Zn::FieldModule
{
	Zn::FieldConcatenate createConcatenate(int numberOfSourceFields, void **sourceFieldsVoid)
	{
		Zn::Field **temp = (Zn::Field **)sourceFieldsVoid;
		Zn::Field *fieldsArray = new Zn::Field[numberOfSourceFields];
		for (int i = 0; i < numberOfSourceFields; i++)
		{
			fieldsArray[i] = *temp[i];
		}
    	Zn::FieldConcatenate return_field = ($self)->createConcatenate(numberOfSourceFields, fieldsArray);
    	delete[] fieldsArray;
    	return return_field;
	}
	
	Zn::FieldCrossProduct createCrossProduct(int numberOfSourceFields, void **sourceFieldsVoid)
	{
		Zn::Field **temp = (Zn::Field **)sourceFieldsVoid;
		Zn::Field *fieldsArray = new Zn::Field[numberOfSourceFields];
		for (int i = 0; i < numberOfSourceFields; i++)
		{
			fieldsArray[i] = *temp[i];
		}
    	Zn::FieldCrossProduct return_field = ($self)->createCrossProduct(numberOfSourceFields + 1, fieldsArray);
    	delete[] fieldsArray;
    	return return_field;
	}
};

%ignore FieldAdd;
%ignore FieldPower;
%ignore FieldMultiply;
%ignore FieldDivide;
%ignore FieldSubtract;
%ignore FieldSumComponents;
%ignore FieldLog;
%ignore FieldSqrt;
%ignore FieldExp;
%ignore FieldAbs;
%ignore FieldIdentity;
%ignore FieldComponent;
%ignore FieldConcatenate;
%ignore FieldIf;
%ignore FieldConstant;
%ignore FieldStringConstant;
%ignore FieldCoordinateTransformation;
%ignore FieldVectorCoordinateTransformation;
%ignore FieldFiniteElement;
%ignore FieldEmbedded;
%ignore FieldFindMeshLocation;
%ignore FieldNodeValue;
%ignore FieldStoredMeshLocation;
%ignore FieldGroup;
%ignore FieldImage;
%ignore FieldAnd;
%ignore FieldEqualTo;
%ignore FieldGreaterThan;
%ignore FieldLessThan;
%ignore FieldOr;
%ignore FieldNot;
%ignore FieldXor;
%ignore FieldDeterminant;
%ignore FieldEigenvalues;
%ignore FieldEigenvectors;
%ignore FieldMatrixInvert;
%ignore FieldMatrixMultiply;
%ignore FieldProjection;
%ignore FieldTranspose;
%ignore FieldNodesetSum;
%ignore FieldNodesetMean;
%ignore FieldNodesetSumSquares;
%ignore FieldNodesetMeanSquares;
%ignore FieldElementGroup;
%ignore FieldNodeGroup;
%ignore FieldTimeLookup;
%ignore FieldTimeValue;
%ignore FieldSin;
%ignore FieldCos;
%ignore FieldTan;
%ignore FieldAsin;
%ignore FieldAcos;
%ignore FieldAtan;
%ignore FieldAtan2;
%ignore FieldCrossProduct;
%ignore FieldDotProduct;
%ignore FieldMagnitude;
%ignore FieldNormalise;
%ignore ElementBasis;
%ignore ElementTemplate;
%ignore Element;
%ignore ElementIterator;
%ignore Field;
%ignore FieldCache;
%ignore Mesh;
%ignore MeshGroup;
%ignore Node;
%ignore NodeIterator;
%ignore Nodeset;
%ignore NodesetGroup;
%ignore Optimisation;
%ignore TimeSequence;
%ignore TimeKeeper;
%ignore Zn::operator+;
%ignore Zn::operator*;
%ignore Zn::operator/;
%ignore Zn::operator-;
%ignore Zn::log;
%ignore Zn::sqrt;
%ignore Zn::exp;
%ignore Zn::abs;
%ignore Zn::operator&&;
%ignore Zn::operator==;
%ignore Zn::operator>;
%ignore Zn::operator<;
%ignore Zn::operator||;
%ignore Zn::operator!;
%ignore Region;
%ignore StreamInformationImage;
%ignore StreamInformationRegion;
%ignore StreamResource;
%ignore StreamResourceFile;
%ignore StreamResourceMemory;

%{
#include "api++/field.hpp"
#include "api++/fieldtypesarithmeticoperators.hpp"
#include "api++/fieldtypescomposite.hpp"
#include "api++/fieldtypesconditional.hpp"
#include "api++/fieldtypesconstant.hpp"
#include "api++/fieldtypescoordinatetransformation.hpp"
#include "api++/fieldtypesfiniteelement.hpp"
#include "api++/fieldtypessubobjectgroup.hpp"
#include "api++/fieldtypesgroup.hpp"
#include "api++/fieldtypesimage.hpp"
#include "api++/fieldtypeslogicaloperators.hpp"
#include "api++/fieldtypesmatrixoperators.hpp"
#include "api++/fieldtypesnodesetoperators.hpp"
#include "api++/fieldtypestime.hpp"
#include "api++/fieldtypestrigonometry.hpp"
#include "api++/fieldtypesvectoroperators.hpp"
#include "api++/fieldmodule.hpp"
%}

%include "api++/fieldcache.hpp"
%include "api++/field.hpp"
%include "api++/element.hpp"
%include "api++/node.hpp"
%include "api++/timesequence.hpp"
%include "api++/optimisation.hpp"
%include "api++/fieldmodule.hpp"
