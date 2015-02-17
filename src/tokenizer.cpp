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
                 ltrim(line.substr(post_dot - begin(line))) };
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
  return { {token::type::integer, num}, ltrim(line.substr(last - begin(line))) };
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
               ltrim(line.substr(nonfloat - begin(line)))};
  }

  return { {token::type::integer, {begin(line), nondigit}},
           ltrim(line.substr(nondigit - begin(line)))};
}

// }}}
// '=' {{{

tok_res eq_tokens(boost::string_ref line)
{
  if (line.size() == 1 || line[1] != '=')
    return { {token::type::assignment, "="}, ltrim(line.substr(1))};
  return { {token::type::equals, "=="}, ltrim(line.substr(2))};
}

// }}}
// '!' {{{

tok_res bang_tokens(boost::string_ref line)
{
  if (line.size() == 1 || line[1] != '=')
    return { {token::type::bang, "!"}, ltrim(line.substr(1))};
  return { {token::type::unequal, "!="}, ltrim(line.substr(2)) };
}

// }}}
// '*' {{{

tok_res star_tokens(boost::string_ref line)
{
  if (line.size() == 1 || line[1] != '*')
    return { {token::type::star, "*"}, ltrim(line.substr(1))};
  return { {token::type::double_star, "**"}, ltrim(line.substr(2))};
}

// }}}
// '&' {{{

tok_res and_tokens(boost::string_ref line)
{
  if (line.size() == 1 || line[1] != '&')
    return { {token::type::ampersand, "&"}, ltrim(line.substr(1))};
  return { {token::type::and_sign, "&&"}, ltrim(line.substr(2))};
}

// }}}
// '|' {{{

tok_res or_tokens(boost::string_ref line)
{
  if (line.size() == 1 || line[1] != '|')
    return { {token::type::pipe, "|"}, ltrim(line.substr(1))};
  return { {token::type::or_sign, "||"}, ltrim(line.substr(2))};
}

// }}}
// '<' {{{

tok_res lt_tokens(boost::string_ref line)
{
  if (line.size() > 1 && line[1] == '=')
    return { {token::type::less_eq, "<="}, ltrim(line.substr(2))};
  if (line.size() > 1 && line[1] == '<')
    return { {token::type::lshift, "<<"}, ltrim(line.substr(2))};
  return { {token::type::less, "<"}, ltrim(line.substr(1))};
}

// }}}
// '>' {{{

tok_res gt_tokens(boost::string_ref line)
{
  if (line.size() > 1 && line[1] == '=')
    return { {token::type::greater_eq, ">="}, ltrim(line.substr(2))};
  if (line.size() > 1 && line[1] == '>')
    return { {token::type::rshift, ">>"}, ltrim(line.substr(2))};
  return { {token::type::greater, ">"}, ltrim(line.substr(1))};
}

// }}}
// '"' {{{

char escaped(char nonescaped) {
  switch (nonescaped) {
  case 'a':  return '\a';
  case 'b':  return '\b';
  case 'n':  return '\n';
  case 'f':  return '\f';
  case 'r':  return '\r';
  case 't':  return '\t';
  case 'v':  return '\v';
  case '"':  return '"';
  case '\\': return '\\';
  case '0':  return '\0';
  default:   return nonescaped;
  }
}

tok_res string_token(boost::string_ref line)
{
  std::string token{'"'};
  line.remove_prefix(1);
  while (line.front() != '"') {
    if (line.front() == '\\') {
      line.remove_prefix(1);
      token += escaped(line.front());
    } else {
      token += line.front();
    }
    line.remove_prefix(1);
  }
  return { {token::type::string, token += '"'}, ltrim(line.substr(1))};
}

// }}}
// '/' {{{

tok_res slash_token(boost::string_ref line)
{
  if (line.size() == 1 || line[1] != '/')
    return { {token::type::slash, "/"}, ltrim(line.substr(1))};
  // Whoops, comment
  line.remove_prefix(1 + std::find(begin(line), end(line), '\n') - begin(line));
  return { {token::type::newline, "\n"}, line};
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
    return { {token::type::invalid, {line.front()}}, ltrim(line.substr(1)) };
  std::string name{begin(line), last};
  return { {type_for(name), name}, ltrim(line.substr(last - begin(line)))};
}

// }}}

tok_res first_token(boost::string_ref line)
{
  tok_res token;
  switch (line.front()) {
  case '{': return {{token::type::open_brace,    "{"}, ltrim(line.substr(1))};
  case '}': return {{token::type::close_brace,   "}"}, ltrim(line.substr(1))};
  case '[': return {{token::type::open_bracket,  "["}, ltrim(line.substr(1))};
  case ']': return {{token::type::close_bracket, "]"}, ltrim(line.substr(1))};
  case '(': return {{token::type::open_paren,    "("}, ltrim(line.substr(1))};
  case ')': return {{token::type::close_paren,   ")"}, ltrim(line.substr(1))};

  case ',': return {{token::type::comma,     ","}, ltrim(line.substr(1))};
  case ':': return {{token::type::colon,     ":"}, ltrim(line.substr(1))};
  case ';': return {{token::type::semicolon, ";"}, ltrim(line.substr(1))};
  case '.': return {{token::type::dot,       "."}, ltrim(line.substr(1))};
  case '\'': return {{token::type::quote,    "'"}, ltrim(line.substr(1))};

  case '+': return {{token::type::plus,    "+"}, ltrim(line.substr(1))};
  case '-': return {{token::type::dash,    "-"}, ltrim(line.substr(1))};
  case '~': return {{token::type::tilde,   "~"}, ltrim(line.substr(1))};
  case '^': return {{token::type::caret,   "^"}, ltrim(line.substr(1))};
  case '%': return {{token::type::percent, "%"}, ltrim(line.substr(1))};

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
  case '=': return eq_tokens(line);
  case '!': return bang_tokens(line);
  case '*': return star_tokens(line);
  case '&': return and_tokens(line);
  case '|': return or_tokens(line);
  case '<': return lt_tokens(line);
  case '>': return gt_tokens(line);
  case '"': return string_token(line);
  case '/': return slash_token(line);
  default:  return name_token(line);
  }
}

// }}}

}
#include <iostream>

std::vector<token> parser::tokenize(std::istream& input)
{
  std::vector<token> tokens{};
  std::string current_line;
  while (input.peek() != EOF) {
    getline(input, current_line);
    boost::string_ref line{current_line};
    line = ltrim(line); // remove leading whitespace from line
    while (line.size()) {
      auto res = first_token(line);
      tokens.push_back(res.first);
      line = res.second;
    }
    tokens.push_back({token::type::newline, "\n"});
  }
  return tokens;
}
