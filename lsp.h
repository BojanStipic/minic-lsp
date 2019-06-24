#ifndef LSP_H
#define LSP_H

#include <cjson/cJSON.h>
#include "io.h"

/*
 * Main event loop.
 */
void lsp_event_loop(void);
/*
 * Parses message header and returns the content length.
 */
unsigned long lsp_parse_header(void);
/*
 * Converts message body of specified length to cJSON object.
 */
cJSON* lsp_parse_content(unsigned long content_length);
/*
 * Parses RPC request and calls the appropriate function.
 */
void json_rpc(const cJSON *request);
/*
 * Sends a LSP message response.
 */
void lsp_send_response(int id, cJSON *result);

// **************
// RPC functions:
// **************

/*
 * Parses LSP initialize request, and sends a response accordingly.
 * Response specifies language server's capabilities.
 */
void lsp_initialize(int id);
/*
 * Parses LSP shutdown request, and sends a response.
 */
void lsp_shutdown(int id);
/*
 * Stops the language server.
 */
void lsp_exit(void);
/*
 * Parses LSP text sync notifications, and updates buffers and diagnostics.
 */
void lsp_sync_open(const cJSON *params_json);
void lsp_sync_change(const cJSON *params_json);
void lsp_sync_close(const cJSON *params_json);
/*
 * Runs a linter and returns LSP publish diagnostics notification.
 */
void lsp_lint(Buffer buffer);
/*
 * Clears diagnostics for a file with specified `uri`.
 */
void lsp_lint_clear(const char *uri);
/*
 * Parses LSP hover request, and returns hover information.
 */
void lsp_hover(int id, const cJSON *params_json);
/*
 * Parses LSP definition request, and returns jump position.
 */
void lsp_goto_definition(int id, const cJSON *params_json);
/*
 * Parses LSP completion request, and returns completion results.
 */
void lsp_completion(int id, const cJSON *params_json);

#endif /* end of include guard: LSP_H */
