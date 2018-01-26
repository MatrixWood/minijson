#ifndef MINIJSON_H
#define MINIJSON_H

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
	double n;
	MJ_type type;
}MJ_value;

enum
{
	MJ_PARSE_OK = 0,
	MJ_PARSE_EXPECT_VALUE,
	MJ_PARSE_INVALID_VALUE,
	MJ_PARSE_ROOT_NOT_SINGULAR,
	MJ_PARSE_NUMBER_TOO_BIG
};

int MJ_parse(MJ_value *v, const char *json);

MJ_type MJ_get_type(const MJ_value *v);

double MJ_get_number(const MJ_value *v);

#endif