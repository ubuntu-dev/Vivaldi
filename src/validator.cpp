#include "parser.h"

#include "utils/string_helpers.h"

using namespace vv;
using namespace parser;
using namespace std::literals;

namespace {

// Basic synposis:
// - Toplevel
// - Postfix and high-precedence infix operators (a[b], a(b, c...), a.b, a->b)
// - Prefix operators (!a, ~a, -a)
// - Low-precedence infix operators (a + b, a - b, etc)
// - Complex expressions (parenthesized expressions, function declarations, etc)
// - Single-token expressions (literals, variables, members)

// Declarations {{{

// High-level validators {{{

val_res val_toplevel(token_string tokens);
val_res val_expression(token_string tokens);
// Low-granularity precedence, since in most cases it doesn't matter
val_res val_prec2_expression(token_string tokens); // // most infixes
val_res val_prec1_expression(token_string tokens); // prefixes
val_res val_prec0_expression(token_string tokens); // brackets/parens, a.b, a->b

// }}}
// Complex expression validators {{{

val_res val_nonop_expression(token_string tokens);

val_res val_array_literal(token_string tokens);
val_res val_block(token_string tokens);
val_res val_cond_statement(token_string tokens);
val_res val_dictionary_literal(token_string tokens);
val_res val_throw(token_string tokens);
val_res val_for_loop(token_string tokens);
val_res val_function_definition(token_string tokens);
val_res val_lambda(token_string tokens);
val_res val_member_assignment(token_string tokens);
val_res val_object_creation(token_string tokens);
val_res val_require(token_string tokens);
val_res val_return(token_string tokens);
val_res val_try_catch(token_string tokens);
val_res val_type_definition(token_string tokens);
val_res val_variable_assignment(token_string tokens);
val_res val_variable_declaration(token_string tokens);
val_res val_while_loop(token_string tokens);

// }}}
// Single-token expression validators {{{

val_res val_single_token_expression(token_string tokens);

// }}}
// Helpers {{{

// Accepts token strings of the form
// opening
// [newlines]*
// inner
// [newlines]*
// closing
//
// where 'inner' is anything that innner_expr_validator returns a valid result
// for. Returns {} for anything not beginning with opening, and returns an
// invalid result for anything else.
template <typename F>
val_res val_delimited_expression(token_string tokens,
                                 token::type opening,
                                 token::type closing,
                                 const F& innner_expr_validator,
                                 const std::string& closing_name = "closing",
                                 const std::string& expr_name = "expression");

// Accepts token strings matching zero or more alternations of expr_func,
// sep_func, [newlines]*, beginning and ending with expr_func; returns invalid
// for anything else (never returns {}). Newlines at the beginning or end of the
// token string, as well as between expr_func's and sep_func's are *not* dealt
// with automatically.
template <typename SepFunc, typename ExprFunc>
val_res val_separated_list(token_string tokens,
                           const SepFunc& sep_validator,
                           const ExprFunc& expr_validator,
                           const std::string& expr_name = "expression");

// Strips zero or more token::type::newlines from the start of tokens
token_string trim_newline_group(token_string tokens);
// Strips zero or more token::type::newlines or token::type::semicolons from the
// start of tokens
token_string trim_newline_or_semicolon_group(token_string tokens);

// Return wrapped version of the trim_* equivalent, or {} if the token doesn't
// start with the appropriate tokens
val_res val_newline_or_semicolon_group(token_string tokens);

// Validates a single comma at the start of the token string
val_res val_single_comma(token_string tokens);

// Validates an expr: expr pair (used in cond statements and in dictionaries)
val_res val_colon_separated_pair(token_string tokens);

// Validates a method definition inside a type definition.
val_res val_method_definition(token_string tokens);

// Validates an argument list for a function, method, or lambda.
val_res val_arglist(token_string tokens);

// }}}

// }}}
// Definitions {{{

// High-level validators {{{

val_res val_toplevel(const token_string tokens)
{
  const auto trimmed = rtrim_if(trim_newline_or_semicolon_group(tokens),
                                [](const auto& tok)
  {
    return tok.which == token::type::newline
        || tok.which == token::type::semicolon;
  });
  const auto res = val_separated_list(trimmed,
                                      val_newline_or_semicolon_group,
                                      val_expression);

  if (!res || res->empty())
    return res;
  return {*res, "expected end of line"};
}

val_res val_expression(const token_string tokens)
{
  return val_prec2_expression(tokens);
}

val_res val_prec2_expression(const token_string tokens)
{
  const auto res = val_separated_list(tokens, [](const token_string sep_str)
  {
    if (sep_str.empty())
      return val_res{};
    switch (sep_str.front().which) {
    case token::type::plus:
    case token::type::slash:
    case token::type::star:
    case token::type::dash:
    case token::type::percent:
    case token::type::ampersand:
    case token::type::pipe:
    case token::type::caret:
    case token::type::lshift:
    case token::type::rshift:
    case token::type::equals:
    case token::type::unequal:
    case token::type::less:
    case token::type::greater:
    case token::type::less_eq:
    case token::type::greater_eq:
    case token::type::double_star:
    case token::type::and_sign:
    case token::type::or_sign:
    case token::type::to: return val_res{sep_str.subvec(1)};
    default: return val_res{};
    }
  }, val_prec1_expression);

  return (res && res->size() == tokens.size()) ? val_res{} : res;
}

val_res val_prec1_expression(const token_string tokens)
{
  auto cur_str = tokens;
  while (!cur_str.empty() && (cur_str.front().which == token::type::bang ||
                              cur_str.front().which == token::type::tilde ||
                              cur_str.front().which == token::type::dash)) {
    cur_str = trim_newline_group(cur_str.subvec(1)); // prefix
  }
  const auto res = val_prec0_expression(cur_str);
  if (res || res.invalid())
    return res;
  if (cur_str.size() == tokens.size())
    return {};
  return {cur_str, "expected expression"};
}

val_res val_prec0_expression(const token_string tokens)
{
  const auto base_res = val_nonop_expression(tokens);
  if (!base_res)
    return base_res;

  auto cur_str = *base_res;
  while (!cur_str.empty() && (cur_str.front().which == token::type::open_paren
                           || cur_str.front().which == token::type::open_bracket
                           || cur_str.front().which == token::type::dot
                           || cur_str.front().which == token::type::arrow)) {
    if (cur_str.front().which == token::type::open_paren) {
      const auto call_res = val_delimited_expression(cur_str,
                                                     token::type::open_paren,
                                                     token::type::close_paren,
                                                     [](const auto inner_str)
      {
        return val_separated_list(inner_str, val_single_comma, val_expression);
      }, "')'", "argument list");
      if (call_res.invalid())
        return call_res;
      if (!call_res)
        return {cur_str,
                "comma-separated list of expressions enclosed in parentheses"};
      cur_str = *call_res;
    }
    else if (cur_str.front().which == token::type::open_bracket) {
      const auto at_res = val_delimited_expression(cur_str,
                                                   token::type::open_bracket,
                                                   token::type::close_bracket,
                                                   val_expression,
                                                   "']'");
      if (at_res.invalid())
        return at_res;
      if (!at_res)
        return {cur_str.subvec(1), "expected expression enclosed in brackets"};
      cur_str = *at_res;
      if (!cur_str.empty() && cur_str.front().which == token::type::assignment) {
        const auto expr_str = trim_newline_group(cur_str.subvec(1)); // '='
        const auto expr_res = val_expression(expr_str);
        if (expr_res.invalid())
          return expr_res;
        if (!expr_res)
          return {expr_str, "expected expression"};
        cur_str = *expr_res;
      }
    }
    else { // dot, arrow
      const auto name_str = trim_newline_group(cur_str.subvec(1)); // '.' or '->'
      if (name_str.size() == 0 || name_str[0].which != token::type::name)
        return {name_str, "expected function name"};
      cur_str = trim_newline_group(name_str.subvec(1)); // function name
    }
  }
  return cur_str;
}

// }}}
// Complex expression validators {{{

val_res val_nonop_expression(const token_string tokens)
{
  if (!tokens.empty() && tokens.front().which == token::type::open_paren) {
    return val_delimited_expression(tokens,
                                    token::type::open_paren,
                                    token::type::close_paren,
                                    val_expression,
                                    "')'",
                                    "expression");
  }

  for (auto&& i : { val_array_literal,
                    val_block,
                    val_cond_statement,
                    val_dictionary_literal,
                    val_throw,
                    val_for_loop,
                    val_function_definition,
                    val_lambda,
                    val_member_assignment,
                    val_object_creation,
                    val_require,
                    val_return,
                    val_try_catch,
                    val_type_definition,
                    val_variable_assignment,
                    val_variable_declaration,
                    val_while_loop,
                    val_single_token_expression }) {

    const auto res = i(tokens);
    if (res || res.invalid())
      return res;
  }
  return {};
}

val_res val_array_literal(const token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::open_bracket)
    return {};
  return val_delimited_expression(tokens,
                                  token::type::open_bracket,
                                  token::type::close_bracket,
                                  [](const token_string inner_tokens)
  {
    return val_separated_list(inner_tokens, val_single_comma, val_expression,
                              "expression");
  }, "']'", "expression");
}

