#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "basic.h"

#define WITH_FAILURE(x)     \
ParseResult backup = *rtn;  \
x                           \
failure:                    \
  *rtn = backup;            \
  return 0;

#define FAIL() goto failure

UInt is_whitespace(char c) {
  return c == ' ' || c == '\t';
}

UInt is_digit(char c) {
  return c >= '0' && c <= '9';
}

UInt is_end_char(char c) {
  return c == '\0' || c == '\n' || c == '\r';
}

void skip_whitespace(UncompiledCommand* cmd) {
  while (cmd->Size && is_whitespace(cmd->Contents[0])) {
    cmd->Size--, cmd->Contents++;
  }
}

UInt assert_at_end(ParseResult* rtn) {
  skip_whitespace(&rtn->Cmd);
  return is_end_char(*rtn->Cmd.Contents);
}

UInt parse_value(ParseResult*);

UInt parse_bracketed(ParseResult* rtn) {
  WITH_FAILURE (
    skip_whitespace(&rtn->Cmd);
    if (!rtn->Cmd.Size) FAIL();
    if (rtn->Cmd.Contents[0] != '(') FAIL();
    rtn->Cmd.Contents++;
    rtn->Cmd.Size--;
    if (!parse_value(rtn)) FAIL();
    skip_whitespace(&rtn->Cmd);
    if (!rtn->Cmd.Size) FAIL();
    if (rtn->Cmd.Contents[0] != ')') FAIL();
    rtn->Cmd.Contents++;
    rtn->Cmd.Size--;
    return 1;
  )
}

UInt parse_unsigned(ParseResult* rtn) {
  WITH_FAILURE(
    UInt s = 0;

    skip_whitespace(&rtn->Cmd);
    if (!rtn->Cmd.Size) FAIL();
    if (!is_digit(rtn->Cmd.Contents[0])) FAIL();

    while (rtn->Cmd.Size && is_digit(rtn->Cmd.Contents[0])) {
      if (s > MAX_LOCAL_INT / 10) FAIL();
      s *= 10;
      if (MAX_LOCAL_INT - (rtn->Cmd.Contents[0] - '0') < s) FAIL();
      s += rtn->Cmd.Contents[0] - '0';

      rtn->Cmd.Contents++;
      rtn->Cmd.Size--;
    }

    rtn->Value = malloc(sizeof(UInt));
    *((UInt*)rtn->Value) = s;
    return 1;
  )
}

UInt parse_register(ParseResult* rtn) {
  WITH_FAILURE (
    
  )
}

UInt parse_address(ParseResult* rtn) {
  WITH_FAILURE (
    
  )
}

typedef struct {char* Symbol; BinOpcode Code;} SymbolSpec;

UInt pick_symbol(
    SymbolSpec* active,
    SymbolSpec* symbols,
    UInt n_symbols,
    ParseResult* rtn
  ) {
  for (UInt i = 0; i < n_symbols; i++) {
    SymbolSpec symbol = symbols[i];
    if (
      rtn->Cmd.Size > strlen(symbol.Symbol) &&
      !strncmp(symbol.Symbol, rtn->Cmd.Contents, strlen(symbol.Symbol))
    ) {
      *active = symbol;
      return 1;
    }
  }
  return 0;
}

UInt parse_binop (
    UInt try_next_op(ParseResult*),
    SymbolSpec* symbols,
    UInt n_symbols,
    ParseResult* rtn
  ) { WITH_FAILURE (
    SymbolSpec active;
    if (!try_next_op(rtn)) FAIL();
    skip_whitespace(&rtn->Cmd);
    while (pick_symbol(&active, symbols, n_symbols, rtn)) {
      struct ValueToken* left = (struct ValueToken*)rtn->Value;
      struct ValueToken* right;
      rtn->Cmd.Contents++;
      rtn->Cmd.Size--;
      if (!try_next_op(rtn)) {
        /* Beyond here, it is a known syntax error *
         * As we have passed the op symbol         */
        free_value((ValueToken*)left);
        FAIL();
      }
      right = (struct ValueToken*)rtn->Value;
      rtn->Value = malloc(sizeof(ValueToken));
      ((ValueToken*)rtn->Value)->Id = value_binop;
      ((ValueToken*)rtn->Value)->Details =
        (ValueDetails*)malloc(sizeof(ValueDetails));
      ((ValueToken*)rtn->Value)->Details->BinOp.Op = active.Code;
      ((ValueToken*)rtn->Value)->Details->BinOp.Left = left;
      ((ValueToken*)rtn->Value)->Details->BinOp.Right = right;
      skip_whitespace(&rtn->Cmd);
    }
    return 1;
  )
}

