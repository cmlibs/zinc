/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include "zinctestsetup.hpp"
#include <opencmiss/zinc/zincconfigure.h>
#include <opencmiss/zinc/core.h>
#include <opencmiss/zinc/field.h>
#include <opencmiss/zinc/fieldconditional.h>
#include <opencmiss/zinc/fieldconstant.h>
#include <opencmiss/zinc/fieldimage.h>
#include <opencmiss/zinc/stream.h>
#include <opencmiss/zinc/streamimage.h>

#include "zinctestsetupcpp.hpp"
#include <opencmiss/zinc/element.hpp>
#include <opencmiss/zinc/field.hpp>
#include <opencmiss/zinc/fieldarithmeticoperators.hpp>
#include <opencmiss/zinc/fieldcache.hpp>
#include <opencmiss/zinc/fieldcomposite.hpp>
#include <opencmiss/zinc/fieldconditional.hpp>
#include <opencmiss/zinc/fieldconstant.hpp>
#include <opencmiss/zinc/fieldimage.hpp>
#include <opencmiss/zinc/fieldlogicaloperators.hpp>
#include <opencmiss/zinc/fieldvectoroperators.hpp>
#include <opencmiss/zinc/result.hpp>
#include <opencmiss/zinc/stream.hpp>
#include <opencmiss/zinc/streamimage.hpp>

#include "test_resources.h"

TEST(cmzn_fieldmodule_create_image, invalid_args)
{
	ZincTestSetup zinc;

	cmzn_field_id f0 = cmzn_fieldmodule_create_field_image(0);
	EXPECT_EQ(0, f0);

	cmzn_field_id f1 = cmzn_fieldmodule_create_field_image_from_source(zinc.fm, 0);
	EXPECT_EQ(0, f1);

	cmzn_field_destroy(&f0);
	cmzn_field_destroy(&f1);
}

TEST(cmzn_fieldmodule_create_image, read_png)
{
	ZincTestSetup zinc;

	cmzn_field_id f1 = cmzn_fieldmodule_create_field_image(zinc.fm);
	EXPECT_NE(static_cast<cmzn_field_id>(0), f1);

	cmzn_field_image_id im = cmzn_field_cast_image(f1);
	cmzn_streaminformation_id si = cmzn_field_image_create_streaminformation_image(im);
	EXPECT_NE(static_cast<cmzn_streaminformation_id>(0), si);

    cmzn_streamresource_id sr = cmzn_streaminformation_create_streamresource_file(si, resourcePath("image-1.png").c_str());
	cmzn_streaminformation_image_id sii = cmzn_streaminformation_cast_image(si);
	EXPECT_EQ(CMZN_OK, cmzn_field_image_read(im, sii));
	cmzn_streaminformation_image_destroy(&sii);

	cmzn_streamresource_destroy(&sr);
	cmzn_streaminformation_destroy(&si);
	cmzn_field_image_destroy(&im);
	cmzn_field_destroy(&f1);
}

TEST(ZincFieldImage, read_png)
{
	ZincTestSetupCpp zinc;

	FieldImage im = zinc.fm.createFieldImage();
	EXPECT_TRUE(im.isValid());

	StreaminformationImage si = im.createStreaminformationImage();
	EXPECT_TRUE(si.isValid());

    std::string resource = resourcePath("image-1.png");
    Streamresource sr = si.createStreamresourceFile(resource.c_str());
	EXPECT_TRUE(sr.isValid());

	EXPECT_EQ(CMZN_OK, im.read(si));
}

