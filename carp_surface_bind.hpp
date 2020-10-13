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
			.addFunction("FreeCarpSurface", FreeCarpSurface)
			.addFunction("BlitCarpSurface", BlitCarpSurface)
			.addFunction("LoadCarpSurface", LoadCarpSurfaceForLua)
			.addFunction("SaveCarpSurface", SaveCarpSurface)
			.addFunction("GetCarpSurfaceWidth", GetCarpSurfaceWidth)
			.addFunction("GetCarpSurfaceHeight", GetCarpSurfaceHeight)
			.addFunction("CutBlitCarpSurface", CutBlitCarpSurface)
			.addFunction("GetCarpSurfaceGrid9", GetCarpSurfaceGrid9)
			.addFunction("GetCarpSurfacePixel", GetCarpSurfacePixel)
			.addFunction("SetCarpSurfacePixel", SetCarpSurfacePixel)
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

	static void FreeCarpSurface(CarpSurface* surface)
	{
		delete surface;
	}
	
	static CarpSurface* LoadCarpSurfaceForLua(const char* file_path) { return LoadSurface(file_path); }
	static CarpSurface* LoadSurface(const char* file_path)
	{
		CarpLocalFile file;
		file.SetPath(file_path);
		if (!file.Load(false)) return nullptr;
		return LoadSurface(file);
	}

	static CarpSurface* LoadSurface(const CarpLocalFile& file)
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
		CarpSurface* surface = CreateCarpSurface(width, height);
		if (surface) memcpy(surface->GetPixels(), uc, width * height * comp);
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

	static void	BlitCarpSurface(CarpSurface* src, CarpSurface* dest, unsigned int x, unsigned int y)
	{
		if (src == nullptr || dest == nullptr) return;
		dest->CopyFrom(src, nullptr, x, y);
	}

	static bool	CutBlitCarpSurface(CarpSurface* src, CarpSurface* dest, const char* from, const char* to)
	{
		std::vector<std::string> to_list;
		CarpString::Split(to, ",", to_list);
		if (to_list.size() < 4) return false;

		std::vector<std::string> from_list;
		CarpString::Split(from, ",", from_list);
		if (from_list.size() < 4) return false;

		CarpSurfaceRect dst_rect;
		dst_rect.x = atoi(to_list[0].c_str());
		dst_rect.y = atoi(to_list[1].c_str());
		dst_rect.w = atoi(to_list[2].c_str());
		dst_rect.h = atoi(to_list[3].c_str());

		CarpSurfaceRect src_rect;
		src_rect.x = atoi(from_list[0].c_str());
		src_rect.y = atoi(from_list[1].c_str());
		src_rect.w = atoi(from_list[2].c_str());
		src_rect.h = atoi(from_list[3].c_str());

		dest->ScaleFrom(src, &src_rect, &dst_rect);
		return true;
	}

	static unsigned int GetCarpSurfaceGrid9(CarpSurface* surface, const char* type)
	{
		if (surface == nullptr) return 0;
		return surface->GetGrid9(type);
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
#include "stb/stb_image_write.h"
#endif
#endif