#define  _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include "basic.h"

#define UINT_MAX ((UInt)-1u)

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
      case rcode_stop: return rcode_stop;
      case rcode_continue: return run_statements(s->next);
      case rcode_return: return rcode_continue;
    }
  } else {
    printf("Reached the end of control without stop.\n");
    return rcode_stop;
  }
}

RCode run_command(CommandDetails* cmd, UInt* LineNum) {
  switch (cmd->Id) {
    case id_null:
      CommandDetails* c = parse_command(cmd->Command.Uncompiled);
      if (c && c->Id != id_null) run_command(c, LineNum);
      else if (LineNum) printf(" On Line %u\n", *LineNum);
      else printf("\n");
      if (c) free_command(*c), free(c);
    return rcode_stop;
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
    case id_run:
      run_statements(statements);
    case id_let:
      notimpl("LET");
    case id_goto:
      notimpl("GOTO");
    case id_call:
      notimpl("CALL");
    case id_end:
      notimpl("END");
    case id_return: return rcode_return;
    case id_stop: return rcode_stop;
    case id_note: return rcode_continue;
    case id_multip:
      MultipleCommand mc = cmd->Command.Multiple;
      run_command(mc.Left, LineNum);
      run_command(mc.Right, LineNum);
      
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
    cmd.Details.Id = id_null;
    cmd.LineNum = *(UInt*)ln.value;
    statements = insert_cmd(cmd, statements);
  } else {
    UncompiledCommand uCmd = {input, size};
    cmd.Details.Command.Uncompiled = uCmd;
    cmd.Details.Id = id_null;
    run_command(&cmd.Details, 0);
  }
}

void run_interpreter() {
  /* TODO: Set read mode
  */
  printf("READY\n");
start:
  char* buf = 0;
  size_t size = 0;
  getline(&buf, &size, stdin);
  update_env(buf, size);

  goto start;
}