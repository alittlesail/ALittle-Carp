#ifndef CARP_MIXER_INCLUDED
#define CARP_MIXER_INCLUDED (1)

typedef struct _carp_mixer_chunk_t
{
	short* data;
	int data_len;
	int channels;
	int sample_rate;
} carp_mixer_chunk_t;

#define CARP_MIXER_CHANNELS 32

typedef void (*CARP_MIXER_STOPED_FUNC)(int);

extern carp_mixer_chunk_t* carp_mixer_load_chunk(const char* buffer, size_t len);

/*
Please keep in mind that the stoped_func callback function is running in a
separate thread, if you need to share data with the main thread you need
to take care yourself to make the access to the shared data thread - safe!
*/
// loop <= 0 is Infinity
extern int carp_mixer_play_chunk(carp_mixer_chunk_t* chunk, float volume, int loop, CARP_MIXER_STOPED_FUNC stoped_func);
extern void carp_mixer_set_volume(int channel, float volume);
extern float carp_mixer_get_volume(int channel);
extern void carp_mixer_stop_chunk(carp_mixer_chunk_t* chunk);
extern void carp_mixer_stop_channel(int channel);
extern void carp_mixer_free_chunk(carp_mixer_chunk_t* chunk);
extern void carp_mixer_shutdown();

#ifdef CARP_MIXER_IMPL

#define SOKOL_IMPL
#include "sokol/sokol_audio.h"

#include "stb/stb_vorbis.c"

typedef struct 
{
	float volume;
	carp_mixer_chunk_t* chunk;
	int cursor;
	int loop;
	CARP_MIXER_STOPED_FUNC stoped_cb;
} _carp_mixer_channel_t;

typedef struct
{
	bool setup;
	_carp_mixer_channel_t channel[CARP_MIXER_CHANNELS];
	_saudio_mutex_t mutex;
} _carp_mixer_t;
static _carp_mixer_t _carp_mixer = { 0 };

static carp_mixer_chunk_t* carp_mixer_load_chunk(const char* buffer, size_t len)
{
	int channels = 0;
	int sample_rate = 0;
	short* output = 0;
	int data_len = stb_vorbis_decode_memory((const unsigned char*)buffer, (int)len, &channels, &sample_rate, &output);
	if (data_len < 0) return nullptr;

	carp_mixer_chunk_t* chunk = (carp_mixer_chunk_t*)malloc(sizeof(carp_mixer_chunk_t));
	memset(chunk, 0, sizeof(carp_mixer_chunk_t));
	chunk->data = output;
	chunk->channels = channels;
	chunk->sample_rate = sample_rate;
	chunk->data_len = data_len;
	return chunk;
}