TEST(ZincFieldImage, set_get_buffer)
{
	ZincTestSetupCpp zinc;

	FieldImage im = zinc.fm.createFieldImage();
	EXPECT_TRUE(im.isValid());

	EXPECT_EQ(FieldImage::PIXEL_FORMAT_RGBA, im.getPixelFormat());
	EXPECT_EQ(CMZN_OK, im.setPixelFormat(FieldImage::PIXEL_FORMAT_LUMINANCE));
	EXPECT_EQ(FieldImage::PIXEL_FORMAT_LUMINANCE, im.getPixelFormat());

	EXPECT_EQ(8, im.getNumberOfBitsPerComponent());
	EXPECT_EQ(CMZN_OK, im.setNumberOfBitsPerComponent(16));
	EXPECT_EQ(16, im.getNumberOfBitsPerComponent());

	int myArray[3] = {4,3,2};
	EXPECT_EQ(CMZN_OK, im.setSizeInPixels(3, myArray));

	unsigned int actualSize = 4 *3 * 2 * 2;
	unsigned char *buffer = new unsigned char[actualSize];
	for (unsigned int i = 0; i < actualSize; i++)
	{
		buffer[i] = i;
	}

	const void *returnedBuffer = 0;
	unsigned int length = 0;
	EXPECT_EQ(CMZN_OK, im.setBuffer((const void *)buffer, actualSize));
	EXPECT_EQ(CMZN_OK, im.getBuffer(&returnedBuffer, &length));
	const unsigned char *myBuffer = (const unsigned char *)returnedBuffer;
	for  (unsigned int i = 0; i < actualSize; i++)
	{
		EXPECT_EQ(buffer[i], myBuffer[i]);
	}
	EXPECT_EQ(actualSize, length);
	delete[] buffer;
}

// Issue 3707: Image filter, wrap and other modes should not be reset after
// reading an image file.
TEST(ZincFieldImage, issue_3707_keep_attributes_after_read_image)
{
	ZincTestSetupCpp zinc;
	int result;

	FieldImage im = zinc.fm.createFieldImage();
	EXPECT_TRUE(im.isValid());

	// Check default attributes and set to non-default

	FieldImage::CombineMode combineMode;
	EXPECT_EQ(FieldImage::COMBINE_MODE_DECAL, combineMode = im.getCombineMode());
	EXPECT_EQ(OK, result = im.setCombineMode(FieldImage::COMBINE_MODE_BLEND));
	EXPECT_EQ(FieldImage::COMBINE_MODE_BLEND, combineMode = im.getCombineMode());

	FieldImage::FilterMode filterMode;
	EXPECT_EQ(FieldImage::FILTER_MODE_NEAREST, filterMode = im.getFilterMode());
	EXPECT_EQ(OK, result = im.setFilterMode(FieldImage::FILTER_MODE_LINEAR));
	EXPECT_EQ(FieldImage::FILTER_MODE_LINEAR, filterMode = im.getFilterMode());

	FieldImage::HardwareCompressionMode hardwareCompressionMode;
	EXPECT_EQ(FieldImage::HARDWARE_COMPRESSION_MODE_UNCOMPRESSED, hardwareCompressionMode = im.getHardwareCompressionMode());
	EXPECT_EQ(OK, result = im.setHardwareCompressionMode(FieldImage::HARDWARE_COMPRESSION_MODE_AUTOMATIC));
	EXPECT_EQ(FieldImage::HARDWARE_COMPRESSION_MODE_AUTOMATIC, hardwareCompressionMode = im.getHardwareCompressionMode());

	FieldImage::WrapMode wrapMode;
	EXPECT_EQ(FieldImage::WRAP_MODE_REPEAT, wrapMode = im.getWrapMode());
	EXPECT_EQ(OK, result = im.setWrapMode(FieldImage::WRAP_MODE_CLAMP));
	EXPECT_EQ(FieldImage::WRAP_MODE_CLAMP, wrapMode = im.getWrapMode());

	double width, height, depth;
	ASSERT_DOUBLE_EQ(1.0, width = im.getTextureCoordinateWidth());
	EXPECT_EQ(OK, im.setTextureCoordinateWidth(5.5));
	ASSERT_DOUBLE_EQ(5.5, width = im.getTextureCoordinateWidth());
	ASSERT_DOUBLE_EQ(1.0, height = im.getTextureCoordinateHeight());
	EXPECT_EQ(OK, im.setTextureCoordinateHeight(6.7));
	ASSERT_DOUBLE_EQ(6.7, height = im.getTextureCoordinateHeight());
	ASSERT_DOUBLE_EQ(1.0, depth = im.getTextureCoordinateDepth());
	EXPECT_EQ(OK, im.setTextureCoordinateDepth(2.2));
	ASSERT_DOUBLE_EQ(2.2, depth = im.getTextureCoordinateDepth());

    EXPECT_EQ(OK, im.readFile(resourcePath("image-1.png").c_str()));

	// Check attributes kept their non-default values

	EXPECT_EQ(FieldImage::COMBINE_MODE_BLEND, combineMode = im.getCombineMode());
	EXPECT_EQ(FieldImage::FILTER_MODE_LINEAR, filterMode = im.getFilterMode());
	EXPECT_EQ(FieldImage::HARDWARE_COMPRESSION_MODE_AUTOMATIC, hardwareCompressionMode = im.getHardwareCompressionMode());
	EXPECT_EQ(FieldImage::WRAP_MODE_CLAMP, wrapMode = im.getWrapMode());
	ASSERT_DOUBLE_EQ(5.5, width = im.getTextureCoordinateWidth());
	ASSERT_DOUBLE_EQ(6.7, height = im.getTextureCoordinateHeight());
	ASSERT_DOUBLE_EQ(2.2, depth = im.getTextureCoordinateDepth());
}

