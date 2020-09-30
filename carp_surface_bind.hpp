#ifndef CARP_SURFACE_BIND_INCLUDED
#define CARP_SURFACE_BIND_INCLUDED

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include "carp_surface.h"
#include "carp_lua.hpp"
#include "carp_rwops.hpp"
#include "carp_string.hpp"

class CarpSurfaceBind
{
public:
	static void Bind(lua_State* l_state)
	{
		luabridge::getGlobalNamespace(l_state)
			.beginNamespace("carp")
			.beginClass<Carp_Surface>("CarpSurface")
			.endClass()

			.addFunction("__CPPAPI_CreateSurface", Carp_CreateSurface)
			.addFunction("__CPPAPI_FreeSurface", Carp_FreeSurface)
			.addFunction("__CPPAPI_BlitSurface", BlitSurface)
			.addFunction("__CPPAPI_LoadSurface", LoadSurfaceForLua)
			.addFunction("__CPPAPI_SaveSurface", SaveSurface)
			.addFunction("__CPPAPI_GetSurfaceWidth", GetSurfaceWidth)
			.addFunction("__CPPAPI_GetSurfaceHeight", GetSurfaceHeight)
			.addFunction("__CPPAPI_CutBlitSurface", CutBlitSurface)
			.addFunction("__CPPAPI_GetSurfaceGrid9", GetSurfaceGrid9)
			.addFunction("__CPPAPI_GetSurfacePixel", Carp_GetSurfacePixel)
			.addFunction("__CPPAPI_SetSurfacePixel", Carp_SetSurfacePixel)
			.addFunction("__CPPAPI_GetPixelAlpha", GetPixelAlpha)
			.addFunction("__CPPAPI_SetPixelAlpha", SetPixelAlpha)
			.addFunction("__CPPAPI_GetPixelRed", GetPixelRed)
			.addFunction("__CPPAPI_SetPixelRed", SetPixelRed)
			.addFunction("__CPPAPI_GetPixelGreen", GetPixelGreen)
			.addFunction("__CPPAPI_SetPixelGreen", SetPixelGreen)
			.addFunction("__CPPAPI_GetPixelBlue", GetPixelBlue)
			.addFunction("__CPPAPI_SetPixelBlue", SetPixelBlue)
		
			.endNamespace();
	}

public:
	/**
	 * load surface
	 * @param file_path
	 * @return surface
	 */
	static Carp_Surface* LoadSurfaceForLua(const char* file_path) { return LoadSurface(file_path); }
	static Carp_Surface* LoadSurface(const char* file_path)
	{
		CarpLocalFile file;
		file.SetPath(file_path);
		if (!file.Load(false)) return nullptr;
		return LoadSurface(file);
	}

	static Carp_Surface* LoadSurface(const CarpLocalFile& file)
	{
		int width = 0;
		int height = 0;
		int comp = 0;
		stbi_uc* uc = stbi_load_from_memory((unsigned char*)file.GetContent(), (int)file.GetSize(), &width, &height, &comp, 4);
		if (uc == nullptr) return nullptr;
		if (comp != 4)
		{
			stbi_image_free(uc);
			return nullptr;
		}
		Carp_Surface* surface = Carp_CreateSurface(width, height);
		if (surface) memcpy(surface->pixels, uc, width * height * comp);
		stbi_image_free(uc);
		return surface;
	}

	static bool SaveSurface(Carp_Surface* surface, const char* file_path)
	{
		return stbi_write_png(file_path, surface->w, surface->h, 4, surface->pixels, surface->w * 4) != 0;
	}

	/**
	 * get surface width
	 * @return width
	 */
	static unsigned int GetSurfaceWidth(Carp_Surface* surface) { return surface->w; }

	/**
	 * get surface height
	 * @return height
	 */
	static unsigned int GetSurfaceHeight(Carp_Surface* surface) { return surface->h; }

	static void	BlitSurface(Carp_Surface* dest, Carp_Surface* src, unsigned int x, unsigned int y)
	{
		Carp_CopySurface(src, nullptr, dest, x, y);
	}

