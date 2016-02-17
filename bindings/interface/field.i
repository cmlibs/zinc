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
#include "opencmiss/zinc/fieldalias.hpp"
#include "opencmiss/zinc/fieldarithmeticoperators.hpp"
#include "opencmiss/zinc/fieldcomposite.hpp"
#include "opencmiss/zinc/fieldconditional.hpp"
#include "opencmiss/zinc/fieldconstant.hpp"
#include "opencmiss/zinc/fieldcoordinatetransformation.hpp"
#include "opencmiss/zinc/fieldderivatives.hpp"
#include "opencmiss/zinc/fieldfibres.hpp"
#include "opencmiss/zinc/fieldfiniteelement.hpp"
#include "opencmiss/zinc/fieldgroup.hpp"
#include "opencmiss/zinc/fieldimage.hpp"
#include "opencmiss/zinc/fieldimageprocessing.hpp"
#include "opencmiss/zinc/fieldlogicaloperators.hpp"
#include "opencmiss/zinc/fieldmatrixoperators.hpp"
#include "opencmiss/zinc/fieldmeshoperators.hpp"
#include "opencmiss/zinc/fieldnodesetoperators.hpp"
#include "opencmiss/zinc/fieldsceneviewerprojection.hpp"
#include "opencmiss/zinc/fieldsubobjectgroup.hpp"
#include "opencmiss/zinc/fieldvectoroperators.hpp"
#include "opencmiss/zinc/fieldtime.hpp"
#include "opencmiss/zinc/fieldtrigonometry.hpp"
#include "opencmiss/zinc/field.hpp"
#include "opencmiss/zinc/fieldcache.hpp"
#include "opencmiss/zinc/fieldsmoothing.hpp"
#include "opencmiss/zinc/streamimage.hpp"
%}

%include "opencmiss/zinc/field.hpp"
%include "opencmiss/zinc/fieldcomposite.hpp"
%include "opencmiss/zinc/fieldconditional.hpp"
%include "opencmiss/zinc/fieldconstant.hpp"
%include "opencmiss/zinc/fieldcoordinatetransformation.hpp"
%include "opencmiss/zinc/fieldderivatives.hpp"
%include "opencmiss/zinc/fieldfibres.hpp"
%include "opencmiss/zinc/fieldfiniteelement.hpp"
%include "opencmiss/zinc/fieldsubobjectgroup.hpp"
%include "opencmiss/zinc/fieldgroup.hpp"
%include "opencmiss/zinc/fieldimage.hpp"
%include "opencmiss/zinc/fieldimageprocessing.hpp"
%include "opencmiss/zinc/fieldlogicaloperators.hpp"
%include "opencmiss/zinc/fieldmatrixoperators.hpp"
%include "opencmiss/zinc/fieldmeshoperators.hpp"
%include "opencmiss/zinc/fieldnodesetoperators.hpp"
%include "opencmiss/zinc/fieldsceneviewerprojection.hpp"
%include "opencmiss/zinc/fieldvectoroperators.hpp"
%include "opencmiss/zinc/fieldtime.hpp"
%include "opencmiss/zinc/fieldtrigonometry.hpp"
%include "opencmiss/zinc/fieldarithmeticoperators.hpp"
%include "opencmiss/zinc/fieldalias.hpp"
