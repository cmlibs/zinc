/*
 * Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __ZINCTEST_UTILITIES_TESTENUM_HPP__
#define __ZINCTEST_UTILITIES_TESTENUM_HPP__

/** Test enumerated value to/from string methods.
 * Works only for enumerations with values starting at 0.
 * Also checks enumToString returns nullptr for value enumCount
 * so will fail if new enumerations are added.
 * @param enumCount  Size of enumNames.
 * @param enumNames  Expected names for enum value 0..enumCount-1.
 * enumNames[0] is typically nullptr for INVALID
 */
template <typename T> void testEnum(int enumCount, const char **enumNames,
	char *enumToString(T), T enumFromString(const char *))
{
	for (int i = 0; i < enumCount; ++i)
	{
		const T t = static_cast<T>(i);
		char *name = enumToString(t);
		EXPECT_STREQ(enumNames[i], name);
		EXPECT_EQ(t, enumFromString(enumNames[i]));
		cmzn_deallocate(name);
	}
	EXPECT_STREQ(nullptr, enumToString(static_cast<T>(enumCount)));
}

/** Test enumerated value to/from string methods.
 * Works only for enumerations with values starting at 0, 1, 2, 4, 8...
 * i.e. following numbers are 1 bit shifted.
 * Also checks enumToString returns nullptr for value 1 << enumCount-1
 * so will fail if new enumerations are added.
 * @param enumCount  Size of enumNames.
 * @param enumNames  Expected names for enum values.
 * enumNames[0] is typically nullptr for INVALID
 */
template <typename T> void testEnumBitShift(int enumCount, const char **enumNames,
	char *enumToString(T), T enumFromString(const char *))
{
	for (int i = 0; i < enumCount; ++i)
	{
		const T t = static_cast<T>((i == 0) ? 0 : 1 << (i - 1));;
		char *name = enumToString(t);
		EXPECT_STREQ(enumNames[i], name);
		EXPECT_EQ(t, enumFromString(enumNames[i]));
		cmzn_deallocate(name);
	}
	EXPECT_STREQ(nullptr, enumToString(static_cast<T>(1 << (enumCount - 1))));
}

#endif // __ZINCTEST_UTILITIES_TESTENUM_HPP__
