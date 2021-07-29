/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <opencmiss/zinc/field.hpp>
#include <opencmiss/zinc/fieldarithmeticoperators.hpp>
#include <opencmiss/zinc/fieldassignment.hpp>
#include <opencmiss/zinc/fieldcache.hpp>
#include <opencmiss/zinc/fieldcomposite.hpp>
#include <opencmiss/zinc/fieldconstant.hpp>
#include <opencmiss/zinc/fieldderivatives.hpp>
#include <opencmiss/zinc/fieldfiniteelement.hpp>
#include <opencmiss/zinc/fieldlogicaloperators.hpp>
#include <opencmiss/zinc/fieldmatrixoperators.hpp>
#include <opencmiss/zinc/fieldvectoroperators.hpp>

#include "zinctestsetupcpp.hpp"

#include "test_resources.h"

#include <limits>
#include <sstream>

// Class comparing double arrays that can be set to generate expected values from current values as C++ code
class CompareDoubleArrays
{
	bool generate;
	std::stringstream output;

public:
	CompareDoubleArrays() :
		generate(false)
	{
		//this->output.precision(std::numeric_limits<double>::max_digits10);
		this->output.precision(17);
	}

	~CompareDoubleArrays()
	{
		if (this->generate)
		{
			std::string s = this->output.str();
		}
	}

	void operator ()(const char *expected_name, const double *expected, const double *actual, int rows, int columns, const double tol)
	{
		const int vcount = rows*columns;
		if (this->generate)
		{
			// generate expected answers from actual values passed
			this->output << "\tconst double " << expected_name << "[" << vcount << "] = {\n";
			const int lastcolumn = columns - 1;
			const int lastvi = rows*columns - 1;
			int vi = 0;
			for (int row = 0; row < rows; ++row)
			{
				this->output << "\t\t";
				for (int column = 0; column < columns; ++column)
				{
					// "%22.15le%s"
					this->output << actual[vi];
					if (vi != lastvi)
					{
						if (column == lastcolumn)
							this->output << ",";
						else
							this->output << ", ";
					}
					++vi;
				}
				this->output << "\n";
			}
			this->output << "\t};\n";
		}
		else
		{
			// regular testing
			for (int vi = 0; vi < vcount; ++vi)
				EXPECT_NEAR(expected[vi], actual[vi], tol);
		}
	}

	void generateExpected()
	{
		this->generate = true;
	}
};

