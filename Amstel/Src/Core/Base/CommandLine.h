// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Strings/StringUtils.h"

namespace Rio
{

struct CommandLine
{
	CommandLine(int argumentListCount, const char** argumentList)
		: argumentListCount(argumentListCount)
		, argumentList(argumentList)
	{
	}

	int findArgument(const char* longOpt, char shortOpt)
	{
		for (int i = 0; i < this->argumentListCount; ++i)
		{
			if (getIsLongOpt(this->argumentList[i], longOpt) || getIsShortOpt(this->argumentList[i], shortOpt))
			{
				return i;
			}
		}

		return this->argumentListCount;
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

	const char* getParameter(int i, const char* longOpt, char shortOpt = '\0')
	{
		int argumentListCount = findArgument(longOpt, shortOpt);
		return argumentListCount + i < this->argumentListCount ? this->argumentList[argumentListCount + i + 1] : nullptr;
	}

	bool hasArgument(const char* longOpt, char shortOpt = '\0')
	{
		return findArgument(longOpt, shortOpt) < this->argumentListCount;
	}

	int argumentListCount;
	const char** argumentList;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka