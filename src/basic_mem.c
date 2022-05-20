#include "basic.h"
#include <stdio.h>

void free_value(ValueToken* val) {
  if (!val) return;
  switch (val->Id) {
    case value_null:
    // ???
    break;
    case value_register:
    // ???
    break;
    case value_memory:
    // ???
    break;
    case value_string:
    // ???
    break;
    case value_int:
    // TODO: Switch details to ptr & free
    break;
  }

  free(val);
}

void free_command(CommandDetails* cmd) {
  printf("Free: %p\n", cmd);
  if (!cmd) return;
  switch (cmd->Id) {
    case cmd_null:
      printf("Free Null\n");
      if (cmd->Command.Uncompiled) {
        if (cmd->Command.Uncompiled->Contents) {
          printf("Free Contents: %s (%p)\n", cmd->Command.Uncompiled->Contents);
          free(cmd->Command.Uncompiled->Contents);
          printf("Free'd 1\n");
        }
        free(cmd->Command.Uncompiled);
        printf("Free'd 2\n");
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
      notimpl("Free Let");
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
