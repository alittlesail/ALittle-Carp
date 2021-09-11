#ifndef CARP_AUDIO_CODEC_INCLUDED
#define CARP_AUDIO_CODEC_INCLUDED

#include <string>
#include <vector>
#include <list>
#include <map>

class CarpAudioEncoder
{
public:
	virtual ~CarpAudioEncoder() { }

public:
	// �ļ�ͷ��С����С�ǹ̶�
	virtual int GetHeadSize() { return 0; }
	// �ļ�ͷ����
	virtual unsigned char* GetHeadMemory() { return 0; }
	// ����������Ĭ����1
	virtual int GetChannelNum() { return 1; }

public:
	// ִ�б���
	// sample_data �����������ڴ�
	// sample_count ��������
	// buffer �����ĵ�ַ
	// buffer_len �������ݵĳ���
	virtual bool Encode(short* sample_data, int sample_count, std::vector<unsigned char>& buffer) = 0;

	// ���������л�����
	virtual void Flush(std::vector<unsigned char>& buffer) { buffer.resize(0); }
};

class CarpAudioDecoder
{
public:
	virtual ~CarpAudioDecoder() { }

public:
	// �Ƿ���Ҫ��ȡ�ļ�ͷ
	virtual bool NeedReadHead() { return false; }

	// ÿ�ζ�ȡ�ļ�ͷ�Ĵ�С
	virtual int MinReadHead() { return 1020; }

	// �����ļ�ͷ��is_completedΪtrue��ʾ�ļ�ͷȫ���������
	virtual bool ReadHead(unsigned char* buffer, int buffer_len, std::vector<short>& sample_data, bool& is_completed) { sample_data.resize(0);  is_completed = true; return true; }

public:
	// ��ȡ������
	virtual int GetChannelNum() { return 1; }
	// ��ȡ������
	virtual int GetSampleRate() { return 8000; }

public:
	// ִ�н���
	// buffer �����ĵ�ַ
	// buffer_len �������ݵĳ���
	// sample_data �����������ڴ�
	virtual bool Decode(unsigned char* buffer, int buffer_len, std::vector<short>& sample_data) = 0;

	// ���������л�����
	virtual void Flush(std::vector<short>& sample_data) { sample_data.resize(0); }
};

class CarpPCMEncoder : public CarpAudioEncoder
{
public:
	bool Encode(short* sample_data, int sample_count, std::vector<unsigned char>& buffer) override
	{
		buffer.resize(sample_count * sizeof(short));
		memcpy(buffer.data(), sample_data, sample_count * sizeof(short));
		return true;
	}
};

class CarpPCMDecoder : public CarpAudioDecoder
{
public:
	bool Decode(unsigned char* buffer, int buffer_len, std::vector<short>& sample_data) override
	{
		sample_data.resize(buffer_len / sizeof(short));
		memcpy(sample_data.data(), buffer, buffer_len);
		return true;
	}
};

class CarpPCMAEncoder : public CarpAudioEncoder
{
public:
	bool Encode(short* sample_data, int sample_count, std::vector<unsigned char>& buffer) override
	{
		buffer.resize(sample_count);
		for (size_t i = 0; i < buffer.size(); ++i)
			buffer[i] = PCM2ALaw(sample_data[i]);
		return true;
	}

public:
	static short search(short  val, short* table, short  size)
	{
		short i;
		for (i = 0; i < size; i++)
		{
			if (val <= *table++)     // find out a value most close(larger) to input value
			{
				return i;
			}
		}
		return (size);
	}

