#include "minijson.h"
#include <assert.h>		/* assert() */
#include <errno.h>		/* error, ERANGE */
#include <math.h>		/* HUGE_VAL */
#include <stdlib.h>		/* NULL, strtod() */
#include <string.h>  	/* memcpy() */

#define EXPECT(c, ch) 	do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)		((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')
#define PUTC(c, ch) do { *(char*)MJ_context_push(c, sizeof(char)) = (ch); } while(0)


#ifndef MJ_PARSE_STACK_INIT_SIZE
#define MJ_PARSE_STACK_INIT_SIZE 256
#endif

/*
*	why need stack?
*	case if we parse string, we make a dynamic array every time.
*	it will waste more time.
*/
typedef struct
{
	const char *json;
	char *stack;
	size_t size, top;
}MJ_context;

static void* MJ_context_push(MJ_context *c, size_t size)
{
	void *ret;
	assert(size > 0);
	if(c->top + size >= c->size)
	{
		if(c->size == 0)
			c->size = MJ_PARSE_STACK_INIT_SIZE;
		while(c->top + size >= c->size)
		{
			c->size += c->size >> 1; // growth factor = 1.5
		}
		c->stack = (char *)realloc(c->stack, c->size);
	}
	ret = c->stack + c->top;
	c->top += size;
	return ret;
}

static void* MJ_context_pop(MJ_context *c, size_t size)
{
	assert(c->top >= size);
	return c->stack + (c->top -= size);
}

// whitespace
static void MJ_parse_whitespace(MJ_context *c)
{
	const char *p = c->json;
	while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
		++p;
	c->json = p;
}

/*
// deprecated
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
*/

// literal: null, true, false
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

// number
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
	v->u.n = strtod(c->json, NULL);
	if(errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL))
		return MJ_PARSE_INVALID_VALUE;
	c->json = p;
	v->type = MJ_NUMBER;
	return MJ_PARSE_OK;
}

static int MJ_parse_string(MJ_context *c, MJ_value *v)
{
	size_t head = c->top;
	size_t len;
	const char *p;
	EXPECT(c, '\"');
	p = c->json;
	while(1)
	{
		char ch = *p++;
		switch (ch) 
		{
            case '\"':
                len = c->top - head;
                MJ_set_string(v, (const char*)MJ_context_pop(c, len), len);
                c->json = p;
                return MJ_PARSE_OK;
            case '\\':
                switch (*p++) 
                {
                    case '\"': PUTC(c, '\"'); break;
                    case '\\': PUTC(c, '\\'); break;
                    case '/':  PUTC(c, '/' ); break;
                    case 'b':  PUTC(c, '\b'); break;
                    case 'f':  PUTC(c, '\f'); break;
                    case 'n':  PUTC(c, '\n'); break;
                    case 'r':  PUTC(c, '\r'); break;
                    case 't':  PUTC(c, '\t'); break;
                    default:
                        c->top = head;
                        return MJ_PARSE_INVALID_STRING_ESCAPE;
                }
                break;
            case '\0':
                c->top = head;
                return MJ_PARSE_MISS_QUOTATION_MARK;
            default:
                if ((unsigned char)ch < 0x20) 
                { 
                    c->top = head;
                    return MJ_PARSE_INVALID_STRING_CHAR;
                }
                PUTC(c, ch);
        }
	}
}

static int MJ_parse_value(MJ_context *c, MJ_value *v)
{
	switch(*c->json)
	{
		case 't': return MJ_parse_literal(c, v, "true", MJ_TRUE);
		case 'f': return MJ_parse_literal(c, v, "false", MJ_FALSE);
		case 'n': return MJ_parse_literal(c, v, "null", MJ_NULL);
		default: return MJ_parse_number(c, v);
		case '"':  return MJ_parse_string(c, v);
		case '\0': return MJ_PARSE_EXPECT_VALUE;
	}
}

int MJ_parse(MJ_value *v, const char *json)
{
	MJ_context c;
	int ret;
	assert(v != NULL);
	c.json = json;
	c.stack = NULL;
	c.size = c.top = 0;
	MJ_init(v);
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
	assert(c.top == 0);
	free(c.stack);
	return ret;
}

void MJ_free(MJ_value *v)
{
	assert(v != NULL);
	if(v->type == MJ_STRING)
		free(v->u.s.s);
	v->type = MJ_NULL;
}

MJ_type MJ_get_type(const MJ_value *v)
{
	assert(v != NULL);
	return v->type;
}

int MJ_get_boolean(const MJ_value *v) 
{
    assert(v != NULL && (v->type == MJ_TRUE || v->type == MJ_FALSE));
    return v->type == MJ_TRUE;
}

void MJ_set_boolean(MJ_value *v, int b) 
{
    MJ_free(v);
    v->type = b ? MJ_TRUE : MJ_FALSE;
}

void MJ_set_number(MJ_value *v, double n) 
{
    MJ_free(v);
    v->u.n = n;
    v->type = MJ_NUMBER;
}

double MJ_get_number(const MJ_value *v)
{
	assert(v != NULL && v->type == MJ_NUMBER);
	return v->u.n;
}

const char* MJ_get_string(const MJ_value *v) 
{
    assert(v != NULL && v->type == MJ_STRING);
    return v->u.s.s;
}

size_t MJ_get_string_length(const MJ_value *v) 
{
    assert(v != NULL && v->type == MJ_STRING);
    return v->u.s.len;
}

void MJ_set_string(MJ_value *v, const char *s, size_t len)
{
	assert(v != NULL && (s != NULL || len == 0));
	MJ_free(v);
	v->u.s.s = (char *)malloc(len + 1);
	memcpy(v->u.s.s, s, len);
	v->u.s.s[len] = '\0';
	v->u.s.len = len;
	v->type = MJ_STRING;
}