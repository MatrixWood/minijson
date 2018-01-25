#ifndef MINIJSON_H
#define MINIJSON_H

typedef enum 
{
	MJ_NULL,
	MJ_FALSE,
	MJ_TRUE,
	MJ_NUMBER,
	MJ_STRING,
	MJ_ARRAY
	MJ_OBJECT
}MJ_type;

typedef struct 
{
	MJ_type type;
}MJ_value;

enum
{
	MJ_PARSE_OK = 0;
	MJ_PARSE_EXPECT_VALUE,
	MJ_PARSE_INVAILD_VALUE,
	MJ_PARSE_ROOT_NOT_SINGULAR
};

int MJ_parse(MJ_value *v, const char *json);

MJ_type MJ_get_type(const MJ_value *v);

#endif