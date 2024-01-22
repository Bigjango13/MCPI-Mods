// libinvscrolling: Allows you to use the mouse wheel to scroll in inventories
// Know bug: if you hover outside of the pane, and build up some scrolling, it will be applied once your mouse goes back into the pane
#include <SDL/SDL.h>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

// Config
#define SCROLL_SENSITIVITY 10
#define SCROLL_SPEED 4

// Reborn symbol split fix
#define ScrollingPane_setContentOffset ((ScrollingPane_setContentOffset_t) 0x220a0)
#define ScrollingPane_render ((ScrollingPane_render_t) 0x22ee4)
#define ScrollingPane_adjustContentSize ((ScrollingPane_adjustContentSize_t) 0x21aa4)
#define ScrollingPane_constructor ((ScrollingPane_constructor_t) 0x22b44)
#define RectangleArea_isInside ((RectangleArea_isInside_t) 0x1e720)
#define Mouse_getX ((Mouse_getX_t) 0x1385c)
#define Mouse_getY ((Mouse_getY_t) 0x1386c)

// The code
int targetScrollOffset = 0;
HOOK(SDL_PollEvent, int, (SDL_Event *event)) {
    ensure_SDL_PollEvent();
    int ret = real_SDL_PollEvent(event);
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

ScrollingPane *ScrollingPane_injection(ScrollingPane *self, int param_2, IntRectangle *bb, IntRectangle *itembb, int columns, int item_count, float scale, IntRectangle *itembb2) {
    ScrollingPane *ret = ScrollingPane_constructor(self, param_2, bb, itembb, columns, item_count, scale, itembb2);
    // This isn't set normally until the pane is clicked
    ScrollingPane_adjustContentSize(self);
    self->max_y = self->size_y - self->adjust_content_size_y;
    // Hack to fix scrolling building up even when not in a scrolling pane
    targetScrollOffset = 0;
    return ret;
}

void ScrollingPane_render_injection(ScrollingPane *self, int param_2, int param_3, float alpha) {
    // Check that the mouse is inside the pane
    float x = (float) Mouse_getX() * self->scale;
    float y = (float) Mouse_getY() * self->scale;

    bool inside = RectangleArea_isInside(&self->area, x, y);
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
        scroll += self->content_offset.y;
        scroll = std::max(scroll, self->max_y);
        scroll = std::min(scroll, 0.f);
        ScrollingPane_setContentOffset(self, self->content_offset.x, scroll);
    }
    // Call original
    ScrollingPane_render(self, param_2, param_3, alpha);
}

__attribute__((constructor)) static void init() {
    overwrite_calls((void*) ScrollingPane_render, (void*) ScrollingPane_render_injection);
    overwrite_calls((void*) ScrollingPane_constructor, (void*) ScrollingPane_injection);
}
