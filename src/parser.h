#ifndef VV_PARSER_H
#define VV_PARSER_H

#include "expression.h"
#include "utils/vector_ref.h"

#include <boost/optional/optional.hpp>

#include <istream>
#include <string>
#include <vector>

namespace vv {

namespace parser {

struct token {
  enum class type {
    newline,

    open_paren,
    close_paren,
    open_bracket,
    close_bracket,
    open_brace,
    close_brace,

    dot,
    semicolon,
    colon,
    comma,

    plus,
    slash,
    star,
    dash,
    bang,
    tilde,
    percent,
    ampersand,
    pipe,
    caret,
    lshift,
    rshift,

    arrow,

    assignment,
    equals,
    unequal,
    less,
    greater,
    less_eq,
    greater_eq,

    double_star,
    and_sign,
    or_sign,

    to,

    key_let,

    key_class,
    key_fn,

    key_do,
    key_end,
    key_cond,
    key_while,
    key_for,
    key_in,
    key_return,

    key_require,

    key_try,
    key_catch,
    key_throw,

    member,
    name,

    character,
    integer,
    floating_point,
    regex,
    string,
    symbol,
    boolean,
    nil,

    invalid

  };
  type which;
  std::string str;
};

using token_string = vector_ref<token>;

// TODO: Make this internal to validator.cpp, and expose a simpler interface
class val_res {
public:
  val_res(token_string token) : m_tokens{token} { }
  val_res() { }
  val_res(token_string where, const std::string& what)
    : m_tokens {where},
      m_error  {what}
  { }

  const std::string& error() const { return *m_error; }

  auto operator*() const { return *m_tokens; }
  auto* operator->() const { return &*m_tokens; }

  operator bool() const { return m_tokens && !m_error; }

  bool invalid() const { return m_error.operator bool(); }
  bool valid() const { return !m_error; }

private:
  boost::optional<token_string> m_tokens;
  boost::optional<std::string> m_error;
};

std::vector<token> tokenize(std::istream& input);

val_res is_valid(token_string tokens);

std::vector<std::unique_ptr<ast::expression>> parse(token_string tokens);

}

}

#endif