TEST(cmzn_fieldmodule_create_image, analyze_bigendian)
{
	ZincTestSetup zinc;

	EXPECT_NE(static_cast<cmzn_fieldmodule *>(0), zinc.fm);

	cmzn_field_id f1 = cmzn_fieldmodule_create_field_image(zinc.fm);
	EXPECT_NE(static_cast<cmzn_field_id>(0), f1);

	cmzn_field_image_id im = cmzn_field_cast_image(f1);
	cmzn_streaminformation_id si = cmzn_field_image_create_streaminformation_image(im);
	EXPECT_NE(static_cast<cmzn_streaminformation_id>(0), si);
	cmzn_streaminformation_image_id sii = cmzn_streaminformation_cast_image(si);
	EXPECT_EQ(CMZN_OK, cmzn_streaminformation_image_set_file_format(sii, CMZN_STREAMINFORMATION_IMAGE_FILE_FORMAT_ANALYZE));

    cmzn_streamresource_id sr = cmzn_streaminformation_create_streamresource_file(si, resourcePath("bigendian.hdr").c_str());

	EXPECT_EQ(CMZN_OK, cmzn_field_image_read(im, sii));

	cmzn_streaminformation_image_destroy(&sii);
	cmzn_streamresource_destroy(&sr);
	cmzn_streaminformation_destroy(&si);
	cmzn_field_image_destroy(&im);
	cmzn_field_destroy(&f1);

}

TEST(cmzn_fieldmodule_create_image, analyze_lung)
{
	ZincTestSetup zinc;

	EXPECT_NE(static_cast<cmzn_fieldmodule *>(0), zinc.fm);

	cmzn_field_id f1 = cmzn_fieldmodule_create_field_image(zinc.fm);
	EXPECT_NE(static_cast<cmzn_field_id>(0), f1);

	cmzn_field_image_id im = cmzn_field_cast_image(f1);
	cmzn_streaminformation_id si = cmzn_field_image_create_streaminformation_image(im);
	EXPECT_NE(static_cast<cmzn_streaminformation_id>(0), si);
	cmzn_streaminformation_image_id sii = cmzn_streaminformation_cast_image(si);
	EXPECT_EQ(CMZN_OK, cmzn_streaminformation_image_set_file_format(sii, CMZN_STREAMINFORMATION_IMAGE_FILE_FORMAT_ANALYZE));

    cmzn_streamresource_id sr = cmzn_streaminformation_create_streamresource_file(si, resourcePath("littleendian.hdr").c_str());

	EXPECT_EQ(CMZN_OK, cmzn_field_image_read(im, sii));

	cmzn_streaminformation_image_destroy(&sii);
	cmzn_streamresource_destroy(&sr);
	cmzn_streaminformation_destroy(&si);
	cmzn_field_image_destroy(&im);
	cmzn_field_destroy(&f1);
}

