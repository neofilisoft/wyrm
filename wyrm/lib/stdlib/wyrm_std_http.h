/*
 * wyrm_std_http.h - Wyrm Standard Library: HTTP Client Module
 *
 * HTTP client using libcurl. Requires libcurl to be available on the target
 * system. On Windows: link with -lcurl. On Linux/macOS: link with -lcurl.
 *
 * Install libcurl:
 *   Windows (MSYS2):   pacman -S mingw-w64-x86_64-curl
 *   Ubuntu/Debian:     apt install libcurl4-openssl-dev
 *   macOS (Homebrew):  brew install curl
 */
#ifndef WYRM_STD_HTTP_H
#define WYRM_STD_HTTP_H

#include "../wyrm_core.h"

/* Performs a GET request to the given URL.
 * Returns a map: { "status": number, "body": string, "ok": bool }
 * Returns val_error("message") on network/curl failure. */
Value http_get(Value url);

/* Performs a POST request to the given URL with the given body string.
 * Returns a map: { "status": number, "body": string, "ok": bool }
 * Returns val_error("message") on failure. */
Value http_post(Value url, Value body);

/* Performs a POST request with a Content-Type header.
 * http_post_json(url, json_string) sets Content-Type: application/json.
 * Returns a map: { "status": number, "body": string, "ok": bool } */
Value http_post_json(Value url, Value body);

/* Sets a custom header for all subsequent requests (thread-local effect).
 * headers_map: array of "Key: Value" strings.
 * Returns val_bool(true). */
Value http_get_headers(Value url, Value headers_array);

/* Returns val_bool(true) if libcurl is available and initialized. */
Value http_available(void);

#endif /* WYRM_STD_HTTP_H */
