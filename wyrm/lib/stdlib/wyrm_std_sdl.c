/*
 * wyrm_std_sdl.c - Wyrm Standard Library: SDL2 Runtime Binding
 *
 * Loads SDL2 at runtime via dlopen/LoadLibrary so no compile-time SDL2
 * headers or libraries are needed. Function pointers are resolved from
 * the shared library and called through the function pointer table.
 *
 * SDL2 ABI types are redefined here to avoid the SDL2 header dependency.
 */
#include "wyrm_std_sdl.h"
#include "wyrm_std_json.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/* --------------------------------------------------------------------------
 * Event type constants
 * -------------------------------------------------------------------------- */
const char *const WYRM_SDL_EVT_QUIT        = "quit";
const char *const WYRM_SDL_EVT_KEYDOWN     = "keydown";
const char *const WYRM_SDL_EVT_KEYUP       = "keyup";
const char *const WYRM_SDL_EVT_MOUSEMOTION = "mousemotion";
const char *const WYRM_SDL_EVT_MOUSEDOWN   = "mousedown";
const char *const WYRM_SDL_EVT_MOUSEUP     = "mouseup";
const char *const WYRM_SDL_EVT_NONE        = "none";

/* --------------------------------------------------------------------------
 * Platform-specific dynamic loading (duplicates from wyrm_ffi.c
 * intentionally - this module is self-contained)
 * -------------------------------------------------------------------------- */
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
typedef HMODULE SdlDlHandle;
static SdlDlHandle sdl_dl_open(const char *p) { return LoadLibraryA(p); }
static void       *sdl_dl_sym(SdlDlHandle h, const char *s) { return (void *)(uintptr_t)GetProcAddress(h, s); }
#else
#  include <dlfcn.h>
typedef void *SdlDlHandle;
static SdlDlHandle sdl_dl_open(const char *p) { return dlopen(p, RTLD_LAZY | RTLD_LOCAL); }
static void       *sdl_dl_sym(SdlDlHandle h, const char *s) { return dlsym(h, s); }
#endif

/* --------------------------------------------------------------------------
 * Minimal SDL2 ABI types (redefined without SDL2 headers)
 * -------------------------------------------------------------------------- */
typedef void  SDL2_Window;
typedef void  SDL2_Renderer;
typedef uint32_t SDL2_EventType;

#define SDL2_INIT_VIDEO   0x00000020u
#define SDL2_WINDOW_SHOWN 0x00000004u

#define SDL2_QUIT          0x100u
#define SDL2_KEYDOWN       0x300u
#define SDL2_KEYUP         0x301u
#define SDL2_MOUSEMOTION   0x400u
#define SDL2_MOUSEBUTTONDOWN 0x401u
#define SDL2_MOUSEBUTTONUP   0x402u

#define SDL2_WINDOWPOS_CENTERED 0x2FFF0000u

typedef struct { uint8_t scancode; } SDL2_Keysym;
typedef struct { SDL2_EventType type; uint8_t pad[3]; SDL2_Keysym keysym; } SDL2_KeyboardEvent;
typedef struct { SDL2_EventType type; uint8_t pad[3]; int32_t x; int32_t y; int32_t xrel; int32_t yrel; } SDL2_MouseMotionEvent;
typedef struct { SDL2_EventType type; uint8_t pad[3]; uint8_t button; int32_t x; int32_t y; } SDL2_MouseButtonEvent;

typedef union {
    SDL2_EventType        type;
    SDL2_KeyboardEvent    key;
    SDL2_MouseMotionEvent motion;
    SDL2_MouseButtonEvent button;
    uint8_t               padding[56];
} SDL2_Event;

/* --------------------------------------------------------------------------
 * SDL2 function pointer table
 * -------------------------------------------------------------------------- */
