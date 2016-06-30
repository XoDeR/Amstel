// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Containers/ContainerTypes.h"
#include "World/WorldTypes.h"

namespace Rio
{

class UnitManager
{
private:
	using DestroyFunction = void (*)(UnitId unit, void* userPtr);

	struct DestroyData
	{
		DestroyFunction destroyFunction;
		void* userPtr;
	};
public:
	UnitManager(Allocator& a);
	UnitId makeUnit(uint32_t idx, uint8_t gen);
	// Creates a new empty unit
	UnitId create();
	// Creates a new empty unit in the given <world>
	UnitId create(World& world);
	// Returns whether the unit <id> is alive
	bool getIsAlive(UnitId id) const;
	void destroy(UnitId id);
	void registerDestroyFunction(DestroyFunction destroyFunction, void* userPtr);
	void unregisterDestroyFunction(void* userPtr);
	void triggerDestroyCallbacks(UnitId id);
private:
	Array<uint8_t> generation;
	Queue<uint32_t> freeIndicesQueue;
	Array<DestroyData> destroyCallbackList;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka