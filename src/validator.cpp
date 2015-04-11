#include "parser.h"

#include "utils/string_helpers.h"

using namespace vv;
using namespace parser;

namespace {

// Validates a string of tokens via recursive descent, so that when we're
// parsing it properly we don't have to worry about malformed syntax

bool trim_test(const token& t)
{
  return t.which == token::type::newline || t.which == token::type::semicolon;
};

bool newline_test(const token& t)
{
  return t.which == token::type::newline;
};

val_res val_toplevel(vector_ref<token> tokens);

val_res val_expression(vector_ref<token> tokens); // infix binop
val_res val_monop(vector_ref<token> tokens); // prefix monop
val_res val_accessor(vector_ref<token> tokens); // member, call, or index
val_res val_noop(vector_ref<token> tokens); // any other expression

val_res val_array_literal(vector_ref<token> tokens);
val_res val_dict_literal(vector_ref<token> tokens);
val_res val_assignment(vector_ref<token> tokens);
val_res val_block(vector_ref<token> tokens);
val_res val_cond_statement(vector_ref<token> tokens);
val_res val_except(vector_ref<token> tokens);
val_res val_for_loop(vector_ref<token> tokens);
val_res val_function_definition(vector_ref<token> tokens);
val_res val_literal(vector_ref<token> tokens);
val_res val_member(vector_ref<token> tokens);
val_res val_new_obj(vector_ref<token> tokens);
val_res val_require(vector_ref<token> tokens);
val_res val_return(vector_ref<token> tokens);
val_res val_try_catch(vector_ref<token> tokens);
val_res val_type_definition(vector_ref<token> tokens);
val_res val_variable_declaration(vector_ref<token> tokens);
val_res val_variable(vector_ref<token> tokens);
val_res val_while_loop(vector_ref<token> tokens);

// Literals
val_res val_bool(vector_ref<token> tokens);
val_res val_float(vector_ref<token> tokens);
val_res val_integer(vector_ref<token> tokens);
val_res val_nil(vector_ref<token> tokens);
val_res val_regex(vector_ref<token> tokens);
val_res val_string(vector_ref<token> tokens);
val_res val_symbol(vector_ref<token> tokens);

// Helpers
template <typename F>
val_res val_comma_separated_list(vector_ref<token> tokens,
                                 const F& val_item);
template <typename F>
val_res val_bracketed_subexpr(vector_ref<token> tokens,
                              const F& val_item,
                              token::type opening,
                              token::type closing);
// a: b pair, as in dicts or cond statements
val_res val_pair(vector_ref<token> tokens);

// Definitions

val_res val_toplevel(parser::token_string tokens)
{
  tokens = ltrim_if(tokens, trim_test);

  while (!tokens.empty()) {
    auto res = val_expression(tokens);
    if (!res)
      return res.invalid() ? res : tokens;
    tokens = ltrim_if(*res, trim_test);
  }
  return tokens;
}

val_res val_expression(vector_ref<token> tokens)
{
  auto res = val_monop(tokens);
  if (!res)
    return res;
  tokens = *res;
  if (tokens.empty())
    return tokens;
  switch (tokens.front().which) {
  case token::type::or_sign:
  case token::type::and_sign:
  case token::type::equals:
  case token::type::unequal:
  case token::type::greater_eq:
  case token::type::less_eq:
  case token::type::less:
  case token::type::greater:
  case token::type::to:
  case token::type::pipe:
  case token::type::caret:
  case token::type::ampersand:
  case token::type::lshift:
  case token::type::rshift:
  case token::type::plus:
  case token::type::dash:
  case token::type::star:
  case token::type::slash:
  case token::type::percent:
  case token::type::double_star: return val_expression(tokens.subvec(1));
  default: return res;
  }
}

val_res val_monop(vector_ref<token> tokens)
{
  if (tokens.empty())
    return {};
  switch (tokens.front().which) {
  case token::type::bang:
  case token::type::tilde:
  case token::type::dash: return val_monop(tokens.subvec(1));
  default: return val_accessor(tokens);
  }
}

val_res val_accessor(vector_ref<token> tokens)
{
  auto base = val_noop(tokens);
  if (!base || base->empty())
    return base;
  tokens = *base;
  while (!tokens.empty() && (tokens.front().which == token::type::open_paren
                         || tokens.front().which == token::type::open_bracket
                         || tokens.front().which == token::type::dot)) {

    if (tokens.front().which == token::type::open_paren) {
      auto args = val_bracketed_subexpr(tokens, [](auto t)
      {
        return val_comma_separated_list(t, val_expression);
      }, token::type::open_paren, token::type::close_paren);

      if (args.invalid())
        return args;
      if (!args)
        return {tokens.subvec(1), "expected argument list"};
      tokens = *args;
    }
    else {
      if (tokens.front().which == token::type::open_bracket) {
         const auto res = val_bracketed_subexpr(tokens,
                                                val_expression,
                                                token::type::open_bracket,
                                                token::type::close_bracket);
        if (res.invalid())
          return res;
        if (!res)
          return {tokens.subvec(1), "expected index expression"}; // '['
        tokens = *res;
        if (!tokens.empty() && tokens.front().which == token::type::assignment) {
          const auto expr = val_expression(tokens.subvec(1)); // '='
          if (expr.invalid())
            return expr;
          if (!expr)
            return {tokens.subvec(1), "expected assignment expression"};
          tokens = *expr;
        }
      }
      else { // dot
        const auto res = val_variable(tokens.subvec(1)); // '.'
        if (res.invalid())
          return res;
        if (!res)
          return {tokens.subvec(1), "expected method name"}; // '.'
        tokens = *res;
      }
    }
  }
  return tokens;
}

val_res val_noop(vector_ref<token> tokens)
{
  if (tokens.empty())
    return {};
  if (tokens.front().which == token::type::open_paren) {
    auto expr = val_bracketed_subexpr(tokens,
                                      val_expression,
                                      token::type::open_paren,
                                      token::type::close_paren);
    if (expr || expr.invalid())
      return expr;
    return {tokens.subvec(1), "expected expression in parentheses"}; // ')'
  }

  val_res res;
  if ((res = val_array_literal(tokens))        || res.invalid()) return res;
  if ((res = val_assignment(tokens))           || res.invalid()) return res;
  if ((res = val_block(tokens))                || res.invalid()) return res;
  if ((res = val_cond_statement(tokens))       || res.invalid()) return res;
  if ((res = val_dict_literal(tokens))         || res.invalid()) return res;
  if ((res = val_except(tokens))               || res.invalid()) return res;
  if ((res = val_for_loop(tokens))             || res.invalid()) return res;
  if ((res = val_function_definition(tokens))  || res.invalid()) return res;
  if ((res = val_literal(tokens))              || res.invalid()) return res;
  if ((res = val_member(tokens))               || res.invalid()) return res;
  if ((res = val_new_obj(tokens))              || res.invalid()) return res;
  if ((res = val_require(tokens))              || res.invalid()) return res;
  if ((res = val_return(tokens))               || res.invalid()) return res;
  if ((res = val_try_catch(tokens))            || res.invalid()) return res;
  if ((res = val_type_definition(tokens))      || res.invalid()) return res;
  if ((res = val_variable_declaration(tokens)) || res.invalid()) return res;
  if ((res = val_while_loop(tokens))           || res.invalid()) return res;
  if ((res = val_variable(tokens))             || res.invalid()) return res;
  return {};
}

val_res val_array_literal(vector_ref<token> tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::open_bracket)
    return {};
  auto body = val_bracketed_subexpr(tokens, [](auto t)
  {
    return val_comma_separated_list(t, val_expression);
  }, token::type::open_bracket, token::type::close_bracket);
  if (body || body.invalid())
    return body;
  return {tokens.subvec(1), "expected array literal"};
}

