#ifndef MINIC_H
#define MINIC_H

#include <cjson/cJSON.h>

/*
 * Parse the `text` string and fill `diagnostics`.
 *
 * If `diagnostics` is NULL, only parsing is done
 * (useful to fill symtab without reporting diagnostics).
 */
void parse(cJSON *diagnostics, const char *text);

/*
 * Parse the `text` string and return info about the specified symbol.
 */
cJSON* symbol_info(const char *symbol_name, const char *text);

/*
 * Parse the `text` string and return definition location of the specified symbol.
 */
cJSON* symbol_location(const char *symbol_name, const char *text);

/*
 * Parse the `text` string and return all completions for the specified name part.
 */
cJSON* symbol_completion(const char *symbol_name_part, const char *text);

#endif /* end of include guard: MINIC_H */
