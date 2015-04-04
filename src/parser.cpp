#include "parser.h"

#include "builtins.h"
#include "ast/assignment.h"
#include "ast/array.h"
#include "ast/block.h"
#include "ast/cond_statement.h"
#include "ast/dictionary.h"
#include "ast/except.h"
#include "ast/for_loop.h"
#include "ast/function_call.h"
#include "ast/function_definition.h"
#include "ast/literal.h"
#include "ast/logical_and.h"
#include "ast/logical_or.h"
#include "ast/member.h"
#include "ast/member_assignment.h"
#include "ast/object_creation.h"
#include "ast/require.h"
#include "ast/return_statement.h"
#include "ast/try_catch.h"
#include "ast/type_definition.h"
#include "ast/variable.h"
#include "ast/variable_declaration.h"
#include "ast/while_loop.h"
#include "utils/string_helpers.h"

// Implements a fairly simple recursive-descent parser

using namespace vv;
using namespace ast;
using namespace parser;

// Parsing {{{

namespace {

bool trim_test(const token& t)
{
  return t.which == token::type::newline || t.which == token::type::semicolon;
};

bool newline_test(const token& t)
{
  return t.which == token::type::newline;
};

// Declarations {{{

template <typename T = std::unique_ptr<expression>>
using parse_res = boost::optional<std::pair<T, token_string>>;

using arg_t = std::vector<std::unique_ptr<expression>>;

parse_res<> parse_expression(token_string tokens);

parse_res<> parse_nonop_expression(token_string tokens);
parse_res<> parse_prec0(token_string tokens); // member, call, index
parse_res<> parse_prec1(token_string tokens); // monops
parse_res<> parse_prec2(token_string tokens); // **
parse_res<> parse_prec3(token_string tokens); // *, /, %
parse_res<> parse_prec4(token_string tokens); // +, -
parse_res<> parse_prec5(token_string tokens); // <<, >>
parse_res<> parse_prec6(token_string tokens); // &
parse_res<> parse_prec7(token_string tokens); // ^
parse_res<> parse_prec8(token_string tokens); // |
parse_res<> parse_prec9(token_string tokens); // to
parse_res<> parse_prec10(token_string tokens); // <, >, <=, =>
parse_res<> parse_prec11(token_string tokens); // ==, !=
parse_res<> parse_prec12(token_string tokens); // &&
parse_res<> parse_prec13(token_string tokens); // ||

parse_res<> parse_array_literal(token_string tokens);
parse_res<> parse_dict_literal(token_string tokens);
parse_res<> parse_assignment(token_string tokens);
parse_res<> parse_block(token_string tokens);
parse_res<> parse_cond_statement(token_string tokens);
parse_res<> parse_except(token_string tokens);
parse_res<> parse_for_loop(token_string tokens);
parse_res<> parse_function_definition(token_string tokens);
parse_res<> parse_literal(token_string tokens);
parse_res<> parse_new_obj(token_string tokens);
parse_res<> parse_require(token_string tokens);
parse_res<> parse_return(token_string tokens);
parse_res<> parse_try_catch(token_string tokens);
parse_res<> parse_type_definition(token_string tokens);
parse_res<> parse_variable_declaration(token_string tokens);
parse_res<> parse_variable(token_string tokens);
parse_res<> parse_while_loop(token_string tokens);

parse_res<> parse_symbol(token_string tokens);
parse_res<> parse_integer(token_string tokens);
parse_res<> parse_float(token_string tokens);
parse_res<> parse_bool(token_string tokens);
parse_res<> parse_nil(token_string tokens);
parse_res<> parse_regex(token_string tokens);
parse_res<> parse_string(token_string tokens);

template <typename F>
auto parse_comma_separated_list(token_string tokens,
                               const F& parse_item)
    -> parse_res<std::vector<decltype(parse_item(tokens)->first)>>;
template <typename F>
auto parse_bracketed_subexpr(token_string tokens,
                             const F& parse_item,
                             token::type opening,
                             token::type closing)
    -> decltype(parse_item(tokens));
parse_res<arg_t> parse_function_call(token_string tokens);
parse_res<std::pair<std::unique_ptr<expression>, std::unique_ptr<expression>>>
  parse_cond_pair(token_string tokens);
parse_res<std::pair<symbol, function_definition>>
  parse_method_definition(token_string tokens);
// }}}
// Individual parsing functions {{{

parse_res<> parse_expression(token_string tokens)
{
  return parse_prec13(tokens);
}

// Operators {{{

template <typename F1, typename Pred, typename Transform>
parse_res<> parse_operator_expr(token_string tokens,
                                const F1& lower_pred,
                                const Pred& test,
                                const Transform& convert)
{
  auto left_res = lower_pred(tokens);
  if (!left_res)
    return left_res;
  tokens = left_res->second;

  while (tokens.size() && test(tokens.front().which)) {
    auto left = move(left_res->first);

    symbol method{convert(tokens.front().which)};

    auto right_res = lower_pred(tokens.subvec(1)); // method (i.e. binop)
    auto right = move(right_res->first);
    tokens = right_res->second;

    auto member = std::make_unique<ast::member>( move(left), method );
    arg_t arg{};
    arg.emplace_back(move(right));

    left_res = {{std::make_unique<function_call>(move(member), move(arg)), tokens}};
  }
  return left_res;
}

parse_res<> parse_prec13(token_string tokens)
{
  auto left_res = parse_prec12(tokens);
  if (!left_res)
    return left_res;
  tokens = left_res->second;

  while (tokens.size() && tokens.front().which == token::type::or_sign) {
    auto left = move(left_res->first);

    auto right_res = parse_prec13(tokens.subvec(1)); // '||'
    auto right = move(right_res->first);
    tokens = right_res->second;

    left_res = {{ std::make_unique<logical_or>(move(left), move(right)), tokens }};
  }
  return left_res;
}

parse_res<> parse_prec12(token_string tokens)
{
  auto left_res = parse_prec11(tokens);
  if (!left_res)
    return left_res;
  tokens = left_res->second;

  while (tokens.size() && tokens.front().which == token::type::and_sign) {
    auto left = move(left_res->first);
    auto right_res = parse_prec12(tokens.subvec(1)); // '&&'
    auto right = move(right_res->first);
    tokens = right_res->second;

    left_res = {{ std::make_unique<logical_and>(move(left), move(right)), tokens }};
  }
  return left_res;
}

parse_res<> parse_prec11(token_string tokens)
{
  return parse_operator_expr(tokens, parse_prec10,
                             [](auto t)
                             {
                               return t == token::type::equals
                                   || t == token::type::unequal;
                             },
                             [](auto t)
                             {
                               return (t == token::type::equals) ? "equals"
                                                                 : "unequal";
                             });
}

parse_res<> parse_prec10(token_string tokens)
{
  return parse_operator_expr(tokens, parse_prec9,
                             [](auto t)
                             {
                               return t == token::type::greater
                                   || t == token::type::less
                                   || t == token::type::greater_eq
                                   || t == token::type::less_eq;
                             },
                             [](auto s)
                             {
                               return (s == token::type::greater)    ? "greater"        :
                                      (s == token::type::less)       ? "less"           :
                                      (s == token::type::greater_eq) ? "greater_equals" :
                                                                       "less_equals";
                             });
}

parse_res<> parse_prec9(token_string tokens)
{
  auto left_res = parse_prec8(tokens);
  if (!left_res)
    return left_res;
  tokens = left_res->second;

  while (tokens.size() && tokens.front().which == token::type::to) {
    auto left = move(left_res->first);
    auto right_res = parse_prec9(tokens.subvec(1)); // 'to'
    auto right = move(right_res->first);
    tokens = right_res->second;

    arg_t args;
    args.emplace_back(move(left));
    args.emplace_back(move(right));

    auto range = std::make_unique<variable>( symbol{"Range"} );

    left_res = {{std::make_unique<object_creation>(move(range), move(args)), tokens}};
  }
  return left_res;
}

parse_res<> parse_prec8(token_string tokens)
{
  return parse_operator_expr(tokens, parse_prec7,
                             [](auto t) { return t == token::type::pipe; },
                             [](auto) { return "bitor"; });
}

parse_res<> parse_prec7(token_string tokens)
{
  return parse_operator_expr(tokens, parse_prec6,
                             [](auto t) { return t == token::type::caret; },
                             [](auto) { return "xor"; });
}

parse_res<> parse_prec6(token_string tokens)
{
  return parse_operator_expr(tokens, parse_prec5,
                             [](auto t) { return t == token::type::ampersand; },
                             [](auto) { return "bitand"; });
}

parse_res<> parse_prec5(token_string tokens)
{
  return parse_operator_expr(tokens, parse_prec4,
                             [](auto t)
                             {
                               return t == token::type::lshift
                                   || t == token::type::rshift;
                             },
                             [](auto t)
                             {
                               return (t == token::type::rshift) ? "rshift" : "lshift";
                             });
}

parse_res<> parse_prec4(token_string tokens)
{
  return parse_operator_expr(tokens, parse_prec3,
                             [](auto t) { return t == token::type::plus
                                              || t == token::type::dash; },
                             [](auto t)
                             {
                               return (t == token::type::plus) ? "add" : "subtract";
                             });
}

parse_res<> parse_prec3(token_string tokens)
{
  return parse_operator_expr(tokens, parse_prec2,
                             [](auto s)
                             {
                               return s == token::type::star
                                   || s == token::type::slash
                                   || s == token::type::percent;
                             },
                             [](auto t)
                             {
                               return (t == token::type::star)  ? "times"   :
                                      (t == token::type::slash) ? "divides" :
                                                                  "modulo";
                             });
}

parse_res<> parse_prec2(token_string tokens)
{
  return parse_operator_expr(tokens, parse_prec1,
                             [](auto t) { return t == token::type::double_star; },
                             [](auto) { return "pow"; });
}

parse_res<> parse_prec1(token_string tokens)
{
  if (tokens.size() && (tokens.front().which == token::type::bang
                     || tokens.front().which == token::type::tilde
                     || tokens.front().which == token::type::dash)) {


    symbol method{(tokens.front().which == token::type::bang)  ? "not" :
                  (tokens.front().which == token::type::tilde) ? "negate" :
                                                                 "negative"};
    auto expr_res = parse_prec1(tokens.subvec(1)); // monop
    tokens = expr_res->second;
    auto expr = move(expr_res->first);
    auto member = std::make_unique<ast::member>( move(expr), method );

    return {{ std::make_unique<function_call>( move(member), arg_t{} ),
              tokens }};
  }
  return parse_prec0(tokens);
}

// TODO: split function calls, members, and indexing into their own functions so
// this one isn't so monstruously long
parse_res<> parse_prec0(token_string tokens)
{
  auto expr_res = parse_nonop_expression(tokens);
  if (!expr_res)
    return expr_res;
  tokens = expr_res->second;

  auto expr = move(expr_res->first);
  while (tokens.size() && (tokens.front().which == token::type::open_paren
                        || tokens.front().which == token::type::dot
                        || tokens.front().which == token::type::open_bracket)) {
    if (tokens.front().which == token::type::open_paren) {
      auto list_res = parse_function_call(tokens);
      auto list = move(list_res->first);
      tokens = list_res->second;

      expr = std::make_unique<function_call>(move(expr), move(list));
    }
    else if (tokens.front().which == token::type::open_bracket) {
      auto idx_res = parse_bracketed_subexpr(tokens,
                                             parse_expression,
                                             token::type::open_bracket,
                                             token::type::close_bracket);
      auto idx = move(idx_res->first);
      tokens = idx_res->second;

      if (tokens.size() && tokens.front().which == token::type::assignment) {
        auto value_res = parse_expression(tokens.subvec(1)); // '='
        auto value = move(value_res->first);
        tokens = value_res->second;

        auto member = std::make_unique<ast::member>( move(expr), symbol{"set_at"} );
        arg_t args{};
        args.emplace_back(move(idx));
        args.emplace_back(move(value));

        return {{ std::make_unique<function_call>( move(member), move(args) ),
                  tokens }};
      }

      auto member = std::make_unique<ast::member>( move(expr), symbol{"at"} );
      arg_t arg{};
      arg.emplace_back(move(idx));
      expr = std::make_unique<function_call>(move(member), move(arg));
    }
    else {
      tokens = tokens.subvec(1); // '.'
      symbol name{tokens.front().str};
      tokens = tokens.subvec(1); // name

      if (tokens.size() && tokens.front().which == token::type::assignment) {
        auto value_res = parse_expression(tokens.subvec(1)); // '='
        auto value = move(value_res->first);
        tokens = value_res->second;
        return {{ std::make_unique<member_assignment>( move(expr),
                                                       name,
                                                       move(value) ),
                  tokens }};
      }
      expr = std::make_unique<member>( move(expr), name );
    }
  }
  return {{ move(expr), tokens }};
}

parse_res<> parse_nonop_expression(token_string tokens)
{
  if (tokens.size() && tokens.front().which == token::type::open_paren) {
    return parse_bracketed_subexpr(tokens,
                                   parse_expression,
                                   token::type::open_paren,
                                   token::type::close_paren);
  }

  parse_res<> res;
  if ((res = parse_array_literal(tokens)))        return res;
  if ((res = parse_assignment(tokens)))           return res;
  if ((res = parse_block(tokens)))                return res;
  if ((res = parse_cond_statement(tokens)))       return res;
  if ((res = parse_dict_literal(tokens)))         return res;
  if ((res = parse_except(tokens)))               return res;
  if ((res = parse_for_loop(tokens)))             return res;
  if ((res = parse_function_definition(tokens)))  return res;
  if ((res = parse_literal(tokens)))              return res;
  if ((res = parse_new_obj(tokens)))              return res;
  if ((res = parse_require(tokens)))              return res;
  if ((res = parse_return(tokens)))               return res;
  if ((res = parse_try_catch(tokens)))            return res;
  if ((res = parse_type_definition(tokens)))      return res;
  if ((res = parse_variable_declaration(tokens))) return res;
  if ((res = parse_while_loop(tokens)))           return res;
  if ((res = parse_variable(tokens)))             return res;
  return {};
}

// }}}
// Other expressions {{{

parse_res<> parse_assignment(token_string tokens)
{
  if (tokens.size() < 2 || tokens[1].which != token::type::assignment)
    return {};
  symbol name{tokens.front().str};
  auto expr_res = parse_expression(tokens.subvec(2)); // name '='
  auto expr = move(expr_res->first);
  tokens = expr_res->second;
  return {{ std::make_unique<assignment>( name, move(expr) ), tokens }};
}

parse_res<> parse_block(token_string tokens)
{
  if (!tokens.size() || tokens.front().which != token::type::key_do)
    return {};
  tokens = tokens.subvec(1); // 'do'

  std::vector<std::unique_ptr<expression>> subexprs;
  tokens = ltrim_if(tokens, trim_test);

  while (tokens.front().which != token::type::key_end) {
    auto expr_res = parse_expression(tokens);
    subexprs.push_back(move(expr_res->first));
    tokens = expr_res->second;
    tokens = ltrim_if(tokens, trim_test);
  }

  tokens = tokens.subvec(1); // 'end'
  return {{ std::make_unique<block>( move(subexprs) ), tokens }};
}

parse_res<> parse_array_literal(token_string tokens)
{
  if (!tokens.size() || tokens.front().which != token::type::open_bracket)
    return {};

  auto vals_res = parse_bracketed_subexpr(tokens, [](auto t)
  {
    return parse_comma_separated_list(t, parse_expression);
  }, token::type::open_bracket, token::type::close_bracket);
  auto vals = move(vals_res->first);
  tokens = vals_res->second;

  return {{ std::make_unique<array>( move(vals) ), tokens }};
}

parse_res<> parse_dict_literal(token_string tokens)
{
  if (!tokens.size() || tokens.front().which != token::type::open_brace)
    return {};

  auto vals_res = parse_bracketed_subexpr(tokens, [](auto t)
  {
    return parse_comma_separated_list(t, parse_cond_pair);
  }, token::type::open_brace, token::type::close_brace);
  auto vals = move(vals_res->first);
  tokens = vals_res->second;
  // inefficient, but do I really care at this point?
  arg_t flattened;
  for (auto& i : vals) {
    flattened.push_back(move(i.first));
    flattened.push_back(move(i.second));
  }

  return {{ std::make_unique<dictionary>( move(flattened) ), tokens }};
}

parse_res<> parse_cond_statement(token_string tokens)
{
  if (!tokens.size() || tokens.front().which != token::type::key_cond)
    return {};
  tokens = ltrim_if(tokens.subvec(1), newline_test);

  auto pairs_res = parse_comma_separated_list(tokens, parse_cond_pair);
  auto pairs = move(pairs_res->first);
  tokens = pairs_res->second;

  return {{ std::make_unique<cond_statement>(move(pairs)), tokens }};
}

parse_res<> parse_except(token_string tokens)
{
  if (!tokens.size() || tokens.front().which != token::type::key_except)
    return {};
  tokens = tokens.subvec(1); // 'except'
  auto expr_res = parse_expression(tokens);
  auto expr = move(expr_res->first);
  tokens = expr_res->second;
  return {{ std::make_unique<except>( move(expr) ), tokens }};
}

parse_res<> parse_for_loop(token_string tokens)
{
  if (!tokens.size() || tokens.front().which != token::type::key_for)
    return {};
  tokens = tokens.subvec(1); // 'for'

  symbol iterator{tokens.front().str};
  tokens = tokens.subvec(2); // iterator 'in'

  auto range_res = parse_expression(tokens);
  auto range = move(range_res->first);
  tokens = range_res->second.subvec(1); // ':'

  auto body_res = parse_expression(tokens); // ':'
  auto body = move(body_res->first);
  tokens = body_res->second;

  return {{ std::make_unique<for_loop>( iterator, move(range), move(body) ),
            tokens }};
}

parse_res<> parse_function_definition(token_string tokens)
{
  if (!tokens.size() || tokens.front().which != token::type::key_fn)
    return {};
  tokens = tokens.subvec(1);

  symbol name;
  if (tokens.front().which != token::type::open_paren) {
    name = tokens.front().str;
    tokens = tokens.subvec(1); // name
  }

  auto arg_res = parse_bracketed_subexpr(tokens, [](auto tokens)
  {
    return parse_comma_separated_list(tokens, [](auto t)
    {
      if (t.front().which == token::type::close_paren)
        return parse_res<symbol>{};
      return parse_res<symbol>{{ t.front().str, t.subvec(1) }}; // arg name
    });
  }, token::type::open_paren, token::type::close_paren);
  auto args = move(arg_res->first);
  tokens = arg_res->second;

  auto body_res = parse_expression(tokens.subvec(1)); // ':'
  auto body = move(body_res->first);
  tokens = body_res->second;

  return {{ std::make_unique<function_definition>( name, move(body), args ),
            tokens }};
}

parse_res<> parse_literal(token_string tokens)
{
  parse_res<> res;
  if ((res = parse_float(tokens)))   return res;
  if ((res = parse_integer(tokens))) return res;
  if ((res = parse_bool(tokens)))    return res;
  if ((res = parse_nil(tokens)))     return res;
  if ((res = parse_regex(tokens)))   return res;
  if ((res = parse_string(tokens)))  return res;
  if ((res = parse_symbol(tokens)))  return res;
  return res;
}

parse_res<> parse_new_obj(token_string tokens)
{
  if (!tokens.size() || tokens.front().which != token::type::key_new)
    return {};
  tokens = tokens.subvec(1); // 'new'

  auto type_res = parse_variable(tokens);
  auto type = move(type_res->first);
  tokens = type_res->second;
  tokens = type_res->second;

  auto args_res = parse_function_call(tokens);
  auto args = move(args_res->first);
  tokens = args_res->second;

  return {{ std::make_unique<object_creation>( move(type), move(args) ),
            tokens }};
}

parse_res<> parse_require(token_string tokens)
{
  if (!tokens.size() || tokens.front().which != token::type::key_require)
    return {};
  tokens = tokens.subvec(1); // 'require'
  std::string filename{++begin(tokens.front().str), --end(tokens.front().str)};
  tokens = tokens.subvec(1); // filename
  return {{ std::make_unique<require>( filename ), tokens }};
}

parse_res<> parse_return(token_string tokens)
{
  if (!tokens.size() || tokens.front().which != token::type::key_return)
    return {};
  tokens = tokens.subvec(1); // 'except'
  auto expr_res = parse_expression(tokens);
  auto expr = move(expr_res->first);
  tokens = expr_res->second;
  return {{ std::make_unique<return_statement>( move(expr) ), tokens }};
}

parse_res<> parse_try_catch(token_string tokens)
{
  if (!tokens.size() || tokens.front().which != token::type::key_try)
    return {};
  tokens = tokens.subvec(2); // 'try' ':'

  auto body_res = parse_expression(tokens);
  auto body = move(body_res->first);
  tokens = body_res->second;

  tokens = ltrim_if(tokens, newline_test);
  tokens = tokens.subvec(1); // 'catch'
  symbol exception_name{tokens.front().str};
  std::vector<symbol> exception_arg{exception_name};
  tokens = tokens.subvec(2); // name ':

  auto catcher_res = parse_expression(tokens);
  auto catcher = move(catcher_res->first);
  tokens = catcher_res->second;

  return {{std::make_unique<try_catch>(move(body),exception_name,move(catcher)),
           tokens}};
}

parse_res<> parse_type_definition(token_string tokens)
{
  if (!tokens.size() || tokens.front().which != token::type::key_class)
    return{};
  symbol name{tokens[1].str};
  tokens = tokens.subvec(2); // 'class' name

  symbol parent{"Object"};
  if (tokens.front().which == token::type::colon) {
    parent = tokens[1].str;
    tokens = tokens.subvec(2); // ':' parent
  }

  std::unordered_map<symbol, function_definition> method_map;
  tokens = ltrim_if(tokens, trim_test);
  while (tokens.front().which != token::type::key_end) {
    auto method = parse_method_definition(tokens);
    method_map.insert(std::make_pair(method->first.first,
                                     method->first.second));
    tokens = ltrim_if(method->second, trim_test);
  }
  tokens = tokens.subvec(1); // 'end'

  return {{ std::make_unique<type_definition>( name, parent, move(method_map) ),
            tokens }};
}

parse_res<> parse_variable_declaration(token_string tokens)
{
  if (!tokens.size() || tokens.front().which != token::type::key_let)
    return {};
  symbol name{tokens[1].str};
  auto expr_res = parse_expression(tokens.subvec(3)); // 'let' name '='
  auto expr = move(expr_res->first);
  tokens = expr_res->second;
  return {{ std::make_unique<variable_declaration>(name, move(expr)), tokens }};
}

parse_res<> parse_variable(token_string tokens)
{
  if (!tokens.size() || tokens.front().which != token::type::name)
    return {};
  symbol name{tokens.front().str};
  tokens = tokens.subvec(1); // name
  return {{ std::make_unique<variable>(name), tokens }};
}

parse_res<> parse_while_loop(token_string tokens)
{
  if (!tokens.size() || tokens.front().which != token::type::key_while)
    return {};

  auto test_res = parse_expression(tokens.subvec(1)); // 'while'
  auto test = move(test_res->first);
  tokens = test_res->second;

  auto body_res = parse_expression(tokens.subvec(1)); // ':'
  auto body = move(body_res->first);
  tokens = body_res->second;

  return {{ std::make_unique<while_loop>( move(test), move(body) ), tokens }};
}

// }}}
// Literals {{{

parse_res<> parse_integer(token_string tokens)
{
  if (!tokens.size() || tokens.front().which != token::type::integer)
    return {};
  auto str = tokens.front().str;
  tokens = tokens.subvec(1); // number

  return {{ std::make_unique<literal::integer>( to_int(str) ), tokens }};
}

parse_res<> parse_float(token_string tokens)
{
  if (!tokens.size() || tokens.front().which != token::type::floating_point)
    return {};
  auto str = tokens.front().str;
  tokens = tokens.subvec(1); // number

  return {{ std::make_unique<literal::floating_point>( stod(str) ), tokens }};
}

parse_res<> parse_bool(token_string tokens)
{
  if (!tokens.size() || tokens.front().which != token::type::boolean)
    return {};
  bool value{tokens.front().str == "true"};
  tokens = tokens.subvec(1); // value
  return {{ std::make_unique<literal::boolean>( value ), tokens }};
}

parse_res<> parse_nil(token_string tokens)
{
  if (!tokens.size() || tokens.front().which != token::type::nil)
    return {};
  tokens = tokens.subvec(1); // 'nil'
  return {{ std::make_unique<literal::nil>( ), tokens }};
}

parse_res<> parse_regex(token_string tokens)
{
  if (!tokens.size() || tokens.front().which != token::type::regex)
    return {};
  // inc/decrement to remove quotes
  std::string val{tokens.front().str};
  tokens = tokens.subvec(1); // val
  return {{ std::make_unique<literal::regex>( val ), tokens }};
}

parse_res<> parse_string(token_string tokens)
{
  if (!tokens.size() || tokens.front().which != token::type::string)
    return {};
  // inc/decrement to remove quotes
  std::string val{++begin(tokens.front().str), --end(tokens.front().str)};
  tokens = tokens.subvec(1); // val
  return {{ std::make_unique<literal::string>( val ), tokens }};
}

parse_res<> parse_symbol(token_string tokens)
{
  if (!tokens.size() || tokens.front().which != token::type::symbol)
    return {};
  symbol name{tokens[0].str};
  tokens = tokens.subvec(1); // symbol
  return {{ std::make_unique<literal::symbol>( name ), tokens }};
}

// }}}
// Helpers {{{

template <typename F>
auto parse_comma_separated_list(token_string tokens,
                               const F& parse_item)
    -> parse_res<std::vector<decltype(parse_item(tokens)->first)>>
{
  tokens = ltrim_if(tokens, newline_test);

  std::vector<decltype(parse_item(tokens)->first)> items;
  decltype(parse_item(tokens)) item_res = parse_item(tokens);

  while (item_res) {
    items.push_back(std::move(item_res->first));
    tokens = item_res->second;
    if (!tokens.size() || tokens.front().which != token::type::comma)
      return {{ move(items), tokens }};
    tokens = ltrim_if(tokens.subvec(1), newline_test); // ','
    item_res = parse_item(tokens);
  }
  return {{ move(items), tokens }};
}

template <typename F>
auto parse_bracketed_subexpr(token_string tokens,
                             const F& parse_item,
                             token::type opening,
                             token::type)
    -> decltype(parse_item(tokens))
{
  if (!tokens.size() || tokens.front().which != opening)
    return {};
  tokens = ltrim_if(tokens.subvec(1), newline_test); // opening
  auto res = parse_item(tokens);
  res->second = ltrim_if(res->second, newline_test).subvec(1); // closing
  return res;
}

parse_res<arg_t> parse_function_call(token_string tokens)
{
  return parse_bracketed_subexpr(tokens, [](auto t)
  {
    return parse_comma_separated_list(t, parse_expression);
  }, token::type::open_paren, token::type::close_paren);
}

parse_res<std::pair<std::unique_ptr<expression>, std::unique_ptr<expression>>>
  parse_cond_pair(token_string tokens)
{
  auto test_res = parse_expression(tokens);
  if (!test_res)
    return {};
  auto test = move(test_res->first);
  tokens = test_res->second.subvec(1); // ':'

  auto body_res = parse_expression(tokens);
  auto body = move(body_res->first);
  tokens = body_res->second;

  return {{ make_pair(move(test), move(body)), tokens }};
}

// Syntactically identical to (named) function definitions, but the returned
// result is different
parse_res<std::pair<symbol, function_definition>>
  parse_method_definition(token_string tokens)
{
  if (!tokens.size() || tokens.front().which != token::type::key_fn)
    return {};
  tokens = tokens.subvec(1); // 'fn'

  symbol name{tokens.front().str};
  tokens = tokens.subvec(1); // name

  auto arg_res = parse_bracketed_subexpr(tokens, [](auto tokens)
  {
    return parse_comma_separated_list(tokens, [](auto t)
    {
      if (t.front().which == token::type::close_paren)
        return parse_res<symbol>{};
      return parse_res<symbol>{{ symbol{t.front().str}, t.subvec(1) }}; // argname
    });
  }, token::type::open_paren, token::type::close_paren);
  auto args = move(arg_res->first);
  tokens = arg_res->second;

  auto body_res = parse_expression(tokens.subvec(1)); // ':'
  auto body = move(body_res->first);
  tokens = body_res->second;

  return {{ std::make_pair( name,function_definition{ {}, move(body), args} ),
            tokens}};
}

// }}}

// }}}

}

std::vector<std::unique_ptr<expression>> parser::parse(token_string tokens)
{
  std::vector<std::unique_ptr<expression>> expressions;
  tokens = ltrim_if(tokens, trim_test);

  while (tokens.size()) {
    auto res = parse_expression(tokens);
    expressions.push_back(move(res->first));
    tokens = res->second;
    tokens = ltrim_if(res->second, trim_test);
  }
  return expressions;
}

// }}}