val_res val_dict_literal(vector_ref<token> tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::open_brace)
    return {};
  auto body = val_bracketed_subexpr(tokens, [](auto t)
  {
    return val_comma_separated_list(t, val_pair);
  }, token::type::open_brace, token::type::close_brace);
  if (body || body.invalid())
    return body;
  return {tokens.subvec(1), "expected dictionary literal"};
}

val_res val_assignment(vector_ref<token> tokens)
{
  auto expr = val_variable(tokens);
  if (!expr || expr->empty() || expr->front().which != token::type::assignment)
    return !expr ? expr : val_res{};
  tokens = expr->subvec(1); // '='
  auto val = val_expression(tokens);
  if (val || val.invalid())
    return val;
  return {tokens, "expected expression"};
}
val_res val_block(vector_ref<token> tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_do)
    return {};
  tokens = tokens.subvec(1); // 'do'
  tokens = ltrim_if(tokens, trim_test);

  while (!tokens.empty() && tokens.front().which != token::type::key_end) {
    auto res = val_expression(tokens);
    if (res.invalid())
      return res;
    if (!res)
      return {tokens, "expected expression or 'end'"};
    tokens = ltrim_if(*res, trim_test);
  }
  if (tokens.empty())
    return {tokens, "expected 'end'"};
  return tokens.subvec(1); // 'end'
}

