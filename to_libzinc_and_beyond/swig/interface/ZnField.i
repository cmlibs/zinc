/*******************************************************************************
 * ZnField.i
 * 
 * Swig interface file for wrapping api functions in api/field.hpp
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
 * Portions created by the Initial Developer are Copyright (C) 2010
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

%module Field
%include "ZnFieldArrayTypemap.i"
%include "ZnDoubleValuesArrayTypemap.i"
%include "ZnFieldOperators.i"

%ignore FieldCache;
%ignore ElementBasis;
%ignore ElementTemplate;
%ignore Element;
%ignore ElementIterator;
%ignore Mesh;
%ignore MeshGroup;
%ignore Node;
%ignore NodeIterator;
%ignore Nodeset;
%ignore NodesetGroup;
%ignore DifferentialOperator;
%ignore Region;
%ignore FieldModule;
%ignore StreamInformationRegion;
%ignore StreamResource;
%ignore StreamResourceFile;
%ignore StreamResourceMemory;
%ignore TimeKeeper;
%ignore Zn::operator+;

%{
#include "api++/field.hpp"
#include "api++/fieldtypesarithmeticoperators.hpp"
#include "api++/fieldtypescomposite.hpp"
#include "api++/fieldtypesconditional.hpp"
#include "api++/fieldtypesconstant.hpp"
#include "api++/fieldtypescoordinatetransformation.hpp"
#include "api++/fieldtypesfiniteelement.hpp"
#include "api++/fieldtypesgroup.hpp"
#include "api++/fieldtypesimage.hpp"
#include "api++/fieldtypeslogicaloperators.hpp"
#include "api++/fieldtypesmatrixoperators.hpp"
#include "api++/fieldtypesnodesetoperators.hpp"
#include "api++/fieldtypessubobjectgroup.hpp"
#include "api++/fieldtypesvectoroperators.hpp"
#include "api++/fieldtypestime.hpp"
#include "api++/fieldtypestrigonometry.hpp"
%}

%include "api++/differentialoperator.hpp"
%include "api++/timekeeper.hpp"
%include "api++/stream.hpp"
%include "api++/region.hpp"
%include "api++/fieldcache.hpp"
%include "api++/field.hpp"
%include "api++/element.hpp"
%include "api++/node.hpp"
%include "api++/fieldmodule.hpp"
%include "api++/fieldtypesarithmeticoperators.hpp"
%include "api++/fieldtypescomposite.hpp"
%include "api++/fieldtypesconditional.hpp"
%include "api++/fieldtypesconstant.hpp"
%include "api++/fieldtypescoordinatetransformation.hpp"
%include "api++/fieldtypesfiniteelement.hpp"
%include "api++/fieldtypessubobjectgroup.hpp"
%include "api++/fieldtypesgroup.hpp"
%include "api++/fieldtypesimage.hpp"
%include "api++/fieldtypeslogicaloperators.hpp"
%include "api++/fieldtypesmatrixoperators.hpp"
%include "api++/fieldtypesnodesetoperators.hpp"
%include "api++/fieldtypesvectoroperators.hpp"
%include "api++/fieldtypestime.hpp"
%include "api++/fieldtypestrigonometry.hpp"
