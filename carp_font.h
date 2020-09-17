#ifndef CARP_FONT_INCLUDED
#define CARP_FONT_INCLUDED (1)

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

#define STB_DS_IMPLEMENTATION
#define STBDS_NO_SHORT_NAMES
#include "stb/stb_ds.h"

typedef struct _carp_font_t
{
	int font_size;
	int font_style;
	int font_height;
	int font_italic_extra_width;
	stbtt_fontinfo font;
	char* font_buffer;

	struct { unsigned int key; int value; }* unicode_map_glyphindex;

	float scale;
	int line_gap;
	int baseline;
} carp_font_t;

#define CARP_FONT_STYLE_NORMAL 0
#define CARP_FONT_STYLE_BOLD 1
#define CARP_FONT_STYLE_ITALIC 2
#define CARP_FONT_STYLE_UNDERLINE 4
#define CARP_FONT_STYLE_DELETELINE 8

/* x offset = cos(((90.0-12)/360) * 2 * M_PI), or 12 degree angle */
/* same value as in FT_GlyphSlot_Oblique, fixed point 16.16 */
#define CARP_FONT_GLYPH_ITALICS  0x0366AL

static carp_font_t* carp_font_load(const char* buffer, size_t len, int font_size, int font_style)
{
	font_size += 4;
	char* data = (char*)malloc(len);
	if (data == 0) return 0;

	carp_font_t* t = (carp_font_t*)malloc(sizeof(carp_font_t));
	if (t == 0)
	{
		free(data);
		return 0;
	}
	memset(t, 0, sizeof(carp_font_t));
	memcpy(data, buffer, len);
	
	if (stbtt_InitFont(&t->font, (unsigned char*)data, stbtt_GetFontOffsetForIndex((unsigned char*)buffer, 0)) == 0)
	{
		free(data);
		free(t);
		return 0;
	}

	t->font_buffer = data;
	t->font_size = font_size;
	t->font_style = font_style;
	t->scale = stbtt_ScaleForPixelHeight(&t->font, (float)font_size);
	int ascent, descent;
	stbtt_GetFontVMetrics(&t->font, &ascent, &descent, &t->line_gap);
	t->baseline = (int)(ascent * t->scale);
	t->font_height = (int)((ascent - descent) * t->scale);
	t->line_gap = (int)(t->line_gap * t->scale);

	if (font_style & CARP_FONT_STYLE_ITALIC) t->font_italic_extra_width = (CARP_FONT_GLYPH_ITALICS * t->font_height) >> 16;

	stbds_hmdefault(t->unicode_map_glyphindex, 0);
	return t;
}

static int carp_font_get_glyph_index(carp_font_t* font, unsigned int unicode_char)
{
	if (font == 0) return 0;
	int index = stbds_hmget(font->unicode_map_glyphindex, unicode_char);
	if (index == 0)
	{
		index = stbtt_FindGlyphIndex(&font->font, unicode_char);
		if (index == 0) return 0;
		stbds_hmput(font->unicode_map_glyphindex, unicode_char, index);
	}
	return index;
}

static int carp_font_height(carp_font_t* font)
{
	if (font == 0) return 0;
	return font->font_height;
}

static int carp_font_linegap(carp_font_t* font)
{
	if (font == 0) return 0;
	return font->line_gap;
}

static int carp_font_italic_extra_wdith(carp_font_t* font)
{
	if (font == 0) return 0;
	return font->font_italic_extra_width;
}

static void carp_font_free(carp_font_t* font)
{
	if (font == 0) return;
	free(font->font_buffer);
	stbds_hmfree(font->unicode_map_glyphindex);
	free(font);
}

static int carp_font_calc_wchar_width(carp_font_t* font, unsigned int unicode_char, unsigned int pre_char)
{
	if (font == 0) return 0;
	int index = stbds_hmget(font->unicode_map_glyphindex, unicode_char);
	if (index == 0)
	{
		index = stbtt_FindGlyphIndex(&font->font, unicode_char);
		if (index == 0) return 0;
		stbds_hmput(font->unicode_map_glyphindex, unicode_char, index);
	}
	int pre_index = 0;
	if (pre_char != 0)
	{
		pre_index = stbtt_FindGlyphIndex(&font->font, pre_char);
		if (pre_index != 0)	stbds_hmput(font->unicode_map_glyphindex, pre_char, pre_index);
	}

	int x0 = 0;
	int y0 = 0;
	int x1 = 0;
	int y1 = 0;
	stbtt_GetGlyphBitmapBox(&font->font, index, font->scale, font->scale, &x0, &y0, &x1, &y1);

	int width = x1 - x0;
	if (pre_index != 0) width += (int)(font->scale * stbtt_GetGlyphKernAdvance(&font->font, pre_index, index));
	
	return width;
}

static unsigned char* carp_font_create_bitmap(carp_font_t* font, unsigned int* unicode_char, size_t len, int* width, int* height)
{
	if (font == 0) return 0;

	int acc_width = 0;
	unsigned int pre_char = 0;
	for (size_t i = 0; i < len; ++i)
	{
		acc_width += carp_font_calc_wchar_width(font, unicode_char[i], pre_char);
		pre_char = unicode_char[i];
	}
	int acc_height = carp_font_height(font);
	if (acc_width == 0 || acc_height == 0) return 0;

	acc_width += carp_font_italic_extra_wdith(font);
	if (width) *width = acc_width;
	if (height) *height = acc_height;

	unsigned char* bitmap = (unsigned char*)malloc(acc_width * acc_height);
	if (bitmap == 0) return 0;
	memset(bitmap, 0, acc_width * acc_height);

	pre_char = 0;
	int tw = 0;
	for (size_t i = 0; i < len; ++i)
	{
		int index = carp_font_get_glyph_index(font, unicode_char[i]);
		int w = carp_font_calc_wchar_width(font, unicode_char[i], pre_char);
		int x0, y0, x1, y1;
		stbtt_GetGlyphBitmapBox(&font->font, index, font->scale, font->scale, &x0, &y0, &x1, &y1);
		int off_x = 0;
		int off_y = font->baseline + y0;
		stbtt_MakeGlyphBitmap(&font->font, bitmap + tw + off_x + off_y * acc_width, w - off_x, acc_height - off_y, acc_width, font->scale, font->scale, index);
		tw += w;
		pre_char = unicode_char[i];
	}

	return bitmap;
}

static void carp_font_release_bitmap(unsigned char* bitmap)
{
	free(bitmap);
}

#endif