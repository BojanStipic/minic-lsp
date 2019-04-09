#ifndef LSP_H
#define LSP_H

typedef struct cJSON cJSON;

/*
 * Main event loop.
 */
void lsp_event_loop();
/*
 * Parses message header and returns the content length.
 */
unsigned long lsp_parse_header();
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
void lsp_exit();

#endif