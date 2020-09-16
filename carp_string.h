
#ifndef CARP_STRING_INCLUDED
#define CARP_STRING_INCLUDED

#include <stdlib.h>
#include <string.h>

#define carp_roundup32(x) (--(x), (x)|=(x)>>1, (x)|=(x)>>2, (x)|=(x)>>4, (x)|=(x)>>8, (x)|=(x)>>16, ++(x))

typedef struct {
	size_t l;	// 字符串长度
	size_t m;	// 字符串容量
	char *s;	// 字符串首地址
} carp_string_t;

// 如果容量不足size，就扩展到size，否则不变
static bool carp_string_setcap(carp_string_t* s, size_t size)
{
	if (s->m >= size) return true;

	s->m = size;
	carp_roundup32(s->m);
	
	char* tmp = (char*)realloc(s->s, s->m);
	if (tmp == 0) return false;
	s->s = tmp;
	return true;
}

// 设置字符串长度
static bool carp_string_setlen(carp_string_t* s, size_t size)
{
	if (!carp_string_setcap(s, size + 1)) return false;
	s->l = size;
	s->s[size] = 0;
	return true;
}

static const char* carp_string_str(carp_string_t* s)
{
	static char s_empty[1] = { 0 };
	if (s->s == 0 && !carp_string_setlen(s, 0)) return s_empty;
	return s->s;
}

static size_t carp_string_len(carp_string_t* s)
{
	return s->l;
}

static void carp_string_free(carp_string_t* s)
{
	if (s->s) free(s->s);
	s->s = 0;
	s->l = s->m = 0;
}

static void carp_string_putsn(carp_string_t* s, const char* p, size_t l)
{
	if (!carp_string_setcap(s, s->l + l + 1)) return;
	
	memcpy(s->s + s->l, p, l);
	s->l += l;
	s->s[s->l] = 0;
}

static void carp_string_puts(carp_string_t* s, const char *p)
{
	carp_string_putsn(s, p, strlen(p));
}

static void carp_string_putc(carp_string_t* s, char c)
{
	if (!carp_string_setcap(s, s->l + 1)) return;
	s->s[s->l++] = c;
	s->s[s->l] = 0;
}

typedef struct {
	size_t l;	// 字符串长度
	size_t m;	// 字符串容量
	wchar_t* s;	// 字符串首地址
} carp_wstring_t;

// 如果容量不足size，就扩展到size，否则不变
static bool carp_wstring_setcap(carp_wstring_t* s, size_t size)
{
	if (s->m >= size) return true;

	s->m = size;
	carp_roundup32(s->m);

	wchar_t* tmp = (wchar_t*)realloc(s->s, s->m * sizeof(wchar_t));
	if (tmp == 0) return false;
	s->s = tmp;
	return true;
}

// 设置字符串长度
static bool carp_wstring_setlen(carp_wstring_t* s, size_t size)
{
	if (!carp_wstring_setcap(s, size + 1)) return false;
	s->l = size;
	s->s[size] = 0;
	return true;
}

static const wchar_t* carp_wstring_str(carp_wstring_t* s)
{
	static wchar_t s_empty[1] = { 0 };
	if (s->s == 0 && !carp_wstring_setlen(s, 0)) return s_empty;
	return s->s;
}

static size_t carp_wstring_len(carp_wstring_t* s)
{
	return s->l;
}

static void carp_wstring_free(carp_wstring_t* s)
{
	if (s->s) free(s->s);
	s->s = 0;
	s->l = s->m = 0;
}

static void carp_wstring_putsn(carp_wstring_t* s, const wchar_t* p, size_t l)
{
	if (!carp_wstring_setcap(s, s->l + l + 1)) return;

	memcpy(s->s + s->l, p, l * sizeof(wchar_t));
	s->l += l;
	s->s[s->l] = 0;
}

static void carp_wstring_puts(carp_wstring_t* s, const wchar_t* p)
{
	carp_wstring_putsn(s, p, wcslen(p));
}

static void carp_wstring_putc(carp_wstring_t* s, wchar_t c)
{
	if (!carp_wstring_setcap(s, s->l + 1)) return;
	s->s[s->l++] = c;
	s->s[s->l] = 0;
}

#endif
