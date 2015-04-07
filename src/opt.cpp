#include "opt.h"

#include "builtins.h"
#include "vm/instruction.h"

#include <iostream>

using namespace vv;

namespace {

// Helper functions {{{

bool is_eblk(const vm::command& com)
{
  return com.instr == vm::instruction::eblk;
}

bool is_lblk(const vm::command& com)
{
  return com.instr == vm::instruction::lblk;
}

bool affects_env(const vm::command& com)
{
  return com.instr == vm::instruction::let || com.instr == vm::instruction::req;
}

bool uses_local_vars(const vm::command& com)
{
  return com.instr == vm::instruction::let || com.instr == vm::instruction::read
      || com.instr == vm::instruction::write;
}

bool is_opt_fn(const vm::command& com)
{
  if (com.instr != vm::instruction::method)
    return false;
  const auto sym = com.arg.as_sym();
  return sym == builtin::sym::add   || sym == builtin::sym::subtract
      || sym == builtin::sym::times || sym == builtin::sym::divides;
}

bool is_prim_push(const vm::command& com)
{
  switch (com.instr) {
  case vm::instruction::pnil:
  case vm::instruction::pint:
  case vm::instruction::pflt:
  case vm::instruction::pfn:
  case vm::instruction::pbool:
  case vm::instruction::pstr:
  case vm::instruction::psym: return true;
  default: return false;
  }
}

bool is_side_effect_free(const vm::command& com)
{
  switch (com.instr) {
  case vm::instruction::pnil:
  case vm::instruction::pint:
  case vm::instruction::pflt:
  case vm::instruction::pfn:
  case vm::instruction::pbool:
  case vm::instruction::pstr:
  case vm::instruction::psym:
  case vm::instruction::arg:
  case vm::instruction::dup: return true;
  default: return false;
  }
}

bool is_opt(const vm::command& com)
{
  switch (com.instr) {
  case vm::instruction::opt_add:
  case vm::instruction::opt_sub:
  case vm::instruction::opt_mul:
  case vm::instruction::opt_div: return true;
  default: return false;
  }
}

vm::instruction instr_for(symbol sym)
{
  if (sym == builtin::sym::add)
    return vm::instruction::opt_add;
  if (sym == builtin::sym::subtract)
    return vm::instruction::opt_sub;
  if (sym == builtin::sym::times)
    return vm::instruction::opt_mul;
  return vm::instruction::opt_div;
}

bool is_noop(const vm::command& com)
{
  return com.instr == vm::instruction::noop;
}

bool is_cjmp(const vm::command& com)
{
  return com.instr == vm::instruction::jf || com.instr == vm::instruction::jt;
}

bool is_ncjmp(const vm::command& com)
{
  return com.instr == vm::instruction::jmp;
}

bool is_jump(const vm::command& com)
{
  return is_cjmp(com) || is_ncjmp(com);
}

std::vector<size_t> jump_targets(const std::vector<vm::command>& code)
{
  std::vector<size_t> targets;
  for (size_t i{}; i != code.size(); ++i)
    if (is_jump(code[i]))
      targets.push_back(i + 1 + code[i].arg.as_int());
  sort(begin(targets), end(targets));
  return targets;
}

// }}}
// Optimization functions {{{

// Remove unnecessary 'eblk'/'lblk' pairs in which nothing is defined (since
// creating a new environment is potentially expensive)
bool optimize_blocks(std::vector<vm::command>& code)
{
  auto changed = false;

  auto i = find_if(begin(code), end(code), is_eblk);
  for (; i != end(code); i = find_if(i, end(code), is_eblk)) {
    size_t imbalance = 1;
    auto match = i;

    while (imbalance) {
      ++match;
      if (is_eblk(*match))
        ++imbalance;
      else if (is_lblk(*match))
        --imbalance;
    }

    if (!any_of(i, match, affects_env)) {
      changed = true;

      i->instr = vm::instruction::noop;
      match->instr = vm::instruction::noop;
    }
    ++i;
  }
  return changed;
}

// Replace calls to 'add', 'subtract', etc. with optimized instructions
bool optimize_instructions(std::vector<vm::command>& code)
{
  auto changed = false;

  auto i = find_if(begin(code), end(code), is_opt_fn);
  for (; i != end(code); i = find_if(i, end(code), is_opt_fn)) {
    const auto next = find_if_not(i + 1, end(code), is_noop);
    if (next != end(code)) {
      if (next->instr == vm::instruction::call && next->arg.as_int() == 1) {
        changed = true;

        i->instr = instr_for(i->arg.as_sym());
        i->arg = {};
        next->instr = vm::instruction::noop;
        next->arg = {};
      }
    }
    i = next;
  }
  return changed;
}

// Remove 'push literal' / 'pop literal'
bool optimize_noops(std::vector<vm::command>& code)
{
  const auto immut = jump_targets(code);

  auto changed = false;

  auto i = find_if(begin(code), end(code), is_side_effect_free);
  for (; i != end(code); i = find_if(i, end(code), is_side_effect_free)) {
    const auto next = i + 1;
    if (next != end(code) &&
        !binary_search(begin(immut), end(immut), i    - begin(code)) &&
        !binary_search(begin(immut), end(immut), next - begin(code))) {
      if (next->instr == vm::instruction::pop && next->arg.as_int() == 1) {
        changed = true;

        i->instr = vm::instruction::noop;
        i->arg = {};
        next->instr = vm::instruction::noop;
        next->arg = {};
      }
    }
    ++i;
  }
  return changed;
}

// Expression folding--- e.g. convert '1 + 2 * 3 - 5' to just '2'
bool optimize_constants(std::vector<vm::command>& code)
{
  auto changed = false;

  auto i = find_if(begin(code), end(code), is_opt);
  for (; i != end(code); i = find_if(i, end(code), is_opt)) {
    if (i - begin(code) > 1) {
      const auto a2 = i - 2;
      const auto a1 = i - 1;
      if (a2->instr == vm::instruction::pint && a1->instr == vm::instruction::pint) {
        changed = true;

        int result;
        const auto val1 = a1->arg.as_int();
        const auto val2 = a2->arg.as_int();
        switch (i->instr) {
        case vm::instruction::opt_add: result = val1 + val2; break;
        case vm::instruction::opt_sub: result = val1 - val2; break;
        case vm::instruction::opt_mul: result = val1 * val2; break;
        default:                       result = val1 / val2; break;
        }
        i->instr = vm::instruction::pint;
        i->arg = result;
        a1->instr = vm::instruction::noop;
        a2->instr = vm::instruction::noop;
        a1->arg = {};
        a2->arg = {};
      }
    }
    ++i;
  }
  return changed;
}

// If possible, replace named argument access with argument number
bool optimize_args(std::vector<vm::command>& code)
{
  if (any_of(begin(code), end(code), is_jump))
    return false;

  std::unordered_map<vv::symbol, int> arg_nos;

  bool changed = false;

  for (auto i = begin(code); i != end(code); ++i) {
    if (i->instr == vm::instruction::arg) {
      const auto next = std::next(i);
      if (next != end(code)) {
        if (next->instr == vm::instruction::let)
          arg_nos[next->arg.as_sym()] = i->arg.as_int();
        i = next;
      }
    }
    else if (i->instr == vm::instruction::req) {
      return changed;
    }
    else if ((i->instr == vm::instruction::let || i->instr == vm::instruction::write)) {
      arg_nos.erase(i->arg.as_sym());
    }
    else if (i->instr == vm::instruction::read && arg_nos.count(i->arg.as_sym())) {
      i->instr = vm::instruction::arg;
      i->arg = arg_nos[i->arg.as_sym()];
      changed = true;
    }
  }
  return changed;
}

bool optimize_lets(std::vector<vm::command>& code)
{
  if (any_of(begin(code), end(code), [](const auto& c) { return c.instr == vm::instruction::pfn; }))
    return false;

  auto changed = false;
  for (auto i = begin(code); i != end(code); ++i) {
    if (i->instr == vm::instruction::let) {
      const auto used = any_of(std::next(i), end(code), [&](const auto& c)
      {
        return uses_local_vars(c) && c.arg.as_sym() == i->arg.as_sym();
      });
      if (!used) {
        changed = true;
        i->instr = vm::instruction::noop;
      }
    }
  }
  return changed;
}

// Replace conditional jumps (jf, jt) with absolute jump (jmp) if the condition
// is a literal and so known at compile time
bool optimize_cond_jumps(std::vector<vm::command>& code)
{
  auto changed = false;

  const auto immut = jump_targets(code);

  auto i = find_if(++begin(code), end(code), is_cjmp);
  for (; i != end(code); i = find_if(i, end(code), is_cjmp)) {
    const auto condition = i - 1;
    if (is_prim_push(*condition) &&
        !binary_search(begin(immut), end(immut), i         - begin(code)) &&
        !binary_search(begin(immut), end(immut), condition - begin(code))) {

      changed = true;
      bool truthiness;
      switch (condition->instr) {
      case vm::instruction::pnil:  truthiness = false;                    break;
      case vm::instruction::pbool: truthiness = condition->arg.as_bool(); break;
      default:                     truthiness = true;                     break;
      }
      if (i->instr == vm::instruction::jf) {
        i->instr = truthiness ? vm::instruction::noop : vm::instruction::jmp;
      }
      else {
        i->instr = truthiness ? vm::instruction::jmp : vm::instruction::noop;
      }
    }
    ++i;
  }
  return changed;
}

// remove code that's never reached because it's always jumped over (most useful
// in conjunction with optimize_cond_jumps)
bool optimize_abs_jumps(std::vector<vm::command>& code)
{
  if (any_of(begin(code), end(code), is_cjmp))
    return false;
  if (any_of(begin(code), end(code),
             [](const auto& c) { return is_ncjmp(c) && c.arg.as_int() < 0; }))
    return false;

  auto i = find_if(begin(code), end(code), is_ncjmp);
  if (i == end(code))
    return false;

  for (; i != end(code); i = find_if(i, end(code), is_ncjmp)) {
    if (i->arg.as_int() >= 0)
      i = code.erase(i, i + 1 + i->arg.as_int());
  }
  return true;
}

// Remove noop instructions created during optimization, unless they're needed
// to ensure that jump indices are correct
bool optimize_noop_instrs(std::vector<vm::command>& code)
{
  if (any_of(begin(code), end(code), is_jump))
    return false;

  const auto last_valid = remove_if(begin(code), end(code), is_noop);
  if (last_valid != end(code)) {
    code.erase(last_valid, end(code));
    return true;
  }
  return false;
}

// }}}

bool optimize_once(std::vector<vm::command>& code)
{
  auto changed = false;
  if (optimize_blocks(code))       changed = true;
  if (optimize_instructions(code)) changed = true;
  if (optimize_noops(code))        changed = true;
  if (optimize_constants(code))    changed = true;
  if (optimize_args(code)) changed = true;
  if (optimize_cond_jumps(code))   changed = true;
  if (optimize_abs_jumps(code))    changed = true;
  if (optimize_noop_instrs(code))  changed = true;

  return changed;
}

bool optimize_independent_once(std::vector<vm::command>& code)
{
  auto changed = false;

  if (optimize_lets(code)) changed = true;

  return changed;
}

}

void vv::optimize(std::vector<vm::command>& code)
{
  while (optimize_once(code))
    ;
}

void vv::optimize_independent_block(std::vector<vm::command>& code)
{
  auto changed = true;
  while (changed) {
    changed = false;
    if (optimize_once(code))             changed = true;
    if (optimize_independent_once(code)) changed = true;
  }
}
