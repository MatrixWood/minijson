#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minijson.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

// test marco
#define EXPECT_EQ_BASE(equality, expect, actual, format) \
	do {\
		test_count++;\
		if(equality)\
			test_pass++;\
		else{\
			fprintf(stderr, "%s:%d: expect: " format "actual: " format "\n", __FILE__, __LINE__, expect, actual);\
			main_ret = 1;\
		}\
	}while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define EXPECT_EQ_STRING(expect, actual, alength)\
	EXPECT_EQ_BASE(sizeof(expect) - 1 == alength && memcmp(expect, actual, alength) == 0, expect, actual, "%s")
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")	

#if defined(_MSC_VER)
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%Iu")
#else
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%zu")
#endif

#define TEST_ERROR(error, json)\
	do{\
		MJ_value v;\
		MJ_init(&v);\
		v.type = MJ_FALSE;\
		EXPECT_EQ_INT(error, MJ_parse(&v, json));\
		EXPECT_EQ_INT(MJ_NULL, MJ_get_type(&v));\
		MJ_free(&v);\
	}while(0)

#define TEST_NUMBER(expect, json)\
	do{\
		MJ_value v;\
		EXPECT_EQ_INT(MJ_PARSE_OK, MJ_parse(&v, json));\
		EXPECT_EQ_INT(MJ_NUMBER, MJ_get_type(&v));\
		EXPECT_EQ_DOUBLE(expect, MJ_get_number(&v));\
	}while(0)

#define TEST_STRING(expect, json)\
	do{\
		MJ_value v;\
		MJ_init(&v);\
		EXPECT_EQ_INT(MJ_PARSE_OK, MJ_parse(&v, json));\
		EXPECT_EQ_INT(MJ_STRING, MJ_get_type(&v));\
		EXPECT_EQ_STRING(expect, MJ_get_string(&v), MJ_get_string_length(&v));\
		MJ_free(&v);\
	}while(0)

//TEST_MARCO : if define this marco, there are some tests will never pass.
#if 0
#define TEST_MARCO 1
#endif


// all test
static void test_parse_null() 
{
	MJ_value v;
    MJ_init(&v);
    MJ_set_boolean(&v, 0);
	EXPECT_EQ_INT(MJ_PARSE_OK, MJ_parse(&v, "null"));
	EXPECT_EQ_INT(MJ_NULL, MJ_get_type(&v));
	MJ_free(&v);
}

static void test_parse_true()
{
	MJ_value v;
    MJ_init(&v);
    MJ_set_boolean(&v, 0);
	EXPECT_EQ_INT(MJ_PARSE_OK, MJ_parse(&v, "true"));
	EXPECT_EQ_INT(MJ_TRUE, MJ_get_type(&v));
	MJ_free(&v);
}

static void test_parse_false()
{
	MJ_value v;
	MJ_init(&v);
    MJ_set_boolean(&v, 0);
	EXPECT_EQ_INT(MJ_PARSE_OK, MJ_parse(&v, "false"));
	EXPECT_EQ_INT(MJ_FALSE, MJ_get_type(&v));
	MJ_free(&v);
}

