/*******************************************************************\

Module: Modifies

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

/// \file
/// Modifies

#include "function_modifies.h"

#include <util/std_expr.h>

#include <analyses/local_may_alias.h>

#include "loop_utils.h"

void function_modifiest::get_modifies(
  const local_may_aliast &local_may_alias,
  const goto_programt::const_targett i_it,
  modifiest &modifies)
{
  const goto_programt::instructiont &instruction=*i_it;

  if(instruction.is_assign())
  {
    get_modifies_lhs(local_may_alias, i_it, instruction.assign_lhs(), modifies);
  }
  else if(instruction.is_function_call())
  {
    const exprt &lhs = instruction.call_lhs();

    // return value assignment
    if(lhs.is_not_nil())
      get_modifies_lhs(local_may_alias, i_it, lhs, modifies);

    get_modifies_function(instruction.call_function(), modifies);
  }
}

void function_modifiest::get_modifies_function(
  const exprt &function,
  modifiest &modifies)
{
  if(function.id()==ID_symbol)
  {
    const irep_idt &identifier=to_symbol_expr(function).get_identifier();

    function_mapt::const_iterator fm_it=
      function_map.find(identifier);

    if(fm_it!=function_map.end())
    {
      // already done
      modifies.insert(fm_it->second.begin(), fm_it->second.end());
      return;
    }

    goto_functionst::function_mapt::const_iterator
      f_it=goto_functions.function_map.find(identifier);

    if(f_it==goto_functions.function_map.end())
      return;

    local_may_aliast local_may_alias(f_it->second);

    const goto_programt &goto_program=f_it->second.body;

    forall_goto_program_instructions(i_it, goto_program)
      get_modifies(local_may_alias, i_it, modifies);
  }
  else if(function.id()==ID_if)
  {
    get_modifies_function(to_if_expr(function).true_case(), modifies);
    get_modifies_function(to_if_expr(function).false_case(), modifies);
  }
}
