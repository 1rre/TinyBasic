#define  _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "basic.h"

UInt variables[26], *memory = 0;

#ifdef _WIN32
int getline(char** buf, size_t* size, FILE* stream) {
  *buf = (char*)malloc(120 * sizeof(char));
  printf("Malloc'd %p\n", *buf);
  fscanf(stream, "%[^\r\n]*\r?\n", *buf);
  printf("Read %s\n", *buf);
  *size = strlen(*buf);
}
#endif

struct StatementNode {
  CommandToken cmd;
  struct StatementNode* next;
} *statements = 0;

typedef struct StatementNode* StatementList;

void free_statement(StatementList s) {
  if (s) {
    free_command(s->cmd.Details);
    free_statement(s->next);
    free(s);
  }
}

void free_all_statements() {
  free_statement(statements);
}

void notimpl(char* c) {
  fprintf(stderr, "Not Implemented: %s\n", c);
  free_all_statements();
  exit(1);
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
    switch (run_command(s->cmd.Details, &s->cmd.LineNum)) {
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

RCode run_from(UInt i) {
  StatementList s = statements;
  while (s && s->cmd.LineNum < i) s = s->next;
  if (s && s->cmd.LineNum == i) return run_statements(s);
  else {
    printf("? line %d undefined\n", i);
    return rcode_stop;
  }
}

RCode run_command(CommandDetails* cmd, UInt* LineNum) {
  CommandDetails* c;
  RCode rtn = rcode_stop;
  switch (cmd->Id) {
    case cmd_null:
      c = parse_command(*cmd->Command.Uncompiled);
      printf("Got command code: %d\n", c->Id);
      if (c && c->Id != cmd_null) rtn = run_command(c, LineNum);
      else if (LineNum) printf(" On Line %u\n", *LineNum);
      else printf("\n");
      if (c) free_command(c);
    return rtn;
    case cmd_if:
      notimpl("IF");
    case cmd_while:
      notimpl("WHILE");
    case cmd_until:
      notimpl("UNTIL");
    case cmd_for:
      notimpl("FOR");
    case cmd_input:
      notimpl("INPUT");
    case cmd_list:
      notimpl("LIST");
    case cmd_let:
      notimpl("LET");
    case cmd_goto:
      /* TODO: Next line <- goto...
               Is it necessary to return "return" || stop here due to run_statements?
      */
      notimpl("GOTO");
    case cmd_call:
      notimpl("CALL");
    case cmd_end:
      notimpl("END");
    case cmd_run: return run_statements(statements);
    case cmd_return: return rcode_return;
    case cmd_stop: return rcode_stop;
    case cmd_note: return rcode_continue;
    case cmd_multiple:
      switch (run_command(cmd->Command.Multiple->Left, LineNum)) {
      case rcode_continue:
        //__attribute__((musttail))
        return run_command(cmd->Command.Multiple->Right, LineNum);
      case rcode_return: return rcode_continue;
      case rcode_stop: return rcode_stop;
      }
    default:
      printf("Unrecognised Command On Line %u!\n", *LineNum);
    exit(1);
  }
}

RCode update_env(char* input, size_t size) {
  // TODO: Parse input
  ParseResult ln = parse_linenum(input, size);
  CommandToken cmd;
  cmd.Details = (CommandDetails*)malloc(sizeof(CommandDetails));
  UncompiledCommand* uCmd =
    (UncompiledCommand*)malloc(sizeof(UncompiledCommand));
  if (ln.value) {
    printf("Parse Line %u\n", *(UInt*)ln.value);
    uCmd->Contents = input, uCmd->Size = size - (ln.position - input);
    cmd.Details->Command.Uncompiled = uCmd;
    cmd.Details->Id = cmd_null;
    cmd.LineNum = *(UInt*)ln.value;
    free(ln.value);
    statements = insert_cmd(cmd, statements);
    return rcode_continue;
  } else {
    printf("No line given.\n");
    uCmd->Contents = input;
    uCmd->Size = size;
    cmd.Details->Command.Uncompiled = uCmd;
    cmd.Details->Id = cmd_null;
    RCode rtn = run_command(cmd.Details, 0);
    free_command(cmd.Details);
    printf("Returning %d\n", rtn);
    return rtn;
  }
}

void run_interpreter() {
  /* TODO: Set read mode
  */
  char* buf;
  size_t size;
  RCode r = rcode_stop;
  printf("READY\n");
start:
  buf = 0;
  size = 0;
  getline(&buf, &size, stdin);
  printf("Read: %s\n", buf);
  r = update_env(buf, size);
  printf("Rcode: %d\n", r);
  switch (r) {
    case rcode_continue:
      printf("Continue\n");
      goto start;
    case rcode_return:
      printf("Return with no stop\n");
      goto start;
    case rcode_stop:
      printf("Goodbye\n");
  }
  free_all_statements();
}
