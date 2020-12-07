#ifndef CARP_SURFACE_INCLUDED
#define CARP_SURFACE_INCLUDED

#include <memory>
#include <string>
#include <vector>
#include <cmath>

struct CarpSurfaceRect
{
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
};

// 图片像素大小固定为4个字节，格式为ARGB
class CarpSurface
{
public:
	CarpSurface(int width, int height)
	{
		if (width <= 0 || height <= 0) return;

		m_width = width;
		m_height = height;
		m_pixels.resize(width * height, 0);
	}

	void Reset(int width, int height, unsigned int color)
	{
		if (width <= 0 || height <= 0) return;

		m_width = width;
		m_height = height;
		m_pixels.resize(0);
		m_pixels.resize(width * height, color);
	}

	void Clear()
	{
		m_width = 0;
		m_height = 0;
		m_pixels.clear();
	}

	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }

	void CopyFrom(CarpSurface* src, const CarpSurfaceRect* src_rect, int dst_x, int dst_y)
	{
		if (src == nullptr) return;

		int src_x = 0;
		int src_y = 0;
		int src_width = src->m_width;
		int src_height = src->m_height;
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
			if (src_row >= src->m_height) break;

			const int dst_row = row + dst_y;
			if (dst_row < 0) continue;
			if (dst_row >= m_height) break;

			for (int col = 0; col < src_width; ++col)
			{
				const int src_col = col + src_x;
				if (src_col < 0) continue;
				if (src_col >= src->m_width) break;

				const int dst_col = col + dst_x;
				if (dst_col < 0) continue;
				if (dst_col >= m_width) break;

				m_pixels[dst_row * m_width + dst_col] = src->m_pixels[src_row * src->m_width + src_col];
			}
		}
	}

	unsigned int* GetPixels() { return m_pixels.data(); }
	unsigned int GetPitch() const { return m_width * sizeof(unsigned int); }

	unsigned int GetGray(size_t offset)
	{
		if (offset >= m_pixels.size()) return 0;

		unsigned int color = m_pixels[offset];
		unsigned int a = color >> 24;
		unsigned int r = (color & 0x00FF0000) >> 16;
		unsigned int g = (color & 0x0000FF00) >> 8;
		unsigned int b = color & 0x000000FF;
		return (unsigned int)std::floor((r + g + b) / 3.0 * (a / 255.0));
	}

	void ScaleFrom(CarpSurface* src, const CarpSurfaceRect* src_rect, const CarpSurfaceRect* dst_rect)
	{
		if (src == 0) return;

		int src_x = 0;
		int src_y = 0;
		int src_width = src->m_width;
		int src_height = src->m_height;
		if (src_rect)
		{
			src_x = src_rect->x;
			src_y = src_rect->y;
			src_width = src_rect->w;
			src_height = src_rect->h;
		}

		int dst_x = 0;
		int dst_y = 0;
		int dst_width = m_width;
		int dst_height = m_height;
		if (dst_rect)
		{
			dst_x = dst_rect->x;
			dst_y = dst_rect->y;
			dst_width = dst_rect->w;
			dst_height = dst_rect->h;
		}

		// 如果宽高一致那么说明不需要缩放，直接进行拷贝即可
		if (src_width == dst_width && src_height == dst_height)
		{
			CopyFrom(src, src_rect, dst_x, dst_y);
			return;
		}

		const float ratio_width = static_cast<float>(src_width) / static_cast<float>(dst_width);
		const float ratio_height = static_cast<float>(src_height) / static_cast<float>(dst_height);
		for (int row = 0; row < dst_height; ++row)
		{
			const int src_row = static_cast<int>(row * ratio_height) + src_y;
			if (src_row < 0) continue;
			if (src_row >= src->m_height) break;

			const int dst_row = row + dst_y;
			if (dst_row < 0) continue;
			if (dst_row >= m_height) break;

			for (int col = 0; col < dst_width; ++col)
			{
				const int src_col = static_cast<int>(col * ratio_width) + src_x;
				if (src_col < 0) continue;
				if (src_col >= src->m_width) break;

				const int dst_col = col + dst_x;
				if (dst_col < 0) continue;
				if (dst_col >= m_width) break;

				m_pixels[dst_row * m_width + dst_col] = src->m_pixels[src_row * src->m_width + src_col];
			}
		}
	}

	unsigned int GetPixel(int x, int y)
	{
		if (x < 0 || y < 0 || x >= m_width || y >= m_height) return 0;
		return m_pixels[y * m_width + x];
	}

	void SetPixel(int x, int y, unsigned int pixel)
	{
		if (x < 0 || y < 0 || x >= m_width || y >= m_height) return;
		m_pixels[y * m_width + x] = pixel;
	}

	inline void SetPixelByIndex(int index, unsigned int pixel)
	{
		if (index < 0 || index >= m_pixels.size()) return;
		m_pixels[index] = pixel;
	}

	void TransferPixel(const char* type, int step)
	{
		if (type == 0 || m_width == 0 || m_height == 0 || step <= 0) return;
		std::string new_type = type;

		if (new_type == "left")
		{
			if (step > m_width) step = m_width;
			
			for (int col = step; col < m_width; ++col)
			{
				for (int row = 0; row < m_height; ++row)
				{
					m_pixels[row * m_width + col - step] = m_pixels[row * m_width + col];
				}
			}

			for (int col = m_width - step; col < m_width; ++col)
			{
				for (int row = 0; row < m_height; ++row)
				{
					m_pixels[row * m_width + col] = 0;
				}
			}
		}
		else if (new_type == "right")
		{
			if (step > m_width) step = m_width;
			
			for (int col = m_width - 1; col >= step; ++col)
			{
				for (int row = 0; row < m_height; ++row)
				{
					m_pixels[row * m_width + col] = m_pixels[row * m_width + col - step];
				}
			}

			for (int col = 0; col < step; ++col)
			{
				for (int row = 0; row < m_height; ++row)
				{
					m_pixels[row * m_width + col] = 0;
				}
			}
		}
		else if (new_type == "top")
		{
			if (step > m_height) step = m_height;

			for (int row = step; row < m_height; ++row)
			{
				for (int col = 0; col < m_width; ++col)
				{
					m_pixels[(row - step) * m_width + col] = m_pixels[row * m_width + col];
				}
			}

			for (int row = m_height - step; row < m_height; ++row)
			{
				for (int col = 0; col < m_width; ++col)
				{
					m_pixels[row * m_width + col] = 0;
				}
			}
		}
		else if (new_type == "bottom")
		{
			if (step > m_height) step = m_height;

			for (int row = m_height - 1; row >= step; ++row)
			{
				for (int col = 0; col < m_width; ++col)
				{
					m_pixels[row * m_width + col] = m_pixels[(row - step) * m_width + col];
				}
			}

			for (int row = 0; row < step; ++row)
			{
				for (int col = 0; col < m_width; ++col)
				{
					m_pixels[row * m_width + col] = 0;
				}
			}
		}
	}

	unsigned int GetGrid9(const char* type)
	{
		if (type == 0 || m_width == 0 || m_height == 0) return 0;

		std::string new_type = type;

		if (new_type == "left")
		{
			int col = m_width / 2;
			while (col)
			{
				for (int row = 0; row < m_height; ++row)
				{
					if (m_pixels[row * m_width + col] != m_pixels[row * m_width + col - 1])
						return col;
				}
				--col;
			}
			return col;
		}

		if (new_type == "right")
		{
			int col = m_width / 2;
			while (col + 1 < m_width)
			{
				for (int row = 0; row < m_height; ++row)
				{
					if (m_pixels[row * m_width + col] != m_pixels[row * m_width + col + 1])
						return col;
				}
				++col;
			}
			return col;
		}

		if (new_type == "top")
		{
			int row = m_height / 2;
			while (row)
			{
				for (int col = 0; col < m_width; ++col)
				{
					if (m_pixels[row * m_width + col] != m_pixels[(row - 1) * m_width + col])
						return row;
				}
				--row;
			}
			return row;
		}

		if (new_type == "bottom")
		{
			int row = m_height / 2;
			while (row + 1 < m_height)
			{
				for (int col = 0; col < m_width; ++col)
				{
					if (m_pixels[row * m_width + col] != m_pixels[(row + 1) * m_width + col])
						return row;
				}
				++row;
			}
			return row;
		}

		return 0;
	}
	
private:
	int m_width = 0;					// 图片宽度
	int m_height = 0;					// 图片高度
	std::vector<unsigned int> m_pixels;	// 像素内容
};

#endif