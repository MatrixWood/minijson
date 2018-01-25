#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minijson.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
	do {\
		test_count++;\
		if(equality)\
			test_pass;\
		else{\
			fprintf(stderr, "%s:%d: expect: " format "actual: " format "\n", __FILE__, __LINE__, expect, actual);\
			main_ret;\
		}\
	}while(0)

#define EXPEC_EQ_INT(expect, actual) EXPEC_EQ_BASE((expect) == (actual), expect, actual, "%d")

static void test_parser_null()
{
	MJ_value v;
	v.type = MJ_FALSE;
    EXPECT_EQ_INT(MJ_PARSE_EXPECT_VALUE, MJ_parse(&v, ""));
    EXPECT_EQ_INT(MJ_NULL, MJ_get_type(&v));

    v.type = MJ_FALSE;
    EXPECT_EQ_INT(MJ_PARSE_EXPECT_VALUE, MJ_parse(&v, " "));
    EXPECT_EQ_INT(MJ_NULL, MJ_get_type(&v));
}

static void test_parse_invalid_value() {
    MJ_value v;
    v.type = MJ_FALSE;
    EXPECT_EQ_INT(MJ_PARSE_INVALID_VALUE, MJ_parse(&v, "nul"));
    EXPECT_EQ_INT(MJ_NULL, MJ_get_type(&v));

    v.type = MJ_FALSE;
    EXPECT_EQ_INT(MJ_PARSE_INVALID_VALUE, MJ_parse(&v, "?"));
    EXPECT_EQ_INT(MJ_NULL, MJ_get_type(&v));
}

static void test_parse_root_not_singular() {
    MJ_value v;
    v.type = MJ_FALSE;
    EXPECT_EQ_INT(MJ_PARSE_ROOT_NOT_SINGULAR, MJ_parse(&v, "null x"));
    EXPECT_EQ_INT(MJ_NULL, MJ_get_type(&v));
}

static void test_parse() {
    test_parse_null();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
}

int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}