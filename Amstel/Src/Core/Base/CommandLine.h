// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Strings/StringUtils.h"

namespace Rio
{

struct CommandLine
{
	CommandLine(int argc, const char** argv)
		: argc(argc)
		, argv(argv)
	{
	}

	int findArgument(const char* longOpt, char shortOpt)
	{
		for (int i = 0; i < this->argc; ++i)
		{
			if (getIsLongOpt(this->argv[i], longOpt) || getIsShortOpt(this->argv[i], shortOpt))
			{
				return i;
			}
		}

		return this->argc;
	}

	bool getIsShortOpt(const char* arg, char shortOpt)
	{
		return shortOpt != '\0'
			&& getStringLength32(arg) > 1
			&& arg[0] == '-'
			&& arg[1] == shortOpt
			;
	}

	bool getIsLongOpt(const char* arg, const char* longOpt)
	{
		return longOpt != nullptr
			&& getStringLength32(arg) > 2
			&& arg[0] == '-'
			&& arg[1] == '-'
			&& strcmp(&arg[2], longOpt) == 0
			;
	}

	const char* getParameter(const char* longOpt, char shortOpt = '\0')
	{
		int argc = findArgument(longOpt, shortOpt);
		return argc < this->argc ? this->argv[argc + 1] : nullptr;
	}

	bool hasArgument(const char* longOpt, char shortOpt = '\0')
	{
		return findArgument(longOpt, shortOpt) < this->argc;
	}

	int argc;
	const char** argv;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka