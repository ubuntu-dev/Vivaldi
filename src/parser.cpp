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
#include "ast/method.h"
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

// XXX: A lot of this code will look *really* wonky if you're following along
// according to the grammar. This is because all of it is assuming that whatever
// it's parsing is valid code (since otherwise it wouldn't make it through the
// validator), and takes shortcuts all over the place. Look in validator.cpp for
// code that reflects the grammar more closely.

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
parse_res<> parse_throw(token_string tokens);
parse_res<> parse_for_loop(token_string tokens);
parse_res<> parse_function_definition(token_string tokens);
parse_res<> parse_lambda(token_string tokens);
parse_res<> parse_literal(token_string tokens);
parse_res<> parse_member(token_string tokens);
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
parse_res<> parse_char(token_string tokens);
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

parse_res<std::pair<std::vector<symbol>, boost::optional<symbol>>>
  parse_arglist(token_string tokens);

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

  while (!tokens.empty() && test(tokens.front().which)) {
    auto left = move(left_res->first);

    const symbol name{convert(tokens.front().which)};

    std::unique_ptr<ast::expression> right;
    const auto right_tokens = ltrim_if(tokens.subvec(1), newline_test); // binop
    tie(right, tokens) = *lower_pred(right_tokens);

    auto method = std::make_unique<ast::method>( move(left), name );
    arg_t arg{};
    arg.emplace_back(move(right));

    left_res = {{std::make_unique<function_call>(move(method), move(arg)), tokens}};
  }
  return left_res;
}

// numbers 13 and 12 can't be done as parse_operator_expr calls because && and
// || aren't method calls, and have a AST type

