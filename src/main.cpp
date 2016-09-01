#include "builtins.h"
#include "gc.h"
#include "get_file_contents.h"
#include "messages.h"
#include "opt.h"
#include "repl.h"
#include "vm.h"
#include "gc/alloc.h"
#include "utils/error.h"
#include "value/array.h"
#include "value/string.h"

#include <iostream>

int main(int argc, char** argv)
{
  vv::builtin::init();
  // Run REPL if run with no arguments; otherwise, run Vivaldi file
  if (argc == 1) {
    vv::run_repl();
  }
  else {
    // Try to parse file; if unsuccessful, exit with status 64
    auto tok_res = vv::get_file_contents(argv[1]);
    if (!tok_res.successful()) {
      std::cerr << tok_res.error() << '\n';
      return 64; // bad usage
    }

    // Perform various optimizations on main code body that we couldn't perform
    // if we didn't know it was the main body (e.g. eliminate unused variables)
    vv::optimize_independent_block(tok_res.result());

    // Populate base environment with builtin classes and functions
    const auto env = vv::gc::alloc<vv::vm::environment>( );
    vv::builtin::make_base_env(env);
    vv::vm::call_frame frame{tok_res.result(), env};

    // Initiate VM
    vv::vm::machine vm{std::move(frame)};

    // Add argument 'argv' to the base environment. Done in this weird order for
    // GC reasons (not *really* necessary, since GC won't be triggered until 1Kb
    // or so objects have been allocated, but this is probably more future-proof
    // or something. Besides, you never know; someone might have passed a *lot*
    // of arguments...)
    const auto arg_array = vv::gc::alloc<vv::value::array>( );
    vv::value::get<vv::vm::environment>(env).members[{"argv"}] = arg_array;

    // Fill 'argv' with command-line arguments, chopping off 'vivaldi' and the
    // filename
    transform(argv + 2, argv + argc,
              back_inserter(vv::value::get<vv::value::array>(arg_array)),
              vv::gc::alloc<vv::value::string, std::string>);

    // Actually run the VM; if an uncaught Vivaldi exception is thrown, print
    // the error to stderr and exit with status 65
    try {
      vm.run();
    } catch (vv::vm_error& err) {
      std::cerr << vv::message::caught_exception(err.error()) << '\n';
      return 65; // data err
    }
  }
}
