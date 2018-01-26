#include "minijson.h"
#include <assert.h>		/* assert() */
#include <errno.h>		/* error, ERANGE */
#include <math.h>		/* HUGE_VAL */
#include <stdlib.h>		/* NULL, strtod() */

#define EXPECT(c, ch) 	do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)		((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')

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
/*
// deprecated
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
*/
static int MJ_parse_literal(MJ_context *c, MJ_value *v, const char *literal, MJ_type type)
{
	size_t i;
	EXPECT(c, literal[0]);
	for(i = 0; literal[i + 1]; i++)
		if(c->json[i] != literal[i + 1])
			return MJ_PARSE_INVALID_VALUE;
	c->json += i;
	v->type = type;
	return MJ_PARSE_OK;
}

static int MJ_parse_number(MJ_context *c, MJ_value *v)
{
	const char *p = c->json;
	if(*p == '-')
		p++;
	if(*p == '0')
		p++;
	else
	{
		if(!ISDIGIT1TO9(*p))
			return MJ_PARSE_INVALID_VALUE;
		for(p++; ISDIGIT(*p); p++);
	}
	if(*p == '.')
	{
		p++;
		if(!ISDIGIT(*p))
			return MJ_PARSE_INVALID_VALUE;
		for(p++; ISDIGIT(*p); p++);
	}
	if(*p == 'e' || *p == 'E')
	{
		p++;
		if(*p == '+' || *p == '-')
			p++;
		if(!ISDIGIT(*p))
			return MJ_PARSE_INVALID_VALUE;
		for(p++; ISDIGIT(*p); p++);
	}
	errno = 0;
	v->n = strtod(c->json, NULL);
	if(errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL))
		return MJ_PARSE_INVALID_VALUE;
	c->json = p;
	v->type = MJ_NUMBER;
	return MJ_PARSE_OK;
}

static int MJ_parse_value(MJ_context *c, MJ_value *v)
{
	switch(*c->json)
	{
		case 't': return MJ_parse_literal(c, v, "true", MJ_TRUE);
		case 'f': return MJ_parse_literal(c, v, "false", MJ_FALSE);
		case 'n': return MJ_parse_literal(c, v, "null", MJ_NULL);
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