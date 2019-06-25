#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minic.h"
#include "err_codes.h"
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
        exit(EXIT_HEADER_INCOMPLETE);
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
  char *buffer = malloc(content_length + 1);
  if(buffer == NULL)
    exit(EXIT_OUT_OF_MEMORY);
  size_t read_elements = fread(buffer, 1, content_length, stdin);
  if(read_elements != content_length) {
    free(buffer);
    exit(EXIT_IO_ERROR);
  }
  buffer[content_length] = '\0';

  cJSON *request = cJSON_Parse(buffer);

  free(buffer);
  if(request == NULL)
    exit(EXIT_PARSE_ERROR);
  return request;
}

void json_rpc(const cJSON *request) {
  const char *method;
  int id = -1;

  const cJSON *method_json = cJSON_GetObjectItem(request, "method");
  if(!cJSON_IsString(method_json)) {
    exit(EXIT_CONTENT_INCOMPLETE);
  }
  method = method_json->valuestring;

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
    lsp_sync_open(params_json);
  }
  else if(strcmp(method, "textDocument/didChange") == 0) {
    lsp_sync_change(params_json);
  }
  else if(strcmp(method, "textDocument/didClose") == 0) {
    lsp_sync_close(params_json);
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

// *********************
// LSP helper functions:
// *********************

DOCUMENT_LOCATION lsp_parse_document(const cJSON *params_json) {
  DOCUMENT_LOCATION document;

  const cJSON *text_document_json = cJSON_GetObjectItem(params_json, "textDocument");
  const cJSON *uri_json = cJSON_GetObjectItem(text_document_json, "uri");
  document.uri = cJSON_GetStringValue(uri_json);
  if(document.uri == NULL) {
    exit(EXIT_CONTENT_INCOMPLETE);
  }

  const cJSON *position_json = cJSON_GetObjectItem(params_json, "position");
  const cJSON *line_json = cJSON_GetObjectItem(position_json, "line");
  if(!cJSON_IsNumber(line_json)) {
    exit(EXIT_CONTENT_INCOMPLETE);
  }
  document.line = line_json->valueint;
  const cJSON *character_json = cJSON_GetObjectItem(position_json, "character");
  if(!cJSON_IsNumber(character_json)) {
    exit(EXIT_CONTENT_INCOMPLETE);
  }
  document.character = character_json->valueint;

  return document;
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
  cJSON *capabilities = cJSON_AddObjectToObject(result, "capabilities");
  cJSON_AddNumberToObject(capabilities, "textDocumentSync", 1);
  cJSON_AddBoolToObject(capabilities, "hoverProvider", 1);
  cJSON_AddBoolToObject(capabilities, "definitionProvider", 1);
  cJSON *completion = cJSON_AddObjectToObject(capabilities, "completionProvider");
  cJSON_AddBoolToObject(completion, "resolveProvider", 0);

  lsp_send_response(id, result);
}

void lsp_shutdown(int id) {
  lsp_send_response(id, NULL);
}

void lsp_exit(void) {
  exit(0);
}

void lsp_sync_open(const cJSON *params_json) {
  const cJSON *text_document_json = cJSON_GetObjectItem(params_json, "textDocument");

  const cJSON *uri_json = cJSON_GetObjectItem(text_document_json, "uri");
  const char *uri = cJSON_GetStringValue(uri_json);

  const cJSON *text_json = cJSON_GetObjectItem(text_document_json, "text");
  const char *text = cJSON_GetStringValue(text_json);

  if(uri == NULL || text == NULL) {
    exit(EXIT_CONTENT_INCOMPLETE);
  }

  BUFFER buffer = open_buffer(uri, text);
  lsp_lint(buffer);
}

void lsp_sync_change(const cJSON *params_json) {
  const cJSON *text_document_json = cJSON_GetObjectItem(params_json, "textDocument");

  const cJSON *uri_json = cJSON_GetObjectItem(text_document_json, "uri");
  const char *uri = cJSON_GetStringValue(uri_json);

  const cJSON *content_changes_json = cJSON_GetObjectItem(params_json, "contentChanges");
  const cJSON *content_change_json = cJSON_GetArrayItem(content_changes_json, 0);
  const cJSON *text_json = cJSON_GetObjectItem(content_change_json, "text");
  const char *text = cJSON_GetStringValue(text_json);

  if(uri == NULL || text == NULL) {
    exit(EXIT_CONTENT_INCOMPLETE);
  }

  BUFFER buffer = update_buffer(uri, text);
  lsp_lint(buffer);
}

void lsp_sync_close(const cJSON *params_json) {
  const cJSON *text_document_json = cJSON_GetObjectItem(params_json, "textDocument");

  const cJSON *uri_json = cJSON_GetObjectItem(text_document_json, "uri");
  const char *uri = cJSON_GetStringValue(uri_json);

  if(uri == NULL) {
    exit(EXIT_CONTENT_INCOMPLETE);
  }

  close_buffer(uri);
  lsp_lint_clear(uri);
}

void lsp_lint(BUFFER buffer) {
  cJSON *params = cJSON_CreateObject();
  cJSON_AddStringToObject(params, "uri", buffer.uri);
  cJSON *diagnostics = cJSON_AddArrayToObject(params, "diagnostics");
  parse(diagnostics, buffer.content);
  lsp_send_notification("textDocument/publishDiagnostics", params);
}

void lsp_lint_clear(const char *uri) {
  cJSON *params = cJSON_CreateObject();
  cJSON_AddStringToObject(params, "uri", uri);
  cJSON_AddArrayToObject(params, "diagnostics");
  lsp_send_notification("textDocument/publishDiagnostics", params);
}

void lsp_hover(int id, const cJSON *params_json) {
  DOCUMENT_LOCATION document = lsp_parse_document(params_json);

  BUFFER buffer = get_buffer(document.uri);
  char *text = strdup(buffer.content);
  truncate_string(text, document.line, document.character);
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
  DOCUMENT_LOCATION document = lsp_parse_document(params_json);

  BUFFER buffer = get_buffer(document.uri);
  char *text = strdup(buffer.content);
  truncate_string(text, document.line, document.character);
  const char *symbol_name  = extract_last_symbol(text);
  int jump_line = symbol_location(symbol_name, text);
  free(text);

  if(jump_line == -1) {
    lsp_send_response(id, NULL);
    return;
  }
  cJSON *result = cJSON_CreateObject();
  cJSON_AddStringToObject(result, "uri", document.uri);
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
  DOCUMENT_LOCATION document = lsp_parse_document(params_json);

  BUFFER buffer = get_buffer(document.uri);
  char *text = strdup(buffer.content);
  truncate_string(text, document.line, document.character);
  const char *symbol_name_part  = extract_last_symbol(text);
  cJSON *result = symbol_completion(symbol_name_part, text);
  free(text);

  lsp_send_response(id, result);
}