TEST(ZincField, field_operator_derivatives_3d)
{
	const double xi_coordinates[12] =
	{
		0.5, 0.5, 0.5,
		0.2, 0.1, 0.4,
		0.0, 0.6666666666666666, 0.3333333333333333,
		0.95, 0.95, 0.95
	};

	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(RESULT_OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDMODULE_CUBE_TRICUBIC_DEFORMED_RESOURCE)));

	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	Field deformed = zinc.fm.findFieldByName("deformed");
	EXPECT_TRUE(deformed.isValid());
	Field temperature = zinc.fm.findFieldByName("temperature");
	EXPECT_TRUE(temperature.isValid());

	Field sqrt_temperature = zinc.fm.createFieldSqrt(temperature);
	EXPECT_TRUE(sqrt_temperature.isValid());
	Field deformed_temperature_array[2] = { deformed, temperature };
	Field concatenate_deformed_temperature = zinc.fm.createFieldConcatenate(2, deformed_temperature_array);
	EXPECT_TRUE(concatenate_deformed_temperature.isValid());
	Field sum_components_deformed = zinc.fm.createFieldSumComponents(deformed);
	EXPECT_TRUE(sum_components_deformed.isValid());
	Field dot_product_coordinates_deformed = zinc.fm.createFieldDotProduct(coordinates, deformed);
	EXPECT_TRUE(dot_product_coordinates_deformed.isValid());

	Field add_coordinates_deformed = zinc.fm.createFieldAdd(coordinates, deformed);
	EXPECT_TRUE(add_coordinates_deformed.isValid());
	Field subtract_coordinates_deformed = zinc.fm.createFieldSubtract(coordinates, deformed);
	EXPECT_TRUE(subtract_coordinates_deformed.isValid());
	Field multiply_coordinates_deformed = zinc.fm.createFieldMultiply(coordinates, deformed);
	EXPECT_TRUE(multiply_coordinates_deformed.isValid());
	Field divide_coordinates_deformed = zinc.fm.createFieldDivide(coordinates, deformed);
	EXPECT_TRUE(divide_coordinates_deformed.isValid());

	Field add_deformed_temperature = zinc.fm.createFieldAdd(deformed, temperature);
	EXPECT_TRUE(add_deformed_temperature.isValid());
	Field subtract_deformed_temperature = zinc.fm.createFieldSubtract(deformed, temperature);
	EXPECT_TRUE(subtract_deformed_temperature.isValid());
	Field multiply_deformed_temperature = zinc.fm.createFieldMultiply(deformed, temperature);
	EXPECT_TRUE(multiply_deformed_temperature.isValid());
	Field divide_deformed_temperature = zinc.fm.createFieldDivide(deformed, temperature);
	EXPECT_TRUE(divide_deformed_temperature.isValid());

	// output values

	double coordinates_values[12];
	double deformed_values[12];
	double temperature_values[4];

	double sqrt_temperature_values[4];
	double concatenate_deformed_temperature_values[16];
	double sum_components_deformed_values[4];
	double dot_product_coordinates_deformed_values[4];

	double add_coordinates_deformed_values[12];
	double subtract_coordinates_deformed_values[12];
	double multiply_coordinates_deformed_values[12];
	double divide_coordinates_deformed_values[12];

	double add_deformed_temperature_values[12];
	double subtract_deformed_temperature_values[12];
	double multiply_deformed_temperature_values[12];
	double divide_deformed_temperature_values[12];

	// output first derivatives

	double coordinates_derivatives1[36];
	double deformed_derivatives1[36];
	double temperature_derivatives1[12];

	double sqrt_temperature_derivatives1[12];
	double concatenate_deformed_temperature_derivatives1[48];
	double sum_components_deformed_derivatives1[12];
	double dot_product_coordinates_deformed_derivatives1[12];

	double add_coordinates_deformed_derivatives1[36];
	double subtract_coordinates_deformed_derivatives1[36];
	double multiply_coordinates_deformed_derivatives1[36];
	double divide_coordinates_deformed_derivatives1[36];

	double add_deformed_temperature_derivatives1[36];
	double subtract_deformed_temperature_derivatives1[36];
	double multiply_deformed_temperature_derivatives1[36];
	double divide_deformed_temperature_derivatives1[36];

	Fieldcache cache = zinc.fm.createFieldcache();
	EXPECT_TRUE(cache.isValid());
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_TRUE(mesh3d.isValid());
	Differentialoperator d_dxi[3];
	for (int d = 0; d < 3; ++d)
	{
		d_dxi[d] = mesh3d.getChartDifferentialoperator(/*order*/1, d + 1);
		EXPECT_TRUE(d_dxi[d].isValid());
	}
	Element element = mesh3d.findElementByIdentifier(1);
	EXPECT_TRUE(element.isValid());
	for (int i = 0; i < 4; ++i)
	{
		EXPECT_EQ(RESULT_OK, cache.setMeshLocation(element, 3, xi_coordinates + i*3));
		EXPECT_EQ(RESULT_OK, coordinates.evaluateReal(cache, 3, coordinates_values + i*3));
		EXPECT_EQ(RESULT_OK, deformed.evaluateReal(cache, 3, deformed_values + i*3));
		EXPECT_EQ(RESULT_OK, temperature.evaluateReal(cache, 3, temperature_values + i));

		EXPECT_EQ(RESULT_OK, sqrt_temperature.evaluateReal(cache, 3, sqrt_temperature_values + i));
		EXPECT_EQ(RESULT_OK, concatenate_deformed_temperature.evaluateReal(cache, 4, concatenate_deformed_temperature_values + i*4));
		EXPECT_EQ(RESULT_OK, sum_components_deformed.evaluateReal(cache, 3, sum_components_deformed_values + i));
		EXPECT_EQ(RESULT_OK, dot_product_coordinates_deformed.evaluateReal(cache, 3, dot_product_coordinates_deformed_values + i));

		EXPECT_EQ(RESULT_OK, add_coordinates_deformed.evaluateReal(cache, 3, add_coordinates_deformed_values + i*3));
		EXPECT_EQ(RESULT_OK, subtract_coordinates_deformed.evaluateReal(cache, 3, subtract_coordinates_deformed_values + i*3));
		EXPECT_EQ(RESULT_OK, multiply_coordinates_deformed.evaluateReal(cache, 3, multiply_coordinates_deformed_values + i*3));
		EXPECT_EQ(RESULT_OK, divide_coordinates_deformed.evaluateReal(cache, 3, divide_coordinates_deformed_values + i*3));

		EXPECT_EQ(RESULT_OK, add_deformed_temperature.evaluateReal(cache, 3, add_deformed_temperature_values + i*3));
		EXPECT_EQ(RESULT_OK, subtract_deformed_temperature.evaluateReal(cache, 3, subtract_deformed_temperature_values + i*3));
		EXPECT_EQ(RESULT_OK, multiply_deformed_temperature.evaluateReal(cache, 3, multiply_deformed_temperature_values + i*3));
		EXPECT_EQ(RESULT_OK, divide_deformed_temperature.evaluateReal(cache, 3, divide_deformed_temperature_values + i*3));
		for (int d = 0; d < 3; ++d)
		{
			const int sdi = i*3 + d;
			const int vdi3 = sdi*3;
			const int vdi4 = sdi*4;
			EXPECT_EQ(RESULT_OK, coordinates.evaluateDerivative(d_dxi[d], cache, 3, coordinates_derivatives1 + vdi3));
			EXPECT_EQ(RESULT_OK, deformed.evaluateDerivative(d_dxi[d], cache, 3, deformed_derivatives1 + vdi3));
			EXPECT_EQ(RESULT_OK, temperature.evaluateDerivative(d_dxi[d], cache, 1, temperature_derivatives1 + sdi));

			EXPECT_EQ(RESULT_OK, sqrt_temperature.evaluateDerivative(d_dxi[d], cache, 1, sqrt_temperature_derivatives1 + sdi));
			EXPECT_EQ(RESULT_OK, concatenate_deformed_temperature.evaluateDerivative(d_dxi[d], cache, 4, concatenate_deformed_temperature_derivatives1 + vdi4));
			EXPECT_EQ(RESULT_OK, sum_components_deformed.evaluateDerivative(d_dxi[d], cache, 1, sum_components_deformed_derivatives1 + sdi));
			EXPECT_EQ(RESULT_OK, dot_product_coordinates_deformed.evaluateDerivative(d_dxi[d], cache, 1, dot_product_coordinates_deformed_derivatives1 + sdi));

			EXPECT_EQ(RESULT_OK, add_coordinates_deformed.evaluateDerivative(d_dxi[d], cache, 3, add_coordinates_deformed_derivatives1 + vdi3));
			EXPECT_EQ(RESULT_OK, subtract_coordinates_deformed.evaluateDerivative(d_dxi[d], cache, 3, subtract_coordinates_deformed_derivatives1 + vdi3));
			EXPECT_EQ(RESULT_OK, multiply_coordinates_deformed.evaluateDerivative(d_dxi[d], cache, 3, multiply_coordinates_deformed_derivatives1 + vdi3));
			EXPECT_EQ(RESULT_OK, divide_coordinates_deformed.evaluateDerivative(d_dxi[d], cache, 3, divide_coordinates_deformed_derivatives1 + vdi3));

			EXPECT_EQ(RESULT_OK, add_deformed_temperature.evaluateDerivative(d_dxi[d], cache, 3, add_deformed_temperature_derivatives1 + vdi3));
			EXPECT_EQ(RESULT_OK, subtract_deformed_temperature.evaluateDerivative(d_dxi[d], cache, 3, subtract_deformed_temperature_derivatives1 + vdi3));
			EXPECT_EQ(RESULT_OK, multiply_deformed_temperature.evaluateDerivative(d_dxi[d], cache, 3, multiply_deformed_temperature_derivatives1 + vdi3));
			EXPECT_EQ(RESULT_OK, divide_deformed_temperature.evaluateDerivative(d_dxi[d], cache, 3, divide_deformed_temperature_derivatives1 + vdi3));
		}
	}

	CompareDoubleArrays compare_double_array;
	// call following to generate expected values equal to current values in
	// C++ string for pasting into these tests. Inspect string in destructor
	//compare_double_array.generateExpected();

	// test values

	const double expected_coordinates_values[12] = {
		0.5, 0.5, 0.5,
		0.20000000000000001, 0.10000000000000001, 0.40000000000000002,
		0, 0.66666666666666663, 0.33333333333333331,
		0.94999999999999996, 0.94999999999999996, 0.94999999999999996
	};
	const double expected_deformed_values[12] = {
		0.50075510807820856, 0.51143970478732725, 0.51222453477511909,
		0.19679909263605694, 0.069074101569614604, 0.43796862245459939,
		-4.8572257327350599e-17, 0.66666666666666652, 0.33333333333333381,
		0.9899449877878439, 1.1942263666488913, 1.1009160292662443
	};
	const double expected_temperature_values[4] = {
		135.625,
		118.40000000000001,
		120.55555555555557,
		191.734375
	};
	const double expected_sqrt_temperature_values[4] = {
		11.645814698852115,
		10.881176406988354,
		10.979779394667069,
		13.846818226581874
	};
	const double expected_concatenate_deformed_temperature_values[16] = {
		0.50075510807820856, 0.51143970478732725, 0.51222453477511909, 135.625,
		0.19679909263605694, 0.069074101569614604, 0.43796862245459939, 118.40000000000001,
		-4.8572257327350599e-17, 0.66666666666666652, 0.33333333333333381, 120.55555555555557,
		0.9899449877878439, 1.1942263666488913, 1.1009160292662443, 191.734375
	};
	const double expected_sum_components_deformed_values[4] = {
		1.5244193476406549,
		0.7038418166602709,
		1.0000000000000004,
		3.2850873837029795
	};
	const double expected_dot_product_coordinates_deformed_values[4] = {
		0.76220967382032745,
		0.22145467766601262,
		0.55555555555555558,
		3.1208330145178307
	};
	const double expected_add_coordinates_deformed_values[12] = {
		1.0007551080782084, 1.0114397047873274, 1.0122245347751191,
		0.39679909263605695, 0.16907410156961461, 0.83796862245459947,
		-4.8572257327350599e-17, 1.333333333333333, 0.66666666666666718,
		1.9399449877878439, 2.1442263666488914, 2.0509160292662445
	};
	const double expected_subtract_coordinates_deformed_values[12] = {
		-0.00075510807820855508, -0.011439704787327254, -0.012224534775119089,
		0.003200907363943073, 0.030925898430385401, -0.037968622454599366,
		4.8572257327350599e-17, 1.1102230246251565e-16, -4.9960036108132044e-16,
		-0.039944987787843944, -0.2442263666488913, -0.15091602926624437
	};
	const double expected_multiply_coordinates_deformed_values[12] = {
		0.25037755403910428, 0.25571985239366363, 0.25611226738755954,
		0.03935981852721139, 0.0069074101569614604, 0.17518744898183977,
		-0, 0.44444444444444431, 0.11111111111111127,
		0.94044773839845164, 1.1345150483164466, 1.0458702278029322
	};
	const double expected_divide_coordinates_deformed_values[12] = {
		0.99849206115718625, 0.9776323490721468, 0.97613442163506281,
		1.0162648481813001, 1.4477206033467913, 0.91330743686202032,
		-0, 1.0000000000000002, 0.99999999999999856,
		0.95964928528290649, 0.79549407593954491, 0.8629177655203828
	};
	const double expected_add_deformed_temperature_values[12] = {
		136.12575510807821, 136.13643970478734, 136.13722453477513,
		118.59679909263606, 118.46907410156962, 118.83796862245461,
		120.55555555555557, 121.22222222222224, 120.8888888888889,
		192.72431998778785, 192.92860136664888, 192.83529102926624
	};
	const double expected_subtract_deformed_temperature_values[12] = {
		-135.12424489192179, -135.11356029521266, -135.11277546522487,
		-118.20320090736395, -118.33092589843039, -117.9620313775454,
		-120.55555555555557, -119.8888888888889, -120.22222222222224,
		-190.74443001221215, -190.54014863335112, -190.63345897073376
	};
	const double expected_multiply_deformed_temperature_values[12] = {
		67.914911533107031, 69.364009961781264, 69.470452528875526,
		23.301012568109144, 8.1783736258423687, 51.855484898624567,
		-5.8556554666861565e-15, 80.370370370370367, 40.185185185185247,
		189.80648351788489, 228.974246017946, 211.08344679884507
	};
	const double expected_divide_deformed_temperature_values[12] = {
		0.0036922035618669755, 0.003770983998431906, 0.0037767707633188504,
		0.0016621544986153457, 0.0005833961281217449, 0.0036990593112719542,
		-4.0290351700106484e-19, 0.0055299539170506895, 0.0027649769585253491,
		0.0051631064475936772, 0.0062285459592151451, 0.0057418813359171733
	};

	const double coordinatesTol = 1.0E-11;
	const double temperatureTol = 1.0E-8;

	compare_double_array("expected_coordinates_values", expected_coordinates_values, coordinates_values, 4, 3, coordinatesTol);
	compare_double_array("expected_deformed_values", expected_deformed_values, deformed_values, 4, 3, coordinatesTol);
	compare_double_array("expected_temperature_values", expected_temperature_values, temperature_values, 4, 1, temperatureTol);

	compare_double_array("expected_sqrt_temperature_values", expected_sqrt_temperature_values, sqrt_temperature_values, 4, 1, temperatureTol);
	compare_double_array("expected_concatenate_deformed_temperature_values", expected_concatenate_deformed_temperature_values, concatenate_deformed_temperature_values, 4, 4, temperatureTol);
	compare_double_array("expected_sum_components_deformed_values", expected_sum_components_deformed_values, sum_components_deformed_values, 4, 1, coordinatesTol);
	compare_double_array("expected_dot_product_coordinates_deformed_values", expected_dot_product_coordinates_deformed_values, dot_product_coordinates_deformed_values, 4, 1, coordinatesTol);

	compare_double_array("expected_add_coordinates_deformed_values", expected_add_coordinates_deformed_values, add_coordinates_deformed_values, 4, 3, coordinatesTol);
	compare_double_array("expected_subtract_coordinates_deformed_values", expected_subtract_coordinates_deformed_values, subtract_coordinates_deformed_values, 4, 3, coordinatesTol);
	compare_double_array("expected_multiply_coordinates_deformed_values", expected_multiply_coordinates_deformed_values, multiply_coordinates_deformed_values, 4, 3, coordinatesTol);
	compare_double_array("expected_divide_coordinates_deformed_values", expected_divide_coordinates_deformed_values, divide_coordinates_deformed_values, 4, 3, coordinatesTol);

	compare_double_array("expected_add_deformed_temperature_values", expected_add_deformed_temperature_values, add_deformed_temperature_values, 4, 3, temperatureTol);
	compare_double_array("expected_subtract_deformed_temperature_values", expected_subtract_deformed_temperature_values, subtract_deformed_temperature_values, 4, 3, temperatureTol);
	compare_double_array("expected_multiply_deformed_temperature_values", expected_multiply_deformed_temperature_values, multiply_deformed_temperature_values, 4, 3, temperatureTol);
	compare_double_array("expected_divide_deformed_temperature_values", expected_divide_deformed_temperature_values, divide_deformed_temperature_values, 4, 3, temperatureTol);

	// test first derivatives

	const double expected_coordinates_derivatives1[36] = {
		1, 0, 0,
		0, 1, 0,
		0, 0, 1,
		1, 0, 0,
		0, 1, 0,
		0, 0, 1,
		1, 0, 0,
		0, 1, 0,
		0, 0, 1,
		1, 0, 0,
		0, 1, 0,
		0, 0, 1
	};
	const double expected_deformed_derivatives1[36] = {
		0.98672330823272358, -0.029988442515503255, -0.00051961045601656153,
		-0.027462781513686307, 0.95987677062358756, 0.050851247621929385,
		0.011265646842190336, -0.010137710772936792, 0.97440239231229286,
		1.0167140787602942, -0.15641290898271462, 0.0031162901921704825,
		-0.037012192057350464, 1.4054007712187735, 0.26522609398614533,
		0.048148056924547558, 0.012715295491157864, 0.97529014507609724,
		0.99999999999999356, -5.2180482157382357e-15, -8.4376949871511897e-15,
		7.6327832942979512e-17, 0.99999999999999989, 1.3600232051658168e-15,
		-0.17027439632143562, 0.058406239818569308, 1.175854240588891,
		1.0752579343505602, 0.67598189301386924, 0.5040852550607724,
		0.036547019830294403, 2.3281274295070205, -0.0010666447992093708,
		0.23425799466940589, 0.13969383438757177, 1.9474508001911204
	};
	const double expected_temperature_derivatives1[12] = {
		21.25,
		21.25,
		51.25,
		15,
		16,
		38.5,
		18.888888888888886,
		13.333333333333332,
		41.666666666666664,
		42.0625,
		42.0625,
		76.5625
	};
	const double expected_sqrt_temperature_derivatives1[12] = {
		0.91234493032482022,
		0.91234493032482022,
		2.2003613025480959,
		0.6892637082129448,
		0.73521462209380783,
		1.7691101844132251,
		0.86016704935179789,
		0.60717674071891625,
		1.8974273147466132,
		1.5188507320494828,
		1.5188507320494828,
		2.7646242893917035
	};
	const double expected_concatenate_deformed_temperature_derivatives1[48] = {
		0.98672330823272358, -0.029988442515503255, -0.00051961045601656153, 21.25,
		-0.027462781513686307, 0.95987677062358756, 0.050851247621929385, 21.25,
		0.011265646842190336, -0.010137710772936792, 0.97440239231229286, 51.25,
		1.0167140787602942, -0.15641290898271462, 0.0031162901921704825, 15,
		-0.037012192057350464, 1.4054007712187735, 0.26522609398614533, 16,
		0.048148056924547558, 0.012715295491157864, 0.97529014507609724, 38.5,
		0.99999999999999356, -5.2180482157382357e-15, -8.4376949871511897e-15, 18.888888888888886,
		7.6327832942979512e-17, 0.99999999999999989, 1.3600232051658168e-15, 13.333333333333332,
		-0.17027439632143562, 0.058406239818569308, 1.175854240588891, 41.666666666666664,
		1.0752579343505602, 0.67598189301386924, 0.5040852550607724, 42.0625,
		0.036547019830294403, 2.3281274295070205, -0.0010666447992093708, 42.0625,
		0.23425799466940589, 0.13969383438757177, 1.9474508001911204, 76.5625
	};
	const double expected_sum_components_deformed_derivatives1[12] = {
		0.95621525526120377,
		0.98326523673183064,
		0.97553032838154641,
		0.86341745996975006,
		1.6336146731475683,
		1.0361534974918027,
		0.9999999999999799,
		1.0000000000000013,
		1.0639860840860247,
		2.2553250824252018,
		2.3636078045381055,
		2.321402629248098
	};
	const double expected_dot_product_coordinates_deformed_derivatives1[12] = {
		0.97886273570881044,
		1.0030723231532426,
		0.99998969896589229,
		0.38574713356671253,
		0.30830217787448,
		0.83898582141906353,
		-6.3398360635365707e-15,
		1.3333333333333335,
		0.76422224007534367,
		3.1325038160917851,
		3.4396537809600916,
		3.3062485270519373
	};
	const double expected_add_coordinates_deformed_derivatives1[36] = {
		1.9867233082327236, -0.029988442515503255, -0.00051961045601656153,
		-0.027462781513686307, 1.9598767706235876, 0.050851247621929385,
		0.011265646842190336, -0.010137710772936792, 1.9744023923122929,
		2.0167140787602942, -0.15641290898271462, 0.0031162901921704825,
		-0.037012192057350464, 2.4054007712187735, 0.26522609398614533,
		0.048148056924547558, 0.012715295491157864, 1.9752901450760971,
		1.9999999999999936, -5.2180482157382357e-15, -8.4376949871511897e-15,
		7.6327832942979512e-17, 2, 1.3600232051658168e-15,
		-0.17027439632143562, 0.058406239818569308, 2.1758542405888912,
		2.0752579343505602, 0.67598189301386924, 0.5040852550607724,
		0.036547019830294403, 3.3281274295070205, -0.0010666447992093708,
		0.23425799466940589, 0.13969383438757177, 2.9474508001911204
	};
	const double expected_subtract_coordinates_deformed_derivatives1[36] = {
		0.013276691767276416, 0.029988442515503255, 0.00051961045601656153,
		0.027462781513686307, 0.040123229376412439, -0.050851247621929385,
		-0.011265646842190336, 0.010137710772936792, 0.025597607687707136,
		-0.016714078760294182, 0.15641290898271462, -0.0031162901921704825,
		0.037012192057350464, -0.40540077121877349, -0.26522609398614533,
		-0.048148056924547558, -0.012715295491157864, 0.024709854923902763,
		6.4392935428259079e-15, 5.2180482157382357e-15, 8.4376949871511897e-15,
		-7.6327832942979512e-17, 1.1102230246251565e-16, -1.3600232051658168e-15,
		0.17027439632143562, -0.058406239818569308, -0.17585424058889099,
		-0.075257934350560163, -0.67598189301386924, -0.5040852550607724,
		-0.036547019830294403, -1.3281274295070205, 0.0010666447992093708,
		-0.23425799466940589, -0.13969383438757177, -0.94745080019112038
	};
	const double expected_multiply_coordinates_deformed_derivatives1[36] = {
		0.99411676219457035, -0.014994221257751628, -0.00025980522800828076,
		-0.013731390756843154, 0.99137809009912103, 0.025425623810964693,
		0.0056328234210951678, -0.0050688553864683961, 0.99942573093126552,
		0.40014190838811581, -0.015641290898271463, 0.001246516076868193,
		-0.0074024384114700931, 0.20961417869149196, 0.10609043759445813,
		0.009629611384909512, 0.0012715295491157865, 0.82808468048503836,
		-4.8572257327350599e-17, -3.478698810492157e-15, -2.8125649957170631e-15,
		0, 1.333333333333333, 4.5334106838860555e-16,
		-0, 0.038937493212379534, 0.72528474686296418,
		2.011440025420876, 0.64218279836317571, 0.47888099230773373,
		0.03471966883877968, 3.4059474246805608, -0.0010133125592489023,
		0.22254509493593558, 0.13270914266819317, 2.9509942894478085
	};
	const double expected_divide_coordinates_deformed_derivatives1[36] = {
		0.02948469208338924, 0.057323808118569318, 0.0009902096005259427,
		0.054760039141582575, 0.12043143568817832, -0.096906043769741948,
		-0.022463393291977652, 0.019378538475579427, 0.09537279264037192,
		-0.16895798882879903, 3.278250253251116, -0.0064984815395639694,
		0.19112989413818482, -14.978517692483704, -0.55308291888540939,
		-0.24863517969129548, -0.26649923548610416, 0.2494705141007855,
		-20587884010836552, 7.8270723236073568e-15, 2.5313084961453493e-14,
		-0, 0, -4.0800696154974377e-15,
		0, -0.087609359727853983, -0.52756272176666696,
		-0.032194221484476634, -0.45028279926851622, -0.39511108055963995,
		-0.035428556022830848, -0.713442276941195, 0.00083605535960008521,
		-0.22708889880705224, -0.093052390069415319, -0.6181124398882144
	};
	const double expected_add_deformed_temperature_derivatives1[36] = {
		22.236723308232722, 21.220011557484497, 21.249480389543983,
		21.222537218486313, 22.209876770623588, 21.300851247621928,
		51.26126564684219, 51.239862289227062, 52.224402392312292,
		16.016714078760295, 14.843587091017286, 15.003116290192171,
		15.96298780794265, 17.405400771218773, 16.265226093986147,
		38.548148056924546, 38.512715295491155, 39.475290145076094,
		19.888888888888879, 18.888888888888882, 18.888888888888879,
		13.333333333333332, 14.333333333333332, 13.333333333333334,
		41.496392270345225, 41.725072906485231, 42.842520907255555,
		43.13775793435056, 42.738481893013869, 42.566585255060772,
		42.099047019830294, 44.39062742950702, 42.061433355200791,
		76.796757994669406, 76.702193834387572, 78.50995080019112
	};
	const double expected_subtract_deformed_temperature_derivatives1[36] = {
		-20.263276691767278, -21.279988442515503, -21.250519610456017,
		-21.277462781513687, -20.290123229376412, -21.199148752378072,
		-51.23873435315781, -51.260137710772938, -50.275597607687708,
		-13.983285921239705, -15.156412908982714, -14.996883709807829,
		-16.03701219205735, -14.594599228781227, -15.734773906013855,
		-38.451851943075454, -38.487284704508845, -37.524709854923906,
		-17.888888888888893, -18.888888888888889, -18.888888888888893,
		-13.333333333333332, -12.333333333333332, -13.33333333333333,
		-41.836941062988103, -41.608260426848098, -40.490812426077774,
		-40.98724206564944, -41.386518106986131, -41.558414744939228,
		-42.025952980169706, -39.73437257049298, -42.063566644799209,
		-76.328242005330594, -76.422806165612428, -74.61504919980888
	};
	const double expected_multiply_deformed_temperature_derivatives1[36] = {
		144.46539472572508, 6.8009112105655749, 10.814299195874034,
		6.9164063038682269, 141.05138074255476, 17.781471822695451,
		27.191602641980253, 24.836357846770969, 158.40483186457959,
		123.33093331475969, -17.483176900009195, 6.9384980955719762,
		-1.233458057413384, 167.50463693741665, 38.410267487233199,
		13.277495006354624, 4.1648438965832533, 132.33614514151199,
		120.55555555555479, 12.592592592591959, 6.2962962962952878,
		8.554114207094524e-15, 129.44444444444446, 4.4444444444446152,
		-20.527524445417523, 34.818974467016403, 155.64465011543857,
		247.80346905532187, 179.84111231550006, 142.9577518068042,
		48.646881054100284, 496.61420416405412, 46.102768007087995,
		120.70797332419866, 118.21706622421031, 457.68214550859119
	};
	const double expected_divide_deformed_temperature_derivatives1[36] = {
		0.0066968772906400038, -0.00081195835931562217, -0.00059558333033395128,
		-0.00078099249550864181, 0.0064865869910186872, -0.00021681202653342811,
		-0.0013121458853713709, -0.0014997282263046782, 0.0057573669359793685,
		0.0083765351459549331, -0.001394964956964027, -0.00044231080639281099,
		-0.00053721844624320938, 0.011791101631493458, 0.0017402123733597473,
		-0.00013382509520391258, -8.2309589877781374e-05, 0.0070344287296632349,
		0.0082949308755759822, -0.00086644439253333067, -0.00043322219626671445,
		6.7769485578520115e-19, 0.0076833230690819491, -0.00030580390324703186,
		-0.0014124143473667466, -0.0014267986732966675, 0.0087979924477907933,
		0.0044753830365507029, 0.0021592042564322761, 0.0013694329531089887,
		-0.0009420644843764433, 0.01077605001762221, -0.0012652114598189579,
		-0.00083992420620707684, -0.0017585746719378712, 0.0078642028087554036
	};

	const double coordinatesDerivatives1Tol = 1.0E-11;
	const double temperatureDerivatives1Tol = 1.0E-8;

	compare_double_array("expected_coordinates_derivatives1", expected_coordinates_derivatives1, coordinates_derivatives1, 12, 3, coordinatesDerivatives1Tol);
	compare_double_array("expected_deformed_derivatives1", expected_deformed_derivatives1, deformed_derivatives1, 12, 3, coordinatesDerivatives1Tol);
	compare_double_array("expected_temperature_derivatives1", expected_temperature_derivatives1, temperature_derivatives1, 12, 1, temperatureDerivatives1Tol);

	compare_double_array("expected_sqrt_temperature_derivatives1", expected_sqrt_temperature_derivatives1, sqrt_temperature_derivatives1, 12, 1, temperatureDerivatives1Tol);
	compare_double_array("expected_concatenate_deformed_temperature_derivatives1", expected_concatenate_deformed_temperature_derivatives1, concatenate_deformed_temperature_derivatives1, 12, 4, temperatureDerivatives1Tol);
	compare_double_array("expected_sum_components_deformed_derivatives1", expected_sum_components_deformed_derivatives1, sum_components_deformed_derivatives1, 12, 1, coordinatesDerivatives1Tol);
	compare_double_array("expected_dot_product_coordinates_deformed_derivatives1", expected_dot_product_coordinates_deformed_derivatives1, dot_product_coordinates_deformed_derivatives1, 12, 1, coordinatesDerivatives1Tol);

	compare_double_array("expected_add_coordinates_deformed_derivatives1", expected_add_coordinates_deformed_derivatives1, add_coordinates_deformed_derivatives1, 12, 3, coordinatesDerivatives1Tol);
	compare_double_array("expected_subtract_coordinates_deformed_derivatives1", expected_subtract_coordinates_deformed_derivatives1, subtract_coordinates_deformed_derivatives1, 12, 3, coordinatesDerivatives1Tol);
	compare_double_array("expected_multiply_coordinates_deformed_derivatives1", expected_multiply_coordinates_deformed_derivatives1, multiply_coordinates_deformed_derivatives1, 12, 3, coordinatesDerivatives1Tol);
	compare_double_array("expected_divide_coordinates_deformed_derivatives1", expected_divide_coordinates_deformed_derivatives1, divide_coordinates_deformed_derivatives1, 12, 3, coordinatesDerivatives1Tol);

	compare_double_array("expected_add_deformed_temperature_derivatives1", expected_add_deformed_temperature_derivatives1, add_deformed_temperature_derivatives1, 12, 3, temperatureDerivatives1Tol);
	compare_double_array("expected_subtract_deformed_temperature_derivatives1", expected_subtract_deformed_temperature_derivatives1, subtract_deformed_temperature_derivatives1, 12, 3, temperatureDerivatives1Tol);
	compare_double_array("expected_multiply_deformed_temperature_derivatives1", expected_multiply_deformed_temperature_derivatives1, multiply_deformed_temperature_derivatives1, 12, 3, temperatureDerivatives1Tol);
	compare_double_array("expected_divide_deformed_temperature_derivatives1", expected_divide_deformed_temperature_derivatives1, divide_deformed_temperature_derivatives1, 12, 3, temperatureDerivatives1Tol);
}

