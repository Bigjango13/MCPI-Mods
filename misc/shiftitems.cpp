#include <SDL/SDL.h>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

typedef unsigned char uchar;
typedef bool (*ScrollingPane_queryHoldTime_t)(uchar *scrollingPane, int *item_id, int *pressed_time);
ScrollingPane_queryHoldTime_t ScrollingPane_queryHoldTime = (ScrollingPane_queryHoldTime_t) 0x22ab4;

static bool shift_pressed = false;
HOOK(SDL_PollEvent, int, (SDL_Event *event)) {
    ensure_SDL_PollEvent();
    int ret = (*real_SDL_PollEvent)(event);
    // 340 is GLFW_KEY_LEFT_SHIFT
    if (event != NULL && event->type == SDL_USEREVENT && event->user.data2 == 340) {
        shift_pressed = event->user.data1 == SDL_PRESSED;
    }
    return ret;
}

static bool ScrollingPane_queryHoldTime_injection(uchar *scrollingPane, int *item_id, int *pressed_time) {
    // Call original
    bool ret = ScrollingPane_queryHoldTime(scrollingPane, item_id, pressed_time);
    if (ret && shift_pressed) {
        // The max time
        *pressed_time = 3460;
    }
    return ret;
}

__attribute__((constructor)) static void init() {
    overwrite_calls((void *) ScrollingPane_queryHoldTime, (void *) ScrollingPane_queryHoldTime_injection);
}
