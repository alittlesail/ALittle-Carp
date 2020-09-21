
#ifndef CARP_ARRAY_INCLUDED
#define CARP_ARRAY_INCLUDED

#include <stdlib.h>

// ʹ�÷���
/*
 * int* s = 0;	// һ��Ҫ��ʼ��Ϊ��ָ��
 * carp_array_push(s, 1);
 * carp_array_push(s, 2);
 * carp_array_pop(s);
 * size_t len = carp_array_len(s);
 * carp_array_free(s);
 */

typedef struct
{
	size_t l;
	size_t m;
} carp_array_header;

// �����������size������չ��size�����򲻱�
extern bool _carp_array_setcap(void** s, size_t type_size, size_t size);
#define carp_array_setcap(v, size) _carp_array_setcap(&(v), sizeof(*(v)), size)

// ���ó���
extern bool _carp_array_setlen(void** s, size_t type_size, size_t size);
#define carp_array_setlen(v, size) _carp_array_setlen(&(v), sizeof(*(v)), size)

// ��ȡ����
extern size_t carp_array_len(void* s);
#define carp_array_push(v, p) do { size_t index = carp_array_len(v); if (carp_array_setlen(v, index + 1)) v[index] = p; } while(false)

// β������
extern void carp_array_pop(void* s);

// �ͷ�
extern void _carp_array_free(void** s);
#define carp_array_free(v) _carp_array_free(&(v))

#ifdef CARP_ARRAY_IMPL

#define carp_array_roundup32(x) (--(x), (x)|=(x)>>1, (x)|=(x)>>2, (x)|=(x)>>4, (x)|=(x)>>8, (x)|=(x)>>16, ++(x))

// �����������size������չ��size�����򲻱�
static bool _carp_array_setcap(void** s, size_t type_size, size_t size)
{
	// ����ǿ�ָ�룬��ô��ֱ������ؼ�
	if (*s == 0)
	{
		carp_array_roundup32(size);
		carp_array_header* h = (carp_array_header*)malloc(sizeof(carp_array_header) + type_size * size);
		if (h == 0) return false;
		*s = h + 1;
		h->l = 0;
		h->m = size;
		return true;
	}
	
	carp_array_header* h = (carp_array_header*)(*s) - 1;
	if (h->m >= size) return true;

	carp_array_roundup32(size);
	carp_array_header* tmp = (carp_array_header*)realloc(h, sizeof(carp_array_header) + type_size * size);
	if (tmp == 0) return false;

	h = tmp;
	h->m = size;
	*s = h + 1;
	return true;
}

// ���ó���
static bool _carp_array_setlen(void** s, size_t type_size, size_t size)
{
	if (!_carp_array_setcap(s, type_size, size)) return false;

	carp_array_header* h = (carp_array_header*)(*s) - 1;
	h->l = size;
	return true;
}

static size_t carp_array_len(void* s)
{
	if (s == 0) return 0;
	carp_array_header* h = (carp_array_header*)s - 1;
	return h->l;
}

static void carp_array_pop(void* s)
{
	if (s == 0) return;

	carp_array_header* h = (carp_array_header*)s - 1;
	if (h->l <= 0) return;
	h->l--;
}

static void _carp_array_free(void** s)
{
	if (*s == 0) return;
	carp_array_header* h = (carp_array_header*)(*s) - 1;
	free(h);
	*s = 0;
}

#endif
#endif
