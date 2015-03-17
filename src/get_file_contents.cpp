#include "get_file_contents.h"

#include "builtins.h"
#include "gc.h"
#include "parser.h"
#include "utils/dynamic_library.h"
#include "value/string.h"
#include "vm.h"

#include <boost/filesystem.hpp>

#include <dlfcn.h>

#include <fstream>
#include <sstream>

using namespace vv;
using namespace parser;

namespace {

// Provide a pretty error message for syntax errors
std::string message_for(token_string tokens,
                        const std::string& filename,
                        val_res validator)
{
  // Check if we properly detected the specific syntax error
  if (validator.invalid()) {
    // O(n), but unless I radically refactor tokenizing, it'll have to do; if
    // this is ever reached the program's basically guaranteed to terminate
    // (unless someone wraps a 'require' in a try...catch block, which seems to
    // unlikely), so added runtime isn't too big a concern in any case
    auto line = std::count_if(tokens.begin(), validator->begin(),
                              [](const auto& i)
                                { return i.which == token::type::newline; });

    std::ostringstream str;
    str << "Invalid syntax at ";
    // print specific token, but don't try to print past end of vector_ref!
    if (validator->size() == 0)
      str <<  "end of input";
    else
      str << '\'' << validator->front().str << '\'';
    str << " in " << filename << " on line " << std::to_string(line + 1) << ": "
        << validator.error();

    return str.str();
  }
  // Generic message, in case the validator failed to detect a specific syntax
  // error; hopefully never reached, but not all syntactic edge cases are taken
  // care of at the moment
  return "Invalid syntax in " + filename;
}

}

std::string vv::get_real_filename(const std::string& filename, const std::string& path)
{
  boost::string_ref ref{filename};
  if (ref.ends_with(".vv") || ref.ends_with(".so") || ref.ends_with(".dylib"))
    return filename;

  auto check_vv = absolute(boost::filesystem::path{filename + ".vv"}, path);
  if (exists(check_vv))
    return check_vv.native();

  auto check_so = absolute(boost::filesystem::path{filename + ".so"}, path);
  if (exists(check_so))
    return check_so.native();

  auto check_dylib = absolute(boost::filesystem::path{filename + ".dylib"}, path);
  if (exists(check_dylib))
    return check_dylib.native();

  return filename;
}

bool vv::is_c_exension(const std::string& filename)
{
  boost::string_ref ref{filename};
  return ref.ends_with(".so") || ref.ends_with(".dylib");
}

read_file_result vv::get_file_contents(const std::string& filename,
                                       const std::string& cur_path)
{
  auto real_filename = absolute(boost::filesystem::path{filename}, cur_path);
  std::ifstream file{real_filename.native()};
  if (!file)
    return { "", '"' + filename + "\": file not found" };

  auto path = boost::filesystem::path{filename}.parent_path();
  // Search in current search path, NOT cwd
  path = absolute(path, cur_path);

  auto tokens = tokenize(file);
  auto validator = is_valid(tokens);
  if (!validator)
    return { path.native(), message_for(tokens, filename, validator) };

  auto exprs = parse(tokens);
  std::vector<vm::command> body;
  // set working directory to path of file, so nested 'require's don't bork
  body.emplace_back(vm::instruction::chreqp, path.native());
  body.emplace_back(vm::instruction::pnil); // HACK--- for pops below
  for (const auto& i : exprs) {
    auto code = i->code();
    body.emplace_back(vm::instruction::pop, 1);
    copy(begin(code), end(code), back_inserter(body));
  }
  return { path.native(), move(body) };
}

boost::optional<std::string> vv::read_c_lib(const std::string& filename)
{
  try {
    auto& dylib = gc::load_dynamic_library(filename);
    auto init = dylib.get_fn<void()>("vv_init_lib");

    init();

  } catch (const dylib_error& err) {
    return std::string{err.what()};
  }
  return {};
}