	static unsigned char PCM2ALaw(short  pcm_val)    /* 2's complement (16-bit range) */
	{
		// a-law
		static const unsigned char SIGN_BIT = 0x80;      /* Sign bit for a A-law byte. */
		static const unsigned char QUANT_MASK = 0xf;       /* Quantization field mask. */
		static const unsigned char NSEGS = 8;         /* Number of A-law segments. */
		static const unsigned char SEG_SHIFT = 4;         /* Left shift for segment number. */
		static const unsigned char SEG_MASK = 0x70;      /* Segment field mask. */

		static const unsigned char BIAS = 0x84;      /* Bias for linear code. */
		static const unsigned int CLIP = 8159;

		static const short seg_aend[8] = { 0x1F, 0x3F, 0x7F, 0xFF,
			0x1FF, 0x3FF, 0x7FF, 0xFFF };
		static const short seg_uend[8] = { 0x3F, 0x7F, 0xFF, 0x1FF,
			0x3FF, 0x7FF, 0xFFF, 0x1FFF };

		short       mask;
		short       seg;
		unsigned char   aval;


		pcm_val = pcm_val >> 3;

		if (pcm_val >= 0)
		{
			mask = 0xD5;        /* sign (7th) bit = 1 */
		}
		else
		{
			mask = 0x55;        /* sign bit = 0 */
			pcm_val = -pcm_val - 1;
		}

		/* Convert the scaled magnitude to segment number. */
		seg = search(pcm_val, (short*)seg_aend, (short)8);

		/* Combine the sign, segment, and quantization bits. */

		if (seg >= 8)        /* out of range, return maximum value. */
		{
			return (unsigned char)(0x7F ^ mask);
		}
		else
		{
			aval = (unsigned char)seg << SEG_SHIFT;

			if (seg < 2)
			{
				aval |= (pcm_val >> 1) & QUANT_MASK;
			}
			else
			{
				aval |= (pcm_val >> seg) & QUANT_MASK;
			}
			return (aval ^ mask);
		}
	}
};

class CarpPCMADecoder : public CarpAudioDecoder
{
public:
	bool Decode(unsigned char* buffer, int buffer_len, std::vector<short>& sample_data) override
	{
		sample_data.resize(buffer_len);
		for (size_t i = 0; i < sample_data.size(); ++i)
			sample_data[i] = ALaw2PCM(buffer[i]);
		return true;
	}

public:
	static short ALaw2PCM(unsigned char a_val)
	{
		// a-law
		static const unsigned char SIGN_BIT = 0x80;      /* Sign bit for a A-law byte. */
		static const unsigned char QUANT_MASK = 0xf;       /* Quantization field mask. */
		static const unsigned char NSEGS = 8;         /* Number of A-law segments. */
		static const unsigned char SEG_SHIFT = 4;         /* Left shift for segment number. */
		static const unsigned char SEG_MASK = 0x70;      /* Segment field mask. */

		static const unsigned char BIAS = 0x84;      /* Bias for linear code. */
		static const unsigned int CLIP = 8159;

		static const short seg_aend[8] = { 0x1F, 0x3F, 0x7F, 0xFF,
			0x1FF, 0x3FF, 0x7FF, 0xFFF };
		static const short seg_uend[8] = { 0x3F, 0x7F, 0xFF, 0x1FF,
			0x3FF, 0x7FF, 0xFFF, 0x1FFF };

		short       t;
		short       seg;

		a_val ^= 0x55;

		t = (a_val & QUANT_MASK) << 4;
		seg = ((unsigned)a_val & SEG_MASK) >> SEG_SHIFT;
		switch (seg)
		{
		case 0:
			t += 8;
			break;
		case 1:
			t += 0x108;
			break;
		default:
			t += 0x108;
			t <<= seg - 1;
			break;
		}
		return ((a_val & SIGN_BIT) ? t : -t);
	}
};

class CarpPCMUEncoder : public CarpAudioEncoder
{
public:
	bool Encode(short* sample_data, int sample_count, std::vector<unsigned char>& buffer) override
	{
		buffer.resize(sample_count);
		for (size_t i = 0; i < buffer.size(); ++i)
			buffer[i] = PCM2ULaw(sample_data[i]);
		return true;
	}

public:
	static short search(short val, short* table, short size)
	{
		short  i;

		for (i = 0; i < size; i++)
		{
			if (val <= *table++)    // find out a value most close(larger) to input value
			{
				return (i);
			}
		}
		return (size);
	}

