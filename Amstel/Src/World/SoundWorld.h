// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Math/MathTypes.h"
#include "Core/Memory/MemoryTypes.h"
#include "Core/Base/Types.h"
#include "Resource/ResourceTypes.h"
#include "World/WorldTypes.h"

namespace Rio
{

class SoundWorld
{
public:
	virtual ~SoundWorld() {};
	// Plays the sound <soundResource> at the given <volume> [0 .. 1]
	// If loop is true the sound will be played looping
	virtual SoundInstanceId play(const SoundResource& soundResource, bool loop, float volume, float range, const Vector3& position) = 0;
	// Stops the sound with the given <id>
	// After this call, the instance will be destroyed
	virtual void stop(SoundInstanceId id) = 0;
	// Returns whether the sound <id> is playing
	virtual bool getIsPlaying(SoundInstanceId id) = 0;
	// Stops all the sounds in the world
	virtual void stopAll() = 0;
	// Pauses all the sounds in the world
	virtual void pauseAll() = 0;
	// Resumes all previously paused sounds in the world
	virtual void resumeAll() = 0;
	// Sets the <positionList> (in world space) of <soundInstanceListCount> sound instances <soundInstanceIdList>
	virtual void setSoundPositions(uint32_t soundInstanceListCount, const SoundInstanceId* soundInstanceIdList, const Vector3* positionList) = 0;
	// Sets the <rangeList> (in meters) of <soundInstanceListCount> sound instances <soundInstanceIdList>
	virtual void setSoundRanges(uint32_t soundInstanceListCount, const SoundInstanceId* soundInstanceIdList, const float* rangeList) = 0;
	// Sets the <volumeList> of <soundInstanceListCount> sound instances <soundInstanceIdList>
	virtual void setSoundVolumes(uint32_t soundInstanceListCount, const SoundInstanceId* soundInstanceIdList, const float* volumeList) = 0;
	virtual void reloadSounds(const SoundResource& oldSoundResource, const SoundResource& newSoundResource) = 0;
	// Sets the <pose> of the listener in world space
	virtual void setListenerPose(const Matrix4x4& pose) = 0;
	virtual void update() = 0;
};

namespace SoundWorldFn
{
	SoundWorld* create(Allocator& a);
	void destroy(Allocator& a, SoundWorld* soundWorld);
} // namespace SoundWorldFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka