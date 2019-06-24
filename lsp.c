#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "io.h"
#include "minic.h"
#include "lsp.h"
#define MAX_HEADER_FIELD_LEN 100

void lsp_event_loop(void) {
  for(;;) {
    unsigned long content_length = lsp_parse_header();
    cJSON *request = lsp_parse_content(content_length);
    json_rpc(request);
    cJSON_Delete(request);
  }
}

unsigned long lsp_parse_header(void) {
  char buffer[MAX_HEADER_FIELD_LEN];
  unsigned long content_length = 0;

  for(;;) {
    fgets(buffer, MAX_HEADER_FIELD_LEN, stdin);
    if(strcmp(buffer, "\r\n") == 0) { // End of header
      if(content_length == 0)
        exit(1);
      return content_length;
    }

    char *buffer_part = strtok(buffer, " ");
    if(strcmp(buffer_part, "Content-Length:") == 0) {
      buffer_part = strtok(NULL, "\n");
      content_length = atoi(buffer_part);
    }
  }
}

cJSON* lsp_parse_content(unsigned long content_length) {
  char *buffer = (char *) malloc(content_length + 1);
  if(buffer == NULL)
    exit(2);
  size_t read_elements = fread(buffer, 1, content_length, stdin);
  if(read_elements != content_length) {
    free(buffer);
    exit(3);
  }
  buffer[content_length] = '\0';

  cJSON *request = cJSON_Parse(buffer);

  free(buffer);
  if(request == NULL)
    exit(4);
  return request;
}

void json_rpc(const cJSON *request) {
  // Logs for debugging
  /*
  char *log_output = cJSON_Print(request);
  FILE *log = fopen("lsp.log", "a");
  fwrite(log_output, 1, strlen(log_output), log);
  fclose(log);
  */
  // <== Logs for debugging

  const char *method;
  int id;

  const cJSON *method_json = cJSON_GetObjectItem(request, "method");
  if(cJSON_IsString(method_json) && (method_json->valuestring != NULL)) {
    method = method_json->valuestring;
  }
  const cJSON *id_json = cJSON_GetObjectItem(request, "id");
  if(cJSON_IsNumber(id_json)) {
    id = id_json->valueint;
  }
  const cJSON *params_json = cJSON_GetObjectItem(request, "params");

  // RPC
  if(strcmp(method, "initialize") == 0) {
    lsp_initialize(id);
  }
  else if(strcmp(method, "shutdown") == 0) {
    lsp_shutdown(id);
  }
  else if(strcmp(method, "exit") == 0) {
    lsp_exit();
  }
  else if(strcmp(method, "textDocument/didOpen") == 0) {
    lsp_text_sync("didOpen", params_json);
  }
  else if(strcmp(method, "textDocument/didChange") == 0) {
    lsp_text_sync("didChange", params_json);
  }
  else if(strcmp(method, "textDocument/hover") == 0) {
    lsp_hover(id, params_json);
  }
  else if(strcmp(method, "textDocument/definition") == 0) {
    lsp_goto_definition(id, params_json);
  }
  else if(strcmp(method, "textDocument/completion") == 0) {
    lsp_completion(id, params_json);
  }
}

void lsp_send_response(int id, cJSON *result) {
  cJSON *response = cJSON_CreateObject();
  cJSON_AddStringToObject(response, "jsonrpc", "2.0");
  cJSON_AddNumberToObject(response, "id", id);
  if(result != NULL)
    cJSON_AddItemToObject(response, "result", result);

  char *output = cJSON_Print(response);
  printf("Content-Length: %lu\r\n\r\n", strlen(output));
  fwrite(output, 1, strlen(output), stdout);
  fflush(stdout);
  free(output);
  cJSON_Delete(response);
}

void lsp_send_notification(const char *method, cJSON *params) {
  cJSON *response = cJSON_CreateObject();
  cJSON_AddStringToObject(response, "jsonrpc", "2.0");
  cJSON_AddStringToObject(response, "method", method);
  if(params != NULL)
    cJSON_AddItemToObject(response, "params", params);

  char *output = cJSON_Print(response);
  printf("Content-Length: %lu\r\n\r\n", strlen(output));
  fwrite(output, 1, strlen(output), stdout);
  fflush(stdout);
  free(output);
  cJSON_Delete(response);
}

// **************
// RPC functions:
// **************

void lsp_initialize(int id) {
  cJSON *result = cJSON_CreateObject();
  cJSON *capabilities = cJSON_CreateObject();
  cJSON_AddNumberToObject(capabilities, "textDocumentSync", 1);
  cJSON_AddBoolToObject(capabilities, "hoverProvider", 1);
  cJSON_AddBoolToObject(capabilities, "definitionProvider", 1);
  cJSON *completion = cJSON_AddObjectToObject(capabilities, "completionProvider");
  cJSON_AddBoolToObject(completion, "resolveProvider", 0);

  // TODO add other capabilities

  cJSON_AddItemToObject(result, "capabilities", capabilities);
  lsp_send_response(id, result);
}

void lsp_shutdown(int id) {
  lsp_send_response(id, NULL);
}

void lsp_exit(void) {
  exit(0);
}

