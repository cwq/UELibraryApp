// Copyright Epic Games, Inc. All Rights Reserved.

#include "Parse.h"

wchar_t* wcsistr(const wchar_t* Dest, const wchar_t* Src)
{
	for (;;)
	{
		const wchar_t* SrcIter  = Src;
		const wchar_t* DestIter = Dest;
		for (;;)
		{
			wchar_t SrcCh = towupper(*SrcIter++);
			if (!SrcCh)
			{
				return (wchar_t*)Dest;
			}
			wchar_t DestCh = towupper(*DestIter++);
			if (!DestCh)
			{
				return nullptr;
			}
			if (SrcCh != DestCh)
			{
				++Dest;
				break;
			}
		}
	}
}

bool Parse(const wchar_t*& Command, unsigned int& Val)
{
	bool bResult = false;
	Val = 0;
	for (;;)
	{
		wchar_t Ch = *Command;
		if (Ch < '0' || Ch > '9')
		{
			return bResult;
		}
		bResult = true;
		unsigned int OldVal = Val;
		Val = (Val * 10) + (Ch - '0');
		if (Val < OldVal)
		{
			// Overflow
			return false;
		}
		++Command;
	}
}

bool Parse(const wchar_t*& Command, std::wstring& Val)
{
	Val.clear();
	for (;;)
	{
		wchar_t Ch = *Command;
		if (!Ch || iswspace(Ch))
		{
			return true;
		}
		Val.push_back(Ch);
		++Command;
	}
}

bool ParseSeparator(const wchar_t*& Command)
{
	bool bFoundSeparator = false;
	for (;;)
	{
		wchar_t Ch = *Command;
		if (!Ch)
		{
			return true;
		}
		if (!iswspace(Ch))
		{
			return bFoundSeparator;
		}
		bFoundSeparator = true;
		++Command;
	}
}
