// Copyright (c) 2016 Volodymyr Syvochka
#include "Core/Base/Os.h"
#include "Core/Strings/DynamicString.h"
#include "Core/Strings/StringStream.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Containers/Vector.h"

#include <string.h> // strcmp

#if RIO_PLATFORM_POSIX
	#include <dirent.h> // opendir, readdir
#endif

namespace Rio
{

namespace OsFn
{
	void getFileList(const char* path, Vector<DynamicString>& files)
	{
#if RIO_PLATFORM_POSIX
		DIR* directory;
		struct dirent *entry;

		if (!(directory = opendir(path)))
		{
			return;
		}

		while ((entry = readdir(directory)))
		{
			const char* dname = entry->d_name;

			if (!strcmp(dname, ".") || !strcmp(dname, ".."))
			{
				continue;
			}

			TempAllocator512 ta;
			DynamicString fileName(, ta);
			fileName.set(dname, getStringLength32(dname));
			VectorFn::pushBack(files, fileName);
		}

		closedir(directory);
#elif RIO_PLATFORM_WINDOWS
		TempAllocator1024 ta;
		DynamicString currentPath(ta);
		currentPath += path;
		currentPath += "\\*";

		WIN32_FIND_DATA ffd;
		HANDLE file = FindFirstFile(currentPath.getCStr(), &ffd);
		if (file == INVALID_HANDLE_VALUE)
		{
			return;
		}

		do
		{
			const char* fileName = ffd.cFileName;

			if (!strcmp(fileName, ".") || !strcmp(fileName, ".."))
			{
				continue;
			}

			TempAllocator512 ta;
			DynamicString fileNameStr(ta);
			fileNameStr.set(fileName, getStringLength32(fileName));
			VectorFn::pushBack(files, fileNameStr);
		}
		while (FindNextFile(file, &ffd) != 0);

		FindClose(file);
#endif
	}

	int executeProcess(const char* path, const char* args, StringStream& output)
	{
#if RIO_PLATFORM_POSIX
		TempAllocator512 ta;
		DynamicString cmd(ta);
		cmd += path;
		cmd += " 2>&1 ";
		cmd += args;
		FILE* file = popen(cmd.getCStr(), "r");

		char buffer[1024];
		while (fgets(buffer, sizeof(buffer), file) != NULL)
		{
			output << buffer;
		}

		return pclose(file);
#elif RIO_PLATFORM_WINDOWS
		STARTUPINFO info;
		memset(&info, 0, sizeof(info));
		info.cb = sizeof(info);

		PROCESS_INFORMATION process;
		memset(&process, 0, sizeof(process));

		int err = CreateProcess(path, (LPSTR)args, NULL, NULL, TRUE, 0, NULL, NULL, &info, &process);
		RIO_ASSERT(err != 0, "CreateProcess: GetLastError = %d", GetLastError());
		RIO_UNUSED(err);

		DWORD exitCode = 1;
		::WaitForSingleObject(process.hProcess, INFINITE);
  		GetExitCodeProcess(process.hProcess, &exitCode);
		CloseHandle(process.hProcess);
		CloseHandle(process.hThread);
		return (int)exitCode;
#endif // RIO_PLATFORM_
	}

} // namespace OsFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka