/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cmath>
#include <gtest/gtest.h>

#include <opencmiss/zinc/field.hpp>
#include <opencmiss/zinc/fieldcache.hpp>
#include <opencmiss/zinc/fieldmatrixoperators.hpp>
#include <opencmiss/zinc/fieldconstant.hpp>

#include "zinctestsetupcpp.hpp"


TEST(ZincFieldEigenvaluesEigenvectors, invalidArguments)
{
	ZincTestSetupCpp zinc;

	// test can't create eigenvalues field for non-square matrix
	const double aValues[8] = { -2.0, -4.0, 2.0, -2.0, 1.0, 2.0, 4.0, 2.0 };
	FieldConstant a = zinc.fm.createFieldConstant(8, aValues);
	EXPECT_TRUE(a.isValid());
	FieldEigenvalues eigenvalues = zinc.fm.createFieldEigenvalues(a);
	EXPECT_FALSE(eigenvalues.isValid());

	// test can't create eigenvectors field from a non-eigenvalues field
	FieldEigenvectors eigenvectors = zinc.fm.createFieldEigenvectors(a);
	EXPECT_FALSE(eigenvectors.isValid());
}

TEST(ZincFieldEigenvaluesEigenvectors, evaluate)
{
	ZincTestSetupCpp zinc;

	const double aValues[9] = { 3.0, 1.0, 1.0, 1.0, 2.0, 2.0, 1.0, 2.0, 2.0 };
	FieldConstant a = zinc.fm.createFieldConstant(9, aValues);
	EXPECT_TRUE(a.isValid());
	FieldEigenvalues eigenvalues = zinc.fm.createFieldEigenvalues(a);
	EXPECT_TRUE(eigenvalues.isValid());
	FieldEigenvectors eigenvectors = zinc.fm.createFieldEigenvectors(eigenvalues);
	EXPECT_TRUE(eigenvectors.isValid());

	double eigenvaluesOut[3], eigenvectorsOut[9];
	const double expectedEigenvalues[3] = { 5.0, 2.0, 0.0 };
	const double one_root2 = 1.0 / sqrt(2.0);
	const double one_root3 = 1.0 / sqrt(3.0);
	const double one_root6 = 1.0 / sqrt(6.0);
	const double expectedEigenvectors[9] = {
		one_root3, one_root3, one_root3,
		-2.0*one_root6, one_root6, one_root6,
		0.0, -one_root2, one_root2
	};
	const double TOL = 1.0E-8;

	Fieldcache fieldcache = zinc.fm.createFieldcache();
	EXPECT_EQ(RESULT_OK, eigenvalues.evaluateReal(fieldcache, 3, eigenvaluesOut));
	for (int i = 0; i < 3; ++i)
	{
		EXPECT_NEAR(expectedEigenvalues[i], eigenvaluesOut[i], TOL);
	}
	EXPECT_EQ(RESULT_OK, eigenvectors.evaluateReal(fieldcache, 9, eigenvectorsOut));
	for (int i = 0; i < 9; ++i)
	{
		EXPECT_NEAR(expectedEigenvectors[i], eigenvectorsOut[i], TOL);
	}
}

TEST(ZincFieldTranspose, invalidArguments)
{
	ZincTestSetupCpp zinc;

	const double aValues[8] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0 };
	FieldConstant a = zinc.fm.createFieldConstant(8, aValues);
	EXPECT_TRUE(a.isValid());

	FieldTranspose transpose;
	Field noField;
	transpose = zinc.fm.createFieldTranspose(2, noField);
	EXPECT_FALSE(transpose.isValid());
	transpose = zinc.fm.createFieldTranspose(-1, a);
	EXPECT_FALSE(transpose.isValid());
	transpose = zinc.fm.createFieldTranspose(0, a);
	EXPECT_FALSE(transpose.isValid());
	transpose = zinc.fm.createFieldTranspose(9, a);
	EXPECT_FALSE(transpose.isValid());
	transpose = zinc.fm.createFieldTranspose(3, a);
	EXPECT_FALSE(transpose.isValid());

	transpose = zinc.fm.createFieldTranspose(2, a);
	EXPECT_TRUE(transpose.isValid());
	FieldTranspose invalidTranspose;
	EXPECT_EQ(0, invalidTranspose.getSourceNumberOfRows());
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, transpose.setSourceNumberOfRows(-1));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, transpose.setSourceNumberOfRows(0));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, transpose.setSourceNumberOfRows(0));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, transpose.setSourceNumberOfRows(3));
}

TEST(ZincFieldTranspose, evaluate)
{
	ZincTestSetupCpp zinc;

	const double aValues[8] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0 };
	FieldConstant a = zinc.fm.createFieldConstant(8, aValues);
	EXPECT_TRUE(a.isValid());

	FieldTranspose transpose = zinc.fm.createFieldTranspose(2, a);
	EXPECT_TRUE(transpose.isValid());

	Fieldcache fieldcache = zinc.fm.createFieldcache();
	double valuesOut[8];

	EXPECT_EQ(2, transpose.getSourceNumberOfRows());
	EXPECT_EQ(RESULT_OK, transpose.evaluateReal(fieldcache, 8, valuesOut));
	for (int i = 0; i < 8; ++i)
	{
		EXPECT_DOUBLE_EQ(aValues[(i % 2) * 4 + (i / 2)], valuesOut[i]);
	}

	EXPECT_EQ(RESULT_OK, transpose.setSourceNumberOfRows(4));
	EXPECT_EQ(4, transpose.getSourceNumberOfRows());
	EXPECT_EQ(RESULT_OK, transpose.evaluateReal(fieldcache, 8, valuesOut));
	for (int i = 0; i < 8; ++i)
	{
		EXPECT_DOUBLE_EQ(aValues[(i % 4) * 2 + (i / 4)], valuesOut[i]);
	}
}
