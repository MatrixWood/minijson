#include "minijson.h"
#include <assert.h>
#include <stdlib.h>		/* NULL, strtod() */

#define EXPECT(c, ch) do { assert(*c->json == (ch)); c->json++; } while(0)

typedef struct
{
	const char *json;
}MJ_context;

static void MJ_parse_whitespace(MJ_context *c)
{
	const char *p = c->json;
	while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
		++p;
	c->json = p;
}

static int MJ_parse_null(MJ_context *c, MJ_value *v)
{
	EXPECT(c, 'n');
	if(c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
		return MJ_PARSE_INVALID_VALUE;
	c->json += 3;
	v->type = MJ_NULL;
	return MJ_PARSE_OK;
}

static int MJ_parse_true(MJ_context *c, MJ_value *v)
{
	EXPECT(c, 't');
	if(c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
		return MJ_PARSE_INVALID_VALUE;
	c->json += 3;
	v->type = MJ_TRUE;
	return MJ_PARSE_OK;
}

static int MJ_parse_false(MJ_context *c, MJ_value *v)
{
	EXPECT(c, 'f');
	if(c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
		return MJ_PARSE_INVALID_VALUE;
	c->json += 4;
	v->type = MJ_FALSE;
	return MJ_PARSE_OK;
}

static int MJ_parse_number(MJ_context *c, MJ_value *v)
{
	char *end;
	v->n = strtod(c->json, &end);
	if(c->json == end)
		return MJ_PARSE_INVALID_VALUE;
	c->json = end;
	v->type = MJ_NUMBER;
	return MJ_PARSE_OK;
}

static int MJ_parse_value(MJ_context *c, MJ_value *v)
{
	switch(*c->json)
	{
		case 't': return MJ_parse_true(c, v);
		case 'f': return MJ_parse_false(c, v);
		case 'n': return MJ_parse_null(c, v);
		default: return MJ_parse_number(c, v);
		case '\0': return MJ_PARSE_EXPECT_VALUE;
	}
}

int MJ_parse(MJ_value *v, const char *json)
{
	MJ_context c;
	int ret;
	assert(v != NULL);
	c.json = json;
	v->type = MJ_NULL;
	MJ_parse_whitespace(&c);
	if((ret = MJ_parse_value(&c, v)) == MJ_PARSE_OK)
	{
		MJ_parse_whitespace(&c);
		if(*c.json != '\0')
		{
			v->type = MJ_NULL;
			ret = MJ_PARSE_ROOT_NOT_SINGULAR;
		}
	}
	return ret;
}

MJ_type MJ_get_type(const MJ_value *v)
{
	assert(v != NULL);
	return v->type;
}

double MJ_get_number(const MJ_value *v)
{
	assert(v != NULL && v->type == MJ_NUMBER);
	return v->n;
}