#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "io.h"

char* read_to_string(const char *uri) {
  // uri + 7 -> skips file://
  FILE *f = fopen(uri + 7, "r");
  fseek(f, 0, SEEK_END);
  unsigned long fsize = ftell(f);
  rewind(f);

  char *text = malloc(fsize + 1);
  if(text == NULL)
    exit(2);
  fread(text, 1, fsize, f);
  fclose(f);

  text[fsize] = '\0';
  return text;
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
