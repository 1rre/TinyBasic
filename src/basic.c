#define  _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include "basic.h"

void notimpl(char* c) {
  fprintf(stderr, "Not Implemented: %s\n", c);
  exit(1);
}

UInt variables[26], *memory = 0;

struct StatementNode {
  CommandToken cmd;
  struct StatementNode* next;
} *statements = 0;

typedef struct StatementNode* StatementList;

StatementList insert_cmd(CommandToken cmd, StatementList s) {
  if (!s || s->cmd.LineNum > cmd.LineNum) {
    StatementList rtn = (StatementList)malloc(sizeof(struct StatementNode));
    rtn->cmd = cmd;
    rtn->next = s;
    return rtn;
  } else if (s->cmd.LineNum < cmd.LineNum) {
    s->next = insert_cmd(cmd, s->next);
    return s;
  } else /* if (s->cmd.LineNum == cmd.LineNum) */ {
    free_command(s->cmd.Details);
    s->cmd.Details = cmd.Details;
    return s;
  }
}

typedef enum {
  rcode_stop,
  rcode_continue,
  rcode_return
} RCode;

RCode run_statements(StatementList);
RCode run_command(CommandDetails*, UInt*);

RCode run_statements(StatementList s) {
  if (s) {
    switch (run_command(&s->cmd.Details, &s->cmd.LineNum)) {
      case rcode_stop:
        return rcode_stop;
      case rcode_continue:
        //__attribute__((musttail))
        return run_statements(s->next);
      case rcode_return:
        return rcode_continue;
      default: exit(1);
    }
  } else {
    printf("Reached the end of control without stop.\n");
    return rcode_stop;
  }
}

RCode run_command(CommandDetails* cmd, UInt* LineNum) {
  CommandDetails* c;
  switch (cmd->Id) {
    case cmd_null:
      c = parse_command(cmd->Command.Uncompiled);
      //printf("Got command code: %d\n", c->Id);
      if (c && c->Id != cmd_null) run_command(c, LineNum);
      else if (LineNum) printf(" On Line %u\n", *LineNum);
      else printf("\n");
      if (c) free_command(*c), free(c);
    return rcode_stop;
    case cmd_if:
      notimpl("IF");
    __attribute__ ((fallthrough));
    case cmd_while:
      notimpl("WHILE");
    __attribute__ ((fallthrough));
    case cmd_until:
      notimpl("UNTIL");
    __attribute__ ((fallthrough));
    case cmd_for:
      notimpl("FOR");
    __attribute__ ((fallthrough));
    case cmd_input:
      notimpl("INPUT");
    __attribute__ ((fallthrough));
    case cmd_list:
      notimpl("LIST");
    __attribute__ ((fallthrough));
    case cmd_let:
      notimpl("LET");
    __attribute__ ((fallthrough));
    case cmd_goto:
      notimpl("GOTO");
    __attribute__ ((fallthrough));
    case cmd_call:
      notimpl("CALL");
    __attribute__ ((fallthrough));
    case cmd_end:
      notimpl("END");
    __attribute__ ((fallthrough));
    case cmd_run: return run_statements(statements);
    case cmd_return: return rcode_return;
    case cmd_stop: return rcode_stop;
    case cmd_note: return rcode_continue;
    case cmd_multiple:
      switch (run_command(cmd->Command.Multiple.Left, LineNum)) {
      case rcode_continue:
        //__attribute__((musttail))
        return run_command(cmd->Command.Multiple.Right, LineNum);
      case rcode_return: return rcode_continue;
      case rcode_stop: return rcode_stop;
      }
    default:
      printf("Unrecognised Command On Line %u!\n", *LineNum);
    exit(1);
  }
}

void update_env(char* input, size_t size) {
  // TODO: Parse input
  ParseResult ln = parse_linenum(input, size);
  CommandToken cmd;
  if (ln.value) {
    UncompiledCommand uCmd = {ln.position, size - (ln.position - input)};
    cmd.Details.Command.Uncompiled = uCmd;
    cmd.Details.Id = cmd_null;
    cmd.LineNum = *(UInt*)ln.value;
    statements = insert_cmd(cmd, statements);
  } else {
    UncompiledCommand uCmd = {input, size};
    cmd.Details.Command.Uncompiled = uCmd;
    cmd.Details.Id = cmd_null;
    run_command(&cmd.Details, 0);
  }
}

void run_interpreter() {
  /* TODO: Set read mode
  */
  char* buf;
  size_t size;
  printf("READY\n");
start:
  buf = 0;
  size = 0;
  getline(&buf, &size, stdin);
  update_env(buf, size);

  goto start;
}