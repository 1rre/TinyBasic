#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "basic.h"

UInt is_whitespace(char c) {
  return c == ' ' || c == '\t';
}

UInt is_digit(char c) {
  return c >= '0' && c <= '9';
}

UInt is_letter(char c) {
  return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'z';
}

UInt is_nonident(char c) {
  return !(is_digit(c) || is_letter(c));
}

void skip_whitespace(UncompiledCommand* cmd) {
  while (cmd->Size && is_whitespace(cmd->Contents[0])) {
    cmd->Size--, cmd->Contents++;
  }
}

UInt check(UncompiledCommand* cmd, char* expect, int len) {
  if (cmd->Size >= len && !strncasecmp(expect, cmd->Contents, len)) {
    cmd->Size -= len;
    cmd->Contents += len;
    return 1;
  } else return 0;
}

CommandId get_token(UncompiledCommand* cmd) {
  /* Non-Unary */
  if (check(cmd, "CALL", 4)) return id_call;
  if (check(cmd, "FOR", 3)) return id_for;
  if (check(cmd, "GOTO", 4)) return id_goto;
  if (check(cmd, "IF", 2)) return id_if;
  if (check(cmd, "INPUT", 5)) return id_input;
  if (check(cmd, "LET", 3)) return id_let;
  if (check(cmd, "LIST", 4)) return id_list;
  if (check(cmd, "NOTE", 4)) return id_note;
  if (check(cmd, "UNTIL", 5)) return id_until;
  if (check(cmd, "WHILE", 5)) return id_while;
  /* Unary */
  if (check(cmd, "END", 3)) return id_end;
  if (check(cmd, "RETURN", 6)) return id_return;
  if (check(cmd, "RUN", 3)) return id_run;
  if (check(cmd, "STOP", 4)) return id_stop;
  /* Fallback */
  return id_null;
}

UInt at_end(UncompiledCommand cmd) {
  return cmd.Size == 0 ||
        *cmd.Contents == ';'  ||
        *cmd.Contents == '\0' ||
        *cmd.Contents == '\n';
}

CommandDetails* parse_command(UncompiledCommand cmd) {
  // ???
  skip_whitespace(&cmd);
  CommandId token = get_token(&cmd);
  CommandDetails* rtn = 0;
  Command out;
  switch (token) {
    case id_null: goto syntax_error;
    case id_if:
      notimpl("IF");
    case id_while:
      notimpl("WHILE");
    case id_until:
      notimpl("UNTIL");
    case id_for:
      notimpl("FOR");
    case id_input:
      notimpl("INPUT");
    case id_list:
      notimpl("LIST");
    case id_let:
      notimpl("LET");
    case id_goto:
      notimpl("GOTO");
    case id_call:
      notimpl("CALL");
    /* Don't care about comment contents */
    case id_note:
      if (cmd.Size == 0 || is_nonident(*cmd.Contents)) break;
      else goto syntax_error;
    /* Assert no junk at end of line for unary commands */
    case id_run: case id_return: case id_stop: case id_end:
      if (!at_end(cmd)) goto syntax_error;
    break;
    /* Multiple should not be possible here */
    default: exit(1);
  }
  rtn = (CommandDetails*)malloc(sizeof(CommandDetails));
  rtn->Id = token;
  rtn->Command = out;
  return rtn;
  
syntax_error:
  printf("? Syntax error");
  return rtn;
}
