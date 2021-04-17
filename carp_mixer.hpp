
#ifndef CARP_MIXER_INCLUDED
#define CARP_MIXER_INCLUDED

#include <mutex>
#include <vector>
#include "SDL.h"
#define STB_VORBIS_HEADER_ONLY
#include "stb/stb_vorbis.c"

#define CARP_MIXER_CHANNELS 32

typedef void (*CARP_MIXER_STOPPED_FUNC)(int);

class CarpMixer;

struct CarpMixerChunk
{
	short* data = nullptr;
	int data_len = 0;
	int channels = 0;
	int sample_rate = 0;
};

struct CarpMixerChannel
{
	float volume = 0.0f;
	CarpMixerChunk* chunk = nullptr;
	int cursor = 0;
	int loop = 0;
	CARP_MIXER_STOPPED_FUNC stopped_cb = nullptr;
};

class CarpMixer
{
public:
	bool Setup(int sample_rate, int channels)
	{
		if (m_setup)
		{
			SDL_CloseAudio();
			m_setup = false;
		}

		SDL_AudioSpec desired = { 0 };

		desired.freq = sample_rate;
		desired.format = AUDIO_S16LSB;
		desired.channels = channels;
		desired.samples = 4096;
		desired.callback = StreamCallback;
		desired.userdata = this;
		if (SDL_OpenAudio(&desired, nullptr) != 0) {
			return false;
		}
		SDL_PauseAudio(0);

		m_sample_rate = sample_rate;
		m_channel_num = channels;
		m_setup = true;

		return true;
	}

	void Shutdown()
	{
		if (!m_setup) return;

		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_stream)
		{
			SDL_FreeAudioStream(m_stream);
			m_stream = nullptr;
			m_stream_channels = 0;
		}

		m_setup = false;
		SDL_CloseAudio();
	}

public:
	CarpMixerChunk* LoadChunk(const char* buffer, size_t len)
	{
		int channels = 0;
		int sample_rate = 0;
		short* output = nullptr;
		int data_len = stb_vorbis_decode_memory((const unsigned char*)buffer, (int)len, &channels, &sample_rate, &output);
		if (data_len <= 0) return nullptr;

		if (!m_setup)
		{
			if (!Setup(sample_rate, channels))
			{
				free(output);
				return nullptr;
			}
		}

		if (channels != m_channel_num || sample_rate != m_sample_rate)
		{
			SDL_AudioCVT cvt;
			const int result = SDL_BuildAudioCVT(&cvt, AUDIO_S16LSB, channels, sample_rate, AUDIO_S16LSB, m_channel_num, m_sample_rate);
			if (result < 0)
			{
				free(output);
				return nullptr;
			}

			if (result > 0)
			{
				cvt.buf = static_cast<Uint8*>(malloc(cvt.len * cvt.len_mult));
				cvt.len = data_len;
				memcpy(cvt.buf, output, cvt.len);
				free(output);
				if (SDL_ConvertAudio(&cvt) < 0)
				{
					free(cvt.buf);
					return nullptr;
				}
				output = reinterpret_cast<short*>(cvt.buf);
				data_len = cvt.len_cvt;
				sample_rate = m_sample_rate;
				channels = m_channel_num;
			}
		}

		auto* chunk = static_cast<CarpMixerChunk*>(malloc(sizeof(CarpMixerChunk)));
		chunk->data = output;
		chunk->channels = channels;
		chunk->sample_rate = sample_rate;
		chunk->data_len = data_len;
		return chunk;
	}

	void FreeChunk(CarpMixerChunk* chunk)
	{
		if (chunk == nullptr) return;
		StopChunk(chunk);
		free(chunk->data);
		free(chunk);
	}

	/*
	Please keep in mind that the stopped_func callback function is running in a
	separate thread, if you need to share data with the main thread you need
	to take care yourself to make the access to the shared data thread - safe!
	*/
	// loop <= 0 is Infinity
	int PlayChunk(CarpMixerChunk* chunk, float volume, int loop, CARP_MIXER_STOPPED_FUNC stopped_func)
	{
		if (chunk == nullptr) return -1;
		if (!m_setup) return -1;

		if (chunk->sample_rate != m_sample_rate) return -1;
		if (chunk->channels != m_channel_num) return -1;

		m_mutex.lock();
		for (int i = 0; i < CARP_MIXER_CHANNELS; ++i)
		{
			CarpMixerChannel* channel = m_channel + i;
			if (channel->chunk != nullptr) continue;

			channel->chunk = chunk;
			channel->volume = volume;
			channel->cursor = 0;
			channel->loop = loop;
			channel->stopped_cb = stopped_func;

			m_mutex.unlock();
			return i;
		}

		m_mutex.unlock();
		return -1;
	}

	void SetChannelVolume(int channel, float volume)
	{
		if (channel < 0 || channel >= CARP_MIXER_CHANNELS) return;
		m_channel[channel].volume = volume;
	}

	float GetChannelVolume(int channel) const
	{
		if (channel < 0 || channel >= CARP_MIXER_CHANNELS) return 0.0f;
		return m_channel[channel].volume;
	}

	void StopChunk(CarpMixerChunk* chunk)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		for (int i = 0; i < CARP_MIXER_CHANNELS; ++i)
		{
			CarpMixerChannel* channel = m_channel + i;
			if (channel->chunk != chunk) continue;
			memset(channel, 0, sizeof(CarpMixerChannel));
		}
	}

	void StopChannel(int channel)
	{
		if (channel < 0 || channel >= CARP_MIXER_CHANNELS) return;

		std::lock_guard<std::mutex> lock(m_mutex);
		memset(m_channel + channel, 0, sizeof(CarpMixerChannel));
	}

