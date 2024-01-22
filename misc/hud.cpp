#include <cstdio>
#include <iomanip>
#include <sstream>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#define MAX_LEN (255 - 1)
Minecraft *minecraft = NULL;
void on_tick(Minecraft *mcpi) {
    minecraft = mcpi;
}

void ungetOffset(OffsetPosTranslator *pos_translator, float *x, float *y, float *z){
    // Removes the offset from the xyz.
    *x += pos_translator->x;
    *y += pos_translator->y;
    *z += pos_translator->z;
}

int format_hud(char *str, __attribute__((unused)) const char *format, ...){
    LocalPlayer *player = minecraft->player;
    if (player == NULL) return 0;
    float x = player->x, y = player->y, z = player->z;
    ungetOffset(&minecraft->command_server->pos_translator, &x, &y, &z);

    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << x << ", " <<  y << ", " << z << "\nMCPI-Reborn (FG6 edition)\n";

    if (minecraft->level) {
        for (auto const &player : minecraft->level->players) {
            if (player == NULL) continue;
            stream << "\n" << player->username;
        }
    }

    memcpy(str, stream.str().c_str(), MAX_LEN);

    return 0;
}

__attribute__((constructor)) static void init() {
    overwrite_call((void *) 0x269d8, (void *) format_hud);
}
