#ifndef DEFS_H
#define DEFS_H

#define bool int
#define TRUE  1
#define FALSE 0

#define SYMBOL_TABLE_LENGTH   64
#define NO_ATR                 0
#define LAST_WORKING_REG      12
#define FUN_REG               13
#define CHAR_BUFFER_LENGTH   128
extern char char_buffer[CHAR_BUFFER_LENGTH];

// Output macros
extern void warning(char *s);
extern int yyerror(char *s);
#define err(...)  sprintf(char_buffer, __VA_ARGS__), \
                      yyerror(char_buffer)
#define warn(...) sprintf(char_buffer, __VA_ARGS__), \
                      warning(char_buffer)


// Data types
enum types { NO_TYPE, INT, UINT };

// Kinds of symbols (maximum 32 different kinds)
enum kinds { NO_KIND = 0x1, REG = 0x2, LIT = 0x4,
             FUN = 0x8, VAR = 0x10, PAR = 0x20 };

// Arithmetic operators
enum arops { ADD, SUB, MUL, DIV, AROP_NUMBER };

// Relational operators
enum relops { LT, GT, LE, GE, EQ, NE, RELOP_NUMBER };

#endif