UInt parse_memory(ParseResult* rtn) {
  return parse_register(rtn) || parse_address(rtn);
}

UInt parse_non_op(ParseResult* rtn) {
  if (parse_unsigned(rtn)) {
    UInt u = *(UInt*)rtn->Value;
    free(rtn->Value);
    rtn->Value = malloc(sizeof(ValueToken));
    ((ValueToken*)rtn->Value)->Id = value_int;
    ((ValueToken*)rtn->Value)->Details = (ValueDetails*)malloc(sizeof(ValueDetails));
    ((ValueToken*)rtn->Value)->Details->IntLiteral = u;
    return 1;
  }
  return parse_bracketed(rtn) || parse_memory(rtn);
}

/* Operators ·, / & % */
UInt parse_multiplicative(ParseResult* rtn) {
  SymbolSpec symbols[] = {
    {"*", op_mul}, {"/", op_div}, {"%", op_rem}
  };
  return parse_binop(parse_non_op, symbols, 3, rtn);
}

/* Operators + & - */
UInt parse_additive(ParseResult* rtn) {
  SymbolSpec symbols[] = {
    {"+", op_add}, {"-", op_sub}
  };
  return parse_binop(parse_multiplicative, symbols, 2, rtn);
}

/* Operators <, ≤, > & ≥ */
UInt parse_comparative(ParseResult* rtn) {
  SymbolSpec symbols[] = {
    {">=", op_geq}, {"<=", op_leq}, {">", op_gtt}, {"<", op_lst}
  };
  return parse_binop(parse_additive, symbols, 4, rtn);
}

/* Operators = and ≠ */
UInt parse_equality(ParseResult* rtn) {
  SymbolSpec symbols[] = {{"=", op_eqs}, {"!=", op_neq}};
  return parse_binop(parse_comparative, symbols, 2, rtn);
}

/* Operator 'xor' */
UInt parse_bxor(ParseResult* rtn) {
  SymbolSpec symbols[] = {{"xor", op_bxr}};
  return parse_binop(parse_equality, symbols, 1, rtn);
}

/* Operator 'and' */
UInt parse_bor(ParseResult* rtn) {
  SymbolSpec symbols[] = {{"and", op_bnd}};
  return parse_binop(parse_bxor, symbols, 1, rtn);
}

/* Operator 'or' */
UInt parse_band(ParseResult* rtn) {
  SymbolSpec symbols[] = {{"or", op_bor}};
  return parse_binop(parse_bor, symbols, 1, rtn);
}

/* Operator '|' */
UInt parse_lor(ParseResult* rtn) {
  SymbolSpec symbols[] = {{"|", op_lor}};
  return parse_binop(parse_band, symbols, 1, rtn);
}

/* Operator '&' */
UInt parse_land(ParseResult* rtn) {
  SymbolSpec symbols[] = {{"&", op_lnd}};
  return parse_binop(parse_lor, symbols, 1, rtn);
}

UInt parse_value(ParseResult* rtn) {
  return parse_land(rtn);
}

UInt check_ident(const char* ident, ParseResult* rtn) {
  if (*ident && rtn->Cmd.Size) {
    return
      *ident == *rtn->Cmd.Contents &&
      (rtn->Cmd.Contents++, rtn->Cmd.Size--, check_ident(ident+1, rtn));
  } else return !(*ident);
}

