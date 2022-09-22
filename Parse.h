// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include <string>
#include <wchar.h>
#include <wctype.h>

// Returns true if the value was parsed and advances the Command ptr after the 
// end of the parsed section.  Val and Command are unspecified if false is
// returned.
bool Parse(const wchar_t*& Command, unsigned int& Val);
bool Parse(const wchar_t*& Command, std::wstring& Val);

// Returns true if a separator was parsed (including a null terminator),
// advancing the pointer past any separator (but not the null terminator).
bool ParseSeparator(const wchar_t*& Command);

// Case-insensitive version of wcsstr
wchar_t* wcsistr(const wchar_t* Dest, const wchar_t* Src);

// Parses a value from the command line and removes it.
template <typename T>
bool ExtractValueFromCommandLine(wchar_t* CmdLine, const wchar_t* Command, T& OutValue)
{
	// Ensure the command string was found
	wchar_t* Found = wcsistr(CmdLine, Command);
	if (!Found)
	{
		return false;
	}

	// Ensure the found substring is the start of the cmdline or whitespace
	if (Found != CmdLine && !iswspace(Found[-1]))
	{
		return false;
	}

	// Ensure the found substring reaches the separator or whitespace/end of the string if there is no separator.
	const wchar_t* FoundEnd = Found + wcslen(Command);
	if (*FoundEnd != '=')
	{
		return false;
	}

	// Advance past separator
	++FoundEnd;

	if (!Parse(FoundEnd, OutValue))
	{
		return false;
	}

	if (!ParseSeparator(FoundEnd))
	{
		return false;
	}

	// Remove the parsed command so that we don't pass it on to the engine
	while (Found != FoundEnd)
	{
		*Found++ = ' ';
	}

	return true;
}