#include <stdint.h>

namespace test
{

typedef ::size_t      BufferSizeType;

void SwapRange2(void *void_p, BufferSizeType n)
{
	uint16_t *p = (uint16_t *)void_p;
	uint16_t h1, h2;
	BufferSizeType i;

	for (i = 0; i < n; i++)
	{
		h1 = (*p) & 0xff;
		h2 = ((*p) >> 8) & 0xff;
		*p = (h1 << 8) | h2;

		p = p + 1;
	}
}

void SwapRange4(void *void_p, BufferSizeType n)
{
        unsigned char tmp[4], *buffer = (unsigned char *)void_p;
        unsigned int i, j;
        for(i = 0; i < n; i++)
        {
                for(j = 0; j < 4; ++j)
                {
                        tmp[j] = *(buffer + 4*i + j);
                }
                for(j = 0; j < 4; ++j)
                {
                        *(buffer + 4*i + j) = tmp[3 - j];
                }
        }
}

void SwapRange8(void *void_p, BufferSizeType n)
{
	uint64_t *p = (uint64_t *)void_p;
	uint64_t h1, h2, h3, h4, h5, h6, h7, h8;
	BufferSizeType i;

	for (i = 0; i < n; i++)
	{
		h1 = (*p) & 0xff;
		h2 = ((*p) >> 8) & 0xff;
		h3 = ((*p) >> 16) & 0xff;
		h4 = ((*p) >> 24) & 0xff;
		h5 = ((*p) >> 32) & 0xff;
		h6 = ((*p) >> 40) & 0xff;
		h7 = ((*p) >> 48) & 0xff;
		h8 = ((*p) >> 56) & 0xff;
		*p = (h1 << 56) | (h2 << 48) | (h3 << 40) | (h4 << 32) | (h5 << 24) | (h6 << 16) | (h7 << 8) | h8;

		p = p + 1;
	}

}

template <class myType>
void ByteSwap(myType *p)
{
	switch (sizeof(myType))
	{
	case 2:
		SwapRange2((void *)p, 1);
		break;
	case 4:
		SwapRange4((void *)p, 1);
		break;
	case 8:
		SwapRange8((void *)p, 1);
		break;
	default:
		printf("No can do!\n");
	}
}

template <class myType>
void ByteSwapRange(myType *p, BufferSizeType n)
{
	switch (sizeof(myType))
	{
	case 2:
		SwapRange2((void *)p, n);
		break;
	case 4:
		SwapRange4((void *)p, n);
		break;
	case 8:
		SwapRange8((void *)p, n);
		break;
	default:
		printf("No can do!\n");
	}
}

}

bool systemBigEndian()
{
	unsigned char swapTest[2] = { 1, 0 };
	if (*(short *) swapTest == 1 )
	{
		return true;
	}

	return false;
}

#include "../image_io/analyze_header.h"

TEST(ByteSwap, bigendian)
{
	struct dsr hdr;
    FILE *file = fopen(resourcePath("bigendian.hdr").c_str(), "rb");
	fread(&hdr, sizeof(hdr), 1, file);
    fclose(file);

	int sizeof_hdr = hdr.hk.sizeof_hdr;
	if (sizeof_hdr != 348)
		test::ByteSwap<int>(&hdr.hk.sizeof_hdr);
	EXPECT_EQ(348, hdr.hk.sizeof_hdr);
}

TEST(ByteSwap, littleendian)
{
	struct dsr hdr;
    FILE *file = fopen(resourcePath("littleendian.hdr").c_str(), "rb");
	fread(&hdr, sizeof(hdr), 1, file);
    fclose(file);

	EXPECT_EQ(348, hdr.hk.sizeof_hdr);
}

