#ifndef CARP_FONT_INCLUDED
#define CARP_FONT_INCLUDED (1)

#include <unordered_map>

#ifdef CARP_FONT_IMPL
#define STB_TRUETYPE_IMPLEMENTATION
#endif
#include "stb/stb_truetype.h"

#define CARP_FONT_STYLE_NORMAL 0
#define CARP_FONT_STYLE_BOLD 1
#define CARP_FONT_STYLE_ITALIC 2
#define CARP_FONT_STYLE_UNDERLINE 4
#define CARP_FONT_STYLE_DELETELINE 8

/* x offset = cos(((90.0-12)/360) * 2 * M_PI), or 12 degree angle */
/* same value as in FT_GlyphSlot_Oblique, fixed point 16.16 */
#define CARP_FONT_GLYPH_ITALICS  0x0366AL

#define CARP_FONT_LINE_THICKNESS 33

class CarpFontBitmap
{
public:
	CarpFontBitmap(int w, int h)
	{
		width = w;
		height = h;
		if (width > 0 && height > 0)
		{
			bitmap = (unsigned char*)malloc(width * height);
			memset(bitmap, 0, width * height);
		}
	}

	~CarpFontBitmap()
	{
		if (bitmap) free(bitmap);
	}

public:
	int width = 0;
	int height = 0;
	unsigned char* bitmap = 0;
};

class CarpFont
{
public:
	// buffer reference by CarpFont, do not free before delete CarpFont
	CarpFont(const char* buffer, size_t len, unsigned int font_size, unsigned int font_style)
	{
		memset(&m_font, 0, sizeof(m_font));
		
		font_size += 5;

		if (stbtt_InitFont(&m_font, (unsigned char*)buffer, stbtt_GetFontOffsetForIndex((unsigned char*)buffer, 0)) == 0)
			return;

		m_font_size = font_size;
		m_font_style = font_style;
		m_scale = stbtt_ScaleForPixelHeight(&m_font, (float)font_size);
		int ascent, descent;
		stbtt_GetFontVMetrics(&m_font, &ascent, &descent, &m_line_gap);
		m_base_line = (int)(ascent * m_scale);
		m_font_height = (int)((ascent - descent) * m_scale);
		m_line_gap = (int)(m_line_gap * m_scale);

		if (m_font_style & CARP_FONT_STYLE_ITALIC) m_italic_extra_width = (CARP_FONT_GLYPH_ITALICS * m_font_height) >> 16;
	}

public:
	int GetFontHeight() const { return m_font_height; }
	int GetLineGap() const { return m_line_gap; }
	int GetLineSkip() const { return m_font_height + m_line_gap; }
	int GetItalicExtraWidth() const { return m_italic_extra_width; }

	/**
	* cut text by width
	* @param content: total text(utf8)
	* @param width: remain width to cut
	* @param max_width: max width to cut
	* @return pos offset from content
	*/
	int CutTextByWidth(const char* content, int width, int max_width, std::vector<int>* list)
	{
		if (content == 0 || width <= 0) return 0;
		int len = (int)strlen(content);
		if (len == 0) return 0;

		int acc_width = GetItalicExtraWidth();
		unsigned int pre_char = 0;
		int char_count = 0;
		while (len > 0)
		{
			int inc = 0;
			unsigned int c = GetOneUnicodeFromUTF8(content, len, &inc);

			acc_width += CalcWCharWidth(c, pre_char);
			if (acc_width > width)
			{
				// 如果一个字符都没有切到，并且大于最大宽度，那么如果留到下一行依然无法切到
				// 所以这里干脆直接切掉
				if (char_count == 0 && acc_width > max_width)
				{
					if (list) list->push_back(acc_width);
					return inc;
				}
				break;
			}

			if (list) list->push_back(acc_width);
			pre_char = c;
			content += inc;
			len -= inc;
			char_count += inc;
		}

		return char_count;
	}

	/**
	* cut text width
	* @param content: total text(utf8)
	*/
	int CutTextWidth(const char* content)
	{
		if (content == 0) return 0;
		int len = (int)strlen(content);
		if (len == 0) return 0;

		int acc_width = GetItalicExtraWidth();
		unsigned int pre_char = 0;
		while (len > 0)
		{
			int inc = 0;
			unsigned int c = GetOneUnicodeFromUTF8(content, len, &inc);

			acc_width += CalcWCharWidth(c, pre_char);
			pre_char = c;
			content += inc;
			len -= inc;
		}

		return acc_width;
	}

