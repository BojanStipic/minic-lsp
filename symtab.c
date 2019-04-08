
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "defs.h"
#include "symtab.h"

SYMBOL_ENTRY symbol_table[SYMBOL_TABLE_LENGTH];
int first_empty = 0;


// Returns index of the first empty element.
int get_next_empty_element(void) {
  if(first_empty < SYMBOL_TABLE_LENGTH)
    return first_empty++;
  else {
    err("Compiler error! Symbol table overflow!");
    exit(EXIT_FAILURE);
  }
}

// Returns index of the last occupied element.
int get_last_element(void) {
  return first_empty-1;
}

/*
 * Inserts a new symbol (1 row in the table),
 * and returns index of the inserted element.
 * Returns -1 if there is no empty space in the symbol table.
 */
int insert_symbol(char *name, unsigned kind, unsigned type,
                  unsigned atr1, unsigned atr2){
  int index = get_next_empty_element();
  symbol_table[index].name = name;
  symbol_table[index].kind = kind;
  symbol_table[index].type = type;
  symbol_table[index].atr1 = atr1;
  symbol_table[index].atr2 = atr2;
  return index;
}

// Inserts a literal into the symbol table (if it doesn't already exist).
int insert_literal(char *str, unsigned type) {
  int idx;
  for(idx = first_empty - 1; idx > FUN_REG; idx--) {
    if(strcmp(symbol_table[idx].name, str) == 0
       && symbol_table[idx].type == type)
       return idx;
  }

  // check literal range
  long int num = atol(str);
  if(((type==INT) && (num<INT_MIN || num>INT_MAX) )
    || ((type==UINT) && (num<0 || num>UINT_MAX)) )
      err("literal out of range");
  idx = insert_symbol(str, LIT, type, NO_ATR, NO_ATR);
  return idx;
}

/*
 * Returns index of the found element.
 * If the element is not found, returns -1.
 */
int lookup_symbol(char *name, unsigned kind) {
  int i;
  for(i = first_empty - 1; i > FUN_REG; i--) {
    if(strcmp(symbol_table[i].name, name) == 0
       && symbol_table[i].kind & kind)
       return i;
  }
  return -1;
}

void set_name(int index, char *name) {
  if(index > -1 && index < SYMBOL_TABLE_LENGTH)
    symbol_table[index].name = name;
}

char *get_name(int index) {
  if(index > -1 && index < SYMBOL_TABLE_LENGTH)
    return symbol_table[index].name;
  return "?";
}

void set_kind(int index, unsigned kind) {
  if(index > -1 && index < SYMBOL_TABLE_LENGTH)
    symbol_table[index].kind = kind;
}

unsigned get_kind(int index) {
  if(index > -1 && index < SYMBOL_TABLE_LENGTH)
    return symbol_table[index].kind;
  return NO_KIND;
}

void set_type(int index, unsigned type) {
  if(index > -1 && index < SYMBOL_TABLE_LENGTH)
    symbol_table[index].type = type;
}

unsigned get_type(int index) {
  if(index > -1 && index < SYMBOL_TABLE_LENGTH)
    return symbol_table[index].type;
  return NO_TYPE;
}

void set_atr1(int index, unsigned atr1) {
  if(index > -1 && index < SYMBOL_TABLE_LENGTH)
    symbol_table[index].atr1 = atr1;
}

unsigned get_atr1(int index) {
  if(index > -1 && index < SYMBOL_TABLE_LENGTH)
    return symbol_table[index].atr1;
  return NO_ATR;
}

void set_atr2(int index, unsigned atr2) {
  if(index > -1 && index < SYMBOL_TABLE_LENGTH)
    symbol_table[index].atr2 = atr2;
}

unsigned get_atr2(int index) {
  if(index > -1 && index < SYMBOL_TABLE_LENGTH)
    return symbol_table[index].atr2;
  return NO_ATR;
}

// Removes elements beginning with the specified index.
void clear_symbols(int begin_index) {
  int i;
  if(begin_index == first_empty) // Already empty
    return;
  if(begin_index > first_empty) {
    err("Compiler error! Wrong clear symbols argument");
    exit(EXIT_FAILURE);
  }
  for(i = begin_index; i < first_empty; i++) {
    if(symbol_table[i].name)
      free(symbol_table[i].name);
    symbol_table[i].name = 0;
    symbol_table[i].kind = NO_KIND;
    symbol_table[i].type = NO_TYPE;
    symbol_table[i].atr1 = NO_ATR;
    symbol_table[i].atr2 = NO_TYPE;
  }
  first_empty = begin_index;
}

// Removes all elements.
void clear_symtab(void) {
  first_empty = SYMBOL_TABLE_LENGTH - 1;
  clear_symbols(0);
}

// Prints all elements.
void print_symtab(void) {
  static const char *symbol_kinds[] = {
    "NONE", "REG", "LIT", "FUN", "VAR", "PAR" };
  int i;
  printf("\n\nSYMBOL TABLE\n");
  printf("\n       name           kind   type  atr1   atr2");
  printf("\n-- ---------------- -------- ----  -----  -----");
  for(i = 0; i < first_empty; i++) {
    printf("\n%2d %-19s %-4s %4d  %4d  %4d ", i,
    symbol_table[i].name,
    symbol_kinds[(int)(logarithm2(symbol_table[i].kind))],
    symbol_table[i].type,
    symbol_table[i].atr1,
    symbol_table[i].atr2);
  }
  printf("\n\n");
}

unsigned logarithm2(unsigned value) {
  unsigned mask = 1;
  int i = 0;
  for(i = 0; i < 32; i++) {
    if(value & mask)
      return i;
    mask <<= 1;
  }
  return 0;
}

// Initializes the table of symbols.
void init_symtab(void) {
  clear_symtab();

  int i = 0;
  char s[4];
  for(i = 0; i < 14; i++) {
    sprintf(s, "%%%d", i);
    insert_symbol(strdup(s), REG, NO_TYPE, NO_ATR, NO_ATR);
  }
}