val_res val_cond_statement(vector_ref<token> tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_cond)
    return {};
  tokens = ltrim_if(tokens.subvec(1), newline_test); // 'cond'
  auto body = val_comma_separated_list(tokens, val_pair);
  if (body || body.invalid())
    return body;
  return {tokens, "expected cond pair"};
}

val_res val_except(vector_ref<token> tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_except)
    return {};
  tokens = tokens.subvec(1); // 'except'
  auto body = val_expression(tokens);
  if (body || body.invalid())
    return body;
  return {tokens, "expected expression"};
}

val_res val_for_loop(vector_ref<token> tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_for)
    return {};

  auto iter = val_variable(tokens.subvec(1)); // 'for'
  if (iter.invalid())
    return iter;
  if (!iter)
    return {tokens.subvec(1), "expected variable name"}; // 'for'
  tokens = *iter;
  if (tokens.empty() || tokens.front().which != token::type::key_in)
    return {tokens, "expected 'in'"};

  auto range = val_expression(tokens.subvec(1)); // 'in'
  if (range.invalid())
    return range;
  if (!range)
    return {tokens.subvec(1), "expected expression"}; // 'in'
  tokens = *range;

  if (tokens.empty() || tokens.front().which != token::type::colon)
    return {tokens, "expected ':'"};
  auto body = val_expression(tokens.subvec(1)); // ':'
  if (body || body.invalid())
    return body;
  return {tokens.subvec(1), "expected expression"}; // ':'
}

val_res val_function_definition(vector_ref<token> tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_fn)
    return {};
  tokens = tokens.subvec(1); // 'fn'

  auto name = val_variable(tokens);
  if (name.invalid())
    return name;
  if (name)
    tokens = *name;

  auto arglist = val_bracketed_subexpr(tokens, [](auto t)
  {
    return val_comma_separated_list(t, val_variable);
  }, token::type::open_paren, token::type::close_paren);
  if (arglist.invalid())
    return arglist;
  if (!arglist)
    return {tokens, "expected argument list"};
  tokens = *arglist;

  if (tokens.empty() || tokens.front().which != token::type::colon)
    return {tokens, "expected ':'"};

  auto body = val_expression(tokens.subvec(1)); // ':'
  if (body || body.invalid())
    return body;
  return {tokens.subvec(1), "expected expression"}; // ':'
}

val_res val_member(vector_ref<token> tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::member)
    return {};

  tokens = tokens.subvec(1); // member
  if (!tokens.empty() && tokens.front().which == token::type::assignment) {
    auto expr = val_expression(tokens.subvec(1)); // '='
    if (expr || expr.invalid())
      return expr;
    return {tokens.subvec(1), "expected assignment expression"};
  }
  return tokens;
}