	CarpFontBitmap* CreateBitmapFromUTF8(const char* content)
	{
		int len = (int)strlen(content);
		if (len == 0) return nullptr;
		
		std::vector<unsigned int> unicode_list;
		while (len > 0)
		{
			int inc = 0;
			unicode_list.push_back(GetOneUnicodeFromUTF8(content, len, &inc));
			content += inc;
			len -= inc;
		}

		return CreateBitmapFromUnicode(unicode_list.data(), unicode_list.size());
	}

	CarpFontBitmap* CreateBitmapFromUnicode(unsigned int* unicode_char, size_t len)
	{
		int acc_width = 0;
		int pre_index = 0;
		for (size_t i = 0; i < len; ++i)
		{
			int index = GetGlyphIndex(unicode_char[i]);
			int advance, lsb;
			stbtt_GetGlyphHMetrics(&m_font, index, &advance, &lsb);
			acc_width += (int)(advance * m_scale);
			if (pre_index != 0)
				acc_width += (int)(m_scale * stbtt_GetGlyphKernAdvance(&m_font, pre_index, index));
			pre_index = index;
		}

		int acc_height = m_font_height;
		if (acc_width == 0 || acc_height == 0) return 0;

		acc_width += m_italic_extra_width;
		CarpFontBitmap* carp_bitmap = new CarpFontBitmap(acc_width, acc_height);
		unsigned char* bitmap = carp_bitmap->bitmap;

		pre_index = 0;
		int tw = 0;
		for (size_t i = 0; i < len; ++i)
		{
			int index = GetGlyphIndex(unicode_char[i]);
			int advance, lsb;
			stbtt_GetGlyphHMetrics(&m_font, index, &advance, &lsb);
			int w = (int)(advance * m_scale);
			int x0, y0, x1, y1;
			stbtt_GetGlyphBitmapBox(&m_font, index, m_scale, m_scale, &x0, &y0, &x1, &y1);
			if (pre_index != 0)
			{
				int kern = (int)(m_scale * stbtt_GetGlyphKernAdvance(&m_font, pre_index, index));
				x0 += kern;
				x1 += kern;
				w += kern;
			}
			int off_x = x0;
			int off_y = m_base_line + y0;
			stbtt_MakeGlyphBitmap(&m_font, bitmap + tw + off_x + off_y * acc_width, w - off_x, acc_height - off_y, acc_width, m_scale, m_scale, index);
			tw += w;
			pre_index = index;
		}

		// italic
		if (m_font_style & CARP_FONT_STYLE_ITALIC) DrawItalic(bitmap, acc_width, acc_height);

		// bold
		if (m_font_style & CARP_FONT_STYLE_BOLD) DrawBold(bitmap, acc_width, acc_height);

		// underline
		if (m_font_style & CARP_FONT_STYLE_UNDERLINE) DrawUnderline(bitmap, acc_width, acc_height);

		// delete line
		if (m_font_style & CARP_FONT_STYLE_DELETELINE) DrawDeleteLine(bitmap, acc_width, acc_height);

		return carp_bitmap;
	}

private:
	int GetGlyphIndex(unsigned int unicode_char)
	{
		auto it = m_unicode_map_glyph_index.find(unicode_char);
		if (it != m_unicode_map_glyph_index.end()) return it->second;
		
		int index = stbtt_FindGlyphIndex(&m_font, unicode_char);
		if (index == 0) return 0;
		m_unicode_map_glyph_index[unicode_char] = index;
		return index;
	}