val_res val_block(const token_string tokens)
{
  // Can't use val_delimited_expression, since the 'separator' is an
  // indeterminate number of newlines/semicolons, such as might be placed before
  // the 'end' delimiter.
  //
  //   do
  //     1; 2; 3
  //   end
  //
  // will be interpreted like [1, 2, 3,]--- that is, an error
  if (tokens.empty() || tokens.front().which != token::type::key_do)
    return {};

  auto cur_str = trim_newline_or_semicolon_group(tokens.subvec(1)); // 'do';
  while (!cur_str.empty() && cur_str.front().which != token::type::key_end) {
    const auto expr_res = val_expression(cur_str);
    if (!expr_res) {
      return expr_res.valid() ? val_res{cur_str, "expected expression or 'end'"}
                              : expr_res;
    }

    cur_str = trim_newline_or_semicolon_group(*expr_res);
    if (cur_str.empty() || cur_str.front().which == token::type::key_end)
      break;
    if (cur_str.size() == expr_res->size())
      return {cur_str, "expected linebreak, ';', or 'end'"};
  }
  return cur_str.empty() ? val_res{cur_str, "expected 'end'"}
                         : cur_str.subvec(1); // 'end'
}

val_res val_cond_statement(const token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_cond)
    return {};

  const auto list_str = trim_newline_group(tokens.subvec(1)); // 'cond'

  const auto after_res = val_separated_list(list_str,
                                            val_single_comma,
                                            val_colon_separated_pair,
                                            "'predicate: result' pair");
  if (after_res.invalid())
    return after_res;
  if (!after_res || after_res->size() == list_str.size())
    return {list_str, "expected list of 'predicate: result' pairs"};
  return *after_res;
}