val_res val_literal(vector_ref<token> tokens)
{
  if (tokens.empty())
    return {};
  val_res res;
  if ((res = val_bool(tokens))    || res.invalid()) return res;
  if ((res = val_float(tokens))   || res.invalid()) return res;
  if ((res = val_integer(tokens)) || res.invalid()) return res;
  if ((res = val_nil(tokens))     || res.invalid()) return res;
  if ((res = val_regex(tokens))   || res.invalid()) return res;
  if ((res = val_string(tokens))  || res.invalid()) return res;
  if ((res = val_symbol(tokens))  || res.invalid()) return res;
  return {};
}

val_res val_new_obj(vector_ref<token> tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_new)
    return {};
  tokens = tokens.subvec(1); // 'new'

  auto name = val_noop(tokens);
  if (name.invalid())
    return name;
  if (!name)
    return {tokens, "expected variable name"};
  tokens = *name;

  auto args = val_bracketed_subexpr(tokens, [](auto t)
  {
    return val_comma_separated_list(t, val_expression);
  }, token::type::open_paren, token::type::close_paren);
  if (args || args.invalid())
    return args;
  return {tokens, "expected argument list"};
}

val_res val_require(vector_ref<token> tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_require)
    return {};
  auto value = val_string(tokens.subvec(1)); // 'require'
  if (value || value.invalid())
    return value;
  return {tokens.subvec(1), "expected expression"}; // 'require'
}

val_res val_return(vector_ref<token> tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_return)
    return {};
  auto value = val_expression(tokens.subvec(1)); // 'return'
  if (value || value.invalid())
    return value;
  return {tokens.subvec(1), "expected expression"}; // 'return'
}

val_res val_try_catch(vector_ref<token> tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_try)
    return {};
  if (tokens.size() < 2 || tokens[1].which != token::type::colon)
    return {tokens.subvec(1), "expected ':'"}; // 'try'
  tokens = tokens.subvec(2); // 'try' ':'

  auto body = val_expression(tokens);
  if (body.invalid())
    return body;
  if (!body)
    return {tokens, "expected expression"};
  tokens = ltrim_if(*body, newline_test);
  if (tokens.empty() || tokens.front().which != token::type::key_catch)
    return {tokens, "expected 'catch'"};
  auto name = val_variable(tokens.subvec(1)); // 'catch'
  if (name.invalid())
    return name;
  if (!name)
    return {tokens, "expected variable name"};
  tokens = *name;

  if (tokens.empty() || tokens.front().which != token::type::colon)
    return {tokens, "expected ':'"};
  tokens = tokens.subvec(1); // ':'

  auto catcher = val_expression(tokens);
  if (catcher || catcher.invalid())
    return catcher;
  return {tokens, "expected expression"};
}

val_res val_type_definition(vector_ref<token> tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_class)
    return {};
  auto name = val_variable(tokens.subvec(1)); // 'class'
  if (name.invalid())
    return name;
  if (!name)
    return {tokens.subvec(1), "expected variable name"}; // 'class'
  tokens = *name;

  if (!tokens.empty() && tokens.front().which == token::type::colon) {
    auto parent = val_variable(tokens.subvec(1)); // ':'
    if (parent.invalid())
      return parent;
    if (!parent)
      return {tokens.subvec(1), "expected variable name"}; // ':'
    tokens = *parent;
  }

  tokens = ltrim_if(tokens, trim_test);
  while (!tokens.empty() && tokens.front().which != token::type::key_end) {
    auto next_fn = val_function_definition(tokens);
    if (next_fn.invalid())
      return next_fn;
    if (!next_fn)
      return {tokens, "expected method definition or 'end'"};
    tokens = ltrim_if(*next_fn, trim_test);
  }
  if (!tokens.empty())
    return tokens.subvec(1); // 'end'
  return {tokens, "expected 'end'"};
}