	int CalcWCharWidth(unsigned int unicode_char, unsigned int pre_char)
	{
		int index = GetGlyphIndex(unicode_char);
		if (index == 0) return 0;

		int pre_index = 0;
		if (pre_char != 0) pre_index = GetGlyphIndex(pre_char);

		int advance, lsb;
		stbtt_GetGlyphHMetrics(&m_font, index, &advance, &lsb);

		int width = (int)(advance * m_scale);
		if (pre_index != 0) width += (int)(m_scale * stbtt_GetGlyphKernAdvance(&m_font, pre_index, index));

		return width;
	}

private:
#define UNKNOWN_UNICODE 0xFFFD
	static unsigned int GetOneUnicodeFromUTF8(const char* src, size_t srclen, int* increase)
	{
		const unsigned char* p = (const unsigned char*)src;
		size_t left = 0;
		size_t save_srclen = srclen;
		int overlong = 0;
		int underflow = 0;
		unsigned int ch = UNKNOWN_UNICODE;

		if (srclen == 0) {
			return UNKNOWN_UNICODE;
		}
		if (p[0] >= 0xFC) {
			if ((p[0] & 0xFE) == 0xFC) {
				if (p[0] == 0xFC && (p[1] & 0xFC) == 0x80) {
					overlong = 1;
				}
				ch = (unsigned int)(p[0] & 0x01);
				left = 5;
			}
		}
		else if (p[0] >= 0xF8) {
			if ((p[0] & 0xFC) == 0xF8) {
				if (p[0] == 0xF8 && (p[1] & 0xF8) == 0x80) {
					overlong = 1;
				}
				ch = (unsigned int)(p[0] & 0x03);
				left = 4;
			}
		}
		else if (p[0] >= 0xF0) {
			if ((p[0] & 0xF8) == 0xF0) {
				if (p[0] == 0xF0 && (p[1] & 0xF0) == 0x80) {
					overlong = 1;
				}
				ch = (unsigned int)(p[0] & 0x07);
				left = 3;
			}
		}
		else if (p[0] >= 0xE0) {
			if ((p[0] & 0xF0) == 0xE0) {
				if (p[0] == 0xE0 && (p[1] & 0xE0) == 0x80) {
					overlong = 1;
				}
				ch = (unsigned int)(p[0] & 0x0F);
				left = 2;
			}
		}
		else if (p[0] >= 0xC0) {
			if ((p[0] & 0xE0) == 0xC0) {
				if ((p[0] & 0xDE) == 0xC0) {
					overlong = 1;
				}
				ch = (unsigned int)(p[0] & 0x1F);
				left = 1;
			}
		}
		else {
			if ((p[0] & 0x80) == 0x00) {
				ch = (unsigned int)p[0];
			}
		}
		--srclen;
		while (left > 0 && srclen > 0) {
			++p;
			if ((p[0] & 0xC0) != 0x80) {
				ch = UNKNOWN_UNICODE;
				break;
			}
			ch <<= 6;
			ch |= (p[0] & 0x3F);
			--srclen;
			--left;
		}
		if (left > 0) {
			underflow = 1;
		}
		/* Technically overlong sequences are invalid and should not be interpreted.
		   However, it doesn't cause a security risk here and I don't see any harm in
		   displaying them. The application is responsible for any other side effects
		   of allowing overlong sequences (e.g. string compares failing, etc.)
		   See bug 1931 for sample input that triggers this.
		*/
		/* if (overlong) return UNKNOWN_UNICODE; */

		(void)overlong;

		if (underflow ||
			(ch >= 0xD800 && ch <= 0xDFFF) ||
			(ch == 0xFFFE || ch == 0xFFFF) || ch > 0x10FFFF) {
			ch = UNKNOWN_UNICODE;
		}

		if (increase) *increase = (int)(save_srclen - srclen);

		return ch;
	}

private:
	void DrawUnderline(unsigned char* bitmap, int width, int height) const
	{
		int start = m_base_line + (int)(CARP_FONT_LINE_THICKNESS * m_scale);
		int end = start + (int)(CARP_FONT_LINE_THICKNESS * m_scale);
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

	void DrawItalic(unsigned char* bitmap, int width, int height) const
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

	void DrawBold(unsigned char* bitmap, int width, int height) const
	{
		int overhang = (int)(CARP_FONT_LINE_THICKNESS * m_scale);
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

	void DrawDeleteLine(unsigned char* bitmap, int width, int height) const
	{
		int start = (height + (int)(CARP_FONT_LINE_THICKNESS * m_scale)) / 2;
		int end = start + (int)(CARP_FONT_LINE_THICKNESS * m_scale);
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
	
private:
	unsigned int m_font_size = 0;
	unsigned int m_font_style = 0;
	int m_font_height = 0;
	int m_italic_extra_width = 0;
	stbtt_fontinfo m_font;

	std::unordered_map<int, int> m_unicode_map_glyph_index;

	float m_scale = 0.0f;
	int m_line_gap = 0;
	int m_base_line = 0;
};

#endif