val_res val_dictionary_literal(const token_string tokens)
{
  return val_delimited_expression(tokens,
                                  token::type::open_brace,
                                  token::type::close_brace,
                                  [](const token_string inner_tokens)
  {
    return val_separated_list(inner_tokens,
                              val_single_comma,
                              val_colon_separated_pair,
                              "'key: value' pair");
  }, "'}'", "list of 'key: value' pairs");
}

val_res val_throw(const token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_throw)
    return {};
  const auto expr_res = val_expression(tokens.subvec(1)); // 'throw'
  if (expr_res || expr_res.invalid())
    return expr_res;
  return {tokens.subvec(1), "expected expression"}; // 'throw'
}

val_res val_for_loop(const token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_for)
    return {};

  const auto name_str = tokens.subvec(1); // 'for'
  if (name_str.empty() || name_str.front().which != token::type::name)
    return {name_str, "expected variable name"};

  const auto in_str = name_str.subvec(1); // name
  if (in_str.empty() || in_str.front().which != token::type::key_in)
    return {in_str, "expected 'in'"};

  const auto rng_str = in_str.subvec(1); // 'in'
  const auto rng_res = val_expression(rng_str);
  if (!rng_res)
    return rng_res.invalid() ? rng_res : val_res{rng_str, "expected expression"};

  if (rng_res->empty() || rng_res->front().which != token::type::colon)
    return {*rng_res, "expected ':'"};

  const auto body_str = trim_newline_group(rng_res->subvec(1)); // ':'
  const auto body_res = val_expression(body_str);
  if (body_res || body_res.invalid())
    return body_res;
  return {body_str, "expected expression"};
}

val_res val_function_definition(const token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_let)
    return {};

  const auto name_str = tokens.subvec(1); // 'let'
  if (name_str.empty() || name_str.front().which != token::type::name)
    return {name_str, "expected variable or function name"};

  const auto arglist_str = name_str.subvec(1); // name
  if (arglist_str.empty() || arglist_str.front().which != token::type::open_paren)
    return {}; // variable declaration

  const auto arglist_res = val_delimited_expression(arglist_str,
                                                    token::type::open_paren,
                                                    token::type::close_paren,
                                                    val_arglist);

  if (arglist_res.invalid())
    return arglist_res;
  if (!arglist_res)
    return {arglist_str, "expected argument list enclosed in parentheses"};

  const auto eq_str = *arglist_res;
  if (eq_str.empty() || eq_str.front().which != token::type::assignment)
    return {eq_str, "expected '='"};

  const auto body_str = trim_newline_group(eq_str.subvec(1)); // '='
  const auto body_res = val_expression(body_str);
  if (body_res || body_res.invalid())
    return body_res;

  return  {body_str, "expected expression"};
}

