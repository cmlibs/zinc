/**
 * field.i
 *
 * Swig interface file for wrapping api functions in api/field.hpp
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") field

%include "fieldarraytypemap.i"
%include "doublevaluesarraytypemap.i"
%include "fieldoperators.i"
%include "pyzincstringhandling.i"

%import "differentialoperator.i"
%import "element.i"
%import "fieldcache.i"
%import "fieldmodule.i"
%import "fieldsmoothing.i"
%import "region.i"
%import "streamimage.i"

%{
#include "zinc/fieldalias.hpp"
#include "zinc/fieldarithmeticoperators.hpp"
#include "zinc/fieldcomposite.hpp"
#include "zinc/fieldconditional.hpp"
#include "zinc/fieldconstant.hpp"
#include "zinc/fieldcoordinatetransformation.hpp"
#include "zinc/fieldderivatives.hpp"
#include "zinc/fieldfibres.hpp"
#include "zinc/fieldfiniteelement.hpp"
#include "zinc/fieldgroup.hpp"
#include "zinc/fieldimage.hpp"
#include "zinc/fieldimageprocessing.hpp"
#include "zinc/fieldlogicaloperators.hpp"
#include "zinc/fieldmatrixoperators.hpp"
#include "zinc/fieldmeshoperators.hpp"
#include "zinc/fieldnodesetoperators.hpp"
#include "zinc/fieldsceneviewerprojection.hpp"
#include "zinc/fieldsubobjectgroup.hpp"
#include "zinc/fieldvectoroperators.hpp"
#include "zinc/fieldtime.hpp"
#include "zinc/fieldtrigonometry.hpp"
#include "zinc/field.hpp"
#include "zinc/fieldcache.hpp"
#include "zinc/fieldsmoothing.hpp"
#include "zinc/streamimage.hpp"
%}

%include "zinc/field.hpp"
%include "zinc/fieldcomposite.hpp"
%include "zinc/fieldconditional.hpp"
%include "zinc/fieldconstant.hpp"
%include "zinc/fieldcoordinatetransformation.hpp"
%include "zinc/fieldderivatives.hpp"
%include "zinc/fieldfibres.hpp"
%include "zinc/fieldfiniteelement.hpp"
%include "zinc/fieldsubobjectgroup.hpp"
%include "zinc/fieldgroup.hpp"
%include "zinc/fieldimage.hpp"
%include "zinc/fieldimageprocessing.hpp"
%include "zinc/fieldlogicaloperators.hpp"
%include "zinc/fieldmatrixoperators.hpp"
%include "zinc/fieldmeshoperators.hpp"
%include "zinc/fieldnodesetoperators.hpp"
%include "zinc/fieldsceneviewerprojection.hpp"
%include "zinc/fieldvectoroperators.hpp"
%include "zinc/fieldtime.hpp"
%include "zinc/fieldtrigonometry.hpp"
%include "zinc/fieldarithmeticoperators.hpp"
%include "zinc/fieldalias.hpp"
