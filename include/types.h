/**
*
* @file
*
* @brief  Basic types definitions
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//common includes
#include <char_type.h>
//std includes
#include <string>
#include <vector>
//boost includes
#include <boost/cstdint.hpp>
#include <boost/static_assert.hpp>

//@{
//! @brief Integer types
#ifndef BOOST_HAS_STDINT_H
using boost::int8_t;
using boost::uint8_t;
using boost::int16_t;
using boost::uint16_t;
using boost::int32_t;
using boost::uint32_t;
using boost::int64_t;
using boost::uint64_t;
#endif

#define __STDC_CONSTANT_MACROS

/// Unsigned integer type
typedef boost::uint_fast32_t uint_t;
/// Signed integer type
typedef boost::int_fast32_t int_t;
//@}

/// String-related types

//! @brief %String type
typedef std::basic_string<Char> String;

//! @brief Helper for creating String from the array of chars
template<std::size_t D>
inline String FromCharArray(const char (&str)[D])
{
  //do not keep last zero symbol
  return String(str, str + D);
}

//! @brief Helper for creating String from ordinary std::string
inline String FromStdString(const std::string& str)
{
  return String(str.begin(), str.end());
}

//! @brief Helper for creating ordinary std::string from the String
inline std::string ToStdString(const String& str)
{
  return std::string(str.begin(), str.end());
}

//@{
//! Structure packing macros
//! @code
//! #ifdef USE_PRAGMA_PACK
//! #pragma pack(push,1)
//! #endif
//! PACK_PRE struct Foo
//! {
//! ...
//! } PACK_POST;
//!
//! PACK_PRE struct Bar
//! {
//! ...
//! } PACK_POST;
//! #ifdef USE_PRAGMA_PACK
//! #pragma pack(pop)
//! #endif
//! @endcode
#if defined(_MSC_VER) || defined(__MINGW32__)
#define PACK_PRE
#define PACK_POST
#define USE_PRAGMA_PACK
#elif defined __GNUC__
#define PACK_PRE
#define PACK_POST __attribute__ ((packed))
#else
#define PACK_PRE
#define PACK_POST
#endif
//@}

//! @brief Plain data type
typedef std::vector<uint8_t> Dump;

//assertions
BOOST_STATIC_ASSERT(sizeof(uint_t) >= sizeof(uint32_t));
BOOST_STATIC_ASSERT(sizeof(int_t) >= sizeof(int32_t));
BOOST_STATIC_ASSERT(sizeof(uint8_t) == 1);
BOOST_STATIC_ASSERT(sizeof(int8_t) == 1);
BOOST_STATIC_ASSERT(sizeof(uint16_t) == 2);
BOOST_STATIC_ASSERT(sizeof(int16_t) == 2);
BOOST_STATIC_ASSERT(sizeof(uint32_t) == 4);
BOOST_STATIC_ASSERT(sizeof(int32_t) == 4);
BOOST_STATIC_ASSERT(sizeof(uint64_t) == 8);
BOOST_STATIC_ASSERT(sizeof(int64_t) == 8);