val_res val_variable_declaration(vector_ref<token> tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_let)
    return {};
  auto name = val_variable(tokens.subvec(1)); // 'let'
  if (name.invalid())
    return name;
  if (!name)
    return {tokens.subvec(1), "expected variable name"}; // 'let'
  tokens = *name;
  if (tokens.empty() || tokens.front().which != token::type::assignment)
    return {tokens, "expected '='"};
  auto value = val_expression(tokens.subvec(1)); // '='
  if (value || value.invalid())
    return value;
  return {tokens.subvec(1), "expected expression"}; // '='
}

val_res val_variable(vector_ref<token> tokens)
{
  if (tokens.empty())
    return {};
  if (tokens.front().which == token::type::invalid)
    return {tokens, '\'' + tokens.front().str + "': invalid token"};
  if (tokens.front().which != token::type::name)
    return {};
  return tokens.subvec(1); // variable
}

val_res val_while_loop(vector_ref<token> tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_while)
    return {};
  tokens = tokens.subvec(1); // 'while'
  auto test = val_expression(tokens);
  if (test.invalid())
    return test;
  if (!test)
    return {tokens, "expected expression"};

  tokens = *test;
  if (tokens.empty() || tokens.front().which != token::type::colon)
    return {tokens, "expected ':'"};
  auto body = val_expression(tokens.subvec(1)); // ':'
  if (body || body.invalid())
    return body;
  return {tokens.subvec(1), "expected expression"}; // ':'
}

val_res val_bool(vector_ref<token> tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::boolean)
    return {};
  return tokens.subvec(1); // 'true'/'false'
}

val_res val_float(vector_ref<token> tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::floating_point)
    return {};
  return tokens.subvec(1); // number
}

val_res val_integer(vector_ref<token> tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::integer)
    return {};
  const auto& str = tokens.front().str;
  if (str.front() == '0' && str.size() > 1) {
    // all other errors should be dealt with in tokenizing
    if (!isdigit(str[1]) && str.size() == 2)
      return {tokens, "invalid number"};
  }
  return tokens.subvec(1); // number
}

val_res val_nil(vector_ref<token> tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::nil)
    return {};
  return tokens.subvec(1); // 'nil'
}

val_res val_regex(vector_ref<token> tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::regex)
    return {};
  return tokens.subvec(1); // string
}

val_res val_string(vector_ref<token> tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::string)
    return {};
  return tokens.subvec(1); // string
}

val_res val_symbol(vector_ref<token> tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::symbol)
    return {};
  return tokens.subvec(1); // symbol
}

template <typename F>
val_res val_comma_separated_list(vector_ref<token> tokens,
                                 const F& val_item)
{
  val_res res{};
  if (!val_item(tokens))
    return tokens;
  while ((res = val_item(tokens))) {
    tokens = *res;
    if (tokens.empty() || tokens.front().which != token::type::comma)
      return tokens;
    tokens = ltrim_if(tokens.subvec(1), newline_test); // ','
  }
  if (res.invalid())
    return res;
  return {tokens, "expected end of comma-separated list"};
}

template <typename F>
val_res val_bracketed_subexpr(vector_ref<token> tokens,
                              const F& val_item,
                              token::type opening,
                              token::type closing)
{
  if (tokens.empty() || tokens.front().which != opening)
    return {};
  tokens = ltrim_if(tokens.subvec(1), newline_test); // opening
  auto item = val_item(tokens);
  if (!item)
    return item;
  tokens = ltrim_if(*item, newline_test);
  if (tokens.empty() || tokens.front().which != closing)
    return {tokens, "expected closing"};
  return tokens.subvec(1); // closing
}

val_res val_pair(vector_ref<token> tokens)
{
  auto left = val_expression(tokens);
  if (!left)
    return left;
  tokens = *left;
  if (tokens.empty() || tokens.front().which != token::type::colon)
    return {tokens, "expected ':'"};
  auto right = val_expression(tokens.subvec(1)); // ':'
  if (right || right.invalid())
    return right;
  return {tokens.subvec(1), "expected expression"}; // ':'
}

}

val_res parser::is_valid(parser::token_string tokens)
{
  auto res = val_toplevel(tokens);
  if (res && !res->empty())
    return {*res, "expected end of input"};
  return res;
}
