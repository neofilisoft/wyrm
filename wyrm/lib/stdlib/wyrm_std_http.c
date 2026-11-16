/*
 * wyrm_std_http.c - Wyrm Standard Library: HTTP Client Module Implementation
 *
 * Uses libcurl for all HTTP operations. Falls back gracefully if libcurl
 * is not available at compile time (WYRM_NO_CURL is defined).
 */
#include "wyrm_std_http.h"
#include "../wyrm_core.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WYRM_NO_CURL
#  include <curl/curl.h>
#  define WYRM_CURL_AVAILABLE 1
#else
#  define WYRM_CURL_AVAILABLE 0
#endif

/* ------------------------------------------------------------------ */
/* Response buffer helper                                               */
/* ------------------------------------------------------------------ */

struct WyrmCurlBuf {
    char  *data;
    size_t size;
};

#if WYRM_CURL_AVAILABLE
static size_t curl_write_cb(void *ptr, size_t size, size_t nmemb, void *userdata) {
    struct WyrmCurlBuf *buf = (struct WyrmCurlBuf *)userdata;
    size_t bytes = size * nmemb;
    char *new_data = (char *)realloc(buf->data, buf->size + bytes + 1);
    if (!new_data) return 0; /* curl will return CURLE_WRITE_ERROR */
    buf->data = new_data;
    memcpy(buf->data + buf->size, ptr, bytes);
    buf->size += bytes;
    buf->data[buf->size] = '\0';
    return bytes;
}

/* Build the standard response Value map. */
static Value make_response(long status, const char *body) {
    const char *fields[] = { "status", "body", "ok" };
    Value vals[] = {
        val_number((double)status),
        val_string(body ? body : ""),
        val_bool(status >= 200 && status < 300)
    };
    return val_struct_create("HttpResponse", 3, (const char **)fields, vals);
}

/* Common curl setup for all requests. */
static CURL *setup_curl(const char *url, struct WyrmCurlBuf *buf) {
    static int curl_inited = 0;
    if (!curl_inited) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl_inited = 1;
    }

    CURL *curl = curl_easy_init();
    if (!curl) return NULL;

    buf->data = (char *)malloc(1);
    buf->size = 0;
    if (!buf->data) { curl_easy_cleanup(curl); return NULL; }
    buf->data[0] = '\0';

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, buf);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Wyrm/1.2.1 (wyrm-lang.org)");
#ifdef _WIN32
    /* On Windows, skip SSL peer verification if no CA bundle is found.
     * In production, bundle a CA cert instead of disabling verification. */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif
    return curl;
}
#endif /* WYRM_CURL_AVAILABLE */

/* ------------------------------------------------------------------ */

Value http_available(void) {
    return val_bool(WYRM_CURL_AVAILABLE);
}

Value http_get(Value url) {
#if WYRM_CURL_AVAILABLE
    if (url.type != VAL_STRING || !url.as.string) {
        return val_error("http_get: url must be a string");
    }

    struct WyrmCurlBuf buf;
    CURL *curl = setup_curl(url.as.string, &buf);
    if (!curl) return val_error("http_get: failed to initialize curl");

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        char msg[512];
        snprintf(msg, sizeof(msg), "http_get: %s", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        free(buf.data);
        return val_error(msg);
    }

    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);

    Value result = make_response(status, buf.data);
    free(buf.data);
    return result;
#else
    (void)url;
    return val_error("http_get: libcurl not available. Rebuild with WYRM_NO_CURL undefined and link -lcurl.");
#endif
}

Value http_post(Value url, Value body) {
#if WYRM_CURL_AVAILABLE
    if (url.type != VAL_STRING || !url.as.string) {
        return val_error("http_post: url must be a string");
    }
    const char *post_data = (body.type == VAL_STRING && body.as.string) ? body.as.string : "";

    struct WyrmCurlBuf buf;
    CURL *curl = setup_curl(url.as.string, &buf);
    if (!curl) return val_error("http_post: failed to initialize curl");

    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(post_data));

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        char msg[512];
        snprintf(msg, sizeof(msg), "http_post: %s", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        free(buf.data);
        return val_error(msg);
    }

    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);

    Value result = make_response(status, buf.data);
    free(buf.data);
    return result;
#else
    (void)url; (void)body;
    return val_error("http_post: libcurl not available.");
#endif
}

Value http_post_json(Value url, Value body) {
#if WYRM_CURL_AVAILABLE
    if (url.type != VAL_STRING || !url.as.string) {
        return val_error("http_post_json: url must be a string");
    }
    const char *post_data = (body.type == VAL_STRING && body.as.string) ? body.as.string : "{}";

    struct WyrmCurlBuf buf;
    CURL *curl = setup_curl(url.as.string, &buf);
    if (!curl) return val_error("http_post_json: failed to initialize curl");

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(post_data));
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        char msg[512];
        snprintf(msg, sizeof(msg), "http_post_json: %s", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        free(buf.data);
        return val_error(msg);
    }

    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);

    Value result = make_response(status, buf.data);
    free(buf.data);
    return result;
#else
    (void)url; (void)body;
    return val_error("http_post_json: libcurl not available.");
#endif
}

Value http_get_headers(Value url, Value headers_array) {
#if WYRM_CURL_AVAILABLE
    if (url.type != VAL_STRING || !url.as.string) {
        return val_error("http_get_headers: url must be a string");
    }

    struct WyrmCurlBuf buf;
    CURL *curl = setup_curl(url.as.string, &buf);
    if (!curl) return val_error("http_get_headers: failed to initialize curl");

    struct curl_slist *headers = NULL;
    if (headers_array.type == VAL_ARRAY && headers_array.as.array) {
        for (int i = 0; i < headers_array.as.array->size; i++) {
            Value h = headers_array.as.array->data[i];
            if (h.type == VAL_STRING && h.as.string) {
                headers = curl_slist_append(headers, h.as.string);
            }
        }
    }
    if (headers) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    CURLcode res = curl_easy_perform(curl);
    if (headers) curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        char msg[512];
        snprintf(msg, sizeof(msg), "http_get_headers: %s", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        free(buf.data);
        return val_error(msg);
    }

    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);

    Value result = make_response(status, buf.data);
    free(buf.data);
    return result;
#else
    (void)url; (void)headers_array;
    return val_error("http_get_headers: libcurl not available.");
#endif
}
