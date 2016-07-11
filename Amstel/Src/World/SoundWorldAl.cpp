// Copyright (c) 2016 Volodymyr Syvochka
#include "Config.h"

#if RIO_SOUND_OPENAL

#include "Core/Containers/Array.h"
#include "Core/Math/Matrix4x4.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Math/Vector3.h"
#include "Device/Log.h"
#include "Resource/SoundResource.h"
#include "World/SoundWorld.h"
#include "World/Audio.h"

#include <AL/al.h>
#include <AL/alc.h>

namespace Rio
{

#if RIO_DEBUG
	static const char* getStringFromAlError(ALenum error)
	{
		switch (error)
		{
			case AL_INVALID_ENUM: return "AL_INVALID_ENUM";
			case AL_INVALID_VALUE: return "AL_INVALID_VALUE";
			case AL_INVALID_OPERATION: return "AL_INVALID_OPERATION";
			case AL_OUT_OF_MEMORY: return "AL_OUT_OF_MEMORY";
			default: return "UNKNOWN_AL_ERROR";
		}
	}

	#define AL_CHECK(function)\
		function;\
		do { ALenum error; RIO_ASSERT((error = alGetError()) == AL_NO_ERROR,\
				"OpenAL error: %s", getStringFromAlError(error)); } while (0)
#else
	#define AL_CHECK(function) function
#endif // RIO_DEBUG

// Global audio-related functions
namespace AudioGlobalFn
{
	void init()
	{
		alcDevice = alcOpenDevice(NULL);
		RIO_ASSERT(alcDevice, "Cannot open OpenAL audio device");

		alcContext = alcCreateContext(alcDevice, NULL);
		RIO_ASSERT(alcContext, "Cannot create OpenAL context");

		AL_CHECK(alcMakeContextCurrent(alcContext));

		RIO_LOGD("OpenAL Vendor : %s", alGetString(AL_VENDOR));
		RIO_LOGD("OpenAL Version : %s", alGetString(AL_VERSION));
		RIO_LOGD("OpenAL Renderer : %s", alGetString(AL_RENDERER));

		AL_CHECK(alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED));
		AL_CHECK(alDopplerFactor(1.0f));
		AL_CHECK(alDopplerVelocity(343.0f));
	}

	void shutdown()
	{
		alcDestroyContext(alcContext);
	    alcCloseDevice(alcDevice);
	}

	static ALCdevice* alcDevice;
	static ALCcontext* alcContext;
}

struct SoundInstance
{
	void create(const SoundResource& soundResource, const Vector3& position, float range)
	{
		using namespace SoundResourceFn;

		AL_CHECK(alGenSources(1, &alSource));
		RIO_ASSERT(alIsSource(alSource), "alGenSources: error");

		AL_CHECK(alSourcef(alSource, AL_REFERENCE_DISTANCE, 0.01f));
		AL_CHECK(alSourcef(alSource, AL_MAX_DISTANCE, range));
		AL_CHECK(alSourcef(alSource, AL_PITCH, 1.0f));

		// Generates AL buffers
		AL_CHECK(alGenBuffers(1, &buffer));
		RIO_ASSERT(alIsBuffer(buffer), "alGenBuffers: error");

		ALenum formatList = AL_INVALID_ENUM;
		switch (soundResource.bitsPerSample)
		{
			case 8: formatList = soundResource.channels > 1 ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8; break;
			case 16: formatList = soundResource.channels > 1 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16; break;
			default: RIO_FATAL("Number of bits per sample not supported."); break;
		}
		AL_CHECK(alBufferData(buffer, formatList, getData(&soundResource), soundResource.size, soundResource.sampleRate));

		this->soundResource = &soundResource;
		setPosition(position);
	}

	void destroy()
	{
		stop();
		AL_CHECK(alSourcei(alSource, AL_BUFFER, 0));
		AL_CHECK(alDeleteBuffers(1, &buffer));
		AL_CHECK(alDeleteSources(1, &alSource));
	}

	void reload(const SoundResource& newSoundResource)
	{
		destroy();
		create(newSoundResource, getPosition(), getRange());
	}

	void play(bool loop, float volume)
	{
		setVolume(volume);
		AL_CHECK(alSourcei(alSource, AL_LOOPING, (loop ? AL_TRUE : AL_FALSE)));
		AL_CHECK(alSourceQueueBuffers(alSource, 1, &buffer));
		AL_CHECK(alSourcePlay(alSource));
	}

