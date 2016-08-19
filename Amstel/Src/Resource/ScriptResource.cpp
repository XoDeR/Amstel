// Copyright (c) 2016 Volodymyr Syvochka
#include "Resource/ScriptResource.h"
#include "Config.h"
#include "Core/Strings/DynamicString.h"
#include "Core/Strings/StringStream.h"
#include "Core/Base/Os.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Containers/Array.h"
#include "Resource/CompileOptions.h"

#define LUAJIT_NAME "./luajit"

#if RIO_PLATFORM_WINDOWS
	#define EXE ".exe"
#else
 	#define EXE ""
#endif // RIO_PLATFORM_WINDOWS

#define LUAJIT_EXE LUAJIT_NAME EXE

#if RIO_DEBUG
	#define LUAJIT_FLAGS "-bg" // Keep debug info
#else
	#define LUAJIT_FLAGS "-b"
#endif // RIO_DEBUG

namespace Rio
{

namespace ScriptResourceFn
{
	void compile(const char* path, CompileOptions& compileOptions)
	{
		TempAllocator1024 ta;
		DynamicString scriptSource(ta);
		DynamicString scriptBinary(ta);
		compileOptions.getAbsolutePath(path, scriptSource);
		compileOptions.getTemporaryPath("lua.bin", scriptBinary);

		StringStream arguments(ta);
		arguments << " " << LUAJIT_FLAGS;
		arguments << " " << scriptSource.getCStr();
		arguments << " " << scriptBinary.getCStr();

		StringStream output(ta);
		int exitCode = OsFn::executeProcess(LUAJIT_EXE, StringStreamFn::getCStr(arguments), output);
		RESOURCE_COMPILER_ASSERT(exitCode == 0
			, compileOptions
			, "Failed to compile lua script:\n%s"
			, StringStreamFn::getCStr(output)
			);

		Buffer blob = compileOptions.readTemporary(scriptBinary.getCStr());
		compileOptions.deleteFile(scriptBinary.getCStr());

		ScriptResource scriptResource;
		scriptResource.version = RESOURCE_VERSION_SCRIPT;
		scriptResource.size = ArrayFn::getCount(blob);

		compileOptions.write(scriptResource.version);
		compileOptions.write(scriptResource.size);
		compileOptions.write(blob);
	}

	void* load(File& file, Allocator& a)
	{
		const uint32_t fileSize = file.getSize();
		void* resource = a.allocate(fileSize);
		file.read(resource, fileSize);
		RIO_ASSERT(*(uint32_t*)resource == RESOURCE_VERSION_SCRIPT, "Wrong version");
		return resource;
	}

	void unload(Allocator& allocator, void* resource)
	{
		allocator.deallocate(resource);
	}

	const char* getProgram(const ScriptResource* scriptResource)
	{
		return (char*)&scriptResource[1];
	}
} // namespace ScriptResourceFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka