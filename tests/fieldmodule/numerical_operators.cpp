/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * Test all numerical field operators and their derivatives.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <opencmiss/zinc/core.h>
#include <opencmiss/zinc/result.h>

#include <opencmiss/zinc/field.hpp>
#include <opencmiss/zinc/fieldarithmeticoperators.hpp>
#include <opencmiss/zinc/fieldcache.hpp>
#include <opencmiss/zinc/fieldcomposite.hpp>
#include <opencmiss/zinc/fieldconditional.hpp>
#include <opencmiss/zinc/fieldconstant.hpp>
#include <opencmiss/zinc/fieldcoordinatetransformation.hpp>
#include <opencmiss/zinc/fieldderivatives.hpp>
#include <opencmiss/zinc/fieldlogicaloperators.hpp>
#include <opencmiss/zinc/fieldmatrixoperators.hpp>
#include <opencmiss/zinc/fieldtrigonometry.hpp>
#include <opencmiss/zinc/fieldvectoroperators.hpp>

#include "zinctestsetupcpp.hpp"

#include "test_resources.h"
#include <cctype>
#include <cmath>
#include <iostream>
#include <string>

typedef void (*Binary_operator)(int, const double *, const double *, double *);

struct Field_binary_operator
{
	const char *name;
	Field field;
	Binary_operator binary_operator;
	const double v_tol;
	const double d1_tol;
	const double d2_tol;
	double max_v_error;
	double max_d1_error;
	double max_d2_error;
};

void binary_operator_add(int count, const double *a, const double *b, double *result)
{
	for (int i = 0; i < count; ++i)
		result[i] = a[i] + b[i];
}

void binary_operator_subtract(int count, const double *a, const double *b, double *result)
{
	for (int i = 0; i < count; ++i)
		result[i] = a[i] - b[i];
}

void binary_operator_multiply(int count, const double *a, const double *b, double *result)
{
	for (int i = 0; i < count; ++i)
		result[i] = a[i]*b[i];
}

void binary_operator_divide(int count, const double *a, const double *b, double *result)
{
	for (int i = 0; i < count; ++i)
		result[i] = a[i]/b[i];
}

void binary_operator_power(int count, const double *a, const double *b, double *result)
{
	for (int i = 0; i < count; ++i)
		result[i] = pow(a[i], b[i]);
}

void unary_operator_log(int count, const double *a, const double *, double *result)
{
	for (int i = 0; i < count; ++i)
		result[i] = log(a[i]);
}

void unary_operator_sqrt(int count, const double *a, const double *, double *result)
{
	for (int i = 0; i < count; ++i)
		result[i] = sqrt(a[i]);
}

void unary_operator_exp(int count, const double *a, const double *, double *result)
{
	for (int i = 0; i < count; ++i)
		result[i] = exp(a[i]);
}

void unary_operator_abs(int count, const double *, const double *b, double *result)
{
	for (int i = 0; i < count; ++i)
		result[i] = fabs(b[i]);
}

void unary_operator_identity(int count, const double *a, const double *, double *result)
{
	for (int i = 0; i < count; ++i)
		result[i] = a[i];
}

void unary_operator_reverse(int count, const double *a, const double *, double *result)
{
	for (int i = 0; i < count; ++i)
		result[i] = a[count - i - 1];
}

void binary_operator_concatenate_a2b3b1(int count, const double *a, const double *b, double *result)
{
	result[0] = a[1];
	result[1] = b[2];
	result[2] = b[0];
}

void binary_operator_if_b_lt_zero(int count, const double *a, const double *b, double *result)
{
	for (int i = 0; i < count; ++i)
		if (b[i] < 0.0)
			result[i] = a[i];
		else
			result[i] = b[i];
}

void constant_operator_m020507(int count, const double *, const double *, double *result)
{
	result[0] = -0.2;
	result[1] = 0.5;
	result[2] = 0.7;
}

void unary_operator_coordinate_transformation_spherical_polar_to_rc(int count, const double *a, const double *, double *result)
{
	const double r = a[0];
	const double theta = a[1];
	const double phi = a[2];
	const double cos_phi = cos(phi);
	result[0] = r*cos_phi*cos(theta);
	result[1] = r*cos_phi*sin(theta);
	result[2] = r*sin(phi);
}

void binary_operator_matrix_multiply_aba_m020507(int count, const double *a, const double *b, double *result)
{
	double c[3] = { -0.2, 0.5, 0.7 };
	// assume alternate rows of matrix are a, b, a etc. then multiply c
	double ac = 0.0;
	for (int i = 0; i < count; ++i)
		ac += a[i]*c[i];
	double bc = 0.0;
	for (int i = 0; i < count; ++i)
		bc += b[i]*c[i];
	result[0] = ac;
	result[1] = bc;
	result[2] = ac;
}

void binary_operator_matrix_multiply_m020507_aba(int count, const double *a, const double *b, double *result)
{
	double c[3] = { -0.2, 0.5, 0.7 };
	result[0] = c[0]*a[0] + c[1]*b[0] + c[2]*a[0];
	result[1] = c[0]*a[1] + c[1]*b[1] + c[2]*a[1];
	result[2] = c[0]*a[2] + c[1]*b[2] + c[2]*a[2];
}

void binary_operator_matrix_multiply_aba_b(int count, const double *a, const double *b, double *result)
{
	// assume alternate rows of matrix are a, b, a etc. then multiply b
	double ab = 0.0;
	for (int i = 0; i < count; ++i)
		ab += a[i]*b[i];
	double bb = 0.0;
	for (int i = 0; i < count; ++i)
		bb += b[i]*b[i];
	result[0] = ab;
	result[1] = bb;
	result[2] = ab;
}

void binary_operator_matrix_multiply_a_aba(int count, const double *a, const double *b, double *result)
{
	result[0] = a[0]*a[0] + a[1]*b[0] + a[2]*a[0];
	result[1] = a[0]*a[1] + a[1]*b[1] + a[2]*a[1];
	result[2] = a[0]*a[2] + a[1]*b[2] + a[2]*a[2];
}

void binary_operator_transpose_aba_upper(int count, const double *a, const double *b, double *result)
{
	// return transpose of aba upper off-diagonal entries
	result[0] = b[0];
	result[1] = a[0];
	result[2] = a[1];
}

void unary_operator_sin(int count, const double *a, const double *, double *result)
{
	for (int i = 0; i < count; ++i)
		result[i] = sin(a[i]);
}

void unary_operator_cos(int count, const double *a, const double *, double *result)
{
	for (int i = 0; i < count; ++i)
		result[i] = cos(a[i]);
}

void unary_operator_tan(int count, const double *a, const double *, double *result)
{
	for (int i = 0; i < count; ++i)
		result[i] = tan(a[i]);
}

void unary_operator_asin(int count, const double *a, const double *, double *result)
{
	// assume a[i] clamped to [ -1, +1 ]
	for (int i = 0; i < count; ++i)
		result[i] = asin(a[i]);
}

void unary_operator_acos(int count, const double *a, const double *, double *result)
{
	// assume a[i] clamped to [ -1, +1 ]
	for (int i = 0; i < count; ++i)
		result[i] = acos(a[i]);
}

void unary_operator_atan(int count, const double *a, const double *, double *result)
{
	for (int i = 0; i < count; ++i)
		result[i] = atan(a[i]);
}

void binary_operator_atan2(int count, const double *a, const double *b, double *result)
{
	for (int i = 0; i < count; ++i)
		result[i] = atan2(a[i], b[i]);
}

void binary_operator_cross_product(int count, const double *a, const double *b, double *result)
{
	result[0] = a[1]*b[2] - a[2]*b[1];
	result[1] = a[2]*b[0] - a[0]*b[2];
	result[2] = a[0]*b[1] - a[1]*b[0];
}

void binary_operator_dot_product(int count, const double *a, const double *b, double *result)
{
	// put result in all components
	double ab = 0.0;
	for (int i = 0; i < count; ++i)
		ab += a[i]*b[i];
	for (int i = 0; i < count; ++i)
		result[i] = ab;
}

void unary_operator_magnitude(int count, const double *a, const double *, double *result)
{
	// put result in all components
	double maga = 0.0;
	for (int i = 0; i < count; ++i)
		maga += a[i]*a[i];
	maga = sqrt(maga);
	for (int i = 0; i < count; ++i)
		result[i] = maga;
}

void unary_operator_normalise(int count, const double *a, const double *, double *result)
{
	double maga = 0.0;
	for (int i = 0; i < count; ++i)
		maga += a[i]*a[i];
	maga = sqrt(maga);
	for (int i = 0; i < count; ++i)
		result[i] = a[i]/maga;
}