	static unsigned char PCM2ULaw(short  pcm_val)    /* 2's complement (16-bit range) */
	{
		static const unsigned char SIGN_BIT = 0x80;  /* Sign bit for a A-law byte. */
		static const unsigned char QUANT_MASK = 0xf;  /* Quantization field mask. */
		static const unsigned char NSEGS = 8;   /* Number of A-law segments. */
		static const unsigned char SEG_SHIFT = 4;   /* Left shift for segment number. */
		static const unsigned char SEG_MASK = 0x70;  /* Segment field mask. */

		static const unsigned char BIAS = 0x84;  /* Bias for linear code. */
		static const unsigned int CLIP = 8159;

		static const short seg_aend[8] = { 0x1F, 0x3F, 0x7F, 0xFF,
			0x1FF, 0x3FF, 0x7FF, 0xFFF };
		static const short seg_uend[8] = { 0x3F, 0x7F, 0xFF, 0x1FF,
			0x3FF, 0x7FF, 0xFFF, 0x1FFF };

		short  mask;
		short  seg;
		unsigned char uval;

		/* Get the sign and the magnitude of the value. */
		pcm_val = pcm_val >> 2;
		if (pcm_val < 0)
		{
			pcm_val = -pcm_val;
			mask = 0x7F;
		}
		else
		{
			mask = 0xFF;
		}
		if (pcm_val > CLIP)
		{
			pcm_val = CLIP;  /* clip the magnitude */
		}
		pcm_val += (BIAS >> 2);

		/* Convert the scaled magnitude to segment number. */
		seg = search(pcm_val, (short*)seg_uend, (short)8);

		/*
		 * Combine the sign, segment, quantization bits;
		 * and complement the code word.
		 */
		if (seg >= 8)  /* out of range, return maximum value. */
		{
			return (unsigned char)(0x7F ^ mask);
		}
		else
		{
			uval = (unsigned char)(seg << 4) | ((pcm_val >> (seg + 1)) & 0xF);
			return (uval ^ mask);
		}
	}
};

class CarpPCMUDecoder : public CarpAudioDecoder
{
public:
	bool Decode(unsigned char* buffer, int buffer_len, std::vector<short>& sample_data) override
	{
		sample_data.resize(buffer_len);
		for (size_t i = 0; i < sample_data.size(); ++i)
			sample_data[i] = ULaw2PCM(buffer[i]);
		return true;
	}

public:
	static short ULaw2PCM(unsigned char u_val)
	{
		static const unsigned char SIGN_BIT = 0x80;  /* Sign bit for a A-law byte. */
		static const unsigned char QUANT_MASK = 0xf;  /* Quantization field mask. */
		static const unsigned char NSEGS = 8;   /* Number of A-law segments. */
		static const unsigned char SEG_SHIFT = 4;   /* Left shift for segment number. */
		static const unsigned char SEG_MASK = 0x70;  /* Segment field mask. */

		static const unsigned char BIAS = 0x84;  /* Bias for linear code. */
		static const unsigned int CLIP = 8159;

		static const short seg_aend[8] = { 0x1F, 0x3F, 0x7F, 0xFF,
			0x1FF, 0x3FF, 0x7FF, 0xFFF };
		static const short seg_uend[8] = { 0x3F, 0x7F, 0xFF, 0x1FF,
			0x3FF, 0x7FF, 0xFFF, 0x1FFF };

		short  t;

		/* Complement to obtain normal u-law value. */
		u_val = ~u_val;

		/*
		 * Extract and bias the quantization bits. Then
		 * shift up by the segment number and subtract out the bias.
		 */
		t = ((u_val & QUANT_MASK) << 3) + BIAS;
		t <<= ((unsigned)u_val & SEG_MASK) >> SEG_SHIFT;

return ((u_val & SIGN_BIT) ? (BIAS - t) : (t - BIAS));
	}
};

extern "C" {
#include "bcg729/include/bcg729/decoder.h"
#include "bcg729/include/bcg729/encoder.h"
}

class CarpG729Encoder : public CarpAudioEncoder
{
public:
	// G729 Annex B is signaled to be disabled
	CarpG729Encoder() { m_encoder = initBcg729EncoderChannel(0); }
	~CarpG729Encoder() { closeBcg729EncoderChannel(m_encoder); }

public:
	bool Encode(short* sample_data, int sample_count, std::vector<unsigned char>& buffer) override
	{
		buffer.resize(sample_count / 80 * 10, 0);
		unsigned char* buffer_ptr = buffer.data();

		int count = sample_count / 80;
		uint8_t len = 0;
		for (int i = 0; i < count; ++i)
		{
			bcg729Encoder(m_encoder, sample_data, buffer_ptr, &len);
			sample_data += 80;
			buffer_ptr += 10;
		}

		return true;
	}

private:
	bcg729EncoderChannelContextStruct* m_encoder = nullptr;
};

