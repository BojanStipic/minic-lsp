%{
  #include <stdio.h>
  #include <stdbool.h>
  #include "defs.h"
  #include "symtab.h"
  #include <cjson/cJSON.h>
  #include "lsp.h"

  int yyparse(void);
  int yylex(void);
  int yyerror(const char *text);
  typedef struct yy_buffer_state * YY_BUFFER_STATE;
  YY_BUFFER_STATE yy_scan_string(const char *str);
  void yy_delete_buffer(YY_BUFFER_STATE buffer);

  extern int yylineno;
  char char_buffer[CHAR_BUFFER_LENGTH];
  cJSON *_diagnostics = NULL;
  int severity;
  int var_num = 0;
  int fun_idx = -1;
  int fcall_idx = -1;
%}

%union {
  int i;
  char *s;
}

%token <i> _TYPE
%token _IF
%token _ELSE
%token _RETURN
%token <s> _ID
%token <s> _INT_NUMBER
%token <s> _UINT_NUMBER
%token _LPAREN
%token _RPAREN
%token _LBRACKET
%token _RBRACKET
%token _ASSIGN
%token _SEMICOLON
%token <i> _AROP
%token <i> _RELOP

%type <i> type num_exp exp literal
%type <i> function_call argument rel_exp

%nonassoc ONLY_IF
%nonassoc _ELSE

%%

program
  : function_list
      {
        int idx = lookup_symbol("main", FUN);
        if(idx == -1)
          err("undefined reference to 'main'");
        else
          if(get_type(idx) != INT)
            warn("return type of 'main' is not int");
      }
  ;

function_list
  : function
  | function_list function
  ;

function
  : type _ID
      {
        fun_idx = lookup_symbol($2, FUN);
        if(fun_idx == -1)
          fun_idx = insert_symbol($2, FUN, $1, NO_ATR, NO_ATR);
        else
          err("redefinition of function '%s'", $2);
      }
    _LPAREN parameter _RPAREN body
      {
        clear_symbols(fun_idx + 1);
        var_num = 0;
      }
  ;

type
  : _TYPE
      { $$ = $1; }
  ;

parameter
  : /* empty */
      { set_atr1(fun_idx, 0); }

  | type _ID
      {
        insert_symbol($2, PAR, $1, 1, NO_ATR);
        set_atr1(fun_idx, 1);
        set_atr2(fun_idx, $1);
      }
  ;

body
  : _LBRACKET variable_list statement_list _RBRACKET
  ;

variable_list
  : /* empty */
  | variable_list variable
  ;

variable
  : type _ID _SEMICOLON
      {
        if(lookup_symbol($2, VAR|PAR) == -1)
           insert_symbol($2, VAR, $1, ++var_num, NO_ATR);
        else
           err("redefinition of '%s'", $2);
      }
  ;

statement_list
  : /* empty */
  | statement_list statement
  ;

statement
  : compound_statement
  | assignment_statement
  | if_statement
  | return_statement
  ;

compound_statement
  : _LBRACKET statement_list _RBRACKET
  ;

assignment_statement
  : _ID _ASSIGN num_exp _SEMICOLON
      {
        int idx = lookup_symbol($1, VAR|PAR);
        if(idx == -1)
          err("invalid lvalue '%s' in assignment", $1);
        else
          if(get_type(idx) != get_type($3))
            err("incompatible types in assignment");
      }
  ;

num_exp
  : exp
  | num_exp _AROP exp
      {
        if(get_type($1) != get_type($3))
          err("invalid operands: arithmetic operation");
      }
  ;

exp
  : literal
  | _ID
      {
        $$ = lookup_symbol($1, VAR|PAR);
        if($$ == -1)
          err("'%s' undeclared", $1);
      }
  | function_call
  | _LPAREN num_exp _RPAREN
      { $$ = $2; }
  ;

literal
  : _INT_NUMBER
      { $$ = insert_literal($1, INT); }

  | _UINT_NUMBER
      { $$ = insert_literal($1, UINT); }
  ;

function_call
  : _ID
      {
        fcall_idx = lookup_symbol($1, FUN);
        if(fcall_idx == -1)
          err("'%s' is not a function", $1);
      }
    _LPAREN argument _RPAREN
      {
        if(get_atr1(fcall_idx) != (unsigned int) $4)
          err("wrong number of args to function '%s'",
              get_name(fcall_idx));
        set_type(FUN_REG, get_type(fcall_idx));
        $$ = FUN_REG;
      }
  ;

argument
  : /* empty */
    { $$ = 0; }

  | num_exp
    {
      if(get_atr2(fcall_idx) != get_type($1))
        err("incompatible type for argument in '%s'",
            get_name(fcall_idx));
      $$ = 1;
    }
  ;

if_statement
  : if_part %prec ONLY_IF
  | if_part _ELSE statement
  ;

if_part
  : _IF _LPAREN rel_exp _RPAREN statement
  ;

rel_exp
  : num_exp _RELOP num_exp
      {
        if(get_type($1) != get_type($3))
          err("invalid operands: relational operator");
      }
  ;

return_statement
  : _RETURN num_exp _SEMICOLON
      {
        if(get_type(fun_idx) != get_type($2))
          err("incompatible types in return");
      }
  ;

%%

int yyerror(const char *text) {
  if(_diagnostics == NULL) {
    return 0;
  }
  cJSON *diagnostic = cJSON_CreateObject();

  // Range
  cJSON *range = cJSON_AddObjectToObject(diagnostic, "range");
  cJSON *start_position = cJSON_AddObjectToObject(range, "start");
  cJSON_AddNumberToObject(start_position, "line", yylineno);
  cJSON_AddNumberToObject(start_position, "character", 0);
  cJSON *end_position = cJSON_AddObjectToObject(range, "end");
  cJSON_AddNumberToObject(end_position, "line", yylineno + 1);
  cJSON_AddNumberToObject(end_position, "character", 0);
  // Severity
  cJSON_AddNumberToObject(diagnostic, "severity", severity);
  // Message
  cJSON_AddStringToObject(diagnostic, "message", text);

  cJSON_AddItemToArray(_diagnostics, diagnostic);

  severity = ERROR;
  return 0;
}

void parse(cJSON *diagnostics, const char *text) {
  _diagnostics = diagnostics;
  init_symtab();
  yylineno = 0;
  YY_BUFFER_STATE buffer = yy_scan_string(text);
  yyparse();
  yy_delete_buffer(buffer);
  clear_symtab();
  _diagnostics = NULL;
}

int main() {
  lsp_event_loop();
  return 0;
}
