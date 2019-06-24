#ifndef IO_H
#define IO_H

#define BUFFER_LENGTH 100
typedef struct Buffer {
	char *uri;
	char *content;
} Buffer;

/*
 * Opens a new buffer.
 */
Buffer open_buffer(const char *uri, const char *content);
/*
 * Updates content of an existing buffer.
 */
Buffer update_buffer(const char *uri, const char *content);
/*
 * Searches a buffer by `uri` and returns its handle.
 */
Buffer get_buffer(const char *uri);
/*
 * Closes a buffer.
 */
void close_buffer(const char *uri);

/*
 * Truncates a given string to the specified length.
 */
void truncate_string(char *text, int line, int character);
/*
 * Returns the last symbol in a string
 */
const char* extract_last_symbol(char *text);

#endif /* end of include guard: IO_H */
