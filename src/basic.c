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

UInt mem_size = 0;

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
  rcode_return,
  rcode_end,
  rcode_kill
} RCode;

RCode run_statements(StatementList);
RCode run_command(StatementList*, CommandDetails*);

RCode run_statements(StatementList s) {
  if (s) {
    switch (run_command(&s, &s->cmd.Details)) {
      case rcode_stop:
        return rcode_stop;
      case rcode_continue:
        if (s) return run_statements(s->next);
        printf("? Undefined statement\n");
        return rcode_stop;
      /* These should both work the same */
      case rcode_return: case rcode_end:
        return rcode_continue;
      default: exit(1);
    }
  } else {
    printf("? beyond end of control\n");
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

enum ValueType {
  NullT,
  StringT,
  IntT
};

typedef struct {
  enum ValueType Type;
   union {
    char* String;
    UInt Int;
  } Value;
} ResolvedValue;

ResolvedValue resolveValue(ValueToken*);

ResolvedValue do_binop(BinOp value) {
  ResolvedValue left = resolveValue(&value.Left);
  ResolvedValue right = resolveValue(&value.Right);
  ResolvedValue rtn;
  if (left.Type != IntT || right.Type != IntT) {
    printf("? binop: non-int value\n");
    rtn.Type = NullT;
    return rtn;
  }
  rtn.Type = IntT;
  switch (value.Op) {
    case op_add:
      if (MAX_LOCAL_INT - left.Value.Int < right.Value.Int) {
        printf("? overflow\n");
        rtn.Type = NullT;
        return rtn;
      }
      rtn.Type = IntT;
      rtn.Value.Int = left.Value.Int + right.Value.Int;
    return rtn;
    case op_sub:
      /* TODO: Detect underflow */
      rtn.Type = IntT;
      rtn.Value.Int = left.Value.Int - right.Value.Int;
    return rtn;
    case op_mul:
      if (left.Value.Int && MAX_LOCAL_INT / left.Value.Int < right.Value.Int) {
        printf("? overflow\n");
        rtn.Type = NullT;
        return rtn;
      }
      rtn.Type = IntT;
      rtn.Value.Int = left.Value.Int * right.Value.Int;
    return rtn;
    case op_eqs:
      rtn.Type = IntT;
      rtn.Value.Int = left.Value.Int == right.Value.Int;
    return rtn;
    case op_neq:
      rtn.Type = IntT;
      rtn.Value.Int = left.Value.Int != right.Value.Int;
    return rtn;
    default:
      printf("Code is %d\n", value.Op);
      notimpl("Non basic ops");
    return rtn;
  }
}

ResolvedValue resolveValue(ValueToken* value) {
  ResolvedValue rtn;
  switch (value->Id) {
    case value_null: // Error?
      exit(1);
    case value_register:
      rtn.Type = IntT;
      rtn.Value.Int = variables[value->Details.Register];
    break;
    case value_memory:
      rtn.Type = IntT;
      rtn.Value.Int =
        memory.UInt[resolveValue(value->Details.Reference).Value.Int];
    break;
    case value_int:
      rtn.Type = IntT;
      rtn.Value.Int = value->Details.IntLiteral;
    break;
    case value_string:
      rtn.Value.String = "";
      rtn.Type = StringT;
    break;
    case value_binop:
      return do_binop(*value->Details.BinOp);
    break;
    case value_funcall:
      notimpl("Function Calls");
      rtn.Value.String = "";
      rtn.Type = StringT;
    break;
  }
  return rtn;
}

RCode run_list(CommandDetails* cmd) {
  ResolvedValue r = resolveValue(cmd->Command.Value);
  if (r.Type != IntT) {
    printf("? list: non-int value\n");
    return rcode_stop;
  }
  UInt line = r.Value.Int;
  StatementList s = get_statement(line);
  if (s) {
    printf("%s", s->cmd.Details.Command.Uncompiled->FullLine);
    return rcode_continue;
  } else {
    printf("? undefined line %u\n", line);
  }
}

RCode set_register(uint8_t r, ValueToken* t) {
  ResolvedValue rv = resolveValue(t);
  if (rv.Type != IntT) {
    printf("? set: non-int value\n");
    return rcode_stop;
  }
  variables[r] = rv.Value.Int;
  return rcode_continue;
}

RCode set_memory_int(ValueToken* m, UInt v) {
  ResolvedValue loc = resolveValue(m);
  if (loc.Type != IntT) {
    printf("? set[]: non-int value\n");
    return rcode_stop;
  }
  if (mem_size <= loc.Value.Int) {
    memory.UInt = (UInt*)realloc(memory.UInt, loc.Value.Int * sizeof(UInt));
    mem_size = loc.Value.Int - 1;
  }
}

RCode set_memory(ValueToken* m, ValueToken* t) {
  ResolvedValue rv = resolveValue(t);
  if (rv.Type == IntT) set_memory_int(m, rv.Value.Int);
  ResolvedValue loc = resolveValue(m);
  if (loc.Type != IntT) {
    printf("? set[]: non-int value\n");
    return rcode_stop;
  }
  notimpl("String");
  return rcode_continue;
}

RCode run_input(CommandDetails* cmd) {
  printf("%s", cmd->Command.Input->Prompt);
  char* x = 0;
  size_t size = 0;
  getline(&x, &size, stdin);
  UInt v = atoi(x);
  free(x);
  switch (cmd->Command.Input->To.Id) {
    case value_register:
      variables[cmd->Command.Input->To.Details.Register] = v;
    return rcode_continue;
    case value_memory:
      return set_memory_int(cmd->Command.Input->To.Details.Reference, v);
    default:
      printf("? non-settable memory");
      return rcode_stop;
  }
}

RCode run_let(CommandDetails* cmd) {
  switch (cmd->Command.Let->Memory.Id) {
    case value_register:
      return set_register(
        cmd->Command.Let->Memory.Details.Register,
        &cmd->Command.Let->Value
      );
    case value_memory:
      return set_memory(
        cmd->Command.Let->Memory.Details.Reference,
        &cmd->Command.Let->Value
      );
    default:
      printf("? non-settable memory");
      return rcode_stop;
  }
}

void update_uc(int* uc, CommandDetails deets) {

  ParseResult p;
  p.Value = 0;
  switch (deets.Id) {
    case cmd_multiple:
      notimpl("Multiple when searching for end");
    return;
    case cmd_if: case cmd_for: case cmd_while: case cmd_until:
      (*uc)++;
    return;
    case cmd_end:
      (*uc)--;
    return;
    case cmd_null:
      p.Cmd = *deets.Command.Uncompiled;
      if (parse_line(&p)) {
        update_uc(uc, *(CommandDetails*)p.Value);
        free_command(*(CommandDetails*)p.Value);
        free(p.Value);
      }
    return;
    default:
    return;
  }
}

RCode run_to_end(StatementList s, CommandDetails* cmd) {
  if (s->cmd.Details.Id == cmd_multiple) {
    notimpl("If in Multiple commands");
  }
  return run_statements(s->next);
}

UInt find_next_end(StatementList* sl, int uc) {
  if (!(*sl)) return 0;
  update_uc(&uc, (*sl)->cmd.Details);
  if (uc == 0) return 1;
  *sl = (*sl)->next;
  return find_next_end(sl, uc);
}

RCode predicate_inner(StatementList* sl, CommandDetails* cmd) {
  ResolvedValue rv;
  rv = resolveValue(cmd->Command.Value);
  if (rv.Type != IntT) {
    printf("? non-int value for block\n");
    return rcode_stop;
  }
  if (rv.Value.Int) {
    return rcode_continue;
  } else return rcode_end;
}

RCode run_command(StatementList* sl, CommandDetails* cmd) {
  ParseResult p;
  p.Value = 0;
  RCode rtn = rcode_stop;
  ResolvedValue rv;
  StatementList s;

  switch (cmd->Id) {
    case cmd_null:
      p.Cmd = *cmd->Command.Uncompiled;
      if (parse_line(&p))
        rtn = run_command(sl, (CommandDetails*)p.Value);
      else if (sl) printf(" On Line %u\n", (*sl)->cmd.LineNum);
      else if (cmd->Command.Uncompiled->Contents[0] != '\n') printf("\n");
      if (p.Value) {
        free_command(*(CommandDetails*)p.Value);
        free(p.Value);
      }
    return rtn;
    case cmd_if:
      s = *sl;
      if (!find_next_end(sl, 0)) {
        printf("? no end for block\n");
        return rcode_stop;
      }
      switch (predicate_inner(sl, cmd)) {
        case rcode_continue: return run_to_end(s, cmd);
        case rcode_stop: return rcode_stop;
        case rcode_end: return rcode_continue;
        default: return rcode_kill;
      }
    case cmd_while:
      s = *sl;
      if (!find_next_end(sl, 0)) {
        printf("? no end for block\n");
        return rcode_stop;
      }
      while_start:
      switch (predicate_inner(sl, cmd)) {
        case rcode_continue:
          run_to_end(s, cmd);
          goto while_start;
        case rcode_stop: return rcode_stop;
        case rcode_end: return rcode_continue;
        default: return rcode_kill;
      }
    case cmd_until:
      s = *sl;
      if (!find_next_end(sl, 0)) {
        printf("? no end for block\n");
        return rcode_stop;
      }
      until_start:
      switch (predicate_inner(sl, cmd)) {
        /* invert for until */
        case rcode_end:
          run_to_end(s, cmd);
          goto until_start;
        case rcode_stop: return rcode_stop;
        case rcode_continue: return rcode_continue;
        default: return rcode_kill;
      }
    case cmd_for:
      notimpl("FOR");
    case cmd_input:
      return run_input(cmd);
    case cmd_list:
      return run_list(cmd);
    case cmd_let:
      return run_let(cmd);
    case cmd_goto:
      rv = resolveValue(cmd->Command.Value);
      if (rv.Type != IntT) {
        printf("? goto non-int value\n");
        return rcode_stop;
      }
      return run_from(rv.Value.Int);
    case cmd_call:
      rv = resolveValue(cmd->Command.Value);
      if (rv.Type != IntT) {
        printf("? call non-int value\n");
        return rcode_stop;
      }
      if (run_from(rv.Value.Int) == rcode_stop)
        return rcode_stop;
      else return rcode_continue;
    case cmd_end: return rcode_end;
    case cmd_run: return run_statements(statements);
    case cmd_return: return rcode_return;
    case cmd_stop: return rcode_stop;
    case cmd_note: return rcode_continue;
    case cmd_multiple:
    /* TODO: Ensure support for if; ...; end in one line */
      switch (run_command(sl, &cmd->Command.Multiple->Left)) {
      case rcode_continue:
        return run_command(sl, &cmd->Command.Multiple->Right);
      case rcode_return: return rcode_continue;
      case rcode_end: return rcode_end;
      case rcode_stop: return rcode_stop;
      case rcode_kill: return rcode_kill;
      }
    default:
      printf("Unrecognised Command On Line %u!\n", (*sl)->cmd.LineNum);
    exit(1);
  }
}

RCode update_env(char* input, size_t size) {
  ParseResult ln = {{input, input, size}, 0};

  /* Blank line - return nothing */
  if (input[0] == '\n') return rcode_continue;
  
  if (!strncmp(input, "KILL", 4)) {
    free(input);
    return rcode_kill;
  }
  
  /* Line number given => store uncompiled */
  if (parse_unsigned(&ln)) {
    UncompiledCommand* uCmd =
      (UncompiledCommand*)malloc(sizeof(UncompiledCommand));
    CommandToken cmd;
    cmd.Details.Id = cmd_null;
    cmd.Details.Command.Uncompiled = uCmd;
    *uCmd = ln.Cmd;
    cmd.LineNum = *((UInt*)ln.Value);
    free(ln.Value);
    statements = insert_cmd(cmd, statements);
    return rcode_continue;
  }

  /* Line number not given => run command */
  UncompiledCommand uCmd = {input, input, size};
  CommandToken cmd;
  RCode r;
  cmd.Details.Id = cmd_null;
  cmd.Details.Command.Uncompiled = &uCmd;
  r = run_command(0, &cmd.Details);
  free(input);
  if (r != rcode_kill) return rcode_return;
  else return r;
}

void list_registers() {
  for (int i = 0; i < 26; i++) {
    printf("Reg %c: %u\n", i + 'a', variables[i]);
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
    case rcode_end:
      printf("? nothing to end\nREADY\n");
      goto start;
    case rcode_stop:
    case rcode_kill:
      printf("Goodbye\n");

  }
  free_all_statements();
  list_registers();
  free(memory.UInt);
}
