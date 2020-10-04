
#ifndef CARP_MIXER_INCLUDED
#define CARP_MIXER_INCLUDED

#include <mutex>
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
	CarpMixerChunk* LoadChunk(const char* buffer, size_t len)
	{
		int channels = 0;
		int sample_rate = 0;
		short* output = 0;
		const int data_len = stb_vorbis_decode_memory((const unsigned char*)buffer, (int)len, &channels, &sample_rate, &output);
		if (data_len < 0) return nullptr;

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

		if (!m_setup)
		{
			SDL_AudioSpec desired = {0};

			desired.freq = chunk->sample_rate;
			desired.format = AUDIO_S16LSB;
			desired.channels = chunk->channels;
			desired.samples = 4096;
			desired.callback = StreamCallback;
			desired.userdata = this;
			if (SDL_OpenAudio(&desired, nullptr) != 0) {
				return -1;
			}

			m_sample_rate = chunk->sample_rate;
			m_channel_num = chunk->channels;
			m_setup = true;
		}

		if (chunk->sample_rate != m_sample_rate) return -1;
		if (chunk->channels != m_channel_num) return -1;

		m_mutex.lock();
		for (int i = 0; i < CARP_MIXER_CHANNELS; ++i)
		{
			CarpMixerChannel* channel = m_channel + i;
			if (channel->chunk != 0) continue;

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

	void SetVolume(int channel, float volume)
	{
		if (channel < 0 || channel >= CARP_MIXER_CHANNELS) return;
		m_channel[channel].volume = volume;
	}

	float GetVolume(int channel) const
	{
		if (channel < 0 || channel >= CARP_MIXER_CHANNELS) return 0.0f;
		return m_channel[channel].volume;
	}
	
	void StopChunk(CarpMixerChunk* chunk)
	{
		m_mutex.lock();
		for (int i = 0; i < CARP_MIXER_CHANNELS; ++i)
		{
			CarpMixerChannel* channel = m_channel + i;
			if (channel->chunk != chunk) continue;
			memset(channel, 0, sizeof(CarpMixerChannel));
		}
		m_mutex.unlock();
	}

	void StopChannel(int channel)
	{
		if (channel < 0 || channel >= CARP_MIXER_CHANNELS) return;

		m_mutex.lock();
		memset(m_channel + channel, 0, sizeof(CarpMixerChannel));
		m_mutex.unlock();
	}

public:
	void Shutdown()
	{
		if (!m_setup) return;
		m_setup = false;
		SDL_CloseAudio();
	}

private:
	static void StreamCallback(void* user_data, Uint8* stream, int len)
	{
		auto* self = static_cast<CarpMixer*>(user_data);

		self->m_mutex.lock();
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
			short* buffer = (short*)stream;
			for (int i = 0; i < num_frames; ++i)
			{
				buffer[i] = 0;
				for (int j = 0; j < CARP_MIXER_CHANNELS; ++j)
				{
					CarpMixerChannel* channel = self->m_channel + j;
					if (channel->chunk == 0) continue;
					if (channel->cursor >= channel->chunk->data_len) continue;
					int sample = channel->chunk->data[channel->cursor];
					channel->cursor += channel->chunk->channels;

					sample = static_cast<int>(floor(sample * channel->volume * channel->volume)) + buffer[i];
					if (sample > 32767) buffer[i] = 32767;
					else if (buffer[i] < -32768) buffer[i] = -32768;
					else buffer[i] = static_cast<short>(sample);
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
				int left_sample = 0;
				int right_sample = 0;
				buffer[left] = 0;
				buffer[right] = 0;
				for (int j = 0; j < CARP_MIXER_CHANNELS; ++j)
				{
					CarpMixerChannel* channel = self->m_channel + j;
					if (channel->chunk == 0) continue;
					if (channel->cursor + 1 >= channel->chunk->data_len)
						left_sample = channel->chunk->data[channel->cursor];
					right_sample = channel->chunk->data[channel->cursor + 1];
					channel->cursor += channel->chunk->channels;

					left_sample = static_cast<int>(floor(left_sample * channel->volume * channel->volume)) + buffer[left];
					if (left_sample > 32767) buffer[left] = 32767;
					else if (buffer[left] < -32768) buffer[left] = -32768;
					else buffer[left] = (short)left_sample;

					right_sample = static_cast<int>(floor(right_sample * channel->volume * channel->volume)) + buffer[right];
					if (right_sample > 32767) buffer[right] = 32767;
					else if (buffer[right] < -32768) buffer[right] = -32768;
					else buffer[right] = static_cast<short>(right_sample);
				}
			}
		}
		else
		{
			memset(stream, 0, len);
		}
		self->m_mutex.unlock();
	}

private:
	bool m_setup = false;
	CarpMixerChannel m_channel[CARP_MIXER_CHANNELS];
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