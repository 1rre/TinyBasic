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
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'z');
}

UInt is_nonident(char c) {
  return !(is_digit(c) || is_letter(c));
}

ParseResult parse_linenum(char* input, size_t size) {
  ParseResult rtn = {input, 0};
  if (size > 0 && input[0] >= '0' && input[0] <= '9')
    rtn.value = calloc(1, sizeof(UInt));
  for (unsigned i = 0; i < size && input[i] >= '0' && input[i] <= '9'; i++) {
    if (*(UInt*)rtn.value > (UINT_MAX/10)) {
      // Overflow
      exit(1);
    }
    UInt to_add = input[i] - '0';
    *(UInt*)rtn.value *= 10;
    if (UINT_MAX - to_add < *(UInt*)rtn.value) {
      // Overflow
      exit(1);
    }
    *(UInt*)rtn.value += input[i] - '0';
    rtn.position++;
  }
  return rtn;
}

void skip_whitespace(UncompiledCommand* cmd) {
  while (cmd->Size && is_whitespace(cmd->Contents[0])) {
    cmd->Size--, cmd->Contents++;
  }
}

UInt check(UncompiledCommand* cmd, char* expect, unsigned len) {
  if (cmd->Size >= len && !strncasecmp(expect, cmd->Contents, len)) {
    cmd->Size -= len;
    cmd->Contents += len;
    return 1;
  } else return 0;
}

CommandId get_token(UncompiledCommand* cmd) {
  /* Non-Unary */
  if (check(cmd, "CALL", 4)) return cmd_call;
  if (check(cmd, "FOR", 3)) return cmd_for;
  if (check(cmd, "GOTO", 4)) return cmd_goto;
  if (check(cmd, "IF", 2)) return cmd_if;
  if (check(cmd, "INPUT", 5)) return cmd_input;
  if (check(cmd, "LET", 3)) return cmd_let;
  if (check(cmd, "LIST", 4)) return cmd_list;
  if (check(cmd, "NOTE", 4)) return cmd_note;
  if (check(cmd, "UNTIL", 5)) return cmd_until;
  if (check(cmd, "WHILE", 5)) return cmd_while;
  /* Unary */
  if (check(cmd, "END", 3)) return cmd_end;
  if (check(cmd, "RETURN", 6)) return cmd_return;
  if (check(cmd, "RUN", 3)) return cmd_run;
  if (check(cmd, "STOP", 4)) return cmd_stop;
  /* Fallback */
  return cmd_null;
}

UInt at_end(UncompiledCommand cmd) {
  return cmd.Size == 0 ||
        *cmd.Contents == ';'  ||
        *cmd.Contents == '\0' ||
        *cmd.Contents == '\n';
}

UInt parse_value(CommandDetails* rtn,  UncompiledCommand* cmd) {
  printf("Before whitespace skip: %s\n", cmd->Contents);
  skip_whitespace(cmd);
  printf("After whitespace skip: %s\n", cmd->Contents);
  
  if (cmd->Size <= 0u) return 0;

  if (is_digit(*cmd->Contents)) {
    notimpl("Parse Number");
  }
  if (is_letter(*cmd->Contents)) {
    notimpl("Parse Identifier");
  }
  if (*cmd->Contents == '(') {
    notimpl("Parse Bracketed");
  }
  if (*cmd->Contents == '"') {
    notimpl("Parse String");
  }

  // Unexpected char?
  return 0;
}

CommandDetails* parse_command(UncompiledCommand cmd) {
  // ???
  printf("Parsing %s\n", cmd.Contents);
  skip_whitespace(&cmd);
  CommandId token = get_token(&cmd);
  CommandDetails* rtn = 0;
  switch (token) {
    case cmd_null: goto syntax_error;
    case cmd_for:
      notimpl("FOR");
    case cmd_input:
      notimpl("INPUT");
    case cmd_let:
      notimpl("LET");
    /* These take a value */
    case cmd_list: case cmd_goto: case cmd_call:
    case cmd_until: case cmd_while: case cmd_if:
      if (!parse_value(rtn, &cmd) || !rtn)
        goto syntax_error;
    break;
    /* Don't care about comment contents */
    case cmd_note:
      rtn = (CommandDetails*)malloc(sizeof(CommandDetails));
      while (!at_end(cmd))
        cmd.Contents++, cmd.Size--;
    break;
    /* Assert no junk at end of line for unary commands */
    case cmd_run: case cmd_return: case cmd_stop: case cmd_end:
      if (!at_end(cmd)) goto syntax_error;
    break;
    /* Multiple should not be possible here */
    default: exit(1);
  }
  printf("Remaining: %s\n", cmd.Contents);
  if (!at_end(cmd)) {
    free_command(*rtn);
    free(rtn);
    printf("? Junk at end of line");
    return 0;
  }
  rtn->Id = token;
  if (cmd.Size && *cmd.Contents == ';') {
    printf("Is multiple.\n");
    cmd.Size--, cmd.Contents++;
    CommandDetails* right = parse_command(cmd);
    if (right) {
      CommandDetails* rtn1 = (CommandDetails*)malloc(sizeof(CommandDetails));
      rtn1->Id=cmd_multiple;
      rtn1->Command.Multiple.Left = rtn;
      rtn1->Command.Multiple.Right = right;
      return rtn1;
    }
  }
  return rtn;

syntax_error:
  printf("? Syntax error");
  return 0;
}
