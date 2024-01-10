// libinvscrolling: Allows you to use the mouse wheel to scroll in inventories
// Know bug: if you hover outside of the pane, and build up some scrolling, it will be applied once your mouse goes back into the pane
#include <SDL/SDL.h>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

// Config
#define SCROLL_SENSITIVITY 10
#define SCROLL_SPEED 4

// Symbols
typedef unsigned char uchar;

typedef void (*ScrollingPane_setContentOffset_t)(uchar *scrollingPane, float x, float y);
ScrollingPane_setContentOffset_t ScrollingPane_setContentOffset = (ScrollingPane_setContentOffset_t) 0x220a0;
typedef void (*ScrollingPane_render_t)(uchar *scrollingPane, int param_2, int param_3, float param_4);
ScrollingPane_render_t ScrollingPane_render = (ScrollingPane_render_t) 0x22ee4;
typedef void (*ScrollingPane_adjustContentSize_t)(uchar *scrollingPane);
ScrollingPane_adjustContentSize_t ScrollingPane_adjustContentSize = (ScrollingPane_adjustContentSize_t) 0x21aa4;
typedef uchar *(*ScrollingPane_t)(uchar *scrollingPane, int param_2, uchar *rect1, uchar *rect2, int param_5, int param_6, float param_7, uchar *rect3);
ScrollingPane_t ScrollingPane = (ScrollingPane_t) 0x22b44;
static uint32_t ScrollingPane_content_offset_property_offset = 0xe4; // Vec3
static uint32_t ScrollingPane_scale_property_offset = 0x2c; // float
static uint32_t ScrollingPane_maxY_property_offset = 0x130; // float
static uint32_t ScrollingPane_adjustContentSizeY_property_offset = 0x1ec; // float
static uint32_t ScrollingPane_sizeY_property_offset = 0x170; // float
static uint32_t ScrollingPane_area_property_offset = 0x60; // RectangleArea

typedef bool (*RectangleArea_isInside_t)(uchar *rectangleArea, float x, float y);
RectangleArea_isInside_t RectangleArea_isInside = (RectangleArea_isInside_t) 0x1e720;

// The code
int targetScrollOffset = 0;
HOOK(SDL_PollEvent, int, (SDL_Event *event)) {
    ensure_SDL_PollEvent();
    int ret = (*real_SDL_PollEvent)(event);
    if (ret != 1 || event == NULL) return ret;
    if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP) {
        // Get the scroll
        if (event->button.button == SDL_BUTTON_WHEELUP) {
            targetScrollOffset += SCROLL_SENSITIVITY;
        } else if (event->button.button == SDL_BUTTON_WHEELDOWN) {
            targetScrollOffset -= SCROLL_SENSITIVITY;
        }
    }
    return ret;
}

uchar *ScrollingPane_injection(uchar *scrollingPane, int param_2, uchar *rect1, uchar *rect2, int param_5, int param_6, float param_7, uchar *rect3) {
    uchar *ret = ScrollingPane(scrollingPane, param_2, rect1, rect2, param_5, param_6, param_7, rect3);
    // This isn't set normally until the pane is clicked
    ScrollingPane_adjustContentSize(scrollingPane);
    *(float *) (scrollingPane + ScrollingPane_maxY_property_offset)
        = *(int *)(scrollingPane + ScrollingPane_sizeY_property_offset)
        - *(int *)(scrollingPane + ScrollingPane_adjustContentSizeY_property_offset);
    // Hack to fix scrolling building up even when not in a scrolling pane
    targetScrollOffset = 0;
    return ret;
}

void ScrollingPane_render_injection(uchar *scrollingPane, int param_2, int param_3, float param_4) {
    // Check that the mouse is inside the pane
    float scale = *(float *) (scrollingPane + ScrollingPane_scale_property_offset);
    float x = (float) Mouse_getX() * scale;
    float y = (float) Mouse_getY() * scale;
    uchar *areap = (uchar *) (scrollingPane + ScrollingPane_area_property_offset);
    bool inside = RectangleArea_isInside(areap, x, y);
    if (inside && targetScrollOffset != 0) {
        // Offset
        float scroll = 0;
        if (std::abs(targetScrollOffset) < SCROLL_SPEED) {
            scroll = targetScrollOffset;
            targetScrollOffset = 0;
        } else if (0 < targetScrollOffset) {
            scroll = SCROLL_SPEED;
            targetScrollOffset -= SCROLL_SPEED;
        } else if (0 > targetScrollOffset) {
            scroll = -SCROLL_SPEED;
            targetScrollOffset += SCROLL_SPEED;
        }
        // Block from going off screen
        Vec3 content_offset = *(Vec3 *) (scrollingPane + ScrollingPane_content_offset_property_offset);
        scroll += content_offset.y;
        scroll = std::max(scroll, *(float *) (scrollingPane + ScrollingPane_maxY_property_offset));
        scroll = std::min(scroll, 0.f);
        ScrollingPane_setContentOffset(scrollingPane, content_offset.x, scroll);
    }
    // Call original
    ScrollingPane_render(scrollingPane, param_2, param_3, param_4);
}

__attribute__((constructor)) static void init() {
    overwrite_calls((void*) ScrollingPane_render, (void*) ScrollingPane_render_injection);
    overwrite_calls((void*) ScrollingPane, (void*) ScrollingPane_injection);
}