val_res val_lambda(const token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_fn)
    return {};

  const auto arglist_str = tokens.subvec(1); // 'fn'

  const auto arglist_res = val_delimited_expression(arglist_str,
                                                    token::type::open_paren,
                                                    token::type::close_paren,
                                                    val_arglist);

  if (arglist_res.invalid())
    return arglist_res;
  if (!arglist_res)
    return {arglist_str, "expected argument list enclosed in parentheses"};

  const auto colon_str = *arglist_res;
  if (colon_str.empty() || colon_str.front().which != token::type::colon)
    return {colon_str, "expected ':'"};

  const auto body_str = trim_newline_group(colon_str.subvec(1)); // ':'
  const auto body_res = val_expression(body_str);
  if (body_res || body_res.invalid())
    return body_res;

  return  {body_str, "expected expression"};
}

val_res val_member_assignment(const token_string tokens)
{
  if (tokens.size() < 2 || tokens[0].which != token::type::member
                        || tokens[1].which != token::type::assignment)
    return {};
  const auto expr_str = trim_newline_group(tokens.subvec(2)); // member, '='
  const auto expr_res = val_expression(expr_str);
  if (expr_res || expr_res.invalid())
    return expr_res;
  return {expr_str, "expected expression"};
}

val_res val_object_creation(const token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_new)
    return {};
  const auto type_str = tokens.subvec(1); // 'new'
  const auto type_res = val_nonop_expression(type_str);
  if (type_res.invalid())
    return type_res;
  if (!type_res)
    return {type_str, "expected expression"};

  const auto arglist_str = *type_res;
  const auto arglist_res = val_delimited_expression(arglist_str,
                                                    token::type::open_paren,
                                                    token::type::close_paren,
                                                    [](const auto inner_str)
  {
    return val_separated_list(inner_str, val_single_comma, val_expression);
  }, "comma-separated list of expressions enclosed in parentheses");

  if (arglist_res || arglist_res.invalid())
    return arglist_res;
  return {
    arglist_str,
    "expected comma-separated list of expressions enclosed in parentheses"
  };
}

val_res val_require(const token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_require)
    return {};
  const auto filename_str = tokens.subvec(1); // 'require'
  if (filename_str.empty() || filename_str.front().which != token::type::string)
    return {filename_str, "expected string"};
  return filename_str.subvec(1); // filename
}

val_res val_return(const token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_return)
    return {};
  const auto expr_res = val_expression(tokens.subvec(1)); // 'return'
  if (expr_res || expr_res.invalid())
    return expr_res;
  return {tokens.subvec(1), "expected expression"}; // 'return'
}

val_res val_try_catch(const token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_try)
    return {};
  if (tokens.size() < 2 || tokens[1].which != token::type::colon)
    return {tokens.subvec(1), "expected ':'"}; // 'try'

  const auto try_str = trim_newline_group(tokens.subvec(2)); // 'try', ':'
  const auto try_res = val_expression(try_str);
  if (try_res.invalid())
    return try_res;
  if (!try_res)
    return {try_str, "expected expression"};


  const auto catch_str = trim_newline_group(*try_res);
  if (catch_str.empty() || catch_str.front().which != token::type::key_catch)
    return {catch_str, "expected 'catch'"};
  const auto after_str = trim_newline_group(catch_str.subvec(1)); // 'catch'

  const auto after_res = val_separated_list(after_str,
                                            val_single_comma,
                                            [](const auto& tokens)
  {
    if (tokens.size() < 2 || tokens[0].which != token::type::name)
      return val_res{};
    if (tokens.size() < 2 || tokens[1].which != token::type::name)
      return val_res{tokens.subvec(1), "expected variable name"}; // 'catch'
    if (tokens.size() < 3 || tokens[2].which != token::type::colon)
      return val_res{tokens.subvec(2), "expected ':'"}; // 'catch', name
    return val_expression(trim_newline_group(tokens.subvec(3)));
  }, "'type name: expression'");

  if (after_res || after_res.invalid())
    return after_res;
  return {after_str, "expected list of catch expressions"};
}

