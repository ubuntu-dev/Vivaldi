#include "builtins.h"
#include "gc.h"
#include "get_file_contents.h"
#include "parser.h"
#include "vm.h"
#include "value/array.h"
#include "value/builtin_function.h"
#include "value/nil.h"
#include "value/string.h"

#include <iostream>
#include <sstream>

void write_error(const std::string& error)
{
  std::cerr << "\033[1;31m" << error << "\033[22;39m\n";
}

void repl_catcher(vv::vm::machine& vm)
{
  write_error("caught exception: " + vm.top()->value());
  vm.pop(1);
  vm.pnil();
}

std::vector<std::unique_ptr<vv::ast::expression>> get_valid_line()
{
  std::cout << ">>> ";

  std::vector<vv::parser::token> tokens;
  vv::parser::val_res validator;

  while (!std::cin.eof()) {
    std::string line;
    getline(std::cin, line);
    std::istringstream linestream{line};

    auto new_tokens = vv::parser::tokenize(linestream);
    copy(begin(new_tokens), end(new_tokens), back_inserter(tokens));
    validator = vv::parser::is_valid(tokens);
    if (validator.valid())
      break;

    if (validator.invalid() && validator->size()) {
      std::string error{"invalid syntax"};
      if (validator.invalid()) {
        error += " at "
              + (validator->front().which == vv::parser::token::type::newline
                  ? "end of line: "
                  : '\'' + validator->front().str + "': ")
              + validator.error();
      }
      write_error(error);
      tokens.clear();
      std::cout << ">>> ";
    } else {
      std::cout << "... ";
    }
  }

  return vv::parser::parse(tokens);
}

void run_repl()
{
  vv::gc::init();

  auto env = vv::gc::alloc<vv::vm::environment>( );
  vv::builtin::make_base_env(*env);

  while (!std::cin.eof()) {
    for (const auto& expr : get_valid_line()) {
      auto body = expr->code();
      vv::vm::call_frame frame{body, env};
      vv::vm::machine machine{std::move(frame), repl_catcher};
      machine.run();
      std::cout << "=> " << machine.top()->value() << '\n';
    }
  }
  std::cout << '\n'; // stick prompt on newline on ^D
}

int main(int argc, char** argv)
{
  vv::gc::init();

  if (argc == 1) {
    run_repl();

  } else {
    auto tok_res = vv::get_file_contents(argv[1]);
    if (!tok_res.successful()) {
      std::cerr << tok_res.error() << '\n';
      return 64; // bad usage
    }

    auto excepted = false;

    //auto base_frame = vv::gc::alloc<vv::vm::call_frame>( tok_res.result() );
    //auto frame = static_cast<vv::vm::call_frame*>(base_frame);
    auto env = vv::gc::alloc<vv::vm::environment>( );
    vv::builtin::make_base_env(*env);
    vv::vm::call_frame frame{tok_res.result(), env};

    vv::vm::machine vm{std::move(frame), [&](auto&){ excepted = true; }};

    vv::builtin::make_base_env(*env);
    auto arg_array = vv::gc::alloc<vv::value::array>( );
    env->members[{"argv"}] = arg_array;

    auto cast_argv = static_cast<vv::value::array*>( arg_array );
    transform(argv + 2, argv + argc, back_inserter(cast_argv->val),
              vv::gc::alloc<vv::value::string, std::string>);

    vm.run();

    if (excepted)
      std::cerr << "Caught exception: " << vm.top()->value() << '\n';

    return excepted ? 65 : 0; // data err
  }
}