static void test_parse_number()
{
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

static void test_parse_string() 
{
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
}

/*
// deprecated
static void test_parse_expect_value() 
{
	MJ_value v;

	v.type = MJ_FALSE;
	EXPECT_EQ_INT(MJ_PARSE_EXPECT_VALUE, MJ_parse(&v, ""));
	EXPECT_EQ_INT(MJ_NULL, MJ_get_type(&v));

	v.type = MJ_FALSE;
	EXPECT_EQ_INT(MJ_PARSE_EXPECT_VALUE, MJ_parse(&v, " "));
	EXPECT_EQ_INT(MJ_NULL, MJ_get_type(&v));
}

static void test_parse_invalid_value() 
{
	MJ_value v;
	v.type = MJ_FALSE;
	EXPECT_EQ_INT(MJ_PARSE_INVALID_VALUE, MJ_parse(&v, "nul"));
	EXPECT_EQ_INT(MJ_NULL, MJ_get_type(&v));

	v.type = MJ_FALSE;
	EXPECT_EQ_INT(MJ_PARSE_INVALID_VALUE, MJ_parse(&v, "?"));
	EXPECT_EQ_INT(MJ_NULL, MJ_get_type(&v));
}
*/

static void test_parse_expect_value() 
{
	TEST_ERROR(MJ_PARSE_EXPECT_VALUE, "");
	TEST_ERROR(MJ_PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value() 
{
	TEST_ERROR(MJ_PARSE_INVALID_VALUE, "nul");
	TEST_ERROR(MJ_PARSE_INVALID_VALUE, "?");

#ifdef TEST_MARCO
    /* invalid number */
    TEST_ERROR(MJ_PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(MJ_PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(MJ_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(MJ_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_ERROR(MJ_PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(MJ_PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(MJ_PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(MJ_PARSE_INVALID_VALUE, "nan");
    
    /* invalid value in array */
    TEST_ERROR(MJ_PARSE_INVALID_VALUE, "[1,]");
    TEST_ERROR(MJ_PARSE_INVALID_VALUE, "[\"a\", nul]");
#endif
}

static void test_parse_root_not_singular() 
{
	TEST_ERROR(MJ_PARSE_ROOT_NOT_SINGULAR, "null x");

#ifdef TEST_MARCO
    /* invalid number */
    TEST_ERROR(MJ_PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' or nothing */
    TEST_ERROR(MJ_PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(MJ_PARSE_ROOT_NOT_SINGULAR, "0x123");
#endif
}

static void test_parse_number_too_big() 
{
#ifdef TEST_MARCO
    TEST_ERROR(MJ_PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(MJ_PARSE_NUMBER_TOO_BIG, "-1e309");
#endif
}

static void test_parse_missing_quotation_mark() 
{
    TEST_ERROR(MJ_PARSE_MISS_QUOTATION_MARK, "\"");
    TEST_ERROR(MJ_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() 
{
    TEST_ERROR(MJ_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(MJ_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(MJ_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(MJ_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() 
{
    TEST_ERROR(MJ_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(MJ_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_parse_invalid_unicode_hex() 
{
    TEST_ERROR(MJ_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
    TEST_ERROR(MJ_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
    TEST_ERROR(MJ_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
    TEST_ERROR(MJ_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
    TEST_ERROR(MJ_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
    TEST_ERROR(MJ_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
    TEST_ERROR(MJ_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_ERROR(MJ_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
    TEST_ERROR(MJ_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_ERROR(MJ_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
    TEST_ERROR(MJ_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
    TEST_ERROR(MJ_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
}

static void test_parse_invalid_unicode_surrogate() 
{
    TEST_ERROR(MJ_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    TEST_ERROR(MJ_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    TEST_ERROR(MJ_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    TEST_ERROR(MJ_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    TEST_ERROR(MJ_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

static void test_parse_miss_comma_or_square_bracket() 
{
#ifdef TEST_MARCO
    TEST_ERROR(MJ_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
    TEST_ERROR(MJ_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
    TEST_ERROR(MJ_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
    TEST_ERROR(MJ_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
#endif
}

static void test_access_null() 
{
    MJ_value v;
    MJ_init(&v);
    MJ_set_string(&v, "a", 1);
    MJ_set_null(&v);
    EXPECT_EQ_INT(MJ_NULL, MJ_get_type(&v));
    MJ_free(&v);
}

static void test_access_boolean() 
{
    MJ_value v;
    MJ_init(&v);
    MJ_set_string(&v, "a", 1);
    MJ_set_boolean(&v, 1);
    EXPECT_TRUE(MJ_get_boolean(&v));
    MJ_set_boolean(&v, 0);
    EXPECT_FALSE(MJ_get_boolean(&v));
    MJ_free(&v);
}

static void test_access_number()
{
    MJ_value v;
    MJ_init(&v);
    MJ_set_string(&v, "a", 1);
    MJ_set_number(&v, 1234.5);
    EXPECT_EQ_DOUBLE(1234.5, MJ_get_number(&v));
    MJ_free(&v);
}

static void test_access_string()
{
	MJ_value v;
	MJ_init(&v);
	MJ_set_string(&v, "", 0);
	EXPECT_EQ_STRING("", MJ_get_string(&v), MJ_get_string_length(&v));
	MJ_set_string(&v, "Hello", 5);
	EXPECT_EQ_STRING("Hello", MJ_get_string(&v), MJ_get_string_length(&v));
	MJ_free(&v);
}

static void test_parse_array() 
{
    size_t i, j;
    MJ_value v;

    MJ_init(&v);
    EXPECT_EQ_INT(MJ_PARSE_OK, MJ_parse(&v, "[ ]"));
    EXPECT_EQ_INT(MJ_ARRAY, MJ_get_type(&v));
    EXPECT_EQ_SIZE_T(0, MJ_get_array_size(&v));
    MJ_free(&v);

    MJ_init(&v);
    EXPECT_EQ_INT(MJ_PARSE_OK, MJ_parse(&v, "[ null , false , true , 123 , \"abc\" ]"));
    EXPECT_EQ_INT(MJ_ARRAY, MJ_get_type(&v));
    EXPECT_EQ_SIZE_T(5, MJ_get_array_size(&v));
    EXPECT_EQ_INT(MJ_NULL,   MJ_get_type(MJ_get_array_element(&v, 0)));
    EXPECT_EQ_INT(MJ_FALSE,  MJ_get_type(MJ_get_array_element(&v, 1)));
    EXPECT_EQ_INT(MJ_TRUE,   MJ_get_type(MJ_get_array_element(&v, 2)));
    EXPECT_EQ_INT(MJ_NUMBER, MJ_get_type(MJ_get_array_element(&v, 3)));
    EXPECT_EQ_INT(MJ_STRING, MJ_get_type(MJ_get_array_element(&v, 4)));
    EXPECT_EQ_DOUBLE(123.0, MJ_get_number(MJ_get_array_element(&v, 3)));
    EXPECT_EQ_STRING("abc", MJ_get_string(MJ_get_array_element(&v, 4)), MJ_get_string_length(MJ_get_array_element(&v, 4)));
    MJ_free(&v);

    MJ_init(&v);
    EXPECT_EQ_INT(MJ_PARSE_OK, MJ_parse(&v, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
    EXPECT_EQ_INT(MJ_ARRAY, MJ_get_type(&v));
    EXPECT_EQ_SIZE_T(4, MJ_get_array_size(&v));
    for (i = 0; i < 4; i++) {
        MJ_value* a = MJ_get_array_element(&v, i);
        EXPECT_EQ_INT(MJ_ARRAY, MJ_get_type(a));
        EXPECT_EQ_SIZE_T(i, MJ_get_array_size(a));
        for (j = 0; j < i; j++) {
            MJ_value* e = MJ_get_array_element(a, j);
            EXPECT_EQ_INT(MJ_NUMBER, MJ_get_type(e));
            EXPECT_EQ_DOUBLE((double)j, MJ_get_number(e));
        }
    }
    MJ_free(&v);
}

static void test_parse() 
{
	test_parse_null();
	test_parse_true();
	test_parse_false();
	test_parse_number();
    test_parse_string();
    test_parse_array();

	test_parse_expect_value();
	test_parse_invalid_value();
	test_parse_root_not_singular();
	test_parse_number_too_big();
	test_parse_missing_quotation_mark();
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();
    test_parse_invalid_unicode_hex();
    test_parse_invalid_unicode_surrogate();
    test_parse_miss_comma_or_square_bracket();

    test_access_null();
    test_access_boolean();
    test_access_number();
    test_access_string();
}

int main() {
#ifdef _WINDOWS
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	test_parse();
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
	return main_ret;
}
