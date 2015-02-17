#include "string_helpers.h"

boost::string_ref vv::ltrim(boost::string_ref str)
{
  auto last = std::find_if_not(begin(str), end(str), isspace);
  str.remove_prefix(static_cast<size_t>(last - begin(str)));
  return str;
}

int vv::to_int(const std::string& str)
{
  if (str.front() == '0' && str.size() > 1) {
    switch (str[1]) {
    case 'x': return stoi(str.substr(2), nullptr, 16);
    case 'b': return stoi(str.substr(2), nullptr, 2);
    default:  return stoi(str.substr(1), nullptr, 8);
    }
  }
  if (isdigit(str.front()))
     return stoi(str);
  return 0;
}