public:
	bool StartStream(int sample_rate, int channels)
	{
		if (!m_setup)
		{
			if (!Setup(sample_rate, channels))
				return false;
		}

		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_stream)
		{
			SDL_FreeAudioStream(m_stream);
			m_stream = nullptr;
			m_stream_channels = 0;
			m_stream_data.resize(0);
		}

		m_stream = SDL_NewAudioStream(AUDIO_S16LSB, channels, sample_rate, AUDIO_S16LSB, m_channel_num, m_sample_rate);
		m_stream_channels = channels;
		m_stream_sample_rate = sample_rate;
	    return m_stream != nullptr;
	}

	bool PushStreamSample(short left_sample, short right_sample)
	{
		if (m_stream_channels >= 1)
			m_stream_data.push_back(left_sample);

		if (m_stream_channels >= 2)
			m_stream_data.push_back(right_sample);

		if (m_stream_data.size() % (m_stream_sample_rate * m_stream_channels) != 0) return true;

		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_stream == nullptr)
		    return false;

		const int result = SDL_AudioStreamPut(m_stream, m_stream_data.data(), static_cast<int>(m_stream_data.size() * sizeof(short)));
		m_stream_data.resize(0);
		return true;
	}

	void SetStreamVolume(float volume)
	{
		m_stream_volume = volume;
	}

	float GetStreamVolume() const
	{
		return m_stream_volume;
	}

	void StopStream()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_stream)
		{
			SDL_FreeAudioStream(m_stream);
			m_stream = nullptr;
			m_stream_channels = 0;
			m_stream_data.resize(0);
		}
	}

