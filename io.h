#ifndef IO_H
#define IO_H

/*
 * Reads the entire contents of a file into a string.
 */
char* read_to_string(const char *uri);
/*
 * Truncates a given string to the specified length.
 */
void truncate_string(char *text, int line, int character);
/*
 * Returns the last symbol in a string
 */
const char* extract_last_symbol(char *text);

#endif