parse_res<> parse_prec13(token_string tokens)
{
  auto left_res = parse_prec12(tokens);
  if (!left_res)
    return left_res;
  tokens = left_res->second;

  while (!tokens.empty() && tokens.front().which == token::type::or_sign) {
    auto left = move(left_res->first);

    std::unique_ptr<ast::expression> right;
    const auto right_tokens = ltrim_if(tokens.subvec(1), newline_test); // '||'
    tie(right, tokens) = *parse_prec13(right_tokens);

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

  while (!tokens.empty() && tokens.front().which == token::type::and_sign) {
    auto left = move(left_res->first);
    std::unique_ptr<ast::expression> right;
    const auto right_tokens = ltrim_if(tokens.subvec(1), newline_test); // '&&'
    tie(right, tokens) = *parse_prec12(right_tokens);

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

  while (!tokens.empty() && tokens.front().which == token::type::to) {
    auto left = move(left_res->first);
    std::unique_ptr<ast::expression> right;
    const auto right_tokens = ltrim_if(tokens.subvec(1), newline_test); // 'to'
    tie(right, tokens) = *parse_prec9(right_tokens);

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
  if (!tokens.empty() && (tokens.front().which == token::type::bang
                      || tokens.front().which == token::type::tilde
                      || tokens.front().which == token::type::dash)) {


    symbol name{(tokens.front().which == token::type::bang)  ? "not" :
                (tokens.front().which == token::type::tilde) ? "negate" :
                                                               "negative"};
    std::unique_ptr<ast::expression> expr;
    tie(expr, tokens) = *parse_prec1(tokens.subvec(1)); // monop
    auto method = std::make_unique<ast::method>( move(expr), name );

    return {{ std::make_unique<function_call>( move(method), arg_t{} ),
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
  while (!tokens.empty() && (tokens.front().which == token::type::open_paren
                          || tokens.front().which == token::type::dot
                          || tokens.front().which == token::type::arrow
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

      if (!tokens.empty() && tokens.front().which == token::type::assignment) {
        std::unique_ptr<ast::expression> value;
        tie(value, tokens) = *parse_expression(tokens.subvec(1)); // '='

        auto method = std::make_unique<ast::method>( move(expr), symbol{"set_at"} );
        arg_t args{};
        args.emplace_back(move(idx));
        args.emplace_back(move(value));

        return {{ std::make_unique<function_call>( move(method), move(args) ),
                  tokens }};
      }

      auto method = std::make_unique<ast::method>( move(expr), symbol{"at"} );
      arg_t arg{};
      arg.emplace_back(move(idx));
      expr = std::make_unique<function_call>(move(method), move(arg));
    }
    else if (tokens.front().which == token::type::arrow) {
      tokens = ltrim_if(tokens.subvec(1), newline_test); // '->'
      std::unique_ptr<ast::expression> bound_fn;
      tie(bound_fn, tokens) = *parse_nonop_expression(tokens);

      auto method = std::make_unique<ast::method>( move(bound_fn), symbol{"bind"} );
      arg_t args{};
      args.emplace_back(move(expr));
      expr = std::make_unique<function_call>( move(method), move(args) );
    }
    else {
      tokens = ltrim_if(tokens.subvec(1), newline_test); // '.'
      symbol name{tokens.front().str};
      tokens = tokens.subvec(1); // name
      expr = std::make_unique<method>( move(expr), name );
    }
  }
  return {{ move(expr), tokens }};
}

parse_res<> parse_nonop_expression(token_string tokens)
{
  if (!tokens.empty() && tokens.front().which == token::type::open_paren) {
    return parse_bracketed_subexpr(tokens,
                                   parse_expression,
                                   token::type::open_paren,
                                   token::type::close_paren);
  }

  parse_res<> res;
  if ((res = parse_array_literal(tokens)))        return res;
  if ((res = parse_member(tokens)))               return res;
  if ((res = parse_assignment(tokens)))           return res;
  if ((res = parse_block(tokens)))                return res;
  if ((res = parse_cond_statement(tokens)))       return res;
  if ((res = parse_dict_literal(tokens)))         return res;
  if ((res = parse_for_loop(tokens)))             return res;
  if ((res = parse_function_definition(tokens)))  return res;
  if ((res = parse_lambda(tokens)))               return res;
  if ((res = parse_literal(tokens)))              return res;
  if ((res = parse_new_obj(tokens)))              return res;
  if ((res = parse_require(tokens)))              return res;
  if ((res = parse_return(tokens)))               return res;
  if ((res = parse_throw(tokens)))                return res;
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
  const symbol name{tokens.front().str};
  std::unique_ptr<ast::expression> expr;
  tie(expr, tokens) = *parse_expression(tokens.subvec(2)); // name '='
  return {{ std::make_unique<assignment>( name, move(expr) ), tokens }};
}

parse_res<> parse_block(token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_do)
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
  if (tokens.empty() || tokens.front().which != token::type::open_bracket)
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
  if (tokens.empty() || tokens.front().which != token::type::open_brace)
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
  if (tokens.empty() || tokens.front().which != token::type::key_cond)
    return {};
  tokens = ltrim_if(tokens.subvec(1), newline_test);

  auto pairs_res = parse_comma_separated_list(tokens, parse_cond_pair);
  auto pairs = move(pairs_res->first);
  tokens = pairs_res->second;

  return {{ std::make_unique<cond_statement>(move(pairs)), tokens }};
}

parse_res<> parse_for_loop(token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_for)
    return {};
  tokens = tokens.subvec(1); // 'for'

  const symbol iterator{tokens.front().str};
  tokens = tokens.subvec(2); // iterator 'in'

  std::unique_ptr<ast::expression> range;
  tie(range, tokens) = *parse_expression(tokens);

  std::unique_ptr<ast::expression> body;
  tie(body, tokens) = *parse_expression(ltrim_if(tokens.subvec(1), newline_test)); // ':'

  return {{ std::make_unique<for_loop>( iterator, move(range), move(body) ),
            tokens }};
}

parse_res<> parse_function_definition(token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_let)
    return {};
  const symbol name{tokens[1].str};
  tokens = tokens.subvec(2); // let name

  if (tokens.empty() || tokens.front().which != token::type::open_paren)
    return {};

  auto arg_res = parse_bracketed_subexpr(tokens,
                                         parse_arglist,
                                         token::type::open_paren,
                                         token::type::close_paren);
  auto args = move(arg_res->first.first);
  auto vararg = arg_res->first.second; // just a symbol, no need for move
  tokens = arg_res->second;

  std::unique_ptr<ast::expression> body;
  tie(body, tokens) = *parse_expression(ltrim_if(tokens.subvec(1), newline_test)); // '='

  return {{ std::make_unique<function_definition>( name, move(body), args, vararg ),
            tokens }};
}

parse_res<> parse_lambda(token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_fn)
    return {};
  tokens = tokens.subvec(1); // 'fn'

  if (tokens.empty() || tokens.front().which != token::type::open_paren)
    return {};

  auto arg_res = parse_bracketed_subexpr(tokens,
                                         parse_arglist,
                                         token::type::open_paren,
                                         token::type::close_paren);
  auto args = move(arg_res->first.first);
  auto vararg = arg_res->first.second; // just a symbol, no need for move
  tokens = arg_res->second;

  std::unique_ptr<ast::expression> body;
  tie(body, tokens) = *parse_expression(ltrim_if(tokens.subvec(1), newline_test)); // ':'

  return {{ std::make_unique<function_definition>( symbol{}, move(body), args, vararg ),
            tokens }};
}

parse_res<> parse_literal(token_string tokens)
{
  parse_res<> res;
  if ((res = parse_float(tokens)))   return res;
  if ((res = parse_integer(tokens))) return res;
  if ((res = parse_bool(tokens)))    return res;
  if ((res = parse_char(tokens)))    return res;
  if ((res = parse_nil(tokens)))     return res;
  if ((res = parse_regex(tokens)))   return res;
  if ((res = parse_string(tokens)))  return res;
  if ((res = parse_symbol(tokens)))  return res;
  return res;
}

parse_res<> parse_member(token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::member)
    return {};
  const symbol mem_name{tokens.front().str};
  tokens = tokens.subvec(1); // member
  if (!tokens.empty() && tokens.front().which == token::type::assignment) {
    tokens = tokens.subvec(1); // '='
    auto res = parse_expression(tokens);
    return {{ std::make_unique<member_assignment>( mem_name, std::move(res->first) ),
              res->second }};
  }
  return {{ std::make_unique<member>( mem_name ), tokens }};
}

parse_res<> parse_new_obj(token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_new)
    return {};
  tokens = tokens.subvec(1); // 'new'

  std::unique_ptr<ast::expression> type;
  tie(type, tokens) = *parse_nonop_expression(tokens);

  auto args_res = parse_function_call(tokens);
  auto args = move(args_res->first);
  tokens = args_res->second;

  return {{ std::make_unique<object_creation>( move(type), move(args) ),
            tokens }};
}

