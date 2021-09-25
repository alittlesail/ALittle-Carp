
#ifndef CARP_OPENCV_BIND_INCLUDED
#define CARP_OPENCV_BIND_INCLUDED

#include "opencv2/opencv.hpp"
#include "carp_surface.hpp"
#include "carp_lua.hpp"

class CarpOpenCVBind
{
public:
	static void Bind(lua_State* l_state)
	{
        luabridge::getGlobalNamespace(l_state)
            .beginNamespace("carp")
			.beginClass<cv::VideoCapture>("CarpVideoCapture")
			.endClass()

			.addFunction("CreateCarpVideoCapture", CreateCarpVideoCapture)
			.addFunction("FreeCarpVideoCapture", FreeCarpVideoCapture)
			.addFunction("GetCarpVideoCameraWidth", GetCarpVideoCameraWidth)
			.addFunction("GetCarpVideoCameraHeight", GetCarpVideoCameraHeight)
			.addFunction("OpenCarpVideoCamera", OpenCarpVideoCamera)
			.addFunction("IsOpenCarpVideoCamera", IsOpenCarpVideoCamera)
			.addFunction("CloseCarpVideoCamera", CloseCarpVideoCamera)
			.addFunction("GetCarpVideoCameraFrame", GetCarpVideoCameraFrame)
			.endNamespace();
	}

	static cv::VideoCapture* CreateCarpVideoCapture()
	{
		return new cv::VideoCapture();
	}

	static void FreeCarpVideoCapture(cv::VideoCapture* cap)
	{
		delete cap;
	}

	static int GetCarpVideoCameraWidth(cv::VideoCapture* cap)
	{
		return (int)cap->get(cv::CAP_PROP_FRAME_WIDTH);
	}

	static int GetCarpVideoCameraHeight(cv::VideoCapture* cap)
	{
		return (int)cap->get(cv::CAP_PROP_FRAME_HEIGHT);
	}

	static bool OpenCarpVideoCamera(cv::VideoCapture* cap, int index, int width, int height, int channels)
	{
		if (width > 0)
			cap->set(cv::CAP_PROP_FRAME_WIDTH, width);
		if (height > 0)
			cap->set(cv::CAP_PROP_FRAME_HEIGHT, height);
		int type = cv::CAP_ANY;
		if (channels > 0)
			type = CV_8UC(channels);
		return cap->open(index, type);
	}

	static bool IsOpenCarpVideoCamera(cv::VideoCapture* cap)
	{
		return cap->isOpened();
	}

	static void CloseCarpVideoCamera(cv::VideoCapture* cap)
	{
		cap->release();
	}

	static bool GetCarpVideoCameraFrame(cv::VideoCapture* cap, size_t surface_address)
	{
		cv::Mat frame;
		(*cap) >> frame;
		if (frame.empty()) return false;

		CarpSurface* surface = nullptr;
		memcpy(&surface, &surface_address, sizeof(size_t));
		if (surface == nullptr) return false;

		if (frame.rows != surface->GetHeight()) return false;
		if (frame.cols != surface->GetWidth()) return false;

		if (frame.type() == CV_8UC1)
		{
			for (int row = 0; row < frame.rows; ++row)
			{
				for (int col = 0; col < frame.cols; ++col)
				{
					unsigned char color[4];
					color[3] = 0xff;
					int count = frame.rows * frame.cols;
					auto* pixels = surface->GetPixels();
					for (int i = 0; i < count; ++i)
					{
						color[0] = frame.data[i];
						color[1] = frame.data[i];
						color[2] = frame.data[i];
						pixels[i] = *(unsigned int*)&color;
					}
				}
			}

			return true;
		}
		
		if (frame.type() == CV_8UC3)
		{
			unsigned char color[4];
			color[3] = 0xFF;
			const auto size = sizeof(unsigned char) * 3;
			int count = frame.rows * frame.cols;
			auto* pixels = surface->GetPixels();
			for (int i = 0; i < count; ++i)
			{
				color[0] = frame.data[i * size + 2];
				color[1] = frame.data[i * size + 1];
				color[2] = frame.data[i * size + 0];
				pixels[i] = *(unsigned int*)&color;
			}

			return true;
		}
		
		if (frame.type() == CV_8UC4)
		{
			memcpy(surface->GetPixels(), frame.data, frame.rows * frame.cols * frame.channels());
			return true;
		}

		return false;
	}

};

#endif