class CarpG729Decoder : public CarpAudioDecoder
{
public:
	CarpG729Decoder() { m_decoder = initBcg729DecoderChannel(); }
	~CarpG729Decoder() { closeBcg729DecoderChannel(m_decoder); }

public:
	bool Decode(unsigned char* buffer, int buffer_len, std::vector<short>& sample_data) override
	{
		if (m_remain.empty())
		{
			// ֱ�Ӵ�������
			sample_data.resize(buffer_len / 10 * 80, 0);
			short* sample_data_ptr = sample_data.data();

			int count = buffer_len / 10;
			for (int i = 0; i < count; ++i)
			{
				bcg729Decoder(m_decoder, buffer, 10, 0, 0, 0, sample_data_ptr);
				buffer += 10;
				sample_data_ptr += 80;
			}

			// ����ʣ�µ�����
			if (buffer_len % 10 > 0)
			{
				m_remain.resize(buffer_len % 10);
				memcpy(m_remain.data(), buffer, buffer_len % 10);
			}

			return true;
		}

		size_t old_size = m_remain.size();
		m_remain.resize(old_size + buffer_len);
		memcpy(m_remain.data() + old_size, buffer, buffer_len);

		buffer = m_remain.data();
		buffer_len = m_remain.size();

		// ��ʼ��������
		sample_data.resize(buffer_len / 10 * 80, 0);
		short* sample_data_ptr = sample_data.data();

		int count = buffer_len / 10;
		for (int i = 0; i < count; ++i)
		{
			bcg729Decoder(m_decoder, buffer, 10, 0, 0, 0, sample_data_ptr);
			buffer += 10;
			sample_data_ptr += 80;
		}

		// �ƶ�ʣ�µ�����
		if (buffer_len % 10 > 0)
			memmove(m_remain.data(), buffer, buffer_len % 10);
		m_remain.resize(buffer_len % 10);

		return true;
	}

	void Flush(std::vector<short>& sample_data) override
	{
		sample_data.resize(0);
		if (m_remain.empty()) return;
		m_remain.resize(10, 0);
		sample_data.resize(80);

		bcg729Decoder(m_decoder, m_remain.data(), 10, 1, 0, 0, sample_data.data());
		m_remain.resize(0);
	}

private:
	bcg729DecoderChannelContextStruct* m_decoder = nullptr;
	std::vector<unsigned char> m_remain;
};

#include "carp_rtp.hpp"

class CarpRtpDecoder : public CarpAudioDecoder
{
public:
	bool Decode(unsigned char* buffer, int buffer_len, std::vector<short>& sample_data) override
	{
		PushBuffer(buffer, buffer_len);
		ParsePackage();

		size_t total_size = 0;
		for (auto& value : m_out_list)
			total_size += value.size();

		sample_data.resize(total_size);

		size_t offset = 0;
		for (auto& value : m_out_list)
		{
			memcpy(sample_data.data() + offset, value.data(), value.size() * sizeof(short));
			offset += value.size();
		}
		m_out_list.clear();
		return true;
	}

	void Flush(std::vector<short>& sample_data) override
	{
		while (true)
		{
			if (!MixAudio(1, 1)) break;
		}

		size_t total_size = 0;
		for (auto& value : m_out_list)
			total_size += value.size();

		sample_data.resize(total_size);

		size_t offset = 0;
		for (auto& value : m_out_list)
		{
			memcpy(sample_data.data() + offset, value.data(), value.size() * sizeof(short));
			offset += value.size();
		}
		m_out_list.clear();
	}

private:
	// �ռ�����
	void PushBuffer(unsigned char* buffer, int buffer_len)
	{
		int buffer_index = 0;

		// ѭ����ȡ
		while (true)
		{
			// ��������ˣ�ֱ�ӷ���
			if (buffer_index >= buffer_len) return;

			// ����б��ǿյģ���ô�����һ��
			// ������һ�������ˣ���ôҲ���һ��
			if (m_package_list.empty()
				|| (m_package_list.back().size() >= sizeof(size_t)
					&& m_package_list.back().size() >= sizeof(size_t) + *(size_t*)(m_package_list.back().data())))
				m_package_list.emplace_back(std::vector<unsigned char>());
				
			// ȡ�����һ��
			auto& back = m_package_list.back();

			// �������С��δ�����ϣ���ô�ͼ������
			if (back.size() < sizeof(size_t))
			{
				for (size_t i = back.size(); i < sizeof(size_t); ++i)
				{
					back.push_back(buffer[buffer_index]);
					buffer_index++;

					if (buffer_index >= buffer_len) return;
				}
			}

			// ��������С��δ�����ϣ���ô�ͼ������
			for (size_t i = back.size(); i < sizeof(size_t) + *(size_t*)(back.data()); ++i)
			{
				back.push_back(buffer[buffer_index]);
				buffer_index++;

				if (buffer_index >= buffer_len) return;
			}
		}
	}

