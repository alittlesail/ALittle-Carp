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
	const char* font_buffer;

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

#define CARP_FONT_LINE_THICKNESS 33

static carp_font_t* carp_font_load(const char* buffer, size_t len, int font_size, int font_style)
{
	font_size += 5;
	
	carp_font_t* t = (carp_font_t*)malloc(sizeof(carp_font_t));
	if (t == 0) return 0;
	memset(t, 0, sizeof(carp_font_t));
	
	if (stbtt_InitFont(&t->font, (unsigned char*)buffer, stbtt_GetFontOffsetForIndex((unsigned char*)buffer, 0)) == 0)
	{
		free(t);
		return 0;
	}

	t->font_buffer = buffer;
	t->font_size = font_size;
	t->font_style = font_style;
	t->scale = stbtt_ScaleForPixelHeight(&t->font, (float)font_size);
	int ascent, descent;
	stbtt_GetFontVMetrics(&t->font, &ascent, &descent, &t->line_gap);
	t->baseline = (int)(ascent * t->scale);
	t->font_height = (int)((ascent - descent) * t->scale);
	t->line_gap = (int)(t->line_gap * t->scale);

	if (t->font_style & CARP_FONT_STYLE_ITALIC) t->font_italic_extra_width = (CARP_FONT_GLYPH_ITALICS * t->font_height) >> 16;

	stbds_hmdefault(t->unicode_map_glyphindex, 0);
	return t;
}

static void carp_font_free(carp_font_t* font)
{
	if (font == 0) return;
	stbds_hmfree(font->unicode_map_glyphindex);
	free(font);
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

static int carp_font_calc_wchar_width(carp_font_t* font, unsigned int unicode_char, unsigned int pre_char)
{
	if (font == 0) return 0;
	int index = carp_font_get_glyph_index(font, unicode_char);
	if (index == 0) return 0;

	int pre_index = 0;
	if (pre_char != 0) pre_index = carp_font_get_glyph_index(font, pre_char);

	int x0 = 0;
	int y0 = 0;
	int x1 = 0;
	int y1 = 0;
	stbtt_GetGlyphBitmapBox(&font->font, index, font->scale, font->scale, &x0, &y0, &x1, &y1);

	int width = x1;
	if (pre_index != 0) width += (int)(font->scale * stbtt_GetGlyphKernAdvance(&font->font, pre_index, index));
	
	return width;
}

static void _carp_font_draw_underline(carp_font_t* font, unsigned char* bitmap, int width, int height)
{
	int start = font->baseline + (int)(CARP_FONT_LINE_THICKNESS * font->scale);
	int end = start + (int)(CARP_FONT_LINE_THICKNESS * font->scale);
	if (end <= start) end = start + 1;
	if (end >= height)
	{
		start = height - 1;
		end = height;
	}
	for (int row = start; row < end; ++row)
	{
		int offset = row * width;
		for (int col = 0; col < width; ++col)
			bitmap[offset + col] = 255;
	}
}

static void _carp_font_draw_italic(carp_font_t* font, unsigned char* bitmap, int width, int height)
{
	for (int row = 0; row < height; ++row)
	{
		int offx = ((height - row) * CARP_FONT_GLYPH_ITALICS) >> 16;
		if (offx == 0) continue;

		int offset = row * width;
		for (int col = width - 1; col >= 0; --col)
		{
			unsigned char value = 0;
			if (col - offx >= 0)
				value = bitmap[offset + col - offx];
			bitmap[offset + col] = value;
		}
	}
}

static void _carp_font_draw_bold(carp_font_t* font, unsigned char* bitmap, int width, int height)
{
	int overhang = (int)(CARP_FONT_LINE_THICKNESS * font->scale);
	for (int row = height - 1; row >= 0; --row) {
		unsigned char* pixmap = bitmap + row * width;
		for (int offset = 1; offset <= overhang; ++offset) {
			for (int col = width - 1; col > 0; --col) {
				int pixel = (pixmap[col] + pixmap[col - 1]);
				if (pixel > 255) pixel = 255;
				pixmap[col] = (unsigned char)pixel;
			}
		}
	}
}

static void _carp_font_draw_deleteline(carp_font_t* font, unsigned char* bitmap, int width, int height)
{
	int start = (height + (int)(CARP_FONT_LINE_THICKNESS * font->scale)) / 2;
	int end = start + (int)(CARP_FONT_LINE_THICKNESS * font->scale);
	if (end <= start) end = start + 1;
	if (end >= height)
	{
		start = height - 1;
		end = height;
	}
	for (int row = start; row < end; ++row)
	{
		int offset = row * width;
		for (int col = 0; col < width; ++col)
			bitmap[offset + col] = 255;
	}
}

static unsigned char* carp_font_create_bitmap(carp_font_t* font, unsigned int* unicode_char, size_t len, int* width, int* height)
{
	if (font == 0) return 0;

	int acc_width = 0;
	int pre_index = 0;
	for (size_t i = 0; i < len; ++i)
	{
		int index = carp_font_get_glyph_index(font, unicode_char[i]);
		int x0, y0, x1, y1;
		stbtt_GetGlyphBitmapBox(&font->font, index, font->scale, font->scale, &x0, &y0, &x1, &y1);
		if (pre_index != 0)
		{
			int kern = (int)(font->scale * stbtt_GetGlyphKernAdvance(&font->font, pre_index, index));
			x0 += kern;
			x1 += kern;
		}
		acc_width += x1;
		pre_index = index;
	}

	int acc_height = carp_font_height(font);
	if (acc_width == 0 || acc_height == 0) return 0;

	acc_width += carp_font_italic_extra_wdith(font);
	if (width) *width = acc_width;
	if (height) *height = acc_height;

	unsigned char* bitmap = (unsigned char*)malloc(acc_width * acc_height);
	if (bitmap == 0) return 0;
	memset(bitmap, 0, acc_width * acc_height);

	pre_index = 0;
	int tw = 0;
	for (size_t i = 0; i < len; ++i)
	{
		int index = carp_font_get_glyph_index(font, unicode_char[i]);
		int x0, y0, x1, y1;
		stbtt_GetGlyphBitmapBox(&font->font, index, font->scale, font->scale, &x0, &y0, &x1, &y1);
		if (pre_index != 0)
		{
			int kern = (int)(font->scale * stbtt_GetGlyphKernAdvance(&font->font, pre_index, index));
			x0 += kern;
			x1 += kern;
		}
		int w = x1;
		int off_x = x0;
		int off_y = font->baseline + y0;
		stbtt_MakeGlyphBitmap(&font->font, bitmap + tw + off_x + off_y * acc_width, w - off_x, acc_height - off_y, acc_width, font->scale, font->scale, index);
		tw += w;
		pre_index = index;
	}

	// italic
	if (font->font_style & CARP_FONT_STYLE_ITALIC) _carp_font_draw_italic(font, bitmap, acc_width, acc_height);

	// bold
	if (font->font_style & CARP_FONT_STYLE_BOLD) _carp_font_draw_bold(font, bitmap, acc_width, acc_height);

	// underline
	if (font->font_style & CARP_FONT_STYLE_UNDERLINE) _carp_font_draw_underline(font, bitmap, acc_width, acc_height);

	// deleteline
	if (font->font_style & CARP_FONT_STYLE_DELETELINE) _carp_font_draw_deleteline(font, bitmap, acc_width, acc_height);

	return bitmap;
}

static void carp_font_release_bitmap(unsigned char* bitmap)
{
	free(bitmap);
}

#endif