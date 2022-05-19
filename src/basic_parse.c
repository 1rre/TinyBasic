#include <stdlib.h>
#include <stdio.h>
#include "basic.h"

UInt is_whitespace(char c) {
  return c == ' ' || c == '\n' || c == '\v' || c == '\t' || c == '\r';
}

UInt is_digit(char c) {
  return c >= '0' && c <= '9';
}

void skip_whitespace(UncompiledCommand* cmd) {
  while (cmd->Size && is_whitespace(cmd->Contents[0])) {
    cmd->Size--, cmd->Contents++;
  }
}

CommandId get_token(UncompiledCommand* cmd) {
  if (!cmd->Size) return id_null;
  notimpl("Get Token");
}

CommandDetails* parse_command(UncompiledCommand cmd) {
  // ???
  printf("Trying to parse: %s (size %lu)\n", cmd.Contents, cmd.Size);
  skip_whitespace(&cmd);
  printf("Trying to parse: %s (size %lu)\n", cmd.Contents, cmd.Size);
  CommandId token = get_token(&cmd);
  printf("Trying to parse: %s (size %lu)\n", cmd.Contents, cmd.Size);
  switch (token) {
    case id_if:
    case id_while:
    case id_until:
    case id_for:
    case id_input:
    case id_list:
    case id_run:
    case id_return:
    case id_let:
    case id_goto:
    case id_call:
    case id_stop:
    case id_end:
    case id_rem:
    case id_multip:
    case id_null:
      printf("? Syntax error");
    return 0;
    /* Uncompiled should not be possible */
    default: exit(1);
  }
  return 0;
}
