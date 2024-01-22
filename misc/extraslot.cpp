#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

void GuiComponent_blit_injection() {}

Gui_renderToolBar_t Gui_renderToolBar_original = NULL;
void Gui_renderToolBar_injection(Gui *gui, float param_1, int param_2, int param_3) {
    gui->num_slots = 9;
    Gui_renderToolBar_original(gui, param_1, param_2, param_3);
    gui->num_slots = 10;
}

__attribute__((constructor)) static void init() {
    Gui_renderToolBar_original = (Gui_renderToolBar_t) extract_from_bl_instruction((uchar *) 0x27704);
    overwrite_calls((void*) Gui_renderToolBar_original, (void*) Gui_renderToolBar_injection);
    // Fixes item texture rendering
    uchar sub0_patch[4] = {0x00, 0x20, 0x43, 0xe2}; // "sub r2,r3,#0x00"
    patch((void *) 0x26f00, sub0_patch);
    // Fixes durability rendering
    uchar nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
    patch((void *) 0x27038, nop_patch);
    // Fixes item count rendering
    patch((void *) 0x27120, nop_patch);
    // Blocks three dots from rendering
    overwrite_call((void*) 0x26f50, (void*) GuiComponent_blit_injection);
}
