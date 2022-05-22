#define  _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "basic.h"

UInt variables[26];

union {
  UInt* UInt;
  char* String;
} memory = {0};

#ifdef _WIN32
size_t getline(char** buf, size_t* size, FILE* stream) {
  *buf = (char*)calloc(120, sizeof(char));
  scanf("%s\n", *buf);
  return strlen(*buf);
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

StatementList get_statement(UInt i) {
  StatementList s = statements;
  while (s && s->cmd.LineNum < i) s = s->next;
  if (s && s->cmd.LineNum == i) return s;
  else return 0;
}

RCode run_from(UInt i) {
  StatementList s = get_statement(i);
  if (s) return run_statements(s);
  else {
    printf("? line %d undefined\n", i);
    return rcode_stop;
  }
}

typedef union {
  char* String;
  UInt Int;
} ResolvedValue;

ResolvedValue resolveValue(ValueToken* value) {
  ResolvedValue rtn;
  if (!value) exit(1); // Error?
  switch (value->Id) {
    case value_null: // Error?
      exit(1);
    case value_register:
      rtn.Int = variables[value->Details->Register.name];
    break;
    case value_memory:
      rtn = resolveValue(
        (ValueToken*)value->Details->Memory.Reference
      );
    break;
    case value_int:
      rtn.Int = value->Details->IntLiteral;
    break;
    case value_string:
      rtn.String = "";
      notimpl("String");
    break;
  }
  return rtn;
}

RCode run_list(CommandDetails* cmd) {
  if (!cmd) {
    printf("? command null\n");
    return rcode_stop;
  }
  UInt line = resolveValue(cmd->Command.Value).Int;
  StatementList s = get_statement(line);
  printf("%s\n", s->cmd.Details->Command.Uncompiled->Contents);
  return rcode_continue;
}

RCode run_command(CommandDetails* cmd, UInt* LineNum) {
  CommandDetails* c;
  RCode rtn = rcode_stop;
  switch (cmd->Id) {
    case cmd_null:
      c = parse_command(*cmd->Command.Uncompiled);
      if (c && c->Id != cmd_null) rtn = run_command(c, LineNum);
      else if (LineNum) printf(" On Line %u\n", *LineNum);
      else if (cmd->Command.Uncompiled->Contents[0] != '\n') printf("\n");
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
      return run_list(cmd);
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
    uCmd->Contents = input, uCmd->Size = size - (ln.position - input);
    cmd.Details->Command.Uncompiled = uCmd;
    cmd.Details->Id = cmd_null;
    cmd.LineNum = *(UInt*)ln.value;
    free(ln.value);
    statements = insert_cmd(cmd, statements);
    return rcode_continue;
  } else {
    uCmd->Contents = input;
    uCmd->Size = size;
    cmd.Details->Command.Uncompiled = uCmd;
    cmd.Details->Id = cmd_null;
    run_command(cmd.Details, 0);
    free_command(cmd.Details);
    return rcode_return;
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
  r = update_env(buf, size);
  switch (r) {
    case rcode_continue:
      goto start;
    case rcode_return:
      printf("READY\n");
      goto start;
    case rcode_stop:
      printf("Goodbye\n");
  }
  free_all_statements();
}