	// ��������
	void ParsePackage()
	{
		std::vector<short> sample_buffer;
		while (true)
		{
			// ����Ƿ��а�
			if (m_package_list.empty()) break;

			// ����������
			auto& front = m_package_list.front();
			if (front.size() < sizeof(size_t)) break;
			if (front.size() < sizeof(size_t) + *(size_t*)(front.data())) break;

			// ������
			CarpRtpPacket packet;
			auto result = CarpRtp::CarpRtpPacketDeserialize(&packet, front.data() + sizeof(size_t), front.size() - sizeof(size_t));
			if (!result)
			{
				m_package_list.pop_front();
				continue;
			}

			// ��ȡ������
			CarpAudioDecoder* decoder = nullptr;
			if (packet.rtp.pt == RTP_G729)
				decoder = &m_g729_decoder;
			else if (packet.rtp.pt == RTP_PCMA)
				decoder = &m_pcma_decoder;
			else if (packet.rtp.pt == RTP_PCMU)
				decoder = &m_pcmu_decoder;

			// ��ʼ����
			if (decoder != nullptr)
			{
				decoder->Decode((unsigned char*)packet.payload, packet.payloadlen, sample_buffer);
				m_sample_map[packet.rtp.ssrc].push_back(sample_buffer);

				// ��������ȴ���5�����ģ���ô���ȴ���4����
				MixAudio(5, 4);
			}

			// ɾ����
			m_package_list.pop_front();
		}
	}

	// ������������ʵ�ʻ������
	bool MixAudio(size_t package_count, size_t mix_count)
	{
		// �ж��������Ƿ��������
		bool need_mix = false;
		for (auto& pair : m_sample_map)
		{
			if (pair.second.size() >= package_count)
			{
				need_mix = true;
				break;
			}
		}
		if (!need_mix) return false;

		// ��ʼ����
		for (size_t i = 0; i < mix_count; ++i)
		{
			// ���һ�����
			m_out_list.push_back(std::vector<short>());
			// ��ӻ��
			for (auto& pair : m_sample_map)
			{
				if (pair.second.empty()) continue;

				MixList(m_out_list.back(), pair.second.front());
				pair.second.pop_front();
			}
		}

		return true;
	}

	static void MixList(std::vector<short>& out, const std::vector<short>& sample_list)
	{
		// ��ʼ��Ҫ��ϵ�����
		size_t mix_count = sample_list.size();
		// �����ǰ�������С�ڲ����б���ô�Ϳ���β��
		if (out.size() < sample_list.size())
		{
			// �����������Ϊout��С
			mix_count = out.size();
			// ��ʼ���ݲ�����
			out.resize(sample_list.size());
			for (size_t i = mix_count; i < sample_list.size(); ++i)
				out[i] = sample_list[i];
		}

		// ��ʼ���
		for (size_t i = 0; i < mix_count; ++i)
			out[i] = Mix(out[i], sample_list[i]);
	}

	static short Mix(short sample1, short sample2)
	{
		int value = 0;
		if (sample1 < 0 && sample2 < 0)
			value = sample1 + sample2 - (sample1 * sample2 / -32767);
		else
			value = sample1 + sample2 - (sample1 * sample2 / 32767);
		return (short)value;
	}

	enum RtpFormat
	{
		RTP_PCM = 1,
		RTP_PCMU = 0,
		RTP_PCMA = 8,
		RTP_G729 = 18
	};

private:
	std::list<std::vector<unsigned char>> m_package_list;
	std::map<uint32_t, std::list<std::vector<short>>> m_sample_map;
	std::list<std::vector<short>> m_out_list;

private:
	CarpPCMADecoder m_pcma_decoder;
	CarpPCMUDecoder m_pcmu_decoder;
	CarpG729Decoder m_g729_decoder;
};

class CarpWAVEncoder : public CarpAudioEncoder
{
public:
	struct WAVCODERHEAD_1
	{
		char riff_name[4] = { 0 };
		unsigned long riff_len = 0;
		char wav_name[4] = { 0 };
		char fmt_name[4] = { 0 };
		unsigned long fmt_len = 0;
	};

