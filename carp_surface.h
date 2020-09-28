#ifndef CARP_SURFACE_INCLUDED
#define CARP_SURFACE_INCLUDED (1)

#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif

// ͼƬ���ش�С�̶�Ϊ4���ֽڣ���ʽΪARGB
typedef struct CARP_Surface
{
	int w;					// ͼƬ����
	int h;					// ͼƬ�߶�
	unsigned int* pixels;	// ��������
} CARP_Surface;

typedef struct CARP_SurfaceRect
{
	int x;
	int y;
	int w;
	int h;
} CARP_SurfaceRect;

extern CARP_Surface* Carp_CreateSurface(int width, int height);
extern void Carp_FreeSurface(CARP_Surface* surface);
extern void Carp_CopySurface(CARP_Surface* src, const CARP_SurfaceRect* src_rect, CARP_Surface* dst, int x, int y);
extern void Carp_ScaleSurface(CARP_Surface* src, const CARP_SurfaceRect* src_rect, CARP_Surface* dst, const CARP_SurfaceRect* dst_rect);
extern unsigned int Carp_GetSurfacePixel(CARP_Surface* src, int x, int y);
extern void Carp_SetSurfacePixel(CARP_Surface* src, int x, int y, unsigned int pixel);
extern int Carp_GetSurfaceWidth(CARP_Surface* src);
extern int Carp_GetSurfaceHeight(CARP_Surface* src);

#ifdef __cplusplus
}
#endif
#endif

#ifndef CARP_SURFACE_IMPL

CARP_Surface* Carp_CreateSurface(int width, int height)
{
	if (width <= 0 || height <= 0) return 0;

	CARP_Surface* surface = (CARP_Surface*)malloc(sizeof(CARP_Surface));
	surface->w = width;
	surface->h = height;
	surface->pixels = (unsigned int*)malloc(width * height * sizeof(unsigned int));
	return surface;
}

void Carp_FreeSurface(CARP_Surface* surface)
{
	free(surface->pixels);
	free(surface);
}

int Carp_GetSurfaceWidth(CARP_Surface* src)
{
	return src->w;
}

int Carp_GetSurfaceHeight(CARP_Surface* src)
{
	return src->h;
}

void Carp_CopySurface(CARP_Surface* src, const CARP_SurfaceRect* src_rect, CARP_Surface* dst, int dst_x, int dst_y)
{
	if (src == 0 || dst == 0) return;
	
	int src_x = 0;
	int src_y = 0;
	int src_width = src->w;
	int src_height = src->h;
	if (src_rect)
	{
		src_x = src_rect->x;
		src_y = src_rect->y;
		src_width = src_rect->w;
		src_height = src_rect->h;
	}

	for (int row = 0; row < src_height; ++row)
	{
		const int src_row = row + src_y;
		if (src_row < 0) continue;
		if (src_row >= src->h) break;

		const int dst_row = row + dst_y;
		if (dst_row < 0) continue;
		if (dst_row >= dst->h) break;
		
		for (int col = 0; col < src_width; ++col)
		{
			const int src_col = col + src_x;
			if (src_col < 0) continue;
			if (src_col >= src->w) break;
			
			const int dst_col = col + dst_x;
			if (dst_col < 0) continue;
			if (dst_col >= dst->w) break;
			
			dst->pixels[dst_row * dst->w + dst_col] = src->pixels[src_row * src->w + src_col];
		}
	}
}

void Carp_ScaleSurface(CARP_Surface* src, const CARP_SurfaceRect* src_rect, CARP_Surface* dst, const CARP_SurfaceRect* dst_rect)
{
	if (src == 0 || dst == 0) return;

	int src_x = 0;
	int src_y = 0;
	int src_width = src->w;
	int src_height = src->h;
	if (src_rect)
	{
		src_x = src_rect->x;
		src_y = src_rect->y;
		src_width = src_rect->w;
		src_height = src_rect->h;
	}

	int dst_x = 0;
	int dst_y = 0;
	int dst_width = src->w;
	int dst_height = src->h;
	if (dst_rect)
	{
		dst_x = dst_rect->x;
		dst_y = dst_rect->y;
		dst_width = dst_rect->w;
		dst_height = dst_rect->h;
	}

	// �������һ����ô˵������Ҫ���ţ�ֱ�ӽ��п�������
	if (src_width == dst_width && src_height == dst_height)
	{
		Carp_CopySurface(src, src_rect, dst, dst_x, dst_y);
		return;
	}

	const float ratio_width = (float)src_width / (float)dst_width;
	const float ratio_height = (float)src_height / (float)dst_height;
	for (int row = 0; row < dst_height; ++row)
	{
		const int src_row = (int)(row * ratio_height) + src_y;
		if (src_row < 0) continue;
		if (src_row >= src->h) break;
		
		const int dst_row = row + dst_y;
		if (dst_row < 0) continue;
		if (dst_row >= dst->h) break;
		
		for (int col = 0; col < dst_width; ++col)
		{
			const int src_col = (int)(col * ratio_width) + src_x;
			if (src_col < 0) continue;
			if (src_col >= src->w) break;
			
			const int dst_col = col + dst_x;
			if (dst_col < 0) continue;
			if (dst_col >= dst->w) break;

			dst->pixels[dst_row * dst->w + dst_col] = src->pixels[src_row * src->w + src_col];
		}
	}
}

unsigned int Carp_GetSurfacePixel(CARP_Surface* src, int x, int y)
{
	if (x < 0 || y < 0 || x >= src->w || y >= src->h) return 0;
	return src->pixels[y * src->w + x];
}

void Carp_SetSurfacePixel(CARP_Surface* src, int x, int y, unsigned int pixel)
{
	if (x < 0 || y < 0 || x >= src->w || y >= src->h) return;
	src->pixels[y * src->w + x] = pixel;
}

#endif