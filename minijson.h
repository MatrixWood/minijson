#ifndef MINIJSON_H
#define MINIJSON_H

#include <stddef.h> /* size_t */

typedef enum 
{
	MJ_NULL,
	MJ_FALSE,
	MJ_TRUE,
	MJ_NUMBER,
	MJ_STRING,
	MJ_ARRAY,
	MJ_OBJECT
}MJ_type;

typedef struct 
{
	union
	{
		struct
		{
			char *s;
			size_t len;
		}s;				/* string */
		double n;		/* number */
	}u;
	MJ_type type;
}MJ_value;

enum
{
	MJ_PARSE_OK = 0,
	MJ_PARSE_EXPECT_VALUE,
	MJ_PARSE_INVALID_VALUE,
	MJ_PARSE_ROOT_NOT_SINGULAR,
	MJ_PARSE_NUMBER_TOO_BIG,
	MJ_PARSE_MISS_QUOTATION_MARK,
	MJ_PARSE_INVALID_STRING_ESCAPE,
	MJ_PARSE_INVALID_STRING_CHAR,
	MJ_PARSE_INVALID_UNICODE_HEX,
    MJ_PARSE_INVALID_UNICODE_SURROGATE
};

#define MJ_init(v) do { (v)->type = MJ_NULL; } while(0)

int MJ_parse(MJ_value *v, const char *json);

void MJ_free(MJ_value *v);

MJ_type MJ_get_type(const MJ_value *v);

#define MJ_set_null(v) MJ_free(v)

int MJ_get_boolean(const MJ_value *v);
void MJ_set_boolean(MJ_value *v, int b);

double MJ_get_number(const MJ_value *v);
void MJ_set_number(MJ_value *v, double n);

const char* MJ_get_string(const MJ_value *v);
size_t MJ_get_string_length(const MJ_value *v);
void MJ_set_string(MJ_value *v, const char *s, size_t len);

#endif