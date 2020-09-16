
#ifndef _carp_STRING_INCLUDED
#define _carp_STRING_INCLUDED

#include <stdlib.h>
#include <string.h>

#define _carp_string_roundup32(x) (--(x), (x)|=(x)>>1, (x)|=(x)>>2, (x)|=(x)>>4, (x)|=(x)>>8, (x)|=(x)>>16, ++(x))

// 使用范例
/*
 * carp_string_t s = {0}; // 内部一定要初始化为0
 * carp_string_putc(s, 'a');
 * carp_string_puts(s, "asd");
 * carp_string_setlen(s, 2);
 * carp_string_free(s);
 */

typedef struct {
	size_t l;	// 字符串长度
	size_t m;	// 字符串容量
	char *s;	// 字符串首地址
} carp_string_t;

// 如果容量不足size，就扩展到size，否则不变
static bool _carp_string_setcap(carp_string_t* s, size_t size)
{
	if (s->m >= size) return true;

	_carp_string_roundup32(size);	
	char* tmp = (char*)realloc(s->s, size);
	if (tmp == 0) return false;
	
	s->m = size;
	s->s = tmp;
	return true;
}

// 设置字符串长度
static bool _carp_string_setlen(carp_string_t* s, size_t size)
{
	if (!_carp_string_setcap(s, size + 1)) return false;
	s->l = size;
	s->s[size] = 0;
	return true;
}


static const char* _carp_string_str(carp_string_t* s)
{
	static char s_empty[1] = { 0 };
	if (s->s == 0 && !_carp_string_setlen(s, 0)) return s_empty;
	return s->s;
}

static size_t _carp_string_len(carp_string_t* s)
{
	return s->l;
}

static void _carp_string_free(carp_string_t* s)
{
	if (s->s) free(s->s);
	s->s = 0;
	s->l = s->m = 0;
}

static void _carp_string_putsn(carp_string_t* s, const char* p, size_t l)
{
	if (!_carp_string_setcap(s, s->l + l + 1)) return;
	
	memcpy(s->s + s->l, p, l);
	s->l += l;
	s->s[s->l] = 0;
}

static void _carp_string_puts(carp_string_t* s, const char *p)
{
	_carp_string_putsn(s, p, strlen(p));
}

static void _carp_string_putc(carp_string_t* s, char c)
{
	if (!_carp_string_setcap(s, s->l + 1)) return;
	s->s[s->l++] = c;
	s->s[s->l] = 0;
}

static void _carp_string_assigns(carp_string_t* s, const char* p)
{
	_carp_string_setlen(s, 0);
	_carp_string_puts(s, p);
}

static void _carp_string_assignsn(carp_string_t* s, const char* p, size_t l)
{
	_carp_string_setlen(s, 0);
	_carp_string_putsn(s, p, l);
}

#define carp_string_npos ((size_t)(-1))

static size_t _carp_string_findc(carp_string_t* s, char c)
{
	if (s->s == 0) return carp_string_npos;
	for (size_t i = 0; i < s->l; ++i)
	{
		if (s->s[i] == c)
			return i;
	}
	return carp_string_npos;
}

static size_t _carp_string_finds(carp_string_t* s, const char* p)
{
	if (s->s == 0) return carp_string_npos;
	char* t = strstr(s->s, p);
	if (t == 0) return carp_string_npos;
	return t - s->s;
}

#define carp_string_setlen(s, size) _carp_string_setlen(&(s), size)
#define carp_string_str(s) _carp_string_str(&(s))
#define carp_string_len(s) _carp_string_len(&(s))
#define carp_string_free(s) _carp_string_free(&(s))
#define carp_string_putsn(s, p, l) _carp_string_putsn(&(s), p, l)
#define carp_string_puts(s, p) _carp_string_puts(&(s), p)
#define carp_string_putc(s, c) _carp_string_putc(&(s), c)
#define carp_string_assigns(s, p) _carp_string_assigns(&(s), p)
#define carp_string_assignsn(s, p, l) _carp_string_assignsn(&(s), p, l)
#define carp_string_findc(s, c) _carp_string_findc(&(s), c)
#define carp_string_finds(s, p) _carp_string_finds(&(s), p)

