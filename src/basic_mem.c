#include "basic.h"
#include <stdio.h>

void free_value(ValueToken value) {
  switch (value.Id) {
    case value_memory:
      if (value.Details.Reference) {
        free_value(*value.Details.Reference);
        free(value.Details.Reference);
      }
    break;
    case value_binop:
      if (value.Details.BinOp) {
        free_value(value.Details.BinOp->Left);
        free_value(value.Details.BinOp->Right);
        free(value.Details.BinOp);
      }
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
}

void free_command(CommandDetails cmd) {
  switch (cmd.Id) {
    case cmd_null:
      if (cmd.Command.Uncompiled) {
        free(cmd.Command.Uncompiled->FullLine);
        free(cmd.Command.Uncompiled);
      }
    break;
    case cmd_list: case cmd_goto: case cmd_call:
    case cmd_until: case cmd_while: case cmd_if:
      if (cmd.Command.Value) {
        free_value(*cmd.Command.Value);
        free(cmd.Command.Value);
      }
    break;
    case cmd_for:
      if (cmd.Command.For) {
        free_value(cmd.Command.For->Hi);
        free_value(cmd.Command.For->Lo);
        free_value(cmd.Command.For->Step);
      }
    break;
    case cmd_input:
      if (cmd.Command.Input) {
        free(cmd.Command.Input->Prompt);
        free_value(cmd.Command.Input->To);
        free(cmd.Command.Input);
      }
    break;
    case cmd_let:
      free_value(cmd.Command.Let->Memory);
      free_value(cmd.Command.Let->Value);
      free(cmd.Command.Let);
    break;
    case cmd_print:
    /* DEPRECATED */
      free_value(*cmd.Command.Value);
      free(cmd.Command.Value);
    break;
    case cmd_multiple:
      free_command(cmd.Command.Multiple->Left);
      free_command(cmd.Command.Multiple->Right);
      free(cmd.Command.Multiple);
    break;
    case cmd_end: case cmd_note: case cmd_stop:
    case cmd_return: case cmd_run:
    break;
  }
}
