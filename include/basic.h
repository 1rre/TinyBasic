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
  /* No Contents */
} RemCommand;

typedef struct {
  struct CommandDetails* contents;
  UInt Size;
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
  RemCommand Rem;
  MultipleCommand Multiple;
  UncompiledCommand Uncompiled;
} Command;

typedef enum {
  id_null,
  id_if,
  id_while,
  id_until,
  id_for,
  id_input,
  id_list,
  id_run,
  id_return,
  id_let,
  id_goto,
  id_call,
  id_stop,
  id_end,
  id_rem,
  id_multip
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