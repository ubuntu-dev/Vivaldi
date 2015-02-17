#include "run_file.h"

#include "builtins.h"
#include "gc.h"
#include "parser.h"
#include "value/string.h"
#include "vm.h"

#include <boost/filesystem.hpp>

#include <fstream>

using namespace vv;

namespace {

std::string message_for(parser::token_string tokens,
                        const std::string& filename,
                        vv::parser::val_res validator)
{
  static auto line_test = [](auto i) { return i.which == parser::token::type::newline; };
  if (validator.invalid()) {
    auto line = std::count_if(tokens.begin(), validator->begin(), line_test);
    auto token = validator->size() == 0 ? "end of input"
                                        : '\'' + validator->front().str + '\'';

    return "Invalid syntax at " + token
           + " in " + filename + " on line " + std::to_string(line + 1) + ": "
           + validator.error();
  }

  return "Invalid syntax in " + filename;
}

}

vv::run_file_result vv::run_file(const std::string& filename)
{
  std::ifstream file{filename};
  if (!file)
    return { run_file_result::result::file_not_found,
             gc::alloc<value::string>( '"' + filename + "\": file not found" ),
             {} };

  auto tokens = parser::tokenize(file);
  auto validator = parser::is_valid(tokens);
  if (!validator)
    return { run_file_result::result::failure,
             gc::alloc<value::string>(message_for(tokens, filename, validator)),
             {} };

  auto exprs = parser::parse(tokens);
  std::vector<vm::command> body;
  for (const auto& i : exprs) {
    auto code = i->generate();
    copy(begin(code), end(code), back_inserter(body));
  }

  // set working directory to path of file
  auto pwd = boost::filesystem::current_path();
  boost::filesystem::path path{filename};
  if (path.has_parent_path())
    boost::filesystem::current_path(path.parent_path());

  // Set up base env
  auto vm_base = std::make_shared<vm::call_frame>(nullptr,
                                                  nullptr,
                                                  0,
                                                  body);
  builtin::make_base_env(*vm_base);

  auto excepted = false;
  vm::machine machine{vm_base, [&](vm::machine&) { excepted = true; }};
  machine.run();

  // reset working directory
  boost::filesystem::current_path(pwd);

  if (excepted)
    return { run_file_result::result::failure, machine.retval, {} };

  return { run_file_result::result::success,
           gc::alloc<value::boolean>( true ),
           machine.frame->local.front() };
}
