#ifndef CARP_AUDIO_CONVERSION_INCLUDED
#define CARP_AUDIO_CONVERSION_INCLUDED

#include "carp_rtp.hpp"
#include "carp_audio_codec.hpp"

class CarpAudioConversion
{
public:
	static std::string GetFileExtByPath(const std::string& file_path)
	{
		std::string::size_type pos = file_path.find_last_of('.');
		if (pos == std::string::npos) return "";

		return file_path.substr(pos + 1);
	}

	static void UpperString(std::string& target)
	{
		size_t len = target.size();
		for (size_t i = 0; i < len; ++i)
		{
			char c = target[i];
			if (c >= 'a' && c <= 'z')
			{
				c -= 'a' - 'A';
				target[i] = c;
			}
		}
	}

	static CarpAudioEncoder* CreateEncoder(const std::string& ext, int channel_num, int sample_rate)
	{
		std::string upper_ext = ext;
		UpperString(upper_ext);
		if (upper_ext == "PCM") return new CarpPCMEncoder;
		if (upper_ext == "PCMA") return new CarpPCMAEncoder;
		if (upper_ext == "PCMU") return new CarpPCMUEncoder;
		if (upper_ext == "G729") return new CarpG729Encoder;
		if (upper_ext == "WAV") return new CarpWAVEncoder(channel_num, sample_rate);
		return nullptr;
	}

	static CarpAudioDecoder* CreateDecoder(const std::string& ext)
	{
		std::string upper_ext = ext;
		UpperString(upper_ext);
		if (upper_ext == "PCM") return new CarpPCMDecoder;
		if (upper_ext == "PCMA") return new CarpPCMADecoder;
		if (upper_ext == "PCMU") return new CarpPCMUDecoder;
		if (upper_ext == "G729") return new CarpG729Decoder;
		if (upper_ext == "WAV") return new CarpWAVDecoder;
		if (upper_ext == "RTP") return new CarpRtpDecoder;
		return nullptr;
	}

	enum class ConversionError
	{
		OK = 0,
		SRC_FILE_OPEN_FAILED,
		DST_FILE_OPEN_FAILED,
		DECODER_CREATE_FAILED,
		ENCODER_CREATE_FAILED,
		DECODER_READ_FILE_FAILED,
		DECODER_READ_HEAD_FAILED,
		DECODER_DECODE_FAILED,
	};

    static ConversionError Conversion(const std::string& src_path, const std::string& dst_path)
    {
		// 打开源文件
		FILE* src_file = nullptr;
#ifdef _WIN32
		fopen_s(&src_file, src_path.c_str(), "rb");
#else
		src_file = fopen(src_path.c_str(), "rb");
#endif
		if (src_file == nullptr) return ConversionError::SRC_FILE_OPEN_FAILED;

		// 打开目标文件
		FILE* dst_file = nullptr;
#ifdef _WIN32
		fopen_s(&dst_file, dst_path.c_str(), "wb");
#else
		dst_file = fopen(dst_path.c_str(), "wb");
#endif
		if (dst_file == nullptr)
		{
			fclose(src_file);
			return ConversionError::DST_FILE_OPEN_FAILED;
		}

		// 创建解码器
		std::string src_ext = GetFileExtByPath(src_path);
		auto* decoder = CreateDecoder(src_ext);
		if (decoder == nullptr)
		{
			fclose(src_file);
			fclose(dst_file);
			return ConversionError::DECODER_CREATE_FAILED;
		}

		// 如果简码器需要读文件头，那么就需要读一下才能知道声道，采样率等信息
		CarpAudioEncoder* encoder = nullptr;
		if (decoder->NeedReadHead())
		{
			std::vector<unsigned char> encode_buffer;

			// 每次读文件头，至少读多少字节
			int min_read_head_buffer_size = decoder->MinReadHead();
			std::vector<unsigned char> head_buffer;
			std::vector<short> sample_data;
			head_buffer.resize(min_read_head_buffer_size);
			while (true)
			{
				// 读取文件
				int read_size = (int)fread(head_buffer.data(), 1, min_read_head_buffer_size, src_file);
				if (read_size == 0)
				{
					fclose(src_file);
					fclose(dst_file);
					delete decoder;
					return ConversionError::DECODER_READ_FILE_FAILED;
				}
				// 解析文件头
				bool is_completed = false;
				if (decoder->ReadHead(head_buffer.data(), read_size, sample_data, is_completed) == false)
				{
					fclose(src_file);
					fclose(dst_file);
					delete decoder;
					return ConversionError::DECODER_READ_HEAD_FAILED;
				}

				// 如果还未结束，那么就继续读取
				if (!is_completed) continue;

				// 读取声道数和采样率
				int channel_num = decoder->GetChannelNum();
				int sample_rate = decoder->GetSampleRate();

				// 创建编码器
				std::string dst_ext = GetFileExtByPath(dst_path);
				encoder = CreateEncoder(dst_ext, channel_num, sample_rate);
				if (encoder == nullptr)
				{
					fclose(src_file);
					fclose(dst_file);
					delete decoder;
					return ConversionError::ENCODER_CREATE_FAILED;
				}

				// 如果有输出文件头，那么就写入。其实这里主要是占用文件空间，还不是最后的数据。后面会再写入
				int head_size = encoder->GetHeadSize();
				if (head_size > 0) fwrite(encoder->GetHeadMemory(), 1, head_size, dst_file);

				// 如果在读取文件头的时候，多读了一些语音内容，并且又解码了这部分内容，那么就要处理下
				if (!sample_data.empty())
				{
					// 开始编码
					encoder->Encode(sample_data.data(), (int)sample_data.size(), encode_buffer);
					// 如果得到编码结果，那么就写入文件
					if (!encode_buffer.empty()) fwrite(encode_buffer.data(), 1, encode_buffer.size(), dst_file);
				}
				break;
			}
		}

		// 创建编码器
		if (encoder == nullptr)
		{
			int channel_num = decoder->GetChannelNum();
			int sample_rate = decoder->GetSampleRate();

			std::string dst_ext = GetFileExtByPath(dst_path);
			encoder = CreateEncoder(dst_ext, channel_num, sample_rate);
			if (encoder == nullptr)
			{
				fclose(src_file);
				fclose(dst_file);
				delete decoder;
				return ConversionError::ENCODER_CREATE_FAILED;
			}

			// 如果有输出文件头，那么就写入。其实这里主要是占用文件空间，还不是最后的数据。后面会再写入
			int head_size = encoder->GetHeadSize();
			if (head_size > 0) fwrite(encoder->GetHeadMemory(), 1, head_size, dst_file);
		}

		std::vector<short> sample_data;
		std::vector<unsigned char> encode_buffer;

		// 开始循环读取文件内容进行解码和编码
		while (true)
		{
			// 读取源文件
			unsigned char buffer[1020]; // 这里使用10的整数倍，对g729的解码有提速
			int read_size = (int)fread(buffer, 1, sizeof(buffer), src_file);
			
			if (read_size > 0)
			{
				if (decoder->Decode(buffer, read_size, sample_data) == false)
				{
					fclose(src_file);
					fclose(dst_file);
					delete decoder;
					delete encoder;
					return ConversionError::DECODER_DECODE_FAILED;
				}
			}
			else
			{
				decoder->Flush(sample_data);
			}

			// 如果有解码出来的语音数据
			if (!sample_data.empty())
			{
				// 开始编码
				encoder->Encode(sample_data.data(), (int)sample_data.size(), encode_buffer);
				// 如果有编码出内容，那么就写入文件
				if (!encode_buffer.empty()) fwrite(encode_buffer.data(), 1, encode_buffer.size(), dst_file);
			}

			if (read_size <= 0) break;
		}

		// 把剩余的语音全部编码出来
		encoder->Flush(encode_buffer);
		// 如果有编码出来，那么就写入文件
		if (!encode_buffer.empty()) fwrite(encode_buffer.data(), 1, encode_buffer.size(), dst_file);

		// 因为在编码过后，文件头的信息会发生变化，所以这里再次写入文件头
		int head_size = encoder->GetHeadSize();
		if (head_size > 0)
		{
			fseek(dst_file, 0, SEEK_SET);
			fwrite(encoder->GetHeadMemory(), 1, head_size, dst_file);
		}

		delete decoder;
		delete encoder;
		fclose(src_file);
		fclose(dst_file);

		return ConversionError::OK;
    }
};

#endif