void evaluateCubicLagrangeBasis(double xi, int derivative, double *basisValuesOut4)
{
	if (derivative == 0)
	{
		const double xi2 = xi*xi;
		const double xi3 = xi2*xi;
		basisValuesOut4[0] = 1.0 - 5.5*xi +  9.0*xi2 -  4.5*xi3;
		basisValuesOut4[1] =       9.0*xi - 22.5*xi2 + 13.5*xi3;
		basisValuesOut4[2] =      -4.5*xi + 18.0*xi2 - 13.5*xi3;
		basisValuesOut4[3] =       1.0*xi -  4.5*xi2 +  4.5*xi3;
	}
	else if (derivative == 1)
	{
		const double xi2 = xi*xi;
		basisValuesOut4[0] = -5.5 + 18.0*xi - 13.5*xi2;
		basisValuesOut4[1] =  9.0 - 45.0*xi + 40.5*xi2;
		basisValuesOut4[2] = -4.5 + 36.0*xi - 40.5*xi2;
		basisValuesOut4[3] =  1.0 -  9.0*xi + 13.5*xi2;
	}
	else if (derivative == 2)
	{
		basisValuesOut4[0] =  18.0 - 27.0*xi;
		basisValuesOut4[1] = -45.0 + 81.0*xi;
		basisValuesOut4[2] =  36.0 - 81.0*xi;
		basisValuesOut4[3] =  -9.0 + 27.0*xi;
	}
	else if (derivative == 3)
	{
		basisValuesOut4[0] = -27.0;
		basisValuesOut4[1] =  81.0;
		basisValuesOut4[2] = -81.0;
		basisValuesOut4[3] =  27.0;
	}
	else
	{
		basisValuesOut4[0] = 0.0;
		basisValuesOut4[1] = 0.0;
		basisValuesOut4[2] = 0.0;
		basisValuesOut4[3] = 0.0;
	}
}

