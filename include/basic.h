#ifndef header_basic
#define header_basic
#include <stdlib.h>
#include <stdint.h>

#ifdef _WIN32
  #define strncasecmp(x,y,z) _strnicmp(x,y,z)
#endif

void notimpl(char*);

typedef uint32_t UInt;
/* 2^32 - 2 */
#define MAX_LOCAL_INT ((UInt)4294967294u)


typedef enum {
  op_add, /* ADD */
  op_sub, /* SUBtract */
  op_mul, /* MULtiply*/
  op_div, /* DIVide */
  op_rem, /* REMainder */
  op_gtt, /* GreaTer Than */
  op_lst, /* LeSs Than */
  op_geq, /* Greater than or EQual to */
  op_leq, /* Less than or EQual to */
  op_eqs, /* EQualS */
  op_neq, /* Not EQuals */
  op_bor, /* Bitwise OR */
  op_bnd, /* Bitwise aND */
  op_bxr, /* Bitwise XoR */
  op_lor, /* Logical OR */
  op_lnd  /* Logical aND */
} BinOpcode;

typedef enum {
  fun_neg, /* NEGate */
  fun_abs, /* ABSolute value */
  fun_inv, /* INVert */
  fun_not, /* Logical Not */
  fun_rdm, /* RanDoM */
  fun_pos, /* cursor POSition */
  fun_sin, /* SINe */
  fun_cos, /* COSine */
  fun_tan, /* TANgent */
  fun_asn, /* ArcSiN */
  fun_acs, /* ArcCoS */
  fun_atn, /* ArcTaNgent */
  fun_log, /* LOGarithm (base e) */
  fun_sig, /* SIGnum */
  fun_exp, /* EXPonential (to base e) */
  fun_sqt  /* SQuare rooT */
} FunOpcode;

typedef enum {
  value_null,
  value_register,
  value_memory,
  value_int,
  value_string,
  value_binop,
  value_funcall
} ValueId;

typedef union {
  uint8_t Register;
  struct ValueToken* Reference;
  struct BinOp* BinOp;
  struct FunCall* FunCall;
  UInt IntLiteral;
  char* String;
} ValueDetails;

typedef struct ValueToken {
  ValueDetails Details;
  ValueId Id;
} ValueToken;

typedef struct BinOp {
  ValueToken Left;
  ValueToken Right;
  BinOpcode Op;
} BinOp;

typedef struct {
  ValueToken Value;
  FunOpcode Op;
} FunCall;


typedef enum {
  cmd_null,
  cmd_call,
  cmd_end,
  cmd_for,
  cmd_goto,
  cmd_if,
  cmd_input,
  cmd_let,
  cmd_list,
  cmd_print,
  cmd_multiple,
  cmd_note, /* Used over `REM` to simplify parsing */
  cmd_return,
  cmd_run,
  cmd_stop,
  cmd_until,
  cmd_while
} CommandId;

typedef union {
  /* Call, Goto, If, Run, Until, While */
  struct ValueToken* Value;
  struct UncompiledCommand* Uncompiled;
  struct MultipleCommand* Multiple;
  struct InputCommand* Input;
  struct LetCommand* Let;
  struct ForCommand* For;
} Command;

typedef struct {
  CommandId Id;
  Command Command;
} CommandDetails;

typedef struct ForCommand {
  ValueToken Lo;
  ValueToken Hi;
  ValueToken Step;
} ForCommand;

typedef struct InputCommand {
  char* Prompt;
  ValueToken To;
} InputCommand;

typedef struct LetCommand {
  ValueToken Memory;
  ValueToken Value;
} LetCommand;

typedef struct MultipleCommand {
  CommandDetails Left;
  CommandDetails Right;
} MultipleCommand;

typedef struct UncompiledCommand {
  char* Contents;
  char* FullLine;
  size_t Size;
} UncompiledCommand;

typedef struct {
  UInt LineNum;
  CommandDetails Details;
} CommandToken;

typedef struct {
  struct UncompiledCommand Cmd;
  void* Value;
} ParseResult;

void run_interpreter(void);
void free_command(CommandDetails);
void free_value(ValueToken);

UInt parse_unsigned(ParseResult*);
UInt parse_line(ParseResult*);

#endif /* header_basic */
