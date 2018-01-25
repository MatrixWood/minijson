#include "minijson.h"
#include <assert.h>
#include <stdlib.h>

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

static int MJ_parse_value(MJ_context *c, MJ_value *v)
{
	switch(*c->json)
	{
		case 'n': return MJ_parse_null(c, v);
		case '\0': return MJ_PARSE_EXPECT_VALUE;
		default: return MJ_PARSE_INVALID_VALUE;
	}
}

int MJ_parse(MJ_value *v, const char *json)
{
	MJ_context c;
	assert(v != NULL);
	c.json = json;
	v->type = MJ_NULL;
	MJ_parse_whitespace(&c);
	return MJ_parse_value(&c, v);
}

MJ_type MJ_get_type(const MJ_value *v)
{
	assert(v != NULL);
	return v->type;
}