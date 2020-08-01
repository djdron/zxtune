/**
*
* @file
*
* @brief  std::locale-independent symbol categories check functions
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

#include <cctype>

bool IsAlpha(Char c)
{
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool IsDigit(Char c)
{
	return c >= '0' && c <= '9';
}

bool IsAlNum(Char c)
{
  return IsDigit(c) || IsAlpha(c);
}