UInt parse_call(ParseResult* rtn) {
  WITH_FAILURE (
    if (!check_ident("call", rtn)) FAIL();
    
  )
}
UInt parse_for(ParseResult* rtn) {
  WITH_FAILURE (
    if (!check_ident("for", rtn)) FAIL();
    
  )
}
UInt parse_goto(ParseResult* rtn) {
  WITH_FAILURE (
    if (!check_ident("goto", rtn)) FAIL();
  )
}
UInt parse_if(ParseResult* rtn) {
  WITH_FAILURE (
    if (!check_ident("if", rtn)) FAIL();
    
  )
}
UInt parse_input(ParseResult* rtn) {
  WITH_FAILURE (
    if (!check_ident("input", rtn)) FAIL();
    
  )
}
UInt parse_let(ParseResult* rtn) {
  WITH_FAILURE (
    if (!check_ident("let", rtn)) FAIL();
    
  )
}
UInt parse_list(ParseResult* rtn) {
  WITH_FAILURE (
    ValueToken* v;
    if (!check_ident("list", rtn)) FAIL();
    skip_whitespace(&rtn->Cmd);
    if (!parse_value(rtn)) FAIL();
    v = (ValueToken*)rtn->Value;
    if (!assert_at_end(rtn)) {
      free_value(v);
      FAIL();
    }
    rtn->Value = malloc(sizeof(CommandDetails));
    ((CommandDetails*)rtn->Value)->Id = cmd_list;
    ((CommandDetails*)rtn->Value)->Command.Value = v;
    return 1;    
  )
}
UInt parse_note(ParseResult* rtn) {
  WITH_FAILURE (
    if (!check_ident("note", rtn)) FAIL();
    rtn->Value = malloc(sizeof(CommandDetails));
    ((CommandDetails*)rtn->Value)->Id = cmd_note;
    return 1;
  )
}
UInt parse_print(ParseResult* rtn) {
  WITH_FAILURE (
    if (!check_ident("print", rtn)) FAIL();
    
  )
}
UInt parse_until(ParseResult* rtn) {
  WITH_FAILURE (
    if (!check_ident("until", rtn)) FAIL();
    
  )
}
UInt parse_while(ParseResult* rtn) {
  WITH_FAILURE (
    if (!check_ident("while", rtn)) FAIL();
    
  )
}
UInt parse_end(ParseResult* rtn) {
  WITH_FAILURE (
    if (!check_ident("end", rtn)) FAIL();
    if (!assert_at_end(rtn)) FAIL();
    rtn->Value = malloc(sizeof(CommandDetails));
    ((CommandDetails*)rtn->Value)->Id = cmd_end;
    return 1;
  )
}
UInt parse_return(ParseResult* rtn) {
  WITH_FAILURE (
    if (!check_ident("return", rtn)) FAIL();
    if (!assert_at_end(rtn)) FAIL();
    rtn->Value = malloc(sizeof(CommandDetails));
    ((CommandDetails*)rtn->Value)->Id = cmd_return;
    return 1;
  )
}
UInt parse_run(ParseResult* rtn) {
  WITH_FAILURE (
    if (!check_ident("run", rtn)) FAIL();
    if (!assert_at_end(rtn)) FAIL();
    rtn->Value = malloc(sizeof(CommandDetails));
    ((CommandDetails*)rtn->Value)->Id = cmd_run;
    return 1;
  )
}
UInt parse_stop(ParseResult* rtn) {
  WITH_FAILURE (
    if (!check_ident("stop", rtn)) FAIL();
    if (!assert_at_end(rtn)) FAIL();
    rtn->Value = malloc(sizeof(CommandDetails));
    ((CommandDetails*)rtn->Value)->Id = cmd_stop;
    return 1;
  )
}

UInt parse_line(ParseResult* rtn) {
  skip_whitespace(&rtn->Cmd);
  if (rtn->Cmd.Size <= 0) goto syntax_error;
  if (
    parse_call(rtn) ||
    parse_for(rtn) ||
    parse_goto(rtn) ||
    parse_if(rtn) ||
    parse_input(rtn) ||
    parse_let(rtn) ||
    parse_list(rtn) ||
    parse_note(rtn) ||
    parse_print(rtn) ||
    parse_run(rtn) ||
    parse_return(rtn) ||
    parse_stop(rtn) ||
    parse_while(rtn) ||
    parse_until(rtn)
  ) return 1;
syntax_error:
  printf("? syntax error");
  return 0;
}
