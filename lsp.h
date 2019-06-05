#ifndef LSP_H
#define LSP_H

typedef struct cJSON cJSON;

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
 * Parses LSP text sync notifications and calls `lsp_lint`.
 */
void lsp_text_sync(const char *method, const cJSON *params_json);
/*
 * Runs a linter and returns LSP publish diagnostics notification.
 */
void lsp_lint(const char *uri, const char *text);
/*
 * Parses LSP hover request, and returns hover information.
 */
void lsp_hover(int id, const cJSON *params_json);

#endif
