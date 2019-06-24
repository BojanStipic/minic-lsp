#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "io.h"

Buffer buffers[BUFFER_LENGTH];
unsigned int first_empty;

Buffer open_buffer(const char *uri, const char *content) {
  if(first_empty >= BUFFER_LENGTH)
    exit(5);
  buffers[first_empty].uri = strdup(uri);
  buffers[first_empty].content = strdup(content);
  return buffers[first_empty++];
}

Buffer update_buffer(const char *uri, const char *content) {
  for(unsigned int i = 0; i < first_empty; i++) {
    if(strcmp(buffers[i].uri, uri) == 0) {
      free(buffers[i].content);
      buffers[i].content = strdup(content);
      return buffers[i];
    }
  }
  exit(5);
}

Buffer get_buffer(const char *uri) {
  for(unsigned int i = 0; i < first_empty; i++) {
    if(strcmp(buffers[i].uri, uri) == 0) {
      return buffers[i];
    }
  }
  exit(5);
}

void close_buffer(const char *uri) {
  for(unsigned int i = 0; i < first_empty; i++) {
    if(strcmp(buffers[i].uri, uri) == 0) {
      free(buffers[i].uri);
      free(buffers[i].content);
      for(unsigned int j = i; j < first_empty - 1; j++) {
        buffers[j] = buffers[j + 1];
      }
      --first_empty;
      return;
    }
  }
}

void truncate_string(char *text, int line, int character) {
  int position = 0;
  for(int i = 0; i < line; i++) {
    position += strcspn(text + position, "\n") + 1;
  }
  position += character;

  while(isalnum(*(text + position))) {
    ++position;
  }
  text[position] = '\0';
}

const char* extract_last_symbol(char *text) {
  unsigned int position = strlen(text) - 1;

  while(isalnum(*(text + position))) {
    --position;
  }
  ++position;

  return text + position;
}
