#include "basic.h"
#include <stdio.h>

void free_value(ValueToken* value) {
  if (!value) return;
  if (!value->Details) {
    free(value);
    return;
  }
  switch (value->Id) {
    case value_memory:
      free_value((ValueToken*)value->Details->Memory.Reference);
    break;
    case value_binop:
      free_value((ValueToken*)value->Details->BinOp.Left);
      free_value((ValueToken*)value->Details->BinOp.Right);
    break;
    case value_null:
    // ???
    break;
    case value_register:
    break;
    case value_string:
    // ???
    break;
    case value_int:
    // TODO: Switch details to ptr & free
    break;
    case value_funcall:

    break;
  }
  free(value->Details);
  free(value);
}

void free_command(CommandDetails* cmd) {
  if (!cmd) return;
  switch (cmd->Id) {
    case cmd_null:
      if (cmd->Command.Uncompiled) {
        free(cmd->Command.Uncompiled->FullLine);
        free(cmd->Command.Uncompiled);
      }
    break;
    case cmd_list: case cmd_goto: case cmd_call:
    case cmd_until: case cmd_while: case cmd_if:
      free_value(cmd->Command.Value);
    break;
    case cmd_for:
      notimpl("Free For");
    break;
    case cmd_input:
      notimpl("Free Input");
    break;
    case cmd_let:
      free_value(cmd->Command.Let->Memory);
      free_value(cmd->Command.Let->Value);
      free(cmd->Command.Let);
    break;
    case cmd_print:
      notimpl("Free Print");
    break;
    case cmd_multiple:
      free_command(cmd->Command.Multiple->Left);
      free_command(cmd->Command.Multiple->Right);
      free(cmd->Command.Multiple);
    break;
    case cmd_end: case cmd_note: case cmd_stop:
    case cmd_return: case cmd_run:
    break;
  }
  free(cmd);
}