typedef struct {
    SdlDlHandle handle;
    int  (*SDL_Init)(uint32_t flags);
    void (*SDL_Quit)(void);
    SDL2_Window    *(*SDL_CreateWindow)(const char *title, int x, int y, int w, int h, uint32_t flags);
    SDL2_Renderer  *(*SDL_CreateRenderer)(SDL2_Window *win, int index, uint32_t flags);
    void (*SDL_DestroyWindow)(SDL2_Window *win);
    void (*SDL_DestroyRenderer)(SDL2_Renderer *ren);
    int  (*SDL_PollEvent)(SDL2_Event *event);
    int  (*SDL_SetRenderDrawColor)(SDL2_Renderer *ren, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    int  (*SDL_RenderClear)(SDL2_Renderer *ren);
    void (*SDL_RenderPresent)(SDL2_Renderer *ren);
    int  (*SDL_RenderFillRect)(SDL2_Renderer *ren, const void *rect);
    int  (*SDL_RenderDrawLine)(SDL2_Renderer *ren, int x1, int y1, int x2, int y2);
    void (*SDL_Delay)(uint32_t ms);
    uint32_t (*SDL_GetTicks)(void);
    const char *(*SDL_GetKeyName)(int scancode);
    const char *(*SDL_GetError)(void);
} SdlFnTable;

typedef struct {
    SDL2_Window   *window;
    SDL2_Renderer *renderer;
} WyrmSdlWindow;

static SdlFnTable g_sdl = { 0 };
static int g_sdl_loaded = 0;

/* --------------------------------------------------------------------------
 * Load SDL2 shared library
 * -------------------------------------------------------------------------- */
static int load_sdl2(void) {
    if (g_sdl_loaded) return 1;

    /* Resolve library path */
    const char *lib_path = getenv("WYRM_SDL2_PATH");
#ifdef _WIN32
    const char *default_path = "SDL2.dll";
#elif defined(__APPLE__)
    const char *default_path = "libSDL2-2.0.0.dylib";
#else
    const char *default_path = "libSDL2-2.0.so.0";
#endif
    if (!lib_path) lib_path = default_path;

    SdlDlHandle h = sdl_dl_open(lib_path);
    if (!h) {
        fprintf(stderr, "Runtime Error [sdl_init]: cannot load SDL2 library '%s'.\n"
                        "  Install SDL2 or set WYRM_SDL2_PATH to point to it.\n", lib_path);
        return 0;
    }

#define LOAD_SYM(fn) \
    *(void **)(&g_sdl.fn) = sdl_dl_sym(h, #fn); \
    if (!g_sdl.fn) { fprintf(stderr, "Runtime Error [sdl_init]: SDL2 symbol '%s' not found\n", #fn); return 0; }

    LOAD_SYM(SDL_Init)
    LOAD_SYM(SDL_Quit)
    LOAD_SYM(SDL_CreateWindow)
    LOAD_SYM(SDL_CreateRenderer)
    LOAD_SYM(SDL_DestroyWindow)
    LOAD_SYM(SDL_DestroyRenderer)
    LOAD_SYM(SDL_PollEvent)
    LOAD_SYM(SDL_SetRenderDrawColor)
    LOAD_SYM(SDL_RenderClear)
    LOAD_SYM(SDL_RenderPresent)
    LOAD_SYM(SDL_RenderFillRect)
    LOAD_SYM(SDL_RenderDrawLine)
    LOAD_SYM(SDL_Delay)
    LOAD_SYM(SDL_GetTicks)
    LOAD_SYM(SDL_GetKeyName)
    LOAD_SYM(SDL_GetError)
#undef LOAD_SYM

    g_sdl.handle  = h;
    g_sdl_loaded  = 1;
    return 1;
}

/* --------------------------------------------------------------------------
 * Helper: build an event map object
 * -------------------------------------------------------------------------- */
static Value make_event_map(const char *type, const char *key,
                             int scancode, int mx, int my, int btn) {
    Value obj = json_object();
    Value vtype  = val_string(type);
    Value vkey   = val_string(key);
    Value vsc    = val_number((double)scancode);
    Value vmx    = val_number((double)mx);
    Value vmy    = val_number((double)my);
    Value vbtn   = val_number((double)btn);
    Value k_type = val_string("type");
    Value k_key  = val_string("key");
    Value k_sc   = val_string("scancode");
    Value k_mx   = val_string("mouse_x");
    Value k_my   = val_string("mouse_y");
    Value k_btn  = val_string("button");
    obj = json_set(obj, k_type, vtype); val_drop(k_type); val_drop(vtype);
    obj = json_set(obj, k_key,  vkey);  val_drop(k_key);  val_drop(vkey);
    obj = json_set(obj, k_sc,   vsc);   val_drop(k_sc);   val_drop(vsc);
    obj = json_set(obj, k_mx,   vmx);   val_drop(k_mx);   val_drop(vmx);
    obj = json_set(obj, k_my,   vmy);   val_drop(k_my);   val_drop(vmy);
    obj = json_set(obj, k_btn,  vbtn);  val_drop(k_btn);  val_drop(vbtn);
    return obj;
}

/* --------------------------------------------------------------------------
 * Public: sdl_init
 * -------------------------------------------------------------------------- */
Value sdl_init(void) {
    if (!load_sdl2()) return val_error("sdl_init: failed to load SDL2");
    if (g_sdl.SDL_Init(SDL2_INIT_VIDEO) < 0) {
        const char *err = g_sdl.SDL_GetError ? g_sdl.SDL_GetError() : "unknown";
        char msg[512];
        snprintf(msg, sizeof(msg), "sdl_init: SDL_Init failed: %s", err);
        return val_error(msg);
    }
    return val_null();
}

/* --------------------------------------------------------------------------
 * Public: sdl_quit
 * -------------------------------------------------------------------------- */
Value sdl_quit(void) {
    if (g_sdl_loaded && g_sdl.SDL_Quit) g_sdl.SDL_Quit();
    g_sdl_loaded = 0;
    return val_null();
}

/* --------------------------------------------------------------------------
 * Public: sdl_window(title, width, height)
 * -------------------------------------------------------------------------- */
Value sdl_window(Value title, Value width, Value height) {
    if (!g_sdl_loaded) return val_error("sdl_window: call sdl_init() first");
    if (title.type != VAL_STRING) return val_error("sdl_window: title must be a string");
    if (width.type != VAL_NUMBER || height.type != VAL_NUMBER)
        return val_error("sdl_window: width and height must be numbers");

    int w = (int)width.as.number;
    int h = (int)height.as.number;
    if (w <= 0 || h <= 0) return val_error("sdl_window: width and height must be positive");

    SDL2_Window *win = g_sdl.SDL_CreateWindow(
        title.as.string,
        (int)SDL2_WINDOWPOS_CENTERED, (int)SDL2_WINDOWPOS_CENTERED,
        w, h,
        SDL2_WINDOW_SHOWN
    );
    if (!win) {
        const char *err = g_sdl.SDL_GetError ? g_sdl.SDL_GetError() : "unknown";
        char msg[512];
        snprintf(msg, sizeof(msg), "sdl_window: SDL_CreateWindow failed: %s", err);
        return val_error(msg);
    }

    SDL2_Renderer *ren = g_sdl.SDL_CreateRenderer(win, -1, 0);
    if (!ren) {
        g_sdl.SDL_DestroyWindow(win);
        const char *err = g_sdl.SDL_GetError ? g_sdl.SDL_GetError() : "unknown";
        char msg[512];
        snprintf(msg, sizeof(msg), "sdl_window: SDL_CreateRenderer failed: %s", err);
        return val_error(msg);
    }

    WyrmSdlWindow *ctx = malloc(sizeof(WyrmSdlWindow));
    if (!ctx) { g_sdl.SDL_DestroyRenderer(ren); g_sdl.SDL_DestroyWindow(win); return val_error("sdl_window: OOM"); }
    ctx->window   = win;
    ctx->renderer = ren;
    return val_raw_ptr(ctx);
}

/* --------------------------------------------------------------------------
 * Public: sdl_destroy_window
 * -------------------------------------------------------------------------- */
Value sdl_destroy_window(Value win_val) {
    if (win_val.type != VAL_RAW_PTR || !win_val.as.raw_ptr) return val_null();
    WyrmSdlWindow *ctx = (WyrmSdlWindow *)win_val.as.raw_ptr;
    if (ctx->renderer) g_sdl.SDL_DestroyRenderer(ctx->renderer);
    if (ctx->window)   g_sdl.SDL_DestroyWindow(ctx->window);
    free(ctx);
    return val_null();
}

/* --------------------------------------------------------------------------
 * Public: sdl_poll_event
 * -------------------------------------------------------------------------- */
Value sdl_poll_event(void) {
    if (!g_sdl_loaded) return val_error("sdl_poll_event: call sdl_init() first");
    SDL2_Event ev;
    if (!g_sdl.SDL_PollEvent(&ev)) {
        return make_event_map(WYRM_SDL_EVT_NONE, "", 0, 0, 0, 0);
    }
    switch (ev.type) {
        case SDL2_QUIT:
            return make_event_map(WYRM_SDL_EVT_QUIT, "", 0, 0, 0, 0);
        case SDL2_KEYDOWN: {
            int sc = (int)ev.key.keysym.scancode;
            const char *kname = g_sdl.SDL_GetKeyName ? g_sdl.SDL_GetKeyName(sc) : "";
            return make_event_map(WYRM_SDL_EVT_KEYDOWN, kname ? kname : "", sc, 0, 0, 0);
        }
        case SDL2_KEYUP: {
            int sc = (int)ev.key.keysym.scancode;
            const char *kname = g_sdl.SDL_GetKeyName ? g_sdl.SDL_GetKeyName(sc) : "";
            return make_event_map(WYRM_SDL_EVT_KEYUP, kname ? kname : "", sc, 0, 0, 0);
        }
        case SDL2_MOUSEMOTION:
            return make_event_map(WYRM_SDL_EVT_MOUSEMOTION, "", 0,
                                  (int)ev.motion.x, (int)ev.motion.y, 0);
        case SDL2_MOUSEBUTTONDOWN:
            return make_event_map(WYRM_SDL_EVT_MOUSEDOWN, "", 0,
                                  (int)ev.button.x, (int)ev.button.y, (int)ev.button.button);
        case SDL2_MOUSEBUTTONUP:
            return make_event_map(WYRM_SDL_EVT_MOUSEUP, "", 0,
                                  (int)ev.button.x, (int)ev.button.y, (int)ev.button.button);
        default:
            return make_event_map(WYRM_SDL_EVT_NONE, "", 0, 0, 0, 0);
    }
}

/* --------------------------------------------------------------------------
 * Helper: validate and extract WyrmSdlWindow from Value
 * -------------------------------------------------------------------------- */
static WyrmSdlWindow *get_ctx(const char *fn, Value win_val) {
    if (win_val.type != VAL_RAW_PTR || !win_val.as.raw_ptr) {
        fprintf(stderr, "Runtime Error [%s]: argument must be a window handle from sdl_window()\n", fn);
        exit(1);
    }
    return (WyrmSdlWindow *)win_val.as.raw_ptr;
}

/* --------------------------------------------------------------------------
 * Public: sdl_clear(win, r, g, b)
 * -------------------------------------------------------------------------- */
Value sdl_clear(Value win_val, Value r, Value g, Value b) {
    WyrmSdlWindow *ctx = get_ctx("sdl_clear", win_val);
    g_sdl.SDL_SetRenderDrawColor(ctx->renderer,
        (uint8_t)r.as.number, (uint8_t)g.as.number,
        (uint8_t)b.as.number, 255);
    g_sdl.SDL_RenderClear(ctx->renderer);
    return val_null();
}

/* --------------------------------------------------------------------------
 * Public: sdl_present(win)
 * -------------------------------------------------------------------------- */
Value sdl_present(Value win_val) {
    WyrmSdlWindow *ctx = get_ctx("sdl_present", win_val);
    g_sdl.SDL_RenderPresent(ctx->renderer);
    return val_null();
}

/* --------------------------------------------------------------------------
 * Public: sdl_draw_rect(win, x, y, w, h, r, g, b)
 * -------------------------------------------------------------------------- */
Value sdl_draw_rect(Value win_val, Value x, Value y, Value w, Value h,
                    Value r, Value g, Value b) {
    WyrmSdlWindow *ctx = get_ctx("sdl_draw_rect", win_val);
    g_sdl.SDL_SetRenderDrawColor(ctx->renderer,
        (uint8_t)r.as.number, (uint8_t)g.as.number,
        (uint8_t)b.as.number, 255);
    /* SDL_Rect layout: x, y, w, h (int32) */
    int32_t rect[4] = { (int32_t)x.as.number, (int32_t)y.as.number,
                         (int32_t)w.as.number, (int32_t)h.as.number };
    g_sdl.SDL_RenderFillRect(ctx->renderer, rect);
    return val_null();
}

/* --------------------------------------------------------------------------
 * Public: sdl_draw_line(win, x1, y1, x2, y2, r, g, b)
 * -------------------------------------------------------------------------- */
Value sdl_draw_line(Value win_val, Value x1, Value y1, Value x2, Value y2,
                    Value r, Value g, Value b) {
    WyrmSdlWindow *ctx = get_ctx("sdl_draw_line", win_val);
    g_sdl.SDL_SetRenderDrawColor(ctx->renderer,
        (uint8_t)r.as.number, (uint8_t)g.as.number,
        (uint8_t)b.as.number, 255);
    g_sdl.SDL_RenderDrawLine(ctx->renderer,
        (int)x1.as.number, (int)y1.as.number,
        (int)x2.as.number, (int)y2.as.number);
    return val_null();
}

/* --------------------------------------------------------------------------
 * Public: sdl_delay(ms)
 * -------------------------------------------------------------------------- */
Value sdl_delay(Value ms) {
    if (!g_sdl_loaded || !g_sdl.SDL_Delay) return val_null();
    uint32_t delay_ms = ms.type == VAL_NUMBER ? (uint32_t)ms.as.number : 0;
    g_sdl.SDL_Delay(delay_ms);
    return val_null();
}

/* --------------------------------------------------------------------------
 * Public: sdl_ticks()
 * -------------------------------------------------------------------------- */
Value sdl_ticks(void) {
    if (!g_sdl_loaded || !g_sdl.SDL_GetTicks) return val_number(0);
    return val_number((double)g_sdl.SDL_GetTicks());
}