	/**
	 * copy surface
	 * @param dest
	 * @param src
	 * @param from
	 * @param to
	 */
	static bool	CutBlitSurface(Carp_Surface* dest, Carp_Surface* src, const char* to, const char* from)
	{
		std::vector<std::string> to_list;
		CarpString::Split(to, ",", to_list);
		if (to_list.size() < 4) return false;

		std::vector<std::string> from_list;
		CarpString::Split(from, ",", from_list);
		if (from_list.size() < 4) return false;

		Carp_SurfaceRect dst_rect;
		dst_rect.x = atoi(to_list[0].c_str());
		dst_rect.y = atoi(to_list[1].c_str());
		dst_rect.w = atoi(to_list[2].c_str());
		dst_rect.h = atoi(to_list[3].c_str());

		Carp_SurfaceRect src_rect;
		src_rect.x = atoi(from_list[0].c_str());
		src_rect.y = atoi(from_list[1].c_str());
		src_rect.w = atoi(from_list[2].c_str());
		src_rect.h = atoi(from_list[3].c_str());

		Carp_ScaleSurface(src, &src_rect, dest, &dst_rect);
		return true;
	}

	/**
	 * Get scratchable latex four edges
	 * @param surface
	 * @param type left, right, top, bottom
	 * @return edge size
	 */
	static unsigned int GetSurfaceGrid9(Carp_Surface* surface, const char* type)
	{
		if (surface == 0 || type == 0 || surface->w == 0 || surface->h == 0) return 0;

		std::string new_type = type;

		if (new_type == "left")
		{
			int col = surface->w / 2;
			while (col)
			{
				for (int row = 0; row < surface->h; ++row)
				{
					if (surface->pixels[row * surface->w + col] != surface->pixels[row * surface->w + col - 1])
						return col;
				}
				--col;
			}
			return col;
		}

		if (new_type == "right")
		{
			int col = surface->w / 2;
			while (col + 1 < surface->w)
			{
				for (int row = 0; row < surface->h; ++row)
				{
					if (surface->pixels[row * surface->w + col] != surface->pixels[row * surface->w + col + 1])
						return col;
				}
				++col;
			}
			return col;
		}

		if (new_type == "top")
		{
			int row = surface->h / 2;
			while (row)
			{
				for (int col = 0; col < surface->w; ++col)
				{
					if (surface->pixels[row * surface->w + col] != surface->pixels[(row - 1) * surface->w + col])
						return row;
				}
				--row;
			}
			return row;
		}

		if (new_type == "bottom")
		{
			int row = surface->h / 2;
			while (row + 1 < surface->h)
			{
				for (int col = 0; col < surface->w; ++col)
				{
					if (surface->pixels[row * surface->w + col] != surface->pixels[(row + 1) * surface->w + col])
						return row;
				}
				++row;
			}
			return row;
		}

		return 0;
	}

	static unsigned int GetPixelAlpha(unsigned int color) { return color >> 24; }
	static unsigned int SetPixelAlpha(unsigned int color, unsigned int alpha) { return (color & 0x00ffffff) | (alpha << 24); }
	static unsigned int GetPixelRed(unsigned int color) { return color & 0x000000ff; }
	static unsigned int SetPixelRed(unsigned int color, unsigned int red) { return (color & 0xffffff00) | red; }
	static unsigned int GetPixelGreen(unsigned int color) { return (color & 0x0000ff00) >> 8; }
	static unsigned int SetPixelGreen(unsigned int color, unsigned int green) { return (color & 0xffff00ff) | (green << 8); }
	static unsigned int GetPixelBlue(unsigned int color) { return (color & 0x00ff0000) >> 16; }
	static unsigned int SetPixelBlue(unsigned int color, unsigned int blue) { return (color & 0xff00ffff) | (blue << 16); }
};

#endif

#ifdef CARP_SURFACE_IMPL
#ifndef CARP_SURFACE_IMPL_INCLUDE
#define CARP_SURFACE_IMPL_INCLUDE
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#endif
#endif