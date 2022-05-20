#include "basic.h"

void free_value(ValueToken val) {
  notimpl("Value Token");
}

void free_command(CommandDetails cmd) {
  switch (cmd.Id) {
    case cmd_null:
      free(cmd.Command.Uncompiled.Contents);
    return;
    case cmd_if:
      free_value(*cmd.Command.If.Predicate); return;
    case cmd_while:
      free_value(*cmd.Command.While.Predicate); return;
    case cmd_until:
      free_value(*cmd.Command.Until.Predicate); return;
    case cmd_for:

    return;
    case cmd_input:

    return;
    case cmd_list:

    return;
    case cmd_run:

    return;
    case cmd_return:

    return;
    case cmd_let:

    return;
    case cmd_goto:

    return;
    case cmd_call:

    return;
    case cmd_stop:

    return;
    case cmd_end:

    return;
    case cmd_note:

    return;
    case cmd_multiple:

    return;
  }
}