TEST(ByteSwap, size2)
{
	int16_t value = 16;
	test::ByteSwap<int16_t>(&value);
	EXPECT_EQ(4096, value);

	int16_t value1 = 0x1234;
	test::ByteSwap<int16_t>(&value1);
	EXPECT_EQ(0x3412, value1);

	int16_t values[] = {16, 1, 4096, 1};
	test::ByteSwapRange<int16_t>(values, 4);
	EXPECT_EQ(16, values[2]);
	EXPECT_EQ(256, values[1]);
}

TEST(ByteSwap, size4)
{
	int32_t value = 2048;
	test::ByteSwap<int32_t>(&value);
	EXPECT_EQ(524288, value);

	int32_t value1 = 0x12345678;
	test::ByteSwap<int32_t>(&value1);
	EXPECT_EQ(0x78563412, value1);

	int32_t values[] = {1, 256, 4096};
	test::ByteSwapRange<int32_t>(values, 3);
	EXPECT_EQ(1048576, values[2]);
	EXPECT_EQ(65536, values[1]);

	float fvalue = 0.12f;
	test::ByteSwap<float>(&fvalue);
	EXPECT_FLOAT_EQ(-1.92243393014834230422427828262E-29, fvalue);
}

union floatbitconvert
{
    float f;
    uint32_t u;
};

#ifdef STATIC_BUILD
float halffloat2float(uint16_t input);
#else
float halffloat2float(uint16_t input)
{
	floatbitconvert myresult, nan;
	myresult.u = nan.u = 0xFFC00000u;

	uint32_t source = input;
	uint16_t hs, he, hm;
	uint32_t xs, xe, xm;
	int32_t xes;
	int e;
	static int checkieee = 1;  // Flag to check for IEEE754, Endian, and word size
	float one = 1.0; // Used for checking IEEE754 floating point format, we are of course assuming that a float is 4 bytes!!
	uint32_t *ip = 0; // Used for checking IEEE754 floating point format

	if( checkieee ) { // 1st call, so check for IEEE754, Endian, and word size
		ip = (uint32_t *) &one;
		if (!systemBigEndian())
		{
			test::ByteSwap<uint32_t>(ip);
		}

		if( (*ip) != (uint32_t)0x3F800000u ) { // Check for exact IEEE 754 bit pattern of 1.0
			return nan.f;  // Floating point bit pattern is not IEEE 754
		}

		if (sizeof(float) != 4) { // float is not 4 bytes
			return nan.f;
		}

		checkieee = 0; // Everything checks out OK
	}

	if( (input & 0x7FFFu) == 0 ) {  // Signed zero
		myresult.u = source << 16;
	} else { // Not zero
		hs = source & 0x8000u;  // Pick off sign bit
		he = source & 0x7C00u;  // Pick off exponent bits
		hm = source & 0x03FFu;  // Pick off mantissa bits
		if( he == 0 ) {  // Denormal will convert to normalized
			e = -1; // The following loop figures out how much extra to adjust the exponent
			do {
				e++;
				hm <<= 1;
			} while( (hm & 0x0400u) == 0 ); // Shift until leading bit overflows into exponent bit
			xs = ((uint32_t) hs) << 16; // Sign bit
			xes = ((int32_t) (he >> 10)) - 15 + 127 - e; // Exponent unbias the halfp, then bias the single
			xe = (uint32_t) (xes << 23); // Exponent
			xm = ((uint32_t) (hm & 0x03FFu)) << 13; // Mantissa
			myresult.u = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
		} else if( he == 0x7C00u ) {  // Inf or NaN (all the exponent bits are set)
			if( hm == 0 ) { // If mantissa is zero ...
				myresult.u = (((uint32_t) hs) << 16) | ((uint32_t) 0x7F800000u); // Signed Inf
			} else {
				myresult.u = 0xFFC00000u; // NaN, only 1st mantissa bit set
			}
		} else { // Normalized number
			xs = ((uint32_t) hs) << 16; // Sign bit
			xes = ((int32_t) (he >> 10)) - 15 + 127; // Exponent unbias the halfp, then bias the single
			xe = (uint32_t) (xes << 23); // Exponent
			xm = ((uint32_t) hm) << 13; // Mantissa
			myresult.u = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
		}
	}

	return myresult.f;
}
#endif

