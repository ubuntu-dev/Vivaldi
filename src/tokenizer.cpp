#include "parser.h"

#include "utils/string_helpers.h"

using namespace vv;
using namespace parser;

// Available tokens:
// '{', '}'
// '[', ']'
// '(', ')'
// '.'
// ','
// ':'
// '='
// '=='
// '+', '-'
// '*', '/', '%'
// '!', '~'
// '^', '&', '|'
// '&&', '||'
// '<', '>', '<=', '>='
// '''
// (strings)
// (names)
// (numbers)
// (hexadecimal numbers)
// (binary numbers)
// (octal numbers)
// (newline)

namespace {

using tok_res = std::pair<parser::token, boost::string_ref>;

// Individual tokenizing functions {{{

// '0' {{{

tok_res zero_token(boost::string_ref line)
{
  auto last = begin(line) + 1;
  if (line.size() > 1) {
    if (line[1] == '.') {
      auto post_dot = std::find_if_not(begin(line) + 2, end(line), isdigit);
      if (post_dot != begin(line) + 2) {

        std::string num{begin(line), post_dot};
        return { {token::type::floating_point, num},
                 line.substr(post_dot - begin(line)) };
      }

    } else if (line[1] == 'x') {
      last = std::find_if(begin(line) + 2, end(line),
                         [](auto c) { return !isdigit(c) || c < 'a' || c > 'f'; });

    } else if (line[1] == 'b') {
      last = std::find_if(begin(line) + 2, end(line),
                          [](auto c) { return c != '0' && c != '1'; });

    } else if (isdigit(line[1])) {
      last = std::find_if(begin(line) + 2, end(line),
                          [](auto c)
                            { return !isdigit(c) || c == '8' || c == '9'; });
    }
  }
  std::string num{begin(line), last};
  return { {token::type::integer, num}, line.substr(last - begin(line)) };
}

// }}}
// '1'-'9' {{{

tok_res digit_token(boost::string_ref line)
{
  auto nondigit = std::find_if_not(begin(line), end(line), isdigit);
  if (nondigit != end(line) && *nondigit == '.') {
    auto nonfloat = std::find_if_not(nondigit + 1, end(line), isdigit);
    if (nonfloat != nondigit + 1)
      return { {token::type::floating_point, {begin(line), nonfloat}},
               line.substr(nonfloat - begin(line))};
  }

  return { {token::type::integer, {begin(line), nondigit}},
           line.substr(nondigit - begin(line))};
}

// }}}
// '\'' {{{

tok_res sym_token(boost::string_ref line)
{
  auto name = line.substr(1);
  auto last = std::find_if_not(begin(name), end(name), isnamechar);
  if (last == begin(name) || isdigit(name.front()))
    return { {token::type::invalid, {begin(line), last}},
             line.substr(last - begin(line)) };

  std::string sym{begin(name), last};
  return { {token::type::symbol, sym}, line.substr(last - begin(line))};
}

// }}}
// '=' {{{

tok_res eq_tokens(boost::string_ref line)
{
  if (line.size() == 1 || line[1] != '=')
    return { {token::type::assignment, "="}, line.substr(1)};
  return { {token::type::equals, "=="}, line.substr(2)};
}

// }}}
// '!' {{{

tok_res bang_tokens(boost::string_ref line)
{
  if (line.size() == 1 || line[1] != '=')
    return { {token::type::bang, "!"}, line.substr(1)};
  return { {token::type::unequal, "!="}, line.substr(2) };
}

// }}}
// '*' {{{

tok_res star_tokens(boost::string_ref line)
{
  if (line.size() == 1 || line[1] != '*')
    return { {token::type::star, "*"}, line.substr(1)};
  return { {token::type::double_star, "**"}, line.substr(2)};
}

// }}}
// '&' {{{

tok_res and_tokens(boost::string_ref line)
{
  if (line.size() == 1 || line[1] != '&')
    return { {token::type::ampersand, "&"}, line.substr(1)};
  return { {token::type::and_sign, "&&"}, line.substr(2)};
}

// }}}
// '|' {{{

tok_res or_tokens(boost::string_ref line)
{
  if (line.size() == 1 || line[1] != '|')
    return { {token::type::pipe, "|"}, line.substr(1)};
  return { {token::type::or_sign, "||"}, line.substr(2)};
}

// }}}
// '<' {{{

tok_res lt_tokens(boost::string_ref line)
{
  if (line.size() > 1 && line[1] == '=')
    return { {token::type::less_eq, "<="}, line.substr(2)};
  if (line.size() > 1 && line[1] == '<')
    return { {token::type::lshift, "<<"}, line.substr(2)};
  return { {token::type::less, "<"}, line.substr(1)};
}

// }}}
// '>' {{{

tok_res gt_tokens(boost::string_ref line)
{
  if (line.size() > 1 && line[1] == '=')
    return { {token::type::greater_eq, ">="}, line.substr(2)};
  if (line.size() > 1 && line[1] == '>')
    return { {token::type::rshift, ">>"}, line.substr(2)};
  return { {token::type::greater, ">"}, line.substr(1)};
}

// }}}
// '"' {{{
std::pair<char, boost::string_ref> escaped_oct(boost::string_ref line)
{
  auto last = std::find_if_not(begin(line),
                               begin(line) + std::min(line.size(), size_t{3}),
                               [](auto c) { return '0' <= c && c < '8'; });

  auto val = stoi(std::string{begin(line), last}, nullptr, 8);
  return { static_cast<char>(val), line.substr(last - begin(line)) };
}

std::pair<char, boost::string_ref> escaped(boost::string_ref line)
{
  auto nonescaped = line.front();
  if (isdigit(nonescaped))
    return escaped_oct(line);
  line = line.substr(1);
  switch (nonescaped) {
  case 'a':  return { '\a', line };
  case 'b':  return { '\b', line };
  case 'n':  return { '\n', line };
  case 'f':  return { '\f', line };
  case 'r':  return { '\r', line };
  case 't':  return { '\t', line };
  case 'v':  return { '\v', line };
  case '"':  return { '"',  line };
  case '\\': return { '\\', line };
  default:   return { nonescaped, line };
  }
}

tok_res string_token(boost::string_ref line)
{
  std::string token{'"'};
  line.remove_prefix(1);
  while (line.front() != '"') {
    if (line.front() == '\\') {
      line.remove_prefix(1);
      char chr;
      tie(chr, line) = escaped(line);
      token += chr;
    } else {
      token += line.front();
      line.remove_prefix(1);
    }
    if (!line.size())
      return { {token::type::invalid, token}, line };
  }
  return { {token::type::string, token += '"'}, line.substr(1)};
}

// }}}
// '`' {{{

tok_res regex_token(boost::string_ref line)
{
  std::string token{};
  line.remove_prefix(1);
  while (line.front() != '`') {
    if (line.front() == '\\') {
      line.remove_prefix(1);
      char chr;
      tie(chr, line) = escaped(line);
      token += chr;
    } else {
      token += line.front();
      line.remove_prefix(1);
    }
    if (!line.size())
      return { {token::type::invalid, token}, line };
  }
  return { {token::type::regex, token}, line.substr(1)};
}

// }}}
// /./ {{{

token::type type_for(const std::string& name)
{
  if (name == "to") return token::type::to;

  if (name == "let") return token::type::key_let;
  if (name == "new") return token::type::key_new;

  if (name == "class") return token::type::key_class;
  if (name == "fn")    return token::type::key_fn;

  if (name == "do")     return token::type::key_do;
  if (name == "end")    return token::type::key_end;
  if (name == "cond")   return token::type::key_cond;
  if (name == "if")     return token::type::key_cond;
  if (name == "while")  return token::type::key_while;
  if (name == "for")    return token::type::key_for;
  if (name == "in")     return token::type::key_in;
  if (name == "return") return token::type::key_return;

  if (name == "require") return token::type::key_require;

  if (name == "try")    return token::type::key_try;
  if (name == "catch")  return token::type::key_catch;
  if (name == "except") return token::type::key_except;

  if (name == "true")  return token::type::boolean;
  if (name == "false") return token::type::boolean;
  if (name == "nil")   return token::type::nil;
  return token::type::name;
}

tok_res name_token(boost::string_ref line)
{
  auto last = std::find_if_not(begin(line), end(line), isnamechar);
  if (last == begin(line))
    return { {token::type::invalid, {line.front()}}, line.substr(1) };
  std::string name{begin(line), last};
  return { {type_for(name), name}, line.substr(last - begin(line))};
}

// }}}

tok_res first_token(boost::string_ref line)
{
  tok_res token;
  switch (line.front()) {
  case '{': return {{token::type::open_brace,    "{"}, line.substr(1)};
  case '}': return {{token::type::close_brace,   "}"}, line.substr(1)};
  case '[': return {{token::type::open_bracket,  "["}, line.substr(1)};
  case ']': return {{token::type::close_bracket, "]"}, line.substr(1)};
  case '(': return {{token::type::open_paren,    "("}, line.substr(1)};
  case ')': return {{token::type::close_paren,   ")"}, line.substr(1)};

  case ',': return {{token::type::comma,     ","}, line.substr(1)};
  case ':': return {{token::type::colon,     ":"}, line.substr(1)};
  case ';': return {{token::type::semicolon, ";"}, line.substr(1)};
  case '.': return {{token::type::dot,       "."}, line.substr(1)};

  case '+': return {{token::type::plus,    "+"}, line.substr(1)};
  case '-': return {{token::type::dash,    "-"}, line.substr(1)};
  case '~': return {{token::type::tilde,   "~"}, line.substr(1)};
  case '^': return {{token::type::caret,   "^"}, line.substr(1)};
  case '%': return {{token::type::percent, "%"}, line.substr(1)};
  case '/': return {{token::type::slash,   "/"}, line.substr(1)};

  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9': return digit_token(line);
  case '0': return zero_token(line);
  case '\'': return sym_token(line);
  case '=': return eq_tokens(line);
  case '!': return bang_tokens(line);
  case '*': return star_tokens(line);
  case '&': return and_tokens(line);
  case '|': return or_tokens(line);
  case '<': return lt_tokens(line);
  case '>': return gt_tokens(line);
  case '"': return string_token(line);
  case '`': return regex_token(line);
  default:  return name_token(line);
  }
}

// }}}

}

std::vector<token> parser::tokenize(std::istream& input)
{
  std::vector<token> tokens{};
  std::string current_line;
  while (input.peek() != EOF) {
    getline(input, current_line);
    boost::string_ref line{current_line};
    line = ltrim(line); // remove leading whitespace from line
    while (line.size() && !line.starts_with("//")) {
      auto res = first_token(line);
      tokens.push_back(res.first);
      line = ltrim(res.second);
    }
    tokens.push_back({token::type::newline, "\n"});
  }
  return tokens;
}
