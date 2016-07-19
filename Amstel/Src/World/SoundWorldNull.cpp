// Copyright (c) 2016 Volodymyr Syvochka
#include "Config.h"

#if RIO_SOUND_NULL

#include "Core/Memory/Memory.h"
#include "World/SoundWorld.h"
#include "World/Audio.h"

namespace Rio
{

namespace AudioGlobalFn
{
	void init()
	{
	}

	void shutdown()
	{
	}
}

class NullSoundWorld : public SoundWorld
{
public:

	NullSoundWorld()
	{
	}

	virtual ~NullSoundWorld()
	{
	}

	virtual SoundInstanceId play(const SoundResource& /*soundResource*/, bool /*loop*/, float /*volume*/, float /*range*/, const Vector3& /*position*/)
	{
		return 0;
	}

	virtual void stop(SoundInstanceId /*id*/)
	{
	}

	virtual bool getIsPlaying(SoundInstanceId /*id*/)
	{
		return false;
	}

	virtual void stopAll()
	{
	}

	virtual void pauseAll()
	{
	}

	virtual void resumeAll()
	{
	}

	virtual void setSoundPositions(uint32_t /*soundInstanceListCount*/, const SoundInstanceId* /*soundInstanceIdList*/, const Vector3* /*positionList*/)
	{
	}

	virtual void setSoundRanges(uint32_t /*soundInstanceListCount*/, const SoundInstanceId* /*soundInstanceIdList*/, const float* /*rangeList*/)
	{
	}

	virtual void setSoundVolumes(uint32_t /*soundInstanceListCount*/, const SoundInstanceId* /*soundInstanceIdList*/, const float* /*volumeList*/)
	{
	}

	virtual void reloadSounds(const SoundResource& /*oldSoundResource*/, const SoundResource& /*newSoundResource*/)
	{
	}

	virtual void setListenerPose(const Matrix4x4& /*pose*/)
	{
	}

	virtual void update()
	{
	}
};

SoundWorld* SoundWorldFn::create(Allocator& a)
{
	return RIO_NEW(a, NullSoundWorld)();
}

void SoundWorldFn::destroy(Allocator& a, SoundWorld* soundWorld)
{
	RIO_DELETE(a, soundWorld);
}

} // namespace Rio

#endif // RIO_SOUND_NULL
// Copyright (c) 2016 Volodymyr Syvochka