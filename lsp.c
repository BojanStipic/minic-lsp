#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "lsp.h"
#define MAX_HEADER_FIELD_LEN 100

void lsp_event_loop() {
  for(;;) {
    unsigned long content_length = lsp_parse_header();
    cJSON *request = lsp_parse_content(content_length);
    json_rpc(request);
    cJSON_Delete(request);
  }
}

unsigned long lsp_parse_header() {
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
  char *log_output = cJSON_Print(request);
  FILE *log = fopen("lsp.log", "a");
  fwrite(log_output, 1, strlen(log_output), log);
  fclose(log);
  // <== Logs for debugging

  const char *method;
  int id;

  const cJSON *method_json = cJSON_GetObjectItem(request, "method");
  if (cJSON_IsString(method_json) && (method_json->valuestring != NULL)) {
    method = method_json->valuestring;
  }
  const cJSON *id_json = cJSON_GetObjectItem(request, "id");
  if (cJSON_IsNumber(id_json)) {
    id = id_json->valueint;
  }
  //const cJSON *params_json = cJSON_GetObjectItem(request, "params");

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