	void pause()
	{
		AL_CHECK(alSourcePause(alSource));
	}

	void resume()
	{
		AL_CHECK(alSourcePlay(alSource));
	}

	void stop()
	{
		AL_CHECK(alSourceStop(alSource));
		AL_CHECK(alSourceRewind(alSource)); // TODO Workaround
		ALint processed;
		AL_CHECK(alGetSourcei(alSource, AL_BUFFERS_PROCESSED, &processed));

		if (processed > 0)
		{
			ALuint removed;
			AL_CHECK(alSourceUnqueueBuffers(alSource, 1, &removed));
		}
	}

	bool getIsPlaying()
	{
		ALint state;
		AL_CHECK(alGetSourcei(alSource, AL_SOURCE_STATE, &state));
		return state == AL_PLAYING;
	}

	bool getIsFinished()
	{
		ALint state;
		AL_CHECK(alGetSourcei(alSource, AL_SOURCE_STATE, &state));
		return (state != AL_PLAYING && state != AL_PAUSED);
	}

	Vector3 getPosition()
	{
		ALfloat position[3];
		AL_CHECK(alGetSourcefv(alSource, AL_POSITION, position));
		return createVector3(position[0], position[1], position[2]);
	}

	float getRange()
	{
		ALfloat range;
		AL_CHECK(alGetSourcefv(alSource, AL_MAX_DISTANCE, &range));
		return range;
	}

	void setPosition(const Vector3& position)
	{
		AL_CHECK(alSourcefv(alSource, AL_POSITION, getFloatPointer(position)));
	}

	void setRange(float range)
	{
		AL_CHECK(alSourcef(alSource, AL_MAX_DISTANCE, range));
	}

	void setVolume(float volume)
	{
		AL_CHECK(alSourcef(alSource, AL_GAIN, volume));
	}

	const SoundResource* getSoundResource()
	{
		return soundResource;
	}

public:
	const SoundResource* soundResource;
	SoundInstanceId id;
	ALuint buffer;
	ALuint alSource;
};

#define MAX_OBJECTS 1024
#define INDEX_MASK 0xffff
#define NEW_OBJECT_ID_ADD 0x10000

class ALSoundWorld : public SoundWorld
{
private:
	struct SoundIndex
	{
		SoundInstanceId id;
		uint16_t index;
		uint16_t next;
	};
public:
	ALSoundWorld()
	{
		for (uint32_t i = 0; i < MAX_OBJECTS; ++i)
		{
			soundIndexList[i].id = i;
			soundIndexList[i].next = i + 1;
		}
		setListenerPose(MATRIX4X4_IDENTITY);
	}

	virtual ~ALSoundWorld()
	{
	}

	virtual SoundInstanceId play(const SoundResource& soundResource, bool loop, float volume, float range, const Vector3& position)
	{
		SoundInstanceId soundInstanceId = add();
		SoundInstance& soundInstance = lookup(soundInstanceId);
		soundInstance.create(soundResource, position, range);
		soundInstance.play(loop, volume);
		return soundInstanceId;
	}

	virtual void stop(SoundInstanceId soundInstanceId)
	{
		SoundInstance& soundInstance = lookup(soundInstanceId);
		soundInstance.destroy();
		remove(soundInstanceId);
	}

	virtual bool getIsPlaying(SoundInstanceId soundInstanceId)
	{
		return has(soundInstanceId) && lookup(soundInstanceId).getIsPlaying();
	}

	virtual void stopAll()
	{
		for (uint32_t i = 0; i < soundIndexListCount; ++i)
		{
			playingSounds[i].stop();
		}
	}

	virtual void pauseAll()
	{
		for (uint32_t i = 0; i < soundIndexListCount; ++i)
		{
			playingSounds[i].pause();
		}
	}

	virtual void resumeAll()
	{
		for (uint32_t i = 0; i < soundIndexListCount; ++i)
		{
			playingSounds[i].resume();
		}
	}

	virtual void setSoundPositions(uint32_t soundInstanceListCount, const SoundInstanceId* soundInstanceIdList, const Vector3* positionList)
	{
		for (uint32_t i = 0; i < soundInstanceListCount; ++i)
		{
			lookup(soundInstanceIdList[i]).setPosition(positionList[i]);
		}
	}

