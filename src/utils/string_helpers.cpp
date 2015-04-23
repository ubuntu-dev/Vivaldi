#include "string_helpers.h"

#include <regex>

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

std::string vv::escape_chars(const std::string& orig)
{
  std::string tmp;
  for (const auto c : orig) {
    switch (c) {
    case '\a': tmp += "\\a"; break;
    case '\b': tmp += "\\b"; break;
    case '\n': tmp += "\\n"; break;
    case '\f': tmp += "\\f"; break;
    case '\r': tmp += "\\r"; break;
    case '\t': tmp += "\\t"; break;
    case '\v': tmp += "\\v"; break;
    case '"':  tmp += "\\\""; break;
    case '\\': tmp += "\\\\"; break;
    default:
      if (isgraph(c) || c == ' ') {
        tmp += c;
      }
      else {
        tmp += '\\';
        tmp += ('0' + (int{c} / 64));
        tmp += ('0' + (int{c} % 64 / 8));
        tmp += ('0' + (int{c} % 8));
      }
    }
  }
  return tmp;
}

std::string vv::get_escaped_name(const char orig)
{
  using namespace std::string_literals;

  switch (orig) {
  case '\a': return "\\alarm";
  case '\b': return "\\backspace";
  case '\n': return "\\newline";
  case '\f': return "\\page";
  case '\r': return "\\return";
  case '\t': return "\\tab";
  case '\v': return "\\vtab";
  case '\0': return "\\nul";
  case ' ':  return "\\space";
  default:
    if (isgraph(orig)) {
      return "\\"s + orig;
    }
    else {
      std::string tmp{"\\"};
      tmp += ('0' + (int{orig} / 64));
      tmp += ('0' + (int{orig} % 64 / 8));
      tmp += ('0' + (int{orig} % 8));
      return tmp;
    }
  }
}