void unary_operator_sum_components(int count, const double *a, const double *, double *result)
{
	// put result in all components
	double sum = 0.0;
	for (int i = 0; i < count; ++i)
		sum += a[i];
	for (int i = 0; i < count; ++i)
		result[i] = sum;
}

TEST(ZincField, numerical_operators_with_derivatives)
{
	ZincTestSetupCpp zinc;

    EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(resourcePath("fieldmodule/cube_tricubic_deformed.exfile").c_str()));
    EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(resourcePath("fieldmodule/cube_triquadratic_delta.ex2").c_str()));

	Field fielda = zinc.fm.findFieldByName("deformed");
	EXPECT_TRUE(fielda.isValid());
	EXPECT_EQ(3, fielda.getNumberOfComponents());
	Field fieldb = zinc.fm.findFieldByName("delta");
	EXPECT_TRUE(fieldb.isValid());
	EXPECT_EQ(3, fieldb.getNumberOfComponents());

	// all test fields are forced to have 3 components by repeating, alternating or sampling values
	const int component_indexes_reverse[3] = { 3, 2, 1 };
	const int component_indexes_matrix3x3_upper[3] = { 2, 3, 6 };
	const int component_indexes_111[3] = { 1, 1, 1 };
	const double zero3[3] = { 0.0, 0.0, 0.0 };
	const double constants_m020507[3] = { -0.2, 0.5, 0.7 };
	Field concatenate_fields_a2b3b1[3] = {
		zinc.fm.createFieldComponent(fielda, 2),
		zinc.fm.createFieldComponent(fieldb, 3),
		zinc.fm.createFieldComponent(fieldb, 1)
	};
	Field field_const_vector = zinc.fm.createFieldConstant(3, constants_m020507);
	FieldIdentity fielda_spherical_polar = zinc.fm.createFieldIdentity(fielda);
	fielda_spherical_polar.setCoordinateSystemType(Field::COORDINATE_SYSTEM_TYPE_SPHERICAL_POLAR);
	FieldCoordinateTransformation fieldCoordinateTransformation_spherical_polar_to_rc = zinc.fm.createFieldCoordinateTransformation(fielda_spherical_polar);
	fieldCoordinateTransformation_spherical_polar_to_rc.setCoordinateSystemType(Field::COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN);
	Field concatenate_fields_aba[3] = { fielda, fieldb, fielda };
	Field field_matrix_aba = zinc.fm.createFieldConcatenate(3, concatenate_fields_aba);
	const int field_binary_operator_count = 32;
	Field_binary_operator field_binary_operators[field_binary_operator_count] =
	{
		// arithmetic operators
		{ "add",         zinc.fm.createFieldAdd(fielda, fieldb),      binary_operator_add,      1.0E-12, 1.0E-7,  2.5E-2,  0.0, 0.0, 0.0 },
		{ "subtract",    zinc.fm.createFieldSubtract(fielda, fieldb), binary_operator_subtract, 1.0E-12, 1.0E-7,  2.5E-2,  0.0, 0.0, 0.0 },
		{ "multiply",    zinc.fm.createFieldMultiply(fielda, fieldb), binary_operator_multiply, 1.0E-12, 1.0E-7,  2.0E-2,  0.0, 0.0, 0.0 },
		{ "divide",      zinc.fm.createFieldDivide(fielda, fieldb),   binary_operator_divide,   1.0E-12, 1.0E-7,  3.0E-2,  0.0, 0.0, 0.0 },
		{ "power",       zinc.fm.createFieldPower(fielda, fieldb),    binary_operator_power,    1.0E-12, 1.0E-5,  2.0E-2,  0.0, 0.0, 0.0 },
		{ "log",         zinc.fm.createFieldLog(fielda),              unary_operator_log,       1.0E-12, 1.0E-5,  3.0E-2,  0.0, 0.0, 0.0 },
		{ "sqrt",        zinc.fm.createFieldSqrt(fielda),             unary_operator_sqrt,      1.0E-12, 2.0E-7,  2.0E-2,  0.0, 0.0, 0.0 },
		{ "exp",         zinc.fm.createFieldExp(fielda),              unary_operator_exp,       1.0E-12, 2.0E-7,  7.0E-2,  0.0, 0.0, 0.0 },
		{ "abs",         zinc.fm.createFieldAbs(fieldb),              unary_operator_abs,       1.0E-12, 1.0E-10, 1.0E-5,  0.0, 0.0, 0.0 },
		// composite operators
		{ "identity",    zinc.fm.createFieldIdentity(fielda),         unary_operator_identity,  1.0E-12, 1.0E-7,  3.0E-2,  0.0, 0.0, 0.0 },
		{ "component",   zinc.fm.createFieldComponent(fielda, 3, component_indexes_reverse), unary_operator_reverse,
		                                                                                        1.0E-12, 1.0E-7,  3.0E-2,  0.0, 0.0, 0.0 },
		{ "concatenate", zinc.fm.createFieldConcatenate(3, concatenate_fields_a2b3b1), binary_operator_concatenate_a2b3b1,
		                                                                                        1.0E-12, 1.0E-7,  1.0E-2,  0.0, 0.0, 0.0 },
		// conditional operators
		{ "if",          zinc.fm.createFieldIf(zinc.fm.createFieldLessThan(fieldb, zinc.fm.createFieldConstant(3, zero3)), fielda, fieldb), binary_operator_if_b_lt_zero,
		                                                                                        1.0E-12, 5.0E-10, 4.0E-4,  0.0, 0.0, 0.0 },
		// constant operators
		{ "constant",    field_const_vector, constant_operator_m020507,                         1.0E-12, 1.0E-12, 1.0E-12, 0.0, 0.0, 0.0 },
		// coordinate transformation
		{ "coordinate_transformation_spherical_polar_to_rc", fieldCoordinateTransformation_spherical_polar_to_rc, unary_operator_coordinate_transformation_spherical_polar_to_rc,
		                                                                                        1.0E-12, 6.0E-8,  3.0E-2,  0.0, 0.0, 0.0 },
		// matrix operators
		{ "matrix_multiply_matrix_constvector", zinc.fm.createFieldMatrixMultiply(3, field_matrix_aba, field_const_vector), binary_operator_matrix_multiply_aba_m020507,
		                                                                                        1.0E-12, 1.0E-7,  3.0E-2,  0.0, 0.0, 0.0 },
		{ "matrix_multiply_constvector_matrix", zinc.fm.createFieldMatrixMultiply(1, field_const_vector, field_matrix_aba), binary_operator_matrix_multiply_m020507_aba,
		                                                                                        1.0E-12, 5.0E-8,  2.0E-2,  0.0, 0.0, 0.0 },
		{ "matrix_multiply_matrix_vector", zinc.fm.createFieldMatrixMultiply(3, field_matrix_aba, fieldb), binary_operator_matrix_multiply_aba_b,
		                                                                                        1.0E-12, 2.0E-7,  4.0E-2,  0.0, 0.0, 0.0 },
		{ "matrix_multiply_vector_matrix", zinc.fm.createFieldMatrixMultiply(1, fielda, field_matrix_aba), binary_operator_matrix_multiply_a_aba,
		                                                                                        1.0E-12, 3.0E-7,  1.0E-1,  0.0, 0.0, 0.0 },
		{ "transpose",   zinc.fm.createFieldComponent(zinc.fm.createFieldTranspose(3, field_matrix_aba), 3, component_indexes_matrix3x3_upper), binary_operator_transpose_aba_upper,
		                                                                                        1.0E-12, 2.0E-8,  9.0E-3,  0.0, 0.0, 0.0 },
		// trigonometry operators
		{ "sin",         zinc.fm.createFieldSin(fielda),              unary_operator_sin,       1.0E-12, 3.0E-8,  2.0E-2,  0.0, 0.0, 0.0 },
		{ "cos",         zinc.fm.createFieldCos(fielda),              unary_operator_cos,       1.0E-12, 6.0E-8,  3.0E-2,  0.0, 0.0, 0.0 },
		{ "tan",         zinc.fm.createFieldTan(fielda),              unary_operator_tan,       1.0E-12, 8.0E-7,  2.0E-1,  0.0, 0.0, 0.0 },
		{ "asin",        zinc.fm.createFieldAsin(fielda),             unary_operator_asin,      1.0E-12, 1.0E-8,  8.0E-4,  0.0, 0.0, 0.0 },
		{ "acos",        zinc.fm.createFieldAcos(fielda),             unary_operator_acos,      1.0E-12, 1.0E-8,  8.0E-4,  0.0, 0.0, 0.0 },
		{ "atan",        zinc.fm.createFieldAtan(fielda),             unary_operator_atan,      1.0E-12, 3.0E-8,  2.0E-2,  0.0, 0.0, 0.0 },
		{ "atan2",       zinc.fm.createFieldAtan2(fielda, fieldb),    binary_operator_atan2,    1.0E-12, 3.0E-8,  2.0E-2,  0.0, 0.0, 0.0 },
		// vector operators
		{ "cross_product", zinc.fm.createFieldCrossProduct(fielda, fieldb), binary_operator_cross_product,
		                                                                                        1.0E-12, 5.0E-8,  2.0E-2,  0.0, 0.0, 0.0 },
		{ "dot_product", zinc.fm.createFieldComponent(zinc.fm.createFieldDotProduct(fielda, fieldb), 3, component_indexes_111), binary_operator_dot_product,
		                                                                                        1.0E-12, 2.0E-7,  5.0E-2,  0.0, 0.0, 0.0 },
		{ "magnitude", zinc.fm.createFieldComponent(zinc.fm.createFieldMagnitude(fielda), 3, component_indexes_111), unary_operator_magnitude,
		                                                                                        1.0E-12, 1.0E-7,  4.0E-2,  0.0, 0.0, 0.0 },
		{ "normalise",   zinc.fm.createFieldNormalise(fielda),        unary_operator_normalise, 1.0E-12, 1.0E-7,  8.0E-3,  0.0, 0.0, 0.0 },
		{ "sum_components", zinc.fm.createFieldComponent(zinc.fm.createFieldSumComponents(fielda), 3, component_indexes_111), unary_operator_sum_components,
		                                                                                        1.0E-12, 2.0E-7,  6.0E-2,  0.0, 0.0, 0.0 }
	};
	// store values of d2 where max d2 error occurred for later check
	double max_d2_error_mag[field_binary_operator_count];
	for (int f = 0; f < field_binary_operator_count; ++f)
		max_d2_error_mag[f] = 0.0;

	Fieldcache fieldcache = zinc.fm.createFieldcache();
	EXPECT_TRUE(fieldcache.isValid());

	Mesh mesh = zinc.fm.findMeshByDimension(3);
	EXPECT_TRUE(mesh.isValid());
	Element element = mesh.findElementByIdentifier(1);
	EXPECT_TRUE(element.isValid());

	const int pointCount = 10;
	const double xi[pointCount][3] =
	{
		{ 0.00, 0.00, 0.00 },
		{ 0.00, 0.75, 0.25 },
		{ 0.50, 0.50, 0.50 },
		{ 0.20, 0.10, 0.40 },
		{ 0.75, 0.33, 0.45 },
		{ 0.95, 0.95, 0.95 },
		{ 1.00, 1.00, 1.00 },
		{ 2.0/3.0, 1.0/3.0, 1.0/3.0 },
		{ 1.0/3.0, 2.0/3.0, 1.0/3.0 },
		{ 1.0/3.0, 1.0/3.0, 2.0/3.0 }
	};
	const double half_delta_xi = 0.5E-5;
	const double delta_xi = 2.0*half_delta_xi;
	const double delta_xi_squared = delta_xi*delta_xi;
	const double delta_xi_offsets[3] = { -half_delta_xi, 0.0, half_delta_xi };
	// sample 27 locations +/- half_delta_xi from location to numerically calculate derivatives
	double samplea[27][3], sampleb[27][3], samplev[27][3], v[3], d1[3], d2[3], d1exp[3], d2exp[3];
	const int sample_centre = 13;
	const int sample_d1[3][2] = { { 12, 14 }, { 10, 16 }, { 4, 22 } };
	// mixed derivatives 1-2, 1-3, 2-3
	const int sample_mixed_d2[3][4] = { { 9, 11, 15, 17 }, { 3, 5, 21, 23 }, { 1, 7, 19, 25 } };
	// which case to use for derivative [d = xi index 1][e = xi index 2]; -1 is not used
	const int d2_ix[3][3] = { { -1, 0, 1 }, { 0, -1, 2 }, { 1, 2, -1 } };
	for (int p = 0; p < pointCount; ++p)
	{
		//std::cerr << p << ". xi " << xi[p][0] << ", " << xi[p][1] << ", " << xi[p][2] << "\n";
		for (int s = 0; s < 27; ++s)
		{
			double xi_s[3] = { xi[p][0] + delta_xi_offsets[s % 3], xi[p][1] + delta_xi_offsets[(s/3) % 3], xi[p][2] + delta_xi_offsets[s/9] };
			EXPECT_EQ(RESULT_OK, fieldcache.setMeshLocation(element, 3, xi_s));
			EXPECT_EQ(RESULT_OK, fielda.evaluateReal(fieldcache, 3, samplea[s]));
			EXPECT_EQ(RESULT_OK, fieldb.evaluateReal(fieldcache, 3, sampleb[s]));
		}
		const double *a = samplea[sample_centre];
		const double *b = sampleb[sample_centre];
		//std::cerr << "   a " << a[0] << ", " << a[1] << ", " << a[2] << "\n";
		//std::cerr << "   b " << b[0] << ", " << b[1] << ", " << b[2] << "\n";
		EXPECT_EQ(RESULT_OK, fieldcache.setMeshLocation(element, 3, xi[p]));
		for (int f = 0; f < field_binary_operator_count; ++f)
		{
			Field_binary_operator &op = field_binary_operators[f];
			const char *name = op.name;
			//std::cerr << "operator: " << name << "\n";
			if (0 == strcmp(name, "divide"))
			{
				// avoid b close to zero
				if ((fabs(b[0]) < 0.1) || (fabs(b[1]) < 0.1) || (fabs(b[2]) < 0.1))
					continue;
			}
			else if ((0 == strcmp(name, "power"))
				|| (0 == strcmp(name, "log"))
				|| (0 == strcmp(name, "sqrt")))
			{
				// operators which don't work with negative values of a
				if ((a[0] <= 0.0) || (a[1] < 0.0) || (a[2] < 0.0))
					continue;
			}
			else if ((0 == strcmp(name, "asin"))
				|| (0 == strcmp(name, "acos")))
			{
				// don't use a values outside [-0.999, +0.999]
				if ((a[0] < -0.999) || (a[0] > 0.999) ||
					(a[1] < -0.999) || (a[1] > 0.999) ||
					(a[2] < -0.999) || (a[2] > 0.999))
					continue;
			}
			else if (0 == strcmp(name, "normalise"))
			{
				// don't evaluate where magnitude is near zero
				if ((b[0]*b[0] + b[1]*b[1] + b[2]*b[2]) < 0.1)
					continue;
			}
			Field &v_field = op.field;
			Binary_operator binary_operator = op.binary_operator;
			const double v_tol = op.v_tol;
			const double d1_tol = op.d1_tol;
			const double d2_tol = op.d2_tol;

			// Evaluate operator at the 27 sample locations
			for (int s = 0; s < 27; ++s)
				binary_operator(3, samplea[s], sampleb[s], samplev[s]);
			// Test value against centre sample location
			EXPECT_EQ(RESULT_OK, v_field.evaluateReal(fieldcache, 3, v));
			const double *vexp = samplev[sample_centre];
			EXPECT_NEAR(vexp[0], v[0], v_tol);
			EXPECT_NEAR(vexp[1], v[1], v_tol);
			EXPECT_NEAR(vexp[2], v[2], v_tol);
			const double v_diff[3] = { (v[0] - vexp[0]), (v[1] - vexp[1]), (v[2] - vexp[2]) };
			const double v_error = sqrt(v_diff[0]*v_diff[0] + v_diff[1]*v_diff[1] + v_diff[2]*v_diff[2]);
			if (v_error > op.max_v_error)
				op.max_v_error = v_error;

			for (int d = 0; d < 3; ++d)
			{
				FieldDerivative d1_field = zinc.fm.createFieldDerivative(v_field, d + 1);
				EXPECT_TRUE(d1_field.isValid());
				EXPECT_EQ(RESULT_OK, d1_field.evaluateReal(fieldcache, 3, d1));
				// evaluate expected first derivatives by finite difference:
				for (int i = 0; i < 3; ++i)
					d1exp[i] = (samplev[sample_d1[d][1]][i] - samplev[sample_d1[d][0]][i])/delta_xi;
				EXPECT_NEAR(d1exp[0], d1[0], d1_tol);
				EXPECT_NEAR(d1exp[1], d1[1], d1_tol);
				EXPECT_NEAR(d1exp[2], d1[2], d1_tol);
				const double d1_diff[3] = { (d1[0] - d1exp[0]), (d1[1] - d1exp[1]), (d1[2] - d1exp[2]) };
				const double d1_error = sqrt(d1_diff[0]*d1_diff[0] + d1_diff[1]*d1_diff[1] + d1_diff[2]*d1_diff[2]);
				if (d1_error > op.max_d1_error)
					op.max_d1_error = d1_error;

				for (int e = 0; e < 3; ++e)
				{
					FieldDerivative d2_field = zinc.fm.createFieldDerivative(d1_field, e + 1);
					EXPECT_TRUE(d2_field.isValid());
					EXPECT_EQ(RESULT_OK, d2_field.evaluateReal(fieldcache, 3, d2));
					// evaluate expected second derivatives by finite difference:
					if (e == d)
					{
						for (int i = 0; i < 3; ++i)
							d2exp[i] = 4.0*(samplev[sample_d1[d][1]][i] - 2.0*samplev[sample_centre][i] + samplev[sample_d1[d][0]][i])/delta_xi_squared;
					}
					else
					{
						const int ix = d2_ix[d][e];
						for (int i = 0; i < 3; ++i)
							d2exp[i] = (samplev[sample_mixed_d2[ix][0]][i] - samplev[sample_mixed_d2[ix][1]][i] - samplev[sample_mixed_d2[ix][2]][i] + samplev[sample_mixed_d2[ix][3]][i])/delta_xi_squared;
					}
					EXPECT_NEAR(d2exp[0], d2[0], d2_tol);
					EXPECT_NEAR(d2exp[1], d2[1], d2_tol);
					EXPECT_NEAR(d2exp[2], d2[2], d2_tol);
					const double d2_diff[3] = { (d2[0] - d2exp[0]), (d2[1] - d2exp[1]), (d2[2] - d2exp[2]) };
					const double d2_error = sqrt(d2_diff[0]*d2_diff[0] + d2_diff[1]*d2_diff[1] + d2_diff[2]*d2_diff[2]);
					if (d2_error > op.max_d2_error)
					{
						op.max_d2_error = d2_error;
						max_d2_error_mag[f] = sqrt(d2[0]*d2[0] + d2[1]*d2[1] + d2[2]*d2[2]);
					}
				}
			}
		}
	}
	for (int f = 0; f < field_binary_operator_count; ++f)
	{
		const Field_binary_operator &op = field_binary_operators[f];
		std::cerr << f << ". " << op.name << "\n";
		EXPECT_LT(op.max_v_error, op.v_tol);
		EXPECT_LT(op.max_d1_error, op.d1_tol);
		EXPECT_LT(op.max_d2_error, op.d2_tol);
		const double d2_error_ratio = ((max_d2_error_mag[f] > 0.0) ? (op.max_d2_error / max_d2_error_mag[f]) : 0);
		if (0 == strcmp(op.name, "log"))
			EXPECT_LT(d2_error_ratio, 0.04);
		else if (0 == strcmp(op.name, "atan"))
			EXPECT_LT(d2_error_ratio, 0.03);
		else if (0 == strcmp(op.name, "atan2"))
			EXPECT_LT(d2_error_ratio, 0.02);
		else
			EXPECT_LT(d2_error_ratio, 0.01);

		std::string expectedClassName = "Field";
		Field field = op.field;
		if ((0 == strcmp(op.name, "transpose"))
			|| (0 == strcmp(op.name, "magnitude"))
			|| (0 == strcmp(op.name, "dot_product"))
			|| (0 == strcmp(op.name, "sum_components")))
		{
			// these fields are within an outer component field
			field = field.getSourceField(1);
		}
		if (strchr(op.name, '_'))
		{
			if (strstr(op.name, "coordinate_transformation"))
			{
				expectedClassName += "CoordinateTransformation";
			}
			else if (strstr(op.name, "matrix_multiply"))
			{
				expectedClassName += "MatrixMultiply";
			}
			else if (strstr(op.name, "cross_product"))
			{
				expectedClassName += "CrossProduct";
			}
			else if (strstr(op.name, "dot_product"))
			{
				expectedClassName += "DotProduct";
			}
			else if (strstr(op.name, "sum_components"))
			{
				expectedClassName += "SumComponents";
			}
		}
		else
		{
			expectedClassName += std::toupper(op.name[0]);
			expectedClassName += op.name + 1;
		}
		char *className = field.getClassName();
		EXPECT_STREQ(expectedClassName.c_str(), className);
		cmzn_deallocate(className);
		EXPECT_TRUE(field.hasClassName(expectedClassName.c_str()));
	}
}