static void _carp_mixer_stream_cb(float* buffer, int num_frames, int num_channels)
{
	_saudio_mutex_lock(&_carp_mixer.mutex);
	// define stoped
	static CARP_MIXER_STOPED_FUNC stoped_func[CARP_MIXER_CHANNELS] = { 0 };
	bool has_stoped = false;

	// collect stoped
	for (int j = 0; j < CARP_MIXER_CHANNELS; ++j)
	{
		_carp_mixer_channel_t* channel = _carp_mixer.channel + j;
		if (channel->chunk == 0) continue;
		if (channel->cursor >= channel->chunk->data_len)
		{
			channel->cursor = 0;
			if (channel->loop <= 0) continue;
			--channel->loop;
			if (channel->loop == 0)
			{
				if (channel->stoped_cb)
				{
					stoped_func[j] = channel->stoped_cb;
					has_stoped = true;
				}
				memset(channel, 0, sizeof(_carp_mixer_channel_t));
			}
		}
	}

	// handle stoped
	if (has_stoped)
	{
		for (int i = 0; i < CARP_MIXER_CHANNELS; ++i)
		{
			if (stoped_func[i] == 0) continue;

			stoped_func[i](i);
			stoped_func[i] = 0;
		}
	}

	if (num_channels == 1)
	{
		for (int i = 0; i < num_frames; ++i)
		{
			buffer[i] = 0.0f;
			for (int j = 0; j < CARP_MIXER_CHANNELS; ++j)
			{
				_carp_mixer_channel_t* channel = _carp_mixer.channel + j;
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
				_carp_mixer_channel_t* channel = _carp_mixer.channel + j;
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
	_saudio_mutex_unlock(&_carp_mixer.mutex);
}

/*
Please keep in mind that the stoped_func callback function is running in a
separate thread, if you need to share data with the main thread you need
to take care yourself to make the access to the shared data thread - safe!
*/
// loop <= 0 is Infinity
static int carp_mixer_play_chunk(carp_mixer_chunk_t* chunk, float volume, int loop, CARP_MIXER_STOPED_FUNC stoped_func)
{
	if (chunk == 0) return -1;
	
	if (!_carp_mixer.setup)
	{
		_carp_mixer.setup = true;

		saudio_desc desc = { 0 };
		desc.num_channels = chunk->channels;
		desc.sample_rate = chunk->sample_rate;
		desc.stream_cb = _carp_mixer_stream_cb;
		saudio_setup(&desc);

		_saudio_mutex_init(&_carp_mixer.mutex);
	}
	if (!saudio_isvalid()) return -1;

	if (chunk->sample_rate != saudio_sample_rate()) return -1;
	if (chunk->channels != saudio_channels()) return -1;

	_saudio_mutex_lock(&_carp_mixer.mutex);
	for (int i = 0; i < CARP_MIXER_CHANNELS; ++i)
	{
		_carp_mixer_channel_t* channel = _carp_mixer.channel + i;
		if (channel->chunk != 0) continue;

		channel->chunk = chunk;
		channel->volume = volume;
		channel->cursor = 0;
		channel->loop = loop;
		channel->stoped_cb = stoped_func;

		_saudio_mutex_unlock(&_carp_mixer.mutex);
		return i;
	}

	_saudio_mutex_unlock(&_carp_mixer.mutex);
	return -1;
}

static void carp_mixer_set_volume(int channel, float volume)
{
	if (channel < 0 || channel >= CARP_MIXER_CHANNELS) return;
	_carp_mixer.channel[channel].volume = volume;
}

static float carp_mixer_get_volume(int channel)
{
	if (channel < 0 || channel >= CARP_MIXER_CHANNELS) return 0.0f;
	return _carp_mixer.channel[channel].volume;
}

static void carp_mixer_stop_chunk(carp_mixer_chunk_t* chunk)
{
	_saudio_mutex_lock(&_carp_mixer.mutex);
	for (int i = 0; i < CARP_MIXER_CHANNELS; ++i)
	{
		_carp_mixer_channel_t* channel = _carp_mixer.channel + i;
		if (channel->chunk != chunk) continue;
		memset(channel, 0, sizeof(_carp_mixer_channel_t));
	}
	_saudio_mutex_unlock(&_carp_mixer.mutex);
}

static void carp_mixer_stop_channel(int channel)
{
	if (channel < 0 || channel >= CARP_MIXER_CHANNELS) return;

	_saudio_mutex_lock(&_carp_mixer.mutex);
	memset(_carp_mixer.channel + channel, 0, sizeof(_carp_mixer_channel_t));
	_saudio_mutex_unlock(&_carp_mixer.mutex);
}

static void carp_mixer_free_chunk(carp_mixer_chunk_t* chunk)
{
	if (chunk == 0) return;
	carp_mixer_stop_chunk(chunk);
	free(chunk->data);
	free(chunk);
}

static void carp_mixer_shutdown()
{
	if (!_carp_mixer.setup) return;
	_carp_mixer.setup = false;
	saudio_shutdown();
	_saudio_mutex_destroy(&_carp_mixer.mutex);
}

#endif
#endif