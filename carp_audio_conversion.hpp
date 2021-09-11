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
		// ��Դ�ļ�
		FILE* src_file = nullptr;
#ifdef _WIN32
		fopen_s(&src_file, src_path.c_str(), "rb");
#else
		src_file = fopen(src_path.c_str(), "rb");
#endif
		if (src_file == nullptr) return ConversionError::SRC_FILE_OPEN_FAILED;

		// ��Ŀ���ļ�
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

		// ����������
		std::string src_ext = GetFileExtByPath(src_path);
		auto* decoder = CreateDecoder(src_ext);
		if (decoder == nullptr)
		{
			fclose(src_file);
			fclose(dst_file);
			return ConversionError::DECODER_CREATE_FAILED;
		}

		// �����������Ҫ���ļ�ͷ����ô����Ҫ��һ�²���֪�������������ʵ���Ϣ
		CarpAudioEncoder* encoder = nullptr;
		if (decoder->NeedReadHead())
		{
			std::vector<unsigned char> encode_buffer;

			// ÿ�ζ��ļ�ͷ�����ٶ������ֽ�
			int min_read_head_buffer_size = decoder->MinReadHead();
			std::vector<unsigned char> head_buffer;
			std::vector<short> sample_data;
			head_buffer.resize(min_read_head_buffer_size);
			while (true)
			{
				// ��ȡ�ļ�
				int read_size = (int)fread(head_buffer.data(), 1, min_read_head_buffer_size, src_file);
				if (read_size == 0)
				{
					fclose(src_file);
					fclose(dst_file);
					delete decoder;
					return ConversionError::DECODER_READ_FILE_FAILED;
				}
				// �����ļ�ͷ
				bool is_completed = false;
				if (decoder->ReadHead(head_buffer.data(), read_size, sample_data, is_completed) == false)
				{
					fclose(src_file);
					fclose(dst_file);
					delete decoder;
					return ConversionError::DECODER_READ_HEAD_FAILED;
				}

				// �����δ��������ô�ͼ�����ȡ
				if (!is_completed) continue;

				// ��ȡ�������Ͳ�����
				int channel_num = decoder->GetChannelNum();
				int sample_rate = decoder->GetSampleRate();

				// ����������
				std::string dst_ext = GetFileExtByPath(dst_path);
				encoder = CreateEncoder(dst_ext, channel_num, sample_rate);
				if (encoder == nullptr)
				{
					fclose(src_file);
					fclose(dst_file);
					delete decoder;
					return ConversionError::ENCODER_CREATE_FAILED;
				}

				// ���������ļ�ͷ����ô��д�롣��ʵ������Ҫ��ռ���ļ��ռ䣬�������������ݡ��������д��
				int head_size = encoder->GetHeadSize();
				if (head_size > 0) fwrite(encoder->GetHeadMemory(), 1, head_size, dst_file);

				// ����ڶ�ȡ�ļ�ͷ��ʱ�򣬶����һЩ�������ݣ������ֽ������ⲿ�����ݣ���ô��Ҫ������
				if (!sample_data.empty())
				{
					// ��ʼ����
					encoder->Encode(sample_data.data(), (int)sample_data.size(), encode_buffer);
					// ����õ�����������ô��д���ļ�
					if (!encode_buffer.empty()) fwrite(encode_buffer.data(), 1, encode_buffer.size(), dst_file);
				}
				break;
			}
		}

		// ����������
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

			// ���������ļ�ͷ����ô��д�롣��ʵ������Ҫ��ռ���ļ��ռ䣬�������������ݡ��������д��
			int head_size = encoder->GetHeadSize();
			if (head_size > 0) fwrite(encoder->GetHeadMemory(), 1, head_size, dst_file);
		}

		std::vector<short> sample_data;
		std::vector<unsigned char> encode_buffer;

		// ��ʼѭ����ȡ�ļ����ݽ��н���ͱ���
		while (true)
		{
			// ��ȡԴ�ļ�
			unsigned char buffer[1020]; // ����ʹ��10������������g729�Ľ���������
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

			// ����н����������������
			if (!sample_data.empty())
			{
				// ��ʼ����
				encoder->Encode(sample_data.data(), (int)sample_data.size(), encode_buffer);
				// ����б�������ݣ���ô��д���ļ�
				if (!encode_buffer.empty()) fwrite(encode_buffer.data(), 1, encode_buffer.size(), dst_file);
			}

			if (read_size <= 0) break;
		}

		// ��ʣ�������ȫ���������
		encoder->Flush(encode_buffer);
		// ����б����������ô��д���ļ�
		if (!encode_buffer.empty()) fwrite(encode_buffer.data(), 1, encode_buffer.size(), dst_file);

		// ��Ϊ�ڱ�������ļ�ͷ����Ϣ�ᷢ���仯�����������ٴ�д���ļ�ͷ
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
