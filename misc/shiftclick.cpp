#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

static int Tile_use_injection(Tile *tile, Level *level, int x, int y, int z, Player *player) {
    // Don't allow block interactions when sneaking
    if (
        player->vtable == (Player_vtable *) LocalPlayer_vtable_base
        && ((LocalPlayer *) player)->input->is_sneaking
    ) return 0;
    // Call original
    return tile->vtable->use(tile, level, x, y, z, player);
}

__attribute__((constructor)) static void init() {
    overwrite_call((void *) 0x1a870, (void *) Tile_use_injection);
}