val_res val_type_definition(const token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_class)
    return {};

  const auto class_str = tokens.subvec(1); // 'class'
  if (class_str.empty() || class_str.front().which != token::type::name)
    return {class_str, "expected class name"};

  auto cur_str = class_str.subvec(1); // classname
  if (!cur_str.empty() && cur_str.front().which == token::type::colon) {
    cur_str = cur_str.subvec(1);
    if (cur_str.empty() || cur_str.front().which != token::type::name)
      return {cur_str, "expected parent class name"};
    cur_str = cur_str.subvec(1);
  }
  cur_str = trim_newline_group(cur_str);

  // As per val_block, can't use val_separated_list because the newlines between
  // the final method and 'end' would get confused for a separator
  while (!cur_str.empty() && cur_str.front().which != token::type::key_end) {
    const auto method_res = val_method_definition(cur_str);
    if (method_res.invalid())
      return method_res;
    if (!method_res)
      return val_res{cur_str, "expected method definition"};
    cur_str = trim_newline_group(*method_res);
    if (cur_str.empty() || cur_str.front().which == token::type::key_end)
      break;
    if (cur_str.size() == method_res->size())
      return {cur_str, "expected linebreak or 'end'"};
  }
  return cur_str.empty() ? val_res{cur_str, "expected 'end'"}
                           : val_res{cur_str.subvec(1)}; // 'end'
}

val_res val_variable_assignment(const token_string tokens)
{
  if (tokens.size() < 2 || tokens[0].which != token::type::name
                        || tokens[1].which != token::type::assignment)
    return {};
  const auto expr_str = trim_newline_group(tokens.subvec(2)); // name, '='
  const auto expr_res = val_expression(expr_str);
  if (expr_res || expr_res.invalid())
    return expr_res;
  return {expr_str, "expected expression"};
}

val_res val_variable_declaration(const token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_let)
    return {};
  const auto name_str = tokens.subvec(1); // 'let'
  if (name_str.empty() || name_str.front().which != token::type::name)
    return {name_str, "expected variable name"};
  const auto eq_str = name_str.subvec(1); // name
  if (eq_str.empty() || eq_str.front().which != token::type::assignment)
    return {eq_str, "expected '='"};
  const auto expr_str = eq_str.subvec(1);
  const auto expr_res = val_expression(expr_str);
  if (expr_res || expr_res.invalid())
    return expr_res;
  return {expr_str, "expected expression"};
}

val_res val_while_loop(const token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_while)
    return {};
  const auto rng_str = tokens.subvec(1); // 'while'
  const auto rng_res = val_expression(rng_str);
  if (!rng_res)
    return rng_res.invalid() ? rng_res : val_res{rng_str, "expected expression"};

  if (rng_res->empty() || rng_res->front().which != token::type::colon)
    return {*rng_res, "expected ':'"};

  const auto body_str = trim_newline_group(rng_res->subvec(1)); // ':'
  const auto body_res = val_expression(body_str);
  if (body_res || body_res.invalid())
    return body_res;
  return {body_str, "expected expression"};
}

// }}}
// Single-token expression validators {{{

val_res val_single_token_expression(const token_string tokens)
{
  if (tokens.empty())
    return {};
  if (tokens.front().which == token::type::invalid)
    return {tokens, "invalid token"};

  switch (tokens.front().which) {
  case token::type::boolean:
  case token::type::character:
  case token::type::floating_point:
  case token::type::integer:
  case token::type::member:
  case token::type::name:
  case token::type::nil:
  case token::type::regex:
  case token::type::string:
  case token::type::symbol: return tokens.subvec(1);
  default: return {};
  }
}

// }}}
// Helpers {{{

template <typename F>
val_res val_delimited_expression(const token_string tokens,
                                 const token::type opening,
                                 const token::type closing,
                                 const F& innner_expr_validator,
                                 const std::string& closing_name,
                                 const std::string& expr_name)
{
  if (tokens.empty() || tokens.front().which != opening)
    return {};
  const auto expr_str = trim_newline_group(tokens.subvec(1)); // opening
  const auto expr_res = innner_expr_validator(expr_str);
  if (expr_res.invalid())
    return expr_res;
  if (!expr_res)
    return {expr_str, "expected " + expr_name};

  const auto closing_str = trim_newline_group(*expr_res);
  if (closing_str.empty() || closing_str.front().which != closing)
    return {closing_str, "expected " + closing_name};
  return closing_str.subvec(1); // closing
}

