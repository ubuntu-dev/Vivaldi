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
  write_error("caught exception: " + vm.retval->value());

  // Clear out remaining instructions once the current line's borked
  auto remaining = vm.frame->instr_ptr.size();
  vm.frame->instr_ptr = vm.frame->instr_ptr.subvec(remaining);
  vm.retval = vv::gc::alloc<vv::value::nil>( );
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

  auto base_frame = std::make_shared<vv::vm::call_frame>( );
  vv::builtin::make_base_env(*base_frame);

  while (!std::cin.eof()) {
    for (const auto& expr : get_valid_line()) {
      auto body = expr->generate();
      base_frame->instr_ptr = vv::vector_ref<vv::vm::command>{body};
      vv::vm::machine machine{base_frame, repl_catcher};
      machine.run();
      std::cout << "=> " << machine.retval->value() << '\n';
    }
  }
  std::cout << '\n'; // stick prompt on newline on ^D
}

int main(int argc, char** argv)
{
  vv::gc::init();

  if (argc == 1) {
    run_repl();
    vv::gc::empty();

  } else {
    auto tok_res = vv::get_file_contents(argv[1]);
    if (!tok_res.successful()) {
      std::cerr << tok_res.error() << '\n';
      vv::gc::empty();
      return 64; // bad usage
    }

    auto base_frame = std::make_shared<vv::vm::call_frame>( tok_res.result() );
    vv::gc::set_current_frame(base_frame);

    vv::builtin::make_base_env(*base_frame);
    auto arg_array = vv::gc::alloc<vv::value::array>( );
    base_frame->local.back()[{"argv"}] = arg_array;

    auto cast_argv = static_cast<vv::value::array*>( arg_array );
    transform(argv + 2, argv + argc, back_inserter(cast_argv->val),
              vv::gc::alloc<vv::value::string, std::string>);

    auto excepted = false;
    vv::vm::machine vm{base_frame, [&](auto&){ excepted = true; }};

    vm.run();

    if (excepted)
      std::cerr << "Caught exception: " << vm.retval->value() << '\n';

    vv::gc::empty();
    return excepted ? 65 : 0; // data err
  }
}
