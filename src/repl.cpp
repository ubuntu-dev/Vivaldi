#include "repl.h"

#include "builtins.h"
#include "gc.h"
#include "messages.h"
#include "parser.h"
#include "utils/error.h"
#include "utils/lang.h"

#include <iostream>
#include <sstream>

using namespace vv;

namespace {

// Colorize error message. TODO: detect terminal properties and adjust
// accordingly
void write_error(const std::string& error)
{
  std::cerr << "\033[1;31m" << error << "\033[22;39m\n";
}

}

// Get valid input, ignoring blank lines, concatenating incomplete lines, and
// rejecting invalid lines
std::vector<std::unique_ptr<ast::expression>> vv::get_valid_line()
{
  std::cout << ">>> ";

  std::vector<parser::token> tokens;
  parser::val_res validator;

  while (!std::cin.eof()) {
    std::string line;
    getline(std::cin, line);

    // Tokenizer takes an istream, so make one out of the current line (we can't
    // just feed it std::cin, because of prompts and stuff).
    std::istringstream linestream{line};
    // Tokenize and validate
    auto new_tokens = parser::tokenize(linestream);
    copy(begin(new_tokens), end(new_tokens), back_inserter(tokens));
    validator = parser::is_valid(tokens);
    // If there were no validation errors, we've grabbed a complete expression.
    if (validator.valid())
      break;

    // Slightly hacky--- if the iterator reached an invalid state at the last
    // token, it's not really 'invalid' (just incomplete), so instead of
    // printing an error, keep the current tokens and just grab some more until
    // we've got a complete expression (or a genuinely invalid one).
    if (validator.invalid() && validator->size()) {
      // Genuine error--- print and clear invalid tokens
      std::ostringstream error;
      error << "Invalid syntax";
      if (validator.invalid()) {
        error << " at ";
        if (validator->front().which == parser::token::type::newline)
          error << "end of line: ";
        else
          error << '\'' << validator->front().str << "': ";
        error << validator.error();
      }
      write_error(error.str());
      tokens.clear();
      // caller is expecting a valid expression, so keep prompting until we get
      // one
      std::cout << ">>> ";
    }
    else {
      // Incomplete expression--- prompt for more
      std::cout << "... ";
    }
  }

  return parser::parse(tokens);
}

void vv::run_repl()
{
  gc::init();

  auto env = gc::alloc<vm::environment>( );
  builtin::make_base_env(*env);

  while (!std::cin.eof()) {
    for (const auto& expr : get_valid_line()) {
      auto body = expr->code();
      vm::call_frame frame{body, env};
      vm::machine machine{std::move(frame)};
      try {
        machine.run();
        std::cout << "=> " << pretty_print(*machine.top(), machine) << '\n';
      } catch (const vm_error& err) {
        write_error(message::caught_exception(*err.error()));
      }
    }
  }
  std::cout << '\n'; // stick prompt on newline on ^D
}
