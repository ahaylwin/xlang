﻿/*
Copyright (C) 2024 The XLang Foundation
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "number.h"
namespace X
{
	ParseState ParseHexBinOctNumber(String& str)
	{
		return ParseState::Null;
	}
	ParseState ParseNumber(String& str,
		double& dVal, long long& llVal)
	{
		ParseState st = ParseState::Null;
		if (str.s == nil || str.size == 0)
		{
			return st;
		}
		if (str.size >= 2 && *str.s == '0')
		{
			char c = *(str.s + 1);
			//for hex(0x),oct(0o),bin(0b), also require first letter is 0
			//so this is true if it is number
			if (c == 'x' || c == 'o' || c == 'b')
			{
				String str2 = { str.s + 2,str.size - 2 };
				return ParseHexBinOctNumber(str2);
			}
		}
		long long primary[2] = { 0,0 };
		int digit_cnt[2] = { 0,0 };
		char* end = str.s + str.size;
		int it = 0;
		bool meetDot = false;
		bool meetJ = false;
		bool correctSyntax = true;
		char* p = str.s;
		while (p < end)
		{
			char c = *p++;
			if (c >= '0' && c <= '9')
			{
				primary[it] = primary[it] * 10 + c - '0';
				digit_cnt[it]++;
			}
			else if (c == '.')
			{
				if (meetDot)
				{//more than one
					//error
					correctSyntax = false;
					break;
				}
				meetDot = true;
				it++;
			}
			else if (c == 'j' && (p==end))//last one is j
			{
				meetJ = true;
			}
			else
			{
				//error
				correctSyntax = false;
				break;
			}
		}
		if (correctSyntax)
		{
			if (meetDot)
			{
				dVal = (double)primary[1];
				//todo: faster way?
				for (int i = 0; i < digit_cnt[1]; i++)
				{
					dVal /= 10;
				}
				dVal += primary[0];
				st = meetJ? ParseState::Complex:ParseState::Double;
			}
			else
			{
				llVal = primary[0];
				dVal = digit_cnt[0];//reuse for count of digits
				if (meetJ)
				{
					st = ParseState::Complex;
					dVal = llVal;
				}
				else
				{
					st = ParseState::Long_Long;
				}
			}
		}
		return st;
	}
}