	struct WAVCODERHEAD_2
	{
		unsigned short audio_fmt = 0;
		unsigned short channel = 0;
		unsigned long sample_rate = 0;
		unsigned long bytes_per_second = 0;
		unsigned short bytes_per_sample = 0;
		unsigned short bits_per_sample = 0;
	};

	struct WAVCODERHEAD_3
	{
		char data_name[4] = { 0 };
		unsigned long data_len = 0;
	};

	struct WAVCODERHEAD
	{
		WAVCODERHEAD_1 head_1;
		WAVCODERHEAD_2 head_2;
		WAVCODERHEAD_3 head_3;
	};

public:
	CarpWAVEncoder(int channel_num, int sample_rate)
	{
		m_head.head_1.riff_name[0] = 'R';
		m_head.head_1.riff_name[1] = 'I';
		m_head.head_1.riff_name[2] = 'F';
		m_head.head_1.riff_name[3] = 'F';
		m_head.head_1.riff_len = 36;

		m_head.head_1.wav_name[0] = 'W';
		m_head.head_1.wav_name[1] = 'A';
		m_head.head_1.wav_name[2] = 'V';
		m_head.head_1.wav_name[3] = 'E';

		m_head.head_1.fmt_name[0] = 'f';
		m_head.head_1.fmt_name[1] = 'm';
		m_head.head_1.fmt_name[2] = 't';
		m_head.head_1.fmt_name[3] = ' ';
		m_head.head_1.fmt_len = 16;

		m_head.head_2.audio_fmt = 6;
		m_head.head_2.channel = channel_num;
		m_head.head_2.sample_rate = sample_rate;
		m_head.head_2.bytes_per_second = sample_rate * channel_num;
		m_head.head_2.bytes_per_sample = 1;
		m_head.head_2.bits_per_sample = 8;

		m_head.head_3.data_name[0] = 'd';
		m_head.head_3.data_name[1] = 'a';
		m_head.head_3.data_name[2] = 't';
		m_head.head_3.data_name[3] = 'a';
		m_head.head_3.data_len = 0;
	}

public:
	int GetHeadSize() override { return static_cast<int>(sizeof(m_head)); }
	unsigned char* GetHeadMemory() override { return (unsigned char*)(&m_head); }
	int GetChannelNum() override { return m_head.head_2.channel; }

public:
	bool Encode(short* sample_data, int sample_count, std::vector<unsigned char>& buffer) override
	{
		if (m_pcma.Encode(sample_data, sample_count, buffer) == false)
			return false;

		m_head.head_1.riff_len += (int)buffer.size();
		m_head.head_3.data_len += (int)buffer.size();
		return true;
	}

private:
	WAVCODERHEAD m_head;
	CarpPCMAEncoder m_pcma;
};

class CarpWAVDecoder : public CarpAudioDecoder
{
public:
	struct WAVCODERHEAD_1
	{
		char riff_name[4] = { 0 };
		unsigned long riff_len = 0;
		char wav_name[4] = { 0 };
		char fmt_name[4] = { 0 };
		unsigned long fmt_len = 0;
	};

	struct WAVCODERHEAD_2
	{
		unsigned short audio_fmt = 0;
		unsigned short channel = 0;
		unsigned long sample_rate = 0;
		unsigned long bytes_per_second = 0;
		unsigned short bytes_per_sample = 0;
		unsigned short bits_per_sample = 0;
	};

	struct WAVCODERHEAD_3
	{
		char data_name[4] = { 0 };
		unsigned long data_len = 0;
	};