template <typename SepFunc, typename ExprFunc>
val_res val_separated_list(const token_string tokens,
                           const SepFunc& sep_validator,
                           const ExprFunc& expr_validator,
                           const std::string& expr_name)
{
  const auto first_expr_res = expr_validator(tokens);
  if (first_expr_res.invalid())
    return first_expr_res;
  if (!first_expr_res)
    return tokens;

  auto cur_str = *first_expr_res;
  for (;;) {
    const auto sep_res = sep_validator(cur_str);
    if (sep_res.invalid())
      return sep_res;
    if (!sep_res)
      return cur_str;

    const auto expr_str = trim_newline_group(*sep_res);
    const auto expr_res = expr_validator(expr_str);
    if (expr_res.invalid())
      return expr_res;
    if (!expr_res)
      return {expr_str, "expected " + expr_name};
    cur_str = *expr_res;
  }
}

token_string trim_newline_group(const token_string tokens)
{
  return ltrim_if(tokens, [](const auto& token)
  {
    return token.which == token::type::newline;
  });
}

token_string trim_newline_or_semicolon_group(const token_string tokens)
{
  return ltrim_if(tokens, [](const auto& token)
  {
    return token.which == token::type::newline
        || token.which == token::type::semicolon;
  });
}

val_res val_newline_or_semicolon_group(const token_string tokens)
{
  const auto after = trim_newline_or_semicolon_group(tokens);
  if (after.size() == tokens.size())
    return {};
  return after;
}

val_res val_single_comma(const token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::comma)
    return {};
  return tokens.subvec(1);
}

val_res val_colon_separated_pair(const token_string tokens)
{
  const auto first_res = val_expression(tokens);
  if (!first_res)
    return first_res;
  const auto colon_str = *first_res;
  if (colon_str.empty() || colon_str.front().which != token::type::colon)
    return {colon_str, "expected ':'"};

  const auto second_str = trim_newline_group(colon_str.subvec(1)); // ':'
  const auto second_res = val_expression(second_str);
  if (second_res || second_res.invalid())
    return second_res;
  return {second_str, "expected expression"};
}

val_res val_method_definition(const token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_let)
    return {};

  const auto name_str = tokens.subvec(1); // 'let'
  if (name_str.empty() || name_str.front().which != token::type::name)
    return {name_str, "expected method name"};

  const auto arglist_str = name_str.subvec(1); // name
  const auto arglist_res = val_delimited_expression(arglist_str,
                                                    token::type::open_paren,
                                                    token::type::close_paren,
                                                    val_arglist);

  if (arglist_res.invalid())
    return arglist_res;
  if (!arglist_res)
    return {arglist_str, "expected argument list enclosed in parentheses"};

  const auto eq_str = *arglist_res;
  if (eq_str.empty() || eq_str.front().which != token::type::assignment)
    return {eq_str, "expected '='"};

  const auto body_str = trim_newline_group(eq_str.subvec(1)); // '='
  const auto body_res = val_expression(body_str);
  if (body_res || body_res.invalid())
    return body_res;

  return  {body_str, "expected expression"};
}

val_res val_arglist(token_string tokens)
{
  if (tokens.size() && tokens.front().which == token::type::open_bracket) {
    if (tokens.size() < 3 || tokens[1].which != token::type::name
                          || tokens[2].which != token::type::close_bracket) {
      return {tokens, "expected variable name enclosed in brackets"};
    }
    return tokens.subvec(3);
  }

  if (tokens.front().which != token::type::name)
    return tokens;

  while (tokens.size() && tokens.front().which == token::type::name) {
    tokens = trim_newline_group(tokens.subvec(1));
    if (tokens.size() && tokens.front().which == token::type::comma) {
      tokens = trim_newline_group(tokens.subvec(1));
      if (tokens.size() && tokens.front().which == token::type::open_bracket) {
        if (tokens.size() < 3 || tokens[1].which != token::type::name
                              || tokens[2].which != token::type::close_bracket) {
          return {tokens, "expected variable name enclosed in brackets"};
        }
        return tokens.subvec(3);
      }
    }
    else {
      return tokens;
    }
  }
  return {tokens, "expected ','"};
}

// }}}

// }}}

}

val_res parser::is_valid(const token_string tokens)
{
  return val_toplevel(tokens);
}
