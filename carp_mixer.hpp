
#ifndef CARP_MIXER_INCLUDED
#define CARP_MIXER_INCLUDED (1)

#include <mutex>
#include "sokol/sokol_audio.h"
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
		int data_len = stb_vorbis_decode_memory((const unsigned char*)buffer, (int)len, &channels, &sample_rate, &output);
		if (data_len < 0) return nullptr;

		CarpMixerChunk* chunk = (CarpMixerChunk*)malloc(sizeof(CarpMixerChunk));
		chunk->data = output;
		chunk->channels = channels;
		chunk->sample_rate = sample_rate;
		chunk->data_len = data_len;
		return chunk;
	}

	void FreeChunk(CarpMixerChunk* chunk)
	{
		if (chunk == 0) return;
		StopChunk(chunk);
		free(chunk->data);
		free(chunk);
	}

	/*
	Please keep in mind that the stoped_func callback function is running in a
	separate thread, if you need to share data with the main thread you need
	to take care yourself to make the access to the shared data thread - safe!
	*/
	// loop <= 0 is Infinity
	int PlayChunk(CarpMixerChunk* chunk, float volume, int loop, CARP_MIXER_STOPPED_FUNC stopped_func)
	{
		if (chunk == 0) return -1;

		if (!m_setup)
		{
			m_setup = true;

			saudio_desc desc = { 0 };
			desc.num_channels = chunk->channels;
			desc.sample_rate = chunk->sample_rate;
			desc.user_data = this;
			desc.stream_userdata_cb = StreamCallback;
			saudio_setup(&desc);
		}
		if (!saudio_isvalid()) return -1;

		if (chunk->sample_rate != saudio_sample_rate()) return -1;
		if (chunk->channels != saudio_channels()) return -1;

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
		saudio_shutdown();
	}

private:
	static void StreamCallback(float* buffer, int num_frames, int num_channels, void* user_data)
	{
		CarpMixer* self = (CarpMixer*)user_data;

		self->m_mutex.lock();
		// define stopped
		static CARP_MIXER_STOPPED_FUNC stopped_func[CARP_MIXER_CHANNELS] = { 0 };
		bool has_stopped = false;

		// collect stopped
		for (int j = 0; j < CARP_MIXER_CHANNELS; ++j)
		{
			CarpMixerChannel* channel = self->m_channel + j;
			if (channel->chunk == 0) continue;
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

		if (num_channels == 1)
		{
			for (int i = 0; i < num_frames; ++i)
			{
				buffer[i] = 0.0f;
				for (int j = 0; j < CARP_MIXER_CHANNELS; ++j)
				{
					CarpMixerChannel* channel = self->m_channel + j;
					if (channel->chunk == 0) continue;
					if (channel->cursor >= channel->chunk->data_len) continue;
					float sample = channel->chunk->data[channel->cursor] / 65535.0f;
					channel->cursor += channel->chunk->channels;

					buffer[i] = sample * (channel->volume * channel->volume) + buffer[i];
					if (buffer[i] > 1.0f) buffer[i] = 1.0f;
					else if (buffer[i] < -1.0f) buffer[i] = -1.0f;
				}
			}
		}
		else if (num_channels == 2)
		{
			for (int i = 0; i < num_frames; ++i)
			{
				int left = i * 2;
				int right = left + 1;
				float left_sample = 0.0f;
				float right_sample = 0.0f;
				buffer[left] = 0.0f;
				buffer[right] = 0.0f;
				for (int j = 0; j < CARP_MIXER_CHANNELS; ++j)
				{
					CarpMixerChannel* channel = self->m_channel + j;
					if (channel->chunk == 0) continue;
					if (channel->cursor + 1 >= channel->chunk->data_len)
						left_sample = channel->chunk->data[channel->cursor] / 32767.0f;
					right_sample = channel->chunk->data[channel->cursor + 1] / 32767.0f;
					channel->cursor += channel->chunk->channels;

					buffer[left] = left_sample * (channel->volume * channel->volume) + buffer[left];
					if (buffer[left] > 1.0f) buffer[left] = 1.0f;
					else if (buffer[left] < -1.0f) buffer[left] = -1.0f;

					buffer[right] = right_sample * (channel->volume * channel->volume) + buffer[right];
					if (buffer[right] > 1.0f) buffer[right] = 1.0f;
					else if (buffer[right] < -1.0f) buffer[right] = -1.0f;
				}
			}
		}
		self->m_mutex.unlock();
	}

private:
	bool m_setup = false;
	CarpMixerChannel m_channel[CARP_MIXER_CHANNELS];
	std::mutex m_mutex;
};

extern CarpMixer s_carp_mixer;

#endif

#ifdef CARP_MIXER_IMPL
#ifndef CARP_MIXER_IMPL_INCLUDE
#define CARP_MIXER_IMPL_INCLUDE
CarpMixer s_carp_mixer;
#define SOKOL_IMPL
#include "sokol/sokol_audio.h"
#undef STB_VORBIS_HEADER_ONLY
#include "stb/stb_vorbis.c"
#endif
#endif