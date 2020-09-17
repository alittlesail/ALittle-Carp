#ifndef CARP_FONT_INCLUDED
#define CARP_FONT_INCLUDED (1)

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

#define STB_DS_IMPLEMENTATION
#define STBDS_NO_SHORT_NAMES
#include "stb/stb_ds.h"

typedef struct
{
	int font_size;
	int font_style;
	stbtt_fontinfo font;

	float scale;
	int ascent, descent, line_gap;
	int baseline;
	unsigned char** bitmap_table;
} carp_font_t;

#define CARP_FONT_STYLE_NONE 0
#define CARP_FONT_STYLE_BOLD 1
#define CARP_FONT_STYLE_ITALIC 2
#define CARP_FONT_STYLE_UNDERLINE 4

static carp_font_t* carp_font_load(const char* buffer, int font_size, int font_style)
{
	carp_font_t* t = (carp_font_t*)malloc(sizeof(carp_font_t));
	if (t == 0) return 0;
	memset(t, 0, sizeof(carp_font_t));
	
	if (stbtt_InitFont(&t->font, (unsigned char*)buffer, stbtt_GetFontOffsetForIndex((unsigned char*)buffer, 0)) == 0)
	{
		free(t);
		return 0;
	}

	t->font_size = font_size;
	t->font_style = font_style;
	t->scale = stbtt_ScaleForPixelHeight(&t->font, font_size);
	stbtt_GetFontVMetrics(&t->font, &t->ascent, &t->descent, &t->line_gap);
	t->baseline = (int)(t->ascent * t->scale);
	return t;
}

#endif