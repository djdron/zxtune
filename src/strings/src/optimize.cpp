/**
*
* @file
*
* @brief  String optimization implementation
*
* @author vitamin.caig@gmail.com
*
**/

//library includes
#include <strings/optimize.h>
//std includes
#include <algorithm>
#include <cctype>
//boost includes
#include <boost/algorithm/string/trim.hpp>

namespace Strings
{
  String Optimize(const String& str)
  {
    String res(boost::algorithm::trim_copy_if(str, !boost::is_from_range('\x21', '\x7f')));
	struct IsCntrlSafe
	{
		bool operator()(char ch) const { return ch >= 0 && std::iscntrl(ch); }
	};
    std::replace_if(res.begin(), res.end(), IsCntrlSafe(), '\?');
    return res;
  }
}
