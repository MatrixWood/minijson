#include "minijson.h"
#include <assert.h>		/* assert() */
#include <errno.h>		/* error, ERANGE */
#include <math.h>		/* HUGE_VAL */
#include <stdlib.h>		/* NULL, strtod(), malloc(), realloc(), free() */
#include <string.h>		/* memcpy() */

#define EXPECT(c, ch)		do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)			((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) 	((ch) >= '1' && (ch) <= '9')
#define PUTC(c, ch)			do { *(char*)MJ_context_push(c, sizeof(char)) = (ch); } while(0)
#define STRING_ERROR(ret)	do { c->top = head; return ret; } while(0)

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

static const char* MJ_parse_hex4(const char *p, unsigned *u)
{
	int i;
	*u = 0;
	for(i = 0; i < 4; ++i)
	{
		char ch = *p++;
		*u <<= 4;
		if(ch >= '0' && ch <= '9')
			*u |= ch - '0';
		else if(ch >= 'A' && ch <= 'F')
			*u |= ch - ('A' - 10);
		else if(ch >= 'a' && ch <= 'f')
			*u |= ch - ('a' - 10);
		else
			return NULL;
	}
	return p;
}

static void MJ_encode_utf8(MJ_context *c, unsigned u)
{
	if (u <= 0x7F) 
        PUTC(c, u & 0xFF);
    else if (u <= 0x7FF) 
    {
        PUTC(c, 0xC0 | ((u >> 6) & 0xFF));
        PUTC(c, 0x80 | ( u       & 0x3F));
    }
    else if (u <= 0xFFFF) 
    {
        PUTC(c, 0xE0 | ((u >> 12) & 0xFF));
        PUTC(c, 0x80 | ((u >>  6) & 0x3F));
        PUTC(c, 0x80 | ( u        & 0x3F));
    }
    else 
    {
        assert(u <= 0x10FFFF);
        PUTC(c, 0xF0 | ((u >> 18) & 0xFF));
        PUTC(c, 0x80 | ((u >> 12) & 0x3F));
        PUTC(c, 0x80 | ((u >>  6) & 0x3F));
        PUTC(c, 0x80 | ( u        & 0x3F));
    }
}

static int MJ_parse_string(MJ_context *c, MJ_value *v)
{
	size_t head = c->top;
	size_t len;
	unsigned u, u2;
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
                    case 'u':	/* support utf-8 */
                        if (!(p = MJ_parse_hex4(p, &u)))
                            STRING_ERROR(MJ_PARSE_INVALID_UNICODE_HEX);
                        if (u >= 0xD800 && u <= 0xDBFF) /* surrogate pair */
                        { 
                            if (*p++ != '\\')
                                STRING_ERROR(MJ_PARSE_INVALID_UNICODE_SURROGATE);
                            if (*p++ != 'u')
                                STRING_ERROR(MJ_PARSE_INVALID_UNICODE_SURROGATE);
                            if (!(p = MJ_parse_hex4(p, &u2)))
                                STRING_ERROR(MJ_PARSE_INVALID_UNICODE_HEX);
                            if (u2 < 0xDC00 || u2 > 0xDFFF)
                                STRING_ERROR(MJ_PARSE_INVALID_UNICODE_SURROGATE);
                            u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                        }
                        MJ_encode_utf8(c, u);
                        break;
                    default:
                        STRING_ERROR(MJ_PARSE_INVALID_STRING_ESCAPE);
                }
                break;
            case '\0':
                c->top = head;
                return MJ_PARSE_MISS_QUOTATION_MARK;
            default:
                if ((unsigned char)ch < 0x20) 
                { 
                    STRING_ERROR(MJ_PARSE_INVALID_STRING_ESCAPE);
                }
                PUTC(c, ch);
        }
	}
}

static int MJ_parse_value(MJ_context *c, MJ_value *v);

static int MJ_parse_array(MJ_context *c, MJ_value *v)
{
	size_t size = 0;
	int ret;
	EXPECT(c, '[');
	MJ_parse_whitespace(c);
	if(*c->json == ']')
	{
		c->json++;
		v->type = MJ_ARRAY;
		v->u.a.size = 0;
		v->u.a.e = NULL;
		return MJ_PARSE_OK;
	}
	while(1)
	{
		MJ_value e;
		MJ_init(&e);
		if((ret = MJ_parse_value(c, &e)) != MJ_PARSE_OK)
			break;
		memcpy(MJ_context_push(c, sizeof(MJ_value)), &e, sizeof(MJ_value));
		size++;
		MJ_parse_whitespace(c);
		if(*c->json == ',')
		{
			c->json++;
			MJ_parse_whitespace(c);
		}
		else if(*c->json == ']')
		{
			c->json++;
			v->type = MJ_ARRAY;
			v->u.a.size = size;
			size *= sizeof(MJ_value);
			memcpy(v->u.a.e = (MJ_value*)malloc(size), MJ_context_pop(c, size), size);
			return MJ_PARSE_OK;
		}
		else
		{
			ret = MJ_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
			break;
		}
	}
	/* Pop and free values on the stack */
	for(size_t i = 0; i < size; ++i)
		MJ_free((MJ_value *)MJ_context_pop(c, sizeof(MJ_value)));
	return ret;
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
		case '[':  return MJ_parse_array(c, v);
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
	size_t i;
	assert(v != NULL);
	switch(v->type)
	{
		case MJ_STRING:
			free(v->u.s.s);
			break;
		case MJ_ARRAY:
			for(i = 0; i < v->u.a.size; i++)
				MJ_free(&v->u.a.e[i]);
			free(v->u.a.e);
			break;
		default:
			break;
	}
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

size_t MJ_get_array_size(const MJ_value* v) 
{
    assert(v != NULL && v->type == MJ_ARRAY);
    return v->u.a.size;
}

MJ_value* MJ_get_array_element(const MJ_value* v, size_t index) 
{
    assert(v != NULL && v->type == MJ_ARRAY);
    assert(index < v->u.a.size);
    return &v->u.a.e[index];
}
