/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include "zinctestsetup.hpp"
#include <zinc/zincconfigure.h>
#include <zinc/core.h>
#include <zinc/field.h>
#include <zinc/fieldconditional.h>
#include <zinc/fieldconstant.h>
#include <zinc/fieldimage.h>
#include <zinc/stream.h>
#include <zinc/streamimage.h>

#include "zinctestsetupcpp.hpp"
#include <zinc/fieldimage.hpp>
#include <zinc/stream.hpp>
#include <zinc/streamimage.hpp>

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
	cmzn_streaminformation_id si = cmzn_field_image_create_streaminformation(im);
	EXPECT_NE(static_cast<cmzn_streaminformation_id>(0), si);

	cmzn_streamresource_id sr = cmzn_streaminformation_create_streamresource_file(si, TestResources::getLocation(TestResources::IMAGE_PNG_RESOURCE));
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

	StreaminformationImage si = im.createStreaminformation();
	EXPECT_TRUE(si.isValid());

	Streamresource sr = si.createStreamresourceFile(TestResources::getLocation(TestResources::IMAGE_PNG_RESOURCE));
	EXPECT_TRUE(sr.isValid());

	EXPECT_EQ(OK, im.read(si));
}

TEST(cmzn_fieldmodule_create_image, analyze_bigendian)
{
	ZincTestSetup zinc;

	EXPECT_NE(static_cast<cmzn_fieldmodule *>(0), zinc.fm);

	cmzn_field_id f1 = cmzn_fieldmodule_create_field_image(zinc.fm);
	EXPECT_NE(static_cast<cmzn_field_id>(0), f1);

	cmzn_field_image_id im = cmzn_field_cast_image(f1);
	cmzn_streaminformation_id si = cmzn_field_image_create_streaminformation(im);
	EXPECT_NE(static_cast<cmzn_streaminformation_id>(0), si);
	cmzn_streaminformation_image_id sii = cmzn_streaminformation_cast_image(si);
	EXPECT_EQ(CMZN_OK, cmzn_streaminformation_image_set_file_format(sii, CMZN_STREAMINFORMATION_IMAGE_FILE_FORMAT_ANALYZE));

	cmzn_streamresource_id sr = cmzn_streaminformation_create_streamresource_file(si, TestResources::getLocation(TestResources::IMAGE_ANALYZE_BIGENDIAN_RESOURCE));

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
	cmzn_streaminformation_id si = cmzn_field_image_create_streaminformation(im);
	EXPECT_NE(static_cast<cmzn_streaminformation_id>(0), si);
	cmzn_streaminformation_image_id sii = cmzn_streaminformation_cast_image(si);
	EXPECT_EQ(CMZN_OK, cmzn_streaminformation_image_set_file_format(sii, CMZN_STREAMINFORMATION_IMAGE_FILE_FORMAT_ANALYZE));

	cmzn_streamresource_id sr = cmzn_streaminformation_create_streamresource_file(si, TestResources::getLocation(TestResources::IMAGE_ANALYZE_LITTLEENDIAN_RESOURCE));

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
	uint32_t *p = (uint32_t *)void_p;
	uint32_t h1, h2, h3, h4;
	BufferSizeType i;

	for (i = 0; i < n; i++)
	{
		h1 = (*p) & 0xff;
		h2 = ((*p) >> 8) & 0xff;
		h3 = ((*p) >> 16) & 0xff;
		h4 = ((*p) >> 24) & 0xff;
		*p = (h1 << 24) | (h2 << 16) | (h3 << 8) | h4;

		p = p + 1;
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
	FILE *file = fopen(TestResources::getLocation(TestResources::IMAGE_ANALYZE_BIGENDIAN_RESOURCE), "rb");
	fread(&hdr, sizeof(hdr), 1, file);

	int sizeof_hdr = hdr.hk.sizeof_hdr;
	if (sizeof_hdr != 348)
		test::ByteSwap<int>(&hdr.hk.sizeof_hdr);
	EXPECT_EQ(348, hdr.hk.sizeof_hdr);
}

TEST(ByteSwap, littleendian)
{
	struct dsr hdr;
	FILE *file = fopen(TestResources::getLocation(TestResources::IMAGE_ANALYZE_LITTLEENDIAN_RESOURCE), "rb");
	fread(&hdr, sizeof(hdr), 1, file);

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

	float fvalue = 0.12;
	test::ByteSwap<float>(&fvalue);
	EXPECT_EQ(-1.92243393014834230422427828262E-29, fvalue);
}

union floatbitconvert
{
	float f;
	uint32_t u;
};

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