private:
	static void StreamCallback(void* user_data, Uint8* stream, int len)
	{
		memset(stream, 0, len);
		auto* self = static_cast<CarpMixer*>(user_data);

		std::lock_guard<std::mutex> lock(self->m_mutex);
		// define stopped
		static CARP_MIXER_STOPPED_FUNC stopped_func[CARP_MIXER_CHANNELS] = { nullptr };
		bool has_stopped = false;

		// collect stopped
		for (int j = 0; j < CARP_MIXER_CHANNELS; ++j)
		{
			CarpMixerChannel* channel = self->m_channel + j;
			if (channel->chunk == nullptr) continue;
			if (channel->cursor >= channel->chunk->data_len)
			{
				channel->cursor = 0;
				if (channel->loop <= 0) continue;
				--channel->loop;
				if (channel->loop == 0)
				{
					if (channel->stopped_cb)
					{
						stopped_func[j] = channel->stopped_cb;
						has_stopped = true;
					}
					memset(channel, 0, sizeof(CarpMixerChannel));
				}
			}
		}

		// handle stopped
		if (has_stopped)
		{
			for (int i = 0; i < CARP_MIXER_CHANNELS; ++i)
			{
				if (stopped_func[i] == nullptr) continue;

				stopped_func[i](i);
				stopped_func[i] = nullptr;
			}
		}

		if (self->m_channel_num == 1)
		{
			int num_frames = len / sizeof(short);
			short* buffer = reinterpret_cast<short*>(stream);
			for (int i = 0; i < num_frames; ++i)
			{
				for (int j = 0; j < CARP_MIXER_CHANNELS; ++j)
				{
					CarpMixerChannel* channel = self->m_channel + j;
					if (channel->chunk == 0) continue;
					if (channel->cursor >= channel->chunk->data_len) continue;
					int sample = channel->chunk->data[channel->cursor];
					channel->cursor += channel->chunk->channels;

					sample = static_cast<int>(floor(sample * channel->volume * channel->volume)) + buffer[i];
					if (sample > 32767) buffer[i] = 32767;
					else if (sample < -32768) buffer[i] = -32768;
					else buffer[i] = static_cast<short>(sample);
				}
			}

			if (self->m_stream)
			{
				const auto buff_frames = SDL_AudioStreamAvailable(self->m_stream) / sizeof(short);
				if (buff_frames > 0)
				{
					std::vector<short> stream_buffer;
					stream_buffer.resize(buff_frames < num_frames ? buff_frames : num_frames);
					SDL_AudioStreamGet(self->m_stream, static_cast<void*>(stream_buffer.data()), static_cast<int>(stream_buffer.size() * sizeof(short)));
					for (int i = 0; i < num_frames; ++i)
					{
						if (i >= static_cast<int>(stream_buffer.size() * sizeof(short))) break;

						int sample = static_cast<int>(floor(stream_buffer[i] * self->m_stream_volume * self->m_stream_volume)) + buffer[i];
						if (sample > 32767) buffer[i] = 32767;
						else if (sample < -32768) buffer[i] = -32768;
						else buffer[i] = static_cast<short>(sample);
					}
				}
			}
		}
		else if (self->m_channel_num == 2)
		{
			const int num_frames = len / sizeof(short) / 2;
			short* buffer = reinterpret_cast<short*>(stream);
			for (int i = 0; i < num_frames; ++i)
			{
				const int left = i * 2;
				const int right = left + 1;
				for (int j = 0; j < CARP_MIXER_CHANNELS; ++j)
				{
					CarpMixerChannel* channel = self->m_channel + j;
					if (channel->chunk == nullptr) continue;
					if (channel->cursor + 1 >= channel->chunk->data_len) continue;
					
					int left_sample = channel->chunk->data[channel->cursor];
					int right_sample = channel->chunk->data[channel->cursor + 1];
					channel->cursor += channel->chunk->channels;

					left_sample = static_cast<int>(floor(left_sample * channel->volume * channel->volume)) + buffer[left];
					if (left_sample > 32767) buffer[left] = 32767;
					else if (left_sample < -32768) buffer[left] = -32768;
					else buffer[left] = static_cast<short>(left_sample);

					right_sample = static_cast<int>(floor(right_sample * channel->volume * channel->volume)) + buffer[right];
					if (right_sample > 32767) buffer[right] = 32767;
					else if (right_sample < -32768) buffer[right] = -32768;
					else buffer[right] = static_cast<short>(right_sample);
				}
			}

			if (self->m_stream)
			{
				const auto buff_len = SDL_AudioStreamAvailable(self->m_stream);
				if (buff_len > 0)
				{
					const auto min_len = buff_len < len ? buff_len : len;
					std::vector<short> stream_buffer;
					stream_buffer.resize(min_len / sizeof(short));
					int result = SDL_AudioStreamGet(self->m_stream, static_cast<void*>(stream_buffer.data()), static_cast<int>(stream_buffer.size() * sizeof(short)));
					for (size_t i = 0; i < min_len / sizeof(short) / 2; ++i)
					{
						const auto left = i * 2;
						const auto right = left + 1;
						int left_sample = stream_buffer[left];
						int right_sample = stream_buffer[right];
						
						left_sample = static_cast<int>(floor(left_sample * self->m_stream_volume * self->m_stream_volume)) + buffer[left];
						if (left_sample > 32767) buffer[left] = 32767;
						else if (left_sample < -32768) buffer[left] = -32768;
						else buffer[left] = static_cast<short>(left_sample);

						right_sample = static_cast<int>(floor(right_sample * self->m_stream_volume * self->m_stream_volume)) + buffer[right];
						if (right_sample > 32767) buffer[right] = 32767;
						else if (right_sample < -32768) buffer[right] = -32768;
						else buffer[right] = static_cast<short>(right_sample);
					}
				}
			}
		}
	}

private:
	CarpMixerChannel m_channel[CARP_MIXER_CHANNELS];

private:
	std::vector<short> m_stream_data;
	SDL_AudioStream* m_stream = nullptr;
	int m_stream_channels = 0;
	int m_stream_sample_rate = 0;
	float m_stream_volume = 1.0f;

private:
	bool m_setup = false;
	std::mutex m_mutex;
	int m_sample_rate = 0;
	int m_channel_num = 0;
};

extern CarpMixer s_carp_mixer;

#endif

#ifdef CARP_MIXER_IMPL
#ifndef CARP_MIXER_IMPL_INCLUDE
#define CARP_MIXER_IMPL_INCLUDE
CarpMixer s_carp_mixer;
#undef STB_VORBIS_HEADER_ONLY
#include "stb/stb_vorbis.c"
#endif
#endif