typedef struct {
	size_t l;	// 字符串长度
	size_t m;	// 字符串容量
	wchar_t* s;	// 字符串首地址
} carp_wstring_t;

// 如果容量不足size，就扩展到size，否则不变
static bool _carp_wstring_setcap(carp_wstring_t* s, size_t size)
{
	if (s->m >= size) return true;

	_carp_string_roundup32(size);
	wchar_t* tmp = (wchar_t*)realloc(s->s, size * sizeof(wchar_t));
	if (tmp == 0) return false;
	
	s->m = size;
	s->s = tmp;
	return true;
}

// 设置字符串长度
static bool _carp_wstring_setlen(carp_wstring_t* s, size_t size)
{
	if (!_carp_wstring_setcap(s, size + 1)) return false;
	s->l = size;
	s->s[size] = 0;
	return true;
}

static const wchar_t* _carp_wstring_str(carp_wstring_t* s)
{
	static wchar_t s_empty[1] = { 0 };
	if (s->s == 0 && !_carp_wstring_setlen(s, 0)) return s_empty;
	return s->s;
}

static size_t _carp_wstring_len(carp_wstring_t* s)
{
	return s->l;
}

static void _carp_wstring_free(carp_wstring_t* s)
{
	if (s->s) free(s->s);
	s->s = 0;
	s->l = s->m = 0;
}

static void _carp_wstring_putsn(carp_wstring_t* s, const wchar_t* p, size_t l)
{
	if (!_carp_wstring_setcap(s, s->l + l + 1)) return;

	memcpy(s->s + s->l, p, l * sizeof(wchar_t));
	s->l += l;
	s->s[s->l] = 0;
}

static void _carp_wstring_puts(carp_wstring_t* s, const wchar_t* p)
{
	_carp_wstring_putsn(s, p, wcslen(p));
}

static void _carp_wstring_putc(carp_wstring_t* s, wchar_t c)
{
	if (!_carp_wstring_setcap(s, s->l + 1)) return;
	s->s[s->l++] = c;
	s->s[s->l] = 0;
}

#define _carp_wstring_npos ((size_t)(-1))

static size_t _carp_wstring_findc(carp_wstring_t* s, wchar_t c)
{
	if (s->s == 0) return _carp_wstring_npos;
	for (size_t i = 0; i < s->l; ++i)
	{
		if (s->s[i] == c)
			return i;
	}
	return _carp_wstring_npos;
}

static size_t _carp_wstring_finds(carp_wstring_t* s, const wchar_t* p)
{
	if (s->s == 0) return _carp_wstring_npos;
	wchar_t* t = wcsstr(s->s, p);
	if (t == 0) return _carp_wstring_npos;
	return t - s->s;
}

static void _carp_wstring_assigns(carp_wstring_t* s, const wchar_t* p)
{
	_carp_wstring_setlen(s, 0);
	_carp_wstring_puts(s, p);
}

static void _carp_wstring_assignsn(carp_wstring_t* s, const wchar_t* p, size_t l)
{
	_carp_wstring_setlen(s, 0);
	_carp_wstring_putsn(s, p, l);
}

#define carp_wstring_setlen(s, size) _carp_wstring_setlen(&(s), size)
#define carp_wstring_str(s) _carp_wstring_str(&(s))
#define carp_wstring_len(s) _carp_wstring_len(&(s))
#define carp_wstring_free(s) _carp_wstring_free(&(s))
#define carp_wstring_putsn(s, p, l) _carp_wstring_putsn(&(s), p, l)
#define carp_wstring_puts(s, p) _carp_wstring_puts(&(s), p)
#define carp_wstring_putc(s, c) _carp_wstring_putc(&(s), c)
#define carp_wstring_assigns(s, p) _carp_wstring_assigns(&(s), p)
#define carp_wstring_assignsn(s, p, l) _carp_wstring_assignsn(&(s), p, l)
#define carp_wstring_findc(s, c) _carp_wstring_findc(&(s), c)
#define carp_wstring_finds(s, p) _carp_wstring_finds(&(s), p)

#endif