void lsp_text_sync(const char *method, const cJSON *params_json) {
  const cJSON *text_document_json = cJSON_GetObjectItem(params_json, "textDocument");

  const cJSON *uri_json = cJSON_GetObjectItem(text_document_json, "uri");
  const char *uri = NULL;
  if(cJSON_IsString(uri_json) && (uri_json->valuestring != NULL)) {
    uri = uri_json->valuestring;
  }

  const char *text = NULL;
  if(strcmp(method, "didOpen") == 0) {
    const cJSON *text_json = cJSON_GetObjectItem(text_document_json, "text");
    if(cJSON_IsString(text_json) && (text_json->valuestring != NULL)) {
      text = text_json->valuestring;
    }
  }
  else if(strcmp(method, "didChange") == 0) {
    const cJSON *content_changes_json = cJSON_GetObjectItem(params_json, "contentChanges");
    const cJSON *content_change_json = cJSON_GetArrayItem(content_changes_json, 0);
    const cJSON *text_json = cJSON_GetObjectItem(content_change_json, "text");
    if(cJSON_IsString(text_json) && (text_json->valuestring != NULL)) {
      text = text_json->valuestring;
    }
  }

  if(uri == NULL || text == NULL) {
    return;
  }
  lsp_lint(uri, text);
}

void lsp_lint(const char *uri, const char *text) {
  cJSON *params = cJSON_CreateObject();
  cJSON_AddStringToObject(params, "uri", uri);
  cJSON *diagnostics = cJSON_AddArrayToObject(params, "diagnostics");
  parse(diagnostics, text);
  lsp_send_notification("textDocument/publishDiagnostics", params);
}

void lsp_hover(int id, const cJSON *params_json) {
  const cJSON *text_document_json = cJSON_GetObjectItem(params_json, "textDocument");
  const cJSON *uri_json = cJSON_GetObjectItem(text_document_json, "uri");
  const char *uri = NULL;
  if(cJSON_IsString(uri_json) && (uri_json->valuestring != NULL)) {
    uri = uri_json->valuestring;
  }

  const cJSON *position_json = cJSON_GetObjectItem(params_json, "position");
  const cJSON *line_json = cJSON_GetObjectItem(position_json, "line");
  int line;
  if(cJSON_IsNumber(line_json)) {
    line = line_json->valueint;
  }
  int character;
  const cJSON *character_json = cJSON_GetObjectItem(position_json, "character");
  if(cJSON_IsNumber(character_json)) {
    character = character_json->valueint;
  }

  char *text = read_to_string(uri);
  truncate_string(text, line, character);
  const char *symbol_name  = extract_last_symbol(text);
  char *contents = symbol_info(symbol_name, text);
  free(text);

  if(contents == NULL) {
    lsp_send_response(id, NULL);
    return;
  }
  cJSON *result = cJSON_CreateObject();
  cJSON_AddStringToObject(result, "contents", contents);
  lsp_send_response(id, result);
}

void lsp_goto_definition(int id, const cJSON *params_json) {
  const cJSON *text_document_json = cJSON_GetObjectItem(params_json, "textDocument");
  const cJSON *uri_json = cJSON_GetObjectItem(text_document_json, "uri");
  const char *uri = NULL;
  if(cJSON_IsString(uri_json) && (uri_json->valuestring != NULL)) {
    uri = uri_json->valuestring;
  }

  const cJSON *position_json = cJSON_GetObjectItem(params_json, "position");
  const cJSON *line_json = cJSON_GetObjectItem(position_json, "line");
  int line;
  if(cJSON_IsNumber(line_json)) {
    line = line_json->valueint;
  }
  int character;
  const cJSON *character_json = cJSON_GetObjectItem(position_json, "character");
  if(cJSON_IsNumber(character_json)) {
    character = character_json->valueint;
  }

  char *text = read_to_string(uri);
  truncate_string(text, line, character);
  const char *symbol_name  = extract_last_symbol(text);
  int jump_line = symbol_location(symbol_name, text);
  free(text);

  if(jump_line == -1) {
    lsp_send_response(id, NULL);
    return;
  }
  cJSON *result = cJSON_CreateObject();
  cJSON_AddStringToObject(result, "uri", uri);
  cJSON *range = cJSON_AddObjectToObject(result, "range");
  cJSON *start_position = cJSON_AddObjectToObject(range, "start");
  cJSON_AddNumberToObject(start_position, "line", jump_line);
  cJSON_AddNumberToObject(start_position, "character", 0);
  cJSON *end_position = cJSON_AddObjectToObject(range, "end");
  cJSON_AddNumberToObject(end_position, "line", jump_line);
  cJSON_AddNumberToObject(end_position, "character", 0);
  lsp_send_response(id, result);
}

void lsp_completion(int id, const cJSON *params_json) {
  const cJSON *text_document_json = cJSON_GetObjectItem(params_json, "textDocument");
  const cJSON *uri_json = cJSON_GetObjectItem(text_document_json, "uri");
  const char *uri = NULL;
  if(cJSON_IsString(uri_json) && (uri_json->valuestring != NULL)) {
    uri = uri_json->valuestring;
  }

  const cJSON *position_json = cJSON_GetObjectItem(params_json, "position");
  const cJSON *line_json = cJSON_GetObjectItem(position_json, "line");
  int line;
  if(cJSON_IsNumber(line_json)) {
    line = line_json->valueint;
  }
  int character;
  const cJSON *character_json = cJSON_GetObjectItem(position_json, "character");
  if(cJSON_IsNumber(character_json)) {
    character = character_json->valueint;
  }

  char *text = read_to_string(uri);
  truncate_string(text, line, character);
  const char *symbol_name_part  = extract_last_symbol(text);
  cJSON *result = symbol_completion(symbol_name_part, text);
  free(text);

  lsp_send_response(id, result);
}
