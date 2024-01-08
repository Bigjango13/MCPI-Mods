#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

typedef unsigned char uchar;
static uint32_t Player_input_property_offset = 0xc88; // Input
static uint32_t Input_sneaking_property_offset = 0xe; // bool

static int Tile_use_injection(uchar *tile, uchar *level, int x, int y, int z, uchar *player) {
    uchar *input = *(uchar **) (player + Player_input_property_offset);
    bool is_sneaking = *(bool *) (input + Input_sneaking_property_offset);
    // Don't allow block interactions when sneaking
    if (is_sneaking) return 0;
    // Call original
    uchar *vtable = *(uchar **) tile;
    Tile_use_t Tile_use = *(Tile_use_t *) (vtable + Tile_use_vtable_offset);
    return Tile_use(tile, level, x, y, z, player);
}

__attribute__((constructor)) static void init() {
    overwrite_call((void *) 0x1a870, (void *) Tile_use_injection);
}
