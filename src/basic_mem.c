#include "basic.h"

void free_value(ValueToken val) {
  notimpl("Value Token");
}

void free_command(CommandDetails cmd) {
  switch (cmd.Id) {
    case id_null:
      free(cmd.Command.Uncompiled.Contents);
    return;
    case id_if:
      free_value(*cmd.Command.If.Predicate); return;
    case id_while:
      free_value(*cmd.Command.While.Predicate); return;
    case id_until:
      free_value(*cmd.Command.Until.Predicate); return;
    case id_for:

    return;
    case id_input:

    return;
    case id_list:

    return;
    case id_run:

    return;
    case id_return:

    return;
    case id_let:

    return;
    case id_goto:

    return;
    case id_call:

    return;
    case id_stop:

    return;
    case id_end:

    return;
    case id_rem:

    return;
    case id_multip:

    return;
  }
}