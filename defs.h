#ifndef DEFS_H
#define DEFS_H

#define SYMBOL_TABLE_LENGTH   64
#define NO_ATR                 0
#define LAST_WORKING_REG      12
#define FUN_REG               13
#define CHAR_BUFFER_LENGTH   128
extern char char_buffer[CHAR_BUFFER_LENGTH];

// Output macros
int yyerror(const char *text);
extern int severity;
#define err(...)  sprintf(char_buffer, __VA_ARGS__), \
                      severity = ERROR, yyerror(char_buffer)
#define warn(...) sprintf(char_buffer, __VA_ARGS__), \
                      severity = WARNING, yyerror(char_buffer)
enum severity { ERROR = 1, WARNING, INFORMATION, HINT };

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
