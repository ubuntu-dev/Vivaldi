#include "opt.h"

#include "builtins.h"
#include "vm/instruction.h"

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
  return com.instr == vm::instruction::let;
}

bool is_opt_fn(const vm::command& com)
{
  if (com.instr != vm::instruction::readm)
    return false;
  auto sym = com.arg.as_sym();
  return sym == builtin::sym::add   || sym == builtin::sym::subtract
      || sym == builtin::sym::times || sym == builtin::sym::divides;
}

bool is_prim_push(const vm::command& com)
{
  switch (com.instr) {
  case vm::instruction::pnil:
  case vm::instruction::pint:
  case vm::instruction::pflt:
  case vm::instruction::pbool:
  case vm::instruction::pstr:
  case vm::instruction::psym: return true;
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

bool optimize_instructions(std::vector<vm::command>& code)
{
  auto changed = false;

  auto i = find_if(begin(code), end(code), is_opt_fn);
  for (; i != end(code); i = find_if(i, end(code), is_opt_fn)) {
    auto next = find_if(i + 1, end(code),
                        [](const auto& c)
                          { return c.instr != vm::instruction::noop; });
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

bool optimize_noops(std::vector<vm::command>& code)
{
  auto immut = jump_targets(code);

  auto changed = false;

  auto i = find_if(begin(code), end(code), is_prim_push);
  for (; i != end(code); i = find_if(i, end(code), is_prim_push)) {
    auto next = i + 1;
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

bool optimize_constants(std::vector<vm::command>& code)
{
  auto changed = false;

  auto i = find_if(begin(code), end(code), is_opt);
  for (; i != end(code); i = find_if(i, end(code), is_opt)) {
    if (i - begin(code) > 1) {
      auto a2 = i - 2;
      auto a1 = i - 1;
      if (a2->instr == vm::instruction::pint && a1->instr == vm::instruction::pint) {
        changed = true;

        int result;
        auto val1 = a1->arg.as_int();
        auto val2 = a2->arg.as_int();
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

bool optimize_cond_jumps(std::vector<vm::command>& code)
{
  auto changed = false;

  auto immut = jump_targets(code);

  auto i = find_if(++begin(code), end(code), is_cjmp);
  for (; i != end(code); i = find_if(i, end(code), is_cjmp)) {
    auto condition = i - 1;
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

bool optimize_noop_instrs(std::vector<vm::command>& code)
{
  if (any_of(begin(code), end(code), is_jump))
    return false;

  auto last_valid = remove_if(begin(code), end(code),
                              [](const auto& c)
                                { return c.instr == vm::instruction::noop; });
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
  if (optimize_cond_jumps(code))   changed = true;
  if (optimize_abs_jumps(code))    changed = true;
  if (optimize_noop_instrs(code))  changed = true;

  return changed;
}

}

void vv::optimize(std::vector<vm::command>& code)
{
  while (optimize_once(code))
    ;
}
