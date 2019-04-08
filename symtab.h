
#ifndef SYMTAB_H
#define SYMTAB_H

// Element in the symbol table
typedef struct sym_entry {
  char *   name;          // Symbol name
  unsigned kind;          // Symbol kind
  unsigned type;          // Symbol type
  unsigned atr1;          // Additional symbol attribute
  unsigned atr2;          // Additional symbol attribute
} SYMBOL_ENTRY;

// Returns index of the first empty element.
int get_next_empty_element(void);

// Returns index of the last occupied element.
int get_last_element(void);

/*
 * Inserts a new symbol (1 row in the table),
 * and returns index of the inserted element.
 * Returns -1 if there is no empty space in the symbol table.
 */
int insert_symbol(char *name, unsigned kind, unsigned type,
                  unsigned atr1, unsigned atr2);

// Inserts a literal into the symbol table (if it doesn't already exist).
int insert_literal(char *str, unsigned type);

/*
 * Returns index of the found element.
 * If the element is not found, returns -1.
 */
int lookup_symbol(char *name, unsigned kind);

// Setters and getters for element fields.
void     set_name(int index, char *name);
char*    get_name(int index);
void     set_kind(int index, unsigned kind);
unsigned get_kind(int index);
void     set_type(int index, unsigned type);
unsigned get_type(int index);
void     set_atr1(int index, unsigned atr1);
unsigned get_atr1(int index);
void     set_atr2(int index, unsigned atr2);
unsigned get_atr2(int index);

// Removes elements beginning with the specified index.
void clear_symbols(int begin_index);

// Removes all elements.
void clear_symtab(void);

// Prints all elements.
void print_symtab(void);
unsigned logarithm2(unsigned value);

// Initializes the table of symbols.
void init_symtab(void);

#endif
