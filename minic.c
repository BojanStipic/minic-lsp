#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "defs.h"
#include "symtab.h"
#include "minic.h"

extern int yylineno;
int yyparse(void);
typedef struct yy_buffer_state * YY_BUFFER_STATE;
YY_BUFFER_STATE yy_scan_string(const char *str);
void yy_delete_buffer(YY_BUFFER_STATE buffer);

char char_buffer[CHAR_BUFFER_LENGTH];
int severity = ERROR;
const char *types_str[] = { "void", "int", "unsigned int" };

cJSON *_diagnostics = NULL;

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
  _diagnostics = NULL;
}

char* symbol_info(const char *symbol_name, const char *text) {
  parse(NULL, text);
  int idx = lookup_symbol(symbol_name, VAR|PAR|FUN);
  if(idx == -1) {
    return NULL;
  }
  const char *type = types_str[get_type(idx)];
  const char *name = get_name(idx);
  const char *par_type = "";
  if(get_kind(idx) == FUN) {
    if(get_atr1(idx) == 1) {
      par_type = types_str[get_atr2(idx)];
    }
  }

  char *info = malloc(strlen(type) + strlen(name) + strlen(par_type) + 4);
  strcpy(info, type);
  strcat(info, " ");
  strcat(info, name);
  if(get_kind(idx) == FUN) {
    strcat(info, "(");
    strcat(info, par_type);
    strcat(info, ")");
  }
  return info;
}

int symbol_location(const char *symbol_name, const char *text) {
  parse(NULL, text);
  int idx = lookup_symbol(symbol_name, VAR|PAR|FUN);
  return get_lineno(idx);
}

cJSON* symbol_completion(const char *symbol_name_part, const char *text) {
  parse(NULL, text);
  int indices[SYMBOL_TABLE_LENGTH];
  int indices_num = lookup_starts_with(indices, symbol_name_part);

  cJSON *results = cJSON_CreateArray();
  for(int i = 0; i < indices_num; i++) {
    cJSON *item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "label", get_name(indices[i]));
    //cJSON_AddStringToObject(item, "detail", "");
    cJSON_AddItemToArray(results, item);
  }
  return results;
}