TEST(FloatConversion, halfFloat)
{
	floatbitconvert inf, nan;
	inf.u = 0x7F800000u;
	nan.u = 0xFFC00000u;

	int16_t value = 0x0000u;
	float result;
	result = halffloat2float(value);
	EXPECT_EQ(0.0f, result);
	value = 0x8000u;
	result = halffloat2float(value);
	EXPECT_EQ(-0.0f, result);
	value = 0x3C00u;
	result = halffloat2float(value);
	EXPECT_EQ(1.0f, result);
	value = 0x003Cu;
	result = halffloat2float(value);
	EXPECT_NEAR(3.57628e-06, result, 1e-09);
	value = 0x113Cu;
	result = halffloat2float(value);
	EXPECT_NEAR(0.000638962f, result, 1e-09);
	value = 0x523Cu;
	result = halffloat2float(value);
	EXPECT_NEAR(49.875f, result, 1e-09);

	// inf and nan tests
	value = 0x7C00u;
	result = halffloat2float(value);
	EXPECT_EQ(inf.f, result);
	value = 0x7C01u;
	result = halffloat2float(value);
	nan.f = result;
	EXPECT_TRUE((nan.u & ~0xFFC00000u) == 0);
}

TEST(ZincFieldImageFromSource, evaluateImageFromNonImageSource)
{
	ZincTestSetupCpp zinc;
	int result;

	// load a single 2-D element mesh merely to define xi
    EXPECT_EQ(OK, result = zinc.root_region.readFile(resourcePath("fieldmodule/plate_600x300.exfile").c_str()));
	Field plate_coordinates = zinc.fm.findFieldByName("plate_coordinates");
	EXPECT_TRUE(plate_coordinates.isValid());
	const double offsetConst[3] = { 300.0, 150.0, -350.0 };
	FieldConstant offset = zinc.fm.createFieldConstant(3, offsetConst);
	EXPECT_TRUE(offset.isValid());
	FieldAdd offset_plate_coordinates = plate_coordinates + offset;
	EXPECT_TRUE(offset_plate_coordinates.isValid());
	const int components[2] = { 1, 2 };
	FieldComponent tex_coordinates = zinc.fm.createFieldComponent(offset_plate_coordinates, 2, components);
	EXPECT_TRUE(tex_coordinates.isValid());

	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_TRUE(mesh2d.isValid());
	Element element = mesh2d.findElementByIdentifier(10001);
	EXPECT_TRUE(element.isValid());

	Field xi = zinc.fm.findFieldByName("xi");
	const double redConst[3] = { 1.0, 0.0, 0.0 };
	const double pinkConst[3] = { 0.9, 0.6, 0.6 };
	const double purpleConst[3] = { 0.5, 0.1, 0.7 };
	Field red = zinc.fm.createFieldConstant(3, redConst);
	Field pink = zinc.fm.createFieldConstant(3, pinkConst);
	Field purple = zinc.fm.createFieldConstant(3, purpleConst);
	Field ximag = zinc.fm.createFieldMagnitude(xi);
	Field xi1 = zinc.fm.createFieldComponent(xi, 1);
	Field xi2 = zinc.fm.createFieldComponent(xi, 2);
	const double maglimitConst = 0.9;
	Field maglimit = zinc.fm.createFieldConstant(1, &maglimitConst);
	Field ximag_gt_limit = zinc.fm.createFieldGreaterThan(ximag, maglimit);
	Field magcolour = zinc.fm.createFieldIf(ximag_gt_limit, red, pink);
	const double xilimitConst = 0.5;
	Field xilimit = zinc.fm.createFieldConstant(1, &xilimitConst);
	Field xi1_lt_limit = zinc.fm.createFieldLessThan(xi1, xilimit);
	Field xi2_lt_limit = zinc.fm.createFieldLessThan(xi2, xilimit);
	Field xi_inside = zinc.fm.createFieldAnd(xi1_lt_limit, xi2_lt_limit);
	Field colour = zinc.fm.createFieldIf(xi_inside, purple, magcolour);
	EXPECT_TRUE(colour.isValid());

	FieldImage im1 = zinc.fm.createFieldImageFromSource(colour);
	EXPECT_TRUE(im1.isValid());
	EXPECT_EQ(RESULT_OK, im1.setDomainField(xi));
	EXPECT_EQ(3, im1.getNumberOfComponents());
	int sizeOut[3];
	EXPECT_EQ(1, im1.getSizeInPixels(3, sizeOut));
	EXPECT_EQ(1, sizeOut[0]);
	EXPECT_EQ(1, sizeOut[1]);
	EXPECT_EQ(1, sizeOut[2]);
	const int sizeIn[2] = { 200, 100 };
	EXPECT_EQ(RESULT_OK, im1.setSizeInPixels(2, sizeIn));
	EXPECT_EQ(2, im1.getSizeInPixels(3, sizeOut));
	EXPECT_EQ(sizeIn[0], sizeOut[0]);
	EXPECT_EQ(sizeIn[1], sizeOut[1]);
	EXPECT_EQ(1, sizeOut[2]);
	EXPECT_EQ(FieldImage::FILTER_MODE_NEAREST, im1.getFilterMode());

	const double xiConst[4][3] = {
		{ 0.25, 0.25, 0.0 },
		{ 0.75, 0.25, 0.0 },
		{ 0.25, 0.75, 0.0 },
		{ 0.75, 0.75, 0.0 },
	};
	const double expectedColourOut[4][3] = {
		{ 0.5, 0.1, 0.7 },
		{ 0.9, 0.6, 0.6 },
		{ 0.9, 0.6, 0.6 },
		{ 1.0, 0.0, 0.0 },
	};
	double colourOut[4][3];
	Fieldcache cache = zinc.fm.createFieldcache();
	const double colourTol = 0.0025;
	for (int i = 0; i < 4; ++i)
	{
		EXPECT_EQ(RESULT_OK, cache.setFieldReal(xi, 3, xiConst[i]));
		EXPECT_EQ(RESULT_OK, im1.evaluateReal(cache, 3, colourOut[i]));
		EXPECT_NEAR(expectedColourOut[i][0], colourOut[i][0], colourTol);
		EXPECT_NEAR(expectedColourOut[i][1], colourOut[i][1], colourTol);
		EXPECT_NEAR(expectedColourOut[i][2], colourOut[i][2], colourTol);
	}

	FieldImage im2 = zinc.fm.createFieldImageFromSource(im1);
	EXPECT_TRUE(im2.isValid());
	EXPECT_EQ(xi, im2.getDomainField());
	EXPECT_EQ(3, im2.getNumberOfComponents());
	EXPECT_EQ(2, im2.getSizeInPixels(3, sizeOut));
	EXPECT_EQ(sizeIn[0], sizeOut[0]);
	EXPECT_EQ(sizeIn[1], sizeOut[1]);
	EXPECT_EQ(1, sizeOut[2]);
	EXPECT_EQ(FieldImage::FILTER_MODE_NEAREST, im2.getFilterMode()); // should match source
	EXPECT_EQ(RESULT_OK, im2.setFilterMode(FieldImage::FILTER_MODE_LINEAR));
	EXPECT_EQ(FieldImage::FILTER_MODE_LINEAR, im2.getFilterMode());

	for (int i = 0; i < 4; ++i)
	{
		EXPECT_EQ(RESULT_OK, cache.setFieldReal(xi, 3, xiConst[i]));
		EXPECT_EQ(RESULT_OK, im2.evaluateReal(cache, 3, colourOut[i]));
		EXPECT_NEAR(expectedColourOut[i][0], colourOut[i][0], colourTol);
		EXPECT_NEAR(expectedColourOut[i][1], colourOut[i][1], colourTol);
		EXPECT_NEAR(expectedColourOut[i][2], colourOut[i][2], colourTol);
	}

	// test changing resolution of first source image changes that of second
	const int sizeIn2[2] = { 300, 400 };
	EXPECT_EQ(RESULT_OK, im1.setSizeInPixels(2, sizeIn2));
	EXPECT_EQ(2, im2.getSizeInPixels(3, sizeOut));
	EXPECT_EQ(sizeIn2[0], sizeOut[0]);
	EXPECT_EQ(sizeIn2[1], sizeOut[1]);
	EXPECT_EQ(1, sizeOut[2]);

	for (int i = 0; i < 4; ++i)
	{
		EXPECT_EQ(RESULT_OK, cache.setFieldReal(xi, 3, xiConst[i]));
		EXPECT_EQ(RESULT_OK, im2.evaluateReal(cache, 3, colourOut[i]));
		EXPECT_NEAR(expectedColourOut[i][0], colourOut[i][0], colourTol);
		EXPECT_NEAR(expectedColourOut[i][1], colourOut[i][1], colourTol);
		EXPECT_NEAR(expectedColourOut[i][2], colourOut[i][2], colourTol);
	}

	// test that image copied with change of texture coordinates
	FieldImage im3 = zinc.fm.createFieldImageFromSource(im2);
	EXPECT_TRUE(im3.isValid());
	EXPECT_EQ(xi, im3.getDomainField());
	EXPECT_EQ(RESULT_OK, im3.setDomainField(tex_coordinates));
	EXPECT_EQ(tex_coordinates, im3.getDomainField());
	const double texCoordSizes[3] = { 600.0, 300.0, 1.0 };
	EXPECT_EQ(RESULT_OK, im3.setTextureCoordinateSizes(3, texCoordSizes));
	double texCoordSizesOut[3];
	EXPECT_EQ(RESULT_OK, im3.getTextureCoordinateSizes(3, texCoordSizesOut));
	EXPECT_EQ(texCoordSizes[0], texCoordSizes[0]);
	EXPECT_EQ(texCoordSizes[1], texCoordSizes[1]);
	EXPECT_EQ(texCoordSizes[2], texCoordSizes[2]);
	EXPECT_EQ(3, im3.getNumberOfComponents());
	EXPECT_EQ(2, im3.getSizeInPixels(3, sizeOut));
	EXPECT_EQ(sizeIn2[0], sizeOut[0]);
	EXPECT_EQ(sizeIn2[1], sizeOut[1]);
	EXPECT_EQ(1, sizeOut[2]);
	EXPECT_EQ(FieldImage::FILTER_MODE_LINEAR, im3.getFilterMode()); // should match source

	// both the following locations setting methods work:
	for (int i = 0; i < 4; ++i)
	{
		double xiScaled[2] = { xiConst[i][0]*texCoordSizes[0], xiConst[i][1]*texCoordSizes[1] };
		EXPECT_EQ(RESULT_OK, cache.setFieldReal(tex_coordinates, 2, xiScaled));
		EXPECT_EQ(RESULT_OK, im3.evaluateReal(cache, 3, colourOut[i]));
		EXPECT_NEAR(expectedColourOut[i][0], colourOut[i][0], colourTol);
		EXPECT_NEAR(expectedColourOut[i][1], colourOut[i][1], colourTol);
		EXPECT_NEAR(expectedColourOut[i][2], colourOut[i][2], colourTol);
	}
	for (int i = 0; i < 4; ++i)
	{
		EXPECT_EQ(RESULT_OK, cache.setMeshLocation(element, 2, xiConst[i]));
		EXPECT_EQ(RESULT_OK, im3.evaluateReal(cache, 3, colourOut[i]));
		EXPECT_NEAR(expectedColourOut[i][0], colourOut[i][0], colourTol);
		EXPECT_NEAR(expectedColourOut[i][1], colourOut[i][1], colourTol);
		EXPECT_NEAR(expectedColourOut[i][2], colourOut[i][2], colourTol);
	}

	//StreaminformationImage sii = im3.createStreaminformationImage();
	//sii.createStreamresourceFile("image_from_non_image_source.png");
	//result = im3.write(sii);
	//EXPECT_EQ(RESULT_OK, result);
}