parse_res<> parse_require(token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_require)
    return {};
  tokens = tokens.subvec(1); // 'require'
  std::string filename{++begin(tokens.front().str), --end(tokens.front().str)};
  tokens = tokens.subvec(1); // filename
  return {{ std::make_unique<require>( filename ), tokens }};
}

parse_res<> parse_return(token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_return)
    return {};

  std::unique_ptr<ast::expression> expr;
  tie(expr, tokens) = *parse_expression(tokens.subvec(1)); // 'return'
  return {{ std::make_unique<return_statement>( move(expr) ), tokens }};
}

parse_res<> parse_throw(token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_throw)
    return {};
  std::unique_ptr<ast::expression> expr;
  tie(expr, tokens) = *parse_expression(tokens.subvec(1)); // 'throw'
  return {{ std::make_unique<except>( move(expr) ), tokens }};
}

parse_res<> parse_try_catch(token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_try)
    return {};
  tokens = ltrim_if(tokens.subvec(2), newline_test); // 'try' ':'

  std::unique_ptr<ast::expression> body;
  tie(body, tokens) = *parse_expression(tokens);

  tokens = ltrim_if(tokens, newline_test);
  tokens = tokens.subvec(1); // 'catch'

  auto catch_bodies_res = parse_comma_separated_list(tokens, [](const auto str)
  {
    if (str.size() < 4
        || str[0].which != token::type::name
        || str[1].which != token::type::name
        || str[2].which != token::type::colon) {
      return parse_res<ast::catch_stmt>{};
    }
    symbol type{str[0].str};
    symbol name{str[1].str};

    const auto expr_str = ltrim_if(str.subvec(3), trim_test); // type name ':'
    auto expr = parse_expression(expr_str);

    ast::catch_stmt stmt{name, type, std::move(expr->first)};
    return parse_res<ast::catch_stmt>{{ std::move(stmt), expr->second }};
  });

  auto catch_bodies = std::move(catch_bodies_res->first);

  return {{ std::make_unique<ast::try_catch>( std::move(body),
                                              std::move(catch_bodies) ),
            catch_bodies_res->second }};
}

parse_res<> parse_type_definition(token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_class)
    return{};
  const symbol name{tokens[1].str};
  tokens = tokens.subvec(2); // 'class' name

  symbol parent{"Object"};
  if (tokens.front().which == token::type::colon) {
    parent = std::string_view{tokens[1].str};
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
  if (tokens.empty() || tokens.front().which != token::type::key_let)
    return {};

  const symbol name{tokens[1].str};

  std::unique_ptr<ast::expression> expr;
  tie(expr, tokens) = *parse_expression(tokens.subvec(3)); // 'let' name '='

  return {{ std::make_unique<variable_declaration>(name, move(expr)), tokens }};
}

parse_res<> parse_variable(token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::name)
    return {};
  const symbol name{tokens.front().str};
  tokens = tokens.subvec(1); // name
  return {{ std::make_unique<variable>(name), tokens }};
}

parse_res<> parse_while_loop(token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_while)
    return {};

  std::unique_ptr<ast::expression> test;
  tie(test, tokens) = *parse_expression(tokens.subvec(1)); // 'while'

  std::unique_ptr<ast::expression> body;
  tie(body, tokens) = *parse_expression(ltrim_if(tokens.subvec(1), newline_test)); // ':'

  return {{ std::make_unique<while_loop>( move(test), move(body) ), tokens }};
}

// }}}
// Literals {{{

parse_res<> parse_integer(token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::integer)
    return {};
  auto str = tokens.front().str;
  tokens = tokens.subvec(1); // number

  return {{ std::make_unique<literal::integer>( to_int(str) ), tokens }};
}

parse_res<> parse_float(token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::floating_point)
    return {};
  auto str = tokens.front().str;
  tokens = tokens.subvec(1); // number

  return {{ std::make_unique<literal::floating_point>( stod(str) ), tokens }};
}

parse_res<> parse_bool(token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::boolean)
    return {};
  const bool value{tokens.front().str == "true"};
  tokens = tokens.subvec(1); // value
  return {{ std::make_unique<literal::boolean>( value ), tokens }};
}

parse_res<> parse_char(const token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::character)
    return {};

  return {{ std::make_unique<literal::character>( tokens.front().str.front() ),
            tokens.subvec(1) }};
}

parse_res<> parse_nil(token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::nil)
    return {};
  tokens = tokens.subvec(1); // 'nil'
  return {{ std::make_unique<literal::nil>( ), tokens }};
}

parse_res<> parse_regex(token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::regex)
    return {};
  const std::string val{tokens.front().str};
  tokens = tokens.subvec(1); // val
  return {{ std::make_unique<literal::regex>( val ), tokens }};
}

parse_res<> parse_string(token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::string)
    return {};
  // inc/decrement to remove quotes
  const std::string val{++begin(tokens.front().str), --end(tokens.front().str)};
  tokens = tokens.subvec(1); // val
  return {{ std::make_unique<literal::string>( val ), tokens }};
}

parse_res<> parse_symbol(token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::symbol)
    return {};
  const symbol name{tokens[0].str};
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
    if (tokens.empty() || tokens.front().which != token::type::comma)
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
  if (tokens.empty() || tokens.front().which != opening)
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
  tokens = ltrim_if(test_res->second.subvec(1), newline_test); // ':'

  std::unique_ptr<ast::expression> body;
  tie(body, tokens) = *parse_expression(tokens);

  return {{ make_pair(move(test), move(body)), tokens }};
}

parse_res<std::pair<symbol, function_definition>>
  parse_method_definition(token_string tokens)
{
  if (tokens.empty() || tokens.front().which != token::type::key_let)
    return {};

  const symbol name{tokens[1].str};
  tokens = tokens.subvec(2); // 'let' name

  auto arg_res = parse_bracketed_subexpr(tokens,
                                         parse_arglist,
                                         token::type::open_paren,
                                         token::type::close_paren);
  auto args = move(arg_res->first.first);
  auto vararg = arg_res->first.second; // just a symbol, no need for move
  tokens = arg_res->second;

  std::unique_ptr<ast::expression> body;
  tie(body, tokens) = *parse_expression(ltrim_if(tokens.subvec(1), newline_test)); // '='

  return {{ std::make_pair( name, function_definition{ {}, move(body), args, vararg} ),
            tokens}};
}

parse_res<std::pair<std::vector<symbol>, boost::optional<symbol>>>
  parse_arglist(token_string tokens)
{
  if (tokens.empty())
    return {};

  std::vector<symbol> args;
  while (tokens.front().which == token::type::name) {
    args.push_back(std::string_view{tokens.front().str});
    tokens = ltrim_if(tokens.subvec(1), newline_test); // arg
    if (tokens.empty() || tokens.front().which != token::type::comma)
      return {{ {move(args), {}}, tokens }};
    tokens = ltrim_if(tokens.subvec(1), newline_test); // ','
  }
  boost::optional<symbol> vararg;
  if (!tokens.empty() && tokens.front().which == token::type::open_bracket) {
    tokens = ltrim_if(tokens.subvec(1), newline_test); // '['
    vararg = std::string_view{tokens.front().str};
    tokens = ltrim_if(tokens.subvec(1), newline_test); // 'argname'
    tokens = ltrim_if(tokens.subvec(1), newline_test); // ']'
  }
  return {{ {move(args), vararg}, tokens }};
}

// }}}

// }}}

}

std::vector<std::unique_ptr<expression>> parser::parse(token_string tokens)
{
  std::vector<std::unique_ptr<expression>> expressions;
  tokens = ltrim_if(tokens, trim_test);

  while (!tokens.empty()) {
    expressions.emplace_back();
    tie(expressions.back(), tokens) = *parse_expression(tokens);
    tokens = ltrim_if(tokens, trim_test);
  }
  return expressions;
}

// }}}

