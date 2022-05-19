#ifndef header_basic
#define header_basic
#include <stdlib.h>

void notimpl(char*);

typedef unsigned short UInt;

typedef struct {
  
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
  id_null,
  id_call,
  id_end,
  id_for,
  id_goto,
  id_if,
  id_input,
  id_let,
  id_list,
  id_multip,
  id_note, /* Used over `REM` to simplify parsing */
  id_return,
  id_run,
  id_stop,
  id_until,
  id_while
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

#endif /* header_basic */