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

void write_error(const std::string& error)
{
  std::cerr << "\033[1;31m" << error << "\033[22;39m\n";
}

}

std::vector<std::unique_ptr<ast::expression>> vv::get_valid_line()
{
  std::cout << ">>> ";

  std::vector<parser::token> tokens;
  parser::val_res validator;

  while (!std::cin.eof()) {
    std::string line;
    getline(std::cin, line);
    std::istringstream linestream{line};

    auto new_tokens = parser::tokenize(linestream);
    copy(begin(new_tokens), end(new_tokens), back_inserter(tokens));
    validator = parser::is_valid(tokens);
    if (validator.valid())
      break;

    if (validator.invalid() && validator->size()) {
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
      std::cout << ">>> ";
    } else {
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
