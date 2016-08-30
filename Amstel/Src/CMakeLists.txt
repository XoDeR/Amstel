fips_include_directories(.)

fips_begin_app(Amstel cmdline)
	fips_libs(luajit)
	
	fips_dir(Core/Base GROUP "Core/Base")
	fips_files(
		CommandLine.h
		Functional.h
		Guid.cpp
		Guid.h
		Integer.h
		Macros.h
		Murmur.cpp
		Murmur.h
		Os.cpp
		Os.h
		Platform.h
		Types.h
	)
	fips_dir(Core/Compat/Msvc GROUP "Core/Compat")
	fips_files(
		AllocA.h
		IntTypes.h
	)
	fips_dir(Core/Containers GROUP "Core/Containers")
	fips_files(
		Array.h
		ContainerTypes.h
		EventStream.h
		HashMap.h
		Map.h
		Pair.h
		PriorityQueue.h
		Queue.h
		SortMap.h
		Vector.h
	)
	fips_dir(Core/Error GROUP "Core/Error")
	fips_files(
		Error.cpp
		Error.h
	)
#if (FIPS_ANDROID)
    fips_dir(Core/Error/Android GROUP "Core/Error/Android")
	fips_files(
		Callstack_Android.cpp
	)
#elseif (FIPS_LINUX)
    fips_dir(Core/Error/Linux GROUP "Core/Error/Linux")
	fips_files(
		Callstack_Linux.cpp
	)
#elseif (FIPS_WINDOWS)
    fips_dir(Core/Error/Windows GROUP "Core/Error/Windows")
	fips_files(
		Callstack_Windows.cpp
	)
#endif()
	fips_dir(Core/FileSystem GROUP "Core/FileSystem")
	fips_files(
		FileSystemDisk.cpp
		FileSystemDisk.h
		File.h
		FileSystem.h
		FileSystemTypes.h
		NullFile.h
		Path.cpp
		Path.h
		ReaderWriter.h
	)
#if (FIPS_ANDROID)
    fips_dir(Core/FileSystem/Android GROUP "Core/FileSystem/Android")
	fips_files(
		FileSystemApk_Android.cpp
		FileSystemApk_Android.h
	)
#endif()	
	fips_dir(Core/Json GROUP "Core/Json")
	fips_files(
		Json.cpp
		Json.h
		JsonR.cpp
		JsonR.h
		JsonObject.h
		JsonTypes.h
	)
	fips_dir(Core/Math GROUP "Core/Math")
	fips_files(
		Aabb.cpp
		Aabb.h
		Color4.h
		Frustum.h
		Intersection.cpp
		Intersection.h
		MathTypes.h
		MathUtils.h
		Matrix3x3.h
		Matrix4x4.h
		Plane3.h
		Quaternion.cpp
		Quaternion.h
		Random.h
		Sphere.cpp
		Sphere.h
		Vector2.h
		Vector3.h
		Vector4.h
	)
	fips_dir(Core/Memory GROUP "Core/Memory")
	fips_files(
		Allocator.h
		LinearAllocator.cpp
		LinearAllocator.h
		Memory.cpp
		Memory.h
		MemoryTypes.h
		PoolAllocator.cpp
		PoolAllocator.h
		ProxyAllocator.cpp
		ProxyAllocator.h
		StackAllocator.cpp
		StackAllocator.h
		TempAllocator.h
	)
	fips_dir(Core/Network GROUP "Core/Network")
	fips_files(
		IpAddress.h
		NetworkTypes.h
		Socket.h
	)
	fips_dir(Core/Strings GROUP "Core/Strings")
	fips_files(
		DynamicString.h
		FixedString.h
		StringId.cpp
		StringId.h
		StringStream.h
		StringTypes.h
		StringUtils.h
		Utf8.cpp
		Utf8.h
	)
	fips_dir(Core/Thread GROUP "Core/Thread")
	fips_files(
		AtomicInt.h
		Mutex.h
		Semaphore.h
		Thread.h
	)
	fips_dir(Core GROUP "Core")
	fips_files(
		UnitTests.cpp
	)
	fips_dir(Device)
	fips_files(
		BootConfig.cpp
		BootConfig.h
		ConsoleApi.cpp
		ConsoleApi.h
		ConsoleServer.cpp
		ConsoleServer.h
		Device.cpp
		Device.h
		DeviceOptions.cpp
		DeviceOptions.h
		Display.h
		InputDevice.cpp
		InputDevice.h
		InputManager.cpp
		InputManager.h
		InputTypes.h
		Log.cpp
		Log.h
		DeviceEventQueue.h
		Profiler.cpp
		Profiler.h
		Window.h
	)
#if (FIPS_ANDROID)
    fips_dir(Device/Android GROUP "Device/Android")
	fips_files(
		AmstelActivity.java
		AndroidManifest.xml
		Main_Android.cpp
	)
#elseif (FIPS_LINUX)
    fips_dir(Device/Linux GROUP "Device/Linux")
	fips_files(
		Main_Linux.cpp
	)
#elseif (FIPS_WINDOWS)
    fips_dir(Device/Windows GROUP "Device/Windows")
	fips_files(
		Main_Windows.cpp
		Headers_Windows.h
	)
#endif()
	fips_dir(Resource)
	fips_files(
		DataCompiler.cpp
		DataCompiler.h
		CompileOptions.h
		CompilerTypes.h
		ConfigResource.cpp
		ConfigResource.h
		FontResource.cpp
		FontResource.h
		LevelResource.cpp
		LevelResource.h
		MaterialResource.cpp
		MaterialResource.h
		MeshResource.cpp
		MeshResource.h
		PackageResource.cpp
		PackageResource.h
		PhysicsResource.cpp
		PhysicsResource.h
		ResourceLoader.cpp
		ResourceLoader.h
		ResourceManager.cpp
		ResourceManager.h
		ResourcePackage.cpp
		ResourcePackage.h
		ResourceTypes.h
		ScriptResource.cpp
		ScriptResource.h
		ShaderResource.cpp
		ShaderResource.h
		SoundResource.cpp
		SoundResource.h
		SpriteResource.cpp
		SpriteResource.h
		TextureResource.cpp
		TextureResource.h
		UnitCompiler.cpp
		UnitCompiler.h
		UnitResource.cpp
		UnitResource.h
	)
	fips_dir(Script)
	fips_files(
		ScriptApi.cpp
		ScriptEnvironment.cpp
		ScriptEnvironment.h
		ScriptStack.cpp
		ScriptStack.h
		ScriptTypes.h
	)
	fips_dir(World)
	fips_files(
		Audio.h
		DebugLine.cpp
		DebugLine.h
		DebugGui.cpp
		DebugGui.h
		Level.cpp
		Level.h
		Material.cpp
		Material.h
		MaterialManager.cpp
		MaterialManager.h
		Physics.h
		PhysicsWorld.h
		PhysicsWorldBullet.cpp
		PhysicsWorldNull.cpp
		RenderWorld.cpp
		RenderWorld.h
		SceneGraph.cpp
		SceneGraph.h
		ShaderManager.cpp
		ShaderManager.h
		SoundWorld.h
		SoundWorldAl.cpp
		SoundWorldNull.cpp
		UnitManager.cpp
		UnitManager.h
		World.cpp
		World.h
		WorldTypes.h
	)
	fips_dir(.)
	fips_files(
		Config.h
	)
		
    fips_deps(bgfx)
	fips_deps(Bullet)
fips_end_app()

add_dependencies(Amstel luajit_copy_dlls)
