#ifndef CARP_SURFACE_BIND_INCLUDED
#define CARP_SURFACE_BIND_INCLUDED

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include "carp_surface.hpp"
#include "carp_lua.hpp"
#include "carp_rwops_bind.hpp"
#include "carp_string.hpp"

class CarpSurfaceBind
{
public:
	static void Bind(lua_State* l_state)
	{
		luabridge::getGlobalNamespace(l_state)
			.beginNamespace("carp")
			.beginClass<CarpSurface>("CarpSurface")
			.endClass()

			.addFunction("CreateCarpSurface", CreateCarpSurface)
			.addFunction("GetCarpSurfaceAddress", GetCarpSurfaceAddress)
			.addFunction("FreeCarpSurface", FreeCarpSurface)
			.addFunction("BlitCarpSurface", BlitCarpSurface)
			.addFunction("LoadCarpSurface", LoadCarpSurfaceForLua)
			.addFunction("SaveCarpSurface", SaveCarpSurface)
			.addFunction("GetCarpSurfaceWidth", GetCarpSurfaceWidth)
			.addFunction("GetCarpSurfaceHeight", GetCarpSurfaceHeight)
			.addFunction("CutBlitCarpSurface", CutBlitCarpSurface)
			.addFunction("TransferCarpSurface", TransferCarpSurface)
			.addFunction("GetCarpSurfaceGrid9", GetCarpSurfaceGrid9)
			.addFunction("GetCarpSurfacePixel", GetCarpSurfacePixel)
			.addFunction("SetCarpSurfacePixel", SetCarpSurfacePixel)
			.addCFunction("SetCarpSurfacePixelRect", SetCarpSurfacePixelRect)
			.addFunction("GetPixelAlpha", GetPixelAlpha)
			.addFunction("SetPixelAlpha", SetPixelAlpha)
			.addFunction("GetPixelRed", GetPixelRed)
			.addFunction("SetPixelRed", SetPixelRed)
			.addFunction("GetPixelGreen", GetPixelGreen)
			.addFunction("SetPixelGreen", SetPixelGreen)
			.addFunction("GetPixelBlue", GetPixelBlue)
			.addFunction("SetPixelBlue", SetPixelBlue)
		
			.endNamespace();
	}

public:
	static CarpSurface* CreateCarpSurface(int width, int height)
	{
		if (width <= 0 || height <= 0) return nullptr;
		return new CarpSurface(width, height);
	}

	static size_t GetCarpSurfaceAddress(CarpSurface* surface)
	{
		size_t address = 0;
		memcpy(&address, &surface, sizeof(size_t));
		return address;
	}

	static void FreeCarpSurface(CarpSurface* surface)
	{
		delete surface;
	}
	
	static CarpSurface* LoadCarpSurfaceForLua(const char* file_path) { return LoadSurface(file_path); }
	static CarpSurface* LoadSurface(const char* file_path)
	{
		CarpLocalFile file;
		file.SetPath(file_path);
		if (!file.Load()) return nullptr;
		return LoadSurface(file);
	}

	static CarpSurface* LoadSurface(const CarpLocalFile& file)
	{
		int width = 0;
		int height = 0;
		int comp = 0;
		stbi_uc* uc = stbi_load_from_memory((unsigned char*)file.GetContent(), (int)file.GetSize(), &width, &height, &comp, 0);
		if (uc == nullptr) return nullptr;

		CarpSurface* surface = nullptr;
		if (comp == 4)
		{
			surface = CreateCarpSurface(width, height);
			if (surface) memcpy(surface->GetPixels(), uc, width * height * comp);
		}
		else if (comp == 3)
		{
			surface = CreateCarpSurface(width, height);
			if (surface)
			{
				unsigned char color[4];
				color[3] = 0xFF;
				const auto size = sizeof(unsigned char) * 3;
				int count = width * height;
				auto* pixels = surface->GetPixels();
				for (int i = 0; i < count; ++i)
				{
					memcpy(color, uc + i * size, size);
					pixels[i] = *(unsigned int*)&color;
				}
			}
		}
		else if (comp == 1)
		{
			surface = CreateCarpSurface(width, height);
			if (surface)
			{
				unsigned char color[4];
				color[3] = 0xff;
				int count = width * height;
				auto* pixels = surface->GetPixels();
				for (int i = 0; i < count; ++i)
				{
					color[0] = uc[i];
					color[1] = uc[i];
					color[2] = uc[i];
					pixels[i] = *(unsigned int*)&color;
				}
			}
		}

		stbi_image_free(uc);
		return surface;
	}

	static bool SaveCarpSurface(CarpSurface* surface, const char* file_path)
	{
		return stbi_write_png(file_path, surface->GetWidth(), surface->GetHeight(), 4, surface->GetPixels(), surface->GetPitch()) != 0;
	}

	static unsigned int GetCarpSurfaceWidth(CarpSurface* surface) { return surface->GetWidth(); }
	static unsigned int GetCarpSurfaceHeight(CarpSurface* surface) { return surface->GetHeight(); }
	static unsigned int GetCarpSurfacePixel(CarpSurface* surface, int x, int y) { return surface->GetPixel(x, y); }
	static void SetCarpSurfacePixel(CarpSurface* surface, int x, int y, unsigned int value) { surface->SetPixel(x, y, value); }

	static int SetCarpSurfacePixelRect(lua_State* l_state)
	{
		auto* surface = luabridge::Stack<CarpSurface*>::get(l_state, 1);
		
		lua_pushnil(l_state);
		while (lua_next(l_state, 2) != 0)
		{
			const auto index = static_cast<int>(lua_tointeger(l_state, -2));
			const auto color = static_cast<unsigned int>(lua_tointeger(l_state, -1));
			surface->SetPixelByIndex(index, color);
			lua_pop(l_state, 1);
		}

		return 0;
	}

	static void	BlitCarpSurface(CarpSurface* src, CarpSurface* dest, unsigned int x, unsigned int y)
	{
		if (src == nullptr || dest == nullptr) return;
		dest->CopyFrom(src, nullptr, x, y);
	}

	static bool	CutBlitCarpSurface(CarpSurface* src, CarpSurface* dest, const char* from, const char* to)
	{
		std::vector<std::string> to_list;
		CarpSurfaceRect dst_rect;
		if (to != nullptr)
		{
			CarpString::Split(to, ",", false, to_list);
			if (to_list.size() < 4) return false;
			dst_rect.x = atoi(to_list[0].c_str());
			dst_rect.y = atoi(to_list[1].c_str());
			dst_rect.w = atoi(to_list[2].c_str());
			dst_rect.h = atoi(to_list[3].c_str());
		}
		else
		{
			dst_rect.x = 0;
			dst_rect.y = 0;
			dst_rect.w = dest->GetWidth();
			dst_rect.h = dest->GetHeight();
		}

		std::vector<std::string> from_list;
		CarpSurfaceRect src_rect;
		if (from != nullptr)
		{
			CarpString::Split(from, ",", false, from_list);
			if (from_list.size() < 4) return false;
			src_rect.x = atoi(from_list[0].c_str());
			src_rect.y = atoi(from_list[1].c_str());
			src_rect.w = atoi(from_list[2].c_str());
			src_rect.h = atoi(from_list[3].c_str());
		}
		else
		{
			src_rect.x = 0;
			src_rect.y = 0;
			src_rect.w = src->GetWidth();
			src_rect.h = src->GetHeight();
		}
		
		dest->ScaleFrom(src, &src_rect, &dst_rect);
		return true;
	}

	static unsigned int GetCarpSurfaceGrid9(CarpSurface* surface, const char* type)
	{
		if (surface == nullptr) return 0;
		return surface->GetGrid9(type);
	}

	static void TransferCarpSurface(CarpSurface* surface, const char* type, int step)
	{
		if (surface == nullptr) return;
		surface->TransferPixel(type, step);
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
#define STBI_WINDOWS_UTF8
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#ifdef _WIN32
#define __STDC_LIB_EXT1__
#endif
#include "stb/stb_image_write.h"
#endif
#endif