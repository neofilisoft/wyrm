/*
 * wyrm_std_sdl.h - Wyrm Standard Library: SDL2 Window & Input Binding
 *
 * Runtime-loaded SDL2 binding via dlopen/LoadLibrary.
 * SDL2 does NOT need to be linked at compile time - only SDL2.dll or
 * libSDL2.so needs to be present at runtime. The path is resolved by:
 *   1. WYRM_SDL2_PATH environment variable
 *   2. Platform default name (SDL2.dll / libSDL2.so / libSDL2-2.0.0.dylib)
 *
 * Wyrm SDL2 event map keys:
 *   "type"     -> string: "quit", "keydown", "keyup", "mousemotion",
 *                          "mousedown", "mouseup", "none"
 *   "key"      -> string: SDL key name (e.g. "Escape", "Space", "Up")
 *   "scancode" -> number: SDL scancode integer
 *   "mouse_x"  -> number: mouse X position
 *   "mouse_y"  -> number: mouse Y position
 *   "button"   -> number: mouse button index
 */
#ifndef WYRM_STD_SDL_H
#define WYRM_STD_SDL_H

#include "../wyrm_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize SDL2 (loads the SDL2 library). Must be called first.
 * Returns null on success, error string on failure.        */
Value sdl_init(void);

/* Shut down SDL2 and release resources. */
Value sdl_quit(void);

/* Create a window.
 *   title     - window title string
 *   width     - window width in pixels
 *   height    - window height in pixels
 * Returns a VAL_RAW_PTR(WyrmSdlWindow) or error.          */
Value sdl_window(Value title, Value width, Value height);

/* Destroy a window created by sdl_window. */
Value sdl_destroy_window(Value win_val);

/* Poll the next event from the SDL event queue.
 * Returns a JSON-style object map with event data.
 * Returns an object with type="none" if no event is pending. */
Value sdl_poll_event(void);

/* Clear the window to the given RGB color (0-255 each). */
Value sdl_clear(Value win_val, Value r, Value g, Value b);

/* Present (flip) the window back buffer. */
Value sdl_present(Value win_val);

/* Draw a filled rectangle. */
Value sdl_draw_rect(Value win_val, Value x, Value y, Value w, Value h,
                    Value r, Value g, Value b);

/* Draw a line. */
Value sdl_draw_line(Value win_val, Value x1, Value y1, Value x2, Value y2,
                    Value r, Value g, Value b);

/* Delay execution by ms milliseconds (useful for frame rate control). */
Value sdl_delay(Value ms);

/* Return the number of milliseconds since SDL initialization. */
Value sdl_ticks(void);

/* SDL event type constants as Values - used in Wyrm comparisons */
extern const char *const WYRM_SDL_EVT_QUIT;
extern const char *const WYRM_SDL_EVT_KEYDOWN;
extern const char *const WYRM_SDL_EVT_KEYUP;
extern const char *const WYRM_SDL_EVT_MOUSEMOTION;
extern const char *const WYRM_SDL_EVT_MOUSEDOWN;
extern const char *const WYRM_SDL_EVT_MOUSEUP;
extern const char *const WYRM_SDL_EVT_NONE;

#ifdef __cplusplus
}
#endif

#endif /* WYRM_STD_SDL_H */