// Interpolate nodal parameters for 3 components using tricubic Lagrange basis with 4 basis values in each xi direction.
void interpolateTricubicLagrange(const double nodeParameters[64][3],
	const double basisValues1[4], const double basisValues2[4], const double basisValues3[4], double valuesOut3[3])
{
	for (int c = 0; c < 3; ++c)
		valuesOut3[c] = 0.0;
	double wt;
	for (int n = 0; n < 64; ++n)
	{
		wt = basisValues1[n % 4]*basisValues2[(n/4) % 4]*basisValues3[n/16];
		for (int c = 0; c < 3; ++c)
			valuesOut3[c] += wt*nodeParameters[n][c];
	}
}

// Compare Zinc derivatives to basis derivatives calculated directly
TEST(ZincFieldDerivative, higher_derivatives)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(RESULT_OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDMODULE_CUBE_TRICUBIC_DEFORMED_RESOURCE)));

	FieldFiniteElement deformed = zinc.fm.findFieldByName("deformed").castFiniteElement();
	EXPECT_TRUE(deformed.isValid());

	Mesh mesh = zinc.fm.findMeshByDimension(3);
	Element element = mesh.findElementByIdentifier(1);
	EXPECT_TRUE(element.isValid());
	Elementfieldtemplate eft = element.getElementfieldtemplate(deformed, -1);
	EXPECT_TRUE(eft.isValid());
	EXPECT_EQ(64, eft.getNumberOfLocalNodes());
	Fieldcache cache = zinc.fm.createFieldcache();
	// get nodal parameters for deformed field to interpolate
	double nodeDeformed[64][3];
	for (int n = 0; n < 64; ++n)
	{
		Node node = element.getNode(eft, n + 1);
		EXPECT_TRUE(node.isValid());
;		EXPECT_EQ(RESULT_OK, cache.setNode(node));
		EXPECT_EQ(RESULT_OK, deformed.getNodeParameters(cache, -1, Node::VALUE_LABEL_VALUE, 1, 3, nodeDeformed[n]));
	}
	// get zinc first and second derivative fields
	EXPECT_EQ(RESULT_OK, zinc.fm.beginChange());
	FieldDerivative firstDerivatives[3], secondDerivatives[3][3];
	for (int i = 0; i < 3; ++i)
	{
		firstDerivatives[i] = zinc.fm.createFieldDerivative(deformed, i + 1);
		EXPECT_TRUE(firstDerivatives[i].isValid());
		for (int j = 0; j < 3; ++j)
		{
			secondDerivatives[i][j] = zinc.fm.createFieldDerivative(firstDerivatives[i], j + 1);
			EXPECT_TRUE(secondDerivatives[i][j].isValid());
		}
	}
	EXPECT_EQ(RESULT_OK, zinc.fm.endChange());
	// make first and second derivative operators to compare values match derivative fields
	// these test evaluating all terms at once
	Differentialoperator derivativeOperator1 = mesh.getChartDifferentialoperator(1, -1);
	EXPECT_TRUE(derivativeOperator1.isValid());
	Differentialoperator derivativeOperator2 = mesh.getChartDifferentialoperator(2, -1);
	EXPECT_TRUE(derivativeOperator2.isValid());

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
	const int derivativeCount = 3;
	double basisValues[3][derivativeCount][4];  // xi direction, derivative, nodes
	double expectedValues[3], values[3];
	double derivatives1[3], expectedDerivatives1[3];
	double derivatives2[3], expectedDerivatives2[3];
	double allDerivatives1[9], allDerivatives2[27];
	const double valuesTolerance = 1.0E-11;
	const double derivatives1Tolerance = 1.0E-11;
	const double derivatives2Tolerance = 1.0E-7;  // as currently computed by finite difference
	for (int p = 0; p < pointCount; ++p)
	{
		EXPECT_EQ(RESULT_OK, cache.setMeshLocation(element, 3, xi[p]));
		for (int i = 0; i < 3; ++i)
			for (int d = 0; d < 3; ++d)
				evaluateCubicLagrangeBasis(xi[p][i], d, basisValues[i][d]);
		// value
		interpolateTricubicLagrange(nodeDeformed, basisValues[0][0], basisValues[1][0], basisValues[2][0], expectedValues);
		EXPECT_EQ(RESULT_OK, result = deformed.evaluateReal(cache, 3, values));
		for (int c = 0; c < 3; ++c)
			EXPECT_NEAR(expectedValues[c], values[c], valuesTolerance);
		// first derivatives
		EXPECT_EQ(RESULT_OK, result = deformed.evaluateDerivative(derivativeOperator1, cache, 9, allDerivatives1));
		for (int i = 0; i < 3; ++i)
		{
			interpolateTricubicLagrange(nodeDeformed,
				basisValues[0][(i == 0) ? 1 : 0],
				basisValues[1][(i == 1) ? 1 : 0],
				basisValues[2][(i == 2) ? 1 : 0], expectedDerivatives1);
			EXPECT_EQ(RESULT_OK, result = firstDerivatives[i].evaluateReal(cache, 3, derivatives1));
			for (int c = 0; c < 3; ++c)
			{
				EXPECT_NEAR(expectedDerivatives1[c], derivatives1[c], derivatives1Tolerance);
				EXPECT_EQ(derivatives1[c], allDerivatives1[c*3 + i]);
			}
		}
		// second derivatives
		EXPECT_EQ(RESULT_OK, result = deformed.evaluateDerivative(derivativeOperator2, cache, 27, allDerivatives2));
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
			{
				interpolateTricubicLagrange(nodeDeformed,
					basisValues[0][((i == 0) ? 1 : 0) + ((j == 0) ? 1 : 0)],
					basisValues[1][((i == 1) ? 1 : 0) + ((j == 1) ? 1 : 0)],
					basisValues[2][((i == 2) ? 1 : 0) + ((j == 2) ? 1 : 0)], expectedDerivatives2);
				EXPECT_EQ(RESULT_OK, result = secondDerivatives[i][j].evaluateReal(cache, 3, derivatives2));
				for (int c = 0; c < 3; ++c)
				{
					EXPECT_NEAR(expectedDerivatives2[c], derivatives2[c], derivatives2Tolerance);
					EXPECT_EQ(derivatives2[c], allDerivatives2[c*9 + i*3 + j]);
				}
			}
	}
}