	virtual void setSoundRanges(uint32_t soundInstanceListCount, const SoundInstanceId* soundInstanceIdList, const float* rangeList)
	{
		for (uint32_t i = 0; i < soundInstanceListCount; ++i)
		{
			lookup(soundInstanceIdList[i]).setRange(rangeList[i]);
		}
	}

	virtual void setSoundVolumes(uint32_t soundInstanceListCount, const SoundInstanceId* soundInstanceIdList, const float* volumeList)
	{
		for (uint32_t i = 0; i < soundInstanceListCount; i++)
		{
			lookup(soundInstanceIdList[i]).setVolume(volumeList[i]);
		}
	}

	virtual void reloadSounds(const SoundResource& oldSoundResource, const SoundResource& newSoundResource)
	{
		for (uint32_t i = 0; i < soundIndexListCount; ++i)
		{
			if (playingSounds[i].getSoundResource() == &oldSoundResource)
			{
				playingSounds[i].reload(newSoundResource);
			}
		}
	}

	virtual void setListenerPose(const Matrix4x4& pose)
	{
		const Vector3 position = getTranslation(pose);
		const Vector3 up = getAxisY(pose);
		const Vector3 at = getAxisZ(pose);

		AL_CHECK(alListener3f(AL_POSITION, position.x, position.y, position.z));
		//AL_CHECK(alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z));

		const ALfloat orientation[] = { up.x, up.y, up.z, at.x, at.y, at.z };
		AL_CHECK(alListenerfv(AL_ORIENTATION, orientation));
		listenerPose = pose;
	}

	virtual void update()
	{
		TempAllocator256 ta;
		Array<SoundInstanceId> soundInstanceIdListToDelete(ta);

		// Check what sounds finished playing
		for (uint32_t i = 0; i < soundIndexListCount; ++i)
		{
			SoundInstance& soundInstance = playingSounds[i];
			if (soundInstance.getIsFinished() == true)
			{
				ArrayFn::pushBack(soundInstanceIdListToDelete, soundInstance.id);
			}
		}

		// Destroy instances which finished playing
		for (uint32_t i = 0; i < ArrayFn::getCount(soundInstanceIdListToDelete); ++i)
		{
			stop(soundInstanceIdListToDelete[i]);
		}
	}

private:

	bool has(SoundInstanceId id)
	{
		SoundIndex& soundIndex = soundIndexList[id & INDEX_MASK];
		return soundIndex.id == id && soundIndex.index != UINT16_MAX;
	}

	SoundInstance& lookup(SoundInstanceId id)
	{
		return playingSounds[soundIndexList[id & INDEX_MASK].index];
	}

	SoundInstanceId add()
	{
		SoundIndex& soundIndex = soundIndexList[freelistDequeueIndex];
		freelistDequeueIndex = soundIndex.next;
		soundIndex.id += NEW_OBJECT_ID_ADD;
		soundIndex.index = soundIndexListCount++;
		SoundInstance& soundInstance = playingSounds[soundIndex.index];
		soundInstance.id = soundIndex.id;
		return soundInstance.id;
	}

	void remove(SoundInstanceId id)
	{
		SoundIndex& soundIndex = soundIndexList[id & INDEX_MASK];

		SoundInstance& soundInstance = playingSounds[soundIndex.index];
		soundInstance = playingSounds[--soundIndexListCount];
		soundIndexList[soundInstance.id & INDEX_MASK].index = soundIndex.index;

		soundIndex.index = UINT16_MAX;
		soundIndexList[freelistEnqueueIndex].next = id & INDEX_MASK;
		freelistEnqueueIndex = id & INDEX_MASK;
	}

	uint32_t soundIndexListCount = 0;
	SoundInstance playingSounds[MAX_OBJECTS];
	SoundIndex soundIndexList[MAX_OBJECTS];
	uint16_t freelistEnqueueIndex = MAX_OBJECTS - 1;
	uint16_t freelistDequeueIndex = 0;
	Matrix4x4 listenerPose;
};

namespace SoundWorldFn
{
	SoundWorld* create(Allocator& a)
	{
		return RIO_NEW(a, ALSoundWorld)();
	}

	void destroy(Allocator& a, SoundWorld* soundWorld)
	{
		RIO_DELETE(a, soundWorld);
	}
} // namespace SoundWorldFn

} // namespace Rio

#endif // RIO_SOUND_OPENAL
// Copyright (c) 2016 Volodymyr Syvochka