	struct WAVCODERHEAD
	{
		WAVCODERHEAD_1 head_1;
		WAVCODERHEAD_2 head_2;
		WAVCODERHEAD_3 head_3;
	};

public:
	~CarpWAVDecoder() { if (m_decoder) delete m_decoder; }

public:
	int GetChannelNum() override { return ((WAVCODERHEAD_2*)m_head_2)->channel; }
	int GetSampleRate() override { return ((WAVCODERHEAD_2*)m_head_2)->sample_rate; }

public:
	bool NeedReadHead() override { return true; }
	bool ReadHead(unsigned char* buffer, int buffer_len, std::vector<short>& sample_data, bool& is_completed) override
	{
		sample_data.resize(0);

		if (m_head_1_offset < sizeof(WAVCODERHEAD_1))
		{
			int copy_len = sizeof(WAVCODERHEAD_1) - m_head_1_offset;
			if (buffer_len < copy_len) copy_len = buffer_len;

			memcpy(((char*)&m_head_1) + m_head_1_offset, buffer, copy_len);
			m_head_1_offset += copy_len;

			if (m_head_1_offset < sizeof(WAVCODERHEAD_1))
			{
				is_completed = false;
				return true;
			}

			buffer += copy_len;
			buffer_len -= copy_len;

			if (m_head_1.fmt_len > 32)
			{
				// CARP_ERROR("WAVCODERHEAD_1.fmt_len is too large:" << m_head_1.fmt_len);
				is_completed = false;
				return false;
			}
		}

		if (m_head_2_offset < (int)m_head_1.fmt_len)
		{
			int copy_len = (int)m_head_1.fmt_len - m_head_2_offset;
			if (buffer_len < copy_len) copy_len = buffer_len;

			memcpy(m_head_2 + m_head_2_offset, buffer, copy_len);
			m_head_2_offset += copy_len;

			if (m_head_2_offset < (int)m_head_1.fmt_len)
			{
				is_completed = false;
				return true;
			}

			buffer += copy_len;
			buffer_len -= copy_len;

			WAVCODERHEAD_2* head_2 = (WAVCODERHEAD_2*)m_head_2;

			// check channel num
			if (head_2->channel != 1 && head_2->channel != 2)
			{
				// CARP_ERROR("WAVCODERHEAD_2.channel is error:" << head_2->channel);
				is_completed = false;
				return false;
			}

			// check format
			if (head_2->audio_fmt == 16 || head_2->audio_fmt == 1)
			{
				if (head_2->bytes_per_sample != head_2->channel * 2)
				{
					// CARP_ERROR("WAVCODERHEAD_2.bytes_per_sample is error(audio_fmt:16):" << head_2->bytes_per_sample);
					is_completed = false;
					return false;
				}
				m_decoder = new CarpPCMDecoder;
			}
			else if (head_2->audio_fmt == 6)
			{
				if (head_2->channel != head_2->bytes_per_sample)
				{
					// CARP_ERROR("WAVCODERHEAD_2.bytes_per_sample is error(audio_fmt:6):" << head_2->bytes_per_sample);
					is_completed = false;
					return false;
				}
				m_decoder = new CarpPCMADecoder;
			}
			else if (head_2->audio_fmt == 7)
			{
				if (head_2->channel != head_2->bytes_per_sample)
				{
					// CARP_ERROR("WAVCODERHEAD_2.bytes_per_sample is error(audio_fmt:7):" << head_2->bytes_per_sample);
					is_completed = false;
					return false;
				}
				m_decoder = new CarpPCMUDecoder;
			}
			else
			{
				// CARP_ERROR("WAVCODERHEAD_2.audio_fmt is error:" << head_2->audio_fmt);
				is_completed = false;
				return false;
			}
		}

		if (m_head_3_offset < sizeof(WAVCODERHEAD_3))
		{
			int copy_len = sizeof(WAVCODERHEAD_3) - m_head_3_offset;
			if (buffer_len < copy_len) copy_len = buffer_len;

			memcpy(((char*)&m_head_3) + m_head_3_offset, buffer, copy_len);
			m_head_3_offset += copy_len;

			if (m_head_3_offset < sizeof(WAVCODERHEAD_3))
			{
				is_completed = false;
				return true;
			}

			buffer += copy_len;
			buffer_len -= copy_len;

			m_handle_data_len = 0;
		}

		Decode(buffer, buffer_len, sample_data);

		is_completed = true;
		return true;
	}

public:
	bool Decode(unsigned char* buffer, int buffer_len, std::vector<short>& sample_data) override
	{
		sample_data.resize(0);

		if (m_decoder == nullptr) return false;

		int last_len = m_head_3.data_len - m_handle_data_len;
		if (last_len <= 0) return true;

		int real_buffer_len = buffer_len;
		if (last_len < real_buffer_len) real_buffer_len = last_len;

		bool result = m_decoder->Decode(buffer, real_buffer_len, sample_data);
		if (result == false) return false;

		m_handle_data_len += real_buffer_len;
		return true;
	}

private:
	WAVCODERHEAD_1 m_head_1 = { 0 };
	int m_head_1_offset = 0;

	char m_head_2[32] = { 0 };
	int m_head_2_offset = 0;

	WAVCODERHEAD_3 m_head_3 = { 0 };
	int m_head_3_offset = 0;

	int m_handle_data_len = 0;

private:
	CarpAudioDecoder* m_decoder = nullptr;
};

#endif
