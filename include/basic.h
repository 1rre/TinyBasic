#ifndef header_basic
#define header_basic
#include <stdlib.h>

void notimpl(char*);

typedef u_int32_t UInt;
/* 2^32 - 2 */
#define UINT_MAX ((UInt)4294967294u)


typedef enum {
  op_add, /* ADD */
  op_sub, /* SUBtract */
  op_mul, /* MULtiply*/
  op_div, /* DIVide */
  op_rem, /* REMainder */
  op_eqs, /* EQualS */
  op_neq, /* Not EQuals */
  op_gtt, /* GreaTer Than */
  op_lst, /* LeSs Than */
  op_geq, /* Greater than or EQual to */
  op_leq, /* Less than or EQual to */
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

typedef struct {
  struct ValueToken* Left;
  struct ValueToken* Right;
  BinOpcode Op;
} BinOp;

typedef struct {
  struct ValueToken* Value;
  FunOpcode Op;
} FunCall;

typedef struct {
  u_int8_t name; /* a..z, A..Z */
} Register;

typedef struct {
  struct ValueToken* Reference;
} Memory;

typedef union ValueDetails {
  Register Register;
  Memory Memory;
  BinOp BinOp;
  FunCall FunCall;
  UInt IntLiteral;
  char* String;
} ValueDetails;

typedef enum {
  value_null,
  value_register,
  value_memory,
  value_int,
  value_string
} ValueId;

typedef struct {
  ValueDetails Details;
  ValueId Id;
} ValueToken;

typedef struct {
  ValueToken* Predicate;
} IfCommand;

typedef struct {
  ValueToken* Predicate;
} WhileCommand;

typedef struct {
  ValueToken* Predicate;
} UntilCommand;

typedef struct {

} ForCommand;

typedef struct {

} InputCommand;

typedef struct {

} ListCommand;

typedef struct {

} RunCommand;

typedef struct {

} ReturnCommand;

typedef struct {

} LetCommand;

typedef struct {

} GotoCommand;

typedef struct {

} CallCommand;

typedef struct {

} StopCommand;  

typedef struct {

} EndCommand;

typedef struct {
  /* No Contents (for now) */
} NoteCommand;

typedef struct {
  struct CommandDetails* Left;
  struct CommandDetails* Right;
} MultipleCommand;

typedef struct {
  char* Contents;
  size_t Size;
} UncompiledCommand;

typedef union {
  IfCommand If;
  WhileCommand While;
  UntilCommand Until;
  ForCommand For;
  InputCommand Input;
  ListCommand List;
  RunCommand Run;
  ReturnCommand Return;
  LetCommand Let;
  GotoCommand Goto;
  CallCommand Call;
  StopCommand Stop;
  EndCommand End;
  NoteCommand Note;
  MultipleCommand Multiple;
  UncompiledCommand Uncompiled;
} Command;

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
  cmd_multiple,
  cmd_note, /* Used over `REM` to simplify parsing */
  cmd_return,
  cmd_run,
  cmd_stop,
  cmd_until,
  cmd_while
} CommandId;

typedef struct CommandDetails {
  CommandId Id;
  Command Command;
} CommandDetails;

typedef struct {
  UInt LineNum;
  CommandDetails Details;
} CommandToken;

typedef struct {
  char* position;
  void* value;
} ParseResult;

void run_interpreter(void);
void free_command(CommandDetails);
void free_value(ValueToken);

CommandDetails* parse_command(UncompiledCommand);

ParseResult parse_linenum(char*, size_t);

#endif /* header_basic */