/** Test evaluation of gradient at nodes which uses a finite different approximation */
TEST(ZincFieldGradient, evaluateAtNodeFiniteDifference)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));

	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	// first scale coordinates so not unit cube
	const double scaleValues[3] = { 1.2, 2.3, 3.4 };
	Field scale = zinc.fm.createFieldConstant(3, scaleValues);
	Field scaleCoordinates = coordinates*scale;
	Fieldassignment fieldassignment = coordinates.createFieldassignment(scaleCoordinates);
	EXPECT_TRUE(fieldassignment.isValid());
	EXPECT_EQ(RESULT_OK, fieldassignment.assign());
	const double matrixValues[9] =
	{
		 1.1, -0.1,  0.5,
		 0.2,  0.7, -0.2,
		-0.3,  0.1, -1.6
	};
	Field matrix = zinc.fm.createFieldConstant(9, matrixValues);
	EXPECT_TRUE(matrix.isValid());
	Field transCoordinates = zinc.fm.createFieldMatrixMultiply(3, matrix, coordinates);
	EXPECT_TRUE(transCoordinates.isValid());
	Field dx_dX = zinc.fm.createFieldGradient(transCoordinates, coordinates);
	EXPECT_TRUE(dx_dX.isValid());
	// test directional variant
	EXPECT_TRUE(matrix.isValid());

	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodes.isValid());
	Fieldcache cache = zinc.fm.createFieldcache();
	EXPECT_TRUE(cache.isValid());
	double values[9];
	EXPECT_EQ(RESULT_OK, matrix.evaluateReal(cache, 9, values));
	for (int c = 0; c < 9; ++c)
		EXPECT_DOUBLE_EQ(matrixValues[c], values[c]);
	const double tolerance = 1.0E-6;
	for (int n = 0; n < 8; ++n)
	{
		Node node = nodes.findNodeByIdentifier(n + 1);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(RESULT_OK, cache.setNode(node));
		EXPECT_EQ(RESULT_OK, dx_dX.evaluateReal(cache, 9, values));
		for (int c = 0; c < 9; ++c)
			EXPECT_NEAR(matrixValues[c], values[c], tolerance);
	}
}
